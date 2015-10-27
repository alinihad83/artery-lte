//
// Copyright (C) 2014 Raphael Riebl <raphael.riebl@thi.de>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "artery/application/ItsG5LTEMiddleware.h"
#include "artery/application/ItsG5Service.h"
#include "artery/mac/AccessCategories.h"
#include "artery/mac/AccessCategoriesVanetza.h"
#include "artery/mac/MacToGeoNetControlInfo.h"
#include "artery/messages/ChannelLoadReport_m.h"
#include "artery/messages/GeoNetPacket_m.h"
#include "artery/netw/GeoNetToMacControlInfo.h"
#include "veins/base/connectionManager/ChannelAccess.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include <vanetza/btp/header.hpp>
#include <vanetza/btp/header_conversion.hpp>
#include <vanetza/btp/ports.hpp>
#include <vanetza/geonet/data_confirm.hpp>
#include <vanetza/geonet/packet.hpp>
#include <vanetza/geonet/position_vector.hpp>
#include <vanetza/net/mac_address.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/lexical_cast.hpp>
#include "cpacket_byte_buffer_convertible.h"
#include <algorithm>
#include <regex>
#include <string>

Define_Module(ItsG5LTEMiddleware);

const simsignalwrap_t cMobilityStateChangedSignal(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

long ItsG5LTEMiddleware::sentMessagesViaLte = 0;
long ItsG5LTEMiddleware::sentBytesViaLte = 0;
bool ItsG5LTEMiddleware::isStatisticsPrinted = false;

ItsG5LTEMiddleware::ItsG5LTEMiddleware() {
}

void ItsG5LTEMiddleware::request(const vanetza::btp::DataRequestB& req, std::unique_ptr<cPacket> payload){
    Enter_Method ("request");

    cPacket *tmp = payload.release();
    LTEReport *lteReport = dynamic_cast<LTEReport* >(tmp);
    this->take(lteReport);

    if (lteReport != nullptr) {

        // implement connection to LTE-module
        IPv4Address address = IPvXAddressResolver().resolve(lteReport->getDst()).get4();
        if (address.isUnspecified()) {
            address = manager->getIPAddressForID(lteReport->getDst());
        }
        if (address.isUnspecified()) {
            opp_error((std::string("Address of ") + lteReport->getDst() + " still unspecified!").c_str());
            return;
        }

        // Update statistics
        ++sentMessagesViaLte;
        sentBytesViaLte += lteReport->getByteLength();

        if(debug) {
            std::cout << "[ITSG5LTEMiddleware] Sending LTEReport from node" << lteReport->getSrc() << std::endl;
        }
        socket.sendTo(lteReport, address, ltePort);
    } else {
        opp_error("Unable to extract LTEReport out of payload");
    }
}

void ItsG5LTEMiddleware::request(const vanetza::btp::DataRequestB& req, std::unique_ptr<vanetza::btp::DownPacket> payload){
    Enter_Method("request");
    using namespace vanetza;
    btp::HeaderB btp_header;
    btp_header.destination_port = req.destination_port;
    btp_header.destination_port_info = req.destination_port_info;
    payload->layer(OsiLayer::Transport) = btp_header;


    if(debug) {
        std::cout << "[ItsG5LTEMiddleware] Sending with DSRC" << endl;
    }

    switch (req.gn.transport_type) {
    case geonet::TransportType::SHB: {
        geonet::ShbDataRequest request(mGeoMib);
        request.upper_protocol = geonet::UpperProtocol::BTP_B;
        request.communication_profile = req.gn.communication_profile;
        if (req.gn.maximum_lifetime) {
            request.maximum_lifetime = req.gn.maximum_lifetime.get();
        }
        request.repetition = req.gn.repetition;
        request.traffic_class = req.gn.traffic_class;
        mGeoRouter.request(request, std::move(payload));
    }
        break;
    case geonet::TransportType::GBC: {
        geonet::GbcDataRequest request(mGeoMib);
        request.destination = boost::get<geonet::Area>(req.gn.destination);
        request.upper_protocol = geonet::UpperProtocol::BTP_B;
        request.communication_profile = req.gn.communication_profile;
        if (req.gn.maximum_lifetime) {
            request.maximum_lifetime = req.gn.maximum_lifetime.get();
        }
        request.repetition = req.gn.repetition;
        request.traffic_class = req.gn.traffic_class;
        mGeoRouter.request(request, std::move(payload));
    }
        break;
    default:
        opp_error("Unknown or unimplemented transport type");
        break;
    }
}

void ItsG5LTEMiddleware::initialize(int stage) {
    ItsG5Middleware::initialize(stage);
    switch (stage) {
        case 0:
            initializeMiddleware();
            debug = par("debug").boolValue();
            atexit(ItsG5LTEMiddleware::printStats);
            break;
        default:
            break;
    }
}

void ItsG5LTEMiddleware::initializeMiddleware() {
    manager = Veins::TraCIScenarioManagerAccess().get();

    toLte = findGate("toLte");
    fromLte = findGate("fromLte");
    socket.setOutputGate(gate(toLte));
    ltePort = par("ltePort");
    socket.bind(ltePort);
}

void ItsG5LTEMiddleware::printStats() {
    if (!isStatisticsPrinted) {
        std::cout << "[ItsG5LTEMiddleware] Sent " << sentMessagesViaLte << " messages via LTE" << std::endl;
        std::cout << "[ItsG5LTEMiddleware] Sent " << sentBytesViaLte << " bytes via LTE." << std::endl;

        isStatisticsPrinted = true;
    }
}

void ItsG5LTEMiddleware::finish() {
    ItsG5Middleware::finish();
}

void ItsG5LTEMiddleware::handleMessage(cMessage *msg) {
    // This clock has to be steady, but no time base required
    mClock = decltype(mClock) { std::chrono::milliseconds(simTime().inUnit(SIMTIME_MS)) };

    // Message arrival for services expects GeoNet-Packets

    // check if message arrived via LTE
    int arrivalGate = msg->getArrivalGateId();
    if (arrivalGate == fromLte) {
        // LTE Message handling
        //handleLowerLteMessage(msg);
        handleLowerMsg(msg);
    } else {
        // ITS G5 Message handling
        // Don't forget to dispatch message properly
        BaseApplLayer::handleMessage(msg);
    }
}

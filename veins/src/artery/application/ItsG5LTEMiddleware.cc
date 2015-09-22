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
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <regex>
#include <string>

Define_Module(ItsG5LTEMiddleware);

const simsignalwrap_t cMobilityStateChangedSignal(MIXIM_SIGNAL_MOBILITY_CHANGE_NAME);

ItsG5LTEMiddleware::ItsG5LTEMiddleware() {
}

void ItsG5LTEMiddleware::request(const vanetza::access::DataRequest& req,
        std::unique_ptr<vanetza::geonet::DownPacket> payload) {
    Enter_Method_Silent();
    request(req, std::move(payload), false);
}

void ItsG5LTEMiddleware::request(const vanetza::access::DataRequest& req,
        std::unique_ptr<vanetza::geonet::DownPacket> payload, bool sendWithLte) {
    Enter_Method_Silent();
    GeoNetToMacControlInfo* macCtrlInfo = new GeoNetToMacControlInfo();
    macCtrlInfo->access_category = edca::map(req.access_category);
    macCtrlInfo->destination_addr = convertToL2Type(req.destination_addr);
    macCtrlInfo->source_addr = convertToL2Type(req.source_addr);

    if ((*payload)[vanetza::OsiLayer::Network].ptr() == nullptr) {
        opp_error("Missing network layer payload in middleware request");
    }

    GeoNetPacket* net = new GeoNetPacket("GeoNet");
    net->setByteLength(payload->size());
    net->setPayload(GeoNetPacketWrapper(std::move(payload)));
    net->setControlInfo(macCtrlInfo);
    net->addBitLength(mAdditionalHeaderBits);
    if (sendWithLte) {
        // implement connection to LTE-module
        IPv4Address address = IPvXAddressResolver().resolve("server").get4();
        if (address.isUnspecified()) {
            address = manager->getIPAddressForID("server");
        }
        if (address.isUnspecified()) {
            opp_error("Address of server still unspecified!");
            delete macCtrlInfo;
            delete net;
            return;
        }
        socket.sendTo(net, address, ltePort);
    } else {
        // send via ITS G5
        sendDown(net);
    }
}

void ItsG5LTEMiddleware::initialize(int stage) {
    ItsG5Middleware::initialize(stage);
    switch (stage) {
    case 0:
        fromLte = findGate("fromLte");
        toLte = findGate("toLte");
        break;
    default:
        break;
    }
}

void ItsG5LTEMiddleware::initializeMiddleware() {
    ItsG5Middleware::initializeMiddleware();
    manager = Veins::TraCIScenarioManagerAccess().get();

    toLte = findGate("toLte");
    fromLte = findGate("fromLte");
    socket.setOutputGate(gate(toLte));
    ltePort = par("ltePort");
    socket.bind(ltePort);
}

void ItsG5LTEMiddleware::finish() {
    ItsG5Middleware::finish();
}

void ItsG5LTEMiddleware::handleMessage(cMessage *msg) {
    // This clock has to be steady, but no time base required
    mClock = decltype(mClock) { std::chrono::milliseconds(
            simTime().inUnit(SIMTIME_MS)) };

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

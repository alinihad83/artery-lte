//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "VehicleTelemetryService.h"
#include <vanetza/btp/data_request.hpp>
#include <vanetza/dcc/profile.hpp>
#include <vanetza/geonet/interface.hpp>
#include <artery/messages/LTEReport_m.h>

static const simsignal_t scSignalCamReceived = cComponent::registerSignal("CaService.received");

Define_Module(VehicleTelemetryService);

using namespace vanetza;

VehicleTelemetryService::VehicleTelemetryService()
{
}

VehicleTelemetryService::~VehicleTelemetryService()
{
}

void VehicleTelemetryService::indicate(const btp::DataIndication& ind, cPacket* packet)
{
	if (packet->getByteLength() == 42) {
		findHost()->bubble("packet indication");
	}

	delete(packet);
}

void VehicleTelemetryService::initialize()
{
	ItsG5BaseService::initialize();
	debug = par("debug").boolValue();
	if(debug) {
	    std::cout << "Init "<< this->getFullPath() << std::endl;
	}
	lteTransmissionInterval = par("lteTransmissionInterval");
	lteTransmissionMaxDistance = par( "lteTransmissionMaxDistance" );
	// Initialize point in time when last message was supposedly sent
	//   to a random point in time in the transmission interval:
	lastMessageSent = ev.getRNG( 0 )->doubleRand() * lteTransmissionInterval;
	lastMessagePosition = getFacilities().getMobility().getVehicleCommandInterface()->getPosition();
	if( debug ) {
	    std::cout << "[" << this->getFullPath() << "]: Beginning sending at " << lastMessageSent << " + " << lteTransmissionInterval << std::endl;
	}
	subscribe(scSignalCamReceived);
}

void VehicleTelemetryService::finish()
{
}

void VehicleTelemetryService::handleMessage(cMessage* msg)
{
	Enter_Method("handleMessage");
}

void VehicleTelemetryService::trigger()
{
	Enter_Method("trigger");

	Coord pos = getFacilities().getMobility().getVehicleCommandInterface()->getPosition();

	if (simTime() - lastMessageSent > lteTransmissionInterval
	        // FIXME: getDistance() sometimes returns 1.79769e+308 which seems to be value for 'undefined'?
	        || getFacilities().getMobility().getCommandInterface()->getDistance( lastMessagePosition, pos, true ) > lteTransmissionMaxDistance
	        ) {

	    btp::DataRequestB req;
	    req.destination_port = host_cast<VehicleTelemetryService::port_type>(getPortNumber());
	    req.gn.transport_type = geonet::TransportType::SHB;
	    req.gn.traffic_class.tc_id(static_cast<unsigned>(dcc::Profile::DP3));
	    req.gn.communication_profile = geonet::CommunicationProfile::ITS_G5;

	    if( debug ) {
	        std::cout << "[" << this->getFullPath() << "@" << simTime() << "] Last message sent at time " << lastMessageSent
	                  << ", distance " << getFacilities().getMobility().getCommandInterface()->getDistance( lastMessagePosition, pos, true )
	                  << std::endl;
	    }

	    lastMessageSent = simTime();
	    lastMessagePosition = pos;

        // simulation info
        simtime_t time = simTime();

        // vehicle info
        std::string sumoId = getFacilities().getMobility().getExternalId();
        std::string roadId = getFacilities().getMobility().getVehicleCommandInterface()->getRoadId();
        int32_t laneIndex = getFacilities().getMobility().getVehicleCommandInterface()->getLaneIndex();
        double lanePosition = getFacilities().getMobility().getVehicleCommandInterface()->getLanePosition();
        // pos from above
        double speed = getFacilities().getMobility().getSpeed();
        std::string vehicleType = getFacilities().getMobility().getVehicleCommandInterface()->getTypeId();
        double vehicleLength = getFacilities().getMobility().getCommandInterface()->vehicletype(vehicleType).getLength();

        LTEReport *report = new LTEReport();
        report->setDst("server");
        report->addByteLength(sizeof(char)*6);

        report->setSrc(sumoId.c_str());
        report->addByteLength(sumoId.size());

        report->setRoadId(roadId.c_str());
        report->addByteLength(roadId.size());

        report->setLaneIndex(laneIndex);
        report->addByteLength(sizeof(int32_t));

        report->setXPosition(pos.x);
        report->addByteLength(sizeof(double));

        report->setYPosition(pos.y);
        report->addByteLength(sizeof(double));

        report->setLanePosition(lanePosition);
        report->addByteLength(sizeof(double));

        report->setSpeed(speed);
        report->addByteLength(sizeof(double));

        report->setVehicleType(vehicleType.c_str());
        report->addByteLength(vehicleType.size());

        report->setVehicleLength(vehicleLength);
        report->addByteLength(sizeof(double));

        report->setSendingTime(time);
        report->addByteLength(sizeof(long));

        request(req, report, true);
	}

}

void VehicleTelemetryService::receiveSignal(cComponent* source, simsignal_t signal, bool valid)
{
	if (signal == scSignalCamReceived && valid) {
		EV << "Vehicle " << getFacilities().getMobility().getExternalId() << " received a CAM in sibling service\n";
	}
}

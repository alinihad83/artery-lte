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

#include "ExampleService.h"
#include <vanetza/btp/data_request.hpp>
#include <vanetza/dcc/profile.hpp>
#include <vanetza/geonet/interface.hpp>
#include <artery/messages/LTEReport_m.h>

static const simsignal_t scSignalCamReceived = cComponent::registerSignal("CaService.received");

Define_Module(ExampleService);

using namespace vanetza;

// TODO rename
ExampleService::ExampleService()
{
}

ExampleService::~ExampleService()
{
}

void ExampleService::indicate(const btp::DataIndication& ind, cPacket* packet)
{
	if (packet->getByteLength() == 42) {
		findHost()->bubble("packet indication");
	}

	delete(packet);
}

void ExampleService::initialize()
{
	ItsG5BaseService::initialize();
	m_self_msg = new cMessage("Example Service");
	subscribe(scSignalCamReceived);

	// TODO parameterize
	scheduleAt(simTime() + 3.0, m_self_msg);
}

void ExampleService::finish()
{
	cancelAndDelete(m_self_msg);
}

void ExampleService::handleMessage(cMessage* msg)
{
	Enter_Method("handleMessage");
	if (msg == m_self_msg) {
		findHost()->bubble("self message");
	}
}

void ExampleService::trigger()
{
	Enter_Method("trigger");
	btp::DataRequestB req;
	req.destination_port = host_cast<ExampleService::port_type>(getPortNumber());
	req.gn.transport_type = geonet::TransportType::SHB;
	req.gn.traffic_class.tc_id(static_cast<unsigned>(dcc::Profile::DP3));
	req.gn.communication_profile = geonet::CommunicationProfile::ITS_G5;

	// simulation info
    simtime_t time = simTime();

	// vehicle info
	std::string sumoId = getFacilities().getMobility().getExternalId();
    std::string roadId = getFacilities().getMobility().getVehicleCommandInterface()->getRoadId();
    int32_t laneIndex = getFacilities().getMobility().getVehicleCommandInterface()->getLaneIndex();
    double lanePosition = getFacilities().getMobility().getVehicleCommandInterface()->getLanePosition();
    double speed = getFacilities().getMobility().getSpeed();
    std::string vehicleType = getFacilities().getMobility().getVehicleCommandInterface()->getTypeId();
    double vehicleLength = getFacilities().getMobility().getCommandInterface()->vehicletype(vehicleType).getLength();

	LTEReport *report = new LTEReport();
	report->setDst("server");
	report->setSrc(sumoId.c_str());
	report->setRoadId(roadId.c_str());
	report->setLaneIndex(laneIndex);
	report->setLanePosition(lanePosition);
	report->setSpeed(speed);
	report->setVehicleType(vehicleType.c_str());
	report->setVehicleLength(vehicleLength);
	report->setByteLength(42); //TODO set actual byte length
	report->setSendingTime(time);

    request(req, report, true);
}

void ExampleService::receiveSignal(cComponent* source, simsignal_t signal, bool valid)
{
	if (signal == scSignalCamReceived && valid) {
		EV << "Vehicle " << getFacilities().getMobility().getExternalId() << " received a CAM in sibling serivce\n";
	}
}

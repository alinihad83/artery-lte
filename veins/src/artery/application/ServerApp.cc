#include "ServerApp.h"
#include <string.h>
#include <list>
#include <artery/messages/LTEReport_m.h>

Define_Module(ServerApp);

ServerApp::ServerApp() {
}

ServerApp::~ServerApp() {
}

void ServerApp::initialize(int stage) {
    ApplicationBase::initialize(stage);
    if (stage == 0) {

        debug = par("debug").boolValue();
        traciLogInterval = par("traciLogInterval").doubleValue();

        receivedMessagesViaLte = 0;
        receivedMessagesViaDsrc = 0;
        receivedBytes = 0;

        udpIn = findGate("udpIn");
        udpOut = findGate("udpOut");

        scenarioManager = TraCIScenarioManagerAccess().get();
        ASSERT(scenarioManager);

        // Open database connection
        db = new ServerDatabase();

        scheduleAt(simTime() + traciLogInterval, new cMessage("self")); // FIXME: when is the right time for initialization?
    }
}

void ServerApp::finish() {
    std::cout << "[ServerApp] Received " << receivedMessagesViaLte << " messages via LTE" << std::endl;
    std::cout << "[ServerApp] Received " << receivedBytes << " bytes via LTE." << std::endl;
    std::cout << "[ServerApp] Received " << receivedMessagesViaDsrc << " messages via DSRC." << std::endl;
}

void ServerApp::handleMessageWhenUp(cMessage *msg) {
    if (!msg->isSelfMessage()) {
        int gateId = msg->getArrivalGateId();
        if (gateId == udpIn) {

            cPacket* report = dynamic_cast<cPacket*>(msg);

            if (report != NULL && simTime() > 0) {

                LTEReport* lte = dynamic_cast<LTEReport*>(report);

                if (lte != NULL && simTime() > 0) {

                    ++receivedMessagesViaLte;
                    std::string source = lte->getSrc();
                    receivedBytes += lte->getByteLength();
                    std::cout << "[ServerApp] Received LTEReport from " <<  source << std::endl;

                }
            }

        }
    } else if (msg->isName("self") && simTime() > 0) {

            traci = scenarioManager->getCommandInterface();

            // Get the information of all vehicles in the simulation via TraCI and store it in the database
            std::list<std::string> allVehicles = traci->getVehicleIds();

            for (std::list<std::string>::const_iterator it = allVehicles.begin(); it != allVehicles.end(); it++) {
                std::string vehicleId = it->c_str();
                Veins::TraCICommandInterface::Vehicle v = traci->vehicle(vehicleId);

                // Vehicle already processed in an earlier iteration?
                const bool is_in = insertedVehicles.find(vehicleId) != insertedVehicles.end();

                if (!is_in) {
                    double length = traci->vehicletype(v.getTypeId()).getLength();
                    db->insertVehicle(vehicleId, v.getTypeId(), length);
                    insertedVehicles.insert(vehicleId);
                }

                //std::string roadId = v.getRoadId();
                //int32_t laneIndex = v.getLaneIndex();

                //double lanePosition = v.getLanePosition();
                //double speed = v.getSpeed();
                simtime_t simtime = simTime();
                // inserTraci(it->c_str());
            }

            scheduleAt(simTime() + traciLogInterval, new cMessage("self")); //schedule next measurement
    }
    delete msg; // FIXME warning: can't find linker symbol for virtual table for `cMessage' value
}

bool ServerApp::handleNodeStart(IDoneCallback *doneCallback) {
    socket.setOutputGate(gate("udpOut"));
    int localPort = par("localPort");
    socket.bind(localPort);
    return true;
}

bool ServerApp::handleNodeShutdown(IDoneCallback *doneCallback) {
    return true;
}

void ServerApp::handleNodeCrash() {
}

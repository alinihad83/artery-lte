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
        if(debug){
            std::cout << "Init " << this->getFullPath() << std::endl;
        }
        traciLogInterval = par("traciLogInterval");

        receivedMessagesViaLte = 0;
        receivedMessagesViaDsrc = 0;
        receivedBytes = 0;

        udpIn = findGate("udpIn");
        udpOut = findGate("udpOut");

        scenarioManager = TraCIScenarioManagerAccess().get();
        ASSERT(scenarioManager);

        // Open database connection
        db = new ServerDatabase();

    } else if (stage == 1) {
        scheduleAt(simTime() + traciLogInterval, new cMessage("self"));
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
        if (gateId == udpIn) { // Message arrived via LTE

            cPacket* packet = dynamic_cast<cPacket*>(msg);

            if (packet != nullptr && simTime() > 0) {

                LTEReport* report = dynamic_cast<LTEReport*>(packet);

                if (report != nullptr) {

                    traci = scenarioManager->getCommandInterface();
                    Veins::TraCICommandInterface::Vehicle v = traci->vehicle(report->getSrc());

                    // Make sure vehicle is in database
                    storeVehicle(report->getSrc(), v);

                    // Make sure lane section is in database
                    storeSection(v.getRoadId(), v.getLaneIndex());

                    // Store LTE report
                    uint64_t simtime_rx = simTime().raw();
                    db->insertLTEReport(report, simtime_rx);

                    // Update statistics
                    ++receivedMessagesViaLte;
                    receivedBytes += report->getByteLength();
                    if(debug) {
                        std::cout << "[ServerApp] Received LTEReport from " <<  report->getSrc() << std::endl;
                    }
                }
            }
        }
    } else if (msg->isName("self") && simTime() > 0) { // Self messages

        storeTraCISnapshot();

        scheduleAt(simTime() + traciLogInterval, new cMessage("self")); //schedule next measurement
    }
    delete msg;
}

/**
 * Get the information of all vehicles in the simulation via TraCI and store it in the database
 */
void ServerApp::storeTraCISnapshot() {

    traci = scenarioManager->getCommandInterface();

    std::list<std::string> allVehicles = traci->getVehicleIds();

    for (std::list<std::string>::const_iterator it = allVehicles.begin(); it != allVehicles.end(); it++) {
        std::string vehicleId = it->c_str();
        Veins::TraCICommandInterface::Vehicle v = traci->vehicle(vehicleId);

        // Make sure vehicle is in database
        storeVehicle(vehicleId, v);

        // Get road information
        std::pair<std::string, int32_t> section(v.getRoadId(), v.getLaneIndex());
        double lanePosition = v.getLanePosition();
        double speed = v.getSpeed();
        Coord pos = v.getPosition();

        // Make sure lane section is in database
        storeSection(section);

        // Store ground truth vehicle information
        uint64_t simtime = simTime().raw();
        db->insertTraCI(vehicleId, section, simtime, speed, lanePosition, pos.x, pos.y);
    }
}

/**
 * Store unique vehicle if not already processed in an earlier iteration
 */
void ServerApp::storeVehicle(const std::string& vehicleId, Veins::TraCICommandInterface::Vehicle& v) {
    const bool isVehicleProcessed = insertedVehicles.find(vehicleId) != insertedVehicles.end();

    if (!isVehicleProcessed) {
        double length = traci->vehicletype(v.getTypeId()).getLength();

        db->insertVehicle(vehicleId, v.getTypeId(), length);
        insertedVehicles.insert(vehicleId);
    }
}


/**
 * Store unique lane section if not already processed in an earlier iteration
 */
void ServerApp::storeSection(const std::string roadId, const int32_t laneIndex) {
    std::pair<std::string, int32_t> section(roadId, laneIndex);
    storeSection(section);
}

/**
 * Store unique lane section if not already processed in an earlier iteration
 */
void ServerApp::storeSection(const std::pair<std::string, int32_t>& section) {
    const bool isLaneProcessed = insertedSections.find(section) != insertedSections.end();

    if (!isLaneProcessed) {
        Veins::TraCICommandInterface::Lane l = traci->lane(section.first + '_' + std::to_string(section.second));
        double laneLength = l.getLength();

        db->insertSection(section, laneLength);
        insertedSections.insert(section);
    }
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

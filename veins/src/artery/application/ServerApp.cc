#include "ServerApp.h"
#include <string.h>
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
        receivedMessagesViaLte = 0;
        receivedMessagesViaDsrc = 0;
        receivedBytes = 0;

        udpIn = findGate("udpIn");
        udpOut = findGate("udpOut");

        //scheduleAt(simTime() + traciLogInterval, new cMessage("self")); // FIXME: when is the right time for initialization?
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

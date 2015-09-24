#include "ServerApp.h"
#include <string.h>

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

        manager = TraCIScenarioManagerAccess().get();
        ASSERT(manager);
        udpIn = findGate("udpIn");
        udpOut = findGate("udpOut");

        //scheduleAt(simTime() + traciLogInterval, new cMessage("self")); // FIXME: when is the right time for initialization?
    }
}

void ServerApp::finish() {
    std::cout << "[ServerApp] Received " << receivedMessagesViaLte
            << " messages via LTE" << std::endl;
    //std::cout << "[ServerApp] Received " << receivedBytes << " bytes via LTE." << std::endl;
    std::cout << "[ServerApp] Received " << receivedMessagesViaDsrc
            << " messages via DSRC." << std::endl;
}

void ServerApp::handleMessageWhenUp(cMessage *msg) {
    if (!msg->isSelfMessage()) {
        int gateId = msg->getArrivalGateId();
        if (gateId == udpIn) {
            HeterogeneousMessage* heterogeneousMessage =
                    dynamic_cast<HeterogeneousMessage*>(msg);

            if (heterogeneousMessage != NULL && simTime() > 0) {
                if (heterogeneousMessage->getNetworkType() == LTE) {
                    ++receivedMessagesViaLte;
                    std::string source =
                            heterogeneousMessage->getSourceAddress();
                    //receivedBytes += heterogeneousMessage->getByteLength();
                    std::cout << "[ServerApp] Received message via LTE."
                            << std::endl;
                }
                if (heterogeneousMessage->getNetworkType() == DSRC) {
                    ++receivedMessagesViaDsrc;
                    std::cout << "[ServerApp] Received message via DSRC."
                            << std::endl;
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

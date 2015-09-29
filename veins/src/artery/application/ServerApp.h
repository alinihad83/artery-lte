#ifndef SERVERAPP_H_
#define SERVERAPP_H_

#include <omnetpp.h>
#include "ApplicationBase.h"
#include "INETDefs.h"
#include "UDPSocket.h"
#include "IPv4Address.h"

/*
 * @brief
 * A simple server that just prints the received messages.
 *
 * @author Julian Timpner
 */

class ServerApp: public ApplicationBase {
protected:
    UDPSocket socket;
    long receivedMessagesViaLte;
    long receivedMessagesViaDsrc;
    long receivedBytes;
    bool debug;
    int udpOut;
    int udpIn;

public:
    ServerApp();
    virtual ~ServerApp();

    virtual int numInitStages() const {
        return 4;
    }
    virtual void initialize(int stage);
    virtual void finish();
    virtual void handleMessageWhenUp(cMessage *msg);

    virtual bool handleNodeStart(IDoneCallback *doneCallback);
    virtual bool handleNodeShutdown(IDoneCallback *doneCallback);
    virtual void handleNodeCrash();
};

#endif /* SERVERAPP_H_ */

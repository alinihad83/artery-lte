package artery.application;

import inet.applications.IUDPApp;

simple ServerApp like IUDPApp
{
    parameters:
        int localPort = default(4242);
        bool debug = default(false);
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}

package artery.application;

// Interface for application layer modules
moduleinterface IBaseLTEApplLayer
{
    parameters:
        int headerLength @unit("bit"); // length of the application message header (in bits)
    gates:
        input lowerLayerIn; // from network layer
        output lowerLayerOut; // to network layer
        input lowerControlIn; // control from network layer
        output lowerControlOut; // control to network layer
        
        input fromLte;
        output toLte;
}


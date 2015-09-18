// @author: Florian Hagenauer
// @date: 25.06.2014
// ArteryLTECar
//
// This car module provides two interfaces for sending and receiving messages via LTE and/or DSRC.

package artery.nodes;

import org.car2x.veins.base.modules.*;
import org.car2x.veins.modules.nic.Nic80211p;

import artery.application.*;
import artery.nic.Nic80211pItsG5;

import org.car2x.veins.modules.lte.*;
import org.car2x.veins.modules.heterogeneous.application.*;
import lte.corenetwork.nodes.*;

import inet.nodes.inet.NetworkLayerNoConfig;
import lte.stack.phy.LteNicUe;
import inet.transport.udp.UDP;
import inet.transport.tcp.TCP;
import inet.applications.ITCPApp;

import inet.networklayer.ipv4.RoutingTable;
import inet.mobility.contract.IMobility;
import inet.base.NotificationBoard;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.autorouting.ipv4.HostAutoConfigurator;

module ArteryLTECar {
    parameters:
        @node();

        string applType;           //type of the application layer
        string veinsmobilityType;  //type of the mobility module
        string mobilityType;       // INET mobility

        // Following Parameters taken from Ue.ned to make LTE work
        string nodeType = "UE";  // DO NOT CHANGE
        int masterId;
        int macNodeId = default(0); // TODO: this is not a real parameter
        int macCellId = default(0); // TODO: this is not a real parameter

        @display("bgb=429,707");

    gates:
        input radioIn @directIn; //LTE input gate

    submodules:
        interfaceTable: InterfaceTable {
            @display("p=375,270;is=s");
        }

        mobility: <mobilityType> like IMobility {
            @display("p=375,370;is=s");
        }

        notificationBoard: NotificationBoard {
            @display("p=375,220;is=s");
        }

        routingTable: RoutingTable {
            @display("p=375,320;is=s");
        }

        nic80211p: Nic80211pItsG5 {
            parameters:
                @display("p=157,501");
        }

        veinsmobility: <veinsmobilityType> like org.car2x.veins.base.modules.IMobility {
            parameters:
                @display("p=375,56;i=block/cogwheel");
        }

        appl: <applType> like artery.application.IBaseLTEApplLayer {
            parameters:
                @display("p=196,140;i=block/fork");
        }

        networkLayer: NetworkLayerNoConfig {
            @display("p=235,420");
        }

        // NOTE: instance must be named "nic", therefore the other nic is called nic80211p
        nic: LteNicUe {
            nodeType = nodeType;
            @display("p=235,501");
        }

        UDP: UDP {
            @display("p=235,340");
        }

        configurator: HostAutoConfigurator {
            @display("p=171,335");
        }

    connections allowunconnected:

        nic80211p.upperLayerOut --> appl.lowerLayerIn;
        nic80211p.upperLayerIn <-- appl.lowerLayerOut;

        nic80211p.upperControlOut --> appl.lowerControlIn;
        nic80211p.upperControlIn <-- appl.lowerControlOut;

        appl.toLte --> UDP.appIn++;
        UDP.ipOut --> networkLayer.transportIn++;

        appl.fromLte <-- UDP.appOut++;
        UDP.ipIn <-- networkLayer.transportOut++;

        networkLayer.ifOut++ --> nic.upperLayerIn;
        networkLayer.ifIn++ <-- nic.upperLayerOut;

        radioIn --> nic.radioIn;
}

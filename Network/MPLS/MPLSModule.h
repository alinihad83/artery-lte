/*******************************************************************
*
*    This library is free software, you can redistribute it
*    and/or modify
*    it under  the terms of the GNU Lesser General Public License
*    as published by the Free Software Foundation;
*    either version 2 of the License, or any later version.
*    The library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*    See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/

#ifndef __MPLSMODULE_H__

#define __MPLSMODULE_H__

#include <vector>
#include <omnetpp.h>
#include "MPLSPacket.h"
#include "IPDatagram.h"
#include "LIBTableAccess.h"
#include "RoutingTableAccess.h"
#include "RoutingTable.h"
#include "ConstType.h"


#define DEST_CLASSIFIER         1
#define SRC_AND_DEST_CLASSIFIER 2

#define MAX_LSP_NO            25

#define PUSH_OPER              0
#define SWAP_OPER              1
#define POP_OPER               2



/**
 * Implements MPLS.
 */
class MPLSModule : public cSimpleModule
{
   private:
     /**
      * Element in the FEC (Forwarding Equivalence Class) table.
      * SrcAddr and destAddr are criteria for the IP packet used in classification,
      * and fecId is the resulting FEC.
      */
     struct FECElem
     {
         int fecId;
         IPAddress srcAddr;
         IPAddress destAddr;
     };

   private:
      RoutingTableAccess routingTableAccess;
      LIBTableAccess libTableAccess;

      bool isIR;
      bool isER;
      bool isSignallingReady;
      int  classifierType;
      simtime_t delay1;

      std::vector<FECElem> fecList;

      cArray ipdataQueue;  // Queue of packets need label from LDP -- FIXME should be cQueue!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      cQueue ldpQueue;   // Queue of queries to LDP when the LDP is not ready
      int maxFecId;

   public:
      Module_Class_Members(MPLSModule,cSimpleModule,0);

      /**
       * Initialize the module parameters
       */
      virtual void initialize();

      /** Utility: dumps FEC table */
      void dumpFECTable();

      /** @name Message handling routines */
      //@{
      virtual void handleMessage(cMessage *msg);

      virtual void processPacketFromL3(cMessage *msg);
      virtual void processPacketFromSignalling(cMessage *msg);
      virtual void processPacketFromL2(cMessage *msg);

      virtual void processMPLSPacketFromL2(MPLSPacket *mplsPacket);
      virtual void processIPDatagramFromL2(IPDatagram *ipdatagram);

      /** Invoked from processPacketFromSignalling() */
      virtual void trySendBufferedPackets(int returnedFEC);

      virtual void requestLDP(int fecID, IPAddress src, IPAddress dest, int gateindex);

      //@}

      /**
       * Classify FEC for packet. If FEC already exists, return its fecId.
       * If no existing FEC is found: add a new one, and return -1.
       *
       * @param type Scheme of classification, i.g destination-based or
       * destination and sender-based (DEST_CLASSIFIER, SRC_AND_DEST_CLASSIFIER).
       */
      int classifyPacket(IPDatagram *ipdata, int type);

};



#endif


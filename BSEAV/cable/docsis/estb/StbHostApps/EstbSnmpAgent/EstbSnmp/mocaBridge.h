//****************************************************************************
//
// Copyright (c) 2009-2012 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized License, Broadcom grants no license
// (express or implied), right to use, or waiver of any kind with respect to
// the Software, and Broadcom expressly reserves all rights in and to the
// Software and all intellectual property rights therein.  IF YOU HAVE NO
// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
// SOFTWARE.  
//
// Except as expressly set forth in the Authorized License,
//
// 1.     This program, including its structure, sequence and organization,
// constitutes the valuable trade secrets of Broadcom, and you shall use all
// reasonable efforts to protect the confidentiality thereof, and to use this
// information only in connection with your use of Broadcom integrated circuit
// products.
//
// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//****************************************************************************
//    Description:                                                          
//                                                                          
//****************************************************************************
//    Revision History:                                                     
//                                                                          
//****************************************************************************
//    Filename: mocaBridge.h
//    Creation Date: Friday, October 16, 2009 at 10:25:11
//    Created by Broadcom BFC Mib Compiler BCMIBC.EXE version 3.6.1b
//    from input file MOCA11-MIB.mib
//
//****************************************************************************

#ifndef MOCABRIDGE_H
#define MOCABRIDGE_H


#include "CoreObjs.h"
#include "MibObjs.h"
#include "MibBridge.h"

#include "mocaMib.h"

extern "C" {
#include "devctl_moca.h"
}


/*\
 *  mocaIfConfigEntry 
\*/

#define CONSTVAL_MOCAIFENABLE_TRUE  1
#define CONSTVAL_MOCAIFENABLE_FALSE  2
#define CONSTVAL_MOCAIFPOWERCONTROL_TRUE  1
#define CONSTVAL_MOCAIFPOWERCONTROL_FALSE  2
#define MINVAL_MOCAIFTXPOWERLIMIT  0
#define MAXVAL_MOCAIFTXPOWERLIMIT  127
#define MINVAL_MOCAIFBEACONPOWERLIMIT  0
#define MAXVAL_MOCAIFBEACONPOWERLIMIT  127
#define MINVAL_MOCAIFPOWERCONTROLTARGETRATE  0
#define MAXVAL_MOCAIFPOWERCONTROLTARGETRATE  1024
#define CONSTVAL_MOCAIFPRIVACYENABLE_TRUE  1
#define CONSTVAL_MOCAIFPRIVACYENABLE_FALSE  2
#define MINLEN_MOCAIFPASSWORD  0
#define MAXLEN_MOCAIFPASSWORD  17
#define CONSTVAL_MOCAIFPREFERREDNC_TRUE  1
#define CONSTVAL_MOCAIFPREFERREDNC_FALSE  2
#define CONSTVAL_MOCAIFACCESSENABLE_TRUE  1
#define CONSTVAL_MOCAIFACCESSENABLE_FALSE  2
#define MINVAL_MOCAIFPHYTHRESHOLD  0
#define MAXVAL_MOCAIFPHYTHRESHOLD  1024
#define CONSTVAL_MOCAIFPHYTHRESHOLDENABLE_TRUE  1
#define CONSTVAL_MOCAIFPHYTHRESHOLDENABLE_FALSE  2
#define CONSTVAL_MOCAIFSTATUSCHANGEENABLE_TRUE  1
#define CONSTVAL_MOCAIFSTATUSCHANGEENABLE_FALSE  2
#define CONSTVAL_MOCAIFNUMNODESCHANGEENABLE_TRUE  1
#define CONSTVAL_MOCAIFNUMNODESCHANGEENABLE_FALSE  2

class mocaIfConfigEntryBridge : public TabularMibBridge
{
  friend class mocaIfConfigEntry;

  public:
    mocaIfConfigEntryBridge (int ifIndex, struct moca_init_time *mocaInitTime, 
                             BcmSnmpAgent *pAgent);
   ~mocaIfConfigEntryBridge ();

    /* GET methods */

    bool Get_mocaIfEnable ();
    unsigned int Get_mocaIfChannelMask ();
    bool Get_mocaIfPowerControl ();
    unsigned int Get_mocaIfTxPowerLimit ();
    unsigned int Get_mocaIfBeaconPowerLimit ();
    unsigned int Get_mocaIfPowerControlTargetRate ();
    bool Get_mocaIfPrivacyEnable ();
    int Get_mocaIfPassword (BcmString &mocaIfPassword);
    bool Get_mocaIfPreferredNC ();
    bool Get_mocaIfAccessEnable ();
    unsigned int Get_mocaIfPhyThreshold ();
    bool Get_mocaIfPhyThresholdEnable ();
    bool Get_mocaIfStatusChangeEnable ();
    bool Get_mocaIfNumNodesChangeEnable ();

    /* TEST methods - not required */


    /* SET methods */

    SNMP_STATUS Set_mocaIfEnable (bool mocaIfEnable);
    SNMP_STATUS Set_mocaIfChannelMask (unsigned int mocaIfChannelMask);
    SNMP_STATUS Set_mocaIfPowerControl (bool mocaIfPowerControl);
    SNMP_STATUS Set_mocaIfTxPowerLimit (unsigned int mocaIfTxPowerLimit);
    SNMP_STATUS Set_mocaIfBeaconPowerLimit (unsigned int mocaIfBeaconPowerLimit);
    SNMP_STATUS Set_mocaIfPowerControlTargetRate (unsigned int mocaIfPowerControlTargetRate);
    SNMP_STATUS Set_mocaIfPrivacyEnable (bool mocaIfPrivacyEnable);
    SNMP_STATUS Set_mocaIfPassword (const BcmString &mocaIfPassword);
    SNMP_STATUS Set_mocaIfPreferredNC (bool mocaIfPreferredNC);
    SNMP_STATUS Set_mocaIfAccessEnable (bool mocaIfAccessEnable);
    SNMP_STATUS Set_mocaIfPhyThreshold (unsigned int mocaIfPhyThreshold);
    SNMP_STATUS Set_mocaIfPhyThresholdEnable (bool mocaIfPhyThresholdEnable);

  private:
    bool lMocaIfEnable;
    unsigned int lMocaIfChannelMask;
    bool lMocaIfPowerControl;
    unsigned int lMocaIfTxPowerLimit;
    unsigned int lMocaIfBeaconPowerLimit;
    unsigned int lMocaIfPowerControlTargetRate;
    bool lMocaIfPrivacyEnable;
    BcmString lMocaIfPassword;
    bool lMocaIfPreferredNC;
    bool lMocaIfAccessEnable;
    unsigned int lMocaIfPhyThreshold;
    bool lMocaIfPhyThresholdEnable;
    bool lMocaIfStatusChangeEnable;
    bool lMocaIfNumNodesChangeEnable;

};



/*\
 *  mocaIfAccessEntry 
\*/

#define MINVAL_MOCAIFACCESSINDEX  1
#define MAXVAL_MOCAIFACCESSINDEX  2147483647
#define CONSTVAL_MOCAIFACCESSSTATUS_ACTIVE  1
#define CONSTVAL_MOCAIFACCESSSTATUS_NOTINSERVICE  2
#define CONSTVAL_MOCAIFACCESSSTATUS_NOTREADY  3
#define CONSTVAL_MOCAIFACCESSSTATUS_CREATEANDGO  4
#define CONSTVAL_MOCAIFACCESSSTATUS_CREATEANDWAIT  5
#define CONSTVAL_MOCAIFACCESSSTATUS_DESTROY  6

class mocaIfAccessEntryBridge : public TabularMibBridge
{
  friend class mocaIfAccessEntry;

  public:
    mocaIfAccessEntryBridge (int mocaIfAccessIndex, BcmSnmpAgent *pAgent);
    mocaIfAccessEntryBridge (const BcmObjectId &Index, BcmSnmpAgent *pAgent);
   ~mocaIfAccessEntryBridge ();

    /* GET methods */

    int Get_mocaIfAccessIndex ();
    void Get_mocaIfAccessMacAddress (BcmMacAddress &mocaIfAccessMacAddress);
    int Get_mocaIfAccessStatus ();

    /* TEST methods */

    SNMP_STATUS Test_mocaIfAccessMacAddress (const BcmMacAddress &mocaIfAccessMacAddress);
    SNMP_STATUS Test_mocaIfAccessStatus (int mocaIfAccessStatus);

    /* SET methods */

    SNMP_STATUS Set_mocaIfAccessMacAddress (const BcmMacAddress &mocaIfAccessMacAddress);
    SNMP_STATUS Set_mocaIfAccessStatus (int &mocaIfAccessStatus);

  protected:
    int fRowStatus;

    enum
    {
      kBIT_mocaIfAccessIndex = 1 << (mocaIfAccessEntry::kLEAF_mocaIfAccessIndex - 1),
      kBIT_mocaIfAccessMacAddress = 1 << (mocaIfAccessEntry::kLEAF_mocaIfAccessMacAddress - 1),
      kBIT_mocaIfAccessStatus = 1 << (mocaIfAccessEntry::kLEAF_mocaIfAccessStatus - 1),
    };
};



/*\
 *  mocaIfStatusEntry 
\*/

#define CONSTVAL_MOCAIFSTATUS_DISABLED  1
#define CONSTVAL_MOCAIFSTATUS_NOLINK  2
#define CONSTVAL_MOCAIFSTATUS_LINKUP  3
#define MINLEN_MOCAIFSOFTWAREVERSION  0
#define MAXLEN_MOCAIFSOFTWAREVERSION  80
#define CONSTVAL_MOCAIFMOCAVERSION_MOCA1DOT0  10
#define CONSTVAL_MOCAIFMOCAVERSION_MOCA1DOT1  11
#define CONSTVAL_MOCAIFMOCAVERSION_MOCA1DOT1PROTEM  12
#define CONSTVAL_MOCAIFNETWORKVERSION_MOCA1DOT0  10
#define CONSTVAL_MOCAIFNETWORKVERSION_MOCA1DOT1  11
#define CONSTVAL_MOCAIFNETWORKVERSION_MOCA1DOT1PROTEM  12
#define MINVAL_MOCAIFNODEID  0
#define MAXVAL_MOCAIFNODEID  15
#define MINLEN_MOCAIFNAME  0
#define MAXLEN_MOCAIFNAME  16
#define MINVAL_MOCAIFNUMNODES  0
#define MAXVAL_MOCAIFNUMNODES  16
#define MINVAL_MOCAIFNC  0
#define MAXVAL_MOCAIFNC  15
#define MINVAL_MOCAIFBACKUPNC  0
#define MAXVAL_MOCAIFBACKUPNC  15
#define CONSTVAL_MOCAIFRFCHANNEL_UNKNOWN  0
#define CONSTVAL_MOCAIFRFCHANNEL_A1  875
#define CONSTVAL_MOCAIFRFCHANNEL_B1  900
#define CONSTVAL_MOCAIFRFCHANNEL_C1  925
#define CONSTVAL_MOCAIFRFCHANNEL_C2  950
#define CONSTVAL_MOCAIFRFCHANNEL_C3  975
#define CONSTVAL_MOCAIFRFCHANNEL_C4  1000
#define CONSTVAL_MOCAIFRFCHANNEL_D1  1150
#define CONSTVAL_MOCAIFRFCHANNEL_D2  1200
#define CONSTVAL_MOCAIFRFCHANNEL_D3  1250
#define CONSTVAL_MOCAIFRFCHANNEL_D4  1300
#define CONSTVAL_MOCAIFRFCHANNEL_D5  1350
#define CONSTVAL_MOCAIFRFCHANNEL_D6  1400
#define CONSTVAL_MOCAIFRFCHANNEL_D7  1450
#define CONSTVAL_MOCAIFRFCHANNEL_D8  1500
#define CONSTVAL_MOCAIFLOF_UNKNOWN  0
#define CONSTVAL_MOCAIFLOF_A1  875
#define CONSTVAL_MOCAIFLOF_B1  900
#define CONSTVAL_MOCAIFLOF_C1  925
#define CONSTVAL_MOCAIFLOF_C2  950
#define CONSTVAL_MOCAIFLOF_C3  975
#define CONSTVAL_MOCAIFLOF_C4  1000
#define CONSTVAL_MOCAIFLOF_D1  1150
#define CONSTVAL_MOCAIFLOF_D2  1200
#define CONSTVAL_MOCAIFLOF_D3  1250
#define CONSTVAL_MOCAIFLOF_D4  1300
#define CONSTVAL_MOCAIFLOF_D5  1350
#define CONSTVAL_MOCAIFLOF_D6  1400
#define CONSTVAL_MOCAIFLOF_D7  1450
#define CONSTVAL_MOCAIFLOF_D8  1500
#define MINVAL_MOCAIFTXGCDPOWERREDUCTION  0
#define MAXVAL_MOCAIFTXGCDPOWERREDUCTION  127
#define CONSTVAL_MOCAIFQAM256CAPABLE_TRUE  1
#define CONSTVAL_MOCAIFQAM256CAPABLE_FALSE  2
#define CONSTVAL_MOCAIFPACKETSAGGRCAPABILITY_NONE  0
#define CONSTVAL_MOCAIFPACKETSAGGRCAPABILITY_AGGR6  6
#define CONSTVAL_MOCAIFPACKETSAGGRCAPABILITY_AGGR10  10
#define MINVAL_MOCAIFMAXINGRESSNODEBW  0
#define MAXVAL_MOCAIFMAXINGRESSNODEBW  1024
#define MINVAL_MOCAIFMAXEGRESSNODEBW  0
#define MAXVAL_MOCAIFMAXEGRESSNODEBW  1024
#define MINVAL_MOCAIFTXGCDRATE  0
#define MAXVAL_MOCAIFTXGCDRATE  1024

class mocaIfStatusEntryBridge : public TabularMibBridge
{
  friend class mocaIfStatusEntry;

  public:
    mocaIfStatusEntryBridge (int ifIndex, void *mocaHandle, 
                             BcmSnmpAgent *pAgent);
   ~mocaIfStatusEntryBridge ();

    inline void Assign_mocaIfMaxIngressNodeBw (int bw);
    inline void Assign_mocaIfMaxEgressNodeBw (int bw);


    /* GET methods */

    int Get_mocaIfStatus ();
    unsigned int Get_mocaIfLinkUpTime ();
    int Get_mocaIfSoftwareVersion (BcmString &mocaIfSoftwareVersion);
    int Get_mocaIfMocaVersion ();
    int Get_mocaIfNetworkVersion ();
    void Get_mocaIfMacAddress (BcmMacAddress &mocaIfMacAddress);
    unsigned int Get_mocaIfNodeID ();
    int Get_mocaIfName (BcmString &mocaIfName);
    unsigned int Get_mocaIfNumNodes ();
    unsigned int Get_mocaIfNC ();
    unsigned int Get_mocaIfBackupNC ();
    int Get_mocaIfRFChannel ();
    int Get_mocaIfLOF ();
    unsigned int Get_mocaIfTabooChannelMask ();
    unsigned int Get_mocaIfNodeTabooChannelMask ();
    unsigned int Get_mocaIfCapabilityMask ();
    unsigned int Get_mocaIfTxGcdPowerReduction ();
    bool Get_mocaIfQAM256Capable ();
    int Get_mocaIfPacketsAggrCapability ();
    unsigned int Get_mocaIfMaxIngressNodeBw ();
    unsigned int Get_mocaIfMaxEgressNodeBw ();
    unsigned int Get_mocaIfTxGcdRate ();
    void *MocaHandle;

    /* TEST methods - not required */


    /* SET methods */


  private:

    int lMocaIfStatus;
    unsigned int lMocaIfLinkUpTime;
    BcmString lMocaIfSoftwareVersion;
    int lMocaIfMocaVersion;
    int lMocaIfNetworkVersion;
    BcmMacAddress lMocaIfMacAddress;
    unsigned int lMocaIfNodeID;
    BcmString lMocaIfName;
    unsigned int lMocaIfNumNodes;
    unsigned int lMocaIfNC;
    unsigned int lMocaIfBackupNC;
    int lMocaIfRFChannel;
    int lMocaIfLOF;
    unsigned int lMocaIfTabooChannelMask;
    unsigned int lMocaIfNodeTabooChannelMask;
    unsigned int lMocaIfCapabilityMask;
    unsigned int lMocaIfTxGcdPowerReduction;
    bool lMocaIfQAM256Capable;
    int lMocaIfPacketsAggrCapability;
    unsigned int lMocaIfMaxIngressNodeBw;
    unsigned int lMocaIfMaxEgressNodeBw;
    unsigned int lMocaIfTxGcdRate;
};

inline void mocaIfStatusEntryBridge::Assign_mocaIfMaxIngressNodeBw (int bw) { lMocaIfMaxIngressNodeBw = bw;} ;
inline void mocaIfStatusEntryBridge::Assign_mocaIfMaxEgressNodeBw (int bw) { lMocaIfMaxEgressNodeBw = bw;} ;



/*\
 *  mocaIfStatsEntry 
\*/


class mocaIfStatsEntryBridge : public TabularMibBridge
{
  friend class mocaIfStatsEntry;

  public:
    mocaIfStatsEntryBridge (int ifIndex, void *mocaHandle, 
                            BcmSnmpAgent *pAgent);
   ~mocaIfStatsEntryBridge ();

    /* GET methods */

    unsigned int Get_mocaIfTxPackets ();
    unsigned int Get_mocaIfTxDrops ();
    unsigned int Get_mocaIfRxPackets ();
    unsigned int Get_mocaIfRxCorrectedErrors ();
    unsigned int Get_mocaIfRxDrops ();
    unsigned int Get_mocaIfEgressNodeNumFlows ();
    unsigned int Get_mocaIfIngressNodeNumFlows ();

    /* TEST methods - not required */


    /* SET methods */

  private:
    unsigned int lMocaIfTxPackets;
    unsigned int lMocaIfTxDrops;
    unsigned int lMocaIfRxPackets;
    unsigned int lMocaIfRxCorrectedErrors;
    unsigned int lMocaIfRxDrops;
    unsigned int lMocaIfEgressNodeNumFlows;
    unsigned int lMocaIfIngressNodeNumFlows;

};



/*\
 *  mocaIfFlowStatsEntry 
\*/

#define MINVAL_MOCAIFFLOWINDEX  1
#define MAXVAL_MOCAIFFLOWINDEX  2147483647

class mocaIfFlowStatsEntryBridge : public TabularMibBridge
{
  friend class mocaIfFlowStatsEntry;

  public:
    mocaIfFlowStatsEntryBridge (int ifIndex, int mocaIfFlowIndex, void *mocaHandle, 
                                struct moca_pqos_query_response *query_rsp, 
                                BcmSnmpAgent *pAgent);
   ~mocaIfFlowStatsEntryBridge ();

    /* GET methods */

    int Get_mocaIfFlowIndex ();
    void Get_mocaIfFlowID (BcmMacAddress &mocaIfFlowID);
    void Get_mocaIfPacketDA (BcmMacAddress &mocaIfPacketDA);
    unsigned int Get_mocaIfPeakDataRate ();
    unsigned int Get_mocaIfBurstSize ();
    unsigned int Get_mocaIfLeaseTime ();
    unsigned int Get_mocaIfFlowTag ();
    unsigned int Get_mocaIfLeaseTimeLeft ();
    unsigned int Get_mocaIfTxPacketsOnePacketAggr ();
    unsigned int Get_mocaIfTxPacketsTwoPacketsAggr ();
    unsigned int Get_mocaIfTxPacketsThreePacketsAggr ();
    unsigned int Get_mocaIfTxPacketsFourPacketsAggr ();
    unsigned int Get_mocaIfTxPacketsFivePacketsAggr ();
    unsigned int Get_mocaIfTxPacketsSixPacketsAggr ();
    unsigned int Get_mocaIfTxPacketsSevenPacketsAggr ();
    unsigned int Get_mocaIfTxPacketsEightPacketsAggr ();
    unsigned int Get_mocaIfTxPacketsNinePacketsAggr ();
    unsigned int Get_mocaIfTxPacketsTenPacketsAggr ();
    unsigned int Get_mocaIfTxPacketsFlow ();

    /* TEST methods - not required */


    /* SET methods */

  private:
    int lIfIndex;
    int lMocaIfFlowIndex;
    BcmMacAddress lMocaIfFlowID;
    BcmMacAddress lMocaIfPacketDA;
    unsigned int lMocaIfPeakDataRate;
    unsigned int lMocaIfBurstSize;
    unsigned int lMocaIfLeaseTime;
    unsigned int lMocaIfFlowTag;
    unsigned int lMocaIfLeaseTimeLeft;
    unsigned int lMocaIfTxPacketsOnePacketAggr;
    unsigned int lMocaIfTxPacketsTwoPacketsAggr;
    unsigned int lMocaIfTxPacketsThreePacketsAggr;
    unsigned int lMocaIfTxPacketsFourPacketsAggr;
    unsigned int lMocaIfTxPacketsFivePacketsAggr;
    unsigned int lMocaIfTxPacketsSixPacketsAggr;
    unsigned int lMocaIfTxPacketsSevenPacketsAggr;
    unsigned int lMocaIfTxPacketsEightPacketsAggr;
    unsigned int lMocaIfTxPacketsNinePacketsAggr;
    unsigned int lMocaIfTxPacketsTenPacketsAggr;
    unsigned int lMocaIfTxPacketsFlow;


};



/*\
 *  mocaNodeEntry 
\*/

#define MINVAL_MOCANODEINDEX  0
#define MAXVAL_MOCANODEINDEX  15
#define CONSTVAL_MOCANODEMOCAVERSION_MOCA1DOT0  10
#define CONSTVAL_MOCANODEMOCAVERSION_MOCA1DOT1  11
#define CONSTVAL_MOCANODEMOCAVERSION_MOCA1DOT1PROTEM  12
#define MINVAL_MOCANODETXGCDRATE  0
#define MAXVAL_MOCANODETXGCDRATE  1024
#define MINVAL_MOCANODERXGCDPOWER  -100
#define MAXVAL_MOCANODERXGCDPOWER  27
#define MINVAL_MOCANODETXPOWERREDUCTION  0
#define MAXVAL_MOCANODETXPOWERREDUCTION  127
#define MINVAL_MOCANODERXPOWER  -100
#define MAXVAL_MOCANODERXPOWER  27
#define CONSTVAL_MOCANODEPREFERREDNC_TRUE  1
#define CONSTVAL_MOCANODEPREFERREDNC_FALSE  2
#define CONSTVAL_MOCANODEQAM256CAPABLE_TRUE  1
#define CONSTVAL_MOCANODEQAM256CAPABLE_FALSE  2
#define CONSTVAL_MOCANODEPACKETSAGGRCAPABILITY_NONE  0
#define CONSTVAL_MOCANODEPACKETSAGGRCAPABILITY_AGGR6  6
#define CONSTVAL_MOCANODEPACKETSAGGRCAPABILITY_AGGR10  10
#define MINVAL_MOCANODESNR  0
#define MAXVAL_MOCANODESNR  127

class mocaNodeEntryBridge : public TabularMibBridge
{
  friend class mocaNodeEntry;

  public:
    mocaNodeEntryBridge (int ifIndex, unsigned long mocaNodeIndex, 
                         void *mocaHandle, BcmSnmpAgent *pAgent);
   ~mocaNodeEntryBridge ();

    /* GET methods */

    unsigned int Get_mocaNodeIndex ();
    int Get_mocaNodeMocaVersion ();
    void Get_mocaNodeMacAddress (BcmMacAddress &mocaNodeMacAddress);
    unsigned int Get_mocaNodeTxGcdRate ();
    int Get_mocaNodeRxGcdPower ();
    unsigned int Get_mocaNodeTxPowerReduction ();
    int Get_mocaNodeRxPower ();
    bool Get_mocaNodePreferredNC ();
    bool Get_mocaNodeQAM256Capable ();
    int Get_mocaNodePacketsAggrCapability ();
    unsigned int Get_mocaNodeRxPackets ();
    unsigned int Get_mocaNodeRxCorrected ();
    unsigned int Get_mocaNodeRxDrops ();
    unsigned int Get_mocaNodeSNR ();

    /* TEST methods - not required */


    /* SET methods */

  private:
    unsigned int lMocaNodeIndex;
    int lMocaNodeMocaVersion;
    BcmMacAddress lMocaNodeMacAddress;
    unsigned int lMocaNodeTxGcdRate;
    int lMocaNodeRxGcdPower;
    unsigned int lMocaNodeTxPowerReduction;
    int lMocaNodeRxPower;
    bool lMocaNodePreferredNC;
    bool lMocaNodeQAM256Capable;
    int lMocaNodePacketsAggrCapability;
    unsigned int lMocaNodeRxPackets;
    unsigned int lMocaNodeRxCorrected;
    unsigned int lMocaNodeRxDrops;
    unsigned int lMocaNodeSNR;


};



/*\
 *  mocaMeshEntry 
\*/

#define MINVAL_MOCAMESHTXNODEINDEX  0
#define MAXVAL_MOCAMESHTXNODEINDEX  15
#define MINVAL_MOCAMESHRXNODEINDEX  0
#define MAXVAL_MOCAMESHRXNODEINDEX  15
#define MINVAL_MOCAMESHTXRATE  0
#define MAXVAL_MOCAMESHTXRATE  1024

class mocaMeshEntryBridge : public TabularMibBridge
{
  friend class mocaMeshEntry;

  public:
    mocaMeshEntryBridge (int ifIndex, unsigned long mocaMeshTxNodeIndex, 
                         unsigned long mocaMeshRxNodeIndex, void *mocaHandle, 
                         BcmSnmpAgent *pAgent);
   ~mocaMeshEntryBridge ();

    /* GET methods */

    unsigned int Get_mocaMeshTxNodeIndex ();
    unsigned int Get_mocaMeshRxNodeIndex ();
    unsigned int Get_mocaMeshTxRate ();

    /* TEST methods - not required */


    /* SET methods */

  private:
    unsigned int lMocaMeshTxNodeIndex;
    unsigned int lMocaMeshRxNodeIndex;
    unsigned int lMocaMeshTxRate;


};



/*\
 *  mocaBridgeEntry 
\*/

#define MINVAL_MOCABRIDGENODEINDEX  0
#define MAXVAL_MOCABRIDGENODEINDEX  15
#define MINVAL_MOCABRIDGEMACINDEX  1
#define MAXVAL_MOCABRIDGEMACINDEX  2147483647

class mocaBridgeEntryBridge : public TabularMibBridge
{
  friend class mocaBridgeEntry;

  public:
    mocaBridgeEntryBridge (int ifIndex, unsigned long mocaBridgeNodeIndex, int mocaBridgeMacIndex, BcmSnmpAgent *pAgent);
   ~mocaBridgeEntryBridge ();

    /* GET methods */

    unsigned int Get_mocaBridgeNodeIndex ();
    int Get_mocaBridgeMacIndex ();
    void Get_mocaBridgeMacAddress (BcmMacAddress &mocaBridgeMacAddress);

    /* TEST methods - not required */


    /* SET methods */


};



#endif



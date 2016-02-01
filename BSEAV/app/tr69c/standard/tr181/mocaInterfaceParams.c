/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
*  ANY LIMITED REMEDY.
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
***************************************************************************/

#include "sharedparams.h"
#include "mocaInterfaceParams.h"

/* Device.Moca.Interface.{i}.AssociatedDevice.{i}. */

TRXGFUNC(getMocaInterfaceAssociatedDeviceMACAddress);
TRXGFUNC(getMocaInterfaceAssociatedDeviceNodeID);
TRXGFUNC(getMocaInterfaceAssociatedDevicePreferredNC);
TRXGFUNC(getMocaInterfaceAssociatedDeviceHighestVersion);
TRXGFUNC(getMocaInterfaceAssociatedDevicePHYTxRate);
TRXGFUNC(getMocaInterfaceAssociatedDevicePHYRxRate);
TRXGFUNC(getMocaInterfaceAssociatedDeviceTxPowerControlReduction);
TRXGFUNC(getMocaInterfaceAssociatedDeviceRxPowerLevel);
TRXGFUNC(getMocaInterfaceAssociatedDeviceTxBcastRate);
TRXGFUNC(getMocaInterfaceAssociatedDeviceRxBcastPowerLevel);
TRXGFUNC(getMocaInterfaceAssociatedDeviceTxPackets);
TRXGFUNC(getMocaInterfaceAssociatedDeviceRxPackets);
TRXGFUNC(getMocaInterfaceAssociatedDeviceRxErroredAndMissedPackets);
TRXGFUNC(getMocaInterfaceAssociatedDeviceQAM256Capable);
TRXGFUNC(getMocaInterfaceAssociatedDevicePacketAggregationCapability);
TRXGFUNC(getMocaInterfaceAssociatedDeviceRxSNR);
/* TRXGFUNC(getMocaInterfaceAssociatedDeviceActive); */ /* NOT SUPPORT */

TRxObjNode  mocaInterfaceAssociatedDeviceInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {MACAddress, {{tString, 32, 0}}, NULL, getMocaInterfaceAssociatedDeviceMACAddress, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {NodeID, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceAssociatedDeviceNodeID, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {PreferredNC, {{tBool, 0, 0}}, NULL, getMocaInterfaceAssociatedDevicePreferredNC, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {HighestVersion, {{tString, 64, 0}}, NULL, getMocaInterfaceAssociatedDeviceHighestVersion, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {PHYTxRate, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceAssociatedDevicePHYTxRate, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {PHYRxRate, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceAssociatedDevicePHYRxRate, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {TxPowerControlReduction, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceAssociatedDeviceTxPowerControlReduction, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {RxPowerLevel, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceAssociatedDeviceRxPowerLevel, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {TxBcastRate, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceAssociatedDeviceTxBcastRate, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {RxBcastPowerLevel, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceAssociatedDeviceRxBcastPowerLevel, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {TxPackets, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceAssociatedDeviceTxPackets, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {RxPackets, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceAssociatedDeviceRxPackets, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {RxErroredAndMissedPackets, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceAssociatedDeviceRxErroredAndMissedPackets, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {QAM256Capable, {{tBool, 0, 0}}, NULL, getMocaInterfaceAssociatedDeviceQAM256Capable, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {PacketAggregationCapability, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceAssociatedDevicePacketAggregationCapability, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {RxSNR, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceAssociatedDeviceRxSNR, NULL, NULL, 2, 0, 0, 0, NULL, true},
    /* {Active, {{tBool, 0, 0}}, NULL, getMocaInterfaceAssociatedDeviceActive, NULL, NULL, 2, 0, 0, 0, NULL, fasle}, */ /* NOT SUPPORT */
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{MACAddress, {{tString, 32, 0}}, NULL, getMocaInterfaceAssociatedDeviceMACAddress, NULL, NULL},
    {NodeID, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceAssociatedDeviceNodeID, NULL, NULL},
    {PreferredNC, {{tBool, 0, 0}}, NULL, getMocaInterfaceAssociatedDevicePreferredNC, NULL, NULL},
    {HighestVersion, {{tString, 64, 0}}, NULL, getMocaInterfaceAssociatedDeviceHighestVersion, NULL, NULL},
    {PHYTxRate, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceAssociatedDevicePHYTxRate, NULL, NULL},
    {PHYRxRate, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceAssociatedDevicePHYRxRate, NULL, NULL},
    {TxPowerControlReduction, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceAssociatedDeviceTxPowerControlReduction, NULL, NULL},
    {RxPowerLevel, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceAssociatedDeviceRxPowerLevel, NULL, NULL},
    {TxBcastRate, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceAssociatedDeviceTxBcastRate, NULL, NULL},
    {RxBcastPowerLevel, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceAssociatedDeviceRxBcastPowerLevel, NULL, NULL},
    {TxPackets, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceAssociatedDeviceTxPackets, NULL, NULL},
    {RxPackets, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceAssociatedDeviceRxPackets, NULL, NULL},
    {RxErroredAndMissedPackets, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceAssociatedDeviceRxErroredAndMissedPackets, NULL, NULL},
    {QAM256Capable, {{tBool, 0, 0}}, NULL, getMocaInterfaceAssociatedDeviceQAM256Capable, NULL, NULL},
    {PacketAggregationCapability, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceAssociatedDevicePacketAggregationCapability, NULL, NULL},
    {RxSNR, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceAssociatedDeviceRxSNR, NULL, NULL},
    /* {Active, {{tBool, 0, 0}}, NULL, getMocaInterfaceAssociatedDeviceActive, NULL, NULL}, */ /* NOT SUPPORT */
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

/* Device.Moca.Interface.{i}.AssociatedDevice. */

TRXGFUNC(addMocaInterfaceAssociatedDeviceInstance);
TRXSFUNC(deleteMocaInterfaceAssociatedDeviceInstance);

TRxObjNode  mocaInterfaceAssociatedDeviceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, deleteMocaInterfaceAssociatedDeviceInstance, addMocaInterfaceAssociatedDeviceInstance, mocaInterfaceAssociatedDeviceInstanceDesc, NULL, 2, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {instanceIDMASK,{{0, 0, 0}}, deleteMocaInterfaceAssociatedDeviceInstance, addMocaInterfaceAssociatedDeviceInstance, mocaInterfaceAssociatedDeviceInstanceDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

/* Device.Moca.Interface.{i}.QoS.FlowStats.{i}. */

TRXGFUNC(getMocaInterfaceQoSFlowStatsFlowID);
TRXGFUNC(getMocaInterfaceQoSFlowStatsPacketDA);
TRXGFUNC(getMocaInterfaceQoSFlowStatsMaxRate);
TRXGFUNC(getMocaInterfaceQoSFlowStatsMaxBurstSize);
TRXGFUNC(getMocaInterfaceQoSFlowStatsLeaseTime);
TRXGFUNC(getMocaInterfaceQoSFlowStatsLeaseTimeLeft);
/* TRXGFUNC(getMocaInterfaceQoSFlowStatsFlowPackets); */ /* NOT SUPPORT */

TRxObjNode  mocaInterfaceQoSFlowStatsInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {FlowID, {{tString, 17, 0}}, NULL, getMocaInterfaceQoSFlowStatsFlowID, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {PacketDA, {{tString, 17, 0}}, NULL, getMocaInterfaceQoSFlowStatsPacketDA, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {MaxRate, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceQoSFlowStatsMaxRate, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {MaxBurstSize, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceQoSFlowStatsMaxBurstSize, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {LeaseTime, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceQoSFlowStatsLeaseTime, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {LeaseTimeLeft, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceQoSFlowStatsLeaseTimeLeft, NULL, NULL, 2, 0, 0, 0, NULL, true},
    /* {FlowPackets, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceQoSFlowStatsFlowPackets, NULL, NULL, 2, 0, 0, 0, NULL, false}, */ /* NOT SUPPORT */
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{FlowID, {{tString, 17, 0}}, NULL, getMocaInterfaceQoSFlowStatsFlowID, NULL, NULL},
    {PacketDA, {{tString, 17, 0}}, NULL, getMocaInterfaceQoSFlowStatsPacketDA, NULL, NULL},
    {MaxRate, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceQoSFlowStatsMaxRate, NULL, NULL},
    {MaxBurstSize, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceQoSFlowStatsMaxBurstSize, NULL, NULL},
    {LeaseTime, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceQoSFlowStatsLeaseTime, NULL, NULL},
    {LeaseTimeLeft, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceQoSFlowStatsLeaseTimeLeft, NULL, NULL},
    /* {FlowPackets, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceQoSFlowStatsFlowPackets, NULL, NULL}, */ /* NOT SUPPORT */
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

/* Device.Moca.Interface.{i}.QoS.FlowStats. */

TRXGFUNC(addMocaInterfaceQoSFlowStatsInstance);
TRXSFUNC(deleteMocaInterfaceQoSFlowStatsInstance);

TRxObjNode  mocaInterfaceQoSFlowStatsDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, deleteMocaInterfaceQoSFlowStatsInstance, addMocaInterfaceQoSFlowStatsInstance, mocaInterfaceQoSFlowStatsInstanceDesc, NULL, 2, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {instanceIDMASK,{{0, 0, 0}}, deleteMocaInterfaceQoSFlowStatsInstance, addMocaInterfaceQoSFlowStatsInstance, mocaInterfaceQoSFlowStatsInstanceDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

/* Device.Moca.Interface.{i}.QoS. */

TRXGFUNC(getMocaInterfaceQoSEgressNumFlows);
TRXGFUNC(getMocaInterfaceQoSIngressNumFlows);
TRXGFUNC(getMocaInterfaceQoSFlowStatsNumberOfEntries);

TRxObjNode  mocaInterfaceQoSDesc[] = {
#ifdef XML_DOC_SUPPORT
    {EgressNumFlows, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceQoSEgressNumFlows, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {IngressNumFlows, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceQoSIngressNumFlows, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {FlowStatsNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceQoSFlowStatsNumberOfEntries, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {FlowStats, {{tInstance, 0, 0}}, NULL, NULL, mocaInterfaceQoSFlowStatsDesc, NULL, 2, 0, 0, 0xffffffff, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {EgressNumFlows, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceQoSEgressNumFlows, NULL, NULL},
    {IngressNumFlows, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceQoSIngressNumFlows, NULL, NULL},
    {FlowStatsNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceQoSFlowStatsNumberOfEntries, NULL, NULL},
    {FlowStats, {{tInstance, 0, 0}}, NULL, NULL, mocaInterfaceQoSFlowStatsDesc, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

/* Device.Moca.Interface.{i}.Stats. */

TRXGFUNC(getMocaInterfaceStatsBytesSent);
TRXGFUNC(getMocaInterfaceStatsBytesReceived);
TRXGFUNC(getMocaInterfaceStatsPacketsSent);
TRXGFUNC(getMocaInterfaceStatsPacketsReceived);
TRXGFUNC(getMocaInterfaceStatsErrorsSent);
TRXGFUNC(getMocaInterfaceStatsErrorsReceived);
TRXGFUNC(getMocaInterfaceStatsUnicastPacketsSent);
TRXGFUNC(getMocaInterfaceStatsUnicastPacketsReceived);
TRXGFUNC(getMocaInterfaceStatsDiscardPacketsSent);
TRXGFUNC(getMocaInterfaceStatsDiscardPacketsReceived);
TRXGFUNC(getMocaInterfaceStatsMulticastPacketsSent);
TRXGFUNC(getMocaInterfaceStatsMulticastPacketsReceived);
TRXGFUNC(getMocaInterfaceStatsBroadcastPacketsSent);
TRXGFUNC(getMocaInterfaceStatsBroadcastPacketsReceived);
/* TRXGFUNC(getMocaInterfaceStatsUnknownProtoPacketsReceived); */ /* NOT SUPPORT */

TRxObjNode  mocaInterfaceStatsDesc[] = {
#ifdef XML_DOC_SUPPORT
    {BytesSent, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsBytesSent, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {BytesReceived, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsBytesReceived, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {PacketsSent, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsPacketsSent, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {PacketsReceived, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsPacketsReceived, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {ErrorsSent, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsErrorsSent, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {ErrorsReceived, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsErrorsReceived, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {UnicastPacketsSent, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsUnicastPacketsSent, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {UnicastPacketsReceived, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsUnicastPacketsReceived, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {DiscardPacketsSent, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsDiscardPacketsSent, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {DiscardPacketsReceived, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsDiscardPacketsReceived, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {MulticastPacketsSent, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsMulticastPacketsSent, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {MulticastPacketsReceived, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsMulticastPacketsReceived, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {BroadcastPacketsSent, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsBroadcastPacketsSent, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {BroadcastPacketsReceived, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsBroadcastPacketsReceived, NULL, NULL, 2, 0, 0, 0, NULL, true},
    /* {UnknownProtoPacketsReceived, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsUnknownProtoPacketsReceived, NULL, NULL, 2, 0, 0, 0, NULL, false}, */ /* NOT SUPPORT */
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{BytesSent, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsBytesSent, NULL, NULL},
	{BytesReceived, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsBytesReceived, NULL, NULL},
	{PacketsSent, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsPacketsSent, NULL, NULL},
	{PacketsReceived, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsPacketsReceived, NULL, NULL},
	{ErrorsSent, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsErrorsSent, NULL, NULL},
	{ErrorsReceived, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsErrorsReceived, NULL, NULL},
	{UnicastPacketsSent, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsUnicastPacketsSent, NULL, NULL},
	{UnicastPacketsReceived, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsUnicastPacketsReceived, NULL, NULL},
	{DiscardPacketsSent, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsDiscardPacketsSent, NULL, NULL},
	{DiscardPacketsReceived, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsDiscardPacketsReceived, NULL, NULL},
	{MulticastPacketsSent, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsMulticastPacketsSent, NULL, NULL},
	{MulticastPacketsReceived, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsMulticastPacketsReceived, NULL, NULL},
	{BroadcastPacketsSent, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsBroadcastPacketsSent, NULL, NULL},
	{BroadcastPacketsReceived, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsBroadcastPacketsReceived, NULL, NULL},
	/* {UnknownProtoPacketsReceived, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceStatsUnknownProtoPacketsReceived, NULL, NULL}, */ /* NOT SUPPORT */
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

/* Device.Moca.Interface.{i}. */

TRXGFUNC(getMocaInterfaceEnable);
TRXSFUNC(setMocaInterfaceEnable);
TRXGFUNC(getMocaInterfaceStatus);
TRXGFUNC(getMocaInterfaceAlias);
TRXSFUNC(setMocaInterfaceAlias);
TRXGFUNC(getMocaInterfaceName);
TRXGFUNC(getMocaInterfaceLastChange);
TRXGFUNC(getMocaInterfaceLowerLayers);
TRXSFUNC(setMocaInterfaceLowerLayers);
TRXGFUNC(getMocaInterfaceUpstream);
TRXGFUNC(getMocaInterfaceMACAddress);
TRXGFUNC(getMocaInterfaceMaxBitRate);
TRXGFUNC(getMocaInterfaceFirmwareVersion);
TRXGFUNC(getMocaInterfaceMaxIngressBW);
TRXGFUNC(getMocaInterfaceMaxEgressBW);
TRXGFUNC(getMocaInterfaceHighestVersion);
TRXGFUNC(getMocaInterfaceCurrentVersion);
TRXGFUNC(getMocaInterfaceNetworkCoordinator);
TRXGFUNC(getMocaInterfaceNodeID);
TRXGFUNC(getMocaInterfaceMaxNodes);
TRXGFUNC(getMocaInterfacePreferredNC);
TRXSFUNC(setMocaInterfacePreferredNC);
TRXGFUNC(getMocaInterfaceBackupNC);
TRXGFUNC(getMocaInterfacePrivacyEnabledSetting);
TRXSFUNC(setMocaInterfacePrivacyEnabledSetting);
TRXGFUNC(getMocaInterfacePrivacyEnabled);
TRXGFUNC(getMocaInterfaceFreqCapabilityMask);
TRXGFUNC(getMocaInterfaceFreqCurrentMaskSetting);
TRXSFUNC(setMocaInterfaceFreqCurrentMaskSetting);
TRXGFUNC(getMocaInterfaceFreqCurrentMask);
TRXGFUNC(getMocaInterfaceCurrentOperFreq);
TRXGFUNC(getMocaInterfaceLastOperFreq);
TRXGFUNC(getMocaInterfaceKeyPassphrase);
TRXSFUNC(setMocaInterfaceKeyPassphrase);
TRXGFUNC(getMocaInterfaceTxPowerLimit);
TRXSFUNC(setMocaInterfaceTxPowerLimit);
TRXGFUNC(getMocaInterfacePowerCntlPhyTarget);
TRXSFUNC(setMocaInterfacePowerCntlPhyTarget);
TRXGFUNC(getMocaInterfaceBeaconPowerLimit);
TRXSFUNC(setMocaInterfaceBeaconPowerLimit);
TRXGFUNC(getMocaInterfaceNetworkTabooMask);
TRXGFUNC(getMocaInterfaceNodeTabooMask);
TRXGFUNC(getMocaInterfaceTxBcastRate);
TRXGFUNC(getMocaInterfaceTxBcastPowerReduction);
TRXGFUNC(getMocaInterfaceQAM256Capable);
TRXGFUNC(getMocaInterfacePacketAggregationCapability);
TRXGFUNC(getMocaInterfaceAssociatedDeviceNumberOfEntries);

TRxObjNode  mocaInterfaceInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Enable, {{tBool, 0, 0}}, setMocaInterfaceEnable, getMocaInterfaceEnable, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {Status, {{tString, 32, 0}}, NULL, getMocaInterfaceStatus, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {Alias, {{tString, 64, 0}}, setMocaInterfaceAlias, getMocaInterfaceAlias, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {Name, {{tString, 64, 0}}, NULL, getMocaInterfaceName, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {LastChange, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceLastChange, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {LowerLayers, {{tString, 1024, 0}}, setMocaInterfaceLowerLayers, getMocaInterfaceLowerLayers, NULL, NULL, 2, 0, 0, 0, NULL, true}, /* LowerLayers is not settable on this system */
    {Upstream, {{tBool, 0, 0}}, NULL, getMocaInterfaceUpstream, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {MACAddress, {{tString, 17, 0}}, NULL, getMocaInterfaceMACAddress, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {FirmwareVersion, {{tString, 64, 0}}, NULL, getMocaInterfaceFirmwareVersion, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {MaxBitRate, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceMaxBitRate, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {MaxIngressBW, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceMaxIngressBW, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {MaxEgressBW, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceMaxEgressBW, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {HighestVersion, {{tString, 64, 0}}, NULL, getMocaInterfaceHighestVersion, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {CurrentVersion, {{tString, 64, 0}}, NULL, getMocaInterfaceCurrentVersion, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {NetworkCoordinator, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceNetworkCoordinator, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {NodeID, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceNodeID, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {MaxNodes, {{tBool, 0, 0}}, NULL, getMocaInterfaceMaxNodes, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {PreferredNC, {{tBool, 0, 0}}, setMocaInterfacePreferredNC, getMocaInterfacePreferredNC, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {BackupNC, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceBackupNC, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {PrivacyEnabledSetting, {{tBool, 0, 0}}, setMocaInterfacePrivacyEnabledSetting, getMocaInterfacePrivacyEnabledSetting, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {PrivacyEnabled, {{tBool, 0, 0}}, NULL, getMocaInterfacePrivacyEnabled, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {FreqCapabilityMask, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceFreqCapabilityMask, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {FreqCurrentMaskSetting, {{tUnsigned, 0, 0}}, setMocaInterfaceFreqCurrentMaskSetting, getMocaInterfaceFreqCurrentMaskSetting, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {FreqCurrentMask, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceFreqCurrentMask, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {CurrentOperFreq, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceCurrentOperFreq, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {LastOperFreq, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceLastOperFreq, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {KeyPassphrase, {{tString, 17, 0}}, setMocaInterfaceKeyPassphrase, getMocaInterfaceKeyPassphrase, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {TxPowerLimit, {{tUnsigned, 0, 0}}, setMocaInterfaceTxPowerLimit, getMocaInterfaceTxPowerLimit, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {PowerCntlPhyTarget, {{tUnsigned, 0, 0}}, setMocaInterfacePowerCntlPhyTarget, getMocaInterfacePowerCntlPhyTarget, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {BeaconPowerLimit, {{tUnsigned, 0, 0}}, setMocaInterfaceBeaconPowerLimit, getMocaInterfaceBeaconPowerLimit, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {NetworkTabooMask, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceNetworkTabooMask, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {NodeTabooMask, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceNodeTabooMask, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {TxBcastRate, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceTxBcastRate, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {TxBcastPowerReduction, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceTxBcastPowerReduction, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {QAM256Capable, {{tBool, 0, 0}}, NULL, getMocaInterfaceQAM256Capable, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {PacketAggregationCapability, {{tUnsigned, 0, 0}}, NULL, getMocaInterfacePacketAggregationCapability, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {AssociatedDeviceNumberOfEntries, {{tUnsigned, 0, 0}},  NULL, getMocaInterfaceAssociatedDeviceNumberOfEntries, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {Stats, {{tObject, 0, 0}}, NULL, NULL, mocaInterfaceStatsDesc, NULL, 2, 0, 0, 0, NULL, true},
    {QoS, {{tObject, 0, 0}}, NULL, NULL, mocaInterfaceQoSDesc, NULL, 2, 0, 0, 0, NULL, true},
    {AssociatedDevice, {{tInstance, 0, 0}}, NULL, NULL, mocaInterfaceAssociatedDeviceDesc, NULL, 2, 0, 0, 0xffffffff, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{Enable, {{tBool, 0, 0}}, setMocaInterfaceEnable, getMocaInterfaceEnable, NULL, NULL},
	{Status, {{tString, 32, 0}}, NULL, getMocaInterfaceStatus, NULL, NULL},
	{Alias, {{tString, 64, 0}}, setMocaInterfaceAlias, getMocaInterfaceAlias, NULL, NULL},
	{Name, {{tString, 64, 0}}, NULL, getMocaInterfaceName, NULL, NULL},
	{LastChange, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceLastChange, NULL, NULL},
	{LowerLayers, {{tString, 1024, 0}}, setMocaInterfaceLowerLayers, getMocaInterfaceLowerLayers, NULL, NULL}, /* LowerLayers is not settable on this system */
	{Upstream, {{tBool, 0, 0}}, NULL, getMocaInterfaceUpstream, NULL, NULL},
	{MACAddress, {{tString, 17, 0}}, NULL, getMocaInterfaceMACAddress, NULL, NULL},
	{FirmwareVersion, {{tString, 64, 0}}, NULL, getMocaInterfaceFirmwareVersion, NULL, NULL},
	{MaxBitRate, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceMaxBitRate, NULL, NULL},
	{MaxIngressBW, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceMaxIngressBW, NULL, NULL},
	{MaxEgressBW, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceMaxEgressBW, NULL, NULL},
	{HighestVersion, {{tString, 64, 0}}, NULL, getMocaInterfaceHighestVersion, NULL, NULL},
	{CurrentVersion, {{tString, 64, 0}}, NULL, getMocaInterfaceCurrentVersion, NULL, NULL},
	{NetworkCoordinator, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceNetworkCoordinator, NULL, NULL},
	{NodeID, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceNodeID, NULL, NULL},
	{MaxNodes, {{tBool, 0, 0}}, NULL, getMocaInterfaceMaxNodes, NULL, NULL},
	{PreferredNC, {{tBool, 0, 0}}, setMocaInterfacePreferredNC, getMocaInterfacePreferredNC, NULL, NULL},
	{BackupNC, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceBackupNC, NULL, NULL},
	{PrivacyEnabledSetting, {{tBool, 0, 0}}, setMocaInterfacePrivacyEnabledSetting, getMocaInterfacePrivacyEnabledSetting, NULL, NULL},
	{PrivacyEnabled, {{tBool, 0, 0}}, NULL, getMocaInterfacePrivacyEnabled, NULL, NULL},
	{FreqCapabilityMask, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceFreqCapabilityMask, NULL, NULL},
	{FreqCurrentMaskSetting, {{tUnsigned, 0, 0}}, setMocaInterfaceFreqCurrentMaskSetting, getMocaInterfaceFreqCurrentMaskSetting, NULL, NULL},
	{FreqCurrentMask, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceFreqCurrentMask, NULL, NULL},
	{CurrentOperFreq, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceCurrentOperFreq, NULL, NULL},
	{LastOperFreq, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceLastOperFreq, NULL, NULL},
	{KeyPassphrase, {{tString, 17, 0}}, setMocaInterfaceKeyPassphrase, getMocaInterfaceKeyPassphrase, NULL, NULL},
	{TxPowerLimit, {{tUnsigned, 0, 0}}, setMocaInterfaceTxPowerLimit, getMocaInterfaceTxPowerLimit, NULL, NULL},
	{PowerCntlPhyTarget, {{tUnsigned, 0, 0}}, setMocaInterfacePowerCntlPhyTarget, getMocaInterfacePowerCntlPhyTarget, NULL, NULL},
	{BeaconPowerLimit, {{tUnsigned, 0, 0}}, setMocaInterfaceBeaconPowerLimit, getMocaInterfaceBeaconPowerLimit, NULL, NULL},
	{NetworkTabooMask, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceNetworkTabooMask, NULL, NULL},
	{NodeTabooMask, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceNodeTabooMask, NULL, NULL},
	{TxBcastRate, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceTxBcastRate, NULL, NULL},
	{TxBcastPowerReduction, {{tUnsigned, 0, 0}}, NULL, getMocaInterfaceTxBcastPowerReduction, NULL, NULL},
	{QAM256Capable, {{tBool, 0, 0}}, NULL, getMocaInterfaceQAM256Capable, NULL, NULL},
	{PacketAggregationCapability, {{tUnsigned, 0, 0}}, NULL, getMocaInterfacePacketAggregationCapability, NULL, NULL},
	{AssociatedDeviceNumberOfEntries, {{tUnsigned, 0, 0}},  NULL, getMocaInterfaceAssociatedDeviceNumberOfEntries, NULL, NULL},
	{Stats, {{tObject, 0, 0}}, NULL, NULL, mocaInterfaceStatsDesc, NULL},
    {QoS, {{tObject, 0, 0}}, NULL, NULL, mocaInterfaceQoSDesc, NULL},
    {AssociatedDevice, {{tInstance, 0, 0}}, NULL, NULL, mocaInterfaceAssociatedDeviceDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

/* Device.Moca.Interface. */

TRXGFUNC(addMocaInterfaceInstance);
TRXSFUNC(deleteMocaInterfaceInstance);

TRxObjNode  mocaInterfaceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, deleteMocaInterfaceInstance, addMocaInterfaceInstance, mocaInterfaceInstanceDesc, NULL, 2, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, deleteMocaInterfaceInstance, addMocaInterfaceInstance, mocaInterfaceInstanceDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

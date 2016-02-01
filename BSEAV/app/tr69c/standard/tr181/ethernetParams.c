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
#include "ethernetParams.h"

/* Device.Ethernet.Interface.{i}.Stats. */

TRXGFUNC(getEthernetInterfaceStatsBytesSent);
TRXGFUNC(getEthernetInterfaceStatsBytesReceived);
TRXGFUNC(getEthernetInterfaceStatsPacketsSent);
TRXGFUNC(getEthernetInterfaceStatsPacketsReceived);
TRXGFUNC(getEthernetInterfaceStatsErrorsSent);
TRXGFUNC(getEthernetInterfaceStatsErrorsReceived);
TRXGFUNC(getEthernetInterfaceStatsUnicastPacketsSent);
TRXGFUNC(getEthernetInterfaceStatsUnicastPacketsReceived);
TRXGFUNC(getEthernetInterfaceStatsDiscardPacketsSent);
TRXGFUNC(getEthernetInterfaceStatsDiscardPacketsReceived);
TRXGFUNC(getEthernetInterfaceStatsMulticastPacketsSent);
TRXGFUNC(getEthernetInterfaceStatsMulticastPacketsReceived);
TRXGFUNC(getEthernetInterfaceStatsBroadcastPacketsSent);
TRXGFUNC(getEthernetInterfaceStatsBroadcastPacketsReceived);
TRXGFUNC(getEthernetInterfaceStatsUnknownProtoPacketsReceived);

TRxObjNode  ethernetInterfaceStatsDesc[] = {
#ifdef XML_DOC_SUPPORT
    {BytesSent, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsBytesSent, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {BytesReceived, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsBytesReceived, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {PacketsSent, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsPacketsSent, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {PacketsReceived, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsPacketsReceived, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {ErrorsSent, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsErrorsSent, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {ErrorsReceived, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsErrorsReceived, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {UnicastPacketsSent, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsUnicastPacketsSent, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {UnicastPacketsReceived, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsUnicastPacketsReceived, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {DiscardPacketsSent, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsDiscardPacketsSent, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {DiscardPacketsReceived, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsDiscardPacketsReceived, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {MulticastPacketsSent, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsMulticastPacketsSent, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {MulticastPacketsReceived, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsMulticastPacketsReceived, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {BroadcastPacketsSent, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsBroadcastPacketsSent, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {BroadcastPacketsReceived, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsBroadcastPacketsReceived, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {UnknownProtoPacketsReceived, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsUnknownProtoPacketsReceived, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{BytesSent, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsBytesSent, NULL, NULL},
	{BytesReceived, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsBytesReceived, NULL, NULL},
	{PacketsSent, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsPacketsSent, NULL, NULL},
	{PacketsReceived, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsPacketsReceived, NULL, NULL},
	{ErrorsSent, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsErrorsSent, NULL, NULL},
	{ErrorsReceived, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsErrorsReceived, NULL, NULL},
	{UnicastPacketsSent, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsUnicastPacketsSent, NULL, NULL},
	{UnicastPacketsReceived, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsUnicastPacketsReceived, NULL, NULL},
	{DiscardPacketsSent, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsDiscardPacketsSent, NULL, NULL},
	{DiscardPacketsReceived, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsDiscardPacketsReceived, NULL, NULL},
	{MulticastPacketsSent, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsMulticastPacketsSent, NULL, NULL},
	{MulticastPacketsReceived, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsMulticastPacketsReceived, NULL, NULL},
	{BroadcastPacketsSent, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsBroadcastPacketsSent, NULL, NULL},
	{BroadcastPacketsReceived, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsBroadcastPacketsReceived, NULL, NULL},
	{UnknownProtoPacketsReceived, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceStatsUnknownProtoPacketsReceived, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

/* Device.Ethernet.Interface.{i}. */

TRXGFUNC(getEthernetInterfaceEnable);
TRXSFUNC(setEthernetInterfaceEnable);
TRXGFUNC(getEthernetInterfaceStatus);
TRXGFUNC(getEthernetInterfaceAlias);
TRXSFUNC(setEthernetInterfaceAlias);
TRXGFUNC(getEthernetInterfaceName);
TRXGFUNC(getEthernetInterfaceLastChange);
TRXGFUNC(getEthernetInterfaceLowerLayers);
TRXSFUNC(setEthernetInterfaceLowerLayers);
TRXGFUNC(getEthernetInterfaceUpstream);
TRXGFUNC(getEthernetInterfaceMACAddress);
TRXGFUNC(getEthernetInterfaceMaxBitRate);
TRXSFUNC(setEthernetInterfaceMaxBitRate);
TRXGFUNC(getEthernetInterfaceCurrentBitRate);
TRXGFUNC(getEthernetInterfaceDuplexMode);
TRXSFUNC(setEthernetInterfaceDuplexMode);
TRXGFUNC(getEthernetInterfaceEEECapability);
TRXGFUNC(getEthernetInterfaceEEEEnable);
TRXSFUNC(setEthernetInterfaceEEEEnable);

TRxObjNode  ethernetInterfaceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Enable, {{tBool, 0, 0}}, setEthernetInterfaceEnable, getEthernetInterfaceEnable, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {Status, {{tString, 32, 0}}, NULL, getEthernetInterfaceStatus, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {Alias, {{tString, 64, 0}}, setEthernetInterfaceAlias, getEthernetInterfaceAlias, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {Name, {{tString, 64, 0}}, NULL, getEthernetInterfaceName, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {LastChange, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceLastChange, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {LowerLayers, {{tString, 1024, 0}}, setEthernetInterfaceLowerLayers, getEthernetInterfaceLowerLayers, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {Upstream, {{tBool, 0, 0}}, NULL, getEthernetInterfaceUpstream, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {MACAddress, {{tString, 17, 0}}, NULL, getEthernetInterfaceMACAddress, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {MaxBitRate, {{tInt, 0, 0}}, setEthernetInterfaceMaxBitRate, getEthernetInterfaceMaxBitRate, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {CurrentBitRate, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceCurrentBitRate, NULL, NULL, 2, 7, 0, 0, NULL, true},
    {DuplexMode, {{tString, 32, 0}}, setEthernetInterfaceDuplexMode, getEthernetInterfaceDuplexMode, NULL, NULL, 2, 0, 0, 0, NULL, true},
    {EEECapability, {{tBool, 0, 0}}, NULL, getEthernetInterfaceEEECapability, NULL, NULL, 2, 8, 0, 0, NULL, true},
    {EEEEnable, {{tBool, 0, 0}}, setEthernetInterfaceEEEEnable, getEthernetInterfaceEEEEnable, NULL, NULL, 2, 8, 0, 0, NULL, true},
    {Stats, {{tObject, 0, 0}}, NULL, NULL, ethernetInterfaceStatsDesc, NULL, 2, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{Enable, {{tBool, 0, 0}}, setEthernetInterfaceEnable, getEthernetInterfaceEnable, NULL, NULL},
	{Status, {{tString, 32, 0}}, NULL, getEthernetInterfaceStatus, NULL, NULL},
	{Alias, {{tString, 64, 0}}, setEthernetInterfaceAlias, getEthernetInterfaceAlias, NULL, NULL},
	{Name, {{tString, 64, 0}}, NULL, getEthernetInterfaceName, NULL, NULL},
	{LastChange, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceLastChange, NULL, NULL},
	{LowerLayers, {{tString, 1024, 0}}, setEthernetInterfaceLowerLayers, getEthernetInterfaceLowerLayers, NULL, NULL},
	{Upstream, {{tBool, 0, 0}}, NULL, getEthernetInterfaceUpstream, NULL, NULL},
	{MACAddress, {{tString, 17, 0}}, NULL, getEthernetInterfaceMACAddress, NULL, NULL},
	{MaxBitRate, {{tInt, 0, 0}}, setEthernetInterfaceMaxBitRate, getEthernetInterfaceMaxBitRate, NULL, NULL},
	{CurrentBitRate, {{tUnsigned, 0, 0}}, NULL, getEthernetInterfaceCurrentBitRate, NULL, NULL},
	{DuplexMode, {{tString, 32, 0}}, setEthernetInterfaceDuplexMode, getEthernetInterfaceDuplexMode, NULL, NULL},
	{EEECapability, {{tBool, 0, 0}}, NULL, getEthernetInterfaceEEECapability, NULL, NULL},
	{EEEEnable, {{tBool, 0, 0}}, setEthernetInterfaceEEEEnable, getEthernetInterfaceEEEEnable, NULL, NULL},
    {Stats, {{tObject, 0, 0}}, NULL, NULL, ethernetInterfaceStatsDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

/* Device.Ethernet.Interface. */

TRXGFUNC(addEthernetInterfaceInstance);
TRXSFUNC(deleteEthernetInterfaceInstance);

TRxObjNode  ethernetInterfaceInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, deleteEthernetInterfaceInstance, addEthernetInterfaceInstance, ethernetInterfaceDesc, NULL, 2, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, deleteEthernetInterfaceInstance, addEthernetInterfaceInstance, ethernetInterfaceDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

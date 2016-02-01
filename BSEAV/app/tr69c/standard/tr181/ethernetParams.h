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

#ifndef __ETHERNET_PARAMS_H__
#define __ETHERNET_PARAMS_H__

#include "../../inc/tr69cdefs.h"

/* Device.Ethernet. */

/* SVAR(InterfaceNumberOfEntries); */
/* SVAR(LinkNumberOfEntries); */
/* SVAR(VLANTerminationNumberOfEntries); */
/* SVAR(RMONStatsNumberOfEntries); */

/* Device.Ethernet.RMONStats. */

/* SVAR(Enable); */
/* SVAR(Status); */
/* SVAR(Alias); */
/* SVAR(Name); */
/* SVAR(Interface); */
SVAR(VLANID);
SVAR(Queue);
SVAR(AllQueues);
SVAR(DropEvents);
SVAR(Bytes);
SVAR(Packets);
SVAR(BroadcastPackets);
SVAR(MuiltcastPackets);
SVAR(CRCErrorPackets);
SVAR(UndersizePackets);
SVAR(OversizePackets);
SVAR(Packets64Bytes);
SVAR(Packets65to127Bytes);
SVAR(Packets128to255Bytes);
SVAR(Packets256to511Bytes);
SVAR(Packets512to1023Bytes);
SVAR(Packets1024to1518Bytes);

/* Device.Ethernet.Interface. */

/* SVAR(Enable); */
/* SVAR(Status); */
/* SVAR(Alias); */
/* SVAR(Name); */
/* SVAR(LastChange); */
/* SVAR(LowerLayers); */
/* SVAR(Upstream); */
/* SVAR(MACAddress); */
/* SVAR(MaxBitRate); */
SVAR(CurrentBitRate);
SVAR(DuplexMode);
SVAR(EEECapability);
SVAR(EEEEnable);

/* Device.Ethernet.Interface.Stats. */

/* SVAR(BytesSent); */
/* SVAR(BytesReceived); */
/* SVAR(PacketsSent); */
/* SVAR(PacketsReceived); */
/* SVAR(ErrorsSent); */
/* SVAR(ErrorsReceived); */
/* SVAR(UnicastPacketsSent); */
/* SVAR(UnicastPacketsReceived); */
/* SVAR(DiscardPacketsSent); */
/* SVAR(DiscardPacketsReceived); */
/* SVAR(MulticastPacketsSent); */
/* SVAR(MulticastPacketsReceived); */
/* SVAR(BroadcastPacketsSent); */
/* SVAR(BroadcastPacketsReceived); */
/* SVAR(UnknownProtoPacketsReceived); */

/* Device.Ethernet.Link. */

/* SVAR(Enable); */
/* SVAR(Status); */
/* SVAR(Alias); */
/* SVAR(Name); */
/* SVAR(LastChange); */
/* SVAR(LowerLayers); */
/* SVAR(Upstream); */
/* SVAR(MACAddress); */
SVAR(PriorityTagging);

/* Device.Ethernet.Link.Stats. */

/* SVAR(BytesSent); */
/* SVAR(BytesReceived); */
/* SVAR(PacketsSent); */
/* SVAR(PacketsReceived); */
/* SVAR(ErrorsSent); */
/* SVAR(ErrorsReceived); */
/* SVAR(UnicastPacketsSent); */
/* SVAR(UnicastPacketsReceived); */
/* SVAR(DiscardPacketsSent); */
/* SVAR(DiscardPacketsReceived); */
/* SVAR(MulticastPacketsSent); */
/* SVAR(MulticastPacketsReceived); */
/* SVAR(BroadcastPacketsSent); */
/* SVAR(BroadcastPacketsReceived); */
/* SVAR(UnknownProtoPacketsReceived); */

/* Device.Ethernet.VLANTermination. */

/* SVAR(Enable); */
/* SVAR(Status); */
/* SVAR(Alias); */
/* SVAR(Name); */
/* SVAR(LastChange); */
/* SVAR(LowerLayers); */
/* SVAR(VLANID); */
SVAR(TPID);

/* Device.Ethernet.VLANTermination.Stats. */

/* SVAR(BytesSent); */
/* SVAR(BytesReceived); */
/* SVAR(PacketsSent); */
/* SVAR(PacketsReceived); */
/* SVAR(ErrorsSent); */
/* SVAR(ErrorsReceived); */
/* SVAR(UnicastPacketsSent); */
/* SVAR(UnicastPacketsReceived); */
/* SVAR(DiscardPacketsSent); */
/* SVAR(DiscardPacketsReceived); */
/* SVAR(MulticastPacketsSent); */
/* SVAR(MulticastPacketsReceived); */
/* SVAR(BroadcastPacketsSent); */
/* SVAR(BroadcastPacketsReceived); */
/* SVAR(UnknownProtoPacketsReceived); */

#endif   /* __ETHERNET_PARAMS_H__ */

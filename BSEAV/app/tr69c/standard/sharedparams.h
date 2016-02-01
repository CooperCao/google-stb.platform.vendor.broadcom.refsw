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

#ifndef __SHARED_PARAMS_H__
#define __SHARED_PARAMS_H__

#include "../inc/tr69cdefs.h"

/* they are used by lan and wan profiles*/
SVAR(Enable);
SVAR(Status);
SVAR(SubnetMask);
SVAR(DNSServers);
SVAR(Stats);
SVAR(TotalBytesSent);
SVAR(TotalBytesReceived);
SVAR(TotalPacketsSent);
SVAR(TotalPacketsReceived);

/* It is used by lan and wireless profiles */
SVAR(MACAddressControlEnabled);

/* It is used by lan and ipping profiles */
SVAR(Host);

/* they are used by baseline and wan profiles*/
SVAR(Username);
SVAR(Password);

/* they are used by wan and ipping profiles */
SVAR(DiagnosticsState);
SVAR(NumberOfRepetitions);
SVAR(Timeout);
SVAR(SuccessCount);
SVAR(FailureCount);
SVAR(AverageResponseTime);
SVAR(MinimumResponseTime);
SVAR(MaximumResponseTime);

/* It is used by ipping and route profiles*/
SVAR(Interface);

/* they are used by lan and voiceservice profiles */
/* Device.LANDevice.LANEthernetInterfaceConfig.Stats. */
SVAR(BytesSent);
SVAR(BytesReceived);
SVAR(PacketsSent);
SVAR(PacketsReceived);

/* It is used by wan and voiceservice profiles*/
SVAR(Name);

/* The followings are shared by Device.Moca. */
SVAR(InterfaceNumberOfEntries);
SVAR(ErrorsSent);
SVAR(ErrorsReceived);
SVAR(UnicastPacketsSent);
SVAR(UnicastPacketsReceived);
SVAR(DiscardPacketsSent);
SVAR(DiscardPacketsReceived);
SVAR(MulticastPacketsSent);
SVAR(MulticastPacketsReceived);
SVAR(BroadcastPacketsSent);
SVAR(BroadcastPacketsReceived);
SVAR(UnknownProtoPacketsReceived);
SVAR(LastChange);
SVAR(LowerLayers);
SVAR(Upstream);
SVAR(MACAddress);
SVAR(MaxBitRate);

/* The followings are shared by Device.Service.STBServices. */
SVAR(Alias);
SVAR(ProfileLevelNumberOfEntries);
SVAR(ProfileLevel);
SVAR(Profile);
SVAR(Level);
SVAR(MaximumDecodingCapability);
SVAR(FrontEnd);
SVAR(IP);
SVAR(Inbound);
SVAR(Outbound);
SVAR(PVR);
SVAR(AudioDecoder);
SVAR(VideoDecoder);
SVAR(MPEG2Part2);
SVAR(MPEG4Part2);
SVAR(MPEG4Part10);
SVAR(SMPTEVC1);
SVAR(AudioOutput);
SVAR(VideoOutput);
SVAR(SCART);
SVAR(HDMI);
SVAR(SupportedResolutions);
SVAR(CECSupport);
SVAR(CA);
SVAR(DRM);
SVAR(ServiceMonitoring);
SVAR(UpTime);
SVAR(Total);
SVAR(CurrentDay);
SVAR(QuarterHour);
SVAR(TotalStart);
SVAR(CurrentDayStart);
SVAR(NumberOfJoins);
SVAR(NumberOfLeaves);
SVAR(MaxJoinDelay);
SVAR(DestinationAddress);
SVAR(MultiplexType);
SVAR(URI);
SVAR(Macrovision);
SVAR(HDCP);
SVAR(AVStream);
SVAR(MainStream);
SVAR(Reset);
SVAR(ResetTime);
SVAR(Reference);
SVAR(AudienceStats);

SVAR(X_BROADCOM_COM_QAM);
SVAR(LogicalChannelConnect);
SVAR(ServiceListDatabase);
SVAR(Install);
SVAR(QAMService);
SVAR(Modulation);
SVAR(LogicalChannel);
SVAR(LogicalChannelService);
#endif   /* __SHARED_PARAMS_H__ */

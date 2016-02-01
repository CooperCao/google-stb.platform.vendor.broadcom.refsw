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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include "sharedparams.h"
#include "stbcomponentsparams.h" /* profiles for parameter callbacks */


/* STBService.Components.DRM Object */
TRXSFUNC(setCompDRMEnable);
TRXGFUNC(getCompDRMEnable);
TRXGFUNC(getCompDRMStatus);
TRXSFUNC(setCompDRMAlias);
TRXGFUNC(getCompDRMAlias);
TRXGFUNC(getCompDRMName);
TRXGFUNC(getCompDRMSmartCardReader);

/* STBService.Components.CA Object */
TRXSFUNC(setCompCAEnable);
TRXGFUNC(getCompCAEnable);
TRXGFUNC(getCompCAStatus);
TRXSFUNC(setCompCAAlias);
TRXGFUNC(getCompCAAlias);
TRXGFUNC(getCompCAName);
TRXGFUNC(getCompCASmartCardReader);

/* STBService.Components.HDMI.DisplayDevice Object */
TRXGFUNC(getCompHDMIDisplayDeviceStatus);
TRXGFUNC(getCompHDMIDisplayDeviceName);
TRXGFUNC(getCompHDMIDisplayDeviceEEDID);
TRXGFUNC(getCompHDMIDisplayDeviceSupportedResolutions);
TRXGFUNC(getCompHDMIDisplayDevicePreferredResolution);
TRXGFUNC(getCompHDMIDisplayDeviceVideoLatency);
TRXGFUNC(getCompHDMIDisplayDeviceCECSupport);
TRXGFUNC(getCompHDMIDisplayDeviceAutoLipSyncSupport);
TRXGFUNC(getCompHDMIDisplayDeviceHDMI3DPresent);

/* STBService.Components.HDMI Object */
TRXSFUNC(setCompHDMIEnable);
TRXGFUNC(getCompHDMIEnable);
TRXGFUNC(getCompHDMIStatus);
TRXSFUNC(setCompHDMIAlias);
TRXGFUNC(getCompHDMIAlias);
TRXGFUNC(getCompHDMIName);
TRXSFUNC(setCompHDMIResolutionMode);
TRXGFUNC(getCompHDMIResolutionMode);
TRXSFUNC(setCompHDMIResolutionValue);
TRXGFUNC(getCompHDMIResolutionValue);

/* STBService.Components.SCART Object */
TRXSFUNC(setCompSCARTEnable);
TRXGFUNC(getCompSCARTEnable);
TRXGFUNC(getCompSCARTStatus);
TRXSFUNC(setCompSCARTAlias);
TRXGFUNC(getCompSCARTAlias);
TRXGFUNC(getCompSCARTName);
TRXSFUNC(setCompSCARTPresence);
TRXGFUNC(getCompSCARTPresence);

/* STBService.Components.VideoOutput Object */
TRXSFUNC(setCompVideoOutputEnable);
TRXGFUNC(getCompVideoOutputEnable);
TRXGFUNC(getCompVideoOutputStatus);
TRXSFUNC(setCompVideoOutputAlias);
TRXGFUNC(getCompVideoOutputAlias);
TRXSFUNC(setCompVideoOutputColorbarEnable);
TRXGFUNC(getCompVideoOutputColorbarEnable);
TRXGFUNC(getCompVideoOutputName);
TRXGFUNC(getCompVideoOutputCompositeVideoStandard);
TRXSFUNC(setCompVideoOutputVideoFormat);
TRXGFUNC(getCompVideoOutputVideoFormat);
TRXGFUNC(getCompVideoOutputAspectRatioBehaviour);
TRXSFUNC(setCompVideoOutputDisplayFormat);
TRXGFUNC(getCompVideoOutputDisplayFormat);
TRXGFUNC(getCompVideoOutputMacrovision);
TRXGFUNC(getCompVideoOutputHDCP);
TRXGFUNC(getCompVideoOutputSCARTs);

/* STBService.Components.SPDIF Object */
TRXSFUNC(setCompSPDIFEnable);
TRXGFUNC(getCompSPDIFEnable);
TRXGFUNC(getCompSPDIFStatus);
TRXSFUNC(setCompSPDIFAlias);
TRXGFUNC(getCompSPDIFAlias);
TRXGFUNC(getCompSPDIFName);
TRXSFUNC(setCompSPDIFForcePCM);
TRXGFUNC(getCompSPDIFForcePCM);
TRXGFUNC(getCompSPDIFPassthrough);
TRXGFUNC(getCompSPDIFAudioDelay);

/* STBService.Components.AudioOutput Object */
TRXSFUNC(setCompAudioOutputEnable);
TRXGFUNC(getCompAudioOutputEnable);
TRXGFUNC(getCompAudioOutputStatus);
TRXSFUNC(setCompAudioOutputAlias);
TRXGFUNC(getCompAudioOutputAlias);
TRXGFUNC(getCompAudioOutputName);
TRXGFUNC(getCompAudioOutputAudioFormat);
TRXSFUNC(setCompAudioOutputAudioLevel);
TRXGFUNC(getCompAudioOutputAudioLevel);
TRXSFUNC(setCompAudioOutputCancelMute);
TRXGFUNC(getCompAudioOutputCancelMute);
TRXGFUNC(getCompAudioOutputSCARTs);

/* STBService.Components.VideoDecoder Object */
TRXSFUNC(setCompVideoDecoderEnable);
TRXGFUNC(getCompVideoDecoderEnable);
TRXGFUNC(getCompVideoDecoderStatus);
TRXSFUNC(setCompVideoDecoderAlias);
TRXGFUNC(getCompVideoDecoderAlias);
TRXGFUNC(getCompVideoDecoderName);
TRXGFUNC(getCompVideoDecoderMPEG2Part2);
TRXGFUNC(getCompVideoDecoderMPEG4Part2);
TRXGFUNC(getCompVideoDecoderMPEG4Part10);
TRXGFUNC(getCompVideoDecoderSMPTEVC1);
TRXGFUNC(getCompVideoDecoderContentAspectRatio);

/* STBService.Components.AudioDecoder Object */
TRXSFUNC(setCompAudioDecoderEnable);
TRXGFUNC(getCompAudioDecoderEnable);
TRXGFUNC(getCompAudioDecoderStatus);
TRXSFUNC(setCompAudioDecoderAlias);
TRXGFUNC(getCompAudioDecoderAlias);
TRXGFUNC(getCompAudioDecoderName);
TRXGFUNC(getCompAudioDecoderStandard);

/* STBService.Components.PVR.Storage Object */
TRXSFUNC(setCompPVRStorageAlias);
TRXGFUNC(getCompPVRStorageAlias);
TRXGFUNC(getCompPVRStorageName);
TRXGFUNC(getCompPVRStorageReference);

/* STBService.Components.PVR Object */
TRXGFUNC(getCompPVRStorageNumberOfEntries);

/* STBService.Components.FrontEnd.IP.Dejittering Object */
TRXSFUNC(setCompFEIPDejitteringBufferSize);
TRXGFUNC(getCompFEIPDejitteringBufferSize);
TRXSFUNC(setCompFEIPDejitteringBufferInitialLevel);
TRXGFUNC(getCompFEIPDejitteringBufferInitialLevel);

/* STBService.Components.FrontEnd.IP.Outbound Object */
TRXGFUNC(getCompFEIPOutboundStatus);
TRXSFUNC(setCompFEIPOutboundAlias);
TRXGFUNC(getCompFEIPOutboundAlias);
TRXGFUNC(getCompFEIPOutboundMultiplexType);
TRXGFUNC(getCompFEIPOutboundURI);

/* STBService.Components.FrontEnd.IP.ServiceConnect Object */
TRXGFUNC(getCompFEIPServiceConnect);
TRXSFUNC(setCompFEIPServiceConnect);


/* STBService.Components.FrontEnd.IP.Inbound Object */
TRXGFUNC(getCompFEIPInboundStatus);
TRXSFUNC(setCompFEIPInboundAlias);
TRXGFUNC(getCompFEIPInboundAlias);
TRXGFUNC(getCompFEIPInboundStreamingControlProtocol);
TRXGFUNC(getCompFEIPInboundStreamingTransportProtocol);
TRXGFUNC(getCompFEIPInboundStreamingTransportControlProtocol);
TRXGFUNC(getCompFEIPInboundMultiplexType);
TRXGFUNC(getCompFEIPInboundDownloadTransportProtocol);
TRXGFUNC(getCompFEIPInboundSourceAddress);
TRXGFUNC(getCompFEIPInboundSourcePort);
TRXGFUNC(getCompFEIPInboundDestinationAddress);
TRXGFUNC(getCompFEIPInboundDestinationPort);
TRXGFUNC(getCompFEIPInboundURI);

/* STBService.Components.FrontEnd.IP.IGMP.ClientGroupStats.QuarterHour Object */
TRXGFUNC(getCompFEIPIGMPClientGroupStatsQuarterHourNumberOfJoins);
TRXGFUNC(getCompFEIPIGMPClientGroupStatsQuarterHourNumberOfLeaves);
TRXGFUNC(getCompFEIPIGMPClientGroupStatsQuarterHourMaxJoinDelay);

/* STBService.Components.FrontEnd.IP.IGMP.ClientGroupStats.Current Object */
TRXGFUNC(getCompFEIPIGMPClientGroupStatsCurrentDayNumberOfJoins);
TRXGFUNC(getCompFEIPIGMPClientGroupStatsCurrentDayNumberOfLeaves);
TRXGFUNC(getCompFEIPIGMPClientGroupStatsCurrentDayMaxJoinDelay);

/* STBService.Components.FrontEnd.IP.IGMP.ClientGroupStats.Total Object */
TRXGFUNC(getCompFEIPIGMPClientGroupStatsTotalNumberOfJoins);
TRXGFUNC(getCompFEIPIGMPClientGroupStatsTotalNumberOfLeaves);
TRXGFUNC(getCompFEIPIGMPClientGroupStatsTotalMaxJoinDelay);

/* STBService.Components.FrontEnd.IP.IGMP.ClientGroupStats Object */
TRXSFUNC(setCompFEIPIGMPClientGroupStatsAlias);
TRXGFUNC(getCompFEIPIGMPClientGroupStatsAlias);
TRXGFUNC(getCompFEIPIGMPClientGroupStatsAddress);
TRXGFUNC(getCompFEIPIGMPClientGroupStatsTotalStart);
TRXGFUNC(getCompFEIPIGMPClientGroupStatsCurrentDayStart);
TRXGFUNC(getCompFEIPIGMPClientGroupStatsQuartHourStart);

/* STBService.Components.FrontEnd.IP.IGMP.ClientGroup Object */
TRXSFUNC(setCompFEIPIGMPClientGroupAlias);
TRXGFUNC(getCompFEIPIGMPClientGroupAlias);
TRXGFUNC(getCompFEIPIGMPClientGroupAddress);
TRXGFUNC(getCompFEIPIGMPClientGroupUpTime);

/* STBService.Components.FrontEnd.IP.IGMP Object */
TRXSFUNC(setCompFEIPIGMPEnable);
TRXGFUNC(getCompFEIPIGMPEnable);
TRXGFUNC(getCompFEIPIGMPStatus);
TRXGFUNC(getCompFEIPIGMPMaximumNumberOfConcurrentGroups);
TRXGFUNC(getCompFEIPIGMPMaximumNumberOfTrackedGroups);
TRXSFUNC(setCompFEIPIGMPLoggingEnable);
TRXGFUNC(getCompFEIPIGMPLoggingEnable);
TRXSFUNC(setCompFEIPIGMPDSCPMark);
TRXGFUNC(getCompFEIPIGMPDSCPMark);
TRXSFUNC(setCompFEIPIGMPVLANIDMark);
TRXGFUNC(getCompFEIPIGMPVLANIDMark);
TRXSFUNC(setCompFEIPIGMPEthernetPriorityMark);
TRXGFUNC(getCompFEIPIGMPEthernetPriorityMark);
TRXGFUNC(getCompFEIPIGMClientVersion);
TRXSFUNC(setCompFEIPIGMClientRobustness);
TRXGFUNC(getCompFEIPIGMClientRobustness);
TRXSFUNC(setCompFEIPIGMClientUnsolicitedReportInterval);
TRXGFUNC(getCompFEIPIGMClientUnsolicitedReportInterval);
TRXGFUNC(getCompFEIPIGMClientGroupNumberOfEntries);
TRXGFUNC(getCompFEIPIGMClientGroupStatsNumberOfEntries);

/* STBService.Components.FrontEnd.IP Object */
TRXGFUNC(getCompFEIPInboundNumberOfEntries);
TRXGFUNC(getCompFEIPOutboundNumberOfEntries);
TRXGFUNC(getCompFEIPActiveInboundIPStreams);
TRXGFUNC(getCompFEIPActiveOutboundIPStreams);

/* STBService.Components.FrontEnd Object */
TRXSFUNC(setCompFEEnable);
TRXGFUNC(getCompFEEnable);
TRXGFUNC(getCompFEStatus);
TRXSFUNC(setCompFEAlias);
TRXGFUNC(getCompFEAlias);
TRXGFUNC(getCompFEName);

/* STBService.Components Object */
TRXGFUNC(getCompFENumberOfEntries);
TRXGFUNC(getCompAudioDecoderNumberOfEntries);
TRXGFUNC(getCompVideoDecoderNumberOfEntries);
TRXGFUNC(getCompAudioOutputNumberOfEntries);
TRXGFUNC(getCompVideoOutputNumberOfEntries);
TRXGFUNC(getCompSCARTNumberOfEntries);
TRXGFUNC(getCompCANNumberOfEntries);
TRXGFUNC(getCompDRMNumberOfEntries);
TRXGFUNC(getCompHDMINumberOfEntries);
TRXGFUNC(getCompSPDIFNumberOfEntries);

/* STBService.Components.FrontEnd.X_BROADCOM_COM_QAM.LogicalChannelConnect object */
TRXSFUNC(setCompFEQAMLogicalChannelConnectLogicalChannelNumber);
TRXGFUNC(getCompFEQAMLogicalChannelConnectLogicalChannelNumber);

/* STBService.Components.FrontEnd.X_BROADCOM_COM_QAM.LogicalChannelService object */
TRXGFUNC(getCompFEQAMLogicalChannelServiceAlias);
TRXGFUNC(getCompFEQAMLogicalChannelServiceName);
TRXGFUNC(getCompFEQAMLogicalChannelServiceFrequency);
TRXGFUNC(getCompFEQAMLogicalChannelServiceBER);
TRXGFUNC(getCompFEQAMLogicalChannelServiceCBER);
TRXGFUNC(getCompFEQAMLogicalChannelServiceSNR);
TRXSFUNC(setCompFEQAMLogicalChannelServicePreferred);
TRXGFUNC(getCompFEQAMLogicalChannelServicePreferred);

/* STBService.Components.FrontEnd.X_BROADCOM_COM_QAM.LogicalChannel object */
TRXSFUNC(setCompFEQAMServiceListDatabaseLogicalChannelAlias);
TRXGFUNC(getCompFEQAMServiceListDatabaseLogicalChannelAlias);
TRXGFUNC(getCompFEQAMServiceListDatabaseLogicalChannelLogicalChannelNumber);
TRXGFUNC(getCompFEQAMServiceListDatabaseLogicalChannelServiceNumberOfEntries);

/* STBService.Components.FrontEnd.X_BROADCOM_COM_QAM.ServiceListDatabase object */
TRXSFUNC(setCompFEQAMServiceListDatabaseReset);
TRXGFUNC(getCompFEQAMServiceListDatabaseReset);
TRXGFUNC(getCompFEQAMServiceListDatabaseTotalServices);
TRXGFUNC(getCompFEQAMServiceListDatabaseLogicalChannelNumberOfEntries);

/* STBService.Components.FrontEnd.X_BROADCOM_COM_QAM.Install object */
TRXSFUNC(setCompFEQAMInstallStart);
TRXGFUNC(getCompFEQAMInstallStart);
TRXGFUNC(getCompFEQAMInstallStatus);
TRXGFUNC(getCompFEQAMInstallProgress);
TRXSFUNC(setCompFEQAMInstallStartFrequency);
TRXGFUNC(getCompFEQAMInstallStartFrequency);
TRXSFUNC(setCompFEQAMInstallStopFrequency);
TRXGFUNC(getCompFEQAMInstallStopFrequency);

/* STBService.Components.FrontEnd.X_BROADCOM_COM_QAM.Service object */
TRXGFUNC(getCompFEQAMServiceCurrentLogicalChannel);
TRXGFUNC(getCompFEQAMServiceCurrentService);

/* STBService.Components.FrontEnd.X_BROADCOM_COM_QAM.Modulation object */
TRXSFUNC(setCompFEQAMModulationFrequency);
TRXGFUNC(getCompFEQAMModulationFrequency);
TRXSFUNC(setCompFEQAMModulationChannelBandwidth);
TRXGFUNC(getCompFEQAMModulationChannelBandwidth);
TRXSFUNC(setCompFEQAMModulationQamMode);
TRXGFUNC(getCompFEQAMModulationQamMode);
TRXSFUNC(setCompFEQAMModulationAnnex);
TRXGFUNC(getCompFEQAMModulationAnnex);
TRXSFUNC(setCompFEQAMModulationFrequency);
TRXGFUNC(getCompFEQAMModulationFrequency);
TRXSFUNC(setCompFEQAMModulationEnablePowerMeasurement);
TRXGFUNC(getCompFEQAMModulationEnablePowerMeasurement);
TRXSFUNC(setCompFEQAMModulationSpectrumMode);
TRXGFUNC(getCompFEQAMModulationSpectrumMode);
TRXSFUNC(setCompFEQAMModulationAcquisitionMode);
TRXGFUNC(getCompFEQAMModulationAcquisitionMode);
TRXGFUNC(getCompFEQAMModulationReceiverLock);
TRXGFUNC(getCompFEQAMModulationFECLock);
TRXGFUNC(getCompFEQAMModulationOpllLock);
TRXSFUNC(setCompFEQAMModulationSpectrumInverted);
TRXGFUNC(getCompFEQAMModulationSpectrumInverted);
TRXSFUNC(setCompFEQAMModulationSymbolRate);
TRXGFUNC(getCompFEQAMModulationSymbolRate);
TRXGFUNC(getCompFEQAMModulationSymbolRateError);
TRXGFUNC(getCompFEQAMModulationIFAgcLevel);
TRXGFUNC(getCompFEQAMModulationRFAgcLevel);
TRXGFUNC(getCompFEQAMModulationIntAgcLevel);
TRXGFUNC(getCompFEQAMModulationSNREstimate);
TRXGFUNC(getCompFEQAMModulationFECCorrected);
TRXGFUNC(getCompFEQAMModulationFECUncorrected);
TRXGFUNC(getCompFEQAMModulationFECClean);
TRXGFUNC(getCompFEQAMModulationBitErrCorrected);
TRXGFUNC(getCompFEQAMModulationReacquireCount);
TRXGFUNC(getCompFEQAMModulationViterbiUncorrectedBits);
TRXGFUNC(getCompFEQAMModulationViterbiTotalBits);
TRXGFUNC(getCompFEQAMModulationViterbiErrorRate);
TRXGFUNC(getCompFEQAMModulationErrorRateUnits);
TRXGFUNC(getCompFEQAMModulationCarrierFreqOffset);
TRXGFUNC(getCompFEQAMModulationCarrierPhaseOffset);
TRXGFUNC(getCompFEQAMModulationGoodRsBlockCount);
TRXGFUNC(getCompFEQAMModulationBerRawCount);
TRXGFUNC(getCompFEQAMModulationDSChannelPower);
TRXGFUNC(getCompFEQAMModulationMainTap);
TRXGFUNC(getCompFEQAMModulationPostRsBer);
TRXGFUNC(getCompFEQAMModulationPostRsBerElapsedTime);
TRXGFUNC(getCompFEQAMModulationInterleaveDepth);
TRXGFUNC(getCompFEQAMModulationLnaAgcLevel);
TRXGFUNC(getCompFEQAMModulationEqualizerGain);
TRXGFUNC(getCompFEQAMModulationFrontendGain);
TRXGFUNC(getCompFEQAMModulationDigitalAgcGain);
TRXGFUNC(getCompFEQAMModulationHighResEqualizerGain);

TRxObjNode CompDRMDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Enable, {{tBool, 0, 0}}, setCompDRMEnable, getCompDRMEnable, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {Status, {{tString, 32, 0}}, NULL, getCompDRMStatus, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {Alias, {{tString, 64, 0}}, setCompDRMAlias, getCompDRMAlias, NULL, NULL, 1, 2, 0, 0, NULL, false},
    {Name, {{tString, 256, 0}}, NULL, getCompDRMName, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {SmartCardReader, {{tString, 256, 0}}, NULL, getCompDRMSmartCardReader, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Enable, {{tBool, 0, 0}}, setCompDRMEnable, getCompDRMEnable, NULL, NULL},
    {Status, {{tString, 32, 0}}, NULL, getCompDRMStatus, NULL, NULL},
	{Alias, {{tString, 64, 0}}, setCompDRMAlias, getCompDRMAlias, NULL, NULL},
    {Name, {{tString, 256, 0}}, NULL, getCompDRMName, NULL, NULL},
    {SmartCardReader, {{tString, 256, 0}}, NULL, getCompDRMSmartCardReader, NULL, NULL},
	{NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompDRMInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompDRMDesc, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompDRMDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompCADesc[] = {
#ifdef XML_DOC_SUPPORT
    {Enable, {{tBool, 0, 0}}, setCompCAEnable, getCompCAEnable, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {Status, {{tString, 32, 0}}, NULL, getCompCAStatus, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {Alias, {{tString, 64, 0}}, setCompCAAlias, getCompCAAlias, NULL, NULL, 1, 2, 0, 0, NULL, false},
    {Name, {{tString, 256, 0}}, NULL, getCompCAName, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {SmartCardReader, {{tString, 256, 0}}, NULL, getCompCASmartCardReader, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Enable, {{tBool, 0, 0}}, setCompCAEnable, getCompCAEnable, NULL, NULL},
    {Status, {{tString, 32, 0}}, NULL, getCompCAStatus, NULL, NULL},
	{Alias, {{tString, 64, 0}}, setCompCAAlias, getCompCAAlias, NULL, NULL},
    {Name, {{tString, 256, 0}}, NULL, getCompCAName, NULL, NULL},
    {SmartCardReader, {{tString, 256, 0}}, NULL, getCompCASmartCardReader, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompCAInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompCADesc, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompCADesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompHDMIDisplayDeviceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Status, {{tString, 32, 0}}, NULL, getCompHDMIDisplayDeviceStatus, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {Name, {{tString, 256, 0}}, NULL, getCompHDMIDisplayDeviceName, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {EEDID, {{tString, 256, 0}}, NULL, getCompHDMIDisplayDeviceEEDID, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {SupportedResolutions, {{tString, 32, 0}}, NULL, getCompHDMIDisplayDeviceSupportedResolutions, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {PreferredResolution, {{tString, 32, 0}}, NULL, getCompHDMIDisplayDevicePreferredResolution, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {VideoLatency, {{tUnsigned, 0, 0}}, NULL, getCompHDMIDisplayDeviceVideoLatency, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {CECSupport, {{tBool, 0, 0}}, NULL, getCompHDMIDisplayDeviceCECSupport, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {AutoLipSyncSupport, {{tBool, 0, 0}}, NULL, getCompHDMIDisplayDeviceAutoLipSyncSupport, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {HDMI3DPresent, {{tBool, 0, 0}}, NULL, getCompHDMIDisplayDeviceHDMI3DPresent, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Status, {{tString, 32, 0}}, NULL, getCompHDMIDisplayDeviceStatus, NULL, NULL},
	{Name, {{tString, 256, 0}}, NULL, getCompHDMIDisplayDeviceName, NULL, NULL},
	{EEDID, {{tString, 256, 0}}, NULL, getCompHDMIDisplayDeviceEEDID, NULL, NULL},
    {SupportedResolutions, {{tString, 32, 0}}, NULL, getCompHDMIDisplayDeviceSupportedResolutions, NULL, NULL},
    {PreferredResolution, {{tString, 32, 0}}, NULL, getCompHDMIDisplayDevicePreferredResolution, NULL, NULL},
	{VideoLatency, {{tUnsigned, 0, 0}}, NULL, getCompHDMIDisplayDeviceVideoLatency, NULL, NULL},
	{CECSupport, {{tBool, 0, 0}}, NULL, getCompHDMIDisplayDeviceCECSupport, NULL, NULL},
	{AutoLipSyncSupport, {{tBool, 0, 0}}, NULL, getCompHDMIDisplayDeviceAutoLipSyncSupport, NULL, NULL},
	{HDMI3DPresent, {{tBool, 0, 0}}, NULL, getCompHDMIDisplayDeviceHDMI3DPresent, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompHDMIDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Enable, {{tBool, 0, 0}}, setCompHDMIEnable, getCompHDMIEnable, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {Status, {{tString, 32, 0}}, NULL, getCompHDMIStatus, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {Alias, {{tString, 64, 0}}, setCompHDMIAlias, getCompHDMIAlias, NULL, NULL, 1, 2, 0, 0, NULL, true},
    {Name, {{tString, 256, 0}}, NULL, getCompHDMIName, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {ResolutionMode, {{tString, 32, 0}}, setCompHDMIResolutionMode, getCompHDMIResolutionMode, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {ResolutionValue, {{tString, 32, 0}}, setCompHDMIResolutionValue, getCompHDMIResolutionValue, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {DisplayDevice, {{tObject, 0, 0}}, NULL, NULL, CompHDMIDisplayDeviceDesc, NULL, 1, 1, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Enable, {{tBool, 0, 0}}, setCompHDMIEnable, getCompHDMIEnable, NULL, NULL},
    {Status, {{tString, 32, 0}}, NULL, getCompHDMIStatus, NULL, NULL},
	{Alias, {{tString, 64, 0}}, setCompHDMIAlias, getCompHDMIAlias, NULL, NULL},
    {Name, {{tString, 256, 0}}, NULL, getCompHDMIName, NULL, NULL},
	{ResolutionMode, {{tString, 32, 0}}, setCompHDMIResolutionMode, getCompHDMIResolutionMode, NULL, NULL},
	{ResolutionValue, {{tString, 32, 0}}, setCompHDMIResolutionValue, getCompHDMIResolutionValue, NULL, NULL},
	{DisplayDevice, {{tObject, 0, 0}}, NULL, NULL, CompHDMIDisplayDeviceDesc, NULL},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompHDMIInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompHDMIDesc, NULL, 1, 1, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompHDMIDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompSCARTDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Enable, {{tBool, 0, 0}}, setCompSCARTEnable, getCompSCARTEnable, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {Status, {{tString, 32, 0}}, NULL, getCompSCARTStatus, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {Alias, {{tString, 64, 0}}, setCompSCARTAlias, getCompSCARTAlias, NULL, NULL, 1, 2, 0, 0, NULL, false},
    {Name, {{tString, 256, 0}}, NULL, getCompSCARTName, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {Presence, {{tBool, 0, 0}}, setCompSCARTPresence, getCompSCARTPresence, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Enable, {{tBool, 0, 0}}, setCompSCARTEnable, getCompSCARTEnable, NULL, NULL},
    {Status, {{tString, 32, 0}}, NULL, getCompSCARTStatus, NULL, NULL},
	{Alias, {{tString, 64, 0}}, setCompSCARTAlias, getCompSCARTAlias, NULL, NULL},
    {Name, {{tString, 256, 0}}, NULL, getCompSCARTName, NULL, NULL},
    {Presence, {{tBool, 0, 0}}, setCompSCARTPresence, getCompSCARTPresence, NULL, NULL},
	{NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompSCARTInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompSCARTDesc, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompSCARTDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompVideoOutputDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Enable, {{tBool, 0, 0}}, setCompVideoOutputEnable, getCompVideoOutputEnable, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Status, {{tString, 32, 0}}, NULL, getCompVideoOutputStatus, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Alias, {{tString, 64, 0}}, setCompVideoOutputAlias, getCompVideoOutputAlias, NULL, NULL, 1, 2, 0, 0, NULL, true},
    {ColorbarEnable, {{tBool, 0, 0}}, setCompVideoOutputColorbarEnable, getCompVideoOutputColorbarEnable, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {Name, {{tString, 256, 0}}, NULL, getCompVideoOutputName,NULL, NULL, 1, 0, 0, 0, NULL, true},
    {CompositeVideoStandard, {{tString, 32, 0}}, NULL, getCompVideoOutputCompositeVideoStandard,NULL, NULL, 1, 0, 0, 0, NULL, true},
    {VideoFormat, {{tString, 32, 0}}, setCompVideoOutputVideoFormat, getCompVideoOutputVideoFormat, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {AspectRatioBehaviour, {{tString, 32, 0}}, NULL, getCompVideoOutputAspectRatioBehaviour, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {DisplayFormat, {{tString, 32, 0}}, setCompVideoOutputDisplayFormat, getCompVideoOutputDisplayFormat, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {Macrovision, {{tBool, 0, 0}}, NULL, getCompVideoOutputMacrovision, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {HDCP, {{tBool, 0, 0}}, NULL, getCompVideoOutputHDCP, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {SCARTs, {{tString, 1024, 0}}, NULL, getCompVideoOutputSCARTs, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Enable, {{tBool, 0, 0}}, setCompVideoOutputEnable, getCompVideoOutputEnable, NULL, NULL},
    {Status, {{tString, 32, 0}}, NULL, getCompVideoOutputStatus, NULL, NULL},
	{Alias, {{tString, 64, 0}}, setCompVideoOutputAlias, getCompVideoOutputAlias, NULL, NULL},
	{ColorbarEnable, {{tBool, 0, 0}}, setCompVideoOutputColorbarEnable, getCompVideoOutputColorbarEnable, NULL, NULL},
    {Name, {{tString, 256, 0}}, NULL, getCompVideoOutputName,NULL, NULL},
	{CompositeVideoStandard, {{tString, 32, 0}}, NULL, getCompVideoOutputCompositeVideoStandard,NULL, NULL},
    {VideoFormat, {{tString, 32, 0}}, setCompVideoOutputVideoFormat, getCompVideoOutputVideoFormat, NULL, NULL},
    {AspectRatioBehaviour, {{tString, 32, 0}}, NULL, getCompVideoOutputAspectRatioBehaviour, NULL, NULL},
	{DisplayFormat, {{tString, 32, 0}}, setCompVideoOutputDisplayFormat, getCompVideoOutputDisplayFormat, NULL, NULL},
    {Macrovision, {{tBool, 0, 0}}, NULL, getCompVideoOutputMacrovision, NULL, NULL},
    {HDCP, {{tBool, 0, 0}}, NULL, getCompVideoOutputHDCP, NULL, NULL},
    {SCARTs, {{tString, 1024, 0}}, NULL, getCompVideoOutputSCARTs, NULL, NULL},
	{NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompVideoOutputInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompVideoOutputDesc, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompVideoOutputDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompSPDIFDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Enable, {{tBool, 0, 0}}, setCompSPDIFEnable, getCompSPDIFEnable, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {Status, {{tString, 32, 0}}, NULL, getCompSPDIFStatus, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {Alias, {{tString, 64, 0}}, setCompSPDIFAlias, getCompSPDIFAlias, NULL, NULL, 1, 2, 0, 0, NULL, true},
    {Name, {{tString, 256, 0}}, NULL, getCompSPDIFName, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {ForcePCM, {{tBool, 0, 0}}, setCompSPDIFForcePCM, getCompSPDIFForcePCM, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {Passthrough, {{tBool, 0, 0}}, NULL, getCompSPDIFPassthrough, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {AudioDelay, {{tUnsigned,0,0}}, NULL, getCompSPDIFAudioDelay, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Enable, {{tBool, 0, 0}}, setCompSPDIFEnable, getCompSPDIFEnable, NULL, NULL},
	{Status, {{tString, 32, 0}}, NULL, getCompSPDIFStatus, NULL, NULL},
	{Alias, {{tString, 64, 0}}, setCompSPDIFAlias, getCompSPDIFAlias, NULL, NULL},
	{Name, {{tString, 256, 0}}, NULL, getCompSPDIFName, NULL, NULL},
	{ForcePCM, {{tBool, 0, 0}}, setCompSPDIFForcePCM, getCompSPDIFForcePCM, NULL, NULL},
	{Passthrough, {{tBool, 0, 0}}, NULL, getCompSPDIFPassthrough, NULL, NULL},
	{AudioDelay, {{tUnsigned,0,0}}, NULL, getCompSPDIFAudioDelay, NULL, NULL},
	{NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompSPDIFInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompSPDIFDesc, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompSPDIFDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompAudioOutputDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Enable, {{tBool, 0, 0}}, setCompAudioOutputEnable, getCompAudioOutputEnable, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Status, {{tString, 32, 0}}, NULL, getCompAudioOutputStatus, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Alias, {{tString, 64, 0}}, setCompAudioOutputAlias, getCompAudioOutputAlias, NULL, NULL, 1, 2, 0, 0, NULL, true},
    {Name, {{tString, 256, 0}}, NULL, getCompAudioOutputName, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {AudioFormat, {{tString, 32, 0}}, NULL, getCompAudioOutputAudioFormat, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {AudioLevel, {{tUnsigned, 0, 0}}, setCompAudioOutputAudioLevel, getCompAudioOutputAudioLevel, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {CancelMute,{ {tBool, 0, 0}}, setCompAudioOutputCancelMute, getCompAudioOutputCancelMute, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {SCARTs, {{tString, 1024, 0}}, NULL,getCompAudioOutputSCARTs, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Enable, {{tBool, 0, 0}}, setCompAudioOutputEnable, getCompAudioOutputEnable, NULL, NULL},
    {Status, {{tString, 32, 0}}, NULL, getCompAudioOutputStatus, NULL, NULL},
	{Alias, {{tString, 64, 0}}, setCompAudioOutputAlias, getCompAudioOutputAlias, NULL, NULL},
    {Name, {{tString, 256, 0}}, NULL, getCompAudioOutputName, NULL, NULL},
    {AudioFormat, {{tString, 32, 0}}, NULL, getCompAudioOutputAudioFormat, NULL, NULL},
    {AudioLevel, {{tUnsigned, 0, 0}}, setCompAudioOutputAudioLevel, getCompAudioOutputAudioLevel, NULL, NULL},
    {CancelMute,{ {tBool, 0, 0}}, setCompAudioOutputCancelMute, getCompAudioOutputCancelMute, NULL, NULL},
    {SCARTs, {{tString, 1024, 0}}, NULL,getCompAudioOutputSCARTs, NULL, NULL},
	{NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompAudioOutputInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompAudioOutputDesc, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompAudioOutputDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompVideoDecoderDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Enable, {{tBool, 0, 0}}, setCompVideoDecoderEnable, getCompVideoDecoderEnable, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Status, {{tString, 32, 0}}, NULL, getCompVideoDecoderStatus, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Alias, {{tString, 64, 0}}, setCompVideoDecoderAlias, getCompVideoDecoderAlias, NULL, NULL, 1, 2, 0, 0, NULL, true},
    {Name, {{tString, 256, 0}}, NULL, getCompVideoDecoderName, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MPEG2Part2, {{tString, 256, 0}}, NULL, getCompVideoDecoderMPEG2Part2, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MPEG4Part2, {{tString, 256, 0}}, NULL, getCompVideoDecoderMPEG4Part2, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MPEG4Part10, {{tString, 256, 0}}, NULL, getCompVideoDecoderMPEG4Part10, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {SMPTEVC1, {{tString, 256, 0}}, NULL, getCompVideoDecoderSMPTEVC1, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ContentAspectRatio, {{tString, 256, 0}}, NULL, getCompVideoDecoderContentAspectRatio, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Enable, {{tBool, 0, 0}}, setCompVideoDecoderEnable, getCompVideoDecoderEnable, NULL, NULL},
    {Status, {{tString, 32, 0}}, NULL, getCompVideoDecoderStatus, NULL, NULL},
	{Alias, {{tString, 64, 0}}, setCompVideoDecoderAlias, getCompVideoDecoderAlias, NULL, NULL},
    {Name, {{tString, 256, 0}}, NULL, getCompVideoDecoderName, NULL, NULL},
    {MPEG2Part2, {{tString, 256, 0}}, NULL, getCompVideoDecoderMPEG2Part2, NULL, NULL},
    {MPEG4Part2, {{tString, 256, 0}}, NULL, getCompVideoDecoderMPEG4Part2, NULL, NULL},
    {MPEG4Part10, {{tString, 256, 0}}, NULL, getCompVideoDecoderMPEG4Part10, NULL, NULL},
    {SMPTEVC1, {{tString, 256, 0}}, NULL, getCompVideoDecoderSMPTEVC1, NULL, NULL},
    {ContentAspectRatio, {{tString, 256, 0}}, NULL, getCompVideoDecoderContentAspectRatio, NULL, NULL},
	{NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompVideoDecoderInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompVideoDecoderDesc, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompVideoDecoderDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompAudioDecoderDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Enable, {{tBool, 0, 0}}, setCompAudioDecoderEnable, getCompAudioDecoderEnable, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Status, {{tString, 32, 0}}, NULL, getCompAudioDecoderStatus, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Alias, {{tString, 64, 0}}, setCompAudioDecoderAlias, getCompAudioDecoderAlias, NULL, NULL, 1, 2, 0, 0, NULL, true},
    {Name, {{tString, 256, 0}}, NULL, getCompAudioDecoderName, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {AudioStandard, {{tString, 32, 0}}, NULL, getCompAudioDecoderStandard, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Enable, {{tBool, 0, 0}}, setCompAudioDecoderEnable, getCompAudioDecoderEnable, NULL, NULL},
    {Status, {{tString, 32, 0}}, NULL, getCompAudioDecoderStatus, NULL, NULL},
	{Alias, {{tString, 64, 0}}, setCompAudioDecoderAlias, getCompAudioDecoderAlias, NULL, NULL},
    {Name, {{tString, 256, 0}}, NULL, getCompAudioDecoderName, NULL, NULL},
    {AudioStandard, {{tString, 32, 0}}, NULL, getCompAudioDecoderStandard, NULL, NULL},
	{NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompAudioDecoderInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompAudioDecoderDesc, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompAudioDecoderDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompPVRStorageDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Alias, {{tString, 64, 0}}, setCompPVRStorageAlias, getCompPVRStorageAlias, NULL, NULL, 1, 2, 0, 0, NULL, true},
    {Name, {{tString, 256, 0}}, NULL, getCompPVRStorageName, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Reference, {{tUnsigned, 0, 0}}, NULL, getCompPVRStorageReference, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{Alias, {{tString, 64, 0}}, setCompPVRStorageAlias, getCompPVRStorageAlias, NULL, NULL},
	{Name, {{tString, 256, 0}}, NULL, getCompPVRStorageName, NULL, NULL},
    {Reference, {{tUnsigned, 0, 0}}, NULL, getCompPVRStorageReference, NULL, NULL},
	{NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompPVRStorageInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompPVRStorageDesc, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompPVRStorageDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompPVRDesc[] = {
#ifdef XML_DOC_SUPPORT
    {StorageNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompPVRStorageNumberOfEntries, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Storage, {{tInstance, 0, 0}}, NULL, NULL, CompPVRStorageInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, true},
    {NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {StorageNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompPVRStorageNumberOfEntries, NULL, NULL},
    {Storage, {{tInstance, 0, 0}}, NULL, NULL, CompPVRStorageInstanceDesc, NULL},
	{NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompFEIPDejitteringDesc[] = {
#ifdef XML_DOC_SUPPORT
    {BufferSize, {{tUnsigned, 0, 0}}, setCompFEIPDejitteringBufferSize, getCompFEIPDejitteringBufferSize, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {BufferInitialLevel, {{tUnsigned, 0, 0}}, setCompFEIPDejitteringBufferInitialLevel, getCompFEIPDejitteringBufferInitialLevel, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {BufferSize, {{tUnsigned, 0, 0}}, setCompFEIPDejitteringBufferSize, getCompFEIPDejitteringBufferSize, NULL, NULL},
    {BufferInitialLevel, {{tUnsigned, 0, 0}}, setCompFEIPDejitteringBufferInitialLevel, getCompFEIPDejitteringBufferInitialLevel, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompFEIPOutboundDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Status, {{tString, 32, 0}}, NULL, getCompFEIPOutboundStatus, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {Alias, {{tString, 64, 0}}, setCompFEIPOutboundAlias, getCompFEIPOutboundAlias, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {MultiplexType, {{tString, 32, 0}}, NULL, getCompFEIPOutboundMultiplexType, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {URI, {{tString, 32, 0}}, NULL, getCompFEIPOutboundURI, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Status, {{tString, 32, 0}}, NULL, getCompFEIPOutboundStatus, NULL, NULL},
	{Alias, {{tString, 64, 0}}, setCompFEIPOutboundAlias, getCompFEIPOutboundAlias, NULL, NULL},
    {MultiplexType, {{tString, 32, 0}}, NULL, getCompFEIPOutboundMultiplexType, NULL, NULL},
    {URI, {{tString, 32, 0}}, NULL, getCompFEIPOutboundURI, NULL, NULL},
	{NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompFEIPOutboundInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompFEIPOutboundDesc, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompFEIPOutboundDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompFEIPServiceConnectInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {ServiceConnect, {{tUnsigned, 0, 0}}, setCompFEIPServiceConnect, getCompFEIPServiceConnect, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{ServiceConnect, {{tUnsigned, 0, 0}}, setCompFEIPServiceConnect, getCompFEIPServiceConnect, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompFEIPInboundDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Status, {{tString, 32, 0}}, NULL, getCompFEIPInboundStatus, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Alias, {{tString, 64, 0}}, setCompFEIPInboundAlias, getCompFEIPInboundAlias, NULL, NULL, 1, 2, 0, 0, NULL, true},
    {StreamingControlProtocol, {{tString, 32, 0}}, NULL, getCompFEIPInboundStreamingControlProtocol, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {StreamingTransportProtocol, {{tString, 32, 0}}, NULL, getCompFEIPInboundStreamingTransportProtocol, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {StreamingTransportControlProtocol, {{tString, 32, 0}}, NULL, getCompFEIPInboundStreamingTransportControlProtocol, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MultiplexType, {{tString, 32, 0}}, NULL, getCompFEIPInboundMultiplexType, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {DownloadTransportProtocol, {{tString, 32, 0}}, NULL, getCompFEIPInboundDownloadTransportProtocol, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {SourceAddress, {{tString, 32, 0}}, NULL, getCompFEIPInboundSourceAddress, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {SourcePort, {{tUnsigned, 0, 0}}, NULL, getCompFEIPInboundSourcePort, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {DestinationAddress, {{tString, 32, 0}}, NULL, getCompFEIPInboundDestinationAddress, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {DestinationPort, {{tUnsigned, 0, 0}}, NULL, getCompFEIPInboundDestinationPort, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {URI, {{tString, 32, 0}}, NULL, getCompFEIPInboundURI, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Status, {{tString, 32, 0}}, NULL, getCompFEIPInboundStatus, NULL, NULL},
	{Alias, {{tString, 64, 0}}, setCompFEIPInboundAlias, getCompFEIPInboundAlias, NULL, NULL},
    {StreamingControlProtocol, {{tString, 32, 0}}, NULL, getCompFEIPInboundStreamingControlProtocol, NULL, NULL},
    {StreamingTransportProtocol, {{tString, 32, 0}}, NULL, getCompFEIPInboundStreamingTransportProtocol, NULL, NULL},
    {StreamingTransportControlProtocol, {{tString, 32, 0}}, NULL, getCompFEIPInboundStreamingTransportControlProtocol, NULL, NULL},
    {MultiplexType, {{tString, 32, 0}}, NULL, getCompFEIPInboundMultiplexType, NULL, NULL},
    {DownloadTransportProtocol, {{tString, 32, 0}}, NULL, getCompFEIPInboundDownloadTransportProtocol, NULL, NULL},
    {SourceAddress, {{tString, 32, 0}}, NULL, getCompFEIPInboundSourceAddress, NULL, NULL},
    {SourcePort, {{tUnsigned, 0, 0}}, NULL, getCompFEIPInboundSourcePort, NULL, NULL},
    {DestinationAddress, {{tString, 32, 0}}, NULL, getCompFEIPInboundDestinationAddress, NULL, NULL},
    {DestinationPort, {{tUnsigned, 0, 0}}, NULL, getCompFEIPInboundDestinationPort, NULL, NULL},
    {URI, {{tString, 32, 0}}, NULL, getCompFEIPInboundURI, NULL, NULL},
	{NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompFEIPInboundInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompFEIPInboundDesc, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompFEIPInboundDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompFEIPIGMPClientGroupStatsQuarterHourDesc[] = {
#ifdef XML_DOC_SUPPORT
    {NumberOfJoins, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupStatsQuarterHourNumberOfJoins, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NumberOfLeaves, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupStatsQuarterHourNumberOfLeaves, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {MaxJoinDelay, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupStatsQuarterHourMaxJoinDelay, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {NumberOfJoins, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupStatsQuarterHourNumberOfJoins, NULL, NULL},
    {NumberOfLeaves, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupStatsQuarterHourNumberOfLeaves, NULL, NULL},
    {MaxJoinDelay, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupStatsQuarterHourMaxJoinDelay, NULL, NULL},
	{NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompFEIPIGMPClientGroupStatsCurrentDayDesc[] = {
#ifdef XML_DOC_SUPPORT
    {NumberOfJoins, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupStatsCurrentDayNumberOfJoins, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NumberOfLeaves, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupStatsCurrentDayNumberOfLeaves, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {MaxJoinDelay, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupStatsCurrentDayMaxJoinDelay, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {NumberOfJoins, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupStatsCurrentDayNumberOfJoins, NULL, NULL},
    {NumberOfLeaves, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupStatsCurrentDayNumberOfLeaves, NULL, NULL},
    {MaxJoinDelay, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupStatsCurrentDayMaxJoinDelay, NULL, NULL},
	{NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompFEIPIGMPClientGroupStatsTotalDesc[] = {
#ifdef XML_DOC_SUPPORT
    {NumberOfJoins, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupStatsTotalNumberOfJoins, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NumberOfLeaves, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupStatsTotalNumberOfLeaves, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {MaxJoinDelay, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupStatsTotalMaxJoinDelay, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {NumberOfJoins, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupStatsTotalNumberOfJoins, NULL, NULL},
    {NumberOfLeaves, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupStatsTotalNumberOfLeaves, NULL, NULL},
    {MaxJoinDelay, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupStatsTotalMaxJoinDelay, NULL, NULL},
	{NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompFEIPIGMPClientGroupStatsDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Alias, {{tString, 64, 0}}, setCompFEIPIGMPClientGroupStatsAlias, getCompFEIPIGMPClientGroupStatsAlias, NULL, NULL, 1, 2, 0, 0, NULL, false},
    {Address, {{tString, 256, 0}}, NULL, getCompFEIPIGMPClientGroupStatsAddress, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {TotalStart, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupStatsTotalStart, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {CurrentDayStart, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupStatsCurrentDayStart, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {QuartHourStart, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupStatsQuartHourStart, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {Total, {{tObject, 0, 0}}, NULL, NULL, CompFEIPIGMPClientGroupStatsTotalDesc, NULL, 1, 0, 0, 0, NULL, false},
    {CurrentDay, {{tObject, 0, 0}}, NULL, NULL, CompFEIPIGMPClientGroupStatsCurrentDayDesc, NULL, 1, 0, 0, 0, NULL, false},
    {QuarterHour, {{tObject, 0, 0}}, NULL, NULL, CompFEIPIGMPClientGroupStatsQuarterHourDesc, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{Alias, {{tString, 64, 0}}, setCompFEIPIGMPClientGroupStatsAlias, getCompFEIPIGMPClientGroupStatsAlias, NULL, NULL},
    {Address, {{tString, 256, 0}}, NULL, getCompFEIPIGMPClientGroupStatsAddress, NULL, NULL},
    {TotalStart, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupStatsTotalStart, NULL, NULL},
    {CurrentDayStart, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupStatsCurrentDayStart, NULL, NULL},
    {QuartHourStart, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupStatsQuartHourStart, NULL, NULL},
    {Total, {{tObject, 0, 0}}, NULL, NULL, CompFEIPIGMPClientGroupStatsTotalDesc, NULL},
    {CurrentDay, {{tObject, 0, 0}}, NULL, NULL, CompFEIPIGMPClientGroupStatsCurrentDayDesc, NULL},
    {QuarterHour, {{tObject, 0, 0}}, NULL, NULL, CompFEIPIGMPClientGroupStatsQuarterHourDesc, NULL},
	{NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompFEIPIGMPClientGroupStatsInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompFEIPIGMPClientGroupStatsDesc, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompFEIPIGMPClientGroupStatsDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompFEIPIGMPClientGroupDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Alias, {{tString, 64, 0}}, setCompFEIPIGMPClientGroupAlias, getCompFEIPIGMPClientGroupAlias, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {GroupAddress, {{tString, 256, 0}}, NULL, getCompFEIPIGMPClientGroupAddress, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {UpTime, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupUpTime, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{Alias, {{tString, 64, 0}}, setCompFEIPIGMPClientGroupAlias, getCompFEIPIGMPClientGroupAlias, NULL, NULL},
    {GroupAddress, {{tString, 256, 0}}, NULL, getCompFEIPIGMPClientGroupAddress, NULL, NULL},
    {UpTime, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPClientGroupUpTime, NULL, NULL},
	{NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompFEIPIGMPClientGroupInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompFEIPIGMPClientGroupDesc, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompFEIPIGMPClientGroupDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompFEIPIGMPDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Enable, {{tBool, 0, 0}}, setCompFEIPIGMPEnable, getCompFEIPIGMPEnable, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {Status, {{tString, 32, 0}}, NULL, getCompFEIPIGMPStatus, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {MaximumNumberOfConcurrentGroups, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPMaximumNumberOfConcurrentGroups, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {MaximumNumberOfTrackedGroups, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPMaximumNumberOfTrackedGroups, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {LoggingEnable, {{tBool, 0, 0}}, setCompFEIPIGMPLoggingEnable, getCompFEIPIGMPLoggingEnable, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {DSCPMark, {{tUnsigned, 0, 0}}, setCompFEIPIGMPDSCPMark, getCompFEIPIGMPDSCPMark, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {VLANIDMark, {{tInt, 0, 0}}, setCompFEIPIGMPVLANIDMark, getCompFEIPIGMPVLANIDMark, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {EthernetPriorityMark, {{tInt, 0, 0}}, setCompFEIPIGMPEthernetPriorityMark, getCompFEIPIGMPEthernetPriorityMark, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {ClientVersion, {{tString, 32, 0}}, NULL, getCompFEIPIGMClientVersion, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {ClientRobustness, {{tUnsigned, 0, 0}}, setCompFEIPIGMClientRobustness, getCompFEIPIGMClientRobustness, NULL, NULL, 1, 3, 0, 0, NULL, false},
    {ClientUnsolicitedReportInterval, {{tUnsigned, 0, 0}}, setCompFEIPIGMClientUnsolicitedReportInterval, getCompFEIPIGMClientUnsolicitedReportInterval, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {ClientGroupNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMClientGroupNumberOfEntries,NULL,NULL, 1, 0, 0, 0, NULL, false},
    {ClientGroupStatsNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMClientGroupStatsNumberOfEntries, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {ClientGroup, {{tInstance, 0, 0}}, NULL, NULL, CompFEIPIGMPClientGroupInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, false},
    {ClientGroupStats, {{tInstance, 0, 0}}, NULL, NULL, CompFEIPIGMPClientGroupStatsInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, false},
    {NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Enable, {{tBool, 0, 0}}, setCompFEIPIGMPEnable, getCompFEIPIGMPEnable, NULL, NULL},
    {Status, {{tString, 32, 0}}, NULL, getCompFEIPIGMPStatus, NULL, NULL},
    {MaximumNumberOfConcurrentGroups, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPMaximumNumberOfConcurrentGroups, NULL, NULL},
    {MaximumNumberOfTrackedGroups, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMPMaximumNumberOfTrackedGroups, NULL, NULL},
    {LoggingEnable, {{tBool, 0, 0}}, setCompFEIPIGMPLoggingEnable, getCompFEIPIGMPLoggingEnable, NULL, NULL},
    {DSCPMark, {{tUnsigned, 0, 0}}, setCompFEIPIGMPDSCPMark, getCompFEIPIGMPDSCPMark, NULL, NULL},
    {VLANIDMark, {{tInt, 0, 0}}, setCompFEIPIGMPVLANIDMark, getCompFEIPIGMPVLANIDMark, NULL, NULL},
    {EthernetPriorityMark, {{tInt, 0, 0}}, setCompFEIPIGMPEthernetPriorityMark, getCompFEIPIGMPEthernetPriorityMark, NULL, NULL},
    {ClientVersion, {{tString, 32, 0}}, NULL, getCompFEIPIGMClientVersion, NULL, NULL},
    {ClientRobustness, {{tUnsigned, 0, 0}}, setCompFEIPIGMClientRobustness, getCompFEIPIGMClientRobustness, NULL, NULL},
    {ClientUnsolicitedReportInterval, {{tUnsigned, 0, 0}}, setCompFEIPIGMClientUnsolicitedReportInterval, getCompFEIPIGMClientUnsolicitedReportInterval, NULL, NULL},
    {ClientGroupNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMClientGroupNumberOfEntries,NULL,NULL},
    {ClientGroupStatsNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompFEIPIGMClientGroupStatsNumberOfEntries, NULL, NULL},
    {ClientGroup, {{tInstance, 0, 0}}, NULL, NULL, CompFEIPIGMPClientGroupInstanceDesc, NULL},
    {ClientGroupStats, {{tInstance, 0, 0}}, NULL, NULL, CompFEIPIGMPClientGroupStatsInstanceDesc, NULL},
	{NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompFEIPDesc[] = {
#ifdef XML_DOC_SUPPORT
    {InboundNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompFEIPInboundNumberOfEntries, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {OutboundNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompFEIPOutboundNumberOfEntries, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ActiveInboundIPStreams, {{tUnsigned, 0, 0}}, NULL, getCompFEIPActiveInboundIPStreams, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ActiveOutboundIPStreams, {{tUnsigned, 0, 0}}, NULL, getCompFEIPActiveOutboundIPStreams, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {IGMP, {{tObject, 0, 0}}, NULL, NULL, CompFEIPIGMPDesc, NULL, 1, 0, 0, 0, NULL, false},
    {Dejittering, {{tObject, 0, 0}}, NULL, NULL, CompFEIPDejitteringDesc, NULL, 1, 0, 0, 0, NULL, true},
    {Inbound, {{tInstance, 0, 0}}, NULL, NULL, CompFEIPInboundInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, true},
    {Outbound, {{tInstance, 0, 0}}, NULL, NULL, CompFEIPOutboundInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, true},
    {ServiceConnect, {{tObject, 0, 0}}, NULL, NULL, CompFEIPServiceConnectInstanceDesc, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {InboundNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompFEIPInboundNumberOfEntries, NULL, NULL},
    {OutboundNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompFEIPOutboundNumberOfEntries, NULL, NULL},
    {ActiveInboundIPStreams, {{tUnsigned, 0, 0}}, NULL, getCompFEIPActiveInboundIPStreams, NULL, NULL},
    {ActiveOutboundIPStreams, {{tUnsigned, 0, 0}}, NULL, getCompFEIPActiveOutboundIPStreams, NULL, NULL},
    {IGMP, {{tObject, 0, 0}}, NULL, NULL, CompFEIPIGMPDesc, NULL},
    {Dejittering, {{tObject, 0, 0}}, NULL, NULL, CompFEIPDejitteringDesc, NULL},
    {Inbound, {{tInstance, 0, 0}}, NULL, NULL, CompFEIPInboundInstanceDesc, NULL},
    {Outbound, {{tInstance, 0, 0}}, NULL, NULL, CompFEIPOutboundInstanceDesc, NULL},
	{ServiceConnect, {{tObject, 0, 0}}, NULL, NULL, CompFEIPServiceConnectInstanceDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode LogicalChannelConnectInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {LogicalChannelNumber, {{tUnsigned, 0, 0}}, setCompFEQAMLogicalChannelConnectLogicalChannelNumber, getCompFEQAMLogicalChannelConnectLogicalChannelNumber, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {LogicalChannelNumber, {{tUnsigned, 0, 0}}, setCompFEQAMLogicalChannelConnectLogicalChannelNumber, getCompFEQAMLogicalChannelConnectLogicalChannelNumber, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode LogicalChannelConnectDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, LogicalChannelConnectInstanceDesc, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, LogicalChannelConnectInstanceDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode LogicalChannelServiceInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Alias, {{tString, 64, 0}}, NULL, getCompFEQAMLogicalChannelServiceAlias, NULL, NULL, 1, 2, 0, 0, NULL, false},
    {Name, {{tUnsigned, 0, 0}}, NULL, getCompFEQAMLogicalChannelServiceName, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {Frequency, {{tUnsigned, 0, 0}}, NULL, getCompFEQAMLogicalChannelServiceFrequency, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {BER, {{tUnsigned, 0, 0}}, NULL, getCompFEQAMLogicalChannelServiceBER, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {CBER, {{tUnsigned, 0, 0}}, NULL, getCompFEQAMLogicalChannelServiceCBER, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {SNR, {{tUnsigned, 0, 0}}, NULL, getCompFEQAMLogicalChannelServiceSNR, NULL, NULL, 1, 1, 0, 0, NULL, false},
    {Preferred, {{tBool, 0, 0}}, setCompFEQAMLogicalChannelServicePreferred, getCompFEQAMLogicalChannelServicePreferred, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Alias, {{tString, 64, 0}}, NULL, getCompFEQAMLogicalChannelServiceAlias, NULL, NULL},
	{Name, {{tUnsigned, 0, 0}}, NULL, getCompFEQAMLogicalChannelServiceName, NULL, NULL},
    {Frequency, {{tUnsigned, 0, 0}}, NULL, getCompFEQAMLogicalChannelServiceFrequency, NULL, NULL},
	{BER, {{tUnsigned, 0, 0}}, NULL, getCompFEQAMLogicalChannelServiceBER, NULL, NULL},
	{CBER, {{tUnsigned, 0, 0}}, NULL, getCompFEQAMLogicalChannelServiceCBER, NULL, NULL},
    {SNR, {{tUnsigned, 0, 0}}, NULL, getCompFEQAMLogicalChannelServiceSNR, NULL, NULL},
	{Preferred, {{tBool, 0, 0}}, setCompFEQAMLogicalChannelServicePreferred, getCompFEQAMLogicalChannelServicePreferred, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode LogicalChannelServiceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, LogicalChannelServiceInstanceDesc, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, LogicalChannelServiceInstanceDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};


TRxObjNode LogicalChannelDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Alias, {{tString, 64, 0}}, setCompFEQAMServiceListDatabaseLogicalChannelAlias, getCompFEQAMServiceListDatabaseLogicalChannelAlias, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {LogicalChannelNumber, {{tUnsigned, 0, 0}}, NULL, getCompFEQAMServiceListDatabaseLogicalChannelLogicalChannelNumber, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {ServiceNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompFEQAMServiceListDatabaseLogicalChannelServiceNumberOfEntries, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {LogicalChannelService, {{tInstance, 0, 0}}, NULL, NULL, LogicalChannelServiceInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Alias, {{tString, 64, 0}}, setCompFEQAMServiceListDatabaseLogicalChannelAlias, getCompFEQAMServiceListDatabaseLogicalChannelAlias, NULL, NULL},
	{LogicalChannelNumber, {{tUnsigned, 0, 0}}, NULL, getCompFEQAMServiceListDatabaseLogicalChannelLogicalChannelNumber, NULL, NULL},
    {ServiceNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompFEQAMServiceListDatabaseLogicalChannelServiceNumberOfEntries, NULL, NULL},
	{LogicalChannelService, {{tInstance, 0, 0}}, NULL, NULL, LogicalChannelServiceInstanceDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode ServiceListDatabaseDesc[] = {
#ifdef XML_DOC_SUPPORT
    {ServiceListReset, {{tBool, 0, 0}}, setCompFEQAMServiceListDatabaseReset, getCompFEQAMServiceListDatabaseReset, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {TotalServices, {{tUnsigned, 0, 0}}, NULL, getCompFEQAMServiceListDatabaseTotalServices, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {LogicalChannelNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompFEQAMServiceListDatabaseLogicalChannelNumberOfEntries, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {LogicalChannel, {{tInstance, 0, 0}}, NULL, NULL, LogicalChannelDesc, NULL, 1, 0, 0, 0xffffffff, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {ServiceListReset, {{tBool, 0, 0}}, setCompFEQAMServiceListDatabaseReset, getCompFEQAMServiceListDatabaseReset, NULL, NULL},
	{TotalServices, {{tUnsigned, 0, 0}}, NULL, getCompFEQAMServiceListDatabaseTotalServices, NULL, NULL},
    {LogicalChannelNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompFEQAMServiceListDatabaseLogicalChannelNumberOfEntries, NULL, NULL},
	{LogicalChannel, {{tInstance, 0, 0}}, NULL, NULL, LogicalChannelDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode ServiceListDatabaseInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, ServiceListDatabaseDesc, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, ServiceListDatabaseDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompFEQAMInstallInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Start, {{tBool, 0, 0}}, setCompFEQAMInstallStart, getCompFEQAMInstallStart, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {QAMInstallStatus, {{tString, 0, 0}}, NULL, getCompFEQAMInstallStatus, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {Progress, {{tUnsigned, 0, 0}}, NULL, getCompFEQAMInstallProgress, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {StartFrequency, {{tUnsigned, 0, 0}}, setCompFEQAMInstallStartFrequency, getCompFEQAMInstallStartFrequency, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {StopFrequency, {{tUnsigned, 0, 0}}, setCompFEQAMInstallStopFrequency, getCompFEQAMInstallStopFrequency, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Start, {{tBool, 0, 0}}, setCompFEQAMInstallStart, getCompFEQAMInstallStart, NULL, NULL},
	{QAMInstallStatus, {{tString, 0, 0}}, NULL, getCompFEQAMInstallStatus, NULL, NULL},
    {Progress, {{tUnsigned, 0, 0}}, NULL, getCompFEQAMInstallProgress, NULL, NULL},
    {StartFrequency, {{tUnsigned, 0, 0}}, setCompFEQAMInstallStartFrequency, getCompFEQAMInstallStartFrequency, NULL, NULL},
	{StopFrequency, {{tUnsigned, 0, 0}}, setCompFEQAMInstallStopFrequency, getCompFEQAMInstallStopFrequency, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode InstallDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompFEQAMInstallInstanceDesc, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompFEQAMInstallInstanceDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompFEServiceDescInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {CurrentLogicalChannel, {{tString, 0, 0}}, NULL, getCompFEQAMServiceCurrentLogicalChannel, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {CurrentService, {{tString, 0, 0}}, NULL, getCompFEQAMServiceCurrentService, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {CurrentLogicalChannel, {{tString, 0, 0}}, NULL, getCompFEQAMServiceCurrentLogicalChannel, NULL, NULL},
	{CurrentService, {{tString, 0, 0}}, NULL, getCompFEQAMServiceCurrentService, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode ServiceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompFEServiceDescInstanceDesc, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompFEServiceDescInstanceDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};


TRxObjNode QAMModulationInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {QamMode, {{tUnsigned, 0, 0}}, setCompFEQAMModulationQamMode, getCompFEQAMModulationQamMode, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Annex, {{tUnsigned, 0, 0}}, setCompFEQAMModulationAnnex, getCompFEQAMModulationAnnex, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Frequency, {{tUnsigned, 0, 0}}, setCompFEQAMModulationFrequency, getCompFEQAMModulationFrequency, NULL, NULL, 1, 0, 0, 0, NULL, true},
    /*{Bandwidth, {{tUnsigned, 0, 0}}, setCompFEQAMModulationBandwidth, getCompFEQAMModulationBandwidth, NULL, NULL, 1, 0, 0, 0, NULL, true},*/
    {EnablePowerMeasurement, {{tBool, 0, 0}}, setCompFEQAMModulationEnablePowerMeasurement, getCompFEQAMModulationEnablePowerMeasurement, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {SpectrumMode, {{tUnsigned, 0, 0}}, setCompFEQAMModulationSpectrumMode, getCompFEQAMModulationSpectrumMode, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {AcquisitionMode, {{tUnsigned, 0, 0}}, setCompFEQAMModulationAcquisitionMode, getCompFEQAMModulationAcquisitionMode, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ReceiverLock, {{tBool, 0, 0}}, NULL, getCompFEQAMModulationReceiverLock, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {FecLock, {{tBool, 0, 0}}, NULL, getCompFEQAMModulationFECLock, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {OpllLock, {{tBool, 0, 0}}, NULL, getCompFEQAMModulationOpllLock, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {SpectrumInverted, {{tBool, 0, 0}}, setCompFEQAMModulationSpectrumInverted, getCompFEQAMModulationSpectrumInverted, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {SymbolRate, {{tUnsigned, 0, 0}}, setCompFEQAMModulationSymbolRate, getCompFEQAMModulationSymbolRate, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {SymbolRateError, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationSymbolRateError, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {IFAgcLevel, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationIFAgcLevel, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {RFAgcLevel, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationRFAgcLevel, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {IntAgcLevel, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationIntAgcLevel, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {SNREstimate, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationSNREstimate, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {FECCorrected, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationFECCorrected, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {FECUncorrected, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationFECUncorrected, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {FECClean, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationFECClean, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {BitErrCorrected, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationBitErrCorrected, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ReacquireCount, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationReacquireCount, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ViterbiUncorrectedBits, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationViterbiUncorrectedBits, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ViterbiTotalBits, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationViterbiTotalBits, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ViterbiErrorRate, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationViterbiErrorRate, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {ErrorRateUnits, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationErrorRateUnits, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {CarrierFreqOffset, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationCarrierFreqOffset, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {CarrierPhaseOffset, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationCarrierPhaseOffset, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {GoodRsBlockCount, {{tUnsigned, 0, 0}}, NULL, getCompFEQAMModulationGoodRsBlockCount, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {BerRawCount, {{tUnsigned, 0, 0}}, NULL, getCompFEQAMModulationBerRawCount, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {DSChannelPower, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationDSChannelPower, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {MainTap, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationMainTap, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {PostRsBer, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationPostRsBer, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {PostRsBerElapsedTime, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationPostRsBerElapsedTime, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {InterleaveDepth, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationInterleaveDepth, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {LnaAgcLevel, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationLnaAgcLevel, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {EqualizerGain, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationEqualizerGain, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {FrontendGain, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationFrontendGain, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {DigitalAgcGain, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationDigitalAgcGain, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {HighResEqualizerGain, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationHighResEqualizerGain, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{QamMode, {{tUnsigned, 0, 0}}, setCompFEQAMModulationQamMode, getCompFEQAMModulationQamMode, NULL, NULL},
	{Annex, {{tUnsigned, 0, 0}}, setCompFEQAMModulationAnnex, getCompFEQAMModulationAnnex, NULL, NULL},
    {Frequency, {{tUnsigned, 0, 0}}, setCompFEQAMModulationFrequency, getCompFEQAMModulationFrequency, NULL, NULL},
	/*{Bandwidth, {{tUnsigned, 0, 0}}, setCompFEQAMModulationBandwidth, getCompFEQAMModulationBandwidth, NULL, NULL},*/
	{EnablePowerMeasurement, {{tBool, 0, 0}}, setCompFEQAMModulationEnablePowerMeasurement, getCompFEQAMModulationEnablePowerMeasurement, NULL, NULL},
	{SpectrumMode, {{tUnsigned, 0, 0}}, setCompFEQAMModulationSpectrumMode, getCompFEQAMModulationSpectrumMode, NULL, NULL},
	{AcquisitionMode, {{tUnsigned, 0, 0}}, setCompFEQAMModulationAcquisitionMode, getCompFEQAMModulationAcquisitionMode, NULL, NULL},
	{ReceiverLock, {{tBool, 0, 0}}, NULL, getCompFEQAMModulationReceiverLock, NULL, NULL},
	{FecLock, {{tBool, 0, 0}}, NULL, getCompFEQAMModulationFECLock, NULL, NULL},
	{OpllLock, {{tBool, 0, 0}}, NULL, getCompFEQAMModulationOpllLock, NULL, NULL},
	{SpectrumInverted, {{tBool, 0, 0}}, setCompFEQAMModulationSpectrumInverted, getCompFEQAMModulationSpectrumInverted, NULL, NULL},
	{SymbolRate, {{tUnsigned, 0, 0}}, setCompFEQAMModulationSymbolRate, getCompFEQAMModulationSymbolRate, NULL, NULL},
	{SymbolRateError, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationSymbolRateError, NULL, NULL},
	{IFAgcLevel, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationIFAgcLevel, NULL, NULL},
	{RFAgcLevel, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationRFAgcLevel, NULL, NULL},
	{IntAgcLevel, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationIntAgcLevel, NULL, NULL},
	{SNREstimate, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationSNREstimate, NULL, NULL},
	{FECCorrected, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationFECCorrected, NULL, NULL},
	{FECUncorrected, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationFECUncorrected, NULL, NULL},
	{FECClean, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationFECClean, NULL, NULL},
	{BitErrCorrected, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationBitErrCorrected, NULL, NULL},
	{ReacquireCount, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationReacquireCount, NULL, NULL},
	{ViterbiUncorrectedBits, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationViterbiUncorrectedBits, NULL, NULL},
	{ViterbiTotalBits, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationViterbiTotalBits, NULL, NULL},
	{ViterbiErrorRate, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationViterbiErrorRate, NULL, NULL},
	{ErrorRateUnits, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationErrorRateUnits, NULL, NULL},
	{CarrierFreqOffset, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationCarrierFreqOffset, NULL, NULL},
	{CarrierPhaseOffset, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationCarrierPhaseOffset, NULL, NULL},
	{GoodRsBlockCount, {{tUnsigned, 0, 0}}, NULL, getCompFEQAMModulationGoodRsBlockCount, NULL, NULL},
	{BerRawCount, {{tUnsigned, 0, 0}}, NULL, getCompFEQAMModulationBerRawCount, NULL, NULL},
	{DSChannelPower, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationDSChannelPower, NULL, NULL},
	{MainTap, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationMainTap, NULL, NULL},
	{PostRsBer, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationPostRsBer, NULL, NULL},
	{PostRsBerElapsedTime, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationPostRsBerElapsedTime, NULL, NULL},
	{InterleaveDepth, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationInterleaveDepth, NULL, NULL},
	{LnaAgcLevel, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationLnaAgcLevel, NULL, NULL},
	{EqualizerGain, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationEqualizerGain, NULL, NULL},
	{FrontendGain, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationFrontendGain, NULL, NULL},
	{DigitalAgcGain, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationDigitalAgcGain, NULL, NULL},
	{HighResEqualizerGain, {{tInt, 0, 0}}, NULL, getCompFEQAMModulationHighResEqualizerGain, NULL, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode ModulationDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, QAMModulationInstanceDesc, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, QAMModulationInstanceDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};


TRxObjNode CompFEQAMInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Modulation, {{tObject, 0, 0}}, NULL, NULL, ModulationDesc, NULL, 1, 0, 0, 0, NULL, true},
    {QAMService, {{tObject, 0, 0}}, NULL, NULL, ServiceDesc, NULL, 1, 0, 0, 0, NULL, false},
    {Install, {{tObject, 0, 0}}, NULL, NULL, InstallDesc, NULL, 1, 0, 0, 0, NULL, false},
    {ServiceListDatabase, {{tObject, 0, 0}}, NULL, NULL, ServiceListDatabaseDesc, NULL, 1, 0, 0, 0, NULL, false},
    {LogicalChannelConnect, {{tObject, 0, 0}}, NULL, NULL, LogicalChannelConnectDesc, NULL, 1, 0, 0, 0, NULL, false},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Modulation, {{tObject, 0, 0}}, NULL, NULL, ModulationDesc, NULL},
	{QAMService, {{tObject, 0, 0}}, NULL, NULL, ServiceDesc, NULL},
    {Install, {{tObject, 0, 0}}, NULL, NULL, InstallDesc, NULL},
	{ServiceListDatabase, {{tObject, 0, 0}}, NULL, NULL, ServiceListDatabaseDesc, NULL},
    {LogicalChannelConnect, {{tObject, 0, 0}}, NULL, NULL, LogicalChannelConnectDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompFEQAMDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompFEQAMInstanceDesc, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompFEQAMInstanceDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompFEDesc[] = {
#ifdef XML_DOC_SUPPORT
    {Enable, {{tBool, 0, 0}}, setCompFEEnable, getCompFEEnable, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Status, {{tString, 32, 0}}, NULL, getCompFEStatus, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {Alias, {{tString, 64, 0}}, setCompFEAlias, getCompFEAlias, NULL, NULL, 1, 2, 0, 0, NULL, true},
    {Name, {{tString, 256, 0}}, NULL, getCompFEName, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {IP, {{tObject, 0, 0}}, NULL, NULL, CompFEIPDesc, NULL, 1, 0, 0, 0, NULL, true},
    {X_BROADCOM_COM_QAM, {{tInstance, 0, 0}}, NULL, NULL, CompFEQAMDesc, NULL, 1, 0, 0, 0xffffffff, NULL, true},
    {NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {Enable, {{tBool, 0, 0}}, setCompFEEnable, getCompFEEnable, NULL, NULL},
    {Status, {{tString, 32, 0}}, NULL, getCompFEStatus, NULL, NULL},
	{Alias, {{tString, 64, 0}}, setCompFEAlias, getCompFEAlias, NULL, NULL},
    {Name, {{tString, 256, 0}}, NULL, getCompFEName, NULL, NULL},
    {IP, {{tObject, 0, 0}}, NULL, NULL, CompFEIPDesc, NULL},
	{X_BROADCOM_COM_QAM, {{tInstance, 0, 0}}, NULL, NULL, CompFEQAMDesc, NULL},
	{NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode CompFEInstanceDesc[] = {
#ifdef XML_DOC_SUPPORT
    {instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompFEDesc, NULL, 1, 0, 0, 0, NULL, true},
    {NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
	{instanceIDMASK,{{0, 0, 0}}, NULL, NULL, CompFEDesc, NULL},
	{NULL, {{0, 0, 0}}, NULL, NULL, NULL, NULL}
#endif
};

TRxObjNode ComponentsDesc[] = {
#ifdef XML_DOC_SUPPORT
    {FrontEndNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompFENumberOfEntries, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {AudioDecoderNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompAudioDecoderNumberOfEntries, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {VideoDecoderNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompVideoDecoderNumberOfEntries, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {AudioOutputNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompAudioOutputNumberOfEntries, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {VideoOutputNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompVideoOutputNumberOfEntries, NULL, NULL, 1, 0, 0, 0, NULL, true},
    {SCARTNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompSCARTNumberOfEntries, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {CANumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompCANNumberOfEntries, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {DRMNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompDRMNumberOfEntries, NULL, NULL, 1, 0, 0, 0, NULL, false},
    {HDMINumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompHDMINumberOfEntries, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {SPDIFNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompSPDIFNumberOfEntries, NULL, NULL, 1, 1, 0, 0, NULL, true},
    {FrontEnd, {{tInstance, 0, 0}}, NULL, NULL, CompFEInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, false},
    {PVR, {{tObject, 0, 0}}, NULL, NULL, CompPVRDesc, NULL, 1, 0, 0, 0, NULL, true},
    {AudioDecoder, {{tInstance, 0, 0}}, NULL, NULL, CompAudioDecoderInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, true},
    {VideoDecoder, {{tInstance, 0, 0}}, NULL, NULL, CompVideoDecoderInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, true},
    {AudioOutput, {{tInstance, 0, 0}}, NULL, NULL, CompAudioOutputInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, true},
    {SPDIF, {{tInstance, 0, 0}}, NULL, NULL, CompSPDIFInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, true},
    {VideoOutput, {{tInstance, 0, 0}}, NULL, NULL, CompVideoOutputInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, true},
    {SCART, {{tInstance, 0, 0}}, NULL, NULL, CompSCARTInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, false},
    {HDMI, {{tInstance, 0, 0}}, NULL, NULL, CompHDMIInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, true},
    {CA, {{tInstance, 0, 0}}, NULL, NULL, CompCAInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, false},
    {DRM, {{tInstance, 0, 0}}, NULL, NULL, CompDRMInstanceDesc, NULL, 1, 0, 0, 0xffffffff, NULL, false},
    {NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL, 0, 0, 0, 0, NULL, false}
#else
    {FrontEndNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompFENumberOfEntries, NULL, NULL},
    {AudioDecoderNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompAudioDecoderNumberOfEntries, NULL, NULL},
    {VideoDecoderNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompVideoDecoderNumberOfEntries, NULL, NULL},
    {AudioOutputNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompAudioOutputNumberOfEntries, NULL, NULL},
    {VideoOutputNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompVideoOutputNumberOfEntries, NULL, NULL},
    {SCARTNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompSCARTNumberOfEntries, NULL, NULL},
    {CANumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompCANNumberOfEntries, NULL, NULL},
    {DRMNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompDRMNumberOfEntries, NULL, NULL},
	{HDMINumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompHDMINumberOfEntries, NULL, NULL},
	{SPDIFNumberOfEntries, {{tUnsigned, 0, 0}}, NULL, getCompSPDIFNumberOfEntries, NULL, NULL},
    {FrontEnd, {{tInstance, 0, 0}}, NULL, NULL, CompFEInstanceDesc, NULL},
    {PVR, {{tObject, 0, 0}}, NULL, NULL, CompPVRDesc, NULL},
    {AudioDecoder, {{tInstance, 0, 0}}, NULL, NULL, CompAudioDecoderInstanceDesc, NULL},
    {VideoDecoder, {{tInstance, 0, 0}}, NULL, NULL, CompVideoDecoderInstanceDesc, NULL},
    {AudioOutput, {{tInstance, 0, 0}}, NULL, NULL, CompAudioOutputInstanceDesc, NULL},
	{SPDIF, {{tInstance, 0, 0}}, NULL, NULL, CompSPDIFInstanceDesc, NULL},
    {VideoOutput, {{tInstance, 0, 0}}, NULL, NULL, CompVideoOutputInstanceDesc, NULL},
    {SCART, {{tInstance, 0, 0}}, NULL, NULL, CompSCARTInstanceDesc, NULL},
	{HDMI, {{tInstance, 0, 0}}, NULL, NULL, CompHDMIInstanceDesc, NULL},
    {CA, {{tInstance, 0, 0}}, NULL, NULL, CompCAInstanceDesc, NULL},
    {DRM, {{tInstance, 0, 0}}, NULL, NULL, CompDRMInstanceDesc, NULL},
    {NULL, {{0, 0, 0}},NULL, NULL, NULL, NULL}
#endif
};



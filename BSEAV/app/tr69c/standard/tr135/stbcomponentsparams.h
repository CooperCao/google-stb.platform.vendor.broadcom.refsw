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

#ifndef __STB_COMPONENTS_PARAMS_H__
#define __STB_COMPONENTS_PARAMS_H__

/*
* The CPE parameters are represented by a tree where each node in the
 * tree is an array of TRxObjNode structures. Each array item represents
 * either a paramater item or an object containing a pointer to the next
 * level of TRxObjNode items that make up  parameters that make up the object.
 *
 * Each item that describes an object contains a pointer component, *objDetail,
 * that points to the array of parameter and object items that form the
 * object.
 * The function pointer, getTRxParam, may be NULL or point to
 * a hardware dependent function that returns the value of the parameter in
 * a string variable.
 * The function pointer, setTRxParam, is used to call a hardware dependent
 * function to set the parameter. If the function pointer is NULL the
 * parameter is not writeable. If the item is an object and the pointer is
 * NULL the rpc function AddObject/DeleteObject are not supported by that
 * item.
 *
 * The global variable thisTRxObjNode points to the current TRxObjNode entry
 * when the set/getTRxParam functions are called.
 *
 * If the node contains a single TRxObjNode item with TRxType of tInstance
 * then this item represents all instances of the object. This function is
 * responsible for keeping track of all instances associated with this
 * object. The parameter handling framework expects the following functionality
 * of the getTRxParam function which will need to maintain state information
 * as the framework accesses the instances. The framework will always call
 * the getTRxParam to access an instance on its way to access its
 * sub-objects/parameters. As the framework is traversing the parameter
 * tree it will call the getTRxParam function with one of the following
 * forms to validate and set the instance state before proceeding
 * to the next object/parameter level.
 *
 * To determine the existance of a specific instance:
 *  The getTRxParam function is called as follows:
 *
 *  node->getTRXParam(char **ParamInstance)
 *  If *ParamInstance is not NULL then it points to the value to be found.
 *  Returns: TRx_OK if ParameterInstance found. The function should set
 *                  a global state variable for use by the next level
 *                  get/setTRxParam functions to the ParameterInstance.
 *           TRx_ERR if ParameterInstance not found
 *
 *  To retrieve each of the instances in order:
 *  If *ParamInstance is NULL then it returns the first instance of the
 *  object.
 *  Returns: TRx_OK if any Instances exist. The *ParamInstance pointer points to the
 *                  name (instance number string) of the first instance.
 *                  The global instance state variable is set to the
 *                  instance returned in the value string.
 *           TRx_ERR no instances of  this object exist.
 *
 *  If *ParamInstance is (void *)(-1) then find the next instance relative
 * to the last instance returned.
 *  Returns: TRx_OK The *ParamInstance pointer points to the next instance.
 *                  instance name. Repeated calls with the returned
 *                  instance name from the previous call as the InstanceValue
 *                  will return all instances. The global instance state
 *                  variable is the instance returned in the value string.
 *          TRx_ERR no more instances.
 * See xxx for an example of how this is coded.
 */

#include "../inc/tr69cdefs.h"

/* STBService.Components.DRM Object */
/* SVAR(Enable); */
/* SVAR(Status); */
/* SVAR(Alias); */
/* SVAR(Name); */
/* SVAR(SmartCardReader); */

/* STBService.Components.CA Object */
/* SVAR(Enable); */
/* SVAR(Status); */
/* SVAR(Alias); */
/* SVAR(Name); */
SVAR(SmartCardReader);

/* STBService.Components.HDMI.DisplayDevice Object */
/* SVAR(Status); */
/* SVAR(Name); */
SVAR(EEDID);
/* SVAR(SupportedResolutions); */
SVAR(PreferredResolution);
SVAR(VideoLatency);
/* SVAR(CECSupport); */
SVAR(AutoLipSyncSupport);
SVAR(HDMI3DPresent);

/* STBService.Components.HDMI Object */
/* SVAR(Enable); */
/* SVAR(Status); */
/* SVAR(Alias); */
/* SVAR(Name); */
SVAR(ResolutionMode);
SVAR(ResolutionValue);
SVAR(DisplayDevice);

/* STBService.Components.SCART Object */
/* SVAR(Enable); */
/* SVAR(Status); */
/* SVAR(Alias); */
/* SVAR(Name); */
SVAR(Presence);

/* STBService.Components.VideoOutput Object */
/* SVAR(Enable); */
/* SVAR(Status); */
/* SVAR(Alias); */
SVAR(ColorbarEnable);
/* SVAR(Name); */
SVAR(CompositeVideoStandard);
SVAR(VideoFormat);
SVAR(AspectRatioBehaviour);
SVAR(DisplayFormat);
/* SVAR(Macrovision); */
/* SVAR(HDCP); */
SVAR(SCARTs);

/* STBService.Components.SPDIF Object */
/* SVAR(Enable); */
/* SVAR(Status); */
/* SVAR(Alias); */
/* SVAR(Name); */
SVAR(ForcePCM);
SVAR(Passthrough);
SVAR(AudioDelay);

/* STBService.Components.AudioOutput Object */
/* SVAR(Enable); */
/* SVAR(Status); */
/* SVAR(Alias); */
/* SVAR(Name); */
SVAR(AudioFormat);
SVAR(AudioLevel);
SVAR(CancelMute);
/* SVAR(SCARTs); */

/* STBService.Components.VideoDecoder Object */
/* SVAR(Enable); */
/* SVAR(Status); */
/* SVAR(Alias); */
/* SVAR(Name); */
/* SVAR(MPEG2Part2); */
/* SVAR(MPEG4Part2); */
/* SVAR(MPEG4Part10); */
/* SVAR(SMPTEVC1); */
SVAR(ContentAspectRatio);

/* STBService.Components.AudioDecoder Object */
/* SVAR(Enable); */
/* SVAR(Status); */
/* SVAR(Alias); */
/* SVAR(Name); */
SVAR(AudioStandard);

/* STBService.Components.PVR.Storage Object */
/* SVAR(Alias); */
/* SVAR(Name); */
/* SVAR(Reference); */

/* STBService.Components.PVR Object */
SVAR(StorageNumberOfEntries);
SVAR(Storage);

/* STBService.Components.FrontEnd.IP.Dejittering Object */
SVAR(BufferSize);
SVAR(BufferInitialLevel);

/* STBService.Components.FrontEnd.IP.Outbound Object */
/* SVAR(Status); */
/* SVAR(Alias); */
/* SVAR(MultiplexType); */
/* SVAR(URI); */

/* STBService.Components.FrontEnd.IP.Inbound Object */
/* SVAR(Status); */
/* SVAR(Alias); */
SVAR(StreamingControlProtocol);
SVAR(StreamingTransportProtocol);
SVAR(StreamingTransportControlProtocol);
/* SVAR(MultiplexType); */
SVAR(DownloadTransportProtocol);
SVAR(SourceAddress);
SVAR(SourcePort);
/* SVAR(DestinationAddress); */
SVAR(DestinationPort);
/* SVAR(URI); */

/* STBService.Components.FrontEnd.IP.IGMP.ClientGroupStats.QuarterHour Object */
/* SVAR(NumberOfJoins); */
/* SSVAR(NumberOfLeaves); */
/* SSVAR(MaxJoinDelay); */

/* STBService.Components.FrontEnd.IP.IGMP.ClientGroupStats.CurrentDay Object */
/* SVAR(NumberOfJoins); */
/* SSVAR(NumberOfLeaves); */
/* SSVAR(MaxJoinDelay); */

/* STBService.Components.FrontEnd.IP.IGMP.ClientGroupStats.Total Object */
/* SVAR(NumberOfJoins); */
/* SSVAR(NumberOfLeaves); */
/* SSVAR(MaxJoinDelay); */

/* STBService.Components.FrontEnd.IP.IGMP.ClientGroupStats Object */
/* SVAR(Alias); */
SVAR(Address);
/* SVAR(TotalStart); */
/* SVAR(CurrentDayStart); */
SVAR(QuartHourStart);
/* SVAR(Total); */
/* SVAR(CurrentDay); */
/* SVAR(QuarterHour); */

/* STBService.Components.FrontEnd.IP.IGMP.ClientGroup Object */
/* SVAR(Alias); */
SVAR(GroupAddress);
/* SVAR(UpTime); */

/* STBService.Components.FrontEnd.IP.IGMP Object */
/* SVAR(Enable); */
/* SVAR(Status); */
SVAR(MaximumNumberOfConcurrentGroups);
SVAR(MaximumNumberOfTrackedGroups);
SVAR(LoggingEnable);
SVAR(DSCPMark);
SVAR(VLANIDMark);
SVAR(EthernetPriorityMark);
SVAR(ClientVersion);
SVAR(ClientRobustness);
SVAR(ClientUnsolicitedReportInterval);
SVAR(ClientGroupNumberOfEntries);
SVAR(ClientGroupStatsNumberOfEntries);
SVAR(ClientGroup);
SVAR(ClientGroupStats);

/* STBService.Components.FrontEnd.IP Object */
SVAR(InboundNumberOfEntries);
SVAR(OutboundNumberOfEntries);
SVAR(ActiveInboundIPStreams);
SVAR(ActiveOutboundIPStreams);
SVAR(IGMP);
SVAR(Dejittering);
SVAR(ServiceConnect);

/* SVAR(Inbound); */
/* SVAR(OutBound); */

/* STBService.Components.FrontEnd Object */
/* SVAR(Enable); */
/* SVAR(Status); */
/* SVAR(Alias); */
/* SVAR(Name); */
/* SVAR(IP); */

SVAR(Locked);
SVAR(CBER);
SVAR(BER);
SVAR(SNR);
SVAR(TransmissionMode);
SVAR(HierarchicalInformation);
SVAR(Constellation);
SVAR(ChannelBandwidth);
/*SVAR(Frequency);*/
SVAR(CurrentService);
SVAR(CurrentLogicalChannel);
SVAR(StopFrequency);
SVAR(StartFrequency);
SVAR(QAMInstallProgress);
SVAR(QAMInstallStatus);
SVAR(Start);
SVAR(LogicalChannelNumberOfEntries);
SVAR(Progress);
SVAR(TotalServices);
SVAR(ServiceListReset);
SVAR(ServiceNumberOfEntries);
SVAR(Preferred);
SVAR(LogicalChannelNumber);
/* STBService.Components Object */
SVAR(FrontEndNumberOfEntries);
SVAR(AudioDecoderNumberOfEntries);
SVAR(VideoDecoderNumberOfEntries);
SVAR(AudioOutputNumberOfEntries);
SVAR(VideoOutputNumberOfEntries);
SVAR(SCARTNumberOfEntries);
SVAR(CANumberOfEntries);
SVAR(DRMNumberOfEntries);
SVAR(HDMINumberOfEntries);
SVAR(SPDIFNumberOfEntries);
/* SVAR(FrontEnd); */
/* SVAR(PVR); */
/* SVAR(AudioDecoder); */
/* SVAR(VideoDecoder); */
/* SVAR(AudioOutput); */
SVAR(SPDIF);
/* SVAR(VideoOutput); */
/* SVAR(SCART); */
/* SVAR(HDMI); */
/* SVAR(CA); */
/* SVAR(DRM); */

/* QAM.Modulation */
SVAR(QamMode);
SVAR(Annex);
SVAR(Frequency);
SVAR(Bandwidth);
SVAR(EnablePowerMeasurement);
SVAR(SpectrumMode);
SVAR(AcquisitionMode);
SVAR(ReceiverLock);
SVAR(FecLock);
SVAR(OpllLock);
SVAR(SpectrumInverted);
SVAR(SymbolRate);
SVAR(SymbolRateError);
SVAR(IFAgcLevel);
SVAR(RFAgcLevel);
SVAR(IntAgcLevel);
SVAR(SNREstimate);
SVAR(FECCorrected);
SVAR(FECUncorrected);
SVAR(FECClean);
SVAR(BitErrCorrected);
SVAR(ReacquireCount);
SVAR(ViterbiUncorrectedBits);
SVAR(ViterbiTotalBits);
SVAR(ViterbiErrorRate);
SVAR(ErrorRateUnits);
SVAR(CarrierFreqOffset);
SVAR(CarrierPhaseOffset);
SVAR(GoodRsBlockCount);
SVAR(BerRawCount);
SVAR(DSChannelPower);
SVAR(MainTap);
SVAR(PostRsBer);
SVAR(PostRsBerElapsedTime);
SVAR(InterleaveDepth);
SVAR(LnaAgcLevel);
SVAR(EqualizerGain);
SVAR(FrontendGain);
SVAR(DigitalAgcGain);
SVAR(HighResEqualizerGain);


#endif   /* __STB_COMPONENTS_PARAMS_H__ */

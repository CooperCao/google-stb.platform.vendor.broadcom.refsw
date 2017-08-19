/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *   See Module Overview below.
 *
  ***************************************************************************/
/*=************************ Module Overview ********************************
<verbatim>
Overview

The XVD (X Video Decoder) API is a library used to control the advanced video
decoder on integrated chipsets using the AVD core. It has two primary
functions:

 - Initialization and control of the AVD core to decode data from the
   compressed data buffers.

 - Managing picture delivery to VDC for display.  An internal module called
   the "display manager" is responsible for TSM and picture delivery.

There is a single instance of XVD instantiated per hardware decoder in the
system. An instance is created by a call to BXVD_Open that returns a
BXVD_Handle.  The BXVD_Handle refers to an opaque structure that is used by
other API routines to maintain the state of XVD. An decode channel is created
and associated with a decoder instance by a call to BXVD_OpenChannel.  In
general, a channel encapsulates the state required to perform a decode.

The call to BXVD_OpenChannel will return a BXVD_ChannelHandle which is used in
subsequent API calls that operate on a per channel basis which actually
constitute the majority of the XVD API for live decode. See the definitions of
BXVD_Handle and BXVD_ChannelHandle for a description of which calls use each of
these different handles.

XVD supports multiple channels to be opened on a single decoder.  While the
API supports a maximum of 16 channels, in practice the number of channels
will be limited by the size of the decodes being performed.  The XVD API allows
flexibility in the creation of channels, so that the number of channels and
the amount of memory which channels consume is largely under the control of
the application when calling BXVD_ChannelOpen.

XVD has the ability to support up to 4 different memory pools when BXVD_Open is
called.  Whether all of these pools are required is dependent on the
application and the system.  Basic applications running on unified memory
systems (UMA) will not have to be concerned with the different memory pools.
In addition, the application may specify memory on a per channel basis via
BXVD_OpenChannel.

The 4 pools of memory XVD uses are 1.) firmware code, 2.) general heap,
3.) secure or CABAC heap, and 4.) picture heap.  The firmware code heap
is only used when the decoder firmware has been relocated off-line to a
fixed location.  BXVD_Open accepts a hXvdFmHeap handle to indicate the location
of the firmware; the heap must be defined to exactly match the relocated code
for all systems before AVD Rev K (7422).  For later systems, the relocation logic
has been changed and this retriction does not apply.
Notes: a.) the general heap and firmware code heap (if specified) need to be accessible
by the host processor. b.) applications need to be aware of the system implications
of heap placement.  For example, a particular platfrom may restrict certain AVD
clients to a particular memory controller in hardware or via RTS.  Trying to
configure XVD without an understanding of these restrictions may result in unpredictable
behaviors. c.) The first 2 MB of system address space cannot be accessed by the AVD.

If the target system is configured with dedicated picture buffers, then the
application is required to create a heap for the picture buffers and provide
that heap to XVD.  In unified memory systems, it is permitted to provide a
picture buffer heap, but if none is provided XVD will attempt to allocate
picture buffers from the general heap.  The heaps are specified at BXVD_Open,
in the formal arguement list and in the BXVD_Settings structure.

The secure or CABAC heap is only required to be specified when the target
system has enhanced security enabled.  Like the picture buffer heap, the
application may provide a secure heap, however, it is not recommended to
specify a secure heap unless enhanced security is enabled.

The general heap is used for context and other storage on a per channel basis.
The application must specify a general heap to provide working storage for the
firmware.  This heap will also be used when the application has not specified
picture or secure heaps.

The application is responsible for calculating the sizes of the various heaps
and providing those sizes to XVD by filling in the xvdSettings.stFWMemConfig
structure at BXVD_Open.  The size of the various members will vary, depending
on the kind and number of channels the application will be using and the memory
characteristics of the target system.  The sChannelFWMemCfg and sVideoAtomSize
structures defined in bxvd_priv.c reflect the memory requirements for currently
defined configurations.  XVD will sub-allocate and free the memory provided to
BXVD_Open on a LIFO basis when BXVD_OpenChannel and BXVD_CloseChannel are
called.

It should be noted here that the term channel as used by XVD does not refer
to a broadcast program channel, but to a path within XVD that exists from a
RAVE (transport controller) context through the decoder. A channel is normally
associated with a single VDC (Video Display Controller) source.

On many platforms, XVD supports two independent display interrupts to allow
for two VDC sources to be assigned for display.  The use of two VDC sources
requires the configuration of a complete display path from VDC through XVD.
The application is required to install device callbacks for each display
interrupt/VDC source being used.  Also, in most cases where two display
interrupts are employed, the application will have to associate the appropriate
STC with a channel.  The association of display interrupt and STC is made at
BXVD_StartDecode.  The the default settings support single display
interrupt operation without additional configuration.

A fairly simple example of XVD usage is presented here. It will cover the
opening, closing and usage of an XVD and XVD channel instance. It is assumed
that all necessary calls to other porting interface APIs have been made and
all necessary memory has been allocated by a higher level application.
Please refer to the individual API call's documentation for more details.

void main()
{
   XVD_Handle hXvd;
   BXVD_Settings xvdSettings;
   BXVD_ChannelHandle hXvdChannel[NUM_XVD_CHANNELS];
   BXVD_ChannelSettings xvdChannelSettings;
   BAVC_VideoCompressionStd stVideoCompressionList[NUM_XVD_CHANNELS][1];
   BXVD_DecodeSettings xvdDecodeSettings;

   BINT_Id  TopInterruptName;
   BINT_Id  BotInterruptName;
   BINT_Id  ProgressiveInterruptName;

   BXVD_DeviceVdcInterruptSettings VDCDevIntrSettings;
   BXVD_DeviceInterrupt eXvdDeviceInterrupt;

   BXVD_DecodeMode aeDecodeModes[] =
   {
           BXVD_DecodeMode_eCustom,
           BXVD_DecodeMode_eTerminateList
   };

   ********************************************************************

   Open XVD - usage of xvdSettings.stFWMemConfig is discussed above

   ********************************************************************

   BXVD_GetDefaultSettings(&xvdSettings);
   xvdSettings.pDecodeModes = aeDecodeModes;
   xvdSettings.stFWMemConfig.uiGeneralHeapSize = WORST_CASE_GENERAL_HEAP_SIZE;
   xvdSettings.stFWMemConfig.uiCabacHeapSize = WORST_CASE_CABAC_HEAP_SIZE;
   xvdSettings.stFWMemConfig.uiPictureHeapSize = WORST_CASE_PICTURE_HEAP_SIZE;

   BXVD_Open(&hXvd,
      hChp,
      hReg,
      NULL,
      hMem,
      hInt,
      &xvdSettings);

   ...

   ***************************************

   Configure callback interface to VDC

   ***************************************

   BVDC_Source_GetInterruptName(
      hVdcSource,
      BAVC_Polarity_eTopField,
      &TopInterruptName);

   BVDC_Source_GetInterruptName(
      hVdcSource,
      BAVC_Polarity_eBotField,
      &BotInterruptName);

   BVDC_Source_GetInterruptName(
      hVdcSource,
      BAVC_Polarity_eFrame,
      &ProgressiveInterruptName);

   VDCDevIntrSettings.VDCIntId_Topfield = TopInterruptName;
   VDCDevIntrSettings.VDCIntId_Botfield = BotInterruptName;
   VDCDevIntrSettings.VDCIntId_Frame = ProgressiveInterruptName;

   VDCDevIntrSettings.eDisplayInterrupt = BXVD_DisplayInterrupt_eZero;
   VDCDevIntrSettings.uiFlags = BXVD_DeviceVdcIntrSettingsFlags_None;

   BXVD_RegisterVdcDeviceInterrupt(hXvd,
      &VDCDevIntrSettings);

   ************************************

   Install MPEG data ready callback

   ************************************

   BXVD_InstallDeviceInterruptCallback(
      hCheckXvdInfo->hXvd,
      BXVD_DisplayInterrupt_eZero,
      BVDC_Source_MpegDataReady_isr,
      (void *)hVdcSource,
      0);

   *********************

   Open XVD channels

   *********************

   for (uiChannelNum = 0; uiChannelNum < NUM_XVD_CHANNELS; uiChannelNum++)
   {
           BXVD_GetChannelDefaultSettings(hXvd,
              uiChannelNum,
              &xvdChannelSettings);

           * The following code sets up the following channels:
           Channel 0: MPEG 2 HD Still
           Channel 1: AVC SD Video
           Channel 2: AVD HD Video
           *
           switch(uiChannelNum)
           {
                   case 0: * MPEG 2 HD Still *
                   xvdChannelSettings.eDecodeResolution = BXVD_DecodeResolution_eHD;

                   stVideoCompressionList[uiChannelNum][0] = BAVC_VideoCompressionStd_eMPEG2;
                   xvdChannelSettings.peVideoCmprStdList = stVideoCompressionList[uiChannelNum];
                   xvdChannelSettings.uiVideoCmprCount = 1;

                   xvdChannelSettings.eChannelMode = BXVD_ChannelMode_eStill;
                   break;

                   case 1: * AVC SD Video *
                   xvdChannelSettings.eDecodeResolution = BXVD_DecodeResolution_eSD;

                   stVideoCompressionList[uiChannelNum][0] = BAVC_VideoCompressionStd_eH264;
                   xvdChannelSettings.peVideoCmprStdList = stVideoCompressionList[uiChannelNum];
                   xvdChannelSettings.uiVideoCmprCount = 1;

                   xvdChannelSettings.eChannelMode = BXVD_ChannelMode_eVideo;
                   break;

                   case 2: * AVC HD Video *
                   xvdChannelSettings.eDecodeResolution = BXVD_DecodeResolution_eHD;

                   stVideoCompressionList[uiChannelNum][0] = BAVC_VideoCompressionStd_eH264;
                   xvdChannelSettings.peVideoCmprStdList = stVideoCompressionList[uiChannelNum];
                   xvdChannelSettings.uiVideoCmprCount = 1;

                   xvdChannelSettings.eChannelMode = BXVD_ChannelMode_eVideo;
                   break;

                   default:
                   BDBG_ERR("Error: unsupported channel");
                   return;
           }

           BXVD_OpenChannel(hXvd,
              &hXvdChannel[uiChannelNum],
              uiChannelNum,
              &xvdChannelSettings);
   }

   * ... *

   ****************************************************************

   Start decodes Note, RAVE contexts would be acquired from XPT

   ****************************************************************

   for (uiChannelNum = 0; uiChannelNum < NUM_XVD_CHANNELS; uiChannelNum++)
   {
           BXVD_GetDecodeDefaultSettings(hXvd, &xvdDecodeSettings);

           switch(uiChannelNum)
           {
                   case 0:
                   xvdDecodeSettings.eVideoCmprStd = BAVC_VideoCompressionStd_eMPEG2;
                   xvdDecodeSettings.pContextMap = CHANNEL_0_XPT_RAVE_CONTEXT_MAP;
                   break;

                   case 1:
                   xvdDecodeSettings.eVideoCmprStd = BAVC_VideoCompressionStd_eH264;
                   xvdDecodeSettings.pContextMap = CHANNEL_1_XPT_RAVE_CONTEXT_MAP;
                   break;

                   case 1:
                   xvdDecodeSettings.eVideoCmprStd = BAVC_VideoCompressionStd_eH264;
                   xvdDecodeSettings.pContextMap = CHANNEL_2_XPT_RAVE_CONTEXT_MAP;
                   break;

                   default:
                   BDBG_ERR("Error: unsupported channel");
                   return;
           }

           BXVD_StartDecode(hXvdChannel[uiChannelNum], &xvdDecodeSettings);
   }


   * ... *

}
</verbatim>
****************************************************************************/
#ifndef BXVD_H__
#define BXVD_H__

#include "bchp.h"          /* Chip information */
#include "bmma.h"          /* New Chip memory access. */
#include "breg_mem.h"      /* Chip register access */
#include "bfmt.h"          /* Video timing format */
#include "bavc.h"          /* Analog Video Common */
#include "bint.h"          /* Interrupt access */
#include "bxvd_errors.h"   /* XVD defined errors */
#include "bimg.h"          /* FW image interface */
#include "btmr.h"          /* Timer access */

#include "bxdm_pp.h"
#include "bxdm_picture.h"
#include "bxdm_dih.h"
/*SW7425-1064: */
#include "bxdm_pp_xmo.h"

#if BXVD_P_AVD_ARC600
#include "bafl.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
Summary:
    Main XVD Context Handle. This handle represents a single instance of a
 hardware decoder. BXVD_Open returns an initialized BXVD_Handle that is
 used in subsequent calls to the API.

See also:
 BXVD_Open
 BXVD_Close
 BXVD_GetChannelDefaultSettings
 BXVD_ProcessWatchdog
 BXVD_OpenChannel
 BXVD_GetTotalChannels
 BXVD_GetRevision
 BXVD_InstallDeviceInterruptCallback
 BXVD_UnInstallDeviceInterruptCallback
 BXVD_GetBufferConfig
*****************************************************************************/
typedef struct BXVD_P_Context *BXVD_Handle;

/***************************************************************************
Summary:
    XVD channel handle. The channel handle represents a logical decode path
 from RAVE (XPT) through to VDC via the Display Mananger (DM).

See also:
  BXVD_ChannelOpen
  BXVD_ChannelClose
  BXVD_StartDecode
  BXVD_StopDecode
  BXVD_GetChannelStatus
  BXVD_DisableForFlush
  BXVD_FlushDecode
  BXVD_GetDecodeSettings
  BXVD_EnableMute
  BXVD_SetDisplayOffset
  BXVD_GetDisplayOffset
  BXVD_SetVideoDisplayMode
  BXVD_GetVideoDisplayMode
  BXVD_GetPTS_isr
  BXVD_GetPTS
  BXVD_SetDisplayFieldMode
  BXVD_GetDisplayFieldMode
  BXVD_RegisterVdcInterrupt
  BXVD_InstallInterruptCallbac
  BXVD_UnInstallInterruptCallback
  BXVD_SetChannelChangeMode
  BXVD_GetChannelChangeMode
  BXVD_GetIPictureFoundStatus
  BXVD_SetTSMPassWindow
  BXVD_DisplayUntoPTS
  BXVD_PauseUntoPTS
  BXVD_SetInterpolationModeForStillContent
  BXVD_GetInterpolationModeForStillContent
  BXVD_SetInterpolationModeForMovingContent
  BXVD_GetInterpolationModeForMovingContent
  BXVD_GetLastCodedPTS
  BXVD_GetDisplayThresholds
  BXVD_SetDiscardThreshold
  BXVD_DecodeStillPicture
  BXVD_SetSkipPictureModeDecode
  BXVD_GetSkipPictureModeConfig
  BXVD_SetSTCInvalidFlag_isr
  BXVD_GetSTCInvalidFlag
  BXVD_SetMonitorRefreshRate
  BXVD_GetMonitorRefreshRate
  BXVD_EnableVideoFreeze
  BXVD_DisableVideoFreeze
  BXVD_GetVideoFreezeState
  BXVD_SetPtsStcDiffThreshold
  BXVD_GetPtsStcDiffThreshold
  BXVD_GetPictureTag
  BXVD_SetClipTime
  BXVD_SetPtsErrorReportLowerThreshold
  BXVD_GetPtsErrorReportLowerThreshold
  BXVD_SetPtsErrorReportUpperThreshold
  BXVD_GetPtsErrorReportUpperThreshold
  BXVD_EnableInterrupt

**************************************************************************/
typedef struct BXVD_P_Channel *BXVD_ChannelHandle;

#include "bxvd_userdata.h"

/*************************************************************************
Summary:
 Used to specify the still picture decode mode.

See also:
 BXVD_DecodeStillPicture.
**************************************************************************/
typedef enum BXVD_DecodeStillMode
{
   BXVD_DecodeModeStill_eTerminateList = 0,  /* used to mark the end of list */
   BXVD_DecodeModeStill_eAll,
   BXVD_DecodeModeStill_eMPEG_SD,
   BXVD_DecodeModeStill_eMPEG_HD,
   BXVD_DecodeModeStill_eAVC_SD,
   BXVD_DecodeModeStill_eAVC_HD,
   BXVD_DecodeModeStill_eVC1_SD,
   BXVD_DecodeModeStill_eVC1_HD,
   BXVD_DecodeModeStill_eMPEG4Part2_SD,
   BXVD_DecodeModeStill_eMPEG4Part2_HD,
   BXVD_DecodeModeStill_eVP8_SD,
   BXVD_DecodeModeStill_eVP8_HD,
   BXVD_DecodeModeStill_eHEVC_SD,
   BXVD_DecodeModeStill_eHEVC_HD,
   BXVD_DecodeModeStill_eHEVC_4K,
   BXVD_DecodeModeStill_eVP9_SD,
   BXVD_DecodeModeStill_eVP9_HD,
   BXVD_DecodeModeStill_eVP9_4K,
   BXVD_DecodeModeStill_MaxModes
} BXVD_DecodeStillMode;

/*************************************************************************
Summary:
 Addresses and other info of buffers used for still picture decode. This is
 used in the "Still Picture Ready"  interrupt handler.
**************************************************************************/

/*
 * enums to indicate the still picture Source/BufferType.
 *
 * The picture can be decoded from a progressive, interlaced or unknown
 * source type.
 *
 *    BXVD_SourceType_eInterlaced  - Source data interlaced
 *    BXVD_SourceType_eProgressive - Source data progressive
 *    BXVD_SourceType_eUnknown     - Source type not specified
 *
 * Independent of the SourceType, the BufferType will be frame, field-pair,
 * top-field or bottom-field.
 *
 *    BXVD_BufferType_eFrame     - frame decoded
 *    BXVD_BufferType_eFieldPair - both fields decoded
 *    BXVD_BufferType_eTopField  - top field only decode
 *    BXVD_BufferType_eBotFiel   - bottom field only decode.
 */
typedef enum BXVD_SourceType
{
   BXVD_SourceType_eInterlaced = 1,
   BXVD_SourceType_eProgressive,
   BXVD_SourceType_eUnknown,
   BXVD_SourceType_eMax
} BXVD_SourceType;

typedef enum BXVD_BufferType
{
   BXVD_BufferType_eFrame = 1,
   BXVD_BufferType_eFieldPair,
   BXVD_BufferType_eTopField,
   BXVD_BufferType_eBotField,
   BXVD_BufferType_eMax
} BXVD_BufferType;

typedef struct BXVD_StillPictureBuffers
{
   BMMA_Block_Handle        hLuminanceFrameBufferBlock;
   uint32_t                 ulLuminanceFrameBufferBlockOffset;
   BMMA_Block_Handle        hChrominanceFrameBufferBlock;
   uint32_t                 ulChrominanceFrameBufferBlockOffset;
   unsigned long    ulImageWidth;
   unsigned long    ulImageHeight;
   unsigned long    ulDisplayWidth;
   unsigned long    ulDisplayHeight;
   unsigned long    ulStripedWidth;
   unsigned long    ulLumaStripedHeight;
   unsigned long    ulChromaStripedHeight;
   BAVC_VideoBitDepth eBitDepth;
   BXVD_SourceType  eSourceType;
   BXVD_BufferType  eBufferType;
   BFMT_AspectRatio eAspectRatio;
   uint16_t         uiSampleAspectRatioX;
   uint16_t         uiSampleAspectRatioY;
   bool             bDecodeError;
} BXVD_StillPictureBuffers;

#define BXVD_DECODE_MODE_RECT_NUM_INVALID UINT32_C (-1)

/***************************************************************************
Summary:
    Enum used to select the size of the picture buffers

Description:
        Max video decode resolution used by this channel

    Decode max resolution used by the channel to determine the size of
    FW Pictures buffers

See also:
****************************************************************************/
typedef enum BXVD_DecodeResolution
{
   BXVD_DecodeResolution_eHD,
   BXVD_DecodeResolution_eSD,
   BXVD_DecodeResolution_eCIF,
   BXVD_DecodeResolution_eQCIF,
   BXVD_DecodeResolution_e4K,
   BXVD_DecodeResolution_eMaxModes
} BXVD_DecodeResolution;


/***************************************************************************
Summary:
    Enum used to select the type of decoder skip mode.

Description:
    The decoder can be instructed to skip B pictures only or B and P pictures
    to effectively achieve a fast forward mode.
See Also:
    BXVD_SetTrickModeDecode
****************************************************************************/
typedef enum BXVD_SkipMode
{
    BXVD_SkipMode_eDecode_I_Only=0,
    BXVD_SkipMode_eDecode_IP_Only,
    BXVD_SkipMode_eDecode_IPB,
    BXVD_SkipMode_eDecode_Ref_Only,
    BXVD_SkipMode_eDecode_MaxModes
} BXVD_SkipMode;

/***************************************************************************
Summary:
    Enum used to select RAVE endianess

Description:
    The endianess of the RAVE is specified by setting the eRaveEndianess value
  in the BXVD_Settings structure prior to the call to BXVD_Open. The default
  value is BXVD_RaveEndianess_eBig.

See Also:
    BXVD_Settings structure
****************************************************************************/
typedef enum BXVD_RaveEndianess
{
    BXVD_RaveEndianess_eBig,
    BXVD_RaveEndianess_eLittle,
    BXVD_RaveEndianess_eMaxValue
} BXVD_RaveEndianess;


/***************************************************************************
Summary:
    Enum used to select timestamp mode.

Description:
    This is to support AVC in AVI container files. The default is
  BXVD_TimestampMode_eDecode.

See Also:
    BXVD_Settings structure
****************************************************************************/

typedef enum BXVD_TimestampMode
{
    BXVD_TimestampMode_eDecode = 0, /* Timestamps are expected in decode order - default use for normal streams */
    BXVD_TimestampMode_eDisplay,    /* Timestamps are expected in display order - primarily for AVC in AVI containers */
    BXVD_TimestampMode_eMaxModes
} BXVD_TimestampMode;


/***************************************************************************
 Summary:
  Enum used to specify AVD core boot mode.

 Description:
  This enum is used to determine if an AVD core boot is the result of a
  "normal" bootup sequence or the result of a watchdog restart.

****************************************************************************/
typedef enum BXVD_AVDBootMode
{
    BXVD_AVDBootMode_eNormal = 0,   /* Normal boot */
    BXVD_AVDBootMode_eWatchdog,      /* Watchdog reboot */
    BXVD_AVDBootMode_eMaxModes
} BXVD_AVDBootMode;


/***************************************************************************
Summary:

   Application must specify the memory sizes used by video decoder.
***************************************************************************/
typedef struct BXVD_FWMemConfig
{
  uint32_t uiGeneralHeapSize;
  uint32_t uiCabacHeapSize;
  uint32_t uiPictureHeapSize;
  uint32_t uiPictureHeap1Size;
  uint32_t uiAVDNonMappedHeapSize;
} BXVD_FWMemConfig;

/*************************************************************************
  Used to specify memory controller configuration of system.

*************************************************************************/
typedef struct  BXVD_FWMemConfigSettings
{
  unsigned int     uiAVDInstance;     /* Decoder instance */
  unsigned int     MemcIndex;         /* Memory controller for main picture buffer */
  unsigned int     MemcIndexExtended; /* Memory controller for split picture buffers  */
  BCHP_MemoryInfo *pInfo;             /* Memory characteristics */
} BXVD_FWMemConfigSettings;

/***************************************************************************
 Summary:
  Enum used to specify Display Manager mode.

 Description:
  This enum is used to specify which flavor of DM that should be used

****************************************************************************/
typedef enum BXVD_DisplayMgrMode
{
   BXVD_DisplayMgrMode_eSTB = 0,  /* STB flavor of DM */
   BXVD_DisplayMgrMode_eUOD1,     /* UOD 1.x flavor of DM */
   BXVD_DisplayMgrMode_eUOD2,     /* UOD 2.x flavor of DM */

   BXVD_DisplayMgrMode_eMaxModes
} BXVD_DisplayMgrMode;

/***************************************************************************
 Summary:
  Enum used to specify Display Manager pulldown mode.
 See also:
  BXVD_SetPulldown
****************************************************************************/
typedef enum BXVD_PulldownMode
{
   BXVD_PulldownMode_e3to2 = 0,  /* enable 3:2 pulldown */
   BXVD_PulldownMode_e2to2,      /* disable 3:2 pulldown */
   BXVD_PulldownMode_eMaxPulldownMode
} BXVD_PulldownMode;

/***************************************************************************
 Summary:
  Enum used to specify Display Manager picture error handling.
****************************************************************************/
typedef enum BXVD_Picture_ErrorHandling
{
   BXVD_ErrorHandling_eOff = 0,     /* display all pictures */
   BXVD_ErrorHandling_ePicture,     /* drop pictures if BXVD_P_PPB_FLAG_DECODE_ERROR is set  */
   BXVD_ErrorHandling_ePrognostic,   /* ~drop pictures from BXVD_P_PPB_FLAG_DECODE_ERROR to the next RAP */
   BXVD_ErrorHandling_eMaxErrorHandling
} BXVD_Picture_ErrorHandling;

/***************************************************************************
 Summary:
  Enum used to evaluate the video protocol level.

 Description:
  The video decoder writes the video protocol level into the PPB structure.
  This information is saved by the display manager and can be retrieved
  with the BXVD_GetChannelStatus() command.

 See also:
  BXVD_GetChannelStatus

****************************************************************************/
typedef enum BXVD_Protocol_Level
{
   BXVD_Video_Protocol_eLevel_Unknown = 0,
   BXVD_Video_Protocol_eLevel_00,
   BXVD_Video_Protocol_eLevel_10,
   BXVD_Video_Protocol_eLevel_1B,
   BXVD_Video_Protocol_eLevel_11,
   BXVD_Video_Protocol_eLevel_12,
   BXVD_Video_Protocol_eLevel_13,
   BXVD_Video_Protocol_eLevel_20,
   BXVD_Video_Protocol_eLevel_21,
   BXVD_Video_Protocol_eLevel_22,
   BXVD_Video_Protocol_eLevel_30,
   BXVD_Video_Protocol_eLevel_31,
   BXVD_Video_Protocol_eLevel_32,
   BXVD_Video_Protocol_eLevel_40,
   BXVD_Video_Protocol_eLevel_41,
   BXVD_Video_Protocol_eLevel_42,
   BXVD_Video_Protocol_eLevel_50,
   BXVD_Video_Protocol_eLevel_51,
   BXVD_Video_Protocol_eLevel_60,
   BXVD_Video_Protocol_eLevel_62,
   BXVD_Video_Protocol_eLevel_low,
   BXVD_Video_Protocol_eLevel_main,
   BXVD_Video_Protocol_eLevel_high,
   BXVD_Video_Protocol_eLevel_high1440,
   BXVD_Video_Protocol_eLevel_MaxLevel

} BXVD_Video_Protocol_Level;

/***************************************************************************
 Summary:
  Enum used to evaluate the video profile.

 Description:
  The video decoder writes the video profile into the PPB structure.
  This information is saved by the display manager and can be retrieved
  with the BXVD_GetChannelStatus() command.

 See also:
  BXVD_GetChannelStatus

****************************************************************************/
typedef enum BXVD_Video_Protocol_Profile
{
   BXVD_Video_Protocol_eProfile_Unknown = 0,
   BXVD_Video_Protocol_eProfile_Simple,
   BXVD_Video_Protocol_eProfile_Main,
   BXVD_Video_Protocol_eProfile_High,
   BXVD_Video_Protocol_eProfile_Advanced,
   BXVD_Video_Protocol_eProfile_Jizhun,
   BXVD_Video_Protocol_eProfile_SnrScalable,
   BXVD_Video_Protocol_eProfile_SpatiallyScalable,
   BXVD_Video_Protocol_eProfile_AdvancedSimple,
   BXVD_Video_Protocol_eProfile_Baseline,
   BXVD_Video_Protocol_eProfile_MultiHigh,
   BXVD_Video_Protocol_eProfile_StereoHigh,
   BXVD_Video_Protocol_eProfile_Main10,

   BXVD_Video_Protocol_eProfile_MaxProfile

} BXVD_Video_Protocol_Profile;


/***************************************************************************
 Summary:
  Enum used to set XVD DM's picture drop mode

 Description:
  DM may need to drop pictures in order to perform rate matching.  DM
  can be set into a field-drop or frame-drop mode with the
  BXVD_SetPictureDropMode() command and/or in
  BXVD_DecodeSettings.ePictureDropMode

  SW7425-4786: the drop mode is only applied during normal playback.  The
  drop mode is ignored during trick play. The middleware/application
  should put the system into top-file-only mode during trick play.

 See also:
  BXVD_SetPictureDropMode

****************************************************************************/
typedef enum BXVD_PictureDropMode
{
   BXVD_PictureDropMode_eField,
   BXVD_PictureDropMode_eFrame,

   /* Add more enums ABOVE this line */
   BXVD_PictureDropMode_eMax
} BXVD_PictureDropMode;

/***************************************************************************
Summary:
  Enum used to set XVD DM's Horizontal Overscan mode

 Description:
  it may be desired to change the default behavior of the Horizontal Overscan
  calculations within the DM.  Thus the DM can be configured to bypass the
  Horizonal Overscan calculation using the BXVD_SetHorizontalOverscanMode()
  command.
  Default behavior is specified by eAuto.

 See also:
  BXVD_SetHorizontalOverscanMode

****************************************************************************/
typedef enum BXVD_HorizontalOverscanMode
{
   BXVD_HorizontalOverscanMode_eAuto,
   BXVD_HorizontalOverscanMode_eDisable,

   /* Add more enums ABOVE this line */
   BXVD_HorizontalOverscanMode_eMax
} BXVD_HorizontalOverscanMode;

/***************************************************************************
Summary:
    XVD settings used in BXVD_Open

NOTE:
The following members are deprecated and should not be used in new code:

   bSelfTest
   hTimerDev
   ulWatchdogTimeValue;

See also:
 BXVD_Open
 BXVD_GetDefaultSettings
***************************************************************************/
typedef struct BXVD_Settings
{
      unsigned int       uiAVDInstance;       /* Decoder instance. Always 0 for 7401 and single decoder systems  */
      BXVD_RaveEndianess eRaveEndianess;      /* RAVE endianess. Default is big */

      BMMA_Heap_Handle   hFirmwareHeap;       /* Decoder firmware memory heap handle where FW is loaded */
      BMMA_Heap_Handle   hCabacHeap;          /* Secure cabac memory heap handle */
      BMMA_Heap_Handle   hGeneralHeap;        /* General purpose device memory heap handle */
      BMMA_Heap_Handle   hPictureHeap;        /* Decoder picture buffer memory heap handle */
      BMMA_Heap_Handle   hPictureHeap1;       /* Decoder picture buffer 1 memory heap handle */

      BTMR_Handle        hTimerDev;           /* Debug timer */

      /* Add BIMG Interface support */

      /* Interface to access firmware image. This interface must be
       * implemented and the function pointers must be stored here.
       */
      const BIMG_Interface *pImgInterface;

      /* Context for the Image Interface. This is also provided by
       * the implementation of the Image interface
       */
      void **pImgContext;

      /* Add Authenticated image support */

      /* Pointer to the AVD boot callback function. If non-NULL,
       * XVD will call this function after the firmware has been
       * loaded into memory instead of booting the AVD. If this
       * function is NULL, then the XVD will boot the AVD
       * normally. This function should return BERR_SUCCESS if
       * successful.
       */

#if BXVD_P_AVD_ARC600
      BERR_Code (*pAVDBootCallback)(void* pPrivateData,
                                    BAFL_BootInfo *pstAVDBootInfo);
#else
      BERR_Code (*pAVDBootCallback)(void* pPrivateData,
                                    BXVD_AVDBootMode eAVDBootMode);
#endif

      /* Pointer to AVD boot callback private data that is passed
       * back into the callback. Can be used to store any
       * information necessary for the application to boot the
       * core.
       */
      void *pAVDBootCallbackData;

      /* Pointer to the AVD reset callback function. If non-NULL,
       * XVD will call this function in the chip reset routine.
       * If this function is NULL, then the XVD will reset the AVD
       * normally. This function should return BERR_SUCCESS if
       * successful.
       */
      BERR_Code (*pAVDResetCallback)(void* pPrivateData);

      /* Pointer to AVD reset callback private data that is passed
       * back into the callback. Can be used to store any
       * information necessary for the application to reset the
       * core.
       */
      void *pAVDResetCallbackData;

      /* PR30249: Add UOD2 support */
      /* The following configuration flag specifies the DM mode to be
       * used */
      BXVD_DisplayMgrMode eDisplayMgrMode;

      BXVD_FWMemConfig stFWMemConfig; /* Custom FW video memory configuration */

      /* Size of Decoder debug logging buffer */
      uint32_t  uiDecoderDebugLogBufferSize;

      bool bInterLayerBandwidthOptimized;

      /* Number of channel contexts to pre-allocate */
      uint32_t uiNumPreAllocateChannels;
} BXVD_Settings;


/**************************************************************************
Summary:
  Used to specify decoder supported video compression standards
****************************************************************************/
typedef struct BXVD_HardwareCapabilities
{
   bool bCodecSupported[BAVC_VideoCompressionStd_eMax];
   uint32_t uiSTC_Count; /* Number of STCs supported by AVD core */
   bool bIncludeRepeatedItbStartCodes;
} BXVD_HardwareCapabilities;

/**************************************************************************
Summary:
  Used to specify 7401 still content interpolation mode
****************************************************************************/
typedef enum BXVD_StillContentInterpolationMode
{
  BXVD_StillContentInterpolationMode_eDefault, /* This is the default state wherein XVD is instructed to use the correct display reccomendations */
  BXVD_StillContentInterpolationMode_eSingleField,/* Last Valid Field Display on Pause */
  BXVD_StillContentInterpolationMode_eBothField,/* Both Field Display on Pause */
  BXVD_StillContentInterpolationMode_eFrame, /* Frame Display (conditional) on Pause */
  BXVD_StillContentInterpolationMode_eMaxModes
} BXVD_StillContentInterpolationMode;

/**************************************************************************
Summary:
  Used to specify 7401 moving content interpolation mode
****************************************************************************/
typedef enum BXVD_MovingContentInterpolationMode
{
  BXVD_MovingContentInterpolationMode_eDefault,
  BXVD_MovingContentInterpolationMode_eInterlacedScanout,
  BXVD_MovingContentInterpolationMode_eProgressiveScanout,
  BXVD_MovingContentInterpolationMode_eMaxModes
} BXVD_MovingContentInterpolationMode;


/**************************************************************************
Summary:
  Used to specify the channel change mode. This instructs the display
  manager to either mute video during a channel change or to display
  the last frame of the previous channel.

  FirstPicturePreview allows for the first picture of the new channel
  to be displayed as soon as it is available, while TSM lock is in
  the process of being acquired.  The first picture will remain on
  screen until display can continue with TSM, if TSM is enabled.
****************************************************************************/
typedef enum BXVD_ChannelChangeMode
{
  BXVD_ChannelChangeMode_eMute,                                /* Mute on Channel change */
  BXVD_ChannelChangeMode_eLastFramePreviousChannel,            /* Display last frame of previous channel */
  BXVD_ChannelChangeMode_eMuteWithFirstPicturePreview,         /* Mute until first picture of new channel */
  BXVD_ChannelChangeMode_eLastFramePreviousWithFirstPicturePreview, /* Show last picture until first picture of new channel */
  BXVD_ChannelChangeMode_eMaxModes
} BXVD_ChannelChangeMode;

/**************************************************************************
Summary:
  Used to specify the PTS type
****************************************************************************/

#define BXVD_PTSType BXDM_PictureProvider_PTSType

#define BXVD_PTSType_eCoded                        BXDM_PictureProvider_PTSType_eCoded
#define BXVD_PTSType_eInterpolatedFromValidPTS     BXDM_PictureProvider_PTSType_eInterpolatedFromValidPTS
#define BXVD_PTSType_eHostProgrammedPTS            BXDM_PictureProvider_PTSType_eHostProgrammedPTS_Deprecated
#define BXVD_PTSType_eInterpolatedFromInvalidPTS   BXDM_PictureProvider_PTSType_eInterpolatedFromInvalidPTS
#define BXVD_PTSType_eMaxPTSType                   BXDM_PictureProvider_PTSType_eMax


/**************************************************************************
Summary:
  Used to specify a display interrupt number from the video display.
  Depening on the platform, each AVD may get multiple display
  interrupts.
****************************************************************************/
typedef enum BXVD_DisplayInterrupt
{
   BXVD_DisplayInterrupt_eZero,
   BXVD_DisplayInterrupt_eOne,
   BXVD_DisplayInterrupt_eTwo,

   BXVD_DisplayInterrupt_eMax
} BXVD_DisplayInterrupt;

/**************************************************************************
Summary:
  Used to specify an STC number. On a display interrupt, the AVD core
  may snapshot multiple STC counters. Not all platforms support eight STCs
****************************************************************************/
typedef enum BXVD_STC
{
   BXVD_STC_eZero,
   BXVD_STC_eOne,
   BXVD_STC_eTwo,
   BXVD_STC_eThree,
   BXVD_STC_eFour,
   BXVD_STC_eFive,
   BXVD_STC_eSix,
   BXVD_STC_eSeven,
   BXVD_STC_eEight,
   BXVD_STC_eNine,
   BXVD_STC_eTen,
   BXVD_STC_eEleven,

   BXVD_STC_eMax
} BXVD_STC;

typedef struct BXVD_STCInfo
{
      uint32_t ui32STCValue;
      BAVC_Polarity eInterruptPolarity;
} BXVD_STCInfo;

typedef enum BXVD_HITSMode
{
   BXVD_HITSMode_eDisabled,  /* Off, default */
   BXVD_HITSMode_eLegacy,    /* HITS enabled without progressive/banded display */
   BXVD_HITSMode_eClean      /* Clean HITS mode builds the display progressively */
} BXVD_HITSMode;

/*
 * Summary: Enum for Video Instruction Checker Interrupts
 */

typedef enum BXVD_VICHInt
{
   BXVD_VICHInt_eRegAccess,  /* Register access address violation */
   BXVD_VICHInt_eSCBWrite,   /* SCB write address violation */
   BXVD_VICHInt_eInstrRead   /* instruction read address violation */
} BXVD_VICHInt;

/**************************************************************************
Summary:
  7401 device interrupts.

Description:
  These interrupts are for the XVD device, they are not for channel specific
  interrupts.

   BXVD_Interrupt_eVideoInstructionChecker:
     This callback is triggered when one of the Video Instruction Checker Interrupts occur.

     The parameter passed by this callback is the BXVD_VICHInt enum to indicate which
     VICH interrupt occured.

****************************************************************************/
typedef enum BXVD_DeviceInterrupt
{
   BXVD_DeviceInterrupt_eWatchdog,                /* Video Decoder WatchDog Interrupt */
   BXVD_DeviceInterrupt_eDecodedStillBufferReady, /* Decoded Buffer Ready interrupt for Still picture mode */
   BXVD_DeviceInterrupt_eDisplayInterrupt0,       /* Display Interrupt number 0 (from source feeder) */
   BXVD_DeviceInterrupt_eDisplayInterrupt1,       /* Display Interrupt number 1 (from source feeder) */
   BXVD_DeviceInterrupt_eDisplayInterrupt2,       /* Display Interrupt number 2 (from source feeder) */

   BXVD_DeviceInterrupt_ePictureDataReady0,       /* Picture Data Ready Interrupt 0 */
   BXVD_DeviceInterrupt_ePictureDataReady1,       /* Picture Data Ready Interrupt 1 */
   BXVD_DeviceInterrupt_ePictureDataReady2,       /* Picture Data Ready Interrupt 2 */
   BXVD_DeviceInterrupt_eVidInstrChecker,         /* Video Instruction Checker */
   BXVD_DeviceInterrupt_eStereoSeqError,          /* Stereo Sequence Error */
   BXVD_DeviceInterrupt_eMaxInterrupts            /* Not a Real interrupt just the max no of XVD Device interrupts */
} BXVD_DeviceInterrupt;

/**************************************************************************
Summary:
  7401 channel interrupts.

Description:
  These interrupts are for the XVD channel events. The events that occur are
  handled on a per channel basis.

NOTE:
  As interrupts are added to or removed from this list, the corresponding
  entries in BXVD_EnableInterrupt's case statement should be maintained
  accordingly. Note that not all interrupts can be disabled or enabled via
  the BXVD_EnableInterrupt API.

  Information on specific callbacks

  BXVD_Interrupt_ePtsStcOffset:
     This callback is triggered when the difference between the STC and the running PTS is greater than
     the uiVsyncDiffThreshDefault value specified at BXVD_OpenChannel.  The threshold need to be programmed
     appropriately taking the clock domain of MPEG or legacy clock domains into consideration.  The
     callback is not generated if the BXVD_DisplayMode is BXVD_DisplayMode_eVSYNCMode.

     The parameter passed by this callback is the PTS to PCR difference.  This value is always positive.

  BXVD_Interrupt_eFirstPTSReady:
     This callback is triggered by either of two events: when the first picture
     is delivered from the decoder to the display manager or when the display manager
     sees the first picture with a "coded PTS", that is, a PTS directly encoded for the
     picture being evaluated by the display manager.  If both these triggers occur on
     the same vsync interrupt, then the callback will only be made once.  If these
     triggers occur on different interrupts than the callback will be made twice.
     BXVD_Interrupt_eFirstPTSReady always gets generated for the very first picture
     even if STCInvalid is set, the first picture fails to qualify for display, and regardless
     of the BXVD_DisplayMode setting.

     The parameter passed by this callback is the BXVD_PTSInfo structure.

   BXVD_Interrupt_eFirstPTSPassed:
     This is callback is triggered when a picture with either coded PTS or interpolated
     from valid PTS, matures (i.e. passes TSM and parity check) for display, for the
     first time.  The callback is not generated if the BXVD_DisplayMode is BXVD_DisplayMode_eVSYNCMode.

     The parameter passed by this callback is the BXVD_PTSInfo structure.

  BXVD_Interrupt_ePTSError:
     This callback is triggered when the display manager detects a PTS error.  If either
     the picture is too early for display or too late for display, time stamp management
     will fail and this callback is invoked. Typically this callback is invoked to denote
     that there is some error in the PTS of the picture or the STC is not matching the PTS.
     The callback is not generated if the BXVD_DisplayMode is BXVD_DisplayMode_eVSYNCMode.

     The parameter passed by this callback is the BXVD_PTSInfo structure.

  BXVD_Interrupt_ePauseUntoPTS:
     This callback is triggered when the running PTS value exceeds the PTS value specified
     by BXVD_PauseUntoPTS.  The display will be paused and this callback is generated.
     Note, BXVD_PauseUntoPTS allows the application can specify a value for "counter", such
     that the DM performs the above action only after the stream wraps around as many times
     as specified in the "counter".  The callback is generated regardless of the BXVD_DisplayMode
     setting.

     The parameter passed by this callback is the BXVD_PTSInfo structure.

  BXVD_Interrupt_eDisplayUntoPTS:
     This callback is triggered when the running PTS value exceeds the PTS value specified
     by BXVD_DisplayUntoPTS.  The display will be unpaused and this callback is generated.
     Note, BXVD_DisplayUntoPTS allows the application can specify a value for "counter", such
     that the DM performs the above action only after the stream wraps around as many times
     as specified in the "counter".  The callback is generated regardless of the BXVD_DisplayMode
     setting.

     The parameter passed by this callback is the BXVD_PTSInfo structure.

   BXVD_Interrupt_eIFrame:
     This callback is triggered when the first iframe is detected after a channel change.  It is
     useful to detect streams which do not contain iframes (HITS).  The callback is generated regardless
     of the BXVD_DisplayMode setting.

     The parameter passed by this callback is a boolean, always true.

   BXVD_Interrupt_ePictureParameters:
     This callback is triggered on every vsync to inform the application of current picture parameters.
     The callback is generated regardless of the BXVD_DisplayMode setting.

     The parameter passed by this callback is the BXVD_PictureParameterInfo structure.

   BXVD_Interrupt_ePPBParameters:
      This callback is triggered every time a PPB is evaluated from
      the AVD picture delivery queue. The callback is generated
      regardless of the BXVD_DisplayMode setting.

      The parameter passed by this callback is a pointer to the
      BXVD_PPBParameterInfo structure.

   BXVD_Interrupt_eDecodeError
      This callback is triggered on any vsync that the Display Manager
      encounters a PPB with a decode error.

      The parameter passed is the running total of decode errors.

   BXVD_Interrupt_eTSMResult
      This callback is triggered for every TSM evaluation on each
      undisplayed picture that the Display Manager evaluates.  It is
      possible that the same picture can generate multiple TSMResult
      callbacks (E.g. if the picture's TSMResult is eWait).  The app
      can override how the Display Manager handles the picture by
      changing the ePictureHandlingMode of the picture.

      The parameter passed by this callback is a pointer to the
      BXVD_TSMInfo structure.

   BXVD_Interrupt_PictureExtensionData
      Generic data callback mechanism, called when a picture has extension data.

      The parameter pass by this callback is a "BXDM_Picture_ExtensionInfo *".

   BXVD_Interrupt_eChunkDone
      SW7425-3358: FNRT, executed when the last picture of a chunk is sent to VDC

   **************************************************************
   ** Below is a description of how the callbacks are enabled. **
   **************************************************************

   The following callbacks are ENABLED by default.
   They are enabled/disabled by calling "BXVD_EnableInterrupt".
   They are "continuous" and must be explicitly disabled.

      BXVD_Interrupt_ePtsStcOffset
      BXVD_Interrupt_ePTSError
      BXVD_Interrupt_ePictureParameters
      BXVD_Interrupt_eTSMPassInASTMMode
      BXVD_Interrupt_eTSMResult
      BXVD_Interrupt_ePPBParameters

   The following callbacks are ENABLED by default.
   They are enabled/disabled by calling "BXVD_EnableInterrupt".
   They are "one shots", i.e. they are disabled after being executed.

      BXVD_Interrupt_eRequestSTC
      BXVD_Interrupt_eFirstPTSReady
      BXVD_Interrupt_eFirstPTSPassed
      BXVD_Interrupt_eIFrame
      BXVD_Interrupt_eChunkDone


   The following callbacks are DISABLED by default.
   They are enabled/disabled by calling "BXVD_EnableInterrupt".
   They are "continuous" and must be explicitly disabled.

      BXVD_Interrupt_eMarker
      BXVD_Interrupt_PictureExtensionData
      BXVD_Interrupt_eDecodeError

   The following callbacks are DISABLED by default.
   They are enabled when "BXVD_SetClipTime" is called.
   They are "one shots", i.e. they are disabled after being executed.

      BXVD_Interrupt_eClipStart
      BXVD_Interrupt_eClipStop
      BXVD_Interrupt_ePictureMarker

****************************************************************************/
typedef enum BXVD_Interrupt
{
   BXVD_Interrupt_ePtsStcOffset,            /* STC offset changed interrupt */
   BXVD_Interrupt_ePictureDataReady,        /* Picture Data Ready Display Interrupt for legacy API only.
                                             * New apps should use the device interrupts when more than one
                                             * Display Interrupt is needed. */
   BXVD_Interrupt_eUserData,                /* User Data Available */
   BXVD_Interrupt_ePicDepthLowerThreshold,  /* DEPRECATED: Picture Depth Lower Thresold Crossed */
   BXVD_Interrupt_ePicDepthHigherThreshold, /* DEPRECATED: Picture Depth Higher Thresold Crossed */
   BXVD_Interrupt_eFirstPTSReady,           /* First Picture Ready */
   BXVD_Interrupt_eFirstPTSPassed,          /* First PTS Passed */
   BXVD_Interrupt_ePTSError,                /* PTS mismatch */
   BXVD_Interrupt_ePauseUntoPTS,            /* DEPRECATED: PauseUntoPTS event occured notification */
   BXVD_Interrupt_eDisplayUntoPTS,          /* DEPRECATED: DisplayUntoPTS event occured notification */
   BXVD_Interrupt_ePTS1Match,               /* DEPRECATED: EnableInterruptOnPTS1Match event occured notification */
   BXVD_Interrupt_ePTS2Match,               /* DEPRECATED: EnableInterruptOnPTS2Match event occured notification */
   BXVD_Interrupt_eIFrame,                  /* I-Frame detect event occured notification */
   BXVD_Interrupt_ePictureParameters,       /* Called when first picture is queued to the display manager from
                                               the firmware to inform application of size, framerate, etc. */
   BXVD_Interrupt_eTSMPassInASTMMode,       /* Called when DM is in
                                             * vsync mode, configured
                                             * for ASTM mode, and DM
                                             * sees a TSM pass */
   BXVD_Interrupt_eVideoInstructionChecker, /* VIC interrupt */
   BXVD_Interrupt_eClipStart,               /* ClipStart event occured notification */
   BXVD_Interrupt_eClipStop,                /* ClipStop event occured notification */
   BXVD_Interrupt_ePictureMarker,           /* PictureMarker event occurred notification */
   BXVD_Interrupt_eMarker,                  /* Generic Marker event occurred notification. Pointer to marker value passed as pXVD_data during callback */
   BXVD_Interrupt_eRequestSTC,              /* Called when DM needs the application to provide a valid STC before proceeding with TSM calculations */

   BXVD_Interrupt_ePPBReceived,             /* Called when the "next" PPB is */

   BXVD_Interrupt_ePPBParameters,           /* Called when a PPB is first evaluated for display */

   BXVD_Interrupt_eDecodeError,             /* Called when the decode error bit is set in the PPB */

   BXVD_Interrupt_eTSMResult,               /* Called for each TSM evaluation before a picture is displayed */

   BXVD_Interrupt_ePictureExtensionData,     /* Generic data callback mechanism, called when a picture has extension data. */

   BXVD_Interrupt_eChunkDone,               /* SW7425-3358: FNRT, executed when the last picture of a chunk is sent to VDC */

   BXVD_Interrupt_eEndOfGOP,                /* SW7425-2686: multi-pass DQT: the end of the GOP has been reached, beginning reverse playback. */

   BXVD_Interrupt_eMaxInterrupts            /* Not a Real interrupt just the max no of XVD interrupts */
} BXVD_Interrupt;


typedef enum BXVD_ChannelMode
{
   BXVD_ChannelMode_eVideo = 0,
   BXVD_ChannelMode_eStill,

   BXVD_ChannelMode_eMax
} BXVD_ChannelMode;

typedef enum BXVD_1080pScanMode
{
   /* eDefault: 1080p pass-thru, else 1080i
    *
    * 1080p pass-thru cases:
    *   - 1080p23.97/24 --> 23.97/24 Hz
    *   - 1080p25 --> 25 Hz
    *   - 1080p29.97/30 --> 29.97/30 Hz
    */
   BXVD_1080pScanMode_eDefault = 0,

   /* eAdvanced: 1080p pass-thru + 1080p pulldown, else 1080i
    *
    * 1080p pulldown cases:
    *   - 1080p23.97/24 --> 59.94/60 Hz (3:2 frame cadence)
    *   - 1080p29.97/30 --> 59.94/60 Hz (2:2 frame cadence)
    *   - 1080p25 --> 50Hz (2:2 frame cadence)
    */
   BXVD_1080pScanMode_eAdvanced,

   /* Add new enums ABOVE this line */
   BXVD_1080pScanMode_eMax
} BXVD_1080pScanMode;

/***************************************************************************
Summary:
BXVD_SetMonitorRefreshRate symbolic constants. Used as alternative to
BFMT_VERT_xxHz constants for uiMonitorRefreshRate argument.

If BFMT_VERT_INVALID is defined, use equivalent symbolic constants.
BFMT_VERT_XX contants maybe deprecated with new refresh rates being
unsupported by this group of symbolic contants. If not defined, create own
values solely used by XVD
****************************************************************************/

#ifdef BFMT_VERT_INVALID
#define BXVD_MONITOR_REFRESH_RATE_INVALID   BFMT_VERT_INVALID
#define BXVD_MONITOR_REFRESH_RATE_50Hz      BFMT_VERT_50Hz
#define BXVD_MONITOR_REFRESH_RATE_59_94Hz   BFMT_VERT_59_94Hz
#define BXVD_MONITOR_REFRESH_RATE_60Hz      BFMT_VERT_60Hz
#define BXVD_MONITOR_REFRESH_RATE_23_976Hz  BFMT_VERT_23_976Hz
#define BXVD_MONITOR_REFRESH_RATE_24Hz      BFMT_VERT_24Hz
#define BXVD_MONITOR_REFRESH_RATE_25Hz      BFMT_VERT_25Hz
#define BXVD_MONITOR_REFRESH_RATE_30Hz      BFMT_VERT_30Hz
#ifdef BFMT_VERT_48Hz
#define BXVD_MONITOR_REFRESH_RATE_48Hz      BFMT_VERT_48Hz
#else
#define BXVD_MONITOR_REFRESH_RATE_48Hz      0x00004000
#endif
#ifdef BFMT_VERT_29_97Hz
#define BXVD_MONITOR_REFRESH_RATE_29_97Hz   BFMT_VERT_29_97Hz
#else
#define BXVD_MONITOR_REFRESH_RATE_29_97Hz   0x00000400
#endif
#ifdef BFMT_VERT_12_5Hz
#define BXVD_MONITOR_REFRESH_RATE_12_5Hz   BFMT_VERT_12_5Hz
#else
#define BXVD_MONITOR_REFRESH_RATE_12_5Hz   0x00020000
#endif
#ifdef BFMT_VERT_14_985Hz
#define BXVD_MONITOR_REFRESH_RATE_14_985Hz   BFMT_VERT_14_985Hz
#else
#define BXVD_MONITOR_REFRESH_RATE_14_985Hz   0x00040000
#endif
#ifdef BFMT_VERT_15Hz
#define BXVD_MONITOR_REFRESH_RATE_15Hz   BFMT_VERT_15Hz
#else
#define BXVD_MONITOR_REFRESH_RATE_15Hz   0x00080000
#endif
#ifdef BFMT_VERT_20Hz
#define BXVD_MONITOR_REFRESH_RATE_20Hz   BFMT_VERT_20Hz
#else
#define BXVD_MONITOR_REFRESH_RATE_20Hz   0x00100000
#endif
#ifdef BFMT_VERT_100Hz
#define BXVD_MONITOR_REFRESH_RATE_100Hz     BFMT_VERT_100Hz
#else
#define BXVD_MONITOR_REFRESH_RATE_100Hz     0x00200000
#endif
#ifdef BFMT_VERT_119_88Hz
#define BXVD_MONITOR_REFRESH_RATE_119_88Hz  BFMT_VERT_119_88Hz
#else
#define BXVD_MONITOR_REFRESH_RATE_119_88Hz  0x00400000
#endif
#ifdef BFMT_VERT_120Hz
#define BXVD_MONITOR_REFRESH_RATE_120Hz     BFMT_VERT_120Hz
#else
#define BXVD_MONITOR_REFRESH_RATE_120Hz     0x00800000
#endif
#ifdef BFMT_VERT_7_493Hz
#define BXVD_MONITOR_REFRESH_RATE_7_493Hz  BFMT_VERT_7_493Hz
#else
#define BXVD_MONITOR_REFRESH_RATE_7_493Hz 0x01000000
#endif
#ifdef BFMT_VERT_7_5Hz
#define BXVD_MONITOR_REFRESH_RATE_7_5Hz  BFMT_VERT_7_5Hz
#else
#define BXVD_MONITOR_REFRESH_RATE_7_5Hz  0x02000000
#endif
#ifdef BFMT_VERT_9_99Hz
#define BXVD_MONITOR_REFRESH_RATE_9_99Hz  BFMT_VERT_9_99Hz
#else
#define BXVD_MONITOR_REFRESH_RATE_9_99Hz  0x04000000
#endif
#ifdef BFMT_VERT_10Hz
#define BXVD_MONITOR_REFRESH_RATE_10Hz  BFMT_VERT_10Hz
#else
#define BXVD_MONITOR_REFRESH_RATE_10Hz  0x08000000
#endif
#ifdef BFMT_VERT_11_988Hz
#define BXVD_MONITOR_REFRESH_RATE_11_988Hz  BFMT_VERT_11_988Hz
#else
#define BXVD_MONITOR_REFRESH_RATE_11_988Hz  0x10000000
#endif
#ifdef BFMT_VERT_12Hz
#define BXVD_MONITOR_REFRESH_RATE_12Hz  BFMT_VERT_12Hz
#else
#define BXVD_MONITOR_REFRESH_RATE_12Hz  0x20000000
#endif
#ifdef BFMT_VERT_19_98Hz
#define BXVD_MONITOR_REFRESH_RATE_19_98Hz  BFMT_VERT_19_98Hz
#else
#define BXVD_MONITOR_REFRESH_RATE_19_98Hz  0x40000000
#endif

#else
#define BXVD_MONITOR_REFRESH_RATE_INVALID   0
#define BXVD_MONITOR_REFRESH_RATE_50Hz      1
#define BXVD_MONITOR_REFRESH_RATE_59_94Hz   2
#define BXVD_MONITOR_REFRESH_RATE_60Hz      3
#define BXVD_MONITOR_REFRESH_RATE_23_976Hz  4
#define BXVD_MONITOR_REFRESH_RATE_24Hz      5
#define BXVD_MONITOR_REFRESH_RATE_25Hz      6
#define BXVD_MONITOR_REFRESH_RATE_30Hz      7
#define BXVD_MONITOR_REFRESH_RATE_48Hz      8
#define BXVD_MONITOR_REFRESH_RATE_29_97Hz   9
#define BXVD_MONITOR_REFRESH_RATE_12_5Hz    10
#define BXVD_MONITOR_REFRESH_RATE_14_985Hz  11
#define BXVD_MONITOR_REFRESH_RATE_15Hz      12
#define BXVD_MONITOR_REFRESH_RATE_20Hz      13
#define BXVD_MONITOR_REFRESH_RATE_100Hz     14
#define BXVD_MONITOR_REFRESH_RATE_119_88Hz  15
#define BXVD_MONITOR_REFRESH_RATE_120Hz     16
#define BXVD_MONITOR_REFRESH_RATE_7_493Hz   17
#define BXVD_MONITOR_REFRESH_RATE_7_5Hz     18
#define BXVD_MONITOR_REFRESH_RATE_9_99Hz    19
#define BXVD_MONITOR_REFRESH_RATE_10Hz      20
#define BXVD_MONITOR_REFRESH_RATE_11_988Hz  21
#define BXVD_MONITOR_REFRESH_RATE_12Hz      22
#define BXVD_MONITOR_REFRESH_RATE_19_98Hz   23

#endif


/***************************************************************************
Summary:
XVD callback function typedef. Different from the BINT_CallbackFunc, returns
one extra parameter, which contains XVD data
****************************************************************************/
typedef void (*BXVD_CallbackFunc)( void *pParm1, int parm2, void *pXVD_data);

/***************************************************************************
Summary:
    Channel specific settings to BXVD_OpenChannel() call. The monitor refresh rate
  is specified in Hertz and the removal delay is specified in vsync units.
****************************************************************************/
typedef struct BXVD_ChannelSettings
{
  uint32_t ui32MonitorRefreshRate;              /* The monitor Refresh Rate in Hertz*/
  unsigned long ulRemovalDelay;                 /* Initial removal delay in vsync units */
  uint32_t uiVsyncDiffThreshDefault;            /* Lip Sync fix */
  BAVC_FrameRateCode eDefaultFrameRate;         /* User-settable default frame rate if unknown from stream */
  BXVD_DecodeResolution eDecodeResolution;      /* Max video decode resolution used by this channel */
  BAVC_VideoCompressionStd *peVideoCmprStdList; /* List of Video compression standards used by this channel */
  uint32_t                   uiVideoCmprCount;  /* Number of video compression standards in list */
  BXVD_DisplayInterrupt eDisplayInterrupt;      /* FW PictDataRdy Interrupt to be used by this channel */
  uint32_t uiVDCRectangleNum;                   /* VDC rectangle number, multi-decode mode */
  BXVD_ChannelMode eChannelMode;                /* The type of decode for this channel (video or still) */
  bool bMPEG2BTPEnable;                         /* Enable MPEG2 BTP mode for this channel */
  bool bAVC41Enable;                            /* Enable AVC 4.1 mode for this channel */
  bool bAVC51Enable;                            /* Enable AVC 5.1 mode for this channel */
  bool bBluRayEnable;                           /* Enable Blu Ray mode for this channel */
  bool bExcessDirModeEnable;                    /* Enable Excess Direct memory mode for this channel */
  bool bSVC3DModeEnable;                        /* Enable 3D decode for SVC decodes on this channel */
  bool bSVCProgressiveOnly;                     /* SVC progressive decode only on this channel */
  bool b1920PortraitModeEnable;                 /* 1080x1920 portrait mode */
  bool b1200HDEnable;                           /* 1920x1200 HD mode */
  bool bSplitPictureBuffersEnable;              /* Separate Luma and Chroma buffers mode */
  bool b10BitBuffersEnable;                     /* 10 Bit Luma and Chroma buffers mode */
  uint32_t uiExtraPictureMemoryAtoms;           /* Additional memory atoms to be allocated, max is 2 */
  BMMA_Block_Handle  hChannelGeneralBlock;        /* General purpose device memory block handle for this channel */
  uint32_t           uiChannelGeneralBlockOffset; /* General purpose device memory offset for this channel */
  uint32_t           uiChannelGeneralBlockSize;   /* General purpose device memory sizefor this channel */
  BMMA_Block_Handle  hChannelCabacBlock;          /* Secure cabac memory block handle for this channel */
  uint32_t           uiChannelCabacBlockOffset;   /* Secure cabac memory offset for this channel */
  uint32_t           uiChannelCabacBlockSize;     /* Secure cabac memory size for this channel */
  BMMA_Block_Handle  hChannelPictureBlock;        /* Decoder picture buffer memory block handle for this channel */
  uint32_t           uiChannelPictureBlockOffset; /* Decoder picture buffer memory offset for this channel */
  uint32_t           uiChannelPictureBlockSize;   /* Decoder picture buffer memory size for this channel */
  BMMA_Block_Handle  hChannelPictureBlock1;        /* Decoder picture buffer 1 memory block handle for this channel */
  uint32_t           uiChannelPictureBlockOffset1; /* Decoder picture buffer 1 memory offset for this channel */
  uint32_t           uiChannelPictureBlockSize1;   /* Decoder picture buffer 1 memory size for this channel */
  BXVD_1080pScanMode e1080pScanMode;            /* 1080p scan mode for this channel */
  BXVD_PictureDropMode ePictureDropMode;        /* PR48726: Indicates whether DM should drop pictures on field or frame boundaries */
} BXVD_ChannelSettings;

/***************************************************************
 BXVD_ChannelStatus AVD Status Block bit flags.
****************************************************************/
#define BXVD_CHANNELSTATUS_AVD_STALLED_ON_INPUT              0x001    /* Informative: Decode stalling due to input */
#define BXVD_CHANNELSTATUS_AVD_STALLED_ON_PIF                0x002    /* Informative: Decode stalling due to PIF shortage.*/
#define BXVD_CHANNELSTATUS_AVD_STALLED_ON_CABAC_WORKLIST     0x004    /* Informative: Decode stalling due to Cabac Work list */
#define BXVD_CHANNELSTATUS_AVD_STALLED_ON_IL_WORKLIST        0x008    /* Informative: Decode stalling due to IL Work list */
#define BXVD_CHANNELSTATUS_AVD_STALLED_ON_PPB                0x010    /* Informative: Decode stalling due to PPB resource */
#define BXVD_CHANNELSTATUS_AVD_STALLED_ON_CABAC_BINBUFFER    0x020    /* Informative: Decode stalling due to BIN resource */
#define BXVD_CHANNELSTATUS_AVD_DECODE_STALLED                0x040    /* Informative: Decode stalling due to any resource */
#define BXVD_CHANNELSTATUS_AVD_STALLED_ON_USERDATA_BUFFER    0x080    /* Informative: Decode stalling due to UserData shortage */
#define BXVD_CHANNELSTATUS_AVD_RAP_NOT_DETECTED              0x100    /* Error: Random Access Point not yet found. No pictures are decoding. */
#define BXVD_CHANNELSTATUS_AVD_UNSUPPORTED_FEATURE_DETECTED  0x200    /* Error: Unsupported feature resulting in no decoding */
#define BXVD_CHANNELSTATUS_AVD_IMAGE_SIZE_TOO_BIG            0x400    /* Error: Picture size too large for video buffers provided */
#define BXVD_CHANNELSTATUS_AVD_BAD_STREAM                    0x800    /* Error: Stream contains illegal syntax */
#define BXVD_CHANNELSTATUS_AVD_LESS_MEM_RESTRICT_BUFF        0x1000   /* Warning: Decoder using less video memory for decoding due to restricted memory resources */
#define BXVD_CHANNELSTATUS_AVD_DECODE_WARNING                0x2000   /* Warning: Recoverable illegal stream */
#define BXVD_CHANNELSTATUS_AVD_INPUT_OVERFLOW                0x4000   /* Error: Input buffer overflow occurs */
#define BXVD_CHANNELSTATUS_AVD_DECODE_STEREO_SEQ_ERROR       0x8000   /* Deprecated, bit now unused */
#define BXVD_CHANNELSTATUS_AVD_STATE_DROP_UNSUPP_TEMP_DM     0x10000  /* Error: Decoder is dropping picture for EchoStart Temporal Direct disable mode */
#define BXVD_CHANNELSTATUS_AVD_10_BIT_UNSUPPORTED            0x20000  /* Error: 10-bit stream dropped for 8-bit capable cores */

/* This mask filters error status from info - Note: RAP_NOT_DETECTED        *
 * is not included since it happens during normal operation at start decode */
#define BXVD_CHANNELSTATUS_AVD_ERROR_MASK \
BXVD_CHANNELSTATUS_AVD_STATE_DROP_UNSUPP_TEMP_DM | \
BXVD_CHANNELSTATUS_AVD_INPUT_OVERFLOW | \
BXVD_CHANNELSTATUS_AVD_BAD_STREAM | \
BXVD_CHANNELSTATUS_AVD_IMAGE_SIZE_TOO_BIG | \
BXVD_CHANNELSTATUS_AVD_UNSUPPORTED_FEATURE_DETECTED | \
BXVD_CHANNELSTATUS_AVD_10_BIT_UNSUPPORTED

/***************************************************************************
Summary:
  Individual channel status information.
****************************************************************************/
/*
 * Most of the following counts are reset to '0' whenever "BXVD_StartDecode()" is called (with exceptions).
 *
 * The counts ARE reset when;
 * - "BXVD_StartDecode()" is called directly by the application
 * - "BXVD_StartDecode()" is called from within "BXVD_FlushDecode()" or "BXVD_DecodeStillPicture()"
 *
 * The counts ARE NOT reset when;
 * - "BXVD_StartDecode()" is called from within "BXVD_ProcessWatchdog()", "BXVD_SetSkipPictureModeDecode()"
 *   or "BXVD_PVR_SetHostSparseMode()"
 */
typedef struct BXVD_ChannelStatus
{

   /*
    * "Picture Delivery Queue" underflow count, for debugging purposes.
    * Reset as described above.
    */
   uint32_t ulUnderflowCount;

   /*
    * A snap shot of the number of pictures "currently" in the "Picture Delivery Queue".
    * This is a very dynamic value which may be stale by the time the
    * application evaluates it.
    * Since this is a dynamic value, it never needs to be reset.
    */
   uint32_t ulPictureDeliveryCount;

   /*
    * A running count of the number of elements (fields/frames) dropped by
    * the "Display Manager" due to "Time Stamp Management" failures.
    * Reset as described above.
    */
   uint32_t uiDisplayManagerDroppedCount;

   /*
    * A running count of the number of elements (fields/frames) the "Display Manager"
    * requests the decoder to drop.  Some drops may be pending.
    * Reset as described above.
    */
   uint32_t uiDecoderDroppedCount;

   /*
    * A running count of the number of elements (fields/frames) the "Display Manager"
    * displayed even though there was a parity failure.
    * Reset as described above.
    */
   uint32_t uiDisplayedParityFailureCount;

   /*
    * A running count of the number of elements (fields/frames) the "Display Manager"
    * delivered to the "Video Display Controller".
    * Reset as described above.
    */
   uint32_t uiDisplayedCount;

   /*
    * A running count of the number of pictures the decoder delivered to
    * the "Display Manager". Reset as described above.
    */
   uint32_t uiPicturesDecodedCount;

   /*
    * A running count of the number vsyncs where the "Display Manager" input
    * queue (Picture Delivery Queue) underflowed.
    * Reset as described above.
    */
   uint32_t uiVsyncUnderflowCount;

   /*
    * A running count of the number decode errors reported by the decoder.
    * Reset as described above.
    */
   uint32_t uiDecodeErrorCount;

   /*
    * A running count of number of times the DM vsync callback has been executed for this channel.
    * Reset as described above.
    */
   uint32_t uiVsyncCount;

   /*
    * A snap shot of the depth of the CABAC bin buffer.  This is a dynamic value
    * which may be stale by the time the application evaluates it.
    * Since this is a dynamic value, it never needs to be reset.
    */
   uint32_t uiCabacBinDepth;

   /*
    * A snap shot of the AVD channel status. The AVD FW updates this status
    * ten times a second.
    */
   uint32_t uiAVDStatusBlock;

   /*
    * A running count of the number of I frames (inclusive of errors) delivered to the Display Manager.
    * Reset as described above.
    */
   uint32_t uiIFrameCount;

   /*
    * A running count of the number of I frames with the error bit set delivered to the Display Manager .
    * Reset as described above.
    */
    uint32_t uiErrorIFrameCount;

   /*
    * Video protocol for the picture being delivered to VDC.
    */
    BAVC_VideoCompressionStd eVideoProtocol;

   /*
    * Video protocol level for the picture being delivered to VDC.
    */
    BXVD_Video_Protocol_Level eProtocolLevel;

   /*
    * Video protocol profile for the picture being delivered to VDC.
    */
    BXVD_Video_Protocol_Profile eProtocolProfile;

    /* Count of all the pictures received by the video decoder.
     * This includes those that are skipped, dropped and decoded.
     * Reset as described above.
     */
    uint32_t uiPicturesReceivedCount;

    /* Number of times the decoder input has gotten into an overflow state.
     * Reset as described above.
     */
    uint32_t uiDecoderInputOverflow;

} BXVD_ChannelStatus;

/***************************************************************************
Summary:
  PTS information structure.
****************************************************************************/
#define BXVD_PTSInfo BXDM_PictureProvider_PTSInfo

/***************************************************************************
Summary:
  Callback structure for BXVD_Interrupt_ePictureParameters.  This structure
  contains stream specific picture information and is delivered when the
  first picture is dequeued by the display manager.  It is to replace the
  deprecated sequence header interrupt, and is how applications should get
  notified of stream parameters at the earliest opportunity.

  Many of the fields in this structure are the same as corresponding fields
  in BAVC_MFD_Picture.  Refer to the documentation in bavc.h for details as to
  what these fields mean.

  The contents of identical fields in BAVC_MFD_Picture and
  BXVD_PictureParameterInfo may be different when the channel is first started,
  because the BXVD_PictureParameterInfo describes pictures which are being
  processed from the display queue, while BAVC_MFD_Picture describes pictures
  that are actively being displayed.  For example, BAVC_MFD_Picture may have
  bMute == true, which would allow certain fields to be undefined, whereas the
  BXVD_PictureParameterInfo would be populated with information from the decoder.

  The BXVD_P_PPB_FLAG_PROG_SEQUENCE flag in the PPB indicates whether a stream is
  progressive or interlaced.  Two copies of this flag are maintained in the
  BXVD_PictureParameterInfo structure.

  "bStreamProgressive_7411" is a filtered copy of BXVD_P_PPB_FLAG_PROG_SEQUENCE.
  It is initially loaded when playback begins.  It is reloaded when any of the
  following stream parameters change: eFrameRateCode, ulSourceHorizontalSize,
  ulSourceVerticalSize, eAspectRatio or eColorPrimaries.  It is being maintained
  for legacy applications.

  "bStreamProgressive" tracks BXVD_P_PPB_FLAG_PROG_SEQUENCE directly.  It is
  copied from the picture being delivered to VDC.

****************************************************************************/
typedef struct BXVD_PictureParameterInfo
{
  uint32_t ulSourceHorizontalSize;     /* see bavc.h BAVC_MFD_Picture */
  uint32_t ulSourceVerticalSize;       /* see bavc.h BAVC_MFD_Picture */
  uint32_t ulDisplayHorizontalSize;    /* see bavc.h BAVC_MFD_Picture */
  uint32_t ulDisplayVerticalSize;      /* see bavc.h BAVC_MFD_Picture */
  BFMT_AspectRatio eAspectRatio;       /* see bavc.h BAVC_MFD_Picture */
  uint16_t uiSampleAspectRatioX;       /* see bavc.h BAVC_MFD_Picture */
  uint16_t uiSampleAspectRatioY;       /* see bavc.h BAVC_MFD_Picture */
  BAVC_FrameRateCode eFrameRateCode;   /* see bavc.h BAVC_MFD_Picture */
  BAVC_ColorPrimaries eColorPrimaries; /* see bavc.h BAVC_MFD_Picture */
  BAVC_TransferCharacteristics eTransferCharacteristics;  /* see bavc.h */
  BAVC_TransferCharacteristics ePreferredTransferCharacteristics; /* SWSTB-1629: see bavc.h */
  BAVC_MatrixCoefficients eMatrixCoefficients; /* see bavc.h BAVC_MFD_Picture */
  bool bFrameProgressive;              /* see bavc.h BAVC_MFD_Picture */
  int32_t i32_HorizontalPanScan;       /* see bavc.h BAVC_MFD_Picture */
  int32_t i32_VerticalPanScan;         /* see bavc.h BAVC_MFD_Picture */

  uint32_t   ulAvgContentLight;       /* see bavc.h BAVC_MFD_Picture */
  uint32_t   ulMaxContentLight;       /* see bavc.h BAVC_MFD_Picture */
  BAVC_Point stDisplayPrimaries[3];   /* see bavc.h BAVC_MFD_Picture */
  BAVC_Point stWhitePoint;            /* see bavc.h BAVC_MFD_Picture */
  uint32_t   ulMaxDispMasteringLuma;  /* see bavc.h BAVC_MFD_Picture */
  uint32_t   ulMinDispMasteringLuma;  /* see bavc.h BAVC_MFD_Picture */

  bool bStreamProgressive;             /* unfiltered version of BXVD_P_PPB_FLAG_PROG_SEQUENCE */

  bool bStreamProgressive_7411;        /* 7411 style progressive indication */
                                       /* i.e. filtered version of BXVD_P_PPB_FLAG_PROG_SEQUENCE */

  uint32_t  uiProfile;                 /* stream encoding profile from the sequence extension */
  uint32_t  uiLevel;                   /* stream encoding level from the sequence extension */

  uint32_t  uiBitRate;                 /* bit rate from sequence header */
  uint32_t  uiVideoFormat;             /* video format from the sequence display extension */
  uint32_t  uiLowDelayFlag;            /* delay flag from the sequence extension */
  uint32_t  uiFrameRateExtN;           /* frame rate extension N from the sequence extension */
  uint32_t  uiFrameRateExtD;           /* frame rate extension D from the sequence extension */
  uint32_t  uiPictureCodingType;       /* frame type from the picture header */
  uint32_t  uiFixedFrameRateFlag;      /* fixed frame rate flag delivered in the PPB */

  uint32_t  uiMacroBlockCntInter;      /* inter macro block count calculated by the video decoder */
  uint32_t  uiMacroBlockCntIntra;      /* intra macro block count calculated by the video decoder */
  uint32_t  uiMacroBlockCntTotal;      /* total macro block count calculated by the video decoder */

   /* This field indicate if valid AFD present present in the stream or not.
    * if bValidAfd=true, then ulAfd can then be used. */
   bool bValidAfd;

   /* AFD values defined by User data registered by ITU-T Recommendation T.35
    * SEI message (see ISO/IEC 14496-10:2005, Subclauses D.8.5 and D.9.5).
    * The AFD value describe the area of interest of the stream. */
   uint32_t uiAfd;

   /* SW7405-4378: save the unaltered width and height for use by the application. */
   uint32_t uiCodedSourceHeight;
   uint32_t uiCodedSourceWidth;

   /* SW7445-1023: report the depth of the picture buffers */
   BAVC_VideoBitDepth eBitDepth;

} BXVD_PictureParameterInfo;

/***************************************************************************
Summary:
  Callback structure for BXVD_Interrupt_ePPBParameters.  This
  structure contains PPB specific information and is delivered each
  time a PPB is evaluated from the picture delivery queue.
****************************************************************************/
/* the following provide a mapping for the symbol names of the types used in
   this structure and its members to the new XDM structuring
   ***** IMPORTANT ****
   It is expected that the *names* of the members of the structures are IDENTICAL
*/
#define BXVD_PPBParameterInfo BXDM_Picture
#define BXVD_PPB_PTS          BXDM_Picture_PTS
#define BXVD_PPB_PictureTag   BXDM_Picture_Tag
#define BXVD_PPB_Marker       BXDM_Picture_Marker
#define BXVD_PPB_PictureType  BXDM_Picture_Type
#define BXVD_PPB_Error        BXDM_Picture_Error
#define BXVD_PPB_Overscan     BXDM_Picture_Overscan
#define BXVD_PPB_PCROffset    BXDM_Picture_PCROffset

#define BXVD_PictureCoding    BXDM_Picture_Coding

#define BXVD_PictureCoding_eUnknown    BXDM_Picture_Coding_eUnknown
#define BXVD_PictureCoding_eI          BXDM_Picture_Coding_eI
#define BXVD_PictureCoding_eP          BXDM_Picture_Coding_eP
#define BXVD_PictureCoding_eB          BXDM_Picture_Coding_eB
#define BXVD_PictureCoding_eMax        BXDM_Picture_Coding_eMax


#define BXVD_TSMResult BXDM_PictureProvider_TSMResult

#define BXVD_TSMResult_eTooEarly BXDM_PictureProvider_TSMResult_eTooEarly /* (PTS > STC)  and ((PTS - STC) > discard threshold) */
#define BXVD_TSMResult_eWait     BXDM_PictureProvider_TSMResult_eWait     /* (PTS > STC)  and ((PTS - STC) <= discard threshold) */
#define BXVD_TSMResult_ePass     BXDM_PictureProvider_TSMResult_ePass     /* (PTS <= STC) and ((STC - PTS) <= too late threshold) */
#define BXVD_TSMResult_eTooLate  BXDM_PictureProvider_TSMResult_eTooLate  /* (PTS < STC)  and ((STC - PTS) > too late threshold) */
#define BXVD_TSMResult_eDrop     BXDM_PictureProvider_TSMResult_eDrop     /* Drop for non-TSM reason (freeze, pts filtering, invalid pts, invalid pcr, etc) */
#define BXVD_TSMResult_eMax      BXDM_PictureProvider_TSMResult_eMax


#define BXVD_PictureHandlingMode BXDM_PictureProvider_PictureHandlingMode

/* In Broadcast:
   - TooEarly --> eDrop
   - TooLate --> eHonorPTS
   In Playback:
   - TooEarly --> eHonorPTS for 3 vsyncs.  eDrop after 3 vsyncs.  Reset on non-TooEarly.
   - TooLate --> eIgnorePTS for first picture.  eHonorPTS for susequent pictures.  Reset on non-TooLate.
*/
#define BXVD_PictureHandlingMode_eDefault    BXDM_PictureProvider_PictureHandlingMode_eDefault

/* Honor the PTS of the picture and assume it is correct */
#define BXVD_PictureHandlingMode_eHonorPTS   BXDM_PictureProvider_PictureHandlingMode_eHonorPTS

/* Ignore the PTS of the picture and instead interpolate from the previous PTS */
#define BXVD_PictureHandlingMode_eIgnorePTS  BXDM_PictureProvider_PictureHandlingMode_eIgnorePTS

/* Drop the picture immediately */
#define BXVD_PictureHandlingMode_eDrop       BXDM_PictureProvider_PictureHandlingMode_eDrop

/* SW7635-51: give the middleware/application the option to hold off processing a picture. */
#define BXVD_PictureHandlingMode_eWait       BXDM_PictureProvider_PictureHandlingMode_eWait

#define BXVD_PictureHandlingMode_eMax        BXDM_PictureProvider_PictureHandlingMode_eMax

typedef struct BXVD_TSMInfo
{
   BXVD_TSMResult eTSMResult; /* The TSM Result for the currently evaluated picture.  *read only* */
   BXVD_PictureHandlingMode ePictureHandlingMode; /* The picture handling mode for the currently evaluated picture */
} BXVD_TSMInfo;


#define BXVD_FrameRateDetectionMode BXDM_PictureProvider_FrameRateDetectionMode

/* Disable frame rate detection.  Will always return unknown */
#define BXVD_FrameRateDetectionMode_eOff  BXDM_PictureProvider_FrameRateDetectionMode_eOff

/* Fast frame rate detection.  Will return best guess as soon as
 * possible.  Result may oscillate initially for very close frame
 * rates (e.g. 29.97 vs 30 fps)
 */
#define BXVD_FrameRateDetectionMode_eFast BXDM_PictureProvider_FrameRateDetectionMode_eFast

/* Stable frame rate detection.  Will return "unknown" until frame
 * rate stabilizes.  Good for differentiating between close frame
 * rates (e.g. 29.97 vs 30 fps)
 */
#define BXVD_FrameRateDetectionMode_eStable  BXDM_PictureProvider_FrameRateDetectionMode_eStable

#define BXVD_FrameRateDetectionMode_eMax  BXDM_PictureProvider_FrameRateDetectionMode_eMax


typedef enum BXVD_ProgressiveOverrideMode
{
   /* Assume 480p/576p/1080p content with pulldown="F" is
    * pulldown="TB" + source_format="Progressive" */
   BXVD_ProgressiveOverrideMode_eTopBottom,

   /* Assume 480p/576p/1080p content with pulldown="F" is
    * pulldown="BT" + source_format="Progressive" */
   BXVD_ProgressiveOverrideMode_eBottomTop,

   /* Assume 480p/576p/1080p content with pulldown="F" is
    * correct */
   BXVD_ProgressiveOverrideMode_eDisable,

   BXVD_ProgressiveOverrideMode_eMax
} BXVD_ProgressiveOverrideMode;

/* By default, 240i content is scanned out as progressive.
 *
 * Basically the content on MPEG-1 and VCD used 352x240
 * progressive for video. The transition to MPEG-2 caused
 * confusion to the encoders at the time, and they
 * mis-labeled some streams 240i for some streams, but we
 * found the content was actually progressive in almost every
 * case we could find. It was just experimental evidence that
 * showed better performance scanning out progressive.
 *
 * The customer can disable 240i force progressive scanout
 * if they know their content is encoded properly (e.g. a mosaic
 * channel, or a transcoded downsampled video sequence)
 */
typedef enum BXVD_240iScanMode
{
   /* Assume non-AVC 240i/288i has pulldown="F" (default) */
   BXVD_240iScanMode_eForceProgressive,

   /* Assume non-AVC 240i/288i is coded correctly */
   BXVD_240iScanMode_eUseEncodedFormat,

   BXVD_240iScanMode_eMax
} BXVD_240iScanMode;

/* SWSTB-68: how to handle a picture if the PCR is marked as discontinuous,
 * currently only used at the beginning of start decode.
 * Passed in BXVD_DecodeSettings.ePCRDiscontinuityModeAtStartup.
 * BXVD_PCRDiscontinuityMode_eDrop - drop the picture
 * BXVD_PCRDiscontinuityMode_eVsync - evaluate the picture in vsync mode
 * BXVD_PCRDiscontinuityMode_eLegacy - evaluate the picture in TSM mode */

#define BXVD_PCRDiscontinuityMode            BXDM_PictureProvider_PCRDiscontinuityMode

#define BXVD_PCRDiscontinuityMode_eDrop      BXDM_PictureProvider_PCRDiscontinuityMode_eDrop
#define BXVD_PCRDiscontinuityMode_eVsync     BXDM_PictureProvider_PCRDiscontinuityMode_eVsync
#define BXVD_PCRDiscontinuityMode_eLegacy    BXDM_PictureProvider_PCRDiscontinuityMode_eLegacy
#define BXVD_PCRDiscontinuityMode_eMax       BXDM_PictureProvider_PCRDiscontinuityMode_eMax

/***************************************************************************
Summary:
   SW7425-2686: for multi-pass DQT, add a generic structure for setting
   trick modes as well MP DQT specific enums and structure.

   The BXVD_Interrupt_eEndOfGOP callback will return a BXVD_DQTStatus structure.

****************************************************************************/

typedef struct BXVD_DQTStatus
{
   uint32_t uiIntraGOPIndex;  /* Trigger index for the next pass. A '0' indicates that
                               * this GOP has been full displayed.  */

   uint32_t uiOpenGopPictures;   /* The number of open GOP pictures in this GOP. Should be
                                  * added to the value passed by the "display_n" BTP command. */
} BXVD_DQTStatus;

typedef enum BXVD_DQTTrickMode
{
   BXVD_DQTTrickMode_eDisable,         /* normal playback. */
   BXVD_DQTTrickMode_eDQT,             /* enables the orignal DQT logic. */
   BXVD_DQTTrickMode_eMultiPassDQT,    /* enables the multi-pass DQT logic. */
   BXVD_DQTTrickMode_eMax

} BXVD_DQTTrickMode;

typedef struct BXVD_TrickModeSettings
{
   /* TODO: Add other XVD trick mode settings here (e.g. Sparse Mode, etc) */

   struct
   {
      BXVD_DQTTrickMode eMode;
      /* The following are only used when eMode == BXVD_DQTTrickMode_eMultiPassDQT */
      uint32_t          uiTargetPTS;      /* PTS on which to begin reverse playback. */
      BXVD_PTSType      eTargetPTSType;   /* Need to know if the target PTS is a coded PTS. */
      uint32_t          uiTargetIndex;    /* Position of the picture within the GOP, 1 based. */
   } stGopTrickMode;


} BXVD_TrickModeSettings;

/***************************************************************************
Summary:
XVD callback function typedef. This is different from the BINT_CallbackFunc
definition in that it allows for an extra parameter to be passed to the
callback which contains XVD specific data.
****************************************************************************/
typedef void (*BXVD_IntCallbackFunc)(void *pParm1, int parm2, void *pXVD_data);

#define BXVD_NUM_EXT_RAVE_CONTEXT 1


/***************************************************************************
Summary:
SWSTB-3450: add support for passing in the XDM start/stop parameters.  This is
part of a long term goal of getting away from all the separate Set/Get API's.
****************************************************************************/

#define BXVD_StartSettings BXDM_PictureProvider_StartSettings
#define BXVD_StopSettings BXDM_PictureProvider_StopSettings

/***************************************************************************
 * Summary:
 *  Input structure to the BXVD_StartDecode() call.
 *  ulFrameRate - bitmask specifying multiple refresh rates to choose from
 *  Defined in bfmt.h(BFMT_VERT_50Hz, BFMT_VERT_59_94Hz, BFMT_VERT_60Hz)
 *
 * NOTE:
 *  The following structure members are deprecated and should not be referenced
 *  in new code:
 *     uint32_t ulMultiStreamId;
 *     uint32_t ulVideoSubStreamId;
 *     BAVC_XptOutput stDataXprtOutput;
 *     BAVC_Timebase  eTimeBase;
 *
 */
typedef struct BXVD_DecodeSettings
{
  BAVC_VideoCompressionStd  eVideoCmprStd;  /* Video compression standard */
  uint32_t ulMultiStreamId;                 /* DEPRECATED: StreamId for compressed multistream */
  uint32_t ulVideoSubStreamId;              /* DEPRECATED: PES header streamID for DirectTV TS */

  bool bPlayback;                           /* decoder is in playback mode */
  bool bCrcMode;                            /* DM is in CRC checking mode */

  BAVC_XptOutput stDataXprtOutput;          /* DEPRECATED: Data Xpt output and Playback info */
  BAVC_Timebase  eTimeBase;                 /* DEPRECATED: Time base info */

  BAVC_XptContextMap *pContextMap;      /* RAVE context used for this stream */
  BAVC_XptContextMap *aContextMapExtended[BXVD_NUM_EXT_RAVE_CONTEXT]; /* RAVE exxtended context used for this stream */
  uint32_t uiContextMapExtNum;               /* Number of extended rave contexts */
  uint32_t uiSignature;                     /* Signature written if BXVD_GetDecodeDefaultSettings is called */

  BXVD_STC eSTC;                            /* STC used by this channel */
  BXVD_DisplayInterrupt eDisplayInterrupt;  /* FW PictDataRdy Interrupt to be used by this channel */
  uint32_t uiVDCRectangleNum;               /* VDC rectangle number, multi-decode mode */
  bool bAstmMode;                           /* Set DM to Adaptive System Time Management (ASTM) mode */

  /* if the STC is valid, continue displaying pictures even if the PCR
   * offset becomes invalid. */
  bool bVsyncModeOnPcrDiscontinuity;

  BXVD_HITSMode eHITSMode;                  /* MPEG HITS mode */
  bool bZeroDelayOutputMode;                /* Deliver pictures when decoded, no delay */
  BAVC_FrameRateCode eDefaultFrameRate;     /* User-settable default frame rate if unknown from stream */

  uint32_t uiPreRollRate;                   /* essentially a slow motion rate to be applied until a PPB passes TSM test */
  uint32_t uiPreRollNumerator;              /* SW7445-2421: allows for rates > 50%, uiPreRollRate is the denomintor */

  BXVD_Picture_ErrorHandling eErrorHandling;  /* Display Manager picture error handling.  */

  BXVD_FrameRateDetectionMode eFrameRateDetectionMode; /* Frame Rate Detection Mode that should be
                                                        * used if both the coded frame rate and
                                                        * eDefaultFrameRate are unknown */
  bool bBluRayDecode;                       /* Blu-ray decode mode */
  BXVD_ProgressiveOverrideMode eProgressiveOverrideMode; /* 480p/576p/1080p override mode */
  BXVD_TimestampMode eTimestampMode;        /* Timestamps can be either Decode or Display order */
  bool bIFrameAsRAP;                        /* Treat I Frame as a RAP when decoding AVC */
  bool bAVCErrorConcealmentMode;            /* Enable AVC error concealment */
  bool bIOnlyFieldOutputMode;               /* Enable AVC I only field output mode */
  bool bIgnoreDPBOutputDelaySyntax;         /* SW3556-1058:: ignore the DPB output delay syntax */
  uint32_t uiSEIMessageFlags;               /* Enable various Userdata SEI message features */
  bool bPFrameSkipDisable;                  /* Disable P frame skip mode */
  bool bExternalPictureProviderMode;        /* An external picture provider interface is being used */
  bool bAVCAspectRatioOverrideMode;         /* Use current decode previous valid AR if not specified in SPS */
  bool bSVC3DModeEnable;                    /* Enable 3D decode for SVC protocol */
  bool bSWCoefAVCDecodeModeEnable;          /* Enable SW coefficient decode of AVC streams */
  bool bIgnoreNumReorderFramesEqZero;       /* Ignore AVC Num Reorder Frames equal zero */
  bool bEarlyPictureDeliveryMode;           /* Enable Early Picture Delivery Mode */
  struct BXVD_DecodeSettings * pstEnhancedSettings;  /* SW7425-1064: with linked channels, BXVD_StartDecode will be called */
                                                     /* once for both channel, hence the need to link the decode settings. */
  bool bUserDataBTPModeEnable;              /* Enable Userdata in BTP mode */
  bool bNRTModeEnable;                      /* Enable NRT decode mode */

  BXVD_PCRDiscontinuityMode   ePCRDiscontinuityModeAtStartup;  /* SWSTB-68: drop, vsync or legacy. */

  uint32_t uiErrorThreshold;  /* SWSTB-439: drop a picture if the percentage of macro blocks with an error
                               * is greater than or equal to this value. The range of uiErrorThreshold is 0:100.
                               * Evaluated when eErrorHandlingMode is either ePicture or ePrognostic. */

  BXVD_StartSettings stXDMSettings; /* SWSTB-3450: the XDM start settings. */

} BXVD_DecodeSettings;

/*
 * SEI Message flag definitions
 */
#define BXVD_DECODESETTINGS_SEI_MESSAGE_FRAMEPACKING 1

/***************************************************************************
Summary:
  Structure for setting device VDC interrupts.

Description:

  Some of the platforms have the capability to generate more than one
  PictureDataReady interrupt to a decoder. Also, one interrupt may trigger
  more than one decoder. One decoder could be linked to another decoder. If
  the decoder is linked to another decoder, the interrupt service routine
  will be invoked via XVD device ISR context of primary decoder.

See Also:
****************************************************************************/
typedef struct BXVD_DeviceVdcInterruptSettings
{
  BINT_Id                 VDCIntId_Topfield; /* [in] VDC interrupt for Top field */
  BINT_Id                 VDCIntId_Botfield; /* [in] VDC interrupt for Bottom field */
  BINT_Id                 VDCIntId_Frame;    /* [in] VDC interrupt for Frame */
  BXVD_DisplayInterrupt   eDisplayInterrupt; /* To indicate which
                                              * picture data ready interrupt */
  uint32_t                uiFlags;           /* Various flags */
  BXDM_DisplayInterruptHandler_Handle hAppXdmDih; /* Application supplied Display Interrupt Handle */
} BXVD_DeviceVdcInterruptSettings;

/* Linked - to disable the ISR for this Display interrupt */
/* UseFieldAsFrame - to treat top/bot fields as frame  */

#define BXVD_DeviceVdcIntrSettingsFlags_None              0
#define BXVD_DeviceVdcIntrSettingsFlags_Linked            1
#define BXVD_DeviceVdcIntrSettingsFlags_UseFieldAsFrame   2


/***************************************************************************
Summary:
    Struct return in BXVD_GetRevision
****************************************************************************/
typedef struct BXVD_RevisionInfo
{
  unsigned long             ulDecoderFwRev;  /* Decoder ARC FW version */
} BXVD_RevisionInfo;

/***************************************************************************
Summary:
  Structure for returning all display threshold values (in 45KHz clock ticks)

Description:
  This structure contains display thresholds that were returned by separate
  calls in XVD. This structure can be added to if new display thresholds
  become available. The values are all expressed as ticks of the 45KHz clock.

See Also:
  BXVD_GetDisplayThresholds
****************************************************************************/
typedef struct BXVD_DisplayThresholds
{
  uint32_t  ui32ThresholdLo;
  uint32_t  ui32ThresholdHi;
  uint32_t  ui32DiscardThreshold;
  uint32_t  ui32VeryLateThreshold;
} BXVD_DisplayThresholds;

/***************************************************************************
Summary:
    Enum used to select the type of field display.

Description:
    We can select top field only, bottom field or both field display.

See Also:
    BXVD_SetDisplayFieldMode, BXVD_GetDisplayFieldMode, BXVD_GetDisplayFieldMode_isr

Below is an explanation of the new eDisplayFieldMode mode "BXVD_DisplayFieldType_eAuto" and
the supporting API "BXVD_SetDeinterlacerMaxHeight".

For normal play and fast forward, eDisplayFieldMode should be set to "eBothField".
The question is how to set eDisplayFieldMode for slow motion and pause.
If there is an "appropriate" deinterlacer in the output path, it should be set to
"eBothField" for the best results.  If there is NOT an "appropriate" deinterlacer,
it should be set to "eSingleField"; this will minimize jitter at the expense of a lower
resolution image. The new eDisplayFieldMode of "BXVD_DisplayFieldType_eAuto" solves this
problem by automatically selecting either "eBothField" or "eSingleField" during slow
motion and pause.

Having an "appropriate" deinterlacer is defined as:
- every output is driven by a deinterlacer
- AND the least capable deinterlacer can support the stream height
- AND the deinterlacer(s) are enabled

The API "BXVD_SetDeinterlacerMaxHeight" is used to set the variable "uiMaxHeightSupportedByDeinterlacer".
The application/middleware will use "uiMaxHeightSupportedByDeinterlacer" to indicate if there
is an "appropriate" deinterlacer.  If "uiMaxHeightSupportedByDeinterlacer" is set to "0",
there is NOT an "appropriate" deinterlacer and the scan out will be "eSingleField".
If "uiMaxHeightSupportedByDeinterlacer" is set to a non-zero value, the scan out will be "eBothField" if
the stream height is less than or equal to "uiMaxHeightSupportedByDeinterlacer".
If the height is greater than "uiMaxHeightSupportedByDeinterlacer", the scan out will be "eSingleField".

The following pseudo code summaries the behavior:

   if ( stGlobal.eDisplayFieldMode == BXDM_PictureProvider_DisplayFieldMode_eAuto )
   {
      // Default to eBothField

      stLocal.eDisplayFieldMode = BXDM_PictureProvider_DisplayFieldMode_eBothField;

      // When paused or in slow motion, revert to eSingleField if the deinterlacer cannot support the stream.

      if ( stSource.uiHeight > uiMaxHeightSupportedByDeinterlacer
            && true == bPausedOrSlowMotion
         )
      {
         stLocal.eDisplayFieldMode = BXDM_PictureProvider_DisplayFieldMode_eSingleField;
      }

   }


****************************************************************************/
typedef enum BXVD_DisplayFieldType
{
  BXVD_DisplayFieldType_eBothField,       /* regular both field display */
  BXVD_DisplayFieldType_eTopFieldOnly,    /* Display Top fld only */
  BXVD_DisplayFieldType_eBottomFieldOnly,  /* Display Bottom fld only */
  BXVD_DisplayFieldType_eSingleField, /* Display single field only.
                                       * The field that is displayed
                                       * is the same as the one that
                                       * is currently being
                                       * displayed. */
  BXVD_DisplayFieldType_eAuto,     /* SW7405-4117: select either eSingleField or eBothField based on the
                                        * steam height during slow motion (and preroll). */
  BXVD_DisplayFieldType_eMaxFieldType
} BXVD_DisplayFieldType;

/***************************************************************************
Summary:
    Enum used to select display modes

Description:
    We can select either to use PTS or ignore PTS, time-stamp or non time-stamp
 mode.

See Also:
    BXVD_SetVideoDisplayMode, BXVD_GetVideoDisplayMode

****************************************************************************/
/* Display timing mode */
typedef enum BXVD_DisplayMode
{
  BXVD_DisplayMode_eTSMMode,   /* Use PTS for display timing */
  BXVD_DisplayMode_eVSYNCMode, /* Ignore PTS and follow pulldown */
  BXVD_DisplayMode_eMaxModes
} BXVD_DisplayMode;

/***************************************************************************
Summary:
    Structure for returning the GOP time code value

Description:
    SMPTE time codes coming in the MPEG stream as per ISO/IEC 13818-2
        standard.
See Also:
    BXVD_GetGopTimeCode

****************************************************************************/
typedef struct BXVD_GopTimeCode
{
  unsigned long ulTimeCodeHours;    /* The hours field */
  unsigned long ulTimeCodeMinutes;  /* The minutes field */
  unsigned long ulTimeCodeSeconds;  /* The seconds field */
  unsigned long ulTimeCodePictures; /* The pictures (frames) field */
  bool bTimeCodeValid;              /* Valid timecode in input stream */
} BXVD_GopTimeCode;

/***************************************************************************
Summary:
    Enum used to specify interrupt enable/disable status.

Description:
    This enum is used in calls to BXVD_EnableInterrupt. This enum
        along with the interrupt specified sets the enabled or disabled
        status of the interrupt.

See Also:
    BXVD_EnableInterrupt

****************************************************************************/
typedef enum BXVD_InterruptEnable
{
   BXVD_InterruptEnable_eDisable = 0,
   BXVD_InterruptEnable_eEnable
} BXVD_InterruptEnable;


#define BXVD_ClipTimeType BXDM_PictureProvider_ClipTimeType

 /* Disables all clip logic. If clipping is currently active, it
  * will be disabled and normal TSM will resume. If a clip time was
  * previously set (but not yet used), it will be reset
  */
#define BXVD_ClipTimeType_eDisable  BXDM_PictureProvider_ClipTimeType_eDisable

/* Both clip start and stop times are valid w/ DVD specific
 * functionality:
 * - Pictures causing PTS Errors are displayed in vsync mode
 * - Start Time of 0 means start immediately
 * - Stop Time of 0xFFFFFFFF means stop on a marker
 * - Stop Event:
 *   - TSM Mode: STC value is compared to stop time
 *   - vTSM Mode: PTS value is compared to stop time
 *   - EOC/VSYNC Mode: stop time is ignored.  Marker is used to indicate stop.
 */
#define BXVD_ClipTimeType_eEnableBothWithDVDCompatibility   BXDM_PictureProvider_ClipTimeType_eEnableBothWithDVDCompatibility

/* Both clip start and stop times are valid */
#define BXVD_ClipTimeType_eEnableBoth     BXDM_PictureProvider_ClipTimeType_eEnableBoth

/* Only clip start time is valid */
#define BXVD_ClipTimeType_eClipStartOnly  BXDM_PictureProvider_ClipTimeType_eClipStartOnly

/* Only clip stop time is valid */
#define BXVD_ClipTimeType_eClipStopOnly   BXDM_PictureProvider_ClipTimeType_eClipStopOnly

#define BXVD_ClipTimeType_eMax            BXDM_PictureProvider_ClipTimeType_eMax



/***************************************************************************
Summary:
    Enum used to specify the pulldown override mode for MPEG content.

Description:

    This enum is used in calls to BXVD_SetMPEGPulldownOverride.
    When "BXVD_MPEGPulldownOverride_eRFF" is specified, the Display Manager
    will look at the "RepeatFirstField" flag delivered with the picture.
    If "RepeatFirstField" is set for a given picture, the pulldown will
    be overridden as follows;
      TB -> TBT
      BT -> BTB

See Also:

****************************************************************************/

typedef enum BXVD_MPEGPulldownOverride
{

   BXVD_MPEGPulldownOverride_eDefault,    /* use the pulldown information as supplied with the PPB */
   BXVD_MPEGPulldownOverride_eRFF,        /* override the pulldown based on the RepeatFirstField flag
                                             delivered with the PPB. */
   BXVD_MPEGPulldownOverride_eMax

} BXVD_MPEGPulldownOverride;

/***************************************************************************
Summary:

Description:

See Also:

****************************************************************************/

typedef struct BXVD_PlaybackRateSettings
{
   uint16_t uiNumerator;   /* a value of '0' will cause the system to pause */
   uint16_t uiDenominator; /* a value of '0' will generate an BERR_INVALID_PARAMETER error */

} BXVD_PlaybackRateSettings;

/***************************************************************************
Summary:

Description:

See Also:

****************************************************************************/

typedef struct BXVD_FrameRate
{
      uint16_t uiNumerator;   /* 0 indicates unknown rate */
      uint16_t uiDenominator; /* 0 indicates unknown rate */
} BXVD_FrameRate;

/* Temporarily add this define for backward compatibility with Nexus */
#define BXVD_FrameRateSettings BXVD_FrameRateOverride

typedef struct BXVD_FrameRateOverride
{
   bool           bValid;
   BXVD_FrameRate stRate;

   bool           bTreatAsSingleElement;  /* SWDEPRECATED-1003: assume FrameX1 when calculating
                                             the predicted PTS value of the next picture. */
} BXVD_FrameRateOverride;

/***************************************************************************
Summary:
SW7422-72: structure and constant defintions for setting the 3D orientation
of pictures.

See Also:
See comment at BXVD_Set3D

****************************************************************************/

#define BXVD_3DSetting  BXDM_PictureProvider_3DSettings

#define BXVD_Orientation     BXDM_PictureProvider_Orientation

#define BXVD_Orientation_e2D                             BXDM_PictureProvider_Orientation_e2D
#define BXVD_Orientation_eLeftRight                      BXDM_PictureProvider_Orientation_eLeftRight
#define BXVD_Orientation_eOverUnder                      BXDM_PictureProvider_Orientation_eOverUnder
#define BXVD_Orientation_eLeftRightFullFrame             BXDM_PictureProvider_Orientation_eLeftRightFullFrame
#define BXVD_Orientation_eRightLeftFullFrame             BXDM_PictureProvider_Orientation_eRightLeftFullFrame
#define BXVD_Orientation_eLeftRightEnhancedResolution    BXDM_PictureProvider_Orientation_eLeftRightEnhancedResolution
#define BXVD_Orientation_eMax                            BXDM_PictureProvider_Orientation_eMax

/***************************************************************************
Summary:
SW7425-1264: support for a synthesized STC; can create a clock that run backwards.
See bxdm_pp.h for the definition of BXDM_PictureProvider_ClockOverride.

****************************************************************************/

#define BXVD_ClockOverride BXDM_PictureProvider_ClockOverride

/*******************/
/* XVD Device APIs */
/*******************/

/******************************************************************************
Summary:
    Get the default settings for the XVD device.

Returns:
    BERR_SUCCESS - Successfully opened XVD connection.
    BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:
*******************************************************************************/
BERR_Code BXVD_GetDefaultSettings
(
  BXVD_Settings    *pDefaultSetings   /* [out] Default XVD settings */
);

/***************************************************************************
Summary:
    Opens and initializes XVD

Description:
    Applications are required to call this function once, prior to any
    other XVD calls.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully opened XVD.

See Also:
    BXVD_Close, BXVD_GetDefaultSettings.
**************************************************************************/
BERR_Code BXVD_Open
(
   BXVD_Handle                *phXvd,       /* [out] XVD handle returned */
   BCHP_Handle                 hChip,       /* [in] chip handle */
   BREG_Handle                 hRegister,   /* [in] register handle */
   BINT_Handle                 hInterrupt,  /* [in] interrupt handle */
   const BXVD_Settings        *pDefSettings /* [in] default XVD settings */
   );

/******************************************************************************
Summary:
    Release allocated resources and close XVD

Description:
    Applications must call this function before it terminates.

Returns:
    BERR_SUCCESS - Successfully closed the XVD connection.
    BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:
    BXVD_Open
*******************************************************************************/
BERR_Code BXVD_Close
(
   BXVD_Handle     hXvd    /* [in] XVD handle */
   );

#if BXVD_P_POWER_MANAGEMENT
/******************************************************************************
Summary:
    Put decoder in Standby mode.

Description:
    Save current power state, set power state to power Off

Returns:
    BERR_SUCCESS - Successfully closed the XVD connection.
    BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:
    BXVD_Resume
*******************************************************************************/

BERR_Code BXVD_Standby
   (
   BXVD_Handle     hXvd    /* [in] XVD handle */
   );

/******************************************************************************
Summary:
    Restore decoder to pre-standby state.

Description:
    Set power state to on, boot decoder and restore power state to saved
    standby power state

Returns:
    BERR_SUCCESS - Successfully closed the XVD connection.
    BERR_INVALID_PARAMETER - Invalid function parameter.

See Also:
    BXVD_Standby

*******************************************************************************/
BERR_Code BXVD_Resume
   (
   BXVD_Handle     hXvd    /* [in] XVD handle */
   );
#endif

#if !B_REFSW_MINIMAL /* SWSTB-461 */
/***************************************************************************
Summary:
    API used to retrieve the max video channels supported.
Description:
    returns the number of video channels supported.
Returns:
    BERR_SUCCESS

See Also:
****************************************************************************/
BERR_Code BXVD_GetTotalChannels
(
   BXVD_Handle       hXvd,                /* [in] XVD device handle */
   unsigned         *puiTotalChannels     /* [out] Total channels available */
   );

/***************************************************************************
Summary:
    ISR version of BXVD_GetTotalChannels

See Also:
   BXVD_GetTotalChannels
***************************************************************************/
BERR_Code BXVD_GetTotalChannels_isr
(
   BXVD_Handle       hXvd,                /* [in] XVD device handle */
   unsigned         *puiTotalChannels     /* [out] Total channels available */
   );
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
Summary:
    API used to retrieve the 7401 FW version
Description:
    Returns Decoder ARC FW revision
Returns:
    BERR_SUCCESS

See Also:
****************************************************************************/
BERR_Code BXVD_GetRevision
(
   BXVD_Handle              hXvd,       /* [in] XVD device handle */
   BXVD_RevisionInfo       *psRevInfo   /* [out] FW and HW revision info */
   );

/***************************************************************************
Summary:
    ISR version of BXVD_GetRevision

See Also:
   BXVD_GetRevision
***************************************************************************/
BERR_Code BXVD_GetRevision_isr
(
   BXVD_Handle              hXvd,       /* [in] XVD device handle */
   BXVD_RevisionInfo       *psRevInfo   /* [out] FW and HW revision info */
   );

/***************************************************************************
 Summary:
         Link two decoders picture procssing

 Description:
         Routine used to link the primary decoder picture processing
 with the processing of the secondary decoders pictures. Two decoders
 are needed to decode 12 CIF video streams which will be displayed on
 a single display device.
***************************************************************************/
void BXVD_LinkDecoders
(
   BXVD_Handle hXvd_Primary,
   BXVD_Handle hXvd_Secondary,
   BXVD_DisplayInterrupt eDisplayInterrupt
   );

/***************************************************************************
Summary:
        Unlink two decoders picture procssing

Description:
        Routine used to unlink the primary decoder picture processing
with the processing of the secondary decoders pictures.
***************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
void BXVD_UnlinkDecoders(
   BXVD_Handle hXvd_Primary
   );
#endif

/***************************************************************************************
Summary:
    BXVD_GetVdcDeviceInterruptDefaultSettings:
    Gets the DeviceVdcInterrupt default settings.
Description:
    This API is used to initialize the VDC interrupt structure.
Returns:
    BERR_SUCCESS

See Also:
    BXVD_RegisterVdcDeviceInterrupt

***************************************************************************************/
BERR_Code BXVD_GetVdcDeviceInterruptDefaultSettings
(
   BXVD_Handle                      hXvd,                   /* [in] XVD */
   BXVD_DeviceVdcInterruptSettings  *pDefDevVdcIntrSettings /* [out] default settings */
   );

/***************************************************************************
Summary:
    This API is used to register the VDC interrupt with the video firmware.
Description:
    The VDC PI uses this API to register the channel specific RDMA done
    interrupt with the firmware. The firmware then waits on this interrupt to
    issue the Picture data ready interrupt.

        BXVD_DeviceVdcIntrSettingsFlags_Linked:

        Used to link two decoders picture delivery processing.

        BXVD_DeviceVdcIntrSettingsFlags_UseFieldAsFrame:

        Used to interpret top/bottom fields as frame. This is used by VDC to use
        the GRC block to read the pixel data. GRC functionality requires XVD to be
        configured to change from interpreting top/bottom pixel data to frame pixel
        data. This is done by interpreting the device top or bottom vsyncs as frame
        vsyncs for GRC's use. The decoder does not have to be stopped to change from
        normal interpretation to this UseFieldAsFrame intpretation or back to normal
        again. If the same RULIDMask bits for top/bottom fields are being used, the
        decoder does not have to be in the idle state.

Returns:
    BERR_SUCCESS

See Also:

****************************************************************************/
BERR_Code BXVD_RegisterVdcDeviceInterrupt
(
   BXVD_Handle                      hXvd,                /* [in] XVD */
   BXVD_DeviceVdcInterruptSettings  *pDevVdcIntrSettings /* [in] Settings to control VDC Interrupt */
   );

/***************************************************************************
Summary:
    Used to enable and install an application callback for a decoder
    device relevant interrupt.

Description:
    This API should be used by the application /syslib to enable an XVD
    decoder specific interrupt enumerated by the BXVD_DeviceInterrupt enum.

Returns:
    BERR_SUCCESS
        BERR_INVALID_PARAMETER

See Also:
****************************************************************************/
BERR_Code BXVD_InstallDeviceInterruptCallback
(
   BXVD_Handle hXvd,                /* [in] XVD device handle */
   BXVD_DeviceInterrupt eInterrupt, /* [in] Desired interrupt that needs to be activated */
   BXVD_CallbackFunc fCallBack,     /* [in] XVD callback function */
   void *pParm1,                    /* [in] application specified parameter */
   int  parm2                       /* [in] application specified parameter */
   );

/***************************************************************************
Summary:
    Used to disable and un-install an application callback for a decoder
    device relevant interrupt.

Description:
    This API should be used by the application /syslib to disable an XVD
    decoder specific interrupt, handle enumerated by the
    BXVD_Device Interrupt enum.

Returns:
    BERR_SUCCESS
        BERR_INVALID_PARAMETER

See Also:

****************************************************************************/
BERR_Code BXVD_UnInstallDeviceInterruptCallback
(
   BXVD_Handle hXvd,               /* [in] XVD channel handle */
   BXVD_DeviceInterrupt eInterrupt /* [in] Desired interrupt to disable */
   );

/**************************************************************************
 Summary:
    API returns the buffer requirements for CDB and ITB.
 Description:
    Retrieve maximum ITB/CDB buffer and Picture Buffer configuration for list of decoder
    modes passed required by the application.

 Parameters:
     BXVD_Handle              hXvd           Xvd handle
     BAVC_CdbItbBufferConfig  *CdbItbBufcfg  pointer to AVC common structure to receive maximum size and allignment configuration
     int32_t                  *PicBuflength  pointer to receive maximum size needed for picture buffers

 Returns:
        BERR_SUCCESS  Buffer configuration generated successfully.
        BERR_INVALID_PARAMETER  Bad input parameter
**************************************************************************/

BERR_Code BXVD_GetBufferConfig
(
   BXVD_Handle        hXvd,               /* [in] XVD handle  */
   BAVC_CdbItbConfig  *pCdbItbConfigInfo, /* [out] ptr to size and alignment config structure */
   int32_t            *PicBuflength     /* [out] ptr to picture buffer maximum size */
);

/***************************************************************************
 Summary: BXVD_GetFWMemConfigDefaultSettings

 Description:
    Returns the default values for BXVD_FWMemConfigSettings;

 Returns:
    BERR_SUCCESS  Values set successfully.
    BERR_INVALID_PARAMETER  Bad input parameter
**************************************************************************/
BERR_Code BXVD_GetFWMemConfigDefaultSettings
(
   BXVD_FWMemConfigSettings  *pFWMemConfigSettings /* [out] memory configuration settings */
);

/***************************************************************************
 Summary: BXVD_GetChannelMemoryParameters
    Returns the FW Memory needed for the specified channel settings.

 Description:
    Using the specified channel settings resolution, decode mode and system DDR information
    the FW memory configuration is determined and returned.

 Returns:
    BERR_SUCCESS  Memory configuration generated successfully.
    BERR_INVALID_PARAMETER  Bad input parameter
**************************************************************************/

BERR_Code BXVD_GetChannelMemoryParameters
(
   const BXVD_ChannelSettings      *pChSettings,          /* [in] channel settings */
   const BXVD_FWMemConfigSettings  *pFWMemConfigSettings, /* [in] memory configuration settings */
   BXVD_FWMemConfig                *pFWMemConfig          /* [out] memory configuration for this channel */
);

/***************************************************************************
 Summary: BXVD_GetChannelMemoryConfig
    Returns the FW Memory needed for the specified channel settings.

 Description:
    Using the specified channel settings resolution and decode mode, the FW
    memory configuration is determined and returned.

 Returns:
    BERR_SUCCESS  Memory configuration generated successfully.
    BERR_INVALID_PARAMETER  Bad input parameter
**************************************************************************/

BERR_Code BXVD_GetChannelMemoryConfig
(
   BXVD_Handle                hXvd,           /* [in] XVD handle */
   const BXVD_ChannelSettings *pChSettings,   /* [in] channel settings */
   BXVD_FWMemConfig           *pFWMemConfig   /* [out] memory configuration for this channel */
);

/***************************************************************************
Summary:
  Resets Video Decoder.

Description:
  When the Video Decoder stops updating the Watchdog counter the device needs
  to be reset. This is accomplished by initiializing the hardware state of the
  decoder followed by reloading the decoder firmware. The decoder is started
  using the previously configured Rave context.

Returns:
  BERR_SUCCESS  Decoder reset successful

See Also:
***************************************************************************/
BERR_Code BXVD_ProcessWatchdog
(
   BXVD_Handle hXvd /* [in] XVD context handle */
   );

/***************************************************************************
 Summary: BXVD_GetHardwareCapabilties
    Returns the decoder HW video protocol capabilities

 Description:
    Using the specified video decoder handle, determine the supported
    video compression standards.

 Returns:
    BERR_SUCCESS  Hardware capabilities determined successfully.
    BXVD_ERR_INVALID_HANDLE: BXVD Handle not valid.
**************************************************************************/

BERR_Code BXVD_GetHardwareCapabilities
(
   BXVD_Handle hXvd,                 /* [in] XVD context handle */
   BXVD_HardwareCapabilities *pCap   /* [out] Decoder Video protocol capabilities */
   );

/********************/
/* XVD Channel APIs */
/********************/

/***************************************************************************
Summary:
    Gets the default settings of the desired XVD channel.

Description:

Returns:
    BERR_SUCCESS

See Also:
****************************************************************************/
BERR_Code BXVD_GetChannelDefaultSettings
(
   BXVD_Handle             hXvd,           /* [in] XVD handle */
   unsigned int            uiChannelNum,   /* [in] channel Id */
   BXVD_ChannelSettings   *pChnDefSettings /* [out] default channel settings */
   );

/***************************************************************************
Summary:
    API used to open a video channel.

Description:

Returns:
    BERR_SUCCESS

See Also:
    BXVD_CloseChannel
****************************************************************************/
BERR_Code BXVD_OpenChannel
(
   BXVD_Handle                hXvd,           /* [in] XVD handle */
   BXVD_ChannelHandle         *phXvdCh,       /* [out] channel handle returned */
   unsigned int               uiChannelNum,   /* [in] channel# (starts with zero) */
   const BXVD_ChannelSettings *pChDefSettings /* [in] channel default settings */
   );

/***************************************************************************
Summary:
    API used to close a channel

Description:

Returns:
    BERR_SUCCESS

See Also:
    BXVD_OpenChannel
***************************************************************************/
BERR_Code BXVD_CloseChannel
(
   BXVD_ChannelHandle      hXvdCh          /* [in] The XVD Channel handle */
   );

/***************************************************************************
Summary:
  Gets the status of the specified channel.

Description:
  This API returns the status of the channel specified by the channel handle.
  The picture delivery queue count and picture delivery underflow count is returned
  in the ChannelStatus structure.


Returns:
  BERR_SUCCESS
  BERR_INVALID_PARAMETER

See Also:
****************************************************************************/
BERR_Code BXVD_GetChannelStatus
(
   BXVD_ChannelHandle hXvdCh,          /* [in] channel handle */
   BXVD_ChannelStatus *psChannelStatus /* [out] channel status */
   );

/***************************************************************************
Summary:
  ISR version of BXVD_GetChannelStatus

See Also:
  BXVD_GetChannelStatus
****************************************************************************/
BERR_Code BXVD_GetChannelStatus_isr
(
   BXVD_ChannelHandle hXvdCh,          /* [in] channel handle */
   BXVD_ChannelStatus *psChannelStatus /* [out] channel status */
   );

/***************************************************************************
Summary:
    This API is used to register the VDC interrupt with the video firmware.
Description:
    The VDC PI uses this API to register the channel specific RDMA done
    interrupt with the firmware. The firmware then waits on this interrupt to
    issue the Picture data ready interrupt.
Returns:
    BERR_SUCCESS

See Also:


****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_RegisterVdcInterrupt
(
   BXVD_ChannelHandle  hXvdChn,       /* [in] XVD Channel handle */
   BINT_Id             VDCIntId,      /* [in] VDC interrupt needs to be registered in FW */
   BAVC_Polarity       eFieldPolarity /* [in] Display Polarity for interrupt coming to FW */
   );
#endif

/***************************************************************************
Summary:
    Used to enable and install an application callback for channel
    relevant interrupt.

Description:
    This API should be used by the application /syslib to enable an XVD
    channel specific interrupt enumerated by the BXVD_Interrupt enum and
    can also optionally install a callback.

Returns:
    BERR_SUCCESS
        BERR_INVALID_PARAMETER

See Also:
****************************************************************************/
BERR_Code BXVD_InstallInterruptCallback
(
   BXVD_ChannelHandle hXvdCh,    /* [in] XVD device handle */
   BXVD_Interrupt eInterrupt,    /* [in] Desired interrupt that needs to be activated */
   BXVD_CallbackFunc fCallBack,  /* [in] XVD callback function */
   void *pParm1,                 /* [in] application specified parameter */
   int  parm2                    /* [in] application specified parameter */
   );


/***************************************************************************
Summary:
    Used to disable and un-install an application callback for a channel
    relevant interrupt.

Description:
    This API should be used by the application /syslib to disable an XVD
    specific interrupt for a given channe; handle enumerated by the
    BXVD_Interrupt enum and can also optionally uninstall the callback for
    the same.

Returns:
    BERR_SUCCESS
        BERR_INVALID_PARAMETER

See Also:

****************************************************************************/
BERR_Code BXVD_UnInstallInterruptCallback
(
   BXVD_ChannelHandle hXvdCh, /* [in] XVD channel handle */
   BXVD_Interrupt eInterrupt  /* [in] Desired interrupt to disable */
   );

/***************************************************************************
Summary:
  Used to enable or disable the specified interrupt. ISR version

Description:
  This function is used to either enable or disable the interrupt specified
  in the eInterrupt argument.

Returns:
  BERR_SUCCESS
  BERR_INVALID_PARAMETER

See Also:
  BXVD_EnableInterrupt
****************************************************************************/
BERR_Code BXVD_EnableInterrupt_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_Interrupt eInterrupt,
   BXVD_InterruptEnable eEnable
   );

/***************************************************************************
Summary:
  Used to enable or disable the specified interrupt. Non-ISR version

Description:
  This function is used to either enable or disable the interrupt specified
  in the eInterrupt argument.

Returns:
  BERR_SUCCESS
  BERR_INVALID_PARAMETER

See Also:
  BXVD_EnableInterrupt_isr
****************************************************************************/
BERR_Code BXVD_EnableInterrupt(
   BXVD_ChannelHandle hXvdCh,
   BXVD_Interrupt eInterrupt,
   BXVD_InterruptEnable eEnable
   );


/*******************/
/* XVD Decode APIs */
/*******************/

/***************************************************************************
Summary:
    Gets the default settings of a XVD channel decode.

Description:

Returns:
    BERR_SUCCESS

See Also:
****************************************************************************/
BERR_Code BXVD_GetDecodeDefaultSettings
(
   BXVD_ChannelHandle   hXvdCh,             /* [in] XVD channel handle */
   BXVD_DecodeSettings  *pdecodeDefSettings /* [out] default channel settings */
   );

/***************************************************************************
Summary:
    Starts decode on a given channel.

Description:
    This API is required to start the decode of the selected channel (stream)
    Sets the pid in the Video Transport and enables the specified Decode
    Channel of the Decoder. All the relevant channel/stream  information
    needs to conveyed through the BXVD_BXVD_DecodeSettings struct.

Returns:
    BERR_SUCCESS

See Also:
****************************************************************************/
BERR_Code BXVD_StartDecode
(
   BXVD_ChannelHandle               hXvdChannel,      /* [in] channel handle */
   const BXVD_DecodeSettings       *psDecodeSettings  /* [in] video info */
   );

/***************************************************************************
Summary:
    Stops the decode on a channel.

Description:
    This API stops the decode of the specified channel(stream)
    Disables the pid in the Video Transport and closes the specified Decode
    Channel of the Decoder.

Returns:
    BERR_SUCCESS

See Also:
****************************************************************************/
BERR_Code BXVD_StopDecode
(
   BXVD_ChannelHandle  hXvdChannel    /* [in] channel handle */
   );

/***************************************************************************
Summary:
    Returns the last decode settings for a specific channel.

Description:
    Applications can query the current decode settings for a specific channel.

Returns:
    BERR_SUCCESS

See Also:
****************************************************************************/
BERR_Code BXVD_GetDecodeSettings_isrsafe
(
   const BXVD_ChannelHandle      hXvdCh,             /* [in] channel handle */
   BXVD_DecodeSettings          *psDecodeSettings    /* [out] decode settings */
   );

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetDecodeSettings
(
   const BXVD_ChannelHandle      hXvdCh,             /* [in] channel handle */
   BXVD_DecodeSettings          *psDecodeSettings    /* [out] decode settings */
   );
#endif

/***************************************************************************
Summary:
    Disables the decode on a channel, preparing for a flush.

Description:
    This API flushes all state of the decoder channel. Functionally it's
    equalient to Stop decode, however _all_ decoder state programmed
    by a host is preserved after flush. This function is usually used during
    transitions, when host have introduced discontinuity in the stream, e.g.
    it's often case for transitions between trickmodes.

Returns:
    BERR_SUCCESS

See Also:
   BXVD_StopDecode, BXVD_StartDecode
****************************************************************************/
BERR_Code BXVD_DisableForFlush
(
   BXVD_ChannelHandle  hXvdChannel    /* [in] channel handle */
   );

/***************************************************************************
Summary:
    Flushes the decode on a channel.

Description:
    This API flushes all state of the decoder channel. Functionally it's
    equalient to Start decode, however _all_ decoder state programmed
    by a host is preserved after flush. This function is usually used during
    transitions, when host have introduced discontinuity in the stream, e.g.
    it's often case for transitions between trickmodes.

Returns:
    BERR_SUCCESS

See Also:
   BXVD_StopDecode, BXVD_StartDecode
****************************************************************************/
BERR_Code BXVD_FlushDecode
(
   BXVD_ChannelHandle  hXvdChannel    /* [in] channel handle */
   );

/***************************************************************************
Summary:
  Decodes a still picture specified by RaveContext and type.

Description:
  Decodes a still picture specified by RaveContext and type. The Rave context
  will be passed to the decoder firmware to initiate a still picture decode.

Returns:
  BERR_SUCCESS  Still picture decode in progress
  BERR_INVALID_PARAMETER  Bad input parameter

See Also:

****************************************************************************/
BERR_Code BXVD_DecodeStillPicture
(
   BXVD_ChannelHandle   hXvdCh,       /* [in] The XVD Channel handle */
   BXVD_DecodeStillMode ePictureType, /* [in] Still picture type */
   BAVC_XptContextMap   *pContextMap  /* [in] Rave context map */
   );

/***************************************************************************
Summary:

This routine is used to reset the still picture decoder state.

Description:

When a still picture decode fails a
BXVD_DeviceInterrupt_eDecodedStillBufferReady callback will not be invoked.
This most likely is caused by an error that occurred performing the still picture
decode operation. This routine will reset still picture state, the still
picture channel will be stopped, ready for the next still picture to be decoded
using BXVD_DecodeStillPicture.

Returns:
  BERR_SUCCESS
****************************************************************************/
BERR_Code BXVD_DecodeStillPictureReset
(
   BXVD_ChannelHandle   hXvdCh
   );

/***************************************************************************
Summary:
    Used to set skip picture mode decode.
Description:
    This API can be used to configure the decoder into skipping B pictures
    or B and P pictures to achieve a kind of fastforward.
Returns:
    BERR_SUCCESS

See Also:
****************************************************************************/
BERR_Code BXVD_SetSkipPictureModeDecode
(
   BXVD_ChannelHandle hXvdCh, /* [in] XVD channel handle */
   BXVD_SkipMode eSkipMode    /* [in] new skip mode */
   );

/***************************************************************************
Summary:
    Used to get the current skip picture mode decode.
Description:
    This API is used to get the current skip picture decode mode.
Returns:
    BERR_SUCCESS

See Also:
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetSkipPictureModeConfig
(
   BXVD_ChannelHandle hXvdCh,     /* [in] XVD channel handle */
   BXVD_SkipMode      *peSkipMode /* [out] ptr to current skip mode */
   );
#endif

/************************/
/* Display Manager APIs */
/************************/

/*********************************/
/* Display Manager APIs - Status */
/*********************************/

/***************************************************************************
Summary:
    API used to get the current PTS.
Description:
    This API is used to get the running Presentation Time Stamp from the
    video decoder. The running PTS value and PTS type are updated in the
        BXVD_PTSInfo structure provided by the caller.

Returns:
    BERR_SUCCESS

See Also:
        BXVD_GetPTS_isr
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetPTS
(
   BXVD_ChannelHandle      hXvdCh,   /* [in] XVD Channel handle */
   BXVD_PTSInfo           *pPTSInfo  /* [out] PTS Info is returned*/
   );
#endif

/***************************************************************************
Summary:
    ISR version of BXVD_GetPTS

See Also:
    BXVD_GetPTS
****************************************************************************/
BERR_Code BXVD_GetPTS_isr
(
   BXVD_ChannelHandle      hXvdCh,   /* [in] XVD Channel handle */
   BXVD_PTSInfo           *pPTSInfo  /* [out] PTS Info is returned*/
   );

#if !B_REFSW_MINIMAL /* SWSTB-461 */
/***************************************************************************
Summary:
    API used to get the last coded PTS.
Description:
    This API is used to get the last coded Presentation Time STamp from the
    video decoder. The PTS value returned by this API corresponds to the
    to the last coded PTS that arrived in the stream.
Returns:
    BERR_SUCCESS

See Also:


****************************************************************************/
BERR_Code BXVD_GetLastCodedPTS
(
   BXVD_ChannelHandle   hXvdCh,   /* [in] The XVD Channel handle */
   BXVD_PTSInfo         *pPTSInfo /* [out] PTS Info is returned*/
   );

/***************************************************************************
Summary:
    ISR version of BXVD_GetLastCodedPTS

See Also:
   BXVD_GetLastCodedPTS
***************************************************************************/
BERR_Code BXVD_GetLastCodedPTS_isr
(
   BXVD_ChannelHandle   hXvdCh,   /* [in] The XVD Channel handle */
   BXVD_PTSInfo         *pPTSInfo /* [out] PTS Info is returned*/
   );
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
Summary:
  Used to get the PTS value of the next picture in the picture
  delivery queue.  A coded PTS is returned if one exists, otherwise an
  interpolated PTS is returned.

  If there isn't a next picture available in the queue, an error
  result code is returned.

See Also:
  BXVD_GetNextPTS_isr
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetNextPTS
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_PTSInfo *pPTSInfo /* [out] PTS Info is returned*/
   );
#endif

/***************************************************************************
Summary:
  ISR version of BXVD_GetNextPTS

See Also:
  BXVD_GetNextPTS
****************************************************************************/
BERR_Code BXVD_GetNextPTS_isr
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_PTSInfo *pPTSInfo /* [out] PTS Info is returned*/
   );

#if !B_REFSW_MINIMAL /* SWSTB-461 */
/***************************************************************************
Summary:
  This API retrieves the I picture found status.

Description:
  Used to indicate the presents of a HITS type stream. This is a stream with
  no I pictures and progressive I slice refresh.

Returns:
  BERR_SUCCESS

See Also:

****************************************************************************/
BERR_Code BXVD_GetIPictureFoundStatus
(
   BXVD_ChannelHandle hXvdCh, /* [in] XVD channel handle */
   bool *pbIPictureFound /* [out] current I picture found status */
   );

/***************************************************************************
Summary:
    ISR version of BXVD_GetIPictureFoundStatus

See Also:
    BXVD_GetIPictureFoundStatus
****************************************************************************/
BERR_Code BXVD_GetIPictureFoundStatus_isr
(
   BXVD_ChannelHandle hXvdCh, /* [in] XVD channel handle */
   bool *pbIPictureFound /* [out] current I picture found status */
   );
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
Summary:
    Get the picture tag last seen in the stream
Description:
    This API reads the picture tag that was last seen in the stream
    by decoder. Picture tag is injected by host in bud packets.
Returns:
    BERR_SUCCESS
See Also:
****************************************************************************/
BERR_Code BXVD_GetPictureTag
(
   BXVD_ChannelHandle  hXvdCh,          /* [in] XVD Channel handle */
   unsigned long       *pulPictureTag   /* [Out] current PictureTag */
   );

/***************************************************************************
Summary:
    ISR version of BXVD_GetPictureTag

See Also:
    BXVD_GetPictureTag
****************************************************************************/
BERR_Code BXVD_GetPictureTag_isr
(
   BXVD_ChannelHandle  hXvdCh,          /* [in] XVD Channel handle */
   unsigned long       *pulPictureTag   /* [Out] current PictureTag */
   );

/***************************************************************************
Summary:
    Get the current SMPTE time code as per ISO/IEC 13818-2
Description:
    This API reads the current SMPTE time codes coming in the MPEG stream
        as per the standard ISO/IEC 13818-2. This value typically comes in once
        in every GOP or approximately 15 pictures.
Returns:
    BERR_SUCCESS
See Also:
        None
****************************************************************************/
BERR_Code BXVD_GetGopTimeCode
(
   BXVD_ChannelHandle hXvdCh,       /* [in]  The XVD channel handle    */
   BXVD_GopTimeCode   *pGopTimeCode /* [out] The current GOP time code */
   );

/***************************************************************************
Summary:
    ISR version of BXVD_GetGopTimeCode

See Also:
    BXVD_GetGopTimeCode
****************************************************************************/
BERR_Code BXVD_GetGopTimeCode_isr
(
   BXVD_ChannelHandle hXvdCh,       /* [in]  The XVD channel handle    */
   BXVD_GopTimeCode   *pGopTimeCode /* [out] The current GOP time code */
   );

/**********************************/
/* Display Manager APIs - Display */
/**********************************/

/***************************************************************************
Summary:
    The Application can override the decoder mute with this API.
Description:
    The user can typically mute the video for Pay Per View/Parental Control
    applications. If enabled, XVD will pass a mute request to VDC in the
    call back. If disabled, XVD will pass back the info received from FW.

Returns:
    BERR_SUCCESS

See Also:
***************************************************************************/
BERR_Code BXVD_EnableMute
(
   BXVD_ChannelHandle hXvdCh,   /* [in] channel handle */
   bool bEnable                 /* [in] true=force mute, false=use info passed back from FW */
   );

/***************************************************************************
Summary:
    ISR version of BXVD_EnableMute

See Also:
   BXVD_EnableMute
***************************************************************************/
BERR_Code BXVD_EnableMute_isr
(
   BXVD_ChannelHandle hXvdCh,   /* [in] channel handle */
   bool bEnable                 /* [in] true=force mute, false=use info passed back from FW */
   );

/***************************************************************************
Summary:
    API used to set the display field mode.
Description:
    This function enables various types of field display enumerated by the
    BXVD_DisplayFieldType enum. The various options available are Top field
    only display, Bottom field only display and normal display(both field).

Returns:
    BERR_SUCCESS

See Also:
****************************************************************************/
BERR_Code BXVD_SetDisplayFieldMode
(
   BXVD_ChannelHandle     hXvdCh,           /* [in] XVD Channel handle */
   BXVD_DisplayFieldType  eDisplayFieldType /* [in] display field type */
   );

/***************************************************************************
Summary:
    ISR version of BXVD_SetDisplayFieldMode

See Also:
    BXVD_SetDisplayFieldMode
****************************************************************************/
BERR_Code BXVD_SetDisplayFieldMode_isr
(
   BXVD_ChannelHandle    hXvdCh,
   BXVD_DisplayFieldType eDisplayFieldType
   );

#if !B_REFSW_MINIMAL /* SWSTB-461 */
/***************************************************************************
Summary:
    Gets the video display mode status.
Description:
    This API is used to get the display mode status enumerated by the
    BXVD_DisplayMode.

Returns:
    BERR_SUCCESS

See Also:
****************************************************************************/
BERR_Code BXVD_GetDisplayFieldMode
(
   BXVD_ChannelHandle     hXvdCh,             /* [in] XVD Channel handle */
   BXVD_DisplayFieldType  *peDisplayFieldType /* [out] video display mode */
   );

/***************************************************************************
Summary:
    ISR version pf BXVD_GetDisplayFieldMode

See Also:
    BXVD_GetDisplayFieldMode
****************************************************************************/
BERR_Code BXVD_GetDisplayFieldMode_isr
(
   BXVD_ChannelHandle     hXvdCh,             /* [in] XVD Channel handle */
   BXVD_DisplayFieldType  *peDisplayFieldType /* [out] video display mode */
   );
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
Summary:
  This API sets the channel change mode for DM.

Description:
  DM needs to know how to handle video during/after a channel change. This
  API allows you to set the channel change mode to the type specified by the
  eChChangeMode argument.

Returns:
  BERR_SUCCESS
  BERR_INVALID_PARAMETER

See Also:
  BXVD_GetChannelChangeMode

****************************************************************************/
BERR_Code BXVD_SetChannelChangeMode
(
   BXVD_ChannelHandle hXvdCh,           /* [in] XVD channel handle */
   BXVD_ChannelChangeMode eChChangeMode /* [in] desired channel change mode */
   );

/***************************************************************************
Summary:
    ISR version of BXVD_SetChannelChangeMode

See Also:
    BXVD_SetSetChannelChangeMode
****************************************************************************/
BERR_Code BXVD_SetChannelChangeMode_isr
(
   BXVD_ChannelHandle hXvdCh,           /* [in] XVD channel handle */
   BXVD_ChannelChangeMode eChChangeMode /* [in] desired channel change mode */
   );

/***************************************************************************
Summary:
  This API retrieves the current channel change mode.

Description:
  Returns the current channel change mode in the peChChangeMode argument.

Returns:
  BERR_SUCCESS
  BERR_INVALID_PARAMETER

See Also:
  BXVD_SetChannelChangeMode

****************************************************************************/
BERR_Code BXVD_GetChannelChangeMode
(
   BXVD_ChannelHandle hXvdCh,             /* [in] XVD channel handle */
   BXVD_ChannelChangeMode *peChChangeMode /* [out] current channel change mode */
   );

/***************************************************************************
Summary:
    ISR version of BXVD_GetChannelChangeMode

See Also:
    BXVD_GetChannelChangeMode
****************************************************************************/
BERR_Code BXVD_GetChannelChangeMode_isr
(
   BXVD_ChannelHandle hXvdCh,             /* [in] XVD channel handle */
   BXVD_ChannelChangeMode *peChChangeMode /* [out] current channel change mode */
   );

/***************************************************************************
Summary:
    This API sets the SPIM for the video decoder.

Description:
    This function overrides the internally computed interpolation mode
    which decides how the scanout should be in when a single picture is
    repeatedly displayed i.e. when
    the user see still picture in the display. This will be required during
    Video Pause, Video Freeze or when MTPMV has not got a new pictures to
    display but repeat what was displayed last. Depending on this intepolation
    mode setting the display manager keeps on displaying field or frame.
    In field it can be either last displayed field, or both fields.

    Note: This API needs to be used only in a experimental mode. It is
    recommended to use the XVD's internal correct display algo to make
    decisions for the SPIM and MPIM.
Returns:
    BERR_SUCCESS

See Also:

****************************************************************************/
BERR_Code BXVD_SetInterpolationModeForStillContent
(
   BXVD_ChannelHandle hXvdCh,          /* [in] The XVD Channel handle */
   BXVD_StillContentInterpolationMode  eNewInterpolation  /* [in] The still
                                                             content interpolation
                                                             mode */
   );

/***************************************************************************
Summary:
    ISR version of BXVD_SetInterpolationModeForStillContent

See Also:
    BXVD_SetInterpolationModeForStillContent
****************************************************************************/
BERR_Code BXVD_SetInterpolationModeForStillContent_isr
(
   BXVD_ChannelHandle hXvdCh,          /* [in] The XVD Channel handle */
   BXVD_StillContentInterpolationMode  eNewInterpolation  /* [in] The still
                                                             content interpolation
                                                             mode */
   );

/***************************************************************************
Summary:
    This function gets the still content interpolation mode(SPIM).

Description:
    The possible SPIMs are enumerated in BXVD_StillContentInterpolationMode.
    CDB underflows, trick mode conditions like pause are also considered as
    still content.

Returns:
    BERR_SUCCESS

See Also:

****************************************************************************/
BERR_Code  BXVD_GetInterpolationModeForStillContent
(
   BXVD_ChannelHandle hXvdChn,  /* [in] The XVD Channel handle */
   BXVD_StillContentInterpolationMode *peStillContentIntrpMode /* [out]
                                                                  The still content interpolation mode */
   );

/***************************************************************************
Summary:
    ISR version of BXVD_GetInterpolationModeForStillContent

See Also:
    BXVD_GetInterpolationModeForStillContent
****************************************************************************/
BERR_Code  BXVD_GetInterpolationModeForStillContent_isr
(
   BXVD_ChannelHandle hXvdChn,  /* [in] The XVD Channel handle */
   BXVD_StillContentInterpolationMode *peStillContentIntrpMode /* [out]
                                                                  The still content interpolation mode */
   );

/***************************************************************************
Summary:
    This API sets the MPIM for the video decoder.

Description:
    This function overrides the internally computed interpolation mode
    which decides how the scanout
    should be in normal running video. Depending on intepolation mode
    display manager displays interlaced or progressive video.

    Note: This API needs to be used only in a experimental mode. It is
    recommended to use the XVD's internal correct display algo to make
    decisions for the SPIM and MPIM.

Returns:
    BERR_SUCCESS

See Also:


****************************************************************************/
BERR_Code BXVD_SetInterpolationModeForMovingContent
(
   BXVD_ChannelHandle hXvdChn,     /* [in] The XVD Channel handle */
   BXVD_MovingContentInterpolationMode    eNewInterpolation  /* [in] The interpolation
                                                                mode for motion content.*/
   );

/***************************************************************************
Summary:
   ISR version of BXVD_SetInterpolationModeForMovingContent

See Also:
   BXVD_SetInterpolationModeForMovingContent
****************************************************************************/
BERR_Code BXVD_SetInterpolationModeForMovingContent_isr
(
   BXVD_ChannelHandle hXvdChn,     /* [in] The XVD Channel handle */
   BXVD_MovingContentInterpolationMode    eNewInterpolation  /* [in] The interpolation
                                                                mode for motion content.*/
   );

/***************************************************************************
Summary:
    This function gets interpolation mode for moving content.
Description:
    The possible MPIMs are enumerated in BXVD_MovingContentInterpolationMode.
Returns:
    BERR_SUCCESS

See Also:


****************************************************************************/
BERR_Code BXVD_GetInterpolationModeForMovingContent
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_MovingContentInterpolationMode    *peNewInterpolation /* [out] Moving
                                                                 content interpolation Mode */
   );

/***************************************************************************
Summary:
   ISR version of BXVD_GetInterpolationModeForMovingContent

See Also:
   BXVD_GetInterpolationModeForMovingContent
****************************************************************************/
BERR_Code BXVD_GetInterpolationModeForMovingContent_isr
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_MovingContentInterpolationMode    *peNewInterpolation /* [out] Moving
                                                                 content interpolation Mode */
   );

/***************************************************************************
Summary:
    Used to set the monitor refresh rate

Description:
    This API is used to set the value of the monitor refresh rate. The refresh
  rate is specified as ONE of the following values:

    BFMT_VERT_50Hz
    BFMT_VERT_59_94Hz
    BFMT_VERT_60Hz
    BFMT_VERT_70Hz
    BFMT_VERT_72Hz
    BFMT_VERT_75Hz
    BFMT_VERT_85Hz
    BFMT_VERT_23_976Hz
    BFMT_VERT_24Hz
    BFMT_VERT_25Hz
    BFMT_VERT_30Hz

These values are specified one at a time to BXVD_SetMonitorRefreshRate.
Although each of these values is capable of being OR'ed to form a bit mask,
doing so will produce unspecified results.

Returns:
    BERR_SUCCESS

See Also:
  BXVD_GetMonitorRefreshRate
****************************************************************************/
BERR_Code BXVD_SetMonitorRefreshRate
(
   BXVD_ChannelHandle hXvdCh,      /* [in] XVD channel handle */
   uint32_t ui32MonitorRefreshRate /* [in] monitor refresh rate value */
   );

/***************************************************************************
Summary:
   ISR version of BXVD_SetMonitorRefreshRate

See Also:
   BXVD_SetMonitorRefreshRate
****************************************************************************/
BERR_Code BXVD_SetMonitorRefreshRate_isr
(
   BXVD_ChannelHandle hXvdCh,      /* [in] XVD channel handle */
   uint32_t ui32MonitorRefreshRate /* [in] monitor refresh rate value */
   );

#if !B_REFSW_MINIMAL /* SWSTB-461 */
/***************************************************************************
Summary:
    Used to get the current value of the monitor refresh rate

Description:
    This API is used to get the current value of the monitor refresh rate.
  See the description of BXVD_SetMonitorRefreshRate for a summary of the
  values that will be returned from this API.

Returns:
    BERR_SUCCESS

See Also:
  BXVD_SetMonitorRefreshRate
****************************************************************************/
BERR_Code BXVD_GetMonitorRefreshRate
(
   BXVD_ChannelHandle hXvdCh,        /* [in] XVD channel handle */
   uint32_t *pui32MonitorRefreshRate /* [out] monitor refresh rate value */
   );

/***************************************************************************
Summary:
   ISR version of BXVD_GetMonitorRefreshRate

See Also:
   BXVD_GetMonitorRefreshRate
****************************************************************************/
BERR_Code BXVD_GetMonitorRefreshRate_isr
(
   BXVD_ChannelHandle hXvdCh,        /* [in] XVD channel handle */
   uint32_t *pui32MonitorRefreshRate /* [out] monitor refresh rate value */
   );
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
Summary:
  Used to freeze video while the decoder continues to run

Description:
  This function is used during live decode to freeze video. The decoder will
  continue to run in the background.

Returns:
    BERR_SUCCESS

See Also:
  BXVD_DisableVideoFreeze, BXVD_GetVideoFreezeState
****************************************************************************/
BERR_Code BXVD_EnableVideoFreeze
(
   BXVD_ChannelHandle hXvdCh /* [in] XVD channel handle */
   );

/***************************************************************************
Summary:
   ISR version of BXVD_EnableVideoFreeze

See Also:
   BXVD_EnableVideoFreeze
****************************************************************************/
BERR_Code BXVD_EnableVideoFreeze_isr
(
   BXVD_ChannelHandle hXvdCh /* [in] XVD channel handle */
   );

/***************************************************************************
Summary:
  Used to re-enable previously frozen video

Description:
  This function resumes the display of video previously stopped by a call to
  BXVD_EnableVideoFreeze.

Returns:
    BERR_SUCCESS

See Also:
  BXVD_DisableVideoFreeze, BXVD_GetVideoFreezeState
****************************************************************************/
BERR_Code BXVD_DisableVideoFreeze
(
   BXVD_ChannelHandle hXvdCh /* [in] XVD channel handle */
   );

/***************************************************************************
Summary:
   ISR version of BXVD_DisableVideoFreeze

See Also:
   BXVD_DisableVideoFreeze
****************************************************************************/
BERR_Code BXVD_DisableVideoFreeze_isr
(
   BXVD_ChannelHandle hXvdCh /* [in] XVD channel handle */
   );

#if !B_REFSW_MINIMAL /* SWSTB-461 */
/***************************************************************************
Summary:
  Used to get the video freeze state

Description:
  This function returns the current state of video freeze. True is returned
  the supplied return parameter if freeze is enabled, false otherwise.

Returns:
    BERR_SUCCESS

See Also:
  BXVD_EnableVideoFreeze, BXVD_DisableVideoFreeze
****************************************************************************/
BERR_Code BXVD_GetVideoFreezeState
(
   BXVD_ChannelHandle hXvdCh, /* [in] XVD channel handle */
   bool             *bVFState /* [out] current video freeze state */
   );

/***************************************************************************
Summary:
   ISR version of BXVD_GetVideoFreezeState

See Also:
   BXVD_GetVideoFreezeState
****************************************************************************/
BERR_Code BXVD_GetVideoFreezeState_isr
(
   BXVD_ChannelHandle hXvdCh, /* [in] XVD channel handle */
   bool             *bVFState /* [out] current video freeze state */
   );
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
Summary:
  Used to set the 1080p scan mode

See Also:
  BXVD_Get1080pScanMode
****************************************************************************/
BERR_Code BXVD_Set1080pScanMode
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_1080pScanMode e1080pScanMode /* [in] The new 1080p scan mode */
   );

/***************************************************************************
Summary:
   ISR version of BXVD_Set1080pScanMode

See Also:
   BXVD_Set1080pScanMode
****************************************************************************/
BERR_Code BXVD_Set1080pScanMode_isr
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_1080pScanMode e1080pScanMode /* [in] The new 1080p scan mode */
   );

#if !B_REFSW_MINIMAL /* SWSTB-461 */
/***************************************************************************
Summary:
  Used to get the 1080p scan mode

See Also:
  BXVD_Set1080pScanMode
****************************************************************************/
BERR_Code BXVD_Get1080pScanMode
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_1080pScanMode *pe1080pScanMode /* [out] The current 1080p scan mode */
   );

/***************************************************************************
Summary:
   ISR version of BXVD_Get1080pScanMode

See Also:
   BXVD_Set1080pScanMode
****************************************************************************/
BERR_Code BXVD_Get1080pScanMode_isr
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_1080pScanMode *pe1080pScanMode /* [out] The current 1080p scan mode */
   );
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
Summary:
  Used to set the picture drop mode for the channel

See Also:
  BXVD_SetPictureDropMode
  BXVD_GetPictureDropMode_isr
****************************************************************************/
BERR_Code BXVD_SetPictureDropMode
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_PictureDropMode ePictureDropMode /* [in] the picture drop mode */
   );

/***************************************************************************
Summary:
  ISR version of BXVD_SetPictureDropMode

See Also:
  BXVD_SetPictureDropMode
****************************************************************************/
BERR_Code BXVD_SetPictureDropMode_isr
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_PictureDropMode ePictureDropMode /* [in] the picture drop mode */
   );

#if !B_REFSW_MINIMAL /* SWSTB-461 */
/***************************************************************************
Summary:
  Used to get the current picture drop mode for the channel

See Also:
  BXVD_SetPictureDropMode
  BXVD_GetPictureDropMode_isr
****************************************************************************/
BERR_Code BXVD_GetPictureDropMode
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_PictureDropMode *pePictureDropMode /* [out] the current picture drop mode */
   );

/***************************************************************************
Summary:
  ISR version of BXVD_GetPictureDropMode

See Also:
  BXVD_GetPictureDropMode
****************************************************************************/
BERR_Code BXVD_GetPictureDropMode_isr
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_PictureDropMode *pePictureDropMode /* [out] the current picture drop mode */
   );

/***************************************************************************
Summary:
  Used to set the 240i scan mode

See Also:
  BXVD_Get240iScanMode
****************************************************************************/
BERR_Code BXVD_Set240iScanMode
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_240iScanMode e240iScanMode /* [in] The new 240i scan mode */
   );

/***************************************************************************
Summary:
   ISR version of BXVD_Set240iScanMode

See Also:
   BXVD_Set240iScanMode
****************************************************************************/
BERR_Code BXVD_Set240iScanMode_isr
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_240iScanMode e240iScanMode /* [in] The new 240i scan mode */
   );

/***************************************************************************
Summary:
  Used to get the 240i scan mode

See Also:
  BXVD_Set240iScanMode
****************************************************************************/
BERR_Code BXVD_Get240iScanMode
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_240iScanMode *pe240iScanMode /* [out] The current 240i scan mode */
   );

/***************************************************************************
Summary:
   ISR version of BXVD_Get240iScanMode

See Also:
   BXVD_Set240iScanMode
****************************************************************************/
BERR_Code BXVD_Get240iScanMode_isr
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   BXVD_240iScanMode *pe240iScanMode /* [out] The current 240i scan mode */
   );
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/******************************/
/* Display Manager APIs - TSM */
/******************************/

/***************************************************************************
Summary:
    API used to set the display offset.
Description:
    The display offset supplied is applied for the specified channel.
    The Presentation Offset is a value used to compensate for any additional
    delay that arises from the data flow through the Compressed data buffer
    and Video Decoder and the Video back end display.

 If decoding standard MPEG the display offset value is 1/45KHz.
 If decoding Direct-TV the display offset value is 1/27MHz.

Returns:
    BERR_SUCCESS

See Also:
    BXVD_GetDisplayOffset
    BXVD_SetDisplayOffset_isr

****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_SetDisplayOffset
(
   BXVD_ChannelHandle    hXvdCh,               /* [in] XVD Channel handle */
   long                  lDisplayOffsetValue   /* [in] display offset */
   );
#endif

/***************************************************************************
Summary:
    ISR verion of API used to set the display offset.
Description:
    See BXVD_SetDisplayOffset
Returns:
    BERR_SUCCESS

See Also:
    BXVD_GetDisplayOffset
    BXVD_SetDisplayOffset

****************************************************************************/
BERR_Code BXVD_SetDisplayOffset_isr
(
   BXVD_ChannelHandle    hXvdCh,               /* [in] XVD Channel handle */
   long                  lDisplayOffsetValue   /* [in] display offset */
   );

/***************************************************************************
Summary:
    API used to get the display offset.
Description:
    Use to retrieve display offset value
Returns:
    BERR_SUCCESS

See Also:
    BXVD_SetDisplayOffset
    BXVD_SetDisplayOffset_isr

****************************************************************************/
BERR_Code BXVD_GetDisplayOffset
(
   BXVD_ChannelHandle    hXvdCh,               /* [in] XVD Channel handle */
   long                 *plDisplayOffsetValue  /* [out] display offset */
   );

/***************************************************************************
Summary:
    API used to get the display offset in isr context.
Description:
    See BXVD_GetDisplayOffset
Returns:
    BERR_SUCCESS

See Also:
    BXVD_SetDisplayOffset
    BXVD_SetDisplayOffset_isr

****************************************************************************/
BERR_Code BXVD_GetDisplayOffset_isr
(
   BXVD_ChannelHandle    hXvdCh,               /* [in] XVD Channel handle */
   long                 *plDisplayOffsetValue  /* [out] display offset */
   );

/***************************************************************************
Summary:
    Used to set the display mode.
Description:
    This API is used to set the display mode enumerated by the
    BXVD_DisplayMode. The display can be configure to follow the STC ie in
    Time Stamp managed mode or in non Time Stamp mode ie in VSync mode.

Returns:
    BERR_SUCCESS

See Also:
****************************************************************************/
BERR_Code BXVD_SetVideoDisplayMode
(
   BXVD_ChannelHandle      hXvdCh,        /* [in] XVD Channel handle */
   BXVD_DisplayMode        eDisplayMode  /* [in] display mode requested */
   );

/***************************************************************************
Summary:
   ISR version of BXVD_SetVideoDisplayMode

See Also:
   BXVD_SetVideoDisplayMode
***************************************************************************/
BERR_Code BXVD_SetVideoDisplayMode_isr
(
   BXVD_ChannelHandle      hXvdCh,        /* [in] XVD Channel handle */
   BXVD_DisplayMode        eDisplayMode  /* [in] display mode requested */
   );

#if !B_REFSW_MINIMAL /* SWSTB-461 */
/***************************************************************************
Summary:
    Used to get the display mode.
Description:
    This function is used to retrieve the video display mode
Returns:
    BERR_SUCCESS

See Also:
****************************************************************************/
BERR_Code BXVD_GetVideoDisplayMode
(
   BXVD_ChannelHandle     hXvdCh,          /* [in] XVD Channel handle */
   BXVD_DisplayMode      *peDisplayMode   /* [out] display mode */
   );

/***************************************************************************
Summary:
   ISR version of BXVD_GetVideoDisplayMode

See Also:
   BXVD_GetVideoDisplayMode
***************************************************************************/
BERR_Code BXVD_GetVideoDisplayMode_isr
(
   BXVD_ChannelHandle     hXvdCh,          /* [in] XVD Channel handle */
   BXVD_DisplayMode      *peDisplayMode   /* [out] display mode */
   );
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
Summary:
    API used to get display threshold values
Description:
    This API is used to get current display thresholds. This was implemented
  by separate APIs for high and low thresholds and discard thresholds in XVD.
Returns:
    BERR_SUCCESS

See Also:

****************************************************************************/
BERR_Code BXVD_GetDisplayThresholds
(
   BXVD_ChannelHandle     hXvdCh,          /* [in] The XVD Channel handle */
   BXVD_DisplayThresholds *pDispThresholds /* [out] returned threshold info */
   );

/***************************************************************************
Summary:
   ISR version of BXVD_GetDisplayThresholds

See Also:
   BXVD_GetDisplayThresholds
***************************************************************************/
BERR_Code BXVD_GetDisplayThresholds_isr
(
   BXVD_ChannelHandle     hXvdCh,          /* [in] The XVD Channel handle */
   BXVD_DisplayThresholds *pDispThresholds /* [out] returned threshold info */
   );

/***************************************************************************
Summary:
    API used to supply the decoder with display discard threshold.

Description:
    The discard threshold can be specified through this API. If PTS-STC
    difference (STC lagging behind PTS) is more than display discard
    threshold, video decoder doesn't wait for STC to mature to PTS value
    for that field/frame and drops it.

 Note that the value passed as a threshold to this API will be overwritten
 by the default value determined in BXVD_StartDecode. Therefore, this API
 must be called after BXVD_StartDecode in order to use a user-supplied
 discard threshold.

Returns:
    BERR_SUCCESS

See Also:
    BXVD_SetDisplayOffset
        BXVD_SetVeryLateThreshold

****************************************************************************/
BERR_Code BXVD_SetDiscardThreshold
(
   BXVD_ChannelHandle hXvdChn,             /* [in] The XVD Channel handle */
   uint32_t           ui32DiscardThreshold /* [in] The discard thresshold */
   );

/***************************************************************************
Summary:
   ISR version of BXVD_SetDiscardThreshold

See Also:
   BXVD_SetDiscardThreshold
***************************************************************************/
BERR_Code BXVD_SetDiscardThreshold_isr
(
   BXVD_ChannelHandle hXvdChn,             /* [in] The XVD Channel handle */
   uint32_t           ui32DiscardThreshold /* [in] The discard thresshold */
   );

/***************************************************************************
Summary:
    API used to supply the decoder with display very late threshold.

Description:
    The very late threshold can be specified through this API. If PTS-STC
    difference (STC lagging behind PTS) is more than display very late
    threshold, the STC is set invalid and DM switches to non-TSM mode.

 Note that the value passed as a threshold to this API will be overwritten
 by the default value determined in BXVD_StartDecode. Therefore, this API
 must be called after BXVD_StartDecode in order to use a user-supplied
 very late threshold.

Returns:
    BERR_SUCCESS

See Also:
        BXVD_SetDisplayOffset
        BXVD_SetDiscardThreshold

****************************************************************************/
BERR_Code BXVD_SetVeryLateThreshold
(
   BXVD_ChannelHandle hXvdChn,              /* [in] The XVD Channel handle */
   uint32_t           ui32VeryLateThreshold /* [in] The very late thresshold */
   );

/***************************************************************************
Summary:
   ISR version of BXVD_SetVeryLateThreshold

See Also:
   BXVD_SetVeryLateThreshold
***************************************************************************/
BERR_Code BXVD_SetVeryLateThreshold_isr
(
   BXVD_ChannelHandle hXvdChn,              /* [in] The XVD Channel handle */
   uint32_t           ui32VeryLateThreshold /* [in] The very late thresshold */
   );

/***************************************************************************
Summary:
    Used to set the state of the STC invalid flag
Description:
    This API is used to set the state of the STC invalid flag to true or false.
Returns:
    BERR_SUCCESS

See Also:
****************************************************************************/
BERR_Code BXVD_SetSTCInvalidFlag
(
   BXVD_ChannelHandle hXvdCh, /* [in] XVD channel handle */
   bool bStcInvalidFlag       /* [in] STC invalid flag state */
   );

/***************************************************************************
Summary:
   ISR version of BXVD_SetSTCInvalidFlag

See Also:
   BXVD_SetSTCInvalidFlag
***************************************************************************/
BERR_Code BXVD_SetSTCInvalidFlag_isr
(
   BXVD_ChannelHandle hXvdCh, /* [in] XVD channel handle */
   bool bStcInvalidFlag       /* [in] STC invalid flag state */
   );

#if !B_REFSW_MINIMAL /* SWSTB-461 */
/***************************************************************************
Summary:
    Used to get the current state of the STC invalid flag
Description:
    This API is used to get the current state of the STC invalid flag.
Returns:
    BERR_SUCCESS

See Also:
****************************************************************************/
BERR_Code BXVD_GetSTCInvalidFlag
(
   BXVD_ChannelHandle hXvdCh,           /* [in] XVD channel handle */
   bool               *pbStcInvalidFlag /* [out] STC invalid flag value */
   );

/***************************************************************************
Summary:
   ISR version of BXVD_GetSTCInvalidFlag

See Also:
   BXVD_GetSTCInvalidFlag
***************************************************************************/
BERR_Code BXVD_GetSTCInvalidFlag_isr
(
   BXVD_ChannelHandle hXvdCh,           /* [in] XVD channel handle */
   bool               *pbStcInvalidFlag /* [out] STC invalid flag value */
   );
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
Summary:
  Used to set the Pts/Stc difference threshold

Description:
  This function is used to set the Pts/Stc difference threshold.

Returns:
    BERR_SUCCESS

See Also:
  BXVD_GetPtsStcDiffThreshold
****************************************************************************/
BERR_Code BXVD_SetPtsStcDiffThreshold
(
   BXVD_ChannelHandle hXvdCh, /* [in] XVD channel handle */
   long uiPtsStcDiffThreshold /* [in] Stc difference threshold */
   );

/***************************************************************************
Summary:
   ISR version of BXVD_SetPtsStcDiffThreshold

See Also:
   BXVD_SetPtsStcDiffThreshold
***************************************************************************/
BERR_Code BXVD_SetPtsStcDiffThreshold_isr
(
   BXVD_ChannelHandle hXvdCh, /* [in] XVD channel handle */
   long uiPtsStcDiffThreshold /* [in] Stc difference threshold */
   );

#if !B_REFSW_MINIMAL /* SWSTB-461 */
/***************************************************************************
Summary:
  Used to get the Pts/Stc difference threshold

Description:
  This function returns the current value of the Pts/Stc difference threshold.

Returns:
    BERR_SUCCESS

See Also:
  BXVD_SetPtsStcDiffThreshold
****************************************************************************/
BERR_Code BXVD_GetPtsStcDiffThreshold
(
   BXVD_ChannelHandle hXvdCh,   /* [in] XVD channel handle */
   long *puiPtsStcDiffThreshold /* [out] Stc difference threshold */
   );

/***************************************************************************
Summary:
   ISR version of BXVD_GetPtsStcDiffThreshold

See Also:
   BXVD_GetPtsStcDiffThreshold
***************************************************************************/
BERR_Code BXVD_GetPtsStcDiffThreshold_isr
(
   BXVD_ChannelHandle hXvdCh,   /* [in] XVD channel handle */
   long *puiPtsStcDiffThreshold /* [out] Stc difference threshold */
   );
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
Summary:
  Used to set the source STC time base. (Non-ISR version)

Description:
  TSM calculations will be based on the STC counter value of the
  specified time base.  This permanently overrides the time base
  specified in BXVD_StartDecode.

Returns:
  BERR_SUCCESS
  BERR_INVALID_PARAMETER

See Also:
  BXVD_StartDecode
  BXVD_SetSTCSource_isr
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_SetSTCSource
(
   BXVD_ChannelHandle hXvdCh,  /* [in] The XVD Channel handle */
   BXVD_STC eSTC               /* [in] STC Time base */
   );
#endif

/***************************************************************************
Summary:
  ISR version of BXVD_SetSTCSource

See Also:
  BXVD_SetSTCSource
****************************************************************************/
BERR_Code BXVD_SetSTCSource_isr
(
   BXVD_ChannelHandle hXvdCh,  /* [in] The XVD Channel handle */
   BXVD_STC eSTC               /* [in] STC Time base */
   );

#if !B_REFSW_MINIMAL /* SWSTB-461 */
/***************************************************************************
Summary:
  Used to get the source STC time base. (Non-ISR version)

Description:
  Returns the source STC for the specified channel

Returns:
  BERR_SUCCESS
  BERR_INVALID_PARAMETER

See Also:
  BXVD_GetSTCSource_isr
  BXVD_SetSTCSource
  BXVD_SetSTCSource_isr
****************************************************************************/
BERR_Code BXVD_GetSTCSource
(
   BXVD_ChannelHandle hXvdCh,  /* [in] The XVD Channel handle */
   BXVD_STC *peSTC             /* [out] STC Time base */
   );

/***************************************************************************
Summary:
  ISR version of BXVD_GetSTCSource

See Also:
  BXVD_GetSTCSource
****************************************************************************/
BERR_Code BXVD_GetSTCSource_isr
(
   BXVD_ChannelHandle hXvdCh,  /* [in] The XVD Channel handle */
   BXVD_STC *peSTC             /* [out] STC Time base */
   );


/***************************************************************************
Summary:
  Used to retrieve the captured STC for the specified time
  base. (Non-ISR version)

Description:
 Returns the STC value.

Returns:
  BERR_SUCCESS
  BERR_INVALID_PARAMETER

See Also:
  BXVD_GetSTCValue_isr
****************************************************************************/
BERR_Code BXVD_GetSTCValue
(
   BXVD_Handle hXvd,                         /* [in] The XVD Channel handle */
   BXVD_DisplayInterrupt eDisplayInterrupt,  /* [in] Display Interrupt number */
   BXVD_STC eSTC,                            /* [in] STC Time base */
   BXVD_STCInfo *pSTCInfo                    /* [out] STC info */
   );


/***************************************************************************
Summary:
  ISR version of BXVD_GetSTCValue

See Also:
  BXVD_GetSTCValue
****************************************************************************/
BERR_Code BXVD_GetSTCValue_isr
(
   BXVD_Handle hXvd,                         /* [in] The XVD Channel handle */
   BXVD_DisplayInterrupt eDisplayInterrupt,  /* [in] Display Interrupt number */
   BXVD_STC eSTC,                            /* [in] STC Time base */
   BXVD_STCInfo *pSTCInfo                    /* [out] STC info */
   );
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
Summary:
  Starts display when the specified time has been reached.  Stops
  display when the specified time has been reached.

Description:
  Keeps the decode in background and shows the last valid picture on
  display until the specified start time is reached then resumes
  decode and display of the stream.  The stream is decoded and
  displayed until the specified stop time is reached.  Then the
  display repeats the last valid picture.  The next stop time is
  automatically used after a picture marker event has been processed.
  To ensure a seamless connection between clips, the next start time
  is automatically used after a picture marker event has been
  processed.


Returns:
  BERR_SUCCESS

See Also:
  BXVD_Interrupt_eClipStart
  BXVD_Interrupt_eClipStop

****************************************************************************/
BERR_Code BXVD_SetClipTime
(
   BXVD_ChannelHandle hXvdCh,  /* [in] The XVD Channel handle */
   BXVD_ClipTimeType eClipTimeType, /* [in] Clip Time Type */
   uint32_t      ui32StartTime, /* [in] Start display from the specified time/PTS */
   uint32_t       ui32StopTime  /* [in] Stop display at the specified time/PTS */
   );


/***************************************************************************
Summary:
  ISR version of BXVD_SetClipTime

Description:
 See BXVD_SetClipTime

Returns:
  BERR_SUCCESS

See Also:
  BXVD_SetClipTime

****************************************************************************/
BERR_Code BXVD_SetClipTime_isr
(
   BXVD_ChannelHandle hXvdCh,  /* [in] The XVD Channel handle */
   BXVD_ClipTimeType eClipTimeType, /* [in] Clip Time Type */
   uint32_t    ui32StartTime, /* [in] Start display from the specified time/PTS */
   uint32_t    ui32StopTime /* [in] Stop display at the specified time/PTS */
   );


/***************************************************************************
Summary:
  Used to pause DM TSM processing until the STC is set.  This call
  will:
   - invalidate STC
   - pause DM picture processing (all pictures will be held)
   - enable the RequestSTC callback

  The application must have the BXVD_Interrupt_eRequestSTC callback
  installed.  After the application receives the RequestSTC callback,
  it must validate the STC before DM processing will continue.

See Also:
  BXVD_SetTSMWaitForRequestSTC
  BXVD_Interrupt_eRequestSTC
  BXVD_SetSTCInvalidFlag_isr
****************************************************************************/
BERR_Code BXVD_SetTSMWaitForValidSTC
(
   BXVD_ChannelHandle hXvdCh /* [in] The XVD Channel handle */
   );

/***************************************************************************
Summary:
  ISR version of BXVD_SetTSMWaitForRequestSTC

See Also:
  BXVD_SetTSMWaitForRequestSTC
****************************************************************************/
BERR_Code BXVD_SetTSMWaitForValidSTC_isr
(
   BXVD_ChannelHandle hXvdCh /* [in] The XVD Channel handle */
   );

/***************************************************************************
Summary:
  Used to set the software PCR offset, s programmable offset added to
  the STC prior to TSM evaluation.

See Also:
  BXVD_GetSwPcrOffset
****************************************************************************/
BERR_Code BXVD_SetSwPcrOffset
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   uint32_t uiSwPcrOffset     /* [in] software PCR offset */
   );

/***************************************************************************
Summary:
  ISR version of BXVD_SetSwPcrOffset.

See Also:
  BXVD_GetSwPcrOffset
****************************************************************************/
BERR_Code BXVD_SetSwPcrOffset_isr
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   uint32_t uiSwPcrOffset     /* [in] software PCR offset */
   );

/***************************************************************************
Summary:
  Used to get the software PCR offset, s programmable offset added to
  the STC prior to TSM evaluation.

See Also:
  BXVD_SetSwPcrOffset
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetSwPcrOffset
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   uint32_t * puiSwPcrOffset  /* [out] software PCR offset */
   );
#endif

/***************************************************************************
Summary:
  ISR version of BXVD_GetSwPcrOffset.

See Also:
  BXVD_SetSwPcrOffset
****************************************************************************/
BERR_Code BXVD_GetSwPcrOffset_isr
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   uint32_t * puiSwPcrOffset  /* [out] software PCR offset */
   );

/***************************************************************************
Summary:
  Used to set a programmable flag which indicates whether or not to
  include the HW PCR offset in the TSM equation.
  Note: this only applies to "live" mode. When "playback" is enabled, the
  PCR offset is set to '0' in the TSM equation; in this case
  "bHwPcrOffsetEnable" is a "don't care".

See Also:
  BXVD_GetHwPcrOffsetEnable
****************************************************************************/
BERR_Code BXVD_SetHwPcrOffsetEnable
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   bool bHwPcrOffsetEnable    /* [in] hardware PCR offset enable flag */
   );

/***************************************************************************
Summary:
  ISR version of BXVD_SetHwPcrOffsetEnable.

See Also:
  BXVD_GetHwPcrOffsetEnable
****************************************************************************/
BERR_Code BXVD_SetHwPcrOffsetEnable_isr
(
   BXVD_ChannelHandle hXvdCh, /* [in] The XVD Channel handle */
   bool bHwPcrOffsetEnable    /* [in] hardware PCR offset enable flag */
   );

#if !B_REFSW_MINIMAL /* SWSTB-461 */
/***************************************************************************
Summary:
  Used to get a programmable flag which indicates whether or not to
  include the HW PCR offset in the TSM equation.

See Also:
  BXVD_SetHwPcrOffsetEnable
****************************************************************************/
BERR_Code BXVD_GetHwPcrOffsetEnable
(
   BXVD_ChannelHandle hXvdCh,    /* [in] The XVD Channel handle */
   bool * pbHwPcrOffsetEnable    /* [out] hardware PCR offset enable flag */
   );

/***************************************************************************
Summary:
  ISR version of BXVD_GetHwPcrOffsetEnable.

See Also:
  BXVD_SetHwPcrOffsetEnable
****************************************************************************/
BERR_Code BXVD_GetHwPcrOffsetEnable_isr
(
   BXVD_ChannelHandle hXvdCh,    /* [in] The XVD Channel handle */
   bool * pbHwPcrOffsetEnable    /* [out] hardware PCR offset enable flag */
   );


/***************************************************************************
Summary:
  SW7400-2870:
  DEPRECATED: use BXVD_SetPlaybackRate(isr) instead

  Used to set the playback rate when in vsync mode.
  "uiVsyncPlaybackRate" is a percentage.

  "uiVsyncPlaybackRate" == 100 // is normal
  "uiVsyncPlaybackRate" < 100  // is slow motion
  "uiVsyncPlaybackRate" > 100  // is fast motion

See Also:
  BXVD_GetVsyncPlaybackRate
****************************************************************************/

BERR_Code BXVD_SetVsyncPlaybackRate(
   BXVD_ChannelHandle hXvdCh,
   uint32_t uiVsyncPlaybackRate
   );

BERR_Code BXVD_SetVsyncPlaybackRate_isr(
   BXVD_ChannelHandle hXvdCh,
   uint32_t uiVsyncPlaybackRate
   );


/***************************************************************************
Summary:
  SW7400-2870:
  DEPRECATED: use BXVD_GetPlaybackRate(isr) instead

  Used to get the playback rate when in vsync mode.

See Also:
  BXVD_SetVsyncPlaybackRate
****************************************************************************/

BERR_Code BXVD_GetVsyncPlaybackRate(
   BXVD_ChannelHandle hXvdCh,
   uint32_t * puiVsyncPlaybackRate
   );

BERR_Code BXVD_GetVsyncPlaybackRate_isr(
   BXVD_ChannelHandle hXvdCh,
   uint32_t * puiVsyncPlaybackRate
   );
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

/***************************************************************************
Summary:
  SW7400-2870:
  Sets the playback rate for vsync mode. The rate is expressed as the ratio
  of stPlaybackRateSettings.uiNumerator/stPlaybackRateSettings.uiDenominator.

  Note: setting the rate to a ratio not equal to '1' will result in pictures
  being evaluated in vsync mode even if the system is still in TSM mode.

  This ratio is applied as a multiplier to the vsync STC delta. Take for
  example a playback rate of 60 Hz, the vsync STC delta would be 750.
  A ratio of 1/2 will result in a delta of 375 and playback at half speed.

  If stPlaybackRateSettings.uiNumerator is set to '0', the system will pause.

  If stPlaybackRateSettings.uiDenominator is set to '0', the XDM state will
  not be modified and BERR_INVALID_PARAMETER will be returned.

  This playback rate is intended to be used for slow motion.  It could be
  used for slight fast forward, however there is the risk of underflowing
  buffers if the ratio is too large.

  In addition, the playback rate is only intended to be used when playing
  back from disk.  If ratio is something other than '1' during live
  playback, there is the risk of either buffer under or overflows.

  "uiNumerator" and " uiDenominator" are uint16_t's to prevent arithmetic
  overflows.

  This API supplants BXVD_SetVsyncPlaybackRate(_isr) and
  BXVD_PVR_SetSlowMotionRate(_isr).

See Also:
  BXVD_GetVsyncPlaybackRate
****************************************************************************/

BERR_Code BXVD_SetPlaybackRate(
   BXVD_ChannelHandle hXvdCh,
   BXVD_PlaybackRateSettings stPlaybackRateSettings
   );

BERR_Code BXVD_SetPlaybackRate_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_PlaybackRateSettings stPlaybackRateSettings
   );


/***************************************************************************
Summary:
  Used to get the playback rate when in vsync mode.

  This API supplants BXVD_GetVsyncPlaybackRate(_isr) and
  BXVD_PVR_GetSlowMotionRate(_isr).

See Also:
  BXVD_SetPlaybackRate
****************************************************************************/

BERR_Code BXVD_GetPlaybackRate(
   BXVD_ChannelHandle hXvdCh,
   BXVD_PlaybackRateSettings * pstPlaybackRateSettings
   );

BERR_Code BXVD_GetPlaybackRate_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_PlaybackRateSettings * pstPlaybackRateSettings
   );


/***************************************************************************
Summary:
  Used to get the PPB Parameter info for the currently displayed
  picture. Returns an error if a picture is not being displayed

See Also:
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetPPBParameterInfo(
   BXVD_ChannelHandle hXvdCh, /* [in] XVD Channel handle */
   BXVD_PPBParameterInfo *pPPBParameterInfo /* [out] PPB Parameter Info is returned */
   );

BERR_Code BXVD_GetPPBParameterInfo_isr(
   BXVD_ChannelHandle hXvdCh, /* [in] XVD Channel handle */
   BXVD_PPBParameterInfo *pPPBParameterInfo /* [out] PPB Parameter Info is returned */
   );
#endif

/***************************************************************************
Summary:
  Used to get the PPB Parameter info for the pictures that are on the
  delivery queue.

See Also:
****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetPPBParameterQueueInfo(
   BXVD_ChannelHandle hXvdCh, /* [in] XVD Channel handle */
   const BXVD_PPBParameterInfo* astPPBParameterInfo[], /* [out] pointers to const PPB Parameter info structs are returned in this array */
   uint32_t uiPPBParameterInfoCount, /* size of PPB Parameter Info array */
   uint32_t *puiValidPPBParameterInfoCount /* number of valid entries returned in the PPB Parameter Info array */
   );
#endif

BERR_Code BXVD_GetPPBParameterQueueInfo_isr(
   BXVD_ChannelHandle hXvdCh, /* [in] XVD Channel handle */
   const BXVD_PPBParameterInfo* astPPBParameterInfo[], /* [out] pointers to const PPB Parameter info structs are returned in this array */
   uint32_t uiPPBParameterInfoCount, /* size of PPB Parameter Info array */
   uint32_t *puiValidPPBParameterInfoCount /* number of valid entries returned in the PPB Parameter Info array */
   );

/***************************************************************************
Summary:
  Used to set the picture error handling mode.

See Also:
  BXVD_Picture_ErrorHandling for the definition of the modes.
****************************************************************************/

BERR_Code BXVD_SetErrorHandlingMode(
   BXVD_ChannelHandle hXvdCh,
   BXVD_Picture_ErrorHandling eErrorMode
   );

BERR_Code BXVD_SetErrorHandlingMode_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_Picture_ErrorHandling eErrorMode
   );

/***************************************************************************
Summary:
  Used to get the picture error handling mode.

See Also:
  BXVD_Picture_ErrorHandling for the definition of the modes.
****************************************************************************/

BERR_Code BXVD_GetErrorHandlingMode(
   BXVD_ChannelHandle hXvdCh,
   BXVD_Picture_ErrorHandling * peErrorMode
   );

BERR_Code BXVD_GetErrorHandlingMode_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_Picture_ErrorHandling * peErrorMode
   );

/***************************************************************************
Summary:
  Used to enable/disable XVD DM's Jitter Tolerance Improvement (JTI) logic.

See Also:
****************************************************************************/

BERR_Code BXVD_SetJitterToleranceImprovementEnable(
   BXVD_ChannelHandle hXvdCh,
   bool bEnable
   );

BERR_Code BXVD_SetJitterToleranceImprovementEnable_isr(
   BXVD_ChannelHandle hXvdCh,
   bool bEnable
   );

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetJitterToleranceImprovementEnable(
   BXVD_ChannelHandle hXvdCh,
   bool *pbEnable
   );

BERR_Code BXVD_GetJitterToleranceImprovementEnable_isr(
   BXVD_ChannelHandle hXvdCh,
   bool *pbEnable
   );
#endif


/***************************************************************************
Summary:
    Set the pulldown override mode for MPEG content.

See Also:
   typedef enum BXVD_MPEGPulldownOverride
****************************************************************************/

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_SetMPEGPulldownOverride(
   BXVD_ChannelHandle hXvdCh,
   BXVD_MPEGPulldownOverride ePulldownOverride
   );

BERR_Code BXVD_SetMPEGPulldownOverride_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_MPEGPulldownOverride ePulldownOverride
   );

BERR_Code BXVD_GetMPEGPulldownOverride(
   BXVD_ChannelHandle hXvdCh,
   BXVD_MPEGPulldownOverride * pePulldownOverride
   );

BERR_Code BXVD_GetMPEGPulldownOverride_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_MPEGPulldownOverride * pePulldownOverride
   );
#endif

/***************************************************************************

Summary:

   SW7405-4117: deinterlacer max height is used in conjuction with
   BXVD_DisplayFieldType_eAuto to choose either eSingleField or eBothField
   based on the steam height during slow motion (and preroll).

   See the usage note above at the defintion of "BXVD_DisplayFieldType".

****************************************************************************/

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_SetDeinterlacerMaxHeight(
   BXVD_ChannelHandle hXvdCh,
   uint32_t uiMaxHeight
   );

BERR_Code BXVD_SetDeinterlacerMaxHeight_isr(
   BXVD_ChannelHandle hXvdCh,
   uint32_t uiMaxHeight
   );

BERR_Code BXVD_GetDeinterlacerMaxHeight(
   BXVD_ChannelHandle hXvdCh,
   uint32_t * puiMaxHeight
   );

BERR_Code BXVD_GetDeinterlacerMaxHeight_isr(
   BXVD_ChannelHandle hXvdCh,
   uint32_t * puiMaxHeight
   );
#endif

/***************************************************************************

Summary:
   SW7405-4703: API to Set Horizontal Overscan calculation mode

Description:
   This API can be used to change the default behavior of the Horizontal Overscan
   calculations within the DM.  Thus the DM can be configured to bypass the
   Horizonal Overscan calculation if desired.
   Default behavior is specified by BXVD_HorizontalOverscanMode_eAuto.

See Also:
   BXVD_GetHorizontalOverscanMode

****************************************************************************/
BERR_Code BXVD_SetHorizontalOverscanMode
(
   BXVD_ChannelHandle     hXvdCh,                  /* [in] XVD Channel handle */
   BXVD_HorizontalOverscanMode  eHorizOverscanMode /* [in] Horizontal Overscan mode  */
);

BERR_Code BXVD_SetHorizontalOverscanMode_isr
(
   BXVD_ChannelHandle     hXvdCh,                  /* [in] XVD Channel handle */
   BXVD_HorizontalOverscanMode  eHorizOverscanMode /* [in] Horizontal Overscan mode  */
);


/***************************************************************************

Summary:
   SW7405-4703: API to Get current Horizontal Overscan mode

Description:
   This API can be used to query the current behavior of the Horizontal Overscan
   calculations within the DM.

See Also:
   BXVD_SetHorizontalOverscanMode

****************************************************************************/
#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_GetHorizontalOverscanMode
(
   BXVD_ChannelHandle     hXvdCh,                  /* [in] XVD Channel handle */
   BXVD_HorizontalOverscanMode  *peHorizOverscanMode /* [out] Horizontal Overscan mode  */
);

BERR_Code BXVD_GetHorizontalOverscanMode_isr
(
   BXVD_ChannelHandle     hXvdCh,                  /* [in] XVD Channel handle */
   BXVD_HorizontalOverscanMode  *peHorizOverscanMode /* [out] Horizontal Overscan mode  */
);
#endif

/***************************************************************************

Summary:
   SWDEPRECATED-1003:
   API to specify the video frame rate as a numerator / denominator.
   This value will override any frame rate coded in the steam was well as
   the "default" specified in the decode settings.

Description:

   The BXVD_FrameRateOverride structure contains two elements; "bValid"
   and "stRate".  If "bValid" is TRUE, the values specified in "stRate"
   will be used as the frame rate.  If "bValid" is FALSE, the system will
   use either the coded or default frame rate.

   "stRate" consists of a numerator and a denominator.  If either element
   is equal to "0", "bValid" will be set to FALSE.

   There are a number of scenarios when this API might be useful.  It could
   be used when a stream has an incorrectly coded frame rate.  It could be
   used to control the playback rate when the system is in vsync mode; by
   setting the frame rate equal to the display rate, pictures will be
   scanned out one element per vsync.

   Internal to XDM, the result of the numerator/denominator is mapped to
   one of the BAVC_FrameRateCode values specified in bavc.h.  Eventually
   XDM will be modified to handle arbitrary frame rates to support all
   the new internet protocols.

See Also:

****************************************************************************/


BERR_Code BXVD_SetFrameRateOverride(
   BXVD_ChannelHandle hXvdCh,
   BXVD_FrameRateOverride * pstFrameRateOverrideSettings
   );

BERR_Code BXVD_SetFrameRateOverride_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_FrameRateOverride * pstFrameRateOverrideSettings
   );

BERR_Code BXVD_GetFrameRateOverride(
   BXVD_ChannelHandle hXvdCh,
   BXVD_FrameRateOverride * pstFrameRateOverrideSettings
   );

BERR_Code BXVD_GetFrameRateOverride_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_FrameRateOverride * pstFrameRateOverrideSettings
   );

/***************************************************************************

Summary:
   SW7422-72: API to set the 3D orientation of pictures.

Description:
   In general the XVD Decoder simply passes content through as is. It has
   no knowledge of what formats are supported by VDC.  The "fix up" of
   unsupported or unspecified formats falls upon XDM in concert with the
   middleware/application.  To support "fix up", the following XDM API's
   have been added.

   BXVD_Set3D(_isr) takes a pointer to a BXVD_3DSetting structure.  This
   structure contains two elements, bOverrideOrientation and eOrientation.
   When bOverrideOrientation is set to true, XDM will use the eOrientation
   element to populate the MFD picture structure.  When
   bOverrideOrientation is set to false, XDM will use in-band information
   (SEI messages) to  populate the MFD picture structure.  The default
   setting of bOverrideOrientation is false.

   The SEI message callback will be generated before filling in the MFD
   picture structure.  The middleware/application can use the contents of
   the SEI message to determine if BXVD _Set3D(_isr) needs to be called to
   force a specific  3D orientation.

   The values set by BXVD_Set3D(_isr) are sticky and will persist until
   BXVD_Set3D(_isr) is called again.  The values will be reset to the default
   when BXVD_StopDecode is called.

   BXVD_Get3D(_isr) returns the current 3D settings.

   BXVD_GetDefault3D returns the default settings.

See Also:

****************************************************************************/

BERR_Code BXVD_Set3D(
   BXVD_ChannelHandle hXvdCh,
   const BXVD_3DSetting * pst3DSettings
   );

BERR_Code BXVD_Set3D_isr(
   BXVD_ChannelHandle hXvdCh,
   const BXVD_3DSetting * pst3DSettings
   );

BERR_Code BXVD_Get3D(
   BXVD_ChannelHandle hXvdCh,
   BXVD_3DSetting * pst3DSettings
   );

BERR_Code BXVD_Get3D_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_3DSetting * pst3DSettings
   );

BERR_Code BXVD_GetDefault3D(
   BXVD_ChannelHandle hXvdCh,
   BXVD_3DSetting * pst3DSettings
   );

/***************************************************************************

Summary:
   SW7425-1264: support for a synthesized SW STC, can create a clock
   internal to XDM that run backwards.

   The clock override is set by calling "BXVD_SetClockOverride".
   "BXVD_GetClockOverride" returns the most recently written values.

   See bxdm_pp.h for the definition of BXDM_PictureProvider_ClockOverride.

   When "bEnableClockOverride" is true, XDM will use an internal SW STC.

   When "bLoadSwStc" is true, the SW STC will be initialized with "uiStcValue".
   This loading occurs at the beginning of XDM's execution.  If "bLoadSwStc"
   is false, the SW STC will not be loaded.  This mechanism allows
   "BXVD_SetClockOverride" to be called without changing the current value
   of the SW STC.

   "iStcDelta" is added to the SW STC every vsync, effectively at the end of
   XDM's execution.  "iStcDelta" can be negative, this will cause the clock to
   run backwards.  The units of "iStcDelta" are clock ticks; for example to
   achieve a 60 Hz display rate, "iStcDelta" would be set to 750.

   The current SW STC can be retrieved by calling BXVD_GetPTS_isr or
   BXDM_PictureProvider_GetCurrentPTSInfo_isr with "bEnableClockOverride" set
   to true.  The SW STC will be returned in "stPTSInfo.uiSTCSnapShot".

****************************************************************************/

BERR_Code BXVD_SetClockOverride(
   BXVD_ChannelHandle hXvdCh,
   const BXVD_ClockOverride * pstClockOverride
   );

BERR_Code BXVD_SetClockOverride_isr(
   BXVD_ChannelHandle hXvdCh,
   const BXVD_ClockOverride * pstClockOverride
   );

BERR_Code BXVD_GetClockOverride(
   BXVD_ChannelHandle hXvdCh,
   BXVD_ClockOverride * pstClockOverride
   );

BERR_Code BXVD_GetClockOverride_isr(
   BXVD_ChannelHandle hXvdCh,
   BXVD_ClockOverride * pstClockOverride
   );

/***************************************************************************
Summary:
   SW7425-1064:: support for "simulcast" 3D.  Installs/uninstalls the
   XMO (XVD Merge Object).

   The BXVD_LinkChannels call needs to be made after the XVD channels have
   been opened, but prior to calling BXVD_StartDecode. The XMO filter is
   created and installed in this routine. The installation includes
   connecting the XMO filter to the base channel XDM, to the base channel
   XVD decoder and to the enhanced channel XVD decoder.

   The BXVD_UnlinkChannels call needs to be made after calling BXVD_StopDecode,
   but prior to closing either channel. The XMO filter is uninstalled and
   destroyed in this routine. Uninstalling the filter restores the initial
   connections between the XVD decoders and the instances of XDM.

****************************************************************************/

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_LinkChannels(
   BXVD_ChannelHandle hXvdChPrimary,
   BXVD_ChannelHandle hXvdChSecondary
   );

BERR_Code BXVD_UnlinkChannels(
   BXVD_ChannelHandle hXvdChPrimary,
   BXVD_ChannelHandle hXvdChSecondary
   );
#endif

/***************************************************************************
Summary:
   SW7425-4630
   The XDM filter provides a way for Nexus/middleware to fetch pictures from
   the XVD decoder and process them in some manner.  The pictures can then
   be passed directly to VDC or sent to XDM. The implementation of the filter
   itself and the associated picture queues is the responsibility of Nexus/middleware.

   XDM interacts with the XVD Decoder by means of a number of callbacks that
   are defined in the BXDM_Decoder_Interface structure. These callbacks allow
   XDM to get pictures from and return pictures to the XVD Decoder. When an
   XVD channel is opened, the XVD Decoder registers the callbacks with XDM by
   calling BXDM_PictureProvider_SetDecoderInterface_isr. In addition to the
   callbacks, XVD also provides a handle to a private context. This handle is
   passed back to XVD via the callbacks.

   The XDM filter uses this same callback mechanism. When a filter is installed,
   the XVD Decoder callbacks are registered with the filter, the filter's
   callbacks are registered with XDM.  The filter also provides a handle to a
   private context which is passed in the callbacks.

   It should be noted that this mechanism supports installing an unlimited
   number of filters.  However with the current design, each new filter is
   added in front of the preceding filter. The filters cannot be installed
   in a random order. Also, the filters must be uninstalled in reverse from
   the order they were installed.

****************************************************************************/

/*
 * SW7425-4630
 */
#if !B_REFSW_MINIMAL /* SWSTB-461 */
#define BXVD_FilterSettings BXDM_PictureProvider_FilterSettings
#define BXVD_FilterInterface BXDM_PictureProvider_FilterInterface

BERR_Code BXVD_InstallFilter(
   BXVD_ChannelHandle hXvdCh,
   BXDM_Decoder_Interface *pstDecoderInterface,
   void *pPrivateFilterContext,
   BXVD_FilterSettings *pstFilterSettings,
   BXVD_FilterInterface ** pstFilterInterface
   );

BERR_Code BXVD_UninstallFilter(
   BXVD_ChannelHandle hXvdCh,
   BXVD_FilterInterface * pstFilterInterface
   );
#endif

/***************************************************************************
Summary:
   SW7425-2686: added for multi-pass DQT.  A "generic" API for setting
   trick modes.

****************************************************************************/

void BXVD_GetDefaultTrickModeSettings(
   BXVD_TrickModeSettings *pstTrickModeSettings
);

BERR_Code BXVD_SetTrickModeSettings(
   BXVD_ChannelHandle hXvdCh, /* [In] XVD channel handle */
   const BXVD_TrickModeSettings *pstTrickModeSettings
);

BERR_Code BXVD_GetTrickModeSettings(
   BXVD_ChannelHandle hXvdCh, /* [In] XVD channel handle */
   BXVD_TrickModeSettings *pstTrickModeSettings
);

/*******************/
/* Deprecated APIs */
/*******************/

#if !B_REFSW_MINIMAL /* SWSTB-461 */
/***************************************************************************
Summary:
    [DEPRECATED] Sets the TSM Pass Display Threshold in number of decodable units of
    display decided by the Correct Display Algorithm
Description:
    Sets the TSM Pass Display Threshold to a fractional number of frames
    of display decided by the Correct Display Algorithm
    For example to set the window to 1.5 frames you need to set the
    numerator to 3 and denominator to 2.
    The input accepts the fraction as a numerator and a denominator. If
    this API is not called it defaults to one unit
Returns:
    BERR_SUCCESS
See Also:
    None

****************************************************************************/
BERR_Code BXVD_SetTSMPassWindow
(
 BXVD_ChannelHandle     hXvdCh,    /* [in] The XVD Channel handle */
 uint32_t ui32PassFractNumerator,  /* [in] The numerator of the fraction of frame multiple */
 uint32_t ui32PassFractDenominator /* [in] The denominator of the fraction of frame multiple */
);

/***************************************************************************
Summary:
    [DEPRECATED] Decodes and displays until the specified PTS is reached.
Description:
    Decodes and displays until the specified PTS is reached.
Returns:
    BERR_SUCCESS
See Also:
    BXVD_PauseUntoPTS

****************************************************************************/
BERR_Code BXVD_DisplayUntoPTS
(
  BXVD_ChannelHandle hXvdCh,  /* [in] The XVD Channel handle */
  bool               bEnable, /* [in] Enables Display upto the specified PTS */
  uint32_t       ui32PTS, /* [in] Display upto the specified  PTS */
  uint32_t ui32ThreshLoopAroundCntr /* [in] how much apart (after how many
                       loop arounds) the Disp PTS is programmed */
);


/***************************************************************************
Summary:
    [DEPRECATED] Decodes and displays until the specified PTS is reached.
Description:
    Keeps the decode in background and shows the last valid
    picture on display until the specified PTS is reached then resumes
    decode and display of the stream.
Returns:
    BERR_SUCCESS
See Also:
    BXVD_DisplayUntoPTS

****************************************************************************/
BERR_Code BXVD_PauseUntoPTS
(
 BXVD_ChannelHandle hXvdCh,  /* [in] The XVD Channel handle */
 bool             bEnable, /* [in] Enables Pause upto the specified PTS */
 uint32_t      ui32PTS, /* [in] Pause till the specified PTS is reached */
 uint32_t ui32ThreshLoopAroundCntr /* [in] how much apart (after how many loop
                    arounds) the Pause PTS is programmed*/
);

/***************************************************************************
Summary:
  [DEPRECATED] Allows enable and disable of 3:2 pulldown processing in display manager

See Also:
  BXVD_GetPulldown
****************************************************************************/
BERR_Code BXVD_SetPulldownMode(
   BXVD_ChannelHandle hXvdCh,          /* [in] The XVD Channel handle */
   BXVD_PulldownMode ePulldownMode     /* [in] Pulldown state */
   );

/***************************************************************************
Summary:
  [DEPRECATED] Returns enable and disable of 3:2 pulldown processing in display manager

See Also:
  BXVD_SetPulldown
****************************************************************************/
BERR_Code BXVD_GetPulldown(
   BXVD_ChannelHandle hXvdCh,          /* [in] The XVD Channel handle */
   BXVD_PulldownMode *pePulldownMode   /* [in] Pulldown state */
   );
#endif /* !B_REFSW_MINIMAL SWSTB-461 */

void BXVD_SetPictureParameterInfo_isrsafe(BXVD_PictureParameterInfo *pInfo, const BAVC_MFD_Picture *pstMFDPicture, const BXDM_Picture *pstUnifiedPicture);

#ifdef __cplusplus
}
#endif

#endif /* BXVD_H__ */
/* End of file. */

/***************************************************************************
*  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
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
***************************************************************************/
#ifndef NEXUS_CORE_COMPAT_H__
#define NEXUS_CORE_COMPAT_H__

#include "nexus_types.h"
#ifdef __cplusplus
extern "C"
{
#endif


#ifndef NEXUS_HAS_DISPLAY
typedef unsigned NEXUS_DisplayMemConfig;
typedef unsigned NEXUS_DisplayModuleSettings;
typedef unsigned NEXUS_DisplayModuleStatus;
typedef void *NEXUS_ComponentOutputHandle;
typedef void *NEXUS_CompositeOutputHandle;
typedef void *NEXUS_SvideoOutputHandle;
typedef void *NEXUS_Ccir656OutputHandle;
#endif

#if !(defined(NEXUS_HAS_AUDIO) && defined(NEXUS_NUM_I2S_MULTI_OUTPUTS) && NEXUS_NUM_I2S_MULTI_OUTPUTS)
typedef void *NEXUS_I2sMultiOutputHandle;
#endif
#ifndef NEXUS_HAS_AUDIO
typedef void *NEXUS_AudioDacHandle;
typedef void *NEXUS_AudioDummyOutputHandle;
typedef void *NEXUS_SpdifOutputHandle;
typedef void *NEXUS_I2sOutputHandle;
typedef void *NEXUS_AudioPlaybackHandle, *NEXUS_I2sInputHandle, *NEXUS_AudioMixerHandle;
typedef void *NEXUS_AudioCaptureHandle, *NEXUS_AudioMuxOutputHandle, *NEXUS_AudioDecoderPrimerHandle;
typedef void *NEXUS_AudioProcessorHandle, *NEXUS_AudioEncoderHandle, *NEXUS_AudioInputCaptureHandle;
typedef void *NEXUS_AudioDecoderHandle;
typedef unsigned NEXUS_AudioModuleUsageSettings, NEXUS_AudioModuleSettings;
typedef unsigned NEXUS_AutoVolumeLevelSettings, NEXUS_TruVolumeSettings;
typedef unsigned NEXUS_DolbyDigitalReencodeSettings, NEXUS_AudioMixerDolbySettings;
typedef unsigned NEXUS_DolbyVolume258Settings;
typedef unsigned NEXUS_KaraokeVocalSettings;
typedef unsigned NEXUS_AudioProcessorStatus, NEXUS_AudioProcessorSettings;
typedef unsigned NEXUS_AudioDecoderCodecSettings;
typedef unsigned NEXUS_AudioDecoderPresentationStatus;
typedef unsigned NEXUS_AudioDecoderSettings, NEXUS_AudioDecoderDolbyDrcMode;
typedef unsigned NEXUS_AudioEqualizerStageSettings;
/* indirection needed to ignore compat structs in 32/64 bit bridging */
#include "priv/nexus_core_audio_compat.h"
#endif

#ifndef NEXUS_HAS_GRAPHICS2D
typedef unsigned NEXUS_Graphics2DModuleSettings;
#endif

#ifndef NEXUS_HAS_SAGE
typedef unsigned NEXUS_SageModuleSettings;
#endif

#ifndef NEXUS_HAS_TRANSPORT
typedef unsigned NEXUS_TransportModuleSettings;
#endif

#ifndef NEXUS_HAS_VIDEO_ENCODER
typedef void *NEXUS_VideoEncoderHandle;
typedef unsigned NEXUS_VideoEncoderMemory;
typedef unsigned NEXUS_VideoEncoderModuleSettings;
#endif

#ifndef NEXUS_HAS_STREAM_MUX
typedef void *NEXUS_StreamMuxHandle;
typedef unsigned NEXUS_StreamMuxSystemData;
#endif

#ifndef NEXUS_HAS_VIDEO_DECODER
typedef unsigned NEXUS_VideoDecoderModuleSettings;
typedef unsigned NEXUS_VideoDecoderMemory;
#endif

#ifndef NEXUS_HAS_SMARTCARD
typedef unsigned NEXUS_SmartcardModuleSettings;
#endif

#ifndef NEXUS_HAS_SECURITY
typedef unsigned NEXUS_SecurityModuleSettings;
#endif

#ifndef NEXUS_HAS_PWM
typedef unsigned NEXUS_PwmModuleSettings;
#endif

#ifndef  NEXUS_HAS_RFM
typedef void *NEXUS_RfmHandle;
#endif

#ifndef NEXUS_HAS_HDMI_DVO
typedef void *NEXUS_HdmiDvoHandle;
#endif

#ifndef NEXUS_HAS_HDMI_OUTPUT
typedef void *NEXUS_HdmiOutputHandle;
typedef unsigned NEXUS_HdmiOutputOpenSettings;
typedef unsigned NEXUS_HdmiOutputStatus;
typedef unsigned NEXUS_HdmiOutputHdcpStatus;
typedef unsigned NEXUS_HdmiOutputCrcData;
#endif

#ifndef NEXUS_HAS_CEC
typedef void *NEXUS_CecHandle;
#endif

#ifndef NEXUS_HAS_SPI
typedef void *NEXUS_SpiHandle;
#endif

#ifndef NEXUS_HAS_I2C
typedef unsigned NEXUS_I2cModuleSettings;
typedef void *NEXUS_I2cHandle;
#endif

#ifndef NEXUS_HAS_GPIO
typedef void *NEXUS_GpioHandle;
typedef unsigned NEXUS_GpioType;

#define NEXUS_GpioType_eStandard 0
#define NEXUS_GpioType_eSpecial 0
#define NEXUS_GpioType_eTvMicro 0;
#define NEXUS_GpioType_eAonStandard 0
#define NEXUS_GpioType_eAonSpecial 0
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #ifndef NEXUS_CORE_COMPAT_H__ */

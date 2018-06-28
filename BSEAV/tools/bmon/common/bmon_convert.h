/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#ifndef __BMON_CONVERT_H__
#define __BMON_CONVERT_H__

#include <stdio.h>
#include <string.h>

#include "bmon_convert_macros.h"
#include "nexus_platform.h"
#include "nexus_audio_types.h"
#include "nexus_video_types.h"
#include "nexus_hdmi_output.h"
#include "nexus_ir_input.h"

ENUM_TO_STRING_DECLARE(audioCodecToString, NEXUS_AudioCodec);
STRING_TO_ENUM_DECLARE(stringToAudioCodec, NEXUS_AudioCodec);

ENUM_TO_STRING_DECLARE(videoCodecToString, NEXUS_VideoCodec);
STRING_TO_ENUM_DECLARE(stringToVideoCodec, NEXUS_VideoCodec);

ENUM_TO_STRING_DECLARE(videoFormatToString, NEXUS_VideoFormat);
STRING_TO_ENUM_DECLARE(stringToVideoFormat, NEXUS_VideoFormat);

ENUM_TO_STRING_DECLARE(aspectRatioToString, NEXUS_AspectRatio);
STRING_TO_ENUM_DECLARE(stringToAspectRatio, NEXUS_AspectRatio);

ENUM_TO_STRING_DECLARE(colorSpaceToString, NEXUS_ColorSpace);
STRING_TO_ENUM_DECLARE(stringToColorSpace, NEXUS_ColorSpace);

ENUM_TO_STRING_DECLARE(edidColorimetryDbToString, NEXUS_HdmiEdidColorimetryDbSupport);
STRING_TO_ENUM_DECLARE(stringToEdidColorimetry, NEXUS_HdmiEdidColorimetryDbSupport);

ENUM_TO_STRING_DECLARE(edidColorimetryDbMetadataProfileToString, NEXUS_HdmiEdidColorimetryDbMetadataProfile);
STRING_TO_ENUM_DECLARE(stringToEdidColorimetryDbMetadataProfile, NEXUS_HdmiEdidColorimetryDbMetadataProfile);

ENUM_TO_STRING_DECLARE(eotfToString, NEXUS_VideoEotf);
STRING_TO_ENUM_DECLARE(stringToEotf, NEXUS_VideoEotf);

ENUM_TO_STRING_DECLARE(videoFrameRateToString, NEXUS_VideoFrameRate);
STRING_TO_ENUM_DECLARE(stringToVideoFrameRate, NEXUS_VideoFrameRate);

ENUM_TO_STRING_DECLARE(videoProtocolLevelToString, NEXUS_VideoProtocolLevel);
STRING_TO_ENUM_DECLARE(stringToVideoProtocolLevel, NEXUS_VideoProtocolLevel);

ENUM_TO_STRING_DECLARE(videoProtocolProfileToString, NEXUS_VideoProtocolProfile);
STRING_TO_ENUM_DECLARE(stringToVideoProtocolProfile, NEXUS_VideoProtocolProfile);

ENUM_TO_STRING_DECLARE(pictureCodingToString, NEXUS_PictureCoding);
STRING_TO_ENUM_DECLARE(stringToPictureCoding, NEXUS_PictureCoding);

ENUM_TO_STRING_DECLARE(ptsTypeToString, NEXUS_PtsType);
STRING_TO_ENUM_DECLARE(stringToPtsType, NEXUS_PtsType);

ENUM_TO_STRING_DECLARE(irInputModeToString, NEXUS_IrInputMode);
STRING_TO_ENUM_DECLARE(stringToIrInputMode, NEXUS_IrInputMode);

#endif /* __BMON_CONVERT_H__ */
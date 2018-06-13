/***************************************************************************
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
 *
 * Module Description:
 *
 ***************************************************************************/
#ifndef BBOX_VCE_H__
#define BBOX_VCE_H__

#include "bstd.h"
#include "berr_ids.h"    /* Error codes */
#include "bfmt.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BBOX_VCE_CHANNEL_INFO_V1( _instance, _channels, _memcIndex, _videoFormat ) \
{\
   _instance, /* uiInstance */\
   _channels, /* uiChannels */\
   _memcIndex, /* uiMemcIndex */\
   _memcIndex, /* uiFirmwareMemcIndex */\
   _videoFormat, /* eVideoFormat */\
   16, /* uiPixelAlignment */\
   { \
      { _channels, _videoFormat },\
      { 0, BFMT_VideoFmt_eMaxCount },\
   }\
}

#define BBOX_VCE_CHANNEL_INFO_NO_TRANSCODE( _instance, _channels, _memcIndex )\
   BBOX_VCE_CHANNEL_INFO_V1(\
      0, 0x00, 0, 0\
   )

#define BBOX_VCE_CHANNEL_INFO_720p25( _instance, _channels, _memcIndex )\
   BBOX_VCE_CHANNEL_INFO_V1(\
      _instance, _channels, _memcIndex, BFMT_VideoFmt_e720p_25Hz\
   )

#define BBOX_VCE_CHANNEL_INFO_720p30( _instance, _channels, _memcIndex )\
   BBOX_VCE_CHANNEL_INFO_V1(\
      _instance, _channels, _memcIndex, BFMT_VideoFmt_e720p_30Hz\
   )

#define BBOX_VCE_CHANNEL_INFO_720p60( _instance, _channels, _memcIndex )\
   BBOX_VCE_CHANNEL_INFO_V1(\
      _instance, _channels, _memcIndex, BFMT_VideoFmt_e720p\
   )

#define BBOX_VCE_CHANNEL_INFO_1080p25( _instance, _channels, _memcIndex )\
   BBOX_VCE_CHANNEL_INFO_V1(\
      _instance, _channels, _memcIndex, BFMT_VideoFmt_e1080p_25Hz\
   )

#define BBOX_VCE_CHANNEL_INFO_1080p30( _instance, _channels, _memcIndex )\
   BBOX_VCE_CHANNEL_INFO_V1(\
      _instance, _channels, _memcIndex, BFMT_VideoFmt_e1080p_30Hz\
   )

#define BBOX_VCE_CHANNEL_INFO_1080p60( _instance, _channels, _memcIndex )\
   BBOX_VCE_CHANNEL_INFO_V1(\
   _instance, _channels, _memcIndex, BFMT_VideoFmt_e1080p\
   )

#define BBOX_VCE_CHANNEL_INFO_V2( _instance, _channels, _memcIndex, _fwMemcIndex, _videoFormat ) \
{\
   _instance, /* uiInstance */\
   _channels, /* uiChannels */\
   _memcIndex, /* uiMemcIndex */\
   _fwMemcIndex, /* uiFirmwareMemcIndex */\
   _videoFormat, /* eVideoFormat */\
   16, /* uiPixelAlignment */\
   { \
      { _channels, _videoFormat },\
      { 0, BFMT_VideoFmt_eMaxCount },\
   }\
}

#define BBOX_VCE_CHANNEL_INFO_720p25_V2( _instance, _channels, _memcIndex, _fwMemcIndex )\
   BBOX_VCE_CHANNEL_INFO_V2(\
      _instance, _channels, _memcIndex, _fwMemcIndex, BFMT_VideoFmt_e720p_25Hz\
   )

#define BBOX_VCE_CHANNEL_INFO_720p30_V2( _instance, _channels, _memcIndex, _fwMemcIndex )\
   BBOX_VCE_CHANNEL_INFO_V2(\
      _instance, _channels, _memcIndex, _fwMemcIndex, BFMT_VideoFmt_e720p_30Hz\
   )

#define BBOX_VCE_CHANNEL_INFO_720p60_V2( _instance, _channels, _memcIndex, _fwMemcIndex )\
   BBOX_VCE_CHANNEL_INFO_V2(\
      _instance, _channels, _memcIndex, _fwMemcIndex, BFMT_VideoFmt_e720p\
   )

#define BBOX_VCE_CHANNEL_INFO_1080p25_V2( _instance, _channels, _memcIndex, _fwMemcIndex )\
   BBOX_VCE_CHANNEL_INFO_V2(\
      _instance, _channels, _memcIndex, _fwMemcIndex, BFMT_VideoFmt_e1080p_25Hz\
   )

#define BBOX_VCE_CHANNEL_INFO_1080p30_V2( _instance, _channels, _memcIndex, _fwMemcIndex )\
   BBOX_VCE_CHANNEL_INFO_V2(\
      _instance, _channels, _memcIndex, _fwMemcIndex, BFMT_VideoFmt_e1080p_30Hz\
   )

#define BBOX_VCE_CHANNEL_INFO_1080p60_V2( _instance, _channels, _memcIndex, _fwMemcIndex )\
   BBOX_VCE_CHANNEL_INFO_V2(\
   _instance, _channels, _memcIndex, _fwMemcIndex, BFMT_VideoFmt_e1080p\
   )

/* _modes should follow this format:
 * {
 *    {_channels_0, _videoFormat_0},
 *    {_channels_1, _videoFormat_1},
 *    { 0, BFMT_VideoFmt_eMaxCount }
 *    etc.
 * }
 */
#define BBOX_VCE_CHANNEL_INFO_V3_EXCLUSIVE_DUAL( _instance, _memcIndex, _fwMemcIndex, _channels_0, _videoFormat_0, _channels_1, _videoFormat_1 ) \
{\
   _instance, /* uiInstance */\
   _channels_0, /* uiChannels */\
   _memcIndex, /* uiMemcIndex */\
   _fwMemcIndex, /* uiFirmwareMemcIndex */\
   _videoFormat_0, /* eVideoFormat */\
   16, /* uiPixelAlignment */\
   { \
      { _channels_0, _videoFormat_0 },\
      { _channels_1, _videoFormat_1 },\
      { 0, BFMT_VideoFmt_eMaxCount },\
   }\
}

#define BBOX_VCE_MAX_MODE_COUNT 3

typedef struct BBOX_Vce_Channel_Capabilities
{
   uint8_t uiInstance;
   uint8_t uiChannels; /* Deprecated. See stMode[0].uiChannels */
   uint8_t uiMemcIndex; /* Picture Buffer MEMC Index */
   uint8_t uiFirmwareMemcIndex; /* Firmware MEMC Index */
   BFMT_VideoFmt eVideoFormat; /* Deprecated. See stMode[0].eVideoFormat */
   uint8_t uiPixelAlignment; /* height and width must me a multiple of uiPixelAlignment */

   /* List of supported encode combinations
    * { 0, BFMT_VideoFmt_eMaxCount } indicates the end of the list */
   struct
   {
      uint8_t uiChannels; /* A bit mask indicating which channels are valid */
      BFMT_VideoFmt eVideoFormat;
   } stMode[BBOX_VCE_MAX_MODE_COUNT];
} BBOX_Vce_Channel_Capabilities;

#define BBOX_VCE_MAX_INSTANCE_COUNT 2
#define BBOX_VCE_MAX_CHANNEL_COUNT 3

/***************************************************************************
Summary:
    List of VCE's exposed capabilities
****************************************************************************/
typedef struct BBOX_Vce_Capabilities
{
   uint32_t uiBoxId;
   BBOX_Vce_Channel_Capabilities stInstance[BBOX_VCE_MAX_INSTANCE_COUNT];
} BBOX_Vce_Capabilities;

extern const BBOX_Vce_Capabilities BBOX_P_Vce_CapabilitiesLUT[];
extern const size_t BBOX_P_Vce_CapabilitiesLUT_size;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BBOX_VCE_H__ */

/* end of file */

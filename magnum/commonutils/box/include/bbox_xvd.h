/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *****************************************************************************/

#ifndef BBOX_XVD_H__
#define BBOX_XVD_H__

#include "bavc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BBOX_XVD_MAX_DECODERS 3
#define BBOX_XVD_MAX_CHANNELS 3

#define BBOX_XVD_UNUSED ((unsigned)-1)

typedef enum BBOX_XVD_DecodeResolution
{
   BBOX_XVD_DecodeResolution_eHD,
   BBOX_XVD_DecodeResolution_eSD,
   BBOX_XVD_DecodeResolution_eCIF,
   BBOX_XVD_DecodeResolution_eQCIF,
   BBOX_XVD_DecodeResolution_e4K,
   BBOX_XVD_DecodeResolution_eMaxModes
} BBOX_XVD_DecodeResolution;


#define BBOX_XVD_DECODER_INFO_V1( _memIndex, _secondaryMemcIndex, _numChannels, \
                                  _bitDepth0, _maxResolution0, _frameRate0, _mfdIndex0, _nexusIndex0,  \
                                  _bitDepth1, _maxResolution1, _frameRate1, _mfdIndex1, _nexusIndex1,  \
                                  _bitDepth2, _maxResolution2, _frameRate2, _mfdIndex2, _nexusIndex2 ) \
{ \
   _memIndex,           \
   _secondaryMemcIndex, \
   _numChannels, \
   {\
      BBOX_XVD_CHANNEL_INFO_V1( _bitDepth0, _maxResolution0, _frameRate0, _mfdIndex0, _nexusIndex0 ), \
      BBOX_XVD_CHANNEL_INFO_V1( _bitDepth1, _maxResolution1, _frameRate1, _mfdIndex1, _nexusIndex1 ), \
      BBOX_XVD_CHANNEL_INFO_V1( _bitDepth2, _maxResolution2, _frameRate2, _mfdIndex2, _nexusIndex2 ), \
   }\
}

#define BBOX_XVD_CHANNEL_INFO_V1( _bitDepth, _maxResolution, _frameRate, _mfdIndex, _nexusIndex ) \
{ \
   _mfdIndex,       \
   _nexusIndex,     \
   BBOX_XVD_DECODER_USAGE_INFO_V1( _bitDepth, _maxResolution, _frameRate ), \
}

#define BBOX_XVD_DECODER_USAGE_INFO_V1( _bitDepth, _maxResolution, _frameRate ) \
{ \
   _bitDepth, \
   _maxResolution, \
   _frameRate \
}

#define BBOX_XVD_DECODER_INFO_4Kp60_V1( _memIndex, _secondaryMemcIndex, _bitDepth, _mfdIndex0, _nexusIndex0) \
{\
      BBOX_XVD_DECODER_INFO_V1(          \
         _memIndex, _secondaryMemcIndex, 1,                             \
         _bitDepth, BBOX_XVD_DecodeResolution_e4K, 60,_mfdIndex0, _nexusIndex0, \
         BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, \
         BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED), \
}

#define BBOX_XVD_DECODER_INFO_4Kp50_V1( _memIndex, _secondaryMemcIndex, _bitDepth, _mfdIndex0, _nexusIndex0) \
{\
      BBOX_XVD_DECODER_INFO_V1(          \
         _memIndex, _secondaryMemcIndex, 1,                             \
         _bitDepth, BBOX_XVD_DecodeResolution_e4K, 50,_mfdIndex0, _nexusIndex0, \
         BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, \
         BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED), \
}

#define BBOX_XVD_DECODER_INFO_4Kp25_V1( _memIndex, _secondaryMemcIndex, _bitDepth, _mfdIndex0, _nexusIndex0) \
{\
      BBOX_XVD_DECODER_INFO_V1(          \
         _memIndex, _secondaryMemcIndex, 1,                             \
         _bitDepth, BBOX_XVD_DecodeResolution_e4K, 25,_mfdIndex0, _nexusIndex0, \
         BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, \
         BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED), \
}


#define BBOX_XVD_DECODER_INFO_Dual4Kp60_V1( _memIndex, _secondaryMemcIndex, _bitDepth, _mfdIndex0, _nexusIndex0, _mfdIndex1, _nexusIndex1) \
{ \
   BBOX_XVD_DECODER_INFO_V1( \
      _memIndex, _secondaryMemcIndex, 2, \
      _bitDepth, BBOX_XVD_DecodeResolution_e4K, 60, _mfdIndex0, _nexusIndex0, \
      _bitDepth, BBOX_XVD_DecodeResolution_e4K, 60, _mfdIndex1, _nexusIndex1, \
      BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED) \
}

#define BBOX_XVD_DECODER_INFO_1080p60_V1( _memIndex, _secondaryMemcIndex, _bitDepth, _mfdIndex0, _nexusIndex0) \
{ \
   BBOX_XVD_DECODER_INFO_V1( \
      _memIndex, _secondaryMemcIndex, 1,                                   \
      _bitDepth, BBOX_XVD_DecodeResolution_eHD, 60, _mfdIndex0, _nexusIndex0, \
      BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, \
      BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED) \
}

#define BBOX_XVD_DECODER_INFO_1080p50_V1( _memIndex, _secondaryMemcIndex, _bitDepth, _mfdIndex0, _nexusIndex0) \
{ \
   BBOX_XVD_DECODER_INFO_V1( \
      _memIndex, _secondaryMemcIndex, 1,                                   \
      _bitDepth, BBOX_XVD_DecodeResolution_eHD, 50, _mfdIndex0, _nexusIndex0, \
      BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, \
      BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED) \
}

#define BBOX_XVD_DECODER_INFO_1080p25_V1( _memIndex, _secondaryMemcIndex, _bitDepth, _mfdIndex0, _nexusIndex0) \
{ \
   BBOX_XVD_DECODER_INFO_V1( \
      _memIndex, _secondaryMemcIndex, 1,                                   \
      _bitDepth, BBOX_XVD_DecodeResolution_eHD, 25, _mfdIndex0, _nexusIndex0, \
      BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, \
      BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED) \
}

#define BBOX_XVD_DECODER_INFO_Dual1080p60_V1( _memIndex, _secondaryMemcIndex, _bitDepth, _mfdIndex0, _nexusIndex0, _mfdIndex1, _nexusIndex1) \
{ \
   BBOX_XVD_DECODER_INFO_V1( \
      _memIndex, _secondaryMemcIndex, 2,                                  \
      _bitDepth, BBOX_XVD_DecodeResolution_eHD, 60, _mfdIndex0, _nexusIndex0, \
      _bitDepth, BBOX_XVD_DecodeResolution_eHD, 60, _mfdIndex1, _nexusIndex1, \
      BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED) \
}

#define BBOX_XVD_DECODER_INFO_1080p30_V1( _memIndex, _secondaryMemcIndex, _bitDepth, _mfdIndex0, _nexusIndex0) \
{ \
   BBOX_XVD_DECODER_INFO_V1( \
      _memIndex, _secondaryMemcIndex, 1, \
      _bitDepth,  BBOX_XVD_DecodeResolution_eHD, 30, _mfdIndex0, _nexusIndex0, \
      BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, \
      BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED) \
}

#define BBOX_XVD_DECODER_INFO_Dual1080p30_V1( _memIndex, _secondaryMemcIndex, _bitDepth, _mfdIndex0, _nexusIndex0, _mfdIndex1, _nexusIndex1) \
{ \
   BBOX_XVD_DECODER_INFO_V1( \
      _memIndex, _secondaryMemcIndex, 2,                                \
      _bitDepth, BBOX_XVD_DecodeResolution_eHD, 30, _mfdIndex0, _nexusIndex0, \
      _bitDepth, BBOX_XVD_DecodeResolution_eHD, 30, _mfdIndex1, _nexusIndex1, \
      BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED) \
}

#define BBOX_XVD_DECODER_INFO_Dual1080p50_V1( _memIndex, _secondaryMemcIndex, _bitDepth, _mfdIndex0, _nexusIndex0, _mfdIndex1, _nexusIndex1) \
{ \
   BBOX_XVD_DECODER_INFO_V1( \
      _memIndex, _secondaryMemcIndex, 2, \
      _bitDepth, BBOX_XVD_DecodeResolution_eHD, 50, _mfdIndex0, _nexusIndex0, \
      _bitDepth, BBOX_XVD_DecodeResolution_eHD, 50, _mfdIndex1, _nexusIndex1, \
      BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED) \
}

#define BBOX_XVD_DECODER_INFO_Triple1080p25_V1( _memIndex, _secondaryMemcIndex, _bitDepth, _mfdIndex0, _nexusIndex0, \
                                                _mfdIndex1, _nexusIndex1,  _mfdIndex2, _nexusIndex2) \
{ \
   BBOX_XVD_DECODER_INFO_V1( \
      _memIndex, _secondaryMemcIndex, 3, \
      _bitDepth, BBOX_XVD_DecodeResolution_eHD, 25, _mfdIndex0, _nexusIndex0, \
      _bitDepth, BBOX_XVD_DecodeResolution_eHD, 25, _mfdIndex1, _nexusIndex1, \
      _bitDepth, BBOX_XVD_DecodeResolution_eHD, 25, _mfdIndex2, _nexusIndex2) \
}

#define BBOX_XVD_DECODER_INFO_Triple1080p60_V1( _memIndex, _secondaryMemcIndex, _bitDepth, _mfdIndex0, _nexusIndex0, \
                                                _mfdIndex1, _nexusIndex1,  _mfdIndex2, _nexusIndex2) \
{ \
   BBOX_XVD_DECODER_INFO_V1( \
      _memIndex, _secondaryMemcIndex, 3, \
      _bitDepth, BBOX_XVD_DecodeResolution_eHD, 60, _mfdIndex0, _nexusIndex0, \
      _bitDepth, BBOX_XVD_DecodeResolution_eHD, 60, _mfdIndex1, _nexusIndex1, \
      _bitDepth, BBOX_XVD_DecodeResolution_eHD, 60, _mfdIndex2, _nexusIndex2) \
}

#define BBOX_XVD_DECODER_INFO_288p50_V1( _memIndex, _secondaryMemcIndex, _bitDepth, _mfdIndex0, _nexusIndex0) \
{ \
   BBOX_XVD_DECODER_INFO_V1( \
      _memIndex, _secondaryMemcIndex, 1,                                   \
      _bitDepth, BBOX_XVD_DecodeResolution_eCIF, 50, _mfdIndex0, _nexusIndex0, \
      BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, \
      BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED) \
}

#define BBOX_XVD_DECODER_INFO_4Kp60_1080p30_V1( _memIndex, _secondaryMemcIndex, \
                                                _bitDepth0, _mfdIndex0, _nexusIndex0, \
                                                _bitDepth1, _mfdIndex1, _nexusIndex1) \
{ \
   BBOX_XVD_DECODER_INFO_V1( \
      _memIndex, _secondaryMemcIndex, 2, \
      _bitDepth0, BBOX_XVD_DecodeResolution_e4K, 60, _mfdIndex0, _nexusIndex0, \
      _bitDepth1, BBOX_XVD_DecodeResolution_eHD, 30, _mfdIndex1, _nexusIndex1, \
      BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED) \
}

#define BBOX_XVD_DECODER_INFO_4Kp60_1080p60_V1( _memIndex, _secondaryMemcIndex, \
                                                _bitDepth0, _mfdIndex0, _nexusIndex0, \
                                                _bitDepth1, _mfdIndex1, _nexusIndex1) \
{ \
   BBOX_XVD_DECODER_INFO_V1( \
      _memIndex, _secondaryMemcIndex, 2, \
      _bitDepth0, BBOX_XVD_DecodeResolution_e4K, 60, _mfdIndex0, _nexusIndex0, \
      _bitDepth1, BBOX_XVD_DecodeResolution_eHD, 60, _mfdIndex1, _nexusIndex1, \
      BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED) \
}

#define BBOX_XVD_DECODER_INFO_1080p30_1080p60_V1( _memIndex, _secondaryMemcIndex, \
                                                _bitDepth0, _mfdIndex0, _nexusIndex0, \
                                                _bitDepth1, _mfdIndex1, _nexusIndex1) \
{ \
   BBOX_XVD_DECODER_INFO_V1( \
      _memIndex, _secondaryMemcIndex, 2, \
      _bitDepth0, BBOX_XVD_DecodeResolution_e4K, 30, _mfdIndex0, _nexusIndex0, \
      _bitDepth1, BBOX_XVD_DecodeResolution_eHD, 60, _mfdIndex1, _nexusIndex1, \
      BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED) \
}
#define BBOX_XVD_DECODER_INFO_1080p30_480p30_V1( _memIndex, _secondaryMemcIndex, \
                                                _bitDepth0, _mfdIndex0, _nexusIndex0, \
                                                _bitDepth1, _mfdIndex1, _nexusIndex1) \
{ \
   BBOX_XVD_DECODER_INFO_V1( \
      _memIndex, _secondaryMemcIndex, 2, \
      _bitDepth0, BBOX_XVD_DecodeResolution_e4K, 30, _mfdIndex0, _nexusIndex0, \
      _bitDepth1, BBOX_XVD_DecodeResolution_eHD, 30, _mfdIndex1, _nexusIndex1, \
      BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED, BBOX_XVD_UNUSED) \
}

typedef struct BBOX_XVD_Decoder_Usage
{
    BAVC_VideoBitDepth bitDepth; /* 8 or 10 bit */
    BBOX_XVD_DecodeResolution maxResolution;
    unsigned framerate; /* 25, 30, 50, 60 */
} BBOX_XVD_Decoder_Usage;


typedef struct BBOX_Xvd_Decoder_Config
{
    struct
    {
       uint8_t memcIndex;          /* Memory controller index for pictures, for this box mode */
       uint32_t secondaryMemcIndex; /* Can be BBOX_XVD_UNUSED */
       uint8_t numChannels;
       struct
       {
          uint32_t mfdIndex;
          uint32_t nexusIndex;
          BBOX_XVD_Decoder_Usage DecoderUsage;
       } stChannel[BBOX_XVD_MAX_CHANNELS]; /* bounded by pDecoderUsage->numChannels */
    } stDevice;
} BBOX_Xvd_Decoder_Config;

typedef struct BBOX_Xvd_Config
{
   uint32_t uiBoxId;
   BBOX_Xvd_Decoder_Config stInstance[BBOX_XVD_MAX_DECODERS];
} BBOX_Xvd_Config;

#ifdef __cplusplus
}
#endif

#endif

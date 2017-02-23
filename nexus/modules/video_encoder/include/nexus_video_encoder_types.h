/***************************************************************************
 *  Copyright (C) 2016-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 **************************************************************************/
#ifndef NEXUS_VIDEO_ENCODER_TYPES_H__
#define NEXUS_VIDEO_ENCODER_TYPES_H__

#include "nexus_types.h"
#include "nexus_video_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
**/
typedef struct NEXUS_VideoEncoder *NEXUS_VideoEncoderHandle;


/**
Summary:
**/
typedef enum NEXUS_VideoCodecLevel {
   NEXUS_VideoCodecLevel_eUnknown = 0,
   NEXUS_VideoCodecLevel_e00,
   NEXUS_VideoCodecLevel_e10,
   NEXUS_VideoCodecLevel_e1B,
   NEXUS_VideoCodecLevel_e11,
   NEXUS_VideoCodecLevel_e12,
   NEXUS_VideoCodecLevel_e13,
   NEXUS_VideoCodecLevel_e20,
   NEXUS_VideoCodecLevel_e21,
   NEXUS_VideoCodecLevel_e22,
   NEXUS_VideoCodecLevel_e30,
   NEXUS_VideoCodecLevel_e31,
   NEXUS_VideoCodecLevel_e32,
   NEXUS_VideoCodecLevel_e40,
   NEXUS_VideoCodecLevel_e41,
   NEXUS_VideoCodecLevel_e42,
   NEXUS_VideoCodecLevel_e50,
   NEXUS_VideoCodecLevel_e51,
   NEXUS_VideoCodecLevel_e60,
   NEXUS_VideoCodecLevel_e62,
   NEXUS_VideoCodecLevel_eLow,
   NEXUS_VideoCodecLevel_eMain,
   NEXUS_VideoCodecLevel_eHigh,
   NEXUS_VideoCodecLevel_eHigh1440,
   NEXUS_VideoCodecLevel_eMax
} NEXUS_VideoCodecLevel;

/**
Summary:
**/
typedef enum NEXUS_VideoCodecProfile
{
   NEXUS_VideoCodecProfile_eUnknown = 0,
   NEXUS_VideoCodecProfile_eSimple,
   NEXUS_VideoCodecProfile_eMain,
   NEXUS_VideoCodecProfile_eHigh,
   NEXUS_VideoCodecProfile_eAdvanced,
   NEXUS_VideoCodecProfile_eJizhun,
   NEXUS_VideoCodecProfile_eSnrScalable,
   NEXUS_VideoCodecProfile_eSpatiallyScalable,
   NEXUS_VideoCodecProfile_eAdvancedSimple,
   NEXUS_VideoCodecProfile_eBaseline,

   NEXUS_VideoCodecProfile_eMax
} NEXUS_VideoCodecProfile;

/**
Summary:
**/
typedef enum NEXUS_EntropyCoding
{
    NEXUS_EntropyCoding_eAuto,
    NEXUS_EntropyCoding_eCavlc,
    NEXUS_EntropyCoding_eCabac,
    NEXUS_EntropyCoding_eMax
} NEXUS_EntropyCoding;


typedef struct NEXUS_VideoEncoder_MemoryConfig 
{
    /* This structure defines heap sizes in bytes per hardware encoder
       These values should be calculated based upon the maximum number of concurrent encodes
       you wish to perform, and must be appropriately sized based upon stream codec and resolution.
       */
       
    unsigned general;   /* General encoder heap.
                           This is used for general buffer allocation that must be host accessible.
                           Firmware is not allocated from this size. RAVE CDB/ITB buffers are not allocated from this size. */
    unsigned secure;    /* Secure encoder heap. This is only used for SVP systems where the host is prevented from reading clear, compressed data. For
                           non-SVP systems, set to zero.*/
    unsigned picture;   /* Picture buffer heap. This memory does not need to be host accessible.  On SVP systems, this is the size
                           of the Uncompressed Restricted Region (URR) */
    unsigned firmware;  /* Firmware Heap. This is used for allocating FW memory.  On non-SVP systems, this must be host accessible. */
    unsigned index;     /* Index (ITB) Heap.  This is used for allocating index (ITB) memory for the output of the encoder.
                           This memory needs to be host accessible. */
    unsigned data;      /* Data (CDB) Heap. This is used for allocating data (CDB) memory for the output of the encoder.
                           On SVP systems, this is the size of the Compresses Restricted Region (CRR). */
} NEXUS_VideoEncoder_MemoryConfig;

/**
Init time memory per channel.
Can be set with NEXUS_MemoryConfigurationSettings at init time.
Can be read with NEXUS_VideoEncoderCapabilities at runtime.
**/
typedef struct NEXUS_VideoEncoderMemory {
    bool used;
    bool interlaced;
    unsigned maxWidth, maxHeight;
    NEXUS_VideoFrameRate maxFrameRate;
} NEXUS_VideoEncoderMemory;

typedef struct NEXUS_VideoEncoderModuleSettings
{
    bool deferInit; /* if set to true, HW and FW initialization will be deferred until actually used */
} NEXUS_VideoEncoderModuleSettings;


#ifdef __cplusplus
}
#endif


#endif /* NEXUS_VIDEO_ENCODER_TYPES_H__ */


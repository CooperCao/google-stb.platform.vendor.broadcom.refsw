/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#ifndef BOXMODES_DEFINES_H__
#define BOXMODES_DEFINES_H__

#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_video_decoder_init.h"
#endif
#if NEXUS_HAS_DISPLAY
#include "nexus_display_init.h"
#endif
#if NEXUS_HAS_VIDEO_ENCODER
#include "nexus_video_encoder_init.h"
#endif

typedef enum {
    Memconfig_VideoDecoderProperty_eUnused,
    Memconfig_VideoDecoderProperty_eMain,
    Memconfig_VideoDecoderProperty_ePip,
    Memconfig_VideoDecoderProperty_eTranscode,
    Memconfig_VideoDecoderProperty_eGraphicsPip,
    Memconfig_VideoDecoderProperty_eMax
} Memconfig_VideoDecoderProperty;

typedef enum {
    Memconfig_DisplayProperty_eUnused,
    Memconfig_DisplayProperty_ePrimary,
    Memconfig_DisplayProperty_eSecondary,
    Memconfig_DisplayProperty_eTertiary,
    Memconfig_DisplayProperty_eTranscode,
    Memconfig_DisplayProperty_eGraphicsPip,
    Memconfig_DisplayProperty_eMax
} Memconfig_DisplayProperty;

typedef struct {
    unsigned int boxModeId; /* 1, 3 for 97445; 2, 4 for 97252  */
    char        *boxModeDescription;
    struct {
        Memconfig_VideoDecoderProperty property;
        int    pictureBufferHeapIdx;
        char * usage;
        int    secondaryPictureBufferHeapIdx;
        char * secondaryUsage;
    } videoDecoder[NEXUS_MAX_VIDEO_DECODERS];
    struct {
        Memconfig_DisplayProperty property;
        int              mainPictureBufferHeapIdx;
        char *           usageMain;
        int              pipPictureBufferHeapIdx;
        char *           usagePip;
    } display[NEXUS_MAX_DISPLAYS];
    struct {
        bool             used;
        Memconfig_DisplayProperty property;
        int              heapIdx; /*NEXUS_MEMC2_GRAPHICS_HEAP, NEXUS_MEMC1_GRAPHICS_HEAP  from nexus_platform_features.h */
        char *           usage;
    } graphics[NEXUS_MAX_DISPLAYS];
    struct {
        int              videoDecoder;
        int              audioDecoder;
        int              graphicsWidth; /* if either is zero, graphics disabled */
        int              graphicsHeight;
    } transcoders[NEXUS_MAX_VIDEO_ENCODERS];
    struct {
        bool             used;
        unsigned         heapIdx;
        char *           usage;
    } graphics3d;
    struct {
        bool             used;
        unsigned         heapIdx;
        char *           usage;
        unsigned         usageSizeBytes;
    } frontend;
} Memconfig_BoxMode;

typedef struct {
    int  memcIndex;
    char heapName[128];
    char heapMapping[128];
} Memconfig_HeapInfo;

#define BMEMCONFIG_MAX_BOXMODES       30
#define BMEMCONFIG_MAX_DDR_SCB_STRING 128

typedef struct {
    int  boxmode;
    int  numMemc;
    char strDdrScb[BMEMCONFIG_MAX_DDR_SCB_STRING]; /* for 97425, this is the string that is possible ... DDR3@866MHz SCB@277.714285714286MHz */
    char strProductId[8];
    unsigned int ddrFreq;
    unsigned int scbFreq;
} bmemconfig_box_info;

int Memconfig_GetBoxModeDefaultSettings( int boxMode, Memconfig_BoxMode *pSettings );char * Memconfig_GetVideoDecoderPropertyStr ( int propertyIdx );
char * Memconfig_GetDisplayPropertyStr ( int propertyIdx );
#endif /* BOXMODES_DEFINES_H__ */

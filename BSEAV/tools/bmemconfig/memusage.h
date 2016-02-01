/******************************************************************************
 *    (c)2008-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *****************************************************************************/

#include "nexus_platform.h"
#include "nexus_core_utils.h"
#include "nexus_surface.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct Memconfig_AppUsageSettings {
    bool         bSecure; /* if on, RS/XC go to secure heap, CDB/ITB for audio/video, still decode */
                          /*  record CDB/ITB  and encode are always, playpump  buffers and message buffers main heap*/
    struct {
        unsigned int number;  /* 4 */
        unsigned int max;     /* get this from platform_features.h */
        unsigned int bitRate;
        unsigned int latency;
    } record; /* record CDB/ITB always non-secure */
    struct {
        unsigned int number; /* 4 */
        unsigned int max;    /* get this from platform_features.h */
        unsigned int size;   /*Read Only for now.  not configurable, set by define*/
    } playback;
    struct {
        unsigned int number; /* 4 */
        unsigned int max;    /* get this from platform_features.h */
        unsigned int size;   /* message BufSize */
    } message;
    struct {
        unsigned int number; /* 4 */
        unsigned int max;    /* get this from platform_features.h */
        unsigned int size;   /*Read Only for now.  not configurable, set by define*/
    } live;
    struct {
        unsigned int number; /* 2 */
        unsigned int max;    /* get this from platform_features.h */
        unsigned int size;   /*Read Only for now.  not configurable, set by define*/
    } remux;
#if NEXUS_HAS_VIDEO_DECODER
    struct {
        bool         enabled;
        bool         bIp;
        bool         b3840x2160;
        bool         bSecure; /* RAVE could be coming from secure or non-secure */
        bool         bSoft;
        bool         bMvc;
        unsigned int numMosaic;
        unsigned int numFcc;
    } videoDecoder[NEXUS_NUM_VIDEO_DECODERS]; /*includeds decoder used for video decoder and transcode */
#endif
#if NEXUS_NUM_STILL_DECODES
    struct {
        bool         enabled;
        bool         bIp;
        bool         b3840x2160;
        bool         bSecure;
        bool         bSoft;
        bool         bMvc;
        unsigned int numFcc;
    } stillDecoder[NEXUS_NUM_STILL_DECODES]; /*includeds decoder used for video decoder and transcode */
#endif
    struct {
        bool         enabled;
        bool         bPassthru;
        bool         bSecondary;
        bool         bIp;
        bool         bSecure;
        unsigned int numFcc;
    } audioDecoder[NEXUS_NUM_AUDIO_DECODERS]; /*includeds decoder used for audio decoder and transcode */

#if NEXUS_NUM_VIDEO_ENCODERS
    struct {
        bool enabled;
        bool streamMux; /*default should be on */
    } encoders[NEXUS_NUM_VIDEO_ENCODERS];
#endif /* NEXUS_NUM_VIDEO_ENCODERS */

#if NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS
     struct {
        unsigned numFrameBufs;
        unsigned width;
        unsigned height;
        unsigned bits_per_pixel;
        bool     teletext;
    }surfaceSettings[NEXUS_NUM_DISPLAYS];
#endif

     struct {
        bool     enabled;
        unsigned maxChannels;  /*16 max channels*/
        unsigned numChannels;
    } sidSettings;
} Memconfig_AppUsageSettings;

typedef struct Memconfig_AppMemUsage {
    struct {  /*  Recpump CDB/ITB * number  of records (ALWAYS on main heap) */
        unsigned int bytesGeneral;
    } record;


    struct { /* (RS buffer for PBP +  RAVE XC buffer for PBP + playback buffer )*number of playbacks */
        unsigned int bytesGeneral;
        unsigned int bytesSecure; /* if secure is enabled, RS and XC go here */
    } playback;


    struct  /*(XC buffer for message  * number of live channels) + ( XC buffer for message  * number of playback)  + message Buffer) * number of message buffers   */{
        unsigned int bytesGeneral;
        unsigned int bytesSecure; /* if secure is enabled, RS and XC go here */
    } message;

    struct { /* (RS for IBP + XC IBP RAVE) * number of live channels */
        unsigned int bytesGeneral;
        unsigned int bytesSecure; /* if secure is enabled, RS and XC go here */
    } live;
    struct { /* ( remux message XC Buf size * number of live + remux message XC Buf size * numer of playback)*number of remux ( up to 2)*/
        unsigned int bytesGeneral;
        unsigned int bytesSecure; /* if secure is enabled, RS and XC go here */
    } remux;
    struct {
        unsigned int bytesGeneral;
        unsigned int bytesSecure;
    } videoDecoder[NEXUS_NUM_VIDEO_DECODERS];
    struct {
        unsigned int bytesGeneral; /* video decoder RAVE CDB/ITB */
        unsigned int bytesSecure;
    } audioDecoder[NEXUS_NUM_AUDIO_DECODERS];
    struct {
        unsigned int bytesGeneral;
        unsigned int bytesSecure;
    } encode;

#if NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS
    struct {
        unsigned int bytesGeneral;
    } surfaceSettings[NEXUS_NUM_DISPLAYS];
    struct {
        unsigned int bytesGeneral;
    } teletext;
#endif

    struct {
        unsigned int bytesGeneral;
    } sidSettings;

    struct {
        unsigned int bytesGeneral;
    } graphics3d;

    struct {
        unsigned int bytesGeneral;
    } frontend;

    unsigned mainHeapTotal;
    unsigned secureHeapTotal;
} Memconfig_AppMemUsage;

int Memconfig_AppUsageGetDefaultSettings( Memconfig_AppUsageSettings *settings );
int Memconfig_AppUsageCalculate( Memconfig_AppUsageSettings *settings, Memconfig_AppMemUsage *pMemUsage );
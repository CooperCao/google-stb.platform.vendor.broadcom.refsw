/******************************************************************************
 *    (c)2013-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELYn
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
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ******************************************************************************/
#ifndef __BBOXREPORT_SVGPLOT_H__
#define __BBOXREPORT_SVGPLOT_H__

#include "nexus_types.h"
#include "nexus_video_types.h"
#include "nexus_video_decoder_types.h"
#include "nexus_platform_memconfig.h"
#if NEXUS_HAS_VIDEO_ENCODER
#include "nexus_video_encoder_init.h"
#endif
#include "boxmodes_defines.h"

typedef char STRING32[32];

typedef struct
{
    char platform[12];
    int  boxmode;
    int  boxmodeFirst; /* use to add page-break after the first boxmode is done */
    char strDdrScb[BMEMCONFIG_MAX_DDR_SCB_STRING];
#if NEXUS_HAS_VIDEO_DECODER
    int  num_decoders;
    struct
    {
        int      enabled;
        int      color_depth;
        STRING32 str_max_resolution;
        STRING32 str_compression;
        STRING32 str_gfx_resolution;
    } videoDecoder[NEXUS_MAX_VIDEO_DECODERS];
    bool supportedCodecsSuperset[NEXUS_VideoCodec_eMax]; /* combine all codecs together for all decoders */
#endif /* if NEXUS_HAS_VIDEO_DECODER */
#if NEXUS_HAS_DISPLAY
    struct
    {
        int numVideoWindows;
        STRING32 str_max_format;
    } display[NEXUS_MAX_DISPLAYS];
#endif
#if NEXUS_HAS_VIDEO_ENCODER
    int      num_encoders;
    int      encoder_color_depth;
    STRING32 encoder_max_resolution;
    STRING32 encoder_compression;

    int      num_encoders_graphics;
    STRING32 encoder_gfx_resolution;
    struct
    {
        int      enabled;
        int      color_depth;
        int      displayIndex;
        STRING32 str_max_resolution;
        STRING32 str_compression;
        STRING32 str_gfx_resolution;
    } videoEncoder[NEXUS_MAX_VIDEO_ENCODERS];
#endif /* if NEXUS_HAS_VIDEO_ENCODER */
#if NEXUS_HAS_HDMI_INPUT
    int num_hdmi_inputs;
#endif
} Bboxreport_SvgPlot;

typedef struct
{
    STRING32 line[3];                                      /* each rectangle box has a 3-line text description */
} Bboxreport_ElementDescriptive;

int Add_BoxmodeTableHtml(
    Bboxreport_SvgPlot                *svgPlot,
    char                              *boxmodeTableHtml,
    NEXUS_MemoryConfigurationSettings *memConfigSettings
    );
int Create_SvgPlot(
    Bboxreport_SvgPlot                *svgPlot,
    NEXUS_MemoryConfigurationSettings *memConfigSettings
    );

const char * get_maxformat_name ( NEXUS_VideoFormat maxFormat );
const char *get_codec_name( int codec );
int get_codec_value( int codec );
int printCodecs( NEXUS_MemoryConfigurationSettings *memConfigSettings );
char *getVideoCodecsStr( unsigned int whichDecIdx, NEXUS_MemoryConfigurationSettings *memConfigSettings );

#endif /*__BBOXREPORT_SVGPLOT_H__ */

/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/
/* this file shall be included only from nexus_display_module.h */
#ifndef NEXUS_VIDEO_OUTPUT_IMPL_H__
#define NEXUS_VIDEO_OUTPUT_IMPL_H__

/**
This section contains internal API's for the generic NEXUS_VideoOutput connector.
**/

BDBG_OBJECT_ID_DECLARE(NEXUS_VideoOutput_P_Link);

struct NEXUS_VideoOutput_P_Link;

typedef struct NEXUS_VideoOutput_P_Iface {
    BERR_Code (*connect)(void *output,  NEXUS_DisplayHandle display);
    BERR_Code (*disconnect)(void *output,  NEXUS_DisplayHandle display);
    BERR_Code (*formatChange)(void *output, NEXUS_DisplayHandle display, NEXUS_VideoFormat format, NEXUS_DisplayAspectRatio aspectRatio, bool _3dOrientationChange);
} NEXUS_VideoOutput_P_Iface;

typedef struct NEXUS_VideoOutput_P_Link {
    BDBG_OBJECT(NEXUS_VideoOutput_P_Link)
    BLST_D_ENTRY(NEXUS_VideoOutput_P_Link) link; /* list of what's displayed */
    NEXUS_DisplayHandle display; /* display that output is connected to */
    NEXUS_VideoOutput output; /* handle of the specific video output */
    NEXUS_VideoOutput_P_Iface iface;
    bool sdOnly;
    BVDC_DisplayOutput displayOutput;
#define NEXUS_P_MAX_DACS 4
    struct {
        NEXUS_VideoDac dac; /* eNone if unused */
        BVDC_DacOutput type;
    } dac[NEXUS_P_MAX_DACS];
    NEXUS_VideoOutputSettings settings;
    bool connected;
    bool dacsConnected; /* for systems with limited DAC's, we black out the lesser output, WRN to console, but don't return a failure. */
} NEXUS_VideoOutput_P_Link;

NEXUS_VideoOutput_P_Link *
NEXUS_P_VideoOutput_Link(NEXUS_VideoOutput output);

NEXUS_VideoOutput_P_Link *
NEXUS_VideoOutput_P_CreateLink(NEXUS_VideoOutput output, const NEXUS_VideoOutput_P_Iface *iface, bool sdOnly);

void
NEXUS_VideoOutput_P_DestroyLink(NEXUS_VideoOutput_P_Link *);

NEXUS_DisplayHandle
NEXUS_VideoOutput_P_GetDisplay(NEXUS_VideoOutput output);

/**
This section contains internal API's for specific video outputs.
**/

struct NEXUS_ComponentOutput {
    NEXUS_OBJECT(NEXUS_ComponentOutput);
    NEXUS_ComponentOutputSettings cfg;
    bool opened;
    NEXUS_VideoOutputObject output;
    unsigned index;
};

struct NEXUS_CompositeOutput {
    NEXUS_OBJECT(NEXUS_CompositeOutput);
    NEXUS_CompositeOutputSettings cfg;
    bool opened;
    NEXUS_VideoOutputObject output;
    BVDC_DacOutput dacOutputType; /* only used for 3548/3556 for AnalogVideoDecoder bypass */
};

struct NEXUS_SvideoOutput {
    NEXUS_OBJECT(NEXUS_SvideoOutput);
    NEXUS_SvideoOutputSettings cfg;
    bool opened;
    NEXUS_VideoOutputObject output;
};

struct NEXUS_Ccir656Output {
    NEXUS_OBJECT(NEXUS_Ccir656Output);
    NEXUS_Ccir656OutputSettings cfg;
    bool opened;
    NEXUS_VideoOutputObject output;
};

void NEXUS_VideoOutputs_P_Init(void);
void NEXUS_VideoOutput_P_PostSetHdmiFormat(void);

/* TODO: setHdrSettings only works for a single window, need to make more like cap update below to handle pip/multipip */
void NEXUS_VideoOutput_P_SetHdrSettings(
    void *output, const NEXUS_HdmiDynamicRangeMasteringInfoFrame *drmInfoFrame);
NEXUS_Error NEXUS_VideoOutput_P_UpdateDisplayDynamicRangeProcessingCapabilities(NEXUS_DisplayHandle display);


NEXUS_Error nexus_p_bypass_video_output_connect(NEXUS_VideoOutput_P_Link *link, const NEXUS_DisplaySettings *pSettings, NEXUS_VideoFormat hdmiOutputFormat);
NEXUS_Error nexus_videooutput_p_connect(NEXUS_VideoOutput_P_Link *link);
void nexus_videooutput_p_disconnect(NEXUS_VideoOutput_P_Link *link);

#if NEXUS_NUM_HDMI_OUTPUTS
void NEXUS_VideoOutput_P_SetHdmiSettings(void *context);
#endif

#endif /* NEXUS_VIDEO_OUTPUT_IMPL_H__ */



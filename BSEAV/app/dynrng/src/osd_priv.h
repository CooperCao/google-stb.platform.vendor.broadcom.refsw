/******************************************************************************
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
 *****************************************************************************/
#ifndef OSD_PRIV_H__
#define OSD_PRIV_H__ 1

#include "platform_types.h"
#include "bwt.h"
#include "osd.h"
#include <pthread.h>

#define OSD_PADDING 70 /* ppt */
#define PANEL_PADDING 14 /* ppt */

typedef struct OsdPictureInfoView
{
    BWT_LabelHandle format;
    BWT_LabelHandle dynrng;
    BWT_LabelHandle gamut;
    BWT_LabelHandle space;
    BWT_LabelHandle depth;
    BWT_LabelHandle processing;
} OsdPictureInfoView;

typedef struct OsdInfoPanel
{
    BWT_PanelHandle base;
    BWT_TableHandle table;
    OsdPictureInfoView vid[MAX_MOSAICS];
    OsdPictureInfoView gfx;
    OsdPictureInfoView sel;
    OsdPictureInfoView out;
    OsdPictureInfoView rcv;
} OsdInfoPanel;

typedef struct Osd
{
    OsdCreateSettings createSettings;
    PlatformListenerHandle renderer;
    BWT_ToolkitHandle bwt;
    PlatformUsageMode usageMode;
    unsigned layout;
    BWT_ImageHandle thumbnail;
    BWT_ImageHandle background;
    unsigned mosaicCount;
    BWT_VideoWindowHandle window[MAX_MOSAICS];
    OsdInfoPanel info;
    OsdInfoPanel mosaicInfo;
    OsdInfoPanel pipInfo;
    OsdInfoPanel *current;
    PlatformModel model;
    BWT_PanelHandle main;
    BWT_PanelHandle video;
    BWT_PanelHandle pip;
    pthread_mutex_t lock;
} Osd;

BWT_PanelHandle osd_p_create_main_panel(OsdHandle osd);
BWT_PanelHandle osd_p_create_info_panel(OsdHandle osd, OsdInfoPanel *infoPanel, unsigned textHeight, unsigned numVideos);

void osd_label_p_update_dynamic_range(BWT_LabelHandle label, PlatformDynamicRange eotf);
void osd_label_p_update_processing(BWT_LabelHandle label, PlatformTriState plm);
void osd_p_scheduler_callback(void * pContext, int param);

#endif /* OSD_PRIV_H__ */

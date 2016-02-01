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
******************************************************************************/

#include "nexus_platform.h"
#include "nexus_types.h"
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "bstd.h"
#include "bkni.h"
#include "dynrng_osd.h"

#ifndef DYNRNG_OSD_PRIV_H__
#define DYNRNG_OSD_PRIV_H__

typedef struct OSD_Screen * OSD_ScreenHandle;
typedef struct OSD_Icon * OSD_IconHandle;
typedef struct OSD_Image * OSD_ImageHandle;

/* GIMP format */
struct OSD_Image
{
    unsigned int width;
    unsigned int height;
    unsigned int bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
    char * pixel_data;
};

struct OSD_Icon
{
    OSD_OsdHandle osd;
    NEXUS_SurfaceHandle surface;
    NEXUS_Rect rect;
};

struct OSD_Screen
{
    /* type icons */
    OSD_IconHandle tv;
    OSD_IconHandle output;
    OSD_IconHandle input;

    NEXUS_SurfaceHandle surface;
    NEXUS_Rect rect;
};

struct OSD_Osd
{
    OSD_OsdSettings settings;
    OSD_OsdModel model;
    BKNI_EventHandle checkpointEvent;
    NEXUS_Graphics2DHandle graphics;
    OSD_ScreenHandle screen;
    /* state icons */
    OSD_IconHandle arib;
    OSD_IconHandle smpte;
    OSD_IconHandle hdr;
    OSD_IconHandle sdr;
    OSD_IconHandle unknown;
    OSD_IconHandle yes;
    OSD_IconHandle no;
};

typedef enum OSD_ImageId
{
    OSD_ImageId_eArib,
    OSD_ImageId_eSmpte,
    OSD_ImageId_eHdr,
    OSD_ImageId_eSdr,
    OSD_ImageId_eUnknown,
    OSD_ImageId_eYes,
    OSD_ImageId_eNo,
    OSD_ImageId_eInput,
    OSD_ImageId_eOutput,
    OSD_ImageId_eTv,
    OSD_ImageId_eMax
} OSD_ImageId;

OSD_ImageHandle OSD_GetImageById(OSD_ImageId id);
void OSD_DestroyScreen(OSD_ScreenHandle screen);
OSD_ScreenHandle OSD_CreateScreen(unsigned width, unsigned height, unsigned statusHeight);
void OSD_DestroyIcon(OSD_IconHandle icon);
OSD_IconHandle OSD_CreateIcon(OSD_ImageId imageId);
int OSD_DrawIcon(OSD_OsdHandle osd, OSD_IconHandle icon, int x, int y);
int OSD_DrawBlank(OSD_OsdHandle osd, OSD_ScreenHandle screen);
int OSD_PopulateEotfIcon(OSD_OsdHandle osd, OSD_IconHandle icon, OSD_Eotf eotf);
int OSD_PopulateEotfSupportIcon(OSD_OsdHandle osd, OSD_IconHandle icon, OSD_EotfSupport support);
int OSD_DrawScreen(OSD_OsdHandle osd, OSD_ScreenHandle screen);
int OSD_Draw(OSD_OsdHandle osd);
void OSD_MoveIcon(OSD_IconHandle icon, int x, int y);
void OSD_PrintModel(const OSD_OsdModel * pModel);
const char * OSD_GetEotfName(OSD_Eotf eotf);
const char * OSD_GetEotfSupportName(OSD_EotfSupport support);
void OSD_PrintIcon(const char * tag, OSD_IconHandle icon);
void OSD_PrintScreen(const char * tag, OSD_ScreenHandle screen);

#endif /* DYNRNG_OSD_H__ */

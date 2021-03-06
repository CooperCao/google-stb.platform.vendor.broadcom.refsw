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
 *
 * Module Description:
 *
******************************************************************************/

#include "nexus_platform.h"
#include "nexus_surface.h"

#ifndef DYNRNG_OSD_H__
#define DYNRNG_OSD_H__

typedef struct OSD_Osd * OSD_OsdHandle;

typedef enum OSD_OsdMode
{
    OSD_OsdMode_eOff,
    OSD_OsdMode_eOn,
    OSD_OsdMode_eTimer,
    OSD_OsdMode_eMax
} OSD_OsdMode;

typedef struct OSD_OsdSettings
{
    OSD_OsdMode mode;
    unsigned timeout;
} OSD_OsdSettings;

typedef enum OSD_Eotf
{
    OSD_Eotf_eUnknown,
    OSD_Eotf_eSdr,
    OSD_Eotf_eHdr,
    OSD_Eotf_eSmpte,
    OSD_Eotf_eArib,
    OSD_Eotf_eMax
} OSD_Eotf;

typedef enum OSD_EotfSupport
{
    OSD_EotfSupport_eUnknown,
    OSD_EotfSupport_eNo,
    OSD_EotfSupport_eYes,
    OSD_EotfSupport_eMax
} OSD_EotfSupport;

typedef struct OSD_OsdModel
{
    OSD_Eotf input;
    OSD_Eotf output;
    OSD_EotfSupport tv;
} OSD_OsdModel;

void OSD_Destroy(OSD_OsdHandle osd);
OSD_OsdHandle OSD_Create(unsigned width, unsigned height);
NEXUS_SurfaceHandle OSD_GetFrameBuffer(OSD_OsdHandle osd);
const NEXUS_Rect * OSD_GetFrameBufferDimensions(OSD_OsdHandle osd);
void OSD_GetSettings(OSD_OsdHandle osd, OSD_OsdSettings * pSettings);
int OSD_SetSettings(OSD_OsdHandle osd, const OSD_OsdSettings * pSettings);
void OSD_GetModel(OSD_OsdHandle osd, OSD_OsdModel * model);
int OSD_SetModel(OSD_OsdHandle osd, const OSD_OsdModel * model);
void OSD_Print(OSD_OsdHandle osd);
OSD_OsdMode OSD_ParseOsdMode(const char * osdStr);
const char * OSD_GetModeName(OSD_OsdMode mode);

#endif /* DYNRNG_OSD_H__ */

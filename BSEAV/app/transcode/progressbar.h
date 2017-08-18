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
 ***************************************************************************/

/* this is a pseudo-library to display a progress-bar.
   it's a simple wrapper around NEXUS_Graphics2D_Fill and related functions.
   the function prototypes and definitions are in one file to keep things simple.
*/

#include "nexus_platform.h"
#include "nexus_surface.h"
#if NEXUS_HAS_GRAPHICS2D
#include "nexus_graphics2d.h"
#endif

/*
Public API
*/

typedef struct ProgressBarSettings {
    NEXUS_Graphics2DHandle gfx;
    NEXUS_SurfaceHandle surface;
    NEXUS_Rect rect;        /* dimensions of progress bar */
    uint16_t maxProgress;   /* max value of progress. set it equal to rect.width to work in units of pixels. set it equal to 100 to work in units of percentages */
    NEXUS_Pixel resetColor; /* default color to empty progress bar with */
    NEXUS_Pixel fillColor;  /* default color to fill progress bar with */
} ProgressBarSettings;

struct ProgressBar {
    ProgressBarSettings settings;
    BKNI_EventHandle checkpointEvent, spaceAvailableEvent;
    bool internalCheckpointCb;
    bool internalSpaceAvailableCb;
};

typedef struct ProgressBar *ProgressBarHandle;

ProgressBarHandle ProgressBar_Open(const ProgressBarSettings* pSettings);
void ProgressBar_Close(ProgressBarHandle handle);
void ProgressBar_Reset(ProgressBarHandle handle); /* empty out the progress bar */
void ProgressBar_FillSimple(ProgressBarHandle handle, uint16_t end); /* fill progress bar upto (end / settings.maxProgress) */
void ProgressBar_FillSegment(ProgressBarHandle handle, uint16_t start, uint16_t end); /* fill progress bar segment */
void ProgressBar_FillSegmentWithColor(ProgressBarHandle handle, uint16_t start, uint16_t end, NEXUS_Pixel color); /* fill progress bar segment and override settings.fillcolor */

/*
End of public API
*/

/**************************************************************************/

void ProgressBar_P_Callback(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

ProgressBarHandle ProgressBar_Open(const ProgressBarSettings* pSettings)
{
    ProgressBarHandle pPb = NULL;
    NEXUS_Graphics2DSettings gfxSettings;
    BERR_Code rc;
    pPb = BKNI_Malloc(sizeof(*pPb));
    if (pPb == NULL) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }

    if (pSettings->gfx==NULL) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto error;
    }

    /* hijack the callbacks */
    NEXUS_Graphics2D_GetSettings(pSettings->gfx, &gfxSettings);
    if(!gfxSettings.checkpointCallback.callback) {
        rc = BKNI_CreateEvent(&pPb->checkpointEvent);
        if (rc != BERR_SUCCESS) {
            rc = BERR_TRACE(BERR_UNKNOWN);
            goto error;
        }

        gfxSettings.checkpointCallback.callback = ProgressBar_P_Callback;
        gfxSettings.checkpointCallback.context = pPb->checkpointEvent;
        pPb->internalCheckpointCb = true;
    }
    else {
        pPb->checkpointEvent = gfxSettings.checkpointCallback.context;
        pPb->internalCheckpointCb = false;
    }

    if(!gfxSettings.packetSpaceAvailable.callback) {
        rc = BKNI_CreateEvent(&pPb->spaceAvailableEvent);
        if (rc != BERR_SUCCESS) {
            rc = BERR_TRACE(BERR_UNKNOWN);
            goto error;
        }

        gfxSettings.packetSpaceAvailable.callback = ProgressBar_P_Callback;
        gfxSettings.packetSpaceAvailable.context = pPb->spaceAvailableEvent;
        pPb->internalSpaceAvailableCb = true;
    }
    else {
        pPb->spaceAvailableEvent = gfxSettings.packetSpaceAvailable.context;
        pPb->internalSpaceAvailableCb = false;
    }

    if(pPb->internalCheckpointCb || pPb->internalSpaceAvailableCb) {
        NEXUS_Graphics2D_SetSettings(pSettings->gfx, &gfxSettings);
    }

    pPb->settings = *pSettings;
    return pPb;

error:
    if (pPb) {
        ProgressBar_Close(pPb);
    }
    return NULL;
}

void ProgressBar_Close(ProgressBarHandle handle)
{
    if (handle) {
        if (handle->checkpointEvent && handle->internalCheckpointCb) {
            BKNI_DestroyEvent(handle->checkpointEvent);
        }
        if (handle->spaceAvailableEvent && handle->internalSpaceAvailableCb) {
            BKNI_DestroyEvent(handle->spaceAvailableEvent);
        }
        BKNI_Free(handle);
    }
}

void ProgressBar_P_Fill(ProgressBarHandle handle, NEXUS_Graphics2DFillSettings *pFillSettings)
{
    NEXUS_Error rc;
    ProgressBarSettings *settings = &handle->settings;

    while (1) {
        rc = NEXUS_Graphics2D_Fill(settings->gfx, pFillSettings);
        if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL) {
            BKNI_WaitForEvent(handle->spaceAvailableEvent, BKNI_INFINITE);
        }
        else {
            break;
        }
    }

    rc = NEXUS_Graphics2D_Checkpoint(settings->gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(handle->checkpointEvent, BKNI_INFINITE);
    }
}

void ProgressBar_Reset(ProgressBarHandle handle)
{
    NEXUS_Graphics2DFillSettings fillSettings;
    ProgressBarSettings *settings = &handle->settings;

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = settings->surface;
    BKNI_Memcpy(&fillSettings.rect, &settings->rect, sizeof(fillSettings.rect));
    fillSettings.colorOp = NEXUS_FillOp_eCopy;
    fillSettings.alphaOp = NEXUS_FillOp_eCopy;
    fillSettings.color = settings->resetColor;
    ProgressBar_P_Fill(handle, &fillSettings);
}

void ProgressBar_FillSimple(ProgressBarHandle handle, uint16_t end)
{
    ProgressBar_FillSegmentWithColor(handle, 0, end, handle->settings.fillColor);
}

void ProgressBar_FillSegment(ProgressBarHandle handle, uint16_t start, uint16_t end)
{
    ProgressBar_FillSegmentWithColor(handle, start, end, handle->settings.fillColor);
}

void ProgressBar_FillSegmentWithColor(ProgressBarHandle handle, uint16_t start, uint16_t end, NEXUS_Pixel color)
{
    NEXUS_Graphics2DFillSettings fillSettings;
    ProgressBarSettings *settings = &handle->settings;

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = settings->surface;
    fillSettings.rect.x = settings->rect.x + ((start * settings->rect.width) / settings->maxProgress);
    fillSettings.rect.width = ((end-start) * settings->rect.width) / settings->maxProgress;
    fillSettings.rect.y = settings->rect.y;
    fillSettings.rect.height = settings->rect.height;
    fillSettings.colorOp = NEXUS_FillOp_eCopy;
    fillSettings.alphaOp = NEXUS_FillOp_eCopy;
    fillSettings.color = color;
    #if 0
    BKNI_Printf("(%u, %u), %u x %u (%u)\n", fillSettings.rect.x, fillSettings.rect.y, fillSettings.rect.width, fillSettings.rect.height,settings->rect.height);
    #endif
    ProgressBar_P_Fill(handle, &fillSettings);
}

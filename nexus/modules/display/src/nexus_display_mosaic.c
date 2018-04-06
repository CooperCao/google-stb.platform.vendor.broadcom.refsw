/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *
 ***************************************************************************/
#include "nexus_display_module.h"

BDBG_MODULE(nexus_display_mosaic);
BDBG_FILE_MODULE(nexus_flow_video_window);

NEXUS_VideoWindowHandle NEXUS_VideoWindow_OpenMosaic( NEXUS_VideoWindowHandle parentWindow, unsigned mosaicIndex)
{
#if NEXUS_NUM_MOSAIC_DECODES
    NEXUS_VideoWindowHandle window;

    BDBG_OBJECT_ASSERT(parentWindow, NEXUS_VideoWindow);

    if (parentWindow->mosaic.parent) {
        BDBG_ERR(("Window %p is a mosaic child and so can't be a mosaic parent", (void *)parentWindow));
        return NULL;
    }
    if (mosaicIndex >= NEXUS_NUM_MOSAIC_DECODES) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }
    for (window = BLST_S_FIRST(&parentWindow->mosaic.children); window; window = BLST_S_NEXT(window, mosaic.link)) {
        if (window->mosaic.userIndex == mosaicIndex) {
            BDBG_ERR(("Window %p mosaic %u already opened", (void *)parentWindow, mosaicIndex));
            return NULL;
        }
    }

    window = BKNI_Malloc(sizeof(*window));
    if (!window) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_VideoWindow_P_InitState(window, parentWindow->index, mosaicIndex, parentWindow->display);
    window->mosaic.parent = parentWindow;
    window->mosaic.userIndex = mosaicIndex;

    /* Insert in mosaicIndex order
    This is convenient for NEXUS_VideoWindow_P_CutBackendMosaic_isr where there is no VideoDecoder order, only window order. */
    BKNI_EnterCriticalSection();
    {
        NEXUS_VideoWindowHandle mosaicWindow, prev = NULL;
        for (mosaicWindow = BLST_S_FIRST(&parentWindow->mosaic.children); mosaicWindow; mosaicWindow = BLST_S_NEXT(mosaicWindow, mosaic.link)) {
            if (mosaicWindow->mosaic.userIndex > mosaicIndex) {
                if (prev) {
                    BLST_S_INSERT_AFTER(&parentWindow->mosaic.children, prev, window, mosaic.link);
                }
                else {
                    BLST_S_INSERT_HEAD(&parentWindow->mosaic.children, window, mosaic.link);
                }
                break;
            }
            prev = mosaicWindow;
        }
        if (!mosaicWindow) {
            if (prev) {
                BLST_S_INSERT_AFTER(&parentWindow->mosaic.children, prev, window, mosaic.link);
            }
            else {
                BLST_S_INSERT_HEAD(&parentWindow->mosaic.children, window, mosaic.link);
            }
        }
    }
    BKNI_LeaveCriticalSection();

    /* default to a tile layout based on mosaicIndex */
    window->cfg.position.x = 50 + 130 * (mosaicIndex%4);
    window->cfg.position.y = 50 + 130 * (mosaicIndex/4);
    window->cfg.position.width = 100;
    window->cfg.position.height = 100;
    BDBG_MODULE_MSG(nexus_flow_video_window, ("open mosaic %p, display %d, parent %d, mosaic %d", (void *)window, window->display->index, window->mosaic.parent->index, window->index));

    return window;
#else
    BSTD_UNUSED(parentWindow);
    BSTD_UNUSED(mosaicIndex);
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return NULL;
#endif
}

/*
Summary:
Returns bits 'e'..'b' from word 'w',

Example:
    B_GET_BITS(0xDE,7,4)==0x0D
    B_GET_BITS(0xDE,3,0)=0x0E
*/
#define B_GET_BITS(w,e,b)  (((w)>>(b))&(((unsigned)(-1))>>((sizeof(unsigned))*8-(e+1-b))))

#define pVideo (&g_NEXUS_DisplayModule_State)

NEXUS_Error NEXUS_VideoWindow_P_ApplyMosaic(NEXUS_VideoWindowHandle window)
{
#if NEXUS_NUM_MOSAIC_DECODES
    BERR_Code rc;
    NEXUS_VideoWindowHandle mosaicWindow;
    BVDC_Window_Handle windowVdc;
    unsigned cnt = 0;
    unsigned total = 0;
    bool mosaic_visible[NEXUS_NUM_MOSAIC_DECODES];
    BVDC_Rect mosaic_rects[NEXUS_NUM_MOSAIC_DECODES];
    uint8_t mosaic_zorder[NEXUS_NUM_MOSAIC_DECODES];
    unsigned mosaicTrackIndex = NEXUS_NUM_MOSAIC_DECODES;
    NEXUS_VideoInput_P_Link *link;
    BAVC_MFD_Picture *pPrevPicture = NULL;
    unsigned minzorder = UINT32_MAX;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    BDBG_ASSERT(!window->mosaic.parent);

    windowVdc = window->vdcState.window;
    if (!windowVdc) return 0;

    BKNI_Memset(mosaic_visible, 0, sizeof(mosaic_visible));

    for (mosaicWindow = BLST_S_FIRST(&window->mosaic.children); mosaicWindow; mosaicWindow = BLST_S_NEXT(mosaicWindow, mosaic.link)) {
        unsigned index;

        if (!mosaicWindow->input) continue;

        link = NEXUS_VideoInput_P_Get(mosaicWindow->input);
        BDBG_ASSERT(link);

        if (mosaicWindow->mosaic.mosaicSettings.backendMosaic.enabled) {
            index = mosaicWindow->mosaic.userIndex;

            /* build a linked list of BAVC_MFD_Picture structs for backend mosaic */
            BKNI_EnterCriticalSection();
            if (pPrevPicture) {
                pPrevPicture->pNext = &mosaicWindow->mosaic.picture;
            }
            pPrevPicture = &mosaicWindow->mosaic.picture;
            pPrevPicture->pNext = NULL;
            BKNI_LeaveCriticalSection();
        }
        else {
            index = link->mosaic.index;
        }

        mosaic_visible[index] = mosaicWindow->cfg.visible;
        mosaic_rects[index].lLeft = mosaicWindow->cfg.position.x;
        mosaic_rects[index].lTop = mosaicWindow->cfg.position.y;
        mosaic_rects[index].ulWidth = mosaicWindow->cfg.position.width;
        mosaic_rects[index].ulHeight = mosaicWindow->cfg.position.height;
        if ( mosaicWindow->cfg.zorder > 255 ) {
            BDBG_WRN(("mosaicWindow->cfg.zorder(%u) > 255", mosaicWindow->cfg.zorder));
        }
        mosaic_zorder[index] = mosaicWindow->cfg.zorder;
        if (mosaicWindow->cfg.zorder < minzorder) {
            minzorder = mosaicWindow->cfg.zorder;
        }
        BDBG_MSG(("mosaic %d: v=%c z=%d position=%d,%d,%d,%d", index,
            mosaicWindow->cfg.visible?'y':'n',
            mosaicWindow->cfg.zorder,
            mosaicWindow->cfg.position.x,mosaicWindow->cfg.position.y,mosaicWindow->cfg.position.width,mosaicWindow->cfg.position.height));

        if (window->display->cfg.frameRateMaster) {
            if (window->display->cfg.frameRateMaster == mosaicWindow->input) {
                mosaicTrackIndex = index;
            }
        }
        else if (mosaicWindow->cfg.autoMaster) {
            mosaicTrackIndex = index;
        }

        if (index+1 > cnt) {
            cnt = index+1;
        }
        total++;
    }

    if (!total) {
        BDBG_MSG(("no mosaics"));
        rc = BVDC_Window_SetMosaicConfiguration(windowVdc, false, NULL);
        if (rc) {return BERR_TRACE(rc);}
    }
    else {
        bool dummy;
        NEXUS_DisplayHandle display = window->display;
        BVDC_MosaicConfiguration mosaic_config;
        unsigned i;

        /* set the zorder of the parent to 0 or 1 based on min zorder of all mosaics */
        rc = BVDC_Window_SetZOrder(windowVdc, minzorder ? 1 : 0);
        if (rc) {return BERR_TRACE(rc);}

         /* VDC does not ignore invisible windows, so we must ensure ok values */
         for (i=0;i<cnt;i++) {
             if (!mosaic_visible[i]) {
                 mosaic_rects[i].lLeft = mosaic_rects[i].lTop = 0;
                 mosaic_rects[i].ulWidth = mosaic_rects[i].ulHeight = 100;
                 mosaic_zorder[i]=0;
             }
         }


        BDBG_MSG(("%d mosaics, array size %d", total, cnt));

        BVDC_Window_GetMosaicConfiguration(windowVdc, &dummy, &mosaic_config);
        mosaic_config.bVideoInMosaics = true;
        mosaic_config.bClearRectByMaskColor = true;
        mosaic_config.ulClearRectAlpha = 0;
        mosaic_config.ulMaskColorBlue = B_GET_BITS(display->cfg.background, 7, 0);
        mosaic_config.ulMaskColorGreen = B_GET_BITS(display->cfg.background, 15, 8);
        mosaic_config.ulMaskColorRed = B_GET_BITS(display->cfg.background, 23, 16);
        rc = BVDC_Window_SetMosaicConfiguration(windowVdc, true, &mosaic_config);
        if (rc) {return BERR_TRACE(rc);}

        rc = BVDC_Window_SetMosaicDstRects(windowVdc, cnt, mosaic_rects);
        if (rc) {return BERR_TRACE(rc);}
        rc = BVDC_Window_SetMosaicRectsVisibility(windowVdc, cnt, mosaic_visible);
        if (rc) {return BERR_TRACE(rc);}
        rc = BVDC_Window_SetMosaicRectsZOrder(windowVdc, cnt, mosaic_zorder);
        if (rc) {return BERR_TRACE(rc);}
        if (mosaicTrackIndex != NEXUS_NUM_MOSAIC_DECODES) {
            rc = BVDC_Window_SetMosaicTrack(windowVdc, mosaicTrackIndex);
            if (rc) {return BERR_TRACE(rc);}
        }
    }

    rc = NEXUS_Display_P_ApplyChanges();
    if (rc) {return BERR_TRACE(rc);}

    return 0;
#else
    BSTD_UNUSED(window);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

void NEXUS_VideoWindow_GetMosaicSettings( NEXUS_VideoWindowHandle window, NEXUS_VideoWindowMosaicSettings *pSettings )
{
    NEXUS_VideoWindow_GetMosaicSettings_priv(window, pSettings);
}

NEXUS_Error NEXUS_VideoWindow_SetMosaicSettings( NEXUS_VideoWindowHandle window, const NEXUS_VideoWindowMosaicSettings *pSettings )
{
    return NEXUS_VideoWindow_SetMosaicSettings_priv(window, pSettings);
}

void NEXUS_VideoWindow_GetMosaicSettings_priv( NEXUS_VideoWindowHandle window, NEXUS_VideoWindowMosaicSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
#if NEXUS_NUM_MOSAIC_DECODES
    *pSettings = window->mosaic.mosaicSettings;
#else
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
#endif
}

NEXUS_Error NEXUS_VideoWindow_SetMosaicSettings_priv( NEXUS_VideoWindowHandle window, const NEXUS_VideoWindowMosaicSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
#if NEXUS_NUM_MOSAIC_DECODES
    BKNI_EnterCriticalSection();
    window->mosaic.mosaicSettings = *pSettings;
    BKNI_LeaveCriticalSection();
    return 0;
#else
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

#if NEXUS_NUM_MOSAIC_DECODES
const BAVC_MFD_Picture *NEXUS_VideoWindow_P_CutBackendMosaic_isr(NEXUS_VideoWindowHandle window, const BAVC_MFD_Picture *pPicture)
{
    unsigned ulStripeWidth;
    NEXUS_VideoWindowHandle mosaicWindow;
    const BAVC_MFD_Picture *pFirstPicture = NULL;
    unsigned id = 0;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);

    ulStripeWidth = NEXUS_P_StripeWidth_FromMagnum_isrsafe(pPicture->eStripeWidth);

    for (mosaicWindow = BLST_S_FIRST(&window->mosaic.children); mosaicWindow; mosaicWindow = BLST_S_NEXT(mosaicWindow, mosaic.link)) {
        BAVC_MFD_Picture *pThis, *pNext;
        unsigned ulStripeNum;
        NEXUS_VideoWindowMosaicSettings *pMosaicSettings = &mosaicWindow->mosaic.mosaicSettings;
        unsigned x, y, xByte;

        if (!pMosaicSettings->backendMosaic.enabled) continue;

        pThis = &mosaicWindow->mosaic.picture;
        if (!pFirstPicture) {
            pFirstPicture = pThis;
        }

        pNext = pThis->pNext;
        *pThis = *pPicture; /* overwrite everything */
        pThis->pNext = pNext; /* restore the pNext pointer */
        pThis->ulChannelId = id++;

        /* avoid div-by-zero math */
        if (!pMosaicSettings->backendMosaic.clipBase.width || !pMosaicSettings->backendMosaic.clipBase.height) {
            continue;
        }

        x = pMosaicSettings->backendMosaic.clipRect.x * pThis->ulSourceHorizontalSize / pMosaicSettings->backendMosaic.clipBase.width;
        y = pMosaicSettings->backendMosaic.clipRect.y * pThis->ulSourceVerticalSize / pMosaicSettings->backendMosaic.clipBase.height;

        pThis->ulSourceHorizontalSize = pThis->ulSourceHorizontalSize * pMosaicSettings->backendMosaic.clipRect.width / pMosaicSettings->backendMosaic.clipBase.width;
        pThis->ulDisplayHorizontalSize = pThis->ulDisplayHorizontalSize * pMosaicSettings->backendMosaic.clipRect.width / pMosaicSettings->backendMosaic.clipBase.width;
        pThis->ulSourceVerticalSize = pThis->ulSourceVerticalSize * pMosaicSettings->backendMosaic.clipRect.height / pMosaicSettings->backendMosaic.clipBase.height;
        pThis->ulDisplayVerticalSize = pThis->ulDisplayVerticalSize * pMosaicSettings->backendMosaic.clipRect.height / pMosaicSettings->backendMosaic.clipBase.height;

        xByte = (pThis->eBitDepth == BAVC_VideoBitDepth_e10Bit) ? ((4*x)/3) & ~7 : x;
        ulStripeNum = xByte / ulStripeWidth;

        pThis->ulLuminanceFrameBufferBlockOffset +=
             ulStripeNum * pThis->ulLuminanceNMBY * 16 * ulStripeWidth +
             y * ulStripeWidth +
             (xByte % ulStripeWidth);

        pThis->ulChrominanceFrameBufferBlockOffset +=
             /* stripe offset */
             ulStripeNum * pThis->ulChrominanceNMBY * 16 * ulStripeWidth +

            /* vertical offset within a stripe (take care of 422 vs 420 chroma type) */
             ((y * ulStripeWidth /2) << (pThis->eYCbCrType - BAVC_YCbCrType_e4_2_0)) +

            /* horizontal offset within a stripe */
             (xByte % ulStripeWidth);
    }

    return pFirstPicture ? pFirstPicture : pPicture;
}

/**
Need to find parent mosaic NEXUS_VideoInput which matches the child mosaic mosaicInput.
We must match by going back to the VideoDecoder mosaic parent.
Be aware that a single parent mosaic NEXUS_VideoInput will serve multiple mosaic windows on multiple displays.
**/
static NEXUS_VideoInput NEXUS_VideoWindow_P_AssignMosaicInput(NEXUS_VideoInput mosaicInput)
{
#if NEXUS_HAS_VIDEO_DECODER
    NEXUS_VideoInput input = NULL;
    unsigned freeInput = NEXUS_NUM_MOSAIC_DECODE_SETS;
    NEXUS_VideoDecoderDisplayConnection connection;
    unsigned i;

    if (!pVideo->modules.videoDecoder) {
        BERR_TRACE(NEXUS_NOT_SUPPORTED);
        return NULL;
    }

    NEXUS_Module_Lock(pVideo->modules.videoDecoder);
    NEXUS_VideoDecoder_GetDisplayConnection_priv(mosaicInput->source, &connection);
    NEXUS_Module_Unlock(pVideo->modules.videoDecoder);

    for (i=0;i<NEXUS_NUM_MOSAIC_DECODE_SETS;i++) {
        if (pVideo->mosaicInput[i].input.source) {
            if (connection.parentIndex == pVideo->mosaicInput[i].parentIndex) {
                input = &pVideo->mosaicInput[i].input;
                break;
            }
        }
        else if (freeInput == NEXUS_NUM_MOSAIC_DECODE_SETS) {
            freeInput = i;
        }
    }

    if (!input && freeInput < NEXUS_NUM_MOSAIC_DECODE_SETS) {
        input = &pVideo->mosaicInput[freeInput].input;
        input->source = mosaicInput->source; /* this is the link */
        pVideo->mosaicInput[freeInput].parentIndex = connection.parentIndex; /* cache this value */
    }

    return input;
#else
    BSTD_UNUSED(mosaicInput);
    return NULL;
#endif
}

NEXUS_Error NEXUS_VideoWindow_P_AddMosaicInput(NEXUS_VideoWindowHandle window, NEXUS_VideoInput input)
{
    NEXUS_Error rc;
    NEXUS_VideoInput parentVideoInput;
    NEXUS_VideoInput_P_Link *parentLink;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);

    /* Mosaic windows share a common parent VideoWindow and a common parent NEXUS_VideoInput_P_Link.
       There is a single NEXUS_VideoInput_P_Link for each VideoDecoder set of mosaics. One NEXUS_VideoInput_P_Link could serve multiple displays (HD/SD simul).
       The parent link is not connected back to VideoDecoder; it only holds the BVDC_Source for the Display module.
       Each mosaic window has its own, separate NEXUS_VideoInput_P_Link as well, used to connect back to each mosaic decoder and to store its mosaicIndex. */

    if (input->type != NEXUS_VideoInputType_eDecoder) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    parentVideoInput = NEXUS_VideoWindow_P_AssignMosaicInput(input);
    if (!parentVideoInput) return BERR_TRACE(NEXUS_INVALID_PARAMETER);

    if (!window->mosaic.parent->input) { /* ...parent->input is NULL if no videoInput currently connected to parent */
        NEXUS_VideoInput_P_Link *link;
        BLST_S_DICT_FIND(&pVideo->inputs, link, input, input, link);
        if (link && !input->destination) {
            BDBG_WRN(("Shutting down input %p. You must call NEXUS_VideoInput_Shutdown() before calling NEXUS_VideoWindow_AddInput() with a mosaic window", (void *)input));
            NEXUS_VideoInput_Shutdown(input);
        }

        rc = NEXUS_VideoWindow_AddInput(window->mosaic.parent, parentVideoInput);
        if (rc) {rc = BERR_TRACE(rc);goto error;}

        parentLink = parentVideoInput->destination;
        BDBG_ASSERT(parentLink);

        parentLink->mosaic.parentWindow[window->display->index] = window->mosaic.parent;

        /* The decoder is not mosaic. so only the parentLink can connect. instead, the display module
        will cut up the single stream into the desired number of mosaics in NEXUS_VideoInput_P_DecoderDataReady_isr.
        This must be set after NEXUS_VideoWindow_AddInput. */
        parentLink->mosaic.backendMosaic = window->mosaic.mosaicSettings.backendMosaic.enabled;
    }
    else {
        if (window->mosaic.parent->input != parentVideoInput) {
            BDBG_ERR(("mosaic mismatch parent window %p already has input %p, mosaic window %p maps to input %p",
                (void *)window->mosaic.parent, (void *)window->mosaic.parent->input, (void *)window, (void *)parentVideoInput));
            rc = BERR_TRACE(NEXUS_UNKNOWN);
            goto error;
        }
        parentLink = parentVideoInput->destination;
    }

    /* now create this input's link, but use the parentLink for the shared VDC source. */
    if (input->destination == NULL) {
        NEXUS_VideoInput_P_Link *link;
        BLST_S_DICT_FIND(&pVideo->inputs, link, input, input, link);
        if (!link) {
            link = NEXUS_VideoInput_P_OpenDecoder(input, parentLink, NULL);
            if (!link) {
                rc = BERR_TRACE(NEXUS_UNKNOWN);
                goto error;
            }
        }
    }

    return 0;

error:
    parentVideoInput->source = NULL; /* reverse the effect of NEXUS_VideoWindow_P_AssignMosaicInput */
    return rc;
}

void NEXUS_VideoWindow_P_RemoveMosaicInput(NEXUS_VideoWindowHandle window, NEXUS_VideoInput input)
{
    NEXUS_VideoInput parentVideoInput;
    NEXUS_VideoInput_P_Link *parentLink;

    BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
    BDBG_ASSERT(window->mosaic.parent);
    BDBG_OBJECT_ASSERT(window->mosaic.parent, NEXUS_VideoWindow);

    /* check if this source is used for the mosaic parent. if so, switch the mosaic parent to
    another mosaic's source. if there are no more mosaics, destroy the parent link. */

    parentVideoInput = window->mosaic.parent->input;
    if (!parentVideoInput) return;
    BDBG_OBJECT_ASSERT(parentVideoInput, NEXUS_VideoInput);

    parentLink = parentVideoInput->destination;
    if (!parentLink) return;
    BDBG_OBJECT_ASSERT(parentLink, NEXUS_VideoInput_P_Link);

    /* are we removing the mosaic video input which was used to connect the parent video input? if so, we must adjust the parent */
    if (parentVideoInput->source == input->source) {
        /* find another mosaic which is currently using this parentLink */
        NEXUS_VideoWindowHandle mosaicChild;
        NEXUS_VideoWindowHandle parentWindow = parentLink->mosaic.parentWindow[window->display->index];

        BDBG_OBJECT_ASSERT(parentWindow, NEXUS_VideoWindow);
        for (mosaicChild = BLST_S_FIRST(&parentWindow->mosaic.children); mosaicChild; mosaicChild = BLST_S_NEXT(mosaicChild, mosaic.link)) {
            if ((mosaicChild != window) && mosaicChild->input && mosaicChild->input->source) {
                break;
            }
        }

        if (mosaicChild) {
            BDBG_MSG(("moving internal mosaic parent input(%p) from %p to %p", (void *)parentVideoInput, (void *)window, (void *)mosaicChild));
            parentVideoInput->source = mosaicChild->input->source;
            parentLink->input_info.source = mosaicChild->input->source;
        }
        else {
            /* parentVideoInput was not assigned to another mosaic child. It is unused. */
            BDBG_MSG(("shutting down internal mosaic parent input(%p)", (void *)parentVideoInput));
            parentLink->mosaic.backendMosaic = false; /* this will allow parent to disconnect if backendMosaic was true */
            NEXUS_VideoInput_Shutdown(parentVideoInput);
            parentVideoInput->source = NULL;
        }
    }
}
#endif

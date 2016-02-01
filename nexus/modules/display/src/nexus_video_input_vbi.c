/***************************************************************************
 *     (c)2008-2015 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 **************************************************************************/
#include "nexus_base.h"
#include "nexus_display_module.h"
#include "priv/nexus_core.h"

BDBG_MODULE(nexus_video_input_vbi);

void NEXUS_VideoInput_GetVbiSettings( NEXUS_VideoInput videoInput, NEXUS_VideoInputVbiSettings *pSettings )
{
    NEXUS_VideoInput_P_Link *link = NEXUS_VideoInput_P_Get(videoInput);
    if (link) {
        *pSettings = link->vbiSettings;
    }
    else {
        BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    }
}

NEXUS_Error NEXUS_VideoInput_SetVbiSettings( NEXUS_VideoInput videoInput, const NEXUS_VideoInputVbiSettings *pSettings )
{
#if NEXUS_VBI_SUPPORT
    NEXUS_VideoInput_P_Link *link = NEXUS_VideoInput_P_Get(videoInput);

    if (!link) return BERR_TRACE(NEXUS_UNKNOWN);

    link->vbiSettings = *pSettings;
    NEXUS_IsrCallback_Set(link->vbi.wss.isrCallback, &link->vbiSettings.wssChanged);
    NEXUS_IsrCallback_Set(link->vbi.cgms.isrCallback, &link->vbiSettings.cgmsChanged);
    NEXUS_IsrCallback_Set(link->vbi.vps.isrCallback, &link->vbiSettings.vpsChanged);
    NEXUS_IsrCallback_Set(link->vbi.cc.isrCallback, &link->vbiSettings.closedCaptionDataReady);
    NEXUS_IsrCallback_Set(link->vbi.tt.isrCallback, &link->vbiSettings.teletextDataReady);
    NEXUS_IsrCallback_Set(link->vbi.gs.isrCallback, &link->vbiSettings.gemStarDataReady);
    return NEXUS_VideoInput_P_SetVbiState(videoInput);
#else
    BSTD_UNUSED(videoInput);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

#define NEXUS_VideoInput_P_FifoWptr(fifo) (&((uint8_t*)(fifo)->data)[(fifo)->elementSize*(fifo)->wptr])

#if NEXUS_VBI_SUPPORT
static void NEXUS_VideoInput_P_IncFifo_isr(NEXUS_VideoInput_P_Link_Fifo *fifo) 
{
    if (++fifo->wptr == fifo->bufferSize) {
        fifo->wptr = 0;
    }
    if (fifo->wptr == fifo->rptr) {
        BDBG_WRN(("%s overflow", fifo->name));
    }
    NEXUS_IsrCallback_Fire_isr(fifo->isrCallback);
}
#endif

#if NEXUS_VBI_SUPPORT
static void NEXUS_VideoInput_P_Fifo_Flush(NEXUS_VideoInput_P_Link_Fifo *fifo)
{
    /* because we're modifying the wptr & rptr in sync, we must do a critical section */
    BKNI_EnterCriticalSection();
    fifo->wptr = 0;
    fifo->rptr = 0;
    BKNI_LeaveCriticalSection();
    return;
}
#endif

#if NEXUS_VBI_SUPPORT
static NEXUS_Error NEXUS_VideoInput_P_ReallocFifo(unsigned desiredNum, NEXUS_VideoInput_P_Link_Fifo *fifo)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    if (desiredNum != fifo->bufferSize) {
        /* enter/leave critical section used as barrier against already running isr */
        BKNI_EnterCriticalSection();
        fifo->inactive = true;
        fifo->wptr = 0;
        fifo->rptr = 0;
        BKNI_LeaveCriticalSection();
        if (fifo->data) {
            BKNI_Free(fifo->data);
            fifo->data = NULL;
        }
        fifo->bufferSize = desiredNum;
        if (desiredNum) {
            fifo->data = BKNI_Malloc(fifo->elementSize * desiredNum);
            if (!fifo->data) {
                rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            }
        }
        /* enter/leave critical section used as barrier so '*ptr' would get updated prior to 'inactive' */
        BKNI_EnterCriticalSection();
        fifo->inactive = false;
        BKNI_LeaveCriticalSection();
    }
    return rc;
}
#endif

static void NEXUS_VideoInput_P_ReadFifo(NEXUS_VideoInput_P_Link_Fifo *fifo, void *pLines, size_t numLines, size_t *pNumLinesRead)
{
    unsigned local_wptr;
    unsigned total_read = 0;

    /* critical section used as barrier to ensure thread sees wptr update before data[] update */
    BKNI_EnterCriticalSection();
    local_wptr = fifo->wptr;
    BKNI_LeaveCriticalSection();
    while (total_read<numLines && local_wptr != fifo->rptr && fifo->data) {
        uint8_t *dest = pLines;
        uint8_t *src = fifo->data;
        dest = &dest[fifo->elementSize*total_read++];
        src = &src[fifo->elementSize*fifo->rptr++];
        BKNI_Memcpy(dest, src, fifo->elementSize);
        if (fifo->rptr == fifo->bufferSize) {
            fifo->rptr = 0;
        }
    }
    *pNumLinesRead = total_read;
}

NEXUS_Error NEXUS_VideoInput_ReadTeletext( NEXUS_VideoInput videoInput, NEXUS_TeletextLine *pLines,
    size_t numLines, size_t *pNumLinesRead )
{
#if NEXUS_VBI_SUPPORT
    NEXUS_VideoInput_P_Link *link = NEXUS_VideoInput_P_Get(videoInput);
    if (!link) return BERR_TRACE(NEXUS_UNKNOWN);
    if (numLines && !pLines) return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    NEXUS_VideoInput_P_ReadFifo(&link->vbi.tt, pLines, numLines, pNumLinesRead);
    return 0;
#else
    BSTD_UNUSED(videoInput);
    BSTD_UNUSED(pLines);
    BSTD_UNUSED(pNumLinesRead);
    return 0;
#endif
}

NEXUS_Error NEXUS_VideoInput_ReadClosedCaption( NEXUS_VideoInput videoInput, NEXUS_ClosedCaptionData *pEntries,
    size_t numEntries, size_t *pNumEntriesRead )
{
    NEXUS_VideoInput_P_Link *link = NEXUS_VideoInput_P_Get(videoInput);
    if (!link) return BERR_TRACE(NEXUS_UNKNOWN);
    if (numEntries && !pEntries) return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    NEXUS_VideoInput_P_ReadFifo(&link->vbi.cc, pEntries, numEntries, pNumEntriesRead);
    return 0;
}

NEXUS_Error NEXUS_VideoInput_ReadGemStar( NEXUS_VideoInput videoInput, NEXUS_GemStarData *pEntries,
    size_t numEntries, size_t *pNumEntriesRead )
{
    NEXUS_VideoInput_P_Link *link = NEXUS_VideoInput_P_Get(videoInput);
    if (!link) return BERR_TRACE(NEXUS_UNKNOWN);
    if (numEntries && !pEntries) return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    NEXUS_VideoInput_P_ReadFifo(&link->vbi.gs, pEntries, numEntries, pNumEntriesRead);
    return 0;
}

NEXUS_Error NEXUS_VideoInput_GetWss( NEXUS_VideoInput videoInput, uint16_t *pWssData )
{
    NEXUS_VideoInput_P_Link *link = NEXUS_VideoInput_P_Get(videoInput);

    if (!link) return BERR_TRACE(NEXUS_UNKNOWN);
    BKNI_EnterCriticalSection();
    *pWssData = link->vbi.wss.data;
    BKNI_LeaveCriticalSection();
    return 0;
}

NEXUS_Error NEXUS_VideoInput_GetCgms( NEXUS_VideoInput videoInput, uint32_t *pCgmsData )
{
    NEXUS_VideoInput_P_Link *link = NEXUS_VideoInput_P_Get(videoInput);

    if (!link) return BERR_TRACE(NEXUS_UNKNOWN);
    BKNI_EnterCriticalSection();
    *pCgmsData = link->vbi.cgms.data;
    BKNI_LeaveCriticalSection();
    return 0;
}

NEXUS_Error NEXUS_VideoInput_GetVps( NEXUS_VideoInput videoInput, NEXUS_VpsData *pVpsData )
{
    NEXUS_VideoInput_P_Link *link = NEXUS_VideoInput_P_Get(videoInput);

    if (!link) return BERR_TRACE(NEXUS_UNKNOWN);
    BKNI_EnterCriticalSection();
    *pVpsData = link->vbi.vps.data;
    BKNI_LeaveCriticalSection();
    return 0;
}

#if NEXUS_NUM_VIDEO_DECODERS
#if NEXUS_VBI_SUPPORT
static void
NEXUS_VideoInput_P_DigitalVbiData_isr(NEXUS_VideoInput videoInput, bool is608data, const NEXUS_ClosedCaptionData *pData)
{
    NEXUS_DisplayModule_State *video= &g_NEXUS_DisplayModule_State;
    if ((pData->field == 0 || pData->field == 1) && is608data) {
        BVBI_Field_Handle vbiField;
        NEXUS_Error rc;

        /* put it into a VBI field structure. this allows analog and digital CC 608 to be processed in the same code path */
        rc = BVBIlib_List_Obtain_isr(video->vbilist, &vbiField);
        if (rc!=BERR_SUCCESS) {
            BDBG_ERR(("BVBIlib_List_Obtain_isr failed, rc=%d", rc));
            return;
        }
        rc = BVBI_Field_SetCCData_isr(vbiField, pData->data[0], pData->data[1]);
        if (rc!=BERR_SUCCESS) {
            BDBG_ERR(("BVBI_Field_SetCCData_isr %#x %#x failed, rc=%d", pData->data[0], pData->data[1], rc));
            BVBIlib_List_Return_isr(video->vbilist, vbiField);
            return;
        }
        rc = BVBI_Field_SetPolarity_isr(vbiField, (pData->field==0) ? (1<<BAVC_Polarity_eTopField) : (1<<BAVC_Polarity_eBotField));
        if (rc!=BERR_SUCCESS) {
            BDBG_ERR(("BVBI_Field_SetPolarity_isr field=%d, failed, rc=%d", pData->field, rc));
            BVBIlib_List_Return_isr(video->vbilist, vbiField);
            return;
        }

        NEXUS_VideoInput_P_VbiData_isr(videoInput, &vbiField, pData, 1);
    }
    else {
        /* this is CC708 data; therefore no encoding. */

        NEXUS_VideoInput_P_Link *link = videoInput->destination;

        if(!link) {
            BDBG_MSG(("708 VBI data from unknown input"));
            return;
        }

        BDBG_OBJECT_ASSERT(link, NEXUS_VideoInput_P_Link);

        if(!link->vbi.cc.inactive) {
            if (!link->vbi.cc.data) {
                BDBG_WRN(("No ClosedCaption buffer"));
            } else if (link->vbiSettings.closedCaptionEnabled) {
                NEXUS_ClosedCaptionData *pCc = (NEXUS_ClosedCaptionData *)NEXUS_VideoInput_P_FifoWptr(&link->vbi.cc);
                *pCc = *pData;
                NEXUS_VideoInput_P_IncFifo_isr(&link->vbi.cc);
            }
        }
    }
}
#endif
#endif

#if NEXUS_VBI_SUPPORT
/* copy the BVBI_Field, except for CC */
static NEXUS_Error nexus_videoinput_p_dup_vbifield_isr(NEXUS_VideoInput_P_Link *link, NEXUS_DisplayHandle display, BVBI_Field_Handle vbiField, BVBI_Field_Handle *pNewVbiField)
{
    int rc;
    NEXUS_DisplayModule_State *video = &g_NEXUS_DisplayModule_State;
    uint32_t polarity;
    BVBI_GSData gsData;
    BVBI_TT_Line *ttLines = link->vbi.ttLines;
    uint16_t wss;
    uint32_t cgms;
    BVBI_VPSData vps;

    rc = BVBIlib_List_Obtain_isr(video->vbilist, pNewVbiField);
    if (rc) return BERR_TRACE(rc);
    if (!BVBI_Field_GetPolarity_isr(vbiField, &polarity)) {
        rc = BVBI_Field_SetPolarity_isr(*pNewVbiField, polarity);
        if (rc) {rc = BERR_TRACE(rc); goto error;}
    }
    if (link->vbiSettings.gemStarEnabled && !BVBI_Field_GetGSData_isr(vbiField, &gsData)) {
        rc = BVBI_Field_SetGSData_isr(*pNewVbiField, &gsData);
        if (rc) {rc = BERR_TRACE(rc); goto error;}
    }
    if (link->vbiSettings.teletextEnabled && !BVBI_Field_GetTTData_isr(vbiField, B_VBI_TT_LINES, ttLines)) {
        BFMT_VideoFmt formatVdc;
        rc = NEXUS_P_VideoFormat_ToMagnum_isrsafe(display->cfg.format, &formatVdc);
        if (rc) {return BERR_TRACE(rc);}
        rc = BVBI_Field_SetTTData_isr(*pNewVbiField, formatVdc, B_VBI_TT_LINES, ttLines);
        if (rc) {rc = BERR_TRACE(rc); goto error;}
    }
    if (!BVBI_Field_GetWSSData_isr(vbiField, &wss)) {
        rc = BVBI_Field_SetWSSData_isr(*pNewVbiField, wss);
        if (rc) {rc = BERR_TRACE(rc); goto error;}
    }
    if (!BVBI_Field_GetCGMSData_isr(vbiField, &cgms)) {
        rc = BVBI_Field_SetCGMSData_isr(*pNewVbiField, cgms);
        if (rc) {rc = BERR_TRACE(rc); goto error;}
    }
    if (!BVBI_Field_GetVPSData_isr(vbiField, &vps)) {
        rc = BVBI_Field_SetVPSData_isr(*pNewVbiField, &vps);
        if (rc) {rc = BERR_TRACE(rc); goto error;}
    }
    return 0;
error:
    BVBIlib_List_Return_isr(video->vbilist, *pNewVbiField);
    return rc;
}
#endif

#if NEXUS_VBI_SUPPORT
void
NEXUS_VideoInput_P_VbiData_isr(NEXUS_VideoInput videoInput, const BVBI_Field_Handle *pVbiData, const NEXUS_ClosedCaptionData *pData, unsigned vbiCount)
{
    unsigned i, j;
    NEXUS_DisplayModule_State *video= &g_NEXUS_DisplayModule_State;
    BERR_Code rc;
    NEXUS_VideoInput_P_Link *link = videoInput->destination;

    /* there may be no link. but if there is, we need to assure right away that it's good. */
    if (link) {
        BDBG_OBJECT_ASSERT(link, NEXUS_VideoInput_P_Link);
    }

#if 0
    BDBG_MSG(("NEXUS_VideoInput_P_VbiData_isr %d, type %s", vbiCount, pVbiData?"608":"708"));
#endif

    if(!link) {
        for (i=0;i<vbiCount;i++) {
            BVBIlib_List_Return_isr(video->vbilist, pVbiData[i]);
        }
        BDBG_MSG(("VBI data from unknown input"));
        return;
    }
    BDBG_ASSERT(videoInput->type == link->input_info.type && videoInput->source == link->input_info.source);

    /* capture data that's desired */
    for (i=0;i<vbiCount;i++) {
        uint32_t cgms;
        BVBI_VPSData vps;
        bool used;
        uint32_t polarity;

        (void)BVBI_Field_GetPolarity_isr(pVbiData[i], &polarity);

        if (link->vbiSettings.closedCaptionEnabled) {
            NEXUS_ClosedCaptionData cc;
            if (pData) {
                /* cc from VideoDecoder will have this set. don't extract from BVBI_Field. */
                cc = pData[i]; /* this will init and copy pts & type */
                rc = 0;
            }
            else {
                BKNI_Memset(&cc, 0, sizeof(cc));
                cc.type = NEXUS_VbiDataType_eClosedCaption;

                cc.field = (polarity & (1 << BAVC_Polarity_eTopField)) ? 0 : 1;

                rc = BVBI_Field_GetCCData_isr(pVbiData[i], &cc.data[0], &cc.data[1]);
                if (rc == 0) {
                    cc.noData = false;
                    cc.parityError = false;
                }
                else if (rc == BVBI_ERR_FIELD_BADDATA) {
                    uint32_t errInfo;
                    (void)BVBI_Field_GetErrorInfo_isr(pVbiData[i], &errInfo);
                    cc.noData = (errInfo & (BVBI_LINE_ERROR_FLDH_CONFLICT | BVBI_LINE_ERROR_CC_NODATA));
                    cc.parityError = (errInfo & BVBI_LINE_ERROR_CC_PARITY);
                    rc = 0;
                }
            }

            if (rc==BERR_SUCCESS && !link->vbi.cc.inactive) {
                if (!link->vbi.cc.data) {
                    BDBG_WRN(("No ClosedCaption buffer"));
                } else {
                    NEXUS_ClosedCaptionData *pCc = (NEXUS_ClosedCaptionData *)NEXUS_VideoInput_P_FifoWptr(&link->vbi.cc);
                    *pCc = cc;
                    NEXUS_VideoInput_P_IncFifo_isr(&link->vbi.cc);
                }
            }
        }

        if (link->vbiSettings.gemStarEnabled) {
            BVBI_GSData gsData;
            rc = BVBI_Field_GetGSData_isr(pVbiData[i], &gsData);
            if (rc==BERR_SUCCESS && !link->vbi.gs.inactive) {
                if (!link->vbi.gs.data) {
                    BDBG_WRN(("No GemStar buffer"));
                } else {
                    NEXUS_GemStarData *pGemStarData = (NEXUS_GemStarData *)NEXUS_VideoInput_P_FifoWptr(&link->vbi.gs);

                    /* convert from Magnum to Nexus */
                    pGemStarData->lineMask = gsData.ulDataLines;
                    pGemStarData->errorMask = gsData.ulErrorLines;
                    pGemStarData->topField = (polarity & (1 << BAVC_Polarity_eTopField));
                    BDBG_CASSERT(sizeof(pGemStarData->data) == sizeof(gsData.ulData));
                    BKNI_Memcpy(pGemStarData->data, gsData.ulData, sizeof(pGemStarData->data));

                    NEXUS_VideoInput_P_IncFifo_isr(&link->vbi.gs);
                }
            }
        }

        if (link->vbiSettings.teletextEnabled) {
            BVBI_TT_Line *ttLines = link->vbi.ttLines;
            rc = BVBI_Field_GetTTData_isr(pVbiData[i], B_VBI_TT_LINES, ttLines);
            if (rc==BERR_SUCCESS && !link->vbi.tt.inactive) {
                unsigned i, invalid_cnt = 0;
                unsigned lastLine = 0;

                /* TODO: I'm short circuiting the write loop. We should investigate a change in teletext data
                allocation, in nexus and/or PI, to avoid memcpy altogether. */
                /* take one pass to see if there are any valid lines */
                for (i=0;i<B_VBI_TT_LINES;i++) {
                    /* coverity[uninit_use: FALSE] - BVBI_Field_GetTTData_isr does set all ucFramingCode values, but the logic is complex */
                    if (ttLines[i].ucFramingCode != 0xFF) {
                        lastLine = i;
                    }
                    else {
                        invalid_cnt++;
                    }
                }
                if (!link->vbi.tt.data && invalid_cnt != B_VBI_TT_LINES) {
                    BDBG_WRN(("Teletext enabled and data found, but no buffer allocated"));
                }
                else if (invalid_cnt != B_VBI_TT_LINES) {
                    /* copy up to the last valid line */
                    for (i=0;i<=lastLine;i++) {
                        NEXUS_TeletextLine *pTT = (NEXUS_TeletextLine *)NEXUS_VideoInput_P_FifoWptr(&link->vbi.tt);
                        pTT->lineNumber = i;
                        /* coverity[uninit_use: FALSE] - BVBI_Field_GetTTData_isr does set all ucFramingCode values, but the logic is complex */
                        pTT->framingCode = ttLines[i].ucFramingCode;
                        pTT->topField = (polarity & (1 << BAVC_Polarity_eTopField));

                        if (pTT->framingCode != 0xFF) {
                            BDBG_MSG(("decoding teletext line fc=%x", pTT->framingCode));
                            BDBG_CASSERT(BVBI_TT_MAX_LINESIZE == NEXUS_TELETEXT_LINESIZE);
                            BKNI_Memcpy(pTT->data, ttLines[i].aucData, NEXUS_TELETEXT_LINESIZE);
                        }

                        NEXUS_VideoInput_P_IncFifo_isr(&link->vbi.tt);
                    }
                }
            }
        }

        /* WSS is defined for top-field only */
        if (polarity & (1 << BAVC_Polarity_eTopField)) {
            uint16_t wss;
            /* TODO: consider a decode-enabled flag so we don't always have to test for this */
            rc = BVBI_Field_GetWSSData_isr(pVbiData[i], &wss);
            /* Allow capture of data with parity errors */
            if (rc != BERR_SUCCESS && rc != BVBI_ERR_FIELD_BADDATA) {
                wss = 0xffff; /* there is no current WSS value, so use a special value. */
            }
            if (wss != link->vbi.wss.data) {
                link->vbi.wss.data = wss;
                NEXUS_IsrCallback_Fire_isr(link->vbi.wss.isrCallback);
            }
        }

        rc = BVBI_Field_GetCGMSData_isr(pVbiData[i], &cgms);
        if (!rc) {
            /* success means there is CGMS data present */
            if (cgms != link->vbi.cgms.data) {
                link->vbi.cgms.data = cgms;
                NEXUS_IsrCallback_Fire_isr(link->vbi.cgms.isrCallback);
            }
        }

        rc = BVBI_Field_GetVPSData_isr(pVbiData[i], &vps);
        if (!rc) {
            /* success means there is VPS data present */
            if (BKNI_Memcmp(&link->vbi.vps.data, &vps, sizeof(vps))) {
                BDBG_CASSERT(sizeof(BVBI_VPSData) == sizeof(NEXUS_VpsData));
                BKNI_Memcpy(&link->vbi.vps.data, &vps, sizeof(vps));
                NEXUS_IsrCallback_Fire_isr(link->vbi.vps.isrCallback);
            }
        }

        /* forward to each display connected to this input */
        used = false;
        for(j=0;j<sizeof(video->displays)/sizeof(video->displays[0]);j++) {
            NEXUS_DisplayHandle display = video->displays[j];
            if (display) {
                if (display->vbi.settings.vbiSource == videoInput) {
                    uint8_t data[2];
                    if (display->vbi.settings.closedCaptionEnabled &&
                        !display->vbi.settings.closedCaptionRouting &&
                        BVBI_Field_GetCCData_isr(pVbiData[i], &data[0], &data[1]) == 0 &&
                        (data[0] != 0x80 || data[1] != 0x80))
                    {
                        BVBI_Field_Handle vbiField;
                        rc = nexus_videoinput_p_dup_vbifield_isr(link, display, pVbiData[i], &vbiField);
                        if (rc) {
                            BERR_TRACE(rc);
                        }
                        else {
                            if (!NEXUS_Display_P_VbiData_isr(display, vbiField)) {
                                BVBIlib_List_Return_isr(video->vbilist, vbiField);
                            }
                        }
                    }
                    else if (!NEXUS_Display_P_VbiData_isr(display, pVbiData[i])) {
                        used = true;
                    }
                }
            }
        }
        if (!used) {
            /* if not consumed by at least one display, return it */
            BVBIlib_List_Return_isr(video->vbilist, pVbiData[i]);
        }
    }
}
#endif

NEXUS_Error NEXUS_VideoInput_P_SetVbiState( NEXUS_VideoInput videoInput )
{
#if NEXUS_VBI_SUPPORT
    NEXUS_VideoInput_P_Link *link = NEXUS_VideoInput_P_Get(videoInput);
    NEXUS_DisplayModule_State *video = &g_NEXUS_DisplayModule_State;
    NEXUS_Error rc = 0;
    NEXUS_VideoInputInternalVbiSettings internalVbiSettings;
    unsigned i;

    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(videoInput, NEXUS_VideoInput);
    if (!link) return BERR_TRACE(NEXUS_UNKNOWN);

    /* figure out what needs to be enabled for this video input */
    BKNI_Memset(&internalVbiSettings, 0, sizeof(internalVbiSettings));
    if (link->vbiSettings.teletextEnabled) internalVbiSettings.teletextRefCnt++;
    if (link->vbiSettings.closedCaptionEnabled) internalVbiSettings.closedCaptionRefCnt++;
    if (link->vbiSettings.wssChanged.callback) internalVbiSettings.wssRefCnt++;
    if (link->vbiSettings.cgmsChanged.callback) internalVbiSettings.cgmsRefCnt++;
    if (link->vbiSettings.gemStarEnabled) internalVbiSettings.gemStarRefCnt++;
    if (link->vbiSettings.vpsChanged.callback) internalVbiSettings.vpsRefCnt++;
    for (i=0;i<NEXUS_NUM_DISPLAYS;i++) {
        if (video->displays[i] && video->displays[i]->vbi.settings.vbiSource == videoInput) {
            NEXUS_DisplayVbiSettings *displayVbiSettings = &video->displays[i]->vbi.settings;
            if (displayVbiSettings->teletextRouting) internalVbiSettings.teletextRefCnt++;
            if (displayVbiSettings->closedCaptionRouting) internalVbiSettings.closedCaptionRefCnt++;
            if (displayVbiSettings->wssRouting) internalVbiSettings.wssRefCnt++;
            if (displayVbiSettings->cgmsRouting) internalVbiSettings.cgmsRefCnt++;
        }
    }

    rc = NEXUS_VideoInput_P_ReallocFifo(link->vbiSettings.closedCaptionBufferSize, &link->vbi.cc);
    if (rc) return BERR_TRACE(rc);

    rc = NEXUS_VideoInput_P_ReallocFifo(link->vbiSettings.teletextBufferSize, &link->vbi.tt);
    if (rc) return BERR_TRACE(rc);

    rc = NEXUS_VideoInput_P_ReallocFifo(link->vbiSettings.gemStarBufferSize, &link->vbi.gs);
    if (rc) return BERR_TRACE(rc);

    switch (videoInput->type) {
#if NEXUS_NUM_VIDEO_DECODERS
    case NEXUS_VideoInputType_eDecoder:
        {
        NEXUS_VideoDecoderDisplayConnection decoderConnect;
        bool changed = false;

        NEXUS_Module_Lock(video->modules.videoDecoder);
        NEXUS_VideoDecoder_GetDisplayConnection_priv((NEXUS_VideoDecoderHandle)videoInput->source, &decoderConnect);
        /* VideoDecoder only parses userdata for closedCaption */
        if (internalVbiSettings.closedCaptionRefCnt) {
            if (decoderConnect.vbiDataCallback_isr != NEXUS_VideoInput_P_DigitalVbiData_isr) {
                decoderConnect.vbiDataCallback_isr = NEXUS_VideoInput_P_DigitalVbiData_isr;
                changed = true;
            }
        }
        else {
            if (decoderConnect.vbiDataCallback_isr != NULL) {
                decoderConnect.vbiDataCallback_isr = NULL;
                changed = true;
            }
        }
        if (changed) { /* avoid unecessary work */
            rc = NEXUS_VideoDecoder_SetDisplayConnection_priv((NEXUS_VideoDecoderHandle)videoInput->source, &decoderConnect);
        }
        else {
            rc = 0;
        }
        NEXUS_Module_Unlock(video->modules.videoDecoder);
        }
        break;
#endif
    default:
        /* this is normal. don't print error message here. */
        break;
    }

    return rc;
#else
    return 0;
#endif
}

void NEXUS_VideoInput_FlushClosedCaption(NEXUS_VideoInput videoInput)
{
#if NEXUS_VBI_SUPPORT
    NEXUS_VideoInput_P_Link *link = NEXUS_VideoInput_P_GetExisting(videoInput);
    if (link) {
        NEXUS_VideoInput_P_Fifo_Flush(&link->vbi.cc);
    }
#else
    BSTD_UNUSED(videoInput);
#endif
}

void NEXUS_VideoInput_FlushTeletext(NEXUS_VideoInput videoInput)
{
#if NEXUS_VBI_SUPPORT
    NEXUS_VideoInput_P_Link *link = NEXUS_VideoInput_P_GetExisting(videoInput);
    if (link) {
        NEXUS_VideoInput_P_Fifo_Flush(&link->vbi.tt);
    }
#else
    BSTD_UNUSED(videoInput);
#endif
}

void NEXUS_VideoInput_FlushGemStar(NEXUS_VideoInput videoInput)
{
#if NEXUS_VBI_SUPPORT
    NEXUS_VideoInput_P_Link *link = NEXUS_VideoInput_P_GetExisting(videoInput);
    if (link) {
        NEXUS_VideoInput_P_Fifo_Flush(&link->vbi.gs);
    }
#else
    BSTD_UNUSED(videoInput);
#endif
}

void NEXUS_VideoInput_FlushWss(NEXUS_VideoInput videoInput)
{
    NEXUS_VideoInput_P_Link *link = NEXUS_VideoInput_P_GetExisting(videoInput);
    if (link) {
        BKNI_EnterCriticalSection();
        link->vbi.wss.data = 0;
        BKNI_LeaveCriticalSection();
    }
}

void NEXUS_VideoInput_FlushCgms(NEXUS_VideoInput videoInput)
{
    NEXUS_VideoInput_P_Link *link = NEXUS_VideoInput_P_GetExisting(videoInput);
    if (link) {
        BKNI_EnterCriticalSection();
        link->vbi.cgms.data = 0;
        BKNI_LeaveCriticalSection();
    }
}

void NEXUS_VideoInput_FlushVps(NEXUS_VideoInput videoInput)
{
    NEXUS_VideoInput_P_Link *link = NEXUS_VideoInput_P_GetExisting(videoInput);
    if (link) {
        /* because the data is not atomic, we must do a critical section */
        BKNI_EnterCriticalSection();
        BKNI_Memset(&(link->vbi.vps.data), 0, sizeof(link->vbi.vps.data));
        BKNI_LeaveCriticalSection();
    }
}

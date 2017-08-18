/***************************************************************************
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
#include "bvdc_test.h"
#include "bvdc_display_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_feeder_priv.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_pep_priv.h"
#include "bvdc_mcvp_priv.h"
#include "bvdc_mcdi_priv.h"
#include "bvdc_cfc_priv.h"


BDBG_MODULE(BVDC_TEST);
BDBG_FILE_MODULE(capbuf);

/***************************************************************************
 *
 */
static BERR_Code  BVDC_Test_P_Buffer_ExtractBuffer_isr
    ( BVDC_P_Buffer_Handle            hBuffer,
      BVDC_P_PictureNode            **ppPicture )
{
    BVDC_P_PictureNode *pPrevReaderNode = NULL, *pTempNode = NULL, *pNextTempNode = NULL;
    BERR_Code rc = BERR_SUCCESS;

    /* user will call GetBuffer_isr in the callback function called at the end of reader_isr
     * at this time, the PrevActiveAndNotUsedByUserNode of hBuffer->pCurReaderBuf is used
     * by VFD right now, and we give it to user. This leads to correct lypsync. This buffer
     * will then not be used by VDC PI software until it it returned.  Note: it is OK to have
     * both user and VFD to read the same buffer. Note: if vnet reconfigure happens, we would
     * have hBuffer->ulActiveBufCnt as 0, and pPrevReaderNode->pHeapNode might be NULL!!!
     */
    /* NOTE: when reader pauses, it's a repeat; don't return the previous picture otherwise back-motion! */
    BVDC_P_Buffer_GetPrevActiveAndNotUsedByUserNode(pPrevReaderNode, hBuffer->pCurReaderBuf);
    if ((hBuffer->pPrevReaderBuf != hBuffer->pCurReaderBuf) &&
        (pPrevReaderNode != hBuffer->pCurWriterBuf) && (pPrevReaderNode->pHeapNode) &&
        (!pPrevReaderNode->stFlags.bMute) && (!pPrevReaderNode->stFlags.bMuteFixedColor))
    {
        /* Mark picture node as currently used by user */
        pPrevReaderNode->stFlags.bUsedByUser = true;
        pPrevReaderNode->stFlags.bActiveNode = false;

        *ppPicture =pPrevReaderNode;

        /* XXX hSurface should NOT be needed !!!*/
        /*pTempNode->hSurface = NULL;*/

        /* Decrement active buffer count. */
        hBuffer->ulActiveBufCnt--;
        BDBG_MSG(("Extract bufferId %d, w=%d, r=%d", pPrevReaderNode->ulBufferId,
            hBuffer->pCurWriterBuf->ulBufferId, hBuffer->pCurReaderBuf->ulBufferId));

        /* Toggle the picture node destination polarity pointed
         * to by the next writer and the nodes after it but before the reader. This
         * is necessary to keep the field prediction correct.
         */
        BVDC_P_Buffer_GetPrevActiveAndNotUsedByUserNode(pTempNode, hBuffer->pCurReaderBuf);

        while (pTempNode != hBuffer->pCurWriterBuf)
        {
            pTempNode->eDstPolarity = BVDC_P_NEXT_POLARITY(pTempNode->eDstPolarity);
            BVDC_P_Buffer_GetPrevActiveAndNotUsedByUserNode(pNextTempNode, pTempNode);
            pTempNode = pNextTempNode;
        } ;
    }
    else
    {
        BDBG_MSG(("Extract bufferId %d, node=%p???, w=%d, r=%d",
            pPrevReaderNode->ulBufferId, (void *)pPrevReaderNode->pHeapNode,
            hBuffer->pCurWriterBuf->ulBufferId, hBuffer->pCurReaderBuf->ulBufferId));
        rc = BVDC_ERR_NO_AVAIL_CAPTURE_BUFFER;
    }
    hBuffer->pPrevReaderBuf = hBuffer->pCurReaderBuf;
    return rc;
}

/***************************************************************************
 * {private}
 *
 */
static BERR_Code BVDC_Test_P_Window_CapturePicture_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_Window_CapturedPicture   *pCapturedPic )
{
    BERR_Code eRet = BERR_SUCCESS;
    unsigned int uiPitch;
    BVDC_P_BufferHeapNode  *pHeapNode;
    BVDC_P_BufferHeap_Info *pHeapInfo;
    uint32_t ulBlockOffset = 0;
    BVDC_P_BufferHeapNode  *pHeapNode_R;
    BVDC_P_BufferHeap_Info *pHeapInfo_R;
    uint32_t ulBlockOffset_R = 0;

    BDBG_ENTER(BVDC_Test_P_Window_CapturePicture_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_ASSERT(pCapturedPic);

    /* Clear content */
    BKNI_Memset((void*)pCapturedPic, 0x0, sizeof(BVDC_P_Window_CapturedPicture));

    if(hWindow->uiAvailCaptureBuffers)
    {
        /* Mark the buffer as used by user */
        if((eRet = BVDC_Test_P_Buffer_ExtractBuffer_isr(hWindow->hBuffer, &pCapturedPic->pPicture)))
        {
            pCapturedPic->hPicBlock = NULL;
            pCapturedPic->ulPicBlockOffset = 0;
            pCapturedPic->ulPicBlockOffset_R = 0;
            return eRet;
        }

        /* Decrement number of capture buffers used */
        hWindow->uiAvailCaptureBuffers--;

        pHeapNode = pCapturedPic->pPicture->pHeapNode;

        while (pHeapNode->uiParentNodeBufIndex != BVDC_P_HEAP_INVAID_BUF_INDEX)
        {
            ulBlockOffset += pHeapNode->ulBlockOffset;
            pHeapInfo = pHeapNode->pHeapInfo->pParentHeapInfo;
            pHeapNode = &pHeapInfo->pBufList[pHeapNode->uiParentNodeBufIndex];
        }

        /* Give the MMA block handle to convert a picture to a surface. */
        pCapturedPic->hPicBlock = pHeapNode->pHeapInfo->hMmaBlock;
        pCapturedPic->ulPicBlockOffset = pHeapNode->ulBlockOffset + ulBlockOffset;

        if(pCapturedPic->pPicture->pHeapNode_R != NULL)
        {
            pHeapNode_R = pCapturedPic->pPicture->pHeapNode_R;

            while (pHeapNode_R->uiParentNodeBufIndex != BVDC_P_HEAP_INVAID_BUF_INDEX)
            {
                ulBlockOffset_R += pHeapNode_R->ulBlockOffset;
                pHeapInfo_R = pHeapNode_R->pHeapInfo->pParentHeapInfo;
                pHeapNode_R = &pHeapInfo_R->pBufList[pHeapNode_R->uiParentNodeBufIndex];
            }

            /* Give the MMA block handle to convert a picture to a surface. */
            pCapturedPic->hPicBlock_R = pHeapNode_R->pHeapInfo->hMmaBlock;
            pCapturedPic->ulPicBlockOffset_R = pHeapNode_R->ulBlockOffset + ulBlockOffset_R;
        }

        pCapturedPic->eDispOrientation = pCapturedPic->pPicture->eDispOrientation;

        /* Get polarity */
        pCapturedPic->ePolarity = (BVDC_P_VNET_USED_SCALER_AT_WRITER(pCapturedPic->pPicture->stVnetMode)
                ? pCapturedPic->pPicture->eDstPolarity : pCapturedPic->pPicture->eSrcPolarity);

        /* Get Pixel Format */
        pCapturedPic->ePxlFmt = pCapturedPic->pPicture->ePixelFormat;

        /* Get Height */
        if(pCapturedPic->ePolarity != BAVC_Polarity_eFrame)
        {
            pCapturedPic->ulHeight = pCapturedPic->pPicture->pVfdIn->ulHeight/2;
        }
        else
        {
            pCapturedPic->ulHeight = pCapturedPic->pPicture->pVfdIn->ulHeight;
        }

        /* Get width */
        pCapturedPic->ulWidth = pCapturedPic->pPicture->pVfdIn->ulWidth;

        /* Get picture Id */
        pCapturedPic->ulEncPicId = hWindow->ulEncPicId;
        pCapturedPic->ulDecPicId = hWindow->ulDecPicId;
        pCapturedPic->ulSourceRate = hWindow->ulSourceRate;


        /* Get pitch. See ulPitch in BVDC_P_Capture_SetEnable_isr and ulStride in
           BVDC_P_Feeder_SetPlaybackInfo_isr */
        BPXL_GetBytesPerNPixels_isr(pCapturedPic->ePxlFmt,
            pCapturedPic->pPicture->pVfdIn->ulWidth, &uiPitch);
        pCapturedPic->ulPitch = BVDC_P_ALIGN_UP(uiPitch, BVDC_P_PITCH_ALIGN);

        /* Get Original PTS */
        pCapturedPic->ulOrigPTS = pCapturedPic->pPicture->ulOrigPTS;
        pCapturedPic->bStallStc = pCapturedPic->pPicture->bStallStc;
        pCapturedPic->bIgnorePicture = pCapturedPic->pPicture->bIgnorePicture;
        pCapturedPic->ulPxlAspRatio_x = pCapturedPic->pPicture->ulStgPxlAspRatio_x_y >> 16;
        pCapturedPic->ulPxlAspRatio_y = pCapturedPic->pPicture->ulStgPxlAspRatio_x_y & 0xffff;;
        BDBG_MODULE_MSG(capbuf, ("win[%d] epic[%d] dpic[%d] OrigPTS %x stallstc %d ignorePicture %d asp %x:%x, pol?%d",
            hWindow->eId, pCapturedPic->ulEncPicId, pCapturedPic->ulDecPicId,
            pCapturedPic->ulOrigPTS, pCapturedPic->bStallStc,
            pCapturedPic->bIgnorePicture, pCapturedPic->ulPxlAspRatio_x,
            pCapturedPic->ulPxlAspRatio_y, pCapturedPic->ePolarity));
    }
    else
    {
        pCapturedPic->hPicBlock = NULL;
        pCapturedPic->ulPicBlockOffset = 0;
        pCapturedPic->ulPicBlockOffset_R = 0;
        eRet = BVDC_ERR_NO_AVAIL_CAPTURE_BUFFER;
    }

        BDBG_MSG(("No available buffer: block %p, offset 0x%x",
            (void *)pCapturedPic->hPicBlock, pCapturedPic->ulPicBlockOffset));

    BDBG_LEAVE(BVDC_Test_P_Window_CapturePicture_isr);
    return eRet;
}

/***************************************************************************
 * {private}
 *
 */
static BERR_Code BVDC_Test_P_Window_ReleasePicture_isr
    ( BVDC_Window_Handle               hWindow,
      BMMA_Block_Handle                hPicBlock,
      uint32_t                         ulPicBlockOffset )
{
    BVDC_P_PictureNode *pPicture, *pNextPicture;
    uint32_t cnt = 0;

    BVDC_P_BufferHeapNode  *pHeapNode;
    BVDC_P_BufferHeap_Info *pHeapInfo;
    uint32_t ulBlockOffset = 0;

    BDBG_ENTER(BVDC_Test_P_Window_ReleasePicture_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    pNextPicture = hWindow->hBuffer->pCurReaderBuf;

    do
    {
        pPicture = pNextPicture;
        pHeapNode = pPicture->pHeapNode;
        if (pHeapNode)
        {
            ulBlockOffset = 0;
            while (pHeapNode->uiParentNodeBufIndex != BVDC_P_HEAP_INVAID_BUF_INDEX)
            {
                ulBlockOffset += pHeapNode->ulBlockOffset;
                pHeapInfo = pHeapNode->pHeapInfo->pParentHeapInfo;
                pHeapNode = &pHeapInfo->pBufList[pHeapNode->uiParentNodeBufIndex];
            }
            cnt++;
        }
        BVDC_P_Buffer_GetNextUsedByUserNode(pNextPicture, pPicture);
    /* during vnet reconfigure pHeapNode may be NULL */
    } while ((!pHeapNode || (hPicBlock != pHeapNode->pHeapInfo->hMmaBlock) ||
            (ulPicBlockOffset != pHeapNode->ulBlockOffset + ulBlockOffset)) &&
            (cnt <= BVDC_P_MAX_USER_CAPTURE_BUFFER_COUNT));

    if(cnt > BVDC_P_MAX_USER_CAPTURE_BUFFER_COUNT)
    {
        /* likely it is during vnet reconfigure, what we should do with this
         * return? does vnet reconfigure free and reset flag for all active and
         * usedByUser buffers? */
        BDBG_MSG(("return mistached block, %p???", (void *)hPicBlock));
        return BERR_TRACE(BVDC_ERR_CAPTURED_BUFFER_NOT_FOUND);
    }
    else
    {
        BDBG_MSG(("return bufferId %d, block %p, offset 0x%x; Is Used? %d",
                  pPicture->ulBufferId, (void *)pPicture->pHeapNode->pHeapInfo->hMmaBlock,
                  pPicture->pHeapNode->ulBlockOffset, pPicture->stFlags.bUsedByUser));
        BVDC_P_Buffer_ReturnBuffer_isr(hWindow->hBuffer, pPicture);
        hWindow->uiAvailCaptureBuffers++;
    }

    BDBG_LEAVE(BVDC_Test_P_Window_ReleasePicture_isr);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Test_Window_GetBuffer_isr
    ( BVDC_Window_Handle              hWindow,
      BVDC_Test_Window_CapturedImage *pCapturedImage )
{
    BERR_Code eRet = BERR_SUCCESS;
    BVDC_P_Window_CapturedPicture stCaptPic;

    BDBG_ENTER(BVDC_Test_Window_GetBuffer_isr);
    BDBG_ASSERT(pCapturedImage);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    eRet = BVDC_Test_P_Window_CapturePicture_isr(hWindow, &stCaptPic);
    if (eRet == BERR_SUCCESS)
    {
        pCapturedImage->hPicBlock = stCaptPic.hPicBlock;
        pCapturedImage->ulPicBlockOffset = stCaptPic.ulPicBlockOffset;
        pCapturedImage->ulWidth = stCaptPic.ulWidth;
        pCapturedImage->ulHeight = stCaptPic.ulHeight;
        pCapturedImage->ulEncPicId = stCaptPic.ulEncPicId;
        pCapturedImage->ulDecPicId = stCaptPic.ulDecPicId;
        pCapturedImage->ulPitch = stCaptPic.ulPitch;
        pCapturedImage->ePxlFmt = stCaptPic.ePxlFmt;
        pCapturedImage->eCapturePolarity = stCaptPic.ePolarity;
        pCapturedImage->ulSourceRate = stCaptPic.ulSourceRate;
        pCapturedImage->ulPicBlockOffset_R = stCaptPic.ulPicBlockOffset_R;
        pCapturedImage->eDispOrientation = stCaptPic.eDispOrientation;
        pCapturedImage->ulOrigPTS = stCaptPic.ulOrigPTS;
        pCapturedImage->bStallStc = stCaptPic.bStallStc;
        pCapturedImage->bIgnorePicture = stCaptPic.bIgnorePicture;
        pCapturedImage->ulPxlAspRatio_x = stCaptPic.ulPxlAspRatio_x;
        pCapturedImage->ulPxlAspRatio_y = stCaptPic.ulPxlAspRatio_y;

        BDBG_MODULE_MSG(capbuf, ("OrigPTS %x stallstc %d ignorePicture %d asp %x:%x",
            pCapturedImage->ulOrigPTS, pCapturedImage->bStallStc,
            pCapturedImage->bIgnorePicture,
            pCapturedImage->ulPxlAspRatio_x, pCapturedImage->ulPxlAspRatio_y));
    }
    else
    {
        pCapturedImage->hPicBlock = NULL;
        pCapturedImage->ulPicBlockOffset = 0;
        pCapturedImage->ulPicBlockOffset_R = 0;
    }

    BDBG_MSG(("block %p,  offset 0x%x", (void *)stCaptPic.hPicBlock, stCaptPic.ulPicBlockOffset));

    BDBG_LEAVE(BVDC_Test_Window_GetBuffer_isr);
    return eRet;
    /*return BERR_TRACE(eRet); turn off error print per app request*/
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Test_Window_ReturnBuffer_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_Test_Window_CapturedImage  *pCapturedImage )
{
    BERR_Code eRet = BERR_SUCCESS;

    BDBG_ENTER(BVDC_Test_Window_ReturnBuffer_isr);
    BDBG_ASSERT(pCapturedImage);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if (pCapturedImage->hPicBlock)
    {
        eRet = BVDC_Test_P_Window_ReleasePicture_isr(hWindow,
            pCapturedImage->hPicBlock, pCapturedImage->ulPicBlockOffset);
        if (eRet != BERR_SUCCESS)
        {
            return BERR_TRACE(eRet);
        }
    }

    BDBG_LEAVE(BVDC_Test_Window_ReturnBuffer_isr);
    return BERR_SUCCESS;
}

/***************************************************************************
 * {private}
 *
 */
static void BVDC_P_Cfc_SetForceCfg
    ( BVDC_P_Cfc_ForceCfg             *pCfcForceCfg,
      BVDC_Test_Window_ForceCfcConfig *pWinForceCfcCfg)
{
    pCfcForceCfg->stBits.bDisableNl2l = (pWinForceCfcCfg->bDisableNl2l)? 1 : 0;
    pCfcForceCfg->stBits.bDisableLRangeAdj = (pWinForceCfcCfg->bDisableLRangeAdj)? 1 : 0;
    pCfcForceCfg->stBits.bDisableLmr = (pWinForceCfcCfg->bDisableLmr)? 1 : 0;
    pCfcForceCfg->stBits.bDisableL2nl = (pWinForceCfcCfg->bDisableL2nl)? 1 : 0;
    pCfcForceCfg->stBits.bDisableRamLuts = (pWinForceCfcCfg->bDisableRamLuts)? 1 : 0;
    pCfcForceCfg->stBits.bDisableMb = (pWinForceCfcCfg->bDisableMb)? 1 : 0;
    pCfcForceCfg->stBits.bDisableDolby = (pWinForceCfcCfg->bDisableDolby)? 1 : 0;
    pCfcForceCfg->stBits.bDisableTch = (pWinForceCfcCfg->bDisableTch)? 1 : 0;
    /*pCfcForceCfg->stBits.bDisableItm = (pWinForceCfcCfg->bDisableItm)? 1 : 0;*/
}

/*************************************************************************
 *  BVDC_Test_Window_SetCfcConfig
 *************************************************************************/
BERR_Code BVDC_Test_Window_SetCfcConfig
    ( BVDC_Window_Handle               hWindow,
      BVDC_Test_Window_ForceCfcConfig *pForceCfcCfg)
{
    int ii;
    BVDC_P_Cfc_ForceCfg  stCfcForceCfg;

    BDBG_ENTER(BVDC_Test_Window_SetCfcConfig);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_ASSERT(NULL != pForceCfcCfg);

    BVDC_P_Cfc_SetForceCfg(&stCfcForceCfg, pForceCfcCfg);
    if (BVDC_P_WIN_IS_VIDEO_WINDOW(hWindow->eId))
    {
        for (ii=0; ii<BVDC_P_CMP_CFCS; ii++)
        {
            hWindow->astMosaicCfc[ii].stForceCfg.ulInts = stCfcForceCfg.ulInts;
            hWindow->astMosaicCfc[ii].stColorSpaceIn.stCfg.stBits.bDirty = BVDC_P_DIRTY;
        }
        hWindow->astMosaicCfc[0].pColorSpaceOut->stCfg.stBits.bDirty = BVDC_P_DIRTY;
        hWindow->stNewInfo.stDirty.stBits.bCscAdjust = BVDC_P_DIRTY;
        hWindow->stCurInfo.stDirty.stBits.bCscAdjust = BVDC_P_DIRTY;
        BDBG_MSG(("win%d stCfcForceCfg = %08x", hWindow->eId, stCfcForceCfg.ulInts));
    }
    else if (NULL != hWindow->stCurInfo.hSource->hGfxFeeder)
    {
        hWindow->stCurInfo.hSource->hGfxFeeder->stCfc.stForceCfg.ulInts = stCfcForceCfg.ulInts;
        hWindow->stCurInfo.hSource->hGfxFeeder->stCfc.stColorSpaceIn.stCfg.stBits.bDirty = BVDC_P_DIRTY;
        hWindow->stCurInfo.hSource->hGfxFeeder->stCfc.pColorSpaceOut->stCfg.stBits.bDirty = BVDC_P_DIRTY;
        hWindow->stCurInfo.hSource->hGfxFeeder->stNewCfgInfo.stDirty.stBits.bCsc = BVDC_P_DIRTY;
        hWindow->stCurInfo.hSource->hGfxFeeder->stCurCfgInfo.stDirty.stBits.bCsc = BVDC_P_DIRTY;
        BDBG_MSG(("gfx%d stCfcForceCfg = %08x", hWindow->stCurInfo.hSource->hGfxFeeder->eId, stCfcForceCfg.ulInts));
    }

    BDBG_LEAVE(BVDC_Test_Window_SetCfcConfig);
    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL /** { **/

/*************************************************************************
 *  BVDC_Test_Window_SetMadOsd
 *************************************************************************/
BERR_Code BVDC_Test_Window_SetMadOsd
    ( BVDC_Window_Handle               hWindow,
      bool                             bEnable,
      uint32_t                         ulHpos,
      uint32_t                         ulVpos)
{
    BVDC_P_Mcdi_Handle  hMcdi  = NULL;
    BDBG_ENTER(BVDC_Test_Window_SetMadOsd);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    hMcdi = hWindow->stCurResource.hMcvp->hMcdi;

    if(!hMcdi)
    {
        BDBG_ERR(("Window %d doesn't support deinterlacing  Mcdi%p", hWindow->eId, (void *)hMcdi));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if(hMcdi)
    {
        /* set new value */
        hMcdi->bEnableOsd = bEnable;
        /* Note: ulHPos must be an even number due to YUV422 format */
        hMcdi->ulOsdHpos  = ulHpos & (~1);
        hMcdi->ulOsdVpos  = ulVpos;

    }

    BDBG_LEAVE(BVDC_Test_Window_SetMadOsd);
    return BERR_SUCCESS;
}

/*************************************************************************
 *  BVDC_Test_Window_GetMadOsd
 *************************************************************************/
BERR_Code BVDC_Test_Window_GetMadOsd
    ( BVDC_Window_Handle               hWindow,
      bool                            *pbEnable,
      uint32_t                        *pulHpos,
      uint32_t                        *pulVpos)
{
    BVDC_P_Mcdi_Handle hMcdi = NULL;
    BDBG_ENTER(BVDC_Test_Window_GetMadOsd);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    hMcdi = hWindow->stCurResource.hMcvp->hMcdi;
    if((!hMcdi) ||
        (!(pbEnable && pulHpos && pulVpos)))
    {
        BDBG_ERR(("Mcdi %p", (void *)hMcdi));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    /* Use BREG_Write32 to load table first */

    if(hMcdi)
    {
        *pbEnable = hMcdi->bEnableOsd;
        *pulHpos  = hMcdi->ulOsdHpos;
        *pulVpos  = hMcdi->ulOsdVpos;
    }

    BDBG_LEAVE(BVDC_Test_Window_GetMadOsd);
    return BERR_SUCCESS;
}

/*************************************************************************
 *  BVDC_Test_Source_SetFixColor
 *************************************************************************/
BERR_Code BVDC_Test_Source_SetFixColor
    ( BVDC_Source_Handle               hSource,
      BAVC_Polarity                    eFieldId,
      bool                             bEnable,
      uint32_t                         ulRed,
      uint32_t                         ulGreen,
      uint32_t                         ulBlue )
{
    BVDC_P_Source_Info *pNewInfo;
    uint32_t ulColorARGB;
    unsigned int ulFixColorYCrCb;

    BDBG_ENTER(BVDC_Test_Source_SetFixColor);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
    pNewInfo = &hSource->stNewInfo;

    ulColorARGB = BPXL_MAKE_PIXEL(BPXL_eA8_R8_G8_B8, 0x00,
        ulRed, ulGreen, ulBlue);
    BPXL_ConvertPixel_RGBtoYCbCr(BPXL_eA8_Y8_Cb8_Cr8, BPXL_eA8_R8_G8_B8,
        ulColorARGB, &ulFixColorYCrCb);

    BDBG_ERR(("ulFixColorYCrCb[%d] = 0x%x", eFieldId, ulFixColorYCrCb));

    if((hSource->stCurInfo.bFixColorEnable != bEnable) ||
       (hSource->stCurInfo.aulFixColorYCrCb[eFieldId] != ulFixColorYCrCb) ||
       (hSource->stNewInfo.bErrorLastSetting))
    {
        BVDC_P_Source_DirtyBits *pNewDirty = &pNewInfo->stDirty;

        pNewInfo->bFixColorEnable = bEnable;
        pNewInfo->aulFixColorYCrCb[eFieldId] = ulFixColorYCrCb;

        /* Dirty bit set */
        pNewDirty->stBits.bColorspace = BVDC_P_DIRTY;
    }

    BDBG_LEAVE(BVDC_Test_Source_SetFixColor);
    return BERR_SUCCESS;
}

/*************************************************************************
 *  BVDC_Test_Source_GetFixColor
 *************************************************************************/
BERR_Code BVDC_Test_Source_GetFixColor
    ( BVDC_Source_Handle               hSource,
      BAVC_Polarity                    eFieldId,
      bool                            *pbEnable,
      uint32_t                        *pulRed,
      uint32_t                        *pulGreen,
      uint32_t                        *pulBlue )
{
    unsigned int ulColorARGB;

    BDBG_ENTER(BVDC_Test_Source_GetFixColor);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    BPXL_ConvertPixel_YCbCrtoRGB(BPXL_eA8_R8_G8_B8, BPXL_eA8_Y8_Cb8_Cr8,
        hSource->stCurInfo.aulFixColorYCrCb[eFieldId], 0, 0xFF, &ulColorARGB);

    BDBG_ERR(("aulFixColorYCrCb[%d] = 0x%x", eFieldId, hSource->stCurInfo.aulFixColorYCrCb[eFieldId]));

    if(pbEnable)
    {
        *pbEnable = hSource->stCurInfo.bFixColorEnable;
    }

    if(pulRed)
    {
        *pulRed   = (uint32_t)BPXL_GET_COMPONENT(BPXL_eA8_R8_G8_B8, ulColorARGB, 2);
    }

    if(pulGreen)
    {
        *pulGreen = (uint32_t)BPXL_GET_COMPONENT(BPXL_eA8_R8_G8_B8, ulColorARGB, 1);
    }

    if(pulBlue)
    {
        *pulBlue  = (uint32_t)BPXL_GET_COMPONENT(BPXL_eA8_R8_G8_B8, ulColorARGB, 0);
    }

    BDBG_LEAVE(BVDC_Test_Source_GetFixColor);
    return BERR_SUCCESS;
}

#endif /** } !B_REFSW_MINIMAL **/

BERR_Code BVDC_Test_Source_GetGfdScratchRegisters
    ( BVDC_Source_Handle               hSource,
      uint32_t                        *pulScratchReg1,
      uint32_t                        *pulScratchReg2 )
{
    BVDC_P_GfxFeeder_Handle hGfxFeeder;

    BDBG_ASSERT(hSource);

    hGfxFeeder = hSource->hGfxFeeder;

    *pulScratchReg1 = hGfxFeeder->stGfxSurface.ulSurAddrReg[0];
    *pulScratchReg2 = hGfxFeeder->stGfxSurface.ulSurAddrReg[1];

    return BERR_SUCCESS;
}

/* End of File */

/******************************************************************************
* Copyright (C) 2018 Broadcom.
* The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
*
* This program is the proprietary software of Broadcom and/or its licensors,
* and may only be used, duplicated, modified or distributed pursuant to
* the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied),
* right to use, or waiver of any kind with respect to the Software, and
* Broadcom expressly reserves all rights in and to the Software and all
* intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
* THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
* IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1.     This program, including its structure, sequence and organization,
* constitutes the valuable trade secrets of Broadcom, and you shall use all
* reasonable efforts to protect the confidentiality thereof, and to use this
* information only in connection with your use of Broadcom integrated circuit
* products.
*
* 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
* "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
* OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
* RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
* IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
* A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
* ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
* THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
* OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
* INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
* RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
* HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
* EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
* WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
* FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bvdc_buffer_priv.h"
#include "bvdc_vnet_priv.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_display_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_displayfmt_priv.h"
#include "bvdc_common_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_capture_priv.h"
#include "bvdc_feeder_priv.h"
#include "bvdc_buffer_timestamp_priv.h"
#include "bvdc_buffer_dbg_priv.h"


BDBG_MODULE(BVDC_BUF);
BDBG_FILE_MODULE(BVDC_WIN_BUF);
BDBG_FILE_MODULE(BVDC_BUF_MTGW);
BDBG_OBJECT_ID(BVDC_BUF);

/* VEC alignment may have 200 usecs error, RUL sampling of timestamp may have
   another +/-100 usecs error; worst case double-buffer reader/writer pointers
   timestamps relative error may be up to ~400usecs; */
#define BVDC_P_DOUBLE_BUFFER_TS_TOLERANCE   (\
    2 * BVDC_P_MULTIBUFFER_RW_TOLERANCE + BVDC_P_USEC_ALIGNMENT_THRESHOLD)

#define BVDC_P_PHASE2_TIME_STAMP_ADJUST  0x208D

static void BVDC_P_Buffer_CheckWriterIsrOrder_isr
    ( BVDC_Window_Handle               hWindow,
      const BAVC_Polarity              eSrcPolarity,
      const BAVC_Polarity              eDispPolarity,
      const BVDC_P_Buffer_MtgMode      eMtgMode );

static void BVDC_P_Buffer_MoveSyncSlipWriterNode_isr
    ( BVDC_Window_Handle               hWindow,
      const BAVC_Polarity              eSrcPolarity,
      const BVDC_P_Buffer_MtgMode      eMtgMode );

static void BVDC_P_Buffer_CheckReaderIsrOrder_isr
    ( BVDC_Window_Handle               hWindow,
      const BAVC_Polarity              eSrcPolarity,
      const BAVC_Polarity              eDispPolarity,
      const BVDC_P_Buffer_MtgMode      eMtgMode );

static void BVDC_P_Buffer_MoveSyncSlipReaderNode_isr
    ( BVDC_Window_Handle               hWindow,
      const BAVC_Polarity              eVecPolarity,
      const BVDC_P_Buffer_MtgMode      eMtgMode );


/***************************************************************************
 *
 */
static BVDC_P_PictureNode *BVDC_P_GetWriterByDelay_isr
    ( BVDC_P_PictureNode     *pCurReader,
      uint32_t                ulVsyncDelay )
{
    uint32_t             i = 0;
    BVDC_P_PictureNode  *pTempNode = pCurReader;

    while( i++ < ulVsyncDelay )
    {
        BVDC_P_Buffer_GetNextActiveNode(pTempNode, pTempNode);
    }

    return pTempNode;
}


/***************************************************************************
 *
 */
static BVDC_P_PictureNode *BVDC_P_GetReaderByDelay_isr
     ( BVDC_P_PictureNode             *pCurWriter,
       uint32_t                        ulVsyncDelay )
{
    uint32_t i = 0;
    BVDC_P_PictureNode  *pTempNode = pCurWriter;

    while( i++ < ulVsyncDelay )
    {
        BVDC_P_Buffer_GetPrevActiveNode(pTempNode, pTempNode);
    }

    return pTempNode;
}

/***************************************************************************
  * {private}
  * To get the current reader->writer buffer delay;
  */
static uint32_t BVDC_P_Buffer_GetCurrentDelay_isr
    ( BVDC_Window_Handle               hWindow )
{
    BVDC_P_PictureNode      *pTempNode;

    uint32_t delay_count;

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->hBuffer, BVDC_BUF);

    /* So far there are 3 cases that we allow reader and writer point
     * to the same buffer node:
     *
     * 1) game mode: buffer delay is 0;
     * 2) progressive pull down: buffer delay is the count between reader and writer;
     * 3) flag bRepeatCurrReader is set when adding buffer nodes: buffer delay is 0.
     */
    if (hWindow->hBuffer->pCurReaderBuf == hWindow->hBuffer->pCurWriterBuf)
    {
        bool bProgressivePullDown;

        /* buffer delay is always 0 if bRepeatCurrReader is set */
        if (hWindow->bRepeatCurrReader)
            return 0;
        /* Check whether it is caused by progress pull down */
        BVDC_P_Buffer_CalculateRateGap_isr(hWindow->stCurInfo.hSource->ulVertFreq,
            hWindow->hCompositor->stCurInfo.pFmtInfo->ulVertFreq,
            &hWindow->hBuffer->eWriterVsReaderRateCode, &hWindow->hBuffer->eReaderVsWriterRateCode);

        /* forced synclocked double-buffer might have interlaced pulldown reader overlapped with writer pointer */
        bProgressivePullDown =  VIDEO_FORMAT_IS_PROGRESSIVE(hWindow->stCurInfo.hSource->stCurInfo.pFmtInfo->eVideoFmt) &&
                                (hWindow->hBuffer->eWriterVsReaderRateCode == BVDC_P_WrRate_NotFaster) &&
                                (hWindow->hBuffer->eReaderVsWriterRateCode >= BVDC_P_WrRate_2TimesFaster);

        if (!bProgressivePullDown)
            return 0;
    }

    delay_count = 1;
    BVDC_P_Buffer_GetNextActiveNode(pTempNode, hWindow->hBuffer->pCurReaderBuf);

    while (pTempNode != hWindow->hBuffer->pCurWriterBuf)
    {
        BVDC_P_Buffer_GetNextActiveNode(pTempNode, pTempNode);
        delay_count++;

        if (delay_count == hWindow->hBuffer->ulActiveBufCnt)
        {
            delay_count--;
            BDBG_MSG(("Set delay count to max delay %d", delay_count));
            break;
        }
    }

    return delay_count;
}


/***************************************************************************
 * {private}
 *
 */
static void BVDC_P_Buffer_InitPictureNode
    ( BVDC_P_PictureNode              *pPicture )
{
    int ii;

    /* Fill-in the default info for each node. */
    pPicture->pHeapNode            = NULL;
    pPicture->pHeapNode_R          = NULL;
    BKNI_Memset((void*)&pPicture->stFlags, 0x0, sizeof(BVDC_P_PicNodeFlags));
    pPicture->stFlags.bMute           = true;
    pPicture->stFlags.bCadMatching    = false;

    BVDC_P_CLEAN_ALL_DIRTY(&(pPicture->stVnetMode));
    pPicture->stVnetMode.stBits.bInvalid  = BVDC_P_ON;
    pPicture->eFrameRateCode       = BAVC_FrameRateCode_e29_97;
    pPicture->eDisplayPolarity     = BAVC_Polarity_eTopField;
    pPicture->eSrcPolarity         = BAVC_Polarity_eTopField;
    pPicture->eSrcOrientation      = BFMT_Orientation_e2D;
    pPicture->eDispOrientation     = BFMT_Orientation_e2D;
    pPicture->PicComRulInfo.eSrcOrigPolarity = BAVC_Polarity_eTopField;
    pPicture->eDstPolarity         = BAVC_Polarity_eTopField;
    pPicture->pSurface             = NULL;
    pPicture->pSurface_R           = NULL;
    for (ii=0; ii<BAVC_MOSAIC_MAX; ii++)
    {
        BCFC_InitCfcColorSpace(&pPicture->astMosaicColorSpace[ii]);
        pPicture->astMosaicColorSpace[ii].stMetadata.pDynamic = (void *)&pPicture->astMosaicMetaData[ii];
    }

    pPicture->bValidTimeStampDelay = false;

    pPicture->stSrcOut.lLeft       = 0;
    pPicture->stSrcOut.lLeft_R     = 0;
    pPicture->stSrcOut.lTop        = 0;
    pPicture->stSrcOut.ulWidth     = BFMT_NTSC_WIDTH;
    pPicture->stSrcOut.ulHeight    = BFMT_NTSC_HEIGHT;

    pPicture->stSclOut             = pPicture->stSrcOut;
    pPicture->stWinOut             = pPicture->stSrcOut;
    pPicture->stSclCut             = pPicture->stSrcOut;
    pPicture->stHsclCut            = pPicture->stSrcOut;
    pPicture->stCapOut             = pPicture->stSrcOut;
    pPicture->stVfdOut             = pPicture->stSrcOut;
    pPicture->stMadOut             = pPicture->stSrcOut;

    pPicture->pSrcOut              = &pPicture->stSrcOut;
    pPicture->pSclOut              = &pPicture->stSclOut;
    pPicture->pWinOut              = &pPicture->stWinOut;
    pPicture->pVfdOut              = &pPicture->stVfdOut;
    pPicture->pVfdIn               = &pPicture->stVfdOut;

    pPicture->pDnrOut              = NULL;
    pPicture->pAnrOut              = NULL;
    pPicture->pMadOut              = NULL;
    pPicture->pCapOut              = NULL;
    pPicture->pDnrIn               = NULL;
    pPicture->pAnrIn               = NULL;
    pPicture->pMadIn               = NULL;
    pPicture->pSclIn               = NULL;
    pPicture->pCapIn               = NULL;

    pPicture->pWinIn               = NULL;

    pPicture->ulCaptureTimestamp   = 0;
    pPicture->ulPlaybackTimestamp  = 0;
    pPicture->ulIdrPicID           = 0;
    pPicture->ulPicOrderCnt        = 0;

    /* used for NRT mode transcode: default true to freeze STC and not get encoded */
    pPicture->bIgnorePicture       = true;
    pPicture->bStallStc            = true;
    pPicture->bEnable10Bit         = false;
    pPicture->bEnableDcxm          = false;

    /* used for inbound STG Fmt switch */
    pPicture->pStgFmtInfo          = (BFMT_VideoInfo *)BFMT_GetVideoFormatInfoPtr(BFMT_VideoFmt_eNTSC);

    pPicture->stCustomFormatInfo.eVideoFmt = BFMT_VideoFmt_eCustom2;
    pPicture->stCustomFormatInfo.ulDigitalWidth  = pPicture->stCustomFormatInfo.ulScanWidth  = pPicture->stCustomFormatInfo.ulWidth = 352;
    pPicture->stCustomFormatInfo.ulDigitalHeight = pPicture->stCustomFormatInfo.ulScanHeight = pPicture->stCustomFormatInfo.ulWidth = 288;
    pPicture->stCustomFormatInfo.ulTopActive = pPicture->stCustomFormatInfo.ulActiveSpace
        = pPicture->stCustomFormatInfo.ulTopMaxVbiPassThru = pPicture->stCustomFormatInfo.ulBotMaxVbiPassThru = 0;
    pPicture->stCustomFormatInfo.ulVertFreq     = 5000;
    pPicture->stCustomFormatInfo.ulPxlFreqMask  = BFMT_PXL_27MHz;
    pPicture->stCustomFormatInfo.bInterlaced    = false,
    pPicture->stCustomFormatInfo.eAspectRatio   = BFMT_AspectRatio_e4_3,
    pPicture->stCustomFormatInfo.eOrientation   = BFMT_Orientation_e2D,
    pPicture->stCustomFormatInfo.ulPxlFreq      = 2700,
    pPicture->stCustomFormatInfo.pchFormatStr   =
                                        BDBG_STRING("BFMT_VideoFmt_eCustom2");
    pPicture->stCustomFormatInfo.pCustomInfo    = NULL;

    /* mosaic data init*/
    {

        pPicture->eMadPixelHeapId = BVDC_P_BufferHeapId_eUnknown;
        pPicture->eMadQmHeapId    = BVDC_P_BufferHeapId_eUnknown;
    }

    BKNI_Memset((void*)&pPicture->stCapCompression,
        0x0, sizeof(BVDC_P_Compression_Settings));

    return;
}


/***************************************************************************
 * {private}
 *
 * This function creates a multi-buffer list with zero (0) node.
 */
BERR_Code BVDC_P_Buffer_Create
    ( const BVDC_Window_Handle         hWindow,
      BVDC_P_Buffer_Handle            *phBuffer )
{
    uint32_t i;
    BVDC_P_BufferContext *pBuffer;
    BVDC_P_PictureNode *pPicture;

#if (BVDC_BUF_LOG==1)
    BVDC_P_BufLog *pBufLog;
#endif

    BDBG_ENTER(BVDC_P_Buffer_Create);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* BDBG_SetModuleLevel("BVDC_BUF", BDBG_eMsg);  */

    /* (1) Create buffer context */
    pBuffer = (BVDC_P_BufferContext*)BKNI_Malloc(sizeof(BVDC_P_BufferContext));
    if(!pBuffer)
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Clear out context */
    BKNI_Memset((void*)pBuffer, 0x0, sizeof(BVDC_P_BufferContext));
    BDBG_OBJECT_SET(pBuffer, BVDC_BUF);

    /* These fields do not change during runtime. */
    pBuffer->hWindow      = hWindow;
    pBuffer->ulBufCnt     = BVDC_P_MAX_MULTI_BUFFER_COUNT;

    /* (2) Create buffer head */
    pBuffer->pBufList =
        (BVDC_P_Buffer_Head*)BKNI_Malloc(sizeof(BVDC_P_Buffer_Head));
    if(!pBuffer->pBufList)
    {
        BDBG_OBJECT_DESTROY(pBuffer, BVDC_BUF);
        BKNI_Free(pBuffer);
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BLST_CQ_INIT(pBuffer->pBufList);


    /* (3) Create picture nodes */
    for(i = 0; i < pBuffer->ulBufCnt; i++)
    {

        pPicture = (BVDC_P_PictureNode*)BKNI_Malloc(sizeof(BVDC_P_PictureNode));
        if(!pPicture)
        {
            goto oom_err;
        }


        /* Clear out, insert it into the list. */
        BKNI_Memset((void*)pPicture, 0x0, sizeof(BVDC_P_PictureNode));

        /* Initialize non-changing fields. */
        pPicture->hBuffer    = (BVDC_P_Buffer_Handle)pBuffer;
        pPicture->ulBufferId = i;


#if BVDC_P_DBV_SUPPORT || BVDC_P_TCH_SUPPORT /* alloc DBV or TCH metadata buffer */
        if(hWindow->hCompositor->stCfcCapability[hWindow->eId - BVDC_P_CMP_GET_V0ID(hWindow->hCompositor)].stBits.bDbvToneMapping ||
           hWindow->hCompositor->stCfcCapability[hWindow->eId - BVDC_P_CMP_GET_V0ID(hWindow->hCompositor)].stBits.bTpToneMapping) {
            unsigned j;
            for(j = 0; j < BVDC_P_CMP_0_DBVTCH_NUM_CTX; j++) {
                BDBG_ASSERT(BVDC_P_CMP_0_DBVTCH_NUM_CTX <= BAVC_MOSAIC_MAX);
#if BVDC_P_DBV_SUPPORT
                pPicture->astMosaicMetaData[j].stDbvInput.stHdrMetadata.pData = BKNI_Malloc(BAVC_HDR_METADATA_SIZE_MAX);
                if(!pPicture->astMosaicMetaData[j].stDbvInput.stHdrMetadata.pData)
#elif BVDC_P_TCH_SUPPORT
                pPicture->astMosaicMetaData[j].stTchInput.stHdrMetadata.pData = BKNI_Malloc(BAVC_HDR_METADATA_SIZE_MAX);
                if(!pPicture->astMosaicMetaData[j].stTchInput.stHdrMetadata.pData)
#endif
                {
                    BDBG_ERR(("Win[%d] failed to allocate HDR metadata buffer[%d]", hWindow->eId, i));
                    BKNI_Free(pPicture);
                    goto oom_err;
                }
            }
        }
#endif

        BLST_CQ_INSERT_TAIL(pBuffer->pBufList, pPicture, link);
    }

#if (BVDC_BUF_LOG==1)
    pBufLog = (BVDC_P_BufLog *)BKNI_Malloc(sizeof(BVDC_P_BufLog));
    if(pBufLog == NULL)
    {
        BDBG_ERR(("Failed to create multi-buffer log object."));
        goto oom_err;
    }
    pBuffer->pBufLog = pBufLog;
#endif

    /* All done. now return the new fresh context to user. */
    *phBuffer = (BVDC_P_Buffer_Handle)pBuffer;

    BDBG_LEAVE(BVDC_P_Buffer_Create);
    return BERR_SUCCESS;
oom_err:
    while(i--)
    {
        pPicture = BLST_CQ_FIRST(pBuffer->pBufList);
        BLST_CQ_REMOVE_HEAD(pBuffer->pBufList, link);

#if BVDC_P_DBV_SUPPORT || BVDC_P_TCH_SUPPORT
        if(hWindow->hCompositor->stCfcCapability[hWindow->eId - BVDC_P_CMP_GET_V0ID(hWindow->hCompositor)].stBits.bDbvToneMapping ||
           hWindow->hCompositor->stCfcCapability[hWindow->eId - BVDC_P_CMP_GET_V0ID(hWindow->hCompositor)].stBits.bTpToneMapping) {
            unsigned j;
            for(j = 0; j < BVDC_P_CMP_0_DBVTCH_NUM_CTX; j++) {
#if BVDC_P_DBV_SUPPORT
                if(pPicture->astMosaicMetaData[j].stDbvInput.stHdrMetadata.pData) {
                    BKNI_Free(pPicture->astMosaicMetaData[j].stDbvInput.stHdrMetadata.pData);
                }
#elif BVDC_P_TCH_SUPPORT
                if(pPicture->astMosaicMetaData[j].stTchInput.stHdrMetadata.pData) {
                    BKNI_Free(pPicture->astMosaicMetaData[j].stTchInput.stHdrMetadata.pData);
                }
#endif
            }
        }
#endif
        BKNI_Free(pPicture);
    }
    BKNI_Free(pBuffer->pBufList);
    BDBG_OBJECT_DESTROY(pBuffer, BVDC_BUF);
    BKNI_Free(pBuffer);
    return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
}

/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Buffer_Destroy
    ( BVDC_P_Buffer_Handle             hBuffer )
{
    BVDC_P_PictureNode         *pPicture;

    BDBG_ENTER(BVDC_P_Buffer_Destroy);
    BDBG_OBJECT_ASSERT(hBuffer, BVDC_BUF);

#if (BVDC_BUF_LOG==1)
    /* [4] Free memory alloc for multi-buffer log object */
    BKNI_Free(hBuffer->pBufLog);
    hBuffer->pBufLog = NULL;
#endif

    /* [3] Free memory for individual buffer node */
    while(hBuffer->ulBufCnt--)
    {
        pPicture = BLST_CQ_FIRST(hBuffer->pBufList);
        BLST_CQ_REMOVE_HEAD(hBuffer->pBufList, link);
#if BVDC_P_DBV_SUPPORT || BVDC_P_TCH_SUPPORT
        /* TODO: mosaic mode */
        {
            unsigned j;
            for(j = 0; j < BVDC_P_CMP_0_DBVTCH_NUM_CTX && hBuffer->hWindow->astMosaicCfc[j].stCapability.stBits.bDbvCmp; j++) {
#if BVDC_P_DBV_SUPPORT
                if(pPicture->astMosaicMetaData[j].stDbvInput.stHdrMetadata.pData) {
                    BKNI_Free(pPicture->astMosaicMetaData[j].stDbvInput.stHdrMetadata.pData);
                }
#elif BVDC_P_TCH_SUPPORT
                if(pPicture->astMosaicMetaData[j].stTchInput.stHdrMetadata.pData) {
                    BKNI_Free(pPicture->astMosaicMetaData[j].stTchInput.stHdrMetadata.pData);
                }
#endif
            }
        }
#endif
        BKNI_Free(pPicture);
    }

    /* [2] Free memory for buffer head. */
    BKNI_Free((void*)hBuffer->pBufList);

    BDBG_OBJECT_DESTROY(hBuffer, BVDC_BUF);
    /* [1] Free memory for main context. */
    BKNI_Free((void*)hBuffer);

    BDBG_LEAVE(BVDC_P_Buffer_Destroy);
    return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Buffer_Init
    ( BVDC_P_Buffer_Handle             hBuffer )
{
    uint32_t i;
    BVDC_P_PictureNode   *pPicture;

    BDBG_ENTER(BVDC_P_Buffer_Init);
    BDBG_OBJECT_ASSERT(hBuffer, BVDC_BUF);

    /* Re-Initialize fields that may changes during previous run. */
    hBuffer->bSyncLock        = false;
    hBuffer->ulSkipCnt        = 0;
    hBuffer->ulNumCapField    = 0;
    hBuffer->ulActiveBufCnt   = 0;
    hBuffer->ulVsyncDelay     = 0;
    hBuffer->eWriterVsReaderRateCode  = BVDC_P_WrRate_NotFaster;
    hBuffer->eReaderVsWriterRateCode  = BVDC_P_WrRate_NotFaster;
    hBuffer->eLastBuffAction  = BVDC_P_Last_Buffer_Action_Reader_Moved;

    /* For reader and writer ISR ordering */
    hBuffer->bWriterNodeMovedByReader = false;
    hBuffer->bReaderNodeMovedByWriter = false;
    hBuffer->ulPrevWriterTimestamp = 0;
    hBuffer->ulCurrWriterTimestamp = 0;
    hBuffer->ulPrevReaderTimestamp = 0;
    hBuffer->ulCurrReaderTimestamp = 0;
    hBuffer->bReaderWrapAround = false;
    hBuffer->bWriterWrapAround = false;

#if (BVDC_P_USE_RDC_TIMESTAMP)
    hBuffer->ulMaxTimestamp = BRDC_GetTimerMaxValue(hBuffer->hWindow->hCompositor->hVdc->hRdc);
#else
    hBuffer->ulMaxTimestamp = BTMR_ReadTimerMax();
#endif

    hBuffer->ulGameDelaySamplePeriod = 1;
    hBuffer->ulGameDelaySampleCnt    = 0;

    /* Keep track of skip/repeat statistics */
    hBuffer->ulSkipStat       = 0;
    hBuffer->ulRepeatStat     = 0;

    /* Default for reader and writer. */
    hBuffer->pCurReaderBuf    = BLST_CQ_FIRST(hBuffer->pBufList);
    hBuffer->pCurWriterBuf    = hBuffer->pCurReaderBuf;

    hBuffer->bMtgMadDisplay1To1RateRelationship = false;
    hBuffer->ulMtgPictureRepeatCount = 0;
    hBuffer->ulMtgXdmPicSkipCount = 0;

    /* Initialize all the picture nodes. */
    pPicture = hBuffer->pCurReaderBuf;
    for(i = 0; i < hBuffer->ulBufCnt; i++)
    {
        BVDC_P_Buffer_InitPictureNode(pPicture);
        pPicture = BVDC_P_Buffer_GetNextNode(pPicture);
    }

    BKNI_Memset((void*)hBuffer->aBufAdded, 0x0, sizeof(hBuffer->aBufAdded));
    hBuffer->iLastAddedBufIndex = 0;

#if BVDC_P_REPEAT_ALGORITHM_ONE
    hBuffer->bRepeatForGap = false;
#endif

#if (BVDC_BUF_LOG==1)
    BVDC_P_BufLog_Init(hBuffer->pBufLog);
#endif

    BDBG_LEAVE(BVDC_P_Buffer_Init);
}


/***************************************************************************
 * {private}
 *
 * Add additioanl picture node to the buffer context.
 *
 * ahSurface is the array of all the surface allocated for the buffer/window.
 * Totally ulSurfaceCount surfaces will be added. The index of surfaces added
 * are:
 * hBuffer->ulActiveBufCnt ... hBuffer->ulActiveBufCnt + ulSurfaceCount - 1
 */
BERR_Code BVDC_P_Buffer_AddPictureNodes_isr
    ( BVDC_P_Buffer_Handle             hBuffer,
      BVDC_P_HeapNodePtr               apHeapNode[],
      BVDC_P_HeapNodePtr               apHeapNode_R[],
      uint32_t                         ulSurfaceCount,
      uint32_t                         ulBufDelay,
      bool                             bSyncLock,
      bool                             bInvalidate)
{
    uint32_t                    i;
    BVDC_P_PictureNode         *pPicture;
    BERR_Code                   err = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_Buffer_AddPictureNodes_isr);
    BDBG_OBJECT_ASSERT(hBuffer, BVDC_BUF);

    if(ulSurfaceCount + hBuffer->ulActiveBufCnt > BVDC_P_MAX_MULTI_BUFFER_COUNT)
    {
        BDBG_ERR(("More than MAX!"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Update delay */
    hBuffer->ulVsyncDelay = ulBufDelay;

    /* 1) always add the new buffer node right after the current writer*/
    for(i = 0; i < ulSurfaceCount; i++)
    {
        BDBG_ASSERT(apHeapNode);
        BDBG_ASSERT(apHeapNode[i]);

        pPicture = BVDC_P_Buffer_GetNextNode(hBuffer->pCurWriterBuf);

        while(pPicture->stFlags.bActiveNode || pPicture->stFlags.bUsedByUser)
        {
            pPicture = BVDC_P_Buffer_GetNextNode(pPicture);
        }

        /* Get a non active node */
        pPicture->pHeapNode        = apHeapNode[i];
        if(apHeapNode_R)
            pPicture->pHeapNode_R      = apHeapNode_R[i];
        else
            pPicture->pHeapNode_R      = NULL;
        pPicture->stFlags.bActiveNode      = true;
        pPicture->stFlags.bMute            = true;
        pPicture->eDisplayPolarity = BAVC_Polarity_eTopField;
        pPicture->eSrcOrientation  = BFMT_Orientation_e2D;
        pPicture->eDispOrientation = BFMT_Orientation_e2D;

        /* We will have to reposition this newly allocated picture node.
         */
        /* Take the node out from the buffer chain.
         */
        BLST_CQ_REMOVE(hBuffer->pBufList, pPicture, link);

        /* Add this node back to the chain. Place it to be
         * the one right after the current writer.
         */
        BLST_CQ_INSERT_AFTER(hBuffer->pBufList, hBuffer->pCurWriterBuf, pPicture, link);

        hBuffer->ulActiveBufCnt++;
        hBuffer->aBufAdded[hBuffer->iLastAddedBufIndex] = pPicture;
        hBuffer->iLastAddedBufIndex++;

        /* Buffer initialization, point current writer to a active buffer */
        if (!hBuffer->pCurWriterBuf->stFlags.bActiveNode)
            hBuffer->pCurWriterBuf = pPicture;

#if (BVDC_BUF_LOG == 1)
        BVDC_P_BufLog_AddEvent_isr('U',
                            hBuffer->hWindow->eId,
                            hBuffer->ulActiveBufCnt,
                            0,
                            0,
                            0,
                            0,
                            0,
                            hBuffer);
#else

        BDBG_MSG(("Add buffer heap node %p (%s %2d) to B%d",
            (void *)apHeapNode[i],
            BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(apHeapNode[i]->pHeapInfo->eBufHeapId),
            apHeapNode[i]->uiBufIndex, pPicture->ulBufferId));
        if(apHeapNode_R)
        {
            BDBG_MSG(("Add Right buffer heap node %p (%s %2d) to B%d",
                (void *)apHeapNode_R[i],
                BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(apHeapNode_R[i]->pHeapInfo->eBufHeapId),
                apHeapNode_R[i]->uiBufIndex, pPicture->ulBufferId));
        }
#endif
    }

    hBuffer->bSyncLock = bSyncLock;

    if (bInvalidate)
    {
        hBuffer->pCurReaderBuf = BVDC_P_GetReaderByDelay_isr(
            hBuffer->pCurWriterBuf, ulBufDelay);
    }
    else
    {
        /* 2) Set repeat current reader flag until buffer delay is reached;
           force reader repeat until the reader/writer buffer delay catches up
           with hBuffer->ulVsyncDelay; resume normal movement of buffer pointers
           afterwards. */
        hBuffer->hWindow->bRepeatCurrReader = true;
    }

    BDBG_LEAVE(BVDC_P_Buffer_AddPictureNodes_isr);
    return err;
}


/***************************************************************************
 *
 */
static BVDC_P_PictureNode *BVDC_P_Buffer_LastAddedNonUserCaptureNode_isr
    ( BVDC_P_Buffer_Handle             hBuffer )
{
    int tempIdx, i;
    BVDC_P_PictureNode *pPicture;

    BDBG_OBJECT_ASSERT(hBuffer, BVDC_BUF);

    /* Locate the last added but not used for user capture buffer
     * node.
     */
    tempIdx = hBuffer->iLastAddedBufIndex - 1;

    while(hBuffer->aBufAdded[tempIdx]->stFlags.bUsedByUser)
    {
        BDBG_ASSERT(tempIdx);
        tempIdx--;
    }

    pPicture = hBuffer->aBufAdded[tempIdx];

    /* Shift all the nodes before this one down so that
     * there is no hole in aBufAdded[] array.
     */
    for (i = tempIdx; i < (hBuffer->iLastAddedBufIndex - 1); i++)
        hBuffer->aBufAdded[i] = hBuffer->aBufAdded[i+1];

    return pPicture;

}



/***************************************************************************
 * {private}
 *
 * Release picture nodes from the buffer context.
 *
 * ahSurface is the array of all the surface allocated for the buffer/window.
 * Totally ulSurfaceCount surfaces will be released. The index of surfaces
 * released are:
 * pBuffer->ulActiveBufCnt - ulSurfaceCount... pBuffer->ulActiveBufCnt - 1
 *
 */
BERR_Code BVDC_P_Buffer_ReleasePictureNodes_isr
    ( BVDC_P_Buffer_Handle             hBuffer,
      BVDC_P_HeapNodePtr               apHeapNode[],
      BVDC_P_HeapNodePtr               apHeapNode_R[],
      uint32_t                         ulSurfaceCount,
      uint32_t                         ulBufDelay)
{
    uint32_t                    i;
    BVDC_P_PictureNode         *pBufferToRemove;
    BERR_Code                   err = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_Buffer_ReleasePictureNodes_isr);
    BDBG_OBJECT_ASSERT(hBuffer, BVDC_BUF);

    if(hBuffer->ulActiveBufCnt < ulSurfaceCount)
    {
        BDBG_ERR(("Less than MIN!"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    hBuffer->ulVsyncDelay = ulBufDelay;

    /* Always remove the last added buffer node, but it
     * must not be marked as user capture buffer
     */
    for(i = 0; i < ulSurfaceCount; i++)
    {
        BDBG_ASSERT(hBuffer->iLastAddedBufIndex);

        pBufferToRemove = BVDC_P_Buffer_LastAddedNonUserCaptureNode_isr(hBuffer);
        pBufferToRemove->stFlags.bActiveNode = false;
        apHeapNode[i] = pBufferToRemove->pHeapNode;
        if(apHeapNode_R)
            apHeapNode_R[i] = pBufferToRemove->pHeapNode_R;

#if (BVDC_BUF_LOG == 1)
        hBuffer->iLastAddedBufIndex--;
        hBuffer->ulActiveBufCnt--;

        BVDC_P_BufLog_AddEvent_isr('V',
                            hBuffer->hWindow->eId,
                            hBuffer->ulActiveBufCnt,
                            0,
                            0,
                            0,
                            0,
                            0,
                            hBuffer);
#else
        BDBG_MSG(("Release buffer heap node %p (%s %2d) to B%d",
            (void *)pBufferToRemove->pHeapNode,
            BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(apHeapNode[i]->pHeapInfo->eBufHeapId),
            pBufferToRemove->pHeapNode->uiBufIndex, pBufferToRemove->ulBufferId));
        if(apHeapNode_R)
        {
            BDBG_MSG(("Release Right buffer heap node %p (%s %2d) to B%d",
                (void *)pBufferToRemove->pHeapNode_R,
                BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(apHeapNode_R[i]->pHeapInfo->eBufHeapId),
                pBufferToRemove->pHeapNode->uiBufIndex, pBufferToRemove->ulBufferId));
        }

        hBuffer->iLastAddedBufIndex--;
        hBuffer->ulActiveBufCnt--;
#endif
    }

    if(hBuffer->ulActiveBufCnt)
    {
        /* Current reader and writer nodes might have been released
         * during the above process. So we have to reposition them.
         */
        while(!hBuffer->pCurWriterBuf->stFlags.bActiveNode)
            BVDC_P_Buffer_GetNextActiveNode(hBuffer->pCurWriterBuf, hBuffer->pCurWriterBuf);

        hBuffer->pCurReaderBuf = BVDC_P_GetReaderByDelay_isr(
            hBuffer->pCurWriterBuf, ulBufDelay);
    }
    else
    {
        hBuffer->pCurReaderBuf = BLST_CQ_FIRST(hBuffer->pBufList);
        hBuffer->pCurWriterBuf = hBuffer->pCurReaderBuf;
    }

    BDBG_LEAVE(BVDC_P_Buffer_ReleasePictureNodes_isr);
    return err;
}


/***************************************************************************
 * {private}
 * Add or free picture node for right capture buffer.
 *
 */
BERR_Code BVDC_P_Buffer_SetRightBufferPictureNodes_isr
    ( BVDC_P_Buffer_Handle             hBuffer,
      BVDC_P_HeapNodePtr               apHeapNode_R[],
      uint32_t                         ulCount,
      bool                             bAdd)
{
    uint32_t                    i;
    BVDC_P_PictureNode         *pPicture;
    BERR_Code                   err = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_Buffer_SetRightBufferPictureNodes_isr);
    BDBG_OBJECT_ASSERT(hBuffer, BVDC_BUF);

    BDBG_ASSERT(ulCount == hBuffer->ulActiveBufCnt);

    /* Start with current writer*/
    pPicture = hBuffer->pCurWriterBuf;

    if(bAdd)
    {
        for(i = 0; i < ulCount; i++)
        {
            /* Get next active node */
            while(!pPicture->stFlags.bActiveNode || pPicture->pHeapNode_R)
            {
                BVDC_P_Buffer_GetNextActiveNode(pPicture, pPicture);
            }
            pPicture->pHeapNode_R = apHeapNode_R[i];
            BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Add Right buffer heap node %p (%s %2d) to B%d (%d)",
                (void *)apHeapNode_R[i],
                BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(apHeapNode_R[i]->pHeapInfo->eBufHeapId),
                apHeapNode_R[i]->uiBufIndex, pPicture->ulBufferId,
                pPicture->stFlags.bMute));
        }
    }
    else
    {
        for(i = 0; i < ulCount; i++)
        {
            /* Get next active node */
            while(!pPicture->stFlags.bActiveNode || (pPicture->pHeapNode_R == NULL))
            {
                BVDC_P_Buffer_GetNextActiveNode(pPicture, pPicture);
            }
            BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Free Right buffer heap node %p (%s %2d) from B%d",
                (void *)pPicture->pHeapNode_R,
                BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(pPicture->pHeapNode_R->pHeapInfo->eBufHeapId),
                pPicture->pHeapNode_R->uiBufIndex, pPicture->ulBufferId));
            apHeapNode_R[i] = pPicture->pHeapNode_R;
            pPicture->pHeapNode_R = NULL;
        }
    }


    BDBG_LEAVE(BVDC_P_Buffer_SetRightBufferPictureNodes_isr);
    return err;
}


/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Buffer_Invalidate_isr
    ( BVDC_P_Buffer_Handle             hBuffer )
{
    uint32_t                    ulCount;
    BVDC_P_PictureNode         *pTempNode;

    BDBG_ENTER(BVDC_P_Buffer_Invalidate_isr);
    BDBG_OBJECT_ASSERT(hBuffer, BVDC_BUF);

    BDBG_MSG(("Invalidate buffer nodes"));

    /* Invalidate all capture data */
    pTempNode = hBuffer->pCurReaderBuf;
    for( ulCount = 0; ulCount < hBuffer->ulBufCnt; ulCount++ )
    {
        pTempNode->eDstPolarity               = BAVC_Polarity_eTopField;
        pTempNode->eDisplayPolarity           = BAVC_Polarity_eTopField;
        pTempNode->eOrigSrcOrientation        = BFMT_Orientation_e2D;
        pTempNode->eSrcOrientation            = BFMT_Orientation_e2D;
        pTempNode->eDispOrientation           = BFMT_Orientation_e2D;
#if (BVDC_P_DCX_3D_WORKAROUND)
        pTempNode->bEnableDcxm = hBuffer->hWindow->bSupportDcxm &&
            !BFMT_IS_3D_MODE(hBuffer->hWindow->hCompositor->stCurInfo.pFmtInfo->eVideoFmt);
        pTempNode->bEnable10Bit = hBuffer->hWindow->bIs10BitCore && pTempNode->bEnableDcxm;
#else
        pTempNode->bEnable10Bit               = hBuffer->hWindow->bIs10BitCore;
        pTempNode->bEnableDcxm                = hBuffer->hWindow->bSupportDcxm;
#endif
        pTempNode->stFlags.bMute              = true;
        pTempNode->stFlags.bMuteMad           = false;
        pTempNode->stFlags.bPictureRepeatFlag = false;
        pTempNode->stFlags.bRepeatField       = false;
        pTempNode->stFlags.bCadMatching       = false;
        pTempNode->ulAdjQp                    = 0;
        pTempNode->ulCaptureTimestamp         = 0;
        pTempNode->ulPlaybackTimestamp        = 0;
        pTempNode = BVDC_P_Buffer_GetNextNode(pTempNode);
    }

    /* reset capture number */
    hBuffer->ulNumCapField  = 0;
    hBuffer->ulSkipCnt      = 0;
    hBuffer->pCurWriterBuf = BVDC_P_GetWriterByDelay_isr(
        hBuffer->pCurReaderBuf, hBuffer->ulVsyncDelay);
    hBuffer->eLastBuffAction = BVDC_P_Last_Buffer_Action_Reader_Moved;
    hBuffer->bWriterNodeMovedByReader = false;
    hBuffer->bReaderNodeMovedByWriter = false;
    hBuffer->ulPrevWriterTimestamp = 0;
    hBuffer->ulCurrWriterTimestamp = 0;
    hBuffer->ulPrevReaderTimestamp = 0;
    hBuffer->ulCurrReaderTimestamp = 0;

    hBuffer->bMtgMadDisplay1To1RateRelationship = false;
    hBuffer->ulMtgPictureRepeatCount = 0;
    hBuffer->ulMtgXdmPicSkipCount = 0;

#if BVDC_P_REPEAT_ALGORITHM_ONE
    hBuffer->bRepeatForGap = false;
#endif

#if (BVDC_BUF_LOG == 1)
    BVDC_P_BufLog_AddEvent_isr('Q',
                        hBuffer->hWindow->eId,
                        hBuffer->pCurReaderBuf->ulBufferId,
                        hBuffer->pCurWriterBuf->ulBufferId,
                        0,
                        0,
                        0,
                        0,
                        hBuffer);

#else
    BDBG_MSG(("Set reader buffer node to B%d", hBuffer->pCurReaderBuf->ulBufferId));
    BDBG_MSG(("Set writer buffer node to B%d", hBuffer->pCurWriterBuf->ulBufferId));
#endif

    BDBG_LEAVE(BVDC_P_Buffer_Invalidate_isr);
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
void  BVDC_P_Buffer_ReturnBuffer_isr
    ( BVDC_P_Buffer_Handle            hBuffer,
      BVDC_P_PictureNode             *pPicture )
{
    BVDC_P_PictureNode *pTempNode, *pNextTempNode;

    BDBG_OBJECT_ASSERT(hBuffer, BVDC_BUF);

    pPicture->stFlags.bUsedByUser = false;
    pPicture->pSurface = NULL;
    pPicture->stFlags.bMute = true;

    /* TODO: if returned buf heapId is not the same, then free it and re-alloc
     * a buf with right heapId */

    /* If the chain has no active node, it means we are not doing
     * capture any more. Then mark the picture node as not used by user
     * and not active.
     */
    if (hBuffer->ulActiveBufCnt == 0)
    {
        pPicture->stFlags.bActiveNode = false;
    }
    else
    {
        /* We will have to reposition this returned picture node.
         * It can not be inserted between reader and writer since
         * that will affect vsync delay. So we insert it right
         * after the current writer.
         */

        /* Take the node out from the buffer chain.
         */
        BLST_CQ_REMOVE(hBuffer->pBufList, pPicture, link);

        /* Add this node back to the chain. Place it to be
         * the one right after the current writer.
         */
        pNextTempNode = BVDC_P_Buffer_GetNextNode(hBuffer->pCurWriterBuf);
        pPicture->eDstPolarity = pNextTempNode->eDstPolarity;
        BLST_CQ_INSERT_AFTER(hBuffer->pBufList, hBuffer->pCurWriterBuf, pPicture, link);

        /* Increment active buffer count */
        pPicture->stFlags.bActiveNode = true;
        hBuffer->ulActiveBufCnt++;

        /* Toggle the picture node destination polarity pointed
         * to by the nodes after newly adde node but before the reader. This
         * is necessary to keep the field prediction correct.
         */
        BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pTempNode, pPicture);

        while (pTempNode != hBuffer->pCurReaderBuf)
        {
            pTempNode->eDstPolarity = BVDC_P_NEXT_POLARITY(pTempNode->eDstPolarity);
            BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pNextTempNode, pTempNode);
            pTempNode = pNextTempNode;
        } ;
    }

    return;
}


/***************************************************************************
 *
 */
BERR_Code  BVDC_P_Buffer_ExtractBuffer_isr
    ( BVDC_P_Buffer_Handle            hBuffer,
      BVDC_P_PictureNode            **ppPicture )
{
    BVDC_P_PictureNode *pTempNode = NULL, *pNextTempNode = NULL, *pPrevWriterNode = NULL;

    BDBG_OBJECT_ASSERT(hBuffer, BVDC_BUF);

    /* Criterion for such a node:
     * 1) Active
     * 2) Not used by user
     * 3) Not current writer buffer
     * 4) Not the previous writer buffer which could be in use
     * 5) Not between reader and writer because taking one of those out will affect
     *    lipsync delay.
     * 6) The buffer is not muted.
     * 7) The buffer's pixel format matches with user specified.
     *
     * The one after current writer should satisfy the above criterion.
     */

    BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pTempNode, hBuffer->pCurWriterBuf);
    BVDC_P_Buffer_GetPrevActiveAndNotUsedByUserNode(pPrevWriterNode, hBuffer->pCurWriterBuf);

    if ((pTempNode == hBuffer->pCurWriterBuf) || (pTempNode == pPrevWriterNode)
        || (pTempNode == hBuffer->pCurReaderBuf) || pTempNode->stFlags.bMute
        || (pTempNode->ePixelFormat != hBuffer->hWindow->stCurInfo.ePixelFormat)
        || (hBuffer->ulActiveBufCnt ==0)
    )
    {
        BDBG_MSG(("No user capture buffer available! Window %d ", hBuffer->hWindow->eId));
#if 0
        BDBG_ERR(( "current writer ID %d, previous writer ID %d, current reader ID %d, next writer ID %d ",
                    hBuffer->pCurWriterBuf->ulBufferId, pPrevWriterNode->ulBufferId,
                    hBuffer->pCurReaderBuf->ulBufferId, pTempNode->ulBufferId));

        BDBG_ERR(("   Dump out the whole buffer chain    "  ));
        while(pTempNode != hBuffer->pCurWriterBuf)
        {
            BDBG_ERR(("Buffer Id = %d ", pTempNode->ulBufferId));
            BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pNextTempNode, pTempNode);
            pTempNode = pNextTempNode;
        }

        BDBG_ASSERT(0);
#endif
        return BVDC_ERR_NO_AVAIL_CAPTURE_BUFFER;
    }


    /* Mark picture node as currently used by user */
    pTempNode->stFlags.bUsedByUser = true;
    pTempNode->stFlags.bActiveNode = false;

    /* need to be set after the gfx surface has been created */
    pTempNode->pSurface = NULL;

    *ppPicture = pTempNode;

    /* Decrement active buffer count. */
    hBuffer->ulActiveBufCnt--;

    /* Toggle the picture node destination polarity pointed
     * to by the next writer and the nodes after it but before the reader. This
     * is necessary to keep the field prediction correct.
     */
    BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pTempNode, hBuffer->pCurWriterBuf);

    while (pTempNode != hBuffer->pCurReaderBuf)
    {
        pTempNode->eDstPolarity = BVDC_P_NEXT_POLARITY(pTempNode->eDstPolarity);
        BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pNextTempNode, pTempNode);
        pTempNode = pNextTempNode;
    } ;

    return BERR_SUCCESS;
}


/***************************************************************************
 * The main multi-buffering algorithm
 ***************************************************************************/

/***************************************************************************
 * When a pic node is returned by BVDC_P_Buffer_GetNextWriterNode_isr, it is
 * marked as muted for safe. Window_Writer_isr will mark it as un-muted when
 * it is sure that the pic node is valid: the rect ptr is initialized, and
 * the video pixels are really captured.
 */
BVDC_P_PictureNode* BVDC_P_Buffer_GetNextWriterNode_isr
    ( BVDC_Window_Handle           hWindow,
      const BAVC_Polarity          eSrcPolarity,
      const BVDC_P_Buffer_MtgMode  eMtgMode )
{
    BVDC_P_PictureNode  *pNextNode;
    uint32_t             ulTimeStamp;
    uint32_t             ulMadOutPhase=1;
    bool                 bSkipPicDueToXdmMtgRepeat = false;
#if (BVDC_BUF_LOG == 1)
    bool                 bSkipPicDueToMtgPhase = false;
#endif

    BDBG_ENTER(BVDC_P_Buffer_GetNextWriterNode_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->hBuffer, BVDC_BUF);

    if(!hWindow->hBuffer->ulActiveBufCnt)
    {
        hWindow->hBuffer->pCurWriterBuf->stFlags.bMute = true;
        goto done;
    }

    hWindow->hBuffer->ulNumCapField++;

    if (hWindow->stCurInfo.hSource->bMtgSrc)
    {
        uint32_t ulSrcFreq = BVDC_P_Source_RefreshRate_FromFrameRateCode_isrsafe(hWindow->stCurInfo.hSource->eFrameRateCode);

        bool bMtgRepeatPicture = (eMtgMode == BVDC_P_Buffer_MtgMode_eXdmRepeat) &&
                             (hWindow->hBuffer->pCurWriterBuf->bChannelChange ||
                              (hWindow->hBuffer->pCurWriterBuf->stFlags.bPictureRepeatFlag &&
                              hWindow->hBuffer->pCurWriterBuf->stFlags.bRepeatField)) &&
                              !hWindow->hBuffer->pCurWriterBuf->bIgnorePicture;

        bool bUpSampledRate = (ulSrcFreq == hWindow->stCurInfo.hSource->ulVertFreq) ? false : true;

        bSkipPicDueToXdmMtgRepeat = bMtgRepeatPicture && bUpSampledRate &&
                (hWindow->hCompositor->stCurInfo.pFmtInfo->ulVertFreq == ulSrcFreq) ? true : false;

        if (bSkipPicDueToXdmMtgRepeat)
            hWindow->hBuffer->ulMtgXdmPicSkipCount++;
        else
            hWindow->hBuffer->ulMtgXdmPicSkipCount = 0;

        /* More than 2 skips indicates pause. Allow writer to continue instead of skipping. */
        if (hWindow->hBuffer->ulMtgXdmPicSkipCount > 2)
            bSkipPicDueToXdmMtgRepeat = false;
    }

    /* ----------------------------------
     * sync lock case
     *-----------------------------------*/
    /* Move both reader and writer at same time if sync locked */
    if(hWindow->hBuffer->bSyncLock)
    {
        /* Don't need to check state */
        BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pNextNode, hWindow->hBuffer->pCurWriterBuf);
        pNextNode->stFlags.bMute  = true;
        hWindow->hBuffer->pCurWriterBuf = pNextNode;

        /* Move reader */
        /* if just added lipsync delay, repeat until the delay number is reached */
        if(hWindow->hBuffer->hWindow->bRepeatCurrReader)
        {
            uint32_t ulCurDelay = BVDC_P_Buffer_GetCurrentDelay_isr(hWindow);
            if(ulCurDelay >= hWindow->hBuffer->ulVsyncDelay)
            {
                hWindow->hBuffer->hWindow->bRepeatCurrReader = false;
            }

            /* repeat if current delay less or equal to desired delay */
            if(ulCurDelay <= hWindow->hBuffer->ulVsyncDelay)
            {
                BDBG_MSG(("Win%d current buffer delay = %d, expect %d",
                    hWindow->hBuffer->hWindow->eId, ulCurDelay, hWindow->hBuffer->ulVsyncDelay));
                goto done;
            }
        }
        BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pNextNode, hWindow->hBuffer->pCurReaderBuf);
        hWindow->hBuffer->pCurReaderBuf = pNextNode;
        goto done;
    }
    /* forced sync-lock double-buffer works under VEC locking scheme */
    else if(hWindow->stSettings.bForceSyncLock)
    {
        uint32_t ulTimestampDiff;

        /* Note the reduced memory mode (VEC locking) will use timestamp to avoid sticky tearing:
           1) Every writer/reader isr will update its own ts;
           2) if writer or reader isr finds that its updated ts is close to the counterpart's, then
              it means its counterpart was just serviced before itself; in this case, if my next move
              steps onto its counterpart, then next field/frame will get tearing, so pause my move
              right here to prevent tearing;
         */
        /* 1) update writer timestamp */
        BVDC_P_Buffer_UpdateWriterTimestamps_isr(hWindow, eSrcPolarity);

#if (BVDC_BUF_LOG == 1)
        BVDC_P_BufLog_AddEvent_isr('T',
                                  hWindow->eId,
                                  hWindow->hBuffer->ulCurrWriterTimestamp,
                                  0xAABB, /* Writer time stamp mark */
                                  0,
                                  0,
                                  0,
                                  0,
                                  hWindow->hBuffer);
#endif


        /* 2) get the delta ts = |w_ts - r_ts|; */
        if(hWindow->hBuffer->ulCurrWriterTimestamp > hWindow->hBuffer->ulCurrReaderTimestamp)
        {
            ulTimestampDiff = hWindow->hBuffer->ulCurrWriterTimestamp - hWindow->hBuffer->ulCurrReaderTimestamp;
        }
        else
        {
            ulTimestampDiff = hWindow->hBuffer->ulCurrReaderTimestamp - hWindow->hBuffer->ulCurrWriterTimestamp;
        }

        BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pNextNode, hWindow->hBuffer->pCurWriterBuf);

        /* If a) delta Ts is small (reader isr was just served for the aligned vsync), and
              b) writer will step on reader, and
              c) src/display vsync rates are similar,
           then don't move writer pointer since it could tear; */
        if( (ulTimestampDiff > BVDC_P_DOUBLE_BUFFER_TS_TOLERANCE) ||
            (hWindow->hBuffer->pCurReaderBuf != pNextNode) ||
            !BVDC_P_EQ_DELTA(hWindow->stCurInfo.hSource->ulVertFreq,
            hWindow->hCompositor->hDisplay->stCurInfo.ulVertFreq, 50))
        {
            pNextNode->stFlags.bMute  = true;
            hWindow->hBuffer->pCurWriterBuf = pNextNode;
        }
        else
        {
            BDBG_MSG((">>> Pause writer to avoid tearing!"));
        }
#ifdef BVDC_DEBUG_FORCE_SYNC_LOCK
        BDBG_MSG(("Win%d W(%d), R(%d), p(%d), ts_w=%d",
            hWindow->eId, hWindow->hBuffer->pCurWriterBuf->ulBufferId,
            hWindow->hBuffer->pCurReaderBuf->ulBufferId,
            eSrcPolarity, ulTimestampDiff));
#endif
        hWindow->hBuffer->pCurWriterBuf->ulCaptureTimestamp = hWindow->hBuffer->ulCurrWriterTimestamp;
        goto done;
    }


    hWindow->hBuffer->bMtgMadDisplay1To1RateRelationship = ((eMtgMode == BVDC_P_Buffer_MtgMode_eMadPhase) &&
        ((hWindow->hCompositor->stCurInfo.pFmtInfo->ulVertFreq == 24 * BFMT_FREQ_FACTOR)  ||
         (hWindow->hCompositor->stCurInfo.pFmtInfo->ulVertFreq == 25 * BFMT_FREQ_FACTOR))) ? true : false;

    /* See BVDC_P_Window_Writer_isr after the call to BVDC_P_Buffer_GetNextWriterNode_isr(). This ascertains
       that the deinterlacer phase set by the Writer ISR is the same phase used by the multibuffer algorithm in
       determining which MTG-based picture warrants multibuffer processing. */
    if (eMtgMode == BVDC_P_Buffer_MtgMode_eMadPhase)
    {
        ulMadOutPhase = (hWindow->pCurWriterNode->ulMadOutPhase + 1) % 5;
    }

    /* Determine if MTG phase or MTG-based picture repeats warrant multibuffer processing. If not, drop the picture. */
    if (((hWindow->pCurWriterNode->stFlags.bRev32Locked &&
          ((ulMadOutPhase != 3 && ulMadOutPhase != 1) && eMtgMode == BVDC_P_Buffer_MtgMode_eMadPhase)) ||
          bSkipPicDueToXdmMtgRepeat) && !hWindow->pCurWriterNode->bMute)
    {
        hWindow->hBuffer->pCurWriterBuf = hWindow->pCurWriterNode;
        if (eMtgMode == BVDC_P_Buffer_MtgMode_eXdmRepeat)
        {
            BDBG_MODULE_MSG(BVDC_BUF_MTGW,("Win[%d]: Drop repeated picture %d, B[%d]",
                    hWindow->eId,
                    hWindow->hBuffer->pCurWriterBuf->ulDecodePictureId,
                    hWindow->hBuffer->pCurWriterBuf->ulBufferId));
        }
        else
        {
            BDBG_ASSERT(eMtgMode == BVDC_P_Buffer_MtgMode_eMadPhase);
            BDBG_MODULE_MSG(BVDC_BUF_MTGW,("Win[%d]: Use current phase %d, B[%d]",
                    hWindow->eId, hWindow->hBuffer->pCurWriterBuf->ulMadOutPhase, hWindow->hBuffer->pCurWriterBuf->ulBufferId));

#if (BVDC_BUF_LOG == 1)
            bSkipPicDueToMtgPhase = true;
#endif
        }
        goto done;
    }

    /* ----------------------------------
     * sync slip case
     *-----------------------------------*/
    /* if src has lost signal, writer vnet will shut down, we should avoid to force
     * reader's pic node, otherwise reader might start to use a un-initialized pic
     * node.
     * if src is set as constant or repeat mode, also can't move reader's pic
     * node. */
    if((hWindow->stCurInfo.hSource->bStartFeed) &&
       (BVDC_MuteMode_eDisable == hWindow->stCurInfo.hSource->stCurInfo.eMuteMode))
    {
        BVDC_P_Buffer_CheckWriterIsrOrder_isr(hWindow,
            hWindow->stCurInfo.hSource->eNextFieldId,
            BVDC_P_NEXT_POLARITY(hWindow->hBuffer->pCurReaderBuf->eDisplayPolarity),
            eMtgMode);
    }

    if (!hWindow->hBuffer->bWriterNodeMovedByReader)
    {
        BVDC_P_Buffer_MoveSyncSlipWriterNode_isr(hWindow, eSrcPolarity, eMtgMode); /* Update current writer node */
    }
    else
    {
        hWindow->hBuffer->bWriterNodeMovedByReader = false; /* clear the flag */
    }

    /* Update picture timestamp */
    ulTimeStamp = hWindow->hBuffer->ulCurrWriterTimestamp;
    while( ulTimeStamp > hWindow->hBuffer->ulMaxTimestamp )
        ulTimeStamp -= hWindow->hBuffer->ulMaxTimestamp;
    hWindow->hBuffer->pCurWriterBuf->ulCaptureTimestamp = ulTimeStamp;
    /* The delay in this node is now invalid */
    hWindow->hBuffer->pCurWriterBuf->bValidTimeStampDelay = false;


done:

#if (BVDC_BUF_LOG == 1)
    BVDC_P_BufLog_AddEvent_isr('K',
        hWindow->eId,
        hWindow->hBuffer->pCurWriterBuf->ulBufferId,
        (hWindow->hBuffer->ulNumCapField << BVDC_P_BUF_LOG_NUM_CAP_FIELDS_SHIFT) |
            (eSrcPolarity << BVDC_P_BUF_LOG_SRC_POLARITY_SHIFT) |
            (hWindow->hBuffer->pCurWriterBuf->PicComRulInfo.eSrcOrigPolarity << BVDC_P_BUF_LOG_ORIG_SRC_POLARITY_SHIFT) |
            (hWindow->hBuffer->pCurWriterBuf->eDstPolarity << BVDC_P_BUF_LOG_DST_POLARITY_SHIFT),
        hWindow->hBuffer->ulCurrWriterTimestamp,
        (100000/hWindow->hCompositor->stCurInfo.pFmtInfo->ulVertFreq),
        (((bSkipPicDueToXdmMtgRepeat || bSkipPicDueToMtgPhase) ? 1 : 0) << BVDC_P_BUF_LOG_SKIP_DUE_TO_MTG_SHIFT),
        hWindow->hBuffer->pCurWriterBuf->ulDecodePictureId,
        hWindow->hBuffer);
#endif

    BDBG_LEAVE(BVDC_P_Buffer_GetNextWriterNode_isr);
    return (hWindow->hBuffer->pCurWriterBuf);
}



/***************************************************************************
 *
 */
static void BVDC_P_Buffer_CheckWriterIsrOrder_isr
    ( BVDC_Window_Handle               hWindow,
      const BAVC_Polarity              eSrcPolarity,
      const BAVC_Polarity              eDispPolarity,
      const BVDC_P_Buffer_MtgMode      eMtgMode )
{
    uint32_t ulTimestampDiff;

    BDBG_ENTER(BVDC_P_Buffer_CheckWriterIsrOrder_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->hBuffer, BVDC_BUF);

    BVDC_P_Buffer_UpdateTimestamps_isr(hWindow, eSrcPolarity, eDispPolarity);

#if (BVDC_BUF_LOG == 1)
    BVDC_P_BufLog_AddEvent_isr('T',
                                hWindow->eId,
                                hWindow->hBuffer->ulCurrWriterTimestamp,
                                hWindow->hBuffer->ulCurrReaderTimestamp,
                                hWindow->hBuffer->ulNumCapField,
                                0,
                                0,
                                0,
                                hWindow->hBuffer);
#endif

    /* reader is seemingly ahead of writer so maybe misordered */
#if (BVDC_BUF_LOG == 1)
    BVDC_P_BufLog_AddEvent_isr('Y',
                                hWindow->eId,
                                hWindow->stCurResource.hPlayback->ulTimestamp,
                                hWindow->hBuffer->ulCurrWriterTimestamp,
                                hWindow->hBuffer->ulCurrReaderTimestamp,
                                0,
                                0,
                                0,
                                hWindow->hBuffer);
#endif

    if ( hWindow->hBuffer->ulCurrReaderTimestamp <= hWindow->hBuffer->ulCurrWriterTimestamp )
    {
        /* Verify if reader timestamp is equivalent to playback timestamp. If not, then we have a misordered ISR */
        if (hWindow->stCurResource.hPlayback->ulTimestamp != hWindow->hBuffer->ulCurrReaderTimestamp)
        {
            ulTimestampDiff = hWindow->hBuffer->ulCurrWriterTimestamp - hWindow->hBuffer->ulCurrReaderTimestamp;

            /* detemine whether we keep cadence or not */
            if (ulTimestampDiff < BVDC_P_MULTIBUFFER_RW_TOLERANCE) /* keep current cadence */
            {
                if ((hWindow->hBuffer->eLastBuffAction == BVDC_P_Last_Buffer_Action_Writer_Moved) &&
                    (hWindow->hBuffer->eWriterVsReaderRateCode == hWindow->hBuffer->eReaderVsWriterRateCode))
                {
                    BVDC_P_Buffer_MoveSyncSlipReaderNode_isr(hWindow, eDispPolarity, eMtgMode);
                hWindow->hBuffer->bReaderNodeMovedByWriter = true;
#if (BVDC_BUF_LOG == 1)
                    BVDC_P_BufLog_AddEvent_isr('S',
                        hWindow->eId,
                        hWindow->hBuffer->ulNumCapField,
                        hWindow->hBuffer->ulCurrWriterTimestamp,
                        hWindow->hBuffer->ulCurrReaderTimestamp,
                        0,
                        0,
                        0,
                        hWindow->hBuffer);
#else
                    BDBG_MSG(("(A) Win[%d] W: %d writer ISR moved reader due to misordered ISR",
                        hWindow->eId, hWindow->hBuffer->ulNumCapField));
#endif
                }
                else
                {

                    BDBG_MSG(("(A) Win[%d] Writer ISR can only move the reader once.", hWindow->eId));
                }
            }
        }
    }



    /* Update Capture ISR timestamp */
    if (hWindow->hBuffer->bWriterWrapAround)
    {
        hWindow->stCurResource.hCapture->ulTimestamp = hWindow->hBuffer->ulCurrWriterTimestamp - hWindow->hBuffer->ulMaxTimestamp;
    }
    else
    {
        hWindow->stCurResource.hCapture->ulTimestamp = hWindow->hBuffer->ulCurrWriterTimestamp;
    }

    BDBG_LEAVE(BVDC_P_Buffer_CheckWriterIsrOrder_isr);
    return;
}

static void BVDC_P_Buffer_MoveSyncSlipWriterNode_isr
    ( BVDC_Window_Handle               hWindow,
      const BAVC_Polarity              eSrcPolarity,
      const BVDC_P_Buffer_MtgMode      eMtgMode )
{
    BVDC_P_PictureNode *pNextNode, *pNextNextNode, *pPrevNode;
    bool                bSkip;
    uint32_t            ulGap;
    bool                bProgressivePullDown = false;

    BDBG_ENTER(BVDC_P_Buffer_MoveSyncSlipWriterNode_isr);

    BSTD_UNUSED(eSrcPolarity);
    BSTD_UNUSED(eMtgMode);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->hBuffer, BVDC_BUF);

    if(hWindow->hBuffer->ulNumCapField < BVDC_P_BUFFER_NUM_FIELD_CAPTURE_B4_DISPLAY)
        goto done;

    /* keep track of the last buffer buffer action  */
    hWindow->hBuffer->eLastBuffAction = BVDC_P_Last_Buffer_Action_Writer_Moved;

    /* Get next writer buffer */
    BVDC_P_Buffer_GetPrevActiveAndNotUsedByUserNode(pPrevNode, hWindow->hBuffer->pCurWriterBuf);
    BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pNextNode, hWindow->hBuffer->pCurWriterBuf);

    /* Get writer buffer after next. Needed for progressive mode. */
    BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pNextNextNode, pNextNode);

    /* Determine rate gap */
    BVDC_P_Buffer_CalculateRateGap_isr(hWindow->stCurInfo.hSource->ulVertFreq,
        hWindow->hCompositor->stCurInfo.pFmtInfo->ulVertFreq,
        &hWindow->hBuffer->eWriterVsReaderRateCode, &hWindow->hBuffer->eReaderVsWriterRateCode);

    /* determine the timestamp sampling period for game mode */
    if((BVDC_P_ROUND_OFF(hWindow->stCurInfo.hSource->ulVertFreq,
        (BFMT_FREQ_FACTOR/2), BFMT_FREQ_FACTOR) << 1) ==
        BVDC_P_ROUND_OFF(hWindow->hCompositor->stCurInfo.pFmtInfo->ulVertFreq,
        (BFMT_FREQ_FACTOR/2), BFMT_FREQ_FACTOR))
    {
        /* 1) 1to2 src/disp rate ratio */
        hWindow->hBuffer->ulGameDelaySamplePeriod = 2;
    }
    else if((BVDC_P_ROUND_OFF(hWindow->stCurInfo.hSource->ulVertFreq,
        (BFMT_FREQ_FACTOR/2), BFMT_FREQ_FACTOR) * 5) ==
        BVDC_P_ROUND_OFF(hWindow->hCompositor->stCurInfo.pFmtInfo->ulVertFreq,
        (BFMT_FREQ_FACTOR/2), BFMT_FREQ_FACTOR) * 2)
    {
        /* 2) 2to5 src/disp rate ratio */
        hWindow->hBuffer->ulGameDelaySamplePeriod = 5;
    }
    else
    {
        /* 3) default */
        hWindow->hBuffer->ulGameDelaySamplePeriod = 1;
    }

    if (hWindow->stCurInfo.uiVsyncDelayOffset)
    {
        /* Calculate gap between reader and writer. This guarantees that the
         * delay between the writer and the reader will not exceed the
         * desired delay (vysnc delay + 1).
         * Note, this decision is before advancing the write pointer! */
        ulGap = BVDC_P_Buffer_GetCurrentDelay_isr(hWindow);
        bSkip = (ulGap > (hWindow->hBuffer->ulVsyncDelay)) ? true : false;
    }
    else
    {
        bSkip = false;
    }

    /* When displaying 1080p24/25/30 source as 1080p48/50/60, we cut the number
     * of buffer to 3 to save memory. The algorithm allows writer to catch up
     * reader and both of them point to the same buffer node.
     *
     * Note: This may cause video tearing if reader somehow misses interrupts.
     */
    bProgressivePullDown =  VIDEO_FORMAT_IS_PROGRESSIVE(hWindow->stCurInfo.hSource->stCurInfo.pFmtInfo->eVideoFmt) &&
                            (hWindow->hBuffer->eWriterVsReaderRateCode == BVDC_P_WrRate_NotFaster) &&
                            (hWindow->hBuffer->eReaderVsWriterRateCode >= BVDC_P_WrRate_2TimesFaster);

    /* Check if next node is reader OR if we are in full progressive mode. */
    if (((pNextNode == hWindow->hBuffer->pCurReaderBuf) && !bProgressivePullDown) ||
        (((hWindow->hBuffer->eWriterVsReaderRateCode > BVDC_P_WrRate_NotFaster) ||
          hWindow->hBuffer->bMtgMadDisplay1To1RateRelationship) &&
         (pNextNextNode == hWindow->hBuffer->pCurReaderBuf)) ||
        bSkip)
    {
        bool bMtgAlignSrcWithDisplay =
             (hWindow->hBuffer->bMtgAlignSrcAndDisp && (pNextNode->PicComRulInfo.eSrcOrigPolarity != BAVC_Polarity_eFrame)) ?
            true : false;

        hWindow->hBuffer->ulSkipStat++;

        /* Skip one frame if capture as frame, or one field if capture as field but display is progressive and
         * MAD is not at reader side. Otherwise, skip two fields.
         * Note: If MAD is at reader side, skipping one field will break the MAD input TBTB cadence. This
         * will result in MAD falling back to spatial-only mode.
         */
        if (!((hWindow->bFrameCapture) ||
              (VIDEO_FORMAT_IS_PROGRESSIVE(hWindow->hCompositor->stCurInfo.pFmtInfo->eVideoFmt) &&
               (!BVDC_P_MVP_USED_MAD_AT_READER(hWindow->stVnetMode, hWindow->stMvpMode)))) ||
               bMtgAlignSrcWithDisplay)
        {
            hWindow->hBuffer->pCurWriterBuf = pPrevNode;

#if (BVDC_BUF_LOG == 1)
            BVDC_P_BufLog_AddEvent_isr('A',
                hWindow->eId,
                hWindow->hBuffer->pCurWriterBuf->ulBufferId,
                (hWindow->hBuffer->ulNumCapField << BVDC_P_BUF_LOG_NUM_CAP_FIELDS_SHIFT) |
                    (hWindow->hBuffer->ulSkipStat << BVDC_P_BUF_LOG_PIC_SKIP_CNT_SHIFT),
                hWindow->hBuffer->ulCurrWriterTimestamp,
                0,
                (bMtgAlignSrcWithDisplay ? 1 : 0) << BVDC_P_BUF_LOG_SKIP_DUE_TO_MTG_ALIGN_SRC_SHIFT,
                hWindow->hBuffer->pCurWriterBuf->ulDecodePictureId,
                hWindow->hBuffer);
#else
            BDBG_MSG(("Win[%d] W:     Skip 2 fields, num of cap fields %d, total %d",
                hWindow->eId, hWindow->hBuffer->ulNumCapField, hWindow->hBuffer->ulSkipStat));
#endif
        }
        else
        {
#if (BVDC_BUF_LOG == 1)
            BVDC_P_BufLog_AddEvent_isr('B',
                hWindow->eId,
                hWindow->hBuffer->pCurWriterBuf->ulBufferId,
                hWindow->hBuffer->ulNumCapField << BVDC_P_BUF_LOG_NUM_CAP_FIELDS_SHIFT |
                    (hWindow->hBuffer->ulSkipStat << BVDC_P_BUF_LOG_PIC_SKIP_CNT_SHIFT),
                hWindow->hBuffer->ulCurrWriterTimestamp,
                0,
                0,
                0,
                hWindow->hBuffer);
#else
            BDBG_MSG(("Win[%d] W:     Skip 1 frame/field, num of cap frames/field %d, total %d",
                hWindow->eId, hWindow->hBuffer->ulNumCapField, hWindow->hBuffer->ulSkipStat));
#endif
        }
    }
    else
    {
        hWindow->hBuffer->pCurWriterBuf = pNextNode;
    }

#if (BVDC_BUF_LOG == 1)
    if (hWindow->hBuffer->pCurWriterBuf == hWindow->hBuffer->pCurReaderBuf)
    {
        BVDC_P_BufLog_AddEvent_isr('F',
                hWindow->eId,
                hWindow->hBuffer->pCurWriterBuf->ulBufferId,
                hWindow->hBuffer->ulNumCapField << BVDC_P_BUF_LOG_NUM_CAP_FIELDS_SHIFT |
                    (hWindow->hBuffer->ulSkipStat << BVDC_P_BUF_LOG_PIC_SKIP_CNT_SHIFT),
                hWindow->hBuffer->ulCurrWriterTimestamp,
                0,
                0,
                0,
                hWindow->hBuffer);
    }
#endif

    hWindow->hBuffer->pCurWriterBuf->stFlags.bMute = true;

done:

    BDBG_LEAVE(BVDC_P_Buffer_MoveSyncSlipWriterNode_isr);

    return;
}


/***************************************************************************
 * {private}
 *
 */
BVDC_P_PictureNode* BVDC_P_Buffer_GetNextReaderNode_isr
    ( BVDC_Window_Handle     hWindow,
      const BAVC_Polarity    eVecPolarity,
      BVDC_P_Buffer_MtgMode  eMtgMode )
{
    uint32_t                 ulTimeStamp;

    BDBG_ENTER(BVDC_P_Buffer_GetNextReaderNode_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->hBuffer, BVDC_BUF);

    BSTD_UNUSED(eMtgMode);

    if(!hWindow->hBuffer->ulActiveBufCnt)
    {
        goto done;
    }

    /* ----------------------------------
     * sync lock case
     *-----------------------------------*/
    /* Move both reader and writer at same time if sync locked */
    if(hWindow->hBuffer->bSyncLock)
    {
        goto done;
    }
    /* Forced sync-lock under VEC locking scheme */
    else if(hWindow->stSettings.bForceSyncLock)
    {
        uint32_t ulTimestampDiff;
        BVDC_P_PictureNode         *pNextNode;
        uint32_t ulCurDelay;

        /* lipsync delay is enforced later */
        hWindow->bRepeatCurrReader = false;
        ulCurDelay = BVDC_P_Buffer_GetCurrentDelay_isr(hWindow);

        /* Note the reduced memory mode (VEC locking) will use timestamp to avoid sticky tearing:
           1) Every writer/reader isr will update its own ts;
           2) if writer or reader isr finds that its updated ts is close to the counterpart's, then
              it means its counterpart was just serviced before itself; in this case, if my next move
              steps onto its counterpart, then next field/frame will get tearing, so pause my move
              right here to prevent tearing;
         */
        /* 1) update reader timestamp */
        BVDC_P_Buffer_UpdateReaderTimestamps_isr(hWindow, eVecPolarity);

#if (BVDC_BUF_LOG == 1)
        BVDC_P_BufLog_AddEvent_isr('T',
                                  hWindow->eId,
                                  0xBBAA, /* Reader time stamp mark */
                                  hWindow->hBuffer->ulCurrReaderTimestamp,
                                  0,
                                  0,
                                  0,
                                  0,
                                  hWindow->hBuffer);
#endif

        /* 2) get the deltaTs between w_ts and r_ts; */
        if(hWindow->hBuffer->ulCurrWriterTimestamp > hWindow->hBuffer->ulCurrReaderTimestamp)
        {
            ulTimestampDiff = hWindow->hBuffer->ulCurrWriterTimestamp - hWindow->hBuffer->ulCurrReaderTimestamp;
        }
        else
        {
            ulTimestampDiff = hWindow->hBuffer->ulCurrReaderTimestamp - hWindow->hBuffer->ulCurrWriterTimestamp;
        }

        /* get next reader */
        BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pNextNode, hWindow->hBuffer->pCurReaderBuf);

        /* If a) delta Ts is small (writer isr was just served for the aligned vsync), and
              b) reader will step on writer, and
              c) src/display vsync rates are similar,
           then don't move reader pointer since it could tear; */
        if( (ulTimestampDiff > BVDC_P_DOUBLE_BUFFER_TS_TOLERANCE) ||
            (hWindow->hBuffer->pCurWriterBuf != pNextNode) ||
            !BVDC_P_EQ_DELTA(hWindow->stCurInfo.hSource->ulVertFreq,
            hWindow->hCompositor->hDisplay->stCurInfo.ulVertFreq, 50))
        {
            if((/* 1->1 sync-slipped */
                BVDC_P_EQ_DELTA(hWindow->stCurInfo.hSource->ulVertFreq, hWindow->hCompositor->hDisplay->stCurInfo.ulVertFreq, 50)
                ) ||
               (/* 24->60; 60Hz sync-slipped master */
               (hWindow->hBuffer->pCurWriterBuf == hWindow->hBuffer->pCurReaderBuf) &&
               (2397 == hWindow->stCurInfo.hSource->ulVertFreq) &&
               (5994 == hWindow->hCompositor->hDisplay->stCurInfo.ulVertFreq) &&
               (((ulTimestampDiff >= BVDC_P_USEC_ONE_VSYNC_INTERVAL(hWindow->hCompositor->hDisplay->stCurInfo.ulVertFreq) -
                  BVDC_P_DOUBLE_BUFFER_TS_TOLERANCE) &&
                 (ulTimestampDiff <= BVDC_P_USEC_ONE_VSYNC_INTERVAL(hWindow->hCompositor->hDisplay->stCurInfo.ulVertFreq) +
                  BVDC_P_DOUBLE_BUFFER_TS_TOLERANCE)) ||
                ((ulTimestampDiff >= 3 * BVDC_P_USEC_ONE_VSYNC_INTERVAL(hWindow->hCompositor->hDisplay->stCurInfo.ulVertFreq) / 2 -
                  BVDC_P_DOUBLE_BUFFER_TS_TOLERANCE) &&
                 (ulTimestampDiff <= 3 * BVDC_P_USEC_ONE_VSYNC_INTERVAL(hWindow->hCompositor->hDisplay->stCurInfo.ulVertFreq) / 2 +
                  BVDC_P_DOUBLE_BUFFER_TS_TOLERANCE)))) ||
               (/* 1->2 (25/30->50/60); SD sync-slipped master */
               (hWindow->hBuffer->pCurWriterBuf == hWindow->hBuffer->pCurReaderBuf) &&
               (2*hWindow->stCurInfo.hSource->ulVertFreq == hWindow->hCompositor->hDisplay->stCurInfo.ulVertFreq) &&
               ((ulTimestampDiff >= BVDC_P_USEC_ONE_VSYNC_INTERVAL(hWindow->hCompositor->hDisplay->stCurInfo.ulVertFreq) -
                 BVDC_P_DOUBLE_BUFFER_TS_TOLERANCE) &&
                (ulTimestampDiff <= BVDC_P_USEC_ONE_VSYNC_INTERVAL(hWindow->hCompositor->hDisplay->stCurInfo.ulVertFreq) +
                 BVDC_P_DOUBLE_BUFFER_TS_TOLERANCE))))
            {
                /* make sure lipsync delay enforced in case of missed isr */
                if(((ulTimestampDiff > BVDC_P_DOUBLE_BUFFER_TS_TOLERANCE) &&
                    (ulCurDelay >= hWindow->hBuffer->ulVsyncDelay)) ||
                   ((ulTimestampDiff <= BVDC_P_DOUBLE_BUFFER_TS_TOLERANCE) &&
                   (ulCurDelay > hWindow->hBuffer->ulVsyncDelay || ulCurDelay==0)))
                {
                    hWindow->hBuffer->pCurReaderBuf = pNextNode;
                }
                else
                {
                    BDBG_MSG(("== Pause reader to keep desired lipsync delay! curDly=%d, tgtDly=%d",
                        ulCurDelay, hWindow->hBuffer->ulVsyncDelay));
                }
            }
        }
        else
        {
            BDBG_MSG(("|| Pause reader to avoid tearing!"));
        }
#ifdef  BVDC_DEBUG_FORCE_SYNC_LOCK
        BDBG_MSG(("Win%d     R(%d), W(%d), s(%d)->d(%d)->v(%d), TS [%d, %d] [%d, %d], ts_r=%d, srcFreq=%d, dispFreq=%d",
            hWindow->eId, hWindow->hBuffer->pCurReaderBuf->ulBufferId, hWindow->hBuffer->pCurWriterBuf->ulBufferId,
            hWindow->hBuffer->pCurReaderBuf->eSrcPolarity, hWindow->hBuffer->pCurReaderBuf->eDstPolarity, eVecPolarity,
            hWindow->hCompositor->hDisplay->ulTsSampleCount, hWindow->hCompositor->hDisplay->ulTsSamplePeriod,
            hWindow->hCompositor->hDisplay->ulCurrentTs,
            hWindow->stCurInfo.hSource->hSyncLockCompositor->hDisplay->ulCurrentTs, ulTimestampDiff,
            hWindow->stCurInfo.hSource->ulVertFreq, hWindow->hCompositor->hDisplay->stCurInfo.ulVertFreq));
#endif
        hWindow->hBuffer->pCurReaderBuf->ulPlaybackTimestamp = hWindow->hBuffer->ulCurrReaderTimestamp;
        goto done;
    }

    /* ----------------------------------
     * starts sync slip case
     *-----------------------------------*/
    if(hWindow->stCurInfo.hSource->bStartFeed)
    {
        /* Check if reader ISR is executing out of order */
        BVDC_P_Buffer_CheckReaderIsrOrder_isr(hWindow,
            hWindow->stCurInfo.hSource->eNextFieldId,
            BVDC_P_NEXT_POLARITY(hWindow->hBuffer->pCurReaderBuf->eDisplayPolarity),
            eMtgMode);
    }

    if (!hWindow->hBuffer->bReaderNodeMovedByWriter)
        BVDC_P_Buffer_MoveSyncSlipReaderNode_isr(hWindow, eVecPolarity, eMtgMode); /* Update current writer node */
    else
    {
        hWindow->hBuffer->bReaderNodeMovedByWriter = false; /* clear the flag */
    }

    /* Update picture timestamp */
    ulTimeStamp = hWindow->hBuffer->ulCurrReaderTimestamp;
    while( ulTimeStamp > hWindow->hBuffer->ulMaxTimestamp )
        ulTimeStamp -= hWindow->hBuffer->ulMaxTimestamp;
    hWindow->hBuffer->pCurReaderBuf->ulPlaybackTimestamp = ulTimeStamp;
    /* The delay in this node is now valid */
    hWindow->hBuffer->pCurReaderBuf->bValidTimeStampDelay = true;

done:


#if (BVDC_BUF_LOG == 1)
    BVDC_P_BufLog_AddEvent_isr('L',
        hWindow->eId,
        hWindow->hBuffer->pCurReaderBuf->ulBufferId,
        hWindow->hBuffer->ulNumCapField << BVDC_P_BUF_LOG_NUM_CAP_FIELDS_SHIFT |
            (eVecPolarity << BVDC_P_BUF_LOG_VEC_POLARITY_SHIFT) |
            (hWindow->hBuffer->pCurReaderBuf->PicComRulInfo.eSrcOrigPolarity << BVDC_P_BUF_LOG_ORIG_SRC_POLARITY_SHIFT) |
            (hWindow->hBuffer->pCurReaderBuf->eDstPolarity << BVDC_P_BUF_LOG_DST_POLARITY_SHIFT),
        hWindow->hBuffer->ulCurrReaderTimestamp,
        (100000/hWindow->hCompositor->stCurInfo.pFmtInfo->ulVertFreq),
        0,
        hWindow->hBuffer->pCurReaderBuf->ulDecodePictureId,
        hWindow->hBuffer);
#endif

    /* NOTE: The reader and writer should not be pointing to the same node at this point;
     * otherwise, there will be tearing or unexpected video glitches. The only situation
     * where R == W is during the execution of 1/2 field delay game mode.
     */
    BDBG_LEAVE(BVDC_P_Buffer_GetNextReaderNode_isr);
    return (hWindow->hBuffer->pCurReaderBuf);
}

/***************************************************************************
 *
 */
static void BVDC_P_Buffer_CheckReaderIsrOrder_isr
    ( BVDC_Window_Handle               hWindow,
      const BAVC_Polarity              eSrcPolarity,
      const BAVC_Polarity              eDispPolarity,
      const BVDC_P_Buffer_MtgMode      eMtgMode )
{
    uint32_t ulTimestampDiff;

    BDBG_ENTER(BVDC_P_Buffer_CheckReaderIsrOrder_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->hBuffer, BVDC_BUF);

    BVDC_P_Buffer_UpdateTimestamps_isr(hWindow, eSrcPolarity, eDispPolarity);

#if (BVDC_BUF_LOG == 1)
    BVDC_P_BufLog_AddEvent_isr('T',
                              hWindow->eId,
                              hWindow->hBuffer->ulCurrWriterTimestamp,
                              hWindow->hBuffer->ulCurrReaderTimestamp,
                              hWindow->hBuffer->ulNumCapField,
                              0,
                              0,
                              0,
                              hWindow->hBuffer);

    BVDC_P_BufLog_AddEvent_isr('Z',
                              hWindow->eId,
                              hWindow->stCurResource.hCapture->ulTimestamp,
                              hWindow->hBuffer->ulCurrReaderTimestamp,
                              hWindow->hBuffer->ulCurrWriterTimestamp,
                              0,
                              0,
                              0,
                              hWindow->hBuffer);
#endif


    /* writer is seemingly ahead of reader so maybe misordered */
    if ( hWindow->hBuffer->ulCurrReaderTimestamp >= hWindow->hBuffer->ulCurrWriterTimestamp )
    {
        /* Verify if writer timestamp is equivalent to capture timestamp. If not, then we have a misordered ISR */
        if (hWindow->stCurResource.hCapture->ulTimestamp != hWindow->hBuffer->ulCurrWriterTimestamp)
        {
            ulTimestampDiff = hWindow->hBuffer->ulCurrReaderTimestamp - hWindow->hBuffer->ulCurrWriterTimestamp;

            /* detemine whether we keep cadence or not */
            if (ulTimestampDiff < BVDC_P_MULTIBUFFER_RW_TOLERANCE) /* keep current cadence */
            {
                if ((hWindow->hBuffer->eLastBuffAction == BVDC_P_Last_Buffer_Action_Reader_Moved) &&
                    (hWindow->hBuffer->eWriterVsReaderRateCode == hWindow->hBuffer->eReaderVsWriterRateCode))
                {
                    BVDC_P_Buffer_MoveSyncSlipWriterNode_isr(hWindow, eSrcPolarity, eMtgMode);
                    hWindow->hBuffer->bWriterNodeMovedByReader = true;

#if (BVDC_BUF_LOG == 1)
                    BVDC_P_BufLog_AddEvent_isr('T',
                        hWindow->eId,
                        hWindow->hBuffer->ulNumCapField,
                        hWindow->hBuffer->ulCurrReaderTimestamp,
                        hWindow->hBuffer->ulCurrWriterTimestamp,
                        0,
                        0,
                        0,
                        hWindow->hBuffer);
#else
                    BDBG_MSG(("(A) Win[%d] R: %d reader ISR moved writer due to ISR misorder", hWindow->eId, hWindow->hBuffer->ulNumCapField));
#endif
                }
                else
                {

                    BDBG_MSG(("(A) Win[%d] Reader ISR can only move the writer once.", hWindow->eId));
                }
            }
        }
    }


    /* Update Feeder ISR timestamp */
    if (hWindow->hBuffer->bReaderWrapAround)
    {
        hWindow->stCurResource.hPlayback->ulTimestamp = hWindow->hBuffer->ulCurrReaderTimestamp - hWindow->hBuffer->ulMaxTimestamp;
    }
    else
    {
        hWindow->stCurResource.hPlayback->ulTimestamp = hWindow->hBuffer->ulCurrReaderTimestamp;
    }

    BDBG_LEAVE(BVDC_P_Buffer_CheckReaderIsrOrder_isr);
    return;
}


static void BVDC_P_Buffer_MoveSyncSlipReaderNode_isr
    ( BVDC_Window_Handle               hWindow,
      const BAVC_Polarity              eVecPolarity,
      const BVDC_P_Buffer_MtgMode      eMtgMode )
{
    BVDC_P_PictureNode *pNextNode, *pNextNextNode, *pPrevNode, *pCurNode;
    bool                bRepeat;
    uint32_t            ulGap;
    bool                bReverseProgressivePulldown;

#if (BVDC_BUF_LOG == 1)
    bool bForcedRepeat = false;
#endif

    BDBG_ENTER(BVDC_P_Buffer_MoveSyncSlipReaderNode_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->hBuffer, BVDC_BUF);

    if(hWindow->hBuffer->ulNumCapField < BVDC_P_BUFFER_NUM_FIELD_CAPTURE_B4_DISPLAY)
        goto done;

    /* keep track of the last buffer buffer action  */
    hWindow->hBuffer->eLastBuffAction = BVDC_P_Last_Buffer_Action_Reader_Moved;

    /* ----------------------------------
     * if lipsync delay was adjusted and the expected delay has not been reached;
     *-----------------------------------*/
    if(hWindow->hBuffer->hWindow->bRepeatCurrReader)
    {
        uint32_t ulCurDelay = BVDC_P_Buffer_GetCurrentDelay_isr(hWindow);
        if(ulCurDelay >= hWindow->hBuffer->ulVsyncDelay)
        {
            hWindow->hBuffer->hWindow->bRepeatCurrReader = false;
        }

        /* repeat if current delay less or equal to desired */
        if(ulCurDelay <= hWindow->hBuffer->ulVsyncDelay)
        {
            BDBG_MSG(("Win%d current buffer delay = %d, expect %d",
                hWindow->hBuffer->hWindow->eId, ulCurDelay, hWindow->hBuffer->ulVsyncDelay));
            goto done;
        }
    }

    /* Get next reader buffer */
    BVDC_P_Buffer_GetPrevActiveAndNotUsedByUserNode(pPrevNode, hWindow->hBuffer->pCurReaderBuf);
    BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pNextNode, hWindow->hBuffer->pCurReaderBuf);

    /* Get reader buffer after next */
    BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pNextNextNode, pNextNode);

    if (hWindow->stCurInfo.uiVsyncDelayOffset)
    {
        /* Calculate gap between reader and writer. This guarantees that the
         * delay between the writer and the reader will not be less than the
         * desired delay (vysnc delay - 1).
         * Note, this decision is before advancing the read pointer! */
        ulGap = BVDC_P_Buffer_GetCurrentDelay_isr(hWindow);
        bRepeat = (ulGap < hWindow->hBuffer->ulVsyncDelay) ? true : false;

    }
    else
    {
        bRepeat = false;
    }

    /* Source rate is 60(or 59.94) Hz and display rate is 24(or 23.976) Hz. This allows the reader to encroach the writer. It exploits
       the rate difference in that the reader is slow enough and will not overrun the writer at the next Vsync. The writer will be done
       by then. This is needed to avoid judder. */
    bReverseProgressivePulldown = VIDEO_FORMAT_IS_PROGRESSIVE(hWindow->stCurInfo.hSource->stCurInfo.pFmtInfo->eVideoFmt) &&
                                  VIDEO_FORMAT_IS_PROGRESSIVE(hWindow->hCompositor->stCurInfo.pFmtInfo->eVideoFmt) &&
                                  (hWindow->hBuffer->eWriterVsReaderRateCode == BVDC_P_WrRate_MoreThan2TimesFaster) &&
                                  (hWindow->hBuffer->eReaderVsWriterRateCode == BVDC_P_WrRate_NotFaster);

    /* Check if we are encroaching the writer */
    if (((((pNextNode == hWindow->hBuffer->pCurWriterBuf) &&
           !hWindow->stCurInfo.stGameDelaySetting.bEnable) ||
          ((hWindow->hBuffer->eReaderVsWriterRateCode > BVDC_P_WrRate_NotFaster) && (pNextNextNode == hWindow->hBuffer->pCurWriterBuf)) ||
           bRepeat) && !hWindow->hBuffer->bMtgMadDisplay1To1RateRelationship) &&
           !bReverseProgressivePulldown)

    {
        /* Repeat reader. We may have to bear a field inversion here if both
         * source and display are interlaced and pictures are not captured as
         * frame.
         */
#if BVDC_P_REPEAT_ALGORITHM_ONE
            hWindow->hBuffer->bRepeatForGap = true;
#endif
            hWindow->hBuffer->ulRepeatStat++;
            hWindow->hBuffer->pCurReaderBuf->stFlags.bPictureRepeatFlag = true;

            hWindow->hBuffer->ulMtgPictureRepeatCount++;


#if (BVDC_BUF_LOG == 1)
            if (!hWindow->bFrameCapture && hWindow->stCurInfo.hSource->bSrcInterlaced &&
                hWindow->hCompositor->stCurInfo.pFmtInfo->bInterlaced)
            {
                 BVDC_P_BufLog_AddEvent_isr('E',
                    hWindow->eId,
                    hWindow->hBuffer->pCurReaderBuf->ulBufferId,
                    hWindow->hBuffer->ulNumCapField << BVDC_P_BUF_LOG_NUM_CAP_FIELDS_SHIFT |
                        (hWindow->hBuffer->ulRepeatStat << BVDC_P_BUF_LOG_PIC_REPEAT_CNT_SHIFT),
                    hWindow->hBuffer->ulCurrReaderTimestamp,
                    0,
                    0,
                    0,
                    hWindow->hBuffer);
            }
            else
            {
                BVDC_P_BufLog_AddEvent_isr('C',
                    hWindow->eId,
                    hWindow->hBuffer->pCurReaderBuf->ulBufferId,
                    hWindow->hBuffer->ulNumCapField << BVDC_P_BUF_LOG_NUM_CAP_FIELDS_SHIFT |
                        (hWindow->hBuffer->ulRepeatStat << BVDC_P_BUF_LOG_PIC_REPEAT_CNT_SHIFT),
                    hWindow->hBuffer->ulCurrReaderTimestamp,
                    0,
                    0,
                    0,
                    hWindow->hBuffer);
            }
#else
            BDBG_MSG(("Win[%d], B[%d] R:      Repeat one field for r-w gap, total %d",
                hWindow->hBuffer->hWindow->eId, hWindow->hBuffer->pCurReaderBuf->ulBufferId,
                hWindow->hBuffer->ulRepeatStat));
#endif
    }
    else
    {
        BAVC_Polarity eDstPolarity;
        bool bMtgAlignSrcWithDisplay;

        pCurNode = hWindow->hBuffer->pCurReaderBuf;

        /* Advance the reader anyway first */
        hWindow->hBuffer->pCurReaderBuf = pNextNode;

        eDstPolarity = hWindow->hBuffer->pCurReaderBuf->eDstPolarity;

        bMtgAlignSrcWithDisplay = (hWindow->hBuffer->bMtgAlignSrcAndDisp &&
            (hWindow->hBuffer->pCurReaderBuf->PicComRulInfo.eSrcOrigPolarity != BAVC_Polarity_eFrame) &&
            (hWindow->hBuffer->pCurReaderBuf->PicComRulInfo.eSrcOrigPolarity != eVecPolarity)) ?
            true : false;

        /* Now we need to decide if reader's polarity satisfies VEC.
         */
        if ((!hWindow->hBuffer->pCurReaderBuf->stFlags.bMute &&
            hWindow->hBuffer->pCurReaderBuf->stFlags.bCadMatching &&
            (eDstPolarity != eVecPolarity)) ||
                (!hWindow->hBuffer->pCurReaderBuf->stFlags.bCadMatching && bMtgAlignSrcWithDisplay))
        {

#if BVDC_P_REPEAT_ALGORITHM_ONE
            /* If polarity mismatch is caused by a reader repeat to avoid catching up writer, then
             * move reader to next field anyway to spread 2 repeats over 4 fields.
             * Bear a field inversion. Reader bvdc_buffer_priv.h for details.
             */
            if ((!hWindow->hBuffer->bRepeatForGap) ||
                ((pNextNode == hWindow->hBuffer->pCurWriterBuf) && (!hWindow->stCurInfo.stGameDelaySetting.bEnable)) ||
                ((hWindow->hBuffer->eReaderVsWriterRateCode > BVDC_P_WrRate_NotFaster) && (pNextNextNode == hWindow->hBuffer->pCurWriterBuf)) ||
                bMtgAlignSrcWithDisplay)
#endif
            {
                hWindow->hBuffer->ulRepeatStat++;

#if (BVDC_BUF_LOG == 1)
                BVDC_P_BufLog_AddEvent_isr('D',
                    hWindow->eId,
                    hWindow->hBuffer->pCurReaderBuf->ulBufferId,
                    hWindow->hBuffer->ulNumCapField << BVDC_P_BUF_LOG_NUM_CAP_FIELDS_SHIFT |
                        (eVecPolarity << BVDC_P_BUF_LOG_VEC_POLARITY_SHIFT) |
                        (hWindow->hBuffer->pCurReaderBuf->PicComRulInfo.eSrcOrigPolarity << BVDC_P_BUF_LOG_ORIG_SRC_POLARITY_SHIFT) |
                        (eDstPolarity << BVDC_P_BUF_LOG_DST_POLARITY_SHIFT),
                    hWindow->hBuffer->ulCurrReaderTimestamp,
                    0,
                    (bMtgAlignSrcWithDisplay ? 1 : 0) << BVDC_P_BUF_LOG_REPEAT_DUE_TO_MTG_ALIGN_SRC_SHIFT,
                    0,
                    hWindow->hBuffer);
#else
                BDBG_MSG(("Win[%d] R:      Repeat for polarity, src I, VEC %d. orig src p %d, cap %d, total %d",
                    hWindow->hBuffer->hWindow->eId, eVecPolarity, hWindow->hBuffer->pCurReaderBuf->PicComRulInfo.eSrcOrigPolarity,
                    eDstPolarity, hWindow->hBuffer->ulRepeatStat));
#endif

                /* Repeat one field so that next field will match VEC polarity.
                 * We bear a field inversion here.
                 */
                /* restore the current reader buffer */
                hWindow->hBuffer->pCurReaderBuf = pCurNode;
                hWindow->hBuffer->pCurReaderBuf->stFlags.bPictureRepeatFlag = true;
                hWindow->hBuffer->ulMtgPictureRepeatCount++;
            }
        }

        /* This is a special case for handling MTG 24Hz sources (3:2 cadence by the deinterlacer or XDM) and displayed at 50Hz.
         * This is needed to prevent a 2:2:2:2:2:2:3:1:3 cadence. The expected cadence is 2: 2:2:2:2:2:3:2:2:2.
         *  See multibuffer analysis.
         */
        if (hWindow->hBuffer->pCurReaderBuf->stFlags.bRev32Locked &&
            hWindow->hCompositor->stCurInfo.pFmtInfo->ulVertFreq == (50 * BFMT_FREQ_FACTOR))

        {
            if (eMtgMode == BVDC_P_Buffer_MtgMode_eMadPhase)
            {
                /* Check if R encroaches W. */
                if (pNextNode == hWindow->hBuffer->pCurWriterBuf)
                {
                    /* Restore the current reader buffer. */
                    hWindow->hBuffer->pCurReaderBuf = pCurNode;
                    hWindow->hBuffer->ulMtgPictureRepeatCount++;

#if (BVDC_BUF_LOG == 1)
                    bForcedRepeat = true;
#endif
                }
                else
                {
                    /* Check if R has repeated at least once */
                    if (hWindow->hBuffer->ulMtgPictureRepeatCount == 0)
                    {
                        /* Restore the current reader buffer. This ensures a repeat. */
                        hWindow->hBuffer->pCurReaderBuf = pCurNode;
                        hWindow->hBuffer->ulMtgPictureRepeatCount++;

#if (BVDC_BUF_LOG == 1)
                        bForcedRepeat = true;
#endif
                    }
                    else
                    {
                        hWindow->hBuffer->ulMtgPictureRepeatCount = 0;
                    }
                }
            }
            else if (eMtgMode == BVDC_P_Buffer_MtgMode_eXdmRepeat)
            {
                if (pNextNode->stFlags.bPictureRepeatFlag)
                {
                    if (hWindow->hBuffer->ulMtgPictureRepeatCount == 0)
                    {
                        if (pCurNode->ulDecodePictureId != pNextNode->ulDecodePictureId)
                        {
                            /* Restore the current reader buffer. This is a repeat. */
                            hWindow->hBuffer->pCurReaderBuf = pCurNode;

#if (BVDC_BUF_LOG == 1)
                            bForcedRepeat = true;
#endif
                        }
                        hWindow->hBuffer->ulMtgPictureRepeatCount++;
                    }
                    else
                    {
                        if (pCurNode->ulDecodePictureId != pNextNode->ulDecodePictureId)
                        {
                            /* New picture */
                            hWindow->hBuffer->ulMtgPictureRepeatCount = 0;
                        }
                        else
                        {
                            hWindow->hBuffer->ulMtgPictureRepeatCount++;
                        }
                    }
                }
                else
                {
                    if (hWindow->hBuffer->ulMtgPictureRepeatCount != 0)
                    {
                        /* New picture */
                        hWindow->hBuffer->ulMtgPictureRepeatCount = 0;
                    }
                    else
                    {
                        if (pCurNode->ulDecodePictureId != pNextNode->ulDecodePictureId)
                        {
                            /* Restore the current reader buffer. This is a repeat. */
                            hWindow->hBuffer->pCurReaderBuf = pCurNode;
                            hWindow->hBuffer->ulMtgPictureRepeatCount++;


#if (BVDC_BUF_LOG == 1)
                            bForcedRepeat = true;
#endif
                        }
                    }
                }


#if (BVDC_BUF_LOG == 1)
                if (bForcedRepeat)
                {
                    BVDC_P_BufLog_AddEvent_isr('C',
                            hWindow->eId,
                            hWindow->hBuffer->pCurReaderBuf->ulBufferId,
                            hWindow->hBuffer->ulNumCapField << BVDC_P_BUF_LOG_NUM_CAP_FIELDS_SHIFT |
                                (hWindow->hBuffer->ulSkipStat << BVDC_P_BUF_LOG_PIC_SKIP_CNT_SHIFT),
                            hWindow->hBuffer->ulCurrReaderTimestamp,
                            0,
                            (bForcedRepeat ? 1 : 0 << BVDC_P_BUF_LOG_FORCED_REPEAT_PIC_SHIFT)|
                            (hWindow->hBuffer->ulMtgPictureRepeatCount << BVDC_P_BUF_LOG_FORCED_REPEAT_PIC_COUNT_SHIFT),
                            hWindow->hBuffer->pCurReaderBuf->ulDecodePictureId,
                            hWindow->hBuffer);
                }
#endif
                }
            }
        }

#if (BVDC_BUF_LOG == 1)
    if (hWindow->hBuffer->pCurReaderBuf == hWindow->hBuffer->pCurWriterBuf)
    {
        BVDC_P_BufLog_AddEvent_isr('G',
                hWindow->eId,
                hWindow->hBuffer->pCurReaderBuf->ulBufferId,
                hWindow->hBuffer->ulNumCapField << BVDC_P_BUF_LOG_NUM_CAP_FIELDS_SHIFT |
                    (hWindow->hBuffer->ulSkipStat << BVDC_P_BUF_LOG_PIC_SKIP_CNT_SHIFT),
                hWindow->hBuffer->ulCurrReaderTimestamp,
                0,
                0,
                0,
                hWindow->hBuffer);
    }
#endif


#if BVDC_P_REPEAT_ALGORITHM_ONE
        hWindow->hBuffer->bRepeatForGap = false;
#endif

done:
    BDBG_LEAVE(BVDC_P_Buffer_MoveSyncSlipReaderNode_isr);
    return ;
}


/***************************************************************************
 *
 */
void BVDC_P_Buffer_CalculateRateGap_isr
    ( const uint32_t         ulSrcVertFreq,
      const uint32_t         ulDispVertFreq,
      BVDC_P_WrRateCode     *peWriterVsReaderRateCode,
      BVDC_P_WrRateCode     *peReaderVsWriterRateCode )
{
    uint32_t    ulDstVertRate, ulSrcVertRate;

    /* 29.97Hz and 30Hz, 59.94Hz and 60Hz are considered
     * the same rate after round off. */
    ulDstVertRate = BVDC_P_ROUND_OFF(ulDispVertFreq,
        (BFMT_FREQ_FACTOR/2), BFMT_FREQ_FACTOR);
    ulSrcVertRate = BVDC_P_ROUND_OFF(ulSrcVertFreq,
        (BFMT_FREQ_FACTOR/2), BFMT_FREQ_FACTOR);

    /* Preset writer and reader rate gap */
    *peWriterVsReaderRateCode = BVDC_P_WrRate_NotFaster;
    *peReaderVsWriterRateCode = BVDC_P_WrRate_Faster;

    if(ulDstVertRate == ulSrcVertRate)
    {
        *peWriterVsReaderRateCode = BVDC_P_WrRate_NotFaster;
        *peReaderVsWriterRateCode = BVDC_P_WrRate_NotFaster;
    }
    else
    {
        if(ulSrcVertRate > ulDstVertRate)
        {
            *peReaderVsWriterRateCode = BVDC_P_WrRate_NotFaster;
            if ((ulSrcVertRate / ulDstVertRate) >= 2)
            {
                *peWriterVsReaderRateCode = ((ulSrcVertRate % ulDstVertRate) > 0) ?
                    BVDC_P_WrRate_MoreThan2TimesFaster : BVDC_P_WrRate_2TimesFaster;
            }
            else
            {
                *peWriterVsReaderRateCode = BVDC_P_WrRate_Faster;
            }
        }
        else
        {
            *peWriterVsReaderRateCode = BVDC_P_WrRate_NotFaster;
            if ((ulDstVertRate / ulSrcVertRate) >= 2)
            {
                *peReaderVsWriterRateCode = ((ulDstVertRate % ulSrcVertRate) > 0) ?
                    BVDC_P_WrRate_MoreThan2TimesFaster : BVDC_P_WrRate_2TimesFaster;
            }
            else
            {
                *peReaderVsWriterRateCode = BVDC_P_WrRate_Faster;
            }
        }
    }
    return;
}

/***************************************************************************
 *
 */
uint32_t BVDC_P_Buffer_CalculateBufDelay_isr
    ( BVDC_P_PictureNode   *pPicture,
      bool                 *pbValidDelay )
{
    if(pbValidDelay)
        *pbValidDelay = pPicture->bValidTimeStampDelay;

    return ((pPicture->ulPlaybackTimestamp < pPicture->ulCaptureTimestamp) ?
            (pPicture->ulPlaybackTimestamp + pPicture->hBuffer->ulMaxTimestamp - pPicture->ulCaptureTimestamp)
            : (pPicture->ulPlaybackTimestamp - pPicture->ulCaptureTimestamp));
}

/***************************************************************************
 *
 */
BVDC_P_PictureNode * BVDC_P_Buffer_GetCurWriterNode_isr
    ( const BVDC_P_Buffer_Handle       hBuffer )
{
    BDBG_OBJECT_ASSERT(hBuffer, BVDC_BUF);
    return (hBuffer->pCurWriterBuf);
}

/***************************************************************************
 *
 */
BVDC_P_PictureNode * BVDC_P_Buffer_GetCurReaderNode_isr
    ( const BVDC_P_Buffer_Handle       hBuffer )
{
    BDBG_OBJECT_ASSERT(hBuffer, BVDC_BUF);
    return (hBuffer->pCurReaderBuf);
}

/* End of file. */

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
 ******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bvdc.h"
#include "bvdc_bufferheap_priv.h"
#include "bvdc_priv.h"
#include "bvdc_displayfmt_priv.h"
#include "bvdc_window_priv.h"


BDBG_MODULE(BVDC_HEAP);
BDBG_FILE_MODULE(BVDC_MEMC_INDEX_CHECK);
BDBG_FILE_MODULE(BVDC_WIN_BUF);
BDBG_OBJECT_ID(BVDC_BFH);

/* TODO: Rework to support generic buffer broken down. */
#define BVDC_P_MEMC_ALIGNMENT  (1)
#define BVDC_P_PITCH_ALIGNMENT (1)
#define BVDC_P_HSIZE_ALIGNMENT (1)

#define BVDC_P_BUFHEAP_SPLIT_RATIO_FACTOR        (100)
#define BVDC_P_BUFHEAP_SPLIT_RATIO_THRESHOLD     (75)

#if (BCHP_CHIP==7271) || (BCHP_CHIP==7268)
/* TODO: Turn it back on temporarily */
#define BVDC_P_HEAP_FULL_SPLIT             (1)
#else
#define BVDC_P_HEAP_FULL_SPLIT             (0)
#endif
#define BVDC_MAX_BUFFERHEAP_NODES          (300)

/***************************************************************************
 *
 */
static void BVDC_P_BufferHeap_DumpHeapNode_isr
    ( const BVDC_P_BufferHeapNode     *pBufferHeapNode )
{
/*#if 1*/
#if (BDBG_DEBUG_BUILD)
    /* uiBufIndex (ucNodeCntPerParent eOrigBufHeapId) (Continous)
     *       (Used - ucNumChildNodeUsed) at ullDeviceOffset (pvBufAddr) */
    BDBG_MSG(("    - Node %p %2d (%s) (%s) (%s - %d/%d) at " BDBG_UINT64_FMT ", block offset 0x%x",
    (void *)pBufferHeapNode,
    pBufferHeapNode->uiBufIndex,
    BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(pBufferHeapNode->eOrigBufHeapId),
    pBufferHeapNode->bContinous? "c" : "n",
    pBufferHeapNode->bUsed ? "x" : " ",
    pBufferHeapNode->ucNumChildNodeUsed, pBufferHeapNode->ucNumChildNodeSplit,
    BDBG_UINT64_ARG(pBufferHeapNode->ullDeviceOffset),
    pBufferHeapNode->ulBlockOffset));
#else
    BSTD_UNUSED(pBufferHeapNode); /* hush warning. */
#endif
    return;
}

/***************************************************************************
 *
 */
static  void BVDC_P_BufferHeap_DumpHeapInfo_isr
    ( const BVDC_P_BufferHeap_Info    *pHeapInfo )
{
    uint16_t i;

    BDBG_MSG((" "));
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("%s Heap",
        BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(pHeapInfo->eBufHeapId)));

    if( pHeapInfo->pParentHeapInfo )
    {
        BDBG_MODULE_MSG(BVDC_WIN_BUF, ("    Parent Heap        = %s",
            BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(pHeapInfo->pParentHeapInfo->eBufHeapId)));
    }
    else
    {
        BDBG_MODULE_MSG(BVDC_WIN_BUF, ("    Parent Heap        = NULL"));
    }

    if( pHeapInfo->pChildHeapInfo )
    {
        BDBG_MODULE_MSG(BVDC_WIN_BUF, ("    Child Heap         = %s",
            BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(pHeapInfo->pChildHeapInfo->eBufHeapId)));
    }
    else
    {
        BDBG_MODULE_MSG(BVDC_WIN_BUF, ("    Child Heap         = NULL"));
    }

    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("    pHeapInfo          = %p:",    (void *)pHeapInfo));
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("    ucNodeCntPerParent = 0x%x:",  pHeapInfo->ucNodeCntPerParent));
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("    uiPrimaryBufCnt    = %d:",    pHeapInfo->uiPrimaryBufCnt));
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("    uiTotalBufCnt      = %d:",    pHeapInfo->uiTotalBufCnt));
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("    ulBufSize          = %d:",    pHeapInfo->ulBufSize));
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("    hMmaBlock          = %p:",    (void *)pHeapInfo->hMmaBlock));
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("    uiBufUsed          = 0x%d:",  pHeapInfo->uiBufUsed));
    BDBG_MSG(("    Node list:"));

    for(i = 0; i < pHeapInfo->uiTotalBufCnt; i++)
    {
        BVDC_P_BufferHeap_DumpHeapNode_isr(&(pHeapInfo->pBufList[i]));
    }

    return;
}

/***************************************************************************
 *
 */
static void BVDC_P_BufferHeap_Dump_isr
    ( const BVDC_P_BufferHeapContext  *pBufferHeap )
{
    uint32_t i;

    for( i = 0; i < BVDC_P_BufferHeapId_eCount; i++ )
    {
        BVDC_P_BufferHeap_DumpHeapInfo_isr(&pBufferHeap->astHeapInfo[i]);
    }

    return;
}

/***************************************************************************
 *
 */
uint32_t BVDC_P_BufferHeap_GetHeapSize
    ( const BFMT_VideoInfo       *pHeapFmtInfo,
      BPXL_Format                 ePixelFmt,
      bool                        bPip,
      uint32_t                   *pulUnalignedBufSize,
      uint32_t                   *pulWidth,
      uint32_t                   *pulHeight )
{
    unsigned int uiPitch;
    uint32_t  ulWidth, ulHeight, ulBufSize, ulUnalignedBufSize;

    /* Get width and Height */
    ulWidth   = pHeapFmtInfo->ulWidth;
    ulHeight  = (pHeapFmtInfo->bInterlaced) ?
        ((pHeapFmtInfo->ulHeight + 1)/ 2) : (pHeapFmtInfo->ulHeight);

    if( bPip )
    {
        ulWidth  /= 2;
        ulHeight /= 2;
    }

    /* Get buf size wo alignment */
    /* Get pitch */
    BPXL_GetBytesPerNPixels(ePixelFmt, ulWidth, &uiPitch);
    ulUnalignedBufSize = uiPitch * ulHeight;

    /* Here starts alignment */
#if BVDC_P_HSIZE_ALIGNMENT
#if (BVDC_P_MADR_HSIZE_WORKAROUND)
    /* the "+ 1" is for potential cap left/top align down, and
     * the 4 is for DCX_HSIZE_WORKAROUND */
    ulWidth  = BVDC_P_ALIGN_UP(ulWidth  + 1, 4);
    ulHeight = BVDC_P_ALIGN_UP(ulHeight + 1, 2);
#endif
#endif

    /* Get pitch */
    BPXL_GetBytesPerNPixels(ePixelFmt, ulWidth, &uiPitch);

#if (BVDC_P_BUFFER_ADD_GUARD_MEMORY)
    /* TODO: Optimize */
    uiPitch += BVDC_P_MAX(BVDC_P_MAX_CAP_GUARD_MEMORY_3D,
        BVDC_P_MAX_CAP_GUARD_MEMORY_2D);
#endif

    /* Make sure 32 byte aligned */
#if BVDC_P_PITCH_ALIGNMENT
    uiPitch = BVDC_P_ALIGN_UP(uiPitch, BVDC_P_PITCH_ALIGN);
#endif

    /* See SW7445-2936 */
#if (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_CAP_DCXM)
    ulHeight += BVDC_P_DCXM_BUFFERHEAP_INCREASE_WORKAROUND;
#endif

    ulBufSize = uiPitch * ulHeight;

#if (BVDC_P_MVFD_ALIGNMENT_WORKAROUND)
    ulBufSize += 4;
#endif

    /* Memc alignment requirement */
#if BVDC_P_MEMC_ALIGNMENT
    ulBufSize = BVDC_P_ALIGN_UP(ulBufSize, BVDC_P_HEAP_ALIGN_BYTES);
#endif

    if(pulWidth)
    {
        *pulWidth = ulWidth;
    }

    if(pulHeight)
    {
        *pulHeight = ulHeight;
    }

    if(pulUnalignedBufSize)
    {
        *pulUnalignedBufSize = ulUnalignedBufSize;
    }

    return ulBufSize;
}

/***************************************************************************
 * Fill out const properties:
 *    eBufHeapId, ulBufSize, uiPrimaryBufCnt, pParentHeapInfo,
 *    ucNodeCntPerParent
 */
void BVDC_P_BufferHeap_GetHeapOrder
    ( const BVDC_Heap_Settings      *pSettings,
      BVDC_P_BufferHeap_SizeInfo    *pHeapSizeInfo,
      BVDC_P_BufferHeapContext      *pBufferHeap )
{
    uint32_t i = 0;
    uint32_t   ulNewBufSize;

    uint32_t ulSDBufSize, ulSDBufWidth, ulSDBufHeight;
    uint32_t ulHDBufSize, ulHDBufWidth, ulHDBufHeight;
    uint32_t ul2HDBufSize, ul2HDBufWidth, ul2HDBufHeight;
    uint32_t ul4HDBufSize, ul4HDBufWidth, ul4HDBufHeight;

    uint32_t ulSDPipBufSize, ulSDPipBufWidth, ulSDPipBufHeight;
    uint32_t ulHDPipBufSize, ulHDPipBufWidth, ulHDPipBufHeight;
    uint32_t ul2HDPipBufSize, ul2HDPipBufWidth, ul2HDPipBufHeight;
    uint32_t ul4HDPipBufSize, ul4HDPipBufWidth, ul4HDPipBufHeight;

    uint32_t ulUnalignedSDBufSize, ulUnalignedHDBufSize;
    uint32_t ulUnaligned2HDBufSize, ulUnaligned4HDBufSize;
    uint32_t ulUnalignedSDPipBufSize, ulUnalignedHDPipBufSize;
    uint32_t ulUnaligned2HDPipBufSize, ulUnaligned4HDPipBufSize;
    uint32_t  aulUnAlignedBufSize[BVDC_P_BufferHeapId_eCount];
    uint8_t   aucNodeCntPerParent[BVDC_P_BufferHeapId_eCount];

    /* Heap settings */
    const BFMT_VideoInfo *pSDFmt = BFMT_GetVideoFormatInfoPtr(pSettings->eBufferFormat_SD);
    const BFMT_VideoInfo *pHDFmt = BFMT_GetVideoFormatInfoPtr(pSettings->eBufferFormat_HD);
    const BFMT_VideoInfo *p2HDFmt = BFMT_GetVideoFormatInfoPtr(pSettings->eBufferFormat_2HD);
    const BFMT_VideoInfo *p4HDFmt = BFMT_GetVideoFormatInfoPtr(pSettings->eBufferFormat_4HD);

    /* Get buffer size */
    ulSDBufSize = BVDC_P_BufferHeap_GetHeapSize(pSDFmt, pSettings->ePixelFormat_SD,
        false, &ulUnalignedSDBufSize,
        &ulSDBufWidth, &ulSDBufHeight);
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("SD      buffer size = %d (%dx%d)", ulSDBufSize, ulSDBufWidth, ulSDBufHeight));

    ulHDBufSize = BVDC_P_BufferHeap_GetHeapSize(pHDFmt, pSettings->ePixelFormat_HD,
        false, &ulUnalignedHDBufSize,
        &ulHDBufWidth, &ulHDBufHeight);
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("HD      buffer size = %d (%dx%d)", ulHDBufSize, ulHDBufWidth, ulHDBufHeight));

    ul2HDBufSize = BVDC_P_BufferHeap_GetHeapSize(p2HDFmt, pSettings->ePixelFormat_2HD,
        false, &ulUnaligned2HDBufSize,
        &ul2HDBufWidth, &ul2HDBufHeight);
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("2HD     buffer size = %d (%dx%d)", ul2HDBufSize, ul2HDBufWidth, ul2HDBufHeight));

    ul4HDBufSize = BVDC_P_BufferHeap_GetHeapSize(p4HDFmt, pSettings->ePixelFormat_4HD,
        false, &ulUnaligned4HDBufSize,
        &ul4HDBufWidth, &ul4HDBufHeight);
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("4HD     buffer size = %d (%dx%d)", ul4HDBufSize, ul4HDBufWidth, ul4HDBufHeight));

    ulSDPipBufSize = BVDC_P_BufferHeap_GetHeapSize(pSDFmt, pSettings->ePixelFormat_SD,
        true, &ulUnalignedSDPipBufSize, &ulSDPipBufWidth, &ulSDPipBufHeight);
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("SD_PIP  buffer size = %d (%dx%d)", ulSDPipBufSize, ulSDPipBufWidth, ulSDPipBufHeight));

    ulHDPipBufSize = BVDC_P_BufferHeap_GetHeapSize(pHDFmt, pSettings->ePixelFormat_HD,
        true, &ulUnalignedHDPipBufSize, &ulHDPipBufWidth, &ulHDPipBufHeight);
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("HD_PIP  buffer size = %d (%dx%d)", ulHDPipBufSize, ulHDPipBufWidth, ulHDPipBufHeight));

    ul2HDPipBufSize = BVDC_P_BufferHeap_GetHeapSize(p2HDFmt, pSettings->ePixelFormat_2HD,
        true, &ulUnaligned2HDPipBufSize, &ul2HDPipBufWidth, &ul2HDPipBufHeight);
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("2HD_PIP buffer size = %d (%dx%d)", ul2HDPipBufSize, ul2HDPipBufWidth, ul2HDPipBufHeight));

    ul4HDPipBufSize = BVDC_P_BufferHeap_GetHeapSize(p4HDFmt, pSettings->ePixelFormat_4HD,
        true, &ulUnaligned4HDPipBufSize, &ul4HDPipBufWidth, &ul4HDPipBufHeight);
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("4HD_PIP buffer size = %d (%dx%d)", ul4HDPipBufSize, ul4HDPipBufWidth, ul4HDPipBufHeight));

    /* Make sure SD_Pip  < SD, HD_Pip  < HD and 2HD_Pip < 2HD */
    BDBG_ASSERT(ulSDPipBufSize  < ulSDBufSize);
    BDBG_ASSERT(ulHDPipBufSize  < ulHDBufSize);
    BDBG_ASSERT(ul2HDPipBufSize < ul2HDBufSize);
    BDBG_ASSERT(ul4HDPipBufSize < ul4HDBufSize);

    /* Sort the order of buffers */
    if(pBufferHeap)
    {
        pBufferHeap->astHeapInfo[i].eBufHeapId   = BVDC_P_BufferHeapId_e4HD;
        pBufferHeap->astHeapInfo[i].uiPrimaryBufCnt = pSettings->ulBufferCnt_4HD;
        pBufferHeap->astHeapInfo[i].ulWidth      = ul4HDBufWidth;
        pBufferHeap->astHeapInfo[i].ulHeight     = ul4HDBufHeight;
        pBufferHeap->astHeapInfo[i].ulBufSize    = ul4HDBufSize;
        pBufferHeap->astHeapInfo[i].ulBufSizeByFmt = ulUnaligned4HDBufSize;
    }
    aulUnAlignedBufSize[i] = ulUnaligned4HDBufSize;
    pHeapSizeInfo->aeBufHeapId[i] = BVDC_P_BufferHeapId_e4HD;
    pHeapSizeInfo->aulBufSize[i] = ul4HDBufSize;
    pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e4HD] = i++;

    if( ul2HDBufSize > ul4HDPipBufSize )
    {
        /* 4HD > 2HD > 4HD_Pip */
        if(pBufferHeap)
        {
            pBufferHeap->astHeapInfo[i].eBufHeapId   = BVDC_P_BufferHeapId_e2HD;
            pBufferHeap->astHeapInfo[i].uiPrimaryBufCnt = pSettings->ulBufferCnt_2HD;
            pBufferHeap->astHeapInfo[i].ulWidth      = ul2HDBufWidth;
            pBufferHeap->astHeapInfo[i].ulHeight     = ul2HDBufHeight;
            pBufferHeap->astHeapInfo[i].ulBufSize    = ul2HDBufSize;
            pBufferHeap->astHeapInfo[i].ulBufSizeByFmt = ulUnaligned2HDBufSize;
        }
        aulUnAlignedBufSize[i] = ulUnaligned2HDBufSize;
        pHeapSizeInfo->aeBufHeapId[i] = BVDC_P_BufferHeapId_e2HD;
        pHeapSizeInfo->aulBufSize[i] = ul2HDBufSize;
        pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e2HD] = i++;

        if(pBufferHeap)
        {
            pBufferHeap->astHeapInfo[i].eBufHeapId   = BVDC_P_BufferHeapId_e4HD_Pip;
            pBufferHeap->astHeapInfo[i].uiPrimaryBufCnt = pSettings->ulBufferCnt_4HD_Pip;
            pBufferHeap->astHeapInfo[i].ulWidth      = ul4HDPipBufWidth;
            pBufferHeap->astHeapInfo[i].ulHeight     = ul4HDPipBufHeight;
            pBufferHeap->astHeapInfo[i].ulBufSize    = ul4HDPipBufSize;
            pBufferHeap->astHeapInfo[i].ulBufSizeByFmt = ulUnaligned4HDPipBufSize;
        }
        aulUnAlignedBufSize[i] = ulUnaligned4HDPipBufSize;
        pHeapSizeInfo->aeBufHeapId[i] = BVDC_P_BufferHeapId_e4HD_Pip;
        pHeapSizeInfo->aulBufSize[i] = ul4HDPipBufSize;
        pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e4HD_Pip] = i++;
    }
    else
    {
        /* 4HD > 4HD_Pip > 2HD */
        if(pBufferHeap)
        {
            pBufferHeap->astHeapInfo[i].eBufHeapId   = BVDC_P_BufferHeapId_e4HD_Pip;
            pBufferHeap->astHeapInfo[i].uiPrimaryBufCnt = pSettings->ulBufferCnt_4HD_Pip;
            pBufferHeap->astHeapInfo[i].ulWidth      = ul4HDPipBufWidth;
            pBufferHeap->astHeapInfo[i].ulHeight     = ul4HDPipBufHeight;
            pBufferHeap->astHeapInfo[i].ulBufSize    = ul4HDPipBufSize;
            pBufferHeap->astHeapInfo[i].ulBufSizeByFmt = ulUnaligned4HDPipBufSize;
        }
        aulUnAlignedBufSize[i] = ulUnaligned4HDPipBufSize;
        pHeapSizeInfo->aeBufHeapId[i] = BVDC_P_BufferHeapId_e4HD_Pip;
        pHeapSizeInfo->aulBufSize[i] = ul4HDPipBufSize;
        pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e4HD_Pip] = i++;

        if(pBufferHeap)
        {
            pBufferHeap->astHeapInfo[i].eBufHeapId   = BVDC_P_BufferHeapId_e2HD;
            pBufferHeap->astHeapInfo[i].uiPrimaryBufCnt = pSettings->ulBufferCnt_2HD;
            pBufferHeap->astHeapInfo[i].ulWidth      = ul2HDBufWidth;
            pBufferHeap->astHeapInfo[i].ulHeight     = ul2HDBufHeight;
            pBufferHeap->astHeapInfo[i].ulBufSize    = ul2HDBufSize;
            pBufferHeap->astHeapInfo[i].ulBufSizeByFmt = ulUnaligned2HDBufSize;
        }
        aulUnAlignedBufSize[i] = ulUnaligned2HDBufSize;
        pHeapSizeInfo->aeBufHeapId[i] = BVDC_P_BufferHeapId_e2HD;
        pHeapSizeInfo->aulBufSize[i] = ul2HDBufSize;
        pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e2HD] = i++;
    }

    if( ulHDBufSize > ul2HDPipBufSize )
    {
        /* 2HD > HD > 2HD_Pip */
        if(pBufferHeap)
        {
            pBufferHeap->astHeapInfo[i].eBufHeapId   = BVDC_P_BufferHeapId_eHD;
            pBufferHeap->astHeapInfo[i].uiPrimaryBufCnt = pSettings->ulBufferCnt_HD;
            pBufferHeap->astHeapInfo[i].ulWidth      = ulHDBufWidth;
            pBufferHeap->astHeapInfo[i].ulHeight     = ulHDBufHeight;
            pBufferHeap->astHeapInfo[i].ulBufSize    = ulHDBufSize;
            pBufferHeap->astHeapInfo[i].ulBufSizeByFmt = ulUnalignedHDBufSize;
        }
        aulUnAlignedBufSize[i] = ulUnalignedHDBufSize;
        pHeapSizeInfo->aeBufHeapId[i] = BVDC_P_BufferHeapId_eHD;
        pHeapSizeInfo->aulBufSize[i] = ulHDBufSize;
        pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eHD] = i++;

        if(pBufferHeap)
        {
            pBufferHeap->astHeapInfo[i].eBufHeapId   = BVDC_P_BufferHeapId_e2HD_Pip;
            pBufferHeap->astHeapInfo[i].uiPrimaryBufCnt = pSettings->ulBufferCnt_2HD_Pip;
            pBufferHeap->astHeapInfo[i].ulWidth      = ul2HDPipBufWidth;
            pBufferHeap->astHeapInfo[i].ulHeight     = ul2HDPipBufHeight;
            pBufferHeap->astHeapInfo[i].ulBufSize    = ul2HDPipBufSize;
            pBufferHeap->astHeapInfo[i].ulBufSizeByFmt = ulUnaligned2HDPipBufSize;
        }
        aulUnAlignedBufSize[i] = ulUnaligned2HDPipBufSize;
        pHeapSizeInfo->aeBufHeapId[i] = BVDC_P_BufferHeapId_e2HD_Pip;
        pHeapSizeInfo->aulBufSize[i] = ul2HDPipBufSize;
        pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e2HD_Pip] = i++;
    }
    else
    {
        /* 2HD > 2HD_Pip > HD */
        if(pBufferHeap)
        {
            pBufferHeap->astHeapInfo[i].eBufHeapId   = BVDC_P_BufferHeapId_e2HD_Pip;
            pBufferHeap->astHeapInfo[i].uiPrimaryBufCnt = pSettings->ulBufferCnt_2HD_Pip;
            pBufferHeap->astHeapInfo[i].ulWidth      = ul2HDPipBufWidth;
            pBufferHeap->astHeapInfo[i].ulHeight     = ul2HDPipBufHeight;
            pBufferHeap->astHeapInfo[i].ulBufSize    = ul2HDPipBufSize;
            pBufferHeap->astHeapInfo[i].ulBufSizeByFmt = ulUnaligned2HDPipBufSize;
        }
        aulUnAlignedBufSize[i] = ulUnaligned2HDPipBufSize;
        pHeapSizeInfo->aeBufHeapId[i] = BVDC_P_BufferHeapId_e2HD_Pip;
        pHeapSizeInfo->aulBufSize[i] = ul2HDPipBufSize;
        pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e2HD_Pip] = i++;

        if(pBufferHeap)
        {
            pBufferHeap->astHeapInfo[i].eBufHeapId   = BVDC_P_BufferHeapId_eHD;
            pBufferHeap->astHeapInfo[i].uiPrimaryBufCnt = pSettings->ulBufferCnt_HD;
            pBufferHeap->astHeapInfo[i].ulWidth      = ulHDBufWidth;
            pBufferHeap->astHeapInfo[i].ulHeight     = ulHDBufHeight;
            pBufferHeap->astHeapInfo[i].ulBufSize    = ulHDBufSize;
            pBufferHeap->astHeapInfo[i].ulBufSizeByFmt = ulUnalignedHDBufSize;
        }
        aulUnAlignedBufSize[i] = ulUnalignedHDBufSize;
        pHeapSizeInfo->aeBufHeapId[i] = BVDC_P_BufferHeapId_eHD;
        pHeapSizeInfo->aulBufSize[i] = ulHDBufSize;
        pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eHD] = i++;
    }

    if( ulSDBufSize > ulHDPipBufSize )
    {
        /* SD > HD_Pip > SD_Pip */
        if(pBufferHeap)
        {
            pBufferHeap->astHeapInfo[i].eBufHeapId   = BVDC_P_BufferHeapId_eSD;
            pBufferHeap->astHeapInfo[i].uiPrimaryBufCnt = pSettings->ulBufferCnt_SD;
            pBufferHeap->astHeapInfo[i].ulWidth      = ulSDBufWidth;
            pBufferHeap->astHeapInfo[i].ulHeight     = ulSDBufHeight;
            pBufferHeap->astHeapInfo[i].ulBufSize    = ulSDBufSize;
            pBufferHeap->astHeapInfo[i].ulBufSizeByFmt = ulUnalignedSDBufSize;
        }
        aulUnAlignedBufSize[i] = ulUnalignedSDBufSize;
        pHeapSizeInfo->aeBufHeapId[i] = BVDC_P_BufferHeapId_eSD;
        pHeapSizeInfo->aulBufSize[i] = ulSDBufSize;
        pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eSD] = i++;

        if(pBufferHeap)
        {
            pBufferHeap->astHeapInfo[i].eBufHeapId   = BVDC_P_BufferHeapId_eHD_Pip;
            pBufferHeap->astHeapInfo[i].uiPrimaryBufCnt = pSettings->ulBufferCnt_HD_Pip;
            pBufferHeap->astHeapInfo[i].ulWidth      = ulHDPipBufWidth;
            pBufferHeap->astHeapInfo[i].ulHeight     = ulHDPipBufHeight;
            pBufferHeap->astHeapInfo[i].ulBufSize    = ulHDPipBufSize;
            pBufferHeap->astHeapInfo[i].ulBufSizeByFmt = ulUnalignedHDPipBufSize;
        }
        aulUnAlignedBufSize[i] = ulUnalignedHDPipBufSize;
        pHeapSizeInfo->aeBufHeapId[i] = BVDC_P_BufferHeapId_eHD_Pip;
        pHeapSizeInfo->aulBufSize[i] = ulHDPipBufSize;
        pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eHD_Pip] = i++;
    }
    else
    {
        /* HD_Pip > SD > SD_Pip */
        if(pBufferHeap)
        {
            pBufferHeap->astHeapInfo[i].eBufHeapId   = BVDC_P_BufferHeapId_eHD_Pip;
            pBufferHeap->astHeapInfo[i].uiPrimaryBufCnt = pSettings->ulBufferCnt_HD_Pip;
            pBufferHeap->astHeapInfo[i].ulWidth      = ulHDPipBufWidth;
            pBufferHeap->astHeapInfo[i].ulHeight     = ulHDPipBufHeight;
            pBufferHeap->astHeapInfo[i].ulBufSize    = ulHDPipBufSize;
            pBufferHeap->astHeapInfo[i].ulBufSizeByFmt = ulUnalignedHDPipBufSize;
        }
        aulUnAlignedBufSize[i] = ulUnalignedHDPipBufSize;
        pHeapSizeInfo->aeBufHeapId[i] = BVDC_P_BufferHeapId_eHD_Pip;
        pHeapSizeInfo->aulBufSize[i] = ulHDPipBufSize;
        pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eHD_Pip] = i++;

        if(pBufferHeap)
        {
            pBufferHeap->astHeapInfo[i].eBufHeapId   = BVDC_P_BufferHeapId_eSD;
            pBufferHeap->astHeapInfo[i].uiPrimaryBufCnt = pSettings->ulBufferCnt_SD;
            pBufferHeap->astHeapInfo[i].ulWidth      = ulSDBufWidth;
            pBufferHeap->astHeapInfo[i].ulHeight     = ulSDBufHeight;
            pBufferHeap->astHeapInfo[i].ulBufSize    = ulSDBufSize;
            pBufferHeap->astHeapInfo[i].ulBufSizeByFmt = ulUnalignedSDBufSize;
        }
        aulUnAlignedBufSize[i] = ulUnalignedSDBufSize;
        pHeapSizeInfo->aeBufHeapId[i] = BVDC_P_BufferHeapId_eSD;
        pHeapSizeInfo->aulBufSize[i] = ulSDBufSize;
        pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eSD] = i++;
    }

    if(pBufferHeap)
    {
        pBufferHeap->astHeapInfo[i].eBufHeapId   = BVDC_P_BufferHeapId_eSD_Pip;
        pBufferHeap->astHeapInfo[i].uiPrimaryBufCnt = pSettings->ulBufferCnt_SD_Pip;
        pBufferHeap->astHeapInfo[i].ulWidth      = ulSDPipBufWidth;
        pBufferHeap->astHeapInfo[i].ulHeight     = ulSDPipBufHeight;
        pBufferHeap->astHeapInfo[i].ulBufSize    = ulSDPipBufSize;
        pBufferHeap->astHeapInfo[i].ulBufSizeByFmt = ulUnalignedSDPipBufSize;
    }
    aulUnAlignedBufSize[i] = ulUnalignedSDPipBufSize;
    pHeapSizeInfo->aeBufHeapId[i] = BVDC_P_BufferHeapId_eSD_Pip;
    pHeapSizeInfo->aulBufSize[i] = ulSDPipBufSize;
    pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eSD_Pip] = i++;

    for( i = 0; i < BVDC_P_BufferHeapId_eCount; i++ )
    {
        /* Initalize settings */
        if(pBufferHeap)
        {
            pBufferHeap->astHeapInfo[i].uiBufUsed     = 0;
            pBufferHeap->astHeapInfo[i].hBufferHeap   = pBufferHeap;
            pBufferHeap->astHeapInfo[i].uiTotalBufCnt = pBufferHeap->astHeapInfo[i].uiPrimaryBufCnt;
        }

        if( i == 0 )
        {
            aucNodeCntPerParent[i] = 0;
            if(pBufferHeap)
            {
                pBufferHeap->astHeapInfo[i].pParentHeapInfo    = NULL;
                pBufferHeap->astHeapInfo[i].ucNodeCntPerParent = 0;
                BDBG_ASSERT(aucNodeCntPerParent[i] == pBufferHeap->astHeapInfo[i].ucNodeCntPerParent);
            }
        }
        else
        {
            aucNodeCntPerParent[i] = BVDC_P_MAX(
                pHeapSizeInfo->aulBufSize[i-1] / pHeapSizeInfo->aulBufSize[i],
                aulUnAlignedBufSize[i-1] / aulUnAlignedBufSize[i]);
            if(pBufferHeap)
            {
                pBufferHeap->astHeapInfo[i].pParentHeapInfo
                    = &pBufferHeap->astHeapInfo[i-1];
                pBufferHeap->astHeapInfo[i].ucNodeCntPerParent
                    = BVDC_P_MAX(
                    pBufferHeap->astHeapInfo[i-1].ulBufSize / pBufferHeap->astHeapInfo[i].ulBufSize,
                    pBufferHeap->astHeapInfo[i-1].ulBufSizeByFmt / pBufferHeap->astHeapInfo[i].ulBufSizeByFmt);
                BDBG_ASSERT(aucNodeCntPerParent[i] == pBufferHeap->astHeapInfo[i].ucNodeCntPerParent);
            }
        }

        if(pBufferHeap)
        {
            if( i == BVDC_P_BufferHeapId_eCount - 1 )
            {
                pBufferHeap->astHeapInfo[i].pChildHeapInfo = NULL;
            }
            else
            {
                pBufferHeap->astHeapInfo[i].pChildHeapInfo = &pBufferHeap->astHeapInfo[i+1];
            }
        }
    }

#if (BVDC_P_BUFFER_ADD_GUARD_MEMORY)
    if(pBufferHeap)
    {
        uint32_t   ulIntNum, ulFrac;
        /* recalculate ucNodeCntPerParent */
        for( i = 1; i < BVDC_P_BufferHeapId_eCount; i++ )
        {
            ulIntNum =
                pBufferHeap->astHeapInfo[i-1].ulBufSize / pBufferHeap->astHeapInfo[i].ulBufSize;
            ulFrac =
                pBufferHeap->astHeapInfo[i-1].ulBufSize % pBufferHeap->astHeapInfo[i].ulBufSize;
            ulFrac =
                (ulFrac * BVDC_P_BUFHEAP_SPLIT_RATIO_FACTOR) / pBufferHeap->astHeapInfo[i].ulBufSize;

            if(ulFrac > BVDC_P_BUFHEAP_SPLIT_RATIO_THRESHOLD)
            {
                ulIntNum++;
            }
            aucNodeCntPerParent[i] = ulIntNum;
            pBufferHeap->astHeapInfo[i].ucNodeCntPerParent = ulIntNum;
        }
    }
#endif

    /* Adjust buf size for alignment: from bottom up */
    for( i = BVDC_P_BufferHeapId_eCount - 1; i > 0; i-- )
    {
        ulNewBufSize = aucNodeCntPerParent[i] * pHeapSizeInfo->aulBufSize[i];

#if BVDC_P_MEMC_ALIGNMENT
        ulNewBufSize = BVDC_P_ALIGN_UP(ulNewBufSize, BVDC_P_HEAP_ALIGN_BYTES);
#endif
        pHeapSizeInfo->aulBufSize[i-1] = BVDC_P_MAX(ulNewBufSize,
            pHeapSizeInfo->aulBufSize[i-1]);

        if(pBufferHeap && pBufferHeap->astHeapInfo[i].pParentHeapInfo)
        {
            /* TODO: adjust width and height? ulWidth and ulHeigh are only
             * used in bvdc_bufferheap_priv.c, not need to adjust now */
            pBufferHeap->astHeapInfo[i].pParentHeapInfo->ulBufSize =
                pHeapSizeInfo->aulBufSize[i-1];

            BDBG_MODULE_MSG(BVDC_WIN_BUF, ("%7s buffer size with alignment = %d",
                BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(pBufferHeap->astHeapInfo[i].pParentHeapInfo->eBufHeapId),
                pBufferHeap->astHeapInfo[i].pParentHeapInfo->ulBufSize));
        }
    }

    return;
}

/***************************************************************************
 *
 */
static BERR_Code BVDC_P_BufferHeap_BuildPrimaryNodeList
    ( BMMA_Heap_Handle             hMemory,
      BVDC_P_BufferHeap_Info      *pHeapInfo )
{
    uint16_t               i;
    uint32_t               ulBlockOffset = 0;
    BMMA_DeviceOffset      ullDeviceOffset = 0;
    uint32_t               ulBufSize = pHeapInfo->ulBufSize;
    uint16_t               uiPrimaryBufCnt = pHeapInfo->uiPrimaryBufCnt;
    BERR_Code              err = BERR_SUCCESS;
    BVDC_P_BufferHeapNode *pBufferHeapNode;

    BSTD_UNUSED(hMemory);

    if (uiPrimaryBufCnt)
    {
        ullDeviceOffset = BMMA_LockOffset(pHeapInfo->hMmaBlock);
    }

    for(i = 0; i < uiPrimaryBufCnt; i++)
    {
        pBufferHeapNode = &(pHeapInfo->pBufList[i]);

        /* Initialize fields. */
        pBufferHeapNode->uiBufIndex   = i;
        pBufferHeapNode->pHeapInfo    = pHeapInfo;
        pBufferHeapNode->bUsed        = false;
        pBufferHeapNode->ullDeviceOffset = ullDeviceOffset;
        pBufferHeapNode->ulBlockOffset  = ulBlockOffset;
        pBufferHeapNode->ucNumChildNodeUsed = 0;
        pBufferHeapNode->ucNumChildNodeSplit = 0;
        pBufferHeapNode->bContinous = true;
        pBufferHeapNode->eOrigBufHeapId = pHeapInfo->eBufHeapId;
        pBufferHeapNode->uiParentNodeBufIndex = BVDC_P_HEAP_INVAID_BUF_INDEX;
        pBufferHeapNode->uiFirstChildNodeBufIndex = BVDC_P_HEAP_INVAID_BUF_INDEX;

        ullDeviceOffset += ulBufSize;
        ulBlockOffset += ulBufSize;
    }

    if(uiPrimaryBufCnt && !pHeapInfo->hBufferHeap->ullMosaicDrainOffset)
    {
        pHeapInfo->hBufferHeap->ullMosaicDrainOffset = ullDeviceOffset;
    }

    return err;
}

/***************************************************************************
 *
 */
static BERR_Code BVDC_P_BufferHeap_BuildChildNodeList
    ( BMMA_Heap_Handle             hMemory,
      BVDC_P_BufferHeap_Info      *pHeapInfo )
{
#if (!BVDC_P_HEAP_FULL_SPLIT)
    bool                    bLimitSplt = false;
    uint32_t                aulNumNodesSplit[BVDC_P_BufferHeapId_eCount];
    uint32_t                aulMaxParentNodes2Split[BVDC_P_BufferHeapId_eCount];
#endif
    uint32_t                i, j;
    uint32_t                ulBufSize, ulBlockOffset;
    BMMA_DeviceOffset       ullDeviceOffset;
    BVDC_P_BufferHeapNode  *pBufferHeapNode, *pParentHeapNode;
    BVDC_P_BufferHeap_Info *pParentHeapInfo = pHeapInfo->pParentHeapInfo;
    BSTD_UNUSED(hMemory);

    if( !pParentHeapInfo )
    {
        return BERR_SUCCESS;
    }

    ulBufSize = pHeapInfo->ulBufSize;

#if (!BVDC_P_HEAP_FULL_SPLIT)
    if((pParentHeapInfo->uiTotalBufCnt * pHeapInfo->ucNodeCntPerParent)
        > BVDC_MAX_BUFFERHEAP_NODES)
    {
        uint32_t  ulSplitCnt = 1;
        BVDC_P_BufferHeap_Info  *pTempHeap = pHeapInfo;
        BVDC_P_BufferHeap_Info  *pParentHeap = pHeapInfo->pParentHeapInfo;

        bLimitSplt = true;
        aulNumNodesSplit[pHeapInfo->eBufHeapId] = 0;
        while(pParentHeap)
        {
            aulNumNodesSplit[pParentHeap->eBufHeapId] = 0;
            ulSplitCnt *= pTempHeap->ucNodeCntPerParent;
            aulMaxParentNodes2Split[pParentHeap->eBufHeapId] =
                (BVDC_MAX_BUFFERHEAP_NODES + ulSplitCnt - 1) / ulSplitCnt;
            aulMaxParentNodes2Split[pParentHeap->eBufHeapId] *=
                (ulSplitCnt/pHeapInfo->ucNodeCntPerParent);

            pTempHeap = pParentHeap;
            pParentHeap = pParentHeap->pParentHeapInfo;
        }
    }
#endif

    for(j = 0; j < pParentHeapInfo->uiTotalBufCnt; j++)
    {
        /* Break a larger buffer into smaller buffers */
        pParentHeapNode = &(pParentHeapInfo->pBufList[j]);
        ullDeviceOffset = pParentHeapNode->ullDeviceOffset;
        ulBlockOffset = 0;

        for(i = 0; i < pHeapInfo->ucNodeCntPerParent; i++ )
        {
#if (!BVDC_P_HEAP_FULL_SPLIT)
            if(bLimitSplt)
            {
                aulNumNodesSplit[pParentHeapNode->eOrigBufHeapId]++;
                if(aulNumNodesSplit[pParentHeapNode->eOrigBufHeapId] >
                    aulMaxParentNodes2Split[pParentHeapNode->eOrigBufHeapId])
                {
                    break;
                }
            }
#endif

            /* Initialize fields. */
            pBufferHeapNode = &(pHeapInfo->pBufList[pHeapInfo->uiTotalBufCnt]);

            pBufferHeapNode->uiBufIndex   = pHeapInfo->uiTotalBufCnt;
            pBufferHeapNode->pHeapInfo    = pHeapInfo;
            pBufferHeapNode->bUsed        = false;
            pBufferHeapNode->ucNumChildNodeUsed = 0;
            pBufferHeapNode->ucNumChildNodeSplit = 0;
            pBufferHeapNode->ullDeviceOffset = ullDeviceOffset;
            pBufferHeapNode->ulBlockOffset = ulBlockOffset;
            pBufferHeapNode->uiParentNodeBufIndex = pParentHeapNode->uiBufIndex;
            pBufferHeapNode->eOrigBufHeapId = pParentHeapNode->eOrigBufHeapId;

            if( i == 0 )
            {
                pParentHeapNode->uiFirstChildNodeBufIndex = pBufferHeapNode->uiBufIndex;
                pBufferHeapNode->bContinous = pParentHeapNode->bContinous;
                if( (pHeapInfo->uiPrimaryBufCnt != 0) &&
                    (pBufferHeapNode->uiBufIndex == pHeapInfo->uiPrimaryBufCnt) )
                {
                    pBufferHeapNode->bContinous = false;
                }
            }
            else
            {
                pBufferHeapNode->bContinous = true;
            }
            pParentHeapNode->ucNumChildNodeSplit++;

            ullDeviceOffset += ulBufSize;
            ulBlockOffset += ulBufSize;

            pHeapInfo->uiTotalBufCnt++;
        }
    }

    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
static BERR_Code BVDC_P_BufferHeap_CreateHeapInfo
    ( BMMA_Heap_Handle             hMemory,
      BVDC_P_BufferHeap_Info      *pHeapInfo )
{
    uint16_t               uiTotalBufCnt;
    uint32_t               ulBufSize = pHeapInfo->ulBufSize;
    uint16_t               uiPrimaryBufCnt = pHeapInfo->uiPrimaryBufCnt;
    BERR_Code              err = BERR_SUCCESS;

    BDBG_MSG(("Create heapInfo [%p]: %7s Heap size: %8d (%d, %d)",
        (void *)pHeapInfo,
        BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(pHeapInfo->eBufHeapId),
        ulBufSize, pHeapInfo->ulWidth, pHeapInfo->ulHeight));

    /* A size of 0 yields a valid MMA block pointer so catch it here. */
    if (ulBufSize * uiPrimaryBufCnt == 0)
    {
        /* coverity[assign_zero] */
        pHeapInfo->hMmaBlock = NULL;
    }
    else
    {
        uint32_t   ulExtraBufSize4MosaicDrain = 0;

        if(!pHeapInfo->hBufferHeap->ullMosaicDrainOffset)
        {
            /* 2 pixels wide, in case 10-bit 4:2:2 capture rounding,
               16 bytes aligned for capture engine. */
            ulExtraBufSize4MosaicDrain = 2;
        }

        /* Allocate MMA block from heap. */
        pHeapInfo->hMmaBlock = BMMA_Alloc(hMemory,
            ulBufSize * uiPrimaryBufCnt + ulExtraBufSize4MosaicDrain,
            1<<BVDC_P_HEAP_MEMORY_ALIGNMENT, NULL);
        if( !pHeapInfo->hMmaBlock && (0 != uiPrimaryBufCnt) )
        {
            BDBG_ERR(("Not enough device memory[%d]!", ulBufSize * uiPrimaryBufCnt));
            BDBG_ASSERT(0);
            return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        }
    }

    /* (2) Create buffer heap node list */
    uiTotalBufCnt = uiPrimaryBufCnt;
    if(pHeapInfo->pParentHeapInfo)
        uiTotalBufCnt += pHeapInfo->ucNodeCntPerParent * pHeapInfo->pParentHeapInfo->uiTotalBufCnt;

    pHeapInfo->pBufList =
        (BVDC_P_BufferHeapNode *)BKNI_Malloc(sizeof(BVDC_P_BufferHeapNode)*uiTotalBufCnt);

    if( !pHeapInfo->pBufList )
    {
        err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }

    /* (3) Create buffer heap primitive nodes of its own */
    if (BVDC_P_BufferHeap_BuildPrimaryNodeList(hMemory, pHeapInfo) != BERR_SUCCESS)
    {
        err = BERR_TRACE(BVDC_ERR_BAD_BUFFER_HEAP_NODE);
        goto err_malloc;
    }

    /* (4) Create buffer heap child nodes */
    if (BVDC_P_BufferHeap_BuildChildNodeList(hMemory, pHeapInfo) != BERR_SUCCESS)
    {
        err = BERR_TRACE(BVDC_ERR_BAD_BUFFER_HEAP_NODE);
        goto err_malloc;
    }

    return err;

err_malloc:
    if (pHeapInfo->pBufList)
    {
        BKNI_Free((void*)pHeapInfo->pBufList);
        pHeapInfo->pBufList = NULL;
    }
    if (pHeapInfo->hMmaBlock)
    {
        BMMA_Free(pHeapInfo->hMmaBlock);
    }
    return err;
}


/***************************************************************************
 *
 */
static BERR_Code BVDC_P_BufferHeap_DestroyHeapInfo
    ( BMMA_Heap_Handle             hMemory,
      BVDC_P_BufferHeap_Info      *pHeapInfo )
{
    BSTD_UNUSED(hMemory);

    /* [2] Free buffer heap node list */
    if (pHeapInfo->pBufList)
        BKNI_Free((void*)pHeapInfo->pBufList);

    /* [1] Free block of memory */
    if (pHeapInfo->hMmaBlock)
        BMMA_Free(pHeapInfo->hMmaBlock);

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
static BERR_Code BVDC_P_BufferHeap_MarkParentNode_isr
    ( BVDC_P_BufferHeap_Info      *pParentHeapInfo,
      BVDC_P_BufferHeapNode       *pCurrentNode )
{
    uint16_t    uiParentNodeBufIndex;
    BVDC_P_BufferHeapNode   *pParentHeapNode;

    if( !pParentHeapInfo || !pCurrentNode )
    {
        return BERR_SUCCESS;
    }

    /* Don't need to mark parent of primary nodes */
    if(pCurrentNode->uiParentNodeBufIndex == BVDC_P_HEAP_INVAID_BUF_INDEX)
    {
        return BERR_SUCCESS;
    }

    uiParentNodeBufIndex = pCurrentNode->uiParentNodeBufIndex;
    BDBG_ASSERT(uiParentNodeBufIndex < pParentHeapInfo->uiTotalBufCnt);
    pParentHeapNode = &pParentHeapInfo->pBufList[uiParentNodeBufIndex];
    if( pParentHeapNode )
    {
        pParentHeapNode->ucNumChildNodeUsed++;
        if( !pParentHeapNode->bUsed )
        {
            pParentHeapNode->bUsed = true;
            pParentHeapInfo->uiBufUsed++;
            BDBG_MSG(("Mark parent node: "BDBG_UINT64_FMT, BDBG_UINT64_ARG(pParentHeapNode->ullDeviceOffset)));
            BVDC_P_BufferHeap_DumpHeapNode_isr(pParentHeapNode);

            BVDC_P_BufferHeap_MarkParentNode_isr(
                pParentHeapInfo->pParentHeapInfo, pParentHeapNode);
        }
    }
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
static BERR_Code BVDC_P_BufferHeap_MarkChildNodes_isr
    ( BVDC_P_BufferHeap_Info      *pChildHeapInfo,
      BVDC_P_BufferHeapNode       *pCurrentNode )
{
    uint32_t                  i = 0;
    BVDC_P_BufferHeapNode    *pChildNode;
    uint16_t                  uiFirstChildNodeBufIndex;

    if( !pChildHeapInfo || !pCurrentNode )
    {
        return BERR_SUCCESS;
    }

    uiFirstChildNodeBufIndex = pCurrentNode->uiFirstChildNodeBufIndex;
    if(uiFirstChildNodeBufIndex == BVDC_P_HEAP_INVAID_BUF_INDEX)
    {
        return BERR_SUCCESS;
    }

    while(i < pCurrentNode->ucNumChildNodeSplit)
    {
        pChildNode = &pChildHeapInfo->pBufList[uiFirstChildNodeBufIndex+i];
        if( pChildNode )
        {
            pChildNode->bUsed = true;
            pChildHeapInfo->uiBufUsed++;
            pCurrentNode->ucNumChildNodeUsed++;
            BDBG_MSG(("Mark child node: "BDBG_UINT64_FMT, BDBG_UINT64_ARG(pChildNode->ullDeviceOffset)));
            BVDC_P_BufferHeap_DumpHeapNode_isr(pChildNode);

            BVDC_P_BufferHeap_MarkChildNodes_isr(pChildHeapInfo->pChildHeapInfo,
                pChildNode);
        }

        i++;
    }

    return BERR_SUCCESS;
}

/***************************************************************************
 * Mark a Heap node and its parent node and child nodes to be used.
 * This is called after a heap node is found
 */
static BERR_Code BVDC_P_BufferHeap_MarkNode_isr
    ( BVDC_P_BufferHeap_Info      *pHeapInfo,
      BVDC_P_BufferHeapNode       *pBufferHeapNode )
{
    /* Mark the found node */
    pBufferHeapNode->bUsed = true;
    pHeapInfo->uiBufUsed++;
    BDBG_MSG(("Mark node: "BDBG_UINT64_FMT, BDBG_UINT64_ARG(pBufferHeapNode->ullDeviceOffset)));
    BVDC_P_BufferHeap_DumpHeapNode_isr(pBufferHeapNode);

    /* Mark parent node and parent heap */
    BVDC_P_BufferHeap_MarkParentNode_isr(pHeapInfo->pParentHeapInfo,
        pBufferHeapNode);

    /* Mark child node and child heap */
    BVDC_P_BufferHeap_MarkChildNodes_isr(pHeapInfo->pChildHeapInfo,
        pBufferHeapNode);

    return BERR_SUCCESS;
}

/***************************************************************************
 * Unmark a Heap node and its parent node to be used.
 * This is called after a heap node is freed
 */
static BERR_Code BVDC_P_BufferHeap_UnmarkParentNode_isr
    ( BVDC_P_BufferHeap_Info       *pParentHeapInfo,
      BVDC_P_BufferHeapNode        *pCurrentNode )
{
    BVDC_P_BufferHeapNode  *pParentHeapNode;

    if( !pParentHeapInfo || !pCurrentNode )
    {
        return BERR_SUCCESS;
    }

    /* Don't need to unmark parent of primary nodes */
    if(pCurrentNode->uiParentNodeBufIndex == BVDC_P_HEAP_INVAID_BUF_INDEX)
    {
        return BERR_SUCCESS;
    }

    BDBG_ASSERT(pCurrentNode->uiParentNodeBufIndex < pParentHeapInfo->uiTotalBufCnt);
    pParentHeapNode = &pParentHeapInfo->pBufList[pCurrentNode->uiParentNodeBufIndex];

    if(pParentHeapNode)
    {
        pParentHeapNode->ucNumChildNodeUsed--;
        BDBG_MSG(("Check parent node: "));

        if( pParentHeapNode->ucNumChildNodeUsed == 0 )
        {
            if( pParentHeapNode->bUsed )
            {
                pParentHeapNode->bUsed = false;
                pParentHeapInfo->uiBufUsed--;
                BDBG_MSG(("Unmark parent node: "BDBG_UINT64_FMT, BDBG_UINT64_ARG(pParentHeapNode->ullDeviceOffset)));
                BVDC_P_BufferHeap_DumpHeapNode_isr(pParentHeapNode);
            }

            BVDC_P_BufferHeap_UnmarkParentNode_isr(pParentHeapInfo->pParentHeapInfo,
                pParentHeapNode);
        }
    }

    return BERR_SUCCESS;
}

/***************************************************************************
 * Unmark a Heap node and its parent node to be used.
 * This is called after a heap node is freed
 */
static BERR_Code BVDC_P_BufferHeap_UnmarkChildNodes_isr
    ( BVDC_P_BufferHeap_Info      *pChildHeapInfo,
      BVDC_P_BufferHeapNode       *pCurrentNode )
{
    uint16_t    i = 0, uiFirstChildNodeBufIndex;
    BVDC_P_BufferHeapNode   *pChildNode;

    if( !pChildHeapInfo || !pCurrentNode )
    {
        return BERR_SUCCESS;
    }

    uiFirstChildNodeBufIndex = pCurrentNode->uiFirstChildNodeBufIndex;
    if(uiFirstChildNodeBufIndex == BVDC_P_HEAP_INVAID_BUF_INDEX)
    {
        return BERR_SUCCESS;
    }

    while(i < pCurrentNode->ucNumChildNodeSplit)
    {
        pChildNode = &pChildHeapInfo->pBufList[uiFirstChildNodeBufIndex+i];

        pChildNode->bUsed = false;
        pChildHeapInfo->uiBufUsed--;
        pCurrentNode->ucNumChildNodeUsed--;
        BVDC_P_BufferHeap_DumpHeapNode_isr(pChildNode);
        BDBG_MSG(("Unmark child node: "BDBG_UINT64_FMT, BDBG_UINT64_ARG(pChildNode->ullDeviceOffset)));
        BVDC_P_BufferHeap_UnmarkChildNodes_isr(pChildHeapInfo->pChildHeapInfo,
            pChildNode);
        i++;
    }

    return BERR_SUCCESS;
}

/***************************************************************************
 * Unmark a Heap node and its parent node to be used.
 * This is called after a heap node is freed
 */
static BERR_Code BVDC_P_BufferHeap_UnmarkNode_isr
    ( BVDC_P_BufferHeapNode       *pBufferHeapNode )
{
    BVDC_P_BufferHeap_Info  *pHeapInfo = pBufferHeapNode->pHeapInfo;

    /* Unmark the found node */
    pBufferHeapNode->bUsed = false;
    /*
    pBufferHeapNode->ucNumChildNodeUsed = 0;
    */
    pHeapInfo->uiBufUsed--;
    BDBG_MSG(("Unmark node: "BDBG_UINT64_FMT, BDBG_UINT64_ARG(pBufferHeapNode->ullDeviceOffset)));
    BVDC_P_BufferHeap_DumpHeapNode_isr(pBufferHeapNode);

    /* Unmark parent node and parent heap */
    BVDC_P_BufferHeap_UnmarkParentNode_isr(pHeapInfo->pParentHeapInfo,
        pBufferHeapNode);

    /* Unmark child node and child heap */
    BVDC_P_BufferHeap_UnmarkChildNodes_isr(pHeapInfo->pChildHeapInfo,
        pBufferHeapNode);

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
static BERR_Code BVDC_P_BufferHeap_AllocateNodes_isr
    ( BVDC_P_BufferHeap_Info      *pHeapInfo,
      BVDC_P_HeapNodePtr          *apHeapNode,
      uint32_t                     ulCount,
      BVDC_P_BufferHeapId          eBufferHeapIdPrefer )
{
    bool                     bMatchNode = false;
    uint32_t                 i;
    BERR_Code                err = BERR_SUCCESS;
    BVDC_P_BufferHeapNode   *pBufferHeapNode;
    uint32_t                 j = 0;
    bool                     bFound = false;

    BDBG_ASSERT(apHeapNode);

    pBufferHeapNode = &pHeapInfo->pBufList[0];
    for( i = 0; i < ulCount; i++ )
    {
        bFound = false;
        while((j < pHeapInfo->uiTotalBufCnt) && !bFound)
        {
            pBufferHeapNode = &pHeapInfo->pBufList[j];

            /* Can use any node for BVDC_P_BufferHeapId_eUnknown */
            bMatchNode = (eBufferHeapIdPrefer == BVDC_P_BufferHeapId_eUnknown) |
                         (eBufferHeapIdPrefer == pBufferHeapNode->eOrigBufHeapId);
            bFound = !pBufferHeapNode->bUsed && bMatchNode;

            j++;
        }

        /* Find the node */
        if( pBufferHeapNode && bFound )
        {
            BDBG_MSG(("Found an available node: "BDBG_UINT64_FMT, BDBG_UINT64_ARG(pBufferHeapNode->ullDeviceOffset)));

            /* Mark the node and its parent node */
            BVDC_P_BufferHeap_MarkNode_isr(pHeapInfo, pBufferHeapNode);
            apHeapNode[i] = pBufferHeapNode;
        }
        else if(eBufferHeapIdPrefer != BVDC_P_BufferHeapId_eUnknown)
        {
            BDBG_MSG(("Can not find prefered %s for %s buffers ",
                BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(eBufferHeapIdPrefer),
                BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(pHeapInfo->eBufHeapId)));

            /* unmark nodes previously marked */
            while(i)
            {
                BVDC_P_BufferHeap_UnmarkNode_isr(apHeapNode[--i]);
            }
            return BERR_INVALID_PARAMETER;
        }
        else
        {
            BDBG_ERR(("Can not find an available node"));
            /* unmark nodes previously marked */
            while(i)
            {
                BVDC_P_BufferHeap_UnmarkNode_isr(apHeapNode[--i]);
            }
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

    }

    return err;

}

/***************************************************************************
 *
 */
static BERR_Code BVDC_P_BufferHeap_FreeNode_isr
    ( BVDC_P_HeapNodePtr           pHeapNode )
{
    bool       bFound = false;
    uint32_t   i = 0;
    BVDC_P_BufferHeapNode   *pTempNode;
    BVDC_P_BufferHeap_Info  *pHeapInfo = pHeapNode->pHeapInfo;

    BVDC_P_BufferHeap_DumpHeapNode_isr(pHeapNode);

    /* Find the node first */
    pTempNode = &pHeapInfo->pBufList[0];
    while((i < pHeapInfo->uiTotalBufCnt) && !bFound)
    {
        pTempNode = &pHeapInfo->pBufList[i];
        if(pTempNode->ullDeviceOffset != pHeapNode->ullDeviceOffset)
            i++;
        else
            bFound = true;
    }

    if( pTempNode && pTempNode->bUsed & bFound)
    {
        /* Unmark the node and its parent node */
        BVDC_P_BufferHeap_UnmarkNode_isr(pTempNode);
        BDBG_MSG(("Free node: "BDBG_UINT64_FMT, BDBG_UINT64_ARG(pTempNode->ullDeviceOffset)));
    }
    else
    {
        BDBG_MSG(("Can not find a matching node %p at "BDBG_UINT64_FMT "(%d)",
            (void *)pTempNode, BDBG_UINT64_ARG(pTempNode->ullDeviceOffset), pTempNode->bUsed));

        BVDC_P_BufferHeap_Dump_isr(pHeapInfo->hBufferHeap);
        BDBG_ASSERT(0);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_ASSERT(pHeapInfo->uiBufUsed <= pHeapInfo->uiTotalBufCnt);

    if(!pHeapInfo->uiBufUsed)
    {
        for(i = 0; i < pHeapInfo->uiTotalBufCnt; i++)
        {
            BDBG_ASSERT(!pHeapInfo->pBufList[i].bUsed);
        }
    }
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
static BERR_Code BVDC_P_BufferHeap_AllocateContNodes_isr
    ( BVDC_P_BufferHeap_Info          *pHeapInfo,
      BVDC_P_HeapNodePtr              *apHeapNode,
      uint32_t                         ulCount )
{
    bool                     bFound = false, bNext = true;
    uint32_t                 ulAllocCnt = 0;
    BERR_Code                err = BERR_SUCCESS;
    BVDC_P_BufferHeapNode   *pBufferHeapNodeStart, *pBufferHeapTempNode;
    uint32_t                 j = 0, ulStartNodeIndex = 0;
    bool                     bStartNodeFound = false;

    BDBG_ASSERT(apHeapNode);

    /* Get the first node in list */
    pBufferHeapNodeStart = &pHeapInfo->pBufList[0];

    while( pBufferHeapNodeStart && !bFound )
    {
        /* Get the first node */
        bStartNodeFound = false;
        while((j < pHeapInfo->uiTotalBufCnt) && !bStartNodeFound)
        {
            ulStartNodeIndex = j;
            pBufferHeapNodeStart = &pHeapInfo->pBufList[j];

            if(!pBufferHeapNodeStart->bUsed)
            {
                bStartNodeFound = true;
                ulAllocCnt = 1;
            }

            j++;
        }

        if( !bStartNodeFound )
        {
            break;
        }

        BDBG_MSG(("Find first available node(%d): "BDBG_UINT64_FMT,
            ulAllocCnt,BDBG_UINT64_ARG( pBufferHeapNodeStart->ullDeviceOffset)));
        BVDC_P_BufferHeap_DumpHeapNode_isr(pBufferHeapNodeStart);

        bNext = true;
        pBufferHeapTempNode = pBufferHeapNodeStart;
        while( (ulAllocCnt < ulCount) && (j < pHeapInfo->uiTotalBufCnt))
        {
            pBufferHeapTempNode= &pHeapInfo->pBufList[j];
            if( pBufferHeapTempNode->bUsed )
            {
                BDBG_MSG(("Find a non-available node: "BDBG_UINT64_FMT,
                    BDBG_UINT64_ARG(pBufferHeapTempNode->ullDeviceOffset)));
                BVDC_P_BufferHeap_DumpHeapNode_isr(pBufferHeapTempNode);
                bNext  = false;
                break;
            }
            else if( !pBufferHeapTempNode->bContinous )
            {
                BDBG_MSG(("Find a non-continous node: "BDBG_UINT64_FMT,
                    BDBG_UINT64_ARG(pBufferHeapTempNode->ullDeviceOffset)));
                BVDC_P_BufferHeap_DumpHeapNode_isr(pBufferHeapTempNode);
                bNext  = false;
                break;
            }
            else
            {
                ulAllocCnt++;
                BDBG_MSG(("Find next available node (%d): "BDBG_UINT64_FMT,
                    ulAllocCnt, BDBG_UINT64_ARG(pBufferHeapTempNode->ullDeviceOffset)));
                BVDC_P_BufferHeap_DumpHeapNode_isr(pBufferHeapTempNode);
            }

            j++;
        }

        if( !bNext )
        {
            BDBG_MSG(("Restart ..."));
            ulStartNodeIndex = j;
            pBufferHeapNodeStart = pBufferHeapTempNode;
        }
        else
        {
            bFound = true;
        }

    }

    if( pBufferHeapNodeStart && bFound )
    {
        if(ulAllocCnt != ulCount)
        {
            BDBG_ERR(("Need %d only find %d available nodes ", ulCount, ulAllocCnt));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        else
        {
            BDBG_MSG((" "));
            BDBG_MSG(("Find %d available nodes "BDBG_UINT64_FMT,
                ulCount, BDBG_UINT64_ARG(pBufferHeapNodeStart->ullDeviceOffset)));
            for( j = 0; j < ulCount; j++ )
            {
                /* Mark the node and its parent node */
                pBufferHeapTempNode= &pHeapInfo->pBufList[j+ulStartNodeIndex];
                BVDC_P_BufferHeap_MarkNode_isr(pHeapInfo, pBufferHeapTempNode);
                apHeapNode[j] = pBufferHeapTempNode;
            }
        }
    }
    else
    {
        BDBG_ERR(("Can not find an available node "));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    return err;
}

/***************************************************************************
 *
 */
static BERR_Code BVDC_P_BufferHeap_FreeContNodes_isr
    ( BVDC_P_HeapNodePtr              *pHeapNode,
      uint32_t                         ulCount )
{
    uint32_t                 i;
    BVDC_P_BufferHeapNode   *pTempNode;
    BVDC_P_BufferHeap_Info  *pHeapInfo;
    bool                     bFound = false;
    uint32_t                 j = 0, ulStartNodeIndex;

    if( !pHeapNode )
        return BERR_SUCCESS;

    pHeapInfo = pHeapNode[0]->pHeapInfo;

    ulStartNodeIndex = 0;
    pTempNode = &pHeapInfo->pBufList[0];

    /* Find the node first */
    while((j < pHeapInfo->uiTotalBufCnt) && !bFound)
    {
        ulStartNodeIndex = j;
        pTempNode = &pHeapInfo->pBufList[j];
        if(pTempNode->ullDeviceOffset != (*pHeapNode)->ullDeviceOffset)
            j++;
        else
            bFound = true;
    }

    if( pTempNode && bFound)
    {
        for(i = 0; i < ulCount; i++ )
        {
            pTempNode = &pHeapInfo->pBufList[ulStartNodeIndex+i];
            BDBG_MSG(("Free Cnt node: "BDBG_UINT64_FMT,
                BDBG_UINT64_ARG(pTempNode->ullDeviceOffset)));
            BVDC_P_BufferHeap_DumpHeapNode_isr(pTempNode);

            if( pTempNode && pTempNode->bUsed )
            {
                /* Unmark the node and its parent node */
                BVDC_P_BufferHeap_UnmarkNode_isr(pTempNode);
            }
            else
            {
                BDBG_ERR(("Can not find a matching node"));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }

        }
    }
    else
    {
        BDBG_ERR(("Can not find a matching node"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_ASSERT(pHeapInfo->uiBufUsed <= pHeapInfo->uiTotalBufCnt);
    if(!pHeapInfo->uiBufUsed)
    {
        for(i = 0; i < pHeapInfo->uiTotalBufCnt; i++)
        {
            BDBG_ASSERT(!pHeapInfo->pBufList[i].bUsed);
        }
    }
    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_P_BufferHeap_Create
    ( const BVDC_Handle           hVdc,
      BVDC_Heap_Handle           *phHeap,
      BMMA_Heap_Handle            hMem,
      const BVDC_Heap_Settings   *pSettings )
{
    uint32_t                  i;
    BERR_Code                 err = BERR_SUCCESS;
    BVDC_P_BufferHeapContext *pBufferHeap;

    BDBG_ENTER(BVDC_P_BufferHeap_Create);
    BDBG_OBJECT_ASSERT(hVdc, BVDC_VDC);

    /* BDBG_SetModuleLevel("BVDC_HEAP", BDBG_eMsg);  */

    /* (1) Create memory heap context */
    pBufferHeap = (BVDC_P_BufferHeapContext *)BKNI_Malloc(sizeof(BVDC_P_BufferHeapContext));
    if( !pBufferHeap )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    /* Clear out context */
    BKNI_Memset((void*)pBufferHeap, 0x0, sizeof(BVDC_P_BufferHeapContext));
    BDBG_OBJECT_SET(pBufferHeap, BVDC_BFH);

    /* Initialize fields. */
    pBufferHeap->hVdc         = hVdc;
    pBufferHeap->hMem         = hMem;
    pBufferHeap->stSettings   = *pSettings;

    /* Get const properties */
    BVDC_P_BufferHeap_GetHeapOrder(&pBufferHeap->stSettings,
        &pBufferHeap->stHeapSizeInfo, pBufferHeap);

    for( i = 0; i < BVDC_P_BufferHeapId_eCount; i++ )
    {
        /* (2) Create heap info */
        err = BERR_TRACE(BVDC_P_BufferHeap_CreateHeapInfo(
            pBufferHeap->hMem, &(pBufferHeap->astHeapInfo[i])));
        if( err != BERR_SUCCESS )
        {

            /* Free already allocated memory */
            while(i--)
            {
                BVDC_P_BufferHeap_DestroyHeapInfo(
                    pBufferHeap->hMem, &(pBufferHeap->astHeapInfo[i]));
            }
            /* Free memory context */
            BDBG_OBJECT_DESTROY(pBufferHeap, BVDC_BFH);
            BKNI_Free((void*)pBufferHeap);
            return BERR_TRACE(err);
        }
    }

    /* All done. now return the new fresh context to user. */
    *phHeap = (BVDC_P_BufferHeap_Handle)pBufferHeap;

    /* What's the output */
    BVDC_P_BufferHeap_Dump_isr(pBufferHeap);

    BDBG_LEAVE(BVDC_P_BufferHeap_Create);
    return err;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_P_BufferHeap_Destroy
    ( BVDC_P_BufferHeap_Handle      hBufferHeap )
{
    uint32_t                  i;

    BDBG_ENTER(BVDC_P_BufferHeap_Destroy);
    BDBG_OBJECT_ASSERT(hBufferHeap, BVDC_BFH);

    for( i = 0; i < BVDC_P_BufferHeapId_eCount; i++ )
    {
        /* [2] Free heap info */
        BVDC_P_BufferHeap_DestroyHeapInfo(hBufferHeap->hMem,
            &(hBufferHeap->astHeapInfo[i]));
    }

    BDBG_OBJECT_DESTROY(hBufferHeap, BVDC_BFH);
    /* [1] Free memory context */
    BKNI_Free((void*)hBufferHeap);

    BDBG_LEAVE(BVDC_P_BufferHeap_Destroy);
    return BERR_SUCCESS;
}


/***************************************************************************
 * eBufferHeapIdPrefer is where the user prefer buffer node original comes
 * from.
 * example: User prefers a SD buffer node comes from HD buffer heap. It will
 * try to find a break up SD node from HD buffer. If not found, it will loose
 * the requirement and set eBufferHeapIdPrefer to BVDC_P_BufferHeapId_eUnknown
 * and try to find a SD node from any buffer.
 *
 * if eBufferHeapIdPrefer is BVDC_P_BufferHeapId_eUnknown, the node can come from
 * any buffer heap.
 */
BERR_Code BVDC_P_BufferHeap_AllocateBuffers_isr
    ( BVDC_P_BufferHeap_Handle         hBufferHeap,
      BVDC_P_HeapNodePtr               apHeapNode[],
      uint32_t                         ulCount,
      bool                             bContinuous,
      BVDC_P_BufferHeapId              eBufferHeapId,
      BVDC_P_BufferHeapId              eBufferHeapIdPrefer )
{
    uint32_t                  ulIndex;
    uint32_t                  ulBufferAvailable;
    BERR_Code                 err = BERR_SUCCESS;
    BVDC_P_BufferHeap_Info   *pHeapInfo;

    BDBG_ENTER(BVDC_P_BufferHeap_AllocateBuffers_isr);
    BDBG_OBJECT_ASSERT(hBufferHeap, BVDC_BFH);

    if( eBufferHeapId >= BVDC_P_BufferHeapId_eCount )
    {
        BDBG_ERR(("Not supported"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_MSG(("Allocate %d %s (prefered %s) buffers ", ulCount,
        BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(eBufferHeapId),
        BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(eBufferHeapIdPrefer)));

    ulIndex = hBufferHeap->stHeapSizeInfo.aulIndex[eBufferHeapId];
    pHeapInfo = &(hBufferHeap->astHeapInfo[ulIndex]);

    BDBG_ASSERT(pHeapInfo->eBufHeapId == eBufferHeapId);

    /* Get buffer heap */
    ulBufferAvailable = pHeapInfo->uiTotalBufCnt - pHeapInfo->uiBufUsed;

    if( ulCount > ulBufferAvailable )
    {
        const BFMT_VideoInfo *pFmtInfo;
        const BVDC_Heap_Settings *pHeap = &hBufferHeap->stSettings;

        BDBG_ERR(("App needs to alloc more memory! Needs [%d] %s (%s preferred) buffers and has [%d] %s buffers.",
            ulCount,
            BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(eBufferHeapId),
            BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(eBufferHeapIdPrefer),
            ulBufferAvailable, BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(eBufferHeapId)));

        BDBG_ERR(("Heap settings:"));
        pFmtInfo = BFMT_GetVideoFormatInfoPtr_isr(pHeap->eBufferFormat_4HD);
        if(pFmtInfo == NULL)
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ERR(("--------4HD---------"));
        BDBG_ERR(("ulBufferCnt     = %d", pHeap->ulBufferCnt_4HD));
        BDBG_ERR(("ulBufferCnt_Pip = %d", pHeap->ulBufferCnt_4HD_Pip));
        BDBG_ERR(("eBufferFormat   = %s", pFmtInfo->pchFormatStr));
        BDBG_ERR(("ePixelFormat    = %s", BPXL_ConvertFmtToStr_isr(pHeap->ePixelFormat_4HD)));

        pFmtInfo = BFMT_GetVideoFormatInfoPtr_isr(pHeap->eBufferFormat_2HD);
        if(pFmtInfo == NULL)
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ERR(("--------2HD---------"));
        BDBG_ERR(("ulBufferCnt     = %d", pHeap->ulBufferCnt_2HD));
        BDBG_ERR(("ulBufferCnt_Pip = %d", pHeap->ulBufferCnt_2HD_Pip));
        BDBG_ERR(("eBufferFormat   = %s", pFmtInfo->pchFormatStr));
        BDBG_ERR(("ePixelFormat    = %s", BPXL_ConvertFmtToStr_isr(pHeap->ePixelFormat_2HD)));

        pFmtInfo = BFMT_GetVideoFormatInfoPtr_isr(pHeap->eBufferFormat_HD);
        if(pFmtInfo == NULL)
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ERR(("---------HD---------"));
        BDBG_ERR(("ulBufferCnt     = %d", pHeap->ulBufferCnt_HD));
        BDBG_ERR(("ulBufferCnt_Pip = %d", pHeap->ulBufferCnt_HD_Pip));
        BDBG_ERR(("eBufferFormat   = %s", pFmtInfo->pchFormatStr));
        BDBG_ERR(("ePixelFormat    = %s", BPXL_ConvertFmtToStr_isr(pHeap->ePixelFormat_HD)));

        pFmtInfo = BFMT_GetVideoFormatInfoPtr_isr(pHeap->eBufferFormat_SD);
        if(pFmtInfo == NULL)
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ERR(("---------SD---------"));
        BDBG_ERR(("ulBufferCnt     = %d", pHeap->ulBufferCnt_SD));
        BDBG_ERR(("ulBufferCnt_Pip = %d", pHeap->ulBufferCnt_SD_Pip));
        BDBG_ERR(("eBufferFormat   = %s", pFmtInfo->pchFormatStr));
        BDBG_ERR(("ePixelFormat    = %s", BPXL_ConvertFmtToStr_isr(pHeap->ePixelFormat_SD)));
        BDBG_ERR(("--------------------"));

        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }

    if( bContinuous )
    {
        BDBG_ASSERT(eBufferHeapIdPrefer == BVDC_P_BufferHeapId_eUnknown);
        err = BERR_TRACE(BVDC_P_BufferHeap_AllocateContNodes_isr(
            pHeapInfo, apHeapNode, ulCount));
    }
    else
    {
        if(eBufferHeapIdPrefer == BVDC_P_BufferHeapId_eUnknown)
        {
            err = BERR_TRACE(BVDC_P_BufferHeap_AllocateNodes_isr(
                pHeapInfo, apHeapNode, ulCount, eBufferHeapIdPrefer));
        }
        else
        {
            /* Try to find prefered nodes first */
            err = BVDC_P_BufferHeap_AllocateNodes_isr(
                pHeapInfo, apHeapNode, ulCount, eBufferHeapIdPrefer);

            /* Didn't find prefered nodes, find them from other heap buffer */
            if(err != BERR_SUCCESS)
            {
                BDBG_MSG(("Try to find any %d %s buffers ", ulCount,
                    BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(eBufferHeapId)));
                err = BERR_TRACE(BVDC_P_BufferHeap_AllocateNodes_isr(
                    pHeapInfo, apHeapNode, ulCount, BVDC_P_BufferHeapId_eUnknown));
            }
        }
    }

    BDBG_LEAVE(BVDC_P_BufferHeap_AllocateBuffers_isr);
    return err;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_P_BufferHeap_FreeBuffers_isr
    ( BVDC_P_BufferHeap_Handle         hBufferHeap,
      BVDC_P_HeapNodePtr               apHeapNode[],
      uint32_t                         ulCount,
      bool                             bContinuous )
{
    BERR_Code                 err = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_BufferHeap_FreeBuffers_isr);
    BDBG_OBJECT_ASSERT(hBufferHeap, BVDC_BFH);

    BDBG_MSG(("Free %d buffers ", ulCount));

    if( bContinuous )
    {
        err = BERR_TRACE(BVDC_P_BufferHeap_FreeContNodes_isr(
            apHeapNode, ulCount));
    }
    else
    {
        uint32_t i;

        for( i = 0; i < ulCount; i++ )
        {
            err = BERR_TRACE(BVDC_P_BufferHeap_FreeNode_isr(apHeapNode[i]));
            if( err != BERR_SUCCESS )
                return err;
        }
    }

    BDBG_LEAVE(BVDC_P_BufferHeap_FreeBuffers_isr);
    return err;
}

/***************************************************************************
 * Find the best fit buffer
 */
void BVDC_P_BufferHeap_GetHeapIdBySize_isr
    ( const BVDC_P_BufferHeap_SizeInfo  *pHeapSizeInfo,
      uint32_t                           ulSize,
      BVDC_P_BufferHeapId               *peBufHeapId )
{
    uint32_t                  i, ulIndex;
    BVDC_P_BufferHeapId       eBufferHeapId = BVDC_P_BufferHeapId_eUnknown;

    /* astHeapInfo are stored from big to small */
    for(i = 0; i < BVDC_P_BufferHeapId_eCount; i++)
    {
        ulIndex = BVDC_P_BufferHeapId_eCount -i - 1;
        if(pHeapSizeInfo->aulBufSize[ulIndex] >= ulSize)
        {
            eBufferHeapId = pHeapSizeInfo->aeBufHeapId[ulIndex];
            break;
        }
    }

    if(eBufferHeapId == BVDC_P_BufferHeapId_eUnknown)
    {
        BDBG_WRN(("Can not find a buffer heap to fit 0x%x", ulSize));
    }

    if(peBufHeapId)
        *peBufHeapId = eBufferHeapId;

}


void BVDC_P_BufferHeap_GetHeapSizeById_isr
    ( const BVDC_P_BufferHeapContext  *pHeap,
      BVDC_P_BufferHeapId              eBufHeapId,
      uint32_t                        *pulBufHeapSize )
{
    uint32_t   i, ulBufSize = 0;

    if(eBufHeapId == BVDC_P_BufferHeapId_eUnknown)
    {
        BDBG_WRN(("Unknown buffer heap Id: %d", eBufHeapId));
    }

    /* astHeapInfo are stored from big to small */
    for(i = 0; i < BVDC_P_BufferHeapId_eCount; i++)
    {
        if(pHeap->astHeapInfo[i].eBufHeapId == eBufHeapId)
        {
            ulBufSize = pHeap->astHeapInfo[i].ulBufSize;
            break;
        }
    }

    if(pulBufHeapSize)
        *pulBufHeapSize = ulBufSize;

}

#if (BVDC_P_CHECK_MEMC_INDEX)
BERR_Code BVDC_P_BufferHeap_CheckHeapMemcIndex
    ( BVDC_P_BufferHeapContext      *pHeap,
      uint32_t                       ulMemcIndex,
      BVDC_DisplayId                 eDispId,
      BVDC_WindowId                  eWinId )
{
    uint32_t    i;
    BVDC_P_BufferHeap_Info   *pHeapInfo;

#if (!BDBG_DEBUG_BUILD)
    BSTD_UNUSED(eDispId);
    BSTD_UNUSED(eWinId);
#endif

    BDBG_ASSERT(pHeap);
    BDBG_ASSERT(ulMemcIndex != BBOX_MemcIndex_Invalid);

    if(ulMemcIndex == BBOX_VDC_DISREGARD)
        return BERR_SUCCESS;

    /* TODO: Call BCHP API to validate heap (SW7445-2325) */
    for(i = 0; i < BVDC_P_BufferHeapId_eCount; i++)
    {
        pHeapInfo = &pHeap->astHeapInfo[i];
        if(pHeapInfo->uiPrimaryBufCnt)
        {
            BSTD_DeviceOffset ulHeapStartOffset = (uint64_t)pHeapInfo->pBufList[0].ullDeviceOffset;
            if( !BCHP_OffsetOnMemc(pHeap->hVdc->hChip, ulHeapStartOffset, ulMemcIndex) )
            {
                BDBG_MODULE_MSG(BVDC_MEMC_INDEX_CHECK,
                    ("Disp[%d]Win[%d] mismatch between memory heap " BDBG_UINT64_FMT " and Box mode MEMC%u",
                    eDispId, eWinId, BDBG_UINT64_ARG(ulHeapStartOffset), ulMemcIndex));
            }
        }
    }

    return BERR_SUCCESS;
}
#endif


/* End of file. */

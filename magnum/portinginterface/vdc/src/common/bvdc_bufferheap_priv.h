/***************************************************************************
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
 ***************************************************************************/
#ifndef BVDC_BUFFERHEAP_PRIV_H__
#define BVDC_BUFFERHEAP_PRIV_H__

#include "bvdc.h"
#include "blst_queue.h"
#include "bvdc_common_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

BDBG_OBJECT_ID_DECLARE(BVDC_VDC);

/***************************************************************************
 * Private macros
 ***************************************************************************/
#define BVDC_P_BUFFERHEAP_HEAP_IS_4HD(eBufHeapId) \
    (eBufHeapId == BVDC_P_BufferHeapId_e4HD)

#define BVDC_P_BUFFERHEAP_HEAP_IS_2HD(eBufHeapId) \
    (eBufHeapId == BVDC_P_BufferHeapId_e2HD)

#define BVDC_P_BUFFERHEAP_HEAP_IS_HD(eBufHeapId) \
    (eBufHeapId == BVDC_P_BufferHeapId_eHD)

#define BVDC_P_BUFFERHEAP_HEAP_IS_SD(eBufHeapId) \
    (eBufHeapId == BVDC_P_BufferHeapId_eSD)

#define BVDC_P_BUFFERHEAP_HEAP_IS_4HD_PIP(eBufHeapId) \
    (eBufHeapId == BVDC_P_BufferHeapId_e4HD_Pip)

#define BVDC_P_BUFFERHEAP_HEAP_IS_2HD_PIP(eBufHeapId) \
    (eBufHeapId == BVDC_P_BufferHeapId_e2HD_Pip)

#define BVDC_P_BUFFERHEAP_HEAP_IS_HD_PIP(eBufHeapId) \
    (eBufHeapId == BVDC_P_BufferHeapId_eHD_Pip)

#define BVDC_P_BUFFERHEAP_HEAP_IS_SD_PIP(eBufHeapId) \
    (eBufHeapId == BVDC_P_BufferHeapId_eSD_Pip)

#define BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(eBufHeapId)      \
    BVDC_P_BUFFERHEAP_HEAP_IS_4HD(eBufHeapId)     ? "4HD"     :    \
    BVDC_P_BUFFERHEAP_HEAP_IS_2HD(eBufHeapId)     ? "2HD"     :    \
    BVDC_P_BUFFERHEAP_HEAP_IS_HD(eBufHeapId)      ? "HD"      :    \
    BVDC_P_BUFFERHEAP_HEAP_IS_SD(eBufHeapId)      ? "SD"      :    \
    BVDC_P_BUFFERHEAP_HEAP_IS_4HD_PIP(eBufHeapId) ? "4HD_Pip" :    \
    BVDC_P_BUFFERHEAP_HEAP_IS_2HD_PIP(eBufHeapId) ? "2HD_Pip" :    \
    BVDC_P_BUFFERHEAP_HEAP_IS_HD_PIP(eBufHeapId)  ? "HD_Pip"  :    \
    BVDC_P_BUFFERHEAP_HEAP_IS_SD_PIP(eBufHeapId)  ? "SD_Pip"  : "Unknown"


#define BVDC_P_BUFFERHEAP_GetDeviceOffset(pHeapNode)   \
    pHeapNode->ullDeviceOffset

/***************************************************************************
 * Internal defines
 ***************************************************************************/
#define BVDC_P_HEAP_MEMORY_ALIGNMENT       (8)
#define BVDC_P_HEAP_ALIGN_BYTES            (256)

/***************************************************************************
 * Enums
 ***************************************************************************/
typedef enum BVDC_P_BufferHeapId
{
    BVDC_P_BufferHeapId_eSD = 0,
    BVDC_P_BufferHeapId_eHD,
    BVDC_P_BufferHeapId_e2HD,
    BVDC_P_BufferHeapId_e4HD,
    BVDC_P_BufferHeapId_eSD_Pip,
    BVDC_P_BufferHeapId_eHD_Pip,
    BVDC_P_BufferHeapId_e2HD_Pip,
    BVDC_P_BufferHeapId_e4HD_Pip,

    BVDC_P_BufferHeapId_eCount,             /* Counter. Do not use! */
    BVDC_P_BufferHeapId_eUnknown

} BVDC_P_BufferHeapId;

/***************************************************************************
 * BVDC_P_BufferHeap_Head
 ***************************************************************************/
typedef struct BVDC_P_BufferHeap_Head  BVDC_P_BufferHeap_Head;
BLST_Q_HEAD(BVDC_P_BufferHeap_Head, BVDC_P_BufferHeapNode);


/***************************************************************************
 * BVDC_P_BufferHeapNode
 ***************************************************************************/
typedef struct BVDC_P_BufferHeapNode
{
    /* Node info: linked-list bookeeping */
    BLST_Q_ENTRY(BVDC_P_BufferHeapNode)  link;

    BVDC_P_HeapInfoPtr                   pHeapInfo;      /* heap the node belongs to */
    uint32_t                             ulBlockOffset;  /* offset from MMA block's base address */

    BMMA_DeviceOffset                    ullDeviceOffset; /* Device offset */
    uint32_t                             ulBufIndex;  /* index to heap's bufferlist */
    BVDC_P_BufferHeapId                  eOrigBufHeapId; /* which primary heap the node came from */

    bool                                 bUsed;       /* node is used or not */
    bool                                 bContinous;  /* continuous with prev node in bufferlist */

    uint32_t                             ulParentNodeBufIndex; /* parent node */
    uint32_t                             ulFirstChildNodeBufIndex; /* first child node */

    uint32_t                             ulNumChildNodeUsed; /* num of child nodes used */
    uint32_t                             ulNumChildNodeSplit; /* num of child nodes this node split into */

} BVDC_P_BufferHeapNode;

/***************************************************************************
 * BVDC_P_BufferHeap_Info
 ***************************************************************************/
typedef struct BVDC_P_BufferHeap_Info
{
    BVDC_P_BufferHeap_Handle   hBufferHeap;

    /* Const properties */
    BVDC_P_BufferHeapId        eBufHeapId;
    uint32_t                   ulWidth;
    uint32_t                   ulHeight;
    /* real size including alignment */
    uint32_t                   ulBufSize;
    /* size by format without alignment, only used to decide how to split buffers */
    uint32_t                   ulBufSizeByFmt;
    uint32_t                   ulPrimaryBufCnt;
    uint32_t                   ulTotalBufCnt;
    uint32_t                   ulBufUsed;
    uint32_t                   ulNodeCntPerParent; /* # nodes = 1 parent node */
    BVDC_P_HeapInfoPtr         pParentHeapInfo;
    BVDC_P_HeapInfoPtr         pChildHeapInfo;

    /* handle to MMA block */
    BMMA_Block_Handle          hMmaBlock;

    BVDC_P_BufferHeapNode     *pBufList;

} BVDC_P_BufferHeap_Info;



/***************************************************************************
 * BVDC_P_BufferHeap_ConfigInfo
 ***************************************************************************/
typedef struct BVDC_P_BufferHeap_SizeInfo
{
    /* Index of astHeapInfo by BVDC_P_BufferHeapId */
    uint32_t                         aulIndex[BVDC_P_BufferHeapId_eCount];

    /* Sorted from bif to small. Use aulIndex to access array */
    uint32_t                         aulBufSize[BVDC_P_BufferHeapId_eCount];
    BVDC_P_BufferHeapId              aeBufHeapId[BVDC_P_BufferHeapId_eCount];

} BVDC_P_BufferHeap_SizeInfo;

/***************************************************************************
 * BVDC_P_BufferHeapContext
 ***************************************************************************/
typedef struct BVDC_P_BufferHeapContext
{
    BDBG_OBJECT(BVDC_BFH)

    /* Handed down by created */
    BVDC_Handle                      hVdc;          /* Created from this Vdc */
    BMMA_Heap_Handle                 hMem;          /* corresponding heap. */
    BMMA_DeviceOffset                ullMosaicDrainOffset; /* drain buffer offset */

    /* Heap Settings */
    BVDC_Heap_Settings               stSettings;

    BVDC_P_BufferHeap_SizeInfo       stHeapSizeInfo;

    BVDC_P_BufferHeap_Info           astHeapInfo[BVDC_P_BufferHeapId_eCount];

} BVDC_P_BufferHeapContext;


/***************************************************************************
 * Memory private functions
 ***************************************************************************/
BERR_Code BVDC_P_BufferHeap_Create
    ( const BVDC_Handle                hVdc,
      BVDC_Heap_Handle                *phHeap,
      BMMA_Heap_Handle                 hMem,
      const BVDC_Heap_Settings        *pSettings );

BERR_Code BVDC_P_BufferHeap_Destroy
    ( BVDC_P_BufferHeap_Handle         hBufferHeap );

BERR_Code BVDC_P_BufferHeap_AllocateBuffers_isr
    ( BVDC_P_BufferHeap_Handle         hBufferHeap,
      BVDC_P_HeapNodePtr               apHeapNode[],
      uint32_t                         ulCount,
      bool                             bContinuous,
      BVDC_P_BufferHeapId              eBufferHeapId,
      BVDC_P_BufferHeapId              eBufferHeapIdPrefer );

BERR_Code BVDC_P_BufferHeap_FreeBuffers_isr
    ( BVDC_P_BufferHeap_Handle         hBufferHeap,
      BVDC_P_HeapNodePtr               apHeapNode[],
      uint32_t                         ulCount,
      bool                             bContinuous );

uint32_t BVDC_P_BufferHeap_GetHeapSize
    ( const BFMT_VideoInfo            *pFmtInfo,
      BPXL_Format                      ePixelFmt,
      bool                             bPip,
      uint32_t                        *pulUnalignedBufSize,
      uint32_t                        *pulWidth,
      uint32_t                        *pulHeight );

void BVDC_P_BufferHeap_GetHeapIdBySize_isr
    ( const BVDC_P_BufferHeap_SizeInfo *pHeapSizeInfo,
      uint32_t                          ulSize,
      BVDC_P_BufferHeapId              *peBufHeapId );
#define BVDC_P_BufferHeap_GetHeapIdBySize  BVDC_P_BufferHeap_GetHeapIdBySize_isr

void BVDC_P_BufferHeap_GetHeapSizeById_isr
    ( const BVDC_P_BufferHeapContext  *pHeap,
      BVDC_P_BufferHeapId              eBufHeapId,
      uint32_t                        *pulBufHeapSize );

void BVDC_P_BufferHeap_GetHeapOrder
    ( const BVDC_Heap_Settings        *pSettings,
      BVDC_P_BufferHeap_SizeInfo      *pHeapSizeInfo,
      BVDC_P_BufferHeapContext        *pBufferHeap );

BERR_Code BVDC_P_BufferHeap_CheckHeapMemcIndex
    ( BVDC_P_BufferHeapContext        *pHeap,
      uint32_t                         ulMemcIndex,
      BVDC_DisplayId                   eDispId,
      BVDC_WindowId                    eWinId );
#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_BUFFERHEAP_PRIV_H__*/

/* End of file. */

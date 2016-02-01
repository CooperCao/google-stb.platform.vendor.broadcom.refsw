/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bvdc.h"
#include "bvdc_bufferheap_priv.h"
#include "bvdc_priv.h"
#include "bvdc_displayfmt_priv.h"


BDBG_MODULE(BVDC_HEAP);
BDBG_FILE_MODULE(BVDC_MEMC_INDEX_CHECK);
BDBG_FILE_MODULE(BVDC_WIN_BUF);
BDBG_OBJECT_ID(BVDC_BFH);

/* TODO: Rework to support generic buffer broken down. */
#define BVDC_P_MEMC_ALIGNMENT  (1)
#define BVDC_P_PITCH_ALIGNMENT (1)
#define BVDC_P_HSIZE_ALIGNMENT (1)

/***************************************************************************
 *
 */
static void BVDC_P_BufferHeap_DumpHeapNode_isr
	( const BVDC_P_BufferHeapNode     *pBufferHeapNode )
{
/*#if 1*/
#if (BDBG_DEBUG_BUILD)
	/* ulBufIndex (ulNodeCntPerParent eOrigBufHeapId) (Continous)
	 *       (Used - ulNumChildNodeUsed) at ulDeviceOffset (pvBufAddr) */
	BDBG_MSG(("\t- Node %p %2d (%s) (%s) (%s - %d) at 0x%x, block offset 0x%x",
	(void *)pBufferHeapNode,
	pBufferHeapNode->ulBufIndex,
	BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(pBufferHeapNode->eOrigBufHeapId),
	pBufferHeapNode->bContinous? "c" : "n",
	pBufferHeapNode->bUsed ? "x" : " ",
	pBufferHeapNode->ulNumChildNodeUsed,
	pBufferHeapNode->ulDeviceOffset,
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
	uint32_t i;

	BDBG_MSG((" "));
	BDBG_MODULE_MSG(BVDC_WIN_BUF, ("%s Heap",
		BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(pHeapInfo->eBufHeapId)));

	if( pHeapInfo->pParentHeapInfo )
	{
		BDBG_MODULE_MSG(BVDC_WIN_BUF, ("\tParent Heap        = %s",
			BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(pHeapInfo->pParentHeapInfo->eBufHeapId)));
	}
	else
	{
		BDBG_MODULE_MSG(BVDC_WIN_BUF, ("\tParent Heap        = NULL"));
	}

	if( pHeapInfo->pChildHeapInfo )
	{
		BDBG_MODULE_MSG(BVDC_WIN_BUF, ("\tChild Heap         = %s",
			BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(pHeapInfo->pChildHeapInfo->eBufHeapId)));
	}
	else
	{
		BDBG_MODULE_MSG(BVDC_WIN_BUF, ("\tChild Heap         = NULL"));
	}

	BDBG_MODULE_MSG(BVDC_WIN_BUF, ("\tpHeapInfo          = %p:",    (void *)pHeapInfo));
	BDBG_MODULE_MSG(BVDC_WIN_BUF, ("\tulNodeCntPerParent = 0x%x:",  pHeapInfo->ulNodeCntPerParent));
	BDBG_MODULE_MSG(BVDC_WIN_BUF, ("\tulPrimaryBufCnt    = %d:",    pHeapInfo->ulPrimaryBufCnt));
	BDBG_MODULE_MSG(BVDC_WIN_BUF, ("\tulTotalBufCnt      = %d:",    pHeapInfo->ulTotalBufCnt));
	BDBG_MODULE_MSG(BVDC_WIN_BUF, ("\tulBufSize          = %d:",    pHeapInfo->ulBufSize));
	BDBG_MODULE_MSG(BVDC_WIN_BUF, ("\thMmaBlock          = %p:",    (void *)pHeapInfo->hMmaBlock));
	BDBG_MODULE_MSG(BVDC_WIN_BUF, ("\tulBufUsed          = 0x%d:",  pHeapInfo->ulBufUsed));
	BDBG_MSG(("\tNode list:"));

	for(i = 0; i < pHeapInfo->ulTotalBufCnt; i++)
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
	/* Make sure 32 byte aligned */
#if BVDC_P_PITCH_ALIGNMENT
	uiPitch = BVDC_P_ALIGN_UP(uiPitch, BVDC_P_PITCH_ALIGN);
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
 *    eBufHeapId, ulBufSize, ulPrimaryBufCnt, pParentHeapInfo,
 *    ulNodeCntPerParent
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
	uint32_t  aulNodeCntPerParent[BVDC_P_BufferHeapId_eCount];

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
		pBufferHeap->astHeapInfo[i].ulPrimaryBufCnt = pSettings->ulBufferCnt_4HD;
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
			pBufferHeap->astHeapInfo[i].ulPrimaryBufCnt = pSettings->ulBufferCnt_2HD;
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
			pBufferHeap->astHeapInfo[i].ulPrimaryBufCnt = pSettings->ulBufferCnt_4HD_Pip;
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
			pBufferHeap->astHeapInfo[i].ulPrimaryBufCnt = pSettings->ulBufferCnt_4HD_Pip;
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
			pBufferHeap->astHeapInfo[i].ulPrimaryBufCnt = pSettings->ulBufferCnt_2HD;
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
			pBufferHeap->astHeapInfo[i].ulPrimaryBufCnt = pSettings->ulBufferCnt_HD;
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
			pBufferHeap->astHeapInfo[i].ulPrimaryBufCnt = pSettings->ulBufferCnt_2HD_Pip;
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
			pBufferHeap->astHeapInfo[i].ulPrimaryBufCnt = pSettings->ulBufferCnt_2HD_Pip;
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
			pBufferHeap->astHeapInfo[i].ulPrimaryBufCnt = pSettings->ulBufferCnt_HD;
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
			pBufferHeap->astHeapInfo[i].ulPrimaryBufCnt = pSettings->ulBufferCnt_SD;
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
			pBufferHeap->astHeapInfo[i].ulPrimaryBufCnt = pSettings->ulBufferCnt_HD_Pip;
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
			pBufferHeap->astHeapInfo[i].ulPrimaryBufCnt = pSettings->ulBufferCnt_HD_Pip;
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
			pBufferHeap->astHeapInfo[i].ulPrimaryBufCnt = pSettings->ulBufferCnt_SD;
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
		pBufferHeap->astHeapInfo[i].ulPrimaryBufCnt = pSettings->ulBufferCnt_SD_Pip;
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
			pBufferHeap->astHeapInfo[i].ulBufUsed     = 0;
			pBufferHeap->astHeapInfo[i].hBufferHeap   = pBufferHeap;
			pBufferHeap->astHeapInfo[i].ulTotalBufCnt = pBufferHeap->astHeapInfo[i].ulPrimaryBufCnt;
		}

		if( i == 0 )
		{
			aulNodeCntPerParent[i] = 0;
			if(pBufferHeap)
			{
				pBufferHeap->astHeapInfo[i].pParentHeapInfo    = NULL;
				pBufferHeap->astHeapInfo[i].ulNodeCntPerParent = 0;
				BDBG_ASSERT(aulNodeCntPerParent[i] == pBufferHeap->astHeapInfo[i].ulNodeCntPerParent);
			}
		}
		else
		{
			aulNodeCntPerParent[i] = BVDC_P_MAX(
				pHeapSizeInfo->aulBufSize[i-1] / pHeapSizeInfo->aulBufSize[i],
				aulUnAlignedBufSize[i-1] / aulUnAlignedBufSize[i]);
			if(pBufferHeap)
			{
				pBufferHeap->astHeapInfo[i].pParentHeapInfo
					= &pBufferHeap->astHeapInfo[i-1];
				pBufferHeap->astHeapInfo[i].ulNodeCntPerParent
					= BVDC_P_MAX(
					pBufferHeap->astHeapInfo[i-1].ulBufSize / pBufferHeap->astHeapInfo[i].ulBufSize,
					pBufferHeap->astHeapInfo[i-1].ulBufSizeByFmt / pBufferHeap->astHeapInfo[i].ulBufSizeByFmt);
				BDBG_ASSERT(aulNodeCntPerParent[i] == pBufferHeap->astHeapInfo[i].ulNodeCntPerParent);
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

	/* Adjust buf size for alignment: from bottom up */
	for( i = BVDC_P_BufferHeapId_eCount - 1; i > 0; i-- )
	{
		ulNewBufSize = aulNodeCntPerParent[i] * pHeapSizeInfo->aulBufSize[i];

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
	uint32_t               i, ulDeviceOffset = 0, ulBlockOffset = 0;
	uint32_t               ulBufSize = pHeapInfo->ulBufSize;
	uint32_t               ulPrimaryBufCnt = pHeapInfo->ulPrimaryBufCnt;
	BERR_Code              err = BERR_SUCCESS;
	BVDC_P_BufferHeapNode *pBufferHeapNode;

	BSTD_UNUSED(hMemory);

	if (ulPrimaryBufCnt)
	{
        ulDeviceOffset = BMMA_LockOffset(pHeapInfo->hMmaBlock);
	}

	for(i = 0; i < ulPrimaryBufCnt; i++)
	{
		pBufferHeapNode = &(pHeapInfo->pBufList[i]);

		/* Initialize fields. */
		pBufferHeapNode->ulBufIndex   = i;
		pBufferHeapNode->pHeapInfo    = pHeapInfo;
		pBufferHeapNode->bUsed        = false;
		pBufferHeapNode->ulDeviceOffset = ulDeviceOffset;
		pBufferHeapNode->ulBlockOffset  = ulBlockOffset;
		pBufferHeapNode->ulNumChildNodeUsed = 0;
		pBufferHeapNode->bContinous = true;
		pBufferHeapNode->eOrigBufHeapId = pHeapInfo->eBufHeapId;
		pBufferHeapNode->ulParentNodeBufIndex = 0xffffffff;
		pBufferHeapNode->ulFirstChildNodeBufIndex = 0xffffffff;

		ulDeviceOffset += ulBufSize;
		ulBlockOffset += ulBufSize;
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
	uint32_t                i, j;
	uint32_t                ulBufSize, ulDeviceOffset, ulBlockOffset;
	BVDC_P_BufferHeapNode  *pBufferHeapNode, *pParentHeapNode;
	BVDC_P_BufferHeap_Info *pParentHeapInfo = pHeapInfo->pParentHeapInfo;

	BSTD_UNUSED(hMemory);

	if( !pParentHeapInfo )
	{
		return BERR_SUCCESS;
	}

	ulBufSize = pHeapInfo->ulBufSize;

	for(j = 0; j < pParentHeapInfo->ulTotalBufCnt; j++)
	{
		/* Break a larger buffer into smaller buffers */
		pParentHeapNode = &(pParentHeapInfo->pBufList[j]);
		ulDeviceOffset = pParentHeapNode->ulDeviceOffset;
		ulBlockOffset = 0;

		for(i = 0; i < pHeapInfo->ulNodeCntPerParent; i++ )
		{
			/* Initialize fields. */
			pBufferHeapNode = &(pHeapInfo->pBufList[pHeapInfo->ulTotalBufCnt]);

			pBufferHeapNode->ulBufIndex   = pHeapInfo->ulTotalBufCnt;
			pBufferHeapNode->pHeapInfo    = pHeapInfo;
			pBufferHeapNode->bUsed        = false;
			pBufferHeapNode->ulNumChildNodeUsed = 0;
			pBufferHeapNode->ulDeviceOffset = ulDeviceOffset;
			pBufferHeapNode->ulBlockOffset = ulBlockOffset;
			pBufferHeapNode->ulParentNodeBufIndex = pParentHeapNode->ulBufIndex;
			pBufferHeapNode->eOrigBufHeapId = pParentHeapNode->eOrigBufHeapId;

			if( i == 0 )
			{
				pParentHeapNode->ulFirstChildNodeBufIndex = pBufferHeapNode->ulBufIndex;
				pBufferHeapNode->bContinous = pParentHeapNode->bContinous;
				if( (pHeapInfo->ulPrimaryBufCnt != 0) &&
				    (pBufferHeapNode->ulBufIndex == pHeapInfo->ulPrimaryBufCnt) )
				{
					pBufferHeapNode->bContinous = false;
				}
			}
			else
			{
				pBufferHeapNode->bContinous = true;
			}

			ulDeviceOffset += ulBufSize;
			ulBlockOffset += ulBufSize;

			pHeapInfo->ulTotalBufCnt++;
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
	uint32_t               ulTotalBufCnt;
	uint32_t               ulBufSize = pHeapInfo->ulBufSize;
	uint32_t               ulPrimaryBufCnt = pHeapInfo->ulPrimaryBufCnt;
	BERR_Code              err = BERR_SUCCESS;

	BDBG_MSG(("Create heapInfo [%p]: %7s Heap size: %8d (%d, %d)",
		(void *)pHeapInfo,
		BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(pHeapInfo->eBufHeapId),
		ulBufSize, pHeapInfo->ulWidth, pHeapInfo->ulHeight));

	/* A size of 0 yields a valid MMA block pointer so catch it here. */
	if (ulBufSize * ulPrimaryBufCnt == 0)
	{
		/* coverity[assign_zero] */
		pHeapInfo->hMmaBlock = NULL;
	}
	else
	{
		/* Allocate MMA block from heap. */
	    pHeapInfo->hMmaBlock = BMMA_Alloc(hMemory, ulBufSize * ulPrimaryBufCnt, 1<<BVDC_P_HEAP_MEMORY_ALIGNMENT, NULL);
		if( !pHeapInfo->hMmaBlock && (0 != ulPrimaryBufCnt) )
		{
			BDBG_ERR(("Not enough device memory[%d]!", ulBufSize * ulPrimaryBufCnt));
			BDBG_ASSERT(0);
			return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		}
	}
	/* (2) Create buffer heap node list */
	ulTotalBufCnt = ulPrimaryBufCnt;
	if(pHeapInfo->pParentHeapInfo)
		ulTotalBufCnt += pHeapInfo->ulNodeCntPerParent * pHeapInfo->pParentHeapInfo->ulTotalBufCnt;

	pHeapInfo->pBufList =
		(BVDC_P_BufferHeapNode *)BKNI_Malloc(sizeof(BVDC_P_BufferHeapNode)*ulTotalBufCnt);
	if( !pHeapInfo->pBufList )
	{
		if (pHeapInfo->hMmaBlock)
		{
			BMMA_Free(pHeapInfo->hMmaBlock);
		}
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}

	/* (3) Create buffer heap primitive nodes of its own */
	if (BVDC_P_BufferHeap_BuildPrimaryNodeList(hMemory, pHeapInfo) != BERR_SUCCESS)
	{
        BMMA_Free(pHeapInfo->hMmaBlock);
	}

	/* (4) Create buffer heap child nodes */
	if (BVDC_P_BufferHeap_BuildChildNodeList(hMemory, pHeapInfo) != BERR_SUCCESS)
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
	BKNI_Free((void*)pHeapInfo->pBufList);

	/* [1] Free block of memory */
	if (pHeapInfo->hMmaBlock)
		BMMA_Free(pHeapInfo->hMmaBlock);

	return BERR_SUCCESS;
}

#if 0
/***************************************************************************
 *
 */
 static void BVDC_P_BufferHeap_InitHeapInfo
	( BVDC_P_BufferHeap_Info      *pHeapInfo )
{
	uint32_t                  i;
	BVDC_P_BufferHeapNode    *pBufferHeapNode;

	for(i = 0; i < pHeapInfo->ulTotalBufCnt; i++)
	{
		pBufferHeapNode = &pHeapInfo->pBufList[i];
		pBufferHeapNode->bUsed = false;
	}

	pHeapInfo->ulBufUsed = 0;
	return;
}
#endif

/***************************************************************************
 *
 */
static BERR_Code BVDC_P_BufferHeap_MarkParentNode_isr
	( BVDC_P_BufferHeap_Info      *pParentHeapInfo,
	  BVDC_P_BufferHeapNode       *pCurrentNode )
{
	uint32_t    ulParentNodeBufIndex;
	BVDC_P_BufferHeapNode   *pParentHeapNode;

	if( !pParentHeapInfo || !pCurrentNode )
	{
		return BERR_SUCCESS;
	}

	/* Don't need to mark parent of primary nodes */
	if(pCurrentNode->ulParentNodeBufIndex == 0xffffffff)
	{
		return BERR_SUCCESS;
	}

	ulParentNodeBufIndex = pCurrentNode->ulParentNodeBufIndex;
	BDBG_ASSERT(ulParentNodeBufIndex < pParentHeapInfo->ulTotalBufCnt);
	pParentHeapNode = &pParentHeapInfo->pBufList[ulParentNodeBufIndex];
	if( pParentHeapNode )
	{
		pParentHeapNode->ulNumChildNodeUsed++;
		if( !pParentHeapNode->bUsed )
		{
			pParentHeapNode->bUsed = true;
			pParentHeapInfo->ulBufUsed++;
			BDBG_MSG(("Mark parent node: 0x%x", pParentHeapNode->ulDeviceOffset));
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
	uint32_t                  ulFirstChildNodeBufIndex;

	if( !pChildHeapInfo || !pCurrentNode )
	{
		return BERR_SUCCESS;
	}

	ulFirstChildNodeBufIndex = pCurrentNode->ulFirstChildNodeBufIndex;
	if(ulFirstChildNodeBufIndex == 0xffffffff)
	{
		return BERR_SUCCESS;
	}

	while(i < pChildHeapInfo->ulNodeCntPerParent)
	{
		pChildNode = &pChildHeapInfo->pBufList[ulFirstChildNodeBufIndex+i];
		if( pChildNode )
		{
			pChildNode->bUsed = true;
			pChildHeapInfo->ulBufUsed++;
			pCurrentNode->ulNumChildNodeUsed++;
			BVDC_P_BufferHeap_DumpHeapNode_isr(pChildNode);
			BDBG_MSG(("Mark child nodes: 0x%x", pChildNode->ulDeviceOffset));

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
	pHeapInfo->ulBufUsed++;
	BDBG_MSG(("Mark node: 0x%x", pBufferHeapNode->ulDeviceOffset));
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
	if(pCurrentNode->ulParentNodeBufIndex == 0xffffffff)
	{
		return BERR_SUCCESS;
	}

	BDBG_ASSERT(pCurrentNode->ulParentNodeBufIndex < pParentHeapInfo->ulTotalBufCnt);
	pParentHeapNode = &pParentHeapInfo->pBufList[pCurrentNode->ulParentNodeBufIndex];

	if(pParentHeapNode)
	{
		pParentHeapNode->ulNumChildNodeUsed--;
		BDBG_MSG(("Check parent node: "));

		if( pParentHeapNode->ulNumChildNodeUsed == 0 )
		{
			if( pParentHeapNode->bUsed )
			{
				pParentHeapNode->bUsed = false;
				pParentHeapInfo->ulBufUsed--;
				BDBG_MSG(("Unmark parent node: 0x%x", pParentHeapNode->ulDeviceOffset));
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
	uint32_t    i = 0, ulFirstChildNodeBufIndex;
	BVDC_P_BufferHeapNode   *pChildNode;

	if( !pChildHeapInfo || !pCurrentNode )
	{
		return BERR_SUCCESS;
	}

	ulFirstChildNodeBufIndex = pCurrentNode->ulFirstChildNodeBufIndex;
	if(ulFirstChildNodeBufIndex == 0xffffffff)
	{
		return BERR_SUCCESS;
	}

	while(i < pChildHeapInfo->ulNodeCntPerParent)
	{
		pChildNode = &pChildHeapInfo->pBufList[ulFirstChildNodeBufIndex+i];

		pChildNode->bUsed = false;
		pChildHeapInfo->ulBufUsed--;
		pCurrentNode->ulNumChildNodeUsed--;
		BVDC_P_BufferHeap_DumpHeapNode_isr(pChildNode);
		BDBG_MSG(("Unmark child nodes: 0x%x", pChildNode->ulDeviceOffset));
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
	pBufferHeapNode->ulNumChildNodeUsed = 0;
	*/
	pHeapInfo->ulBufUsed--;
	BDBG_MSG(("UnMark node: 0x%x", pBufferHeapNode->ulDeviceOffset));
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
		while((j < pHeapInfo->ulTotalBufCnt) && !bFound)
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
			BDBG_MSG(("Found an available node: 0x%x", pBufferHeapNode->ulDeviceOffset));

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
	while((i < pHeapInfo->ulTotalBufCnt) && !bFound)
	{
		pTempNode = &pHeapInfo->pBufList[i];
		if(pTempNode->ulDeviceOffset != pHeapNode->ulDeviceOffset)
			i++;
		else
			bFound = true;
	}

	if( pTempNode && pTempNode->bUsed & bFound)
	{
		/* Unmark the node and its parent node */
		BVDC_P_BufferHeap_UnmarkNode_isr(pTempNode);
		BDBG_MSG(("Free node: 0x%x", pTempNode->ulDeviceOffset));
	}
	else
	{
		BDBG_MSG(("Can not find a matching node %p at 0x%x (%d)",
			(void *)pTempNode, pTempNode->ulDeviceOffset, pTempNode->bUsed));

		BVDC_P_BufferHeap_Dump_isr(pHeapInfo->hBufferHeap);
		BDBG_ASSERT(0);
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	BDBG_ASSERT(pHeapInfo->ulBufUsed <= pHeapInfo->ulTotalBufCnt);

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
		while((j < pHeapInfo->ulTotalBufCnt) && !bStartNodeFound)
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

		BDBG_MSG(("Find first available node(%d): 0x%x",
			ulAllocCnt, pBufferHeapNodeStart->ulDeviceOffset));
		BVDC_P_BufferHeap_DumpHeapNode_isr(pBufferHeapNodeStart);

		bNext = true;
		pBufferHeapTempNode = pBufferHeapNodeStart;
		while( (ulAllocCnt < ulCount) && (j < pHeapInfo->ulTotalBufCnt))
		{
			pBufferHeapTempNode= &pHeapInfo->pBufList[j];
			if( pBufferHeapTempNode->bUsed )
			{
				BDBG_MSG(("Find a non-available node: 0x%x", pBufferHeapTempNode->ulDeviceOffset));
				BVDC_P_BufferHeap_DumpHeapNode_isr(pBufferHeapTempNode);
				bNext  = false;
				break;
			}
			else if( !pBufferHeapTempNode->bContinous )
			{
				BDBG_MSG(("Find a non-continous node: 0x%x", pBufferHeapTempNode->ulDeviceOffset));
				BVDC_P_BufferHeap_DumpHeapNode_isr(pBufferHeapTempNode);
				bNext  = false;
				break;
			}
			else
			{
				ulAllocCnt++;
				BDBG_MSG(("Find next available node (%d): 0x%x", ulAllocCnt, pBufferHeapTempNode->ulDeviceOffset));
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
			BDBG_MSG(("Find %d available nodes 0x%x", ulCount, pBufferHeapNodeStart->ulDeviceOffset));
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
	while((j < pHeapInfo->ulTotalBufCnt) && !bFound)
	{
		ulStartNodeIndex = j;
		pTempNode = &pHeapInfo->pBufList[j];
		if(pTempNode->ulDeviceOffset != (*pHeapNode)->ulDeviceOffset)
			j++;
		else
			bFound = true;
	}

	if( pTempNode && bFound)
	{
		for(i = 0; i < ulCount; i++ )
		{
			pTempNode = &pHeapInfo->pBufList[ulStartNodeIndex+i];
			BDBG_MSG(("Free Cnt node: 0x%x", pTempNode->ulDeviceOffset));
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

	BDBG_ASSERT(pHeapInfo->ulBufUsed <= pHeapInfo->ulTotalBufCnt);
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

	pBufferHeap->hMosaicDrainBlock = BMMA_Alloc(pBufferHeap->hMem,
			    16, /* 2 pixels wide, in case 10-bit 4:2:2 capture rounding; */
			    16,  /* 16 bytes aligned for capture engine */
			    NULL);

	if(!pBufferHeap->hMosaicDrainBlock)
	{
		BDBG_ERR(("Not enough device memory"));
		BDBG_ASSERT(0);
		return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
	}

	pBufferHeap->ulMosaicDrainOffset = (uint32_t) BMMA_LockOffset(pBufferHeap->hMosaicDrainBlock);

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

	if(hBufferHeap->hMosaicDrainBlock)
	{
		/* free drain buffer */
		BMMA_UnlockOffset(hBufferHeap->hMosaicDrainBlock, hBufferHeap->ulMosaicDrainOffset);
		BMMA_Free(hBufferHeap->hMosaicDrainBlock);
	}

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


#if 0
/***************************************************************************
 *
 */
void BVDC_P_BufferHeap_Init
	( BVDC_P_BufferHeap_Handle     hBufferHeap )
{
	uint32_t                  i;

	BDBG_ENTER(BVDC_P_BufferHeap_Init);
	BDBG_OBJECT_ASSERT(hBufferHeap, BVDC_BFH);

	/* Re-Initialize fields that may changes during previous run. */
	for( i = 0; i < BVDC_P_BufferHeapId_eCount; i++ )
	{
		BVDC_P_BufferHeap_InitHeapInfo(&(hBufferHeap->astHeapInfo[i]));
	}

	BDBG_LEAVE(BVDC_P_BufferHeap_Init);
	return;
}
#endif

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
	ulBufferAvailable = pHeapInfo->ulTotalBufCnt - pHeapInfo->ulBufUsed;

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
	  BCHP_MemoryInfo               *pMemoryInfo,
	  uint32_t                       ulMemcIndex,
	  BVDC_DisplayId                 eDispId,
	  BVDC_WindowId                  eWinId )
{
	uint32_t    i;
	uint64_t    ulHeapStartOffset, ulHeapEndOffset;
	uint64_t    ulMemcStartOffset, ulMemcEndOffset;
	BVDC_P_BufferHeap_Info   *pHeapInfo;

#if (!BDBG_DEBUG_BUILD)
	BSTD_UNUSED(eDispId);
	BSTD_UNUSED(eWinId);
#endif

	BDBG_ASSERT(pHeap);
	BDBG_ASSERT(pMemoryInfo);
	BDBG_ASSERT(ulMemcIndex != BBOX_MemcIndex_Invalid);

	if(ulMemcIndex == BBOX_VDC_DISREGARD)
		return BERR_SUCCESS;

	ulMemcStartOffset = (uint64_t)pMemoryInfo->memc[ulMemcIndex].offset;
	ulMemcEndOffset = ulMemcStartOffset + pMemoryInfo->memc[ulMemcIndex].size;

	/* TODO: Call BCHP API to validate heap (SW7445-2325) */
	for(i = 0; i < BVDC_P_BufferHeapId_eCount; i++)
	{
		pHeapInfo = &pHeap->astHeapInfo[i];
		if(pHeapInfo->ulPrimaryBufCnt)
		{
			ulHeapStartOffset = (uint64_t)pHeapInfo->pBufList[0].ulDeviceOffset;
			ulHeapEndOffset = ulHeapStartOffset + (uint64_t)pHeapInfo->ulBufSize;

			BDBG_MODULE_MSG(BVDC_MEMC_INDEX_CHECK,
				("Boxmode[%d]: Memc[%d] device offset: " BDBG_UINT64_FMT " - " BDBG_UINT64_FMT,
				pHeap->hVdc->stBoxConfig.stBox.ulBoxId, ulMemcIndex,
				BDBG_UINT64_ARG(ulMemcStartOffset), BDBG_UINT64_ARG(ulMemcEndOffset)));
			BDBG_MODULE_MSG(BVDC_MEMC_INDEX_CHECK,
				("Window Heap[%s]: device offset: " BDBG_UINT64_FMT "-  " BDBG_UINT64_FMT,
				BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(pHeapInfo->eBufHeapId),
				BDBG_UINT64_ARG(ulHeapStartOffset), BDBG_UINT64_ARG(ulHeapEndOffset)));

			if( !((ulHeapStartOffset >= ulMemcStartOffset) &&
			    (ulHeapStartOffset < ulMemcEndOffset) &&
			    (ulHeapEndOffset > ulMemcStartOffset) &&
			    (ulHeapEndOffset <= ulMemcEndOffset)) )
			{
				BDBG_MODULE_MSG(BVDC_MEMC_INDEX_CHECK,
					("Disp[%d]Win[%d] Possible mismatch between Memory heap and Box mode settings:",
					eDispId, eWinId));

#if 0
				return BERR_INVALID_PARAMETER;
#endif
			}
		}
	}

	return BERR_SUCCESS;
}
#endif


/* End of file. */

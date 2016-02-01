/***************************************************************************
*     Copyright (c) 2004-2013, Broadcom Corporation
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
#ifndef BVDC_MCVP_PRIV_H__
#define BVDC_MCVP_PRIV_H__

#include "bavc.h"
#include "breg_mem.h"      /* Chip register access (memory mapped). */
#include "bvdc_common_priv.h"
#include "bvdc_bufferheap_priv.h"
#include "bvdc_buffer_priv.h"
#include "bvdc_subrul_priv.h"
#include "bvdc_window_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

	BDBG_OBJECT_ID_DECLARE(BVDC_ANR);
	BDBG_OBJECT_ID_DECLARE(BVDC_MDI);

/***************************************************************************
* {private}
*
* Mcvp Sub-module Overview:
*
*/

/*-------------------------------------------------------------------------
* macro used by mcvp sub-module
*/
#define BVDC_P_MCVP_BUFFER_MAX_COUNT            (4)

/* MCVP Versions */
#define BVDC_P_MCVP_VER_1                       (1) /* 7420 */
#define BVDC_P_MCVP_VER_2                       (2) /* 7422Ax/7425Ax */
#define BVDC_P_MCVP_VER_3                       (3) /* 7231Ax/7344Ax/7346Ax/7358Ax/7552Ax */
#define BVDC_P_MCVP_VER_4                       (4) /* 7366 */
#define BVDC_P_MCVP_VER_5                       (5) /* 7445 D0 multi context support DCX MADR/MCVP enum conflict*/
#define BVDC_P_MCVP_VER_6                       (6) /* 7364 Ax 7439 B0 MVP_TOP_1_DITHER_CTR*/



#ifdef BCHP_MVP_TOP_5_REG_START
#define BVDC_P_MVP_GET_REG_OFFSET(eMcvpId) \
	((BVDC_P_McvpId_eMcvp5==(eMcvpId)) ? (BCHP_MVP_TOP_5_REG_START - BCHP_MVP_TOP_0_REG_START) \
	:(BVDC_P_McvpId_eMcvp4==(eMcvpId)) ? (BCHP_MVP_TOP_4_REG_START - BCHP_MVP_TOP_0_REG_START) \
	:(BVDC_P_McvpId_eMcvp3==(eMcvpId)) ? (BCHP_MVP_TOP_3_REG_START - BCHP_MVP_TOP_0_REG_START) \
	:(BVDC_P_McvpId_eMcvp2==(eMcvpId)) ? (BCHP_MVP_TOP_2_REG_START - BCHP_MVP_TOP_0_REG_START) \
	:(BVDC_P_McvpId_eMcvp1==(eMcvpId)) ? (BCHP_MVP_TOP_1_REG_START - BCHP_MVP_TOP_0_REG_START) \
	:(0))
#else
#ifdef BCHP_MVP_TOP_4_REG_START
#define BVDC_P_MVP_GET_REG_OFFSET(eMcvpId) \
	((BVDC_P_McvpId_eMcvp4==(eMcvpId)) ? (BCHP_MVP_TOP_4_REG_START - BCHP_MVP_TOP_0_REG_START) \
	:(BVDC_P_McvpId_eMcvp3==(eMcvpId)) ? (BCHP_MVP_TOP_3_REG_START - BCHP_MVP_TOP_0_REG_START) \
	:(BVDC_P_McvpId_eMcvp2==(eMcvpId)) ? (BCHP_MVP_TOP_2_REG_START - BCHP_MVP_TOP_0_REG_START) \
	:(BVDC_P_McvpId_eMcvp1==(eMcvpId)) ? (BCHP_MVP_TOP_1_REG_START - BCHP_MVP_TOP_0_REG_START) \
	:(0))
#else
#ifdef BCHP_MVP_TOP_3_REG_START
#define BVDC_P_MVP_GET_REG_OFFSET(eMcvpId) \
	((BVDC_P_McvpId_eMcvp3==(eMcvpId)) ? (BCHP_MVP_TOP_3_REG_START - BCHP_MVP_TOP_0_REG_START) \
	:(BVDC_P_McvpId_eMcvp2==(eMcvpId)) ? (BCHP_MVP_TOP_2_REG_START - BCHP_MVP_TOP_0_REG_START) \
	:(BVDC_P_McvpId_eMcvp1==(eMcvpId)) ? (BCHP_MVP_TOP_1_REG_START - BCHP_MVP_TOP_0_REG_START) \
	:(0))
#else
#ifdef BCHP_MVP_TOP_2_REG_START
#define BVDC_P_MVP_GET_REG_OFFSET(eMcvpId) \
	((BVDC_P_McvpId_eMcvp2==(eMcvpId)) ? (BCHP_MVP_TOP_2_REG_START - BCHP_MVP_TOP_0_REG_START) \
	:(BVDC_P_McvpId_eMcvp1==(eMcvpId)) ? (BCHP_MVP_TOP_1_REG_START - BCHP_MVP_TOP_0_REG_START) \
	:(0))
#else
#ifdef BCHP_MVP_TOP_1_REG_START
#define BVDC_P_MVP_GET_REG_OFFSET(eMcvpId) \
	((BVDC_P_McvpId_eMcvp1==(eMcvpId)) ? (BCHP_MVP_TOP_1_REG_START - BCHP_MVP_TOP_0_REG_START) \
	:(0))
#else
#define BVDC_P_MVP_GET_REG_OFFSET(eMcvpId)           (0)
#endif /* Mcvp_1 */
#endif /* Mcvp_2 */
#endif /* Mcvp_3 */
#endif /* Mcvp_4 */
#endif /* Mcvp_5 */
/****************************************************************************
* Mcvp dirty bits to makr RUL building and executing dirty.
*/
typedef union
{
	struct {
		uint32_t                           bSize           : 1;
		uint32_t                           bPrevBypass     : 1;
	} stBits;

	uint32_t aulInts [BVDC_P_DIRTY_INT_ARRAY_SIZE];
} BVDC_P_McvpDirtyBits;


/*-------------------------------------------------------------------------
* mcvp main context
*/
typedef struct BVDC_P_McvpContext
{
	BDBG_OBJECT(BVDC_MVP)

	/* mcvp Id */
	BVDC_P_McvpId                      eId;
	uint32_t                           ulMaxWidth; /* max width limited by line buf size */
	uint32_t                           ulMaxHeight; /* max height limited by RTS */
	uint32_t                           ulHsclSizeThreshold; /* hsize that triggers use of HSCL before deinterlacing */
	uint32_t                           ulRegOffset;

	/* Core & Vnet Channel Reset */
	uint32_t                           ulCoreResetAddr;
	uint32_t                           ulCoreResetMask;
	uint32_t                           ulVnetResetAddr;
	uint32_t                           ulVnetResetMask;
	uint32_t                           ulVnetMuxAddr;
	uint32_t                           ulVnetMuxValue;
	uint32_t                           ulUpdateAll[BAVC_MOSAIC_MAX];

	/* static info from creating */
	BREG_Handle                        hRegister;

	/* from acquireConnect */
	BVDC_Heap_Handle                   hHeap;
	BVDC_Window_Handle                 hWindow;

	/* sub-modules */
	BVDC_P_Hscaler_Handle              hHscaler;
	BVDC_P_Anr_Handle                  hAnr;
	BVDC_P_Mcdi_Handle                 hMcdi;
	BVDC_P_MvpDcxCore                  eDcxCore;
	bool                               bAnr;

	/* stream properties */
	uint32_t                           ulHSize;
	uint32_t                           ulVSize;
	bool                               bChannelChange[BAVC_MOSAIC_MAX];

	/* user settings */
	BVDC_Mode                          ePqEnhancement;

	/* buffers */
	uint32_t                           ulPixelBufCnt;
	uint32_t                           ulQmBufCnt;
	/*BVDC_P_HeapNodePtr               apHeapNode[BVDC_P_MCVP_BUFFER_MAX_COUNT];*/

	/* sub-struct to manage vnet and rul build opreations */
	BVDC_P_SubRulContext               SubRul;

	BVDC_P_VnetMode                    stMcvpMode[BAVC_MOSAIC_MAX];
} BVDC_P_McvpContext;


/***************************************************************************
* private functions
***************************************************************************/

#define BVDC_P_Mcvp_MuxAddr(hMcvp)      (hMcvp->ulVnetMuxAddr)
#define BVDC_P_Mcvp_PostMuxValue(hMcvp) (hMcvp->ulVnetMuxValue)
#define BVDC_P_Mcvp_SetVnet_isr(hMcvp, ulSrcMuxValue, eVnetPatchMode) \
	BVDC_P_SubRul_SetVnet_isr(&((hMcvp)->SubRul), ulSrcMuxValue, eVnetPatchMode)
#define BVDC_P_Mcvp_UnsetVnet_isr(hMcvp) \
	BVDC_P_SubRul_UnsetVnet_isr(&((hMcvp)->SubRul))


/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_Create
*
* called by BVDC_Open only
*/
BERR_Code BVDC_P_Mcvp_Create
	( BVDC_P_Mcvp_Handle *           phMcvp,
	BVDC_P_McvpId                    eMcvpId,
	BREG_Handle                      hRegister,
	BVDC_P_Resource_Handle           hResource );

/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_Destroy
*
* called by BVDC_Close only
*/
BERR_Code BVDC_P_Mcvp_Destroy
	( BVDC_P_Mcvp_Handle               hMcvp );

/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_AcquireConnect_isr
*
* It is called by BVDC_Window_Validate after changing from disable mcvp to
* enable mcvp.
*/
BERR_Code BVDC_P_Mcvp_AcquireConnect_isr
	( BVDC_P_Mcvp_Handle               hMcvp,
	BVDC_Heap_Handle                   hHeap,
	BVDC_Window_Handle                 hWindow);


/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_ReleaseConnect_isr
*
* It is called after window decided that mcvp is no-longer used by HW in its
* vnet mode (i.e. it is really shut down and teared off from vnet).
*/
BERR_Code BVDC_P_Mcvp_ReleaseConnect_isr
	( BVDC_P_Mcvp_Handle              *phMcvp );

/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_SetVnetAllocBuf_isr
*
* Called by BVDC_P_*_BuildRul_isr to setup for joinning into vnet (including
* optionally acquiring loop-back) and allocate buffers
*/
void BVDC_P_Mcvp_SetVnetAllocBuf_isr
	( BVDC_P_Mcvp_Handle               hMcvp,
	  uint32_t                         ulSrcMuxValue,
	  BVDC_P_VnetPatch                 eVnetPatchMode,
	  bool                             bRfcgVnet);

/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_UnsetVnetFreeBuf_isr
*
* called by BVDC_P_Window_UnsetWriter(Reader)Vnet_isr to to release the
* potentially used loop-back, and free buffers
*/
void BVDC_P_Mcvp_UnsetVnetFreeBuf_isr
	( BVDC_P_Mcvp_Handle                hMcvp );

/***************************************************************************
* {private}
*
* BVDC_P_MCVP_SetInfo_isr
*
* called by BVDC_P_Window_Writer(Reader)_isr to to detect size difference between
* two continuous rul
*/
void BVDC_P_MCVP_SetInfo_isr
	(BVDC_P_Mcvp_Handle                 hMcvp,
	BVDC_Window_Handle                 hWindow,
	BVDC_P_PictureNode                *pPicture);

/***************************************************************************
* {private}
*
* BVDC_P_Mcvp_BuildRul_isr
*
* called by BVDC_Window_BuildRul_isr at every src or vec vsync (depending on
* whether reader side or writer side is using this module)
*
* Input:
*    eVnetState - reader or writer window/vnet state
*    pPicComRulInfo - the PicComRulInfo that is the shared Picture info by
*      all sub-modules when they build rul.
*/
void BVDC_P_Mcvp_BuildRul_isr(
	BVDC_P_Mcvp_Handle                 hMcvp,
	BVDC_P_ListInfo                   *pList,
	BVDC_P_State                       eVnetState,
	BVDC_P_WindowContext              *pWindow,
	BVDC_P_PictureNode                *pPicture );


#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_MCVP_PRIV_H__ */
/* End of file. */

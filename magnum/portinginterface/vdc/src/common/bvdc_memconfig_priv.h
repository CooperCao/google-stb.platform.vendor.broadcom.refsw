/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
#ifndef BVDC_MEMCONFIG_PRIV_H__
#define BVDC_MEMCONFIG_PRIV_H__

#include "bvdc.h"
#include "bvdc_common_priv.h"
#include "bvdc_bufferheap_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const BVDC_P_Features s_VdcFeatures;
extern const BVDC_Settings s_stDefaultSettings;

/***************************************************************************
 * Private macros
 ***************************************************************************/
#define BVDC_P_MEMCONFIG_DEINTERLACER_ON(eDeinterlacerMode)   \
	((eDeinterlacerMode == BVDC_DeinterlacerMode_eBestQuality) || \
	 (eDeinterlacerMode == BVDC_DeinterlacerMode_eLowestLatency))

typedef struct
{
	/* Total number available */
	uint32_t                     ulNumCmp;  /* CMP count */
	uint32_t                     ulNumStg;  /* STG count, <= ulNumCmp */
	uint32_t                     ulNumMad;  /* jTotal Deinterlacer count, including ulNumMadr */
	uint32_t                     ulNumMadr; /* Madr count, <= ulNumMad */

	/* Number used */
	uint32_t                     ulNumCmpUsed;
	uint32_t                     ulNumStgUsed;
	uint32_t                     ulNumMadUsed;
	uint32_t                     ulNumMadrUsed;

	bool                         bSingleMemc;
	BVDC_P_BufferHeap_SizeInfo   stHeapSizeInfo;

} BVDC_P_MemConfig_SystemInfo;

typedef struct
{
	/* These flags control capture buffer count */
	bool                     bSyncLock;
	bool                     bCapture;
	bool                     bPip;
	bool                     bLipsync;
	bool                     b5060Convert;
	bool                     bSlave_24_25_30_Display;

	/* This flags control deinterlcer buffer count */
	bool                     bMadr;
	BVDC_DeinterlacerMode    eDeinterlacerMode;

	/* This flags control buffer size */
	bool                     b3d;
	bool                     bMosaicMode;
	BFMT_VideoFmt            eFormat;
	/* TODO: 10bit 422 for 7445 Dx
	bool                     b10Bit422;
	*/
	uint32_t                 ulAdditionalBufCnt;
	uint32_t                 aulCapBufCnt[BVDC_P_BufferHeapId_eCount];
	uint32_t                 aulMadBufCnt[BVDC_P_BufferHeapId_eCount];

} BVDC_P_MemConfig_WindowInfo;

/***************************************************************************
 * Memory private functions
 ***************************************************************************/
BERR_Code BVDC_P_MemConfig_GetDefaultRdcSettings
	( BVDC_RdcMemConfigSettings          *pRdc );

BERR_Code BVDC_P_MemConfig_GetDefaultDisplaySettings
	( BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo,
	  uint32_t                            ulDispIndex,
	  BVDC_DispMemConfigSettings         *pDisplay );

BERR_Code BVDC_P_MemConfig_GetDefaultWindowSettings
	( BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo,
	  uint32_t                            ulDispIndex,
	  uint32_t                            ulWinIndex,
	  BVDC_WinMemConfigSettings          *pWindow );

BERR_Code BVDC_P_MemConfigInfo_Init
	( BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo );

BERR_Code BVDC_P_MemConfig_GetBufSize
	( const BVDC_Heap_Settings           *pHeapSettings,
	  BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo );

BERR_Code BVDC_P_MemConfig_Validate
	( const BVDC_MemConfigSettings       *pMemConfigSettings,
	  BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo );

BERR_Code BVDC_P_MemConfig_GetWindowInfo
	( BVDC_WinMemConfigSettings          *pWindow,
	  BVDC_DispMemConfigSettings         *pDisplay,
	  BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo,
	  uint32_t                            ulDispIndex,
	  uint32_t                            ulWinIndex,
	  BVDC_P_MemConfig_WindowInfo        *pWinConfigInfo );

BERR_Code BVDC_P_MemConfig_GetWindowBufCnt
	( BVDC_WinMemConfigSettings          *pWindow,
	  BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo,
	  BVDC_P_MemConfig_WindowInfo        *pWinConfigInfo,
	  uint32_t                            ulDispIndex,
	  uint32_t                            ulWinIndex );

BERR_Code BVDC_P_MemConfig_GetWinBufSize
	( BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo,
	  BVDC_P_MemConfig_WindowInfo        *pWinConfigInfo,
	  uint32_t                           *pulCapSize,
	  uint32_t                           *pulMadSize );

BERR_Code BVDC_P_MemConfig_SetBufFormat
	( const BVDC_Heap_Settings           *pHeapSettingsIn,
	  BVDC_Heap_Settings                 *pHeapSettingsOut );

BERR_Code BVDC_P_MemConfig_GetRulSize
	( uint32_t                           *pulRulSize );


#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_MEMCONFIG_PRIV_H__*/

/* End of file. */

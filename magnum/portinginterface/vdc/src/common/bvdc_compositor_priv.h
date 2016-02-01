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
#ifndef BVDC_COMPOSITOR_PRIV_H__
#define BVDC_COMPOSITOR_PRIV_H__

#include "bvdc.h"
#include "bchp_common.h"
#include "bvdc_common_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_display_priv.h"
#include "bchp_cmp_0.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * Private register cracking macros
 ***************************************************************************/
#define BVDC_P_CMP_OFFSET_GET_REG_IDX(reg, offset) \
	((BCHP##_##reg + offset - BCHP_CMP_0_REG_START) / sizeof(uint32_t))

/* Get/Set reg data */
#define BVDC_P_CMP_OFFSET_GET_REG_DATA(reg, offset) \
	(hCompositor->aulRegs[BVDC_P_CMP_OFFSET_GET_REG_IDX(reg, offset)])
#define BVDC_P_CMP_OFFSET_SET_REG_DATA(reg, offset, data) \
	(BVDC_P_CMP_OFFSET_GET_REG_DATA(reg, offset) = (uint32_t)(data))

#define BVDC_P_CMP_OFFSET_GET_REG_DATA_I(idx, reg, offset) \
	(hCompositor->aulRegs[BVDC_P_CMP_OFFSET_GET_REG_IDX(reg,offset) + (idx)])

/* Get field */
#define BVDC_P_CMP_OFFSET_GET_FIELD_NAME(reg, offset, field) \
	(BVDC_P_GET_FIELD(BVDC_P_CMP_OFFSET_GET_REG_DATA(reg, offset), reg, field))

/* Compare field */
#define BVDC_P_CMP_OFFSET_COMPARE_FIELD_DATA(reg, offset, field, data) \
	(BVDC_P_COMPARE_FIELD_DATA(BVDC_P_CMP_OFFSET_GET_REG_DATA(reg, offset), reg, field, (data)))
#define BVDC_P_CMP_OFFSET_COMPARE_FIELD_NAME(reg, offset, field, name) \
	(BVDC_P_COMPARE_FIELD_NAME(BVDC_P_CMP_OFFSET_GET_REG_DATA(reg, offset), reg, field, name))

/* This macro does a write into RUL (write, addr, data). 3 dwords. */
#define BVDC_P_CMP_OFFSET_WRITE_TO_RUL(reg, offset, addr_ptr) \
{ \
	*addr_ptr++ = BRDC_OP_IMM_TO_REG(); \
	*addr_ptr++ = BRDC_REGISTER(BCHP##_##reg + hCompositor->ulRegOffset + offset); \
	*addr_ptr++ = BVDC_P_CMP_OFFSET_GET_REG_DATA(reg, offset); \
}


/* This macro does a block write into RUL */
#define BVDC_P_CMP_OFFSET_BLOCK_WRITE_TO_RUL(from, to, offset, pulCurrent) \
do { \
	uint32_t ulBlockSize = \
		BVDC_P_REGS_ENTRIES(from, to);\
	*pulCurrent++ = BRDC_OP_IMMS_TO_REGS( ulBlockSize ); \
	*pulCurrent++ = BRDC_REGISTER(BCHP##_##from + hCompositor->ulRegOffset+ offset); \
	BKNI_Memcpy((void*)pulCurrent, \
		(void*)&(hCompositor->aulRegs[BVDC_P_CMP_OFFSET_GET_REG_IDX(from, offset)]), \
		ulBlockSize * sizeof(uint32_t)); \
	pulCurrent += ulBlockSize; \
} while(0)

/* This macro does a block write into RUL */
#define BVDC_P_CMP_OFFSET_RECT_BLOCK_WRITE_TO_RUL(from, cnt, offset, pulCurrent) \
do { \
	*pulCurrent++ = BRDC_OP_IMMS_TO_REGS( cnt ); \
	*pulCurrent++ = BRDC_REGISTER(BCHP##_##from + hCompositor->ulRegOffset + offset); \
	BKNI_Memcpy((void*)pulCurrent, \
		(void*)&(hCompositor->aulRegs[BVDC_P_CMP_OFFSET_GET_REG_IDX(from, offset)]), \
		(cnt) * sizeof(uint32_t)); \
	pulCurrent += cnt; \
} while(0)

#define BVDC_P_CMP_GET_REG_IDX(reg) BVDC_P_CMP_OFFSET_GET_REG_IDX(reg, 0)

	/* Get/Set reg data */
#define BVDC_P_CMP_GET_REG_DATA(reg) \
	BVDC_P_CMP_OFFSET_GET_REG_DATA(reg, 0)

#define BVDC_P_CMP_SET_REG_DATA(reg, data) \
	BVDC_P_CMP_OFFSET_SET_REG_DATA(reg, 0, data)

#define BVDC_P_CMP_GET_REG_DATA_I(idx, reg) \
	BVDC_P_CMP_OFFSET_GET_REG_DATA_I(idx, reg, 0)

	/* Get field */
#define BVDC_P_CMP_GET_FIELD_NAME(reg, field) \
	BVDC_P_CMP_OFFSET_GET_FIELD_NAME(reg, 0, field)

	/* Compare field */
#define BVDC_P_CMP_COMPARE_FIELD_DATA(reg, field, data) \
	BVDC_P_CMP_OFFSET_COMPARE_FIELD_DATA(reg, 0, field, data)
#define BVDC_P_CMP_COMPARE_FIELD_NAME(reg, field, name) \
	BVDC_P_CMP_OFFSET_COMPARE_FIELD_NAME(reg, 0, field, name)

	/* This macro does a write into RUL (write, addr, data). 3 dwords. */
#define BVDC_P_CMP_WRITE_TO_RUL(reg, addr_ptr) \
	BVDC_P_CMP_OFFSET_WRITE_TO_RUL(reg, 0, addr_ptr)

	/* This macro does a block write into RUL */
#define BVDC_P_CMP_BLOCK_WRITE_TO_RUL(from, to, pulCurrent) \
	BVDC_P_CMP_OFFSET_BLOCK_WRITE_TO_RUL(from, to, 0, pulCurrent)

	/* This macro does a block write into RUL */
#define BVDC_P_CMP_RECT_BLOCK_WRITE_TO_RUL(from, cnt, pulCurrent) \
	BVDC_P_CMP_OFFSET_BLOCK_WRITE_TO_RUL(from, cnt, 0, pulCurrent)
#define BVDC_P_CMP_REGS_COUNT \
	BVDC_P_REGS_ENTRIES(CMP_0_REG_START, CMP_0_REG_END)

/* get register offset base on id. */
#ifdef BCHP_CMP_6_REG_START
#define BVDC_P_CMP_GET_REG_OFFSET(cmp_id) \
	((BVDC_CompositorId_eCompositor6==(cmp_id)) ? (BCHP_CMP_6_REG_START - BCHP_CMP_0_REG_START) \
	:(BVDC_CompositorId_eCompositor5==(cmp_id)) ? (BCHP_CMP_5_REG_START - BCHP_CMP_0_REG_START) \
	:(BVDC_CompositorId_eCompositor4==(cmp_id)) ? (BCHP_CMP_4_REG_START - BCHP_CMP_0_REG_START) \
	:(BVDC_CompositorId_eCompositor3==(cmp_id)) ? (BCHP_CMP_3_REG_START - BCHP_CMP_0_REG_START) \
	:(BVDC_CompositorId_eCompositor2==(cmp_id)) ? (BCHP_CMP_2_REG_START - BCHP_CMP_0_REG_START) \
	:(BVDC_CompositorId_eCompositor1==(cmp_id)) ? (BCHP_CMP_1_REG_START - BCHP_CMP_0_REG_START) \
	:(0))
#else
#ifdef BCHP_CMP_5_REG_START
#define BVDC_P_CMP_GET_REG_OFFSET(cmp_id) \
	((BVDC_CompositorId_eCompositor5==(cmp_id)) ? (BCHP_CMP_5_REG_START - BCHP_CMP_0_REG_START) \
	:(BVDC_CompositorId_eCompositor4==(cmp_id)) ? (BCHP_CMP_4_REG_START - BCHP_CMP_0_REG_START) \
	:(BVDC_CompositorId_eCompositor3==(cmp_id)) ? (BCHP_CMP_3_REG_START - BCHP_CMP_0_REG_START) \
	:(BVDC_CompositorId_eCompositor2==(cmp_id)) ? (BCHP_CMP_2_REG_START - BCHP_CMP_0_REG_START) \
	:(BVDC_CompositorId_eCompositor1==(cmp_id)) ? (BCHP_CMP_1_REG_START - BCHP_CMP_0_REG_START) \
	:(0))
#else
#ifdef BCHP_CMP_4_REG_START
#define BVDC_P_CMP_GET_REG_OFFSET(cmp_id) \
	((BVDC_CompositorId_eCompositor4==(cmp_id)) ? (BCHP_CMP_4_REG_START - BCHP_CMP_0_REG_START) \
	:(BVDC_CompositorId_eCompositor3==(cmp_id)) ? (BCHP_CMP_3_REG_START - BCHP_CMP_0_REG_START) \
	:(BVDC_CompositorId_eCompositor2==(cmp_id)) ? (BCHP_CMP_2_REG_START - BCHP_CMP_0_REG_START) \
	:(BVDC_CompositorId_eCompositor1==(cmp_id)) ? (BCHP_CMP_1_REG_START - BCHP_CMP_0_REG_START) \
	:(0))
#else
#ifdef BCHP_CMP_3_REG_START
#define BVDC_P_CMP_GET_REG_OFFSET(cmp_id) \
	((BVDC_CompositorId_eCompositor3==(cmp_id)) ? (BCHP_CMP_3_REG_START - BCHP_CMP_0_REG_START) \
	:(BVDC_CompositorId_eCompositor2==(cmp_id)) ? (BCHP_CMP_2_REG_START - BCHP_CMP_0_REG_START) \
	:(BVDC_CompositorId_eCompositor1==(cmp_id)) ? (BCHP_CMP_1_REG_START - BCHP_CMP_0_REG_START) \
	:(0))
#else
#ifdef BCHP_CMP_2_REG_START
#define BVDC_P_CMP_GET_REG_OFFSET(cmp_id) \
	((BVDC_CompositorId_eCompositor2==(cmp_id)) ? (BCHP_CMP_2_REG_START - BCHP_CMP_0_REG_START) \
	:(BVDC_CompositorId_eCompositor1==(cmp_id)) ? (BCHP_CMP_1_REG_START - BCHP_CMP_0_REG_START) \
	:(0))
#else
#ifdef BCHP_CMP_1_REG_START
#define BVDC_P_CMP_GET_REG_OFFSET(cmp_id) \
	((BVDC_CompositorId_eCompositor1==(cmp_id)) ? (BCHP_CMP_1_REG_START - BCHP_CMP_0_REG_START) \
	:(0))
#else
#define BVDC_P_CMP_GET_REG_OFFSET(cmp_id)           (0)
#endif /* CMP_1 */
#endif /* CMP_2 */
#endif /* CMP_3 */
#endif /* CMP_4 */
#endif /* CMP_5 */
#endif /* CMP_6 */

#define BVDC_P_CMP_GET_LIST_IDX(polarity_id, idx) \
	(BVDC_P_MAX_MULTI_RUL_BUFFER_COUNT * (polarity_id) + (idx))

/* For RUL multi-buffering. */
#define BVDC_P_CMP_NEXT_RUL(hCompositor, polarity_id) \
	((hCompositor)->aulRulIdx[(polarity_id)] = \
	BVDC_P_NEXT_RUL_IDX((hCompositor)->aulRulIdx[(polarity_id)]))

/* Get the current list pointed by aulRulIdx[field]. */
#define BVDC_P_CMP_GET_LIST(hCompositor, polarity_id) \
	((hCompositor)->ahList[BVDC_P_CMP_GET_LIST_IDX((polarity_id), \
		(hCompositor)->aulRulIdx[(polarity_id)])])

/* Compositor only uses T/B slot. */
#define BVDC_P_CMP_MAX_SLOT_COUNT \
	(2)

#define BVDC_P_CMP_MAX_LIST_COUNT \
	(BVDC_P_CMP_MAX_SLOT_COUNT * BVDC_P_MAX_MULTI_RUL_BUFFER_COUNT)

/* Get the current slot pointed by field. */
#define BVDC_P_CMP_GET_SLOT(hCompositor, polarity_id) \
	((hCompositor)->ahSlot[(polarity_id)])

#define BVDC_P_CMP_GET_DISP_TOP_TRIGGER(hCompositor) \
	(BVDC_P_DISP_GET_TOP_TRIGGER((hCompositor)->hDisplay))

#define BVDC_P_CMP_GET_DISP_BOT_TRIGGER(hCompositor) \
	(BVDC_P_DISP_GET_BOT_TRIGGER((hCompositor)->hDisplay))

/* Number of blender availables. */
#define BVDC_P_CMP_MAX_BLENDER        (BVDC_Z_ORDER_MAX + 1)

#define BVDC_P_CMP_GET_V0ID(hCompositor) \
    (((hCompositor)->eId >= BVDC_CompositorId_eCompositor2)? \
	 (BVDC_P_WindowId_eComp2_V0 + ((hCompositor)->eId - BVDC_CompositorId_eCompositor2)) :\
	 (BVDC_P_WindowId_eComp0_V0 + ((hCompositor)->eId - BVDC_CompositorId_eCompositor0) *2))

/* Compositor feature entry */
typedef struct
{
	uint32_t                          ulMaxVideoWindow;
	uint32_t                          ulMaxGfxWindow;
	uint32_t                          ulMaxWindow;

} BVDC_P_Compositor_Features;

/* Compositor dirty bits */
typedef union
{
	struct
	{
		uint32_t                          bColorClip : 1; /* new colorclip settings */
#if	BVDC_P_SUPPORT_OSCL
		uint32_t						  bOScl: 1; /* enable/disable OSCL */
#endif
	} stBits;

	uint32_t aulInts[BVDC_P_DIRTY_INT_ARRAY_SIZE];

} BVDC_P_Compositor_DirtyBits;

/* Compositor infos */
typedef struct
{
	uint32_t                          ulBgColorYCrCb;
	uint8_t                           ucRed;
	uint8_t                           ucGreen;
	uint8_t                           ucBlue;
	const BFMT_VideoInfo             *pFmtInfo;
	bool                              bLumaRectUserSet;
	BVDC_LumaSettings                 stLumaRect;
	BVDC_ColorClipSettings            stColorClipSettings;
	BFMT_Orientation                  eOrientation;

#if BVDC_P_SUPPORT_OSCL
	uint32_t                          bEnableOScl; /* enable OSCL for 1080i to 1080p convertion */
#endif

	/* dirty bits */
	BVDC_P_Compositor_DirtyBits       stDirty;
} BVDC_P_Compositor_Info;


/***************************************************************************
 * Compositor Context
 ***************************************************************************/
typedef struct BVDC_P_CompositorContext
{
	BDBG_OBJECT(BVDC_CMP)

	/* flag initial state, requires reset; */
	bool                              bIsBypass;
	bool                              bInitial;
	uint32_t                          ulCoreResetAddr;
	uint32_t                          ulCoreResetMask;

	/* public fields that expose thru API. */
	BVDC_P_Compositor_Info            stNewInfo;
	BVDC_P_Compositor_Info            stCurInfo;

	/* for display to infom window ApplyChange */
	bool                              bDspAspRatDirty;

	/* Set to true when new & old validated by apply changes  These
	 * flags get updated at applychanges. */
	bool                              bUserAppliedChanges;
	BVDC_Window_Handle                hSyncLockWin; /* window locked to this compositor */
	BVDC_Source_Handle                hSyncLockSrc; /* source locked to this compositor */
	BVDC_Source_Handle                hForceTrigPipSrc;
	BVDC_Source_Handle                hSrcToBeLocked;
	uint32_t                          ulSlip2Lock;
#if BVDC_P_SUPPORT_STG
	BVDC_Compositor_Handle            hCmpToLock; /* the sync-slaved compositor to grab the sync lock */
	bool                              bSyncSlave;
#endif

	/* RUL use for this compositor & display, (not created by compositor) */
	uint32_t                          aulRulIdx[BVDC_P_CMP_MAX_SLOT_COUNT];
	BRDC_Slot_Handle                  ahSlot[BVDC_P_CMP_MAX_SLOT_COUNT];
	BRDC_List_Handle                  ahList[BVDC_P_CMP_MAX_LIST_COUNT];
	BINT_CallbackHandle               ahCallback[BVDC_P_CMP_MAX_SLOT_COUNT];

	/* shadowed registers */
	BVDC_CompositorId                 eId;
	BVDC_P_State                      eState;
	uint32_t                          ulRegOffset; /* CMP_0, CMP_1, and etc. */
	uint32_t                          aulRegs[BVDC_P_CMP_REGS_COUNT];

	/* Compositor features. */
	const BVDC_P_Compositor_Features *pFeatures;

	/* Computed value */
	uint32_t                          ulActiveVideoWindow;
	uint32_t                          ulActiveGfxWindow;
	bool                              abBlenderUsed[BVDC_P_CMP_MAX_BLENDER];
	BVDC_P_WindowId                   aeBlenderWinId[BVDC_P_CMP_MAX_BLENDER];


	/* Compositor output to Display STG*/

	/* DW-1 MBOX Original PTS */
	uint32_t                          ulOrigPTS;
	int32_t                           uiHorizontalPanScan;   /* MPEG-2 Data format*/
	int32_t                           uiVerticalPanScan;     /* Same as above*/
	uint32_t                          ulDisplayHorizontalSize;
	uint32_t                          ulDisplayVerticalSize;
	uint32_t                          ulPicId;
	BAVC_USERDATA_PictureCoding       ePictureType;
	uint32_t                          ulChannelId;
	BAVC_Polarity                     eSourcePolarity;
	bool                              bPictureRepeatFlag;      /*picture repeat due to cadence detection or frame rate conversion*/
	bool                              bStgIgnorePicture;       /* actual ignore flag used for encoder */
	bool                              bCrcToIgnore, bCrcIgnored;/* delayed ignore flags(due to RUL delay and EOP property of CRC) used for CRC capture */
	bool                              bIgnorePicture;          /* from DM*/
	bool                              bStallStc;               /* from DM*/
	bool                              bLast;                   /* from DM*/
	bool                              bChannelChange;          /* from DM*/
	bool                              bGfxChannelChange;       /* GFX window indicator, maintained by vdc*/
	bool                              bMute;                   /* mute flag from DM */
	uint32_t                          ulStgPxlAspRatio_x_y;    /* PxlAspRatio_x<<16 | PxlAspRatio_y */
	uint32_t                          ulDecodePictureId;
	bool                              bValidAfd;
	uint32_t                          ulAfd;
	BAVC_BarDataType                  eBarDataType;
	uint32_t                          ulTopLeftBarValue;
	uint32_t                          ulBotRightBarValue;
	BFMT_Orientation                  eDspOrientation;
	BAVC_TransferCharacteristics      eTransferCharacteristics; /* transfer characteristics */
	/*  Fast non real time (FNRT) meta data support */
	bool                              bPreChargePicture;
	bool                              bEndofChunk;
	uint32_t                          ulChunkId;

	/* Ouptut to VEC. */
	BVDC_P_CmpColorSpace              eCmpColorSpace;
	BAVC_FrameRateCode                eSrcFRateCode;
	bool                              bFullRate;

	/* active windows (declare max).  Could also be dynamically allocated
	 * to BVDC_P_CMP_X_MAX_WINDOW_COUNT.  But this is pretty much fix. */
	BVDC_Window_Handle                ahWindow[BVDC_P_MAX_WINDOW_COUNT];
	uint32_t                          ulCscAdjust[BVDC_P_MAX_WINDOW_COUNT];
	bool                              bCscCompute[BVDC_P_MAX_WINDOW_COUNT];
	bool                              bCscDemoCompute[BVDC_P_MAX_WINDOW_COUNT];
	uint32_t                          ulColorKeyAdjust[BVDC_P_MAX_WINDOW_COUNT];
	uint32_t                          ulMosaicAdjust[BVDC_P_MAX_WINDOW_COUNT];
	uint32_t                          ulNLCscCtrl[2];    /* V0 and V1 */
	bool                              bSupportMACsc[2];  /* V0 and V1 */
	bool                              bSupportNLCsc[2];  /* V0 and V1 */
	/* this affects dvi dither setting */
	bool                              bIs10BitCore;


	/* Associated w/ this display handle. */
	BVDC_Display_Handle               hDisplay;

	 /* Created from this vdc */
	BVDC_Handle                       hVdc;
} BVDC_P_CompositorContext;


/***************************************************************************
 * Compositor private functions
 ***************************************************************************/
BERR_Code BVDC_P_Compositor_Create
	( BVDC_Handle                      hVdc,
	  BVDC_Compositor_Handle          *phCompositor,
	  BVDC_CompositorId                eCompositorId );

void BVDC_P_Compositor_Destroy
	( BVDC_Compositor_Handle           hCompositor );

void BVDC_P_Compositor_Init
	( BVDC_Compositor_Handle           hCompositor );

BERR_Code BVDC_P_Compositor_ValidateChanges
	( const BVDC_Compositor_Handle     ahCompositor[] );

void BVDC_P_Compositor_ApplyChanges_isr
	( BVDC_Compositor_Handle           hCompositor );

void BVDC_P_Compositor_AbortChanges
	( BVDC_Compositor_Handle           hCompositor );

void BVDC_P_Compositor_BuildSyncLockRul_isr
	( BVDC_Compositor_Handle           hCompositor,
	  BVDC_P_ListInfo                 *pList,
	  BAVC_Polarity                    eFieldId );

bool BVDC_P_Compositor_BuildSyncSlipRul_isr
	( BVDC_Compositor_Handle           hCompositor,
	  BVDC_P_ListInfo                 *pList,
	  BAVC_Polarity                    eFieldId,
	  bool                             bBuildCanvasCtrl );

void BVDC_P_Compositor_BuildConvasCtrlRul_isr
	( BVDC_Compositor_Handle           hCompositor,
	  BVDC_P_ListInfo                 *pList );

/* Miscellaneous access functions */
#if !BVDC_P_ORTHOGONAL_VEC
BERR_Code BVDC_P_Compositor_GetOutputInfo_isr
	( const BVDC_Compositor_Handle     hCompositor,
	  bool                            *pbFullRate );
#endif

BERR_Code BVDC_P_Compositor_AssignTrigger_isr
	( BVDC_Compositor_Handle           hCompositor,
	  BRDC_Trigger                     eTopTrigger,
	  BRDC_Trigger                     eBotTrigger );

void BVDC_P_Compositor_WindowsReader_isr
	( BVDC_Compositor_Handle           hCompositor,
	  BAVC_Polarity                    eNextFieldId,
	  BVDC_P_ListInfo                 *pList );

#if BDBG_DEBUG_BUILD
void BVDC_P_AssertEnumAndTables( void );
#endif

void BVDC_P_Compositor_GetCscTable_isrsafe
	( BVDC_P_CscCfg                   *pCscCfg,
	  bool                             bCscRgbMatching,
	  BAVC_MatrixCoefficients          eInputColorSpace,
	  BVDC_P_CmpColorSpace             eOutputColorSpace,
	  bool                             bInputXvYcc );

void BVDC_P_Compositor_GetCscToApplyAttenuationRGB_isr
	( const BVDC_P_CscCoeffs         **ppYCbCrToRGB,
	  const BVDC_P_CscCoeffs         **ppRGBToYCbCr,
	  BVDC_P_CmpColorSpace             eOutputColorSpace );

uint32_t BVDC_P_Compositor_GetCmpRegAddr_isr
	( BVDC_CompositorId                eId,
	  uint32_t                         ulRegAddr);

void BVDC_P_Compositor_SetMBoxMetaData_isr
	(
	const BVDC_P_PictureNode              *pPicture,
	BVDC_Compositor_Handle                hCompositor,
	BVDC_P_WindowId                       eId);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_COMPOSITOR_PRIV_H__ */
/* End of file. */

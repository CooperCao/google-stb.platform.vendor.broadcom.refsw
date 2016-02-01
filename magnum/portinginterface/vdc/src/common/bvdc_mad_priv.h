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
#ifndef BVDC_MAD_PRIV_H__
#define BVDC_MAD_PRIV_H__

#include "bvdc.h"
#include "bavc.h"
#include "bchp_common.h"
#include "bvdc_common_priv.h"
#include "bvdc_scaler_priv.h"
#include "bvdc_subrul_priv.h"
#include "bvdc_buffer_priv.h"
#include "bvdc_window_priv.h"

/***************************************************************************
 * Private defines
 ***************************************************************************/
/* TODO: See also bvdc_feeder_priv.h as example for updating comment */
#define BVDC_P_MAD_VER_0                            (0)
#define BVDC_P_MAD_VER_1                            (1)
#define BVDC_P_MAD_VER_2                            (2)
#define BVDC_P_MAD_VER_3                            (3)
#define BVDC_P_MAD_VER_4                            (4)
#define BVDC_P_MAD_VER_5                            (5)
#define BVDC_P_MAD_VER_6                            (6)
/* 3548 B0:
 * Removed MAD_0_SCHG_MOTION_THRESHOLD
 * Removed MAD_0_DITHER_*
 * TODO: added many */
#define BVDC_P_MAD_VER_7                            (7)

/* 7125C0 added PIXEL_FEED_x_ENABLE */
#define BVDC_P_MAD_VER_8                            (8)


#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * Private register cracking macros
 ***************************************************************************/
#if (0!=BVDC_P_SUPPORT_MAD_VER) && (0<BVDC_P_SUPPORT_MAD)

#define BVDC_P_MAD_GET_REG_IDX(reg) \
	((BCHP##_##reg - BCHP_MAD_0_REG_START) / sizeof(uint32_t))

/* Get/Set reg data */
#define BVDC_P_MAD_GET_REG_DATA(reg) \
	(hMad->aulRegs[BVDC_P_MAD_GET_REG_IDX(reg)])
#define BVDC_P_MAD_SET_REG_DATA(reg, data) \
	(BVDC_P_MAD_GET_REG_DATA(reg) = (uint32_t)(data))

/* Get with index. */
#define BVDC_P_MAD_GET_REG_DATA_I(reg, idx) \
	(hMad->aulRegs[BVDC_P_MAD_GET_REG_IDX(reg) + (idx)])

/* Get field */
#define BVDC_P_MAD_GET_FIELD_NAME(reg, field) \
	(BVDC_P_GET_FIELD(BVDC_P_MAD_GET_REG_DATA(reg), reg, field))

/* Compare field */
#define BVDC_P_MAD_COMPARE_FIELD_DATA(reg, field, data) \
	(BVDC_P_COMPARE_FIELD_DATA(BVDC_P_MAD_GET_REG_DATA(reg), reg, field, (data)))
#define BVDC_P_MAD_COMPARE_FIELD_NAME(reg, field, name) \
	(BVDC_P_COMPARE_FIELD_NAME(BVDC_P_MAD_GET_REG_DATA(reg), reg, field, name))

/* This macro does a write into RUL (write, addr, data). 3 dwords. */
#define BVDC_P_MAD_WRITE_TO_RUL(reg, addr_ptr) \
{ \
	*addr_ptr++ = BRDC_OP_IMM_TO_REG(); \
	*addr_ptr++ = BRDC_REGISTER(BCHP##_##reg + hMad->ulRegOffset); \
	*addr_ptr++ = BVDC_P_MAD_GET_REG_DATA(reg); \
}

/* This macro does a block write into RUL */
#define BVDC_P_MAD_BLOCK_WRITE_TO_RUL(from, to, pulCurrent) \
do { \
	uint32_t ulBlockSize = \
		BVDC_P_REGS_ENTRIES(from, to);\
	*pulCurrent++ = BRDC_OP_IMMS_TO_REGS( ulBlockSize ); \
	*pulCurrent++ = BRDC_REGISTER(BCHP##_##from + hMad->ulRegOffset); \
	BKNI_Memcpy((void*)pulCurrent, \
		(void*)&(hMad->aulRegs[BVDC_P_MAD_GET_REG_IDX(from)]), \
		ulBlockSize * sizeof(uint32_t)); \
	pulCurrent += ulBlockSize; \
} while(0)

/* number of registers in one block. */
#define BVDC_P_MAD_REGS_COUNT    \
	BVDC_P_REGS_ENTRIES(MAD_0_REG_START, MAD_0_REG_END)

/* get register offset base on id. */
#define BVDC_P_MAD_GET_REG_OFFSET(mad_id)     (0)

/* Set horizontal scale down ratio */
#define BVDC_P_MAD_SET_HORZ_RATIO(ratio) \
{ \
	BVDC_P_MAD_GET_REG_DATA(MAD_0_HORIZ_STEP) &= ~( \
		BCHP_MASK(MAD_0_HORIZ_STEP, SIZE)); \
	BVDC_P_MAD_GET_REG_DATA(MAD_0_HORIZ_STEP) |=  ( \
		BCHP_FIELD_DATA(MAD_0_HORIZ_STEP, SIZE, ratio)); \
}

/* fixed point stuffs */
#define BVDC_P_MAD_H_PAN_SCAN_I_BITS          (1)
#define BVDC_P_MAD_H_PAN_SCAN_F_BITS          (6)

#define BVDC_P_MAD_H_RATIO_I_BITS             (4)
#define BVDC_P_MAD_H_RATIO_F_BITS             (17)

#define BVDC_P_MAD_LUMA_INIT_PHASE_I_BITS     (11)
#define BVDC_P_MAD_LUMA_INIT_PHASE_F_BITS     (6)

#define BVDC_P_MAD_CHROMA_INIT_PHASE_I_BITS   (11)
#define BVDC_P_MAD_CHROMA_INIT_PHASE_F_BITS   (6)

#define BVDC_P_MAD_COEFFS_I_BITS              (1)
#define BVDC_P_MAD_COEFFS_F_BITS              (10)

#define BVDC_P_MAD_LARGEST_F_BITS             (17)
#define BVDC_P_MAD_ZERO_F_BITS                (0)

/* to normalize everything into S14.17 fixed format */
#define BVDC_P_MAD_NORMALIZE(value, f_bit) ((value) << (BVDC_P_MAD_LARGEST_F_BITS - (f_bit)))

/* to innormalize everything from S14.17 fixed format */
#define BVDC_P_MAD_NORM_2_SPEC(value, f_bit) ((value) >> (BVDC_P_MAD_LARGEST_F_BITS - (f_bit)))

/* Miscellaneous constants */
#define BVDC_P_MAD_HORZ_REGIONS_COUNT         (1)

#define BVDC_P_MAD_HORZ_FIR_TAP_COUNT         (8)
#define BVDC_P_MAD_HORZ_FIR_PHASE_COUNT       (8)

#define BVDC_P_MAD_FIR_TAP_COUNT_MAX          (8)
#define BVDC_P_MAD_FIR_PHASE_COUNT_MAX        (8)

#define BVDC_P_MAD_4TAP_HORZ_THRESHOLD_0      (1280)
#define BVDC_P_MAD_4TAP_HORZ_THRESHOLD_1      (1024)

#define BVDC_P_MAD_HORZ_HWF_FACTOR            (2)

/* Make Horizontal ratio */
#define BVDC_P_MAD_MAKE_H_RATIO(src, dst) \
	(BVDC_P_MAD_NORM_2_SPEC((src), BVDC_P_MAD_H_RATIO_F_BITS) / (dst))

#define BVDC_P_MAD_HORZ_1_FIXED BVDC_P_FLOAT_TO_FIXED(1.000, \
	BVDC_P_MAD_LUMA_INIT_PHASE_I_BITS, BVDC_P_MAD_LUMA_INIT_PHASE_F_BITS)

#define BVDC_P_MAD_FIR_COEFFS_MAX \
	(BVDC_P_MAD_FIR_TAP_COUNT_MAX * BVDC_P_MAD_FIR_PHASE_COUNT_MAX)

#define BVDC_P_MAD_LAST UINT32_C(-1)

/* PR26785:  vsyncs to wait before starting 3field, 4field, or 5field game mode */
#define BVDC_P_MAD_GAME_MODE_START_DELAY          (6)

/* PR36539: vsyncs to waiting hardstart finish, before set Mad freeze buffer */
#define BVDC_P_MAD_TRICK_MODE_START_DELAY         (3)

#else  /* #if (0!=BVDC_P_SUPPORT_MAD_VER) */
#define BVDC_P_MAD_REGS_COUNT               1
#endif

/***************************************************************************
 * Software 2:2 reverse pulldown
 ***************************************************************************/
/*
   This value is the minimum PCC count required to use the PCC algorithm.
   If the PCC is below this, the hardware 2:2 algorithm is used
   (as implemented in software).
*/
#define BVDC_P_MAD_MIN_USABLE_PCC           2100 /* PR38878 */

/*
   This value is the multiplier used by the pixel weave check.
*/
#define BVDC_P_MAD_PW_MATCH_MULTIPLIER      0x10

/*
   Indicates how strong the non-weave PCC counter has to be in relation to
   the weave PCC counter. A larger value means that the algorithm would
   be more selective in declaring 2:2 lock.
*/
#define BVDC_P_MAD_PCC_NONMATCH_MATCH_RATIO 8

/*
   Multiplier to apply to histogram[4] (measure of motion) used in conjunction
   with HISTOGRAM4_OFFSET.
*/
#define BVDC_P_MAD_HISTOGRAM4_RATIO_NTSC         8
#define BVDC_P_MAD_HISTOGRAM4_RATIO_PAL          10

/*
   Allowable PCC count in the weave direction assuming that histogram[4]
   (measure of motion) is zero.  A larger value means that the algorithm
   would be less likely to lose lock in low motion.
*/
#define BVDC_P_MAD_HISTOGRAM4_OFFSET_NTSC        (0x2c0)
#define BVDC_P_MAD_HISTOGRAM4_OFFSET_PAL         (0x300)
#define BVDC_P_MAD_HISTOGRAM4_OFFSET_PAL_OVERSAMPLE         (0x900)

/*
   Amount to decrease the phase counter if the motion-adjusted
   (histogram[4]) PCC weave is too high. Recommendation is to make
   the value at least equal to REV22_LOCK_SAT_LEVEL - REV22_EXIT_LOCK_LEVEL
   so that the check can take the chip out of lock immediately.
*/
#define BVDC_P_MAD_HISTOCHECK_DEC           15

/*
   Tolerable delta between PCC counters in the weave and nonweave direction
   before we decrement the phase counter.
*/
#define BVDC_P_MAD_WEAVE_22_THRESHOLD       150
#define BVDC_P_MAD_WEAVE_22_THRESHOLD_OVERSAMPLE       0

/*
  (Emulates RDB register function when PCC counters do not
  exceed MIN_USABLE_PCC).
*/
#define BVDC_P_MAD_UPPER_MATCH_THRESH       (625 << 5)
#define BVDC_P_MAD_LOWER_NONMATCH_THRESH    (468 << 5)
#define BVDC_P_MAD_NONMATCH_MATCH_RATIO     8
#define BVDC_P_MAD_REV22_LOCK_SAT_LEVEL     32
#define BVDC_P_MAD_REV22_ENTER_LOCK_LEVEL   25
#define BVDC_P_MAD_REV22_EXIT_LOCK_LEVEL    20

/*
   If both PCC counts are below this value, we're not getting enough
   information for the PCC method to be useful.
*/
#define BVDC_P_MAD_REV22_DONTCARE           150

/*
   Bad weave threshold for sudden increases in PCC value.
*/
#define BVDC_P_MAD_MAX_PCC_CHANGE           7400

/*
   If PCC in the weave direction is higher than this threshold,
   algorithm will perform a check on the PCC against the repf_motion.
*/
#define BVDC_P_MAD_RM_CHECK_THRESH_NTSC     2000
#define BVDC_P_MAD_RM_CHECK_THRESH_PAL      2750

/*
   Multiplier used in repf_motion check.
*/
#define BVDC_P_MAD_RM_CHECK_RATIO_NTSC      29
#define BVDC_P_MAD_RM_CHECK_RATIO_PAL       28

#define BVDC_P_Mad_MuxAddr(hMad)        (BCHP_VNET_F_MAD_0_SRC + (hMad)->eId * sizeof(uint32_t))
#if (BVDC_P_SUPPORT_MAD > 1)
#define BVDC_P_Mad_PostMuxValue(hMad)   \
   ((0 == (hMad)->eId)? BCHP_VNET_B_CAP_0_SRC_SOURCE_MAD_0 : BCHP_VNET_B_CAP_0_SRC_SOURCE_MAD_1)
#else
#define BVDC_P_Mad_PostMuxValue(hMad)   (BCHP_VNET_B_CAP_0_SRC_SOURCE_MAD_0)
#endif


/***************************************************************************
 * Mad private data structures
 ***************************************************************************/
typedef struct BVDC_P_MadRev22Statistics
{
	uint32_t   ulMatchWeave;
	uint32_t   ulNonMatchWeave;
	uint32_t   ulMatchUM;
	uint32_t   ulNonMatchUM;
	uint32_t   ulAvgWeave;
	uint32_t   ulPixelWeave;
	uint32_t   ulRepfMotion;
} BVDC_P_MadRev22Statistics;

typedef struct BVDC_P_MadRev32Statistics
{
	uint32_t           ulBwvCtrl5;
	uint32_t           ulPhaseCalc0;
	uint32_t           ulPhaseCalc1;
	uint32_t           ulPhaseCalc2;
	uint32_t           ulPhaseCalc8;
	uint32_t           ulPccLumaPcc;
	uint32_t           ulPccChromaPcc;
	uint32_t           ulPrevLumaPcc;
	uint32_t           ulPrevChromaPcc;

	uint32_t           ulWndBias;

	bool               abRev32Locked[5];
	uint32_t           aulSigma[5];
	uint32_t           aulX[5];
	uint32_t           aulP[5];
	uint32_t           aulV[5];
} BVDC_P_MadRev32Statistics;

typedef struct BVDC_P_MadGameModeInfo
{
	BVDC_MadGameMode               eMode;
	uint16_t                       usDelay;
	uint16_t                       usPixelBufferCnt;
	const char                    *pchModeName;

} BVDC_P_MadGameModeInfo;


typedef struct BVDC_P_MadContext
{
	BDBG_OBJECT(BVDC_MAD)

	BREG_Handle                    hRegister;
	BVDC_Heap_Handle               hHeap;
	BVDC_Window_Handle             hWindow;

	BVDC_P_Compression_Settings    *pstCompression;

	/* flag initial state, require reset; */
	bool                           bInitial;
	uint32_t                       ulResetRegAddr;
	uint32_t                       ulResetMask;

	/* OSD feature for MAD */
	bool                           bEnableOsd;
	uint32_t                       ulOsdHpos;
	uint32_t                       ulOsdVpos;

	/* flag for changes */
	uint32_t                       ulUpdateAll;
	uint32_t                       ulUpdateMisc;
	uint32_t                       ulUpdatePullDown;
	uint32_t                       ulUpdate22PullDown;
	uint32_t                       ulUpdateChromaSettings;
	uint32_t                       ulUpdateMotionSettings;

	/* parameters to used in software 2:2 pulldown algorithm */
	bool                           bReverse22Pulldown;
	bool                           bReverse32Pulldown;

	int32_t                        ulRev22NonMatchMatchRatio;
	int32_t                        ulRev22EnterLockLevel;
	int32_t                        ulRev22LocksatLevel;
	int32_t                        ulRev22BwvLumaThreshold;
	uint32_t                       ulRev22Candidate;
	uint32_t                       ulPrevCtIndexLuma;
	uint32_t                       ulPrevCtIndexChroma;
	BVDC_P_CtInput                 ePrevCtInputType;

	/* Optimized */
	int32_t                        lMadCutLeft; /* S11.6, same fmt as SclCut->lLeft */

	/* for 3:2 pulldown alg */
#if (BVDC_P_SUPPORT_MAD_VER >= 5)
	BVDC_P_MadRev32Statistics      stRev32Statistics;
#endif

	/* for both 3:2 and 2:2 pulldown alg */
	uint32_t                       ulPhaseCounter;
	uint32_t                       ulPhaseCounter22;
	int32_t                        alPhase[5];
	int32_t                        alPhase22[2];

	/* parameters to used in optimize algorithm */
	uint32_t                       ulRepfMotion;
	uint32_t                       aulSigma;
	uint32_t                       ulPreFieldMotion;
	uint32_t                       ulStillFieldNum;
	uint32_t                       ulChangeFieldNum;
	bool                           b5FieldMode;
	bool                           bOptimizeStill;

	/* Fir coeff tables */
	const BVDC_P_FirCoeffTbl      *pHorzFirCoeffTbl;
	const BVDC_P_FirCoeffTbl      *pChromaHorzFirCoeffTbl;

	/* private fields. */
	BVDC_P_MadId                   eId;
	uint32_t                       ulRegOffset; /* MAD_0, MAD_1, and etc. */
	uint32_t                       ulMaxWidth; /* max width limited by line buf size */
	uint32_t                       ulMaxHeight; /* max height limited by RTS */
	uint32_t                       ulHsclSizeThreshold; /* hsize that triggers use of HSCL before deinterlacing */
	uint32_t                       aulRegs[BVDC_P_MAD_REGS_COUNT];

	/* Pixel Field Memory Store */
	BVDC_P_HeapNodePtr             apPixelHeapNode[BVDC_P_MAD_PIXEL_SD_BUFFER_COUNT];
	/* Quantized Motion Field Memory Store */
	BVDC_P_HeapNodePtr             apQmHeapNode[BVDC_P_MAD_QM_BUFFER_COUNT];

	/* values for MAD game mode */
	uint16_t                       usFeedCapture;
	uint16_t                       usCurQm;
	uint16_t                       usGameModeStartDelay;
	uint16_t                       usGameModeQmDelay;
	uint16_t                       usPixelBufferCnt;
	bool                           bBufferCntChanged;

	/* mad user setting */
	BVDC_MadGameMode               eGameMode;
	bool                           bRev32Custom;
	bool                           bRev22Custom;
	bool                           bChromaCustom;
	bool                           bMotionCustom;

	/* flag user changes to optimize MAD code */
	bool                           bUsrChanges;

	/* sub-struct to manage vnet and rul build opreations */
	BVDC_P_SubRulContext           SubRul;

	/* last field saved PCC_FWD */
	uint32_t                       ulPccFwd;
	uint32_t                       ulPrePccFwd;

	/* Dither */
	BVDC_P_DitherSetting           stDither;

	/* delay for MAD freeze transition */
	uint16_t                       usTrickModeStartDelay;

#if (BVDC_P_SUPPORT_MAD_VER >= BVDC_P_MAD_VER_7)
	/* mem saving mode between MAD/ANR */
	bool                            bMemSaving;
#endif

#if (BVDC_P_SUPPORT_HSCL_MAD_HARD_WIRED)
	BVDC_P_Hscaler_Handle           hHscaler;
#endif

	uint32_t                        ulHardStartCountdown;

	uint32_t                        ulGameModeTransCnt;
} BVDC_P_MadContext;


/***************************************************************************
 * Mad private functions
 ***************************************************************************/
BERR_Code BVDC_P_Mad_Create
	( BVDC_P_Mad_Handle            *phMad,
	  BREG_Handle                   hRegister,
	  BVDC_P_MadId                  eMadId,
	  BVDC_P_Resource_Handle        hResource );

void BVDC_P_Mad_Destroy
	( BVDC_P_Mad_Handle             hMad );

BERR_Code BVDC_P_Mad_AcquireConnect_isr
	( BVDC_P_Mad_Handle             hMad,
	  BVDC_Heap_Handle              hHeap,
	  BVDC_Window_Handle            hWindow);

BERR_Code BVDC_P_Mad_ReleaseConnect_isr
	( BVDC_P_Mad_Handle            *phMad );

void BVDC_P_Mad_SetVnetAllocBuf_isr
	( BVDC_P_Mad_Handle             hMad,
	  uint32_t                      ulSrcMuxValue,
	  BVDC_P_VnetPatch              eVnetPatchMode,
	  BVDC_P_BufferHeapId           eMadPixelBufHeapId,
	  BVDC_P_BufferHeapId           eMadQmBufHeapId,
	  uint16_t                      usMadPixelSdBufferCnt);

void BVDC_P_Mad_UnsetVnetFreeBuf_isr
	( BVDC_P_Mad_Handle             hMad );

void BVDC_P_Mad_BuildRul_isr
	( const BVDC_P_Mad_Handle       hMad,
	  BVDC_P_ListInfo              *pList,
	  BVDC_P_State                  eVnetState,
	  BVDC_P_PicComRulInfo         *pPicComRulInfo );

BERR_Code BVDC_P_Mad_SetInfo_isr
	( BVDC_P_Mad_Handle             hMad,
	  const BVDC_Window_Handle      hWindow,
	  const BVDC_P_PictureNodePtr   pPicture );

BERR_Code BVDC_P_Mad_SetEnable_isr
	( BVDC_P_Mad_Handle             hMad,
	  bool                          bEnable );

/***************************************************************************
 * Return the user set of mad chroma
 */
void BVDC_P_Mad_GetUserChroma_isr
	( BVDC_P_Mad_Handle                hMad,
	  BVDC_Deinterlace_ChromaSettings *pstChromaSettings );

/***************************************************************************
 * Return the user set of mad motino
 */
void BVDC_P_Mad_GetUserMotion_isr
	( BVDC_P_Mad_Handle                hMad,
	  BVDC_Deinterlace_MotionSettings *pstMotionSettings );

/***************************************************************************
 * Return the user set of mad 3:2 pulldown
 */
void BVDC_P_Mad_GetUserReverse32_isr
	( BVDC_P_Mad_Handle                   hMad,
	  BVDC_Deinterlace_Reverse32Settings *pstRev32Settings );

/***************************************************************************
 * Return the user set of mad chroma
 */
void BVDC_P_Mad_GetUserReverse22_isr
	( BVDC_P_Mad_Handle                   hMad,
	  BVDC_Deinterlace_Reverse22Settings *pstRev22Settings );

void BVDC_P_Mad_Init_Default
	( BVDC_MadGameMode                      *peGameMode,
	  BPXL_Format                           *pePxlFormat,
	  BVDC_Mode                             *pePqEnhancement,
	  bool                                  *pbShrinkWidth,
	  bool                                  *pbReverse32Pulldown,
	  BVDC_Deinterlace_Reverse32Settings    *pReverse32Settings,
	  bool                                  *pbReverse22Pulldown,
	  BVDC_Deinterlace_Reverse22Settings    *pReverse22Settings,
	  BVDC_Deinterlace_ChromaSettings       *pChromaSettings,
	  BVDC_Deinterlace_MotionSettings       *pMotionSettings );

void BVDC_P_Mad_Init_Custom
	( BVDC_422To444UpSampler          *pUpSampler,
	  BVDC_444To422DnSampler          *pDnSampler,
	  BVDC_Deinterlace_LowAngleSettings *pLowAngles );

BERR_Code BVDC_P_Mad_Init_DynamicDefault
	( BVDC_Window_Handle                  hWindow,
	  BVDC_Deinterlace_Reverse32Settings *pReverse32Settings,
	  BVDC_Deinterlace_Reverse22Settings *pReverse22Settings,
	  BVDC_Deinterlace_ChromaSettings    *pChromaSettings );

BERR_Code BVDC_P_Mad_Init_DynamicDefault_isr
	( BVDC_Window_Handle                  hWindow,
	  BVDC_Deinterlace_Reverse32Settings *pReverse32Settings,
	  BVDC_Deinterlace_Reverse22Settings *pReverse22Settings,
	  BVDC_Deinterlace_ChromaSettings    *pChromaSettings );

uint16_t BVDC_P_Mad_GetVsyncDelayNum_isr
	( BVDC_MadGameMode                 eGameMode );

uint16_t BVDC_P_Mad_GetPixBufCnt_isr
	(  BVDC_MadGameMode                eGameMode );

bool BVDC_P_Mad_BeHardStart_isr
	( BVDC_P_Mad_Handle                hMad );

void BVDC_P_Mad_ReadOutPhase_isr
	( BVDC_P_Mad_Handle                  hMad,
	  BVDC_P_PictureNode                *pPicture);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_MAD_PRIV_H__ */
/* End of file. */

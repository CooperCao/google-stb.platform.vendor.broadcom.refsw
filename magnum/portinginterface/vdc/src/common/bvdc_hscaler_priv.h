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
#ifndef BVDC_HSCALER_PRIV_H__
#define BVDC_HSCALER_PRIV_H__

#include "bvdc.h"
#include "bchp_common.h"
#include "bvdc_common_priv.h"
#include "bvdc_subrul_priv.h"
#include "bvdc_buffer_priv.h"
#include "bvdc_scaler_priv.h"

#if (BVDC_P_SUPPORT_HSCL_MAD_HARD_WIRED)
#include "bchp_hscl_0.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Core revs to track increatmental changes! */
#define BVDC_P_SUPPORT_HSCL_VER_1             (1) /* 3548 Ax */
#define BVDC_P_SUPPORT_HSCL_VER_2             (2) /* 7420 */
#define BVDC_P_SUPPORT_HSCL_VER_3             (3) /* 3548 Bx */
#define BVDC_P_SUPPORT_HSCL_VER_4             (4) /* 7340, 7550 */
#define BVDC_P_SUPPORT_HSCL_VER_5             (5) /* 7422, 7425 */
#define BVDC_P_SUPPORT_HSCL_VER_6             (6) /* 7231B0, 7344B0, 7346B0, 7425B0, Added more derring knobs */
#define BVDC_P_SUPPORT_HSCL_VER_7             (7) /* 7364A0, remove HSCL_0_TOP_CONTROL UPDATE_SEL Field */


/***************************************************************************
 * Private register cracking macros
 ***************************************************************************/
#if (BVDC_P_SUPPORT_HSCL_MAD_HARD_WIRED)

#ifdef BCHP_HSCL_5_REG_START
#define BVDC_P_HSCL_GET_REG_OFFSET(eHsclId) \
	((BVDC_P_HscalerId_eHscl5==(eHsclId)) ? (BCHP_HSCL_5_REG_START - BCHP_HSCL_0_REG_START) \
	:(BVDC_P_HscalerId_eHscl4==(eHsclId)) ? (BCHP_HSCL_4_REG_START - BCHP_HSCL_0_REG_START) \
	:(BVDC_P_HscalerId_eHscl3==(eHsclId)) ? (BCHP_HSCL_3_REG_START - BCHP_HSCL_0_REG_START) \
	:(BVDC_P_HscalerId_eHscl2==(eHsclId)) ? (BCHP_HSCL_2_REG_START - BCHP_HSCL_0_REG_START) \
	:(BVDC_P_HscalerId_eHscl1==(eHsclId)) ? (BCHP_HSCL_1_REG_START - BCHP_HSCL_0_REG_START) \
	:(0))
#else
#ifdef BCHP_HSCL_4_REG_START
#define BVDC_P_HSCL_GET_REG_OFFSET(eHsclId) \
	((BVDC_P_HscalerId_eHscl4==(eHsclId)) ? (BCHP_HSCL_4_REG_START - BCHP_HSCL_0_REG_START) \
	:(BVDC_P_HscalerId_eHscl3==(eHsclId)) ? (BCHP_HSCL_3_REG_START - BCHP_HSCL_0_REG_START) \
	:(BVDC_P_HscalerId_eHscl2==(eHsclId)) ? (BCHP_HSCL_2_REG_START - BCHP_HSCL_0_REG_START) \
	:(BVDC_P_HscalerId_eHscl1==(eHsclId)) ? (BCHP_HSCL_1_REG_START - BCHP_HSCL_0_REG_START) \
	:(0))
#else
#ifdef BCHP_HSCL_3_REG_START
#define BVDC_P_HSCL_GET_REG_OFFSET(eHsclId) \
	((BVDC_P_HscalerId_eHscl3==(eHsclId)) ? (BCHP_HSCL_3_REG_START - BCHP_HSCL_0_REG_START) \
	:(BVDC_P_HscalerId_eHscl2==(eHsclId)) ? (BCHP_HSCL_2_REG_START - BCHP_HSCL_0_REG_START) \
	:(BVDC_P_HscalerId_eHscl1==(eHsclId)) ? (BCHP_HSCL_1_REG_START - BCHP_HSCL_0_REG_START) \
	:(0))
#else
#ifdef BCHP_HSCL_2_REG_START
#define BVDC_P_HSCL_GET_REG_OFFSET(eHsclId) \
	((BVDC_P_HscalerId_eHscl2==(eHsclId)) ? (BCHP_HSCL_2_REG_START - BCHP_HSCL_0_REG_START) \
	:(BVDC_P_HscalerId_eHscl1==(eHsclId)) ? (BCHP_HSCL_1_REG_START - BCHP_HSCL_0_REG_START) \
	:(0))
#else
#ifdef BCHP_HSCL_1_REG_START
#define BVDC_P_HSCL_GET_REG_OFFSET(eHsclId) \
	((BVDC_P_HscalerId_eHscl1==(eHsclId)) ? (BCHP_HSCL_1_REG_START - BCHP_HSCL_0_REG_START) \
	:(0))
#else
#define BVDC_P_HSCL_GET_REG_OFFSET(eHsclId)           (0)
#endif /* Hscl_1 */
#endif /* Hscl_2 */
#endif /* Hscl_3 */
#endif /* Hscl_4 */
#endif /* Hscl_5 */
#define BVDC_P_HSCL_GET_REG_IDX(reg) \
	((BCHP##_##reg - BCHP_HSCL_0_REG_START) / sizeof(uint32_t))

/* Get/Set reg data */
#define BVDC_P_HSCL_GET_REG_DATA(reg) \
	(hHscaler->aulRegs[BVDC_P_HSCL_GET_REG_IDX(reg)])
#define BVDC_P_HSCL_SET_REG_DATA(reg, data) \
	(BVDC_P_HSCL_GET_REG_DATA(reg) = (uint32_t)(data))

/* Get with index. */
#define BVDC_P_HSCL_GET_REG_DATA_I(idx, reg) \
	(hHscaler->aulRegs[BVDC_P_HSCL_GET_REG_IDX(reg) + (idx)])

/* Get field */
#define BVDC_P_HSCL_GET_FIELD_NAME(reg, field) \
	(BVDC_P_GET_FIELD(BVDC_P_HSCL_GET_REG_DATA(reg), reg, field))

/* Compare field */
#define BVDC_P_HSCL_COMPARE_FIELD_DATA(reg, field, data) \
	(BVDC_P_COMPARE_FIELD_DATA(BVDC_P_HSCL_GET_REG_DATA(reg), reg, field, (data)))
#define BVDC_P_HSCL_COMPARE_FIELD_NAME(reg, field, name) \
	(BVDC_P_COMPARE_FIELD_NAME(BVDC_P_HSCL_GET_REG_DATA(reg), reg, field, name))

/* This macro does a write into RUL (write, addr, data). 3 dwords. */
#define BVDC_P_HSCL_WRITE_TO_RUL(reg, addr_ptr) \
{ \
	*addr_ptr++ = BRDC_OP_IMM_TO_REG(); \
	*addr_ptr++ = BRDC_REGISTER(BCHP##_##reg + hHscaler->ulRegOffset); \
	*addr_ptr++ = BVDC_P_HSCL_GET_REG_DATA(reg); \
}

/* This macro does a block write into RUL */
#define BVDC_P_HSCL_BLOCK_WRITE_TO_RUL(from, to, pulCurrent) \
do { \
	uint32_t ulBlockSize = \
		BVDC_P_REGS_ENTRIES(from, to);\
	*pulCurrent++ = BRDC_OP_IMMS_TO_REGS( ulBlockSize ); \
	*pulCurrent++ = BRDC_REGISTER(BCHP##_##from + hHscaler->ulRegOffset); \
	BKNI_Memcpy((void*)pulCurrent, \
		(void*)&(hHscaler->aulRegs[BVDC_P_HSCL_GET_REG_IDX(from)]), \
		ulBlockSize * sizeof(uint32_t)); \
	pulCurrent += ulBlockSize; \
} while(0)

/* This macro does a scaler reset RUL. */
#define BVDC_P_HSCL_RESET_RUL(hHscaler) \
{ \
	*pulCurrent++ = BRDC_OP_IMM_TO_REG(); \
	*pulCurrent++ = BRDC_REGISTER(hHscaler->ulResetRegAddr); \
	*pulCurrent++ = hHscaler->ulResetMask; \
	*pulCurrent++ = BRDC_OP_IMM_TO_REG(); \
	*pulCurrent++ = BRDC_REGISTER(hHscaler->ulResetRegAddr); \
	*pulCurrent++ = 0; \
}

/* number of registers in one block. */
#define BVDC_P_HSCL_REGS_COUNT      \
	BVDC_P_REGS_ENTRIES(HSCL_0_REG_START, HSCL_0_REG_END)

/* set scaling regions */
#define BVDC_P_HSCL_SET_HORZ_REGION02(region, end_pixel, step_inc) \
{ \
	BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_DEST_PIC_REGION_##region##_STEP_DELTA) &= ~( \
		BCHP_MASK(HSCL_0_HORIZ_FIR_DEST_PIC_REGION_0_STEP_DELTA, SIZE)); \
	BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_DEST_PIC_REGION_##region##_STEP_DELTA) |=  ( \
		BCHP_FIELD_DATA(HSCL_0_HORIZ_FIR_DEST_PIC_REGION_0_STEP_DELTA, SIZE, step_inc)); \
	BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_DEST_PIC_REGION_##region##_END) &= ~( \
		BCHP_MASK(HSCL_0_HORIZ_DEST_PIC_REGION_0_END, POSITION)); \
	BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_DEST_PIC_REGION_##region##_END) |=  ( \
		BCHP_FIELD_DATA(HSCL_0_HORIZ_DEST_PIC_REGION_0_END, POSITION, end_pixel)); \
}

/* Set horizontal scale down ratio */
#define BVDC_P_HSCL_HORZ_SCALE_REGION          (1)
#if (BVDC_P_SUPPORT_HSCL_VER < BVDC_P_SUPPORT_HSCL_VER_2)
#define BVDC_P_HSCL_SET_HORZ_RATIO(ratio) \
{ \
	BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_INIT_STEP) &= ~( \
		BCHP_MASK(HSCL_0_HORIZ_FIR_INIT_STEP, SIZE)); \
	BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_INIT_STEP) |=  ( \
		BCHP_FIELD_DATA(HSCL_0_HORIZ_FIR_INIT_STEP, SIZE, ratio)); \
}
#else
#define BVDC_P_HSCL_SET_HORZ_RATIO(ratio) \
{ \
	BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_INIT_STEP_FRAC) &= ~( \
		BCHP_MASK(HSCL_0_HORIZ_FIR_INIT_STEP_FRAC, FRAC_SIZE)); \
	BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_INIT_STEP_FRAC) |=  ( \
		BCHP_FIELD_DATA(HSCL_0_HORIZ_FIR_INIT_STEP_FRAC, FRAC_SIZE, \
		                ratio & ~(1 << BVDC_P_SCL_H_RATIO_F_BITS))); \
	BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_INIT_STEP_INT) &= ~( \
		BCHP_MASK(HSCL_0_HORIZ_FIR_INIT_STEP_INT, INT_SIZE)); \
	BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_INIT_STEP_INT) |=  ( \
		BCHP_FIELD_DATA(HSCL_0_HORIZ_FIR_INIT_STEP_INT, INT_SIZE, \
		                ratio >> BVDC_P_SCL_H_RATIO_F_BITS)); \
}
#endif

#define BVDC_P_HSCL_SET_HORZ_REGION02(region, end_pixel, step_inc) \
{ \
	BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_DEST_PIC_REGION_##region##_STEP_DELTA) &= ~( \
		BCHP_MASK(HSCL_0_HORIZ_FIR_DEST_PIC_REGION_0_STEP_DELTA, SIZE)); \
	BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_FIR_DEST_PIC_REGION_##region##_STEP_DELTA) |=  ( \
		BCHP_FIELD_DATA(HSCL_0_HORIZ_FIR_DEST_PIC_REGION_0_STEP_DELTA, SIZE, step_inc)); \
	BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_DEST_PIC_REGION_##region##_END) &= ~( \
		BCHP_MASK(HSCL_0_HORIZ_DEST_PIC_REGION_0_END, POSITION)); \
	BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_DEST_PIC_REGION_##region##_END) |=  ( \
		BCHP_FIELD_DATA(HSCL_0_HORIZ_DEST_PIC_REGION_0_END, POSITION, end_pixel)); \
}

#define BVDC_P_HSCL_SET_HORZ_STEP_MISC(end_pixel_1, step_init) \
{ \
	BVDC_P_HSCL_SET_HORZ_RATIO(step_init); \
	BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_DEST_PIC_REGION_1_END) &= ~( \
		BCHP_MASK(HSCL_0_HORIZ_DEST_PIC_REGION_1_END, POSITION)); \
	BVDC_P_HSCL_GET_REG_DATA(HSCL_0_HORIZ_DEST_PIC_REGION_1_END) |=  ( \
		BCHP_FIELD_DATA(HSCL_0_HORIZ_DEST_PIC_REGION_1_END, POSITION, end_pixel_1)); \
}

/* fixed point stuffs */
#define BVDC_P_HSCL_H_PAN_SCAN_I_BITS          (1)
#define BVDC_P_HSCL_H_PAN_SCAN_F_BITS          (6)

#define BVDC_P_HSCL_H_RATIO_I_BITS             (4)
#define BVDC_P_HSCL_H_RATIO_F_BITS             (17)

#define BVDC_P_HSCL_LUMA_INIT_PHASE_I_BITS     (11)
#define BVDC_P_HSCL_LUMA_INIT_PHASE_F_BITS     (6)

#define BVDC_P_HSCL_CHROMA_INIT_PHASE_I_BITS   (11)
#define BVDC_P_HSCL_CHROMA_INIT_PHASE_F_BITS   (6)

#define BVDC_P_HSCL_COEFFS_I_BITS              (1)
#define BVDC_P_HSCL_COEFFS_F_BITS              (10)

#define BVDC_P_HSCL_LARGEST_F_BITS             (17)
#define BVDC_P_HSCL_ZERO_F_BITS                (0)

/* to normalize everything into S14.17 fixed format */
#define BVDC_P_HSCL_NORMALIZE(value, f_bit) ((value) << (BVDC_P_HSCL_LARGEST_F_BITS - (f_bit)))

/* to innormalize everything from S14.17 fixed format */
#define BVDC_P_HSCL_NORM_2_SPEC(value, f_bit) ((value) >> (BVDC_P_HSCL_LARGEST_F_BITS - (f_bit)))

/* Miscellaneous constants */
#define BVDC_P_HSCL_HORZ_REGIONS_COUNT         (1)

#define BVDC_P_HSCL_HORZ_FIR_TAP_COUNT         (8)
#define BVDC_P_HSCL_HORZ_FIR_PHASE_COUNT       (8)

#define BVDC_P_HSCL_FIR_TAP_COUNT_MAX          (8)
#define BVDC_P_HSCL_FIR_PHASE_COUNT_MAX        (8)

#define BVDC_P_HSCL_4TAP_HORZ_THRESHOLD_0      (1280)
#define BVDC_P_HSCL_4TAP_HORZ_THRESHOLD_1      (1024)

#define BVDC_P_HSCL_HORZ_HWF_FACTOR            (2)
#define BVDC_P_HSCL_SRC_HORZ_THRESHOLD         (960)

/* Make Horizontal ratio */
#define BVDC_P_HSCL_MAKE_H_RATIO(src, dst) \
	(BVDC_P_HSCL_NORM_2_SPEC((src), BVDC_P_HSCL_H_RATIO_F_BITS) / (dst))

#define BVDC_P_HSCL_HORZ_1_FIXED BVDC_P_FLOAT_TO_FIXED(1.000, \
	BVDC_P_HSCL_LUMA_INIT_PHASE_I_BITS, BVDC_P_HSCL_LUMA_INIT_PHASE_F_BITS)

#define BVDC_P_HSCL_FIR_COEFFS_MAX \
	(BVDC_P_HSCL_FIR_TAP_COUNT_MAX * BVDC_P_HSCL_FIR_PHASE_COUNT_MAX)

#define BVDC_P_HSCL_LAST UINT32_C(-1)
#else
#define BVDC_P_HSCL_REGS_COUNT     1
#endif

/***************************************************************************
 * Hscaler private data structures
 ***************************************************************************/
typedef struct BVDC_P_HscalerContext
{
	BDBG_OBJECT(BVDC_HSL)

	/* flag initial state, requires reset; */
	bool                           bInitial;

	/* Fir coeff tables */
	const BVDC_P_FirCoeffTbl      *pHorzFirCoeffTbl;
	const BVDC_P_FirCoeffTbl      *pChromaHorzFirCoeffTbl;

	/* update flag. */
	uint32_t                       ulUpdateAll;

	BVDC_P_WindowId                eWinId;

	uint32_t                       ulPrevSrcWidth;
	int32_t                        lPrevHsclCutLeft;
	int32_t                        lPrevHsclCutLeft_R;
	uint32_t                       ulPrevHsclCutWidth;
	uint32_t                       ulPrevOutWidth;
	uint32_t                       ulPrevSrcHeight;
	uint32_t                       ulPrevCtIndexLuma;
	uint32_t                       ulPrevCtIndexChroma;
	BVDC_P_CtInput                 ePrevCtInputType;

	/* private fields. */
	BVDC_P_HscalerId               eId;
	uint32_t                       ulRegOffset; /* HSCL_0, HSCL_1, and etc. */
	uint32_t                       aulRegs[BVDC_P_HSCL_REGS_COUNT];
	BREG_Handle                    hReg;

	/* sub-struct to manage vnet and rul build opreations */
	BVDC_P_SubRulContext           SubRul;

	uint32_t                       ulHorzTaps;

	bool                           bDeringing;

	BFMT_Orientation               ePrevSrcOrientation;
	BFMT_Orientation               ePrevDispOrientation;

	bool                           bChannelChange;
	/* src width alignment: 1 for 444 format internally, 2 for 422 format internally */
	uint32_t                       ulSrcHrzAlign;
} BVDC_P_HscalerContext;


/***************************************************************************
 * Hscaler private functions
 ***************************************************************************/
BERR_Code BVDC_P_Hscaler_Create
	( BVDC_P_Hscaler_Handle           *phHscaler,
	  BVDC_P_HscalerId                 eHscalerId,
	  BVDC_P_Resource_Handle           hResource,
	  BREG_Handle                      hReg );

void BVDC_P_Hscaler_Destroy
	( BVDC_P_Hscaler_Handle            hHscaler );

void BVDC_P_Hscaler_Init_isr
	( BVDC_P_Hscaler_Handle            hHscaler );

void BVDC_P_Hscaler_BuildRul_SrcInit_isr
	( BVDC_P_Hscaler_Handle            hHscaler,
	  BVDC_P_ListInfo                 *pList );

void BVDC_P_Hscaler_BuildRul_isr
	( const BVDC_P_Hscaler_Handle      hHscaler,
	  BVDC_P_ListInfo                 *pList,
	  BVDC_P_State                     eVnetState,
	  BVDC_P_PicComRulInfo            *pPicComRulInfo );

void BVDC_P_Hscaler_SetInfo_isr
	( BVDC_P_Hscaler_Handle            hHscaler,
	  BVDC_Window_Handle               hWindow,
	  const BVDC_P_PictureNodePtr      pPicture );

void BVDC_P_Hscaler_SetEnable_isr
	( BVDC_P_Hscaler_Handle            hHscaler,
	  bool                             bEnable,
	  BVDC_P_ListInfo                  *pList);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_HSCALER_PRIV_H__ */
/* End of file. */

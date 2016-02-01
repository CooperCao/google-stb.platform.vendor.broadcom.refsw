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
#ifndef BVDC_FEEDER_PRIV_H__
#define BVDC_FEEDER_PRIV_H__

#include "bchp_common.h"
#include "bvdc_common_priv.h"
#include "bchp_mfd_0.h"
#include "bvdc_subrul_priv.h"
#include "bvdc_buffer_priv.h"
#include "bvdc_gfxsurface_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * Private defines
 ***************************************************************************/
#if 0
/* The following are legacy chips */
/* 3563:
 * MPEG feeder + BVDC_P_SUPPORT_BVN_10BIT_444. No AVC support, */
#define BVDC_P_MFD_VER_0                            (0)

/* 7401A, 7401B, 7440:
 * Basic AVC feeder*/
#define BVDC_P_MFD_VER_1                            (1)

/* 7401C and above, 7403, 7118:
 * AVC feeder + MFD_0_RANGE_EXP_REMAP_CNTL support */
#define BVDC_P_MFD_VER_2                            (2)
#endif

/* 7400:
 * AVC feeder + MFD_0_RANGE_EXP_REMAP_CNTL + MFD_0_CSC_CNTL +
 * MFD_0_DS_CNTL support */
#define BVDC_P_MFD_VER_3                            (3)

/* 7405:
 * AVC feeder + MFD_0_RANGE_EXP_REMAP_CNTL + MFD_0_CSC_CNTL +
 * MFD_0_DS_CNTL + MFD_0_LAC_CNTL.STRIPE_WIDTH_SEL support */
#define BVDC_P_MFD_VER_4                            (4)

/* 7325, 7335:
 * AVC feeder + MFD_0_RANGE_EXP_REMAP_CNTL +
 * MFD_0_CSC_CNTL (new BVDC_P_SUPPORT_CSC_MAT_COEF_VER) +
 * MFD_0_LAC_CNTL.STRIPE_WIDTH_SEL support */
#define BVDC_P_MFD_VER_5                            (5)

/* 3548 A0:
 * AVC feeder + MFD_0_RANGE_EXP_REMAP_CNTL +
 * BVDC_P_SUPPORT_BVN_10BIT_444 +
 * MFD_0_LAC_CNTL.STRIPE_WIDTH_SEL support +
 * BVDC_P_Feeder_ImageFormat_ePacked_new +
 * MFD_0_BYTE_ORDER
 */
#define BVDC_P_MFD_VER_6                            (6)

/* 3548 B0:
 * AVC feeder + MFD_0_RANGE_EXP_REMAP_CNTL +
 * BVDC_P_SUPPORT_BVN_10BIT_444 +
 * MFD_0_LAC_CNTL.STRIPE_WIDTH_SEL support +
 * BVDC_P_Feeder_ImageFormat_ePacked_new +
 * MFD_0_BYTE_ORDER +
 * CRC Status Register updated on the "Go" Bit
 */
#define BVDC_P_MFD_VER_7                            (7)

/* 7420 A0:
 * AVC feeder + MFD_0_RANGE_EXP_REMAP_CNTL +
 * MFD_0_LAC_CNTL.STRIPE_WIDTH_SEL support +
 * BVDC_P_Feeder_ImageFormat_ePacked_new +
 * MFD_0_BYTE_ORDER +
 * CRC Status Register updated on the "Go" Bit
 */
#define BVDC_P_MFD_VER_8                            (8)

/* 7340, 7342, 7550, 7125, 7468, 7408:
 * AVC feeder + MFD_0_RANGE_EXP_REMAP_CNTL +
 * MFD_0_LAC_CNTL.STRIPE_WIDTH_SEL support +
 * BVDC_P_Feeder_ImageFormat_ePacked_new +
 * MFD_0_BYTE_ORDER +
 * CRC Status Register updated on the "Go" Bit +
 * MFD_0_FEEDER_CNTL.PIXEL_SATURATION_ENABLE
 */
#define BVDC_P_MFD_VER_9                            (9)

/* 7422, 7425Ax, 7231Ax, 7346Ax, 7344Ax:
 * 3D support + Replace MFD_0_PICTURE0_DISP_VERT_WINDOW
 * with MFD_0_DISP_VSIZE + MFD_0_DATA_MODE
 */
#define BVDC_P_MFD_VER_10                           (10)

/* 7358Ax, 7552Ax,
 * Add MFD_0_LUMA_CRC_R, MFD_0_CHROMA_CRC_R
 */
#define BVDC_P_MFD_VER_11                           (11)

/* 7552Bx,
 * New MFD design (MVFD):
 * Remove:
 *  - MFD_0_FEEDER_CNTL.PACKING_TYPE, PACKED_NEW format
 * Add:
 *  - INITIAL_PHASE, LINE_REPEAT, MFD_0_US_422_TO_444_DERING
 */
#define BVDC_P_MFD_VER_12                           (12)

/* 7425 Bx, 7231Bx, 7346Bx, 7344Bx
 * New MFD design (MVFD):
 * Add MRE3D support:
 */
#define BVDC_P_MFD_VER_13                           (13)

/* 7435Ax+, 7429Bx+, 7584Ax+
 * Add BVDC_P_MVFD_ALIGNMENT_WORKAROUND fix.
 */
#define BVDC_P_MFD_VER_14                           (14)

/* 7445Ax, 7145Ax
 * Add 4kx2k and BCHP_MFD_0_TEST_MODE_CNT_DISABLE_DOUBLE_BUFFER
 */
#define BVDC_P_MFD_VER_15                           (15)

/* 7366Bx, 7364Ax, 7445D0
 * Add HEVC interlaced, 10/8-bit dither, VFD_0_DCDM_CFG and VFD_0_DCDM_RECT_SIZE[0..15]
 */
#define BVDC_P_MFD_VER_16                           (16)

/* 7271A:
 * New setting for MFD_0_STRIDE in mosaic mode
 */
#define BVDC_P_MFD_VER_17                           (17)

/* New MFD design (MVFD) */
#define BVDC_P_MFD_FEEDER_IS_MVFD                    \
	(BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_12)

/* MFD_0_FEEDER_CNTL */
#define BVDC_P_MFD_SUPPORT_IMAGE_FORMAT_PACKED_NEW   \
	((BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_6) && \
	 (BVDC_P_SUPPORT_MFD_VER <= BVDC_P_MFD_VER_11))

#define BVDC_P_MFD_SUPPORT_PIXEL_SATURATION_ENABLE   \
	(BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_9)

#define BVDC_P_MFD_SUPPORT_PACKING_TYPE   \
	(BVDC_P_SUPPORT_MFD_VER <= BVDC_P_MFD_VER_11)

#define BVDC_P_MFD_SUPPORT_SMOOTHING_BUFFER   \
	((BVDC_P_SUPPORT_MFD_VER == BVDC_P_MFD_VER_10) || \
	 (BVDC_P_SUPPORT_MFD_VER == BVDC_P_MFD_VER_11))

#define BVDC_P_MFD_SUPPORT_SCB_CLIENT_SEL   \
	(BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_10)

#define BVDC_P_MFD_SUPPORT_INIT_PHASE   \
	(BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_12)

#define BVDC_P_MFD_SUPPORT_SKIP_FIRST_LINE   \
	(BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_12)


#define BVDC_P_MFD_SUPPORT_3D_VIDEO   \
	 (BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_10)

/* VFD_0_FEEDER_CNTL */
#define BVDC_P_VFD_SUPPORT_IMAGE_FORMAT   \
	(BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_6)

/* MFD_0_LAC_CNTL */
#define BVDC_P_MFD_SUPPORT_INTERLACED_HEVC  \
	 (BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_16)

#define BVDC_P_MFD_SUPPORT_STRIPE_WIDTH_SEL  \
	 (BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_4)

#define BVDC_P_MFD_SUPPORT_CHROMA_VERT_POSITION   \
	(BVDC_P_SUPPORT_MFD_VER <= BVDC_P_MFD_VER_11)


#define BVDC_P_MFD_SUPPORT_CSC   \
	((BVDC_P_SUPPORT_MFD_VER == BVDC_P_MFD_VER_3) || \
	 (BVDC_P_SUPPORT_MFD_VER == BVDC_P_MFD_VER_4) || \
	 (BVDC_P_SUPPORT_MFD_VER == BVDC_P_MFD_VER_5))

#define BVDC_P_MFD_SUPPORT_NEW_CSC   \
	 (BVDC_P_SUPPORT_MFD_VER == BVDC_P_MFD_VER_5)

#define BVDC_P_MFD_SUPPORT_10BIT_444   \
	((BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_6) && \
	 (BVDC_P_SUPPORT_MFD_VER <= BVDC_P_MFD_VER_7))

#define BVDC_P_MFD_SUPPORT_BYTE_ORDER   \
	(BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_6)

#define BVDC_P_MFD_SUPPORT_DERINGING    \
	(BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_6)

#define BVDC_P_MFD_SUPPORT_DATA_MODE    \
	((BVDC_P_MFD_SUPPORT_10BIT_444)      || \
	(BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_10))

#define BVDC_P_MFD_SUPPORT_10BIT_DITHER    \
	(BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_16)

#define BVDC_P_MFD_SUPPORT_CRC_TYPE      \
	(BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_16)

#define BVDC_P_MFD_SUPPORT_DCDM          \
	(BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_16)

/* New pitch setting for mosaic */
#define BVDC_P_MFD_SUPPORT_LPDDR4_MEMORY_PITCH   \
	(BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_17)

/* The CRC Status register had some problems when it swapped on end of picture.
 * The issue is that if an interrupt fires but isn't handled until after the
 * picture is finished (very small feed) then the CRC might be wrong.
 * The workaround is to use a scratch register to read the value in RUL.
 * This is fixed in BVDC_P_MFD_VER_7, the update now is to flip based on
 * the "Go" bit for the feeder. */
#define BVDC_P_MFD_NEED_CRC_WORKAROUND               \
	 (BVDC_P_SUPPORT_MFD_VER <= BVDC_P_MFD_VER_6)

#define BVDC_P_MFD_SUPPORT_CRC_R      \
	 (BVDC_P_SUPPORT_MFD_VER >= BVDC_P_MFD_VER_11)

#define BVDC_P_MFD_INVALID_CRC         (0xffffffff)

/***************************************************************************
 * Private macros
 ***************************************************************************/

#define BVDC_P_MFD_GET_REG_IDX(reg) \
	((BCHP##_##reg - BCHP_MFD_0_REG_START) / sizeof(uint32_t))

/* Get register data */
#define BVDC_P_MFD_GET_REG_DATA(reg) \
	(hFeeder->aulRegs[BVDC_P_MFD_GET_REG_IDX(reg)])
#define BVDC_P_MFD_SET_REG_DATA(reg, data) \
	(BVDC_P_MFD_GET_REG_DATA(reg) = (uint32_t)(data))

/* Get field */
#define BVDC_P_MFD_GET_FIELD_NAME(reg, field) \
	((BVDC_P_MFD_GET_REG_DATA(reg) & BCHP##_##reg##_##field##_MASK) >> \
	BCHP##_##reg##_##field##_SHIFT)

/* Compare field */
#define BVDC_P_MFD_COMPARE_FIELD_DATA(reg, field, data) \
	(BVDC_P_MFD_GET_FIELD_NAME(reg, field)==(data))

#define BVDC_P_MFD_COMPARE_FIELD_NAME(reg, field, name) \
	(BVDC_P_MFD_GET_FIELD_NAME(reg, field)==BCHP##_##reg##_##field##_##name)

#define BVDC_P_MFD_WRITE_TO_RUL(reg, addr_ptr) \
{ \
	*addr_ptr++ = BRDC_OP_IMM_TO_REG(); \
	*addr_ptr++ = BRDC_REGISTER(BCHP##_##reg + hFeeder->ulRegOffset); \
	*addr_ptr++ = BVDC_P_MFD_GET_REG_DATA(reg); \
}

/* This macro does a block write into RUL */
#define BVDC_P_MFD_BLOCK_WRITE_TO_RUL(from, to, pulCurrent) do { \
	uint32_t ulBlockSize = \
		BVDC_P_REGS_ENTRIES(from, to);\
	*pulCurrent++ = BRDC_OP_IMMS_TO_REGS( ulBlockSize ); \
	*pulCurrent++ = BRDC_REGISTER(BCHP##_##from + hFeeder->ulRegOffset); \
	BKNI_Memcpy((void*)pulCurrent, \
		(void*)&(hFeeder->aulRegs[BVDC_P_MFD_GET_REG_IDX(from)]), \
		ulBlockSize * sizeof(uint32_t)); \
	pulCurrent += ulBlockSize; \
} while (0)

/* check parameters */
#define BVDC_P_MFD_REGS_COUNT    \
	BVDC_P_REGS_ENTRIES(MFD_0_REG_START, MFD_0_REG_END)

#define BVDC_P_Feeder_IS_MPEG(hFeeder) \
	((BVDC_P_FeederId_eVfd0 > (hFeeder->eId)) ? true : false)

#define BVDC_P_Feeder_IS_VFD(hFeeder) \
	((BVDC_P_FeederId_eVfd0<=(hFeeder->eId)) ? true : false)

/* Gets macroblock pan scan. The result pan scan is in the uint
 * of macroblock.
 * This is related to frame buffer organization in MVD. */
#define BVDC_P_Feeder_GET_MB_PANSCAN(PanScan) \
	( PanScan >> BVDC_P_16TH_PIXEL_SHIFT )

/* Gets pixel pan scan. The result pan scan is in the uint
 * of pixel.
 * This is related to frame buffer organization in MVD. */
#define BVDC_P_Feeder_GET_PIXEL_PANSCAN(PanScan) \
	( PanScan & 0xf )

/* Combines Macroblock pan scan and pixel pan scan together.
 * This is related to frame buffer organization in MVD. */
#define BVDC_P_Feeder_GET_COMBINE_PANSCAN(PanScan) \
	( (BVDC_P_Feeder_GET_MB_PANSCAN(PanScan)<< BVDC_P_16TH_PIXEL_SHIFT) + \
	BVDC_P_Feeder_GET_PIXEL_PANSCAN(PanScan) )

#define BVDC_P_FEEDER_IMAGE_FORMAT_IS_MPEG(eImageFormat) \
	((eImageFormat) == BVDC_P_Feeder_ImageFormat_eAVC_MPEG)

#define BVDC_P_FEEDER_IMAGE_FORMAT_IS_PACKED(eImageFormat) \
	(((eImageFormat) == BVDC_P_Feeder_ImageFormat_ePacked) ||  \
	 ((eImageFormat) == BVDC_P_Feeder_ImageFormat_ePacked_new))

#define BVDC_P_FEEDER_IMAGE_FORMAT_IS_YUV444(eImageFormat) \
	((eImageFormat) == BVDC_P_Feeder_ImageFormat_eYUV444)

#define BVDC_P_FEEDER_PIXEL_FORMAT_IS_10BIT(ePxlFmt) \
	(((ePxlFmt) == BVDC_P_CAP_PIXEL_FORMAT_10BIT444) ||  \
	 ((ePxlFmt) == BVDC_P_CAP_PIXEL_FORMAT_10BIT422))


/* NOTE: weirdness of VNET_F crossbar puts MFD2 in non-contiguous value */
#ifdef BCHP_VNET_F_SCL_0_SRC_SOURCE_MPEG_Feeder_2
#define BVDC_P_Feeder_PostMuxValue(hFeeder)  \
    ((BVDC_P_FeederId_eMfd2 == (hFeeder)->eId)? (BCHP_VNET_F_SCL_0_SRC_SOURCE_MPEG_Feeder_2) :  \
	 (BVDC_P_FeederId_eMfd2 > (hFeeder)->eId)? (BCHP_VNET_F_SCL_0_SRC_SOURCE_MPEG_Feeder_0 + (hFeeder)->eId) :  \
	 (BVDC_P_FeederId_eVfd0 > (hFeeder)->eId)? (BCHP_VNET_F_SCL_0_SRC_SOURCE_MPEG_Feeder_0 + (hFeeder)->eId) :  \
	 (BCHP_VNET_F_SCL_0_SRC_SOURCE_Video_Feeder_0 + (hFeeder)->eId - BVDC_P_FeederId_eVfd0))
#else
#define BVDC_P_Feeder_PostMuxValue(hFeeder)  \
    ((BVDC_P_FeederId_eMfd2 > (hFeeder)->eId)? (BCHP_VNET_F_SCL_0_SRC_SOURCE_MPEG_Feeder_0 + (hFeeder)->eId) :  \
	 (BCHP_VNET_F_SCL_0_SRC_SOURCE_Video_Feeder_0 + (hFeeder)->eId - BVDC_P_FeederId_eVfd0))
#endif

#define BVDC_P_feeder_SetRulBuildWinId_isr(hFeeder, eWinId) \
    BVDC_P_SubRul_SetRulBuildWinId_isr(&((hFeeder)->SubRul), eWinId)

/***************************************************************************
 * MPEG/Video feeder private data structures
 ***************************************************************************/
/* Position of chroma pixel component corresponding to luma
 * pixel component in vertical direction */
typedef enum BVDC_P_Feeder_ChromaVertPos
{
	BVDC_P_Feeder_ChromaVertPos_eColorcatedWithLuma = 0,
	BVDC_P_Feeder_ChromaVertPos_eHalfPixelGridBetweenLuma

} BVDC_P_Feeder_ChromaVertPos;


/* Image format */
typedef enum BVDC_P_Feeder_ImageFormat
{
#if (BVDC_P_SUPPORT_MFD_VER == BVDC_P_MFD_VER_6)
	BVDC_P_Feeder_ImageFormat_ePacked = 0,
	BVDC_P_Feeder_ImageFormat_eYUV444,
	BVDC_P_Feeder_ImageFormat_eAVC_MPEG,
#else
	BVDC_P_Feeder_ImageFormat_eAVC_MPEG = 0,
	BVDC_P_Feeder_ImageFormat_ePacked,
	BVDC_P_Feeder_ImageFormat_eYUV444,
#endif

	BVDC_P_Feeder_ImageFormat_ePacked_new,

	BVDC_P_Feeder_ImageFormat_eUnknown

} BVDC_P_Feeder_ImageFormat;

typedef enum BVDC_P_Feeder_VideoFormatMode
{
	BVDC_P_Feeder_VideoFormatMode_e2D = 0,
	BVDC_P_Feeder_VideoFormatMode_e3DLR,
	BVDC_P_Feeder_VideoFormatMode_e3DOU,
	BVDC_P_Feeder_VideoFormatMode_e3DDP,
	BVDC_P_Feeder_VideoFormatMode_e3DMR

} BVDC_P_Feeder_VideoFormatMode;

/* Stripe width settings. */
typedef struct
{
	BAVC_StripeWidth               eStripeWidth;
	uint32_t                       ulStripeWidth;
	uint32_t                       ulShift;

} BVDC_P_Feeder_StripeWidthConfig;

/* Device address settings. */
typedef struct
{
	uint32_t        ulLumaDeviceAddr;
	uint32_t        ulChromaDeviceAddr;
#if (BVDC_P_MFD_SUPPORT_INTERLACED_HEVC) /* for fields pair buffer */
	uint32_t        ulLumaBotDeviceAddr;
	uint32_t        ulChromaBotDeviceAddr;
#endif

#if (BVDC_P_MFD_SUPPORT_3D_VIDEO)
	uint32_t        ulLumaDeviceAddr_R;
	uint32_t        ulChromaDeviceAddr_R;
#if (BVDC_P_MFD_SUPPORT_INTERLACED_HEVC) /* for fields pair buffer */
	uint32_t        ulLumaBotDeviceAddr_R;
	uint32_t        ulChromaBotDeviceAddr_R;
#endif

#if (BVDC_P_MFD_SUPPORT_3D_VIDEO_MRE)
	uint32_t        ulLumaDeviceAddr_hp;
	uint32_t        ulChromaDeviceAddr_hp;
	uint32_t        ulLumaDeviceAddr_R_hp;
	uint32_t        ulChromaDeviceAddr_R_hp;
#endif

#endif

} BVDC_P_Feeder_MpegDeviceAddrConfig;

typedef struct BVDC_P_FeederContext
{
	BDBG_OBJECT(BVDC_FDR)

	BVDC_Source_Handle             hSource;
	BMMA_Heap_Handle               hMem;
	bool                           bInitial;
	uint32_t                       ulResetMask;
	uint32_t                       ulResetRegAddr;
	uint32_t                       ulVnetResetAddr;
	uint32_t                       ulVnetResetMask;

	BVDC_P_FeederId                eId;
	/* This is the regular offset from BCHP_MFD_0_REG_START */
	uint32_t                       ulRegOffset;
	/* This is the offset from BCHP_VFD_0_REG_START, apply to VFD_x only.
	 * Used for registers in VFD not in MFD, or registers have different
	 * offset in MFD and VFD
	 */
	uint32_t                       ulVfd0RegOffset;
	uint32_t                       aulRegs[BVDC_P_MFD_REGS_COUNT];

	bool                           bSupportDcxm;

	BPXL_Format                    ePxlFormat;
	BVDC_P_Feeder_ImageFormat      eImageFormat;

	BFMT_Orientation               eInputOrientation;
	BFMT_Orientation               eOutputOrientation;

	/* A register handle. */
	BRDC_Handle                    hRdc;
	BREG_Handle                    hRegister;

	/* Keeps track of when ISR was executed */
	uint32_t                       ulTimestamp;

#if (!BVDC_P_USE_RDC_TIMESTAMP)
	BTMR_TimerHandle               hTimer;
	BTMR_TimerRegisters            stTimerReg;
	uint32_t                       ulTimestampRegAddr;
#endif

#if (BVDC_P_MFD_NEED_CRC_WORKAROUND)
	uint32_t                       ulLumaCrcRegAddr;
	uint32_t                       ulChromaCrcRegAddr;
#endif

	/* sub-struct to manage vnet and rul build opreations */
	BVDC_P_SubRulContext           SubRul;

	/* 422 to 444 up sampler setting */
	BVDC_422To444UpSampler         stUpSampler;

	/* Window associated with this vfd.
	 * note: one vfd can not be shared by more than one window */
	BVDC_Window_Handle             hWindow;

	/* gfx surface manager :
	 * For Video Feeder cores with a minor version earlier than 0x50, the
	 * surface for the video source must have a pixel format of either
	 * BPXL_eY18_Cr8_Y08_Cb8, BPXL_eCr8_Y18_Cb8_Y08, BPXL_eY18_Cb8_Y08_Cr8,
	 * or BPXL_eCb8_Y18_Cr8_Y08; otherwise pink and green video will result.
	 * Cores with minor version 0x50 and later can support all formats.
	 */
	BVDC_P_GfxSurfaceContext       stGfxSurface;
	BVDC_P_SurfaceInfo            *pNewSur;

	bool                           bGfxSrc;
	bool                           b3dSrc;

	/* MTG: mpeg timer generator */
	bool                           bMtgSrc;
#if BVDC_P_SUPPORT_MTG
	bool                           bMtgInited;
	bool                           bMtgInterlaced;
	BAVC_FrameRateCode             eMtgFrameRateCode;  /* 50 or 60 */
#endif

	/* Used only when VFD is used to display graphics surface */
	BVDC_P_PictureNode             stPicture;
	bool                           bFixedColor;

	/* For MFD only */
	uint32_t                       ulThroughput;
	bool                           bSkipFirstLine;
	uint32_t                       ulChromaStride;
	uint32_t                       ulLumaStride;
	uint32_t                       ulPicOffset;
	uint32_t                       ulPicOffset_R;
	BVDC_Source_CrcType            eCrcType;
	BAVC_MFD_Picture               stMfdPicture;

} BVDC_P_FeederContext;


/***************************************************************************
 * MPEG/Video feeder private functions
 ***************************************************************************/
BERR_Code BVDC_P_Feeder_Create
	( BVDC_P_Feeder_Handle            *phFeeder,
	  BRDC_Handle                      hRdc,
	  const BREG_Handle                hReg,
	  const BVDC_P_FeederId            eFeederId,
#if (!BVDC_P_USE_RDC_TIMESTAMP)
	  BTMR_TimerHandle                 hTimer,
#endif
	  BVDC_Source_Handle               hSource,
	  BVDC_P_Resource_Handle           hResource,
	  bool                             b3dSrc );

void BVDC_P_Feeder_Destroy
	( BVDC_P_Feeder_Handle             hFeeder );

void BVDC_P_Feeder_Init
	( BVDC_P_Feeder_Handle             hFeeder,
	  BVDC_Window_Handle               hWindow,
	  BMMA_Heap_Handle                 hMem,
	  bool                             bGfxSrc,
	  bool                             bMtgSrc );

void BVDC_P_Feeder_BuildRul_isr
	( const BVDC_P_Feeder_Handle       hFeeder,
	  BVDC_P_ListInfo                 *pList,
	  const BVDC_P_PictureNodePtr      pPicture,
	  BVDC_P_State                     eVnetState,
	  BVDC_P_PicComRulInfo            *pPicComRulInfo );

BERR_Code BVDC_P_Feeder_SetMpegAddr_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  const BAVC_MVD_Field            *pFieldData,
	  const BVDC_P_Rect               *pScanoutRect);

BERR_Code BVDC_P_Feeder_SetMpegInfo_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  const uint32_t                   ulMuteColorYCrCb,
	  const BAVC_MVD_Field            *pFieldData,
	  const BVDC_P_Rect               *pScanOutRect);

void BVDC_P_Feeder_SetPlaybackInfo_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  const BVDC_P_PictureNodePtr      pPicture,
	  bool                             bFixedColor,
	  const uint32_t                   ulMuteColorYCrCb );

BERR_Code BVDC_P_Feeder_SetEnable_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  bool                             bEnable );

void BVDC_P_Feeder_GetCrc_isr
	( BVDC_P_Feeder_Handle             hFeeder,
	  BREG_Handle                      hReg,
	  BVDC_Source_CallbackData        *pData );

BERR_Code BVDC_P_Feeder_ValidateChanges
	( BVDC_P_Feeder_Handle             hFeeder,
	  BVDC_Source_PictureCallback_isr  pfPicCallbackFunc );

BERR_Code BVDC_P_Feeder_ApplyChanges_isr
	( BVDC_P_Feeder_Handle             hFeeder );

void BVDC_P_Feeder_AbortChanges
	( BVDC_P_Feeder_Handle             hFeeder );

BERR_Code BVDC_P_Feeder_ValidateGfxSurAndRects_isr
	( BVDC_P_SurfaceInfo                *pSur,
	  const BVDC_P_ClipRect             *pClipRect,
	  const BVDC_P_Rect                 *pSclOutRect );
#define BVDC_P_Feeder_ValidateGfxSurAndRects  BVDC_P_Feeder_ValidateGfxSurAndRects_isr

void BVDC_P_Feeder_HandleIsrGfxSurface_isr
	( BVDC_P_Feeder_Handle         hFeeder,
  	  BVDC_P_Source_Info *         pCurSrcInfo,
	  BAVC_Polarity                eFieldId );

#if BVDC_P_SUPPORT_MTG
void BVDC_P_Source_MtgCallback_isr
	( void                            *pvSourceHandle,
	  int                              iParam2 );

void BVDC_P_Feeder_Mtg_Bringup_isr
	( BVDC_P_Feeder_Handle             hFeeder );

void BVDC_P_Feeder_Mtg_MpegDataReady_isr
	( BVDC_P_Feeder_Handle             hFeeder,
      BRDC_List_Handle                 hList,
	  BAVC_MVD_Field                  *pNewPic);
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_FEEDER_PRIV_H__ */
/* End of file. */

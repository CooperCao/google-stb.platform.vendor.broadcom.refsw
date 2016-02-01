/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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
#include "bstd.h"                /* standard types */
#include "bdbg.h"                /* Dbglib */
#include "bkni.h"                /* malloc */
#include "bmem.h"

#include "bchp_common.h"
#include "bchp_vnet_f.h"
#include "bchp_vnet_b.h"
#include "bchp_clkgen.h"
#include "bchp_vfd_0.h"
#include "bchp_fmisc.h"
#include "bchp_mmisc.h"
#include "bchp_bmisc.h"

#include "btst_video_shmoo.h"

#if (defined(BCHP_PWR_SUPPORT))
#include "bchp_pwr_resources.h"
#include "bchp_pwr.h"
#endif

#define BTST_P_CAPTURE_CRC_PICTURE          0

#if BTST_P_CAPTURE_CRC_PICTURE
#include "bchp_cap_0.h"
#include "bchp_cap_1.h"
#endif

static BREG_Handle g_hRegister;

#define BTST_P_VIDEO_SHMOO_SURFACE_WIDTH    720
#define BTST_P_VIDEO_SHMOO_SURFACE_HEIGHT   6

#define BTST_P_SHMOO_VFD_GET_REG_IDX(reg) \
	((BCHP##_##reg - BCHP_VFD_0_REG_START))

#define BTST_P_ALIGN_UP(addr, alignment) \
	(((uint32_t)(addr) + (alignment) - 1) & ~((alignment) - 1))

#define BTST_P_ALIGN_DN(addr, alignment) \
	(((uint32_t)(addr)) & ~((alignment) - 1))

#define BTST_P_IS_ALIGN(addr, alignment) \
	(~(((uint32_t)(addr)) & ~((alignment)-1)))

#define BTST_REG_WRITE(reg, data) \
	(BREG_Write32(g_hRegister, reg, data))

#define BTST_REG_READ(reg) \
	(BREG_Read32(g_hRegister, reg))

/* Pixel helper macros */
#define BTST_PXL_P_COMP_SIZE_MASK  0xF
#define BTST_PXL_P_COMP_SIZE_SHIFT 4
#define BTST_PXL_P_COMP_LOC_MASK   0x3
#define BTST_PXL_P_COMP_LOC_SHIFT  16
#define BTST_PXL_P_COMP_LOC(f,c) \
	(((f) >> (BTST_PXL_P_COMP_LOC_SHIFT + (c) * 2)) & BTST_PXL_P_COMP_LOC_MASK)

#define BTST_PXL_COMPONENT_SIZE(x,n) \
	(((x) >> (BTST_PXL_P_COMP_LOC(x,n) * BTST_PXL_P_COMP_SIZE_SHIFT)) & BTST_PXL_P_COMP_SIZE_MASK)

#define BTST_PXL_P_COMP_SIZE(f,c) \
	(((f) >> (c * BTST_PXL_P_COMP_SIZE_SHIFT)) & BTST_PXL_P_COMP_SIZE_MASK)

#define BTST_PXL_COMPONENT_POS(x,n) \
	((BTST_PXL_COMPONENT_SIZE(x,n) == 0) ? 0 : \
	((BTST_PXL_P_COMP_LOC(x,n) == 0) ? 0 : \
	((BTST_PXL_P_COMP_LOC(x,n) == 1) ? BTST_PXL_P_COMP_SIZE(x,0) : \
	((BTST_PXL_P_COMP_LOC(x,n) == 2) ? BTST_PXL_P_COMP_SIZE(x,0) + BTST_PXL_P_COMP_SIZE(x,1) : \
	(BTST_PXL_P_COMP_SIZE(x,0) + BTST_PXL_P_COMP_SIZE(x,1) + BTST_PXL_P_COMP_SIZE(x,2))))))

#if (BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE)
#define BTST_PXL_MAKE_PIXEL(x,c3,c2,c1,c0) ( \
	(((c3) >> (8 - BTST_PXL_COMPONENT_SIZE(x,3))) << BTST_PXL_COMPONENT_POS(x,3)) | \
	(((c2) >> (8 - BTST_PXL_COMPONENT_SIZE(x,2))) << BTST_PXL_COMPONENT_POS(x,2)) | \
	(((c1) >> (8 - BTST_PXL_COMPONENT_SIZE(x,1))) << BTST_PXL_COMPONENT_POS(x,1)) | \
	(((c0) >> (8 - BTST_PXL_COMPONENT_SIZE(x,0))) << BTST_PXL_COMPONENT_POS(x,0)))
#else
#define BTST_PXL_MAKE_PIXEL(x,c3,c2,c1,c0) ( \
	(((c3) >> (8 - BTST_PXL_COMPONENT_SIZE(x,3))) << BTST_PXL_COMPONENT_POS(x,0)) | \
	(((c2) >> (8 - BTST_PXL_COMPONENT_SIZE(x,2))) << BTST_PXL_COMPONENT_POS(x,1)) | \
	(((c1) >> (8 - BTST_PXL_COMPONENT_SIZE(x,1))) << BTST_PXL_COMPONENT_POS(x,2)) | \
	(((c0) >> (8 - BTST_PXL_COMPONENT_SIZE(x,0))) << BTST_PXL_COMPONENT_POS(x,3)))
#endif

#define BTST_PXL_BITS_PER_PIXEL(x) ((\
	(((x) & BTST_PXL_P_COMP_SIZE_MASK) + \
	(((x) >> BTST_PXL_P_COMP_SIZE_SHIFT) & BTST_PXL_P_COMP_SIZE_MASK) + \
	(((x) >> (BTST_PXL_P_COMP_SIZE_SHIFT * 2)) & BTST_PXL_P_COMP_SIZE_MASK) + \
	(((x) >> (BTST_PXL_P_COMP_SIZE_SHIFT * 3)) & BTST_PXL_P_COMP_SIZE_MASK))) / \
	(2))

BDBG_MODULE(BTST_VIDEO_SHMOO);
BDBG_OBJECT_ID(BTST_VIDEO_SHMOO);

static void BTST_P_ResetBvn
	( void )
{
	bool bFreeRun;
	uint32_t ulReg;

	/* Note we can not use the SUN_TOP_CTRL_SW_RESET, because it would
	 * also reset the RDC, and MEMC memory controller. */
#if (BVDC_P_SUPPORT_CLOCK_GATING)
	bFreeRun = false;
#else
	bFreeRun = true;
#endif

	/* Reset BVN front-end, middle-end, and back-end modules.
	 * To reset write a 1 to a bit, and then write a 0.*/
	/*---------------------------*/
	/* FRONT-END & MIDDLE BLOCKS */
	/*---------------------------*/
	ulReg = 0xffffffff;
	ulReg &= ~(

	BCHP_MASK(FMISC_SW_INIT, RDC ));
	BTST_REG_WRITE(BCHP_FMISC_SW_INIT, ulReg);
	BTST_REG_WRITE(BCHP_FMISC_SW_INIT, 0);

 	BTST_REG_WRITE(BCHP_FMISC_BVNF_CLOCK_CTRL,
		BCHP_FIELD_DATA(FMISC_BVNF_CLOCK_CTRL, CLK_FREE_RUN_MODE, bFreeRun));

	/*---------------*/
	/* MIDDLE BLOCKS */
	/*---------------*/
	ulReg = 0xffffffff;
	BTST_REG_WRITE(BCHP_MMISC_SW_INIT, ulReg);
	BTST_REG_WRITE(BCHP_MMISC_SW_INIT, 0);

	/*------------------*/
	/* BACK-END BLOCKS */
	/*------------------*/
	ulReg = 0xffffffff;
 	BTST_REG_WRITE(BCHP_BMISC_SW_INIT, ulReg);
	BTST_REG_WRITE(BCHP_BMISC_SW_INIT, 0);

 	BTST_REG_WRITE(BCHP_BMISC_BVNB_CLOCK_CTRL,
		BCHP_FIELD_DATA(BMISC_BVNB_CLOCK_CTRL, CLK_FREE_RUN_MODE, bFreeRun));

	return;
}

/***************************************************************************/
static BERR_Code BTST_P_GetBytesPerNPixels(
	uint32_t     ulPixelFormat,
	unsigned int uiNPixels,
	unsigned int *puiBytes )
{
	unsigned int uiBytes = 0;
	unsigned int uiBitsPerPixel;

	/* Check parameter */
	if( puiBytes == NULL )
	{
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	uiBitsPerPixel = BTST_PXL_BITS_PER_PIXEL(ulPixelFormat);

	/* calculate bytes for sub-byte formats */
	if( uiBitsPerPixel < 8 )
	{
		uiNPixels += 8 / uiBitsPerPixel - 1;
		uiBytes = uiNPixels / (8 / uiBitsPerPixel);
	}
	/* calculate bytes for formats that are a byte or larger */
	else
	{
		uiNPixels = (uiNPixels + 1) & (~1);
		uiBytes = (uiNPixels * uiBitsPerPixel) / 8;
	}

	*puiBytes = uiBytes;
	return BERR_SUCCESS;
}

static void BTST_P_CreateLumaRampSurface
	( uint32_t  xi,
	  uint32_t  x_size,
	  uint32_t  yi,
	  uint32_t  y_size,
	  uint8_t  *y,
	  uint8_t  *cb,
	  uint8_t  *cr,
	  bool      bHorz )
{
	uint32_t value;
	uint8_t  uiMinY = 50, uiMaxY = 100;

	/* horizontal ramp */
	if(bHorz)
	{
		value = (uint16_t)((uiMaxY - uiMinY) * xi / (x_size - 1) + uiMinY);

		if(value >= uiMaxY)
		{
			value = uiMaxY;
		}
		else if(value == uiMinY)
		{
			value = uiMinY;
		}

        *y = (uint8_t) value;
		*cb = 0x80; *cr = 0x80;
	}
	else
	{
		value = (uint16_t)((uiMaxY - uiMinY) * yi / (y_size - 1) + uiMinY);

		if(value >= uiMaxY)
		{
			value = uiMaxY;
		}

        *y = (uint8_t) value;
		*cb = 0x80; *cr = 0x80;
	}
}

static BERR_Code BTST_P_MakeShmooSurface
	( uint8_t                  *puiAddr,
	  uint32_t                  ulPixelFormat,
	  uint32_t                  ulStride,
	  uint32_t                  ulWidth,
	  uint32_t                  ulHeight )
{
	uint32_t ii, jj = 0;
	uint8_t  Y, Cb, Cr;
	uint32_t ulStartAddr = (uint32_t)puiAddr;

	/* fill in each line of the frame */
	for( ii = 0; ii < ulHeight; ii++ )
	{
		/* for each pixel of the line */
		for( jj = 0; jj < ulWidth; jj++ )
		{
			/* horizontal ramp */
			BTST_P_CreateLumaRampSurface( jj, ulWidth, ii, ulHeight, &Y, &Cb, &Cr, true );

			/* restore start address */
			puiAddr = (uint8_t *)ulStartAddr;
			puiAddr +=  ii  * ulStride;

			if( jj & 1 )
			{
				puiAddr[(jj-1) * 2 + BTST_PXL_COMPONENT_POS(ulPixelFormat, 2) / 8] = Y;

#if BTST_P_CAPTURE_CRC_PICTURE
				printf("pixel [%dx%d] : Y[0x%x]\n", ii, jj,
					puiAddr[(jj-1) * 2 + BTST_PXL_COMPONENT_POS(ulPixelFormat, 2) / 8]);
#endif
			}
			else
			{
				((uint32_t *) puiAddr) [jj / 2] =
					BTST_PXL_MAKE_PIXEL(ulPixelFormat, Y, Y, Cb, Cr);
#if BTST_P_CAPTURE_CRC_PICTURE
				printf("pixel [%dx%d] : YCbCr[0x%x]\n", ii, jj,
					((uint32_t *) puiAddr) [jj / 2]);
#endif
			}
		}/* end of width for loop */
	}/* end of height for loop */

	return BERR_SUCCESS;
}


#if BTST_P_CAPTURE_CRC_PICTURE
static BERR_Code BTST_P_SetupCap
	( uint32_t                         ulWidth,
	  uint32_t                         ulHeight,
	  uint32_t                         ulStride,
	  uint32_t                         ulCapOffset )
{
	uint32_t ulRegData = 0;
	uint32_t ulByteOrder = 0;

	/* setup CAP VNET path */
	BTST_REG_WRITE(BCHP_VNET_B_CAP_1_SRC, BCHP_VNET_B_CAP_1_SRC_SOURCE_Free_Ch_0);

	BTST_REG_WRITE(BCHP_CAP_1_CTRL, BCHP_CAP_0_CTRL_CAP_ENABLE_ENABLE);

#if (BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG)
	ulByteOrder |= (
		BCHP_FIELD_ENUM(VFD_0_BYTE_ORDER, BYTE_0_SEL, CR) |
		BCHP_FIELD_ENUM(VFD_0_BYTE_ORDER, BYTE_1_SEL, Y0) |
		BCHP_FIELD_ENUM(VFD_0_BYTE_ORDER, BYTE_2_SEL, CB) |
		BCHP_FIELD_ENUM(VFD_0_BYTE_ORDER, BYTE_3_SEL, Y1));
#else
	ulByteOrder |= (
		BCHP_FIELD_ENUM(VFD_0_BYTE_ORDER, BYTE_0_SEL, Y1) |
		BCHP_FIELD_ENUM(VFD_0_BYTE_ORDER, BYTE_1_SEL, CB) |
		BCHP_FIELD_ENUM(VFD_0_BYTE_ORDER, BYTE_2_SEL, Y0) |
		BCHP_FIELD_ENUM(VFD_0_BYTE_ORDER, BYTE_3_SEL, CR));
#endif

	BTST_REG_WRITE(BCHP_CAP_1_BYTE_ORDER, ulByteOrder);

	BTST_REG_WRITE(BCHP_CAP_1_MODE, 0x0);

	ulRegData = 0;
	BTST_REG_WRITE(BCHP_CAP_1_PIC_SIZE, ulRegData);
	ulRegData |= (ulWidth << BCHP_CAP_0_PIC_SIZE_HSIZE_SHIFT);
	ulRegData |= (ulHeight << BCHP_CAP_0_PIC_SIZE_VSIZE_SHIFT);
	BTST_REG_WRITE(BCHP_CAP_1_PIC_SIZE, ulRegData);

	ulRegData = 0;
	BTST_REG_WRITE(BCHP_CAP_1_PIC_OFFSET, ulRegData);
	ulRegData |= (0 << BCHP_CAP_0_PIC_OFFSET_HSIZE_SHIFT);
	ulRegData |= (0 << BCHP_CAP_0_PIC_OFFSET_VSIZE_SHIFT);
	BTST_REG_WRITE(BCHP_CAP_1_PIC_OFFSET, ulRegData);

	ulRegData = 0;
	BTST_REG_WRITE(BCHP_CAP_1_BVB_IN_SIZE, ulRegData);
	ulRegData |= (ulWidth << BCHP_CAP_0_BVB_IN_SIZE_HSIZE_SHIFT);
	ulRegData |= (ulHeight << BCHP_CAP_0_BVB_IN_SIZE_VSIZE_SHIFT);
	BTST_REG_WRITE(BCHP_CAP_1_BVB_IN_SIZE, ulRegData);

	/* set mstart */
	BTST_REG_WRITE(BCHP_CAP_1_MSTART, ulCapOffset);

	BTST_REG_WRITE(BCHP_CAP_1_PITCH, ulStride);

	return BERR_SUCCESS;
}
#endif

static BERR_Code BTST_P_SetupVfd
	( uint8_t                          uiVfdId,
	  uint32_t                         ulStride,
	  uint32_t                         ulWidth,
	  uint32_t                         ulHeight,
	  uint32_t                         ulPixelFmt,
	  uint32_t                         ulOffset )
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ulVfdRegStart, ulVfdRegEnd;
	uint32_t ulRegOffset;
	uint32_t ulRegData;
	bool     b8BitMode;
	uint32_t ulPackingType, ulByteOrder = 0, ulImageFormat;

	BSTD_UNUSED(ulPixelFmt);

	switch (uiVfdId)
	{
		case BCHP_VNET_F_FCH_0_SRC_SOURCE_Video_Feeder_0:
			ulVfdRegStart = BCHP_VFD_0_REG_START;
			ulVfdRegEnd = BCHP_VFD_0_REG_END;
			break;
		case BCHP_VNET_F_FCH_0_SRC_SOURCE_Video_Feeder_1:
			ulVfdRegStart = BCHP_VFD_1_REG_START;
			ulVfdRegEnd = BCHP_VFD_1_REG_END;
			break;
		case BCHP_VNET_F_FCH_0_SRC_SOURCE_Video_Feeder_2:
			ulVfdRegStart = BCHP_VFD_2_REG_START;
			ulVfdRegEnd = BCHP_VFD_2_REG_END;
			break;
		case BCHP_VNET_F_FCH_0_SRC_SOURCE_Video_Feeder_3:
			ulVfdRegStart = BCHP_VFD_3_REG_START;
			ulVfdRegEnd = BCHP_VFD_3_REG_END;
			break;
#if BCHP_VFD_4_REG_START
		case BCHP_VNET_F_FCH_0_SRC_SOURCE_Video_Feeder_4:
			ulVfdRegStart = BCHP_VFD_4_REG_START;
			ulVfdRegEnd = BCHP_VFD_4_REG_END;
			break;
#endif
		default:
			err = BERR_INVALID_PARAMETER;
            return err;
			break;
	}

	/* set the width */
	ulRegOffset = BTST_P_SHMOO_VFD_GET_REG_IDX(VFD_0_DISP_HSIZE);
	ulRegData = (ulWidth + 1) & ~1;
	BTST_REG_WRITE(ulVfdRegStart + ulRegOffset, ulRegData);

	/* set the height */
	ulRegOffset = BTST_P_SHMOO_VFD_GET_REG_IDX(VFD_0_DISP_VSIZE);
	ulRegData = ulHeight;
	BTST_REG_WRITE(ulVfdRegStart + ulRegOffset, ulRegData);


	/* set packing type, compression, byte order, image format and orientation */
	b8BitMode = true;

	ulPackingType = 2; /* PACKING_TYPE_Y0_U0_Y1_V1 */

	ulImageFormat = 0x3; /* packed new */

#if (BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG)
	ulByteOrder |= (
		BCHP_FIELD_ENUM(VFD_0_BYTE_ORDER, BYTE_0_SEL, CR) |
		BCHP_FIELD_ENUM(VFD_0_BYTE_ORDER, BYTE_1_SEL, Y0) |
		BCHP_FIELD_ENUM(VFD_0_BYTE_ORDER, BYTE_2_SEL, CB) |
		BCHP_FIELD_ENUM(VFD_0_BYTE_ORDER, BYTE_3_SEL, Y1));
#else
	ulByteOrder |= (
		BCHP_FIELD_ENUM(VFD_0_BYTE_ORDER, BYTE_0_SEL, Y1) |
		BCHP_FIELD_ENUM(VFD_0_BYTE_ORDER, BYTE_1_SEL, CB) |
		BCHP_FIELD_ENUM(VFD_0_BYTE_ORDER, BYTE_2_SEL, Y0) |
		BCHP_FIELD_ENUM(VFD_0_BYTE_ORDER, BYTE_3_SEL, CR));
#endif

	ulRegData = (ulImageFormat << BCHP_VFD_0_FEEDER_CNTL_IMAGE_FORMAT_SHIFT) |
				(ulPackingType << BCHP_VFD_0_FEEDER_CNTL_PACKING_TYPE_SHIFT) |
				(BCHP_VFD_0_FEEDER_CNTL_FIXED_COLOUR_ENABLE_OFF << BCHP_VFD_0_FEEDER_CNTL_FIXED_COLOUR_ENABLE_SHIFT) |
				(BCHP_VFD_0_FEEDER_CNTL_PIXEL_SATURATION_ENABLE_OFF << BCHP_VFD_0_FEEDER_CNTL_PIXEL_SATURATION_ENABLE_SHIFT) |
				(BCHP_VFD_0_FEEDER_CNTL_BVB_VIDEO_MODE_2D << BCHP_VFD_0_FEEDER_CNTL_BVB_VIDEO_SHIFT) |
				(BCHP_VFD_0_FEEDER_CNTL_MEM_VIDEO_MODE_2D << BCHP_VFD_0_FEEDER_CNTL_MEM_VIDEO_SHIFT);

	ulRegOffset = BTST_P_SHMOO_VFD_GET_REG_IDX(VFD_0_FEEDER_CNTL);
	BTST_REG_WRITE(ulVfdRegStart + ulRegOffset, ulRegData);

	ulRegData = 35 << BCHP_VFD_0_FIXED_COLOUR_LUMA_SHIFT;
	ulRegData |= 212 << BCHP_VFD_0_FIXED_COLOUR_CB_SHIFT;
	ulRegData |= 114 << BCHP_VFD_0_FIXED_COLOUR_CR_SHIFT;
	ulRegOffset = BTST_P_SHMOO_VFD_GET_REG_IDX(VFD_0_FIXED_COLOUR);
	BTST_REG_WRITE(ulVfdRegStart + ulRegOffset, ulRegData);


	ulRegData = (BCHP_VFD_0_LAC_CNTL_OUTPUT_TYPE_PROGRESSIVE << BCHP_VFD_0_LAC_CNTL_OUTPUT_TYPE_SHIFT) |
				(BCHP_VFD_0_LAC_CNTL_OUTPUT_FIELD_POLARITY_TOP << BCHP_VFD_0_LAC_CNTL_OUTPUT_FIELD_POLARITY_SHIFT);
	ulRegOffset = BTST_P_SHMOO_VFD_GET_REG_IDX(VFD_0_LAC_CNTL);
	BTST_REG_WRITE(ulVfdRegStart + ulRegOffset, ulRegData);

	ulRegData = (((uint32_t)(ulStride) + (32) - 1) & ~((32) - 1)); /* 32-bit aligned */
	ulRegOffset = BTST_P_SHMOO_VFD_GET_REG_IDX(VFD_0_STRIDE);
	BTST_REG_WRITE(ulVfdRegStart + ulRegOffset, ulRegData);

	ulRegData = (uint32_t)ulOffset /*+ ulByteOffset*/ + ulStride;
	ulRegOffset = BTST_P_SHMOO_VFD_GET_REG_IDX(VFD_0_PICTURE0_LINE_ADDR_0);
	BTST_REG_WRITE(ulVfdRegStart + ulRegOffset, ulRegData);

	ulRegOffset = BTST_P_SHMOO_VFD_GET_REG_IDX(VFD_0_DATA_MODE);
	BTST_REG_WRITE(ulVfdRegStart + ulRegOffset, 0x0);

	ulRegOffset = BTST_P_SHMOO_VFD_GET_REG_IDX(VFD_0_PIC_OFFSET);
	BTST_REG_WRITE(ulVfdRegStart + ulRegOffset, 0x0);

	ulRegOffset = BTST_P_SHMOO_VFD_GET_REG_IDX(VFD_0_BYTE_ORDER);
	BTST_REG_WRITE(ulVfdRegStart + ulRegOffset, ulByteOrder);

	return err;
}

static void BTST_P_SetCrcControl
	( uint32_t ulPhase,
	  uint32_t ulHeight,
	  uint32_t ulWidth )
{
	uint32_t ulRegData;

	/* generate pulse */
	ulRegData = BTST_REG_READ(BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_3);
	ulRegData |= (1 << BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_3_POST_DIVIDER_LOAD_EN_CH3_SHIFT);
	BTST_REG_WRITE(BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_3, ulRegData);

	ulRegData = BTST_REG_READ(BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_3);
	ulRegData &= ~(1 << BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_3_POST_DIVIDER_LOAD_EN_CH3_SHIFT);
	BTST_REG_WRITE(BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_3, ulRegData);

	/* select phase */
	ulRegData &= ~(0x7 << BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_3_PHASE_OFFSET_CH3_SHIFT);
	ulRegData |= (ulPhase << BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_3_PHASE_OFFSET_CH3_SHIFT);
	BTST_REG_WRITE(BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_3, ulRegData);

	/* generate pulse */
	ulRegData = BTST_REG_READ(BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_3);
	ulRegData |= (1 << BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_3_POST_DIVIDER_LOAD_EN_CH3_SHIFT);
	BTST_REG_WRITE(BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_3, ulRegData);

	ulRegData = BTST_REG_READ(BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_3);
	ulRegData &= ~(1 << BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_3_POST_DIVIDER_LOAD_EN_CH3_SHIFT);
	BTST_REG_WRITE(BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_3, ulRegData);

	/* setup VNET_B drain */
	BTST_REG_WRITE(BCHP_VNET_B_DRAIN_0_SRC, BCHP_VNET_B_DRAIN_0_SRC_SOURCE_Free_Ch_0);
	ulRegData = BTST_REG_READ(BCHP_VNET_B_DRAIN_0_DEBUG_CTRL);
	ulRegData |= (ulHeight << BCHP_VNET_B_DRAIN_0_DEBUG_CTRL_EXP_PIC_YSIZE_SHIFT);
	ulRegData |= (ulWidth << BCHP_VNET_B_DRAIN_0_DEBUG_CTRL_EXP_PIC_XSIZE_SHIFT);
	BTST_REG_WRITE(BCHP_VNET_B_DRAIN_0_DEBUG_CTRL, ulRegData);

	/* Setup CRC control for 1 picture only. Clear CRC initial value. */
	BTST_REG_WRITE(BCHP_VNET_B_CRC_CTRL, 0x0);
	ulRegData = BTST_REG_READ(BCHP_VNET_B_CRC_CTRL);
	ulRegData |= 1;
	ulRegData |= (BCHP_VNET_B_CRC_CTRL_PROBE_RATE_ONE_PICTURE_PERIOD << BCHP_VNET_B_CRC_CTRL_PROBE_RATE_SHIFT);
	ulRegData |= (BCHP_VNET_B_CRC_CTRL_CLEAR_CLEAR << BCHP_VNET_B_CRC_CTRL_CLEAR_SHIFT);
	BTST_REG_WRITE(BCHP_VNET_B_CRC_CTRL, ulRegData);

#if 0
	/* Set normal operation */
	ulRegData &= ~(BCHP_VNET_B_CRC_CTRL_CLEAR_NORMAL << BCHP_VNET_B_CRC_CTRL_CLEAR_SHIFT);
	BTST_REG_WRITE(BCHP_VNET_B_CRC_CTRL, ulRegData);
#endif


}


static BERR_Code BTST_P_StartFeed
	( uint8_t                          uiVfdId,
	  uint32_t                         ulHeight,
	  uint32_t                         ulWidth )
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t ulVfdRegStart, ulVfdRegEnd;
	uint32_t ulRegOffset;
	uint32_t ulRegData;
	bool bFinishedFeed = false, bDrainedPicture = false;
	uint32_t ulTimeout = 0;

	switch (uiVfdId)
	{
		case BCHP_VNET_F_FCH_0_SRC_SOURCE_Video_Feeder_0:
			ulVfdRegStart = BCHP_VFD_0_REG_START;
			ulVfdRegEnd = BCHP_VFD_0_REG_END;
			break;
		case BCHP_VNET_F_FCH_0_SRC_SOURCE_Video_Feeder_1:
			ulVfdRegStart = BCHP_VFD_1_REG_START;
			ulVfdRegEnd = BCHP_VFD_1_REG_END;
			break;
		case BCHP_VNET_F_FCH_0_SRC_SOURCE_Video_Feeder_2:
			ulVfdRegStart = BCHP_VFD_2_REG_START;
			ulVfdRegEnd = BCHP_VFD_2_REG_END;
			break;
		case BCHP_VNET_F_FCH_0_SRC_SOURCE_Video_Feeder_3:
			ulVfdRegStart = BCHP_VFD_3_REG_START;
			ulVfdRegEnd = BCHP_VFD_3_REG_END;
			break;
#if BCHP_VFD_4_REG_START
		case BCHP_VNET_F_FCH_0_SRC_SOURCE_Video_Feeder_4:
			ulVfdRegStart = BCHP_VFD_4_REG_START;
			ulVfdRegEnd = BCHP_VFD_4_REG_END;
			break;
#endif
		default:
			err = BERR_INVALID_PARAMETER;
			return err;
	}

	/* set the width */
	ulRegOffset = BTST_P_SHMOO_VFD_GET_REG_IDX(VFD_0_PIC_FEED_CMD);
	ulRegData = 1;
	BTST_REG_WRITE(ulVfdRegStart + ulRegOffset, ulRegData);

	ulTimeout = 0;

	/* poll for completed feed */
	while (!bFinishedFeed)
	{
		ulRegData = BTST_REG_READ(ulVfdRegStart + ulRegOffset);
		if (ulRegData == 0)
			bFinishedFeed = true;
		BKNI_Sleep(1);
		ulTimeout++;
		if (ulTimeout > 100)
		{
			err = BERR_TIMEOUT;
			return err;
		}
	}

	/* setup VNET_B drain */
	BTST_REG_WRITE(BCHP_VNET_B_DRAIN_0_SRC, BCHP_VNET_B_DRAIN_0_SRC_SOURCE_Free_Ch_0);
	ulRegData = BTST_REG_READ(BCHP_VNET_B_DRAIN_0_DEBUG_CTRL);
	ulRegData |= (ulHeight << BCHP_VNET_B_DRAIN_0_DEBUG_CTRL_EXP_PIC_YSIZE_SHIFT);
	ulRegData |= (ulWidth << BCHP_VNET_B_DRAIN_0_DEBUG_CTRL_EXP_PIC_XSIZE_SHIFT);
	BTST_REG_WRITE(BCHP_VNET_B_DRAIN_0_DEBUG_CTRL, ulRegData);

	ulTimeout = 0;

	/* poll for completed feed */
	while (!bDrainedPicture)
	{
		/* read picture count and must match surface dimension to proceed */
		ulRegData = BTST_REG_READ(BCHP_VNET_B_DRAIN_0_PIC_SIZE_CNT);

		if (((ulRegData >> BCHP_VNET_B_DRAIN_0_PIC_SIZE_CNT_YCOUNT_SHIFT) >= ulHeight) &&
			((ulRegData >> BCHP_VNET_B_DRAIN_0_PIC_SIZE_CNT_XCOUNT_SHIFT) >= ulWidth))
		{
			ulRegData = BTST_REG_READ(BCHP_VNET_B_DRAIN_0_ERR_STATUS);
			if (((ulRegData >> BCHP_VNET_B_DRAIN_0_ERR_STATUS_PIC_HEIGHT_ERROR_SHIFT) != 0) ||
				((ulRegData >> BCHP_VNET_B_DRAIN_0_ERR_STATUS_PIC_WIDTH_ERROR_SHIFT) != 0))
			{
				BDBG_ERR(("Error in draining picture."));
			}
			else
			{
				bDrainedPicture = true;
			}
		}

		BKNI_Sleep(1);

		ulTimeout++;
		if (ulTimeout > 100)
		{
			err = BERR_TIMEOUT;
			return err;
		}
	}

	return err;
}

BERR_Code BTST_VideoShmoo
	( const BMEM_Heap_Handle           hMemory,
	  const BCHP_Handle                hChip,
	  const BREG_Handle                hRegister )
{
	BERR_Code err = BERR_SUCCESS;

	void         *pvAddress;
	uint32_t      ulOffset;
	uint32_t      ulStride;
	uint32_t      ulWidth  = BTST_P_VIDEO_SHMOO_SURFACE_WIDTH;
	uint32_t      ulHeight = BTST_P_VIDEO_SHMOO_SURFACE_HEIGHT;
	uint32_t      ulMemorySize;
	unsigned int  uiPitch = 0;

	uint32_t      ulPixelFormat = 0xa728888;
	uint32_t      ulGoldenLuma = 0x4435;
	uint32_t      ulGoldenChroma = 0x37a1;

	uint8_t       uiVfdId;
	uint32_t      ulLumaData, ulChromaData;
	uint8_t       auiPhase[8] = {1, 1, 1, 1, 1, 1, 1, 1};
	bool          bFoundPhase = false;
	uint32_t      ulPhase = 0;
	uint32_t      ulLocalHeight;

#if BTST_P_CAPTURE_CRC_PICTURE
	void         *pvCaptureAddress;
	uint32_t      ulCaptureOffset;
	uint8_t      *puiCapStartAddr;
	uint32_t      ulCapAddr;
	uint32_t      ii, jj;
#endif

	BDBG_ASSERT(hRegister);
	BDBG_ASSERT(hMemory);

#ifdef BCHP_PWR_RESOURCE_BVN
	BDBG_ASSERT(hChip);
	BCHP_PWR_AcquireResource(hChip, BCHP_PWR_RESOURCE_BVN);
#else
	BSTD_UNUSED(hChip);
#endif

	/* create surface */
	BTST_P_GetBytesPerNPixels( 0xa728888, ulWidth, &uiPitch );
	ulStride = (uint32_t) uiPitch;

	ulMemorySize = ulStride * ulHeight;

	pvAddress = BMEM_AllocAligned(hMemory, ulMemorySize, 2, 0);
	if( pvAddress == NULL )
	{
		return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
	}

	err = BMEM_ConvertAddressToOffset(hMemory, pvAddress, &ulOffset);
	if (err != BERR_SUCCESS)
	{
		return err;
	}

	/* allocate memory and create luma ramp test pattern */
	err = BTST_P_MakeShmooSurface((uint8_t *)pvAddress, 0xa728888,
								ulStride, ulWidth, ulHeight);
	if (err != BERR_SUCCESS)
	{
		return err;
	}

#if BTST_P_CAPTURE_CRC_PICTURE
	{
		pvCaptureAddress = BMEM_AllocAligned(hMemory, ulMemorySize, 2, 0);
		if( pvCaptureAddress == NULL )
		{
			return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		}

		puiCapStartAddr = (uint8_t *)pvCaptureAddress;
		ulCapAddr = (uint32_t)puiCapStartAddr;

		BKNI_Memset(pvCaptureAddress, 0, ulMemorySize);

		err = BMEM_ConvertAddressToOffset(hMemory, pvCaptureAddress, &ulCaptureOffset);
		if (err != BERR_SUCCESS)
		{
			return err;
		}
	}
#endif

	g_hRegister = hRegister;

	BTST_P_ResetBvn();

	/* check alignment */
	BDBG_ASSERT(BTST_P_IS_ALIGN(pvAddress, 256));

	/* work around CRC/code change issue */
	ulLocalHeight = ulHeight - 1;

	/* cycle through all phases for CLKGEN PLL 3 (SCB clock) until a working phase is found */
	for (ulPhase=0; ulPhase<8 && !bFoundPhase; ulPhase++)
	{
#if BTST_P_CAPTURE_CRC_PICTURE
		BTST_P_SetupCap(ulWidth, ulLocalHeight, ulStride, ulCaptureOffset);

		for (uiVfdId = BCHP_VNET_F_FCH_0_SRC_SOURCE_Video_Feeder_0;
			uiVfdId < BCHP_VNET_F_FCH_0_SRC_SOURCE_Video_Feeder_1; uiVfdId++)
#else

		for (uiVfdId = BCHP_VNET_F_FCH_0_SRC_SOURCE_Video_Feeder_0;
#if BCHP_VFD_4_REG_START
			uiVfdId <= BCHP_VNET_F_FCH_0_SRC_SOURCE_Video_Feeder_4;
#else
			uiVfdId <= BCHP_VNET_F_FCH_0_SRC_SOURCE_Video_Feeder_3;
#endif
            uiVfdId++)
#endif
		{
			BTST_P_SetupVfd(uiVfdId, ulStride, ulWidth, ulLocalHeight, ulPixelFormat, ulOffset);

			/* establish VFD_N-to-XBAR-CRC path */
			BTST_REG_WRITE(BCHP_VNET_F_FCH_0_SRC, uiVfdId);
			BTST_REG_WRITE(BCHP_VNET_B_CRC_SRC, BCHP_VNET_B_CRC_SRC_SOURCE_Free_Ch_0);

			/* setup CRC drain */
			BTST_P_SetCrcControl(ulPhase, ulLocalHeight, ulWidth);

			/* enable path and CRC */
			err = BTST_P_StartFeed(uiVfdId, ulLocalHeight, ulWidth);
			if (err != BERR_SUCCESS)
			{
				return err;
			}

			/* read CRC Luma and Chroma status */
			ulLumaData = BTST_REG_READ(BCHP_VNET_B_CRC_Y_STATUS);
			ulChromaData = BTST_REG_READ(BCHP_VNET_B_CRC_C_STATUS);

			BDBG_MSG(("VFD %d: Phase %d Y/C CRC: Y = [0x%x], C = [0x%x]",
				uiVfdId-2, ulPhase, ulLumaData, ulChromaData));

			/* compare to golden CRC */
			if (ulLumaData == ulGoldenLuma &&
				ulChromaData == ulGoldenChroma)
			{
				/* found a candidate phase */
				auiPhase[ulPhase] &= 1;
				BDBG_MSG(("Phase %d test: VFD %d Y/C CRC match passed.", ulPhase, uiVfdId-2));
			}
			else
			{
				auiPhase[ulPhase] &= 0;
				BDBG_MSG(("Phase %d test: VFD %d Y/C CRC match failed.", ulPhase, uiVfdId-2));
			}
		}

		if (auiPhase[ulPhase] == 1)
		{
			bFoundPhase = true;
		}
	}

#if BTST_P_CAPTURE_CRC_PICTURE
	/* fill in each line of the frame */
	for( ii = 0; ii < ulLocalHeight; ii++ )
	{
		/* restore start addr */
		puiCapStartAddr = (uint8_t *)ulCapAddr;

		/* select start row */
		puiCapStartAddr += ii * ulStride;

		/* for each pixel of the line */
		for( jj = 0; jj < ulWidth; jj++ )
		{
			if( jj & 1 )
			{
				printf("pixel [%dx%d] : Y[0x%x]\n", ii, jj,
					puiCapStartAddr[(jj-1) * 2 + BTST_PXL_COMPONENT_POS(0xa728888, 2) / 8]);
			}
			else
			{
				printf("pixel [%dx%d] : YCbCr[0x%x]\n", ii, jj,
					((uint32_t *) puiCapStartAddr) [jj / 2]);
			}
		}
	}/* end of frame index for loop */

	BMEM_Free( hMemory, pvCaptureAddress );
#endif

	if (!bFoundPhase)
	{
		BDBG_ERR(("There is no working clock phase for 277MHz DRAM. Replace the chip if you plan to use 277 Mhz DRAM; otherwise, it is usable with 216MHz DRAM."));
		err = BERR_UNKNOWN;
	}
	else
	{
		BDBG_MSG(("Phase %d is a usable clock phase for 277MHz DRAM.", ulPhase-1));
		err = BERR_SUCCESS;
	}

	BMEM_Free( hMemory, pvAddress );

#ifdef BCHP_PWR_RESOURCE_BVN
	BCHP_PWR_ReleaseResource(hChip, BCHP_PWR_RESOURCE_BVN);
#endif

	return err;
}

/* End of file */

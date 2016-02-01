/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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

#ifndef BSUR_PRIVATE_H__
#define BSUR_PRIVATE_H__

#ifdef __cplusplus
extern "C" {
#endif

BDBG_OBJECT_ID_DECLARE(BSUR_Palette);
BDBG_OBJECT_ID_DECLARE(BSUR_Surface);

/***************************************************************************
Description:
	This is the private palette structure.
****************************************************************************/
typedef struct BSUR_P_Palette_Handle
{
	BDBG_OBJECT(BSUR_Palette)
	BMEM_Handle hMem;                /* handle to BMEM module */
	void *pvAddress;                 /* palette address */
	void *pvCached;                  /* cached palette address */
	uint32_t ulNumEntries;           /* number of palette entries */
	uint32_t ulConstraintMask;       /* constraint mask */
	BPXL_Format eFormat;             /* format of palette */
	bool bDriverOwned;               /* memory allocated by driver */
}
BSUR_P_Palette_Handle;

/***************************************************************************
Description:
	This is the private surface structure.
****************************************************************************/
typedef struct BSUR_P_Surface_Handle
{
	BDBG_OBJECT(BSUR_Surface)
	BSUR_Surface_Settings stSettings; /* extended settings */
	BMEM_Handle hMem;                 /* handle to BMEM module */
	void *pvAddress;                  /* surface address */
	void *pvCached;                   /* cached surface address */
	uint32_t ulWidth;                 /* surface width */
	uint32_t ulHeight;                /* surface height */
	uint32_t ulPitch;                 /* surface pitch */
	uint32_t ulConstraintMask;        /* constraint mask */
	uint32_t ulID;                    /* surface ID */
	BPXL_Format eFormat;              /* format of palette */
	BSUR_Palette_Handle hPalette;     /* pointer to attached palette */
	bool bDriverOwned;                /* memory allocated by driver */
}
BSUR_P_Surface_Handle;

/***************************************************************************
Description:
	Gets format's alignment mask.

Input:
	format - Pixel format.
****************************************************************************/
#define BSUR_P_ALIGN_MASK( format ) \
	((BPXL_BITS_PER_PIXEL(format) > 8) ? \
	(((BPXL_BITS_PER_PIXEL(format) / 8) & 1) ? 0 : \
	((BPXL_BITS_PER_PIXEL(format) / 8) - 1)) : 0)

/***************************************************************************
Description:
	Determines if value is unaligned to size of pixel in given format.

Input:
	format - Pixel format.
	value  - Value to check.
****************************************************************************/
#define BSUR_P_PIXEL_UNALIGNED( format, value ) \
	(((unsigned long) (value)) & BSUR_P_ALIGN_MASK( format ))

/***************************************************************************
Description:
	Determines if palette address is unaligned.

Input:
	value  - Value to check.
****************************************************************************/
#define BSUR_P_PALETTE_ALIGNMENT   5
#define BSUR_P_PALETTE_UNALIGNED( value ) \
	(((unsigned long) (value)) & ((1 << BSUR_P_PALETTE_ALIGNMENT) - 1))

/***************************************************************************
Description:
	Determines if value is a power of 2.

Input:
	value  - Value to check.
	valid  - Returned bool (true if power of 2).
****************************************************************************/
#define BSUR_P_POWER_OF_2( value, valid ) \
{ \
	uint32_t ulSize = (uint32_t) (value); \
	uint32_t ulCount = 0; \
	while( ulSize > 1 ) \
	{ \
		ulSize /= 2; \
		ulCount++; \
	} \
	while( ulCount ) \
	{ \
		ulSize *= 2; \
		ulCount--; \
	} \
	(valid) = (ulSize == (value)) ? true : false; \
}

/***************************************************************************
Description:
	Determines if YCbCr420 format is unaligned. (eg. BPXL_eY8, BPXL_eCb8_Cr8)

Input:
	value  - Value to check.
****************************************************************************/
#define BSUR_P_YCbCr420_ALIGNMENT   6
#define BSUR_P_YCbCr420_UNALIGNED( value ) \
	(((unsigned long) (value)) & ((1 << BSUR_P_YCbCr420_ALIGNMENT) - 1))

/***************************************************************************
Description:
	Determines if YCbCr422 10-bit format is unaligned.
	(eg. BPXL_eX2_Y010_Cb10_Y110_X2_Cr10_Y010_Cb10_X2_Y110_Cr10_Y010_X2_Cb10_Y110_Cr10)

Input:
	value  - Value to check.
****************************************************************************/
#define BSUR_P_YCbCr422_10BIT_ALIGNMENT   4
#define BSUR_P_YCbCr422_10BIT_UNALIGNED( value ) \
	(((unsigned long) (value)) & ((1 << BSUR_P_YCbCr422_10BIT_ALIGNMENT) - 1))

/***************************************************************************
Description:
	Determines if YCbCr422 10-bit packed format is unaligned.
	(eg. BPXL_eCr10_Y110_Cb10_Y010)

Input:
	value  - Value to check.
****************************************************************************/
#define BSUR_P_YCbCr422_10BIT_PACKED_ALIGNMENT   5
#define BSUR_P_YCbCr422_10BIT_PACKED_UNALIGNED( value ) \
	(((unsigned long) (value)) & ((1 << BSUR_P_YCbCr422_10BIT_PACKED_ALIGNMENT) - 1))

/***************************************************************************
Description:
	Determines if DCX TestFeature1 format is unaligned.

Input:
	value  - Value to check.
****************************************************************************/
#define BSUR_P_DCX_TESTFEATURE1_ALIGNMENT   6
#define BSUR_P_DCX_TESTFEATURE1_UNALIGNED( value ) \
	(((unsigned long) (value)) & ((1 << BSUR_P_DCX_TESTFEATURE1_ALIGNMENT) - 1))

#define BSUR_P_DCX_TESTFEATURE1_BITSPERPIXEL_MINIMUM   14
#define BSUR_P_DCX_TESTFEATURE1_BITSPERPIXEL_MAXIMUM   64
#define BSUR_P_DCX_TESTFEATURE1_BITSPERPIXEL_DEFAULT   BSUR_P_DCX_TESTFEATURE1_BITSPERPIXEL_MINIMUM

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BSUR_PRIVATE_H__ */

/* end of file */

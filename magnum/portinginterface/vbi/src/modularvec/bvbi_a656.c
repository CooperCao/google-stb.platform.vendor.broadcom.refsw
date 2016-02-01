
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
 * This module provides access to the ANCI_656 cores for the VBI porting
 * interface (BVBI).  This module is private to BVBI.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bstd.h"			/* standard types */
#include "bdbg.h"			/* Dbglib */
#include "bvbi.h"			/* VBI processing, this module. */
#include "bvbi_cap.h"
#include "bvbi_priv.h"		/* VBI internal data structures */

#if (BVBI_NUM_ANCI656_656 > 0) /** { **/

#include "bchp_anci656_656.h"	/* RDB info for ANCI_656_656 registers */

BDBG_MODULE(BVBI);

/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/
static void BVBI_P_A656_Enc_Init (BREG_Handle hReg, uint32_t ulCoreOffset);
static uint32_t P_GetCoreOffset_isr (uint8_t hwCoreIndex);


/***************************************************************************
* Implementation of "BVBI_" API functions
***************************************************************************/


/***************************************************************************
* Implementation of supporting VBI_ENC functions that are not in API
***************************************************************************/


BERR_Code BVBI_P_A656_Init( BVBI_P_Handle *pVbi )
{
	uint32_t ulCoreOffset;

	BDBG_ENTER(BVBI_P_A656_Init);

	/* Initialize ANCI656 cores. At this point, only one core is supported. */
	ulCoreOffset = 0x0;
	BVBI_P_A656_Enc_Init (pVbi->hReg, ulCoreOffset);

	BDBG_LEAVE(BVBI_P_A656_Init);
	return BERR_SUCCESS;
}


BERR_Code BVBI_P_A656_Enc_Program (
	BREG_Handle hReg,
	uint8_t hwCoreIndex,
	BVBI_P_Encode_656_Options* h656options,
	bool bPR18010_bad_line_number,
	BFMT_VideoFmt eVideoFormat)
{
/*
	Programming note: the implementation here assumes that the bitfield layout
	within registers is the same for all ANCI656 cores in the chip.

	If a chip is built that has multiple ANCI656 encoder cores that are not
	identical, then this routine will have to be redesigned.
*/
	uint32_t ulCoreOffset;
	uint32_t ulReg;
	uint8_t sdid;
	uint8_t did;
	uint8_t dcountCC;
	uint8_t dcountTT;
	uint8_t dcountWSS;
	bool    isPal;

	BDBG_ENTER(BVBI_P_A656_Enc_Program);

	/* Figure out which encoder core to use */
	ulCoreOffset = P_GetCoreOffset_isr (hwCoreIndex);
	if (ulCoreOffset == 0xFFFFFFFF)
	{
		/* This should never happen!  This parameter was checked by
		   BVBI_Encode_Create() */
		BDBG_LEAVE(BVBI_P_A656_Enc_Program);
		return BERR_TRACE (BERR_INVALID_PARAMETER);
	}

	/* Determine whether PAL or NTSC.  The hardware can only handle straight
	   PAL or NTSC, so some of the video standards are "don't cares." */
	/* Select video format */
	switch (eVideoFormat)
	{
    case BFMT_VideoFmt_eNTSC:
    case BFMT_VideoFmt_eNTSC_J:
	case BFMT_VideoFmt_e720x482_NTSC:
	case BFMT_VideoFmt_e720x482_NTSC_J:
	case BFMT_VideoFmt_e1080i:
	case BFMT_VideoFmt_e720p:
	case BFMT_VideoFmt_e480p:
	case BFMT_VideoFmt_e720x483p:
		isPal = false;
		break;

    case BFMT_VideoFmt_ePAL_B:
    case BFMT_VideoFmt_ePAL_B1:
    case BFMT_VideoFmt_ePAL_D:
    case BFMT_VideoFmt_ePAL_D1:
    case BFMT_VideoFmt_ePAL_G:
    case BFMT_VideoFmt_ePAL_H:
    case BFMT_VideoFmt_ePAL_K:
    case BFMT_VideoFmt_ePAL_I:
    case BFMT_VideoFmt_ePAL_M:
    case BFMT_VideoFmt_ePAL_N:
    case BFMT_VideoFmt_ePAL_NC:
    case BFMT_VideoFmt_eSECAM_L:
    case BFMT_VideoFmt_eSECAM_B:
    case BFMT_VideoFmt_eSECAM_G:
    case BFMT_VideoFmt_eSECAM_D:
    case BFMT_VideoFmt_eSECAM_K:
    case BFMT_VideoFmt_eSECAM_H:
	case BFMT_VideoFmt_e1080i_50Hz:
	case BFMT_VideoFmt_e720p_50Hz:
    case BFMT_VideoFmt_e576p_50Hz:
		isPal = true;
		break;

	default:
		BDBG_LEAVE(BVBI_P_A656_Enc_Program);
		BDBG_ERR(("This video format not supported yet"));
		return BERR_TRACE (BERR_NOT_SUPPORTED);
		break;
	}

	/* Figure out the data ID and length fields for ancillary data packets */
	/* TODO: some of this is likely wrong, especially SAA7113. */
	switch (h656options->e656Format)
	{
	case BVBI_656Fmt_SAA7113:
	case BVBI_656Fmt_Modified_SAA7113:
		dcountCC = (isPal ? 1 : 5);
		dcountTT = (isPal ? 8 : 12);
		dcountWSS = 3;
		break;
		break;
	case BVBI_656Fmt_SMPTE291:
		dcountCC = 1;
		dcountTT = (isPal ? 21 : 17);
		dcountWSS = 2;
		break;
	default:
		BDBG_LEAVE(BVBI_P_A656_Enc_Program);
		BDBG_ERR(("This ITU-R 656 encoding format not supported"));
		return BERR_TRACE (BERR_INVALID_PARAMETER);
		break;
	}
	sdid = BVBI_P_p656_SetEEbits (h656options->sdid);

	/* Program the "type A" register for closed caption */
	if (h656options->e656Format != BVBI_656Fmt_SMPTE291)
		dcountCC = BVBI_P_p656_SetEEbits (dcountCC);
	ulReg = (
		BCHP_FIELD_DATA(ANCI656_656_TYPE_A_VBI_HEADER,        SDID,     sdid) |
		BCHP_FIELD_DATA(ANCI656_656_TYPE_A_VBI_HEADER, DWORD_COUNT, dcountCC)
	);
	if (isPal)
	{
		did = BVBI_P_p656_SetEEbits (
			BCHP_ANCI656_656_TYPE_A_VBI_HEADER_TYPE_EURO_CLOSED_CAPTION);
	}
	else
	{
		did = BVBI_P_p656_SetEEbits (
			BCHP_ANCI656_656_TYPE_A_VBI_HEADER_TYPE_US_CLOSED_CAPTION);
	}
	ulReg |=
		BCHP_FIELD_DATA (ANCI656_656_TYPE_A_VBI_HEADER, TYPE, did);
    BREG_Write32 (
		hReg, BCHP_ANCI656_656_TYPE_A_VBI_HEADER + ulCoreOffset, ulReg);

	/* Program the "type B" register for teletext */
	dcountTT = BVBI_P_p656_SetEEbits (dcountTT);
	ulReg = (
		BCHP_FIELD_DATA(ANCI656_656_TYPE_B_VBI_HEADER,        SDID,     sdid) |
		BCHP_FIELD_DATA(ANCI656_656_TYPE_B_VBI_HEADER, DWORD_COUNT, dcountTT)
	);
	if (isPal)
	{
		did = BVBI_P_p656_SetEEbits (
			BCHP_ANCI656_656_TYPE_B_VBI_HEADER_TYPE_TELETEXT);
	}
	else
	{
		did = BVBI_P_p656_SetEEbits (
			BCHP_ANCI656_656_TYPE_B_VBI_HEADER_TYPE_US_NABTS);
	}
	ulReg |=
		BCHP_FIELD_DATA (ANCI656_656_TYPE_B_VBI_HEADER, TYPE, did);
    BREG_Write32 (
		hReg, BCHP_ANCI656_656_TYPE_B_VBI_HEADER + ulCoreOffset, ulReg);

	/* Program the "type C" register for WSS */
	dcountWSS = BVBI_P_p656_SetEEbits (dcountWSS);
	did = BVBI_P_p656_SetEEbits (
		BCHP_ANCI656_656_TYPE_C_VBI_HEADER_TYPE_WIDE_SCREEN);
	ulReg = (
		BCHP_FIELD_DATA(ANCI656_656_TYPE_C_VBI_HEADER,        SDID,      sdid)|
		BCHP_FIELD_DATA(ANCI656_656_TYPE_C_VBI_HEADER, DWORD_COUNT, dcountWSS)
	);
	ulReg |=
		BCHP_FIELD_DATA (ANCI656_656_TYPE_C_VBI_HEADER, TYPE, did);
    BREG_Write32 (
		hReg, BCHP_ANCI656_656_TYPE_C_VBI_HEADER + ulCoreOffset, ulReg);

	/* Program the ITU-R 656 data encoding format */
	ulReg =
		BREG_Read32 ( hReg,  BCHP_ANCI656_656_ANCI656_CNTRL + ulCoreOffset );
	ulReg &=
		 ~BCHP_MASK      (ANCI656_656_ANCI656_CNTRL, FORMAT         );
	switch (h656options->e656Format)
	{
	case BVBI_656Fmt_SAA7113:
		ulReg |=
			BCHP_FIELD_ENUM(ANCI656_656_ANCI656_CNTRL, FORMAT, SAA7113H);
		break;
	case BVBI_656Fmt_Modified_SAA7113:
		ulReg |=
			BCHP_FIELD_ENUM(ANCI656_656_ANCI656_CNTRL, FORMAT,
				MODIFIED_SAA7113H);
		break;
	case BVBI_656Fmt_SMPTE291:
		ulReg |=
			BCHP_FIELD_ENUM(ANCI656_656_ANCI656_CNTRL, FORMAT, SMPTE291M);
		break;
	default:
		BDBG_LEAVE(BVBI_P_A656_Enc_Program);
		BDBG_ERR(("This ITU-R 656 encoding format not supported"));
		return BERR_TRACE (BERR_INVALID_PARAMETER);
		break;
	}

	/* Take care of the PR18010 backwards compatibility issue */
	if (bPR18010_bad_line_number)
	{
		ulReg |= (
			BCHP_FIELD_ENUM(ANCI656_656_ANCI656_CNTRL, V_BIT_SEL, REG) |
			BCHP_FIELD_ENUM(ANCI656_656_ANCI656_CNTRL, F_BIT_SEL, REG)
		);
	}

    BREG_Write32 (hReg, BCHP_ANCI656_656_ANCI656_CNTRL + ulCoreOffset, ulReg);

	BDBG_LEAVE(BVBI_P_A656_Enc_Program);
	return BERR_SUCCESS;
}

/***************************************************************************
* Static (private) functions
***************************************************************************/

/***************************************************************************
 *
 */
static void BVBI_P_A656_Enc_Init (BREG_Handle hReg, uint32_t ulCoreOffset)
{
	BDBG_ENTER(BVBI_P_A656_Enc_Init);

	BSTD_UNUSED(ulCoreOffset);

	/* Just reset the core.  Probably not necessary. */
	BVBI_P_VIE_AncilSoftReset (hReg, 0);

	BDBG_LEAVE(BVBI_P_A656_Enc_Init);
}

/***************************************************************************
 *
 */
static uint32_t P_GetCoreOffset_isr (uint8_t hwCoreIndex)
{
	uint32_t ulCoreOffset = 0xFFFFFFFF;

	if (hwCoreIndex == 0)
		ulCoreOffset = 0;

	return ulCoreOffset;
}

#endif /** } (BVBI_NUM_ANCI656_656 > 0) **/

/* End of file */

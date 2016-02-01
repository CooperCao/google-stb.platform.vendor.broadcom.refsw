/***************************************************************************
 *     Copyright (c) 2003-2009, Broadcom Corporation
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

#include "bstd.h"           /* standard types */
#include "bdbg.h"           /* Dbglib */
#include "bkni.h"			/* For critical sections */
#include "bvbi.h"           /* VBI processing, this module. */
#include "bvbi_cap.h"
#include "bvbi_priv.h"      /* VBI internal data structures */
#if (BVBI_NUM_WSE >= 1)
#include "bchp_wse_prim.h"  /* RDB info for primary WSE core */
#endif
#if (BVBI_NUM_WSE >= 2)
#include "bchp_wse_sec.h"   /* RDB info for secondary WSE core */
#endif
#if (BVBI_NUM_WSE >= 3)
#include "bchp_wse_tert.h"  /* RDB info for tertiary WSE core */
#endif

BDBG_MODULE(BVBI);


/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/
static uint32_t P_GetCoreOffset_isr (uint8_t hwCoreIndex);


/***************************************************************************
* Implementation of supporting VPS functions that are not in API
***************************************************************************/


/***************************************************************************
 *
 */
void BVBI_P_VPS_Enc_Init (BREG_Handle hReg, uint8_t hwCoreIndex)
{
	uint32_t ulCoreOffset;
	uint32_t ulControlReg;
	uint32_t ulActiveLine;

	BDBG_ENTER(BVBI_P_VPS_Enc_Init);

	/* No reset here.  There is a reset in BVBI_P_WSS_Enc_Init(),though. */

	/* Determine which core to access */
	ulCoreOffset = P_GetCoreOffset_isr (hwCoreIndex);
	if (ulCoreOffset == 0xFFFFFFFF)
	{
		/* This should never happen!  This parameter was checked by
		   BVBI_Encode_Create() */
		BDBG_ASSERT (0);
	}

	ulActiveLine = 16 - 1;

	BKNI_EnterCriticalSection();

	/* Program the control register */
	ulControlReg = BREG_Read32 (hReg, BCHP_WSE_PRIM_vps_control + ulCoreOffset);
	ulControlReg &= ~(
		BCHP_MASK       (WSE_PRIM_vps_control, vps_bit_order            ) |
		BCHP_MASK       (WSE_PRIM_vps_control, invert_data              ) |
		BCHP_MASK       (WSE_PRIM_vps_control, active_line              ) |
		BCHP_MASK       (WSE_PRIM_vps_control, start_delay              ) |
		BCHP_MASK       (WSE_PRIM_vps_control, enable                   ) );
	ulControlReg |= (
		BCHP_FIELD_ENUM (WSE_PRIM_vps_control, vps_bit_order, msb_to_lsb) |
		BCHP_FIELD_ENUM (WSE_PRIM_vps_control, invert_data,          Off) |
		BCHP_FIELD_DATA (WSE_PRIM_vps_control, active_line, ulActiveLine) |
		BCHP_FIELD_DATA (WSE_PRIM_vps_control, start_delay,            1) |
		BCHP_FIELD_DATA (WSE_PRIM_vps_control, enable,                 0) );
	BREG_Write32 (hReg, BCHP_WSE_PRIM_vps_control + ulCoreOffset, ulControlReg);

	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BVBI_P_VPS_Enc_Init);
}

BERR_Code BVBI_P_VPS_Enc_Program (
	BREG_Handle hReg,
	bool is656,
	uint8_t hwCoreIndex,
	bool bActive,
	BFMT_VideoFmt eVideoFormat)
{
/*
	Programming note: the implementation here assumes that the bitfield layout
	within registers is the same for all VPS encoder cores in the chip.

	If a chip is built that has multiple VPS encoder cores that are not
	identical, then this routine will have to be redesigned.
*/
	BERR_Code eErr;

	BDBG_ENTER(BVBI_P_VPS_Enc_Program);

	/* Take care of a special case for 656 programming */
#if (BVBI_NUM_WSE_656 >= 1)
	if (is656)
	{
		BDBG_LEAVE(BVBI_P_VPS_Enc_Program);
		if (bActive)
		{
			/* No bypass encoder for VPS */
			return BERR_TRACE(BERR_INVALID_PARAMETER);
		}
		else
		{
			return BERR_SUCCESS;
		}
	}
#endif

	/* Complain if video format is not supported */
	switch (eVideoFormat)
	{
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
    case BFMT_VideoFmt_eSECAM:
		break;

	default:
		if (bActive)
			return BERR_TRACE (BVBI_ERR_VFMT_CONFLICT);
	}

	BKNI_EnterCriticalSection();
	eErr = BERR_TRACE (BVBI_P_VPS_Encode_Enable_isr (
		hReg, is656, hwCoreIndex, BFMT_VideoFmt_ePAL_G, bActive));
	BKNI_LeaveCriticalSection();
	if (eErr != BERR_SUCCESS)
		goto done;

done:
	BDBG_LEAVE(BVBI_P_VPS_Enc_Program);
	return eErr;
}

uint32_t BVBI_P_VPS_Encode_Data_isr (
	BREG_Handle hReg,
	bool is656,
	uint8_t hwCoreIndex,
	BAVC_Polarity polarity,
	BVBI_VPSData *pVPSData)
{
	uint32_t ulCoreOffset;
	uint32_t ulWssReg;
	uint32_t ulVpsReg;
	uint32_t ulErrInfo = 0;

	BDBG_ENTER(BVBI_P_VPS_Encode_Data_isr);

	/* Size check for field data */
	if (!pVPSData)
	{
		return (BVBI_LINE_ERROR_FLDH_CONFLICT);
	}

	/* VPS is for top field only */
	if (polarity != BAVC_Polarity_eTopField)
	{
		return (BVBI_LINE_ERROR_PARITY_CONFLICT);
	}

	/* Get register offset */
	ulCoreOffset = P_GetCoreOffset_isr (hwCoreIndex);

	/* Sanity check */
	if (is656 || (ulCoreOffset == 0xFFFFFFFF))
	{
		/* This should never happen!  This parameter was checked by
		   BVBI_Encode_Create() */
		BDBG_ASSERT(0);
	}

	/* Some of the data goes to the WSS data register */
	ulWssReg = BREG_Read32 (hReg, BCHP_WSE_PRIM_wss_data + ulCoreOffset);
	ulWssReg &= ~(
		 BCHP_MASK       (WSE_PRIM_wss_data, vps_byte_5  ) |
		 BCHP_MASK       (WSE_PRIM_wss_data, vps_byte_11 ) );
	ulWssReg |= (
		 BCHP_FIELD_DATA (WSE_PRIM_wss_data, vps_byte_5,
		 	(uint32_t)pVPSData->ucByte05                 ) |
		 BCHP_FIELD_DATA (WSE_PRIM_wss_data, vps_byte_11,
		 	(uint32_t)pVPSData->ucByte11                 ) );
	BREG_Write32 (hReg, BCHP_WSE_PRIM_wss_data + ulCoreOffset, ulWssReg);

	/* The rest of the data goes right into the VPS data register */
	ulVpsReg = BREG_Read32 (hReg, BCHP_WSE_PRIM_vps_data_1 + ulCoreOffset);
	ulVpsReg &= ~(
		 BCHP_MASK       (WSE_PRIM_vps_data_1, vps_byte_12 ) |
		 BCHP_MASK       (WSE_PRIM_vps_data_1, vps_byte_13 ) |
		 BCHP_MASK       (WSE_PRIM_vps_data_1, vps_byte_14 ) |
		 BCHP_MASK       (WSE_PRIM_vps_data_1, vps_byte_15 ) );
	ulVpsReg |= (
		 BCHP_FIELD_DATA (WSE_PRIM_vps_data_1, vps_byte_12,
		 	(uint32_t)pVPSData->ucByte12                 ) |
		 BCHP_FIELD_DATA (WSE_PRIM_vps_data_1, vps_byte_13,
		 	(uint32_t)pVPSData->ucByte13                 ) |
		 BCHP_FIELD_DATA (WSE_PRIM_vps_data_1, vps_byte_14,
		 	(uint32_t)pVPSData->ucByte14                 ) |
		 BCHP_FIELD_DATA (WSE_PRIM_vps_data_1, vps_byte_15,
		 	(uint32_t)pVPSData->ucByte15                 ) );
	BREG_Write32 (hReg, BCHP_WSE_PRIM_vps_data_1 + ulCoreOffset, ulVpsReg);

	BDBG_LEAVE(BVBI_P_VPS_Encode_Data_isr);
	return ulErrInfo;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_P_VPS_Encode_Enable_isr (
	BREG_Handle hReg,
	bool is656,
	uint8_t hwCoreIndex,
	BFMT_VideoFmt eVideoFormat,
	bool bEnable)
{
	uint32_t ulCoreOffset;
	uint32_t ulVps_controlReg;

	/* TODO: handle progressive video */
	BSTD_UNUSED (eVideoFormat);

	BDBG_ENTER(BVBI_P_VPS_Encode_Enable_isr);

	/* Figure out which encoder core to use */
	ulCoreOffset = P_GetCoreOffset_isr (hwCoreIndex);
	if (ulCoreOffset == 0xFFFFFFFF)
	{
		/* This should never happen!  This parameter was checked by
		   BVBI_Encode_Create() */
			BDBG_LEAVE(BVBI_P_VPS_Encode_Enable_isr);
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* No VPS over ITU-R 656 */
	if (is656)
	{
		if (bEnable)
		{
			/* No bypass encoder for VPS */
			BDBG_LEAVE(BVBI_P_VPS_Encode_Enable_isr);
			return BERR_TRACE(BERR_INVALID_PARAMETER);
		}
		else
		{
			/* Do nothing */
			return BERR_SUCCESS;
		}
	}

	ulVps_controlReg =
		BREG_Read32 (hReg, BCHP_WSE_PRIM_vps_control + ulCoreOffset);
	ulVps_controlReg &= ~(
		BCHP_MASK       (WSE_PRIM_vps_control, enable                   ) );
	if (bEnable)
	{
		ulVps_controlReg |= (
			BCHP_FIELD_DATA (WSE_PRIM_vps_control, enable,                 1) );
	}
	else
	{
		ulVps_controlReg |= (
			BCHP_FIELD_DATA (WSE_PRIM_vps_control, enable,                 0) );
	}
	BREG_Write32 (
		hReg, BCHP_WSE_PRIM_vps_control + ulCoreOffset, ulVps_controlReg);

	BDBG_LEAVE(BVBI_P_VPS_Encode_Enable_isr);
	return BERR_SUCCESS;
}


/***************************************************************************
* Static (private) functions
***************************************************************************/

/***************************************************************************
 *
 */
static uint32_t P_GetCoreOffset_isr (uint8_t hwCoreIndex)
{
	uint32_t ulCoreOffset = 0xFFFFFFFF;

	switch (hwCoreIndex)
	{
#if (BVBI_NUM_WSE >= 1)
	case 0:
		ulCoreOffset = 0;
		break;
#endif
#if (BVBI_NUM_WSE >= 2)
	case 1:
		ulCoreOffset = (BCHP_WSE_SEC_control - BCHP_WSE_PRIM_control);
		break;
#endif
#if (BVBI_NUM_WSE >= 3)
	case 2:
		ulCoreOffset = (BCHP_WSE_TERT_control - BCHP_WSE_PRIM_control);
		break;
#endif
	default:
		break;
	}

	return ulCoreOffset;
}

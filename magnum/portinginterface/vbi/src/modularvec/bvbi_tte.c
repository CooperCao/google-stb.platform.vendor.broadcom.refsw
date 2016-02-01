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

#include "bstd.h"           /* standard types */
#include "bdbg.h"           /* Dbglib */
#include "bvbi.h"           /* VBI processing, this module. */
#include "bvbi_cap.h"
#include "bkni.h"			/* For critical sections */
#include "bvbi_priv.h"      /* VBI internal data structures */
#if (BVBI_NUM_TTE >= 1)
#include "bchp_tte_prim.h"  /* RDB info for primary TTE core */
#endif
#if (BVBI_NUM_TTE >= 2)
#include "bchp_tte_sec.h"   /* RDB info for secondary TTE core */
#endif
#if (BVBI_NUM_TTE >= 3)
#include "bchp_tte_tert.h"   /* RDB info for tertiary TTE core */
#endif
#if (BVBI_NUM_TTE_656 >= 1)
#include "bchp_tte_656.h"   /* RDB info for ITU-R 656 "bypass" TTE core */
#endif

/* Welcome to alias central */
#if (BCHP_CHIP == 7601) && (BCHP_VER >= BCHP_VER_B0)
	#define BCHP_TTE_PRIM_control_shift_direction_MSBToLSB \
		BCHP_TTE_PRIM_CONTROL_shift_direction_MSBToLSB
	#define BCHP_TTE_PRIM_control BCHP_TTE_PRIM_CONTROL
	#define BCHP_TTE_PRIM_control_shift_direction_LSBToMSB \
		BCHP_TTE_PRIM_CONTROL_shift_direction_LSBToMSB
	#define BCHP_TTE_PRIM_control_constant_phase_MASK \
		BCHP_TTE_PRIM_CONTROL_constant_phase_MASK
	#define BCHP_TTE_PRIM_control_anci656_enable_MASK \
		BCHP_TTE_PRIM_CONTROL_anci656_enable_MASK
	#define BCHP_TTE_PRIM_control_anci656_output_fc_MASK \
		BCHP_TTE_PRIM_CONTROL_anci656_output_fc_MASK
	#define BCHP_TTE_PRIM_control_shift_direction_MASK \
		BCHP_TTE_PRIM_CONTROL_shift_direction_MASK
	#define BCHP_TTE_PRIM_control_enable_tf_MASK \
		BCHP_TTE_PRIM_CONTROL_enable_tf_MASK
	#define BCHP_TTE_PRIM_control_enable_bf_MASK \
		BCHP_TTE_PRIM_CONTROL_enable_bf_MASK
	#define BCHP_TTE_PRIM_control_constant_phase_SHIFT \
		BCHP_TTE_PRIM_CONTROL_constant_phase_SHIFT
	#define BCHP_TTE_PRIM_control_anci656_enable_SHIFT \
		BCHP_TTE_PRIM_CONTROL_anci656_enable_SHIFT
	#define BCHP_TTE_PRIM_control_anci656_output_fc_SHIFT \
		BCHP_TTE_PRIM_CONTROL_anci656_output_fc_SHIFT
	#define BCHP_TTE_PRIM_control_shift_direction_SHIFT \
		BCHP_TTE_PRIM_CONTROL_shift_direction_SHIFT
	#define BCHP_TTE_PRIM_control_enable_tf_SHIFT \
		BCHP_TTE_PRIM_CONTROL_enable_tf_SHIFT
	#define BCHP_TTE_PRIM_control_enable_bf_SHIFT \
		BCHP_TTE_PRIM_CONTROL_enable_bf_SHIFT
	#define BCHP_TTE_PRIM_top_mask BCHP_TTE_PRIM_TOP_MASK
	#define BCHP_TTE_PRIM_bottom_mask BCHP_TTE_PRIM_BOTTOM_MASK
	#define BCHP_TTE_PRIM_output_format BCHP_TTE_PRIM_OUTPUT_FORMAT
	#define BCHP_TTE_PRIM_control_start_delay_MASK \
		BCHP_TTE_PRIM_CONTROL_start_delay_MASK
	#define BCHP_TTE_PRIM_control_teletext_mode_MASK \
		BCHP_TTE_PRIM_CONTROL_teletext_mode_MASK
	#define BCHP_TTE_PRIM_control_start_delay_SHIFT \
		BCHP_TTE_PRIM_CONTROL_start_delay_SHIFT
	#define BCHP_TTE_PRIM_control_teletext_mode_NABTS \
		BCHP_TTE_PRIM_CONTROL_teletext_mode_NABTS
	#define BCHP_TTE_PRIM_control_teletext_mode_SHIFT \
		BCHP_TTE_PRIM_CONTROL_teletext_mode_SHIFT
	#define BCHP_TTE_PRIM_output_format_output_attenuation_MASK \
		BCHP_TTE_PRIM_OUTPUT_FORMAT_output_attenuation_MASK
	#define BCHP_TTE_PRIM_control_teletext_mode_ETSTeletext \
		BCHP_TTE_PRIM_CONTROL_teletext_mode_ETSTeletext
	#define BCHP_TTE_PRIM_read_address_top BCHP_TTE_PRIM_READ_ADDRESS_TOP
	#define BCHP_TTE_PRIM_output_format_output_attenuation_SHIFT \
		BCHP_TTE_PRIM_OUTPUT_FORMAT_output_attenuation_SHIFT
	#define BCHP_TTE_PRIM_read_address_bottom BCHP_TTE_PRIM_READ_ADDRESS_BOTTOM
	#define BCHP_TTE_PRIM_lines_active BCHP_TTE_PRIM_LINES_ACTIVE
	#define BCHP_TTE_PRIM_status BCHP_TTE_PRIM_STATUS
	#define BCHP_TTE_PRIM_status_data_sent_tf_MASK \
		BCHP_TTE_PRIM_STATUS_data_sent_tf_MASK
	#define BCHP_TTE_PRIM_lines_active_startline_tf_MASK \
		BCHP_TTE_PRIM_LINES_ACTIVE_startline_tf_MASK
	#define BCHP_TTE_PRIM_lines_active_startline_tf_SHIFT \
		BCHP_TTE_PRIM_LINES_ACTIVE_startline_tf_SHIFT
	#define BCHP_TTE_PRIM_status_data_sent_bf_MASK \
		BCHP_TTE_PRIM_STATUS_data_sent_bf_MASK
	#define BCHP_TTE_PRIM_lines_active_startline_bf_MASK \
		BCHP_TTE_PRIM_LINES_ACTIVE_startline_bf_MASK
	#define BCHP_TTE_PRIM_lines_active_startline_bf_SHIFT \
		BCHP_TTE_PRIM_LINES_ACTIVE_startline_bf_SHIFT
#endif

BDBG_MODULE(BVBI);

/* Welcome to alias central */
#if (BCHP_CHIP == 7118) || (BCHP_CHIP == 7400) || (BCHP_CHIP == 7401) || \
    (BCHP_CHIP == 7403) || (BCHP_CHIP == 3563) || (BCHP_CHIP == 7038) || \
	(BCHP_CHIP == 7438)
	#define BCHP_TTE_PRIM_Reset             BCHP_TTE_PRIM_reset
	#define BCHP_TTE_PRIM_Reset_reset_SHIFT BCHP_TTE_PRIM_reset_reset_SHIFT
#endif
#if (BCHP_CHIP == 7440)
	#define BCHP_TTE_PRIM_Reset             BCHP_TTE_PRIM_reset
	#define BCHP_TTE_PRIM_Reset_reset_SHIFT BCHP_TTE_PRIM_reset_reset_SHIFT
#endif


/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/
static uint32_t P_GetCoreOffset_isr (bool is656, uint8_t hwCoreIndex);


/***************************************************************************
* Implementation of "BVBI_" API functions
***************************************************************************/


/***************************************************************************
* Implementation of supporting teletext functions that are not in API
***************************************************************************/

static BERR_Code BVBI_P_TT_Enc_Program_isr (
	BREG_Handle hReg,
	BMEM_Handle hMem,
	bool is656,
	uint8_t hwCoreIndex,
	bool bActive,
	bool bXserActive,
	BFMT_VideoFmt eVideoFormat,
	bool tteShiftDirMsb2Lsb,
	BVBI_XSER_Settings* xserSettings,
	BVBI_P_TTData* topData,
	BVBI_P_TTData* botData
);

static void BVBI_P_TT_Enc_Init_isr (
	BREG_Handle hReg, bool is656, uint8_t hwCoreIndex);

/***************************************************************************
 *
 */
void BVBI_P_TT_Enc_Init (BREG_Handle hReg, bool is656, uint8_t hwCoreIndex)
{
	BKNI_EnterCriticalSection();
	BVBI_P_TT_Enc_Init_isr (hReg, is656, hwCoreIndex);
	BKNI_LeaveCriticalSection();
}

/***************************************************************************
 *
 */
void BVBI_P_TT_Enc_Init_isr (BREG_Handle hReg, bool is656, uint8_t hwCoreIndex)
{
	BDBG_ENTER(BVBI_P_TT_Enc_Init_isr);

	BVBI_P_VIE_SoftReset_isr (hReg, is656, hwCoreIndex, BVBI_P_SELECT_TT);

	BDBG_LEAVE(BVBI_P_TT_Enc_Init_isr);
}


/***************************************************************************
 *
 */
BERR_Code BVBI_P_TT_Enc_Program (
	BREG_Handle hReg,
	BMEM_Handle hMem,
	bool is656,
	uint8_t hwCoreIndex,
	bool bActive,
	bool bXserActive,
	BFMT_VideoFmt eVideoFormat,
	bool tteShiftDirMsb2Lsb,
	BVBI_XSER_Settings* xserSettings,
	BVBI_P_TTData* topData,
	BVBI_P_TTData* botData
)
{
	BERR_Code retval;
	BDBG_ENTER(BVBI_P_TT_Enc_Program);
	BKNI_EnterCriticalSection();
	retval = BVBI_P_TT_Enc_Program_isr (
		hReg, hMem, is656, hwCoreIndex, bActive, bXserActive,
		eVideoFormat, tteShiftDirMsb2Lsb, xserSettings, topData, botData);
	BKNI_LeaveCriticalSection();
	BDBG_LEAVE(BVBI_P_TT_Enc_Program);
	return retval;
}


/***************************************************************************
 *
 */
static BERR_Code BVBI_P_TT_Enc_Program_isr (
	BREG_Handle hReg,
	BMEM_Handle hMem,
	bool is656,
	uint8_t hwCoreIndex,
	bool bActive,
	bool bXserActive,
	BFMT_VideoFmt eVideoFormat,
	bool tteShiftDirMsb2Lsb,
	BVBI_XSER_Settings* xserSettings,
	BVBI_P_TTData* topData,
	BVBI_P_TTData* botData
)
{
	uint32_t ulControlReg;
	uint32_t ulFormatReg;
#if (BVBI_P_HAS_XSER_TT >= 1)
	uint32_t iSerialPortMode;
	uint32_t iSerialPort;
#endif

	uint8_t  ucNumLinesTF;
	uint8_t  ucNumLinesBF;
	uint8_t  ucBytesPerLine;

	uint32_t offset;
	uint32_t ulCoreOffset;
	uint32_t ulShiftDir;
	BERR_Code eErr;

#if (BVBI_P_HAS_XSER_TT >= 1)
#else
	BSTD_UNUSED (bXserActive);
	BSTD_UNUSED (xserSettings);
#endif

	BDBG_ENTER(BVBI_P_TT_Enc_Program_isr);

	/* Figure out which encoder core to use */
	ulCoreOffset = P_GetCoreOffset_isr (is656, hwCoreIndex);
	if (ulCoreOffset == 0xFFFFFFFF)
	{
		/* This should never happen!  This parameter was checked by
		   BVBI_Encode_Create() */
		BDBG_LEAVE(BVBI_P_TT_Enc_Program_isr);
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* TODO: Verify little endian */

	/* If user wants to turn off teletext processing, just reset the
	   entire core. */
	if (!bActive)
	{
		BVBI_P_TT_Enc_Init_isr (hReg, is656, hwCoreIndex);
		BDBG_LEAVE(BVBI_P_TT_Enc_Program_isr);
		return BERR_SUCCESS;
	}

#if (BVBI_P_HAS_XSER_TT >= 1)
	iSerialPort = (bXserActive ?
		BCHP_TTE_PRIM_control_serial_port_ENABLE :
		BCHP_TTE_PRIM_control_serial_port_DISABLE);
	switch (xserSettings->xsSerialDataContent)
	{
	case BVBI_TTserialDataContent_None:
		iSerialPortMode = BCHP_TTE_PRIM_control_serial_port_mode_DATA_ONLY;
		iSerialPort = BCHP_TTE_PRIM_control_serial_port_DISABLE;
		break;
	case BVBI_TTserialDataContent_DataOnly:
		iSerialPortMode = BCHP_TTE_PRIM_control_serial_port_mode_DATA_ONLY;
		break;
	case BVBI_TTserialDataContent_DataMag:
		iSerialPortMode = BCHP_TTE_PRIM_control_serial_port_mode_MAGAZINE_DATA;
		break;
	case BVBI_TTserialDataContent_DataMagFrm:
		iSerialPortMode =
			BCHP_TTE_PRIM_control_serial_port_mode_FRM_MAGAZINE_DATA;
		break;
	case BVBI_TTserialDataContent_DataMagFrmRun:
		iSerialPortMode =
			BCHP_TTE_PRIM_control_serial_port_mode_RUNIN_FRM_MAGAZINE_DATA;
		break;
	default:
		iSerialPortMode = BCHP_TTE_PRIM_control_serial_port_mode_DATA_ONLY;
		/* This should never happen!  This parameter was checked by
		   BVBI_Encode_Create() */
		BDBG_LEAVE(BVBI_P_TT_Enc_Program_isr);
		return BERR_TRACE(BERR_INVALID_PARAMETER);
		break;
	}
#endif

	if (tteShiftDirMsb2Lsb)
		ulShiftDir = BCHP_TTE_PRIM_control_shift_direction_MSBToLSB;
	else
		ulShiftDir = BCHP_TTE_PRIM_control_shift_direction_LSBToMSB;

	/* Start programming the TTE control register */
    ulControlReg = BREG_Read32 (hReg, BCHP_TTE_PRIM_control + ulCoreOffset);
	ulControlReg &= ~(
#if (BVBI_P_HAS_XSER_TT >= 1)
		BCHP_MASK       (TTE_PRIM_control, serial_port_mode         ) |
		BCHP_MASK       (TTE_PRIM_control, serial_port              ) |
#endif
		BCHP_MASK       (TTE_PRIM_control, constant_phase           ) |
		BCHP_MASK       (TTE_PRIM_control, anci656_enable           ) |
		BCHP_MASK       (TTE_PRIM_control, anci656_output_fc        ) |
		BCHP_MASK       (TTE_PRIM_control, shift_direction          ) |
		BCHP_MASK       (TTE_PRIM_control, enable_tf                ) |
		BCHP_MASK       (TTE_PRIM_control, enable_bf                ) );
	ulControlReg |= (
#if (BVBI_P_HAS_XSER_TT >= 1)
		 BCHP_FIELD_DATA (TTE_PRIM_control, serial_port_mode,
													 iSerialPortMode) |
		 BCHP_FIELD_DATA (TTE_PRIM_control, serial_port, iSerialPort) |
#endif
		BCHP_FIELD_DATA (TTE_PRIM_control, constant_phase,         0) |
		BCHP_FIELD_DATA (TTE_PRIM_control, anci656_enable,         1) |
		BCHP_FIELD_DATA (TTE_PRIM_control, anci656_output_fc,      1) |
		BCHP_FIELD_DATA (TTE_PRIM_control, shift_direction, ulShiftDir) |
		BCHP_FIELD_DATA (TTE_PRIM_control, enable_tf,              1) |
		BCHP_FIELD_DATA (TTE_PRIM_control, enable_bf,              1) );

    /* Program the TTE top mask register */
    BREG_Write32 (hReg, BCHP_TTE_PRIM_top_mask + ulCoreOffset, 0x0);

    /* Program the TTE bottom mask register */
    BREG_Write32 (hReg, BCHP_TTE_PRIM_bottom_mask + ulCoreOffset, 0x0);

	/* Start programming the output format register */
	ulFormatReg =
		BREG_Read32 (hReg, BCHP_TTE_PRIM_output_format + ulCoreOffset);

	/* Select video format */
	switch (eVideoFormat)
	{
    case BFMT_VideoFmt_eNTSC:
    case BFMT_VideoFmt_eNTSC_J:
	case BFMT_VideoFmt_e720x482_NTSC:
	case BFMT_VideoFmt_e720x482_NTSC_J:
    case BFMT_VideoFmt_ePAL_M:
        /* NTSC specific settings */

		ucNumLinesTF   =  11;
		ucNumLinesBF   =  11;
		ucBytesPerLine =  34;

		/* Continue programming the control register */
		ulControlReg &= ~(
			BCHP_MASK       (TTE_PRIM_control, start_delay         ) |
			BCHP_MASK       (TTE_PRIM_control, teletext_mode       ) );
		ulControlReg |= (
			BCHP_FIELD_DATA (TTE_PRIM_control, start_delay,    0x1F) |
			BCHP_FIELD_ENUM (TTE_PRIM_control, teletext_mode, NABTS) );

		/* Continue programming the output_format register */
		ulFormatReg &=
			~BCHP_MASK       (TTE_PRIM_output_format, output_attenuation     );
		ulFormatReg |=
			 BCHP_FIELD_DATA (TTE_PRIM_output_format, output_attenuation,0x63);

		break;

    case BFMT_VideoFmt_ePAL_B:
    case BFMT_VideoFmt_ePAL_B1:
    case BFMT_VideoFmt_ePAL_D:
    case BFMT_VideoFmt_ePAL_D1:
    case BFMT_VideoFmt_ePAL_G:
    case BFMT_VideoFmt_ePAL_H:
    case BFMT_VideoFmt_ePAL_K:
    case BFMT_VideoFmt_ePAL_I:
    case BFMT_VideoFmt_ePAL_N:
    case BFMT_VideoFmt_ePAL_NC:
    case BFMT_VideoFmt_eSECAM_L:
    case BFMT_VideoFmt_eSECAM_B:
    case BFMT_VideoFmt_eSECAM_G:
    case BFMT_VideoFmt_eSECAM_D:
    case BFMT_VideoFmt_eSECAM_K:
    case BFMT_VideoFmt_eSECAM_H:
        /* 576I specific settings */

		ucNumLinesTF   = 17;
		ucNumLinesBF   = 18;
		ucBytesPerLine = 43;

		/* Continue programming the control register */
		ulControlReg &= ~(
			BCHP_MASK       (TTE_PRIM_control, start_delay               ) |
			BCHP_MASK       (TTE_PRIM_control, teletext_mode             ) );
		ulControlReg |= (
			BCHP_FIELD_DATA (TTE_PRIM_control, start_delay,          0x00) |
			BCHP_FIELD_ENUM (TTE_PRIM_control, teletext_mode, ETSTeletext) );

		/* Continue programming the output_format register */
		ulFormatReg &=
			~BCHP_MASK       (TTE_PRIM_output_format, output_attenuation      );
		ulFormatReg |=
			 BCHP_FIELD_DATA (TTE_PRIM_output_format, output_attenuation, 0x5a);

		break;

	default:
		BDBG_LEAVE(BVBI_P_TT_Enc_Program_isr);
		return BERR_TRACE (BERR_INVALID_PARAMETER);
		break;
	}

	/* Prepare to send data in the encode handle */
	eErr = BERR_TRACE (BMEM_ConvertAddressToOffset_isr (
		hMem, topData->pucData, &offset));
	BDBG_ASSERT (eErr == BERR_SUCCESS);
	BREG_Write32 (hReg, BCHP_TTE_PRIM_read_address_top + ulCoreOffset, offset);
	eErr = BERR_TRACE (BMEM_ConvertAddressToOffset_isr (
		hMem, botData->pucData, &offset));
	BDBG_ASSERT (eErr == BERR_SUCCESS);
	BREG_Write32 (
		hReg, BCHP_TTE_PRIM_read_address_bottom + ulCoreOffset, offset);

	/* Update the field handles that send the data */
	topData->ucLines    = ucNumLinesTF;
	topData->ucLineSize = ucBytesPerLine;
	botData->ucLines    = ucNumLinesBF;
	botData->ucLineSize = ucBytesPerLine;

	/* write the three registers with updated values */
    BREG_Write32 (
		hReg, BCHP_TTE_PRIM_control       + ulCoreOffset, ulControlReg);
    BREG_Write32 (
		hReg, BCHP_TTE_PRIM_output_format + ulCoreOffset,  ulFormatReg);

	BDBG_LEAVE(BVBI_P_TT_Enc_Program_isr);
	return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
uint32_t BVBI_P_TT_Encode_Data_isr (
	BREG_Handle hReg,
	BMEM_Handle hMem,
	bool is656,
	uint8_t hwCoreIndex,
	BFMT_VideoFmt eVideoFormat,
	BAVC_Polarity polarity,
	bool bPR18010_bad_line_number,
	BVBI_P_TTData* pTTDataNext )
{
/*
	Programming note: the implementation here assumes that the bitfield layout
	within registers is the same for all teletext encoder cores in the chip.

	If a chip is built that has multiple teletext encoder cores that are not
	identical, then this routine will have to be redesigned.
*/
	uint32_t ulCoreOffset;
	uint32_t H_ReAdTop;
	uint32_t H_ReAdBot;
	uint32_t H_MaskTop;
	uint32_t H_MaskBot;
	uint32_t H_Lines;
	uint32_t H_Status;
	uint32_t ulStatusReg;
	uint16_t usStartLineTF;
	uint16_t usStartLineBF;
	uint8_t  ucMinLines;
	uint8_t  ucMinLineSize;
	uint32_t ulLinesReg;
	uint32_t offset;
	uint32_t lineMask;
#ifndef BVBI_P_TTE_WA15
	void*    cached_ptr;
	BERR_Code eErr;
#endif
	uint32_t ulErrInfo = 0;

	/* Debug code
	uint8_t* printme = 0;
	*/

#if (BVBI_NUM_TTE_656 >= 1)
#else
	BSTD_UNUSED (bPR18010_bad_line_number);
#endif

	BDBG_ENTER(BVBI_P_TT_Encode_Data_isr);

	/* Figure out which encoder core to use */
	ulCoreOffset = P_GetCoreOffset_isr (is656, hwCoreIndex);
	if (ulCoreOffset == 0xFFFFFFFF)
	{
		/* This should never happen!  This parameter was checked by
		   BVBI_Encode_Create() */
		BDBG_LEAVE(BVBI_P_TT_Encode_Data_isr);
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	H_ReAdTop = BCHP_TTE_PRIM_read_address_top + ulCoreOffset;
	H_ReAdBot = BCHP_TTE_PRIM_read_address_bottom + ulCoreOffset;
	H_MaskTop = BCHP_TTE_PRIM_top_mask + ulCoreOffset;
	H_MaskBot = BCHP_TTE_PRIM_bottom_mask + ulCoreOffset;
	H_Lines   = BCHP_TTE_PRIM_lines_active + ulCoreOffset;
	H_Status  = BCHP_TTE_PRIM_status + ulCoreOffset;

	/* Verify that field handle is big enough to hold the TT data */
	switch (eVideoFormat)
	{
    case BFMT_VideoFmt_eNTSC:
    case BFMT_VideoFmt_eNTSC_J:
	case BFMT_VideoFmt_e720x482_NTSC:
	case BFMT_VideoFmt_e720x482_NTSC_J:
    case BFMT_VideoFmt_ePAL_M:
		ucMinLines    = 13;
		ucMinLineSize = 34;
		usStartLineTF  =  10 - 1;
		usStartLineBF  = 273 - 263;
#if (BVBI_NUM_TTE_656 >= 1)
		if (is656 && !bPR18010_bad_line_number)
		{
			usStartLineTF -= 1;
			usStartLineBF -= 1;
		}
#endif
		break;
    case BFMT_VideoFmt_ePAL_B:
    case BFMT_VideoFmt_ePAL_B1:
    case BFMT_VideoFmt_ePAL_D:
    case BFMT_VideoFmt_ePAL_D1:
    case BFMT_VideoFmt_ePAL_G:
    case BFMT_VideoFmt_ePAL_H:
    case BFMT_VideoFmt_ePAL_K:
    case BFMT_VideoFmt_ePAL_I:
    case BFMT_VideoFmt_ePAL_N:
    case BFMT_VideoFmt_ePAL_NC:
    case BFMT_VideoFmt_eSECAM_L:
    case BFMT_VideoFmt_eSECAM_B:
    case BFMT_VideoFmt_eSECAM_G:
    case BFMT_VideoFmt_eSECAM_D:
    case BFMT_VideoFmt_eSECAM_K:
    case BFMT_VideoFmt_eSECAM_H:
		ucMinLines    = 18;
		ucMinLineSize = 43;
		usStartLineTF  = 6 - 1;
		usStartLineBF  = 318 - 313;
#if (BVBI_NUM_TTE_656 >= 1)
		if (is656 && !bPR18010_bad_line_number)
		{
			usStartLineTF -= 1;
			usStartLineBF -= 1;
		}
#endif
		/* PR18343 */
		usStartLineTF -= 1;
		break;
	default:
		/* This should never happen! */
		ulErrInfo = (uint32_t)(-1);
		BDBG_LEAVE(BVBI_P_TT_Encode_Data_isr);
		return ulErrInfo;
		break;
	}
	if ( (pTTDataNext->ucLines     >    ucMinLines) ||
		 (pTTDataNext->ucLineSize != ucMinLineSize)    )
	{
		ulErrInfo |= BVBI_LINE_ERROR_FLDH_CONFLICT;
		BDBG_LEAVE(BVBI_P_TT_Encode_Data_isr);
		return ulErrInfo;
	}

	/* Read the status register */
	ulStatusReg = BREG_Read32 (hReg, H_Status);

	/* Start programming the lines_active register */
	ulLinesReg = BREG_Read32 (hReg, H_Lines);

	/* If top field */
	if (polarity == BAVC_Polarity_eTopField)
	{
		/* Check for hardware busy */
		if ((ulStatusReg & BCHP_MASK (TTE_PRIM_status, data_sent_tf)) == 0)
		{
			ulErrInfo |= BVBI_LINE_ERROR_TELETEXT_OVERRUN;
			goto done;
		}

		/* Will clear hardware status */
		ulStatusReg = BCHP_MASK (TTE_PRIM_status, data_sent_tf);

		/* Give hardware a new place to encode data from */
		if (BMEM_ConvertAddressToOffset_isr (
			hMem, pTTDataNext->pucData, &offset) !=
			BERR_SUCCESS)
		{
			ulErrInfo = (uint32_t)(-1);
			goto done;
		}
		BREG_Write32 (hReg, H_ReAdTop, offset);

		/* Program the masking register */
		lineMask =  pTTDataNext->lineMask;
#ifdef BVBI_P_TTE_WA15
		/* This causes problems on 3563-C0 */
		BREG_Write32 (hReg, H_MaskTop, lineMask);
#else
		BREG_Write32 (hReg, H_MaskTop, 0xFFFFFFFF);
		eErr = BMEM_ConvertAddressToCached_isr (
			hMem, pTTDataNext->pucData, &cached_ptr);
		if (eErr != BERR_SUCCESS)
		{
			ulErrInfo = (uint32_t)(-1);
			goto done;
		}
		*(uint32_t*)(cached_ptr) = lineMask;
		BMEM_FlushCache_isr (hMem, cached_ptr, sizeof(uint32_t));
#endif

		/* Continue programming the lines_active register */
		ulLinesReg &= ~BCHP_MASK (TTE_PRIM_lines_active, startline_tf);
		ulLinesReg |=  BCHP_FIELD_DATA (
			TTE_PRIM_lines_active, startline_tf,
			pTTDataNext->firstLine + usStartLineTF);

		/* Debug code
		printme = pTTDataNext->pucData;
		*/
	}
	else /* Bottom field */
	{
		/* Check for hardware busy */
		if ((ulStatusReg & BCHP_MASK (TTE_PRIM_status, data_sent_bf)) == 0)
		{
			ulErrInfo |= BVBI_LINE_ERROR_TELETEXT_OVERRUN;
			goto done;
		}

		/* Will clear hardware status */
		ulStatusReg = BCHP_MASK (TTE_PRIM_status, data_sent_bf);

		/* Give hardware a new place to encode data from */
		if (BMEM_ConvertAddressToOffset_isr (
			hMem, pTTDataNext->pucData, &offset) !=
			BERR_SUCCESS)
		{
			ulErrInfo = (uint32_t)(-1);
			goto done;
		}
		BREG_Write32 (hReg, H_ReAdBot, offset);

		/* Program the masking register */
		lineMask =  pTTDataNext->lineMask;
#ifdef BVBI_P_TTE_WA15
		/* This causes problems on 3563-C0 */
		BREG_Write32 (hReg, H_MaskBot, lineMask);
#else
		BREG_Write32 (hReg, H_MaskBot, 0xFFFFFFFF);
		eErr = BMEM_ConvertAddressToCached_isr (
			hMem, pTTDataNext->pucData, &cached_ptr);
		if (eErr != BERR_SUCCESS)
		{
			ulErrInfo = (uint32_t)(-1);
			goto done;
		}
		*(uint32_t*)(cached_ptr) = lineMask;
		BMEM_FlushCache_isr (hMem, cached_ptr, sizeof(uint32_t));
#endif

		/* Continue programming the lines_active register */
		ulLinesReg &= ~BCHP_MASK (TTE_PRIM_lines_active, startline_bf);
		ulLinesReg |=  BCHP_FIELD_DATA (
			TTE_PRIM_lines_active, startline_bf,
			pTTDataNext->firstLine + usStartLineBF);

		/* Debug code
		printme = pTTDataNext->pucData;
		*/
	}

	/* Finish programming the lines_active register */
    BREG_Write32 (hReg, H_Lines, ulLinesReg);

	/* Finish clearing status */
    BREG_Write32 (hReg, H_Status, ulStatusReg);

	/* Debug code */
	/*
	{
	static uint32_t dcounter = 0;
	++dcounter;
	if ((dcounter > 80) && (dcounter < 150))
	{
		if (printme)
		{
			uint32_t mask = *(uint32_t*)printme;
			char* p1 = printme + 4;
			char* p2 = printme + (4 + 34);
			printf ("%d%c: At %08x: encoding M:%08x \"%s\" \"%s\"\n",
				dcounter,
				(polarity == BAVC_Polarity_eTopField) ? 'T' : 'B',
				offset,
				mask, p1, p2);
		}
		else
			printf ("%d%c: Did not encode anything\n",
				dcounter,
				(polarity == BAVC_Polarity_eTopField) ? 'T' : 'B');
		{
		}
	}
	}
	*/

done:
	BDBG_LEAVE(BVBI_P_TT_Encode_Data_isr);
	return ulErrInfo;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_P_TT_Encode_Enable_isr (
	BREG_Handle hReg,
	bool is656,
	uint8_t hwCoreIndex,
	BFMT_VideoFmt eVideoFormat,
	bool bEnable)
{
	uint32_t ulCoreOffset;
	uint32_t ulControlReg;

	/* TODO: handle progressive video */
	BSTD_UNUSED (eVideoFormat);

	BDBG_ENTER(BVBI_P_TT_Encode_Enable_isr);

	/* Figure out which encoder core to use */
	ulCoreOffset = P_GetCoreOffset_isr (is656, hwCoreIndex);
	if (ulCoreOffset == 0xFFFFFFFF)
	{
		/* This should never happen!  This parameter was checked by
		   BVBI_Encode_Create() */
		BDBG_LEAVE(BVBI_P_TT_Encode_Enable_isr);
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

    ulControlReg = BREG_Read32 ( hReg, BCHP_TTE_PRIM_control + ulCoreOffset );
	ulControlReg &= ~(
		BCHP_MASK (TTE_PRIM_control, enable_tf) |
		BCHP_MASK (TTE_PRIM_control, enable_bf) );
	if (bEnable)
	{
		ulControlReg |= (
			BCHP_FIELD_DATA (TTE_PRIM_control, enable_tf, 1) |
			BCHP_FIELD_DATA (TTE_PRIM_control, enable_bf, 1) );
	}
	else
	{
		ulControlReg |= (
			BCHP_FIELD_DATA (TTE_PRIM_control, enable_tf, 0) |
			BCHP_FIELD_DATA (TTE_PRIM_control, enable_bf, 0) );
	}
	BREG_Write32 ( hReg, BCHP_TTE_PRIM_control + ulCoreOffset, ulControlReg );

	BDBG_LEAVE(BVBI_P_TT_Encode_Enable_isr);
	return BERR_SUCCESS;
}

/***************************************************************************
* Static (private) functions
***************************************************************************/

/***************************************************************************
 *
 */
static uint32_t P_GetCoreOffset_isr (bool is656, uint8_t hwCoreIndex)
{
	uint32_t ulCoreOffset = 0xFFFFFFFF;

	if (is656)
	{
#if (BVBI_NUM_TTE_656 >= 1)
		ulCoreOffset = (BCHP_TTE_656_status - BCHP_TTE_PRIM_status);
#endif
	}
	else
	{
		switch (hwCoreIndex)
		{
#if (BVBI_NUM_TTE >= 1)
		case 0:
			ulCoreOffset = 0;
			break;
#endif
#if (BVBI_NUM_TTE >= 2)
		case 1:
			ulCoreOffset = (BCHP_TTE_SEC_status - BCHP_TTE_PRIM_status);
			break;
#endif
#if (BVBI_NUM_TTE >= 3)
		case 2:
			ulCoreOffset = (BCHP_TTE_SEC_status - BCHP_TTE_PRIM_status);
			break;
#endif
		default:
			break;
		}
	}

	return ulCoreOffset;
}

/* End of file */

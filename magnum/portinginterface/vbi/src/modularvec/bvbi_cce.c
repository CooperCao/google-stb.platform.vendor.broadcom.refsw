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

#include "bstd.h"           /* standard types */
#include "bdbg.h"           /* Dbglib */
#include "bkni.h"			/* For critical sections */
#include "bvbi.h"           /* VBI processing, this module. */
#include "bvbi_cap.h"
#include "bvbi_priv.h"      /* VBI internal data structures */

#if (BVBI_NUM_CCE >= 1)
#include "bchp_cce_prim.h"  /* RDB info for primary CCE core */
#endif
#if (BVBI_NUM_CCE >= 2)
#include "bchp_cce_sec.h"   /* RDB info for secondary CCE core */
#endif
#if (BVBI_NUM_CCE >= 3)
#include "bchp_cce_tert.h"  /* RDB info for tertiary CCE core */
#endif
#if (BVBI_NUM_CCE_656 >= 1)
#include "bchp_cce_656.h"   /* RDB info for ITU-R 656 passthrough CCE core */
#endif

/* Welcome to alias central */
#if (BCHP_CHIP == 7440) && (BCHP_VER == BCHP_VER_A0)
	#define BCHP_CCE_PRIM_Soft_Reset_CLOSED_CAPTION_RESET_SHIFT \
		BCHP_CCE_PRIM_Soft_Reset_RESET_SHIFT
#endif

BDBG_MODULE(BVBI);


/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/
static uint8_t BVBI_P_SetOddParityBit_isr( uint8_t uchByte );
static uint32_t P_GetCoreOffset_isr (bool is656, uint8_t hwCoreIndex);


/***************************************************************************
* Implementation supporting closed caption functions that are not in API
***************************************************************************/


/***************************************************************************
 *
 */
void BVBI_P_CC_Enc_Init (BREG_Handle hReg, bool is656, uint8_t hwCoreIndex)
{
	BDBG_ENTER(BVBI_P_CC_Enc_Init);

	BVBI_P_VIE_SoftReset_isr (hReg, is656, hwCoreIndex, BVBI_P_SELECT_CC);

	BDBG_LEAVE(BVBI_P_CC_Enc_Init);
}

/***************************************************************************
 *
 */
uint16_t BVBI_P_SetCCParityBits_isr( uint16_t uchData )
{
	union {
		uint16_t d16;
		uint8_t  d8[2];
	} convenience;

	BDBG_ENTER(BVBI_P_SetCCParityBits_isr);

	convenience.d16 = uchData;
	convenience.d8[0] = BVBI_P_SetOddParityBit_isr (convenience.d8[0]);
	convenience.d8[1] = BVBI_P_SetOddParityBit_isr (convenience.d8[1]);

	BDBG_LEAVE(BVBI_P_SetCCParityBits_isr);
	return convenience.d16;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_P_CC_Enc_Program (
	BREG_Handle hReg,
	bool is656,
	uint8_t hwCoreIndex,
	bool bActive,
	bool bPR18010_bad_line_number,
	BFMT_VideoFmt eVideoFormat,
	bool bArib480p)
{
/*
	Programming note: the implementation here assumes that the bitfield layout
	within registers is the same for all CC encoder cores in the chip.

	If a chip is built that has multiple CC encoder cores that are not
	identical, then this routine will have to be redesigned.
*/
	uint32_t ulControlReg;
	uint32_t ulLinesReg;
	uint32_t ulGain;
	uint32_t ulLineMask;
	uint32_t ulDelayCount;
	uint32_t ulGainDelayReg = 0;
	uint32_t ulCoreOffset;

	BDBG_ENTER(BVBI_P_CC_Enc_Program);

	/* Determine which core to access */
	ulCoreOffset = P_GetCoreOffset_isr (is656, hwCoreIndex);
	if (ulCoreOffset == 0xFFFFFFFF)
	{
		/* This should never happen!  This parameter was checked by
		   BVBI_Encode_Create() */
		BDBG_LEAVE(BVBI_P_CC_Enc_Program);
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* If user wants to turn off closed caption processing, just use the
	   enable bit. */
	if (!bActive)
	{
		ulControlReg =
			BREG_Read32 ( hReg, BCHP_CCE_PRIM_Control + ulCoreOffset );
		ulControlReg &=
			~BCHP_MASK      (CCE_PRIM_Control, ENABLE_CLOSED_CAPTION          );
		ulControlReg |=
			BCHP_FIELD_ENUM (CCE_PRIM_Control, ENABLE_CLOSED_CAPTION, DISABLED);
		BREG_Write32 (
			hReg, BCHP_CCE_PRIM_Control + ulCoreOffset, ulControlReg );
		BDBG_LEAVE(BVBI_P_CC_Enc_Program);
		return BERR_SUCCESS;
	}

	/* Select video format */
	switch (eVideoFormat)
	{
    case BFMT_VideoFmt_eNTSC:
    case BFMT_VideoFmt_eNTSC_J:
	case BFMT_VideoFmt_e720x482_NTSC:
	case BFMT_VideoFmt_e720x482_NTSC_J:
    case BFMT_VideoFmt_ePAL_M:
        /* NTSC specific settings */

		ulGain = 0x47;
		ulDelayCount = 0x3A;
		if (is656)
		{
			ulLineMask = (bPR18010_bad_line_number ? 0x0400 : 0x1000);
		}
		else
		{
			ulLineMask = 0x2000;
		}
		if (eVideoFormat == BFMT_VideoFmt_ePAL_M)
		{
			ulGain = 0x4A;
			ulDelayCount = 0xD;
			ulLineMask >>= 3;
		}
		else
		{
			if (bArib480p)
				ulLineMask >>= 1;
		}
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
		ulGain = 0x46;
		ulDelayCount = 0x01;
		if (is656)
		{
			ulLineMask = (bPR18010_bad_line_number ? 0x4000 : 0x2000);
		}
		else
		{
			ulLineMask = 0x4000;
		}
		break;
	default:
		BDBG_LEAVE(BVBI_P_CC_Enc_Program);
		BDBG_ERR(("This video format not supported yet"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
		break;
	}

	BKNI_EnterCriticalSection();

	/* set the format specific gain delay values, if any. */
	if (!is656)
	{
		ulGainDelayReg = BREG_Read32 (
			hReg, BCHP_CCE_PRIM_Gain_Delay + ulCoreOffset );
		ulGainDelayReg &= ~(
			BCHP_MASK       ( CCE_PRIM_Gain_Delay, GAIN             ) |
			BCHP_MASK       ( CCE_PRIM_Gain_Delay, DELAY_COUNT      )   );
		ulGainDelayReg |= (
			BCHP_FIELD_DATA ( CCE_PRIM_Gain_Delay, GAIN,      ulGain) |
			BCHP_FIELD_DATA ( CCE_PRIM_Gain_Delay, DELAY_COUNT,
			                                            ulDelayCount)   );
		BREG_Write32 (
			hReg, BCHP_CCE_PRIM_Gain_Delay + ulCoreOffset, ulGainDelayReg );
	}
	
	/* get the active lines register */
	/* Activate lines 22 and 335 for NTSC */
	ulLinesReg =
		BREG_Read32 ( hReg,  BCHP_CCE_PRIM_Active_Lines + ulCoreOffset );
	ulLinesReg &= ~(
		BCHP_MASK       ( CCE_PRIM_Active_Lines, TOP_ACTIVE             ) |
		BCHP_MASK       ( CCE_PRIM_Active_Lines, BOT_ACTIVE             ) );
	ulLinesReg |= (
		BCHP_FIELD_DATA ( CCE_PRIM_Active_Lines, TOP_ACTIVE, ulLineMask ) |
		BCHP_FIELD_DATA ( CCE_PRIM_Active_Lines, BOT_ACTIVE, ulLineMask ) );
	BREG_Write32 (
		hReg, BCHP_CCE_PRIM_Active_Lines + ulCoreOffset, ulLinesReg );

	/* program the control register with non-format specific values */
    ulControlReg = BREG_Read32 ( hReg, BCHP_CCE_PRIM_Control + ulCoreOffset );
	ulControlReg &= ~(
		BCHP_MASK      (CCE_PRIM_Control, reserved0                           )|
		BCHP_MASK      (CCE_PRIM_Control, reserved_for_eco1                   )|
		/* set parity bits so we provide the parity */
		BCHP_MASK      (CCE_PRIM_Control, TOP_FLD_PARITY                      )|
		BCHP_MASK      (CCE_PRIM_Control, BOT_FLD_PARITY                      )|
		BCHP_MASK      (CCE_PRIM_Control, TOP_FLD_STAT                        )|
		BCHP_MASK      (CCE_PRIM_Control, BOT_FLD_STAT                        )|
		BCHP_MASK      (CCE_PRIM_Control, NULL_CHARACTER                      )|
		BCHP_MASK      (CCE_PRIM_Control, BYTEIF_ENDIAN_ORDER                 )|
		BCHP_MASK      (CCE_PRIM_Control, BYTE_SWAP                           )|
		BCHP_MASK      (CCE_PRIM_Control, SHIFT_DIRECTION                     )|
		BCHP_MASK      (CCE_PRIM_Control, reserved2                           )|
		BCHP_MASK      (CCE_PRIM_Control, REGISTER_USE_MODE                   )|
		BCHP_MASK      (CCE_PRIM_Control, ENABLE_CLOSED_CAPTION               )
	);
	ulControlReg |= (
		BCHP_FIELD_DATA(CCE_PRIM_Control, reserved0,             0            )|
		BCHP_FIELD_DATA(CCE_PRIM_Control, reserved_for_eco1,     0            )|
		/* set parity bits so we provide the parity */
		BCHP_FIELD_ENUM( CCE_PRIM_Control, TOP_FLD_PARITY,       MANUAL       )|
		BCHP_FIELD_ENUM( CCE_PRIM_Control, BOT_FLD_PARITY,       MANUAL       )|
		BCHP_FIELD_DATA(CCE_PRIM_Control, TOP_FLD_STAT,          0            )|
		BCHP_FIELD_DATA(CCE_PRIM_Control, BOT_FLD_STAT,          0            )|
		BCHP_FIELD_DATA(CCE_PRIM_Control, NULL_CHARACTER,        0x80         )|
		BCHP_FIELD_ENUM(CCE_PRIM_Control, BYTEIF_ENDIAN_ORDER,   MAINTAIN     )|
		BCHP_FIELD_ENUM(CCE_PRIM_Control, BYTE_SWAP,             LITTLE_ENDIAN)|
		BCHP_FIELD_ENUM(CCE_PRIM_Control, SHIFT_DIRECTION,       LSB2MSB      )|
		BCHP_FIELD_DATA(CCE_PRIM_Control, reserved2,             0            )|
		BCHP_FIELD_ENUM(CCE_PRIM_Control, REGISTER_USE_MODE,     SPLIT        )|
		BCHP_FIELD_ENUM(CCE_PRIM_Control, ENABLE_CLOSED_CAPTION, ENABLED      )
	);

	/* Format register for PAL vs NTSC */
	ulControlReg &= ~BCHP_MASK ( CCE_PRIM_Control, VIDEO_FORMAT      );
	if ((eVideoFormat == BFMT_VideoFmt_eNTSC  ) ||
	    (eVideoFormat == BFMT_VideoFmt_eNTSC_J) ||
		(eVideoFormat == BFMT_VideoFmt_e720x482_NTSC) ||
		(eVideoFormat == BFMT_VideoFmt_e720x482_NTSC_J) ||

	    (eVideoFormat == BFMT_VideoFmt_ePAL_M )   )
	{
		ulControlReg |= BCHP_FIELD_ENUM(CCE_PRIM_Control, VIDEO_FORMAT, NTSC);
	}
	else /* eVideoFormat == BFMT_VideoFmt_ePAL_whatever */
	{
		ulControlReg |= BCHP_FIELD_ENUM(CCE_PRIM_Control, VIDEO_FORMAT, PAL );
	}

	/* Write the finished control register value, finally. */
	BREG_Write32 ( hReg, BCHP_CCE_PRIM_Control + ulCoreOffset, ulControlReg );

	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BVBI_P_CC_Enc_Program);
	return BERR_SUCCESS;
}

uint32_t BVBI_P_CC_Encode_Data_isr (
	BREG_Handle hReg,
    bool is656,
	uint8_t hwCoreIndex,
	BAVC_Polarity polarity,
	uint16_t usData)
{
	uint8_t  ucRegNum;
	uint16_t usWord0;
	uint32_t ulCoreOffset;
	uint32_t ulReg;
	uint32_t ulRegAddr;
	uint32_t ulErrInfo = 0;

	BDBG_ENTER(BVBI_P_CC_Encode_Data_isr);

	/* Get register offset */
	ulCoreOffset = P_GetCoreOffset_isr (is656, hwCoreIndex);
	if (ulCoreOffset == 0xFFFFFFFF)
	{
		/* This should never happen!  This parameter was checked by
		   BVBI_Encode_Create() */
		BDBG_LEAVE(BVBI_P_CC_Encode_Data_isr);
		return (uint32_t)(-1);
	}

	/* Choose register number */
	ucRegNum =  (uint8_t)(polarity == BAVC_Polarity_eTopField) ? 0 : 3;
	ulRegAddr =
		BCHP_CCE_PRIM_Data0 + ulCoreOffset + (sizeof(uint32_t) * ucRegNum);

	/* Get existing register value */
	ulReg =
		BREG_Read32 (hReg, ulRegAddr);

	/* Encode VBI data back into register value */
	usWord0 = BVBI_P_SetCCParityBits_isr (usData);
	ulReg &= ~BCHP_MASK       (CCE_PRIM_Data0, WORD0         );
	ulReg |=  BCHP_FIELD_DATA (CCE_PRIM_Data0, WORD0, usWord0);

	/* Write new register value */
	BREG_Write32 (hReg, ulRegAddr, ulReg);

	BDBG_LEAVE(BVBI_P_CC_Encode_Data_isr);
	return ulErrInfo;
}


/***************************************************************************
 *
 */
BERR_Code BVBI_P_CC_Encode_Enable_isr (
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

	BDBG_ENTER(BVBI_P_CC_Encode_Enable_isr);

	/* Figure out which encoder core to use */
	ulCoreOffset = P_GetCoreOffset_isr (is656, hwCoreIndex);
	if (ulCoreOffset == 0xFFFFFFFF)
	{
		/* This should never happen!  This parameter was checked by
		   BVBI_Encode_Create() */
		BDBG_LEAVE(BVBI_P_CC_Encode_Enable_isr);
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

    ulControlReg = BREG_Read32 ( hReg, BCHP_CCE_PRIM_Control + ulCoreOffset );
	ulControlReg &=
		~BCHP_MASK (CCE_PRIM_Control, ENABLE_CLOSED_CAPTION);
	if (bEnable)
	{
		ulControlReg |=
			BCHP_FIELD_ENUM (CCE_PRIM_Control, ENABLE_CLOSED_CAPTION, ENABLED);
	}
	else
	{
		ulControlReg |=
			BCHP_FIELD_ENUM (CCE_PRIM_Control, ENABLE_CLOSED_CAPTION, DISABLED);
	}
	BREG_Write32 ( hReg, BCHP_CCE_PRIM_Control + ulCoreOffset, ulControlReg );

	BDBG_LEAVE(BVBI_P_CC_Encode_Enable_isr);
	return BERR_SUCCESS;
}


/***************************************************************************
* Static (private) functions
***************************************************************************/

/***************************************************************************
 *
 */
static uint8_t BVBI_P_SetOddParityBit_isr( uint8_t uchByte )
{
    uint8_t uchOriginalByte = uchByte;
    uint8_t uchParity       = 0;
    uint8_t i               = 0;

    /* traverse through all 7 bits to count the number of 1's */
    for (i = 0; i < 7; ++i)
    {
        /* sum current bit and shift */
        uchParity = (uint8_t)(uchParity + (uchByte & 0x1));
        uchByte >>= 1;
    }

    /* return parity with original bits */
    return  (uint8_t)(((uchParity & 0x1) ? 0x00 : 0x80) |
        ((uint8_t)(uchOriginalByte & 0x7F)));
}

/***************************************************************************
 *
 */
static uint32_t P_GetCoreOffset_isr (bool is656, uint8_t hwCoreIndex)
{
	uint32_t ulCoreOffset = 0xFFFFFFFF;

	if (is656)
	{
#if (BVBI_NUM_CCE_656 >= 1)
		ulCoreOffset = (BCHP_CCE_656_Ancil_RevID - BCHP_CCE_PRIM_RevID);
#endif
	}
	else
	{
		switch (hwCoreIndex)
		{
#if (BVBI_NUM_CCE >= 1)
		case 0:
			ulCoreOffset = 0;
			break;
#endif
#if (BVBI_NUM_CCE >= 2)
		case 1:
			ulCoreOffset = (BCHP_CCE_SEC_RevID - BCHP_CCE_PRIM_RevID);
			break;
#endif
#if (BVBI_NUM_CCE >= 3)
		case 2:
			ulCoreOffset = (BCHP_CCE_TERT_RevID - BCHP_CCE_PRIM_RevID);
			break;
#endif
		default:
			break;
		}
	}

	return ulCoreOffset;
}

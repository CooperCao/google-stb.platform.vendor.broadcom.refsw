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
static uint32_t P_GetCoreOffset_isr (bool is656, uint8_t hwCoreIndex);


/***************************************************************************
* Implementation supporting closed caption functions that are not in API
***************************************************************************/

BERR_Code BVBI_P_MCC_Enc_Program (
	BREG_Handle hReg,
	bool is656,
	uint8_t hwCoreIndex,
	bool bActive,
	BFMT_VideoFmt eVideoFormat,
	bool bArib480p)
{
/*
	Programming note: the implementation here assumes that the bitfield layout
	within registers is the same for all CC encoder cores in the chip.

	If a chip is built that has multiple CC encoder cores that are not
	identical, then this routine will have to be redesigned.
*/
	uint32_t ulCoreOffset;
	uint32_t ulControlReg;
	uint32_t ulGainDelayReg = 0;

	BDBG_ENTER(BVBI_P_MCC_Enc_Program);

	BSTD_UNUSED (bArib480p);

	/* Figure out which encoder core to use */
	ulCoreOffset = P_GetCoreOffset_isr (is656, hwCoreIndex);
	if (ulCoreOffset == 0xFFFFFFFF)
	{
		/* This should never happen!  This parameter was checked by
		   BVBI_Encode_Create() */
		BDBG_LEAVE(BVBI_P_MCC_Enc_Program);
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
		BDBG_LEAVE(BVBI_P_MCC_Enc_Program);
		return BERR_SUCCESS;
	}

	BKNI_EnterCriticalSection();

	/* Program the Gain-Delay register, if any. */
	if (!is656)
	{
		ulGainDelayReg = BREG_Read32 (
			hReg, BCHP_CCE_PRIM_Gain_Delay + ulCoreOffset );
	}

	/* Select video format */
	switch (eVideoFormat)
	{
    case BFMT_VideoFmt_eNTSC:
    case BFMT_VideoFmt_eNTSC_J:
	case BFMT_VideoFmt_e720x482_NTSC:
	case BFMT_VideoFmt_e720x482_NTSC_J:
        /* NTSC specific settings */

		/* set the format specific gain delay values, if any. */
		if (!is656)
		{
			ulGainDelayReg &= ~(
				BCHP_MASK       ( CCE_PRIM_Gain_Delay, GAIN             ) |
				BCHP_MASK       ( CCE_PRIM_Gain_Delay, DELAY_COUNT      )   );
			ulGainDelayReg |= (
				BCHP_FIELD_DATA ( CCE_PRIM_Gain_Delay, GAIN,        0x47) |
				BCHP_FIELD_DATA ( CCE_PRIM_Gain_Delay, DELAY_COUNT, 0x3A)   );
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
    case BFMT_VideoFmt_ePAL_M:
    case BFMT_VideoFmt_ePAL_N:
    case BFMT_VideoFmt_ePAL_NC:
    case BFMT_VideoFmt_eSECAM_L:
    case BFMT_VideoFmt_eSECAM_B:
    case BFMT_VideoFmt_eSECAM_G:
    case BFMT_VideoFmt_eSECAM_D:
    case BFMT_VideoFmt_eSECAM_K:
    case BFMT_VideoFmt_eSECAM_H:
		/* set the format specific gain delay values, if any. */
		if (!is656)
		{
			ulGainDelayReg &= ~(
				BCHP_MASK       ( CCE_PRIM_Gain_Delay, GAIN             ) |
				BCHP_MASK       ( CCE_PRIM_Gain_Delay, DELAY_COUNT      )   );
			ulGainDelayReg |= (
				BCHP_FIELD_DATA ( CCE_PRIM_Gain_Delay, GAIN,        0x46) |
				BCHP_FIELD_DATA ( CCE_PRIM_Gain_Delay, DELAY_COUNT, 0x01)   );
		}
		break;

	default:
		BKNI_LeaveCriticalSection();
		BDBG_LEAVE(BVBI_P_MCC_Enc_Program);
		BDBG_ERR(("This video format not supported"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
		break;
	}

	/* write the gain delay register, if any. */
	if (!is656)
	{
		BREG_Write32 (
			hReg, BCHP_CCE_PRIM_Gain_Delay + ulCoreOffset, ulGainDelayReg );
	}

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
	if ((eVideoFormat == BFMT_VideoFmt_eNTSC) ||
	    (eVideoFormat == BFMT_VideoFmt_eNTSC_J) ||
		(eVideoFormat == BFMT_VideoFmt_e720x482_NTSC) ||
		(eVideoFormat == BFMT_VideoFmt_e720x482_NTSC_J))

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

	BDBG_LEAVE(BVBI_P_MCC_Enc_Program);
	return BERR_SUCCESS;
}

uint32_t BVBI_P_MCC_Encode_Data_isr (
	BREG_Handle hReg,
	bool is656,
	uint8_t hwCoreIndex,
	BFMT_VideoFmt eVideoFormat,
	bool bArib480p,
	BAVC_Polarity polarity,
	bool bPR18010_bad_line_number,
	BVBI_MCCData* pMCCData)
{
	int iLine;
	unsigned int numSet;
	uint8_t *datum;
	uint8_t  ucRegNum;
	uint16_t usWord;
	uint32_t ulCoreOffset;
	uint32_t ulDataReg;
	uint32_t ulDataRegAddr;
	uint32_t ulLinesReg;
	uint32_t lineMask;
	uint32_t ulErrInfo = 0;

	BDBG_ENTER(BVBI_P_MCC_Encode_Data_isr);

#if (BVBI_NUM_CCE_656 >= 1)
#else
	BSTD_UNUSED (bPR18010_bad_line_number);
#endif

	/* Get register offset */
	ulCoreOffset = P_GetCoreOffset_isr (is656, hwCoreIndex);
	if (ulCoreOffset == 0xFFFFFFFF)
	{
		/* This should never happen!  This parameter was checked by
		   BVBI_Encode_Create() */
		BDBG_LEAVE(BVBI_P_MCC_Encode_Data_isr);
		return (uint32_t)(-1);
	}

	/* Choose first data register */
	ucRegNum =  (uint8_t)(polarity == BAVC_Polarity_eTopField) ? 0 : 3;
	ulDataRegAddr =
		BCHP_CCE_PRIM_Data0 + ulCoreOffset + (sizeof(uint32_t) * ucRegNum);

	/* Prepare to index into user data */
	datum = pMCCData->ucData;
	numSet = 0;
	lineMask = pMCCData->uhLineMask;
	/* PAL-M always seems to cause problems */
	if (eVideoFormat == BFMT_VideoFmt_ePAL_M)
		lineMask >>= 3;

	if (bArib480p)
	{
		if ((eVideoFormat == BFMT_VideoFmt_eNTSC)   ||
			(eVideoFormat == BFMT_VideoFmt_eNTSC_J) ||
			(eVideoFormat == BFMT_VideoFmt_e720x482_NTSC) ||
			(eVideoFormat == BFMT_VideoFmt_e720x482_NTSC_J))
		{
			lineMask >>= 1;
		}
	}

	/* Loop over possible video lines */
	for (iLine = 0 ; iLine < 16 ; ++iLine)
	{
		/* If current video line is selected */
		if (lineMask & 0x1)
		{
			/* Get existing data register value */
			ulDataReg = BREG_Read32 (hReg, ulDataRegAddr);

			/* Encode VBI data back into register value */
			usWord  = *datum++;
			usWord |= ((uint16_t)(*datum++) << 8);
			usWord = BVBI_P_SetCCParityBits_isr (usWord);
			if (numSet & 0x1)
			{
				ulDataReg &= ~BCHP_MASK       (CCE_PRIM_Data0, WORD1        );
				ulDataReg |=  BCHP_FIELD_DATA (CCE_PRIM_Data0, WORD1, usWord);
			}
			else
			{
				ulDataReg &= ~BCHP_MASK       (CCE_PRIM_Data0, WORD0        );
				ulDataReg |=  BCHP_FIELD_DATA (CCE_PRIM_Data0, WORD0, usWord);
			}

			/* Write the data register back */
			BREG_Write32 (hReg, ulDataRegAddr, ulDataReg);

			/* Advance counters */
			if (numSet & 0x1)
			{
				++ucRegNum;
				ulDataRegAddr =
					BCHP_CCE_PRIM_Data0 + ulCoreOffset +
						(sizeof(uint32_t) * ucRegNum);
			}
			++numSet;

			/* Sanity check */
			if (numSet > 6)
				break;
		}

		/* Advance */
		lineMask >>= 1;
	}

	/* Adjust line mask for hardware oddities */
	lineMask = pMCCData->uhLineMask;
	/* PAL-M always seems to cause problems */
	if (eVideoFormat == BFMT_VideoFmt_ePAL_M)
		lineMask >>= 3;
#if (BVBI_NUM_CCE_656 >= 1)
	if (is656)
	{
		if (bPR18010_bad_line_number)
			lineMask >>= 3;
		else
			lineMask >>= 1;
	}
#endif
	if (bArib480p)
	{
		if ((eVideoFormat == BFMT_VideoFmt_eNTSC)   ||
			(eVideoFormat == BFMT_VideoFmt_eNTSC_J) ||
			(eVideoFormat == BFMT_VideoFmt_e720x482_NTSC) ||
			(eVideoFormat == BFMT_VideoFmt_e720x482_NTSC_J))
		{
			lineMask >>= 1;
		}
	}

	/* Finally, set active lines register. */
	ulLinesReg =
		BREG_Read32 ( hReg,  BCHP_CCE_PRIM_Active_Lines + ulCoreOffset );
	switch (polarity)
	{
	case BAVC_Polarity_eTopField:
		ulLinesReg &= ~BCHP_MASK       ( CCE_PRIM_Active_Lines, TOP_ACTIVE);
		ulLinesReg |=  BCHP_FIELD_DATA ( CCE_PRIM_Active_Lines, TOP_ACTIVE,
										 lineMask );
			break;
	case BAVC_Polarity_eBotField:
		ulLinesReg &= ~BCHP_MASK       ( CCE_PRIM_Active_Lines, BOT_ACTIVE);
		ulLinesReg |=  BCHP_FIELD_DATA ( CCE_PRIM_Active_Lines, BOT_ACTIVE,
										 lineMask );
			break;
		default:
			BDBG_LEAVE(BVBI_P_MCC_Encode_Data_isr);
			return (uint32_t)(-1);
			break;
	}
	BREG_Write32 (
		hReg, BCHP_CCE_PRIM_Active_Lines + ulCoreOffset, ulLinesReg );

	BDBG_LEAVE(BVBI_P_MCC_Encode_Data_isr);
	return ulErrInfo;
}

/***************************************************************************
 *
 */
BERR_Code BVBI_P_MCC_Encode_Enable_isr (
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

	BDBG_ENTER(BVBI_P_MCC_Encode_Enable_isr);

	/* Figure out which encoder core to use */
	ulCoreOffset = P_GetCoreOffset_isr (is656, hwCoreIndex);
	if (ulCoreOffset == 0xFFFFFFFF)
	{
		/* This should never happen!  This parameter was checked by
		   BVBI_Encode_Create() */
		BDBG_LEAVE(BVBI_P_MCC_Encode_Enable_isr);
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

	BDBG_LEAVE(BVBI_P_MCC_Encode_Enable_isr);
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

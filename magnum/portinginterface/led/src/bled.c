/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/
#include "bstd.h"
#include "bled.h"
#include "bchp_ldk.h"

#if (HLCD_SUPPORT==1)
#include "bchp_aon_ctrl.h"
#endif

BDBG_MODULE(bled);

#define BLED_CHK_RETCODE( rc, func )        \
do {                                        \
	if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
	{                                       \
		goto done;                          \
	}                                       \
} while(0)

/* Default values */
#define LED_DEFAULT_PRESCALE_HI     0x00
#define LED_DEFAULT_PRESCALE_LO     0x55
#define LED_DEFAULT_DUTYCYCLE_OFF   0x01
#define LED_DEFAULT_DUTYCYCLE_ON    0xAA
#define LED_DEFAULT_DEBOUNCE        0x40

/*******************************************************************************
*
*   Private Module Handles
*
*******************************************************************************/

BDBG_OBJECT_ID(BLED_Handle);

typedef struct BLED_P_Handle
{
	uint32_t        magicId;                    /* Used to check if structure is corrupt */
    BDBG_OBJECT(BLED_Handle)
	BCHP_Handle     hChip;
	BREG_Handle     hRegister;
} BLED_P_Handle;

/*******************************************************************************
*
*   Default Module Settings
*
*******************************************************************************/
static const BLED_Settings defLedSettings =
{
	100                                 /* percent brightness */
};

/*******************************************************************************
*
*   Public Module Functions
*
*******************************************************************************/
BERR_Code BLED_Open(
	BLED_Handle *pLed,                  /* [output] Returns handle */
	BCHP_Handle hChip,                  /* Chip handle */
	BREG_Handle hRegister,              /* Register handle */
	const BLED_Settings *pDefSettings   /* Default settings */
	)
{
	BERR_Code       retCode = BERR_SUCCESS;
	BLED_Handle     hDev;
	uint32_t        lval;

	/* Sanity check on the handles we've been given. */
	BDBG_ASSERT( hChip );
	BDBG_ASSERT( hRegister );

	/* Alloc memory from the system heap */
	hDev = (BLED_Handle) BKNI_Malloc( sizeof( BLED_P_Handle ) );
	if( hDev == NULL )
	{
		*pLed = NULL;
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BLED_Open: BKNI_malloc() failed"));
		goto done;
	}

    BDBG_OBJECT_SET(hDev, BLED_Handle);
	hDev->hChip     = hChip;
	hDev->hRegister = hRegister;
	*pLed = hDev;

	/* Reset LED/Keypad core */
	lval = BREG_Read32 (hDev->hRegister, BCHP_LDK_CONTROL);
	lval |= BCHP_LDK_CONTROL_swr_MASK;
	lval &= 0x0f;                       /* only keep lower 4 bits */
	BREG_Write32 (hDev->hRegister, BCHP_LDK_CONTROL, lval);

	lval &= ~BCHP_LDK_CONTROL_swr_MASK;
	lval |=  BCHP_LDK_CONTROL_ver_MASK;
	BREG_Write32 (hDev->hRegister, BCHP_LDK_CONTROL, lval);

	/* Set up LED */
	BREG_Write32 (hDev->hRegister, BCHP_LDK_PRESCHI, LED_DEFAULT_PRESCALE_HI);
	BREG_Write32 (hDev->hRegister, BCHP_LDK_PRESCLO, LED_DEFAULT_PRESCALE_LO);

	BLED_AdjustBrightness (hDev, pDefSettings->percentBrightness);

done:
	return( retCode );
}

BERR_Code BLED_Close(
	BLED_Handle hDev                    /* Device handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hDev, BLED_Handle);

    BDBG_OBJECT_DESTROY(hDev, BLED_Handle);
	BKNI_Free( (void *) hDev );

	return( retCode );
}

BERR_Code BLED_GetDefaultSettings(
	BLED_Settings *pDefSettings,        /* [output] Returns default setting */
	BCHP_Handle hChip                   /* Chip handle */
)
{
	BERR_Code retCode = BERR_SUCCESS;

	BSTD_UNUSED(hChip);

	*pDefSettings = defLedSettings;

	return( retCode );
}

BERR_Code BLED_Write (
	BLED_Handle         hLed,           /* Device handle */
	uint8_t             digit,          /* digit to write to */
	uint16_t            value           /* value to write */
)
{
	uint32_t    offset;

	switch (digit)
	{
		case 1:
			offset = BCHP_LDK_DIGIT1;
			break;

		case 2:
			offset = BCHP_LDK_DIGIT2;
			break;

		case 3:
			offset = BCHP_LDK_DIGIT3;
			break;

		case 4:
			offset = BCHP_LDK_DIGIT4;
			break;

		default:
			return BERR_INVALID_PARAMETER;
	}

	BREG_Write32 (hLed->hRegister, offset, (uint32_t)value);

	return BERR_SUCCESS;
}

BERR_Code BLED_AdjustBrightness (
	BLED_Handle         hLed,               /* Device handle */
	uint8_t             percentBrightness   /* percent of brightness */
)
{
	uint8_t             ucDutyCycleOn;
	uint8_t             ucDutyCycleOff;
	uint32_t            dutyCycleClks;
	uint32_t            valueOn;

	dutyCycleClks   = LED_DEFAULT_DUTYCYCLE_ON + LED_DEFAULT_DUTYCYCLE_OFF;

	valueOn         = dutyCycleClks * percentBrightness / 100;
	ucDutyCycleOn   = (uint8_t)valueOn;
	ucDutyCycleOff  = dutyCycleClks - ucDutyCycleOn;

	BREG_Write32 (hLed->hRegister, BCHP_LDK_DUTYOFF, (uint32_t)ucDutyCycleOff);
	BREG_Write32 (hLed->hRegister, BCHP_LDK_DUTYON,  (uint32_t)ucDutyCycleOn);

	return BERR_SUCCESS;
}

BERR_Code BLED_SetDiscreteLED (
	BLED_Handle         hLed,           /* Device handle */
	bool                on,             /* turn on or off */
	uint8_t             ledStatusBit    /* bit to turn on or off */
)
{
	uint32_t            lval;

	if (ledStatusBit > 7)
		return BERR_INVALID_PARAMETER;

	lval = BREG_Read32 (hLed->hRegister, BCHP_LDK_STATUS);
	if (on)
		lval &= ~(1 << ledStatusBit);
	else
		lval |= (1 << ledStatusBit);

	BREG_Write32 (hLed->hRegister, BCHP_LDK_STATUS, lval);

	return BERR_SUCCESS;
}

#if (HLCD_SUPPORT==1)
BERR_Code BLED_StartHLCD (
	BLED_Handle         hLed,           /* Device handle */
	uint8_t             hour,           /* used to initialize the hour counter */
	uint8_t             min,            /* used to initialize the minute counter */
	uint8_t             sec,            /* used to initialize the second counter */
	bool                hour_mode,      /* 1 count hours from 0 to 23. 0 count hours from 0 to 11 */
	bool                display_mode    /* 0: in 12-hour mode: display from 1 to 12; in 24-hour mode: display from 0 to 23 */
										/* 1: in 12-hour mode: display from 0 to 11; in 24-hour mode: display from 1 to 24 */
)
{
	uint32_t            lval;

    BDBG_OBJECT_ASSERT( hLed, BLED_Handle);

	if (hour_mode)
	{
		if (hour >= 24)
		{
			return BERR_INVALID_PARAMETER;
		}
	}
	else
	{
		if (hour >= 12)
		{
			return BERR_INVALID_PARAMETER;
		}
	}

	if (min >= 60)
	{
		return BERR_INVALID_PARAMETER;
	}

	if (sec >= 60)
	{
		return BERR_INVALID_PARAMETER;
	}
	lval = 0;
	lval |= (hour << BCHP_AON_CTRL_TIME_COUNTER_hour_counter_init_SHIFT);
	lval |= (min << BCHP_AON_CTRL_TIME_COUNTER_minute_counter_init_SHIFT);
	lval |= (sec << BCHP_AON_CTRL_TIME_COUNTER_second_counter_init_SHIFT);
	lval |= (hour_mode << BCHP_AON_CTRL_TIME_COUNTER_mode_12h_24h_init_SHIFT);
	lval |= (display_mode << BCHP_AON_CTRL_TIME_COUNTER_hour_display_mode_SHIFT);
	BREG_Write32 (hLed->hRegister, BCHP_AON_CTRL_TIME_COUNTER, lval);

	/* Write digit code to the register*/
	lval = 0;
	lval |= (LED_NUM0 << BCHP_AON_CTRL_LED_DIGIT_CODE_0_digit_code_0_SHIFT);
	lval |= (LED_NUM1 << BCHP_AON_CTRL_LED_DIGIT_CODE_0_digit_code_1_SHIFT);
	lval |= (LED_NUM2 << BCHP_AON_CTRL_LED_DIGIT_CODE_0_digit_code_2_SHIFT);
	lval |= (LED_NUM3 << BCHP_AON_CTRL_LED_DIGIT_CODE_0_digit_code_3_SHIFT);
	BREG_Write32 (hLed->hRegister, BCHP_AON_CTRL_LED_DIGIT_CODE_0, lval);
	lval = 0;
	lval |= (LED_NUM4 << BCHP_AON_CTRL_LED_DIGIT_CODE_1_digit_code_4_SHIFT);
	lval |= (LED_NUM5 << BCHP_AON_CTRL_LED_DIGIT_CODE_1_digit_code_5_SHIFT);
	lval |= (LED_NUM6 << BCHP_AON_CTRL_LED_DIGIT_CODE_1_digit_code_6_SHIFT);
	lval |= (LED_NUM7 << BCHP_AON_CTRL_LED_DIGIT_CODE_1_digit_code_7_SHIFT);
	BREG_Write32 (hLed->hRegister, BCHP_AON_CTRL_LED_DIGIT_CODE_1, lval);
	lval = 0;
	lval |= (LED_NUM8 << BCHP_AON_CTRL_LED_DIGIT_CODE_2_digit_code_8_SHIFT);
	lval |= (LED_NUM9 << BCHP_AON_CTRL_LED_DIGIT_CODE_2_digit_code_9_SHIFT);
	BREG_Write32 (hLed->hRegister, BCHP_AON_CTRL_LED_DIGIT_CODE_2, lval);

	/*  program address offset of the hour/minute */
	lval = BREG_Read32 (hLed->hRegister, BCHP_AON_CTRL_LED_DIGIT_ADDR_OFFSET);
	lval &= ~BCHP_AON_CTRL_LED_DIGIT_ADDR_OFFSET_hour_msd_addr_offset_MASK;
	lval &= ~BCHP_AON_CTRL_LED_DIGIT_ADDR_OFFSET_hour_lsd_addr_offset_MASK;
	lval &= ~BCHP_AON_CTRL_LED_DIGIT_ADDR_OFFSET_minute_msd_addr_offset_MASK;
	lval &= ~BCHP_AON_CTRL_LED_DIGIT_ADDR_OFFSET_minute_lsd_addr_offset_MASK;
	lval |= (DIGIT1_OFFSET << BCHP_AON_CTRL_LED_DIGIT_ADDR_OFFSET_hour_msd_addr_offset_SHIFT);
	lval |= (DIGIT2_OFFSET << BCHP_AON_CTRL_LED_DIGIT_ADDR_OFFSET_hour_lsd_addr_offset_SHIFT);
	lval |= (DIGIT3_OFFSET << BCHP_AON_CTRL_LED_DIGIT_ADDR_OFFSET_minute_msd_addr_offset_SHIFT);
	lval |= (DIGIT4_OFFSET << BCHP_AON_CTRL_LED_DIGIT_ADDR_OFFSET_minute_lsd_addr_offset_SHIFT);
	BREG_Write32 (hLed->hRegister, BCHP_AON_CTRL_LED_DIGIT_ADDR_OFFSET, lval);

	/* enable hlcd by writing 0 and 1 to HLCD_CTRL*/
	BREG_Write32 (hLed->hRegister, BCHP_AON_CTRL_HLCD_CTRL, 0);
	BREG_Write32 (hLed->hRegister, BCHP_AON_CTRL_HLCD_CTRL, 1);

	return BERR_SUCCESS;
}


BERR_Code BLED_StopHLCD (
	BLED_Handle         hLed            /* Device handle */
)
{
	uint32_t        lval;
	uint8_t         cnt;
	BERR_Code       retCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT( hLed, BLED_Handle);

	/* Disable HLCD */
	BREG_Write32 (hLed->hRegister, BCHP_AON_CTRL_HLCD_CTRL, 0);

	/* wait until status becomes 0 */
	  for(cnt = 0; cnt < 10; cnt++)
	{
		lval = BREG_Read32 (hLed->hRegister, BCHP_AON_CTRL_HLCD_CTRL);

		if ((lval & BCHP_AON_CTRL_HLCD_CTRL_hlcd_enable_status_MASK) == 0)
			break;
		/* wait 1 ms */
		BKNI_Delay(1000);
	}

	if(cnt == 10)
	{
		retCode = BERR_TIMEOUT;
	}

	return retCode;
}
#endif

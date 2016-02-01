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


/*= Module Overview *********************************************************
<verbatim>

Overview
The LED PI module controls the LED display controller within the BCM7038-B0.  The
LED controller (along with the keypad controller) is not available in the
BCM7038-A0 version.

Design
The design for BLED PI API is broken into two parts.

o Part 1 (open/close/configuration):

  These APIs are used for opening and closing BLED device.
    
o Part 2 (write to LED display)

  These APIs are used to write to an LED digit or discrete LED.

Usage
The usage of BLED involves the following:

   * Configure/Open of BLED

      * Configure BLED device for the target system
      * Open BLED device

   * Write value to LED digit or discrete LED.

Sample Code
void main( void )
{
    BLED_Handle		hLED;
    BLED_Settings	defSettings;
    BREG_Handle		hReg;
    BCHP_Handle		hChip;

    // Do other initialization, i.e. for BREG, BCHP, etc

    // Now, ready to setup the BLED
    BLED_GetDefaultSettings( &defSettings, hChip );

    // Make any changes required from the default values
    defSettings.percentBrightness = 100;

    BLED_Open (&hLED, hChip, hReg, &defSettings);

	// Display "7038" on the front panel
	BLED_Write (hLED, 1, LED_NUM7);
	BLED_Write (hLED, 2, LED_NUM0);
	BLED_Write (hLED, 3, LED_NUM3);
	BLED_Write (hLED, 4, LED_NUM8);
}

</verbatim>
***************************************************************************/


#ifndef BLED_H__
#define BLED_H__

#include "bchp.h"
#include "breg_mem.h"
#include "bint.h"
#include "bkni.h"
#include "berr_ids.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
	LED segments and characters

Description:

See Also:

****************************************************************************/
#define LED_ON          0x80
#define LED_TOP_OFF     0x01
#define LED_RU_OFF      0x02 
#define LED_RL_OFF      0x04
#define LED_BOT_OFF     0x08
#define LED_LL_OFF      0x10
#define LED_LU_OFF      0x20
#define LED_MID_OFF     0x40

#define LED_A			(0x88)
#define LED_B			(0x80)
#define LED_C			(0xc6)
#define LED_D			(0xc0)
#define LED_E			(0x86)
#define LED_F			(0x8e)
#define LED_G			(0x82)
#define LED_H			(0x89)
#define LED_I			(0xf9)
#define LED_J			(0xf1)
#define LED_K			(0x89)
#define LED_L			(0xc7)
#define LED_M			(0x88)
#define LED_N			(0xc8)
#define LED_O			(0xc0)
#define LED_P			(0x8c)
#define LED_Q			(0xc0)
#define LED_R			(0x88)
#define LED_S			(0x92)
#define LED_T			(0xf8)
#define LED_U			(0xC1)
#define LED_V			(0xC1)
#define LED_W			(0x81)
#define LED_X			(0x89)
#define LED_Y			(0x91)
#define LED_Z			(0xa4)

#define LED_DASH		(0xbf)

#define LED_NUM0		(0xc0)
#define LED_NUM1		(0xf9)
#define LED_NUM2		(0xa4)
#define LED_NUM3		(0xb0)
#define LED_NUM4		(0x99)
#define LED_NUM5		(0x92)
#define LED_NUM6		(0x82)
#define LED_NUM7		(0xf8)
#define LED_NUM8		(0x80)
#define LED_NUM9		(0x98)

/* used by HLCD */
#define DIGIT1_OFFSET 	(0x1C)
#define DIGIT2_OFFSET 	(0x18)
#define DIGIT3_OFFSET 	(0x24)
#define DIGIT4_OFFSET 	(0x20)

/***************************************************************************
Summary:
	Error Codes specific to BLED

Description:

See Also:

****************************************************************************/

/***************************************************************************
Summary:
	The handle for led module.

Description:

See Also:
	BLED_Open()

****************************************************************************/
typedef struct BLED_P_Handle				*BLED_Handle;

/***************************************************************************
Summary:
	Required default settings structure for LED module.

Description:
	The default setting structure defines the default configure of
	LED when the device is opened.  

See Also:
	BLED_Open()

****************************************************************************/
typedef struct BLED_Settings
{
   uint8_t		percentBrightness;		/* percent brightness */
} BLED_Settings;

/***************************************************************************
Summary:
	This function opens LED module.

Description:
	This function is responsible for opening BLED module. When BLED is
	opened, it will create a module handle and configure the module based
	on the default settings. Once the device is opened, it must be closed
	before it can be opened again.

Returns:
	TODO:

See Also:
	BLED_Close()
	BLED_GetDefaultSettings()

****************************************************************************/
BERR_Code BLED_Open(
	BLED_Handle *pLED,					/* [out] Returns handle */
	BCHP_Handle hChip,					/* [in] Chip handle */
	BREG_Handle hRegister,				/* [in] Register handle */
	const BLED_Settings *pDefSettings	/* [in] Default settings */
	);

/***************************************************************************
Summary:
	This function closes LED module.

Description:
	This function is responsible for closing BLED module. Closing BLED 
	will free main BLED handle. 

Returns:
	TODO:

See Also:
	BLED_Open()

****************************************************************************/
BERR_Code BLED_Close(
	BLED_Handle hDev					/* [in] Device handle */
	);

/***************************************************************************
Summary:
	This function returns the default settings for LED module.

Description:
	This function is responsible for returns the default setting for 
	BLED module. The returning default setting should be when
	opening the device.

Returns:
	TODO:

See Also:
	BLED_Open()

****************************************************************************/
BERR_Code BLED_GetDefaultSettings(
	BLED_Settings *pDefSettings,		/* [out] Returns default setting */
	BCHP_Handle hChip					/* [in] Chip handle */
	);

/***************************************************************************
Summary:
	This function writes an LED digit

Description:
	This function is used to write a character to an LED digit.
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
BERR_Code BLED_Write ( 
	BLED_Handle 		hLed, 			/* [in] Device handle */
	uint8_t				digit,			/* [in] digit to write to */
	uint8_t				value			/* [in] value to write */
	);

/***************************************************************************
Summary:
	This function adjusts the brightness of LED.

Description:
	This function is used to adjust LED brightness.  The caller specifies
	the percentage of brightness from (0-100).
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
BERR_Code BLED_AdjustBrightness ( 
	BLED_Handle 		hLed,				/* [in] Device handle */
	uint8_t				percentBrightness	/* [in] percent of brightness */
	);

/***************************************************************************
Summary:
	This function writes to a discrete LED

Description:
	This function is used to turn on or off a discrete LED
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
BERR_Code BLED_SetDiscreteLED ( 
	BLED_Handle 		hLed, 			/* [in] Device handle */
	bool				on,				/* [in] turn on or off */
	uint8_t				ledStatusBit	/* [in] bit to turn on or off */
	);

/***************************************************************************
Summary:
	This function starts HLCD

Description:
	This function is used to start HLCD
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
BERR_Code BLED_StartHLCD ( 
	BLED_Handle 		hLed, 			/* Device handle */
	uint8_t				hour,			/* used to initialize the hour counter */
	uint8_t				min,			/* used	to initialize the minute counter */
	uint8_t				sec,			/* used to initialize the second counter */
	bool				hour_mode,		/* 1 count hours from 0 to 23. 0 count hours from 0 to 11 */
	bool				display_mode	/* 0: in 12-hour mode: display from 1 to 12; in 24-hour mode: display from 0 to 23 */
										/* 1: in 12-hour mode: display from 0 to 11; in 24-hour mode: display from 1 to 24 */
);

/***************************************************************************
Summary:
	This function stops HLCD

Description:
	This function is used to stop HLCD
		
Returns:
	TODO:

See Also:
	

****************************************************************************/
BERR_Code BLED_StopHLCD ( 
	BLED_Handle 		hLed 			/* Device handle */
);

#ifdef __cplusplus
}
#endif
 
#endif




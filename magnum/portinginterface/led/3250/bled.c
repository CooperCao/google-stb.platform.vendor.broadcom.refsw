/***************************************************************************
 *     Copyright (c) 2003-2005, Broadcom Corporation
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
#include "bstd.h"
#include "bled.h"
#include "bchp_qam.h"


BDBG_MODULE(bled);

#define	DEV_MAGIC_ID			((BERR_LED_ID<<16) | 0xFACE)

#define	BLED_CHK_RETCODE( rc, func )		\
do {										\
	if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
	{										\
		goto done;							\
	}										\
} while(0)

/* Default values */
#define LED_DEFAULT_PRESCALE_HI		0x00
#define LED_DEFAULT_PRESCALE_LO		0x55
#define LED_DEFAULT_DUTYCYCLE_OFF	0x01
#define LED_DEFAULT_DUTYCYCLE_ON	0xAA
#define LED_DEFAULT_DEBOUNCE		0x40

/*******************************************************************************
*
*	Private Module Handles
*
*******************************************************************************/

typedef struct BLED_P_Handle
{
	uint32_t		magicId;					/* Used to check if structure is corrupt */
	BCHP_Handle 	hChip;
	BREG_Handle		hRegister;
} BLED_P_Handle;

/*******************************************************************************
*
*	Default Module Settings
*
*******************************************************************************/
static const BLED_Settings defLedSettings = 
{
	100									/* percent brightness */
};

/*******************************************************************************
*
*	Public Module Functions
*
*******************************************************************************/
BERR_Code BLED_Open(
	BLED_Handle *pLed,					/* [output] Returns handle */
	BCHP_Handle hChip,					/* Chip handle */
	BREG_Handle hRegister,				/* Register handle */
	const BLED_Settings *pDefSettings	/* Default settings */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
 	BLED_Handle hDev;
 	uint8_t		bval;

	/* Sanity check on the handles we've been given. */
	BDBG_ASSERT( hChip );
	BDBG_ASSERT( hRegister );

	/* Alloc memory from the system heap */
	hDev = (BLED_Handle) BKNI_Malloc( sizeof( BLED_P_Handle ) );
	if( hDev == NULL )
	{
		*pLed = NULL;
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BLED_Open: BKNI_malloc() failed\n"));
		goto done;
	}

	hDev->magicId	= DEV_MAGIC_ID;
	hDev->hChip		= hChip;
	hDev->hRegister = hRegister;
	*pLed = hDev;

	/* Reset LED/Keypad core */
	bval = BREG_Read8 (hDev->hRegister, (BCM3250_UPG_CORE_OFFSET + LED_IRQEN));
	bval |= LED_IRQEN_SWR_MASK;
	BREG_Write8 (hDev->hRegister, (BCM3250_UPG_CORE_OFFSET + LED_IRQEN), bval);
	
	/* Unreset it */
	bval = BREG_Read8 (hDev->hRegister, (BCM3250_UPG_CORE_OFFSET + LED_IRQEN));
	bval &= ~LED_IRQEN_SWR_MASK;
	BREG_Write8 (hDev->hRegister, (BCM3250_UPG_CORE_OFFSET + LED_IRQEN), bval);

	/* Set up LED */
	BREG_Write8 (hDev->hRegister, (BCM3250_UPG_CORE_OFFSET + LED_PRESCHI), LED_DEFAULT_PRESCALE_HI);
	BREG_Write8 (hDev->hRegister, (BCM3250_UPG_CORE_OFFSET + LED_PRESCLO), LED_DEFAULT_PRESCALE_LO);

	BLED_AdjustBrightness (hDev, pDefSettings->percentBrightness);

done:
	return( retCode );
}

BERR_Code BLED_Close(
	BLED_Handle hDev					/* Device handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;


	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	BKNI_Free( (void *) hDev );

	return( retCode );
}

BERR_Code BLED_GetDefaultSettings(
	BLED_Settings *pDefSettings,		/* [output] Returns default setting */
	BCHP_Handle hChip					/* Chip handle */
)
{
	BERR_Code retCode = BERR_SUCCESS;
	BSTD_UNUSED(hChip);

	*pDefSettings = defLedSettings;

	return( retCode );
}

BERR_Code BLED_Write ( 
	BLED_Handle 		hLed, 			/* Device handle */
	uint8_t				digit,			/* digit to write to */
	uint8_t				value			/* value to write */
)
{
	uint32_t	offset;
	
	switch (digit)
	{
		case 1:
			offset = LED_DIGIT1;
			break;

		case 2:
			offset = LED_DIGIT2;
			break;
			
		case 3:
			offset = LED_DIGIT3;
			break;

		case 4:
			offset = LED_DIGIT4;
			break;
			
		default:
			return BERR_INVALID_PARAMETER;
	}
	
	BREG_Write8 (hLed->hRegister, (BCM3250_UPG_CORE_OFFSET + offset), value);

	return BERR_SUCCESS;
}

BERR_Code BLED_AdjustBrightness ( 
	BLED_Handle 		hLed,				/* Device handle */
	uint8_t				percentBrightness	/* percent of brightness */
)
{
	uint8_t				ucDutyCycleOn;
	uint8_t				ucDutyCycleOff;
	uint32_t			dutyCycleClks;
	uint32_t			valueOn;

	dutyCycleClks 	= LED_DEFAULT_DUTYCYCLE_ON + LED_DEFAULT_DUTYCYCLE_OFF;

	valueOn 		= dutyCycleClks * percentBrightness / 100;
	ucDutyCycleOn 	= (uint8_t)valueOn;
	ucDutyCycleOff 	= dutyCycleClks - ucDutyCycleOn;

	BREG_Write8 (hLed->hRegister, (BCM3250_UPG_CORE_OFFSET + LED_DUTYOFF), ucDutyCycleOff);
	BREG_Write8 (hLed->hRegister, (BCM3250_UPG_CORE_OFFSET + LED_DUTYON), ucDutyCycleOn);

	return BERR_SUCCESS;
}

BERR_Code BLED_SetDiscreteLED ( 
	BLED_Handle 		hLed, 			/* Device handle */
	bool				on,				/* turn on or off */
	uint8_t				ledStatusBit	/* bit to turn on or off */
)
{
	uint8_t				bval;

	if (ledStatusBit > 7)
		return BERR_INVALID_PARAMETER;
		
	bval = BREG_Read8 (hLed->hRegister, (BCM3250_UPG_CORE_OFFSET + LED_STATUS));
	if (on)
		bval &= ~(1 << ledStatusBit);
	else
		bval |= (1 << ledStatusBit);

	BREG_Write8 (hLed->hRegister, (BCM3250_UPG_CORE_OFFSET + LED_STATUS), bval);

	return BERR_SUCCESS;
}

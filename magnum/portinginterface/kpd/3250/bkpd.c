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
#include "bkpd.h"
#include "bchp_qam.h"

BDBG_MODULE(bkpd);

#define	DEV_MAGIC_ID			((BERR_KPD_ID<<16) | 0xFACE)

#define	BKPD_CHK_RETCODE( rc, func )		\
do {										\
	if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
	{										\
		goto done;							\
	}										\
} while(0)

#define KPD_DEFAULT_PRESCALE		0x0055
#define KPD_DEFAULT_DUTYCYCLE_OFF	0x01
#define KPD_DEFAULT_DUTYCYCLE_ON	0xAA
#define KPD_DEFAULT_DEBOUNCE		0x40

static void BKPD_P_HandleInterrupt_Isr
(
	void *pParam1,						/* Device channel handle */
	int parm2							/* not used */
);

/*******************************************************************************
*
*	Private Module Handles
*
*******************************************************************************/

typedef struct BKPD_P_Handle
{
	uint32_t			magicId;					/* Used to check if structure is corrupt */
	BCHP_Handle 		hChip;
	BREG_Handle			hRegister;
	BINT_Handle 		hInterrupt;
	BKNI_EventHandle	hEvent;
	BINT_CallbackHandle hCallback;
	bool				intMode;
	BKPD_CallbackInfo	stCallbackInfo;
} BKPD_P_Handle;

/*******************************************************************************
*
*	Default Module Settings
*
*******************************************************************************/
static const BKPD_Settings defKpdSettings = 
{
	KPD_DEFAULT_PRESCALE,
	KPD_DEFAULT_DEBOUNCE,
	true
};

/*******************************************************************************
*
*	Public Module Functions
*
*******************************************************************************/
BERR_Code BKPD_Open(
	BKPD_Handle *pKpd,					/* [output] Returns handle */
	BCHP_Handle hChip,					/* Chip handle */
	BREG_Handle hRegister,				/* Register handle */
	BINT_Handle hInterrupt,				/* Interrupt handle */
	const BKPD_Settings *pDefSettings	/* Default settings */
	)
{
	BERR_Code		retCode = BERR_SUCCESS;
 	BKPD_Handle 	hDev;
	uint8_t			bval;
	uint16_t		sval;
	uint32_t		prescale;

	/* Sanity check on the handles we've been given. */
	BDBG_ASSERT( hChip );
	BDBG_ASSERT( hRegister );
	BDBG_ASSERT( hInterrupt );

	/* Alloc memory from the system heap */
	hDev = (BKPD_Handle) BKNI_Malloc( sizeof( BKPD_P_Handle ) );
	if( hDev == NULL )
	{
		*pKpd = NULL;
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BKPD_Open: BKNI_malloc() failed\n"));
		goto done;
	}

	hDev->magicId	= DEV_MAGIC_ID;
	hDev->hChip		= hChip;
	hDev->hRegister = hRegister;
	hDev->hInterrupt = hInterrupt;
	hDev->intMode = pDefSettings->intMode;

	BKPD_CHK_RETCODE( retCode, BKNI_CreateEvent( &(hDev->hEvent) ) );

	prescale = pDefSettings->prescale;
	BREG_Write8 (hDev->hRegister, (BCM3250_UPG_CORE_OFFSET + LED_PRESCHI), (uint8_t)(prescale >> 8));
	BREG_Write8 (hDev->hRegister, (BCM3250_UPG_CORE_OFFSET + LED_PRESCLO), (uint8_t)(prescale & 0xff));

	BREG_Write8 (hDev->hRegister, (BCM3250_UPG_CORE_OFFSET + LED_DEBOUNCE), pDefSettings->debounce);

	/* Read data to clear any pending interrupt */
	BKPD_Read (hDev, &sval);
	
	if (hDev->intMode)
	{
		BKPD_CHK_RETCODE( retCode, BINT_CreateCallback(
			&(hDev->hCallback), hDev->hInterrupt, BCHP_INT_ID_QAM_UPG_IRQ,
			BKPD_P_HandleInterrupt_Isr, (void *) hDev, 0x00 ) );
		BKPD_CHK_RETCODE( retCode, BINT_EnableCallback( hDev->hCallback ) );

		/* Enable keypad interrupt in UPG */
		bval = BREG_Read8 (hDev->hRegister, (BCM3250_UPG_CORE_OFFSET + UPG_IRQ_EN));
		bval |= UPG_IRQ_LDK_MASK;
		BREG_Write8 (hDev->hRegister, (BCM3250_UPG_CORE_OFFSET + UPG_IRQ_EN), bval);

		/* Enable keypad interrupt in led/keypad controller */
		bval = BREG_Read8 (hDev->hRegister, (BCM3250_UPG_CORE_OFFSET + LED_IRQEN));
		bval |= LED_IRQEN_IRQEN_MASK;
		BREG_Write8 (hDev->hRegister, (BCM3250_UPG_CORE_OFFSET + LED_IRQEN), bval);

		/* Clear user callback */
		hDev->stCallbackInfo.cbFunc = NULL;
	}

	*pKpd = hDev;

done:
	return( retCode );
}

BERR_Code BKPD_Close(
	BKPD_Handle hDev					/* Device handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;


	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	BKPD_CHK_RETCODE( retCode, BINT_DisableCallback( hDev->hCallback ) );
	BKPD_CHK_RETCODE( retCode, BINT_DestroyCallback( hDev->hCallback ) );
	BKNI_DestroyEvent( hDev->hEvent );
	BKNI_Free( (void *) hDev );

done:
	return( retCode );
}

BERR_Code BKPD_GetDefaultSettings(
	BKPD_Settings *pDefSettings,		/* [output] Returns default setting */
	BCHP_Handle hChip					/* Chip handle */
)
{
	BERR_Code retCode = BERR_SUCCESS;
	BSTD_UNUSED(hChip);

	*pDefSettings = defKpdSettings;

	return( retCode );
}

BERR_Code BKPD_GetEventHandle(
	BKPD_Handle		hDev,			/* Device handle */
	BKNI_EventHandle *phEvent		/* [output] Returns event handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;

	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	
	*phEvent = hDev->hEvent;

	return( retCode );
}

BERR_Code BKPD_IsDataReady ( 
	BKPD_Handle 		hKpd, 			/* Device handle */
	bool				*ready			/* flag indicating key is pressed */
)
{
	uint8_t			bval;

	bval = BREG_Read8 (hKpd->hRegister, (BCM3250_UPG_CORE_OFFSET + UPG_IRQ_STATUS));
	*ready = (bval & UPG_IRQ_LDK_MASK) ? true : false;

	return BERR_SUCCESS;
}

BERR_Code BKPD_Read ( 
	BKPD_Handle 		hKpd,			/* Device handle */
	uint16_t			*pData			/* pointer to data read from keypad */
)
{
	uint8_t				hi, lo;
	
	hi = BREG_Read8 (hKpd->hRegister, (BCM3250_UPG_CORE_OFFSET + LED_KEYROW32));
	lo = BREG_Read8 (hKpd->hRegister, (BCM3250_UPG_CORE_OFFSET + LED_KEYROW10));

	*pData = (((uint16_t)hi) << 8) | ((uint16_t)lo);		
	
	return BERR_SUCCESS;
}

BERR_Code BKPD_InstallInterruptCallback (
	BKPD_Handle 		hKpd,			/* Device handle */
	BKPD_CallbackFunc	callback,		/* callback function */
	void				*pParm1, 		/* application specified parameter */
	int					parm2 			/* application specified parameter */
)
{
	BDBG_ASSERT( hKpd );

	/* Put the interrupt callback into our array */
	hKpd->stCallbackInfo.cbFunc = callback;
	hKpd->stCallbackInfo.pParm1 = pParm1 ;
	hKpd->stCallbackInfo.parm2 = parm2 ;

	return BERR_SUCCESS;
}

BERR_Code BKPD_UnInstallInterruptCallback (
	BKPD_Handle 		hKpd			/* Device handle */
)
{
	BDBG_ASSERT( hKpd );

	/* Put the interrupt callback into our array */
	hKpd->stCallbackInfo.cbFunc = NULL;
	hKpd->stCallbackInfo.pParm1 = NULL ;
	hKpd->stCallbackInfo.parm2 = 0 ;

	return BERR_SUCCESS;
}

/*******************************************************************************
*
*	Private Module Functions
*
*******************************************************************************/
static void BKPD_P_HandleInterrupt_Isr
(
	void *pParam1,						/* Device channel handle */
	int parm2							/* not used */
)
{
	BKPD_Handle 		hDev;
	uint16_t			data;
	uint8_t				bval;

	BSTD_UNUSED(parm2);
	
	hDev = (BKPD_Handle) pParam1;
	BDBG_ASSERT( hDev );

	bval = BREG_Read8 (hDev->hRegister, (BCM3250_UPG_CORE_OFFSET + UPG_IRQ_STATUS));
	bval &= BREG_Read8 (hDev->hRegister, (BCM3250_UPG_CORE_OFFSET + UPG_IRQ_EN));
	if (bval & UPG_IRQ_LDK_MASK)
	{
		/* 
		 * Check to see if user has installed a callback
		 */
		if(hDev->stCallbackInfo.cbFunc)
		{
			hDev->stCallbackInfo.cbFunc (	hDev->stCallbackInfo.pParm1, 
											hDev->stCallbackInfo.parm2, 
											NULL) ;
		}

		/*
		 * Clear keypad interrupt by reading the key registers
		 */
		BKPD_Read (hDev, &data);

		BKNI_SetEvent( hDev->hEvent );
	}

	return;
}


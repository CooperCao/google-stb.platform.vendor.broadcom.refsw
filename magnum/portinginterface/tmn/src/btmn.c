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
#include "bstd.h"
#include "btmn.h"
#include "bchp_tmon.h"
#include "bchp_irq0.h"

#if BTMN_SUPPORTS_ONCHIPTEMP
#include "bchp_int_id_upg_aux_intr2.h"
#else
#include "bchp_int_id_mcif_intr2.h"
#endif

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

BDBG_MODULE(btmn);

#define	DEV_MAGIC_ID ((BERR_TMN_ID<<16) | 0xFACE)

#define	BTMN_CHK_RETCODE( rc, func )		\
do {										\
	if ( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
	{										\
		goto done;							\
	}										\
} while(0)

/* Forward references: */
static void BTMN_P_HandleDataReadyInterrupt_Isr (void *pParam1, int parm2);
static void BTMN_P_HandleOverTempInterrupt_Isr (void *pParam1, int parm2);

#define TREL_SCALE_FACTOR				10000

#define T_SCALE_FACTOR					100			/* scale factor for temp lookup table */
#define T_LUT_STARTING_TEMP				20			/* this is the first temp in the lut */

/*******************************************************************************
*
*	Private Module Handles
*
*******************************************************************************/

typedef struct BTMN_P_Handle
{
	uint32_t			magicId;					/* Used to check if structure is corrupt */
	BCHP_Handle 		hChip;
	BREG_Handle			hRegister;
	BINT_Handle 		hInterrupt;
	BKNI_EventHandle	hDataReadyEvent;			/* data ready event */
	BKNI_EventHandle	hOverTempEvent;				/* over temp event */
	uint32_t			top;						/* register value read during interrupt */
	uint32_t			bottom;						/* register value read during interrupt */
	uint32_t			mid1;						/* register value read during interrupt */
	uint32_t			mid0;						/* register value read during interrupt */
#if BTMN_SUPPORTS_ONCHIPTEMP
	uint32_t			vctat;						/* register value read during interrupt */
#endif

	/*
	 * Interrupt stuff
	 */
	BINT_CallbackHandle hDataReadyCB;				/* Data Ready interrupt */
	BINT_CallbackHandle hOverTempCB;				/* Over Temp interrupt */
	BTMN_CallbackInfo	stDataReadyUserCB;			/* Data Ready user callback */
	BTMN_CallbackInfo	stOverTempUserCB;			/* Over Temp user callback */
} BTMN_P_Handle;

typedef struct t_lut
{
	uint32_t			temp;
	uint32_t			ratio;
} t_lut;

/*******************************************************************************
*
*	Default Module Settings
*
*******************************************************************************/
static const BTMN_Settings defTmnSettings = 
{
	10,									/* measurement interval */
};

/*******************************************************************************
*
*	Public Module Functions
*
*******************************************************************************/
BERR_Code BTMN_Open(
	BTMN_Handle *pTmn,					/* [output] Returns handle */
	BCHP_Handle hChip,					/* Chip handle */
	BREG_Handle hRegister,				/* Register handle */
	BINT_Handle hInterrupt,				/* Interrupt handle */
	const BTMN_Settings *pDefSettings	/* Default settings */
	)
{
	BERR_Code 		retCode = BERR_SUCCESS;
 	BTMN_Handle		hDev;
#if BTMN_SUPPORTS_ONCHIPTEMP
	uint32_t		lval;
#endif

	/* Sanity check on the handles we've been given. */
	BDBG_ASSERT( hChip );
	BDBG_ASSERT( hRegister );
	BDBG_ASSERT( hInterrupt );

	/* Alloc memory from the system heap */
	hDev = (BTMN_Handle) BKNI_Malloc( sizeof( BTMN_P_Handle ) );
	if ( hDev == NULL )
	{
		*pTmn = NULL;
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BTMN_Open: BKNI_malloc() failed\n"));
		goto done;
	}

	hDev->magicId			= DEV_MAGIC_ID;
	hDev->hChip				= hChip;
	hDev->hRegister 		= hRegister;
	hDev->hInterrupt 		= hInterrupt;
	hDev->stDataReadyUserCB.cbFunc = NULL;
	hDev->stOverTempUserCB.cbFunc = NULL;
	
	BTMN_CHK_RETCODE( retCode, BKNI_CreateEvent( &(hDev->hDataReadyEvent) ) );
	BTMN_CHK_RETCODE( retCode, BKNI_CreateEvent( &(hDev->hOverTempEvent) ) );

#ifdef BCHP_PWR_RESOURCE_TEMP_MONITOR
	BCHP_PWR_AcquireResource(hDev->hChip, BCHP_PWR_RESOURCE_TEMP_MONITOR);    
#endif

	BTMN_Reset (hDev);
	/* Program measurement interval */
	BTMN_SetMeasurementInterval (hDev, pDefSettings->measureInterval);

#if BTMN_SUPPORTS_ONCHIPTEMP
	/* Enable reading of VCTAT data for on-chip temperature */
	lval = BREG_Read32 (hDev->hRegister, BCHP_TMON_GEN_CTRL);
	lval |= BCHP_TMON_GEN_CTRL_inc_vctat_MASK;
	BREG_Write32 (hDev->hRegister, BCHP_TMON_GEN_CTRL, lval);
#endif

#define TEST_SAMPLES (64*1024) /* this is the number of samples accumulated in vctat (default) */

	/* Make sure the interval is set to 64K */
	lval = BREG_Read32 (hDev->hRegister, BCHP_TMON_TEST_CTRL_1);
	lval &= ~BCHP_MASK(TMON_TEST_CTRL_1,num_samples);
	lval |= BCHP_FIELD_DATA(TMON_TEST_CTRL_1, num_samples, TEST_SAMPLES);
	BREG_Write32 (hDev->hRegister, BCHP_TMON_TEST_CTRL_1, lval);

	/* Clear data ready bit in case it was left over. */
	BREG_Write32 (hDev->hRegister, BCHP_TMON_STATUS, BCHP_TMON_STATUS_data_ready_MASK); 

	/* Register and enable L2 Data Ready interrupt. */
	BTMN_CHK_RETCODE( retCode, BINT_CreateCallback(
	    &(hDev->hDataReadyCB), hDev->hInterrupt, BCHP_INT_ID_TMON_DATA_READY,
	    BTMN_P_HandleDataReadyInterrupt_Isr, (void *) hDev, 0x00 ) );
	BTMN_CHK_RETCODE( retCode, BINT_EnableCallback( hDev->hDataReadyCB ) );

	/* Register and enable L2 Over Temperature interrupt. */
	BTMN_CHK_RETCODE( retCode, BINT_CreateCallback(
	    &(hDev->hOverTempCB), hDev->hInterrupt, BCHP_INT_ID_TMON_OVER_TEMPERATURE,
	    BTMN_P_HandleOverTempInterrupt_Isr, (void *) hDev, 0x00 ) );
	BTMN_CHK_RETCODE( retCode, BINT_EnableCallback( hDev->hOverTempCB ) );

	*pTmn = hDev;

done:
	if ((retCode != BERR_SUCCESS) && hDev)
	{
#ifdef BCHP_PWR_RESOURCE_TEMP_MONITOR
		BCHP_PWR_ReleaseResource(hDev->hChip, BCHP_PWR_RESOURCE_TEMP_MONITOR);    
#endif
		BKNI_Free( (void *) hDev );
		*pTmn = NULL;
	}
	return( retCode );
}

BERR_Code BTMN_Close(
	BTMN_Handle hDev					/* Device handle */
	)
{
	BERR_Code 	retCode = BERR_SUCCESS;

	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	/* Disable TMON */
	BTMN_Enable (hDev, false);

	/* Power down ADC, bandgap */
#if ((BCHP_CHIP==7420) || (BCHP_CHIP==7340) || (BCHP_CHIP==7342))
	BREG_Write32 (hDev->hRegister, BCHP_TMON_AFE_CTRL, 
		(BCHP_TMON_AFE_CTRL_bg_pwrdn_MASK | BCHP_TMON_AFE_CTRL_adc_pwrdn_MASK));
#else
	BREG_Write32 (hDev->hRegister, BCHP_TMON_AFE_CTRL, BCHP_TMON_AFE_CTRL_adc_pwrdn_MASK);
#endif

	if ( hDev->hDataReadyCB )
	{
		BTMN_CHK_RETCODE( retCode, BINT_DisableCallback( hDev->hDataReadyCB ) );
		BTMN_CHK_RETCODE( retCode, BINT_DestroyCallback( hDev->hDataReadyCB ) );
	}
	if ( hDev->hOverTempCB )
	{
		BTMN_CHK_RETCODE( retCode, BINT_DisableCallback( hDev->hOverTempCB ) );
		BTMN_CHK_RETCODE( retCode, BINT_DestroyCallback( hDev->hOverTempCB ) );
	}

	BKNI_DestroyEvent( hDev->hDataReadyEvent );
	BKNI_DestroyEvent( hDev->hOverTempEvent );
#ifdef BCHP_PWR_RESOURCE_TEMP_MONITOR
	BCHP_PWR_ReleaseResource(hDev->hChip, BCHP_PWR_RESOURCE_TEMP_MONITOR);    
#endif
	BKNI_Free( (void *) hDev );

done:
	return( retCode );
}

BERR_Code BTMN_GetDefaultSettings(
	BTMN_Settings *pDefSettings,		/* [output] Returns default setting */
	BCHP_Handle hChip					/* Chip handle */
)
{
	BERR_Code retCode = BERR_SUCCESS;
	BSTD_UNUSED(hChip);

	*pDefSettings = defTmnSettings;

	return( retCode );
}

BERR_Code BTMN_Reset(
	BTMN_Handle hDev					/* Device handle */
	)
{
	BERR_Code 	retCode = BERR_SUCCESS;

	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	/* Reset TMON core, this bit is self-clearing */
	BREG_Write32 (hDev->hRegister, BCHP_TMON_GEN_CTRL, BCHP_TMON_GEN_CTRL_sw_reset_MASK);

	/* Power on ADC, bandgap */
	BREG_Write32 (hDev->hRegister, BCHP_TMON_AFE_CTRL, 0);
	BKNI_Sleep (1);					/* wait 1 ms */

	/* Default to enable */
	BTMN_Enable (hDev, true);

	return( retCode );
}

BERR_Code BTMN_GetDataReadyEventHandle(
	BTMN_Handle		hDev,			/* Device handle */
	BKNI_EventHandle *phEvent		/* [output] Returns event handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;

	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	
	*phEvent = hDev->hDataReadyEvent;

	return( retCode );
}

BERR_Code BTMN_GetOverTempEventHandle(
	BTMN_Handle		hDev,			/* Device handle */
	BKNI_EventHandle *phEvent		/* [output] Returns event handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;

	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	
	*phEvent = hDev->hOverTempEvent;

	return( retCode );
}

BERR_Code BTMN_Enable (
	BTMN_Handle 		hDev, 			/* Device handle */
	bool				enable			/* enable flag */
)
{
	uint32_t			lval;

	if (enable)
	{
		lval = BREG_Read32 (hDev->hRegister, BCHP_TMON_GEN_CTRL);
		lval |= BCHP_TMON_GEN_CTRL_enable_MASK;
		BREG_Write32 (hDev->hRegister, BCHP_TMON_GEN_CTRL, lval);
	}
	else
	{
		lval = BREG_Read32 (hDev->hRegister, BCHP_TMON_GEN_CTRL);
		lval &= ~BCHP_TMON_GEN_CTRL_enable_MASK;
		BREG_Write32 (hDev->hRegister, BCHP_TMON_GEN_CTRL, lval);
	}

	return BERR_SUCCESS;
}

bool BTMN_GetEnable (
	BTMN_Handle 		hDev 			/* [in] Device handle */
)
{
	uint32_t			lval;

	lval = BREG_Read32 (hDev->hRegister, BCHP_TMON_GEN_CTRL);
	return ((lval & BCHP_TMON_GEN_CTRL_enable_MASK) ? true : false);
}

BERR_Code BTMN_AssertOverTempInterrupt (
	BTMN_Handle 		hDev 			/* Device handle */
)
{
	if ( hDev->hOverTempCB )
	     BINT_TriggerInterruptByHandle(hDev->hOverTempCB);
	return BERR_SUCCESS;
}

BERR_Code BTMN_SetMeasurementInterval (
	BTMN_Handle 		hDev, 			/* Device handle */
	uint8_t				interval		/* new measurement interval */
)
{
	uint32_t			lval;

	lval = BREG_Read32 (hDev->hRegister, BCHP_TMON_GEN_CTRL);
	lval &= ~BCHP_TMON_GEN_CTRL_measurement_interval_MASK;
	lval |= interval;
	BREG_Write32 (hDev->hRegister, BCHP_TMON_GEN_CTRL, lval);

	return BERR_SUCCESS;
}

BERR_Code BTMN_GetMeasurementInterval (
	BTMN_Handle 		hDev, 			/* Device handle */
	uint8_t				*interval		/* pointer to measurement interval */
)
{
	uint32_t			lval;

	if (interval == NULL)
	{
		return BERR_INVALID_PARAMETER;
	}

	lval = BREG_Read32 (hDev->hRegister, BCHP_TMON_GEN_CTRL);
	lval &= BCHP_TMON_GEN_CTRL_measurement_interval_MASK;
	*interval = (uint8_t)lval;

	return BERR_SUCCESS;
}

/*
 * Interrupt related stuff
 */

BERR_Code BTMN_InstallDataReadyInterruptCallback (
	BTMN_Handle 		hDev,			/* Device handle */
	BTMN_CallbackFunc	callback,		/* callback function */
	void				*pParm1, 		/* application specified parameter */
	int					parm2 			/* application specified parameter */
)
{
	BDBG_ASSERT( hDev );

	/* Put the interrupt callback into our array */
	BKNI_EnterCriticalSection();
	hDev->stDataReadyUserCB.cbFunc = callback;
	hDev->stDataReadyUserCB.pParm1 = pParm1;
	hDev->stDataReadyUserCB.parm2 = parm2;
	BKNI_LeaveCriticalSection();

	return BERR_SUCCESS;
}

BERR_Code BTMN_UnInstallDataReadyInterruptCallback (
	BTMN_Handle 		hDev			/* Device handle */
)
{
	BDBG_ASSERT( hDev );

	/* Put the interrupt callback into our array */
	BKNI_EnterCriticalSection();
	hDev->stDataReadyUserCB.cbFunc = NULL;
	hDev->stDataReadyUserCB.pParm1 = NULL;
	hDev->stDataReadyUserCB.parm2 = 0;
	BKNI_LeaveCriticalSection();

	return BERR_SUCCESS;
}

BERR_Code BTMN_InstallOverTempInterruptCallback (
	BTMN_Handle 		hDev,			/* Device handle */
	BTMN_CallbackFunc	callback,		/* callback function */
	void				*pParm1, 		/* application specified parameter */
	int					parm2 			/* application specified parameter */
)
{
	BDBG_ASSERT( hDev );

	/* Put the interrupt callback into our array */
	BKNI_EnterCriticalSection();
	hDev->stOverTempUserCB.cbFunc = callback;
	hDev->stOverTempUserCB.pParm1 = pParm1;
	hDev->stOverTempUserCB.parm2 = parm2;
	BKNI_LeaveCriticalSection();

	return BERR_SUCCESS;
}

BERR_Code BTMN_UnInstallOverTempInterruptCallback (
	BTMN_Handle 		hDev			/* Device handle */
)
{
	BDBG_ASSERT( hDev );

	/* Put the interrupt callback into our array */
	BKNI_EnterCriticalSection();
	hDev->stOverTempUserCB.cbFunc = NULL;
	hDev->stOverTempUserCB.pParm1 = NULL;
	hDev->stOverTempUserCB.parm2 = 0;
	BKNI_LeaveCriticalSection();

	return BERR_SUCCESS;
}

BERR_Code BTMN_GetVoltageValues_isr (
	BTMN_Handle 		hDev,			/* Device handle */
	uint32_t			*top,			/* pointer to top */
	uint32_t			*bottom,		/* pointer to bottom */
	uint32_t			*mid1,			/* pointer to mid1 */
	uint32_t			*mid0			/* pointer to mid0 */
)
{
	BDBG_ASSERT( hDev );

	*top 	= hDev->top;
	*bottom	= hDev->bottom;
	*mid1	= hDev->mid1;
	*mid0	= hDev->mid0;

	return BERR_SUCCESS;
}

BERR_Code BTMN_GetVoltageValues (
	BTMN_Handle 		hDev,			/* Device handle */
	uint32_t			*top,			/* pointer to top */
	uint32_t			*bottom,		/* pointer to bottom */
	uint32_t			*mid1,			/* pointer to mid1 */
	uint32_t			*mid0			/* pointer to mid0 */
)
{
	BDBG_ASSERT( hDev );

	BKNI_EnterCriticalSection();
	BTMN_GetVoltageValues_isr (hDev, top, bottom, mid1, mid0);
	BKNI_LeaveCriticalSection();

	return BERR_SUCCESS;
}

/*******************************************************************************
*
*	Private Module Functions
*
*******************************************************************************/
static void BTMN_P_HandleDataReadyInterrupt_Isr
(
    void *pParam1,                      /* Device channel handle */
    int parm2                           /* not used */
)
{
    BTMN_Handle         hDev;

    hDev = (BTMN_Handle) pParam1;
    BDBG_ASSERT( hDev );

    BSTD_UNUSED(parm2);
	BDBG_MSG(("TMON DataReady Int!!\n"));

	/* Clear the interrupt */
	BREG_Write32 (hDev->hRegister, BCHP_TMON_STATUS, BCHP_TMON_STATUS_data_ready_MASK); /* write to clear bit */

	/* Read temp values */
	hDev->top = (BREG_Read32 (hDev->hRegister, BCHP_TMON_TOP_DATA)) & BCHP_TMON_TOP_DATA_top_MASK;
	hDev->bottom = (BREG_Read32 (hDev->hRegister, BCHP_TMON_BOT_DATA)) & BCHP_TMON_BOT_DATA_bot_MASK;
	hDev->mid1 = (BREG_Read32 (hDev->hRegister, BCHP_TMON_MID1_DATA)) & BCHP_TMON_MID1_DATA_mid1_MASK;
	hDev->mid0 = (BREG_Read32 (hDev->hRegister, BCHP_TMON_MID0_DATA)) & BCHP_TMON_MID0_DATA_mid0_MASK;
#if BTMN_SUPPORTS_ONCHIPTEMP
	hDev->vctat = (BREG_Read32 (hDev->hRegister, BCHP_TMON_VCTAT_DATA)) & BCHP_TMON_VCTAT_DATA_bot_MASK;
#endif

	BDBG_MSG(("top = %x bottom = %x, mid1 = %x, mid0 = %x\n", hDev->top, hDev->bottom, hDev->mid1, hDev->mid0));

	/* Check to see if user has installed a callback */
	if (hDev->stDataReadyUserCB.cbFunc)
	{
		hDev->stDataReadyUserCB.cbFunc (hDev->stDataReadyUserCB.pParm1, 
										hDev->stDataReadyUserCB.parm2, 
										NULL);
	}

    BKNI_SetEvent (hDev->hDataReadyEvent);

    return;
}


static void BTMN_P_HandleOverTempInterrupt_Isr
(
    void *pParam1,                      /* Device channel handle */
    int parm2                           /* not used */
)
{
    BTMN_Handle         hDev;

    hDev = (BTMN_Handle) pParam1;
    BDBG_ASSERT( hDev );

    BSTD_UNUSED(parm2);
	BDBG_MSG(("TMON OverTemp Int!!\n"));

	/* Check to see if user has installed a callback */
	if (hDev->stOverTempUserCB.cbFunc)
	{
		hDev->stOverTempUserCB.cbFunc (	hDev->stOverTempUserCB.pParm1, 
										hDev->stOverTempUserCB.parm2, 
										NULL);
	}

    BKNI_SetEvent (hDev->hOverTempEvent);

    return;
}

#if BTMN_SUPPORTS_ONCHIPTEMP
BERR_Code BTMN_GetOnChipTemp (
	BTMN_Handle 		hDev,			/* [in] Device handle */
	signed			*tempC			/* [out] temperature in degrees C */
)
{
	uint32_t  vctat;
	signed temp;

	/* VCTAT is the accumulation of samples (so divide by number of samples to make this an average). */
	vctat = hDev->vctat / TEST_SAMPLES;

#if ((BCHP_CHIP==7445 && BCHP_VER <= BCHP_VER_C0) || \
     (BCHP_CHIP==7439 && BCHP_VER == BCHP_VER_A0) || \
     (BCHP_CHIP==7366 && BCHP_VER == BCHP_VER_A0))
	/* 28nm parts using spec v1 */
	/* T = 425.2179 - o_ADC_data * 0.49935 */
	temp = 425217 - (499 * vctat);
#elif defined(BCHP_TMON_REVID) /* this register added on newer parts */
	/* 28nm parts using specification v3 */
	/* T = 410.04 - o_ADC_data * 0.48705 */
	temp = 410040 - (487 * vctat);
#else
	/* 40nm parts */
	/* T = 407 - (0.538 * o_TEMPS_data) */
	temp = 407000 - (538 * vctat);
#endif

	BDBG_MSG(("Temperature read = %d (vctat=%x)", temp, vctat));
	*tempC = temp / 1000; /* ignore the fractional part */

	return BERR_SUCCESS;
}
#endif


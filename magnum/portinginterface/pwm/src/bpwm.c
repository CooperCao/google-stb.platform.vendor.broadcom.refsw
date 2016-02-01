/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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
#include "bpwm.h"
#include "bchp.h"
#if defined(BCHP_IRQ0_REG_START)
#include "bchp_irq0.h"
#endif
#include "bchp_pwm.h"
#if (MAX_PWM_CHANNELS > 2)
#include "bchp_pwmb.h"
#endif
#if (MAX_PWM_CHANNELS > 4)
#include "bchp_pwmc.h"
#endif
#if (MAX_PWM_CHANNELS > 6)
#include "bchp_pwmd.h"
#endif
#if (MAX_PWM_CHANNELS > 8)
#include "bchp_pwme.h"
#endif


BDBG_MODULE(bpwm);

#define DEV_MAGIC_ID            ((BERR_PWM_ID<<16) | 0xFACE)

#define BPWM_CHK_RETCODE( rc, func )        \
do {                                        \
	if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
	{                                       \
		goto done;                          \
	}                                       \
} while(0)


#define EVEN_CHANNEL(x)	((x & 0x01) == 0)

/*******************************************************************************
*
*   Private Module Handles
*
*******************************************************************************/

typedef struct BPWM_P_Handle
{
	uint32_t        magicId;                    /* Used to check if structure is corrupt */
	BCHP_Handle     hChip;
	BREG_Handle     hRegister;
	unsigned int    maxChnNo;
	BPWM_ChannelHandle hPwmChn[MAX_PWM_CHANNELS];
	BPWM_Settings   settings;
} BPWM_P_Handle;

typedef struct BPWM_P_ChannelHandle
{
	uint32_t            magicId;                    /* Used to check if structure is corrupt */
	BPWM_Handle         hPwm;
	uint32_t            chnNo;
    uint32_t        	coreOffset;					/* added to support multiple PWM cores */
	BPWM_ChannelSettings settings;
	struct {
		unsigned OnInterval;
		BINT_CallbackHandle isrCallback, secondaryIsrCallback;
	} ramp;
} BPWM_P_ChannelHandle;

static void BPWM_SetOnIntervalRamp_isr(void *context, int param);

/*******************************************************************************
*
*   Default Module Settings
*
*******************************************************************************/
static const BPWM_ChannelSettings defPwmChnSettings[MAX_PWM_CHANNELS] =
{
	{ false, Variable_Freq_Mode, 3000, 0, 0 },
	{ false, Variable_Freq_Mode, 3000, 0, 0 },
#if (MAX_PWM_CHANNELS == 4)
	{ false, Variable_Freq_Mode, 3000, 0, 0 },
	{ false, Variable_Freq_Mode, 3000, 0, 0 },
#elif (MAX_PWM_CHANNELS == 10)
	{ false, Variable_Freq_Mode, 3000, 0, 0 },
	{ false, Variable_Freq_Mode, 3000, 0, 0 },
	{ false, Variable_Freq_Mode, 3000, 0, 0 },
	{ false, Variable_Freq_Mode, 3000, 0, 0 },
	{ false, Variable_Freq_Mode, 3000, 0, 0 },
	{ false, Variable_Freq_Mode, 3000, 0, 0 },
	{ false, Variable_Freq_Mode, 3000, 0, 0 },
	{ false, Variable_Freq_Mode, 3000, 0, 0 },
#endif
};

static const uint32_t coreOffsetVal[] =
{
    0
#if (MAX_PWM_CHANNELS > 2)
    , BCHP_PWMB_CTRL - BCHP_PWM_CTRL
#endif
#if (MAX_PWM_CHANNELS > 4)
    , BCHP_PWMC_CTRL - BCHP_PWM_CTRL
#endif
#if (MAX_PWM_CHANNELS > 6)
    , BCHP_PWMD_CTRL - BCHP_PWM_CTRL
#endif
#if (MAX_PWM_CHANNELS > 8)
    , BCHP_PWME_CTRL - BCHP_PWM_CTRL
#endif
};


/*******************************************************************************
*
*   Public Module Functions
*
*******************************************************************************/
BERR_Code BPWM_Open(
	BPWM_Handle *pPwm,                  /* [output] Returns handle */
	BCHP_Handle hChip,                  /* Chip handle */
	BREG_Handle hRegister,              /* Register handle */
	const BPWM_Settings *pDefSettings   /* Default settings */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BPWM_Handle hDev;
	unsigned int chnIdx;

	/* Sanity check on the handles we've been given. */
	BDBG_ASSERT( hChip );
	BDBG_ASSERT( hRegister );

	/* Alloc memory from the system heap */
	hDev = (BPWM_Handle) BKNI_Malloc( sizeof( BPWM_P_Handle ) );
	if( hDev == NULL )
	{
		*pPwm = NULL;
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BPWM_Open: BKNI_malloc() failed\n"));
		goto done;
	}
	BKNI_Memset(hDev, 0, sizeof(*hDev));

	hDev->magicId   = DEV_MAGIC_ID;
	hDev->hChip     = hChip;
	hDev->hRegister = hRegister;
	hDev->maxChnNo  = MAX_PWM_CHANNELS;
	for( chnIdx = 0; chnIdx < hDev->maxChnNo; chnIdx++ )
	{
		hDev->hPwmChn[chnIdx] = NULL;
	}
	if (pDefSettings) {
		hDev->settings = *pDefSettings;
	}

	*pPwm = hDev;

done:
	return( retCode );
}

BERR_Code BPWM_Close(
	BPWM_Handle hDev                    /* Device handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;


	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	BKNI_Free( (void *) hDev );

	return( retCode );
}

BERR_Code BPWM_GetDefaultSettings(
	BPWM_Settings *pDefSettings,        /* [output] Returns default setting */
	BCHP_Handle hChip                   /* Chip handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;

	BSTD_UNUSED( hChip );

	BKNI_Memset(pDefSettings, 0, sizeof(*pDefSettings));

	return( retCode );
}

BERR_Code BPWM_GetTotalChannels(
	BPWM_Handle hDev,                   /* Device handle */
	unsigned int *totalChannels         /* [output] Returns total number downstream channels supported */
	)
{
	BERR_Code retCode = BERR_SUCCESS;


	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	*totalChannels = hDev->maxChnNo;

	return( retCode );
}

BERR_Code BPWM_GetChannelDefaultSettings(
	BPWM_Handle hDev,                   /* Device handle */
	unsigned int channelNo,             /* Channel number to default setting for */
	BPWM_ChannelSettings *pChnDefSettings /* [output] Returns channel default setting */
	)
{
	BERR_Code retCode = BERR_SUCCESS;

	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	if (channelNo >= MAX_PWM_CHANNELS)
	{
		retCode = BERR_INVALID_PARAMETER;
	}
	else
	{
		*pChnDefSettings = defPwmChnSettings[channelNo];
	}

	return( retCode );
}


BERR_Code BPWM_OpenChannel(
	BPWM_Handle hDev,                   /* Device handle */
	BPWM_ChannelHandle *phChn,          /* [output] Returns channel handle */
	unsigned int channelNo,             /* Channel number to open */
	const BPWM_ChannelSettings *pChnDefSettings /* Channel default setting */
	)
{
	BERR_Code           retCode = BERR_SUCCESS;
	BPWM_ChannelHandle  hChnDev;
	uint32_t            lval;

	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	if( channelNo < hDev->maxChnNo )
	{
		if( hDev->hPwmChn[channelNo] == NULL )
		{
			/* Alloc memory from the system heap */
			hChnDev = (BPWM_ChannelHandle) BKNI_Malloc( sizeof( BPWM_P_ChannelHandle ) );
			if( hChnDev == NULL )
			{
				*phChn = NULL;
				retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
				BDBG_ERR(("BPWM_OpenChannel: BKNI_malloc() failed\n"));
				goto done;
			}

			/* Zero out initializations */
			BKNI_Memset(hChnDev, 0, sizeof(BPWM_P_ChannelHandle));

			hChnDev->settings = *pChnDefSettings;
			hChnDev->magicId    = DEV_MAGIC_ID;
			hChnDev->hPwm       = hDev;
			hChnDev->chnNo      = channelNo;
			hDev->hPwmChn[channelNo] = hChnDev;

            /*
             * Offsets are based off of BSCA
             */
            hChnDev->coreOffset = coreOffsetVal[channelNo>>1];

			/* Program the open drain configuration setting */
			lval = BREG_Read32(hDev->hRegister, (hChnDev->coreOffset + BCHP_PWM_CTRL));
			if (pChnDefSettings->openDrainb == false)
			{
				lval |= EVEN_CHANNEL(channelNo) ? BCHP_PWM_CTRL_pwm1_opendrainb_MASK : BCHP_PWM_CTRL_pwm2_opendrainb_MASK;
				lval &= EVEN_CHANNEL(channelNo) ? ~BCHP_PWM_CTRL_pwm1_oeb_MASK : ~BCHP_PWM_CTRL_pwm2_oeb_MASK;
			}
			else
			{
				lval &= EVEN_CHANNEL(channelNo) ? ~BCHP_PWM_CTRL_pwm1_opendrainb_MASK : ~BCHP_PWM_CTRL_pwm2_opendrainb_MASK;
				lval &= EVEN_CHANNEL(channelNo) ? ~BCHP_PWM_CTRL_pwm1_oeb_MASK : ~BCHP_PWM_CTRL_pwm2_oeb_MASK;
			}
			BREG_Write32(hDev->hRegister, (hChnDev->coreOffset + BCHP_PWM_CTRL), lval);

			/* Program the Freq Mode */
			if (pChnDefSettings->FreqMode == Variable_Freq_Mode)
			{
				BPWM_SetFreqMode(hChnDev, Variable_Freq_Mode);
			}
			else
			{
				BPWM_SetFreqMode(hChnDev, Constant_Freq_Mode);
			}

			if (pChnDefSettings->interruptId && hChnDev->hPwm->settings.hInterrupt) {
				retCode = BINT_CreateCallback(&hChnDev->ramp.isrCallback, hChnDev->hPwm->settings.hInterrupt, pChnDefSettings->interruptId, BPWM_SetOnIntervalRamp_isr, hChnDev, 0);
				if (retCode) {
					retCode = BERR_TRACE(retCode);
					BKNI_Free(hChnDev);
					goto done;
				}
			}

			if (pChnDefSettings->secondaryInterruptId && hChnDev->hPwm->settings.hInterrupt) {
				retCode = BINT_CreateCallback(&hChnDev->ramp.secondaryIsrCallback, hChnDev->hPwm->settings.hInterrupt, pChnDefSettings->secondaryInterruptId, BPWM_SetOnIntervalRamp_isr, hChnDev, 0);
				if (retCode) {
					retCode = BERR_TRACE(retCode);
					BINT_DestroyCallback(hChnDev->ramp.isrCallback);
					BKNI_Free(hChnDev);
					goto done;
				}
			}

			*phChn = hChnDev;
		}
		else
		{
			retCode = BPWM_ERR_NOTAVAIL_CHN_NO;
		}
	}
	else
	{
		retCode = BERR_INVALID_PARAMETER;
	}

done:
	return( retCode );
}

BERR_Code BPWM_CloseChannel(
	BPWM_ChannelHandle hChn         /* Device channel handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BPWM_Handle hDev;
	unsigned int chnNo;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	if (hChn->ramp.isrCallback) {
		BINT_DestroyCallback(hChn->ramp.isrCallback);
	}
	if (hChn->ramp.secondaryIsrCallback) {
		BINT_DestroyCallback(hChn->ramp.secondaryIsrCallback);
	}

	hDev = hChn->hPwm;
	chnNo = hChn->chnNo;
	BKNI_Free( hChn );
	hDev->hPwmChn[chnNo] = NULL;

	return( retCode );
}

BERR_Code BPWM_GetDevice(
	BPWM_ChannelHandle hChn,            /* Device channel handle */
	BPWM_Handle *phDev                  /* [output] Returns Device handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;


	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	*phDev = hChn->hPwm;

	return( retCode );
}


BERR_Code BPWM_SetControlWord(
	BPWM_ChannelHandle  hChn,           /* Device channel handle */
	uint16_t            cWord           /* control word to program */
	)
{
	BERR_Code   retCode = BERR_SUCCESS;
	uint32_t    lval;
	BPWM_Handle hDev;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hPwm;

	if (EVEN_CHANNEL(hChn->chnNo))
	{
		lval = (uint32_t)(cWord >> 8);
		BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_PWM1_CWORD_MSB), lval);
		lval = (uint32_t)(cWord & 0xff);
		BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_PWM1_CWORD_LSB), lval);
	}
	else
	{
		lval = (uint32_t)(cWord >> 8);
		BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_PWM2_CWORD_MSB), lval);
		lval = (uint32_t)(cWord & 0xff);
		BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_PWM2_CWORD_LSB), lval);
	}

	return( retCode );
}

BERR_Code BPWM_GetControlWord(
	BPWM_ChannelHandle  hChn,           /* Device channel handle */
	uint16_t            *cWord          /* control word to program */
	)
{
	BERR_Code   retCode = BERR_SUCCESS;
	uint32_t    msb, lsb;
	BPWM_Handle hDev;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hPwm;

	if (EVEN_CHANNEL(hChn->chnNo))
	{
		msb = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_PWM1_CWORD_MSB));
		lsb = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_PWM1_CWORD_LSB));
		*cWord = (uint16_t)((msb << 8) | (lsb & 0xff));
	}
	else
	{
		msb = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_PWM2_CWORD_MSB));
		lsb = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_PWM2_CWORD_LSB));
		*cWord = (uint16_t)((msb << 8) | (lsb & 0xff));
	}

	return( retCode );
}

BERR_Code BPWM_SetFreqMode(
	BPWM_ChannelHandle  hChn,                  /* Device channel handle */
	BPWM_FreqModeType           Frequeny_Mode       /* Frequency mode  to program */
	)
{
	BERR_Code   retCode = BERR_SUCCESS;
	uint32_t    lval;
	BPWM_Handle hDev;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hPwm;

	lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL2));
	   if (Frequeny_Mode == Variable_Freq_Mode)
	{
		lval |= EVEN_CHANNEL(hChn->chnNo) ? BCHP_PWM_CTRL2_pwm1_out_select_MASK : BCHP_PWM_CTRL2_pwm2_out_select_MASK;
		lval &= EVEN_CHANNEL(hChn->chnNo) ? ~BCHP_PWM_CTRL2_pwm1_out_select_MASK : ~BCHP_PWM_CTRL2_pwm2_out_select_MASK;
	}
	else
	{
		lval &= EVEN_CHANNEL(hChn->chnNo) ? ~BCHP_PWM_CTRL2_pwm1_out_select_MASK : ~BCHP_PWM_CTRL2_pwm2_out_select_MASK;
		lval |= EVEN_CHANNEL(hChn->chnNo) ? BCHP_PWM_CTRL2_pwm1_out_select_MASK : BCHP_PWM_CTRL2_pwm2_out_select_MASK;
	}
	BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL2), lval);

	return( retCode );
}


BERR_Code BPWM_GetFreqMode(
	BPWM_ChannelHandle  hChn,                  /* Device channel handle */
	BPWM_FreqModeType           *Frequeny_Mode      /* Frequency mode  read */
	)
{
	BERR_Code   retCode = BERR_SUCCESS;
	uint32_t    lval;
	BPWM_Handle hDev;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hPwm;

	lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL2));

	if (EVEN_CHANNEL(hChn->chnNo))
	{

		if (lval & BCHP_PWM_CTRL2_pwm1_out_select_MASK)
			*Frequeny_Mode = Constant_Freq_Mode;
		else
			*Frequeny_Mode = Variable_Freq_Mode;
	}
	else
	{
		if (lval & BCHP_PWM_CTRL2_pwm2_out_select_MASK)
			*Frequeny_Mode = Constant_Freq_Mode;
		else
			*Frequeny_Mode = Variable_Freq_Mode;
	}

	return( retCode );
}

static void BPWM_SetOnIntervalRamp_isr(void *context, int param)
{
	BPWM_ChannelHandle hChn = context;
	BPWM_Handle hDev = hChn->hPwm;
	uint32_t addr;
	uint32_t currentValue;
	uint32_t targetValue;
	uint32_t cnt;

	BSTD_UNUSED(param);

	if (EVEN_CHANNEL(hChn->chnNo)) {
		addr = BCHP_PWM_PWM1_ON;
	}
	else {
		addr = BCHP_PWM_PWM2_ON;
	}

	currentValue = BREG_Read32_isr(hDev->hRegister, (hChn->coreOffset + addr));
	targetValue = hChn->ramp.OnInterval;
	cnt = hChn->settings.maxChangesPerInterrupt;
	while (currentValue != targetValue && cnt--)
	{
		if (currentValue < targetValue) {
			currentValue++;
		}
		else {
			currentValue--;
		}
		BREG_Write32_isr(hDev->hRegister, (hChn->coreOffset + addr), currentValue);
	}
	if (currentValue == targetValue) {
		if (hChn->ramp.isrCallback) {
			BINT_DisableCallback_isr(hChn->ramp.isrCallback);
		}
		if (hChn->ramp.secondaryIsrCallback) {
			BINT_DisableCallback_isr(hChn->ramp.secondaryIsrCallback);
		}
	}
}

BERR_Code BPWM_RampOnInterval( BPWM_ChannelHandle  hChn, uint16_t OnInterval )
{
	BERR_Code rc;
	BKNI_EnterCriticalSection();
	rc = BPWM_RampOnInterval_isr(hChn, OnInterval);
	BKNI_LeaveCriticalSection();
	return rc;
}

BERR_Code BPWM_RampOnInterval_isr( BPWM_ChannelHandle  hChn, uint16_t OnInterval )
{
	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	if (hChn->settings.maxChangesPerInterrupt == 0 || (!hChn->ramp.isrCallback && !hChn->ramp.secondaryIsrCallback)) {
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	hChn->ramp.OnInterval = OnInterval;
	if (hChn->ramp.isrCallback) {
		BINT_EnableCallback_isr(hChn->ramp.isrCallback);
	}
	if (hChn->ramp.secondaryIsrCallback) {
		BINT_EnableCallback_isr(hChn->ramp.secondaryIsrCallback);
	}
	return 0;
}

BERR_Code BPWM_SetOnInterval(
	BPWM_ChannelHandle  hChn,           /* Device channel handle */
	uint16_t            OnInterval          /* On Interval  to program */
	)
{
	BERR_Code   retCode = BERR_SUCCESS;
	uint32_t    lval;
	BPWM_Handle hDev;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hPwm;

	if (EVEN_CHANNEL(hChn->chnNo))
	{
		lval = (uint32_t)(OnInterval & BCHP_PWM_PWM1_ON_pwm1_on_MASK);
		BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_PWM1_ON), lval);
	}
	else
	{
		lval = (uint32_t)(OnInterval & BCHP_PWM_PWM1_ON_pwm1_on_MASK);
		BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_PWM2_ON), lval);
	}

	return( retCode );
}


BERR_Code BPWM_GetOnInterval(
	BPWM_ChannelHandle  hChn,           /* [in] Device channel handle */
	uint16_t            *OnInterval         /* [out] On Interval  to read */
	)
{
	BERR_Code   retCode = BERR_SUCCESS;
	uint32_t    lval;
	BPWM_Handle hDev;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hPwm;


	if (EVEN_CHANNEL(hChn->chnNo))
	{
		lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_PWM1_ON));
		*OnInterval = (uint16_t)(lval & BCHP_PWM_PWM1_ON_pwm1_on_MASK);
	}
	else
	{
		lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_PWM2_ON));
		*OnInterval = (uint16_t)(lval & BCHP_PWM_PWM1_ON_pwm1_on_MASK);
	}

	return( retCode );
}


BERR_Code BPWM_SetPeriodInterval(
	BPWM_ChannelHandle  hChn,           /* Device channel handle */
	uint16_t            PeriodInterval          /* Period Interval  to program */
	)
{
	BERR_Code   retCode = BERR_SUCCESS;
	uint32_t    lval;
	BPWM_Handle hDev;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hPwm;

	if (EVEN_CHANNEL(hChn->chnNo))
	{
		lval = (uint32_t)(PeriodInterval & BCHP_PWM_PWM1_PERIOD_pwm1_period_MASK);
		BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_PWM1_PERIOD), lval);
	}
	else
	{
		lval = (uint32_t)(PeriodInterval & BCHP_PWM_PWM1_PERIOD_pwm1_period_MASK);
		BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_PWM2_PERIOD), lval);
	}

	return( retCode );
}


BERR_Code BPWM_GetPeriodInterval(
	BPWM_ChannelHandle  hChn,           /* [in] Device channel handle */
	uint16_t            *PeriodInterval         /* [out] On Interval  to read */
	)
{
	BERR_Code   retCode = BERR_SUCCESS;
	uint32_t    lval;
	BPWM_Handle hDev;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hPwm;

	if (EVEN_CHANNEL(hChn->chnNo))
	{
		lval= BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_PWM1_PERIOD));
		*PeriodInterval = (uint16_t)(lval & BCHP_PWM_PWM1_PERIOD_pwm1_period_MASK);
	}
	else
	{
		lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_PWM2_PERIOD));
		*PeriodInterval = (uint16_t)(lval & BCHP_PWM_PWM1_PERIOD_pwm1_period_MASK);
	}

	return( retCode );
}

BERR_Code BPWM_SetOnAndPeriodInterval(
	BPWM_ChannelHandle  hChn,           /* Device channel handle */
	uint16_t            OnInterval,         /* OnInterval  to program */
	uint16_t            PeriodInterval      /* Period Interval  to program */
	)
{
	BERR_Code   retCode = BERR_SUCCESS;


	BPWM_SetOnInterval(hChn, OnInterval);
	BPWM_SetPeriodInterval(hChn, PeriodInterval);


	return( retCode );
}


BERR_Code BPWM_GetOnAndPeriodInterval(
	BPWM_ChannelHandle  hChn,           /* Device channel handle */
	uint16_t            *OnInterval,            /* OnInterval  to program */
	uint16_t            *PeriodInterval     /* Period Interval  to program */
	)
{
	BERR_Code   retCode = BERR_SUCCESS;

	BPWM_GetOnInterval(hChn, OnInterval);
	   BPWM_GetPeriodInterval(hChn, PeriodInterval);


	return( retCode );
}


BERR_Code BPWM_Start(
	BPWM_ChannelHandle hChn     /* Device channel handle */
	)
{
	BERR_Code       retCode = BERR_SUCCESS;
	uint32_t        lval;
	BPWM_Handle     hDev;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hPwm;

	if (EVEN_CHANNEL(hChn->chnNo))
	{
		lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL));
		lval |= BCHP_PWM_CTRL_pwm1_start_MASK;
		BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL), lval);
	}
	else
	{
		lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL));
		lval |= BCHP_PWM_CTRL_pwm2_start_MASK;
		BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL), lval);
	}

	return( retCode );
}

BERR_Code BPWM_Stop(
	BPWM_ChannelHandle hChn         /* Device channel handle */
	)
{
	BERR_Code       retCode = BERR_SUCCESS;
	uint32_t        lval;
	BPWM_Handle     hDev;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hPwm;

	if (EVEN_CHANNEL(hChn->chnNo))
	{
		lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL));
		lval &= ~BCHP_PWM_CTRL_pwm1_start_MASK;
		BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL), lval);
	}
	else
	{
		lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL));
		lval &= ~BCHP_PWM_CTRL_pwm2_start_MASK;
		BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL), lval);
	}

	return( retCode );
}

BERR_Code BPWM_SetOpenDrainConfig
(
	BPWM_ChannelHandle		hChn,			/* [in] Device channel handle */
	BPWM_OpenDrainOutput	type			/* [in] open drain/totem pole select */
)
{
	BERR_Code	retCode = BERR_SUCCESS;
	BPWM_Handle	hDev;
	uint32_t	lval;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hPwm;

	lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL));
	if (type == PwmOutTotemPole)
	{
		lval |= EVEN_CHANNEL(hChn->chnNo) ? BCHP_PWM_CTRL_pwm1_opendrainb_MASK : BCHP_PWM_CTRL_pwm2_opendrainb_MASK;
	}
	else
	{
		lval &= EVEN_CHANNEL(hChn->chnNo) ? ~BCHP_PWM_CTRL_pwm1_opendrainb_MASK : ~BCHP_PWM_CTRL_pwm2_opendrainb_MASK;
	}
	BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL), lval);

	return (retCode);
}

BERR_Code BPWM_GetOpenDrainConfig
(
	BPWM_ChannelHandle		hChn,			/* [in] Device channel handle */
	BPWM_OpenDrainOutput	*type			/* [out] open drain/totem pole select */
)
{
	BERR_Code	retCode = BERR_SUCCESS;
	BPWM_Handle	hDev;
	uint32_t	lval;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hPwm;

	lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL));
	if (EVEN_CHANNEL(hChn->chnNo))
	{
		lval &= BCHP_PWM_CTRL_pwm1_opendrainb_MASK;
	}
	else
	{
		lval &= BCHP_PWM_CTRL_pwm2_opendrainb_MASK;
	}
	*type = ((lval == 0) ? PwmOutOpenDrain : PwmOutTotemPole);

	return (retCode);
}

#if ((BCHP_CHIP==35230) || (BCHP_CHIP==35125) || (BCHP_CHIP==35233))
BERR_Code BPWM_SetControlWordSet1 
(
	BPWM_ChannelHandle  hChn,           /* [in] Device channel handle */
	uint16_t            cWord           /* [in] control word to program for register set 1 */
)
{
	BERR_Code   retCode = BERR_SUCCESS;
	BPWM_Handle hDev;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hPwm;

	if (EVEN_CHANNEL(hChn->chnNo))
	{
		BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_PWM1_RSET1_R0), (uint32_t)cWord);
	}
	else
	{
		BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_PWM2_RSET1_R0), (uint32_t)cWord);
	}

	return( retCode );
}

BERR_Code BPWM_GetControlWordSet1 
(
	BPWM_ChannelHandle  hChn,           /* [in] Device channel handle */
	uint16_t            *cWord          /* [out] control word to program for register set 1 */
)
{
	BERR_Code	retCode = BERR_SUCCESS;
	BPWM_Handle	hDev;
	uint32_t	lval;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hPwm;

	if (EVEN_CHANNEL(hChn->chnNo))
	{
		lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_PWM1_RSET1_R0));
	}
	else
	{
		lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_PWM2_RSET1_R0));
	}
	*cWord = (uint16_t)lval;

	return (retCode);
}

BERR_Code BPWM_SetOnAndPeriodIntervalSet1
(
	BPWM_ChannelHandle  hChn,           	/* [in] Device channel handle */
	uint16_t            OnInterval,         /* [in] OnInterval  to program */
	uint16_t            PeriodInterval      /* [in] Period Interval  to program */
)
{
	BERR_Code	retCode = BERR_SUCCESS;
	BPWM_Handle	hDev;
	uint32_t	lval;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hPwm;

	lval = (((uint32_t)(PeriodInterval)) << BCHP_PWM_PWM1_RSET1_R1_period_SHIFT) | 
			((uint32_t)(OnInterval));
	if (EVEN_CHANNEL(hChn->chnNo))
	{
		BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_PWM1_RSET1_R1), lval);
	}
	else
	{
		BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_PWM2_RSET1_R1), lval);
	}

	return (retCode);
}

BERR_Code BPWM_GetOnAndPeriodIntervalSet1
(
	BPWM_ChannelHandle  hChn,           	/* [in] Device channel handle */
	uint16_t            *OnInterval,        /* [out] OnInterval  to program */
	uint16_t            *PeriodInterval     /* [out] Period Interval  to program */
)
{
	BERR_Code	retCode = BERR_SUCCESS;
	BPWM_Handle	hDev;
	uint32_t	lval;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hPwm;

	if (EVEN_CHANNEL(hChn->chnNo))
	{
		lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_PWM1_RSET1_R1));
	}
	else
	{
		lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_PWM2_RSET1_R1));
	}

	*OnInterval = (uint16_t)(lval & BCHP_PWM_PWM1_RSET1_R1_on_MASK);
	*PeriodInterval = (uint16_t)(lval >> BCHP_PWM_PWM1_RSET1_R1_period_SHIFT);

	return (retCode);
}

BERR_Code BPWM_EnableSyncMode
(
	BPWM_ChannelHandle	hChn,				/* [in] Device channel handle */
	bool				enable				/* [in] sync enable */
)
{
	BERR_Code	retCode = BERR_SUCCESS;
	BPWM_Handle	hDev;
	uint32_t	lval;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hPwm;

	lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL2));
	if (enable == true)
	{
		if (EVEN_CHANNEL(hChn->chnNo))
		{
			lval |= BCHP_PWM_CTRL2_pwm1_enable_sync_MASK;
		}
		else
		{
			lval |= BCHP_PWM_CTRL2_pwm2_enable_sync_MASK;
		}
	}
	else
	{
		if (EVEN_CHANNEL(hChn->chnNo))
		{
			lval &= ~BCHP_PWM_CTRL2_pwm1_enable_sync_MASK;
		}
		else
		{
			lval &= ~BCHP_PWM_CTRL2_pwm2_enable_sync_MASK;
		}
	}
	BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL2), lval);

	return (retCode);
}

BERR_Code BPWM_SetDblBufMode
(
	BPWM_ChannelHandle	hChn,				/* [in] Device channel handle */
	BPWM_DblBufMode		dblBufMode			/* [in] double buffer mode type */
)
{
	BERR_Code	retCode = BERR_SUCCESS;
	BPWM_Handle	hDev;
	uint32_t	lval;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hPwm;

	lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL3));
	if (EVEN_CHANNEL(hChn->chnNo))
	{
		lval &= ~BCHP_PWM_CTRL3_pwm1_dbl_buf_mode_MASK;
		lval |= (uint32_t)dblBufMode;
	}
	else
	{
		lval &= ~BCHP_PWM_CTRL3_pwm2_dbl_buf_mode_MASK;
		lval |= ((uint32_t)dblBufMode) << BCHP_PWM_CTRL3_pwm2_dbl_buf_mode_SHIFT;
	}

	BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL3), lval);

	return (retCode);
}

BERR_Code BPWM_GetDblBufMode
(
	BPWM_ChannelHandle	hChn,				/* [in] Device channel handle */
	BPWM_DblBufMode		*dblBufMode			/* [out] double buffer mode type */
)
{
	BERR_Code	retCode = BERR_SUCCESS;
	BPWM_Handle	hDev;
	uint32_t	lval;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hPwm;

	lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL3));
	if (EVEN_CHANNEL(hChn->chnNo))
	{
		*dblBufMode = (BPWM_DblBufMode)(lval & BCHP_PWM_CTRL3_pwm1_dbl_buf_mode_MASK);
	}
	else
	{
		*dblBufMode = (BPWM_DblBufMode)((lval & BCHP_PWM_CTRL3_pwm2_dbl_buf_mode_MASK) >> 
											BCHP_PWM_CTRL3_pwm2_dbl_buf_mode_SHIFT);
	}

	return (retCode);
}

BERR_Code BPWM_SetSyncEdge
(
	BPWM_ChannelHandle	hChn,				/* [in] Device channel handle */
	BPWM_SyncEdgeSel	SyncEdgeSel			/* [in] edge type */
)
{
	BERR_Code	retCode = BERR_SUCCESS;
	BPWM_Handle	hDev;
	uint32_t	lval;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hPwm;

	lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL2));
	if (EVEN_CHANNEL(hChn->chnNo))
	{
		if (SyncEdgeSel == SyncEdgeNeg)
		{
			lval &= ~BCHP_PWM_CTRL2_pwm1_sync_edge_sel_MASK;
		}
		else
		{
			lval |= BCHP_PWM_CTRL2_pwm1_sync_edge_sel_MASK;
		}
	}
	else
	{
		if (SyncEdgeSel == SyncEdgeNeg)
		{
			lval &= ~BCHP_PWM_CTRL2_pwm2_sync_edge_sel_MASK;
		}
		else
		{
			lval |= BCHP_PWM_CTRL2_pwm2_sync_edge_sel_MASK;
		}
	}
	BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL2), lval);

	return (retCode);
}

BERR_Code BPWM_GetSyncEdge
(
	BPWM_ChannelHandle	hChn,				/* [in] Device channel handle */
	BPWM_SyncEdgeSel	*SyncEdgeSel		/* [out] edge type */
)
{
	BERR_Code	retCode = BERR_SUCCESS;
	BPWM_Handle	hDev;
	uint32_t	lval;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hPwm;

	lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL2));
	if (EVEN_CHANNEL(hChn->chnNo))
	{
		lval &= BCHP_PWM_CTRL2_pwm1_sync_edge_sel_MASK;
	}
	else
	{
		lval &= BCHP_PWM_CTRL2_pwm2_sync_edge_sel_MASK;
	}
	
	*SyncEdgeSel = ((lval == 0) ? SyncEdgeNeg : SyncEdgePos);

	return (retCode);
}

BERR_Code BPWM_SetTwoInputSync
(
	BPWM_ChannelHandle		hChn,			/* [in] Device channel handle */
	BPWM_TwoInputSyncType	type			/* [in] two-input sync type */
)
{
	BERR_Code	retCode = BERR_SUCCESS;
	BPWM_Handle	hDev;
	uint32_t	lval;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hPwm;

	lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL3));
	if (EVEN_CHANNEL(hChn->chnNo))
	{
		if (type == TwoInputSync_Vsync2)
		{
			lval |= BCHP_PWM_CTRL3_pwm1_two_inp_sync_MASK;
		}
		else
		{
			lval &= ~BCHP_PWM_CTRL3_pwm1_two_inp_sync_MASK;
		}
	}
	else
	{
		if (type == TwoInputSync_Vsync2)
		{
			lval |= BCHP_PWM_CTRL3_pwm2_two_inp_sync_MASK;
		}
		else
		{
			lval &= ~BCHP_PWM_CTRL3_pwm2_two_inp_sync_MASK;
		}
	}
	BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL3), lval);

	return (retCode);
}

BERR_Code BPWM_GetTwoInputSync
(
	BPWM_ChannelHandle		hChn,			/* [in] Device channel handle */
	BPWM_TwoInputSyncType	*type			/* [out] two-input sync type */
)
{
	BERR_Code	retCode = BERR_SUCCESS;
	BPWM_Handle	hDev;
	uint32_t	lval;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hPwm;

	lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL3));
	if (EVEN_CHANNEL(hChn->chnNo))
	{
		lval &= BCHP_PWM_CTRL3_pwm1_two_inp_sync_MASK;
	}
	else
	{
		lval &= BCHP_PWM_CTRL3_pwm2_two_inp_sync_MASK;
	}

	*type = ((lval == 0) ? TwoInputSync_Vsync : TwoInputSync_Vsync2);

	return (retCode);
}

BERR_Code BPWM_SetOutputPolarity
(
	BPWM_ChannelHandle	hChn,				/* [in] Device channel handle */
	BPWM_OutPolaritySel	OutPolSel			/* [in] output polarity */
)
{
	BERR_Code	retCode = BERR_SUCCESS;
	BPWM_Handle	hDev;
	uint32_t	lval;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hPwm;

	lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL2));
	if (EVEN_CHANNEL(hChn->chnNo))
	{
		if (OutPolSel == OutPolarityHigh)
		{
			lval &= ~BCHP_PWM_CTRL2_pwm1_out_polarity_MASK;
		}
		else
		{
			lval |= BCHP_PWM_CTRL2_pwm1_out_polarity_MASK;
		}
	}
	else
	{
		if (OutPolSel == OutPolarityHigh)
		{
			lval &= ~BCHP_PWM_CTRL2_pwm2_out_polarity_MASK;
		}
		else
		{
			lval |= BCHP_PWM_CTRL2_pwm2_out_polarity_MASK;
		}
	}
	BREG_Write32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL2), lval);

	return (retCode);
}

BERR_Code BPWM_GetOutputPolarity
(
	BPWM_ChannelHandle	hChn,				/* [in] Device channel handle */
	BPWM_OutPolaritySel	*OutPolSel			/* [out] output polarity */
)
{
	BERR_Code	retCode = BERR_SUCCESS;
	BPWM_Handle	hDev;
	uint32_t	lval;

	BDBG_ASSERT( hChn );
	BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

	hDev = hChn->hPwm;

	lval = BREG_Read32(hDev->hRegister, (hChn->coreOffset + BCHP_PWM_CTRL2));
	if (EVEN_CHANNEL(hChn->chnNo))
	{
		lval &= BCHP_PWM_CTRL2_pwm1_out_polarity_MASK;
	}
	else
	{
		lval &= BCHP_PWM_CTRL2_pwm2_out_polarity_MASK;
	}
	
	*OutPolSel = ((lval == 0) ? OutPolarityHigh : OutPolarityLow);

	return (retCode);
}

#endif


/* End of file */

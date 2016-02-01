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
 * Revision History:  $
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "btnr.h"
#include "btnr_priv.h"
#include "btnr_7125ib_priv.h"
#include "bchp_ds_tuner_ref_0.h"

BDBG_MODULE(btnr_7125ib_priv);

#define DEV_MAGIC_ID                    ((BERR_TNR_ID<<16) | 0xFACE)

#define CHK_RETCODE( rc, func )         \
do {                                        \
    if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
    {                                       \
        goto done;                          \
    }                                       \
} while(0)

/*******************************************************************************
*
*   Private Module Data
*
*******************************************************************************/
BERR_Code BTNR_7125_Close(
    BTNR_Handle hDev                    /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_7125_Handle btnr_7125_handle;
    BTNR_P_7125_Settings btnr_7125_settings;
	BTNR_P_7125_Handle *p7125;

    BDBG_ENTER(BTNR_7125_Close);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    /* verify the handle is good before using it */
    btnr_7125_handle = (BTNR_7125_Handle) hDev->hDevImpl;
    btnr_7125_settings = btnr_7125_handle ->settings;
	p7125 = (BTNR_P_7125_Handle *)(hDev->hDevImpl);

    hDev->magicId = 0x00;       /* clear it to catch inproper use */
	if (p7125->hTimer)
	{
		BTMR_DestroyTimer(p7125->hTimer);
	}
	if (p7125->hInterruptEvent)
	{
		BKNI_DestroyEvent(p7125->hInterruptEvent);
	}
	BMEM_Free( p7125->hHeap, p7125->pTnrModeData );
    BMEM_Free( p7125->hHeap, p7125->pTnrStatus );
    BKNI_Free( (void *) hDev->hDevImpl );
    BKNI_Free( (void *) hDev );



    BDBG_LEAVE(BTNR_7125_Close);
    return( retCode );
}

BERR_Code BTNR_7125_SetRfFreq(
    BTNR_7125_Handle hDev,                /* [in] Device handle */
    uint32_t rfFreq,                    /* [in] Requested tuner freq., in Hertz */
    BTNR_TunerMode tunerMode            /* [in] Requested tuner mode */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
	BSTD_UNUSED(tunerMode);
	BSTD_UNUSED(rfFreq);
    BDBG_ENTER(BTNR_7125_SetRfFreq);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    BDBG_LEAVE(BTNR_7125_SetRfFreq);
    return( retCode );
}
            
BERR_Code BTNR_7125_GetRfFreq(
    BTNR_7125_Handle hDev,                /* [in] Device handle */
    uint32_t *rfFreq,                   /* [output] Returns tuner freq., in Hertz */
    BTNR_TunerMode *tunerMode           /* [output] Returns tuner mode */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_P_7125_Settings *pTnrImplData;


    BDBG_ENTER(BTNR_7125_GetRfFreq);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    pTnrImplData = &hDev->settings;

    *rfFreq = pTnrImplData->rfFreq;
    *tunerMode = pTnrImplData->tunerMode;

    BDBG_LEAVE(BTNR_7125_GetRfFreq);
    return( retCode );
}

BERR_Code BTNR_P_7125_GetAgcRegVal(
    BTNR_7125_Handle hDev,        		/* [in] Device handle */
    uint32_t regOffset,                 /* [in] AGC register offset */
    uint32_t *agcVal                    /* [out] output value */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_P_7125_Settings *pTnrImplData;


    BDBG_ENTER(BTNR_P_7125_GetAgcRegVal);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    BSTD_UNUSED(regOffset);

    pTnrImplData = &hDev->settings;

    BDBG_ERR(("Not Implemented on this tuner"));
    *agcVal = 0;

    BDBG_LEAVE(BTNR_P_7125_GetAgcRegVal);
    return( retCode );
}

BERR_Code BTNR_P_7125_SetAgcRegVal(
	BTNR_7125_Handle hDev,          	/* [in] Device handle */
    uint32_t regOffset,                 /* [in] AGC register offset */
    uint32_t *agcVal                    /* [in] input value */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
	int32_t GainReg;


    BDBG_ENTER(BTNR_P_7125_SetAgcRegVal);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    BSTD_UNUSED(regOffset);

	hDev->pTnrModeData->LNA_Gain = (*agcVal & ~(BTNR_7125_LNA_GAIN_BOOST_ON | 
											    BTNR_7125_LNA_SUPERBOOST_ON |
											    BTNR_7125_LNA_TILT_ON));

	/*The LNA Gain is reported in 1/256 db*/	
	/*Get the LNA GAIN if Boost and SuperBoost are disabled*/
	GainReg = (-14 + hDev->pTnrModeData->LNA_Gain)<<8;
	BDBG_MSG(("%s() GainReg %d",__FUNCTION__,GainReg));

	/*Compenasate if Boost is enabled*/
	if (*agcVal & BTNR_7125_LNA_GAIN_BOOST_ON)
	{
		BDBG_MSG(("%s() Boost On",__FUNCTION__));
		hDev->pTnrStatus->LNA_Boost_Status = 1;
		GainReg = GainReg + 768;
		/*Compensate again if SuperBoost is enabled in addition to Boost*/
		if (*agcVal & BTNR_7125_LNA_SUPERBOOST_ON)
		{
			BDBG_MSG(("%s() Superboost On",__FUNCTION__));
			GainReg = GainReg + 768; 
			hDev->pTnrStatus->LNA_Boost_Status = 2;
		}
	}
	if (*agcVal & BTNR_7125_LNA_TILT_ON)
	{
			hDev->pTnrStatus->LNA_Tilt_Status = 1;
			BDBG_MSG(("%s() Tilt On GainReg before comp %d",__FUNCTION__,GainReg));

			/*Compensate gain if tilt is on, a 5 db rise from 500 MHz to 1 GHz*/
			if ((hDev->pTnrStatus->TunerFreq > 500000000) && (hDev->pTnrStatus->TunerFreq <= 1000000000))
			{
				GainReg = GainReg + (((hDev->pTnrStatus->TunerFreq-500000000)>>8)*5*256)/(500000000>>8); /*keep in range of 32 bits*/
			}
			else if (hDev->pTnrStatus->TunerFreq > 1000000000)
			{
				GainReg = GainReg + 1280;  /*add 5 db*/
			}
			BDBG_MSG(("%s() Tilt On GainReg after comp %d for freq %u",__FUNCTION__,GainReg,hDev->pTnrStatus->TunerFreq));
	}

	/*assign the tilt compensated gain*/
	hDev->pTnrStatus->LNA_Gain_256db = (uint32_t)GainReg;
	
	BDBG_LEAVE(BTNR_P_7125_SetAgcRegVal);
	return( retCode );
}


BERR_Code BTNR_7125_GetInfo(
    BTNR_7125_Handle hDev,                /* [in] Device handle */
    BTNR_TunerInfo *tnrInfo             /* [out] Tuner information */
    )
{   
    BTNR_P_7125_Settings *pTnrImplData;
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_7125_GetInfo);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    pTnrImplData = &hDev->settings;
    tnrInfo->tunerMaker = BRCM_TUNER_MAKER_ID;
    tnrInfo->tunerId = 0x7125;
    tnrInfo->tunerMajorVer = pTnrImplData->iRevLetter;
    tnrInfo->tunerMinorVer = pTnrImplData->iRevNumber;

    BDBG_LEAVE(BTNR_7125_GetInfo);
    return( retCode );
}

BERR_Code BTNR_7125_GetPowerSaver(
	BTNR_7125_Handle hDev,					/* [in] Device handle */
	BTNR_PowerSaverSettings *pwrSettings 		/* [in] Power saver settings. */
	)
{   
    BTNR_P_7125_Settings *pTnrImplData;
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_7125_GetPowerSaver);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    pTnrImplData = &hDev->settings;

	pwrSettings->enable = pTnrImplData->powerSaver;
	
    BDBG_LEAVE(BTNR_7125_GetPowerSaver);
    return( retCode );
}

BERR_Code BTNR_7125_SetPowerSaver(
	BTNR_7125_Handle hDev,					/* [in] Device handle */
	BTNR_PowerSaverSettings *pwrSettings /* [in] Power saver settings. */
	)
{   
    BTNR_P_7125_Settings *pTnrImplData;
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_7125_SetPowerSaver);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    pTnrImplData = &hDev->settings;
	pTnrImplData->powerSaver = pwrSettings->enable;
    if (pwrSettings->enable)
    {
        BTNR_7125_P_PowerDn_TNR_Core0(hDev);
    }
    else
    {
        BTNR_7125_P_PowerUp_TNR_Core0(hDev);
    }
	
    BDBG_LEAVE(BTNR_7125_SetPowerSaver);
    return( retCode );
}



 

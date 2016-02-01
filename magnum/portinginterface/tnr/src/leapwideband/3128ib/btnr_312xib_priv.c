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
 * Revision History:  $
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "btnr.h"
#include "btnr_priv.h"
#include "btnr_312xib_priv.h"
#include "bhab.h"
#include "../../c0/bchp_tm.h"

BDBG_MODULE(btnr_312x_priv);

#define DEV_MAGIC_ID                    ((BERR_TNR_ID<<16) | 0xFACE)

/*******************************************************************************
*
*   Private Module Functions
*
*******************************************************************************/
BERR_Code BTNR_312x_P_EventCallback_isr(
    void * pParam1, int param2
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_312xIb_Handle hDev = (BTNR_312xIb_Handle) pParam1;
    BHAB_InterruptType event = (BHAB_InterruptType) param2;

    BDBG_ENTER(BTNR_312x_P_EventCallback_isr);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
 
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->hHab);

    switch (event) {
        case BHAB_Interrupt_eSpectrumAnalyzerDataReady:      
            {
                if( hDev->pCallback[BTNR_Callback_eSpectrumDataReady] != NULL )
                {
                    (hDev->pCallback[BTNR_Callback_eSpectrumDataReady])(hDev->pCallbackParam[BTNR_Callback_eSpectrumDataReady] );
                }
            }
            break; 
        case BHAB_Interrupt_eIfDacAcquireComplete:      
            {
                if( hDev->pCallback[BTNR_Callback_eIfDacAcquireComplete] != NULL )
                {
                    (hDev->pCallback[BTNR_Callback_eIfDacAcquireComplete])(hDev->pCallbackParam[BTNR_Callback_eIfDacAcquireComplete] );
                }
            }
            break; 
        case BHAB_Interrupt_eIfDacStatusReady:      
            {
                if( hDev->pCallback[BTNR_Callback_eIfDacStatusReady] != NULL )
                {
                    (hDev->pCallback[BTNR_Callback_eIfDacStatusReady])(hDev->pCallbackParam[BTNR_Callback_eIfDacStatusReady] );
                }
            }
            break;        
        default:
            BDBG_WRN((" unknown event code from 312x"));
            break;
    }

    BDBG_LEAVE(BTNR_312x_P_EventCallback_isr);
    return retCode;
}

/*******************************************************************************
*
*   Public Module Functions
*
*******************************************************************************/
BERR_Code BTNR_312xIb_InstallCallback(
    BTNR_312xIb_Handle hDev,     /* [in] Device handle */
    BTNR_Callback callbackType, /* [in] Type of callback */
    BTNR_CallbackFunc pCallback, /* [in] Function Ptr to callback */
    void *pParam                 /* [in] Generic parameter send on callback */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_312xIb_InstallCallback);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    switch( callbackType )
    {
        case BTNR_Callback_eSpectrumDataReady:
        case BTNR_Callback_eIfDacAcquireComplete:
        case BTNR_Callback_eIfDacStatusReady:        
            hDev->pCallback[callbackType] = pCallback;
            hDev->pCallbackParam[callbackType] = pParam;
            break;
        default:
            retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
    }

    hDev->devId = BHAB_DevId_eTNR0 + hDev->channelNo;  
    BHAB_InstallInterruptCallback( hDev->hHab, hDev->devId, BTNR_312x_P_EventCallback_isr , (void *)hDev, callbackType);

    BDBG_LEAVE(BTNR_312xIb_InstallCallback);
    return retCode;
}

BERR_Code BTNR_312xIb_Close(
    BTNR_Handle hDev                    /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_312xIb_Handle btnr_312xib_handle;
    BTNR_P_312xIb_Settings btnr_312xib_settings;


    BDBG_ENTER(BTNR_312xIb_Close);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    /* verify the handle is good before using it */
    btnr_312xib_handle = (BTNR_312xIb_Handle) hDev->hDevImpl;
    btnr_312xib_settings = btnr_312xib_handle ->settings;

    hDev->magicId = 0x00;       /* clear it to catch inproper use */
    BKNI_Free( (void *) hDev->hDevImpl );
    BKNI_Free( (void *) hDev );

    BDBG_LEAVE(BTNR_312xIb_Close);
    return retCode;
}

BERR_Code BTNR_312xIb_SetRfFreq(
    BTNR_312xIb_Handle hDev,            /* [in] Device handle */
    uint32_t rfFreq,                    /* [in] Requested tuner freq., in Hertz */
    BTNR_TunerMode tunerMode            /* [in] Requested tuner mode */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_P_312xIb_Settings *pTnrImplData;
    uint8_t hab[13] = HAB_MSG_HDR(BTNR_ACQUIRE_PARAMS_WRITE, 0x8, BTNR_CORE_TYPE, BTNR_CORE_ID);
    BSTD_UNUSED(tunerMode);
    
    BDBG_ENTER(BTNR_312xIb_SetRfFreq);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    pTnrImplData = &hDev->settings;

    if(hDev->bPowerdown)
    {
        BDBG_ERR(("TNR core %d Powered Off", hDev->channelNo));
        retCode = BERR_TRACE(BTNR_ERR_POWER_DOWN);
    }
    else
    {    
        hab[4] = hDev->channelNo;
        hab[5] = BTNR_CORE_TYPE_TO_FEED;
        hab[6] = 0x80;
        hab[8] = (rfFreq >> 24);
        hab[9] = (rfFreq >> 16);
        hab[10] = (rfFreq >> 8);
        hab[11] = rfFreq;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, hab, 13, hab, 0, false, true, 13));
    }

done:
    BDBG_LEAVE(BTNR_312xIb_SetRfFreq);
    return retCode;
}
            
BERR_Code BTNR_312xIb_GetRfFreq(
    BTNR_312xIb_Handle hDev,            /* [in] Device handle */
    uint32_t *rfFreq,                   /* [output] Returns tuner freq., in Hertz */
    BTNR_TunerMode *tunerMode           /* [output] Returns tuner mode */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_P_312xIb_Settings *pTnrImplData;


    BDBG_ENTER(BTNR_312xIb_GetRfFreq);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    pTnrImplData = &hDev->settings;

    *rfFreq = pTnrImplData->rfFreq;
    *tunerMode = pTnrImplData->tunerMode;

    BDBG_LEAVE(BTNR_312xIb_GetRfFreq);
    return retCode;
}

BERR_Code BTNR_P_312xIb_GetAgcRegVal(
    BTNR_312xIb_Handle hDev,            /* [in] Device handle */
    uint32_t regOffset,                 /* [in] AGC register offset */
    uint32_t *agcVal                    /* [out] output value */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(hDev);
    BSTD_UNUSED(regOffset);
    BSTD_UNUSED(agcVal);

    return retCode;
}

BERR_Code BTNR_312xIb_SetAgcRegVal(
    BTNR_312xIb_Handle hDev,            /* [in] Device handle */
    uint32_t regOffset,                 /* [in] AGC register offset */
    uint32_t *agcVal                    /* [in] input value */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(hDev);
    BSTD_UNUSED(regOffset);
    BSTD_UNUSED(agcVal);
    
    return retCode;
}

BERR_Code BTNR_312xIb_GetInfo(
    BTNR_312xIb_Handle hDev,            /* [in] Device handle */
    BTNR_TunerInfo *tnrInfo             /* [out] Tuner information */
    )
{   
    BTNR_P_312xIb_Settings *pTnrImplData;
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_312xIb_GetInfo);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    pTnrImplData = &hDev->settings;
    tnrInfo->tunerMaker = BRCM_TUNER_MAKER_ID;
    tnrInfo->tunerId = 0x3128;       
    tnrInfo->tunerMajorVer = pTnrImplData->iRevLetter;
    tnrInfo->tunerMinorVer = pTnrImplData->iRevNumber;

    BDBG_LEAVE(BTNR_312xIb_GetInfo);
    return retCode;
}

BERR_Code BTNR_312xIb_GetPowerSaver(
    BTNR_312xIb_Handle hDev,                    /* [in] Device handle */
    BTNR_PowerSaverSettings *pwrSettings        /* [in] Power saver settings. */
    )
{   
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[9] = HAB_MSG_HDR(BTNR_POWER_STATUS_READ, 0x0, BTNR_CORE_TYPE, BTNR_CORE_ID); 
    
    BDBG_ENTER(BTNR_312xIb_GetPowerSaver);
    
    buf[3] = hDev->channelNo;     
    CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 5, buf, 9, false, true, 9));  
       
    if(buf[4]){
        pwrSettings->enable = false;
    }
    else {
        pwrSettings->enable = true;
    }

done: 
    BDBG_LEAVE(BTNR_312xIb_GetPowerSaver);
    return retCode;
}

BERR_Code BTNR_312xIb_SetPowerSaver(
    BTNR_312xIb_Handle hDev,                    /* [in] Device handle */
    BTNR_PowerSaverSettings *pwrSettings /* [in] Power saver settings. */
    )
{   
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[5] = HAB_MSG_HDR(BTNR_POWER_ON, 0x0, BTNR_CORE_TYPE, BTNR_CORE_ID); 
    uint8_t buf1[5] = HAB_MSG_HDR(BTNR_POWER_OFF, 0x0, BTNR_CORE_TYPE, BTNR_CORE_ID);

    BDBG_ENTER(BTNR_312xIb_SetPowerSaver);
    
    buf[3] = hDev->channelNo;    
    buf1[3] = hDev->channelNo;

    if(pwrSettings->enable) {
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf1, 5, buf1, 0, false, true, 5));
        hDev->bPowerdown = true;
    }
    else {
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 5, buf, 0, false, true, 5));
        hDev->bPowerdown = false;
    }
    
done:    
    BDBG_LEAVE(BTNR_312xIb_SetPowerSaver);
    return retCode;
}

BERR_Code BTNR_312xIb_GetSettings(
    BTNR_312xIb_Handle hDev,    /* [in] Device handle */
    BTNR_Settings *settings     /* [out] TNR settings. */
    )
{   
    BERR_Code retCode = BERR_SUCCESS;  

    BDBG_ENTER(BTNR_312xIb_GetSettings);
    BDBG_ASSERT( hDev );    
    
    CHK_RETCODE(retCode, BHAB_ReadRegister(hDev->hHab, BCHP_TM_SFT0, &settings->agcVal));
    
done:       
    BDBG_LEAVE(BTNR_312xIb_GetSettings);
    return retCode;
}

BERR_Code BTNR_312xIb_SetSettings(
    BTNR_312xIb_Handle hDev,    /* [in] Device handle */
    BTNR_Settings *settings     /* [in] TNR settings. */
    )

{  
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t buf;

    BDBG_ENTER(BTNR_312xIb_SetSettings);
    BDBG_ASSERT( hDev );    
    
    CHK_RETCODE(retCode, BHAB_ReadRegister(hDev->hHab, BCHP_TM_SFT0, &buf));
    
    if(buf & 0x80000000) /* If the CPPM bit is set by docsis and is being consumed by 3128(once consumed, 3128 sets this bit to 0), then update 3128 CPPM */
        settings->agcVal |= 0x80000000;
    
    CHK_RETCODE(retCode, BHAB_WriteRegister(hDev->hHab, BCHP_TM_SFT0, &settings->agcVal));

done:       
    BDBG_LEAVE(BTNR_312xIb_SetSettings);
    return retCode;
}

BERR_Code BTNR_312xIb_RequestSpectrumAnalyzerData(
    BTNR_312xIb_Handle hDev,     /* [in] Device handle */ 
    BTNR_SpectrumSettings *pSettings /* [in] spectrum settings */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    uint8_t buf[21] = HAB_MSG_HDR(BTNR_REQUEST_SPECTRUM_DATA, 0x10, BTNR_CORE_TYPE, BTNR_CORE_ID);    

    BDBG_ENTER(BTNR_312xIb_RequestSpectrumAnalyzerData);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( pSettings );

    buf[3] = hDev->channelNo;
    buf[4] = pSettings->startFreq >> 24;
    buf[5] = pSettings->startFreq >> 16;
    buf[6] = pSettings->startFreq >> 8;
    buf[7] = pSettings->startFreq;
    buf[8] = pSettings->stopFreq >> 24;
    buf[9] = pSettings->stopFreq >> 16;
    buf[10] = pSettings->stopFreq >> 8;
    buf[11] = pSettings->stopFreq;
    buf[12] = pSettings->fftSize;
    buf[16] = pSettings->numSamples >> 24;
    buf[17] = pSettings->numSamples >> 16;
    buf[18] = pSettings->numSamples >> 8;
    buf[19] = pSettings->numSamples;
    CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 21, buf, 0, false, true, 21));


done:    
    BDBG_LEAVE(BTNR_312xIb_RequestSpectrumAnalyzerData);
    return( retCode );
}    
   
BERR_Code BTNR_312xIb_GetSpectrumAnalyzerData(
    BTNR_312xIb_Handle hDev,     /* [in] Device handle */ 
    BTNR_SpectrumData  *pSpectrumData /* [out] spectrum Data*/
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[512] = HAB_MSG_HDR(BTNR_GET_SPECTRUM_DATA, 0x8, BTNR_CORE_TYPE, BTNR_CORE_ID);  
    uint16_t i;

    BDBG_ENTER(BTNR_312xIb_GetSpectrumAnalyzerData);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( pSpectrumData );
      
   
    buf[3] = hDev->channelNo;  
    CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 13, buf, 512, false, true, 512)); 

    pSpectrumData->datalength = ((((buf[1] & 0x3F) << 4) | (buf[2] >> 4) ) - 8)/4;
    pSpectrumData->moreData = (buf[4] >> 7) & 0x1;

    if(pSpectrumData->data != NULL)
    {

        for(i=0; i < pSpectrumData->datalength; i++)
        {
            *pSpectrumData->data++ = ((buf[12 + i*4] << 24) | (buf[13 + i*4] << 16)| (buf[14 + i*4] << 8)| buf[15 + i*4]);   
        }
    }

done:    
    BDBG_LEAVE(BTNR_312xIb_GetSpectrumAnalyzerData);
    return( retCode );
}

BERR_Code BTNR_312xIb_GetVersionInfo(
    BTNR_312xIb_Handle hDev,        /* [in] Device handle */
    BFEC_VersionInfo *pVersionInfo  /* [out] Returns version Info */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[29] = HAB_MSG_HDR(BTNR_SYS_VERSION, 0, BTNR_CORE_TYPE, BTNR_CORE_ID);

    BDBG_ENTER(BTNR_312xIb_GetVersionInfo);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 5, buf, 29, false, true, 29)); 
    pVersionInfo->majorVersion = (buf[4] << 8) | buf[5];
    pVersionInfo->minorVersion = (buf[6] << 8) | buf[7];
    pVersionInfo->buildType = (buf[8] << 8) | buf[9];
    pVersionInfo->buildId = (buf[10] << 8) | buf[11];
    
done:    
    BDBG_LEAVE(BTNR_312xIb_GetVersionInfo);
    return( retCode );
}

BERR_Code BTNR_312xIb_TuneIfDac(
    BTNR_312xIb_Handle hDev,        /* [in] Device handle */
    BTNR_IfDacSettings *pSettings   /* [in] IF DAC Settings */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[35] = HAB_MSG_HDR(BTNR_TUNE_IFDAC, 0x1E, BTNR_CORE_TYPE, BTNR_CORE_ID);
    uint8_t bandwidth=0;

    BDBG_ENTER(BTNR_312xIb_TuneIfDac);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    
    if(hDev->bPowerdown)
    {
        BDBG_ERR(("TNR core %d Powered Off", hDev->channelNo));
        retCode = BERR_TRACE(BTNR_ERR_POWER_DOWN);
    }
    else
    {    
        if(pSettings->bandwidth == 6000000)
            bandwidth = 1;
        buf[3] = hDev->channelNo;    
        buf[4] = hDev->channelNo;    
        buf[5] = (bandwidth << 4) | 0x8;
        buf[6] = 0x80;
        buf[8] = (pSettings->frequency >> 24);
        buf[9] = (pSettings->frequency >> 16);
        buf[10] = (pSettings->frequency >> 8);
        buf[11] = pSettings->frequency;
        buf[12] = (pSettings->outputFrequency >> 24);
        buf[13] = (pSettings->outputFrequency >> 16);
        buf[14] = (pSettings->outputFrequency >> 8);
        buf[15] = pSettings->outputFrequency;
        buf[16] = (pSettings->dacAttenuation >> 8);
        buf[17] = pSettings->dacAttenuation;    
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 35, buf, 0, false, true, 35));
        hDev->ifDacSettings = *pSettings;
    }
    
done:    
    BDBG_LEAVE(BTNR_312xIb_TuneIfDac);
    return( retCode );
}

BERR_Code BTNR_312xIb_ResetIfDacStatus(
    BTNR_312xIb_Handle hDev        /* [in] Device handle */
    )    
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[17] = HAB_MSG_HDR(BTNR_RESET_IFDAC_STATUS, 0xc, BTNR_CORE_TYPE, BTNR_CORE_ID);

    BDBG_ENTER(BTNR_312xIb_ResetIfDacStatus);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );  
    
    if(hDev->bPowerdown)
    {
        BDBG_ERR(("TNR core %d Powered Off", hDev->channelNo));
        retCode = BERR_TRACE(BTNR_ERR_POWER_DOWN);
    }
    else
    {     
        buf[3] = hDev->channelNo;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 17, buf, 0, false, true, 17));
    }
    
done:    
    BDBG_LEAVE(BTNR_312xIb_ResetIfDacStatus);
    return( retCode );
}
    
BERR_Code BTNR_312xIb_RequestIfDacStatus(
    BTNR_312xIb_Handle hDev        /* [in] Device handle */
    )    
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[17] = HAB_MSG_HDR(BTNR_REQ_IFDAC_STATUS, 0xc, BTNR_CORE_TYPE, BTNR_CORE_ID);

    BDBG_ENTER(BTNR_312xIb_RequestIfDacStatus);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );  
    if(hDev->bPowerdown)
    {
        BDBG_ERR(("TNR core %d Powered Off", hDev->channelNo));
        retCode = BERR_TRACE(BTNR_ERR_POWER_DOWN);
    }
    else
    {    
        buf[3] = hDev->channelNo;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 17, buf, 0, false, true, 17));
    }
    
done:    
    BDBG_LEAVE(BTNR_312xIb_RequestIfDacStatus);
    return( retCode );
}
    
BERR_Code BTNR_312xIb_GetIfDacStatus(
    BTNR_312xIb_Handle hDev,               /* [in] Device handle */
    BTNR_IfDacStatus *pStatus       /* [out] Returns status */
    )    
{
    BERR_Code retCode = BERR_SUCCESS;   
    uint8_t buf[121] = HAB_MSG_HDR(BTNR_GET_TUNER_STATUS, 0xc, BTNR_CORE_TYPE, BTNR_CORE_ID);

    BDBG_ENTER(BTNR_312xIb_GetIfDacStatus);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    if(hDev->bPowerdown)
    {
        BDBG_ERR(("TNR core %d Powered Off", hDev->channelNo));
        retCode = BERR_TRACE(BTNR_ERR_POWER_DOWN);
    }
    else
    {     
        pStatus->ifDacSettings = hDev->ifDacSettings;   
        buf[3] = hDev->channelNo;  
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 17, buf, 121, false, true, 121));
        pStatus->rssi = (int16_t)((buf[26] << 8) | buf[27])*100/256;
    }

done:    
    BDBG_LEAVE(BTNR_312xIb_GetIfDacStatus);
    return( retCode );
}

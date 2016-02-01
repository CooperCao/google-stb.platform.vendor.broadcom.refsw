/***************************************************************************
 *     Copyright (c) 2013-2013, Broadcom Corporation
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
 ****************************************************************************/
#include "bdcm_device.h"
BDBG_MODULE(bdcm_tnr);

/***************************************************************************
Summary:
    This structure represents a DOCSIS tuner module. There would be a tuner
    associated with each of the down stream channels (DOCSIS data and QAM),
    up stream channel and out of band channel. Each type of tuner
    is differentiated using BDCM_TnrType. Tuners associated with
    DOCSIS data channels wouldn't be controlled by the host.
****************************************************************************/
struct BDCM_Tnr
{
	BRPC_DevId devId;
    int adsTunerNum;
	BDCM_DeviceHandle hDevice;
    BDCM_TnrType type;
	uint32_t ifFreq;
	uint32_t rfFreq;
	BDCM_TnrMode tunerMode;
	BDCM_TnrPowerSaverSettings pwrSettings;
};


BERR_Code BDCM_Tnr_GetDefaultSettings(BDCM_TnrSettings *pSettings)
{
    BERR_Code retCode = BERR_SUCCESS;
    BDBG_ASSERT(pSettings);
    BDBG_ENTER(BDCM_Tnr_GetDefaultSettings);
    BKNI_Memset(pSettings,0,sizeof(*pSettings));
    BDBG_LEAVE(BDCM_Tnr_GetDefaultSettings);
    return retCode;
}

BERR_Code BDCM_Tnr_SetRfFreq(
    BDCM_TnrHandle hTnr,
    uint32_t rfFreq,
    BDCM_TnrMode tunerMode
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_TNR_SetRfFreq adsParam;
    BRPC_Param_TNR_OOB_SetRfFreq aobParam;
    BERR_Code retVal;
    unsigned proc_id=0;
    uint32_t *Param=NULL;
    unsigned numOfParams=0;

    BDBG_ENTER(BDCM_Tnr_SetRfFreq);
    BDBG_ASSERT(hTnr);
    switch(hTnr->type)
    {
    case BDCM_TnrType_eAds:
        proc_id = BRPC_ProcId_TNR_SetRfFreq;
        adsParam.devId = hTnr->devId;
        adsParam.rfFreq = rfFreq;
	    adsParam.tunerMode = tunerMode;
        hTnr->pwrSettings.enable = false;
        Param = (uint32_t *) &adsParam;
        numOfParams = sizeInLong(adsParam);
        break;
    case BDCM_TnrType_eAob:
        BSTD_UNUSED(tunerMode);
        proc_id = BRPC_ProcId_TNR_Oob_SetRfFreq;
        aobParam.devId = hTnr->devId;
	    aobParam.rfFreq = rfFreq;
        hTnr->tunerMode = BDCM_TnrMode_eDigital;
        Param = (uint32_t *) &aobParam;
        numOfParams = sizeInLong(aobParam);
        break;
    default:
        BDBG_ERR(("unsupported tuning type"));
    }

    CHK_RETCODE( retCode, BRPC_CallProc(hTnr->hDevice->hRpc,
                                        proc_id, (const uint32_t *)Param,
                                        numOfParams, NULL,
                                        0, &retVal) );
	CHK_RETCODE( retCode, retVal);
    hTnr->rfFreq = rfFreq;
    if(hTnr->type == BDCM_TnrType_eAds) 
    {
        hTnr->tunerMode = tunerMode;
    }
done:
    BDBG_LEAVE(BDCM_Tnr_SetRfFreq);
    return retCode;
}

BERR_Code BDCM_Tnr_GetRfFreq(
    BDCM_TnrHandle hTnr,
    uint32_t *rfFreq,
    BDCM_TnrMode *tunerMode
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BDBG_ENTER(BDCM_Tnr_GetRfFreq);
    BDBG_ASSERT(rfFreq);
    BDBG_ASSERT(tunerMode);
    *rfFreq = hTnr->rfFreq;
	*tunerMode = hTnr->tunerMode;
    BDBG_LEAVE(BDCM_Tnr_GetRfFreq);
    return retCode;
}

BERR_Code BDCM_Tnr_GetInfo(
    BDCM_TnrHandle hTnr,
    BDCM_TnrInfo *tnrInfo
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_XXX_Get Param;
	BRPC_Param_TNR_GetVersion outParam;
    BERR_Code retVal;

    BDBG_ENTER(BDCM_Tnr_GetInfo);
    BDBG_ASSERT(hTnr);
    BDBG_ASSERT(tnrInfo);

    if(hTnr->type != BDCM_TnrType_eAds) 
    {
        retCode = BERR_NOT_SUPPORTED;
        BDBG_ERR(("%s: not supported for OOB/UpStream tuners",__FUNCTION__));
        goto done;
    }
    Param.devId = hTnr->devId;
    CHK_RETCODE(retCode,BRPC_CallProc(hTnr->hDevice->hRpc,
                                      BRPC_ProcId_TNR_GetVersion,
                                      (const uint32_t *)&Param,
                                      sizeInLong(Param),
                                      (uint32_t *)&outParam,
                                      sizeInLong(outParam), &retVal) );
	CHK_RETCODE(retCode,retVal);

	tnrInfo->tunerMaker = outParam.manafactureId;
	tnrInfo->tunerId = outParam.modelId;
	tnrInfo->tunerMajorVer = outParam.majVer;
	tnrInfo->tunerMinorVer = outParam.minVer;
done:
    BDBG_LEAVE(BDCM_Tnr_GetInfo);
    return retCode;
}

BDCM_TnrHandle BDCM_Tnr_Open(
    void *handle,
    BDCM_TnrSettings *pSettings
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BDCM_TnrHandle hTnr = NULL;
    BDCM_DeviceHandle hDevice = (BDCM_DeviceHandle)(handle);
    BRPC_Param_TNR_Open adsParam;
    BRPC_Param_TNR_OOB_Open aobParam;
    BERR_Code retVal;
    unsigned proc_id=0;
    uint32_t *param=NULL;
    unsigned numOfParams=0;
    BDBG_ENTER(BDCM_Tnr_Open);
    BDBG_ASSERT(hDevice);
   	/* Alloc memory from the system heap */
	hTnr = (BDCM_TnrHandle) BKNI_Malloc( sizeof( struct BDCM_Tnr ) );
	if( hTnr == NULL )
	{
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("%s: BKNI_malloc() failed",__FUNCTION__));
		goto done;
	}
	BKNI_Memset( hTnr, 0x00, sizeof( struct BDCM_Tnr ) );
    switch(pSettings->type)
    {
    case BDCM_TnrType_eAds:
        proc_id = BRPC_ProcId_TNR_Open;
        if(pSettings->minVer <= 0x9) 
        {
            adsParam.devId = pSettings->adsTunerNum + BRPC_DevId_DOCSIS_TNR0;
        }
        else
        {
            adsParam.devId = pSettings->adsTunerNum + BRPC_DevId_ECM_TNR0;
        }
        adsParam.ifFreq = pSettings->ifFreq;
        param = (uint32_t *) &adsParam;
        numOfParams = sizeInLong(adsParam);
        hTnr->devId = adsParam.devId;
        break;
    case BDCM_TnrType_eAob:
        proc_id = BRPC_ProcId_TNR_Oob_Open;
        aobParam.devId = BRPC_DevId_DOCSIS_TNR0_OOB;
	    aobParam.ifFreq = pSettings->ifFreq;
        param = (uint32_t *) &aobParam;
        numOfParams = sizeInLong(aobParam);
        hTnr->devId = aobParam.devId;
        break;
    default:
        BDBG_ERR(("unsupported tuning type"));
    }
    CHK_RETCODE(retCode, BRPC_CallProc(hDevice->hRpc,
                                       proc_id,
                                       param,
                                       numOfParams,
                                       NULL, 0, &retVal));
	CHK_RETCODE(retCode, retVal);
    switch(pSettings->type)
    {
    case BDCM_TnrType_eAds:
         hDevice->hAdsTnr[pSettings->adsTunerNum] = hTnr;
         hTnr->adsTunerNum = pSettings->adsTunerNum;
         break;
    case BDCM_TnrType_eAob:
         hDevice->hAobTnr = hTnr;
         break;
    default:
        BDBG_ERR(("unsupported tuning type"));
    }
    hTnr->ifFreq = pSettings->ifFreq;
    hTnr->type = pSettings->type;
    hTnr->hDevice = hDevice;
done:
	if( retCode != BERR_SUCCESS )
	{
		if( hTnr != NULL )
		{
			BKNI_Free(hTnr);
		}
		hTnr = NULL;
	}
    BDBG_LEAVE(BDCM_Tnr_Open);
    return hTnr;
}

void BDCM_Tnr_Close(
    BDCM_TnrHandle hTnr
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_TNR_OOB_Close aobParam;
    BRPC_Param_TNR_Close adsParam;
    BDCM_DeviceHandle hDevice = hTnr->hDevice;
    BERR_Code retVal;
    unsigned proc_id=0;
    uint32_t *param=NULL;
    unsigned numOfParams=0;

    BDBG_ENTER(BDCM_Tnr_Close);
    BDBG_ASSERT(hTnr);
    switch(hTnr->type)
    {
    case BDCM_TnrType_eAds:
        proc_id = BRPC_ProcId_TNR_Close;
        adsParam.devId = hTnr->devId;
        param = (uint32_t *) &adsParam;
        numOfParams = sizeInLong(adsParam);
        break;
    case BDCM_TnrType_eAob:
        proc_id = BRPC_ProcId_TNR_Oob_Close;
        aobParam.devId = hTnr->devId;
        param = (uint32_t *)&aobParam;
        numOfParams = sizeInLong(aobParam);
        break;
    default:
        BDBG_ERR(("unsupported tuning type"));
    }
    CHK_RETCODE(retCode, BRPC_CallProc(hDevice->hRpc,
                                       proc_id,
                                       (const uint32_t *)param,
                                       numOfParams,
                                       NULL, 0, &retVal));
    CHK_RETCODE( retCode, retVal );
    switch(hTnr->type)
    {
    case BDCM_TnrType_eAds:
         hDevice->hAdsTnr[hTnr->adsTunerNum] = NULL;
         break;
    case BDCM_TnrType_eAob:
         hDevice->hAobTnr = NULL;
         break;
    default:
        BDBG_ERR(("unsupported tuning type"));
    }
    BKNI_Free( (void *) hTnr );
done:
    BDBG_LEAVE(BDCM_Tnr_Close);
    return;
}

BERR_Code BDCM_Tnr_GetTunerAgcRegVal(
    BDCM_TnrHandle hTnr,
    uint32_t *agcVal
    )
{
    BERR_Code retCode = BERR_SUCCESS;
	BRPC_Param_XXX_Get Param;
	BRPC_Param_TNR_GetAgcVal outParam;
	BERR_Code retVal;
    BDBG_ENTER(BDCM_Tnr_GetTunerAgcRegVal);
    BDBG_ASSERT(hTnr);
    if(hTnr->type != BDCM_TnrType_eAds) 
    {
        retCode = BERR_NOT_SUPPORTED;
        BDBG_ERR(("%s: not supported for OOB/UpStream tuners",__FUNCTION__));
        goto done;
    }
    *agcVal = 0;

	Param.devId = hTnr->devId;
	CHK_RETCODE( retCode, BRPC_CallProc(hTnr->hDevice->hRpc,
                                        BRPC_ProcId_TNR_GetVersion,
                                        (const uint32_t *)&Param,
                                        sizeInLong(Param),
                                        (uint32_t *)&outParam,
                                        sizeInLong(outParam),
                                        &retVal) );
	CHK_RETCODE( retCode, retVal );
	*agcVal = outParam.AgcVal;
done:
    BDBG_LEAVE(BDCM_Tnr_GetTunerAgcRegVal);
    return retCode;
}


BERR_Code BDCM_Tnr_GetPowerSaver(
    BDCM_TnrHandle hTnr,
    BDCM_TnrPowerSaverSettings *pwrSettings
    )
{
    BERR_Code retCode = BERR_SUCCESS;
	BRPC_Param_XXX_Get Param;
	BRPC_Param_TNR_GetPowerSaver outParam;
	BERR_Code retVal;
    BDBG_ENTER(BDCM_Tnr_GetPowerSaver);
    BDBG_ASSERT(hTnr);
    if(hTnr->type != BDCM_TnrType_eAds) 
    {
        retCode = BERR_NOT_SUPPORTED;
        BDBG_ERR(("%s: not supported for OOB/UpStream tuners",__FUNCTION__));
        goto done;
    }
    Param.devId = hTnr->devId;
	pwrSettings->enable = false;
	CHK_RETCODE(retCode,BRPC_CallProc(hTnr->hDevice->hRpc,
                                      BRPC_ProcId_TNR_GetPowerSaver,
                                      (const uint32_t *)&Param,
                                      sizeInLong(Param),
                                      (uint32_t *)&outParam,
                                      sizeInLong(outParam),
                                      &retVal) );
	CHK_RETCODE( retCode, retVal );
	pwrSettings->enable = outParam.powersaver;
	hTnr->pwrSettings.enable = outParam.powersaver;
done:
    BDBG_LEAVE(BDCM_Tnr_GetPowerSaver);
    return retCode;
}

BERR_Code BDCM_Tnr_SetPowerSaver(
    BDCM_TnrHandle hTnr,
    BDCM_TnrPowerSaverSettings *pwrSettings
    )
{
    BERR_Code retCode = BERR_SUCCESS;
	BRPC_Param_TNR_EnablePowerSaver Param;
	BERR_Code retVal;
    BDBG_ENTER(BDCM_Tnr_SetPowerSaver);
    BDBG_ASSERT(hTnr);
    if(hTnr->type != BDCM_TnrType_eAds) 
    {
        retCode = BERR_NOT_SUPPORTED;
        BDBG_ERR(("%s: not supported for OOB/UpStream tuners"));
        goto done;
    }

    if (!hTnr->pwrSettings.enable && pwrSettings->enable)
    {
        Param.devId = hTnr->devId;
        hTnr->pwrSettings.enable = pwrSettings->enable;
        CHK_RETCODE(retCode, BRPC_CallProc(hTnr->hDevice->hRpc,
                                            BRPC_ProcId_TNR_EnablePowerSaver,
                                            (const uint32_t *)&Param,
                                            sizeInLong(Param),
                                            NULL, 0, &retVal));
        CHK_RETCODE(retCode,retVal);
    }
    else
    {
        BDBG_MSG(("%s: Auto enabled by BDCM_Tnr_SetRfFreq",__FUNCTION__));
    }

done:
    BDBG_LEAVE(BDCM_Tnr_SetPowerSaver);
    return retCode;
}
    
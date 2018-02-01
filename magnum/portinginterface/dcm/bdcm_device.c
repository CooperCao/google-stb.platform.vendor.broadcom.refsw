/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/



#include "bdcm_device.h"

BDBG_MODULE(bdcm);

BDCM_DeviceHandle BDCM_OpenDevice(BDCM_DeviceSettings *pSettings)
{
    BERR_Code retCode = BERR_SUCCESS;
    BDCM_DeviceHandle hDevice = NULL;
    BRPC_OpenSocketImplSettings socketSettings;
    BDBG_ASSERT(pSettings);
    BDBG_ENTER(BDCM_OpenDevice);
    /* Alloc memory from the system heap */
    hDevice = (BDCM_DeviceHandle) BKNI_Malloc(sizeof(BDCM_Device) );
    if( hDevice == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("%s: BKNI_malloc() failed",BSTD_FUNCTION));
        goto error_alloc;
    }
    BKNI_Memset( hDevice, 0x00, sizeof(BDCM_Device));
    BRPC_GetDefaultOpenSocketImplSettings(&socketSettings);
    socketSettings.timeout = pSettings->rpcTimeout;
    retCode = BRPC_Open_SocketImpl(&hDevice->hRpc, &socketSettings);
    if ( retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("%s: BRPC_Open_SocketImpl failed",BSTD_FUNCTION));
        goto error_socket_open;
    }
    return hDevice;
error_socket_open:
    BKNI_Free(hDevice);
error_alloc:
    BDBG_LEAVE(BDCM_OpenDevice);
    return NULL;
}

BERR_Code BDCM_CloseDevice(BDCM_DeviceHandle hDevice)
{
    BERR_Code retCode = BERR_SUCCESS;
    BDBG_ENTER(BDCM_CloseDevice);
    BDBG_ASSERT(hDevice);
    BRPC_Close_SocketImpl(hDevice->hRpc);
    BKNI_Free((void *) hDevice);
    BDBG_LEAVE(BDCM_CloseDevice);
    return retCode;
}

BERR_Code BDCM_InitDevice(BDCM_DeviceHandle hDevice)
{
    BERR_Code retCode = BERR_SUCCESS;
    BERR_Code retVal;
    unsigned rpcInitRetryCount;
    BRPC_Param_InitSession Param;
    BDBG_ENTER(BDCM_InitDevice);
    BDBG_ASSERT(hDevice);
    BDBG_ASSERT(hDevice->hRpc);
    Param.devId = BRPC_DevId_DOCSIS;
    rpcInitRetryCount = 0;
    while (1)
    {
        #if BDBG_DEBUG_BUILD
        if (rpcInitRetryCount%20==0)
        {
            BDBG_WRN(("%s: waiting for DOCSIS bootup",BSTD_FUNCTION));
        }
        #endif
        retCode = BRPC_CallProc(hDevice->hRpc,
                                BRPC_ProcId_InitSession,
                                (const uint32_t *)&Param,
                                sizeof(Param)/4,
                                NULL, 0, &retVal);
         if (retCode == BERR_SUCCESS && retVal == BERR_SUCCESS)
         {
             break;
         }
         BKNI_Sleep(10);
         if (rpcInitRetryCount++ >= 1000)
         {
            BDBG_ERR(("%s: timeout in RPC interface initialization",BSTD_FUNCTION));
            goto done;
         }
    }
    BDBG_WRN(("%s RPC interface between DOCSIS and Host initialized",BSTD_FUNCTION));
done:
    BDBG_LEAVE(BDCM_InitDevice);
    return retCode;
}

BERR_Code BDCM_GetDeviceVersion(BDCM_DeviceHandle hDevice, BDCM_Version *pVersion)
{
    BERR_Code retCode = BERR_SUCCESS;
    BERR_Code retVal;
    BRPC_Param_ADS_GetVersion outVerParam;
    BRPC_Param_XXX_Get Param;
    BDBG_ENTER(BDCM_GetDeviceVersion);
    BDBG_ASSERT(hDevice);
    BDBG_ASSERT(pVersion);
    Param.devId = BRPC_DevId_DOCSIS;
    /* Get the version information for this device */
    CHK_RETCODE(retCode, BRPC_CallProc(hDevice->hRpc, BRPC_ProcId_ADS_GetVersion,
                                       (const uint32_t *)&Param, sizeInLong(Param),
                                       (uint32_t *)&outVerParam,
                                       sizeInLong(outVerParam), &retVal));
    CHK_RETCODE( retCode, retVal );
    pVersion->majVer = outVerParam.majVer;
    pVersion->minVer = outVerParam.minVer;
    BDBG_MSG((" DOCSIS version majVer %d minVer%d",pVersion->majVer, pVersion->minVer));
done:
    BDBG_LEAVE(BDCM_GetDeviceVersion);
    return retCode;
}

BERR_Code BDCM_GetDeviceTotalDsChannels(BDCM_DeviceHandle hDevice,unsigned int *totalChannels)
{
    BERR_Code retCode = BERR_SUCCESS;
    BERR_Code retVal;
    BRPC_Param_XXX_Get Param;
    BRPC_Param_ADS_GetTotalChannels outChnNoParam;
    BDBG_ENTER(BDCM_GetDeviceTotalChannels);
    BDBG_ASSERT(hDevice);
    Param.devId = BRPC_DevId_DOCSIS;
    /* Get the number of In-Band Downstream channels available for Video use */
    CHK_RETCODE(retCode, BRPC_CallProc(hDevice->hRpc, BRPC_ProcId_ADS_GetTotalChannels,
                                       (const uint32_t *)&Param, sizeInLong(Param),
                                       (uint32_t *)&outChnNoParam,
                                       sizeInLong(outChnNoParam), &retVal));
    CHK_RETCODE( retCode, retVal );
    hDevice->maxChannels = outChnNoParam.totalChannels;
    *totalChannels = outChnNoParam.totalChannels | (outChnNoParam.externalChannels << 16);
    BDBG_MSG((" DOCSIS return total channels is 0x%x, external 0x%x", *totalChannels, outChnNoParam.externalChannels));
done:
    BDBG_LEAVE(BDCM_GetDeviceTotalChannels);
    return retCode;
}


BERR_Code BDCM_GetDeviceBondingCapability(BDCM_DeviceHandle hDevice,unsigned int *num)
{
    BERR_Code retCode = BERR_SUCCESS;
    BERR_Code retVal = BERR_SUCCESS;
    BRPC_Param_ADS_GetBondingCapability outParam;
    BRPC_Param_XXX_Get Param;
    BDBG_ENTER(BDCM_GetDeviceBondingCapability);
    BDBG_ASSERT(hDevice);
    /* Get the bonding capability of DOCSIS */
    CHK_RETCODE(retCode, BRPC_CallProc(hDevice->hRpc, BRPC_ProcId_ADS_GetBondingCapability,
                                       (const uint32_t *)&Param, sizeInLong(Param),
                                       (uint32_t *)&outParam,
                                       sizeInLong(outParam), &retVal));
    CHK_RETCODE(retCode, retVal);

    *num = hDevice->lastBondedChannel = outParam.maxNum;
    BDBG_MSG(("bonded channels 0-%d",hDevice->lastBondedChannel));
done:
    BDBG_LEAVE(BDCM_GetDeviceBondingCapability);
    return retCode;
}

BERR_Code BDCM_SetDeviceHostChannelsStatus(
    BDCM_DeviceHandle hDevice,
    uint32_t lockStatus
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BERR_Code retVal;
    BRPC_Param_ECM_HostChannelsLockStatus Param;
    BDBG_ASSERT(hDevice);

    BDBG_ENTER(BDCM_SetDeviceHostChannelsStatus);
    Param.NonCmControlledVideoChLockStatus = lockStatus;
    CHK_RETCODE(retCode, BRPC_CallProc(hDevice->hRpc,
                                       BRPC_ProcId_ECM_HostChannelsLockStatus,
                                       (const uint32_t *)&Param,
                                       sizeInLong(Param),
                                       NULL, 0, &retVal));
    CHK_RETCODE(retCode, retVal);
    hDevice->hostChannelsStatus = lockStatus;
done:
    BDBG_MSG(("%s host channel status %x", BSTD_FUNCTION, hDevice->hostChannelsStatus));
    BDBG_LEAVE(BDCM_SetDeviceHostChannelsStatus);
    return retCode;
}

BERR_Code BDCM_GetDeviceHostChannelsStatus(
    BDCM_DeviceHandle hDevice,
    uint32_t *pLockStatus
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BDBG_ASSERT(hDevice);
    BDBG_ASSERT(pLockStatus);
    BDBG_ENTER(BDCM_GetDeviceHostChannelsStatus);
    *pLockStatus = hDevice->hostChannelsStatus;
    BDBG_MSG(("%s host channel status %x", BSTD_FUNCTION, hDevice->hostChannelsStatus));
    BDBG_LEAVE(BDCM_GetDeviceHostChannelsStatus);
    return retCode;
}


BERR_Code BDCM_ConfigureDeviceLna(BDCM_DeviceHandle hDevice)
{
    BERR_Code retCode = BERR_SUCCESS;
    BERR_Code retVal;
	BRPC_Param_ECM_DoLnaReConfig Param;
    BDBG_ENTER(BDCM_ConfigureDeviceLna);
    BDBG_ASSERT(hDevice);
    Param.devId = BRPC_DevId_DOCSIS;
    CHK_RETCODE(retCode,BRPC_CallProc(hDevice->hRpc,
                                      BRPC_ProcId_ECM_DoLnaReConfig,
                                      (const uint32_t *)&Param,
                                      sizeInLong(Param),
                                      NULL, 0, &retVal));
    CHK_RETCODE(retCode,retVal);
done:
    BDBG_LEAVE(BDCM_ConfigureDeviceLna);
    return retCode;
}

BERR_Code BDCM_GetDeviceTemperature(
    BDCM_DeviceHandle hDevice,
    BDCM_DeviceTemperature *pTemperature)
{
    BERR_Code retCode = BERR_SUCCESS;
    BERR_Code retVal;
    BRPC_Param_ECM_ReadDieTemperature outParam;
    BDBG_ASSERT(hDevice);
    BDBG_ASSERT(pTemperature);
    BDBG_ENTER(BDCM_GetDeviceTemperature);
    CHK_RETCODE(retCode, BRPC_CallProc(hDevice->hRpc,
                                       BRPC_ProcId_ECM_ReadDieTemperature,
                                       NULL, 0, (uint32_t *)&outParam,
                                       sizeInLong(outParam), &retVal));

    CHK_RETCODE(retCode,retVal) ;
    pTemperature->temperature  = outParam.TempInDot00DegC;
done:
    BDBG_LEAVE(BDCM_GetDeviceTemperature);
    return retCode;
}

BERR_Code BDCM_SetDevicePowerMode(
    BDCM_DeviceHandle hDevice,
    BDCM_DevicePowerMode powerMode)
{
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_ECM_PowerSaver Param;
    BERR_Code retVal;
    BDBG_ASSERT(hDevice);
    BDBG_ENTER(BDCM_SetDevicePowerMode);
   if (hDevice->powerMode == powerMode)
    {
        BDBG_WRN(("%s: DOCSIS already in power Mode %u",BSTD_FUNCTION,powerMode));
    	return retCode;
    }

    Param.devId = BRPC_DevId_DOCSIS;

    CHK_RETCODE(retCode, BRPC_CallProc(hDevice->hRpc,
                                       BRPC_ProcId_ECM_PowerSaver,
                                       (const uint32_t *)&Param,
                                       sizeInLong(Param),
                                       NULL, 0, &retVal));


    CHK_RETCODE(retCode,retVal);
    hDevice->powerMode = powerMode;
done:
    BDBG_LEAVE(BDCM_SetDevicePowerMode);
    return retCode;
}


BERR_Code BDCM_GetDevicePowerMode(
    BDCM_DeviceHandle hDevice,
    BDCM_DevicePowerMode *pPowerMode)
{
    BERR_Code retCode = BERR_SUCCESS;
    BDBG_ASSERT(hDevice);
    BDBG_ENTER(BDCM_GetDevicePowerMode);
    *pPowerMode = hDevice->powerMode;
    BDBG_LEAVE(BDCM_GetDevicePowerMode);
    return retCode;
}


BERR_Code BDCM_GetDeviceDataChannelStatus(
    BDCM_DeviceHandle hDevice,
    BDCM_Version version,
    unsigned dataChannelNum,
    BDCM_DataStatus *pStatus
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_XXX_Get Param;
    BRPC_Param_ECM_GetStatus outParam;
    BERR_Code retVal;
    BDBG_ASSERT(hDevice);
    BDBG_ASSERT(pStatus);
    Param.devId = ((version.minVer <= 0x9) ? BRPC_DevId_DOCSIS_DS0 : BRPC_DevId_ECM_DS0) + dataChannelNum;
    CHK_RETCODE(retCode,BRPC_CallProc(hDevice->hRpc,
                                      BRPC_ProcId_ECM_GetStatus,
                                      (const uint32_t *)&Param,
                                      sizeInLong(Param),
                                      (uint32_t *)&outParam,
                                      sizeInLong(outParam),&retVal));
    CHK_RETCODE(retCode,retVal);
    BKNI_Memcpy(pStatus,&outParam,sizeof(*pStatus));
done:
    return retCode;
}

BERR_Code BDCM_EnableCableCardOutOfBandPins(
    BDCM_DeviceHandle  hDevice, /* [in] device handle   */
    bool enabled
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_POD_CardApplyPower Param;
    BERR_Code retVal;
    BDBG_ASSERT(hDevice);
    Param.devId = BRPC_DevId_DOCSIS;
    Param.powerMode = enabled? ENABLE_POD_OUTPINS:DISABLE_POD_OUTPINS;

    CHK_RETCODE(retCode,BRPC_CallProc(hDevice->hRpc,
                                      BRPC_ProcId_POD_CardApplyPower,
                                      (const uint32_t *)&Param,
                                      sizeInLong(Param),
                                      NULL,0,&retVal));
    CHK_RETCODE(retCode,retVal);
done:
     return retCode;
}

BERR_Code BDCM_GetLnaDeviceAgcValue(
    BDCM_DeviceHandle  hDevice, /* [in] device handle   */
    BDCM_Version version,
    uint32_t *agcVal
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_XXX_Get Param;
    BRPC_Param_TNR_GetAgcVal outParam;
    BERR_Code retVal;
    BDBG_ASSERT(hDevice);
    Param.devId = (version.minVer <= 0x9) ? BRPC_DevId_DOCSIS_TNR0 : BRPC_DevId_ECM_TNR0;
    CHK_RETCODE(retCode,BRPC_CallProc(hDevice->hRpc,
                                      BRPC_ProcId_TNR_GetAgcVal,
                                      (const uint32_t *)&Param,
                                      sizeInLong(Param),
                                      (uint32_t *)&outParam,
                                      sizeInLong(outParam),&retVal));
    CHK_RETCODE(retCode,retVal);
    *agcVal = outParam.AgcVal;
    /********************************************************************************************************
    The format of 32 bit agcGain passed by 3383

    B3-B2: 16 bit chipid. Ex: 0x3412
    B1: b7b6=Output1Tilt, b5b4=Output2Tilt, b3=0, b2=SuperBoost, b1= Boost, b0=Tilt {0=OFF, 1=ON}
    B0:  Lna gain value from 0-0x1F (RDB indicates a 6 bit value but a valid gain value is only 5 bits)
    +---------------+----------------+-----------------+--------------+
    |            LnaChipId           |  T1/T2/0/S/B/T  |      AGC     |
    +---------------+----------------+-----------------+--------------+
            B3             B2                B1               B0


    Example:  Host receives LNA reading such of 0x3412561f
    Break it down:
    LnaChipId = 3412
    Output1 Stage2 Tilt: 1
    Output2 Stage2 Tilt: 1
    SuperBoost = ON
    Boost = ON
    Stage1 Tilt = OFF
    AGC = 0x1F  (max value)
    **********************************************************************************************************/
    BDBG_MSG((" %s AGC is 0x%x LNA Chip ID %#lx "
              "tilt enabled %s boost enabled %s "
              "superBoostEnabled %s output1 tilt gain %d "
              "output2 tilt gain %d",
              BSTD_FUNCTION, outParam.AgcVal >> 16, (long unsigned int)(0x1f & outParam.AgcVal),
              (outParam.AgcVal & 0x00000100)?"true":"false", (outParam.AgcVal & 0x00000200)?"true":"false",
              (outParam.AgcVal & 0x00000400)?"true":"false",((outParam.AgcVal >> 14) & 0x3),
              ((outParam.AgcVal >> 12) & 0x3)));
done:
     return retCode;
}



BERR_Code BDCM_GetDeviceSystemInfo(
    BDCM_DeviceHandle  hDevice,
    BDCM_SystemInfo *pSystemInfo)
{
	BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_ECM_GetSystemInfo outParam;
	BERR_Code retVal = 0;
	BDBG_ASSERT(hDevice);
	CHK_RETCODE(retCode,BRPC_CallProc(hDevice->hRpc,
									  BRPC_ProcId_ECM_GetSystemInfo,
									  NULL,0,
									  (uint32_t *)&outParam,
									  sizeInLong(outParam),&retVal));
	CHK_RETCODE(retCode,retVal);
	BKNI_Memcpy(pSystemInfo,&outParam,sizeof(*pSystemInfo));

done:
	return retCode;
}

BERR_Code BDCM_GetDeviceWapStatus(
    BDCM_DeviceHandle  hDevice,
    BDCM_WapInterfaceType wapIfType,
    BDCM_WapStatus *pWapStatus)
{
	BERR_Code retCode = BERR_SUCCESS;
	BRPC_Param_ECM_WapInterfaceType Param;
	BDCM_WapStatus *pOutParam = NULL;
	BERR_Code retVal = 0;
	BDBG_ASSERT(hDevice);
	Param.type = wapIfType;
	/* use malloc to avoid kernel stack overflow*/
	pOutParam = BKNI_Malloc(sizeof(BDCM_WapStatus));
	if(!pOutParam)
	{
		retCode = BERR_OUT_OF_SYSTEM_MEMORY;
		goto done;
	}
	CHK_RETCODE(retCode,BRPC_CallProc(hDevice->hRpc,
									  BRPC_ProcId_ECM_GetWapStatus,
									  (const uint32_t *)&Param,
									  sizeInLong(Param),
									  (uint32_t *)pOutParam,
									  sizeInLong(BDCM_WapStatus),&retVal));
	CHK_RETCODE(retCode,retVal);
	BKNI_Memcpy(pWapStatus,pOutParam,sizeof(*pWapStatus));
done:
	if(pOutParam)
	{
		BKNI_Free(pOutParam);
	}
	return retCode;
}

BERR_Code BDCM_GetDeviceMtaStatus(
    BDCM_DeviceHandle  hDevice,
    BDCM_MtaStatus *pMtaStatus)
{
	BERR_Code retCode = BERR_SUCCESS;
	BDCM_MtaStatus *pOutParam = NULL;
	BERR_Code retVal = 0;
	BDBG_ASSERT(hDevice);
	/* use malloc to avoid kernel stack overflow*/
	pOutParam = BKNI_Malloc(sizeof(BDCM_MtaStatus));
	if(!pOutParam)
	{
		retCode = BERR_OUT_OF_SYSTEM_MEMORY;
		goto done;
	}
	CHK_RETCODE(retCode,BRPC_CallProc(hDevice->hRpc,
									  BRPC_ProcId_ECM_GetMtaStatus,
									  NULL, 0,
									  (uint32_t *)pOutParam,
									  sizeInLong(BDCM_MtaStatus),&retVal));
	CHK_RETCODE(retCode,retVal);
	BKNI_Memcpy(pMtaStatus,pOutParam,sizeof(BDCM_MtaStatus));

done:
	if(pOutParam)
	{
		BKNI_Free(pOutParam);
	}
	return retCode;
}


BERR_Code BDCM_GetDeviceRouterStatus(
    BDCM_DeviceHandle  hDevice,
    BDCM_RouterStatus *pRouterStatus)
{
    BERR_Code retCode = BERR_SUCCESS;
    BDCM_RouterStatus OutParam;
    BERR_Code retVal = 0;
    BDBG_ASSERT(hDevice);

    CHK_RETCODE(retCode,BRPC_CallProc(hDevice->hRpc,
                                      BRPC_ProcId_ECM_GetRouterStatus,
                                      NULL, 0,
                                      (uint32_t *)&OutParam,
                                      sizeInLong(BDCM_RouterStatus),&retVal));
    CHK_RETCODE(retCode,retVal);
    BKNI_Memcpy(pRouterStatus,&OutParam,sizeof(*pRouterStatus));
done:
    return retCode;
}


#ifdef VENDOR_REQUEST
BERR_Code BDCM_GetDeviceVsi(
    BDCM_DeviceHandle  hDevice,
    BDCM_Vsi_Request *pVsiRequest,
    BDCM_Vsi *pVsi)
{
    BERR_Code retCode = BERR_SUCCESS;
	BDCM_Vsi *pOutParam = NULL;
    BERR_Code retVal = 0;
    BDBG_ASSERT(hDevice);
	/* use malloc to avoid kernel stack overflow*/
	pOutParam = BKNI_Malloc(sizeof(BDCM_Vsi));

    CHK_RETCODE(retCode,BRPC_CallProc(hDevice->hRpc,
                                      BRPC_ProcId_VEN_Request,
                                      (const uint32_t *)pVsiRequest,
                                      sizeInLong(BDCM_Vsi_Request),
                                      (uint32_t *)pOutParam,
                                      sizeInLong(BDCM_Vsi),&retVal));
    CHK_RETCODE(retCode,retVal);
    BKNI_Memcpy(pVsi,pOutParam,sizeof(BDCM_Vsi));
done:
    return retCode;
}
#endif

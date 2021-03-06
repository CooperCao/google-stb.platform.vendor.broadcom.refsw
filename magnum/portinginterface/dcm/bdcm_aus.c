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
 ***************************************************************************/
#include "bdcm_device.h"

BDBG_MODULE(bdcm_aus);

/***************************************************************************
Summary:
    This structure represents DOCSIS upstream stream channel.
    In general, there would be only one upstream channel per DOCSIS device.
****************************************************************************/
struct BDCM_AusChannel
{
    BDCM_DeviceHandle hDevice;
    BRPC_DevId devId;
    unsigned long xtalFreq;
    bool isTransmitterEnabled;
    BDCM_AusOperationMode operationMode;
    unsigned long rfFreq;                   /* RF Frequency in Hertz */
    unsigned int powerLevel;                /* in hundredth of dBmV */
    unsigned int symbolRate;                /* in baud */
    unsigned premablePatternSz;
};

BDCM_AusChannelHandle BDCM_Aus_OpenChannel(
    void *handle,
    const BDCM_AusSettings *pSettings
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BDCM_AusChannelHandle hChannel=NULL;
    BDCM_DeviceHandle hDevice = (BDCM_DeviceHandle)handle;
    BRPC_Param_AUS_Open Param;
    BERR_Code retVal;

    BDBG_ENTER(BDCM_Aus_OpenChannel);
    BDBG_ASSERT(hDevice);
    BDBG_ASSERT(pSettings);
    if(hDevice->hAus) 
    {
        BDBG_WRN(("%s AUS channel already opened hAus %ul",__FUNCTION__,hDevice->hAus));
        retCode = BERR_INVALID_PARAMETER;
        goto done;
    }

    /* Alloc memory from the system heap */
	hChannel = (BDCM_AusChannelHandle)BKNI_Malloc(sizeof(struct BDCM_AusChannel));
	if( hChannel == NULL )
	{
		retCode = BERR_OUT_OF_SYSTEM_MEMORY;
		BDBG_ERR(("%s: BKNI_malloc() failed",__FUNCTION__));
		goto done;
	}
	BKNI_Memset(hChannel, 0x00, sizeof(struct BDCM_AusChannel));
    
    Param.devId = BRPC_DevId_DOCSIS_US0;
	Param.xtalFreq = pSettings->xtalFreq;

	CHK_RETCODE(retCode, BRPC_CallProc(hDevice->hRpc,
                                       BRPC_ProcId_AUS_Open,
                                       (const uint32_t *)&Param, 
                                       sizeInLong(Param),
                                       NULL, 0, &retVal));
	CHK_RETCODE(retCode, retVal);
    hChannel->hDevice = hDevice;
    hDevice->hAus = hChannel;
    hChannel->isTransmitterEnabled = false;
    hChannel->operationMode = BDCM_AUS_OPERATIONMODE;
    hChannel->xtalFreq = pSettings->xtalFreq;
    hChannel->devId = BRPC_DevId_DOCSIS_US0;
    hChannel->powerLevel = 0;
    hChannel->premablePatternSz  = 0;
    hChannel->rfFreq = 0;
    hChannel->symbolRate = 0;
done:
	if( retCode != BERR_SUCCESS )
	{
		if( hDevice != NULL )
		{
			BKNI_Free((void *) hChannel);
		}
        hChannel = NULL;
	}
	BDBG_LEAVE(BDCM_Aus_OpenChannel);
	return hChannel;
}

BERR_Code BDCM_Aus_CloseChannel(BDCM_AusChannelHandle hChannel)
{
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_AUS_Close Param;
    BERR_Code retVal;

    BDBG_ENTER(BDCM_Aus_CloseChannel);
    BDBG_ASSERT(hChannel);
    BDBG_MSG(("%s: Aus devId %d",__FUNCTION__,hChannel->devId));
    Param.devId = hChannel->devId;
    CHK_RETCODE( retCode, BRPC_CallProc(hChannel->hDevice->hRpc,
                                        BRPC_ProcId_AUS_Close,
                                        (const uint32_t *)&Param,
                                        sizeInLong(Param), NULL,
                                        0, &retVal));
    CHK_RETCODE(retCode, retVal);
    hChannel->hDevice->hAob = NULL;
    BKNI_Free((void *)hChannel);
done:
    BDBG_LEAVE(BDCM_Aus_CloseChannel);
    return retCode;
}

BERR_Code BDCM_Aus_GetChannelDefaultSettings(
	BDCM_AusSettings *pSettings
	)
{
    BERR_Code retCode = BERR_SUCCESS;
    BDBG_ENTER(BDCM_Aus_GetChannelDefaultSettings);
    pSettings->xtalFreq = BDCM_AUS_XTALFREQ;
    BDBG_LEAVE(BDCM_Aus_GetChannelDefaultSettings);
	return retCode;
}

BERR_Code BDCM_Aus_SetChannelOperationMode(
	BDCM_AusChannelHandle hChannel,
	BDCM_AusOperationMode operationMode
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BRPC_Param_AUS_SetOperationMode Param;
	BERR_Code retVal;

	BDBG_ENTER(BDCM_Aus_SetOperationMode);
	BDBG_ASSERT(hChannel);

	switch( operationMode )
	{
		case BDCM_AusOperationMode_eAnnexA:
		case BDCM_AusOperationMode_eDvs178:
		case BDCM_AusOperationMode_eDocsis:
		case BDCM_AusOperationMode_ePod:
		case BDCM_AusOperationMode_eTestCw:
		case BDCM_AusOperationMode_eTestPn23:
		case BDCM_AusOperationMode_ePodAnnexA:
		case BDCM_AusOperationMode_ePodDvs178:

			Param.devId = hChannel->devId;
			Param.operationMode = operationMode;

			CHK_RETCODE(retCode, BRPC_CallProc(hChannel->hDevice->hRpc,
                                               BRPC_ProcId_AUS_SetOperationMode,
                                               (const uint32_t *)&Param,
                                               sizeInLong(Param),
                                               NULL, 0, &retVal));
			CHK_RETCODE(retCode, retVal);

			hChannel->operationMode = operationMode;
			break;
		default:
			retCode = BERR_INVALID_PARAMETER;
			break;
	}

done:
	BDBG_LEAVE(BDCM_Aus_SetOperationMode);
	return retCode;
}

BERR_Code BDCM_Aus_GetChannelOperationMode(
	BDCM_AusChannelHandle hChannel,
    BDCM_AusOperationMode *operationMode
	)
{
	BERR_Code retCode = BERR_SUCCESS;
    BDBG_ENTER(BDCM_Aus_GetOperationMode);
	BDBG_ASSERT(hChannel);
    *operationMode = hChannel->operationMode;
    BDBG_LEAVE(BDCM_Aus_GetOperationMode);
	return retCode;
}

BERR_Code BDCM_Aus_SetChannelSymbolRate(
    BDCM_AusChannelHandle hChannel,
    unsigned long symbolRate
    )
{  
    BERR_Code retCode = BERR_SUCCESS;
	BRPC_Param_AUS_SetSymbolRate Param;
	BERR_Code retVal;
  
	BDBG_ENTER(BDCM_Aus_SetSymbolRate);
	BDBG_MSG(("%s: symbolRate=%d", __FUNCTION__, symbolRate));
	BDBG_ASSERT(hChannel);
	Param.devId = hChannel->devId;
	Param.symbolRate = symbolRate;
	CHK_RETCODE(retCode, BRPC_CallProc(hChannel->hDevice->hRpc,
                                       BRPC_ProcId_AUS_SetSymbolRate,
                                       (const uint32_t *)&Param,
                                       sizeInLong(Param),
                                       NULL, 0, &retVal));
	CHK_RETCODE( retCode, retVal );
    hChannel->symbolRate = symbolRate;
done:
	BDBG_LEAVE(BDCM_Aus_SetSymbolRate);
	return( retCode );
}

BERR_Code BDCM_Aus_GetChannelSymbolRate(
	BDCM_AusChannelHandle hChannel,
    unsigned long *symbolRate
    )
{
	BERR_Code retCode = BERR_SUCCESS;
    BDBG_ENTER(BDCM_Aus_GetChannelSymbolRate);
	BDBG_ASSERT(hChannel);
	*symbolRate = hChannel->symbolRate;
	BDBG_LEAVE(BDCM_Aus_GetChannelSymbolRate);
	return retCode;
}

BERR_Code BDCM_Aus_SetChannelRfFreq(
	BDCM_AusChannelHandle hChannel,
    unsigned long rfFreq
    )
{
	BERR_Code retCode = BERR_SUCCESS;
	BRPC_Param_AUS_SetRfFreq Param;
	BERR_Code retVal;


	BDBG_ENTER(BDCM_Aus_SetChannelRfFreq);
	BDBG_MSG(("%s: rfFreq=%d", __FUNCTION__, rfFreq));
	BDBG_ASSERT( hChannel );

	Param.devId = hChannel->devId;
	Param.rfFreq = rfFreq;
	CHK_RETCODE(retCode, BRPC_CallProc(hChannel->hDevice->hRpc,
                                       BRPC_ProcId_AUS_SetRfFreq,
                                       (const uint32_t *)&Param,
                                       sizeInLong(Param),
                                       NULL, 0, &retVal));
	CHK_RETCODE(retCode, retVal);

	hChannel->rfFreq = rfFreq;
done:
	BDBG_LEAVE(BDCM_Aus_SetChannelRfFreq);
	return( retCode );
}

BERR_Code BDCM_Aus_getChannelRfFreq(
	BDCM_AusChannelHandle hChannel,
	unsigned long *rfFreq
    )
{
	BERR_Code retCode = BERR_SUCCESS;


	BDBG_ENTER(BDCM_Aus_getChannelRfFreq);
	BDBG_ASSERT(hChannel);
	*rfFreq = hChannel->rfFreq;
	BDBG_LEAVE(BDCM_Aus_getChannelRfFreq);
	return( retCode );
}

BERR_Code BDCM_Aus_SetChannelPowerLevel(
	BDCM_AusChannelHandle hChannel,
	unsigned int powerLevel
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BRPC_Param_AUS_SetPowerLevel Param;
	BERR_Code retVal;


	BDBG_ENTER(BDCM_Aus_SetChannelPowerLevel);
	BDBG_MSG(("%s: powerLevel=%d", __FUNCTION__, powerLevel));
	BDBG_ASSERT( hChannel );
	Param.devId = hChannel->devId;
	Param.powerLevel = powerLevel;
	CHK_RETCODE(retCode, BRPC_CallProc(hChannel->hDevice->hRpc,
                                       BRPC_ProcId_AUS_SetPowerLevel,
                                       (const uint32_t *)&Param,
                                       sizeInLong(Param),
                                       NULL, 0, &retVal));
	CHK_RETCODE(retCode, retVal);
	hChannel->powerLevel = powerLevel;

done:
	BDBG_LEAVE(BDCM_Aus_SetChannelPowerLevel);
	return( retCode );
}

BERR_Code BDCM_Aus_GetChannelPowerLevel(
	BDCM_AusChannelHandle hChannel,
	unsigned int *powerLevel
	)
{
	BERR_Code retCode = BERR_SUCCESS;


	BDBG_ENTER(BDCM_Aus_GetChannelPowerLevel);
	BDBG_ASSERT(hChannel);
	*powerLevel = hChannel->powerLevel;
	BDBG_LEAVE(BDCM_Aus_GetChannelPowerLevel);
	return retCode;
}


BERR_Code BDCM_Aus_GetChannelStatus(
    BDCM_AusChannelHandle hChannel,
	BDCM_AusStatus *pStatus
    )
{
	BERR_Code retCode = BERR_SUCCESS;
	BDBG_ENTER(BDCM_Aus_GetChannelStatus);
	BDBG_ASSERT(hChannel);
	pStatus->operationMode = hChannel->operationMode;
	pStatus->symbolRate = hChannel->symbolRate;
	pStatus->rfFreq = hChannel->rfFreq;
	pStatus->powerLevel = hChannel->powerLevel;
	pStatus->isPowerSaverEnabled = hChannel->isTransmitterEnabled;
	pStatus->sysXtalFreq = hChannel->xtalFreq;
	BDBG_LEAVE(BDCM_Aus_GetChannelStatus);
	return retCode;
}


BERR_Code BDCM_Aus_TransmitChannelStarvuePkt(
    BDCM_AusChannelHandle hChannel,
    uint8_t *ucXmitBuffer,
    unsigned int size
    )
{
    BRPC_Param_AUS_TxStarVuePkt  Param;
    int i;
    BERR_Code retVal,retCode = BERR_SUCCESS;

    BDBG_ENTER(BDCM_Aus_TransmitChannelStarvuePkt);
    BDBG_ASSERT(hChannel);
    BDBG_ASSERT(ucXmitBuffer);

    if (size != 54)
    {
        BDBG_ERR(("%s: Packet Size Must be 54 bytes!",__FUNCTION__));
        retCode = BERR_INVALID_PARAMETER;
    }
    else
    {
        Param.devId = BRPC_DevId_DOCSIS_US0;
        /* Move the buffer into an array of longs */
        for (i = 0; i < 13; i++)
        {
            Param.svBuffer[i] = (ucXmitBuffer[(i*4)  ] << 24) | (ucXmitBuffer[(i*4)+1] << 16) |
                                (ucXmitBuffer[(i*4)+2] << 8)  | (ucXmitBuffer[(i*4)+3]) ;
        }
        Param.svBuffer[i] = (ucXmitBuffer[i*4] << 24) | (ucXmitBuffer[(i*4)+1] << 16);    /* Last 2 bytes*/

        CHK_RETCODE(retCode,BRPC_CallProc(hChannel->hDevice->hRpc,
                                          BRPC_ProcId_AUS_TxStarVuePkt,
                                          (const uint32_t *)&Param,
                                          sizeInLong(Param),
                                          NULL, 0, &retVal));
        if (retVal != BERR_SUCCESS)
        {
            retCode = BERR_OUT_OF_DEVICE_MEMORY;    /* DOCSIS Aus Starvue Ring Buffer is Full, Signal Error */
        }
        BKNI_Delay(4000);	/* pace transmits to avoid overflow */
    }
done:
    BDBG_LEAVE(BDCM_Aus_TransmitChannelStarvuePkt);
    return retCode;
}

BERR_Code BDCM_Aus_EnableChannelTransmitter(
    BDCM_AusChannelHandle hChannel
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_AUS_SetSetTransmitMode Param;
    BERR_Code retVal;

    BDBG_ENTER(BDCM_Aus_EnableChannelTransmitter);
    BDBG_ASSERT(hChannel);
    Param.devId = hChannel->devId;
    Param.transmitMode = 1;
    CHK_RETCODE(retCode,BRPC_CallProc(hChannel->hDevice->hRpc,
                                      BRPC_ProcId_AUS_SetTransmitMode,
                                      (const uint32_t *)&Param,
                                      sizeInLong(Param),
                                      NULL, 0, &retVal));
    CHK_RETCODE(retCode,retVal );
    hChannel->isTransmitterEnabled = true;
    BDBG_MSG(("%s: Upstream Transmitter Enabled", __FUNCTION__));
done:
	BDBG_LEAVE(BAUS_EnableTransmitter);
	return( retCode );
}

BERR_Code BDCM_Aus_DisableChannelTransmitter(
    BDCM_AusChannelHandle hChannel
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_AUS_SetSetTransmitMode Param;
    BERR_Code retVal;
    BDBG_ENTER(BDCM_Aus_DisableChannelTransmitter);
    BDBG_ASSERT(hChannel);
    
 	Param.devId = hChannel->devId;
    Param.transmitMode = 0;
    CHK_RETCODE(retCode,BRPC_CallProc(hChannel->hDevice->hRpc,
                                      BRPC_ProcId_AUS_SetTransmitMode,
                                      (const uint32_t *)&Param,
                                      sizeInLong(Param),
                                      NULL, 0, &retVal));
    CHK_RETCODE( retCode, retVal );
    hChannel->isTransmitterEnabled = false;
    BDBG_MSG(("%s: Upstream Transmitter Disabled", __FUNCTION__));
done:
    BDBG_LEAVE(BDCM_Aus_DisableChannelTransmitter);
    return retCode;
}



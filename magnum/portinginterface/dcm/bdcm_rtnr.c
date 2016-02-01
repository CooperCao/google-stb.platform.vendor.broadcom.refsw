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
BDBG_MODULE(bdcm_rtnr);

/***************************************************************************
Summary:
    This file contains remote tuner functions for reverse rmagnum support.
****************************************************************************/
BERR_Code BDCM_Rtnr_TuneAck(
    BDCM_DeviceHandle hDevice
    )
{
    BRPC_Param_TuneAck Param;
    BERR_Code retCode, retVal;

    BDBG_ENTER(BDCM_Rtnr_TuneAck);
    BDBG_ASSERT(hDevice);
    Param.TuneAck = 1;
    CHK_RETCODE( retCode, BRPC_CallProc(hDevice->hRpc,
                                        BRPC_ProcId_ECM_TuneAck, (const uint32_t *)&Param,
                                        sizeof(Param)/4, NULL,
                                        0, &retVal) );
    CHK_RETCODE( retCode, retVal);

done:
    BDBG_LEAVE(BDCM_Rtnr_TuneAck);
    return retCode;
}


BERR_Code BDCM_Rtnr_WfeRssiAck(
    BDCM_DeviceHandle hDevice,
    uint32_t WfeRssi,
    uint32_t WfeAdcPgaGain
    )
{
    BRPC_Param_WfeRssiAck Param;
    BERR_Code retCode, retVal;

    BDBG_ENTER(BDCM_Rtnr_WfeRssiAck);
    BDBG_ASSERT(hDevice);
	Param.WfeRssi = WfeRssi;
	Param.WfeRaw2 = WfeAdcPgaGain;

    CHK_RETCODE( retCode, BRPC_CallProc(hDevice->hRpc,
                                        BRPC_ProcId_ECM_WfeRssiAck, (const uint32_t *)&Param,
                                        sizeof(Param)/4, NULL,
                                        0, &retVal) );
    CHK_RETCODE( retCode, retVal);

done:
    BDBG_LEAVE(BDCM_Rtnr_WfeRssiAck);
    return retCode;
}


BERR_Code BDCM_Rtnr_SetDevId(
    BDCM_DeviceHandle hDevice,
    uint32_t devId
    )
{
    BRPC_Param_SetDevid Param;
    BERR_Code retCode, retVal;

    BDBG_ENTER(BDCM_Rtnr_SetDevId);
    BDBG_ASSERT(hDevice);
    Param.devId = devId;
    CHK_RETCODE( retCode, BRPC_CallProc(hDevice->hRpc,
                                        BRPC_ProcId_ECM_SetDevId, (const uint32_t *)&Param,
                                        sizeof(Param)/4, NULL,
                                        0, &retVal) );
    CHK_RETCODE( retCode, retVal);

done:
    BDBG_LEAVE(BDCM_Rtnr_SetDevId);
    return retCode;
}

    

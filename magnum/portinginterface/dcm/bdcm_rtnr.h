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


#ifndef BDCM_RTNR_H__
#define BDCM_RTNR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "brpc_docsis.h"


/***************************************************************************
Summary:
	This function sends the remote tuner complete rpc to DOCSIS device.
****************************************************************************/
BERR_Code BDCM_Rtnr_TuneAck(
    BDCM_DeviceHandle hDevice
    );

/***************************************************************************
Summary:
	This function sends the remote tuner signal strength indicator and 
	ADC Gain to DOCSIS device.
****************************************************************************/
BERR_Code BDCM_Rtnr_WfeRssiAck(
    BDCM_DeviceHandle hDevice,
    uint32_t WfeRssi,
    uint32_t WfeAdcPgaGain
    );

/***************************************************************************
Summary:
	This function sets the remote tuner device ID to DOCSIS device.
****************************************************************************/
BERR_Code BDCM_Rtnr_SetDevId(
    BDCM_DeviceHandle hDevice,
    uint32_t devId
    );


#ifdef __cplusplus
}
#endif
 
#endif

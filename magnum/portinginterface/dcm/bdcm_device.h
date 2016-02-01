/***************************************************************************
 *     Copyright (c) 2013-2014, Broadcom Corporation
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


#ifndef BDCM_DEVICE_H__
#define BDCM_DEVICE_H__
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "brpc.h"
#include "brpc_plat.h"
#include "brpc_docsis.h"
#include "brpc_socket.h"
#include "bdcm_ads.h"
#include "bdcm_aob.h"
#include "bdcm_aus.h"
#include "bdcm_tnr.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BDCM_MAX_ADS_CHANNELS 40
#define BDCM_MAX_OOB_CHANNELS  1
#define BDCM_MAX_US_CHANNELS   1

#define sizeInLong(x)	(sizeof(x)/sizeof(uint32_t))
#define	CHK_RETCODE( rc, func )				\
do {										\
	if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
	{										\
		goto done;							\
	}										\
} while(0)

/***************************************************************************
Summary:
    A handle for a DOCSIS device.
****************************************************************************/
typedef struct BDCM_Device                *BDCM_DeviceHandle;


/***************************************************************************
Summary:
    This structure provided info on the DOCSIS device
****************************************************************************/
typedef struct BDCM_Version
{
    uint32_t    majVer; /* major chip revision number */
    uint32_t    minVer; /* minor chip revision number */
} BDCM_Version;

typedef enum
{ 
    BDCM_DevicePowerMode_eOn,
    BDCM_DevicePowerMode_eStandby,
    BDCM_DevicePowerMode_eMax
}BDCM_DevicePowerMode;


/**************************************************************************
Summary:
    This structure represents a DOCSIS device.
****************************************************************************/
typedef struct BDCM_Device
{
    BRPC_Handle hRpc;
    unsigned int maxChannels;       /* Number of total channels*/
    unsigned int lastBondedChannel; /* bonded channels = 0 to lastBondedChannel */
    uint32_t hostChannelsStatus;
    BDCM_AdsChannelHandle hAds[BDCM_MAX_ADS_CHANNELS];
    BDCM_AobChannelHandle hAob;
    BDCM_AusChannelHandle hAus;
    BDCM_TnrHandle hAdsTnr[BDCM_MAX_ADS_CHANNELS];
    BDCM_TnrHandle hAobTnr;
    BDCM_TnrHandle hAusTnr;
    BDCM_DevicePowerMode powerMode;
 } BDCM_Device;

typedef struct BDCM_DeviceSettings
{
    unsigned rpcTimeout;
}BDCM_DeviceSettings;

typedef struct BDCM_DeviceTemperature
{ 
    unsigned temperature;                           /* units of 1/100 degree celcius */
}BDCM_DeviceTemperature;

/***************************************************************************
Summary:
    This structure represents status of DOCSIS (Data) downstream channels.
****************************************************************************/
#define BDCM_DataStatus BRPC_Param_ECM_GetStatus

/***************************************************************************
Summary:
    This structure represents system info of DOCSIS device
****************************************************************************/
#define BDCM_SystemInfo BRPC_Param_ECM_GetSystemInfo

/***************************************************************************
Summary:
    This structure represents Wap status of DOCSIS device.
****************************************************************************/
#define BDCM_WapStatus  BRPC_Param_ECM_GetWapStatus

/***************************************************************************
Summary:
    This structure represents Wap interface type
****************************************************************************/
#define BDCM_WapInterfaceType  BRPC_ECM_WapInterfaceType

/***************************************************************************
Summary:
    This structure represents MTA status of DOCSIS device.
****************************************************************************/
#define BDCM_MtaStatus  BRPC_Param_ECM_GetMtaStatus

/***************************************************************************
Summary:
    This structure represents Router status of DOCSIS device.
****************************************************************************/
#define BDCM_RouterStatus  BRPC_Param_ECM_GetRouterStatus

#ifdef VENDOR_REQUEST
/***************************************************************************
Summary:
    This structure represents Vendor Specific Request of DOCSIS device.
****************************************************************************/
#define BDCM_Vsi_Request  BRPC_Param_VEN_Request

/***************************************************************************
Summary:
    This structure represents Vendor Specific Reply of DOCSIS device.
****************************************************************************/
#define BDCM_Vsi  BRPC_Param_VEN_Reply

#endif


/***************************************************************************
Summary:
	This function opens a DOCSIS device by creating a DOCSIS device 
        context.
****************************************************************************/
BDCM_DeviceHandle BDCM_OpenDevice(
        BDCM_DeviceSettings *pSettings
	);
/***************************************************************************
Summary:
    This function initializes a DOCSIS device by making an RPC call to the
    remote DOCSIS chip and getting DOCSIS version info. 
****************************************************************************/
BERR_Code BDCM_InitDevice(
    BDCM_DeviceHandle hDevice /* [in] Device handle */
    );


/***************************************************************************
Summary:
	This function closes a DOCSIS device by destroying a DOCSIS device
        context
****************************************************************************/
BERR_Code BDCM_CloseDevice(
	BDCM_DeviceHandle hDevice					/* [in] Device handle */
	);

/***************************************************************************
Summary:
	This function returns the saved version information.
****************************************************************************/
BERR_Code BDCM_GetDeviceVersion(
	BDCM_DeviceHandle hDevice,	        /* [in] Device handle */
	BDCM_Version *pVersion		/* [out] Returns version */
	);

/***************************************************************************
Summary:
    This function returns the number of bonded channels reserved
    for DOCSIS data channels. The DOCSIS data channels start from index=DS0 
    while the video channels start at an offset determined by
    number of bonded channels returned in bondingChannels output parameter.
****************************************************************************/
BERR_Code BDCM_GetDeviceBondingCapability(
        BDCM_DeviceHandle hDevice,                /* [in] Device handle */
        unsigned int *bondingChannels                   /* [out] Returns total number downstream channels supported */
        );

/***************************************************************************
Summary:
	This function returns the total number of channels available
    in a DOCSIS device. 
    Total Channels = DOCSIS data channels + Reserved video channels
****************************************************************************/
BERR_Code BDCM_GetDeviceTotalDsChannels(
	BDCM_DeviceHandle hDevice,					/* [in] Device handle */
	unsigned int *totalDsChannels			/* [out] Returns total number downstream channels supported */
	);

/***************************************************************************
Summary:
    This function communicates the lockstatus bit map for host channels'
    lock status to the DOCSIS device.
****************************************************************************/
BERR_Code BDCM_SetDeviceHostChannelsStatus(
    BDCM_DeviceHandle hDevice, /* [in] Device handle */
    uint32_t lockStatus /* [in] host channels lock status bit map */
    );

/***************************************************************************
Summary:
    This function gets the lockstatus bit map for host channels'
    lock status from the DOCSIS device.
****************************************************************************/
BERR_Code BDCM_GetDeviceHostChannelsStatus(
    BDCM_DeviceHandle hDevice, /* [in] Device handle */
    uint32_t *pLockStatus /* [out] host channels lock status bit map */
    );

/***************************************************************************
Summary:
    This function initiates a manual LNA reconfiguration which consists
    of a CPPM calculation. An LNA reconfiguration is automatically
    performed by a DOCSIS device with every 20th tune attempt on the
    condition that the 20th tune attempt fails. This function provides
    a mechanism to additionally perform this reconfiguration at any
    arbitrary point. However, the auto reconfiguration can be turned
    off by completely by calling BDCM_SetDeviceHostChannelsStatus() 
    and passing in a non-zero "lockStatus" value. It can be turned on
    again by calling this same function with a value of zero.
****************************************************************************/
BERR_Code BDCM_ConfigureDeviceLna(
    BDCM_DeviceHandle hDevice /* [in] Device handle */
    );

BERR_Code BDCM_GetDeviceTemperature(
    BDCM_DeviceHandle hDevice,
    BDCM_DeviceTemperature *pTemperature
    );

BERR_Code BDCM_SetDevicePowerMode(
    BDCM_DeviceHandle hDevice, /* [in] Device handle */
    BDCM_DevicePowerMode powerMode /* [in] Power Mode */
    );


BERR_Code BDCM_GetDevicePowerMode(
    BDCM_DeviceHandle hDevice, /* [in] Device Handle */
    BDCM_DevicePowerMode *pPowerMode /* [out] Power Mode */
    );

/***************************************************************************
Summary:
	This function gets the status of a DOCSIS data channel.

****************************************************************************/
BERR_Code BDCM_GetDeviceDataChannelStatus(
    BDCM_DeviceHandle  hDevice, /* [in] device handle   */
    BDCM_Version version,       /* [in] device version  */
    unsigned dataChannelNum,    /* [in] data channel num */ 
    BDCM_DataStatus *pStatus	/* [out] Returns status */
    );

/***************************************************************************
Summary:
	Enable/disable(tristate) the Cable Card pins (CC_DRX & CC_CRX & CC_CTX) 
    that are specifically used for an OOB channel. These pins should enabled 
    when Cable Card plug in/out is detected in the application 
****************************************************************************/
BERR_Code BDCM_EnableCableCardOutOfBandPins(
    BDCM_DeviceHandle  hDevice, /* [in] device handle   */
    bool enabled
    );

/***************************************************************************
Summary 
     Get AGC value from the LNA device controlled by a
     DOCSIS device
****************************************************************************/
BERR_Code BDCM_GetLnaDeviceAgcValue(
    BDCM_DeviceHandle  hDevice, /* [in] device handle   */
    BDCM_Version versionInfo,
    uint32_t *agcVal
    );

/***************************************************************************
Summary:
	This function gets the status of a DOCSIS data channel.

****************************************************************************/
BERR_Code BDCM_GetDeviceSystemInfo(
    BDCM_DeviceHandle  hDevice, /* [in] device handle   */
    BDCM_SystemInfo *pSystemInfo /* [out] Returns system info */
    );

/***************************************************************************
Summary:
	This function gets the WAP status of a DOCSIS data channel.

****************************************************************************/
BERR_Code BDCM_GetDeviceWapStatus(
    BDCM_DeviceHandle  hDevice, /* [in] device handle   */
    BDCM_WapInterfaceType wapIfType, /* [in] */
    BDCM_WapStatus *pWapStatus	/* [out] Returns WAP status */
    );

/***************************************************************************
Summary:
	This function gets the MTA status of a DOCSIS data channel.

****************************************************************************/
BERR_Code BDCM_GetDeviceMtaStatus(
    BDCM_DeviceHandle  hDevice,
    BDCM_MtaStatus *pMtaStatus
    );

/***************************************************************************
Summary:
	This function gets the Router status of a DOCSIS data channel.

****************************************************************************/
BERR_Code BDCM_GetDeviceRouterStatus(
    BDCM_DeviceHandle  hDevice,
    BDCM_RouterStatus *pRouterStatus
    );

#ifdef VENDOR_REQUEST
/***************************************************************************
Summary:
	This function gets the Vendor Specific Request of the DOCSIS device.

****************************************************************************/
BERR_Code BDCM_GetDeviceVsi(
    BDCM_DeviceHandle  hDevice,
    BDCM_Vsi_Request *pVsiRequest,
    BDCM_Vsi *pVsi
    );
#endif

#ifdef __cplusplus
}
#endif

#endif




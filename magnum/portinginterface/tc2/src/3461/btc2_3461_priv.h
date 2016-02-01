/***************************************************************************
 *     Copyright (c) 2005-2012, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BTC2_3461_PRIV_H__
#define BTC2_3461_PRIV_H__

#include "bchp.h"
#include "breg_mem.h"
#include "bint.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "berr_ids.h"
#include "btc2.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Defines raw HAB test mesg hdr (struct) */
#define HAB_MSG_HDR(OPCODE,N,CORE_TYPE,CORE_ID) \
    { ((uint8_t)(((uint16_t)(OPCODE)) >> 2)), \
    (((uint8_t)(0x03 & (OPCODE)) << 6) | ((uint8_t)((N)>>4))), \
    ((((uint8_t)(((N)& 0x0F) << 4))) | ((uint8_t)(0x0F & (CORE_TYPE)))), \
    ((uint8_t)(CORE_ID)) }
    
#define BTC2_CORE_TYPE		                0x6
#define BTC2_CORE_ID		                0x0
#define CORE_TYPE_GLOBAL	                0x0

#if (BTC2_3461_VER == BCHP_VER_A0)
#define BTC2_L1PLP_BUF_MAX                  124
#define HAB_MEM_SIZE                        128
#define BTC2_L1PLP_PKT_0_PLP_COUNT          28
#define BTC2_L1PLP_PKT_1_N_PLP_COUNT        29
#else
#define BTC2_L1PLP_BUF_MAX                  512
#define HAB_MEM_SIZE                        512
#define BTC2_L1PLP_PKT_0_PLP_COUNT          124
#define BTC2_L1PLP_PKT_1_N_PLP_COUNT        125
#define BTC2_SEL_L1PLP_PKT_0_PLP_COUNT      123
#define BTC2_SEL_L1PLP_PKT_1_N_PLP_COUNT    124
#endif
#define BTC2_CONFIG_PARAMS_BUF1             0x18
#define BTC2_CONFIG_PARAMS_BUF2             0x40
#define BTC2_CONFIG_PARAMS_BUF3             0

typedef enum BTC2_OpCodesDS{
    BTC2_eAcquire = 0x10,
    BTC2_eAcquireParamsWrite = 0x11,
    BTC2_eAcquireParamsRead = 0x91,
    BTC2_eAnnexASymbolRateWrite = 0x12,
    BTC2_eAnnexASymbolRateRead = 0x92,
    BTC2_eScanParamsWrite = 0x13,
    BTC2_eScanParamsRead = 0x93,
    BTC2_eAcqWordsWrite = 0x14,
    BTC2_eAcqWordsRead = 0x94,
    BTC2_eResetStatus = 0x15,
    BTC2_eRequestAsyncStatus = 0x16,
    BTC2_eGetAsyncStatus = 0x96,
    BTC2_eL1PreStatus = 0x1C0,
    BTC2_eL1PostConfigurableStatus = 0x1D0,
    BTC2_eL1PostDynamicStatus = 0x1E0,
    BTC2_eL1PlpStatus = 0x142,        
    BTC2_eGetConstellation = 0xA3,
    BTC2_eGetVersion = 0xB9,
    BTC2_eGetVersionInfo = 0xBA,
    BTC2_ePowerCtrlOn = 0x19,
    BTC2_ePowerCtrlOff = 0x18,
    BTC2_ePowerCtrlRead = 0x98,
    BTC2_eResetSelectiveAsyncStatus = 0x55,
    BTC2_eRequestSelectiveAsyncStatus = 0x56,
    BTC2_eGetSelectiveAsyncStatusReadyType = 0xD7,
    BTC2_eGetSelectiveAsyncStatus = 0xD6,
    BTC2_eConfigParamsWrite = 0x1A  
}BTC2_OpCodesDS;


/***************************************************************************
Summary:
    This function gets the L1 Pre status synchronously of TC2 module channel.

Description:
    This function is responsible for getting the L1 Pre status synchronously for
    a TC2 module channel.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BTC2_3461_P_GetL1PreStatus(
    BTC2_ChannelHandle hChn,            /* [in] Device channel handle */
    BTC2_Status *pStatus                /* [out] Returns status */
    );

/***************************************************************************
Summary:
    This function gets the L1 Post Configurable status synchronously of TC2 module channel.

Description:
    This function is responsible for getting the L1 Post Configurable status synchronously for
    a TC2 module channel.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BTC2_3461_P_GetL1PostConfigurableStatus(
    BTC2_ChannelHandle hChn,            /* [in] Device channel handle */
    BTC2_Status *pStatus                /* [out] Returns status */
    );

/***************************************************************************
Summary:
    This function gets the L1 Post Dynamic status synchronously of TC2 module channel.

Description:
    This function is responsible for getting the L1 Post Dynamic status synchronously for
    a TC2 module channel.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BTC2_3461_P_GetL1PostDynamicStatus(
    BTC2_ChannelHandle hChn,            /* [in] Device channel handle */
    BTC2_Status *pStatus                /* [out] Returns status */
    );

/***************************************************************************
Summary:
    This function gets the L1 PLP status synchronously of TC2 module channel.

Description:
    This function is responsible for getting the L1 PLP status synchronously for
    a TC2 module channel.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BTC2_3461_P_GetL1PLPStatus(
    BTC2_ChannelHandle hChn,            /* [in] Device channel handle */
    BTC2_Status *pStatus                /* [out] Returns status */
    );

/***************************************************************************
Summary:
    This function opens TC2 module.

Description:
    This function is responsible for opening BTC2 module. When BTC2 is
    opened, it will create a module handle and configure the module based
    on the default settings. Once the device is opened, it must be closed
    before it can be opened again.

Returns:
    TODO:

See Also:
    BTC2_3461_Close(), BTC2_3461_OpenChannel(), BTC2_3461_CloseChannel(),
    BTC2_3461_GetDefaultSettings()

****************************************************************************/
BERR_Code BTC2_3461_Open(
    BTC2_Handle *pTc2,                  /* [out] Returns handle */
    BCHP_Handle hChip,                  /* [in] Chip handle */
    BREG_Handle hRegister,              /* [in] Register handle */
    BINT_Handle hInterrupt,             /* [in] Interrupt handle, Bcm3250 */
    const struct BTC2_Settings *pDefSettings    /* [in] Default settings */
    );

/***************************************************************************
Summary:
    This function closes TC2 module.

Description:
    This function is responsible for closing BTC2 module. Closing BTC2
    will free main BTC2 handle. It is required that all opened
    BDQS channels must be closed before calling this function. If this
    is not done, the results will be unpredicable.

Returns:
    TODO:

See Also:
    BTC2_3461_Open(), BTC2_3461_CloseChannel()

****************************************************************************/
BERR_Code BTC2_3461_Close(
    BTC2_Handle hDev                    /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function initialize TC2 module.

Description:
    This function is responsible for initializing BTC2 module. The initialize
    function may do none or one or more of the following:
    - Download FW into TC2 core
    - Startup TC2 core
    - Retrieve information from TC2 core regarding TC2 static configuration
    - etc

Returns:
    TODO:

See Also:
    BTC2_3461_Open(), BTC2_3461_Close()

****************************************************************************/
BERR_Code BTC2_3461_Init(
    BTC2_Handle hDev                    /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function returns the version information.

Description:
    This function is responsible for returning the version information.

Returns:
    TODO:

See Also:
    BTC2_3461_Open()

****************************************************************************/
BERR_Code BTC2_3461_GetVersion(
    BTC2_Handle hDev,                   /* [in] Device handle */
    BTC2_Version *pVersion              /* [out] Returns version */
    );
    
/***************************************************************************
Summary:
    This function returns the version information.

Description:
    This function is responsible for returning the core driver version 
    information. It return the majorVersion and minorVersion of the core
    driver.
Returns:
    TODO:

See Also:
    BTC2_3461_Open()

****************************************************************************/    
BERR_Code BTC2_3461_GetVersionInfo(
    BTC2_Handle hDev,                   /* [in] Device handle */
    BFEC_VersionInfo *pVersionInfo /* [out] Returns version Info */
    );       

/***************************************************************************
Summary:
    This function returns the total number of channels supported by
    TC2 module.

Description:
    This function is responsible for getting total number of channels
    supported by BTC2 module, since BTC2 device is implemented as a
    device channel.

Returns:
    TODO:

See Also:
    BTC2_3461_OpenChannel(), BTC2_3461_ChannelDefaultSettings()

****************************************************************************/
BERR_Code BTC2_3461_GetTotalChannels(
    BTC2_Handle hDev,                   /* [in] Device handle */
    unsigned int *totalChannels         /* [out] Returns total number downstream channels supported */
    );

/***************************************************************************
Summary:
    This function opens TC2 module channel.

Description:
    This function is responsible for opening BTC2 module channel. When a
    BTC2 channel is opened, it will create a module channel handle and
    configure the module based on the channel default settings. Once a
    channel is opened, it must be closed before it can be opened again.

Returns:
    TODO:

See Also:
    BTC2_3461_CloseChannel(), BTC2_3461_GetChannelDefaultSettings()

****************************************************************************/
BERR_Code BTC2_3461_OpenChannel(
    BTC2_Handle hDev,                   /* [in] Device handle */
    BTC2_ChannelHandle *phChn,          /* [out] Returns channel handle */
    unsigned int channelNo,             /* [in] Channel number to open */
    const struct BTC2_ChannelSettings *pChnDefSettings /* [in] Channel default setting */
    );

/***************************************************************************
Summary:
    This function closes TC2 module channel.

Description:
    This function is responsible for closing BTC2 module channel. Closing
    BTC2 channel it will free BTC2 channel handle. It is required that all
    opened BDQS channels must be closed before closing BTC2.

Returns:
    TODO:

See Also:
    BTC2_3461_OpenChannel(), BTC2_3461_CloseChannel()

****************************************************************************/
BERR_Code BTC2_3461_CloseChannel(
    BTC2_ChannelHandle hChn             /* [in] Device channel handle */
    );

/***************************************************************************
Summary:
    This function gets TC2 module device handle based on
    the device channel handle.

Description:
    This function is responsible returning BTC2 module handle based on the
    BTC2 module channel.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BTC2_3461_GetDevice(
    BTC2_ChannelHandle hChn,            /* [in] Device channel handle */
    BTC2_Handle *pQds                   /* [out] Returns Device handle */
    );


/***************************************************************************
Summary:
    This function gets default setting for a TC2 module channel.

Description:
    This function is responsible for returning the default setting for
    channel of BTC2. The return default setting is used when opening
    a channel.

Returns:
    TODO:

See Also:
    BTC2_3461_OpenChannel()

****************************************************************************/
BERR_Code BTC2_3461_GetChannelDefaultSettings(
    BTC2_Handle hDev,                   /* [in] Device handle */
    unsigned int channelNo,             /* [in] Channel number to default setting for */
    BTC2_ChannelSettings *pChnDefSettings /* [out] Returns channel default setting */
    );

/***************************************************************************
Summary:
    This function gets the status synchronously of TC2 module channel.

Description:
    This function is responsible for getting the complete status synchronously for
    a TC2 module channel.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BTC2_3461_GetStatus(
    BTC2_ChannelHandle hChn,            /* [in] Device channel handle */
    BTC2_Status *pStatus                /* [out] Returns status */
    );

/***************************************************************************
Summary:
    This function requests the status asynchronously of TC2 module channel.

Description:
    This function is responsible for requesting the status to be calculated asynchronously for
    a TC2 module channel. The Qam frontend is responsible to inform 
    the backend when the status is ready either through an interrupt or by any other predetermined
    method.
    
Returns:
    TODO:

See Also: BTC2_GetStatus

****************************************************************************/
BERR_Code BTC2_3461_RequestAsyncStatus(
    BTC2_ChannelHandle hChn            /* [in] Device channel handle */
    );

/***************************************************************************
Summary:
    This function gets the status asynchronously of TC2 module channel.

Description:
    This function is responsible for getting the complete status asynchronously for
    a TC2 module channel.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BTC2_3461_GetAsyncStatus(
    BTC2_ChannelHandle hChn,            /* [in] Device channel handle */
    BTC2_Status *pStatus                /* [out] Returns status */   
    );

/***************************************************************************
Summary:
    This function gets the lock status of TC2 module channel.

Description:
    This function is responsible for getting the lock status for a TC2 module channel.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BTC2_3461_GetLockStatus(
    BTC2_ChannelHandle hChn,            /* [in] Device channel handle */
    BTC2_LockStatus *pLockStatus         /* [out] Returns lock status */
    );


/***************************************************************************
Summary:
    This function gets the I and Q values for soft decision of a
    TC2 module channel.

Description:
    This function is responsible for getting the I and Q values for soft
    decision of a BTC2 module channel.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BTC2_3461_GetSoftDecision(
    BTC2_ChannelHandle hChn,            /* [in] Device channel handle */
    int16_t nbrToGet,                   /* [in] Number values to get */
    int16_t *ival,                      /* [out] Ptr to array to store output I soft decision */
    int16_t *qVal,                      /* [out] Ptr to array to store output Q soft decision */
    int16_t *nbrGotten                  /* [out] Number of values gotten/read */
    );


/***************************************************************************
Summary:
    This function installs a callback function for Lock State Change event.

Description:
    This function is responsible for installing a callback function for
    Lock State Change event.  The application code should use this function
    to install a callback function, which will be called when the
    TC2 channel changes lock state.
    A lock state change is defined at switching from Lock-Unlock or Unlock-Lock.
    To determine the current lock state, a call to BTC2_GetAsyncStatus() is
    required.

    Note: It is "highly" recommended that the callback function do the minimum
    require to notify the application of this event, such sent a message or
    fire an event.  This callback function may be called from an
    interrupt context.  Please use with caution.

Returns:
    TODO:

See Also:
    BTC2_3461_GetAsyncStatus()

****************************************************************************/
BERR_Code BTC2_3461_InstallCallback(
    BTC2_ChannelHandle hChn,            /* [in] Device channel handle */
    BTC2_Callback callbackType,         /* [in] type of Callback */
    BTC2_CallbackFunc pCallbackFunc,    /* [in] Pointer to completion callback. */
    void *pParam                        /* [in] Pointer to callback user data. */
    );
    
/***************************************************************************
Summary:
    This function sends the acquire parameters for a specific TC2 module.

Description:
    This function sends the acquire parameters for a specific TC2 module.
    This sets the acquire parameters for a specific acquire.

Returns:
    TODO:

See Also:
    BTC2_GetAsyncStatus(), BTC2_GetSoftDecision()

****************************************************************************/
BERR_Code BTC2_3461_SetAcquireParams(
    BTC2_ChannelHandle hChn ,           /* [in] Device channel handle */
    BTC2_InbandParam *ibParamss          /* [in] Inband Parameters to use */
    );

/***************************************************************************
Summary:
    This function retrieves the acquire parameters set for a specific TC2 module.

Description:
    This function gets the acquire parameters for a specific TC2 module.

Returns:
    TODO:

See Also:
    BTC2_GetAsyncStatus(), BTC2_GetSoftDecision()

****************************************************************************/ 
BERR_Code BTC2_3461_GetAcquireParams(
    BTC2_ChannelHandle hChn ,           /* [in] Device channel handle */
    BTC2_InbandParam *ibParamss          /* [out] Inband Parameters to use */
    );

    
/***************************************************************************
Summary:
    This function tries to acquire downstream lock for the specific
    TC2 module channel.

Description:
    This function is responsible for trying to acquire downstream lock of
    the input IF signal. Acquiring downstream lock involves configuring
    the H/W to desire configuration, then running a TC2
    acquisition script. If this is the first acquisition for the current
    annex mode, then a TC2 configuration script will be run
    prior to running acquisition script.
    This function will automatically enable the downstream receiver if
    the receiver was in power-saver mode.

Returns:
    TODO:

See Also:
    BTC2_3461_GetLock(), BTC2_3461_GetStatus(), BTC2_3461_GetSoftDecision()

****************************************************************************/
BERR_Code BTC2_3461_Acquire(
    BTC2_ChannelHandle hChn,             /* [in] Device channel handle */
    BTC2_InbandParam *ibParams           /* [in] Inband Parameters to use */
    );


/***************************************************************************
Summary:
    This function enable the power-saver mode.

Description:
    This function is responsible for enabling the downstream receiver
    power-saver mode.  When the BTC2 is in the power-saver mode, the
    TC2 receiver is shutdown.

Returns:
    TODO:

See Also:
    BTC2_3461_Acquire()

****************************************************************************/
BERR_Code BTC2_3461_EnablePowerSaver(
    BTC2_ChannelHandle hChn             /* [in] Device channel handle */
    );

/***************************************************************************
Summary:
    This function disables the power-saver mode.

Description:
    This function is responsible for disabling the downstream receiver
    power-saver mode.  When the BTC2 is in the power-saver mode, the
    TC2 receiver is shutdown.

Returns:
    TODO:

See Also:
    BTC2_3461_EnablePowerSaver()
    BTC2_3461_Acquire()

****************************************************************************/
BERR_Code BTC2_3461_DisablePowerSaver(
    BTC2_ChannelHandle hChn             /* [in] Device channel handle */
    );

/***************************************************************************
Summary:
    This function is responsible for processing a notificiation for the specific
    TC2 module channel.

Description:
    This function needs to called when notification is received.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BTC2_3461_ProcessNotification(
    BTC2_ChannelHandle hChn,            /* [in] Device channel handle */
    unsigned int event                  /* [in] Event code and event data*/
    );


/***************************************************************************
Summary:
    This function resets the 3461 device's FEC bit error and block counters.

Description:
    
Returns:
    TODO:

See Also:   
****************************************************************************/
BERR_Code BTC2_3461_ResetStatus(
    BTC2_ChannelHandle hChn             /* [in] Device channel handle */
    );

/***************************************************************************
Summary:
    This function performs an i2c read from the slave attached to the 3461's i2c bus.

Description:
    
Returns:
    TODO:

See Also:   
****************************************************************************/
BERR_Code BTC2_3461_ReadSlave(
    BTC2_ChannelHandle hChn,     /* [in] Device channel handle */
	uint8_t chipAddr,			 /* [in] chip addr of the i2c slave device */
	uint32_t subAddr,			 /* [in] sub addr of the register to read from the slave device */
	uint8_t subAddrLen,			 /* [in] how many bytes is the sub addr? one to four*/
	uint32_t *data,				 /* [out] ptr to the data that we will read from the slave device */
	uint8_t dataLen				 /* [in] how many bytes are we going to read? one to four*/
    );

/***************************************************************************
Summary:
    This function performs an i2c write to the slave attached to the 3461's i2c bus.

Description:
    
Returns:
    TODO:

See Also:   
****************************************************************************/
BERR_Code BTC2_3461_WriteSlave(
    BTC2_ChannelHandle hChn,     /* [in] Device channel handle */
	uint8_t chipAddr,			 /* [in] chip addr of the i2c slave device */
	uint32_t subAddr,			 /* [in] sub addr of the register to read from the slave device */
	uint8_t subAddrLen,			 /* [in] how many bytes is the sub addr? one to four*/
	uint32_t *data,				 /* [in] ptr to the data that we will write to the slave device */
	uint8_t dataLen				 /* [in] how many bytes are we going to write? one to four*/
    );

/*******************************************************************************
Summary:
   This function requests the TC2 selective status of the requested type. 
Description:
  
Returns:
   BERR_Code
********************************************************************************/
BERR_Code BTC2_3461_RequestSelectiveAsyncStatus(
    BTC2_ChannelHandle hChn,
    BTC2_SelectiveAsyncStatusType type
);

/*******************************************************************************
Summary:
   This function returns the TC2 selective status ready type of the statuses that
   are ready.
Description:
  
Returns:
   BERR_Code
********************************************************************************/
BERR_Code BTC2_3461_GetSelectiveAsyncStatusReadyType(
    BTC2_ChannelHandle hChn,
    BTC2_SelectiveAsyncStatusReady *ready
);

/*******************************************************************************
Summary:
   This function gets the TC2 Selective status of the requested type.
Description:
  
Returns:
   BERR_Code
********************************************************************************/
BERR_Code BTC2_3461_GetSelectiveAsyncStatus(
    BTC2_ChannelHandle hChn,
    BTC2_SelectiveAsyncStatusType type,
    BTC2_SelectiveStatus *pStatus   /* [out] */
);
    
#ifdef __cplusplus
}
#endif

#endif




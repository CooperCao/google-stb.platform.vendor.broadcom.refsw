/***************************************************************************
 *     Copyright (c) 2005-2011, Broadcom Corporation
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
#ifndef BADS_31XX_PRIV_H__
#define BADS_31XX_PRIV_H__

#include "bchp.h"
#include "breg_mem.h"
#include "bint.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "berr_ids.h"
#include "bads.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    This function opens Qam In-Band Downstream module.

Description:
    This function is responsible for opening BADS module. When BADS is
    opened, it will create a module handle and configure the module based
    on the default settings. Once the device is opened, it must be closed
    before it can be opened again.

Returns:
    TODO:

See Also:
    BADS_31xx_Close(), BADS_31xx_OpenChannel(), BADS_31xx_CloseChannel(),
    BADS_31xx_GetDefaultSettings()

****************************************************************************/
BERR_Code BADS_31xx_Open(
    BADS_Handle *pAds,                  /* [out] Returns handle */
    BCHP_Handle hChip,                  /* [in] Chip handle */
    BREG_Handle hRegister,              /* [in] Register handle */
    BINT_Handle hInterrupt,             /* [in] Interrupt handle, Bcm3250 */
    const struct BADS_Settings *pDefSettings    /* [in] Default settings */
    );

/***************************************************************************
Summary:
    This function closes Qam In-Band Downstream module.

Description:
    This function is responsible for closing BADS module. Closing BADS
    will free main BADS handle. It is required that all opened
    BDQS channels must be closed before calling this function. If this
    is not done, the results will be unpredicable.

Returns:
    TODO:

See Also:
    BADS_31xx_Open(), BADS_31xx_CloseChannel()

****************************************************************************/
BERR_Code BADS_31xx_Close(
    BADS_Handle hDev                    /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function initialize Qam In-Band Downstream module.

Description:
    This function is responsible for initializing BADS module. The initialize
    function may do none or one or more of the following:
    - Download FW into ADS core
    - Startup ADS core
    - Retrieve information from ADS core regarding ADS static configuration
    - etc

Returns:
    TODO:

See Also:
    BADS_31xx_Open(), BADS_31xx_Close()

****************************************************************************/
BERR_Code BADS_31xx_Init(
    BADS_Handle hDev                    /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function returns the version information.

Description:
    This function is responsible for returning the version information.

Returns:
    TODO:

See Also:
    BADS_31xx_Open()

****************************************************************************/
BERR_Code BADS_31xx_GetVersion(
    BADS_Handle hDev,                   /* [in] Device handle */
    BADS_Version *pVersion              /* [out] Returns version */
    );

/***************************************************************************
Summary:
    This function returns the total number of channels supported by
    Qam In-Band Downstream module.

Description:
    This function is responsible for getting total number of channels
    supported by BADS module, since BADS device is implemented as a
    device channel.

Returns:
    TODO:

See Also:
    BADS_31xx_OpenChannel(), BADS_31xx_ChannelDefaultSettings()

****************************************************************************/
BERR_Code BADS_31xx_GetTotalChannels(
    BADS_Handle hDev,                   /* [in] Device handle */
    unsigned int *totalChannels         /* [out] Returns total number downstream channels supported */
    );

/***************************************************************************
Summary:
    This function opens Qam In-Band Downstream module channel.

Description:
    This function is responsible for opening BADS module channel. When a
    BADS channel is opened, it will create a module channel handle and
    configure the module based on the channel default settings. Once a
    channel is opened, it must be closed before it can be opened again.

Returns:
    TODO:

See Also:
    BADS_31xx_CloseChannel(), BADS_31xx_GetChannelDefaultSettings()

****************************************************************************/
BERR_Code BADS_31xx_OpenChannel(
    BADS_Handle hDev,                   /* [in] Device handle */
    BADS_ChannelHandle *phChn,          /* [out] Returns channel handle */
    unsigned int channelNo,             /* [in] Channel number to open */
    const struct BADS_ChannelSettings *pChnDefSettings /* [in] Channel default setting */
    );

/***************************************************************************
Summary:
    This function closes Qam In-Band Downstream module channel.

Description:
    This function is responsible for closing BADS module channel. Closing
    BADS channel it will free BADS channel handle. It is required that all
    opened BDQS channels must be closed before closing BADS.

Returns:
    TODO:

See Also:
    BADS_31xx_OpenChannel(), BADS_31xx_CloseChannel()

****************************************************************************/
BERR_Code BADS_31xx_CloseChannel(
    BADS_ChannelHandle hChn             /* [in] Device channel handle */
    );

/***************************************************************************
Summary:
    This function gets Qam In-Band Downstream module device handle based on
    the device channel handle.

Description:
    This function is responsible returning BADS module handle based on the
    BADS module channel.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BADS_31xx_GetDevice(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_Handle *pQds                   /* [out] Returns Device handle */
    );


/***************************************************************************
Summary:
    This function gets default setting for a Qam In-Band Downstream module channel.

Description:
    This function is responsible for returning the default setting for
    channel of BADS. The return default setting is used when opening
    a channel.

Returns:
    TODO:

See Also:
    BADS_31xx_OpenChannel()

****************************************************************************/
BERR_Code BADS_31xx_GetChannelDefaultSettings(
    BADS_Handle hDev,                   /* [in] Device handle */
    unsigned int channelNo,             /* [in] Channel number to default setting for */
    BADS_ChannelSettings *pChnDefSettings /* [out] Returns channel default setting */
    );

/***************************************************************************
Summary:
    This function gets the status synchronously of Qam In-Band Downstream module channel.

Description:
    This function is responsible for getting the complete status synchronously for
    a Qam In-Band Downstream module channel.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BADS_31xx_GetStatus(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_Status *pStatus                /* [out] Returns status */
    );

/***************************************************************************
Summary:
    This function requests the status asynchronously of Qam In-Band Downstream module channel.

Description:
    This function is responsible for requesting the status to be calculated asynchronously for
    a Qam In-Band Downstream module channel. The Qam frontend is responsible to inform 
    the backend when the status is ready either through an interrupt or by any other predetermined
    method.
    
Returns:
    TODO:

See Also: BADS_GetStatus

****************************************************************************/
BERR_Code BADS_31xx_RequestAsyncStatus(
    BADS_ChannelHandle hChn            /* [in] Device channel handle */
    );

/***************************************************************************
Summary:
    This function gets the status asynchronously of Qam In-Band Downstream module channel.

Description:
    This function is responsible for getting the complete status asynchronously for
    a Qam In-Band Downstream module channel.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BADS_31xx_GetAsyncStatus(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_Status *pStatus                /* [out] Returns status */
    );

/***************************************************************************
Summary:
    This function gets the lock status for a Qam In-Band Downstream
    module channel.

Description:
    This function is responsible for getting the lock status
    for a BADS module channel.

Returns:
    TODO:

See Also:
    BADS_31xx_GetStatus()

****************************************************************************/
BERR_Code BADS_31xx_GetLockStatus(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_LockStatus *pLockStatus         /* [out] Returns lock status */
    );

/***************************************************************************
Summary:
    This function gets the I and Q values for soft decision of a
    Qam In-Band Downstream module channel.

Description:
    This function is responsible for getting the I and Q values for soft
    decision of a BADS module channel.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BADS_31xx_GetSoftDecision(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
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
    Qam In-Band Downstream channel changes lock state.
    A lock state change is defined at switching from Lock-Unlock or Unlock-Lock.
    To determine the current lock state, a call to BADS_GetLockStatus() is
    required. To get more a more detail status, call BADS_GetStatus().

    Note: It is "highly" recommended that the callback function do the minimum
    require to notify the application of this event, such sent a message or
    fire an event.  This callback function may be called from an
    interrupt context.  Please use with caution.

Returns:
    TODO:

See Also:
    BADS_31xx_GetLockStatus(), BADS_31xx_GetStatus()

****************************************************************************/
BERR_Code BADS_31xx_InstallCallback(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_Callback callbackType,         /* [in] type of Callback */
    BADS_CallbackFunc pCallbackFunc,    /* [in] Pointer to completion callback. */
    void *pParam                        /* [in] Pointer to callback user data. */
    );

/***************************************************************************
Summary:
    This function tries to acquire downstream lock for the specific
    Qam In-Band Downstream module channel.

Description:
    This function is responsible for trying to acquire downstream lock of
    the input IF signal. Acquiring downstream lock involves configuring
    the H/W to desire configuration, then running a Qam In-Band Downstream
    acquisition script. If this is the first acquisition for the current
    annex mode, then a Qam In-Band Downstream configuration script will be run
    prior to running acquisition script.
    This function will automatically enable the downstream receiver if
    the receiver was in power-saver mode.

Returns:
    TODO:

See Also:
    BADS_31xx_GetLock(), BADS_31xx_GetStatus(), BADS_31xx_GetSoftDecision()

****************************************************************************/
BERR_Code BADS_31xx_Acquire(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_InbandParam *ibParam           /* [in] Inband Parameters to use */
    );

/***************************************************************************
Summary:
    This function enable the power-saver mode.

Description:
    This function is responsible for enabling the downstream receiver
    power-saver mode.  When the BADS is in the power-saver mode, the
    Qam In-Band Downstream receiver is shutdown.

Returns:
    TODO:

See Also:
    BADS_31xx_Acquire()

****************************************************************************/
BERR_Code BADS_31xx_EnablePowerSaver(
    BADS_ChannelHandle hChn             /* [in] Device channel handle */
    );

/***************************************************************************
Summary:
    This function disables the power-saver mode.

Description:
    This function is responsible for disabling the downstream receiver
    power-saver mode.  When the BADS is in the power-saver mode, the
    Qam In-Band Downstream receiver is shutdown.

Returns:
    TODO:

See Also:
    BADS_31xx_EnablePowerSaver()
    BADS_31xx_Acquire()

****************************************************************************/
BERR_Code BADS_31xx_DisablePowerSaver(
    BADS_ChannelHandle hChn             /* [in] Device channel handle */
    );

/***************************************************************************
Summary:
    This function is responsible for processing a notificiation for the specific
    Qam In-Band Downstream module channel.

Description:
    This function needs to called when notification is received.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BADS_31xx_ProcessNotification(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    unsigned int event                  /* [in] Event code and event data*/
    );

/***************************************************************************
Summary:
    This function opens configures (enables/disables) the 31xx device's 
    RF out for daisy chaining.

Description:
    This function opens configures (enables/disables) the 31xx device's 
    RF out for daisy chaining.
    
Returns:
    TODO:

See Also:   
****************************************************************************/
BERR_Code BADS_31xx_SetDaisyChain(
    BADS_Handle hDev,       /* [in] Device channel handle */
    bool enableDaisyChain   /* [in] Eanble/disable daisy chain. */
    );

/***************************************************************************
Summary:
    This function opens tells if the 31xx device's RF out daisy chaining is enabled/disabled.

Description:
    This function opens tells if the 31xx device's RF out daisy chaining is enabled/disabled.
    
Returns:
    TODO:

See Also:   
****************************************************************************/

BERR_Code BADS_31xx_GetDaisyChain(
    BADS_Handle hDev,           /* [in] Device channel handle */
    bool *isEnableDaisyChain    /* [in] Eanble/disable daisy chain. */
    );

/***************************************************************************
Summary:
    This function resets the 31xx device's FEC bit error and block counters.

Description:
    
Returns:
    TODO:

See Also:   
****************************************************************************/
BERR_Code BADS_31xx_ResetStatus(
    BADS_ChannelHandle hChn     /* [in] Device channel handle */
    );

/***************************************************************************
Summary:
    This function performs an i2c read from the slave attached to the 31xx's i2c bus.

Description:
    
Returns:
    TODO:

See Also:   
****************************************************************************/
BERR_Code BADS_31xx_ReadSlave(
    BADS_ChannelHandle hChn,     /* [in] Device channel handle */
	uint8_t chipAddr,			 /* [in] chip addr of the i2c slave device */
	uint32_t subAddr,			 /* [in] sub addr of the register to read from the slave device */
	uint8_t subAddrLen,			 /* [in] how many bytes is the sub addr? one to four*/
	uint32_t *data,				 /* [out] ptr to the data that we will read from the slave device */
	uint8_t dataLen				 /* [in] how many bytes are we going to read? one to four*/
    );

/***************************************************************************
Summary:
    This function performs an i2c write to the slave attached to the 31xx's i2c bus.

Description:
    
Returns:
    TODO:

See Also:   
****************************************************************************/
BERR_Code BADS_31xx_WriteSlave(
    BADS_ChannelHandle hChn,     /* [in] Device channel handle */
	uint8_t chipAddr,			 /* [in] chip addr of the i2c slave device */
	uint32_t subAddr,			 /* [in] sub addr of the register to read from the slave device */
	uint8_t subAddrLen,			 /* [in] how many bytes is the sub addr? one to four*/
	uint32_t *data,				 /* [in] ptr to the data that we will write to the slave device */
	uint8_t dataLen				 /* [in] how many bytes are we going to write? one to four*/
    );

#ifdef __cplusplus
}
#endif

#endif




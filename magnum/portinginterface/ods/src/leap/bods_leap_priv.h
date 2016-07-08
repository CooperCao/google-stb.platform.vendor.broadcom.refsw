/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#ifndef BODS_Leap_PRIV_H__
#define BODS_Leap_PRIV_H__

#include "bchp.h"
#include "breg_mem.h"
#include "bint.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "berr_ids.h"
#include "bods.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Defines raw HAB test mesg hdr (struct) */
#define HAB_MSG_HDR(OPCODE,N,CORE_TYPE,CORE_ID) \
    { ((uint8_t)(((uint16_t)(OPCODE)) >> 2)), \
    (((uint8_t)(0x03 & (OPCODE)) << 6) | ((uint8_t)((N)>>4))), \
    ((((uint8_t)(((N)& 0x0F) << 4))) | ((uint8_t)(0x0F & (CORE_TYPE)))), \
    ((uint8_t)(CORE_ID)) }

#define BODS_DVBT_CORE_TYPE                 0x4
#define BODS_ISDBT_CORE_TYPE                0x5
#define BODS_DVBT2_CORE_TYPE                0x6
#define BODS_DVBC2_CORE_TYPE                0x7
#define BODS_CORE_ID                        0x0
#define CORE_TYPE_GLOBAL                    0x0
#define BODS_CONFIG_PARAMS_READ             0x9A
#define BODS_CONFIG_PARAMS_WRITE            0x1A
#define BODS_L1PLP_BUF_MAX                  512
#define HAB_MEM_SIZE                        512
#define BODS_L1PLP_PKT_0_PLP_COUNT          124
#define BODS_L1PLP_PKT_1_N_PLP_COUNT        125
#define BODS_SEL_L1PLP_PKT_0_PLP_COUNT      123
#define BODS_SEL_L1PLP_PKT_1_N_PLP_COUNT    124
#define BODS_TS_MODE_SERIAL                 1
#define BODS_TS_MODE_PARALLEL               2
#if (BODS_CHIP == 7563) || (BODS_CHIP == 7364) || (BODS_CHIP == 75525)
#define BODS_THD_CONFIG_PARAMS_BUF1         0x18
#else
#define BODS_THD_CONFIG_PARAMS_BUF1         0x10
#endif
#define BODS_THD_CONFIG_PARAMS_BUF2         0x40
#define BODS_THD_CONFIG_PARAMS_BUF3         0
#define BODS_DVBT2_CONFIG_PARAMS_BUF1       0x18
#define BODS_DVBT2_CONFIG_PARAMS_BUF2       0x40
#define BODS_DVBT2_CONFIG_PARAMS_BUF3       0
#define BODS_DVBC2_CONFIG_PARAMS_BUF1       0x18
#define BODS_DVBC2_CONFIG_PARAMS_BUF2       0x40
#define BODS_DVBC2_CONFIG_PARAMS_BUF3       0

typedef enum BODS_OpCodesDS{
        BODS_eAcquire = 0x10,
        BODS_eAcquireParamsWrite = 0x11,
        BODS_eAcquireParamsRead = 0x91,
        BODS_eAnnexASymbolRateWrite = 0x12,
        BODS_eAnnexASymbolRateRead = 0x92,
        BODS_eScanParamsWrite = 0x13,
        BODS_eScanParamsRead = 0x93,
        BODS_eAcqWordsWrite = 0x14,
        BODS_eAcqWordsRead = 0x94,
        BODS_eResetStatus = 0x15,
        BODS_eRequestAsyncStatus = 0x16,
        BODS_eGetAsyncStatus = 0x96,
        BODS_eL1PreStatus = 0x1C0,
        BODS_eL1PostConfigurableStatus = 0x1D0,
        BODS_eL1PostDynamicStatus = 0x1E0,
        BODS_eL1PlpStatus = 0x142,
        BODS_eGetConstellation = 0xA3,
        BODS_eGetVersion = 0xB9,
        BODS_eGetVersionInfo = 0xBA,
        BODS_ePowerCtrlOn = 0x19,
        BODS_ePowerCtrlOff = 0x18,
        BODS_ePowerCtrlRead = 0x98,
        BODS_eResetSelectiveAsyncStatus = 0x55,
        BODS_eRequestSelectiveAsyncStatus = 0x56,
        BODS_eGetSelectiveAsyncStatusReadyType = 0xD7,
        BODS_eGetSelectiveAsyncStatus = 0xD6
}BODS_OpCodesDS;

static const BODS_AcquireParams defAcquireParams =
{
    BODS_AcquisitionMode_eAuto,
    BODS_TuneAcquire_eTune,
    false,
    BODS_BertHeaderLength_e4Byte,
    BODS_BertPolynomial_e23,
    false,
    BODS_SpectrumMode_eManual,
    BODS_InvertSpectrum_eNormal,
    {
        {
            BODS_DvbtBandwidth_e8MHz,
            BODS_DvbtCciMode_eAuto,
            BODS_DvbtCarrierRange_eWide,
            BODS_DvbtOfdmMode_eAuto,
            BODS_DvbtTransmissionMode_e2K,
            BODS_DvbtGuardInterval_e1_4,
            BODS_DvbtDecodeMode_eHighPriority,
            BODS_DvbtTpsMode_eAuto,
            BODS_DvbtModulation_e64Qam,
            BODS_DvbtCodeRate_e1_2,
            BODS_DvbtCodeRate_e1_2,
            BODS_DvbtHierarchy_e0
        }
    }
};

/***************************************************************************
Summary:
    This function opens ODS module.

Description:
    This function is responsible for opening BODS module. When BODS is
    opened, it will create a module handle and configure the module based
    on the default settings. Once the device is opened, it must be closed
    before it can be opened again.

Returns:
    TODO:

See Also:
    BODS_Leap_Close(), BODS_Leap_OpenChannel(), BODS_Leap_CloseChannel(),
    BODS_Leap_GetDefaultSettings()

****************************************************************************/
BERR_Code BODS_Leap_Open(
    BODS_Handle *pOds,                  /* [out] Returns handle */
    BCHP_Handle hChip,                  /* [in] Chip handle */
    BREG_Handle hRegister,              /* [in] Register handle */
    BINT_Handle hInterrupt,             /* [in] Interrupt handle, Bcm3250 */
    const struct BODS_Settings *pDefSettings    /* [in] Default settings */
    );

/***************************************************************************
Summary:
    This function closes ODS module.

Description:
    This function is responsible for closing BODS module. Closing BODS
    will free main BODS handle. It is required that all opened
    BDQS channels must be closed before calling this function. If this
    is not done, the results will be unpredicable.

Returns:
    TODO:

See Also:
    BODS_Leap_Open(), BODS_Leap_CloseChannel()

****************************************************************************/
BERR_Code BODS_Leap_Close(
    BODS_Handle hDev                    /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function initialize ODS module.

Description:
    This function is responsible for initializing BODS module. The initialize
    function may do none or one or more of the following:
    - Download FW into ODS core
    - Startup ODS core
    - Retrieve information from ODS core regarding ODS static configuration
    - etc

Returns:
    TODO:

See Also:
    BODS_Leap_Open(), BODS_Leap_Close()

****************************************************************************/
BERR_Code BODS_Leap_Init(
    BODS_Handle hDev                    /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function returns the version information.

Description:
    This function is responsible for returning the version information.

Returns:
    TODO:

See Also:
    BODS_Leap_Open()

****************************************************************************/
BERR_Code BODS_Leap_GetVersion(
    BODS_Handle hDev,                   /* [in] Device handle */
    BODS_Version *pVersion              /* [out] Returns version */
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
    BODS_Leap_Open()

****************************************************************************/
BERR_Code BODS_Leap_GetVersionInfo(
    BODS_Handle hDev,                    /* [in] Device handle */
    BFEC_VersionInfo *pVersionInfo /* [out] Returns version Info */
    );

/***************************************************************************
Summary:
    This function returns the total number of channels supported by
    ODS module.

Description:
    This function is responsible for getting total number of channels
    supported by BODS module, since BODS device is implemented as a
    device channel.

Returns:
    TODO:

See Also:
    BODS_Leap_OpenChannel(), BODS_Leap_ChannelDefaultSettings()

****************************************************************************/
BERR_Code BODS_Leap_GetTotalChannels(
    BODS_Handle hDev,                   /* [in] Device handle */
    BODS_Standard standard,             /* [in] Device handle */
    unsigned int *totalChannels         /* [out] Returns total number downstream channels supported */
    );

/***************************************************************************
Summary:
    Return a list of capabilities for a given ODS module.

Description:
    This function returns a list of capabilities for an ODS module. Because
    many frontends support multiple types of standards, this allows the
    application to distinguish the capabilities of one frontend versus
    another. If a frontend has multiple capabilities, only one of the modes
    may be used at any given time.

Returns:
    BERR_Code

See Also:

***************************************************************************/
BERR_Code BODS_Leap_GetCapabilities(
    BODS_Handle hDev,                           /* [in] Device handle */
    BODS_FrontendCapabilities *pCapabilities    /* [out] Returns frontend capabilities, all the standards supported by the chip */
    );

/***************************************************************************
Summary:
    This function opens ODS module channel.

Description:
    This function is responsible for opening BODS module channel. When a
    BODS channel is opened, it will create a module channel handle and
    configure the module based on the channel default settings. Once a
    channel is opened, it must be closed before it can be opened again.

Returns:
    TODO:

See Also:
    BODS_Leap_CloseChannel(), BODS_Leap_GetChannelDefaultSettings()

****************************************************************************/
BERR_Code BODS_Leap_OpenChannel(
    BODS_Handle hDev,                   /* [in] Device handle */
    BODS_ChannelHandle *phChn,          /* [out] Returns channel handle */
    const struct BODS_ChannelSettings *pChnDefSettings /* [in] Channel default setting */
    );

/***************************************************************************
Summary:
    This function closes ODS module channel.

Description:
    This function is responsible for closing BODS module channel. Closing
    BODS channel it will free BODS channel handle. It is required that all
    opened BDQS channels must be closed before closing BODS.

Returns:
    TODO:

See Also:
    BODS_Leap_OpenChannel(), BODS_Leap_CloseChannel()

****************************************************************************/
BERR_Code BODS_Leap_CloseChannel(
    BODS_ChannelHandle hChn             /* [in] Device channel handle */
    );

/***************************************************************************
Summary:
    This function gets ODS module device handle based on
    the device channel handle.

Description:
    This function is responsible returning BODS module handle based on the
    BODS module channel.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BODS_Leap_GetDevice(
    BODS_ChannelHandle hChn,            /* [in] Device channel handle */
    BODS_Handle *pQds                   /* [out] Returns Device handle */
    );


/***************************************************************************
Summary:
    This function gets default setting for a ODS module channel.

Description:
    This function is responsible for returning the default setting for
    channel of BODS. The return default setting is used when opening
    a channel.

Returns:
    TODO:

See Also:
    BODS_Leap_OpenChannel()

****************************************************************************/
BERR_Code BODS_Leap_GetChannelDefaultSettings(
    BODS_Handle hDev,                       /* [in] Device handle */
    BODS_Standard standard,                 /* [in] Channel number to default setting for */
    BODS_ChannelSettings *pChnDefSettings   /* [out] Returns channel default setting */
    );

/***************************************************************************
Summary:
    This function gets the lock status of ODS module channel.

Description:
    This function is responsible for getting the lock status for a ODS module channel.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BODS_Leap_GetLockStatus(
    BODS_ChannelHandle hChn,            /* [in] Device channel handle */
    BODS_LockStatus *pLockStatus         /* [out] Returns lock status */
    );


/***************************************************************************
Summary:
    This function gets the I and Q values for soft decision of a
    ODS module channel.

Description:
    This function is responsible for getting the I and Q values for soft
    decision of a BODS module channel.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BODS_Leap_GetSoftDecision(
    BODS_ChannelHandle hChn,            /* [in] Device channel handle */
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
    ODS channel changes lock state.
    A lock state change is defined at switching from Lock-Unlock or Unlock-Lock.
    To determine the current lock state, a call to BODS_GetAsyncStatus() is
    required.

    Note: It is "highly" recommended that the callback function do the minimum
    require to notify the application of this event, such sent a message or
    fire an event.  This callback function may be called from an
    interrupt context.  Please use with caution.

Returns:
    TODO:

See Also:
    BODS_Leap_GetAsyncStatus()

****************************************************************************/
BERR_Code BODS_Leap_InstallCallback(
    BODS_ChannelHandle hChn,            /* [in] Device channel handle */
    BODS_Callback callbackType,         /* [in] type of Callback */
    BODS_CallbackFunc pCallbackFunc,    /* [in] Pointer to completion callback. */
    void *pParam                        /* [in] Pointer to callback user data. */
    );

/***************************************************************************
Summary:
    This function returns the default settings for Qam In-Band Downstream module.

Description:
    This function is responsible for returns the default setting for
    BODS module. The returning default setting should be when
    opening the device.

Returns:
    TODO:

See Also:
    BODS_Leap_Open()

****************************************************************************/
BERR_Code BODS_Leap_GetDefaultAcquireParams(
    BODS_ChannelHandle hChn ,           /* [in] Device channel handle */
    BODS_AcquireParams *acquireParams   /* [out] default Acquire Parameters */
    );

/***************************************************************************
Summary:
    This function sends the acquire parameters for a specific ODS module.

Description:
    This function sends the acquire parameters for a specific ODS module.
    This sets the acquire parameters for a specific acquire.

Returns:
    TODO:

See Also:
    BODS_GetAsyncStatus(), BODS_GetSoftDecision()

****************************************************************************/
BERR_Code BODS_Leap_SetAcquireParams(
    BODS_ChannelHandle hChn ,           /* [in] Device channel handle */
    const BODS_AcquireParams *acquireParams     /* [in] Acquire Parameters to use */
    );

/***************************************************************************
Summary:
    This function retrieves the acquire parameters set for a specific ODS module.

Description:
    This function gets the acquire parameters for a specific ODS module.

Returns:
    TODO:

See Also:
    BODS_GetAsyncStatus(), BODS_GetSoftDecision()

****************************************************************************/
BERR_Code BODS_Leap_GetAcquireParams(
    BODS_ChannelHandle hChn ,           /* [in] Device channel handle */
    BODS_AcquireParams *acquireParams     /* [out] Acquire Parameters to use */
    );


/***************************************************************************
Summary:
    This function tries to acquire downstream lock for the specific
    ODS module channel.

Description:
    This function is responsible for trying to acquire downstream lock of
    the input IF signal. Acquiring downstream lock involves configuring
    the H/W to desire configuration, then running a ODS
    acquisition script. If this is the first acquisition for the current
    annex mode, then a ODS configuration script will be run
    prior to running acquisition script.
    This function will automatically enable the downstream receiver if
    the receiver was in power-saver mode.

Returns:
    TODO:

See Also:
    BODS_Leap_GetLock(), BODS_Leap_GetStatus(), BODS_Leap_GetSoftDecision()

****************************************************************************/
BERR_Code BODS_Leap_Acquire(
    BODS_ChannelHandle hChn,            /* [in] Device channel handle */
    BODS_AcquireParams *acquireParams   /* [in] Acquire Parameters to use */
    );


/***************************************************************************
Summary:
    This function enable the power-saver mode.

Description:
    This function is responsible for enabling the downstream receiver
    power-saver mode.  When the BODS is in the power-saver mode, the
    ODS receiver is shutdown.

Returns:
    TODO:

See Also:
    BODS_Leap_Acquire()

****************************************************************************/
BERR_Code BODS_Leap_EnablePowerSaver(
    BODS_ChannelHandle hChn,                /* [in] Device channel handle */
    BODS_PowerSaverSettings *pwrSettings    /* [in] Power saver settings */
    );

/***************************************************************************
Summary:
    This function disables the power-saver mode.

Description:
    This function is responsible for disabling the downstream receiver
    power-saver mode.  When the BODS is in the power-saver mode, the
    ODS receiver is shutdown.

Returns:
    TODO:

See Also:
    BODS_Leap_EnablePowerSaver()
    BODS_Leap_Acquire()

****************************************************************************/
BERR_Code BODS_Leap_DisablePowerSaver(
    BODS_ChannelHandle hChn,                /* [in] Device channel handle */
    BODS_PowerSaverSettings *pwrSettings    /* [in] Power saver settings */
    );

/***************************************************************************
Summary:
    This function is responsible for processing a notificiation for the specific
    ODS module channel.

Description:
    This function needs to called when notification is received.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BODS_Leap_ProcessNotification(
    BODS_ChannelHandle hChn,            /* [in] Device channel handle */
    unsigned int event                  /* [in] Event code and event data*/
    );


/***************************************************************************
Summary:
    This function resets the leap device's FEC bit error and block counters.

Description:

Returns:
    TODO:

See Also:
****************************************************************************/
BERR_Code BODS_Leap_ResetStatus(
    BODS_ChannelHandle hChn             /* [in] Device channel handle */
    );

/***************************************************************************
Summary:
    This function performs an i2c read from the slave attached to the leap's i2c bus.

Description:

Returns:
    TODO:

See Also:
****************************************************************************/
BERR_Code BODS_Leap_ReadSlave(
    BODS_ChannelHandle hChn,     /* [in] Device channel handle */
    uint8_t chipAddr,            /* [in] chip addr of the i2c slave device */
    uint32_t subAddr,            /* [in] sub addr of the register to read from the slave device */
    uint8_t subAddrLen,          /* [in] how many bytes is the sub addr? one to four*/
    uint32_t *data,              /* [out] ptr to the data that we will read from the slave device */
    uint8_t dataLen              /* [in] how many bytes are we going to read? one to four*/
    );

/***************************************************************************
Summary:
    This function performs an i2c write to the slave attached to the leap's i2c bus.

Description:

Returns:
    TODO:

See Also:
****************************************************************************/
BERR_Code BODS_Leap_WriteSlave(
    BODS_ChannelHandle hChn,     /* [in] Device channel handle */
    uint8_t chipAddr,            /* [in] chip addr of the i2c slave device */
    uint32_t subAddr,            /* [in] sub addr of the register to read from the slave device */
    uint8_t subAddrLen,          /* [in] how many bytes is the sub addr? one to four*/
    uint32_t *data,              /* [in] ptr to the data that we will write to the slave device */
    uint8_t dataLen              /* [in] how many bytes are we going to write? one to four*/
    );

/*******************************************************************************
Summary:
   This function requests the ODS selective status of the requested type.
Description:

Returns:
   BERR_Code
********************************************************************************/
BERR_Code BODS_Leap_RequestSelectiveAsyncStatus(
    BODS_ChannelHandle hChn,
    BODS_SelectiveAsyncStatusType type
);

/*******************************************************************************
Summary:
   This function returns the ODS selective status ready type of the statuses that
   are ready.
Description:

Returns:
   BERR_Code
********************************************************************************/
BERR_Code BODS_Leap_GetSelectiveAsyncStatusReadyType(
    BODS_ChannelHandle hChn,
    BODS_SelectiveAsyncStatusReadyType *ready
);

/*******************************************************************************
Summary:
   This function gets the ODS Selective status of the requested type.
Description:

Returns:
   BERR_Code
********************************************************************************/
BERR_Code BODS_Leap_GetSelectiveAsyncStatus(
    BODS_ChannelHandle hChn,
    BODS_SelectiveAsyncStatusType type,
    BODS_SelectiveStatus *pStatus   /* [out] */
);

#ifdef __cplusplus
}
#endif

#endif

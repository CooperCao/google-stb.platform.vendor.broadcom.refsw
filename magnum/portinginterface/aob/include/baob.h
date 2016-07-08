/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/

/*= Module Overview *********************************************************
<verbatim>

Overview
The Advanced Qam Out Of Band (AOB) Downstream receiver is responsible for taking an
Out-of-Band RF signal (70-130 MHz), down converting into IF signal. This IF
signal is then digitalized, demodulated and error corrected. This error
corrected data stream can be then routed to Davic MAC.


Design
  The design for BAOB PI API is broken into three parts.

o Part 1 (open/close/configuration):

    These APIs are used for opening and closing BAOB device.
    When a device is opened, the device can be configured.

o Part 2 (acquire/get/set):

    These APIs are used for getting and setting runtime attributes of BAOB.
    Along with get and set APIs, an acquire API is also provided.  The acquire
    API is used to acquire (lock) OutOfBand Downstream signal.

o Part 3 (process):

    This API is used to process notification.

Usage
The usage of BAOB involves the following:

   * Configure/Open of BQOB

      * Configure BAOB device for the target system
      * Open BAOB device

   * Program/Acquire device

      * Try to acquire OutOfBand Downstream signal
      * Check status of acquisition
      * Wait for Lock/Unlock ntotification sent from 325X
         * Process Lock/Unlock Notification


Interrupt Requirements
No interrrupt processing is done by this module.


Sample Code:
//
// NOTE: The following sample code does not do any error checking.
//
// In this example, the target H/W board is configured for an
// Out-of-Band Downstream DVS 178 application, with a Crystal Frequency of 24.00 MHz
//
static BAOB_Handle aobDev;
static BCHP_Handle hChip3128;
static BREG_Handle hReg3128;
static BRPC_Handle hRpc;

static BAOB_CallbackFunc cbLockChangeFunc(
    BAOB_Handle hIbChn,
    void *pParam
    )
{
    bool *lockStatus = (bool *) pParam;

    BAOB_GetLockStatus( hIbChn, lockStatus );

    return;
}

main( void )
{
    BAOB_Settings aobDevSettings;
    bool isLock;

    // Initialize hChip3128, hReg3128, and hRpc . . .

    // Initialize BTNR_3128Ob


    // BAOB initialization for BCM3128
    BAOB_GetDefaultSetting( &aobDevSettings );

    // Configure the Crystal Frequency of H/W board.
    aobDevSettings.xtalFreq = 24000000;
    aobDevSettings.enableFEC = true; // false: CableCARD will do FEC
                                      // true: legacy OOB needs FEC
    aobDevSettings.hRpc = hRpc;
    BAOB_Open( &aobDev, hChip3128, hReg3128, &aobDevSettings );


    // Now startup the RPC Notificiation Background thread/task . . .

    // Install callback for state change event, where last parameter
    // is for general use.
    BAOB_InstallCallback( aobDev, BAOB_Callback_eLockChange, cbLockChangeFunc, (void *) &isLock );

    // Tune Tuner . . .


    // Acquire OutOfBand Downstream Signal, DVS 178 QPSK modulation with a
    // symbol rate of 777 KBaud.
    BAOB_Set_Spectrum( aobDev, BAOB_SpectrumMode_eNoInverted);

    BAOB_Acquire( aobDev, BAOB_ModulationType_eDvs178Qpsk, 777000 );

    while(1)
    {
        // Print the lock status every second
        BKNI_Sleep(1000);
        BAOB_GetLOckStatus(aobDev, &isLock);
        printf("Lock Status: %s\n", (isLock==true): "locked", "not lock");
    }
}

void checkRpcNotificationThread(. . .)
{
    while (!system_terminated)
    {
        uint32_t device_id, event;

        BRPC_CheckNotification(hRpc, &device_id, &event, 0);
        if ((BRPC_GET_NOTIFICATION_EVENT(event))
        {
            switch (device_id)
            {
                case BRPC_DevId_3128_DS0:
                    break;
                case BRPC_DevId_3128_DS1:
                    break;
                case BRPC_DevId_3128_OB0:
                    BAOB_ProcessNotification(aobDev);
                    break;
                default:
                    // device not used
                    break;
            }
        }
        else
        {
            BKNI_Sleep(100);
        }
    }
}

</verbatim>

***************************************************************************/

#ifndef BAOB_H__
#define BAOB_H__

#include "bchp.h"
#include "breg_mem.h"
#include "bint.h"
#include "bkni.h"
#include "berr_ids.h"
#include "bfec.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    Error Codes specific to BAOB

Description:

See Also:

****************************************************************************/
#define BAOB_ERR_NOTAVAIL_MODULATION_TYPE   BERR_MAKE_CODE(BERR_AOB_ID, 0)
#define BAOB_ERR_POWER_DOWN                 BERR_MAKE_CODE(BERR_AOB_ID, 1)

/***************************************************************************
Summary:
    The handles for Qam Out-of-Band Downstream module.

Description:
    An opaque handle for BAOB device.

See Also:
    BAOB_Open()

****************************************************************************/
typedef struct BAOB_P_Handle                *BAOB_Handle;

/***************************************************************************
Summary:
    Enumeration for modulation type

Description:
    This enumeration defines the valid modulation types.

See Also:
    BAOB_Acquire(), BAOB_GetStatus()

****************************************************************************/
typedef enum BAOB_ModulationType
{
    BAOB_ModulationType_eDvs167Qpsk, /*Annex A*/
    BAOB_ModulationType_eDvs178Qpsk, /*Annex B*/
    BAOB_ModulationType_ePod_Dvs167Qpsk,
    BAOB_ModulationType_ePod_Dvs178Qpsk,
    BAOB_ModulationType_eLast
} BAOB_ModulationType;

/***************************************************************************
Summary:
    Enumeration for BERT Source

Description:
    This enumeration defines the valid BERT input Sources

See Also:
    BAOB_Acquire()

****************************************************************************/
typedef enum BAOB_BerInputSrc
{
    BAOB_BerInputSrc_eRcvIChOutput, /* BERT input source is the receiver I-channel output */
    BAOB_BerInputSrc_eRcvQChOutput, /* BERT input source is the receiver Q-channel output */
    BAOB_BerInputSrc_eRcvIQChOutputIntrlv, /* BERT input source is the receiver I-channel and Q-channel output interleaved */
    BAOB_BerInputSrc_eFecOutput /* BERT input source is the FEC output */
} BAOB_BerInputSrc;

/***************************************************************************
Summary:
    Enumeration for QPSK OOB spectrum setting

Description:
    This enumeration defines the valid spectrum setting.

See Also:
    BAOB_Get_Spectrum(), BAOB_Set_Spectrum()

****************************************************************************/
typedef enum BAOB_SpectrumMode
{
    BAOB_SpectrumMode_eAuto,
    BAOB_SpectrumMode_eNoInverted,
    BAOB_SpectrumMode_eInverted
} BAOB_SpectrumMode;

/***************************************************************************
Summary:
    Enumeration for Lock Status

Description:
    This enumeration represents the lock status of an AOB channel.

See Also:

****************************************************************************/
typedef enum BAOB_LockStatus
{
    BAOB_LockStatus_eUnlocked,
    BAOB_LockStatus_eLocked,
    BAOB_LockStatus_eNoSignal,
    BAOB_LockStatus_eLast
} BAOB_LockStatus;

/***************************************************************************
Summary:
    Enumeration for Callback types

Description:
    This enumeration represents the AOB callback types
See Also:
    BAOB_Acquire()

****************************************************************************/
typedef enum BAOB_Callback
{
    BAOB_Callback_eLockChange,          /* Callback to notify application of lock change */
    BAOB_Callback_eAsyncStatusReady,    /* Callback to notify application asynchronous status is ready */
    BAOB_Callback_eLast                 /* More may be required */
} BAOB_Callback;

/***************************************************************************
Summary:
    Enumeration for Nyquist Filter

Description:
    This enumeration represents the Nyquist Filter modes of an AOB channel.

See Also:

****************************************************************************/
typedef enum BAOB_NyquistFilter
{
    BAOB_NyquistFilter_eRaisedCosine_50, /* Raised Cosine Nyquist Filter with 50% Roll Off */
    BAOB_NyquistFilter_eRootRaisedCosine_50,  /* Root Raised Cosine Nyquist Filter with 50% Roll Off */
    BAOB_NyquistFilter_eRootRaisedCosine_30,  /* Root Raised Cosine Nyquist Filter with 30% Roll Off */
    BAOB_NyquistFilter_eLast
} BAOB_NyquistFilter;

/***************************************************************************
Summary:
    Enumeration for OOB output mode

Description:
    This enumeration represents the OOB output modes.

See Also:

****************************************************************************/
typedef enum BAOB_OutputMode
{
    BAOB_OutputMode_eDifferentialDecoder, /* pre-FEC Transport Stream sent to Cable Card */
    BAOB_OutputMode_eFEC, /* post FEC transport sent to the backend chip */
    BAOB_OutputMode_eMax
} BAOB_OutputMode;

/***************************************************************************
Summary:
    Enumeration for BERT Polynomial

Description:
    This enumeration defines the valid BERT polynomials.

See Also:
    BAOB_Acquire()
****************************************************************************/
typedef enum BAOB_BertPolynomial
{
    BAOB_BertPolynomial_e23, /* 23-bit polynomial */
    BAOB_BertPolynomial_e15, /* 15-bit polynomial */
    BAOB_BertPolynomial_eLast
} BAOB_BertPolynomial;

/***************************************************************************
Summary:
    This structure represents the AOB Status for a Qam Out-of-Band Downstream
    module.

Description:
    This structure is returned when BAOB_GetStatus() is called.  This
    structure contains the complete status of Qam Out-of-Band Downstream
    module.

See Also:
    BAOB_GetStatus()

****************************************************************************/
typedef struct BAOB_Status
{
    bool isPowerSaverEnabled;           /* Enable=1, Disable=0 */
    BAOB_ModulationType modType;        /* Modulation Type */
    uint32_t sysXtalFreq;               /* in Hertz, Sys. Xtal freq. */
    bool autoInvertSpectrum;            /* If 1, auto Invert Spectrum is enabled */
    bool autoAcquire;                   /* If 1, autoAcquire is enabled */
    bool isFecLock;                     /* lock=1, unlock=0 */
    bool isQamLock;                     /* lock=1, unlock=0 */
    bool isSpectrumInverted;            /* inverted=1, not inverted=0 */
    bool bertSync;                      /* This bit indicates whether BERT sync is locked. lock=1, unlock=0 */
    BAOB_BerInputSrc berSrc;            /* BERT Input Source */
    uint32_t reacqCount;                /* re-acquisition count*/
    int32_t snrEstimate;                /* in 1/256th of db */
    uint32_t sampleRate;                /* baseband Sample Rate in Hz */
    uint32_t symbolRate;                /* in Baud */
    int32_t carrierFreqOffset;          /* in 1/1000 Hz */
    int32_t carrierPhaseOffset;         /* in 1/1000 Hz */
    int32_t symbolError;                /* symbol error in Hz */
    uint32_t berErrorCount;             /* bit errors accumulated by the internal bit error rate counter */
    uint32_t correctedCount;            /* not self-clearing */
    uint32_t uncorrectedCount;          /* not self-clearing */
    uint32_t atmTotalCount;             /* ATM Cell total count */
    uint32_t atmLossCount;              /* ATM Cell loss count */
    int32_t aobChannelPower;            /* 1/100th of a dBmV */
    int32_t feGain;                     /* in 1/100th of a dB */
    int32_t digitalAgcGain;             /* in 1/100th of a dB */
    int32_t equalizerGain;              /* in 1/100th of a dB */
    int32_t agcIntLevel;                /* in 1/10 percent */
    int32_t agcExtLevel;                /* in 1/10 percent */
    uint32_t acquisitionTime;           /* time in mS for the last acquisition */
    uint32_t totalAcquisitionTime;      /* time in mS for all acquisitions to acquire the signal. This value can be different from acquisitionTime in case of autoAcquire, as the number of acquisitions to acquire can be more than one in case of autoAcquire. */
    uint32_t rfFrequency;               /* RF frequency of the acquired signal */
    BAOB_BertPolynomial bertPolynomial; /* PRBS polynomial used by the BER module. */
} BAOB_Status;

/***************************************************************************
Summary:
    Required default settings structure for QPSK Out-of_Band module.

Description:
    The default setting structure defines the default configuration of
    QPSK Out-of_Band when the device is opened.

See Also:
    BAOB_Open()

****************************************************************************/
#define BAOB_SETTINGS_XTALFREQ                  (24000000)  /* 24.00 MHz */
typedef struct BAOB_Settings
{
    unsigned int devId; /* generic device ID */
    void *hGeneric; /* generic handle can be used for anything */
    uint32_t xtalFreq; /* Crystal Freqency in Hertz */
    bool enableFEC; /* enableFEC is deprecated. Please use BAOB_SetConfigSettings()/BAOB_GetConfigSettings() API to switch between pre FEC and post FEC modes. */
    bool isOpenDrain; /* true = Open Drain mode, false = Normal mode. */
    bool serialData; /* true = transport data is serial, false = transport data is parallel and is output on 3128_GPIO[0:7]*/
    BAOB_NyquistFilter  nyquist; /* specifies Nyquist filter rolloff */
} BAOB_Settings;

/***************************************************************************
Summary:
    This structure represents AOB Qam In-Band Downstream acquisition parameters.

Description:

See Also:
    BAOB_Acquire()

****************************************************************************/
typedef struct BAOB_AcquireParam
{
    BAOB_ModulationType modType;     /* Modulation type */
    uint32_t symbolRate;  /* in Baud */
    bool autoAcquire;
    BAOB_SpectrumMode spectrum;    /*default Spectrum setting*/
    BAOB_BerInputSrc berSrc;      /* BERT Input Source */
    BAOB_BertPolynomial bertPolynomial;     /* PRBS polynomial used by the BER module. */
	unsigned frequency; /*  RF Frequency in units of Hz */
} BAOB_AcquireParam;

/***************************************************************************
Summary:
    AOB Config Settings

Description:
    This structure represents Config Settings supported by AOB PI

See Also:
    BAOB_GetConfigSettings(), BAOB_SetConfigSettings()

****************************************************************************/
typedef struct BAOB_ConfigSettings
{
    BAOB_OutputMode outputMode; /* POD usage should set outputMode to BAOB_OutputMode_eDifferentialDecoder (default mode) */
} BAOB_ConfigSettings;

/***************************************************************************
Summary:
    Callback used for event notification.

Description:
    When this PI wants to notify an application, it will call this callback
    function the callback function is registered.

See Also:
    BAOB_InstallLockStateChangeCallback()

****************************************************************************/
typedef BERR_Code (*BAOB_CallbackFunc)(void *pParam );


/***************************************************************************
Summary:
    This function returns the default settings for QPSK Out-of_Band module.

Description:
    This function is responsible for returns the default setting for
    BAOB module. The returning default setting should be when
    opening the device.

Returns:
    TODO:

See Also:
    BAOB_Open()

****************************************************************************/
BERR_Code BAOB_GetDefaultSettings(
    BAOB_Settings *pDefSettings,        /* [out] Returns default setting */
    BCHP_Handle hChip                   /* [in] Chip handle */
    );



/***************************************************************************
Summary:
    This function opens Qam Out-of-Band Downstream module.

Description:
    This function is responsible for opening BAOB module. When BAOB is
    opened, it will create a module handle and configure the module based
    on the default settings. Once the device is opened, it must be closed
    before it can be opened again.

Returns:
    TODO:

See Also:
    BAOB_Close(), BAOB_GetDefaultSettings()

****************************************************************************/
BERR_Code BAOB_Open(
    BAOB_Handle *pAob,                  /* [out] Returns handle */
    BCHP_Handle hChip,                  /* [in] Chip handle */
    BREG_Handle hRegister,              /* [in] Register handle */
    BINT_Handle hInterrupt,             /* [in] Interrupt handle, Bcm3250 */
    const BAOB_Settings *pDefSettings   /* [in] Default settings */
    );

/***************************************************************************
Summary:
    This function closes Qam Out-of-Band Downstream module.

Description:
    This function is responsible for closing BAOB module. Closing BAOB
    will free main BAOB handle.

Returns:
    TODO:

See Also:
    BAOB_Open()

****************************************************************************/
BERR_Code BAOB_Close(
    BAOB_Handle hAob                    /* [in] Device handle */
    );


/***************************************************************************
Summary:
    This function returns the default acquire parameters for a specific
    AOB module.

Description:
    This function returns the default acquire parameters for a specific
    AOB module.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BAOB_GetDefaultAcquireParams(
    BAOB_AcquireParam *obParams         /* [Out] default Out of band acquire parameters to use */
    );


/***************************************************************************
Summary:
    This function sends the acquire parameters for a specific AOB module.

Description:
    This function sends the acquire parameters for a specific AOB module.
    This sets the acquire parameters for a specific acquire.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BAOB_SetAcquireParams(
    BAOB_Handle hDev,                   /* [in] Device handle */
    const BAOB_AcquireParam *obParams         /* [in] Out of band acquire parameters to use */
    );


/***************************************************************************
Summary:
    This function sends the acquire parameters for a specific AOB module.

Description:
    This function sends the acquire parameters for a specific AOB module.
    This sets the acquire parameters for a specific acquire.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BAOB_GetAcquireParams(
    BAOB_Handle hDev,                   /* [in] Device handle */
    BAOB_AcquireParam *obParams         /* [Out] Out of band acquire parameters to use */
    );


/***************************************************************************
Summary:
    This function tries to acquire downstream lock for the specific
    Qam Out-of-Band Downstream module.

Description:
    This function is responsible for trying to acquire downstream lock of
    the input Out-of-Band signal. Acquiring Out-of-Band downstream lock involves
    configuring the H/W to desire configuration, then running a Qam Out-of-Band
    downstream acquisition script. If this is the first acquisition for the
    current annex mode, then a Qam Out-of-Band downstream configuration script
    will be run prior to running acquisition script.
    This function will automatically enable the downstream receiver if
    the receiver was in power-saver mode.

Returns:
    TODO:

See Also:
    BAOB_GetLock(), BAOB_GetStatus(), BAOB_GetSoftDecision()

****************************************************************************/
BERR_Code BAOB_Acquire(
    BAOB_Handle hDev,                   /* [in] Device handle */
    BAOB_AcquireParam *obParams         /* [in] Out of band acquire parameters to use */
    );

/***************************************************************************
Summary:
    This function gets the status synchronously of Qam Out-of-Band Downstream module.

Description:
    This function is responsible for synchronously getting the complete status for
    a Qam Out-of-Band Downstream module.

Returns:
    TODO:

See Also: BAOB_RequestAsyncStatus(), BAOB_GetAsyncStatus().

****************************************************************************/
BERR_Code BAOB_GetStatus(
    BAOB_Handle hDev,                   /* [in] Device handle */
    BAOB_Status *pStatus                /* [out] Returns status */
    );

/***************************************************************************
Summary:
    This function gets the status asynchronously of Qam Out-of-Band Downstream module.

Description:
     This function is responsible for asynchronously getting the complete status for
    a Qam Out-of-Band Downstream module channel. BAOB_RequestAsyncStatus() is called before to
    notify the frontend to calculate the status and notify the backend asynchronously that
    the status is ready then BAOB_GetAsyncStatus() needs to be called to retrieve the status.

Returns:
    TODO:

See Also: BAOB_RequestAsyncStatus(), BAOB_GetStatus().


****************************************************************************/
BERR_Code BAOB_GetAsyncStatus(
    BAOB_Handle hDev,                   /* [in] Device handle */
    BAOB_Status *pStatus                /* [out] Returns status */
    );

/***************************************************************************
Summary:
    This function requests the status asynchronously of Qam Out-of-Band Downstream module.

Description:
    This function is responsible for requesting the status to be calculated asynchronously for
    a Qam Out-of-Band Downstream module channel. The Qam frontend is responsible to inform
    the backend when the status is ready either through an interrupt or by any other predetermined
    method.

Returns:
    TODO:

See Also: BAOB_GetAsyncStatus
               BAOB_GetStatus

****************************************************************************/
BERR_Code BAOB_RequestAsyncStatus(
    BAOB_Handle hDev                   /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function gets the lock status for a Qam Out-of-Band Downstream
    module.

Description:
    This function is responsible for getting the lock status
    for a BAOB module.

Returns:
    TODO:

See Also:
    BAOB_GetStatus()

****************************************************************************/
BERR_Code BAOB_GetLockStatus(
    BAOB_Handle hDev,                   /* [in] Device handle */
    BAOB_LockStatus *pLockStatus        /* [out] Returns lock status */
    );

/***************************************************************************
Summary:
    This function gets the I and Q values for soft decision of a
    Qam Out-of-Band Downstream module.

Description:
    This function is responsible for getting the I and Q values for soft
    decision of a BAOB module.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BAOB_GetSoftDecision(
    BAOB_Handle hDev,                   /* [in] Device handle */
    int16_t nbrToGet,                   /* [in] Number values to get */
    int16_t *iVal,                      /* [out] Ptr to array to store output I soft decision */
    int16_t *qVal,                      /* [out] Ptr to array to store output Q soft decision */
    int16_t *nbrGotten                  /* [out] Number of values gotten/read */
    );

/***************************************************************************
Summary:
    This function enable the power-saver mode.

Description:
    This function is responsible for enabling the OOB downstream receiver
    power-saver mode.  When the BAOB is in the power-saver mode, the
    Qam/QPSK OOB Downstream receiver is shutdown. BAOB_Acquire() will disable
    power-saver mode automatically. Also BAOB_DisablePowerSaver()
    can be used to disable power-saver mode.

Returns:
    TODO:

See Also:
    BAOB_DisablePowerSaver()
    BAOB_Acquire()

****************************************************************************/
BERR_Code BAOB_EnablePowerSaver(
    BAOB_Handle hDev                    /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function disables the power-saver mode.

Description:
    This function is responsible for disabling the OOB downstream receiver
    power-saver mode.  When the BAOB is in the power-saver mode, the
    Qam/QPSK OOB Downstream receiver is shutdown.

Returns:
    TODO:

See Also:
    BAOB_EnablePowerSaver()
    BAOB_Acquire()

****************************************************************************/
BERR_Code BAOB_DisablePowerSaver(
    BAOB_Handle hDev                    /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function is responsible for processing a notificiation for the specific
    Qam Out-of-Band Downstream module channel.

Description:
    This function needs to called when notification is received.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BAOB_ProcessNotification(
    BAOB_Handle hDev,                       /* [in] Device handle */
    unsigned int event                      /* [in] Event code and event data*/
    );

/***************************************************************************
Summary:
    This function is responsible for installing a callback function.

Description:
    This function installs a callback function.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BAOB_InstallCallback(
    BAOB_Handle hDev,                   /* [in] Device handle */
    BAOB_Callback callbackType,         /* [in] Type of callback */
    BAOB_CallbackFunc pCallback,        /* [in] Function Ptr to callback */
    void *pParam                        /* [in] Generic parameter send on callback */
    );

/***************************************************************************
Summary:
    This function clears the corrBlockCount and ucorrBlockCount counters in
    BAOB_Status.

Description:


Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BAOB_ResetStatus(
    BAOB_Handle hDev                    /* [in] Device handle */
    );

/***************************************************************************
Summary:
    This function gets the current Config Settings..

Description:
    This function gets the current Config Settings.

Returns:
    TODO:

See Also:
    BAOB_SetConfigSettings()

****************************************************************************/
BERR_Code BAOB_GetConfigSettings(
    BAOB_Handle hDev,                   /* [in] Device handle */
    BAOB_ConfigSettings *pConfigSettings /* [out] Config Settings */
    );

/***************************************************************************
Summary:
    This function sets the Config Settings.

Description:
    This function sets the Config Settings.

Returns:
    TODO:

See Also:
    BAOB_GetConfigSettings()

****************************************************************************/
BERR_Code BAOB_SetConfigSettings(
    BAOB_Handle hDev,                   /* [in] Device handle */
    const BAOB_ConfigSettings *pConfigSettings /* [in] Config Settings */
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
    BAOB_Open()

****************************************************************************/
BERR_Code BAOB_GetVersionInfo(
    BAOB_Handle hDev,               /* [in] Device handle */
    BFEC_VersionInfo *pVersionInfo  /* [out] Returns version Info */
    );

#ifdef __cplusplus
}
#endif

#endif

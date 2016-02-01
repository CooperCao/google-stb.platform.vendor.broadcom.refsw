/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
/*================== Module Overview =====================================
<verbatim>
The BTFE (Broadcom Theater Front End) portinginterface (PI) controls the
DFE core on any ASIC based Broadcom chip, such as BCM7543 and future ones. 
The Magnum TFE is built on top of lower layer SCD SW stack, which controls the
DFE core and an integrated acquisition processor.

The magnum/portinginterface/tfe/btfe.h header file defines a standard TFE 
front-end API for all TFE based Broadcom chips. The implementation of 
the API for each chip is in the magnum/portinginterface/tfe/<chip> directory.  

BTFE is low-medium level interface and is HW dependant on Theater Frontend.

The BTFE PI device is accessed by the handle BTFE_Handle.  There is one 
BTFE_Handle per Broadcom chip.

The BTFE_Handle shall be provided in the following API functions to control
the acquisition state machine, get front end status:

    BTFE_Open()
    BTFE_Close()
    BTFE_GetDefaultSettings()
    BTFE_GetTotalChannels()
    BTFE_GetChannelDefaultSettings()
    BTFE_OpenChannel()
    BTFE_CloseChannel()
    BTFE_Initialize()
    BTFE_GetVersion()
    BTFE_Acquire()
    BTFE_GetStatus()
    BTFE_SetConfig()
    BTFE_GetConfig()
    BTFE_GetLockStateChangeEventHandle()

Sample Code
//
// NOTE: This is sample code for a system that contains TFE core.  
//         This code does not do any error checking (for simplicity sake).
//
#include "btfe.h"
#include "btfe_priv.h"

static BTFE_Handle  hTFE;  // handle for TFE core
static BTFE_ChannelHandle *hTFEChan = NULL;
static BCHP_Handle  hCHP;
static BREG_Handle  hReg;

void main(void)
{
    BTFE_Settings      settings;
    BTFE_ModulationFormat format;
    BTFE_ConfigSetTunerIF tunerIF;

    // do initialization
    // (get BCHP_Handle, BREG_Handle, etc)
    ...

    // BTFE initialization
    BTFE_GetDefaultSettings(&settings);
    BTFE_Open(&hTFE, hChip, hReg, &settings);

    // allocate handles for the BTFE channels
    BTFE_GetTotalChannels(hTFE, &numChannels);
    hTFEChan = (BTFE_ChannelHandle *)BKNI_Malloc(numChannels * sizeof(BTFE_ChannelHandle));

    // open each 7543 channel
    for (i = 0; i < numChannels; i++)
    {
        BTFE_GetChannelDefaultSettings(hTFE, i, &chnSettings);
        BTFE_OpenChannel(hTFE, &hTFEChan[i], i, &chnSettings);
    }

    BTFE_Initialize(hTFE, NULL);

    // acquire TFE
    format = BTFE_ModulationFormat_eVSB;
    BTFE_Acquire(hTFEChan[i], format);

    ...

    close_tfe:
     for (i = 0; i < numChannels; i++)
        BTFE_CloseChannel(hTFEChan[i]);
    BTFE_Close(hTFE);
}
</verbatim>
========================================================================*/

#ifndef BTFE_H__
#define BTFE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bchp.h"
#include "bkni.h"
#include "bint.h"
#include "berr_ids.h"
#include "btmr.h"
#include "bmem.h"
#include "bfec.h"

/***************************************************************************
Summary:
    BTFE error codes

Description:

See Also:

****************************************************************************/

#define BTFE_ERR_SCD_NOT_INITIALIZED             BERR_MAKE_CODE(BERR_TFE_ID, 0) /* This is a BERR_Code to indicate that scd in not initialized. */
#define BTFE_ERR_ARG_OUT_OF_RANGE                 BERR_MAKE_CODE(BERR_TFE_ID, 1) /* This is a BERR_Code to indicate that the argument is out of range.  */
#define BTFE_ERR_CHIP_FEATURE_NOT_IMPLEMENTED BERR_MAKE_CODE(BERR_TFE_ID, 2) /* This is a BERR_Code to indicate that the chip feature is not implemented. */
#define BTFE_ERR_CHIP_NOT_AVAILABLE              BERR_MAKE_CODE(BERR_TFE_ID, 3) /* This is a BERR_Code to indicate that the chip is not available. */
#define BTFE_ERR_HANDLE_NOT_OPEN                  BERR_MAKE_CODE(BERR_TFE_ID, 4) /* This is a BERR_Code to indicate that the handle is not open. */
#define BTFE_ERR_CHIP_NOT_OPEN                     BERR_MAKE_CODE(BERR_TFE_ID, 5) /* This is a BERR_Code to indicate that the chip is not open. */
#define BTFE_ERR_CHIP_ERROR                         BERR_MAKE_CODE(BERR_TFE_ID, 6) /* This is a BERR_Code to indicate chip error. */
#define BTFE_ERR_BAD_FIRMWARE                      BERR_MAKE_CODE(BERR_TFE_ID, 7) /* This is a BERR_Code to indicate bad firmware. */
#define BTFE_ERR_COMMAND_IN_PROGRESS             BERR_MAKE_CODE(BERR_TFE_ID, 8) /* This is a BERR_Code to indicate that the command is still in progress. */
#define BTFE_ERR_SCD_ERROR                          BERR_MAKE_CODE(BERR_TFE_ID, 9) /* This is a BERR_Code to indicate scd function returned an error. */
#define BTFE_ERR_INVALID_PARAM_COMBO             BERR_MAKE_CODE(BERR_TFE_ID, 10) /* This is a BERR_Code to indicate parameter set is invalid */
#define BTFE_ERR_POWER_DOWN                   BERR_MAKE_CODE(BERR_TFE_ID, 11) /* This is a BERR_Code to indicate that TFE core is powered down */

typedef BERR_Code (*BTFE_CallbackFunc)(void *pParam );

typedef enum BTFE_Callback
{
    BTFE_Callback_eLockChange,              /* Callback to notify application of lock change */
    BTFE_Callback_eNoSignal,                /* Callback to notify application there is no signal */
    BTFE_Callback_eAsyncStatusReady,        /* Callback to notify application that the status requested is ready */
    BTFE_Callback_eLast
} BTFE_Callback;

/*================================
 * enum's
 *==============================*/


/***************************************************************************
Summary:
  This enumeration lists all TFE supported modulation formats.

Description:
  This enumeration lists all TFE supported modulation formats.
  All analog and digital, ATSC and DVB are listed as well as
  "internal state" related ones, like auto and last.
  Note: Unknown is used with mode auto detection, where it indicates
  failed demodulation detection on a present signal.

See Also:
  BTFE_Acquire(), BTFE_StatusFAT
***************************************************************************/
typedef enum BTFE_ModulationFormat
{
    BTFE_ModulationFormat_eAuto, /* Auto VSB/QAM Mode detection */
    BTFE_ModulationFormat_eVSB,
    BTFE_ModulationFormat_eQAM64,
    BTFE_ModulationFormat_eQAM256,
    BTFE_ModulationFormat_eLast
} BTFE_ModulationFormat;


/***************************************************************************
Summary:
  This enumeration lists two signal polarities.

Description:
  Selection of output signal polarity.
  Used for clock, data, sync, error and valid,..

See Also:
  BTFE_ConfigFATData, BTFE_ConfigFATAGC, BTFE_StatusFAT
***************************************************************************/
typedef enum BTFE_SignalPolarity
{
    BTFE_SignalPolarity_eInvert, /* signal is inverted */
    BTFE_SignalPolarity_eNoInvert, /* signal is normal */
    BTFE_SignalPolarity_eLast
} BTFE_SignalPolarity;


/***************************************************************************
Summary:
  This enumeration lists lock statuses.

Description:
  This enumeration lists lock statuses.

Note:
  NotReady - means that TFE is "in transition" trying to lock - check later.

See Also:
  BTFE_StatusFAT
***************************************************************************/
typedef enum BTFE_LockStatus
{
    BTFE_LockStatus_eUnlocked,
    BTFE_LockStatus_eLocked,
    BTFE_LockStatus_eNosignal,
    BTFE_LockStatus_eLast
} BTFE_LockStatus;


/***************************************************************************
Summary:
     Enumeration for selective status types

Description:
     This enumeration represents the status types for TFE selective status.

See Also:
****************************************************************************/
typedef enum BTFE_SelectiveAsyncStatusType
{
     BTFE_SelectiveAsyncStatusType_eDemod,
     BTFE_SelectiveAsyncStatusType_eAgcIndicator,
     BTFE_SelectiveAsyncStatusType_eLast
} BTFE_SelectiveAsyncStatusType;


/*================================
 * structures
 *==============================*/

/***********************
 * CONFIGURATION types
 ***********************/
/***************************************************************************
Summary:
  This structure defines tuner IF parameters.

Description:
  This structure defines tuner IF parameters.

See Also:
***************************************************************************/
typedef struct BTFE_ConfigSetTunerIF
{
    bool        overrideDefault; /* if TRUE, then center and shift values will apply */
    uint32_t    center; /* tuner IF frequency */
    int32_t     shift;  /* tuner IF frequency shift */
} BTFE_ConfigSetTunerIF;

/******************
 * STATUS types
 ******************/

/***************************************************************************
Summary:
    This structure contains VSB status information.

Description:
    Demod Status for VSB core

See Also:
    BTFE_GetStatus()
***************************************************************************/
typedef struct BTFE_DemodStatus
{
    BTFE_LockStatus                lock;
    BTFE_ModulationFormat          modulationFormat;
    uint32_t                       equalizerSNR; /* SNR in units of 1/100 dB */
    uint32_t                       reacqCount;
    int32_t                        carrierOffset; /* carrier offset in Hz */
    int32_t                        timingOffset; /* Baud offset scaled in units of Hz */
    uint32_t                       rSUncorrectableErrors;
    uint32_t                       rSCorrectableErrors;
    int32_t                        numRSpacketsTotal;
    BTFE_SignalPolarity            spectrumPolarity;
    uint32_t                       ifFrequency; /* IF Frequency the demod was able to Lock to */
    uint32_t                       signalQualityIndex; /* Signal Quality Index */
} BTFE_DemodStatus;


/***************************************************************************
Summary:
    This structure contains AGC status information.

Description:
    The user sets bits in flags corresponding to the AGC status items to be
    queried, then calls BTFE_GetStatus() passing the address to this structure
    and setting status item to BTFE_StatusItem_eAGCIndicator.  When
    BTFE_GetStatus() returns, only the status items specified in flags will be
    valid.

See Also:
  BTFE_GetStatus()
***************************************************************************/
typedef struct BTFE_StatusAGCIndicator
{
    uint32_t  flags; /* bit packed flags BTFE_AGCStatus_* bits */
    uint32_t  sdm1; /* FAT only */
    uint32_t  sdm2; /* FAT only */
    uint32_t  sdmx; /* FAT only */
    int32_t   adcMin; /* FAT only (legacy only) */
    int32_t   adcMax; /* FAT only (legacy only) */
    uint32_t  adcPower; /* analog modes only */
    uint32_t  pdetPower; /* FAT only */
    int32_t   vidPower; /* FAT modes that use UAGC and analog only */
    uint16_t  vdcLevel; /* For calculation of DVGA gain */
} BTFE_StatusAGCIndicator;


/***************************************************************************
Summary:
     This structure represents the status ready types for selective status.

Description:
     This structure is returned when BTFE_GetSelectiveAsyncStatusReadyType()
     is called. This structure returns the available status types.

See Also:

****************************************************************************/
typedef struct BTFE_SelectiveAsyncStatusReadyType
{
    uint32_t     demod:1;  /* demod Status */
    uint32_t     agcIndicator:1;  /* AGC Indicator Status */
}BTFE_SelectiveAsyncStatusReadyType;


/***************************************************************************
Summary:
     This structure represents the TFE Selective Status for an TFE channel.

Description:
     This structure is returned when BTFE_GetSelectiveAsyncStatus() is called.
     This structure contains selective status of an TFE channel.

See Also:
     BTFE_GetSelectiveAsyncStatus()

****************************************************************************/
typedef struct BTFE_SelectiveStatus
{
    BTFE_SelectiveAsyncStatusType    type;    /* Async Status Type returned */
    union
    {
        BTFE_DemodStatus                    demodStatus;
        BTFE_StatusAGCIndicator          agcIndicatorStatus;
    } status;
} BTFE_SelectiveStatus;

/***************************************************************************
Summary:
    This structure contains VSB status information.

Description:
    The Data pointer passed into BTFE_GetStatus() will point to this data
    structure when status item is BTFE_StatusItem_eVSB.

See Also:
    BTFE_GetStatus()
***************************************************************************/
typedef struct BTFE_ScanStatus
{
    uint8_t  operationDone;
    uint8_t  confirmVSB;
} BTFE_ScanStatus;


/******************************************************************************
Summary:
    Handle for the tfe device
Description:
    This is an opaque handle for the BTFE device.
See Also:
    BTFE_Open()
******************************************************************************/
typedef struct BTFE_P_Handle *BTFE_Handle;


/******************************************************************************
Summary:
    Handle for an BTFE device channel
Description:
    This is an opaque handle for the BTFE device channel.
See Also:
    BTFE_OpenChannel()
******************************************************************************/
typedef struct BTFE_P_ChannelHandle *BTFE_ChannelHandle;


/***************************************************************************
Summary:
    Structure for TFE settings

Description:
    This structure contains the settings for the BTFE PI.

See Also:
    each BTFE_ConfigItem
****************************************************************************/
typedef struct BTFE_Settings /* TODO */
{
    void *hGeneric; /* generic handle can be used for anything */
} BTFE_Settings;


/******************************************************************************
Summary:
    Structure containing BTFE device channel settings.
Description:
    This structure contains BTFE device channel settings.
See Also:
    BTFE_OpenChannel(), BTFE_GetChannelDefaultSettings()
******************************************************************************/
typedef struct BTFE_ChannelSettings
{
    unsigned long ifFreq; /* IF Frequency in Hertz */
} BTFE_ChannelSettings;


/******************************************************************************
Summary:
     Structure for acquisition parameters
Description:
     This structure contains parameters used for channel acquisition.
See Also:
    BTFE_SetAcquireParams(), BTFE_GetAcquireParams()
******************************************************************************/
typedef struct BTFE_AcquireParams
{
    BTFE_ModulationFormat modulationFormat; /* Modulation Format */
    bool            spectrumInversion; /* inverted spectrum related to the turner default (true - yes, false - no)
                                            Notes:
                                            most of the Tuners invert spectrum
                                            broadcaster can invert or not the the spectrum */
    bool            spectrumAutoDetect; /* auto spectrum detection */
    uint32_t        ifFrequency;/* IF frequency intended for demod to tune to */
    BTFE_ConfigSetTunerIF tunerIf; /* Only applies to external Tuners */
} BTFE_AcquireParams;

/*================================
 * FUNCTIONS
 *==============================*/

/******************************************************************************
Summary:
  This function opens and initializes the TFE block.

Description:
  This function must be called first to get a BTFE_Handle.  This handle is
  used by all other function calls in the BTFE API.
  This handle eventually be closed by calling BTFE_Close.

Input:
  hChip - The chip handle that application created earlier during chip
            initialization sequence.
            This handle is used to query chip information, chip revision,
            and miscellaneous chip info.

  hReg - The register handle that application created earlier during
            chip initialization sequence.
            This handle is used to access chip registers (TFE registers).

  hInterrupt - The level2 interrupt handle that application created
            earlier chip initialization sequence.
            This handle is use to install level 2 interrupt callback.

  pDefSettings - The default setting for BTFE to be set in.
                      This parameter can be NULL. In this case BTFE's default
                      structure will be used. This default structure could be
                      queried prior to BTFE_Open with BTFE_GetDefaultSettings,
                      modified and passed to BTFE_Open.

Output:
  hTFE - a reference to a TFE handle. Upon successful open this will
             reference to a fresh new TFE handle (context).
             If error occurs in BTFE_Open, then *h shall be NULL.

See Also:
    BTFE_Close, BTFE_GetDefaultSettings.
******************************************************************************/
BERR_Code BTFE_Open(
    BTFE_Handle *hTFE, /* [out] BTFE handle */
    BCHP_Handle hChip, /* [in] chip handle */
    BREG_Handle hReg, /* [in] register handle */
    BINT_Handle hInterrupt, /* [in] interrupt handle */
    const BTFE_Settings *pDefSettings /* [in] default settings */
);


/******************************************************************************
Summary:
    Closes the BTFE API.

Description:
    This function releases all the resources allocated by BTFE API and disables
    DFE interrupts.

******************************************************************************/
BERR_Code BTFE_Close(
    BTFE_Handle hTFE /* [in] BTFE handle */
);


/******************************************************************************
Summary:
    Gets default BTFE settings.
Description:
    This function gets the default BTFE settings.

******************************************************************************/
BERR_Code BTFE_GetDefaultSettings(
    BTFE_Settings *pChnDefSettings /* [out] default channel settings */
);


/******************************************************************************
Summary:
    Gets the total number of logical sub-devices for the BTFE device.
Description:
    This function gets the total number of channels supported by the BTFE
    device, e.g. for BCM4501 devices, totalChannels returns the value 2.

******************************************************************************/
BERR_Code BTFE_GetTotalChannels(
    BTFE_Handle  hTFE,                 /* [in] BTFE handle */
    uint32_t      *totalChannels /* [out] number of channels supported */
);


/******************************************************************************
Summary:
    Gets default channel settings.
Description:
    This function gets the default settings for an BTFE device channel.

******************************************************************************/
BERR_Code BTFE_GetChannelDefaultSettings(
    BTFE_Handle    hTFE,                             /* [in] BTFE handle */
    uint32_t        chnNo,                        /* [in] channel number */
    BTFE_ChannelSettings *pChnDefSettings /* [out] default channel settings */
);


/******************************************************************************
Summary:
    Initializes a BTFE device channel.
Description:
    This function initializes a BTFE device channel and returns a
    BTFE_ChannelHandle.

******************************************************************************/
BERR_Code BTFE_OpenChannel(
    BTFE_Handle                      hTFE,                  /* [in] BTFE handle */
    BTFE_ChannelHandle            *pChannelHandle, /* [out] BTFE channel handle */
    uint32_t                         chnNo,              /* [in] channel number */
    const BTFE_ChannelSettings *pSettings         /* [in] channel settings */
);


/******************************************************************************
Summary:
    Closes the BTFE device channel.
Description:
    This function frees the channel handle and any resources contained in the
    channel handle.

******************************************************************************/
BERR_Code BTFE_CloseChannel(
    BTFE_ChannelHandle hTFEChan  /* [in] BTFE channel handle */
);


/******************************************************************************
Summary:
    Downloads the firmware, runs the integrated microcontroller, and initializes
    DFE memory/registers.
Description:
    This function downloads and runs the firmware, and initializes DFE memory
    and registers.  If pImage is NULL, then the default firmware will be used.

******************************************************************************/
BERR_Code BTFE_Initialize(
    BTFE_Handle hTFE, /* [in] BTFE handle */
    void *pImage /* [in] pointer to AP microcode image */
);


/******************************************************************************
Summary:
    Returns TFE PI version information.
Description:
    This function returns the major version, minor version, firmware CRC, and
    customer version.

******************************************************************************/
BERR_Code BTFE_GetVersion(
     BTFE_Handle hTFE, /* [in]  TFE handle */
     BFEC_VersionInfo *pVersion /* [out]  Returns FW version information */
);

/***************************************************************************
Summary:
     Default Acquire parameters for FAT channel.

Description:
     This function returns the default acquire parameters for a specific
     FAT channel.

See Also:
    BTFE_Acquire(), BTFE_GetAcquireParams(), BTFE_SetAcquireParams()

****************************************************************************/
BERR_Code BTFE_GetDefaultAcquireParams(
    BTFE_ChannelHandle hTFEChan, /* [in] BTFE handle */
    BTFE_AcquireParams *acquireParams     /* [out] Acquire Parameters */
    );

/***************************************************************************
Summary:
     Get Acquire Parameters

Description:
     This function retrieves the acquire parameters set for a specific FAT channel.

See Also:
     BTFE_Acquire()
****************************************************************************/
BERR_Code BTFE_GetAcquireParams(
    BTFE_ChannelHandle hTFEChan, /* [in] BTFE handle */
    BTFE_AcquireParams *acquireParams     /* [out] Acquire Parameters */
     );

/***************************************************************************
Summary:
     Set Acquire Parameters

Description:
     This function sends the acquire parameters for a specific FAT channel.

See Also:
     BTFE_Acquire()

****************************************************************************/
BERR_Code BTFE_SetAcquireParams(
    BTFE_ChannelHandle hTFEChan, /* [in] BTFE handle */
    const BTFE_AcquireParams *acquireParams  /* [in] Acquire Parameters */
     );

/******************************************************************************
Summary:
    Start FAT channel acquisition.
Description:
    Tells the DFE core to start channel acquisition.

******************************************************************************/
BERR_Code BTFE_Acquire(
    BTFE_ChannelHandle hTFEChan, /* [in] BTFE handle */
    BTFE_AcquireParams *acquireParams     /* [out] Acquire Parameters */
);

/******************************************************************************
Summary:
    Returns the Lock State Change event handle.
Description:
    If the application wants to know when the channel goes in lock or out of
    lock, it should use this function to get a handle to the Lock State Change
    event.  This event is set whenever the channel lock status changes.

******************************************************************************/
BERR_Code BTFE_GetLockStateChangeEventHandle( /* TODO: still used by 7543 */
    BTFE_ChannelHandle hTFEChan, /* [in] BTFE Handle */
    BKNI_EventHandle *hEvent /* [out] lock event handle */
);


/******************************************************************************
Summary:
    Returns the channel scan event handle.
Description:
    This function returns the handle to the scan event.

******************************************************************************/
BERR_Code BTFE_GetScanEventHandle(  /* TODO: still used by 7543 */
    BTFE_ChannelHandle hTFEChan, /* [in] BTFE Handle */
    BKNI_EventHandle *hEvent /* [out] scan event handle */
);


/***************************************************************************
Summary:
     This function is responsible for installing a callback function.

Description:
     This function installs a callback function.

See Also:

****************************************************************************/
BERR_Code BTFE_InstallCallback(
     BTFE_ChannelHandle hTFEChan,       /* [in] Device channel handle */
     BTFE_Callback callbackType,        /* [in] Type of callback */
     BTFE_CallbackFunc pCallbackFunc,   /* [in] Function Ptr to callback */
     void *pParam                       /* [in] Generic parameter send on callback */
     );


/***************************************************************************
Summary:
     This function resets the 3462 device's FEC bit error and block counters.

Description:
     This function clears the following counters in BTFE Status:
     reacqCount, rsCorrectedBlocks, rsUncorrectedBlocks, rsCleanBlocks,
     rsTotalBlocks, viterbiUncorrectedBits, viterbiTotalbits, viterbiBer and
     preViterbiBer.

See Also:
     BTFE_RequestSelectiveAsyncStatus(), BTFE_GetSelectiveAsyncStatusReadyType(),
     BTFE_GetSelectiveAsyncStatus()

****************************************************************************/
BERR_Code BTFE_ResetStatus(
     BTFE_ChannelHandle hTFEChan                 /* [in] Device channel handle */
     );

/*******************************************************************************
Summary:
     Request TFE selective status.

Description:
     This function requests the TFE selective status of the requested type.

See Also:
     BTFE_ResetStatus(), BTFE_GetSelectiveAsyncStatusReadyType(),
     BTFE_GetSelectiveAsyncStatus()


********************************************************************************/
BERR_Code BTFE_RequestSelectiveAsyncStatus(
     BTFE_ChannelHandle hTFEChan,                      /* [in] Device channel handle */
     BTFE_SelectiveAsyncStatusType type         /* [in] Device channel handle */
     );

/*******************************************************************************
Summary:
     Get Selective Status ready type.

Description:
     This function returns the TFE selective status ready type of the statuses that
     are ready.

See Also:
     BTFE_ResetStatus(), BTFE_RequestSelectiveAsyncStatus(),
     BTFE_GetSelectiveAsyncStatus()

********************************************************************************/
BERR_Code BTFE_GetSelectiveAsyncStatusReadyType(
     BTFE_ChannelHandle hTFEChan,                          /* [in] Device channel handle */
     BTFE_SelectiveAsyncStatusReadyType *ready    /* [in] Device channel handle */
     );

/*******************************************************************************
Summary:
     Get TFE Selective Status of the requested type.

Description:
     This function gets the TFE Selective status of the requested type.

See Also:
     BTFE_ResetStatus(), BTFE_RequestSelectiveAsyncStatus(),
     BTFE_GetSelectiveAsyncStatusReadyType()

********************************************************************************/
BERR_Code BTFE_GetSelectiveAsyncStatus(
     BTFE_ChannelHandle hTFEChan,
     BTFE_SelectiveAsyncStatusType type,             /* [in] Device channel handle */
     BTFE_SelectiveStatus *pStatus                     /* [out] */
     );

/***************************************************************************
Summary:
     This function gets the scan status synchronously of a VSB In-Band
     Downstream module channel.

Description:
     This function is responsible for synchronously getting the scan status
     for a VSB In-Band Downstream module channel. Please note that this API
     should only be called after calling tune and acquire. Scan Status is an
     acquisition status, it gives the status at the last acquisition and not
     the current status. So, it is only valid if a tune and acquire was
     issued prior to calling this API.

See Also:

****************************************************************************/
BERR_Code BTFE_GetScanStatus(
     BTFE_ChannelHandle hTFEChan,                /* [in] Device channel handle */
     BTFE_ScanStatus *pScanStatus                     /* [out] Returns status */
     );


/***************************************************************************
Summary:
     This function gets the I and Q values for soft decision of a
     VSB In-Band Downstream module channel.

Description:
     This function is responsible for getting the I and Q values for soft
     decision of a BTFE module channel.

See Also:

****************************************************************************/
BERR_Code BTFE_GetSoftDecision(
     BTFE_ChannelHandle hTFEChan,                /* [in] Device channel handle */
     int16_t nbrToGet,                         /* [in] Number values to get */
     int16_t *iVal,                             /* [out] Ptr to array to store output I soft decision */
     int16_t *qVal,                             /* [out] Ptr to array to store output Q soft decision */
     int16_t *nbrGotten                        /* [out] Number of values gotten/read */
     );

/***************************************************************************
Summary:
     This function enable the power-saver mode.

Description:
     This function is responsible for enabling the downstream receiver
     power-saver mode.  When the BTFE is in the power-saver mode, the
     Qam In-Band Downstream receiver is shutdown. BTFE_Acquire() will
     disables power-saver mode automatically. Also BTFE_DisablePowerSaver()
     can be used to disable power-saver mode.

See Also:
     BTFE_Acquire()

****************************************************************************/
BERR_Code BTFE_EnablePowerSaver(
     BTFE_ChannelHandle hTFEChan                 /* [in] Device channel handle */
     );


/***************************************************************************
Summary:
     This function disables the power-saver mode.

Description:
     This function is responsible for disabling the downstream receiver
     power-saver mode.  When the BTFE is in the power-saver mode, the
     Qam In-Band Downstream receiver is shutdown.

See Also:
     BTFE_Acquire()

****************************************************************************/
BERR_Code BTFE_DisablePowerSaver(
     BTFE_ChannelHandle hTFEChan                 /* [in] Device channel handle */
     );


#ifdef __cplusplus
}
#endif

#endif /* BTFE_H__ */

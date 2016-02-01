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
#ifndef BADS_7125_PRIV_H__
#define BADS_7125_PRIV_H__

#include "bchp.h"
#include "breg_mem.h"
#include "bint.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "berr_ids.h"
#include "bads.h"
#include "bchp_ds_0.h"
#include "bchp_ds_1.h"

#ifdef __cplusplus
extern "C" {
#endif


#define CHK_RETCODE( rc, func )             \
do {                                        \
    if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
    {                                       \
        goto done;                          \
    }                                       \
} while(0)

#define MX_ADS_CHANNELS         (2)
#define DEV_MAGIC_ID            ((BERR_ADS_ID<<16) | 0xFACE)
#define BADS_7125_P_CHN_REG_OFFSET (BCHP_DS_1_GBL-BCHP_DS_0_GBL)


#define BADS_7125_P_WriteField(h, Register, Field, Data, ChNum) \
  BREG_Write32(h, (BCHP_##Register+(BADS_7125_P_CHN_REG_OFFSET*ChNum)),\
   ((BREG_Read32(h, (BCHP_##Register+(BADS_7125_P_CHN_REG_OFFSET*ChNum))) & \
  ~((uint32_t)BCHP_MASK(Register, Field))) | \
  BCHP_FIELD_DATA(Register, Field, Data)))

#define BADS_7125_P_Write32(h, Register, Data, ChNum) \
  BREG_Write32(h, (Register+(BADS_7125_P_CHN_REG_OFFSET*ChNum)),Data)

#define BADS_7125_P_ReadField(h, Register, Field, ChNum) \
  ((((BREG_Read32(h, (BCHP_##Register+(BADS_7125_P_CHN_REG_OFFSET*ChNum)))) & \
  BCHP_MASK(Register,Field)) >> BCHP_SHIFT(Register,Field)))

#define BADS_7125_P_Read32(h, Register, ChNum) \
  (BREG_Read32(h, (Register+(BADS_7125_P_CHN_REG_OFFSET*ChNum))))

typedef struct BADS_P_7125_Handle               *BADS_7125_Handle;
typedef struct BADS_P_7125_ChannelHandle        *BADS_7125_ChannelHandle;

typedef enum BADS_AcquireStartMode
{
  BADS_AcquireStartMode_Acquire = 1,
  BADS_AcquireStartMode_ResetStatus = 2
} BADS_AcquireStartMode_t;




/*******************************************************************************/
/*******************************************************************************/
/************BEGINNING OF THE DEFINITIONS FOR THE ADS FUNCTIONS*****************/
/*******************************************************************************/
/*******************************************************************************/


/*************************************************************************
 * Summary:
 *   Structure for chip-specific Acquire Parameters
 *
 * Description:
 *   This is the chip-specific component of the ADS Acquire command. It maps
 *   byte for byte to the acquire parameter inside the chip's firmware
 *
 * See Also:
 *   None.
 *
 ***************************************************************************/

/*enumerated type to monitor the lock state of the Qam Receiver*/
typedef enum BADS_QAMStatus_s
{
    BADS_QAMStatus_eNotLocked = 0,
    BADS_QAMStatus_eLocked = 1
}BADS_QAMStatus_t;

/*enumerated type to tell if the Timing Loop is Locked*/
typedef enum BADS_TimingLock_s
{
    BADS_Timing_eNotChecked = 0,
    BADS_Timing_eLocked = 1,
    BADS_Timing_eUnLocked = 2,
    BADS_Timing_eLockedAlternateQam = 3
}BADS_TimingLock_t;

/*enumerated type to monitor the lock state of the FEC*/
typedef enum BADS_FECStatus_s
{
    BADS_FECStatus_eNotLocked = 0,
    BADS_FECStatus_eLocked = 1
}BADS_FECStatus_t;

/*enumerated type to tell if the FEC is spectrally inverted*/
typedef enum BADS_FECSpectrum_s
{
    BADS_FECSpectrum_eNotInverted = 0,
    BADS_FECSpectrum_eInverted = 1,
    BADS_FECSpectrum_eNotInvertedAutoInvert = 2,
    BADS_FECSpectrum_eInvertedAutoInvert = 3
}BADS_FECSpectrum_t;

/*enumerated type for features that have an Off or On state*/
/*DO NOT CHANGE THE ORDER OF THIS ENUMERATED TYPE*/
typedef enum BADS_OffOn_s
{
    BADS_OffOn_eOff = 0,
    BADS_OffOn_eOn = 1
}BADS_OffOn_t;

/*enumerated type for acquisition state */
/*DO NOT CHANGE THE ORDER OF THIS ENUMERATED TYPE*/
typedef enum BADS_AcquireState_s
{
    BADS_Acq_Inactive = 0,
    BADS_Acq_HasAcquired = 1,
    BADS_Acq_Acquiring = 2,
    BADS_Acq_ReAcquiring = 3
}BADS_AcquireState_t;

/*enumerated type for the carrier sweep direction*/
typedef enum BADS_SweepDirection_s
{
    BADS_SweepDirection_eNeg2Pos = 0,
    BADS_SweepDirection_ePos2Neg = 1
}BADS_SweepDirection_t;

/*enumerated type for the Fast Acquire mode used by the ADS functions*/
/*DO NOT CHANGE THE ORDER OF THIS ENUMERATED TYPE*/
typedef enum BADS_FastAcquire_s
{
    BADS_eSlowAcquire = 0,
    BADS_eFastAcquire = 1
}BADS_FastAcquire_t;


/*enumerated type for the FEC mode used by the ADS functions*/
/*DO NOT CHANGE THE ORDER OF THIS ENUMERATED TYPE*/
/*This enumerated type is also used as an index into an array*/
typedef enum BADS_Annex_s
{
    BADS_Annex_eAnnexA = 0,
    BADS_Annex_eAnnexB = 1,
    BADS_Annex_eAnnexC = 2

}BADS_Annex_t;

/*enumerated type for the QAM mode used by the ADS functions*/
/*DO NOT CHANGE THE ORDER OF THIS ENUMERATED TYPE*/
/*This enumerated type is also used as an index into an array*/
typedef enum BADS_QAM_s
{
    BADS_QAM_eQam16 = 0,
    BADS_QAM_eQam32 = 1,
    BADS_QAM_eQam64 = 2,
    BADS_QAM_eQam128 = 3,
    BADS_QAM_eQam256 = 4,
    BADS_QAM_eQam512 = 5,
    BADS_QAM_eQam1024 = 6,
    BADS_QAM_eQam2048 = 7,
    BADS_QAM_eQam4096 = 8
}BADS_QAM_t;

typedef enum BADS_NexusStatusMode
{
  BADS_NexusStatusMode_EnableStatusForNexus = 1
} BADS_NexusStatusMode_t;

/*This is the main structure by the ADS acquisition functions*/
/*DO NOT CHANGE THE ORDER OF THIS STRUCTURE OR IT WILL MESS UP BBS*/
/*add any new entries at the end*/
typedef struct BADS_7125_P_AcquireParams_s
{
    uint32_t StatusStartAddress;                        /*BEGIN: words are used by the PI and should not be changed*/
    volatile BADS_AcquireStartMode_t AcquireStartMode;
    BADS_ModulationType ModulationType;
    BADS_InvertSpectrum InvertSpectrum;
    BADS_SpectrumMode   spectrum;
    uint32_t            IfFrequency;
    BADS_DpmMode        EnableDpm;
    uint32_t            SymbolRate;
    uint32_t            AutoAcquireMode;
    BADS_FastAcquire_t  fastAcquire;
    uint32_t            NexusStatusMode;                /*END: words are used by the PI and should not be changed*/
    uint32_t    BBS_AcqWord0;                           /*BEGIN: 17 words of memory for BBS and Acquisition parameters*/
    uint32_t    BBS_AcqWord1;
    uint32_t    BBS_AcqWord2;
    uint32_t    BBS_AcqWord3;
    uint32_t    BBS_Rerun_Init;
    uint32_t    BBS_Burst_mode;
    uint32_t    BBS_CWC_Enable;
    uint32_t    BBS_CWC1_Freq;
    uint32_t    BBS_CWC2_Freq;
    uint32_t    BBS_CWC3_Freq;
    uint32_t    BBS_CWC4_Freq;
    uint32_t    BBS_IQIMB_on;
    uint32_t    BBS_CFL_on;
    uint32_t    BBS_sweep_dir;
    uint32_t    BBS_DDAGC_on;
    uint32_t    BBS_IMC_on;
    uint32_t    BBS_IQPHS_on;                           /*End: 17 words of memory for BBS and Acquisition parameters*/
    BADS_OffOn_t        DoneLoadFlag;                   /*The following words are used ONLY by the ADS functions*/
    BADS_OffOn_t        DoneFirstTimeFlag;
    BADS_Annex_t        Annex;
    BADS_QAM_t          QAM;
    BADS_TimingLock_t TimingCheck;
    BADS_FECSpectrum_t FECSpectrum;
    uint32_t            Old_UERC1;
    uint32_t            Old_NBERC1;
    uint32_t            Old_CBERC1;
    uint32_t            BadBlockCount;
    uint32_t            StuckFECCount;
    bool                LockUpdate; /* the last lock status called back unlock: false, lock: true */
    BADS_AcquireState_t AcqState;
}BADS_7125_P_AcquireParams_t;

/***************************************************************************
 * Summary:
 *   Structure for ADS status
 *
 * Description:
 *   This structure contains ADS status that is return by the chip
 *
 * See Also:
 *   None.
 *
 ***************************************************************************/

/*This is the main structure by the ADS to allow status monitoring*/
/*DO NOT CHANGE THE ORDER OF THIS STRUCTURE OR IT WILL MESS UP BBS*/
/*add any new entries at the end*/
typedef struct BADS_7125_P_Status_s
{
    uint32_t    AutoAcquireMode;
    uint32_t    QamMode;
    uint32_t    FecMode;
    uint32_t    QamStatus;
    uint32_t    FecStatus;
    uint32_t    SampleRate;
    uint32_t    SymbolRate;
    uint32_t    AGF;
    uint32_t    AGF2;
    uint32_t    AGF_Level;
    uint32_t    AGF2_Level;
    uint32_t    SNR;
    uint32_t    BerPreRS;
    uint32_t    BerPostRS;
    uint32_t    BERTMode;
    uint32_t    DS_CERC2;
    uint32_t    DS_NBERC2;
    uint32_t    DS_UERC2;
    uint32_t    DS_CBERC2;
    uint32_t    TimingFrequencyError;
    uint32_t    CarrierFrequencyError;
    uint32_t    CarrierPhaseError;
    uint32_t    FEC_Corr_RS_Blocks;
    uint32_t    FEC_UCorr_RS_Blocks;
    uint32_t    FEC_Clean_RS_Blocks;
    uint32_t    ChannelPower;
    uint32_t    ReAcquireCount;
    uint32_t    ReSyncCount;
    uint32_t    pCount;
    uint32_t    TimingStatus;
    uint32_t    SpectrumStatus;
    uint32_t    SNRAVG;
    uint32_t    InterleaverDepth;
    uint32_t    IsSpectrumInverted;
    uint32_t    IfFrequency;
    BADS_ModulationType ModulationType;
    bool        PowerSave;
}BADS_7125_P_Status_t;

/* function prototypes in bads_7125_acquire.c (cable QAM Annes A and Annex B) */
BERR_Code BADS_7125_P_Init(BADS_ChannelHandle);
BERR_Code BADS_7125_P_Acquire(BADS_ChannelHandle);
BERR_Code BADS_7125_P_Get_LockStatus(BADS_ChannelHandle);

/* function prototypes in bads_7125_status.c (cable QAM Annes A and Annex B)*/
BERR_Code BADS_7125_P_Get_Status(BADS_ChannelHandle);

/* function prototypes in bads_7125_utils.c (cable QAM Annes A and Annex B)*/
BERR_Code BADS_7125_P_PowerDn(BADS_ChannelHandle);
BERR_Code BADS_7125_P_PowerUp(BADS_ChannelHandle);

/*Functions Called by the tuner PI for DPM control in utils.c*/
BERR_Code BADS_7125_P_SetDPM_isr(BADS_ChannelHandle, uint8_t);
BERR_Code BADS_7125_P_ResetDPM_isr(BADS_ChannelHandle);

/*******************************************************************************/
/*******************************************************************************/
/*****************END OF THE DEFINITIONS FOR THE ADS FUNCTIONS******************/
/*******************************************************************************/
/*******************************************************************************/



/*******************************************************************************
*
*   Private Module Handles
*
*******************************************************************************/

typedef struct BADS_P_7125_Handle
{
    uint32_t magicId;                   /* Used to check if structure is corrupt */
    BCHP_Handle hChip;
    BREG_Handle hRegister;
    BINT_Handle hInterrupt;
    BADS_Version verInfo;
    unsigned int mxChnNo;
    BADS_ChannelHandle hAdsChn[MX_ADS_CHANNELS];
    BADS_CallbackFunc pCallback[MX_ADS_CHANNELS][BADS_Callback_eLast];
    void *pCallbackParam[MX_ADS_CHANNELS][BADS_Callback_eLast];
    bool isDaisyChain;
    BTMR_TimerHandle hTimer;
    BKNI_EventHandle hInterruptEvent;
} BADS_P_7125_Handle;


typedef struct BADS_P_7125_ChannelHandle
{
    uint32_t magicId;                   /* Used to check if structure is corrupt */
    BADS_Handle hAds;
    unsigned int chnNo;
    BADS_ChannelSettings settings;
    bool isLock;                        /* current lock status */
    BKNI_MutexHandle mutex;             /* mutex to protect lock status*/
    BADS_7125_P_AcquireParams_t  *pAcquireParam;
    BADS_7125_P_Status_t *pStatus;
    BREG_Handle hRegister;
} BADS_P_7125_ChannelHandle;

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
    BADS_7125_Close(), BADS_7125_OpenChannel(), BADS_7125_CloseChannel(),
    BADS_7125_GetDefaultSettings()

****************************************************************************/
BERR_Code BADS_7125_Open(
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
    BADS_7125_Open(), BADS_7125_CloseChannel()

****************************************************************************/
BERR_Code BADS_7125_Close(
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
    BADS_7125_Open(), BADS_7125_Close()

****************************************************************************/
BERR_Code BADS_7125_Init(
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
    BADS_7125_Open()

****************************************************************************/
BERR_Code BADS_7125_GetVersion(
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
    BADS_7125_OpenChannel(), BADS_7125_ChannelDefaultSettings()

****************************************************************************/
BERR_Code BADS_7125_GetTotalChannels(
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
    BADS_7125_CloseChannel(), BADS_7125_GetChannelDefaultSettings()

****************************************************************************/
BERR_Code BADS_7125_OpenChannel(
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
    BADS_7125_OpenChannel(), BADS_7125_CloseChannel()

****************************************************************************/
BERR_Code BADS_7125_CloseChannel(
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
BERR_Code BADS_7125_GetDevice(
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
    BADS_7125_OpenChannel()

****************************************************************************/
BERR_Code BADS_7125_GetChannelDefaultSettings(
    BADS_Handle hDev,                   /* [in] Device handle */
    unsigned int channelNo,             /* [in] Channel number to default setting for */
    BADS_ChannelSettings *pChnDefSettings /* [out] Returns channel default setting */
    );

/***************************************************************************
Summary:
    This function gets the status of Qam In-Band Downstream module channel.

Description:
    This function is responsible for getting the complete status for
    a Qam In-Band Downstream module channel.

Returns:
    TODO:

See Also:

****************************************************************************/
BERR_Code BADS_7125_GetStatus(
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
    BADS_7125_GetStatus()

****************************************************************************/
BERR_Code BADS_7125_GetLockStatus(
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
BERR_Code BADS_7125_GetSoftDecision(
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
    BADS_7125_GetLockStatus(), BADS_7125_GetStatus()

****************************************************************************/
BERR_Code BADS_7125_InstallCallback(
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
    BADS_7125_GetLock(), BADS_7125_GetStatus(), BADS_7125_GetSoftDecision()

****************************************************************************/
BERR_Code BADS_7125_Acquire(
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
    BADS_7125_Acquire()

****************************************************************************/
BERR_Code BADS_7125_EnablePowerSaver(
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
    BADS_7125_Acquire()

****************************************************************************/
BERR_Code BADS_7125_DisablePowerSaver(
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
BERR_Code BADS_7125_ProcessNotification(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    unsigned int event                  /* [in] Event code and event data*/
    );

/***************************************************************************
Summary:
    This function opens configures (enables/disables) the 7125 device's
    RF out for daisy chaining.

Description:
    This function opens configures (enables/disables) the 7125 device's
    RF out for daisy chaining.

Returns:
    TODO:

See Also:
****************************************************************************/
BERR_Code BADS_7125_SetDaisyChain(
    BADS_Handle hDev,       /* [in] Returns handle */
    bool enableDaisyChain   /* [in] Eanble/disable daisy chain. */
    );

/***************************************************************************
Summary:
    This function opens tells if the 7125 device's RF out daisy chaining is enabled/disabled.

Description:
    This function opens tells if the 7125 device's RF out daisy chaining is enabled/disabled.

Returns:
    TODO:

See Also:
****************************************************************************/

BERR_Code BADS_7125_GetDaisyChain(
    BADS_Handle hDev,           /* [in] Returns handle */
    bool *isEnableDaisyChain    /* [in] Eanble/disable daisy chain. */
    );

/***************************************************************************
Summary:
    This function resets the 7125 device's FEC bit error and block counters.

Description:

Returns:
    TODO:

See Also:
****************************************************************************/
BERR_Code BADS_7125_ResetStatus(
    BADS_ChannelHandle hChnDev      /* [in] Returns handle */
    );


/***************************************************************************
Summary:
    Get the Event handle

Description:

Returns:
    TODO:

See Also:
****************************************************************************/

BERR_Code BADS_7125_GetInterruptEventHandle(
   BADS_Handle hDev,            /* [in] BADS handle */
   BKNI_EventHandle *phEvent /* [out] event handle */
);

/***************************************************************************
Summary:
    Process the Interrupt event

Description:

Returns:
    TODO:

See Also:
****************************************************************************/
BERR_Code BADS_7125_ProcessInterruptEvent(
    BADS_Handle hDev           /* [in] Returns handle */
    );




#ifdef __cplusplus
}
#endif

#endif




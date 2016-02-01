/***************************************************************************
*     (c)2004-2014 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: Frontend SAT
*    Generic routines to control any SAT satellite frontend
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef NEXUS_FRONTEND_SAT_H__
#define NEXUS_FRONTEND_SAT_H__

#include "nexus_frontend_module.h"
#include "bhab.h"
#include "bsat.h"
#include "bdsq.h"
#include "bwfe.h"
#if NEXUS_HAS_FSK
#include "bfsk.h"
#endif

#include "nexus_i2c.h"
#include "breg_i2c.h"

typedef struct NEXUS_SatDevice * NEXUS_SatDeviceHandle;
typedef struct NEXUS_SatChannel * NEXUS_SatChannelHandle;

/***************************************************************************
Summary:
    SAT Devices
***************************************************************************/
typedef struct NEXUS_FrontendSatDevices
{
    NEXUS_TunerHandle tuner;    /* Optional tuner -- used on 97335 and 97325 boards */
} NEXUS_FrontendSatDevices;

/***************************************************************************
Summary:
    SAT Channel Settings
***************************************************************************/
typedef struct NEXUS_FrontendSatDeviceSettings
{
    BHAB_Handle habHandle;
    BSAT_Handle satHandle;
    BDSQ_Handle dsqHandle;
    BWFE_Handle wfeHandle;

    unsigned numChannels;
    unsigned numWfe;
} NEXUS_FrontendSatDeviceSettings;

/***************************************************************************
Summary:
    SAT Channel Settings
***************************************************************************/
typedef struct NEXUS_FrontendSatChannelSettings
{
    NEXUS_SatDeviceHandle satDevice;
    BSAT_ChannelHandle satChannel;
    unsigned satChip; /* unused */
    NEXUS_FrontendSatDevices devices;
    unsigned channelIndex;
    unsigned diseqcIndex;
    unsigned boardRev;
    void (*restoreInternalLnaFunction)(NEXUS_FrontendHandle handle, void *pParam);
    void *pRestoreParam;
    void (*closeFunction)(NEXUS_FrontendHandle handle, void *pParam);   /* Called after handle is closed */
    void *pCloseParam;
    void *pDevice;
    BDSQ_ChannelHandle (*getDiseqcChannelHandle)(void *param, int index); /* Required for SAT-based devices, returns channel handle currently associated with this demod. */
    void *getDiseqcChannelHandleParam;
    NEXUS_EventCallbackHandle (*getDiseqcEventHandle)(void *param, int index); /* Populated if ast handle for diseqc purposes is not astChannel */
    NEXUS_TaskCallbackHandle (*getDiseqcRxAppCallback)(void *param, int index); /* Populated if ast handle for diseqc purposes is not astChannel */
    NEXUS_TaskCallbackHandle (*getDiseqcTxAppCallback)(void *param, int index); /* Populated if ast handle for diseqc purposes is not astChannel */
    void (*setDiseqcRxAppCallback)(void *param, int index, NEXUS_TaskCallbackHandle appCallback); /* Populated if sat handle for diseqc purposes is not satChannel */
    void (*setDiseqcTxAppCallback)(void *param, int index, NEXUS_TaskCallbackHandle appCallback); /* Populated if sat handle for diseqc purposes is not satChannel */
    NEXUS_Error (*setVoltage)(void *pSatDevice, NEXUS_FrontendDiseqcVoltage voltage); /* Populated if DSQ_SetVoltage doesn't set voltage. */
    void (*getDefaultDiseqcSettings)(void *pDevice, BDSQ_ChannelSettings *settings); /* Populated if diseqc support for this frontend has default settings */
    NEXUS_FrontendCapabilities capabilities;
    NEXUS_I2cHandle i2cDevice;
    BREG_I2C_Handle i2cRegHandle;
    BWFE_Handle wfeHandle;
    void (*deviceClearSpectrumCallbacks)(void *handle); /* Populated if the device has a single spectrum analyzer callback, so that all related frontends can have their links cleared prior to registering. */
#if NEXUS_HAS_FSK
    BFSK_ChannelHandle (*getFskChannelHandle)(void *param, int index); /* Required for SAT-based devices, returns FSK channel handle currently associated with this demod. */
#endif
} NEXUS_FrontendSatChannelSettings;

/***************************************************************************
Summary:
    Get Default SAT Channel Settings
***************************************************************************/
void NEXUS_Frontend_P_Sat_GetDefaultDeviceSettings(
    NEXUS_FrontendSatDeviceSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
    Get Default SAT Channel Settings
***************************************************************************/
void NEXUS_Frontend_P_Sat_GetDefaultChannelSettings(
    NEXUS_FrontendSatChannelSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
    Create an SAT frontend handle
***************************************************************************/
NEXUS_SatDeviceHandle NEXUS_Frontend_P_Sat_Create_Device(
    const NEXUS_FrontendSatDeviceSettings *pSettings
    );

/***************************************************************************
Summary:
    Create an SAT frontend handle
***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_P_Sat_Create_Channel(
    const NEXUS_FrontendSatChannelSettings *pSettings
    );

/***************************************************************************
Summary:
    Set devices for an SAT channel.
Description:
    May be needed if devices are connected to an on-chip I2C master to
    avoid a chicken and egg problem connecting devices.
***************************************************************************/
NEXUS_Error NEXUS_Frontend_P_Sat_SetDevices(
    NEXUS_FrontendHandle handle,
    const NEXUS_FrontendSatDevices *pDevices
    );

/***************************************************************************
Summary:
    Get the SAT channel handle from a frontend handle
***************************************************************************/
BSAT_ChannelHandle NEXUS_Frontend_P_Sat_GetChannel(
    NEXUS_FrontendHandle handle
    );

/***************************************************************************
Summary:
    Internal structure for peakscan
***************************************************************************/
typedef struct NEXUS_SatellitePeakscanStatus {
    unsigned symRateCount;  /* consecutive scans with same symbol rate */
    uint32_t lastSymRate;   /* symbol rate from last scan */
    uint32_t maxPeakPower;  /* maximum of peakPower from scans matching the peak scan parameters */
    uint32_t maxPeakFreq;   /* tuner frequency corresponding to maxPeakPower */
    uint32_t minPeakPower;  /* minimum of peakPower from scans matching the peak scan parameters (only used for tone search) */
    uint32_t maxPeakIndex;  /* index of maximum peakPower (only used for tone search) */

    struct {
        unsigned numerator;
        unsigned denominator;
    } minRatio;

    /* for-loop book-keeping */
    uint32_t curFreq;
    uint32_t endFreq;
    uint32_t stepFreq;
    bool singleScan;
    int32_t binsize;

    uint32_t minSymbolRate;
    uint32_t maxSymbolRate;

    bool scanFinished;
} NEXUS_SatellitePeakscanStatus;

BDBG_OBJECT_ID_DECLARE(NEXUS_SatDevice);

/* Maximum possible SAT frontend channels for a single chip/device. Can be larger than actual number. */
#define NEXUS_SAT_MAX_CHANNELS 16

/***************************************************************************
Summary:
Internal structure for SAT devices

Like the over-arching frontend code, a NEXUS_SatDevice corresponds to the chip,
while a NEXUS_SatChannel corresponds to the individual demod.

NEXUS_SatDevice corresponds to the BSAT_Handle, and each NEXUS_SatChannel
in the same device should have it's pDevice set to the corresponding NEXUS_SatDevice.
***************************************************************************/
typedef struct NEXUS_SatDevice
{
    BDBG_OBJECT(NEXUS_SatDevice)

    NEXUS_FrontendSatDeviceSettings settings;

    NEXUS_FrontendType type;

    BHAB_Handle habHandle;
    BSAT_Handle satHandle;
    BDSQ_Handle dsqHandle;
    BWFE_Handle wfeHandle;

    BWFE_ChannelInfo wfeInfo;

    unsigned numChannels;
    unsigned numDsq;
    unsigned numWfe;

    bool satActive[NEXUS_SAT_MAX_CHANNELS];
    bool dsqActive[NEXUS_SAT_MAX_CHANNELS];
    bool wfeActive[NEXUS_SAT_MAX_CHANNELS];

    BWFE_ChannelHandle wfeChannels[NEXUS_SAT_MAX_CHANNELS];
    BDSQ_ChannelHandle dsqChannels[NEXUS_SAT_MAX_CHANNELS];
    unsigned wfeMap[NEXUS_SAT_MAX_CHANNELS];

    BKNI_EventHandle wfeReadyEvent[NEXUS_SAT_MAX_CHANNELS];
} NEXUS_SatDevice;

BDBG_OBJECT_ID_DECLARE(NEXUS_SatChannel);

/***************************************************************************
Summary:
Internal structure for SAT channels
Be aware that each NEXUS_SatDevice corresponds to one BSAT_ChannelHandle, not one BSAT_Handle.
There is a one-to-one mapping of satChannel to frontendHandle.
There is a one-to-many mapping of satHandle to satChannel.
***************************************************************************/
typedef struct NEXUS_SatChannel
{
    BDBG_OBJECT(NEXUS_SatChannel)

    NEXUS_FrontendSatChannelSettings settings;

    NEXUS_SatDeviceHandle satDevice;

    BSAT_ChannelHandle satChannel;
    NEXUS_I2cHandle deviceI2cHandle; /* For a possible master I2C controller */
    NEXUS_FrontendHandle frontendHandle;

    NEXUS_FrontendSatelliteSettings lastSettings;   /* Last tune settings, returned in status */
    NEXUS_FrontendDiseqcSettings diseqcSettings;

    BKNI_EventHandle lockEvent;
    NEXUS_EventCallbackHandle lockEventCallback;
    NEXUS_TaskCallbackHandle lockAppCallback;

    BKNI_EventHandle diseqcTxEvent;
    BKNI_EventHandle diseqcRxEvent;
    NEXUS_EventCallbackHandle diseqcTxEventCallback;
    NEXUS_EventCallbackHandle diseqcRxEventCallback;
    NEXUS_TaskCallbackHandle diseqcRxAppCallback;
    NEXUS_TaskCallbackHandle diseqcTxAppCallback;

    /* This group of variables are used to do symbol rate scans and tone searches */
    bool symbolRateScan;
    bool toneSearch;
    BKNI_EventHandle peakscanEvent;
    NEXUS_EventCallbackHandle peakscanEventCallback;
    NEXUS_TaskCallbackHandle peakscanAppCallback;
    NEXUS_SatellitePeakscanStatus peakscanStatus;

    NEXUS_TaskCallbackHandle ftmCallback; /* optionally set by customer extension. must be created here because it's
                                             based on a BSAT_Handle event. */

#ifdef NEXUS_FRONTEND_CUSTOMER_EXTENSION
    void *customerData;
#endif

    NEXUS_FrontendCapabilities capabilities; /* copy of capabilites for use in nexus_frontend_sat.c */
    unsigned satChip;
    unsigned channel; /* the channel number associated with this frontend. */
    unsigned diseqcIndex; /* the diseqc output number associated with this frontend. */
    BDSQ_ChannelHandle (*getDiseqcChannelHandle)(void *param, int index); /* Populated if SAT handle for diseqc purposes is not satChannel */
    void *getDiseqcChannelHandleParam;
    NEXUS_EventCallbackHandle (*getDiseqcEventHandle)(void *param, int index); /* Populated if SAT handle for diseqc purposes is not satChannel */
    NEXUS_TaskCallbackHandle (*getDiseqcRxAppCallback)(void *param, int index); /* Populated if SAT handle for diseqc purposes is not satChannel */
    NEXUS_TaskCallbackHandle (*getDiseqcTxAppCallback)(void *param, int index); /* Populated if SAT handle for diseqc purposes is not satChannel */
    void (*setDiseqcRxAppCallback)(void *param, int index, NEXUS_TaskCallbackHandle appCallback); /* Populated if SAT handle for diseqc purposes is not satChannel */
    void (*setDiseqcTxAppCallback)(void *param, int index, NEXUS_TaskCallbackHandle appCallback); /* Populated if SAT handle for diseqc purposes is not satChannel */
    NEXUS_Error (*setVoltage)(void *pSatDevice, NEXUS_FrontendDiseqcVoltage voltage);

    void (*deviceClearSpectrumCallbacks)(void *handle); /* Populated if the device has a single spectrum analyzer callback, so that all related frontends can have their links cleared prior to registering. */

    unsigned selectedAdc; /* WFE index for this frontend. */

    BKNI_EventHandle spectrumDataReadyEvent;
    NEXUS_EventCallbackHandle spectrumEventCallback;
    NEXUS_EventCallbackHandle spectrumDataReadyEventCallback;
    NEXUS_TaskCallbackHandle spectrumDataReadyAppCallback;
    uint32_t *spectrumDataPointer;
    unsigned spectrumDataLength;
#if NEXUS_HAS_FSK
    BFSK_ChannelHandle (*getFskChannelHandle)(void *param, int index); /* Required for SAT-based devices, returns FSK channel handle currently associated with this demod. */
#endif
    struct {
        bool valid;
        NEXUS_Time lastRead;
        NEXUS_FrontendSatelliteAgcStatus values;
    } cachedAgc;
} NEXUS_SatChannel;

/* only use this your chip's .c file. you must #define B_SAT_CHIP in that .c file (never in a .h file). 
this will verify that the handle is correct before doing a potentially dangerous typecast. */
#define NEXUS_Frontend_P_GetSatDevice(frontendHandle) NEXUS_Frontend_P_GetSatDeviceByChip(frontendHandle, B_SAT_CHIP)

NEXUS_SatDevice *NEXUS_Frontend_P_GetSatDeviceByChip( NEXUS_FrontendHandle handle, unsigned chipId );

/* Private interface to allow events/callbacks to be unregistered and reregistered */
NEXUS_Error NEXUS_Frontend_P_Sat_RegisterEvents(NEXUS_SatChannel *pChannel);
NEXUS_Error NEXUS_Frontend_P_Sat_UnregisterEvents(NEXUS_SatChannel *pChannel);

#endif /* #ifndef NEXUS_FRONTEND_SAT_H__ */


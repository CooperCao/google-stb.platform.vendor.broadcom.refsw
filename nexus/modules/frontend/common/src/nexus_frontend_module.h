/******************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef NEXUS_FRONTEND_MODULE_H__
#define NEXUS_FRONTEND_MODULE_H__

#include "nexus_frontend_thunks.h"
#include "nexus_frontend_init.h"

#include "nexus_base.h"

#include "priv/nexus_i2c_priv.h"
#include "priv/nexus_spi_priv.h"

#include "nexus_frontend.h"
#include "nexus_tuner.h"
#include "nexus_power_management.h"
#include "priv/nexus_core.h"
#include "priv/nexus_core_audio.h"
#include "priv/nexus_core_video.h"
#include "blst_squeue.h"
#include "nexus_frontend_extension_api.h"
#include "priv/nexus_frontend_standby_priv.h"

#include "nexus_frontend_channelbonding.h"
#if NEXUS_HAS_MXT
#include "bmxt.h"
#endif

#ifdef NEXUS_FRONTEND_2940
#include "nexus_frontend_2940.h"
#endif
#ifdef NEXUS_FRONTEND_3510
#include "nexus_frontend_3510.h"
#endif
#ifdef NEXUS_FRONTEND_3520
#include "nexus_frontend_3520.h"
#endif
#ifdef NEXUS_FRONTEND_DTT760X
#include "nexus_tuner_dtt760x.h"
#endif
#ifdef NEXUS_FRONTEND_DTT7045
#include "nexus_tuner_dtt7045.h"
#endif
#ifdef NEXUS_FRONTEND_DTT76800
#include "nexus_tuner_dtt76800.h"
#endif
#ifdef NEXUS_FRONTEND_DTT75409
#include "nexus_tuner_dtt75409.h"
#endif
#ifdef NEXUS_FRONTEND_3418
#include "nexus_tuner_3418.h"
#endif
#ifdef NEXUS_FRONTEND_3420
#include "nexus_tuner_3420.h"
#endif
#ifdef NEXUS_FRONTEND_3440
#include "nexus_tuner_3440.h"
#endif
#ifdef NEXUS_FRONTEND_3412
#include "nexus_amplifier_3412.h"
#endif
#ifdef NEXUS_FRONTEND_3431
#include "nexus_amplifier_3431.h"
#endif
#ifdef NEXUS_FRONTEND_4501
#include "nexus_frontend_4501.h"
#endif
#ifdef NEXUS_FRONTEND_4506
#include "nexus_frontend_4506.h"
#endif
#ifdef NEXUS_FRONTEND_4517
#include "nexus_frontend_4517.h"
#endif
#ifdef NEXUS_FRONTEND_4528
#include "nexus_frontend_4528.h"
#endif
#ifdef NEXUS_FRONTEND_4538
#include "nexus_frontend_4538.h"
#endif
#ifdef NEXUS_FRONTEND_45216
#include "nexus_frontend_45216.h"
#endif
#ifdef NEXUS_FRONTEND_45308
#include "nexus_frontend_45308.h"
#endif
#ifdef NEXUS_FRONTEND_7552
#include "nexus_frontend_7552.h"
#endif
#ifdef NEXUS_FRONTEND_73XX
#include "nexus_frontend_73xx.h"
#endif
#ifdef NEXUS_FRONTEND_7346
#include "nexus_frontend_7346.h"
#endif
#ifdef NEXUS_FRONTEND_7364
#include "nexus_frontend_7364.h"
#endif
#ifdef NEXUS_FRONTEND_7366
#include "nexus_frontend_7366.h"
#endif
#ifdef NEXUS_FRONTEND_3255
#include "nexus_frontend_3255.h"
#if (BCHP_CHIP == 7420) || (BCHP_CHIP == 7405)
#include "nexus_docsis_interface.h"
#endif
#endif
#ifdef NEXUS_FRONTEND_3383
#include "nexus_frontend_3255.h"
#include "nexus_docsis_interface.h"
#endif
#ifdef NEXUS_FRONTEND_DOCSIS
#include "nexus_docsis.h"
#include "nexus_docsis_interface.h"
#include "nexus_docsis_data.h"
#endif
#ifdef NEXUS_FRONTEND_3117
#include "nexus_frontend_31xx.h"
#endif
#ifdef NEXUS_FRONTEND_3114
#include "nexus_frontend_31xx.h"
#endif
#ifdef NEXUS_FRONTEND_3112
#include "nexus_frontend_31xx.h"
#endif
#ifdef NEXUS_FRONTEND_3109
#include "nexus_frontend_31xx.h"
#endif
#ifdef NEXUS_FRONTEND_7584
#include "nexus_frontend_3128.h"
#endif
#ifdef NEXUS_FRONTEND_3128
#include "nexus_frontend_3128.h"
#endif
#ifdef NEXUS_FRONTEND_3158
#include "nexus_frontend_3158.h"
#endif
#ifdef NEXUS_FRONTEND_3123
#include "nexus_frontend_3128.h"
#endif
#ifdef NEXUS_FRONTEND_3124
#include "nexus_frontend_3128.h"
#endif
#ifdef NEXUS_FRONTEND_3461
#include "nexus_frontend_3461.h"
#endif
#ifdef NEXUS_FRONTEND_3472
#include "nexus_frontend_3461.h"
#endif
#ifdef NEXUS_FRONTEND_3462
#include "nexus_frontend_3461.h"
#endif
#ifdef NEXUS_FRONTEND_7563
#include "nexus_frontend_3461.h"
#endif
#ifdef NEXUS_FRONTEND_7125
#include "nexus_frontend_7125.h"
#include "nexus_tuner_7125.h"
#endif
#ifdef NEXUS_FRONTEND_SCAN_SUPPORTED
#include "nexus_frontend_scan.h"
#endif

#ifdef NEXUS_FRONTEND_EXTENSION
/* This is for NEXUS_FrontendHandle inheritence used to integrated board-specific logic
into the frontend module. */
#include "nexus_frontend_extension.h"
#endif

#ifdef NEXUS_FRONTEND_CUSTOMER_EXTENSION
/* This is for customer's to add customer API's into the Frontend module. This is the
normal meaning of the term "extension." */
#include "nexus_frontend_customer_extension.h"

/* this function must exist in the extension to uninit the customerData hook */
extern void NEXUS_Frontend_P_UninitExtension( NEXUS_FrontendHandle handle );
#endif

#include "priv/nexus_frontend_analog_priv.h"
#include "priv/nexus_tuner_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NEXUS_MODULE_SELF
#error Cant be in two modules at the same time
#endif

#define NEXUS_MODULE_NAME frontend
#define NEXUS_MODULE_SELF g_NEXUS_frontendModule

/***************************************************************************
 * Generic Amplifier Handle
 ***************************************************************************/
#if NEXUS_AMPLIFIER_SUPPORT
#include "blna.h"
typedef struct NEXUS_Amplifier
{
    NEXUS_OBJECT(NEXUS_Amplifier);
    BLNA_Handle lnaHandle;
    NEXUS_AmplifierSettings settings;
} NEXUS_Amplifier;
#else
typedef struct NEXUS_Amplifier
{
    NEXUS_OBJECT(NEXUS_Amplifier);
} NEXUS_Amplifier;
#endif
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Amplifier);

/***************************************************************************
 * Frontend Device structures and private api.
 ***************************************************************************/
typedef struct NEXUS_InternalGainSettings
{
    unsigned frequency;                  /* frequency */
    int16_t  loopThroughGain;            /* Internal Gain to Loop Through.  In 1/100th of a dB. */
    int16_t  daisyGain;                  /* Internal Gain to Daisy.  In 1/100th of a dB. */
    unsigned totalVariableGain;          /* Total platform variable gain ahead of the current frontend.  In 1/100th of a dB. */
    bool     isExternalFixedGainBypassed;/* This flag indicates whether the external fixed gain of the parent device is bypassed or not */
} NEXUS_InternalGainSettings;

typedef struct NEXUS_ExternalGainSettings
{
    int16_t bypassableGain; /* External fixed bypassable gain. */
    int16_t totalGain;      /* External gain total. This also includes all the fixed gains before this device. */
} NEXUS_ExternalGainSettings;

typedef struct NEXUS_GainParameters {
    unsigned frequency; /* As the gain is frequency dependent. */
    NEXUS_FrontendDeviceRfInput rfInput;
    bool accumulateTillRootDevice;
}NEXUS_GainParameters;

typedef enum NEXUS_FrontendDeviceApplication
{
  NEXUS_FrontendDeviceApplication_eUnset,
  NEXUS_FrontendDeviceApplication_eCable,
  NEXUS_FrontendDeviceApplication_eTerrestrial,
  NEXUS_FrontendDeviceApplication_eSatellite,
  NEXUS_FrontendDeviceApplication_eLast
} NEXUS_FrontendDeviceApplication;

/* common struct to cache per-frontend device transport settings.
we must cache because demod register access can be slow, and we have to make a lot of them at Tune-time */
#if NEXUS_HAS_MXT
#define NEXUS_MAX_MTSIF_CONFIG BMXT_MAX_NUM_PARSERS
#else
#define NEXUS_MAX_MTSIF_CONFIG 1
#endif
typedef struct NEXUS_FrontendDeviceMtsifConfig {
#if NEXUS_HAS_MXT
    BMXT_Handle mxt;
#endif

    struct { /* cached demod settings */
        /* for data routing */
        bool enabled;
        unsigned inputBandNum;
        unsigned virtualNum;
        unsigned mtsifTxSel;
        NEXUS_FrontendConnectorHandle connector;

        /* other settings propagated from host PB */
        bool errorInputIgnore;
        unsigned timestampMode;
        bool dssMode;
    } demodPbSettings[NEXUS_MAX_MTSIF_CONFIG];

    unsigned numDemodIb, numDemodPb; /* actual number of demod IB/PB, known at runtime */
    NEXUS_TimerHandle timer;
    bool slave; /* slave (TX-only) in a daisy-chain */
} NEXUS_FrontendDeviceMtsifConfig;

typedef struct NEXUS_FrontendDevice {
    NEXUS_OBJECT(NEXUS_FrontendDevice);
    BLST_D_ENTRY(NEXUS_FrontendDevice) link; /* This is used to create link devices using NEXUS_FrontendDevice_Link. */
    BLST_D_ENTRY(NEXUS_FrontendDevice) node; /* This is used to create g_frontendDeviceList using g_frontendList in NEXUS_FrontendModule_Standby_priv() */
    BLST_D_HEAD(deviceChildList, NEXUS_FrontendDevice) deviceChildList;

    unsigned tripwire;

    unsigned familyId; /* Chip's family Id. In hex, e.g. 0x3128. */
    void *pDevice;     /* Chip specific device handle. */
    struct NEXUS_FrontendDevice *parent;
    NEXUS_FrontendDeviceLinkSettings linkSettings;
    int16_t bypassableFixedGain;                 /* External fixed bypassable gain. */
    int16_t totalFixedBoardGain;                 /* Total external fixed gain. This also includes all the fixed gains(bypassable and not bypassable) before this device. */
    NEXUS_FrontendDeviceApplication application; /* Receiver mode: Terrestrial = NEXUS_TunerApplication_eTerrestrial,Cable = NEXUS_TunerApplication_eCable.
                                            If set to NEXUS_TunerApplication_eUnset, and if this device uses internal lna, then a warning message will be printed out.
                                                                       This supercedes NEXUS_FrontendQamSettings.terrestrial and NEXUS_FrontendOfdmSettings.terrestrial and a mismatch will result in a warning. */
    NEXUS_StandbyMode mode;
    bool openPending; /* Set true if async device open/resume is pending */
    bool openFailed; /* Set true if async device open/resume has failed */
    bool delayedInitializationRequired; /* set true if, after async device open, there is additional initialization requiring the frontend module lock; frontend will then call delayedInit() */
#if NEXUS_AMPLIFIER_SUPPORT
    NEXUS_AmplifierHandle amplifier;
#endif
    bool abortThread;
    /* one frontend device may have up to two mtsif configs. i.e. host only sees one device, but there are two frontend XPT blocks to control.
       the chained config is always HAB/RPC-chained, and only optionally MTSIF-chained */
    NEXUS_FrontendDeviceMtsifConfig mtsifConfig, *chainedConfig;

    NEXUS_Error (*getInternalGain)(void *handle, const NEXUS_GainParameters *params, NEXUS_InternalGainSettings *pSettings);
    NEXUS_Error (*getExternalGain)(void *handle, NEXUS_ExternalGainSettings *pSettings);
    NEXUS_Error (*setExternalGain)(void *handle, const NEXUS_ExternalGainSettings *pSettings);
    NEXUS_Error (*getAmplifierStatus)(void *handle, NEXUS_AmplifierStatus *pStatus);
    NEXUS_Error (*setAmplifierStatus)(void *handle, const NEXUS_AmplifierStatus *pStatus);
    NEXUS_Error (*getStatus)(void *handle, NEXUS_FrontendDeviceStatus *pStatus);
    NEXUS_Error (*standby)(void *handle, const NEXUS_StandbySettings *pSettings);
    NEXUS_Error (*recalibrate)(void *handle, const NEXUS_FrontendDeviceRecalibrateSettings *pSettings);
    void        (*getCapabilities)(void *handle, NEXUS_FrontendDeviceCapabilities *pCapabilities);
    NEXUS_Error (*getSatelliteCapabilities)(void *handle, NEXUS_FrontendSatelliteCapabilities *pCapabilities);
    void        (*getTunerCapabilities)(void *handle, unsigned tunerIndex, NEXUS_FrontendCapabilities *pCapabilities);
#if defined(NEXUS_FRONTEND_DOCSIS) && defined(NEXUS_PLATFORM_DOCSIS_PLUS_BCM3128_IB_SUPPORT)
    NEXUS_Error (*getDocsisLnaDeviceAgcValue)(void *handle,uint32_t *agcValue);
    NEXUS_Error (*setHostChannelLockStatus)(void *handle,unsigned channelNum,bool locked);
#endif
    NEXUS_Error (*delayedInit)(NEXUS_FrontendDeviceHandle handle);
    void        (*close)(void * handle);
    NEXUS_Error (*getSettings)(void *handle, NEXUS_FrontendDeviceSettings *pSettings);
    NEXUS_Error (*setSettings)(void *handle, const NEXUS_FrontendDeviceSettings *pSettings);
    NEXUS_Error (*getDeviceAmplifierStatus)(void *handle, NEXUS_FrontendDeviceAmplifierStatus *pStatus);
    void        (*getWakeupSettings)(NEXUS_FrontendDeviceHandle handle, NEXUS_TransportWakeupSettings *pSettings);
    NEXUS_Error (*setWakeupSettings)(NEXUS_FrontendDeviceHandle handle, const NEXUS_TransportWakeupSettings *pSettings);

    /* Calls which may or may not block on async initialization. if set to true, does not block
     * while initialization is still pending. Not all calls need to block on all frontends.
     */
    struct {
        bool getCapabilities;
        bool getSatelliteCapabilities;
    } nonblocking;
} NEXUS_FrontendDevice;

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_FrontendDevice);

typedef struct NEXUS_FrontendDeviceList{
    BLST_D_HEAD(nexus_device_list, NEXUS_FrontendDevice) deviceList;
}NEXUS_FrontendDeviceList;

#if defined(NEXUS_FRONTEND_DOCSIS) && defined(NEXUS_PLATFORM_DOCSIS_PLUS_BCM3128_IB_SUPPORT)
NEXUS_Error NEXUS_FrontendDevice_P_GetDocsisLnaDeviceAgcValue(NEXUS_FrontendDeviceHandle handle,
                                                              uint32_t *agcValue);
NEXUS_Error NEXUS_FrontendDevice_P_SetHostChannelLockStatus(NEXUS_FrontendDeviceHandle handle,
                                                            unsigned channelNum,bool locked);
#endif
NEXUS_Error NEXUS_FrontendDevice_P_Standby(NEXUS_FrontendDeviceHandle handle, const NEXUS_StandbySettings *pSettings);
NEXUS_Error NEXUS_FrontendDevice_P_GetInternalGain(NEXUS_FrontendDeviceHandle handle, const NEXUS_GainParameters *params, NEXUS_InternalGainSettings *pSettings);
NEXUS_Error NEXUS_FrontendDevice_P_SetExternalGain(NEXUS_FrontendDeviceHandle handle, const NEXUS_ExternalGainSettings *pSettings);
#if NEXUS_AMPLIFIER_SUPPORT
NEXUS_Error NEXUS_FrontendDevice_P_GetAmplifierStatus(NEXUS_FrontendDeviceHandle handle, NEXUS_AmplifierStatus *pStatus);
NEXUS_Error NEXUS_FrontendDevice_P_SetAmplifierStatus(NEXUS_FrontendDeviceHandle handle, const NEXUS_AmplifierStatus *pStatus);
#endif
NEXUS_Error NEXUS_Frontend_P_CheckDeviceOpen(NEXUS_FrontendHandle handle);
NEXUS_Error NEXUS_FrontendDevice_P_CheckOpen(NEXUS_FrontendDeviceHandle handle);

typedef struct NEXUS_FrontendChannelBonding *NEXUS_FrontendChannelBondingHandle;

/***************************************************************************
 * Handle for a generic frontend object
 ***************************************************************************/
typedef struct NEXUS_Frontend
{
    NEXUS_OBJECT(NEXUS_Frontend);
    BLST_SQ_ENTRY(NEXUS_Frontend) node;
    BLST_SQ_ENTRY(NEXUS_Frontend) link; /* list of all frontends maintained by Create/Destroy */
    bool acquired;
    struct NEXUS_Frontend *pParentFrontend;     /* Only set for extended frontends */
    unsigned numExtensions;
    NEXUS_FrontendConnectorHandle connector;
    void *pDeviceHandle;
    NEXUS_FrontendDevice *pGenericDeviceHandle;
    NEXUS_StandbyMode mode;
    struct {
        bool digital;
        bool analog;
    } power;
    struct {
        /* internal MTSIF mapping information, for interaction with MXT */
        unsigned inputBand;
        unsigned txOut;
        unsigned daisyTxOut;
        unsigned daisyOverride; /* per-FrontendHandle override of daisy-chaining, to support cases where a slave device feeds to both the backend and a frontend device.
                                   if true, then this handle is routed directly to the backend. if false, then daisy-chained as usual */
    } mtsif;
    NEXUS_FrontendCapabilities capabilities;
    NEXUS_FrontendUserParameters userParameters;
    NEXUS_FrontendChannelBondingHandle chbond;
    struct NEXUS_Frontend* bondingMaster;
    void        (*close)(NEXUS_FrontendHandle handle);
    NEXUS_Error (*registerExtension)(NEXUS_FrontendHandle parentHandle, NEXUS_FrontendHandle extensionHandle);

    NEXUS_Error (*tuneAnalog)(void *handle, const NEXUS_FrontendAnalogSettings *pSettings);
    NEXUS_Error (*getAnalogStatus)(void *handle, NEXUS_FrontendAnalogStatus *pStatus);
    NEXUS_VideoInput (*getAnalogVideoConnector)(void *handle);
    NEXUS_AudioInputHandle (*getAnalogAudioConnector)(void *handle);
    NEXUS_Error (*tuneOutOfBand)(void *handle, const NEXUS_FrontendOutOfBandSettings *pSettings);
    NEXUS_Error (*getOutOfBandStatus)(void *handle, NEXUS_FrontendOutOfBandStatus *pStatus);
    NEXUS_Error (*tuneQam)(void *handle, const NEXUS_FrontendQamSettings *pSettings);
    NEXUS_Error (*getQamStatus)(void *handle, NEXUS_FrontendQamStatus *pStatus);
    NEXUS_Error (*tuneUpstream)(void *handle, const NEXUS_FrontendUpstreamSettings *pSettings);
    NEXUS_Error (*getUpstreamStatus)(void *handle, NEXUS_FrontendUpstreamStatus *pStatus);
    NEXUS_Error (*tuneVsb)(void *handle, const NEXUS_FrontendVsbSettings *pSettings);
    NEXUS_Error (*getVsbStatus)(void *handle, NEXUS_FrontendVsbStatus *pStatus);
    NEXUS_Error (*requestVsbAsyncStatus)(void *handle);
    NEXUS_Error (*getVsbAsyncStatus)(void *handle, NEXUS_FrontendVsbStatus *pStatus);
    NEXUS_Error (*tuneSatellite)(void *handle, const NEXUS_FrontendSatelliteSettings *pSettings);
    NEXUS_Error (*getSatelliteStatus)(void *handle, NEXUS_FrontendSatelliteStatus *pStatus);
    void        (*getDiseqcSettings)(void *handle, NEXUS_FrontendDiseqcSettings *pSettings);
    NEXUS_Error (*setDiseqcSettings)(void *handle, const NEXUS_FrontendDiseqcSettings *pSettings);
    NEXUS_Error (*getDiseqcStatus)(void *handle, NEXUS_FrontendDiseqcStatus *pStatus);
    NEXUS_Error (*sendDiseqcMessage)(void *handle, const uint8_t *pSendData, size_t sendDataSize, const NEXUS_CallbackDesc *pSendComplete);
    NEXUS_Error (*getDiseqcReply)(void *handle, NEXUS_FrontendDiseqcMessageStatus *pStatus, uint8_t *pReplyBuffer, size_t replyBufferSize, size_t *pReplyLength);
    NEXUS_Error (*sendDiseqcAcw)(void *handle, uint8_t codeWord);
    NEXUS_Error (*resetDiseqc)(void *handle, uint8_t options);
    NEXUS_Error (*tuneOfdm)(void *handle, const NEXUS_FrontendOfdmSettings *pSettings);
    NEXUS_Error (*getOfdmStatus)(void *handle, NEXUS_FrontendOfdmStatus *pStatus);
    NEXUS_Error (*getSoftDecisions)(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length);
    NEXUS_Error (*readSoftDecisions)(void *handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length, size_t *pNumRead);
    void        (*resetStatus)(void *handle);
    #ifdef NEXUS_FRONTEND_SCAN_SUPPORTED
    NEXUS_Error (*scanFrequency)(void *handle, const NEXUS_FrontendScanSettings *pSettings, NEXUS_FrontendScanResults *pResults);
    void        (*getScanThresholds)(void *handle, NEXUS_FrontendScanThresholds *pThresholds);
    NEXUS_Error (*setScanThresholds)(void *handle, const NEXUS_FrontendScanThresholds *pThresholds);
    #endif
    void        (*getAnalogAgcSettings)(void *handle, NEXUS_FrontendAnalogAgcSettings *pSettings);
    NEXUS_Error (*setAnalogAgcSettings)(void *handle, const NEXUS_FrontendAnalogAgcSettings *pSettings);
    void        (*notifyAnalogVideoFormat)(void *handle, NEXUS_VideoFormat format);
    void        (*getAnalogVideoFormat)(void *handle, NEXUS_VideoFormat *pFormat);
    NEXUS_Error (*setAnalogAVFormat)(void *handle, NEXUS_VideoFormat format, NEXUS_IfdAudioMode audioMode, bool carriersOnly);
    void        (*setAnalogAudioInterrupt)(void *handle, NEXUS_FrontendAnalogAudioInterruptFunc function_isr, void *pFuncParam);
    void        (*untune)(void *handle); /* optional */
    void        (*uninstallCallbacks)(void *handle);
    NEXUS_Error (*readSatelliteConfig)(void *handle, unsigned id, void *buffer, unsigned bufferSize);
    NEXUS_Error (*writeSatelliteConfig)(void *handle, unsigned id, const void *buffer, unsigned bufferSize);
    NEXUS_Error (*satellitePeakscan)(void *handle, const NEXUS_FrontendSatellitePeakscanSettings *pSettings);
    NEXUS_Error (*getSatellitePeakscanResult)(void *handle, NEXUS_FrontendSatellitePeakscanResult *pResult);
    NEXUS_Error (*satelliteToneSearch)(void *handle, const NEXUS_FrontendSatelliteToneSearchSettings *pSettings);
    NEXUS_Error (*getSatelliteToneSearchResult)(void *handle, NEXUS_FrontendSatelliteToneSearchResult *pResult);
    NEXUS_Error (*requestQamAsyncStatus)(void *handle);
    NEXUS_Error (*getQamAsyncStatus)(void *handle, NEXUS_FrontendQamStatus *pStatus);
    NEXUS_Error (*requestOutOfBandAsyncStatus)(void *handle);
    NEXUS_Error (*getOutOfBandAsyncStatus)(void *handle, NEXUS_FrontendOutOfBandStatus *pStatus);
    NEXUS_Error (*requestOfdmAsyncStatus)(void *handle);
    NEXUS_Error (*getOfdmAsyncStatus)(void *handle, NEXUS_FrontendOfdmStatus *pStatus);
    NEXUS_Error (*getFastStatus)(void *handle, NEXUS_FrontendFastStatus *pStatus );
    NEXUS_Error (*reapplyTransportSettings)(void *handle);
    NEXUS_Error (*getQamScanStatus)(void *handle, NEXUS_FrontendQamScanStatus *pScanStatus );
    void        (*getType)(void *handle, NEXUS_FrontendType *pType);
    NEXUS_Error (*requestSpectrumData)(void *handle, const NEXUS_FrontendSpectrumSettings *settings );
    NEXUS_Error (*requestDvbtAsyncStatus)(void *handle, NEXUS_FrontendDvbtStatusType type);
    NEXUS_Error (*getDvbtAsyncStatusReady)(void *handle, NEXUS_FrontendDvbtStatusReady *pStatusReady);
    NEXUS_Error (*getDvbtAsyncStatus)(void *handle, NEXUS_FrontendDvbtStatusType type, NEXUS_FrontendDvbtStatus *pStatus);
    NEXUS_Error (*requestIsdbtAsyncStatus)(void *handle, NEXUS_FrontendIsdbtStatusType type);
    NEXUS_Error (*getIsdbtAsyncStatusReady)(void *handle, NEXUS_FrontendIsdbtStatusReady *pStatusReady);
    NEXUS_Error (*getIsdbtAsyncStatus)(void *handle, NEXUS_FrontendIsdbtStatusType type, NEXUS_FrontendIsdbtStatus *pStatus);
    NEXUS_Error (*requestDvbt2AsyncStatus)(void *handle, NEXUS_FrontendDvbt2StatusType type);
    NEXUS_Error (*getDvbt2AsyncStatusReady)(void *handle, NEXUS_FrontendDvbt2StatusReady *pStatusReady);
    NEXUS_Error (*getDvbt2AsyncStatus)(void *handle, NEXUS_FrontendDvbt2StatusType type, NEXUS_FrontendDvbt2Status *pStatus);
    NEXUS_Error (*requestDvbc2AsyncStatus)(void *handle, NEXUS_FrontendDvbc2StatusType type);
    NEXUS_Error (*getDvbc2AsyncStatusReady)(void *handle, NEXUS_FrontendDvbc2StatusReady *pStatusReady);
    NEXUS_Error (*getDvbc2AsyncStatus)(void *handle, NEXUS_FrontendDvbc2StatusType type, NEXUS_FrontendDvbc2Status *pStatus);
    NEXUS_Error (*writeRegister)(void *handle, unsigned address, uint32_t value);
    NEXUS_Error (*readRegister)(void *handle, unsigned address, uint32_t *value);
    NEXUS_Error (*standby)(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings);
    NEXUS_Error (*getTemperature)(void *handle, NEXUS_FrontendTemperature *pTemp);
    NEXUS_Error (*transmitDebugPacket)(void *handle, NEXUS_FrontendDebugPacketType type, const uint8_t *pBuffer, size_t size);
    NEXUS_Error (*getSatelliteSignalDetectStatus)(void *handle, NEXUS_FrontendSatelliteSignalDetectStatus *pStatus);
    NEXUS_Error (*getSatelliteAgcStatus)(void *handle, NEXUS_FrontendSatelliteAgcStatus *pStatus);
    NEXUS_Error (*getBertStatus)(void *handle, NEXUS_FrontendBertStatus *pStatus);
    NEXUS_Error (*getSatelliteRuntimeSettings)(void *handle, NEXUS_FrontendSatelliteRuntimeSettings *pSettings);
    NEXUS_Error (*setSatelliteRuntimeSettings)(void *handle, const NEXUS_FrontendSatelliteRuntimeSettings *pSettings);
    NEXUS_Time resetStatusTime;
    NEXUS_FrontendChipType chip;
} NEXUS_Frontend;

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Frontend);

typedef struct NEXUS_FrontendList{
    BLST_SQ_HEAD(nexus_frontend_list, NEXUS_Frontend) frontends;
} NEXUS_FrontendList;

#if NEXUS_HAS_MXT
NEXUS_Error NEXUS_Frontend_P_InitMtsifConfig(NEXUS_FrontendDeviceMtsifConfig *pConfig, const BMXT_Settings *mxtSettings);
#else
NEXUS_Error NEXUS_Frontend_P_InitMtsifConfig(NEXUS_FrontendDeviceMtsifConfig *pConfig, const void *mxtSettings);
#endif
NEXUS_Error NEXUS_Frontend_P_SetMtsifConfig(NEXUS_FrontendHandle frontend);
void NEXUS_Frontend_P_UnsetMtsifConfig(NEXUS_FrontendHandle frontend);

/***************************************************************************
 * Module Instance Data
 ***************************************************************************/
/* global module handle & data */
extern NEXUS_ModuleHandle g_NEXUS_frontendModule;
extern NEXUS_FrontendModuleSettings g_NEXUS_frontendModuleSettings;

/***************************************************************************
 * Generic tuner handle
 ***************************************************************************/
typedef struct NEXUS_Tuner
{
    NEXUS_OBJECT(NEXUS_Tuner);
    BLST_SQ_ENTRY(NEXUS_Tuner) link; /* list of all frontends maintained by Create/Destroy */
    bool acquired;
    void *pDeviceHandle;
    NEXUS_FrontendDevice *pGenericDeviceHandle;
    NEXUS_StandbyMode mode;
    void (*close)(void *handle);
    NEXUS_Error (*setFrequency)(void *handle, NEXUS_TunerMode mode, unsigned frequency);
    unsigned ifFrequency;
    NEXUS_Error (*init)(void *handle);
    NEXUS_Error (*requestAsyncStatus)(void *handle);
    NEXUS_Error (*getAsyncStatus)(void *handle, NEXUS_TunerStatus *pStatus);
    NEXUS_Error (*getStatus)(void *handle, NEXUS_TunerStatus *pStatus);
    void (*getDefaultTuneSettings)(NEXUS_TunerMode mode, NEXUS_TunerTuneSettings *pSettings);
    NEXUS_Error (*tune)(void *handle, const NEXUS_TunerTuneSettings *pSettings);
    NEXUS_Error (*getSettings)(void *handle, NEXUS_TunerSettings *pSettings);
    NEXUS_Error (*setSettings)(void *handle, const NEXUS_TunerSettings *pSettings);
    void *(*getAgcScript)(void *handle);
    void (*getAttributes)(void *handle, NEXUS_TunerAttributes *pAttributes);
    NEXUS_Error (*readPowerLevel)(void *handle, int *pPowerLevel);
    void (*untune)(void *handle); /* optional */
    NEXUS_Error (*standby)(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings);
    void        (*uninstallCallbacks)(void *handle);
} NEXUS_Tuner;

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Tuner);

typedef struct NEXUS_TunerList{
    BLST_SQ_HEAD(nexus_tuner_list, NEXUS_Tuner) tuners;
} NEXUS_TunerList;

/***************************************************************************
 * Generic method to create a nexus tuner.  It will be automatically
 * destroyed when NEXUS_Tuner_Close is called.
 ***************************************************************************/
NEXUS_TunerHandle NEXUS_Tuner_P_Create(
    void *pDeviceHandle
    );

#if NEXUS_TUNER_SUPPORT
#include "btnr.h"
/***************************************************************************
 * Method to create a tuner from a BTNR handle.
 ***************************************************************************/
NEXUS_Tuner *NEXUS_Tuner_P_CreateFromBTNR(
    BTNR_Handle tnrHandle
    );
#endif

/***************************************************************************
 * Generic IFD Handle
 ***************************************************************************/
#if NEXUS_IFD_SUPPORT
#include "bifd.h"
typedef struct NEXUS_Ifd
{
    NEXUS_OBJECT(NEXUS_Ifd);
    BIFD_Handle ifdHandle;
} NEXUS_Ifd;
#else
typedef struct NEXUS_Ifd
{
    NEXUS_OBJECT(NEXUS_Ifd);
} NEXUS_Ifd;
#endif
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Ifd);

/***************************************************************************
 * Generic Frontend Card handle
 ***************************************************************************/
#define NEXUS_MAX_FRONTEND_CHANNELS 8
#define NEXUS_MAX_FRONTEND_OOB_CHANNELS 1
#define NEXUS_MAX_FRONTEND_AUS_CHANNELS 1

BDBG_OBJECT_ID_DECLARE(NEXUS_FrontendCard);
typedef struct NEXUS_FrontendCard
{
    NEXUS_OBJECT(NEXUS_FrontendCard);
    unsigned numChannels;
    unsigned numOutOfBandChannels;
    unsigned numUpstreamChannels;
    NEXUS_FrontendHandle frontends[NEXUS_MAX_FRONTEND_CHANNELS];
    NEXUS_FrontendHandle outOfBandFrontends[NEXUS_MAX_FRONTEND_OOB_CHANNELS];
    NEXUS_FrontendHandle upstreamFrontends[NEXUS_MAX_FRONTEND_AUS_CHANNELS];
    NEXUS_AmplifierHandle amplifiers[NEXUS_MAX_FRONTEND_CHANNELS];
    NEXUS_TunerHandle tuners[NEXUS_MAX_FRONTEND_CHANNELS];
    NEXUS_IfdHandle ifds[NEXUS_MAX_FRONTEND_CHANNELS];
} NEXUS_FrontendCard;

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_FrontendCard);


/***************************************************************************
 * Frontend Private Routines
 ***************************************************************************/
void NEXUS_Frontend_P_Init(void);
NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_P_Create(void);
NEXUS_FrontendHandle NEXUS_Frontend_P_Create(void *pDeviceHandle);
void NEXUS_Frontend_P_Destroy(NEXUS_FrontendHandle handle);
void NEXUS_Ifd_P_GetDefaultSettings(NEXUS_IfdSettings *pSettings);
bool NEXUS_Frontend_P_ProbeCard(NEXUS_FrontendCard *pCard, const NEXUS_FrontendCardSettings *pSettings);
unsigned NEXUS_Frontend_P_GetDefaultQamSymbolRate(NEXUS_FrontendQamMode mode, NEXUS_FrontendQamAnnex annex);

void NEXUS_FrontendModule_P_Print(void);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_MODULE_H__ */

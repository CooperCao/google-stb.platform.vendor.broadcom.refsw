/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*
* API Description:
*   API name: Frontend AST
*    Generic routines to control any AST satellite frontend
*
***************************************************************************/
#ifndef NEXUS_FRONTEND_AST_H__
#define NEXUS_FRONTEND_AST_H__

#include "nexus_frontend_module.h"
#include "bast.h"
#include "nexus_i2c.h"
#include "breg_i2c.h"

#if BCHP_CHIP==7346 || BCHP_CHIP==73465
#include "bast_g3.h"
#endif

#if NEXUS_FRONTEND_AST_DISABLE_PM
/* remove power management when 4056 external
   is using internal LNA of 7346/73465/... */
#undef NEXUS_POWER_MANAGEMENT
#endif

#ifdef NEXUS_FRONTEND_73XX
#if BCHP_CHIP==7344 || BCHP_CHIP==7358
#include "bast_g3.h"
#else
#include "bast_g2.h"
#endif
#if BCHP_CHIP!=7344 && BCHP_CHIP!=7346 && BCHP_CHIP!=73465
#define TEST_NETWORK_TUNER 1
#endif
#endif

#if defined NEXUS_FRONTEND_4506
#include "bast_4506.h"
#endif
#if defined NEXUS_FRONTEND_4501
#include "bast_4501.h"
#endif

/* NOTE: do not use NEXUS_MAX_FRONTENDS here */
#if BCHP_CHIP==7325
#define NEXUS_P_MAX_AST_CHANNELS 2
#else
#define NEXUS_P_MAX_AST_CHANNELS 3
#endif

/***************************************************************************
Summary:
    AST Devices
***************************************************************************/
typedef struct NEXUS_FrontendAstDevices
{
    NEXUS_TunerHandle tuner;    /* Optional tuner -- used on 97335 and 97325 boards */
} NEXUS_FrontendAstDevices;

/***************************************************************************
Summary:
    AST Channel Settings
***************************************************************************/
typedef struct NEXUS_FrontendAstSettings
{
    BAST_Handle astHandle;
    BAST_ChannelHandle astChannel;
    unsigned astChip; /* unused */
    NEXUS_FrontendAstDevices devices;
    unsigned channelIndex;
    unsigned diseqcIndex;
    unsigned boardRev;
    void (*restoreInternalLnaFunction)(NEXUS_FrontendHandle handle, void *pParam);
    void *pRestoreParam;
    void (*closeFunction)(NEXUS_FrontendHandle handle, void *pParam);   /* Called after handle is closed */
    void *pCloseParam;
    void *pDevice;
    BAST_ChannelHandle (*getDiseqcChannelHandle)(void *param, int index); /* Populate this if ast handle for diseqc purposes is not astChannel */
    void *getDiseqcChannelHandleParam;
    NEXUS_EventCallbackHandle (*getDiseqcEventHandle)(void *param, int index); /* Populated if ast handle for diseqc purposes is not astChannel */
    NEXUS_TaskCallbackHandle (*getDiseqcAppCallback)(void *param, int index); /* Populated if ast handle for diseqc purposes is not astChannel */
    void (*setDiseqcAppCallback)(void *param, int index, NEXUS_TaskCallbackHandle appCallback); /* Populated if ast handle for diseqc purposes is not astChannel */
    NEXUS_FrontendCapabilities capabilities;
    NEXUS_I2cHandle i2cDevice;
    BREG_I2C_Handle i2cRegHandle;
    bool delayedInitialization; /* frontend will call internal AST functions later as part of delayed initialization */
} NEXUS_FrontendAstSettings;

/***************************************************************************
Summary:
    Get Default AST Channel Settings
***************************************************************************/
void NEXUS_Frontend_P_Ast_GetDefaultSettings(
    NEXUS_FrontendAstSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
    Create an AST frontend handle
***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_P_Ast_Create(
    const NEXUS_FrontendAstSettings *pSettings
    );

/***************************************************************************
Summary:
    AST
***************************************************************************/
NEXUS_Error NEXUS_Frontend_P_Ast_DelayedInitialization(
    NEXUS_FrontendHandle frontend
    );

/***************************************************************************
Summary:
    Set devices for an AST channel.
Description:
    May be needed if devices are connected to an on-chip I2C master to
    avoid a chicken and egg problem connecting devices.
***************************************************************************/
NEXUS_Error NEXUS_Frontend_P_Ast_SetDevices(
    NEXUS_FrontendHandle handle,
    const NEXUS_FrontendAstDevices *pDevices
    );

/***************************************************************************
Summary:
    Get the AST channel handle from a frontend handle
***************************************************************************/
BAST_ChannelHandle NEXUS_Frontend_P_Ast_GetChannel(
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

    bool scanFinished;
} NEXUS_SatellitePeakscanStatus;

BDBG_OBJECT_ID_DECLARE(NEXUS_AstDevice);

/***************************************************************************
Summary:
Internal structure for AST channels
Be aware that each NEXUS_AstDevice corresponds to one BAST_ChannelHandle, not one BAST_Handle.
There is a one-to-one mapping of astChannel to frontendHandle.
There is a one-to-many mapping of astHandle to astChannel.
***************************************************************************/
typedef struct NEXUS_AstDevice
{
    BDBG_OBJECT(NEXUS_AstDevice)

    NEXUS_FrontendAstSettings settings;

    BAST_Handle astHandle; /* copied for each BAST_ChannelHandle */
    BAST_ChannelHandle astChannel;
    NEXUS_I2cHandle deviceI2cHandle; /* For a possible master I2C controller */
    NEXUS_FrontendHandle frontendHandle;

    NEXUS_FrontendSatelliteSettings lastSettings;   /* Last tune settings, returned in status */
    NEXUS_FrontendDiseqcSettings diseqcSettings;

    BKNI_EventHandle lockEvent;
    NEXUS_EventCallbackHandle lockEventCallback;
    NEXUS_TaskCallbackHandle lockAppCallback;

    BKNI_EventHandle diseqcEvent;
    NEXUS_EventCallbackHandle diseqcEventCallback;
    NEXUS_TaskCallbackHandle diseqcAppCallback;

    /* This group of variables are used to do blind scans and tone searches via BAST Peak Scan */
    bool toneSearch;
    bool psdSymbolSearch;
    struct {
        int32_t  data_dB[3];
        int32_t  raw_dB[5];
        uint32_t freq[3];
        uint32_t index;
        uint32_t rawIndex;
        uint32_t risingFreq;
        uint32_t dataSize;
        uint32_t risingType;
        uint32_t freqPointIndex;
        uint32_t maxStep;
        uint32_t maxStep_D1;
        uint32_t maxStep_D2;
        uint32_t maxStep_D3;
        uint32_t numCandidates;
    } psd;
    BKNI_EventHandle peakscanEvent;
    NEXUS_EventCallbackHandle peakscanEventCallback;
    NEXUS_TaskCallbackHandle peakscanAppCallback;
    NEXUS_FrontendSatellitePeakscanSettings peakscanSettings;
    NEXUS_SatellitePeakscanStatus peakscanStatus;

    NEXUS_TaskCallbackHandle ftmCallback; /* optionally set by customer extension. must be created here because it's
                                             based on a BAST_Handle event. */

#ifdef NEXUS_FRONTEND_CUSTOMER_EXTENSION
    void *customerData;
#endif

    NEXUS_FrontendCapabilities capabilities; /* copy of capabilities for use in nexus_frontend_ast.c */
    unsigned astChip;
    unsigned channel; /* For multi-demod AST devices, e.g. 4538, this is the channel number associated with this frontend. */
    unsigned diseqcIndex; /* For multi-demod AST devices, e.g. 4538, this is the diseqc output number associated with this frontend. */
    BAST_ChannelHandle (*getDiseqcChannelHandle)(void *param, int index); /* Populated if ast handle for diseqc purposes is not astChannel */
    void *getDiseqcChannelHandleParam;
    NEXUS_EventCallbackHandle (*getDiseqcEventHandle)(void *param, int index); /* Populated if ast handle for diseqc purposes is not astChannel */
    NEXUS_TaskCallbackHandle (*getDiseqcAppCallback)(void *param, int index); /* Populated if ast handle for diseqc purposes is not astChannel */
    void (*setDiseqcAppCallback)(void *param, int index, NEXUS_TaskCallbackHandle appCallback); /* Populated if ast handle for diseqc purposes is not astChannel */

    NEXUS_FrontendType type; /* filled on initial creation, used in GetType */
} NEXUS_AstDevice;

/* only use this your chip's .c file. you must #define B_AST_CHIP in that .c file (never in a .h file). 
this will verify that the handle is correct before doing a potentially dangerous typecast. */
#define NEXUS_Frontend_P_GetAstDevice(frontendHandle) NEXUS_Frontend_P_GetAstDeviceByChip(frontendHandle, B_AST_CHIP)

NEXUS_AstDevice *NEXUS_Frontend_P_GetAstDeviceByChip( NEXUS_FrontendHandle handle, unsigned chipId );

/* Private interface to allow events/callbacks to be unregistered and reregistered */
NEXUS_Error NEXUS_Frontend_P_Ast_RegisterEvents(NEXUS_AstDevice *pChannel);
NEXUS_Error NEXUS_Frontend_P_Ast_UnregisterEvents(NEXUS_AstDevice *pChannel);

/* Private interface for empty satellite capabilities */
NEXUS_Error NEXUS_FrontendDevice_P_Ast_GetSatelliteCapabilities(void *handle, NEXUS_FrontendSatelliteCapabilities *pCapabilities);


#endif /* #ifndef NEXUS_FRONTEND_AST_H__ */


/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
*   API name: Frontend Tuner
*    Generic APIs for tuner device control
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_TUNER_H__
#define NEXUS_TUNER_H__

#include "nexus_frontend.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Generic tuner handle
***************************************************************************/
typedef struct NEXUS_Tuner *NEXUS_TunerHandle;

/***************************************************************************
Summary:
Tuner Mode
***************************************************************************/
typedef enum NEXUS_TunerMode
{
    NEXUS_TunerMode_eDigital,
    NEXUS_TunerMode_eAnalog,
    NEXUS_TunerMode_eQam,
    NEXUS_TunerMode_eVsb,
    NEXUS_TunerMode_eOfdm,
    NEXUS_TunerMode_eMax
} NEXUS_TunerMode;

/***************************************************************************
Summary:
    Set the frequency of a tuner device

Description:
    This API has been deprecated, but is maintained for backward compatibility.
    Please see NEXUS_Tuner_Tune.

See Also:
    NEXUS_Tuner_Tune
***************************************************************************/
NEXUS_Error NEXUS_Tuner_SetFrequency(
    NEXUS_TunerHandle handle,
    NEXUS_TunerMode mode,               /* Tuner Mode */
    unsigned frequency                  /* In Hz */
    );

/***************************************************************************
Summary:
    Initialize current tuner
***************************************************************************/
void NEXUS_Tuner_Init(
    NEXUS_TunerHandle tuner
    );

/***************************************************************************
Summary:
    Tuner Status
***************************************************************************/
typedef struct NEXUS_TunerStatus
{
    uint32_t centerIf;      /* In Hz */
    int32_t ifShift;        /* In Hz */

    unsigned frequency;     /* In Hz */
    bool spectrumInverted;  /* true if spectrum is inverted, false otherwise */
    int adcPgaGain;         /* in 1/10 dB, desired ADC PGA Gain based on
        bandwidth, demodulation, mode (for example DVB-T/ISDB-T for OFDM),
        and frequency, valid only when adcPgaGainReadingSupported of
        NEXUS_TunerAttributes is true */
        
    bool tuneComplete;
    int rssi; /* receive signal strength indicator, units are dBm */
} NEXUS_TunerStatus;

/***************************************************************************
Summary:
    Request current tuner async tatus
***************************************************************************/
NEXUS_Error NEXUS_Tuner_RequestAsyncStatus(
    NEXUS_TunerHandle tuner
    );

/***************************************************************************
Summary:
    Get current tuner async status
***************************************************************************/
NEXUS_Error NEXUS_Tuner_GetAsyncStatus(
    NEXUS_TunerHandle tuner,
    NEXUS_TunerStatus *pStatus  /* [out] */
    );

/***************************************************************************
Summary:
    Get current tuner status
***************************************************************************/
void NEXUS_Tuner_GetStatus(
    NEXUS_TunerHandle tuner,
    NEXUS_TunerStatus *pStatus  /* [out] */
    );

/***************************************************************************
Summary:
    Tuning Parameters
***************************************************************************/
typedef struct NEXUS_TunerTuneSettings
{
    NEXUS_TunerMode mode;

    struct
    {
        struct
        {
            NEXUS_FrontendQamMode qamMode;
            NEXUS_FrontendQamAnnex annex;
        } qam;
        struct
        {
            NEXUS_FrontendVsbMode vsbMode;
        } vsb;
        struct
        {
            NEXUS_FrontendOfdmMode ofdmMode;
        } ofdm;
        struct
        {
            NEXUS_VideoFormat videoFormat;
        } analog;
    } modeSettings;

    unsigned frequency; /* In Hz */
    NEXUS_CallbackDesc tuneCompleteCallback; /* called when tune is complete */
    NEXUS_CallbackDesc asyncStatusReadyCallback; /* called when async status is ready */
} NEXUS_TunerTuneSettings;

/***************************************************************************
Summary:
    Get default settings for a particular tuner mode
***************************************************************************/
void NEXUS_Tuner_GetDefaultTuneSettings(
    NEXUS_TunerMode mode,
    NEXUS_TunerTuneSettings *pSettings  /* [out] */
    );

/***************************************************************************
Summary:
    Tune a tuner device
***************************************************************************/
NEXUS_Error NEXUS_Tuner_Tune(
    NEXUS_TunerHandle tuner,
    const NEXUS_TunerTuneSettings *pSettings
    );

/***************************************************************************
Summary:
    UnTune a tuner device.
***************************************************************************/
void NEXUS_Tuner_Untune(
    NEXUS_TunerHandle tuner
    );

/***************************************************************************
Summary:
    Tuner Settings
***************************************************************************/
typedef struct NEXUS_TunerSettings
{
    unsigned top;           /* In Hz. */
    unsigned ifFrequency;   /* In Hz. */
    unsigned bandwidth;     /* In Hz. */
    int      dacAttenuation; /* DAC Gain Adjust in units of 1/256 of dB, range: 0 to 20*/
} NEXUS_TunerSettings;

/***************************************************************************
Summary:
    Get the current settings for a tuner.
***************************************************************************/
void NEXUS_Tuner_GetSettings(
    NEXUS_TunerHandle tuner,
    NEXUS_TunerSettings *pSettings  /* [out] */
    );

/***************************************************************************
Summary:
    Set the current settings for a tuner.
***************************************************************************/
NEXUS_Error NEXUS_Tuner_SetSettings(
    NEXUS_TunerHandle tuner,
    const NEXUS_TunerSettings *pSettings
    );

/* generic stub needed to create class. do not use. */
NEXUS_TunerHandle NEXUS_Tuner_OpenGeneric( /* attr{destructor=NEXUS_Tuner_Close} */
    void
    );
    
/***************************************************************************
Summary:
    Close a tuner handle
***************************************************************************/
void NEXUS_Tuner_Close(
    NEXUS_TunerHandle handle
    );

/***************************************************************************
Summary:
    Tuner attributes, static information
***************************************************************************/
typedef struct NEXUS_TunerAttributes
{
    struct
    {
        uint32_t major;
        uint32_t minor;
    } version;
    bool powerLevelReadingSupported; /* true if NEXUS_Tuner_ReadPowerLevel() is
        supported, false otherwise. */
    bool adcPgaGainReadingSupported; /* true if NEXUS_TunerStatus.adcPgaGain is
        meaningful, false otherwise. */
} NEXUS_TunerAttributes;

/***************************************************************************
Summary:
    Get the attributes (static information) of a tuner.
***************************************************************************/
void NEXUS_Tuner_GetAttributes(
    NEXUS_TunerHandle tuner,
    NEXUS_TunerAttributes *pAttributes /* [out] */
    );

/***************************************************************************
Summary:
    Read tuner RF input power level

Description:
    This API is supported only when powerLevelReadingSupported of
    NEXUS_TunerAttributes is true. If it is not, this API returns
    NEXUS_NOT_SUPPORTED.

See Also:
    NEXUS_TunerAttributes, NEXUS_Tuner_GetAttributes
***************************************************************************/
NEXUS_Error NEXUS_Tuner_ReadPowerLevel(
    NEXUS_TunerHandle handle,
    int *pPowerLevel /* [out] RF input power level in 1/10 dBm */
    );

/***************************************************************************
Summary:
Settings for NEXUS_Tuner_Acquire
***************************************************************************/
typedef struct NEXUS_TunerAcquireSettings 
{    
    unsigned index; /* used if mode == eByIndex. this is a platform-wide index of Tuner typically corresponding 
        to the index of NEXUS_PlatformConfiguration.Tuner[].
        it may differ if the application opens/closes Tuners outside of platform. */
} NEXUS_TunerAcquireSettings;

/***************************************************************************
Summary:
Get default acquire settings
***************************************************************************/
void NEXUS_Tuner_GetDefaultAcquireSettings(
    NEXUS_TunerAcquireSettings *pSettings
    );
    
/***************************************************************************
Summary:
Find a Tuner based on capabilities and register handle for protected client use
***************************************************************************/
NEXUS_TunerHandle NEXUS_Tuner_Acquire( /* attr{release=NEXUS_Tuner_Release} */
    const NEXUS_TunerAcquireSettings *pSettings
    );

/***************************************************************************
Summary:
Release an acquired client
***************************************************************************/
void NEXUS_Tuner_Release(
    NEXUS_TunerHandle handle
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_TUNER_H__ */


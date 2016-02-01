/***************************************************************************
*     (c)2010-2013 Broadcom Corporation
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
*   API name: Frontend 7552
*    APIs to open, close, and setup initial settings for a BCM7552
*    Demodulator Device.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef NEXUS_FRONTEND_7552_H__
#define NEXUS_FRONTEND_7552_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
7552 has a single tuner for OFDM and QAM, but separate demods that connect to different input bands. 
The app or platform code should open one OFDM handle and one QAM handle, but can only use one at a time. */
typedef enum NEXUS_7552ChannelType
{
    NEXUS_7552ChannelType_eOfdm,
    NEXUS_7552ChannelType_eQam,
    NEXUS_7552ChannelType_eOutOfBand,
    NEXUS_7552ChannelType_eMax
} NEXUS_7552ChannelType;

/***************************************************************************
Summary:
Power mode configuration of the 7552's wide band tuner.
***************************************************************************/
typedef enum NEXUS_7552TunerRfInput
{
    NEXUS_7552TunerRfInput_eOff,         /* Tuner is off. */
    NEXUS_7552TunerRfInput_eExternalLna, /* Tuner Rf input through UHF path. This Rf path does not use internal LNA. */
    NEXUS_7552TunerRfInput_eInternalLna, /* Tuner Rf input through UHF path. This Rf path does uses internal LNA. */
    NEXUS_7552TunerRfInput_eStandardIf,  /* Tuner input is not RF. Insted a (36 or 44MHz) standard IF signal is input.  */
    NEXUS_7552TunerRfInput_eLowIf,       /* Tuner input is not RF. Insted a (4 to 5MHz) Low IF signal is input.  */
    NEXUS_7552TunerRfInput_eBaseband,    /* Tuner input is baseband.  */
    NEXUS_7552TunerRfInput_eMax
} NEXUS_7552TunerRfInput;

/***************************************************************************
Summary:
Settings for a BCM7552 Device
***************************************************************************/
typedef struct NEXUS_FrontendDevice7552OpenSettings
{
    bool supportIsdbt;      /* allocate memory for ISDB-T if true, set at open-time only */

    struct {
        int bypassable; /* in units of 1/100 db. */
        int total; /* includes the bypassable and all non-bypassable fixed gains before this device. in units of 1/100 db. */
    } externalFixedGain;
} NEXUS_FrontendDevice7552OpenSettings;

/***************************************************************************
Summary:
Settings for a BCM7552 channel
***************************************************************************/
typedef struct NEXUS_Frontend7552Settings
{
    NEXUS_FrontendDeviceHandle device;

    /* The below settings are only used in deviceHandle is set to NULL. 
       if different from device settings, channel open will fail. */
    bool supportIsdbt;      /* deprecated */

    /* the following are channel settings */
    NEXUS_7552ChannelType type;
    unsigned channelNumber;         /* Which channel to open from this device */
} NEXUS_Frontend7552Settings;

/***************************************************************************
Summary:
Config settings for a BCM7552 Device
***************************************************************************/
typedef struct NEXUS_FrontendDevice7552Settings
{
    NEXUS_7552TunerRfInput rfInput;        /* Determines how Rf is input to the tuner. */
    bool enableRfLoopThrough;              /* True = Enables RF loop through. */
} NEXUS_FrontendDevice7552Settings;

/***************************************************************************
Summary:
    Get the default settings for a BCM7552 OFDM demodulator
***************************************************************************/
void NEXUS_Frontend_GetDefault7552Settings(
    NEXUS_Frontend7552Settings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
    Open a handle to a BCM7552 OFDM frontend device.
***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open7552(  /* attr{destructor=NEXUS_Frontend_Close} */
    const NEXUS_Frontend7552Settings *pSettings
    );

/***************************************************************************
Summary:
Get the default config settings to a BCM7552 device.
***************************************************************************/
void NEXUS_FrontendDevice_GetDefault7552Settings(
    NEXUS_FrontendDevice7552Settings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
Get the config settings to a BCM7552 device.
***************************************************************************/
void NEXUS_FrontendDevice_Get7552Settings(
    NEXUS_FrontendDeviceHandle handle,                 
    NEXUS_FrontendDevice7552Settings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Set the config settings to a BCM7552 device.
***************************************************************************/
NEXUS_Error NEXUS_FrontendDevice_Set7552Settings(
    NEXUS_FrontendDeviceHandle handle,                 
    const NEXUS_FrontendDevice7552Settings *pSettings    
    );  

/***************************************************************************
Summary:
***************************************************************************/
void NEXUS_FrontendDevice_GetDefault7552OpenSettings(
    NEXUS_FrontendDevice7552OpenSettings *pSettings
    );

/***************************************************************************
Summary:
***************************************************************************/
NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_Open7552(
    unsigned index,
    const NEXUS_FrontendDevice7552OpenSettings *pSettings
    );

#define NEXUS_7552FrontendSettings NEXUS_Frontend7552Settings
#define NEXUS_7552QamFrontendSettings NEXUS_7552FrontendSettings
#define NEXUS_7552OfdmFrontendSettings NEXUS_7552FrontendSettings
#define NEXUS_Frontend_GetDefault7552QamSettings NEXUS_Frontend_GetDefault7552Settings
#define NEXUS_Frontend_GetDefault7552OfdmSettings NEXUS_Frontend_GetDefault7552Settings
#define NEXUS_Frontend_Open7552Qam NEXUS_Frontend_Open7552
#define NEXUS_Frontend_Open7552Ofdm NEXUS_Frontend_Open7552

#define NEXUS_7552ConfigSettings NEXUS_FrontendDevice7552Settings
#define NEXUS_Frontend_GetDefault7552ConfigSettings NEXUS_FrontendDevice_GetDefault7552Settings
#define NEXUS_Frontend_7552_GetConfigSettings(handle,pSettings) NEXUS_FrontendDevice_Get7552Settings(NEXUS_Frontend_GetDevice(handle), pSettings)
#define NEXUS_Frontend_7552_SetConfigSettings(handle,pSettings) NEXUS_FrontendDevice_Set7552Settings(NEXUS_Frontend_GetDevice(handle), pSettings)

/***************************************************************************
Summary:
***************************************************************************/
typedef struct NEXUS_7552ProbeResults
{
    NEXUS_FrontendChipType chip;
} NEXUS_7552ProbeResults;


/***************************************************************************
Summary:
  Probe to see if a BCM7552 device exists with the specified settings

Description:
  Probe to see if a BCM7552 device exists with the specified settings

See Also:
    NEXUS_Frontend_Open7552
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Probe7552(
    const NEXUS_FrontendDevice7552OpenSettings *pSettings,
    NEXUS_7552ProbeResults *pResults    /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_7552_H__ */


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
*   API name: Frontend 3461
*    APIs to open, close, and setup initial settings for a BCM3461
*    Cable and Terrestrial Tuner/Demodulator Device.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef NEXUS_FRONTEND_3461_H__
#define NEXUS_FRONTEND_3461_H__

#include "nexus_frontend.h"
#include "nexus_gpio.h"
#include "nexus_i2c.h"
#include "nexus_spi.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Settings for a BCM3461 Device Channel
***************************************************************************/
typedef enum NEXUS_3461ChannelType
{
    NEXUS_3461ChannelType_eInBand, /* QAM */
    NEXUS_3461ChannelType_eIsdbt,
    NEXUS_3461ChannelType_eDvbt,
    NEXUS_3461ChannelType_eDvbt2,
    NEXUS_3461ChannelType_eDvbc2, /* Does not apply for 3461 and applies only for 3462. */
    NEXUS_3461ChannelType_eMax
} NEXUS_3461ChannelType;

/***************************************************************************
Summary:
Power mode configuration of the 3461's wide band tuner.
***************************************************************************/
typedef enum NEXUS_3461TunerRfInput
{
    NEXUS_3461TunerRfInput_eOff,         /* Tuner is off. */
    NEXUS_3461TunerRfInput_eExternalLna, /* Tuner Rf input through UHF path. This Rf path does not use internal LNA. */
    NEXUS_3461TunerRfInput_eInternalLna, /* Tuner Rf input through UHF path. This Rf path does uses internal LNA. */
    NEXUS_3461TunerRfInput_eStandardIf,  /* Tuner input is not RF. Insted a (36 or 44MHz) standard IF signal is input.  */
    NEXUS_3461TunerRfInput_eLowIf,       /* Tuner input is not RF. Insted a (4 to 5MHz) Low IF signal is input.  */
    NEXUS_3461TunerRfInput_eBaseband,    /* Tuner input is baseband.  */
    NEXUS_3461TunerRfInput_eMax
} NEXUS_3461TunerRfInput;

/***************************************************************************
Summary:
    Enumeration for Tuner Application

****************************************************************************/
typedef enum NEXUS_3461RfDaisyChain
{
    NEXUS_3461RfDaisyChain_eOff,         /* Daisychaining is tunrned off. */
    NEXUS_3461RfDaisyChain_eExternalLna, /* Daisychaining through UHF path. This Rf daisychaining path does not use internal LNA. */
    NEXUS_3461RfDaisyChain_eInternalLna, /* Daisychaining through VHF path. This Rf daisychaining path uses internal LNA. */    
    NEXUS_3461RfDaisyChain_eMax
} NEXUS_3461RfDaisyChain;

/***************************************************************************
Summary:
Settings for a BCM3461 Device
***************************************************************************/
typedef struct NEXUS_FrontendDevice3461OpenSettings
{
    /* either GPIO or an L1 is used for notification from the frontend to the host. */
    NEXUS_GpioHandle gpioInterrupt; /* GPIO pin for interrupt. If not NULL, isrNumber is ignored. If NULL, isrNumber is used. */
    unsigned isrNumber;             /* L1 interrupt number. (typically 0..63). See gpioInterrupt for other interrupt option. */

    /* either I2C or SPI is used for the host to control the frontend. */
    NEXUS_I2cHandle i2cDevice;      /* I2C device to use. spiDevice should be NULL to use I2C. */
    uint16_t i2cAddr;               /* master device I2C Address */
    uint8_t i2cSlaveAddr;           /* slave device I2C Address */

    NEXUS_SpiHandle spiDevice;      /* SPI device to use. i2cDevice should be NULL to use SPI. */
    uint16_t spiAddr;               /* master device SPI Address */

    bool configureWatchdog;         /* Depending on the platform decide whether to configure watchdog or not. */
    bool loadAP;                    /* Load Acquisition Processor microcode when starting. */
    bool isMtsif;                   /* Determines if the transport out if configured for legacy mode or MTSIF. */
    bool inBandOpenDrain;           /* If true, inband is open drain else it is push pull. */

    struct {
        int bypassable; /* in units of 1/100 db. */
        int total; /* includes the bypassable and all non-bypassable fixed gains before this device. in units of 1/100 db. */
    } externalFixedGain;
    
    struct {
        bool enableDaisyChain; /* enable clock to the next frontend device. */
    } crystalSettings; 
} NEXUS_FrontendDevice3461OpenSettings;

/***************************************************************************
Summary:
Settings for a BCM3461 channel
***************************************************************************/
typedef struct NEXUS_Frontend3461Settings
{
    NEXUS_FrontendDeviceHandle device;

    /* The below settings are only used in deviceHandle is set to NULL. 
    if different from device settings, channel open will fail. */
    NEXUS_GpioHandle gpioInterrupt; /* deprecated */
    unsigned isrNumber;             /* deprecated */
    NEXUS_I2cHandle i2cDevice;      /* deprecated */
    uint16_t i2cAddr;               /* deprecated */
    uint8_t i2cSlaveAddr;           /* deprecated */
    NEXUS_SpiHandle spiDevice;      /* deprecated */
    uint16_t spiAddr;               /* deprecated */
    bool configureWatchdog;         /* deprecated */
    bool loadAP;                    /* deprecated */
    bool isMtsif;                   /* deprecated */
    bool inBandOpenDrain;           /* deprecated */

    /* the following are channel settings */
    NEXUS_3461ChannelType type;     /* Channel type to open from this device */
    unsigned channelNumber;         /* Which channel to open from this device */
} NEXUS_Frontend3461Settings;

/***************************************************************************
Summary:
Config settings for a BCM3461 Device
***************************************************************************/
typedef struct NEXUS_FrontendDevice3461Settings
{
    unsigned agcValue;                     /* Gain Value*/
    NEXUS_CallbackDesc updateGainCallback; /* Callback will be called when the gain from the lna needs to be updated. */
    NEXUS_3461TunerRfInput rfInput;        /* Determines how Rf is input to the tuner. */
    NEXUS_3461RfDaisyChain rfDaisyChain;   /* Determines if the Rf daisychain is on/off/uses internal LNA/ does not use internal LNA. */
    bool enableRfLoopThrough;              /* True = Enables RF loop through. */
    bool terrestrial;                      /* Receiver mode: true=terrestrial, false=cable. This supercedes NEXUS_FrontendQamSettings.terrestrial 
                                              and NEXUS_FrontendOfdmSettings.terrestrial and a mismatch will result in a warning. */
} NEXUS_FrontendDevice3461Settings;

/***************************************************************************
Summary:
Get the default settings for a BCM3461 tuner

Description:

See Also:
NEXUS_Frontend_Open3461
***************************************************************************/
void NEXUS_Frontend_GetDefault3461Settings(
    NEXUS_Frontend3461Settings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
Open a handle to a BCM3461 device.

Description:
Close with NEXUS_Frontend_Close
***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open3461( /* attr{destructor=NEXUS_Frontend_Close} */
    const NEXUS_Frontend3461Settings *pSettings
    );


/* Device API. */
/***************************************************************************
Summary:
Get the default config settings to a BCM3461 device.
***************************************************************************/
void NEXUS_FrontendDevice_GetDefault3461Settings(
    NEXUS_FrontendDevice3461Settings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
Get the config settings to a BCM3461 device.
***************************************************************************/
NEXUS_Error NEXUS_FrontendDevice_Get3461Settings(
    NEXUS_FrontendDeviceHandle handle,                 
    NEXUS_FrontendDevice3461Settings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Set the config settings to a BCM3461 device.
***************************************************************************/
NEXUS_Error NEXUS_FrontendDevice_Set3461Settings(
    NEXUS_FrontendDeviceHandle handle,                 
    const NEXUS_FrontendDevice3461Settings *pSettings    
    );  

/***************************************************************************
Summary:
***************************************************************************/
void NEXUS_FrontendDevice_GetDefault3461OpenSettings(
    NEXUS_FrontendDevice3461OpenSettings *pSettings
    );

/***************************************************************************
Summary:
***************************************************************************/
NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_Open3461(
    unsigned index,
    const NEXUS_FrontendDevice3461OpenSettings *pSettings
    );

/* deprecated */
#define NEXUS_Frontend_GetDefault3461ConfigSettings NEXUS_FrontendDevice_GetDefault3461Settings
#define NEXUS_Frontend_3461_GetConfigSettings(handle,pSettings) NEXUS_FrontendDevice_Get3461Settings(NEXUS_Frontend_GetDevice(handle), pSettings)
#define NEXUS_Frontend_3461_SetConfigSettings(handle,pSettings) NEXUS_FrontendDevice_Set3461Settings(NEXUS_Frontend_GetDevice(handle), pSettings)
#define NEXUS_3461Settings NEXUS_Frontend3461Settings 
#define NEXUS_3461ConfigSettings NEXUS_FrontendDevice3461Settings

/***************************************************************************
Summary:
***************************************************************************/
typedef struct NEXUS_3461ProbeResults
{
    NEXUS_FrontendChipType chip;
} NEXUS_3461ProbeResults;


/***************************************************************************
Summary:
  Probe to see if a BCM3461 device exists with the specified settings

Description:
  Probe to see if a BCM3461 device exists with the specified settings

See Also:
    NEXUS_Frontend_Open3461
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Probe3461(
    const NEXUS_FrontendDevice3461OpenSettings *pSettings,
    NEXUS_3461ProbeResults *pResults    /* [out] */
    );

/**
proposed future API's for device-level status and channel-level runtime settings:

typedef struct NEXUS_Frontend3461Status
{
    unsigned tbd;
} NEXUS_Frontend3461Status;

NEXUS_Error NEXUS_FrontendDevice_Get3461Status(
    NEXUS_FrontendDeviceHandle handle,                 
    NEXUS_Frontend3461Status *pStatus   
    );  
    
typedef struct NEXUS_Frontend3461RuntimeSettings
{
    unsigned tbd;
} NEXUS_Frontend3461RuntimeSettings;

void NEXUS_Frontend_Get3461RuntimeSettings(
    NEXUS_FrontendHandle handle,                 
    NEXUS_Frontend3461RuntimeSettings *pSettings    
    );
    
NEXUS_Error NEXUS_Frontend_Set3461RuntimeSettings(
    NEXUS_FrontendHandle handle,                 
    const NEXUS_Frontend3461RuntimeSettings *pSettings    
    );  
**/

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_3461_H__ */



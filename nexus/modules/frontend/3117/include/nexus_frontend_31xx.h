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
*   API name: Frontend 31xx
*    APIs to open, close, and setup initial settings for a BCM31xx
*    Cable Tuner/Demodulator Device.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_FRONTEND_31xx_H__
#define NEXUS_FRONTEND_31xx_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nexus_frontend.h"
#include "nexus_gpio.h"


/***************************************************************************
Summary:
3117 Device Handle
***************************************************************************/
typedef struct NEXUS_31xxDevice *NEXUS_31xxDeviceHandle;

/***************************************************************************
Summary:
Settings for a BCM31xx Device Channel
***************************************************************************/
typedef enum NEXUS_31xxChannelType
{
    NEXUS_31xxChannelType_eInBand,
    NEXUS_31xxChannelType_eOutOfBand,
    NEXUS_31xxChannelType_eUpstream,
    NEXUS_31xxChannelType_eMax
} NEXUS_31xxChannelType;

/***************************************************************************
Summary:
Settings for a BCM31xx Device
***************************************************************************/
typedef struct NEXUS_FrontendDevice31xxOpenSettings
{
    /* either GPIO or an L1 is used for notification from the frontend to the host. */
    NEXUS_GpioHandle gpioInterrupt; /* GPIO pin for interrupt. If not NULL, isrNumber is ignored. If NULL, isrNumber is used. */
    unsigned isrNumber;             /* L1 interrupt number. (typically 0..63). See gpioInterrupt for other interrupt option. */

    /* either I2C or SPI is used for the host to control the frontend. */
    NEXUS_I2cHandle i2cDevice;      /* I2C device to use. spiDevice should be NULL to use I2C. */
    uint16_t i2cAddr;               /* master device I2C Address */
    uint8_t i2cSlaveAddr;           /* slave device I2C Address */

    bool configureWatchdog;         /* Depending on the platform decide whether to configure watchdog or not. */
    bool loadAP;                    /* Load Acquisition Processor microcode when starting. */
    bool inBandOpenDrain;           /* If true, inband is open drain else it is push pull. */

    struct {
        unsigned ifFrequency;       /* IF frequency used for 3128 OOB module. */
        bool openDrain;             /* If true, out of band is open drain else it is push pull. */
    }outOfBand;
} NEXUS_FrontendDevice31xxOpenSettings;

/***************************************************************************
Summary:
Settings for a BCM31xx Device
***************************************************************************/
typedef struct NEXUS_Frontend31xxSettings
{
    NEXUS_FrontendDeviceHandle device;
    NEXUS_31xxChannelType type;     /* Channel type to open from this device */
    unsigned channelNumber;         /* Which channel to open from this device */


    /* the following have been deprecated. see NEXUS_FrontendDevice31xxOpenSettings. */
    NEXUS_I2cHandle i2cDevice;      /* deprecated */
    NEXUS_GpioHandle gpioInterrupt; /* deprecated */
    unsigned isrNumber;             /* deprecated */
    uint16_t i2cAddr;               /* deprecated */
    uint8_t i2cSlaveAddr;           /* deprecated */

    unsigned ifFrequency;           /* deprecated */
    bool configureWatchdog;         /* deprecated */
    bool outOfBandOpenDrain;        /* deprecated */
    bool inBandOpenDrain;           /* deprecated */
    bool loadAP;                    /* deprecated */
} NEXUS_Frontend31xxSettings;

/***************************************************************************
Summary:
Config settings for a BCM31xx Device
***************************************************************************/
typedef struct NEXUS_FrontendDevice31xxSettings
{
    unsigned agcValue;                     /* Gain Value*/
    NEXUS_CallbackDesc updateGainCallback; /* Callback will be called when the gain from the lna needs to be updated. */
    bool enableDaisyChain;
    NEXUS_CallbackDesc qamAsyncStatusReadyCallback;   /* Callback will be called when the async qam status is ready. */
    NEXUS_CallbackDesc oobAsyncStatusReadyCallback; /* Callback will be called when the async out of band status is ready. */
} NEXUS_FrontendDevice31xxSettings;

/***************************************************************************
Summary:
Get the default settings for a BCM31xx tuner

Description:

See Also:
NEXUS_Frontend_Open31xx
***************************************************************************/
void NEXUS_Frontend_GetDefault31xxSettings(
    NEXUS_Frontend31xxSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
Open a handle to a BCM31xx device.

Description:

See Also:
NEXUS_Frontend_Close
***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open31xx( /* attr{destructor=NEXUS_Frontend_Close} */
    const NEXUS_Frontend31xxSettings *pSettings
    );

/* Device API. */
/***************************************************************************
Summary:
Get the default config settings to a BCM31xx device.
***************************************************************************/
void NEXUS_FrontendDevice_GetDefault31xxSettings(
    NEXUS_FrontendDevice31xxSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
Get the config settings to a BCM31xx device.
***************************************************************************/
NEXUS_Error NEXUS_FrontendDevice_Get31xxSettings(
    NEXUS_FrontendDeviceHandle handle,                 
    NEXUS_FrontendDevice31xxSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Set the config settings to a BCM31xx device.
***************************************************************************/
NEXUS_Error NEXUS_FrontendDevice_Set31xxSettings(
    NEXUS_FrontendDeviceHandle handle,                 
    const NEXUS_FrontendDevice31xxSettings *pSettings    
    );  

/***************************************************************************
Summary:
***************************************************************************/
void NEXUS_FrontendDevice_GetDefault31xxOpenSettings(
    NEXUS_FrontendDevice31xxOpenSettings *pSettings
    );

/***************************************************************************
Summary:
***************************************************************************/
NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_Open31xx(
    unsigned index,
    const NEXUS_FrontendDevice31xxOpenSettings *pSettings
    );

/* deprecated */
#define NEXUS_Frontend_GetDefault31xxConfigSettings NEXUS_FrontendDevice_GetDefault31xxSettings
#define NEXUS_Frontend_31xx_GetConfigSettings(handle,pSettings) NEXUS_FrontendDevice_Get31xxSettings(NEXUS_Frontend_GetDevice(handle), pSettings)
#define NEXUS_Frontend_31xx_SetConfigSettings(handle,pSettings) NEXUS_FrontendDevice_Set31xxSettings(NEXUS_Frontend_GetDevice(handle), pSettings)
#define NEXUS_31xxSettings NEXUS_Frontend31xxSettings 
#define NEXUS_31xxConfigSettings NEXUS_FrontendDevice31xxSettings

/***************************************************************************
Summary:
***************************************************************************/
typedef struct NEXUS_31xxProbeResults
{
    NEXUS_FrontendChipType chip;
} NEXUS_31xxProbeResults;

/***************************************************************************
Summary:
  Probe to see if a BCM31xx device exists with the specified settings

Description:
  Probe to see if a BCM31xx device exists with the specified settings

See Also:
    NEXUS_Frontend_Open31xx
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Probe31xx(
    const NEXUS_FrontendDevice31xxOpenSettings *pSettings,
    NEXUS_31xxProbeResults *pResults    /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_31xx_H__ */


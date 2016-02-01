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
*   API name: Frontend 7346
*    APIs to open, close, and setup initial settings for a BCM7346
*    Dual-Channel Integrated Satellite Tuner/Demodulator Device.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef NEXUS_FRONTEND_7346_H__
#define NEXUS_FRONTEND_7346_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nexus_frontend.h"
#include "nexus_i2c.h"
#include "nexus_frontend_3445.h"

#ifndef NEXUS_7346_MAX_FRONTEND_CHANNELS
#define NEXUS_7346_MAX_FRONTEND_CHANNELS 2
#endif

/******************************************************************************
Summary:
   Specifies the configuration of an output channel on the attached LNA

Description:
   Specifies the configuration of an output channel on the attached LNA
******************************************************************************/
typedef enum NEXUS_7346LnaInput
{
    NEXUS_7346LnaInput_eNone = 0,    /* output is powered off */
    NEXUS_7346LnaInput_eIn0,         /* output comes from IN0. Mapped to BAST_TunerLnaOutputConfig_eIn0 */
    NEXUS_7346LnaInput_eIn1,         /* output comes from IN1. Mapped to BAST_TunerLnaOutputConfig_eIn1 */
    NEXUS_7346LnaInput_eDaisy,       /* output comes from daisy */
    NEXUS_7346LnaInput_eMax
} NEXUS_7346LnaInput;

/******************************************************************************
Summary:
    LNA output configuration.

Description:
    This structure specifies the connection of LNA inputs to outputs.

For example, this configuration:
      +----------+
IN0 --+--\       +-- OUT0
      |   \      +-- OUT1
IN1 --+    \-----+-- DAISY
      +----------+

would be represented by:
        NEXUS_7346LnaSettings lnaSettings;
        lnaSettings.out0 = NEXUS_7346LnaInput_eNone;
        lnaSettings.out1 = NEXUS_7346LnaInput_eNone;
        lnaSettings.daisy = NEXUS_7346LnaInput_eIn0;

    If the input for a given output is set to NEXUS_7346LnaInput_eNone, then this LNA
will not be used.

    NEXUS_7346LnaSettings controls the internal LNA on a 7346.
******************************************************************************/
typedef struct NEXUS_7346LnaSettings
{
   NEXUS_7346LnaInput out0;  /* Configure the input linked to OUT0. Set this to eNone to not use this LNA. */
   NEXUS_7346LnaInput out1;  /* Configure the input linked to OUT1. Set this to eNone to not use this LNA. */
   NEXUS_7346LnaInput daisy; /* Configure the input linked to DAISY. Set this to eNone to not use this LNA. */
} NEXUS_7346LnaSettings;

/******************************************************************************
Summary:
   Specifies an output channel on the attached LNA
******************************************************************************/
typedef enum NEXUS_7346LnaOutput
{
    NEXUS_7346LnaOutput_eNone = 0, /* No output */
    NEXUS_7346LnaOutput_eOut0,     /* LNA primary output */
    NEXUS_7346LnaOutput_eOut1,     /* LNA secondary output */
    NEXUS_7346LnaOutput_eDaisy,    /* LNA daisy output */
    NEXUS_7346LnaOutput_eMax
} NEXUS_7346LnaOutput;

/***************************************************************************
Summary:
    Settings for a BCM7346 Device
 ***************************************************************************/
typedef struct NEXUS_7346FrontendSettings
{
    /* The channel of the device */
    unsigned channelNumber;              /* Which channel from the device will be opened */
    
    bool isInternalLna;                  /* If true, 7346 uses the internal LNA too. */
    NEXUS_7346LnaSettings lnaSettings;   /* Internal LNA settings. */

    struct {
        bool enabled;                    /* If true, demod is connected and uses the external 3445 LNA */
        NEXUS_3445LnaSettings settings;  /* 3445 settings */
        unsigned i2cChannelNumber;       /* i2c bus for 3445 */
        NEXUS_3445LnaOutput lnaOutput;   /* Which LNA output feeds this channel. Set to eNone for external LNA. */
    } external3445Lna;

    bool lnbPowerUpPinSelect; /* if true, the LNBPU signal will be routed to the TXEN pin */
    bool usePga;              /* If true, override the external LNA path to use the PGA pins. */
} NEXUS_7346FrontendSettings;

/***************************************************************************
Summary:
    Get the default settings for a BCM7346 frontend

Description:
See Also:
    NEXUS_Frontend_Open7346
 ***************************************************************************/
void NEXUS_Frontend_GetDefault7346Settings(
    NEXUS_7346FrontendSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
    Open a handle to a BCM7346 device.

Description:
See Also:
    NEXUS_Frontend_Close7346
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open7346(  /* attr{destructor=NEXUS_Frontend_Close} */
    const NEXUS_7346FrontendSettings *pSettings
    );

/***************************************************************************
Summary:
    Get the settings of an LNA attached to the 7346 frontend
 ***************************************************************************/
void NEXUS_Frontend_Get7346LnaSettings(
    NEXUS_FrontendHandle handle,
    NEXUS_7346LnaSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Set the settings of an LNA attached to the 7346 frontend

Description:
Do not call this for an external LNA.
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Set7346LnaSettings(
    NEXUS_FrontendHandle handle,
    const NEXUS_7346LnaSettings *pSettings
    );

/***************************************************************************
Summary:
    Get an I2C Handle for the 7346 frontend I2C controller.

Description:
    The BCM7346 chips have an on-board I2C master interface per frontend
    channel.  This function allows you to get a handle to this interface
    for use with other I2C devices such as tuners or DiSEQc controllers.
    This interface implements the Read, ReadNoAddr and Write, and WriteNoAddr
    routines.

See Also:
    NEXUS_I2c_Open
    NEXUS_I2c_Read
    NEXUS_I2c_Write
    NEXUS_I2c_ReadNoAddr
    NEXUS_I2c_WriteNoAddr
 ***************************************************************************/
NEXUS_I2cHandle NEXUS_Frontend_Get7346MasterI2c(
    NEXUS_FrontendHandle handle
    );

/***************************************************************************
Summary:
    Status for a BCM7346 LNA Device
 ***************************************************************************/
typedef struct NEXUS_7346LnaStatus
{
    NEXUS_7346LnaInput  lnaInput;
    unsigned            status;
    unsigned            agc;         /* LNA AGC value, typically used in input power calculations */
    unsigned            baseBandAgc; /* Baseband AGC value, typically used in input power calculations */
} NEXUS_7346LnaStatus;

/***************************************************************************
Summary:
Get the status of an LNA attached to the 7346 frontend

Description:
Do not call this for an external LNA.
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Get7346LnaStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_7346LnaStatus *pStatus    /* [out] */
    );

/***************************************************************************
Summary:
Additional 7346 settings used for tuning

Description:
These settings are 7346 specific. Instead of adding them to the generic NEXUS_FrontendSatelliteSettings,
they are added in this chip-specific settings structure.
***************************************************************************/
typedef struct NEXUS_Frontend7346TuneSettings
{
    bool disableFecReacquire;
    bool bypassLnaGain; /* Bypass the internal LNA Gain stage */
    struct  {
        bool override;  /* Override the value of Daisy Gain */
        unsigned value; /* Daisy Gain value to be used */
    } daisyGain; 
} NEXUS_Frontend7346TuneSettings;

/***************************************************************************
Summary:
Get current 7346 settings using for tuning
***************************************************************************/
void NEXUS_Frontend_Get7346TuneSettings(
    NEXUS_FrontendHandle handle,
    NEXUS_Frontend7346TuneSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Set new 7346 settings used for tuning, usually set before calling NEXUS_Frontend_TuneSatellite
***************************************************************************/
NEXUS_Error NEXUS_Frontend_Set7346TuneSettings(
    NEXUS_FrontendHandle handle,
    const NEXUS_Frontend7346TuneSettings *pSettings
    );

/*
 * Power Estimation values for 7346:
 *
 * The values returned in NEXUS_FrontendSatelliteAgcStatus are from
 * BAST_GetChannelStatus and BAST_GetTunerLnaStatus:
 *
 * [0]: BAST_ChannelStatus.IFagc
 * [1]: BAST_ChannelStatus.agf
 * [2]: BAST_ChannelStatus.tunerFreq
 * [3]: BAST_TunerLnaStatus.lnaAgc
 * [4]: BAST_TunerLnaStatus.bbAgc
 */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_7346_H__ */


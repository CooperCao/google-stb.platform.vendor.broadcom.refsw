/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
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
*   API name: Frontend 73xx
*    APIs to open, close, and setup initial settings for a BCM73xx
*    Dual-Channel Satellite Tuner/Demodulator Device.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_FRONTEND_73XX_H__
#define NEXUS_FRONTEND_73XX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nexus_frontend.h"
#include "nexus_i2c.h"

/******************************************************************************
Summary:
   Specifies the configuration of an output channel on the attached LNA

Description:
   Specifies the configuration of an output channel on the attached LNA
******************************************************************************/
typedef enum NEXUS_73xxLnaInput
{
    NEXUS_73xxLnaInput_eNone = 0, /* output is powered off */
    NEXUS_73xxLnaInput_eDaisy,    /* output comes from daisy */
    NEXUS_73xxLnaInput_eIn1Vga,   /* output comes from IN1(VGA) */
    NEXUS_73xxLnaInput_eIn1Db,    /* output comes from IN1(DB) */
    NEXUS_73xxLnaInput_eIn2Vga,   /* output comes from IN2(VGA) */
    NEXUS_73xxLnaInput_eMax
} NEXUS_73xxLnaInput;

/******************************************************************************
Summary:
   Specifies an output channel on the attached LNA
******************************************************************************/
typedef enum NEXUS_73xxLnaOutput
{
    NEXUS_73xxLnaOutput_eNone = 0, /* No output */
    NEXUS_73xxLnaOutput_eOut1,     /* LNA primary output */
    NEXUS_73xxLnaOutput_eOut2,     /* LNA secondary output */
    NEXUS_73xxLnaOutput_eDaisy,    /* LNA daisy output */
    NEXUS_73xxLnaOutput_eMax
} NEXUS_73xxLnaOutput;

/******************************************************************************
Summary:
    LNA output configuration.

Description:
    This structure specifies the configuration of the attached LNA outputs.

For example, this configuration:
              +------------+
        IN1 --+-+-VGA------+-- OUT1
              |  \-DB-\    +-- OUT2
        IN2 --+        \---+-- DAISY
              +------------+

would be represented by:
        NEXUS_73xxLnaSettings lnaSettings;
        lnaSettings.out1 = NEXUS_73xxLnaInput_eIn1Vga;
        lnaSettings.out2 = NEXUS_73xxLnaInput_eNone;
        lnaSettings.daisy = NEXUS_73xxLnaInput_eIn1Db;

    If the input for a given output is set to NEXUS_73xxLnaInput_eNone, then this LNA
will not be used.
******************************************************************************/
typedef struct NEXUS_73xxLnaSettings
{
   NEXUS_73xxLnaInput out1;  /* Configure the input linked to OUT1. Set to eNone to not use this LNA. */
   NEXUS_73xxLnaInput out2;  /* Configure the input linked to OUT2. Set to eNone to not use this LNA. */
   NEXUS_73xxLnaInput daisy; /* Configure the input linked to DAISY. Set to eNone to not use this LNA. */
} NEXUS_73xxLnaSettings;

/***************************************************************************
Summary:
    Settings for a BCM73xx Device
 ***************************************************************************/
typedef struct NEXUS_73xxFrontendSettings
{
    /* The channel of the device */
    unsigned channelNumber;             /* Which channel from the device will be opened */

    /* Device properties*/
    unsigned lnaI2cChannelNumber;       /* Specifies the Satellite I2C bus that the LNA is attached to */
    NEXUS_73xxLnaSettings lnaSettings;

    /* Channel properties */
    NEXUS_73xxLnaOutput lnaOutput;      /* What LNA output feeds this channel. Set to eNone for external LNA. */

    bool lnbPowerUpPinSelect; /* if true, the LNBPU signal will be routed to the TXEN pin */
} NEXUS_73xxFrontendSettings;

/***************************************************************************
Summary:
    Get the default settings for a BCM73xx frontend

Description:
See Also:
    NEXUS_Frontend_Open73xx
 ***************************************************************************/
void NEXUS_Frontend_GetDefault73xxSettings(
    NEXUS_73xxFrontendSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
    Open a handle to a BCM73xx device.

Description:
See Also:
    NEXUS_Frontend_Close73xx
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open73xx(  /* attr{destructor=NEXUS_Frontend_Close} */
    const NEXUS_73xxFrontendSettings *pSettings
    );

/***************************************************************************
Summary:
    Get the settings of an LNA attached to the 73xx frontend
 ***************************************************************************/
void NEXUS_Frontend_Get73xxLnaSettings(
    NEXUS_FrontendHandle handle,
    NEXUS_73xxLnaSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Set the settings of an LNA attached to the 73xx frontend

Description:
Do not call this for an external LNA.
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Set73xxLnaSettings(
    NEXUS_FrontendHandle handle,
    const NEXUS_73xxLnaSettings *pSettings
    );

/***************************************************************************
Summary:
    Get an I2C Handle for the 73xx frontend I2C controller.

Description:
    The BCM73xx chips have an on-board I2C master interface per frontend
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
NEXUS_I2cHandle NEXUS_Frontend_Get73xxMasterI2c(
    NEXUS_FrontendHandle handle
    );

/***************************************************************************
Summary:
    Status for a BCM73xx LNA Device
 ***************************************************************************/
typedef struct NEXUS_73xxLnaStatus
{
    NEXUS_73xxLnaInput  lnaInput;
    NEXUS_73xxLnaOutput lnaOutput;
    uint8_t             status;      /* see BAST_BCM3445_STATUS_* macros */
    uint8_t             version;     /* BCM3445 version number */
    uint8_t             agc;         /* AGC value read from BCM3445 */
} NEXUS_73xxLnaStatus;

/***************************************************************************
Summary:
Get the status of an LNA attached to the 73xx frontend

Description:
Do not call this for an external LNA.
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Get73xxLnaStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_73xxLnaStatus *pStatus    /* [out] */
    );

/***************************************************************************
Summary:
Additional 73xx settings used for tuning

Description:
These settings are 73xx specific. Instead of adding them to the generic NEXUS_FrontendSatelliteSettings,
they are added in this chip-specific settings structure.
***************************************************************************/
typedef struct NEXUS_Frontend73xxTuneSettings
{
    bool disableFecReacquire;
} NEXUS_Frontend73xxTuneSettings;

/***************************************************************************
Summary:
Get current 73xx settings using for tuning
***************************************************************************/
void NEXUS_Frontend_Get73xxTuneSettings(
    NEXUS_FrontendHandle handle,
    NEXUS_Frontend73xxTuneSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
Set new 73xx settings used for tuning, usually set before calling NEXUS_Frontend_TuneSatellite
***************************************************************************/
NEXUS_Error NEXUS_Frontend_Set73xxTuneSettings(
    NEXUS_FrontendHandle handle,
    const NEXUS_Frontend73xxTuneSettings *pSettings
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_73XX_H__ */


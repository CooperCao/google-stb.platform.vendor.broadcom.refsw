/***************************************************************************
*     (c)2004-2011 Broadcom Corporation
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
*   API name: Frontend 7125
*    APIs to open, close, and setup initial settings for demodulators on a
*    BCM7125 Device.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_FRONTEND_7125_H__
#define NEXUS_FRONTEND_7125_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nexus_frontend.h"

/***************************************************************************
Summary:
Supporting devices for a BCM7125 Demodulator Device
***************************************************************************/
typedef struct NEXUS_7125FrontendDevices
{
    NEXUS_TunerHandle tuner;            /* Optional, pass NULL if the tuner is controlled exernally */
} NEXUS_7125FrontendDevices;

/***************************************************************************
Summary:
Settings for a BCM7125 QAM Demodulator
***************************************************************************/
typedef struct NEXUS_7125FrontendSettings
{
    NEXUS_7125FrontendDevices devices;  /* Optional, set of attached devices and parameters.  This is set only at open-time. */
    unsigned channelNumber;             /* Which channel to open from this device. This is set only at open-time. */
    bool bypassEnabled;                 /* Enables or disables down stream RF bypass. This can be set at run-time. */ 
} NEXUS_7125FrontendSettings;

/***************************************************************************
Summary:
Config Settings for a BCM7125 Device Channel
***************************************************************************/
typedef struct NEXUS_7125ConfigSettings
{
    unsigned agcValue;              /* obtained from NEXUS_AmplifierStatus.agcGain */
    bool gainBoostEnabled;          /* obtained from NEXUS_AmplifierStatus.gainBoostEnabled */
    bool superBoostEnabled;         /* obtained from NEXUS_AmplifierStatus.superBoostEnabled */
    bool tiltEnabled;               /* obtained from NEXUS_AmplifierStatus.tiltEnabled */
    NEXUS_CallbackDesc updateGainCallback;    /* Callback will be called when the gain from the lna needs to be updated. */
} NEXUS_7125ConfigSettings;

/***************************************************************************
Summary:
Get the default settings for a BCM7125 QAM demodulator
***************************************************************************/
void NEXUS_Frontend_GetDefault7125Settings(
    NEXUS_7125FrontendSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
    Open a handle to a BCM7125 QAM frontend device.

Description:
Close with NEXUS_Frontend_Close
***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open7125(  /* attr{destructor=NEXUS_Frontend_Close} */
    const NEXUS_7125FrontendSettings *pSettings
    );

/***************************************************************************
Summary:
Get the default config settings to a BCM7125 device.
***************************************************************************/
void NEXUS_Frontend_GetDefault7125ConfigSettings(
    NEXUS_7125ConfigSettings *pConfigSettings   /* [out] */
    );

/***************************************************************************
Summary:
Get the config settings to a BCM7125 device.
***************************************************************************/
NEXUS_Error NEXUS_Frontend_7125_GetConfigSettings(
    NEXUS_FrontendHandle handle,
    NEXUS_7125ConfigSettings *pConfigSettings    /* [out]  */
    );

/***************************************************************************
Summary:
Set the config settings to a BCM7125 device.
***************************************************************************/
NEXUS_Error NEXUS_Frontend_7125_SetConfigSettings(
    NEXUS_FrontendHandle handle,
    const NEXUS_7125ConfigSettings *pConfigSettings
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_7125_H__ */


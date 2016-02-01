/***************************************************************************
*     (c)2012-2013 Broadcom Corporation
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
* API name:  DOCSIS
* APIs to open a DOCSIS device and it's channels.
* These APIs also link the private APIs of DOCSIS
* device and channels to the FrontendDevice
* and Frontend API abstractions.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef NEXUS_DOCSIS_H__
#define NEXUS_DOCSIS_H__

#include "nexus_frontend.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Enumerations for DOCSIS channel types
***************************************************************************/
typedef enum NEXUS_DocsisChannelType
{
    NEXUS_DocsisChannelType_eQam,
    NEXUS_DocsisChannelType_eOutOfBand,
    NEXUS_DocsisChannelType_eUpstream,
    NEXUS_DocsisChannelType_eData,
    NEXUS_DocsisChannelType_eMax
} NEXUS_DocsisChannelType;


/***************************************************************************
Summary:
Capabilities for a DOCSIS device.
***************************************************************************/
typedef struct NEXUS_DocsisDeviceCapabilities
{
    unsigned numDataChannels;   /* Number of DOCSIS data channels
                                   controlled by eCM SW on a DOCSIS device */
    unsigned numQamChannels;    /* Number of QAM Video channels */
    unsigned numOutOfBandChannels;  /* Number of OOB channels */
    unsigned numUpStreamChannels;   /* Number of upStream channels */
    unsigned totalChannels;     /* Total number of channels (QAM+OOB+US+DATA) */
    unsigned numTsmfParsers;    /* Number of TSMF parsers on DOCSIS device */
    bool isMtsif;               /* Is this device based on MTSIF interface? */
}NEXUS_DocsisDeviceCapabilities;

#define NEXUS_DOCSIS_MAX_DATA_TUNERS 1

/***************************************************************************
Summary:
Open settings for a DOCSIS device
***************************************************************************/
typedef struct NEXUS_DocsisOpenDeviceSettings
{
    unsigned rpcTimeOut;
    NEXUS_TunerHandle dataTuner[NEXUS_DOCSIS_MAX_DATA_TUNERS]; /* Remote host data tuner. */
} NEXUS_DocsisOpenDeviceSettings;


/***************************************************************************
Summary:
Open settings for a DOCSIS channel
***************************************************************************/
typedef struct NEXUS_DocsisOpenChannelSettings
{

    unsigned channelNum;                  /* The channel index to open from a docsis device.
                                             Applicable for QAM channels only */
    NEXUS_DocsisChannelType channelType;  /* QAM/DATA/OOB/UPSTREAM*/
    bool autoAcquire;
    bool fastAcquire;
    bool enableFEC;
} NEXUS_DocsisOpenChannelSettings;


/***************************************************************************
Summary:
Docsis device state
***************************************************************************/
typedef enum NEXUS_DocsisDeviceState
{
    NEXUS_DocsisDeviceState_eUninitialized,
    NEXUS_DocsisDeviceState_eOperational,
    NEXUS_DocsisDeviceState_eReset,
    NEXUS_DocsisDeviceState_eFailed,
    NEXUS_DocsisDeviceState_eMax
} NEXUS_DocsisDeviceState;

/***************************************************************************
Summary:
Docsis device status
***************************************************************************/
typedef struct NEXUS_DocsisDeviceStatus
{
    NEXUS_DocsisDeviceState state;
} NEXUS_DocsisDeviceStatus;


/***************************************************************************
Summary:
Docsis device settings
***************************************************************************/
typedef struct NEXUS_DocsisDeviceSettings
{
    NEXUS_CallbackDesc stateChange; /* Callback when frontend device status changes.
                                       App can call NEXUS_Docsis_GetDeviceStatus
                                       to get changed status. */
} NEXUS_DocsisDeviceSettings;

/***************************************************************************
Summary:
Get default settings for NEXUS_Docsis_OpenDevice
***************************************************************************/
void NEXUS_Docsis_GetDefaultOpenDeviceSettings(
    NEXUS_DocsisOpenDeviceSettings *pOpenSettings  /* [out] */
    );

/***************************************************************************
Summary:
Initialize a docsis device
***************************************************************************/
NEXUS_FrontendDeviceHandle NEXUS_Docsis_OpenDevice(
    unsigned index,
    const NEXUS_DocsisOpenDeviceSettings *pSettings
    );

/***************************************************************************
Summary:
Get the docsis device capabilities
***************************************************************************/
void NEXUS_Docsis_GetDeviceCapabilities(
    NEXUS_FrontendDeviceHandle hDevice,
    NEXUS_DocsisDeviceCapabilities *pCapabilities
    );

/***************************************************************************
Summary:
Get the default settings for NEXUS_Docsis_OpenChannel
***************************************************************************/
void NEXUS_Docsis_GetDefaultOpenChannelSettings(
    NEXUS_DocsisOpenChannelSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
Open a docsis channel like QAM/US/OOB/DATA.
***************************************************************************/
NEXUS_FrontendHandle NEXUS_Docsis_OpenChannel( /* attr{destructor=NEXUS_Frontend_Close} */
    NEXUS_FrontendDeviceHandle hDevice,
    const NEXUS_DocsisOpenChannelSettings *pOpenSettings
    );

/***************************************************************************
Summary:
Returns the frontend device status
***************************************************************************/
NEXUS_Error NEXUS_Docsis_GetDeviceStatus(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_DocsisDeviceStatus *pStatus  /* [out] */
    );

/***************************************************************************
Summary:
Returns the frontend device settings
***************************************************************************/
void NEXUS_Docsis_GetDeviceSettings(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_DocsisDeviceSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Sets the frontend device settings
***************************************************************************/
NEXUS_Error NEXUS_Docsis_SetDeviceSettings(
    NEXUS_FrontendDeviceHandle handle,
    const NEXUS_DocsisDeviceSettings *pSettings
    );

/***************************************************************************
Summary:
Enable/disable the Cable Card pins (CC_DRX & CC_CRX & CC_CTX)
that are specifically used for an OOB channel. These pins should be enabled
when Cable Card plug in/out is detected in the application
***************************************************************************/
NEXUS_Error NEXUS_Docsis_EnableCableCardOutOfBandPins(
    NEXUS_FrontendDeviceHandle handle,
    bool enabled
    );

/***************************************************************************
Summary:
Initiates a manual LNA reconfiguration which consists of a CPPM calculation.
An LNA reconfiguration is automatically performed by the DOCSIS device
with every 20th tune attempt on the condition that the 20th tune attempt fails.
This API provides a mechanism to additionally perform this reconfiguration
at any arbitrary point.  However, the auto reconfiguration can be turned
off by completely by calling BRPC_ProcId_ECM_HostChannelsLockStatus and
passing in a non-zero "lockStatus" value. It can be turned on again by
calling this same function with a value of zero.
***************************************************************************/
NEXUS_Error NEXUS_Docsis_ConfigureDeviceLna(
    NEXUS_FrontendDeviceHandle handle
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_DOCSIS_H__ */


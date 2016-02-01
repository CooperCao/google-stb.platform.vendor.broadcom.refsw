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
*   API name: Frontend 3255
*    APIs to open, close, and setup initial settings for a BCM3255
*    Cable Tuner/Demodulator Device.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_FRONTEND_3255_H__
#define NEXUS_FRONTEND_3255_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nexus_frontend.h"
#include "nexus_gpio.h"


/***************************************************************************
Summary:
    3255 Device Handle
 ***************************************************************************/
typedef struct NEXUS_3255Device *NEXUS_3255DeviceHandle;

/***************************************************************************
Summary:
    Settings for a BCM3255 Device Channel
 ***************************************************************************/
typedef enum NEXUS_3255ChannelType
{
    NEXUS_3255ChannelType_eInBand,
    NEXUS_3255ChannelType_eOutOfBand,
    NEXUS_3255ChannelType_eUpstream,
    NEXUS_3255ChannelType_eMax
} NEXUS_3255ChannelType;

/***************************************************************************
Summary:
    Settings for a BCM3255 Device
 ***************************************************************************/
typedef struct NEXUS_3255Settings
{
    bool enableFEC;                       /* 3255 will do FEC or not */
    uint32_t ipAddress;                   /* 3255 IP Address -- Identifies which device is being used */
    bool fastAcquire;                     /* ADS fast acquire enabled */
    bool autoAcquire;                     /* 325X auto acquire enabled */
    NEXUS_3255ChannelType type;           /* The channel type to open from this device */
    unsigned channelNumber;               /* The channel index to open from this device */
    NEXUS_CallbackDesc docsisStateChange; /* callback when NEXUS_FrontendDocsisStatus.status changes */
    bool disableDocsis;                   /* Ds0 will act as Video tuner or Docsis tuner */
                                          /*   true:  stop Docsis scanning and switch DS0 as video tuner*/
                                          /*            Note that it takes up to 2 seconds to shutdown Docsis*/
                                          /*   false: switch DS0 as Docsis tuner and start scanning immediately*/
} NEXUS_3255Settings;

/***************************************************************************
Summary:
   Types for 325x GPIO pins
 ***************************************************************************/

typedef enum NEXUS_3255GpioPin
{
    NEXUS_3255GpioPin_eOob,
    NEXUS_3255GpioPin_eMax
} NEXUS_3255GpioPin;

 /***************************************************************************
Summary:
   Settings for 325x GPIO pins
 ***************************************************************************/
typedef struct NEXUS_3255GpioPinSettings
{
    NEXUS_GpioMode mode;
    NEXUS_GpioValue value;
} NEXUS_3255GpioPinSettings;

 /***************************************************************************
Summary:
   Docsis channel S-CDMA statua
 ***************************************************************************/
typedef enum NEXUS_DocsisScdmaType
{
    NEXUS_DocsisScdmaType_eNo,
    NEXUS_DocsisScdmaType_eTcm,
    NEXUS_DocsisScdmaType_eTdma,
    NEXUS_DocsisScdmaType_eOther
} NEXUS_DocsisScdmaType;

 /***************************************************************************
Summary:
   Docsis Upstream Modulation Type
 ***************************************************************************/
typedef enum NEXUS_DocsisUsModType
{
    NEXUS_DocsisUsModType_eQpsk,
    NEXUS_DocsisUsModType_eQam16,
    NEXUS_DocsisUsModType_eQam32,
    NEXUS_DocsisUsModType_eQam64,
    NEXUS_DocsisUsModType_eQam128,
    NEXUS_DocsisUsModType_eQam256,
    NEXUS_DocsisUsModType_eQam512,
    NEXUS_DocsisUsModType_eOther
} NEXUS_DocsisUsModType;

 /***************************************************************************
Summary:
   Docsis Status
 ***************************************************************************/
typedef enum NEXUS_DocsisStatus
{
    NEXUS_DocsisStatus_eUninitialized,
    NEXUS_DocsisStatus_eOperational,
    NEXUS_DocsisStatus_eReset,
    NEXUS_DocsisStatus_eFailed,
    NEXUS_DocsisStatus_ePowerSave,
    NEXUS_DocsisStatus_eMax
} NEXUS_DocsisStatus;

 /***************************************************************************
Summary:
   325x Docsis Status
 ***************************************************************************/
typedef struct NEXUS_FrontendDocsisStatus
{
    NEXUS_DocsisStatus status;               /* Docsis current status */
    int downstreamCenterFreq;                /* Docsis channel DS frequency in MHz */
    int downstreamPowerLevel;                /* Docsis channel DS power in tenths of dBmV*/
    bool downstreamCarrierLock;              /* Docsis channel DS lock status */
    NEXUS_DocsisScdmaType channelScdmaStatus;/* Docsis channel DS S-CDMA status */
    NEXUS_DocsisUsModType upstreamModType;   /* US modulation type */
    int upstreamXmtCenterFreq;               /* US tramsmition frequency in Mhz*/
    int upstreamPowerLevel;                  /* US power in dBmV */
    int upStreamSymbolrate;                  /* US Symbol rate */
    char macAddress[6];                      /* eCM Mac address */
    char ipv4Address[4];                     /* eCM IPv4 address */
    char ipv6Address[16];                    /* eCM IPv6 address */
} NEXUS_FrontendDocsisStatus;


/***************************************************************************
Summary:
    Get the default settings for a BCM3255 tuner

Description:

See Also:
    NEXUS_Frontend_Open3255
 ***************************************************************************/
void NEXUS_Frontend_GetDefault3255Settings(
    NEXUS_3255Settings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
    Open a handle to a BCM3255 device.

Description:

See Also:
    NEXUS_Frontend_Close3255
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open3255( /* attr{destructor=NEXUS_Frontend_Close} */
    const NEXUS_3255Settings *pSettings
    );

/***************************************************************************
Summary:
    Get the lock status of a 3255 QAM tuner
***************************************************************************/
NEXUS_Error NEXUS_Frontend_3255_GetQamLockStatus(
    NEXUS_FrontendHandle handle,
    bool *pLocked /* [out] */
    );

/***************************************************************************
Summary:
    Get the lock status of a 3255 OOB tuner
***************************************************************************/
NEXUS_Error NEXUS_Frontend_3255_GetOutOfBandLockStatus(
    NEXUS_FrontendHandle handle,
    bool *pLocked /* [out] */
    );

/***************************************************************************
Summary:
    Get the current settings of 325x GPIO pins
***************************************************************************/
NEXUS_Error NEXUS_Frontend_3255_GetGpioPinSettings(
    NEXUS_FrontendHandle handle,
    NEXUS_3255GpioPin pin,
    NEXUS_3255GpioPinSettings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
    Set the current settings of 325x GPIO pins
***************************************************************************/
NEXUS_Error NEXUS_Frontend_3255_SetGpioPinSettings(
    NEXUS_FrontendHandle handle,
    NEXUS_3255GpioPin pin,
    const NEXUS_3255GpioPinSettings *pSettings
    );

/***************************************************************************
Summary:
    Request  the current Inband full status including DS power level
***************************************************************************/
NEXUS_Error NEXUS_Frontend_3255_RequestAsyncQamStatus(
    NEXUS_FrontendHandle frontHandle, 
    const NEXUS_CallbackDesc *cb
    );

/***************************************************************************
Summary:
    Retrieve  the current Inband full status including DS power level
***************************************************************************/
NEXUS_Error NEXUS_Frontend_3255_GetAsyncQamStatus(
    NEXUS_FrontendHandle frontHandle, 
    NEXUS_FrontendQamStatus *pStatus    /* [out] */
    );


/***************************************************************************
Summary:
    Get the current status of Docsis (eCM)
***************************************************************************/
NEXUS_Error NEXUS_Frontend_3255_GetDocsisStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDocsisStatus *pStatus    /* [out] */
    );

/***************************************************************************
Summary:
    Get the current settings for a BCM3255 tuner

Description:
See Also:
    NEXUS_Frontend_3255_SetSettings
 ***************************************************************************/
void NEXUS_Frontend_3255_GetSettings(
    NEXUS_FrontendHandle handle,
    NEXUS_3255Settings *pSettings    /* [out] */
    );

/***************************************************************************
Summary:
    Set the settings for a BCM3255 tuner

Description:
See Also:
    NEXUS_Frontend_3255_GetSettings
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_3255_SetSettings(
    NEXUS_FrontendHandle handle,
    const NEXUS_3255Settings *pSettings
    );

NEXUS_Error NEXUS_Frontend_3255_PowerDownFrontendDevices(
    NEXUS_FrontendHandle handle
    );

NEXUS_Error NEXUS_Frontend_3255_PowerUpFrontendDevices(
    NEXUS_FrontendHandle handle
    );

NEXUS_Error NEXUS_Frontend_3255_TransferFrontendControlToHost(
    NEXUS_FrontendHandle handle
    );

NEXUS_Error NEXUS_Frontend_3255_TransferFrontendControlToCM(
    NEXUS_FrontendHandle handle
    );

NEXUS_Error NEXUS_Frontend_3255_IsCMOperational(
    NEXUS_FrontendHandle handle,
    bool * pStatus
    );

NEXUS_Error NEXUS_Frontend_3255_EnableRPC(
    NEXUS_FrontendHandle handle,
    bool toEnable
    );

NEXUS_Error NEXUS_Frontend_3255_EnableHeartBeat(
    NEXUS_FrontendHandle handle,
    bool toEnable
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_3255_H__ */


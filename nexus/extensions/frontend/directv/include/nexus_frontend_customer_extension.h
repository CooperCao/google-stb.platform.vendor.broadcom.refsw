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
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef NEXUS_FRONTEND_FTM_H__
#define NEXUS_FRONTEND_FTM_H__

#include "nexus_frontend.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
FTM settings
**/
typedef struct NEXUS_FrontendFtmSettings
{
    bool enabled; /* You must set enabled = true before using the FTM API. */
    NEXUS_CallbackDesc dataReady; /* Callback fired when data is available using NEXUS_Frontend_DirecTV_ReceiveFTMMessage. */
    uint8_t ftmTxPower; /* Allow modification of the FTM TX Carrier power */
} NEXUS_FrontendFtmSettings;

/**
Summary:
Get FTM settings
**/
void NEXUS_Frontend_DirecTV_GetFTMSettings(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendFtmSettings *pSettings /* [out] */
    );

/**
Summary:
Set FTM settings
**/
NEXUS_Error NEXUS_Frontend_DirecTV_SetFTMSettings(
    NEXUS_FrontendHandle handle,
    const NEXUS_FrontendFtmSettings *pSettings
    );

/**
Summary:
Send an FTM message
**/
NEXUS_Error NEXUS_Frontend_DirecTV_SendFTMMessage(
    NEXUS_FrontendHandle handle,
    const uint8_t *pSendCommand, /* attr{nelem=sendCommandSize} */
    size_t sendCommmandSize
    );

/**
Summary:
Received a FTM message
**/
NEXUS_Error NEXUS_Frontend_DirecTV_ReceiveFTMMessage(
    NEXUS_FrontendHandle handle,
    uint8_t *pReceiveCommand, /* [out] attr{nelem=receiveCommmandSize} */
    size_t receiveCommmandSize,
    size_t *pDataReceived /* [out] */
    );

/**
Summary:
Reset FTM
**/
NEXUS_Error NEXUS_Frontend_DirecTV_ResetFTM(
    NEXUS_FrontendHandle handle
    );

/**
Summary:
**/
NEXUS_Error NEXUS_Frontend_DirecTV_SetAmcScramblingSequence(
    NEXUS_FrontendHandle frontend,
    uint32_t xseed,       /* [in] physical layer scrambler seed */
    uint32_t plhdrscr1,   /* [in] PL Header Scrambling Sequence Bit[31:0] */
    uint32_t plhdrscr2,   /* [in] PL Header Scrambling Sequence Bit[63:32] */
    uint32_t plhdrscr3    /* [in] PL Header Scrambling Sequence Bit[89:64] */
    );

/**
Summary:
**/
typedef enum NEXUS_DirecTV_FskChannel
{
    NEXUS_DirecTV_FskChannel_eCh0 = 0,
    NEXUS_DirecTV_FskChannel_eCh1,
    NEXUS_DirecTV_FskChannel_eMax
} NEXUS_DirecTV_FskChannel;

/**
Summary:
**/
NEXUS_Error NEXUS_Frontend_DirecTV_SetFskChannel(
    NEXUS_FrontendHandle frontend,
    NEXUS_DirecTV_FskChannel fskTxChannel,
    NEXUS_DirecTV_FskChannel fskRxChannel
    );
    
/**
Summary:
Allow DirecTV-specific frontend configuration settings to be applied. 
NOTE: These settings are not meant to be generic across all DirecTV platform
configurations. The app code what and when it's appropriate to apply these changes.
**/
typedef struct NEXUS_Frontend_DirecTV_FrontendSettings
{
    /* These settings are also in nexus_frontend_7346.h and are applied in the 
       function NEXUS_Frontend_Set7346TuneSettings(). These settings are really 
       specific to DirecTV and probably shouldn't be in the generic mainline.
    */
    bool bypassLnaGain; /* Bypass the internal LNA Gain stage */
    struct  {
        bool override;  /* Override the value of Daisy Gain */
        unsigned value; /* Daisy Gain value to be used */
    } daisyGain; 

    /* BB and LNA AGC settings */
    unsigned bbClipThreshold;
    unsigned lnaClipThreshold;
    unsigned bbWindow;
    unsigned lnaWindow;
    unsigned bbAmpThreshold;
    unsigned lnaAmpThreshold;
    unsigned bbLoopCoeff;
    unsigned lnaLoopCoeff;
} NEXUS_Frontend_DirecTV_FrontendSettings;

/**
Summary:
Get the default values for our Directv-specific frontend configuration settings
**/
void
NEXUS_Frontend_DirecTV_GetDefaultSettings(
    NEXUS_FrontendHandle frontend, 
    NEXUS_Frontend_DirecTV_FrontendSettings * pSettings
    );

/**
Summary:
Set our Directv-specific frontend configuration settings
**/
NEXUS_Error 
NEXUS_Frontend_DirecTV_SetSettings(
    NEXUS_FrontendHandle frontend, 
    const NEXUS_Frontend_DirecTV_FrontendSettings * pSettings
    );


#ifdef __cplusplus
}
#endif

#endif

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
*   API name: Frontend Card
*    APIs to detect, open, and close removable frontend cards.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_FRONTEND_CARD_H__
#define NEXUS_FRONTEND_CARD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nexus_frontend.h"
#include "nexus_i2c.h"
#if NEXUS_HAS_GPIO
#include "nexus_gpio.h"
#endif

/***************************************************************************
Summary:
    Handle for a frontend card
 ***************************************************************************/
typedef struct NEXUS_FrontendCard *NEXUS_FrontendCardHandle;

/***************************************************************************
Summary:
    Settings for a frontend card
 ***************************************************************************/
typedef struct NEXUS_FrontendCardSettings
{
    NEXUS_I2cHandle i2cDevice;      /* I2C device to use */
    unsigned isrNumber;             /* L1 interrupt number (typically 0..63)*/
    unsigned numChannels;           /* Number of downstream channels supported */
    unsigned numOutOfBandChannels;  /* Number of out of band channels supported (1 or 2 typically) */
    unsigned numUpstreamChannels;   /* Number of upstream channels supported (1 or 2 typically) */
    bool bitWideSync;
    NEXUS_GpioHandle gpioInterrupt; /* GPIO pin for interrupt. If not NULL, isrNumber is ignored. If NULL, isrNumber is used. */
} NEXUS_FrontendCardSettings;

/***************************************************************************
Summary:
	Get the default settings for a frontend card
 ***************************************************************************/
void NEXUS_FrontendCard_GetDefaultSettings(
    NEXUS_FrontendCardSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
    Open a frontend card.

Description:
    This routine will probe a frontend card slot for all known card types.
    If a recognized card is found, a valid handle will be returned.
    Otherwise, NULL handle will be returned.

See Also:
    NEXUS_FrontendCard_Close
 ***************************************************************************/
NEXUS_FrontendCardHandle NEXUS_FrontendCard_Open(   /* attr{destructor=NEXUS_FrontendCard_Close} */
    const NEXUS_FrontendCardSettings *pSettings
    );

/***************************************************************************
Summary:
    Close a frontend card.
 ***************************************************************************/
void NEXUS_FrontendCard_Close(
    NEXUS_FrontendCardHandle handle
    );

/***************************************************************************
Summary:
    Get the number of channels present on this frontend card

Description:
    Frontend cards may have multiple channels.  This API allows the caller
    to determine the number of channels available.

See Also:
    NEXUS_FrontendCard_GetChannel
 ***************************************************************************/
NEXUS_Error NEXUS_FrontendCard_GetNumChannels(
    NEXUS_FrontendCardHandle handle,
    unsigned *pNumChannels
    );

/***************************************************************************
Summary:
    Get a frontend channel handle for this card

Description:
    Frontend cards may have multiple channels.  This API allows the caller
    to retrieve each channel using an index.

See Also:
    NEXUS_FrontendCard_GetNumChannels
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_FrontendCard_GetChannel(
    NEXUS_FrontendCardHandle handle,
    unsigned index
    );

/***************************************************************************
Summary:
    Get the number of out of band channels present on this frontend card

Description:
    Frontend cards may have multiple out of band channels.  This API allows the caller
    to determine the number of out of band channels available.

See Also:
    NEXUS_FrontendCard_GetChannel
 ***************************************************************************/
NEXUS_Error NEXUS_FrontendCard_GetNumOutOfBandChannels(
    NEXUS_FrontendCardHandle handle,
    unsigned *pNumOutOfBandChannels
    );

/***************************************************************************
Summary:
    Get a frontend out of band channel handle for this card

Description:
    Frontend cards may have multiple  out of band channels.  This API allows the caller
    to retrieve each out of band  channel using an index.

See Also:
    NEXUS_FrontendCard_GetNumChannels
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_FrontendCard_GetOutOfBandChannel(
    NEXUS_FrontendCardHandle handle,
    unsigned index
    );

/***************************************************************************
Summary:
    Get the number of upstream channels present on this frontend card

Description:
    Frontend cards may have upstream multiple channels.  This API allows the caller
    to determine the number of upstream channels available.

See Also:
    NEXUS_FrontendCard_GetChannel
 ***************************************************************************/
NEXUS_Error NEXUS_FrontendCard_GetNumUpstreamChannels(
    NEXUS_FrontendCardHandle handle,
    unsigned *pNumUpstreamChannels
    );

/***************************************************************************
Summary:
    Get a frontend upstream channel handle for this card

Description:
    Frontend cards may have multiple upstream channels.  This API allows the caller
    to retrieve each upstream channel using an index.

See Also:
    NEXUS_FrontendCard_GetNumChannels
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_FrontendCard_GetUpstreamChannel(
    NEXUS_FrontendCardHandle handle,
    unsigned index
    );

#ifdef __cplusplus
}
#endif

#endif


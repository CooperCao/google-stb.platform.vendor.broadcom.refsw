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
*   API name: Frontend Extension
*    Private APIs to allow frontend extension hooks
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "nexus_frontend.h"

#ifndef NEXUS_FRONTEND_EXTENSION_API_H__
#define NEXUS_FRONTEND_EXTENSION_API_H__

/***************************************************************************
*
* Frontend/Demodulator Extension Interfaces
*
****************************************************************************/

/***************************************************************************
Summary:
    Frontend extension settings
****************************************************************************/
typedef struct NEXUS_FrontendExtensionSettings
{
    NEXUS_FrontendHandle parent; /* The nexus frontend handle that you are extending.  */
    void *pDeviceHandle;         /* This value will be passed as the first argument to each callback function.
                                    It's used to store any internal state required by this extension */

    /*
       Override functions.  Close is required, all others are optional.  If a
       function is not specified, the parent's routine will be called for that routine instead.
    */
    void        (*close)(NEXUS_FrontendHandle handle);
    NEXUS_Error (*tuneAnalog)(void *handle, const NEXUS_FrontendAnalogSettings *pSettings);
    NEXUS_Error (*tuneOutOfBand)(void *handle, const NEXUS_FrontendOutOfBandSettings *pSettings);
    NEXUS_Error (*tuneQam)(void *handle, const NEXUS_FrontendQamSettings *pSettings);
    NEXUS_Error (*tuneUpstream)(void *handle, const NEXUS_FrontendUpstreamSettings *pSettings);
    NEXUS_Error (*tuneVsb)(void *handle, const NEXUS_FrontendVsbSettings *pSettings);
    NEXUS_Error (*tuneSatellite)(void *handle, const NEXUS_FrontendSatelliteSettings *pSettings);
    void        (*getDiseqcSettings)(void *handle, NEXUS_FrontendDiseqcSettings *pSettings);
    NEXUS_Error (*setDiseqcSettings)(void *handle, const NEXUS_FrontendDiseqcSettings *pSettings);
    NEXUS_Error (*sendDiseqcMessage)(void *handle, const uint8_t *pSendData, size_t sendDataSize, const NEXUS_CallbackDesc *pSendComplete);
    NEXUS_Error (*getDiseqcReply)(void *handle, NEXUS_FrontendDiseqcMessageStatus *pStatus, uint8_t *pReplyBuffer, size_t replyBufferSize, size_t *pReplyLength);
    NEXUS_Error (*sendDiseqcAcw)(void *handle, uint8_t codeWord);
    NEXUS_Error (*resetDiseqc)(void *handle, uint8_t options);
    NEXUS_Error (*tuneOfdm)(void *handle, const NEXUS_FrontendOfdmSettings *pSettings);
    void        (*untune)(void *handle);
} NEXUS_FrontendExtensionSettings;

/***************************************************************************
Summary:
    Get default settings for a frontend extension
****************************************************************************/
void NEXUS_Frontend_P_GetDefaultExtensionSettings(
    NEXUS_FrontendExtensionSettings *pSettings          /* [out] */
    );

/***************************************************************************
Summary:
    Create an extended frontend interface

Description:
    The nexus frontend interface is a high-level interface that controls
    tuning, demodulator, and even other devices such as LNA and DiSEqC
    controllers.  Because of the large number of board dependencies in
    these interfaces, we have a standard method of installing nexus
    extension functions for a frontend interface.  Using this technique,
    a platform developer can override certain functions to call platform-
    specific code and then call common code afterward.  For example, if your
    platform has a tuner device not supported in nexus, you can create a
    frontend extension that will first tune your tuner and then call the
    nexus demodulator API.

    Please see the nexus development guide for more details on how to use
    this interface.
****************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_P_CreateExtension(
    const NEXUS_FrontendExtensionSettings *pSettings    /* Functions to override.  The close function is required, all other functions are optional.
                                                           If you do not want to extend a function, leave it in the default state and the parent's
                                                           handler will be called instead. */
    );

/***************************************************************************
Summary:
    Destroy a frontend handle created by another module.

Description:
    Destroy a frontend handle created by another module.  This should be
    called by your extension's close handler routine.
****************************************************************************/
void NEXUS_Frontend_P_DestroyExtension(
    NEXUS_FrontendHandle handle
    );



/***************************************************************************
*
* Tuner Extension Interfaces
*
****************************************************************************/


/***************************************************************************
Summary:
    Tuner extension settings
****************************************************************************/
typedef struct NEXUS_TunerExtensionSettings
{
    void *pDeviceHandle;        /* This value will be passed as the first argument to each callback function.
                                   It's used to store any internal state required by this extension */
    unsigned ifFrequency;       /* (Hz) This is read by demodulators to determine the expected IF frequency of the tuner */

    /* Tuner routines.  All are required. */
    void        (*close)(void *handle);
    NEXUS_Error (*setFrequency)(void *handle, NEXUS_TunerMode mode, unsigned frequency);
} NEXUS_TunerExtensionSettings;

/***************************************************************************
Summary:
    Get default settings for a tuner extension
****************************************************************************/
void NEXUS_Tuner_P_GetDefaultExtensionSettings(
    NEXUS_TunerExtensionSettings *pSettings          /* [out] */
    );

/***************************************************************************
Summary:
    Create a nexus tuner handle from an extension

Description:
    This interface allows a custom platform to easily integrate tuner
    functions for a tuner into the nexus framework.  Using this, the standard
    nexus frontend routines can control both your tuner and demodulator if
    desired.
****************************************************************************/
NEXUS_TunerHandle NEXUS_Tuner_P_CreateExtension(
    const NEXUS_TunerExtensionSettings *pSettings
    );

#endif /* #ifndef NEXUS_FRONTEND_EXTENSION_API_H__ */


/***************************************************************************
 *     (c)2006-2013 Broadcom Corporation
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
 * Module Description: Audio Decoder Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef NEXUS_ECHO_CANCELLER_H_
#define NEXUS_ECHO_CANCELLER_H_

#include "nexus_types.h"

/***************************************************************************
Summary:
Echo Canceller Handle
***************************************************************************/
typedef struct NEXUS_EchoCanceller *NEXUS_EchoCancellerHandle;

/***************************************************************************
Summary:
Settings for an echo canceller
***************************************************************************/
typedef struct NEXUS_EchoCancellerSettings
{
    NEXUS_EchoCancellerAlgorithm algorithm;  /* What echo canceller algorithm to use */
} NEXUS_EchoCancellerSettings;

/***************************************************************************
Summary:
Get default settings for an echo canceller
***************************************************************************/
void NEXUS_EchoCanceller_GetDefaultSettings(
    NEXUS_EchoCancellerSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
    Create an echo canceller
***************************************************************************/
NEXUS_EchoCancellerHandle NEXUS_EchoCanceller_Create( /* attr{destructor=NEXUS_EchoCanceller_Destroy}  */
    const NEXUS_EchoCancellerSettings *pSettings      /* attr{null_allowed=y} */
    );

#define NEXUS_EchoCanceller_Open NEXUS_EchoCanceller_Create

/***************************************************************************
Summary:
    Destroy an echo canceller
***************************************************************************/
void NEXUS_EchoCanceller_Destroy(
    NEXUS_EchoCancellerHandle handle
    );

#define NEXUS_EchoCanceller_Close NEXUS_EchoCanceller_Destroy

/***************************************************************************
Summary:
    Get Settings for an echo canceller
***************************************************************************/
void NEXUS_EchoCanceller_GetSettings(
    NEXUS_EchoCancellerHandle handle,
    NEXUS_EchoCancellerSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
    Set Settings for an echo canceller
***************************************************************************/
NEXUS_Error NEXUS_EchoCanceller_SetSettings(
    NEXUS_EchoCancellerHandle handle,
    const NEXUS_EchoCancellerSettings *pSettings
    );

/***************************************************************************
Summary:
Speex Echo Canceller Gain Modes
***************************************************************************/
typedef enum NEXUS_SpeexEchoCancellerGainMode
{
    NEXUS_SpeexEchoCancellerGainMode_eBark,      /* Bark mode (default).  Provides faster response. */
    NEXUS_SpeexEchoCancellerGainMode_eLinear,    /* Linear mode (optional).  Provides higher quality but slower. */
    NEXUS_SpeexEchoCancellerGainMode_eMax
} NEXUS_SpeexEchoCancellerGainMode;

/***************************************************************************
Summary:
Speex Echo Canceller Settings
***************************************************************************/
typedef struct NEXUS_SpeexEchoCancellerSettings
{
    NEXUS_SpeexEchoCancellerGainMode gainMode;
} NEXUS_SpeexEchoCancellerSettings;

/***************************************************************************
Summary:
Echo Canceller Algorithm Settings
***************************************************************************/
typedef struct NEXUS_EchoCancellerAlgorithmSettings
{
    NEXUS_EchoCancellerAlgorithm algorithm;

    union
    {
        NEXUS_SpeexEchoCancellerSettings speex;  /* Settings for Speex Echo Canceller */
    } algorithmSettings;
} NEXUS_EchoCancellerAlgorithmSettings;

/***************************************************************************
Summary:
Get Echo Canceller Algorithm Settings
***************************************************************************/
void NEXUS_EchoCanceller_GetAlgorithmSettings(
    NEXUS_EchoCancellerHandle handle,
    NEXUS_EchoCancellerAlgorithm algorithm,
    NEXUS_EchoCancellerAlgorithmSettings *pSettings  /* [out] */
    );

/***************************************************************************
Summary:
Set Echo Canceller Algorithm Settings
***************************************************************************/
NEXUS_Error NEXUS_EchoCanceller_SetAlgorithmSettings(
    NEXUS_EchoCancellerHandle handle,
    const NEXUS_EchoCancellerAlgorithmSettings *pSettings
    );

/***************************************************************************
Summary:
    Get the audio connector for an Audio EchoCanceller stage
***************************************************************************/
NEXUS_AudioInput NEXUS_EchoCanceller_GetConnector(
    NEXUS_EchoCancellerHandle handle
    );

/***************************************************************************
Summary:
Add a local (near-end) input to the echo canceller
***************************************************************************/
NEXUS_Error NEXUS_EchoCanceller_AddLocalInput(
    NEXUS_EchoCancellerHandle handle,
    NEXUS_AudioInput input
    );

/***************************************************************************
Summary:
Remove a local (near-end) input from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_EchoCanceller_RemoveLocalInput(
    NEXUS_EchoCancellerHandle handle,
    NEXUS_AudioInput input
    );

/***************************************************************************
Summary:
Add a remote (far-end) input to the echo canceller
***************************************************************************/
NEXUS_Error NEXUS_EchoCanceller_AddRemoteInput(
    NEXUS_EchoCancellerHandle handle,
    NEXUS_AudioInput input
    );

/***************************************************************************
Summary:
Remove a remote (far-end) input from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_EchoCanceller_RemoveRemoteInput(
    NEXUS_EchoCancellerHandle handle,
    NEXUS_AudioInput input
    );

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
NEXUS_Error NEXUS_EchoCanceller_RemoveAllInputs(
    NEXUS_EchoCancellerHandle handle
    );

#endif /* #ifndef NEXUS_ECHO_CANCELLER_H_ */


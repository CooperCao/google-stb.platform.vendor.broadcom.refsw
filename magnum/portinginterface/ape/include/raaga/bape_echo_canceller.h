/***************************************************************************
 *     Copyright (c) 2006-2012, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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
#ifndef BAPE_ECHO_CANCELLER_H_
#define BAPE_ECHO_CANCELLER_H_

#include "bape.h"

/***************************************************************************
Summary:
Echo Canceller Handle
***************************************************************************/
typedef struct BAPE_EchoCanceller *BAPE_EchoCancellerHandle;

/***************************************************************************
Summary:
Echo Cancellation Algorithms
***************************************************************************/
typedef enum BAPE_EchoCancellerAlgorithm
{
    BAPE_EchoCancellerAlgorithm_eSpeex,     /* Speex Algorithm (http://www.speex.org) */
    BAPE_EchoCancellerAlgorithm_eMax
} BAPE_EchoCancellerAlgorithm;

/***************************************************************************
Summary:
Settings for an echo canceller
***************************************************************************/
typedef struct BAPE_EchoCancellerSettings
{
    BAPE_EchoCancellerAlgorithm algorithm;  /* What echo canceller algorithm to use */
} BAPE_EchoCancellerSettings;

/***************************************************************************
Summary:
Get default settings for an echo canceller
***************************************************************************/
void BAPE_EchoCanceller_GetDefaultSettings(
    BAPE_EchoCancellerSettings *pSettings   /* [out] */
    );

/***************************************************************************
Summary:
    Create an echo canceller
***************************************************************************/
BERR_Code BAPE_EchoCanceller_Create(
    BAPE_Handle deviceHandle,
    const BAPE_EchoCancellerSettings *pSettings,
    BAPE_EchoCancellerHandle *pHandle               /* [out] */
    );

/***************************************************************************
Summary:
    Destroy an echo canceller
***************************************************************************/
void BAPE_EchoCanceller_Destroy(
    BAPE_EchoCancellerHandle handle
    );

/***************************************************************************
Summary:
    Get Settings for an echo canceller
***************************************************************************/
void BAPE_EchoCanceller_GetSettings(
    BAPE_EchoCancellerHandle handle,
    BAPE_EchoCancellerSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
    Set Settings for an echo canceller
***************************************************************************/
BERR_Code BAPE_EchoCanceller_SetSettings(
    BAPE_EchoCancellerHandle handle,
    const BAPE_EchoCancellerSettings *pSettings
    );

/***************************************************************************
Summary:
Speex Echo Canceller Gain Modes
***************************************************************************/
typedef enum BAPE_SpeexEchoCancellerGainMode
{
    BAPE_SpeexEchoCancellerGainMode_eBark,      /* Bark mode (default).  Provides faster response. */
    BAPE_SpeexEchoCancellerGainMode_eLinear,    /* Linear mode (optional).  Provides higher quality but slower. */
    BAPE_SpeexEchoCancellerGainMode_eMax
} BAPE_SpeexEchoCancellerGainMode;

/***************************************************************************
Summary:
Speex Echo Canceller Settings
***************************************************************************/
typedef struct BAPE_SpeexEchoCancellerSettings
{
    BAPE_SpeexEchoCancellerGainMode gainMode;
} BAPE_SpeexEchoCancellerSettings;

/***************************************************************************
Summary:
Echo Canceller Algorithm Settings
***************************************************************************/
typedef struct BAPE_EchoCancellerAlgorithmSettings
{
    BAPE_EchoCancellerAlgorithm algorithm;

    union
    {
        BAPE_SpeexEchoCancellerSettings speex;  /* Settings for Speex Echo Canceller */
    } algorithmSettings;
} BAPE_EchoCancellerAlgorithmSettings;

/***************************************************************************
Summary:
Get Echo Canceller Algorithm Settings
***************************************************************************/
void BAPE_EchoCanceller_GetAlgorithmSettings(
    BAPE_EchoCancellerHandle handle,
    BAPE_EchoCancellerAlgorithm algorithm,
    BAPE_EchoCancellerAlgorithmSettings *pSettings  /* [out] */
    );

/***************************************************************************
Summary:
Set Echo Canceller Algorithm Settings
***************************************************************************/
BERR_Code BAPE_EchoCanceller_SetAlgorithmSettings(
    BAPE_EchoCancellerHandle handle,
    const BAPE_EchoCancellerAlgorithmSettings *pSettings
    );

/***************************************************************************
Summary:
    Get the audio connector for an Audio EchoCanceller stage
***************************************************************************/
void BAPE_EchoCanceller_GetConnector(
    BAPE_EchoCancellerHandle handle,
    BAPE_Connector *pConnector   /* [out] */
    );

/***************************************************************************
Summary:
Add a local (near-end) input to the echo canceller
***************************************************************************/
BERR_Code BAPE_EchoCanceller_AddLocalInput(
    BAPE_EchoCancellerHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove a local (near-end) input from this processing stage
***************************************************************************/
BERR_Code BAPE_EchoCanceller_RemoveLocalInput(
    BAPE_EchoCancellerHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Add a remote (far-end) input to the echo canceller
***************************************************************************/
BERR_Code BAPE_EchoCanceller_AddRemoteInput(
    BAPE_EchoCancellerHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove a remote (far-end) input from this processing stage
***************************************************************************/
BERR_Code BAPE_EchoCanceller_RemoveRemoteInput(
    BAPE_EchoCancellerHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
BERR_Code BAPE_EchoCanceller_RemoveAllInputs(
    BAPE_EchoCancellerHandle handle
    );

#endif /* #ifndef BAPE_ECHO_CANCELLER_H_ */

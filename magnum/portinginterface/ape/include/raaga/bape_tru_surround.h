/***************************************************************************
*     (c)2004-2010 Broadcom Corporation
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
*   API name: TruSurround
*    Specific APIs related to SRS TruSurround Audio Processing
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef BAPE_TRU_SURROUND_H__
#define BAPE_TRU_SURROUND_H__

#include "bape.h"

/***************************************************************************
Summary: 
TruSurroundHd Handle 
***************************************************************************/
typedef struct BAPE_TruSurroundHd *BAPE_TruSurroundHdHandle;

/***************************************************************************
Summary:
This enumeration specifies the size of the speakers that the TruBass sound will be
played back on. TruBass adjusts its filter parameters to optimize playback for that size.
Size should be selected based on the lowest reproducible frequency that is just above
the specified value
***************************************************************************/
typedef enum BAPE_TruBassSpeakerSize
{
    BAPE_TruBassSpeakerSize_e40Hz,
    BAPE_TruBassSpeakerSize_e60Hz,
    BAPE_TruBassSpeakerSize_e100Hz,
    BAPE_TruBassSpeakerSize_e150Hz,
    BAPE_TruBassSpeakerSize_e200Hz,
    BAPE_TruBassSpeakerSize_e250Hz,
    BAPE_TruBassSpeakerSize_e300Hz,
    BAPE_TruBassSpeakerSize_e400Hz,
    BAPE_TruBassSpeakerSize_eMax
} BAPE_TruBassSpeakerSize;

/***************************************************************************
Summary:
TruBass Processing Mode
***************************************************************************/
typedef enum BAPE_TruBassMode
{
    BAPE_TruBassMode_eMono,
    BAPE_TruBassMode_eStereo,
    BAPE_TruBassMode_eMax
} BAPE_TruBassMode;

/***************************************************************************
Summary:
TruSurround HD Crossover Frequency
***************************************************************************/
typedef enum BAPE_TruSurroundHdCrossover
{
    BAPE_TruSurroundHdCrossover_e80Hz,
    BAPE_TruSurroundHdCrossover_e120Hz,
    BAPE_TruSurroundHdCrossover_e160Hz,
    BAPE_TruSurroundHdCrossover_e200Hz,
    BAPE_TruSurroundHdCrossover_eMax
} BAPE_TruSurroundHdCrossover;

/***************************************************************************
Summary:
SRS WOW HD 3D Mode
***************************************************************************/
typedef enum BAPE_WowHdMode
{
    BAPE_WowHdMode_eMono,
    BAPE_WowHdMode_eSingleSpeaker,
    BAPE_WowHdMode_eStereo,
    BAPE_WowHdMode_eExtreme,
    BAPE_WowHdMode_eMax
} BAPE_WowHdMode;

/***************************************************************************
Summary:
SRS ACMOD values
***************************************************************************/
typedef enum BAPE_TruSurroundHdAcmod
{
    BAPE_TruSurroundHdAcmod_e1_0_C,          /* C */
    BAPE_TruSurroundHdAcmod_e2_0_LR,         /* L,R */
    BAPE_TruSurroundHdAcmod_e3_0_LCR,        /* L,C,R */
    BAPE_TruSurroundHdAcmod_e2_1_LRS,        /* L,R,S */
    BAPE_TruSurroundHdAcmod_e3_1_LCRS,       /* L,C,R,S */
    BAPE_TruSurroundHdAcmod_e2_2_LRLsRs,     /* L,R,LS,RS */
    BAPE_TruSurroundHdAcmod_e3_2_LCRLsRs,    /* L,C,R,LS,RS */
    BAPE_TruSurroundHdAcmod_e3_3_LCRLsRsCs,  /* L,C,R,LS,RS,CS */
    BAPE_TruSurroundHdAcmod_e3_2_BSDigital,  /* L,C,R,LS,RS */
    BAPE_TruSurroundHdAcmod_eLtRt,           /* L,R */
    BAPE_TruSurroundHdAcmod_ePLII_Movie,     /* L,C,R,LS,RS */
    BAPE_TruSurroundHdAcmod_ePLII_Music,     /* L,C,R,LS,RS */
    BAPE_TruSurroundHdAcmod_ePassiveMatrix,  /* L,R */
    BAPE_TruSurroundHdAcmod_eCSII,           /* L,C,R,Ls,Rs,Cs */
    BAPE_TruSurroundHdAcmod_eMax
} BAPE_TruSurroundHdAcmod;

/***************************************************************************
Summary:
TruSurround HD Settings
***************************************************************************/
typedef struct BAPE_TruSurroundHdSettings
{
    bool enabled;                       /* If true, processing is enabled.  Otherwise this stage is bypassed */

    bool headphoneEnabled;              /* Turns the headphone processing of the TruSurround process on and off. */
    bool monoToStereoEnabled;           /* Enables Mono to Stereo Conversion */

    bool dialogClarityEnabled;          /* Enables the Dialog Clarity portion of the TruSurroundHD processing. */
    unsigned dialogClarityLevel;        /* Dialog clarity level (in %).  Range is 0 to 100%. */

    bool truBassEnabled;                /* Enables the TruBass portion of the processing. */
    unsigned truBassLevel;              /* TruBass Level - Specifies the amount of TruBass that is mixed into the final signal (in %). */
    BAPE_TruBassSpeakerSize speakerSize;   /* Specifies the size of the speakers that the sound will be played back on. */
    BAPE_TruBassMode truBassMode;      /* TruBass Mode.  Currently Mono or Stereo. */

    unsigned inputGain;                 /* Input gain. The valid range is 0 to 100% */
    unsigned outputGain;                /* Output gain.  The valid range is 0 to 100% */
    unsigned bypassGain;                /* Bypass gain.  The valid range is 0 to 100% */

    bool definitionEnabled;             /* Enables the definition portion of TruSurround HD */
    unsigned definitionLevel;           /* The definition level (in %). The valid range is 0 to 100% */

    unsigned surroundLevel;             /* Surround level (in %).  The valid range is 0 to 100% */

    bool wowHdEnabled;                  /* Enables SRS WOW HD 3D processing. */
    bool wowHdHighBitRateEnabled;       /* Enables High Bit Rate for SRS WOW HD. */
    bool wowHdFocusEnabled;             /* Enables the WOW HD Focus Processing */
    BAPE_WowHdMode wowHdMode;          /* Processing Mode for SRS WOW HD */
    unsigned wowHdSpaceLevel;           /* SRS WOW HD 3D Space Level (in %).  The valid range is 0 to 100% */
    unsigned wowHdCenterLevel;          /* SRS WOW HD 3D Center Level (in %).  The valid range is 0 to 100% */
    unsigned wowHdFocusLevel;           /* SRS WOW HD 3D Focus Level (in %).  The valid range is 0 to 100% */

    bool certificationApp;              /* True if Certification app. Default: false */                        
    bool inputLfeEnabled;               /* True if input LFE is present. Default: false */
    BAPE_TruSurroundHdAcmod inputAcmod; /* Default: BAPE_TruSurroundHdAcmod_e2_0_LR, i.e. Stereo */                               
} BAPE_TruSurroundHdSettings;

/***************************************************************************
Summary:
    Get default settings for an SRS TruSurround HD stage
***************************************************************************/
void BAPE_TruSurroundHd_GetDefaultSettings(
    BAPE_TruSurroundHdSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
    Open an SRS TruSurround HD stage
***************************************************************************/
BERR_Code BAPE_TruSurroundHd_Create(
    BAPE_Handle deviceHandle,
    const BAPE_TruSurroundHdSettings *pSettings,
    BAPE_TruSurroundHdHandle *pHandle               /* [out] */
    );

/***************************************************************************
Summary:
    Close an SRS TruSurround HD stage
    
Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
void BAPE_TruSurroundHd_Destroy(
    BAPE_TruSurroundHdHandle handle
    );

/***************************************************************************
Summary:
    Get Settings for an SRS TruSurround HD stage
***************************************************************************/
void BAPE_TruSurroundHd_GetSettings(
    BAPE_TruSurroundHdHandle handle,
    BAPE_TruSurroundHdSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
    Set Settings for an SRS TruSurround HD stage
***************************************************************************/
BERR_Code BAPE_TruSurroundHd_SetSettings(
    BAPE_TruSurroundHdHandle handle,
    const BAPE_TruSurroundHdSettings *pSettings
    );

/***************************************************************************
Summary:
    Get the audio connector for an SRS TruSurround HD stage
***************************************************************************/
void BAPE_TruSurroundHd_GetConnector(
    BAPE_TruSurroundHdHandle handle,
    BAPE_Connector *pConnector
    );

/***************************************************************************
Summary:
Add an input to this processing stage
***************************************************************************/
BERR_Code BAPE_TruSurroundHd_AddInput(
    BAPE_TruSurroundHdHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
BERR_Code BAPE_TruSurroundHd_RemoveInput(
    BAPE_TruSurroundHdHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
BERR_Code BAPE_TruSurroundHd_RemoveAllInputs(
    BAPE_TruSurroundHdHandle handle
    );

#endif /* #ifndef BAPE_TRU_SURROUND_H__ */


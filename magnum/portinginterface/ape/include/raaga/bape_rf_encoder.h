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
*   API name: RfEncoder
*    Specific APIs related to RF Audio Encoding
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef BAPE_RF_ENCODER_H__
#define BAPE_RF_ENCODER_H__

#include "bape.h"

/***************************************************************************
Summary:
Audio Encoder Settings
***************************************************************************/
typedef struct BAPE_RfEncoder *BAPE_RfEncoderHandle;

/***************************************************************************
Summary:
Audio Encoder Settings
***************************************************************************/
typedef struct BAPE_RfEncoderSettings
{
    BAPE_RfAudioEncoding encoding; /* Encoding to generate.  Cannot be changed while running. */
} BAPE_RfEncoderSettings;

/***************************************************************************
Summary:
    Get default settings for an RF Audio Encoder
***************************************************************************/
void BAPE_RfEncoder_GetDefaultSettings(
    BAPE_RfEncoderSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
    Open an RF Audio Encoder
***************************************************************************/
BERR_Code BAPE_RfEncoder_Create(
    BAPE_Handle deviceHandle,
    const BAPE_RfEncoderSettings *pSettings,
    BAPE_RfEncoderHandle *pHandle             /* [out] */
    );

/***************************************************************************
Summary:
    Close an RF Audio Encoder
    
Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
void BAPE_RfEncoder_Destroy(
    BAPE_RfEncoderHandle handle
    );

/***************************************************************************
Summary:
    Get Settings for an RF Audio Encoder
***************************************************************************/
void BAPE_RfEncoder_GetSettings(
    BAPE_RfEncoderHandle handle,
    BAPE_RfEncoderSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
    Set Settings for an RF Audio Encoder
***************************************************************************/
BERR_Code BAPE_RfEncoder_SetSettings(
    BAPE_RfEncoderHandle handle,
    const BAPE_RfEncoderSettings *pSettings
    );

/***************************************************************************
Summary:
BTSC RF Encoding Modes
***************************************************************************/
typedef enum BAPE_BtscEncodingMode
{
    BAPE_BtscEncodingMode_eStereo, /* Default will output L+R and L-R channels with the pilot tone enabled */
    BAPE_BtscEncodingMode_eMono,   /* Mono will output L+R only */
    BAPE_BtscEncodingMode_eMax
} BAPE_BtscEncodingMode;

/***************************************************************************
Summary:
BTSC Encoding Settings

Description:
The parameters below are used for calibration and tuning of the BTSC
encoder.
***************************************************************************/
typedef struct BAPE_BtscEncodeSettings
{
    bool useDeEmphasizedSourceSignals;  /* Default = true. */
    bool equivalentMode75us;            /* Default = false.  When enabled, L+R and L-R will go through preephasis and be rendered out. */
    bool fmDeviationControl;            /* Default = true.  Used to Control FM Deviation of Final Output to 50Khz */
    bool rightChannelMute;              /* Default = false.  Used to test stereo separation */
    
    int32_t volume;                     /* Default = BAPE_VOLUME_NORMAL.  Ranges from BAPE_VOLUME_NORMAL to BAPE_VOLUME_MIN. */
    uint32_t srdCalibrationFactor;      /* Stereo Reference Decoder Calibration Factor.  Default = 0x12BE76C9.  Specified in 4.28 format */

    BAPE_BtscEncodingMode encodingMode; /* Default = stereo. */

    struct
    {
        bool preemphasis;               /* Default = true. */
        bool fmDeviationControl;        /* Default = true. */
    } mainChannel;    /* Settings for the main (L+R) channel */
    
    struct
    {
        bool clipping;                  /* Default = true. */
        bool preemphasis;               /* Default = true. */
        bool fmDeviationControl;        /* Default = true. */
    } diffChannel;    /* Settings for the difference (L-R) channel */
       
    struct
    {
        bool enabled;                   /* If true, a test tone is enabled */
        bool frequency;                 /* Test tone frequency in Hz.  If 0, a sweep will be output */
    } testTone;
    
} BAPE_BtscEncodeSettings;
    
/***************************************************************************
Summary:
Encoding-Specific Settings for an audio encoder
***************************************************************************/
typedef struct BAPE_RfEncoderEncodingSettings
{
    BAPE_RfAudioEncoding encoding; /* this is used for the codecSettings lookup */
    union
    {
        BAPE_BtscEncodeSettings btsc;
    } encodingSettings;
} BAPE_RfEncoderEncodingSettings;

/***************************************************************************
Summary:
    Get Encoding-Specific Settings for an RF Audio Encoder
***************************************************************************/
void BAPE_RfEncoder_GetEncodingSettings(
    BAPE_RfEncoderHandle handle,
    BAPE_RfAudioEncoding encoding,               /* The encoding for which you are retrieving settings. */
    BAPE_RfEncoderEncodingSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
    Set Encoding-Specific Settings for an RF Audio Encoder
***************************************************************************/
BERR_Code BAPE_RfEncoder_SetEncodingSettings(
    BAPE_RfEncoderHandle handle,
    const BAPE_RfEncoderEncodingSettings *pSettings
    );

/***************************************************************************
Summary:
    Get the audio connector for an RF Audio Encoder
***************************************************************************/
void BAPE_RfEncoder_GetConnector(
    BAPE_RfEncoderHandle handle,
    BAPE_Connector *pConnector   /* [out] */
    );

/***************************************************************************
Summary:
Add an input to this processing stage
***************************************************************************/
BERR_Code BAPE_RfEncoder_AddInput(
    BAPE_RfEncoderHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
BERR_Code BAPE_RfEncoder_RemoveInput(
    BAPE_RfEncoderHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
BERR_Code BAPE_RfEncoder_RemoveAllInputs(
    BAPE_RfEncoderHandle handle
    );

#endif /* #ifndef BAPE_RF_ENCODER_H__ */



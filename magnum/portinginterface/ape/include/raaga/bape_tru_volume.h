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
*   API name: TruVolume
*    Specific APIs related to SRS TruVolume (formerly Volume IQ) Audio Processing
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef BAPE_TRU_VOLUME_H__
#define BAPE_TRU_VOLUME_H__

#include "bape.h"

/***************************************************************************
Summary: 
TruVolume Handle 
***************************************************************************/
typedef struct BAPE_TruVolume *BAPE_TruVolumeHandle;

/***************************************************************************
Summary:
TruVolume Block Size
***************************************************************************/
typedef enum BAPE_TruVolumeBlockSize
{
    BAPE_TruVolumeBlockSize_e256,
    BAPE_TruVolumeBlockSize_e512,
    BAPE_TruVolumeBlockSize_e768,
    BAPE_TruVolumeBlockSize_e1024,
    BAPE_TruVolumeBlockSize_eMax
} BAPE_TruVolumeBlockSize;

/***************************************************************************
Summary:
TruVolume Mode
***************************************************************************/
typedef enum BAPE_TruVolumeMode
{
    BAPE_TruVolumeMode_eLight,
    BAPE_TruVolumeMode_eNormal,
    BAPE_TruVolumeMode_eHeavy,
    BAPE_TruVolumeMode_eMax
} BAPE_TruVolumeMode;

/***************************************************************************
Summary:
TruVolume Speaker Resolution
***************************************************************************/
typedef enum BAPE_TruVolumeSpeakerResolution
{
    BAPE_TruVolumeSpeakerResolution_e20Hz,
    BAPE_TruVolumeSpeakerResolution_e40Hz,
    BAPE_TruVolumeSpeakerResolution_e110Hz,
    BAPE_TruVolumeSpeakerResolution_e200Hz,
    BAPE_TruVolumeSpeakerResolution_e315Hz,
    BAPE_TruVolumeSpeakerResolution_e410Hz,
    BAPE_TruVolumeSpeakerResolution_eMax
} BAPE_TruVolumeSpeakerResolution;

/***************************************************************************
Summary:
SRS Filter Coefficient Generation Modes. 
Also required by StudioSound.
***************************************************************************/
typedef enum BAPE_SrsFilterCoefficientMode
{
    BAPE_SrsFilterCoefficientMode_eUser,            /* Use user specified coefficients */
    BAPE_SrsFilterCoefficientMode_eSpecification,   /* Use Filter Specification to generate filter coeffcients */
    BAPE_SrsFilterCoefficientMode_eMax
} BAPE_SrsFilterCoefficientMode;

/***************************************************************************
Summary:
SRS High Pass Filter Order. 
Also required by StudioSound.
***************************************************************************/
typedef enum BAPE_SrsFilterOrder
{
    BAPE_SrsFilterOrder_e0 = 0,
    BAPE_SrsFilterOrder_e2 = 2,
    BAPE_SrsFilterOrder_e4 = 4,  
    BAPE_SrsFilterOrder_e6 = 6,
    BAPE_SrsFilterOrder_eMax
} BAPE_SrsFilterOrder;

/***************************************************************************
Summary:
SRS High Pass Filter Coefficients. 
Also required by StudioSound.
***************************************************************************/
typedef struct BAPE_SrsHighPassFilterCoefficientSet
{
    unsigned scale;    /* Specified q-format of the coefficients. (1+scale).(31-scale) format 
                          Default: 1.  scale=1 implies a q-format of 2.30 */
    int coefficientB0;
    int coefficientB1;
    int coefficientB2;
    int coefficientA1;
    int coefficientA2;
} BAPE_SrsHighPassFilterCoefficientSet;

/***************************************************************************
Summary:
SRS High Pass Filter parameter for coefficients specified by user. 
Also required by StudioSound.
***************************************************************************/
typedef struct BAPE_SrsHighPassFilterCoefficients
{
    BAPE_SrsFilterOrder filterOrder;                        /* HighPass Filter order. Default BAPE_SrsFilterOrder_eOrder4 */
    BAPE_SrsHighPassFilterCoefficientSet coefficients[3];   /* Number of active biquads filter decided by filterOrder,  N = filterOrder/2 */
} BAPE_SrsHighPassFilterCoefficients;

/***************************************************************************
Summary:
SRS High Pass Filter parameter for coefficients generated by specification. 
Also required by StudioSound.
***************************************************************************/
typedef struct BAPE_SrsHighPassFilterCoefficientSpecification
{
    unsigned cutoffFrequency;           /* Range: 20 to 1000 hz, Default: 180 Hz */
    BAPE_SrsFilterOrder filterOrder;    /* HighPass Filter order. Default BAPE_SrsFilterOrder_e4 */
} BAPE_SrsHighPassFilterCoefficientSpecification;

/***************************************************************************
Summary:
SRS High Pass Filter Settings. 
Also required by StudioSound.
***************************************************************************/
typedef struct BAPE_SrsHighPassFilterSettings
{
    bool enabled;                                   /* If true, High Pass Filter processing is enabled. Default: true */
    BAPE_SrsFilterCoefficientMode coefficientMode;  /* Coefficient generation mode. Default: BAPE_SrsFilterCoefficientMode_eUserSpecified */

    struct
    {
        BAPE_SrsHighPassFilterCoefficients user[3];                     /* Array size 3, representing supported sampling rates 32, 44.1 and 48kHz, 
                                                                           will be used if coefficientMode is BAPE_SrsFilterCoefficientMode_eUser */
        BAPE_SrsHighPassFilterCoefficientSpecification specification;   /* Filter specification for generating filter coeffcients, 
                                                                           will be used if coefMode is BAPE_SrsFilterCoefficientMode_eSpecification */ 
    } coefficientSettings;
} BAPE_SrsHighPassFilterSettings;

/***************************************************************************
Summary:
TruVolume Settings
***************************************************************************/
typedef struct BAPE_TruVolumeSettings
{
    bool enabled;               /* If true, processing is enabled.  Otherwise this stage is bypassed */

    BAPE_TruVolumeBlockSize blockSize;    /* Block Size for processing */

    bool enableNormalGain;      /* If true, normal processing will be performed.  If false, only bypassGain is applied. */

    unsigned inputGain;         /* Input gain ranges from 0 to 3200 in %. Default: 100 (1.0) */
    unsigned outputGain;        /* Output gain ranges from 0 to 400 in %. Default: 100 (1.0) */
    unsigned bypassGain;        /* Bypass gain (In %).  Ranges from 0 to 100.  Default: 100 (1.0) */

    unsigned referenceLevel;    /* Reference level.  Ranges from 0 to 100 in %.  Default: 50 (0.5) */

    BAPE_TruVolumeMode mode;    /* Processing Mode. */

    BAPE_TruVolumeSpeakerResolution speakerResolution;

    unsigned maxGain;           /* Max gain control (In %).  Ranges from 25 to 25600.  Default: 1600 (16.0) */

    bool enableDcNotchFilter;   /* If true, the DC notch filter will be enabled */

    bool enableNoiseManager;
    int noiseManagerThreshold;  /* Noise manager threshold.  Ranges from 0 to 100 in %.  Default: 10 (0.10) */

    bool enableNormalizer;      /* If true, the normalizer is enabled.  */

    unsigned calibrate;         /* Ranges from 0..25600 in %.  Default=100 (1.0) */

    BAPE_SrsHighPassFilterSettings highPassFilter;  /* High Pass Filter Settings applied prior to TruVolume */
} BAPE_TruVolumeSettings;

/***************************************************************************
Summary:
    Get default settings for an SRS TruVolume stage
***************************************************************************/
void BAPE_TruVolume_GetDefaultSettings(
    BAPE_TruVolumeSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
    Open an SRS TruVolume stage
***************************************************************************/
BERR_Code BAPE_TruVolume_Create(
    BAPE_Handle deviceHandle,
    const BAPE_TruVolumeSettings *pSettings,
    BAPE_TruVolumeHandle *pHandle
    );

/***************************************************************************
Summary:
    Close an SRS TruVolume stage
    
Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
void BAPE_TruVolume_Destroy(
    BAPE_TruVolumeHandle handle
    );

/***************************************************************************
Summary:
    Get Settings for an SRS TruVolume stage
***************************************************************************/
void BAPE_TruVolume_GetSettings(
    BAPE_TruVolumeHandle handle,
    BAPE_TruVolumeSettings *pSettings    /* [out] Settings */
    );

/***************************************************************************
Summary:
    Set Settings for an SRS TruVolume stage
***************************************************************************/
BERR_Code BAPE_TruVolume_SetSettings(
    BAPE_TruVolumeHandle handle,
    const BAPE_TruVolumeSettings *pSettings
    );

/***************************************************************************
Summary:
    Get the audio connector for an SRS TruVolume stage
***************************************************************************/
void BAPE_TruVolume_GetConnector(
    BAPE_TruVolumeHandle handle,
    BAPE_Connector *pConnector
    );

/***************************************************************************
Summary:
Add an input to this processing stage
***************************************************************************/
BERR_Code BAPE_TruVolume_AddInput(
    BAPE_TruVolumeHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
BERR_Code BAPE_TruVolume_RemoveInput(
    BAPE_TruVolumeHandle handle,
    BAPE_Connector input
    );

/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
BERR_Code BAPE_TruVolume_RemoveAllInputs(
    BAPE_TruVolumeHandle handle
    );

#endif /* #ifndef BAPE_TRU_VOLUME_H__ */


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
*   API name: Frontend Amplifier
*    Generic APIs for Low-Noise Amplifiers (LNA).
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_AMPLIFIER_H__
#define NEXUS_AMPLIFIER_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Handle for a generic frontend amplifier object
***************************************************************************/
typedef struct NEXUS_Amplifier *NEXUS_AmplifierHandle;

typedef enum NEXUS_AmplifierGainMode
{
    NEXUS_AmplifierGainMode_eAutomatic,
    NEXUS_AmplifierGainMode_eManual,
    NEXUS_AmplifierGainMode_eMax
} NEXUS_AmplifierGainMode;

/***************************************************************************
Summary:
Settings for a frontend amplifier
***************************************************************************/
typedef struct NEXUS_AmplifierSettings
{
    bool enabled;    /* if not enabled, amplifier can be powered down. */
    NEXUS_AmplifierGainMode gainMode;
    union
    {
        struct
        {
            unsigned outputLevel;   /* AGC Output Level, in 2/10 dB */
            int deltaValue;         /* Delta Value, specifies window, in 2/10 dB */
        } automatic;
        struct
        {
            unsigned gain;          /* Gain, in 2/10 dB */
        } manual;
    } gainSettings;
} NEXUS_AmplifierSettings;

/***************************************************************************
Summary:
    Get the settings of a frontend amplifier
***************************************************************************/
void NEXUS_Amplifier_GetSettings(
    NEXUS_AmplifierHandle handle,
    NEXUS_AmplifierSettings *pSettings  /* [out] */
    );

/***************************************************************************
Summary:
    Set the settings of a frontend amplifier
***************************************************************************/
NEXUS_Error NEXUS_Amplifier_SetSettings(
    NEXUS_AmplifierHandle handle,
    const NEXUS_AmplifierSettings *pSettings
    );

/***************************************************************************
Summary:
    Enable the external output for an amplifier port
***************************************************************************/
NEXUS_Error NEXUS_Amplifier_EnableExternalDriver(
    NEXUS_AmplifierHandle handle,
    unsigned portNum,                       /* The port number you want to enable/disable */
    bool enabled                            /* If true, output will be enabled.  If false, it will be disabled */
    );

/***************************************************************************
Summary:
InBand amplifier mode
***************************************************************************/
typedef enum NEXUS_AmplifierInBandMode
{
    NEXUS_AmplifierInBandMode_eOpenCable,
    NEXUS_AmplifierInBandMode_eAnalog,
    NEXUS_AmplifierInBandMode_eDigital,
    NEXUS_AmplifierInBandMode_ePowerDown,
    NEXUS_AmplifierInBandMode_eMax
} NEXUS_AmplifierInBandMode;

/***************************************************************************
Summary:
    Set the amplifier mode for an InBand port
***************************************************************************/
NEXUS_Error NEXUS_Amplifier_SetInBandMode(
    NEXUS_AmplifierHandle handle,
    unsigned inBandPortNum,
    NEXUS_AmplifierInBandMode inBandMode
    );

/***************************************************************************
Summary:
OutOfBand amplifier mode
***************************************************************************/
typedef enum NEXUS_AmplifierOutOfBandMode
{
    NEXUS_AmplifierOutOfBandMode_eMaxGain,
    NEXUS_AmplifierOutOfBandMode_eBypass,
    NEXUS_AmplifierOutOfBandMode_eNormal,
    NEXUS_AmplifierOutOfBandMode_ePowerDown,
    NEXUS_AmplifierOutOfBandMode_eMax
} NEXUS_AmplifierOutOfBandMode;

/***************************************************************************
Summary:
    Set the amplifier mode for an OutOfBand port
***************************************************************************/
NEXUS_Error NEXUS_Amplifier_SetOutOfBandMode(
    NEXUS_AmplifierHandle handle,
    unsigned outOfBandPortNum,
    NEXUS_AmplifierOutOfBandMode outOfBandMode
    );

/***************************************************************************
Summary:
    Close an amplifier handle
***************************************************************************/
void NEXUS_Amplifier_Close(
    NEXUS_AmplifierHandle handle
    );

#define NEXUS_AMPLIFIER_NUM_INBAND_PORTS        (5)
#define NEXUS_AMPLIFIER_NUM_OUTOFBAND_PORTS     (5)
#define NEXUS_AMPLIFIER_NUM_EXTERNAL_DRIVER     (5)

/***************************************************************************
Summary:
Status of a frontend amplifier
***************************************************************************/
typedef struct NEXUS_AmplifierStatus
{
    NEXUS_AmplifierGainMode gainMode;
    NEXUS_AmplifierInBandMode inBandPortMode[NEXUS_AMPLIFIER_NUM_INBAND_PORTS];
    NEXUS_AmplifierOutOfBandMode outOfBandPortMode[NEXUS_AMPLIFIER_NUM_OUTOFBAND_PORTS];
    bool isExternalDriverEnabled[NEXUS_AMPLIFIER_NUM_EXTERNAL_DRIVER];
    unsigned agcOutputLevel;            /* AGC Output Level, in 1/10 dB */
    int agcDeltaVal;                    /* AGC Delta Value, in 1/10 dB */
    unsigned manualGainVal;             /* If Manual Gain, manual gain value, in 1/10 dB */
    unsigned agcGain;                   /* Gain read from the LNA, for use by the frontend, 
                                           delivered, for example, in the agcValue of the 
                                           NEXUS_31xxConfigSettings */
    bool gainBoostEnabled;              /* True if Stage 1 gain boost is enabled */
    bool superBoostEnabled;             /* True if SuperBoost is enabled */
    bool tiltEnabled;                   /* True if Stage 1 tilt is enabled  */
} NEXUS_AmplifierStatus;

/***************************************************************************
Summary:
    Get the status of a frontend amplifier
***************************************************************************/
NEXUS_Error NEXUS_Amplifier_GetStatus(
    NEXUS_AmplifierHandle handle,
    NEXUS_AmplifierStatus *pStatus  /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_AMPLIFIER_H__ */


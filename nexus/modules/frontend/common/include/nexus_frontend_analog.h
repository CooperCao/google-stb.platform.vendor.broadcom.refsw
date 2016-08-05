/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2004-2016 Broadcom. All rights reserved.
*  
*  This program is the proprietary software of Broadcom and/or its licensors,
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
*   API name: Frontend Analog
*    Generic APIs for Analog tuning.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "nexus_ifd.h"

#ifndef NEXUS_FRONTEND_ANALOG_H__
#define NEXUS_FRONTEND_ANALOG_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Analog Tuning Parameters
***************************************************************************/
typedef struct NEXUS_FrontendAnalogSettings
{
    unsigned frequency;                     /* Analog Frequency, in Hz */

    bool fastAcquisition;                   /* true = fast acquisition mode, false = default */
    bool terrestrial;                       /* Receiver mode: true=terrestrial, false=cable */

    NEXUS_IfdSettings ifdSettings;          /* IFD settings -- Requires an onboard IFD or one bound to this frontend */
} NEXUS_FrontendAnalogSettings;

/***************************************************************************
Summary:
    Initialize an analog settings structure to defaults
***************************************************************************/
void NEXUS_Frontend_GetDefaultAnalogSettings(
    NEXUS_FrontendAnalogSettings *pSettings
    );

/***************************************************************************
Summary:
    Tune a frontend to an analog channel
***************************************************************************/
NEXUS_Error NEXUS_Frontend_TuneAnalog(
    NEXUS_FrontendHandle handle,
    const NEXUS_FrontendAnalogSettings *pSettings
    );

/***********************************************************************
Summary:
      RF Audio NICAM encoder mode
***********************************************************************/
typedef enum NEXUS_FrontendNicamEncoding
{
    NEXUS_FrontendNicamEncoding_eUnknown,
    NEXUS_FrontendNicamEncoding_eStereo_BackupFm,
    NEXUS_FrontendNicamEncoding_eDualMono_BackupFm,
    NEXUS_FrontendNicamEncoding_eStereo_NoBackup,
    NEXUS_FrontendNicamEncoding_eDualMono_NoBackup,
    NEXUS_FrontendNicamEncoding_eMonoData_BackupFm,
    NEXUS_FrontendNicamEncoding_eMonoData_NoBackup,
    NEXUS_FrontendNicamEncoding_eData_BackupFm,
    NEXUS_FrontendNicamEncoding_eData_NoBackup,
    NEXUS_FrontendNicamEncoding_eMax
} NEXUS_FrontendNicamEncoding;

/***************************************************************************
Summary:
Analog Tuning Status Values -- Only valid if an IFD is integrated with
                               or bound to this frontend
***************************************************************************/
typedef struct NEXUS_FrontendAnalogStatus
{
    NEXUS_FrontendAnalogSettings settings;  /* Settings provided at last call to NEXUS_Frontend_TuneAnalog */

    bool locked;        /* lock=true, unlocked=false */
    int carrierOffset;  /* Carrier Frequency Offset, in Hertz */
    int rfAgcLevel;     /* RF AGC level, in 1/10 percent of reference voltage. if 3.3v, 333 is 1.1v */
    int ifAgcLevel;     /* IF AGC level, in 1/10 percent of reference voltage. if 3.3v, 333 is 1.1v */
    int rfInputPower;   /* in units of 1/10 dBuV of the desired signal, supported by not all platforms */
    bool sapDetected;   /* true if SAP is detected    */
    bool stereoDetected;/* true if stereo is detected */
    bool nicamLocked;   /* true if NICAM is detected and locked */
    NEXUS_FrontendNicamEncoding nicamEncoding;  /* NICAM encoder mode */
    uint32_t nicamParityError;                  /* Number of parity errors in the current frame */
    uint32_t nicamErrorCount;                   /* Number of bit errors in the last 128 frames */
    uint64_t avRatio1;                          /* AV Ratio for primary audio carrier in linear units */
    uint64_t avRatio2;                          /* AV Ratio for primary audio carrier in linear units */
    NEXUS_VideoFormat  ifdVideoFormat;          /* Current IFD video format. */
    NEXUS_IfdAudioMode ifdAudioMode;            /* Current IFD audio mode. */
} NEXUS_FrontendAnalogStatus;

/***************************************************************************
Summary:
    Get the analog status of a frontend
***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetAnalogStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendAnalogStatus *pStatus /* [out] */
    );

/***************************************************************************
Summary:
    Get the analog video connector from a frontend
    
Description:
    This function is valid only for internal IF demodulators, and allows
    the decoded video output to be routed directly to the VDEC.
    This routine will return NULL if no direct connection exists between
    the IFD and the audio hardware, in which case an external connection
    such as Ccir656Input will be needed.
See Also:
    NEXUS_AudioInput_Shutdown
***************************************************************************/
NEXUS_VideoInputHandle NEXUS_Frontend_GetAnalogVideoConnector(
    NEXUS_FrontendHandle handle
    );

/***************************************************************************
Summary:
    Get the analog audio connector from a frontend
    
Description:
    This function is valid only for internal IF demodulators, and allows
    the decoded audio output to be routed directly to the audio module.
    This routine will return NULL if no direct connection exists between
    the IFD and the audio hardware, in which case an external connection
    such as I2sInput will be needed.
See Also:
    NEXUS_AudioInput_Shutdown
***************************************************************************/
NEXUS_AudioInputHandle NEXUS_Frontend_GetAnalogAudioConnector(
    NEXUS_FrontendHandle handle
    );

/***************************************************************************
Summary:
    Analog AGC Settings
***************************************************************************/
typedef struct NEXUS_FrontendAnalogAgcSettings
{
    uint16_t top;           /* tuner TOP point */
    uint8_t rfBandwidth;    /* RF AGC band width */
    uint8_t ifBandwidth;    /* IF AGC band width */	
    uint8_t delay;          /* Delay in ms that the acquisition script will wait for the AGC hardware to settle */

    bool agcFreeze;         /* If true, the AGC will be frozen. */
    uint16_t ifAgcValue;    /* The IF AGC value used when frozen. */
} NEXUS_FrontendAnalogAgcSettings;

/***************************************************************************
Summary:
    Get Analog AGC Settings
***************************************************************************/
void NEXUS_Frontend_GetAnalogAgcSettings(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendAnalogAgcSettings *pSettings      /* [out] */
    );

/***************************************************************************
Summary:
    Set Analog AGC Settings
***************************************************************************/
NEXUS_Error NEXUS_Frontend_SetAnalogAgcSettings(
    NEXUS_FrontendHandle handle,
    const NEXUS_FrontendAnalogAgcSettings *pSettings
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_FRONTEND_ANALOG_H__ */

/***************************************************************************
 *     (c)2007-2012 Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 **************************************************************************/
#ifndef NEXUS_PWM_H__
#define NEXUS_PWM_H__

#include "nexus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
**/
typedef enum NEXUS_PwmFreqModeType
{
    NEXUS_PwmFreqModeType_eVariable = 0,
    NEXUS_PwmFreqModeType_eConstant
} NEXUS_PwmFreqModeType;

/**
Summary:
**/
typedef struct NEXUS_PwmChannelSettings
{
    bool openDrain; /* if true, enable open drain */
    NEXUS_PwmFreqModeType  eFreqMode; /* this can also be changed with NEXUS_Pwm_SetFreqMode */
} NEXUS_PwmChannelSettings;

/**
Summary:
PWM channel handle
**/
typedef struct NEXUS_PwmChannel *NEXUS_PwmChannelHandle;

/***************************************************************************
Summary:
Get default settings for call to NEXUS_Pwm_OpenChannel
****************************************************************************/
void NEXUS_Pwm_GetDefaultChannelSettings(
    NEXUS_PwmChannelSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Open PWM channel
****************************************************************************/
NEXUS_PwmChannelHandle NEXUS_Pwm_OpenChannel( /* attr{destructor=NEXUS_Pwm_CloseChannel}  */
    unsigned index, /* PWM channel index */
    const NEXUS_PwmChannelSettings *pSettings
    );

/***************************************************************************
Summary:
Close PWM channel
****************************************************************************/
void NEXUS_Pwm_CloseChannel(
    NEXUS_PwmChannelHandle channel
    );

/***************************************************************************
Summary:
****************************************************************************/
NEXUS_Error NEXUS_Pwm_SetControlWord(
    NEXUS_PwmChannelHandle channel,
    uint16_t word
    );

/***************************************************************************
Summary:
****************************************************************************/
NEXUS_Error NEXUS_Pwm_GetControlWord(
    NEXUS_PwmChannelHandle channel,
    uint16_t *pWord /* [out] */
    );

/***************************************************************************
Summary:
***************************************************************************/
NEXUS_Error NEXUS_Pwm_SetFreqMode(
    NEXUS_PwmChannelHandle channel,
    NEXUS_PwmFreqModeType frequencyMode
    );

/***************************************************************************
Summary:
***************************************************************************/
NEXUS_Error NEXUS_Pwm_GetFreqMode(
    NEXUS_PwmChannelHandle channel,
    NEXUS_PwmFreqModeType *pFrequencyMode /* [out] */
    );

/***************************************************************************
Summary:
****************************************************************************/
NEXUS_Error NEXUS_Pwm_SetOnInterval(
    NEXUS_PwmChannelHandle channel,
    uint16_t onInterval
    );

/***************************************************************************
Summary:
****************************************************************************/
NEXUS_Error NEXUS_Pwm_GetOnInterval(
    NEXUS_PwmChannelHandle channel,
    uint16_t *pOnInterval /* [out] */
    );

/***************************************************************************
Summary:
****************************************************************************/
NEXUS_Error NEXUS_Pwm_RampOnInterval(
    NEXUS_PwmChannelHandle channel,
    uint16_t onInterval
    );

/***************************************************************************
Summary:
****************************************************************************/
NEXUS_Error NEXUS_Pwm_SetPeriodInterval(
    NEXUS_PwmChannelHandle channel,
    uint16_t periodInterval
    );

/***************************************************************************
Summary:
****************************************************************************/
NEXUS_Error NEXUS_Pwm_GetPeriodInterval(
    NEXUS_PwmChannelHandle channel,
    uint16_t *pPeriodInterval /* [out] */
    );

/***************************************************************************
Summary:
****************************************************************************/
NEXUS_Error NEXUS_Pwm_SetOnAndPeriodInterval(
    NEXUS_PwmChannelHandle channel,
    uint16_t OnInterval,
    uint16_t periodInterval
    );

/***************************************************************************
Summary:
****************************************************************************/
NEXUS_Error NEXUS_Pwm_GetOnAndPeriodInterval(
    NEXUS_PwmChannelHandle channel,
    uint16_t *pOnInterval, /* [out] */
    uint16_t *pPeriodInterval /* [out] */
    );

/***************************************************************************
Summary:
****************************************************************************/
NEXUS_Error NEXUS_Pwm_Start(
    NEXUS_PwmChannelHandle channel
    );

/***************************************************************************
Summary:
****************************************************************************/
NEXUS_Error NEXUS_Pwm_Stop(
    NEXUS_PwmChannelHandle channel
    );

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_PWM_H__ */


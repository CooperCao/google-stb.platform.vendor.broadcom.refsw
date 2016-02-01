/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
*   API name: I2sOutput
*    Specific APIs related to I2S audio outputs.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_I2S_OUTPUT_H__
#define NEXUS_I2S_OUTPUT_H__

#include "nexus_types.h"
#include "nexus_audio_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=************************************
Interface: I2sOutput

Header file: nexus_i2s_output.h

Module: Audio

Description: Route PCM audio data to an I2S output

**************************************/

/**
Summary:
Handle for I2S output
**/
typedef struct NEXUS_I2sOutput *NEXUS_I2sOutputHandle;

/***************************************************************************
Summary:
Bit Clock (SCLK) Rate
***************************************************************************/
typedef enum NEXUS_I2sOutputSclkRate
{
    NEXUS_I2sOutputSclkRate_e64Fs,
    NEXUS_I2sOutputSclkRate_e128Fs,
    NEXUS_I2sOutputSclkRate_eMax
} NEXUS_I2sOutputSclkRate;

/***************************************************************************
Summary:
Master Clock (MCLK) Rate
***************************************************************************/
typedef enum NEXUS_I2sOutputMclkRate
{
    NEXUS_I2sOutputMclkRate_e128Fs,
    NEXUS_I2sOutputMclkRate_e256Fs,
    NEXUS_I2sOutputMclkRate_e384Fs,
    NEXUS_I2sOutputMclkRate_e512Fs,
    NEXUS_I2sOutputMclkRate_eAuto,	/* Automatically select based on 
                                           timing source.  Not applicable on 
                                           all platforms */
    NEXUS_I2sOutputMclkRate_eMax
} NEXUS_I2sOutputMclkRate;

/***************************************************************************
Summary:
I2S Output Settings
***************************************************************************/
typedef struct NEXUS_I2sOutputSettings
{
    bool                     lsbAtLRClock;       /* Data Justification.
                                                    Controls whether LSB or 
                                                    MSB is at LRCK transition
                                                    TRUE: LSB, FALSE: MSB */
    bool                     alignedToLRClock;   /* Controls whether data is 
                                                    aligned with LRCK or delayed
                                                    by one SCLK period
                                                    FALSE: Delayed. 
                                                    TRUE: Aligned */
    bool                     lrClkPolarity;      /* Sets the polarity of the 
                                                    left/right clock
                                                    TRUE: High for Left
                                                    FALSE: Low for Left */
    NEXUS_I2sOutputSclkRate  sclkRate;           /* SClk (bit Clock, multiple of Fs) 
                                                    rate. */
    NEXUS_I2sOutputMclkRate  mclkRate;           /* MClk rate. (multiple of Fs) */
} NEXUS_I2sOutputSettings;

/***************************************************************************
Summary:
Get default settings for an I2S output
***************************************************************************/
void NEXUS_I2sOutput_GetDefaultSettings(
    NEXUS_I2sOutputSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
Open an I2S Output device
***************************************************************************/
NEXUS_I2sOutputHandle NEXUS_I2sOutput_Open( /* attr{destructor=NEXUS_I2sOutput_Close}  */
    unsigned index,
    const NEXUS_I2sOutputSettings *pSettings /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
Close an I2S Output device
***************************************************************************/
void NEXUS_I2sOutput_Close(
    NEXUS_I2sOutputHandle handle
    );

/***************************************************************************
Summary:
Get settings for an I2S output
***************************************************************************/
void NEXUS_I2sOutput_GetSettings(
    NEXUS_I2sOutputHandle handle,
    NEXUS_I2sOutputSettings *pSettings  /* [out] Settings */
    );

/***************************************************************************
Summary:
Set settings for an I2S output
***************************************************************************/
NEXUS_Error NEXUS_I2sOutput_SetSettings(
    NEXUS_I2sOutputHandle handle,
    const NEXUS_I2sOutputSettings *pSettings
    );

/***************************************************************************
Summary:
Get the audio connector for an I2S output
***************************************************************************/
NEXUS_AudioOutputHandle NEXUS_I2sOutput_GetConnector(
    NEXUS_I2sOutputHandle handle
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_I2S_OUTPUT_H__ */


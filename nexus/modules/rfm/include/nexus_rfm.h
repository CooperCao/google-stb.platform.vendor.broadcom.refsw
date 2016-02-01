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
#ifndef NEXUS_RFM_H__
#define NEXUS_RFM_H__

#include "nexus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=**********************
RFM is an RF modulated audio and video output.
************************/

/**
Summary:
Handle for the Rfm interface.
**/
typedef struct NEXUS_Rfm *NEXUS_RfmHandle;

/**
Summary:
Settings for Rfm interface

Description:
Audio volume is controlled via the NEXUS_AudioDac output which is connected to this Rfm.
**/
typedef struct NEXUS_RfmSettings
{
    unsigned channel; /* TV channel number. Typical channels are 3 or 4. */
    int volume;       /* Specified in 1/100 dB.  0 = normal, positive numbers indicate gain, negative indicates attenuation. */
    bool muted;       /* Audio muted */
    int audioVideoRatio; /* Nexus RFM Audio-Video Ratio selection: In units of 1/100th dB.
                            For instance: -1700 is -17.00 and -1650 is -16.50.
                            [-17,-16,-15,-14] dB supported for NTSC only */
} NEXUS_RfmSettings;

/*
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.
*/
void NEXUS_Rfm_GetDefaultSettings(
    NEXUS_RfmSettings *pSettings    /* [out] Settings */
    );

/**
Summary:
Open a new Rfm interface
**/
NEXUS_RfmHandle NEXUS_Rfm_Open( /* attr{destructor=NEXUS_Rfm_Close}  */
    unsigned index,
    const NEXUS_RfmSettings *pSettings /* attr{null_allowed=y} */
    );

/**
Summary:
Close the Rfm interface
**/
void NEXUS_Rfm_Close(
    NEXUS_RfmHandle output
    );


/**
Summary:
Get current settings
**/
void NEXUS_Rfm_GetSettings(
    NEXUS_RfmHandle output,  
    NEXUS_RfmSettings *pSettings    /* [out] Settings */
    );

/**
Summary:
Apply new settings
**/
NEXUS_Error NEXUS_Rfm_SetSettings(
    NEXUS_RfmHandle output,  
    const NEXUS_RfmSettings *pSettings    
    );

/**
Summary:
Returns the abstract NEXUS_VideoOutput connector for this Interface. 
The NEXUS_VideoOutput connector is added to a Display in order to route that Display's video to the output.

Description:
Used in NEXUS_Display_AddOutput
**/
NEXUS_VideoOutput NEXUS_Rfm_GetVideoConnector( /* attr{shutdown=NEXUS_VideoOutput_Shutdown} */
    NEXUS_RfmHandle output
    );

/**
Summary:
Returns the abstract NEXUS_AudioOutput connector for this Interface. 
**/
NEXUS_AudioOutputHandle NEXUS_Rfm_GetAudioConnector( /* attr{shutdown=NEXUS_AudioOutput_Shutdown} */
    NEXUS_RfmHandle output
    );

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_RFM_H__ */

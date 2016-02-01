/******************************************************************************
 *    (c)2008-2013 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
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
 *****************************************************************************/

#ifndef NEXUS_REMUX_H__
#define NEXUS_REMUX_H__

#include "nexus_types.h"
#include "nexus_pid_channel.h"
#include "nexus_playpump.h"
#ifdef __cplusplus
extern "C" {
#endif

/*=************************
The Remux block outputs data transport data to an external transport receiver.

Please note that the remux API does not follow nexus conventions regarding Open/Close.
You must Open and Close the remux handle for every session in order to reconfigure
its NEXUS_RemuxSettings. This may be changed in the future.
**************************/

/*
Summary:
Selects the clock source for the remux channel
*/
typedef enum NEXUS_RemuxClock
{
    NEXUS_RemuxClock_e81Mhz,        /* 81 MHz */
    NEXUS_RemuxClock_e54Mhz,        /* 54 MHz */
    NEXUS_RemuxClock_e40_5Mhz,      /* 40.5 MHz */
    NEXUS_RemuxClock_e20_25Mhz,     /* 20.25 MHz */
    NEXUS_RemuxClock_e27Mhz_VCXO_A, /* 27 MHz, from VCXO A */
    NEXUS_RemuxClock_eInputBand,    /* Input band to clock. */
    NEXUS_RemuxClock_eMax
} NEXUS_RemuxClock;

/*
Summary:
Settings needed for opening a remux channel.
*/
typedef struct NEXUS_RemuxSettings
{
    /* these settings are only applies at NEXUS_Remux_Open */
    NEXUS_RemuxClock outputClock;   /* Select the output clock source */
    NEXUS_InputBand remuxClockIBSrc; /* If outputClock == NEXUS_RemuxClock_eInputBand, then use this to specify the input band */

    bool parallelOutput;            /* If true, parallel output will be requested.  If false, serial output will be enabled */
    bool invertClock;               /* If true, data will be output on the falling edge.  */
    bool invertSync;                /* If true, sync will be active low */
    bool byteSync;                  /* If true, sync will be active for the entire packet.
                                       If false, sync will be active for the first byte (serial only). */
    bool insertNullPackets;         /* If true, enable the insertion of null packets.  Otherwise, insertion is disabled. */
    unsigned packetDelayCount;      /* Programmable delay between output packets in units of remux clock cycles. 0 means no gap between output packets */
    
    
    /* the following settings are applied at NEXUS_Remux_Open and NEXUS_Remux_SetSettings */
    bool allPass;                   /* allPass mode passes all transport packets through remux.
                                       If you set NEXUS_ParserBandSettings.allPass or NEXUS_PlaypumpSettings.allPass and want all packets to also be passed through remux, you must
                                       also set this allPass boolean.
                                       Unlike bypass, allPass allows for NULL packet insertion to hit a constant output bitrate. */
    bool bypass;                    /* bypass mode passes all data through remux without regard for packetization.
                                       This can be used for sending non-transport data. */

    bool pcrCorrectionEnabled;
    uint32_t pcrCorrectionOffset;   /* Only used if pcrCorrectionOffset is true. */

    bool pacing;                    /* If true, pace this stream using embedded timestamp information. */
    unsigned pacingMaxError;        /* Set the maximum allowable timestamp error (TS_RANGE_ERROR) used during playback pacing. */
    unsigned maxDataRate;           /* unused. see NEXUS_RemuxParserBandwidth. */

    bool enablePcrJitterAdjust;
    
    bool paused;
} NEXUS_RemuxSettings;

/**
Summary:
Settings for an parser band added to a remux channel.
**/
typedef struct NEXUS_RemuxParserBandwidth
{
    NEXUS_PlaypumpHandle playpump;         /* playpump parser band. for live, set this to NULL. */
    NEXUS_ParserBand parserBand;           /* live parser band. only used if playpump == NULL. */
    unsigned maxDataRate;                  /* Maximum data rate for this parser band input to remux in units of bits per second.
                                              Default is typically 25000000 (i.e. 25 Mbps). If you increase this, you need to analyze total
                                              transport bandwidth and overall system bandwidth. */
}NEXUS_RemuxParserBandwidth;

/*
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the
addition of new strucutre members in the future.
*/
void NEXUS_Remux_GetDefaultSettings(
    NEXUS_RemuxSettings *pSettings /* [out] */
    );

/*
Summary:
Open a remux channel
*/
NEXUS_RemuxHandle NEXUS_Remux_Open( /* attr{destructor=NEXUS_Remux_Close} */
    unsigned index,
    const NEXUS_RemuxSettings *pSettings
    );

/*
Summary:
Close the remux channel
*/
void NEXUS_Remux_Close(
    NEXUS_RemuxHandle handle
    );


/*
Summary:
Get default settings for NEXUS_RemuxParserBand.
*/
void NEXUS_Remux_GetDefaultParserBandwidth(
    NEXUS_RemuxParserBandwidth *pSettings /* [out] */
    );

/*
Summary:
Set the bit rate for a parser band that needs to be added
to a remux channel.
*/
NEXUS_Error NEXUS_Remux_SetParserBandwidth(
    NEXUS_RemuxHandle handle,
    const NEXUS_RemuxParserBandwidth *pSettings
    );

/*
Summary:
Add a pid channel to the remux channel
*/
NEXUS_Error NEXUS_Remux_AddPidChannel(
    NEXUS_RemuxHandle handle,
    NEXUS_PidChannelHandle pidChannel
    );


/*
Summary:
Remove a pid channel from the remux channel
*/
NEXUS_Error NEXUS_Remux_RemovePidChannel(
    NEXUS_RemuxHandle handle,
    NEXUS_PidChannelHandle pidChannel
    );


/*
Summary:
Remove all pid channels from the remux channel
*/
void NEXUS_Remux_RemoveAllPidChannels(
    NEXUS_RemuxHandle handle
    );

/*
Summary:
Start the remux channel
*/
NEXUS_Error NEXUS_Remux_Start(
    NEXUS_RemuxHandle handle
    );

/*
Summary:
Stop the remux channel
*/
void NEXUS_Remux_Stop(
    NEXUS_RemuxHandle handle
    );

/*
Summary:
Set the new settings for a remux channel
*/
NEXUS_Error NEXUS_Remux_SetSettings(
    NEXUS_RemuxHandle handle,
    const NEXUS_RemuxSettings *pSettings
    );

/*
Summary:
Get the current settings for a remux channel
*/
void NEXUS_Remux_GetSettings(
    NEXUS_RemuxHandle handle,
    NEXUS_RemuxSettings *pSettings        /* [out] */
    );


#ifdef __cplusplus
}
#endif

#endif /* NEXUS_REMUX_H__ */


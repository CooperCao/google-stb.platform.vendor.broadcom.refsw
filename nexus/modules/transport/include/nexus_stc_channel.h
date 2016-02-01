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
*   Management of STC Channels that deliver timebases to decoders.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef NEXUS_STCCHANNEL_H__
#define NEXUS_STCCHANNEL_H__

#include "nexus_stc_channel_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.
**/
void NEXUS_StcChannel_GetDefaultSettings(
    unsigned index,                        /* index of the StcChannel you want to open */
    NEXUS_StcChannelSettings *pSettings    /* [out] Default Settings */
    );

/**
Summary:
Open a new STC channel
**/
NEXUS_StcChannelHandle NEXUS_StcChannel_Open(  /* attr{destructor=NEXUS_StcChannel_Close}  */
    unsigned index,                            /* index of the StcChannel you want to open. This corresponds to the HW PCR_OFFSET context number.
                                                  See NEXUS_StcChannelSettings.stcIndex if you want to control the selection of your HW STC mapping. */
    const NEXUS_StcChannelSettings *pSettings  /* attr{null_allowed=y} Initial settings */
    );

/**
Summary:
Close an STC channel
**/
void NEXUS_StcChannel_Close(
    NEXUS_StcChannelHandle handle
    );

/**
Summary:
Get current settings
**/
void NEXUS_StcChannel_GetSettings(
    NEXUS_StcChannelHandle handle,
    NEXUS_StcChannelSettings *pSettings          /* [out] Current Settings */
    );

/**
Summary:
Update current settings
**/
NEXUS_Error NEXUS_StcChannel_SetSettings(
    NEXUS_StcChannelHandle handle,
    const NEXUS_StcChannelSettings *pSettings    /* Settings for this StcChannel */
    );

/*
Summary:
Returns status info for this STC channel
*/
NEXUS_Error NEXUS_StcChannel_GetStatus(
    NEXUS_StcChannelHandle handle,
    NEXUS_StcChannelStatus *pStatus /* [out] */
    );

/**
Summary:
Mark the current STC as invalid, preventing any TSM decision from being made until new input is received.

Description:
This is useful when the application transitions the decoders and it knows that no TSM decision should
be made until the StcChannel receives new feedback from the decoders.
**/
NEXUS_Error NEXUS_StcChannel_Invalidate(
    NEXUS_StcChannelHandle handle
    );

/**
Summary:
Get the current STC value.

Description:
For RAVE-based systems, this will return the Serial STC + offset value.
**/
void NEXUS_StcChannel_GetStc(
    NEXUS_StcChannelHandle handle,
    uint32_t *pStc                                /* [out] Current Stc */
    );

/*
Summary:
Force a value to be loaded for the STC.

Description:
This is only effective in NEXUS_StcChannelMode_eHost mode.
*/
NEXUS_Error NEXUS_StcChannel_SetStc(
    NEXUS_StcChannelHandle handle,
    uint32_t stc                                  /* New STC */
    );

/*
Summary:
Freeze will stop the STC from changing value. All other settings are preserved.
*/
NEXUS_Error NEXUS_StcChannel_Freeze(
    NEXUS_StcChannelHandle stcChannel,
    bool frozen /* if true, the STC stops. if false, the STC starts. */
    );

/*
Summary:
Change the rate of the STC. Rate is calculated as: rate = (increment) / (prescale + 1).

Description:
The following are some examples of how the increment and prescale parameters
map to some typical rates:

    1, 0 = 1x (normal speed)
    0, X = 0.0x (paused)
    1, 1 = 0.5x (slow motion)
    2, 0 = 2x (fast forward)
    8, 9 = 0.8x (slow motion, audio trick mode may be possible)
    12, 9 = 1.2x (fast forward, audio trick mode may be possible)

Be aware that the audio and video decoders may not be able to decoder at faster than 1.0x speeds.
They are limited by overall memory bandwidth and RTS (real time scheduler) programming.
Also, speeds that may work for low bitrate (SD) streams or low-complexity streams may not work for
high bitrate (HD) streams or high-complexity streams.

If a decoder cannot keep up with the STC rate, it will drop data. This will result in
video drops or audio pops. Video drops may be acceptable because it will be similar to a host trick mode.
Audio pops are usually not acceptable.

Some versions of the audio decoder support 0.8x and 1.2x audio trick modes.
For any other STC trick mode, the audio decoder should be muted (for fast resumption to normal play) or stopped.
If you are using the Nexus Playback module, this will be done automatically if NEXUS_PlaybackSettings.stcTrick is true.

There is no reverse stc rate.
*/
NEXUS_Error NEXUS_StcChannel_SetRate(
    NEXUS_StcChannelHandle stcChannel,
    unsigned increment,
    unsigned prescale
    );

/**
Summary:
Verify that an STC channel and PID channel have compatible settings

Description:
NEXUS_StcChannel and a live or playback NEXUS_PidChannel are configured separately in an application.
If you misconfigure them (for example, set one to MPEG2TS and another to DSS), you will
get obscure TSM failures. This function allows the VideoDecoder or AudioDecoder, which joins the two
together, to provide more direct feedback on such a misconfiguration.

This function returns a non-zero value if there is a problem.
**/
NEXUS_Error NEXUS_StcChannel_VerifyPidChannel(
    NEXUS_StcChannelHandle stcChannel,
    NEXUS_PidChannelHandle pidChannel
    );

/*
Summary:
Settings passed into NEXUS_StcChannel_SetPairConfiguration
*/
typedef struct NEXUS_StcChannelPairSettings {
    bool connected; /* true if pair of STC's are cconnected */
    unsigned window; /* Window, expressed in mSec. The STC increment is stalled when the comparison exceedes this window. */
} NEXUS_StcChannelPairSettings;

/**
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.
**/
void NEXUS_StcChannel_GetDefaultPairSettings(
            NEXUS_StcChannelPairSettings *pSettings
        );

/*
Summary:
Connects or disconnects pair of STC's

Description:
This is used when transcoding audio and video streams in non-realtime configuration
*/
NEXUS_Error NEXUS_StcChannel_SetPairSettings(
    NEXUS_StcChannelHandle stcChannel1,
    NEXUS_StcChannelHandle stcChannel2,
    const NEXUS_StcChannelPairSettings *pSettings
    );


#ifdef __cplusplus
}
#endif

#endif

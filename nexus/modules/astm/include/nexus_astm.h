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
#ifndef NEXUS_ASTM_H__
#define NEXUS_ASTM_H__

/*=***********************************
Module Overview

This interface allows the system to adapt to bad TSM environments including
bad PCR's, missing PCR's, bad PTS's, etc. ASTM monitors the system and moves
the decoders into and out of TSM mode as needed. The goal is to keep seeing
and hearing the audio and video with the best lispsync possible, even if TSM
is not possible.

The ASTM algorithm can be summarized as follows:

Presentation Control
 - ASTM monitors both audio and video decoders looking for TSM failures.
   - A TSM failure is defined as a certain number of PTS errors. The current
     threshold is 10,but that is programmable.
 - If both decoders are failing, ASTM will try a PTS master mode first.
 - If only one decoder is failing, ASTM will enter directly into non-TSM
   mode.
 - If either decoder is still failing after trying a master mode, it will
   switch to non-TSM mode.

Clock Recovery
 - ASTM monitors the receipt of PCRs looking for high jitter
   - The jitter tolerance threshold and consecutive out-of-tolerance counter
     are programmable.
 - If the incoming clock reference has high jitter, ASTM will decouple the
   timebase from the input clock (and couple it to the internal crystal)
 - If the incoming clock reference returns to low jitter, ASTM will recouple
   the timebase to the input clock

If the application manually puts a decoder into vsync mode, ASTM will not
move it into TSM mode.  The application can also manually set the three
controls ASTM provides:
 - presentation control (PTS or output clock),
 - stc source (PCR or PTS, with which PTS is used being a preference), and
 - clock coupling (input or crystal).
Any manual setting will not be overridden by ASTM.

See nexus/examples/astm.c for an example.

**************************************/

#include "nexus_video_decoder.h"
#include "nexus_audio_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_timebase.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
Summary:
Handle for Astm interface
**/
typedef struct NEXUS_Astm * NEXUS_AstmHandle;

/**
Summary:
Determines what controls the rate of presentation
**/
typedef enum NEXUS_AstmPresentationRateControl
{
    NEXUS_AstmPresentationRateControl_eAdaptive, /* adapts to current conditions */
    NEXUS_AstmPresentationRateControl_eTimeStamp, /* frames are released based
        on their time stamp */
    NEXUS_AstmPresentationRateControl_eOutputClock, /* frames are released based
        on output clock rate (timestamps are ignored) */
    NEXUS_AstmPresentationRateControl_eMax
} NEXUS_AstmPresentationRateControl;

/* maintained for backward compatibility */
#define NEXUS_PresentationRateControl_eTimeStamp NEXUS_AstmPresentationRateControl_eTimeStamp
#define NEXUS_PresentationRateControl_eOutputClock NEXUS_AstmPresentationRateControl_eOutputClock
#define NEXUS_PresentationRateControl_eMax NEXUS_AstmPresentationRateControl_eMax

/**
Summary:
Determines what quantity is used to seed the STC
**/
typedef enum NEXUS_AstmStcSource
{
    NEXUS_AstmStcSource_eAdaptive, /* adapts to current conditions */
    NEXUS_AstmStcSource_eClockReference, /* PCR, SCR, RTS, etc. */
    NEXUS_AstmStcSource_ePts, /* Presentation Time Stamp, per ISO 13818-1 */
    NEXUS_AstmStcSource_eMax
} NEXUS_AstmStcSource;

/**
Summary:
Describes whether the timebases will be coupled to the input or the crystal
**/
typedef enum NEXUS_AstmClockCoupling
{
    NEXUS_AstmClockCoupling_eAdaptive, /* adapts to current conditions */
    NEXUS_AstmClockCoupling_eInputClock, /* PCR, VSYNC, HSYNC, etc. */
    NEXUS_AstmClockCoupling_eInternalClock, /* internal XTAL */
    NEXUS_AstmClockCoupling_eMax /* enum terminator */
} NEXUS_AstmClockCoupling;

/**
Summary:
Describes whether the timebases will be coupled to the input or the crystal
**/
typedef enum NEXUS_AstmClockRate
{
    NEXUS_AstmClockRate_e45Khz = 45000,
    NEXUS_AstmClockRate_e27Mhz = 27000000,
    NEXUS_AstmClockRate_eMax /* enum terminator */
} NEXUS_AstmClockRate;

#define NEXUS_ASTM_AUDIO_DECODERS 2

/**
Summary:
Settings for NEXUS_Astm
**/
typedef struct NEXUS_AstmPresenterConfig
{
    unsigned minimumTimeBetweenEvents;/* minimum time that must elapse between receiving events, in ms. used with presentation processing frequency to determine size of 
                                                                      event ring buffer. if events come more often than this time, the event buffer may overflow, and newer events   will be lost */
    unsigned passEventCountThreshold; /* how many pass events must be received before deeming this  presenter as "passing", in events */
    unsigned failEventCountThreshold; /* how many fail events must be received before deeming this presenter as "failing", in events */
    bool tsmThresholdAdjustment; /* if true, during certain types of failures ASTM will attempt to adjust the TSM threshold instead of entering non-TSM mode */
    bool manageRateControl; /* if true, presentation rate control changes performed by ASTM will affect this presenter */
    unsigned tsmRecoveryAcquisitionPeriod; /* how often do we schedule attempts to adjust the PTS offset for TSM recovery in non-TSM mode, in ms */
    unsigned tsmRecoveryTrackingTimeout; /* once we see a TSM pass, how long do we count them and then compare to frames decoded, in ms */
    unsigned ptsStcDiffAdjustmentThreshold; /* if the abs(pts/stc diff) is below this, we don't adjust, only applies in non-TSM mode during tsm recovery acquisition */
    unsigned maxAllowableFirstPtsStcDiff; /* if we exceed this, presentation rate control will be changed to output clock for the exceeding presenter; this behavior is disabled when set to zero */
} NEXUS_AstmPresenterConfig;

/**
Summary:
Settings for NEXUS_Astm
**/
typedef struct NEXUS_AstmSettings
{
    bool enabled;
    bool enableAutomaticLifecycleControl; /* DEPRECATED; if true, ASTM will auto-start/stop
        based on decoder start/stop conditions; if false, the app will be
        responsible for starting/stopping ASTM */
    NEXUS_AstmPresentationRateControl presentationRateControl;
    NEXUS_AstmStcSource stcSource;
    NEXUS_AstmClockCoupling clockCoupling;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_VideoDecoderHandle videoDecoder; /* Astm supports one video decoder */
    NEXUS_AudioDecoderHandle audioDecoder[NEXUS_ASTM_AUDIO_DECODERS]; /* Both
        audio decoders must be for the same program. Two audio decoders are used
        for PCM and compressed pass-through configurations. */
    void * stcMaster; /* points to video decoder handle or either audio decoder handle
        for pts master mode or adaptive mode */
    unsigned int syncLimit; /* in a master mode, this is the limit on the difference
        between PTS and STC to which the master mode is applied in the decoder.
        Beyond this limit, the decoder applies it's normal TSM processing. */
    NEXUS_AstmPresenterConfig videoPresenterConfig; /* Astm supports one video decoder */
    NEXUS_AstmPresenterConfig audioPresenterConfig[NEXUS_ASTM_AUDIO_DECODERS];
    struct
    {
        unsigned minimumTimeBetweenEvents;    /* minimum time that must elapse between receiving events, in ms. used with processing frequency to determine size of event 
                                                                                            ring buffer. if events come more often than this time, the event buffer may overflow, and newer events will be lost */
        unsigned deviationThreshold;      /* threshold above which abs value of clock reference rate difference to STC will be considered deviant */
        unsigned deviantCountThreshold;   /* how many deviant clock coupling measurements must be received before deeming the clock reference bad and decoupling 
                                                                               the timebases from the input */
        unsigned idealCountThreshold;     /* how many ideal clock coupling measurements must be received before deeming the clock reference good and recoupling the timebases to the input */
        unsigned initialAcquisitionTime;  /* time just after calling start before processing clock reference event queue for the first time, in ms */
        unsigned processingFrequency;     /* after initial acquisition period,  how often clock reference event queue will be processed for deviants, in ms */
        unsigned idealProcessingFrequency; /* after detecting deviants, how often clock reference event queue will be processed for ideals, in ms */
        unsigned settlingTime;        /* after changing clock coupling modes,   how long we will wait for clock reference to settle in the new mode */          
    } clockCouplingConfig; /* clock coupling config */

    struct
    {
        unsigned initialAcquisitionTime;     /* time just after calling start before processing presenter event queues for the first time, in ms */
        unsigned processingFrequency;        /* after initial acquisition period, how often presenter event queues will be processed, in ms */
        unsigned tsmDisabledWatchdogTimeout; /* after entering TSM-disabled mode, how long before we attempt to re-enter TSM-enabled mode unconditionally */
        unsigned settlingTime;               /* after changing presentation modes, how long we will wait for presenters to settle in the new mode */
        NEXUS_AstmPresentationRateControl preferredPresentationRateControl; /* This is used as the highest level presentation rate control goal of ASTM's presentation task */
    } presentationConfig; /* presentation config */    
} NEXUS_AstmSettings;

/**
Summary:
Get default settings
**/
void NEXUS_Astm_GetDefaultSettings(
    NEXUS_AstmSettings * pSettings /* [out] */
    );

/**
Summary:
Create a new Astm.

Description:
You will need one Astm for every program for which you want adaptive system time management.

You cannot attach a decoder to more than one Astm at a time.
**/
NEXUS_AstmHandle NEXUS_Astm_Create( /* attr{destructor=NEXUS_Astm_Destroy} */
    const NEXUS_AstmSettings * pSettings
    );

/**
Summary:
**/
void NEXUS_Astm_Destroy(
    NEXUS_AstmHandle handle
    );

/**
Summary:
Get current settings.
**/
void NEXUS_Astm_GetSettings(
    NEXUS_AstmHandle    handle,
    NEXUS_AstmSettings * pSettings /* [out] */
    );

/**
Summary:
Apply new settings.
**/
NEXUS_Error NEXUS_Astm_SetSettings(
    NEXUS_AstmHandle          handle,
    const NEXUS_AstmSettings * pSettings
    );

/*
Summary:
Start ASTM processing
*/
NEXUS_Error NEXUS_Astm_Start(
    NEXUS_AstmHandle astm
    );

/*
Summary:
Stop ASTM processing
*/
void NEXUS_Astm_Stop(
    NEXUS_AstmHandle astm
    );

#ifdef __cplusplus
}
#endif

#endif



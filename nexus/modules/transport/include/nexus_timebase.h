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
*   Management of timebase (clock rate) blocks.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_TIMEBASE_H__
#define NEXUS_TIMEBASE_H__

#include "nexus_types.h"
#include "nexus_video_types.h"
#include "nexus_pid_channel.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=*
A NEXUS_Timebase performs clock recovery using a DPCR hardware block.
It provides interfaces to connect and sync with various inputs.

See nexus/examples/decode.c for an example application.

See NEXUS_StcChannelHandle to see how NEXUS_Timebase is used to provide lipsync.
See NEXUS_DisplayHandle and NEXUS_AudioMixerHandle to see how NEXUS_Timebase is used to regulate output timing.
**/

/**
Summary:
The type of source that NEXUS_Timebase is connected to.
**/
typedef enum NEXUS_TimebaseSourceType
{
    NEXUS_TimebaseSourceType_ePcr,       /* Lock to a PCR in a transport stream. Used for live decode. Can also be used for PVR with pacing. */
    NEXUS_TimebaseSourceType_eFreeRun,   /* Lock to the 27MHz system clock. Used for PVR. */
    NEXUS_TimebaseSourceType_eAnalog,    /* Lock to AnalogVideoDecoder. */
    NEXUS_TimebaseSourceType_eHdDviIn,   /* Lock to the HdmiInput. */
    NEXUS_TimebaseSourceType_eCcir656In, /* Lock to the CCIR 656 video input. */
    NEXUS_TimebaseSourceType_eI2sIn,     /* Lock to the I2S audio input. */
    NEXUS_TimebaseSourceType_eSpdifIn,   /* Lock to the SPDIF audio input. */
    NEXUS_TimebaseSourceType_eMax
} NEXUS_TimebaseSourceType;

/**
Summary:
Tracking range for DPCR block in units of parts per million
**/
typedef enum NEXUS_TimebaseTrackRange
{
    NEXUS_TimebaseTrackRange_e8ppm,
    NEXUS_TimebaseTrackRange_e15ppm,
    NEXUS_TimebaseTrackRange_e30ppm,
    NEXUS_TimebaseTrackRange_e61ppm,
    NEXUS_TimebaseTrackRange_e122ppm,
    NEXUS_TimebaseTrackRange_e244ppm,
    NEXUS_TimebaseTrackRange_eMax
} NEXUS_TimebaseTrackRange;

/**
Summary:
Settings for the free-run source type
**/
typedef struct NEXUS_TimebaseFreeRunSourceSettings
{
    unsigned centerFrequency; /* Controls the absolute center frequency of the timebase relative to the crystal.
                                 A 27MHz timebase is generated with 0x400000 (default). centerFrequency may be adjusted to compensate for crystal variations.
                                 Adjusting centerFrequency by 1 LSB has an effect of roughly 3.8 ppm. */
    NEXUS_TimebaseTrackRange trackRange;
    /*
    ** For the internal (free-running) source, the 27 MHz clock may be scaled by the prescale and inc values.
    ** The scaled clock signal is used to generate an imitation PCR, which the hardware uses as if it where
    ** the real thing.
    */
    uint32_t prescale;   /* A prescale of N divides the clock by N+1. For example, setting prescale
                        to zero will not affect the clock. Or, setting prescale to 57 will swallow
                        57 of every 58 clock ticks */
    uint32_t inc;        /* Together with prescale, this value converts clock ticks to PCRs for lock.
                        The clock ticks are divided by the prescaler. At every divided event,
                        the synthesized PCR is incremented by the amount in this inc register */
    uint8_t loopDirectPathGain;
    uint8_t loopGain;
    uint8_t loopIntegratorLeak;
} NEXUS_TimebaseFreeRunSourceSettings;

/**
Summary:
Settings for the PCR source type
**/
typedef struct NEXUS_TimebasePcrSourceSettings
{
    NEXUS_PidChannelHandle pidChannel; /* Pid channel for the PCR */
    unsigned maxPcrError;      /* Maximum amount of PCR jitter which DPCR hardware will absorb. Any PCR change outside of this
                                 threshold will result in the STC being updated. Measured in 33 bit PTS units (e.g. 90KHz MPEG TS).
                                 Default is 255 (which, for MPEG TS, is 255/90000 = 0.0028 = 2.8 milliseconds). */
    NEXUS_TimebaseTrackRange trackRange;
    NEXUS_TristateEnable jitterCorrection;
} NEXUS_TimebasePcrSourceSettings;

/**
Summary:
Settings for the VDEC source type
**/
typedef struct NEXUS_TimebaseVdecSourceSettings
{
    unsigned index; /* index of the AnalogVideoDecoder */
    NEXUS_VideoFormat format;
    NEXUS_VideoFrameRate frameRate; /* deprecated. use NEXUS_Timebase_SetVdecFrameRate instead. */
    NEXUS_TimebaseTrackRange trackRange;
} NEXUS_TimebaseVdecSourceSettings;

/**
Summary:
Settings for the HDDVI source type
**/
typedef struct NEXUS_TimebaseHddviSourceSettings
{
    unsigned index; /* index of the HD-DVI input */
    unsigned vertSyncClock; /* Vertical sync clock measured from HD-DVI. This is the number of clock cycles between vsync's.
                               Set to 0 to ignore/bypass this value. */
    NEXUS_VideoFormat format;
    NEXUS_VideoFrameRate frameRate; /* deprecated. use NEXUS_Timebase_SetHdDviFrameRate instead. */
    NEXUS_TimebaseTrackRange trackRange;
} NEXUS_TimebaseHddviSourceSettings;

/**
Summary:
Settings for the 656 source type
**/
typedef struct NEXUS_TimebaseCcir656SourceSettings
{
    unsigned index; /* index of the CCIR 656 input */
    NEXUS_VideoFormat format;
    NEXUS_TimebaseTrackRange trackRange;
} NEXUS_TimebaseCcir656SourceSettings;

/**
Summary:
Settings for the I2S source type
**/
typedef struct NEXUS_TimebaseI2sSourceSettings
{
    unsigned index; /* index of the I2S input */
    unsigned sampleRate; /* in Hz. This must be programmed by the application when it sets NEXUS_I2sSettings.sampleRate. */
    NEXUS_TimebaseTrackRange trackRange;
} NEXUS_TimebaseI2sSourceSettings;

/**
Summary:
Settings to configure a NEXUS_Timebase
**/
typedef struct NEXUS_TimebaseSettings
{
    NEXUS_TimebaseSourceType sourceType;    /* Type of timebase source */

    struct {
        NEXUS_TimebaseFreeRunSourceSettings freeRun; /* valid for sourceType NEXUS_TimebaseSourceType_eFreeRun */
        NEXUS_TimebasePcrSourceSettings pcr;     /* valid for sourceType NEXUS_TimebaseSourceType_ePcr */
        NEXUS_TimebaseVdecSourceSettings vdec;    /* valid for sourceType NEXUS_TimebaseSourceType_eAnalog */
        NEXUS_TimebaseHddviSourceSettings hdDvi;    /* valid for sourceType NEXUS_TimebaseSourceType_eHdDviIn */
        NEXUS_TimebaseCcir656SourceSettings ccir656; /* valid for sourceType NEXUS_TimebaseSource_eCcir656In */
        NEXUS_TimebaseI2sSourceSettings i2s; /* valid for sourceType NEXUS_TimebaseSourceType_eI2sIn */
    } sourceSettings; /* source-type-specific settings */

    /* NOTE: Callbacks are outside substruct because of kernel mode proxy parser */
    NEXUS_CallbackDesc pcrCallback;      /* Called when a new PCR is received. */
    NEXUS_CallbackDesc pcrErrorCallback; /* Called whenever there are two consecutive PCR out-of-range errors (as defined by maxPcrError.
                                            If the callback is set, it will be fired and NEXUS_TimebaseStatus.pcrErrors will accumulate. */
    bool freeze;                         /* Stop loop tracking by freezing the integrator. */
} NEXUS_TimebaseSettings;

/**
Summary:
Update timebase settings
**/
NEXUS_Error NEXUS_Timebase_SetSettings(
    NEXUS_Timebase timebase,
    const NEXUS_TimebaseSettings *pSettings   /* Settings */
    );

/**
Summary:
Get current timebase settings
**/
void NEXUS_Timebase_GetSettings(
    NEXUS_Timebase timebase,
    NEXUS_TimebaseSettings *pSettings    /* [out] Settings */
    );

/**
Summary:
Open a timebase for exclusive use

Description:
NEXUS_Timebase_Open is required for protected clients to exclusively use a timebase.
**/
NEXUS_Timebase NEXUS_Timebase_Open( /* attr{destructor=NEXUS_Timebase_Close}  */
    unsigned index /* Use a specific index or NEXUS_ANY_ID */
    );

/**
Summary:
Close a timebase opened with NEXUS_Timebase_Open
**/
void NEXUS_Timebase_Close(
    NEXUS_Timebase timebase
    );

/**
Summary:
Get default settings for a timebase.
**/
void NEXUS_Timebase_GetDefaultSettings(
    NEXUS_TimebaseSettings *pSettings
    );

/**
Summary:
Current status of the timebase.

Description:
This is chiefly used for diagnostics.
Call NEXUS_Timebase_GetStatus to get the status.
Call NEXUS_Timebase_ResetStatus to reset any acculumated values.
**/
typedef struct NEXUS_TimebaseStatus
{
    unsigned timebase;    /* Hardware DPCR block */
    uint32_t lastValue;   /* Last PCR value received.
                             For DSS, this is the full 32 bits of PCR (27 MHz units).
                             For MPEG2 TS, this is the most significant 32 bits of the 42 bit PCR (that is, 45 KHz units).
                             Throughout the system, whenever the "PCR" is referred to, it is this 32 bit value. */
    uint32_t lastValueLo; /* Least significant 10 bits of the last 42 bit PCR for MPEG2 TS. Unused for DSS. */
    int32_t lastError;  /* error between last PCR and STC at time of receipt of last PCR */
    unsigned pcrCount;   /* number of PCR's received since call to NEXUS_Timebase_ResetStatus */
    unsigned pcrErrors;  /* number of PCR errors since call to NEXUS_Timebase_ResetStatus. This only accumulates if NEXUS_TimebaseSettings.sourceSettings.pcr.pcrError is set. */
    bool pcrValid;       /* valid PCR was captured from the stream since call to NEXUS_Timebase_ResetStatus */
    NEXUS_TimebaseSourceType sourceType; /* current source type this timebase is tracking */
} NEXUS_TimebaseStatus;

/**
Summary:
Get current NEXUS_TimebaseStatus
**/
NEXUS_Error NEXUS_Timebase_GetStatus(
    NEXUS_Timebase timebase,
    NEXUS_TimebaseStatus *pStatus        /* [out] current status */
    );

/**
Summary:
Reset any accumulated values in NEXUS_TimebaseStatus
**/
NEXUS_Error NEXUS_Timebase_ResetStatus(
    NEXUS_Timebase timebase
    );

/* 
Atomically set NEXUS_TimebaseSettings.sourceSettings.hdDvi
This function is automatically called by the HdmiInput Interface. 
A non-zero value will override anything set by NEXUS_TimebaseSettings.
*/
NEXUS_Error NEXUS_Timebase_SetHdDviFrameRate(
    NEXUS_Timebase timebase,
    NEXUS_VideoFrameRate frameRate
    );

/* 
Atomically set NEXUS_TimebaseSettings.sourceSettings.vdec
This function is automatically called by the AnalogVideoDecoder interface.
A non-zero value will override anything set by NEXUS_TimebaseSettings.
*/
NEXUS_Error NEXUS_Timebase_SetVdecFrameRate(
    NEXUS_Timebase timebase,
    NEXUS_VideoFrameRate frameRate
    );
    
/**
Summary:
Get index of hardware DPCR block (equivalenet to NEXUS_TimebaseStatus.timebase)
**/
NEXUS_Error NEXUS_Timebase_GetIndex(
    NEXUS_Timebase timebase,
    unsigned *pIndex  /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif

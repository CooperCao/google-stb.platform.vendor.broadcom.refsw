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
#ifndef NEXUS_STCCHANNEL_TYPES_H__
#define NEXUS_STCCHANNEL_TYPES_H__

#include "nexus_types.h"
#include "nexus_pid_channel.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Handle for an STC Channel

Description:
An STC (system time clock) channel provides basic synchronization between audio and video decoders.
This synchronization is called TSM (time stamp management).
In TSM mode, decoders will only present pictures or audio frames when the data's PTS matches the current STC, within
a certain threshold.

See Nexus_Usage.doc for more usage information.
See NEXUS_SyncChannel for the high-precision lipsync Interface.

An StcChannel manages a PCR_OFFSET block and related software.
**/
typedef struct NEXUS_StcChannel *NEXUS_StcChannelHandle;

/*
Summary:
TSM modes used in NEXUS_StcChannelSettings
*/
typedef enum NEXUS_StcChannelMode
{
    NEXUS_StcChannelMode_ePcr,  /* Live TSM.
                                    STC values are derived from the stream's PCR using the DPCR block.
                                    The PCR_OFFSET block pipelines the offsets in the ITB to handle PCR discontinuities.
                                    CDB/ITB buffer levels are determined by relative PCR/PTS muxing. */
    NEXUS_StcChannelMode_eAuto, /* Playback TSM.
                                    Decoders report PTS values and the StcChannel sets the STC.
                                    The PCR_OFFSET block is not used and offsets are not pipelined.
                                    CDB/ITB buffer levels are determined by the first decoder to fill its buffer
                                    and by relative audio/video muxing. */
    NEXUS_StcChannelMode_eHost, /* Playback TSM where the application is responsible for setting the STC.
                                    This has all the same characteristics as eAuto, except the PTS interrupts
                                    from the decoder do not set the STC. */
    NEXUS_StcChannelMode_eMax
} NEXUS_StcChannelMode;

/*
Summary:
This describes the behavior for STC seeding during PVR when StcChannelMode is set to eAuto
*/
typedef enum NEXUS_StcChannelAutoModeBehavior
{
    NEXUS_StcChannelAutoModeBehavior_eFirstAvailable, /* The STC will be driven by either the video or audio PTS, depending on stream muxing and error conditions. */
    NEXUS_StcChannelAutoModeBehavior_eVideoMaster,    /* The video PTS will always drive the STC.  Audio errors will be ignored. */
    NEXUS_StcChannelAutoModeBehavior_eAudioMaster,    /* The audio PTS will always drive the STC.  Video errors will be ignored. */
    NEXUS_StcChannelAutoModeBehavior_eMax
} NEXUS_StcChannelAutoModeBehavior;

/*
Summary:
This controls number of PCR bits send to the downstream modules
*/
typedef enum NEXUS_StcChannel_PcrBits {
    NEXUS_StcChannel_PcrBits_eLegacy,
    NEXUS_StcChannel_PcrBits_eLsb32,
    NEXUS_StcChannel_PcrBits_eMsb32,
    NEXUS_StcChannel_PcrBits_eFull42,
    NEXUS_StcChannel_PcrBits_eMax
} NEXUS_StcChannel_PcrBits;

/*
Summary:
Pcr-mode-specific stc channel settings
*/
typedef struct NEXUS_StcChannelPcrModeSettings
{
    NEXUS_PidChannelHandle pidChannel; /* The pid channel for the PCR. This determines the transport type as well. */
    unsigned offsetThreshold;          /* Threshold for filtering PCR change notifications to decoders, in PTS units. */
    unsigned maxPcrError;              /* Maximum absolute PCR error, in PTS units. See NEXUS_TimebasePcrSourceSettings for more detail. */
    bool disableTimestampCorrection;   /* deprecated. only applies to 65nm. */
    bool disableJitterAdjustment;      /* PCR offset jitter adjustment */
} NEXUS_StcChannelPcrModeSettings;

/*
Summary:
Auto-mode-specific stc channel settings
*/
typedef struct NEXUS_StcChannelAutoModeSettings
{
    NEXUS_TransportType transportType;         /* There is no pcr for auto mode, but transport type is needed for correct STC management. */
    NEXUS_StcChannelAutoModeBehavior behavior; /* controls how the STC is seeded in Auto mode */
    unsigned offsetThreshold;          /* Threshold for filtering STC change notifications to decoders, in PTS units. */
} NEXUS_StcChannelAutoModeSettings;

/*
Summary:
Host-mode-specific stc channel settings
*/
typedef struct NEXUS_StcChannelHostModeSettings
{
    NEXUS_TransportType transportType; /* There is no pcr for host mode, but transport type is needed for correct STC management. */
    unsigned offsetThreshold;          /* Threshold for filtering STC change notifications to decoders, in PTS units. */
} NEXUS_StcChannelHostModeSettings;

/*
Summary:
Specifies the way in which STC channel will handle decoder underflow during
non-real-time transcode.
*/
typedef enum NEXUS_StcChannelUnderflowHandling
{
    NEXUS_StcChannelUnderflowHandling_eDefault,            /* any underflow resulting in an STC stall is declared a gap and requires zero-filling */
    NEXUS_StcChannelUnderflowHandling_eAllowProducerPause, /* only underflows that result in the opposing decoder being full and stalled are declared gaps and zero-filled */
    NEXUS_StcChannelUnderflowHandling_eMax
} NEXUS_StcChannelUnderflowHandling;

/*
Summary:
Settings passed into NEXUS_StcChannel_Open
*/
typedef struct NEXUS_StcChannelSettings
{
    NEXUS_Timebase timebase;       /* The timebase that drives the rate of this StcChannel */
    NEXUS_StcChannelMode mode;     /* Mode of this channel, either PCR-based or host-loaded */
    bool autoConfigTimebase;       /* If true, the STC Channel will automatically program the timebase. This works for typical scenarios like live broadcast or PVR.
                                      You may want to manually program the timebase for modes like IP settop or Mosaic mode. */
    int stcIndex;                  /* Index of the HW Serial STC. -1 means use the index passed to NEXUS_StcChannel_Open to select both the HW PCR_OFFSET context and the HW Serial STC.
                                      This is typical when the number of PCR_OFFSET's and STC's are the same. Default is -1. */

    NEXUS_StcChannel_PcrBits pcrBits; /* select PCR bits delivered to the downstream modules */

    NEXUS_StcChannelUnderflowHandling underflowHandling; /* select how underflows are handled in NRT mode */

    struct
    {
        NEXUS_StcChannelPcrModeSettings pcr;
        NEXUS_StcChannelAutoModeSettings Auto;
        NEXUS_StcChannelHostModeSettings host;
    } modeSettings;
} NEXUS_StcChannelSettings;

/*
Summary:
STC Channel status
*/
typedef struct NEXUS_StcChannelStatus
{
    NEXUS_StcChannelMode mode; /* The mode that the STC channel is currently in. This can vary from NEXUS_StcChannelSettings.mode
                                  because of ASTM. */
    uint32_t stc;
    bool stcValid;
} NEXUS_StcChannelStatus;

#ifdef __cplusplus
}
#endif

#endif

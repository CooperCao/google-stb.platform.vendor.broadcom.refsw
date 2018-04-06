/***************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 ***************************************************************************/

/***************************************************************************
Overview

***************************************************************************/

#ifndef BXPT_TSMUX_H__
#define BXPT_TSMUX_H__

#include "bxpt.h"
#include "bxpt_playback.h"
#include "bxpt_pcr_offset.h"

/* SW7425-4698: Max pacing speed was 128; limit to 8 for now */
/* SW7425-5730: On 40nm chips, limit to 4 for now. It will be done internally
** by the PI, since the 28nm chips do NOT have the hw bug.
*/
#define BXPT_TSMUX_MAX_PACING_SPEED 8

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Handle for accessing the TsMux API via a channel. Users should not directly
access the contents of the structure.
****************************************************************************/
typedef struct BXPT_P_TsMuxHandle *BXPT_TsMux_Handle;

/***************************************************************************
Summary:
Status bits for the TsMux channel.
****************************************************************************/
typedef struct BXPT_TsMux_Status
{
    uint64_t uiSTC;     /* 42-bit value in 27 Mhz */
    uint32_t uiESCR;    /* 32-bit value in 27Mhz; */
}
BXPT_TsMux_Status;

/***************************************************************************
Summary:
Settings for the TsMux.
****************************************************************************/
typedef struct BXPT_TsMux_Settings
{
    unsigned uiMuxDelay;    /* in mSec */

    bool bAFAPMode;       /* As fast as possible mode. */
    struct
    {
        /* If bAFAPMode true, the following parameters are required.
        They affect all playback channels involved in the mux instance. */
        bool bEnablePacingPause;     /* Defaults to true. Currently not implemented */
        unsigned uiPacingSpeed;      /* Multiplier applied to the pacing counter increment.
                                    This must be a power of 2, ie 1, 2, 4 ... up to
                                    BXPT_TSMUX_MAX_PACING_SPEED. Defaults to 8. */
        unsigned uiPacingCounter;   /* Current value of the pacing counter. */
    }
    AFAPSettings;

    /*
    ** The PCR offset channel to be used in TS muxing.
    */
    BXPT_PcrOffset_Handle hPcrOffset;
}
BXPT_TsMux_Settings;

/***************************************************************************
Summary:
Return the TsMux default settings.

Description:
Return the TsMux default settings.

Returns:
    void
****************************************************************************/
void BXPT_TsMux_GetDefaultSettings(
    BXPT_TsMux_Settings *Defaults   /* [out] The defaults */
    );

/***************************************************************************
Summary:
Create a new TsMux instance.

Description:
This just creates the empty TsMux framework. No playback channels are
allocated or configured at this time.

Returns:
    BERR_SUCCESS                - TsMux instance created.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_TsMux_Create(
    BXPT_Handle hXpt,                        /* [in] Handle for this transport */
    BXPT_TsMux_Handle *hTsMux                /* [out] Handle for opened record channel */
    );

/***************************************************************************
Summary:
Destroy the TsMux instance.

Description:
Undo the bindings between the playback channels and PCR Offsets that were
created by the calls to BXPT_TsMux_AddPlaybackToTsMux() and
BXPT_TsMux_AddPcrOffsetToTsMux() Release any other resources held by the TsMux
instance.

Returns:
    void
****************************************************************************/
void BXPT_TsMux_Destroy(
    BXPT_TsMux_Handle hTsMux     /* [in] Handle for the TsMux to destroy */
    );

/***************************************************************************
Summary:
Add a playback channel to be used in TS muxing.

Description:
Add a playback channel to the TS muxer instance. The channel must be already
opened before calling this function. The appropriate binding between this
playback and any playbacks already in the mux instance are done here.

Returns:
    BERR_SUCCESS                - Playback allocated succesfully.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_TsMux_AddPlayback(
    BXPT_TsMux_Handle hTsMux,               /* [in] Handle for the TsMux */
    BXPT_Playback_Handle PlaybackHandle    /* [in] Handle for allocated playback channel */
    );

/***************************************************************************
Summary:
Remove a playback channel from the TS object.

Description:
Remove a playback channel from the TS muxer instance.

Returns:
    void
****************************************************************************/
void BXPT_TsMux_RemovePlayback(
    BXPT_TsMux_Handle hTsMux,               /* [in] Handle for the TsMux */
    BXPT_Playback_Handle PlaybackHandle    /* [in] Handle for allocated playback channel */
    );

/***************************************************************************
Summary:
Get the current channel settings.

Description:
The current channel settings are read from the hardware cores and returned in
the Settings structure.

Returns:
    void
****************************************************************************/
void BXPT_TsMux_GetSettings(
    BXPT_TsMux_Handle hTsMux,               /* [in] Handle for the TsMux */
    BXPT_TsMux_Settings *Settings   /* [out] The current settings  */
    );

/***************************************************************************
Summary:
Set the TsMux instance parameters.

Description:
Set the TsMux instance parameters.

Returns:
    BERR_SUCCESS                - New settings are now in effect.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_TsMux_SetSettings(
    BXPT_TsMux_Handle hTsMux,               /* [in] Handle for the TsMux */
    const BXPT_TsMux_Settings *Settings /* [in] New settings to use */
    );

/***************************************************************************
Summary:
Get the current status.

Description:
Get the current status.

Returns:
    void
****************************************************************************/
void BXPT_TsMux_GetStatus(
    BXPT_TsMux_Handle hTsMux,               /* [in] Handle for the TsMux */
    BXPT_TsMux_Status *Status       /* [out] Channel status. */
    );

/***************************************************************************
Summary:
Configure a playback linked-list descriptor for TS muxing.

Description:
Initialize the contents of a playback descriptor. The caller passes in the
starting address of the buffer that the descriptor will point to, along with
the length of that buffer.

Returns:
    void
****************************************************************************/
void BXPT_Playback_ConfigTsMuxDesc(
    BXPT_Playback_Handle hPb,   /* [in] Handle for the playback channel */
    BXPT_PvrDescriptor *Desc,              /* [in] Descriptor to be configured */
    const BAVC_TsMux_DescConfig *Config     /* [in] Data to be loaded */
    );

/***************************************************************************
Summary:
Return the muxing flags from the given descriptor.

Returns:
    void
****************************************************************************/
void BXPT_Tsmux_GetDescConfig(
    const BXPT_PvrDescriptor *Desc,
    BAVC_TsMux_DescConfig *Config     /* [out] muxing flags unpacked from the descriptor */
    );

/* Private functions. Should not be called directly by user code. */
void BXPT_TsMux_P_ResetBandPauseMap(
    BXPT_Handle hXpt
    );

void BXPT_TsMux_P_ResetPacingPauseMap(
    BXPT_Handle hXpt
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BXPT_TSMUX_H__ */

/* end of file */

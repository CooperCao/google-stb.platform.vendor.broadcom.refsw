/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/*= Module Overview *********************************************************
***************************************************************************/

#ifndef BXPT_REMUX_H__
#define BXPT_REMUX_H__

#include "bxpt.h"
#include "bxpt_playback.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Handle for accessing the remux API via a channel. Users should not directly
access the contents of the structure.
****************************************************************************/
typedef struct BXPT_P_RemuxHandle *BXPT_Remux_Handle;

/***************************************************************************
Summary:
Defines the two channels inside the remux block.
****************************************************************************/
typedef enum BXPT_RemuxInput
{
    BXPT_RemuxInput_eA,
    BXPT_RemuxInput_eB
}
BXPT_RemuxInput;

/***************************************************************************
Summary:
Defines the types of sources a remux channel can use.
****************************************************************************/
typedef enum BXPT_RemuxSource
{
    BXPT_RemuxSource_ePidChannelNonDecrypt, /* Parsed Non-Decrypted by PID Channel  */
    BXPT_RemuxSource_ePidChannelDecrypted   /* Parsed Decrypted by PID Channel */
}
BXPT_RemuxSource;

/***************************************************************************
Selects the clock source for the remux channel. The API will return an error
if the particular clock source selected is not supported on the chip.
****************************************************************************/
typedef enum BXPT_RemuxClock
{
    BXPT_RemuxClock_e81Mhz          = 0,    /* 81 MHz */
    BXPT_RemuxClock_e54Mhz          = 1,    /* 54 MHz */
    BXPT_RemuxClock_e40_5Mhz        = 2,    /* 40.5 MHz */
    BXPT_RemuxClock_e20_25Mhz       = 3,    /* 20.25 MHz */
    BXPT_RemuxClock_e27Mhz_VCXO_A   = 4,    /* 27 MHz, from VCXO A */
    BXPT_RemuxClock_eIb0            = 8,    /* Input band 0 clock. */
    BXPT_RemuxClock_eIb1            = 9,    /* Input band 1 clock. */
    BXPT_RemuxClock_eIb2            = 10,   /* Input band 2 clock. */
    BXPT_RemuxClock_eIb3            = 11,   /* Input band 3 clock. */
    BXPT_RemuxClock_eIb4            = 12,
    BXPT_RemuxClock_eIb5            = 13,
    BXPT_RemuxClock_eIb6            = 14,
    BXPT_RemuxClock_eIb7            = 15,
    BXPT_RemuxClock_eIb8            = 16,
    BXPT_RemuxClock_eIb9            = 17,
    BXPT_RemuxClock_eIb10           = 18,
    BXPT_RemuxClock_eIb11           = 19,
    BXPT_RemuxClock_eIb12           = 20,
    BXPT_RemuxClock_eIbMax                  /* Marks the end of the supported values. */
}
BXPT_RemuxClock;
/***************************************************************************
Summary:
Selects the buffer level at which the Remux block will assert a HOLD signal
to the playback channel that is feeding it (if a playback is being used).
****************************************************************************/
typedef enum BXPT_RemuxHoldLevel
{
    BXPT_RemuxHoldLevel_eNever = 0,     /* Never assert hold. */
    BXPT_RemuxHoldLevel_e1_4Full = 1,   /* Assert when 1/4 full. */
    BXPT_RemuxHoldLevel_e1_2Full = 2,   /* Assert when 1/2 full. */
    BXPT_RemuxHoldLevel_e3_4Full = 3    /* Assert when 1/4 full. */
}
BXPT_RemuxHoldLevel;

/***************************************************************************
Summary:
Selects one of several timebases to be used by the timestamp counter.
****************************************************************************/
typedef enum BXPT_RemuxTimeBase
{
    BXPT_RemuxTimeBase_e27Mhz = 0,      /* Free running 27 Mhz source. */
    BXPT_RemuxTimeBase_ePcr0  = 1,      /* Timebase locked to PCR 0 */
    BXPT_RemuxTimeBase_ePcr1  = 2       /* Timebase locked to PCR 1 */
}
BXPT_RemuxTimeBase;

/***************************************************************************
Summary:
Defines the types of pass-though mode for a remux input.
****************************************************************************/
typedef enum BXPT_Remux_PassMode
{
    BXPT_Remux_PassMode_eNormal = 0,
    BXPT_Remux_PassMode_eAllPass = 1
}
BXPT_Remux_PassMode;

/***************************************************************************
Summary:
Settings for the remux channels. These values can be passed in when the
channel is opened, and will be used as the new default settings.
****************************************************************************/
typedef struct BXPT_Remux_ChannelSettings
{
    BXPT_RemuxClock OutputClock;        /* Clock to be used to output the RMX serial data. */

    /*
    ** Programmable delay output packets. This value is specified in 108MHz
    ** clock ticks. Programming this field to 0 specifies no gap between output packets
    */
    unsigned int PacketDelayCount;

    bool PauseEn;   /* NOTE: This member has been deprecated. Please use BXPT_Remux_PauseEnable(). */

    /*
    ** Settings for the remux physical interface. From the Remux, it is possible
    ** to have both serial and parallel output enabled simultaneously. Top-level
    ** routing in the chip may not support that, however.
    */
    bool ParallelEn;            /* Enable parallel output. */

    /* These settings are only used when parallel output is enabled */
    bool ParallelInvertClk;     /* Data will be output on falling edge if true */
    bool ParallelInvertSync;    /* Active low sync when true */

    /* Settings for serial output mode. */
    bool InvertClk;     /* Data will be output on falling edge if true */
    bool InvertSync;    /* Active low sync when true */
    bool ByteSync;      /* Sync is active for entire byte of a packet when true */

#if BXPT_HAS_REMUX_PID_REMAPPING

    BXPT_RemuxHoldLevel HoldLevel;      /* Buffer level at which the RMX buffer triggers a hold to the playback */

    BXPT_RemuxTimeBase RmxATimeBase;
    BXPT_RemuxTimeBase RmxBTimeBase;

    /*
    ** Select the timebase to be used for remux pacing. Set UsePcrTimeBase to
    ** false if the free running 27 MHz clock is to be used. Set UsePcrTimeBase
    ** to true to use a PCR timebase; WhichPcrToUse specifies the PCR block to
    ** use in that case.
    */
    bool UsePcrTimeBase;
    unsigned int WhichPcrToUse; /* Which PCR module, if UsePcrTimeBase == true */
    BXPT_ParserTimestampMode TimestampMode;

    /*
    ** When PCR correction is desired, the parser bands carrying the PIDs
    ** must be set.
    */
    unsigned RmxBandA;
    unsigned RmxBandB;

#endif
    /* Insert NULL packets in the output, when the selected output rate is
    ** higher than the input rate. By default, this is enabled (true).
    */
    bool InsertNullPackets;

#if BXPT_HAS_RMX_NULL_THRESHOLD
    /*
    ** When NULL packet insertion is enabled (above), the hardware will do so
    ** after the number of clocks specified below have elapsed with further packets
    ** from the input. This threshold is in 108MHz clock ticks. Default is 16 ticks.
    */
    unsigned NullPacketInsertionThreshold;
#endif
}
BXPT_Remux_ChannelSettings;

/***************************************************************************
Summary:
Return the number of remux channels.

Description:
For the given transport core, return the number of remux channels that is
supported.

Returns:
    BERR_SUCCESS                - Retrieved address from hardware.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Remux_GetTotalChannels(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    unsigned int *TotalChannels     /* [out] The number of remux channels. */
    );

/***************************************************************************
Summary:
Return the remux channel default settings.

Description:
Each remux channel has a set of default configuration values. This
function retrieves these values and places them in a structure that can be
used to change the defaults when the remux channel is opened.

Returns:
    BERR_SUCCESS                - Retrieved remux defaults.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Remux_GetChannelDefaultSettings(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    unsigned int ChannelNo,         /* [in] Which channel to get defaults from. */
    BXPT_Remux_ChannelSettings *ChannelSettings /* [out] The defaults */
    );

/***************************************************************************
Summary:
Open a given remux channel.

Description:
Reset the given remux channel, configure it using the given defaults, then
return a handle to access that channel.

Returns:
    BERR_SUCCESS                - Remux channel opened.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Remux_OpenChannel(
    BXPT_Handle hXpt,                       /* [in] Handle for this transport */
    BXPT_Remux_Handle *RemuxHandle,         /* [out] Handle for opened remux channel */
    unsigned int ChannelNo,                     /* [in] Which channel to open. */
    BXPT_Remux_ChannelSettings *ChannelSettings /* [in] The defaults to use */
    );

/***************************************************************************
Summary:
Close a given remux channel.

Description:
Shutdown a given remux channel, disable any interrupts from it.

Returns:
    void
****************************************************************************/
void BXPT_Remux_CloseChannel(
    BXPT_Remux_Handle RemuxHandle   /* [in] Handle for the channel to close*/
    );

/***************************************************************************
Summary:
Start or stop the remultiplexing function.

Description:
Enable or disables remuxing on the given channel. The channel should be
configured before calling this function.

Returns:
    BERR_SUCCESS                - Remuxing started or stopped successfully.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Remux_DoRemux(
    BXPT_Remux_Handle RemuxHandle,  /* [in] Handle for the remux channel */
    bool Enable                 /* [in] Remux on if TRUE, or off if FALSE. */
    );
#if BXPT_SEPARATE_REMUX_IO
#else
/***************************************************************************
Summary:
Reload the timestamp counters used for PCR correction.

Description:
PCR jitter correction, if enabled, requires the use of arrival timestamps.
This API commands a load/reload of the those timestamps, and should be called
during a channel change.

Returns:
    void
****************************************************************************/
void BXPT_Remux_ReloadTimestamps(
    BXPT_Remux_Handle hRmx
    );
#endif

/***************************************************************************
Summary:
Include the given PID channel in the remux input.

Description:
Add a PID channel to the given remux.

Returns:
    BERR_SUCCESS                - PID channel added successfully.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Remux_AddPidChannelToRemux(
    BXPT_Remux_Handle RemuxHandle,  /* [in] Handle for the remux channel */
    BXPT_RemuxInput RemuxInput,         /* [in] Which remux input */
    unsigned int PidChannelNum          /* [in] Which PID channel. */
    );

/***************************************************************************
Summary:
Include the given PID channel in the remux input, using the R-Pipe feed.

Description:
Add a PID channel to the given remux.

Returns:
    BERR_SUCCESS                - PID channel added successfully.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Remux_AddRPipePidChannelToRemux(
    BXPT_Remux_Handle hRmx,             /* [in] Handle for the remux channel */
    BXPT_RemuxInput RemuxInput,         /* [in] Which remux input */
    unsigned int PidChannelNum          /* [in] Which PID channel. */
    );

/***************************************************************************
Summary:
Remove the given PID channel in the remux input.

Description:
Remove a PID channel from the given remux. The data carried on this PID
channel will not be used in the remuxing.

Returns:
    BERR_SUCCESS                - PID channel removed successfully.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Remux_RemovePidChannelFromRemux(
    BXPT_Remux_Handle RemuxHandle,  /* [in] Handle for the remux channel */
    BXPT_RemuxInput RemuxInput,     /* [in] Which remux input */
    unsigned int PidChannelNum          /* [in] Which PID channel. */
    );

/***************************************************************************
Summary:
Is the given PID channel enabled for remuxing?

Description:
Determines if a given PID channel is enabled for remuxing or not.

Returns:
    BERR_SUCCESS                - Got PID channel's status successfully.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Remux_GetPidChannel(
    BXPT_Remux_Handle RemuxHandle,  /* [in] Handle for the remux channel */
    BXPT_RemuxInput RemuxInput,             /* [in] Which remux input */
    unsigned int PidChannelNum,             /* [in] Which PID channel. */
    bool *Enable                        /* [out] Where to put channel status. */
    );

/***************************************************************************
Summary:
Removes all PID channels on the remux channel.

Description:
Removes all PID channels in the given block and remux channel from the
remuxing session.

Returns:
    BERR_SUCCESS                - PID channels removed uccessfully.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Remux_RemoveAllPidChannel(
    BXPT_Remux_Handle RemuxHandle,  /* [in] Handle for the remux channel */
    BXPT_RemuxInput RemuxInput          /* [in] Which remux input */
    );

/***************************************************************************
Summary:
Enable or disable the bypass mode for the remux.

Description:
Enables or disables the RMX bypass mode. In bypass mode, the selected data
is output directly without any buffering, null packet insertion, pcr
correction or packet substitution. Bypass mode must be used to output
non-transport data such as PES or ES. In bypass mode, the output clock is
gated off when there is no data to output.

Returns:
    BERR_SUCCESS                - All bypass mode successfully changed.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Remux_SetBypassMode(
    BXPT_Remux_Handle hRmx,         /* [in] Handle for the remux channel */
    bool Enable
    );

#if BXPT_HAS_REMUX_PCR_OFFSET
/***************************************************************************
Summary:
Add a fixed offset to the PCR values output from remux.

Description:
When enabled, FixedPcrOffset will be added to the PCR values in the output
stream, after PCR jitter adjustment has been done.

Returns:
    BERR_SUCCESS                - Fixed offset has been set.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Remux_AddPcrOffset(
    BXPT_Remux_Handle hRmx,     /* [in] Handle for the remux channel */
    bool AddOffsetEn,           /* [in] Enable/disable the PCR correction */
    uint32_t FixedPcrOffset         /* [in] The PCR correction offset, if enabled */
    );

/***************************************************************************
Summary:
Enable or disable PCR jitter adjustment in the remux.

Description:
Controls PCR jitter adjustment for live or playback streams. The remux must
not be in bypass mode to use jitter adjustment. Timestamps must also be
enabled in the playback (for such streams).

Returns:
    void
****************************************************************************/
void BXPT_Remux_SetPcrJitterAdj(
    BXPT_Remux_Handle hRmx,     /* [in] Handle for the remux channel */
    bool ForPlayback,       /* [in] true = control adjustment for playback, false = control for live */
    bool EnableAdjust           /* [in] Enable/disable the PCR jitter adjust */
    );
#endif
#if BXPT_HAS_REMUX_PID_REMAPPING

/***************************************************************************
Summary:
Start a PID remap.

Description:
Enable a given PID mapping entry.

Returns:
    BERR_SUCCESS                - Remap entry enabled.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Remux_EnablePidMap(
    BXPT_Remux_Handle RemuxHandle,  /* [in] Handle for the remux channel */
    unsigned int PmIndex                /* [in] Which PID map. */
    );

/***************************************************************************
Summary:
Stop a PID remap.

Description:
Disable a given PID mapping entry.

Returns:
    BERR_SUCCESS                - Remap entry disabled.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Remux_DisablePidMap(
    BXPT_Remux_Handle RemuxHandle,  /* [in] Handle for the remux channel */
    unsigned int PmIndex                /* [in] Which PID map. */
    );

/***************************************************************************
Summary:
Allocate a PID remap entry.

Description:
Obtain an entry from the PID remap table, and configure associated PID values.
In previous chips, the argument "ParserBand" was "RemuxInput". For this chip,
the usage has changed. ParserBand must be the particular parser band that the
PID to be remapped is on.

Returns:
    BERR_SUCCESS                - Remap entry allocated and setup.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Remux_AllocPidMap(
    BXPT_Remux_Handle hRmx,         /* [in] Handle for the remux channel */
    unsigned int IncomingPid,               /* [in] PID that will be remapped. */
    unsigned int OutgoingPid,               /* [in] New value for the remapped PID. */
    unsigned ParserBand,            /* [in] Which parser band has the PID to remapped */
    unsigned int *PmIndex                   /* [out] Which PID map entry was allocated to enable. */
    );

/***************************************************************************
Summary:
Free a PID remap entry.

Description:
Free up an entry from the PID remap table. The entry must have been allocated
previously by a call to BXPT_Remux_AllocPidMap().

Returns:
    BERR_SUCCESS                - Remap entry freed.
    BERR_INVALID_PARAMETER      - Bad input parameter

See Also:
BXPT_Remux_AllocPidMap
****************************************************************************/
BERR_Code BXPT_Remux_FreePidMap(
    BXPT_Remux_Handle RemuxHandle,  /* [in] Handle for the remux channel */
    unsigned int PmIndex                    /* [in] Which PID map entry to free up. */
    );

/***************************************************************************
Summary:
Free all PID remap entries on a remux channel.

Description:
Free up ALL entries from the PID remap table on the given remux channel.

Returns:
    BERR_SUCCESS                - Remap entry freed.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Remux_FreeAllPidMaps(
    BXPT_Remux_Handle RemuxHandle   /* [in] Handle for the remux channel */
    );

/***************************************************************************
Summary:
Load data into a given packet substitution table.

Description:
Copy data into a given packet substitution table in the Remux block. Caller
specifies the starting address in the packet sub table to copy to, and the
number of bytes to copy.

Returns:
    BERR_SUCCESS                - Data copied successfully.
    BERR_INVALID_PARAMETER      - Bad input parameter

See Also:
BXPT_Remux_GetPktSubTable
****************************************************************************/
BERR_Code BXPT_Remux_SetPktSubTable(
    BXPT_Remux_Handle RemuxHandle,  /* [in] Handle for the remux channel */
    unsigned int WhichTable,        /* [in] The remux table to copy into. */
    unsigned int Start,             /* [in] Where in table to start copying. */
    unsigned int Size,              /* [in] Number of bytes to copy. */
    const uint8_t *SubData          /* [in] Bytes to copy. */
    );

/***************************************************************************
Summary:
Get data from a given packet substitution table.

Description:
Copy data out of a given remux packet sub table. User passes in a pointer to
where the data is copied to, along with the offset into the table where the
data should be copied from and the number of bytes to copy.

Returns:
    BERR_SUCCESS                - Data copied out successfully.
    BERR_INVALID_PARAMETER      - Bad input parameter

See Also:
BXPT_Remux_SetPktSubTable
****************************************************************************/
BERR_Code BXPT_Remux_GetPktSubTable(
    BXPT_Remux_Handle RemuxHandle,  /* [in] Handle for the remux channel */
    unsigned int WhichTable,    /* [in] The remux table to copy into. */
    unsigned int Start,         /* [in] Where in table to start copying. */
    unsigned int Size,          /* [in] Number of bytes to copy. */
    uint8_t *SubData        /* [out] Where to copy data to. */
    );

/***************************************************************************
Summary:
Configure packet-based data substitution.

Description:
Put the specified remux PID table entry into packet substitution mode. Caller
specifies which PID should have it's payload substituted, and the number of
payload bytes to be substituted. The caller also selects if the adaptation
field contents should be preserved.

Returns:
    BERR_SUCCESS                - Data copied out successfully.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Remux_SetPktSubPacketMode(
    BXPT_Remux_Handle RemuxHandle,  /* [in] Handle for the remux channel */
    unsigned int WhichTable,    /* [in] The remux table to copy into. */
    unsigned int Pid,           /* [in] The target PID. */
    unsigned int Length,        /* [in] Number of payload bytes to substitute. */
    bool SaveAdaptField         /* [in] Preserve adaptation field? */
    );

/***************************************************************************
Summary:
Configure block-based data substitution.

Description:
Put the specified remux PID table entry into block substitution mode. Caller
specifies which PID should have it's payload substituted, the start and ending
byte offsets in the payload section to be substituted. Note that in this mode,
the payload cannot be automatically saved; the start offset can be set to 3,
which will begin substitution at byte 3 of the packet (where the adaptation
fields starts ).  Note also that setting the start offset to 0 will begin
substitution at byte 0, which is the sync byte.

Returns:
    BERR_SUCCESS                - Data copied out successfully.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Remux_SetPktSubBlockMode(
    BXPT_Remux_Handle RemuxHandle,  /* [in] Handle for the remux channel */
    unsigned int WhichTable,    /* [in] The remux table to copy into. */
    unsigned int Pid,           /* [in] The target PID. */
    unsigned int Start,         /* [in] Starting offset for substitution. */
    unsigned int End            /* [in] Ending offset for substitution. */
    );

/***************************************************************************
Summary:
Enable packet substitution for the given remux.

Description:
Packet substitution for the given PID table is started. The table should be
initialized before calling this function.

Returns:
    BERR_SUCCESS                - Substitution started.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Remux_EnablePktSub(
    BXPT_Remux_Handle RemuxHandle,  /* [in] Handle for the remux channel */
    unsigned int WhichTable             /* [in] Which remux table. */
    );

/***************************************************************************
Summary:
Disable packet substitution for the given remux.

Description:
Packet substitution for the given PID table is stopped.

Returns:
    BERR_SUCCESS                - Substitution stopped.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Remux_DisablePktSub(
    BXPT_Remux_Handle RemuxHandle,  /* [in] Handle for the remux channel */
    unsigned int WhichTable             /* [in] Which remux table. */
    );

/***************************************************************************
Summary:
Force a packet output from the substitution table.

Description:
Inserts the packet residing in the packet substitution table into the output
stream. This packet is inserted at the next time where a null packet would
have otherwise been inserted.

Returns:
    BERR_SUCCESS                - Packet output successfully.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Remux_GeneratePkt(
    BXPT_Remux_Handle RemuxHandle,  /* [in] Handle for the remux channel */
    unsigned int WhichTable             /* [in] Which remux table. */
    );

/***************************************************************************
Summary:
Sets PCR correction related configuration.

Description:
This function sets PCR correction offset and enable/disable PCR correction
logic.

Returns:
    BERR_SUCCESS                - Correction enabled or disabled, as requested.
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code BXPT_Remux_SetPcrCorrection(
    BXPT_Remux_Handle hRmx,     /* [in] Handle for the remux channel */
    bool PcrCorrectEn,          /* [in] Enable/disable the PCR correction */
    uint32_t PcrOffset          /* [in] The PCR correction offset, if enabled */
    );

/***************************************************************************
Summary:
Controls pacing in the playback channel.

Description:
Start, or stop, pacing. When started, the pacing logic will capture the next
timestamp and use it to generate the reference offset.

Returns:
    BERR_SUCCESS                - New pacing mode has been set.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Remux_ConfigPacing(
    BXPT_Remux_Handle hRmx,     /* [in] Handle for the remux channel */
    BXPT_PacingControl Mode                 /* [in] New mode for pacing. */
    );

/***************************************************************************
Summary:
Set the maximum allowable timestamp error used during pacing.

Description:
The pacing logic checks the delta between successive packets. The delta is
compared with MaxTsError to determine if the packet is too early or too
late. If either case is true, the timestamp is adjusted by <See Rajesh>

When Mod-300 timestamps are used, MaxTsError is expressed in 90kHz ticks.
For binary timestamps, MaxTsError is 52734 Hz ticks (or 27MHz/512).

Returns:
    BERR_SUCCESS                - New error bound is set.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Remux_SetPacingErrorBound(
    BXPT_Remux_Handle hRmx,     /* [in] Handle for the remux channel */
    unsigned long MaxTsError        /* [in] Maximum timestamp error. */
    );

/***************************************************************************
Summary:
Put remux into All-Pass mode.

Description:
In All-Pass mode, the remux block will not drop any packets. PID substitution
is still done. All-Pass mode is disabled by default.

Returns:
    BERR_SUCCESS                - All-Pass enabled.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Remux_SetAllPassMode(
    BXPT_Remux_Handle hRmx,         /* [in] Handle for the remux channel */
    bool Enable
    );

#endif
#if defined(BXPT_HAS_RSBUF) && defined(BXPT_HAS_XCBUF)

/***************************************************************************
Summary:
Set the Banddata rate for corresponding Remux XC buffer and parserband.

Description:
That API would take a client handle and the ID of the parser band. The
XPT PI would then walk backwards through the pipeline, increasing only
the bitrates for the XC and RS buffers that it needs to.

Returns:
    void.
****************************************************************************/

BERR_Code BXPT_Remux_SetRSXCDataRate(
    BXPT_Remux_Handle hRmx,     /* [in] Handle for Rave */
    BXPT_ParserType ParserType, /* [in] Input band or playback */
    unsigned BandNum,           /* [in] Which Input band or playback parser to configure */
    unsigned long Rate,         /* [in] Max rate in bps */
    unsigned PacketLen          /* [in] size of mpeg packet */
    );

#endif


/***************************************************************************
Summary:
Configure hardware to pause output when/if the RMX pause pin is asserted.

Returns:
    void.
****************************************************************************/
#if BXPT_HAS_REMUX_PAUSE
void BXPT_Remux_PauseEnable(
    BXPT_Remux_Handle hRmx,
    bool UsePauseSignal
    );
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BXPT_REMUX_H__ */

/* end of file */

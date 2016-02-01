/***************************************************************************
 *     (c)2003-2012 Broadcom Corporation
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
 * Porting interface code for the packet substitution section of the
 * data transport core.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef NEXUS_PACKETSUB_H__
#define NEXUS_PACKETSUB_H__

#include "nexus_pid_channel.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Handle for packet substition (PSUB) channel returned by NEXUS_PacketSub_Open

Be aware that "packet substition" does not substitute data packets. It inserts
new data packets without replacing old data packets.
By default PSUB inserts those new packets into bandwidth openings created
by dropping NULL packets. So, it is a bandwidth substition, not a data substitution.
If you set forcedOutput=true, then there is no substitution at all, only unconditional insertion.
****************************************************************************/
typedef struct NEXUS_PacketSub *NEXUS_PacketSubHandle;

/***************************************************************************
Summary:
Settings used in NEXUS_PacketSub_Open
****************************************************************************/
typedef struct NEXUS_PacketSubOpenSettings
{
    unsigned fifoSize;       /* Size of buffer for adding data to be inserted */
    NEXUS_HeapHandle heap;   /* Heap from which to allocate the buffer */
    unsigned numDescriptors; /* one descriptor is needed per pending WriteComplete calls */
} NEXUS_PacketSubOpenSettings;

/***************************************************************************
Summary:
Get default settings before calling NEXUS_PacketSub_Open
****************************************************************************/
void NEXUS_PacketSub_GetDefaultOpenSettings(
    NEXUS_PacketSubOpenSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Open a packet substitution channel
****************************************************************************/
NEXUS_PacketSubHandle NEXUS_PacketSub_Open( /* attr{destructor=NEXUS_PacketSub_Close} */
    unsigned index,
    const NEXUS_PacketSubOpenSettings *pSettings /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
Close a packet substitution channel
****************************************************************************/
void NEXUS_PacketSub_Close(
    NEXUS_PacketSubHandle packetSub
    );

/***************************************************************************
Summary:
Start packet substitution on this channel

Description:
All data must be added to the buffer using NEXUS_PacketSub_GetBuffer and NEXUS_PacketSub_WriteComplete before starting.
You cannot call NEXUS_PacketSub_GetBuffer and NEXUS_PacketSub_WriteComplete after starting.
****************************************************************************/
NEXUS_Error NEXUS_PacketSub_Start(
    NEXUS_PacketSubHandle packetSub
    );

/***************************************************************************
Summary:
Stop a packet substitution

Description:
After calling stop, the buffer is not cleared. If you want to clear the buffer and
add new data, call NEXUS_PacketSub_Flush.
****************************************************************************/
void NEXUS_PacketSub_Stop(
    NEXUS_PacketSubHandle packetSub
    );

/***************************************************************************
Summary:
Settings used in NEXUS_PacketSub_SetSettings
****************************************************************************/
typedef struct NEXUS_PacketSubSettings
{
    NEXUS_PidChannelHandle pidChannel; /* Pid channel that data will be inserted into. This does not imply that data will be substituted 
                                        on this channel. This must be set before call to NEXUS_PacketSub_Start. Insertion into playback PID 
                                        channels is supported on some, but not all, platforms. */

    bool highPriorityDma; /* Prioritize between PSUB channels on the same pid channel. High priority channels will insert before low priority. */

    /* The following 3 settings are listed in priority: If forcedInsertion is true, fullRateOutput and outputRate is 
    ignored. If fullRateOutput is true, outputRate is ignored. */
    bool forcedInsertion; /* Insert a packet between two existing packets, rather than replacing packets dropped by the PID parser. 
                             More packets go out than went in. Not supported on all chips. 
                             If true, fullRateOutput and outputRate are ignored. */
    bool fullRateOutput;  /* Insert a packet at every available slot in the stream. 
                             If true, outputRate is ignored. */
    unsigned outputRate;  /* The rate at which a single packet should be inserted, in bits/second.
                             If you specify 188*8*2, then PSUB will insert two packets per second.
                             If you have 16 PMT packets that you want inserted once every second, you should specify 16*188*8. */

    bool loop;            /* continually loop the data into the PSUB channel. only one WriteComplete is allowed. */

    NEXUS_CallbackDesc dataCallback; /* A descriptor has completed. You can call GetBuffer and submit more data. */
    NEXUS_CallbackDesc finished; /* All data has been fed into HW. Corresponds to NEXUS_PacketSubStatus.finished. */
} NEXUS_PacketSubSettings;

/***************************************************************************
Summary:
Set new settings
****************************************************************************/
NEXUS_Error NEXUS_PacketSub_SetSettings(
    NEXUS_PacketSubHandle packetSub,
    const NEXUS_PacketSubSettings *pSettings
    );

/***************************************************************************
Summary:
Get current settings
****************************************************************************/
void NEXUS_PacketSub_GetSettings(
    NEXUS_PacketSubHandle packetSub,
    NEXUS_PacketSubSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Status
****************************************************************************/
typedef struct NEXUS_PacketSubStatus
{
    bool finished; /* set true after all data has been fed into HW. data may not have been fed into stream yet.
                      this will never be true if NEXUS_PacketSubSettings.loop is true. */
    bool busy;     /* set true if data is being fed into stream. when finished is true and busy is false, then
                      all the data has been read into HW and fed into the stream. */
} NEXUS_PacketSubStatus;

/***************************************************************************
Summary:
Get status
****************************************************************************/
NEXUS_Error NEXUS_PacketSub_GetStatus(
    NEXUS_PacketSubHandle packetSub,
    NEXUS_PacketSubStatus *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Pause or unpause
****************************************************************************/
NEXUS_Error NEXUS_PacketSub_SetPause(
    NEXUS_PacketSubHandle packetSub,
    bool paused
    );

/***************************************************************************
Summary:
Get buffer for adding data to be inserted

Description:
The underlying hardware is descriptor-based, which supports scatter-gather usage.
However, because most usage of packet substitution is simple, the Nexus API was designed to be like most
of the other data feed API's in Nexus (using GetBuffer/ReadComplete).

All data must be added before calling NEXUS_PacketSub_Start. Because of this, the underlying implementation is not
actually a ring buffer.
If you want the data to be fed in a continual loop, set NEXUS_PacketSubSettings.loop = true.
****************************************************************************/
NEXUS_Error NEXUS_PacketSub_GetBuffer(
    NEXUS_PacketSubHandle packetSub,
    void **pBuffer, /* [out] attr{memory=cached} */
    unsigned *pSize /* [out] */
    );

/***************************************************************************
Summary:
Report amount of data written into the buffer returned by NEXUS_PacketSub_GetBuffer
****************************************************************************/
NEXUS_Error NEXUS_PacketSub_WriteComplete(
    NEXUS_PacketSubHandle packetSub,
    unsigned amount
    );

/***************************************************************************
Summary:
Report amount of data written into the buffer returned by NEXUS_PacketSub_GetBuffer
along with amount skipped before the data
****************************************************************************/
NEXUS_Error NEXUS_PacketSub_WriteCompleteWithSkip(
    NEXUS_PacketSubHandle packetSub,
    unsigned skip,
    unsigned amount
    );

#define NEXUS_PacketSub_ReadComplete NEXUS_PacketSub_WriteComplete

/***************************************************************************
Summary:
Flush data from the buffer
****************************************************************************/
void NEXUS_PacketSub_Flush(
    NEXUS_PacketSubHandle packetSub
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_PACKETSUB_H__ */


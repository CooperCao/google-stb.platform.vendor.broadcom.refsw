/******************************************************************************
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
 *****************************************************************************/

#ifndef TSPSIMGR2_H__
#define TSPSIMGR2_H__

#include "berr.h"
#include "nexus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum bresult {
    b_ok=0,
    berr_out_of_memory=1,
    berr_invalid_parameter=2,
    berr_not_supported=3,
    berr_not_available=4,
    berr_busy=5,
    berr_external_error=6,
    berr_invalid_state=7,
    berr_timeout=8
} bresult;

typedef struct
{
    uint16_t    pid;
    uint8_t     streamType;
    uint16_t    ca_pid;
} EPID;

#define MAX_PROGRAM_MAP_PIDS    12
typedef struct
{
    uint16_t    program_number;
    uint16_t    map_pid;
    uint8_t     version;
    uint16_t    pcr_pid;
    uint16_t    ca_pid;
    uint8_t     num_video_pids;
    EPID        video_pids[MAX_PROGRAM_MAP_PIDS];
    uint8_t     num_audio_pids;
    EPID        audio_pids[MAX_PROGRAM_MAP_PIDS];
    uint8_t     num_other_pids;
    EPID        other_pids[MAX_PROGRAM_MAP_PIDS];
    uint16_t    maxWidth[MAX_PROGRAM_MAP_PIDS];                     /*!< Coded video width, or 0 if unknown, maps to video pid */
    uint16_t    maxHeight[MAX_PROGRAM_MAP_PIDS];                    /*!< Coded video height, or 0 if unknown maps to video pid */
} PROGRAM_INFO_T;

#define MAX_PROGRAMS_PER_CHANNEL 64
typedef struct
{
    uint8_t     version;
    uint16_t    transport_stream_id;
    uint32_t    sectionBitmap;
    uint16_t    num_programs;
    PROGRAM_INFO_T program_info[MAX_PROGRAMS_PER_CHANNEL];
} CHANNEL_INFO_T;

/**
Summary:
Populate a CHANNEL_INFO_T structure by scanning PSI information on
a band.

Description:
This call waits for a PAT, then waits for the PMT's for each program,
then builds the structure.

If you want finer-grain control, use the other tspsimgr functions below.
**/
BERR_Code tsPsi_getChannelInfo2(CHANNEL_INFO_T * pChanInfo, NEXUS_ParserBand band);

/**
Set the timeout values for various blocking operations in tspsimgr.
**/
void tsPsi_setTimeout2( int patTimeout, int pmtTimeout );

/**
Get the timeout values for various blocking operations in tspsimgr.
**/
void tsPsi_getTimeout2( int *patTimeout, int *pmtTimeout );

/**
Summary:
Synchronous call to read the PAT, using the patTimeout.

Description:
bufferSize should be >= TS_PSI_MAX_PSI_TABLE_SIZE in order to read the PAT.

Return Values:
Returns the number of bytes read.
-1 for an error.
0 for no PAT read.
>0 for successful PAT read.
**/
int tsPsi_getPAT2(NEXUS_ParserBand band, void * pPatBuffer, unsigned patBufferSize);

/**
Callback used by tsPsi_getPMTs
**/
typedef void (*tsPsi_PMT_callback)(void *context, uint16_t pmt_pid, const void *pmt, unsigned pmtSize);

/**
Summary:
Read PMT's for each program specified in the PAT.

Description:
This will launch multiple bmessage_stream's and call the callback as each
PMT is read.
**/
BERR_Code tsPsi_getPMTs2(NEXUS_ParserBand     band,
                         const void         * pPatBuffer,
                         unsigned             patBufferSize,
                         tsPsi_PMT_callback   callback,
                         void               * context);

/**
Summary:
Parse a PMT structure into a PROGRAM_INFO_T structure.
**/
void tsPsi_parsePMT2(const void *pmt, unsigned pmtSize, PROGRAM_INFO_T *p_programInfo);

#ifdef __cplusplus
}
#endif

#endif /* TSPSIMGR_H__ */

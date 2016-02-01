/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
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
 ***************************************************************************/
#ifndef B_PLAYBACK_H__
#define B_PLAYBACK_H__

/* uses nexus types for enums, but not handles */
#include "nexus_types.h"

typedef struct b_playback *b_playback_t;

struct b_playback_create_settings
{
    unsigned fifo_size; /* stream data fifo, in bytes */
};

void b_playback_get_default_create_settings(struct b_playback_create_settings *p_settings);

/**
Create a b_playback engine for pull-based data feed using bcmplayer
**/
b_playback_t b_playback_create(const struct b_playback_create_settings *p_settings);

void b_playback_destroy( b_playback_t playback );

struct b_playback_start_settings
{
    const char *datafile; /* optional data file. only required if you are calling get_buffer/read_complete */
    const char *indexfile; /* index file */
    unsigned short video_pid;
    NEXUS_VideoCodec videoCodec; /* eMpeg2 or eH264 */
};

void b_playback_get_default_start_settings( struct b_playback_start_settings *p_settings );

int b_playback_start( b_playback_t playback, const struct b_playback_start_settings *p_settings );

void b_playback_stop( b_playback_t playback );

#define B_PLAYBACK_NORMAL_RATE 1000
struct b_playback_trick_mode
{
    int rate; /* in units of B_PLAYBACK_NORMAL_RATE. used for "adaptive rate" trick modes.
                      if you want to specific GOP-based trick modes (I/P/B), just modify this API. */
    bool dqt;      /* if dqt, rate%100 is # of pictures per GOP, rate/100 is # of GOPS to skip */
};

void b_playback_get_default_trick_mode( struct b_playback_trick_mode *p_trick_mode );

/**
This will flush any internal pipeline.
**/
int b_playback_trick_mode( b_playback_t playback, const struct b_playback_trick_mode *p_trick_mode );

/**
Get the data and index file byte offsets for a given timestamp
**/
int b_playback_get_offsets( b_playback_t playback, unsigned timestamp, off_t *p_data_offset, off_t *p_index_offset);

/**
b_playback_seek will flush any internal pipeline.
To jump forward 30 seconds, call b_playback_seek(playback, 30*1000, SEEK_CUR).
To jump to the beginning, call b_playback_seek(playback, 0, SEEK_SET).
To flush but stay in the current location, call seek(playback, 0, SEEK_CUR);

Returns 0 on success.
**/
int b_playback_seek( b_playback_t playback, long timestamp, int whence );

/**
Get stream data based on current location and trick mode.

TODO: extend this to include index information about what is being returned. can't do that now
because I'm not sure what's actually needed, but it could be done.
**/
int b_playback_get_buffer( b_playback_t playback, void **buffer, unsigned *p_size );

/**
Tell b_playback how much stream data returned by b_playback_get_buffer was consumed.
**/
int b_playback_read_complete( b_playback_t playback, unsigned size );

/* NOTE: there is no need for b_playback_pause. Just stop feeding data. */

/**
Find the timestamp of the index that best matches the PTS, starting from a specific index
**/
long b_playback_find_index_from_pts_and_starting_index( b_playback_t playback, uint32_t pts, long starting_index, long ending_index, unsigned threshold );

#endif

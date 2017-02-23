/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *
 * Module Description: Common types for bcmplayer and tsplayer
 *
 ***************************************************************************/
#ifndef PLAYERTYPES_H
#define PLAYERTYPES_H

/**
* The eBpParam enumerated type is passed to ts/bcmplayer_getParam and
* ts/bcmplayer_setParam as "param". For each value, the usage and meaning varies, so I've
* specified:
* 1. Whether bcmplayer and/or tsplayer support the value.
* 2. Whether get and/or set is supported.
* 3. Whether it uses the paramEnum and/or value in getParam, and p_paramEnum and/or p_value in setParam,
*    and what the meaning is.
**/
typedef enum
{
    eBpPlayMode,            /*  PLAYER: bcmplayer and tsplayer
                                GET: Gets the current eBpPlayMode into p_paramEnum and the
                                rate into p_value.
                                Sets the current eBpPlayMode using paramEnum and the
                                rate using value.

                                See eBpPlayMode for the semantics of value/p_value for each play mode. */

    eBpLoopMode,            /*  PLAYER: bcmplayer and tsplayer
                                Gets the current eBpLoopMode into p_paramEnum.
                                Sets the current eBpLoopMode using paramEnum. */

    eBpDirection,           /*  PLAYER: bcmplayer and tsplayer
                                Gets the current eBpDirection into p_paramEnum.
                                Sets the current eBpDirection using paramEnum.

                                Reverse is only supported for eBpPlayI and eBpPlayBrcm. */

    eBpCurrentStreamPts,    /*  PLAYER: bcmplayer and tsplayer
                                Gets the PTS parameter of the current index into p_value.
                                Sets the current index to the frame nearest to the PTS
                                    passed in using value.

                                In bcmplayer, setting the current index using a PTS is display-order
                                accurate.

                                PTS is not continguous across in a video stream, therefore time
                                jumps should be based on timestamp. */

    eBpCurrentStreamOffset, /*  PLAYER: bcmplayer and tsplayer
                                Gets the current stream byte offset using p_value.
                                Sets the current index to be the frame nearest to the byte offset
                                    passed in using value.

                                The sematics of value and p_value differ between bcmplayer and tsplayer
                                because of bcmplayer's 64 bit stream offset support.

                                For tsplayer:
                                value is the 32-bit stream offset in bytes.
                                p_value is a pointer to a 32-bit unsigned long.

                                For bcmplayer:
                                both value and p_value are treated as a pointer to unsigned long[2] which
                                contains a 64 bit value. [1] is the low order 32 bits and [0] is the
                                high order 32 bits. */

    eBpCurrentSctIndex,     /*  PLAYER: bcmplayer and tsplayer
                                Gets current index into p_value.
                                Sets current index using value. */

    eBpCurrentTimestamp,    /*  PLAYER: bcmplayer
                                Gets the current timestamp into p_value.
                                Sets the current index to be the frame nearest to the
                                    timestamp passed in using value.

                                Timestamp is a relative time from the beginning of
                                a recording measured in milliseconds.

                                It is set by bcmindexer right before writing the bcmindex to
                                disk and is continguous across
                                the entire recording. It precision of the timestamp is dependent on
                                driver's index interface and bcmindexer's
                                write callback. The timestamp is also in decode order, not display
                                order, unlike PTS.

                                With these limitations, it should only be used for time jumps
                                of 1 second accuracy or more.
                                */

    eBpFindIFrame,          /*  PLAYER: bcmplayer and tsplayer
                                Get not supported.
                                Sets the current index to be the nearest I frame from current
                                    location. paramEnum is the eBpDirection to seek in. */

    eBpLastSctIndex,        /*  PLAYER: bcmplayer and tsplayer
                                Gets the last sct index into p_value.
                                Set not supported. */

    eBpIndexFromOffset,     /*  PLAYER: bcmplayer and tsplayer
                                NOTE: This is a lookup function and does not change the state
                                of the player, except for the cache.

                                Get the index corresponding to a stream byte offset.

                                See eBpCurrentStreamOffset for discussion of 32-bit and 64-bit
                                values pointed to by p_value. With the exception that the return
                                value for bcmplayer is only a 32 bit value stored in the high order
                                32 bits (value[0]). */

    eBpIndexFromPts,        /*  PLAYER: bcmplayer and tsplayer
                                NOTE: This is a lookup function and does not change the state
                                of the player.

                                Gets the play index corresponding to a certain PTS and stores it
                                to the location specified by p_value.  When bcmplayer_getParam
                                is called, the location pointed to by p_value should contain
                                the PTS value to search for.
                                Set not supported. */

    eBpVersion,             /*  PLAYER: bcmplayer and tsplayer
                                Get the current version number of ts/bcmplayer into p_value.
                                After allocate/reset, this will be the latest version of the player.
                                But this value can be changed by setting eBpVersion into a
                                compatibility mode.

                                Set not supported. You can set ts/bcmplayer into a compatibility
                                mode using allocate() or reset().

                                Version number is a decimal number in the format XXYY where
                                XX is the major number, YY is the minor number. For example:
                                version 302 is major number 3, minor number 2.

                                The major numbers of ts/bcmplayer and ts/bcmindexer must match
                                for them to work properly.

                                Current versions supported:
                                    0101
                                */

    eBpFirstSctIndex,       /*  PLAYER: bcmplayer
                                Gets the first sct index into p_value.
                                Set not supported.

                                For normal files this will be 0. But your file callbacks passed to
                                bcmplayer_allocation could
                                implement a file that trims the beginning, causing the
                                first sct to be non-zero. */

    eBpFifoSize,            /*  PLAYER: bcmplayer
                                Gets fifo size into p_value. The fifo is the list of packets and frames
                                that are queued to be sent. eBpPlayBrcm play mode will generate
                                multiple packets and frames in order to display one frame. To implement
                                host-based frame advance and slow motion when using eBpPlayBrcm, you
                                should contine to call GetNextEntry until the fifo size is 0,
                                then the host can pause or delay.

                                You can also set the size, but only to 0, which clears the fifo. This
                                is needed when you flush the decoder without changing the
                                bcmplayer mode. */

    eBpCacheSize,           /*  PLAYER: bcmplayer
                                Gets/sets the number of elements in the two index caches using p_value/value.
                                The first cache is used for playback and is driven by the
                                current index. The second cache is used by eBpIndexFromPts and
                                is normally driven by the current PTS on the decoder, which
                                is used for the PVR GUI. */

    eBpDataStartMode,       /*  PLAYER: tsplayer
                                Get/Set the eBpDataStartModeParam using p_value/value.
                                See eBpDataStartModeParam for explanation of this mode. */

    eBpStreamEnd,           /*  PLAYER: tsplayer
                                Get is not supported.
                                Set the current index to the end of the stream, offset by the
                                number of frames specified in value. If value is 0, tsplayer
                                will be at the end of the stream. If value is 1, tsplayer will
                                be 1 frame prior to the end of the stream, etc.

                                This can be implemented with get LastSctIndex and set CurrentSctIndex. */
    eBpDisableExtraBOptimization,
                            /* PLAYER: bcmplayer
                                Gets/sets the DisableExtraBOptimization use value/p_value.

                                By default is false.

                                The "Extra B Optimization" will cause bcmplayer to send multiple
                                B's in a simple DISPLAY group if those B's predict off the same
                                frames. This gives an 18% throughput efficiency. However, you can
                                disable this when doing frame reverse. */
    eBpCurrentVChipState,   /* PLAYER: bcmplayer
                                Gets the vchip state for the current index using p_value.
                                Sets 16 bits of vchip data.
                                The vchip should be placed in that state. */
    eBpIsHits               /* PLAYER: bcmplayer
                                Get only. Determines whether the index is for a HITS stream or
                                a GOP-based stream.

                                Using p_value you should pass in the maximum number of frames
                                to search.

                                If the getParam call succeeds, then p_value will be 0
                                if it's not HITS, 1 if it is. If getParam fails, then
                                it could not find either an I frame or a reference P frame
                                within the specified search range and p_value will be -1. */
} eBpParam;

/**
* paramEnum values for eBpPlayMode. I've documented the valid
* "values" that can be passed to ts/bcmplayer_setParam.
*
* The only valid modes for HITS streams are eBpPlayNormal, eBpPlaySkipB, eBpPlayIP and eBpPlayBrcm.
* eBpPlaySkipP and eBpPlayI will produce macroblocking. bcmplayer and tsplayer don't know
* if the stream is HITS or not, so the calling application is responsible for using
* the correct trick modes.
**/
typedef enum
{
    eBpPlayNormal,  /* Play all frames in the forward direction. "value" is forced to be 1.
                        This feeds data out of bcmplayer in fixed sized blocks. This is more efficient. */
    eBpPlayNormalByFrames,  /* Play all frames in the forward direction. "value" is forced to be 1.
                        Unlike eBpPlayNormal, this feeds data out of bcmplayer in frames.
                        This can lead to underflows for 60 fps streams. */
    eBpPlayI,       /* Play Only I Frames. "value" is the number of I-frames to advance
                        each time, and can be positive or negative.
                        Note that this is the only valid reverse mode apart from Broadcom
                        Trick Modes (eBpPlayBrcm). */
    eBpPlaySkipB,   /* Skip B frames. "value" is the number of
                        B's that should be shown before a single skip. For example, 1
                        would mean play every-other B frame. If value >= the total
                        B's in a GOP, it is equivalent to eBpPlayNormal. */
    eBpPlayIP,      /* Play all I and P Frames. Skip all B's. "value" is ignored. This can
                        by used with GOP-based or HITS streams. */
    eBpPlaySkipP,   /* Skip trailing P frames in a GOP.
                        "value" is equal to the number of P's to be played per GOP.
                        If value >= the total P's in a GOP, it is equivalent to
                        eBpPlayIP.
                        Not supported in tsplayer. */
    eBpPlayBrcm,    /* Broadcom's Mpeg Trick Mode playback algorithm.
                        "value" is the number of frames to advance each time. It can be positive
                        or negative. A value of 1 is equivalent to eBpPlayI.

                        However, for performance reasons we recommend only using values 2, -3,-2 and
                        -1 for GOP-based trick modes that can't be obtained from other modes,
                        otherwise use the other modes.
                        Not supported in tsplayer. */
    eBpPlayDecoderGOPTrick, /* Feed GOP's to decoder. Can be forward or backward.
                        "value"/100 is the number of frames to feed in each GOP. 0 means all frames.
                        "value"%100 is the number of GOP's to skip between each GOP. 0 means every GOP, 1 means every other GOP, etc.
                        The sign of "value" indicates the direction. -8 would play 8 frames per GOP in reverse. */
    eBpPlayDecoderGOPIPTrick, /* like eBpPlayDecoderGOPTrick, but drop B frames */
    eBpPlayDecoderMDqtTrick,
    eBpPlayDecoderMDqtIPTrick,
    eBpPlayTimeSkip, /* bcmplayer attempts to output I frames based on the time interval specified by "value" in milliseconds.
                        "value" can be positive or negative.
                        bcmplayer may skip I frames, but will never repeat I frames. */
    eBpPlayModeParamMax
} eBpPlayModeParam;

/**
* paramEnum values for eBpDirection
**/
typedef enum
{
    eBpReverse = -1,    /* Reverse direction. Only valid with eBpPlayI and eBpPlayBrcm play modes. */
    eBpForward = 1      /* Forward direction */
} eBpDirectionParam;

/**
* paramEnum values for eBpLoopMode
**/
typedef enum
{
    eBpSinglePlay,      /* Play once. getNextPlayEntry() returns -1 at EOF or BOF.. */
    eBpLoopForever      /* When getNextPlayEntry() hits EOF or BOF, loop instead of fail.
                            Note that the decoder needs to be flushed when looping,
                            so using this mode may cause macroblocking. To avoid macroblocking,
                            the caller should always use eBpSinglePlay and implement its own
                            loop with an accompanying decoder flush. */
} eBpLoopModeParam;

/**
* paramEnum values for eBpDataStartMode. When getNextEntry returns
* a frame, this specifies if it should back up to include the previous PES header when doing
* a trick mode, regardless if it is immediately prior or not.
*
* TODO: Not supported in bcmplayer. It might need to be.
**/
typedef enum
{
    eBpPlayFromTrans,   /* Start with nearest Trans packet before destination frame. If the PES
                            header is immediately prior, then back up to it. */
    eBpPlayFromPes      /* Start with nearest PES contained in a trans packet before destination frame,
                            even if not immediately prior. */
} eBpDataStartModeParam;

/**
* Callback functions passed to ts/bcmplayer_allocate. You can typecast fread/ftell/fseek for the
* simplest implementation.
**/
typedef long (*BP_READ_CB)( void *buffer, long size, long count, void *fp );
typedef long (*BP_TELL_CB)( void *fp );
typedef int (*BP_SEEK_CB)( void *fp, long offset, int origin );

/*
 * Optional callback to return file offset of start of file (0) and offset
 * of end of file (file lenght),
 * it could be implemented by using fileno(3) and fstat(2) for STDIO based access */
typedef int (*BP_STAT_CB) ( void *fp, unsigned long *pstart, unsigned long *pend);

/**
Summary:
BTP mode values
Description:
These are the supported values in word[0] of a BTP or BUD
**/
#define TT_MODE_PLAY                0x00
#define TT_MODE_DISPLAY             0x01
#define TT_MODE_BUILD_REFERENCE     0x02
#define TT_MODE_PROCESS             0x03
#define TT_MODE_DISCARD_TILL_BTP    0x04
#define TT_MODE_DISPLAY_FORWARD     0x05
#define TT_MODE_DISPLAY_REWIND      0x06
#define TT_MODE_DISPLAY_PAST_BUFFER 0x07
#define TT_MODE_PIC_OUTPUT_COUNT    0x09 /* picture output count */
#define TT_MODE_INLINE_FLUSH        0x0A /* also known as Trick Play Discontinuity (TPD) */
#define TT_MODE_TIMING_MARKER       0x0C
#define TT_MODE_PICTURE_TAG         0x0D
#define TT_MODE_PICTURE_LAST        0x82
#define TT_MODE_OUTPUT_N            0x1C

#endif /* PLAYERTYPES_H */

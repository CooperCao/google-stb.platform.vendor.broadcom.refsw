/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description: Transport Stream Index Player Header File
 *
 ***************************************************************************/
#ifndef BCMPLAYER_H__
#define BCMPLAYER_H__

#include "playertypes.h"
#include "bcmindexer.h"
#include "bcmplayer_version.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* Summary:
* Opaque handle used to identify one instance of Bcmplayer.
*/
typedef struct BNAV_Player_HandleImpl *BNAV_Player_Handle;

/**
* Summary:
* Optional callback to determine the first and last index offsets of the index file.
*
* Description:
* This callback is set using the BNAV_Player_Settings structure.
* It is called anytime Bcmplayer needs to bound itself using the firstIndex or lastIndex.
* Note that lastIndex == totalIndexes - 1.
* This is necessary for files which are growing (lastIndex is increasing) or are being
* trimmed (firstIndex is increasing). For static files, it's most efficient to not
* specify a callback and call BNAV_Player_SetBounds instead.
*
* Return values:
* 0 = success
* nonzero = failure
**/
typedef int (*BP_BOUNDS_CB)(
    BNAV_Player_Handle handle,
    void *filePointer,
    long *firstIndex,       /* [out] if -1, value is ignored */
    long *lastIndex         /* [out] if -1, value is ignored  */
    );

/**
* Summary:
* Default implementation of BP_BOUNDS_CB which uses the seekCb and tellCb callbacks to
* determine the lastIndex.
*
* Return values:
* 0 = success
* nonzero = failure
**/
int BNAV_Player_DefaultGetBounds(
    BNAV_Player_Handle handle,
    void *filePointer,
    long *firstIndex,   /* [out] */
    long *lastIndex     /* [out] */
    );

/**
* Summary:
* Debug modes for BNAV_Player_SetDebugMode.
**/
typedef enum {
    BNAV_Player_DebugNone,                  /* Don't print anything */
    BNAV_Player_DebugDisplayFramesSent,     /* Only print displayable frames */
    BNAV_Player_DebugFramesSent,            /* Print all decode-only and decode-and-display
                                                frames */
    BNAV_Player_DebugFramesAndAllPacketsSent    /* Print everything sent, including all
                                                BTP packets. */
} BNAV_Player_DebugMode;

/**
* Summary:
* Settings used to initalize Bcmplayer.
*
* Description:
* You must call BNAV_Player_GetDefaultSettings before setting any member of this structure.
* See Also:
* BNAV_Player_GetDefaultSettings, BNAV_Player_Open, BNAV_Player_Reset
**/
typedef struct {
    unsigned short videoPid;/* Video pid for the stream. This is used to generate
                                valid BTP packets.
                                REQUIRED to be set. */
    unsigned short enhancementVideoPid; /*  Enhancement Video pid for the stream, REQUIRED be set to playback SVC/MVC streams */
    BP_READ_CB readCb;      /* Callback bcmplayer uses to read the index.
                                fread() works fine, but you can use your own callback.
                                REQUIRED to be set. */
    BP_TELL_CB tellCb;      /* Callback bcmplayer uses to tell the location of the index.
                                ftell() works fine, but you can use your own callback.
                                ftello() with 64 bit off_t does not work (therefore you can't have
                                >2GB indexes.)
                                REQUIRED to be set. */
    BP_SEEK_CB seekCb;      /* Callback bcmplayer uses to seek to a location in the index.
                                fseek() works fine, but you can use your own callback.
                                fseeko() with 64 bit off_t does not work (therefore you can't have
                                >2GB indexes.)
                                REQUIRED to be set. */
    BP_BOUNDS_CB boundsCb;  /* Callback bcmplayer uses to set firstIndex and lastIndex.
                                The callback is invoked every time before bcmplayer uses
                                the firstIndex and lastIndex values.

                                BNAV_Player_GetDefaultSettings() sets this to
                                BNAV_Player_DefaultGetBounds, which uses the seek and tell
                                callbacks.

                                If you set it to NULL, this disables the polling feature.
                                You can then change firstIndex and
                                lastIndex by calling BNAV_Player_SetBounds(). This gives
                                a performance increase if the file size is static, and it
                                gives applications an option to control when the player is
                                updated. Also, if you updating the bounds from a separate thread
                                (i.e. the record thread), you must be careful to not call
                                bcmplayer functions re-entrantly. */
    bool changingBounds;    /* if true, boundsCb will always be called before updating firstIndex/lastIndex.
                                this is useful for timeshifting playbacks. */
    void *filePointer;      /* Pointer to be passed to the callback functions. If you
                                are using fread()/fseek()/ftell(), then this should be the
                                corresponding pointer to FILE.
                                REQUIRED to be set. */
    BNAV_DecoderFeatures decoderFeatures;
                            /* Lists which features the decoder supports. */
    int normalPlayBufferSize;   /* Normal playback isn't frame based. This is because
                                disk seek times will overwhelm 60 frames/sec content.
                                However, using bcmplayer for normal play does allow
                                the currentIndex to be kept up-to-date. The normalPlayBufferSize
                                determines how big the blocks will be. Generally HD content
                                needs larger sized blocks than SD content. */
    unsigned normalPlayAlignment; /* The layer above will align all I/O reads to this amount (like 4096).
                                So, we get better performance in normal play if normalPlayBufferSize is used
                                with this alignment in mind. */
    int cacheSize;          /* Size of NAV Table cache. */
    BNAV_Player_DebugMode debugMode;
                            /* Runtime control for debug information. This feature is subject to change.
                                See implementation for details. */
    long firstIndex;        /* Determines the first index accessible by bcmplayer.
                                If boundsCb is not NULL, this is ignored. If boundsCb is NULL,
                                you can update these values with BNAV_Player_SetBounds(). */
    long lastIndex;         /* Determines the last index accessiable by bcmplayer. Note that
                                lastIndex = totalIndexes - 1.
                                If boundsCb is not NULL, this is ignored. If boundsCb is NULL,
                                you can update these values with BNAV_Player_SetBounds(). */
    BNAV_Version navVersion;/* You can specify the version of the NAV table.
                                BNAV_Player_GetDefaultSettings() sets this to BNAV_VersionUnknown
                                add the actual version is auto-detected. */
    int isPes;              /* If non-zero, the NAV table is for PES format.
                                bcmplayer may not be able to autodetect the difference between
                                TS and PES NAV Tables because the entries are of different
                                size.
                                NOTE: This is not supported yet. */
    int transportTimestampEnabled; /* If true, then transport has prepended a 4 byte
                                    timestamp to every transport packet and this must be
                                    taken into account. */
    int skipSanityCheck;    /* When bcmplayer opens a new index, it performs a few checks
                                to make sure the index is valid. By default the check is
                                performed. You may want to turn this off (for instance,
                                if you're accessing a "virtualized" index which doesn't
                                exist yet.) */
    int useReferenceFrame;  /* Allows bcmplayer algorithm to use a "reference frame"
                                which leads to better performance but more memory usage.
                                This is a boolean which defaults true. */

    bool useBtpsForHostTrickModes; /* If true, use PROCESS BTP command packets to trim ES payload
                                even on non-Broadcom trick modes (e.g. I-frame trick modes).
                                This is useful for trimming undesired start codes from leading or
                                trailing payload. If you set this to false, you should set the
                                decoder into I-frame only mode so the partial pictures are ignored. */
    bool strictFind;        /* if true, then any 'find' call that is out of bounds will fail.
                               if false (default), then it will be bounded to first or last index. */
} BNAV_Player_Settings;

/**
* Summary:
* Get default values for BNAV_Player_Settings.
* Description:
* This must be called before using the BNAV_Player_Settings structure.
**/
void BNAV_Player_GetDefaultSettings(
    BNAV_Player_Settings *settings      /* [out] */
    );

/**
* Summary:
* Open a new Bcmplayer handle.
*
* Description:
* You can open multiple players. Each player can process a single index. All calls
* to the BNAV_Player API must be synchronized if using the same handle.
*
* Return values:
* 0 on success
* -1 on error
**/
int BNAV_Player_Open(
    BNAV_Player_Handle *handle,             /* [out] New handle. Only valid if function returns 0. */
    const BNAV_Player_Settings *settings    /* Some members of BNAV_Player_Settings must be
                                            set to non-default values. See the structure's
                                            documentation for details. */
    );

/**
* Summary:
* Reset Bcmplayer to an initial state.
* Description:
* If cacheSize is unchanged, the cache will not be reallocated.
*
* Return values:
* -1 on error
* 0 on success
**/
int BNAV_Player_Reset(
    BNAV_Player_Handle handle,
    const BNAV_Player_Settings *settings
    );

/**
* Summary:
* Free a Bcmplayer instance.
* Description:
* The handle becomes invalid and any further use leads to undefined behavior.
**/
void BNAV_Player_Close(
    BNAV_Player_Handle handle
    );

/**
* Summary:
* Populate a BNAV_Player_Settings structure with current values.
* Description:
* This performs no disk access or searches, and so it is guaranteed to be fast.
**/
void BNAV_Player_GetSettings(
    BNAV_Player_Handle handle,
    BNAV_Player_Settings *settings      /* [out] */
    );

/**
* Summary:
* Update BNAV_Player_Settings.changingBounds
**/
void BNAV_Player_UpdateChangingBounds(
    BNAV_Player_Handle handle,
    bool changingBounds
    );

/**
* Summary:
* Set the search bounds of the player.
* Description:
* This is an alternative to using the boundsCb member of BNAV_Player_Settings.
* This can be used if you know the file is fixed size and the bounds will never change.
*
* By calling this function with both params not equal to -1, you will NULL out any bounds callback you may have
* specified. This includes the default bounds callback. Instead, the values you pass
* in will be used.
*
* If you set either firstIndex, lastIndex or both to -1, bcmplayer will use the bounds callback to get the missing value.
*
* Return values:
* 0 if successful.
* -1 if bounds are inconsistent or negative.
**/
int BNAV_Player_SetBounds(
    BNAV_Player_Handle handle,
    long firstIndex, /* if -1, value is ignored */
    long lastIndex   /* if -1, value is ignored */
    );

/**
* Summary:
* Set the debug mode of BNAV_Player.
* Description:
* Debug modes will print information to the console about what frames are being
* sent out of BNAV_Player_GetNextEntry.
**/
void BNAV_Player_SetDebugMode(
    BNAV_Player_Handle handle,
    BNAV_Player_DebugMode debugMode
    );

/******************************************
*
* Getting play entries
*
**/

/**
* Summary:
* Data structure used to return data from BNAV_Player_GetNextEntry().
**/
typedef struct
{
    unsigned long   startOffsetHi;      /* High-order 32 bits of mpeg offset. */
    unsigned long   startOffset;        /* Low-order 32 bits of mpeg offset. */
    unsigned long   byteCount;          /* Number of bytes of mpeg to transfer. */
    char            isInsertedPacket;   /* Inserted packet flag. If true, then
                                            ignore startOffset/startOffsetHi/byteCount and
                                            write the 188 (or 192) bytes pointed to be pkt
                                            parameter to the decoder.
                                            The size is 192 if transportTimestampEnabled
                                            is true. */
    char            isLastEntry;        /* True if this is the last piece of data in
                                            bcmplayer's fifo. */
    unsigned short  filteredPid;        /* if not zero, then payload of only this pid should be handed down */
    unsigned long   zeroByteCountBegin; /* deprecated */
    unsigned long   zeroByteCountEnd;   /* deprecated */
    bool waitingForIntraGopPictureIndex;
} BNAV_Player_PlayEntry;

/**
* Summary:
*   Signal to calling app different index retrieval failures.
*
* Description:
*   In most case it is sufficient to signal to calling app that index retrieval failed ( BNAV_GENERIC_IDX_FAIL ),
*   but during continuous record we need to know when Advance Index retrieval fails, as this usually happens
*   before begin/end of index entries are reached.
*
**/
#define BNAV_GENERIC_IDX_FAIL   -1
#define BNAV_ADVANCE_IDX_FAIL   -2

/**
* Summary:
* Retrieve the offset and size of the next chunk of mpeg data to be
* sent to the decoder.
*
* Description:
* This function is repeatedly called by the user application in the playback thread.
* See the Sample Usage Code for an example of how to call this.
*
* Return values:
* 0 = success
* -1 = failure
**/
int BNAV_Player_GetNextPlayEntry(
    BNAV_Player_Handle handle,
    BNAV_Player_PlayEntry *entry,   /* [out] Contains the result of the call. */
    unsigned char *pkt          /* [out] Pointer to 188 (or 192) byte array. If
                                    p_tpEntry->isInsertedPacket is true, then
                                    this contains data which must be sent
                                    to the decoder. Ignore the offset/byteCount
                                    in p_tpEntry.
                                    The packet size must be 192 if transportTimestampEnabled
                                    is true, otherwise 188.
                                    */
    );

void BNAV_Player_SetMDqtInfo(
    BNAV_Player_Handle handle,
    unsigned intraGopPictureIndex,
    unsigned openGopPictures
    );

/******************************************
*
* Play Mode
*
**/

/**
* Summary:
* The play mode stucture tells BNAV_Player how it should feed frames to the application.
*/
typedef struct {
    eBpPlayModeParam playMode;          /* Determines what kind of playback to use. */
    int playModeModifier;               /* Sets direction and speed for the playMode. Some
                                            modifiers are not allowed for certain play modes:

                                            eBpPlayNormal and eBpPlayIP assert this value
                                            to 1 and so the value you pass in is ignored.

                                            eBpPlayI and eBpPlayBrcm are the only modes which
                                            can have a negative value.

                                            0 is never allowed.
                                            */
    eBpLoopModeParam loopMode;          /* Sets whether bcmplayer will automatically loop
                                            at the end of the file. If you choose eBpLoopForever,
                                            be aware that the firmware must be able to detect
                                            a discontinuity and not decode open gop
                                            B's at the beginning of the stream, otherwise you
                                            will macroblock. This is currently not done. */
    int disableExtraBOptimization;      /* If false, adjacent B's will be combined into
                                            the same DISPLAY_REWIND sequence when doing
                                            Brcm 1x rewind. This results in a significant
                                            bandwidth savings. You may want to see this
                                            to true when doing host-based frame reverse. */
} BNAV_Player_PlayMode;

/**
* Summary:
* Change the play mode.
*
* Description:
* This causes bcmplayer's play state to be reset and fifo to be
* flushed.
**/
int BNAV_Player_SetPlayMode(
    BNAV_Player_Handle handle,
    const BNAV_Player_PlayMode *playMode
    );

/**
* Summary:
* Populates a BNAV_Player_PlayMode structure with the current play mode parameters.
**/
void BNAV_Player_GetPlayMode(
    const BNAV_Player_Handle handle,
    BNAV_Player_PlayMode *playMode      /* [out] */
    );



/******************************************
*
* Position and Status
*
**/

/**
* Summary:
* Information about the current position of the player.
**/
typedef struct {
    long index;                     /* Index offset in the Broadcom index */
    unsigned long pts;              /* PTS (Presentation Time Stamp) */
    unsigned long offsetHi;         /* Hi 32 bits of the MPEG file offset */
    unsigned long offsetLo;         /* Lo 32 bits of the MPEG file offset */
    unsigned long metadataOffset;   /* Byte offset backward from offsetLo/offsetHi to this frame's metadata (for example, seqhdr or SPS/PPS).
                                       This data must exist in the stream for this frame to be decodeable. */
    unsigned long timestamp;        /* Bcmindexer calculated timestamp */
    unsigned short vchipState;      /* Current vchip state set by BNAV_Indexer_SetVChipState */
} BNAV_Player_Position;

/**
* Summary:
* Get information about the current position of the player.
*
* Description:
* Getting the position of bcmplayer reads the current index. If that index is not already
* in the index cache, it will involve one disk seek and read. There are no searches involved.
*
* Notice that the currentIndex is returned by both getStatus and getPosition.
* It is the only position paramter that doesn't require disk access.
*
* Return values:
* 0 = index was found and the BNAV_Player_Position structue has been successfully populated.
* -1 = index was not found and the BNAV_Player_Position structure is unchanged.
**/
int BNAV_Player_GetPosition(
    BNAV_Player_Handle handle,
    BNAV_Player_Position *position      /* [out] */
    );

/**
* Summary:
* Get information about a specified index entry without resetting the current location.
*
* Description:
* Reading any index entry will first check the cache. If it is not present in the cache,
* it will involve one disk seek and read. There are no searches involved.
*
* BNAV_Player_GetPosition works by calling this function with the current index.
*
* Return values:
* 0 = index was found and the BNAV_Player_Position structue has been successfully populated.
* -1 = index was not found and the BNAV_Player_Position structure is unchanged.
**/
int BNAV_Player_GetPositionInformation(
    BNAV_Player_Handle handle,
    long index,                         /* Index to query */
    BNAV_Player_Position *position      /* [out] */
    );


/**
* Summary:
* Status returned by BNAV_Player_GetStatus.
**/
typedef struct {
    long currentIndex;      /* Current index offset. */
    int fifoSize;           /* How many frames are queued up in the internal Bcmplayer FIFO.
                                Each call to BNAV_Player_GetNextEntry will consume one entry
                                in the FIFO. When the FIFO is empty, the player will advance
                                and the FIFO is filled again.
                                You can clear this FIFO using BNAV_Player_FlushFifo(). */
} BNAV_Player_Status;

/**
* Summary:
* Populate a BNAV_Player_Status structure with current status.
*
* Description:
* Getting the status of Bcmplayer performs no disk access or searches, and so it
* is guaranteed to be fast.
*
* Notice that the currentIndex is returned by both getStatus and getPosition.
* It is the only position paramter that doesn't require disk access.
**/
void BNAV_Player_GetStatus(
    const BNAV_Player_Handle handle,
    BNAV_Player_Status *status      /* [out] */
    );

/**
* Summary:
* Reposition the current index.
* Description:
* This resets the play state variables in bcmplayer including
* flushing the fifo (BNAV_Player_FlushFifo). It is automatically called by
* BNAV_Player_SetPlayMode.
*
* The current index must fall within the bounds.
*
* Because the new position is not validated, there is no return code.
*
* Return values:
* 0 = success, current index was changed
* -1 = index was invalid and current index is unchanged
**/
int BNAV_Player_SetCurrentIndex(
    BNAV_Player_Handle handle,
    long index              /* new current index */
    );

/**
* Summary:
* Flush all frames from the bcmplayer internal fifo.
* Description:
* Bcmplayer maintains an internal FIFO. The state of the FIFO is usually of no concern to the
* application, unless the application wants to flush the system (including decode and playback
* buffers) without changing the bcmplayer mode. Then you will need an explicit call to
* BNAV_Player_FlushFifo.
**/
void BNAV_Player_FlushFifo(
    BNAV_Player_Handle handle
    );



/******************************************
*
* Various search and information algorithms. None of these algorithms changes the current
* index of bcmplayer.
*
**/

/**
* Summary:
* Determine if the stream is a HITS (progressive I slice) stream.
*
* Description:
* Performs search from beginning of the index, looking for an I-frame. It is bounded
* the number of frames specified by searchRange
*
* Return values:
* 0 = successful search, no I-frame, therefore HITS.
* 1 = successful search, I-frame found, therefore not HITS.
* -1 = unsuccessful search
**/
int BNAV_Player_IsHits(
    BNAV_Player_Handle handle,
    int searchRange         /* Maximum number of frames to search. The correct value
                                depends on the GOP size. */
    );

/**
* Summary:
* Set ptscache search direction to backwards
*
* Description:
* In steady state, Ptscache is normally searched from oldest to newest entry ( forward search )
* For transitions ( eg trickmode change ) it is more efficient to search from newest entry
* to oldest ( backwards search ).
*
**/
void BNAV_Player_SetBackwardPtsCacheSearch(
    BNAV_Player_Handle handle
    );

/**
* Summary:
* Search for the index which most closely matches the pts using the current location as a starting point.
*
* Description:
* It does not change the current index.
*
* This function was written with two usage modes in mind:
* o Using the decoder's current PTS, converted to an index, to position the PVR cursor in a GUI
* o Using the decoder's current PTS, converted to an index, to relocate the player for trick mode transitions.
*
* Both modes assume you are using the decoder's current PTS. Because of this, we maintain a cache of picture's PTS
* and index values that have recently been sent to the decoder. This is efficient and will work regardless of the
* trick mode speed used (even very fast speeds). Because of PTS discontinuities, an index-wide binary search
* for a PTS is impossible anyway; therefore this narrow definition of PTS search is acceptible.
*
* Initially this function only ever searched from newest to oldest when trying to find a match. This is the simplest
* and most reliable scheme, but the tradeoff is that the more compressed pictures that fit in a decoder's compressed
* picture buffer, the more entries we need to maintain in our cache, and the longer the search becomes.
* In order to mitigate this, the function now does a backward search on trick mode transitions / decoder cache flushes,
* and thereafter maintains a read ptr to the last found cache hit ( minus back off ), and searches forward from this
* point.
*
* This function also now interpolates the index value for streams which have all the pictures in a GOP encoded with
* the same PTS.
*
* If you get bad index values back, you should also see BDBG_ERR messages stating that the PTS diff exceeded a minimum
* threshold. In this case, you are either using this function in an unsupported way (i.e. searching for PTS
* values that have not been recently decoded), or you need to expand the internal cache size (see
* PTS_CACHE_SIZE in the implementation).
*
* If you want to do index-wide searches, you should use other values like index number, MPEG offset or
* timestamp.
*
* Return values:
* -1 = failure
* Otherwise returns the index number.
**/
long BNAV_Player_FindIndexFromPts(
    BNAV_Player_Handle handle,
    unsigned long pts,      /* the pts to search for */
    int searchWindow        /* this parameter is no longer used */
    );

void BNAV_Player_DiscardToPts(
    BNAV_Player_Handle handle,
    uint32_t pts
    );

/**
* Summary:
* Search for the index which most closely matches the pts using the specified location and direction as a starting point.
*
* This search does not use the PTS cache because it is not limited to what has been recently played.
* Because PTS's are not guaranteed continue, this is a linear search. It should not be used for large scale searches.
* The search direction is strictly obeyed.
**/
long BNAV_Player_FindIndexFromPtsAndStartingIndex(
    BNAV_Player_Handle handle,
    unsigned long target_pts,      /* the pts to search for */
    long startingIndex, /* index to start search */
    long endingIndex, /* index to end search. can be > or < startingIndex. start -> end sets the direction. */
    unsigned threshold /* find match if abs(target_pts - pts) <= threshold. if (unsigned)-1 is used, an internal default is used. */
    );

/**
* Summary:
* Search for the index which contains the specified byte offset.
*
* Description:
* It does not change the current index.
* The position is interpolated using the size of the file, followed
* by a very brief linear search. It should be very fast.
*
* Return values:
* -1 = failure
* Otherwise returns the index number.
**/
long BNAV_Player_FindIndexFromOffset(
    BNAV_Player_Handle handle,
    unsigned long offsetHi,     /* high 32 bits of the mpeg file offset. */
    unsigned long offsetLo      /* low 32 bits of the mpeg file offset. */
    );

/**
* Summary:
* Search for the index which most closely matches the timestamp.
*
* Description:
* It does not change the current index.
* Because timestamp is guaranteed to be continuous, the position is interpolated, followed
* by a very brief linear search. It should be very fast.
*
* Return values:
* -1 = failure
* Otherwise returns the index number.
**/
long BNAV_Player_FindIndexFromTimestamp(
    BNAV_Player_Handle handle,
    unsigned long timestamp
    );

/**
* Summary:
* Searches for the first I frame from the specified starting index in the specified
* direction.
*
* Description:
* It does not change the current index.
*
* WARNING: For HITS streams, this will search to the beginning or end of the index
* and then fail.
*
* Return values:
* -1 = failure
* Otherwise returns the index number.
**/
long BNAV_Player_FindIFrameFromIndex(
    BNAV_Player_Handle handle,
    long index,                 /* starting location */
    eBpDirectionParam dir       /* search direction */
    );

/**
* Summary:
* Checks if frame at passed in index value is a B frame or not.
*
* Description:
* Checks if frame at passed in index value is a B frame or not.
*
* Return values:
* true if B frame
* false if other Frame.
**/
bool BNAV_Player_IndexIsBFrame(
    BNAV_Player_Handle handle,
    long index                   /* index to check */
    );

/**
* Summary:
* Prints the hit/miss ratios for the two internal index caches to stdout.
*
* Description:
* This is used to test the cache performance of bcmplayer. The cache size may need
* to be adjusted depending on the normal types of trick modes, frames rates (e.g. 60 fps),
* and memory requirements of the system.
**/
void BNAV_Player_PrintCachePerformance(
    const BNAV_Player_Handle handle
    );

/**
* Summary:
* Reads the BNAV_Entry specified by the index number.
*
* Description:
* This uses bcmplayer's seek/tell/read
* callbacks as well as the index cache. It does not change the state of the player, except
* for possibly the cache.
*
* Return values:
* -1 on failure
* 0 on success
**/
int BNAV_Player_ReadIndex(
    BNAV_Player_Handle handle,
    int index,                  /* Index offset to read. */
    BNAV_Entry *entry           /* [out] Memory location to read index into. */
    );
    
/**
* Summary:
* Reads the BNAV_AVC_Entry specified by the index number.
*
* Description:
* See BNAV_Player_ReadIndex for semantics.
**/
int BNAV_Player_ReadAvcIndex(
    BNAV_Player_Handle handle, 
    int index, 
    BNAV_AVC_Entry *entry
    );

/**
Summary:
Return the entry size in bytes for the specified nav version.

Description:
Typically the size will be either sizeof(BNAV_AVC_Entry) or sizeof(BNAV_Entry).
You can learn the detected nav version by calling BNAV_Player_GetSettings and using BNAV_Player_Settings.navVersion.
**/
unsigned BNAV_GetEntrySize(
    BNAV_Version navVersion
    );

/* override BNAV_Player_Settings.normalPlayBufferSize */
void BNAV_Player_SetNormalPlayBufferSize(
    BNAV_Player_Handle handle,
    unsigned long normalPlayBufferSize
    );

#ifdef __cplusplus
}
#endif

#endif /* BCMPLAYER_H__ */

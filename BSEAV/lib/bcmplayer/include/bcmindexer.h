/***************************************************************************
 *     Copyright (c) 2002-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: Broadcom Index Player
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BCMINDEXER_H__
#define BCMINDEXER_H__

#include "bcmplayer_version.h"
#include "bcmsct_index.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* Summary:
* Structure of a Broadcom index entry.
*
* Description:
* It is used internally but is made public to support BNAV_Player_ReadIndex().
**/
typedef struct
{
    uint32_t words[8];    /* Index entry data */
} BNAV_Entry;

/**
* Summary:
* An extension of BNAV_Entry to support AVC indexing.
*
* Description:
* The first 8 words are identical to BNAV_Entry.
**/
typedef struct
{
    uint32_t words[16];    /* Index entry data */
} BNAV_AVC_Entry;

typedef BNAV_AVC_Entry BNAV_HEVC_Entry;

/**
* Summary:
* The size of one BNAV_Entry.
**/
#define BNAV_ENTRY_SIZE     sizeof(BNAV_Entry)  /* in bytes, but should be an even word */

/**
* Summary:
* Opaque handle used to identify one instance of Bcmindexer.
**/
typedef struct BNAV_Indexer_HandleImpl *BNAV_Indexer_Handle;

/**
* Summary:
* Callback function which is used by BNAV_Indexer_Settings.
**/
typedef unsigned long (*BNAV_WRITE_CB)( const void *p_bfr, unsigned long numEntries, unsigned long entrySize, void *fp );

/**
* Summary:
* Optionally used callback for determining the current MPEG file size in order
* to resolve wrapping issues.
*
* Description:
* See BNAV_Indexer_Settings.mpegSizeCallback for details on how and why this is used.
* Should returns 0 on success.
**/
typedef int (*BNAV_MPEG_SIZE_CB) (
    BNAV_Indexer_Handle handle,
    unsigned long *sizeHi,  /* [out] Hi 32 bits of the current MPEG file size */
    unsigned long *sizeLo   /* [out] Lo 32 bits of the current MPEG file size */
    );

/**
Summary:
The type of video being processed.

Description:
These enum values should match bvideo_codec in the Settop API for consistency.
**/
typedef enum BNAV_Indexer_VideoFormat {
    BNAV_Indexer_VideoFormat_MPEG2 = 2,
    BNAV_Indexer_VideoFormat_AVC = 0x1b,
    BNAV_Indexer_VideoFormat_AVC_SVC = 0x1f,
    BNAV_Indexer_VideoFormat_AVC_MVC = 0x20,
    BNAV_Indexer_VideoFormat_HEVC,
    BNAV_Indexer_VideoFormat_VC1 = 0xea,
    BNAV_Indexer_VideoFormat_AVS =  0x42
} BNAV_Indexer_VideoFormat;

/**
* Summary:
* Settings structure which is passed to BNAV_Indexer_Open.
**/
typedef struct {
    BNAV_WRITE_CB writeCallback;/* Write callback used to output the new index.
                                    You can use fwrite. */
    void *filePointer;          /* File pointer passed to the write callback.
                                    If you use fwrite, this should be FILE*. */
    BSCT_Version sctVersion;    /* Sets the version of the incoming SCT entries. These cannot
                                    be autodetected. */
    BNAV_Version navVersion;        /* Set compatibility version. bcmplayer will auto-detect
                                    the eBcmNavTableVersion when allocated or reset. */
    BNAV_Indexer_VideoFormat videoFormat; /* Defaults to MPEG2 */

    bool ptsBasedFrameRate;         /* If you're using bcmindexer off-line (not during live
                                    recording), this will set the timestamp based on the PTS
                                    in the stream. */
    unsigned simulatedFrameRate;    /* If you're using bcmindexer off-line (not during live
                                    recording), the timestamp needs to set according
                                    to a simulated frame rate. A non-zero simulatedFrameRate enables
                                    this simulation and the value of simulatedFrameRate is used as the rate,
                                    in units of frames per second. Default is 0 (disabled). */
    unsigned long maxFrameSize; /* Any frame larger than this will be assumed to be bad and
                                    discarded. Value is specified in units of bytes.
                                    Default is 5 MB. This addresses the problem of
                                    partially encrypted streams. When a stream becomes
                                    encrypted midway through the record, SCT entries
                                    stop being generated and a huge frame results when the
                                    next SCT entry is fed. When this happens, the huge
                                    frame, all frames up to the next I frame, and
                                    any open GOP B's after the next I frame must be
                                    discarded. */
    BNAV_MPEG_SIZE_CB mpegSizeCallback;
                                /* Optional callback retrieves the size of the MPEG file.
                                    MPEG data is usually recorded in a different thread
                                    than index data, therefore the size is not available.
                                    This makes it available.
                                    The size of MPEG data is needed to determine how many
                                    times the 32 bit SCT frame offset has wrapped when
                                    the MPEG becomes unencrypted after a section of
                                    encryption. During the encrypted section, the 32 bit SCT frame
                                    offset may have wrapped any number of times.
                                    Otherwise there is no way for bcmindexer to know.
                                    Defaults to NULL (no callback). This callback is ignored
                                    for 40 bit SCT frame offset systems because it is assumed
                                    that they will never wrap. */
    int transportTimestampEnabled; /* If true, then transport has prepended a 4 byte
                                    timestamp to every transport packet and this must be
                                    taken into account. */

    /* If these are set, the index is being appended on to another index.
    The offset will be added to every index entry. timestamps will resume from the last timestamp.
    The start of the appended index will be the next random access point. */
    struct {
        unsigned long offsetHi;         /* Hi 32 bits of the MPEG file offset */
        unsigned long offsetLo;         /* Lo 32 bits of the MPEG file offset */
        unsigned long timestamp;        /* Bcmindexer calculated timestamp */
    } append;

    bool allowLargeTimeGaps;       /* By default, any large gap in the recording (e.g. from a disconnected frontend) will cause
                                      the NAV index timestamp to be paused. This means that the gap will not appear in the timestamps on playback.
                                      Set allowLargeTimeGaps to true if you want to disable this feature. Any gap in the recording will
                                      appear in the index timestamps. */
} BNAV_Indexer_Settings;

/**
* Summary:
* Must initialize the BNAV_Indexer_Settings before using it and
* passing it to BNAV_Indexer_Open.
**/
void BNAV_Indexer_GetDefaultSettings(
    BNAV_Indexer_Settings *settings     /* [out] */
    );

/**
* Summary:
* Open an instance of Bcmindexer as identified by the handle.
*
* Return values:
* 0 on success
* -1 if the sctVersion or navVersion is not supported.
**/
int BNAV_Indexer_Open(
    BNAV_Indexer_Handle *handle,        /* [out] */
    const BNAV_Indexer_Settings *settings
    );

/**
* Summary:
* Reset Bcmindexer instance to its initial state.
*
* Return values:
* 0 on success
* -1 if the sctVersion or navVersion is not supported.
**/
int BNAV_Indexer_Reset(
    BNAV_Indexer_Handle handle,
    const BNAV_Indexer_Settings *settings
    );

/**
* Summary:
* Free a Bcmindexer instance.
* Description:
* The handle becomes invalid and any further use leads to undefined behavior.
**/
void BNAV_Indexer_Close(
    BNAV_Indexer_Handle handle
    );

/**
* Summary:
* Feed start code table (SCT) data into Bcmindexer.
*
* Description:
* You must send even blocks of size BSCT_Entry or BSCT_SixWord_Entry. All write callbacks
* will happen while BNAV_Indexer_Feed() is executing. This means that there is no
* buffer which gets flushed to the write callback when you call BNAV_Indexer_Close().
*
* Return values:
* -1 on error
* Number of entries processed
**/
long BNAV_Indexer_Feed(
    BNAV_Indexer_Handle handle,
    void *p_bfr,                /* Pointer to start code index data. This could be
                                    either BSCT_Entry's or BSCT_SixWord_Entry's,
                                    depending on the BSCT_Version. */
    long numEntries             /* Number of SCT entries (either 4 or 6 word) pointed
                                    to by p_bfr. */
    );

/**
* Summary:
* Feed PES data into Bcmindexer.
*
* Description:
* No intermediate SCT entries are produced.
* This is only supported for VC1 currently.
*
* Return values:
* -1 on error
* 0 on success
**/
int BNAV_Indexer_FeedPES(
    BNAV_Indexer_Handle handle,
    uint8_t *p_bfr,             /* Pointer to PES data. */
    unsigned size                   /* Number of bytes pointed to by p_bfr. */
    );

/**
* Summary:
* Feed start code table (SCT) data in reverse order into Bcmindexer.
*
* Description:
* Works like BNAV_Indexer_Feed, except that you feed SCT data in reverse order.
* If p_bfr points to more than one SCT, those SCT's should be in forward order
* and bcmindexer will traverse them in reverse. Subsequent calls to BNAV_Indexer_FeedReverse
* should send data in reverse order.
*
* Return values:
* -1 on error
* Number of entries processed
**/
long BNAV_Indexer_FeedReverse(
    BNAV_Indexer_Handle handle,
    const BSCT_Entry *p_bfr,                /* Pointer to start code index data */
    long numEntries                 /* Number of BSCT_Entry's pointed to by p_bfr. */
    );

/**
* Summary:
* Read whether Bcmindexer believes the MPEG it's indexing is HITS
* or not.
*
* Description:
* HITS means Head-end-in-the-sky and uses progressive refresh instead of
* I frames.
*
* You must feed a "good amount" of index through before you can tell, otherwise it
* defaults to true. Right now, the algorthim looks for one I frame. When it finds
* just one, it switches it to be not HITS. Therefore you need to feed in at least
* one whole GOP to get an I frame.
*
* Return values:
* 0 = not HITS
* 1 = it is HITS
**/
int BNAV_Indexer_IsHits(
    BNAV_Indexer_Handle handle
    );

/**
* Summary:
* Set the current VChip state.
*
* Description:
* Every entry written to disk after this call will contain this vchip state.
*
* Return values:
* 0 = success
* -1 = failure, invalid vchip state
**/
int BNAV_Indexer_SetVChipState(
    BNAV_Indexer_Handle handle,
    unsigned short vchipState
    );

/**
* Summary:
* Get the current VChip state.
*
* Return values:
* vchip state
**/
unsigned short BNAV_Indexer_GetVChipState(
    BNAV_Indexer_Handle handle
    );

/**
* Summary:
* Current position data returned by BNAV_Indexer_GetPosition.
* Description:
* Even though BNAV_Indexer_Position is identical with BNAV_Player_Position,
* they are separate structures because they belong to different API's and
* could diverge in the future.
**/
typedef struct {
    long index;                     /* Index offset in the Broadcom index */
    unsigned long pts;              /* PTS (Presentation Time Stamp) */
    unsigned long offsetHi;         /* Hi 32 bits of the MPEG file offset */
    unsigned long offsetLo;         /* Lo 32 bits of the MPEG file offset */
    unsigned long timestamp;        /* Bcmindexer calculated timestamp */
    unsigned short vchipState;      /* Current vchip state set by BNAV_Indexer_SetVChipState */
} BNAV_Indexer_Position;

/**
* Summary:
* Get current position information for Bcmindexer.
*
* Return values:
* 0 if successful
* -1 if unsuccessful (because no entries have been written yet)
**/
int BNAV_Indexer_GetPosition(
    BNAV_Indexer_Handle handle,
    BNAV_Indexer_Position *position     /* [out] */
    );

/**
* Summary:
* indexer status
**/
typedef struct BNAV_Indexer_Status {
    unsigned parsingErrors; /* number of SCT parsing errors detected by bcmindexer since open or reset.
                               these errors could result from either errors in the stream or errors in the indexing HW/FW/SW. */
} BNAV_Indexer_Status;

/**
* Summary:
* Get status
**/
int BNAV_Indexer_GetStatus(
    BNAV_Indexer_Handle handle,
    BNAV_Indexer_Status *pStatus     /* [out] */
    );

/**
* Convert 12 bit vchip data to 16 bit vchip data.
**/
unsigned short BNAV_unpack_vchip(unsigned short packed_vchip);

/**
* Convert 16 bit vchip data to 12 bit vchip data, checking the 16 bit
* data for bits 6 and 14. If bits 6 and 14 are both 1, the vchip data
* is valid and BNAV_pack_vchip returns 0 and the packed value is
* written to *packed_vchip. Otherwise it returns -1 and *packed_vchip is left unchanged.
*/
int BNAV_pack_vchip(unsigned short unpacked_vchip, unsigned short *packed_vchip);

#ifdef __cplusplus
}
#endif

#endif /* BCMINDEXER_H__ */


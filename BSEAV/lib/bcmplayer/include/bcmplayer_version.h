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
 * Module Description: Converts startcode index to bcmplayer index
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/
#ifndef BCMPLAYER_VERSION_H
#define BCMPLAYER_VERSION_H

/**
* Input to bcmindexer based on SCD (Start code detect) hardware. This cannot
* be autodetected reliably.
**/
typedef enum {
    BSCT_Version32bitOffset,    /* 32 bit index entries, 4 words per entry. */
    BSCT_Version40bitOffset,    /* 40 bit index entries, 4 words per entry.
                                    Not available in venom2. */
    BSCT_Version6wordEntry      /* 6 words per entry. */
} BSCT_Version;

/**
* Output of bcmindexer and input to bcmplayer.
* Bcmindexer can output all formats, but defaults to the latest format.
* Bcmplayer reads the version from the NAV table as is always backward compatible.
*
* Max storage is 4 bits. Therefore only values 0..14 should be used. Value of 15 should be reserved
* for extended version field.
**/
typedef enum {
    BNAV_VersionOriginal = 0,       /* Open GOP B's use the immediate I frame as reference frame */
    BNAV_VersionOpenGopBs = 1,      /* Open GOP B's use the prior I frame as reference frame */
    BNAV_Version2 = 2,              /* Drop dangling open GOP B's and set initial I frame
                                    reference offsets to 0. */
    /* There may be extensions of non-AVC versions.*/

    BNAV_Version_AVC = 3,           /* First extended-size AVC version */

    BNAV_Version_VC1_PES = 4,       /* VC1 video in PES stream. Use BNAV_Indexer_FeedPES. */
    BNAV_Version_AVS = 5,           /* AVS video */
    BNAV_Version_TimestampOnly = 6, /* Content-agnostic index. Use timestamps and PTS for seek and current position status.
                                       This can be used for audio-only streams, encrypted streams, or unsupported
                                       video codecs. */
    BNAV_Version_AVC_Extended = 7,  /* AVC/H.264 with extended layer */
    BNAV_Version_HEVC = 8,           /* HEVC/H.265 */
    BNAV_VersionUnknown     /* Unknown version. Needs to be auto detected. */
} BNAV_Version;

#define BNAV_VersionLatest BNAV_Version2

/**
* Output of bcmplayer to mpegvideo decoder.
* Mpegvideo porting interface reports its capabilities.
**/
typedef struct {
    int supportsBroadcomTrickModes; /* This must be true to have any Broadcom trick modes.
                                        If false, bcmplayer will only allow host trick modes. */
    int supportsCombiningBs;        /* When doing 1x rewind, you can send multiple B's
                                        in the same DISPLAY_REVERSE sequence. Venom2 support
                                        is in development. */
    int supportsDisplayPastBuffer;  /* BTP command to display past predictor. */
    int supportsDummyPicture;       /* Send a one packet dummy picture if it doesn't
                                        need to be decoded. */
} BNAV_DecoderFeatures;

#endif /* BCMPLAYER_VERSION_H */

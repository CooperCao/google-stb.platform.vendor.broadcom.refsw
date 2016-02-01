/***************************************************************************
 *     Copyright (c) 2002-2011, Broadcom Corporation
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
 * Module Description: Software startcode index generator
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef TSINDEXER_H
#define TSINDEXER_H

#include "bcmsct_index.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Callback function tsindexer needs to write new SCT's back to the calling app
**/
typedef unsigned long (*INDEX_WRITE_CB)( const void *p_bfr, unsigned long numEntries, unsigned long entrySize, void *fp );

/**
Summary:
Structure on a 4 four SCT entry.
**/
typedef BSCT_Entry sIndexEntry;

/**
Summary:
Structure on a 6 word SCT entry.
**/
typedef BSCT_SixWord_Entry sSixWordIndexEntry;

/**
Summary:
Main tsindexer object.
**/
typedef struct sTsIndexer sTsIndexer;

/**
Summary:
Allocate new sTsIndexer.

Return values:
NULL on error
New sTsIndexer object
**/
sTsIndexer	*tsindex_allocate(
	INDEX_WRITE_CB cb,			/* write callback. see tsindex_settings for details. */
	void *fp,					/* file pointer. see tsindex_settings for details. */
	unsigned short pid,			/* video pid. see tsindex_settings for details. */
	int version					/* Compatibility version. see tsindex_settings for details. */
	);

/**
Summary:
Reset sTsIndexer to post-allocate state.

Return values:
-1 if version is not supported
0 on success
**/
int tsindex_reset(
	sTsIndexer	*p_tsi,
	INDEX_WRITE_CB cb,			/* write callback. see tsindex_settings for details. */
	void *fp,					/* file pointer. see tsindex_settings for details. */
	unsigned short pid,			/* video pid. see tsindex_settings for details. */
	int version					/* Compatibility version. see tsindex_settings for details. */
	);

/**
Summary:
Settings structure for initializing a tsindex object.
**/
typedef struct tsindex_settings {
	INDEX_WRITE_CB cb;			/* write callback. fwrite() is valid. */
	void *fp;					/* file pointer to pass to callback. If you used fwrite(), this
									will be FILE* */
	unsigned short pid;			/* video pid */
	int version;				/* Compatibility version. If version == -1, latest version is used.
									Major #'s for tsindexer and tsplayer should match.

									Current version supported:
										0101
								 */
	int entry_size;				/* number of words in SCT entry. Default is 4 and you
									should use sIndexEntry to parse the generated data. 
									The other supported value is 6 and you should then
									use sSixWordIndexEntry to parse the generated data. */
	unsigned short start_code_lo;
	unsigned short start_code_hi;
	int is_avc;				/* This is an AVC stream, so don't interpret the
									start codes. Just do raw capture. */
    unsigned ts_packet_size;        /* transport packet size, default is 188, apps can set it 192 to support ts streams w/ additional 4 bytes of timestamps */
} tsindex_settings;

/**
Summary:
Initialize a settings struture to default values.
Description:
b, fp and pid cannot be defaulted.
**/
void tsindex_settings_init(
	tsindex_settings *settings
	);

/**
Summary:
Allocate a tsindex object using the settings structure.
Description:
This is preferred to tsindex_allocate because it can be extended. The old
API is fixed.
**/
sTsIndexer	*tsindex_allocate_ex(
	const tsindex_settings *settings
	);

int tsindex_reset_ex(
	sTsIndexer *p_tsi,
	const tsindex_settings *settings
	);

/**
Summary:
Free a sTsIndexer.
**/
void		tsindex_free(
	sTsIndexer *p_tsi
	);

/**
Summary:
Feed MPEG2 Transport into a sTsIndexer.

Description:
It will build the startcode table and write it out
using the callback passed into tsindex_allocate().

Return values:
-1 on error
Number of packets processed
**/
long		tsindex_feed( sTsIndexer * const p_tsi, const unsigned char *p_bfr, long numBytes );

/**
Summary:
Set the PES id before calling tsindex_feedPes
**/
void		tsindex_setPesId( sTsIndexer *p_tsi, unsigned char pesStreamId );

/**
Summary:
Feed PES mpeg data.
**/
long		tsindex_feedPes( sTsIndexer * const p_tsi, const unsigned char *p_bfr, long numBytes );

/**
Summary:
Feed DVD VOB mpeg data.
**/
long		tsindex_feedVob( sTsIndexer * const p_tsi, const unsigned char *p_bfr, long numBytes );

#ifdef __cplusplus
}
#endif

#endif

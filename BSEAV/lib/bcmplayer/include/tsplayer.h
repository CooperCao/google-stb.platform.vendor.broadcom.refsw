/***************************************************************************
 *     Copyright (c) 2002, Broadcom Corporation
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
 * Module Description: Transport Stream Index Player
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef TSPLAYER_H
#define	TSPLAYER_H

#include "tsindexer.h"
#include "playertypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sTsPlayer sTsPlayer;

typedef struct
{
	unsigned long	startOffset;	/* Starting offset of data */
	unsigned long	byteCount;		/* Number of bytes to transfer */
	unsigned long	startOffsetHi;	/* 31:8 - Reserved, 7:0 - msbyte of starting address */
} sTpEntry;

/**
* Allocate a new sTsPlayer.
*
* Return values:
* NULL = error
* Pointer to allocated sTsPlayer.
**/
sTsPlayer *tsplayer_allocate(
	BP_READ_CB readCb,		/* Callback tsplayer uses to read the index.
								fread() works fine, but you can use your own callback. */
	BP_TELL_CB tellCb,		/* Callback tsplayer uses to tell the location of the index.
								ftell() works fine, but you can use your own callback.
								ftello() does not work (therefore you can't have
								>2GB indexes.) */
	BP_SEEK_CB seekCb,		/* Callback tsplayer uses to seek to a location in the index.
								fseek() works fine, but you can use your own callback.
								fseeko() does not work (therefore you can't have
								>2GB indexes.) */
	void *sctFp,			/* Pointer to be passed to the callback functions. If you
								are using fread()/fseek()/ftell(), then this should be the
								corresponding FILE*. */
	int version					/* Set compatibility version. If version == -1,
									tsplayer will operator in its latest version. The
									major #'s of tsindexer and tsplayer must match. */
	);

/**
* Reset sTsPlayer to post-allocate state.
*
* Return values:
* -1 on error
* 0 on success
**/
int tsplayer_reset(
	sTsPlayer *p_tsp,		/* tsplayer allocated by tsplayer_alloc() */
	BP_READ_CB readCb,		/* Callback tsplayer uses to read the index.
								fread() works fine, but you can use your own callback. */
	BP_TELL_CB tellCb,		/* Callback tsplayer uses to tell the location of the index.
								ftell() works fine, but you can use your own callback.
								ftello() does not work (therefore you can't have
								>2GB indexes.) */
	BP_SEEK_CB seekCb,		/* Callback tsplayer uses to seek to a location in the index.
								fseek() works fine, but you can use your own callback.
								fseeko() does not work (therefore you can't have
								>2GB indexes.) */
	void *sctFp,			/* Pointer to be passed to the callback functions. If you
								are using fread()/fseek()/ftell(), then this should be the
								corresponding FILE*. */
	int version					/* Set compatibility version. If version == -1,
									tsplayer will operator in its latest version. The
									major #'s of tsindexer and tsplayer must match. */
	);

/**
* Free a sTsPlayer when done.
*
* Return values:
* none
**/
void tsplayer_free(
	sTsPlayer *p_tsp		/* tsplayer allocated by tsplayer_alloc() */
	);

/**
* Set a parameter. See eBpParam documentation in playertypes.h for the various parameters
* that can be set and the corresponding meanings of paramEnum and value.
*
* Return values:
* 0 = success
* -1 = failure
**/
int tsplayer_setParam(
	sTsPlayer * const p_tsp,		/* tsplayer allocated by tsplayer_alloc() */
	eBpParam param,
	int paramEnum,
	long value
	);

/**
* Get a parameter. See eBpParam documentation in playertypes.h for the various parameters
* that can be gotten and the corresponding meanings of p_paramEnum and p_value.
*
* Return values:
* 0 = success
* -1 = failure
**/
int tsplayer_getParam(
	sTsPlayer * const p_tsp,		/* tsplayer allocated by tsplayer_alloc() */
	eBpParam param,
	int * const p_paramEnum,
	long * const p_value
	);

/**
* Get the offset and size of the next chunk of mpeg to be sent to the decoder.
*
* Return values:
* 0 = success
* -1 = failure
**/
char tsplayer_getNextPlayEntry(
	sTsPlayer * const p_tsp,		/* tsplayer allocated by tsplayer_alloc() */
	sTpEntry * const p_tpEntry
	);

/**
* This function enables the debuging feature of the tsplayer.
* When debugging is enabled, tsplayer automatically compares
* the stream data with the start code data it is extracting from
* the sct file. This feature should be used with care, as it could negatively
* effect performance.
*
* Return values:
* none
**/
void tsplayer_verifyIndexWithStream(
	sTsPlayer * const p_tsp,		/* tsplayer allocated by tsplayer_alloc() */
	void *streamFp,			/* file pointer to the stream file */
	int streamIsLE			/* 0 (stream is big endian), 1 (stream is little endian) */
	);

/**
* This function dumps the values stored internally in the ts player.
* This can be used to help debug problems.
*
* Return values:
* none
**/
void tsplayer_dumpDebugInfo(
	sTsPlayer * const p_tsp		/* tsplayer allocated by tsplayer_alloc() */
	);

/**
* Set stat callback for ts player.
*
* Return values:
* 0 = success
* -1 = failure
**/
int tsplayer_set_statcb(
	sTsPlayer *p_tsp,		/* tsplayer allocated by tsplayer_alloc() */
	BP_STAT_CB statCb		/* pointer to stat function */
	);


#ifdef __cplusplus
}
#endif

#endif

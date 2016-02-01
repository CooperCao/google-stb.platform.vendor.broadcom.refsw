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
 * Created: 02/08/2001 by Marcus Kellerman
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifdef CMDLINE_TEST
#include <stdlib.h>
#include <memory.h>
#define bcmKNIMalloc	malloc
#define bcmKNIFree		free
#define BRCM_DBG_ERR(a) do {printf a; printf("\n");} while(0)
#define BRCM_DBG_WRN(a) /*do {printf a; printf("\n");} while(0)*/
#define BRCM_DBG_MSG(a) /*do {printf a; printf("\n");} while(0)*/ /* Don't print messages */
#else
#include "bcmkernel.h"
#define BRCM_DBG_MODULE_NAME "tsplayer.c"
#include "brcm_dbg.h"
#endif

#define VERSION 101 /* see playertypes.h for definition of version */

#include <stdio.h>
#include "tsplayer.h"
#include "mpeg2types.h"

/* #define PRINT_PACKETTYPES_SENT */

#define NUM_SCT_ENTRIES_TO_OVERLAP_IN_CACHE		(NUM_SCT_ENTRIES_TO_CACHE/4)

struct sTsPlayer {
#define NUM_SCT_ENTRIES_TO_CACHE	1000
	eBpPlayModeParam			playMode;
	eBpDirectionParam			playDir;
	eBpDataStartModeParam		dataStartMode;
	eBpLoopModeParam			loopMode;
	BP_READ_CB			readCb;
	BP_TELL_CB			tellCb;
	BP_SEEK_CB			seekCb;
	BP_STAT_CB 			statCb;		/* optional file stat callback */
	void				*sctFp;
	void				*streamFp;
	int					streamIsLE;
	sIndexEntry			sctCache[NUM_SCT_ENTRIES_TO_CACHE];
	long				sctCacheIndexStart;
	long				sctCacheIndexEnd;
	long				curSctIndex;
	long				numFramesToSkip;
	long				curNumSkippedBFrames;
	int					currentVersion;
};

/*---------------------------------------------------------------
- PRIVATE FUNCTIONS
---------------------------------------------------------------*/
static sIndexEntry *tsplayer_readSct( sTsPlayer * const p_tsp, long index );
static char tsplayer_getNextFrameEntry( sTsPlayer * const p_tsp, sTpEntry * const p_tpEntry, eBpPlayModeParam mode );
static long tsplayer_findSctIndexFromOffset( sTsPlayer * const p_tsp, unsigned long streamByteOffset );
static long tsplayer_findStartCodeIndex( sTsPlayer * const p_tsp, long startIndex, unsigned char startCode, eBpDirectionParam dir );
static long tsplayer_findPictureIndex( sTsPlayer * const p_tsp, long startIndex, unsigned char picCode, eBpDirectionParam dir );
static long tsplayer_findSctIndexFromPts( sTsPlayer * const p_tsp, unsigned long streamPts );
static unsigned long tsplayer_returnPts( sIndexEntry *p_sct );
static char tsplayer_verifyIndexEntry( sTsPlayer * const p_tsp, sIndexEntry *p_sct );
static int tsplayer_getIndexLimits(sTsPlayer * const p_tsp, long *pFirstIndex, long *pLastIndex);

#ifdef PRINT_PACKETTYPES_SENT
static char *pictureCodeStr[] = {
	"I",
	"P",
	"B"
};
#endif

/*! Function Definitions */

/******************************************************************************
* INPUTS:	readCb = pointer to a function that reads table entries (same format as fread)
*			seekCb = pointer to a function that seeks inside SCT file (same format as fseek)
*			sctFp = file pointer to SCT file (used in above callbacks)
* OUTPUTS:	None.
* RETURNS:	pointer to sTsPlayer structure
* FUNCTION: This function allocates and initializes an player structure.
******************************************************************************/
sTsPlayer *tsplayer_allocate( BP_READ_CB readCb, BP_TELL_CB tellCb, BP_SEEK_CB seekCb, void *sctFp, int version )
{
	sTsPlayer *p_tsp;
	p_tsp = bcmKNIMalloc( sizeof( sTsPlayer ) );
	if (tsplayer_reset(p_tsp, readCb, tellCb, seekCb, sctFp, version)) {
		bcmKNIFree(p_tsp);
		p_tsp = NULL;
	}
	return p_tsp;
}

int tsplayer_reset( sTsPlayer *p_tsp, BP_READ_CB readCb, BP_TELL_CB tellCb, BP_SEEK_CB seekCb, void *sctFp, int version )
{
	int result;

	p_tsp->readCb	= readCb;
	p_tsp->seekCb	= seekCb;
	p_tsp->tellCb	= tellCb;
	p_tsp->sctFp	= sctFp;

	p_tsp->playMode				= eBpPlayNormal;
	p_tsp->playDir				= eBpForward;
	p_tsp->dataStartMode		= eBpPlayFromTrans;
	p_tsp->loopMode				= eBpLoopForever;
	p_tsp->sctCacheIndexStart	= -1;
	p_tsp->sctCacheIndexEnd		= -1;
	p_tsp->curSctIndex			= 0;
	p_tsp->numFramesToSkip		= 0;
	p_tsp->curNumSkippedBFrames = 0;
	p_tsp->streamFp				= 0;
	p_tsp->statCb				= NULL;
	p_tsp->currentVersion		= VERSION;

	if (version == -1)
		version = VERSION;
	switch (version) {
	case VERSION:
		p_tsp->currentVersion = version;
		result = 0;
		break;
	default:
		result = -1;
		break;
	}

	return result;
}

/******************************************************************************
* INPUTS:	p_tsp = pointer to a previously allocated sTsPlayer structure
* OUTPUTS:	None.
* RETURNS:	None.
* FUNCTION: This function frees any memory used by a player structure.
******************************************************************************/
void tsplayer_free( sTsPlayer *p_tsp )
{
	bcmKNIFree( p_tsp );
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+ INPUTS:	p_tsp = pointer to a previously allocated sTsPlayer structure
+			index = index number of SCT entry you would like to read
+ OUTPUTS:	None.
+ RETURNS:	pointer to SCT entry
+ FUNCTION: This function returns a pointer to an SCT entry.  This entry is
+			stored in an internal cache, and may become invalid if another
+			tsplayer_readSct() function is called.  Therefore, any processing
+			that needs to be perfomed with the SCT entry must be done before
+			reading another one.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
sIndexEntry *tsplayer_readSct( sTsPlayer * const p_tsp, long index )
{
	long firstIndex;
	long lastIndex;

	tsplayer_getIndexLimits(p_tsp, &firstIndex, &lastIndex);
	if (firstIndex == -1 || lastIndex == -1 || index < firstIndex || index >= lastIndex)
		return 0;

	/* Check if this sct is in our cache */
	if( (p_tsp->sctCacheIndexStart == -1)
 		|| !((index >= p_tsp->sctCacheIndexStart) && (index < p_tsp->sctCacheIndexEnd)) )
	{
		unsigned long saveStart = p_tsp->sctCacheIndexStart;
		unsigned long saveEnd = p_tsp->sctCacheIndexEnd;
		unsigned long pos;
		long t;

		if( index < p_tsp->sctCacheIndexStart )
		{
			/* We are reading indexes backwards so cache backwards */
			p_tsp->sctCacheIndexStart = index - NUM_SCT_ENTRIES_TO_CACHE + NUM_SCT_ENTRIES_TO_OVERLAP_IN_CACHE + 1;
		}
		else
		{
			p_tsp->sctCacheIndexStart = index - NUM_SCT_ENTRIES_TO_OVERLAP_IN_CACHE;
		}

		if( p_tsp->sctCacheIndexStart < firstIndex)
		{
			p_tsp->sctCacheIndexStart = firstIndex;
		}

		pos = p_tsp->sctCacheIndexStart*sizeof(sIndexEntry);
		if ((*p_tsp->seekCb)( p_tsp->sctFp, pos, SEEK_SET ))
			t = -1;
		else {
			t = (*p_tsp->tellCb)( p_tsp->sctFp );
			if (t != -1)
				t /= sizeof(sIndexEntry);
		}
		/* We may not get exactly what we asked for, but it may work. */
		if (t != -1 && t <= index && t < index + NUM_SCT_ENTRIES_TO_CACHE)
			p_tsp->sctCacheIndexStart = t;
		else {
			BRCM_DBG_MSG(("TSPLAYER seek was invalidated: %ld\n", t));
			p_tsp->sctCacheIndexStart = saveStart;
			p_tsp->sctCacheIndexEnd = saveEnd;
			return 0;
		}

		p_tsp->sctCacheIndexEnd = p_tsp->sctCacheIndexStart;

		/* The end is equal to the start plus whatever we are able to read */
		p_tsp->sctCacheIndexEnd += ((*p_tsp->readCb)( p_tsp->sctCache, 1, sizeof(sIndexEntry) * NUM_SCT_ENTRIES_TO_CACHE, p_tsp->sctFp ) / sizeof(sIndexEntry));

		if( index >= p_tsp->sctCacheIndexEnd )
			return 0;
	}

	return &(p_tsp->sctCache[index-p_tsp->sctCacheIndexStart]);
}

/******************************************************************************
* INPUTS:	p_tsp = pointer to a previously allocated sTsPlayer structure
*			param = parameter that is to be set
*			paramEnum = enum value of the parameter
*			value = value that you wish to program into the paramEnum
* OUTPUTS:	None.
* RETURNS:	0 (success), -1 (fail)
* FUNCTION: This function sets a parameter of the playback
******************************************************************************/
int tsplayer_setParam( sTsPlayer * const p_tsp, eBpParam param, int paramEnum, long value )
{
	int status = 0;
	long index;

	BRCM_DBG_MSG(("tsplayer_setParam %d, %d, %ld\n", param, paramEnum, value));
	switch( param )
	{
	case eBpPlayMode:
		if( p_tsp->playMode != paramEnum )
		{
			p_tsp->playMode = paramEnum;

			/* When switching modes, always start from an I-frame (if we can find one) */
			index = tsplayer_findPictureIndex( p_tsp, p_tsp->curSctIndex, PC_I_FRAME, eBpForward );
			if( index != -1 )
			{
				p_tsp->curSctIndex = index;

				/* If possible we want to find the previous sequence header for the decoder */
				index = tsplayer_findStartCodeIndex( p_tsp, p_tsp->curSctIndex, SC_SEQUENCE, eBpReverse );
				if( index != -1 )
				{
					p_tsp->curSctIndex = index;
				}
			}
		}
		if( p_tsp->playMode == eBpPlaySkipB || p_tsp->playMode == eBpPlayI )
		{
			if( value < 0 )
			{
				value *= -1;
				p_tsp->playDir = eBpReverse;
			}
			p_tsp->numFramesToSkip = value;
			p_tsp->curNumSkippedBFrames = 0;
		}
		break;

	case eBpDirection:
		p_tsp->playDir = paramEnum;
		break;

	case eBpDataStartMode:
		p_tsp->dataStartMode = paramEnum;
		break;

	case eBpLoopMode:
		p_tsp->loopMode = paramEnum;
		break;

	case eBpCurrentStreamOffset:
		index = tsplayer_findSctIndexFromOffset( p_tsp, value );
		if( index != -1 )
			p_tsp->curSctIndex = index;
		else
			status = -1;
		break;

	case eBpCurrentStreamPts:
		index = tsplayer_findSctIndexFromPts( p_tsp, value );
		if( index != -1 )
			p_tsp->curSctIndex = index;
		else
			status = -1;
		break;

	case eBpCurrentSctIndex:
		p_tsp->curSctIndex = value;
		break;

	case eBpFindIFrame:
		value = p_tsp->curSctIndex;
		if (paramEnum < 0)
			value++;
		index = tsplayer_findPictureIndex( p_tsp, value, PC_I_FRAME, paramEnum);
		if( index == -1 )
		{
			BRCM_DBG_WRN(("Unable to find i-frame on set(currentSctIndex)\n"));
			status = -1;
		}
		else {
			BRCM_DBG_WRN(("findiframe hopped %ld\n", index-p_tsp->curSctIndex));
			p_tsp->curSctIndex = index;

			/* If possible we want to find the previous sequence header for the decoder */
			index = tsplayer_findStartCodeIndex( p_tsp, p_tsp->curSctIndex, SC_SEQUENCE, eBpReverse );
			if( index != -1 )
				p_tsp->curSctIndex = index;
		}
		break;

	case eBpStreamEnd:
		/* Jump the end of the stream */
		tsplayer_getIndexLimits(p_tsp, NULL, &index);

		/* Search back the desired number of frames */
		while( value )
		{
			/* Start at the previous start code */
			index--;

			index = tsplayer_findStartCodeIndex( p_tsp, index, SC_PICTURE, eBpReverse );
			if( index == -1 )
			{
				break;
			}

			value--;
		}

		if( index != -1 )
			p_tsp->curSctIndex = index;
		else
			status = -1;
		break;

	default:
		status = -1;
		break;
	}

	return status;
}

/******************************************************************************
* INPUTS:	p_tsp = pointer to a previously allocated sTsPlayer structure
*			param = parameter that is to be returned
* OUTPUTS:	p_paramEnum = enum value of the parameter
*			p_value = value that is currently used by paramEnum
* RETURNS:	0 (success), -1 (fail)
* FUNCTION: This function gets a parameter of the playback
******************************************************************************/
int tsplayer_getParam( sTsPlayer * const p_tsp, eBpParam param, int * const p_paramEnum, long * const p_value )
{
	sIndexEntry *p_sct;
	int status = 0;
	long index;

	switch( param )
	{
	case eBpPlayMode:
		*p_paramEnum = p_tsp->playMode;
		if( p_tsp->playMode == eBpPlaySkipB || p_tsp->playMode == eBpPlayI )
		{
			*p_value = p_tsp->numFramesToSkip;
		}
		break;

	case eBpDirection:
		*p_paramEnum = p_tsp->playDir;
		break;

	case eBpDataStartMode:
		*p_paramEnum = p_tsp->dataStartMode;
		break;

	case eBpCurrentStreamOffset:
		p_sct = tsplayer_readSct( p_tsp, tsplayer_findStartCodeIndex( p_tsp, p_tsp->curSctIndex, SC_ANY_NON_PTS, eBpForward ) );
		if (p_sct)
			*p_value = p_sct->recordByteCount;
		else
			status = -1;
		break;

	case eBpLoopMode:
		*p_paramEnum = p_tsp->loopMode;
		break;

	case eBpCurrentStreamPts:
		index = tsplayer_findStartCodeIndex( p_tsp, p_tsp->curSctIndex, SC_PTS, eBpForward );
		p_sct = tsplayer_readSct( p_tsp, index );
		if (p_sct)
			*p_value = tsplayer_returnPts( p_sct );
		else
			status = -1;
		break;

	case eBpCurrentSctIndex:
		*p_value = p_tsp->curSctIndex;
		break;

	case eBpLastSctIndex:
		tsplayer_getIndexLimits(p_tsp, NULL, p_value);
		break;

	case eBpIndexFromPts:
		*p_value = tsplayer_findSctIndexFromPts( p_tsp, *p_value );
		break;

	case eBpIndexFromOffset:
		*p_value = tsplayer_findSctIndexFromOffset( p_tsp, *p_value );
		break;

	case eBpVersion:
		*p_value = p_tsp->currentVersion;
		break;

	default:
		status = -1;
		break;
	}

	return status;
}

/******************************************************************************
* INPUTS:	p_tsp = pointer to a previously allocated sTsPlayer structure
* OUTPUTS:	p_tpEntry = filled with information regarding next playback unit
* RETURNS:	0 (success), -1 (fail)
* FUNCTION: This function returns the next playback unit.  The playback unit
*			is based upon the current direction and mode set by the functions
*			provided.
******************************************************************************/
char tsplayer_getNextPlayEntry( sTsPlayer * const p_tsp, sTpEntry * const p_tpEntry )
{
	memset( p_tpEntry, 0, sizeof( sTpEntry ) );

	switch( p_tsp->playMode )
	{
	case eBpPlayNormal:
		/* Fallthrough */
	case eBpPlayI:
		/* Falthrough */
	case eBpPlayIP:
		/* Falthrough */
	case eBpPlaySkipB:
		if( tsplayer_getNextFrameEntry( p_tsp, p_tpEntry, p_tsp->playMode ) != 0 )
            return -1;
        /* Make sure we don't return a byteCount of 0 when possible */
        if( p_tpEntry->byteCount == 0 )
		    return tsplayer_getNextFrameEntry( p_tsp, p_tpEntry, p_tsp->playMode );
        return 0;
	default:
		return -1;
	}
}

void tsplayer_dumpDebugInfo( sTsPlayer * const p_tsp )
{
	unsigned long i;

	BRCM_DBG_ERR(("TSPLAYER DEBUG INFO DUMP"));
	BRCM_DBG_ERR(("---------------------------------------"));

	BRCM_DBG_ERR((" playMode = %d", p_tsp->playMode));
	BRCM_DBG_ERR((" playDir = %d", p_tsp->playDir));
	BRCM_DBG_ERR((" dataStartMode = %d", p_tsp->dataStartMode));
	BRCM_DBG_ERR((" loopMode = %d", p_tsp->loopMode));
	BRCM_DBG_ERR((" curSctIndex = %ld", p_tsp->curSctIndex));
	BRCM_DBG_ERR((" sctCacheIndexStart = %ld", p_tsp->sctCacheIndexStart));
	BRCM_DBG_ERR((" sctCacheIndexEnd = %ld", p_tsp->sctCacheIndexEnd));

	BRCM_DBG_ERR((" SCT CACHE:"));
	for( i = 0; i < NUM_SCT_ENTRIES_TO_CACHE; i++ )
	{
		BRCM_DBG_ERR(("  %05ld: 0x%08lX 0x%08lX 0x%08lX 0x%08lX", i+p_tsp->sctCacheIndexStart, p_tsp->sctCache[i].startCodeBytes, p_tsp->sctCache[i].recordByteCount, p_tsp->sctCache[i].recordByteCountHi, p_tsp->sctCache[i].flags));
	}
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+ INPUTS:	p_tsp = pointer to a previously allocated sTsPlayer structure
+			mode = Normal, I frame or IP frame mode
+ OUTPUTS:	p_tpEntry = filled with information regarding next playback unit
+ RETURNS:	0 (success), -1 (fail)
+ FUNCTION: This function returns a filled in playback unit entry.  This
+			function is used for I or IP playback.  Depending on the data mode
+			it will return these frames aligned to transport or PES transport
+			packets.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
char tsplayer_getNextFrameEntry( sTsPlayer * const p_tsp, sTpEntry * const p_tpEntry, eBpPlayModeParam mode )
{
	sIndexEntry *p_startSct, *p_endSct, *p_sct;
	long		startIndex, endIndex;
	char		entryFound = 0;
	char		endFound = 0;
	long		loopCount = 0;
	long		numSkippedIFrames;
	long		firstIndex;

	startIndex = p_tsp->curSctIndex;

	tsplayer_getIndexLimits(p_tsp, &firstIndex, NULL);

	while( !entryFound )
	{
		if( mode == eBpPlayNormal )
		{
			if( p_tsp->playDir == eBpReverse )
			{
				BRCM_DBG_ERR(("Normal playback does not support reverse play!"));
				return -1;
			}

			/* DME: if we get caught behind the circular buffer, we must jump into it */
			if (startIndex < firstIndex) {
				firstIndex = tsplayer_findPictureIndex( p_tsp, firstIndex, PC_I_FRAME, eBpForward );
				if (firstIndex  == -1)
					return -1;
				startIndex = p_tsp->curSctIndex = firstIndex;
				BRCM_DBG_WRN(("tsplayer: Assigned startIndex to firstIndex %ld\n", startIndex));
			}

			startIndex = tsplayer_findStartCodeIndex( p_tsp, startIndex, SC_ANY_NON_PTS, eBpForward );
		}
		else if( mode == eBpPlayI )
		{
			for( numSkippedIFrames = 0; numSkippedIFrames <= p_tsp->numFramesToSkip; numSkippedIFrames++ )
			{
				startIndex = tsplayer_findPictureIndex( p_tsp, startIndex+p_tsp->playDir, PC_I_FRAME, p_tsp->playDir );

				if( startIndex == -1 )
				{
					/* Exit this for loop because we are past the stream boundaries */
					break;
				}
			}
		}
		else
		{
			char destFrameFound = 0;

			if( p_tsp->playDir == eBpReverse )
			{
				BRCM_DBG_ERR(("IP/SkipB playback not supported in reverse play!"));
				return -1;
			}

			while( !destFrameFound )
			{
				startIndex = tsplayer_findPictureIndex( p_tsp, startIndex, PC_ANY_FRAME, p_tsp->playDir );
				if( startIndex == -1 )
				{
					/* Exit this loop because we are past the stream boundaries */
					destFrameFound = 1;
					continue;
				}

				p_startSct = tsplayer_readSct( p_tsp, startIndex );
				if (!p_startSct)
					return -1;

				/* Check if this start code contains an P frame (we need to include this picture) */
				if( returnPictureCode(p_startSct->startCodeBytes) == PC_B_FRAME )
				{
					if( p_tsp->playMode == eBpPlaySkipB && p_tsp->curNumSkippedBFrames >= p_tsp->numFramesToSkip )
					{
						/* We have skipped the correct number of B frames */
						destFrameFound = 1;
						p_tsp->curNumSkippedBFrames = 0;
					}
					else
					{
						/* Keep searching past any B frames */
						p_tsp->curNumSkippedBFrames += 1;
						startIndex++;
					}
				}
				else
				{
					destFrameFound = 1;
				}
			}
		}

		if (startIndex != -1)
			endIndex = tsplayer_findPictureIndex( p_tsp, startIndex + 1, PC_ANY_FRAME, eBpForward );
		else
			endIndex = -1;

		if( startIndex == -1 || endIndex == -1 )
		{
			loopCount++;

			if( p_tsp->loopMode == eBpSinglePlay || loopCount > 1 )
			{
				BRCM_DBG_WRN(("Frame start code not found. End of file."));
				return -1;
			}
			else
			{
				long index;

				BRCM_DBG_WRN(("Frame start code not found. Looping file."));
				if( p_tsp->playDir == eBpForward )
				{
					startIndex = 0;
				}
				else
				{
					tsplayer_getIndexLimits(p_tsp, NULL, &startIndex);
				}

				/* Find the previous/next I-frame to start from (if one is found) */
				index = tsplayer_findPictureIndex( p_tsp, startIndex, PC_I_FRAME, p_tsp->playDir );
				if( index != -1 )
				{
					startIndex = index;
				}
				endFound = 0;
				continue;
			}
		}

		if( mode != eBpPlayNormal )
		{
			/* If possible we want to find the previous sequence header for the decoder */
			long prevIndex;

			prevIndex = tsplayer_findStartCodeIndex( p_tsp, startIndex, SC_SEQUENCE, eBpReverse );
			if( prevIndex != -1 && prevIndex == (startIndex - 1) )
			{
				startIndex = prevIndex;
			}

			/* The following checks are only used for transport mode (do not use them when in PES mode) */
			if( p_tsp->dataStartMode != eBpPlayFromPes )
			{
				/* We also want to start from the previous PES header if it was immediately before the current start code */
				if( (p_sct = tsplayer_readSct( p_tsp, startIndex-1 )) )
				{
					if( returnStartCode(p_sct->startCodeBytes) >= SC_PES_START
						&& returnStartCode(p_sct->startCodeBytes) <= SC_PES_END )
					{
						startIndex -= 1;
					}
				}

				/* We do not want to include the end PES header if it exists */
				if( (p_sct = tsplayer_readSct( p_tsp, endIndex-1 )) )
				{
					if( returnStartCode(p_sct->startCodeBytes) >= SC_PES_START
						&& returnStartCode(p_sct->startCodeBytes) <= SC_PES_END )
					{
						endIndex -= 1;
					}
				}
			}
		}

		entryFound = 1;
	}

	if (firstIndex > startIndex) {
		BRCM_DBG_WRN(("bad start index, cache was bad\n"));
		return -1;
	}

	/* We must store the data for the start before we read the end sct, due to cache coherency */
	p_startSct = tsplayer_readSct( p_tsp, startIndex );
	if( tsplayer_verifyIndexEntry( p_tsp, p_startSct ) )
	{
		BRCM_DBG_ERR(("tsplayer_getNextFrameEntry(): Error with index entry: %ld!", startIndex));
		return -1;
	}

	p_tpEntry->startOffset = p_startSct->recordByteCount;
	p_tpEntry->startOffsetHi = (p_startSct->recordByteCountHi >> 24);


	p_endSct = tsplayer_readSct( p_tsp, endIndex );
	if( tsplayer_verifyIndexEntry( p_tsp, p_endSct ) )
	{
		BRCM_DBG_ERR(("tsplayer_getNextFrameEntry(): Error with index entry: %ld!", endIndex));
		return -1;
	}

	/* Check if we have wrapped 32 bits of byte count */
	if( p_endSct->recordByteCount < p_tpEntry->startOffset )
	{
		p_tpEntry->byteCount = 0xFFFFFFFF - p_tpEntry->startOffset;
		p_tpEntry->byteCount += p_endSct->recordByteCount;
	}
	else
	{
		p_tpEntry->byteCount = p_endSct->recordByteCount - p_tpEntry->startOffset;
	}

#ifdef PRINT_PACKETTYPES_SENT
	{
	int pc = returnPictureCode(p_startSct->startCodeBytes);
	if (pc >= 1 && pc <= 3)
		BRCM_DBG_ERR(("sent %s (%ld %ld pts 0x%lx)\n",
			pictureCodeStr[pc-1],
			p_startSct->recordByteCount,
			p_tpEntry->byteCount,
			tsplayer_returnPts(p_startSct)));
	else
		BRCM_DBG_ERR(("sent unknown %d\n", pc));
	}
#endif

	if( p_tsp->playDir == eBpForward )
	{
		p_tsp->curSctIndex = endIndex;
	}
	else
	{
		p_tsp->curSctIndex = startIndex;
	}

	/* Don't allow byte counts larger than 10 mbytes */
	if( p_tpEntry->byteCount > 1024 * 1024 * 10 )
	{
		BRCM_DBG_ERR(("tsplayer_getNextFrameEntry(): Invalid byte count of: %ld found!", p_tpEntry->byteCount));
		p_tpEntry->byteCount = 0;
		return -1;
	}

	return 0;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+ INPUTS:	p_tsp = pointer to a previously allocated sTsPlayer structure
+			streamByteOffset = offset in stream search for
+ OUTPUTS:	None.
+ RETURNS:	SCT index covering this stream offset
+ FUNCTION: This function searches the SCT table for an entry that occupies
+			this portion of the stream.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
long tsplayer_findSctIndexFromOffset( sTsPlayer * const p_tsp, unsigned long streamByteOffset )
{
	sIndexEntry		*p_sct;
	char			sctFound = 0;
	unsigned long	prevRecOffset = 0;
	long			index, tempIndex;

	/* need to search in either forward or reverse direction for startcode index */
	eBpDirectionParam dir;

	/* Start from our current index */
	index = p_tsp->curSctIndex;

	if( index == -1 )
	{
		index = 0;
		prevRecOffset = 0;
	}
	else
	{
		index = tsplayer_findStartCodeIndex( p_tsp, index, SC_ANY_NON_PTS, eBpForward );

		/* Initialize our prevRecOffset to our current offset */
		if( !(p_sct = tsplayer_readSct( p_tsp, index )) )
		{
			BRCM_DBG_WRN(("Unable to find SCT entry %ld (1)", index));
			return -1;
		}
		prevRecOffset = p_sct->recordByteCount;
	}

	dir = eBpForward;
	while( !sctFound )
	{
		index = tsplayer_findStartCodeIndex( p_tsp, index, SC_ANY_NON_PTS, dir);
		if (index <= 0)
		{
			index = 0;
			break;
   		}

		if( !(p_sct = tsplayer_readSct( p_tsp, index )) )
		{
			BRCM_DBG_WRN(("Unable to find SCT entry %ld (2)", index));
			return -1;
		}

		if( streamByteOffset >= prevRecOffset && streamByteOffset <= p_sct->recordByteCount )
		{
			sctFound = 1;
		}
		else if( p_sct->recordByteCount < streamByteOffset )
		{
			dir = eBpForward;
			index += 1;
		}
		else
		{
			dir = eBpReverse;
			index -= 1;
			if( index == 0 )
			{
				sctFound = 1;
			}
		}

		prevRecOffset = p_sct->recordByteCount;
	}

	/*
	 * Since there may be several start codes for the same offset we need to start
	 * at the first one of them
	 */
	tempIndex = index;
	while( sctFound )
	{
		tempIndex = tsplayer_findStartCodeIndex( p_tsp, tempIndex-1, SC_ANY_NON_PTS, eBpReverse );

		p_sct = tsplayer_readSct( p_tsp, tempIndex );
		if( !p_sct )
		{
			break;
		}

		if( p_sct->recordByteCount == prevRecOffset )
		{
			/* This is still an index that is valid so make sure to store it */
			index = tempIndex;
		}
		else
		{
			/* 
			 * We went on index beyond the one we are looking for... The real index
			 * is stored in index so we can now quit searching.
			 */
			sctFound = 0;
		}
	}

	return index;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+ INPUTS:	p_tsp = pointer to a previously allocated sTsPlayer structure
+			streamPts = PTS in stream search for
+ OUTPUTS:	None.
+ RETURNS:	SCT index covering this PTS
+ FUNCTION: This function searches the SCT table for an entry that occupies
+			this portion of the stream.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
long tsplayer_findSctIndexFromPts( sTsPlayer * const p_tsp, unsigned long streamPts )
{
	sIndexEntry		*p_sct;
	char			sctFound = 0;
	long			index;
	unsigned long	entryPts;
	eBpDirectionParam		deltaIndex;

	/* Start from our current index */
	index = p_tsp->curSctIndex;

	if( index == -1 )
	{
		index = 0;
		deltaIndex = eBpForward;
	}
	else
	{
		deltaIndex = eBpReverse;
	}

	/*  This loop works as follows :
	    1) start by going backwards
		2) once we've gone too far, turn around
		3) go forward until we pass the desired pts by one then return */
	while( (!sctFound) && (index != 0) )
	{
		index = tsplayer_findStartCodeIndex( p_tsp, index, SC_PTS, deltaIndex );
		if( index == -1 )
		{
			BRCM_DBG_WRN(("Unable to find SCT entry %ld (3)", index));
			return -1;
		}

		p_sct = tsplayer_readSct( p_tsp, index );
		if (!p_sct)
			return -1;
		entryPts = tsplayer_returnPts( p_sct );

		if( deltaIndex == eBpForward && streamPts <= entryPts )
		{
			sctFound = 1;
			continue;
		}
		else if( entryPts < streamPts )
		{
			deltaIndex = eBpForward;
		}
		else
		{
			deltaIndex = eBpReverse;
		}

		index += deltaIndex;
	}

	return index;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+ INPUTS:	p_tsp = pointer to a previously allocated sTsPlayer structure
+			startIndex = index in stream to begin search
+			startCode = start code to search for
+			dir = direction to search
+ OUTPUTS:	None.
+ RETURNS:	SCT index matching this start code
+ FUNCTION: This function searches the SCT table for a matching start code.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
long tsplayer_findStartCodeIndex( sTsPlayer * const p_tsp, long startIndex, unsigned char startCode, eBpDirectionParam dir )
{
	sIndexEntry		*p_sct;
	char			startCodeFound = 0;

	while( !startCodeFound )
	{
		if( !(p_sct = tsplayer_readSct( p_tsp, startIndex )) )
		{
			BRCM_DBG_WRN(("Unable to find SCT entry %ld (4)", p_tsp->curSctIndex));
			return -1;
		}

		if( startCode == SC_ANY_NON_PTS && returnStartCode(p_sct->startCodeBytes) != SC_PTS )
		{
			startCodeFound = 1;
		}
		else if( startCode == SC_PES )
		{
			if( returnStartCode(p_sct->startCodeBytes) >= SC_PES_START
				&& returnStartCode(p_sct->startCodeBytes) <= SC_PES_END )
			{
				startCodeFound = 1;
			}
			else
			{
				startIndex += dir;
			}
		}
		else if( returnStartCode(p_sct->startCodeBytes) == startCode )
		{
			startCodeFound = 1;
		}
		else
		{
			startIndex += dir;
		}
	}

	return startIndex;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+ INPUTS:	p_tsp = pointer to a previously allocated sTsPlayer structure
+			startIndex = index in stream to begin search
+			picCode = picture code to search for
+			dir = direction to search
+ OUTPUTS:	None.
+ RETURNS:	SCT index matching this picture code
+ FUNCTION: This function searches the SCT table for a matching picture code.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
long tsplayer_findPictureIndex( sTsPlayer * const p_tsp, long startIndex, unsigned char picCode, eBpDirectionParam dir )
{
	sIndexEntry *p_sct = 0;
	char		pictureFound = 0;
	unsigned char	foundPictureCode;

	/* Always start at the previous index when searching backwards */
	if( dir == eBpReverse )
	{
		startIndex -= 1;
	}

	while( !pictureFound )
	{
		startIndex = tsplayer_findStartCodeIndex( p_tsp, startIndex, SC_PICTURE, dir );
		if( startIndex == -1 )
		{
			BRCM_DBG_WRN(("Unable to find Picture!"));
			return -1;
		}

		p_sct = tsplayer_readSct( p_tsp, startIndex );
		if (!p_sct)
			return -1;

		foundPictureCode = returnPictureCode(p_sct->startCodeBytes);

		/* Make sure this is a valid picture code type! */
		if( (foundPictureCode != PC_I_FRAME)
			&& (foundPictureCode != PC_B_FRAME)
			&& (foundPictureCode != PC_P_FRAME) )
		{
			BRCM_DBG_WRN(("Found invalid picture code: 0x%0X in SC Num: %ld", foundPictureCode, startIndex));
		}
		else if( picCode == PC_ANY_FRAME || foundPictureCode == picCode )
		{
			pictureFound = 1;
			continue;
		}
		else
		{
			BRCM_DBG_MSG(("Looking for picCode %d.  Found frame type %d.", picCode, foundPictureCode ));
		}
		startIndex += dir;
	}
	return startIndex;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+ INPUTS:	p_sct = pointer to sct entry
+ OUTPUTS:	None.
+ RETURNS:	pts value (bits [32:1]
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
unsigned long tsplayer_returnPts( sIndexEntry *p_sct )
{
	unsigned long pts;
	
	if( p_sct )
	{
		/* Calculate PTS[32:1] */
		pts = p_sct->startCodeBytes & 0x1;
		pts = (pts << 31) + (p_sct->recordByteCount >> 1);
	}
	else
	{
		pts = 0;
	}

	return pts;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+ INPUTS:	p_tsp = pointer to a previously allocated sTsPlayer structure
+ OUTPUTS:	*pFirstIndex - first index
*			*pLastIndex - last index
+ RETURNS:	0 - no errors detected
*			-1 - error detected
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int
tsplayer_getIndexLimits(sTsPlayer * const p_tsp, long *pFirstIndex, long *pLastIndex)
{
	long firstByteOffset, lastByteOffset;
	int rc;

	if (p_tsp->statCb) { /* if stat cb defined operation is straight forward */
		rc = p_tsp->statCb(p_tsp->sctFp, &firstByteOffset, &lastByteOffset);
		if (rc<0) {
			goto error;
		}
	} else {
		/* load only required offsets */
		if (pFirstIndex) {
			rc = p_tsp->seekCb( p_tsp->sctFp, 0, SEEK_SET );
			if (rc) {
				goto error;
			}
			firstByteOffset = p_tsp->tellCb( p_tsp->sctFp );
			if  (firstByteOffset<0) {
				goto error;
			}
		}
		if (pLastIndex) {
			/* Jump to end of the file */
			rc = p_tsp->seekCb( p_tsp->sctFp, 0, SEEK_END );
			if (rc) {
				goto error;
			}
			lastByteOffset = p_tsp->tellCb( p_tsp->sctFp );
			lastByteOffset++;
		}
	}
	if (pFirstIndex) {
		*pFirstIndex = (firstByteOffset + sizeof(sIndexEntry)-1)/sizeof(sIndexEntry);
	}
	if (pLastIndex) {
		*pLastIndex = ((lastByteOffset)/sizeof(sIndexEntry)) - 1;
	}
	return 0;
error:
	if (pFirstIndex) {
		*pFirstIndex = -1;
	}
	if (pLastIndex) {
		*pLastIndex = -1;
	}
	return -1;
}


void tsplayer_verifyIndexWithStream( sTsPlayer * const p_tsp, void *streamFp, int streamIsLE )
{
	p_tsp->streamFp	= streamFp;
	p_tsp->streamIsLE = streamIsLE;

	if( p_tsp->streamIsLE )
	{
		BRCM_DBG_ERR(("tsp_verifyIndexWithStream(): This stream will not be checked due to LE format!"));
	}
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+ INPUTS:	p_tsp = pointer to a previously allocated sTsPlayer structure
+			p_sct = index entry to verify
+ OUTPUTS:	None.
+ RETURNS:	0 (no error), -1 (error)
+ FUNCTION: This function verifies that the start code 
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
char tsplayer_verifyIndexEntry( sTsPlayer * const p_tsp, sIndexEntry *p_sct )
{
	if (!p_sct || !p_tsp)
		return -1;

	/* Check stream to make sure the start code matches the stream */
	if( p_tsp->streamFp && p_sct->recordByteCountHi == 0 )
	{
		unsigned char data[4];
		unsigned long startCode;

		if( p_tsp->streamIsLE )
		{
			/* Verifing a LE stream is a pain, as lots of byte swapping and alignment checking is needed.
			 * Since only old streams will be LE, don't bother to check streams of this format...
			 */
			return 0;
		}

		if( (p_sct->startCodeBytes&0xFF) > 184 )
		{
			BRCM_DBG_WRN(("tsp_verifyIndexEntry(): Start code spans transport packet! Unable to verify."));
			return 0;
		}

		if ((*p_tsp->seekCb)( p_tsp->streamFp, p_sct->recordByteCount+(p_sct->startCodeBytes&0xFF), SEEK_SET ))
			return 0;
		if( (*p_tsp->readCb)( data, 1, 4, p_tsp->streamFp ) != 4 )
		{
			BRCM_DBG_ERR(("tsp_verifyIndexEntry(): Unable to read at stream offset: %ld!", p_sct->recordByteCount+(p_sct->startCodeBytes&0xFF)));
			return 0;
		}

		/* Do this in an endian neutral fashion */
		startCode = data[0];
		startCode <<= 8;
		startCode |= data[1];
		startCode <<= 8;
		startCode |= data[2];
		startCode <<= 8;
		startCode |= data[3];

		if( startCode != (0x00000100 | ((p_sct->startCodeBytes >> 24) & 0xFF)) )
		{
			BRCM_DBG_ERR(("tsp_verifyIndexEntry(): Start code mismatch at stream offset: %ld!", p_sct->recordByteCount+(p_sct->startCodeBytes&0xFF)));
			BRCM_DBG_ERR(("tsp_verifyIndexEntry(): Expecting 0x%08lX, Got 0x%08lX", (0x00000100 | ((p_sct->startCodeBytes >> 24) & 0xFF)), startCode));
			return -1;
		}
	}
	return 0;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+ INPUTS:	p_tsp = pointer to a previously allocated sTsPlayer structure
+			statCb = pointer to stat function
+ OUTPUTS:	None.
+ RETURNS:	0 (no error), -1 (error)
+ FUNCTION: This function verifies that the start code 
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int 
tsplayer_set_statcb( sTsPlayer *p_tsp, BP_STAT_CB statCb)
{
	p_tsp->statCb = statCb;
	return 0;
}


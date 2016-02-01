/***************************************************************************
 *     Copyright (c) 2002-2013, Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/
#ifndef TSPSIMGR_H__
#define TSPSIMGR_H__

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"

#include "nexus_core_utils.h"
#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_frontend.h"
#include "nexus_pid_channel.h"
#include "nexus_video_types.h"
#include "nexus_audio_types.h"
#include "nexus_message.h"
#include "nexus_playback.h"
#include "nexus_playpump.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ISO936_CODE_LENGTH 3
#define MAX_PROGRAM_CC_SERVICE	32
typedef struct
{
	uint16_t	pid;
	uint8_t		streamType;
	uint16_t  	ca_pid;
	unsigned char iso639[ISO936_CODE_LENGTH];
} EPID;

typedef struct
{
	uint8_t		ccType;
	uint8_t		ccService;
	unsigned char iso639[ISO936_CODE_LENGTH];
} ECC;

#define MAX_PROGRAM_MAP_PIDS	12
typedef struct
{
	uint16_t	program_number;
	uint16_t	map_pid;
	uint8_t		version;
	uint8_t		broadcast_flag;
	uint8_t 	num_cc;
	ECC			cc[MAX_PROGRAM_CC_SERVICE];
	uint16_t	pcr_pid;
	uint16_t  	ca_pid;
	uint8_t		num_video_pids;
	EPID		video_pids[MAX_PROGRAM_MAP_PIDS];
	uint8_t		num_audio_pids;
	EPID		audio_pids[MAX_PROGRAM_MAP_PIDS];
	uint8_t		num_other_pids;
	EPID		other_pids[MAX_PROGRAM_MAP_PIDS];
	uint32_t	pmt_size;
	uint8_t		*pmt;
} PROGRAM_INFO_T;

#define MAX_PROGRAMS_PER_CHANNEL 64
typedef struct
{
	uint8_t		version;
	uint16_t	transport_stream_id;
	uint32_t	sectionBitmap;
	uint16_t	num_programs;
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
int  tsPsi_getChannelInfo( CHANNEL_INFO_T *p_chanInfo, NEXUS_ParserBand parserBand);


int  tsPsi_getProgramInfo(PROGRAM_INFO_T *p_pgInfo, unsigned pg_number, NEXUS_ParserBand parserBand, unsigned char *pmt, unsigned int *pmt_size );
/**
Set the timeout values for various blocking operations in tspsimgr.
**/
void tsPsi_setTimeout( int patTimeout, int pmtTimeout );

/**
Get the timeout values for various blocking operations in tspsimgr.
**/
void tsPsi_getTimeout( int *patTimeout, int *pmtTimeout );

/**
Get the timeout counts for PAT and PMT.
**/
int tsPsi_getTimeoutCnt( int *patTimeoutCount, int *pmtTimeoutCount );

/**
Reset the timeout counts for PAT and PMT.
**/
void tsPsi_resetTimeoutCnt( void );

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
int tsPsi_getPAT(NEXUS_ParserBand parserBand, void *buffer, unsigned bufferSize );

/**
Callback used by tsPsi_getPMTs
**/
typedef void (*tsPsi_PMT_callback)(void *context, uint16_t pmt_pid, const void *pmt, unsigned pmtSize);

/**
Summary:
Read PMT's for each program specified in the PAT.

Description:
This will launch multiple NEXUS messages and call the callback as each
PMT is read.
**/
int 	tsPsi_getPMTs(NEXUS_ParserBand parserBand,
	const void *pat, unsigned patSize,
	tsPsi_PMT_callback callback, void *context);


/**
Summary:
Parse a PMT structure into a PROGRAM_INFO_T structure.
**/
void tsPsi_parsePMT(const void *pmt, unsigned pmtSize, PROGRAM_INFO_T *p_programInfo);

#ifdef __cplusplus
}
#endif

#endif /* TSPSIMGR_H__ */

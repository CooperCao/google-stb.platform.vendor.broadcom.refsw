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
 * Module Description:
 *
 ***************************************************************************/
#include "bstd.h"
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include "tsindexer.h"
#include "mpeg2types.h"
BDBG_MODULE(tsindexer);

#define VERSION 0101

struct sTsIndexer
{
#define VOB_PACKET_SIZE         2048
#define TS_MAX_PACKET_SIZE      204
#define TS_BASE_PACKET_SIZE     188
#define NUM_PES_BYTES_TO_CACHE      30

      tsindex_settings settings;

      unsigned char     pesStreamId;
      char              PesParseMode;           /* Are we parsing PES or TS - used to determine proper offsets */
      char              validPesPacket;         /* Determines if the start codes for this pes packet with be stored */

      unsigned char     TransPacket[TS_MAX_PACKET_SIZE];    /* Cached transport packet */
      unsigned long     TransPacketValidCount;          /* Number of valid bytes in the cached transport packet */

      uint32_t          TransRecordByteCountHi;         /* Running counter of number of bytes recorded (hi word) */
      uint32_t          TransRecordByteCount;           /* Running counter of number of bytes recorded */
      unsigned long     TransByteOffset;                    /* Running count of number of bytes processed in cur packet */

      char              PesMode;                        /* 1 (Parsing PES header), 0 (Parsing ES data) */
      unsigned long     PesHeaderByteCount;             /* Num of pes header bytes cached */
      unsigned long     PesHeaderSize;          /* Total size of pes header */
      char              PesPtsFlag;                     /* Flag indicated PTS present */
      unsigned char     PesHeader[NUM_PES_BYTES_TO_CACHE];

      char              EsStoreNextSlice;                   /* 1 (Next Slice SC will the stored), 0 (next Slice SC will be ignored) */
      char              EsStoreNextNonSlice;            /* 1 (Next non-Slice SC will the stored), 0 (next non-Slice SC will be ignored) */
      unsigned long     EsNumZeroBytes;         /* Running count of number of consecutive bytes of zero */
      unsigned long     EsNumStartCodeBytes;            /* Running count of bytes extracted of current start code */
      unsigned long     EsRecordByteCount;              /* Offset into total record of last found zero */
      unsigned long     EsStartCodeBytes;               /* Offset into trans packet of last found zero */
      unsigned long     savePacketOffset;               /* save the value of EsRecordByteCount at the time
                                                           we might be beginning a start code */
      unsigned long     saveStreamOffset;               /* save the value of EsStartCodeBytes at the time
                                                           we might be beginning a start code */

      sIndexEntry           curEntry;                   /* Start Code entry that gets filled in as indexer goes */
      sSixWordIndexEntry    sixWordEntry;

      int               currentVersion;
      unsigned long     actualPesStreamId;
      unsigned long     sctType;
};

/*---------------------------------------------------------------
- PRIVATE FUNCTIONS
---------------------------------------------------------------*/
static unsigned char    tsindex_readByte( const unsigned char *p_bfr, const long offset );
static unsigned short   tsindex_readShort( const unsigned char *p_bfr, const long offset );
static unsigned long    tsindex_readLong( const unsigned char *p_bfr, const long offset );
static long             tsindex_processPes( sTsIndexer * const p_tsi, const unsigned char *p_bfr, const long offset, long numBytes );
static long             tsindex_processEs( sTsIndexer * const p_tsi, const unsigned char *p_bfr, const long offset, long numBytes );

/*! Function Definitions */

/******************************************************************************
* INPUTS:   cb = pointer to a function that stores table entries (same format as fwrite)
*           fp = general pointer that is passed in as param 3 into cb function
*           pid = pid of the transport stream used for table generation
* OUTPUTS:  None.
* RETURNS:  pointer to sTsIndexer structure
* FUNCTION: This function allocates and initializes an indexer structure.
******************************************************************************/
sTsIndexer *tsindex_allocate( INDEX_WRITE_CB cb, void *fp, unsigned short pid, int version )
{
    tsindex_settings settings;
    tsindex_settings_init(&settings);
    settings.cb = cb;
    settings.fp = fp;
    settings.pid = pid;
    settings.version = version;
    return tsindex_allocate_ex(&settings);
}


int tsindex_reset( sTsIndexer * p_tsi, INDEX_WRITE_CB cb, void *fp, unsigned short pid, int version )
{
    tsindex_settings settings;
    tsindex_settings_init(&settings);
    settings.cb = cb;
    settings.fp = fp;
    settings.pid = pid;
    settings.version = version;
    return tsindex_reset_ex(p_tsi, &settings);
}

void tsindex_settings_init(tsindex_settings *settings)
{
    memset(settings, 0, sizeof(*settings));
    settings->version = 0101;
    settings->entry_size = 4;
    settings->start_code_lo = 0xB0;
    settings->start_code_hi = 0xB8;
    settings->ts_packet_size = TS_BASE_PACKET_SIZE; /* default transport packet size */
}

/**
Summary:
Allocate a tsindex object using the settings structure.
Description:
This is preferred to tsindex_allocate because it can be extended. The old
API is fixed.
**/
sTsIndexer  *tsindex_allocate_ex(const tsindex_settings *settings)
{
    sTsIndexer *p_tsi;
    p_tsi = (sTsIndexer *)malloc( sizeof( sTsIndexer ) );
    if (tsindex_reset_ex(p_tsi, settings)) {
        free(p_tsi);
        p_tsi = NULL;
    }
    return p_tsi;
}

int tsindex_reset_ex(sTsIndexer *p_tsi, const tsindex_settings *settings)
{
    int result;

    if (settings->ts_packet_size > TS_MAX_PACKET_SIZE || settings->ts_packet_size < TS_BASE_PACKET_SIZE) {
        return -1;
    }

    memset( p_tsi, 0, sizeof( sTsIndexer ) );

    p_tsi->settings = *settings;

    p_tsi->PesMode = 0;
    p_tsi->pesStreamId = 0xE0; /* only most-significant nibble matters */
    p_tsi->validPesPacket = 0;

    /* Store any type of start code in the beginning */
    p_tsi->EsStoreNextSlice = 1;
    p_tsi->EsStoreNextNonSlice = 1;

    switch (settings->version) {
    case -1:
    case VERSION:
        p_tsi->currentVersion = settings->version;
        result = 0;
        break;
    default:
        result = -1;
        break;
    }
    return result;
}

/******************************************************************************
* INPUTS:   p_tsi = pointer to a previously allocated sTsIndexer structure
* OUTPUTS:  None.
* RETURNS:  None.
* FUNCTION: This function frees any memory used by an indexer structure.
******************************************************************************/
void tsindex_free( sTsIndexer *p_tsi )
{
    free( (void *)p_tsi );
}

static void tsindex_p_callback(sTsIndexer * const p_tsi)
{
    if (p_tsi->settings.entry_size == 6) {
        /* merge curEntry into sixWordEntry. by deferring this until we're ready to fire callback,
        it simplifies calling code. */
        p_tsi->sixWordEntry.word0 = p_tsi->sctType | (p_tsi->settings.pid<<8);
        p_tsi->sixWordEntry.word1 = 0;
        p_tsi->sixWordEntry.startCodeBytes = p_tsi->curEntry.startCodeBytes;
        p_tsi->sixWordEntry.recordByteCount = p_tsi->curEntry.recordByteCount;
        p_tsi->sixWordEntry.recordByteCountHi = p_tsi->curEntry.recordByteCountHi;
        p_tsi->sixWordEntry.flags = p_tsi->curEntry.flags;
        (*p_tsi->settings.cb)(&p_tsi->sixWordEntry, 1, sizeof( sSixWordIndexEntry ), p_tsi->settings.fp );
        memset(&p_tsi->sixWordEntry, 0, sizeof(p_tsi->sixWordEntry));
        memset(&p_tsi->curEntry, 0, sizeof(p_tsi->curEntry));
    }
    else {
        (*p_tsi->settings.cb)(&(p_tsi->curEntry), 1, sizeof( sIndexEntry ), p_tsi->settings.fp );
    }
}

/******************************************************************************
* INPUTS:   p_tsi = pointer to a previously allocated sTsIndexer structure
*           pesStreamId = stream id of the pes packet that is to be indexed (default is 0xE0)
* OUTPUTS:  None.
* RETURNS:  None.
* FUNCTION: This function frees any memory used by an indexer structure.
******************************************************************************/
void tsindex_setPesId( sTsIndexer *p_tsi, unsigned char pesStreamId )
{
    /* only most-significant nibble matters */
    p_tsi->pesStreamId = pesStreamId;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+ INPUTS:   p_bfr = buffer pointer,
+           offset  = offset into the buffer in bytes
+ OUTPUTS:  None.
+ RETURNS:  This function returns an 8 bit value.
+ FUNCTION: Return an 8 bit value assuming the proper byte swapping. Assumes offset
+           is in bounds and p_bfr ends on a 4 byte boundry.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
unsigned char tsindex_readByte( const unsigned char *p_bfr, const long offset )
{
#ifdef SWAPPED_STREAM
    unsigned long offset_to_word = (offset / 4) * 4;
    unsigned long rem = (offset % 4);
    return (unsigned char)p_bfr[offset_to_word + (3 - rem)];
#else
    return (unsigned char)p_bfr[offset];
#endif
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+ INPUTS:   p_bfr = buffer pointer
+           offset  = offset into the buffer in bytes
+ OUTPUTS:  None.
+ RETURNS:  16 bit value
+ FUNCTION: This function returns a 16 bit value from the first two bytes
+           pointed to by p_bfr if p_bfr is big endian and returns the 16
+           bit value that would have been return if the stream was big endian if
+           the stream has every 32-bit word byte swapped.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#ifdef SWAPPED_STREAM
static char smap[4][2] = { {3,2}, {2,1}, {1,0}, {0,7} };
#endif
unsigned short tsindex_readShort( const unsigned char *p_bfr, const long offset )
{
    unsigned short value;
#ifdef SWAPPED_STREAM
    unsigned long offset_to_word = (offset / 4) * 4;
    unsigned long word_offset = (offset % 4);

    value = p_bfr[offset_to_word + smap[word_offset][0]];
    value <<= 8;
    value |= p_bfr[offset_to_word + smap[word_offset][1]];
#else
    value = p_bfr[offset + 0];
    value <<= 8;
    value |= p_bfr[offset + 1];
#endif

    return value;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+ INPUTS:   p_bfr = buffer pointer
+           offset  = offset into the buffer in bytes
+ OUTPUTS:  None.
+ RETURNS:  32 bit value
+ FUNCTION: This function returns a 32 bit value from the first four bytes
+           pointed to by p_bfr if p_bfr is big endian and returns the 32
+           bit value that would have been return if the stream was big endian if
+           the stream has every 32-bit word byte swapped.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#ifdef SWAPPED_STREAM
static const char lmap[4][4] = { {3,2,1,0}, {2,1,0,7}, {1,0,7,6}, {0,7,6,5} };
#endif
unsigned long tsindex_readLong( const unsigned char *p_bfr, const long offset )
{
    unsigned long value;
#ifdef SWAPPED_STREAM
    unsigned long offset_to_word = (offset / 4) * 4;
    unsigned long word_offset = (offset % 4);

    value = p_bfr[offset_to_word + lmap[word_offset][0]];
    value <<= 8;
    value |= p_bfr[offset_to_word + lmap[word_offset][1]];
    value <<= 8;
    value |= p_bfr[offset_to_word + lmap[word_offset][2]];
    value <<= 8;
    value |= p_bfr[offset_to_word + lmap[word_offset][3]];
#else
    value = p_bfr[offset + 0];
    value <<= 8;
    value |= p_bfr[offset + 1];
    value <<= 8;
    value |= p_bfr[offset + 2];
    value <<= 8;
    value |= p_bfr[offset + 3];
#endif
    return value;
}

/******************************************************************************
* INPUTS:   p_tsi = pointer to a previously allocated sTsIndexer structure
*           p_bfr = buffer containing transport data
*           numBytes = number of bytes contained in p_bfr
* OUTPUTS:  None.
* RETURNS:  Number of packets processed.
* FUNCTION: This function parses the transport data and stores the start code
*           tables through the previously installed cb function.
******************************************************************************/
long tsindex_feed( sTsIndexer * const p_tsi, const unsigned char *p_bfr, long numBytes )
{
    unsigned short pid;
    const unsigned char *p_curBfr;
    long i = 0;
    int timestamp_size = p_tsi->settings.ts_packet_size - TS_BASE_PACKET_SIZE;

    /* We are parsing transport packets */
    p_tsi->PesParseMode = 0;

    BDBG_MSG(("feed %ld", numBytes));
    while( i < numBytes )
    {
        /* Check if we have a partial packet cached */
        if( p_tsi->TransPacketValidCount
            || ((unsigned)(numBytes - i) < p_tsi->settings.ts_packet_size) )
        {
            /* Fill the rest of our transport cache */
            while( i < numBytes )
            {
                p_tsi->TransPacket[p_tsi->TransPacketValidCount] = p_bfr[i];

                p_tsi->TransPacketValidCount += 1;
                i++;

                if( p_tsi->TransPacketValidCount == p_tsi->settings.ts_packet_size )
                    break;
            }

            /* We do not have enough data to cache the entire packet, so return */
            if( p_tsi->TransPacketValidCount != p_tsi->settings.ts_packet_size )
            {
                return i;
            }
            else
            {
                p_curBfr = p_tsi->TransPacket;
                p_tsi->TransPacketValidCount = 0;
            }
        }
        else
        {
            p_curBfr = &(p_bfr[i]);
            i += p_tsi->settings.ts_packet_size;
        }

        p_tsi->TransByteOffset = 0;
        p_curBfr += timestamp_size;

        /* make sure this is a trasport packet */
        if( tsindex_readByte(p_curBfr,0) != 0x47 )
        {
            if( p_tsi->TransRecordByteCountHi ) {
                BDBG_ERR(("  bad packet hi %d", p_tsi->TransRecordByteCountHi));
            }
            BDBG_ERR(("  bad packet %d", p_tsi->TransRecordByteCount));
            goto next_packet;
        }

        /* Does this pid match the one we are indexing? */
        pid = tsindex_readShort(p_curBfr,TS_PID_BYTE) & TS_PID_MASK;
        BDBG_MSG(("  pid 0x%x", pid));

        if( pid == p_tsi->settings.pid )
        {
            /* we need to determine if we are currently parsing a PES packet or ES data */
            if( tsindex_readByte(p_curBfr,TS_PAYLOAD_UNIT_START_BYTE) & TS_PAYLOAD_UNIT_START_MASK )
            {
                /* PR 16210 - if currently capturing start code payload, fire callback.
                However, should we fire here, or when we actually detect the PES packet startcode? This
                might need to change in the future. */
                if (p_tsi->EsNumStartCodeBytes > 0) {
                    tsindex_p_callback(p_tsi);
                    p_tsi->EsNumStartCodeBytes = 0;
                }

                p_tsi->PesMode = 1;
                p_tsi->PesHeaderByteCount = 0;
                p_tsi->PesHeaderSize = 0;
            }
            else
            {
                p_tsi->PesMode = 0;
            }

            /* Now we need to find where our payload begins */
            p_tsi->TransByteOffset = 4;

            /* Check for an adaptation field */
            if( ((tsindex_readByte(p_curBfr,TS_ADAPT_BYTE) & TS_ADAPT_MASK) == 0x20)
                || ((tsindex_readByte(p_curBfr,TS_ADAPT_BYTE) & TS_ADAPT_MASK) == 0x30) )
            {
                if(p_tsi->settings.is_avc
                        && tsindex_readByte(p_curBfr,TS_ADAPT_LENGTH_BYTE)
                        && (tsindex_readByte(p_curBfr,TS_RAI_BYTE) & TS_RAI_MASK)){
                    p_tsi->curEntry.startCodeBytes = (1<<5);
                    p_tsi->sctType = (1<<24);
                    tsindex_p_callback(p_tsi);
                    p_tsi->sctType = 0;
                }

                /* We need to skip past the adaptation field */
                p_tsi->TransByteOffset += tsindex_readByte(p_curBfr,TS_ADAPT_LENGTH_BYTE) + 1;
            }

            if( p_tsi->PesMode )
            {
                p_tsi->curEntry.recordByteCount = p_tsi->TransRecordByteCount;
                p_tsi->curEntry.recordByteCountHi &= REC_BYTE_COUNT_HI_MASK;
                p_tsi->curEntry.recordByteCountHi |= p_tsi->TransRecordByteCountHi<<REC_BYTE_COUNT_HI_OFFSET;
                p_tsi->curEntry.startCodeBytes = p_tsi->TransByteOffset;
                BDBG_MSG(("  processPes"));
                tsindex_processPes( p_tsi, p_curBfr, p_tsi->TransByteOffset,
                    p_tsi->settings.ts_packet_size - p_tsi->TransByteOffset - timestamp_size);
            }

            if( p_tsi->TransByteOffset != p_tsi->settings.ts_packet_size )
            {
                BDBG_MSG(("  processEs"));
                tsindex_processEs( p_tsi, p_curBfr, p_tsi->TransByteOffset,
                    p_tsi->settings.ts_packet_size - p_tsi->TransByteOffset - timestamp_size);
            }

        }

next_packet:
        /* Advance to next transport packet */
        p_tsi->TransRecordByteCount += p_tsi->settings.ts_packet_size;

        /* Have we wrapped our 32-bit count? */
        if( p_tsi->TransRecordByteCount < p_tsi->settings.ts_packet_size )
        {
            p_tsi->TransRecordByteCountHi++;
        }
    }

    return i;
}

/******************************************************************************
* INPUTS:   p_tsi = pointer to a previously allocated sTsIndexer structure
*           p_bfr = buffer containing transport data
*           numBytes = number of bytes contained in p_bfr
* OUTPUTS:  None.
* RETURNS:  Number of packets processed.
* FUNCTION: This function parses pes data and stores the start code
*           tables through the previously installed cb function.
******************************************************************************/
long tsindex_feedPes( sTsIndexer * const p_tsi, const unsigned char *p_bfr, long numBytes )
{
    long i = 0;
    long count;
    unsigned long prevRecordByteCount;

    /* We are parsing PES packets */
    p_tsi->PesParseMode = 1;

    while( i < numBytes )
    {
        p_tsi->TransByteOffset = 0;

        if( p_tsi->PesMode )
        {
            p_tsi->curEntry.recordByteCount = p_tsi->TransRecordByteCount - 4;
            p_tsi->curEntry.recordByteCountHi &= REC_BYTE_COUNT_HI_MASK;
            /* Check if we have un-wrapped 32-bit offset */
            if( p_tsi->curEntry.recordByteCount > p_tsi->TransRecordByteCount )
            {
                BDBG_ASSERT(p_tsi->TransRecordByteCountHi>0);
                p_tsi->curEntry.recordByteCountHi |= (p_tsi->TransRecordByteCountHi-1)<<REC_BYTE_COUNT_HI_OFFSET;
            }
            else
            {
                p_tsi->curEntry.recordByteCountHi |= p_tsi->TransRecordByteCountHi<<REC_BYTE_COUNT_HI_OFFSET;
            }
            p_tsi->curEntry.startCodeBytes = p_tsi->TransByteOffset;
            count = tsindex_processPes( p_tsi, p_bfr, i, numBytes-i );
        }
        else
        {
            count = tsindex_processEs( p_tsi, p_bfr, i, numBytes-i );
        }
        i += count;

        prevRecordByteCount = p_tsi->TransRecordByteCount;
        p_tsi->TransRecordByteCount += count;

        /* Check if we have wrapped our 32-bit count */
        if( p_tsi->TransRecordByteCount < prevRecordByteCount )
        {
            p_tsi->TransRecordByteCountHi++;
        }
    }

    return i;
}

/******************************************************************************
* INPUTS:   p_tsi = pointer to a previously allocated sTsIndexer structure
*           p_bfr = buffer containing transport data
*           numBytes = number of bytes contained in p_bfr
* OUTPUTS:  None.
* RETURNS:  Number of packets processed.
* FUNCTION: This function parses pes data and stores the start code
*           tables through the previously installed cb function.
*           We treat VOB files just like transport (except with a bigger
*           sector size and pack header).
*
*           THIS MODE IS CURRENTLY NOT WORKING DUE TO START CODE TABLE FORMAT.
*           IT REQUIRES RE-WORK TO ENSURE THAT THE PACKET OFFSET IS ABLE TO
*           STORE AN OFFSET OF 2048 BYTES.
******************************************************************************/
long tsindex_feedVob( sTsIndexer * const p_tsi, const unsigned char *p_bfr, long numBytes )
{
    long i = 0;
    const unsigned char *p_curBfr;

    /* We are parsing vob packets */
    p_tsi->PesParseMode = 0;

    while( i < numBytes )
    {
        /* Check if we have a partial packet cached */
        if( p_tsi->TransPacketValidCount
            || ((numBytes - i) < VOB_PACKET_SIZE) )
        {
            /* Fill the rest of our transport cache */
            while( i < numBytes )
            {
                p_tsi->TransPacket[p_tsi->TransPacketValidCount] = p_bfr[i];

                p_tsi->TransPacketValidCount += 1;
                i++;

                if( p_tsi->TransPacketValidCount == VOB_PACKET_SIZE )
                    break;
            }

            /* We do not have enough data to cache the entire packet, so return */
            if( p_tsi->TransPacketValidCount != VOB_PACKET_SIZE )
            {
                return i;
            }
            else
            {
                p_curBfr = p_tsi->TransPacket;
                p_tsi->TransPacketValidCount = 0;
            }
        }
        else
        {
            p_curBfr = &(p_bfr[i]);
            i += VOB_PACKET_SIZE;
        }

        /* Check for the pack header */
        if( tsindex_readLong( p_curBfr, 0 ) != 0x000001BA )
        {
            /* This is not a VOB packet!! */
            return -1;
        }

        /* Check for the appropriate PES header ID */
        if( tsindex_readLong( p_curBfr, 14 ) == (0x00000100 | (unsigned long)p_tsi->pesStreamId) )
        {
            p_tsi->PesMode = 1;
            p_tsi->PesHeaderByteCount = 0;
            p_tsi->PesHeaderSize = 0;

            /* Now we need to find where our payload begins */
            p_tsi->TransByteOffset = 14;

            p_tsi->curEntry.recordByteCount = p_tsi->TransRecordByteCount;
            p_tsi->curEntry.recordByteCountHi &= REC_BYTE_COUNT_HI_MASK;
            p_tsi->curEntry.recordByteCountHi |= p_tsi->TransRecordByteCountHi<<REC_BYTE_COUNT_HI_OFFSET;
            p_tsi->curEntry.startCodeBytes = p_tsi->TransByteOffset;
            tsindex_processPes( p_tsi, p_curBfr, p_tsi->TransByteOffset, VOB_PACKET_SIZE - p_tsi->TransByteOffset );

            if( p_tsi->TransByteOffset != VOB_PACKET_SIZE )
            {
                tsindex_processEs( p_tsi, p_curBfr, p_tsi->TransByteOffset, VOB_PACKET_SIZE - p_tsi->TransByteOffset );
            }
        }

        /* Advance to next vob packet */
        p_tsi->TransRecordByteCount += VOB_PACKET_SIZE;
        /* Check if we have wrapped our 32-bit count */
        if( p_tsi->TransRecordByteCount < VOB_PACKET_SIZE )
        {
            p_tsi->TransRecordByteCountHi++;
        }
    }

    return i;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+ INPUTS:   p_tsi = pointer to a previously allocated sTsIndexer structure
+           p_bfr = buffer containing PES header bytes
+           offset  = offset into the buffer in bytes
+           numBytes = number of bytes remaining in the cur trans packet
+ OUTPUTS:  None.
+ RETURNS:  Number of bytes processed
+ FUNCTION: This function processes the PES header, stores the corresponding
+           start code entry, then returns the number of bytes that was
+           processed.  It also clears the PesMode flag in the p_tsi structure
+           that inidicates if the PES header has been entirely processed (ie
+           ES data can now be processed), or if the PES header continues into
+           the next transport packet.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
long tsindex_processPes( sTsIndexer * const p_tsi, const unsigned char *p_bfr,
                        const long offset, long numBytes )
{
    long i = 0;

#ifdef SWAPPED_STREAM
    /* check the offset is a multiple of four.  If it is not then we need more work here */
    if (offset % 4) {
        BDBG_ERR(("PES Header not 4 byte aligned"));
    }
#endif

    for( i = 0; i < numBytes; i++ )
    {
        /* Cache some of the pes header bytes */
        if( p_tsi->PesHeaderByteCount+i < NUM_PES_BYTES_TO_CACHE )
        {
            p_tsi->PesHeader[p_tsi->PesHeaderByteCount + i] = p_bfr[i + offset];

            if( p_tsi->PesHeaderByteCount + i == 4 )
            {
                if( (0xFFFFFFF0 & tsindex_readLong( p_tsi->PesHeader,0 )) !=
                    (0xFFFFFFF0 & (0x00000100 | (unsigned long)p_tsi->pesStreamId) ))
                {
                    /* This is not the type of pes packet we are searching for! */
                    p_tsi->PesMode = 0;
                    p_tsi->PesHeaderByteCount = 0;
                    p_tsi->PesHeaderSize = 0;
                    p_tsi->validPesPacket = 0;
                    break;
                }
                else
                {
                    p_tsi->validPesPacket = 1;
                    p_tsi->actualPesStreamId = 0xFF & tsindex_readLong( p_tsi->PesHeader,0 );
                }
            }
            else if( p_tsi->PesHeaderByteCount+i == 6 )
            {
                /* Store the size of this PES packet */
                p_tsi->curEntry.startCodeBytes |= ((unsigned long)p_tsi->actualPesStreamId<<24) | tsindex_readShort( p_tsi->PesHeader,TS_PES_LENGTH_BYTE) << 8;

                tsindex_p_callback(p_tsi);
            }
            else if( p_tsi->PesHeaderByteCount+i == 9 )
            {
                /* Set the PES header length */
                p_tsi->PesHeaderSize = tsindex_readByte(p_tsi->PesHeader,TS_PES_HEADER_LENGTH_BYTE) + TS_PES_HEADER_LENGTH_BYTE + 1;
            }
            else if( p_tsi->PesHeaderByteCount+i == 14 )
            {
                if(tsindex_readByte(p_tsi->PesHeader,TS_PES_PTS_DTS_FLAGS) & 0x80)
                {
                    uint32_t pts;

                    p_tsi->PesPtsFlag = 1;

                    /* Save PTS in RAVE-style SCT format. pts = PES PTS bits 32..1 >> 1, discard bit 0.
                    The mask shows which bits contain the TS_PES_PTS_xx_xx bits. Then we bit shift up or down to get it into final MSB-32-bit-PTS position. */
                    pts = ((unsigned long)tsindex_readByte(p_tsi->PesHeader,TS_PES_PTS_32_30)&0xE) << 28;
                    pts |= ((unsigned long)tsindex_readShort( p_tsi->PesHeader,TS_PES_PTS_29_15)&0xFFFE) << 13;
                    pts |= ((unsigned long)tsindex_readShort( p_tsi->PesHeader,TS_PES_PTS_14_0)&0xFFFE) >> 2; /* drop LSB 1 bit */
                    p_tsi->curEntry.startCodeBytes = (0xFEul << 24);
                    p_tsi->curEntry.recordByteCount = 0;
                    p_tsi->curEntry.recordByteCountHi = pts;
                    p_tsi->curEntry.flags = 0;
                    tsindex_p_callback(p_tsi);
                }
                else
                {
                    p_tsi->PesPtsFlag = 0;
                }
            }
        }
        /*
         * We can't fit any more header data in our cache, so just just wait
         * until all pes header data is parsed.
         */
        else if( p_tsi->PesHeaderByteCount+i == NUM_PES_BYTES_TO_CACHE )
        {
            p_tsi->PesMode = 0;
        }

        if( p_tsi->PesHeaderSize != 0
            && p_tsi->PesHeaderByteCount+i == p_tsi->PesHeaderSize )
        {
            /* Once we read the entire pes header, then return */
            p_tsi->PesMode = 0;
            break;
        }
    }
    p_tsi->TransByteOffset += i;
    p_tsi->PesHeaderByteCount += i;

    return i;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+ INPUTS:   p_tsi = pointer to a previously allocated sTsIndexer structure
+           p_bfr = buffer containing ES bytes
+           offset  = offset into the buffer in bytes
+           numBytes = number of bytes remaining in the cur trans packet
+ OUTPUTS:  None.
+ RETURNS:  Number of bytes processed
+ FUNCTION: This function processes the ES bytes and stores the corresponding
+           start code entries.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
long tsindex_processEs( sTsIndexer * const p_tsi, const unsigned char *p_bfr,
                       const long offset, long numBytes )
{
    long i = 0;

    for( i = 0; i < numBytes; i++ )
    {
        unsigned char data = tsindex_readByte(p_bfr,(i + offset));

        switch( p_tsi->EsNumStartCodeBytes )
        {
        case 1:
            /* Save the start code */
            p_tsi->curEntry.startCodeBytes = (data << 24) | (uint32_t)p_tsi->savePacketOffset;
            p_tsi->curEntry.recordByteCount = p_tsi->saveStreamOffset;
            p_tsi->curEntry.recordByteCountHi &= REC_BYTE_COUNT_HI_MASK;
            p_tsi->curEntry.recordByteCountHi |= p_tsi->TransRecordByteCountHi<<REC_BYTE_COUNT_HI_OFFSET;

            if( data >= p_tsi->settings.start_code_lo &&
                data <= p_tsi->settings.start_code_hi &&
                p_tsi->settings.is_avc)
            {
                p_tsi->EsNumStartCodeBytes += 1;
                p_tsi->EsNumZeroBytes = 0;
                continue;
            }

            if( data == 0x00 )
            {
                p_tsi->EsStoreNextSlice = 1;
                p_tsi->EsStoreNextNonSlice = 1; /* allow the capture of start codes (e.g. B5) between start codes 00 and 01 */
            }
            else if( data >= p_tsi->settings.start_code_lo &&
                data <= p_tsi->settings.start_code_hi )
            {
                /* Check if we want to store a non-slice start code now */
                if( p_tsi->EsStoreNextNonSlice)
                {
                    if( data == 0xB7 )
                    {
                        /* Sequence end codes don't have any data following them, so store immediately */
                        p_tsi->EsNumStartCodeBytes = 0;
                        p_tsi->EsNumZeroBytes = 0;

#ifdef STORE_SEQUENCE_END_CODES
                        /* Next time only store a slice start code */
                        p_tsi->EsStoreNextSlice = 1;
                        p_tsi->EsStoreNextNonSlice = 0;

                        /* Save this start code entry */
                        if( p_tsi->validPesPacket )
                        {
                            tsindex_p_callback(p_tsi);
                        }
#endif
                        continue;
                    }
                    else
                    {
                        /* Next time only store a slice start code */
                        p_tsi->EsStoreNextSlice = 1;
                        p_tsi->EsStoreNextNonSlice = 0;
                    }
                }
                else
                {
                    /* We don't care about this start code, so move on */
                    p_tsi->EsNumStartCodeBytes = 0;
                    p_tsi->EsNumZeroBytes = 0;
                    continue;
                }
            }
            else if (data >= 0xB9 /* && data <= 0xFF */)
            {
                p_tsi->EsNumStartCodeBytes = 0;

                /* This is not elementary stream data (may be a new PES packet) */
                p_tsi->PesHeaderByteCount = 4;

                /* Initialize the pes header data */
                p_tsi->PesHeader[0] = p_tsi->PesHeader[1] = 0;
                p_tsi->PesHeader[2] = 0x01;
                p_tsi->PesHeader[3] = data;
                p_tsi->PesMode = 1;

                /* Since we found a pes start code init remove the pes zero bytes from the count */
                p_tsi->EsNumZeroBytes -= 2;

                return i+1;
            }
            else if( p_tsi->EsStoreNextSlice)
            {
                /* Next time only store a non-slice start code */
                p_tsi->EsStoreNextNonSlice = 1;
                p_tsi->EsStoreNextSlice = 0;
            }
            else
            {
                /* We don't care about this start code, so move on */
                p_tsi->EsNumStartCodeBytes = 0;
                p_tsi->EsNumZeroBytes = 0;
                continue;
            }
            p_tsi->EsNumStartCodeBytes += 1;

            /* Since we found a start code init our zero byte count */
            p_tsi->EsNumZeroBytes = 0;
            continue;

        case 2:
            /* Save the msb of the 1st byte after start code */
            p_tsi->curEntry.startCodeBytes |= data << 16;
            p_tsi->EsNumStartCodeBytes += 1;
            /* we can't continue here because we might be actually starting the next
            start code */
            break;

        case 3:
            /* Save the lsb of the 1st byte after start code */
            p_tsi->curEntry.startCodeBytes |= data << 8;

            if( p_tsi->EsNumZeroBytes == 2 || p_tsi->settings.entry_size == 6)
            {
                p_tsi->EsNumStartCodeBytes += 1;
            }
            else
            {
                /* This is not the start of a PES header, so we can store the start code */
                p_tsi->EsNumStartCodeBytes = 0;
                p_tsi->EsNumZeroBytes = 0;

                /* Save this start code entry */
                if( p_tsi->validPesPacket )
                {
                    tsindex_p_callback(p_tsi);
                }
            }
            /* we can't continue here because we might be actually starting the next
            start code */
            break;

#ifdef DETECT_EMBEDDED_PES_PACKETS
        case 4:
            /* Check if we have found a PES header immediately following a start code */
            if( p_tsi->EsNumZeroBytes == 2 && data == 0x01 )
            {
                /* Resume where we left off after the PES header */
                p_tsi->EsNumStartCodeBytes = 2;

                /* This is not elementary stream data (may be a new PES packet) */
                p_tsi->PesHeaderByteCount = 3;

                /* Initialize the pes header data */
                p_tsi->PesHeader[0] = p_tsi->PesHeader[1] = 0;
                p_tsi->PesHeader[2] = 0x01;
                p_tsi->PesMode = 1;

                /* Since we found a pes start code init remove the pes zero bytes from the count */
                p_tsi->EsNumZeroBytes = 0;

                return i+1;
            }
            else
            {
                /* This is not the start of a PES header, so we can store the start code */
                p_tsi->EsNumZeroBytes = 0;

                /* Save this start code entry */
                if( p_tsi->validPesPacket )
                {
                    tsindex_p_callback(p_tsi);
                }
            }
            continue;
#else
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            /* capture up to 6 bytes of additional ES data for 6 word format */
            if (p_tsi->settings.entry_size == 6) {
                int i = p_tsi->EsNumStartCodeBytes - 4;

                /* Fill the payload */
                if (i < 3)
                    p_tsi->curEntry.recordByteCountHi |= data << ((2-i)*8);
                else
                    p_tsi->curEntry.flags |= data << ((2-(i-3))*8);

                if (i == 5) {
                    /* We filled the payload */
                    tsindex_p_callback(p_tsi);
                    p_tsi->EsNumStartCodeBytes = 0;
                }
                else {
                    p_tsi->EsNumStartCodeBytes++;
                }
            }
            break;
#endif

        default:
            break;
        }

        if( data == 0x00 )
        {
            p_tsi->EsNumZeroBytes += 1;

            /* The previous zero location is the actual start of the start code sequence.
            Store this is a temp location because we might be finishing up the payload
            of a previous startcode. It's unknown at this point. */
            if( p_tsi->PesParseMode )
            {
                p_tsi->saveStreamOffset = p_tsi->EsRecordByteCount + p_tsi->EsStartCodeBytes;
                p_tsi->savePacketOffset = 0;
            }
            else
            {
                p_tsi->saveStreamOffset = p_tsi->EsRecordByteCount;
                p_tsi->savePacketOffset = p_tsi->EsStartCodeBytes;
            }

            /* Save our location just in case this turns out to be a start code we need to save */
            p_tsi->EsRecordByteCount = p_tsi->TransRecordByteCount;
            p_tsi->EsStartCodeBytes = p_tsi->TransByteOffset + i;
        }
        else if( data == 0x01 && p_tsi->EsNumZeroBytes > 1 )
        {
            /* if currently capturing start code payload, fire callback */
            if (p_tsi->EsNumStartCodeBytes > 0) {
                tsindex_p_callback(p_tsi);
            }

            /* We found a start code! */
            p_tsi->EsNumStartCodeBytes = 1;
        }
        else
        {
            p_tsi->EsNumZeroBytes = 0;
        }
    }
    p_tsi->TransByteOffset += i;

    return i;
}

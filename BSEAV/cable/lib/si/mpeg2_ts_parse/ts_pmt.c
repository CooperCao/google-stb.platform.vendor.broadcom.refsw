/***************************************************************************
 *     Copyright (c) 2003, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bstd.h"
#include "ts_priv.h"
#include "ts_pmt.h"
BDBG_MODULE(ts_pmt);

#define PROGRAM_INFO_LENGTH_OFFSET (TS_PSI_LAST_SECTION_NUMBER_OFFSET+3)
#define PROGRAM_INFO_LENGTH(buf) (TS_READ_16(&buf[PROGRAM_INFO_LENGTH_OFFSET])&0xFFF)
#define DESCRIPTOR_BASE(buf) (&buf[TS_PSI_LAST_SECTION_NUMBER_OFFSET+5])
#define STREAM_BASE(buf) (TS_PSI_LAST_SECTION_NUMBER_OFFSET + 5 + PROGRAM_INFO_LENGTH(buf))

bool TS_PMT_validate(const uint8_t *buf, unsigned bfrSize)
{
	int sectionEnd = TS_PSI_GET_SECTION_LENGTH(buf) + TS_PSI_SECTION_LENGTH_OFFSET;
	int programInfoEnd = PROGRAM_INFO_LENGTH(buf) + PROGRAM_INFO_LENGTH_OFFSET;

	return (buf[0] == 0x2) &&
		sectionEnd < (int)bfrSize &&
		programInfoEnd < (int)bfrSize &&
		programInfoEnd < sectionEnd;
}

static int TS_PMT_P_getStreamByteOffset( const uint8_t *buf, unsigned bfrSize, int streamNum )
{
	int byteOffset;
	int i;

	/* After the last descriptor */
	byteOffset = STREAM_BASE(buf);

	for (i=0; i < streamNum; i++)
	{
		if (byteOffset >= (int)bfrSize || byteOffset >= TS_PSI_MAX_BYTE_OFFSET(buf))
			return -1;
		byteOffset += 5 + (TS_READ_16( &buf[byteOffset+3] ) & 0xFFF);
	}

	return byteOffset;
}


uint16_t TS_PMT_getPcrPid( const uint8_t *buf, unsigned bfrSize)
{
	BSTD_UNUSED(bfrSize);
	return (uint16_t)(TS_READ_16( &buf[TS_PSI_LAST_SECTION_NUMBER_OFFSET+1] ) & 0x1FFF);
}

TS_PSI_descriptor TS_PMT_getDescriptor( const uint8_t *buf, unsigned bfrSize, int descriptorNum )
{
	const uint8_t *descriptorBase = DESCRIPTOR_BASE(buf);
	uint32_t descriptorsLength = PROGRAM_INFO_LENGTH(buf);

	/* Any time we dereference memory based on the contents of live data,
	we should check. */
	if (descriptorBase - buf >= (int)bfrSize) {
		return NULL;
	}
	bfrSize -= (descriptorBase - buf);
	if (bfrSize < descriptorsLength) {
		BDBG_WRN(("Invalid descriptor length: %d>=%d", descriptorsLength, bfrSize));
		return NULL;
	}
	return TS_P_getDescriptor(descriptorBase, descriptorsLength, descriptorNum);
}

int TS_PMT_getNumStreams( const uint8_t *buf, unsigned bfrSize)
{
	int byteOffset;
	int i = 0;

	byteOffset = STREAM_BASE(buf);

	while (byteOffset < TS_PSI_MAX_BYTE_OFFSET(buf) && byteOffset < (int)bfrSize)
	{
		byteOffset += 5 + (TS_READ_16( &buf[byteOffset+3] ) & 0xFFF);
		i++;
	}

	return i;
}

BERR_Code TS_PMT_getStream( const uint8_t *buf, unsigned bfrSize, int streamNum, TS_PMT_stream *p_stream )
{
	int byteOffset;

	byteOffset = TS_PMT_P_getStreamByteOffset( buf, bfrSize, streamNum );
	if (byteOffset == -1)
		return BERR_INVALID_PARAMETER;

	p_stream->stream_type = buf[byteOffset];
	p_stream->elementary_PID = (uint16_t)(TS_READ_16( &buf[byteOffset+1] ) & 0x1FFF);

	return BERR_SUCCESS;
}

TS_PSI_descriptor TS_PMT_getStreamDescriptor( const uint8_t *buf, unsigned bfrSize, int streamNum, int descriptorNum )
{
	int byteOffset;

	byteOffset = TS_PMT_P_getStreamByteOffset( buf, bfrSize, streamNum );
	if (byteOffset == -1)
		return NULL;

	return (TS_P_getDescriptor(&buf[byteOffset+5], TS_READ_16(&buf[byteOffset+3])&0xFFF, descriptorNum ));
}

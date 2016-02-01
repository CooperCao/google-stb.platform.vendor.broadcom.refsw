/***************************************************************************
 *	   Copyright (c) 2007-2009, Broadcom Corporation
 *	   All Rights Reserved
 *	   Confidential Property of Broadcom Corporation
 *
 *	THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *	AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *	EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * File to build/parse PVR marker
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"

#include "botf_marker.h"

BDBG_MODULE(botf_marker);

size_t
bpvr_marker_build_es(bpvr_marker *marker, void *pkt)
{
	uint8_t *data = pkt;

	BDBG_MSG(("set chunk_cnt %d, off_hi %#x, off_lo %#x", marker->chunk_cnt, (unsigned)marker->off_hi, (unsigned)marker->off_lo));
#if 0
	data[0] = 0xFF; /* reset previous start code sequence */
#else
	data[0] = 0x00; 
#endif

	data[1] = 0x00; 
	data[2] = 0x00; /* starrt new start code */
	data[3] = 0x00;
	data[4] = 0x01;
	data[5] = B_MARKER_SC;
	data[6] = (marker->chunk_cnt >> 24) & 0xFF;
	data[7] = (marker->chunk_cnt >> 16) & 0xFF;
	data[8] = 0xFF;
	data[9] = (marker->chunk_cnt >> 8) & 0xFF;
	data[10] = marker->chunk_cnt & 0xFF;
	data[11] = 0xFF;
	data[12] = (marker->off_hi>> 8) & 0xFF;
	data[13] = marker->off_hi & 0xFF;
	data[14] = 0xFF;
	data[15] = (marker->off_lo>> 24) & 0xFF;
	data[16] = (marker->off_lo>> 16) & 0xFF;
	data[17] = 0xFF;
	data[18] = (marker->off_lo>> 8) & 0xFF;
	data[19] = marker->off_lo& 0xFF;

	return 20;
}

/* Build TS Marker packets */ 
BERR_Code BOTF_BuildMarkerTS( const BOTF_MarkerParams *MarkerParams, void *MarkerBuf, size_t buf_len, size_t *pkt_len)
{
	uint8_t *data = MarkerBuf;
	bpvr_marker pvr_marker;    

	BDBG_ASSERT(MarkerParams);
	BDBG_ASSERT(MarkerBuf);
	BDBG_ASSERT(pkt_len);

	*pkt_len = (188+MarkerParams->TimestampOffset)*2;
	if(buf_len<*pkt_len) {
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}
	/* we fill data with FF in order to prevent false start codes */
	BKNI_Memset(data, 0xff, *pkt_len);
	
	data += MarkerParams->TimestampOffset;

	/* build marker packet */
	data[0] = 0x47;
	data[1] = (MarkerParams->Pid >> 8) & 0x1F;
	data[2] = (MarkerParams->Pid & 0xFF);
	data[3] = 0x10;

	/* Leave first 4 bytes as 0xff to avoid false start codes */
	data = (uint8_t *)MarkerBuf+8+MarkerParams->TimestampOffset;

	pvr_marker.chunk_cnt = MarkerParams->ChunkCntr;
	pvr_marker.off_hi = MarkerParams->OffsetHi;
	pvr_marker.off_lo = MarkerParams->OffsetLow;
	bpvr_marker_build_es(&pvr_marker, data);	
	
	data = (uint8_t *)MarkerBuf+188+MarkerParams->TimestampOffset;

	data[0] = 0x47;
	data[1] = (MarkerParams->Pid >> 8) & 0x1F;
	data[2] = (MarkerParams->Pid & 0xFF);
	data[3] = 0x10;

	/* Insert Filler (userdata) sc after marker */
	data[4] = 0x00;
	data[5] = 0x00;
	data[6] = 0x01;
	data[7] = 0xB2;
	data[8] = 0x00;
	data[9] = 0x00;
	data[10] = 0x00;
	data[11] = 0x00;

	return BERR_SUCCESS;
}

/* Build DSS Marker packets */
BERR_Code BOTF_BuildMarkerDSS( const BOTF_MarkerParams *MarkerParams, void *MarkerBuf, size_t buf_len, size_t *pkt_len)
{
	uint8_t *data = MarkerBuf;
	bpvr_marker pvr_marker;

    BDBG_ASSERT(MarkerParams);
    BDBG_ASSERT(MarkerBuf);
    BDBG_ASSERT(pkt_len);

    if(buf_len<188*2) {
		return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

#if 0
	/* Insert filler packet between marker and chunk end to avoid
	 * false start code generation */
	data[0] = 0xA0 | ((MarkerParams->Pid >> 8) & 0x0F);
	data[1] = (MarkerParams->Pid & 0xFF);
	data[2] = 0x24;

        BKNI_Memset(data+3, 0xFF, 127);

	/* build marker packet */
	data = MarkerBuf+130;
#endif
    *pkt_len = 130;
	/* we fill data with FF in order to prevent false start codes */
	BKNI_Memset(data, 0xFF, 130);

	data[0] = 0xA0 | ((MarkerParams->Pid >> 8) & 0x0F);
	data[1] = (MarkerParams->Pid & 0xFF);
	data[2] = 0x24;
	
	pvr_marker.chunk_cnt = MarkerParams->ChunkCntr;
	pvr_marker.off_hi = MarkerParams->OffsetHi;
	pvr_marker.off_lo = MarkerParams->OffsetLow;
	bpvr_marker_build_es(&pvr_marker, data+3);

	return BERR_SUCCESS;
}


BERR_Code BOTF_BuildMarkerPES( const BOTF_MarkerParams *MarkerParams, void *MarkerBuf, size_t buf_len, size_t *pkt_len)
{
	uint8_t *data = MarkerBuf;
	bpvr_marker pvr_marker;    

	BDBG_ASSERT(MarkerParams);
	BDBG_ASSERT(MarkerBuf);
	BDBG_ASSERT(pkt_len);

	*pkt_len = 256*2;
	if(buf_len<*pkt_len) {
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}
	/* we fill data with FF in order to prevent false start codes */
	BKNI_Memset(data, 0xff, *pkt_len);

	/* build marker PES packet */
	data[0] = 0x00;
	data[1] = 0x00;
	data[2] = 0x01;
	data[3] = (uint8_t)MarkerParams->Pid;

	/* Leave first 4 bytes as 0xff to avoid false start codes */
	data = (uint8_t *)MarkerBuf+8;

	pvr_marker.chunk_cnt = MarkerParams->ChunkCntr;
	pvr_marker.off_hi = MarkerParams->OffsetHi;
	pvr_marker.off_lo = MarkerParams->OffsetLow;
	bpvr_marker_build_es(&pvr_marker, data);	
	
	data = (uint8_t *)MarkerBuf+256;

	data[0] = 0x00;
	data[1] = 0x00;
	data[2] = 0x01;
	data[3] = (uint8_t)MarkerParams->Pid;

	/* Insert Filler (userdata) sc after marker */
	data[4] = 0x00;
	data[5] = 0x00;
	data[6] = 0x01;
	data[7] = 0xB2;
	data[8] = 0x00;
	data[9] = 0x00;
	data[10] = 0x00;
	data[11] = 0x00;

	return BERR_SUCCESS;
}

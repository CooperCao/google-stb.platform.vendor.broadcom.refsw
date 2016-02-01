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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************/

#include "tshdrbuilder.h"
#define TS_WRITE_32(buf,val) ((uint8_t*)buf)[0]=((uint32_t)(val)>>24)&0xFF; \
                             ((uint8_t*)buf)[1]=((uint32_t)(val)>>16)&0xFF; \
                             ((uint8_t*)buf)[2]=((uint32_t)(val)>> 8)&0xFF; \
                             ((uint8_t*)buf)[3]=((uint32_t)(val)    )&0xFF;

#define TS_WRITE_16(buf,val) ((uint8_t*)buf)[0]=((uint16_t)(val)>> 8)&0xFF; \
                             ((uint8_t*)buf)[1]=((uint16_t)(val)    )&0xFF;


#define TS_HEADER_LENGTH_OFFSET             0
#define TS_HEADER_SYNC_BYTE_OFFSET          1
#define TS_HEADER_TEI_OFFSET                2
#define TS_HEADER_SCRAMBLING_OFFSET         4
#define TS_HEADER_AP_LENGTH_OFFSET          5
#define TS_HEADER_AP_DI_OFFSET              6
#define TS_PMT_PCR_PID_OFFSET               8
#define TS_PMT_PROGRAM_INFO_LENGTH_OFFSET   10

BDBG_MODULE(tshdrbuilder);

/* == private functions from ts_pmt.c ==*/
#define PROGRAM_INFO_LENGTH_OFFSET (TS_PSI_LAST_SECTION_NUMBER_OFFSET+3)
#define PROGRAM_INFO_LENGTH(buf) (TS_READ_16(&buf[PROGRAM_INFO_LENGTH_OFFSET])&0xFFF)
#define DESCRIPTOR_BASE(buf) (&buf[TS_PSI_LAST_SECTION_NUMBER_OFFSET+5])
#define STREAM_BASE(buf) (TS_PSI_LAST_SECTION_NUMBER_OFFSET + 5 + PROGRAM_INFO_LENGTH(buf))
static int TS_PMT_P_getStreamByteOffset(
    const uint8_t *buf,
    unsigned bfrSize,
    int streamNum
) {
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
/* ======================================*/

/* ============= utilities ==============*/
/* Generate a CRC for the specified data/length */
/* Initialize crc to 0 for new calculation.  Use an old result for subsequent calls. */
static uint32_t CRC32_mpeg(
    uint32_t crc,
    uint8_t *data,
    int length
) {
	int j;
	crc = ~crc;
	while (length--)
	{
		for (j=0; j<8; j++) 
			crc = (crc<<1) ^ ((((*data >> (7-j)) ^ (crc >> 31)) & 1) ? 0x04c11db7 : 0);
		data++;
	}
	return crc;
}
/* ======================================*/

void TS_PID_info_Init(
    TS_PID_info * info,
    uint16_t PID,
    bool PUSI
) {
    if(PID > 0x1FFF)
    {
        BDBG_WRN(("Pid number truncated to 13 bits"));
    }
    info->PID = PID & 0x1FFF;
    info->PUSI = PUSI;
    info->pcr_valid = 0;
    info->pointer_field = 0;
    info->pcr = 0;
    info->pcr_ext = 0;
}

void TS_PAT_program_Init(
    TS_PAT_program *program,
    uint16_t program_number,
    uint16_t PID
) {
    if(PID > 0x1FFF)
    {
        BDBG_WRN(("PID number truncated to 13 bits"));
    }
    program->program_number = program_number;
    program->PID = PID & 0x1FFF;
}

void TS_PMT_stream_Init(
    TS_PMT_stream *stream,
    uint8_t stream_type,
    uint16_t elementary_PID
) {
    stream->stream_type = stream_type;
    stream->elementary_PID = elementary_PID;
}

void TS_PSI_descriptor_Init(
    TS_PSI_descriptor *desc,
    void *descBuf
) {
    *desc = (uint8_t *) descBuf; 
}

void TS_PID_state_Init(
    TS_PID_state * state
) {
    state->continuity_counter = 0; 
}

void TS_PSI_header_Init(
    TS_PSI_header * header
) {
    header->table_id = 0x0;
    header->section_syntax_indicator = 0x1;
    header->private_indicator = 0x0;
    header->section_length = 9;
    header->table_id_extension = 0x0;
    header->version_number = 0x00;
    header->current_next_indicator = 0x1;
    header->section_number = 0x0;
    header->last_section_number = 0x0;
    header->CRC_32 = 0x0;
}

BERR_Code TS_buildTSHeader(
    const TS_PID_info *pidInfo,
    TS_PID_state *pidState, 
    void *tsHeaderBuf,
    size_t tsHeaderBufSz, 
    size_t *tsHeaderBufBuilt,
    size_t payloadSz,
    size_t *payloadBuiltSz, 
    unsigned packetMax
) {
    unsigned tsHdrBufSzLimit, plSzLimit;
    unsigned plRemainder, fillingSz;
    uint8_t *bufHdr, bufStcl[5], cpyBuf[5];
    unsigned plCount, plTotal;
    bool extra_adp;

    if(pidInfo->pcr_valid && packetMax != 1){
        BDBG_ERR(("Invalid input parameter. packetMax must be set to 1 in order to attach PCR info\n"));
        goto TS_HDR_BUILDER_ERROR;
    }
    if(pidInfo->pointer_field && packetMax != 1) {
        BDBG_ERR(("Invalid input parameter. packetMax must be set to 1 in order to attach pointer_field\n"));
        goto TS_HDR_BUILDER_ERROR;
    }
    if(pidInfo->pcr_valid && pidInfo->pointer_field){
        BDBG_ERR(("Invalid input parameter. Cannot attach PCR on a packet that has a pointer_field\n"));
        goto TS_HDR_BUILDER_ERROR;
    }

    if(pidInfo->pointer_field){
        payloadSz+=1;
    }

    /* Determine number of packets to build */
    plSzLimit = payloadSz / 184;
    plRemainder = payloadSz - plSzLimit * 184;
    extra_adp = 0;
    if (pidInfo->pcr_valid){ 
        plSzLimit += 1;
        fillingSz = (4+2+6+payloadSz < 188) ? 188-(4+2+6+payloadSz) : 0;
        tsHdrBufSzLimit = 2;
    } else if (plRemainder == 0) {
        fillingSz = 0;
        tsHdrBufSzLimit = tsHeaderBufSz/5;
    } else if (plRemainder == 183) {                                /* odd case */
        plSzLimit += 2;
        fillingSz = 184-2-1;
        tsHdrBufSzLimit = (tsHeaderBufSz - (1 + 4 + 2) - (fillingSz + 1 + 4 + 2) )/5 + 2;
        extra_adp = 1;
    } else {
        plSzLimit += 1;
        fillingSz = 188 - 4 - 2 - plRemainder;
        tsHdrBufSzLimit = (tsHeaderBufSz - (fillingSz + 1 + 4 + 2))/5 + 1;
    }
    plTotal = packetMax < plSzLimit ? packetMax : plSzLimit;
    plTotal = plTotal < tsHdrBufSzLimit ? plTotal : tsHdrBufSzLimit;

    /* Make stencil */
    bufStcl[TS_HEADER_LENGTH_OFFSET] = 4;
    bufStcl[TS_HEADER_SYNC_BYTE_OFFSET] = 0x47;
    TS_WRITE_16(&bufStcl[TS_HEADER_TEI_OFFSET], pidInfo->PID&0x1FFFF);      /* TEI = 0, Payload Unit Start = 0, Transport Priority = 0, 13 bit-pid# */
    bufStcl[TS_HEADER_SCRAMBLING_OFFSET] = 0x10;                            /* scrambling = 0, adaptation field = payload only, continuity counter */

    /* Build main array */
    bufHdr = (uint8_t *)tsHeaderBuf;
    for (plCount = 0; plCount < plTotal; plCount++) {
        BKNI_Memcpy(&bufHdr[plCount*5], bufStcl, 5);
        bufHdr[plCount*5+TS_HEADER_SCRAMBLING_OFFSET] |= pidState->continuity_counter++&0xF;
    }
    *tsHeaderBufBuilt = (size_t) (plTotal)*5;
    *payloadBuiltSz = (size_t) (plTotal)*184;

    /* First packet customization */
    if (pidInfo->PUSI){
        bufHdr[TS_HEADER_TEI_OFFSET] |= 0x40;
    }
    if (pidInfo->pcr_valid){
        bufHdr[TS_HEADER_LENGTH_OFFSET] = 4 + 2 + 6 + fillingSz;
        bufHdr[TS_HEADER_SCRAMBLING_OFFSET] |= 0x30;
        bufHdr[TS_HEADER_AP_LENGTH_OFFSET] = 1+6+fillingSz;                         /* filler size + 1 byte of adaptation field indicators/flags */
        bufHdr[TS_HEADER_AP_DI_OFFSET] = (pidInfo->PUSI? 0x40 : 0x0) | 0x10;        /* RAI? + PCR */
        TS_WRITE_32(&bufHdr[TS_HEADER_AP_DI_OFFSET+1],pidInfo->pcr>>1);
        TS_WRITE_16(&bufHdr[TS_HEADER_AP_DI_OFFSET+5], ((pidInfo->pcr&0x1)<<15) + (0x3F<<9) + (pidInfo->pcr_ext&0x1FF));
        BKNI_Memset(&bufHdr[TS_HEADER_AP_DI_OFFSET+1+6], 0xFF, fillingSz);
        *tsHeaderBufBuilt += 2 + 6 + fillingSz;
        *payloadBuiltSz = 188-4-(2+6+fillingSz);
    }

    /* 2nd last packet customization, for only the odd case where payloadSz%184 = 183 */
    if (extra_adp && plSzLimit-1 <= plTotal ) {
        plCount = plSzLimit-2;
        if(plSzLimit == plTotal) {                                                  /* moving last packet */
            BKNI_Memcpy(cpyBuf, &bufHdr[(plCount+1)*5], 5);
            BKNI_Memcpy(&bufHdr[(plCount+1)*5+2], cpyBuf, 5);
        }
        bufHdr[plCount*5+TS_HEADER_SCRAMBLING_OFFSET] |= 0x30;
        bufHdr[plCount*5+TS_HEADER_LENGTH_OFFSET] = 6;
        bufHdr[plCount*5+TS_HEADER_AP_LENGTH_OFFSET] = 1;
        bufHdr[plCount*5+TS_HEADER_AP_DI_OFFSET] = bufHdr[plCount*5+TS_HEADER_TEI_OFFSET] & 0x40;
        *tsHeaderBufBuilt += 2;
        *payloadBuiltSz -= 2;
        bufHdr = &bufHdr[(plTotal-1)*5+2];
    } else {
        bufHdr = &bufHdr[(plTotal-1)*5];
    }

    /* Last packet customization */
    if (plRemainder && plSzLimit == plTotal){
        bufHdr[TS_HEADER_LENGTH_OFFSET] = 4 + 2 + fillingSz;
        bufHdr[TS_HEADER_SCRAMBLING_OFFSET] |= 0x30;
        bufHdr[TS_HEADER_AP_LENGTH_OFFSET] = 1+fillingSz;                   /* filler size + 1 byte of adaptation field options/flags */
        bufHdr[TS_HEADER_AP_DI_OFFSET] = pidInfo->PUSI? 0x40 : 0x0;
        BKNI_Memset(&bufHdr[TS_HEADER_AP_DI_OFFSET+1], 0xFF, fillingSz);
        *tsHeaderBufBuilt += 2 + fillingSz;
        *payloadBuiltSz += (int)-184+188-4-2-fillingSz;
    }
    
    /* Pointer field customization */
    if(pidInfo->pointer_field){
        bufHdr = (uint8_t *)tsHeaderBuf;
        bufHdr[TS_HEADER_LENGTH_OFFSET] += 1;
        bufHdr[bufHdr[TS_HEADER_LENGTH_OFFSET]]=0x0;
        *tsHeaderBufBuilt += 1;
        *payloadBuiltSz -= 1;
    }

    return BERR_SUCCESS;

TS_HDR_BUILDER_ERROR:
    *tsHeaderBufBuilt = 0;
    *payloadBuiltSz = 0;
    return BERR_INVALID_PARAMETER;
}

BERR_Code TS_PAT_Init(
    TS_PAT_state *pat,
    const TS_PSI_header *p_header,
    void *patBuf,
    size_t patBufSz
) {
    if( patBufSz < 13 ||                        /* Buffer too small */
        p_header->section_length > 255 )        /* Section length > 255 is unsupported */    
    {
        BDBG_ERR(("TS_PAT_Init check failed"));
        return BERR_INVALID_PARAMETER;
    }
    
    pat->buf = patBuf;
    pat->bufSize = patBufSz;

    /* PAT */
    BKNI_Memset(pat->buf, 0xFF, 12);
    pat->buf[TS_PSI_TABLE_ID_OFFSET] = p_header->table_id;
    TS_WRITE_16(&(pat->buf[TS_PSI_SECTION_LENGTH_OFFSET]), p_header->section_length&0x3FF);
    pat->buf[TS_PSI_SECTION_LENGTH_OFFSET] |= 0x30 + ((p_header->section_syntax_indicator&1)<<7) + ((p_header->private_indicator&1)<<6);
    TS_WRITE_16(&(pat->buf)[TS_PSI_TABLE_ID_EXT_OFFSET], p_header->table_id_extension);
    pat->buf[TS_PSI_CNI_OFFSET] = 0xC0 + ((p_header->version_number&0x1F)<<1) + (p_header->current_next_indicator&1);
    pat->buf[TS_PSI_SECTION_NUMBER_OFFSET] = p_header->section_number;
    pat->buf[TS_PSI_LAST_SECTION_NUMBER_OFFSET] = p_header->last_section_number;
    TS_WRITE_32(&(pat->buf)[p_header->section_length+TS_PSI_SECTION_LENGTH_OFFSET-2], p_header->CRC_32);

    return BERR_SUCCESS;
}


BERR_Code TS_PAT_addProgram(
    TS_PAT_state *pat,
    TS_PMT_state *pmt,
    const TS_PAT_program *p_program,
    void *pmtBuf,
    size_t pmtBufSz
) {
    unsigned numPrograms;
    unsigned sectionLen;

    numPrograms = TS_PAT_getNumPrograms(pat->buf)+1;
    sectionLen =  9+4*numPrograms;

    if( pat->bufSize < 13+4*numPrograms ||              /* PAT buf too small */
        sectionLen > 188-4 ||                           /* PAT section too long */
        pmtBufSz < 17 )                                 /* PMT buf too small */
    {
        BDBG_ERR(("TS_PAT_addProgram check failed"));
        return BERR_INVALID_PARAMETER;
    }
    
    pmt->buf = pmtBuf;
    pmt->bufSize = pmtBufSz;

    /* Append to PAT */
    pat->buf[TS_PSI_SECTION_LENGTH_OFFSET+1] = sectionLen;
    TS_WRITE_16(&(pat->buf[TS_PSI_LAST_SECTION_NUMBER_OFFSET+1+4*(numPrograms-1)]), p_program->program_number);
    TS_WRITE_16(&(pat->buf[TS_PSI_LAST_SECTION_NUMBER_OFFSET+1+4*(numPrograms-1)+2]), p_program->PID|0xE000);

    /* Making a PMT */
    BKNI_Memset(pmt->buf, 0xFF, 16);
    
    BKNI_Memcpy(pmt->buf, pat->buf, TS_PSI_LAST_SECTION_NUMBER_OFFSET+1);               /* copy settings from PAT */
    pmt->buf[TS_PSI_TABLE_ID_OFFSET] = 0x02;                                            /* table PID */
    pmt->buf[TS_PSI_SECTION_LENGTH_OFFSET+1] = 13;                                      /* section length (no ES stream) */
    TS_WRITE_16(&(pmt->buf[TS_PSI_TABLE_ID_EXT_OFFSET]), p_program->program_number);    /* program number */
    pmt->buf[TS_PSI_CNI_OFFSET] &= 0xC1;                                                /* version number = 0 */
    pmt->buf[TS_PSI_CNI_OFFSET] |= 0x01;                                                /* current_next_indicator = 1 */
    pmt->buf[TS_PSI_SECTION_NUMBER_OFFSET] = 0;                                         /* section number = 0 */
    pmt->buf[TS_PSI_LAST_SECTION_NUMBER_OFFSET] = 0;                                    /* last section number = 0 */
                                                                                        /* PCR PID default to 0x1FFF */
    TS_WRITE_16(&(pmt->buf[TS_PMT_PROGRAM_INFO_LENGTH_OFFSET]), 0xF000);                /* 4-bit reserve + Program Info length = 0 */

    return BERR_SUCCESS;
}

/* Extension to ts_pmt class */
BERR_Code TS_PMT_setPcrPid(
    TS_PMT_state *pmt,
    uint16_t pcrPid
) {
    TS_WRITE_16(&pmt->buf[TS_PMT_PCR_PID_OFFSET], pcrPid|0xE000);
    return BERR_SUCCESS;
}

BERR_Code TS_PMT_addStream(
    TS_PMT_state *pmt,
    const TS_PMT_stream *p_stream,
    unsigned *streamNum
) {
    unsigned numStreams;
    uint8_t *newStream;

    if ( TS_PSI_GET_SECTION_LENGTH(pmt->buf)+5 > 188 )               /* section too long */
    {
        BDBG_ERR(("TS_PMT_addStream check failed"));
        return BERR_INVALID_PARAMETER;
    } 
    numStreams = TS_PMT_getNumStreams(pmt->buf, pmt->bufSize);
    newStream = &pmt->buf[TS_PSI_SECTION_LENGTH_OFFSET + TS_PSI_GET_SECTION_LENGTH(pmt->buf) - 2];
    newStream[0] = p_stream->stream_type;                           /* stream type */
    TS_WRITE_16(&newStream[1],p_stream->elementary_PID|0xE000);     /* PID Number */
    TS_WRITE_16(&newStream[3],0xF000);                              /* 4-bit reserve + 0 length descriptor */
    pmt->buf[TS_PSI_SECTION_LENGTH_OFFSET+1]+=5;
    *streamNum = numStreams;
    return BERR_SUCCESS;
}

BERR_Code TS_PMT_setDescriptor(
    TS_PMT_state *pmt,
    const TS_PSI_descriptor descriptor,
    unsigned descLen,
    unsigned streamNum
) {
    uint8_t tmpBuf[188];
    uint8_t *stream;
    unsigned numStreams;
    unsigned streamEndOffset;
    unsigned streamNextOffset;
    unsigned oldDescLen;
    
    numStreams = TS_PMT_getNumStreams(pmt->buf, pmt->bufSize);
    if( streamNum >= numStreams ||                                   /* invalid stream number */
        TS_PSI_GET_SECTION_LENGTH(pmt->buf)+descLen > 188 )         /* descriptor too long */
    {
        BDBG_ERR(("TS_PMT_setDescriptor check failed"));
        return BERR_INVALID_PARAMETER;
    }

    stream = &pmt->buf[TS_PMT_P_getStreamByteOffset(pmt->buf,pmt->bufSize,streamNum)];
    streamEndOffset = TS_PSI_SECTION_LENGTH_OFFSET + TS_PSI_GET_SECTION_LENGTH(pmt->buf) - 2;
    oldDescLen = TS_READ_16(&stream[3])&0x3FF;
    if( streamNum != numStreams-1 ) {
        /* Move data */
        streamNextOffset = (unsigned)TS_PMT_P_getStreamByteOffset(pmt->buf,pmt->bufSize,streamNum+1);
        BKNI_Memcpy(tmpBuf,&pmt->buf[streamNextOffset], streamEndOffset-streamNextOffset);
        BKNI_Memcpy(&pmt->buf[streamNextOffset+descLen-oldDescLen], tmpBuf, streamEndOffset-streamNextOffset);
    }
    BKNI_Memcpy(&stream[5], descriptor, descLen);
    TS_WRITE_16(&stream[3],descLen);

    /*update section length*/
    pmt->buf[TS_PSI_SECTION_LENGTH_OFFSET+1]+=descLen-oldDescLen;
    return BERR_SUCCESS;
}

BERR_Code TS_PAT_finalize(
    TS_PAT_state *pat,
    size_t * pat_size
) {
    uint32_t crc;
    unsigned len = TS_PSI_SECTION_LENGTH_OFFSET + TS_PSI_GET_SECTION_LENGTH(pat->buf) + 2;

    if(pat->buf[TS_PSI_TABLE_ID_OFFSET] == 0x0 &&
        len != TS_PSI_SECTION_LENGTH_OFFSET + 2) {
        crc = CRC32_mpeg(0, pat->buf, len-4);
        TS_WRITE_32(&pat->buf[len-4],crc);
        pat->size = len;
        *pat_size = len;
    } else {
        *pat_size = 0;
        BDBG_ERR(("TS_PAT_finalize operation failed"));
        return BERR_INVALID_PARAMETER;
    }

    return BERR_SUCCESS;
}

BERR_Code TS_PMT_finalize(
    TS_PMT_state *pmt,
    size_t * pmt_size
) {
    uint32_t crc;
    unsigned len = TS_PSI_SECTION_LENGTH_OFFSET + TS_PSI_GET_SECTION_LENGTH(pmt->buf) + 2;

    if(pmt->buf[TS_PSI_TABLE_ID_OFFSET] == 0x2 &&
       len != TS_PSI_SECTION_LENGTH_OFFSET + 2) {
        crc = CRC32_mpeg(0, pmt->buf, len-4);
        TS_WRITE_32(&pmt->buf[len-4],crc);
        pmt->size = len;
        *pmt_size = len;
    } else {
        *pmt_size = 0;
        BDBG_ERR(("TS_PMT_finalize operation failed"));
        return BERR_INVALID_PARAMETER;
    }

    return BERR_SUCCESS;
}


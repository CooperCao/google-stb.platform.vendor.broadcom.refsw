/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
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
 * RTP AMR parser library
 *   RFC 6416 module
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "brtp_parser_amr.h"
#include "brtp_packet.h"
#include "biobits.h"
#include "brtp_util.h"

BDBG_MODULE(brtp_parser_amr);

BDBG_OBJECT_ID(brtp_parser_amr_t);

struct brtp_parser_amr {
	struct brtp_parser parent; /* must be the first element */
	BDBG_OBJECT(brtp_parser_amr_t)
	brtp_parser_amr_stream_cfg stream_cfg;
	unsigned pkt_cnt;
	unsigned pkt_len;
	brtp_parser_amr_cfg cfg;

	const uint8_t *meta_base;
	size_t meta_len;
	
};

typedef struct brtp_parser_amr *brtp_parser_amr_t;

static const brtp_parser_type b_rtp_parser_amr= {
	"AMR",
	brtp_stream_audio
};

const brtp_parser_type *brtp_parser_amr = &b_rtp_parser_amr;



void
brtp_parser_amr_default_cfg(brtp_parser_amr_cfg *cfg)
{
	*cfg = 0;
	return;
}

static const brtp_parser_amr_stream_cfg b_rtp_parser_amr_aac_loas_cfg = {
	brtp_parser_amr_aac_loas, /* mode */
	13, /* sizelength */
	3, /* indexlength */
	2, /* headerlength */
	{0}, /* config */
	0, /* config_len */
	15 /* profile */
};

void 
brtp_parser_amr_default_stream_cfg(brtp_parser_amr_mode mode, brtp_parser_amr_stream_cfg *cfg)
{
	BDBG_ASSERT(cfg);
	switch(mode) {
	default:
		BDBG_WRN(("brtp_parser_amr_default_stream_cfg: not supported mode %u", mode));
		cfg->mode = mode;
		break;
	case brtp_parser_amr_aac_loas:
		*cfg = b_rtp_parser_amr_aac_loas_cfg;
		break;

	}
	return;
}

static void
b_rtp_parser_amr_begin(brtp_parser_amr_t parser)
{
	brtp_parser_begin(&parser->parent);
	parser->pkt_cnt = 0;
	parser->pkt_len = 0;
	return;
}

static void
b_rtp_parser_amr_commit(brtp_parser_amr_t parser)
{
	brtp_parser_commit(&parser->parent);
	return;
}

#define WRITE_LE_16(bs, x) do { (bs)[0] = (x)&0xff; (bs)[1] = ((x)>>8) & 0xff; } while (0);
#define WRITE_LE_32(bs, x) do { (bs)[0] = (x)&0xff; (bs)[1] = ((x)>>8) & 0xff; (bs)[2] = ((x)>>16) & 0xff; (bs)[3] = ((x)>>24) & 0xff; } while (0);
#define WRITE_BE_16(bs, x) do { (bs)[1] = (x)&0xff; (bs)[0] = ((x)>>8) & 0xff; } while (0);
#define WRITE_BE_32(bs, x) do { (bs)[3] = (x)&0xff; (bs)[2] = ((x)>>8) & 0xff; (bs)[1] = ((x)>>16) & 0xff; (bs)[0] = ((x)>>24) & 0xff; } while (0);

typedef struct WAVEFORMATEX
{
    uint16_t wFormatTag;
    uint16_t nChannels;
    uint32_t nSamplesPerSec;
    uint32_t nAvgBytesPerSec;
    uint16_t nBlockAlign;
    uint16_t wBitsPerSample;
    uint16_t cbSize;
}WAVEFORMATEX_t;

#define WAVEFORMATEX_SIZE (18)
#define MAX_TOC_ENTRIES 64

static const unsigned g_frameTypeToBytes[] = {12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0};

#define BCMA_HEADER

static void 
b_rtp_parser_amr_packet(brtp_parser_t parser_, brtp_packet_t pkt, const void *data, size_t len)
{
	brtp_parser_amr_t parser = (brtp_parser_amr_t)parser_;
	BSTD_UNUSED(len);

	BDBG_OBJECT_ASSERT(parser, brtp_parser_amr_t);
	BDBG_ASSERT(parser->parent.stream.mux);
	BDBG_MSG(("Len (%d) %d -> Data %x %x %x %x %x %x %x\n\n",pkt->len,len,B_RTP_LOAD8(data,0),B_RTP_LOAD8(data,1),B_RTP_LOAD8(data,2),B_RTP_LOAD8(data,3),B_RTP_LOAD8(data,4),B_RTP_LOAD8(data,5),B_RTP_LOAD8(data,6),B_RTP_LOAD8(data,7)));

	uint8_t cmr, toc[MAX_TOC_ENTRIES];
	WAVEFORMATEX_t waveformatex;
	unsigned frameLength=0;
	unsigned long i, num;
	int octet_align=1;
	uint8_t value;

	uint8_t *rtpBuffer;
	uint32_t input_bytecnt = 0;
	uint32_t output_bytecnt = 0;
	uint8_t amr_header;
	uint32_t byteNum;
#ifdef BCMA_HEADER
	uint8_t bcmaHeader[8] = {'B', 'C', 'M', 'A', 0x00, 0x00, 0x00, 0x00};
	BDBG_MSG(("bcmaHeader"));

#else
	uint8_t bcm3Header[17] = {'B', 'C', 'M', '3', 
			                              0x00, 0x00, 0x00, 0x00, 
			                              0x00, 0x00, 0x00, 0x00, 
			                              0x00, 0x00, 0x00, 0x00, 
			                              0x00};
	BDBG_MSG(("bcm3Header"));

#endif								 

	 rtpBuffer = (uint8_t *)BKNI_Malloc(2*len*sizeof(char));


	/* Read CMR/TOC entries */        
	if (octet_align){
		/* First byte is CMR + 4 zero bits */
		value = B_RTP_LOAD8(data, input_bytecnt++);
		cmr = value >> 4;

		/* Ignoring ILL/ILP byte, must be indicated out of band */
		for ( num = 0; num < MAX_TOC_ENTRIES; num++ )
		{
			toc[num] = B_RTP_LOAD8(data,input_bytecnt++);
			toc[num] >>= 2;  /* Discard padding bits */
			if ( 0 == (toc[num] & (1<<5)) )
			break;
		}

		if ( num == MAX_TOC_ENTRIES )
		{
			BDBG_ERR(("TOC overflow")); 
			return;
		}

		BDBG_MSG(("Found %u TOC entries\n", num+1));
		/* Ignoring CRC */            
	}
	else	{
		/* Not implementing non-octed-aligned for now... */
		BDBG_ERR(("Only octet-aligned streams are supported")); 
		return;
	}
               
        for ( i = 0; i <= num; i++ )               
        {
//      		BDBG_MSG(("Input Byte Cnt len %d Output Byte Cnt %d",input_bytecnt,output_bytecnt));

		unsigned frameType = (toc[i]>>1)&0xf;
		frameLength = g_frameTypeToBytes[frameType];
		if ( 0 == frameLength ){
		    return;
		}

		/* Populate bcma/waveformatex fields */
		frameLength++;

#ifdef BCMA_HEADER
		/* Setup BCMA packet length */
		WRITE_BE_32(&bcmaHeader[4], frameLength);
		/* Populate WAVEFORMATEX */
		WRITE_LE_16((uint8_t *)&waveformatex.wFormatTag, 0x0501);
		WRITE_LE_16((uint8_t *)&waveformatex.nChannels, 0x0001);
		WRITE_LE_32((uint8_t *)&waveformatex.nSamplesPerSec, 8000);
		WRITE_LE_32((uint8_t *)&waveformatex.nAvgBytesPerSec, frameType);
		WRITE_LE_16((uint8_t *)&waveformatex.nBlockAlign, 0);
		WRITE_LE_16((uint8_t *)&waveformatex.wBitsPerSample, 16);
		WRITE_LE_16((uint8_t *)&waveformatex.cbSize, 0);

		
		/* Write BCMA & WAVEFORMATEX */
		//fwrite(bcmaHeader, 1, sizeof(bcmaHeader), stdout);
		BKNI_Memcpy(&rtpBuffer[output_bytecnt], bcmaHeader, sizeof(bcmaHeader));
		output_bytecnt += sizeof(bcmaHeader);

		//fwrite(&waveformatex, 1, WAVEFORMATEX_SIZE, stdout);
		BKNI_Memcpy(&rtpBuffer[output_bytecnt], &waveformatex, WAVEFORMATEX_SIZE);
		output_bytecnt += WAVEFORMATEX_SIZE;
#else

		/* Add the Header size */
		frameLength+=sizeof(bcm3Header);

		/* Setup BCM3 packet length */
		WRITE_BE_32(&bcm3Header[4], frameLength);
		/* Populate BCM3 data (only num frames is required) */
		bcm3Header[16] = 1;
		/* Write BCM3 */
		//fwrite(bcm3Header, 1, sizeof(bcm3Header), stdout);
		BKNI_Memcpy(&rtpBuffer[output_bytecnt], bcm3Header, sizeof(bcm3Header));
		output_bytecnt += sizeof(bcm3Header);

#endif
		/* Write AMR header */
		//fputc((toc[i]<<2) & 0x7f, stdout);
		amr_header = ((toc[i]<<2) & 0x7f);
		rtpBuffer[output_bytecnt++] = amr_header;

		for ( byteNum = 0; byteNum < g_frameTypeToBytes[frameType]; byteNum++ )
		{
			// fputc(fgetc(pInFile), stdout);
			rtpBuffer[output_bytecnt++] = B_RTP_LOAD8(data,input_bytecnt++);
			
		}
//		BDBG_MSG(("Frame len %d Byte Cnt %d",frameLength,output_bytecnt));

	}
    
//	BDBG_MSG(("Len %d Byte Cnt %d\n\n",len,output_bytecnt));
//	BDBG_MSG(("rtpBuffer %x %x %x %x %x %x %x\n\n",B_RTP_LOAD8(rtpBuffer,0),B_RTP_LOAD8(rtpBuffer,1),B_RTP_LOAD8(rtpBuffer,2),B_RTP_LOAD8(rtpBuffer,3),B_RTP_LOAD8(rtpBuffer,4),B_RTP_LOAD8(rtpBuffer,5),B_RTP_LOAD8(rtpBuffer,6),B_RTP_LOAD8(rtpBuffer,7)));


	b_rtp_parser_amr_begin(parser);
	brtp_parser_add_packet(&parser->parent, pkt, rtpBuffer, output_bytecnt);
	b_rtp_parser_amr_commit(parser);

	BKNI_Free(rtpBuffer);

	return;
}

static void 
b_rtp_parser_amr_discontinuity(brtp_parser_t parser_)
{
	brtp_parser_amr_t parser = (brtp_parser_amr_t)parser_;
	BDBG_OBJECT_ASSERT(parser, brtp_parser_amr_t);
	BDBG_ASSERT(parser->parent.stream.mux);
	brtp_parser_abort(&parser->parent);
	b_rtp_parser_amr_begin(parser);
	return;
}


static brtp_parser_mux_stream_t
b_rtp_parser_amr_start(brtp_parser_t parser_, brtp_parser_mux_t mux, const brtp_parser_mux_stream_cfg *cfg, const void *amr_cfg_, size_t amr_cfg_len)
{
	brtp_parser_amr_t parser = (brtp_parser_amr_t)parser_;
	const brtp_parser_amr_stream_cfg *amr_cfg = amr_cfg_;

	BDBG_OBJECT_ASSERT(parser, brtp_parser_amr_t);
	BDBG_ASSERT(parser->parent.stream.mux==NULL);
	BDBG_ASSERT(cfg);
	BDBG_ASSERT(amr_cfg);
	BDBG_ASSERT(sizeof(*amr_cfg)==amr_cfg_len);

	parser->stream_cfg = *amr_cfg;
	brtp_parser_mux_attach(mux, parser_, cfg);
	return  &parser->parent.stream;

}

void 
b_rtp_parser_amr_stop(brtp_parser_t parser_)
{
	brtp_parser_amr_t parser = (brtp_parser_amr_t)parser_;
	BDBG_OBJECT_ASSERT(parser, brtp_parser_amr_t);
	brtp_parser_stop(&parser->parent);
	return;
}

static void
b_rtp_parser_amr_destroy(brtp_parser_t parser_)
{
	brtp_parser_amr_t parser = (brtp_parser_amr_t)parser_;

	BDBG_OBJECT_ASSERT(parser, brtp_parser_amr_t);
	b_rtp_parser_amr_stop(parser_);

	BDBG_OBJECT_DESTROY(parser, brtp_parser_amr_t);
	BKNI_Free(parser);
	return;
}

brtp_parser_t 
brtp_parser_amr_create(const brtp_parser_amr_cfg *cfg)
{
	brtp_parser_amr_t parser;

	BDBG_ASSERT(cfg);
	parser = BKNI_Malloc(sizeof(*parser));
	if (!parser) {
		BDBG_ERR(("brtp_parser_amr_create: out of memory")); 
		return NULL;
	}
	BDBG_OBJECT_INIT(parser, brtp_parser_amr_t);
	brtp_parser_init(&parser->parent);
	parser->parent.packet = b_rtp_parser_amr_packet;
	parser->parent.discontinuity = b_rtp_parser_amr_discontinuity;
	parser->parent.start = b_rtp_parser_amr_start;
	parser->parent.stop = b_rtp_parser_amr_stop;
	parser->parent.destroy = b_rtp_parser_amr_destroy;
	parser->parent.type = brtp_parser_amr;
	parser->cfg = *cfg;

	BDBG_ERR(("brtp_parser_amr_create: done")); 

	return &parser->parent;
}



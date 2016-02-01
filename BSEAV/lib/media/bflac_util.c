/***************************************************************************
 *     Copyright (c) 2011 Broadcom Corporation
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
 * FLAC parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bflac_util.h"
#include "bkni.h"
#include "biobits.h"

BDBG_MODULE(bflac_util);

#define BDBG_MSG_TRACE(x)

bool bflac_parse_metadata_block_header(batom_cursor *cursor, bflac_metadata_block_header *header)
{
    int data;

    data = batom_cursor_next(cursor);
    header->last = B_GET_BIT(data, 7);
    header->type = B_GET_BITS(data, 6, 0);
    header->length = batom_cursor_uint24_be(cursor);
    if(BATOM_IS_EOF(cursor)) {
        return false;
    }
    BDBG_MSG(("block_header:%p type:%u length:%u %s", (void *)cursor, header->type, (unsigned)header->length, header->last?"last":""));
    return true;
}

bool bflac_parse_streaminfo(batom_cursor *cursor, bflac_streaminfo *streaminfo)
{
    uint32_t data;

    streaminfo->blocksize_min = batom_cursor_uint16_be(cursor);
    streaminfo->blocksize_max = batom_cursor_uint16_be(cursor);
    streaminfo->framesize_min = batom_cursor_uint24_be(cursor);
    streaminfo->framesize_max = batom_cursor_uint24_be(cursor);
    data = batom_cursor_uint32_be(cursor);
    streaminfo->sample_rate = B_GET_BITS(data, 31, 12);
    streaminfo->channel_count = 1+B_GET_BITS(data, 11, 9);
    streaminfo->sample_size = 1+B_GET_BITS(data, 8, 4);
    streaminfo->total_samples = batom_cursor_uint32_be(cursor) + (((uint64_t)B_GET_BITS(data, 3, 0))<<32);
    if(BATOM_IS_EOF(cursor)) {
        return false;
    }
    BDBG_MSG(("streaminfo:%p blocksize:%u,%u framesize:%u,%u sample_rate:%u channel_count:%u sample_size:%u samples:" B_OFFT_FMT, (void *)cursor, streaminfo->blocksize_min, streaminfo->blocksize_max, streaminfo->framesize_min, streaminfo->framesize_max, streaminfo->sample_rate, streaminfo->channel_count, streaminfo->sample_size, B_OFFT_ARG(streaminfo->total_samples)));
    return true;
}

static uint8_t b_flac_crc8_update_one(uint8_t crc, uint8_t data)
{
/* CRC-8, poly = x^8 + x^2 + x^1 + x^0, init = 0 */
static const uint8_t b_flac_crc8_table[0x100] = {
	0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d,  /* 00 */
	0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65, 0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d,  /* 10 */
	0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb, 0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,  /* 20 */
	0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8, 0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd,  /* 30 */
	0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2, 0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea,  /* 40 */
	0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,  /* 50 */
	0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32, 0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a,  /* 60 */
	0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42, 0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a,  /* 70 */
	0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c, 0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,  /* 80 */
	0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec, 0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4,  /* 90 */
	0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c, 0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44,  /* A0 */
	0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,  /* B0 */
	0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78, 0x7f, 0x6a, 0x6d, 0x64, 0x63,  /* C0 */
	0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b, 0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13,  /* D0 */
	0xae, 0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,  /* E0 */
	0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb, 0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3   /* F0 */
};

	return b_flac_crc8_table [crc ^ data];
}

static uint8_t b_flac_crc8(batom_cursor *cursor, size_t bytes)
{
    uint8_t crc;

    for(crc=0;bytes>0;bytes--) {
        int data;
        BATOM_NEXT(data, cursor);
        if(data==BATOM_EOF) {
            BDBG_WRN(("crc8 premature end, left %u", (unsigned)bytes));
            break;
        }
        crc = b_flac_crc8_update_one(crc, data);
    }
    return crc;
}

static uint16_t b_flac_crc16_update_one(uint16_t crc, uint8_t data)
{
/* CRC-16, poly = x^16 + x^15 + x^2 + x^0, init = 0 */
static const uint16_t b_flac_crc16_table[0x100] = {
	0x0000, 0x8005, 0x800f, 0x000a, 0x801b, 0x001e, 0x0014, 0x8011, 0x8033, 0x0036, 0x003c, 0x8039, 0x0028, 0x802d, 0x8027, 0x0022, /* 00 */
	0x8063, 0x0066, 0x006c, 0x8069, 0x0078, 0x807d, 0x8077, 0x0072, 0x0050, 0x8055, 0x805f, 0x005a, 0x804b, 0x004e, 0x0044, 0x8041, /* 10 */
	0x80c3, 0x00c6, 0x00cc, 0x80c9, 0x00d8, 0x80dd, 0x80d7, 0x00d2, 0x00f0, 0x80f5, 0x80ff, 0x00fa, 0x80eb, 0x00ee, 0x00e4, 0x80e1, /* 20 */
	0x00a0, 0x80a5, 0x80af, 0x00aa, 0x80bb, 0x00be, 0x00b4, 0x80b1, 0x8093, 0x0096, 0x009c, 0x8099, 0x0088, 0x808d, 0x8087, 0x0082, /* 30 */
	0x8183, 0x0186, 0x018c, 0x8189, 0x0198, 0x819d, 0x8197, 0x0192, 0x01b0, 0x81b5, 0x81bf, 0x01ba, 0x81ab, 0x01ae, 0x01a4, 0x81a1, /* 40 */
	0x01e0, 0x81e5, 0x81ef, 0x01ea, 0x81fb, 0x01fe, 0x01f4, 0x81f1, 0x81d3, 0x01d6, 0x01dc, 0x81d9, 0x01c8, 0x81cd, 0x81c7, 0x01c2, /* 50 */
	0x0140, 0x8145, 0x814f, 0x014a, 0x815b, 0x015e, 0x0154, 0x8151, 0x8173, 0x0176, 0x017c, 0x8179, 0x0168, 0x816d, 0x8167, 0x0162, /* 60 */
	0x8123, 0x0126, 0x012c, 0x8129, 0x0138, 0x813d, 0x8137, 0x0132, 0x0110, 0x8115, 0x811f, 0x011a, 0x810b, 0x010e, 0x0104, 0x8101, /* 70 */
	0x8303, 0x0306, 0x030c, 0x8309, 0x0318, 0x831d, 0x8317, 0x0312, 0x0330, 0x8335, 0x833f, 0x033a, 0x832b, 0x032e, 0x0324, 0x8321, /* 80 */
	0x0360, 0x8365, 0x836f, 0x036a, 0x837b, 0x037e, 0x0374, 0x8371, 0x8353, 0x0356, 0x035c, 0x8359, 0x0348, 0x834d, 0x8347, 0x0342, /* 90 */
	0x03c0, 0x83c5, 0x83cf, 0x03ca, 0x83db, 0x03de, 0x03d4, 0x83d1, 0x83f3, 0x03f6, 0x03fc, 0x83f9, 0x03e8, 0x83ed, 0x83e7, 0x03e2, /* A0 */
	0x83a3, 0x03a6, 0x03ac, 0x83a9, 0x03b8, 0x83bd, 0x83b7, 0x03b2, 0x0390, 0x8395, 0x839f, 0x039a, 0x838b, 0x038e, 0x0384, 0x8381, /* B0 */
	0x0280, 0x8285, 0x828f, 0x028a, 0x829b, 0x029e, 0x0294, 0x8291, 0x82b3, 0x02b6, 0x02bc, 0x82b9, 0x02a8, 0x82ad, 0x82a7, 0x02a2, /* C0 */
	0x82e3, 0x02e6, 0x02ec, 0x82e9, 0x02f8, 0x82fd, 0x82f7, 0x02f2, 0x02d0, 0x82d5, 0x82df, 0x02da, 0x82cb, 0x02ce, 0x02c4, 0x82c1, /* D0 */
	0x8243, 0x0246, 0x024c, 0x8249, 0x0258, 0x825d, 0x8257, 0x0252, 0x0270, 0x8275, 0x827f, 0x027a, 0x826b, 0x026e, 0x0264, 0x8261, /* E0 */
	0x0220, 0x8225, 0x822f, 0x022a, 0x823b, 0x023e, 0x0234, 0x8231, 0x8213, 0x0216, 0x021c, 0x8219, 0x0208, 0x820d, 0x8207, 0x0202  /* F0 */
};
    return (crc<<8) ^ b_flac_crc16_table[(crc >> 8) ^ data];
}

#if 0
static uint16_t b_flac_crc16(batom_cursor *cursor, size_t bytes)
{
    uint16_t crc;

    for(crc=0;bytes>0;bytes--) {
        int data;
        BATOM_NEXT(data, cursor);
        if(data==BATOM_EOF) {
            BDBG_WRN(("crc16 premature end, left %u", bytes));
            break;
        }
        crc = b_flac_crc16_update_one(crc, data);
    }
    return crc;
}
#endif


#define B_FLAC_ERROR_VAL2(error,desc,v1,v2) do { BDBG_MSG(("FRAME_ERROR: %s " desc " at line:%u", #error, v1, v2, __LINE__));goto err_##error;}while(0)
#define B_FLAC_ERROR_VAL(error,desc,v1) B_FLAC_ERROR_VAL2(error, desc "%s", v1, "")
#define B_FLAC_ERROR(error,desc) B_FLAC_ERROR_VAL(error, "%s", desc)
#define B_FLAC_FRAME_TEST_EOS(bs) do { if(batom_bitstream_eof(bs)) { B_FLAC_ERROR(eos, "end of data");} } while(0)

static bool b_flac_read_utf8(batom_bitstream *bs, unsigned bits, uint64_t *result)
{
    unsigned data;

    BDBG_ASSERT(bits>0 && bits%8==0); /* number of bits is not stream dependent */
    data = batom_bitstream_bits(bs, 8);
    B_FLAC_FRAME_TEST_EOS(bs);
    if(B_GET_BIT(data,7)==0) {
        *result = data;
    } else {
        uint64_t r;
        unsigned n;
        unsigned i;

        if(B_GET_BITS(data, 7,5)==0x6) {
            r = B_GET_BITS(data, 4, 0);
            n = 1;
        } else if(B_GET_BITS(data, 7, 4)==0xE) {
            r = B_GET_BITS(data, 3, 0);
            n = 2;
        } else if(B_GET_BITS(data, 7, 3)==0x1E) {
            r = B_GET_BITS(data, 2, 0);
            n = 3;
        } else if(B_GET_BITS(data, 7, 2)==0x3E) {
            r = B_GET_BITS(data, 1, 0);
            n = 3;
        } else if(B_GET_BITS(data, 7, 1)==0x7E) {
            r = B_GET_BITS(data, 0, 0);
            n = 4;
        } else {
            B_FLAC_ERROR_VAL(utf8,"invalid header %#x", data);
        }
        if(bits < 8*(n+1)) {
            B_FLAC_ERROR_VAL(utf8, "utf8 overflow %u", n);
        }
        for(i=0;i<n;i++) {
            data = batom_bitstream_bits(bs, 8);
            B_FLAC_FRAME_TEST_EOS(bs);
            if(B_GET_BITS(data, 7, 6)!=2) {
                B_FLAC_ERROR_VAL(utf8,"invalid code %#x",data);
            }
            r = (r<<6)|data;
        }
        *result = r;
    }
    return true;

err_eos:
err_utf8:
    return false;
}

bool
bflac_parser_find_frame_end(batom_cursor *cursor, size_t min_size, size_t header_size)
{
    uint16_t crc_computed;
    uint16_t crc_read;
    unsigned i;
    enum {header, warmup, compare} state;
    unsigned offset;


    /* we are looking for CRC-16 match */
    for(offset=0,crc_read=crc_computed=0,state=header, i=0;;) {
        int data;
        uint8_t byte = 0;
        BATOM_NEXT(data, cursor);
        i++;
        offset++;
        /* BDBG_MSG(("state:%u(%u) data:%#x crc:%#x/%#x", state, i, data, crc_computed, crc_read)); */
        if(data==BATOM_EOF) {
            goto err_eof;
        }
        if(state==header) {
            byte = data;
            if(i==header_size) {
                state = warmup; 
                i=0;
            }
        } else if(state==warmup) {
            crc_read = (crc_read<<8) | data;
            if(i==2) {
                state = compare;
            }
            continue; /* skip crc computation */
        } else if(state==compare) {
            byte = (crc_read >> 8); /* previous byte read */
            crc_read = (crc_read<<8) | data; /* build new CRC from stream data */
        }
        crc_computed = b_flac_crc16_update_one(crc_computed, byte);
        if(state==compare) {
            /* BDBG_ERR(("byte %#x crc:%#x/%#x", byte, crc_computed, crc_read)); */
            if(offset>min_size && crc_computed==crc_read) {
                batom_cursor cursor_peek;
                uint16_t sync;
                BATOM_CLONE(&cursor_peek, cursor);
                sync = batom_cursor_uint16_be(&cursor_peek);
                if(BATOM_IS_EOF(&cursor_peek)) {
                    goto err_eof;
                }
                if(B_GET_BITS(sync,15,2)==BFLAC_FRAME_SYNC) {
                    BDBG_MSG(("find_frame_end: found frame boundary at %u crc:%#x", i, crc_read));
                    break;
                } else {
                    BDBG_MSG_TRACE(("find_frame_end: false sync at %u sync:%#x", i, sync));
                }
            }
        }
    }
    return true;
err_eof:
    BDBG_MSG(("find_frame_end: eos at %u(%u)", state, i));
    return false;
}

int
bflac_parse_frame_header(const batom_cursor *cursor, bflac_frame_header *frame, const bflac_streaminfo *streaminfo)
{
    batom_bitstream  bs;
    unsigned data;
    unsigned block_size;
    unsigned sample_rate;
    unsigned channels;
    unsigned sample_size;
    unsigned crc8;
    unsigned frame_crc8;
    batom_cursor cursor_bs;
    size_t header_len;
    int rc=BFLAC_ERROR_BAD_DATA;

    BATOM_CLONE(&cursor_bs, cursor);
    batom_bitstream_init(&bs, &cursor_bs);
                                                                        /* FRAME HEADER */
    data = batom_bitstream_bits(&bs, 14);                               /* <14> Sync code '11111111111110' */
    if(data!=BFLAC_FRAME_SYNC /* 11_1111_1111_1110 */) {
        rc = BFLAC_ERROR_BAD_SYNC;
        B_FLAC_ERROR_VAL2(sync, "%#x != %#x", data, BFLAC_FRAME_SYNC);
    }
    if(batom_bitstream_bit(&bs)) {                                      /* <1>    Reserved: */
        rc = BFLAC_ERROR_BAD_SYNC;
        B_FLAC_ERROR(sync, "sync");
    }
    frame->variable_blocksize = batom_bitstream_bit(&bs);                      /* <1>      Blocking strategy: */
    block_size = batom_bitstream_bits(&bs, 4);                          /* <4>    Block size in inter-channel samples: */
    sample_rate = batom_bitstream_bits(&bs,4);                          /* <4>   Sample rate: */
    channels = batom_bitstream_bits(&bs,4);                             /* <4>   Channel assignment */
    sample_size = batom_bitstream_bits(&bs,3);                          /* <3>   Sample size in bits: */

    if(batom_bitstream_bit(&bs)) {                                      /* <1>   Reserved: */
        B_FLAC_ERROR(sync, "reserved");
    }
    B_FLAC_FRAME_TEST_EOS(&bs);
    /* could used data after test for eos */
    if(frame->variable_blocksize) {                                      /* <?>      if(variable blocksize) */ 
        uint64_t sample_number;
        if(!b_flac_read_utf8(&bs, 56, &sample_number)) {                 /* <8-56>:"UTF-8" coded sample number (decoded number is 36 bits) */
            B_FLAC_ERROR(frame, "sample_number");
        } 
    } else {
        uint64_t frame_number;
        if(!b_flac_read_utf8(&bs, 48, &frame_number)) {                  /* <8-48>:"UTF-8" coded frame number (decoded number is 31 bits) */
            B_FLAC_ERROR(frame, "frame_number");
        } 
    }
    if(block_size==0) {                                                 /* 0000 : reserved */
        B_FLAC_ERROR_VAL(frame, "invalid block_size:%u", block_size);
    } else if(block_size == 1) {                                        /* 0001 : 192 samples */
        frame->samples = 192;
    } else if(block_size <= 5) {                                        /* 0010-0101 : 576 * (2^(n-2)) samples, i.e. 576/1152/2304/4608 */
        frame->samples = 576 << (block_size - 2);
    } else if(block_size == 6 ) {                                       /* 0110 : get 8 bit (blocksize-1) from end of header */
        frame->samples = 1+batom_bitstream_bits(&bs, 8);
    } else if(block_size == 7) {                                        /* 0111 : get 16 bit (blocksize-1) from end of header */
        frame->samples = 1+batom_bitstream_bits(&bs, 16);
    } else {                                                            /* 1000-1111 : 256 * (2^(n-8)) samples, i.e. 256/512/1024/2048/4096/8192/16384/32768 */
        BDBG_ASSERT(block_size < 0x10);
        frame->samples = 256 << (block_size - 8);
    }

    switch(sample_rate) {
    case 0x0: frame->sample_rate = streaminfo->sample_rate;             /* 0000 : get from STREAMINFO metadata block */
              break;
    case 0x1: frame->sample_rate = 882000;                              /* 0001 : 88.2kHz */
              break;
    case 0x2: frame->sample_rate = 176400;                              /* 0010 : 176.4kHz */
              break;
    case 0x3: frame->sample_rate = 192000;                              /* 0011 : 192kHz */
              break;
    case 0x4: frame->sample_rate = 8000;                                /* 0100 : 8kHz */
              break;
    case 0x5: frame->sample_rate = 16000;                               /* 0101 : 16kHz */
              break;
    case 0x6: frame->sample_rate = 22050;                               /* 0110 : 22.05kHz */
              break;
    case 0x7: frame->sample_rate = 24000;                               /* 0111 : 24kHz */
              break;
    case 0x8: frame->sample_rate = 32000;                               /* 1000 : 32kHz */
              break;
    case 0x9: frame->sample_rate = 44100;                               /* 1001 : 44.1kHz */
              break;
    case 0xA: frame->sample_rate = 48000;                               /* 1010 : 48kHz */
              break;
    case 0xB: frame->sample_rate = 96000;                               /* 1011 : 96kHz */
              break;
    case 0xC: frame->sample_rate = 1000 * batom_bitstream_bits(&bs, 8); /* 1100 : get 8 bit sample rate (in kHz) from end of header */
              break;
    case 0xD: frame->sample_rate = batom_bitstream_bits(&bs, 16);       /* 1101 : get 16 bit sample rate (in Hz) from end of header */
              break;
    case 0xE: frame->sample_rate = 10 * batom_bitstream_bits(&bs, 16);  /* 1110 : get 16 bit sample rate (in tens of Hz) from end of header */
              break;
    case 0xF:                                                           /* 1111 : invalid, to prevent sync-fooling string of 1s */
    default: B_FLAC_ERROR(sync, "reserved");
    }

    if(channels<8) {                                                    /* 0000-0111 : (number of independent channels)-1. */
        frame->channel_count = channels+1; 
    } else if (channels<0xB) {                                          /* 1000 : left/side stereo: channel 0 is the left channel, channel 1 is the side(difference) channel */
                                                                        /* 1001 : right/side stereo: channel 0 is the side(difference) channel, channel 1 is the right channel */
        frame->channel_count = 2;                                       /* 1010 : mid/side stereo: channel 0 is the mid(average) channel, channel 1 is the side(difference) channel */
    } else {                                                            /* 1011-1111 : reserved */
        B_FLAC_ERROR(sync, "reserved");
    }

    switch(sample_size) {
    case 0: frame->sample_size = streaminfo->sample_size;               /* 000 : get from STREAMINFO metadata block */
            break;
    case 1: frame->sample_size = 8;                                     /* 001 : 8 bits per sample */
            break;
    case 2: frame->sample_size = 12;                                    /* 010 : 12 bits per sample */
            break;
    case 3: B_FLAC_ERROR(sync, "reserved");                             /* 011 : reserved */
            break;
    case 4: frame->sample_size = 16;                                    /* 100 : 16 bits per sample */
            break;
    case 5: frame->sample_size = 20;                                    /* 101 : 20 bits per sample */
            break;
    case 6: frame->sample_size = 24;                                    /* 110 : 24 bits per sample */
            break;
    case 7:                                                             /* 111 : reserved */
    default: B_FLAC_ERROR(sync, "reserved");
    }

    header_len = batom_bitstream_position(&bs) - batom_cursor_pos(cursor);
    crc8 = batom_bitstream_bits(&bs,8);                                 /* <8>   CRC-8 (polynomial = x^8 + x^2 + x^1 + x^0, initialized with 0) of everything before the crc, including the sync code */
    B_FLAC_FRAME_TEST_EOS(&bs);

    BATOM_CLONE(&cursor_bs, cursor);
    frame_crc8 = b_flac_crc8(&cursor_bs, header_len);
    if(frame_crc8!=crc8) {
        rc = BFLAC_ERROR_BAD_CRC;
        B_FLAC_ERROR_VAL2(frame,"invalid crc %#x!=%#x", crc8, frame_crc8);
    }
    header_len+=BMEDIA_FIELD_LEN(crc8, uint8_t);
    BDBG_MSG(("found frame header:%u samples:%u", (unsigned)header_len, frame->samples));

    return (int)header_len;

err_frame:
err_sync:
    B_FLAC_FRAME_TEST_EOS(&bs);
    return rc;
err_eos:
    return BFLAC_ERROR_EOS;
}


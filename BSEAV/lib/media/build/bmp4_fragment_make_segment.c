/***************************************************************************
 *     Copyright (c) 2007-2012, Broadcom Corporation
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
 * MP4 container parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef BMP4_FRAGMENT_MAKE_EMBEDDED
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#endif

static void 
write_uint8(FILE *fout, uint8_t d)
{
    fwrite(&d, sizeof(d), 1, fout);
}

#if 0
static void
write_uint16_be(FILE *fout, uint16_t data)
{
    uint8_t d[2];
    d[0] = data >> 8;
    d[1] = data;
    fwrite(d, sizeof(d), 1, fout);
}
#endif

static void
write_uint24_be(FILE *fout, uint32_t data)
{
    uint8_t d[3];
    d[0] = data >> 16;
    d[1] = data >> 8;
    d[2] = data;
    fwrite(d, sizeof(d), 1, fout);
}

static void
write_uint32_be(FILE *fout, uint32_t data)
{
    uint8_t d[4];
    d[0] = data >> 24;
    d[1] = data >> 16;
    d[2] = data >> 8;
    d[3] = data;
    fwrite(d, sizeof(d), 1, fout);
}

static void 
write_data(FILE *fout, const uint8_t *data, size_t len)
{
    fwrite(data, len, 1, fout);
}

static void 
write_fourcc(FILE *fout, const char *fourcc)
{
    write_data(fout, (const uint8_t *)fourcc, 4);
}

static int
start_mp4_box(FILE *fout, const char *type)
{
    int offset = ftell(fout);
    write_uint32_be(fout, 0); /* size */
    write_fourcc(fout, type); /* type */
    return offset;
}

static void
finish_mp4_box(FILE *fout, int offset)
{
    int current = ftell(fout);

    fseek(fout, offset, SEEK_SET);
    write_uint32_be(fout, current-offset); /* size */
    fseek(fout, current, SEEK_SET);
    return;
}

static int 
start_mp4_fullbox(FILE *fout, const char *type, unsigned version, uint32_t flags)
{
    int offset = start_mp4_box(fout, type);
    write_uint8(fout, version);
    write_uint24_be(fout, flags);
    return offset; 
}

#ifndef BMP4_FRAGMENT_PRIMITIVES_ONLY
static void
write_trex_box(FILE *fout,  uint32_t track_ID, uint32_t default_sample_description_index, uint32_t default_sample_duration, uint32_t default_sample_size, uint32_t default_sample_flags)
{
    int offset = start_mp4_fullbox(fout, "trex", 0, 0);
    write_uint32_be(fout, track_ID);
    write_uint32_be(fout, default_sample_description_index);
    write_uint32_be(fout, default_sample_duration);
    write_uint32_be(fout, default_sample_size);
    write_uint32_be(fout, default_sample_flags);
    finish_mp4_box(fout, offset);
    return;
}

static void
write_bdat_box(FILE *fout,  const uint8_t *data, size_t data_len)
{
    int offset = start_mp4_box(fout, "bdat");
    write_data(fout, data, data_len);
    finish_mp4_box(fout, offset);
    return;
}

static void
write_uint64_be(FILE *fout, uint64_t data)
{
    uint8_t d[8];
    d[0] = data >> 56;
    d[1] = data >> 48;
    d[2] = data >> 40;
    d[3] = data >> 32;
    d[4] = data >> 24;
    d[5] = data >> 16;
    d[6] = data >> 8;
    d[7] = data;
    fwrite(d, sizeof(d), 1, fout);
}


static void
write_bhed_box(FILE *fout, uint64_t timescale, uint64_t start_time, const char *fourcc)
{
    int offset = start_mp4_box(fout, "bhed");
    write_uint64_be(fout, timescale);
    write_uint64_be(fout, start_time);
    write_fourcc(fout, fourcc);
    finish_mp4_box(fout, offset);
    return;
}

static int
h2b(char code)
{
    if(code>='0' && code<='9') {
        return code - '0';
    } else if (code>='A' && code<='F') {
        return code - 'A' + 0x0A;
    } else if (code>='a' && code<='f') {
        return code - 'a' + 0x0a;
    }
    return -1;
}

static int
hex2bin(const char *data, uint8_t *buf, size_t max_len)
{
    unsigned i;
    for(i=0;;i++) {

        char ch1 = data[2*i];
        char ch2 = data[2*i+1];
        int b1, b2;
        if(i>=max_len) {
            return -1;
        }
        if(ch1==0 || ch2==0) {
            break;
        }
        b1 = h2b(ch1);
        b2 = h2b(ch2);
        if(b1<0 || b2<0) {
            return -1;
        }
        buf[i]= (b1<<4) | b2;
       
    }
    return i;
}

#endif /* BMP4_FRAGMENT_PRIMITIVES_ONLY */

#ifndef BMP4_FRAGMENT_MAKE_EMBEDDED
static void
usage(const char *app)
{
    fprintf(stderr, "Usage:\n"
                    "%s <output-file> <source-fragment-file> <track_ID> <codec_private_data> <fourcc> [<start_time>] [<time_scale>]\n", app);
    return;
}

int 
main(int argc, const char *argv[])
{
    const char *output;
    const char *fragment;
    const char *codec_private_data;
    const char *fourcc;
    uint32_t track_ID;
    uint64_t start_time;
    uint64_t timescale;
    FILE *fout, *fin;
    int offset;
    int codec_private_data_len;
    uint8_t buf[1024];

    if(argc<5) {
        goto error;
    }
    output = argv[1];
    fragment = argv[2];
    track_ID = strtol(argv[3],NULL,0);
    codec_private_data=argv[4];
    fourcc = argv[5];
    start_time = (argc>6)?strtoll(argv[6], NULL, 0):0;
    timescale =  (argc>7)?strtoll(argv[7], NULL, 0):10000000;

    fprintf(stderr, "converting %s -> %s track_ID:%u codec_private_data:'%s' fourcc:'%s' start_time:%u timescale:%u \n", fragment, output, (unsigned)track_ID, codec_private_data, fourcc, (unsigned)start_time, (unsigned)timescale);
    fout = fopen(output, "wb");
    if(!fout) { perror(output);goto error;}
    fin = fopen(fragment, "rb");
    if(!fin) { perror(fragment);return 1;}
    codec_private_data_len = hex2bin(codec_private_data, buf, sizeof(buf));
    if(codec_private_data_len<0) { fprintf(stderr,"invalid codec private data'%s'\n", codec_private_data);goto error;}
    offset = start_mp4_box(fout, "bmp4");
    write_bhed_box(fout, timescale, start_time, fourcc);
    write_bdat_box(fout, buf, codec_private_data_len);
    write_trex_box(fout,  track_ID, 0, 0, 0, 0);
    for(;;) {
        int rc;
        rc = fread(buf, 1, sizeof(buf), fin);
        if(rc<=0) {
            break;
        }
        write_data(fout, buf, rc);
    }
    finish_mp4_box(fout, offset);
    fclose(fout);
    fclose(fin);


    return 0;
error:
    usage(argv[0]);
    return 1;
}
#endif

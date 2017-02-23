/***************************************************************************
 * Copyright (C) 2007-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * BMedia library, stream probe module
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bmp3_probe.h"
#include "bkni.h"
#include "biobits.h"
#include "bmpeg_audio_probe.h"
#include "bid3_parser.h"

BDBG_MODULE(bmp3_probe);

#define BDBG_MSG_TRACE(x)   BDBG_MSG(x)

typedef struct bmp3_probe  *bmp3_probe_t; 

struct bmp3_probe {
    BDBG_OBJECT(bmp3_probe_t)
    batom_accum_t acc;
    bmedia_probe_track *track;
	bmedia_probe_base_es_t mp3_es_probe;
	off_t parse_offset;
};

BDBG_OBJECT_ID(bmp3_probe_t);

static const bmedia_probe_file_ext b_mp3_ext[] =  {
    {"mp3"},
    {""}
};

static bool 
b_mp3_probe_header_match_id3v2(batom_cursor *header)
{
    return bid3v2_parser_probe(header);
}

static bool
b_mp3_probe_header_match_mpeg1l3(batom_cursor *header)
{
    uint16_t sync;
    bmedia_probe_audio info;
    bmedia_mpeg_audio_info mpeg_info;

    BDBG_MSG_TRACE(("b_mp3_probe_header_match_mpeg1l3:%#lx", (unsigned long)header));
    sync = batom_cursor_uint16_be(header);
    if(B_GET_BITS(sync, 15,5)!=B_MPEG_AUDIO_SYNC) {
        return false;
    }
    if(bmpeg_audio_probe_parse_header(header, sync, &info, &mpeg_info)==0) {
        return false;
    }
    return true;
}

static bool 
b_mp3_probe_header_match(batom_cursor *header)
{
    batom_cursor header2;

    BATOM_CLONE(&header2, header);
    return b_mp3_probe_header_match_mpeg1l3(header) || b_mp3_probe_header_match_id3v2(&header2);
}

static bmedia_probe_base_t 
b_mp3_probe_create(batom_factory_t factory)
{
    bmp3_probe_t    probe;

    BSTD_UNUSED(factory);
    BDBG_MSG_TRACE(("b_mp3_probe_create"));

    probe = BKNI_Malloc(sizeof(*probe));
    if(!probe) {
        BDBG_ERR(("b_mp3_probe_create: can't allocate %u bytes", (unsigned)sizeof(*probe)));
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(probe, bmp3_probe_t);
    probe->track = NULL;
    probe->acc = batom_accum_create(factory);
    if(!probe->acc) {
        goto err_create_accum;
    }
	probe->mp3_es_probe = bmpeg_audio_probe.create(factory);
	if(!probe->mp3_es_probe) {
		goto err_create_es_probe;
	}

    return (bmedia_probe_base_t)probe;

err_create_es_probe:
	batom_accum_destroy(probe->acc);	
err_create_accum:
    BKNI_Free(probe);
err_alloc:
    return NULL;
}

static void 
b_mp3_probe_destroy(bmedia_probe_base_t probe_)
{
    bmp3_probe_t probe = (bmp3_probe_t)probe_;
    BDBG_OBJECT_ASSERT(probe, bmp3_probe_t);

    batom_accum_destroy(probe->acc);
	bmpeg_audio_probe.destroy(probe->mp3_es_probe);
    BDBG_OBJECT_DESTROY(probe, bmp3_probe_t);
    BKNI_Free(probe);
    return;
}

typedef struct b_string_allocator {
    size_t off;
    size_t buf_size;
    char *buf;
} b_string_allocator;

static bool
b_mp3_probe_copy(bmp3_text_string *string, b_string_allocator *alloc, batom_cursor *cursor, size_t len)
{
    char *last_char;
    char *wrptr;
    char *lastpr;

    string->str = alloc->buf;
    string->len = 0;
    if(alloc->off == alloc->buf_size) {
        BDBG_WRN(("b_mp3_probe_copy: not enough space"));
        batom_cursor_skip(cursor,len);
        return false;
    }
    for(last_char=NULL,wrptr=alloc->buf+alloc->off,lastpr=alloc->buf+(alloc->buf_size-1);len>0;len--) {
        int byte;

        BATOM_NEXT(byte, cursor);
        if(byte==BATOM_EOF) {
            break;
        }
        if(wrptr<lastpr) {
            if(byte!='\0' && byte!=' ') { 
                last_char = wrptr;
            }
            *wrptr = byte;
            wrptr++;
        }
    }
    if(last_char==NULL) { /* if no character, return NULL, and don't use any space */
        return false;
    }
    last_char[1]='\0'; /* truncate upto last NUL or space */
	string->str =  (alloc->buf+alloc->off);
	string->len = (last_char+1) - string->str;
    alloc->off = (last_char+2) - alloc->buf;
    return wrptr;
}

static const char * const b_id3v1_genres[] = {
 /* 0 */    "Blues", 
 /* 1 */    "Classic Rock", 
 /* 2 */    "Country", 
 /* 3 */    "Dance", 
 /* 4 */    "Disco", 
 /* 5 */    "Funk", 
 /* 6 */    "Grunge", 
 /* 7 */    "Hip-Hop", 
 /* 8 */    "Jazz", 
 /* 9 */    "Metal", 
 /* 10 */   "New Age", 
 /* 11 */   "Oldies", 
 /* 12 */   "Other", 
 /* 13 */   "Pop", 
 /* 14 */   "R&B", 
 /* 15 */   "Rap", 
 /* 16 */   "Reggae", 
 /* 17 */   "Rock", 
 /* 18 */   "Techno", 
 /* 19 */   "Industrial", 
 /* 20 */   "Alternative", 
 /* 21 */   "Ska", 
 /* 22 */   "Death Metal", 
 /* 23 */   "Pranks", 
 /* 24 */   "Soundtrack", 
 /* 25 */   "Euro-Techno", 
 /* 26 */   "Ambient", 
 /* 27 */   "Trip-Hop", 
 /* 28 */   "Vocal", 
 /* 29 */   "Jazz+Funk", 
 /* 30 */   "Fusion", 
 /* 31 */   "Trance", 
 /* 32 */   "Classical", 
 /* 33 */   "Instrumental", 
 /* 34 */   "Acid", 
 /* 35 */   "House", 
 /* 36 */   "Game", 
 /* 37 */   "Sound Clip", 
 /* 38 */   "Gospel", 
 /* 39 */   "Noise", 
 /* 40 */   "AlternRock", 
 /* 41 */   "Bass", 
 /* 42 */   "Soul", 
 /* 43 */   "Punk", 
 /* 44 */   "Space", 
 /* 45 */   "Meditative", 
 /* 46 */   "Instrumental Pop", 
 /* 47 */   "Instrumental Rock", 
 /* 48 */   "Ethnic", 
 /* 49 */   "Gothic", 
 /* 50 */   "Darkwave", 
 /* 51 */   "Techno-Industrial", 
 /* 52 */   "Electronic", 
 /* 53 */   "Pop-Folk", 
 /* 54 */   "Eurodance", 
 /* 55 */   "Dream", 
 /* 56 */   "Southern Rock", 
 /* 57 */   "Comedy", 
 /* 58 */   "Cult", 
 /* 59 */   "Gangsta", 
 /* 60 */   "Top 40", 
 /* 61 */   "Christian Rap", 
 /* 62 */   "Pop/Funk", 
 /* 63 */   "Jungle", 
 /* 64 */   "Native American", 
 /* 65 */   "Cabaret", 
 /* 66 */   "New Wave", 
 /* 67 */   "Psychadelic", 
 /* 68 */   "Rave", 
 /* 69 */   "Showtunes", 
 /* 70 */   "Trailer", 
 /* 71 */   "Lo-Fi", 
 /* 72 */   "Tribal", 
 /* 73 */   "Acid Punk", 
 /* 74 */   "Acid Jazz", 
 /* 75 */   "Polka", 
 /* 76 */   "Retro", 
 /* 77 */   "Musical", 
 /* 78 */   "Rock & Roll", 
 /* 79 */   "Hard Rock", 
 /* 80 */   "Folk", 
 /* 81 */   "Folk-Rock", 
 /* 82 */   "National Folk", 
 /* 83 */   "Swing", 
 /* 84 */   "Fast Fusion", 
 /* 85 */   "Bebob", 
 /* 86 */   "Latin", 
 /* 87 */   "Revival", 
 /* 88 */   "Celtic", 
 /* 89 */   "Bluegrass", 
 /* 90 */   "Avantgarde", 
 /* 91 */   "Gothic Rock", 
 /* 92 */   "Progressive Rock", 
 /* 93 */   "Psychedelic Rock", 
 /* 94 */   "Symphonic Rock", 
 /* 95 */   "Slow Rock", 
 /* 96 */   "Big Band", 
 /* 97 */   "Chorus", 
 /* 98 */   "Easy Listening", 
 /* 99 */   "Acoustic", 
 /* 100 */  "Humour", 
 /* 101 */  "Speech", 
 /* 102 */  "Chanson", 
 /* 103 */  "Opera", 
 /* 104 */  "Chamber Music", 
 /* 105 */  "Sonata", 
 /* 106 */  "Symphony", 
 /* 107 */  "Booty Brass", 
 /* 108 */  "Primus", 
 /* 109 */  "Porn Groove", 
 /* 110 */  "Satire", 
 /* 111 */  "Slow Jam", 
 /* 112 */  "Club", 
 /* 113 */  "Tango", 
 /* 114 */  "Samba", 
 /* 115 */  "Folklore", 
 /* 116 */  "Ballad", 
 /* 117 */  "Poweer Ballad", 
 /* 118 */  "Rhytmic Soul", 
 /* 119 */  "Freestyle", 
 /* 120 */  "Duet", 
 /* 121 */  "Punk Rock", 
 /* 122 */  "Drum Solo", 
 /* 123 */  "A Capela", 
 /* 124 */  "Euro-House", 
 /* 125 */  "Dance Hall"
};

#define B_MP3_ID3V1_LEN 128
#define B_MP3_ID3V1_EXT_LEN 227

static void
b_mp3_probe_string_init(const bmp3_probe_stream *stream, bmp3_text_string *string)
{
    string->len = 0;
    string->str = stream->data;
    return;
}


static bmp3_probe_stream *
b_mp3_probe_alloc_stream(b_string_allocator *alloc, size_t data_size)
{
    bmp3_probe_stream *stream = BKNI_Malloc(sizeof(*stream)+data_size);

    if(!stream) {
        return NULL;
    }
    BKNI_Memset(stream, 0, sizeof(*stream)+data_size);
    bmedia_probe_stream_init(&stream->media, bstream_mpeg_type_es);
    stream->media.index = bmedia_probe_index_self;
    stream->data[0] = '\0';
    stream->data[1] = '\0';
    b_mp3_probe_string_init(stream, &stream->title);
    b_mp3_probe_string_init(stream, &stream->artist);
    b_mp3_probe_string_init(stream, &stream->album);
    b_mp3_probe_string_init(stream, &stream->year);
    b_mp3_probe_string_init(stream, &stream->comment);
    if(alloc) {
        alloc->off = 2;
        alloc->buf_size = data_size;
        alloc->buf = stream->data+2;
    }
    return stream;
}

#define B_MP3_PRINT_STR_FORMAT "%s%s%s "
#define B_MP3_PRINT_STR(track,str_) track->str_.len?#str_:"",track->str_.len?":":"",track->str_.str

static size_t 
b_strlen(const char *s)
{
    size_t off;
    for(off=0;s[off]!='\0';off++) { }
    return off;
}


static bmp3_probe_stream *
b_mp3_probe_parse_id3v1(batom_cursor *cursor)
{
    uint32_t id;
    bmp3_probe_stream *stream;
    b_string_allocator alloc;
    uint8_t genre;

    BDBG_MSG_TRACE(("b_mp3_probe_parse_id3v1: %#lx", (unsigned long)cursor));
    /* http://mpgedit.org/mpgedit/mpeg_format/mpeghdr.htm#MPEGTAG */

    id = batom_cursor_uint24_be(cursor);
    if( id != ((((uint32_t)'T')<<16) | (((uint32_t)'A')<<8) | 'G')) {
        return NULL;
    }
    stream = b_mp3_probe_alloc_stream(&alloc, B_MP3_ID3V1_LEN);
    if(!stream) {
        return NULL;
    }

    b_mp3_probe_copy(&stream->title, &alloc, cursor, 30);
    b_mp3_probe_copy(&stream->artist, &alloc, cursor, 30);
    b_mp3_probe_copy(&stream->album, &alloc, cursor, 30);
    b_mp3_probe_copy(&stream->year, &alloc, cursor, 4);
    b_mp3_probe_copy(&stream->comment, &alloc, cursor, 4);
    genre = batom_cursor_byte(cursor);
    if(BATOM_IS_EOF(cursor)) {
        BKNI_Free(stream);
        return NULL;
    }
    BDBG_CASSERT(sizeof(b_id3v1_genres)/sizeof(*b_id3v1_genres)==126);
    if(genre<126) {
        stream->genre.str = b_id3v1_genres[genre];
    } else {
        stream->genre.str = "Unknown";
    }
    stream->genre.len = b_strlen(stream->genre.str);
	stream->id3 = bmp3_id3v1;
    BDBG_MSG(("b_mp3_probe_parse_id3v1: " B_MP3_PRINT_STR_FORMAT B_MP3_PRINT_STR_FORMAT B_MP3_PRINT_STR_FORMAT B_MP3_PRINT_STR_FORMAT B_MP3_PRINT_STR_FORMAT B_MP3_PRINT_STR_FORMAT, B_MP3_PRINT_STR(stream,title), B_MP3_PRINT_STR(stream,artist), B_MP3_PRINT_STR(stream, album), B_MP3_PRINT_STR(stream, year), B_MP3_PRINT_STR(stream, comment), B_MP3_PRINT_STR(stream, genre)));
    return stream;
}

#define BID3V2_ID(c1,c2,c3,c4) (((uint32_t)c4) | ((uint32_t)c3<<8) | ((uint32_t)c2<<16) | ((uint32_t)c1<<24))

#define B_MP3_ID3V2_DATA_LEN (3*1024)

typedef struct b_id3v2_cached_file {
    batom_cursor cursor;
    off_t off;
	off_t last;
    batom_accum_t acc;
    bfile_buffer_t buf;
}  b_id3v2_cached_file;

static bool
b_id3v2_cached_file_load(b_id3v2_cached_file *file, size_t reserve)
{
    const size_t read_len = BMEDIA_PROBE_FEED_SIZE ;
	size_t acc_len;
	batom_accum_t acc = file->acc;

    batom_accum_trim(acc, &file->cursor);
	acc_len = batom_accum_len(acc);
	BDBG_MSG_TRACE(("b_id3v2_cached_file_load: %#lx %d %u:%u:%u", (unsigned long)file, (int)file->last, (unsigned)file->off, (unsigned)acc_len, (unsigned)reserve));
	if(file->last<0 || file->off < file->last) { /* not reached end of data */
		while(reserve > acc_len) {
			bfile_buffer_result result;
			batom_t atom;

			BDBG_MSG_TRACE(("b_id3v2_cached_file_load: %#lx loading %u:%u", (unsigned long)file, (unsigned)file->off, (unsigned)read_len));

			atom = bfile_buffer_read(file->buf, file->off, read_len, &result);
			if(!atom) {
				goto error;
			}
			file->off+=batom_len(atom);
			batom_accum_add_atom(acc, atom);
			batom_release(atom);
			acc_len = batom_accum_len(acc);
		}
	} else {
		/* reached end of data, truncate accumulator */
		if(file->off != file->last)  {
			BDBG_ASSERT(file->off > file->last);
			BDBG_MSG_TRACE(("b_id3v2_cached_file_load: %#lx truncate %u %u", (unsigned long)file, (unsigned)(file->off-file->last), (unsigned)acc_len));
			if(acc_len > (unsigned)(file->off-file->last)) {
				batom_cursor last;
				batom_t atom;
				batom_cursor_from_accum(&file->cursor, acc);
				batom_cursor_from_accum(&last, acc);
				batom_cursor_skip(&last, acc_len - (file->last - file->off));
				file->off = file->last;
				atom = batom_accum_extract(acc, &file->cursor, &last, NULL, NULL);
				batom_accum_clear(acc);
				if(atom) {
					batom_accum_add_atom(acc, atom);
					batom_release(atom);
				}
			} else {
				batom_accum_clear(acc);
			}
		}
	}
    batom_cursor_from_accum(&file->cursor, acc);
    return true;
error:
    batom_accum_clear(acc);
    batom_cursor_from_accum(&file->cursor, acc);
    return false;
}

static void
b_id3v2_cached_file_skip(b_id3v2_cached_file *file, size_t skip)
{
    size_t avail = batom_cursor_skip(&file->cursor, skip);

	BDBG_MSG_TRACE(("b_id3v2_cached_file_skip: %#lx %d %u:%u:%u", (unsigned long)file, (int)file->last, (unsigned)file->off, (unsigned)avail, (unsigned)skip));
    if(skip == avail) {
        batom_accum_trim(file->acc, &file->cursor);
        return ;
    } else {
        file->off += skip-avail;
        batom_accum_clear(file->acc);
    }
    batom_cursor_from_accum(&file->cursor, file->acc);
    return;
}

static bool
b_mp3_probe_copy_id3v2_frame(b_id3v2_cached_file *file, const bid3v2_header *header, bid3v2_frame *frame, b_string_allocator *alloc, bmp3_text_string *string)
{
    char *buf;
    size_t data_len;
    if(frame->flags.compression || frame->flags.encryption) {
        BDBG_WRN(("b_mp3_probe_copy_id3v2_frame: %#lx don't support compressed or encrypted frames", (unsigned long)file));
        b_id3v2_cached_file_skip(file, frame->size);
        return true;
    }
    if(alloc->off>=alloc->buf_size) {
        b_id3v2_cached_file_skip(file, frame->size);
        return true;
    }
    if(!b_id3v2_cached_file_load(file, frame->size)) {
        return false;
    }
    if(!bid3v2_parse_frame_flags(&file->cursor, header, frame)) {
        return false;
    }
    data_len = batom_cursor_pos(&file->cursor);
    if(frame->size <= data_len) {
        return false;
    }
    data_len = frame->size - data_len;
    buf = alloc->buf+alloc->off;
    string->str = buf;
    if(header->flags.unsynchronisation || frame->flags.unsynchronisation) {
        uint16_t acc=batom_cursor_byte(&file->cursor);

        for(acc=0;data_len>1;data_len--) {
            int byte;

            *buf = acc&0xFF;
            BATOM_NEXT(byte, &file->cursor);
            if(byte!=BATOM_EOF) {
                break;
            }
            if(acc!=0xFF00) {
                buf++;
                alloc->off++;
                if(alloc->off >= alloc->buf_size) {
                    string->len = buf - string->str;
                    batom_cursor_skip(&file->cursor,data_len);
                    return true;
                }
            }
            acc = (acc<<8)|(byte&0xFF);
        }
        buf++;
        alloc->off++;
        string->len = buf - string->str;
    } else {
        if(alloc->off + data_len < alloc->buf_size) {
            batom_cursor_copy(&file->cursor, buf, data_len);
            alloc->off += data_len;
            string->len = data_len;
        } else {
            string->len = alloc->buf_size - alloc->off;
            alloc->off = alloc->buf_size;
            batom_cursor_copy(&file->cursor, buf, string->len);
            batom_cursor_skip(&file->cursor, alloc->off + data_len - alloc->buf_size);
        }
    }
    return true;
}

static bmp3_probe_stream *
b_mp3_probe_parse_id3v2(bmp3_probe_t probe, batom_t atom, bfile_buffer_t buf)
{
    bid3v2_header header;
    bid3v2_frame frame;
    bmp3_probe_stream *stream;
    b_string_allocator alloc;
    b_id3v2_cached_file file;

    batom_accum_add_atom(probe->acc, atom);
    batom_cursor_from_accum(&file.cursor, probe->acc);
    file.buf = buf;
    file.acc = probe->acc;
    file.off = probe->parse_offset+batom_len(atom);
	file.last = -1;
    if(!b_id3v2_cached_file_load(&file, 16)) {
        goto err_header;
    }
	
    if(!bid3v2_parse_header(&file.cursor, &header)) {
        goto err_header;
    }
    /* Set the parse offset to skip this chunk */
	probe->parse_offset += header.size + 3 /*ID3 tag*/ + 2 /* ID3 version */ + 1 /* Flags */ + 4 /* Header size */;
    file.last = probe->parse_offset;

    if(header.flags.extended_header) {
        bid3v2_extended_header extended_header;
        if(!b_id3v2_cached_file_load(&file, 16)) {
            goto err_extended_header;
        }
        if(!bid3v2_parse_extended_header(&file.cursor, &extended_header)) {
            goto err_extended_header;
        }
    }
    stream = b_mp3_probe_alloc_stream(&alloc, B_MP3_ID3V2_DATA_LEN);
    if(!stream) { goto err_stream; }
	switch(header.version) {
	case 0:
	case 1:
		stream->id3 = bmp3_id3_none;
		break;
	case 2:
		stream->id3 = bmp3_id3v2_2;
		break;
	case 3:
		stream->id3 = bmp3_id3v2_3;
		break;
	default:
	case 4:
		stream->id3 = bmp3_id3v2_4;
		break;
	}
    for(;;) {
        if(!b_id3v2_cached_file_load(&file, 4/*id*/ + 4/*size*/ + 2/*flags*/)) {
            break;
        }
        if(!bid3v2_parse_frame(&file.cursor, &header, &frame)) {
            break;
        }
		if(frame.id==0 && frame.size==0) {
			break; /* reached padding area */
		}
        switch(frame.id) {
        case BID3V2_ID('T','P','E','1'):
            b_mp3_probe_copy_id3v2_frame(&file, &header, &frame, &alloc, &stream->artist);
            break;
        case BID3V2_ID('T','I','T','2'):
            b_mp3_probe_copy_id3v2_frame(&file, &header, &frame, &alloc, &stream->title);
            break;
        case BID3V2_ID('T','A','L','B'):
            b_mp3_probe_copy_id3v2_frame(&file, &header, &frame, &alloc, &stream->album);
            break;
        case BID3V2_ID('T','D','O','R'):
            b_mp3_probe_copy_id3v2_frame(&file, &header, &frame, &alloc, &stream->year);
            break;
        case BID3V2_ID('T','C','O','N'):
            b_mp3_probe_copy_id3v2_frame(&file, &header, &frame, &alloc, &stream->genre);
            break;
        case BID3V2_ID('C','O','M','M'):
            b_mp3_probe_copy_id3v2_frame(&file, &header, &frame, &alloc, &stream->comment);
            break;
        default:
            b_id3v2_cached_file_skip(&file, frame.size);
            break;
        }
    }
    batom_accum_clear(probe->acc);
    return stream;

err_header:
err_extended_header:
err_stream:
/* err_header: */
    batom_accum_clear(probe->acc);
    return NULL;
}

static const bmedia_probe_stream *
b_mp3_probe_parse(bmedia_probe_base_t probe_, bfile_buffer_t buf, batom_pipe_t pipe, const bmedia_probe_parser_config *config)
{
    bmp3_probe_t probe = (bmp3_probe_t)probe_;
    const size_t read_len = BMEDIA_PROBE_FEED_SIZE;
    bfile_buffer_result result;
    bmp3_probe_stream *stream=NULL;
    bmedia_probe_track *es_track=NULL;
    batom_t atom = NULL;
    batom_cursor cursor;
    int rc;
    off_t first, last;
    off_t read_offset = config->parse_offset;
	size_t parsed;
    bmp3_vbr_frame_info vbr_info;
    bmedia_mpeg_audio_info mpeg_info;
    enum {
        b_mp3_probe_parse_state_search_header, 
        b_mp3_probe_parse_state_id3v1, 
        b_mp3_probe_parse_state_es, 
        b_mp3_probe_parse_state_finished
    } state;

    BDBG_OBJECT_ASSERT(probe, bmp3_probe_t);
    BSTD_UNUSED(pipe);
    BDBG_MSG_TRACE(("b_mp3_probe_parse:%#lx", (unsigned long)probe));

    bmpeg_audio_probe.reset(probe->mp3_es_probe);
    state = b_mp3_probe_parse_state_search_header;
	probe->parse_offset = config->parse_offset;

    for(parsed=0;parsed<bmpeg_audio_probe.max_parsing_size && state!=b_mp3_probe_parse_state_finished;) {
        atom = bfile_buffer_read(buf, read_offset + parsed, read_len, &result);
        if(!atom) {
            goto error;
        }
        batom_cursor_from_atom(&cursor, atom);

        switch(state) {
        case b_mp3_probe_parse_state_search_header:
            if(b_mp3_probe_header_match_id3v2(&cursor)) {
                bmp3_probe_stream *new_stream = b_mp3_probe_parse_id3v2(probe, atom, buf);
                if (!stream) {
                    stream = new_stream;
                } else {
                    /* There are multiple ID3 tags in this MP3, For now just choose the ID3 tag with the 
                       higher revision and delete the other */
                    if (new_stream->id3 > stream->id3) {
                    	BKNI_Free(stream);
                        stream = new_stream;
                    }
                    else {
                    	BKNI_Free(new_stream);
                    }
                }
                read_offset = probe->parse_offset;
                state = b_mp3_probe_parse_state_search_header;
            } else if (stream) {
                /* There was no additional ID3v2 tag detected, however we already one.
                   transition to the ES search */
                read_offset = probe->parse_offset;
                state = b_mp3_probe_parse_state_es;
            } else {
                /* We have not found an ID3v2 header, check for ID3v1 */
		        rc = bfile_buffer_get_bounds(buf, &first, &last);
		        if(rc!=0) {
			        goto error;
		        }
		        if(last < B_MP3_ID3V1_LEN) {
			        goto error;
		        }
                read_offset = last-B_MP3_ID3V1_LEN;
                state = b_mp3_probe_parse_state_id3v1;
            } 
            break;
        case b_mp3_probe_parse_state_id3v1:
	        stream = b_mp3_probe_parse_id3v1(&cursor);
            if(!stream) {
                /* We were not able to find an ID3v1 header, just make an empty stream and start the search for the ES */
                stream = b_mp3_probe_alloc_stream(NULL, 0);
		        if(!stream) {
			        goto error;
		        }
            }
            read_offset = probe->parse_offset;
            state = b_mp3_probe_parse_state_es;
            break;
        case b_mp3_probe_parse_state_es:
			es_track = bmpeg_audio_probe_feed(probe->mp3_es_probe, atom, &mpeg_info, &vbr_info);
			if(es_track) {
				state = b_mp3_probe_parse_state_finished;
			}
		    parsed += batom_len(atom);
            break;
        default:
            break;
		}
		batom_release(atom);
        atom = NULL;
    }

	if(!es_track) {
		unsigned probability;
		es_track = bmpeg_audio_probe.last(probe->mp3_es_probe, &probability);
	}

    if(es_track) {
        BDBG_ASSERT(stream);
        es_track->number = 1;
        /* coverity[uninit_use: FALSE] */ /* mpeg_info and vbr_info always go through bmpeg_audio_probe_feed() */
        if(vbr_info.stream_type!=bmp3_stream_type_unknown) {
            BDBG_ASSERT(es_track->info.audio.sample_rate);
            /* coverity[uninit_use: FALSE] */
            stream->media.duration = ((1000 * mpeg_info.samples * (uint64_t)vbr_info.num_frames) / es_track->info.audio.sample_rate);
            if(stream->media.duration) {
                /* coverity[uninit_use: FALSE] */
                stream->media.max_bitrate = ((1000*8)*(uint64_t)vbr_info.stream_bytes)/stream->media.duration;
                if(stream->media.max_bitrate) {
                    es_track->info.audio.bitrate = stream->media.max_bitrate/1000;
                }
            }
        }
        if(stream->media.max_bitrate==0) {
            stream->media.max_bitrate = es_track->info.audio.bitrate*1000;
        }
        if(stream->media.duration==0 && stream->media.max_bitrate) {
            rc = bfile_buffer_get_bounds(buf, &first, &last);
            if(rc==0) {
                if(last>probe->parse_offset) {
                    stream->media.duration = ((1000*8)*(last - probe->parse_offset))/stream->media.max_bitrate;
                }
            }
        }
        bmedia_probe_add_track(&stream->media, es_track);
    }
    else {
    	BKNI_Free(stream);
		stream = NULL;
    }
    /* return result of parsing */
error:
    bmpeg_audio_probe.reset(probe->mp3_es_probe);
	batom_accum_clear(probe->acc);
    if (atom) {
        batom_release(atom);
    }
    /* coverity[leaked_storage] */
    return stream ? &stream->media : NULL;
}

const bmedia_probe_format_desc bmp3_probe = {
    bstream_mpeg_type_es,
    b_mp3_ext, /* ext_list */
    16, 
    b_mp3_probe_header_match, /* header_match */
    b_mp3_probe_create, /* create */
    b_mp3_probe_destroy, /* destroy */
    b_mp3_probe_parse, /* parse */
    bmedia_probe_basic_stream_free /* stream free */
};




/***************************************************************************
 *     Copyright (c) 2008, Broadcom Corporation
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
 * BMedia library, stream probe module
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BMP3_PROBE_H__
#define _BMP3_PROBE_H__

#include "bmedia_probe_impl.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define BMP3_PROBE_ID   0x8000
typedef struct bmp3_text_string {
    const char *str;
    size_t len;
} bmp3_text_string;

typedef struct bmp3_probe_stream {
	bmedia_probe_stream media;
    enum {bmp3_id3_none, bmp3_id3v1, bmp3_id3v2_2, bmp3_id3v2_3, bmp3_id3v2_4} id3;
    bmp3_text_string title;
    bmp3_text_string artist;
    bmp3_text_string album;
    bmp3_text_string year;
    bmp3_text_string genre;
    bmp3_text_string start_time;
    bmp3_text_string end_time;
    bmp3_text_string comment;
    char data[1]; /* variable size array, all strings above stored in this array */
} bmp3_probe_stream;


#if 0
bmedia_probe_stream *bmp3_probe_file(bfile_buffer_t buf, off_t file_size, const bmedia_probe_track *track);
void bmp3_prohe_destroy_stream(bmedia_probe_stream *stream);
#endif

extern const bmedia_probe_format_desc bmp3_probe;

#ifdef __cplusplus
}
#endif


#endif /* _BMP3_PROBE_H__ */


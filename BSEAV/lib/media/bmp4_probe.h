/***************************************************************************
 *     Copyright (c) 2007-2011, Broadcom Corporation
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
#ifndef _BMP4_PROBE_H__
#define _BMP4_PROBE_H__

#include "bmedia_probe_impl.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct bmp4_probe_segment {
    uint64_t offset;
    uint64_t size;
} bmp4_probe_segment;

typedef struct bmp4_probe_stream {
    bmedia_probe_stream media;
    bmp4_probe_segment moov;
    bmp4_probe_segment mdat;
    bmp4_probe_segment mvex;
    bmp4_probe_segment rmra;
    bmp4_probe_segment uuid;
    bool compatible;
    bool fragmented;
    char ftyp[5];
} bmp4_probe_stream;

typedef struct bmp4_probe_track {
    bmedia_probe_track media;
    unsigned sample_count; /* number of samples in the stream */
    uint64_t duration; /* track duration in miliseconds */
    size_t protection_scheme_information_size;
    bool encrypted;
    char language[4]; /* NULL terminated ISO-639-2/T language code */
    uint8_t protection_scheme_information[128];
    bmp4_probe_segment sampledescription; /* location of the SAMPLEDESCRIPTION 'stsd' box */
    bmp4_probe_segment mediaheader; /* location of the MEDIAHEADER 'mdhd' box */
} bmp4_probe_track;


extern const bmedia_probe_format_desc bmp4_probe;

#ifdef __cplusplus
}
#endif


#endif /* _BMP4_PROBE_H__ */


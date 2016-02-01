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
 * MPEG-2 TS Parser/Demux library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef __BMPEG2TS_PSI_PROBE_H__
#define __BMPEG2TS_PSI_PROBE_H__


#include "bmedia_probe_impl.h"
#ifdef __cplusplus
extern "C"
{
#endif

typedef enum bmpeg2ts_psi_transport_type {
    bmpeg2ts_psi_transport_unknown,
    bmpeg2ts_psi_transport_dvb_subtitles,
    bmpeg2ts_psi_transport_teletext,
    bmpeg2ts_psi_transport_pmt,
    bmpeg2ts_psi_transport_pat,
    bmpeg2ts_psi_transport_pcr,
    bmpeg2ts_psi_transport_caption_service
} bmpeg2ts_psi_transport_type;


#define BMPEG2TS_PSI_PROBE_ID       0x9000
#define BMPEG2TS192_PSI_PROBE_ID    0x9001

typedef struct bmpeg2ts_psi_dvb_subtitle {
    bool valid;
    uint8_t languageCode[4]; /* 3 ascii codes, add one more for "\0" */
    uint8_t type;
    uint8_t compositionPageId;
    uint8_t ancillaryPageId;
} bmpeg2ts_psi_dvb_subtitle;

typedef struct bmpeg2ts_psi_language {
    bool valid;
    uint8_t type;
    char code[4]; /* 3 ascii codes, add one more for "\0" */
} bmpeg2ts_psi_language;

typedef struct bmpeg2ts_psi_caption_service_one {
    char language[4]; /* 3 ascii codes, add one more for "\0" */
    bool digital_cc;
    union
    {
        bool    line21_field;
        uint8_t caption_service_number;
    } cc;
    bool        easy_reader;
    bool        wide_aspect_ratio;
} bmpeg2ts_psi_caption_service_one;

typedef struct bmpeg2ts_psi_caption_service {
    bool valid;
    unsigned number_of_services;
    bmpeg2ts_psi_caption_service_one services[8];
} bmpeg2ts_psi_caption_service;

typedef struct bmpeg2ts_ac3_bsmod {
    bool valid;
    uint8_t bsmod; /* Bit Stream Mode */
} bmpeg2ts_ac3_bsmod;

/* 
 * pointer bmedia_probe_track should  be converted to bmpeg2ts_psi_probe_track only if following conditions met:
 * 1. bmedia_probe_stream.probe_id == BMPEG2TS192_PSI_PROBE_ID || bmedia_probe_stream.probe_id == BMPEG2TS_PSI_PROBE_ID
 * 2. bmedia_probe_track.program != BMEDIA_PROBE_INVALID_PROGRAM
 */
typedef struct bmpeg2ts_psi_probe_track {
    bmedia_probe_track media;
    bmpeg2ts_psi_transport_type transport_type;
    size_t descriptor_size; /* size of the descriptor data, 0 if unavailable */
    const void *descriptor;
    bmpeg2ts_psi_dvb_subtitle dvb_subtitle;
    bmpeg2ts_psi_language language;
    bmpeg2ts_psi_caption_service caption_service;
    bmpeg2ts_ac3_bsmod ac3_bsmod;
    size_t parsed_payload; /* amount of payload (in bytes) encountered when parsing stream */
} bmpeg2ts_psi_probe_track;

extern const bmedia_probe_format_desc bmpeg2ts_psi_probe;
extern const bmedia_probe_format_desc bmpeg2ts192_psi_probe;

#ifdef __cplusplus
}
#endif


#endif  /* __BMPEG2TS_PSI_PROBE_H__ */


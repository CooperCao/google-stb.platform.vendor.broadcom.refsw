/***************************************************************************
 *     Copyright (c) 2007 Broadcom Corporation
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
 * BMedia library, AC3 elementary stream probe
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BAC3_PROBE_H__
#define _BAC3_PROBE_H__

#include "bmedia_probe_es.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define B_AC3_SYNC      0x0B77
#define B_AC3_SYNC_LEN  2
	
extern const bmedia_probe_es_desc bac3_probe;
typedef struct bmedia_probe_ac3_audio {
    bool valid;
    baudio_format codec; /* type of audio codec in the extended information */
} bmedia_probe_ac3_audio;

size_t b_ac3_probe_parse_header(batom_cursor *cursor, bmedia_probe_audio *info, unsigned *pnblocks);

#ifdef __cplusplus
}
#endif


#endif /* _BMEDIA_PROBE_ES_H__ */


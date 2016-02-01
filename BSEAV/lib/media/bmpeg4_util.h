/***************************************************************************
 *     Copyright (c) 2007-2010, Broadcom Corporation
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
 * MPEG4 stream parsing 
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BMPEG4_UTIL_H__
#define _BMPEG4_UTIL_H__

#include "bioatom.h"
#include "bmedia_util.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* 7.2.6.5 Semantics, Table 5. objectTypeIndication Values ISO/IEC 14496-1 MPEG4 Part-1, page 35 */
#define BMPEG4_Audio_ISO_IEC_14496_3	0x40
#define BMPEG4_Audio_ISO_IEC_13818_7	0x67
#define BMPEG4_Video_ISO_IEC_14496_2	0x20
#define BMPEG4_Audio_ISO_IEC_11172_3    0x6B

typedef struct bmpeg4_es_descriptor {
    uint16_t ES_ID;
    uint8_t objectTypeIndication;
    uint8_t streamType;
    union {
        struct {
            uint8_t audioObjectType;
            uint8_t samplingFrequencyIndex;
            uint8_t channelConfiguration;
            size_t aac_info_size;
            uint8_t aac_info[3*1024];
        } iso_14496_3;
        struct {
            size_t header_size;
            uint8_t header[128];    /* changed from 64 to 128 to accommodate PR35214: LIB-Coverity (CID 3048): OVERRUN_STATIC */
        } iso_14496_2;
    } decoder;
} bmpeg4_es_descriptor;


bool
bmpeg4_parse_es_descriptor(batom_cursor *cursor, bmpeg4_es_descriptor *descriptor);

bool
bmedia_info_probe_aac_info(batom_cursor *cursor, bmedia_info_aac *aac);
#ifdef __cplusplus
}
#endif


#endif /* _BMPEG4_UTIL_H__ */


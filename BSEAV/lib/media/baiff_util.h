/***************************************************************************
 *
 *     Copyright (c) 2009-2012, Broadcom Corporation
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
 * BMedia library, AIFF stream 
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BAIFF_UTIL_H__
#define _BAIFF_UTIL_H__

#include "bioatom.h"
#include "bmedia_pes.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct baiff_comm {
    uint16_t numChannels;
    uint32_t numSampleFrames;
    uint16_t sampleSize;
    unsigned sampleRate;
} baiff_comm;

bool baiff_read_comm(batom_cursor *cursor, baiff_comm *comm);

#define BAIFF_FORM  BMEDIA_FOURCC('F','O','R','M')
#define BAIFF_COMM  BMEDIA_FOURCC('C','O','M','M')

#define BMEDIA_FOURCC_AIFF(aiff) (BMEDIA_FOURCC('A','I','F','F')==aiff || BMEDIA_FOURCC('A','I','F','C'))

#ifdef __cplusplus
}
#endif


#endif /* _BAIFF_UTIL_H__ */


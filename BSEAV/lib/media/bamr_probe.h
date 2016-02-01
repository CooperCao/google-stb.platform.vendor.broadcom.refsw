/***************************************************************************
 *     Copyright (c) 2013 Broadcom Corporation
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
#ifndef _BAMR_PROBE_H__
#define _BAMR_PROBE_H__

#include "bmedia_probe_impl.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct bamr_probe_stream {
    bmedia_probe_stream media;
} bamr_probe_stream;

typedef struct bamr_probe_track {
    bmedia_probe_track media;
} bamr_probe_track;


extern const bmedia_probe_format_desc bamr_probe;

#ifdef __cplusplus
}
#endif


#endif /* _BAMR_PROBE_H__ */


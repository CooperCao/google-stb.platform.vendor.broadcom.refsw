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
 * AMR parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/

#ifndef _BAMR_UTIL_H__
#define _BAMR_UTIL_H__

#include "bioatom.h"
#include "bmedia_util.h"
#include "bmedia_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* RFC4867 - section 5. AMR and AMR-WB Storage Format */
#define BAMR_HEADER_TAG BMEDIA_FOURCC('#','!','A','M')

#define BAMR_HEADER_LENGTH      (2/*#!*/+3/*AMR*/+3/*-WB*/+1/*\n*/)

/* this function would return either baudio_format_amr_nb or baudio_format_amr_wb on success, and baudio_format_unknown on error */
baudio_format bamr_parse_header(batom_cursor *cursor);

/* this function returns size of the frame, or -1 on error, it needs to know AMR codec type */
int bamr_parse_frame_header(batom_cursor *cursor, baudio_format codec);

#ifdef __cplusplus
}
#endif

#endif /* _BAMR_UTIL_H__ */


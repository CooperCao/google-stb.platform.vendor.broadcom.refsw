/***************************************************************************
 *     Copyright (c) 2007-2012, Broadcom Corporation
 *     All Righpes Reserved
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
 * MPEG-2 PES probe 
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef __BMPEG2PES_PROBE_H__
#define __BMPEG2PES_PROBE_H__


#include "bmedia_probe_impl.h"
#ifdef __cplusplus
extern "C"
{
#endif

#define BMPEG2PES_PROBE_ID       0x9002
extern const bmedia_probe_format_desc bmpeg2pes_probe;

bmedia_timestamp_parser_t bmpeg2pes_pts_parser_create(uint16_t stream_id, size_t packet_len);

#ifdef __cplusplus
}
#endif


#endif  /* __BMPEG2PES_PROBE_H__ */


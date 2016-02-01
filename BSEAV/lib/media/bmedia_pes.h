/***************************************************************************
 *     Copyright (c) 2007-2009, Broadcom Corporation
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
 * BMedia library, PES producer
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BMEDIA_PES_H__
#define _BMEDIA_PES_H__

#include "balloc.h"
#include "bioatom.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define B_MEDIA_PACKET_NO_HEADER	(0x7F)
#define B_MEDIA_PACKET_EOS			(0xFE)
#define B_MEDIA_PACKET_FLAG_EOS		(0x80)
#define BMEDIA_PACKET_HEADER_INIT(hdr) { (hdr)->pts_valid=false;(hdr)->key_frame=false;(hdr)->meta_header=false;(hdr)->header_type=B_MEDIA_PACKET_NO_HEADER;(hdr)->header_len=0;(hdr)->header_off=0;} while(0)

typedef struct bmedia_packet_header {
	uint32_t pts; /* PTS of the packet, in 45KHz */
	uint8_t header_type; /* static header that added before payload */
	uint8_t header_len; /* length of the variable length header, that stored past bmedia_packet_header */
	uint8_t header_off; /* offset into the static header, where variable header shall be placed */
	bool pts_valid; /* validator for PTS value */
	bool key_frame; /* set to true if this is a key frame */
    bool meta_header; /* set to true if this packet is required for decoding of following frames */
} bmedia_packet_header;

typedef struct bmedia_pes *bmedia_pes_t;
typedef struct bmedia_pes_stream_cfg {
	size_t nvecs;
	const batom_vec *const*vecs;
    bool single_pts;
} bmedia_pes_stream_cfg;

typedef struct bmedia_pes_cfg {
	size_t pes_hdr_size;
	size_t eos_len;
	const void *eos_data;
} bmedia_pes_cfg;

typedef struct bmedia_pes_status {
    bool last_pts_valid;
    uint32_t last_pts;
} bmedia_pes_status;

void bmedia_pes_default_cfg(bmedia_pes_cfg *cfg);
void bmedia_pes_default_stream_cfg(bmedia_pes_stream_cfg *cfg);
bmedia_pes_t bmedia_pes_create(batom_factory_t factory, balloc_iface_t alloc, const bmedia_pes_cfg *cfg);
void bmedia_pes_destroy(bmedia_pes_t  stream);
bool bmedia_pes_feed(bmedia_pes_t stream, batom_pipe_t pipe);
void bmedia_pes_start(bmedia_pes_t stream, batom_pipe_t pipe, const bmedia_pes_stream_cfg *cfg, uint8_t pes_id);
void bmedia_pes_get_stream_cfg(bmedia_pes_t stream, bmedia_pes_stream_cfg *cfg);
void bmedia_pes_set_stream_cfg(bmedia_pes_t stream, const bmedia_pes_stream_cfg *cfg);
void bmedia_pes_stop(bmedia_pes_t stream);
void bmedia_pes_reset(bmedia_pes_t stream);
void bmedia_pes_get_status(bmedia_pes_t stream, bmedia_pes_status *status);

extern const batom_user bmedia_atom[1];


#ifdef __cplusplus
}
#endif


#endif /* _BMEDIA_PES_H__ */


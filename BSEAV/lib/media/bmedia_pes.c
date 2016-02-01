/***************************************************************************
 *     Copyright (c) 2006-2012, Broadcom Corporation
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
#include "bstd.h"
#include "biobits.h"
#include "bkni.h"
#include "bmedia_pes.h"
#include "bmedia_util.h"

BDBG_MODULE(bmedia_pes);

BDBG_OBJECT_ID(bmedia_pes_t);
#define BDBG_MSG_TRACE(x)   /* BDBG_MSG(x) */


struct bmedia_pes {
	BDBG_OBJECT(bmedia_pes_t)
	balloc_iface_t alloc;
	bmedia_pes_cfg cfg;
	bmedia_pes_stream_cfg stream_cfg;
    bmedia_pes_info pes_info;
	batom_pipe_t pipe;
	void *pes_header; /* PES header */
	size_t hdr_len; /* size of the PES header */
	batom_accum_t acc;
    bool pts_sent; /* true if already sent pts */
    bool last_pts_valid;
    uint32_t last_pts;
};

#define B_MEDIA_MAX_PES_HDR_SIZE	64

void
bmedia_pes_default_cfg(bmedia_pes_cfg *cfg)
{
	BDBG_ASSERT(cfg);
	cfg->pes_hdr_size = B_MEDIA_MAX_PES_HDR_SIZE;
	cfg->eos_len = 0;
	cfg->eos_data = NULL;
	return;
}

void 
bmedia_pes_default_stream_cfg(bmedia_pes_stream_cfg *cfg)
{
	BDBG_ASSERT(cfg);
	cfg->nvecs = 0;
	cfg->vecs = NULL;
    cfg->single_pts = false;
	return;
}

bmedia_pes_t 
bmedia_pes_create(batom_factory_t factory, balloc_iface_t alloc, const bmedia_pes_cfg *cfg)
{
	bmedia_pes_t  stream;

	BDBG_ASSERT(factory);
	BDBG_ASSERT(alloc);
	BDBG_ASSERT(cfg);

	if(!(cfg->eos_len==0 || cfg->eos_data)) {
		goto err_cfg;
	}

	stream = BKNI_Malloc(sizeof(*stream));
	if (!stream) {
		BDBG_ERR(("bmedia_pes_create: can't allocated %u bytes", (unsigned)sizeof(*stream)));
		goto err_alloc;
	}
	BDBG_OBJECT_INIT(stream, bmedia_pes_t);
	stream->cfg = *cfg;
	stream->alloc = alloc;

    BMEDIA_PES_INFO_INIT(&stream->pes_info, 0);
	stream->pipe = NULL;
	stream->acc = batom_accum_create(factory);
	if (!stream->acc) {
		goto err_acc;
	}
	return stream;
err_acc:
	BKNI_Free(stream);
err_alloc:
err_cfg:
	return NULL;
}

void
bmedia_pes_destroy(bmedia_pes_t  stream)
{
	BDBG_OBJECT_ASSERT(stream, bmedia_pes_t);
	batom_accum_destroy(stream->acc);
	BDBG_OBJECT_DESTROY(stream, bmedia_pes_t);
	BKNI_Free(stream);
	return;
}

void
bmedia_pes_start(bmedia_pes_t  stream, batom_pipe_t pipe, const bmedia_pes_stream_cfg *cfg, uint8_t pes_id)
{
	BDBG_OBJECT_ASSERT(stream, bmedia_pes_t);
	BDBG_ASSERT((cfg->nvecs==0 || cfg->vecs));
	stream->stream_cfg = *cfg;
	stream->pes_info.stream_id = pes_id;
	stream->pipe = pipe;
	stream->pes_header = NULL;
    stream->pts_sent = false;
    stream->last_pts_valid = false;
    stream->last_pts = B_MEDIA_INVALID_PTS;
	return;
}

void
bmedia_pes_stop(bmedia_pes_t  stream)
{
	BDBG_OBJECT_ASSERT(stream, bmedia_pes_t);
	stream->pipe = NULL;
	if(stream->pes_header) {
		stream->alloc->bmem_free(stream->alloc, stream->pes_header);
	}
	stream->pes_header = NULL;
	batom_accum_clear(stream->acc);
	return;
}

static void
b_media_pes_atom_free(batom_t atom, void *user)
{
	bmedia_pes_t stream = *(bmedia_pes_t *)user;
	const batom_vec *pes_vec;
	BDBG_OBJECT_ASSERT(stream, bmedia_pes_t);
	BDBG_MSG_TRACE(("b_media_pes_atom_free: %#lx %#lx", (unsigned long)atom, (unsigned long)stream));
	pes_vec = batom_get_vec(atom, 0);
	BDBG_ASSERT(pes_vec);
	stream->alloc->bmem_free(stream->alloc, pes_vec->base);
	return;
}

static const batom_user b_media_pes_atom = {
	b_media_pes_atom_free,
	sizeof(bmedia_pes_t)
};


static bool
b_media_pes_send_step(bmedia_pes_t stream)
{
	batom_accum_t acc;
	balloc_iface_t alloc;

	alloc = stream->alloc;
	acc = stream->acc;


	do {
		batom_cursor cursor;
		batom_cursor start;
		batom_t part,atom;
		size_t pkt_size;
		bmedia_pes_t tmp_stream=stream;

		batom_cursor_from_accum(&cursor, acc);
		BATOM_CLONE(&start, &cursor);
		pkt_size = batom_cursor_skip(&cursor, B_MEDIA_MAX_PES_SIZE);
		part = batom_accum_extract(acc, &start, &cursor, NULL, NULL);
		if(!part) {
			return false;
		}
		if(stream->pes_header==NULL) {
			stream->pes_header = alloc->bmem_alloc(alloc, stream->cfg.pes_hdr_size);
			if(stream->pes_header) {
                BMEDIA_PES_UNSET_PTS(&stream->pes_info);
                BMEDIA_PES_UNSET_DTS(&stream->pes_info);

				stream->hdr_len = bmedia_pes_header_init(stream->pes_header, pkt_size, &stream->pes_info);
				BDBG_ASSERT(stream->hdr_len <= stream->cfg.pes_hdr_size);
			} else {
				batom_release(part);
				return false;
			}
		} 
		atom = batom_from_range_and_atom(stream->pes_header, stream->hdr_len, part, &b_media_pes_atom, &tmp_stream);
		batom_release(part); /* recycle temporary atom */
		if(!atom) {
			return false;
		}
		batom_accum_trim(acc, &cursor);
		batom_pipe_push(stream->pipe, atom);
		stream->pes_header = NULL;
	} while(batom_accum_len(acc)>0);
	return true;
}

static bool
b_media_pes_send(bmedia_pes_t stream, batom_t data, size_t data_len, size_t hdr_len, void *header)
{
	batom_t atom;
	batom_accum_t acc;
	bmedia_pes_t tmp_stream=stream;

	BDBG_MSG_TRACE(("b_media_pes_send<: %#lx %#lx %u", (unsigned long)stream, (unsigned long)data, data_len));
	if(data_len<=B_MEDIA_MAX_PES_SIZE) { /* try a fast path first */
		atom = batom_from_range_and_atom(header, hdr_len, data, &b_media_pes_atom, &tmp_stream);
		if(atom) {
			batom_release(data);
			batom_pipe_push(stream->pipe, atom);
			BDBG_MSG_TRACE(("b_media_pes_send>: %#lx single PES", (unsigned long)stream));
			return true;
		} 
	} 
	acc = stream->acc;
	BDBG_ASSERT(acc);
	BDBG_ASSERT(batom_accum_len(acc)==0);
	batom_accum_add_atom(acc, data);
	batom_release(data);
	stream->pes_header = header;
	stream->hdr_len = hdr_len;
	BDBG_MSG_TRACE(("b_media_pes_send>: %#lx split PES", (unsigned long)stream));
	return b_media_pes_send_step(stream);
}

bool 
bmedia_pes_feed(bmedia_pes_t stream, batom_pipe_t pipe)
{
	bool want_continue;

	BDBG_MSG_TRACE(("bmedia_pes_feed>: %#lx", (unsigned long)stream));
	BDBG_OBJECT_ASSERT(stream, bmedia_pes_t);
	BDBG_ASSERT(stream->pipe);
	if(batom_accum_len(stream->acc)>0) {
		want_continue = b_media_pes_send_step(stream);
		if (!want_continue) {
			BDBG_MSG_TRACE(("bmedia_pes_feed<: %#lx PAUSE", (unsigned long)stream));
			return false;
		}
	}
	do {
		const bmedia_packet_header *hdr;
		size_t pkt_size;
		size_t hdr_len;
		void *pes_header;
		const batom_vec *vec;
		unsigned header_type;
		size_t vec_len;
		size_t header_len;
        size_t max_pes_size;
		batom_t atom=batom_pipe_peek(pipe);
		
		if (!atom) {
			BDBG_MSG_TRACE(("bmedia_pes_feed<: %#lx EOD", (unsigned long)stream));
			break;
		}
		pes_header = stream->alloc->bmem_alloc(stream->alloc, stream->cfg.pes_hdr_size);
		if (!pes_header) {
			BDBG_MSG_TRACE(("bmedia_pes_feed<: %#lx !pes_header", (unsigned long)stream));
			return false;
		}
		batom_pipe_drop(pipe);
		hdr = batom_userdata(atom);
		BDBG_ASSERT(hdr);
		BDBG_MSG(("bmedia_pes_feed: %#lx atom %#lx type:%02x", (unsigned long)stream, (unsigned long)atom, (unsigned)hdr->header_type));
		header_type = hdr->header_type&(~B_MEDIA_PACKET_FLAG_EOS);
		pkt_size = batom_len(atom);
		vec_len = 0;
		vec = NULL;
		/* test for the static header */
		if(header_type!=B_MEDIA_PACKET_NO_HEADER) {
			BDBG_ASSERT(header_type <= stream->stream_cfg.nvecs);
			BDBG_ASSERT(stream->stream_cfg.vecs);
			vec = stream->stream_cfg.vecs[header_type];
			BDBG_ASSERT(vec);
			vec_len = vec->len;
		}
		/* test for the dynamic header */
		header_len = hdr->header_len;
		if(header_len>0) {
			BDBG_ASSERT(hdr->header_off<=vec_len);
			if(header_len + hdr->header_off > vec_len ) {
				vec_len = header_len + hdr->header_off;
			}
		}
		/* check for the EOS flag */
		header_type = hdr->header_type;
        max_pes_size = B_MEDIA_MAX_PES_SIZE+vec_len;
		if(pkt_size==0 && (header_type&B_MEDIA_PACKET_FLAG_EOS)==0) {
            BDBG_MSG(("bmedia_pes_feed: %#lx discarding empty packet %#lx", (unsigned long)stream, (unsigned long)atom));
            batom_release(atom);
            stream->alloc->bmem_free(stream->alloc, pes_header);
            want_continue = true;
            continue;
        }
		pkt_size += vec_len;
		if(header_type&B_MEDIA_PACKET_FLAG_EOS) {
			pkt_size += stream->cfg.eos_len;			
			BMEDIA_PES_UNSET_DTS(&stream->pes_info);
            BMEDIA_PES_UNSET_PTS(&stream->pes_info);
            if(hdr->pts_valid) {
                if(hdr->pts==B_MEDIA_MARKER_PTS) {
                    BMEDIA_PES_SET_PTS(&stream->pes_info, 0);
                    BMEDIA_PES_SET_DTS(&stream->pes_info, B_MEDIA_MARKER_PTS);
                } else {
                    BMEDIA_PES_SET_PTS(&stream->pes_info, hdr->pts);
                }
            } 
			hdr_len = bmedia_pes_header_init(pes_header, 
							 pkt_size<max_pes_size?pkt_size:max_pes_size, /* limit size of the PES packet */
                             &stream->pes_info
							 );
			
		} else {
			if(hdr->pts_valid) {
                if(hdr->pts != B_MEDIA_MARKER_PTS) {
                    stream->last_pts_valid = true;
                    stream->last_pts = hdr->pts;
                }
                if(stream->stream_cfg.single_pts && stream->pts_sent) {
				    BMEDIA_PES_UNSET_PTS(&stream->pes_info);
                } else {
                    stream->pts_sent = true;
				    BMEDIA_PES_SET_PTS(&stream->pes_info, hdr->pts);
                }
			} else {
				BMEDIA_PES_UNSET_PTS(&stream->pes_info);
			}
			BMEDIA_PES_UNSET_DTS(&stream->pes_info);
			hdr_len = bmedia_pes_header_init(pes_header, 
							 pkt_size<max_pes_size?pkt_size:max_pes_size, /* limit size of the PES packet */
                             &stream->pes_info
							 );
		}
		/* add static header */
		if(vec) {
			BDBG_ASSERT(hdr_len+vec_len <= stream->cfg.pes_hdr_size);
			BKNI_Memcpy((uint8_t *)pes_header + hdr_len, vec->base, vec->len);
		}
		/* add dynamic header */
		if(header_len) {
			BKNI_Memcpy((uint8_t *)pes_header + hdr_len + hdr->header_off, (uint8_t*)hdr+sizeof(*hdr), header_len);
		}
		hdr_len += vec_len;
		BDBG_ASSERT(hdr_len <= stream->cfg.pes_hdr_size);
		if(header_type&B_MEDIA_PACKET_FLAG_EOS) {
			batom_accum_t acc = stream->acc;
			batom_t pes_atom;

			BDBG_ASSERT(acc);
			BDBG_ASSERT(batom_accum_len(acc)==0);

			batom_accum_add_atom(acc, atom);
			if(stream->cfg.eos_len) {
				batom_accum_add_range(acc, stream->cfg.eos_data, stream->cfg.eos_len);
			}
			pes_atom = batom_from_accum(acc, NULL, NULL);
			if(!pes_atom) {
				BDBG_WRN(("bmedia_pes_feed: %#lx can't create atom", (unsigned long)stream));
				stream->alloc->bmem_free(stream->alloc, pes_header);
				return false;
			}
			batom_release(atom);
			atom=pes_atom;
		}
		want_continue = b_media_pes_send(stream, atom, pkt_size, hdr_len, pes_header);
	} while(want_continue);

	return true;
}

void
bmedia_pes_reset(bmedia_pes_t stream)
{
	BDBG_OBJECT_ASSERT(stream, bmedia_pes_t);
    stream->pts_sent = false;
	batom_accum_clear(stream->acc);
	return;
}

const batom_user bmedia_atom[] = {{
	NULL,
	sizeof(bmedia_packet_header)
}};

void 
bmedia_pes_get_stream_cfg(bmedia_pes_t stream, bmedia_pes_stream_cfg *cfg)
{
	BDBG_OBJECT_ASSERT(stream, bmedia_pes_t);
    BDBG_ASSERT(cfg);
    *cfg = stream->stream_cfg;
    return;
}

void 
bmedia_pes_set_stream_cfg(bmedia_pes_t stream, const bmedia_pes_stream_cfg *cfg)
{
	BDBG_OBJECT_ASSERT(stream, bmedia_pes_t);
    BDBG_ASSERT(cfg);
    stream->stream_cfg = *cfg;
    return;
}

void 
bmedia_pes_get_status(bmedia_pes_t stream, bmedia_pes_status *status)
{
	BDBG_OBJECT_ASSERT(stream, bmedia_pes_t);
    BDBG_ASSERT(status);
    status->last_pts_valid = stream->last_pts_valid;
    status->last_pts = stream->last_pts;
    return;
}


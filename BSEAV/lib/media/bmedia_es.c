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
 * w
 * BMedia library, ES producer
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "biobits.h"
#include "bkni.h"
#include "bmedia_es.h"

BDBG_MODULE(bmedia_es);

BDBG_OBJECT_ID(bmedia_es_t);


struct bmedia_es {
    BDBG_OBJECT(bmedia_es_t)
    balloc_iface_t alloc;
    bmedia_pes_cfg cfg;
    batom_pipe_t pipe;
    bmedia_pes_stream_cfg stream_cfg;
    batom_accum_t acc;
    bool raw;
};


bmedia_es_t
bmedia_es_create(batom_factory_t factory, balloc_iface_t alloc, const bmedia_pes_cfg *cfg)
{
    bmedia_es_t  stream;

    BDBG_ASSERT(factory);
    BDBG_ASSERT(alloc);

    BSTD_UNUSED(factory);

    stream = BKNI_Malloc(sizeof(*stream));
    if (!stream) {
        BDBG_ERR(("bmedia_es_create: can't allocated %u bytes", (unsigned)sizeof(*stream)));
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(stream, bmedia_es_t);
    stream->cfg = *cfg;
    stream->alloc = alloc;
    stream->pipe = NULL;
    stream->raw = false;
    stream->acc = batom_accum_create(factory);
    if(!stream->acc) {
        goto err_acc;
    }
    return stream;
err_acc:
    BKNI_Free(stream);
err_alloc:
    return NULL;
}

void
bmedia_es_destroy(bmedia_es_t  stream)
{
    BDBG_OBJECT_ASSERT(stream, bmedia_es_t);
    batom_accum_destroy(stream->acc);
    BDBG_OBJECT_DESTROY(stream, bmedia_es_t);
    BKNI_Free(stream);
    return;
}

void
bmedia_es_start(bmedia_es_t  stream, batom_pipe_t pipe, const bmedia_pes_stream_cfg *stream_cfg, bool raw)
{
    BDBG_OBJECT_ASSERT(stream, bmedia_es_t);
    BDBG_ASSERT((stream_cfg->nvecs==0 || stream_cfg->vecs));
    stream->pipe = pipe;
    stream->raw = raw;
    stream->stream_cfg = *stream_cfg;
    return;
}

void
bmedia_es_stop(bmedia_es_t  stream)
{
    BDBG_OBJECT_ASSERT(stream, bmedia_es_t);
    stream->pipe = NULL;
    return;
}

static void
b_media_es_atom_free(batom_t atom, void *user)
{
    bmedia_es_t stream = *(bmedia_es_t *)user;
    const batom_vec *pes_vec;
    BDBG_OBJECT_ASSERT(stream, bmedia_es_t);
    BDBG_MSG(("b_media_es_atom_free: %#lx %#lx", (unsigned long)atom, (unsigned long)stream));
    pes_vec = batom_get_vec(atom, 0);
    BDBG_ASSERT(pes_vec);
    stream->alloc->bmem_free(stream->alloc, pes_vec->base);
    return;
}

static const batom_user b_media_es_atom = {
    b_media_es_atom_free,
    sizeof(bmedia_es_t)
};

bool
bmedia_es_feed(bmedia_es_t stream, batom_pipe_t pipe)
{
    BDBG_MSG(("bmedia_es_feed>: %#lx", (unsigned long)stream));
    BDBG_OBJECT_ASSERT(stream, bmedia_es_t);
    BDBG_ASSERT(stream->pipe);
    for(;;) {
        batom_t atom=batom_pipe_peek(pipe);
        batom_t es_atom;

        if (!atom) {
            BDBG_MSG(("bmedia_es_feed<: %#lx EOD", (unsigned long)stream));
            break;
        }
        es_atom = atom;
        if(!stream->raw) {
            uint8_t header_type;
            const bmedia_packet_header *hdr = batom_userdata(atom);

            BDBG_ASSERT(hdr);
            header_type = hdr->header_type&(~B_MEDIA_PACKET_FLAG_EOS);
            if(header_type!=B_MEDIA_PACKET_NO_HEADER || hdr->header_len) {
                const batom_vec *vec=NULL;
                uint8_t *es_header;
                bmedia_es_t tmp_stream=stream;
                unsigned vec_len = 0;
                size_t header_len;

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
                if (vec) {
                    BDBG_ASSERT(vec->len <= stream->cfg.pes_hdr_size);
                }

                es_header = stream->alloc->bmem_alloc(stream->alloc, stream->cfg.pes_hdr_size);
                if (!es_header) {
                    BDBG_MSG(("bmedia_es_feed<: %#lx !es_header", (unsigned long)stream));
                    return false;
                }
                /* add static header */
                if(vec) {
                    BKNI_Memcpy(es_header, vec->base, vec->len);
                }
                /* add dynamic header */
                if(header_len) {
                    BKNI_Memcpy((uint8_t *)es_header + hdr->header_off, (uint8_t*)hdr+sizeof(*hdr), header_len);
                }
                if(!(hdr->header_type&B_MEDIA_PACKET_FLAG_EOS)) {
                    es_atom = batom_from_range_and_atom(es_header, vec_len, atom, &b_media_es_atom, &tmp_stream);
                } else {
                    BDBG_ASSERT(batom_accum_len(stream->acc)==0);
                    batom_accum_add_range(stream->acc, es_header, vec_len);
                    batom_accum_add_atom(stream->acc, atom);
                    if(stream->cfg.eos_len) {
                        batom_accum_add_range(stream->acc, stream->cfg.eos_data, stream->cfg.eos_len);
                    }
                    es_atom = batom_from_accum(stream->acc, &b_media_es_atom, &tmp_stream);
                }
                if(!es_atom) {
                    stream->alloc->bmem_free(stream->alloc, es_header);
                }
            } else if(hdr->header_type&B_MEDIA_PACKET_FLAG_EOS) {
                BDBG_ASSERT(batom_accum_len(stream->acc)==0);
                batom_accum_add_atom(stream->acc, atom);
                if(stream->cfg.eos_len) {
                    batom_accum_add_range(stream->acc, stream->cfg.eos_data, stream->cfg.eos_len);
                }
                es_atom = batom_from_accum(stream->acc, NULL, NULL);
            }
            if(!es_atom) {
                return false;
            }
        }
        batom_pipe_drop(pipe);
        if(es_atom!=atom) {
            batom_release(atom);
        }
        batom_pipe_push(stream->pipe, es_atom);
    }
    return true;
}



/***************************************************************************
 *     Copyright (c) 2007-2013, Broadcom Corporation
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
 * Helper module to parse ES streams inside demux
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bmedia_probe_demux.h"
#include "bkni.h"

BDBG_MODULE(bmedia_probe_demux);

typedef struct bmedia_probe_demux_stream {
	BHASH_ENTRY(bmedia_probe_demux_stream) hash_link;
	unsigned track_id;
	bmedia_probe_es_t probe;
	bool	done;
} bmedia_probe_demux_stream;

#define B_MEDIA_PROBE_KEY_COMPARE(id,substream) ((id)==(substream)->track_id)
#define B_MEDIA_PROBE_HASH(id) ((id)^((id)>>4))

BHASH_GENERATE(b_media_probe_hash, unsigned, bmedia_probe_demux_stream, hash_link, B_MEDIA_PROBE_HASH, B_MEDIA_PROBE_KEY_COMPARE)

void 
bmedia_probe_demux_init(bmedia_probe_demux *demux)
{
	BHASH_INIT(b_media_probe_hash, &demux->hash);
	demux->tracks = 0;
	demux->completed = 0;
	return;
}

void
bmedia_probe_demux_shutdown(bmedia_probe_demux *demux)
{
    bmedia_probe_demux_stream *substream;
    BHASH_FOREACH_BEGIN(b_media_probe_hash,&demux->hash,bmedia_probe_demux_stream,substream,hash_link)
        if(substream->probe) {
            BDBG_MSG(("bmedia_probe_demux_shutdown: %p destroing es probe %p for id:%#x", (void *)demux, (void *)substream->probe, substream->track_id));
            bmedia_probe_es_destroy(substream->probe);
        }
    BHASH_REMOVE(b_media_probe_hash, &demux->hash, substream->track_id);
    BKNI_Free(substream);
    BHASH_FOREACH_END()
    return;
}

bmedia_probe_track *
bmedia_probe_demux_data(bmedia_probe_demux *demux, batom_factory_t factory, unsigned track_id, bmedia_track_type track_type, batom_t data)
{
    bmedia_probe_demux_stream *substream;
    bmedia_probe_track *track=NULL;

    substream = BHASH_FIND(b_media_probe_hash, &demux->hash, track_id);
    if(!substream) {
        substream = BKNI_Malloc(sizeof(*substream));
        if(!substream) {
            BDBG_ERR(("bmedia_probe_demux_data: %p can't allocate %u bytes", (void *)demux, (unsigned)sizeof(*substream)));
            goto done;
        }
        substream->track_id = track_id;
        substream->done = false;
        substream->probe = bmedia_probe_es_create(factory);
        BDBG_MSG(("bmedia_probe_demux_data: %p creating es probe %p for id:%#x", (void *)demux, (void *)substream->probe, track_id));
        if(!substream->probe) {
            BKNI_Free(substream);
            goto done;
        }
        if(track_type!=bmedia_track_type_other) {
            bmedia_probe_es_filter_type(substream->probe, track_type);
        }
        BHASH_ADD(b_media_probe_hash,&demux->hash, track_id, substream);
        demux->tracks++;
    }
    BDBG_ASSERT(substream); BDBG_ASSERT(substream->probe);
    if(!substream->done) {
        unsigned nactive;
        track = bmedia_probe_es_feed(substream->probe, data, &nactive);
        if(nactive==0 && !track) {
            track = BKNI_Malloc(sizeof(*track));
            if(!track) {
                BDBG_ERR(("bmedia_probe_demux_data: %p can't allocate %u bytes", (void *)demux, (unsigned)sizeof(*track)));
                goto done;
            }
            bmedia_probe_track_init(track);
            track->type = bmedia_track_type_other;
        }
        if(track) {
            track->number = track_id;
            substream->done = true;
            demux->completed++;
            BDBG_MSG(("bmedia_probe_demux_data: %p finished parsing for id:%#x track %p (%u:%u)", (void *)demux, track_id, (void *)track, demux->completed, demux->tracks));
        }
    } else {
        batom_release(data);
    }

done:
    return track;
}

void
bmedia_probe_demux_add_unknown(const bmedia_probe_demux *demux, bmedia_probe_stream *stream, bmedia_probe_track * (*allocate_track)(void))
{
	bmedia_probe_demux_stream *substream;
	bmedia_probe_track *track;

	BHASH_FOREACH_BEGIN(b_media_probe_hash,&demux->hash,bmedia_probe_demux_stream,substream,hash_link) 
		if(substream->done) {
			continue;
		}
		track = bmedia_probe_es_last(substream->probe);
		if(track==NULL) {
            if(allocate_track) {
                track = allocate_track();
            }  else {
			    track = BKNI_Malloc(sizeof(*track));
            }
			if(!track) {
				BDBG_ERR(("bmedia_probe_demux_add_unknown: %p can't allocate %u bytes", (void *)demux, (unsigned)sizeof(*track)));
				break;
			}
			bmedia_probe_track_init(track);
			track->type = bmedia_track_type_other;
		}
		track->number = substream->track_id;
		bmedia_probe_add_track(stream, track);
	BHASH_FOREACH_END()
	return;
}


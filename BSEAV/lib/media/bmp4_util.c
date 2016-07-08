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
 * MP4 container parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bmp4_util.h"
#include "bkni.h"
#include "biobits.h"


BDBG_MODULE(bmp4_util);

#define BDBG_MSG_TRACE(x)   BDBG_MSG(x)


/* ISO/IEC 14496-12:2005 MPEG-4 Part 12 - ISO Base Media File Format */
size_t
bmp4_parse_box(batom_cursor *cursor, bmp4_box *box)
{
	uint32_t size;
	BDBG_ASSERT(cursor);
	BDBG_ASSERT(box);

	/* page 3 */
	size = batom_cursor_uint32_be(cursor);
	box->type = batom_cursor_uint32_be(cursor);
	if(!BATOM_IS_EOF(cursor)) {
		if(size!=1) {
			box->size = size;
            BDBG_MSG_TRACE(("bmp4_parse_box: " B_MP4_TYPE_FORMAT ":%u", B_MP4_TYPE_ARG(box->type), (unsigned)box->size));
			return sizeof(uint32_t)+sizeof(uint32_t); 
		} 
		box->size = batom_cursor_uint64_be(cursor);
		if(!BATOM_IS_EOF(cursor)) {
            BDBG_MSG_TRACE(("bmp4_parse_box: " B_MP4_TYPE_FORMAT ":%u", B_MP4_TYPE_ARG(box->type), (unsigned)box->size));
			return sizeof(uint32_t)+sizeof(uint32_t)+sizeof(uint64_t); 
		}
	}
	return 0;
}

static size_t
b_mp4_find_box(batom_cursor *cursor, uint32_t type, bmp4_sample_codecprivate *codecprivate)
{
    for(;;) {
        bmp4_box box;
        size_t box_hdr_size;
        unsigned offset = batom_cursor_pos(cursor);
        box_hdr_size = bmp4_parse_box(cursor, &box);

        if(box_hdr_size==0) {
            break;
        }
        if(box.type == type) {
            BDBG_MSG(("b_mp4_find_box: find " B_MP4_TYPE_FORMAT ":%u", B_MP4_TYPE_ARG(box.type), (unsigned)box.size));
            if(codecprivate) {
                codecprivate->offset = offset;
                codecprivate->box = box;
            }
            return box_hdr_size;
        }
        BDBG_MSG(("b_mp4_find_box: looking " B_MP4_TYPE_FORMAT " skipping " B_MP4_TYPE_FORMAT ":%u", B_MP4_TYPE_ARG(type), B_MP4_TYPE_ARG(box.type), (unsigned)box.size));
        if(box.size>box_hdr_size) {
            batom_cursor_skip(cursor, box.size - box_hdr_size);
            continue;
        }
    }
    return 0;
}

bool
bmp4_parse_box_extended(batom_cursor *cursor, bmp4_box_extended *box)
{
	size_t size;
	BDBG_ASSERT(cursor);
	BDBG_ASSERT(box);

	/* page 3 */
	size = batom_cursor_copy(cursor, box->usertype, sizeof(box->usertype));
	return size==sizeof(box->usertype);
}

bool
bmp4_parse_fullbox(batom_cursor *cursor, bmp4_fullbox *box)
{
    BDBG_ASSERT(cursor);
    BDBG_ASSERT(box);

    /* page 4 */
    box->version = batom_cursor_byte(cursor);
    box->flags = batom_cursor_uint24_be(cursor);
    BDBG_MSG_TRACE(("bmp4_parse_fullbox: version:%u flags:%#x", (unsigned)box->version, (unsigned)box->flags));
    return !BATOM_IS_EOF(cursor);
}

bool
bmp4_parse_filetype(batom_t box, bmp4_filetypebox *filetype)
{
	batom_cursor cursor;
	BDBG_ASSERT(box);
	BDBG_ASSERT(filetype);

	batom_cursor_from_atom(&cursor, box);

	/* page 5 */
	filetype->major_brand = batom_cursor_uint32_be(&cursor);
	filetype->minor_version = batom_cursor_uint32_be(&cursor);
	if(!BATOM_IS_EOF(&cursor)) {
		unsigned i;
		for(i=0;i<sizeof(filetype->compatible_brands)/sizeof(filetype->compatible_brands[0]);i++) {
			filetype->compatible_brands[i] = batom_cursor_uint32_be(&cursor);
			if(BATOM_IS_EOF(&cursor)) {
				break;
			}
		}
		filetype->ncompatible_brands = i;
		BDBG_MSG(("bmp4_parse_filetype: major_brand:" B_MP4_TYPE_FORMAT " minor_version:%u ncompatible_brands:%u", B_MP4_TYPE_ARG(filetype->major_brand), (unsigned)filetype->minor_version, (unsigned)filetype->ncompatible_brands));
		return true;
	}
	return false;
}

bool
bmp4_parse_movieheader(batom_t box, bmp4_movieheaderbox *movieheader)
{
	batom_cursor cursor;
	bool valid=false;

	BDBG_ASSERT(box);
	BDBG_ASSERT(movieheader);

	batom_cursor_from_atom(&cursor, box);

	/* page 16 */
	if(bmp4_parse_fullbox(&cursor, &movieheader->fullbox)) {
		unsigned i;
		switch(movieheader->fullbox.version) {
		case 0:
			movieheader->creation_time = batom_cursor_uint32_be(&cursor);
			movieheader->modification_time = batom_cursor_uint32_be(&cursor);
			movieheader->timescale = batom_cursor_uint32_be(&cursor);
			movieheader->duration = batom_cursor_uint32_be(&cursor);
			break;
		case 1:
			movieheader->creation_time = batom_cursor_uint64_be(&cursor);
			movieheader->modification_time = batom_cursor_uint64_be(&cursor);
			movieheader->timescale = batom_cursor_uint32_be(&cursor);
			movieheader->duration = batom_cursor_uint64_be(&cursor);
			break;
		default:
			BDBG_WRN(("bmp4_parse_movieheader: unknown version %u", movieheader->fullbox.version));
			return false;
		}
		movieheader->rate = batom_cursor_uint32_be(&cursor);
		movieheader->volume = batom_cursor_uint16_be(&cursor);
		batom_cursor_skip(&cursor, 
				sizeof(uint16_t) +  /* reserved */
				2*sizeof(uint32_t)  /* reserved */
				);
		for(i=0;i<sizeof(movieheader->matrix)/sizeof(movieheader->matrix[0]);i++) {
			movieheader->matrix[i] = batom_cursor_uint32_be(&cursor);
		}
		batom_cursor_skip(&cursor, sizeof(uint32_t) /* pre_defined */ );
		movieheader->next_track_ID = batom_cursor_uint32_be(&cursor);
		valid = movieheader->timescale>0 && (!BATOM_IS_EOF(&cursor));
		if(valid) {
			BDBG_MSG(("bmp4_parse_movieheader: version:%u creation_time:%u modification_time:%u timescale:%u duration:%u(%u sec) rate:%u volume:%u next_track_ID:%u", (unsigned)movieheader->fullbox.version, (unsigned)movieheader->creation_time, (unsigned)movieheader->modification_time, (unsigned)movieheader->timescale, (unsigned)movieheader->duration, (unsigned)(movieheader->duration/movieheader->timescale), (unsigned)movieheader->rate, (unsigned)movieheader->volume, (unsigned)movieheader->next_track_ID));
		}
	}
	return valid;
}

bool
bmp4_parse_trackheader(batom_t box, bmp4_trackheaderbox *trackheader)
{
	batom_cursor cursor;
	bool valid=false;

	BDBG_ASSERT(box);
	BDBG_ASSERT(trackheader);

	batom_cursor_from_atom(&cursor, box);

	/* page 17 */
	if(bmp4_parse_fullbox(&cursor, &trackheader->fullbox)) {
		unsigned i;
		switch(trackheader->fullbox.version) {
		case 0:
			trackheader->creation_time = batom_cursor_uint32_be(&cursor);
			trackheader->modification_time = batom_cursor_uint32_be(&cursor);
			trackheader->track_ID = batom_cursor_uint32_be(&cursor);
			batom_cursor_skip(&cursor, sizeof(uint32_t));
			trackheader->duration = batom_cursor_uint32_be(&cursor);
			break;
		case 1:
			trackheader->creation_time = batom_cursor_uint64_be(&cursor);
			trackheader->modification_time = batom_cursor_uint64_be(&cursor);
			trackheader->track_ID = batom_cursor_uint32_be(&cursor);
			batom_cursor_skip(&cursor, sizeof(uint32_t));
			trackheader->duration = batom_cursor_uint64_be(&cursor);
			break;
		default:
			BDBG_WRN(("bmp4_parse_movieheader: unknown version %u", trackheader->fullbox.version));
			return false;
		}
		batom_cursor_skip(&cursor, 2*sizeof(uint32_t));
		trackheader->layer = batom_cursor_uint16_be(&cursor);
		trackheader->alternate_group = batom_cursor_uint16_be(&cursor);
		trackheader->volume = batom_cursor_uint16_be(&cursor);
		batom_cursor_skip(&cursor, sizeof(uint16_t));
		for(i=0;i<sizeof(trackheader->matrix)/sizeof(trackheader->matrix[0]);i++) {
			trackheader->matrix[i] = batom_cursor_uint32_be(&cursor);
		}
		trackheader->width = batom_cursor_uint32_be(&cursor);
		trackheader->height = batom_cursor_uint32_be(&cursor);
		valid = !BATOM_IS_EOF(&cursor);
		if(valid) {
			BDBG_MSG(("bmp4_parse_trackheader: version:%u track_ID:%u duration:%u layer:%u alternate_group:%u volume:%u width:%u height:%u", (unsigned)trackheader->fullbox.version, (unsigned)trackheader->track_ID, (unsigned)trackheader->duration, (unsigned)trackheader->layer, (unsigned)trackheader->alternate_group, (unsigned)trackheader->volume, (unsigned)trackheader->width>>16, (unsigned)trackheader->height>>16));
		}
	}
	return valid;
}

bool
bmp4_parse_mediaheader(batom_t box, bmp4_mediaheaderbox *mediaheader)
{
	batom_cursor cursor;
	bool valid=false;
    uint16_t language;

	BDBG_ASSERT(box);
	BDBG_ASSERT(mediaheader);

	batom_cursor_from_atom(&cursor, box);

	/* page 20 */
	if(bmp4_parse_fullbox(&cursor, &mediaheader->fullbox)) {
		switch(mediaheader->fullbox.version) {
		case 0:
			mediaheader->creation_time = batom_cursor_uint32_be(&cursor);
			mediaheader->modification_time = batom_cursor_uint32_be(&cursor);
			mediaheader->timescale = batom_cursor_uint32_be(&cursor);
			mediaheader->duration = batom_cursor_uint32_be(&cursor);
			break;

		case 1:
			mediaheader->creation_time = batom_cursor_uint64_be(&cursor);
			mediaheader->modification_time = batom_cursor_uint64_be(&cursor);
			mediaheader->timescale = batom_cursor_uint32_be(&cursor);
			mediaheader->duration = batom_cursor_uint64_be(&cursor);
			break;
		default:
			BDBG_WRN(("bmp4_parse_mediaheader: unknown version %u", mediaheader->fullbox.version));
			return false;
		}
        valid = mediaheader->timescale>0 && (!BATOM_IS_EOF(&cursor));
        mediaheader->language[0] = '\0';
        language = batom_cursor_uint16_be(&cursor);
        if(!BATOM_IS_EOF(&cursor)) {
            mediaheader->language[0] = 0x60 + B_GET_BITS(language, 14, 10);
            mediaheader->language[1] = 0x60 + B_GET_BITS(language,  9, 5);
            mediaheader->language[2] = 0x60 + B_GET_BITS(language,  4, 0);
            mediaheader->language[3] = '\0';
        }
        if(valid) {
            BDBG_MSG(("bmp4_parse_mediaheader: version:%u creation_time:%u modification_time:%u timescale:%u(%u sec) duration:%u language:%s", (unsigned)mediaheader->fullbox.version, (unsigned)mediaheader->creation_time, (unsigned)mediaheader->modification_time, (unsigned)mediaheader->timescale, (unsigned)(mediaheader->duration/mediaheader->timescale), (unsigned)mediaheader->duration, mediaheader->language));
        }
	}
	return valid;
}

bool 
bmp4_parse_string(batom_cursor *cursor, char *string, size_t strlen)
{
	unsigned i;

	BDBG_ASSERT(cursor);
	BDBG_ASSERT(strlen>0);
	BDBG_ASSERT(string);

	for(i=0;i<strlen;i++) {
		int ch = batom_cursor_next(cursor);
		if(ch==BATOM_EOF) {
			i++;
            break;
		}
		string[i]=ch;
		if(ch==0) {
			return true;
		}
	}
	if(i>0) {
		string[i-1] = '\0';
		return true;
	} else {
		return false;
	}
}

bool
bmp4_parse_handler(batom_t box, bmp4_handlerbox *handler)
{
	batom_cursor cursor;
	bool valid=false;
	BDBG_ASSERT(box);
	BDBG_ASSERT(handler);

	batom_cursor_from_atom(&cursor, box);

	/* page 20 */
	if(bmp4_parse_fullbox(&cursor, &handler->fullbox)) {
		batom_cursor_skip(&cursor, sizeof(uint32_t) /* pre defined */);
		handler->handler_type = batom_cursor_uint32_be(&cursor);
		batom_cursor_skip(&cursor, 3*sizeof(uint32_t) /* pre defined */);
		valid = bmp4_parse_string(&cursor, handler->name, sizeof(handler->name));
		if(valid) {
			BDBG_MSG(("bmp4_parse_handler: version:%u handler_type:" B_MP4_TYPE_FORMAT " name:'%s'", (unsigned)handler->fullbox.version, B_MP4_TYPE_ARG(handler->handler_type), handler->name));
		}
	}
	return valid;
}


bool 
bmp4_parse_visualsampleentry(batom_cursor *cursor, bmp4_visualsampleentry *entry)
{
	bool valid;

	batom_cursor_skip(cursor, sizeof(uint16_t) + sizeof(uint16_t) + 3*sizeof(uint32_t));
	entry->width = batom_cursor_uint16_be(cursor);
	entry->height = batom_cursor_uint16_be(cursor);
	entry->horizresolution = batom_cursor_uint32_be(cursor);
	entry->vertresolution = batom_cursor_uint32_be(cursor);
	batom_cursor_skip(cursor, sizeof(uint32_t));
	entry->frame_count = batom_cursor_uint16_be(cursor);
	batom_cursor_copy(cursor, entry->compressorname, sizeof(entry->compressorname));
	entry->compressorname[sizeof(entry->compressorname)-1] = '\0';
	entry->depth = batom_cursor_uint16_be(cursor);
	batom_cursor_skip(cursor, sizeof(int16_t));
	valid = !BATOM_IS_EOF(cursor);
	if(valid) {
		BDBG_MSG(("bmp4_parse_visualsampleentry: width:%u height:%u horizresolution:%u vertresolution:%u frame_count:%u compressorname:%s depth:%u", (unsigned)entry->width, (unsigned)entry->height, (unsigned)entry->horizresolution>>16, (unsigned)entry->vertresolution>>16, (unsigned)entry->frame_count, entry->compressorname, (unsigned)entry->depth));
	}
	return valid;
}

bool 
bmp4_parse_audiosampleentry(batom_cursor *cursor, bmp4_audiosampleentry *entry)
{
	bool valid;

    entry->version = batom_cursor_uint16_be(cursor);
    batom_cursor_skip(cursor, sizeof(uint16_t)+sizeof(uint32_t));
	entry->channelcount = batom_cursor_uint16_be(cursor);
	entry->samplesize = batom_cursor_uint16_be(cursor);
	batom_cursor_skip(cursor, sizeof(uint16_t) + sizeof(uint16_t));
	entry->samplerate = batom_cursor_uint32_be(cursor);
	valid = !BATOM_IS_EOF(cursor);
	if(valid) {
		BDBG_MSG(("bmp4_parse_audiosampleentry: version:%u channelcount:%u samplesize:%u samplerate:%u", (unsigned)entry->version, (unsigned)entry->channelcount, (unsigned)entry->samplesize, (unsigned)entry->samplerate>>16));
	}
	return valid;
}

bool
bmp4_parse_sample_avcC(batom_cursor *cursor, bmp4_sample_avc *avc, size_t entry_data_size)
{
    BDBG_ASSERT(cursor);
    BDBG_ASSERT(avc);

    batom_cursor_copy(cursor, avc->data, entry_data_size);
    if(!bmedia_read_h264_meta(&avc->meta, avc->data, entry_data_size)) {
        return false;
    }
    BDBG_MSG(("b_mp4_parse_sample_avc: configurationVersion:%u AVCProfileIndication:%u profile_compatibility:%u AVCLevelIndication:%u lengthSize:%u nSequenceParameterSets:%u nPictureParameterSets:%u", (unsigned)avc->meta.configurationVersion, (unsigned)avc->meta.profileIndication, (unsigned)avc->meta.profileCompatibility, (unsigned)avc->meta.levelIndication, (unsigned)avc->meta.nalu_len, (unsigned)avc->meta.sps.no, (unsigned)avc->meta.pps.no));
    return true;
}

static bool
b_mp4_parse_sample_avc(batom_cursor *cursor, bmp4_sample_avc *avc, size_t entry_data_size, bmp4_sample_codecprivate *codecprivate)
{
    BDBG_ASSERT(cursor);
    BDBG_ASSERT(avc);
    if(bmp4_parse_visualsampleentry(cursor, &avc->visual)) {
        bmp4_box box;

        for(;;) {
            size_t box_hdr_size;
            unsigned offset = batom_cursor_pos(cursor);
            box_hdr_size = bmp4_parse_box(cursor, &box);
            if(box_hdr_size==0) {
                break;
            }
            if(box.type==BMP4_TYPE('a','v','c','C')) {
                codecprivate->offset = offset;
                codecprivate->box = box;
                return bmp4_parse_sample_avcC(cursor, avc, entry_data_size);
            }
            /* skip unknown boxes */
            BDBG_WRN(("b_mp4_parse_sample_avc: discarding unknown sample " B_MP4_TYPE_FORMAT " %u:%u", B_MP4_TYPE_ARG(box.type), (unsigned)box.size, (unsigned)entry_data_size));
            if(entry_data_size<box.size) {
                break;
            }
            entry_data_size -= box.size;
            batom_cursor_skip(cursor, box.size-box_hdr_size);
        }
    }
    return false;
}

static bool
b_mp4_parse_sample_hevc(batom_cursor *cursor, bmp4_sample_hevc *hevc, bmp4_sample_codecprivate *codecprivate)
{
    BDBG_ASSERT(cursor);
    BDBG_ASSERT(hevc);
    if(bmp4_parse_visualsampleentry(cursor, &hevc->visual)) {
        unsigned configurationVersion;
        if(!b_mp4_find_box(cursor, BMP4_TYPE('h','v','c','C'), codecprivate)) {
            return false;
        }
        configurationVersion = batom_cursor_byte(cursor);
        if(BATOM_IS_EOF(cursor)) {
            return false;
        }
        if(configurationVersion!=1) {
            BDBG_ERR(("b_mp4_parse_sample_hevc: not supported version %u", configurationVersion));
            return false;
        }
        if(!bmedia_read_h265_meta(cursor, &hevc->meta, bkni_alloc)) {
            BDBG_ERR(("b_mp4_parse_sample_hevc: can't parse hvcC data"));
            return false;
        }
        return true;
    }
    return false;
}

static bool
b_mp4_parse_sample_mp4a(batom_cursor *cursor, bmp4_sample_mp4a *mp4a, size_t entry_data_size, bmp4_sample_codecprivate *codecprivate)
{
    size_t last = batom_cursor_pos(cursor)+entry_data_size;

    if(bmp4_parse_audiosampleentry(cursor, &mp4a->audio)) {
        for(;;) {
            batom_checkpoint chk;
            bmp4_box box;
            bmp4_fullbox fullbox;
            size_t box_hdr_size;
            unsigned offset = batom_cursor_pos(cursor);

            BATOM_SAVE(cursor, &chk);
            box_hdr_size = bmp4_parse_box(cursor, &box);

            if(box_hdr_size == 0 || box.type!=BMP4_TYPE('e','s','d','s')) {
                /* not "really" MPEG-4 Part12/Part14 streams have esds box at random position in the MP4A box, so skip unknown data and try over */
                if(BATOM_IS_EOF(cursor) || batom_cursor_pos(cursor)>=last) {
                    break;
                }
                batom_cursor_rollback(cursor, &chk);
                batom_cursor_skip(cursor, 2);
                continue;
            }
            if(!bmp4_parse_fullbox(cursor, &fullbox)) {
                return false;
            }
            codecprivate->offset = offset;
            codecprivate->box = box;
            if(bmpeg4_parse_es_descriptor(cursor, &mp4a->mpeg4)) {
                if(mp4a->mpeg4.objectTypeIndication==BMPEG4_Audio_ISO_IEC_14496_3 || mp4a->mpeg4.objectTypeIndication==BMPEG4_Audio_ISO_IEC_13818_7) {
#if BDBG_DEBUG_BUILD
                    const char *codec;
                    unsigned sample_rate;
                    static const unsigned sample_rate_table[] = {96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350};
                    switch(mp4a->mpeg4.decoder.iso_14496_3.audioObjectType) {
                    case 1:
                        codec = "AAC main";
                        break;
                    case 2:
                        codec = "AAC LC";
                        break;
                    case 3:
                        codec = "AAC SSR";
                        break;
                    case 4:
                        codec = "AAC LTP";
                        break;
                    case 5: 
                        codec = "AAC SBR";
                        break;
                    default:
                        codec = "";
                        break;
                    }
                    if(mp4a->mpeg4.decoder.iso_14496_3.samplingFrequencyIndex<sizeof(sample_rate_table)/sizeof(*sample_rate_table)) {
                        sample_rate = sample_rate_table[mp4a->mpeg4.decoder.iso_14496_3.samplingFrequencyIndex];
                    } else {
                        sample_rate = 0;
                    }
                    BDBG_MSG(("b_mp4_parse_sample_mp4a: codec:%s(%#x) sampling_rate:%u(%u) channelConfiguration:%u", codec, mp4a->mpeg4.decoder.iso_14496_3.audioObjectType, sample_rate, mp4a->mpeg4.decoder.iso_14496_3.samplingFrequencyIndex, mp4a->mpeg4.decoder.iso_14496_3.channelConfiguration));
#endif  /* BDBG_DEBUG_BUILD */
                    return true;
                } else if (mp4a->mpeg4.objectTypeIndication==BMPEG4_Audio_ISO_IEC_11172_3) {
                    return true;
                } else {
                    BDBG_WRN(("b_mp4_parse_sample_mp4a: unsupported objectTypeIndication %#x", mp4a->mpeg4.objectTypeIndication));
                }
            }
            break;
        }
	}
	return false;
}

size_t
bmp4_scan_box(batom_cursor *cursor, uint32_t type, bmp4_box *box)
{
    BDBG_ASSERT(cursor);
    BDBG_ASSERT(box);
    for(;;) {
        uint32_t size;
        batom_cursor box_cursor;

        BATOM_CLONE(&box_cursor, cursor);
        size = batom_cursor_uint32_be(&box_cursor);
        box->type = batom_cursor_uint32_be(&box_cursor);
        BDBG_MSG_TRACE(("bmp4_scan_box: %u %#x:%#x", (unsigned)batom_cursor_pos(&box_cursor), type, box->type));
        if(BATOM_IS_EOF(&box_cursor)) {
            return 0;
        }
        if(box->type == type) {
            return bmp4_parse_box(cursor,box);
        }
        batom_cursor_skip(cursor, 1);
    }
}

static bool
b_mp4_parse_sample_mp4v(batom_cursor *cursor, bmp4_sample_mp4v *mp4v)
{
	if(bmp4_parse_visualsampleentry(cursor, &mp4v->visual)) {
		bmp4_fullbox fullbox;

        if(!b_mp4_find_box(cursor, BMP4_TYPE('e','s','d','s'), NULL)) {
            return false;
        }
		if(!bmp4_parse_fullbox(cursor, &fullbox)) {
			return false;
		}
		if(bmpeg4_parse_es_descriptor(cursor, &mp4v->mpeg4)) {
			if(mp4v->mpeg4.objectTypeIndication==BMPEG4_Video_ISO_IEC_14496_2) {
				return true;
			} else {
			 	BDBG_WRN(("b_mp4_parse_sample_mp4v: unsupported objectTypeIndication %#x", mp4v->mpeg4.objectTypeIndication));
			}
		}
	}
	return false;
}

static bool
b_mp4_parse_sample_s263(batom_cursor *cursor, bmp4_sample_s263 *s263)
{
    if(bmp4_parse_visualsampleentry(cursor, &s263->visual)) {
        return true;
    }
    return false;
}

static bool 
b_mp4_parse_sample_amr(batom_cursor *cursor, bmp4_sample_amr *amr, size_t entry_data_size)
{
    size_t last = batom_cursor_pos(cursor)+entry_data_size;
	if(bmp4_parse_audiosampleentry(cursor, &amr->audio)) {
        for(;;) {
            batom_checkpoint chk;
            bmp4_box box;
            size_t box_hdr_size;

            BATOM_SAVE(cursor, &chk);
            box_hdr_size = bmp4_parse_box(cursor, &box);

            if(box_hdr_size == 0 || box.type!=BMP4_TYPE('d','a','m','r')) {
                /* there is some junk stored priort to the AMRSpecificBox */
                if(BATOM_IS_EOF(cursor) || batom_cursor_pos(cursor)>=last) {
                    break;
                }
	            batom_cursor_rollback(cursor, &chk);
                batom_cursor_skip(cursor, 2);
                continue;
            }
            break;
        }
        amr->vendor = batom_cursor_uint32_be(cursor);
        amr->decoder_version = batom_cursor_byte(cursor);
        amr->mode_set = batom_cursor_uint16_be(cursor);
        amr->mode_change_period = batom_cursor_byte(cursor);
        amr->frames_per_sample = batom_cursor_byte(cursor);

        if(!BATOM_IS_EOF(cursor)) {
            BDBG_MSG(("b_mp4_parse_sample_amr: vendor(" B_MP4_TYPE_FORMAT ") decoder_version:%u mode_set:%u mode_change_period:%u frames_per_sample:%u", B_MP4_TYPE_ARG(amr->vendor), amr->decoder_version, amr->mode_set, amr->mode_change_period, amr->frames_per_sample));
            return true;
        }
	}
	return false;
}


static bool 
b_mp4_parse_sample_ms(batom_cursor *cursor, bmp4_sample_ms *ms)
{
    /* QuickTime File Format Specification , Sound Sample Description Extensions, p 136 */
	if(bmp4_parse_audiosampleentry(cursor, &ms->audio)) {
        uint32_t samplesPerPacket;
        uint32_t bytesPerPacket;
        uint32_t bytesPerFrame;
        uint32_t bytesPerSample;

        samplesPerPacket = batom_cursor_uint32_be(cursor);
        bytesPerPacket = batom_cursor_uint32_be(cursor);
        bytesPerFrame = batom_cursor_uint32_be(cursor);
        bytesPerSample = batom_cursor_uint32_be(cursor);
        if(b_mp4_find_box(cursor, BMP4_TYPE('w','a','v','e'), NULL)) {
            if(b_mp4_find_box(cursor, BMP4_TYPE('f','r','m','a'), NULL)) {
                uint32_t type = batom_cursor_uint32_be(cursor);
                if(b_mp4_find_box(cursor, type, NULL)) {
                    return bmedia_read_waveformatex(&ms->waveformat, cursor);
                }
            }
        }
    }
    return false;
}

typedef struct bmp4_sample_sound_descriptor_v1 {
    uint32_t samplesPerPacket;
    uint32_t bytesPerPacket;
    uint32_t bytesPerFrame;
    uint32_t bytesPerSample;
} bmp4_sample_sound_descriptor_v1;

static bool
b_mp4_parse_sample_sound_descriptor_v1(batom_cursor *cursor, bmp4_sample_sound_descriptor_v1 *desc)
{
    bool valid;
    desc->samplesPerPacket = batom_cursor_uint32_be(cursor);
    desc->bytesPerPacket = batom_cursor_uint32_be(cursor);
    desc->bytesPerFrame = batom_cursor_uint32_be(cursor);
    desc->bytesPerSample = batom_cursor_uint32_be(cursor);
    valid = !BATOM_IS_EOF(cursor);
    if(valid) {
        BDBG_MSG(("b_mp4_parse_sample_sound_descriptor_v1: samplesPerPacket:%u bytesPerSample:%u bytesPerFrame:%u bytesPerSample:%u", (unsigned)desc->samplesPerPacket, (unsigned)desc->bytesPerPacket, (unsigned)desc->bytesPerFrame, (unsigned)desc->bytesPerSample));
    }
    return valid;
}

static bool 
b_mp4_set_sample_sound_frame_info(batom_cursor *cursor, bmp4_sample_sound_frame_info *frame_info)
{
    bmp4_sample_sound_descriptor_v1 desc;
    if(!b_mp4_parse_sample_sound_descriptor_v1(cursor, &desc)) {
        return false;
    }
    frame_info->samplesPerPacket = desc.samplesPerPacket;
    frame_info->bytesPerFrame = desc.bytesPerFrame;
    return true;
}

static bool
b_mp4_parse_sample_ima4(batom_cursor *cursor, bmp4_sample_ima4 *ima4)
{
    if(!bmp4_parse_audiosampleentry(cursor, &ima4->audio)) {
        return false;
    }
    if(ima4->audio.version==1) {
        if(!b_mp4_set_sample_sound_frame_info(cursor, &ima4->frame_info)) {
            return false;
        }
    } else {
        ima4->frame_info.samplesPerPacket = 64;
        ima4->frame_info.bytesPerFrame = 34 * ima4->audio.channelcount;
    }
    BDBG_MSG(("b_mp4_parse_sample_ima4: samplesPerPacket:%u bytesPerFrame:%u",(unsigned)ima4->frame_info.samplesPerPacket, (unsigned)ima4->frame_info.bytesPerFrame));
    return true;
}

static bool 
b_mp4_parse_sample_mjpeg(batom_cursor *cursor, bmp4_sample_mjpeg *mjpeg, uint32_t type)
{
    mjpeg->type = type;
	if(bmp4_parse_visualsampleentry(cursor, &mjpeg->visual)) {
		return true;
	}
	return false;
}

static bool 
b_mp4_parse_sample_twos(batom_cursor *cursor, bmp4_sample_twos *twos, uint32_t type)
{
    twos->type = type;
    if(!bmp4_parse_audiosampleentry(cursor, &twos->audio)) {
        return false;
    }
    if(twos->audio.version==1) {
        if(!b_mp4_set_sample_sound_frame_info(cursor, &twos->frame_info)) {
            return false;
        }
    } else {
        twos->frame_info.samplesPerPacket = 1;
        twos->frame_info.bytesPerFrame = (twos->audio.samplesize * twos->audio.channelcount)/8;
    }
    BDBG_MSG(("b_mp4_parse_sample_twos: samplesPerPacket:%u bytesPerFrame:%u",(unsigned)twos->frame_info.samplesPerPacket, (unsigned)twos->frame_info.bytesPerFrame));
    return true;
}

static bool
b_mp4_parse_sample_dts(batom_cursor *cursor, bmp4_sample_dts *dts, uint32_t type)
{
    dts->type = type;
    if(bmp4_parse_audiosampleentry(cursor, &dts->audio)) {
        return true;
    }
    return false;
}

#define B_MP4_SAMPLE_ENTRY_SIZE        (BMEDIA_FIELD_LEN(SampleEntry:reserved, const uint8_t [6])+ BMEDIA_FIELD_LEN(SampleEntry:mdata_reference_index, uint16_t))
#define B_MP4_VISUAL_SAMPLE_ENTRY_SIZE (B_MP4_SAMPLE_ENTRY_SIZE+\
                                       BMEDIA_FIELD_LEN(VisualSampleEntry:pre_defined, uint16_t)+\
                                       BMEDIA_FIELD_LEN(VisualSampleEntry:reserved, const uint16_t)+\
                                       BMEDIA_FIELD_LEN(VisualSampleEntry:prefetched, uint32_t [3])+\
                                       BMEDIA_FIELD_LEN(VisualSampleEntry:width, uint16_t)+BMEDIA_FIELD_LEN(VisualSampleEntry:height, uint16_t)+\
                                       BMEDIA_FIELD_LEN(VisualSampleEntry:horizresolution, uint32_t)+BMEDIA_FIELD_LEN(VisualSampleEntry:vertresolution, uint32_t)+\
                                       BMEDIA_FIELD_LEN(VisualSampleEntry:reserved, const uint32_t)+\
                                       BMEDIA_FIELD_LEN(VisualSampleEntry:frame_count, uint16_t)+\
                                       BMEDIA_FIELD_LEN(VisualSampleEntry:compressorname, uint8_t [32])+\
                                       BMEDIA_FIELD_LEN(VisualSampleEntry:depth, uint16_t)+\
                                       BMEDIA_FIELD_LEN(VisualSampleEntry:predefined, int16_t))

#define B_MP4_AUDIO_SAMPLE_ENTRY_SIZE  (B_MP4_SAMPLE_ENTRY_SIZE+\
                                       BMEDIA_FIELD_LEN(AudioSampleEntry:reserved, const uint32_t [2])+\
                                       BMEDIA_FIELD_LEN(AudioSampleEntry:channelcount, int16_t)+\
                                       BMEDIA_FIELD_LEN(AudioSampleEntry:samplesize , int16_t)+\
                                       BMEDIA_FIELD_LEN(AudioSampleEntry:pre_defined, int16_t)+\
                                       BMEDIA_FIELD_LEN(AudioSampleEntry:reserved, const int16_t)+\
                                       BMEDIA_FIELD_LEN(AudioSampleEntry:samplerate, int32_t))


bool
bmp4_parse_sample_info(batom_t box, bmp4_sample_info *sample, uint32_t handler_type)
{
    batom_cursor cursor;
    unsigned i;

    BDBG_ASSERT(box);
    BDBG_ASSERT(sample);


    batom_cursor_from_atom(&cursor, box);

    sample->entry_count = 0;
    if(!bmp4_parse_fullbox(&cursor, &sample->fullbox)) {
        return false;
    }
    sample->entry_count = batom_cursor_uint32_be(&cursor);
    if(BATOM_IS_EOF(&cursor)) {
        return false;
    }
    if(sample->entry_count>B_MP4_MAX_SAMPLE_ENTRIES) {
        BDBG_WRN(("bmp4_parse_sample: truncating number of samples %u -> %u", sample->entry_count, B_MP4_MAX_SAMPLE_ENTRIES));
        sample->entry_count=B_MP4_MAX_SAMPLE_ENTRIES;
    }
    for(i=0;i<sample->entry_count;i++) {
        size_t box_hdr_size;
        size_t start_pos;
        size_t end_pos;
        size_t entry_data_size;
        bmp4_sample_type type;
        bmp4_sampleentry *entry;
        bmp4_box entry_box;
        bool encrypted = false;
        batom_cursor protection_scheme_information_cursor;
        size_t protection_scheme_information_size = 0;

        start_pos = batom_cursor_pos(&cursor);
        box_hdr_size = bmp4_parse_box(&cursor, &entry_box);
        if(box_hdr_size==0 || entry_box.size > batom_len(box)) {
            BDBG_WRN(("bmp4_parse_sample: invalid sample %u", i));
            goto error_sample;
        }
        if( (handler_type==BMP4_HANDLER_VISUAL && entry_box.type==BMP4_SAMPLE_ENCRYPTED_VIDEO) || 
            (handler_type==BMP4_HANDLER_AUDIO && entry_box.type==BMP4_SAMPLE_ENCRYPTED_AUDIO) ) {
            batom_cursor sinf_cursor;

            encrypted = true;
            BATOM_CLONE(&sinf_cursor, &cursor);
            batom_cursor_skip(&sinf_cursor, handler_type==BMP4_HANDLER_VISUAL?B_MP4_VISUAL_SAMPLE_ENTRY_SIZE:B_MP4_AUDIO_SAMPLE_ENTRY_SIZE);
            for(;;) {
                size_t sinf_box_hdr_size;
                bmp4_box sinf_box;

                BATOM_CLONE(&protection_scheme_information_cursor, &sinf_cursor);
                sinf_box_hdr_size = bmp4_parse_box(&sinf_cursor, &sinf_box);
                if(sinf_box_hdr_size==0 || sinf_box_hdr_size > sinf_box.size || batom_cursor_reserve(&protection_scheme_information_cursor, sinf_box.size)!=sinf_box.size) {
                    break;
                }
                if(sinf_box.type == BMP4_TYPE('s','i','n','f')) {
                    protection_scheme_information_size = sinf_box.size;
                    if(protection_scheme_information_size>sizeof(entry->protection_scheme_information)) {
                        break;
                    }
                    sinf_box_hdr_size = bmp4_parse_box(&sinf_cursor, &sinf_box);
                    if(sinf_box_hdr_size==0 || sinf_box_hdr_size > sinf_box.size) {
                        protection_scheme_information_size = 0;
                        break;
                    }
                    if(sinf_box.type == BMP4_TYPE('f','r','m','a')) {
                        uint32_t data_format = batom_cursor_uint32_be(&sinf_cursor);
                        if(!BATOM_IS_EOF(&sinf_cursor)) {
                            entry_box.type = data_format;
                        } else {
                            protection_scheme_information_size = 0;
                        }
                    }
                    break;
                }
                batom_cursor_skip(&sinf_cursor, sinf_box.size - sinf_box_hdr_size);
            }
            if(protection_scheme_information_size==0) {
                BDBG_WRN(("bmp4_parse_sample: can't find useful Protection Scheme Informatio Box ('sinf') for protected track"));
            }
        }

        entry_data_size = 0;
        type = bmp4_sample_type_unknown;
        BDBG_MSG(("bmp4_parse_sample: type " B_MP4_TYPE_FORMAT " -> " B_MP4_TYPE_FORMAT, B_MP4_TYPE_ARG(handler_type), B_MP4_TYPE_ARG(entry_box.type)));
        switch(handler_type) {
        case BMP4_HANDLER_VISUAL:
            switch(entry_box.type) {
            case BMP4_SAMPLE_AVC:
                BDBG_MSG(("bmp4_parse_sample: avc video"));
                type = bmp4_sample_type_avc;
                entry_data_size =
                    box_hdr_size +
                    B_MP4_VISUAL_SAMPLE_ENTRY_SIZE +
                    sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint8_t) /* AVCDecoderConfigurationRecord */;
                if(entry_data_size>entry_box.size)  {
                    BDBG_WRN(("bmp4_parse_sample: invalid size of AVC box %u>%u", (unsigned)entry_data_size, (unsigned)entry_box.size));
                    goto error_sample;
                }
                entry_data_size = entry_box.size - entry_data_size;
                break;
            case BMP4_SAMPLE_HVC1:
            case BMP4_SAMPLE_HEV1:
                BDBG_MSG(("bmp4_parse_sample: HEVC video"));
                type = bmp4_sample_type_hevc;
                break;
            case BMP4_SAMPLE_MP4V:
                BDBG_MSG(("bmp4_parse_sample: MPEG4-Part2 video"));
                type = bmp4_sample_type_mp4v;
                break;
            case BMP4_SAMPLE_H263:
            case BMP4_SAMPLE_S263:
                BDBG_MSG(("bmp4_parse_sample: H.263 video"));
                type = bmp4_sample_type_s263;
                break;
               case BMP4_SAMPLE_JPEG:
               case BMP4_SAMPLE_MJPA:
               case BMP4_SAMPLE_MJPB:
                BDBG_MSG(("bmp4_parse_sample: MJPEG video " B_MP4_TYPE_FORMAT, B_MP4_TYPE_ARG(entry_box.type)));
                type = bmp4_sample_type_mjpeg;
                break;
            case BMP4_SAMPLE_DRMI:
                BDBG_MSG(("bmp4_parse_sample: DRM video"));
                type = bmp4_sample_type_drmi;
                break;
            default:
                break;
            }
            break;
        case BMP4_HANDLER_AUDIO:
            switch(entry_box.type) {
            case BMP4_SAMPLE_MP4A:
                BDBG_MSG(("bmp4_parse_sample: MP4A(AAC) audio"));
                type = bmp4_sample_type_mp4a;
                break;
            case BMP4_SAMPLE_AC3:
                BDBG_MSG(("bmp4_parse_sample: AC-3 audio"));
                type = bmp4_sample_type_ac3;
                break;
            case BMP4_SAMPLE_EAC3:
                BDBG_MSG(("bmp4_parse_sample: AC-3 audio"));
                type = bmp4_sample_type_eac3;
                break;
            case BMP4_SAMPLE_SAMR:
                BDBG_MSG(("bmp4_parse_sample: SAMR audio"));
                type = bmp4_sample_type_samr;
                break;
            case BMP4_SAMPLE_SAWB:
                BDBG_MSG(("bmp4_parse_sample: SAWB audio"));
                type = bmp4_sample_type_sawb;
                break;
            case BMP4_SAMPLE_SAWP:
                BDBG_MSG(("bmp4_parse_sample: SAWO audio"));
                type = bmp4_sample_type_sawp;
                break;
            case BMP4_SAMPLE_DRMS:
                BDBG_MSG(("bmp4_parse_sample: DRM audio"));
                type = bmp4_sample_type_drms;
                break;
            case BMP4_SAMPLE_QT_IMA_ADPCM:
                BDBG_MSG(("bmp4_parse_sample: IMA ADPCM audio"));
                type = bmp4_sample_type_qt_ima_adpcm;
                break;
            case BMP4_SAMPLE_QT_IMA4_ADPCM:
                BDBG_MSG(("bmp4_parse_sample: IMA4 ADPCM audio"));
                type = bmp4_sample_type_qt_ima4_adpcm;
                break;
               case BMP4_SAMPLE_TWOS:
            case BMP4_SAMPLE_SOWT:
                BDBG_MSG(("bmp4_parse_sample: TWOS audio"));
                type = bmp4_sample_type_twos;
                break;
               case BMP4_SAMPLE_DTSC:
               case BMP4_SAMPLE_DTSH:
               case BMP4_SAMPLE_DTSL:
               case BMP4_SAMPLE_DTSE:
                BDBG_MSG(("bmp4_parse_sample: DTS audio " B_MP4_TYPE_FORMAT, B_MP4_TYPE_ARG(entry_box.type)));
                type = bmp4_sample_type_dts;
                break;
            case BMP4_SAMPLE_MP3:
                BDBG_MSG(("bmp4_parse_sample: MP3 audio"));
                type = bmp4_sample_type_mp3;
                break;
            default:
                break;
            }
        default:
            break;
        }
        entry = BKNI_Malloc(sizeof(*entry)+entry_data_size);
        if(!entry) {
            BDBG_ERR(("bmp4_parse_sample: can't allocate %u bytes", (unsigned)(sizeof(*entry)+entry_data_size)));
            goto error_sample;
        }
        entry->type = entry_box.type;
        entry->sample_type = type;
        batom_cursor_skip(&cursor, 6*sizeof(uint8_t));
        entry->data_reference_index = batom_cursor_uint16_be(&cursor);
        entry->encrypted = encrypted;
        entry->protection_scheme_information_size = protection_scheme_information_size;
        BKNI_Memset(&entry->codecprivate, 0, sizeof(entry->codecprivate));
        if(protection_scheme_information_size) {
            BDBG_ASSERT(protection_scheme_information_size <= sizeof(entry->protection_scheme_information));
            batom_cursor_copy(&protection_scheme_information_cursor, entry->protection_scheme_information, protection_scheme_information_size);
        }
        switch(type) {
        case bmp4_sample_type_avc:
            if(!b_mp4_parse_sample_avc(&cursor, &entry->codec.avc, entry_data_size, &entry->codecprivate)) {
                BDBG_WRN(("bmp4_parse_sample: error while parsing AVC sample"));
                entry->sample_type = bmp4_sample_type_unknown;
            }
            break;
        case bmp4_sample_type_mp4a:
            entry_data_size = batom_cursor_pos(&cursor);
            if(  entry_data_size >= start_pos && entry_box.size > (entry_data_size - start_pos) ) {
                entry_data_size = entry_box.size - (entry_data_size - start_pos);
            }
            if(!b_mp4_parse_sample_mp4a(&cursor, &entry->codec.mp4a, entry_data_size, &entry->codecprivate)) {
                BDBG_WRN(("bmp4_parse_sample: error while parsing MP4A sample"));
                entry->sample_type = bmp4_sample_type_unknown;
            }
            /* MPEG 1/2/3 audio is listed under the MP4A box */
            if (entry->codec.mp4a.mpeg4.objectTypeIndication == BMPEG4_Audio_ISO_IEC_11172_3) {
                entry->sample_type = bmp4_sample_type_mpg;
            } else if(entry->codec.mp4a.mpeg4.objectTypeIndication==BMPEG4_Audio_ISO_IEC_14496_3 &&  entry->codec.mp4a.mpeg4.decoder.iso_14496_3.audioObjectType == 36) {
                entry->sample_type = bmp4_sample_type_als;
            }
            break;
        case bmp4_sample_type_mp4v:
            if(!b_mp4_parse_sample_mp4v(&cursor, &entry->codec.mp4v)) {
                entry->sample_type = bmp4_sample_type_unknown;
            }
            break;
        case bmp4_sample_type_s263:
            if(!b_mp4_parse_sample_s263(&cursor, &entry->codec.s263)) {
                entry->sample_type = bmp4_sample_type_unknown;
            }
            break;
        case bmp4_sample_type_ac3:
        case bmp4_sample_type_eac3:
            if(!bmp4_parse_audiosampleentry(&cursor, &entry->codec.ac3.audio)) {
                entry->sample_type = bmp4_sample_type_unknown;
            }
            break;
        case bmp4_sample_type_samr:
        case bmp4_sample_type_sawb:
        case bmp4_sample_type_sawp:
            entry_data_size = batom_cursor_pos(&cursor);
            if(  entry_data_size >= start_pos && entry_box.size > (entry_data_size - start_pos) ) {
                entry_data_size = entry_box.size - (entry_data_size - start_pos);
            }
            if(!b_mp4_parse_sample_amr(&cursor, &entry->codec.amr, entry_data_size)) {
                entry->sample_type = bmp4_sample_type_unknown;
            }
            break;
        case bmp4_sample_type_qt_ima_adpcm:
            if(!b_mp4_parse_sample_ms(&cursor, &entry->codec.ms)) {
                entry->sample_type = bmp4_sample_type_unknown;
            }
            break;
        case bmp4_sample_type_qt_ima4_adpcm:
            if(!b_mp4_parse_sample_ima4(&cursor, &entry->codec.ima4)) {
                entry->sample_type = bmp4_sample_type_unknown;
            }
            break;
        case bmp4_sample_type_mjpeg:
            if(!b_mp4_parse_sample_mjpeg(&cursor, &entry->codec.mjpeg, entry_box.type)) {
                entry->sample_type = bmp4_sample_type_unknown;
            }
            break;
        case bmp4_sample_type_twos:
            if(!b_mp4_parse_sample_twos(&cursor, &entry->codec.twos, entry_box.type)) {
                entry->sample_type = bmp4_sample_type_unknown;
            }
            break;
        case bmp4_sample_type_dts:
            if(!b_mp4_parse_sample_dts(&cursor, &entry->codec.dts, entry_box.type)) {
                entry->sample_type = bmp4_sample_type_unknown;
            }
            break;
        case bmp4_sample_type_hevc:
            if(!b_mp4_parse_sample_hevc(&cursor, &entry->codec.hevc, &entry->codecprivate)) {
                entry->sample_type = bmp4_sample_type_unknown;
            }
            break;
        case bmp4_sample_type_mp3:
            if(!bmp4_parse_audiosampleentry(&cursor, &entry->codec.mp3.audio)) {
                entry->sample_type = bmp4_sample_type_unknown;
            }
            break;
        default:
            BDBG_WRN(("bmp4_parse_sample: unknown sample " B_MP4_TYPE_FORMAT " for handler " B_MP4_TYPE_FORMAT , B_MP4_TYPE_ARG(entry_box.type), B_MP4_TYPE_ARG(handler_type)));
            break;
        }
        end_pos = batom_cursor_pos(&cursor);
        BDBG_ASSERT(end_pos>=start_pos);
        end_pos -= start_pos;
        if(entry_box.size>=end_pos) {
            batom_cursor_skip(&cursor, entry_box.size - end_pos);
        } else {
            BDBG_WRN(("bmp4_parse_sample: error while parsing entry"));
            entry->sample_type = bmp4_sample_type_unknown;
        }
        sample->entries[i] = entry;
    }

error_sample:
    sample->entry_count = i;
    return true;
}

void
bmp4_free_sample_info(bmp4_sample_info *sample)
{
    unsigned i;
    BDBG_ASSERT(sample);

    for(i=0;i<sample->entry_count;i++) {
        bmp4_sampleentry *entry = sample->entries[i];

        switch(entry->sample_type) {
        case bmp4_sample_type_hevc:
            bmedia_shutdown_h265_meta(&entry->codec.hevc.meta, bkni_alloc);
            break;
        default:
            break;
        }
        BKNI_Free(sample->entries[i]);
    }
    sample->entry_count = 0;
    return;
}

bool
bmp4_parse_sample_size_header(batom_cursor *cursor, bmp4_sample_size_header *header)
{
    if(!bmp4_parse_fullbox(cursor, &header->fullbox)) {
        return false;
    }
    /* Page 30 */
    header->default_sample_size = batom_cursor_uint32_be(cursor);
    header->sample_count = batom_cursor_uint32_be(cursor);
    return !BATOM_IS_EOF(cursor);
}

bool
bmp4_parse_compact_sample_size_header(batom_cursor *cursor, bmp4_compact_sample_size_header *header)
{
    uint32_t reserved;
    if(!bmp4_parse_fullbox(cursor, &header->fullbox)) {
        return false;
    }
    /* Page 30 */
    reserved = batom_cursor_uint24_be(cursor);
    header->field_size = batom_cursor_byte(cursor);
    header->sample_count = batom_cursor_uint32_be(cursor);
    return !BATOM_IS_EOF(cursor);
}

#if 0
/* unused functions */
bool 
bmp4_parse_sample_size(batom_t box, bmp4_sample_size *sample)
{
	batom_cursor cursor;
	bool valid;
    bmp4_sample_size_header header;

	BDBG_ASSERT(box);
	BDBG_ASSERT(sample);

	batom_cursor_from_atom(&cursor, box);

	if(!bmp4_parse_sample_size_header(&cursor, &header)) {
		return false;
	}
    sample->sample_count = header.sample_count;
    if (!header.default_sample_size) {
        /* We currently do not need the sample table */
        batom_cursor_skip(&cursor, sample->sample_count*sizeof(uint32_t));
    }
    valid = sample->sample_count>0 && (!BATOM_IS_EOF(&cursor));
	if(valid) {
		BDBG_MSG(("bmp4_parse_sample_size: default_sample_size=%u sample_count=%u", header.default_sample_size, sample->sample_count));
	}

	return valid;
}

bool 
bmp4_parse_compact_sample_size(batom_t box, bmp4_sample_size *sample)
{
	batom_cursor cursor;
	bool valid;
    bmp4_compact_sample_size_header header;

	BDBG_ASSERT(box);
	BDBG_ASSERT(sample);


	batom_cursor_from_atom(&cursor, box);
    if(!bmp4_parse_compact_sample_size_header(&cursor, &header)) {
        return false;
    }

    /* Page 30 */
    sample->sample_count = header.sample_count;

    /* Valid feild size values are 16, 8 and 4. If field size is 4 with an odd number of samples, then the last byte is padded. */
    batom_cursor_skip(&cursor, (sample->sample_count*header.field_size + (header.field_size & 0x4))/8);

    valid = sample->sample_count>0 && (!BATOM_IS_EOF(&cursor));
	if(valid) {
		BDBG_MSG(("bmp4_parse_sample_size: sample_count=%u", sample->sample_count));
	}

	return valid;
}
#endif
/* page 47 */
bool 
bmp4_parse_trackextends(batom_t box, bmp4_trackextendsbox *track_extends)
{
    batom_cursor cursor;
	bmp4_fullbox fullbox;

    batom_cursor_from_atom(&cursor, box);

	if(!bmp4_parse_fullbox(&cursor, &fullbox)) {
		return false;
	}
    track_extends->track_ID = batom_cursor_uint32_be(&cursor);
    track_extends->default_sample_description_index = batom_cursor_uint32_be(&cursor);
    track_extends->default_sample_duration = batom_cursor_uint32_be(&cursor);
    track_extends->default_sample_size = batom_cursor_uint32_be(&cursor);
    track_extends->default_sample_flags = batom_cursor_uint32_be(&cursor);
    return !BATOM_IS_EOF(&cursor);
}

bool 
bmp4_parse_movie_fragment_header(batom_cursor *cursor, bmp4_movie_fragment_header *header)
{
    /* page 39 */
	if(!bmp4_parse_fullbox(cursor, &header->fullbox)) {
		return false;
	}
    header->sequence_number = batom_cursor_uint32_be(cursor);
    return !BATOM_IS_EOF(cursor);
}

bool 
bmp4_parse_track_fragment_header(batom_cursor *cursor, bmp4_track_fragment_header *header)
{
    BKNI_Memset(header, 0, sizeof(*header));
    /* page 40 */
	if(!bmp4_parse_fullbox(cursor, &header->fullbox)) {
		return false;
	}
    header->track_ID = batom_cursor_uint32_be(cursor);
    if(header->fullbox.flags&0x0001) {
        header->validate.base_data_offset = true;
        header->base_data_offset = batom_cursor_uint64_be(cursor);
    }
    if(header->fullbox.flags&0x0002) {
        header->validate.sample_description_index = true;
        header->sample_description_index = batom_cursor_uint32_be(cursor);
    }
    if(header->fullbox.flags&0x0008) { 
        header->validate.default_sample_duration = true;
        header->default_sample_duration = batom_cursor_uint32_be(cursor);
    }
    if(header->fullbox.flags&0x0010) { 
        header->validate.default_sample_size = true;
        header->default_sample_size = batom_cursor_uint32_be(cursor);
    }
    if(header->fullbox.flags&0x0020) { 
        header->validate.default_sample_flags = true;
        header->default_sample_flags = batom_cursor_uint32_be(cursor);
    }
    return !BATOM_IS_EOF(cursor);
}

bool 
bmp4_parse_track_fragment_run_header(batom_cursor *cursor, bmp4_track_fragment_run_header  *run_header)
{
    BKNI_Memset(run_header, 0, sizeof(*run_header));
    /* pages 41,42 */
	if(!bmp4_parse_fullbox(cursor, &run_header->fullbox)) {
		return false;
	}
    run_header->sample_count = batom_cursor_uint32_be(cursor);
    if(run_header->fullbox.flags&0x001) {
        run_header->validate.data_offset = true;
        run_header->data_offset = batom_cursor_uint32_be(cursor);
    }
    if(run_header->fullbox.flags&0x004) {
        run_header->validate.first_sample_flags= true;
        run_header->first_sample_flags = batom_cursor_uint32_be(cursor);
    }
    BDBG_MSG(("bmp4_parse_track_fragment_run_header: %#lx %s fullbox.flags:%#x sample_count:%u", (unsigned long)cursor, BATOM_IS_EOF(cursor)?"EOF":"", run_header->fullbox.flags, run_header->sample_count));
    return !BATOM_IS_EOF(cursor);
}

void 
bmp4_init_track_fragment_run_state(bmp4_track_fragment_run_state *state)
{
    state->sample_no = 0;
    state->accumulated_size = 0;
    state->accumulated_time = 0;
    return;
}

bool bmp4_parse_track_fragment_run_sample(batom_cursor *cursor, const bmp4_track_fragment_header *fragment_header, const bmp4_track_fragment_run_header *run_header, const bmp4_trackextendsbox *track_extends, bmp4_track_fragment_run_state *state, bmp4_track_fragment_run_sample *sample)
{
    if(state->sample_no>=run_header->sample_count) {
        return false;
    }
    sample->time = state->accumulated_time;
    sample->offset = state->accumulated_size;
    if(run_header->fullbox.flags&0x0100) {
        sample->duration = batom_cursor_uint32_be(cursor);
    } else if (fragment_header->validate.default_sample_duration) {
        sample->duration = fragment_header->default_sample_duration;
    } else {
        sample->duration = track_extends->default_sample_duration;
    }

    if(run_header->fullbox.flags&0x0200) {
        sample->size = batom_cursor_uint32_be(cursor);
    } else if(fragment_header->validate.default_sample_size) {
        sample->size = fragment_header->default_sample_size;
    } else {
        sample->size = track_extends->default_sample_size;
    }

    if(run_header->fullbox.flags&0x0400) {
        sample->flags = batom_cursor_uint32_be(cursor);
    } else if(state->sample_no==0 && run_header->validate.first_sample_flags) {
        sample->flags = run_header->first_sample_flags;
    } else if(fragment_header->validate.default_sample_flags) {
        sample->flags = fragment_header->default_sample_flags;
    } else {
        sample->flags= track_extends->default_sample_flags;
    }

    if(run_header->fullbox.flags&0x0800) {
        sample->composition_time_offset = batom_cursor_uint32_be(cursor);
    } else {
        sample->composition_time_offset = 0;
    }
    if(!BATOM_IS_EOF(cursor)) {
        state->sample_no++;
        state->accumulated_size += sample->size;
        state->accumulated_time += sample->duration;
        return true;
    }
    return false;
}


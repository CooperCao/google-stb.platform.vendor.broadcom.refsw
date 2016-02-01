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
 * MKV container file_parser library
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bmkv_file_parser.h"
#include "bkni.h"

BDBG_MODULE(bmkv_file_parser);

#define BDBG_MSG_TRACE(x)   BDBG_MSG(x)

BDBG_OBJECT_ID(bmkv_file_parser_t);

static bmkv_parser_action
b_mkv_file_parser_element_begin(void *cntx, bmkv_id id, uint64_t size, uint64_t offset)
{
	bmkv_file_parser_t file_parser = cntx;
	bfile_segment *segment = NULL;

	BDBG_MSG_TRACE(("%s:%#lx %#x %lu bytes at %#lx", "b_mkv_file_parser_object_begin", (unsigned long)cntx, (unsigned)id, (unsigned long)size, (unsigned long)offset));
	BDBG_OBJECT_ASSERT(file_parser, bmkv_file_parser_t);
	switch(id) {
	case BMKV_ATTACHMENT_ID: /* attachment */
		BDBG_MSG(("%s:%#lx found %s at %#lx:%lu", "b_mkv_file_parser_object_begin", (unsigned long)cntx, "Attachment", (unsigned long)size, (unsigned long)offset));
		segment = &file_parser->attachment;
   		file_parser->next_seek = offset+size;
		break;
	case BMKV_CHAPTERS_ID: /* chapter */
		BDBG_MSG(("%s:%#lx found %s at %#lx:%lu", "b_mkv_file_parser_object_begin", (unsigned long)cntx, "Chapters", (unsigned long)size, (unsigned long)offset));
		segment = &file_parser->chapters;
   		file_parser->next_seek = offset+size;
		break;
	case BMKV_VOID_ID:
	case BMKV_CLUSTER_ID:
		file_parser->next_seek = offset+size;
		break;
	case BMKV_SEGMENT_ID:
		BDBG_MSG(("%s:%#lx found %s at %#lx:%lu", "b_mkv_file_parser_object_begin", (unsigned long)cntx, "Segment", (unsigned long)size, (unsigned long)offset));
		segment = &file_parser->segment;
		break;
	case BMKV_SEEKHEAD_ID:
		if(file_parser->cluster_seekhead.len==0) {
			BDBG_MSG(("%s:%#lx found %s at %#lx:%lu", "b_mkv_file_parser_object_begin", (unsigned long)cntx, "First SeekHead", (unsigned long)size, (unsigned long)offset));
			segment = &file_parser->cluster_seekhead;
		}
		break;
	}
	if(segment) {
		segment->start = offset;
		segment->len = size;
	}
	return bmkv_parser_action_none;
}

static bmkv_parser_action
b_mkv_file_parser_element_end(void *cntx, bmkv_id id, uint64_t offset)
{
	bmkv_file_parser_t file_parser = cntx;

	BSTD_UNUSED(offset);
	BDBG_MSG(("b_mkv_file_parser_object_end:%#lx %#x at %#lx", (unsigned long)cntx, (unsigned)id, (unsigned long)offset));
	BDBG_OBJECT_ASSERT(file_parser, bmkv_file_parser_t);
	if(id==file_parser->seek_to_element) {
		file_parser->seek_to_element = 0; /* seek completed */
		BDBG_MSG(("b_mkv_file_parser_object_end:%#lx seek completed %#x at %#lx", (unsigned long)cntx, (unsigned)id, (unsigned long)offset));
	}
	if(id==BMKV_SEGMENT_ID) { /* stop parsing if reached segment end */
		file_parser->segment_end = true;
		return bmkv_parser_action_return;
	}
	return bmkv_parser_action_none;
}

static bmkv_parser_action
b_mkv_file_parser_ebml(bmkv_parser_handler *handler, bmkv_id id, batom_t element)
{
	bmkv_file_parser_t file_parser = ((b_mkv_file_parser_handler *)handler)->file_parser;
	bmkv_parser_action action = bmkv_parser_action_none;

	BSTD_UNUSED(id);
	BDBG_MSG(("%s:%p %#x[%#x] %u bytes", "b_mkv_file_parser_ebml", (void *)file_parser, handler->id, id, element?(unsigned)batom_len(element):0));
	BDBG_OBJECT_ASSERT(file_parser, bmkv_file_parser_t);
	if(file_parser->validate.ebml) {
		BDBG_WRN(("%s:%p duplicate EBML header, multivolume stream", "b_mkv_file_parser_ebml", (void *)file_parser));
		file_parser->next_volume = true;
	    action = bmkv_parser_action_return;
		goto done;
	}
	if(element) {
    	bmkv_EBML ebml;
    	batom_cursor cursor;
    	batom_cursor_from_atom(&cursor, element);
		if(bmkv_EBML_parse(&cursor, batom_len(element), &ebml)) {
			if(bmkv_EBML_validate(&ebml)) {
                BDBG_CASSERT(sizeof(file_parser->docType) == sizeof(ebml.DocType));
                BKNI_Memcpy(file_parser->docType, ebml.DocType, sizeof(file_parser->docType));
				file_parser->validate.ebml = true;
				BDBG_MSG(("%s:%#lx recognized format %s:%u [%u,%u]", "b_mkv_file_parser_ebml", (unsigned long)file_parser, ebml.DocType, ebml.EBMLReadVersion, ebml.EBMLMaxIDLength, ebml.EBMLMaxSizeLength));
			}
		} else {
			BDBG_WRN(("%s:%#lx corrupted %s element, mark stream as invalid", "b_mkv_file_parser_ebml", (unsigned long)file_parser, "EBML"));
			file_parser->stream_error = true;
		}
	}
done:
	if(element) {
		batom_release(element);
	}
	return action;
}


static bmkv_parser_action
b_mkv_file_parser_seekhead(bmkv_parser_handler *handler, bmkv_id id, batom_t element)
{
	bmkv_file_parser_t file_parser = ((b_mkv_file_parser_handler *)handler)->file_parser;
	bmkv_parser_action action = bmkv_parser_action_none;

	BSTD_UNUSED(id);
	BDBG_MSG(("%s:%p %#x[%#x] %u bytes", "b_mkv_file_parser_seekhead", (void *)file_parser, handler->id, id, element?(unsigned)batom_len(element):0));
	BDBG_OBJECT_ASSERT(file_parser, bmkv_file_parser_t);
	if(element) {
    	batom_cursor cursor;
    	batom_cursor_from_atom(&cursor, element);
		if(!file_parser->validate.seekhead) {
			file_parser->validate.seekhead = bmkv_SeekHead_parse(&cursor, batom_len(element), &file_parser->seekhead);
			if(!file_parser->validate.seekhead) {
				bmkv_SeekHead_shutdown(&file_parser->seekhead);
			}
		}
		batom_release(element);
	}
	return action;
}

static bmkv_parser_action
b_mkv_file_parser_tracks(bmkv_parser_handler *handler, bmkv_id id, batom_t element)
{
	bmkv_file_parser_t file_parser = ((b_mkv_file_parser_handler *)handler)->file_parser;
	bmkv_parser_action action = bmkv_parser_action_none;

	BSTD_UNUSED(id);
	BDBG_MSG(("%s:%p %#x[%#x] %u bytes", "b_mkv_file_parser_tracks", (void *)file_parser, handler->id, id, element?(unsigned)batom_len(element):0));
	BDBG_OBJECT_ASSERT(file_parser, bmkv_file_parser_t);
	if(element) {
        if(!file_parser->validate.tracks) {
            batom_cursor cursor;
            batom_cursor_from_atom(&cursor, element);
            file_parser->validate.tracks = bmkv_Tracks_parse(&cursor, batom_len(element), &file_parser->trackData);
            if(!file_parser->validate.tracks) {
                BDBG_WRN(("%s:%#lx corrupted %s element, mark stream as invalid", "b_mkv_file_parser_tracks", (unsigned long)file_parser, "Tracks"));
                file_parser->stream_error = true;
                bmkv_Tracks_shutdown(&file_parser->trackData);
            }
        }
		batom_release(element);
	}
	return action;
}

static bmkv_parser_action
b_mkv_file_parser_segment_info(bmkv_parser_handler *handler, bmkv_id id, batom_t element)
{
	bmkv_file_parser_t file_parser = ((b_mkv_file_parser_handler *)handler)->file_parser;
	bmkv_parser_action action = bmkv_parser_action_none;

	BSTD_UNUSED(id);
	BDBG_MSG(("%s:%p %#x[%#x] %u bytes", "b_mkv_file_parser_segment_info", (void *)file_parser, handler->id, id, element?(unsigned)batom_len(element):0));
	BDBG_OBJECT_ASSERT(file_parser, bmkv_file_parser_t);
	if(element) {
        if(!file_parser->validate.segment_info) {
            batom_cursor cursor;
            batom_cursor_from_atom(&cursor, element);
            file_parser->validate.segment_info = bmkv_SegmentInfo_parse(&cursor, batom_len(element), &file_parser->segment_info);
            if(!file_parser->validate.segment_info) {
                BDBG_WRN(("%s:%p corrupted %s element, mark stream as invalid", "b_mkv_file_parser_segment_info", (void *)file_parser, "Segment Info"));
                file_parser->stream_error = true;
                bmkv_SegmentInfo_shutdown(&file_parser->segment_info);
            }
        }
		batom_release(element);
	}
	return action;
}

static void
b_mkv_file_parser_clear(bmkv_file_parser_t file_parser)
{
	file_parser->next_seek = 0;
	file_parser->stream_error = false;
    file_parser->next_volume = false;
	file_parser->segment_end = false;
	file_parser->ebml_handler.file_parser = file_parser;
	file_parser->seekhead_handler.file_parser = file_parser;
	file_parser->tracks_handler.file_parser = file_parser;
	file_parser->segment_info_handler.file_parser = file_parser;
	file_parser->validate.ebml = false;
	file_parser->validate.seekhead = false;
	file_parser->validate.tracks = false;
	file_parser->validate.segment_info = false;
	file_parser->seek_to.tracks = false;
	file_parser->seek_to.segment_info = false;
	file_parser->seek_to.attachment = false;
	file_parser->seek_to.chapters = false;
	file_parser->seek_to_element = 0;
	file_parser->cluster_seekhead.len=0;

	bfile_segment_clear(&file_parser->segment);
	bfile_segment_clear(&file_parser->attachment);
	bfile_segment_clear(&file_parser->chapters);

	return;
}

void
bmkv_file_parser_release(bmkv_file_parser_t file_parser)
{
	BDBG_OBJECT_ASSERT(file_parser, bmkv_file_parser_t);
	if(file_parser->validate.segment_info) {
		file_parser->validate.segment_info = false;
		bmkv_SegmentInfo_shutdown(&file_parser->segment_info);
	}
	if(file_parser->validate.tracks) {
		file_parser->validate.tracks = false;
		bmkv_Tracks_shutdown(&file_parser->trackData);
	}
	if(file_parser->validate.seekhead) {
		file_parser->validate.seekhead = false;
		bmkv_SeekHead_shutdown(&file_parser->seekhead);
	}
	return;
}

int
bmkv_file_parser_init(bmkv_file_parser_t file_parser, batom_factory_t factory)
{
	bmkv_parser_cfg cfg;

	BDBG_OBJECT_INIT(file_parser, bmkv_file_parser_t);
	b_mkv_file_parser_clear(file_parser);
	bmkv_parser_default_cfg(&cfg);
	cfg.user_cntx = file_parser;
	cfg.element_begin = b_mkv_file_parser_element_begin;
	cfg.element_end = b_mkv_file_parser_element_end;
	file_parser->mkv = bmkv_parser_create(factory, &cfg);

	if(!file_parser->mkv) {
		goto err_mkv;
	}
	bmkv_parser_install_handler(file_parser->mkv, &file_parser->ebml_handler.handler, BMKV_EBML_ID, b_mkv_file_parser_ebml);
	bmkv_parser_install_handler(file_parser->mkv, &file_parser->seekhead_handler.handler, BMKV_SEEKHEAD_ID, b_mkv_file_parser_seekhead);
	bmkv_parser_install_handler(file_parser->mkv, &file_parser->tracks_handler.handler, BMKV_TRACKS_ID, b_mkv_file_parser_tracks);
	bmkv_parser_install_handler(file_parser->mkv, &file_parser->segment_info_handler.handler, BMKV_SEGMENTINFO_ID, b_mkv_file_parser_segment_info);
	file_parser->config.attachment = false;
	file_parser->config.chapters = false;
	return 0;
err_mkv:
	return -1;
}

void
bmkv_file_parser_shutdown(bmkv_file_parser_t file_parser)
{
	BDBG_OBJECT_ASSERT(file_parser, bmkv_file_parser_t);
	bmkv_parser_remove_handler(file_parser->mkv, &file_parser->ebml_handler.handler);
	bmkv_parser_remove_handler(file_parser->mkv, &file_parser->segment_info_handler.handler);
	bmkv_parser_remove_handler(file_parser->mkv, &file_parser->tracks_handler.handler);
	bmkv_parser_remove_handler(file_parser->mkv, &file_parser->seekhead_handler.handler);
	bmkv_parser_destroy(file_parser->mkv);
	return;
}

int
bmkv_file_parser_parse(bmkv_file_parser_t file_parser, bfile_buffer_t file_buf, batom_pipe_t pipe_mkv)
{
	size_t read_len = 4096;
	int rc=-1;
	bool first_packet;
	uint64_t next_seek;
	unsigned i;
	off_t off;
    bool used_seek_head = false;

    b_mkv_file_parser_clear(file_parser);

	for(off=0,first_packet=true;;) {
		size_t mkv_result;
		bool force_seek = false;
		bfile_buffer_result result;
		batom_t atom;
		size_t atom_len;

		BDBG_MSG(("%s: %p reading %u:%u", "b_mkv_file_parser_parse", (void *)file_parser, (unsigned)off, (unsigned)read_len));
		atom = bfile_buffer_read(file_buf, off, read_len, &result);
		if(atom==NULL) {
			break;
		}
		atom_len = batom_len(atom);
		BDBG_MSG(("%s: %px read %u:%u -> %p", "b_mkv_file_parser_parse", (void *)file_parser, (unsigned)off, (unsigned)atom_len, (void *)atom));
		if(atom_len==0) {
            batom_release(atom);
			break;
		}
		off += atom_len;
		batom_pipe_push(pipe_mkv, atom);
		mkv_result = bmkv_parser_feed(file_parser->mkv, pipe_mkv);
		if(first_packet) {
			first_packet = false;
			if(!file_parser->validate.ebml) {
				BDBG_WRN(("%s: %#lx unknown/not supported format", "b_mkv_file_parser_open_file", (unsigned long) file_parser));
				goto err_format;
			}
		}
		if(file_parser->validate.tracks && 
		   file_parser->validate.segment_info && 
		   used_seek_head && /* Ensure we gotten whatever we can from the seek head */
		   (!file_parser->config.attachment || file_parser->attachment.len>0 ) &&
		   (!file_parser->config.chapters || file_parser->chapters.len>0 )
		   ) {
			break; /* enough data captured for playback */
		}
		if(file_parser->segment_end) {
			goto err_end_segment; /* end of segment, but no usefull data */
		}
		if(mkv_result==0) {
			BDBG_WRN(("%s: %#lx unexpected stall in parsing", "b_mkv_file_parser_open_file", (unsigned long) file_parser));
			goto err_file_parser;
		}
		if(file_parser->seek_to_element) {
			continue; /* we currently capture element  what we've seeked to */
		}
		if(file_parser->validate.seekhead) {

			for(i=0;i<file_parser->seekhead.Seek.nelems;i++) {
				bmkv_SeekElement *entry = &BMKV_TABLE_ELEM(file_parser->seekhead.Seek,bmkv_SeekElement,i);
				if(!(entry->validate.SeekPosition && entry->validate.SeekID)) {
					continue;
				}
				switch(entry->SeekID) {
				case BMKV_TRACKS_ID:
					if(!file_parser->seek_to.tracks) {
						file_parser->seek_to.tracks = true;
						force_seek = true;
						BDBG_MSG(("%s: %#lx seeking to %s", "b_mkv_file_parser_open_file", (unsigned long) file_parser, "Tracks"));
					}
					break;
				case BMKV_SEGMENTINFO_ID:
					if(!file_parser->seek_to.segment_info) {
						file_parser->seek_to.segment_info = true;
						force_seek = true;
						BDBG_MSG(("%s: %#lx seeking to %s", "b_mkv_file_parser_open_file", (unsigned long) file_parser, "Segment Information"));
					}
					break;
				case BMKV_ATTACHMENT_ID:
					if(!file_parser->seek_to.attachment) {
						file_parser->seek_to.attachment = true;
						force_seek = true;
						BDBG_MSG(("%s: %#lx seeking to %s", "b_mkv_file_parser_open_file", (unsigned long) file_parser, "Attachment"));
					}
					break;
				case BMKV_CHAPTERS_ID:
					if(!file_parser->seek_to.chapters) {
						file_parser->seek_to.chapters = true;
						force_seek = true;
						BDBG_MSG(("%s: %#lx seeking to %s", "b_mkv_file_parser_open_file", (unsigned long) file_parser, "Chapters"));
					}
					break;
				}
				if(force_seek) {
					file_parser->seek_to_element = entry->SeekID;
					file_parser->next_seek = entry->SeekPosition + file_parser->segment.start;
					break;
				}
			}
			if(!force_seek) {
                used_seek_head = true; /* there is nothing else to get from the file */
			}
		}
		if(file_parser->next_seek>0) {
			bmkv_parser_status status;

			next_seek = file_parser->next_seek;
			file_parser->next_seek=0;
			bmkv_parser_get_status(file_parser->mkv, &status);
			if(force_seek || next_seek>(status.offset+status.acc_length)) { /* only skip forward to jump through junk data */

				batom_pipe_flush(pipe_mkv);
				BDBG_MSG(("%s:%p seek to " B_OFFT_FMT " (skip " B_OFFT_FMT "", "b_mkv_file_parser_open_file", (void *)file_parser, B_OFFT_ARG(next_seek), B_OFFT_ARG(next_seek-(status.offset+status.acc_length)) ));
				off = next_seek;
				bmkv_parser_seek(file_parser->mkv, next_seek); /* skip current element */
			}
		}
	}
	if(file_parser->validate.tracks && file_parser->validate.segment_info) {
		rc = 0;
	}
err_file_parser:
err_end_segment:
err_format:
	bmkv_parser_reset(file_parser->mkv);
	batom_pipe_flush(pipe_mkv);
	return rc;
}


void 
bmkv_file_parser_process_encoding_info(const bmkv_TrackEntry *mkv_track, bmkv_file_encoding_info *info)
{
    const bmkv_ContentEncodings *encodings;
    const bmkv_ContentEncoding *encoding;
    const bmkv_ContentCompression *compression;

    BKNI_Memset(info, 0, sizeof(*info));
    info->present = false;
    info->supported = true;
    info->ContentCompAlgo_valid = false;
    if(!mkv_track->validate.ContentEncodings) {
        return;
    }
    if(mkv_track->ContentEncodings.nelems==0) {
        return;
    }
    info->present = true;
    info->supported = false;
    if(mkv_track->ContentEncodings.nelems!=1) {
        return;
    }
    encodings = &BMKV_TABLE_ELEM(mkv_track->ContentEncodings,bmkv_ContentEncodings, 0);
    if(!encodings->validate.ContentEncoding) {
        return;
    }
    if(encodings->ContentEncoding.nelems!=1) {
        return;
    }
    encoding = &BMKV_TABLE_ELEM(encodings->ContentEncoding,bmkv_ContentEncoding, 0);
    if(encoding->validate.ContentEncodingType && encoding->ContentEncodingType!=0) {
        return;
    }
    if(encoding->validate.ContentEncodingScope && encoding->ContentEncodingScope!=0x01) {
        return;
    }
    if(encoding->validate.ContentCompression && encoding->ContentCompression.nelems!=1) {
        return;
    }
    compression = &BMKV_TABLE_ELEM(encoding->ContentCompression,bmkv_ContentCompression, 0);
    if(!compression->validate.ContentCompAlgo) {
        return;
    }
    info->ContentCompAlgo_valid = true;
    info->ContentCompAlgo = compression->ContentCompAlgo;
    if(compression->validate.ContentCompSettings) {
        info->supported = compression->ContentCompAlgo==3;
        info->ContentCompSettings = compression->ContentCompSettings;
    }
    return;
}

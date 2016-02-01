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
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bmedia_filter.h"
#include "bmedia_probe.h"
#include "bfile_stdio.h"
#include "bfile_util.h"

#include "subtitle_control.h"

#include <stdlib.h>
#include <string.h>

BDBG_MODULE(bsubtitle_control);

titles_list titles;

b_titles_info *cur_title_info=NULL;
b_stitle_info *cur_subtitle_info=NULL;
b_track_info *cur_track_info=NULL;
b_edition_info *cur_edition_info=NULL;
b_chapter_info *cur_chapter_info=NULL;


void
bstitle_parse_get_text_offset(const bmedia_probe_track *track, bstitle_parse_t stitle_parse);

bstitle_parse_t
bstitle_parse_init(char *file, b_stitle_cfg_params *cfg_params)
{
	bmedia_probe_t probe;
	FILE *fin;
	bfile_io_read_t fd, orig_fd = NULL;
	const bmedia_probe_stream *stream;
	const bmedia_probe_track *track;
	bmedia_probe_config probe_config;
	char stream_info[512];
	int audio_trk_num, video_trk_num, track_no;
	b_titles_info *title_info = NULL;
	b_track_info *track_info = NULL;
	b_stitle_info *subtitle_info = NULL;	
	bool multi_title, trick_play_track;
	off_t title_offset=0;
	
	BLST_SQ_INIT(&titles);

	probe = bmedia_probe_create();

	fin = fopen(file,"rb");
	if (!fin) return NULL;

	fd = bfile_stdio_read_attach(fin);

probe_next_title:
	bmedia_probe_default_cfg(&probe_config);
	probe_config.file_name = file;
	probe_config.stream_specific.mkv.probe_next_volume = true;
	stream = bmedia_probe_parse(probe, fd, &probe_config);
	
	audio_trk_num = video_trk_num = track_no= 0;		
	multi_title = false;
	trick_play_track = false;

	if(stream) {
		bmedia_stream_to_string(stream, stream_info, sizeof(stream_info));
		BDBG_WRN(("%s: %s", file, stream_info));
		
		title_info = BKNI_Malloc(sizeof(*title_info));
		BKNI_Memset(title_info, 0, sizeof(*title_info));
		title_info->file_offset = title_offset;
		title_info->last_display_time = 0;
		track_info = &title_info->track_info;
		BKNI_Memset(track_info, 0, sizeof(*track_info));
		track_info->type = stream->type;
		subtitle_info = &title_info->subtitle_info;	

		for(track=BLST_SQ_FIRST(&stream->tracks);track;track=BLST_SQ_NEXT(track, link)) {			
			switch(track->type) {
			case bmedia_track_type_video:					
				if(track->info.video.codec == bvideo_codec_unknown){
					subtitle_info->stitle_parse[track_no] = BKNI_Malloc(sizeof(struct bstitle_parse)); /*Free up this memory on exit*/
					if (!subtitle_info->stitle_parse[track_no]) {
						BDBG_WRN(("bstitle_parse_init: out of memory %u", sizeof(struct bstitle_parse)));
						goto error;
					}				
					subtitle_info->stitle_parse[track_no]->width = track->info.video.width;
					subtitle_info->stitle_parse[track_no]->height = track->info.video.height;
					subtitle_info->stitle_parse[track_no]->subtitle_type = ((bavi_probe_track *)track)->subtitle_type;			
					subtitle_info->stitle_parse[track_no]->track_number = track->number;
					subtitle_info->stitle_parse[track_no]->track_index = track_no+1;					
					subtitle_info->stitle_parse[track_no]->activated = false;
					subtitle_info->stitle_parse[track_no]->cfg_params = cfg_params;
					subtitle_info->stitle_parse[track_no]->stitle_info = subtitle_info;
					subtitle_info->stitle_parse[track_no]->font.data = NULL;
					track_no++;
				}
				else {
					track_info->video_info[video_trk_num].track_id = track->number;
					track_info->video_info[video_trk_num].codec = track->info.video.codec;
					track_info->video_info[video_trk_num].width = track->info.video.width;
					track_info->video_info[video_trk_num].height = track->info.video.height;
					if(stream->type == bstream_mpeg_type_mkv){
						track_info->video_info[video_trk_num].track_uid = ((bmkv_probe_track *)track)->TrackUID;
						track_info->video_info[video_trk_num].trick_track_uid = ((bmkv_probe_track *)track)->TrickTrackUID;						
						track_info->video_info[video_trk_num].master_track_uid = ((bmkv_probe_track *)track)->MasterTrackUID;
						track_info->video_info[video_trk_num].trick_track_flag = ((bmkv_probe_track *)track)->TrickTrackFlag;
						
						track_info->video_info[video_trk_num].display_width = ((bmkv_probe_track *)track)->DisplayWidth;
						track_info->video_info[video_trk_num].display_height = ((bmkv_probe_track *)track)->DisplayHeight;
					}
					video_trk_num++;
				}
				BDBG_MSG(("video track %u codec:%u width %d height %d", track->number, track->info.video.codec, track->info.video.width, track->info.video.height));
				break;
			case bmedia_track_type_audio:
				track_info->audio_info[audio_trk_num].track_id = track->number;
				track_info->audio_info[audio_trk_num].codec = track->info.audio.codec;
				audio_trk_num++;
				break;
			case bmedia_track_type_other:
				subtitle_info->stitle_parse[track_no] = BKNI_Malloc(sizeof(struct bstitle_parse)); /*Free up this memory on exit*/
				if (!subtitle_info->stitle_parse[track_no]) {
					BDBG_WRN(("bstitle_parse_init: out of memory %u", sizeof(struct bstitle_parse)));
					goto error;
				}	
				subtitle_info->stitle_parse[track_no]->track_number = track->number;
				subtitle_info->stitle_parse[track_no]->track_index = track_no+1;					
				subtitle_info->stitle_parse[track_no]->activated = false;
				subtitle_info->stitle_parse[track_no]->cfg_params = cfg_params;
				subtitle_info->stitle_parse[track_no]->stitle_info = subtitle_info;
				subtitle_info->stitle_parse[track_no]->width = 64;
				subtitle_info->stitle_parse[track_no]->height = 32;
				subtitle_info->stitle_parse[track_no]->txt_subtitle_position = 0;
				subtitle_info->stitle_parse[track_no]->font.data = NULL;
				switch(stream->type) {
				case bstream_mpeg_type_mkv:
					if(((bmkv_probe_track *)track)->CodecPrivate_len){
						bstitle_parse_get_text_offset(track, subtitle_info->stitle_parse[track_no]);
					}
					if(((bmkv_probe_track *)track)->AttachmentLink_valid){
						bmkv_probe_stream *mkv_stream = (bmkv_probe_stream *)stream;
						bmkv_Attachment *attachment;
						for(attachment=BLST_SQ_FIRST(&mkv_stream->attachments);attachment;attachment=BLST_SQ_NEXT(attachment, link)){						
							if(attachment->FileUID == ((bmkv_probe_track *)track)->AttachmentLink[0]) {								   
								subtitle_info->stitle_parse[track_no]->font.data = BKNI_Malloc(attachment->FileData_size);
								subtitle_info->stitle_parse[track_no]->font.size = attachment->FileData_size;
								fseek(fin, attachment->FileData_offset, SEEK_SET);
								fread(subtitle_info->stitle_parse[track_no]->font.data, 1, attachment->FileData_size, fin);								
							}
						}
					}
					break;
				default:
					break;
				}			
				track_no++;
			default:
				break;
			}
			
			if(stream->type == bstream_mpeg_type_mkv){																
				if(track_info->video_info[0].trick_track_flag){ 
					b_titles_info *master_title_info = NULL;
					b_track_info *master_track_info = NULL;

					trick_play_track = true;
					for(master_title_info=BLST_SQ_FIRST(&titles);master_title_info;master_title_info=BLST_SQ_NEXT(master_title_info, link)){
						master_track_info = &master_title_info->track_info;
						if((master_track_info->video_info[0].track_uid == track_info->video_info[0].master_track_uid) &&
						   (master_track_info->video_info[0].trick_track_uid == track_info->video_info[0].track_uid)){
							if(!BLST_Q_FIRST(&master_track_info->editions)){
								master_title_info->trick_play = title_info;
								title_info->master = master_title_info;
								break;
							} else {
								int i;
								for(i=0; i<MAX_TRACKS; i++){
									if(subtitle_info->stitle_parse[i]){
										BKNI_Free(subtitle_info->stitle_parse[i]);
									}									
								}
								BKNI_Free(title_info);							
								goto next_title;
							}							
						} 
					}
				}
			}				
		}

		if(!trick_play_track) {
			BLST_SQ_INSERT_TAIL(&titles, title_info, link);	
		} 

		if(stream->type == bstream_mpeg_type_mkv) {
		       bmkv_probe_stream *mkv_stream = (bmkv_probe_stream *)stream;
		       bmkv_Editions *edition;
		       bmkv_Chapters *chapter;
		       b_edition_info *edition_info;
		       b_chapter_info *chapter_info;

		       BLST_Q_INIT(&track_info->editions);
		       for(edition=BLST_SQ_FIRST(&mkv_stream->editions);edition;edition=BLST_SQ_NEXT(edition, link)){				 
			       edition_info = BKNI_Malloc(sizeof(*edition_info));
			       BKNI_Memset(edition_info, 0, sizeof(*edition_info));
			       edition_info->EditionUID = edition->EditionUID;
			       edition_info->EditionFlagHidden = edition->EditionFlagHidden;
			       edition_info->EditionFlagDefault = edition->EditionFlagDefault;
			       edition_info->EditionFlagOrdered = edition->EditionFlagOrdered;
			       
			       BDBG_MSG(("edition : UID %llx Flags %u:%u:%u", edition_info->EditionUID, edition_info->EditionFlagHidden, edition_info->EditionFlagDefault, edition_info->EditionFlagOrdered));

			       BLST_Q_INIT(&edition_info->chapters);
			       for(chapter=BLST_SQ_FIRST(&edition->chapters);chapter;chapter=BLST_SQ_NEXT(chapter, link)){				 
				       chapter_info = BKNI_Malloc(sizeof(*chapter_info));
				       BKNI_Memset(chapter_info, 0, sizeof(*chapter_info));
				       chapter_info->ChapterUID = chapter->ChapterUID;
				       chapter_info->ChapterTimeStart = chapter->ChapterTimeStart;
				       chapter_info->ChapterTimeEnd = chapter->ChapterTimeEnd;
				       chapter_info->ChapterFlagHidden = chapter->ChapterFlagHidden;
				       chapter_info->ChapterFlagEnabled = chapter->ChapterFlagEnabled;
				       if(chapter->ChapterDisplay.ChapString){
					       chapter_info->ChapString = BKNI_Malloc(strlen(chapter->ChapterDisplay.ChapString));
					       strcpy(chapter_info->ChapString, chapter->ChapterDisplay.ChapString);
				       }
				       if(chapter->ChapterDisplay.ChapLanguage){
					       chapter_info->ChapLanguage = BKNI_Malloc(strlen(chapter->ChapterDisplay.ChapLanguage));
					       strcpy(chapter_info->ChapLanguage, chapter->ChapterDisplay.ChapLanguage);
				       }
				       if(chapter->ChapterDisplay.ChapCountry){
					       chapter_info->ChapCountry = BKNI_Malloc(strlen(chapter->ChapterDisplay.ChapCountry));
					       strcpy(chapter_info->ChapCountry, chapter->ChapterDisplay.ChapCountry);
				       }	
				       BDBG_MSG(("\tchapter: UID %llx Time [%02u:%02u.%03u -> %02u:%02u.%03u] Flags %u:%u %s %s %s", chapter_info->ChapterUID, 
							  chapter_info->ChapterTimeStart/60000, (chapter_info->ChapterTimeStart/1000)%60, chapter_info->ChapterTimeStart%1000, 
							  chapter_info->ChapterTimeEnd/60000, (chapter_info->ChapterTimeEnd/1000)%60, chapter_info->ChapterTimeEnd%1000,
							  chapter_info->ChapterFlagHidden, chapter_info->ChapterFlagEnabled, 
							  chapter_info->ChapString, chapter_info->ChapLanguage, chapter_info->ChapCountry));
				       BLST_Q_INSERT_TAIL(&edition_info->chapters, chapter_info, link);
			       }
			       
			       BLST_Q_INSERT_TAIL(&track_info->editions, edition_info, link);
		       }		       		       
		}      
		
		subtitle_info->total_tracks = track_no;	
		subtitle_info->on = true;

		track_info->total_audio_tracks = audio_trk_num;
		track_info->total_video_tracks = video_trk_num;
next_title:		
		if(orig_fd){		
			bfile_read_offset_detach(fd);
			fd = orig_fd;
		}

		if(stream->type == bstream_mpeg_type_mkv  && ((bmkv_probe_stream *)stream)->next_volume.next_volume_offset_valid) {			
			orig_fd = fd;
			title_offset += ((bmkv_probe_stream *)stream)->next_volume.next_volume_offset;
			fd = bfile_read_offset_attach(orig_fd, title_offset);
			multi_title=true;
		}
error:
		bmedia_probe_stream_free(probe, stream);		
	}
	
	if(multi_title)
		goto probe_next_title;
	
	bfile_stdio_read_detach(fd);
	fclose(fin);
	bmedia_probe_destroy(probe);

	cur_title_info = BLST_SQ_FIRST(&titles);
	if(cur_title_info){
		cur_track_info = &cur_title_info->track_info;
		cur_edition_info = BLST_Q_FIRST(&cur_track_info->editions);
		if(cur_edition_info)
			cur_chapter_info = BLST_Q_FIRST(&cur_edition_info->chapters);
		cur_subtitle_info = &cur_title_info->subtitle_info;
		bstitle_activate(cur_subtitle_info->stitle_parse[0]);

		return cur_subtitle_info->stitle_parse[0];
	}

	return NULL;

}

char * findchr (const char * str, int c, int len)
{
	int cnt=0;
	char * ptr;

	while(cnt<len){
		if(str[cnt++]==c){
			ptr = (char*)str+cnt-1;
			return ptr;
		}
	}

	return NULL;
}

void
bstitle_parse_get_text_offset(const bmedia_probe_track *track, bstitle_parse_t stitle_parse)
{
	char *buf, *str;
	size_t size, left;
	unsigned txt_position=0;

	buf = str = (char*)((bmkv_probe_track *)track)->CodecPrivate_data; 
	size = left = ((bmkv_probe_track *)track)->CodecPrivate_len;

	/* Skip over to [Events] section */
	while(1){							
		str = (char*)findchr((char*)str, '[', left);		
		left = size -((uint32_t)str-(uint32_t)buf);
		if((left<=0) || (!str))
			return;
		if((str[0]=='[') && (str[1]=='E') && (str[2]=='v') && (str[3]=='e') 
		   && (str[4]=='n') && (str[5]=='t') && (str[6]=='s') && (str[7]==']')){
			break;
		}
		str++;
	}

	/*Skip over to Text field */
	while(1){
		str = (char*)findchr((char*)str, ',', left);	
		left = size - ((uint32_t)str-(uint32_t)buf);
		if((left<=0) || (!str))
			return;
		if((str[0]==',') && (str[1]==' ') && (str[2]=='T') 
		   && (str[3]=='e') && (str[4]=='x') && (str[5]=='t')){
			break;
		}
		str++;
		txt_position++;
	}	

	stitle_parse->txt_subtitle_position = txt_position;
}


void
bstitle_parse_uninit(bstitle_parse_t subtitles)
{
	unsigned i;	       
	b_titles_info *title_info;
	b_stitle_info* subtitle_info;
	b_track_info *track_info;
	b_edition_info *edition_info;
	b_chapter_info *chapter_info;

	BSTD_UNUSED(subtitles);
	
	while(NULL!=(title_info=BLST_SQ_FIRST(&titles))) {
		track_info = &title_info->track_info;
		subtitle_info = &title_info->subtitle_info;
		BLST_SQ_REMOVE_HEAD(&titles, link);

		while(NULL!=(edition_info=BLST_Q_FIRST(&track_info->editions))) {
			BLST_Q_REMOVE_HEAD(&track_info->editions, link);
			while(NULL!=(chapter_info=BLST_Q_FIRST(&edition_info->chapters))) {
				BLST_Q_REMOVE_HEAD(&edition_info->chapters, link);
				if(chapter_info->ChapString)
					BKNI_Free(chapter_info->ChapString);
				if(chapter_info->ChapLanguage)
					BKNI_Free(chapter_info->ChapLanguage);
				if(chapter_info->ChapCountry)
					BKNI_Free(chapter_info->ChapCountry);
				BKNI_Free(chapter_info);
			}		
			BKNI_Free(edition_info);
		}

		for(i=0; i<MAX_TRACKS; i++){
			if(subtitle_info->stitle_parse[i]){
				bstitle_deactivate(subtitle_info->stitle_parse[i]);
				if(subtitle_info->stitle_parse[i]->font.data){
					BKNI_Free(subtitle_info->stitle_parse[i]->font.data);
				}
				BKNI_Free(subtitle_info->stitle_parse[i]);
			}
		}
		
		if(title_info->trick_play){
			BKNI_Free(title_info->trick_play);
		}
		BKNI_Free(title_info);
	}	  
}

void bstitle_activate(bstitle_parse_t stitle_parse)
{
    int i;
    
    if(stitle_parse){			       
	if(!stitle_parse->activated){	
	    stitle_parse->raw_data_size = ((stitle_parse->width*stitle_parse->height)/2)+BSUBTITLE_INFO_SIZE;
	    for(i=0; i<DATA_BUFFERS; i++){
		stitle_parse->raw_data[i] = BKNI_Malloc(stitle_parse->raw_data_size);
		BKNI_Memset(stitle_parse->raw_data[i], 0, stitle_parse->raw_data_size);
	    }
	    stitle_parse->rptr=0;
	    stitle_parse->wptr=0;
	    stitle_parse->activated = true;	
	}	
    }
}

void bstitle_deactivate(bstitle_parse_t stitle_parse)
{
    int i;

    if(stitle_parse){
	if(stitle_parse->activated){		
	    for(i=0; i<DATA_BUFFERS; i++){
		BKNI_Free(stitle_parse->raw_data[i]);
	    }
	    stitle_parse->raw_data_size = 0;
	    stitle_parse->rptr=0;
	    stitle_parse->wptr=0;
	    stitle_parse->activated = false;		
	}
    }
}

bstitle_parse_t bstitle_switch(bstitle_parse_t stitle_parse, int track_id)
{
    bstitle_info_t subtitle_info = stitle_parse->stitle_info;
    bstitle_parse_t parse = NULL;

    bstitle_deactivate(stitle_parse);
    if(track_id<1 || track_id> subtitle_info->total_tracks){
	parse = stitle_parse;
    }else{
	parse = subtitle_info->stitle_parse[track_id-1];	
	bstitle_activate(parse);
    }
	
    return parse;
}

void bstitle_parser_flush(bstitle_parse_t stitle_parse)
{
    int i;

    if(stitle_parse){
	for(i=0; i<DATA_BUFFERS; i++){		
	    BKNI_Memset(stitle_parse->raw_data[i], 0, stitle_parse->raw_data_size);
	}
	stitle_parse->rptr=0;
	stitle_parse->wptr=0;
    }
}

b_titles_info * bstitle_get_title(unsigned num)
{
    b_titles_info *title_info;
    unsigned title_cnt;

    if(!num)
	return NULL;

    for(title_info=BLST_SQ_FIRST(&titles),title_cnt=1;title_info;title_info=BLST_SQ_NEXT(title_info, link),title_cnt++){
	if (num==title_cnt) {
	    break;
	}		    
    }

    return title_info;
}

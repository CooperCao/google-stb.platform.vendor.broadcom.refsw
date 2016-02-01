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
 * Divx Subtitle nexus app
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "bstd.h"

#include "nexus_platform.h"

#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#endif
#include "subtitle_priv.h"
#include "sync_manager.h"

#include "nexus_file_pvr.h"
#include "bfile_util.h"

#include "bmedia_filter.h"
#include "bmedia_probe.h"
#include "bavi_probe.h"
#include "bmkv_probe.h"
#include "bfile_stdio.h"
#include "bfile_util.h"

BDBG_MODULE(divx_subtitle);
#define BDBG_MSG_TRACE(x)   /* BDBG_MSG(x) */


typedef struct b_divx_subtitle_info {
	bool erase;
} b_divx_subtitle_info;

Subtitle_Handles gHandles;
static Subtitle_Scheduler_data gData;
static Subtitle_DecodeSettings gDecodeSettings;

/* static uint32_t color_key; */

static bstitle_parse_t subtitles;
static unsigned cur_subtitle_track;
extern unsigned cur_audio_track;

static btsm_queue_t tsm_queue;

static B_SchedulerTimerId sync_timer=NULL;
static B_SchedulerTimerId chapter_timer=NULL;

#define HD_WIDTH 1920
#define HD_HEIGHT 1080
#define SD_WIDTH 720
#define SD_HEIGHT 576

#define SUBTITLE_TIMER_INTERVAL 100

int trick_rate_table[] = {-64,-32,-16,-8,-4,-2,0,2,4,8,16,32,64};
#define MAX_TRICK_INDEX ((sizeof(trick_rate_table)/sizeof(trick_rate_table[0]))-1)
#define NORMAL_PLAY_INDEX MAX_TRICK_INDEX/2

/* int tpt_rate_table[] = {100,75,50,30,20,5,1,10,20,30,50,75,100}; */
int tpt_rate_table[] = {64,32,16,8,4,2,1,2,4,8,16,32,64};
#define MAX_TPT_INDEX ((sizeof(tpt_rate_table)/sizeof(tpt_rate_table[0]))-1)
#define NORMAL_TPT_INDEX MAX_TPT_INDEX/2

static struct {
    int trickModeRate;
    int fast;    
} state = {6,0};

#define abs(X) ((X)>0?(X):-(X))

static void sync_callback(void *pParam);
NEXUS_Error Generate_HardwareKey(uint8_t *hardware_secret);

extern b_titles_info *cur_title_info;
extern b_stitle_info *cur_subtitle_info;
extern b_track_info *cur_track_info;
extern b_edition_info *cur_edition_info;
extern b_chapter_info *cur_chapter_info;

uint8_t* message_buffer=NULL;

const char *filename;

#ifdef B_HAS_DIVX_DRM

#define HARDWARE_SECRET_LEN NEXUS_DIVXDRM_HARDWARE_SECRET_LENGTH
#define FRAGMENT1_LEN       NEXUS_DIVXDRM_MEMORY_FRAGMENT1_LENGTH 
#define FRAGMENT2_LEN       NEXUS_DIVXDRM_MEMORY_FRAGMENT2_LENGTH 
#define FRAGMENT3_LEN       NEXUS_DIVXDRM_MEMORY_FRAGMENT3_LENGTH 
#define MAX_DRM_TRACKS      NEXUS_DIVXDRM_MAX_TRACKS

/* Drm Information that needs to be passed down to Nexus/Drm lib */
typedef struct drm_probe_track_info {
    NEXUS_PidType trk_type[MAX_DRM_TRACKS];
    uint8_t * drm_header_data;
    unsigned drm_header_len;
    uint8_t hardware_secret[HARDWARE_SECRET_LEN];
    NEXUS_DivxDrmState drmState;    
    unsigned num_tracks;
} drm_probe_track_info;


drm_probe_track_info drm_info;

void drm_read_memory(void)
{
    FILE *fp=NULL;

    /* Read DRM Memory from file */
    fp = fopen("playervis.db", "rb");
    BDBG_ASSERT(fp);    
    fread(drm_info.drmState.fragment1, 1, FRAGMENT1_LEN, fp);
    fclose(fp);
    drm_info.drmState.fragment1Length = FRAGMENT1_LEN;

    fp = fopen("ccoredm.dll", "rb");
    BDBG_ASSERT(fp);    
    fread(drm_info.drmState.fragment2, 1, FRAGMENT2_LEN, fp);
    fclose(fp);
    drm_info.drmState.fragment2Length = FRAGMENT2_LEN;

    fp = fopen("mskdmpw2.dll", "rb");
    BDBG_ASSERT(fp);    
    fread(drm_info.drmState.fragment3, 1, FRAGMENT3_LEN, fp);
    fclose(fp);
    drm_info.drmState.fragment3Length = FRAGMENT3_LEN;
}

void drm_write_memory(void)
{    
    FILE *fp=NULL;            

    /* Write DRM Memory to file */
    fp = fopen("playervis.db", "wb");
    BDBG_ASSERT(fp);    
    fwrite(drm_info.drmState.fragment1, 1, FRAGMENT1_LEN, fp);
    fclose(fp);

    fp = fopen("ccoredm.dll", "wb");
    BDBG_ASSERT(fp);    
    fwrite(drm_info.drmState.fragment2, 1, FRAGMENT2_LEN, fp);
    fclose(fp);

    fp = fopen("mskdmpw2.dll", "wb");
    BDBG_ASSERT(fp);    
    fwrite(drm_info.drmState.fragment3, 1, FRAGMENT3_LEN, fp);
    fclose(fp);    
}

int drm_init(const char *file, uint64_t file_offset)
{
    bmedia_probe_t probe = bmedia_probe_create();
    FILE *fin = fopen(file,"rb");	
    bfile_io_read_t fd = bfile_stdio_read_attach(fin);
    bfile_io_read_t orig_fd = NULL;
    const bmedia_probe_stream *stream;
    const bmedia_probe_track *track;
    bmedia_probe_config probe_config;
    char stream_info[512];
    NEXUS_Error rc = NEXUS_SUCCESS;  
    
    NEXUS_DivxDrmCreateSettings drmCreateSettings; 
    NEXUS_DivxDrmStartSettings drmStartSettings; 
    NEXUS_SecurityKeySlotSettings genericSettings;

    BKNI_Memset(&drm_info, 0, sizeof(drm_probe_track_info));

    if(file_offset) {
	orig_fd = fd;
	fd = bfile_read_offset_attach(orig_fd, file_offset);
    }

    bmedia_probe_default_cfg(&probe_config);
    probe_config.file_name = file;	
    stream = bmedia_probe_parse(probe, fd, &probe_config);

    if(stream) {
	bmedia_stream_to_string(stream, stream_info, sizeof(stream_info));
	BDBG_WRN(("%s: %s", file, stream_info));

	if(stream->type == bstream_mpeg_type_mkv) {
	    bmkv_probe_stream *mkv_stream = (bmkv_probe_stream *)stream;
	    if(mkv_stream->TracksDataPayload){
		rc = NEXUS_Memory_Allocate(mkv_stream->TracksDataSize, NULL, (void **)&drm_info.drm_header_data);
		BDBG_ASSERT(!rc);
		drm_info.drm_header_len = mkv_stream->TracksDataSize;
		BKNI_Memcpy((void*)drm_info.drm_header_data, (void **)mkv_stream->TracksDataPayload, mkv_stream->TracksDataSize);		
	    }												    
	}
		
	for(track=BLST_SQ_FIRST(&stream->tracks);track;track=BLST_SQ_NEXT(track, link)) {				
	    switch(track->type) {
	    case bmedia_track_type_video:
		if((stream->type == bstream_mpeg_type_avi)){
		    if(((bavi_probe_track *)track)->encryptedContentFlag){
			rc = NEXUS_Memory_Allocate(((bavi_probe_track *)track)->private_data_length-8, NULL, (void *)&drm_info.drm_header_data);
			BDBG_ASSERT(!rc);			    
			drm_info.drm_header_len = ((bavi_probe_track *)track)->private_data_length-8;
			BKNI_Memcpy((void*)drm_info.drm_header_data, (void*)(((bavi_probe_track *)track)->private_data+8), drm_info.drm_header_len);					
		    }
		} 		
		drm_info.trk_type[track->number-1] = NEXUS_PidType_eVideo;		
		if(track->info.video.codec == bvideo_codec_unknown){
		    drm_info.trk_type[track->number-1] = NEXUS_PidType_eOther;
		}		
		break;
	    case bmedia_track_type_audio:
		drm_info.trk_type[track->number-1] = NEXUS_PidType_eAudio;
		break;
	    case bmedia_track_type_other:
	    default:
		drm_info.trk_type[track->number-1] = NEXUS_PidType_eOther;		
		break;
	    }	    
	    drm_info.num_tracks++;
	}	 

	bmedia_probe_stream_free(probe, stream);
    }

    if(orig_fd){		
	bfile_read_offset_detach(fd);
	fd = orig_fd;
    }
    bfile_stdio_read_detach(fd);
    fclose(fin);
    bmedia_probe_destroy(probe);

    gHandles.divxDrm = NULL;

    if(drm_info.drm_header_data) {
	unsigned trk_cnt;	
	NEXUS_DivxDrmStatus status; 	
	char response[2];

	/* Allocate H/W Secret */
	BDBG_MSG(("Generate Hardware Secret"));
	rc = Generate_HardwareKey(drm_info.hardware_secret);  
  	if(rc) {
	    BDBG_WRN(("%d", rc));
	    goto err_key;
	}
	
	drm_read_memory();

	BDBG_MSG(("Divx Drm Create"));
	NEXUS_DivxDrm_GetDefaultCreateSettings(&drmCreateSettings);
	BKNI_Memcpy(drmCreateSettings.drmHardwareSecret, drm_info.hardware_secret, HARDWARE_SECRET_LEN);	
	gHandles.divxDrm = NEXUS_DivxDrm_Create(&drmCreateSettings);
	if(!gHandles.divxDrm) {
	    goto err_open;
	}
	
	BDBG_MSG(("Set Drm Memory"));
	NEXUS_DivxDrm_SetDrmState(gHandles.divxDrm, &drm_info.drmState);
	
	NEXUS_DivxDrm_GetDefaultStartSettings(&drmStartSettings);

	/* Some files have large header size, most of which is unused.For now limit the size */
	if(drm_info.drm_header_len > NEXUS_DIVXDRM_MAX_HEADER_DATA_LEN)
	    drm_info.drm_header_len = NEXUS_DIVXDRM_MAX_HEADER_DATA_LEN;
	BKNI_Memcpy(drmStartSettings.drmHeaderData, drm_info.drm_header_data, drm_info.drm_header_len);
	drmStartSettings.drmHeaderDataLength = drm_info.drm_header_len;
	for(trk_cnt=0; trk_cnt<drm_info.num_tracks; trk_cnt++) {
	    drmStartSettings.track[trk_cnt].type = drm_info.trk_type[trk_cnt];
	}	
	BDBG_MSG(("Start Divx Drm"));
	rc = NEXUS_DivxDrm_Start(gHandles.divxDrm, &drmStartSettings);
	if(rc) {
	    goto err_settings;
	}
	
	BDBG_MSG(("Get Drm Rental Status"));
	NEXUS_DivxDrm_GetStatus(gHandles.divxDrm, &status);
	if(status.rental.valid) {
	    BDBG_WRN(("\n\nYou have used %d out of %d views", status.rental.useCount, status.rental.useLimit ));
	    BDBG_WRN(("Please enter y to continue. Enter n to exit\n\n"));
		     fgets(response, 2, stdin);
		     if(strncmp(response, "n", 1)==0 || strncmp(response, "N", 1)==0) {
			 goto err_rental;
		     }  
	}
	
	BDBG_MSG(("Allocate Key Slot"));
	NEXUS_Security_GetDefaultKeySlotSettings(&genericSettings);
	genericSettings.keySlotEngine = NEXUS_SecurityEngine_eGeneric;    
	gHandles.genericKeyHandle = NEXUS_Security_AllocateKeySlot(&genericSettings);
	BDBG_ASSERT(gHandles.genericKeyHandle);

	BDBG_MSG(("Divx Drm Enable Decrypt"));
	rc = NEXUS_DivxDrm_EnableDecrypt(gHandles.divxDrm, gHandles.genericKeyHandle);
	if(rc){
	    goto err_start;
	}
	
	BDBG_MSG(("Get Divx Output Protection Status"));
	NEXUS_DivxDrm_GetStatus(gHandles.divxDrm, &status);	
	if(status.outputProtection.valid) {
	    outputProtection(NULL, status.outputProtection.acptbSignal, status.outputProtection.cgmsaSignal );
	}
	if(status.ict.valid) {
	    ictDisplay(NULL, status.ict.ictSignal);
	}
	BDBG_MSG(("Drm Init Done"));
    }
    
    return 0;

err_start:
err_rental:
err_settings:
    NEXUS_DivxDrm_Destroy(gHandles.divxDrm);
err_open:
err_key:
    if(drm_info.drm_header_data)
	NEXUS_Memory_Free(drm_info.drm_header_data);
    return 1;
}

void drm_uninit(void)
{
    NEXUS_Security_FreeKeySlot(gHandles.genericKeyHandle);
    gHandles.genericKeyHandle = NULL;
    NEXUS_DivxDrm_Stop(gHandles.divxDrm);
    NEXUS_DivxDrm_GetDrmState(gHandles.divxDrm, &drm_info.drmState);
    drm_write_memory();
    NEXUS_DivxDrm_Destroy(gHandles.divxDrm);
    gHandles.divxDrm = NULL;
    if(drm_info.drm_header_data)
	NEXUS_Memory_Free(drm_info.drm_header_data);
}

#endif

static void chapter_callback(void *pParam)
{
	b_chapter_info *chapter_info = pParam;	
	int trick_rate = trick_rate_table[state.trickModeRate];
	bool discontinuity = false;

	if(chapter_info){
		if(((trick_rate<0) && (cur_chapter_info->ChapterTimeStart != chapter_info->ChapterTimeEnd)) ||
		   ((trick_rate>=0) && (cur_chapter_info->ChapterTimeEnd != chapter_info->ChapterTimeStart))){
			discontinuity = true;
		}
		
		if(discontinuity){			
			NEXUS_Playback_Seek(gHandles.playback, (trick_rate<0)?chapter_info->ChapterTimeEnd:chapter_info->ChapterTimeStart);	
			if(trick_rate){
				NEXUS_PlaybackTrickModeSettings trickModeSettings;
				NEXUS_Playback_GetDefaultTrickModeSettings(&trickModeSettings);
				trickModeSettings.rate = NEXUS_NORMAL_PLAY_SPEED * trick_rate;
				NEXUS_Playback_TrickMode(gHandles.playback, &trickModeSettings);
			}
		} else {
			unsigned diff = chapter_info->ChapterTimeEnd - chapter_info->ChapterTimeStart;
			if(trick_rate)
			  diff /= abs(trick_rate);
			chapter_timer=B_Scheduler_StartTimer(gData.scheduler, gData.mutex, diff, chapter_callback, trick_rate<0?BLST_Q_PREV(chapter_info, link):BLST_Q_NEXT(chapter_info, link));					
		}
		cur_chapter_info = chapter_info;
		BDBG_MSG(("Chapter [%02u:%02u.%03u -> %02u:%02u.%03u]", 
			       cur_chapter_info->ChapterTimeStart/60000, (cur_chapter_info->ChapterTimeStart/1000)%60, cur_chapter_info->ChapterTimeStart%1000, 
			       cur_chapter_info->ChapterTimeEnd/60000, (cur_chapter_info->ChapterTimeEnd/1000)%60, cur_chapter_info->ChapterTimeEnd%1000));	
	} else {
		NEXUS_Playback_Pause(gHandles.playback);
	}
}

void first_pts_passed(void *context, int unused)
{
	uint32_t diff, display_time;
	b_chapter_info *chapter_info = *((b_chapter_info **)context);
	b_chapter_info *callback_context;
	NEXUS_VideoDecoderStatus decoderStatus;
	int trick_rate = trick_rate_table[state.trickModeRate];

	BSTD_UNUSED(unused);
	
	NEXUS_VideoDecoder_GetStatus(gHandles.videoDecoder, &decoderStatus);
	display_time = bmedia_pts2time(decoderStatus.pts, trick_rate?(trick_rate*BMEDIA_TIME_SCALE_BASE):BMEDIA_TIME_SCALE_BASE);
	if(trick_rate<0){
	  diff = display_time - chapter_info->ChapterTimeStart;
	  callback_context = BLST_Q_PREV(chapter_info, link);
	} else {
	  diff = chapter_info->ChapterTimeEnd - display_time;	
	  callback_context = BLST_Q_NEXT(chapter_info, link);
	}
	if(trick_rate)
	  diff /= abs(trick_rate);
	if(!diff){
		/* Dont setup a timer. Callback rigth away */
		chapter_callback((void*)callback_context);
	} else {
		chapter_timer=B_Scheduler_StartTimer(gData.scheduler, gData.mutex, diff, chapter_callback, callback_context);
	}	
}


static void beginningOfStreamCallback(void *context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);

    BDBG_WRN(("Reached beginning of stream. Resuming Normal Play"));
    
    state.trickModeRate = NORMAL_PLAY_INDEX;
    state.fast = 0;

    if(subtitles && subtitles->stitle_info->on){	
	StartSubtitleDecode(&gDecodeSettings);	
    }   
}

static void complete(void *data, int unused)
{
	BSTD_UNUSED(unused);
	BKNI_SetEvent((BKNI_EventHandle)data);
}


static void scheduler_callback(void *pParam)
{
	Subtitle_Scheduler_data *pData = pParam;
	
	BDBG_ASSERT(NULL != pData);
	B_Scheduler_Run(pData->scheduler);
}


static void subtitle_feed(void *pParam)
{
    NEXUS_Error rc;
    Subtitle_Scheduler_data *pData = pParam;
    int trackNo=0;
    bool msg_received=false;
    struct {
	const void *buffer;
	size_t size;
    } msg_info[MAX_TRACKS];
    
    BDBG_ASSERT(NULL != pData);	
    
    gHandles.subtitle_started = true;
    
    while(gHandles.subtitle_started) {		
	trackNo=0;
	msg_received=false;
	
	for(trackNo=0; trackNo<subtitles->stitle_info->total_tracks; trackNo++){			
	    rc = NEXUS_Message_GetBuffer(gHandles.subtitle[trackNo], &(msg_info[trackNo].buffer), &(msg_info[trackNo].size));
	    BDBG_ASSERT(!rc);
	    
	    if(msg_info[trackNo].size)
		msg_received = true;
	}
	
	if (!msg_received) {
	    rc = BKNI_WaitForEvent(pData->subtitle_event, BKNI_INFINITE);
	    continue;
	} else {
	    for(trackNo=0; trackNo<subtitles->stitle_info->total_tracks; trackNo++){
		if(msg_info[trackNo].size){
		    unsigned track_id = *((uint8_t *)msg_info[trackNo].buffer+3) |
			*((uint8_t *)msg_info[trackNo].buffer+2)<<8  | 
			*((uint8_t *)msg_info[trackNo].buffer+1)<<16 | 
			*((uint8_t *)msg_info[trackNo].buffer+0)<<24;						 					 
		    if((track_id) == ((subtitles->track_index-1) | 0x000001D0)){						   						 
			size_t msg_len, pes_len;
			uint8_t *buffer = (uint8_t *)msg_info[trackNo].buffer;
			uint8_t *tmp_buf = message_buffer;
			msg_len = msg_info[trackNo].size;
			pes_len = (*((uint8_t *)msg_info[trackNo].buffer+4)<<8 | *((uint8_t *)msg_info[trackNo].buffer+5)) + 6;						 
			if(msg_len < pes_len) {
			    /* We received partial PES frame. Need to read one more buffer */
			    BKNI_Memcpy(tmp_buf, msg_info[trackNo].buffer, msg_info[trackNo].size);
			    tmp_buf += msg_info[trackNo].size;
			    rc = NEXUS_Message_ReadComplete(gHandles.subtitle[trackNo], msg_info[trackNo].size);
			    BDBG_ASSERT(!rc);
			    do {
				rc = NEXUS_Message_GetBuffer(gHandles.subtitle[trackNo], &(msg_info[trackNo].buffer), &(msg_info[trackNo].size));
				BDBG_ASSERT(!rc);
			    } while(!msg_info[trackNo].size);
			    
			    BKNI_Memcpy(tmp_buf, msg_info[trackNo].buffer, msg_info[trackNo].size);
			    msg_len += msg_info[trackNo].size;
			    buffer = message_buffer;
			}
			
			switch(cur_track_info->type){						 
			case bstream_mpeg_type_mkv:												 
			    bstitle_parser_txt(subtitles, buffer, msg_len);	
			    break;
			case bstream_mpeg_type_avi:
			default:			    
			    bstitle_parser_bmp(subtitles, buffer, msg_len);
			    break;
			}
		    }
		    rc = NEXUS_Message_ReadComplete(gHandles.subtitle[trackNo], msg_info[trackNo].size);
		    BDBG_ASSERT(!rc);
		}
	    }
	}
    }	
}


static void 
sync_set_subtitle(void * cnxt, int32_t start_time, int32_t end_time)
{
	btsm_queue_entry tsm_entry;
	b_divx_subtitle_info *info = (b_divx_subtitle_info*)tsm_entry.userdata;
	BSTD_UNUSED(cnxt);
	BDBG_MSG(("sync_set_subtitle: %#lx %d:%d", (unsigned long)cnxt, start_time, end_time));

	BTSM_QUEUE_ENTRY_INIT(&tsm_entry);
	BTSM_QUEUE_ENTRY_SET_TIMESTAMP(&tsm_entry, start_time);
	info->erase = false;
	btsm_queue_push(tsm_queue, &tsm_entry);
	if(end_time) {
		BTSM_QUEUE_ENTRY_SET_TIMESTAMP(&tsm_entry, end_time);
		info->erase = true;
		btsm_queue_push(tsm_queue, &tsm_entry);
	}
}

static void 
render_subtitle_txt(   NEXUS_Graphics2DHandle graphics, NEXUS_SurfaceHandle surface, uint8_t * buffer)
{
        NEXUS_Graphics2DFillSettings fillSettings;
	NEXUS_Rect dst, src;
	uint32_t size = *((uint32_t*)buffer);
	char *str = (char*)buffer+4;
	unsigned lineheight;
	int xPos, yPos;	
	bool done = false;
	bwin_font_t current_font = subtitles->font.data?gHandles.win_font_attached[subtitles->track_index-1]:gHandles.win_font_default;
	NEXUS_Error rc;

	BSTD_UNUSED(surface);

	BDBG_MSG(("render_subtitle_txt:%#lx", (unsigned long)buffer)); 

	NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
	fillSettings.surface = gHandles.offscreen;
	fillSettings.color = 0;
	while (1) {
	    rc = NEXUS_Graphics2D_Fill(gHandles.graphics, &fillSettings);
	    if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL) {                
                BKNI_WaitForEvent(gHandles.packetSpaceAvailableEvent, BKNI_INFINITE);
            }
            else {
                break;
            }
        }
	rc = NEXUS_Graphics2D_Checkpoint(graphics, NULL);
	if (rc == NEXUS_GRAPHICS2D_QUEUED) {
	    rc = BKNI_WaitForEvent(gHandles.checkpointEvent, BKNI_INFINITE);
	}

	bwin_get_font_height(current_font, &lineheight);
	
	/* Offscreen surce create settings are stores in gHandles */
	yPos = (8*gHandles.createSettings.height)/10;

	while(!done){		
	    char *ptr = str;
	    unsigned len, i, pre_trim=0, post_trim=0;
	    int width, height, base;

	    str = strchr(ptr, '\\');
	    if(str == NULL){
		str = strchr(ptr, 0x0A);
		if(str == NULL){
		    len = size - ((uint32_t)ptr - ((uint32_t)buffer + 4));
		    done = true;
		}
		else {
		    len = (uint32_t)str - (uint32_t)ptr;
		    str++;
		}
	    }
	    else if(!((str[0] =='\\') && (str[1] == 'N'))){
		str++;
		continue;
	    }		
	    else {
		len = (uint32_t)str - (uint32_t)ptr;
		str+=2;
	    }
	    
	    if(!len)
		return;

	    for(i=0; i<len-3; i++){			
		if(((ptr[i] =='<') && ((ptr[i+1] == 'u') || (ptr[i+1] == 'i')) && (ptr[i+2] == '>'))){
		    pre_trim+=3;				
		}			
		if(((ptr[i] =='<') && (ptr[i+1] == '/') && ((ptr[i+2] == 'u') || (ptr[i+2] == 'i')) && (ptr[i+3] == '>'))){			
		    post_trim+=4;
		}
	    }
	
	    ptr+=pre_trim;
	    len-=(pre_trim+post_trim);
	    bwin_measure_text(current_font, ptr, len, &width, &height, &base);		
	    xPos = (gHandles.createSettings.width-width)/2;
	    bwin_draw_text(gHandles.win_fb_settings.window, current_font, xPos, yPos, ptr, len, 0xFFA0A0A0, NULL);	       
	    yPos += lineheight;		
	}

	NEXUS_Surface_Flush(gHandles.offscreen);
	
	dst.x = 0;
	dst.y = 0;
	dst.width = gHandles.displayHDWidth;
	dst.height = gHandles.displayHDHeight;
	src.x = 0;
	src.y = 0;
	src.width = gHandles.createSettings.width;
	src.height = gHandles.createSettings.height;
	while (1) {
	    rc = NEXUS_Graphics2D_FastBlit(graphics, gHandles.surfaceHD, &dst, gHandles.offscreen, &src, NEXUS_BlitColorOp_eCopySource, NEXUS_BlitAlphaOp_eCopySource, 0);	
	    if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL) {
		BKNI_WaitForEvent(gHandles.packetSpaceAvailableEvent, BKNI_INFINITE);
	    }
	    else {
		break;
	    }
	}
	
	dst.x = 0;
	dst.y = 0;
	dst.width = gHandles.displaySDWidth;
	dst.height = gHandles.displaySDHeight;
	src.x = 0;
	src.y = 0;
	src.width = gHandles.createSettings.width;
	src.height = gHandles.createSettings.height;
	while (1) {
	    rc = NEXUS_Graphics2D_FastBlit(graphics, gHandles.surfaceSD, &dst, gHandles.offscreen, &src, NEXUS_BlitColorOp_eCopySource, NEXUS_BlitAlphaOp_eCopySource, 0);	
	    if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL) {
		BKNI_WaitForEvent(gHandles.packetSpaceAvailableEvent, BKNI_INFINITE);
	    }
	    else {
		break;
	    }
	}

	rc = NEXUS_Graphics2D_Checkpoint(graphics, NULL);
	if (rc == NEXUS_GRAPHICS2D_QUEUED) {
	    rc = BKNI_WaitForEvent(gHandles.checkpointEvent, BKNI_INFINITE);
	}
}

static void 
render_subtitle_bmp(   NEXUS_Graphics2DHandle graphics, NEXUS_SurfaceHandle surface, uint8_t * buffer)
{
    uint32_t palette[4];
    unsigned int x, y, height, width;
    unsigned int *ptr;
    unsigned int i,j;
    uint8_t val, *buf, *raw_data;
    b_subtitle_hdr info;
            
    NEXUS_SurfaceMemory mem;   
    NEXUS_Rect dst, src;
    NEXUS_Error rc;

    unsigned scale, scaleX, scaleY, dstWidth, dstHeight;

    BSTD_UNUSED(surface);
        

    BDBG_MSG(("render_subtitle_bmp:%#lx", (unsigned long)buffer)); 

    memcpy(&info, buffer, BSUBTITLE_INFO_SIZE);
    raw_data = buffer + BSUBTITLE_INFO_SIZE;
 
    NEXUS_Surface_GetMemory(gHandles.offscreen, &mem);

    /* fill with black */
    /* Offscreen surce create settings are stores in gHandles */
    BKNI_Memset(mem.buffer, 0, gHandles.createSettings.height * mem.pitch);

    palette[0] = B_CREATE_PALETTE(info.background_transparency, 0, 0, info.background.blue);
    /* palette[0] = B_CREATE_PALETTE(info.background_transparency, info.background.red,info.background.green , info.background.blue); */
    palette[1] = B_CREATE_PALETTE(info.pattern1_transparency, info.pattern1.red, info.pattern1.green, info.pattern1.blue);
    palette[2] = B_CREATE_PALETTE(info.pattern2_transparency, info.pattern2.red, info.pattern2.green, info.pattern2.blue);
    palette[3] = B_CREATE_PALETTE(info.pattern3_transparency, info.pattern3.red, info.pattern3.green, info.pattern3.blue);          

    x = info.left;
    y = info.top;
    height = info.height;
    width = info.width;   

    /* copy to temp surface buf */
    buf= raw_data + ((width/2)*(height-1));
    ptr = (unsigned int*)mem.buffer;
    ptr += (y * mem.pitch) / 4;
    for(i=0; i<height; i++){
	    for(j=x; j<(width+x); j++){
		    if(j&0x1){
			    val = (uint8_t)(*buf & 0x0F) >> 0;
			    buf++;
		    }
		    else{
			    val = (uint8_t)(*buf & 0xF0) >> 4;
		    }
		    ptr[j] = palette[val];
	    }
	    ptr += mem.pitch / 4;
	    buf -= width;
    }
    
    /* flush cached memory */
    NEXUS_Surface_Flush(gHandles.offscreen);        

    /* We need to scale while maintaining the subtitle aspect ratio */
    if(gHandles.displayHDWidth >= subtitles->width) { /* upscaling */
	scaleX = (gHandles.displayHDWidth * 1000 ) / subtitles->width;
	scaleY = (gHandles.displayHDHeight * 1000 ) / subtitles->height;
	scale = scaleX<scaleY?scaleX:scaleY;

	dstWidth = subtitles->width * scale/1000;
	dstHeight = subtitles->height * scale/1000;	
    } else { /* downscaling */
	scaleX = (subtitles->width * 1000 ) / gHandles.displayHDWidth;
	scaleY = (subtitles->height * 1000 ) / gHandles.displayHDHeight; 
	scale = scaleX>scaleY?scaleX:scaleY;
	
	dstWidth = subtitles->width * 1000/scale;
	dstHeight = subtitles->height * 1000/scale;
    }
            
    dst.x = (gHandles.displayHDWidth > dstWidth)?(gHandles.displayHDWidth-dstWidth)/2:0;
    dst.y = (gHandles.displayHDHeight > dstHeight)?(gHandles.displayHDHeight-dstHeight)/2:0;
    dst.width = dstWidth;
    dst.height = dstHeight;
    src.x = 0;
    src.y = 0;
    src.width = subtitles->width;
    src.height = subtitles->height;
    while (1) {
	rc = NEXUS_Graphics2D_FastBlit(graphics, gHandles.surfaceHD, &dst, gHandles.offscreen, &src, NEXUS_BlitColorOp_eCopySource, NEXUS_BlitAlphaOp_eCopySource, 0);	
	if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL) {
	    BKNI_WaitForEvent(gHandles.packetSpaceAvailableEvent, BKNI_INFINITE);
	}
	else {
	    break;
	}
    }

    if(gHandles.displaySDWidth >= subtitles->width) { /* upscaling */
	scaleX = (gHandles.displaySDWidth * 1000 ) / subtitles->width;
	scaleY = (gHandles.displaySDHeight * 1000 ) / subtitles->height;
	scale = scaleX<scaleY?scaleX:scaleY;

	dstWidth = subtitles->width * scale/1000;
	dstHeight = subtitles->height * scale/1000;	
    } else { /* downscaling */
	scaleX = (subtitles->width * 1000 ) / gHandles.displaySDWidth;
	scaleY = (subtitles->height * 1000 ) / gHandles.displaySDHeight; 
	scale = scaleX>scaleY?scaleX:scaleY;
	
	dstWidth = subtitles->width * 1000/scale;
	dstHeight = subtitles->height * 1000/scale;
    }        
    
    dst.x = (gHandles.displaySDWidth > dstWidth)?(gHandles.displaySDWidth-dstWidth)/2:0; 
    dst.y = (gHandles.displaySDHeight > dstHeight)?(gHandles.displaySDHeight-dstHeight)/2:0;
    dst.width = dstWidth;
    dst.height = dstHeight;
    src.x = 0;
    src.y = 0;
    src.width = subtitles->width;
    src.height = subtitles->height;
    while (1) {
    	rc = NEXUS_Graphics2D_FastBlit(graphics, gHandles.surfaceSD, &dst, gHandles.offscreen, &src, NEXUS_BlitColorOp_eCopySource, NEXUS_BlitAlphaOp_eCopySource, 0);
    	if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL) {
    	    BKNI_WaitForEvent(gHandles.packetSpaceAvailableEvent, BKNI_INFINITE);
    	}
    	else {
    	    break;
    	}
    }

    rc = NEXUS_Graphics2D_Checkpoint(graphics, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
    	rc = BKNI_WaitForEvent(gHandles.checkpointEvent, BKNI_INFINITE);
    }
}

static void 
sync_destroy_subtitle(NEXUS_SurfaceHandle fb)
{
        NEXUS_SurfaceCreateSettings createSettings;
	NEXUS_Graphics2DFillSettings fillSettings;	
	NEXUS_Error rc;
	
	BDBG_MSG(("sync_destroy_subtitle: %#lx", (unsigned long)fb));

	NEXUS_Surface_GetCreateSettings(fb, &createSettings);

	/* fill with black */
	NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
	fillSettings.surface = fb;
	fillSettings.rect.width = createSettings.width;
	fillSettings.rect.height = createSettings.height;
	fillSettings.color = 0;
	while (1) {
	    rc = NEXUS_Graphics2D_Fill(gHandles.graphics, &fillSettings);
	    if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL) {                
                BKNI_WaitForEvent(gHandles.packetSpaceAvailableEvent, BKNI_INFINITE);
            }
            else {
                break;
            }
        }
	rc = NEXUS_Graphics2D_Checkpoint(gHandles.graphics, NULL);
	if (rc == NEXUS_GRAPHICS2D_QUEUED) {
	    rc = BKNI_WaitForEvent(gHandles.checkpointEvent, BKNI_INFINITE);
	}

	/* flush cached memory */
	NEXUS_Surface_Flush(fb);
}

static void clear_subtitle(NEXUS_SurfaceHandle fb)
{
	BDBG_MSG(("clear_subtitle: %#lx", (unsigned long)fb));

	sync_destroy_subtitle(fb);

	btsm_queue_flush(tsm_queue);
}

static void sync_callback(void *pParam)
{
	NEXUS_VideoDecoderStatus decoderStatus;
	btsm_queue_entry tsm_entry;
	btsm_queue_action action;
	b_divx_subtitle_info *info = (b_divx_subtitle_info *)tsm_entry.userdata;
	uint8_t *buffer=NULL;
	
	BDBG_MSG_TRACE(("sync_callback:%#lx", (unsigned long)pParam));
	BSTD_UNUSED(pParam);
	
	NEXUS_VideoDecoder_GetStatus(gHandles.videoDecoder, &decoderStatus);
	for(;;) {
		BTSM_QUEUE_ENTRY_INIT(&tsm_entry);
		info->erase = false;
		action = btsm_queue_get(tsm_queue, decoderStatus.pts/45, &tsm_entry);	
		if(action==btsm_queue_action_repeat) {
			break;
		}
		BDBG_MSG(("sync_callback: %s %u:%u %s", action==btsm_queue_action_drop?"drop":"display", decoderStatus.pts/45, tsm_entry.timestamp, info->erase?"erase":""));
		if(action==btsm_queue_action_drop) {
			if(!info->erase) {
				buffer = bstitle_get_buffer(subtitles);
				bstitle_read_complete(subtitles);
			} else {
				sync_destroy_subtitle(gHandles.surfaceHD);
				sync_destroy_subtitle(gHandles.surfaceSD);
			}
		} else { /* btsm_queue_action_display: */
			if(!info->erase) {
				buffer = bstitle_get_buffer(subtitles);
				if(buffer){
					switch(cur_track_info->type){						 
					case bstream_mpeg_type_mkv:
						render_subtitle_txt(gHandles.graphics, NULL, buffer);
						break;
					case bstream_mpeg_type_avi:
					default:
						render_subtitle_bmp(gHandles.graphics, NULL, buffer);
						break;
					}

				}
				bstitle_read_complete(subtitles);
			} else {
			    sync_destroy_subtitle(gHandles.surfaceHD);
			    sync_destroy_subtitle(gHandles.surfaceSD);
			}
			break;
		}
	}

	sync_timer=B_Scheduler_StartTimer(gData.scheduler, gData.mutex, SUBTITLE_TIMER_INTERVAL, sync_callback, NULL);
}

static void
printMenu(void)
{ 
	printf("Subtitle App Menu:\n");
	printf("?                 Print this menu\n");
	printf("st 'track'        Set Subtitle track number to display\n");
	printf("at 'track'        Set Audio track number to display\n");
	printf("ff                Fast Forward\n");
	printf("rew               Rewind\n");
	printf("play              Play\n");
	printf("displayHD 'res'   Set HD display resolution. res=480i,480p,576i,576p,720p,1080i\n");    
	printf("displaySD 'res'   Set SD display resolution. res=480i,576i\n");   
	printf("stat              Display playback statistics\n");
	printf("title   'num'     Display title list. Enter num to select title\n");
	printf("edition 'num'     Display edition list. Enter num to select edition\n");
	printf("chapter 'num'     Display chapters list. Enter num to select chapter\n");
	printf("1 - 10            Select auto-generated chapters 1 - 10\n"); 
	printf("q                 Quit\n");
}


void switch_title(b_titles_info *title_info, bool tsm, uint32_t display_time, unsigned subtitle_track, unsigned audio_track)
{
	NEXUS_Error rc;
	NEXUS_PlaybackSettings playbackSettings;
	Subtitle_DecodeSettings decodeSettings;
	uint32_t trackCount;
	NEXUS_DisplaySettings displaySettings;
	NEXUS_VideoFormatInfo info;		
#if 0/*  NEXUS_HAS_SYNC_CHANNEL */
	NEXUS_SyncChannelSettings syncChannelSettings;
#endif          	
	bool init_drm = false;

	if(sync_timer){
		B_Scheduler_CancelTimer(gData.scheduler, sync_timer);
		sync_timer = NULL;
	}
	if(chapter_timer){
		B_Scheduler_CancelTimer(gData.scheduler, chapter_timer);
		chapter_timer = NULL;
	}
			
	NEXUS_Playback_Stop(gHandles.playback);

#if 0/* NEXUS_HAS_SYNC_CHANNEL  */   	
	/* disconnect sync channel */
	NEXUS_SyncChannel_GetSettings(gHandles.syncChannel, &syncChannelSettings);
	syncChannelSettings.videoInput = NULL;
	syncChannelSettings.audioInput[0] = NULL;
	syncChannelSettings.audioInput[1] = NULL;
	NEXUS_SyncChannel_SetSettings(gHandles.syncChannel, &syncChannelSettings);
#endif
		
	StopVideoDecode();
	StopAudioDecode();
	StopSubtitleDecode();
	BKNI_Memset(gHandles.videoProgram, 0, sizeof(gHandles.videoProgram));
	BKNI_Memset(gHandles.audioProgram, 0, sizeof(gHandles.audioProgram));
	BKNI_Memset(gHandles.subtitleProgram, 0, sizeof(gHandles.subtitleProgram));
	{
	    unsigned trackNo=0;
	    for (trackNo=0; trackNo < gHandles.totalSubtitleTracks; trackNo++)
	    {	
		if(gHandles.subtitle[trackNo]){
		    NEXUS_Message_Close(gHandles.subtitle[trackNo]);
		    gHandles.subtitle[trackNo] = NULL;	
		}	
	    }
	}

	if(subtitles){		
		clear_subtitle(gHandles.surfaceHD);
		clear_subtitle(gHandles.surfaceSD);
		bstitle_parser_flush(subtitles);
	}
	if ( gHandles.playback )
	{
		NEXUS_Playback_CloseAllPidChannels(gHandles.playback);		
	}
			    			    
	if(gHandles.orig_filehandle){
		bfile_read_offset_detach(gHandles.file->file.data);
		bfile_read_offset_detach(gHandles.file->file.index);
		BKNI_Free(gHandles.file);
		gHandles.file = gHandles.orig_filehandle;
	}

#ifdef B_HAS_DIVX_DRM  
	if((cur_title_info->master != title_info) && (cur_title_info->trick_play != title_info)) {
	    if(gHandles.divxDrm) {
		drm_uninit();
		init_drm = true;
	    }
	}
#endif	

	cur_title_info = title_info;
	cur_track_info = &cur_title_info->track_info;
	cur_edition_info = BLST_Q_FIRST(&cur_track_info->editions);	
	if(cur_edition_info)
		cur_chapter_info = BLST_Q_FIRST(&cur_edition_info->chapters);
	cur_subtitle_info = &cur_title_info->subtitle_info;
	bstitle_activate(cur_subtitle_info->stitle_parse[subtitle_track-1]);
	subtitles = cur_subtitle_info->stitle_parse[subtitle_track-1];
	
	gHandles.orig_filehandle = gHandles.file;
	gHandles.file = BKNI_Malloc(sizeof(*gHandles.file));
	gHandles.file->file.data = bfile_read_offset_attach(gHandles.orig_filehandle->file.data, cur_title_info->file_offset);
	gHandles.file->file.index = bfile_read_offset_attach(gHandles.orig_filehandle->file.index, cur_title_info->file_offset);
	gHandles.file->file.close = NULL;

	NEXUS_Playback_GetSettings(gHandles.playback, &playbackSettings);
	playbackSettings.playpumpSettings.transportType = GetTransportType(cur_track_info->type);

#ifdef B_HAS_DIVX_DRM
	if(init_drm) {
	    rc = drm_init(filename, (uint64_t)cur_title_info->file_offset);
	    if(rc)
		return;
	}
	if(gHandles.genericKeyHandle) {
	    playbackSettings.playpumpSettings.securityContext = gHandles.genericKeyHandle;
	}	
#endif    

	rc = NEXUS_Playback_SetSettings(gHandles.playback, &playbackSettings);
	BDBG_ASSERT(!rc);

	/* Initialize and start decode */
	Decode_GetDefaultSettings(&decodeSettings);

	for(trackCount=0; trackCount < (uint32_t)cur_track_info->total_video_tracks; trackCount++){
		decodeSettings.videoPid[trackCount] = (uint16_t)cur_track_info->video_info[trackCount].track_id;
		decodeSettings.videoCodec[trackCount] = GetVideoCodec(cur_track_info->video_info[trackCount].codec);
		BDBG_WRN(("[Video] trackNo=%d Pid=%d Codec=%d", trackCount, decodeSettings.videoPid[trackCount],
			  decodeSettings.videoCodec[trackCount]));
	}

	for(trackCount=0; trackCount < (uint32_t)cur_track_info->total_audio_tracks; trackCount++){
		decodeSettings.audioPid[trackCount] = (uint16_t)cur_track_info->audio_info[trackCount].track_id;
		decodeSettings.audioCodec[trackCount] = GetAudioCodec(cur_track_info->audio_info[trackCount].codec);
		BDBG_WRN(("[Audio] trackNo=%d Pid=%d Codec=%d", trackCount, decodeSettings.audioPid[trackCount],
			  decodeSettings.audioCodec[trackCount]));
	}
	
	if(subtitles){
		for(trackCount=0; trackCount < (uint32_t)subtitles->stitle_info->total_tracks; trackCount++){
			decodeSettings.subtitlePid[trackCount] = (uint16_t)subtitles->stitle_info->stitle_parse[trackCount]->track_number;     
			BDBG_WRN(("[Subtitle] trackNo=%d Pid=%d", trackCount, decodeSettings.subtitlePid[trackCount]));
		}
	}

	NEXUS_Display_GetSettings(gHandles.displayHD, &displaySettings);
	NEXUS_VideoFormat_GetInfo(displaySettings.format, &info);     
	gHandles.videoWidth=cur_track_info->video_info[0].width;
	gHandles.videoHeight=cur_track_info->video_info[0].height;
	gHandles.displayHDWidth = info.width;
	gHandles.displayHDHeight = info.height;
	if(cur_track_info->video_info[0].display_width && cur_track_info->video_info[0].display_height){
		if(cur_track_info->video_info[0].display_width == cur_track_info->video_info[0].width && cur_track_info->video_info[0].display_height == cur_track_info->video_info[0].height){
			gHandles.aspect_ratio = NEXUS_AspectRatio_eSquarePixel;
		} else if((cur_track_info->video_info[0].display_width*9/cur_track_info->video_info[0].display_height)>=19) {
			gHandles.aspect_ratio = NEXUS_AspectRatio_e221x1;
		} else if((cur_track_info->video_info[0].display_width*9/cur_track_info->video_info[0].display_height)>=15) {
			gHandles.aspect_ratio = NEXUS_AspectRatio_e16x9;
		} else {	    
			gHandles.aspect_ratio = NEXUS_AspectRatio_e4x3;
		}	
	} else {
		gHandles.aspect_ratio = NEXUS_AspectRatio_eSquarePixel;
	}
		

	decodeSettings.playback = gHandles.playback;
	decodeSettings.playpump = gHandles.playpump;

	BKNI_Memcpy(&gDecodeSettings, &decodeSettings, sizeof(decodeSettings));
	
	SetVideoProgram(&decodeSettings, cur_track_info->total_video_tracks); 
	SetAudioProgram(&decodeSettings, cur_track_info->total_audio_tracks);

	if(subtitles){
		InitSubtitleDecode(&decodeSettings, subtitles->stitle_info->total_tracks, gData.subtitle_event);
	}	

	StartVideoDecode(&decodeSettings, 0, tsm);
	StartAudioDecode(&decodeSettings, audio_track, tsm);
	
	if(cur_edition_info) {
		NEXUS_VideoDecoderSettings video_settings;			    			    
				    
		NEXUS_VideoDecoder_GetSettings(gHandles.videoDecoder, &video_settings);
		video_settings.firstPtsPassed.callback = first_pts_passed;
		video_settings.firstPtsPassed.context = &cur_chapter_info;
		NEXUS_VideoDecoder_SetSettings(gHandles.videoDecoder, &video_settings);
	}
	
	rc = NEXUS_Playback_Start(gHandles.playback, gHandles.file, NULL);
		
	NEXUS_Playback_Seek(gHandles.playback, display_time);	
	
	if(subtitles) {
	    if(subtitles->stitle_info->on) {
		StartSubtitleDecode(&decodeSettings);
	    }
	    sync_timer=B_Scheduler_StartTimer(gData.scheduler, gData.mutex, SUBTITLE_TIMER_INTERVAL, sync_callback, NULL);
	}
}

void toggle_trick_play(b_titles_info *title_info, bool tsm, unsigned subtitle_track, unsigned audio_track)
{
	NEXUS_PlaybackStatus playbackStatus;
	NEXUS_VideoDecoderStatus vstatus;
	int32_t display_time;

	NEXUS_Playback_GetStatus(gHandles.playback, &playbackStatus);
	NEXUS_VideoDecoder_GetStatus(gHandles.videoDecoder, &vstatus);
	if(vstatus.pts == 0xFFFFFFFF)
	    vstatus.pts = 0;
	display_time = bmedia_pts2time(vstatus.pts, playbackStatus.trickModeSettings.rate>=0?(BMEDIA_TIME_SCALE_BASE):-BMEDIA_TIME_SCALE_BASE);		
	switch_title(title_info, tsm, display_time, subtitle_track, audio_track);		
}

static int tptMode(NEXUS_PlaybackHandle playback, int fast, int dir)     
{	
	if (fast != state.fast) {
		state.trickModeRate = NORMAL_TPT_INDEX;
		state.fast = fast;
	}
	
	if(dir>0)
		state.trickModeRate++;
	else if(dir<0)
		state.trickModeRate--;
	else 
		state.trickModeRate=NORMAL_TPT_INDEX; 

	state.trickModeRate=state.trickModeRate>(int)MAX_TPT_INDEX?MAX_TPT_INDEX:(state.trickModeRate<0?0:state.trickModeRate);    
	
	if(cur_title_info->master){		
		if(state.trickModeRate == NORMAL_TPT_INDEX){
			printf("Resuming normal Playback\n");
			toggle_trick_play(cur_title_info->master, true, cur_subtitle_track, cur_audio_track);
		} 
	} else if(fast && cur_title_info->trick_play){
		if(fast){		
			NEXUS_PlaybackTrickModeSettings trickModeSettings;

			printf("Starting Trick Play Track %d\n", dir);
			cur_subtitle_track = subtitles?subtitles->track_index:1;
			toggle_trick_play(cur_title_info->trick_play, true, 1, 1);	
			if(dir<0){
				NEXUS_Playback_GetDefaultTrickModeSettings(&trickModeSettings);
				trickModeSettings.rate = NEXUS_NORMAL_PLAY_SPEED * -1;
				NEXUS_Playback_TrickMode(playback, &trickModeSettings);
			}
		}		
	}
	
	if(state.trickModeRate != (int)NORMAL_TPT_INDEX){
	    printf("TPT Trickmode : %dX %s\n", tpt_rate_table[state.trickModeRate], dir>0?"Forward":"Rewind");
	    NEXUS_StcChannel_SetRate(gHandles.stcChannel, tpt_rate_table[state.trickModeRate], 0);
	} 		
		
	return 0;
}


static int trickMode(NEXUS_PlaybackHandle playback, int fast, int dir)
{
    int rc;    

    if(chapter_timer){
	B_Scheduler_CancelTimer(gData.scheduler, chapter_timer);
	chapter_timer = NULL;
    }
 
    if(cur_title_info->master || cur_title_info->trick_play){
	return tptMode(playback, fast, dir);
    }

    /* reset */
    if (fast != state.fast) {
        state.trickModeRate = NORMAL_PLAY_INDEX;
        state.fast = fast;
    }
    
    if(dir>0)
	state.trickModeRate++;
    else if(dir<0)
	state.trickModeRate--;
    else 
	state.trickModeRate=NORMAL_PLAY_INDEX; 
    

    state.trickModeRate=state.trickModeRate>(int)MAX_TRICK_INDEX?MAX_TRICK_INDEX:(state.trickModeRate<0?0:state.trickModeRate);    

    if (state.trickModeRate == NORMAL_PLAY_INDEX) {	    	
	if(subtitles && subtitles->stitle_info->on){	
	    StartSubtitleDecode(&gDecodeSettings);	
	}		
	
	rc = NEXUS_Playback_Play(playback);	
    } else {
        NEXUS_PlaybackTrickModeSettings trickModeSettings;

        if(subtitles){	
	    StopSubtitleDecode();
	    clear_subtitle(gHandles.surfaceHD);
	    clear_subtitle(gHandles.surfaceSD);
	    bstitle_parser_flush(subtitles);
        }	

        NEXUS_Playback_GetDefaultTrickModeSettings(&trickModeSettings);

        if (state.fast)
            trickModeSettings.rate = NEXUS_NORMAL_PLAY_SPEED * trick_rate_table[state.trickModeRate];
        else
            trickModeSettings.rate = NEXUS_NORMAL_PLAY_SPEED / trick_rate_table[state.trickModeRate];

	printf("Trickmode : %dX %s\n", trick_rate_table[state.trickModeRate], dir>0?"Forward":"Rewind");

        NEXUS_Playback_TrickMode(playback, &trickModeSettings);
    }
    return rc;
}


static int
processCmd(NEXUS_PlaybackHandle playback, const char *s)
{
    NEXUS_Error rc;

    const char *end = s+strcspn(s, " \t");
    const char *param = NULL;
    if (end)
        param = end+strspn(end, " \t");


#define CMD(STR) (!strncmp(s, STR, end-s))  
    if (CMD("?") || CMD("help"))
    {
        printMenu();
    }
    else if (CMD("q"))
    {
        return 1;
    }
    else if (CMD("st"))
    {
        int track;
        char *cmd;      

        cmd = strtok((char *)param, " \t");

        if (cmd) {
            track = atoi(cmd);
            if(track<=subtitles->stitle_info->total_tracks){	
		    NEXUS_VideoDecoderStatus vstatus;
		    uint32_t display_time;
		    int trick_rate = trick_rate_table[state.trickModeRate];

		    NEXUS_VideoDecoder_GetStatus(gHandles.videoDecoder, &vstatus);
		    display_time = bmedia_pts2time(vstatus.pts, trick_rate?(trick_rate*BMEDIA_TIME_SCALE_BASE):BMEDIA_TIME_SCALE_BASE);
		    StopSubtitleDecode();
		    clear_subtitle(gHandles.surfaceHD);	
		    clear_subtitle(gHandles.surfaceSD);
		    bstitle_parser_flush(subtitles);
		    if(chapter_timer){
			    B_Scheduler_CancelTimer(gData.scheduler, chapter_timer);
			    chapter_timer = NULL;
		    }
		    if(track>0) {
			BDBG_WRN(("Displaying Subtitle Track %d", track));		    
			subtitles = bstitle_switch((bstitle_parse_t)subtitles, track);
			/* Certification requires that we seek backward or minimize the seek in fwd direction */
			/* For Avi we can use accurate seek. Mkv does not support accurate seek. so we hack here to seek back 3s from current lcoation */
			if(cur_track_info->type == bstream_mpeg_type_mkv) {
			    NEXUS_Playback_Seek(gHandles.playback, display_time<3000?0:(display_time-3000));						    
			} else if ( cur_track_info->type == bstream_mpeg_type_avi) {			    
			    NEXUS_Playback_Seek(gHandles.playback, display_time);			    
			}
			if(subtitles){	
			    StartSubtitleDecode(&gDecodeSettings);
			    subtitles->stitle_info->on = true;
			}			
		    } else {
			subtitles->stitle_info->on = false;
		    }
            }
            else {
                BDBG_WRN(("Max Subtitle Tracks Found is : %d", subtitles->stitle_info->total_tracks));
                BDBG_WRN(("Please Enter a Track number between 1 - %d", subtitles->stitle_info->total_tracks));
            }
        }
    }
    else if (CMD("at"))
    {
        int track;
        char *cmd;

        cmd = strtok((char *)param, " \t");

        if (cmd) {
            track = atoi(cmd);
            if (track<=cur_track_info->total_audio_tracks && track>0){
		NEXUS_VideoDecoderStatus vstatus;
		uint32_t display_time;
		int trick_rate = trick_rate_table[state.trickModeRate];

		printf("Switching to Audio Track %d\n", track);
		
		NEXUS_VideoDecoder_GetStatus(gHandles.videoDecoder, &vstatus);
		display_time = bmedia_pts2time(vstatus.pts, trick_rate?(trick_rate*BMEDIA_TIME_SCALE_BASE):BMEDIA_TIME_SCALE_BASE);
		if(chapter_timer){
			    B_Scheduler_CancelTimer(gData.scheduler, chapter_timer);
			    chapter_timer = NULL;
		}
				
		StopAudioDecode();			
		StartAudioDecode(&gDecodeSettings,track-1, true);
		/* Certification requires that we seek backward or minimize the seek in fwd direction */
		if(cur_track_info->type == bstream_mpeg_type_mkv) {
		    NEXUS_Playback_Seek(gHandles.playback, display_time<3000?0:(display_time-3000));
		} else if ( cur_track_info->type == bstream_mpeg_type_avi) {		    
		    NEXUS_Playback_Seek(gHandles.playback, display_time);
		}
            }
            else {
                printf("Max Audio Tracks Found is : %d\n", cur_track_info->total_audio_tracks);
                printf("Please Enter a Track number between 1 - %d\n", cur_track_info->total_audio_tracks);
            }
        }
    }
    else if (CMD("rew"))
    {
	    trickMode(playback, 1, -1);
    }
    else if (CMD("ff"))
    {	    
	    trickMode(playback, 1, 1);
    }
    else if (CMD("play"))
    {
	    trickMode(playback, 0, 0);
    }
    else if (CMD("displayHD"))
    {
         char *cmd;
         NEXUS_DisplaySettings displayCfg;

	 cmd = strtok((char *)param, " \t");

	 /* clear_subtitle(gHandles.surfaceHD); */	
	 /* clear_subtitle(gHandles.surfaceSD); */

	 NEXUS_Display_GetSettings(gHandles.displayHD, &displayCfg);

        if(!strcmp(cmd,"480i"))
        {
		printf("\nChanging Format to 480i\n");
		displayCfg.format = NEXUS_VideoFormat_eNtsc;
		displayCfg.aspectRatio =  NEXUS_DisplayAspectRatio_e4x3;
		Display_SetOutput(&displayCfg,0,0);
        } else if(!strcmp(cmd,"480p"))
	{
		printf("\nChanging Format to 480p\n");
		displayCfg.format = NEXUS_VideoFormat_e480p;
		displayCfg.aspectRatio =  NEXUS_DisplayAspectRatio_e4x3;
		Display_SetOutput(&displayCfg,0,0);
        } else if(!strcmp(cmd,"576i"))
	{
		printf("\nChanging Format to 576i\n");
		displayCfg.format = NEXUS_VideoFormat_ePal;
		displayCfg.aspectRatio =  NEXUS_DisplayAspectRatio_e4x3;
		Display_SetOutput(&displayCfg,0,1);
        } else if(!strcmp(cmd,"576p"))
	{
		printf("\nChanging Format to 576p\n");
		displayCfg.format = NEXUS_VideoFormat_e576p;
		displayCfg.aspectRatio =  NEXUS_DisplayAspectRatio_e4x3;
		Display_SetOutput(&displayCfg,0,1);
        } else if(!strcmp(cmd,"720p"))
	{
		printf("\nChanging Format to 720p\n");
		displayCfg.format = NEXUS_VideoFormat_e720p;
		displayCfg.aspectRatio =  NEXUS_DisplayAspectRatio_e16x9;
		Display_SetOutput(&displayCfg,0,0);
        } else if(!strcmp(cmd,"1080i"))
	{
		printf("\nChanging Format to 1080i\n");
		displayCfg.format = NEXUS_VideoFormat_e1080i;
		displayCfg.aspectRatio =  NEXUS_DisplayAspectRatio_e16x9;
		Display_SetOutput(&displayCfg,0,0);
        } else 
		printf("Not Supported Format\n");
    }
    else if (CMD("displaySD"))
    {
	char *cmd;
	NEXUS_DisplaySettings displayCfg;		

	cmd = strtok((char *)param, " \t");
	NEXUS_Display_GetSettings(gHandles.displaySD, &displayCfg);
	    
	if(!strcmp(cmd,"480i"))
	{
	    printf("\nChanging Format to 480i\n");
	    displayCfg.format = NEXUS_VideoFormat_eNtsc;
	    Display_SetOutput(&displayCfg,1,0);
	} else  if(!strcmp(cmd,"576i"))
	{
	    printf("\nChanging Format to 576i\n");
	    displayCfg.format = NEXUS_VideoFormat_ePal;
	    Display_SetOutput(&displayCfg,1,1);
	}else 
	    printf("Not Supported Format for SD Display \n");	 
    }
    else if (CMD("stat")) {
	NEXUS_VideoDecoderStatus vstatus;
	NEXUS_AudioDecoderStatus astatus;
	NEXUS_PlaybackStatus pstatus;
	uint32_t stc;

	rc = NEXUS_VideoDecoder_GetStatus(gHandles.videoDecoder, &vstatus);
	BDBG_ASSERT(!rc);

	rc = NEXUS_AudioDecoder_GetStatus(gHandles.audioDecoder, &astatus);
	BDBG_ASSERT(!rc);

	rc = NEXUS_Playback_GetStatus(gHandles.playback, &pstatus);
	BDBG_ASSERT(!rc);
	NEXUS_StcChannel_GetStc(gHandles.stcChannel, &stc);

	if (gHandles.videoPid) {
	    printf("video %d/%d (%d%%) pts=%#x, stc=%#x (diff %d) \n", vstatus.fifoDepth, vstatus.fifoSize,
		   vstatus.fifoSize ? vstatus.fifoDepth * 100 / vstatus.fifoSize : 0,
		   vstatus.pts, stc, vstatus.pts - stc);
	    if (vstatus.avdStatusBlock) {
		printf("  avdStatusBlock=%#x\n", vstatus.avdStatusBlock);
	    }
	}

	if (gHandles.audioPid) {
	    printf("audio %d/%d (%d%%) pts=%#x, stc=%#x (diff %d)\n", astatus.fifoDepth, astatus.fifoSize,
		   astatus.fifoSize ? astatus.fifoDepth * 100 / astatus.fifoSize : 0,
		   astatus.pts, stc, astatus.pts - stc);
	}

	printf("playback %d/%d (%d%%) pos=%u:%02u.%03u(%d) last=%u:%02u.%03u\n", pstatus.fifoDepth, pstatus.fifoSize,
	       pstatus.fifoSize ? pstatus.fifoDepth * 100 / pstatus.fifoSize : 0,
	       (unsigned)pstatus.position/60000,
	       (unsigned)(pstatus.position%60000)/1000,
	       (unsigned)pstatus.position%1000,
	       (int)(pstatus.readPosition - pstatus.position),
	       (unsigned)pstatus.last/60000,
	       (unsigned)(pstatus.last%60000)/1000,
	       (unsigned)pstatus.last%1000
	    );
    }
    else if (CMD("chapter")) {
	    if(cur_track_info->type == bstream_mpeg_type_mkv) {
		    unsigned chapter_cnt, chapter_num=0;
		    char *cmd;
		    b_edition_info *edition_info;
		    b_chapter_info *chapter_info;

		    cmd = strtok((char *)param, " \t");
		    if(cmd) 
		      chapter_num = atoi(cmd);

		    edition_info = cur_edition_info;

		    if(edition_info) {
			    for(chapter_info=BLST_Q_FIRST(&edition_info->chapters),chapter_cnt=1;chapter_info;chapter_info=BLST_Q_NEXT(chapter_info, link),chapter_cnt++){
				    if(!chapter_num && chapter_info && !chapter_info->ChapterFlagHidden){
					    printf("%d :\tTime [%02u:%02u.%03u -> %02u:%02u.%03u]\t%s %s %s %s\n", chapter_cnt, 
						   chapter_info->ChapterTimeStart/60000, (chapter_info->ChapterTimeStart/1000)%60, chapter_info->ChapterTimeStart%1000, 
						   chapter_info->ChapterTimeEnd/60000, (chapter_info->ChapterTimeEnd/1000)%60, chapter_info->ChapterTimeEnd%1000, 						  
						   chapter_info->ChapString, chapter_info->ChapLanguage, chapter_info->ChapCountry,
						   (cur_chapter_info == chapter_info)?"*":"");				   
				    } else if (chapter_num==chapter_cnt) {
					    break;
				    }
			    }	    			       
	    
			    if(chapter_info && !chapter_info->ChapterFlagHidden){
				    printf("Starting Chapter %d Time [%02u:%02u.%03u -> %02u:%02u.%03u]\n", chapter_num, 
					   chapter_info->ChapterTimeStart/60000, (chapter_info->ChapterTimeStart/1000)%60, chapter_info->ChapterTimeStart%1000, 
					   chapter_info->ChapterTimeEnd/60000, (chapter_info->ChapterTimeEnd/1000)%60, chapter_info->ChapterTimeEnd%1000);
				    if(chapter_timer){
					    B_Scheduler_CancelTimer(gData.scheduler, chapter_timer);
					    chapter_timer = NULL;
				    }
				    cur_chapter_info = chapter_info;
				    NEXUS_Playback_Seek(gHandles.playback, chapter_info->ChapterTimeStart);
			    } 
		    }
	    } else {
		    printf("Command not supported for Current Stream Type\n");
	    }	    
    }
    else if (CMD("edition")) {
	    if(cur_track_info->type == bstream_mpeg_type_mkv) {
		    unsigned edition_cnt, edition_num=0;
		    char *cmd;
		    b_edition_info *edition_info;
		    b_chapter_info *chapter_info;

		    cmd = strtok((char *)param, " \t");
		    if(cmd) 
		      edition_num = atoi(cmd);
		    
		    for(edition_info=BLST_Q_FIRST(&cur_track_info->editions),edition_cnt=1;edition_info;edition_info=BLST_Q_NEXT(edition_info, link),edition_cnt++){
			    if(!edition_num && edition_info && !edition_info->EditionFlagHidden){
				    printf("Edition %d\n", edition_cnt);
  
			    } else if (edition_num==edition_cnt) {
				    break;
			    }		    
		    }
		    
		    if(edition_info && edition_info != cur_edition_info) {			    
			    printf("Starting Edition %d\n", edition_num);

			    if(chapter_timer){
				    B_Scheduler_CancelTimer(gData.scheduler, chapter_timer);
				    chapter_timer = NULL;
			    }
			    chapter_info = BLST_Q_FIRST(&edition_info->chapters);
			    cur_edition_info = edition_info;
			    cur_chapter_info = chapter_info;
			    NEXUS_Playback_Seek(gHandles.playback, chapter_info->ChapterTimeStart);			    
		    }
	    } else {
		    printf("Command not supported for Current Stream Type\n");
	    }
    }
    else if(CMD("1") || CMD("2") || CMD("3") || CMD("4") || CMD("5") ||
	    CMD("6") || CMD("7") || CMD("8") || CMD("9")|| CMD("10")) {
	    if(cur_track_info->type == bstream_mpeg_type_mkv) {
		    char *cmd;
		    unsigned chapter_num=0;
		    unsigned long position=0;
		    bool auto_chapter_enabled = false;
		    NEXUS_PlaybackStatus pstatus;

		    cmd = strtok((char *)s, " \t");
		    if(cmd) 
		      chapter_num = atoi(cmd) - 1;

		    if(cur_edition_info && cur_edition_info->EditionFlagOrdered){
			    b_chapter_info *chapter_info, *prev_chapter_info=NULL;
			    unsigned long duration;
			    bool discontinuity = false;
			    			    
			    /* Get duration by calculating length of each chapter */
			    for(chapter_info=BLST_Q_FIRST(&cur_edition_info->chapters),duration=0;chapter_info;chapter_info=BLST_Q_NEXT(chapter_info, link)){
				    duration += chapter_info->ChapterTimeEnd - chapter_info->ChapterTimeStart;
				    if(prev_chapter_info && chapter_info->ChapterTimeStart!=prev_chapter_info->ChapterTimeEnd)
				      discontinuity = true;
				    prev_chapter_info = chapter_info;
			    }
			    
			    position = chapter_num * (duration/10);
			    BDBG_MSG(("Seeking to Position %02u:%02u.%03u \n", position/60000, (position/1000)%60, position%1000));

			    for(chapter_info=BLST_Q_FIRST(&cur_edition_info->chapters), duration=0;chapter_info;chapter_info=BLST_Q_NEXT(chapter_info, link)){
				    duration += chapter_info->ChapterTimeEnd - chapter_info->ChapterTimeStart;
				    if(position<duration)
				      break;
			    }
			    if(discontinuity) {
				    position = chapter_info->ChapterTimeEnd - (duration - position);			    
			    }
			    cur_chapter_info = chapter_info;
			    auto_chapter_enabled = true;
		    } else if(!cur_edition_info){
			    rc = NEXUS_Playback_GetStatus(gHandles.playback, &pstatus);
			    BDBG_ASSERT(!rc);
			    position = chapter_num * (pstatus.last/10);
			    auto_chapter_enabled = true;
		    } else {
			    printf("Auto Generated Chapters not supported\n");
		    }

		    if(auto_chapter_enabled){
			    printf("Starting Auto generated chapter %d\n", chapter_num+1);

			    if(subtitles){
				    StopSubtitleDecode();
				    clear_subtitle(gHandles.surfaceHD);
				    clear_subtitle(gHandles.surfaceSD);
				    bstitle_parser_flush(subtitles);
			    }
			    if(chapter_timer){
				    B_Scheduler_CancelTimer(gData.scheduler, chapter_timer);
				    chapter_timer = NULL;
			    }			    
			    rc = NEXUS_Playback_Seek(gHandles.playback, position);

			    if(subtitles && subtitles->stitle_info->on){
				    StartSubtitleDecode(&gDecodeSettings);
			    }
		    }
	    } else {
		    printf("Command not supported for Current Stream Type\n");
	    }
    }
    else if(CMD("title")){
	    if(cur_track_info->type == bstream_mpeg_type_mkv) {
		    b_titles_info *title_info=NULL;
		    unsigned title_num=0;
		    char *cmd;
		    
		    cmd = strtok((char *)param, " \t");
		    if(cmd) 
		      title_num = atoi(cmd);
		    
		    title_info = bstitle_get_title(title_num);
		    
		    if(title_info){
			    NEXUS_VideoDecoderStatus vstatus;
			    int trick_rate = trick_rate_table[state.trickModeRate];
			    NEXUS_VideoDecoder_GetStatus(gHandles.videoDecoder, &vstatus);
			    cur_title_info->last_display_time = bmedia_pts2time(vstatus.pts, trick_rate?(trick_rate*BMEDIA_TIME_SCALE_BASE):BMEDIA_TIME_SCALE_BASE);
			    switch_title(title_info, true, title_info->last_display_time, 1, 0); /* Subtitle track starts from 1. Audio track index start from 0 */			    
		    }
	    } else {
		    printf("Command not supported for Current Stream Type\n");
	    }	    
    }
    else
    {
        printf("Unknown command: %s\n", s);
    }
    return 0;
}

static int
processCmdString(NEXUS_PlaybackHandle playback, char *s)
{
    char *cmd;
    char *end = s+strlen(s);
    cmd = strtok(s, ";");
    while (cmd) {
        /* strtok will be reused again inside processCmd,
        and it means it will change strtok global state.
        This is a workaround. */
        char *testend = cmd + strlen(cmd);
        cmd += strspn(cmd, " \t");
        if (processCmd(playback, cmd))
            return 1;
        if (testend == end)
            break;
        cmd = strtok(testend+1, ";");
    }
    return 0;
}


static void
DVRMenu(NEXUS_PlaybackHandle playback)
{
    char s[256];

    printMenu();
    while (1)
    {
        char *cmd;

        printf(">");
        cmd = fgets(s,sizeof(s)-1,stdin);
        if (cmd==NULL) {
            break;
        }
        cmd = strchr(s, '\n');
        if (cmd) {
            *cmd = '\0';
        }

        if (processCmdString(playback, s))
            break;
    }
}


int main(int argc, const char *argv[])
{
    b_stitle_cfg_params cfg_params;
    NEXUS_Error rc;

    uint32_t trackCount;
    uint32_t trackNo=0;
    btsm_queue_config tsm_queue_config;
    btsm_queue_settings tsm_queue_settings;

    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlatformSettings platformSettings;    
    B_Error errCode;
    Subtitle_DecodeSettings decodeSettings;    
    
    const char *fontname = "AriBlk.ttf";
    int default_font_size = 54;

    NEXUS_SurfaceMemory mem;    
    NEXUS_DisplaySettings displaySettings;    
#if 0/* NEXUS_HAS_SYNC_CHANNEL */
    NEXUS_SyncChannelSettings syncChannelSettings;
#endif   
    NEXUS_Graphics2DSettings gfxSettings; 
    NEXUS_VideoDecoderSettings video_settings;			    			    	    

    if(argc<2){
        printf("Usage: nexus subtitle [filename]\n");
        return 1;
    }
    else{
        filename = argv[1];
    }            

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    platformSettings.displayModuleSettings.vbi.allowCgmsB = true;

    rc = NEXUS_Platform_Init(&platformSettings);
    BDBG_ASSERT(!rc);

    /* Initializes Nexus OS Lib */
    errCode = B_Os_Init();
    BDBG_ASSERT(errCode == B_ERROR_SUCCESS);

    memset(&gData, 0, sizeof(gData));

    gData.scheduler = B_Scheduler_Create(NULL);
    BDBG_ASSERT(NULL != gData.scheduler);

    /* Go ahead and launch the scheduler thread now */
    gData.thread = B_Thread_Create("scheduler", scheduler_callback, &gData, NULL);
    BDBG_ASSERT(NULL != gData.thread);

    gData.mutex = B_Mutex_Create(NULL);
    BDBG_ASSERT(NULL != gData.mutex);

    BKNI_CreateEvent(&gData.event);
    BKNI_CreateEvent(&gData.subtitle_event);

    btsm_queue_get_default_config(&tsm_queue_config);
    tsm_queue_config.length = 128;
    tsm_queue = btsm_queue_create(&tsm_queue_config);
    BDBG_ASSERT(NULL != tsm_queue);
    btsm_queue_get_settings(tsm_queue, &tsm_queue_settings);    
    btsm_queue_start(tsm_queue, &tsm_queue_settings);
    
    cfg_params.app_cb = sync_set_subtitle;
    cfg_params.cnxt = NULL;
    subtitles = bstitle_parse_init((char*)filename, &cfg_params);

#ifndef DIVX_PROFILE_HDPLUS
    if(cur_track_info->type == bstream_mpeg_type_mkv){
	BDBG_WRN(("MKV tracks are not supported for this DivX Profile"));
	bstitle_parse_uninit(subtitles);
	return 1;
    }
#endif
   
    BDBG_MSG(("total_video_tracks = %d, totalaudiotracks = %d",
	      cur_track_info->total_video_tracks, cur_track_info->total_audio_tracks));

    /* Initialize graphics */
    gHandles.graphics = NEXUS_Graphics2D_Open(0, NULL);
    BDBG_ASSERT(gHandles.graphics);   
    
    BKNI_CreateEvent(&gHandles.checkpointEvent);
    BKNI_CreateEvent(&gHandles.packetSpaceAvailableEvent);
    
    NEXUS_Graphics2D_GetSettings(gHandles.graphics, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = gHandles.checkpointEvent;
    gfxSettings.packetSpaceAvailable.callback = complete;
    gfxSettings.packetSpaceAvailable.context = gHandles.packetSpaceAvailableEvent;
    NEXUS_Graphics2D_SetSettings(gHandles.graphics, &gfxSettings);

    /* Initialize display and audio */
    InitDisplay();    

    if(subtitles){
	    NEXUS_SurfaceCreateSettings createSettings;	    

	    /* HD buffer for subtitles */	    
	    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
	    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;       
	    createSettings.width = HD_WIDTH;
	    createSettings.height = HD_HEIGHT;
	    gHandles.surfaceHD = NEXUS_Surface_Create(&createSettings);
	    BDBG_ASSERT(gHandles.surfaceHD);
	    
	    clear_subtitle(gHandles.surfaceHD);

	    /* SD buffer for subtitles */	    
	    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
	    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;       
	    createSettings.width = SD_WIDTH;
	    createSettings.height = SD_HEIGHT;
	    gHandles.surfaceSD = NEXUS_Surface_Create(&createSettings);
	    BDBG_ASSERT(gHandles.surfaceSD);
	    
	    clear_subtitle(gHandles.surfaceSD);

	    /* Offscreen buffer for subtitles. Offscreen surce create settings are stores in gHandles */
	    NEXUS_Surface_GetDefaultCreateSettings(&gHandles.createSettings);
	    gHandles.createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;       
	    gHandles.createSettings.width = HD_WIDTH;
	    gHandles.createSettings.height = HD_HEIGHT;
	    gHandles.offscreen = NEXUS_Surface_Create(&gHandles.createSettings);
	    BDBG_ASSERT(gHandles.offscreen);	    
    
	    if(cur_track_info->type == bstream_mpeg_type_mkv){
		    NEXUS_Surface_GetMemory(gHandles.offscreen, &mem);

		    /* bring up bwin with a single framebuffer */
		    bwin_engine_settings_init(&gHandles.win_engine_settings);
		    gHandles.win_engine = bwin_open_engine(&gHandles.win_engine_settings);
		    BDBG_ASSERT(gHandles.win_engine);


		    gHandles.win_font_default = bwin_open_font(gHandles.win_engine, fontname, default_font_size, true);
		    BDBG_ASSERT(gHandles.win_font_default);

		    for(trackCount=0; trackCount < (uint32_t)subtitles->stitle_info->total_tracks; trackCount++){
			    if(subtitles->stitle_info->stitle_parse[trackCount]->font.data){
				    bwin_font_settings font_settings;
				    
				    font_settings.context = subtitles->stitle_info->stitle_parse[trackCount]->font.data;
				    font_settings.buf_len = subtitles->stitle_info->stitle_parse[trackCount]->font.size;
				    font_settings.size = default_font_size ;
				    font_settings.antialiased = true;				    
				    gHandles.win_font_attached[trackCount] = bwin_open_font_generic(gHandles.win_engine, &font_settings);			
				    BDBG_ASSERT(gHandles.win_font_attached[trackCount]);
			    }
		    }
	
		    bwin_framebuffer_settings_init(&gHandles.win_fb_settings);
		    gHandles.win_fb_settings.width = gHandles.createSettings.width;
		    gHandles.win_fb_settings.height = gHandles.createSettings.height;
		    gHandles.win_fb_settings.pitch = mem.pitch;
		    gHandles.win_fb_settings.buffer = mem.buffer;
		    gHandles.win_fb_settings.pixel_format = bwin_pixel_format_a8_r8_g8_b8;
		    gHandles.win_fb = bwin_open_framebuffer(gHandles.win_engine, &gHandles.win_fb_settings);
		    BDBG_ASSERT(gHandles.win_fb);
	    
		    /* get the toplevel window */
		    bwin_get_framebuffer_settings(gHandles.win_fb, &gHandles.win_fb_settings);
	    }

	    /*Allocated a temp buffer to handle partial messages*/
	    message_buffer = BKNI_Malloc(32*1024);	    
    }
    

    gHandles.playpump = NEXUS_Playpump_Open(0, NULL);
    BDBG_ASSERT(gHandles.playpump);

    gHandles.playback = NEXUS_Playback_Create();
    BDBG_ASSERT(gHandles.playback);

    /* AVI has an index at the end of file */
    gHandles.file = NEXUS_FilePlay_OpenPosix(filename, filename);
    if (!gHandles.file) {
        fprintf(stderr, "can't open file:%s\n", filename);
        goto error;
    }

    gHandles.orig_filehandle = gHandles.file;
    gHandles.file = BKNI_Malloc(sizeof(*gHandles.file));
    gHandles.file->file.data = bfile_read_offset_attach(gHandles.orig_filehandle->file.data, cur_title_info->file_offset);
    gHandles.file->file.index = bfile_read_offset_attach(gHandles.orig_filehandle->file.index, cur_title_info->file_offset);
    gHandles.file->file.close = NULL;
    
    NEXUS_Playback_GetSettings(gHandles.playback, &playbackSettings);
    playbackSettings.playpump = gHandles.playpump;
    playbackSettings.stcChannel = gHandles.stcChannel;
    playbackSettings.playpumpSettings.transportType = GetTransportType(cur_track_info->type);
    playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePause;
    /* For avi streams pauing at BOS causes stream to never actually reach BOS. PErhaps because BOF packet is never sent */ 
    /* Work around is to resume play at BOS and have the callback restart subtitles, fix trick rate, etc */
    playbackSettings.beginningOfStreamAction = (cur_track_info->type == bstream_mpeg_type_avi)?NEXUS_PlaybackLoopMode_ePlay:NEXUS_PlaybackLoopMode_ePause;
    playbackSettings.beginningOfStreamCallback.callback = (cur_track_info->type == bstream_mpeg_type_avi)?beginningOfStreamCallback:NULL;
    playbackSettings.accurateSeek = false /* true */; 

#ifdef B_HAS_DIVX_DRM
    BDBG_MSG(("Drm Init"));
    rc = drm_init(filename, (uint64_t)cur_title_info->file_offset);        
    if(rc)
	goto error;
    if(gHandles.genericKeyHandle) {	
	playbackSettings.playpumpSettings.securityContext = gHandles.genericKeyHandle;
    }
#endif    
    
    rc = NEXUS_Playback_SetSettings(gHandles.playback, &playbackSettings);
    BDBG_ASSERT(!rc);  

#if 0/* NEXUS_HAS_SYNC_CHANNEL */
    /* create a sync channel */
    NEXUS_SyncChannel_GetDefaultSettings(&syncChannelSettings);
    gHandles.syncChannel = NEXUS_SyncChannel_Create(&syncChannelSettings);
#endif

    /* Initialize and start decode */
    Decode_GetDefaultSettings(&decodeSettings);

    for(trackCount=0; trackCount < (uint32_t)cur_track_info->total_video_tracks; trackCount++){
        decodeSettings.videoPid[trackCount] = (uint16_t)cur_track_info->video_info[trackCount].track_id;
        decodeSettings.videoCodec[trackCount] = GetVideoCodec(cur_track_info->video_info[trackCount].codec);
        BDBG_WRN(("[Video] trackNo=%d Pid=%d Codec=%d", trackCount, decodeSettings.videoPid[trackCount],
		  decodeSettings.videoCodec[trackCount]));
    }

    for(trackCount=0; trackCount < (uint32_t)cur_track_info->total_audio_tracks; trackCount++){
        decodeSettings.audioPid[trackCount] = (uint16_t)cur_track_info->audio_info[trackCount].track_id;
        decodeSettings.audioCodec[trackCount] = GetAudioCodec(cur_track_info->audio_info[trackCount].codec);
        BDBG_WRN(("[Audio] trackNo=%d Pid=%d Codec=%d", trackCount, decodeSettings.audioPid[trackCount],
		  decodeSettings.audioCodec[trackCount]));
    }
    
    if(subtitles){
	    for(trackCount=0; trackCount < (uint32_t)subtitles->stitle_info->total_tracks; trackCount++){
		    decodeSettings.subtitlePid[trackCount] = (uint16_t)subtitles->stitle_info->stitle_parse[trackCount]->track_number;     
		    BDBG_WRN(("[Subtitle] trackNo=%d Pid=%d", trackCount, decodeSettings.subtitlePid[trackCount]));
	    }
    }
    
    gHandles.videoWidth=cur_track_info->video_info[0].width;
    gHandles.videoHeight=cur_track_info->video_info[0].height;    
    if(cur_track_info->video_info[0].display_width && cur_track_info->video_info[0].display_height){	   
	    if(cur_track_info->video_info[0].display_width == cur_track_info->video_info[0].width && cur_track_info->video_info[0].display_height == cur_track_info->video_info[0].height){
		    gHandles.aspect_ratio = NEXUS_AspectRatio_eSquarePixel;
	    } else if((cur_track_info->video_info[0].display_width*9/cur_track_info->video_info[0].display_height)>=19) {
		    gHandles.aspect_ratio = NEXUS_AspectRatio_e221x1;
	    } else if((cur_track_info->video_info[0].display_width*9/cur_track_info->video_info[0].display_height)>=15) {
		    gHandles.aspect_ratio = NEXUS_AspectRatio_e16x9;
	    } else {	    
		    gHandles.aspect_ratio = NEXUS_AspectRatio_e4x3;
	    }
    } else {
	    gHandles.aspect_ratio = NEXUS_AspectRatio_eSquarePixel;
    }

    decodeSettings.playback = gHandles.playback;
    decodeSettings.playpump = gHandles.playpump;

    BKNI_Memcpy(&gDecodeSettings, &decodeSettings, sizeof(decodeSettings));

    InitVideoDecode(&decodeSettings, cur_track_info->total_video_tracks);    
    InitAudioDecode(&decodeSettings, cur_track_info->total_audio_tracks);
    if(subtitles){
	    InitSubtitleDecode(&decodeSettings, subtitles->stitle_info->total_tracks, gData.subtitle_event);
    }    

#if 0/* NEXUS_HAS_SYNC_CHANNEL   */  
    /* connect sync channel */
    NEXUS_SyncChannel_GetSettings(gHandles.syncChannel, &syncChannelSettings);
    syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(gHandles.videoDecoder);
    if(gHandles.audioDecoder) {
	    syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(gHandles.audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
    }
    if(gHandles.audioPassthrough) {
	    syncChannelSettings.audioInput[1] = NEXUS_AudioDecoder_GetConnector(gHandles.audioPassthrough, NEXUS_AudioDecoderConnectorType_eCompressed);
    }
    NEXUS_SyncChannel_SetSettings(gHandles.syncChannel, &syncChannelSettings);
#endif   

    StartVideoDecode(&decodeSettings, trackNo, true);
    StartAudioDecode(&decodeSettings, trackNo, true);
    if(subtitles && subtitles->stitle_info->on){
	    StartSubtitleDecode(&decodeSettings);
    }    

    if(cur_track_info->type == bstream_mpeg_type_mkv) {
	if(cur_edition_info) {	    
	    NEXUS_VideoDecoder_GetSettings(gHandles.videoDecoder, &video_settings);
	    video_settings.firstPtsPassed.callback = first_pts_passed;
	    video_settings.firstPtsPassed.context = &cur_chapter_info;
	    NEXUS_VideoDecoder_SetSettings(gHandles.videoDecoder, &video_settings);
	}
    } 

    rc = NEXUS_Playback_Start(gHandles.playback, gHandles.file, NULL);
    if (rc) {	   
	    /* NEXUS_Playback_Start may fail if the AVI file has no index.
	       If so, reopen without an index and try again. */
	    if(gHandles.orig_filehandle){
		    bfile_read_offset_detach(gHandles.file->file.data);
		    bfile_read_offset_detach(gHandles.file->file.index);
		    BKNI_Free(gHandles.file);
		    gHandles.file = gHandles.orig_filehandle;
	    }
	    NEXUS_FilePlay_Close(gHandles.file);

	    gHandles.file = NEXUS_FilePlay_OpenPosix(filename, NULL);
	    BDBG_ASSERT(gHandles.file);

	    gHandles.orig_filehandle = gHandles.file;
	    gHandles.file = BKNI_Malloc(sizeof(*gHandles.file));
	    gHandles.file->file.data = bfile_read_offset_attach(gHandles.orig_filehandle->file.data, cur_title_info->file_offset);
	    gHandles.file->file.index = NULL;
	    gHandles.file->file.close = NULL;
	    
	    rc = NEXUS_Playback_Start(gHandles.playback, gHandles.file, NULL);
	    if (rc) goto error;
    }

    NEXUS_Display_GetSettings(gHandles.displayHD, &displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e1080i;
    displaySettings.aspectRatio =  NEXUS_DisplayAspectRatio_e16x9;
    Display_SetOutput(&displaySettings,0,0);
    
    NEXUS_Display_GetSettings(gHandles.displaySD, &displaySettings);
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    displaySettings.aspectRatio =  NEXUS_DisplayAspectRatio_e4x3;
    Display_SetOutput(&displaySettings,1,0);
    
    /* Start a thread that waits for message event when subtitles are available */
    if(subtitles){
	    gData.subtitle_thread = B_Thread_Create("subtitle", subtitle_feed, &gData, NULL);
	    BDBG_ASSERT(NULL != gData.subtitle_thread);
	    
	    sync_timer=B_Scheduler_StartTimer(gData.scheduler, gData.mutex, SUBTITLE_TIMER_INTERVAL, sync_callback, NULL);
    }

    DVRMenu(gHandles.playback);

    if(sync_timer){
	    B_Scheduler_CancelTimer(gData.scheduler, sync_timer);
	    sync_timer = NULL;
    }
    if(chapter_timer){
	    B_Scheduler_CancelTimer(gData.scheduler, chapter_timer);
	    chapter_timer = NULL;
    }
    btsm_queue_stop(tsm_queue);
    btsm_queue_destroy(tsm_queue);
    bstitle_parse_uninit(subtitles);   

    /* shutdown */
    BDBG_WRN(("Shut down..."));
    NEXUS_Playback_Stop(gHandles.playback);

#if 0/* NEXUS_HAS_SYNC_CHANNEL  */   
    /* disconnect sync channel */
    NEXUS_SyncChannel_GetSettings(gHandles.syncChannel, &syncChannelSettings);
    syncChannelSettings.videoInput = NULL;
    syncChannelSettings.audioInput[0] = NULL;
    syncChannelSettings.audioInput[1] = NULL;
    NEXUS_SyncChannel_SetSettings(gHandles.syncChannel, &syncChannelSettings);

    NEXUS_SyncChannel_Destroy(gHandles.syncChannel);
#endif

    StopAllDecode();

    if(gHandles.orig_filehandle){
	    bfile_read_offset_detach(gHandles.file->file.data);
	    if(gHandles.file->file.index){
		    bfile_read_offset_detach(gHandles.file->file.index);
	    }
	    BKNI_Free(gHandles.file);
	    gHandles.file = gHandles.orig_filehandle;
    }
    NEXUS_FilePlay_Close(gHandles.file);
    NEXUS_Playback_Destroy(gHandles.playback);
    NEXUS_Playpump_Close(gHandles.playpump);
    UninitDisplay();

#ifdef B_HAS_DIVX_DRM  
    if(gHandles.divxDrm) {
	drm_uninit();
    }
#endif

#if 0
    if(gHandles.win_font_default){
	    bwin_close_font(gHandles.win_font_default);
    }
    for(trackCount=0; trackCount < (uint32_t)subtitles->stitle_info->total_tracks; trackCount++){
	    if(subtitles->stitle_info->stitle_parse[trackCount]->font.data){
		    bwin_close_font(gHandles.win_font_attached[trackCount]);
	    }
    }
#endif

    if(gHandles.checkpointEvent) {
	BKNI_DestroyEvent(gHandles.checkpointEvent);
    }
    if(gHandles.packetSpaceAvailableEvent) {
	BKNI_DestroyEvent(gHandles.packetSpaceAvailableEvent);
    }
    if(gHandles.win_fb){
	    bwin_close_framebuffer(gHandles.win_fb);
    }
    if(gHandles.win_engine){
	    bwin_close_engine(gHandles.win_engine);
    }
    
    if(gHandles.surfaceHD){
	    NEXUS_Surface_Destroy(gHandles.surfaceHD);
    }
    if(gHandles.surfaceSD){
	    NEXUS_Surface_Destroy(gHandles.surfaceSD);
    }
    if(gHandles.offscreen){
	    NEXUS_Surface_Destroy(gHandles.offscreen);
    }
    NEXUS_Graphics2D_Close(gHandles.graphics);

    if (gData.scheduler)
    {
        B_Scheduler_Stop(gData.scheduler);
        B_Scheduler_Destroy(gData.scheduler);
    }

    if(message_buffer){
	    BKNI_Free(message_buffer);
    }

    gHandles.subtitle_started = false;
    BKNI_SetEvent(gData.subtitle_event);

    if(gData.subtitle_thread)
        B_Thread_Destroy(gData.subtitle_thread);

    if (gData.thread)
        B_Thread_Destroy(gData.thread);

    if(gData.event){
	    BKNI_DestroyEvent(gData.event);
    }
    if(gData.subtitle_event){
	    BKNI_DestroyEvent(gData.subtitle_event);
    }

    errCode = B_Os_Uninit();
    BDBG_ASSERT(errCode == B_ERROR_SUCCESS);    

    NEXUS_Platform_Uninit();

    return 0;
error:
    return 1;
}

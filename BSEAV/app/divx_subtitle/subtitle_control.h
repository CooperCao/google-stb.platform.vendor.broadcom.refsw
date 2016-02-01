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

#include "bmkv_probe.h"
#include "bavi_probe.h"
#include "blst_queue.h"
#define DATA_BUFFERS 25*3
#define MAX_TRACKS 8

typedef struct bstitle_parse * bstitle_parse_t;
typedef struct bstitle_info * bstitle_info_t;

typedef struct bsubtitle_palette
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} b_subtitle_palette;


typedef struct bsubtitle_hdr
{    
    /*     Duration format : [HH:MM:SS.XXX-hh:mm:ss.xxx] */    
    /*     NOTE: This string is not null terminated! */
	  
    int8_t duration[27];           
	  
    /*     Dimensions and coordinates */
	  
    uint16_t width;                
    uint16_t height;               
    uint16_t left;                 
    uint16_t top;                  
    uint16_t right;                
    uint16_t bottom;               
    uint16_t field_offset;         
	  
    /*     2-bit (4 color) palette */
	  
    b_subtitle_palette background;   
    b_subtitle_palette pattern1;      
    b_subtitle_palette pattern2;    
    b_subtitle_palette pattern3;
	  
    uint8_t background_transparency;
    uint8_t pattern1_transparency;
    uint8_t pattern2_transparency;
    uint8_t pattern3_transparency;

    /*     Rle data */
	  
    uint8_t *rleData;              
} b_subtitle_hdr;

typedef struct bstitle_cfg_params
{
    void * cnxt;
    void (*app_cb)(void * cnxt, int32_t start_time, int32_t end_time);
} b_stitle_cfg_params;

typedef struct b_chapter_info {
    BLST_Q_ENTRY(b_chapter_info) link; /* this field is used to link chapters together */  
    uint64_t ChapterUID;
    uint32_t ChapterTimeStart;
    uint32_t ChapterTimeEnd;
    uint32_t ChapterFlagHidden;
    uint32_t ChapterFlagEnabled;
    char *ChapString;
    char *ChapLanguage;
    char *ChapCountry;
} b_chapter_info;

typedef BLST_Q_HEAD(b_chapter_info_list, b_chapter_info) b_chapter_info_list;

typedef struct b_edition_info {
    BLST_Q_ENTRY(b_edition_info) link; /* this field is used to link editions together */
    uint64_t EditionUID;
    uint32_t EditionFlagHidden;
    uint32_t EditionFlagDefault;
    uint32_t EditionFlagOrdered;
    b_chapter_info_list chapters;
} b_edition_info;

typedef BLST_Q_HEAD(b_edition_info_list, b_edition_info) b_edition_info_list;

typedef struct track_info{
    struct {
	int track_id;
	bvideo_codec codec;
	uint32_t width;
	uint32_t height;
	uint32_t display_width;
	uint32_t display_height;
	uint64_t track_uid;
	uint64_t trick_track_uid;		    
	uint64_t master_track_uid;
	bool trick_track_flag;
    } video_info[MAX_TRACKS];
    struct {
	int track_id;
	baudio_format codec;
    } audio_info[MAX_TRACKS];	  
    int total_audio_tracks;
    int total_video_tracks; 
    bstream_mpeg_type type;
    b_edition_info_list editions;	  	  
} b_track_info;

struct bstitle_parse {
    unsigned track_number;
    unsigned track_index;
    uint32_t width;
    uint32_t height;
    uint8_t *raw_data[DATA_BUFFERS];
    unsigned raw_data_size;
    uint8_t rptr;
    uint8_t wptr;
    b_subtitle_hdr info;
    bool activated;
    b_stitle_cfg_params *cfg_params;     
    bstitle_info_t stitle_info;
    bavi_subtitle_type subtitle_type;
    unsigned txt_subtitle_position;
    struct {
	uint8_t *data;
	uint64_t size;
    } font;
};

typedef struct bstitle_info{
    bstitle_parse_t stitle_parse[MAX_TRACKS];
    int total_tracks;
    bool on;
} b_stitle_info;


typedef struct b_titles_info{
    BLST_SQ_ENTRY(b_titles_info) link;
    b_track_info track_info;
    b_stitle_info subtitle_info;
    off_t file_offset;
    struct b_titles_info *trick_play, *master;
    uint32_t last_display_time;
} b_titles_info;

typedef BLST_SQ_HEAD(titles_list, b_titles_info) titles_list;


#define BSUBTITLE_HDR_SIZE            (0x35)
#define BSUBTITLE_INFO_SIZE           (sizeof(b_subtitle_hdr)) 
#define B_CREATE_PALETTE(a,r,g,b) (a<<24 | r<<16 | g<<8 | b<<0)

bstitle_parse_t bstitle_parse_init(char *file, b_stitle_cfg_params *cfg_params);
void bstitle_parse_uninit(bstitle_parse_t subtitles);
void bstitle_activate(bstitle_parse_t stitle_parse);
void bstitle_deactivate(bstitle_parse_t stitle_parse);
uint8_t * bstitle_get_buffer(bstitle_parse_t stitle_parse); 
void bstitle_read_complete(bstitle_parse_t stitle_parse); 
bstitle_parse_t bstitle_switch(bstitle_parse_t stitle_parse, int track_id);
void bstitle_parser_bmp(bstitle_parse_t stitle_parse, uint8_t *buffer, size_t length);
void bstitle_parser_txt(bstitle_parse_t stitle_parse, uint8_t *buffer, size_t length);
void bstitle_parser_flush(bstitle_parse_t stitle_parse);
char * findchr (const char * str, int c, int len);
b_titles_info * bstitle_get_title(unsigned num);

/***************************************************************************
 *     Copyright (c) 2002-2010, Broadcom Corporation
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
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ****************************************************************************/
#ifndef _CHANNELLIST_H
#define _CHANNELLIST_H

#include "bstd.h"
#include "nexus_base_types.h"
#include "nexus_frontend.h"
#include "nexus_frontend_qam.h"
#include "nexus_video_types.h"
#include "nexus_audio_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_SHORT_PROGRAM_TITLE_LENGTH	7
#define MAX_PROGRAM_TITLE_LENGTH        128
#define MAX_CHANNEL_MAP					300
#define MAX_AUDIO_STREAM				4
#define MAX_AUX_STREAM					4
#define ISO936_CODE_LENGTH				3
#define TOTAL_CHAN						16

enum SERVICE_MEDIA
{
	CHANNEL_SERVICE_BROADCAST,
	CHANNEL_SERVICE_STREAMING,
	CHANNEL_SERVICE_DOWNLOAD
};

typedef struct {
    enum SERVICE_MEDIA  service_media;
	NEXUS_FrontendQamAnnex annex; /* To describe Annex A or Annex B format */
    NEXUS_FrontendQamMode modulation;
    unsigned int  frequency;
    unsigned int  symbolrate;
    unsigned int  program_number;
    unsigned int  source_id;
    unsigned int  ca_pid;
    unsigned int  video_pid;
    unsigned int  audio_pid[MAX_AUDIO_STREAM];
    unsigned int  pcr_pid;
	unsigned int  aux_pid[MAX_AUX_STREAM];
	unsigned int num_audio_streams;
	unsigned int num_aux_streams;
	unsigned int num_total_streams;
    NEXUS_VideoCodec video_format;
    NEXUS_AudioCodec audio_format[MAX_AUDIO_STREAM];
    char ip_addr[16];
    unsigned int  port_no;
    char program_title[MAX_PROGRAM_TITLE_LENGTH];
	char program_long_title[MAX_PROGRAM_TITLE_LENGTH];
    unsigned int  cci; // CCI per CCIF 2.0 spec
	unsigned int broadcast_flag;
    char protocol[32]; /* streaming protocol */
    char url[256]; /* URL for HTTP */
	unsigned int vc_number; /*virtual channel number*/
	bool hidden; /* if the channel is hidden channel*/
	unsigned int  tsid; /* transport stream ID*/
	unsigned char iso639[MAX_AUDIO_STREAM][ISO936_CODE_LENGTH]; /* language ISO639 code for audio stream*/
} channel_info_t;

typedef struct {
    channel_info_t *channel;
    int active_channels;
    int total_channels;
	int current_channel[TOTAL_CHAN];
	int version;
} channel_list_t;


int channel_list_init(channel_list_t *list, unsigned int  number_of_channels);
int channel_list_clear(channel_list_t *list);
void channel_list_destroy(channel_list_t *list);
int channel_list_add_channel(channel_list_t *list,  channel_info_t *channel_info);
int channel_list_insert_channel(channel_list_t *list,  channel_info_t *channel_info);
int channel_list_copy(channel_list_t *to_list, channel_list_t *from_list);
int channel_list_compare(channel_list_t *list1, channel_list_t *list2);
int channel_list_compare_channel(channel_info_t *channel_info1, channel_info_t *channel_info2);
channel_info_t *channel_list_get_next_channel(channel_list_t *list, int id);
channel_info_t *channel_list_get_prev_channel(channel_list_t *list, int id);
channel_info_t *channel_list_get_curr_channel(channel_list_t *list, int id);
#ifdef __cplusplus
}
#endif

#endif /* _CHANNELLIST_H */

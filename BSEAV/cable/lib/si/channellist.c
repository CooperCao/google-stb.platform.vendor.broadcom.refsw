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
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "channellist.h"

int channel_list_init(channel_list_t *list, unsigned int  number_of_channels)
{
	if (list == NULL) return -1;

    memset(list, 0, sizeof(channel_list_t));

    list->channel = (channel_info_t *)malloc(number_of_channels * sizeof(channel_info_t));
    if (!list->channel) {
        printf("error: channel_list_init() return NULL at %d in %s\n", __LINE__, __FILE__);
        return -1;
    }
    memset(list->channel, 0, number_of_channels * sizeof(channel_info_t));
    list->total_channels = number_of_channels;
	return 0;
}

int channel_list_clear(channel_list_t *list)
{
	int i;
	if (list == NULL) return -1;

	list->active_channels = 0;
	for (i=0;i<TOTAL_CHAN;i++) list->current_channel[i]=0;
	return 0;
}

void
channel_list_destroy(channel_list_t *list)
{
    if (list) {
        if (list->channel)
            free(list->channel);
	    memset(list, 0, sizeof(channel_list_t));
    }
}


int channel_list_copy(channel_list_t *to_list, channel_list_t *from_list)
{
	int i;

	if (from_list->active_channels==0) return -1;
	if (from_list->active_channels > to_list->total_channels){
		printf(" can't copy channel list from %d to %d\n",from_list->active_channels, to_list->total_channels );
		return -1;
	}
	memcpy(to_list->channel, from_list->channel, from_list->active_channels*sizeof(channel_info_t));
    to_list->active_channels = from_list->active_channels;
	to_list->version++;
	return 0;
}


int channel_list_compare_channel(channel_info_t *chan1,channel_info_t *chan2)
{
	/* if porgram number is zero , don't compare it*/
	if (chan1->program_number && chan2->program_number
		&& chan1->program_number != chan2->program_number)
	{
		return 1;
	}

	if ( chan1->service_media != chan2->service_media
		|| chan1->frequency != chan2->frequency
		|| chan1->modulation!= chan2->modulation
		|| chan1->symbolrate!= chan2->symbolrate
		|| chan1->vc_number!= chan2->vc_number
		|| chan1->hidden != chan2->hidden
		|| chan1->annex!= chan2->annex
	   )
	{
		 return 1;
	}
	return 0;
}

int channel_list_compare_channel2(channel_info_t *chan1,channel_info_t *chan2)
{

	if ( chan1->service_media != chan2->service_media
		|| chan1->program_number != chan2->program_number
		|| chan1->frequency != chan2->frequency
		|| chan1->modulation!= chan2->modulation
		|| chan1->symbolrate!= chan2->symbolrate
		|| strcmp(chan1->program_title,  chan2->program_title)
		|| strcmp(chan1->program_long_title,  chan2->program_long_title)
		|| chan1->vc_number!= chan2->vc_number
		|| chan1->source_id != chan2->source_id
		|| chan1->hidden != chan2->hidden
		|| chan1->annex!= chan2->annex
	   )
	{
		 return 1;
	}
	return 0;
}


int channel_list_compare(channel_list_t *list1, channel_list_t *list2)
{
	int i;

	if (list1->active_channels != list2->active_channels)
	{
		return 1;
	}

	for (i=0;i<list1->active_channels;i++)
	{
		if (channel_list_compare_channel2( &list1->channel[i], &list2->channel[i]) ) return 1;
	}
	return 0;
}

/* insert channel by index number(no sorting)*/
int
channel_list_add_channel(channel_list_t *list,  channel_info_t *channel_info)
{
    if (list && list->active_channels < list->total_channels) {
        memcpy(&list->channel[list->active_channels++], channel_info, sizeof(channel_info_t));
        return 0;
    }

    return -1;
}

/*********************************************************************
 Function : channel_list_compare_channel
 Description : Function to compare a channel to another to see
 				if the virtual channel numbers are equal, greater or less.
 Input : channel_info_t *chan1.	pointer to one  channel struct
 		 channel_info_t *chan2.	pointer to second  channel struct
 Output : 0 if the chan1 number is smaller, 1 if equal, 2 if chan1 is
 			greater.
**********************************************************************/
static int channel_list_compare_channel_vc(channel_info_t *chan1,channel_info_t *chan2)
{
	int major1, major2, minor1, minor2;

	major1 = chan1->vc_number&0xffff;
	major2 = chan2->vc_number&0xffff;
	minor1 = chan1->vc_number>>16;
	minor2 = chan2->vc_number>>16;

	if (major1 > major2)
		return 2;
	else if (major1 == major2)
	{
		if (minor1 == minor2)
			return 1;
		else
			return (minor1 > minor2) ? 2: 0;
	} else
		return 0;
}

/* insert channel by Virtual channel number(sorted)*/
int
channel_list_insert_channel(channel_list_t *list,  channel_info_t *channel_info)
{
	int i=0, j;
	if (list && list->active_channels < list->total_channels) {
		while ( i<list->active_channels && channel_list_compare_channel_vc(channel_info, &list->channel[i]) == 2)
			{i++;}
		for (j=list->active_channels;j>i;j--)
		{
			memcpy(&list->channel[j], &list->channel[j-1], sizeof(channel_info_t));
		}
        memcpy(&list->channel[i], channel_info, sizeof(channel_info_t));
		list->active_channels++;
        return 0;
    }
    return -1;
}


channel_info_t *
channel_list_get_next_channel(channel_list_t *list, int id)
{
    if (list && list->active_channels) {
        if (++list->current_channel[id]>= list->active_channels) list->current_channel[id] = 0;
        return &list->channel[list->current_channel[id]];
    }

    return NULL;
}

channel_info_t *
channel_list_get_prev_channel(channel_list_t *list, int id)
{
    if (list && list->active_channels) {
        if (--list->current_channel[id] < 0)  list->current_channel[id] = list->active_channels - 1;
        return &list->channel[list->current_channel[id]];
    }
    return NULL;
}

channel_info_t *
channel_list_get_curr_channel(channel_list_t *list, int id)
{
    if (list && list->active_channels ) {
        return &list->channel[list->current_channel[id]];
    }
    return NULL;
}

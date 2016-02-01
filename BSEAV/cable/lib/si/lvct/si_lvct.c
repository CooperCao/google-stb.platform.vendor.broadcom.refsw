/***************************************************************************
 *     Copyright (c) 2002-2009, Broadcom Corporation
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
 ***************************************************************************/
#include "si.h"
#include "si_os.h"
#include "si_dbg.h"
#include "si_util.h"
#include "si_list.h"
#include "si_vct.h"
#include "si_lvct.h"
#include "si_descriptors.h"

#include "nexus_audio_types.h"
#include "nexus_video_types.h"
#include "psip_priv.h"
#include "psip_descriptor.h"


/* local function prototypes. */
static unsigned char SI_LVCT_Compare_channel(SI_LVCT_CHANNEL *chan1, SI_LVCT_CHANNEL *chan2);
static SI_RET_CODE SI_LVCT_Free_Channel(SI_LVCT_CHANNEL *channel);


struct lvct_channel_list LVCT_channels;      /*LVCT_2_channels*/
unsigned long Total_LVCT_Channels;
unsigned char LVCT_version_number;
unsigned long LVCT_section_mask[8];
SI_mutex m_lvct;

void SI_LVCT_Init(void)
{
	unsigned long i;

	SI_LST_D_INIT(&LVCT_channels);
	Total_LVCT_Channels = 0;
	LVCT_version_number = 0xff;
	for (i=0; i<8; i++)
		LVCT_section_mask[i] = 0;
	SI_mutex_init(m_lvct);
}


void SI_LVCT_Get(unsigned char *version_number, unsigned long **p_section_mask)
{
	*version_number = LVCT_version_number;
	*p_section_mask = LVCT_section_mask;
}

/*********************************************************************
 Function : SI_LVCT_Create_Channel
 Description : Function to allocate the space for an L-VCT channel.
 Input : none.
 Output : pointer to the L-VCT channel structure allocated. Will
			return NULL if out of memory.
**********************************************************************/
SI_LVCT_CHANNEL * SI_LVCT_Create_Channel (void)
{
	SI_LVCT_CHANNEL * lvct_channel;
	int	i;

	lvct_channel = (SI_LVCT_CHANNEL *)SI_alloc(sizeof(SI_LVCT_CHANNEL));
	if (lvct_channel == NULL)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("Failed to allocate an L-VCT channel!!!\n"));
		return NULL;
	}

	SI_LST_D_INIT_ENTRY(&(lvct_channel->chan_link));
	lvct_channel->num_of_ts_serv = 0;
	lvct_channel->time_shifted = NULL;

	return lvct_channel;
}

/*********************************************************************
 Function : SI_LVCT_Compare_Channel
 Description : Function to compare an LVCT channel to another to see
 				if the virtual channel numbers are equal, greater or less.
 Input : SI_LVCT_CHANNEL *chan1.	pointer to one LVCT channel struct
 		 SI_LVCT_CHANNEL *chan2.	pointer to second LVCT channel struct
 Output : 0 if the chan1 number is smaller, 1 if equal, 2 if chan1 is
 			greater.
**********************************************************************/
static unsigned char SI_LVCT_Compare_channel(SI_LVCT_CHANNEL *chan1, SI_LVCT_CHANNEL *chan2)
{
#if 0
	if (chan1->vcn_mode != chan2->vcn_mode)
	{
		if (chan1->vcn_mode == ONE_PART)
			return 0;
		else
			return 2;
	}
	/* both channels are one or two part. */
	if (chan1->vcn_mode == ONE_PART)
	{
		if (chan1->channum1 == chan2->channum1)
			return 1;
		else if (chan1->channum1 > chan2->channum1)
			return 2;
	}
	else
	{
		if (chan1->channum1 == chan2->channum1)
		{
			if (chan1->channum2 == chan2->channum2)
				return 1;
			else if (chan1->channum2 > chan2->channum2)
				return 2;

		}
		else if (chan1->channum1 > chan2->channum1)
			return 2;
	}

	return 0;
#else
	if (chan1->vcn_mode == ONE_PART) chan1->channum2 =0;
	if (chan2->vcn_mode == ONE_PART) chan2->channum2 =0;

	if (chan1->channum1 > chan2->channum1)
		return 2;
	else if (chan1->channum1 == chan2->channum1)
	{
		if (chan1->channum2 == chan2->channum2)
			return 1;
		else
			return (chan1->channum2>chan2->channum2) ? 2: 0;
	} else
		return 0;
#endif
}


/*********************************************************************
 Function : SI_LVCT_Free_Channel
 Description : Function to free an LVCT channel from the LVCT
 				channel list. the channel structure will be freed but
 				not removed from the channel list. WE ASSUME THAT WHEN
 				CALLING THIS FUNCTION, THE CHANNEL HAS NOT BEEN ADDED
 				TO THE LIST YET!!!
 Input : SI_LVCT_CHANNEL *channel.	pointer to  LVCT channel
 			structure to be freed.
 Output : SI_RET_CODE.
**********************************************************************/
static SI_RET_CODE SI_LVCT_Free_Channel(SI_LVCT_CHANNEL *channel)
{
	if (channel)
	{
		if (channel->time_shifted)
			SI_free(channel->time_shifted);

		SI_free(channel);
	}

	return SI_SUCCESS;
}


/*********************************************************************
 Function : SI_LVCT_Free_List
 Description : Function to free the whole LVCT channel list.
 Input : None.
 Output : SI_RET_CODE.
**********************************************************************/
SI_RET_CODE SI_LVCT_Free_List(void)
{
	SI_LVCT_CHANNEL *channel;

	SI_mutex_lock(m_lvct);
	while ((channel = SI_LST_D_FIRST(&LVCT_channels)))
	{
		SI_LST_D_REMOVE_HEAD(&LVCT_channels, chan_link);
		SI_LVCT_Free_Channel(channel);
		Total_LVCT_Channels--;
	}

	Total_LVCT_Channels = 0;  /* just to be sure. */
	SI_mutex_unlock(m_lvct);

	return SI_SUCCESS;
}


/*********************************************************************
 Function : SI_LVCT_Ins_Channel
 Description : Function to insert an LVCT channel into the LVCT
 				channel list. The order is that two part numbers
 				are after the one part numbers and within each part
 				the channels are sorted in incrementing order.
 Input : SI_LVCT_CHANNEL *new_channel.	pointer to new LVCT channel
 			structure to be inserted.
 Output : SI_RET_CODE.
**********************************************************************/
SI_RET_CODE SI_LVCT_Ins_Channel(SI_LVCT_CHANNEL *new_channel)
{
	SI_LVCT_CHANNEL * channel;
	unsigned char comp;
	int	i;

	SI_mutex_lock(m_lvct);

	channel = SI_LST_D_FIRST(&LVCT_channels);
	/* if the list is empty, just put the new channel in. */
	if (channel == NULL)
	{
		SI_LST_D_INSERT_HEAD(&LVCT_channels, new_channel, chan_link);
		Total_LVCT_Channels++;
		SI_mutex_unlock(m_lvct);
		return SI_SUCCESS;
	}

	/* search for the the place to insert. */
	while ((comp = SI_LVCT_Compare_channel(new_channel, channel)) == 2 && SI_LST_D_NEXT(channel, chan_link))
		channel = SI_LST_D_NEXT(channel, chan_link);

	if (comp == 2)
	{
		/* we got to the end of list. insert after current element. */
		SI_LST_D_INSERT_AFTER(channel, new_channel, chan_link);
		Total_LVCT_Channels++;
	}
#if 0
	else if (comp == 0)
	{
		/* insert before the current element. */
		SI_LST_D_INSERT_BEFORE(&LVCT_channels, channel, new_channel, chan_link);
		Total_LVCT_Channels++;
	}
	else
	{
		/* equal! It should not happen. But if it does, simply keep it and free the new one. */
		SI_DBG_PRINT(E_SI_WRN_MSG,("LVCT the channel already exists! keeping it and free newbie!\n"));
		SI_LVCT_Free_Channel(new_channel);
	}
#else  /* we do see lots of channel 0 */
	else
	{
		/* insert before the current element. */
		SI_LST_D_INSERT_BEFORE(&LVCT_channels, channel, new_channel, chan_link);
		Total_LVCT_Channels++;
	}
#endif

	SI_mutex_unlock(m_lvct);

	return SI_SUCCESS;
}


/*********************************************************************
 Function : SI_LVCT_Parse
 Description : Function to parse a newly received LVCT table section and
 				put it into the LVCT channel link list
 Input : unsigned char *lvct_table : newly received LVCT table data.
 Output : SI_RET_CODE.
**********************************************************************/
SI_RET_CODE SI_LVCT_Parse (unsigned char *lvct_table,  VCT_MODE mode)
{
	unsigned long temp, i, j, offset;
	unsigned long section_length, version_number;
	unsigned long section_number, last_section_number, num_channels_in_section;
	unsigned long desc_start;
	unsigned long desc_tag, desc_len, len;
	unsigned short major_num, minor_num;
	unsigned char *current;
	SI_LVCT_CHANNEL * channel;
	SI_RET_CODE result;

	SI_DBG_PRINT(E_SI_DBG_MSG,("LVCT Table received.\n"));

	temp = *lvct_table;

	if (( (mode ==VCT_MODE_LVCT || mode ==VCT_MODE_CVCT) && temp != SI_LVCT_TABLE_ID)
		|| (mode ==VCT_MODE_TVCT && temp != SI_TVCT_TABLE_ID))
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("LVCT Table ID error!!! %x\n", temp));
		return SI_TABLE_ID_ERROR;
	}

	/* calculate and check section length. */
	section_length = SI_Construct_Data(lvct_table,
				LVCT_SECTION_LENGTH_BYTE_INDX,
				LVCT_SECTION_LENGTH_BYTE_NUM,
				LVCT_SECTION_LENGTH_SHIFT,
				LVCT_SECTION_LENGTH_MASK);
	section_length += LVCT_SECTION_LENGTH_BYTE_INDX+LVCT_SECTION_LENGTH_BYTE_NUM;
	if (section_length > SI_LONG_SECTION_LENGTH)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("LVCT Table section length error!!! %x\n", section_length));
		return SI_SECTION_LENGTH_ERROR;
	}

	/* We do the CRC check here to verify the contents of this section. */
	if (SI_CRC32_Check(lvct_table, section_length) != SI_SUCCESS)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("LVCT Table section CRC error!!!\n"));
		return SI_CRC_ERROR;
	}

	/* look at current_next_indicator. It should be 1 for AEIT. */
	temp = SI_Construct_Data(lvct_table,
				LVCT_CURRENT_NEXT_INDICATOR_BYTE_INDX,
				LVCT_CURRENT_NEXT_INDICATOR_BYTE_NUM,
				LVCT_CURRENT_NEXT_INDICATOR_SHIFT,
				LVCT_CURRENT_NEXT_INDICATOR_MASK);
	if (temp != 1)
	{
		SI_DBG_PRINT(E_SI_DBG_MSG,("LVCT Table current_next_indicator not one. discarding it.%x\n", temp));
		return SI_SUCCESS;
	}

	/* check to make sure protocol version is zero. */
	temp = SI_Construct_Data(lvct_table,
				LVCT_PROTOCOL_VERSION_BYTE_INDX,
				LVCT_PROTOCOL_VERSION_BYTE_NUM,
				LVCT_PROTOCOL_VERSION_SHIFT,
				LVCT_PROTOCOL_VERSION_MASK);
	if (temp != SI_CURRENT_PROTOCOL_VERSION)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("LVCT Table PROTOCOL version error!!! %x\n", temp));
		return SI_PROTOCOL_VER_ERROR;
	}

	/* now we know where the slot is in the link list, See if we need to update. */
	version_number = SI_Construct_Data(lvct_table,
						LVCT_VERSION_NUMBER_BYTE_INDX,
						LVCT_VERSION_NUMBER_BYTE_NUM,
						LVCT_VERSION_NUMBER_SHIFT,
						LVCT_VERSION_NUMBER_MASK);
	section_number = SI_Construct_Data(lvct_table,
						LVCT_SECTION_NUMBER_BYTE_INDX,
						LVCT_SECTION_NUMBER_BYTE_NUM,
						LVCT_SECTION_NUMBER_SHIFT,
						LVCT_SECTION_NUMBER_MASK);


	if (LVCT_version_number == version_number)
	{
		/* the same version number. Now check if the section number has already be processed. */
		if (SI_Chk_Section_mask(LVCT_section_mask, section_number))
		{
			/* section already processed, we are done! */
			SI_DBG_PRINT(E_SI_DBG_MSG,("LVCT Table section does not need to be updated!\n"));
			return SI_SUCCESS;
		}
		else
			SI_DBG_PRINT(E_SI_DBG_MSG,("New LVCT Table section received! %d\n", section_number));
	}
	else
	{
		/* different version number. free the old channel link list. */
		SI_DBG_PRINT(E_SI_DBG_MSG,("New LVCT Table version received!\n"));
		/* init section mask. */
		last_section_number = SI_Construct_Data(lvct_table,
							LVCT_LAST_SECTION_NUMBER_BYTE_INDX,
							LVCT_LAST_SECTION_NUMBER_BYTE_NUM,
							LVCT_LAST_SECTION_NUMBER_SHIFT,
							LVCT_LAST_SECTION_NUMBER_MASK);
		SI_Init_Section_Mask(LVCT_section_mask, last_section_number);
		LVCT_version_number = version_number;
		printf(" section_number %d last_section_number %d\n", section_number, last_section_number);
		/* free the old list. */
		SI_LVCT_Free_List();
	}

	/* update section mask here. */
	SI_Set_Section_mask(LVCT_section_mask, section_number);

	num_channels_in_section = SI_Construct_Data(lvct_table,
							LVCT_NUM_CHANNELS_BYTE_INDX,
							LVCT_NUM_CHANNELS_BYTE_NUM,
							LVCT_NUM_CHANNELS_SHIFT,
							LVCT_NUM_CHANNELS_MASK);

	/* get channels one by one. */
	current = lvct_table+LVCT_NUM_CHANNELS_BYTE_INDX+LVCT_NUM_CHANNELS_BYTE_NUM;
	for (i=0; i<num_channels_in_section; i++)
	{
		/* create the channel link. */
		if ((channel = SI_LVCT_Create_Channel()) == NULL)
			return SI_NO_MEMORY;

		/* reset long name*/
		channel->long_name[0] = 0;

		/* get the short name. */
		for (j=0; j<LVCT_SHORT_NAME_LENGTH; j++)
		{
			channel->short_name[j] = ((((unsigned short)(*(current++)))<<8)&0xff00);
			channel->short_name[j] |= (((unsigned short)(*(current++)))&0x00ff);
		}

		/* get channel number(s). */
		major_num = SI_Construct_Data(current,
					LVCT_MAJOR_NUMBER_BYTE_INDX,
					LVCT_MAJOR_NUMBER_BYTE_NUM,
					LVCT_MAJOR_NUMBER_SHIFT,
					LVCT_MAJOR_NUMBER_MASK);
		minor_num = SI_Construct_Data(current,
					LVCT_MINOR_NUMBER_BYTE_INDX,
					LVCT_MINOR_NUMBER_BYTE_NUM,
					LVCT_MINOR_NUMBER_SHIFT,
					LVCT_MINOR_NUMBER_MASK);

		SI_DBG_PRINT(E_SI_DBG_MSG,("++++++Channel %x-%x: ", major_num, minor_num));
		if ( (mode == VCT_MODE_CVCT || mode == VCT_MODE_LVCT) &&((major_num&0x03f0) == 0x03f0))
		{
			channel->vcn_mode = ONE_PART;
			channel->channum1 = ((major_num&0x0f)<<10) + minor_num;
			SI_DBG_PRINT(E_SI_DBG_MSG,("Channel %x: ", channel->channum1));
		}
		else
		{
			channel->vcn_mode = TWO_PART;
			channel->channum1 = major_num;
			channel->channum2 = minor_num;
			SI_DBG_PRINT(E_SI_DBG_MSG,("Channel %x-%x: ", channel->channum1, channel->channum2));
		}

		for (j=0; j<LVCT_SHORT_NAME_LENGTH; j++)
			SI_DBG_PRINT(E_SI_DBG_MSG,("%c", (channel->short_name[j]&0xff)));

		/* get other parameters. */
		channel->mod_mode = SI_Construct_Data(current,
							LVCT_MODULATION_MODE_BYTE_INDX,
							LVCT_MODULATION_MODE_BYTE_NUM,
							LVCT_MODULATION_MODE_SHIFT,
							LVCT_MODULATION_MODE_MASK);
		channel->carrier_freq = SI_Construct_Data(current,
							LVCT_CARRIER_FREQUENCY_BYTE_INDX,
							LVCT_CARRIER_FREQUENCY_BYTE_NUM,
							LVCT_CARRIER_FREQUENCY_SHIFT,
							LVCT_CARRIER_FREQUENCY_MASK);
		channel->tsid = SI_Construct_Data(current,
							LVCT_CHANNEL_TSID_BYTE_INDX,
							LVCT_CHANNEL_TSID_BYTE_NUM,
							LVCT_CHANNEL_TSID_SHIFT,
							LVCT_CHANNEL_TSID_MASK);
		channel->program_num = SI_Construct_Data(current,
							LVCT_PROGRAM_NUMBER_BYTE_INDX,
							LVCT_PROGRAM_NUMBER_BYTE_NUM,
							LVCT_PROGRAM_NUMBER_SHIFT,
							LVCT_PROGRAM_NUMBER_MASK);
		channel->chanbits = SI_Construct_Data(current,
							LVCT_CHANNEL_BITS_BYTE_INDX,
							LVCT_CHANNEL_BITS_BYTE_NUM,
							LVCT_CHANNEL_BITS_SHIFT,
							LVCT_CHANNEL_BITS_MASK);
		channel->serv_type = SI_Construct_Data(current,
							LVCT_SERVICE_TYPE_BYTE_INDX,
							LVCT_SERVICE_TYPE_BYTE_NUM,
							LVCT_SERVICE_TYPE_SHIFT,
							LVCT_SERVICE_TYPE_MASK);
		channel->source_ID = SI_Construct_Data(current,
							LVCT_SOURCE_ID_BYTE_INDX,
							LVCT_SOURCE_ID_BYTE_NUM,
							LVCT_SOURCE_ID_SHIFT,
							LVCT_SOURCE_ID_MASK);

		memset(&channel->program_info, 0, sizeof(PROGRAM_INFO_T));
		SI_DBG_PRINT(E_SI_DBG_MSG,("mod %x, carrire %x hz, tsid %x, prog %x, serv type %x, src id %x ", channel->mod_mode,channel->carrier_freq,channel->tsid,channel->program_num,channel->serv_type,channel->source_ID));
		SI_DBG_PRINT(E_SI_DBG_MSG,("\n"));

		/* get descriptor length. */
		desc_len = SI_Construct_Data(current,
					LVCT_DESC_LENGTH_BYTE_INDX,
					LVCT_DESC_LENGTH_BYTE_NUM,
					LVCT_DESC_LENGTH_SHIFT,
					LVCT_DESC_LENGTH_MASK);

		SI_DBG_PRINT(E_SI_DBG_MSG,(" LEN %d", desc_len));
		SI_DBG_PRINT(E_SI_DBG_MSG,("\n"));


		/* go through all descriptors. */
		current += LVCT_DESC_LENGTH_BYTE_INDX+LVCT_DESC_LENGTH_BYTE_NUM;
		offset = 0;
		while (offset < desc_len)
		{
			desc_tag = *(current++); /* points to desc len. */
			len = *(current++); /* point to first desc data. */
			switch(desc_tag)
			{
				case SI_DESC_EXTENDED_CHANNEL_NAME:
					SI_DBG_PRINT(E_SI_DBG_MSG,("LVCT Table: ext channel name descriptor received.\n"));
					{
						uint8_t  num_str;
						int lsize;
						PSIP_MSS_string lstr = PSIP_ECND_getLongChannelName(current-2);
						/* TODO::current only process first one*/
						num_str = PSIP_MSS_getNumStrings(lstr);
						if (num_str > 0)
						{
							lsize = LVCT_LONG_NAME_LENGTH;
							if (PSIP_MSS_getString(lstr,0, &lsize, &channel->long_name) != BERR_SUCCESS)
							{
								SI_DBG_PRINT(E_SI_ERR_MSG,("LVCT Table failed to get channel ext name!!!\n"));
								SI_LVCT_Free_Channel(channel);
								return SI_NO_MEMORY;
							}
						}
					}
					current += len;  /* point to next desc. */
					offset += len+2;
				break;
				case SI_DESC_SERVICE_LOCATION:
					{
						PSIP_SLD_header sld_header;
						PSIP_SLD_element sld_elem;
						int sld_idx,lidx,num_aud, num_vid;

						PSIP_SLD_getHeader(current-2, &sld_header);

						channel->program_info.pcr_pid = sld_header.PCR_PID;
						SI_DBG_PRINT(E_SI_WRN_MSG,("LVCT channel SLD descriptor received!\n"));
						SI_DBG_PRINT(E_SI_DBG_MSG,("SLD number_elements = %d\n",sld_header.number_elements ));

						num_aud = 0;
						num_vid = 0;
						for (sld_idx = 0; sld_idx < sld_header.number_elements; ++sld_idx)
						{
							if (PSIP_SLD_getElement(current-2,sld_idx,&sld_elem) != BERR_SUCCESS)
							{
								SI_DBG_PRINT(E_SI_WRN_MSG,("Error processing SLD Desc %d of %d",sld_idx,sld_header.number_elements));
								break;
							}
							if (sld_elem.stream_type == TS_PSI_ST_13818_2_Video || sld_elem.stream_type ==TS_PSI_ST_ATSC_Video)
							{
								channel->program_info.video_pids[num_vid].streamType = NEXUS_VideoCodec_eMpeg2;
								channel->program_info.video_pids[num_vid++].pid = sld_elem.elementary_PID;
							} else if (sld_elem.stream_type == TS_PSI_ST_14496_10_Video)
							{
								channel->program_info.video_pids[num_vid].streamType = NEXUS_VideoCodec_eH264;
								channel->program_info.video_pids[num_vid++].pid = sld_elem.elementary_PID;
							} else if (sld_elem.stream_type == TS_PSI_ST_SMPTE_VC1)
							{
								channel->program_info.video_pids[num_vid].streamType = NEXUS_VideoCodec_eVc1;
								channel->program_info.video_pids[num_vid++].pid = sld_elem.elementary_PID;
							}else if (sld_elem.stream_type == TS_PSI_ST_AVS_Video)
							{
								channel->program_info.video_pids[num_vid].streamType = NEXUS_VideoCodec_eAvs;
								channel->program_info.video_pids[num_vid++].pid = sld_elem.elementary_PID;
							} else if (sld_elem.stream_type ==TS_PSI_ST_11172_3_Audio||sld_elem.stream_type == TS_PSI_ST_13818_3_Audio)
							{
								channel->program_info.audio_pids[num_aud].streamType = NEXUS_AudioCodec_eMpeg;
								channel->program_info.audio_pids[num_aud].pid = sld_elem.elementary_PID;
								memcpy(channel->program_info.audio_pids[num_aud++].iso639, sld_elem.ISO_639_language_code, ISO936_CODE_LENGTH);
							} else if (sld_elem.stream_type == TS_PSI_ST_ATSC_AC3)
							{
								channel->program_info.audio_pids[num_aud].streamType = NEXUS_AudioCodec_eAc3;
								channel->program_info.audio_pids[num_aud].pid = sld_elem.elementary_PID;
								memcpy(channel->program_info.audio_pids[num_aud++].iso639, sld_elem.ISO_639_language_code, ISO936_CODE_LENGTH);
							}else if (sld_elem.stream_type ==TS_PSI_ST_ATSC_EAC3)
							{
								channel->program_info.audio_pids[num_aud].streamType = NEXUS_AudioCodec_eAc3Plus;
								channel->program_info.audio_pids[num_aud].pid = sld_elem.elementary_PID;
								memcpy(channel->program_info.audio_pids[num_aud++].iso639, sld_elem.ISO_639_language_code, ISO936_CODE_LENGTH);
							}else if (sld_elem.stream_type == TS_PSI_ST_AVS_Audio)
							{
								channel->program_info.audio_pids[num_aud].streamType = NEXUS_AudioCodec_eAvs;
								channel->program_info.audio_pids[num_aud].pid = sld_elem.elementary_PID;
								memcpy(channel->program_info.audio_pids[num_aud++].iso639, sld_elem.ISO_639_language_code, ISO936_CODE_LENGTH);
							} else if ( sld_elem.stream_type == TS_PSI_ST_13818_7_AAC )
							{
								channel->program_info.audio_pids[num_aud].streamType = NEXUS_AudioCodec_eAac;
								channel->program_info.audio_pids[num_aud].pid = sld_elem.elementary_PID;
								memcpy(channel->program_info.audio_pids[num_aud++].iso639, sld_elem.ISO_639_language_code, ISO936_CODE_LENGTH);
							} else if (	sld_elem.stream_type == TS_PSI_ST_14496_3_Audio )
							{
								channel->program_info.audio_pids[num_aud].streamType = NEXUS_AudioCodec_eAacPlus;
								channel->program_info.audio_pids[num_aud].pid = sld_elem.elementary_PID;
								memcpy(channel->program_info.audio_pids[num_aud++].iso639, sld_elem.ISO_639_language_code, ISO936_CODE_LENGTH);
							}
						}
						channel->program_info.num_audio_pids = num_aud;
						channel->program_info.num_video_pids = num_vid;
						SI_DBG_PRINT(E_SI_DBG_MSG,("SLD audio %d  video %d\n", num_aud, num_vid));
					}
					current += len;  /* point to next desc. */
					offset += len+2;
					break;
				case SI_DESC_TIME_SHIFTED_SERVICE:
					SI_DBG_PRINT(E_SI_DBG_MSG,("LVCT Table: Time shifted descriptor received.\n"));
					channel->num_of_ts_serv = SI_Construct_Data(current,
											DESC_TSS_NUM_OF_SERV_BYTE_INDEX,
											DESC_TSS_NUM_OF_SERV_BYTE_NUM,
											DESC_TSS_NUM_OF_SERV_SHIFT,
											DESC_TSS_NUM_OF_SERV_MASK);
					current += DESC_TSS_NUM_OF_SERV_BYTE_INDEX+DESC_TSS_NUM_OF_SERV_BYTE_NUM;
					offset += DESC_TSS_NUM_OF_SERV_BYTE_INDEX+DESC_TSS_NUM_OF_SERV_BYTE_NUM;
					if ( (channel->time_shifted = SI_alloc(channel->num_of_ts_serv*sizeof(TIME_SHIFT_SERV))) == NULL)
					{
						SI_DBG_PRINT(E_SI_ERR_MSG,("LVCT Table failed to alloc mem for time shift serv!!!\n"));
						SI_LVCT_Free_Channel(channel);
						return SI_NO_MEMORY;
					}
					for (j=0; j<channel->num_of_ts_serv; j++)
					{
						channel->time_shifted[j].time_shift = SI_Construct_Data(current,
															DESC_TSS_TIME_SHIFT_BYTE_INDEX,
															DESC_TSS_TIME_SHIFT_BYTE_NUM,
															DESC_TSS_TIME_SHIFT_SHIFT,
															DESC_TSS_TIME_SHIFT_MASK);
						major_num = SI_Construct_Data(current,
									DESC_TSS_MAJOR_NUM_BYTE_INDEX,
									DESC_TSS_MAJOR_NUM_BYTE_NUM,
									DESC_TSS_MAJOR_NUM_SHIFT,
									DESC_TSS_MAJOR_NUM_MASK);
						minor_num = SI_Construct_Data(current,
									DESC_TSS_MINOR_NUM_BYTE_INDEX,
									DESC_TSS_MINOR_NUM_BYTE_NUM,
									DESC_TSS_MINOR_NUM_SHIFT,
									DESC_TSS_MINOR_NUM_MASK);
						if ((mode == VCT_MODE_CVCT || mode == VCT_MODE_LVCT) &&((major_num&0x03f0) == 0x03f0))
						{
							channel->time_shifted[j].vcn_mode = ONE_PART;
							channel->time_shifted[j].channum1 = ((major_num&0x0f)<<10) + minor_num;
						}
						else
						{
							channel->time_shifted[j].vcn_mode = TWO_PART;
							channel->time_shifted[j].channum1 = major_num;
							channel->time_shifted[j].channum2 = minor_num;
						}
						current += DESC_TSS_MINOR_NUM_BYTE_INDEX+DESC_TSS_MINOR_NUM_BYTE_NUM;
						offset += DESC_TSS_MINOR_NUM_BYTE_INDEX+DESC_TSS_MINOR_NUM_BYTE_NUM;
					}
				break;
				default:
					if(desc_tag != SI_DESC_STUFFING)
						SI_DBG_PRINT(E_SI_WRN_MSG,("LVCT channel descriptor %x received!\n", desc_tag));
					current += len;  /* point to next desc. */
					offset += len+2;
				break;
			}
		}

		/* make sure descriptor len works out. */
		if (offset != desc_len)
		{
			SI_DBG_PRINT(E_SI_ERR_MSG,("LVCT Table channel descriptor error!!!\n"));
			SI_LVCT_Free_Channel(channel);
			return SI_DESCRIPTOR_ERROR;
		}

		/* insert the channel in the list. */
		SI_LVCT_Ins_Channel(channel);
	}

	/* TBD just skip the table descriptors and check for length. */
	desc_len = SI_Construct_Data(current,
				LVCT_ADD_DESC_LENGTH_BYTE_INDX,
				LVCT_ADD_DESC_LENGTH_BYTE_NUM,
				LVCT_ADD_DESC_LENGTH_SHIFT,
				LVCT_ADD_DESC_LENGTH_MASK);
	current += LVCT_ADD_DESC_LENGTH_BYTE_INDX+LVCT_ADD_DESC_LENGTH_BYTE_NUM;
	offset = 0;
	while (offset < desc_len)
	{
		if ((desc_tag = *(current++)) != SI_DESC_STUFFING)
			SI_DBG_PRINT(E_SI_WRN_MSG,("LVCT table descriptor %x received! Ignoring!\n", desc_tag));

		len = *(current++);
		current += len;
		offset += len+2;
	}

	if (offset != desc_len)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("LVCT Table section descriptor length error!!!\n"));
		return SI_DESCRIPTOR_ERROR;
	}

	if ((unsigned long)(current-lvct_table) != section_length-SI_CRC_LENGTH)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("LVCT Table length error!!!\n"));
		return SI_SECTION_LENGTH_ERROR;
	}

	return SI_SUCCESS;
}



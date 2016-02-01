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
#include "si_svct_vcm.h"
#include "si_stt.h"
#include "si_descriptors.h"

/* local function prototypes. */
static SI_SVCT_VCM_CHANNEL * SI_SVCT_VCM_Create_Channel (void);
static unsigned char SI_SVCT_VCM_Compare_channel(SI_SVCT_VCM_CHANNEL *chan1, SI_SVCT_VCM_CHANNEL *chan2);
static SI_RET_CODE SI_SVCT_VCM_Free_Channel(SI_SVCT_VCM_CHANNEL *channel);
static SI_RET_CODE SI_SVCT_VCM_Ins_Channel(SI_SVCT_VCM_CHANNEL *new_channel);


struct svct_vcm_channel_list SVCT_VCM_channels, SVCT_VCM_2_channels;
unsigned long Total_SVCT_VCM_Channels;
SI_mutex m_svct_vcm;

void SI_SVCT_VCM_Init(void)
{
	SI_LST_D_INIT(&SVCT_VCM_channels);
	Total_SVCT_VCM_Channels = 0;
	SI_mutex_init(m_svct_vcm);
}


/*********************************************************************
 Function : SI_SVCT_VCM_Create_Channel
 Description : Function to allocate the space for an S-VCT channel.
 Input : none.
 Output : pointer to the S-VCT channel structure allocated. Will
			return NULL if out of memory.
**********************************************************************/
static SI_SVCT_VCM_CHANNEL * SI_SVCT_VCM_Create_Channel (void)
{
	SI_SVCT_VCM_CHANNEL * channel;
	int	i;

	channel = (SI_SVCT_VCM_CHANNEL *)SI_alloc(sizeof(SI_SVCT_VCM_CHANNEL));
	if (channel == NULL)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("Failed to allocate an S-VCT channel!!!\n"));
		return NULL;
	}

   SI_memset(channel, 0, sizeof (SI_SVCT_VCM_CHANNEL));

	SI_LST_D_INIT_ENTRY(&(channel->chan_link));
	channel->more_prop = 0;

	return channel;
}

/*********************************************************************
 Function : SI_SVCT_VCM_Compare_Channel
 Description : Function to compare an SVCT VCM channel to another to see
 				if the virtual channel numbers are equal, greater or less.
 Input : SI_SVCT_VCM_CHANNEL *chan1.	pointer to one SVCT VCM channel struct
 		 SI_SVCT_VCM_CHANNEL *chan2.	pointer to second SVCT VCM channel struct
 Output : 0 if the chan1 number is smaller, 1 if equal, 2 if chan1 is
 			greater.
**********************************************************************/
static unsigned char SI_SVCT_VCM_Compare_channel(SI_SVCT_VCM_CHANNEL *chan1, SI_SVCT_VCM_CHANNEL *chan2)
{
#if 0
	if (chan1->vcn_mode != chan2->vcn_mode)
		if (chan1->vcn_mode == ONE_PART)
			return 0;
		else
			return 2;
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
 Function : SI_SVCT_VCM_Free_Channel
 Description : Function to free an SVCT VCM channel from the SVCT VCM
 				channel list. the channel structure will be freed but
 				not removed from the channel list. WE ASSUME THAT WHEN
 				CALLING THIS FUNCTION, THE CHANNEL HAS NOT BEEN ADDED
 				TO THE LIST YET!!!
 Input : SI_SVCT_VCM_CHANNEL *channel.	pointer to  SVCT VCM channel
 			structure to be freed.
 Output : SI_RET_CODE.
**********************************************************************/
static SI_RET_CODE SI_SVCT_VCM_Free_Channel(SI_SVCT_VCM_CHANNEL *channel)
{
	if (channel)
		SI_free(channel);

	return SI_SUCCESS;
}


/*********************************************************************
 Function : SI_SVCT_VCM_Free_List
 Description : Function to free the whole SVCT VCM channel list.
 Input : None.
 Output : SI_RET_CODE.
**********************************************************************/
SI_RET_CODE SI_SVCT_VCM_Free_List(void)
{
	SI_SVCT_VCM_CHANNEL *channel;

	SI_mutex_lock(m_svct_vcm);
	while ((channel = SI_LST_D_FIRST(&SVCT_VCM_channels)))
	{
		SI_LST_D_REMOVE_HEAD(&SVCT_VCM_channels, chan_link);
		SI_SVCT_VCM_Free_Channel(channel);
		Total_SVCT_VCM_Channels--;
	}

	/* just in case. */
	Total_SVCT_VCM_Channels = 0;

	SI_mutex_unlock(m_svct_vcm);

	return SI_SUCCESS;
}


/*********************************************************************
 Function : SI_SVCT_VCM_Ins_Channel
 Description : Function to insert an SVCT VCM channel into the SVCT VCM
 				channel list. The order is that two part numbers
 				are after the one part numbers and within each part
 				the channels are sorted in incrementing order.
 Input : SI_SVCT_VCM_CHANNEL *new_channel.	pointer to new SVCT VCM channel
 			structure to be inserted.
 Output : SI_RET_CODE.
**********************************************************************/
static SI_RET_CODE SI_SVCT_VCM_Ins_Channel(SI_SVCT_VCM_CHANNEL *new_channel)
{
	SI_SVCT_VCM_CHANNEL * channel;
	unsigned char comp;
	int	i;

	SI_mutex_lock(m_svct_vcm);

	channel = SI_LST_D_FIRST(&SVCT_VCM_channels);
	/* if the list is empty, just put the new channel in. */
	if (channel == NULL)
	{
		SI_LST_D_INSERT_HEAD(&SVCT_VCM_channels, new_channel, chan_link);
		Total_SVCT_VCM_Channels++;
		SI_mutex_unlock(m_svct_vcm);
		return SI_SUCCESS;
	}

	/* search for the the place to insert. */
	while ((comp = SI_SVCT_VCM_Compare_channel(new_channel, channel)) == 2 && SI_LST_D_NEXT(channel, chan_link))
		channel = SI_LST_D_NEXT(channel, chan_link);

	if (comp == 2)
	{
		/* we got to the end of list. insert after current element. */
		SI_LST_D_INSERT_AFTER(channel, new_channel, chan_link);
		Total_SVCT_VCM_Channels++;
	}
	else if (comp == 0)
	{
		/* insert before the current element. */
		SI_LST_D_INSERT_BEFORE(&SVCT_VCM_channels, channel, new_channel, chan_link);
		Total_SVCT_VCM_Channels++;
	}
	else
	{
		/* equal! It should only happen when we do not have the revision descriptor,
			simply keep the old one and free the new one. */
		SI_SVCT_VCM_Free_Channel(new_channel);
	}

	SI_mutex_unlock(m_svct_vcm);

	return SI_SUCCESS;
}


/*********************************************************************
 Function : SI_SVCT_VCM_Pointer
 Description : Function to point past newly received SVCT VCM table
 				section inorder to get to table descriptors.
 Input : unsigned char *table : newly received SVCT VCM table data.
 Output : unsigned char * points to the end of VCM record plus 1 which is the
 					start of table descriptors.
**********************************************************************/
unsigned char * SI_SVCT_VCM_Pointer (unsigned char *table)
{
	unsigned char desc_incl;
	unsigned char num_vc_rec;
	unsigned long desc_start, desc_cnt;
	unsigned long desc_tag, desc_len, len, i, j;
	unsigned char *current;

	/* get descriptors_included bit. */
	desc_incl = SI_Construct_Data(table,
				SVCT_VCM_DESC_INCL_BYTE_INDX,
				SVCT_VCM_DESC_INCL_BYTE_NUM,
				SVCT_VCM_DESC_INCL_SHIFT,
				SVCT_VCM_DESC_INCL_MASK);

	/* get number of VC records. */
	num_vc_rec = SI_Construct_Data(table,
				SVCT_VCM_NUM_VC_REC_BYTE_INDX,
				SVCT_VCM_NUM_VC_REC_BYTE_NUM,
				SVCT_VCM_NUM_VC_REC_SHIFT,
				SVCT_VCM_NUM_VC_REC_MASK);

	current = table + SVCT_VCM_NUM_VC_REC_BYTE_INDX + SVCT_VCM_NUM_VC_REC_BYTE_NUM;

	/* go through all the VCM records. */
	for (i=0; i<num_vc_rec; i++)
	{
		if (desc_incl)
		{
			/* get descriptors count. */
			desc_cnt = SI_Construct_Data(current,
						SVCT_VCM_DESC_CNT_BYTE_INDX,
						SVCT_VCM_DESC_CNT_BYTE_NUM,
						SVCT_VCM_DESC_CNT_SHIFT,
						SVCT_VCM_DESC_CNT_MASK);
			/* current -> first descriptor. */
			current += SVCT_VCM_DESC_CNT_BYTE_INDX+SVCT_VCM_DESC_CNT_BYTE_NUM;

			/* go through all descriptors. */
			for (j=0; j<desc_cnt; j++)
			{
				desc_tag = *(current++);
				len = *(current++);
				/* update current pointer. */
				current += len;
			}
		}
		else
			/* simply update the current pointer. */
			current += SVCT_VCM_MMS_REF_BYTE_INDX+SVCT_VCM_MMS_REF_BYTE_NUM;
	}

	return current;
}


/*********************************************************************
 Function : SI_SVCT_VCM_Parse
 Description : Function to parse a newly received SVCT VCM table section and
 				put it into the SVCT VCM channel link list
 Input : unsigned char *table : newly received SVCT VCM table data.
 Output : SI_RET_CODE.
**********************************************************************/
SI_RET_CODE SI_SVCT_VCM_Parse (unsigned char *table)
{
	unsigned long temp, i, j, offset;
	unsigned char desc_incl;
	unsigned char num_vc_rec;
	unsigned long desc_start, desc_cnt;
	unsigned long desc_tag, desc_len, len;
	unsigned char *current;
	SI_SVCT_VCM_CHANNEL * channel;
	SI_RET_CODE result;


	SI_DBG_PRINT(E_SI_DBG_MSG,("SVCT VCM Table received.\n"));

	/* get descriptors_included bit. */
	desc_incl = SI_Construct_Data(table,
				SVCT_VCM_DESC_INCL_BYTE_INDX,
				SVCT_VCM_DESC_INCL_BYTE_NUM,
				SVCT_VCM_DESC_INCL_SHIFT,
				SVCT_VCM_DESC_INCL_MASK);

	/* check the activation time. */
	temp = SI_Construct_Data(table,
				SVCT_VCM_ACTIVE_TIME_BYTE_INDX,
				SVCT_VCM_ACTIVE_TIME_BYTE_NUM,
				SVCT_VCM_ACTIVE_TIME_SHIFT,
				SVCT_VCM_ACTIVE_TIME_MASK);

	/* zero is considered as immediate action*/
	if (temp != 0 && temp > SI_STT_Get_GPS_UTC_Offset()) {
		temp = SI_STT_Conv_To_UTC_Time(temp);
		if ( temp!=0 && temp > SI_STT_Get_Sys_Time())
		{
			SI_DBG_PRINT(E_SI_DBG_MSG,("SVCT VCM table activation time in the future! Discarding now\n"));
			return SI_SUCCESS;
		}
	}

	/* get number of VC records. */
	num_vc_rec = SI_Construct_Data(table,
				SVCT_VCM_NUM_VC_REC_BYTE_INDX,
				SVCT_VCM_NUM_VC_REC_BYTE_NUM,
				SVCT_VCM_NUM_VC_REC_SHIFT,
				SVCT_VCM_NUM_VC_REC_MASK);
	SI_DBG_PRINT(E_SI_DBG_MSG,("SVCT VCM num of VCM's %x.\n", num_vc_rec));

	current = table + SVCT_VCM_NUM_VC_REC_BYTE_INDX + SVCT_VCM_NUM_VC_REC_BYTE_NUM;
	/* go through all the VCM records. */
	for (i=0; i<num_vc_rec; i++)
	{
		/* create the channel struct. */
		if ((channel = SI_SVCT_VCM_Create_Channel()) == NULL)
		{
			SI_DBG_PRINT(E_SI_ERR_MSG,("SVCT VCM cannot create channel structure!!!\n"));
			return SI_NO_MEMORY;
		}

		channel->appflag = SI_Construct_Data(current,
							SVCT_VCM_APP_VIRT_CHAN_BYTE_INDX,
							SVCT_VCM_APP_VIRT_CHAN_BYTE_NUM,
							SVCT_VCM_APP_VIRT_CHAN_SHIFT,
							SVCT_VCM_APP_VIRT_CHAN_MASK);
		if (channel->appflag)
			SI_DBG_PRINT(E_SI_DBG_MSG,("SVCT VCM received app entry point!!!\n"));

		/* assuming it onepart channel number until we got two part descriptors. */
		channel->vcn_mode = ONE_PART;
		channel->channum1 = SI_Construct_Data(current,
							SVCT_VCM_VC_NUM_BYTE_INDX,
							SVCT_VCM_VC_NUM_BYTE_NUM,
							SVCT_VCM_VC_NUM_SHIFT,
							SVCT_VCM_VC_NUM_MASK);
		channel->path_select = SI_Construct_Data(current,
							SVCT_VCM_PATH_SEL_BYTE_INDX,
							SVCT_VCM_PATH_SEL_BYTE_NUM,
							SVCT_VCM_PATH_SEL_SHIFT,
							SVCT_VCM_PATH_SEL_MASK);
		channel->transport_type = SI_Construct_Data(current,
								SVCT_VCM_XPORT_TYPE_BYTE_INDX,
								SVCT_VCM_XPORT_TYPE_BYTE_NUM,
								SVCT_VCM_XPORT_TYPE_SHIFT,
								SVCT_VCM_XPORT_TYPE_MASK);
		channel->channel_type = SI_Construct_Data(current,
								SVCT_VCM_CHAN_TYPE_BYTE_INDX,
								SVCT_VCM_CHAN_TYPE_BYTE_NUM,
								SVCT_VCM_CHAN_TYPE_SHIFT,
								SVCT_VCM_CHAN_TYPE_MASK);
		channel->CDS_reference = SI_Construct_Data(current,
								SVCT_VCM_CDS_REF_BYTE_INDX,
								SVCT_VCM_CDS_REF_BYTE_NUM,
								SVCT_VCM_CDS_REF_SHIFT,
								SVCT_VCM_CDS_REF_MASK);
		channel->source_ID = SI_Construct_Data(current,
								SVCT_VCM_SOURCE_ID_BYTE_INDX,
								SVCT_VCM_SOURCE_ID_BYTE_NUM,
								SVCT_VCM_SOURCE_ID_SHIFT,
								SVCT_VCM_SOURCE_ID_MASK);
		SI_DBG_PRINT(E_SI_DBG_MSG,("SVCT VCM appflag %x, channel %x, xport type %x, channel type %x, source id %x, cds ref %x.\n", channel->appflag,channel->channum1,channel->transport_type,channel->channel_type,channel->source_ID,channel->CDS_reference));

		if (channel->transport_type == MPEG2_XPORT)
		{
			/* mpeg digital channel. */
			channel->ChanPropUnion.mpeg_prop.prog_num = SI_Construct_Data(current,
													SVCT_VCM_PROG_NUM_BYTE_INDX,
													SVCT_VCM_PROG_NUM_BYTE_NUM,
													SVCT_VCM_PROG_NUM_SHIFT,
													SVCT_VCM_PROG_NUM_MASK);
			channel->ChanPropUnion.mpeg_prop.MMS_reference = SI_Construct_Data(current,
													SVCT_VCM_MMS_REF_BYTE_INDX,
													SVCT_VCM_MMS_REF_BYTE_NUM,
													SVCT_VCM_MMS_REF_SHIFT,
													SVCT_VCM_MMS_REF_MASK);
			SI_DBG_PRINT(E_SI_DBG_MSG,("SVCT VCM MPEG channel: Prog num %x, mms ref %x.\n", channel->ChanPropUnion.mpeg_prop.prog_num, channel->ChanPropUnion.mpeg_prop.MMS_reference));
		}
		else
		{
			/* analog channel. */
			channel->ChanPropUnion.nonmpeg_prop.scrambled = SI_Construct_Data(current,
													SVCT_VCM_SCRAMBLED_BYTE_INDX,
													SVCT_VCM_SCRAMBLED_BYTE_NUM,
													SVCT_VCM_SCRAMBLED_SHIFT,
													SVCT_VCM_SCRAMBLED_MASK);
			channel->ChanPropUnion.nonmpeg_prop.video_standard = SI_Construct_Data(current,
													SVCT_VCM_VIDEO_STANDARD_BYTE_INDX,
													SVCT_VCM_VIDEO_STANDARD_BYTE_NUM,
													SVCT_VCM_VIDEO_STANDARD_SHIFT,
													SVCT_VCM_VIDEO_STANDARD_MASK);
			SI_DBG_PRINT(E_SI_DBG_MSG,("SVCT VCM ANALOG channel: standard %x, scramble %x.\n", channel->ChanPropUnion.nonmpeg_prop.video_standard, channel->ChanPropUnion.nonmpeg_prop.scrambled));
		}

		if (desc_incl)
		{
			/* get descriptors count. */
			desc_cnt = SI_Construct_Data(current,
						SVCT_VCM_DESC_CNT_BYTE_INDX,
						SVCT_VCM_DESC_CNT_BYTE_NUM,
						SVCT_VCM_DESC_CNT_SHIFT,
						SVCT_VCM_DESC_CNT_MASK);
			/* current -> first descriptor. */
			current += SVCT_VCM_DESC_CNT_BYTE_INDX+SVCT_VCM_DESC_CNT_BYTE_NUM;

			/* go through all descriptors. */
			for (j=0; j<desc_cnt; j++)
			{
				desc_tag = *(current++);
				len = *(current++);
				switch(desc_tag)
				{
					case SI_DESC_TWO_PART_CHANNEL_NO:
						SI_DBG_PRINT(E_SI_DBG_MSG,("SVCT VCM two part descriptor.\n"));

						channel->vcn_mode = TWO_PART;
						channel->channum1 = SI_Construct_Data(current,
											DESC_TPCND_MAJOR_NUMBER_BYTE_INDEX,
											DESC_TPCND_MAJOR_NUMBER_BYTE_NUM,
											DESC_TPCND_MAJOR_NUMBER_SHIFT,
											DESC_TPCND_MAJOR_NUMBER_MASK);
						channel->channum2 = SI_Construct_Data(current,
											DESC_TPCND_MINOR_NUMBER_BYTE_INDEX,
											DESC_TPCND_MINOR_NUMBER_BYTE_NUM,
											DESC_TPCND_MINOR_NUMBER_SHIFT,
											DESC_TPCND_MINOR_NUMBER_MASK);
					break;

					case SI_DESC_CHANNEL_PROPERTIES:
						SI_DBG_PRINT(E_SI_DBG_MSG,("SVCT VCM Chan properties descriptor.\n"));

						channel->more_prop = 1;
						channel->tsid = SI_Construct_Data(current,
										DESC_CPD_CHANNEL_TSID_BYTE_INDEX,
										DESC_CPD_CHANNEL_TSID_BYTE_NUM,
										DESC_CPD_CHANNEL_TSID_SHIFT,
										DESC_CPD_CHANNEL_TSID_MASK);
						channel->serv_type = SI_Construct_Data(current,
										DESC_CPD_SERVICE_TYPE_BYTE_INDEX,
										DESC_CPD_SERVICE_TYPE_BYTE_NUM,
										DESC_CPD_SERVICE_TYPE_SHIFT,
										DESC_CPD_SERVICE_TYPE_MASK);
						channel->chanbits = SI_Construct_Data(current,
										DESC_CPD_CHANNEL_BITS_BYTE_INDEX,
										DESC_CPD_CHANNEL_BITS_BYTE_NUM,
										DESC_CPD_CHANNEL_BITS_SHIFT,
										DESC_CPD_CHANNEL_BITS_MASK);
					break;

					default:
						SI_DBG_PRINT(E_SI_WRN_MSG,("SVCT VCM table descriptor %x received! Ignoring!\n", desc_tag));
					break;
				}

				/* update current pointer. */
				current += len;
			}
		}
		else
		{
			SI_DBG_PRINT(E_SI_DBG_MSG,("SVCT VCM NO descriptors.\n"));

			/* simply update the current pointer. */
			current += SVCT_VCM_MMS_REF_BYTE_INDX+SVCT_VCM_MMS_REF_BYTE_NUM;
		}

		/* insert the channel */
		SI_SVCT_VCM_Ins_Channel(channel);
	}

	return SI_SUCCESS;
}


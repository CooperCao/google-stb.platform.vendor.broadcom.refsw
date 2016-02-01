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
#include "si_aeit.h"
#include "si_aett.h"
#include "si_mgt.h"
#include "si_vct.h"
#include "si_lvct.h"
#include "si_nit.h"
#include "si_nit_cds.h"
#include "si_nit_mms.h"
#include "si_rrt.h"
#include "si_svct.h"
#include "si_svct_vcm.h"
#include "si_ntt.h"
#include "si_ntt_sns.h"


extern int GetInputString(char *s);
extern int GetInputChar(void);



void SI_Test_Main(void)
{
	char cmd[256];
	unsigned long end_loop = 0;
	int err;


	while (!end_loop)
	{
		printf("SI test menu:\n");
		printf("1 : Show All SVCT-VCM's we have got.\n");
		printf("2 : Show All NIT-CDS's we have got.\n");
		printf("3 : Show All NIT-MMS's we have got.\n");
		printf("4 : Show All NTT-SNS's we have got.\n");
		printf("5 : Show All LVCT's we have got.\n");
		printf("6 : Show All RRT's we have got.\n");
		printf("7 : Show All AEIT's we have got.\n");
		printf("8 : Show All AETT's we have got.\n");
		printf("0 : Quit this menu.\n");
		
		printf("\nEnter Option:");
		cmd[0] = GetInputChar();
		printf("\n");

		switch (cmd[0])
		{
			case '1':
			{
				extern unsigned char SVCT_VCM_version_number;
				extern unsigned long SVCT_VCM_section_mask[8];
				extern struct svct_vcm_channel_list SVCT_VCM_channels;
				extern SI_mutex m_svct_vcm;
				SI_SVCT_VCM_CHANNEL *channel;
				extern unsigned long NIT_CDS_Freq_KHZ_Table[NIT_CDS_MAX_NUM_OF_CARRIRE_INDEX];
				extern NIT_MMS_RECORD NIT_MMS_Table[NIT_MMS_MAX_TABLE_SIZE];  

				SI_mutex_lock(m_svct_vcm);
				
				channel = SI_LST_D_FIRST(&SVCT_VCM_channels);

				if (SVCT_VCM_version_number != 0xff)
				{
					int i;
					
					printf("SVCT_VCM version 0x%x\n", SVCT_VCM_version_number);
					printf("SVCT_VCM_section mask :\n");
					for (i=0; i<8; i++)
						printf("0x%08x, ", SVCT_VCM_section_mask[i]);
					printf("\n");
				}

				while (channel)
				{
					if (channel->vcn_mode == ONE_PART)
						printf("Channel %d: ", channel->channum1);
					else
						printf("Channel %d-%d: ", channel->channum1, channel->channum2);

					printf("Channel Frequency %d KHz, appflag 0x%x, xport type 0x%x, channel type(0:MPEG2, 1:NONMPEG2) 0x%x, source id 0x%x\n",
							NIT_CDS_Freq_KHZ_Table[channel->CDS_reference],
							channel->appflag,channel->transport_type,channel->channel_type,
							channel->source_ID);

					if (channel->transport_type == MPEG2_XPORT)
					{
						printf("     MPEG channel :Prog num 0x%x, mms ref %d.\n", 
								channel->ChanPropUnion.mpeg_prop.prog_num, 
								channel->ChanPropUnion.mpeg_prop.MMS_reference);
						printf("     MMS Info: xmit sys(2:ANNEXB, 4:ATSC) 0x%x, inner code 0x%x, modulation format(8:QAM64, 0x10:QAM256) 0x%x, symb rate %d symb/s.\n", 
								NIT_MMS_Table[channel->ChanPropUnion.mpeg_prop.MMS_reference].transmission_system,
								NIT_MMS_Table[channel->ChanPropUnion.mpeg_prop.MMS_reference].inner_coding_mode,
								NIT_MMS_Table[channel->ChanPropUnion.mpeg_prop.MMS_reference].modulation_format,
								NIT_MMS_Table[channel->ChanPropUnion.mpeg_prop.MMS_reference].symbol_rate);
					}
					else
					{
						printf("     Analog channel :standard(0-NTSC) 0x%x, scramble 0x%x.\n", 
								channel->ChanPropUnion.nonmpeg_prop.video_standard, 
								channel->ChanPropUnion.nonmpeg_prop.scrambled);
					}

					if (channel->more_prop)
						printf("     Additional Info (from descriptors: tsid 0x%x, serv_type 0x%x, chanbits 0x%x\n",
								channel->tsid, channel->serv_type, channel->chanbits);

					channel = SI_LST_D_NEXT(channel, chan_link);
				}
				SI_mutex_unlock(m_svct_vcm);
			}
			break;

			case '2':
			{
				extern unsigned char NIT_CDS_Version_Number;
				int i;
				extern unsigned long NIT_CDS_Section_Mask[8];
				extern unsigned long NIT_CDS_Freq_KHZ_Table[NIT_CDS_MAX_NUM_OF_CARRIRE_INDEX];


				if (NIT_CDS_Version_Number != 0xff)
				{					
					printf("NIT_CDS version 0x%x\n", NIT_CDS_Version_Number);
					printf("NIT_CDS_section mask :\n");
					for (i=0; i<8; i++)
						printf("0x%08x, ", NIT_CDS_Section_Mask[i]);
					printf("\n");
				}

				printf("NIT CDS table:\n");
				for (i=0; i<NIT_CDS_MAX_NUM_OF_CARRIRE_INDEX; i++)
				{
					if (NIT_CDS_Freq_KHZ_Table[i] != 0)
						printf("Index %d, Frequency %d kHz.\n", i, NIT_CDS_Freq_KHZ_Table[i]);
				}

			}
			break;

			case '3':
			{
				extern unsigned char NIT_MMS_Version_Number;
				int i;
				extern unsigned long NIT_MMS_Section_Mask[8];
				extern NIT_MMS_RECORD NIT_MMS_Table[NIT_MMS_MAX_TABLE_SIZE];  


				if (NIT_MMS_Version_Number != 0xff)
				{					
					printf("NIT_MMS version 0x%x\n", NIT_MMS_Version_Number);
					printf("NIT_MMS_section mask :\n");
					for (i=0; i<8; i++)
						printf("0x%08x, ", NIT_MMS_Section_Mask[i]);
					printf("\n");
				}

				printf("NIT MMS table:\n");
				for (i=0; i<NIT_MMS_MAX_TABLE_SIZE; i++)
				{
					if (NIT_MMS_Table[i].symbol_rate != 0)
						printf("Index %d, xmit sys(2:ANNEXB, 4:ATSC) 0x%x, inner code 0x%x, modulation format(8:QAM64, 0x10:QAM256) 0x%x, symb rate %d symb/s.\n", 
								i,NIT_MMS_Table[i].transmission_system,
								NIT_MMS_Table[i].inner_coding_mode,
								NIT_MMS_Table[i].modulation_format,
								NIT_MMS_Table[i].symbol_rate);
				}

			}
			break;

			case '4':
			{
				extern unsigned char NTT_SNS_version_number;
				extern unsigned long NTT_SNS_section_mask[8];
				extern struct NTT_SNS_List_Struct NTT_SNS_List;
				SI_NTT_SNS_LINK * sns = SI_LST_D_FIRST(&NTT_SNS_List);
				int i;

				if (NTT_SNS_version_number != 0xff)
				{					
					printf("NIT_SNS version 0x%x\n", NTT_SNS_version_number);
					printf("NIT_SNS_section mask :\n");
					for (i=0; i<8; i++)
						printf("0x%08x, ", NTT_SNS_section_mask[i]);
					printf("\n");
				}

				while (sns)
				{
					printf("NTT SNS appflag 0x%x, source id 0x%x :\n", sns->appflag, sns->source_ID);
					for (i=0; i<sns->name_len; i++)
					{
						if (i > 1)
							printf("%c", (char)sns->source_name[i]);		
					}
					printf("\n");

					sns = SI_LST_D_NEXT(sns, sns_link);
				}
			}
			break;

			case '5':
			{
				extern struct lvct_channel_list LVCT_channels;
				extern unsigned char LVCT_version_number;
				extern unsigned long LVCT_section_mask[8];
				extern SI_mutex m_lvct;
				SI_LVCT_CHANNEL * channel;
				int i;

				SI_mutex_lock(m_lvct);

				channel = SI_LST_D_FIRST(&LVCT_channels);
				
				if (LVCT_version_number != 0xff)
				{					
					printf("LVCT version 0x%x\n", LVCT_version_number);
					printf("LVCT_section mask :\n");
					for (i=0; i<8; i++)
						printf("0x%08x, ", LVCT_section_mask[i]);
					printf("\n");
				}

				while (channel)
				{
					if (channel->vcn_mode == ONE_PART)
						printf("Channel %d: ", channel->channum1);
					else
						printf("Channel %d-%d: ", channel->channum1, channel->channum2);

					for (i=0; i<LVCT_SHORT_NAME_LENGTH; i++)
						printf("%c", (channel->short_name[i]&0xff));

					if (channel->ext_name_len)
					{
						printf("  ext name: ");
						for (i=0; i<channel->ext_name_len; i++)
							if (i>7)
								printf("%c", channel->ext_name[i]);
					}
					printf("\n");

					printf("     modulation mode(1:Analog, 2:QAM64-ANNEXB, 3:QAM256-ANNEXB, 4:ATSC-8VSB, 5:ATSC-16VSB) 0x%x\n",
							channel->mod_mode);
					printf("     carrier freq %d hz, tsid 0x%x, program number0x%x, service type(1:analog, 2:ATSC digital, 3:ATSC audio) 0x%x, src id 0x%x, chanbits 0x%x\n", 
							channel->carrier_freq,channel->tsid,
							channel->program_num,channel->serv_type,channel->source_ID,
							channel->chanbits);

					if (channel->num_of_ts_serv)
					{
						printf("TS serv : ");
						for (i=0; i<channel->num_of_ts_serv; i++)
						{
							if (channel->time_shifted[i].vcn_mode == ONE_PART)
								printf("Channel %d, ", channel->time_shifted[i].channum1);
							else
								printf("Channel %d-%d, ", channel->time_shifted[i].channum1,
										channel->time_shifted[i].channum2);

							printf("time shift %d sec\n", channel->time_shifted[i].time_shift);
						}
					}

					channel = SI_LST_D_NEXT(channel, chan_link);
				}
				SI_mutex_unlock(m_lvct);
			}
			break;

			case '6':
			{
				extern struct rrt_region_list RRT_Regions;
				extern SI_mutex m_rrt;
				SI_RRT_REGION * rrt_region;
				int i, j, k;

				SI_mutex_lock(m_rrt);
				rrt_region = SI_LST_D_FIRST(&RRT_Regions);
				while (rrt_region)
				{
					printf("region 0x%x, region name :");
					for (i=0; i<rrt_region->rating_region_name_length; i++)
						if (i>7)
							printf("%c", (char)rrt_region->rating_region_name_text[i]);
					printf("  version 0x%x, dimensions 0x%x\n", rrt_region->version_number,
							rrt_region->dimensions_defined);

					for (i=0; i<rrt_region->dimensions_defined; i++)
					{
						printf("RRT dimension name: ");
						for (k=0; k<rrt_region->rrt_dimensions[i].dimension_name_length; k++)
							if (k>7)
								("%c", (char)rrt_region->rrt_dimensions[i].dimension_name_text[k]);
						printf("grad scale 0x%x\n", rrt_region->rrt_dimensions[i].graduated_scale);

						for (j=0; j<rrt_region->rrt_dimensions[i].values_defined; j++)
						{
							for (k=0; k<rrt_region->rrt_dimensions[i].rrt_values[j].abbrev_rating_value_length; k++)
								if (k>7)
									printf("%c", (char)rrt_region->rrt_dimensions[i].rrt_values[j].abbrev_rating_value_text[k]);
							printf(",   ");
							for (k=0; k<rrt_region->rrt_dimensions[i].rrt_values[j].rating_value_length; k++)
								if (k>7)
									printf("%c", (char)rrt_region->rrt_dimensions[i].rrt_values[j].rating_value_text[k]);
							printf("\n");
						}
					}

					rrt_region = SI_LST_D_NEXT(rrt_region, rrt_link);
				}
				SI_mutex_unlock(m_rrt);
			}
			break;

			case '7':
			{
				extern struct aeit_slot_list head_aeit_slot, current_aeit0_slot;
				extern unsigned short past_aeit_n;
				extern unsigned short mgt_aeit_n;
				extern SI_mutex m_aeit;
				SI_AEIT_SOURCE  *source;
				SI_AEIT_EVENT  *event;
				SI_AEIT_SLOT * slot;
				int i, j;

				printf("Past AEIT slots %d:\n", past_aeit_n);

				SI_mutex_lock(m_aeit);
				slot = SI_LST_D_FIRST(&head_aeit_slot);
				for (i=0; ((i<past_aeit_n) && slot); i++)
				{
					printf("Past AEIT slot %d, tag 0x%x, pid 0x%x, MGT ver 0x%x, ver 0x%x\n",
							i, slot->MGT_tag, slot->pid, slot->MGT_version_number,
							slot->version_number);
					printf("section mask: ");
					for (j=0; j<8; j++)
						printf("0x%08x,", slot->section_mask[j]);
					printf("\n");

					source = SI_LST_D_FIRST(&(slot->aeit_source));
					while (source)
					{
						printf("   source_id : 0x%x event list:\n", source->source_ID);
						event = SI_LST_D_FIRST(&(source->aeit_event));
						while(event)
						{
							printf("      id 0x%x, start 0x%x sec, dur 0x%x sec, title: ",
									event->event_ID, event->start_time, event->duration);
							for (j=0; j<event->title_length; j++)
								if (j>7)
									printf("%c", event->title_text[j]);
							printf("\n");

							event = SI_LST_D_NEXT(event, event_link);
						}
						source = SI_LST_D_NEXT(source, source_link);
					}
					slot = SI_LST_D_NEXT(slot, slot_link);
				}

				if (i!=past_aeit_n || slot!=SI_LST_D_FIRST(&current_aeit0_slot))
				{
					printf("past AEIT count error!!!\n"); 
				}

				printf("current AEIT slots:\n");
				slot = SI_LST_D_FIRST(&current_aeit0_slot);
				for (i=0; ((i<mgt_aeit_n) && slot); i++)
				{
					printf("AEIT %d, tag 0x%x, pid 0x%x, MGT ver 0x%x, ver 0x%x\n",
							i, slot->MGT_tag, slot->pid, slot->MGT_version_number,
							slot->version_number);
					printf("section mask: ");
					for (j=0; j<8; j++)
						printf("0x%08x,", slot->section_mask[j]);
					printf("\n");

					source = SI_LST_D_FIRST(&(slot->aeit_source));
					while (source)
					{
						printf("   source_id : 0x%x event list:\n", source->source_ID);
						event = SI_LST_D_FIRST(&(source->aeit_event));
						while(event)
						{
							printf("      id 0x%x, start 0x%x sec, dur 0x%x sec, title: ",
									event->event_ID, event->start_time, event->duration);
							for (j=0; j<event->title_length; j++)
								if (j>7)
									printf("%c", event->title_text[j]);
							printf("\n");

							event = SI_LST_D_NEXT(event, event_link);
						}
						source = SI_LST_D_NEXT(source, source_link);
					}
					slot = SI_LST_D_NEXT(slot, slot_link);
				}

				if (i!=mgt_aeit_n || slot)
				{
					printf("curr AEIT count error!!!\n"); 
				}
				SI_mutex_unlock(m_aeit);
			}
			break;

			case '8':
			{
				extern struct aett_slot_list head_aett_slot, current_aett0_slot;
				extern unsigned short past_aett_n;
				extern unsigned short mgt_aett_n;
				extern SI_mutex m_aett;
				SI_AETT_SLOT * slot;
				SI_AETT_SOURCE * source;
				SI_AETT_TEXT * text;
				int i, j;

				printf("Past AETT slots %d:\n", past_aett_n);

				SI_mutex_lock(m_aett);
				slot = SI_LST_D_FIRST(&head_aett_slot);
				for (i=0; ((i<past_aett_n) && slot); i++)
				{
					printf("Past AETT slot %d, tag 0x%x, pid 0x%x, MGT ver 0x%x, ver 0x%x\n",
							i, slot->MGT_tag, slot->pid, slot->MGT_version_number,
							slot->version_number);
					printf("section mask: ");
					for (j=0; j<8; j++)
						printf("0x%08x,", slot->section_mask[j]);
					printf("\n");

					source = SI_LST_D_FIRST(&(slot->aett_source));
					while (source)
					{
						printf("   source_id : 0x%x text list:\n", source->source_ID);
						text = SI_LST_D_FIRST(&(source->aett_text));
						while(text)
						{
							printf("      id 0x%x,  text: ",text->event_ID);
							for (j=0; j<text->extended_text_length; j++)
								if (j>7)
									printf("%c", text->extended_text_message[j]);
							printf("\n");

							text = SI_LST_D_NEXT(text, text_link);
						}
						source = SI_LST_D_NEXT(source, source_link);
					}
					slot = SI_LST_D_NEXT(slot, slot_link);
				}

				if (i!=past_aett_n || slot!=SI_LST_D_FIRST(&current_aett0_slot))
				{
					printf("past AETT count error!!!\n"); 
				}

				printf("current AETT slots:\n");
				slot = SI_LST_D_FIRST(&current_aett0_slot);
				for (i=0; ((i<mgt_aett_n) && slot); i++)
				{
					printf("AETT %d, tag 0x%x, pid 0x%x, MGT ver 0x%x, ver 0x%x\n",
							i, slot->MGT_tag, slot->pid, slot->MGT_version_number,
							slot->version_number);
					printf("section mask: ");
					for (j=0; j<8; j++)
						printf("0x%08x,", slot->section_mask[j]);
					printf("\n");

					source = SI_LST_D_FIRST(&(slot->aett_source));
					while (source)
					{
						printf("   source_id : 0x%x text list:\n", source->source_ID);
						text = SI_LST_D_FIRST(&(source->aett_text));
						while(text)
						{
							printf("      id 0x%x,  text: ",text->event_ID);
							for (j=0; j<text->extended_text_length; j++)
								if (j>7)
									printf("%c", text->extended_text_message[j]);
							printf("\n");

							text = SI_LST_D_NEXT(text, text_link);
						}
						source = SI_LST_D_NEXT(source, source_link);
					}
					slot = SI_LST_D_NEXT(slot, slot_link);
				}

				if (i!=mgt_aett_n || slot)
				{
					printf("curr AETT count error!!!\n"); 
				}

				SI_mutex_unlock(m_aett);
			}
			break;
			
			case '0':
				end_loop = 1;
			break;

			default:
			break;
		}
	}
}


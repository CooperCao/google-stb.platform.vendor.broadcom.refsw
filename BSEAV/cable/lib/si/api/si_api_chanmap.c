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
 ***************************************************************************/


#include "si.h"
#include "si_os.h"
#include "si_main.h"
#include "si_dbg.h"
#include "si_util.h"
#include "si_list.h"
#include "si_vct.h"
#include "si_svct_vcm.h"
#include "si_svct.h"
#include "si_lvct.h"
#include "si_nit_mms.h"
#include "si_nit_cds.h"
#include "si_descriptors.h"

#include "si_api_chanmap.h"
#include "nexus_video_types.h"
#include "nexus_audio_types.h"
#include "nexus_frontend_qam.h"

static unsigned char NIT_Version = 0xff;
static unsigned char NTT_SNS_version = 0xff;
static unsigned char SVCT_VCM_version = 0xff;
static unsigned char LVCT_version = 0xff;

extern struct svct_vcm_channel_list SVCT_VCM_channels;
extern unsigned long Total_SVCT_VCM_Channels;
extern SI_mutex m_svct_vcm;
extern struct lvct_channel_list LVCT_channels;
extern unsigned long Total_LVCT_Channels;
extern SI_mutex m_lvct;

extern NIT_MMS_RECORD NIT_MMS_Table[NIT_MMS_MAX_TABLE_SIZE];
extern unsigned long NIT_CDS_Freq_KHZ_Table[NIT_CDS_MAX_NUM_OF_CARRIRE_INDEX];
extern unsigned long Total_NIT_CDS_carriers;
extern unsigned long Total_NIT_MMS_entries;


static clear_chan_map()
{
	SI_LVCT_Free_List();
	SI_MGT_AEIT_Del_All_Slot();
	SI_MGT_AETT_Del_All_Slot();
	SI_NTT_SNS_Free_List();
	/*SI_RRT_UnInit();*/
	SI_SVCT_VCM_Free_List();

	SI_LVCT_Init();
	SI_MGT_Init();
	SI_NIT_Init();
	SI_NTT_Init();
	SI_RRT_Init();
	SI_SVCT_Init();
	NIT_Version = NTT_SNS_version = SVCT_VCM_version = LVCT_version = 0xff;
}

void SCTE_Api_Clear_Chan_Map(void)
{
	clear_chan_map();
}

void PSIP_Api_Clear_Chan_Map(void)
{
	clear_chan_map();
}


int PSIP_Api_Check_Chan_Map()
{
	unsigned char LVCT_version_number;
	unsigned long *p_LVCT_section_mask;
	int i;


	if (Total_LVCT_Channels==0) return -1;

	SI_LVCT_Get(&LVCT_version_number,&p_LVCT_section_mask);

	if ( LVCT_version == LVCT_version_number)
		return 1;

	/* make sure all sections are received before generating channel map*/
	for (i=0;i<5;i++)
	{
		if (p_LVCT_section_mask[i] != -1) return 1;
	}

	return 0;
}

int SCTE_Api_Check_Chan_Map()
{
	unsigned char NIT_Version_Number;
	unsigned char NTT_SNS_version_number;
	unsigned char SVCT_VCM_version_number;
	unsigned long *p_NIT_Section_Mask;
	unsigned long *p_NTT_SNS_section_mask;
	unsigned long *p_SVCT_VCM_section_mask;
	int i;

	SI_DBG_PRINT(E_SI_DBG_MSG,("SCTE_Api_Check_Chan_Map %d  %d  %d\n",Total_NIT_CDS_carriers,Total_NIT_MMS_entries, Total_SVCT_VCM_Channels  ));

	if (Total_NIT_CDS_carriers==0 || Total_NIT_MMS_entries==0
		|| Total_SVCT_VCM_Channels == 0)
		return -1;

	SI_NIT_Get(&NIT_Version_Number,&p_NIT_Section_Mask);
	SI_SVCT_Get(&SVCT_VCM_version_number,&p_SVCT_VCM_section_mask);
	SI_NTT_Get(&NTT_SNS_version_number,&p_NTT_SNS_section_mask);

	SI_DBG_PRINT(E_SI_DBG_MSG,("SCTE_Api_Check_Chan_Map %d  %d\n",NIT_Version_Number, SVCT_VCM_version_number  ));

	if ( NIT_Version == NIT_Version_Number
		&& SVCT_VCM_version == SVCT_VCM_version_number
		&& NTT_SNS_version == NTT_SNS_version_number)
		return 1;

	/* make sure all sections are received before generating channel map*/
	for (i=0;i<5;i++)
	{
 		if (p_NIT_Section_Mask[i] != -1
			|| p_SVCT_VCM_section_mask[i] != -1)
		{
			SI_DBG_PRINT(E_SI_DBG_MSG,("SCTE_Api_Check_Chan_Map %x  %x  \n",p_NIT_Section_Mask[i], p_SVCT_VCM_section_mask[i]  ));
			return 1;
 		}
 	}
	SI_DBG_PRINT(E_SI_DBG_MSG,("SCTE_Api_Check_Chan_Map newe channel available\n"));
	return 0;
}



int PSIP_Api_Get_Chan_Map(channel_list_t *list, unsigned int * num_ch, unsigned int freq)
{
	unsigned long i = 0;
	SI_LVCT_CHANNEL *lvct;
	unsigned long idx;
	int j;
	channel_info_t channel_info;

	*num_ch =0;
	if (Total_LVCT_Channels==0 )
	{
		return -1;
	}

	memset(&channel_info, 0, sizeof(channel_info_t));

	SI_mutex_lock(m_lvct);
	lvct = SI_LST_D_FIRST(&LVCT_channels);
	while (lvct) {
		if (lvct->vcn_mode==TWO_PART)
		{
			channel_info.vc_number = lvct->channum2<<16 | lvct->channum1;
		}
		else
			channel_info.vc_number = lvct->channum1;
		channel_info.source_id = lvct->source_ID;
		//carrier frequency is deprecated in TCVT or CVCT
		channel_info.frequency = (freq) ? freq : lvct->carrier_freq;
		channel_info.hidden = lvct->chanbits&LVCT_HIDDEN_MASK;
		channel_info.tsid = lvct->tsid;

		switch (lvct->mod_mode)
		{
			case LVCT_MM_SCTE1:
                channel_info.modulation = NEXUS_FrontendQamMode_e64;
				channel_info.symbolrate = 5056941;
				break;
			case LVCT_MM_SCTE2:
                channel_info.modulation = NEXUS_FrontendQamMode_e256;
				channel_info.symbolrate = 5360537;
				break;
			case LVCT_MM_ANALOG:
				SI_DBG_PRINT(E_SI_DBG_MSG,("Unsupported modulation mode from LVCT table (ANALOG)%d!!!\n", lvct->mod_mode));
				goto lnext;
			case LVCT_MM_8_VSB:
				SI_DBG_PRINT(E_SI_DBG_MSG,("Unsupported modulation mode from LVCT table %d!!!\n", lvct->mod_mode));
				SI_DBG_PRINT(E_SI_DBG_MSG,("Force to be QAM64!!!\n"));
				channel_info.modulation = NEXUS_FrontendQamMode_e64;
				channel_info.symbolrate = 5056941;
				break;
			case LVCT_MM_16_VSB:
				SI_DBG_PRINT(E_SI_DBG_MSG,("Unsupported modulation mode from LVCT table %d!!!\n", lvct->mod_mode));
				SI_DBG_PRINT(E_SI_DBG_MSG,("Force to be QAM256!!!\n"));
                channel_info.modulation = NEXUS_FrontendQamMode_e256;
				channel_info.symbolrate = 5360537;
				break;
			default:
				SI_DBG_PRINT(E_SI_DBG_MSG,("Reserved modulation mode from LVCT table %d!!!\n", lvct->mod_mode));
#if 1 //TODO
				SI_DBG_PRINT(E_SI_DBG_MSG,("++++force to be QAM64???? %d!!!\n", lvct->mod_mode));
				channel_info.modulation = NEXUS_FrontendQamMode_e64;
				channel_info.symbolrate = 5056941;
				break;
#endif
		}
		channel_info.program_number = lvct->program_num;
		channel_info.annex = NEXUS_FrontendQamAnnex_eB;
		// TODO:: handle only ASCII name
		for (i=0;i<LVCT_SHORT_NAME_LENGTH;i++)
			channel_info.program_title[i] = lvct->short_name[i]&0xff;
		channel_info.program_title[LVCT_SHORT_NAME_LENGTH] = 0;
		memcpy(channel_info.program_long_title, lvct->long_name, MAX_PROGRAM_TITLE_LENGTH);

		channel_info.service_media = CHANNEL_SERVICE_BROADCAST;
		if (lvct->program_info.num_audio_pids == 0 &&
			lvct->program_info.num_video_pids == 0)
		{
    		channel_info.video_pid = 0x1fff;
			for (j=0;j<MAX_AUDIO_STREAM;j++)
			{
            	channel_info.audio_pid[j] = 0x1fff;
				channel_info.audio_format[j] = 0;
			}
            channel_info.pcr_pid = 0x1fff;
			channel_info.video_format =  0;
		} else
		{
			channel_info.pcr_pid = lvct->program_info.pcr_pid;
			if (lvct->program_info.num_video_pids)
			{
	        	channel_info.video_pid = lvct->program_info.video_pids[0].pid;
				channel_info.video_format = (NEXUS_VideoCodec)lvct->program_info.video_pids[0].streamType;
			}
			for (j = 0; j < lvct->program_info.num_audio_pids, j< MAX_AUDIO_STREAM ; j++)
			{
				channel_info.audio_pid[j] = lvct->program_info.audio_pids[j].pid;
	            channel_info.audio_format[j] = (NEXUS_AudioCodec)lvct->program_info.audio_pids[j].streamType;
				memcpy(channel_info.iso639, lvct->program_info.audio_pids[j].iso639, ISO936_CODE_LENGTH);
			}
			channel_info.num_audio_streams = j;
			channel_info.num_total_streams = lvct->program_info.num_audio_pids+ lvct->program_info.num_video_pids;
		}
		SI_DBG_PRINT(E_SI_DBG_MSG,("PSIP Add channel %s!!!\n", channel_info.program_title ));
		channel_list_insert_channel(list, &channel_info);
		(*num_ch)++;
lnext:
		lvct = SI_LST_D_NEXT(lvct, chan_link);
	}

	SI_mutex_unlock(m_lvct);
	{
		unsigned char LVCT_version_number;
		unsigned long *p_LVCT_section_mask;
		SI_LVCT_Get(&LVCT_version_number,&p_LVCT_section_mask);
		LVCT_version = LVCT_version_number;
	}
	return 0;
}

int SCTE_Api_Get_Chan_Map( channel_list_t *list, unsigned int *num_ch )
{
	unsigned long i = 0;
	SI_SVCT_VCM_CHANNEL *svct;
	unsigned long idx;
	int j;
	channel_info_t channel_info;

	*num_ch  = 0;
	if (Total_NIT_CDS_carriers==0 || Total_NIT_MMS_entries==0
		|| Total_SVCT_VCM_Channels == 0 )
	{
		return -1;
	}

	memset(&channel_info, 0, sizeof(channel_info_t));

	SI_mutex_lock(m_svct_vcm);
	svct = SI_LST_D_FIRST(&SVCT_VCM_channels);
	while (svct)
	{
		/*if (svct->channel_type == NORMAL_CHAN)*/
		{
			if (svct->vcn_mode==TWO_PART)
			{
				channel_info.vc_number = svct->channum2<<16 | svct->channum1;
			}
			else
				channel_info.vc_number = svct->channum1;
			channel_info.source_id = svct->source_ID;
			channel_info.frequency = NIT_CDS_Freq_KHZ_Table[svct->CDS_reference]*1000; /* in Hz */
			channel_info.hidden = (svct->channel_type == HIDDEN_CHAN);
			channel_info.tsid = svct->tsid;

			if (svct->transport_type == MPEG2_XPORT)
			{
				channel_info.program_number = svct->ChanPropUnion.mpeg_prop.prog_num;
				idx = svct->ChanPropUnion.mpeg_prop.MMS_reference;
				switch (NIT_MMS_Table[idx].transmission_system)
				{
					case ITU_T_ANNEX_B_TX_SYS:
					break;
					case ATSC_TX_SYS:
					break;
					default:
					break;
				}
				channel_info.annex = NEXUS_FrontendQamAnnex_eB;

				switch (NIT_MMS_Table[idx].modulation_format)
				{

					case MOD_QAM64:
		                channel_info.modulation = NEXUS_FrontendQamMode_e64;
						channel_info.symbolrate = 5056941;
					break;

					case MOD_QAM256:
		                channel_info.modulation = NEXUS_FrontendQamMode_e256;
						channel_info.symbolrate = 5360537;
					break;
					case MOD_VSB8:
						SI_DBG_PRINT(E_SI_DBG_MSG,("Unsupported modulation format from MMS table %d!!!\n", NIT_MMS_Table[idx].modulation_format));
						SI_DBG_PRINT(E_SI_DBG_MSG,("Force to be QAM64!!!\n"));
		                channel_info.modulation = NEXUS_FrontendQamMode_e64;
						channel_info.symbolrate = 5056941;
					break;
					case MOD_VSB16:
						SI_DBG_PRINT(E_SI_DBG_MSG,("Unsupported modulation format from MMS table %d!!!\n", NIT_MMS_Table[idx].modulation_format));
						SI_DBG_PRINT(E_SI_DBG_MSG,("Force to be QAM256!!!\n"));
		                channel_info.modulation = NEXUS_FrontendQamMode_e256;
						channel_info.symbolrate = 5360537;
					break;
					default:
						SI_DBG_PRINT(E_SI_DBG_MSG,("Unknown modulation format from MMS table %d!!!\n", NIT_MMS_Table[idx].modulation_format));
						goto next;
					break;
				}

			//	if (channel_info.symbolrate != NIT_MMS_Table[idx].symbol_rate)
			//		SI_DBG_PRINT(E_SI_ERR_MSG,("inconsitent symbol rate %d!!!\n", NIT_MMS_Table[idx].symbol_rate));

				if (SI_NTT_SNS_Find_SrcId(channel_info.source_id ,&channel_info.program_title, MAX_PROGRAM_TITLE_LENGTH))
					strcpy(channel_info.program_title, "Unknown");

				channel_info.service_media = CHANNEL_SERVICE_BROADCAST;
        		channel_info.video_pid = 0x1fff;
				for (j=0;j<MAX_AUDIO_STREAM;j++)
	            	channel_info.audio_pid[j] = 0x1fff;
	            channel_info.pcr_pid = 0x1fff;
			}
			else
			{
				goto next;
			}
		}
		channel_list_add_channel(list, &channel_info);
		SI_DBG_PRINT(E_SI_DBG_MSG,("SCTE Add channel %s!!!\n", channel_info.program_title ));
		(*num_ch)++;
next:
		svct = SI_LST_D_NEXT(svct, chan_link);
	}

	SI_mutex_unlock(m_svct_vcm);

	{
		unsigned char NIT_Version_Number;
		unsigned char NTT_SNS_version_number;
		unsigned char SVCT_VCM_version_number;
		unsigned long *p_NIT_Section_Mask;
		unsigned long *p_NTT_SNS_section_mask;
		unsigned long *p_SVCT_VCM_section_mask;

		SI_NIT_Get(&NIT_Version_Number,&p_NIT_Section_Mask);
		SI_SVCT_Get(&SVCT_VCM_version_number,&p_SVCT_VCM_section_mask);
		SI_NTT_Get(&NTT_SNS_version_number,&p_NTT_SNS_section_mask);

		NIT_Version =NIT_Version_Number;
		SVCT_VCM_version =SVCT_VCM_version_number;
		NTT_SNS_version = NTT_SNS_version_number;
	}

 	return 0;
}



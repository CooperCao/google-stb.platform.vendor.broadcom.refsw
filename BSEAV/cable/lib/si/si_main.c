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
#include "si_main.h"
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
#include "si_rrt.h"
#include "si_svct.h"
#include "si_ntt.h"
#include "si_ea.h"
#include <stdlib.h>

/* Init all the SI elements. */
void SI_Init(SI_Callback *cb)
{
	if (getenv("SI_DEBUG") != NULL)
		SI_Dbg_Set_Debug_Level(E_SI_DBG_MSG);
	else
		SI_Dbg_Set_Debug_Level(E_SI_WRN_MSG);

	SI_LVCT_Init();
	SI_MGT_Init();
	SI_NIT_Init();
	SI_NTT_Init();
	SI_RRT_Init();
	SI_STT_Init(cb->stt_cb);
	SI_SVCT_Init();
	SI_EA_Init(cb->eas_cb);

}

void SI_UnInit(void)
{

	SI_LVCT_Free_List();
	SI_MGT_AEIT_Del_All_Slot();
	SI_MGT_AETT_Del_All_Slot();
	SI_NTT_SNS_Free_List();
	/*SI_RRT_UnInit();*/
	SI_SVCT_VCM_Free_List();
	SI_EA_UnInit();
	SI_STT_UnInit();

}

/* PSIP data processing entry point. */
void PSIP_Process_Data(unsigned char *data, unsigned long len)
{
	unsigned char *ptr = data;
	unsigned short section_len;

	SI_DBG_PRINT(E_SI_DBG_MSG,("PSIP Process: data len %x\n", len));
	while (((unsigned long)(ptr - data)) < len)
	{
		SI_DBG_PRINT(E_SI_DBG_MSG,("PSIP Process: table id %x\n", *ptr));

		switch (*ptr)
		{
			case PSIP_MGT_ID:
			case PSIP_RRT_ID:
			case PSIP_EIT_ID:
			case PSIP_ETT_ID:
			case PSIP_DET_ID:
			case PSIP_DST_ID:
			case PSIP_NRT_ID:
			case PSIP_LTST_ID:
			case PSIP_DCCSCT_ID:
			default:
				/*TODO*/
			break;

			case PSIP_TVCT_ID:
				SI_LVCT_Parse(ptr, VCT_MODE_TVCT);
			break;

			case PSIP_CVCT_ID:
				SI_LVCT_Parse(ptr, VCT_MODE_CVCT);
			break;

			case PSIP_STT_ID:
				SI_STT_parse(ptr);
			break;

			case PSIP_EAS_ID:
				SI_EA_Parse(ptr);
			break;

		}

		if (*ptr == 0x55 && ((ptr-data+2) == len))
		{
			ptr +=2;
			SI_DBG_PRINT(E_SI_ERR_MSG,("discard last padding byte 0x55\n"));
			break;
		}
		else
		{
			ptr++;
			section_len = ((((unsigned short)*ptr++)<<8)&0x0f00);
			section_len |= (((unsigned short)*ptr++)&0x00ff);

			SI_DBG_PRINT(E_SI_DBG_MSG,("PSIP Process: section len %x\n", section_len));
			ptr += section_len;
		}
	}

	if (((unsigned long)(ptr - data)) != len)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("PSIP Process: data length error %p %p %d!!!\n", ptr, data, len));
	}

}



/* SCTE65 data processing entry point. */
void SCTE_Process_Data(unsigned char *data, unsigned long len)
{
	unsigned char *ptr = data;
	unsigned short section_len;

	SI_DBG_PRINT(E_SI_DBG_MSG,("SCTE Process: data len %x\n", len));
	while (((unsigned long)(ptr - data)) < len)
	{
		SI_DBG_PRINT(E_SI_DBG_MSG,("SCTE Process: table id %x\n", *ptr));

		switch (*ptr)
		{
			case SI_NIT_TABLE_ID:
				SI_NIT_parse(ptr);
			break;

			case SI_NTT_TABLE_ID:
				SI_NTT_parse(ptr);
			break;

			case SI_SVCT_TABLE_ID:
				SI_SVCT_parse(ptr);
			break;

			case SI_STT_TABLE_ID:
				SI_STT_parse(ptr);
			break;

			case SI_MGT_TABLE_ID:
				SI_MGT_parse(ptr);
			break;

			case SI_LVCT_TABLE_ID:
				SI_LVCT_Parse(ptr, VCT_MODE_LVCT);
			break;

			case SI_RRT_TABLE_ID:
				SI_RRT_parse(ptr);
			break;

			case SI_AEIT_TABLE_ID:
				SI_AEIT_Parse(ptr);
			break;

			case SI_AETT_TABLE_ID:
				SI_AETT_Parse(ptr);
			break;

			case SI_EA_TABLE_ID:
				SI_EA_Parse(ptr);
			break;

			default:
			break;
		}

		{
			ptr++;
			section_len = ((((unsigned short)*ptr++)<<8)&0x0f00);
			section_len |= (((unsigned short)*ptr++)&0x00ff);

			SI_DBG_PRINT(E_SI_DBG_MSG,("SI Process: section len %x\n", section_len));
			ptr += section_len;
		}
	}

	if (((unsigned long)(ptr - data)) != len)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG,("Ext SI Process: data length error!!!\n"));
	}

}



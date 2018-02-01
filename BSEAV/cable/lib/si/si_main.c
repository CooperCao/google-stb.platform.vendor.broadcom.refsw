/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/

/***************************************************************************
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
#include "si_stt.h"
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



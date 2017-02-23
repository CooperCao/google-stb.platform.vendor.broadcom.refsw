/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef SI_MAIN_H
#define SI_MAIN_H

#include "si_api_ea.h"
#include "tspsimgr2.h"
#ifdef __cplusplus
extern "C" {
#endif

#define PSIP_MGT_ID		0xC7
#define PSIP_TVCT_ID	0xC8
#define PSIP_CVCT_ID	0xC9
#define PSIP_RRT_ID		0xCA
#define PSIP_EIT_ID		0xCB
#define PSIP_ETT_ID		0xCC
#define PSIP_STT_ID		0xCD
#define PSIP_DET_ID		0xCE
#define PSIP_DST_ID		0xCF
#define PSIP_NRT_ID		0xD1
#define PSIP_LTST_ID	0xD2
#define PSIP_DCCT_ID	0xD3
#define PSIP_DCCSCT_ID	0xD4
#define PSIP_EAS_ID		0xD8

typedef enum
{
	VCT_MODE_LVCT,
	VCT_MODE_CVCT,
	VCT_MODE_TVCT,
	VCT_MODE_SVCT
} VCT_MODE ;


typedef struct SI_Callback {
	void (*stt_cb)(unsigned long , bool);
	void (*eas_cb)(EA_MSG_INFO*);
} SI_Callback;
void SI_Init(SI_Callback *callback);
void SI_UnInit(void);
void SCTE_Process_Data(unsigned char *data, unsigned long len);
void PSIP_Process_Data(unsigned char *data, unsigned long len);

#ifdef __cplusplus
}
#endif


#endif


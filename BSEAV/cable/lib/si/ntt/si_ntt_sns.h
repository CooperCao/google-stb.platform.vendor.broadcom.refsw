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


#ifndef SI_NTT_SNS_H
#define SI_NTT_SNS_H


typedef struct _SI_NTT_SNS_LINK
{
	unsigned char appflag;			/* 1 for app, 0 for source. */
	unsigned short source_ID;		/* source_ID corresponding to the channel. */
	unsigned char name_len;
	unsigned char *source_name;
	SI_LST_D_ENTRY(_SI_NTT_SNS_LINK) sns_link;
} SI_NTT_SNS_LINK;

SI_LST_D_HEAD(NTT_SNS_List_Struct, _SI_NTT_SNS_LINK);


/* For the following, refer to table 5.12 of ANSI/SCTE65 2002 (DVS234) */
/* for SNS subtable. relative offset to SNS subtable start. */
#define NTT_SNS_NUM_REC_BYTE_INDX	0
#define NTT_SNS_NUM_REC_BYTE_NUM	1
#define NTT_SNS_NUM_REC_SHIFT		0
#define NTT_SNS_NUM_REC_MASK		0xff

/* for each SNS record, relative offset to the start of SNS record loop. */
#define NTT_SNS_APP_TYPE_BYTE_INDX		0
#define NTT_SNS_APP_TYPE_BYTE_NUM		1
#define NTT_SNS_APP_TYPE_SHIFT			7
#define NTT_SNS_APP_TYPE_MASK			0x1

#define NTT_SNS_SOURCE_ID_BYTE_INDX		1
#define NTT_SNS_SOURCE_ID_BYTE_NUM		2
#define NTT_SNS_SOURCE_ID_SHIFT			0
#define NTT_SNS_SOURCE_ID_MASK			0xffff

#define NTT_SNS_NAME_LEN_BYTE_INDX		3
#define NTT_SNS_NAME_LEN_BYTE_NUM		1
#define NTT_SNS_NAME_LEN_SHIFT			0
#define NTT_SNS_NAME_LEN_MASK			0xff

/* for descriptor count after the source name, relative to the point after the source name text. */
#define NTT_SNS_DESC_CNT_BYTE_INDX		0
#define NTT_SNS_DESC_CNT_BYTE_NUM		1
#define NTT_SNS_DESC_CNT_SHIFT			0
#define NTT_SNS_DESC_CNT_MASK			0xff



#ifdef __cplusplus
extern "C" {
#endif

void SI_NTT_SNS_Init(void);
SI_RET_CODE SI_NTT_SNS_Free_List(void);
unsigned char * SI_NTT_SNS_Pointer (unsigned char *table);
SI_RET_CODE SI_NTT_SNS_Parse (unsigned char *table,unsigned int iso639);
int SI_NTT_SNS_Find_SrcId(unsigned short id, unsigned char *name, int max_len);

#ifdef __cplusplus
}
#endif



#endif


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

#ifdef __cplusplus
}
#endif



#endif


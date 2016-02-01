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


#ifndef SI_MAIN_H
#define SI_MAIN_H

#include "si_api_ea.h"
#include "tspsimgr.h"
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


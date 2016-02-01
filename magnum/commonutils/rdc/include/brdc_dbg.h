/***************************************************************************
 *     Copyright (c) 2005-2012, Broadcom Corporation
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
#ifndef BRDC_DBG_H__
#define BRDC_DBG_H__

#include "brdc.h"

/* #define BRDC_USE_CAPTURE_BUFFER */

#ifdef __cplusplus
extern "C" {
#endif

/* describes the possible entries in a register DMA list */
typedef enum 
{
	BRDC_DBG_ListEntry_eCommand,
	BRDC_DBG_ListEntry_eRegister,
	BRDC_DBG_ListEntry_eData,
	BRDC_DBG_ListEntry_eEnd
	
} BRDC_DBG_ListEntry;

/* debug functions */

BERR_Code BRDC_DBG_SetList(
	BRDC_List_Handle  hList
	);

BERR_Code BRDC_DBG_SetList_isr(
	BRDC_List_Handle  hList
	);	

BERR_Code BRDC_DBG_GetListEntry(
	BRDC_List_Handle     hList,
	BRDC_DBG_ListEntry  *peEntry,
	uint32_t             aulArgs[4]
	);

BERR_Code BRDC_DBG_GetListEntry_isr(
	BRDC_List_Handle     hList,
	BRDC_DBG_ListEntry	*peEntry,
	uint32_t			 aulArgs[4]
	);
	
BERR_Code BRDC_DBG_DumpList(
	BRDC_List_Handle  hList
	);

typedef struct BRDC_DBG_CaptureBuffer {
	uint8_t *mem;
	int size; /* size of mem in bytes */
	int readptr, writeptr; /* offsets into mem */
	
	/* stats */
	int num_ruls;
	int total_bytes;

	bool enable; /* enable capture */
} BRDC_DBG_CaptureBuffer;

BERR_Code
BRDC_DBG_CreateCaptureBuffer(BRDC_DBG_CaptureBuffer *buffer, int size);
void
BRDC_DBG_DestroyCaptureBuffer(BRDC_DBG_CaptureBuffer *buffer);
void
BRDC_DBG_WriteCapture_isr(BRDC_DBG_CaptureBuffer *buffer, BRDC_Slot_Handle hSlot, BRDC_List_Handle hList);
void
BRDC_P_DBG_WriteCaptures_isr(BRDC_DBG_CaptureBuffer *buffer, BRDC_Slot_Handle *phSlots, BRDC_List_Handle hList, uint32_t ulSlots);

/* prefixes */
#define BRDC_DBG_RUL            1
#define BRDC_DBG_RUL_ERROR      2 /* to capture error messages (strings) into RUL log */
#define BRDC_DBG_RUL_TIMESTAMP  3
#define BRDC_DBG_RUL_MSG        4 /* to capture regular debug messages (strings) into RUL log */
#define BRDC_DBG_BVN_ERROR      BRDC_DBG_RUL_ERROR /* to capture BVN error messages (strings) into RUL log */

/* log errors from throughout the system */
void BRDC_DBG_LogErrorCode_isr(BRDC_Handle rdc, uint32_t prefix, const char *str);

/* called by application */
void BRDC_DBG_ReadCapture_isr(BRDC_Handle rdc, uint8_t *mem, int size, int *read);

void BRDC_DBG_EnableCapture_isr(BRDC_Handle rdc, bool enable);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BRDC_DBG_H__ */


/* end of file */

/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
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
    BRDC_DBG_ListEntry  *peEntry,
    uint32_t             aulArgs[4]
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

#ifdef BRDC_DEBUG
BERR_Code BRDC_Slot_SetList_NoArmSync_isr
    ( BRDC_Slot_Handle                 hSlot,
      BRDC_List_Handle                 hList );
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BRDC_DBG_H__ */


/* end of file */

/***************************************************************************
 *  Copyright (C) 2003-2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 ***************************************************************************/


#ifndef BDBG_FIFO_H
#define BDBG_FIFO_H

#include "berr_ids.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BDBG_P_Atomic {
	volatile long atomic;
} BDBG_P_Atomic;

typedef struct BDBG_Fifo_Token BDBG_Fifo_Token;

struct BDBG_Fifo_Token {
    BDBG_P_Atomic *marker;
};

typedef struct BDBG_Fifo *BDBG_Fifo_Handle;
typedef const struct BDBG_Fifo *BDBG_Fifo_CHandle;
typedef struct BDBG_FifoReader *BDBG_FifoReader_Handle;

typedef struct BDBG_Fifo_CreateSettings {
    size_t elementSize; /* size of the single element, mandatory */
    unsigned nelements; /* [optional] number of elements */
    size_t bufferSize; /* [optional] size of the preallocated buffer */
    void *buffer;       /* [optional] pointer to preallocated buffer */
} BDBG_Fifo_CreateSettings;



#define BERR_FIFO_NO_DATA   BERR_MAKE_CODE(BERR_DBG_ID, 0)
#define BERR_FIFO_BUSY      BERR_MAKE_CODE(BERR_DBG_ID, 1)
#define BERR_FIFO_OVERFLOW  BERR_MAKE_CODE(BERR_DBG_ID, 2)


void BDBG_Fifo_GetDefaultCreateSettings(BDBG_Fifo_CreateSettings *createSettings);
BERR_Code BDBG_Fifo_Create(BDBG_Fifo_Handle *pFifo, const BDBG_Fifo_CreateSettings *createSettings);
void BDBG_Fifo_Destroy(BDBG_Fifo_Handle fifo);
void *BDBG_Fifo_GetBuffer_isrsafe(BDBG_Fifo_Handle fifo, BDBG_Fifo_Token *token);
void BDBG_Fifo_CommitBuffer_isrsafe(const BDBG_Fifo_Token *token);
BERR_Code BDBG_FifoReader_Create(BDBG_FifoReader_Handle *pReader, BDBG_Fifo_Handle fifo);
void BDBG_FifoReader_Destroy(BDBG_FifoReader_Handle fifo);
BERR_Code BDBG_FifoReader_Read(BDBG_FifoReader_Handle fifo, void *buffer, size_t buffer_size);
BERR_Code BDBG_FifoReader_Resync(BDBG_FifoReader_Handle fifo);

#define BDBG_Fifo_GetBuffer(fifo, token) BDBG_Fifo_GetBuffer_isrsafe((fifo), (token))
#define BDBG_Fifo_CommitBuffer(token) BDBG_Fifo_CommitBuffer_isrsafe((token))

#ifdef __cplusplus
}
#endif

#endif  /* BDBG_FIFO_H */

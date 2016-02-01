/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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
void *BDBG_Fifo_GetBuffer(BDBG_Fifo_Handle fifo, BDBG_Fifo_Token *token);
void BDBG_Fifo_CommitBuffer(const BDBG_Fifo_Token *token);
BERR_Code BDBG_FifoReader_Create(BDBG_FifoReader_Handle *pReader, BDBG_Fifo_Handle fifo);
void BDBG_FifoReader_Destroy(BDBG_FifoReader_Handle fifo);
BERR_Code BDBG_FifoReader_Read(BDBG_FifoReader_Handle fifo, void *buffer, size_t buffer_size);
BERR_Code BDBG_FifoReader_Resync(BDBG_FifoReader_Handle fifo);

#ifdef __cplusplus
}
#endif

#endif  /* BDBG_FIFO_H */

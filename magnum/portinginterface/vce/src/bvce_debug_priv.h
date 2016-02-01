/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#ifndef BVCE_DEBUG_PRIV_H_
#define BVCE_DEBUG_PRIV_H_

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif

/* If FW Command MBX handshake polling desired, enable polling here */
#define BVCE_P_POLL_FW_MBX 0
#define BVCE_P_POLL_FW_COUNT 10000

#define BVCE_P_POLL_FW_DATAREADY 0

#if ( BVCE_P_DUMP_OUTPUT_CDB || BVCE_P_DUMP_OUTPUT_ITB || BVCE_P_DUMP_OUTPUT_ITB_DESC || BVCE_P_DUMP_ARC_DEBUG_LOG || BVCE_P_DUMP_USERDATA_LOG || BVCE_P_TEST_MODE || BVCE_P_DUMP_MAU_PERFORMANCE_DATA )
#include <stdio.h>

typedef struct BVCE_Debug_P_Log_Context* BVCE_Debug_Log_Handle;

bool
BVCE_Debug_P_OpenLog(
   char* szFilename,
   BVCE_Debug_Log_Handle *phLog
   );

void
BVCE_Debug_P_CloseLog(
   BVCE_Debug_Log_Handle hLog
   );

void
BVCE_Debug_P_WriteLog_isr(
   BVCE_Debug_Log_Handle hLog,
   const char *fmt,
   ...
   );

void
BVCE_Debug_P_WriteLogBuffer_isr(
   BVCE_Debug_Log_Handle hLog,
   const void * pBuffer,
   size_t uiNumBytes
   );

#endif

#ifdef __cplusplus
}
#endif

#endif /* BVCE_DEBUG_PRIV_H_ */

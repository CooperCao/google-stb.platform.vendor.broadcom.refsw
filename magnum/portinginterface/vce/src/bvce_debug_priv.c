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

#if ( BVCE_P_DUMP_OUTPUT_CDB || BVCE_P_DUMP_OUTPUT_ITB || BVCE_P_DUMP_OUTPUT_ITB_DESC || BVCE_P_DUMP_ARC_DEBUG_LOG || BVCE_P_DUMP_USERDATA_LOG || BVCE_P_TEST_MODE || BVCE_P_DUMP_MAU_PERFORMANCE_DATA )
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
/* base modules */
#include "bstd.h"           /* standard types */
#include "berr.h"           /* error code */
#include "bdbg.h"           /* debug interface */
#include "bkni.h"           /* kernel interface */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "bvce_debug_priv.h"

typedef struct BVCE_Debug_P_Log_Context
{
   int fdFile;
   FILE* hFile;
} BVCE_Debug_P_Log_Context;

bool
BVCE_Debug_P_OpenLog(
   char* szFilename,
   BVCE_Debug_Log_Handle *phLog
   )
{
   BVCE_Debug_Log_Handle hLog;

   hLog = (BVCE_Debug_Log_Handle) BKNI_Malloc( sizeof( BVCE_Debug_P_Log_Context ) );
   if ( NULL == hLog )
   {
      return false;
   }

   hLog->fdFile = open(szFilename, O_WRONLY);
   if ( 0 == hLog->fdFile )
   {
      BKNI_Free(hLog);
      return false;
   }

   hLog->hFile = fdopen(hLog->fdFile, "wb");
   if ( NULL == hLog->hFile )
   {
      close( hLog->fdFile );
      hLog->fdFile = 0;

      hLog->hFile = fopen(szFilename, "wb");
   }

   if ( NULL == hLog->hFile )
   {
      BKNI_Free(hLog);
      return false;
   }

   *phLog = hLog;

   return true;
}

void
BVCE_Debug_P_CloseLog(
   BVCE_Debug_Log_Handle hLog
   )
{
   if ( NULL != hLog )
   {
      if ( NULL != hLog->hFile )
      {
         fclose( hLog->hFile );
         hLog->hFile = NULL;
      }

      if ( 0 != hLog->fdFile )
      {
         close( hLog->fdFile );
         hLog->fdFile = 0;
      }

      BKNI_Free( hLog );
   }
}

void
BVCE_Debug_P_WriteLog_isr(
   BVCE_Debug_Log_Handle hLog,
   const char *fmt,
   ...
   )
{
   if ( NULL != hLog )
   {
      va_list argp;
      va_start ( argp, fmt );
      vfprintf( hLog->hFile, fmt, argp );
      va_end(argp);
   }
}

void
BVCE_Debug_P_WriteLogBuffer_isr(
   BVCE_Debug_Log_Handle hLog,
   const void * pBuffer,
   size_t uiNumBytes
   )
{
   fwrite( pBuffer, 1, uiNumBytes, hLog->hFile );
}

#else
void BVCE_Debug_P_Foo(void)
{
   return;
}
#endif
/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * [File Description:]
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
void BVCE_Debug_P_Foo(void);
void BVCE_Debug_P_Foo(void)
{
   return;
}
#endif

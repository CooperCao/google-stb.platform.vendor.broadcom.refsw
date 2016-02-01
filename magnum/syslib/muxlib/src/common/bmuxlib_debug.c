/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#include "bstd.h"           /* standard types */
#include "bkni.h"
#include "bmuxlib_debug.h"

#if (BMUXLIB_INPUT_P_DUMP_DESC || BMUXLIB_FNRT_P_DUMP_DESC || BMUXLIB_FNRT_P_TEST_MODE)
#include <stdio.h>

typedef struct BMuxlib_Debug_P_CSV_Context
{
   FILE *hFile;
} BMuxlib_Debug_P_CSV_Context;

bool BMuxlib_Debug_CSVOpen(BMuxlib_Debug_CSV_Handle *phCSV, const char *fname)
{
   BMuxlib_Debug_CSV_Handle hCSV = NULL;
   hCSV = (BMuxlib_Debug_CSV_Handle) BKNI_Malloc(sizeof(BMuxlib_Debug_P_CSV_Context));
   if (NULL == hCSV)
   {
      return false;
   }

   hCSV->hFile = fopen(fname, "wb");
   if (NULL == hCSV->hFile)
   {
      BKNI_Free(hCSV);
      return false;
   }

   *phCSV = hCSV;
   return true;
}

void BMuxlib_Debug_CSVClose(BMuxlib_Debug_CSV_Handle hCSV)
{
   if (NULL != hCSV)
   {
      if (NULL != hCSV->hFile)
      {
         fclose(hCSV->hFile);
         hCSV->hFile = NULL;
      }
      BKNI_Free(hCSV);
   }
}

void BMuxlib_Debug_CSVFlush(BMuxlib_Debug_CSV_Handle hCSV)
{
   if (NULL != hCSV && NULL != hCSV->hFile)
      fflush(hCSV->hFile);
}

int BMuxlib_Debug_CSVWrite(BMuxlib_Debug_CSV_Handle hCSV, const char *fmt, ...)
{
   int ret_val = 0;
   if (NULL != hCSV && NULL != hCSV->hFile)
   {
      va_list argp;
      va_start(argp, fmt);
      ret_val = vfprintf(hCSV->hFile, fmt, argp);
      va_end(argp);
   }
   return ret_val;
}

#endif

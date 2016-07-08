/***************************************************************************
 *     Copyright (c) 2002-2013, Broadcom Corporation
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

#include <stdio.h>          /* for printf */
#include <stdlib.h>

/* base modules */
#include "bstd.h"           /* standard types */
#include "bdbg.h"           /* debug interface */
#include "berr.h"           /* error code */
#include "bkni.h"           /* kernel interface */

#include "framework.h"
#include "framework_board.h"

/* porting interface */
#include "bvce_debug.h"

BDBG_MODULE(VCE_DBG_PARSE);

#define MESSAGE_SIZE 1024
int app_main( int argc, char **argv )
{
   int iErr;
   BSystem_Info sysInfo;
   FILE *hDebugLog;
   FILE *hDebugCsv;
   char szMessage[MESSAGE_SIZE];
   uint8_t *auiEntry;
   unsigned uiEntrySize = BVCE_Debug_GetEntrySize();

   iErr = BSystem_Init( argc, argv, &sysInfo );
   if ( iErr )
   {
       printf( "System init FAILED!\n" );
       return iErr;
   }

   BDBG_Init();
   BDBG_SetModuleLevel("BVCE_DBG", BDBG_eLog);

   auiEntry = BKNI_Malloc( uiEntrySize );

   /* Read File */
   {
      char *inFilename = "BVCE_DEBUG_LOG.bin";
      char *outFilename = "BVCE_DEBUG_LOG.csv";
      if ( argc > 1 )
      {
         inFilename = argv[1];
      }
      if ( argc > 2 )
      {
         outFilename = argv[2];
      }

      if ( argc > 3 )
      {
         BKNI_Printf("Enabling print to console\n", inFilename);
      }

      BKNI_Printf("Parsing %s --> %s\n", inFilename, outFilename);
      hDebugLog = fopen(inFilename,"rb");
      if (!hDebugLog)
      {
         BKNI_Printf("ERROR reading: %s\n", inFilename);
         return -1;
      }

      hDebugCsv = fopen(outFilename,"wb");
      if (!hDebugCsv)
      {
         BKNI_Printf("ERROR writing: %s\n", outFilename);
         return -1;
      }
   }

   {
      unsigned i=0;
      while ( BVCE_Debug_FormatLogHeader( i++, szMessage, MESSAGE_SIZE ) )
      {
         fprintf( hDebugCsv, "%s", szMessage );
      };
   }
   /* Parse each entry until end of file */
   while ( 0 != fread( auiEntry, 1, uiEntrySize, hDebugLog ) )
   {
      if ( argc > 3 )
      {
         BVCE_Debug_PrintLogMessageEntry( auiEntry );
      }

      BVCE_Debug_FormatLogMessage( auiEntry, szMessage, MESSAGE_SIZE );
      fprintf( hDebugCsv, "%s", szMessage );
   }

   fclose( hDebugLog );
   fclose( hDebugCsv );
   BKNI_Printf("Wrote BVCE_DEBUG_LOG.csv\n");

   return 0;
}

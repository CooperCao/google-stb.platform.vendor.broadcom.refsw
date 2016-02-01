/******************************************************************************
 * (c) 2002-2015 Broadcom Corporation
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
 *
 *****************************************************************************/


#include <stdio.h>          /* for printf */
#include <stdlib.h>

/* base modules */
#include "bstd.h"           /* standard types */
#include "bdbg.h"           /* debug interface */
#include "berr.h"           /* error code */
#include "bkni.h"           /* kernel interface */

/* porting interface */
#include "bvce_auth.h"
#include "bxvd_auth.h"
#include "bsid_auth.h"
#include "bxpt_auth.h"
#include "bdsp_auth.h"

BDBG_MODULE(FW_IMG_DUMP);

typedef BERR_Code (*BXXX_DumpImage)(
         unsigned eFirmwareId,
         void *pBuffer,
         unsigned uiBufferSize,
         void **pvCodeStart,
         unsigned *puiCodeSize
         );

typedef struct FirmwareDumpEntry
{
   const char szFriendlyId[256];
   unsigned uiImageId;
   BXXX_DumpImage fDumpImage;
   unsigned uiMaxImageSize;
} FirmwareDumpEntry;

#define BXXX_AUTH( _friendlyId, _imageId, _fDump, _maxImageSize ) { _friendlyId, _imageId, _fDump, _maxImageSize },

struct FirmwareDumpEntry astFirmwareDumpList[] =
{
#include "bvce_auth_fw.lst"
#include "bxvd_auth_fw.lst"
#include "bsid_auth_fw.lst"
#include "bxpt_auth_fw.lst"
#include "bdsp_auth_fw.lst"
};

#define xstr(s) str(s)
#define str(s) #s

#define BYTESWAP32(_value) ( ( ( (_value) >> 24 ) & 0x000000FF ) |\
                             ( ( (_value) >>  8 ) & 0x0000FF00 ) |\
                             ( ( (_value) <<  8 ) & 0x00FF0000 ) |\
                             ( ( (_value) << 24 ) & 0xFF000000 ) )

int main( int argc, char **argv )
{
   int result = 0;
   unsigned uiFirmwareIndex;
   void *pBuffer = NULL;

   BSTD_UNUSED( argc );
   BSTD_UNUSED( argv );

   BDBG_Init();
   BKNI_Init();

   for ( uiFirmwareIndex = 0; uiFirmwareIndex < sizeof(astFirmwareDumpList)/sizeof(FirmwareDumpEntry); uiFirmwareIndex++ )
   {
      BERR_Code rc = BERR_SUCCESS;
      char szFilename[1024];
      void *pvCodeStart = NULL;
      unsigned uiCodeSize = 0;


      /* Compute File Name */
      BKNI_Snprintf(szFilename, 1024, "fw_img_dump_%s_%s.bin", xstr(BCHP_CHIP), astFirmwareDumpList[uiFirmwareIndex].szFriendlyId );
      BKNI_Printf("[%u] Dumping %s...", uiFirmwareIndex, szFilename );

      pBuffer = BKNI_Malloc( astFirmwareDumpList[uiFirmwareIndex].uiMaxImageSize );
      BDBG_ASSERT( NULL != pBuffer );

      /* Zero out buffer */
      BKNI_Memset( pBuffer, 0, astFirmwareDumpList[uiFirmwareIndex].uiMaxImageSize );

      /* Dump image to buffer */
      rc = astFirmwareDumpList[uiFirmwareIndex].fDumpImage(
         astFirmwareDumpList[uiFirmwareIndex].uiImageId,
         pBuffer,
         astFirmwareDumpList[uiFirmwareIndex].uiMaxImageSize,
         &pvCodeStart,
         &uiCodeSize
         );

      if ( rc != BERR_SUCCESS )
      {
         BKNI_Printf("ERR: Dump Image returned an error code (%d)\n", rc);
         result = -1;
         BKNI_Free( pBuffer );
         break;
      }

      /* Swap Bytes to force big endian */
      BDBG_ASSERT( 0 == (uiCodeSize % 4));
      {
         unsigned i;

         for ( i = 0; i < uiCodeSize/4; i++ )
         {
            ((uint32_t*) pvCodeStart)[i] = BYTESWAP32( ((uint32_t*) pvCodeStart)[i] );
         }
      }

      /* Dump buffer to file */
      {
         FILE *fDumpFile = NULL;

         fDumpFile = fopen(szFilename, "wb");
         if ( NULL == fDumpFile )
         {
            BKNI_Printf("ERR: Cannot open file\n");
            result = -2;
            BKNI_Free( pBuffer );
            break;
         }

         /* Dump buffer to file */
         fwrite( pvCodeStart, 1, uiCodeSize, fDumpFile );
         fclose( fDumpFile );

         BKNI_Printf("%u bytes written\n", uiCodeSize);

         BKNI_Free( pBuffer );
      }
   }

   BKNI_Uninit();
   BDBG_Uninit();

   return -1;
}

/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
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

#include <stdio.h>          /* for printf */
#include <stdlib.h>

/* base modules */
#include "bstd.h"           /* standard types */
#include "bdbg.h"           /* debug interface */
#include "berr.h"           /* error code */
#include "bkni.h"           /* kernel interface */

/* porting interface */
#if 0
#include "bvce.h"
#endif

#include "framework.h"
#include "framework_board.h"

BDBG_MODULE(ASP_FW_LOAD);

#if 0

#if ((BCHP_CHIP == 7435) || (BCHP_CHIP == 7445) || (BCHP_CHIP == 7145) || (BCHP_CHIP == 7366))
#define NUM_INSTANCES 2
#define FW_HEAP_0 frmInfo.hMem
#define SYSTEM_HEAP_0 frmInfo.hMem
#define PICTURE_HEAP_0 frmInfo.hMem
#define SECURE_HEAP_0 frmInfo.hMem
#define FW_HEAP_1 frmInfo.hFrmWorkBoard->hMemc1
#define SYSTEM_HEAP_1 frmInfo.hFrmWorkBoard->hMemc1
#define PICTURE_HEAP_1 frmInfo.hFrmWorkBoard->hMemc1
#define SECURE_HEAP_1 frmInfo.hMem
#elif (BCHP_CHIP == 7425)
#define NUM_INSTANCES 1
#define FW_HEAP_0 frmInfo.hFrmWorkBoard->hMemc1
#define SYSTEM_HEAP_0 frmInfo.hFrmWorkBoard->hMemc1
#define PICTURE_HEAP_0 frmInfo.hFrmWorkBoard->hMemc1
#define SECURE_HEAP_0 frmInfo.hMem
#else
#error unsupported chip
#endif
#endif


int app_main( int argc, char **argv )
{
   /* Framework and System specific declarations */
   BERR_Code iErr = 0;
   BSystem_Info sysInfo;
   BFramework_Info frmInfo;
   BMEM_Heap_Handle hGeneralHeap;

   /* ASP specific declarations */
#if 0
   BVCE_Handle hVce;
   BVCE_OpenSettings vceOpenSettings;
#endif

   BSTD_UNUSED(argc);
   BSTD_UNUSED(argv);

   BDBG_Init();
#if 0
   BDBG_SetModuleLevel("BVCE", BDBG_eTrace);
   BDBG_SetModuleLevel("BVCE_IMAGE", BDBG_eTrace);
   BDBG_SetModuleLevel("BVCE_PLATFORM", BDBG_eTrace);
   BDBG_SetModuleLevel("BAFL", BDBG_eTrace);
   BDBG_SetModuleLevel("VCE_FW_LOAD", BDBG_eMsg);
   BDBG_MSG(("Initializing Framework"));
#endif

   BSystem_Init(argc, argv, &sysInfo);
   BFramework_Init(&sysInfo, &frmInfo);

#if 0

   BVCE_GetDefaultOpenSettings(&vceOpenSettings);

   vceOpenSettings.uiInstance = 0;

#if ( NUM_INSTANCES > 1 )
   if ( argc > 1 )
   {
      vceOpenSettings.uiInstance = atoi(argv[1]);
   }
#endif

   BDBG_MSG(("Calling BVCE_Open(%d)", vceOpenSettings.uiInstance));

   switch ( vceOpenSettings.uiInstance )
   {
      case 0:
         vceOpenSettings.hFirmwareMem[0] = FW_HEAP_0;
         vceOpenSettings.hFirmwareMem[1] = FW_HEAP_0;
         vceOpenSettings.hPictureMem = PICTURE_HEAP_0;
         vceOpenSettings.hSecureMem = SECURE_HEAP_0;
         hGeneralHeap = SYSTEM_HEAP_0;
         break;
#if ( NUM_INSTANCES > 1 )
      case 1:
         vceOpenSettings.hFirmwareMem[0] = FW_HEAP_1;
         vceOpenSettings.hFirmwareMem[1] = FW_HEAP_1;
         vceOpenSettings.hPictureMem = PICTURE_HEAP_1;
         vceOpenSettings.hSecureMem = SECURE_HEAP_1;
         hGeneralHeap = SYSTEM_HEAP_1;
         break;
#endif
      default:
         BDBG_ERR(("Unsupported instance (%d)", vceOpenSettings.uiInstance ));
         return -1;
   }

   iErr = BVCE_Open(&hVce,
                    frmInfo.hChp,
                    frmInfo.hReg,
		              hGeneralHeap,
                    frmInfo.hInt,
                    &vceOpenSettings);

   if (iErr != BERR_SUCCESS)
   {
      BDBG_ERR(("Error %d from BVCE_Open()", iErr));
      goto cleanup_framework;
   }
   else
   {
      BDBG_MSG(("Success opening VCE!"));

      /* Close VCE instances*/
      BDBG_MSG(("Closing VCE"));
      BVCE_Close( hVce );
   }
#endif

cleanup_framework:
   BFramework_Uninit(&frmInfo);
   BSystem_Uninit(&sysInfo);

   return iErr;
}

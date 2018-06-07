/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

/* base modules */
#include "bstd.h"           /* standard types */
#include "bdbg.h"           /* debug interface */
#include "berr.h"           /* error code */
#include "bkni.h"           /* kernel interface */

#include "bsid_auth.h"
#include "bsid_priv.h"
#include "bsid_fw_load.h"
#include "bsid_img.h"

static BERR_Code DummyCallback(void *pData, const BSID_BootInfo *pBootInfo)
{
   BSTD_UNUSED(pData);
   BSTD_UNUSED(pBootInfo);
   return BERR_SUCCESS;
}

BERR_Code
BSID_DumpImage(
   unsigned uiFirmwareId,
   void *pBuffer,
   unsigned uiBufferSize,
   void **pvCodeStart,
   unsigned *puiCodeSize
)
{
   BERR_Code rc = BERR_SUCCESS;
   BSID_Handle hSid = NULL;
   BSTD_UNUSED(uiFirmwareId);
   BSTD_UNUSED(uiBufferSize);

   BDBG_ASSERT( pvCodeStart );
   BDBG_ASSERT( puiCodeSize );

   *pvCodeStart = NULL;
   *puiCodeSize = 0;

   hSid = (BSID_Handle)BKNI_Malloc(sizeof (BSID_P_Context));
   if (hSid == NULL)
   {
       return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }

   /* init device handle structure to default value */
   BKNI_Memset((void *)hSid, 0x00, sizeof (BSID_P_Context));

   hSid->bArcReLoad = false;
   /* make sure we have a dummy callback or else it wont calculate the boot info */
   hSid->pExternalBootCallback = DummyCallback;
   hSid->sFwHwConfig.sCodeMemory.pv_CachedAddr = pBuffer;
   /* set the IMG interface to the internal interface */
   hSid->pImgInterface = &BSID_ImageInterface;
   hSid->pImgContext = BSID_ImageContext;

   rc = BSID_P_LoadCode(hSid);
   if (BERR_SUCCESS == rc)
   {
      *pvCodeStart = hSid->sBootInfo.pStartAddress;
      *puiCodeSize = hSid->sBootInfo.uiSize;
   }

   if (NULL != hSid)
      BKNI_Free(hSid);
   return BERR_TRACE(rc);
}

/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/
#include "bstd.h"

#include <linux/ktime.h>
#include <linux/module.h>
#include "bvc5_bin_pool_alloc_priv.h"

BDBG_MODULE(BVC5);

BERR_Code BVC5_P_GetTime_isrsafe(uint64_t *pMicroseconds)
{
   struct timespec tp;
   uint64_t res;

   getrawmonotonic(&tp);
   res = 1000000 * (uint64_t)tp.tv_sec;
   res += tp.tv_nsec / 1000;
   *pMicroseconds = res;

   return BERR_SUCCESS;
}

void BVC5_P_DebugDumpHeapContents(uint64_t ulOffset, unsigned uSize, uint32_t uiCoreIndex)
{
   /* Not implemented in kernel mode */
   BSTD_UNUSED(ulOffset);
   BSTD_UNUSED(uSize);
   BSTD_UNUSED(uiCoreIndex);

   BDBG_MSG(("Warning: Debug memory dump is not available in kernel mode"));
}

void BVC5_P_DRMOpen(uint32_t uiDRMDevice)
{
   /* Nothing to do in kernel mode */
   BSTD_UNUSED(uiDRMDevice);
}

/*
 * Exported entrypoint into the DRM kernel driver just for this module to use
 *
 * ARM compiler inserts relative branches here, which are not big enough to patch
 * during insmod.  Trampoline via a uintptr_t function pointer, to force to be
 * big enough for the relocation to work.
 */
extern uintptr_t v3d_drm_term_client_p __attribute__((weak));

bool BVC5_P_HasBrcmv3dko(void)
{
   return ((&v3d_drm_term_client_p) != NULL);
}

void BVC5_P_DRMTerminateClient(uint64_t uiPlatformToken)
{
   if (BVC5_P_HasBrcmv3dko())
   {
      void (*v3d_drm_term_client)(uint64_t) = (void (*)(uint64_t))v3d_drm_term_client_p;
      v3d_drm_term_client(uiPlatformToken);
   }
}

/* We have no BVC5 module handle to store the interface under, so it has to be
 * static here. This means that only one Android can be present at any time. */
static BVC5_BinPoolBlock_MemInterface s_memInterface;

/***************************************************************************/
/* Register Android's bin memory interface                                 */
BERR_Code BVC5_RegisterAlternateMemInterface(
   struct module                  *psModule,
   BVC5_BinPoolBlock_MemInterface *pMemInterface
   )
{
   if (pMemInterface->BinPoolBlock_Alloc  == NULL ||
       pMemInterface->BinPoolBlock_Free   == NULL ||
       pMemInterface->BinPoolBlock_Lock   == NULL ||
       pMemInterface->BinPoolBlock_Unlock == NULL)
      return BERR_INVALID_PARAMETER;

   if (!try_module_get(psModule))
      return BERR_NOT_AVAILABLE;

   /* only allow overridable bin memory on a non MMU system */
   /* coverity[dead_error_condition] */
   if (!BVC5_P_HasBrcmv3dko())
   {
      /* no override for DRM devices, as the bin memory must translate
         to pre existing MMU pages */
      s_memInterface = *pMemInterface;
   }

   return BERR_SUCCESS;
}

void BVC5_UnregisterAlternateMemInterface(
   struct module *psModule
   )
{
   BKNI_Memset(&s_memInterface, 0, sizeof(BVC5_BinPoolBlock_MemInterface));
   module_put(psModule);
}

EXPORT_SYMBOL(BVC5_RegisterAlternateMemInterface);
EXPORT_SYMBOL(BVC5_UnregisterAlternateMemInterface);

/***************************************************************************/
BVC5_BinPoolBlock_MemInterface *BVC5_P_GetBinPoolMemInterface(void)
{
   return &s_memInterface;
}

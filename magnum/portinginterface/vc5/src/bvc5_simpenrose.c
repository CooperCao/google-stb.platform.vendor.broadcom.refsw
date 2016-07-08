/***************************************************************************
 *     Broadcom Proprietary and Confidential. (c)2014 Broadcom.  All rights reserved.
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
 *
 **************************************************************************/

#if defined(BVC5_HARDWARE_SIMPENROSE)

#include "bstd.h"
#include "bvc5.h"
#include "bvc5_priv.h"
#include "bvc5_simpenrose_priv.h"
#include "bvc5_simpenrose_hw_regs.h"

/* NOTE : This is only designed to work in user-space. Kernel builds of Magnum will not
          be able to use this code.
*/
#include <dlfcn.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

BDBG_MODULE(BVC5_P);

/* Stop gcc moaning about dlsym */
#pragma GCC diagnostic ignored  "-pedantic"

#define MAP_FN(F, proto) s_hSim->pfn_##F = proto dlsym(s_hSim->pSimpenroseWrapperDLL, #F); BDBG_ASSERT(s_hSim->pfn_##F != NULL);

static BVC5_Handle            s_hVC5 = NULL;
static BVC5_SimpenroseHandle  s_hSim = NULL;

extern void BVC5_P_InterruptHandler_isr(void *pParm, int iValue);

static void BVC5_P_SimpenroseISR(int iCore)
{
   BSTD_UNUSED(iCore);
   BVC5_P_InterruptHandler_isr(s_hVC5, 0);
}

BERR_Code BVC5_P_SimpenroseInit(
   BVC5_Handle             hVC5,
   BVC5_SimpenroseHandle  *phSim
)
{
   BERR_Code             err;
   char                  *mptr;
   int                   fm;

   s_hVC5 = hVC5;

   if (s_hSim == NULL)
   {
      s_hSim = (BVC5_SimpenroseHandle)BKNI_Malloc(sizeof(struct BVC5_P_Simpenrose));
      if (s_hSim == NULL)
         return BERR_OUT_OF_SYSTEM_MEMORY;

      *phSim = s_hSim;

      BKNI_Memset(s_hSim, 0, sizeof(struct BVC5_P_Simpenrose));

      /* Open our simpenrose wrapper dll */
      s_hSim->pSimpenroseWrapperDLL = dlopen("libv3d_hw_shared.so", RTLD_NOW);
      if (s_hSim->pSimpenroseWrapperDLL == NULL)
      {
         BDBG_ERR(("Unable to open libv3d_hw_shared.so - is it in your LD_LIBRARY_PATH?"));
         BKNI_Free(s_hSim);
         *phSim = NULL;
         return BERR_NOT_AVAILABLE;
      }

      /* Now map the functions we will use */
      MAP_FN(v3d_hw_simpenrose_init_shared, (void(*)(void *heapPtr, size_t heapSize, unsigned int heapBasePhys, void(*isr)(int))));
      MAP_FN(v3d_hw_read_reg_shared, (uint32_t(*)(int core, int64_t addr)));
      MAP_FN(v3d_hw_write_reg_shared, (void(*)(int core, int64_t addr, uint32_t value)));

      /* Map uncached memory */
      fm = open("/dev/mem", O_RDWR | O_SYNC);

      mptr = (char*)mmap(hVC5->pvHeapStartUncached, hVC5->zHeapSize, PROT_READ | PROT_WRITE,
                           MAP_SHARED, fm, hVC5->ulHeapStartPhys);

      /* Initialise simpenrose now */
      BKNI_Printf("Initializing Simpenrose\n");
      s_hSim->pfn_v3d_hw_simpenrose_init_shared(mptr, hVC5->zHeapSize, hVC5->ulHeapStartPhys, BVC5_P_SimpenroseISR);

      err = BKNI_CreateMutex(&s_hSim->hMutex);
   }
   else
   {
      *phSim = s_hSim;
      err    = BERR_SUCCESS;
   }

   return err;
}

void BVC5_P_SimpenroseTerm(
   BVC5_Handle            hVC5,
   BVC5_SimpenroseHandle  hSim
   )
{
   BSTD_UNUSED(hVC5);
   BSTD_UNUSED(hSim);

   /* Currently there is no way to shutdown the simulator */

   /* BKNI_DestroyMutex(hSim->hMutex);
   BKNI_Free(hSim); */
}

uint32_t BVC5_P_SimpenroseReadRegister(
   BVC5_Handle  hVC5,
   uint32_t     uiCoreIndex,
   uint32_t     uiReg
)
{
   int64_t simReg = uiReg;
   return hVC5->hSimpenrose->pfn_v3d_hw_read_reg_shared(uiCoreIndex, simReg);
}

void BVC5_P_SimpenroseWriteRegister(
   BVC5_Handle  hVC5,
   uint32_t     uiCoreIndex,
   uint32_t     uiReg,
   uint32_t     uiValue
)
{
   int64_t simReg = uiReg;
   hVC5->hSimpenrose->pfn_v3d_hw_write_reg_shared(uiCoreIndex, simReg, uiValue);
}

#endif

/* Avoid compiler warning about empty translation unit */
void BVC5_P_SimpenroseDummy(void)
{
}

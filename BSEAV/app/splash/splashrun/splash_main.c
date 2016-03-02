/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

/* #define debug_only */
/* Linux stuff */
#include <stdio.h>          /* for printf */
#include <stdlib.h>
#include <string.h>
#include "bstd.h"
#include "bkni.h"

#include "bmem.h"
#include "bint.h"

#include "nexus_platform.h"
#ifndef debug_only
#include "priv/nexus_core.h"
#endif
#include "nexus_types.h"

/* Programming note:
 * ----------------
 *  The next two paragraphs involve some trickery in order to arrive at the
 *  correct file name. In a real application, please don't do this. You will
 *  have one or two include files with names similar to splash_vdc_rul.h. You
 *  know what these names are, because you picked them yourself when you built
 *  splashgen. Just #include these one or two files directly. Don't bother with
 *  STRINGIZE and macros. This complexity exists only to solve a problem in
 *  splashrun.
 */

/* Tuning from Makefile */
#define XSTRINGIZE(x) #x
#define  STRINGIZE(x) XSTRINGIZE(x)

/* pre-generated register and RUL header files from splashgen */
#include "splash_script_load.h"
#include STRINGIZE(SPLASH_RUL_HFILE)
#ifndef SPLASH_VERSION_2
#include "splash_vdc_reg.h"
#endif

BDBG_MODULE(splashrun);

int splash_script_run(
BREG_Handle hReg, BMEM_Handle *phMem, SplashData* pSplashData);


int main(void)
{
    NEXUS_Error rc;
    NEXUS_PlatformSettings platformSettings;
	NEXUS_MemoryStatus stat;
	BMEM_Handle  *phMem;
    int ii, jj;
#ifdef SPLASH_VERSION_2

/* Programming note:
 * ----------------
 *  The next line of code involves the use of a preprocessor macro to arrive
 *  at the correct function name. In a real application, please don't do this.
 *  You will have one or two function names similar to GetSplashData. You
 *  know what these names are, because you picked them yourself when you built
 *  splashgen. Just type one or two function names directly. Don't bother with
 *  the macro. This complexity exists only to solve a problem in splashrun.
 */
	SplashData* pSplashData = SPLASH_RUL_FUNCTION();
#else
	SplashData lSplashData = {
		BSPLASH_NUM_MEM,
		0,                        /* Fill in later */
		BSPLASH_NUM_SURFACE,
		&g_SplashSurfaceInfo[0],
		BSPLASH_NUM_DISPLAY,
		&g_SplashDisplayInfo[0],
		sizeof(g_aTriggerMap)/sizeof(g_aTriggerMap[0]),
		&g_aTriggerMap[0],
		sizeof(g_aulReg)/(2*sizeof(g_aulReg[0])),
		&g_aulReg[0]
	};
	SplashData* pSplashData = &lSplashData;
	lSplashData.iRulMemIdx = g_iRulMemIdx;
#endif

    NEXUS_SetEnv("NEXUS_BASE_ONLY_INIT","y");
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openI2c = false;
    platformSettings.openFrontend = false;
    platformSettings.openFpga=false;
    platformSettings.openOutputs = false;
    rc = NEXUS_Platform_Init(&platformSettings);

    BDBG_ASSERT(!rc);

	phMem =
		(BMEM_Handle *)BKNI_Malloc(sizeof(BMEM_Handle) * pSplashData->ulNumMem);
	BKNI_Memset(
		(void *)phMem, 0, sizeof(BMEM_Handle) * pSplashData->ulNumMem);
	for (ii=0; ii<NEXUS_MAX_HEAPS; ii++)
	{
		if (g_pCoreHandles->heap[ii].nexus == NULL)
			continue;

		rc = NEXUS_Heap_GetStatus(g_pCoreHandles->heap[ii].nexus, &stat);
		if (!rc)
		{
			bool bGotAll = true;

			BDBG_MSG(("mem %d: idx %d, type 0x%x", ii, stat.memcIndex, stat.memoryType));
			if (stat.memcIndex<pSplashData->ulNumMem &&
				stat.memoryType & NEXUS_MEMORY_TYPE_APPLICATION_CACHED /* &&
				stat.memoryType & NEXUS_MEMORY_TYPE_DRIVER_CACHED */)
			{
				*(phMem+stat.memcIndex) = NEXUS_Heap_GetMemHandle(g_pCoreHandles->heap[ii].nexus);
				BDBG_MSG(("   hMem %p", (void*)(*(phMem + stat.memcIndex))));
			}

			for (jj=0; jj<(int)pSplashData->ulNumMem; jj++)
			{
				if (NULL == *(phMem+jj))
				{
					bGotAll = false; break;
				}
			}
			if (bGotAll) break;
		}
	}

	for(ii=0; ii<(int)pSplashData->ulNumMem; ii++)
	{
		BMEM_HeapInfo  info;

		if (NULL == *(phMem+ii))
		{
			BDBG_ERR(("Failed to get heap %d.", ii));
			return 1;
		}
		BMEM_Heap_GetInfo(*(phMem+ii), &info);
		BDBG_ERR(("mem %d start addr:  virtual(0x%8.8x), offet(0x%8.8x), size %d",
			   ii, (uint32_t) info.pvAddress, info.ulOffset, info.zSize));
	}

#ifdef debug_only
    splash_script_run(NULL,NULL,NULL);
#else
    splash_script_run(g_pCoreHandles->reg, phMem, pSplashData);
#endif

	BKNI_Free(phMem);
    NEXUS_Platform_Uninit();
    return 0;
}

void APP_BREG_Write32(BREG_Handle RegHandle, uint32_t reg, uint32_t data)
{
	BSTD_UNUSED(RegHandle) ;
	BSTD_UNUSED(reg) ;
	BSTD_UNUSED(data) ;
}
/* End of File */

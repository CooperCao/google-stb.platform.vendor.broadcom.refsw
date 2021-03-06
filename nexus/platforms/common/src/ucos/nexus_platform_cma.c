/***************************************************************************
*     (c)2004-2014 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
***************************************************************************/
#include "nexus_platform_priv.h"
#include "bkni.h"

#define MAX_CMA 4

struct device_memory {
    struct {
        unsigned memcIndex, subIndex;
        unsigned offset;
        int size;
    } region[MAX_CMA];
};

/*
    NUM_CMA=2 => 1GB, MEMC0
    NUM_CMA=3 => 2GB, MEMC1
    NUM_CMA=4 => 3GB, MEMC2
*/

static unsigned int get_device_memory(struct device_memory *mem)
{
    unsigned int i;
    int num_cma=0;
    struct bsu_meminfo *xfd_mem;
    xfd_mem = xapi->xfd_mem;

    BKNI_Memset(mem, 0, sizeof(*mem));

    for(i=0; i < xapi->xfd_num_mem; i++) {
        if (xfd_mem[i].memc != (unsigned int)-1) {
            BDBG_MSG(("xfd:  memc=%d, base=0x%08x", xfd_mem[i].memc, xfd_mem[i].base));
            mem->region[i].memcIndex = xfd_mem[i].memc;
            mem->region[i].subIndex = (i==1 ? 1 : 0);
            if (i==0) {
                mem->region[0].offset = 0x11000000; /* Give 16MB to BSU application */
                mem->region[0].size = 488*1024*1024;  /* size is 0x2f800000 minus offset of 0x11000000 */
                mem->region[1].subIndex = 0;
            } else if (i==1) {
                mem->region[1].offset = 0x2f800000;
                mem->region[1].size = 264*1024*1024;
                mem->region[1].subIndex = 1;
            } else {
                mem->region[i].offset = xfd_mem[i].base;
                mem->region[i].size = xfd_mem[i].top - xfd_mem[i].base;
                mem->region[i].subIndex = 0;
            }
            BDBG_MSG(("mem:  offset=0x%08x, size=%d, memcIndex=%d, subIndex=%d", mem->region[i].offset, mem->region[i].size/(1024*1024), mem->region[i].memcIndex, mem->region[i].subIndex));
            num_cma++;
        }
    }
    return num_cma;
}

NEXUS_Error NEXUS_Platform_P_SetCoreCmaSettings(const NEXUS_PlatformSettings *pSettings, const NEXUS_PlatformMemory *pMemory, NEXUS_Core_Settings *pCoreSettings)
{
    unsigned i, j;
    struct device_memory mem;
    unsigned num_cma;

    num_cma = get_device_memory(&mem);
    for(i=0;i<NEXUS_MAX_HEAPS;i++)
    {
        if (!pSettings->heap[i].size) continue;
        BDBG_MSG(("pSettings:  i=%d, memcIndex=%d, subIndex=%d, size=%d", i, pSettings->heap[i].memcIndex, pSettings->heap[i].subIndex, pSettings->heap[i].size/(1024*1024)));
        for (j=0;j<num_cma;j++) {
            if (mem.region[j].memcIndex == pSettings->heap[i].memcIndex &&
                mem.region[j].subIndex == pSettings->heap[i].subIndex &&
                mem.region[j].size >= pSettings->heap[i].size) break;
        }
        if (j == num_cma) {
            return BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
        }
        if (pMemory->max_dcache_line_size > pCoreSettings->heapRegion[i].alignment) {
            pCoreSettings->heapRegion[i].alignment = pMemory->max_dcache_line_size;
        }
        pCoreSettings->heapRegion[i].offset = mem.region[j].offset;
        pCoreSettings->heapRegion[i].length = pSettings->heap[i].size;
        pCoreSettings->heapRegion[i].memcIndex = pSettings->heap[i].memcIndex;
        pCoreSettings->heapRegion[i].memoryType = pSettings->heap[i].memoryType;

        /* now consume the device memory */
        mem.region[j].offset += pSettings->heap[i].size;
        mem.region[j].size -= pSettings->heap[i].size;
    }
    return 0;
}

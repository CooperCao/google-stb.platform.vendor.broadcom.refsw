/***************************************************************************
*  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
* Description:
*   Setup the TLB for the rest of MEMC0 and MEMC1.
*
***************************************************************************/
/***********************************************************************
 *                           Include Files
 ***********************************************************************/
#include <stdio.h>
#include "bstd.h"
#include "cpuctrl.h"
#include "memmap.h"
#include "bkni.h"
#include "bchp_common.h"
#include "bcmmips.h"
#include "bsu-api.h"
#include "bsu-api2.h"

#if (BCHP_CHIP==7125) || (BCHP_CHIP==7231) || (BCHP_CHIP==7340) || (BCHP_CHIP==7344) || (BCHP_CHIP==7420) || (BCHP_CHIP==7425)
    #include "bchp_memc_ddr_0.h"
#endif

#if (BCHP_CHIP==7420)
    #include "bchp_memc_ddr_1.h"
#endif

#if (BCHP_CHIP==7231) || (BCHP_CHIP==7344) || (BCHP_CHIP==7425)
    #include "bchp_memc_ddr23_shim_addr_cntl_0.h"
#endif

#if (BCHP_CHIP==7420)
    #include "bchp_memc_ddr23_aphy_ac_0.h"
    #include "bchp_memc_ddr23_aphy_ac_1.h"
#endif

/***********************************************************************
 *                          Global Variables
 ***********************************************************************/

uint32_t mem0_size_mb;
uint32_t mem1_size_mb;

/***********************************************************************
 *                         External References
 ***********************************************************************/
extern void clear_tlb(void); 

/***********************************************************************
 *                          Local Functions
 ***********************************************************************/
#if __cplusplus
extern "C" {
#endif

#if __cplusplus
}
#endif

#define OFFS_16MB             0x01000000
#define OFFS_32MB             0x02000000
#define OFFS_64MB             0x04000000
#define OFFS_128MB            0x08000000
#define ENTRYLO_CACHED(x)     ((((x) >> 6) & 0x3fffffc0) | 0x1f)
#define ENTRYLO_UNCACHED(x)   ((((x) >> 6) & 0x3fffffc0) | 0x17)

#if defined(BMIPS3300)
    #define TLB_32MB 1
    #define TLB_64MB 2
    #define TLB_96MB 3
    #define TLB_128MB 4
    #define TLB_192MB 5
    #define TLB_256MB 6
    #define TLB_288MB 7
    #define TLB_320MB 8
    #define TLB_512MB 14
    #define TLB_768MB 22
#elif defined (BMIPS4380) || defined(BMIPS5000)
    #define TLB_128MB 1
    #define TLB_256MB 2
    #define TLB_384MB 3
    #define TLB_512MB 4
    #define TLB_640MB 5
    #define TLB_768MB 6
    #define TLB_896MB 7
    #define TLB_1024MB 8
#endif

uint32_t readl( uint32_t addr );

/************************************************************************
 *
 *  setup_tlb_512MB()
 *
 *  CFE only intializes a portion of MEMC0.  This function will initialize
 *  the remainder, which is 256MB, cached.
 *
 ************************************************************************/
static void setup_tlb_MEMC0_512MB(void)
{
    int i;

    printf("setting up tlb for an additional 256MB on MEMC0 (uncached virtual address=0x%x, cached virtual address=0x%x)\n", DRAM_0_VIRT_ADDR_START, DRAM_0_VIRT_CACHED_ADDR_START);

    /* Map 2 x 128MB = 256MB for MEMC0 */
    for (i=0; i<2; i++)
    {
        AddTBLEntry(DRAM_0_VIRT_ADDR_START+OFFS_128MB*i, ENTRYLO_UNCACHED(DRAM_0_PHYS_ADDR_START+OFFS_128MB*i), ENTRYLO_UNCACHED(DRAM_0_PHYS_ADDR_START+OFFS_128MB*i+OFFS_64MB));
    }

    /* Map 2 x 128MB = 256MB for MEMC0 */
    for (i=0; i<2; i++)
    {
        AddTBLEntry(DRAM_0_VIRT_CACHED_ADDR_START+OFFS_128MB*i, ENTRYLO_CACHED(DRAM_0_PHYS_ADDR_START+OFFS_128MB*i), ENTRYLO_CACHED(DRAM_0_PHYS_ADDR_START+OFFS_128MB*i+OFFS_64MB));
    }
}

/************************************************************************
 *
 *  setup_tlb_MEMC0_1024MB()
 *
 *  CFE only intializes a portion of MEMC0.  This function will initialize
 *  the remainder, which is 768MB.
 *
 ************************************************************************/
static void setup_tlb_MEMC0_1024MB(int uncache_size_128MB, int cache_size_128MB)
{
    int i;

    #if (BCHP_CHIP==7445)
        printf("setting up tlb for an additional 512MB on the 1024MB MEMC0 (uncached virtual address=0x%x, cached virtual address=0x%x)\n", DRAM_0_768MB_VIRT_ADDR_START, DRAM_0_768MB_VIRT_CACHED_ADDR_START);
    
        for (i=0; i<16; i++) { /* Map 16*32MB == 512MB */
            AddTBLEntry(DRAM_0_768MB_VIRT_ADDR_START+OFFS_32MB*i, ENTRYLO_UNCACHED(DRAM_0_768MB_PHYS_ADDR_START+OFFS_32MB*i), ENTRYLO_UNCACHED(DRAM_0_768MB_PHYS_ADDR_START+OFFS_32MB*i+OFFS_16MB));
        }

        /* cached address */
        for (i=0; i<16; i++) { /* Map 16*32MB == 512MB */
            AddTBLEntry(DRAM_0_768MB_VIRT_CACHED_ADDR_START+OFFS_32MB*i, ENTRYLO_CACHED(DRAM_0_768MB_PHYS_ADDR_START+OFFS_32MB*i), ENTRYLO_CACHED(DRAM_0_768MB_PHYS_ADDR_START+OFFS_32MB*i+OFFS_16MB));
        }
    #else
        #if defined(BMIPS3300)
            printf("setting up tlb for an additional 256MB on the 1024MB MEMC0 (uncached virtual address=0x%x, cached virtual address=0x%x)\n", DRAM_0_VIRT_ADDR_START, DRAM_0_VIRT_CACHED_ADDR_START);

            for (i=0; i<8; i++) { /* Map 8*32MB == 256MB */
                AddTBLEntry(DRAM_0_VIRT_ADDR_START+OFFS_32MB*i, ENTRYLO_UNCACHED(DRAM_0_PHYS_ADDR_START+OFFS_32MB*i), ENTRYLO_UNCACHED(DRAM_0_PHYS_ADDR_START+OFFS_32MB*i+OFFS_16MB));
            }

            /* cached address */
            for (i=0; i<8; i++) { /* Map 8*32MB == 256MB */
                AddTBLEntry(DRAM_0_VIRT_CACHED_ADDR_START+OFFS_32MB*i, ENTRYLO_CACHED(DRAM_0_PHYS_ADDR_START+OFFS_32MB*i), ENTRYLO_CACHED(DRAM_0_PHYS_ADDR_START+OFFS_32MB*i+OFFS_16MB));
            }
        #elif defined (BMIPS4380) || defined(BMIPS5000)
            if (uncache_size_128MB > 6) {
                printf("Error, uncache_size_128MB > 6\n");
            }

            if (cache_size_128MB > 6) {
                printf("Error, cache_size_128MB > 6\n");
            }

            printf("setting up tlb for an additional %dMB uncached memory at virtual address 0x%08x and an additional %dMB cached memory at virtual address 0x%08xon the 1024MB MEMC0\n", uncache_size_128MB*128, DRAM_0_VIRT_ADDR_START, cache_size_128MB*128, DRAM_0_VIRT_CACHED_ADDR_START);

            for (i=0; i<uncache_size_128MB; i++) { /* Map uncache_size_128MB*128MB == 128MB, 256MB, ... , 768MB */
                AddTBLEntry(DRAM_0_VIRT_ADDR_START+OFFS_128MB*i, ENTRYLO_UNCACHED(DRAM_0_PHYS_ADDR_START+OFFS_128MB*i), ENTRYLO_UNCACHED(DRAM_0_PHYS_ADDR_START+OFFS_128MB*i+OFFS_64MB));
            }

            /* cached address */
            for (i=0; i<cache_size_128MB; i++) { /* Map uncache_size_128MB*128MB == 128MB, 256MB, ..., 768MB */
                AddTBLEntry(DRAM_0_VIRT_CACHED_ADDR_START+OFFS_128MB*i, ENTRYLO_CACHED(DRAM_0_PHYS_ADDR_START+OFFS_128MB*i), ENTRYLO_CACHED(DRAM_0_PHYS_ADDR_START+OFFS_128MB*i+OFFS_64MB));
            }
        #else
            #error Undefined CPU
        #endif
    #endif
}

/************************************************************************
 *
 *  setup_tlb_MEMC0_2048MB()
 *
 *  CFE only intializes a portion of MEMC0.  This function will initialize
 *  the remainder, which is 1792MB.
 *
 ************************************************************************/
static void setup_tlb_MEMC0_2048MB(void)
{
    int i;

    printf("setting up tlb for an additional %dMB uncached memory at virtual address 0x%08x and an additional %dMB cached memory at virtual address 0x%08x on the 2048MB MEMC0\n", TLB_256MB*128, DRAM_0_VIRT_ADDR_START, TLB_512MB*128, DRAM_0_VIRT_CACHED_ADDR_START);
    printf("setting up tlb for an additional %dMB uncached memory at virtual address 0x%08x and an additional %dMB cached memory at virtual address 0x%08x on the 2048MB MEMC0\n", TLB_256MB*128, DRAM_0_VIRT_ADDR_START, TLB_512MB*128, DRAM_0_VIRT_CACHED_ADDR_START);

/*
    #define TLB_128MB 1
    #define TLB_256MB 2
    #define TLB_384MB 3
    #define TLB_512MB 4
    #define TLB_640MB 5
    #define TLB_768MB 6
    #define TLB_896MB 7
    #define TLB_1024MB 8
*/

    /* MEMC0 256MB uncached @ 0x10000000 */
    /* MEMC0 512MB cached @ 0x50000000 */
    /* setup_tlb_MEMC0_1024MB(TLB_256MB, TLB_512MB); */
    /* static void setup_tlb_MEMC0_1024MB(int uncache_size_128MB, int cache_size_128MB)*/

    for (i=0; i<TLB_256MB; i++) { /* Map uncache_size_128MB*128MB == 128MB, 256MB, ... , 768MB */
        AddTBLEntry(DRAM_0_VIRT_ADDR_START+OFFS_128MB*i, ENTRYLO_UNCACHED(DRAM_0_PHYS_ADDR_START+OFFS_128MB*i), ENTRYLO_UNCACHED(DRAM_0_PHYS_ADDR_START+OFFS_128MB*i+OFFS_64MB));
    }

    /* cached address */
    for (i=0; i<TLB_512MB; i++) { /* Map uncache_size_128MB*128MB == 128MB, 256MB, ..., 768MB */
        AddTBLEntry(DRAM_0_VIRT_CACHED_ADDR_START+OFFS_128MB*i, ENTRYLO_CACHED(DRAM_0_PHYS_ADDR_START+OFFS_128MB*i), ENTRYLO_CACHED(DRAM_0_PHYS_ADDR_START+OFFS_128MB*i+OFFS_64MB));
    }

    /* MEMC1 256MB uncached @ 0x30000000 */
    /* MEMC1 512MB cached @ 0xc0000000 */
    /* setup_tlb_MEMC1_1024MB(TLB_256MB, TLB_512MB); */
    /* static void setup_tlb_MEMC1_1024MB(int uncache_size_128MB, int cache_size_128MB) */

    for (i=0; i<TLB_256MB; i++) { /* Map uncache_size_128MB*128MB == 128MB, 256MB, 512MB, 768MB, 1024MB */
        AddTBLEntry(DRAM_1_VIRT_ADDR_START+OFFS_128MB*i, ENTRYLO_UNCACHED(DRAM_0_PHYS_ADDR_START+OFFS_128MB*i), ENTRYLO_UNCACHED(DRAM_0_PHYS_ADDR_START+OFFS_128MB*i+OFFS_64MB));
    }

    /* cached address */
    for (i=0; i<TLB_512MB; i++) { /* Map uncache_size_128MB*128MB == 128MB, 256MB, 512MB, 768MB, 1024MB */
        AddTBLEntry(DRAM_1_VIRT_CACHED_ADDR_START+OFFS_128MB*i, ENTRYLO_CACHED(DRAM_0_PHYS_ADDR_START+OFFS_128MB*i), ENTRYLO_CACHED(DRAM_0_PHYS_ADDR_START+OFFS_128MB*i+OFFS_64MB));
    }
}

/************************************************************************
 *
 *  
 setup_tlb_MEMC1_256MB()
 *
 ************************************************************************/
#ifdef BOARD_HAS_256MB_MEMC1
    static void setup_tlb_MEMC1_256MB(void)
    {
        int i;

        printf("setting up tlb for an additional 256MB on MEMC1 (uncached virtual address=0x%x, cached virtual address=0x%x)\n", DRAM_1_256MB_VIRT_ADDR_START, DRAM_1_256MB_VIRT_CACHED_ADDR_START);

        /* Map 2 x 128MB = 256MB for MEMC1 */
        for (i=0; i<2; i++)
        {
            AddTBLEntry(DRAM_1_256MB_VIRT_ADDR_START+OFFS_128MB*i, ENTRYLO_UNCACHED(DRAM_1_256MB_PHYS_ADDR_START+OFFS_128MB*i), ENTRYLO_UNCACHED(DRAM_1_256MB_PHYS_ADDR_START+OFFS_128MB*i+OFFS_64MB));
        }

        /* Map 2 x 128MB = 256MB for MEMC1 */
        for (i=0; i<2; i++)
        {
            AddTBLEntry(DRAM_1_256MB_VIRT_CACHED_ADDR_START+OFFS_128MB*i, ENTRYLO_CACHED(DRAM_1_256MB_PHYS_ADDR_START+OFFS_128MB*i), ENTRYLO_CACHED(DRAM_1_256MB_PHYS_ADDR_START+OFFS_128MB*i+OFFS_64MB));
        }
    }
#endif

/************************************************************************
 *
 *  setup_tlb_MEMC1_512MB()
 *
 ************************************************************************/
#ifdef BOARD_HAS_512MB_MEMC1
    static void setup_tlb_MEMC1_512MB(void)
    {
        int i;

        printf("setting up tlb for an additional 512MB on MEMC1 (uncached virtual address=0x%x, cached virtual address=0x%x)\n", DRAM_1_512MB_VIRT_ADDR_START, DRAM_1_512MB_VIRT_CACHED_ADDR_START);

        /* Map 4 x 128MB = 512MB for MEMC1 */
        for (i=0; i<4; i++)
        {
            AddTBLEntry(DRAM_1_512MB_VIRT_ADDR_START+OFFS_128MB*i, ENTRYLO_UNCACHED(DRAM_1_512MB_PHYS_ADDR_START+OFFS_128MB*i), ENTRYLO_UNCACHED(DRAM_1_512MB_PHYS_ADDR_START+OFFS_128MB*i+OFFS_64MB));
        }

        /* Map 4 x 128MB = 512MB for MEMC1 */
        for (i=0; i<4; i++)
        {
            AddTBLEntry(DRAM_1_512MB_VIRT_CACHED_ADDR_START+OFFS_128MB*i, ENTRYLO_CACHED(DRAM_1_512MB_PHYS_ADDR_START+OFFS_128MB*i), ENTRYLO_CACHED(DRAM_1_512MB_PHYS_ADDR_START+OFFS_128MB*i+OFFS_64MB));
        }
    }
#endif

/************************************************************************
 *
 *  setup_tlb_MEMC1_1024MB()
 *
 *  CFE only intializes a portion of MEMC1.  This function will initialize
 *  the remainder.
 *
 ************************************************************************/
static void setup_tlb_MEMC1_1024MB(int uncache_size_128MB, int cache_size_128MB)
{
    int i;

    #ifdef BMIPS3300
        printf("setting up tlb for 256MB on the 1024MB MEMC1 (uncached virtual addr=0x%x, cached virtual addr=0x%x)\n", DRAM_1_VIRT_ADDR_START, DRAM_1_VIRT_CACHED_ADDR_START);

        for (i=0; i<8; i++) { /* Map 8*32MB == 256MB */
            AddTBLEntry(DRAM_1_VIRT_ADDR_START+OFFS_32MB*i, ENTRYLO_UNCACHED(DRAM_1_PHYS_ADDR_START+OFFS_32MB*i), ENTRYLO_UNCACHED(DRAM_1_PHYS_ADDR_START+OFFS_32MB*i+OFFS_16MB));
        }

        /* cached address */
        for (i=0; i<8; i++) { /* Map 8*32MB == 256MB */
            AddTBLEntry(DRAM_1_VIRT_CACHED_ADDR_START+OFFS_32MB*i, ENTRYLO_CACHED(DRAM_1_PHYS_ADDR_START+OFFS_32MB*i), ENTRYLO_CACHED(DRAM_1_PHYS_ADDR_START+OFFS_32MB*i+OFFS_16MB));
        }
    #elif defined (BMIPS4380) || defined(BMIPS5000)
        if (uncache_size_128MB > 8) {
            printf("Error, uncache_size_128MB > 8\n");
        }

        if (cache_size_128MB > 8) {
            printf("Error, cache_size_128MB > 8\n");
        }

        printf("setting up tlb for an additional %dMB uncached memory at virtual address 0x%08x and an additional %dMB cached memory at virtual address 0x%08x on the 1024MB MEMC1\n", uncache_size_128MB*128, DRAM_1_VIRT_ADDR_START, cache_size_128MB*128, DRAM_1_VIRT_CACHED_ADDR_START);

        for (i=0; i<uncache_size_128MB; i++) { /* Map uncache_size_128MB*128MB == 128MB, 256MB, 512MB, 768MB, 1024MB */
            AddTBLEntry(DRAM_1_VIRT_ADDR_START+OFFS_128MB*i, ENTRYLO_UNCACHED(DRAM_1_PHYS_ADDR_START+OFFS_128MB*i), ENTRYLO_UNCACHED(DRAM_1_PHYS_ADDR_START+OFFS_128MB*i+OFFS_64MB));
        }

        /* cached address */
        for (i=0; i<cache_size_128MB; i++) { /* Map uncache_size_128MB*128MB == 128MB, 256MB, 512MB, 768MB, 1024MB */
            AddTBLEntry(DRAM_1_VIRT_CACHED_ADDR_START+OFFS_128MB*i, ENTRYLO_CACHED(DRAM_1_PHYS_ADDR_START+OFFS_128MB*i), ENTRYLO_CACHED(DRAM_1_PHYS_ADDR_START+OFFS_128MB*i+OFFS_64MB));
        }
    #else
        #error Undefined CPU
    #endif
}

/************************************************************************
 *
 *  setup_tlb()
 *
 *  CFE only intializes a portion of MEMC0.  This function will initialize
 *  the remainder, in addition to MEMC1.
 *
 ************************************************************************/
void setup_tlb(void)
{
    BREG_Handle regHandle;
    BCHP_MemoryInfo memoryInfo;
    int rc;

    /* 
     * BKNI_Init is called here, sooner than normal because BREG_Open()
     * calls BKNI_Malloc before NEXUS_Platform_Init was able to 
     * normally call BKNI_Init.
     */
    BKNI_Init();
    BREG_Open(&regHandle, (void *)(BCM_PHYS_TO_K1(BCHP_PHYSICAL_OFFSET)), BCHP_REGISTER_END, NULL);

    rc = BCHP_GetMemoryInfo_PreInit(regHandle, &memoryInfo);
    if (rc) BDBG_ASSERT(0);

    BREG_Close(regHandle);
    BKNI_Uninit();
    #if (BCHP_CHIP==7445)
        memory.memc[1].length=0;
    #endif

    mem0_size_mb = memoryInfo.memc[0].deviceTech / 8 * (memoryInfo.memc[0].width/memoryInfo.memc[0].deviceWidth);
    mem1_size_mb = memoryInfo.memc[1].deviceTech / 8 * (memoryInfo.memc[1].width/memoryInfo.memc[1].deviceWidth);
    printf("MEMC0=%dMB, MEMC1=%dMB\n", mem0_size_mb, mem1_size_mb);

    /* Clear TLB entries first */
    clear_tlb(); 

    if (mem0_size_mb == 2048) {
        /* Only setup MEMC0.  No need to check MEMC1 */
        setup_tlb_MEMC0_2048MB();
    }
    else {
        switch(mem0_size_mb)
        {
            case 256:
                break;

            case 512:
                setup_tlb_MEMC0_512MB();
                break;

            case 1024:
                switch(mem1_size_mb) {
                    case 0:
                        setup_tlb_MEMC0_1024MB(TLB_768MB, TLB_768MB);   /* Since Mem1=0MB, we can TLB all of Mem0. */
                        break;
                    case 1024:
                        setup_tlb_MEMC0_1024MB(TLB_256MB, TLB_512MB);
                        break;
                    default:
                        printf("unknown mem1 size\n");
                        BDBG_ASSERT(0);
                        break;
                }
                break;

/*
            case 2048:
                setup_tlb_MEMC0_2048MB();
                break;
*/

            default:
                printf("mem0_size_mb = %d\n", mem0_size_mb);
                BDBG_ASSERT(0);
        }

        switch(mem1_size_mb)
        {
            case 0:
                break;

    #if 0
            #ifdef BOARD_HAS_256MB_MEMC1
                case 256:
                    setup_tlb_MEMC1_256MB();
                    break;
            #endif
    #endif

            #ifdef BOARD_HAS_512MB_MEMC1
                case 512:
                    setup_tlb_MEMC1_512MB();
                    break;
            #endif

            case 1024:
                if (mem0_size_mb == 1024) {
                    setup_tlb_MEMC1_1024MB(TLB_256MB, TLB_512MB);
                }
                break;

            default:
                printf("mem1_size_mb = %d\n", mem1_size_mb);
                BDBG_ASSERT(0);
        }
    }
}

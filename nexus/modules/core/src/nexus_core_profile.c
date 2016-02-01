/***************************************************************************
*     (c)2008-2012 Broadcom Corporation
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "nexus_core_module.h"

#if B_HAS_BPROFILE || B_HAS_TRC
#include "bperf_counter.h"

#if NEXUS_PROFILE_OS_linuxuser
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

#ifdef b_perf_4380_base
volatile uint32_t *b_perf_4380_base=NULL;
static int mem_fd=-1;
#endif

static NEXUS_Error
NEXUS_Core_P_Perf_Init(void)
{
#ifdef NEXUS_PROFILE_OS_linuxuser
#ifdef b_perf_4380_base
    static uint32_t dummy_cnt[8];
    b_perf_4380_base = dummy_cnt;
#endif
#if B_PERF_BASE_ADDR
    {
        mem_fd = open("/dev/mem", O_RDWR|O_SYNC);
        if ( mem_fd >= 0 ) {
            b_perf_4380_base = mmap(0, B_PERF_BASE_LEN, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, B_PERF_BASE_ADDR);
            if ( MAP_FAILED == b_perf_4380_base) {
                b_perf_4380_base = dummy_cnt;
            }
        }
    }
#endif
#endif
    {
        /* set performance counters */
        static const struct {
            const char *name;
            const bperf_counter_mode *mode;
        } modes[] =  {
            {"dcache", &bperf_counter_dcache},
            {"icache", &bperf_counter_icache},
#ifdef bperf_counter_idle
            {"idle", &bperf_counter_idle},
#endif
#ifdef bperf_counter_memory
            {"memory", &bperf_counter_memory},
#endif
#ifdef bperf_counter_memory_cycles
            {"memory_cycles", &bperf_counter_memory_cycles},
#endif
#if BPERF_COUNTER_HAS_RAC
            {"rac_access", &bperf_counter_rac_access},
            {"rac_prefetch", &bperf_counter_rac_prefetch},
            {"rac_hit", &bperf_counter_rac_hit},
#endif
            {"rate", &bperf_counter_instructions}
        };
        unsigned i;
        const bperf_counter_mode *mode  = &bperf_counter_dcache; /* default mode */
        const char *cfg = NEXUS_GetEnv("profile_counters");
        if (cfg) {
            for(i=0;i<sizeof(modes)/sizeof(*modes);i++) {
                if (NEXUS_P_Base_StrCmp(cfg, modes[i].name)==0) {
                    mode = modes[i].mode;
                    break;
                }
            }
        }
        bperf_init(mode);
    }
    return NEXUS_SUCCESS;
}

static void
NEXUS_Core_P_Perf_Uninit(void)
{
#ifdef NEXUS_PROFILE_OS_linuxuser
    if(mem_fd>=0) {
        close(mem_fd);
        mem_fd = -1;
    }
#endif
}

#if B_HAS_TRC
void NEXUS_Core_Btrc_Report(const BTRC_Module *module, const char *moduleName)
{
    BSTD_UNUSED(moduleName);
    BTRC_Module_Report(module);
    return;
}
#endif

#else /* B_HAS_BPROFILE || B_HAS_TRC */
#define NEXUS_Core_P_Perf_Init() NEXUS_SUCCESS
#define NEXUS_Core_P_Perf_Uninit()

#endif /* B_HAS_BPROFILE || B_HAS_TRC */

#if B_HAS_BPROFILE
#include "bsymtable.h"
#include "bprofile.h"
#include "bprofile_tick.h"
#include "bchp_common.h"

BDBG_MODULE(nexus_base_profile);

const bsymbol_table *map;

static const char *
b_symtable_get_name(unsigned long addr, char *buf, size_t buf_len)
{
    unsigned offset;
    const char *name;

    name = bsymbol_lookup(map, addr, &offset);
    if (offset==0) {
        return name;
    }
    if (offset < 4096) { /* maximum function size */
        BKNI_Snprintf(buf, buf_len, "%s+0x%x", name, offset);
    } else {
        BKNI_Snprintf(buf, buf_len, "0x%08x", addr);
    }
    return buf;
}

static void
b_symtable_init(void)
{
    map = bsymbol_fixup();
    BDBG_ASSERT(map);
#if BDBG_DEBUG_BUILD
    {
        unsigned offset;
        static const char s_b_symtable_init[] = "b_symtable_init";
        const char *name = bsymbol_lookup(map, (unsigned)b_symtable_init, &offset);
        /* test whether lookup has returned right name */
        BDBG_MSG(("symtable: %s -> %s",  s_b_symtable_init, name));
        BDBG_ASSERT(name);
        BDBG_ASSERT(offset==0);
        BDBG_ASSERT(!NEXUS_P_Base_StrCmp(name, s_b_symtable_init));
    }
#endif
}

#define B_PROFILE_USE_MALLOC    0
#define B_PROFILE_NENTRIES  1024*1024
static struct {
    BMMA_Block_Handle block;
    bprofile_entry *table;
    unsigned ref_cnt; /* reference counter used to handle not matching calls to profiler_start/stop */
} b_profile = {NULL,NULL,0};



unsigned  b_ticks_100us;

NEXUS_Error
NEXUS_Core_P_Profile_Init(void)
{
    NEXUS_Error rc;
    bprofile_entry *table;

    rc = NEXUS_Core_P_Perf_Init();
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

    BDBG_CASSERT(B_PROFILE_NENTRIES>100);
    b_symtable_init();
#if B_PROFILE_USE_MALLOC
    table = BKNI_Malloc(B_PROFILE_NENTRIES*sizeof(*table));
#else
    BDBG_ASSERT(g_NexusCore.publicHandles.heap[0].mma);
    table = NULL;
    b_profile.block = BMMA_Alloc(g_NexusCore.publicHandles.heap[0].mma, B_PROFILE_NENTRIES*sizeof(*table), 0, NULL);
    if(b_profile.block) {
        table = BMMA_Lock(b_profile.block);
        if(!table) {
            BMMA_Free(b_profile.block);
            b_profile.block = NULL;
        }
    }
#endif
    if (!table) {
        BDBG_ERR(("Can't allocate %u entries for the profiler array", B_PROFILE_NENTRIES));
        return BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
    }
    b_profile.table = table;
    b_profile.ref_cnt = 0;
    /* calibrate CPU tick */
    {
    NEXUS_Time time_start, time_stop;
    unsigned time_delta;
    unsigned tick_start, tick_stop;
    unsigned tick_delta;

        tick_start = b_gettick();
        NEXUS_Time_Get(&time_start);
        BKNI_Sleep(200);
        tick_stop = b_gettick();
        NEXUS_Time_Get(&time_stop);
        time_delta = NEXUS_Time_Diff(&time_stop, &time_start);
        tick_delta = tick_stop - tick_start;
        BDBG_ASSERT(tick_delta>0);
        BDBG_ASSERT(time_delta>0);
        b_ticks_100us =  tick_delta/(time_delta*10);
        BDBG_ASSERT(b_ticks_100us>0);
        BDBG_MSG(("b_ticks_100us %u , clock = %u time = %u ticks = %u time'=%u", b_ticks_100us, b_ticks_100us*10*1000, time_delta, tick_delta, tick_delta/b_ticks_100us));
    }
    bprofile_calibrate(b_profile.table, B_PROFILE_NENTRIES);
    return NEXUS_SUCCESS;
}

void
NEXUS_Core_P_Profile_Uninit(void)
{
    if (b_profile.ref_cnt>0) {
        b_profile.ref_cnt = 0;
        bprofile_stop();
    }
    NEXUS_Core_P_Perf_Uninit();
    BDBG_ASSERT(b_profile.table);
#if B_PROFILE_USE_MALLOC
    BKNI_Free(b_profile.table);
#else
    BDBG_ASSERT(g_NexusCore.publicHandles.heap[0].mma);
    BDBG_ASSERT(b_profile.table);
    BMMA_Unlock(b_profile.block, b_profile.table);
    BMMA_Free(b_profile.block);
    b_profile.block=NULL;
#endif
    b_profile.table = NULL;
    return;
}

NEXUS_Error
NEXUS_Profile_Start(void)
{
    NEXUS_Error rc;
    if (b_profile.ref_cnt==0) {
        BDBG_ASSERT(b_profile.table);
        BDBG_WRN(("starting profiling"));
        bprofile_start(b_profile.table, B_PROFILE_NENTRIES);
        rc = NEXUS_SUCCESS;
    } else {
        BDBG_WRN(("profiler already started %u times", b_profile.ref_cnt));
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    b_profile.ref_cnt++;
    return rc;
}

static const char *
b_task_thread_name(bprofile_thread_id thread)
{
    /* in our case thread_id points to the thread name */
    return (const char *)thread;
}

static bprofile_thread_id
b_task_from_stack(const unsigned long *stack)
{
    bprofile_thread_id  id;

    id = (bprofile_thread_id)NEXUS_P_Base_Os_GetTaskNameFromStack(stack);
    return id;
}

void
NEXUS_Profile_Stop(const char *name)
{
    int nentries;
    bprofile_sys_iface sys_iface;
    const char *cfg;

    BDBG_ASSERT(name);
    if (b_profile.ref_cnt<=0) {
        BDBG_WRN(("profiler already stopped"));
        return;
    }
    b_profile.ref_cnt--;
    if (b_profile.ref_cnt>0) {
        nentries = bprofile_poll();
        BDBG_WRN(("profiler accumulated %d entries and there are %u oustanding requests",  nentries, b_profile.ref_cnt));
        if (nentries >=0 && nentries < (B_PROFILE_NENTRIES*99)/100) {
            return;
        }
    }
    b_profile.ref_cnt=0; /* clear counter  and stop profiling */
    nentries = bprofile_stop();
    BKNI_Sleep(20);

    BDBG_WRN(("Processing profile report:'%s' (%d samples)", name, nentries));
    if (nentries<=0) {
        BDBG_WRN(("no profile data was captured"));
        return;
    }
    BKNI_Printf("profile report for %s\n", name);
    if (nentries >= B_PROFILE_NENTRIES-10) {
        BDBG_WRN(("BProfile has used %d out of %d of profile samples, results might be not accurate", nentries, B_PROFILE_NENTRIES));
    }
    bprofile_sys_iface_init(&sys_iface);
    sys_iface.get_name = b_symtable_get_name;
    sys_iface.thread_from_stack = b_task_from_stack;
    sys_iface.thread_name = b_task_thread_name;
    sys_iface.maxentries = 2000;
    sys_iface.show_entries = 64;
    if(NEXUS_GetEnv("profile_maxentries")) {
        sys_iface.maxentries = NEXUS_atoi(NEXUS_GetEnv("profile_maxentries"));
    }
    if(NEXUS_GetEnv("profile_entries")) {
        sys_iface.show_entries = NEXUS_atoi(NEXUS_GetEnv("profile_entries"));
    }
    cfg = NEXUS_GetEnv("profile_show");
    if (cfg) {
        sys_iface.show_entries = NEXUS_atoi(cfg);
    }
    sys_iface.split_threads = NEXUS_GetEnv("profile_perthread")!=NULL;
    sys_iface.substract_overhead = NEXUS_GetEnv("profile_accurate")!=NULL;
    sys_iface.preempt_time = NEXUS_GetEnv("profile_preempt")!=NULL;
    sys_iface.call_count = NEXUS_GetEnv("profile_calls")!=NULL;
    bprofile_report_flat(b_profile.table, nentries, &sys_iface);
    return;
}

void
NEXUS_Profile_MarkThread(const char *name)
{
    NEXUS_P_Base_Os_MarkThread(name);
    return;
}



#else /* B_HAS_BPROFILE */

NEXUS_Error
NEXUS_Profile_Start(void)
{
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void
NEXUS_Profile_Stop(const char *name)
{
    BSTD_UNUSED(name);
    return;
}

NEXUS_Error
NEXUS_Core_P_Profile_Init(void)
{
    return NEXUS_Core_P_Perf_Init();
}

void
NEXUS_Core_P_Profile_Uninit(void)
{
    NEXUS_Core_P_Perf_Uninit();
    return;
}

void
NEXUS_Profile_MarkThread(const char *name)
{
    BSTD_UNUSED(name);
    return;
}


#endif

/***************************************************************************
*     (c)2011 Broadcom Corporation
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
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bprofile.h"
#include "bprofile_tick.h"
#include "bperf_counter.h"
#include "bsymtable.h"

BDBG_MODULE(profile_app);

const bsymbol_table *map;
void run_idct_test(void);

#ifndef B_SYSTEM_linuxkernel
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

#ifdef b_perf_4380_base
static uint32_t dummy_cnt[8];
volatile uint32_t *b_perf_4380_base=dummy_cnt;
static int mem_fd=-1;
#endif

unsigned  b_ticks_100us = 300 * 1000 * 1000;

#if B_HAS_BPROFILE
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
		BDBG_ASSERT(!strcasecmp(name, s_b_symtable_init));
	}
#endif
}
#endif /* B_HAS_BPROFILE */

#define B_PROFILE_NENTRIES	(1024*1024)
void 
run_profile(void)
{
#if B_HAS_BPROFILE
	bprofile_entry *table = BKNI_Malloc(B_PROFILE_NENTRIES*sizeof(*table));
	int nentries;
	bprofile_sys_iface sys_iface;

	b_symtable_init();
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
    bperf_init(&bperf_counter_dcache);

#if B_HAS_BPROFILE
	{
	unsigned time_delta;
	unsigned tick_start, tick_stop;
	unsigned tick_delta;
	
		BKNI_Sleep(100);
		tick_start = b_gettick();
		BKNI_Sleep(100);
		time_delta = 100;
		tick_stop = b_gettick();
		tick_delta = tick_stop - tick_start;
		BDBG_ASSERT(time_delta>0);
		b_ticks_100us =  tick_delta/(time_delta*10);
		BDBG_ASSERT(b_ticks_100us>0);
		BDBG_LOG(("b_ticks_100us %u , clock = %uKHz time = %u ticks = %u time=%u", b_ticks_100us, b_ticks_100us*10, time_delta, tick_delta, tick_delta/b_ticks_100us));
	}
	bprofile_calibrate(table, B_PROFILE_NENTRIES);
	bprofile_start(table, B_PROFILE_NENTRIES);
#endif

	run_idct_test();
#if B_HAS_BPROFILE
	nentries = bprofile_stop();
	bprofile_sys_iface_init(&sys_iface);
	sys_iface.get_name = b_symtable_get_name;
	sys_iface.show_entries = 32;
	bprofile_report_flat(table, nentries, &sys_iface);
	BKNI_Free(table);
#endif
	return;
}

int main(int argc, const char *argv[])
{
    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

    BKNI_Init();
    BDBG_Init();
    run_profile();

    BDBG_Uninit();
    BKNI_Uninit();
    return 0;
}



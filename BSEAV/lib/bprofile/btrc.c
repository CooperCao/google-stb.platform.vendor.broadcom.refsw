/***************************************************************************
 *     Copyright (c) 2006-2011, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Perfomance counter module
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "btrc.h"
#include "bkni.h"

BDBG_MODULE(btrc);

void 
BTRC_P_DoTrace_COUNT(BTRC_Module *module)
{
    BSTD_UNUSED(module);
}

void 
BTRC_P_DoTrace_START(BTRC_Module *module)
{
	struct BTRC_P_Stats *stats = &module->stats[0];
    if(module->b_trc_enable) {
        b_perf_read(&stats->last.perf);
    }
    return;
}

void 
BTRC_P_DoTrace_STOP(BTRC_Module *module)
{
	struct BTRC_P_Stats *stats = &module->stats[0];
	unsigned i;

    if(module->b_trc_enable) {
        bool first;
	    bperf_sample perf;
        b_perf_read(&perf);
        first = stats->count == 0;
        stats->count ++;
        for(i=0;i<BPERF_N_COUNTERS;i++) {
            unsigned val;
            unsigned tmp;

            val = bperf_sample_diff(perf.data[i], stats->last.perf.data[i]);
            tmp = stats->max.perf.data[i];
            stats->max.perf.data[i] = val > tmp ? val : tmp;
            tmp = stats->min.perf.data[i];
            tmp = first ? val : tmp;
            stats->min.perf.data[i] = val < tmp ? val : tmp;
            stats->total.perf.data[i] +=  val;
            stats->total_hi.perf.data[i] += stats->total.perf.data[i]<val ? 1 : 0;
        }
    }
	return;
}

typedef struct b_snprintf_buf {
    size_t left;
    size_t printed;
    char *buf;
} b_snprintf_buf;

static void
b_snprintf_buf_init(b_snprintf_buf *b, char *dest, size_t size)
{
    b->left = size;
    b->buf = dest;
    b->printed = 0;
    return;
}

static void 
b_snprintf_buf_advance(b_snprintf_buf *b, int rc)
{
    BDBG_ASSERT(b->left>=1);
    if(rc>0) {
        b->printed += rc;
        if((unsigned)rc < b->left) {
            b->left -= rc;
            b->buf += rc;
        } else {
            b->buf += b->left-1;
            b->left = 1;
        }
    }
}

void 
BTRC_Module_Report(const BTRC_Module *module)
{
	const bperf_counter_mode *mode = bperf_get_mode();
	unsigned chn = 0;
	const struct BTRC_P_Stats *stats = &module->stats[chn];
	unsigned i;
    char dest[128];
    b_snprintf_buf buf;

	BDBG_ASSERT(module);
	BDBG_ASSERT(mode);
	if (stats->count==0) {
		return;
	}
	BDBG_LOG(("module %s %u samples", module->name, stats->count));
    b_snprintf_buf_init(&buf, dest, sizeof(dest));

	b_snprintf_buf_advance(&buf, BKNI_Snprintf(buf.buf, buf.left, "%8c ", ' '));
	for(i=0;i<BPERF_N_COUNTERS;i++) {
	    b_snprintf_buf_advance(&buf, BKNI_Snprintf(buf.buf, buf.left,"%12s ", mode->counter_names[i]));
	}
    BDBG_LOG(("%s", dest));
    b_snprintf_buf_init(&buf, dest, sizeof(dest));
	b_snprintf_buf_advance(&buf, BKNI_Snprintf(buf.buf, buf.left, "%8s ", "total"));
	for(i=0;i<BPERF_N_COUNTERS;i++) {
        if(stats->total_hi.perf.data[i]==0) {
	        b_snprintf_buf_advance(&buf, BKNI_Snprintf(buf.buf, buf.left, "%12u ", stats->total.perf.data[i]));
        } else {
	        b_snprintf_buf_advance(&buf, BKNI_Snprintf(buf.buf, buf.left, "%12s ",  "-=*OVERFLOW*=-"));
        }
	}
    BDBG_LOG(("%s", dest));
    b_snprintf_buf_init(&buf, dest, sizeof(dest));
	b_snprintf_buf_advance(&buf, BKNI_Snprintf(buf.buf, buf.left, "%8s ", "average"));
	for(i=0;i<BPERF_N_COUNTERS;i++) {
	    b_snprintf_buf_advance(&buf, BKNI_Snprintf(buf.buf, buf.left, "%12u ", (unsigned)(((((uint64_t)stats->total_hi.perf.data[i])<<((sizeof(unsigned))*8)) + stats->total.perf.data[i])/stats->count)));
	}
    BDBG_LOG(("%s", dest));
    b_snprintf_buf_init(&buf, dest, sizeof(dest));
	b_snprintf_buf_advance(&buf, BKNI_Snprintf(buf.buf, buf.left, "%8s ", "max"));
	for(i=0;i<BPERF_N_COUNTERS;i++) {
	    b_snprintf_buf_advance(&buf, BKNI_Snprintf(buf.buf, buf.left, "%12u ", stats->max.perf.data[i]));
	}
    BDBG_LOG(("%s", dest));
    b_snprintf_buf_init(&buf, dest, sizeof(dest));
	b_snprintf_buf_advance(&buf, BKNI_Snprintf(buf.buf, buf.left, "%8s ", "min"));
	for(i=0;i<BPERF_N_COUNTERS;i++) {
	    b_snprintf_buf_advance(&buf, BKNI_Snprintf(buf.buf, buf.left, "%12u ", stats->min.perf.data[i]));
	}
    BDBG_LOG(("%s", dest));
	return;
}



void 
BTRC_Module_Register(BTRC_Module *module, BTRC_ModuleList *list)
{
	BTRC_Module *next;

	/* check if there is a duplicated */
    for(next= BLST_S_FIRST(list); next ; next = BLST_S_NEXT(next, link)) {
		if (next==module) {
			break;
		}
	}
	if (next==NULL) {
		/* no module was found add a new one */
    	BLST_S_INSERT_HEAD(list, module, link);
	}
	return;
}

void 
BTRC_Module_Reset(BTRC_Module *module)
{
	unsigned chn = 0;
	struct BTRC_P_Stats *stats = &module->stats[chn];
    unsigned i;

	BDBG_ASSERT(module);
	stats->count = 0;
    for(i=0;i<BPERF_N_COUNTERS;i++) {
        stats->max.perf.data[i] = 0;
        stats->min.perf.data[i] = 0;
        stats->total.perf.data[i] = 0;
        stats->total_hi.perf.data[i] = 0;
    }
	return;
}

void 
BTRC_List_Init(BTRC_ModuleList *list)
{
	BDBG_ASSERT(list);
    BLST_S_INIT(list);
	return;
}

void 
BTRC_List_Report(BTRC_ModuleList *list)
{
	const BTRC_Module *module;

	/* check if there is a duplicated */
    for(module=BLST_S_FIRST(list); module ; module = BLST_S_NEXT(module, link)) {
		BTRC_Module_Report(module);
	}
}


void 
BTRC_Module_Enable(BTRC_Module *module, bool enable)
{
	BDBG_ASSERT(module);
	module->b_trc_enable = enable;
	return;
}


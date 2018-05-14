/***************************************************************************
 * Copyright (C) 2007-20182xxx Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 * BMedia library, test bench
 *******************************************************************************/

#include "bstd.h"
#include "bmpeg2ts_parser.h"
#include "bmpeg2pes_parser.h"
#include "bmpeg1_parser.h"
#include "bfile_buffer.h"
#include "bfile_stdio.h"
#include "bkni.h"
#include <stdio.h>

BDBG_MODULE(bmedia_benchmark);

typedef struct b_test_pid {
	bmpeg2ts_parser_pid ts_pid; 
	bmpeg2pes_parser pes;
	unsigned pcr_count;
	unsigned ts_bytes;
	unsigned pes_bytes;
	unsigned pts_count;
} b_test_pid;

static bmpeg2ts_parser_action 
b_test_ts_payload(bmpeg2ts_parser_pid *pid, unsigned flags, batom_accum_t src, batom_cursor *cursor, size_t len)
{
	b_test_pid *test=(b_test_pid *)pid;

	if(flags&BMPEG2TS_PCR_FLAG) {
		test->pcr_count++;
	}
	test->ts_bytes += len;
#if 1
	return bmpeg2pes_parser_feed(&test->pes, flags, src, cursor, len);
#else
	BSTD_UNUSED(src);
	BSTD_UNUSED(cursor);
	return bmpeg2ts_parser_action_skip;
#endif
}

static void 
b_test_pes_packet(void *packet_cnxt, batom_accum_t src, batom_cursor *payload, size_t len, const bmpeg2pes_atom_info *info)
{
	b_test_pid *test=(b_test_pid *)packet_cnxt;

	BSTD_UNUSED(src);
	test->pes_bytes += len;
	if(info->flags&BMPEG2PES_PTS_VALID) {
		test->pts_count++;
	}
	batom_cursor_skip(payload, len);
	return;
}

static void 
run_transport_test(const char *fname)
{
	batom_factory_t factory;
	bfile_buffer_t buffer;
	bfile_buffer_cfg buffer_cfg;
	b_test_pid test_pid;
	const size_t block_size=4*BIO_BLOCK_SIZE;
	const size_t buf_size=128*block_size;
	void *buf;
	FILE *fin;
	bfile_io_read_t  fd;
	off_t off;
    off_t bytes_parsed;
	unsigned i;
	batom_pipe_t pipe;
	bmpeg2ts_parser_t parser;
	bmpeg2ts_parser_cfg parser_cfg;
	const char *file=fname;
	uint16_t pid=0x21;
	int rc;

	/* BDBG_SetModuleLevel("bmpeg2ts_parser", BDBG_eMsg);  */

	factory = batom_factory_create(bkni_alloc, 16);
	BDBG_ASSERT(factory);
	fin = fopen(file,"rb");
	BDBG_ASSERT(fin);
	fd = bfile_stdio_read_attach(fin);
	BDBG_ASSERT(fd);
	pipe = batom_pipe_create(factory);
	BDBG_ASSERT(pipe);


	bmpeg2ts_parser_default_cfg(&parser_cfg);
	parser = bmpeg2ts_parser_create(factory, &parser_cfg);

	test_pid.pcr_count = 0;
	test_pid.ts_bytes = 0;
	test_pid.pts_count = 0;
	test_pid.pes_bytes = 0;
	bmpeg2ts_parser_pid_init(&test_pid.ts_pid, pid);

	buf = BKNI_Malloc(buf_size);
	BDBG_ASSERT(buf);

	bfile_buffer_default_cfg(&buffer_cfg);
	buffer_cfg.buf_len = buf_size;
	buffer_cfg.buf = buf;
	buffer_cfg.fd = fd;
	buffer_cfg.nsegs = buf_size/block_size;
	buffer = bfile_buffer_create(factory, &buffer_cfg);
	BDBG_ASSERT(buffer);

	rc = bmpeg2pes_parser_init(factory, &test_pid.pes, BMPEG2PES_ID_ANY);
	BDBG_ASSERT(rc==0);
	test_pid.pes.packet_cnxt = &test_pid;
	test_pid.pes.packet = b_test_pes_packet;

    for(bytes_parsed=0,i=0;i<1;i++) {
		test_pid.ts_pid.payload = b_test_ts_payload;
		bmpeg2ts_parser_add_pid(parser, &test_pid.ts_pid);
		for(off=0;/* off<1024*1024*1024 */;) {
			batom_t atom;
			bfile_buffer_result result;
			size_t atom_len;
			size_t feed_len;

			atom = bfile_buffer_read(buffer, off, block_size, &result);
            if(!atom) {
                break;
            }
			atom_len = batom_len(atom);
			off += atom_len;
			batom_pipe_push(pipe, atom);
			feed_len = bmpeg2ts_parser_feed(parser, pipe);
            if(feed_len!=atom_len) {
                BDBG_ERR(("bmpeg2ts_parser_feed: %u %u %lld", (unsigned)feed_len, (unsigned)block_size, (long long)off));
            }
			BDBG_ASSERT(feed_len==atom_len);
		}
        bytes_parsed += off;
        batom_pipe_flush(pipe);
		bmpeg2ts_parser_reset(parser);
		bmpeg2pes_parser_reset(&test_pid.pes);
	}
	bmpeg2ts_parser_destroy(parser);
	bmpeg2pes_parser_shutdown(&test_pid.pes);
	bfile_buffer_destroy(buffer);
	batom_pipe_destroy(pipe);
	bfile_stdio_read_detach(fd);
	fclose(fin);
	batom_factory_destroy(factory);
	BKNI_Free(buf);
	BKNI_Printf("parsed %u KBytes pcr:%u pts:%u ts_bytes:%u pes_bytes:%u KBytes\n", (unsigned)(bytes_parsed/1024), test_pid.pcr_count, test_pid.pts_count, test_pid.ts_bytes/1024, test_pid.pes_bytes/1024);
	return;
}

#if B_HAS_BPROFILE
#include "bprofile.h"
#include "bprofile_tick.h"
#include "bperf_counter.h"
#include "bsymtable.h"
#include <string.h>

const bsymbol_table *map;

static const char *
b_symtable_get_name(uint32_t addr, char *buf, size_t buf_len)
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

#define B_PROFILE_NENTRIES	(1024*1024)
void 
run_test(const char *fname) 
{
	bprofile_entry *table = BKNI_Malloc(B_PROFILE_NENTRIES*sizeof(*table));
	int nentries;
	bprofile_sys_iface sys_iface;

	BDBG_ERR(("%#x  %u", table, B_PROFILE_NENTRIES*sizeof(*table)));

	b_symtable_init();

	bprofile_calibrate(table, B_PROFILE_NENTRIES);
	bprofile_start(table, B_PROFILE_NENTRIES);
	run_transport_test(fname);
	nentries = bprofile_stop();
	bprofile_sys_iface_init(&sys_iface);
	sys_iface.get_name = b_symtable_get_name;
	sys_iface.show_entries = 32;
	bprofile_report_flat(table, nentries, &sys_iface);
	BKNI_Free(table);
	return;
}
#else
void 
run_test(const char *fname)
{
    BDBG_WRN(("start"));
	run_transport_test(fname);
    BDBG_WRN(("stop"));
    return;
}
#endif


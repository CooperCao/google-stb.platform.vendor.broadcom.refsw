/*
 * This OSL is used only for unit tests
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 */

#ifndef _utest_osl_h_
#define _utest_osl_h_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <osl_decl.h>
#include <typedefs.h>

#ifdef BCMDBG_ASSERT
	#include <assert.h>
	#define ASSERT(exp)	assert(exp)
#else /* BCMDBG_ASSERT */
	#define ASSERT(exp)     do {} while (0)
#endif /* BCMDBG_ASSERT */

#define MALLOC(o, l) malloc(l)
#define MFREE(o, p, l) free(p)
#define MALLOCZ(osh, size) \
	({void *_ptr; \
	_ptr = MALLOC(osh, size); \
	if (_ptr != NULL) {memset(_ptr, 0, (size));} \
	_ptr; })

/* packet primitives */
#include <hnd_pkt.h>

extern uint32 osl_sysuptime(void);	/* get system up time in milliseconds */
extern uint64 osl_sysuptime_us(void);	/* get system up time in microseconds */
#define OSL_SYSUPTIME()		osl_sysuptime()
#define OSL_SYSUPTIME_US()	osl_sysuptime_us()

#endif	/* _utest_osl_h_ */

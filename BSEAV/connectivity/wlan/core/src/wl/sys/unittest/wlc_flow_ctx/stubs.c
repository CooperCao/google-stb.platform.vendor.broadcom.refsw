/*
 * Stub functions for the TX Flow Classification module
 *
 * $ Copyright Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:.*>>
 *
 * $Id$
 */

#if defined(BCMDBG) || defined(BCMDBG_DUMP)

#include <stdarg.h>
#include <stdio.h>

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>

typedef void wlc_pub_t;	/* unused anyway */
typedef int (*dump_fn_t)(void *ctx, struct bcmstrbuf *b);

#ifdef BCMDBG_ASSERT
#include <assert.h>
#undef ASSERT
#define ASSERT(exp) assert(exp)
#endif

/* ------------- callback functions ---------------------------------
 * These functions are replacements (stubs) of the firmware functions
 * so that the module under test links with no error.
 */

void
bcm_binit(struct bcmstrbuf *b, char *buf, uint size)
{
	b->origsize = b->size = size;
	b->origbuf = b->buf = buf;
}

int
bcm_bprintf(struct bcmstrbuf *b, const char *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = vsnprintf(b->buf, b->size, fmt, ap);

	if ((r == -1) || (r >= (int)b->size)) {
		b->size = 0;
	} else {
		b->size -= r;
		b->buf += r;
	}

	va_end(ap);

	return r;
}

dump_fn_t our_dump_fn = NULL;	/* to be called externally by the test */

int
wlc_dump_add_fns(wlc_pub_t *pub, const char *name, dump_fn_t dump_fn, void *unused,
                 void *dump_fn_arg)
{
	ASSERT(pub != NULL);
	ASSERT(name != NULL);
	ASSERT(dump_fn != NULL);
	UNUSED_PARAMETER(unused);
	UNUSED_PARAMETER(dump_fn_arg);

	our_dump_fn = dump_fn;

	return BCME_OK;
}
#endif /* BCMDBG || BCMDBG_DUMP */

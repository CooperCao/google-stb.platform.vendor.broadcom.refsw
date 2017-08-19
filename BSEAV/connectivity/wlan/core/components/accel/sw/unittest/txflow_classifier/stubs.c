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

#include <stdarg.h>
#include <stdio.h>

#include <typedefs.h>
#include <osl.h>
#include <hnd_pktid.h>
#include <hnd_lbuf.h>
#include <bcmutils.h>

#ifdef BCMDBG_ASSERT
#include <assert.h>
#undef ASSERT
#define ASSERT(exp) assert(exp)
#endif

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
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
#endif /* BCMDBG || BCMDBG_DUMP */

bool
lb_sane(struct lbuf *lb)
{
	return TRUE;	/* for the sake of the unittests, all packets are 'sane' */
}

struct lbuf *
lb_alloc(uint size, enum lbuf_type lbuf_type)
{
	uchar *head;
	struct lbuf *lb;
	uint tot = 0;
	uint lbufsz = LBUFFRAGSZ;
	uint end_off;
	uint16 flags = 0;

	if (lbuf_type == lbuf_basic) {
		lbufsz = LBUFSZ;
	} else if (lbuf_type == lbuf_frag) {
		flags = LBF_TX_FRAG;
	}

	tot = lbufsz + ROUNDUP(size, sizeof(int));

	lb = MALLOCZ(NULL, tot);

	if (lb != NULL) {
		head = (uchar*)((uchar*)lb + lbufsz);
		end_off = tot - lbufsz;
		lb->data = (head + end_off) - ROUNDUP(size, sizeof(int));
		lb->head = head;
		lb->end = head + end_off;
		lb->len = size;
		lb->mem.pktid = hnd_pktid_allocate(lb); /* BCMPKTIDMAP: pktid != 0 */
		ASSERT(lb->mem.pktid != PKT_INVALID_ID);
		lb->mem.refcnt = 1;
		lb->flags = flags;
	}

	return lb;
}

void *
hnd_pkt_alloc(osl_t *osh, uint len, enum lbuf_type type)
{
	UNUSED_PARAMETER(osh);

	return (void *)lb_alloc(len, type);
}

void
lb_free(struct lbuf *lb)
{
	struct lbuf *next;

	while (lb) {
		ASSERT(lb->link == NULL);

		next = lb->next;
		lb->data = (uchar*) 0xdeadbeef;
		MFREE(NULL, lb, LBUFFRAGSZ);
		lb = next;
	}
}

void
hnd_pkt_free(osl_t *osh, void* p, bool send)
{
	UNUSED_PARAMETER(osh);
	UNUSED_PARAMETER(send);

	lb_free((struct lbuf *)p);
}

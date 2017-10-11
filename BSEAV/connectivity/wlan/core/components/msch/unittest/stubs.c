/*
 * Basic unit test for WLC Multiple Channel Scheduler module
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
#include <bcmwifi_channels.h>
#include <wlc_types.h>
#include <wlc_phy_hal.h>

#if defined(BCMDBG) || defined(MSCH_PROFILER)
#include <wlc_pub.h>
#endif

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
wlc_suspend_mac_and_wait(wlc_info_t *wlc)
{
	ASSERT(wlc != NULL);
}

void
wlc_enable_mac(wlc_info_t *wlc)
{
	ASSERT(wlc != NULL);
}

int
bcm_cmp_bytes(const uchar *arg1, const uchar *arg2, uint8 nbytes)
{
	/* duplicate of the actual, to avoid linking all of bcmutils.c */

	int i;

	for (i = nbytes - 1; i >= 0; i--) {
		if (arg1[i] != arg2[i])
			return (arg1[i] - arg2[i]);
	}
	return 0;
}

/* calculate a * b + c */
void
bcm_uint64_multiple_add(uint32* r_high, uint32* r_low, uint32 a, uint32 b, uint32 c)
{
#define FORMALIZE(var) {cc += (var & 0x80000000) ? 1 : 0; var &= 0x7fffffff;}
	uint32 r1, r0;
	uint32 a1, a0, b1, b0, t, cc = 0;

	a1 = a >> 16;
	a0 = a & 0xffff;
	b1 = b >> 16;
	b0 = b & 0xffff;

	r0 = a0 * b0;
	FORMALIZE(r0);

	t = (a1 * b0) << 16;
	FORMALIZE(t);

	r0 += t;
	FORMALIZE(r0);

	t = (a0 * b1) << 16;
	FORMALIZE(t);

	r0 += t;
	FORMALIZE(r0);

	FORMALIZE(c);

	r0 += c;
	FORMALIZE(r0);

	r0 |= (cc % 2) ? 0x80000000 : 0;
	r1 = a1 * b1 + ((a1 * b0) >> 16) + ((b1 * a0) >> 16) + (cc / 2);

	*r_high = r1;
	*r_low = r0;
}

/* calculate a / b */
void
bcm_uint64_divide(uint32* r, uint32 a_high, uint32 a_low, uint32 b)
{
	uint32 a1 = a_high, a0 = a_low, r0 = 0;

	if (b < 2)
		return;

	while (a1 != 0) {
		r0 += (0xffffffff / b) * a1;
		bcm_uint64_multiple_add(&a1, &a0, ((0xffffffff % b) + 1) % b, a1, a0);
	}

	r0 += a0 / b;
	*r = r0;
}

/* calculate uint64 a / b
 * keeping bcm_uint64_divide (returns uint32 value) since many module
 * uses it. Added new func bcm_unit64_div to take care of uint64 quotient
 */
uint64
bcm_uint64_div(uint32 a_high, uint32 a_low, uint32 b)
{
	uint32 a1 = a_high, a0 = a_low;
	uint64 r0 = 0;

	if (!b) {
		ASSERT(0);
		return 0;
	}

	if (b == 1) {
		return (((uint64)a1 << 32) | a0);
	}
	while (a1 != 0) {
		r0 += (uint64)(0xffffffff / b) * a1;
		bcm_uint64_multiple_add(&a1, &a0, ((0xffffffff % b) + 1) % b, a1, a0);
	}

	r0 += a0 / b;
	return r0;
}

#ifdef BCMDBG
int
bcm_bprintf(struct bcmstrbuf *b, const char *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = printf(fmt, ap);
	va_end(ap);

	return r;
}

int
wlc_dump_register(wlc_pub_t *pub, const char *name, dump_fn_t dump_fn, void *dump_fn_arg)
{
	ASSERT(pub != NULL);
	ASSERT(name != NULL);
	ASSERT(dump_fn != NULL);
	UNUSED_PARAMETER(dump_fn_arg);

	return BCME_OK;
}
#endif /* */

uint8
wf_chspec_ctlchan(chanspec_t chspec)
{
	/* cut-down version of the actual, should be sufficient for the purpose of these tests */

	return CHSPEC_CHANNEL(chspec);
}

bool
wf_chspec_coexist(chanspec_t chspec1, chanspec_t chspec2)
{
	bool same_ctl = (wf_chspec_ctlchan(chspec1) == wf_chspec_ctlchan(chspec2));

	if (same_ctl && CHSPEC_IS2G(chspec1)) {
	    if (CHSPEC_IS40(chspec1) && CHSPEC_IS40(chspec2)) {
	        return (CHSPEC_CTL_SB(chspec1) == CHSPEC_CTL_SB(chspec2));
	    }
	}
	return same_ctl;
}

wlc_bsscfg_t *
wlc_bsscfg_find_by_wlcif(wlc_info_t *wlc, wlc_if_t *wlcif)
{
	UNUSED_PARAMETER(wlc);
	UNUSED_PARAMETER(wlcif);

	return NULL;
}

void
wlc_phy_hold_upd(wlc_phy_t *pih, mbool id, bool set)
{
	UNUSED_PARAMETER(pih);
	UNUSED_PARAMETER(id);
	UNUSED_PARAMETER(set);
}

void
wlc_read_tsf(wlc_info_t* wlc, uint32* tsf_l_ptr, uint32* tsf_h_ptr)
{
	ASSERT(wlc != NULL);
	ASSERT(tsf_l_ptr != NULL);
	ASSERT(tsf_h_ptr != NULL);
}

bool
wlc_force_ht(wlc_info_t *wlc, bool force, bool *prev)
{
	ASSERT(wlc != NULL);
	UNUSED_PARAMETER(force);
	UNUSED_PARAMETER(prev);
}

#if (defined(EVENT_LOG_COMPILE) && defined(MSCH_EVENT_LOG)) || \
	defined(MSCH_PROFILER) || !defined(DONGLEBUILD)
char *
wf_chspec_ntoa(chanspec_t chspec, char *buf)
{
	(void) sprintf(buf, "0x%02x", chspec);

	return buf;
}

typedef int (*wlc_iov_disp_fn_t)(void *ctx, uint32 aid,
	void *p, uint plen, void *a, uint alen, uint vsz, void *wlcif);
typedef void (*watchdog_fn_t)(void *handle);
typedef int (*up_fn_t)(void *handle);
typedef int (*down_fn_t)(void *handle);

int
wlc_module_register(void *pub, const void *iovars,
                    const char *name, void *hdl, wlc_iov_disp_fn_t i_fn,
                    watchdog_fn_t w_fn, up_fn_t u_fn, down_fn_t d_fn)
{
	ASSERT(pub != NULL);
	ASSERT(name != NULL);
	UNUSED_PARAMETER(iovars);
	UNUSED_PARAMETER(hdl);
	UNUSED_PARAMETER(i_fn);
	UNUSED_PARAMETER(w_fn);
	UNUSED_PARAMETER(u_fn);
	UNUSED_PARAMETER(d_fn);

	return BCME_OK;
}

int
wlc_module_unregister(void *pub, const char *name, void *hdl)
{
	ASSERT(pub != NULL);
	ASSERT(name != NULL);
	UNUSED_PARAMETER(hdl);

	return BCME_OK;
}
#endif /* (EVENT_LOG_COMPILE && MSCH_EVENT_LOG) || MSCH_PROFILER || !DONGLEBUILD */
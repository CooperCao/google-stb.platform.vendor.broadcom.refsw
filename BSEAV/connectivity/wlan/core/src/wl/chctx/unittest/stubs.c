/*
 * Stub functions for the Channel Context module
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
#include <bcmwifi_channels.h>

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
	ASSERT(name != NULL);
	ASSERT(dump_fn != NULL);

	UNUSED_PARAMETER(unused);
	UNUSED_PARAMETER(dump_fn_arg);

	our_dump_fn = dump_fn;

	return BCME_OK;
}
#endif /* BCMDBG || BCMDBG_DUMP */

uint8
wf_chspec_ctlchan(chanspec_t chspec)
{
	return CHSPEC_CHANNEL(chspec);
}

bool
wf_chspec_coexist(chanspec_t chspec1, chanspec_t chspec2)
{
	bool same_ctl = wf_chspec_ctlchan(chspec1) == wf_chspec_ctlchan(chspec2);

	if (same_ctl &&
	    CHSPEC_IS2G(chspec1)) {
		ASSERT(CHSPEC_IS2G(chspec2));
		if (CHSPEC_IS40(chspec1) && CHSPEC_IS40(chspec2)) {
			return CHSPEC_CTL_SB(chspec1) == CHSPEC_CTL_SB(chspec2);
		}
	}

	return same_ctl;
}

static const char *wf_chspec_bw_str[] =
{
	"5",
	"10",
	"20",
	"40",
	"80",
	"160",
	"80+80",
#ifdef WL11ULB
	"2.5"
#else /* WL11ULB */
	"na"
#endif /* WL11ULB */
};

/* 80MHz channels in 5GHz band */
static const uint8 wf_5g_80m_chans[] =
{42, 58, 106, 122, 138, 155};

#define WF_NUM_5G_80M_CHANS \
	(sizeof(wf_5g_80m_chans)/sizeof(uint8))

bool
wf_chspec_malformed(chanspec_t chanspec)
{
	uint chspec_bw = CHSPEC_BW(chanspec);
	uint chspec_ch = CHSPEC_CHANNEL(chanspec);

	/* must be 2G or 5G band */
	if (CHSPEC_IS2G(chanspec)) {
		/* must be valid bandwidth */
		if (!BW_LE40(chspec_bw)) {
			return TRUE;
		}
	} else if (CHSPEC_IS5G(chanspec)) {
		if (chspec_bw == WL_CHANSPEC_BW_8080) {
			uint ch1_id, ch2_id;

			/* channel IDs in 80+80 must be in range */
			ch1_id = CHSPEC_CHAN1(chanspec);
			ch2_id = CHSPEC_CHAN2(chanspec);
			if (ch1_id >= WF_NUM_5G_80M_CHANS || ch2_id >= WF_NUM_5G_80M_CHANS)
				return TRUE;

		} else if (BW_LE20(chspec_bw) || chspec_bw == WL_CHANSPEC_BW_40 ||
				chspec_bw == WL_CHANSPEC_BW_80 || chspec_bw == WL_CHANSPEC_BW_160) {

			if (chspec_ch > MAXCHANNEL) {
				return TRUE;
			}
		} else {
			/* invalid bandwidth */
			return TRUE;
		}
	} else {
		/* must be 2G or 5G band */
		return TRUE;
	}

	/* side band needs to be consistent with bandwidth */
	if (BW_LE20(chspec_bw)) {
		if (CHSPEC_CTL_SB(chanspec) != WL_CHANSPEC_CTL_SB_LLL)
			return TRUE;
	} else if (chspec_bw == WL_CHANSPEC_BW_40) {
		if (CHSPEC_CTL_SB(chanspec) > WL_CHANSPEC_CTL_SB_LLU)
			return TRUE;
	} else if (chspec_bw == WL_CHANSPEC_BW_80) {
		if (CHSPEC_CTL_SB(chanspec) > WL_CHANSPEC_CTL_SB_LUU)
			return TRUE;
	}
	else if (chspec_bw == WL_CHANSPEC_BW_160 ||
		chspec_bw == WL_CHANSPEC_BW_8080) {
		ASSERT(CHSPEC_CTL_SB(chanspec) <= WL_CHANSPEC_CTL_SB_UUU);
	}
	return FALSE;
}

char *
wf_chspec_ntoa(chanspec_t chspec, char *buf)
{
	const char *band;
	uint ctl_chan;

	if (wf_chspec_malformed(chspec))
		return NULL;

	band = "";

	/* check for non-default band spec */
	if ((CHSPEC_IS2G(chspec) && CHSPEC_CHANNEL(chspec) > CH_MAX_2G_CHANNEL) ||
	    (CHSPEC_IS5G(chspec) && CHSPEC_CHANNEL(chspec) <= CH_MAX_2G_CHANNEL))
		band = (CHSPEC_IS2G(chspec)) ? "2g" : "5g";

	/* ctl channel */
	ctl_chan = wf_chspec_ctlchan(chspec);

	/* bandwidth and ctl sideband */
	if (CHSPEC_IS20(chspec)) {
		snprintf(buf, CHANSPEC_STR_LEN, "%s%d", band, ctl_chan);
	} else if (!CHSPEC_IS8080(chspec)) {
		const char *bw;
		const char *sb = "";

		bw = wf_chspec_bw_str[(chspec & WL_CHANSPEC_BW_MASK) >> WL_CHANSPEC_BW_SHIFT];

#ifdef CHANSPEC_NEW_40MHZ_FORMAT
		/* ctl sideband string if needed for 2g 40MHz */
		if (CHSPEC_IS40(chspec) && CHSPEC_IS2G(chspec)) {
			sb = CHSPEC_SB_UPPER(chspec) ? "u" : "l";
		}

		snprintf(buf, CHANSPEC_STR_LEN, "%s%d/%s%s", band, ctl_chan, bw, sb);
#else
		/* ctl sideband string instead of BW for 40MHz */
		if (CHSPEC_IS40(chspec)) {
			sb = CHSPEC_SB_UPPER(chspec) ? "u" : "l";
			snprintf(buf, CHANSPEC_STR_LEN, "%s%d%s", band, ctl_chan, sb);
		} else {
			snprintf(buf, CHANSPEC_STR_LEN, "%s%d/%s", band, ctl_chan, bw);
		}
#endif /* CHANSPEC_NEW_40MHZ_FORMAT */

	} else {
		/* 80+80 */
		uint chan1 = (chspec & WL_CHANSPEC_CHAN1_MASK) >> WL_CHANSPEC_CHAN1_SHIFT;
		uint chan2 = (chspec & WL_CHANSPEC_CHAN2_MASK) >> WL_CHANSPEC_CHAN2_SHIFT;

		/* convert to channel number */
		chan1 = (chan1 < WF_NUM_5G_80M_CHANS) ? wf_5g_80m_chans[chan1] : 0;
		chan2 = (chan2 < WF_NUM_5G_80M_CHANS) ? wf_5g_80m_chans[chan2] : 0;

		/* Outputs a max of CHANSPEC_STR_LEN chars including '\0'  */
		snprintf(buf, CHANSPEC_STR_LEN, "%d/80+80/%d-%d", ctl_chan, chan1, chan2);
	}

	return (buf);
}

void
bcm_bprhex(struct bcmstrbuf *b, const char *msg, bool newline, const uint8 *buf, int len)
{
	int i;

	if (msg != NULL && msg[0] != '\0')
		bcm_bprintf(b, "%s", msg);
	for (i = 0; i < len; i ++)
		bcm_bprintf(b, "%02X", buf[i]);
	if (newline)
		bcm_bprintf(b, "\n");
}

/* ------------- callback functions ---------------------------------
 * These functions are replacements (stubs) of the firmware functions used by the module
 */
uint
MALLOCED(osl_t *osh)
{
	return 0;
}

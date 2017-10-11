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

/***************************************************************************************************
************* Definitions for module components to be tested with Check tool ***********************
*/

#include <typedefs.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include <osl.h>
#include <wlioctl.h>
#include <wlc_types.h>
#include <wlc_hrt.h>
#include <wlc_msch.h>
#include <wlc_msch_priv.h>
#include <wlc.h>

#include <bcmwifi_channels.h>

#ifdef BCMDBG_ASSERT
#include <assert.h>
#undef ASSERT
#define ASSERT(exp) assert(exp)
#endif

uint wl_msg_level = 0;
#ifdef BCMDBG
uint wl_msg_level2 = 0;
#endif	/* BCMDBG */

#include "test_msch.h"

/***************************************************************************************************
************************************* Start of Test Section ****************************************
*/

#include <check.h> /* Includes Check framework */

/*
 * In order to run unit tests with Check, we must create some test cases,
 * aggregate them into a suite, and run them with a suite runner.
 *
 * The pattern of every unit test is as following
 *
 * START_TEST(name_of_test){
 *
 *     perform tests;
 *	       ...assert results
 * }
 * END_TEST
 *
 * Test Case is a set of at least 1 unit test
 * Test Suite is a collection of Test Cases
 * Check Framework can run multiple Test Suites.
 */

/* ------------- Global Definitoions ------------------------- */


/*
 * Global variables definitions, for setup and teardown function.
 */
static osl_t *osh;
volatile static uint32 gptimer;		/* in us */
volatile static uint32 tsf_timerlow;	/* in us */
static uint32 initial_gptimer;		/* in us */
static uint32 psf_timer;		/* in us */
static int32 realign_offset;		/* in us */
static uint16 callback_counter;
static wlc_info_t our_wlc;
static chanspec_t our_chanspec;
static wlcband_t our_band;
static uint64 sequence_start_time;
static uint32 timeslot_duration;
static uint32 timeslot_interval;
static uint32 expected_next_AW_start;
static chanspec_t *chanspec_list;
static uint8 num_chanspecs;
static wlc_msch_req_handle_t *awdl_hdl;
static wlc_msch_req_handle_t *psf_hdl;
static	wlc_msch_req_handle_t *sfixA_hdl;
static	wlc_msch_req_handle_t *sfixB_hdl;
static	wlc_msch_req_handle_t *sfixC_hdl;
static uint8 expect_PSF_slot = FALSE;
static uint8 allow_send_PSF_outside_AWDL_slot = FALSE;
static uint32 psf_flags;
static uint32 psf_register_time;

typedef volatile struct _d11regs_test {
    uint32  PAD[6];         /* 0x0 - 0x14 */
    uint32  gptimer;        /* 0x18 */
    uint32  PAD[89];        /* 0x20 - 0x17c */
    uint32  tsf_timerlow;       /* 0x180 */
} d11regs_test_t;


typedef enum {
	PSF_RETRY_DISABLED,
	PSF_RETRY_ENABLED,
	PSF_RETRY_PENDING
} psf_register_retry_t;

static psf_register_retry_t psf_register_retry = PSF_RETRY_DISABLED;

static uint32 set_chanspec_duration;	/* in us */

/* ------------- callback functions ---------------------------------
 * These functions are replacements (stubs) of the firmware functions used by the module
 */
/* called by hrt */
void W_REG(osl_t *osh, uint32 *reg, uint value)
{
	UNUSED_PARAMETER(osh);

	switch ((uintptr)reg) {
	case OFFSETOF(d11regs_test_t, gptimer):
		//printf("%s(): setting gptimer to val=%d\n", __FUNCTION__, value);
		initial_gptimer = value;
		gptimer = value;
		break;
	case OFFSETOF(d11regs_test_t, tsf_timerlow):
		//printf("%s(): setting tsf_timerlow to val=%d\n", __FUNCTION__, value);
		tsf_timerlow = value;
		break;
	default:
		ASSERT(0);
	}
}

/* called by hrt */
uint32 R_REG(osl_t *osh, uint32 *reg)
{
	UNUSED_PARAMETER(osh);

	switch ((uintptr)reg) {
	case OFFSETOF(d11regs_test_t, gptimer):
		//printf("%s(): returning %d from gptimer\n", __FUNCTION__, gptimer);
		return gptimer;
	case OFFSETOF(d11regs_test_t, tsf_timerlow):
		//printf("%s(): returning %d from tsf_timerlow\n", __FUNCTION__, tsf_timerlow);
		return tsf_timerlow;
	default:
		ASSERT(0);
	}

	return 0;
}

/* called by msch */
uint32 wlc_read_usec_timer(wlc_info_t* wlc)
{
	/* may as well move along the current time by 1us (in case diffs are used by the caller) */
	++tsf_timerlow;
	if (psf_timer > 1) {
		--psf_timer;
	}
	if (gptimer > 0) {
		--gptimer;
	}
	if (gptimer == 0) {
		gptimer = initial_gptimer;	/* reset back to initial setting */
		wlc_hrt_isr(wlc->hrti);		/* interrupt handler called */
	}

	//printf("%d: %s()\n", tsf_timerlow, __FUNCTION__);
	return tsf_timerlow;
}

uint64 osl_sysuptime_us(void)
{
	return wlc_read_usec_timer(&our_wlc);
}

chanspec_t
phy_utils_get_chanspec(phy_info_t *pi)
{
	UNUSED_PARAMETER(pi);

	return our_chanspec;
}

/* ------------- Startup and Teardown - Fixtures ---------------
 * Setting up objects for each unit test,
 * it may be convenient to add some setup that is constant across all the tests in a test case
 * rather than setting up objects for each unit test.
 * Before each unit test in a test case, the setup() function is run, if defined.
 */
void
setup(void)
{
	our_wlc.pub = (wlc_pub_t*) MALLOCZ(osh, sizeof(*our_wlc.pub));
	our_wlc.pub->corerev = 3;	/* because gptimer is not supported in earlier macrevs */
	our_wlc.clk = TRUE;		/* because hrt checks for clock to be running */
	our_band.phytype = PHY_TYPE_AC; /* because the phy type check is needed */

	our_wlc.hrti = wlc_hrt_attach(&our_wlc);
	our_wlc.msch_info = wlc_msch_attach(&our_wlc);
	our_wlc.band = &our_band;

	gptimer = 0;
	tsf_timerlow = 0;
	initial_gptimer = 0;
	callback_counter = 0;
	psf_flags = MSCH_REQ_FLAGS_PREMT_CURTS;	/* default PSF behaviour */
}

/*
 * Tear down objects for each unit test,
 * it may be convenient to add teardown that is constant across all the tests in a test case
 * rather than tearing down objects for each unit test.
 * After each unit test in a test case, the setup() function is run, if defined.
 * Note: checked teardown() fixture will not run if the unit test fails.
*/
void
teardown(void)
{
	/* cleanup all created memory */
	wlc_msch_detach(our_wlc.msch_info);
	wlc_hrt_detach(our_wlc.hrti);
	MFREE(osh, our_wlc.pub, sizeof(*our_wlc.pub));
}

static char *
cb_type_to_string(uint32 cb_type)
{
#define MAX_STRING_LEN	80
	static char string[MAX_STRING_LEN];

	string[0] = '\0';

	if (cb_type & MSCH_CT_PRE_REQ_START) {
		(void) strncat(string, " PRE_REQ_START", MAX_STRING_LEN - strlen(string));
	}
	if (cb_type & MSCH_CT_REQ_START) {
		(void) strncat(string, " REQ_START", MAX_STRING_LEN - strlen(string));
	}
	if (cb_type & MSCH_CT_PRE_ONCHAN) {
		(void) strncat(string, " PRE_ON_CHAN", MAX_STRING_LEN - strlen(string));
	}
	if (cb_type & MSCH_CT_ON_CHAN) {
		(void) strncat(string, " ON_CHAN", MAX_STRING_LEN - strlen(string));
	}
	if (cb_type & MSCH_CT_SLOT_START) {
		(void) strncat(string, " SLOT_START", MAX_STRING_LEN - strlen(string));
	}
	if (cb_type & MSCH_CT_SLOT_END) {
		(void) strncat(string, " SLOT_END", MAX_STRING_LEN - strlen(string));
	}
	if (cb_type & MSCH_CT_SLOT_SKIP) {
		(void) strncat(string, " SLOT_SKIP", MAX_STRING_LEN - strlen(string));
	}
	if (cb_type & MSCH_CT_OFF_CHAN) {
		(void) strncat(string, " OFF_CHAN", MAX_STRING_LEN - strlen(string));
	}
	if (cb_type & MSCH_CT_OFF_CHAN_DONE) {
		(void) strncat(string, " OFF_CHAN_DONE", MAX_STRING_LEN - strlen(string));
	}
	if (cb_type & MSCH_CT_REQ_END) {
		(void) strncat(string, " REQ_END", MAX_STRING_LEN - strlen(string));
	}
	if (cb_type & MSCH_CT_PARTIAL) {
		(void) strncat(string, " PARTIAL", MAX_STRING_LEN - strlen(string));
	}

	return string;
}

/*
 * The START_TEST/END_TEST pair are macros that setup basic structures to permit testing.
 */

#define MAX_TIMEOUTS_IN_TEST	300
#define AW_TIMESLOT_DURATION	65536	/* in us */
#define AW_TIMESLOT_ON_CHANNEL_CHANGE_DURATION	(AW_TIMESLOT_DURATION - MSCH_ONCHAN_PREPARE)
#define TIMEDIFF_TOLERANCE	(AW_TIMESLOT_DURATION / 1000)	/* in us */

#define AW_TIMESLOT_DURATION_NO_EXT	(AW_TIMESLOT_DURATION / 4)

#define PSF_CHANSPEC	0x1006
#define PSF_DURATION	(16 << 10)


#define SFIX_TIMEDIFF_TOLERANCE 20

static chanspec_t aw_chanspec_list[] = {
	0xe09b, 0xe09b, 0xe09b, 0xe09b, 0xe09b, 0xe09b, 0xe02a, 0xe02a,
	0xe09b, 0xe09b, 0xe09b, 0xe09b, 0xe09b, 0xe09b, 0xe02a, 0xe02a
};

static chanspec_t aw_chanspec_list_no_ext[] = {
	0xe09b, -1, -1, -1, 0xe09b, -1, -1, -1,
	0xe09b, -1, -1, -1, 0xe02a, -1, -1, -1
};

static chanspec_t aw_chanspec_list_with_idle_slots[] = {
	0xe09b, 0xe02a, 0xd024, -1, -1, -1, -1, -1,
	0xe09b, 0xe02a, 0xd024, -1, -1, -1, -1, -1
};

static chanspec_t aw_chanspec_list_change_chan[] = {
	0xe09b, 0xd024, 0xe02a, 0x1001
};

static chanspec_t aw_chanspec_list_same_as_psf[] = {
	0xe09b, PSF_CHANSPEC, PSF_CHANSPEC, 0x1001
};

typedef struct {
	uint32 type;
	uint32 timestamp;
} our_cb_info_t;

static our_cb_info_t prev_aw_cb_info;

static int
PSFcallback(void* handler_ctxt, wlc_msch_cb_info_t *cb_info)
{
	static our_cb_info_t prev_psf_cb_info;

	printf("%d: %s(), chanspec=0x%04x, cb_type =%s\n", tsf_timerlow, __FUNCTION__,
		cb_info->chanspec, cb_type_to_string(cb_info->type));

	expect_PSF_slot = FALSE;

	/* check that we never get the SLOT_SKIP notification */
	ck_assert_uint_eq(cb_info->type & MSCH_CT_SLOT_SKIP, 0);

	/* This slot should always be a full slot.  i.e. not partial */
	ck_assert_uint_eq(cb_info->type & MSCH_CT_PARTIAL, 0);

	if (cb_info->type & MSCH_CT_SLOT_START) {
		/* chanspec must match the list */
		ck_assert_uint_eq(cb_info->chanspec, PSF_CHANSPEC);
	}

	if (cb_info->type & MSCH_CT_SLOT_END) {
		uint32 timediff = tsf_timerlow - prev_psf_cb_info.timestamp;

		/* previous callback must be a SLOT_START */
		ck_assert_uint_eq(prev_psf_cb_info.type & MSCH_CT_SLOT_START, MSCH_CT_SLOT_START);

		printf("%d: %s(), slot period=%dus\n", tsf_timerlow, __FUNCTION__, timediff);

		/* We expect that the timediff should equal PSF_DURATION */
		ck_assert_uint_eq(timediff, PSF_DURATION);
	}

	prev_psf_cb_info.type = cb_info->type;
	prev_psf_cb_info.timestamp = tsf_timerlow;

	if (cb_info->type & MSCH_CT_REQ_END) {
		(void) wlc_msch_timeslot_unregister(our_wlc.msch_info, &psf_hdl);
	}

	return 0;
}

static wlc_msch_req_handle_t *
start_PSF_MSCH(void)
{
	int err;
	wlc_msch_req_param_t params = { 0 };
	wlc_msch_req_handle_t *req_hdl = NULL;
	uint64 start_time = msch_current_time(our_wlc.msch_info);
	uint32 psf_duration_required;
	chanspec_t chanspec = PSF_CHANSPEC;

	/* depending on flag, do not register the PSF if we are currently outside an AWDL slot */
	if (allow_send_PSF_outside_AWDL_slot == FALSE &&
	    (prev_aw_cb_info.type & MSCH_CT_SLOT_START) == 0) {
		printf("%d: wlc_msch_timeslot_register(PSF) skipped, because outside AWDL slot\n",
			tsf_timerlow);

		return NULL;
	}

	/* register timeslot with msched */
	params.duration = PSF_DURATION;	/* us */
	params.interval = 0;	/* This is a non-periodic request */
	params.req_type = MSCH_RT_START_FLEX;
	params.priority = MSCH_RP_SYNC_FRAME;
	params.flags = (prev_aw_cb_info.type & MSCH_CT_SLOT_START) ? psf_flags : 0;
	params.start_time_l = (uint32)start_time;
	params.start_time_h = (uint32)(start_time >> 32);
	err = wlc_msch_timeslot_register(our_wlc.msch_info, &chanspec, 1,
	                                 PSFcallback, NULL, &params, &req_hdl);

	ck_assert_uint_eq(expect_PSF_slot, FALSE);	/* check PSF callback complete */

	/* the register call must fail if we cannot fit in the PSF slot, otherwise pass */
	ck_assert_uint_lt(tsf_timerlow, expected_next_AW_start);

	psf_duration_required = PSF_DURATION + MSCH_ONCHAN_PREPARE;
	if (our_chanspec != PSF_CHANSPEC) {
		psf_duration_required += MSCH_ONCHAN_PREPARE;	/* changing channels */
	}

	if (params.flags & MSCH_REQ_FLAGS_PREMT_IMMEDIATE) {
		psf_duration_required += MSCH_MIN_ONCHAN_TIME;	/* extra time needed */
	}

	if (expected_next_AW_start - tsf_timerlow > psf_duration_required) {
		if (err != BCME_OK) {
			printf("%d: next AW slot time = %d, so PSF duration %d should fit\n",
				tsf_timerlow, expected_next_AW_start, psf_duration_required);
		}
		ck_assert_int_eq(err, BCME_OK);
	} else {
		if (err != BCME_UNSUPPORTED) {
			printf("%d: next AW slot time = %d, so PSF duration %d should not fit\n",
				tsf_timerlow, expected_next_AW_start, psf_duration_required);
		}
		ck_assert_int_eq(err, BCME_UNSUPPORTED);
	}

	if (err == BCME_OK) {
		printf("%d: wlc_msch_timeslot_register(PSF) successful\n", tsf_timerlow);
		expect_PSF_slot = TRUE;	/* indicator for AW callback for an early END_SLOT */
		psf_register_time = tsf_timerlow;
	} else {
		/* ignore failed registers of PSF if PSF_RETRY_DISABLED */
		printf("%d: wlc_msch_timeslot_register(PSF) failed with %d", tsf_timerlow, err);
		if (psf_register_retry != PSF_RETRY_DISABLED) {
			/*
			 * There shouldn't already be a retry pending.
			 * It needs to be retried on the next AW SLOT_START.
			 */
			ASSERT(psf_register_retry == PSF_RETRY_ENABLED);
			psf_register_retry = PSF_RETRY_PENDING;
			printf(".  Will retry on next AWDL SLOT_START.");
		}
		printf("\n");
	}

	return req_hdl;
}

static void
initiate_psf(void)
{
	/*
	 * Will be unregistered in the cb REQ_END.
	 * It's possible, however, that the PSF timer expires before we receive the PSF slot, so
	 * in this case, do not re-register.
	 */
	if (psf_hdl == NULL) {
		psf_hdl = start_PSF_MSCH();
	}
}

static void realign_AWDL_slot(void)
{
	int ret;
	wlc_msch_req_param_t params = { 0 };
	uint64 start_time = sequence_start_time + realign_offset;

	params.start_time_l = (uint32)start_time;
	params.start_time_h = (uint32)(start_time >> 32);

	printf("%d: wlc_msch_timeslot_update(%llu + %d)\n",
		tsf_timerlow, sequence_start_time, realign_offset);
	ret = wlc_msch_timeslot_update(our_wlc.msch_info, awdl_hdl, &params,
	                               MSCH_UPDATE_START_TIME);
	ck_assert_int_eq(ret, BCME_OK);	/* check that start time updated successfully */
}

static int
AWcallback(void* handler_ctxt, wlc_msch_cb_info_t *cb_info)
{
	static uint8 slotnum;
	static uint8 realigned = FALSE;
	static uint32 partial_AW_slot_duration = 0;
	uint64 start_time;
	uint64 end_time;
	uint32 cb_time = tsf_timerlow;
	uint32 timediff = cb_time - prev_aw_cb_info.timestamp;

	++callback_counter;
	printf("%d: %s(), chanspec=0x%04x, cb_type =%s\n", cb_time, __FUNCTION__,
		cb_info->chanspec, cb_type_to_string(cb_info->type));

	if (cb_info->type & MSCH_CT_REQ_START) {
		/* find first valid slot */
		slotnum = 0;
		while (chanspec_list[slotnum] == (chanspec_t)-1) {
			++slotnum;
		}
	}

	if (cb_info->type & MSCH_CT_ON_CHAN) {
		wlc_msch_onchan_info_t *msch_onchan_info =
			(wlc_msch_onchan_info_t *)cb_info->type_specific;

		start_time = ((uint64)msch_onchan_info->start_time_h << 32) |
		             msch_onchan_info->start_time_l;
		end_time = ((uint64)msch_onchan_info->end_time_h << 32) |
		           msch_onchan_info->end_time_l;

//		printf("%d: %s(), timelot_id=%d, end_time - start_time = %dus\n", cb_time,
//		       __FUNCTION__, msch_onchan_info->timeslot_id, (int)(end_time - start_time));
	}

	if (cb_info->type & MSCH_CT_SLOT_SKIP) {
		wlc_msch_skipslot_info_t *msch_skipslot_info =
			(wlc_msch_skipslot_info_t *)cb_info->type_specific;

		start_time = ((uint64)msch_skipslot_info->start_time_h << 32) |
		             msch_skipslot_info->start_time_l;
		end_time = ((uint64)msch_skipslot_info->end_time_h << 32) |
		           msch_skipslot_info->end_time_l;

//		printf("%d: %s(), end_time - start_time = %dus\n", cb_time,
//		       __FUNCTION__, (int)(end_time - start_time));
	}

	/* check that we never get the SLOT_SKIP notification */
	ck_assert_uint_eq(cb_info->type & MSCH_CT_SLOT_SKIP, 0);

	if (cb_info->type & MSCH_CT_SLOT_START) {
		/* chanspec must match the list */
		ck_assert_uint_eq(cb_info->chanspec, chanspec_list[slotnum]);

		/* we should not get an AW SLOT_START if we are expecting a PSF slot */
		ck_assert_uint_eq(expect_PSF_slot, FALSE);

		if (partial_AW_slot_duration != 0) {
			/* this slot should be the remainder of the first partial slot */
			ck_assert_uint_eq(cb_info->type & MSCH_CT_PARTIAL, MSCH_CT_PARTIAL);

			/* it should start exactly after the PSF slot finished */
			ck_assert_uint_eq(expected_next_AW_start, cb_time);

			expected_next_AW_start = cb_time + timeslot_duration -
				partial_AW_slot_duration - (PSF_DURATION + 2 * MSCH_ONCHAN_PREPARE);
		} else {
			/* This slot should be the start of a new slot.  i.e. not partial */
			ck_assert_uint_eq(cb_info->type & MSCH_CT_PARTIAL, 0);

			expected_next_AW_start = cb_time + timeslot_duration;

			/* The sequence period should equal TIMESLOT_INTERVAL */
			if (slotnum == 0) {
				if (cb_time > timeslot_duration) {
					uint32 sequence_timediff = cb_time -
					                           (uint32)sequence_start_time;

					printf("%d: %s(), sequence period=%dus\n", cb_time,
						__FUNCTION__, sequence_timediff);

					if (realign_offset != 0) {
						sequence_timediff -= realign_offset;

						/* double the offset for next sequence */
						if (msch_current_time(our_wlc.msch_info) +
						    ABS(realign_offset) < 0x7fffffff) {
							realign_offset *= 2;
						}
					}

					ck_assert_uint_le(sequence_timediff,
					                  timeslot_interval + TIMEDIFF_TOLERANCE);
					ck_assert_uint_ge(sequence_timediff,
					                  timeslot_interval - TIMEDIFF_TOLERANCE);
				}

				sequence_start_time = cb_time;

				if (realign_offset != 0) {
					realign_AWDL_slot();
					realigned = TRUE;
					expected_next_AW_start += realign_offset;
				}
			}
		}

		prev_aw_cb_info.type = cb_info->type;
		prev_aw_cb_info.timestamp = cb_time;

		if (psf_register_retry == PSF_RETRY_PENDING) {
			initiate_psf();
			psf_register_retry = PSF_RETRY_ENABLED;
		}
	}

	if (cb_info->type & MSCH_CT_SLOT_END) {
		chanspec_t next_chanspec;
		unsigned char i = slotnum;
		uint32 expected_AW_duration;

		/* previous callback must be a SLOT_START */
		ck_assert_uint_eq(prev_aw_cb_info.type & MSCH_CT_SLOT_START, MSCH_CT_SLOT_START);

		/* check that chanspec hasn't changed since SLOT_START */
		ck_assert_uint_eq(cb_info->chanspec, chanspec_list[slotnum]);

		printf("%d: %s(), slot period=%dus\n", cb_time, __FUNCTION__, timediff);

		/* get next valid chanspec in list */
		do {
			if (++i == num_chanspecs) {
				i = 0;
			}
			next_chanspec = chanspec_list[i];
		} while (next_chanspec == (chanspec_t)-1);

		expected_AW_duration = timeslot_duration;

		if (expect_PSF_slot) {
			expected_AW_duration -= PSF_DURATION;
			if (next_chanspec != PSF_CHANSPEC) {
				expected_AW_duration -= MSCH_ONCHAN_PREPARE;
			}
			next_chanspec = PSF_CHANSPEC;
		}

		if (realigned) {
			realigned = FALSE;
			expected_AW_duration += realign_offset;
		}

		if (expect_PSF_slot && (psf_flags & MSCH_REQ_FLAGS_PREMT_IMMEDIATE)) {
			/*
			 * This slot should be partial; the next should be PSF; and the next
			 * should be the rest of the AW slot up until the duration.
			 */
			ck_assert_uint_eq(cb_info->type & MSCH_CT_PARTIAL, MSCH_CT_PARTIAL);

			/* we should get the SLOT_END within 2ms from the PSF register */
			ck_assert_uint_ge(psf_register_time + MS_TO_USEC(2), cb_time);

			partial_AW_slot_duration = timediff;
			expected_next_AW_start = cb_time + PSF_DURATION + 2 * MSCH_ONCHAN_PREPARE;
		} else {
			/* This slot should be complete.  i.e. not partial */
			ck_assert_uint_eq(cb_info->type & MSCH_CT_PARTIAL, 0);

			if (partial_AW_slot_duration != 0) {
				/* there needs to be enough time left for this slot */
				ck_assert_uint_gt(timeslot_duration,
						partial_AW_slot_duration + PSF_DURATION +
						2 * MSCH_ONCHAN_PREPARE);
				expected_AW_duration = timeslot_duration - partial_AW_slot_duration
					- (PSF_DURATION + 2 * MSCH_ONCHAN_PREPARE);
				partial_AW_slot_duration = 0;
			}

			/* check for change of channels, because timediff will be different */
			if (chanspec_list[slotnum] != next_chanspec &&
			    i == (slotnum + 1) % num_chanspecs) {
				expected_AW_duration -= MSCH_ONCHAN_PREPARE;
			}

			/* We expect that the timediff should equal expected_AW_duration */
			ck_assert_uint_le(timediff, expected_AW_duration + TIMEDIFF_TOLERANCE);
			ck_assert_uint_ge(timediff, expected_AW_duration - TIMEDIFF_TOLERANCE);

			/* go to next valid slot */
			do {
				/* chanspec sequence is repeated */
				if (++slotnum >= num_chanspecs) {
					/* reset back to start of sequence */
					slotnum = 0;
				}
				expected_next_AW_start += timeslot_duration;
			} while (chanspec_list[slotnum] == (chanspec_t)-1);
			expected_next_AW_start -= timeslot_duration;	/* only add idle slots */
		}
	}

	prev_aw_cb_info.type = cb_info->type;
	prev_aw_cb_info.timestamp = cb_time;

	return 0;
}

static wlc_msch_req_handle_t *
start_AWDL_MSCH(uint32 duration, uint32 interval, chanspec_t chanspec[], uint8 num)
{
	int err;
	uint8 i = 0;
	wlc_msch_req_param_t params = { 0 };
	wlc_msch_req_handle_t *req_hdl = NULL;
	uint64 start_time = msch_current_time(our_wlc.msch_info) + 15000; // allow some setup time

	set_chanspec_duration = 3000;	/* in us */

	timeslot_duration = duration;
	timeslot_interval = interval;
	chanspec_list = chanspec;
	num_chanspecs = num;

	/* output AWDL MSCH register configuration */
	printf("AWDL: duration = %dus, interval = %dus\n", timeslot_duration, timeslot_interval);
	printf("    : chanspec[] = {");

	while (i < num) {
		printf("0x%04x", chanspec[i++]);
		if (i < num) {
			if ((i % 8) == 0) {
				printf("\n    :               ");
			} else {
				printf(", ");
			}
		}
	}
	printf("} start time = %u\n", (uint32)start_time);

	/* register timeslot with msched */
	params.duration = timeslot_duration;	/* us */
	params.interval = timeslot_interval;	/* us */
	params.req_type = MSCH_RT_BOTH_FIXED;
	params.priority = MSCH_RP_SYNC_FRAME;
	params.flags = MSCH_REQ_FLAGS_CHAN_CONTIGUOUS | MSCH_REQ_FLAGS_PREMTABLE;
	params.start_time_l = (uint32)start_time;
	params.start_time_h = (uint32)(start_time >> 32);

	err = wlc_msch_timeslot_register(our_wlc.msch_info, chanspec_list, num_chanspecs,
	                                 AWcallback, NULL, &params, &req_hdl);
	ck_assert_int_eq(err, BCME_OK);	/* check registered successfully */

	return req_hdl;
}

#define PSF_TIMEOUT	110000	/* 110 ms */

static void
force_instant_timeout(void)
{
	/* check and initiate PSF if timeout occurs */
	if (psf_timer > 0) {
		if (psf_timer <= gptimer) {	/* PSF timer occur before GP timer? */
			tsf_timerlow += psf_timer;
			gptimer -= psf_timer;
			initiate_psf();
			psf_timer = PSF_TIMEOUT;
		}
		psf_timer -= gptimer;
	}

	/* simulate what would happen in hardware (going forward gptimer usecs) */
	tsf_timerlow += gptimer;	/* register increments by duration left */
	//printf("%d: %s()\n", tsf_timerlow, __FUNCTION__);
	gptimer = initial_gptimer;	/* register resets back to initial setting */
	wlc_hrt_isr(our_wlc.hrti);	/* interrupt handler is called (macintstatus & MI_TO) */
	//printf("%d: %s() done\n", tsf_timerlow, __FUNCTION__);
}

static void
go_forward_us(uint32 time_until_complete)
{
	//printf("%d: %s()\n", tsf_timerlow, __FUNCTION__);
	/* loop through all gptimeouts within the given duration */
	while (time_until_complete > 0 && gptimer > 0) {
		if (psf_timer > 0) {
			if (psf_timer <= time_until_complete && psf_timer <= gptimer) {
				/* PSF timeout occurs first */
				tsf_timerlow += psf_timer;
				gptimer -= psf_timer;
				time_until_complete -= psf_timer;
				initiate_psf();
				psf_timer = PSF_TIMEOUT;
			}

			if (gptimer <= time_until_complete) {
				ASSERT(psf_timer > gptimer);
				psf_timer -= gptimer;
			} else {
				ASSERT(psf_timer > time_until_complete);
				psf_timer -= time_until_complete;
			}
		}
		if (gptimer <= time_until_complete) {
			time_until_complete -= gptimer;
			force_instant_timeout();
		} else {
			gptimer -= time_until_complete;
			tsf_timerlow += time_until_complete;
			time_until_complete = 0;
		}
	}
}

void
wlc_set_chanspec(wlc_info_t *wlc, chanspec_t chanspec, int reason)
{
	ASSERT(wlc != NULL);
	printf("%d: %s(), chanspec 0x%04x\n", tsf_timerlow, __FUNCTION__, chanspec);

	if (chanspec != our_chanspec) {
		/* this function will take some time to execute, so simulate that */
		go_forward_us(set_chanspec_duration);

		our_chanspec = chanspec;
	}
}

static void test_AWDL_MSCH_generic(uint32 duration, uint32 interval,
                                   chanspec_t chanspec[], uint8 num)
{
	uint16 loop_counter = MAX_TIMEOUTS_IN_TEST;

	awdl_hdl = start_AWDL_MSCH(duration, interval, chanspec, num);

	/* timeout MAX_TIMEOUTS_IN_TEST times */
	do {
		force_instant_timeout();
	} while (--loop_counter > 0);

	if (awdl_hdl != NULL)
		wlc_msch_timeslot_unregister(our_wlc.msch_info, &awdl_hdl);
}

START_TEST(test_AWDL_MSCH)
{
	printf("\n%s\n", __FUNCTION__);

	test_AWDL_MSCH_generic(AW_TIMESLOT_DURATION,
	                       AW_TIMESLOT_DURATION * ARRAYSIZE(aw_chanspec_list),
	                       aw_chanspec_list, ARRAYSIZE(aw_chanspec_list));

	/* check that the callback function was called the correct number of times */
	ck_assert_int_eq(callback_counter, MAX_TIMEOUTS_IN_TEST * 2.6 - 1);
}
END_TEST

START_TEST(test_AWDL_no_ext_MSCH)
{
	printf("\n%s\n", __FUNCTION__);

	test_AWDL_MSCH_generic(AW_TIMESLOT_DURATION_NO_EXT,
	                       AW_TIMESLOT_DURATION_NO_EXT * ARRAYSIZE(aw_chanspec_list_no_ext),
	                       aw_chanspec_list_no_ext, ARRAYSIZE(aw_chanspec_list_no_ext));

	/* check that the callback function was called the correct number of times */
	ck_assert_int_eq(callback_counter, MAX_TIMEOUTS_IN_TEST * 1.4);
}
END_TEST

START_TEST(test_AWDL_change_chan_MSCH)
{
	printf("\n%s\n", __FUNCTION__);

	test_AWDL_MSCH_generic(AW_TIMESLOT_DURATION,
	                       AW_TIMESLOT_DURATION * ARRAYSIZE(aw_chanspec_list_change_chan),
	                       aw_chanspec_list_change_chan,
	                       ARRAYSIZE(aw_chanspec_list_change_chan));

	/* check that the callback function was called the correct number of times */
	ck_assert_int_eq(callback_counter, MAX_TIMEOUTS_IN_TEST * 2 - 1);
}
END_TEST

static void test_AWDL_PSF_MSCH_generic(uint32 duration, uint32 interval,
                                       chanspec_t chanspec[], uint8 num, psf_register_retry_t retry)
{
	uint16 loop_counter = MAX_TIMEOUTS_IN_TEST;

	awdl_hdl = start_AWDL_MSCH(duration, interval, chanspec, num);

	psf_timer = PSF_TIMEOUT;	/* this should enable the PSF timer */

	psf_register_retry = retry;

	/* timeout MAX_TIMEOUTS_IN_TEST times */
	do {
		force_instant_timeout();
	} while (--loop_counter > 0);

	if (psf_hdl != NULL)
		wlc_msch_timeslot_unregister(our_wlc.msch_info, &psf_hdl);
	if (awdl_hdl != NULL)
		wlc_msch_timeslot_unregister(our_wlc.msch_info, &awdl_hdl);
}

START_TEST(test_AWDL_PSF_MSCH)
{
	printf("\n%s\n", __FUNCTION__);

	allow_send_PSF_outside_AWDL_slot = FALSE;
	test_AWDL_PSF_MSCH_generic(AW_TIMESLOT_DURATION,
	                           AW_TIMESLOT_DURATION * ARRAYSIZE(aw_chanspec_list),
	                           aw_chanspec_list, ARRAYSIZE(aw_chanspec_list),
	                           PSF_RETRY_DISABLED);
}
END_TEST

START_TEST(test_AWDL_PSF_immediate_MSCH)
{
	printf("\n%s\n", __FUNCTION__);

	allow_send_PSF_outside_AWDL_slot = FALSE;
	psf_flags = MSCH_REQ_FLAGS_PREMT_IMMEDIATE;
	test_AWDL_PSF_MSCH_generic(AW_TIMESLOT_DURATION,
	                           AW_TIMESLOT_DURATION * ARRAYSIZE(aw_chanspec_list),
	                           aw_chanspec_list, ARRAYSIZE(aw_chanspec_list),
	                           PSF_RETRY_DISABLED);
}
END_TEST

START_TEST(test_AWDL_with_idle_PSF_MSCH)
{
	printf("\n%s\n", __FUNCTION__);

	allow_send_PSF_outside_AWDL_slot = TRUE;
	test_AWDL_PSF_MSCH_generic(AW_TIMESLOT_DURATION,
	                           AW_TIMESLOT_DURATION *
	                           ARRAYSIZE(aw_chanspec_list_with_idle_slots),
	                           aw_chanspec_list_with_idle_slots,
	                           ARRAYSIZE(aw_chanspec_list_with_idle_slots),
	                           PSF_RETRY_DISABLED);
}
END_TEST

START_TEST(test_AWDL_PSF_outside_AW_slot_MSCH)
{
	printf("\n%s\n", __FUNCTION__);

	allow_send_PSF_outside_AWDL_slot = TRUE;
	test_AWDL_PSF_MSCH_generic(AW_TIMESLOT_DURATION,
	                           AW_TIMESLOT_DURATION * ARRAYSIZE(aw_chanspec_list),
	                           aw_chanspec_list, ARRAYSIZE(aw_chanspec_list),
	                           PSF_RETRY_DISABLED);
}
END_TEST

START_TEST(test_AWDL_PSF_with_same_chan_MSCH)
{
	printf("\n%s\n", __FUNCTION__);

	allow_send_PSF_outside_AWDL_slot = FALSE;
	test_AWDL_PSF_MSCH_generic(AW_TIMESLOT_DURATION,
	                           AW_TIMESLOT_DURATION * ARRAYSIZE(aw_chanspec_list_same_as_psf),
	                           aw_chanspec_list_same_as_psf,
	                           ARRAYSIZE(aw_chanspec_list_same_as_psf),
	                           PSF_RETRY_DISABLED);
}
END_TEST

START_TEST(test_AWDL_PSF_with_retry_MSCH)
{
	printf("\n%s\n", __FUNCTION__);

	allow_send_PSF_outside_AWDL_slot = FALSE;
	test_AWDL_PSF_MSCH_generic(AW_TIMESLOT_DURATION,
	                           AW_TIMESLOT_DURATION * ARRAYSIZE(aw_chanspec_list),
	                           aw_chanspec_list, ARRAYSIZE(aw_chanspec_list),
	                           PSF_RETRY_ENABLED);
}
END_TEST

START_TEST(test_AWDL_change_chan_PSF_with_retry_MSCH)
{
	printf("\n%s\n", __FUNCTION__);

	allow_send_PSF_outside_AWDL_slot = FALSE;
	test_AWDL_PSF_MSCH_generic(AW_TIMESLOT_DURATION,
	                           AW_TIMESLOT_DURATION * ARRAYSIZE(aw_chanspec_list_change_chan),
	                           aw_chanspec_list_change_chan,
	                           ARRAYSIZE(aw_chanspec_list_change_chan),
	                           PSF_RETRY_ENABLED);
}
END_TEST

START_TEST(test_AWDL_realign_pos_PSF_MSCH)
{
	printf("\n%s\n", __FUNCTION__);

	realign_offset = 5000;
	allow_send_PSF_outside_AWDL_slot = FALSE;
	test_AWDL_PSF_MSCH_generic(AW_TIMESLOT_DURATION,
	                           AW_TIMESLOT_DURATION * ARRAYSIZE(aw_chanspec_list),
	                           aw_chanspec_list, ARRAYSIZE(aw_chanspec_list),
	                           PSF_RETRY_DISABLED);
}
END_TEST

START_TEST(test_AWDL_realign_neg_PSF_MSCH)
{
	printf("\n%s\n", __FUNCTION__);

	realign_offset = -5000;
	allow_send_PSF_outside_AWDL_slot = FALSE;
	test_AWDL_PSF_MSCH_generic(AW_TIMESLOT_DURATION,
	                           AW_TIMESLOT_DURATION * ARRAYSIZE(aw_chanspec_list),
	                           aw_chanspec_list, ARRAYSIZE(aw_chanspec_list),
	                           PSF_RETRY_DISABLED);
}
END_TEST

/*----------------- START OF NAN TEST CASES ----------------------*/
#define DW_TIMESLOT_DURATION	(16<<10)	/* in us */
#define DW_INTERVAL	(512<< 10)

typedef int (*sfix_msch_callback)(void* handler_ctxt, wlc_msch_cb_info_t *cb_info);

static int SfixReqAcallback(void* handler_ctxt, wlc_msch_cb_info_t *cb_info);
static int SfixReqBcallback(void* handler_ctxt, wlc_msch_cb_info_t *cb_info);

static wlc_msch_req_handle_t *
start_sfix_req_MSCH(uint32 duration, uint32 interval, chanspec_t chanspec[], uint8 num,
		wlc_msch_req_prio_t priority, sfix_msch_callback func, uint64 time)
{
	wlc_msch_req_param_t params = { 0 };
	wlc_msch_req_handle_t *req_hdl = NULL;
	int err;
	uint64 start_time = msch_current_time(our_wlc.msch_info) + time; // allow some setup time


	chanspec_list = chanspec;
	num_chanspecs = num;

	/* output NAN MSCH register configuration */
	printf("%s: duration = %dus, interval = %dus, chanspec=%d, start time=%u\n",
		__FUNCTION__, duration, interval, chanspec[0], (uint32)start_time);

	/* register timeslot with msched */
	params.duration = duration;	/* us */
	params.interval = interval;	/* us */
	params.req_type = MSCH_RT_BOTH_FIXED;
	params.priority = priority;
	params.start_time_l = (uint32)start_time;
	params.start_time_h = (uint32)(start_time >> 32);

	err = wlc_msch_timeslot_register(our_wlc.msch_info, chanspec_list, num_chanspecs,
	                                 func, NULL, &params, &req_hdl);
	ck_assert_int_eq(err, BCME_OK);

	return req_hdl;
}

static int
SfixReqBcallback(void* handler_ctxt, wlc_msch_cb_info_t *cb_info)
{
	static uint32 prev_timestamp;
	uint64 start_time;
	uint64 end_time;
	uint32 cb_time = tsf_timerlow;

	printf("%d: %s(), chanspec=0x%04x, cb_type =%s\n", tsf_timerlow, __FUNCTION__,
		cb_info->chanspec, cb_type_to_string(cb_info->type));

	if (cb_info->type & MSCH_CT_SLOT_SKIP) {

		wlc_msch_skipslot_info_t *msch_skipslot_info =
			(wlc_msch_skipslot_info_t *)cb_info->type_specific;

		start_time = ((uint64)msch_skipslot_info->start_time_h << 32) |
		             msch_skipslot_info->start_time_l;
		end_time = ((uint64)msch_skipslot_info->end_time_h << 32) |
		           msch_skipslot_info->end_time_l;
		printf("%d: %s(), endtime = %u, starttime=%u, end_time - start_time = %dus\n",
			cb_time, __FUNCTION__, (uint32)end_time, (uint32)start_time,
			(int)(end_time - start_time));
	}

	/* check that we never get the SLOT_SKIP notification */
	ck_assert_uint_eq(cb_info->type & MSCH_CT_SLOT_SKIP, 0);

	if (cb_info->type & MSCH_CT_SLOT_END) {
		uint32 timediff = tsf_timerlow - prev_timestamp;
		uint32 dur = cb_info->req_hdl->req_param->duration;
		printf("%d: %s(), slot period=%dus, expected dur=%dus\n",
				tsf_timerlow, __FUNCTION__, timediff, dur);

		/* Check actual duration with expected duration */
		ck_assert_uint_le(timediff, dur + SFIX_TIMEDIFF_TOLERANCE);
		ck_assert_uint_ge(timediff, dur - SFIX_TIMEDIFF_TOLERANCE);
	}

	prev_timestamp = cb_time;

	if (cb_info->type & MSCH_CT_REQ_END) {
		(void) wlc_msch_timeslot_unregister(our_wlc.msch_info, &sfixB_hdl);
	}
	return 0;
}

static int
SfixReqCcallback(void* handler_ctxt, wlc_msch_cb_info_t *cb_info)
{
	static uint32 prev_timestamp;
	uint64 start_time;
	uint64 end_time;
	uint32 cb_time = tsf_timerlow;

	printf("%d: %s(), chanspec=0x%04x, cb_type =%s\n", tsf_timerlow, __FUNCTION__,
		cb_info->chanspec, cb_type_to_string(cb_info->type));

	if (cb_info->type & MSCH_CT_SLOT_SKIP) {

		wlc_msch_skipslot_info_t *msch_skipslot_info =
			(wlc_msch_skipslot_info_t *)cb_info->type_specific;

		start_time = ((uint64)msch_skipslot_info->start_time_h << 32) |
		             msch_skipslot_info->start_time_l;
		end_time = ((uint64)msch_skipslot_info->end_time_h << 32) |
		           msch_skipslot_info->end_time_l;
		printf("%d: %s(), endtime = %u, starttime=%u, end_time - start_time = %dus\n",
			cb_time, __FUNCTION__, (uint32)end_time, (uint32)start_time,
			(int)(end_time - start_time));
	}

	/* check that we never get the SLOT_SKIP notification */
	ck_assert_uint_eq(cb_info->type & MSCH_CT_SLOT_SKIP, 0);

	if (cb_info->type & MSCH_CT_REQ_END) {
		(void) wlc_msch_timeslot_unregister(our_wlc.msch_info, &sfixC_hdl);
	}
	return 0;
}

static int
SfixSkipcallback(void* handler_ctxt, wlc_msch_cb_info_t *cb_info)
{
	uint64 start_time;
	uint64 end_time;
	uint32 cb_time = tsf_timerlow;
	printf("%d: %s(), chanspec=0x%04x, cb_type =%s\n", tsf_timerlow, __FUNCTION__,
		cb_info->chanspec, cb_type_to_string(cb_info->type));

	if (cb_info->type & MSCH_CT_SLOT_SKIP) {

		wlc_msch_skipslot_info_t *msch_skipslot_info =
			(wlc_msch_skipslot_info_t *)cb_info->type_specific;

		start_time = ((uint64)msch_skipslot_info->start_time_h << 32) |
		             msch_skipslot_info->start_time_l;
		end_time = ((uint64)msch_skipslot_info->end_time_h << 32) |
		           msch_skipslot_info->end_time_l;
		printf("%d: %s(), endtime = %u, starttime=%u, end_time - start_time = %dus\n",
			cb_time, __FUNCTION__, (uint32)end_time, (uint32)start_time,
			(int)(end_time - start_time));
	}

	/* check that we should get the SLOT_SKIP notification */
	ck_assert_uint_ne(cb_info->type & MSCH_CT_SLOT_SKIP, 0);

	if (cb_info->type & MSCH_CT_REQ_END) {
		(void) wlc_msch_timeslot_unregister(our_wlc.msch_info, &sfixC_hdl);
	}
	return 0;
}

static int
SfixReqAcallback(void* handler_ctxt, wlc_msch_cb_info_t *cb_info)
{
	static uint8 slotnum;
	uint64 start_time;
	uint64 end_time;
	uint32 cb_time = tsf_timerlow;
	static uint32 prev_timestamp;

	++callback_counter;
	printf("%d: %s(), chanspec=0x%04x, cb_type =%s, callback_counter=%d\n", cb_time, __FUNCTION__,
		cb_info->chanspec, cb_type_to_string(cb_info->type), callback_counter);

	//printf("SFIXA: next start time = %u, CURTS STARTTIME=%u, CurTS end time=%u\n",
		//cb_info->req_hdl->req_param->start_time_l,
		//(uint32)our_wlc.msch_info->cur_msch_timeslot->pre_start_time,
		//(uint32)our_wlc.msch_info->cur_msch_timeslot->end_time);
	if (cb_info->type & MSCH_CT_REQ_START) {
		/* fall through */
	}

	if (cb_info->type & MSCH_CT_ON_CHAN) {
		wlc_msch_onchan_info_t *msch_onchan_info =
			(wlc_msch_onchan_info_t *)cb_info->type_specific;

		start_time = ((uint64)msch_onchan_info->start_time_h << 32) |
		             msch_onchan_info->start_time_l;
		end_time = ((uint64)msch_onchan_info->end_time_h << 32) |
		           msch_onchan_info->end_time_l;

		printf("%d: %s(), timelot_id=%d, starttime=%u, endtime=%u,"
			"end_time - start_time = %dus\n", cb_time, __FUNCTION__,
			msch_onchan_info->timeslot_id, (uint32)start_time,
			(uint32)end_time, (int)(end_time - start_time));
	}

	if (cb_info->type & MSCH_CT_SLOT_SKIP) {

		wlc_msch_skipslot_info_t *msch_skipslot_info =
			(wlc_msch_skipslot_info_t *)cb_info->type_specific;

		start_time = ((uint64)msch_skipslot_info->start_time_h << 32) |
		             msch_skipslot_info->start_time_l;
		end_time = ((uint64)msch_skipslot_info->end_time_h << 32) |
		           msch_skipslot_info->end_time_l;
		printf("%d: %s(), endtime = %u, starttime=%u, end_time - start_time = %dus\n",
			cb_time, __FUNCTION__, (uint32)end_time, (uint32)start_time,
			(int)(end_time - start_time));
	}

	/* check that we never get the SLOT_SKIP notification */
	ck_assert_uint_eq(cb_info->type & MSCH_CT_SLOT_SKIP, 0);

	if (cb_info->type & MSCH_CT_SLOT_END) {
		/* end time should not be changed */
		uint32 timediff = tsf_timerlow - prev_timestamp;
		uint32 dur = cb_info->req_hdl->req_param->duration;
		printf("%d: %s(), slot period=%dus, prev_timestamp=%u\n",
			tsf_timerlow, __FUNCTION__, timediff, prev_timestamp);
	}
	prev_timestamp = cb_time;

	if (cb_info->type & MSCH_CT_REQ_END) {
		(void) wlc_msch_timeslot_unregister(our_wlc.msch_info, &sfixA_hdl);
	}
	return 0;
}

START_TEST(test_PIGGYBACK_REQInside)
{
	printf("\n%s\n", __FUNCTION__);

	uint16 loop_counter = MAX_TIMEOUTS_IN_TEST;
	chanspec_t chanspec = CH20MHZ_CHSPEC(6);
	/* register fixed request */
	/* test scenario:
	 * Sfix's slot is within DW's slot.
	 * i.e. DW start < sfix start < sfix end < DW end time
	 * Multiple Sfix slots inside DW slot
	 */
	sfixA_hdl = start_sfix_req_MSCH(DW_TIMESLOT_DURATION, DW_INTERVAL,
			&chanspec, 1, MSCH_RP_SYNC_FRAME, SfixReqAcallback, 15000);

	sfixB_hdl = start_sfix_req_MSCH(512, DW_INTERVAL,
			&chanspec, 1, MSCH_RP_DISC_BCN_FRAME, SfixReqBcallback, 21000);

	/* timeout MAX_TIMEOUTS_IN_TEST times */
	do {
		force_instant_timeout();
	} while (--loop_counter > 0);


	if (sfixA_hdl != NULL) {
		wlc_msch_timeslot_unregister(our_wlc.msch_info, &sfixA_hdl);
	}
	if (sfixB_hdl != NULL) {
		wlc_msch_timeslot_unregister(our_wlc.msch_info, &sfixB_hdl);
	}
}
END_TEST

START_TEST(test_PIGGYBACK_MultiREQInside)
{
	printf("\n%s\n", __FUNCTION__);

	uint16 loop_counter = MAX_TIMEOUTS_IN_TEST;
	chanspec_t chanspec = CH20MHZ_CHSPEC(6);
	/* register fixed request */
	/* test scenario:
	 * Multiple Sfix slots (SfixB) inside SfixA slot
	 */
	sfixA_hdl = start_sfix_req_MSCH(50000, 70000,
			&chanspec, 1, MSCH_RP_DISC_BCN_FRAME, SfixReqAcallback, 15000);

	sfixB_hdl = start_sfix_req_MSCH(1000, 70000,
			&chanspec, 1, MSCH_RP_DISC_BCN_FRAME, SfixReqBcallback, 25000);

	sfixC_hdl = start_sfix_req_MSCH(1000, 70000,
			&chanspec, 1, MSCH_RP_DISC_BCN_FRAME, SfixReqCcallback, 36000);

	/* timeout MAX_TIMEOUTS_IN_TEST times */
	do {
		force_instant_timeout();
	} while (--loop_counter > 0);

	if (sfixA_hdl != NULL) {
		wlc_msch_timeslot_unregister(our_wlc.msch_info, &sfixA_hdl);
	}
	if (sfixB_hdl != NULL) {
		wlc_msch_timeslot_unregister(our_wlc.msch_info, &sfixB_hdl);
	}
	if (sfixC_hdl != NULL) {
		wlc_msch_timeslot_unregister(our_wlc.msch_info, &sfixC_hdl);
	}
}
END_TEST

START_TEST(test_PIGGYBACK_REQOverlap)
{
	printf("\n%s\n", __FUNCTION__);

	/* test scenario:
	 * DW's start time < sfix's start time < DW's end time
	 * sfix end time > NAN DW's end time
	 */
	uint16 loop_counter = MAX_TIMEOUTS_IN_TEST;
	chanspec_t chanspec = CH20MHZ_CHSPEC(6);

	sfixA_hdl = start_sfix_req_MSCH(DW_TIMESLOT_DURATION, DW_INTERVAL,
		&chanspec, 1, MSCH_RP_SYNC_FRAME, SfixReqAcallback, 15000);

	sfixB_hdl = start_sfix_req_MSCH(DW_TIMESLOT_DURATION, DW_INTERVAL,
		&chanspec, 1, MSCH_RP_SYNC_FRAME, SfixReqBcallback, 25000);
	do {
		force_instant_timeout();
	} while (--loop_counter > 0);

	if (sfixA_hdl != NULL)
		wlc_msch_timeslot_unregister(our_wlc.msch_info, &sfixA_hdl);

	if (sfixB_hdl != NULL)
		wlc_msch_timeslot_unregister(our_wlc.msch_info, &sfixB_hdl);
}
END_TEST

START_TEST(test_PIGGYBACK_HigherREQOutside)
{
	printf("\n%s\n", __FUNCTION__);

	uint16 loop_counter = MAX_TIMEOUTS_IN_TEST;
	chanspec_t chanspec = CH20MHZ_CHSPEC(6);

	/* test scenario:
	 * Two slots scheduled: req A with priority 1 and req B with higher priority (4) after A
	 * request of piggyback (sfixC) with priority 1 should SKIP
	 */
	sfixA_hdl = start_sfix_req_MSCH(10000, 0,
			&chanspec, 1, MSCH_RP_CONNECTION, SfixReqAcallback, 15000);

	sfixC_hdl = start_sfix_req_MSCH(20000, 0,
			&chanspec, 1, MSCH_RP_CONNECTION, SfixSkipcallback, 22000);

	chanspec = CH20MHZ_CHSPEC(36);
	sfixB_hdl = start_sfix_req_MSCH(10000, 0,
			&chanspec, 1, MSCH_RP_SYNC_FRAME, SfixReqBcallback, 36000);

	/* timeout MAX_TIMEOUTS_IN_TEST times */
	do {
		force_instant_timeout();
	} while (--loop_counter > 0);


	if (sfixA_hdl != NULL) {
		wlc_msch_timeslot_unregister(our_wlc.msch_info, &sfixA_hdl);
	}

	if (sfixB_hdl != NULL) {
		wlc_msch_timeslot_unregister(our_wlc.msch_info, &sfixB_hdl);
	}
	if (sfixC_hdl != NULL) {
		wlc_msch_timeslot_unregister(our_wlc.msch_info, &sfixC_hdl);
	}
}
END_TEST

START_TEST(test_PIGGYBACK_LowerREQinHigherSlot)
{
	printf("\n%s\n", __FUNCTION__);

	uint16 loop_counter = MAX_TIMEOUTS_IN_TEST;
	chanspec_t chanspec = CH20MHZ_CHSPEC(11);

	/* test scenario:
	 * Pre-condition : Req A on different ch (no piggyback at the first)
	 * Two piggyback feasible slots scheduled:
	 * req B with priority 1 and req C with higher priority (4).
	 * When Req C is set(sfixC), req B and C should be piggybacked w/o slotskip.
	 */

	sfixA_hdl = start_sfix_req_MSCH(5000, 0,
			&chanspec, 1, MSCH_RP_CONNECTION, SfixReqAcallback, 10000);

	chanspec = CH20MHZ_CHSPEC(6);
	sfixB_hdl = start_sfix_req_MSCH(10000, 0,
			&chanspec, 1, MSCH_RP_CONNECTION, SfixReqBcallback, 25000);

	sfixC_hdl = start_sfix_req_MSCH(20000, 0,
			&chanspec, 1, MSCH_RP_SYNC_FRAME, SfixReqCcallback, 30000);

	/* timeout MAX_TIMEOUTS_IN_TEST times */
	do {
		force_instant_timeout();
	} while (--loop_counter > 0);


	if (sfixA_hdl != NULL) {
		wlc_msch_timeslot_unregister(our_wlc.msch_info, &sfixA_hdl);
	}

	if (sfixB_hdl != NULL) {
		wlc_msch_timeslot_unregister(our_wlc.msch_info, &sfixB_hdl);
	}
	if (sfixC_hdl != NULL) {
		wlc_msch_timeslot_unregister(our_wlc.msch_info, &sfixC_hdl);
	}
}
END_TEST

START_TEST(test_PIGGYBACK_LowerREQinHigherSlot2)
{
	printf("\n%s\n", __FUNCTION__);

	uint16 loop_counter = MAX_TIMEOUTS_IN_TEST;
	chanspec_t chanspec = CH20MHZ_CHSPEC(11);

	/* test scenario:
	 * negative test item for LowerREQinHigherSlot.
	 * Two slots scheduled: req A with priority 1 and req B with higher priority (4).
	 * Req B is not overlapped with req A duration, it should get call correctly.
	 */
	sfixA_hdl = start_sfix_req_MSCH(10000, 0,
			&chanspec, 1, MSCH_RP_CONNECTION, SfixReqAcallback, 15000);

	sfixB_hdl = start_sfix_req_MSCH(10000, 0,
			&chanspec, 1, MSCH_RP_SYNC_FRAME, SfixReqBcallback, 28000);

	/* timeout MAX_TIMEOUTS_IN_TEST times */
	do {
		force_instant_timeout();
	} while (--loop_counter > 0);


	if (sfixA_hdl != NULL) {
		wlc_msch_timeslot_unregister(our_wlc.msch_info, &sfixA_hdl);
	}

	if (sfixB_hdl != NULL) {
		wlc_msch_timeslot_unregister(our_wlc.msch_info, &sfixB_hdl);
	}
}
END_TEST

/*----------------- END OF NAN TEST CASES ----------------------*/

/*
 * Suite of test cases which check the Multiple Channel Scheduler for each given set of inputs.
 */
Suite *wlc_msch_suite(void)
{
	/* Suite creation */
	Suite *s = suite_create("wlc_msch_run");

	/* Test case creation */
	TCase *tc = tcase_create("Test Case");

#ifdef BCMDBG_ERR
	wl_msg_level |= WL_ERROR_VAL;
#endif /* BCMDBG_ERR */
#ifdef BCMDBG
	wl_msg_level |= WL_INFORM_VAL;
#endif /* BCMDBG */

	tcase_add_checked_fixture(tc, setup, teardown);
	/* Adding unit tests to test case */
	tcase_add_test(tc, test_AWDL_MSCH);
	tcase_add_test(tc, test_AWDL_no_ext_MSCH);
	tcase_add_test(tc, test_AWDL_change_chan_MSCH);
	tcase_add_test(tc, test_AWDL_PSF_MSCH);
	tcase_add_test(tc, test_AWDL_PSF_immediate_MSCH);
#if 0	/* disabled until they get fixed */
	tcase_add_test(tc, test_AWDL_with_idle_PSF_MSCH);	/* Jira:SWWLAN-117407 */
	tcase_add_test(tc, test_AWDL_PSF_outside_AW_slot_MSCH);	/* Jira:SWWLAN-112416 */
	tcase_add_test(tc, test_AWDL_PSF_with_same_chan_MSCH);	/* Jira:SWWLAN-112420 */
#endif
	tcase_add_test(tc, test_AWDL_PSF_with_retry_MSCH);
	tcase_add_test(tc, test_AWDL_change_chan_PSF_with_retry_MSCH);
#if 0	/* disabled until they get fixed */
	tcase_add_test(tc, test_AWDL_realign_pos_PSF_MSCH);	/* Jira:SWWLAN-118771 */
	tcase_add_test(tc, test_AWDL_realign_neg_PSF_MSCH);
#endif

	/* Piggyback test cases */
	tcase_add_test(tc, test_PIGGYBACK_REQInside);
	tcase_add_test(tc, test_PIGGYBACK_MultiREQInside);
	tcase_add_test(tc, test_PIGGYBACK_REQOverlap);
	tcase_add_test(tc, test_PIGGYBACK_HigherREQOutside);
	tcase_add_test(tc, test_PIGGYBACK_LowerREQinHigherSlot);
	tcase_add_test(tc, test_PIGGYBACK_LowerREQinHigherSlot2);
	/* Adding test case to the Suite */
	suite_add_tcase(s, tc);

	return s;
}

/*
 * Basic unit test for WLC Flow Table module
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
#include <wlc_flow_ctx.h>
#include <wlc.h>

#ifdef BCMDBG_ASSERT
#include <assert.h>
#undef ASSERT
#define ASSERT(exp) assert(exp)
#endif

uint wl_msg_level = 0;
#ifdef BCMDBG
uint wl_msg_level2 = 0;
#endif	/* BCMDBG */

#include "test_flow_tbl.h"

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
static wlc_info_t our_wlc;

/* ------------- callback functions ---------------------------------
 * These functions are replacements (stubs) of the firmware functions used by the module
 */
uint
MALLOCED(osl_t *osh)
{
	return 0;
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
	our_wlc.pub->osh = osh;
	our_wlc.flow_tbli = wlc_flow_ctx_attach(&our_wlc);
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
	wlc_flow_ctx_detach(our_wlc.flow_tbli);
	MFREE(osh, our_wlc.pub, sizeof(*our_wlc.pub));
}

#define MAX_FLOWS_AP		MAXSCB
#define NUM_PACKETS_IN_GROUP	10

static void
output_flow_ctx_info(void)
{
	struct bcmstrbuf b;
	char buf[1024];
	extern int (*our_dump_fn)(void *ctx, struct bcmstrbuf *b); /* set by wlc_dump_register */

	bcm_binit(&b, buf, sizeof(buf));
	if (our_dump_fn != NULL) {
		(void) our_dump_fn(our_wlc.flow_tbli, &b);
		(void) puts(buf);
	}
}

START_TEST(test_1)
{
	int ret;
	int i;
	uint16 flowID;
	flow_ctx_t added_flow_entry = { 0, (const scb_t *)0x12, (const wlc_bsscfg_t *)0x34 };
	const flow_ctx_t *flow_entry;

	/* fill flow table plus an extra entry which overflows the array */
	for (i = 0; i < MAX_FLOWS_AP + 1; ++i) {
		added_flow_entry.scb = (const scb_t *)((uintptr)added_flow_entry.scb + 1);
		added_flow_entry.bsscfg = (const wlc_bsscfg_t *)
						((uintptr)added_flow_entry.bsscfg + 1);
		ret = wlc_flow_ctx_add_context(our_wlc.flow_tbli, &added_flow_entry);
		printf("wlc_flow_ctx_add_context(scb=%p, bsscfg=%p): ret=%d\n",
			added_flow_entry.scb, added_flow_entry.bsscfg, ret);
		if (i >= MAX_FLOWS_AP) {
			ck_assert_int_eq(ret, BCME_NORESOURCE);
		} else {
			ck_assert_int_eq(ret, i + 1);
		}
	}

	/* lookup entries (all plus some invalid ones) */
	for (i = 1; i < MAX_FLOWS_AP + 4; ++i) {
		flow_entry = wlc_flow_ctx_lookup(our_wlc.flow_tbli, i);
		if (flow_entry == NULL)
			printf("wlc_flow_ctx_lookup(%d): ret=NULL\n", i);
		else
			printf("wlc_flow_ctx_lookup(%d): scb=%p, bsscfg=%p\n", i,
				flow_entry->scb, flow_entry->bsscfg);

		if (i > MAX_FLOWS_AP) {
			ck_assert_ptr_eq((void *)flow_entry, NULL);
		} else {
			ck_assert_ptr_ne((void *)flow_entry, NULL);
			ck_assert_int_eq((uintptr)flow_entry->scb, 0x12 + i);
			ck_assert_int_eq((uintptr)flow_entry->bsscfg, 0x34 + i);
			ck_assert_uint_eq(flow_entry->ref_cnt, 0);
		}
	}

	/* delete entries (most plus some invalid ones ... leave the first 3) */
	for (i = MAX_FLOWS_AP + 4; i > 3; --i) {
		ret = wlc_flow_ctx_del_context(our_wlc.flow_tbli, i);
		printf("wlc_flow_ctx_del_context(%d): ret=%d\n", i, ret);

		if (i > MAX_FLOWS_AP) {
			ck_assert_int_eq(ret, BCME_NOTFOUND);
		} else {
			ck_assert_int_eq(ret, BCME_OK);
		}
	}

	/* add more flows */
	for (i = 0; i < MAX_FLOWS_AP + 1; ++i) {
		added_flow_entry.scb = (const scb_t *)((uintptr)added_flow_entry.scb + 1);
		added_flow_entry.bsscfg = (const wlc_bsscfg_t *)
						((uintptr)added_flow_entry.bsscfg + 1);
		ret = wlc_flow_ctx_add_context(our_wlc.flow_tbli, &added_flow_entry);
		printf("wlc_flow_ctx_add_context(scb=%p, bsscfg=%p): ret=%d\n",
			added_flow_entry.scb, added_flow_entry.bsscfg, ret);
		if (i > MAX_FLOWS_AP - 4) {
			ck_assert_int_eq(ret, BCME_NORESOURCE);
		} else {
			ck_assert_int_eq(ret, i + 4);
		}
	}

	/* lookup entries (all plus some invalid ones) */
	for (i = 1; i < MAX_FLOWS_AP + 4; ++i) {
		flow_entry = wlc_flow_ctx_lookup(our_wlc.flow_tbli, i);
		if (flow_entry == NULL)
			printf("wlc_flow_ctx_lookup(%d): ret=NULL\n", i);
		else
			printf("wlc_flow_ctx_lookup(%d): scb=%p, bsscfg=%p\n", i,
				flow_entry->scb, flow_entry->bsscfg);

		if (i > MAX_FLOWS_AP) {
			ck_assert_ptr_eq((void *)flow_entry, NULL);
		} else {
			ck_assert_ptr_ne((void *)flow_entry, NULL);

			/* the first 3 entries should be the original adds */
			if (i <= 3) {
				ck_assert_int_eq((uintptr)flow_entry->scb, 0x12 + i);
				ck_assert_int_eq((uintptr)flow_entry->bsscfg, 0x34 + i);
			} else {
				ck_assert_int_eq((uintptr)flow_entry->scb,
				                 0x12 + MAX_FLOWS_AP - 2 + i);
				ck_assert_int_eq((uintptr)flow_entry->bsscfg,
				                 0x34 + MAX_FLOWS_AP - 2 + i);
			}
			ck_assert_uint_eq(flow_entry->ref_cnt, 0);
		}
	}

	/* add packets to an existing flow */
	ret = wlc_flow_ctx_add_packet_refs(our_wlc.flow_tbli, MAX_FLOWS_AP, NUM_PACKETS_IN_GROUP);
	printf("wlc_flow_ctx_add_packet_refs(flowID=%d, num_packets=%d): ret=%d\n",
		MAX_FLOWS_AP, NUM_PACKETS_IN_GROUP, ret);
	ck_assert_int_eq(ret, BCME_OK);

	/* try to delete that flow */
	ret = wlc_flow_ctx_del_context(our_wlc.flow_tbli, MAX_FLOWS_AP);
	printf("wlc_flow_ctx_del_context(%d): ret=%d\n", MAX_FLOWS_AP, ret);
	ck_assert_int_eq(ret, BCME_NOTREADY);

	/* lookup that flow */
	flow_entry = wlc_flow_ctx_lookup(our_wlc.flow_tbli, MAX_FLOWS_AP);
	ck_assert_ptr_ne((void *)flow_entry, NULL);
	ck_assert_uint_eq(flow_entry->ref_cnt, NUM_PACKETS_IN_GROUP);

	output_flow_ctx_info();

	/* remove packets from flow */
	ret = wlc_flow_ctx_remove_packet_refs(our_wlc.flow_tbli, MAX_FLOWS_AP,
	                                      NUM_PACKETS_IN_GROUP);
	printf("wlc_flow_ctx_remove_packet_refs(flowID=%d, num_packets=%d): ret=%d\n",
		MAX_FLOWS_AP, NUM_PACKETS_IN_GROUP, ret);
	ck_assert_int_eq(ret, BCME_OK);

	/* try to delete that flow again */
	ret = wlc_flow_ctx_del_context(our_wlc.flow_tbli, MAX_FLOWS_AP);
	printf("wlc_flow_ctx_del_context(%d): ret=%d\n", MAX_FLOWS_AP, ret);
	ck_assert_int_eq(ret, BCME_OK);

	output_flow_ctx_info();
}
END_TEST

/*
 * Suite of test cases which check the Multiple Channel Scheduler for each given set of inputs.
 */
Suite *wlc_flow_tbl_suite(void)
{
	/* Suite creation */
	Suite *s = suite_create("wlc_flow_ctx_run");

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
	tcase_add_test(tc, test_1);

	/* Adding test case to the Suite */
	suite_add_tcase(s, tc);

	return s;
}

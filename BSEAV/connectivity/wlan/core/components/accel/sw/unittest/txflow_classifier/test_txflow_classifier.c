/*
 * Basic unit test for TX Flow Classification module
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
#include <bcmutils.h>
#include <txflow_classifier.h>
#include <hnd_lbuf.h>
#include <proto/ethernet.h>

#ifdef BCMDBG_ASSERT
#include <assert.h>
#undef ASSERT
#define ASSERT(exp) assert(exp)
#endif

#include "test_txflow_classifier.h"

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
static txflow_class_info_t *txflow_classi;

static uint16 next_flowID = 0;

/* ------------- callback functions ---------------------------------
 * These functions are replacements (stubs) of the firmware functions used by the module
 */
uint
MALLOCED(osl_t *my_osh)
{
	UNUSED_PARAMETER(my_osh);

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
	uint8 i;

	txflow_classi = txflow_classifier_attach(osh);

	for (i = 0; i < 16; ++i) {
		txflow_classifier_set_if_type(txflow_classi, i, i / 4);
	}
}

static void
clear_flows(void)
{
	int ret;

	/* delete classification flow table entries */

	/* check that the next flowID doesn't exist */
	ret = txflow_classifier_delete(txflow_classi, next_flowID + 1);
	ck_assert_int_eq(ret, BCME_NOTFOUND);

	/* delete all the flows that were created */
	while (next_flowID > 0) {
		ret = txflow_classifier_delete(txflow_classi, next_flowID);

		/* note: flowID 2 was deleted in test_1 */
		ck_assert_int_eq(ret, (next_flowID != 2) ? BCME_OK : BCME_NOTFOUND);

		--next_flowID;
	}

	/* check that the table is empty */
	ret = txflow_classifier_delete(txflow_classi, 1);
	ck_assert_int_eq(ret, BCME_NOTFOUND);
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
	clear_flows();

	/* cleanup all created memory */
	txflow_classifier_detach(txflow_classi);
}

static int
classify_packet(const uint8 *txhdr, uint8 ifidx, uint8 priority)
{
	int ret;
	uint16 flowID;
	void *pkt = PKTALLOC(osh, ETHER_HDR_LEN, lbuf_frag);

	ASSERT(pkt != NULL);

	/* fill packet */
	LBP(pkt)->data = (void *)txhdr;
	PKTSETPRIO(pkt, priority);
	PKTSETIFINDEX(osh, pkt, ifidx);
	PKTSETCLINK(pkt, NULL);	/* single packet */

	/* scan for flow that matches packet */
	ret = txflow_classifier_get_flow(txflow_classi,
	                                 PKTDATA(osh, pkt), PKTIFINDEX(osh, pkt), PKTPRIO(pkt),
	                                 &flowID);

	/* create a flow for this packet info if one doesn't already exist */
	if (ret == BCME_NOTFOUND) {
		ret = txflow_classifier_add(txflow_classi, txhdr, ifidx, priority, ++next_flowID);
		ck_assert_int_eq(ret, BCME_OK);
		PKTFREE(osh, pkt, FALSE);

		return BCME_NOTFOUND;
	}

	PKTFREE(osh, pkt, FALSE);

	return (int)flowID;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void
output_classifier_info(void)
{
	struct bcmstrbuf b;
	char buf[1024];

	bcm_binit(&b, buf, sizeof(buf));
	txflow_classifier_dump(txflow_classi, &b);

	(void) puts(buf);
}
#else
#define output_classifier_info()
#endif /* BCMDBG || BCMDBG_DUMP */

static const uint8 txhdr1[ETHER_HDR_LEN] = {
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06,	/* dest */
	0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c,	/* src */
	0x8e, 0x88				/* type */
};

static const uint8 txhdr2[ETHER_HDR_LEN] = {
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06,	/* dest */
	0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0d,	/* src - different from txhdr1 */
	0x8e, 0x88				/* type */
};

static const uint8 txhdr3[ETHER_HDR_LEN] = {
	0x01, 0x02, 0x03, 0x04, 0x05, 0x07,	/* dest - different from txhdr1 */
	0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c,	/* src */
	0x8e, 0x88				/* type */
};

static const uint8 txhdr4[ETHER_HDR_LEN] = {
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06,	/* dest */
	0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c,	/* src */
	0x8e, 0x89				/* type - different from txhdr1 */
};

START_TEST(test_1)
{
	int flowID;

	/*
	 * matching rules:
	 *
	 * if_type 0: match ifidx, priority, DA
	 * if_type 1: match ifidx, priority, SA
	 * if_type 2: match ifidx, priority, DA
	 * if_type 3: match ifidx, priority
	 *
	 * where if_type = ifidx / 4 ... configured in setup()
	 */

	/* see if there's a flow for txhdr1 */
	flowID = classify_packet(txhdr1, 2, 1);	/* txhdr1, ifidx=2 (if_type=0), priority=1 */
	ck_assert_int_eq(flowID, BCME_NOTFOUND); /* shouldn't exist yet */

	/* check again ... should now exist */
	flowID = classify_packet(txhdr1, 2, 1);	/* txhdr1, ifidx=2 (if_type=0), priority=1 */
	ck_assert_int_eq(flowID, 1);		/* should have found match at flowID 1 */

	/* packet with different src address, should still match (i.e checking idx, prio, DA) */
	flowID = classify_packet(txhdr2, 2, 1);	/* changed SA, ifidx=2 (if_type=0), priority=1 */
	ck_assert_int_eq(flowID, 1);		/* should have found match at flowID 1 */

	/* packet with different dest address, should not match (checking DA) */
	flowID = classify_packet(txhdr3, 2, 1);	/* changed DA, ifidx=2 (if_type=0), priority=1 */
	ck_assert_int_eq(flowID, BCME_NOTFOUND); /* should not match */

	/* check again ... should now exist */
	flowID = classify_packet(txhdr3, 2, 1);	/* changed DA, ifidx=2 (if_type=0), priority=1 */
	ck_assert_int_eq(flowID, 2);		/* should have found match at flowID 2 */

	/* packet with different packet type, should still match (i.e checking idx, prio, DA) */
	flowID = classify_packet(txhdr4, 2, 1);	/* changed type, ifidx=2 (if_type=0), priority=1 */
	ck_assert_int_eq(flowID, 1);		/* should have found match at flowID 1 */

	/* packet with different priority, should not match (i.e checking idx, prio, DA) */
	flowID = classify_packet(txhdr1, 2, 2);	/* txhdr1, ifidx=2 (if_type=0), priority=2 */
	ck_assert_int_eq(flowID, BCME_NOTFOUND); /* should not match */

	/* check again ... should now exist */
	flowID = classify_packet(txhdr1, 2, 2);	/* txhdr1, ifidx=2 (if_type=0), priority=2 */
	ck_assert_int_eq(flowID, 3);		/* should have found match at flowID 3 */

	/* packet with different ifidx, should not match (i.e checking idx, prio, DA) */
	flowID = classify_packet(txhdr1, 5, 1);	/* txhdr1, ifidx=5 (if_type=1), priority=1 */
	ck_assert_int_eq(flowID, BCME_NOTFOUND); /* should not match */

	/* check again ... should now exist */
	flowID = classify_packet(txhdr1, 5, 1);	/* txhdr1, ifidx=5 (if_type=1), priority=1 */
	ck_assert_int_eq(flowID, 4);		/* should have found match at flowID 4 */

	/* packet with different dest address, should still match (i.e checking idx, prio, SA) */
	flowID = classify_packet(txhdr3, 5, 1);	/* changed DA, ifidx=5 (if_type=1), priority=1 */
	ck_assert_int_eq(flowID, 4);		/* should have found match at flowID 4 */

	/* delete a flow */
	ck_assert_int_eq(txflow_classifier_delete(txflow_classi, 2), BCME_OK);

	/* packet with different src address, should not match (checking SA) */
	flowID = classify_packet(txhdr2, 5, 1);	/* changed SA, ifidx=5 (if_type=1), priority=1 */
	ck_assert_int_eq(flowID, BCME_NOTFOUND); /* should not match */

	/* check again ... should now exist (at the deleted slot) */
	flowID = classify_packet(txhdr2, 5, 1);	/* changed SA, ifidx=5 (if_type=1), priority=1 */
	ck_assert_int_eq(flowID, 5);		/* should have found match at flowID 5 */

	output_classifier_info();
}
END_TEST

/*
 * Suite of test cases which check the Multiple Channel Scheduler for each given set of inputs.
 */
Suite *txflow_classification_suite(void)
{
	/* Suite creation */
	Suite *s = suite_create("txflow_classification_run");

	/* Test case creation */
	TCase *tc = tcase_create("Test Case");

	tcase_add_checked_fixture(tc, setup, teardown);

	/* Adding unit tests to test case */
	tcase_add_test(tc, test_1);

	/* Adding test case to the Suite */
	suite_add_tcase(s, tc);

	return s;
}

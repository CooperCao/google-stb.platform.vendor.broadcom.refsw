/*
 * Basic unit test for bcm_notif module
 *
 * $ Copyright Broadcom $
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:.*>>
 *
 * $Id: test_bcm_notif.c xxxxxx 2014-10-30 06:00:44Z Dima $
 */


/***************************************************************************************************
************* Definitions for module components to be tested with Check  tool **********************
*/

#include <typedefs.h>
#include <bcm_notif_priv.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <bcmwifi_rates.h>
#include <wlc_ppr.h>


/***************************************************************************************************
************************************* Start of Test Section ****************************************
*/

#include <check.h> /* Includes Check framework */

/*
 * In order to run unit tests with Check, we must create some test cases,
 * aggregate them into a suite, and run them with a suite runner.

 * The pattern of every unit test is as following

 * START_TEST(name_of_test){
 *
 *     perform tests;
 *	       ...assert results
 * }
 * END_TEST

 * Test Case is a set of at least 1 unit test
 * Test Suite is a collection of Test Cases
 * Check Framework can run multiple Test Suites.
 * More details will be on Twiki
 */

/* ------------- Global Definitoions ------------------------- */

struct osl_info {
	uint pad;
};

static struct osl_info dummy_osh;
static osl_t *osh = &dummy_osh;

static bcm_mpm_mgr_h mpm_mgr;

/*
 * Global variables definitions, for setup and teardown function.
 */


/* ------------- Startup and Teardown - Fixtures ---------------
 * Setting up objects for each unit test,
 * it may be convenient to add some setup that is constant across all the tests in a test case
 * rather than setting up objects for each unit test.
 * Before each unit test in a test case, the setup() function is run, if defined.
 */
void setup(void)
{
	int ret = bcm_mpm_init(osh, 5, &mpm_mgr);

	ck_assert_int_eq(ret, BCME_OK);
}

/*
 * Tear down objects for each unit test,
 * it may be convenient to add teardown that is constant across all the tests in a test case
 * rather than tearing down objects for each unit test.
 * After each unit test in a test case, the setup() function is run, if defined.
 * Note: checked teardown() fixture will not run if the unit test fails.
*/
void teardown(void)
{
	int ret = bcm_mpm_deinit(&mpm_mgr);

	ck_assert_int_eq(ret, BCME_OK);
}

/*
 * The START_TEST/END_TEST pair are macros that setup basic structures to permit testing.
 */


START_TEST(test_bcm_notif_attach_max_notif_servers_eq1)
{
	// Create pointer
		bcm_notif_module_t *notif_module;
		bcm_notif_h hdlp;
		wl_tx_bw_t bw;
		ppr_t* pprptr;

		bw = WL_TX_BW_20;
		pprptr = ppr_create(osh, bw);

		notif_module = bcm_notif_attach(osh, mpm_mgr, 1, 1);

		ck_assert_ptr_eq(notif_module, NULL);
}
END_TEST


START_TEST(test_bcm_notif_attach_max_notif_servers_eq5)
{
	// Create pointer
		bcm_notif_module_t *notif_module;
		bcm_notif_h hdlp;
		wl_tx_bw_t bw;
		ppr_t* pprptr;

		bw = WL_TX_BW_20;
		pprptr = ppr_create(osh, bw);

		notif_module = bcm_notif_attach(osh, mpm_mgr, 0, 5);

		ck_assert_ptr_eq(notif_module, NULL);
}
END_TEST


START_TEST(test_bcm_notif_attach_max_notif_servers_eq0)
{
	// Create pointer
		bcm_notif_module_t *notif_module;
		bcm_notif_h hdlp;
		wl_tx_bw_t bw;
		ppr_t* pprptr;

		bw = WL_TX_BW_20;
		pprptr = ppr_create(osh, bw);

		notif_module = bcm_notif_attach(osh, mpm_mgr, 1, 0);

		ck_assert_ptr_eq(notif_module, NULL);
}
END_TEST


/*
 * Suite of test cases which Asserts the size routine for user alloc/dealloc
 * for bw20, bw40 and bw80 sizes.
 */

Suite * bcm_notif(void)
{
	// Suit creation
	Suite *s = suite_create("bcm_notif_attach");
	// Test case creation
	TCase *tc_size = tcase_create("Test Case - SIZE");
	// Adding unit tests to test case.

	tcase_add_test(tc_size, test_bcm_notif_attach_max_notif_servers_eq1);
	tcase_add_test(tc_size, test_bcm_notif_attach_max_notif_servers_eq5);
	tcase_add_test(tc_size, test_bcm_notif_attach_max_notif_servers_eq0);

	// Adding 'tc_ser_size' test case to a Suite
	suite_add_tcase(s, tc_size);

	return s;
}


/*
 * Main flow:
 * 1. Define SRunner object which will aggregate all suites.
 * 2. Adding all suites to SRunner which enables consecutive suite(s) running.
 * 3. Running all suites contained in SRunner.
 */

int main(void)
{
	int number_failed; /* Count number of failed tests */
	//Adding suit to SRunner.
	SRunner *sr = srunner_create(bcm_notif());

	/* Prints the summary, and one message per test (passed or failed) */
	srunner_run_all(sr, CK_VERBOSE);
	number_failed = srunner_ntests_failed(sr); /* count all the failed tests */
	srunner_free(sr);

	return (number_failed == 0) ? EXIT_SUCCESS: EXIT_FAILURE;
}

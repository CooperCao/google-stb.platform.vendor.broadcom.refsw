/*
 * Basic unit test for bcm_mpool.c module
 *
 * $ Copyright Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:.*>>
 *
 * $Id: test_bcm_mpool.c xxxxxx 2014-02-01 06:00:44Z Dima $
 */


/***************************************************************************************************
************* Definitions for module components to be tested with Check  tool **********************
*/

#include <osl.h>
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

/* dummy decl */
struct osl_info {
	uint pad;
};

static struct osl_info dummy_osh;
static osl_t *osh = &dummy_osh;

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

}

/*
 * The START_TEST/END_TEST pair are macros that setup basic structures to permit testing.
 */

START_TEST(test_bcm_mpool_init_BCME_OK_int5)
{
		wl_tx_bw_t bw;
		ppr_t* pprptr;
		bcm_mpm_mgr_h mpm_mgr;
		int i;

		bw = WL_TX_BW_20;
		pprptr = ppr_create(osh, bw);

		mpm_mgr = (bcm_mpm_mgr_h) MALLOC(osh, (sizeof(mpm_mgr)));

		i = bcm_mpm_init(osh, 5, &mpm_mgr);
		ck_assert_int_eq(i, BCME_OK);
}
END_TEST


START_TEST(test_bcm_mpool_init_BCME_OK_int2)
{
		wl_tx_bw_t bw;
		ppr_t* pprptr;
		bcm_mpm_mgr_h mpm_mgr;
		int i;

		bw = WL_TX_BW_40;
		pprptr = ppr_create(osh, bw);

		mpm_mgr = (bcm_mpm_mgr_h) MALLOC(osh, (sizeof(mpm_mgr)));

		i = bcm_mpm_init(osh, 2, &mpm_mgr);
		ck_assert_int_eq(i, BCME_OK);


}
END_TEST


START_TEST(test_bcm_mpool_init_BCME_BADARG)
{
		wl_tx_bw_t bw;
		ppr_t* pprptr;
		bcm_mpm_mgr_h mpm_mgr;
		int i;

		bw = WL_TX_BW_20;
		pprptr = ppr_create(osh, bw);


		mpm_mgr = (bcm_mpm_mgr_h) MALLOC(osh, (sizeof(mpm_mgr)));

		i = bcm_mpm_init(NULL, 5, &mpm_mgr);
		ck_assert_int_eq(i, BCME_BADARG);

		i = bcm_mpm_init(osh, -1, &mpm_mgr);
		ck_assert_int_eq(i, BCME_BADARG);

		i = bcm_mpm_init(osh, 5, NULL);
		ck_assert_int_eq(i, BCME_BADARG);
}
END_TEST


/*
 * Suite of test cases which Asserts the BCME_OK/BCME_BADARG
 * for bw20, bw40 and bw80 sizes.
 */

Suite * bcm_mpool(void)
{
	// Suit creation
	Suite *s = suite_create("bcm_mpool_init");
	// Test case creation
	TCase *tc_size = tcase_create("Test Case - SIZE");

	// Adding unit tests to test case.
	tcase_add_test(tc_size, test_bcm_mpool_init_BCME_OK_int5);
	tcase_add_test(tc_size, test_bcm_mpool_init_BCME_OK_int2);
	tcase_add_test(tc_size, test_bcm_mpool_init_BCME_BADARG);
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

	SRunner *sr = srunner_create(bcm_mpool());	/* Adding suit to SRunner */

	/* Print the summary, and one message per test (passed or failed) */
	srunner_run_all(sr, CK_VERBOSE);
	number_failed = srunner_ntests_failed(sr); /* count all the failed tests */
	srunner_free(sr);

	return (number_failed == 0) ? EXIT_SUCCESS: EXIT_FAILURE;
}

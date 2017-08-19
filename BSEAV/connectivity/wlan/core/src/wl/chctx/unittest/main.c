/*
 * Basic unit test for Channel Context module
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "test_chctx_reg.h"

/***************************************************************************************************
*********************************** Start of Test Section ******************************************
*/

#include <check.h> /* Includes Check framework */

/*
 * Main flow:
 * 1. Define SRunner object which will aggregate all suites.
 * 2. Adding all suites to SRunner which enables consecutive suite(s) running.
 * 3. Running all suites contained in SRunner.
 */

int main(void)
{
	int number_failed;	// Count number of failed tests

	/* Add suite to SRunner */
	SRunner *sr = srunner_create(wlc_chctx_reg_suite());

	/* Add more suites to SRunner */

	/* Prints the summary one message per test (passed or failed) */
	srunner_run_all(sr, CK_VERBOSE);

	/* count all the failed tests */
	number_failed = srunner_ntests_failed(sr);

	/* cleanup */
	srunner_free(sr);

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*
 * main test for wlc_dump_reg module
 *
 * $Copyright (C) 2014, Broadcom Corporation$
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id:$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <test_wlc_dump_reg.h>

/*	*********************
 *	Start of Test Section
 *	*********************
 */

#include <check.h> /* Includes Check framework */

int main(void)
{
	int number_failed; /* Count number of failed tests */

	/* Adding suit to SRunner. */
	SRunner *sr = srunner_create(wlcdumpreg_suite());

	/* Prints the summary one message per test (passed or failed) */
	srunner_run_all(sr, CK_VERBOSE);

	/* count all the failed tests */
	number_failed = srunner_ntests_failed(sr);

	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS: EXIT_FAILURE;
}

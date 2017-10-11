/*
 * main test for bcmutils module
 *
 * $Copyright (C) 2014, Broadcom Corporation$
 *
 * $Id:$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <test_bcm_tlv.h>
#include <test_bcm_bitop.h>

/*	*********************
 *	Start of Test Section
 *	*********************
 */

#include <check.h> /* Includes Check framework */

int main(void)
{
	int number_failed; /* Count number of failed tests */

	/* Adding suit to SRunner. */
	SRunner *sr = srunner_create(bcmwritetlv_suite());
	srunner_add_suite(sr, bcmcopytlv_suite());
	srunner_add_suite(sr, bcmparsetlv_suite());
	srunner_add_suite(sr, bcmbitop_suite());

	/* Prints the summary one message per test (passed or failed) */
	srunner_run_all(sr, CK_VERBOSE);

	/* count all the failed tests */
	number_failed = srunner_ntests_failed(sr);

	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS: EXIT_FAILURE;
}

/*
 * Unit tests for wlc_dump_reg functions.
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
#include <wlc_dump_reg.h>
#include <bcmutils.h>

/*	*********************
 *	Start of Test Section
 *	*********************
 */
#include <check.h> /* Includes Check framework */

/* ------------- Global Definitions ------------------------- */
wlc_dump_reg_info_t *reg;
osl_t *osh;
int dump1[1], dump2[1];

/* ------------- Setup and Teardown - Fixtures --------------- */

static int
dump_test(int *dump)
{
	*dump = BCME_OK;
	return *dump;
}

void DumpRegCreate_Setup(void)
{
	/* Setup for testDumpRegCreate */
}

void DumpRegCreate_Teardown(void)
{
	wlc_dump_reg_destroy(reg);
}

void DumpRegAddFn_Setup(void)
{
	/* Setup for testDumpRegAddFn */
	reg = wlc_dump_reg_create(osh, 1);
}

void DumpRegAddFn_Teardown(void)
{
	wlc_dump_reg_destroy(reg);
}

void DumpRegInvFn_Setup(void)
{
	reg = wlc_dump_reg_create(osh, 2);
	wlc_dump_reg_add_fn(reg, "dump1", (wlc_dump_reg_fn_t)dump_test, dump1);
	wlc_dump_reg_add_fn(reg, "dump2", (wlc_dump_reg_fn_t)dump_test, dump2);
}

void DumpRegInvFn_Teardown(void)
{
	wlc_dump_reg_destroy(reg);
}

START_TEST(testDumpRegCreate)
{
	reg = wlc_dump_reg_create(osh, 10);
	ck_assert_msg(reg != NULL, "Failed in wlc_dump_reg_create with"
		" max value as 10");
	ck_assert_msg(wlc_dump_reg_create(osh, 0) == NULL, "Failed in wlc_dump_reg_create with"
		" max value as 0");
}
END_TEST

START_TEST(testDumpRegAddFn)
{
	int res;

	res = wlc_dump_reg_add_fn(reg, "dump", (wlc_dump_reg_fn_t)dump_test, dump1);
	ck_assert_msg(res == BCME_OK, "Failed in wlc_dump_reg_add_fn");
#if 0
	/* XXX should not be passing NULL as parameter(s) */
	ck_assert_msg(wlc_dump_reg_add_fn(NULL, "dump", (wlc_dump_reg_fn_t)dump_test,
		dump1) == BCME_ERROR, "Failed in wlc_dump_reg_add_fn with reg as NULL");
	ck_assert_msg(wlc_dump_reg_add_fn(reg, NULL, (wlc_dump_reg_fn_t)dump_test,
		dump1) == BCME_ERROR, "Failed in wlc_dump_reg_add_fn with name as NULL");
	ck_assert_msg(wlc_dump_reg_add_fn(reg, "dump", NULL, dump1) == BCME_ERROR,
		"Failed in wlc_dump_reg_add_fn with fn as NULL");
#endif
	ck_assert_msg(wlc_dump_reg_add_fn(reg, "", (wlc_dump_reg_fn_t)dump_test,
		dump1) == BCME_BADARG, "Failed in wlc_dump_reg_add_fn with namelen as 0");
	ck_assert_msg(wlc_dump_reg_add_fn(reg, "dumpfunctionwithbiggerlength",
		(wlc_dump_reg_fn_t)dump_test, dump1) == BCME_BADARG,
		"Failed in wlc_dump_reg_add_fn with bigger namelen");
	ck_assert_msg(wlc_dump_reg_add_fn(reg, "dump", (wlc_dump_reg_fn_t)dump_test,
		dump1) == BCME_ERROR, "Failed in adding function which already exists");
	ck_assert_msg(wlc_dump_reg_add_fn(reg, "dump2", (wlc_dump_reg_fn_t)dump_test,
		dump2) == BCME_NORESOURCE, "Failed in adding dump2");
}
END_TEST

START_TEST(testDumpRegInvFn)
{
	int res;
	*dump1 = -1;
	*dump2 = -11;

	ck_assert_msg(wlc_dump_reg_invoke_fn(reg, "dump1", dump1) == BCME_OK, "Failed in invoking"
		" function dump1");
	ck_assert_msg(*dump1 == BCME_OK, "Failed in validating dump1");
	ck_assert_msg(wlc_dump_reg_invoke_fn(reg, "dump2", dump2) == BCME_OK, "Failed in invoking"
		" function dump2");
	ck_assert_msg(*dump2 == BCME_OK, "Failed in validating dump2");
#if 0
	/* XXX should not be passing NULL as parameter(s) */
	ck_assert_msg(wlc_dump_reg_invoke_fn(NULL, "dump1", dump1) == BCME_ERROR, "Failed in"
		"invoking function dump1 with reg as NULL");
	ck_assert_msg(wlc_dump_reg_invoke_fn(reg, NULL, dump1) == BCME_ERROR, "Failed in"
		"invoking function dump1 with name as NULL");
#endif
	ck_assert_msg(wlc_dump_reg_invoke_fn(reg, "dump", dump1) == BCME_NOTFOUND, "Failed in"
		"invoking function dump1 with invalid name");
#if 0
	/* XXX should not be passing NULL as parameter(s) */
	ck_assert_msg(wlc_dump_reg_invoke_fn(NULL, "dump2", dump2) == BCME_ERROR, "Failed in"
		"invoking function dump2 with reg as NULL");
	ck_assert_msg(wlc_dump_reg_invoke_fn(reg, NULL, dump2) == BCME_ERROR, "Failed in"
		"invoking function dump2 with name as NULL");
#endif
	ck_assert_msg(wlc_dump_reg_invoke_fn(reg, "dump", dump2) == BCME_NOTFOUND, "Failed in"
		"invoking function dump1 with invalid name");
}
END_TEST

/* -------------------TEST SUITES--------------------- */

Suite *wlcdumpreg_suite(void)
{
	Suite *s = suite_create("wlc_dump_reg_suite");
	TCase *tc_dump_cr = tcase_create("Test Case");
	tcase_add_checked_fixture(tc_dump_cr, DumpRegCreate_Setup, DumpRegCreate_Teardown);
	tcase_add_test(tc_dump_cr, testDumpRegCreate);

	TCase *tc_dump_add = tcase_create("Test Case");
	tcase_add_checked_fixture(tc_dump_add, DumpRegAddFn_Setup, DumpRegAddFn_Teardown);
	tcase_add_test(tc_dump_add, testDumpRegAddFn);

	TCase *tc_dump_invfn = tcase_create("Test Case");
	tcase_add_checked_fixture(tc_dump_invfn, DumpRegInvFn_Setup, DumpRegInvFn_Teardown);
	tcase_add_test(tc_dump_invfn, testDumpRegInvFn);

	suite_add_tcase(s, tc_dump_cr);
	suite_add_tcase(s, tc_dump_add);
	suite_add_tcase(s, tc_dump_invfn);

	return s;
}

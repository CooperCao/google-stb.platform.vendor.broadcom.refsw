/*
 * Unit tests for bcm_tlv functions.
 *
 * $Copyright (C) 2014, Broadcom Corporation$
 *
 * $Id$
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bcmutils.h>

#if 0
#include <proto/802.11ax.h>
#endif

/*	*********************
 *	Start of Test Section
 *	*********************
 */
#include <check.h> /* Includes Check framework */

#include <assert.h>
#undef ASSERT
#define ASSERT(exp) assert(exp)

/* -------------------TEST SUITES--------------------- */

static uint8 buf[6];

void BitOp_Setup(void)
{
	memset(buf, 0, sizeof(buf));
}

void BitOp_Teardown(void)
{
}

#if 0
static const bcm_bit_desc_t scb_mac_cap[] =
{
	{HE_MAC_PPE_THRESH_PRESENT, "PPEPresent"},
	{HE_MAC_TWT_REQ_SUPPORT, "TWTReq"},
	{HE_MAC_TWT_RESP_SUPPORT, "TWTResp"},
	{HE_MAC_MAX_NSS_DCM, "Max-Nss-DCM"},
	{HE_MAC_UL_MU_RSP_SCHED, "UL-MU-Resp"},
	{HE_MAC_A_BSR, "A-BSR"}
};

static he_mac_cap_t he_mac_cap;
#endif

START_TEST(testBcmBitOp)
{
	uint val, ret;
	char fmtstr[32];

	/* single bit at bit 0 of differnt byte */

	/* first byte */
	ret = val = 1;
	setbits(buf, sizeof(buf), 0, 1, val);
	printf("buf: %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	ret = getbits(buf, sizeof(buf), 0, 1);
	ck_assert_int_eq(ret, val);
	clrbits(buf, sizeof(buf), 0, 1);
	ret = getbits(buf, sizeof(buf), 0, 1);
	ck_assert_int_eq(ret, 0);

	/* middle byte */
	ret = val = 1;
	setbits(buf, sizeof(buf), 8, 1, val);
	printf("buf: %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	ret = getbits(buf, sizeof(buf), 8, 1);
	ck_assert_int_eq(ret, val);
	clrbits(buf, sizeof(buf), 8, 1);
	ret = getbits(buf, sizeof(buf), 8, 1);
	ck_assert_int_eq(ret, 0);

	/* last byte */
	ret = val = 1;
	setbits(buf, sizeof(buf), 40, 1, val);
	printf("buf: %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	ret = getbits(buf, sizeof(buf), 40, 1);
	ck_assert_int_eq(ret, val);
	clrbits(buf, sizeof(buf), 40, 1);
	ret = getbits(buf, sizeof(buf), 40, 1);
	ck_assert_int_eq(ret, 0);

	/* two bits at byte boundary of differnt bytes */

	/* first & second bytes */
	ret = val = 3;
	setbits(buf, sizeof(buf), 7, 2, val);
	printf("buf: %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	ret = getbits(buf, sizeof(buf), 7, 2);
	ck_assert_int_eq(ret, val);
	clrbits(buf, sizeof(buf), 7, 2);
	ret = getbits(buf, sizeof(buf), 7, 2);
	ck_assert_int_eq(ret, 0);

	/* middle bytes */
	ret = val = 3;
	setbits(buf, sizeof(buf), 15, 2, val);
	printf("buf: %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	ret = getbits(buf, sizeof(buf), 15, 2);
	ck_assert_int_eq(ret, val);
	clrbits(buf, sizeof(buf), 15, 2);
	ret = getbits(buf, sizeof(buf), 15, 2);
	ck_assert_int_eq(ret, 0);

	/* last two bytes */
	ret = val = 3;
	setbits(buf, sizeof(buf), 39, 2, val);
	printf("buf: %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	ret = getbits(buf, sizeof(buf), 39, 2);
	ck_assert_int_eq(ret, val);
	clrbits(buf, sizeof(buf), 39, 2);
	ret = getbits(buf, sizeof(buf), 39, 2);
	ck_assert_int_eq(ret, 0);

	/* one bit followed by one full byte at differnt bytes */

	ret = val = 0x1ff;
	setbits(buf, sizeof(buf), 7, 9, val);
	printf("buf: %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	val = getbits(buf, sizeof(buf), 7, 9);
	ck_assert_int_eq(ret, val);
	clrbits(buf, sizeof(buf), 7, 9);
	ret = getbits(buf, sizeof(buf), 7, 9);
	ck_assert_int_eq(ret, 0);

	ret = val = 0x1ff;
	setbits(buf, sizeof(buf), 15, 9, val);
	printf("buf: %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	val = getbits(buf, sizeof(buf), 15, 9);
	ck_assert_int_eq(ret, val);
	clrbits(buf, sizeof(buf), 15, 9);
	ret = getbits(buf, sizeof(buf), 15, 9);
	ck_assert_int_eq(ret, 0);

	ret = val = 0x1ff;
	setbits(buf, sizeof(buf), 39, 9, val);
	printf("buf: %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	val = getbits(buf, sizeof(buf), 39, 9);
	ck_assert_int_eq(ret, val);
	clrbits(buf, sizeof(buf), 39, 9);
	ret = getbits(buf, sizeof(buf), 39, 9);
	ck_assert_int_eq(ret, 0);

	/* one full byte followd by on bit at differnt bytes */

	ret = val = 0x1ff;
	setbits(buf, sizeof(buf), 0, 9, val);
	printf("buf: %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	val = getbits(buf, sizeof(buf), 0, 9);
	ck_assert_int_eq(ret, val);
	clrbits(buf, sizeof(buf), 0, 9);
	ret = getbits(buf, sizeof(buf), 0, 9);
	ck_assert_int_eq(ret, 0);

	ret = val = 0x1ff;
	setbits(buf, sizeof(buf), 8, 9, val);
	printf("buf: %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	val = getbits(buf, sizeof(buf), 8, 9);
	ck_assert_int_eq(ret, val);
	clrbits(buf, sizeof(buf), 8, 9);
	ret = getbits(buf, sizeof(buf), 8, 9);
	ck_assert_int_eq(ret, 0);

	ret = val = 0x1ff;
	setbits(buf, sizeof(buf), 32, 9, val);
	printf("buf: %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	val = getbits(buf, sizeof(buf), 32, 9);
	ck_assert_int_eq(ret, val);
	clrbits(buf, sizeof(buf), 32, 9);
	ret = getbits(buf, sizeof(buf), 32, 9);
	ck_assert_int_eq(ret, 0);

	/* one bit followed by one full byte followed by one bit at differnt bytes */

	ret = val = 0x3ff;
	setbits(buf, sizeof(buf), 7, 10, val);
	printf("buf: %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	val = getbits(buf, sizeof(buf), 7, 10);
	ck_assert_int_eq(ret, val);
	clrbits(buf, sizeof(buf), 7, 10);
	ret = getbits(buf, sizeof(buf), 7, 10);
	ck_assert_int_eq(ret, 0);

	ret = val = 0x3ff;
	setbits(buf, sizeof(buf), 15, 10, val);
	printf("buf: %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	val = getbits(buf, sizeof(buf), 15, 10);
	ck_assert_int_eq(ret, val);
	clrbits(buf, sizeof(buf), 15, 10);
	ret = getbits(buf, sizeof(buf), 15, 10);
	ck_assert_int_eq(ret, 0);

	ret = val = 0x3ff;
	setbits(buf, sizeof(buf), 31, 10, val);
	printf("buf: %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	val = getbits(buf, sizeof(buf), 31, 10);
	ck_assert_int_eq(ret, val);
	clrbits(buf, sizeof(buf), 31, 10);
	ret = getbits(buf, sizeof(buf), 31, 10);
	ck_assert_int_eq(ret, 0);

	/* one bit followed by two full bytes followed by one bit at differnt bytes */

	ret = val = 0x3ffff;
	setbits(buf, sizeof(buf), 15, 18, val);
	printf("buf: %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	val = getbits(buf, sizeof(buf), 15, 18);
	ck_assert_int_eq(ret, val);
	clrbits(buf, sizeof(buf), 15, 18);
	ret = getbits(buf, sizeof(buf), 15, 18);
	ck_assert_int_eq(ret, 0);

#if 0
	/* format test */

	setbit(he_mac_cap, HE_MAC_PPE_THRESH_PRESENT);
	setbit(he_mac_cap, HE_MAC_TWT_REQ_SUPPORT);
	setbit(he_mac_cap, HE_MAC_TWT_RESP_SUPPORT);
	setbit(he_mac_cap, HE_MAC_MAX_NSS_DCM);
	setbit(he_mac_cap, HE_MAC_UL_MU_RSP_SCHED);
	setbit(he_mac_cap, HE_MAC_A_BSR);
	printf("he_mac_cap: %02x %02x %02x %02x %02x\n",
	       he_mac_cap[0], he_mac_cap[1], he_mac_cap[2], he_mac_cap[3], he_mac_cap[4]);
	bcm_format_octets(scb_mac_cap, ARRAYSIZE(scb_mac_cap), he_mac_cap, sizeof(he_mac_cap),
	                  fmtstr, sizeof(fmtstr));
	printf("%s\n", fmtstr);
#endif
}
END_TEST

Suite *bcmbitop_suite(void)
{
	Suite *s = suite_create("bcmbitop_suite");
	TCase *tc_bitop = tcase_create("Test Case");

	tcase_add_checked_fixture(tc_bitop, BitOp_Setup, BitOp_Teardown);
	tcase_add_test(tc_bitop, testBcmBitOp);

	suite_add_tcase(s, tc_bitop);

	return s;
}

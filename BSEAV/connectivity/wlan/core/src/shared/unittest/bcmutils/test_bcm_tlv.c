/*
 * Unit tests for bcm_tlv functions.
 *
 * $Copyright (C) 2014, Broadcom Corporation$
 *
 * $Id:$
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <proto/802.11.h>
#include <bcmutils.h>

/*	*********************
 *	Start of Test Section
 *	*********************
 */
#include <check.h> /* Includes Check framework */

/* ------------- Global Definitions ------------------------- */
#define BUFFER_LEN 20

/* Sample TLV IDs */
#define TEST_TLV_SSID_ID		0
#define TEST_TLV_RATES_ID		1
#define TEST_TLV_FH_PARMS_ID		2
#define TEST_TLV_DS_PARMS_ID		3
#define TEST_TLV_MAX_IDLE_PERIOD_ID	90
#define TEST_TLV_MAX_IDLE_PERIOD_IE_LEN 3
#define TEST_TLV_HT_OBSS_ID		74
#define TEST_TLV_VHT_CAP_ID		191
#define TEST_TLV_ID5			7
#define TEST_TLV_ID5_LEN		150
#define TEST_TLV_ID_A			1
#define TEST_TLV_ID_B			2
#define TEST_TLV_ID_C			3
#define TEST_TLV_ID_D			4
#define TEST_PROP_OUIA			"\x00\x92\x4C"
#define TEST_PROP_OUIB			"\x00\x11\x18"
#define TEST_PROP_OUIC			"\x00\x11\x14"
#define TEST_PROP_OUID			"\x00\x11\x15"
#define TEST_PROP_OUI1			"\x00\x90\x4C"
#define TEST_PROP_OUI2			"\x00\x10\x18"
#define TEST_PROP_OUI3			"\x00\x10\x14"
#define TEST_PROP_OUI4			"\x00\x10\x15"
#define TEST_PROP_OUI5			"\x00\x10\x12"
#define TEST_PROP_OUI6			"\x00\x10\x11"
#define TEST_PROP_TYPE1			53
#define TEST_PROP_TYPE2			55
#define TEST_PROP_TYPE3			57

uint8 *buffer, *tlv2, *tlv3, *tlv4, *tlv5;
uint8 *vtlv2, *vtlv3, *vtlv4, *vtlv5, type1, type2, type3[4];
uint8 rates[8], tlv5_data[150], typeA[252], typeB[252], typeC[252];
uint8 a_data[255], b_data[255], c_data[255], *ie_a, *ie_b, *ie_c;

int alen, blen, clen, channel;
char ssid[8];

int testExhaustBufLen3IEs(uint8 *, int);
int testExhaustBufLen3VendorIEs(uint8 *, int);
int testExhaustBufLen2IEs(uint8 *, int);

/* ------------- Setup and Teardown - Fixtures --------------- */

void ParseTlv_Setup(void)
{
	int i, max_idle_period = 0;
	channel = 165;
	strcpy(ssid, "brcmwpa");
	uint8 *tlv6;

	buffer = (uint8 *)malloc(BUFFER_LEN + 153);
	if (buffer == NULL) {
		printf("Setup function for ParseTlv failed\n");
		exit(-1);
	}
	memset(buffer, 0, BUFFER_LEN + 153);

	tlv2 = bcm_write_tlv(TEST_TLV_DS_PARMS_ID, &channel, 1, buffer);
	tlv3 = bcm_write_tlv(TEST_TLV_SSID_ID, ssid, strlen(ssid), tlv2);
	tlv4 = bcm_write_tlv(TEST_TLV_MAX_IDLE_PERIOD_ID, &max_idle_period,
		TEST_TLV_MAX_IDLE_PERIOD_IE_LEN, tlv3);
	tlv5 = bcm_write_tlv(TEST_TLV_RATES_ID, rates, 0, tlv4);
	for (i = 0; i < 150; i++)
		tlv5_data[i] = i;
	tlv6 = bcm_write_tlv(TEST_TLV_ID5, tlv5_data, TEST_TLV_ID5_LEN, tlv5);

}

void ParseTlv_Teardown(void)
{

	free(buffer);

}

void BufferLen_Setup3IEs(void)
{
	/* allocate a buffer to fit 3 max sized TLVs, TLV hdr plus 255 data */
	buffer = (uint8 *)malloc(3 * 257);
	if (buffer == NULL) {
		printf("Setup function for Exhaustive BufferLen test 3IEs failed\n");
		exit(-1);
	}

	/* fill the source ID data with a pattern */
	memset(a_data, 'a', sizeof(a_data));
	memset(b_data, 'b', sizeof(b_data));
	memset(c_data, 'c', sizeof(c_data));
}

void BufferLen_Teardown3IEs(void)
{

	free(buffer);

}

void BufferLen_Setup2IEs(void)
{
	/* allocate a buffer to fit 2 max sized TLVs, TLV hdr plus 255 data */
	buffer = (uint8 *)malloc(2 * 257);
	if (buffer == NULL) {
		printf("Setup function for Exhaustive BufferLen test 2IEs failed\n");
		exit(-1);
	}

	/* fill the source ID data with a pattern */
	memset(a_data, 'a', sizeof(a_data));
	memset(b_data, 'b', sizeof(b_data));
}

void BufferLen_Teardown2IEs(void)
{

	free(buffer);

}

void ParseOrderedTlv_Setup(void)
{
	int max_idle_period = 0;
	channel = 165;
	strcpy(ssid, "brcmwpa");

	buffer = (uint8 *)malloc(BUFFER_LEN);
	if (buffer == NULL) {
		printf("Setup function for ParseOrderedTlv failed\n");
		exit(-1);
	}
	memset(buffer, 0, BUFFER_LEN);

	tlv2 = bcm_write_tlv(TEST_TLV_SSID_ID, ssid, strlen(ssid), buffer);
	tlv3 = bcm_write_tlv(TEST_TLV_RATES_ID, rates, 0, tlv2);
	tlv4 = bcm_write_tlv(TEST_TLV_DS_PARMS_ID, &channel, 1, tlv3);
	tlv5 = bcm_write_tlv(TEST_TLV_MAX_IDLE_PERIOD_ID, &max_idle_period,
		TEST_TLV_MAX_IDLE_PERIOD_IE_LEN, tlv4);

}

void ParseOrderedTlv_Teardown(void)
{

	free(buffer);

}

void WriteTlv_Setup(void)
{
	channel = 165;
	strcpy(ssid, "brcmwpa");
	buffer = (uint8 *)malloc(BUFFER_LEN + 1);
	if (buffer == NULL) {
		printf("Setup function for WriteTlv failed\n");
		exit(-1);
	}
	memset(buffer, 0, BUFFER_LEN + 1);
}

void WriteTlv_Teardown(void)
{
	free(buffer);
}

void VendorIE_Setup(void)
{
	int i;
	uint8 vdata1[4], vdata2[4], vdata3[7], vdata4[3], vdata5[4], *vtlv6;
	char *voui1, *voui2, *voui3, *voui4, *voui5;
	voui1 = TEST_PROP_OUI1;
	voui2 = TEST_PROP_OUI2;
	voui3 = TEST_PROP_OUI3;
	voui4 = TEST_PROP_OUI4;
	voui5 = TEST_PROP_OUI5;

	type1 = TEST_PROP_TYPE1;
	type2 = TEST_PROP_TYPE2;

	memcpy(vdata1, voui1, DOT11_OUI_LEN);
	memcpy(&vdata1[DOT11_OUI_LEN], &type1, sizeof(type1));

	memcpy(vdata2, voui2, DOT11_OUI_LEN);
	memcpy(&vdata2[DOT11_OUI_LEN], &type2, sizeof(type2));

	for (i = 0; i < 4; i++)
		type3[i] = i;
	memcpy(vdata3, voui3, DOT11_OUI_LEN);
	memcpy(&vdata3[DOT11_OUI_LEN], type3, sizeof(type3));

	memcpy(vdata4, voui4, DOT11_OUI_LEN);

	memcpy(vdata5, voui5, DOT11_OUI_LEN);
	memcpy(&vdata5[DOT11_OUI_LEN], &type1, sizeof(type1));
	buffer = (uint8 *)malloc(BUFFER_LEN + 12);
	if (buffer == NULL) {
		printf("Setup function for VendorIE failed\n");
		exit(-1);
	}
	memset(buffer, 0, BUFFER_LEN + 12);

	vtlv2 = bcm_write_tlv(DOT11_MNG_PROPR_ID, vdata1, sizeof(vdata1), buffer);
	vtlv3 = bcm_write_tlv(DOT11_MNG_PROPR_ID, vdata2, sizeof(vdata2), vtlv2);
	vtlv4 = bcm_write_tlv(DOT11_MNG_PROPR_ID, vdata3, sizeof(vdata3), vtlv3);
	vtlv5 = bcm_write_tlv(DOT11_MNG_PROPR_ID, vdata4, sizeof(vdata4), vtlv4);
	vtlv6 = bcm_write_tlv(TEST_TLV_SSID_ID, vdata5, sizeof(vdata5), vtlv5);

}

void VendorIE_Teardown(void)
{
	free(buffer);
}

void BufferLen_Setup3VenIEs(void)
{
	char *vouiA, *vouiB, *vouiC;
	vouiA = TEST_PROP_OUIA;
	vouiB = TEST_PROP_OUIB;
	vouiC = TEST_PROP_OUIC;
	/* allocate a buffer to fit 3 max sized TLVs, TLV hdr plus 255 data */
	buffer = (uint8 *)malloc(3 * 257);
	if (buffer == NULL) {
		printf("Setup function for Exhaustive BufferLen test 3VendorIEs failed\n");
		exit(-1);
	}

	/* fill the source type data with a pattern */
	memset(typeA, 'a', sizeof(typeA));
	memset(typeB, 'b', sizeof(typeB));
	memset(typeC, 'c', sizeof(typeC));

	/* fill the source ID data with a pattern */
	memcpy(a_data, vouiA, DOT11_OUI_LEN);
	memcpy(&a_data[DOT11_OUI_LEN], typeA, sizeof(typeA));

	memcpy(b_data, vouiB, DOT11_OUI_LEN);
	memcpy(&b_data[DOT11_OUI_LEN], typeB, sizeof(typeB));

	memcpy(c_data, vouiC, DOT11_OUI_LEN);
	memcpy(&c_data[DOT11_OUI_LEN], typeC, sizeof(typeC));
}

void BufferLen_Teardown3VenIEs(void)
{

	free(buffer);

}

void BufferLen_Setup2VenIEs(void)
{
	char *vouiA, *vouiB;
	vouiA = TEST_PROP_OUIA;
	vouiB = TEST_PROP_OUIB;
	/* allocate a buffer to fit 2 max sized TLVs, TLV hdr plus 255 data */
	buffer = (uint8 *)malloc(2 * 257);
	if (buffer == NULL) {
		printf("Setup function for Exhaustive BufferLen test 2VendorIEs failed\n");
		exit(-1);
	}

	/* fill the source type data with a pattern */
	memset(typeA, 'a', sizeof(typeA));
	memset(typeB, 'b', sizeof(typeB));

	/* fill the source ID data with a pattern */
	memcpy(a_data, vouiA, DOT11_OUI_LEN);
	memcpy(&a_data[DOT11_OUI_LEN], typeA, sizeof(typeA));

	memcpy(b_data, vouiB, DOT11_OUI_LEN);
	memcpy(&b_data[DOT11_OUI_LEN], typeB, sizeof(typeB));
}

void BufferLen_Teardown2VenIEs(void)
{

	free(buffer);

}

/* ----------------------UNIT TESTS------------------------ */

int testExhaustBufLen3IEs(uint8* buf, int buflen)
{
	bcm_tlv_t *p;
	int len;

	/* should find A */
	if (bcm_parse_tlvs(buf, buflen, TEST_TLV_ID_A) != (bcm_tlv_t *)ie_a) {
		printf("Error in parsing TLV-A:alen=%d,blen=%d,clen=%d\n", alen, blen, clen);
		return 1;
	}

	if (bcm_parse_ordered_tlvs(buf, buflen, TEST_TLV_ID_A) != (bcm_tlv_t *)ie_a) {
		printf("Error in parsing ordered TLV-A:alen=%d,blen=%d,clen=%d\n", alen, blen,
			clen);
		return 1;
	}

	if (bcm_parse_tlvs_min_bodylen(buf, buflen, TEST_TLV_ID_A, alen - 1) !=
		(bcm_tlv_t *)ie_a) {
		printf("Error in parsing TLV-A with min length:alen=%d,blen=%d,clen=%d\n", alen,
			blen, clen);
		return 1;
	}

	/* shouldn't find A as it's IE length is < min_length */
	if (bcm_parse_tlvs_min_bodylen(buf, buflen, TEST_TLV_ID_A, alen + 1) != NULL) {
		printf("Error in parsing TLV-A with min length :alen=%d,blen=%d,clen=%d\n", alen,
			blen, clen);
		return 1;
	}

	/* should find B */
	if (bcm_parse_tlvs(buf, buflen, TEST_TLV_ID_B) != (bcm_tlv_t *)ie_b) {
		printf("Error in parsing TLV-B:alen=%d,blen=%d,clen=%d\n", alen, blen, clen);
		return 1;
	}

	if (bcm_parse_ordered_tlvs(buf, buflen, TEST_TLV_ID_B) != (bcm_tlv_t *)ie_b) {
		printf("Error in parsing ordered TLV-B:alen=%d,blen=%d,clen=%d\n", alen, blen,
			clen);
		return 1;
	}

	if (bcm_parse_tlvs_min_bodylen(buf, buflen, TEST_TLV_ID_B, blen - 1) !=
		(bcm_tlv_t *)ie_b) {
		printf("Error in parsing TLV-B with min length:alen=%d,blen=%d,clen=%d\n", alen,
			blen, clen);
		return 1;
	}

	/* shouldn't find B as it's IE length is < min_length */
	if (bcm_parse_tlvs_min_bodylen(buf, buflen, TEST_TLV_ID_B, blen + 1) != NULL) {
		printf("Error in parsing TLV-B with min length:alen=%d,blen=%d,clen=%d\n", alen,
			blen, clen);
		return 1;
	}

	/* should fail on C since it is truncated or missing */
	if (bcm_parse_tlvs(buf, buflen, TEST_TLV_ID_C) != NULL) {
		printf("Error in parsing TLV-C:alen=%d,blen=%d,clen=%d\n", alen, blen, clen);
		return 1;
	}

	if (bcm_parse_ordered_tlvs(buf, buflen, TEST_TLV_ID_C) != NULL) {
		printf("Error in parsing ordered TLV-C:alen=%d,blen=%d,clen=%d\n", alen, blen,
			clen);
		return 1;
	}

	if (bcm_parse_tlvs_min_bodylen(buf, buflen, TEST_TLV_ID_C, clen - 1) != NULL) {
		printf("Error in parsing TLV-C with min length:alen=%d,blen=%d,clen=%d\n", alen,
			blen, clen);
		return 1;
	}

	if (bcm_parse_tlvs_min_bodylen(buf, buflen, TEST_TLV_ID_C, clen + 1) != NULL) {
		printf("Error in parsing TLV-C with min length:alen=%d,blen=%d,clen=%d\n", alen,
			blen, clen);
		return 1;
	}

	/* should fail to find D, but gracefuly handle stopping at a bad IE */
	if (bcm_parse_tlvs(buf, buflen, TEST_TLV_ID_D) != NULL) {
		printf("Error in parsing TLV-D:alen=%d,blen=%d,clen=%d\n", alen, blen, clen);
		return 1;
	}

	if (bcm_parse_ordered_tlvs(buf, buflen, TEST_TLV_ID_D) != NULL) {
		printf("Error in parsing ordered TLV-D:alen=%d,blen=%d,clen=%d\n", alen, blen,
			clen);
		return 1;
	}
	len = buflen;

	/* make sure the 'next' function can walk the first 2 TLVs only */
	/* should find the second TLV */
	p = bcm_next_tlv((bcm_tlv_t*)buf, &len);
	/* verify looking at an IE ID == ID_B ... */
	if (p != (bcm_tlv_t *)ie_b) {
		printf("Error in parsing next TLV-A:alen=%d,blen=%d,clen=%d\n", alen, blen, clen);
		return 1;
	}
	/* should not find a 3rd IE */
	if (bcm_next_tlv(p, &len) != NULL) {
		printf("Error in parsing next TLV-B:alen=%d,blen=%d,clen=%d\n", alen, blen, clen);
		return 1;
	}
	return 0;
}

int testExhaustBufLen2IEs(uint8* buf, int buflen)
{
	/* should find A */
	if (bcm_parse_tlvs(buf, buflen, TEST_TLV_ID_A) != (bcm_tlv_t *)ie_a) {
		printf("Error in parsing TLV-A:alen=%d,blen=%d\n", alen, blen);
		return 1;
	}

	if (bcm_parse_ordered_tlvs(buf, buflen, TEST_TLV_ID_A) != (bcm_tlv_t *)ie_a) {
		printf("Error in parsing ordered TLV-A:alen=%d,blen=%d\n", alen, blen);
		return 1;
	}

	if (bcm_parse_tlvs_min_bodylen(buf, buflen, TEST_TLV_ID_A, alen - 1) !=
		(bcm_tlv_t *)ie_a) {
		printf("Error in parsing TLV-A with min length:alen=%d,blen=%d\n", alen, blen);
		return 1;
	}

	/* shouldn't find A as it's IE length is < min_length */
	if (bcm_parse_tlvs_min_bodylen(buf, buflen, TEST_TLV_ID_A, alen + 1) != NULL) {
		printf("Error in parsing TLV-A with min length :alen=%d,blen=%d\n", alen, blen);
		return 1;
	}

	/* should fail on B since it is truncated or missing */
	if (bcm_parse_tlvs(buf, buflen, TEST_TLV_ID_B) != NULL) {
		printf("Error in parsing TLV-B:alen=%d,blen=%d\n", alen, blen);
		return 1;
	}

	if (bcm_parse_ordered_tlvs(buf, buflen, TEST_TLV_ID_B) != NULL) {
		printf("Error in parsing ordered TLV-B:alen=%d,blen=%d\n", alen, blen);
		return 1;
	}

	if (bcm_parse_tlvs_min_bodylen(buf, buflen, TEST_TLV_ID_B, blen - 1) != NULL) {
		printf("Error in parsing TLV-B with min length:alen=%d,blen=%d\n", alen, blen);
		return 1;
	}

	if (bcm_parse_tlvs_min_bodylen(buf, buflen, TEST_TLV_ID_B, blen + 1) != NULL) {
		printf("Error in parsing TLV-B with min length:alen=%d,blen=%d\n", alen, blen);
		return 1;
	}
	/* should fail to find C, but gracefuly handle stopping at a bad IE */
	if (bcm_parse_tlvs(buf, buflen, TEST_TLV_ID_C) != NULL) {
		printf("Error in parsing TLV-C:alen=%d,blen=%d\n", alen, blen);
		return 1;
	}

	if (bcm_parse_ordered_tlvs(buf, buflen, TEST_TLV_ID_C) != NULL) {
		printf("Error in parsing ordered TLV-C:alen=%d,blen=%d\n", alen, blen);
		return 1;
	}
	/* make sure the 'next' function cannot find a 2nd IE */
	if (bcm_next_tlv((bcm_tlv_t*)buf, &buflen) != NULL) {
		printf("Error in parsing next TLV-A:alen=%d,blen=%d\n", alen, blen);
		return 1;
	}
	return 0;
}

int testExhaustBufLen3VendorIEs(uint8* buf, int buflen)
{
	bcm_tlv_t *p;
	int len, a_typelen, b_typelen, c_typelen;
	a_typelen = alen - 3;
	b_typelen = blen - 3;
	c_typelen = clen - 3;
	/* should find A */
	if (bcm_find_vendor_ie(buf, buflen, TEST_PROP_OUIA, typeA, a_typelen) !=
		(bcm_tlv_t *)ie_a) {
		printf("Error in parsing Vendor TLV-A:a_typelen=%d,b_typelen=%d,c_typelen=%d\n",
			a_typelen, b_typelen, c_typelen);
		return 1;
	}

	/* should find B */
	if (bcm_find_vendor_ie(buf, buflen, TEST_PROP_OUIB, typeB, b_typelen) !=
		(bcm_tlv_t *)ie_b) {
		printf("Error in parsing Vendor TLV-B:a_typelen=%d,b_typelen=%d,c_typelen=%d\n",
			a_typelen, b_typelen, c_typelen);
		return 1;
	}

	/* should fail on C since it is truncated or missing */
	if (bcm_find_vendor_ie(buf, buflen, TEST_PROP_OUIC, typeC, c_typelen) != NULL) {
		printf("Error in parsing Vendor TLV-C:a_typelen=%d,b_typelen=%d,c_typelen=%d\n",
			a_typelen, b_typelen, c_typelen);
		return 1;
	}

	/* should fail to find D, but gracefuly handle stopping at a bad IE */
	if (bcm_find_vendor_ie(buf, buflen, TEST_PROP_OUID, typeC, c_typelen) != NULL) {
		printf("Error in parsing Vendor TLV-D:a_typelen=%d,b_typelen=%d,c_typelen=%d\n",
			a_typelen, b_typelen, c_typelen);
		return 1;
	}

	len = buflen;

	/* make sure the 'next' function can walk the first 2 TLVs only */
	/* should find the second TLV */
	p = bcm_next_tlv((bcm_tlv_t*)buf, &len);
	/* verify looking at an IE ID == ID_B ... */
	if (p != (bcm_tlv_t *)ie_b) {
		printf("Error in parsing next vendor TLV-A:a_typelen=%d,b_typelen=%d,c_typelen=%d"
			"\n", a_typelen, b_typelen, c_typelen);
		return 1;
	}
	/* should not find a 3rd IE */
	if (bcm_next_tlv(p, &len) != NULL) {
		printf("Error in parsing next vendor TLV-B:a_typelen=%d,b_typelen=%d,c_typelen=%d"
			"\n", a_typelen, b_typelen, c_typelen);
		return 1;
	}
	return 0;
}

int testExhaustBufLen2VendorIEs(uint8* buf, int buflen)
{
	int a_typelen, b_typelen;
	a_typelen = alen - 3;
	b_typelen = blen - 3;
	/* should find A */
	if (bcm_find_vendor_ie(buf, buflen, TEST_PROP_OUIA, typeA, a_typelen) !=
		(bcm_tlv_t *)ie_a) {
		printf("Error in parsing Vendor TLV-A:a_typelen=%d,b_typelen=%d\n",
			a_typelen, b_typelen);
		return 1;
	}

	/* should fail on B since it is truncated or missing */
	if (bcm_find_vendor_ie(buf, buflen, TEST_PROP_OUIB, typeB, b_typelen) != NULL) {
		printf("Error in parsing Vendor TLV-B:a_typelen=%d,b_typelen=%d\n",
			a_typelen, b_typelen);
		return 1;
	}

	/* should fail on C but gracefuly handle stopping at a bad IE */
	if (bcm_find_vendor_ie(buf, buflen, TEST_PROP_OUIC, typeB, b_typelen) != NULL) {
		printf("Error in parsing Vendor TLV-C:a_typelen=%d,b_typelen=%d\n",
			a_typelen, b_typelen);
		return 1;
	}

	/* make sure the 'next' function cannot find a 2nd IE */
	if (bcm_next_tlv((bcm_tlv_t*)buf, &buflen) != NULL) {
		printf("Error in parsing next vendor TLV-B:a_typelen=%d,b_typelen=%d\n",
			a_typelen, b_typelen);
		return 1;
	}
	return 0;
}

START_TEST(testBcmWriteTlv)
{
	int max_idle_period = 0;

	tlv2 = bcm_write_tlv(TEST_TLV_DS_PARMS_ID, &channel, 1, buffer);
	ck_assert_msg(tlv2 == (buffer + BCM_TLV_HDR_SIZE + 1), "Error in writing first TLV");

	tlv3 = bcm_write_tlv(TEST_TLV_SSID_ID, ssid, strlen(ssid), tlv2);
	ck_assert_msg(tlv3 == (tlv2 + BCM_TLV_HDR_SIZE + strlen(ssid)),
		"Error in writing second TLV");

	tlv4 = bcm_write_tlv(TEST_TLV_MAX_IDLE_PERIOD_ID, &max_idle_period,
		TEST_TLV_MAX_IDLE_PERIOD_IE_LEN, tlv3);
	ck_assert_msg(tlv4 == (tlv3 + BCM_TLV_HDR_SIZE + TEST_TLV_MAX_IDLE_PERIOD_IE_LEN),
		"Error in writing third TLV");
	tlv5 = bcm_write_tlv(TEST_TLV_VHT_CAP_ID, NULL, 0, tlv4);
	ck_assert_msg(tlv5 == (tlv4 + BCM_TLV_HDR_SIZE), "Error in writing fourth TLV");

	ck_assert_msg(bcm_write_tlv(TEST_TLV_DS_PARMS_ID, NULL, 1, tlv4) == tlv4,
		"Error in writing dummy TLV1");
	ck_assert_msg(bcm_write_tlv(TEST_TLV_DS_PARMS_ID, &channel, -1, tlv4) == tlv4,
		"Error in writing dummy TLV2");
	ck_assert_msg(bcm_write_tlv(TEST_TLV_DS_PARMS_ID, &channel, 1024, tlv4) == tlv4,
		"Error in writing dummy TLV3");
	ck_assert_msg(bcm_write_tlv(TEST_TLV_DS_PARMS_ID, &channel, 1, NULL) == NULL,
		"Error in writing dummy TLV4");
	ck_assert_msg(bcm_write_tlv(TEST_TLV_RATES_ID, rates, 0, tlv5) ==
		(tlv5 + BCM_TLV_HDR_SIZE), "Error in writing fifth TLV");

}
END_TEST

START_TEST(testBcmWriteTlvSafe)
{
	tlv2 = bcm_write_tlv_safe(TEST_TLV_DS_PARMS_ID, &channel, 1, buffer, 5);
	ck_assert_msg(tlv2 == (buffer + BCM_TLV_HDR_SIZE + 1), "Error in writing first TLV");

	tlv3 = bcm_write_tlv_safe(TEST_TLV_SSID_ID, ssid, strlen(ssid), tlv2, 5);
	ck_assert_msg(tlv3 == tlv2, "Error in writing dummy TLV1");

	tlv4 = bcm_write_tlv_safe(TEST_TLV_SSID_ID, ssid, strlen(ssid), tlv2, 9);
	ck_assert_msg(tlv4 == (tlv3 + BCM_TLV_HDR_SIZE + strlen(ssid)),
		"Error in writing second TLV");

	ck_assert_msg(bcm_write_tlv_safe(TEST_TLV_DS_PARMS_ID, &channel, -1, tlv4, 3) == tlv4,
		"Error in writing dummy TLV2");
	ck_assert_msg(bcm_write_tlv_safe(TEST_TLV_DS_PARMS_ID, &channel, 1024, tlv4, 3) == tlv4,
		"Error in writing dummy TLV3");
	ck_assert_msg(bcm_write_tlv_safe(TEST_TLV_RATES_ID, rates, 0, tlv4, 1) == tlv4,
		"Error in writing dummy TLV4");
	ck_assert_msg(bcm_write_tlv_safe(TEST_TLV_RATES_ID, rates, 0, tlv4, 2) ==
		(tlv4 + BCM_TLV_HDR_SIZE), "Error in writing third TLV");
}
END_TEST

START_TEST(testBcmCopyTlv)
{
	/* dst TLVs */
	uint8 *cpbuf, *cp2, *cp3, *cp4;
	cpbuf = (uint8 *)malloc(BUFFER_LEN);
	ck_assert_msg(cpbuf != NULL, "malloc error in testBcmCopyTlv");
	memset(cpbuf, 0, BUFFER_LEN);

	ck_assert_msg(bcm_copy_tlv(buffer, cpbuf) == (uint8 *)(cpbuf + BCM_TLV_HDR_SIZE +
		((bcm_tlv_t *)buffer)->len), "Error in copying first TLV");
	cp2 = (uint8 *)(cpbuf + BCM_TLV_HDR_SIZE + ((bcm_tlv_t *)buffer)->len);
	ck_assert_msg(bcm_copy_tlv(tlv2, cp2) == (uint8 *)(cp2 + BCM_TLV_HDR_SIZE +
		((bcm_tlv_t *)tlv2)->len), "Error in copying second TLV");
	cp3 = (uint8 *)(cp2 + BCM_TLV_HDR_SIZE + ((bcm_tlv_t *)tlv2)->len);
	ck_assert_msg(bcm_copy_tlv(tlv3, cp3) == (uint8 *)(cp3 + BCM_TLV_HDR_SIZE +
		((bcm_tlv_t *)tlv3)->len), "Error in copying third TLV");
	cp4 = (uint8 *)(cp3 + BCM_TLV_HDR_SIZE + ((bcm_tlv_t *)tlv3)->len);

	ck_assert_msg(bcm_copy_tlv(tlv3, NULL) == NULL, "Error in copying dummy TLV1");
	ck_assert_msg(bcm_copy_tlv(NULL, cp4) == cp4, "Error in copying dummy TLV2");
	free(cpbuf);
}
END_TEST

START_TEST(testBcmCopyTlvSafe)
{
	/* dst TLVs */
	uint8 *cpbuf, *cp2, *cp3, *cp4;
	cpbuf = (uint8 *)malloc(BUFFER_LEN);
	ck_assert_msg(cpbuf != NULL, "malloc error in testBcmCopyTlvSafe");
	memset(cpbuf, 0, BUFFER_LEN);

	ck_assert_msg(bcm_copy_tlv_safe(buffer, cpbuf, 5) == (cpbuf + BCM_TLV_HDR_SIZE +
		((bcm_tlv_t *)buffer)->len), "Error in copying first TLV");
	cp2 = (uint8 *)(cpbuf + BCM_TLV_HDR_SIZE + ((bcm_tlv_t *)buffer)->len);
	ck_assert_msg(bcm_copy_tlv_safe(tlv2, cp2, 10) == (cp2 + BCM_TLV_HDR_SIZE +
		((bcm_tlv_t *)tlv2)->len),	"Error in copying second TLV");
	cp3 = (uint8 *)(cp2 + BCM_TLV_HDR_SIZE + ((bcm_tlv_t *)tlv2)->len);
	ck_assert_msg(bcm_copy_tlv_safe(tlv3, cp3, 10) == (cp3 + BCM_TLV_HDR_SIZE +
		((bcm_tlv_t *)tlv3)->len),	"Error in copying third TLV");
	cp4 = (uint8 *)(cp3 + BCM_TLV_HDR_SIZE + ((bcm_tlv_t *)tlv3)->len);

	ck_assert_msg(bcm_copy_tlv_safe(tlv2, cp2, 5) == cp2, "Error in copying dummy TLV1");
	ck_assert_msg(bcm_copy_tlv_safe(tlv3, cp3, 3) == cp3, "Error in copying dummy TLV2");
	ck_assert_msg(bcm_copy_tlv_safe(tlv3, NULL, 10) == NULL, "Error in copying dummy TLV3");
	ck_assert_msg(bcm_copy_tlv_safe(NULL, cp4, 10) == cp4, "Error in copying dummy TLV4");
	free(cpbuf);
}
END_TEST

START_TEST(testBcmParseTlv)
{
	/* Parsing the tlvs
	 * Order of Parsing: second, first & third
	 */

	ck_assert_msg(bcm_parse_tlvs(buffer, BUFFER_LEN, TEST_TLV_SSID_ID) ==
		(bcm_tlv_t *)tlv2, "Error in parsing second TLV");
	ck_assert_msg(bcm_parse_tlvs(buffer, BUFFER_LEN, TEST_TLV_DS_PARMS_ID) ==
		(bcm_tlv_t *)buffer, "Error in parsing first TLV");
	ck_assert_msg(bcm_parse_tlvs(buffer, BUFFER_LEN, TEST_TLV_MAX_IDLE_PERIOD_ID) ==
		(bcm_tlv_t *)tlv3, "Error in parsing third TLV");
	ck_assert_msg(bcm_parse_tlvs(buffer, BUFFER_LEN, TEST_TLV_RATES_ID) ==
		(bcm_tlv_t *)tlv4, "Error in parsing fourth TLV");
	ck_assert_msg(bcm_parse_tlvs(buffer, BUFFER_LEN, TEST_TLV_HT_OBSS_ID) == NULL,
		"Error in parsing dummy TLV1");
	/* 1. buffer only has 1 byte. -> should return null */
	ck_assert_msg(bcm_parse_tlvs(buffer, 1, TEST_TLV_HT_OBSS_ID) == NULL,
		"Error in parsing dummy TLV2");
	ck_assert_msg(bcm_parse_tlvs(buffer, 1, TEST_TLV_SSID_ID) == NULL,
		"Error in parsing dummy TLV3");
	ck_assert_msg(bcm_parse_tlvs(tlv4, 1, TEST_TLV_RATES_ID) ==
		NULL, "Error in parsing dummy TLV4");

	/* 2. buffer has 2 bytes, and len is zero,
	 * should return the buffer pointer (zero len IE)
	 */
	ck_assert_msg(bcm_parse_tlvs(tlv4, 2, TEST_TLV_RATES_ID) ==
		(bcm_tlv_t *)tlv4, "Error in parsing zero len TLV");

	/* 3. buffer has 2 bytes, and len > 0, should return NULL */
	ck_assert_msg(bcm_parse_tlvs(buffer, 2, TEST_TLV_DS_PARMS_ID) == NULL,
		"Error in parsing dummy TLV5");

	/* 4. buffer has 2 bytes and N data bytes, but len is N+1. */
	ck_assert_msg(bcm_parse_tlvs(tlv2, 8, TEST_TLV_SSID_ID) == NULL,
		"Error in parsing dummy TLV6");

	ck_assert_msg(bcm_parse_tlvs(tlv5, 151, TEST_TLV_ID5) ==
		NULL, "Error in parsing dummy TLV7");
	ck_assert_msg(bcm_parse_tlvs(tlv5, 152, TEST_TLV_ID5) ==
		(bcm_tlv_t *)tlv5, "Error in parsing fifth TLV");
}
END_TEST

START_TEST(testBcmParseTlvExhaustBufLen3IEs)
{
	int buflen, result = 0;
	uint8 *p;
	ie_a = buffer;

	/* for each size of the first IE ... */
	for (alen = 0; alen < 256; alen++) {

		ie_b = bcm_write_tlv(TEST_TLV_ID_A, a_data, alen, buffer);

		/* for each size of the second IE ... */
		for (blen = 0; blen < 256; blen++) {

			ie_c = bcm_write_tlv(TEST_TLV_ID_B, b_data, blen, ie_b);

			/* for each size of the third IE ... */
			for (clen = 0; clen < 256; clen++) {

				p = bcm_write_tlv(TEST_TLV_ID_C, c_data, clen, ie_c);

				/* calculate the correct length for all IEs */
				buflen = p - buffer;

				/* run a set of parsing tests with the buffer length
				 * truncated by 1
				 */
				if (testExhaustBufLen3IEs(buffer, buflen - 1) != 0) {
					result = 1;
					ck_assert_msg(result == 0, "Error in verifying exhaustive"
						" buffer length test for 3IEs");
				}

				/* run a set of parsing tests with the buffer length truncated
				 * to all but 1 byte
				 */
				if (testExhaustBufLen3IEs(buffer, buflen - (clen + 1)) != 0) {
					result = 1;
					ck_assert_msg(result == 0, "Error in verifying exhaustive"
						" buffer length test for 3IEs");
				}

				/* run a set of parsing tests with the buffer length truncated
				 * properly to 2 IEs
				 */
				if (testExhaustBufLen3IEs(buffer, buflen - (clen + 2)) != 0) {
					result = 1;
					ck_assert_msg(result == 0, "Error in verifying exhaustive"
						" buffer length test for 3IEs");
				}

				if (clen == 12)
					clen += 241;
			}
		}
	}
	ck_assert_msg(result == 0, "Error in verifying exhaustive buffer length test for 3IEs");
}
END_TEST

START_TEST(testBcmParseTlvExhaustBufLen2IEs)
{
	int buflen, i, result = 0;
	uint8 *p;
	ie_a = buffer;

	/* for each size of the first IE ... */
	for (alen = 0; alen < 256; alen++) {

		ie_b = bcm_write_tlv(TEST_TLV_ID_A, a_data, alen, buffer);

		/* for each size of the second IE ... */
		for (blen = 0; blen < 256; blen++) {

			p = bcm_write_tlv(TEST_TLV_ID_B, b_data, blen, ie_b);

			/* calculate the correct length for all IEs */
			buflen = p - buffer;

			for (i = 1; i <= (blen + 2); i++) {
				/* run a set of parsing tests with the buffer length
				 * truncated by 1
				 */
				if (testExhaustBufLen2IEs(buffer, buflen - i) != 0) {
					result = 1;
					ck_assert_msg(result == 0, "Error in verifying exhaustive"
						" buffer length test for 2IEs");
				}
			}
		}
	}

	ck_assert_msg(result == 0, "Error in verifying exhaustive buffer length test for 2IEs");
}
END_TEST

START_TEST(testBcmParseOrderedTlv)
{
	ck_assert_msg(bcm_parse_ordered_tlvs(buffer, BUFFER_LEN, TEST_TLV_SSID_ID) ==
		(bcm_tlv_t *)buffer, "Error in parsing first TLV");
	ck_assert_msg(bcm_parse_ordered_tlvs(buffer, BUFFER_LEN, TEST_TLV_RATES_ID) ==
		(bcm_tlv_t *)tlv2, "Error in parsing second TLV");
	ck_assert_msg(bcm_parse_ordered_tlvs(buffer, BUFFER_LEN, TEST_TLV_DS_PARMS_ID) ==
		(bcm_tlv_t *)tlv3, "Error in parsing third TLV");
	ck_assert_msg(bcm_parse_ordered_tlvs(buffer, BUFFER_LEN, TEST_TLV_MAX_IDLE_PERIOD_ID)
		== (bcm_tlv_t *)tlv4, "Error in parsing fourth TLV");
	ck_assert_msg(bcm_parse_ordered_tlvs(buffer, BUFFER_LEN, TEST_TLV_HT_OBSS_ID) == NULL,
		"Error in parsing dummy TLV1");
	ck_assert_msg(bcm_parse_ordered_tlvs(buffer, BUFFER_LEN, TEST_TLV_FH_PARMS_ID) == NULL,
		"Error in parsing dummy TLV2");
	ck_assert_msg(bcm_parse_ordered_tlvs(buffer, BUFFER_LEN, TEST_TLV_VHT_CAP_ID) == NULL,
		"Error in parsing dummy TLV3");
	ck_assert_msg(bcm_parse_ordered_tlvs(buffer, 1, TEST_TLV_SSID_ID) == NULL,
		"Error in parsing dummy TLV4");
	ck_assert_msg(bcm_parse_ordered_tlvs(buffer, 2, TEST_TLV_SSID_ID) == NULL,
		"Error in parsing dummy TLV5");

}
END_TEST

START_TEST(testBcmParseTlvsMinBodylen)
{
	ck_assert_msg(bcm_parse_tlvs_min_bodylen(buffer, BUFFER_LEN, TEST_TLV_SSID_ID, 7) ==
		(bcm_tlv_t *)tlv2, "Error in parsing second TLV");
	ck_assert_msg(bcm_parse_tlvs_min_bodylen(buffer, BUFFER_LEN, TEST_TLV_DS_PARMS_ID, 1) ==
		(bcm_tlv_t *)buffer, "Error in parsing first TLV");
	ck_assert_msg(bcm_parse_tlvs_min_bodylen(buffer, BUFFER_LEN, TEST_TLV_MAX_IDLE_PERIOD_ID,
		2) == (bcm_tlv_t *)tlv3, "Error in parsing third TLV");
	ck_assert_msg(bcm_parse_tlvs_min_bodylen(buffer, BUFFER_LEN, TEST_TLV_HT_OBSS_ID, 5) ==
		NULL, "Error in parsing dummy TLV1");
	ck_assert_msg(bcm_parse_tlvs_min_bodylen(buffer, BUFFER_LEN, TEST_TLV_SSID_ID, 10) ==
		NULL, "Error in parsing dummy TLV2");

}
END_TEST

START_TEST(testBcmNextTlv)
{
	int buf_len = BUFFER_LEN;

	ck_assert_msg(bcm_next_tlv((bcm_tlv_t *)buffer, &buf_len) == (bcm_tlv_t *)tlv2,
		"Error in parsing first TLV");
	ck_assert_msg(bcm_next_tlv((bcm_tlv_t *)tlv2, &buf_len) == (bcm_tlv_t *)tlv3,
		"Error in parsing second TLV");
	ck_assert_msg(bcm_next_tlv((bcm_tlv_t *)tlv3, &buf_len) == (bcm_tlv_t *)tlv4,
		"Error in parsing third TLV");
	ck_assert_msg(bcm_next_tlv((bcm_tlv_t *)tlv4, &buf_len) == NULL,
		"Error in parsing dummy TLV1");
	ck_assert_msg(bcm_next_tlv((bcm_tlv_t *)buffer, &buf_len) == NULL,
		"Error in parsing dummy TLV2");
}
END_TEST

START_TEST(testTlvData)
{
	bcm_tlv_t *dtlv1, *dtlv2, *dtlv3, *dtlv5;
	uint8 *ptr;
	int i;
	dtlv1 = bcm_parse_tlvs(buffer, BUFFER_LEN, TEST_TLV_SSID_ID);
	ck_assert_msg(dtlv1->len == strlen("brcmwpa"),
		"Error in validating first TLV data length");
	ck_assert_msg(memcmp("brcmwpa", dtlv1->data, dtlv1->len) == 0,
		"Error in validating first TLV data");
	dtlv2 = bcm_parse_tlvs(buffer, BUFFER_LEN, TEST_TLV_DS_PARMS_ID);
	ck_assert_msg(dtlv2->len == 1, "Error in validating second TLV data length");
	ck_assert_msg(*(dtlv2->data) == 165, "Error in validating second TLV data");
	dtlv3 = bcm_parse_tlvs(buffer, BUFFER_LEN, TEST_TLV_MAX_IDLE_PERIOD_ID);
	ck_assert_msg(dtlv3->len == TEST_TLV_MAX_IDLE_PERIOD_IE_LEN,
		"Error in validating third TLV data length");
	ck_assert_msg(*(dtlv3->data) == 0, "Error in validating third TLV data");
	dtlv5 = bcm_parse_tlvs(buffer, (BUFFER_LEN + 153), TEST_TLV_ID5);
	ck_assert_msg(dtlv5->len == TEST_TLV_ID5_LEN,
		"Error in validating fifth TLV data length");
	ptr = dtlv5->data;
	for (i = 0; i < dtlv5->len; i++)
		ck_assert_msg(ptr[i] == tlv5_data[i], "Error in validating fifth TLV data "
			"element:%d", i);
}
END_TEST

START_TEST(testBcmFindVendorIE)
{
	/* Parsing the tlvs
	 * Order of Parsing: second, first & third
	 */
	int buflen = BUFFER_LEN + 12;
	uint8 type = 213;
	/* Parsing valid Vendor TLVs */
	ck_assert_msg(bcm_find_vendor_ie(buffer, buflen, TEST_PROP_OUI1, &type1,
		sizeof(type1)) == (bcm_tlv_t *)buffer, "Error in parsing first vendor TLV");
	ck_assert_msg(bcm_find_vendor_ie(buffer, buflen, TEST_PROP_OUI2, &type2,
		sizeof(type2)) == (bcm_tlv_t *)vtlv2, "Error in parsing second vendor TLV");
	ck_assert_msg(bcm_find_vendor_ie(buffer, buflen, TEST_PROP_OUI3, type3,
		sizeof(type3)) == (bcm_tlv_t *)vtlv3, "Error in parsing third vendor TLV");
	ck_assert_msg(bcm_find_vendor_ie(buffer, buflen, TEST_PROP_OUI4, 0,
		0) == (bcm_tlv_t *)vtlv4, "Error in parsing fourth vendor TLV");
	/* Should return NULL as there is no TLV with TEST_PROP_OUI6 as voui */
	ck_assert_msg(bcm_find_vendor_ie(buffer, buflen, TEST_PROP_OUI6, type3,
		sizeof(type3)) == NULL, "Error in parsing dummy TLV1");
	/* Should return NULL as ie argument is NULL */
	ck_assert_msg(bcm_find_vendor_ie(NULL, buflen, TEST_PROP_OUI1, &type1,
		sizeof(type1)) == NULL, "Error in parsing dummy TLV3");
	/* Should return NULL as ie(buffer) is invalid tlv */
	ck_assert_msg(bcm_find_vendor_ie(buffer, 5, TEST_PROP_OUI1, &type1,
		sizeof(type1)) == NULL, "Error in parsing dummy TLV4");
	/* Should return NULL as voui is not having DOT11_MNG_PROPR_ID as ie->id */
	ck_assert_msg(bcm_find_vendor_ie(buffer, buflen, TEST_PROP_OUI5, &type1,
		sizeof(type1)) == NULL, "Error in parsing dummy TLV5");
	/* Should return NULL as ie->len is < (DOT11_OUI_LEN + type_len) */
	ck_assert_msg(bcm_find_vendor_ie(buffer, buflen, TEST_PROP_OUI1, &type1,
		4) == NULL, "Error in parsing dummy TLV6"); /* Look for fn flow */
	/* Should return NULL as type is not matching */
	ck_assert_msg(bcm_find_vendor_ie(buffer, buflen, TEST_PROP_OUI3, &type,
		sizeof(type)) == NULL, "Error in parsing dummy TLV7");
	ck_assert_msg(bcm_find_vendor_ie(buffer, buflen, TEST_PROP_OUI1, &type2,
		sizeof(type2)) == NULL, "Error in parsing dummy TLV8");
}
END_TEST

START_TEST(testBcmFindVendorExhaustBufLen3IEs)
{
	int buflen, result = 0;
	uint8 *p;
	ie_a = buffer;

	/* for each size of the first IE ... */
	for (alen = 3; alen < 256; alen++) {

		ie_b = bcm_write_tlv(DOT11_MNG_PROPR_ID, a_data, alen, buffer);

		/* for each size of the second IE ... */
		for (blen = 3; blen < 256; blen++) {

			ie_c = bcm_write_tlv(DOT11_MNG_PROPR_ID, b_data, blen, ie_b);

			/* for each size of the third IE ... */
			for (clen = 3; clen < 256; clen++) {

				p = bcm_write_tlv(DOT11_MNG_PROPR_ID, c_data, clen, ie_c);

				/* calculate the correct length for all IEs */
				buflen = p - buffer;

				/* run a set of parsing tests with the buffer length
				 * truncated by 1
				 */
				if (testExhaustBufLen3VendorIEs(buffer, buflen - 1) != 0) {
					result = 1;
					ck_assert_msg(result == 0, "Error in verifying exhaustive"
						" buffer length test for 3VenIEs");
				}

				/* run a set of parsing tests with the buffer length truncated
				 * to all but 1 byte
				 */
				if (testExhaustBufLen3VendorIEs(buffer, buflen -
					(clen + 1)) != 0) {
					result = 1;
					ck_assert_msg(result == 0, "Error in verifying exhaustive"
						" buffer length test for 3VenIEs");
				}

				/* run a set of parsing tests with the buffer length truncated
				 * properly to 2 IEs
				 */
				if (testExhaustBufLen3VendorIEs(buffer, buflen -
					(clen + 2)) != 0) {
					result = 1;
					ck_assert_msg(result == 0, "Error in verifying exhaustive"
						" buffer length test for 3VenIEs");
				}

				if (clen == 15)
					clen += 238;
			}
		}
	}
	ck_assert_msg(result == 0, "Error in verifying exhaustive buffer length test for 3VenIEs");
}
END_TEST

START_TEST(testBcmFindVendorExhaustBufLen2IEs)
{
	int buflen, i, result = 0;
	uint8 *p;
	ie_a = buffer;

	/* for each size of the first IE ... */
	for (alen = 3; alen < 256; alen++) {

		ie_b = bcm_write_tlv(DOT11_MNG_PROPR_ID, a_data, alen, buffer);

		/* for each size of the second IE ... */
		for (blen = 3; blen < 256; blen++) {

			p = bcm_write_tlv(DOT11_MNG_PROPR_ID, b_data, blen, ie_b);

			buflen = p - buffer;

			for (i = 1; i <= (blen + 2); i++) {
				/* run a set of parsing tests with the buffer length
				 * truncated by 1
				 */
				if (testExhaustBufLen2VendorIEs(buffer, buflen - i) != 0) {
					result = 1;
					ck_assert_msg(result == 0, "Error in verifying exhaustive"
						" buffer length test for 2VenIEs");
				}
			}
		}
	}
	ck_assert_msg(result == 0, "Error in verifying exhaustive buffer length test for 2VenIEs");
}
END_TEST

/* -------------------TEST SUITES--------------------- */

Suite *bcmwritetlv_suite(void)
{
	Suite *s = suite_create("bcmwritetlv_suite");
	TCase *tc_writetlv = tcase_create("Test Case");
	tcase_add_checked_fixture(tc_writetlv, WriteTlv_Setup, WriteTlv_Teardown);
	tcase_add_test(tc_writetlv, testBcmWriteTlv);
	tcase_add_test(tc_writetlv, testBcmWriteTlvSafe);

	suite_add_tcase(s, tc_writetlv);

	return s;
}

Suite *bcmcopytlv_suite(void)
{
	Suite *s = suite_create("bcmcopytlv_suite");
	TCase *tc_copytlv = tcase_create("Test Case");
	tcase_add_checked_fixture(tc_copytlv, ParseTlv_Setup, ParseTlv_Teardown);
	tcase_add_test(tc_copytlv, testBcmCopyTlv);
	tcase_add_test(tc_copytlv, testBcmCopyTlvSafe);

	suite_add_tcase(s, tc_copytlv);

	return s;
}

Suite *bcmparsetlv_suite(void)
{
	Suite *s = suite_create("bcmparsetlv_suite");
	TCase *tc_parsetlv = tcase_create("Test Case");
	tcase_add_checked_fixture(tc_parsetlv, ParseTlv_Setup, ParseTlv_Teardown);
	tcase_add_test(tc_parsetlv, testBcmParseTlv);
	tcase_add_test(tc_parsetlv, testBcmParseTlvsMinBodylen);
	tcase_add_test(tc_parsetlv, testTlvData);

	TCase *tc_parseordertlv = tcase_create("Test Case");
	tcase_add_checked_fixture(tc_parseordertlv, ParseOrderedTlv_Setup,
		ParseOrderedTlv_Teardown);
	tcase_add_test(tc_parseordertlv, testBcmParseOrderedTlv);
	tcase_add_test(tc_parseordertlv, testBcmNextTlv);

	TCase *tc_exbuflentlv3 = tcase_create("Test Case");
	tcase_add_checked_fixture(tc_exbuflentlv3, BufferLen_Setup3IEs,
		BufferLen_Teardown3IEs);
	tcase_add_test(tc_exbuflentlv3, testBcmParseTlvExhaustBufLen3IEs);

	TCase *tc_exbuflentlv2 = tcase_create("Test Case");
	tcase_add_checked_fixture(tc_exbuflentlv2, BufferLen_Setup2IEs,
		BufferLen_Teardown2IEs);
	tcase_add_test(tc_exbuflentlv2, testBcmParseTlvExhaustBufLen2IEs);

	TCase *tc_findvendorie = tcase_create("Test Case");
	tcase_add_checked_fixture(tc_findvendorie, VendorIE_Setup,
		VendorIE_Teardown);
	tcase_add_test(tc_findvendorie, testBcmFindVendorIE);

	TCase *tc_exbuflenven3 = tcase_create("Test Case");
	tcase_add_checked_fixture(tc_exbuflenven3, BufferLen_Setup3VenIEs,
		BufferLen_Teardown3VenIEs);
	tcase_add_test(tc_exbuflenven3, testBcmFindVendorExhaustBufLen3IEs);

	TCase *tc_exbuflenven2 = tcase_create("Test Case");
	tcase_add_checked_fixture(tc_exbuflenven2, BufferLen_Setup2VenIEs,
		BufferLen_Teardown2VenIEs);
	tcase_add_test(tc_exbuflenven2, testBcmFindVendorExhaustBufLen2IEs);

	suite_add_tcase(s, tc_parsetlv);
	suite_add_tcase(s, tc_parseordertlv);
	suite_add_tcase(s, tc_exbuflentlv3);
	suite_add_tcase(s, tc_exbuflentlv2);
	suite_add_tcase(s, tc_findvendorie);
	suite_add_tcase(s, tc_exbuflenven3);
	suite_add_tcase(s, tc_exbuflenven2);

	return s;
}

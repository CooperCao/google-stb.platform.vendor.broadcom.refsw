/* mfgapi_test.cpp : Test harness to test WLM (Wireless LAN Manufacturing) test library.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: mfgapi_test.cpp,v 1.7 2009-10-02 21:38:14 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "wlm.h"

#include "wlioctl.h"

/* --------------------------------------------------------------- */
typedef struct
{
	int count;		/* number of tests run */
	int passed;		/* number of tests passed */
	int failed;		/* number of tests failed */
} TestLog;

static TestLog testLog;

#define TEST_INITIALIZE()						\
{												\
	memset(&testLog, 0, sizeof(testLog));		\
}

#define TEST(condition, error)					\
	testLog.count++;							\
	if ((condition)) {							\
		testLog.passed++;						\
	}											\
	else {										\
		testLog.failed++;						\
		printf("\n*** FAIL *** - %s():%d - %s\n\n",	\
			__FUNCTION__, __LINE__, error);		\
	}

#define TEST_FATAL(condition, error)			\
	testLog.count++;							\
	if ((condition)) {							\
		testLog.passed++;						\
	}											\
	else {										\
		testLog.failed++;						\
		printf("\n*** FAIL *** - %s():%d - %s\n\n",	\
			__FUNCTION__, __LINE__, error);		\
		exit(-1);								\
	}

#define TEST_FINALIZE()							\
{												\
	int percent = testLog.count ?				\
		testLog.passed * 100 / testLog.count : 0; \
	printf("\n\n");								\
	printf("Test Summary:\n\n");				\
	printf("Tests    %d\n", testLog.count);		\
	printf("Pass     %d\n", testLog.passed);	\
	printf("Fail     %d\n\n", testLog.failed);	\
	printf("%d%%\n\n", percent);				\
	printf("-----------------------------------\n\n");	\
}

/* --------------------------------------------------------------- */
static void delay(unsigned int msec)
{
	clock_t start_tick = clock();
	clock_t end_tick = start_tick + msec * CLOCKS_PER_SEC / 1000;

	while (clock() < end_tick) {
		/* do nothing */
	}
}

static void testSelectInterface()
{
	TEST(!wlmSelectInterface(WLM_DUT_SOCKET, "1921681",
		WLM_DEFAULT_DUT_SERVER_PORT, WLM_DUT_OS_LINUX), "invalid IP failed");
	TEST(!wlmSelectInterface(WLM_DUT_SOCKET, "192.168.1",
		WLM_DEFAULT_DUT_SERVER_PORT, WLM_DUT_OS_LINUX), "invalid IP failed");
	TEST(!wlmSelectInterface(WLM_DUT_WIFI, "0011223344",
		WLM_DEFAULT_DUT_SERVER_PORT, WLM_DUT_OS_LINUX), "invalid MAC failed");
	TEST(!wlmSelectInterface(WLM_DUT_WIFI, "00:11:22:33:44",
		WLM_DEFAULT_DUT_SERVER_PORT, WLM_DUT_OS_LINUX), "invalid MAC failed");
}

static void testDutInit()
{
	TEST(wlmMinPowerConsumption(TRUE), "unable disable sleep mode");
	TEST(wlmCountryCodeSet(WLM_COUNTRY_ALL), "unable to set country code");
	TEST(wlmGlacialTimerSet(99999), "unable to set glacial timer");
	TEST(wlmFastTimerSet(99999), "unable to set fast timer");
	TEST(wlmSlowTimerSet(99999), "unable to set slow timer");
	TEST(wlmScanSuppress(1), "unable to set scansuppress");
}

static void testVersion()
{
	char buffer[1024];
	TEST(wlmVersionGet(buffer, 1024), "wlmVersionGet failed");
	printf("version info: %s\n", buffer);
}

static void testEnableAdapter()
{
	int up;

	printf("Testing enabling adapter...\n");
	TEST(wlmEnableAdapterUp(1), "wlmEnableAdapterUp failed enabling adapter");
	TEST(wlmIsAdapterUp(&up), "wlmIsAdapterUp failed retrieve adapter state");
	if (!up) {
		printf("Failed to bring up adapter\n");
		exit(-1);
	}
}

static void testDisableAdapter()
{
	int up;

	printf("Testing disabling adapter...\n");
	TEST(wlmEnableAdapterUp(0), "wlmEnableAdapterUp failed disabling adapter");
	TEST(wlmIsAdapterUp(&up), "wlmIsAdapterUp failed retrieve adapter state");
	TEST(!up, "Failed to bring down adapter");
	TEST(wlmEnableAdapterUp(1), "wlmEnableAdapterUp failed abling adapter");}

static void testMacAddr()
{
	char macAddrBuf[] = "00:11:22:33:44:55";

	printf("\nTesting mac address setting...\n");
	TEST(wlmMacAddrGet(macAddrBuf, sizeof(macAddrBuf)), "wlmMacAddrGet failed");
	printf("\tMAC address = %s\n", macAddrBuf);
	if (!wlmEnableAdapterUp(0))
		printf("Failed to disable adpter for writing MAC address.\n");
	else {
		TEST(wlmMacAddrSet(macAddrBuf), "wlmMacAddrSet failed");
		if (!wlmEnableAdapterUp(1))
			printf("Failed to enable adapter after writing MAC address.\n");
	}
}

static void testBandList()
{
	WLM_BAND bands;

	printf("Testing getting band list...\n");
	TEST(wlmGetBandList(&bands), "wlmGetBandList failed");
	switch (bands) {
		case WLM_BAND_2G:
			printf("\tAvailable bands = 2.4GHz.\n");
			break;
		case WLM_BAND_5G:
			printf("\tAvailable bands = 5GHz.\n");
			break;
		case WLM_BAND_DUAL:
			printf("\tAvailable bands = 2.4GHz and 5GHz.\n");
			break;
		default:
			printf("\tAvailable bands = None.\n");
		break;
	}
}

static void testBand()
{
	printf("Testing band setting...\n");
	TEST(wlmBandSet(WLM_BAND_2G), "wlmBandSet failed");
	printf("\tSetting band to 2.4G\n");
}

static void testGmode()
{
	printf("Testing gmode setting...\n");
	TEST(wlmGmodeSet(WLM_GMODE_GONLY), "wlmGmodeSet failed");
	printf("\tSetting gmode to G only\n");
}


static void testChannel()
{
	printf("Testing channel setting...\n");
	TEST(wlmChannelSet(6), "wlmChannelSet failed");
	printf("\tSetting channel to 6\n");
	/* change channel back to default */
	TEST(wlmChannelSet(1), "wlmChannelSet failed");}

static void testRate()
{
	printf("Testing rate setting...\n");
	TEST(wlmRateSet(WLM_RATE_54M), "wlmRateSet failed");
	printf("\tSetting rate to 54Mbps\n");
	/* change rate back to default */
	TEST(wlmRateSet(WLM_RATE_AUTO), "wlmRateSet failed");
}

static void testSsidGet()
{
	char ssidBuf[128+1];

	printf("Testing SSID getting...\n");
	TEST(wlmSsidGet(ssidBuf, sizeof(ssidBuf)), "wlmSsidGet failed");
	printf("\tSSID = %s\n", ssidBuf);
}

static void testWepSecurity()
{
	char key[512 + 1];
	int length;
	char save;

	for (int i = 0; i < 512; i++) {
		int hex = i % 16;
		key[i] = hex <= 9 ? '0' + hex : 'A' + (hex - 10);
	}
	key[512] = 0;

	/* test null key */
	length = 40;
	save = key[length];
	key[length] = 0;
	TEST(!wlmSecuritySet(WLM_TYPE_OPEN, WLM_WPA_AUTH_DISABLED, WLM_ENCRYPT_WEP, 0),
		"wlmSecuritySet null key failed");
	key[length] = save;

	/* test valid WEP key lengths */
	length = 40;
	save = key[length];
	key[length] = 0;
	TEST(wlmSecuritySet(WLM_TYPE_OPEN, WLM_WPA_AUTH_DISABLED, WLM_ENCRYPT_WEP, key),
		"wlmSecuritySet length 40 failed");
	key[length] = save;
	length = 104;
	save = key[length];
	key[length] = 0;
	TEST(wlmSecuritySet(WLM_TYPE_OPEN, WLM_WPA_AUTH_DISABLED, WLM_ENCRYPT_WEP, key),
		"wlmSecuritySet length 104 failed");
	key[length] = save;
	length = 128;
	save = key[length];
	key[length] = 0;
	TEST(wlmSecuritySet(WLM_TYPE_OPEN, WLM_WPA_AUTH_DISABLED, WLM_ENCRYPT_WEP, key),
		"wlmSecuritySet length 128 failed");
	key[length] = save;
	length = 256;
	save = key[length];
	key[length] = 0;
	TEST(wlmSecuritySet(WLM_TYPE_OPEN, WLM_WPA_AUTH_DISABLED, WLM_ENCRYPT_WEP, key),
		"wlmSecuritySet length 256 failed");
	key[length] = save;

	/* test invalid WEP key lengths */
	length = 39;
	save = key[length];
	key[length] = 0;
	TEST(!wlmSecuritySet(WLM_TYPE_OPEN, WLM_WPA_AUTH_DISABLED, WLM_ENCRYPT_WEP, key),
		"wlmSecuritySet invalid length failed");
	key[length] = save;
	length = 41;
	save = key[length];
	key[length] = 0;
	TEST(!wlmSecuritySet(WLM_TYPE_OPEN, WLM_WPA_AUTH_DISABLED, WLM_ENCRYPT_WEP, key),
		"wlmSecuritySet invalid length failed");
	key[length] = save;
	length = 103;
	save = key[length];
	key[length] = 0;
	TEST(!wlmSecuritySet(WLM_TYPE_OPEN, WLM_WPA_AUTH_DISABLED, WLM_ENCRYPT_WEP, key),
		"wlmSecuritySet invalid length failed");
	key[length] = save;
	length = 105;
	save = key[length];
	key[length] = 0;
	TEST(!wlmSecuritySet(WLM_TYPE_OPEN, WLM_WPA_AUTH_DISABLED, WLM_ENCRYPT_WEP, key),
		"wlmSecuritySet invalid length failed");
	key[length] = save;
	length = 127;
	save = key[length];
	key[length] = 0;
	TEST(!wlmSecuritySet(WLM_TYPE_OPEN, WLM_WPA_AUTH_DISABLED, WLM_ENCRYPT_WEP, key),
		"wlmSecuritySet invalid length failed");
	key[length] = save;
	length = 129;
	save = key[length];
	key[length] = 0;
	TEST(!wlmSecuritySet(WLM_TYPE_OPEN, WLM_WPA_AUTH_DISABLED, WLM_ENCRYPT_WEP, key),
		"wlmSecuritySet invalid length failed");
	key[length] = save;
	length = 255;
	save = key[length];
	key[length] = 0;
	TEST(!wlmSecuritySet(WLM_TYPE_OPEN, WLM_WPA_AUTH_DISABLED, WLM_ENCRYPT_WEP, key),
		"wlmSecuritySet invalid length failed");
	key[length] = save;
	length = 257;
	save = key[length];
	key[length] = 0;
	TEST(!wlmSecuritySet(WLM_TYPE_OPEN, WLM_WPA_AUTH_DISABLED, WLM_ENCRYPT_WEP, key),
		"wlmSecuritySet invalid length failed");
	key[length] = save;
	length = 512;
	save = key[length];
	key[length] = 0;
	TEST(!wlmSecuritySet(WLM_TYPE_OPEN, WLM_WPA_AUTH_DISABLED, WLM_ENCRYPT_WEP, key),
		"wlmSecuritySet invalid length failed");
	key[length] = save;

	/* test invalid WEP key format */
	for (int i = 0; i < 512; i++) {
		key[i] = 'A' + (i % 26);
	}
	key[512] = 0;

	length = 40;
	save = key[length];
	key[length] = 0;
	TEST(!wlmSecuritySet(WLM_TYPE_OPEN, WLM_WPA_AUTH_DISABLED, WLM_ENCRYPT_WEP, key),
		"wlmSecuritySet invalid WEP key format failed");
	key[length] = save;
}

static void testTkipAesSecurity()
{
	char key[512 + 1];
	int length;
	char save;

	for (int i = 0; i < 512; i++) {
		key[i] = 'A' + (i % 26);
	}
	key[512] = 0;

	/* test null key */
	length = 40;
	save = key[length];
	key[length] = 0;
	TEST(!wlmSecuritySet(WLM_TYPE_OPEN, WLM_WPA_AUTH_DISABLED, WLM_ENCRYPT_TKIP, 0),
		"wlmSecuritySet null key failed");

	/* test invalid TKIP/AES authentication modes */
	TEST(!wlmSecuritySet(WLM_TYPE_OPEN, WLM_WPA_AUTH_DISABLED, WLM_ENCRYPT_AES, key),
		"wlmSecuritySet invalid auth mode failed");
	TEST(!wlmSecuritySet(WLM_TYPE_OPEN, WLM_WPA_AUTH_NONE, WLM_ENCRYPT_AES, key),
		"wlmSecuritySet invalid auth mode failed");
	key[length] = save;

	/* test valid TKIP/AES key lengths */
	length = 8;
	save = key[length];
	key[length] = 0;
	TEST(wlmSecuritySet(WLM_TYPE_OPEN, WLM_WPA_AUTH_PSK, WLM_ENCRYPT_TKIP, key),
		"wlmSecuritySet length 8 failed");
	key[length] = save;
	length = 9;
	save = key[length];
	key[length] = 0;
	TEST(wlmSecuritySet(WLM_TYPE_OPEN, WLM_WPA_AUTH_PSK, WLM_ENCRYPT_TKIP, key),
		"wlmSecuritySet length 9 failed");
	key[length] = save;
	length = 63;
	save = key[length];
	key[length] = 0;
	TEST(wlmSecuritySet(WLM_TYPE_OPEN, WLM_WPA_AUTH_PSK, WLM_ENCRYPT_TKIP, key),
		"wlmSecuritySet length 63 failed");
	key[length] = save;

	/* test invalid TKIP/AES key lengths */
	key[length] = save;
	length = 7;
	save = key[length];
	key[length] = 0;
	TEST(!wlmSecuritySet(WLM_TYPE_OPEN, WLM_WPA2_AUTH_PSK, WLM_ENCRYPT_AES, key),
		"wlmSecuritySet invalid length failed");
	key[length] = save;
	length = 65;
	save = key[length];
	key[length] = 0;
	TEST(!wlmSecuritySet(WLM_TYPE_OPEN, WLM_WPA2_AUTH_PSK, WLM_ENCRYPT_AES, key),
		"wlmSecuritySet invalid length failed");
	key[length] = save;
	length = 104;
	save = key[length];
	key[length] = 0;
}

static int testJoinNetwork(char *ssid, WLM_AUTH_MODE authMode,
	WLM_ENCRYPTION encryption, const char *key)
{
	char bssidBuf[256];
	int isAssociated;
	char ssidBuf[256];

	/* make sure not currently associated */
	TEST(wlmDisassociateNetwork(), "wlmDisassociateNetwork failed");

	TEST(wlmSecuritySet(WLM_TYPE_OPEN, authMode, encryption, key),
		"wlmSecuritySet failed");
	TEST(wlmJoinNetwork(ssid, WLM_MODE_BSS), "wlmJoinNetwork failed");
	/* delay to allow network association */
	delay(5000);
	TEST(wlmBssidGet(bssidBuf, 256), "wlmBssidGet failed");
	isAssociated = strlen(bssidBuf) == 0 ? FALSE : TRUE;
	if (isAssociated) {
		TEST(wlmSsidGet(ssidBuf, 256), "wlmSsidGet failed");
		printf("associated to SSID=%s BSSID=%s\n", ssidBuf, bssidBuf);
		TEST(strcmp(ssid, ssidBuf) == 0, "SSID does not match");
	}
	else {
		printf("failed to associate to SSID=%s using key=%s\n", ssid, key ? key : "");
	}

	TEST(wlmDisassociateNetwork(), "wlmDisassociateNetwork failed");

	return isAssociated;
}

static void testJoinNetworkNone()
{
	/* requires an AP with the SSID, authentication, and encryption 
	 * configured to match these settings
	 */
	testJoinNetwork("WLM_NONE", WLM_WPA_AUTH_DISABLED, WLM_ENCRYPT_NONE, 0);
}

static void testJoinNetworkWep()
{
	/* requires an AP with the SSID, authentication, and encryption 
	 * configured to match these settings
	 */
	testJoinNetwork("WLM_WEP", WLM_WPA_AUTH_DISABLED, WLM_ENCRYPT_WEP,
		"2222222222444444444466666666668888888888");
}

static void testJoinNetworkWpaTkip()
{
	/* requires an AP with the SSID, authentication, and encryption 
	 * configured to match these settings
	 */
	testJoinNetwork("WLM_WPA_TKIP", WLM_WPA_AUTH_PSK, WLM_ENCRYPT_TKIP,
		"helloworld");
}

static void testJoinNetworkWpaAes()
{
	/* requires an AP with the SSID, authentication, and encryption 
	 * configured to match these settings
	 */
	testJoinNetwork("WLM_WPA_AES", WLM_WPA_AUTH_PSK, WLM_ENCRYPT_AES,
		"helloworld");
}

static void testJoinNetworkWpa2Tkip()
{
	/* requires an AP with the SSID, authentication, and encryption 
	 * configured to match these settings
	 */
	testJoinNetwork("WLM_WPA2_TKIP", WLM_WPA2_AUTH_PSK, WLM_ENCRYPT_TKIP,
		"helloworld");
}

static void testJoinNetworkWpa2Aes()
{
	/* requires an AP with the SSID, authentication, and encryption 
	 * configured to match these settings
	 */
	testJoinNetwork("WLM_WPA2_AES", WLM_WPA2_AUTH_PSK, WLM_ENCRYPT_AES,
		"helloworld");
}

static void testAntenna()
{
	printf("Testing antenna setting...\n");
	TEST(wlmRxAntSet(0), "wlmRxAntSet failed");
	printf("\tChoose antenna 0 to receive\n");
	TEST(wlmTxAntSet(0), "wlmTxAntSet failed");
	printf("\tChoose antenna 0 to transmit\n");
}

static void testPaParameters()
{
	unsigned int a1 = 0, b0 = 0, b1 = 0;

	printf("Testing reading PA parameters..\n");
	TEST(wlmPaParametersGet(WLM_LPPHY_2G, &a1, &b0, &b1), "wlmPaParametersGet failed");
	printf("\tPA parameters a1 = %d, b0 = %d, b1=%d\n", a1, b0, b1);
	if (!wlmEnableAdapterUp(0))
		printf("Failed to disable adapter for setting PA parameters.\n");
	else {
		TEST(wlmPaParametersSet(WLM_LPPHY_2G, a1, b0, b1), "wlmPaParametersSet failed");
		if (!wlmEnableAdapterUp(1))
			printf("Failed to enable adapter after setting PA parameters.\n");

	}
}

static void testPreamble()
{
	TEST(wlmPreambleSet(WLM_PREAMBLE_LONG), "wlmPreambleSet failed");
	TEST(wlmPreambleSet(WLM_PREAMBLE_SHORT), "wlmPreambleSet failed");
	TEST(wlmPreambleSet(WLM_PREAMBLE_AUTO), "wlmPreambleSet failed");
	/* set back to default */
	TEST(wlmPreambleSet(WLM_PREAMBLE_SHORT), "wlmPreambleSet failed");
}

static void testTransmit()
{
	unsigned int beforeAckCount;
	TEST(wlmTxGetAckedPackets(&beforeAckCount), "wlmTxGetAckedPackets failed");

	unsigned int txPacketCount = 50;
	TEST(wlmTxPacketStart(100, txPacketCount, 1024, "00:11:22:33:44:55", TRUE, TRUE),
		"wlmTxPacketStart failed");

	unsigned int afterAckCount = beforeAckCount;
	TEST(wlmTxGetAckedPackets(&afterAckCount), "wlmTxGetAckedPackets failed");

	unsigned int ackCount = afterAckCount - beforeAckCount;
	printf("Packets tx=%d, ack=%d\n", txPacketCount, ackCount);

	TEST(wlmTxPacketStop(), "wlmTxPacketStop failed");
}

static void testReceive()
{
	unsigned int beforeRxPacketCount;
	TEST(wlmRxGetReceivedPackets(&beforeRxPacketCount), "wlmRxGetReceivedPackets failed");
	TEST(wlmRxPacketStart("00:11:22:33:44:55",
		FALSE, TRUE, 100, 500), "wlmRxPacketStart failed");

	unsigned int afterRxPacketCount = beforeRxPacketCount;
	TEST(wlmRxGetReceivedPackets(&afterRxPacketCount), "wlmRxGetReceivedPackets failed");

	unsigned int rxPacketCount = afterRxPacketCount - beforeRxPacketCount;
	printf("Packets rx=%d\n", rxPacketCount);

	int rssi = 0;
	TEST(wlmRssiGet(&rssi), "wlmRssiGet failed");
	printf("RSSI=%d\n", rssi);

	TEST(wlmRxPacketStop(), "wlmRxPacketStop failed");
}

static void testSequence(bool clientBatching)
{
	unsigned int beforeAckCount;
	TEST(wlmTxGetAckedPackets(&beforeAckCount), "wlmTxGetAckedPackets failed");

	TEST(wlmSequenceStart(clientBatching), "wlmSequenceStart failed");

	unsigned int txPacketCount = 50;
	int count = 3;
	for (int i = 0; i < count; i++) {
		TEST(wlmTxPacketStart(100, txPacketCount, 1024, "00:11:22:33:44:55", TRUE, TRUE),
			"wlmTxPacketStart failed");

		TEST(wlmSequenceDelay(50), "wlmSequenceDelay failed");
	}

	TEST(wlmTxPacketStop(), "wlmTxPacketStop failed");
	TEST(wlmSequenceStop(), "wlmSequenceStop failed");

	int index;
	TEST(wlmSequenceErrorIndex(&index), "wlmSequenceErrorIndex failed");
	printf("Sequence error index=%d\n", index);

	unsigned int afterAckCount = beforeAckCount;
	TEST(wlmTxGetAckedPackets(&afterAckCount), "wlmTxGetAckedPackets failed");

	unsigned int ackCount = afterAckCount - beforeAckCount;
	printf("Packets tx=%d, ack=%d\n", count * txPacketCount, ackCount);
}

static void testTxPower()
{
	int power;
	printf("Testing TX power setting...\n");
	TEST(wlmTxPowerSet(10000),  "wlmTxPowerSet failed");
	TEST(wlmTxPowerGet(&power),  "wlmTxPowerGet failed");
	TEST(power == 10000, "TxPower not set correctly");
	TEST(wlmTxPowerSet(-1),  "wlmTxPowerSet failed");
	TEST(wlmTxPowerGet(&power),  "wlmTxPowerGet failed");
	TEST(power == 31750, "TxPower not set correctly");
}

static void testEstimatedPower()
{
	int estPower;
	printf("Testing estimated power...\n");
	TEST(wlmEstimatedPowerGet(&estPower, 0),  "wlmEstimatedPowerGet failed");
	printf("\tEstimated power: %d\n", estPower);
}

static void testImageAccess()
{
	const int BUFFER_SIZE = 1024;
	char buffer[BUFFER_SIZE];
	unsigned int numBytesRead = 0;
	printf("Testing Device Image Access...\n");
	TEST(numBytesRead = wlmDeviceImageRead(buffer, BUFFER_SIZE, WLM_TYPE_SROM),
    	"wlmDeviceImageRead failed on SROM case");
	if (numBytesRead) {
		printf("\tFirst 8 bytes of SROM:\n\t");
		for (int i = 0; i < 8; i++) {
			printf("0x%X ", buffer[i] & 0xff);
		}
		printf("\n");

		/* commented out to prevent any accidental overwrite of SROM;
		 * should test this functionality periodically to ensure it doesn't break
		 */
		/*
		TEST(wlmDeviceImageWrite(buffer, 2, WLM_TYPE_SROM),
			 "wlmDeviceImageWrite failed on SROM case");
		*/
	}
	TEST(numBytesRead = wlmDeviceImageRead(buffer, BUFFER_SIZE, WLM_TYPE_OTP),
		"wlmDeviceImageRead fail d on OTP case");
	if (numBytesRead) {
		printf("\tFirst 8 bytes of OTP:\n\t");
		for (int i = 0; i < 8; i++) {
			printf("0x%X ", buffer[i] & 0xff);
		}
		printf("\n");

		/* uncomment to write to the OTP */
		/*
		memset(buffer, 0, 2);
		TEST(wlmDeviceImageWrite(buffer, 2, WLM_TYPE_OTP),
			 "wlmDeviceImageWrite failed on OTP case");
		*/
	}

	TEST(numBytesRead = wlmDeviceImageRead(buffer, BUFFER_SIZE, WLM_TYPE_AUTO),
		"wlmDeviceImageRead failed on AUTO case");
	if (numBytesRead) {
		printf("\tFirst 8 bytes of Auto:\n\t");
		for (int i = 0; i < 8; i++) {
			printf("0x%X ", buffer[i] & 0xff);
		}
		printf("\n");
	}

	TEST(numBytesRead = wlmDeviceImageRead(buffer, 0, WLM_TYPE_SROM) == 0,
		"wlmDeviceImageRead did not fail when buffer too small");
	TEST(numBytesRead = wlmDeviceImageRead(NULL, BUFFER_SIZE, WLM_TYPE_SROM) == 0,
		"wlmDeviceImageRead did not fail when buffer was NULL");
}

static void testCarrierTone()
{
	printf("Testing Carrier Tone Set...\n");
	TEST(wlmEnableCarrierTone(1, WLM_BAND_2G, 7),
		"wlmEnableCarrierTone failed for enable case");
	TEST(wlmEnableCarrierTone(0, WLM_BAND_2G, 0),
		"wlmEnableCarrierTone failed for disable case");
}

static void testEvmTest()
{
	printf("Testing EVM Test Set...\n");
	TEST(wlmEnableEVMTest(1, WLM_RATE_1M, 7),
		"wlmEnableEVMTest failed for enable case");
	TEST(wlmEnableEVMTest(0, WLM_RATE_1M, 0),
		"wlmEnableEVMTest failed for disable case");
}

static void testIoctl()
{
	int val;
	int msglevel;
	printf("Testing IOCTL...\n");
	TEST(wlmIoctlGet(WLC_GET_MAGIC, &val, sizeof(int)),
		"wlmIoctlGet failed");
	TEST(val == WLC_IOCTL_MAGIC,
		"wlmIoctlGet value failed");
	val = 0;
	msglevel = 0x5555aaaa;
	TEST(wlmIoctlSet(WLC_SET_MSGLEVEL, &msglevel, sizeof(int)),
		"wlmIoctlSet failed");
	TEST(wlmIoctlGet(WLC_GET_MSGLEVEL, &val, sizeof(int)),
		"wlmIoctlGet failed");
	TEST(val == msglevel,
		"wlmIoctlGet value failed");
	msglevel = 0;
	TEST(wlmIoctlSet(WLC_SET_MSGLEVEL, &msglevel, sizeof(int)),
		"wlmIoctlSet failed");
}

static void testIovar()
{
	int val;
	int msglevel;
	int wsec;
	char buf[128];
	int *p;
	wl_rssi_event_t rssi, *rssi_read;
	int i;

	printf("Testing IOVAR...\n");

	val = 0;
	msglevel = 0xaaaa5555;
	p = (int *)buf;
	p[0] = msglevel;
	TEST(wlmIovarSet("msglevel", buf, 128),
		"wlmIovarSet failed");
	TEST(wlmIovarGet("msglevel", &val, sizeof(int)),
		"wlmIovarGet failed");
	TEST(val == msglevel,
		"wlmIovarGet value failed");
	msglevel = 0;
	p = (int *)buf;
	p[0] = msglevel;
	TEST(wlmIovarSet("msglevel", buf, 128),
		"wlmIovarSet failed");

	val = 0;
	wsec = 3;
	TEST(wlmIovarIntegerSet("wsec", wsec),
		"wlmIovarIntegerSet failed");
	TEST(wlmIovarIntegerGet("wsec", &val),
		"wlmIovarIntegerGet failed");
	TEST(val == wsec,
		"wlmIovarIntegerGet value failed");
	msglevel = 0;
	TEST(wlmIovarIntegerSet("wsec", wsec),
		"wlmIovarIntegerSet failed");

	rssi.rate_limit_msec = 100;
	rssi.num_rssi_levels = MAX_RSSI_LEVELS;
	for (i = 0; i < MAX_RSSI_LEVELS; i++)
		rssi.rssi_levels[i] = 0x11 * (i + 1);
	TEST(wlmIovarBufferSet("rssi_event", &rssi, sizeof(rssi)),
		"wlmIovarBufferSet failed");
	TEST(wlmIovarBufferGet("rssi_event", NULL, 0, (void **)&rssi_read),
		"wlmIovarBufferGet failed");
	TEST(rssi.rate_limit_msec == rssi_read->rate_limit_msec,
		"rate_limit_msec value failed");
	TEST(rssi.num_rssi_levels == rssi_read->num_rssi_levels,
		"num_rssi_levels value failed");
	for (i = 0; i < MAX_RSSI_LEVELS; i++) {
		TEST(rssi.rssi_levels[i] == rssi_read->rssi_levels[i],
			"rssi_levels value failed");
	}
}

static void printUsage(void)
{
	printf("\nUsage: mfgapi_test [--socket <IP address> [server port] | "
		"--serial <serial port> | --wifi <MAC address> | "
		"--dongle <serial port>] [--linux | --linuxdut]]\n\n");
	printf("--socket - Ethernet between client and server (running 'wl_server_socket')\n");
	printf("      IP address - IP address of server (e.g. 10.200.30.10)\n");
	printf("      server port - Server port number (default 8000)\n\n");
	printf("--serial - Serial between client and server (running 'wl_server_serial')\n");
	printf("      serial port - Client serial port (e.g. 1 for COM1)\n\n");
	printf("--wifi - 802.11 between client and server (running 'wl_server_wifi')\n");
	printf("         (external dongle only, NIC not supported)\n");
	printf("      MAC address - MAC address of wifi interface on dongle "
		"(e.g. 00:11:22:33:44:55)\n\n");
	printf("--dongle - Serial between client and dongle UART (running 'wl_server_dongle')\n");
	printf("      serial port - Client serial port (e.g. COM1 or /dev/ttyS0)\n\n");
	printf("--linux|linuxdut - Server DUT running Linux\n");
}

int main(int /* argc */, char **argv)
{
	WLM_DUT_INTERFACE dutInterface = WLM_DUT_LOCAL;
	char *interfaceName = 0;
	WLM_DUT_SERVER_PORT dutServerPort = WLM_DEFAULT_DUT_SERVER_PORT;
	WLM_DUT_OS dutOs = WLM_DUT_OS_WIN32;

	while (*++argv) {

		if (strncmp(*argv, "--help", strlen(*argv)) == 0) {
			printUsage();
			exit(0);
		}

		if (strncmp(*argv, "--socket", strlen(*argv)) == 0) {
			dutInterface = WLM_DUT_SOCKET;
			if (!*++argv) {
				printf("IP address required\n");
				printUsage();
				exit(-1);
			}
			interfaceName = *argv;
			int port;
			if (*++argv && (sscanf(*argv, "%d", &port) == 1)) {
				dutServerPort = (WLM_DUT_SERVER_PORT)port;
			}
			else {
				/* optional parameter */
				--argv;
			}
		}

		if (strncmp(*argv, "--serial", strlen(*argv)) == 0) {
			dutInterface = WLM_DUT_SERIAL;
			if (!*++argv) {
				printf("serial port required\n");
				printUsage();
				exit(-1);
			}
			interfaceName = *argv;
		}

		if (strncmp(*argv, "--wifi", strlen(*argv)) == 0) {
			dutInterface = WLM_DUT_WIFI;
			if (!*++argv) {
				printf("MAC address required\n");
				printUsage();
				exit(-1);
			}
			interfaceName = *argv;
		}

		if (strncmp(*argv, "--dongle", strlen(*argv)) == 0) {
			dutInterface = WLM_DUT_DONGLE;
			if (!*++argv) {
				printf("COM port required\n");
				printUsage();
				exit(-1);
			}
			unsigned int i;
			char buffer[256];
			if (!(sscanf(*argv, "COM%u", &i) == 1 ||
				sscanf(*argv, "/dev/tty%s", buffer) == 1)) {
				printf("serial port invalid\n");
				printUsage();
				exit(-1);
			}
			interfaceName = *argv;
		}

		if ((strncmp(*argv, "--linux", strlen(*argv)) == 0) ||
			strncmp(*argv, "--linuxdut", strlen(*argv)) == 0) {
			dutOs = WLM_DUT_OS_LINUX;
		}
	}

	TEST_INITIALIZE();

	TEST_FATAL(wlmApiInit(), "wlmApiInit failed");

	/* test before selecting actual interface */
	testSelectInterface();

	TEST_FATAL(wlmSelectInterface(dutInterface, interfaceName,
		dutServerPort, dutOs), "wlmSelectInterface failed");

	/* packet engine requires MPC to be disabled and WLAN interface up */
	TEST(wlmMinPowerConsumption(FALSE), "wlmMinPowerConsuption failed");
	TEST(wlmEnableAdapterUp(TRUE), "wlmEnableAdapterUp failed");

	/* invoke test cases */
	testDutInit();
	testVersion();
	testEnableAdapter();
	testBandList();
	testBand();
	testGmode();
	testChannel();
	testRate();
	testAntenna();
	testPreamble();
	testTransmit();
	testReceive();
	testSequence(FALSE);
	testSequence(TRUE);
	testTxPower();
	testEstimatedPower();
	testImageAccess();
	testSsidGet();
	testWepSecurity();
	testTkipAesSecurity();
	testJoinNetworkNone();
	testJoinNetworkWep();
	testJoinNetworkWpaTkip();
	testJoinNetworkWpaAes();
	testJoinNetworkWpa2Tkip();
	testJoinNetworkWpa2Aes();
	testDisableAdapter();

	testMacAddr();
	testPaParameters();
	testCarrierTone();
	testEvmTest();

	testIoctl();
	testIovar();

	TEST_FATAL(wlmApiCleanup(), "wlmApiCleanup failed");

	TEST_FINALIZE();
	return 0;
}

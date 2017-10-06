// mfgtestExe.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "mfgtest.h"

#define MFGTEST_API_DLL "MfgTestApi.dll"

HINSTANCE LoadDLL();

int main(int argc, char **argv)
{
	HINSTANCE dllHndl;
	
	if ((dllHndl = LoadDLL()) == NULL) {
		printf("Failed to load %s.\n", MFGTEST_API_DLL);
		return -1;
	}
	/*initialize adapter before test
	* Example, open a adapter from local machine
	*/
	ADAPTER_INTERFACE_TYPE iftype = IF_SDIO;

	selectInterface(iftype, "none");
	if(!openAdapter()) {
		printf("openAdapter failed.\n");
		return -1;
	}

	/*	
	TESTRESULT result;

	runTest(DUTInit, (void*) iftype, &result);

	typedef struct {
		BOOL useOTP;
		BOOL useEEPROM;
		char *inputFileName;
		char *entryName;
		int addressOffset;
		int *data;
	} PROGRAMNVRAM;

	PROGRAMNVRAM programNvram;

	runTest(programOTP, &programNvram, &result);
	runTest(programSrom, &programNvram, &result);

	
	TEST_PACKET_t testPacket;
	testPacket.band = BGBAND;
	testPacket.channel = 6;
	testPacket.rate = TEST_RATE_11M;

	runTest(txPERStart, &testPacket, &result);

	runTest(txPERStop, &testPacket, &result);

	runTest(rxPERStart, &testPacket, &result);

	runTest(rxPERStop, &testPacket, &result);

	runTest(frequencyAccuracy, &testPacket, &result);

	runTest(evmTest, &testPacket, &result);

	runTest(txPowerTest, &testPacket, &result);

	runTest(powerDetectorTest, &testPacket, &result
	*/

	char buff[250];
	char* ver = "ver";
	if (!getWlIoctl(ver, NULL, 0, (void*) buff))
		printf("Version is retrieved as %s\n", buff);
	else
		printf("Error encountered while getting version.\n");

	char* mpc = "mpc";
	if (setWlIoctl(mpc, "0"))
		printf("Error encountered while setting mpc.\n");
	else
		printf("Set mpc to 0 successfully.\n");


	if (!getWlIoctl(mpc, NULL, 0, (void*) buff))
		printf("MPC is retrieved as %s\n", buff);
	else
		printf("Error encountered while getting MPC.\n");

	/* PMT is intentional. To check error reporting capability. */
	char* pmt = "PMT";
	if (setWlIoctl(pmt, "1"))
		printf("Error encountered while setting PMT.\n");
	if (!getWlIoctl(pmt, NULL, 0, (void*) buff))
		printf("PMT is retrieved as %s\n", buff);
	else
		printf("Error encountered while getting PMT.\n\n");

	char* pm = "PM";
	if (setWlIoctl(pm, "1"))
		printf("Error encountered while setting PM.\n");
	else
		printf("Set PM to 1 successfully.\n");

	if (!getWlIoctl(pm, NULL, 0, (void*) buff))
		printf("PM is retrieved as %s\n", buff);
	else
		printf("Error encountered while getting PM.\n");

	/* close adapter after test ends */
	closeAdapter();
	
	if (dllHndl) {
		FreeLibrary(dllHndl);
	}
	return 0;
}

HINSTANCE LoadDLL () {
	HINSTANCE dllHndl;
	dllHndl = NULL;

	dllHndl = LoadLibrary(TEXT(MFGTEST_API_DLL));
	if (dllHndl == NULL) {
		return NULL;
	}

	if ((selectInterface = (selectInterface_t) GetProcAddress (dllHndl, WIDE_CHAR("selectInterface"))) == NULL) {
		printf("Function %s address is invalid.\n", "selectInterface");
		FreeLibrary(dllHndl);
		return NULL;
	}
	
	if ((openAdapter = (openAdapter_t) GetProcAddress (dllHndl, WIDE_CHAR("openAdapter"))) == NULL) {
		printf("Function %s address is invalid.\n", "openAdapter");
		FreeLibrary(dllHndl);
		return NULL;
	}

	if ((closeAdapter = (closeAdapter_t) GetProcAddress (dllHndl, WIDE_CHAR("closeAdapter"))) == NULL) {
		printf("Function %s address is invalid.\n", "closeAdapter");
		FreeLibrary(dllHndl);
		return NULL;
	}

	if ((runTest = (runTest_t) GetProcAddress (dllHndl, WIDE_CHAR("runTest"))) == NULL) {
		printf("Function %s address is invalid.\n", "runTest");
		FreeLibrary(dllHndl);
		return NULL;
	}

	if ((getTxPERResult = (getTxPERResult_t) GetProcAddress (dllHndl, WIDE_CHAR("getTxPERResult"))) == NULL) {
		printf("Function %s address is invalid.\n", "getTxPERResult");
		FreeLibrary(dllHndl);
		return NULL;
	}

	if ((setTxPERResult = (setTxPERResult_t) GetProcAddress (dllHndl, WIDE_CHAR("setTxPERResult"))) == NULL) {
		printf("Function %s address is invalid.\n", "setTxPERResult");
		FreeLibrary(dllHndl);
		return NULL;
	}

	
	if ((getRxPERResult = (getRxPERResult_t) GetProcAddress (dllHndl, WIDE_CHAR("getRxPERResult"))) == NULL) {
		printf("Function %s address is invalid.\n", "getRxPERResult");
		FreeLibrary(dllHndl);
		return NULL;
	}

	if ((setRxPERResult = (setRxPERResult_t) GetProcAddress (dllHndl, WIDE_CHAR("setRxPERResult"))) == NULL) {
		printf("Function %s address is invalid.\n", "setRxPERResult");
		FreeLibrary(dllHndl);
		return NULL;
	}

	if ((setWlIovar = (setWlIovar_t) GetProcAddress (dllHndl, WIDE_CHAR("setWlIovar"))) == NULL) {
		printf("Function %s address is invalid.\n", "setWlIovar");
		FreeLibrary(dllHndl);
		return NULL;
	}

	if ((getWlIovar = (getWlIovar_t) GetProcAddress (dllHndl, WIDE_CHAR("getWlIovar"))) == NULL) {
		printf("Function %s address is invalid.\n", "getWlIovar");
		FreeLibrary(dllHndl);
		return NULL;
	}

	if ((setWlIoctl = (setWlIoctl_t) GetProcAddress (dllHndl, WIDE_CHAR("setWlIoctl"))) == NULL) {
		printf("Function %s address is invalid.\n", "setWlIoctl");
		FreeLibrary(dllHndl);
		return NULL;
	}

	if ((getWlIoctl = (getWlIoctl_t) GetProcAddress (dllHndl, WIDE_CHAR("getWlIoctl"))) == NULL) {
		printf("Function %s address is invalid.\n", "getWlIoctl");
		FreeLibrary(dllHndl);
		return NULL;
	}

	
	if ((cmdSeqStart = (cmdSeqStart_t) GetProcAddress (dllHndl, WIDE_CHAR("cmdSeqStart"))) == NULL) {
		printf("Function %s address is invalid.\n", "cmdSeqStart");
		FreeLibrary(dllHndl);
		return NULL;
	}

	if ((cmdSeqStop = (cmdSeqStop_t) GetProcAddress (dllHndl, WIDE_CHAR("cmdSeqStop"))) == NULL) {
		printf("Function %s address is invalid.\n", "cmdSeqStop");
		FreeLibrary(dllHndl);
		return NULL;
	}

	/*if ((cmdSeqDelay = (cmdSeqDelay_t) GetProcAddress (dllHndl, WIDE_CHAR("cmdSeqDelay"))) == NULL) {
		printf("Function %s address is invalid.\n", "cmdSeqDelay");
		FreeLibrary(dllHndl);
		return NULL;
	}*/

	return dllHndl;
}

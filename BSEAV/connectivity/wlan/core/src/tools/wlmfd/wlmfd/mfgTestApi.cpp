// mfgTestApiDLL.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "mfgTestApi.h"
#include "wlIoctlDef.h"

#include <assert.h>


#define STRING_LENGTH 128
#define STRING_MAX_LEN 256
#define IP_ADDR_LEN 16

extern bool wlInit(HINSTANCE dllHndl, char *dllStartCmd);
extern void stopDUT(HINSTANCE dllHndl);
extern bool command(const char* cmd, char* data);


#ifdef _MANAGED
#pragma managed(push, off)
#endif

static ADAPTER_INTERFACE_TYPE interfaceType = IF_UNKNOWN;
static CONNECTION connection = CONNECTION_LOCAL;
static char* serialPortNo = "1";
static char remoteIPAddress[IP_ADDR_LEN] = "00.00.00.00";

static char  dllInitString[STRING_LENGTH] = "wl --interactive"; 

static HINSTANCE wlDLLHdl = NULL;


BOOL APIENTRY DllMain( HANDLE hModule, DWORD  dwReason, LPVOID lpReserved)
{
	/* Perform actions based on the reason for calling.*/
    switch(dwReason) 
    { 
        case DLL_PROCESS_ATTACH:
			break;

        case DLL_THREAD_ATTACH:
         // Do thread-specific initialization.
            break;

        case DLL_THREAD_DETACH:
         // Do thread-specific cleanup.
            break;

        case DLL_PROCESS_DETACH:
         // Perform any necessary cleanup.
            break;
    }
    return TRUE;
}

DLLExport bool selectInterface(ADAPTER_INTERFACE_TYPE iftype, char *interfaceName) {
	switch (iftype) {
		case IF_SDIO:
			DBGMSG(("Using SDIO interface in local mode.\n"));
			interfaceType = IF_SDIO;
			connection = CONNECTION_LOCAL;
			setLocalComm();
			break;
		case IF_UART:
			DBGMSG(("Using UART interface in remote mode.\n"));
			interfaceType = IF_UART;
			connection = CONNECTION_REMOTE;
			setSerialComm(interfaceName);
			break;
		case IF_ETHER:
			printf("Using Ethernet interface in remote mode.\n");
			interfaceType = IF_ETHER;
			connection = CONNECTION_REMOTE;
			setNetworkComm(interfaceName);
			break;
		case IF_UNKNOWN:
			printf("Unknow interface type, must select an interface type.\n");
			interfaceType = IF_UNKNOWN;
			return false;
		default:
			return false;
	}
	return true;
}

DLLExport BOOL openAdapter() {
	return wlInit(wlDLLHdl, dllInitString );
}

DLLExport void closeAdapter() {
	stopDUT(wlDLLHdl);
}


DLLExport BOOL runTest(TEST_NAME testName, void *testParameters, Test_Result_t *testResult) {
	switch (testName) {
		case TEST_PROGRAMOTP:
			break;
		case TEST_GET_OTPDUMP:
			break;
		case TEST_PROGRAMSROM:
			break;
		case TEST_GET_SROMDUMP:
			break;
		case TEST_DUTINIT:
			break;
		case TEST_TXPERSTART:
			break;
		case TEST_TXPERSTOP:
			break;
		case TEST_RXPERSTART:
			break;
		case TEST_RXPERSTOP:
			break;
		case TEST_CARRIER_TONE_ON:
			break;
		case  TEST_CARRIER_TONE_OFF:
			break;
		case TEST_EVM_ON:
			break;
		case TEST_EVM_OFF:
			break;
		case TEST_PD_GET_TSSI:
			break;
		case TEST_PD_GET_ESTPOWER:
			break;
		case TEST_PD_GET_PAPARAS:
			break;
		case TEST_PD_SET_PAPARAS:
			break;
		case TEST_PD_COMPUTE_PAPARA:
			break;
		default:
			break;
	}
	return TRUE;
}

DLLExport UINT32 setWlIovar (const char *iovar, void *param) {
	char cmd[DEFAULT_WRITE_BUFFER_SIZE];
	char data[DEFAULT_READ_BUFFER_SIZE];

	assert(iovar);
	
	strcpy(cmd, iovar);
	if (param) {
		strcat(cmd, " ");
		strcat(cmd, (char*)param);
	}
	DBGMSG(("%s : cmd = %s\n",__FUNCTION__, cmd));
	if(!command(cmd, data))
		return -1;

	return 0;
}

DLLExport UINT32 getWlIovar(const char *iovar, void *param, int param_len, void *bufptr) {
	char cmd[DEFAULT_WRITE_BUFFER_SIZE];
	//char data[DEFAULT_READ_BUFFER_SIZE];

	assert(iovar);
	assert(bufptr);

	if(!bufptr) {
		return -1;
	}

	strcpy(cmd, iovar);
	if (param && param_len) {
		strcat(cmd, " ");
		strcat(cmd, (char*)param);
	}
	
	DBGMSG(("%s : cmd = %s\n",__FUNCTION__, cmd));
	if(!command(cmd, (char *)bufptr))
		return -1;

	return 0;
}

DLLExport UINT32 setWlIoctl (const char *iovar, void *param) {
	char cmd[DEFAULT_WRITE_BUFFER_SIZE];
	char data[DEFAULT_READ_BUFFER_SIZE];

	assert(iovar);
	
	strcpy(cmd, iovar);
	if (param) {
		strcat(cmd, " ");
		strcat(cmd, (char*)param);
	}
	DBGMSG(("%s : cmd = %s\n",__FUNCTION__, cmd));
	if (!command(cmd, data))
		return -1;
	
	return 0;
}

DLLExport UINT32 getWlIoctl(const char *iovar, void *param, int param_len, void *bufptr) {
	char cmd[DEFAULT_WRITE_BUFFER_SIZE];
	
	assert(iovar);
	if(!bufptr)
		return -1;

	strcpy(cmd, iovar);
	if(param && param_len) {
		strcat(cmd, " ");
		strcat(cmd, (char*)param);
	}
	
	DBGMSG(("%s : cmd = %s\n",__FUNCTION__, cmd));
	if (!command(cmd, (char *)bufptr))
		return -1;

	return 0;
}

DLLExport Per_Test_Result *getTxPERResult() {
	Per_Test_Result *result;
	result = NULL;
	return result;
}

DLLExport void setTxPERResult(Per_Test_Result *perResult) {
	
}

DLLExport Per_Test_Result *getRxPERResult() {
	Per_Test_Result *result;
	result = NULL;
	return result;
}

DLLExport void setRxPERResult(Per_Test_Result *perResult) {
	
}

DLLExport void cmdSeqStart() {
	setWlIovar("seq_start", NULL);
}

DLLExport void cmdSeqStop() {
	setWlIovar("seq_stop", NULL);
}

DLLExport void cmdSeqDelay(UINT32 delay_ms) {
	setWlIovar("seq_delay", (void*)&delay_ms );
}

DLLExport void appendResult (Test_Result_t *result) {
	
}
void setLocalComm() {
	strcpy(dllInitString, "wl --interactive");
}
void setSerialComm(char *interfaceName) {
	strcpy(serialPortNo, interfaceName);
	sprintf(dllInitString, "wl --serial %s --interactive", serialPortNo);	
}

/* this is not supported case so far*/
void setNetworkComm (char *interfaceName) {
	strcpy(remoteIPAddress, interfaceName);
	sprintf(dllInitString, "wl --ether %s --interactive", remoteIPAddress);
}

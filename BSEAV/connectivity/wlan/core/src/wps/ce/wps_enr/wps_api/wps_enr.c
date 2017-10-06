#include <windows.h>
#include <stdio.h>
#include <Wincrypt.h>
#include <wpserror.h>
#include <portability.h>
#include <wps_enrapi.h>
#include <wps_sta.h>
#include "wps_enr_osl.h"
#include <epictrl.h>
#include <irelay.h>
#include "wlioctl.h"
#include <eap.h>
#include <wzcsapi.h>
#include <winioctl.h>
#include <Devload.h>
#include <string.h>
#include "wps_enr.h"
#include "wps_sdk.h"
#include "pm.h"
#include <reg_prototlv.h>

/* Set this to 1 to remove WZC compeltely. Set it to 0 otherwise */
#define DISABLE_WZC 0

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

#define WPS_EAP_DATA_MAX_LENGTH         2048
#define WPS_EAP_READ_DATA_TIMEOUT         3
#define WZC_DRIVER TEXT("Drivers\\BuiltIn\\ZeroConfig")

static 	char def_pin[9]= "12345670\0";
static int builtInStart = 0;

static int registration_loop(unsigned long start_time);
static void config_init();
static void reg_config_init(WpsEnrCred *credential);
static int print_usage();
static void get_random_credential(WpsEnrCred *credential);
static int find_pbc_ap(char * bssid, char *ssid, uint8 *wsec);
int SetOid(ULONG oid, void* data, ULONG nbytes);
int QueryOid(ULONG oid, void* results, ULONG nbytes);
int CheckForAssociation(char *bssid);
void display_err(PCHAR prefix, DWORD status);
PADAPTER get_adapter();
int init_adapter();
extern int ConfigureWindowsZeroConfig(int bEnable);
int ConfigureWZCInterface(BOOL bEnable);
int SetWZCInterfaceState(BOOL bEnable);
int ConfigureAdapter(PWCHAR pAdapter, DWORD dwCommand);
extern int reload_driver();
void display_error_command_line(char *cmd);
int is_number(char* cmd);
extern wps_ap_list_info_t *wps_get_ap_list();
uint32 wps_generatePin(char c_devPwd[8], IN bool b_display);
bool CallClientCallback(unsigned int uiStatus, void *data);
static bool get_ap_configured(char * bssid, char *ssid);

/* useful macros */
#define ARRAYSIZE(a)  (sizeof(a)/sizeof(a[0]))
#define WPS_DUMP_BUF_LEN (16 * 1024)
// utility macro to convert a hexa digit into its value
#define HEX(c)  ((c)<='9'?(c)-'0':(c)<='F'?(c)-'A'+0xA:(c)-'a'+0xA)
WZC_CONTEXT g_WzcContext;
bool g_bWzcContextInitialized = false;

typedef DWORD (*PFN_WZCQueryContext)(LPWSTR pSrvAddr,DWORD dwInFlags,PWZC_CONTEXT pContext,LPDWORD pdwOutFlags);
typedef DWORD (*PFN_WZCSetContext)(LPWSTR pSrvAddr,DWORD dwInFlags,PWZC_CONTEXT pContext,LPDWORD pdwOutFlags);
typedef DWORD (*PFN_WZCQueryInterface)(LPWSTR pSrvAddr,DWORD dwInFlags,PINTF_ENTRY_EX pIntf,LPDWORD pdwOutFlags);
typedef DWORD (*PFN_WZCDeleteIntfObj)(PINTF_ENTRY_EX pIntf);
typedef DWORD (*PFN_WZCSetInterface)(LPWSTR pSrvAddr,DWORD dwInFlags,PINTF_ENTRY_EX pIntf,LPDWORD pdwOutFlags);
typedef VOID  (*PFN_WZCPassword2Key)(PWZC_WLAN_CONFIG pwzcConfig, LPCSTR cszPassword);

typedef struct _ClientInfo
{
	char * bssid;
	char *ssid;
	char *pin;
	int mode;
	wps_credentials *credentials;
} ClientInfo;

bool gIsOpened = false;
void *gContext=NULL;
fnWpsProcessCB g_wps_join_callback = NULL;


/* This handle needs to be passed to all "ir_" fuinctions in the
 * shared DLL.  It is global because most of the APIs here were written
 * when the handle was maintained as a global in the shared dll.
 */

static DWORD ndevs;
static ADAPTER devlist[10];

/* Structure to hold the adapter information */
typedef struct _adapter_info
{
	HANDLE irh;
	wchar_t adaptername[80];
	uint8 macaddress[6];
	char ssid[33];
} adapter_info;

/* Global declaration of adapter information */
adapter_info stAdapterInfo;

/* Helper functions to access adapter information */
HANDLE getAdapterHandle()
{
	return stAdapterInfo.irh;
}

uint8* getMacAddress()
{
	return &stAdapterInfo.macaddress[0];
}

wchar_t* getAdapterName()
{
	return stAdapterInfo.adaptername;
}

char* getNetwork()
{
	return stAdapterInfo.ssid;
}


void setBuiltInStart(int value)
{
	builtInStart = value;
}

int getBuiltInStart(void)
{
    return builtInStart;
}

bool compare_mac(const uint8 *mac1, const uint8 *mac2)
{
	int i;
	if(mac1 && mac2)
	{
		for(i=0; i<6; i++)
			if(mac1[i] != mac2[i])
				return false;
		return true;
	}
	else
	{
		return false;
	}
}


static uint8 get_wep(const uint8 *bssid)
{
	wps_ap_list_info_t *ap_list = wps_get_ap_list();
	int i = 0;
	for(i=0; i<WPS_MAX_AP_SCAN_LIST_LEN; i++)
	{
		// Find the ap according by comparing the mac address
		if(compare_mac(bssid, ap_list[i].BSSID))
			return ap_list[i].wep;
	}

	return 0;
}

static bool get_ap_configured(char * bssid, char *ssid)
{
	wps_ap_list_info_t *wpsaplist;
	int i = 0, retry_limit = 3;

retry:
	if (retry_limit) {
		wpsaplist = create_aplist();
		if (wpsaplist) {
			wps_get_aplist(wpsaplist, wpsaplist);
			while (i < WPS_MAX_AP_SCAN_LIST_LEN && wpsaplist->used == TRUE) {
				if (strcmp(ssid, (char *)wpsaplist->ssid) == 0 &&
					memcmp(bssid, (char*)wpsaplist->BSSID, 6) == 0) {
					return is_ConfiguredState(wpsaplist->ie_buf,
						wpsaplist->ie_buflen);
				}
				i++;
				wpsaplist++;
			}
		}
		retry_limit--;
		goto retry;
	}
	return false;
}

/* main function : 
   arguments : 

   pin : if NULL, do PBC configuration. 
*/

static bool enroll_device(int mode, uint8 *bssid, char *ssid, char *pin); 

/*
  find an AP with PBC active or timeout.
  Returns SSID and BSSID.
  Note : when we join the SSID, the bssid of the AP might be different
  than this bssid, in case of multiple AP in the ESS ...
  Don't know what to do in that case if roaming is enabled ...
 */

static int find_pbc_ap(char * bssid, char *ssid, uint8 *wsec)
{
	int pbc_ret = PBC_NOT_FOUND;
	char start = true;

	wps_ap_list_info_t *wpsaplist = 0;

	/* add wps ie to probe  */
	add_wps_ie(NULL, 0);

	while (PBC_NOT_FOUND == pbc_ret) {

		wpsaplist = create_aplist();
		if(wpsaplist){
			wps_get_aplist(wpsaplist, wpsaplist);
			pbc_ret = wps_get_pbc_ap(wpsaplist,bssid, ssid,
				wsec, get_current_time(), start);
			start = false;
		}
		WpsSleep(1000);
	}
	rem_wps_ie(NULL, 0);
	if(pbc_ret != PBC_FOUND_OK)
		return 0;
	return 1;
}

static bool enroll_device(int mode, uint8 *bssid, char *ssid, char *pin) 
{
	char start = true;
	HANDLE* irh = 0;
	bool bRet = false;
	uint32 start_time = get_current_time();
	int reattempt = FALSE;

	strcpy(stAdapterInfo.ssid,ssid);

	DBGPRINT(("Enrolling device\n"));

	if (!wps_osl_init(bssid))
	{
		DBGPRINT(("Exiting as init failed\n"));
		CallClientCallback(WPS_STATUS_ERROR,NULL);
		goto done;
	}

	DBGPRINT(("Starting WPS enrollment.\n"));
	while (1) {
		int res;
		
		if(reattempt) {
			/* WPS will be re-attempt till 2min timeout is reached.
			 * The applications should use this status
			 * (WPS_STATUS_REATTEMPTING_WPS), if they want
			 * to quit earlier.
			 */	
			if(!CallClientCallback(WPS_STATUS_REATTEMPTING_WPS, NULL)) 
				goto done;
		}
				
		wpssta_start_enrollment(pin, get_current_time());

		/* registration loop */
		/* exits with either success, failure or indication that 
		   the registrar has not started its end of the protocol yet. 
		 */
		if ((res = registration_loop(start_time)) == WPS_SUCCESS) {
			bRet = true;
			break;
		}
		/* the registrar is not started, maybe the user is walking or entering 
		   the PIN. Try again.
		 */
		else if (res == WPS_CONT) {
			int i=10;
			
			leave_network();

			DBGPRINT(("Waiting for Registrar\n"));
			while(CallClientCallback(WPS_STATUS_IDLE,NULL) && i--) Sleep(100);
			if(!CallClientCallback(WPS_STATUS_IDLE,NULL)) goto done;

			// Re-join/re-associate network. This is required to work with (be compatible to) old broadcom AP firmware
			join_network(ssid, get_wep(bssid));
		}
		else {
			DBGPRINT(("WPS Protocol FAILED \n"));
			CallClientCallback(WPS_STATUS_ERROR,NULL);
			break;
		}

		reattempt = TRUE;
	}

done:
	if(!CallClientCallback(WPS_STATUS_DISCONNECTING,NULL)) bRet = false;
	leave_network();
	return bRet;
}
/* 
   Fill up the device info and pass it to WPS. 
   This will need to be tailored to specific platforms (read from a file,
   nvram ...)
 */

static void config_init()
{
	DevInfo info;
	char uuid[16] = {0x22,0x21,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0xa,0x0b,0x0c,0x0d,0x0e,0x0f};

	/* fill in device specific info. The way this information is stored is app specific */
	/* Would be good to document all of these ...  */

	memset((char *)(&info), 0, sizeof(info));
	info.version = 0x10;
	
	memcpy(info.uuid, uuid, 16);

	if(getMacAddress())
		memcpy(info.macAddr, getMacAddress(), 6);  // Fill mac address

	strcpy(info.deviceName, "Broadcom Client");
	info.primDeviceCategory = 1;
	info.primDeviceOui = 0x0050F204;
	info.primDeviceSubCategory = 1;
	strcpy(info.manufacturer, "Broadcom");
	strcpy(info.modelName, "WPS Wireless Client");
	strcpy(info.modelNumber, "1234");
	strcpy(info.serialNumber, "5678");
	info.configMethods = 0x008C;
	info.authTypeFlags = 0x003f;
	info.encrTypeFlags = 0x000f;
	info.connTypeFlags = 0x01;
	info.rfBand = 1;
	info.osVersion = 0x80000000;
	info.featureId = 0x80000000;

	wpssta_enr_init(&info);
}



/* Main loop.*/
static int registration_loop(unsigned long start_time)
{
	uint32 retVal;
	char buf[WPS_EAP_DATA_MAX_LENGTH];
	uint32 len;
	char *sendBuf;
	unsigned long now;
	int last_recv_msg, last_sent_msg;
	int state;
	char msg_type;
	int nMsgType;

	now = get_current_time();

	/*
	 * start the process by sending the eapol start . Created from the
	 * Enrollee SM Initialize.
	 */
    DBGPRINT(("Sending EAPOL Start\n"));
	len = wps_get_msg_to_send(&sendBuf, (uint32)now);
	if (sendBuf) {
		// Notify EAPOL-Start
        if(!CallClientCallback(WPS_STATUS_STARTING_WPS_EXCHANGE, NULL)) return REG_FAILURE;

		retVal = send_eapol_packet(sendBuf, len);
		if (retVal != WPS_SUCCESS)
		{
			CallClientCallback(WPS_STATUS_WARNING_NOT_INITIALIZED,NULL);
			return WPS_ERR_NOT_INITIALIZED;
		}
	}
	else {
		DBGPRINT(("system not initialized\n"));
		/* this means the system is not initialized */
		CallClientCallback(WPS_STATUS_WARNING_NOT_INITIALIZED,NULL);
		return WPS_ERR_NOT_INITIALIZED;
	}

	/* loop till we are done or failed */
	while (1) {
		len = WPS_EAP_DATA_MAX_LENGTH;

		now = get_current_time();

		DBGPRINT(("Overall protocol time %u\n",now-start_time));

		// according to the WPS spec. the overall protocol time is 2 minutes
		if (now > start_time + 120) {
			DBGPRINT(("Overall protocol timeout \n"));
			CallClientCallback(WPS_STATUS_WARNING_TIMEOUT,NULL);
			return REG_FAILURE;
		}

		if(!CallClientCallback(WPS_STATUS_WAITING_WPS_RESPONSE,NULL)) return REG_FAILURE;
		if ((retVal = wait_for_eapol_packet(buf, &len, WPS_EAP_READ_DATA_TIMEOUT)) == WPS_SUCCESS) {
			DBGPRINT(("Received packet\n"));

			/* Show receive message */
			msg_type = wps_get_ap_msg_type(buf, len);
			DBGPRINT(("Receive EAP-Request%s\n", wps_get_msg_string((int)msg_type)));

            /* process ap message */
			nMsgType = (int)msg_type;  // convert to int type
			if(!CallClientCallback(WPS_STATUS_GOT_WPS_RESPONSE, &nMsgType)) return REG_FAILURE;

			retVal = wps_process_ap_msg(buf, len);

			/* check return code to do more things */
			if (retVal == WPS_SEND_MSG_CONT ||
				retVal == WPS_SEND_MSG_SUCCESS ||
				retVal == WPS_SEND_MSG_ERROR) {
				len = wps_get_msg_to_send(&sendBuf, now);
				if (sendBuf) {
					DBGPRINT(("Waiting -> Sending packet\n"));
					wps_eap_send_msg(sendBuf, len);

					// Send back sending message type
					nMsgType = wps_get_sent_msg_id();
					if(!CallClientCallback(WPS_STATUS_SENDING_WPS_MESSAGE, &nMsgType)) return REG_FAILURE;
				}

				/* over-write retVal */
				if (retVal == WPS_SEND_MSG_SUCCESS)
					retVal = WPS_SUCCESS;
				else if (retVal == WPS_SEND_MSG_ERROR)
					retVal = REG_FAILURE;
				else
					retVal = WPS_CONT;
			}
			else if (retVal == EAP_FAILURE) {
				DBGPRINT(("Received an eap failure from registrar\n"));

				/* we received an eap failure from registrar */
				/*
				 * check if this is coming AFTER the protocol passed the M2
				 * mark or is the end of the discovery after M2D.
				 */
				last_recv_msg = wps_get_recv_msg_id();
				DBGPRINT(("Received eap failure, last recv msg EAP-Request%s\n", wps_get_msg_string(last_recv_msg)));
				if (last_recv_msg > WPS_ID_MESSAGE_M2D) {
                    CallClientCallback(WPS_STATUS_WARNING_WPS_PROTOCOL_FAILED,NULL);
					return REG_FAILURE;
                }
				else
					return WPS_CONT;
			}
			/* special case, without doing wps_eap_create_pkt */
			else if (retVal == WPS_SEND_MSG_IDRESP) {
				len = wps_get_msg_to_send(&sendBuf, now);
				if (sendBuf) {
					DBGPRINT(("Waiting -> Sending IDENTITY Resp packet\n"));
					send_eapol_packet(sendBuf, len);

					// Sending back IDENTITY repsonse packet
					nMsgType = WPS_ID_MESSAGE_DONE + 1;  // Identity
					if(!CallClientCallback(WPS_STATUS_SENDING_WPS_MESSAGE, &nMsgType)) return REG_FAILURE;
				}
			}

			/* SUCCESS or FAILURE */
			if (retVal == WPS_SUCCESS || retVal == REG_FAILURE) {
				return retVal;
			}
		}
		/* timeout with no data, should we re-transmit ? */
		else if (retVal == EAP_TIMEOUT) {
			DBGPRINT(("Timeout while waiting for a msg from AP\n"));

			/* check eap receive timer. It might be time to re-transmit */

			/*
			 * Do we need this API ? We could just count how many
			 * times we re-transmit right here.
			 */
			if ((retVal = wps_eap_check_timer(now)) == WPS_SEND_MSG_CONT) {
                DBGPRINT(("Trying sending msg again\n"));
				len = wps_get_retrans_msg_to_send(&sendBuf, now, &msg_type);
				if (sendBuf) {
					DBGPRINT(("Trying sending msg again\n"));
					state = wps_get_eap_state();
					if (state == EAPOL_START_SENT)
					{
						DBGPRINT(("Re-Send EAPOL-Start\n"));
						if(!CallClientCallback(WPS_STATUS_STARTING_WPS_EXCHANGE,NULL)) return REG_FAILURE;
					}
					else if (state == EAP_IDENTITY_SENT)
					{
						DBGPRINT(("Re-Send EAP-Response / Identity\n"));
						nMsgType = WPS_ID_PROBE_RESP;
						if(!CallClientCallback(WPS_STATUS_SENDING_WPS_MESSAGE,&nMsgType)) return REG_FAILURE;
					}
					else
					{
						DBGPRINT(("Re-Send EAP-Response%s\n", wps_get_msg_string((int)msg_type)));
						if(!CallClientCallback(WPS_STATUS_SENDING_WPS_MESSAGE,&msg_type)) return REG_FAILURE;
					}

					send_eapol_packet(sendBuf, len);
				}
			}
			/* re-transmission count exceeded, give up */
			else if (retVal == EAP_TIMEOUT) {
				DBGPRINT(("Re-transmission count exceeded, wait again\n"));

				last_recv_msg = wps_get_recv_msg_id();

				if (last_recv_msg == WPS_ID_MESSAGE_M2D) {
					DBGPRINT(("M2D Wait timeout, again.\n"));
					return WPS_CONT;
				}
				else if (last_recv_msg > WPS_ID_MESSAGE_M2D) {
					last_sent_msg = wps_get_sent_msg_id();
					DBGPRINT(("Timeout, give up. Last recv/sent msg "
						"[EAP-Response%s/EAP-Request%s]\n",
						wps_get_msg_string(last_recv_msg),
						wps_get_msg_string(last_sent_msg)));
					return REG_FAILURE;
				}
				else {
					DBGPRINT(("Re-transmission count exceeded, again\n"));
					return WPS_CONT;
				}
			}
	        else if(retVal == WPS_ERR_ADAPTER_NONEXISTED)
		        return retVal;  // This is probably due to adapter being removed during wps
		}
	}

	return WPS_SUCCESS;
}

int
wps_wl_ioctl(int cmd, void *buf, int len, bool set)
{
	DWORD dwlen = len;
	int error;
	WINERR err = ERROR_SUCCESS;

	if (!set)
	{
		error = QueryOid(cmd, buf, dwlen);
	}
	else
	{
		error = SetOid(cmd, buf, dwlen);
	}
	return error;
}

PADAPTER get_adapter()
{
	HANDLE irh;
	WINERR err = ERROR_SUCCESS;
	DWORD i;
	PADAPTER		pdev = 0;
	int adapter  =-1;
	DWORD status= ERROR_SUCCESS;

	/* initialize irelay and select default adapter */
	irh = INVALID_HANDLE_VALUE;

	if ((err = ir_init(&irh)) != ERROR_SUCCESS)
		goto done;
	ndevs = ARRAYSIZE(devlist);
	if ((err = ir_adapter_list(irh, &devlist[0], &ndevs)) != ERROR_SUCCESS) 
		goto done;

	for (i = 0; i < ndevs; i++) {
		pdev = &devlist[i];
		if (pdev->type == IR_WIRELESS) {
			adapter = i;
			break;
		}
	}

	if (i == ndevs) {
		fprintf(stderr, "No wireless adapters were found\n");
		goto done;
	}

	if (adapter < 0 || (ULONG) adapter >= ndevs) {
		fprintf(stderr, "Cannot find wireless adapter #%d\n", adapter);
		status = ERROR_INVALID_HANDLE;
	} else {
		pdev = &devlist[adapter];
		if (pdev->type != IR_WIRELESS) {
			fprintf(stderr,
				"Selected adapter #%d is not an BRCM wireless adapter\n",
				adapter);
			status = ERROR_INVALID_HANDLE;
		} else {
			status = ir_bind(irh, pdev->name);
			if (status != ERROR_SUCCESS)
				DBGPRINT(("Failure in ir_bind"));
			else
			{
				stAdapterInfo.irh = irh;
				return pdev;
			}
		}
	}
done:
	DBGPRINT(("Exiting\n"));
	if (irh != INVALID_HANDLE_VALUE) {
		ir_unbind(irh);
		ir_exit(irh);
	}
	return 0;
}

int init_adapter()
{
	PADAPTER pAdapter  = get_adapter();

	if (!pAdapter)
	{
		DBGPRINT(("get_adapter()failed"));
		return 0;
	}
	memcpy(stAdapterInfo.macaddress,pAdapter->macaddr,6);
	wcscpy(stAdapterInfo.adaptername,pAdapter->name);
	return 1;
}

static void reg_config_init(WpsEnrCred *credential)
{
	DevInfo info;
	char uuid[16] = {0x22, 0x21, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0xa, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
	char nwKey[SIZE_64_BYTES+1], *Key = NULL;

	/* fill in device default info */
	memset((char *)(&info), 0, sizeof(info));
	info.version = 0x10;

	if(getMacAddress())
		memcpy(info.macAddr, getMacAddress(), 6);  // Fill mac address

	memcpy(info.uuid, uuid, 16);
	strcpy(info.deviceName, "Broadcom Registrar");
	info.primDeviceCategory = 1;
	info.primDeviceOui = 0x0050F204;
	info.primDeviceSubCategory = 1;
	strcpy(info.manufacturer, "Broadcom");
	strcpy(info.modelName, "WPS Wireless Registrar");
	strcpy(info.modelNumber, "1234");
	strcpy(info.serialNumber, "5678");
	info.configMethods = 0x008c;
	info.authTypeFlags = 0x003f;
	info.encrTypeFlags = 0x000f;
	info.connTypeFlags = 0x01;
	info.rfBand = 1;
	info.osVersion = 0x80000000;
	info.featureId = 0x80000000;

	/* replease if need */
	if (credential) {
		/* SSID */
		memcpy(info.ssid, credential->ssid, SIZE_SSID_LENGTH);

		/* keyMgmt */
		memcpy(info.keyMgmt, credential->keyMgmt, SIZE_20_BYTES);
		/* crypto */
		info.crypto = credential->encrType;
		if(credential->encrType & WPS_ENCRTYPE_WEP)
			info.wep = 1;
		else
			info.wep = 0;

		/* nwKey */
		wps_strncpy(nwKey, credential->nwKey, sizeof(nwKey));
		Key = nwKey;
	}

	wpssta_reg_init(&info, Key, 0);
}

static void get_random_credential(WpsEnrCred *credential)
{
	/* ssid */
	uint8 *mac;
	unsigned short ssid_length, key_length;
	unsigned char random_ssid[33] = {0};
	unsigned char random_key[65] = {0};
	char macString[18];
	int i;

	memset(credential, 0, sizeof(WpsEnrCred));

	mac = getMacAddress();
	sprintf(macString, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	CeGenRandom(sizeof(ssid_length), (unsigned char *)&ssid_length);
	ssid_length = (unsigned short)((((long)ssid_length + 56791)*13579)%23) + 1;

	printf("SSID length %d\n", ssid_length);

	CeGenRandom(ssid_length, random_ssid);

	for (i = 0; i < ssid_length; i++) 
	{
		if ((random_ssid[i] < 48) || (random_ssid[i] > 126))
			random_ssid[i] = random_ssid[i]%79 + 48;
	}

	random_ssid[ssid_length++] = macString[6];
	random_ssid[ssid_length++] = macString[7];
	random_ssid[ssid_length++] = macString[9];
	random_ssid[ssid_length++] = macString[10];
	random_ssid[ssid_length++] = macString[12];
	random_ssid[ssid_length++] = macString[13];
	random_ssid[ssid_length++] = macString[15];
	random_ssid[ssid_length++] = macString[16];
	strcpy(credential->ssid, (char *)random_ssid);

	/* keyMgmt */
	strcpy(credential->keyMgmt, "WPA2-PSK");

	/* network key */
	CeGenRandom(sizeof(key_length), (unsigned char *)&key_length);
	key_length = (unsigned short)((((long)key_length + 56791)*13579)%8) + 8;
	i = 0;
	while (i < key_length) {
		CeGenRandom(1, &random_key[i]);
		if ((islower(random_key[i]) || isdigit(random_key[i])) && (random_key[i] < 0x7f)) {
			i++;
		}
	}
	memset(credential->nwKey, 0, SIZE_64_BYTES);
	credential->nwKeyLen = (uint32)key_length;
	strncpy(credential->nwKey, (char *)random_key, credential->nwKeyLen);

	/* Crypto */
	credential->encrType = ENCRYPT_AES;
}

int SetOid(ULONG oid, void* data, ULONG nbytes)
{
	int status = 0;
	HANDLE irh = getAdapterHandle();

	if (ir_setinformation((HANDLE)(void *)irh, WL_OID_BASE + oid, data, &nbytes) == NO_ERROR )
		status = 1;
	return status;
}

int QueryOid(ULONG oid, void* results, ULONG nbytes)
{
	int status = 0;
	HANDLE irh = getAdapterHandle();

	if (ir_queryinformation((HANDLE)(void *)irh, WL_OID_BASE + oid, results, &nbytes) == NO_ERROR)
		status = 1;
	return status;
}

int CheckForAssociation(char *bssid)
{
	int ret = 0;
	struct ether_addr bssidAssociated;
	if ((ret = QueryOid(WLC_GET_BSSID, &bssidAssociated, ETHER_ADDR_LEN)) == 1) {
		if (bssid[0] == bssidAssociated.octet[0] &&
            bssid[1] == bssidAssociated.octet[1] &&
			bssid[2] == bssidAssociated.octet[2] &&
			bssid[3] == bssidAssociated.octet[3] &&
			bssid[4] == bssidAssociated.octet[4] &&
			bssid[5] == bssidAssociated.octet[5] ) {
		ret = 1;
		}
	}
	return ret;
}

int SetWZCInterfaceState(BOOL bEnable)
{
	DWORD dwInFlags = 0;
	DWORD dwStatus = 0;
	INTF_ENTRY_EX Intf;
	HINSTANCE hWZCLib = NULL;
	PFN_WZCQueryInterface pfnWZCQueryInterface = 0;
	PFN_WZCSetInterface pfnWZCSetInterface = 0; 
	PFN_WZCDeleteIntfObj    pfnWZCDeleteIntfObj;

	hWZCLib = LoadLibraryW(L"wzcsapi.dll");
	if (hWZCLib != NULL) {
		pfnWZCQueryInterface    = (PFN_WZCQueryInterface)GetProcAddress(hWZCLib,L"WZCQueryInterface");
		pfnWZCSetInterface      = (PFN_WZCSetInterface)GetProcAddress(hWZCLib,L"WZCSetInterface");
		pfnWZCDeleteIntfObj     = (PFN_WZCDeleteIntfObj)GetProcAddress(hWZCLib,L"WZCDeleteIntfObj");

		if ((pfnWZCQueryInterface == NULL)   ||
			(pfnWZCSetInterface == NULL)     ||
			(pfnWZCDeleteIntfObj == NULL)) {
				FreeLibrary(hWZCLib);
				return 0;
		}

		memset(&Intf, 0x00, sizeof(INTF_ENTRY));
		Intf.wszGuid = getAdapterName();

		// Query zero config info 
		if ((dwStatus = pfnWZCQueryInterface(
			NULL,
			INTF_ALL,
			&Intf,
			&dwInFlags)) != ERROR_SUCCESS) {
				printf("!WZCUI: WZCQueryInterface failed 0x%X\r\n",dwStatus);
				FreeLibrary(hWZCLib);
				return 0; 
		}

		if (bEnable)
			Intf.dwCtlFlags |= INTFCTL_ENABLED;
		else 
			Intf.dwCtlFlags &= ~INTFCTL_ENABLED;

		dwStatus = pfnWZCSetInterface(NULL, INTF_ALL_FLAGS | INTF_PREFLIST, &Intf, 
			&dwInFlags);

		pfnWZCDeleteIntfObj(&Intf);
		FreeLibrary(hWZCLib);
	}
	return (dwStatus == ERROR_SUCCESS ? 1 : 0);
}

int ConfigureWZCInterface(BOOL bEnable)
{
	DWORD dwStatus = 0;
	WZC_CONTEXT WzcContext = {0};
	HINSTANCE hWZCLib = NULL;
	DWORD dwTimerInFlags = 0x00;
	PFN_WZCQueryContext       pfnWZCQueryContext = 0;
	PFN_WZCSetContext         pfnWZCSetContext = 0;

	hWZCLib = LoadLibraryW(L"wzcsapi.dll");
	if (hWZCLib != NULL) {
		pfnWZCQueryContext     = (PFN_WZCQueryContext)GetProcAddress(hWZCLib,L"WZCQueryContext");
		pfnWZCSetContext     = (PFN_WZCSetContext)GetProcAddress(hWZCLib,L"WZCSetContext");

		if ((pfnWZCQueryContext == NULL)       ||
			(pfnWZCSetContext == NULL)) {
				FreeLibrary(hWZCLib);
				return 0;
		}

		dwStatus = pfnWZCQueryContext(NULL, dwTimerInFlags, &WzcContext, NULL);
		if (dwStatus == ERROR_SUCCESS)
		{
			if (!bEnable)
			{
				memcpy(&g_WzcContext, &WzcContext, sizeof(WZC_CONTEXT));
				g_bWzcContextInitialized = true;
				WzcContext.tmTr = TMMS_INFINITE;
				WzcContext.tmTp =  TMMS_INFINITE;
				WzcContext.tmTc = TMMS_INFINITE;
				WzcContext.tmTf =  TMMS_INFINITE;
			}
			else
			{
				if (!g_bWzcContextInitialized || 
						TMMS_INFINITE == g_WzcContext.tmTr ||
						TMMS_INFINITE == g_WzcContext.tmTp ||
						TMMS_INFINITE == g_WzcContext.tmTc ||
						TMMS_INFINITE == g_WzcContext.tmTp)
				{
					g_WzcContext.tmTr = 3000;
					g_WzcContext.tmTp =  2000;
					g_WzcContext.tmTc = 0x70000000;
					g_WzcContext.tmTf =  60000;
				}
				memcpy(&WzcContext, &g_WzcContext, sizeof(WZC_CONTEXT));
			}
			dwStatus = pfnWZCSetContext(NULL, dwTimerInFlags, &WzcContext, NULL);
		}

		FreeLibrary(hWZCLib);
	}
	return (dwStatus == ERROR_SUCCESS ? 1 : 0);
}

int ConfigureAdapter(PWCHAR pAdapter, DWORD dwCommand)
{
	HANDLE ndis;
	TCHAR multi[100];
	int len = 0;

	if (pAdapter)
		len = _tcslen(pAdapter);

	if (len == 0 || len > 80)
		return 0;

	ndis = CreateFile(DD_NDIS_DEVICE_NAME, GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, 0, NULL);
	if (ndis == INVALID_HANDLE_VALUE) {
		return 0;
	}

	len++;
	memcpy(multi, pAdapter, len * sizeof(TCHAR));
	memcpy(&multi[len], TEXT("NDISUIO\0"), 9 * sizeof(TCHAR));
	len += 9;

	if (!DeviceIoControl(ndis, dwCommand,
		multi, len * sizeof(TCHAR), NULL, 0, NULL, NULL)) {
			CloseHandle(ndis);
			return FALSE;
	}

	CloseHandle(ndis);
	return 1;
}

int is_number(char* cmd)
{
	int ret = 0;
	int i;
	if (cmd)
	{
		ret = 1;
		for(i=0; i<(int)strlen(cmd);i++)
		{
			if (cmd[i]<'0' || cmd[i] > '9')
			{
				ret = 0;
				break;
			}
		}
	}
	return ret;
}

/**********************************************************************************************/
/* WPS SDK supporting functions                                                               */
/**********************************************************************************************/
// Given a list of wps aps, find the pbc ap and set it to the first one in the given ap list
static int get_pbc_ap(wps_ap_list_info_t *list_inout, int count, int *nAP)
{
	char bssid[6];  // bssid is a 48-bit identifier
	char ssid[32] = { 0 };
	uint8 wep = 1;
	int nRet = PBC_NOT_FOUND;
	int i=0;

	*nAP = 0;
	nRet = wps_get_pbc_ap(&list_inout[0], bssid, ssid, &wep, get_current_time(), (char)1);
	if(nRet == PBC_FOUND_OK)
	{
		// Search the wps ap list and set the pbc ap (only one is allowed currently) to the first 
		// one in this given ap list
		while(i<count)
		{
			if(_stricmp(list_inout[i].ssid, ssid) == 0)
			{
				// if i=0, the list one in the ap list is pbc ap, no need to copy
				if(i > 0)
					memcpy(&list_inout[0], &list_inout[i], sizeof(wps_ap_list_info_t));
				*nAP = 1;
				break;
			}
			i++;
		}
	}
	return nRet;
}

bool CallClientCallback(unsigned int uiStatus, void *data)
{
	if(!gIsOpened)
		return false;

	if(g_wps_join_callback!=NULL) 
	{
		if(!g_wps_join_callback(gContext,uiStatus,data))
		{
			g_wps_join_callback(gContext,WPS_STATUS_CANCELED,NULL);
			g_wps_join_callback=NULL; // Disable any more notification to the client at this point
			return FALSE;
		}
	}

	return TRUE;

}

// encrypt WEP key material
// note: this is simply for the security (to protect from memory scanning)
void EncryptWepKMaterial( WZC_WLAN_CONFIG* pwzcConfig)
{
	int i = 0;
    BYTE chFakeKeyMaterial[] = { 0x56, 0x09, 0x08, 0x98, 0x4D, 0x08, 0x11, 0x66, 0x42, 0x03, 0x01, 0x67, 0x66 };
    for (i = 0; i < WZCCTL_MAX_WEPK_MATERIAL; i++)
        pwzcConfig->KeyMaterial[i] ^= chFakeKeyMaterial[(7*i)%13];
}   // EncryptWepKMaterial()

// interpret key value then fill wzcConfig1.KeyLength and KeyMaterial[]
// wzcConfig1.Privacy should be initialized before calling.
// key is interpreted differently based on the wzcConfig1.Privacy
// wzcConfig1.Privacy could be one of these
//      Ndis802_11WEPEnabled = WEP key
//      Ndis802_11Encryption2Enabled = TKIP/WPA key
bool InterpretEncryptionKeyValue(WZC_WLAN_CONFIG* wzcConfig1, WCHAR *szEncryptionKey, PFN_WZCPassword2Key pfnWZCPassword2Key)
{
	WCHAR *szEncryptionKeyValue = 0;
	char szEncryptionKeyValue8[64]; // longest key is 63
	UINT i=0;
	if(wzcConfig1->Privacy == Ndis802_11WEPEnabled)
	{
		if((szEncryptionKey[0] < L'1') || (szEncryptionKey[0] > L'4') || (szEncryptionKey[1]!=L'/'))
		{
			return false;
		}
		wzcConfig1->KeyIndex = szEncryptionKey[0] - L'1';

		szEncryptionKeyValue = szEncryptionKey + 2;
		wzcConfig1->KeyLength = wcslen(szEncryptionKeyValue);
		if((wzcConfig1->KeyLength==5) || (wzcConfig1->KeyLength==13))
		{
			for(i=0; i<wzcConfig1->KeyLength; i++)
				wzcConfig1->KeyMaterial[i] = (UCHAR) szEncryptionKeyValue[i];
		}
		else
		{
			if((szEncryptionKeyValue[0]!=L'0') || (szEncryptionKeyValue[1]!=L'x'))
			{
				return false;
			}
			szEncryptionKeyValue += 2;
			wzcConfig1->KeyLength = wcslen(szEncryptionKeyValue);

			if((wzcConfig1->KeyLength!=10) && (wzcConfig1->KeyLength!=26))
			{
				return false;
			}

			wzcConfig1->KeyLength >>= 1;
			for(i=0; i<wzcConfig1->KeyLength; i++)
				wzcConfig1->KeyMaterial[i] = (HEX(szEncryptionKeyValue[2*i])<<4) | HEX(szEncryptionKeyValue[2*i+1]);
		}
		EncryptWepKMaterial(wzcConfig1);
		wzcConfig1->dwCtlFlags |= WZCCTL_WEPK_PRESENT;
	}
	else if (wzcConfig1->Privacy == Ndis802_11Encryption2Enabled
	  	 || wzcConfig1->Privacy == Ndis802_11Encryption3Enabled)
	{
		// TKIP key
		// -key 12345678   [8-char]
		// -key HelloWorld [10-char]
		// -key abcdefghij1234567890abcdefghij1234567890abcdefghij1234567890abc [63-char]


		wzcConfig1->KeyLength = wcslen(szEncryptionKey);
		if(wzcConfig1->KeyLength == 64) {
			wzcConfig1->KeyLength >>= 1;
			for(i=0; i<wzcConfig1->KeyLength; i++)
				wzcConfig1->KeyMaterial[i] = (HEX(szEncryptionKey[2*i])<<4) | HEX(szEncryptionKey[2*i+1]);
		} else if((wzcConfig1->KeyLength<8) || (wzcConfig1->KeyLength>63)) {
			return false;
		} else {

			// WPA/TKIP pre-shared key takes 256 bit key.
			// Everything else is incorrect format.
			// Translates a user password (8 to 63 ascii chars) into a 256 bit network key.
			// We do this for WPA-PSK and WPA-None.

			memset(szEncryptionKeyValue8, 0, sizeof(szEncryptionKeyValue8));
			WideCharToMultiByte(CP_ACP,
				0,
				szEncryptionKey,
				wzcConfig1->KeyLength+1,
				szEncryptionKeyValue8,
				wzcConfig1->KeyLength+1,
				NULL,
				NULL);
			pfnWZCPassword2Key(wzcConfig1, szEncryptionKeyValue8);
		}
		EncryptWepKMaterial(wzcConfig1);
		wzcConfig1->dwCtlFlags |= WZCCTL_WEPK_XFORMAT
			| WZCCTL_WEPK_PRESENT
			| WZCCTL_ONEX_ENABLED;

		wzcConfig1->EapolParams.dwEapFlags = EAPOL_ENABLED;
		wzcConfig1->EapolParams.dwEapType = DEFAULT_EAP_TYPE;
		wzcConfig1->EapolParams.bEnable8021x = TRUE;
		wzcConfig1->WPAMCastCipher = Ndis802_11Encryption2Enabled;
	}
	return true;
}   // InterpretEncryptionKeyValue()

bool CreateProfile(const wps_credentials *credentials)
{
	bool bRet = false;

	//see if WZC is controlling the adapter
	HKEY hk;
	DWORD dwInFlags = 0;
    DWORD dwOutFlags;
	DWORD dwStatus = 0;
	INTF_ENTRY_EX Intf;
	HINSTANCE hWZCLib = NULL;
	PFN_WZCQueryInterface pfnWZCQueryInterface = 0;
	PFN_WZCSetInterface pfnWZCSetInterface = 0; 
	PFN_WZCDeleteIntfObj    pfnWZCDeleteIntfObj;
	PFN_WZCPassword2Key      pfnWZCPassword2Key;
	WZC_802_11_CONFIG_LIST *pConfigList = 0;
	WZC_802_11_CONFIG_LIST *pNewConfigList = 0;
	DWORD dwDataLen = 0;
	ULONG uiNumberOfItems = 0;
	WCHAR szSsidToConnect[SIZE_32_BYTES+1];
	WZC_WLAN_CONFIG wzcConfig1;
    WCHAR szEncryptionKey[SIZE_64_BYTES+1];
	char szWepIndex[SIZE_64_BYTES+1];

	UINT i = 0;
	LONG ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, WZC_DRIVER, 0, 0, &hk);
	if (ret != ERROR_SUCCESS) {
		RegCloseKey(hk);
		return false;
	}

	hWZCLib = LoadLibraryW(L"wzcsapi.dll");
	if (hWZCLib != NULL) {
		pfnWZCQueryInterface    = (PFN_WZCQueryInterface)GetProcAddress(hWZCLib,L"WZCQueryInterface");
		pfnWZCSetInterface      = (PFN_WZCSetInterface)GetProcAddress(hWZCLib,L"WZCSetInterface");
		pfnWZCDeleteIntfObj     = (PFN_WZCDeleteIntfObj)GetProcAddress(hWZCLib,L"WZCDeleteIntfObj");
		pfnWZCPassword2Key      = (PFN_WZCPassword2Key)GetProcAddress(hWZCLib,L"WZCPassword2Key");

		if ((pfnWZCQueryInterface == NULL)   ||
			(pfnWZCSetInterface == NULL)     ||
			(pfnWZCDeleteIntfObj == NULL) ||
			(pfnWZCPassword2Key == NULL)) {
				FreeLibrary(hWZCLib);
				return false;
		}

		memset(&Intf, 0x00, sizeof(INTF_ENTRY));
		Intf.wszGuid = getAdapterName();

		if ((dwStatus = pfnWZCQueryInterface(
			NULL,
			INTF_PREFLIST,
			&Intf,
			&dwInFlags)) != ERROR_SUCCESS) {
				pfnWZCDeleteIntfObj(&Intf);
				FreeLibrary(hWZCLib);
				return false; 
		}

		//Add to preferred network
	    memset(&wzcConfig1, 0, sizeof(wzcConfig1));
		wzcConfig1.Length = sizeof(wzcConfig1);
		wzcConfig1.dwCtlFlags = 0;

		MultiByteToWideChar(CP_ACP,0,credentials->ssid,-1,szSsidToConnect,SIZE_32_BYTES+1);

		//SSID
	    wzcConfig1.Ssid.SsidLength = wcslen(szSsidToConnect);
		if(wzcConfig1.Ssid.SsidLength >= 32)
		{
			wzcConfig1.Ssid.SsidLength = 32;
		}
		for(i=0; i<wzcConfig1.Ssid.SsidLength; i++)
			wzcConfig1.Ssid.Ssid[i] = (char)szSsidToConnect[i];

	    // infrastructure
		wzcConfig1.InfrastructureMode = Ndis802_11Infrastructure;

	    // Authentication
		wzcConfig1.AuthenticationMode = Ndis802_11AuthModeOpen;

		if (strcmp(credentials->keyMgmt, "SHARED") == 0)
			wzcConfig1.AuthenticationMode = Ndis802_11AuthModeShared;
		else if (strcmp(credentials->keyMgmt, "WPA-PSK") == 0)
			wzcConfig1.AuthenticationMode = Ndis802_11AuthModeWPAPSK;
		else if (strcmp(credentials->keyMgmt, "WPA2-PSK") == 0)
			wzcConfig1.AuthenticationMode = Ndis802_11AuthModeWPA2PSK;
		else if (strcmp(credentials->keyMgmt, "WPA-PSK WPA2-PSK") == 0)
			wzcConfig1.AuthenticationMode = Ndis802_11AuthModeWPA2PSK;


		// Encryption
		wzcConfig1.Privacy = Ndis802_11WEPDisabled;

		if(credentials->encrType & WPS_ENCRYPT_WEP)
		{
			wzcConfig1.Privacy = Ndis802_11WEPEnabled;
			sprintf(szWepIndex, "%d/0x", credentials->wepIndex);
			strcat(szWepIndex, credentials->nwKey);
		}
		else if(credentials->encrType & WPS_ENCRYPT_AES) 
			wzcConfig1.Privacy = Ndis802_11Encryption3Enabled;
		else if(credentials->encrType & WPS_ENCRYPT_TKIP)
			wzcConfig1.Privacy = Ndis802_11Encryption2Enabled;
		
		// Key
		if(wzcConfig1.Privacy == Ndis802_11WEPEnabled)
		{
			MultiByteToWideChar(CP_ACP,0,szWepIndex,-1,szEncryptionKey,SIZE_64_BYTES+1);
		}
		else
		{
			MultiByteToWideChar(CP_ACP,0,credentials->nwKey,-1,szEncryptionKey,SIZE_64_BYTES+1);
		}

		if (InterpretEncryptionKeyValue(&wzcConfig1, &szEncryptionKey[0], pfnWZCPassword2Key))
		{
			pConfigList = (PWZC_802_11_CONFIG_LIST)Intf.rdStSSIDList.pData;
			if(!pConfigList)   // empty [Preferred Networks] list case
			{
				dwDataLen = sizeof(WZC_802_11_CONFIG_LIST);
				pNewConfigList = (WZC_802_11_CONFIG_LIST *)LocalAlloc(LPTR, dwDataLen);
				pNewConfigList->NumberOfItems = 1;
				pNewConfigList->Index = 0;
				memcpy(pNewConfigList->Config, &wzcConfig1, sizeof(wzcConfig1));
				Intf.rdStSSIDList.pData = (BYTE*)pNewConfigList;
				Intf.rdStSSIDList.dwDataLen = dwDataLen;
			}
			else
			{
				int iProfileIndex = -1; //see if profile already present
				uiNumberOfItems = pConfigList->NumberOfItems;
				for(i=0; i<uiNumberOfItems; i++)
				{
					if(memcmp(&wzcConfig1.Ssid, &pConfigList->Config[i].Ssid, sizeof(NDIS_802_11_SSID)) == 0)
					{
						iProfileIndex = i;
						break;
					}
				}

				if (iProfileIndex == -1) //profile not found
				{
					dwDataLen = sizeof(WZC_802_11_CONFIG_LIST) + (uiNumberOfItems+1)*sizeof(WZC_WLAN_CONFIG);
					pNewConfigList = (WZC_802_11_CONFIG_LIST *)LocalAlloc(LPTR, dwDataLen);
					pNewConfigList->NumberOfItems = uiNumberOfItems + 1;
					pNewConfigList->Index = 0;

					memcpy(pNewConfigList->Config, &wzcConfig1, sizeof(wzcConfig1));
					if(pConfigList->NumberOfItems)
					{
						pNewConfigList->Index = pConfigList->Index;
						memcpy(pNewConfigList->Config+1, pConfigList->Config, (uiNumberOfItems)*sizeof(WZC_WLAN_CONFIG));
						LocalFree(pConfigList);
						pConfigList = NULL;
					}

					Intf.rdStSSIDList.pData = (BYTE*)pNewConfigList;
					Intf.rdStSSIDList.dwDataLen = dwDataLen;
				}
				else //profile present. update with new credentials
				{
					dwDataLen = sizeof(WZC_802_11_CONFIG_LIST) + (uiNumberOfItems)*sizeof(WZC_WLAN_CONFIG);
					memcpy(&pConfigList->Config[iProfileIndex], &wzcConfig1, sizeof(WZC_WLAN_CONFIG));
					Intf.rdStSSIDList.pData = (BYTE*)pConfigList;
					Intf.rdStSSIDList.dwDataLen = dwDataLen;
				}
			}

			dwStatus = pfnWZCSetInterface(NULL, INTF_PREFLIST, &Intf, &dwOutFlags);
			bRet = (dwStatus == ERROR_SUCCESS) ? true : false;
		}

		pfnWZCDeleteIntfObj(&Intf);
		FreeLibrary(hWZCLib);
	}

	return bRet;
}

/**********************************************************************************************/
/* End of supporting functions                                                                */
/**********************************************************************************************/

/**********************************************************************************************/
/* WPS SDK APIs                                                                               */
/**********************************************************************************************/
/*
 wps_open function must be called first, before any other wps api call
*/
DLLExport bool wps_open(void *context, fnWpsProcessCB callback)
{
	if(gIsOpened) return false;

	gContext=NULL;
	g_wps_join_callback=NULL;

		/*relaod winpcap driver in case a previous application was terminated in an improper way */
	if (!reload_driver()) {
		DBGPRINT(("Failed to initialize packet capture driver."));
		return false;
	}

	/* Set the priority of thread to a lower value since in our driver the DPCPriority is 0xfb 
	and this is also the default priority for WPS enrollee on PXA. */
	CeSetThreadPriority(GetCurrentThread(), 0xfe);

	if (!init_adapter())
		return false;

	gContext=context;
	g_wps_join_callback=callback;
	gIsOpened=true;

	CallClientCallback(WPS_STATUS_INIT, NULL);
	
	/* 
	setup device configuration for WPS 
	needs to be done before eventual scan for PBC.
	*/ 

	config_init();
	
	return true;

}

/*
 wps_close function must be called once you are done using the wps api
*/
DLLExport bool wps_close(void)
{
	DWORD dwExitCode = 0;
	WCHAR device[] = L"{98C5250D-C29A-4985-AE5F-AFE5367E5006}\\bcmsddhd1";

	if(!gIsOpened) return false;
	
	gContext=NULL;
	g_wps_join_callback=NULL;
	gIsOpened=false;

	wps_cleanup();
	// doing low level cleanup
	wps_osl_cleanup();

	return true;
}

/* 
Since Wi-Fi managing utilities such as Windows ZeroConfig can interfere with operation of 
WPS SDK, they need to be disabled for proper operation of WPS SDK APIs. The application should use this API if 
the platform makes use of Windows Zeroconfig service to manage the adapter. If not, the application
can define its own method for disabling and enabling any other Wi-Fi managing service.
*/
DLLExport bool wps_configure_wzcsvc(bool bEnable)
{
	int bRet = 0;
	HKEY   hkey;
	DWORD dwErrRpt = -1, dwDisable = 0;
	bool bRegSuccess = false;
	DWORD  dwDisposition = 0,dwType = 0, dwSize = 0;
	LONG ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, WZC_DRIVER, 0, 0, &hkey);
	if (ret == ERROR_SUCCESS) {
		RegCloseKey(hkey);
		bRet = 1;
	}
	else {
		return TRUE; /* Do not do anything */
	}

	if(bEnable)
		CallClientCallback(WPS_STATUS_ENABLING_WIFI_MANAGEMENT, NULL);
	else
		CallClientCallback(WPS_STATUS_DISABLING_WIFI_MANAGEMENT, NULL);

#if DISABLE_WZC
	if(RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("System\\ErrorReporting\\DumpSettings"), 
		0, NULL, 0, 0, NULL, &hkey, &dwDisposition) == ERROR_SUCCESS) {
		dwType = REG_DWORD;
		dwSize = sizeof(DWORD);
		bRegSuccess = true;
		
		RegQueryValueEx(hkey, TEXT("DumpEnabled"), NULL, &dwType, 
			(PBYTE)&dwErrRpt, &dwSize);
		
		RegSetValueEx(hkey, TEXT("DumpEnabled"), 0, dwType, 
        (PBYTE)&dwDisable, dwSize);
	}

	if (bRet)
	{
		ConfigureAdapter(getAdapterName(), IOCTL_NDIS_UNBIND_ADAPTER);
		if (bRet)
		{
			bRet = ConfigureWindowsZeroConfig(bEnable);
			bRet &= ConfigureAdapter(getAdapterName(), IOCTL_NDIS_BIND_ADAPTER);
		}
	}

	if(bRegSuccess) {
		RegSetValueEx(hkey, TEXT("DumpEnabled"), 0, dwType, 
			(PBYTE)&dwErrRpt, dwSize);
		RegCloseKey(hkey);
	}
#else
	bRet = ConfigureWZCInterface(bEnable);
	ConfigureAdapter(getAdapterName(), IOCTL_NDIS_REBIND_ADAPTER);
	WpsSleep(2000);
#endif /* DISABLE_WZC */
	return bRet;
}

/*
 wps_findAP scans for WPS PBC APs and returns the one with the strongest RSSI
 Returns true if it finds an AP within the specified time. This function is designed to
 be called repeatidly with small timeouts in seconds (say 4 or 5 secs) to allow for UI updates and user
 cancelation. If multiple PBC APs are found, this is an error condition and FALSE is returned. 8nAP will
 contain the number of PBC APs found (will be greater than 1).

 The value of *nAP is updated with the number of APs found. For PBC APs, it will be always 1 on success (or
 if more than 1 is returned, the UI should warn the user to try again later).
 For PIN APs, it will varie from 0 to the max numbers of the list.

 Call wps_getAP to get the APs found
*/
DLLExport bool wps_findAP(int *nAP, int mode, int timeout)
{
	int wps_ap_total = 0;
	wps_ap_list_info_t *wpsaplist;
	uint32 start_time;
	char ssid[32] = { 0 };
	uint8 wep = 1;
	bool bRet = false;
	int nWpsStatus = WPS_STATUS_SCANNING_OVER_SUCCESS;
	int nFindPbcAp;

	if(!gIsOpened) return false;

	add_wps_ie(NULL, 0);  // add wps ie to probe
	*nAP = 0;
	start_time = get_current_time();
	while((start_time+timeout)>get_current_time()) 
	{

		if(!CallClientCallback(WPS_STATUS_SCANNING,NULL)) 
			return false;
		
		wpsaplist = create_aplist();  // Get the pointer of ap_list (global)
		if(wpsaplist)
		{
			// After this call, the first wps_ap_total elements in the ap_list are wps ones
			wps_ap_total = wps_get_aplist(wpsaplist, wpsaplist);  
			if(wps_ap_total > 0)
			{
				if(mode == STA_ENR_JOIN_NW_PBC)
				{
					// pbc mode 
					nFindPbcAp = get_pbc_ap(wpsaplist, wps_ap_total, nAP);
					
					if(nFindPbcAp == PBC_NOT_FOUND || nFindPbcAp == PBC_TIMEOUT)
						continue;  // if no pbc ap is scanned, continue to scan
					else if(nFindPbcAp == PBC_OVERLAP)
					{
						nWpsStatus = WPS_STATUS_SCANNING_OVER_SESSION_OVERLAP;
						goto FIND_AP_END;
					}
					else if(nFindPbcAp == PBC_FOUND_OK)
					{
						nWpsStatus = WPS_STATUS_SCANNING_OVER_SUCCESS;
						break;
					}
				}
				else
				{
					// pin mode, simply return all wps aps
					*nAP = wps_ap_total;
					break;
				}
			}
		}
		WpsSleep(100);
	}

	if(*nAP > 0)
		nWpsStatus = WPS_STATUS_SCANNING_OVER_SUCCESS;
	else
		nWpsStatus = WPS_STATUS_SCANNING_OVER_NO_AP_FOUND;

FIND_AP_END:
	rem_wps_ie(NULL, 0);
	CallClientCallback(nWpsStatus, NULL);

	return (*nAP > 0);
}

/*
 wps_getAP returns the AP #nAP from the list of WPS APs found by wps_findAP.
*/
DLLExport bool wps_getAP(int nAP, unsigned char * bssid, char *ssid, uint8 *wep, uint16 *band)
{
	int i=0;
	wps_ap_list_info_t *ap;

	if(!gIsOpened) return false;

	ap=wps_get_ap_list();

	if(nAP < (WPS_DUMP_BUF_LEN / sizeof(wps_ap_list_info_t)))
	{
		if (ap[nAP].used == TRUE )
		{
			for(i=0;i<6;i++)
				bssid[i] = ap[nAP].BSSID[i];
			memcpy(ssid,ap[nAP].ssid,ap[nAP].ssidLen);
			ssid[ap[nAP].ssidLen] = '\0';
			*wep = ap[nAP].wep;
			*band = ap[nAP].band;
			return true;
		}
	}
	return false;
}

/*
 wps_join function is used to connect to a WPS AP. Usualy, this function is called after
 wps_findAP returns successfully
*/
DLLExport bool wps_join(uint8 * bssid, char *ssid, uint8 wep)
{
	if(!gIsOpened) return false;
	if(!CallClientCallback(WPS_STATUS_ASSOCIATING, ssid)) return false;
	DBGPRINT(("Connecting to WPS AP %s\n",ssid));

	strcpy(stAdapterInfo.ssid,ssid);

	leave_network();
	if(join_network(ssid, wep)!=0)
	{
		DBGPRINT(("Join failed\n"));
		return false;
	}
	if(!CallClientCallback(WPS_STATUS_ASSOCIATED,ssid)) return false;

	return true;
}

/*
 This function starts the WPS exchange protocol and gathers the credentials
 of the AP. Call this function once wps_join is successful. 

 This function will return only once the WPS exchange is finished or an
 error occurred. 

 The calling process provides a callback function in wps_open() that will be called periodically by the WPS API. When called, this
 callback function will be provided with the current status. If the calling process wants to cancel the WPS protocol, it
 should return FALSE (upon the user pressing a Cancel button, for example). 
 
 If the calling process does not want to be called back, it should send NULL as a function pointer.

 GUI applications should use the asynchronous version of this function so as not to block or slow down a UI's message loop.
*/

DLLExport bool wps_get_AP_info(int wps_mode, uint8 *bssid, char *ssid, char *pin, wps_credentials *credentials)
{
	bool bRet = false;
	WpsEnrCred cred;

	if(!gIsOpened) return false;
	if(!CallClientCallback(WPS_STATUS_STARTING_WPS_EXCHANGE,NULL)) return false;

	if (wps_mode == STA_REG_JOIN_NW) 
	{
		// STA ER joining network

		// wps_cleanup is required here as we have done preliminary config_init to search WPS AP
		// we need to get back to clean state so that reg_config_init can succeed
		wps_cleanup();

		reg_config_init(NULL);
		if(wps_osl_init(bssid))
		{
			wpssta_start_registration(pin, get_current_time());
			if(registration_loop(get_current_time()) == WPS_SUCCESS) 
			{
				wps_get_reg_M7credentials(&cred);
				bRet = true;
			}
		}
		else
		{
			leave_network();
		}
	}
	else
	{
		/* This can be a WPS re-attempt. So start clean */
		wps_cleanup();
		config_init();
		
		if(enroll_device(wps_mode, bssid, ssid, pin))
		{
			// Get credentials
			wpssta_get_credentials(&cred, ssid, strlen(ssid));
			bRet = true;
		}
	}

	if(bRet)
	{
		// Output Wi-Fi credential
		memset(credentials, 0, sizeof(wps_credentials));
		strncpy(credentials->ssid, cred.ssid, SIZE_32_BYTES);
		strncpy(credentials->nwKey, cred.nwKey, SIZE_64_BYTES);
		strncpy(credentials->keyMgmt, cred.keyMgmt, strlen(cred.keyMgmt));
		credentials->encrType=cred.encrType;
		credentials->wepIndex = 1; /* cred.wepIndex */;
		CallClientCallback(WPS_STATUS_SUCCESS,NULL);
	}
	else
	{
		CallClientCallback(WPS_STATUS_ERROR,NULL);
	}

	return bRet;
}

DWORD WINAPI StartThreadGetInfo( LPVOID lpParam ) 
{
	ClientInfo *info=(ClientInfo*) lpParam;
	if(lpParam==NULL) return -1;
	if(!gIsOpened) return -1;
	wps_get_AP_info(info->mode,info->bssid,info->ssid,info->pin, info->credentials);
	free(info);
	return 0;
}


/*
 Asynchronous version of wps_get_AP_info(). This function returns immediately and starts the WPS protocol in a separate thread
 The calling process uses the status callback to determine the state of the WPS protocol.

 The calling process will get a WPS_STATUS_SUCCESS once the WPS protocol completed successfully
 The calling process will get a WPS_STATUS_ERROR if the WPS protocol completed unsuccessfully
 The calling process will get a WPS_STATUS_CANCELED if the WPS protocol was canceled by the calling thread

 The calling process must wait for any one of these 3 status notifications or any error notification
 before calling wps_close() or terminating.

 Unlike the synchronous version of this API call, the callback parameter in wps_open()CANNOT be NULL. 
 A callback is required for this function to work correctly.

 Before this function returns, it will call the calling process' callback with a status of WPS_STATUS_START_WPS_EXCHANGE

*/
DLLExport bool wps_get_AP_infoEx(int wps_mode, uint8 * bssid, char *ssid, char *pin, wps_credentials *credentials)
{
	ClientInfo *info;
	if(!gIsOpened) return false;
	if(g_wps_join_callback==NULL) return FALSE;

	info=(ClientInfo*) malloc(sizeof(ClientInfo));

	if(info)
	{
		info->bssid=bssid;
		info->ssid=ssid;
		info->pin=pin;
		info->credentials=credentials;
		info->mode = wps_mode;

		if(CreateThread(NULL,0,StartThreadGetInfo,info,0,NULL)!=NULL)
		{
			return TRUE;
		}
	}
	return FALSE;
}

/*
 This function creates a preferred network profile that can be used by Windows Zero Config (WZC)
 to connect to the network. Call this function with the results of the last WPS exchange. 

 This function will return WPS_STATUS_ERROR if WZC does not control the adapter or if the creation 
 of profile failed. 

*/

DLLExport bool wps_create_profile(const wps_credentials *credentials)
{
	CallClientCallback(WPS_STATUS_CREATING_PROFILE,NULL);
	if(credentials == NULL)
		return false;

	if(!CreateProfile(credentials))
	{
		CallClientCallback(WPS_STATUS_ERROR,NULL);
		return false;
	}
	return true;
}

DLLExport bool wps_configureAP(uint8 *bssid, const char *pin, const wps_credentials *credentials)
{
	bool bRet = false;
	WpsEnrCred credNew;

	CallClientCallback(WPS_STATUS_CONFIGURING_ACCESS_POINT,NULL);

	if(!credentials)
		get_random_credential(&credNew);
	else
	{
		memset(&credNew, 0, sizeof(credNew));
		strcpy(credNew.ssid, credentials->ssid);
		credNew.ssidLen = strlen(credentials->ssid);
		credNew.encrType = credentials->encrType;
		strcpy(credNew.keyMgmt, credentials->keyMgmt);
		strcpy(credNew.nwKey, credentials->nwKey);
		credNew.nwKeyLen = strlen(credentials->nwKey);
		credNew.wepIndex = credentials->wepIndex;
	}

	// wps_cleanup is required here as we have done preliminary config_init to search WPS AP
	// we need to get back to clean state so that reg_config_init can succeed
	wps_cleanup();

	reg_config_init(&credNew);
	if(wps_osl_init(bssid))
	{
		wpssta_start_registration((char *)pin, get_current_time());
		if (registration_loop(get_current_time()) == WPS_SUCCESS) {
			//wps_get_reg_M8credentials((wps_credentials*)credentials);
			bRet = true;
		}
	}
	else
	{
		leave_network();
	}

	if (bRet)
	{
		CallClientCallback(WPS_STATUS_SUCCESS,NULL);
	}
	else
	{
		CallClientCallback(WPS_STATUS_ERROR,NULL);
	}

	return bRet;
}


DLLExport bool wps_generate_pin(char *pin)
{
	return (wps_generatePin(pin, false) == WPS_SUCCESS);
}

DLLExport bool wps_generate_cred(wps_credentials *credentials)
{
	bool bRet = false;
	WpsEnrCred credNew;

	if(!credentials)
		return bRet;

	get_random_credential(&credNew);
	credentials->encrType = credNew.encrType;
	strcpy(credentials->keyMgmt, credNew.keyMgmt);
	strcpy(credentials->nwKey, credNew.nwKey);
	strcpy(credentials->ssid, credNew.ssid);
	credentials->wepIndex = 1;

	return true;
}

DLLExport bool wps_is_reg_activated(const uint8 *bssid)
{
	wps_ap_list_info_t *ap_list = wps_get_ap_list();
	int i = 0;
	for(i=0; i<WPS_MAX_AP_SCAN_LIST_LEN; i++)
	{
		// Find the ap according by comparing the mac address
		if(compare_mac(bssid, ap_list[i].BSSID))
			return wps_get_select_reg(&ap_list[i]);
	}
	return false;
}

DLLExport bool wps_validate_checksum(const unsigned long pin)
{
	return wps_validateChecksum(pin);
}

DLLExport uint8 wps_get_AP_scstate(const uint8 *bssid)
{
	wps_ap_list_info_t *ap_list = wps_get_ap_list();
	int i = 0;
	for(i=0; i<WPS_MAX_AP_SCAN_LIST_LEN; i++)
	{
		// Find the ap according by comparing the mac address
		if(compare_mac(bssid, ap_list[i].BSSID))
			return ap_list[i].scstate;
	}
	return WPS_SCSTATE_UNKNOWN;
}

/**********************************************************************************************/
/* End WPS SDK APIs                                                                           */
/**********************************************************************************************/


#include <iostream>
#include <tchar.h>
#include <stdio.h>
#include <conio.h>

#include "wps_sdk.h"


#define DEVICE_PASSWORD_ID(appin, pin, new_cred) \
	(((appin) && (new_cred)) ? STA_REG_CONFIG_NW : \
	(appin) ? STA_REG_JOIN_NW : \
	(pin) == NULL ? STA_ENR_JOIN_NW_PBC : \
	strlen((pin)) ? STA_ENR_JOIN_NW_PIN : STA_ENR_JOIN_NW_PBC)

#define PIN_MODE(emode)	(((emode) == STA_REG_JOIN_NW) || \
	((emode) == STA_REG_CONFIG_NW) || \
	((emode) == STA_ENR_JOIN_NW_PIN))
#define PBC_MODE(emode)	((emode) == STA_ENR_JOIN_NW_PBC)

typedef struct event_s
{
	int type;
	char buf[4096];
	uint32 buf_len;
} EVENT_T;

#define GET_EVENT_SUCCESS	1
#define GET_EVENT_ERROR		2
#define GET_EVENT_IDLE		3

#define EVENT_TYPE_WPS		1

#define PROCESS_RESULT_SUCCESS		1
#define PROCESS_RESULT_ERROR		2
#define PROCESS_RESULT_REJOIN		3
#define PROCESS_RESULT_CANCELED		4
#define PROCESS_RESULT_IDLE		5

uint8 empty_mac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

int g_result = PROCESS_RESULT_IDLE;
wps_apinf g_apinf;
unsigned int g_uiStatus = 0;

/* ###### */
/* UI STUFF  */
/* ###### */
static wps_credentials *
ui_wps_build_new_cred(wps_credentials *cred)
{
	char option[10];

	printf("\nDo you want to configure AP [N/Y]:");
	gets_s(option);

	if (option[0] == 'y' || option[0] == 'Y') {
		/* Currently use random cred for this sample code */
		memset(cred, 0, sizeof(wps_credentials));
		wps_api_generate_cred(cred);
		return cred;
	}

	return NULL;
}

static wps_credentials *
ui_wps_select_mode(wps_credentials *credentials, char *pin, int pin_size, bool *bAPPin)
{
	char option[12] = "";
	int len;
	wps_credentials *new_cred = NULL;

enter_ap_pin:
	*bAPPin = false;
	len = 0;

	/* wpsreg WPS through AP's PIN */
	printf("\nIf you have an AP's pin, enter it now, otherwise press ENTER:");
	gets_s(option);

	len = (int)strlen(option);
	if (len) {
		*bAPPin = true;
		if (wps_api_validate_checksum(option) == false) {
			printf("\nInvalid AP's pin %s!\n", option);
			goto enter_ap_pin;
		}

		new_cred = ui_wps_build_new_cred(credentials);
		printf("\nLooking for a WPS PIN AP with AP's pin %s.\n", option);
	}
	else {
enter_pin:
		len = 0;

		/* wpsenr WPS through STA's PIN */
		printf("\nIf you have a pin, enter it now, otherwise press ENTER:");
		gets_s(option);
		len = (int)strlen(option);

		if (len) {
			if (wps_api_validate_checksum(option) == false) {
				printf("\nInvalid pin %s!\n", option);
				goto enter_pin;
			}
			printf("\nLooking for a WPS PIN AP with pin %s.\n", option);
		}
		else
			printf("\nLooking for a WPS PBC AP.\n");
	}

	if (len)
		strncpy(pin, option, pin_size);

	return new_cred;
}

static bool
ui_wps_find_ap(enum eWPS_MODE mode, wps_apinf *apinf, int *nAP, bool b_v2)
{
	uint8 *mac;
	char option[10];
	bool bFoundAP = false;
	int i, j, valc, valc1;
	struct wps_ap_list_info *aplist;
	static bool bIEAdded = false;
	bool bPbcAPFound;

	if (apinf == NULL|| nAP == NULL)
		return false;

scan:
	i = 20;
	*nAP = 0;
	bFoundAP = false;

	while (bFoundAP == false && i) {
		/* Get all APs */
		aplist = wps_api_surveying(PBC_MODE(mode), b_v2, !bIEAdded);
		if(!bIEAdded)
			bIEAdded = true;

		/* Try to find:
		* 1. A PBC enabled AP when we WPS through PBC
		* 2. Collect all WPS APs when we WPS through AP's PIN or PIN
		*/
		/* TODO: in STA PIN mode
		* 1. find_pin_aps : find all APs which has PIN my MAC in AuthorizedMACs
		* 2. find_amac_ap : find an AP which has my MAC in AuthorizedMACs
		*/
		bFoundAP = wps_api_find_ap(aplist, nAP, PBC_MODE(mode), NULL, true, &bPbcAPFound, false);

		if (bFoundAP) {
			/* AP's PIN or PIN */
			if (PIN_MODE(mode) && *nAP > 0) {
				i = 0;
				printf("\n----------------------------------------------------\n");
				/* Retrieve index i AP info for display all WPS APs */
				while (wps_api_get_ap(i, apinf)) {
					printf(" %-2d :  ", i+1);
					printf("SSID:%-16s  ", apinf->ssid);
					printf("BSSID:%02x:%02x:%02x:%02x:%02x:%02x  ",
						apinf->bssid[0], apinf->bssid[1], apinf->bssid[2],
						apinf->bssid[3], apinf->bssid[4], apinf->bssid[5]);
					printf("Channel:%-3d  ", apinf->channel);
					if (apinf->wep)
						printf("WEP  ");
					if (b_v2 && apinf->version2 != 0) {
						printf("V2(0x%02X)  ", apinf->version2);

						mac = apinf->authorizedMACs;
						printf("AuthorizedMACs:");
						for (j = 0; j < 5; j++) {
							if (memcmp(mac, empty_mac, 6) == 0)
								break;

							printf(" %02x:%02x:%02x:%02x:%02x:%02x",
								mac[0], mac[1], mac[2], mac[3],
								mac[4], mac[5]);
							mac += 6;
						}
					}
					printf("\n");
					i++;
				}
				/* Select one to WPS */
				printf("----------------------------------------------------\n");
				printf("\nPlease enter the AP number you wish to connect to.\n"
					"Or enter 0 to search again or x to quit:");
				gets_s(option);

				if (option[0] == 'x' || option[0] == 'X') {
					bFoundAP = false;
					*nAP = -1;
					break;
				}

				if (option[0] < '0' || option[0] > '9')
					goto scan;

				valc = option[0] - '0';
				if ('0' <= option[1] && '9' >= option[1]) {
					valc1 = option[1] - '0';
					valc = valc * 10;
					valc += valc1;
				}

				if (valc > *nAP)
					goto scan;

				/* Retrieve PIN index valc AP info for WPS */
				if (wps_api_get_ap(valc - 1, apinf) == false) {
					printf("Error, wrong number entered!\n");
					goto scan;
				}

				/* Add confirmation check if AP is configured */
				if (mode == STA_REG_CONFIG_NW && apinf->configured) {
					printf("\nDo you want to configure AP [Y/N]:");
					printf("\n%s is a configured network."
						" Are you sure you want to "
						"overwrite existing network"
						" settings? [Y/N]:", apinf->ssid);
					gets_s(option);

					if (option[0] == 'n' || option[0] == 'N') {
						bFoundAP = false;
						*nAP = -1;
						break;
					}
				}
			}
			/* PBC */
			else if (PBC_MODE(mode) && *nAP > 0) {
				if (*nAP > 1) {
					printf("More than one PBC AP found. Restarting scanning\n");
					bFoundAP = false;
				} else {
					/* Retrieve PBC index 0 AP info for WPS */
					wps_api_get_ap(0, apinf);
				}
			}
		} else {
			printf("Did not find a WPS AP.\nPress X to quit, <Enter> to continue\n");
			gets_s(option);

			/* Remove the \n character too. */
			//option[strlen(option)-1] = '\0';

			if (option[0] == 'x' || option[0] == 'X') {
				printf("\nCANCEL REQUESTED BY USER. CANCELING, PLEASE WAIT...\n");
				bFoundAP = false;
				*nAP = -1;
				break;
			}
		}

		i--;
	}
	printf("\n");

	return bFoundAP;
}


/* ######### */
/* WPS STUFF */
/* ######### */
void
_wps_join_callback(void *context, unsigned int uiStatus, void *data)
{
	g_uiStatus = uiStatus;

	switch(uiStatus) {
	case WPS_STATUS_DISABLING_WIFI_MANAGEMENT:
		printf("STATUS: DISABLING_WIFI_MANAGEMENT\n");
		break;
	case WPS_STATUS_SCANNING:
		printf("STATUS: SCANNING\n");
		break;
	case WPS_STATUS_SCANNING_OVER_SUCCESS:
		printf("STATUS: SCANNING OVER. WPS AP FOUND\n");
		break;
	case WPS_STATUS_ASSOCIATING:
		printf("STATUS: ASSOCIATING TO %s\n", (char*) data);
		break;
	case WPS_STATUS_ASSOCIATED:
		printf("STATUS: ASSOCIATED TO %s\n", (char*) data);
		break;
	case WPS_STATUS_STARTING_WPS_EXCHANGE:
		printf("STATUS: STARTING_WPS_EXCHANGE\n");
		break;
	case WPS_STATUS_SENDING_WPS_MESSAGE:
		printf("STATUS: SENDING_WPS_MESSAGE\n");
		break;
	case WPS_STATUS_WAITING_WPS_RESPONSE:
		printf("STATUS: WAITING_WPS_RESPONSE\n");
		break;
	case WPS_STATUS_GOT_WPS_RESPONSE:
		printf("STATUS: GOT_WPS_RESPONSE\n");
		break;
	case WPS_STATUS_DISCONNECTING:
		printf("STATUS: DISCONNECTING\n");
		break;
	case WPS_STATUS_ENABLING_WIFI_MANAGEMENT:
		printf("STATUS: ENABLING_WIFI_MANAGEMENT\n");
		break;
	case WPS_STATUS_CREATING_PROFILE:
		printf("STATUS: CREATING_PROFILE\n");
		break;
	case WPS_STATUS_SUCCESS:
		printf("STATUS: SUCCESS\n");
		break;
	case WPS_STATUS_CANCELED:
		printf("STATUS: CANCELED\n");
		break;
	//case WPS_STATUS_WARNING_TIMEOUT:
	//	printf("STATUS: ERROR_TIMEOUT\n");
	//	break;
	case WPS_STATUS_WARNING_WPS_PROTOCOL_FAILED:
		printf("STATUS: ERROR_WPS_PROTOCOL\n");
		break;
	case WPS_STATUS_WARNING_NOT_INITIALIZED:
		printf("STATUS: WARNING_NOT_INITIALIZED\n");
		break;
	case WPS_STATUS_ERROR:
		printf("STATUS: ERROR\n");
		break;
	case WPS_STATUS_IDLE:
		printf("STATUS: IDLE\n");
		break;
	default:
		printf("STATUS: Unkown\n");
	}
}

static wps_devinf *
wps_my_devinf(wps_devinf *my_devinf)
{
	if (my_devinf == NULL)
		return NULL;

	/* Clear it */
	memset(my_devinf, 0, sizeof(wps_devinf));

	my_devinf->primDeviceCategory = DEV_CAT_COMPUTER;
	my_devinf->primDeviceSubCategory = DEV_SUB_CAT_COMP_PC;
	strncpy(my_devinf->deviceName, "My Broadcom Registrar",
		sizeof(my_devinf->deviceName));
	strncpy(my_devinf->manufacturer, "My Broadcom",
		sizeof(my_devinf->manufacturer));
	strncpy(my_devinf->modelName, "My WPS Wireless Registrar",
		sizeof(my_devinf->modelName));
	strncpy(my_devinf->modelNumber, "0123456789",
		sizeof(my_devinf->modelNumber));
	strncpy(my_devinf->serialNumber, "9876543210",
		sizeof(my_devinf->serialNumber));

	return my_devinf;
}

static void
hup_hdlr(int sig)
{
	g_result = PROCESS_RESULT_CANCELED;
	wps_api_abort();
}

static bool
start_wps(bool async)
{
	char pin[12] = "";
	bool bSucceeded = true;
	bool bFoundAP = false;
	int nAP = 0;
	bool b_v2 = true;
	bool bRet;
	bool bAPPin = false;
	wps_credentials credentials, *new_cred = NULL;
	wps_devinf my_devinf;
	char *my_adapter = NULL;


	printf( "*****************************************************\n" );
	printf( "WPS_API.DLL Test Application. (C) 2007 Broadcom Corp.\n" );
	printf( "*****************************************************\n" );

	/*
	 * What we do:
	 * 1. UI select mode (Enrollee with PIN or PBC) (Registrar with AP's PIN)
	 * 2. Open WPS
	 * 3. UI get an WPS AP
	 * 4. Association
	 * 5. Run WPS
	*/

	/* 1. UI select mode (Enrollee with PIN or PBC) (Registrar with AP's PIN) */
	new_cred = ui_wps_select_mode(&credentials, pin, sizeof(pin), &bAPPin);

	/* establish a handler to handle SIGTERM. */
	//signal(SIGINT, hup_hdlr);

	/* 2. Open WPS */
	bRet = wps_api_open(my_adapter, NULL, _wps_join_callback, wps_my_devinf(&my_devinf), bAPPin, b_v2);
	if (bRet == false) {
		printf("\nOpen WPS failed!\n");
		bSucceeded = false;
		goto err;
	}

	/* 3. UI get an WPS AP */
	memset(&g_apinf, 0, sizeof(g_apinf));
	bFoundAP = ui_wps_find_ap(DEVICE_PASSWORD_ID(bAPPin, pin, new_cred), &g_apinf, &nAP, b_v2);
	if (bFoundAP == false) {
		if (nAP == 0)
			printf("\nNo WPS capable AP found!\n");
		else if (nAP > 0)
			printf("\nMultiple WPS PBC capable AP found with their button pressed!\n"
				"Please try again in about 5mns.\n");
		bSucceeded = false;
		goto err;
	}

	/* 4. Association */
#ifdef TARGETENV_android
	EnableSupplicantEvents(false);
#endif
	printf("\nConnecting to WPS AP %s\n", g_apinf.ssid);
	if (wps_api_join(g_apinf.bssid, g_apinf.ssid, g_apinf.wep) == false) {
		/* Connecting Failed */
		printf("\nConnecting %s failed\n", g_apinf.ssid);
		bSucceeded = false;
		goto err;
	}
	/* Inform link up */
	wps_api_set_linkup();

	/* Connected AP */
	printf("Connected to AP %s\n", g_apinf.ssid);
	printf("Getting credential of AP - %s.\n", g_apinf.ssid);

	/* 5. Run WPS */
	if (wps_api_run(DEVICE_PASSWORD_ID(bAPPin, pin, new_cred),
		g_apinf.bssid, g_apinf.ssid, g_apinf.wep,
		strlen(pin) ? pin : NULL, new_cred, async) == false) {
		printf("\nRun WPS failed!\n");
		bSucceeded = false;
		goto err;
	}

err:
	return bSucceeded;
}

static void
stop_wps()
{
	wps_api_close();

#ifdef TARGETENV_android
	EnableSupplicantEvents(true);
#endif
}

/* ############# */
/* TESTER MAIN STUFF  */
/* ############# */
static int
get_event(EVENT_T *event)
{
	uint32 retVal;
	
	/* Now we only have WPS event */
	event->buf_len = sizeof(event->buf);
	retVal = wps_api_poll_eapol_packet(event->buf, &event->buf_len);
	if (retVal == WPS_STATUS_SUCCESS) {
		event->type = EVENT_TYPE_WPS;
		return GET_EVENT_SUCCESS;
	}
	else if (retVal == WPS_STATUS_ERROR)
		return GET_EVENT_ERROR;
	else
		return GET_EVENT_IDLE;
}

static int
process_success()
{
	char keystr[65] = {0};
	wps_credentials credentials;

	if (wps_api_get_credentials(&credentials) == NULL) {
		printf("\nERROR: Unable to retrieve credential\n");
		return PROCESS_RESULT_ERROR;
	}

	printf("\nWPS AP Credentials:\n");
	printf("SSID = %s\n", credentials.ssid);
	printf("Key Mgmt type is %s\n", credentials.keyMgmt);
	strncpy(keystr, credentials.nwKey, strlen(credentials.nwKey));
	printf("Key : %s\n", keystr);
	printf("Encryption : ");
	if (credentials.encrType == WPS_ENCRYPT_NONE)
		printf("NONE\n");
	if (credentials.encrType & WPS_ENCRYPT_WEP)
		printf(" WEP");
	if (credentials.encrType & WPS_ENCRYPT_TKIP)
		printf(" TKIP");
	if (credentials.encrType & WPS_ENCRYPT_AES)
		printf(" AES");

	printf("\n\n\nCreating profile\n");
	if (wps_api_create_profile(&credentials) == false) {
		printf("\nERROR: Unable to create a profile\n");
	}
	else {
		printf("\nSUCCESS: Created profile\n");
	}

	return PROCESS_RESULT_SUCCESS;
}

static int
process_event()
{
	int retVal;
	EVENT_T event;

	retVal = get_event(&event);
	if (retVal != GET_EVENT_SUCCESS) {
		if (retVal == GET_EVENT_ERROR)
			return PROCESS_RESULT_ERROR;

		return PROCESS_RESULT_IDLE;
	}

	/* Now we only process WPS event */
	retVal = wps_api_process_data(event.buf, event.buf_len);
	if (retVal == WPS_STATUS_SUCCESS)
		return process_success();
	else if (retVal == WPS_STATUS_REJOIN) {
		if (wps_api_join(g_apinf.bssid, g_apinf.ssid, g_apinf.wep) == false) {
			/* Connecting Failed */
			printf("\nConnecting %s failed\n", g_apinf.ssid);
			return PROCESS_RESULT_ERROR;
		}
		/* Tell wps_api link up */
		wps_api_set_linkup();

		return PROCESS_RESULT_REJOIN;
	}
	else if (retVal == WPS_STATUS_ERROR)
		return PROCESS_RESULT_ERROR;

	return PROCESS_RESULT_IDLE;
}

static int
process_timeout()
{
	int retVal;

	/* Now we only process WPS timeout */
	retVal = wps_api_process_timeout();
	if (retVal == WPS_STATUS_REJOIN) {
		if (wps_api_join(g_apinf.bssid, g_apinf.ssid, g_apinf.wep) == false) {
			/* Connecting Failed */
			printf("\nConnecting %s failed\n", g_apinf.ssid);
			return PROCESS_RESULT_ERROR;
		}
		/* Tell wps_api link up */
		wps_api_set_linkup();

		return PROCESS_RESULT_REJOIN;
	}
	else if (retVal == WPS_STATUS_ERROR)
		return PROCESS_RESULT_ERROR;
	else if (retVal == WPS_STATUS_SUCCESS) {
		printf("\nEAP-Failure not received in 10 seconds, assume WPS Success!\n");
		return process_success();
	}

	return PROCESS_RESULT_IDLE;
}

static void
async_process()
{
	/* Async mode wait for thread exit */
	while (g_uiStatus != WPS_STATUS_SUCCESS &&
		g_uiStatus != WPS_STATUS_CANCELED &&
		g_uiStatus != WPS_STATUS_ERROR) {
		if(_kbhit()) {
			if(_getch()=='x' || _getch()=='X') {
				printf("\nCANCEL REQUESTED BY USER. CANCELING, PLEASE WAIT...\n");
				hup_hdlr(0);
			}
		}
	}
	printf("\n");

	if (g_uiStatus == WPS_STATUS_SUCCESS) {
		char keystr[65] = {0};
		wps_credentials credentials;

		if (wps_api_get_credentials(&credentials) == NULL) {
			printf("\nERROR: Unable to retrieve credential\n");
			return;
		}

		printf("\nWPS AP Credentials:\n");
		printf("SSID = %s\n", credentials.ssid);
		printf("Key Mgmt type is %s\n", credentials.keyMgmt);
		strncpy(keystr, credentials.nwKey, strlen(credentials.nwKey));
		printf("Key : %s\n", keystr);
		printf("Encryption : ");
		if (credentials.encrType == WPS_ENCRYPT_NONE)
			printf("NONE\n");
		if (credentials.encrType & WPS_ENCRYPT_WEP)
			printf(" WEP");
		if (credentials.encrType & WPS_ENCRYPT_TKIP)
			printf(" TKIP");
		if (credentials.encrType & WPS_ENCRYPT_AES)
			printf(" AES");

		printf("\n\n\nCreating profile\n");
		if (wps_api_create_profile(&credentials) == false) {
			printf("\nERROR: Unable to create a profile\n");
		}
		else {
			printf("\nSUCCESS: Created profile\n");
		}
	}
	else {
		switch (g_uiStatus) {
		case WPS_STATUS_CANCELED:
			printf("WPS protocol CANCELED by user\n");
			break;
		case WPS_STATUS_ERROR:
			printf("WPS protocol error\n");
			break;
		default:
			printf("WPS protocol error unknown\n");
			break;
		}
	}
}

static void
process()
{
	/* Not Async mode */
	while (1) {
		/* Event process */
		switch (process_event()) {
		case PROCESS_RESULT_SUCCESS:
		case PROCESS_RESULT_CANCELED:
		case PROCESS_RESULT_ERROR:
			return;

		default:
			break;
		}

		/* Timeout process */
		switch (process_timeout()) {
		case PROCESS_RESULT_SUCCESS:
		case PROCESS_RESULT_ERROR:
		case PROCESS_RESULT_CANCELED:
			return;

		default:
			break;
		}

		/* User canceled */
		if (g_result == PROCESS_RESULT_CANCELED)
			break;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
		bool async = false;
	
		/* Async mode */
		if (argc == 2 && _tcscmp(argv[1], _T("-a")) == 0)
			async = true;
	
		if (start_wps(async) == false)
			goto done;
	
		if (async)
			async_process();
		else
			process();
	
done:
		stop_wps();
	
		printf("\nPress any key to exit\n");
		_getch();
	
		return 0;
}

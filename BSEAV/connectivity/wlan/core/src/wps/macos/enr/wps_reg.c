/*
 * Broadcom WPS Registrar
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#include <stdio.h>
#include <ctype.h>

#include <unistd.h>
#include <wpserror.h>

#include <bcmcrypto/bn.h>
#include <bcmcrypto/dh.h>

#include <wpsheaders.h>
#include <wpscommon.h>
#include <sminfo.h>
#include <portability.h>
#include <wps_enrapi.h>
#include <wps_sta.h>
#include <reg_proto.h>
#include <info.h>
#include <statemachine.h>
#include <wpsapi.h>
#include <wps_staeapsm.h>
#include <wps_enr_osl.h>
#include <wps_version.h>
#include <wlioctl.h>

#if !defined(MOD_VERSION_STR)
#error "wps_version.h doesn't exist !"
#endif

extern char *ether_ntoa(const struct ether_addr *addr);

#define WPS_VERSION_STRING
#define WPS_EAP_DATA_MAX_LENGTH         2048
#define WPS_EAP_READ_DATA_TIMEOUT         3

static char ap_pin[10] = "12345670\0\0";

#ifdef _TUDEBUGTRACE
void print_buf(unsigned char *buff, int buflen);
#endif

enum {
	STA_ENR_JOIN_NW = 0,
	STA_REG_JOIN_NW,
	STA_REG_CONFIG_NW
};

static int
display_aplist(wps_ap_list_info_t *ap)
{
	int i = 0;

	if (!ap)
		return 0;

	printf("-------------------------------------\n");
	while (ap->used == TRUE) {
		printf(" %d :  ", i);
		printf("SSID:%s  ", ap->ssid);
		printf("BSSID:%s  ", ether_ntoa((struct ether_addr *)ap->BSSID));
		printf("Channel:%d  ", ap->channel);
		if (ap->wep)
			printf("WEP");
		if (ap->scstate == WPS_SCSTATE_CONFIGURED)
			printf("%sConfigured", (ap->wep) ? " " : "");
		printf("\n");
		ap++;
		i++;
	}

	printf("-------------------------------------\n");
	return 0;
}

static bool
get_ap_configured(char * bssid, char *ssid)
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

/*
 * Fill up the device info and pass it to WPS.
 * This will need to be tailored to specific platforms (read from a file,
 * nvram ...)
 */
static void
reg_config_init(WpsEnrCred *credential)
{
	DevInfo info;
	unsigned char mac[6];
	char uuid[16] = {0x22, 0x21, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0xa, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
	char nwKey[SIZE_64_BYTES+1], *Key = NULL;

	/* fill in device default info */
	memset((char *)(&info), 0, sizeof(info));
	info.version = 0x10;

	/* MAC addr */
	wps_osl_get_mac(mac);
	memcpy(info.macAddr, mac, 6);

	/* generate UUID base on the MAC addr */
	memcpy(info.uuid, uuid, 16);
	memcpy(info.uuid + 10, mac, 6);

	strcpy(info.deviceName, "Broadcom Registrar");
	info.primDeviceCategory = 1;
	info.primDeviceOui = 0x0050F204;
	info.primDeviceSubCategory = 1;
	strcpy(info.manufacturer, "Broadcom");
	strcpy(info.modelName, "WPS Wireless Registrar");
	strcpy(info.modelNumber, "1234");
	strcpy(info.serialNumber, "5678");
	info.configMethods = 0x008C;
	info.authTypeFlags = 0x003f;
	info.encrTypeFlags = 0x000f;
	info.connTypeFlags = 0x01;
	/* rfBand will update again later */
	info.rfBand = WPS_RFBAND_24GHZ;
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
		/* nwKey */
		wps_strncpy(nwKey, credential->nwKey, sizeof(nwKey));
		Key = nwKey;
	}

	wpssta_reg_init(&info, Key, 0);
}

/* Main loop. */
static int
registration_loop(unsigned long start_time)
{
	uint32 retVal;
	char buf[WPS_EAP_DATA_MAX_LENGTH];
	uint32 len;
	char *sendBuf;
	unsigned long now;
	int last_recv_msg, last_sent_msg;
	int state;
	char msg_type;

	now = get_current_time();

	/*
	 * start the process by sending the eapol start . Created from the
	 * Enrollee/Registrar SM Initialize.
	 */
	len = wps_get_msg_to_send(&sendBuf, (uint32)now);

#ifdef _TUDEBUGTRACE
	print_buf((unsigned char*)sendBuf, len);
#endif

	if (sendBuf) {
		send_eapol_packet(sendBuf, len);
		printf("Send EAPOL-Start\n");
	}
	else {
		/* this means the system is not initialized */
		return WPS_ERR_NOT_INITIALIZED;
	}

	/* loop till we are done or failed */
	while (1) {
		len = WPS_EAP_DATA_MAX_LENGTH;

		now = get_current_time();

		if (now > start_time + 120) {
			printf("Overall protocol timeout \n");
			return REG_FAILURE;
		}

		if ((retVal = wait_for_eapol_packet(buf, &len, WPS_EAP_READ_DATA_TIMEOUT))
			== WPS_SUCCESS) {

			/* Show receive message */
			msg_type = wps_get_ap_msg_type(buf, len);
				printf("Receive EAP-Request%s\n",
					wps_get_msg_string((int)msg_type));

			/* process ap message */
			retVal = wps_process_ap_msg(buf, len);

			/* check return code to do more things */
			if (retVal == WPS_SEND_MSG_CONT ||
				retVal == WPS_SEND_MSG_SUCCESS ||
				retVal == WPS_SEND_MSG_ERROR) {
				len = wps_get_msg_to_send(&sendBuf, now);
				if (sendBuf) {
					msg_type = sendBuf[WPS_MSGTYPE_OFFSET];

					wps_eap_send_msg(sendBuf, len);
					printf("Send EAP-Response%s\n",
						wps_get_msg_string((int)msg_type));
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
				/* we received an eap failure from registrar */
				/*
				 * check if this is coming AFTER the protocol passed the M2
				 * mark or is the end of the discovery after M2D.
				 */
				last_recv_msg = wps_get_recv_msg_id();
				printf("Received eap failure, last recv msg EAP-Request%s\n",
					wps_get_msg_string(last_recv_msg));
				if (last_recv_msg > WPS_ID_MESSAGE_M2D)
					return REG_FAILURE;
				else
					return WPS_CONT;
			}
			/* special case, without doing wps_eap_create_pkt */
			else if (retVal == WPS_SEND_MSG_IDRESP) {
				len = wps_get_msg_to_send(&sendBuf, now);
				if (sendBuf) {
					send_eapol_packet(sendBuf, len);
					printf("Send EAP-Response / Identity\n");
				}
			}

			/* SUCCESS or FAILURE */
			if (retVal == WPS_SUCCESS || retVal == REG_FAILURE) {
				return retVal;
			}
		}
		/* timeout with no data, should we re-transmit ? */
		else if (retVal == EAP_TIMEOUT) {
			/* check eap receive timer. It might be time to re-transmit */
			/*
			 * Do we need this API ? We could just count how many
			 * times we re-transmit right here.
			 */
			if ((retVal = wps_eap_check_timer(now)) == WPS_SEND_MSG_CONT) {
				len = wps_get_retrans_msg_to_send(&sendBuf, now, &msg_type);
				if (sendBuf) {
					state = wps_get_eap_state();

					if (state == EAPOL_START_SENT)
						printf("Re-Send EAPOL-Start\n");
					else if (state == EAP_IDENTITY_SENT)
						printf("Re-Send EAP-Response / Identity\n");
					else
						printf("Re-Send EAP-Response%s\n",
							wps_get_msg_string((int)msg_type));

					send_eapol_packet(sendBuf, len);
				}
			}
			/* re-transmission count exceeded, give up */
			else if (retVal == EAP_TIMEOUT) {
				last_recv_msg = wps_get_recv_msg_id();

				if (last_recv_msg == WPS_ID_MESSAGE_M2D) {
					printf("M2D Wait timeout, again.\n");
					return WPS_CONT;
				}
				else if (last_recv_msg > WPS_ID_MESSAGE_M2D) {
					last_sent_msg = wps_get_sent_msg_id();
					printf("Timeout, give up. Last recv/sent msg "
						"[EAP-Response%s/EAP-Request%s]\n",
						wps_get_msg_string(last_recv_msg),
						wps_get_msg_string(last_sent_msg));
					return REG_FAILURE;
				}
				else {
					printf("Re-transmission count exceeded, again\n");
					return WPS_CONT;
				}
			}
		}
	}

	return WPS_SUCCESS;
}

static void
get_new_credential(WpsEnrCred *credential)
{
	int i;
	char inp[3];
	char ssid[SIZE_SSID_LENGTH+1];
	char nwKey[SIZE_64_BYTES+2];
	bool b_tryAgain = true;

	printf("\n** Input new configuration **\n");

	/* ssid */
	memset(credential->ssid, 0, sizeof(credential->ssid));
	memset(ssid, 0, sizeof(ssid));
	while (b_tryAgain) {
		printf("SSID (Max 32 character): ");
		fgets(ssid, sizeof(ssid), stdin);
		fflush(stdin);
		/* remove new line first */
		for (i = 0; i < sizeof(ssid); i++) {
			if (ssid[i] == '\n')
				ssid[i] = '\0';
		}

		if (strlen(ssid) < 1) {
			printf("\tERROR: Invalid input.\n");
			continue;
		}
		else {
			/* remove new line */
			ssid[strlen(ssid)] = '\0';
			strcpy(credential->ssid, ssid);
			b_tryAgain = false;
			printf("SSID: [%s]\n", credential->ssid);
		}
	}

	/* keyMgmt */
	printf("\nKey Management");
	memset(credential->keyMgmt, 0, SIZE_20_BYTES);
	b_tryAgain = true;
	while (b_tryAgain) {
keyMgmt:
		printf("\n\tOptions:\n");
		printf("\t0. None (OPEN)\n");
		printf("\t1. WPA-PSK\n");
		printf("\t2. WPA2-PSK\n");
		printf("\t3. Both WPA-PSK, WPA2-PSK\n");
		printf("\tEnter selection: ");
		fgets(inp, 3, stdin);
		fflush(stdin);
		if (0 == strlen(inp)-1) {
			/* We got no input */
			printf("\tError: Invalid input.\n");
			continue;
		}

		switch (inp[0]) {
			case '0': /* OPEN */
			{
				/* Prompt a warning message when new credential is open */
				while (b_tryAgain) {
					printf("\nWarning:\n");
					printf("Security is not set for the network. Are you sure"
						" you want to continue? [y/n]:");
					fgets(inp, 3, stdin);
					fflush(stdin);
					if (0 == strlen(inp)-1) {
						/* We got no input */
						continue;
					}

					switch (inp[0]) {
					case 'y':
					case 'Y':
						b_tryAgain = false;
						break;
					case 'n':
					case 'N':
						goto keyMgmt;
						break;
					default:
						break;
					}
				}

				credential->keyMgmt[0] = '\0';
				credential->encrType = 0;
				memset(credential->nwKey, 0, SIZE_64_BYTES);
				credential->nwKeyLen = 0;
				return;
			}

			case '1':
			{
				strcpy(credential->keyMgmt, "WPA-PSK");
				b_tryAgain = false;
				break;
			}

			case '2':
			{
				strcpy(credential->keyMgmt, "WPA2-PSK");
				b_tryAgain = false;
				break;
			}

			case '3':
			{
				strcpy(credential->keyMgmt, "WPA-PSK WPA2-PSK");
				b_tryAgain = false;
				break;
			}

			default:
				printf("\tERROR: Invalid input.\n");
				break;
		}
	}

	/* crypto */
	credential->encrType = 0;
	printf("\nCrypto Type");
	b_tryAgain = true;
	while (b_tryAgain) {
		printf("\n\tOptions:\n");
		printf("\t0. TKIP\n");
		printf("\t1. AES\n");
		printf("\t2. Both TKIP, AES\n");
		printf("\tEnter selection: ");
		fgets(inp, 3, stdin);
		fflush(stdin);
		if (0 == strlen(inp)-1) {
			/* We got no input */
			printf("\tError: Invalid input.\n");
			continue;
		}

		switch (inp[0]) {
			case '0':
			{
				credential->encrType |= ENCRYPT_TKIP;
				b_tryAgain = false;
				break;
			}

			case '1':
			{
				credential->encrType |= ENCRYPT_AES;
				b_tryAgain = false;
				break;
			}

			case '2':
			{
				credential->encrType = (ENCRYPT_TKIP | ENCRYPT_AES);
				b_tryAgain = false;
				break;
			}

			default:
				printf("\tERROR: Invalid input.\n");
				break;
		}
	}

	/* nwKey */
	memset(credential->nwKey, 0, sizeof(credential->nwKey));
	memset(nwKey, 0, sizeof(nwKey));
	b_tryAgain = true;
	while (b_tryAgain) {
		printf("Network Key: ");
		fgets(nwKey, sizeof(nwKey), stdin);
		fflush(stdin);

		/* remove new line first */
		for (i = 0; i < sizeof(nwKey); i++) {
			if (nwKey[i] == '\n')
				nwKey[i] = '\0';
		}

		if (strlen(nwKey) < 1) {
			printf("\tERROR: Invalid input.\n");
			continue;
		}
		else {
			/* remove new line */
			nwKey[strlen(nwKey)] = '\0';
			credential->nwKeyLen = strlen(nwKey);
			strncpy(credential->nwKey, nwKey, credential->nwKeyLen);
			b_tryAgain = false;
			printf("Network Key: [%s]\n", nwKey);
		}
	}
	return;
}

static void
print_credential(WpsEnrCred *credential, char *title)
{
	char keystr[65];

	if (title)
		printf("\n%s\n", title);

	printf("SSID = %s\n", credential->ssid);
	printf("Key Mgmt type is %s\n", credential->keyMgmt);
	strncpy(keystr, credential->nwKey, credential->nwKeyLen);
	keystr[credential->nwKeyLen] = 0;
	printf("Key : %s\n", keystr);
	if (credential->encrType == 0) {
		printf("Encryption : NONE\n");
	}
	else {
		if (credential->encrType & ENCRYPT_WEP) {
			printf("Encryption :  WEP\n");
			printf("WEP Index: %d\n", credential->wepIndex);
		}
		if (credential->encrType & ENCRYPT_TKIP)
			printf("Encryption :  TKIP\n");
		if (credential->encrType & ENCRYPT_AES)
			printf("Encryption :  AES\n");
	}
}

static void
get_random_credential(WpsEnrCred *credential)
{
	/* ssid */
	unsigned short ssid_length, key_length;
	unsigned char random_ssid[33] = {0};
	unsigned char random_key[65] = {0};
	unsigned char mac[6] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
	unsigned char macString[18];
	int i;

	wps_osl_get_mac(mac);
	sprintf((char*)macString, "%02x:%02x:%02x:%02x:%02x:%02x",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	RAND_bytes((unsigned char *)&ssid_length, sizeof(ssid_length));
	ssid_length = ((((long)ssid_length + 56791)*13579)%23) + 1;

	RAND_bytes((unsigned char *)random_ssid, ssid_length);

	for (i = 0; i < ssid_length; i++) {
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
	RAND_bytes((unsigned char *)&key_length, sizeof(key_length));
	key_length = ((((long)key_length + 56791)*13579)%8) + 8;
	i = 0;
	while (i < key_length) {
		RAND_bytes(&random_key[i], 1);
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

static int
interactive_start(char *bssid, char *ssid, uint8 *wsec, char **pin, int *mode, int *cred)
{
	char inp[3], inp2[3];
	bool b_tryAgain = true;
	wps_ap_list_info_t *wpsaplist;
	int start_ok = false;
	int i, valc;
	unsigned long pin_num;

	*mode = STA_REG_CONFIG_NW;

	while (b_tryAgain) {
		printf("\nOptions:\n");
		printf("0. Quit\n");
		printf("1. Registrar Join\n");
		printf("2. Configure AP\n");
		printf("Enter selection: ");
		fgets(inp, 3, stdin);
		fflush(stdin);

		if (0 == strlen(inp)-1) {
			/* We got no input */
			printf("Error: Invalid input.\n");
			continue;
		}

		switch (inp[0]) {
		case '0':
			printf("\nShutting down...\n");
			b_tryAgain = false;
			break;

		case '1': /* Registrar Join */
			*mode = STA_REG_JOIN_NW;
		case '2': /* Configure AP */
	scan_retry:
			wpsaplist = create_aplist();
			if (wpsaplist) {
				wps_get_aplist(wpsaplist, wpsaplist);
				printf("------ WPS Enabled AP list --------\n");
				display_aplist(wpsaplist);
			}

			printf("Choose one AP to start!!\n");
			printf("Enter selection: ('a' for scan again, 'q' for quit)");
			fgets(inp2, 3, stdin);
			fflush(stdin);
			if ('a' == inp2[0]) {
				goto scan_retry;
			}
			else if ('q' == inp2[0]) {
				printf("\nShutting down...\n");
				b_tryAgain = false;
				break;
			}
			else if ('0' <= inp2[0] && '9' >= inp2[0]) {
				valc = inp2[0]-48;
				if (wpsaplist[valc].used == TRUE) {
					/*
					 * Prompt a warning message when overwrite existing network
					 * security settings
					 */
					if (*mode == STA_REG_CONFIG_NW &&
						wpsaplist[valc].scstate == WPS_SCSTATE_CONFIGURED) {
						while (b_tryAgain) {
							printf("\nWarning:\n");
							printf("%s is a configured network."
								" Are you sure you want to "
								"overwrite existing network"
								" settings? [y/n]:",
								wpsaplist[valc].ssid);
							fgets(inp, 3, stdin);
							fflush(stdin);
							if (0 == strlen(inp)-1) {
								/* We got no input */
								continue;
							}

							switch (inp[0]) {
							case 'y':
							case 'Y':
								b_tryAgain = false;
								break;
							case 'n':
							case 'N':
								goto scan_retry;
								break;
							default:
								break;
							}
						}
					}

					for (i = 0; i < 6; i++)
						bssid[i] = wpsaplist[valc].BSSID[i];
					memcpy(ssid, wpsaplist[valc].ssid,
						wpsaplist[valc].ssidLen);
					ssid[wpsaplist[valc].ssidLen] = '\0';
					*wsec = wpsaplist[valc].wep;
					start_ok = true;
					b_tryAgain = false;
				}
				else {
					printf("Type error, incorrect number !\n");
					goto scan_retry;
				}
			}
			else {
				printf("Type error!\n");
				goto scan_retry;
			}

			/*  if pin unset, ask user to input */
			if (!*pin) {
				b_tryAgain = true;
				while (b_tryAgain) {
					printf("Please input AP PIN: ");
					fgets(ap_pin, sizeof(ap_pin), stdin);
					fflush(stdin);

					/* remove new line first */
					for (i = 0; i < sizeof(ap_pin); i++) {
						if (ap_pin[i] == '\n')
							ap_pin[i] = '\0';
					}

					if (strlen(ap_pin) != 8) {
						printf("\tInvalid PIN number.\n");
						continue;
					}
					else {
						/* remove new line */
						ap_pin[8] = '\0';
						/* Validate user entered PIN */
						pin_num = strtoul(ap_pin, NULL, 10);
						if (!wps_validateChecksum(pin_num)) {
							printf("\tInvalid PIN number.\n");
							continue;
						}
						printf("Your input AP PIN: %s\n", ap_pin);
						b_tryAgain = false;
					}
				}
				*pin = ap_pin;
			}

			/*  if new credential not specified, ask user to input */
			if (*cred == 0 && *mode == STA_REG_CONFIG_NW) {
				b_tryAgain = true;
				while (b_tryAgain) {
					printf("\nNew Credential:\n");
					printf("1. Random Generated\n");
					printf("2. Manul Input\n");
					printf("Enter selection: ");
					fgets(inp, 3, stdin);
					fflush(stdin);
					if (0 == strlen(inp)-1) {
						/* We got no input */
						printf("Error: Invalid input.\n");
						continue;
					}

					switch (inp[0]) {
					case '1': /* Random */
						b_tryAgain = false;
						*cred = 1;
						break;
					case '2': /* Manul Input */
						b_tryAgain = false;
						*cred = 2;
						break;

					default:
						printf("ERROR: Invalid input.\n");
						break;
					}
				}
			}
			break;

		default:
			printf("ERROR: Invalid input.\n");
			break;
		}
	}

	return start_ok;
}

static int
start_device(char *pin, char *ssid, uint8 wsec, char *bssid, int mode,
	WpsEnrCred *credential, bool *have_cred)
{
	int res;
	unsigned long start_time;

	start_time = get_current_time();
	*have_cred = false;

	while (1) {

		wpssta_start_registration(pin, get_current_time());

		/* registration loop */
		/*
		 * exits with either success, failure or indication that
		 * the registrar has not started its end of the protocol yet.
		 */
		/*
		 * after registration_loop we have AP credentials what should we do next ?
		 * 1. display it and set to wireless interface, if we do join to AP network.
		 * 2. display it and request user provide new credentials and start new registration
		 */
		if ((res = registration_loop(start_time)) == WPS_SUCCESS) {

			printf("WPS Protocol SUCCEEDED !!\n");

			/* get credentials */
			if (mode == STA_REG_JOIN_NW)
				wpssta_get_reg_M7credentials(credential);
			else if (mode == STA_REG_CONFIG_NW)
				wpssta_get_reg_M8credentials(credential);
			else {
				printf("not support mode %d\n", mode);
				break;
			}
			*have_cred = true;
			break;
		}
		else if (res == WPS_CONT) {
			/* Do registration again */
			sleep(1);

			/* Do join again */
			join_network_with_bssid(ssid, wsec, bssid);
		}
		else {
			printf("WPS Protocol FAILED \n");
			break;
		}
	}

	wps_cleanup();

	return 0;
}

int
set_mac_address(char *mac_string, char *mac_bin)
{
	int i = 0;
	char *endptr, *nptr;
	long val;

	nptr = mac_string;

	do {
		val = strtol(nptr, &endptr, 16);
		if (val > 255) {
			printf("invalid MAC address\n");
			return -1;
		}

		if (endptr == nptr) {
			/* no more digits. */
			if (i != 6) {
				printf("invalid MAC address\n");
				return -1;
			}
			return 0;
		}

		if (i >= 6) {
			printf("invalid MAC address\n");
			return -1;
		}

		mac_bin[i++] = val;
		nptr = endptr+1;
	} while (nptr[0]);

	if (i != 6) {
		printf("invalid MAC address\n");
		return -1;
	}

	return 0;
}

static int
print_usage()
{
	printf("Usage : \n\n");
	printf("    Interactive mode : \n");
	printf("       wpsreg <-if eth_name> \n\n");
	printf("    Command line mode (pin) : \n");
	printf("       wpsreg <-if eth_name> <-sec 0|1> -mode (1:reg-join|2:config) "
		"-cred (1:random|2:user input) -ssid ssid -pin ap_pin\n\n");
	printf("    Scan only :\n");
	printf("       wpsreg -scan\n\n");
	printf("    Default values :\n");
	printf("       eth_name :  eth0\n");
	printf("       sec : 1 \n");
	printf("       mode : 1 reg-join \n");
	printf("       cred : 1 random \n");
	return 0;
}

#ifdef _TUDEBUGTRACE
void
print_buf(unsigned char *buff, int buflen)
{
	int i;
	printf("\n print buf : \n");
	for (i = 0; i < buflen; i++) {
		printf("%02X ", buff[i]);
		if (!((i+1)%16))
			printf("\n");
	}
	printf("\n");
}
#endif

/*
 * Name        : main
 * Description : Main entry point for the WPS stack
 * Arguments   : int argc, char *argv[] - command line parameters
 * Return type : int
 */
int
main(int argc, char* argv[])
{
	/* set pin to default */
	char *pin = NULL;
	char start_ok = 0;
	char bssid[6];
	char ssid[SIZE_SSID_LENGTH] = "broadcom\0";
	char if_name[10] = "eth0";
	char user_ssid = false;
	char user_bssid = false;
	char user_pin = false;
	int index;
	char scan = false;
	char *cmd, *val;
	wps_ap_list_info_t *wpsaplist;
	/* by default, assume wep is ON */
	uint8 wsec = 1;
	int mode = STA_REG_JOIN_NW;
	int user_cred = 0;
	bool ap_configured, have_cred;
	WpsEnrCred curr_credential, new_credential;
	unsigned long pin_num;
	uint band_num, active_band;
	bool apply_driver = false;
	char *bssid_ptr = NULL;

	printf("*********************************************\n");
	printf("WPS - Registrar App Broadcom Corp.\n");
	printf("Version: %s\n", MOD_VERSION_STR);
	printf("*********************************************\n");

	/* decount the prog name */
	argc--;
	index = 1;
	while (argc) {
		cmd = argv[index++]; argc--;
		if (!strcmp(cmd, "-help")) {
			print_usage();
			return 0;
		}
		else if (!strcmp(cmd, "-scan")) {
			scan = 1;
		}
		else if (!strcmp(cmd, "-ssid")) {
			val = argv[index++]; argc--;
			strcpy((char *)ssid, val);
			user_ssid = true;
		}
		else if (!strcmp(cmd, "-if")) {
			val = argv[index++]; argc--;
			strcpy(if_name, val);
		}
		else if (!strcmp(cmd, "-bssid")) {
			/* 
			 * WARNING : this "bssid" is used only to create an 802.1X socket.
			 * Normally, it should be the bssid of the AP we will associate to.
			 * Setting this manually means that we might be proceeding to
			 * eapol exchange with a different AP than the one we are associated to,
			 * which might work ... or not.
			 *
			 * When implementing an application, one might want to enforce association
			 * with the AP with that particular BSSID. In case of multiple AP
			 * on the ESS, this might not be stable with roaming enabled.
			 */
			 val = argv[index++]; argc--;
			if (!set_mac_address(val, (char *) bssid)) {
				printf("\n*** WARNING : Setting 802.1X destination manually to:"
					"  %s ***\n\n", val);
				user_bssid = true;
				bssid_ptr = bssid;
			}
		}
		else if (!strcmp(cmd, "-pin")) {
			val = argv[index++]; argc--;
			pin = val;
			user_pin = true;
			/* Validate user entered PIN */
			pin_num = strtoul(pin, NULL, 10);
			if (!wps_validateChecksum(pin_num)) {
				printf("\tInvalid PIN number parameter: %s\n", pin);
				print_usage();
				return 0;
			}
		}
		else if (!strcmp(cmd, "-sec")) {
			val = argv[index++]; argc--;
			wsec = atoi(val);
		}
		else if (!strcmp(cmd, "-mode")) {
			val = argv[index++]; argc--;
			mode = atoi(val);
		}
		else if (!strcmp(cmd, "-cred")) {
			val = argv[index++]; argc--;
			user_cred = atoi(val);
		}
		else {
			printf("Invalid parameter : %s\n", cmd);
			print_usage();
			return 0;
		}
	}

	/* we need to specify the if name before anything else */
	wps_osl_set_ifname(if_name);

	/* if scan requested : display and exit */
	if (scan) {
		wpsaplist = create_aplist();
		if (wpsaplist) {
			display_aplist(wpsaplist);
			wps_get_aplist(wpsaplist, wpsaplist);
			printf("WPS Enabled AP list :\n");
			display_aplist(wpsaplist);
		}
		return 0;
	}

	/*
	 * if ssid, pin specified, use it
	 * eth_name[eth0], sec[1], mode[1:reg-join]
	 * and cred[1:random] use default
	 */
	if (user_ssid && pin) {
		start_ok = true;
	}
	else {
		start_ok = interactive_start((char *)bssid, (char *)ssid,
			&wsec, &pin, &mode, &user_cred);
		bssid_ptr = bssid;
	}

	if (start_ok && pin) {
		/* get credential from user first */
		if (user_cred == 2) { /* user input */
			memset((char *)(&new_credential), 0, sizeof(new_credential));
			get_new_credential(&new_credential);
		}
		else { /* random */
			memset((char *)(&new_credential), 0, sizeof(new_credential));
			get_random_credential(&new_credential);
		}

		/*
		 * join. If user_bssid is specified, it might not
		 * match the actual associated AP.
		 * An implementation might want to make sure
		 * it associates to the same bssid.
		 * There might be problems with roaming.
		 */
		leave_network();
		if (join_network_with_bssid(ssid, wsec, bssid_ptr)) {
			printf("Can not join [%s] network, Quit...\n", ssid);
			return -1;
		}

		/* get specific RF band */
		wps_get_bands(&band_num, &active_band);
		if (active_band == WLC_BAND_5G)
			active_band = WPS_RFBAND_24GHZ;
		else if (active_band == WLC_BAND_2G)
			active_band = WPS_RFBAND_50GHZ;
		else
			active_band = WPS_RFBAND_24GHZ;

		/* If user_bssid not defined, use our AP's */
		if (!user_bssid) {
			if (wps_get_bssid(bssid)) {
				printf("Can not get [%s] BSSID, Quit....\n", ssid);
				return -1;
			}
			bssid_ptr = bssid;
		}

		/*
		 * we need to know AP Wi-Fi Protected Setup State
		 * from beacon/probe-resp or M1
		 */
		ap_configured = get_ap_configured(bssid, ssid);
		printf("AP [%s] is %s\n", ssid, ap_configured ? "Configured" :
			"Unconfigured");

		/*
		 * just config AP with random credential when AP is in unconfigure mode.
		 */
		if (mode == STA_REG_JOIN_NW && !ap_configured) {
			printf("\n\nDirect Config AP [%s] with new %s settings...\n\n",
				user_cred == 1 ? "manul input" : "random generated", ssid);
			mode = STA_REG_CONFIG_NW;
		}

		/* Join Network */
		if (mode == STA_REG_JOIN_NW) {
			printf("Join Network [%s]\n", ssid);

			/* setup device configuration for WPS */
			reg_config_init(NULL);

			/* update specific RF band */
			wps_update_RFBand((uint8)active_band);

			/* setup raw 802.1X socket with "bssid" destination  */
			if (wps_osl_init(bssid) != WPS_SUCCESS) {
				printf("Initializing 802.1x raw socket failed. \n");
				printf("Check PF PACKET support in kernel. \n");
				wps_cleanup();
				wps_osl_deinit();
				leave_network();
				return -1;
			}

			printf("Start registration for BSSID:%s [Join Mode]\n",
				ether_ntoa((struct ether_addr *)bssid));

			/* start device to run registration protocol */
			memset((char *)(&curr_credential), 0, sizeof(curr_credential));
			start_device(pin, ssid, wsec, bssid_ptr, STA_REG_JOIN_NW,
				&curr_credential, &have_cred);
			if (have_cred) {
				print_credential(&curr_credential, "Current AP Credential");
				apply_driver = true;
			}
		}

		/* Config Network */
		if (mode == STA_REG_CONFIG_NW) {
			printf("Config Network [%s]\n", ssid);

			/* setup device configuration for WPS */
			reg_config_init(&new_credential);

			/* update specific RF band */
			wps_update_RFBand((uint8)active_band);

			/* setup raw 802.1X socket with "bssid" destination  */
			if (wps_osl_init(bssid) != WPS_SUCCESS) {
				printf("Initializing 802.1x raw socket failed. \n");
				printf("Check PF PACKET support in kernel, Quit... \n");
				wps_cleanup();
				wps_osl_deinit();
				leave_network();
				return -1;
			}

			printf("Start registration for BSSID:%s [Config Mode]\n",
				ether_ntoa((struct ether_addr *)bssid));

			/* start device to run registrationi protocol */
			memset((char *)(&curr_credential), 0, sizeof(curr_credential));
			start_device(pin, ssid, wsec, bssid_ptr, STA_REG_CONFIG_NW,
				&curr_credential, &have_cred);
			if (have_cred) {
				print_credential(&curr_credential,
					"Please apply below setting to STA");
				apply_driver = true;
			}
		}

		/* wait a moment for send last packet to AP */
		sleep(2);
		leave_network();
	}

	if (apply_driver == true) {
		/* Apply to driver */
		printf("\nApply security to driver ... ");
		fflush(stdout);
		if (do_wpa_psk(&curr_credential)) {
			printf("Fail !!\n\n");
		}
		else {
			printf("Success !!\n\n");
		}
	}

	wps_osl_deinit();

	return 0;
}

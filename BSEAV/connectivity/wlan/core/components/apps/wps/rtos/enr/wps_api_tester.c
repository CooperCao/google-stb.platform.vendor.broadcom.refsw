/* 
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


#include "typedefs.h"
#include "osl_ext.h"
#include <wps_api.h>
#include <wps_sdk.h>
#include <wps_version.h>
#include <portability.h>

bool gContinue = TRUE;
unsigned int g_uiStatus = 0;
uint8 empty_mac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


bool _wps_join_callback(void *context, unsigned int uiStatus, void *data)
{
	g_uiStatus = uiStatus;

	switch (uiStatus)
	{
	case WPS_STATUS_DISABLING_WIFI_MANAGEMENT:
		printf("STATUS: DISABLING_WIFI_MANAGEMENT\n");
		break;
	case WPS_STATUS_SCANNING:
		printf("STATUS: SCANNING\n");
		break;
	case WPS_STATUS_SCANNING_OVER:
		printf("STATUS: SCANNING OVER\n");
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
	case WPS_STATUS_INIT:
		printf("STATUS: INITIALIZING WPS SYSTEM\n");
		break;
	case WPS_STATUS_SUCCESS:
		printf("STATUS: SUCCESS\n");
		break;
	case WPS_STATUS_CANCELED:
		printf("STATUS: CANCELED\n");
		break;
	case WPS_STATUS_WARNING_TIMEOUT:
		printf("STATUS: ERROR_TIMEOUT\n");
		break;
	case WPS_STATUS_WARNING_WPS_PROTOCOL_FAILED:
		printf("STATUS: ERROR_WPS_PROTOCOL\n");
		break;
	case WPS_STATUS_WARNING_NOT_INITIALIZED:
		printf("STATUS: WARNING_NOT_INITIALIZED\n");
		break;
	case WPS_STATUS_ERROR:
		printf("STATUS: ERROR\n");
		break;
	case WPS_STATUS_CREATING_PROFILE:
		printf("STATUS: WPS_STATUS_CREATING_PROFILE\n");
		break;
	case WPS_STATUS_IDLE:
		printf("STATUS: IDLE\n");
		break;
	default:
		printf("STATUS: Unknown\n");
	}


	return gContinue;
}


int wps_api_test()
{
	char pin[80] = "";
	uint8 bssid[6];
	char ssid[32] = "no AP found\0";
	uint8 wep = 1;
	uint16 band = 0;
	char option[10];
	bool bSucceeded = FALSE;
	wps_credentials credentials;
	bool bFoundAP = FALSE;
	int i = 20;
	int nAP = 0;
	char c;

	uint8 channel;
	uint8 version2;
	uint8 *mac, authorizedMACs[6 * 5];
	bool bWpsVersion2 = TRUE;
	int j;

	gContinue = TRUE;
	
	printf("*********************************************\n");
	printf("WPS - Enrollee App Broadcom Corp.\n");
	printf("Version: %s\n", MOD_VERSION_STR);
	printf("*********************************************\n");

	printf("\nIf you have a pin, enter it now, otherwise press ENTER:");
	fgets(pin, 80, stdin);
	pin[strlen(pin)-1] = '\0';

	if (strlen(pin))
	{
		printf("\nLooking for a WPS PIN AP with pin %s. str len %d \n", pin, strlen(pin));
	}
	else
	{
		printf("\nLooking for a WPS PBC AP.\n");
	}

	gContinue = TRUE;
	g_uiStatus = 0;

	if (wps_open(NULL, _wps_join_callback, NULL, bWpsVersion2))
	{
scan:
		bFoundAP = FALSE;
		i = 20;
		nAP = 0;
		gContinue = TRUE;

		do
		{
			bFoundAP = wps_findAP(&nAP,
			                 strlen(pin) ? STA_ENR_JOIN_NW_PIN:STA_ENR_JOIN_NW_PBC, 2);

			if (bFoundAP)
			{
				if (strlen(pin) && nAP > 0)
				{
					i = 0;
					printf("\n--------------------------------\n");
					while (wps_getAP(i, bssid, (char *) ssid, &wep, &band,
						&channel, &version2, authorizedMACs))
					{
						printf(" %-2d :  ", i+1);
						printf("SSID:%-16s  ", ssid);
						printf("BSSID:%02x:%02x:%02x:%02x:%02x:%02x  ",
							bssid[0], bssid[1], bssid[2],
							bssid[3], bssid[4], bssid[5]);
						printf("Channel:%-3d  ", channel);
						if (wep)
							printf("WEP  ");
						if (bWpsVersion2 && version2 != 0) {
							printf("V2(0x%02X)  ", version2);

							mac = authorizedMACs;
							printf("AuthorizedMACs:");
							for (j = 0; j < 5; j++) {
								if (memcmp(mac, empty_mac, 6) == 0)
									break;

							printf(" %02x:%02x:%02x:%02x:%02x:%02x",
								mac[0], mac[1], mac[2],
								mac[3], mac[4], mac[5]);
								mac += 6;
							}
						}
						printf("\n");
						i++;
					}
					printf("-------------------------------------\n");
					printf("\nPlease enter the AP number"
					     "you wish to connect to.\n"
						 "Or enter 0 to search again or x to quit:");
					c = (char)fgetc(stdin);
					if (c == 'x' || c == 'X')
					{
						bFoundAP = FALSE;
						break;
					}
					if (c == '0' || c-'1' > nAP || c-'1' < 0)
					{
						goto scan;
					}
					if (wps_getAP(c-'1', bssid, ssid, &wep, &band,
						&channel, &version2, authorizedMACs) == FALSE)
					{
						printf("Error, wrong number entered!\n");
						goto scan;
					}
				} else if (!strlen(pin) && nAP > 0) {
					if (nAP > 1) {
						printf("More than one PBC AP found."
						        "Restarting scanning\n");
						bFoundAP = FALSE;
					} else {
						wps_getAP(0, bssid, ssid, &wep, &band,
							&channel, &version2, authorizedMACs);
					}
				}
			} else {
				printf("Did not find a WPS AP"
				       ".\nPress X to quit, <Enter> to continue\n");
				fgets(option, 10, stdin);
				/* Remove the \n character too. */
				option[strlen(option)-1] = '\0';

				if (option[0] == 'x' || option[0] == 'X')
				{
					printf("\nCANCEL REQUESTED BY USER"
					        ". CANCELING, PLEASE WAIT...\n");
					gContinue = FALSE;
					bFoundAP = FALSE;
					goto done;
				}
			}
			i--;
		} while (bFoundAP == FALSE && i && gContinue);
		printf("\n");

		if (bFoundAP)
		{
			printf("\nConnecting to WPS AP %s\n", ssid);
			if (wps_join(bssid, ssid, wep))
			{
				printf("Connected to AP %s\n", ssid);
				printf("Getting credential of AP - %s.\n", ssid);
				gContinue = TRUE;
				memset(&credentials, 0, sizeof(wps_credentials));

				if (wps_get_AP_info(strlen(pin) ?
				         STA_ENR_JOIN_NW_PIN:STA_ENR_JOIN_NW_PBC,
				         bssid, ssid,
				         strlen(pin)?pin:NULL,
				         &credentials))
				{
					/* Wait while checking for user cancel action */
					while (g_uiStatus != WPS_STATUS_SUCCESS &&
					        g_uiStatus != WPS_STATUS_CANCELED &&
					        g_uiStatus != WPS_STATUS_ERROR)
					{
						printf(".");
						WpsSleep(1);

					}
					printf("\n");

					if (g_uiStatus == WPS_STATUS_SUCCESS)
					{
						char keystr[65] = { 0 };

						printf("\nWPS AP Credentials:\n");
						printf("SSID = %s\n", credentials.ssid);
						printf("Key Mgmt type is %s\n",
						       credentials.keyMgmt);
						strncpy(keystr, credentials.nwKey,
						          strlen(credentials.nwKey));
						printf("Key : %s\n", keystr);
						printf("Encryption : ");
						if (credentials.encrType == WPS_ENCRYPT_NONE)
							printf("NONE\n");
						if (credentials.encrType & WPS_ENCRYPT_WEP)
							printf(" WEP\n");
						if (credentials.encrType & WPS_ENCRYPT_TKIP)
							printf(" TKIP\n");
						if (credentials.encrType & WPS_ENCRYPT_AES)
							printf(" AES\n");

						bSucceeded = TRUE;
					}
					else
					{
						switch (g_uiStatus)
						{
						case WPS_STATUS_CANCELED:
							printf("WPS protocol CANCELED by user\n");
							break;
						case WPS_STATUS_ERROR:
							printf("WPS protocol error\n");
							break;
						default:
							printf("WPS protocol error unknown\n");
						}
					}
				}
				else
				{
					printf("ERROR: WPS protocol failed\n");
				}
			}
		}
		else
		{
			if (nAP == 0)
				printf("\nNo WPS capable AP found!\n");
			else
				printf("\nMultiple WPS PBC capable AP found"
				        "with their button pressed!\n"
				        "Please try again in about 5mns.\n");
		}
	}

done:

	if (bSucceeded)
	{
		wps_create_profile(&credentials);
	}
	wps_close();

	return 0;
}

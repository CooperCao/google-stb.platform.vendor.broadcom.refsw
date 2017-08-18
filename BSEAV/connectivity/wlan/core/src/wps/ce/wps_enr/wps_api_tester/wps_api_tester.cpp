/* 
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: wps_api_tester.cpp,v 1.7 2008-12-05 06:22:22 $
 */

// wps_api_test.cpp : Defines the entry point for the console application.
//

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <string.h>
#include "wps_sdk.h"


bool gContinue=true;
unsigned int g_uiStatus=0;

/* WPS SDK functions */
#define WPS_SDK_DLL L"wps_api.dll"

wps_open_fptr wps_open;
wps_close_fptr wps_close;
wps_configure_wzcsvc_fptr wps_configure_wzcsvc;
wps_findAP_fptr wps_findAP;
wps_getAP_fptr	wps_getAP;
wps_join_fptr	wps_join;
wps_get_AP_info_fptr wps_get_AP_info;
wps_get_AP_infoEx_fptr wps_get_AP_infoEx;
wps_create_profile_fptr wps_create_profile;
HMODULE hWpsApiDll;

bool loadWpsDll(void)
{
	hWpsApiDll = LoadLibraryW(WPS_SDK_DLL);
	
	if (NULL == hWpsApiDll) {
		printf("Cannot find or load the wps_api.dll");
		return FALSE;
	}
	
	wps_open = (wps_open_fptr) GetProcAddress(hWpsApiDll, _T("wps_open"));
	wps_close = (wps_close_fptr) GetProcAddress(hWpsApiDll, _T("wps_close"));
	wps_configure_wzcsvc = (wps_configure_wzcsvc_fptr) GetProcAddress(hWpsApiDll, _T("wps_configure_wzcsvc"));
	wps_findAP = (wps_findAP_fptr) GetProcAddress(hWpsApiDll, _T("wps_findAP"));
	wps_getAP = (wps_getAP_fptr) GetProcAddress(hWpsApiDll, _T("wps_getAP"));
	wps_join = (wps_join_fptr) GetProcAddress(hWpsApiDll, _T("wps_join"));
	wps_get_AP_info = (wps_get_AP_info_fptr) GetProcAddress(hWpsApiDll, _T("wps_get_AP_info"));
	wps_get_AP_infoEx  = (wps_get_AP_infoEx_fptr) GetProcAddress(hWpsApiDll, _T("wps_get_AP_infoEx"));
	wps_create_profile  = (wps_create_profile_fptr) GetProcAddress(hWpsApiDll, _T("wps_create_profile"));
	
	if (wps_open == NULL || wps_close == NULL || wps_findAP == NULL || 
		wps_join == NULL ||  wps_getAP == NULL || wps_get_AP_info == NULL || 
		wps_get_AP_infoEx == NULL || wps_create_profile == NULL) {
			printf("Failed to find a function.");
		return FALSE;
	}

	return TRUE;
}

bool _wps_join_callback(void *context,unsigned int uiStatus, void *data)
{
	g_uiStatus=uiStatus;

	switch(uiStatus)
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


int main(int argc, _TCHAR* argv[])
{
	char pin[80] = "";
	char start_ok = 0;
	uint8 bssid[6];
	char ssid[32] = "no AP found\0";
	uint8 wep = 1;
	uint16 band = 0;
	char option[10];
	int retries = 3;
	bool bSucceeded = false;
	wps_credentials credentials;

	if (!loadWpsDll()) {
		printf("Failed to load the wps_api.dll. Exiting the application\n");
		return -1;
	}

	printf( "*****************************************************\n" );
	printf( "WPS_API.DLL Test Application. (C) 2008 Broadcom Corp.\n" );
	printf( "*****************************************************\n" );

	printf("\nIf you have a pin, enter it now, otherwise press ENTER:");
	fgets( pin, 80, stdin );
	fflush( stdin );
	pin[strlen(pin)-1] = '\0';

	if(strlen(pin)) 
	{
		printf("\nLooking for a WPS PIN AP with pin %s.\n",pin);
	}
	else
	{
		printf("\nLooking for a WPS PBC AP.\n");
	}

	gContinue=true;
	g_uiStatus=0;

	if (wps_open(NULL,_wps_join_callback))
	{
	 	wps_configure_wzcsvc(0);

scan:
		bool bFoundAP=false;
		int i=20;
		int nAP=0;;
		gContinue=true;

		do
		{
			bFoundAP=wps_findAP(&nAP, strlen(pin)? STA_ENR_JOIN_NW_PIN:STA_ENR_JOIN_NW_PBC, 2);
	
			if(bFoundAP) 
			{
				if(strlen(pin) && nAP>0)
				{
					i=0;
					printf("\n-------------------------------------------------------\n");
					while(wps_getAP(i, bssid, (char *) ssid, &wep, &band))
					{
						printf("[%u] %s\n",i+1,ssid);
						i++;
					}
					printf("-------------------------------------------------------\n");
					printf("\nPlease enter the AP number you wish to connect to.\nOr enter 0 to search again or x to quit:");
					char c=getchar();
					if(c=='x' || c=='X')
					{
						bFoundAP=false;
						break;
					}
					if(c=='0' || c-'1'>nAP || c-'1'<0)
					{
						goto scan;
					}
					if(wps_getAP(c-'1', bssid, ssid, &wep, &band)==false)
					{
						printf("Error, wrong number entered!\n");
						goto scan;
					}
				} else if (!strlen(pin) && nAP > 0) {
					if(nAP > 1) {
						printf("More than one PBC AP found. Restarting scanning\n");
						bFoundAP = false;
					} else {
						wps_getAP(0, bssid, ssid, &wep, &band);
					}
				} 
			} else {
				printf("Did not find a WPS AP.\nPress X to quit, <Enter> to continue\n");
				fgets( option, 10, stdin );
				fflush( stdin );
				option[strlen(option)-1] = '\0';	//Remove the \n character too.
			
				if(option[0] =='x' || option[0] == 'X')
				{
					printf("\nCANCEL REQUESTED BY USER. CANCELING, PLEASE WAIT...\n");
					gContinue=false;
					bFoundAP=false;
					goto done;
				}
			}
			i--;
		} while (bFoundAP==false && i && gContinue);
		printf("\n");

		if(bFoundAP) 
		{
			printf("\nConnecting to WPS AP %s\n",ssid);
			if(wps_join(bssid,ssid,wep))
			{
				printf("Connected to AP %s\n",ssid);
				printf("Getting credential of AP - %s.\n",ssid);
				gContinue=true;
				memset(&credentials, 0, sizeof(wps_credentials));

				if(wps_get_AP_infoEx(strlen(pin)?STA_ENR_JOIN_NW_PIN:STA_ENR_JOIN_NW_PBC,
									bssid, 
									ssid, 
									strlen(pin)?pin:NULL,
									&credentials))
				{
					// Wait for WPS to succeed, fail, or be canceled while checking for user cancel action
					while(g_uiStatus!=WPS_STATUS_SUCCESS && g_uiStatus!=WPS_STATUS_CANCELED && g_uiStatus!=WPS_STATUS_ERROR) 
					{
						printf(".");
						Sleep(1000);
						
					}
					printf("\n");

					if(g_uiStatus==WPS_STATUS_SUCCESS)
					{
						char keystr[65] = { 0 };

						printf("\nWPS AP Credentials:\n");
						printf("SSID = %s\n",credentials.ssid); 
						printf("Key Mgmt type is %s\n", credentials.keyMgmt);
						strncpy(keystr, credentials.nwKey, strlen(credentials.nwKey));
						printf("Key : %s\n", keystr);
						printf("Encryption : ");
						if(credentials.encrType == WPS_ENCRYPT_NONE) 
							printf("NONE\n");
						if(credentials.encrType & WPS_ENCRYPT_WEP)
							printf(" WEP");
						if(credentials.encrType & WPS_ENCRYPT_TKIP)
							printf(" TKIP");
						if(credentials.encrType & WPS_ENCRYPT_AES)
							printf(" AES");

						bSucceeded = true;			
					}
					else
					{
						switch(g_uiStatus)
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
			if(nAP==0)
				printf("\nNo WPS capable AP found!\n");
			else
				printf("\nMultiple WPS PBC capable AP found with their button pressed!\nPlease try again in about 5mns.\n");
		}
	}

done:
 	wps_configure_wzcsvc(1);

	if (bSucceeded)
	{
		printf("\n\n\nCreating profile\n");
		if(!wps_create_profile(&credentials))
		{
			printf("\nERROR: Unable to create a profile\n");
		}
		else
		{
			printf("\nSUCCESS: Created profile\n");
		}	
	}
	wps_close();

	printf("\nPress any key to exit\n");
	getchar();
	return 0;
}

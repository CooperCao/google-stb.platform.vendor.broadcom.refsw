/*
 * Broadcom WPS Enrollee
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 */
#define LOG_TAG "WLWPSService"

#include <utils/Log.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include <cutils/atomic.h>

#include <android_runtime/ActivityManager.h>
#if defined ANDROID_AFTERCUPCAKE
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/MemoryHeapBase.h>
#include <binder/MemoryBase.h>

#else
#include <utils/IPCThreadState.h>
#include <utils/IServiceManager.h>
#include <utils/MemoryHeapBase.h>
#include <utils/MemoryBase.h>
#endif

#include <cutils/properties.h>


#include <WLWPSService.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <typedefs.h>
#include <wps_sdk.h>


#ifndef CONFIG_CTRL_IFACE
#include <wps_version.h>
#include "cutils/memory.h"
#include "cutils/misc.h"
#include "private/android_filesystem_config.h"
#else
#include "includes.h"

#ifdef CONFIG_CTRL_IFACE_UNIX
#include <dirent.h>
#endif /* CONFIG_CTRL_IFACE_UNIX */
#ifdef CONFIG_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif /* CONFIG_READLINE */

#include "wpa_ctrl.h"
#include "common.h"
	//	#include "version.h"
#endif

	/* desktop Linux needs a little help with gettid() */
#if defined(HAVE_GETTID) && !defined(HAVE_ANDROID_OS)
#define __KERNEL__
# include <linux/unistd.h>
#ifdef _syscall0
	_syscall0(pid_t,gettid)
#else
	pid_t gettid() { return syscall(__NR_gettid);}
#endif
#undef __KERNEL__
#endif

#define MAX_SCAN_LIST 20

#define DBGPRINT(x) LOGD x

	namespace android {

		Mutex WLWPSService::mLock;
		sp<IWLWPSClient> WLWPSService::sWLWPSClient;
		unsigned int WLWPSService::s_uiStatus = 0;
		int WLWPSService::s_iCallbackSet = 0;

#ifdef CONFIG_CTRL_IFACE
		typedef struct _wps_credentials
		{
			char        ssid[SIZE_32_BYTES+1];
			char        keyMgmt[SIZE_20_BYTES+1];
			char        nwKey[SIZE_64_BYTES+1];
			uint32	iencrType;
			uint16	wepIndex;
			char        encrType[SIZE_20_BYTES+1];
		} wps_credentials;

		static struct wpa_ctrl *ctrl_conn;
		struct wpa_ctrl *monitor_conn = 0;
		int WLWPSService::m_bThreadLoop = 0;
		char m_SelectedSsid[33];

		const char *WLWPSService::ctrl_iface_dir = "/data/misc/wifi/sockets";

		pthread_cond_t WLWPSService::cond = PTHREAD_COND_INITIALIZER;
		pthread_mutex_t WLWPSService::mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

		wps_credentials m_credentials;


		static char DRIVER_PROP_NAME[25];		/*    = "wlan.driver.status" */
		static char SUPPLICANT_NAME[25];		/*    = "wpa_supplicant" */
		static char SUPP_PROP_NAME[25];			/*    = "init.svc.wpa_supplicant" */
		static char MODULE_FILE[25];			/*    = "/proc/modules" */
		static char IFACE_DIR[256];			/*    = "/data/misc/wifi/sockets" */
		static char DRIVER_MODULE_NAME[10];		/*    = "dhd" */
		static char DRIVER_MODULE_TAG[10];		/*    = "dhd " */
		static char FIRMWARE_LOADER[25];		/*    = "wlan_loader" */
		static char SUPP_CONFIG_TEMPLATE[256];		/*    = "/system/etc/wifi/wpa_supplicant.conf" */
		static char SUPP_CONFIG_FILE[256];		/*    = "/data/misc/wifi/wpa.conf" */
		static char DRIVER_MODULE_PATH[256];		/*    = "/data/local/4325/dhd.ko" */
		static char DRIVER_MODULE_ARG[256];		/*    = "/data/local/4325/dhd.ko" */
		static char INTERFACENAME[25]; 			/*    = "eth0/tiwlan0" */
		const int REFRESH_SCAN_SUCCESS = 200;
		const int REFRESH_SCAN_NO_APS = 201;
		const int REFRESH_SCAN_MULTIPLE_PBC = 202;
		const int REFRESH_SCAN_CONNECTION_ERROR = 203;

		const int ENROLL_WPS_SUCCESS = 100;
		const int ENROLL_WPS_CANCELED = ENROLL_WPS_SUCCESS + 1;
		const int ENROLL_WPS_PROTOCOL_ERROR = ENROLL_WPS_SUCCESS + 2;
		const int ENROLL_WPS_UNKNOWN_ERROR = ENROLL_WPS_SUCCESS + 3;
		const int ENROLL_WPS_PROTOCOL_FAILED = ENROLL_WPS_SUCCESS + 4;
		const int ENROLL_WPS_UNABLE_TO_JOIN = ENROLL_WPS_SUCCESS + 5;
		const int ENROLL_NO_AP_MATCH = ENROLL_WPS_SUCCESS + 6;
		const int ENROLL_WPS_TIMEDOUT = ENROLL_WPS_SUCCESS + 7;
		const int ENROLL_WPS_INPROGRESS = ENROLL_WPS_SUCCESS + 8;

		class CScanListItems
		{
		public:
			uint8 m_bssid[6];
			char m_ssid[33];
			uint8 m_wep;
			uint16 m_band;
			char m_strbssid[20];
		};

		void WLWPSService::instantiate() {
			DBGPRINT(("WLWPSService::instantiate\n"));
			defaultServiceManager()->addService(
				String16("WLWPSService"), new WLWPSService());
		}

#ifndef CONFIG_CTRL_IFACE
		int EnableSupplicantEvents(bool bEnable)
		{
			FILE* ftp = NULL;
			char* filename = (char*)"/data/local/wlapps.lock";
			int iRet = 0;
			if (!bEnable) {
				if (access(filename, F_OK) != 0) {
					if ((ftp = fopen(filename, "w")) != NULL) {
						iRet = 1;
						fclose(ftp);
						DBGPRINT(("Created %s\n", filename));
					}
				} else {
					iRet = 1;
				}
			} else {
				if (access(filename, F_OK) == 0) {
					DBGPRINT(("File found %s\n", filename));
					char cmd[80];
					snprintf(cmd, sizeof(cmd), "/system/bin/rm %s\n", filename);
					iRet = system(cmd);
					if (iRet == -1) {
						iRet = 0;
					} else {
						DBGPRINT(("Deleted %s\n", filename));
						iRet = 1;
					}
				}
			}
			return iRet;
		}

		bool _wps_join_callback(void *context,unsigned int uiStatus, void *data)
		{
			WLWPSService::s_uiStatus=uiStatus;
			char stStatus[1024];

			switch(uiStatus)
			{
			case WPS_STATUS_INIT:
				strcpy(stStatus, "WPS initialized");
				break;
			case WPS_STATUS_DISABLING_WIFI_MANAGEMENT:
				strcpy(stStatus, "Disabling WZC");
				break;
			case WPS_STATUS_SCANNING:
				strcpy(stStatus, "Scanning");
				break;
			case WPS_STATUS_SCANNING_OVER:
				strcpy(stStatus, "Scanning over");
				break;
			case WPS_STATUS_ASSOCIATING:
				{
					sprintf(stStatus, "Associating to %s", (char*)data);
				}
				break;
			case WPS_STATUS_ASSOCIATED:
				{
					sprintf(stStatus, "Associated to %s", (char*)data);
				}
				break;
			case WPS_STATUS_STARTING_WPS_EXCHANGE:
				strcpy(stStatus, "Start WPS Exchange");
				break;
			case WPS_STATUS_SENDING_WPS_MESSAGE:
				strcpy(stStatus, "Sending Message");
				break;
			case WPS_STATUS_WAITING_WPS_RESPONSE:
				strcpy(stStatus, "Waiting for Response");
				break;
			case WPS_STATUS_GOT_WPS_RESPONSE:
				strcpy(stStatus, "Got Response");
				break;
			case WPS_STATUS_DISCONNECTING:
				strcpy(stStatus, "Disconnecting");
				break;
			case WPS_STATUS_ENABLING_WIFI_MANAGEMENT:
				strcpy(stStatus, "Enabling WZC");
				break;
			case WPS_STATUS_SUCCESS:
				strcpy(stStatus, "Success");
				break;
			case WPS_STATUS_CANCELED:
				strcpy(stStatus, "Cancelled");
				break;
			case WPS_STATUS_WARNING_TIMEOUT:
				strcpy(stStatus, "Error: Timeout");
				break;
			case WPS_STATUS_WARNING_WPS_PROTOCOL_FAILED:
				strcpy(stStatus, "Error: WPS Protocol");
				break;
			case WPS_STATUS_WARNING_NOT_INITIALIZED:
				strcpy(stStatus, "Not Intialized");
				break;
			case WPS_STATUS_ERROR:
				strcpy(stStatus, "Error"); 
				break;
			case WPS_STATUS_IDLE:
				strcpy(stStatus, "Idle");
				break;
			case WPS_STATUS_CREATING_PROFILE:
				strcpy(stStatus, "Creating Profile");
				break;
			case WPS_STATUS_OVERALL_PROCESS_TIMOUT:
				strcpy(stStatus, "Process Timeout");
				break;
			case WPS_STATUS_CONFIGURING_ACCESS_POINT:
				strcpy(stStatus, "Configuring AP");
				break;
			default:
				strcpy(stStatus, "Unknown");
			}
			if (WLWPSService::s_iCallbackSet == 1)
			{
				String8 str(stStatus);
				DBGPRINT(("sWLWPSClient->notify"));
				WLWPSService::sWLWPSClient->notify(uiStatus,0,str);
			}
			DBGPRINT(("%s\n", stStatus));		
			return true;
		}
#endif

		WLWPSService::WLWPSService()
		{
			DBGPRINT(("WLWPSService created\n"));
			mWPSInit = false;
			m_pCScanListItems = 0;
			mListCount = 0;
			//	Test();
#ifdef CONFIG_CTRL_IFACE
			ctrl_ifname[0] = '\0';
			ctrl_iface[0] = '\0';
			status_thread_hdl = 0;
			WLWPSService::m_bThreadLoop = 1;
#else
			EnableSupplicantEvents(TRUE);
#endif
		}

		WLWPSService::~WLWPSService()
		{
			DBGPRINT(("WLWPSService destroyed\n"));
			resetScanListItems();
#ifndef CONFIG_CTRL_IFACE
			uninitializeWPS();
			EnableSupplicantEvents(TRUE);
#else
			uninitializeWPA();
			pthread_mutex_destroy(&WLWPSService::mutex);
			m_SelectedSsid[0]='\0';
#endif
		}

		int  WLWPSService::setCallBack(const sp<IWLWPSClient>& client)
		{
			DBGPRINT(("setCallBack\n"));

			Mutex::Autolock _l(mLock);
			sWLWPSClient = client;
			WLWPSService::s_iCallbackSet = 1;
			return 1; 
		}

#ifndef CONFIG_CTRL_IFACE
		bool WLWPSService::initializeWPS()
		{
			if (uninitializeWPS())
			{
				if (wps_open(NULL,_wps_join_callback, NULL))
				{
					mWPSInit = true;   
				}
			}
			return mWPSInit;
		}

		bool WLWPSService::uninitializeWPS()
		{
			DBGPRINT(("uninitializeWPS\n"));
			if (mWPSInit)
			{
				DBGPRINT(("uninitializeWPS 1\n"));
				wps_close();
				mWPSInit = true;
			}
			return true;
		}

		int WLWPSService::refreshList(char* pin)
		{
			char start_ok = 0;
			uint8 wep = 1;
			uint16 band = 0;
			int nAP = 0;
			uint8 bssid[6];
			char  ssid[33];
			int iRet = REFRESH_SCAN_NO_APS;

			resetScanListItems();

			strncpy(m_pin, pin, PIN_LENGTH-1);
			bool bFoundAP = wps_findAP(&nAP, strlen(pin)?STA_ENR_JOIN_NW_PIN:STA_ENR_JOIN_NW_PBC, 10);

			if(bFoundAP) 
			{
				if((strlen(m_pin) > 0) && nAP>0)
				{
					int i=0;

					m_pCScanListItems = new CScanListItems[nAP];
					mListCount = nAP;
					while(wps_getAP(i, bssid, (char *) ssid, &wep, &band))
					{
						strcpy(m_pCScanListItems[i].m_ssid, ssid);
						for(int j=0;j<6;j++)
							m_pCScanListItems[i].m_bssid[j] = bssid[j];
						m_pCScanListItems[i].m_wep = wep;
						m_pCScanListItems[i].m_band = band;
						sprintf(m_pCScanListItems[i].m_strbssid,"%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x", 
							m_pCScanListItems[i].m_bssid[0],
							m_pCScanListItems[i].m_bssid[1],
							m_pCScanListItems[i].m_bssid[2],
							m_pCScanListItems[i].m_bssid[3],
							m_pCScanListItems[i].m_bssid[4],
							m_pCScanListItems[i].m_bssid[5]);
						i++;
					}

					if (i>0)
						iRet = REFRESH_SCAN_SUCCESS;
				}
				else if (!strlen(m_pin) && nAP > 0) 
				{
					if(nAP > 1) 
					{
						iRet = REFRESH_SCAN_MULTIPLE_PBC;
					} 
					else 
					{
						m_pCScanListItems = new CScanListItems[1];
						mListCount = 1;

						if (wps_getAP(0, bssid, ssid, &wep, &band))
						{
							strcpy(m_pCScanListItems[0].m_ssid, ssid);
							for(int j=0;j<6;j++)
								m_pCScanListItems[0].m_bssid[j] = bssid[j];
							m_pCScanListItems[0].m_wep = wep;
							m_pCScanListItems[0].m_band = band;
							iRet = REFRESH_SCAN_SUCCESS;
						}
					}
				} 
			} 

			char strStatus[100];
			switch(iRet)
			{

			case REFRESH_SCAN_NO_APS:
				strcpy(strStatus, "No APs found");
				break;
			case REFRESH_SCAN_MULTIPLE_PBC:
				strcpy(strStatus, "Multiple PBS APs");
				break;
			default:
				strcpy(strStatus, "Scan successful");
				break;
			}
			String8 str(strStatus);
			DBGPRINT(("sWLWPSClient->notify\n"));
			WLWPSService::sWLWPSClient->notify(iRet ,0,str);
			return iRet;
		}

		int  WLWPSService::enrollWPS(char* strbssid)
		{
			bool bContinue = false;
			CScanListItems* pScanItem = 0;
			WLWPSService::s_uiStatus=0;
			int MAX_TRIES = 3*60; /* 3 minutes */
			int iCount = 0;
			int retries = 3;
			uint8 bssid[6];
			char ssid[33] = "no AP found\0";
			uint8 wep = 1;
			char pin[80] = "";
			int iRet = ENROLL_NO_AP_MATCH;
			char stStatus[1024];

			strcpy(pin, m_pin);

			DBGPRINT(("enrollWPS --%s--\n", strbssid));

			for (int i=0;i<mListCount;i++)
			{
				pScanItem = &m_pCScanListItems[i];
				DBGPRINT(("enrollWPS --%s-- --%s-- --%s--\n", bssid, pScanItem->m_strbssid, pin));

				if (strcmp(pScanItem->m_strbssid, strbssid) == 0)
				{
					bContinue = true;
					DBGPRINT(("BSSID found\n"));
					strncpy(ssid, pScanItem->m_ssid, 32);
					memcpy(&bssid[0], &(pScanItem->m_bssid[0]), 6);
					wep = pScanItem->m_wep;
					break;
				}
			}

			if (bContinue)
			{
				EnableSupplicantEvents(FALSE);
				if(wps_join(bssid,ssid,wep))
				{
					DBGPRINT(("wps_join success %s", m_pin));

					bool bOk = false;
					memset(&m_credentials, 0 , sizeof(wps_credentials));
					bOk = wps_get_AP_infoEx(strlen(pin)?STA_ENR_JOIN_NW_PIN:STA_ENR_JOIN_NW_PBC,
						bssid, 
						ssid, 
						strlen(pin)?pin:NULL,
						retries,
						&m_credentials);

					DBGPRINT(("wps_get_AP_infoEx %s\n", m_pin));

					if (bOk)
					{
						// Wait for WPS to succeed, fail, or be canceled while checking for user cancel action
						while(WLWPSService::s_uiStatus!=WPS_STATUS_SUCCESS && 
							WLWPSService::s_uiStatus!=WPS_STATUS_CANCELED && 
							WLWPSService::s_uiStatus!=WPS_STATUS_ERROR) 
						{
							sleep(1);
							DBGPRINT(("."));
							if (WLWPSService::s_iCallbackSet == 1)
							{
								strcpy(stStatus, "WPS in progress");
								String8 str(stStatus);
								DBGPRINT(("sWLWPSClient->notify\n"));
								WLWPSService::sWLWPSClient->notify(ENROLL_WPS_INPROGRESS ,0,str);
							}
						}

						if(WLWPSService::s_uiStatus==WPS_STATUS_SUCCESS)
						{
							DBGPRINT(("WLWPSService::s_uiStatus==WPS_STATUS_SUCCESSs\n"));
							iRet = ENROLL_WPS_SUCCESS;
							char keystr[65] = { 0 };

							DBGPRINT(("\nWPS AP Credentials:\n"));
							DBGPRINT(("SSID = %s\n",m_credentials.ssid)); 
							DBGPRINT(("Key Mgmt type is %s\n", m_credentials.keyMgmt));
							strncpy(keystr, m_credentials.nwKey, strlen(m_credentials.nwKey));
							DBGPRINT(("Key : %s\n", keystr));
							DBGPRINT(("Encryption : "));
							if(m_credentials.encrType == WPS_ENCRYPT_NONE) 
								DBGPRINT(("NONE\n"));
							if(m_credentials.encrType & WPS_ENCRYPT_WEP)
								DBGPRINT((" WEP"));
							if(m_credentials.encrType & WPS_ENCRYPT_TKIP)
								DBGPRINT((" TKIP"));
							if(m_credentials.encrType & WPS_ENCRYPT_AES)
								DBGPRINT((" AES"));
						}
						else
						{
							DBGPRINT(("WLWPSService::s_uiStatus==failure\n"));
							if (iCount>= MAX_TRIES)
							{
								iRet = ENROLL_WPS_TIMEDOUT;
							}
							else
							{
								switch(WLWPSService::s_uiStatus)
								{
								case WPS_STATUS_CANCELED:
									iRet = ENROLL_WPS_CANCELED;
									break;
								case WPS_STATUS_ERROR:
									iRet = ENROLL_WPS_PROTOCOL_ERROR;
									break;
								default:
									iRet = ENROLL_WPS_UNKNOWN_ERROR;
									break;
								}
							}
						}
					}
					else
					{
						iRet = ENROLL_WPS_PROTOCOL_FAILED;
					}
				}
				else
				{
					iRet = ENROLL_WPS_UNABLE_TO_JOIN;
				}
			}
			if (WLWPSService::s_iCallbackSet == 1)
			{
				switch (iRet)
				{
				case ENROLL_WPS_SUCCESS:
					strcpy(stStatus, "WPS Success");
					break;
				case ENROLL_WPS_CANCELED:
					strcpy(stStatus, "WPS Cancelled");
					break;
				case ENROLL_WPS_PROTOCOL_ERROR:
					strcpy(stStatus, "WPS Protocol Error");
					break;
				case ENROLL_WPS_UNKNOWN_ERROR:
					strcpy(stStatus, "WPS Unknown Error");
					break;
				case ENROLL_WPS_PROTOCOL_FAILED:
					strcpy(stStatus, "WPS Protocol Failed");
					break;
				case ENROLL_WPS_UNABLE_TO_JOIN:
					strcpy(stStatus, "Unable to join");
					break;
				case ENROLL_NO_AP_MATCH:
					strcpy(stStatus, "No matching AP");
					break;
				default : //case ENROLL_WPS_TIMEDOUT:
					strcpy(stStatus, "WPS Timed out");
					break;
				}
				String8 str(stStatus);
				DBGPRINT(("sWLWPSClient->notify\n"));
				WLWPSService::sWLWPSClient->notify(iRet ,0,str);
			}
			EnableSupplicantEvents(TRUE);
			return iRet;
		}
#endif

		void WLWPSService::resetScanListItems()
		{
			if (m_pCScanListItems)
			{
				delete [] m_pCScanListItems;
				m_pCScanListItems = 0;
				mListCount = 0;
			}
		}

		int  WLWPSService::wpsOpen()
		{
			Mutex::Autolock _l(mLock);
			DBGPRINT(("WLWPSService wpsOpen\n"));
#ifndef CONFIG_CTRL_IFACE
			return initializeWPS() == true ? 1 : 0;
#else
			return initializeWPA() == true ? 1 : 0;
#endif
		}

		int WLWPSService::wpsClose()
		{
			Mutex::Autolock _l(mLock);
			DBGPRINT(("WLWPSService wpsClose\n"));
#ifndef CONFIG_CTRL_IFACE
			return uninitializeWPS() == true ? 1 : 0;
#else
			return uninitializeWPA() == true ? 1 : 0;
#endif
		}

		int WLWPSService::wpsRefreshScanList(String8 pin)
		{
			Mutex::Autolock _l(mLock);
			unsigned char* results = (unsigned char*)(pin.string());
			int dataLength = pin.length();
			DBGPRINT(("WLWPSService::wpsRefreshScanList %d %s\n", dataLength, pin.string()));
#ifndef CONFIG_CTRL_IFACE
			return refreshList((char*)results);
#else
			return refreshWPAList((char*)results);
#endif
		}

		int WLWPSService::wpsGetScanCount()
		{
			Mutex::Autolock _l(mLock);
			DBGPRINT(("WLWPSService wpsGetScanCount = %d\n", mListCount));
			return mListCount;
		}

		String8 WLWPSService::wpsGetScanSsid(int iIndex)
		{
			Mutex::Autolock _l(mLock);
			DBGPRINT(("WLWPSService::wpsGetScanSsid %d\n", iIndex));

			String8 ssid;
			if (iIndex< mListCount)
			{
				ssid = m_pCScanListItems[iIndex].m_ssid;
				DBGPRINT(("WLWPSService::wpsGetScanSsid  %d\n", iIndex));
			}
			return ssid;
		}

		String8 WLWPSService::wpsGetScanBssid(int iIndex)
		{
			Mutex::Autolock _l(mLock);
			DBGPRINT(("WLWPSService::wpsGetScanBssid %d\n", iIndex));

			String8 bssid;
			if (iIndex< mListCount)
			{
				bssid = m_pCScanListItems[iIndex].m_strbssid;;
				DBGPRINT(("WLWPSService::wpsGetScanBssid  %d\n", iIndex));
			}


			return bssid;
		}		

		int WLWPSService::wpsEnroll(String8 bssid)
		{
			Mutex::Autolock _l(mLock);
			unsigned char* results = (unsigned char*)(bssid.string());
			int dataLength = bssid.length();
			DBGPRINT(("WLWPSService::wpsEnroll* %d %s\n", dataLength, bssid.string()));
#ifndef CONFIG_CTRL_IFACE
			return enrollWPS((char*)results);
#else
			return enrollWPA((char*)results);
#endif
		}

		String8 WLWPSService::wpsGetSsid()
		{
			Mutex::Autolock _l(mLock);
#ifndef CONFIG_CTRL_IFACE
			String8 ssid(m_credentials.ssid);
#else
			String8 ssid(m_credentials.ssid);
#endif
			DBGPRINT(("WLWPSService::wpsGetSsid %s", ssid.string()));
			return ssid;
		}

		String8 WLWPSService::wpsGetKeyMgmt()
		{
			Mutex::Autolock _l(mLock);
#ifndef CONFIG_CTRL_IFACE
			String8 keyMgmt(m_credentials.keyMgmt);
#else
			String8 keyMgmt(m_credentials.keyMgmt);
#endif
			DBGPRINT(("WLWPSService::wpsGetKeyMgmt %s\n", keyMgmt.string()));
			return keyMgmt;
		}

		String8 WLWPSService::wpsGetKey()
		{
			char keystr[65] = { 0 };
			Mutex::Autolock _l(mLock);
#ifndef CONFIG_CTRL_IFACE
			strncpy(keystr, m_credentials.nwKey, strlen(m_credentials.nwKey));
			String8 key(keystr);
#else
			String8 key(m_credentials.nwKey);

#endif
			DBGPRINT(("WLWPSService::wpsGetKey %s\n", key.string()));
			return key;
		}

		String8 WLWPSService::wpsGetEncryption()
		{
			Mutex::Autolock _l(mLock);
			String8 encr;

#ifndef CONFIG_CTRL_IFACE
			DBGPRINT(("WLWPSService wpsGetEncryption\n"));
			switch(m_credentials.encrType)
			{
			case WPS_ENCRYPT_AES:
				encr = "AES";
				break;
			case WPS_ENCRYPT_WEP:
				encr  = "WEP";
				break;
			case WPS_ENCRYPT_TKIP:
				encr = "TKIP";
				break;
			default:
				encr = "NONE";
				break;
			}
#else
			encr = m_credentials.encrType;
#endif
			return encr;
		}

		int WLWPSService::loadconfig(char * configfile)
		{
			FILE *fp;
			char line[512];

			if ((fp = fopen(configfile, "r")) == NULL) {
				DBGPRINT(("Could not open %s: %s", configfile, strerror(errno)));
				return -1;
			}

			while ((fgets(line, sizeof(line), fp)) != NULL) {
				line[strlen(line) - 1] = 0x0;
				if (strncmp(line, "FIRMWARE_LOADER", strlen("FIRMWARE_LOADER")) == 0){
					strcpy(FIRMWARE_LOADER, line + strlen("FIRMWARE_LOADER") + 1); 
				}else if (strncmp(line, "DRIVER_PROP_NAME", strlen("DRIVER_PROP_NAME")) == 0){
					strcpy(DRIVER_PROP_NAME, line + strlen("DRIVER_PROP_NAME") + 1); 
				}else if (strncmp(line, "SUPPLICANT_NAME", strlen("SUPPLICANT_NAME")) == 0){
					strcpy(SUPPLICANT_NAME, line + strlen("SUPPLICANT_NAME") + 1); 
				}else if (strncmp(line, "SUPP_PROP_NAME", strlen("SUPP_PROP_NAME")) == 0){
					strcpy(SUPP_PROP_NAME, line + strlen("SUPP_PROP_NAME") + 1); 
				}else if (strncmp(line, "MODULE_FILE", strlen("MODULE_FILE")) == 0){
					strcpy(MODULE_FILE, line + strlen("MODULE_FILE") + 1); 
				}else if (strncmp(line, "IFACE_DIR", strlen("IFACE_DIR")) == 0){
					strcpy(IFACE_DIR, line + strlen("IFACE_DIR") + 1); 
				}else if (strncmp(line, "DRIVER_MODULE_NAME", strlen("DRIVER_MODULE_NAME")) == 0){
					strcpy(DRIVER_MODULE_NAME, line + strlen("DRIVER_MODULE_NAME") + 1); 
				}else if (strncmp(line, "DRIVER_MODULE_TAG", strlen("DRIVER_MODULE_TAG")) == 0){
					strcpy(DRIVER_MODULE_TAG, line + strlen("DRIVER_MODULE_TAG") + 1); 
				}else if (strncmp(line, "SUPP_CONFIG_TEMPLATE", strlen("SUPP_CONFIG_TEMPLATE")) == 0){
					strcpy(SUPP_CONFIG_TEMPLATE, line + strlen("SUPP_CONFIG_TEMPLATE") + 1);
				}else if (strncmp(line, "SUPP_CONFIG_FILE", strlen("SUPP_CONFIG_FILE")) == 0){
					strcpy(SUPP_CONFIG_FILE, line + strlen("SUPP_CONFIG_FILE") + 1);
				}else if (strncmp(line, "DRIVER_MODULE_PATH", strlen("DRIVER_MODULE_PATH")) == 0){
					strcpy(DRIVER_MODULE_PATH, line + strlen("DRIVER_MODULE_PATH") + 1);
				}else if (strncmp(line, "DRIVER_MODULE_ARG", strlen("DRIVER_MODULE_ARG")) == 0){
					strcpy(DRIVER_MODULE_ARG, line + strlen("DRIVER_MODULE_ARG") + 1);
				}else if (strncmp(line, "INTERFACENAME", strlen("INTERFACENAME")) == 0){
					strcpy(INTERFACENAME, line + strlen("INTERFACENAME") + 1);
				}
			}

			fclose(fp);

			return 0;	
		}

#ifdef CONFIG_CTRL_IFACE
		int WLWPSService::str_match(const char *a, const char *b)
		{
			return strncmp(a, b, strlen(b)) == 0;
		}

		void WLWPSService::wpa_cli_msg_cb(char *msg, size_t len)
		{
			DBGPRINT(("%s\n", msg));
		}

		int WLWPSService::wpa_ctrl_command(const char *cmd, char* buf, size_t* len)
		{
			int ret;

			if (ctrl_conn == NULL) {
				DBGPRINT(("Not connected to wpa_supplicant - command dropped.\n"));
				return -1;
			}
			DBGPRINT(("wpa_ctrl_command(), cmd=%s\n", cmd));
			ret = wpa_ctrl_request(ctrl_conn, cmd, os_strlen(cmd), buf, len,
				wpa_cli_msg_cb);
			if (ret == -2) {
				DBGPRINT(("'%s' command timed out.\n", cmd));
				return -2;
			} else if (ret < 0) {
				DBGPRINT(("'%s' command failed.\n", cmd));
				return -1;
			}
			buf[*len] = '\0';

			return 0;
		}

		void WLWPSService::getCredentials(char* ssid)
		{
			const char* cmdListNetworks = "LIST_NETWORKS";
			char reply[8192];
			size_t reply_len;
			char seps[] = "\n";
			char *str;
			int ifirstLine = 0;
			char listEntry[1024];
			char* token; 
			int iCol = 0;
			char listSSID[33];
			char* saveptr1, *saveptr2;
			char networkid[20];

			char tabseps[] = " \t";
			int bFound = 0;

			memset(&m_credentials, 0 , sizeof(wps_credentials));

			DBGPRINT(("calling getCredentials\n"));
			if (ctrl_conn != NULL)
			{
				reply_len = sizeof(reply) - 1;
				if (wpa_ctrl_command(cmdListNetworks, &reply[0], &reply_len) < 0)
				{
					DBGPRINT(("getCredentials failed\n"));
					return;
				}

				ifirstLine = 0;

				//This is the last network
				str = strtok_r(reply, seps, &saveptr1);
				while (str != NULL && !bFound)
				{
					//ignore first line
					if (ifirstLine != 0)
					{
						DBGPRINT((str));
						DBGPRINT(("\n"));

						strncpy(listEntry, str, 1023);

						DBGPRINT(("List entry: %s\n", listEntry));
						token  = strtok_r(listEntry, tabseps, &saveptr2);
						iCol = 0;
						while(token)
						{
							switch(iCol)
							{
							case 0:  //network ID
								DBGPRINT(("network id = %s\n", token));
								strncpy(networkid, token, 19);
								break;
							case 1:  //ssid
								DBGPRINT(("ssid = %s\n", token));
								strncpy(listSSID, token, 32);
								break;
							case 2:  //bssid
								DBGPRINT(("bssid = %s\n", token));
								break;
							case 3:  //Flags
								DBGPRINT(("Flags = %s\n", token));
								break;
							}
							++iCol;
							token = strtok_r(NULL, tabseps, &saveptr2);         
						} 
						DBGPRINT(("\n"));
					}
					else
					{
						ifirstLine = 1;
					}
					str = strtok_r(NULL, seps, &saveptr1);         
				} 
				DBGPRINT(("Get crdentials %s %s\n", ssid, listSSID));
				if (strcmp(ssid, listSSID) == 0)
				{ 
					char cmdCredentials[100];
					char buf[2048];
					int iParam = 0;
					char sParam[100];
					size_t buf_len;
					DBGPRINT(("Getting crdentials %s %s\n", ssid, listSSID));

					bFound = 1;

					for(iParam = 0;iParam <6 ;iParam++)
					{    
						buf_len = sizeof(buf) - 1;
						switch(iParam)
						{
						case 0: strcpy(sParam, "ssid"); break;
						case 1: strcpy(sParam, "psk"); break;
						case 2: strcpy(sParam, "proto"); break;
						case 3: strcpy(sParam, "key_mgmt"); break;
						case 4: strcpy(sParam, "pairwise"); break;
						case 5: strcpy(sParam, "auth_alg"); break;

						}

						sprintf(cmdCredentials, "GET_NETWORK %s %s", networkid, sParam);
						if (wpa_ctrl_command(cmdCredentials, &buf[0], &buf_len) < 0)
						{
							DBGPRINT(("Failed to get network\n"));
						}
						else
						{
							DBGPRINT(("Output %s\n", buf));
							switch(iParam)
							{
							case 0: strcpy(m_credentials.ssid, buf); break;
							case 1: strcpy(m_credentials.nwKey, buf); break;
							case 3: strcpy(m_credentials.keyMgmt, buf); break;
							case 2: strcpy(m_credentials.encrType, buf); break;
							default: break;
							}
						}
					}
				}
			}
		}

		void WLWPSService::removeNetworks(char* ssid)
		{
			const char* cmdListNetworks = "LIST_NETWORKS";
			char reply[8192];
			size_t reply_len;
			char seps[] = "\n";
			char *str;
			int ifirstLine = 0;
			char listEntry[1024];
			char* token; 
			int iCol = 0;
			char listSSID[33];
			char* saveptr1, *saveptr2;
			char networkid[20];

			char tabseps[] = " \t";

			DBGPRINT(("calling removeNetworks\n"));
			if (ctrl_conn != NULL)
			{
				reply_len = sizeof(reply) - 1;
				if (wpa_ctrl_command(cmdListNetworks, &reply[0], &reply_len) < 0)
				{
					DBGPRINT(("removeNetworks failed\n"));
					return;
				}

				DBGPRINT(("removeNetworks\n%s\n", reply));
				//split the new lines
				str = strtok_r(reply, seps, &saveptr1);
				while (str != NULL)
				{
					//ignore first line
					if (ifirstLine != 0)
					{
						DBGPRINT((str));
						DBGPRINT(("\n"));

						if (strstr(str, "[CURRENT]") == 0)
						{
							strncpy(listEntry, str, 1023);

							DBGPRINT(("List entry: %s\n", listEntry));
							token  = strtok_r(listEntry, tabseps, &saveptr2);
							iCol = 0;
							while(token)
							{
								switch(iCol)
								{
								case 0:  //network ID
									DBGPRINT(("network id = %s\n", token));
									strncpy(networkid, token, 19);
									break;
								case 1:  //ssid
									DBGPRINT(("ssid = %s\n", token));
									strncpy(listSSID, token, 32);
									break;
								case 2:  //bssid
									DBGPRINT(("bssid = %s\n", token));
									break;
								case 3:  //Flags
									DBGPRINT(("Flags = %s\n", token));
									break;
								}
								++iCol;
								token = strtok_r(NULL, tabseps, &saveptr2);         
							} 
							DBGPRINT(("\n"));

							//remove the SSID

							if (strcmp(ssid, listSSID) == 0)
							{ 
								//deleting network
								char cmdRemove[100];
								char buf[10];
								size_t buf_len = sizeof(buf) - 1;
								sprintf(cmdRemove, "REMOVE_NETWORK %s", networkid);
								if (wpa_ctrl_command(cmdRemove, &buf[0], &buf_len) < 0)
								{
									DBGPRINT(("Failed to remove a network\n"));
								}
								else
								{
									DBGPRINT(("Removed a network\n"));
								}
							}
						}
					}
					else
					{
						ifirstLine = 1;
					}
					str = strtok_r(NULL, seps, &saveptr1);         
				} 

			}
		}

		int WLWPSService::updateScanResults(bool isPin)
		{
			const char *cmd = "SCAN_RESULTS";
			char reply[8192];
			size_t reply_len;
			char seps[] = "\n";
			char *str;
			int ifirstLine = 0;
			char scanEntry[1024];
			char* token; 
			int iCol = 0;
			char* saveptr1, *saveptr2;

			char tabseps[] = " \t";
			char compareStr[20];
			int iRet = REFRESH_SCAN_CONNECTION_ERROR;

			//find a way to display only PIN or PBS APs. Now it displays all WPS APs
			/*   if (isPin)
			strcpy(compareStr, "[WPS-PIN]");
			else
			strcpy(compareStr, "[WPS-PBC]");
			*/
			strcpy(compareStr, "[WPS");

			if (ctrl_conn != NULL)
			{
				reply_len = sizeof(reply) - 1;
				if (wpa_ctrl_command(cmd, &reply[0], &reply_len) < 0)
				{
					DBGPRINT(("sWLWPSClient->notify\n"));
					String8 str("Failed to connect to wpa supplicant");
					WLWPSService::sWLWPSClient->notify(iRet ,0,str);
					return iRet;
				}

				DBGPRINT(("updateScanResults\n%s\n", reply));

				mListCount =0;
				m_pCScanListItems = new CScanListItems[MAX_SCAN_LIST];

				//split the new lines
				str = strtok_r(reply, seps, &saveptr1);
				while (str != NULL)
				{
					//ignore first line
					if (ifirstLine != 0)
					{      
						//see whether there are networks with [WPS-PBC] or [WPS-PIN], ignore rest
						if (strstr(str, compareStr))
						{
							strncpy(scanEntry, str, 1023);


							token  = strtok_r(scanEntry, tabseps, &saveptr2);
							iCol = 0;
							while(token)
							{
								switch(iCol)
								{
								case 0:  //BSSID
									DBGPRINT(("BSSID = %s\n", token));
									{
										strcpy(m_pCScanListItems[mListCount].m_strbssid, token);
									}
									break;
								case 1:  //Freq
									DBGPRINT(("Freq = %s\n", token));
									break;
								case 2:  //qual
									DBGPRINT(("Qual = %s\n", token));
									break;
								case 3:  //Flags
									DBGPRINT(("Flags = %s\n", token));
									break;
								case 4:  //SSID
									DBGPRINT(("SSID = %s\n", token));
									if (mListCount < MAX_SCAN_LIST)
									{
										strcpy(m_pCScanListItems[mListCount].m_ssid, token);
									}
									break;


								}
								++iCol;
								token = strtok_r(NULL, tabseps, &saveptr2);         
							} 
							DBGPRINT(("\n"));
							m_pCScanListItems[mListCount].m_wep = 0; //not used
							m_pCScanListItems[mListCount].m_band = 0; //not used
							++mListCount;

						}
					}
					else
					{
						ifirstLine = 1;
					}
					str = strtok_r(NULL, seps, &saveptr1);         
				} 

			}

			if (mListCount>0)
			{
				if (isPin)
					iRet = REFRESH_SCAN_SUCCESS;
				else
				{
					if (mListCount == 1)
						iRet = REFRESH_SCAN_SUCCESS;
					else
						iRet = REFRESH_SCAN_MULTIPLE_PBC;
				}
			}
			else
			{
				iRet = REFRESH_SCAN_NO_APS;
			}

			char strStatus[100];
			switch(iRet)
			{
			case REFRESH_SCAN_NO_APS:
				strcpy(strStatus, "No APs found");
				break;
			case REFRESH_SCAN_MULTIPLE_PBC:
				strcpy(strStatus, "Multiple PBS APs");
				break;
			default:
				strcpy(strStatus, "Scan successful");
				break;
			}
			String8 strNotify(strStatus);
			DBGPRINT(("sWLWPSClient->notify\n"));
			WLWPSService::sWLWPSClient->notify(iRet ,0,strNotify);
			return iRet;
		}

		void WLWPSService::processMsg(char *msg)
		{
			char *pos = msg, *pos2;
			char stStatus[1024];
			unsigned int uiStatus=0;

			int priority = 2;

			if (*pos == '<') {
				/* skip priority */
				pos++;
				priority = atoi(pos);
				pos = strchr(pos, '>');
				if (pos)
					pos++;
				else
					pos = msg;
			}


			/* Update last message with truncated version of the event */
			if (strncmp(pos, "CTRL-", 5) == 0) {
				pos2 = strchr(pos, str_match(pos, WPA_CTRL_REQ) ? ':' : ' ');
				if (pos2)
					pos2++;
				else
					pos2 = pos;
			} else
				pos2 = pos;
			//QString lastmsg = pos2;
			//lastmsg.truncate(40);
			//textLastMessage->setText(lastmsg);

			if (str_match(pos, WPA_CTRL_REQ))
			{
				//processCtrlReq(pos + strlen(WPA_CTRL_REQ));
			}
			else if (str_match(pos, WPA_EVENT_SCAN_RESULTS))
			{
				//updateScanResults();
				strcpy(stStatus, "WPA_EVENT_SCAN_RESULTS");
				//uiStatus = REFRESH_SCAN_SUCCESS;
			}
			else if (str_match(pos, WPA_EVENT_DISCONNECTED))
			{ 
				strcpy(stStatus, "WPA_EVENT_DISCONNECTED : Disconnected from network"); 
			}
			else if (str_match(pos, WPA_EVENT_CONNECTED)) 
			{
				strcpy(stStatus, "Connection to network established.");
				char ssid[33];
				pthread_mutex_lock(&WLWPSService::mutex);
				strcpy(ssid, m_SelectedSsid);
				pthread_mutex_unlock(&WLWPSService::mutex);

				WLWPSService::getCredentials(ssid);
				uiStatus = ENROLL_WPS_SUCCESS;
			} 
			else if (str_match(pos, WPS_EVENT_AP_AVAILABLE_PBC)) 
			{
				strcpy(stStatus, "WPS AP in active PBC mode found.");
			} 
			else if (str_match(pos, WPS_EVENT_AP_AVAILABLE_PIN)) 
			{
				strcpy(stStatus, "Wi-Fi Protected Setup (WPS) AP in active PIN mode found");
			} 
			else if (str_match(pos, WPS_EVENT_AP_AVAILABLE)) 
			{
				strcpy(stStatus, "Wi-Fi Protected Setup (WPS) AP detected");
			} 
			else if (str_match(pos, WPS_EVENT_OVERLAP))
			{
				strcpy(stStatus, "Wi-Fi Protected Setup (WPS) PBC mode overlap detected.");
				uiStatus = REFRESH_SCAN_MULTIPLE_PBC;
			} 
			else if (str_match(pos, WPS_EVENT_CRED_RECEIVED)) 
			{
				strcpy(stStatus, "Network configuration received");
				uiStatus = ENROLL_WPS_INPROGRESS;
			} 
			else if (str_match(pos, WPA_EVENT_EAP_METHOD)) 
			{
				if (strstr(pos, "(WSC)")) {
					strcpy(stStatus, "Registration started");
					uiStatus = ENROLL_WPS_INPROGRESS;
				}

			} 
			else if (str_match(pos, WPS_EVENT_M2D)) 
			{
				strcpy(stStatus, "Registrar does not yet know PIN");
			} 
			else if (str_match(pos, WPS_EVENT_FAIL)) 
			{
				strcpy(stStatus, "Registration failed");
				uiStatus= ENROLL_WPS_PROTOCOL_FAILED;
			} 
			else if (str_match(pos, WPS_EVENT_SUCCESS)) 
			{
				strcpy(stStatus, "Registration succeeded");
				uiStatus = ENROLL_WPS_INPROGRESS;
			}

			if (WLWPSService::s_iCallbackSet == 1 && uiStatus != 0)
			{
				String8 str(stStatus);
				DBGPRINT(("sWLWPSClient->notify\n"));
				WLWPSService::sWLWPSClient->notify(uiStatus,0,str);
			}
			DBGPRINT(("%s\n", stStatus));
		}

		static int
			wpaapi_create_thread(void (*in_thread_fn)(void*), void* in_arg,
			pthread_t* io_thread_hdl)
		{
			int ret;

			ret = pthread_create(io_thread_hdl, NULL,
				(void* (*)(void*)) in_thread_fn, in_arg);

			return ret;
		}

		void WLWPSService:: status_thread(void* arg)
		{
			char buf[256];
			size_t len;

			while (WLWPSService::m_bThreadLoop)
			{
				while (monitor_conn && wpa_ctrl_pending(monitor_conn) > 0) {
					len = sizeof(buf) - 1;
					if (wpa_ctrl_recv(monitor_conn, buf, &len) == 0) {
						buf[len] = '\0';
						processMsg(buf);
					}
				}
				sleep(1);
			}
			DBGPRINT(("End status_thread\n"));

		}

		int WLWPSService::wpa_cli_open_connection(const char *ifname)
		{
			int flen;
			char *cfile;

			DBGPRINT(("wpa_cli_open_connection\n"));

			strcpy(ctrl_iface, ifname);

#ifdef CONFIG_CTRL_IFACE_UNIX
			flen = strlen(ctrl_iface_dir) + strlen(ctrl_iface) + 2;
			cfile = (char *) malloc(flen);
			if (cfile == NULL)
				return -1;
			snprintf(cfile, flen, "%s/%s", ctrl_iface_dir, ctrl_iface);
#else /* CONFIG_CTRL_IFACE_UNIX */
			flen = strlen(ctrl_iface) + 1;
			cfile = (char *) malloc(flen);
			if (cfile == NULL)
				return -1;
			snprintf(cfile, flen, "%s", ctrl_iface);
#endif /* CONFIG_CTRL_IFACE_UNIX */

			if (ctrl_conn) {
				wpa_ctrl_close(ctrl_conn);
				ctrl_conn = NULL;
			}

			if (monitor_conn) {
				wpa_ctrl_detach(monitor_conn);
				wpa_ctrl_close(monitor_conn);
				monitor_conn = NULL;
			}

			DBGPRINT(("Trying to connect to '%s'\n", cfile));
			ctrl_conn = wpa_ctrl_open(cfile);
			if (ctrl_conn == NULL) {
				free(cfile);
				return -1;
			}
			monitor_conn = wpa_ctrl_open(cfile);
			free(cfile);
			if (monitor_conn == NULL) {
				wpa_ctrl_close(ctrl_conn);
				return -1;
			}
			if (wpa_ctrl_attach(monitor_conn)) {
				DBGPRINT(("Failed to attach to wpa_supplicant\n"));
				wpa_ctrl_close(monitor_conn);
				monitor_conn = NULL;
				wpa_ctrl_close(ctrl_conn);
				ctrl_conn = NULL;
				return -1;
			}
			return 0;
		}

		void WLWPSService::wpa_cli_close_connection(void)
		{
			if (monitor_conn) {
				wpa_ctrl_detach(monitor_conn);
				wpa_ctrl_close(monitor_conn);
				monitor_conn = NULL;
			}

			if (ctrl_conn)
			{
				wpa_ctrl_close(ctrl_conn);
				ctrl_conn = NULL;
			}
		}

		int WLWPSService::wpa_cli_cmd_wps_pbc(char* ssid, char* bssid)
		{
			char cmd[256];
			int res;
			char buf[2048];
			size_t buf_len ;

			removeNetworks(ssid);

			buf_len = sizeof(buf) - 1;

			/* Specific BSSID */
			res = os_snprintf(cmd, sizeof(cmd), "WPS_PBC %s", bssid);
			if (res < 0 || (size_t) res >= sizeof(cmd) - 1) {
				DBGPRINT(("Too long WPS_PBC command.\n"));
				return -1;
			}
			return wpa_ctrl_command(cmd, buf, &buf_len);
		}

		int WLWPSService::wpa_cli_cmd_wps_pin(char* ssid, char* bssid, char* pin)
		{

			char cmd[256];
			int res;
			char buf[2048];
			size_t buf_len;

			DBGPRINT(("Entering wpa_cli_cmd_wps_pin()...\n"));

			removeNetworks(ssid);

			buf_len = sizeof(buf) - 1;

			/* Use hardcoded PIN from a label */
			res = os_snprintf(cmd, sizeof(cmd), "WPS_PIN %s %s", bssid, pin);
			if (res < 0 || (size_t) res >= sizeof(cmd) - 1) {
				DBGPRINT(("Too long WPS_PIN command.\n"));	
				return -1;
			}
			return wpa_ctrl_command(cmd, buf, &buf_len);
		}

		void WLWPSService::wpa_cli_cleanup(void)
		{
			wpa_cli_close_connection();
		}

		bool WLWPSService::initializeWPA()
		{
			char wificonf[256] = "/system/etc/wifi.conf";

			if (uninitializeWPA())
			{
				DBGPRINT(("Read Config file"));
				if(loadconfig(wificonf)){
					DBGPRINT(("%s failed to load wifi config file %s\n", __FUNCTION__, wificonf));
				}
				else {
					DBGPRINT(("Read Config file completed - %s", INTERFACENAME));
					strcpy (ctrl_ifname, INTERFACENAME);
					DBGPRINT(("OS alloc completed"));
					if (strlen(ctrl_ifname) <= 0)
					{
						DBGPRINT(("No interface defined\n"));
						return false;
					}

					DBGPRINT(("ctrl_ifname=%s\n", ctrl_ifname));
					wpa_cli_open_connection(ctrl_ifname);
					if (ctrl_conn) {
						DBGPRINT(("Connection established.\n"));
					}
					else
					{
						DBGPRINT(("Not able to connect to the supplicant\n"));
						return false;
					}

					DBGPRINT(("Create thread\n"));
					if (ctrl_conn)
					{
						DBGPRINT(("Creating thread\n"));
						WLWPSService::m_bThreadLoop = 1;
						wpaapi_create_thread(status_thread, NULL, &status_thread_hdl);
					}
					mWPSInit = true;   
				}
			}
			return mWPSInit;
		}

		bool WLWPSService::uninitializeWPA()
		{
			DBGPRINT(("uninitializeWPA\n"));
			if (mWPSInit)
			{
				DBGPRINT(("uninitializeWPA 1\n"));
				if (status_thread_hdl != 0)
				{  
					WLWPSService::m_bThreadLoop = 0;
					pthread_join(status_thread_hdl, NULL);
				}

				wpa_cli_cleanup();

				mWPSInit = false;
			}
			DBGPRINT(("uninitializeWPA done\n"));
			return true;
		}

		int WLWPSService::refreshWPAList(char* pin)
		{	
			bool isPin = false;		
			resetScanListItems();

			strncpy(m_pin, pin, PIN_LENGTH-1);

			if (strlen(m_pin) > 0)
				isPin = true;

			return updateScanResults(isPin);
		}

		int  WLWPSService::enrollWPA(char* strbssid)
		{
			bool bContinue = false;
			CScanListItems* pScanItem = 0;
			WLWPSService::s_uiStatus=0;
			char ssid[33] = "no AP found\0";
			char pin[80] = "";
			int iRet = 1;

			strcpy(pin, m_pin);

			DBGPRINT(("enrollWPS --%s--\n", strbssid));

			for (int i=0;i<mListCount;i++)
			{
				pScanItem = &m_pCScanListItems[i];
				DBGPRINT(("enrollWPS --%s-- --%s-- --%s--\n", pScanItem->m_ssid, pScanItem->m_strbssid, pin));

				if (strcmp(pScanItem->m_strbssid, strbssid) == 0)
				{
					bContinue = true;
					DBGPRINT(("BSSID found\n"));
					strncpy(ssid, pScanItem->m_ssid, 32);

					pthread_mutex_lock(&WLWPSService::mutex);
					strcpy(m_SelectedSsid, ssid);
					pthread_mutex_unlock(&WLWPSService::mutex);
					break;
				}
			}

			if (bContinue)
			{
				String8 str;

				if ( strlen(pin) > 0)
				{
					iRet = wpa_cli_cmd_wps_pin(ssid, strbssid, pin);
					str = "WPS started in PIN mode";
				}
				else
				{
					iRet = wpa_cli_cmd_wps_pbc(ssid, strbssid);
					str = "WPS started in PBC mode";
				}

				if (WLWPSService::s_iCallbackSet)
				{
					DBGPRINT(("sWLWPSClient->notify\n"));
					WLWPSService::sWLWPSClient->notify(ENROLL_WPS_INPROGRESS,0,str);
				}
			}

			return iRet;
		}

#endif /* !CONFIG_CTRL_IFACE */
	};

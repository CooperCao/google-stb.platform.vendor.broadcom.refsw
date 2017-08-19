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
#ifndef ANDROID_WLWPSService_H
#define ANDROID_WLWPSService_H

#if defined ANDROID_AFTERCUPCAKE
/*There is no Utils.h directory in Eclair */
#include <utils/Log.h>
#include <utils/threads.h>
#include <utils/StringArray.h>
#include <utils/Timers.h>
#include <utils/List.h>
#include <utils/misc.h>
#include <utils/Errors.h>
#else
#include <utils.h>
#endif

#include <IWLWPSService.h>
#include <pthread.h>

#define PIN_LENGTH 80

namespace android {

	class CScanListItems;
	class WLWPSService : public BnWLWPSService
	{
	public:
		static void instantiate();

		// IWLWPSService interface
		int        setCallBack(const sp<IWLWPSClient>& client);

		int        wpsOpen();
		int        wpsClose();
		int        wpsRefreshScanList(String8 pin);
		int        wpsGetScanCount();
		String8    wpsGetScanSsid(int iIndex);
		String8    wpsGetScanBssid(int iIndex);
		int        wpsEnroll(String8 bssid);
		String8    wpsGetSsid();
		String8    wpsGetKeyMgmt();
		String8    wpsGetKey();
		String8    wpsGetEncryption();



		static  unsigned int    s_uiStatus;
		static int              s_iCallbackSet;
		static  sp<IWLWPSClient>   sWLWPSClient;

	private:
		WLWPSService();
		virtual          ~WLWPSService();
#ifndef CONFIG_CTRL_IFACE
		bool            initializeWPS();
		bool            uninitializeWPS();
		int				refreshList(char* pin);
		int             enrollWPS(char* ssid);
#else
		bool            initializeWPA();
		bool 			uninitializeWPA();
		int				refreshWPAList(char* pin);
		int  			enrollWPA(char* strbssid);
		int 			wpa_cli_cmd_wps_pbc(char* ssid, char* bssid);
		int 			wpa_cli_cmd_wps_pin(char* ssid, char* bssid, char* pin);


		static int 		str_match(const char *a, const char *b);
		static void 	wpa_cli_msg_cb(char *msg, size_t len);
		static int 		wpa_ctrl_command(const char *cmd, char* buf, size_t* len);
		static void 	getCredentials(char* ssid);
		void 			removeNetworks(char* ssid);
		int 			updateScanResults(bool isPin);
		static void 	processMsg(char *msg);
		static void 	status_thread(void* arg);
		int 			wpa_cli_open_connection(const char *ifname);
		void 			wpa_cli_close_connection(void);	
		void 			wpa_cli_cleanup(void);

		static const char 	*ctrl_iface_dir;
		char 		    ctrl_ifname[30];
		char 			ctrl_iface[30];
		pthread_t 		status_thread_hdl;
		static          pthread_cond_t cond;
		static          pthread_mutex_t mutex;
	public:
		static int 		m_bThreadLoop;
#endif
	private:
		void            resetScanListItems();
		int 			loadconfig(char * configfile);

		static  Mutex   mLock;
		bool            mWPSInit;
		CScanListItems  *m_pCScanListItems;
		int             mListCount;
		char            m_pin[PIN_LENGTH];

#ifndef CONFIG_CTRL_IFACE
		int control_supplicant(int startIt);
		int ensure_config_file_exists();
		int wifi_start_supplicant();
		int wifi_stop_supplicant();
#endif

	};
}; // namespace android

#endif // ANDROID_WLWPSService_H

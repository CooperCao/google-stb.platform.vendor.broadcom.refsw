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
#ifndef ANDROID_WLWPSCLIENT_H
#define ANDROID_WLWPSCLIENT_H

#include <IWLWPSService.h>
#include <IWLWPSClient.h>
#include <utils/String8.h>


namespace android {

	typedef void (*event_callback)(void* cookie, int msg, int ext1, String8 ext2);

	class WLWPSClient : public BnWLWPSClient
	{
	public:
		WLWPSClient();
		~WLWPSClient();
		const sp<IWLWPSService>& getWLWPSService();
		void            notify(int msg, int ext1, String8 ext2);
		int             setCallBack(event_callback cb, void * cookie);

		status_t        wpsOpen(int *iStatus);
		status_t        wpsClose(int *iStatus);
		status_t        wpsRefreshScanList(String8 pin, int *iStatus);
		status_t        wpsGetScanCount(int *count);
		status_t        wpsGetScanSsid(int iIndex, String8& ssid);
		status_t        wpsGetScanBssid(int iIndex, String8& bssid);
		status_t        wpsEnroll(String8 bssid, int *iStatus);
		status_t        wpsGetSsid(String8& ssid);
		status_t        wpsGetKeyMgmt(String8& keyMgmt);
		status_t        wpsGetKey(String8& key);
		status_t        wpsGetEncryption(String8& encType);

	private:
		class DeathNotifier: public IBinder::DeathRecipient
		{
		public:
			DeathNotifier() {}
			virtual ~DeathNotifier();

			virtual void binderDied(const wp<IBinder>& who);
		};

		Mutex                       mLock;


		friend class DeathNotifier;

		static  Mutex                           sServiceLock;
		static  Mutex                           sNotifyLock;
		static  sp<IWLWPSService>	      	    sWLWPSService;
		static  sp<DeathNotifier>               sDeathNotifier;

		event_callback mCallback;
		void * mCallbackCookie;
	};

}; // namespace android

#endif // ANDROID_WLWPSCLIENT_H


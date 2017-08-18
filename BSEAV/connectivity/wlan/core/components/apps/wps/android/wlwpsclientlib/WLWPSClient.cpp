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
#include <utils/Log.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#if defined ANDROID_AFTERCUPCAKE
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include <binder/MemoryBase.h>
#else
#include <utils/IServiceManager.h>
#include <utils/IPCThreadState.h>
#include <utils/MemoryBase.h>
#endif


#include <WLWPSClient.h>
#include <IWLWPSService.h>



namespace android {

	// client singleton for binder interface to service
	Mutex WLWPSClient::sServiceLock;
	Mutex WLWPSClient::sNotifyLock;
	sp<IWLWPSService> WLWPSClient::sWLWPSService;
	sp<WLWPSClient::DeathNotifier> WLWPSClient::sDeathNotifier;

	// establish binder interface to service
	const sp<IWLWPSService>& WLWPSClient::getWLWPSService()
	{
		Mutex::Autolock _l(sServiceLock);
		if (sWLWPSService.get() == 0) {
			sp<IServiceManager> sm = defaultServiceManager();
			sp<IBinder> binder;
			do {
				binder = sm->getService(String16("WLWPSService"));
				if (binder != 0)
					break;
				usleep(500000); // 0.5 s
			} while(true);
			if (sDeathNotifier == NULL) {
				sDeathNotifier = new DeathNotifier();
			}
			binder->linkToDeath(sDeathNotifier);
			sWLWPSService = interface_cast<IWLWPSService>(binder);
		}
		return sWLWPSService;
	}


	WLWPSClient::WLWPSClient()
	{
		LOGE("constructor");
	}


	WLWPSClient::~WLWPSClient()
	{
		LOGE("destructor");
		IPCThreadState::self()->flushCommands();
	}

	int WLWPSClient::setCallBack(event_callback cb, void * cookie)
	{
		Mutex::Autolock _l(mLock);
		const sp<IWLWPSService>& service(getWLWPSService());
		if (service != 0)
		{
			service->setCallBack(this);
			mCallback = cb;
			mCallbackCookie = cookie;
			return NO_ERROR;
		}
		else
		{
			return UNKNOWN_ERROR;
		}
	}


	void WLWPSClient::notify(int msg, int ext1, String8 ext2)
	{
		Mutex::Autolock _l(sNotifyLock);
		//LOGE("WLWPSClient = message received msg=%d, ext1=%d, ext2=%s", msg, ext1, ext2.string());
		if (mCallback)
		{
			//	LOGE("WLWPSClient = message sending to JNI msg=%d, ext1=%d, ext2=%s", msg, ext1, ext2.string());

			mCallback(mCallbackCookie, msg, ext1, ext2);
		}
	}

	void WLWPSClient::DeathNotifier::binderDied(const wp<IBinder>& who) {
		printf("WLWPSClient server died!\n");
	}

	WLWPSClient::DeathNotifier::~DeathNotifier()
	{
		Mutex::Autolock _l(sServiceLock);
	}

	status_t WLWPSClient::wpsOpen(int *iStatus)
	{
		Mutex::Autolock _l(mLock);
		const sp<IWLWPSService>& service(getWLWPSService());
		if (service != 0) {
			*iStatus = service->wpsOpen();
			return NO_ERROR;
		}
		return UNKNOWN_ERROR;
	}

	status_t WLWPSClient::wpsClose(int *iStatus)
	{
		Mutex::Autolock _l(mLock);
		const sp<IWLWPSService>& service(getWLWPSService());
		if (service != 0) {
			*iStatus = service->wpsClose();
			return NO_ERROR;
		}
		return UNKNOWN_ERROR;
	}

	status_t WLWPSClient::wpsRefreshScanList(String8 pin, int *iStatus)
	{
		Mutex::Autolock _l(mLock);
		const sp<IWLWPSService>& service(getWLWPSService());
		if (service != 0) {
			*iStatus = service->wpsRefreshScanList(pin);
			return NO_ERROR;
		}
		return UNKNOWN_ERROR;
	}

	status_t  WLWPSClient::wpsGetScanCount(int *count)
	{
		Mutex::Autolock _l(mLock);
		const sp<IWLWPSService>& service(getWLWPSService());
		if (service != 0) {
			*count = service->wpsGetScanCount();
			return NO_ERROR;
		}
		return UNKNOWN_ERROR;
	}

	status_t WLWPSClient::wpsGetScanSsid(int iIndex, String8& ssid)
	{
		Mutex::Autolock _l(mLock);
		const sp<IWLWPSService>& service(getWLWPSService());

		LOGE("WLWPSClient::wpsGetScanSsid");

		if (service != 0) {
			ssid = service->wpsGetScanSsid(iIndex);
			if (ssid.length())
			{
				return NO_ERROR;
			}
			else
			{
				return UNKNOWN_ERROR;
			}
		}
		return UNKNOWN_ERROR;
	}

	status_t WLWPSClient::wpsGetScanBssid(int iIndex, String8& bssid)
	{
		Mutex::Autolock _l(mLock);
		LOGE("WLWPSClient::wpsGetScanBssid");
		const sp<IWLWPSService>& service(getWLWPSService());
		if (service != 0) {
			bssid = service->wpsGetScanBssid(iIndex);
			if (bssid.length())
			{
				return NO_ERROR;
			}
			else
			{
				return UNKNOWN_ERROR;
			}
		}
		return UNKNOWN_ERROR;
	}

	status_t WLWPSClient::wpsEnroll(String8 bssid, int *iStatus)
	{
		Mutex::Autolock _l(mLock);
		LOGE("WLWPSClient::wpsEnroll");
		const sp<IWLWPSService>& service(getWLWPSService());
		if (service != 0) {
			*iStatus = service->wpsEnroll(bssid);
			return NO_ERROR;
		}
		return UNKNOWN_ERROR;
	}

	status_t WLWPSClient::wpsGetSsid(String8& ssid)
	{
		Mutex::Autolock _l(mLock);
		const sp<IWLWPSService>& service(getWLWPSService());
		if (service != 0) {
			ssid = service->wpsGetSsid();
			if (ssid.length())
			{
				return NO_ERROR;
			}
			else
			{
				return UNKNOWN_ERROR;
			}
		}
		return UNKNOWN_ERROR;
	}

	status_t WLWPSClient::wpsGetKeyMgmt(String8& keyMgmt)
	{
		Mutex::Autolock _l(mLock);
		const sp<IWLWPSService>& service(getWLWPSService());
		if (service != 0) {
			keyMgmt = service->wpsGetKeyMgmt();
			if (keyMgmt.length())
			{
				return NO_ERROR;
			}
			else
			{
				return UNKNOWN_ERROR;
			}
		}
		return UNKNOWN_ERROR;
	}

	status_t WLWPSClient::wpsGetKey(String8& key)
	{
		Mutex::Autolock _l(mLock);
		const sp<IWLWPSService>& service(getWLWPSService());
		if (service != 0) {
			key = service->wpsGetKey();
			if (key.length())
			{
				return NO_ERROR;
			}
			else
			{
				return UNKNOWN_ERROR;
			}
		}
		return UNKNOWN_ERROR;
	}

	status_t WLWPSClient::wpsGetEncryption(String8& encType)
	{
		Mutex::Autolock _l(mLock);
		const sp<IWLWPSService>& service(getWLWPSService());
		if (service != 0) {
			encType = service->wpsGetEncryption();
			if (encType.length())
			{
				return NO_ERROR;
			}
			else
			{
				return UNKNOWN_ERROR;
			}
		}
		return UNKNOWN_ERROR;
	}

}; // namespace android

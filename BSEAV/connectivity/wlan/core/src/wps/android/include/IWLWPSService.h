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
#ifndef ANDROID_IWLWPSService_H
#define ANDROID_IWLWPSService_H

#include <utils/RefBase.h>
#if defined ANDROID_AFTERCUPCAKE
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#else
#include <utils/IInterface.h>
#include <utils/Parcel.h>
#endif
#include <utils/String8.h>
#include <IWLWPSClient.h>

namespace android {

	class IWLWPSService: public IInterface
	{
	public:
		DECLARE_META_INTERFACE(WLWPSService);

		virtual int        setCallBack(const sp<IWLWPSClient>& client) = 0;
		virtual	int        wpsOpen() = 0;
		virtual	int        wpsClose() = 0;
		virtual	int        wpsRefreshScanList(String8 pin) = 0;
		virtual	int        wpsGetScanCount() = 0;
		virtual	String8    wpsGetScanSsid(int iIndex) = 0;
		virtual	String8    wpsGetScanBssid(int iIndex) = 0;
		virtual	int        wpsEnroll(String8 bssid) = 0;
		virtual	String8    wpsGetSsid() = 0;
		virtual	String8    wpsGetKeyMgmt() = 0;
		virtual	String8    wpsGetKey() = 0;
		virtual	String8    wpsGetEncryption() = 0;
	};

	// ----------------------------------------------------------------------------

	class BnWLWPSService: public BnInterface<IWLWPSService>
	{
	public:
		virtual status_t    onTransact( uint32_t code,
			const Parcel& data,
			Parcel* reply,
			uint32_t flags = 0);
	};

}; // namespace android

#endif // ANDROID_IWLWPSService_H

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
#include <stdint.h>
#include <sys/types.h>
#if defined ANDROID_AFTERCUPCAKE
#include <binder/Parcel.h>
#include <binder/IMemory.h>
#else
#include <utils/Parcel.h>
#include <utils/IMemory.h>
#endif
#include <IWLWPSService.h>


namespace android {

	enum {
		WPS_OPEN = IBinder::FIRST_CALL_TRANSACTION,
		WPS_CLOSE,
		WPS_REFRESH_SCAN,
		WPS_GET_SCAN_COUNT,
		WPS_GET_SCAN_SSID,
		WPS_GET_SCAN_BSSID,
		WPS_ENROLL,
		WPS_GET_SSID,
		WPS_GET_KEYMGMT,
		WPS_GET_KEY,
		WPS_GET_ENCRYPTION,
		SET_CALLBACK,
	};

	class BpWLWPSService: public BpInterface<IWLWPSService>
	{
	public:
		BpWLWPSService(const sp<IBinder>& impl)
			: BpInterface<IWLWPSService>(impl)
		{
		}


		virtual	int wpsOpen()
		{
			Parcel data, reply;
			data.writeInterfaceToken(IWLWPSService::getInterfaceDescriptor());
			remote()->transact(WPS_OPEN, data, &reply);
			return reply.readInt32();
		}

		virtual	int wpsClose()
		{
			Parcel data, reply;
			data.writeInterfaceToken(IWLWPSService::getInterfaceDescriptor());
			remote()->transact(WPS_CLOSE, data, &reply);
			return reply.readInt32();
		}

		virtual	int wpsRefreshScanList(String8 pin)
		{
			Parcel data, reply;
			data.writeInterfaceToken(IWLWPSService::getInterfaceDescriptor());
			data.writeString8(pin);
			remote()->transact(WPS_REFRESH_SCAN, data, &reply);
			return reply.readInt32();
		}


		virtual	int wpsGetScanCount()
		{
			Parcel data, reply;
			data.writeInterfaceToken(IWLWPSService::getInterfaceDescriptor());
			remote()->transact(WPS_GET_SCAN_COUNT, data, &reply);
			return reply.readInt32();
		}

		virtual	String8 wpsGetScanSsid(int iIndex)
		{
			Parcel data, reply;
			data.writeInterfaceToken(IWLWPSService::getInterfaceDescriptor());
			data.writeInt32(iIndex);
			remote()->transact(WPS_GET_SCAN_SSID, data, &reply);
			return reply.readString8();
		}

		virtual	String8 wpsGetScanBssid(int iIndex)
		{
			Parcel data, reply;
			data.writeInterfaceToken(IWLWPSService::getInterfaceDescriptor());
			data.writeInt32(iIndex);
			remote()->transact(WPS_GET_SCAN_BSSID, data, &reply);
			return reply.readString8();
		}

		virtual	int  wpsEnroll(String8 bssid)
		{
			Parcel data, reply;
			data.writeInterfaceToken(IWLWPSService::getInterfaceDescriptor());
			data.writeString8(bssid);
			remote()->transact(WPS_ENROLL, data, &reply);
			return reply.readInt32();
		}

		virtual	String8 wpsGetSsid()
		{
			Parcel data, reply;
			data.writeInterfaceToken(IWLWPSService::getInterfaceDescriptor());
			remote()->transact(WPS_GET_SSID, data, &reply);
			return reply.readString8();
		}

		virtual	String8 wpsGetKeyMgmt()
		{
			Parcel data, reply;
			data.writeInterfaceToken(IWLWPSService::getInterfaceDescriptor());
			remote()->transact(WPS_GET_KEYMGMT, data, &reply);
			return reply.readString8();
		}

		virtual	String8 wpsGetKey()
		{
			Parcel data, reply;
			data.writeInterfaceToken(IWLWPSService::getInterfaceDescriptor());
			remote()->transact(WPS_GET_KEY, data, &reply);
			return reply.readString8();
		}

		virtual	String8 wpsGetEncryption()
		{
			Parcel data, reply;
			data.writeInterfaceToken(IWLWPSService::getInterfaceDescriptor());
			remote()->transact(WPS_GET_ENCRYPTION, data, &reply);
			return reply.readString8();
		}
		virtual int setCallBack(const sp<IWLWPSClient>& client)  
		{
			Parcel data, reply;
			data.writeInterfaceToken(IWLWPSService::getInterfaceDescriptor());
			data.writeStrongBinder(client->asBinder());
			remote()->transact(SET_CALLBACK, data, &reply);
			return reply.readInt32();
		}

	};

	IMPLEMENT_META_INTERFACE(WLWPSService, "android.broadcom.IWLWPSService");

	// ----------------------------------------------------------------------

#define CHECK_INTERFACE(interface, data, reply) \
	do { if (!data.enforceInterface(interface::getInterfaceDescriptor())) { \
	LOGW("Call incorrectly routed to " #interface); \
	return PERMISSION_DENIED; \
	} } while (0)

	status_t BnWLWPSService::onTransact(
		uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
	{
		switch(code) {
	case WPS_OPEN: {
		CHECK_INTERFACE(IWLWPSService, data, reply);
		reply->writeInt32(wpsOpen());
		return NO_ERROR;
				   } break;

	case WPS_CLOSE: {
		CHECK_INTERFACE(IWLWPSService, data, reply);
		reply->writeInt32(wpsClose());
		return NO_ERROR;
					} break;

	case WPS_REFRESH_SCAN: {
		CHECK_INTERFACE(IWLWPSService, data, reply);
		reply->writeInt32(wpsRefreshScanList(data.readString8()));
		return NO_ERROR;
						   } break;

	case WPS_GET_SCAN_COUNT: {
		CHECK_INTERFACE(IWLWPSService, data, reply);
		reply->writeInt32(wpsGetScanCount());
		return NO_ERROR;
							 } break;

	case WPS_GET_SCAN_SSID: {
		CHECK_INTERFACE(IWLWPSService, data, reply);
		reply->writeString8(wpsGetScanSsid(data.readInt32()));
		return NO_ERROR;

							} break;
	case WPS_GET_SCAN_BSSID: {
		CHECK_INTERFACE(IWLWPSService, data, reply);
		reply->writeString8(wpsGetScanBssid(data.readInt32()));
		return NO_ERROR;
							 } break;

	case WPS_ENROLL: {
		CHECK_INTERFACE(IWLWPSService, data, reply);
		reply->writeInt32(wpsEnroll(data.readString8()));
		return NO_ERROR;
					 } break;

	case WPS_GET_SSID: {
		CHECK_INTERFACE(IWLWPSService, data, reply);
		reply->writeString8(wpsGetSsid());
		return NO_ERROR;
					   } break;

	case WPS_GET_KEYMGMT: {
		CHECK_INTERFACE(IWLWPSService, data, reply);
		reply->writeString8(wpsGetKeyMgmt());
		return NO_ERROR;
						  } break;

	case WPS_GET_KEY: {
		CHECK_INTERFACE(IWLWPSService, data, reply);
		reply->writeString8(wpsGetKey());
		return NO_ERROR;
					  } break;

	case WPS_GET_ENCRYPTION: {
		CHECK_INTERFACE(IWLWPSService, data, reply);
		reply->writeString8(wpsGetEncryption());
		return NO_ERROR;
							 } break;

	case SET_CALLBACK: {
		CHECK_INTERFACE(IWLWPSService, data, reply);
		sp<IWLWPSClient> client = interface_cast<IWLWPSClient>(data.readStrongBinder());
		reply->writeInt32(setCallBack(client));
		return NO_ERROR;
					   } break;
	default:
		return BBinder::onTransact(code, data, reply, flags);
		}
	}
}; // namespace android

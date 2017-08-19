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
#include <utils/RefBase.h>
#if defined ANDROID_AFTERCUPCAKE
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#else
#include <utils/IInterface.h>
#include <utils/Parcel.h>
#endif

#include <IWLWPSClient.h>

namespace android {

	enum {
		NOTIFY = IBinder::FIRST_CALL_TRANSACTION,
	};

	class BpWLWPSClient: public BpInterface<IWLWPSClient>
	{
	public:
		BpWLWPSClient(const sp<IBinder>& impl)
			: BpInterface<IWLWPSClient>(impl)
		{
		}

		virtual void notify(int msg, int ext1, String8 ext2)
		{
			Parcel data, reply;
			data.writeInterfaceToken(IWLWPSClient::getInterfaceDescriptor());
			data.writeInt32(msg);
			data.writeInt32(ext1);
			data.writeString8(ext2);
			remote()->transact(NOTIFY, data, &reply, IBinder::FLAG_ONEWAY);
		}
	};

	IMPLEMENT_META_INTERFACE(WLWPSClient, "android.broadcom.IWLWPSClient");

	// ----------------------------------------------------------------------

#define CHECK_INTERFACE(interface, data, reply) \
	do { if (!data.enforceInterface(interface::getInterfaceDescriptor())) { \
	LOGW("Call incorrectly routed to " #interface); \
	return PERMISSION_DENIED; \
	} } while (0)

	status_t BnWLWPSClient::onTransact(
		uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
	{
		switch(code) {
	case NOTIFY: {
		CHECK_INTERFACE(IWLWPSClient, data, reply);
		int msg = data.readInt32();
		int ext1 = data.readInt32();
		String8 ext2 = data.readString8();
		notify(msg, ext1, ext2);
		return NO_ERROR;
				 } break;
	default:
		return BBinder::onTransact(code, data, reply, flags);
		}
	}

}; // namespace android

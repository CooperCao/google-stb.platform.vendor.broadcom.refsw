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
#ifndef ANDROID_IWLWPSCLIENT_H
#define ANDROID_IWLWPSCLIENT_H

#include <utils/RefBase.h>
#if defined ANDROID_AFTERCUPCAKE
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#else
#include <utils/IInterface.h>
#include <utils/Parcel.h>
#endif
#include <utils/String8.h>
#define MAX_WPS_AP 10

namespace android {

	class IWLWPSClient: public IInterface
	{
	public:
		DECLARE_META_INTERFACE(WLWPSClient);

		virtual void notify(int msg, int ext1, String8 ext2) = 0;
	};

	// ----------------------------------------------------------------------------

	class BnWLWPSClient: public BnInterface<IWLWPSClient>
	{
	public:
		virtual status_t    onTransact( uint32_t code,
			const Parcel& data,
			Parcel* reply,
			uint32_t flags = 0);
	};

}; // namespace android

#endif // ANDROID_IWLWPSCLIENT_H

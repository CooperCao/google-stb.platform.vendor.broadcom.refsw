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
#include <sys/types.h>
#include <unistd.h>
#include <grp.h>
 
#if defined ANDROID_AFTERCUPCAKE
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#else
#include <utils/IPCThreadState.h>
#include <utils/ProcessState.h>
#include <utils/IServiceManager.h>
#endif

#include <utils/Log.h>


#include <WLWPSService.h>
#include <private/android_filesystem_config.h>

using namespace android; 

static int sort_func(const String16* lhs, const String16* rhs)
{
    return lhs->compare(*rhs);
}

int main(int argc, char** argv)
{
    sp<ProcessState> proc(ProcessState::self());
    sp<IServiceManager> sm = defaultServiceManager();
    if (sm == NULL) {
		LOGE("Unable to get default service manager!n");
        return 20;
    }
    LOGV("Got defaultServiceManager");
    WLWPSService::instantiate();
    ProcessState::self()->startThreadPool();
    IPCThreadState::self()->joinThreadPool();

}

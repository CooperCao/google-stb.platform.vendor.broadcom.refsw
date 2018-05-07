/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include "Drm.h"

#ifdef PRDY_SDK_V25
#include "DrmPlayReady2x.h"
#endif

#ifdef ENABLE_WIDEVINE
#include "DrmWidevine.h"
#endif

namespace Broadcom {
namespace Media {

DrmType Drm::getType(const std::string& keySystem)
{
    DrmType drmType = DrmType::NotSupported;

#ifdef PRDY_SDK_V25
    drmType = DrmPlayReady2x::getDrmType(keySystem);
    if (drmType != DrmType::NotSupported) {
        return drmType;
    }
#endif

#ifdef ENABLE_WIDEVINE
    drmType = DrmWidevine::getDrmType(keySystem);
    if (drmType != DrmType::NotSupported) {
        return drmType;
    }
#endif

    return drmType;
}

std::unique_ptr<Drm> Drm::create(
        DrmType drmType,
        MediaDrmAdaptor *mediaDrmAdaptor)
{
    std::unique_ptr<Drm> sptrDrm;

    switch (drmType) {
#ifdef PRDY_SDK_V25
    case DrmType::PlayReady2x:
        sptrDrm = std::unique_ptr<Drm>(new DrmPlayReady2x(mediaDrmAdaptor));
        return sptrDrm;
#endif

#ifdef ENABLE_WIDEVINE
    case DrmType::Widevine:
        sptrDrm = std::unique_ptr<Drm>(new DrmWidevine(mediaDrmAdaptor));
        return sptrDrm;
#endif
    }

    return nullptr;
}

Drm::Drm(MediaDrmAdaptor *mediaDrmAdaptor)
    : m_mediaDrmAdaptor(mediaDrmAdaptor),
      m_webm(false)
{
}

}  // namespace Media
}  // namespace Broadcom

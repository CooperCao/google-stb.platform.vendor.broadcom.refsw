/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef __MEDIA_DRM_BASE_H__
#define __MEDIA_DRM_BASE_H__

#include <vector>
#include <memory>
#include "Debug.h"

namespace Broadcom {
namespace Media {

class MediaDrmAdaptor;

enum class DrmType {
    NotSupported = 0,
    PlayReady2x,
    Widevine
};

class Drm
{
public:
    static DrmType getType(const std::string& keySystem);
    static std::unique_ptr<Drm> create(
            DrmType drmType,
            MediaDrmAdaptor *mediaDrmAdaptor);

    virtual ~Drm() {}
    virtual bool generateKeyRequest(const std::string& initData) = 0;
    virtual bool addKey(const std::string& key) = 0;
    virtual DrmType getDrmType() = 0;
    virtual void getSessionId(std::string& sessionId) = 0;
    virtual void getAllKeyIds(std::vector<std::string>& keyIdArray) = 0;
    virtual void* getNativeHandle(const std::string keyId) = 0;

    void setWebMFormat(bool webm) {
        m_webm = webm;
    }
    bool isWebMFormat() {
        return m_webm;
    }

protected:
    Drm(MediaDrmAdaptor *mediaDrmAdaptor);
    MediaDrmAdaptor *m_mediaDrmAdaptor;

private:
    bool m_webm;
};

}  // namespace Media
}  // namespace Broadcom

#endif  // __MEDIA_DRM_BASE_H__

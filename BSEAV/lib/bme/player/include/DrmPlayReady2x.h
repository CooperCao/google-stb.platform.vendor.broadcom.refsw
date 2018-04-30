/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef __DRM_PLAYREADY_2X_H__
#define __DRM_PLAYREADY_2X_H__

#include <map>
#include <mutex>
#include "DrmPlayReady.h"

#include "drm_prdy.h"

namespace Broadcom {
namespace Media {

struct DrmPlayReady2xContext {
    DRM_Prdy_Handle_t drmHandle;
    uint8_t sessionCount;

    DrmPlayReady2xContext() : drmHandle(nullptr), sessionCount(0) {}
};

// One MediaDrmAdaptor may create multiple DrmPlayready2x instance.
// However, these instances will use the same m_DrmHandle.
// So this map is for instance to look up the shared m_DrmHandle
// if using the same MediaDrmAdaptor.
typedef std::map<MediaDrmAdaptor*, DrmPlayReady2xContext*> DrmPlayReady2xContextMap;

class DrmPlayReady2x : public DrmPlayReady
{
public:
    static DrmType getDrmType(const std::string& keySystem);

    DrmPlayReady2x(MediaDrmAdaptor *mediaDrmAdaptor);
    virtual ~DrmPlayReady2x();

    virtual bool generateKeyRequest(const std::string& initData) OVERRIDE;
    virtual bool addKey(const std::string& key) OVERRIDE;
    virtual DrmType getDrmType() OVERRIDE {
        return s_drm_type;
    }
    virtual void getSessionId(std::string& sessionId) OVERRIDE {
        sessionId = m_session_id;
    }
    virtual void getAllKeyIds(std::vector<std::string>& keyIdArray) OVERRIDE {
        if (m_key_added) {
            keyIdArray.push_back(m_key_id);
        }
    }
    virtual void* getNativeHandle(const std::string keyId) OVERRIDE {
        return m_key_added ? m_DrmDecryptContext->pDecrypt : nullptr;
    }

private:
    static const std::string s_key_system;
    static DrmType s_drm_type;
    static std::mutex s_drm_lock;
    static uint32_t s_session_serial;
    static DrmPlayReady2xContextMap s_context_map;

    DRM_Prdy_Handle_t m_DrmHandle;
    DRM_Prdy_DecryptContext_t *m_DrmDecryptContext;

    bool m_IsSecureStopEnabled;
    std::string m_pssh;
    std::string m_wrmheader;
    std::string m_key_id;
    std::string m_session_id;
    std::string m_challenge;
    std::string m_url;
    bool m_key_added;

    bool initialize();
};

}  // namespace Media
}  // namespace Broadcom

#endif  // __DRM_PLAYREADY_2X_H__

/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef __DRM_WIDEVINE_H__
#define __DRM_WIDEVINE_H__

#include <mutex>
#include "Drm.h"
#include "MediaDrmContext.h"
#include "cdm.h"

namespace Broadcom {
namespace Media {

class DrmWidevine;

// for receiving events from widevine cdm and dispatch
// the events to the mapping DrmWidevine sessions
typedef std::map<std::string, DrmWidevine*> WidevineSessionMap;
class DrmWidevineCdm : public widevine::Cdm::IEventListener
{
public:
    DrmWidevineCdm();
    ~DrmWidevineCdm();

    widevine::Cdm* getWidevineCdm() { return pCdm; }

    bool createSession(DrmWidevine* session, std::string& session_id);
    bool closeSession(const std::string& session_id);
    bool generateKeyRequest(const std::string& session_id,
                            bool bWebM,
                            const std::string& initData);
    bool addKey(const std::string& session_id, const std::string& key);
    void getAllKeyIds(const std::string& session_id,
                      std::vector<std::string>& keyIdArray);

    // Cdm::IEventListener
    virtual void onMessage(const std::string& session_id,
                           widevine::Cdm::MessageType message_type,
                           const std::string& message) OVERRIDE;
    virtual void onKeyStatusesChange(const std::string& session_id) OVERRIDE;
    virtual void onRemoveComplete(const std::string& session_id) OVERRIDE;
    virtual void onDeferredComplete(const std::string& session_id,
                                    widevine::Cdm::Status result) OVERRIDE;
    virtual void onDirectIndividualizationRequest(
            const std::string& session_id,
            const std::string& request) OVERRIDE;

private:
    widevine::Cdm* pCdm;
    WidevineSessionMap _sessionMap;
    std::mutex _cdm_lock;
    std::string _cdm_request;

    void release();
};

class DrmWidevine : public Drm
{
public:
    static DrmType getDrmType(const std::string& keySystem);

    DrmWidevine(MediaDrmAdaptor *mediaDrmAdaptor);
    virtual ~DrmWidevine();

    virtual bool generateKeyRequest(const std::string& initData) OVERRIDE;
    virtual bool addKey(const std::string& key) OVERRIDE;
    virtual DrmType getDrmType() OVERRIDE {
        return s_drm_type;
    }
    virtual void getSessionId(std::string& sessionId) OVERRIDE {
        sessionId = _session_id;
    }
    virtual void getAllKeyIds(std::vector<std::string>& keyIdArray) OVERRIDE;
    virtual void* getNativeHandle(const std::string keyId) OVERRIDE;

    void onMessage(const std::string& message);
    void onKeyAdded();

private:
    static const std::string s_key_system;
    static DrmType s_drm_type;
    static DrmWidevineCdm s_cdm;

    std::string _session_id;
    bool _key_added;
    std::vector<std::shared_ptr<WidevineKeyContext>> _widevine_key_context;

    bool initialize();
};

}  // namespace Media
}  // namespace Broadcom

#endif  // __DRM_WIDEVINE_H__

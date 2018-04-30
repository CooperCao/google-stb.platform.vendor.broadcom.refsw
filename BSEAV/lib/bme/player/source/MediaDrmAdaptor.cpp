/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include "MediaDrmAdaptor.h"
#include "Media.h"
#include "MediaDrmContext.h"

#ifdef ENABLE_PLAYREADY
#ifdef PRDY_SDK_V25
#include "DrmPlayReady2x.h"
#endif
#endif  // ENABLE_PLAYREADY

#ifdef ENABLE_WIDEVINE
#include "DrmWidevine.h"
#endif  // ENABLE_WIDEVINE

namespace Broadcom {

namespace Media {

TRLS_DBG_MODULE(MediaDrmAdaptor);

bool MediaDrmAdaptor::isKeySystemSupported(const std::string& keySystem)
{
    DrmType drmType = Drm::getType(keySystem);
    if (drmType == DrmType::NotSupported) {
        return false;
    }
    return true;
}

// cobalt uses one MediaDrmAdaptor to create multiple sessions for the same
// drm type. So MediaDrmAdaptor needs to manage sessions using their session id.
MediaDrmAdaptor::MediaDrmAdaptor(const std::string& keySystem)
        : key_system(keySystem)
{
    drm_type = Drm::getType(key_system);
}

MediaDrmAdaptor::~MediaDrmAdaptor()
{
}

void MediaDrmAdaptor::createSessionAndGenerateRequest(
        SessionType sessionType,
        InitDataType initDataType,
        const uint8_t* initData,
        int initDataLength)
{
    std::shared_ptr<Drm> drm(Drm::create(drm_type, this));

    if (!drm) {
        BME_DEBUG_ERROR(("failed to create drm session for system %s",
                    key_system.c_str()));
        return;
    }

    drm->setWebMFormat(initDataType == InitDataType::kWebM);

    std::string initDataStr((const char*)initData, initDataLength);
    if (!drm->generateKeyRequest(initDataStr)) {
        BME_DEBUG_ERROR(("failed to generate request.\n"));
        return;
    }

    std::string sessionId;
    drm->getSessionId(sessionId);
    id_to_session[sessionId] = drm;

    BME_DEBUG_TRACE(("drm session added: %s", sessionId.c_str()));
}

void MediaDrmAdaptor::updateSession(
        const std::string& sessionId,
        const uint8_t* response,
        uint32_t responseSize)
{
    BME_DEBUG_ENTER();

    SessionMap::iterator search = id_to_session.find(sessionId);
    if (search == id_to_session.end()) {
        BME_DEBUG_ERROR(("cannot find session to update: %s", sessionId.c_str()));
        BME_DEBUG_EXIT();
        return;
    }

    std::shared_ptr<Drm> drm(search->second);
    std::string keyResponse((const char*)response, responseSize);
    drm->addKey(keyResponse);

    BME_DEBUG_EXIT();
}

void MediaDrmAdaptor::closeSession(
        const char* sessionId,
        uint32_t sessionIdLength)
{
    std::string session_id(sessionId, sessionIdLength);
    SessionMap::iterator it = id_to_session.find(session_id);

    if (it == id_to_session.end()) {
        BME_DEBUG_ERROR(("ask to close a non-exist session: %s",
                    session_id.c_str()));
        return;
    }

    std::shared_ptr<Drm> drm = it->second;
    id_to_session.erase(it);
    for (it = key_to_session.begin(); it != key_to_session.end(); ) {
        if (it->second == drm) {
            it = key_to_session.erase(it);
        } else {
            ++it;
        }
    }

    BME_DEBUG_TRACE(("drm session erased from adaptor: %s", session_id.c_str()));
}

IMedia::EncryptionType getEncryptionType(DrmType drmType)
{
#if defined(ENABLE_PLAYREADY) && defined(PRDY_SDK_V25)
    if (drmType == DrmType::PlayReady2x)
        return IMedia::PlayReady25EncryptionType;
#endif

#if defined(ENABLE_WIDEVINE)
    if (drmType == DrmType::Widevine)
        return IMedia::YouTubeWidevineEncryptionType;
#endif

    return IMedia::UnknownEncryptionType;
}

std::shared_ptr<MediaDrmContext> MediaDrmAdaptor::getMediaDrmContext(const std::string& keyId)
{
    SessionMap::iterator search = key_to_session.find(keyId);
    if (search == key_to_session.end()) {
        // cobalt thinks context is ready once ONE key is added.
        // However, cobalt may open multiple sessions and the session
        // it wants to decode may not have key added yet.
        return nullptr;
    }

    std::shared_ptr<Drm> drm(search->second);
    std::shared_ptr<MediaDrmContext> context;

    void* handle = drm->getNativeHandle(keyId);
    if (handle == nullptr) {
        return nullptr;
    }

    IMedia::EncryptionType enc = getEncryptionType(drm->getDrmType());
    context.reset(new MediaDrmContext(enc, handle));

    return context;
}

void MediaDrmAdaptor::keyMessage(const std::string& sessionId,
                                 const std::string& message,
                                 const std::string& url)
{
    notify(MediaDrmAdaptorEvents::SessionMessage,
            sessionId, message.c_str(), message.size(), url);
}

void MediaDrmAdaptor::keyAdded(const std::string& sessionId)
{
    SessionMap::iterator search = id_to_session.find(sessionId);
    if (search == id_to_session.end()) {
        BME_DEBUG_ERROR(("cannot find session: %s", sessionId.c_str()));
        return;
    }

    std::shared_ptr<Drm> drm(search->second);
    std::vector<std::string> keyIdArray;
    drm->getAllKeyIds(keyIdArray);

    for (auto it = keyIdArray.begin(); it != keyIdArray.end(); ++it) {
        key_to_session[*it] = drm;
    }

    notify(MediaDrmAdaptorEvents::SessionKeyStatusChange, sessionId, keyIdArray);
}


}  // namespace Media
}  // namespace Broadcom

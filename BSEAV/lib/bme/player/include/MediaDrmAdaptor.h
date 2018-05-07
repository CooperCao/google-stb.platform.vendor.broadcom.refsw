/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef _MEDIA_DRM_BRIDGE_
#define _MEDIA_DRM_BRIDGE_
#include "MediaDrmContext.h"
#include "Drm.h"
#include "Observable.h"

namespace Broadcom {
namespace Media {

enum class MediaDrmAdaptorEvents {
    SessionMessage,
    SessionClose,
    SessionKeyStatusChange,
    SessionKeyError,
};

enum class KeyError {
    kUnknownError = 1,
    kClientError,
    kServiceError,
    kOutputError,
    kHardwareChangeError,
    kDomainError
};

typedef std::map<std::string, std::shared_ptr<Drm>> SessionMap;
typedef std::function<void(const std::string& sessionId,
                           uint8_t* message,
                           uint32_t messageLength,
                           const std::string& url)> SessionMessageCb;

typedef std::function<void(const std::string& sessionId,
                           const std::vector<std::string>& validKeyIds)> SessionKeyStatusChangeCb;

class BME_SO_EXPORT MediaDrmAdaptor
    : public Observable<MediaDrmAdaptorEvents>
{
 public:
    static bool isKeySystemSupported(const std::string& keySystem);

    explicit MediaDrmAdaptor(const std::string& keySystem);
    virtual ~MediaDrmAdaptor();

    // The type of session to create. The valid types are defined in the spec:
    // https://w3c.github.io/encrypted-media/#idl-def-SessionType
    enum SessionType {
        kTemporary = 0,
        kPersistentLicense = 1,
        kPersistentKeyRelease = 2
    };

    // The Initialization Data Type. The valid types are defined in the spec:
    // http://w3c.github.io/encrypted-media/initdata-format-registry.html#registry
    enum InitDataType {
        kCenc = 0,
        kKeyIds = 1,
        kWebM = 2
    };

    void createSessionAndGenerateRequest(SessionType sessionType,
                                         InitDataType initDataType,
                                         const uint8_t* initData,
                                         int initDataLength);

    void updateSession(const std::string& sessionId,
                       const uint8_t* response,
                       uint32_t responseSize);

    void closeSession(const char* sessionId,
                      uint32_t sessionIdLength);

    std::shared_ptr<MediaDrmContext> getMediaDrmContext(const std::string& keyId);

    void keyMessage(const std::string& sessionId,
                    const std::string& message,
                    const std::string& url);

    void keyAdded(const std::string& sessionId);

private:
    std::string key_system;
    DrmType drm_type;
    std::string init_data;
    SessionMap id_to_session;
    SessionMap key_to_session;
};

}  // namespace Media
}  // namespace Broadcom

#endif  // _MEDIA_DRM_BRIDGE_

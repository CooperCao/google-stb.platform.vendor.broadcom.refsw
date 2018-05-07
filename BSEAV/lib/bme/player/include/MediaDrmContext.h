/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef _MEDIA_DRM_CONTEXT_
#define _MEDIA_DRM_CONTEXT_
#include <mutex>
#include "Media.h"
#include "nexus_dma.h"

#if defined(ENABLE_WIDEVINE)
#include "cdm.h"
#endif

namespace Broadcom {
namespace Media {

#if defined(ENABLE_WIDEVINE)
class WidevineKeyContext
{
public:
    widevine::Cdm* pCdm;
    std::string session_id;
    std::string key_id;

    WidevineKeyContext(widevine::Cdm* cp, std::string sid, std::string k)
        : pCdm(cp), session_id(sid), key_id(k) {}
};
#endif

class BME_SO_EXPORT MediaDrmContext
{
 public:
    MediaDrmContext(IMedia::EncryptionType encryptionType = IMedia::UnknownEncryptionType,
            void* context = NULL);
    virtual ~MediaDrmContext();
    void setContext(void *context)
    {
        this->_context = context;
    }

    bool canDecryptPartial() const; // i.e. supports non-zero 'offset' in decrypt()
    bool canChangeLayout() const;   // i.e. supports arbitrary scatter-gather in decrypt()
    void decrypt(NEXUS_DmaJobBlockSettings* dmaBlock, size_t nelem, size_t offset, uint64_t vector);

 private:
    void* _context;
    IMedia::EncryptionType _encType;
    std::mutex _drmLock;

    // PlayReady
    struct PlayReady
    {
        void* dlHandle;
        void* decryptBroadcom;
        void* decryptOpaque;
        PlayReady() : dlHandle(0), decryptBroadcom(0), decryptOpaque(0)
        {
        }
    };
    PlayReady _playReady25;
    PlayReady _playReady30;
    void decryptPR25(NEXUS_DmaJobBlockSettings* dmaBlock, size_t nelem, size_t offset, uint64_t vector);
    void decryptPR30(NEXUS_DmaJobBlockSettings* dmaBlock, size_t nelem, size_t offset, uint64_t vector);
};

}  // namespace Media
}  // namespace Broadcom

#endif  // _MEDIA_DRM_CONTEXT_

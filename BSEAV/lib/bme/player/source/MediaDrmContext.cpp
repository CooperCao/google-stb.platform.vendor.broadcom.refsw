/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include "MediaDrmContext.h"

#include <dlfcn.h>


TRLS_DBG_MODULE(MediaDrmContext);

namespace Broadcom {
namespace Media {

#define MAKE_STRING2(x) #x
#define MAKE_STRING(x) MAKE_STRING2(x)

// C++ bindings
const char *decryptBroadcomNamePR30 =
    "_ZN2PK29Drm_Reader_DecryptOpaque_BrcmEPNS_19DRM_DECRYPT_CONTEXTEPNS_28DRM_AES_COUNTER_MODE_CONTEXTEPvj";
const char *decryptOpaqueNamePR30 =
    "_ZN2PK24Drm_Reader_DecryptOpaqueEPNS_19DRM_DECRYPT_CONTEXTEjPKjyjPKhPjPPh";

MediaDrmContext::MediaDrmContext(IMedia::EncryptionType encryptionType,
        void* context)
    : _context(context),
    _encType(encryptionType)
{
#if defined(ENABLE_PLAYREADY) && defined(LIB_PLAYREADYPK25)
    _playReady25.dlHandle = dlopen(MAKE_STRING(LIB_PLAYREADYPK25), RTLD_LAZY);
    if (_playReady25.dlHandle != 0)
        _playReady25.decryptOpaque = dlsym(_playReady25.dlHandle, "Drm_Reader_DecryptOpaque");
#endif

#if defined(ENABLE_PLAYREADY) && defined(LIB_PLAYREADYPK30)
    _playReady30.dlHandle = dlopen(MAKE_STRING(LIB_PLAYREADYPK30), RTLD_LAZY);
    if (_playReady30.dlHandle != 0) {
        _playReady30.decryptBroadcom = dlsym(_playReady30.dlHandle, decryptBroadcomNamePR30);
        _playReady30.decryptOpaque   = dlsym(_playReady30.dlHandle, decryptOpaqueNamePR30);
    }
#endif
}

MediaDrmContext::~MediaDrmContext()
{
}

bool MediaDrmContext::canDecryptPartial() const
{
    switch (_encType) {
#if defined(ENABLE_PLAYREADY)
#if defined(PRDY_SDK_V30)
    case IMedia::PlayReady33EncryptionType:
        return _playReady30.decryptBroadcom != 0;
#endif
#if defined(PRDY_SDK_V25)
    case IMedia::PlayReady25EncryptionType:
        return true;
#endif
#endif
#if defined(ENABLE_WIDEVINE)
    case IMedia::YouTubeWidevineEncryptionType:
        return true;
#endif
    default:
        break;
    }
    return false;
}

bool MediaDrmContext::canChangeLayout() const
{
    switch (_encType) {
#if defined(ENABLE_PLAYREADY)
#if defined(PRDY_SDK_V30)
    case IMedia::PlayReady33EncryptionType:
        return _playReady30.decryptBroadcom != 0;
#endif
#if defined(PRDY_SDK_V25)
    case IMedia::PlayReady25EncryptionType:
        return true;
#endif
#endif
#if defined(ENABLE_WIDEVINE)
    case IMedia::YouTubeWidevineEncryptionType:
        return true;
#endif
    default:
        break;
    }
    return false;
}

void MediaDrmContext::decrypt(NEXUS_DmaJobBlockSettings *dmaBlock, size_t nelem, size_t offset, uint64_t vector)
{
    switch (_encType) {
#if defined(ENABLE_PLAYREADY)
#if defined(PRDY_SDK_V30)
    case IMedia::PlayReady33EncryptionType:
        decryptPR30(dmaBlock, nelem, offset, vector);
        break;
#endif
#if defined(PRDY_SDK_V25)
    case IMedia::PlayReady25EncryptionType:
        decryptPR25(dmaBlock, nelem, offset, vector);
        break;
#endif
#endif
#if defined(ENABLE_WIDEVINE)
    case IMedia::YouTubeWidevineEncryptionType:
    {
        const uint8_t *sourcebase = (const uint8_t*) dmaBlock[0].pSrcAddr;
        const uint8_t *targetbase = (const uint8_t*) dmaBlock[0].pDestAddr;

        WidevineKeyContext *instance = (WidevineKeyContext *)_context;

        widevine::Cdm::Status result;
        widevine::Cdm::KeyStatusMap key_statuses;
        result = instance->pCdm->getKeyStatuses(instance->session_id, &key_statuses);
        if (widevine::Cdm::kSuccess != result) {
            BME_DEBUG_ERROR(("error in getting key statuses. result=%d\n", result));
            return;
        }

        auto search = key_statuses.find(instance->key_id);
        if (search == key_statuses.end()) {
            BME_DEBUG_TRACE(("no such key"));
            return;
        }

        widevine::Cdm::KeyStatus key_status = search->second;
        if (key_status != widevine::Cdm::kUsable) {
            BME_DEBUG_ERROR(("key is not usable. status=%d\n", key_status));
            return;
        }

        std::vector<uint8_t> iv(16, 0);
        for (int i = 0; i < 8; i++) iv[7-i] = ((uint8_t*)&vector)[i];

        widevine::Cdm::OutputBuffer output_buffer;
        output_buffer.data_offset = 0;
        output_buffer.is_secure = true;

        widevine::Cdm::InputBuffer input_buffer;
        input_buffer.block_offset = offset;
        input_buffer.key_id = reinterpret_cast<const uint8_t*>(instance->key_id.data());
        input_buffer.key_id_length = instance->key_id.size();
        input_buffer.iv = iv.data();
        input_buffer.iv_length = iv.size();

        uint8_t *source = (uint8_t*) sourcebase;
        uint32_t cpos = 0;

        std::lock_guard<std::mutex> lg(_drmLock);

        for (size_t i = 0; i < nelem; i++) {
            uint32_t clearbytes = (uint8_t *)dmaBlock[i].pSrcAddr - source;
            uint32_t cypherbytes = dmaBlock[i].blockSize;
            // only specify the encrypted part
            input_buffer.data = source + clearbytes;
            input_buffer.data_length = cypherbytes;
            input_buffer.block_offset = offset % 16;
            input_buffer.first_subsample = (i == 0);
            input_buffer.last_subsample = (i == nelem-1);

            for (int i = 15, counter = offset / 16; i >= 12; i--, counter = counter >> 8) {
                iv.data()[i] = counter & 0xFF;
            }

            // put decrypted data in the mapping region in the output buffer
            output_buffer.data = (uint8_t *)targetbase+cpos+clearbytes;
            output_buffer.data_length = cypherbytes;

            result = instance->pCdm->decrypt(input_buffer, output_buffer);
            if (result != widevine::Cdm::kSuccess) {
                BME_DEBUG_ERROR(("cdm decrypt failure. result=%d", result));
                return;
            }

            source += clearbytes + cypherbytes;
            cpos += clearbytes + cypherbytes;
            offset += cypherbytes;
        }
    }
#endif // defined(ENABLE_WIDEVINE)
    default:
        break;
    }
}

}  // namespace Media
}  // namespace Broadcom

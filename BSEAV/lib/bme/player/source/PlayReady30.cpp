/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include "MediaDrmContext.h"
#include "drmmanager.h"
#include <csignal> // std::rasie


namespace {
bool addFragment(std::vector<uint32_t> &fragments, NEXUS_DmaJobBlockSettings block, const uint8_t *&source, uint8_t *&destination)
{
    size_t sourceOffset      = reinterpret_cast<const uint8_t *>(block.pSrcAddr)  - source;
    size_t destinationOffset = reinterpret_cast<      uint8_t *>(block.pDestAddr) - destination;
    if (sourceOffset != destinationOffset) {
        fprintf(stderr, "%s: Run ended\n", __FUNCTION__);
        return false;
    }

    fragments.push_back(reinterpret_cast<const uint8_t *>(block.pSrcAddr) - source); // Clear
    fragments.push_back(block.blockSize);                                            // Encrypted
    source      = reinterpret_cast<const uint8_t *>(block.pSrcAddr)  + block.blockSize;
    destination = reinterpret_cast<      uint8_t *>(block.pDestAddr) + block.blockSize;
    return true;
}
}

namespace Broadcom {
namespace Media {

TRLS_DBG_MODULE(PlayReady30);

typedef DRM_API DRM_RESULT DRM_CALL (* DecryptBroadcom30_t)(
    DRM_DECRYPT_CONTEXT          *f_pDecryptContext,
    DRM_AES_COUNTER_MODE_CONTEXT *f_pCtrContext,
    void                         *f_hDmaDescriptors,
    DRM_DWORD                     f_cbData);

typedef DRM_API DRM_RESULT DRM_CALL (* Decrypt30_t)(
    DRM_DECRYPT_CONTEXT      *f_pDecryptContext,
    DRM_DWORD                 f_cEncryptedRegionMappings,
    DRM_DWORD                *f_pdwEncryptedRegionMappings,
    DRM_UINT64                f_ui64InitializationVector,
    DRM_DWORD                 f_cbEncryptedContent,
    DRM_BYTE                 *f_pbEncryptedContent,
    DRM_DWORD                *f_pcbOpaqueClearContent,
    DRM_BYTE                **f_ppbOpaqueClearContent);

typedef DRM_API DRM_RESULT DRM_CALL (* DecryptLegacy30_t)(
    DRM_DECRYPT_CONTEXT          *f_pDecryptContext,
    DRM_AES_COUNTER_MODE_CONTEXT *f_pCtrContext,
    DRM_BYTE                     *f_pbData,
    DRM_DWORD                     f_cbData);

void MediaDrmContext::decryptPR30(NEXUS_DmaJobBlockSettings* dmaBlock, size_t nelem, size_t offset, uint64_t vector)
{
#if defined(ENABLE_PLAYREADY) && defined(PRDY_SDK_V30)
    DRM_AES_COUNTER_MODE_CONTEXT aesConfig;
    aesConfig.qwInitializationVector = vector;
    aesConfig.qwBlockOffset = offset / 16;
    aesConfig.bByteOffset = offset % 16;

    if (_playReady30.decryptBroadcom != 0)
    {
        std::lock_guard<std::mutex> lg(_drmLock);  // Drm_Reader_Decrypt*() not reentrant
        DRM_DECRYPT_CONTEXT *instance = (DRM_DECRYPT_CONTEXT *)_context;
        DecryptBroadcom30_t decrypt = reinterpret_cast<DecryptBroadcom30_t>(_playReady30.decryptBroadcom);
        (void) (*decrypt)(instance, &aesConfig, dmaBlock, nelem);
        return;
    }

    if (_playReady30.decryptOpaque == 0 || offset != 0) {
        std::raise(SIGABRT); // We can't do anything, so crash intentionally
        return;
    }

    std::vector<uint32_t> fragments; // Pairs of (clear, encrypted) byte counts
    const uint8_t *sourceStart      = reinterpret_cast<const uint8_t *>(dmaBlock[0].pSrcAddr);
    uint8_t       *destinationStart = reinterpret_cast<      uint8_t *>(dmaBlock[0].pDestAddr);
    (void) addFragment(fragments, dmaBlock[0], sourceStart, destinationStart);

    for (size_t i = 1; i < nelem; ++i) {
        // Potential issue when wrapping on non-32b buses
        if (!addFragment(fragments, dmaBlock[i], sourceStart, destinationStart)) {
            // canChangeLayout() should return false and pevent us from getting here
            std::raise(SIGABRT);  // This is a programming bug so crash intentionally
        }
    }

    DRM_DWORD bytes = 0;
    for (size_t i = 0; i < fragments.size(); ++i)
        bytes += fragments[i];

    // Decrypt
    std::lock_guard<std::mutex> lg(_drmLock);  // Drm_Reader_Decrypt*() not reentrant
    DRM_DECRYPT_CONTEXT *instance = (DRM_DECRYPT_CONTEXT *)_context;
    Decrypt30_t decrypt = reinterpret_cast<Decrypt30_t>(_playReady30.decryptOpaque);
    const uint8_t *source      = reinterpret_cast<const uint8_t *>(dmaBlock[0].pSrcAddr);
    uint8_t       *destination = reinterpret_cast<      uint8_t *>(dmaBlock[0].pDestAddr);
    (void) (*decrypt)(
        instance, fragments.size(), fragments.data(), aesConfig.qwInitializationVector, bytes,
        const_cast<uint8_t *>(source), &bytes, &destination);
#endif
}

}
}

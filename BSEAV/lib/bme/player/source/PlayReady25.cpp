/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include "MediaDrmContext.h"
#include "drmmanager.h"

#include <dlfcn.h>


namespace Broadcom {
namespace Media {

typedef DRM_API DRM_RESULT DRM_CALL (* Decrypt25_t)(
    DRM_DECRYPT_CONTEXT          *f_pDecryptContext,
    DRM_AES_COUNTER_MODE_CONTEXT *f_pCtrContext,
    OEM_OPAQUE_BUFFER_HANDLE      f_hInBuf,
    OEM_OPAQUE_BUFFER_HANDLE      f_hOutBuf,
    DRM_DWORD                     f_cbData);

void MediaDrmContext::decryptPR25(NEXUS_DmaJobBlockSettings* dmaBlock, size_t nelem, size_t offset, uint64_t vector)
{
    if (_playReady25.decryptOpaque == 0)
        return;

    // Decrypt from the source into the playpump buffer
    DRM_AES_COUNTER_MODE_CONTEXT AesConfig;
    AesConfig.qwInitializationVector = vector;
    AesConfig.qwBlockOffset = offset / 16;
    AesConfig.bByteOffset = offset % 16;

    std::lock_guard<std::mutex> lg(_drmLock);  // Drm_Reader_Decrypt*() not reentrant
    DRM_DECRYPT_CONTEXT *instance = (DRM_DECRYPT_CONTEXT *)_context;
    Decrypt25_t decrypt = reinterpret_cast<Decrypt25_t>(_playReady25.decryptOpaque);
    (void) (*decrypt)(instance, &AesConfig, dmaBlock, dmaBlock, nelem);
}

}
}

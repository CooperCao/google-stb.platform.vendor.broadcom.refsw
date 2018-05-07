/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include "string.h"
#include "Drm.h"
#include "DrmPlayReady.h"
#include "DrmPlayReady2x.h"
#include "MediaDrmAdaptor.h"

#include "Debug.h"

namespace Broadcom {
namespace Media {

TRLS_DBG_MODULE(DrmPlayReady2x);

const std::string DrmPlayReady2x::s_key_system = "com.youtube.playready";
DrmType DrmPlayReady2x::s_drm_type = DrmType::PlayReady2x;
std::mutex DrmPlayReady2x::s_drm_lock;
uint32_t DrmPlayReady2x::s_session_serial = 0;
DrmPlayReady2xContextMap DrmPlayReady2x::s_context_map;

DrmType DrmPlayReady2x::getDrmType(const std::string& keySystem)
{
    if (keySystem == s_key_system) {
        return s_drm_type;
    }

    return DrmType::NotSupported;
}

DrmPlayReady2x::DrmPlayReady2x(MediaDrmAdaptor *mediaDrmAdaptor)
    : DrmPlayReady(mediaDrmAdaptor),
      m_DrmHandle(nullptr),
      m_DrmDecryptContext(nullptr),
      m_IsSecureStopEnabled(false),
      m_key_added(false)
{
    std::lock_guard<std::mutex> lock(s_drm_lock);

    DrmPlayReady2xContext *context;
    auto it = s_context_map.find(m_mediaDrmAdaptor);
    if (it == s_context_map.end()) {
        context = new DrmPlayReady2xContext();
        s_context_map[m_mediaDrmAdaptor] = context;
    } else {
        context = s_context_map[m_mediaDrmAdaptor];
    }

    context->sessionCount += 1;
    s_session_serial += 1;

    std::ostringstream s;
    s << "playready2x_" << s_session_serial;
    m_session_id.assign(s.str());

    BME_DEBUG_TRACE(("drm session created: %s\n", m_session_id.c_str()));
}

DrmPlayReady2x::~DrmPlayReady2x()
{
    std::lock_guard<std::mutex> lock(s_drm_lock);

    BME_DEBUG_ENTER();

    DRM_Prdy_Error_e rc;

    if (m_DrmDecryptContext != nullptr) {
        rc = DRM_Prdy_Reader_Close(m_DrmDecryptContext);

        if (rc != DRM_Prdy_ok) {
            BME_DEBUG_ERROR(("DRM_Prdy_Reader_Close rc 0x%x", rc));
        }

        delete m_DrmDecryptContext;
        m_DrmDecryptContext = nullptr;
    }
    BME_DEBUG_TRACE(("drm session closed: %s\n", m_session_id.c_str()));

    auto it = s_context_map.find(m_mediaDrmAdaptor);
    if (it == s_context_map.end()) {
        BME_DEBUG_ERROR(("no context for this drm session"));
        BME_DEBUG_EXIT();
        return;
    }

    DrmPlayReady2xContext *context = s_context_map[m_mediaDrmAdaptor];
    context->sessionCount -= 1;

    BME_DEBUG_TRACE(("m_DrmHandle: %p, remaining session count: %d",
                m_DrmHandle, context->sessionCount));

    if (context->sessionCount > 0) {
        BME_DEBUG_EXIT();
        return;
    }

    // no session for this context. remove it from map.
    s_context_map.erase(it);
    delete context;

    if (m_DrmHandle == nullptr) {
        BME_DEBUG_EXIT();
        return;
    }

    BME_DEBUG_TRACE(("DRM_Prdy_Cleanup_LicenseStores to cleanup license of "
                "m_DrmHandle: %p\n", m_DrmHandle));

    rc = DRM_Prdy_Cleanup_LicenseStores(m_DrmHandle);
    if (rc != DRM_Prdy_ok) {
        BME_DEBUG_ERROR(("DRM_Prdy_Cleanup_LicenseStores failed. 0x%x", rc));
    }

    rc = DRM_Prdy_Uninitialize(m_DrmHandle);
    if (rc != DRM_Prdy_ok) {
        BME_DEBUG_ERROR(("DRM_Prdy_Uninitialize failed. 0x%x", rc));
    }

    m_DrmHandle = nullptr;
    BME_DEBUG_TRACE(("m_DrmHandle %p uninitialized"));
    BME_DEBUG_EXIT();
}

bool DrmPlayReady2x::generateKeyRequest(const std::string& initData)
{
    BME_DEBUG_ENTER();

    if (!parseInitData(initData, m_wrmheader, m_pssh)) {
        BME_DEBUG_ERROR(("failed to parse init_data\n"));
        BME_DEBUG_EXIT();
        return false;
    }

    if (!playreadyExtractKeyId(m_wrmheader, m_key_id)) {
        BME_DEBUG_ERROR(("failed to extract key id\n"));
        BME_DEBUG_EXIT();
        return false;
    }

    if (!initialize()) {
        BME_DEBUG_ERROR(("failed to initialize()\n"));
        BME_DEBUG_EXIT();
        return false;
    }

    std::lock_guard<std::mutex> lock(s_drm_lock);

    DRM_Prdy_Error_e rc;
    if (m_wrmheader.empty()) {
        BME_DEBUG_TRACE(("DRM_Prdy_Content_SetProperty with pssh"));
        rc = DRM_Prdy_Content_SetProperty(
                m_DrmHandle,
                DRM_Prdy_contentSetProperty_eAutoDetectHeader,
                (const uint8_t*)m_pssh.data(),
                m_pssh.size());
    } else {
        BME_DEBUG_TRACE(("DRM_Prdy_Content_SetProperty with wrmheader"));
        rc = DRM_Prdy_Content_SetProperty(
                m_DrmHandle,
                DRM_Prdy_contentSetProperty_eAutoDetectHeader,
                (const uint8_t*)m_wrmheader.data(),
                m_wrmheader.size());
    }
    if (rc != DRM_Prdy_ok) {
        BME_DEBUG_ERROR(("DRM_Prdy_Content_SetProperty failed. 0x%x", rc));
        BME_DEBUG_EXIT();
        return false;
    }

    char     *pChallenge = NULL;
    uint32_t challengeSize;
    char     *pUrl= NULL;
    uint32_t urlSize;
    rc =  DRM_Prdy_Get_Buffer_Size(
              m_DrmHandle,
              DRM_Prdy_getBuffer_licenseAcq_challenge,
              NULL,
              0,
              (uint32_t *) &urlSize,
              (uint32_t *) &challengeSize);

    if (rc != DRM_Prdy_ok) {
        BME_DEBUG_ERROR(("DRM_Prdy_Get_Buffer_Size failed. 0x%x", rc));
        BME_DEBUG_EXIT();
        return false;
    }
    // allocate memory for url and challenge
    if (urlSize != 0) {
        pUrl = new char[urlSize];
        if (pUrl == NULL) {
            BME_DEBUG_ERROR(("failed to allocate pUrl"));
            BME_DEBUG_EXIT();
            return false;
        }
    }

    if (challengeSize != 0) {
        pChallenge = new char[challengeSize];
        if (pChallenge == NULL) {
            delete [] pUrl;
            BME_DEBUG_ERROR(("failed to allocate pChallenge"));
            BME_DEBUG_EXIT();
            return false;
        }
    }
    // get url and challenge
    rc  = DRM_Prdy_LicenseAcq_GenerateChallenge(
              m_DrmHandle,
              NULL,
              0,
              pUrl,
              &urlSize,
              pChallenge,
              &challengeSize);

    if (rc != DRM_Prdy_ok) {
        delete [] pUrl;
        delete [] pChallenge;
        BME_DEBUG_ERROR(("DRM_Prdy_LicenseAcq_GenerateChallenge failed. 0x%X", rc));
        BME_DEBUG_EXIT();
        return false;
    }

    m_url.assign(pUrl, urlSize);
    m_challenge.assign(pChallenge, challengeSize);

    delete [] pUrl;
    delete [] pChallenge;

    uint32_t numDeleted;
    rc = DRM_Prdy_StoreMgmt_DeleteLicenses(m_DrmHandle, NULL, 0, &numDeleted);

    if (rc != DRM_Prdy_ok) {
        BME_DEBUG_ERROR(("ERROR in DRM_Prdy_StoreMgmt_DeleteLicenses: 0x%08lx",
                    static_cast<uint32_t>(rc)));
    } else {
        BME_DEBUG_TRACE(("DRM_Prdy_StoreMgmt_DeleteLicenses: %d deleted",
                    numDeleted));
    }

    m_mediaDrmAdaptor->keyMessage(m_session_id, m_challenge, m_url);
    BME_DEBUG_EXIT();
    return true;
}

bool DrmPlayReady2x::initialize()
{
    std::lock_guard<std::mutex> lock(s_drm_lock);
    BME_DEBUG_ENTER();

    DrmPlayReady2xContext *context;
    auto it = s_context_map.find(m_mediaDrmAdaptor);
    if (it == s_context_map.end()) {
        BME_DEBUG_ERROR(("failed to find context"));
        BME_DEBUG_EXIT();
        return false;
    }
    context = s_context_map[m_mediaDrmAdaptor];
    m_DrmHandle = context->drmHandle;

    DRM_Prdy_Error_e rc;
    if (m_DrmHandle == nullptr) {
        /*
         * init nexus wmdrm core, set content type to mp4
         */
        DRM_Prdy_Init_t initSettings;
        DRM_Prdy_GetDefaultParamSettings(&initSettings);
        m_DrmHandle = DRM_Prdy_Initialize(&initSettings);
        BME_DEBUG_TRACE(("m_DrmHandle: %p\n", m_DrmHandle));

        if (!m_DrmHandle) {
            BME_DEBUG_ERROR(("failed to create DRM Handle"));
            BME_DEBUG_EXIT();
            return false;
        }

        context->drmHandle = m_DrmHandle;
    }

    m_DrmDecryptContext = new DRM_Prdy_DecryptContext_t();

    if (m_DrmDecryptContext == NULL) {
        BME_DEBUG_ERROR(("failed to allocate m_DrmDecryptContext"));
        BME_DEBUG_EXIT();
        return false;
    }

    memset(m_DrmDecryptContext, 0, sizeof(DRM_Prdy_DecryptContext_t));

    BME_DEBUG_EXIT();
    return true;
}

bool DrmPlayReady2x::addKey(const std::string& key)
{
    std::lock_guard<std::mutex> lock(s_drm_lock);

    BME_DEBUG_ENTER();
    DRM_Prdy_Error_e rc;

    BME_DEBUG_TRACE(("DRM_Prdy_Content_SetProperty with wrmheader"));
    rc = DRM_Prdy_Content_SetProperty(
            m_DrmHandle,
            DRM_Prdy_contentSetProperty_eAutoDetectHeader,
            (const uint8_t*)m_wrmheader.data(),
            m_wrmheader.size());
    if (rc != DRM_Prdy_ok) {
        BME_DEBUG_ERROR(("DRM_Prdy_Content_SetProperty failed. 0x%X", rc));
        BME_DEBUG_EXIT();
        return false;
    }

    rc = DRM_Prdy_LicenseAcq_ProcessResponse(
            m_DrmHandle,
            key.c_str(),
            key.size(),
            NULL);
    if (rc != DRM_Prdy_ok) {
        BME_DEBUG_ERROR(("DRM_Prdy_LicenseAcq_ProcessResponse failed. 0x%X", rc));
        BME_DEBUG_EXIT();
        return false;
    }

    rc = DRM_Prdy_Reader_Bind(m_DrmHandle, m_DrmDecryptContext);
    if (rc != DRM_Prdy_ok) {
        BME_DEBUG_ERROR(("DRM_Prdy_Reader_Bind failed. 0x%X", rc));
        BME_DEBUG_EXIT();
        return false;
    }

    rc = DRM_Prdy_Reader_Commit(m_DrmHandle);
    if (rc != DRM_Prdy_ok) {
        BME_DEBUG_ERROR(("DRM_Prdy_Reader_Commit failed. 0x%X", rc));
        BME_DEBUG_EXIT();
        return false;
    }

    m_key_added = true;

    BME_DEBUG_TRACE(("drm session key added: %s\n", m_session_id.c_str()));
    m_mediaDrmAdaptor->keyAdded(m_session_id);
    BME_DEBUG_EXIT();
    return true;
}

}  // namespace Media
}  // namespace Broadcom

/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
#ifndef _PR30_DECRYPTOR_H_
#define _PR30_DECRYPTOR_H_

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

#include "decryptor.h"
#include "drm_data.h"
#include "drmnamespace.h"
#include "drmmanager.h"

#include <queue>

#define DRM_MAXIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE ( 64 * MINIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE )

namespace dif_streamer
{

class Playready30Decryptor : public BaseDecryptor
{
public:
    Playready30Decryptor();
    virtual ~Playready30Decryptor();

    // Decryptor implementation
    virtual bool Initialize(std::string& pssh) OVERRIDE;

    virtual bool GenerateKeyRequest(std::string initData,
        SessionType type = session_type_eTemporary) OVERRIDE;

    virtual std::string GetKeyRequestResponse(std::string url) OVERRIDE;

    virtual bool AddKey(std::string key) OVERRIDE;

    virtual bool CancelKeyRequest() OVERRIDE;

    virtual uint32_t DecryptSample(SampleInfo *pSample,
        IBuffer *input, IBuffer *output,
        uint32_t sampleSize) OVERRIDE;

    virtual void SetKeyId(std::string keyId) { m_keyId.assign(keyId); }

//    bool GetProtectionPolicy(DRM_Prdy_policy_t *policy);

    bool IsValid() { return m_valid; }

protected:
    DRM_DECRYPT_CONTEXT* m_drmDecryptContext;
    static DRM_APP_CONTEXT* s_pDrmAppCtx;
    static DRM_VOID* s_pOEMContext;
    static DRM_BYTE* s_pbOpaqueBuffer;
    static DRM_DWORD s_cbOpaqueBuffer;
    static DRM_BYTE* s_pbRevocationBuffer;

private:
    static uint32_t s_nextSessionId;
    static uint8_t s_sessionNum;

    bool m_valid;
    std::string m_wrmheader;
    std::string m_initData;
    std::string m_keyId;
    uint32_t m_systemCode;

    Playready30Decryptor(const Playready30Decryptor&);
    void operator=(const Playready30Decryptor&);
};

} // namespace dif_streamer

#pragma GCC diagnostic pop
#endif // _PR_DECRYPTOR_H_

/******************************************************************************
 *    (c)2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 * DRM Integration Framework
 *
 *****************************************************************************/
#ifndef _ANDROID_WV_DECRYPTOR_H_
#define _ANDROID_WV_DECRYPTOR_H_

#include "decryptor.h"
#include "wv_content_decryption_module.h"
#include "wv_cdm_constants.h"

#include <queue>

#define USE_MOCK_OEMCRYPTO 0

/*
 * Flags indicating data endpoints in OEMCrypto_DecryptCTR.
 */
#define OEMCrypto_FirstSubsample 1
#define OEMCrypto_LastSubsample 2

namespace dif_streamer
{

const std::string kWidevineKeySystem = "com.widevine.alpha";

// TODO: probably we don't need this
typedef struct DRM_Context {
    WvContentDecryptionModule decryptor;
    CdmKeyMessage key_msg;
    CdmSessionId session_id;
    CdmKeySetId key_set_id;
} DRM_Context;

class AndroidWidevineDecryptor : public BaseDecryptor
{
public:
    AndroidWidevineDecryptor();
    virtual ~AndoidWidevineDecryptor();

    // Decryptor implementation
    virtual bool Initialize(std::string& pssh) OVERRIDE;

    virtual bool GenerateKeyRequest(std::string initData) OVERRIDE;

    virtual std::string GetKeyRequestResponse(std::string url, std::string clientAuth) OVERRIDE;

    virtual bool AddKey(std::string key) OVERRIDE;

    virtual bool CancelKeyRequest() OVERRIDE;

    virtual int DecryptSample(SampleInfo *pSample,
        IBuffer *input, IBuffer *output,
        uint32_t* bytes_processed) OVERRIDE;

    void SetKeyId(std::string keyId) { m_keyId.assign(keyId); }

    static std::string msgBuffer;

protected:
    cdm::ContentDecryptionModule *m_cdm;

private:
    static bool s_provisioned;
    std::string m_keySetId;
    std::string m_initData;
    std::string m_keyId;
    cdm::MediaKeyError m_errorCode;
    uint32_t m_systemCode;

    CORE_DISALLOW_COPY_AND_ASSIGN(AndroidWidevineDecryptor);
};

} // namespace dif_streamer

#endif // _ANDROID_WV_DECRYPTOR_H_

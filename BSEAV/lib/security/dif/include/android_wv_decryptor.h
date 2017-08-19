/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

class WidevineDecryptor : public BaseDecryptor
{
public:
    WidevineDecryptor();
    virtual ~WidevineDecryptor();

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

    void SetKeyId(std::string keyId) { m_keyId.assign(keyId); }

    bool IsValid() { return m_valid; }

protected:
    wvcdm::WvContentDecryptionModule *m_cdm;

private:
    bool m_valid;
    std::string m_keySetId;
    std::string m_initData;
    std::string m_keyId;
    uint32_t m_systemCode;

    CORE_DISALLOW_COPY_AND_ASSIGN(WidevineDecryptor);
};

} // namespace dif_streamer

#endif // _ANDROID_WV_DECRYPTOR_H_

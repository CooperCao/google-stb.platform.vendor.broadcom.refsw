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
#ifndef _WV_DECRYPTOR_H_
#define _WV_DECRYPTOR_H_

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include "decryptor.h"
#include "cdm.h"
#include "wv_cdm_constants.h"
#include <stdio.h>
#include <map>
#include <queue>

#define OEMCrypto_FirstSubsample 1
#define OEMCrypto_LastSubsample 2

using namespace wvcdm;
using namespace widevine;

namespace dif_streamer {

class CdmHost :
    public widevine::Cdm::IStorage,
    public widevine::Cdm::IClock,
    public widevine::Cdm::ITimer
{
public:
    CdmHost();
    virtual ~CdmHost();
    void Reset();
    virtual bool read(const std::string& name,
        std::string* data) OVERRIDE;
    virtual bool write(const std::string& name,
        const std::string& data) OVERRIDE;
    virtual bool exists(const std::string& name) OVERRIDE;
    virtual bool remove(const std::string& name) OVERRIDE;
    virtual int32_t size(const std::string& name) OVERRIDE;
#ifdef WV_CDM_V32
    virtual bool list(std::vector<std::string>* names) OVERRIDE;
#endif

    virtual int64_t now() OVERRIDE;

    virtual void setTimeout(int64_t delay_ms,
        IClient* client,
        void* context) OVERRIDE;
    virtual void cancel(IClient* client) OVERRIDE;

private:
    struct Timer {
        Timer(int64_t expiry_time, IClient* client, void* context)
            : expiry_time(expiry_time), client(client), context(context) {}

        bool operator<(const Timer& other) const {
            // We want to reverse the order so that the smallest expiry times go to
            // the top of the priority queue.
            return expiry_time > other.expiry_time;
        }

        int64_t expiry_time;
        IClient* client;
        void* context;
    };

    int64_t now_;
    std::priority_queue<Timer> timers_;

    std::string m_basePath;
};

class Widevine3xDecryptor : public BaseDecryptor, public Cdm::IEventListener{
public:
    Widevine3xDecryptor();
    virtual ~Widevine3xDecryptor();

    // Decryptor implementation
    virtual bool Initialize(std::string& pssh) OVERRIDE;

    virtual bool GenerateKeyRequest(std::string initData,
        SessionType type = session_type_eTemporary) OVERRIDE;

    virtual std::string GetKeyRequestResponse(std::string url) OVERRIDE;

    virtual bool AddKey(std::string key) OVERRIDE;

    virtual bool CancelKeyRequest() OVERRIDE;

    virtual uint32_t DecryptSample(SampleInfo *pSample, IBuffer *input, IBuffer *output, uint32_t sampleSize) OVERRIDE;

    virtual void onKeyStatusesChange(const std::string& session_id) OVERRIDE;

    virtual void onRemoveComplete(const std::string& session_id) OVERRIDE;

    virtual void onDeferredComplete(const std::string& session_id, Cdm::Status result) OVERRIDE;

    virtual void onDirectIndividualizationRequest(const std::string& session_id, const std::string& request) OVERRIDE;

    virtual void onMessage(const std::string& session_id, Cdm::MessageType message_type, const std::string& message) OVERRIDE;

    virtual bool Load(std::string license) OVERRIDE;

    void SetKeyId(std::string keyId) { m_keyId.assign(keyId); }

    bool IsValid() { return m_valid; }

    bool HasNewKeyMessage() {
        return m_hasKeyMessage;
    }

protected:
    widevine::Cdm *m_cdm;
    CdmHost* m_cdmHost = NULL;

private:
    bool Provision();

    bool m_valid;
    std::string m_url;
    std::string m_initData;
    std::string m_keyId;

    std::string m_certReq;
    bool m_gotCertReq;
    std::string m_certificate;

    Cdm::SessionType m_cdmSessionType;

    Cdm::MessageType m_messageType;
    bool m_sendkeyreq = false;
    double m_currentTime;
    bool m_hasKey = false;
    bool m_hasKeyMessage;
};

} // namespace dif_streamer
#pragma GCC diagnostic pop
#endif // _WV_DECRYPTOR_H_

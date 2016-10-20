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
#ifndef _WV_DECRYPTOR_H_
#define _WV_DECRYPTOR_H_

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include "decryptor.h"
#include "content_decryption_module.h"
#include "wv_cdm_constants.h"

#include <map>
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

class CdmOutputBuffer : public cdm::Buffer {
 public:

  virtual void Destroy() OVERRIDE {}

  virtual int32_t Capacity() const OVERRIDE {return capacity_;}
  virtual uint8_t* Data() OVERRIDE {return buffer_;}
  virtual void SetSize(int32_t size) OVERRIDE {size_ = size;}
  virtual int32_t Size() const OVERRIDE {return size_;}

  // CdmOutputBuffer can only be created by calling Create().
  CdmOutputBuffer(uint8_t* buffer, uint32_t capacity)
      : buffer_(buffer),
        capacity_(capacity) {
  }
  // CdmOutputBuffer can only be destroyed by calling Destroy().
  virtual ~CdmOutputBuffer() {
  }
  uint8_t* buffer_;
  int32_t capacity_;
  int32_t size_;

  CdmOutputBuffer(const CdmOutputBuffer&);
  void operator=(const CdmOutputBuffer&);
};

class ProxyDecryptedBlock : public cdm::DecryptedBlock {
 public:
  ProxyDecryptedBlock() : buffer_(NULL), timestamp_(0) {}
  virtual ~ProxyDecryptedBlock() {}

  virtual void SetDecryptedBuffer(cdm::Buffer* buffer) OVERRIDE  {
    buffer_ = buffer;
  }

  virtual cdm::Buffer* DecryptedBuffer() OVERRIDE { return buffer_; }

  virtual void SetTimestamp(int64_t timestamp) OVERRIDE {
    timestamp_ = timestamp;
  }
  virtual int64_t Timestamp() const OVERRIDE { return timestamp_; }

 private:
  cdm::Buffer* buffer_;
  int64_t timestamp_;

  ProxyDecryptedBlock(const ProxyDecryptedBlock&);
  void operator=(const ProxyDecryptedBlock&);
};

class WidevineDecryptor : public cdm::Host, public BaseDecryptor
{
public:
    WidevineDecryptor();
    virtual ~WidevineDecryptor();

    // Decryptor implementation
    virtual bool Initialize(std::string& pssh) OVERRIDE;

    virtual bool GenerateKeyRequest(std::string initData) OVERRIDE;

    virtual std::string GetKeyRequestResponse(std::string url) OVERRIDE;

    virtual bool AddKey(std::string key) OVERRIDE;

    virtual bool CancelKeyRequest() OVERRIDE;

    virtual uint32_t DecryptSample(SampleInfo *pSample,
        IBuffer *input, IBuffer *output,
        uint32_t sampleSize) OVERRIDE;

    // cdm::Host implementation.
    virtual cdm::Buffer* Allocate(int32_t capacity) OVERRIDE;

    virtual void SetTimer(int64_t delayMs, void* context) OVERRIDE;

    virtual double GetCurrentWallTimeInSeconds() OVERRIDE;

    virtual void SendKeyMessage(const char* sessionId, int32_t sessionIdLength,
        const char* message, int32_t messageLength,
        const char* defaultUrl, int32_t defaultUrlLength) OVERRIDE;

    virtual void SendKeyError(const char* sessionId, int32_t sessionIdLength,
        cdm::MediaKeyError errorCode, uint32_t systemCode) OVERRIDE;

    virtual void GetPlatformString(const std::string& name,
        std::string* value) OVERRIDE;

    virtual void SetPlatformString(const std::string& name,
        const std::string& value) OVERRIDE;

    bool IsKeyValid(const std::string& key_id);

    bool HasNewKeyMessage() {
        return m_hasKeyMessage;
    }

    void SetKeyId(std::string keyId) { m_keyId.assign(keyId); }

    bool IsValid() { return m_valid; }

protected:
    cdm::ContentDecryptionModule *m_cdm;

private:
    bool m_valid;
    std::string m_keySetId;
    std::string m_initData;
    std::string m_keyId;
    cdm::MediaKeyError m_errorCode;
    uint32_t m_systemCode;

    // Internal functions to handle certificate file
    bool CreateDeviceCertificate();
    bool ReadDeviceCertificate();
    bool WriteDeviceCertificate(const char* buffer, size_t bytes);

    FILE* m_file;
    std::string m_basePath;
    std::string m_certFilePath;
    std::string m_certificate;

private:
    struct Timer {
        Timer(double expiryTime, void* context)
            : m_expiryTime(expiryTime), m_context(context) {}

        bool operator<(const Timer& other) const {
            // We want to reverse the order so that the smallest expiry times go to
            // the top of the priority queue.
            return m_expiryTime > other.m_expiryTime;
        }

        double m_expiryTime;
        void* m_context;
    };

    double m_currentTime;
    std::priority_queue<Timer> m_timers;

    bool m_hasKeyMessage;
    bool m_hasKeyError;

    std::map<std::string, std::string> m_platformStrings;

    WidevineDecryptor(const WidevineDecryptor&);
    void operator=(const WidevineDecryptor&);
};

} // namespace dif_streamer

#pragma GCC diagnostic pop
#endif // _WV_DECRYPTOR_H_

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
#ifndef _DECRYPTOR_H_
#define _DECRYPTOR_H_
#include "streamer.h"

namespace dif_streamer
{

    typedef enum SessionType {
        session_type_eTemporary = 0,
        session_type_ePersistent,
        session_type_ePersistentUsageRecord
    } SessionType;

class IDecryptor;

class DecryptorFactory
{
public:
    static IDecryptor* CreateDecryptor(DrmType type);
    static void DestroyDecryptor(IDecryptor* decryptor);
};

class IDecryptor
{
public:
    IDecryptor() {}
    virtual ~IDecryptor() {}

    virtual bool Initialize(std::string& pssh) = 0;

    virtual bool GenerateKeyRequest(std::string initData,
        SessionType type = session_type_eTemporary) = 0;

    virtual std::string GetKeyRequestResponse(std::string url) = 0;

    virtual bool AddKey(std::string key) = 0;

    virtual bool CancelKeyRequest() = 0;

    virtual uint32_t DecryptSample(SampleInfo *pSample, IBuffer *input, IBuffer *output, uint32_t sampleSize) = 0;

    virtual std::string GetSessionId() = 0;
    virtual std::string GetKeyMessage() = 0;
    virtual std::string GetDefaultUrl() = 0;
};

class BaseDecryptor : public IDecryptor
{
public:
    BaseDecryptor() {}
    virtual ~BaseDecryptor() {}

    virtual bool Initialize(std::string& pssh) OVERRIDE = 0;

    virtual bool GenerateKeyRequest(std::string initData,
        SessionType type = session_type_eTemporary) OVERRIDE = 0;

    virtual std::string GetKeyRequestResponse(std::string url) OVERRIDE = 0;

    virtual bool AddKey(std::string key) OVERRIDE = 0;

    virtual bool CancelKeyRequest() OVERRIDE = 0;

    virtual uint32_t DecryptSample(SampleInfo *pSample, IBuffer *input, IBuffer *output, uint32_t sampleSize) OVERRIDE = 0;

    virtual std::string GetSessionId() OVERRIDE { return m_sessionId; }
    virtual std::string GetKeyMessage() OVERRIDE { return m_keyMessage; }
    virtual std::string GetDefaultUrl() OVERRIDE { return m_defaultUrl; }

protected:
    std::string m_pssh;
    std::string m_sessionId;
    std::string m_keyMessage;
    std::string m_defaultUrl;
};

} // namespace dif_streamer

#endif // _DECRYPTOR_H_

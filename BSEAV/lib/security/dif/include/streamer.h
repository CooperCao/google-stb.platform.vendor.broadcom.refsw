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
#ifndef _STREAMER_H_
#define _STREAMER_H_
#include "nexus_platform.h"
#include "nexus_playback.h"
#include "nexus_dma.h"
#include "nexus_security.h"

#include "bmp4.h"

#define DEBUG_OUTPUT_CAPTURE 0
#define DEFAULT_VIDEO_MULTIPLE 7

#include <string>

#if defined(__clang__)
#define OVERRIDE override
#else
#define OVERRIDE
#endif

namespace dif_streamer
{

typedef enum DrmType {
    drm_type_eUnknown,     /* unknown/not supported DRM Type */
    drm_type_eClear,
    drm_type_eWidevine,    /* WideVine DRM*/
    drm_type_ePlayready,   /* Playready DRM*/
    drm_type_eMax
} DrmType;


class IBuffer;

// This is used to create a Buffer
class BufferFactory
{
public:
    static IBuffer* CreateBuffer(size_t size, uint8_t* nexusBuf = NULL, bool secure = false);
    static void DestroyBuffer(IBuffer* buffer);
};

// This is used as an input for decrypt
class IBuffer
{
public:
    virtual ~IBuffer() {}

    virtual bool Initialize() = 0;

    virtual void Copy(uint32_t offset, uint8_t* dataToCopy, size_t size) = 0;

    virtual void Copy(uint32_t offset, IBuffer* bufToCopy, size_t size) = 0;

    virtual bool IsSecure() = 0;

    virtual size_t GetSize() = 0;

    virtual uint8_t* GetPtr() = 0;
};

class BaseBuffer : public IBuffer
{
public:
    BaseBuffer(size_t size, uint8_t* nexusBuf) {
        m_size = size;
        m_data = nexusBuf;
        if (nexusBuf != NULL)
            m_givenBuffer = true;
        else
            m_givenBuffer = false;
    }

    virtual ~BaseBuffer() {}

    virtual bool Initialize() OVERRIDE = 0;

    virtual void Copy(uint32_t offset, uint8_t* dataToCopy, size_t size) OVERRIDE = 0;

    virtual void Copy(uint32_t offset, IBuffer* bufToCopy, size_t size) OVERRIDE = 0;

    virtual bool IsSecure() OVERRIDE { return false; }

    virtual size_t GetSize() OVERRIDE { return m_size; }

    virtual uint8_t* GetPtr() OVERRIDE { return m_data; }

protected:
    bool m_givenBuffer;
    size_t m_size;
    uint8_t* m_data;
};

class Buffer : public BaseBuffer
{
public:
    Buffer(size_t size, uint8_t* nexusBuf) : BaseBuffer(size, nexusBuf) {};
    virtual ~Buffer();

    virtual bool Initialize() OVERRIDE;

    virtual void Copy(uint32_t offset, uint8_t* dataToCopy, size_t size) OVERRIDE;

    virtual void Copy(uint32_t offset, IBuffer* bufToCopy, size_t size) OVERRIDE;
    virtual bool IsSecure() OVERRIDE { return false; }
};

// This is used to write data to playpump
//  and also used for decrypt output.
// The caller needs to call SecureStreamer's GetBuffer()
// to get this buffer
class SecureBuffer : public BaseBuffer
{
public:
    SecureBuffer(size_t size, uint8_t* nexusBuf) : BaseBuffer(size, nexusBuf) {};
    virtual ~SecureBuffer();

    virtual bool Initialize() OVERRIDE;

    virtual void Copy(uint32_t offset, uint8_t* dataToCopy, size_t size) OVERRIDE;

    virtual void Copy(uint32_t offset, IBuffer* bufToCopy, size_t size) OVERRIDE;

    virtual bool IsSecure() OVERRIDE { return true; }

    void SetDmaJob(NEXUS_DmaJobHandle dmaJob) { m_dmaJob = dmaJob; }
private:
    void PrivateCopy(void *pDest, const void *pSrc, size_t nSize, bool flush);

    NEXUS_DmaHandle m_dmaHandle;
    NEXUS_DmaJobHandle m_dmaJob;
};

class IStreamer;

// This is used to create a Streamer
class StreamerFactory
{
public:
    // unsecureWithSage can be true only for testing a special case
    // where unsecure version of Streamer is tested with Sage.
    static IStreamer* CreateStreamer(bool unsecureWithSage = false);

    static void DestroyStreamer(IStreamer* streamer);
};

// IStreamer interface class
class IStreamer
{
public:
    virtual ~IStreamer() {}

    virtual bool Initialize() = 0;

    // Used to open a playpump
    virtual NEXUS_PlaypumpHandle OpenPlaypump(bool isVideo) = 0;

    // Used to open a pid channel
    virtual NEXUS_PidChannelHandle OpenPidChannel(
        unsigned pid,
        const NEXUS_PlaypumpOpenPidChannelSettings *pSettings) = 0;

    // Used to allocate the destination buffer
    virtual IBuffer* GetBuffer(size_t size) = 0;

    // Used to push data to AV pipeline
    virtual bool Push(uint32_t size) = 0;
};

// Parent class to provide streamer API
// Streamer is used to get buffer and push to playpump
class BaseStreamer : public IStreamer
{
public:
    BaseStreamer();
    virtual ~BaseStreamer() {}

    virtual bool Initialize() OVERRIDE { return true; }

    virtual NEXUS_PlaypumpHandle OpenPlaypump(bool isVideo) OVERRIDE;

    virtual NEXUS_PidChannelHandle OpenPidChannel(
        unsigned pid,
        const NEXUS_PlaypumpOpenPidChannelSettings *pSettings) OVERRIDE;

    virtual IBuffer* GetBuffer(size_t size) OVERRIDE;

    virtual bool Push(uint32_t size) OVERRIDE;

protected:
    virtual bool SetupPlaypump(NEXUS_PlaypumpOpenSettings &playpumpOpenSettings) = 0;
    virtual bool SetupPidChannel() { return true; }
    virtual IBuffer* CreateBuffer(size_t size, uint8_t* data) = 0;

    size_t m_internallyPushed;
    size_t m_offset;
    NEXUS_PlaypumpHandle m_playpump;
    uint8_t* m_playpumpBuffer;
    NEXUS_PidChannelHandle m_pidChannel;
};

class Streamer : public BaseStreamer
{
public:
    Streamer();
    virtual ~Streamer();

protected:
    virtual bool SetupPlaypump(NEXUS_PlaypumpOpenSettings &playpumpOpenSettings) OVERRIDE;
    virtual IBuffer* CreateBuffer(size_t size, uint8_t* data) OVERRIDE;
};

// This class is used to get buffer and push to playpump
class SecureStreamer : public BaseStreamer
{
public:
    SecureStreamer();
    virtual ~SecureStreamer();

    virtual bool Initialize() OVERRIDE;

protected:
    virtual bool SetupPlaypump(NEXUS_PlaypumpOpenSettings &playpumpOpenSettings) OVERRIDE;
    virtual bool SetupPidChannel() OVERRIDE;
    virtual IBuffer* CreateBuffer(size_t size, uint8_t* data) OVERRIDE;
    NEXUS_DmaJobHandle GetDmaJob() { return m_dmaJob; }

private:
    NEXUS_DmaHandle m_dmaHandle;
    NEXUS_DmaJobHandle m_dmaJob;
};

} // namespace dif_streamer

#endif // _STREAMER_H_

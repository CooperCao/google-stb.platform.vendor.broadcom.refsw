/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
// stdint must be included after bstd(_defs)
#include "bstd.h"
#include <stdint.h>
#include <stdio.h>

#include "bkni.h"
#include "WavFormatHeader.h"
#include "bmedia_util.h"
#include "nexus_types.h"
#include "nexus_memory.h"
#include "nexus_base_mmap.h"
#include "nexus_platform_client.h"

typedef uint UINT32_C;

namespace Broadcom
{
namespace Media
{

TRLS_DBG_MODULE(WavFormatHeader);

#define B_WAVFORMAT_HEADER_LEN (bmedia_frame_bcma.len + 4 +  BMEDIA_WAVEFORMATEX_BASE_SIZE)

static NEXUS_HeapHandle FindNexusHeap(NEXUS_MemoryType Access)
{
    NEXUS_ClientConfiguration PlatformConfig;
    NEXUS_Platform_GetClientConfiguration(&PlatformConfig);
    for (unsigned int i = 1; i < NEXUS_MAX_HEAPS; ++i) {
        if (PlatformConfig.heap[i])
        {
            NEXUS_MemoryStatus HeapStatus;
            NEXUS_Heap_GetStatus(PlatformConfig.heap[i], &HeapStatus);
            if ((HeapStatus.memoryType & NEXUS_MemoryType_eFull) != 0)
                return PlatformConfig.heap[i];
        }
    }
    return 0;
}

WavFormatHeader::WavFormatHeader(IMedia::AudioParameters param) {
    Initialize(param);
}

WavFormatHeader::~WavFormatHeader() {
    Deinitialise();
}

bool WavFormatHeader::Initialize(IMedia::AudioParameters param) {
    BME_DEBUG_ENTER();
    uint32_t bytes = 0;

    wavHeader = NULL;
    wavHeaderSize = 0;
    extHeader = NULL;
    extHeaderSize = 0;

    if (param.audioCodec != IMedia::PcmWavAudioCodec &&
         param.audioCodec != IMedia::VorbisAudioCodec &&
         param.audioCodec != IMedia::OpusAudioCodec) {
        BME_DEBUG_TRACE(("%s: not set wave header; codec:%d", __FUNCTION__, param.audioCodec));
        return true;
    }

    codec = param.audioCodec;
    wavHeader = AllocateBuffer(B_WAVFORMAT_HEADER_LEN);
    if (!wavHeader)
        return false;
    wavHeaderSize = B_WAVFORMAT_HEADER_LEN;

    uint8_t *pData = wavHeader;
    bmedia_waveformatex_header wf;
    bmedia_init_waveformatex(&wf);

    if (param.audioCodec == IMedia::PcmWavAudioCodec) {
        wf.wFormatTag = BMEDIA_WAVFMTEX_AUDIO_PCM_TAG;
        wf.nSamplesPerSec = param.samplesPerSecond;
        wf.nChannels = param.numChannels;
        wf.wBitsPerSample = param.bitsPerSample;
        wf.nBlockAlign = wf.nChannels * wf.wBitsPerSample/8;
        wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
        wf.cbSize =  0;

        BKNI_Memcpy(pData, bmedia_frame_bcma.base, bmedia_frame_bcma.len);
        pData += (bmedia_frame_bcma.len + 4);
        bmedia_write_waveformatex(pData, &wf);
    } else if (param.audioCodec  == IMedia::VorbisAudioCodec ||
                  param.audioCodec  == IMedia::OpusAudioCodec) {
        if (param.audioCodec  == IMedia::VorbisAudioCodec) {
            wf.wFormatTag = 0xFFFF;
        } else if (param.audioCodec == IMedia::OpusAudioCodec) {
            wf.wFormatTag = 0xFFFD;
        }
        wf.nSamplesPerSec = param.samplesPerSecond;
        wf.nChannels = param.numChannels;
        wf.cbSize =  0;

        BKNI_Memcpy(pData, bmedia_frame_bcma.base, bmedia_frame_bcma.len);
        pData += (bmedia_frame_bcma.len + 4);
        bmedia_write_waveformatex(pData, &wf);
    }
    BME_DEBUG_EXIT();
    return true;
}

void WavFormatHeader::Deinitialise() {
    BME_DEBUG_ENTER();
    if (wavHeader)
        FreeBuffer(wavHeader);

    if (extHeader)
        FreeBuffer(extHeader);

    wavHeader = NULL;
    wavHeaderSize = 0;
    extHeader = NULL;
    extHeaderSize = 0;
    codec = IMedia::UnknownAudioCodec;
    BME_DEBUG_EXIT();
}

uint8_t* WavFormatHeader::AllocateBuffer(uint32_t bytes) {
    BME_DEBUG_ENTER();

    NEXUS_MemoryAllocationSettings settings;
    bool bAllocate = false;
    uint8_t *pBuffer = NULL;

    NEXUS_Memory_GetDefaultAllocationSettings(&settings);
    settings.heap = FindNexusHeap(NEXUS_MemoryType_eFull);
    if (settings.heap != 0) {
        settings.alignment = 64;
        NEXUS_Error Status = NEXUS_Memory_Allocate(bytes, &settings, (void **) &pBuffer);
        BME_DEBUG_TRACE(("%s: h %p s %d b %p\n", __FUNCTION__, settings.heap, Status, pBuffer));
        if (Status == NEXUS_SUCCESS && pBuffer) {
            bAllocate = true;
        }
    }
    if (!bAllocate) {
        pBuffer = (uint8_t *)malloc(bytes * sizeof(uint8_t));
    }

    if (!pBuffer) {
        BME_DEBUG_ERROR(("%s:%d failed to allocate buffer.", __FUNCTION__, __LINE__));
        return NULL;
    }
    BKNI_Memset(pBuffer, 0, bytes);
    BME_DEBUG_EXIT();
    return pBuffer;
}

void WavFormatHeader::FreeBuffer(uint8_t * buffer) {
    if (buffer) {
        if (NEXUS_GetAddrType((const void *)buffer) != NEXUS_AddrType_eUnknown) {
            NEXUS_Memory_Free(buffer);
        } else {
            free(buffer);
        }
    }
}

void WavFormatHeader::SetPayloadSize(uint32_t size)
{
    if (!wavHeader)
        return;

    B_MEDIA_SAVE_UINT32_BE(wavHeader + bmedia_frame_bcma.len, size);
}

bool WavFormatHeader::AddExtendHeader(const uint8_t *data, const uint32_t size) {
    BME_DEBUG_ENTER();

     if (codec != IMedia::VorbisAudioCodec &&
         codec != IMedia::OpusAudioCodec) {
        return false;
    }

    uint8_t* pData = (uint8_t*)data;
    uint32_t idHdrSize = 0;
    uint32_t commentHdrSize = 0;
    uint32_t setupHdrSize = 0;

    uint8_t* pIdHdr = NULL;
    uint8_t* pCommentHdr = NULL;
    uint8_t* pSetupHdr = NULL;

    FreeBuffer(extHeader);
    extHeader = NULL;
    extHeaderSize = 0;

    if (codec == IMedia::VorbisAudioCodec) {
        idHdrSize = data[1];
        commentHdrSize = data[2];
        setupHdrSize = size - idHdrSize - commentHdrSize - 3;

        pIdHdr = pData + 3;
        pCommentHdr = pData + idHdrSize + 3;
        pSetupHdr = pData + idHdrSize + commentHdrSize + 3;

        if ((pIdHdr[0] != 0x01) || (pCommentHdr[0] != 0x03) || (pSetupHdr[0] != 0x05)) {
            BME_DEBUG_ERROR(("Invalid private headers"));
            return false;
        }
        extHeaderSize = 2 + idHdrSize + 2 + commentHdrSize + 2 + setupHdrSize;
    } else {
        setupHdrSize = size;
        pSetupHdr = pData;
        extHeaderSize = setupHdrSize;
    }

    extHeader =AllocateBuffer(extHeaderSize);
    if (!extHeader) {
        extHeaderSize = 0;
        return false;
    }

    /* Update Vorbis Wav header cbSize */
    uint8_t *pWavHdr = (uint8_t*)(wavHeader);
    pWavHdr +=8;
    B_MEDIA_SAVE_UINT16_LE(pWavHdr + 16, extHeaderSize);

    pWavHdr = (uint8_t*)(extHeader);
    // ID buffer
    if (idHdrSize) {
        B_MEDIA_SAVE_UINT16_BE(pWavHdr, idHdrSize);
        BKNI_Memcpy(pWavHdr + 2, pIdHdr, idHdrSize);
        pWavHdr += (2 + idHdrSize);
    }

    //Comment buffer
    if (commentHdrSize) {
        B_MEDIA_SAVE_UINT16_BE(pWavHdr, commentHdrSize);
        BKNI_Memcpy(pWavHdr + 2, pCommentHdr, commentHdrSize);
        pWavHdr += (2 + commentHdrSize);
    }

    // Setup Buffer
    if (setupHdrSize) {
        if (codec == IMedia::VorbisAudioCodec) {
            B_MEDIA_SAVE_UINT16_BE(pWavHdr, setupHdrSize);
            pWavHdr += 2;
        }
        BKNI_Memcpy(pWavHdr, pSetupHdr, setupHdrSize);
    }

    BME_DEBUG_EXIT();
    return true;
}
}  // namespace Media
}  // namespace Broadcom

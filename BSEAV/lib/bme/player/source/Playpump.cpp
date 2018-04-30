/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/

#include <mutex>

#include <string.h> // memcpy
#include <assert.h> // assert
#include <csignal>  // std::rasie

#include "Debug.h"

#include "nexus_platform_client.h" // NEXUS_Platform_GetClientConfiguration()

#include "MediaDrmContext.h"
#include "WavFormatHeader.h"
#include "Playpump.h"
#include "bmedia_util.h" // BPP, PES utils

#include "nexus_memory.h"

#undef BME_DEBUG_ERROR
#define BME_DEBUG_ERROR(a)


TRLS_DBG_MODULE(Playpump);


// ===================================================================

static NEXUS_HeapHandle findHeap(NEXUS_MemoryType access)
{
    NEXUS_ClientConfiguration PlatformConfig;
    NEXUS_Platform_GetClientConfiguration(&PlatformConfig);
    for (size_t i = 0; i < sizeof(PlatformConfig.heap) / sizeof(PlatformConfig.heap[0]); ++i)
        if (PlatformConfig.heap[i]) {
            NEXUS_MemoryStatus HeapStatus;
            NEXUS_Heap_GetStatus(PlatformConfig.heap[i], &HeapStatus);
            if ((HeapStatus.memoryType & access) == access)
                return PlatformConfig.heap[i];
        }
    return 0;
}

namespace Broadcom
{
namespace Media
{

bool DmaBuffer_t::allocate(size_t bytes)
{
    NEXUS_MemoryAllocationSettings settings;
    NEXUS_Memory_GetDefaultAllocationSettings(&settings);
    settings.heap = findHeap(NEXUS_MemoryType_eFull);
    if (settings.heap == 0)
        return false;
    settings.alignment = 64;
    NEXUS_Error status = NEXUS_Memory_Allocate(bytes, &settings, (void **) &buffer);
    BME_DEBUG_TRACE(("%s: h %p s %d b %p\n", __FUNCTION__, settings.heap, status, buffer));
    BME_CHECK(status);
    if (status == NEXUS_SUCCESS)
        allocated = bytes;
    return status == NEXUS_SUCCESS;
}
void DmaBuffer_t::free()
{
    if (allocated == 0)
        return;
    NEXUS_Memory_Free(buffer);
    allocated = 0;
}

void DmaBuffer_t::flush() const
{
    if (head != 0)
        NEXUS_Memory_FlushCache(buffer, head);
}

size_t DmaBuffer_t::addFragment(const void *buffer, size_t bytes)
{
    // Take what will fit
    bytes = std::min(bytes, allocated - head);
    if (bytes == 0)
        return 0;

    memcpy(this->buffer + head, buffer, bytes);
    head += bytes;
    return bytes;
}
const void *DmaBuffer_t::removeFragment(size_t bytes)
{
    assert(bytes <= this->bytes());
    const void *start = reinterpret_cast<uint8_t *>(buffer) + tail;
    tail += bytes;
    return start;
}


// ===================================================================

static const size_t maxDmaFragments = 64;

static void staticEventSet(void *context, int parameter)
{
    (void) parameter;
    BKNI_SetEvent(reinterpret_cast<BKNI_EventHandle>(context));
}

static void staticDataCallback(void *context, int parameter)
{
    Playpump_t *instance = reinterpret_cast<Playpump_t *>(context);
    (void) parameter;
    BKNI_SetEvent(instance->event);
    instance->onData();
}

static void staticErrorCallback(void *context, int parameter)
{
    (void) parameter;
    reinterpret_cast<Playpump_t *>(context)->onError();
}

void Playpump_t::onError()
{
}

void Playpump_t::onData()
{
}

bool Playpump_t::initialise(bool secure, size_t bytes, size_t dmaBytes)
{
    initialised = 0;

    BKNI_CreateEvent(&event);
    BKNI_CreateEvent(&dma.complete);
    BKNI_CreateMutex(&lock);
    secureBuffer = 0;

    // These will throw an exception on failure
    dma.fragment.dma.reserve(32);
    dma.fragment.decrypt.reserve(16);

    // Header initialisation
    memcpy(sample.bcmvHeader, bmedia_frame_bcmv.base, bmedia_frame_bcmv.len);
    assert(bmedia_frame_bcmv.len == 4);

    dma.handle = NEXUS_Dma_Open(NEXUS_ANY_ID, NULL);
    if (dma.handle == 0) {
      BME_DEBUG_ERROR(("Unable to create DMA job"));
      deinitialise();
      return false;
    }
    ++initialised;

    NEXUS_DmaJobSettings dmaJobSettings;
    NEXUS_DmaJob_GetDefaultSettings(&dmaJobSettings);
    dmaJobSettings.numBlocks                   = maxDmaFragments;
#ifdef BRCM_SAGE
    dmaJobSettings.bypassKeySlot               = secure ? NEXUS_BypassKeySlot_eGR2R : NEXUS_BypassKeySlot_eG2GR;
#else
    dmaJobSettings.bypassKeySlot               = NEXUS_BypassKeySlot_eG2GR;
#endif
    dmaJobSettings.dataFormat                  = NEXUS_DmaDataFormat_eBlock;
    dmaJobSettings.completionCallback.callback = staticEventSet;
    dmaJobSettings.completionCallback.context  = dma.complete;
    dma.job = NEXUS_DmaJob_Create(dma.handle, &dmaJobSettings);
    if (dma.job == 0) {
        BME_DEBUG_ERROR(("Unable to create DMA job"));
        deinitialise();
        return false;
    }
    ++initialised;

    // Open the playpump
    NEXUS_PlaypumpOpenSettings openSettings;
    NEXUS_Playpump_GetDefaultOpenSettings(&openSettings);
    openSettings.fifoSize = bytes;
#if defined(BRCM_SAGE)
    if (secure) {
        NEXUS_MemoryAllocationSettings settings;
        NEXUS_Memory_GetDefaultAllocationSettings(&settings);
        settings.heap      = findHeap(NEXUS_MemoryType_eSecure);
        settings.alignment = 64;
        NEXUS_Error status = NEXUS_Memory_Allocate(bytes, &settings, (void **) &secureBuffer);
        BME_DEBUG_TRACE(("%s: h %p s %d b %p\n", __FUNCTION__, settings.heap, status, secureBuffer));
        BME_CHECK(status);
        if (secureBuffer == 0) {
            BME_DEBUG_ERROR(("Unable to allocate secure buffer"));
            deinitialise();
            return false;
        }

        openSettings.dataNotCpuAccessible = true;
        openSettings.memory = NEXUS_MemoryBlock_FromAddress(secureBuffer);
    }
#endif
    ++initialised;

    if (dmaBytes != 0 && !dmaBuffer.allocate(dmaBytes)) {
        BME_DEBUG_ERROR(("Unable to allocate DMA buffer"));
        deinitialise();
        return false;
    }
    ++initialised;

    handle = NEXUS_Playpump_Open(NEXUS_ANY_ID, &openSettings);
    if (handle == 0) {
        BME_DEBUG_ERROR(("Unable to Open playpump"));
        deinitialise();
        return false;
    }
    ++initialised;

    // Configure
    NEXUS_PlaypumpStatus playpumpStatus;
    BME_CHECK(NEXUS_Playpump_GetStatus(handle, &playpumpStatus));
    end = reinterpret_cast<const uint8_t *>(playpumpStatus.bufferBase) + playpumpStatus.fifoSize;

    NEXUS_PlaypumpSettings settings;
    NEXUS_Playpump_GetSettings(handle, &settings);
    settings.transportType          = NEXUS_TransportType_eMpeg2Pes;
    settings.dataCallback.callback  = staticDataCallback;
    settings.dataCallback.context   = reinterpret_cast<void *>(this);
    settings.errorCallback.callback = staticErrorCallback;
    settings.errorCallback.context  = reinterpret_cast<void *>(this);
    NEXUS_Playpump_SetSettings(handle, &settings);
    NEXUS_Playpump_Start(handle);

    return true;
}

void Playpump_t::deinitialise()
{
    switch (initialised)
    {
    case 5:
        NEXUS_Playpump_Stop(handle);
        NEXUS_Playpump_Close(handle);
    case 4:
        dmaBuffer.free();
    case 3:
#if defined(BRCM_SAGE)
        if (secureBuffer != 0)
            NEXUS_Memory_Free(secureBuffer);
#endif
    case 2:
        NEXUS_DmaJob_Destroy(dma.job);
    case 1:
        NEXUS_Dma_Close(dma.handle);
    case 0:
        BKNI_DestroyMutex(lock);
        BKNI_DestroyEvent(dma.complete);
        BKNI_DestroyEvent(event);
        break;
    }
}


// ===================================================================

void Playpump_t::setPid(uint16_t pid)
{
    bmedia_bpp_pkt bpp;
    BKNI_Memset(&bpp, 0, sizeof(bpp));
    bpp.data[0] = 0x0a; // Flush
    bmedia_make_bpp_pkt(pid, &bpp, sample.flush,      sizeof(sample.flush));

    bpp.data[0] = 0x82; // Last
    bmedia_make_bpp_pkt(pid, &bpp, sample.last,       sizeof(sample.last));

    bpp.data[0] = 0x85; // End of chunk
    bmedia_make_bpp_pkt(pid, &bpp, sample.endOfChunk, sizeof(sample.endOfChunk));
}


// ===================================================================

void Playpump_t::acquireLock()
{
    flush = true;
    BKNI_SetEvent(event);
    BKNI_AcquireMutex(lock);
    flush = false;
}


// ===================================================================

#define PES_HEADER_NO_PTS 9
#define PES_HEADER_PTS 14
#define PES_MAX_SIZE 65535

static size_t makePes(
    uint8_t *buffer, size_t payloadBytes, size_t &pesBytesLeft,
    unsigned int id, TIME45k pts, bool singlePes)
{
    bmedia_pes_info pesInfo;
    bmedia_pes_info_init(&pesInfo, id);
    size_t maxPesBytesLeft = PES_MAX_SIZE - (PES_HEADER_NO_PTS - 6);
    if (pts != ~0ULL) {
        pesInfo.pts_valid = true;
        pesInfo.pts = (uint32_t) pts;
        maxPesBytesLeft -= PES_HEADER_PTS - PES_HEADER_NO_PTS;
        BME_DEBUG_TRACE(("%s: PTS %16llu\n", __FUNCTION__, pts));
    }
    pesBytesLeft = singlePes ? payloadBytes : std::min(payloadBytes, maxPesBytesLeft);
    return bmedia_pes_header_init(buffer, pesBytesLeft, &pesInfo);
}

void Playpump_t::addPes(unsigned int id, TIME45k pts, bool singlePes)
{
    // Ensure that we have enough space for all additional fragments
    sample.pesHeaders.clear();
    sample.pesHeaders.reserve(PES_HEADER_PTS + (sample.bytes >> 15) * PES_HEADER_NO_PTS);

    // Insert the first PES header
    sample.pesHeaders.resize(PES_HEADER_PTS);
    uint8_t *pesHeader = sample.pesHeaders.data();
    size_t pesBytesLeft;
    size_t offset = makePes(pesHeader, sample.bytes, pesBytesLeft, id, pts, singlePes);

    // Prepend the first (with PTS) PES header
    DataFragment_t pesFragment(pesHeader, offset);
    Sample_t::FragmentList_t::iterator iterator = sample.fragmentList.begin();
    iterator = sample.fragmentList.insert(iterator, pesFragment);
    ++iterator;

    size_t fragmentOffset = 0;
    for (size_t sampleBytes = sample.bytes; sampleBytes != 0;) {
        if (pesBytesLeft == 0) {
            // Add another PES header (no PTS)
            sample.pesHeaders.resize(offset + PES_HEADER_NO_PTS);
            pesFragment.data  = pesHeader + offset;
            pesFragment.bytes = makePes(pesHeader + offset, sampleBytes, pesBytesLeft, id, ~0ULL, false);
            offset += pesFragment.bytes;
            iterator = sample.fragmentList.insert(iterator, pesFragment);
            ++iterator;
        }

        // Skip fragments up to pesBytesLeft
        for (
            size_t FragmentBytes;
            sampleBytes != 0 && (FragmentBytes = iterator->bytes - fragmentOffset, FragmentBytes <= pesBytesLeft);
            sampleBytes -= FragmentBytes, pesBytesLeft -= FragmentBytes, fragmentOffset = 0, ++iterator)  {
            FragmentBytes = iterator->bytes - fragmentOffset;
            if (FragmentBytes > pesBytesLeft)
                break;
        }

        if (sampleBytes != 0 && pesBytesLeft != 0) {
            // Need to split the fragment to get the first pesBytesLeft
            const uint8_t *Data = reinterpret_cast<const uint8_t *>(iterator->data) + fragmentOffset;
            DataFragment_t FragmentStart(Data, pesBytesLeft, iterator->encrypted);
            iterator = sample.fragmentList.insert(iterator, FragmentStart);
            ++iterator;
            size_t Consumed = fragmentOffset + pesBytesLeft;
            iterator->data   = Data + pesBytesLeft;
            iterator->bytes -= Consumed;
            sampleBytes     -= Consumed;
            fragmentOffset = pesBytesLeft = 0;
        }
    }
    sample.bytes += offset;
}


// ===================================================================

// Copy the fragments and count the fragment bytes
size_t Playpump_t::copyCountFragments(const DataFragment_t *fragment, size_t n)
{
    size_t sampleBytes = 0;
    for (size_t i = 0; i < n; ++i) {
        sample.fragmentList.push_back(fragment[i]);
        sampleBytes += fragment[i].bytes;
    }
    return sampleBytes;
}


// ===================================================================

void Playpump_t::makeVp9Chunk(unsigned int id, TIME45k pts, const DataFragment_t *fragment, size_t n)
{
    uint8_t *bcmvHeader = sample.bcmvHeader;

    sample.clear();

    DataFragment_t bcmvFragment(sample.bcmvHeader, sizeof(sample.bcmvHeader));
    sample.fragmentList.push_back(bcmvFragment);

    // Append the fragments and calculate the total fragment bytes
    sample.bytes = bcmvFragment.bytes + copyCountFragments(fragment, n);
    B_MEDIA_SAVE_UINT32_BE(&bcmvHeader[4], sample.bytes);
    B_MEDIA_SAVE_UINT16_BE(&bcmvHeader[8], 1);

    // Add PES header(s)
    addPes(id, pts, false);

    // Add BPP packet
    DataFragment_t bppFragment(sample.endOfChunk, sizeof(sample.endOfChunk));
    sample.fragmentList.push_back(bppFragment);
    sample.bytes += bppFragment.bytes;
}


// ===================================================================

void Playpump_t::makePesChunk(unsigned int id, TIME45k pts, const DataFragment_t *fragment, size_t n, bool singlePes)
{
    // Copy the fragments and count the fragment bytes
    sample.clear();
    sample.bytes = copyCountFragments(fragment, n);

    // Add PES header(s)
    addPes(id, pts, singlePes);
}

void Playpump_t::makePcmChunk(WavFormatHeader *header, unsigned int id, TIME45k pts, const DataFragment_t *fragment, size_t n)
{
    // Count the fragment bytes and copy the fragments
    sample.clear();
    size_t sampleBytes = copyCountFragments(fragment, n);

    // Prepend the WAV header
    header->SetPayloadSize(sampleBytes);
    DataFragment_t waveHeader(header->GetWavHeader(), header->GetWavHeaderSize());
    sample.fragmentList.insert(sample.fragmentList.begin(), waveHeader);

    sample.bytes = sampleBytes + waveHeader.bytes;

    // Add PES header(s)
    addPes(id, pts, false);
}

void Playpump_t::makeVorbisChunk(WavFormatHeader *header, unsigned int id, TIME45k pts, const DataFragment_t *fragment, size_t n)
{
    // Count the fragment bytes and copy the fragments
    sample.clear();
    size_t sampleBytes = copyCountFragments(fragment, n);

    // Prepend the WAV header
    header->SetPayloadSize(sampleBytes);
    DataFragment_t waveHeader(header->GetWavHeader(), header->GetWavHeaderSize());
    DataFragment_t waveExtendedHeader(header->GetExtHeader(), header->GetExtHeaderSize());
    Sample_t::FragmentList_t::iterator start = sample.fragmentList.begin();
    if (waveExtendedHeader.bytes != 0)
        start = sample.fragmentList.insert(start, waveExtendedHeader);
    start = sample.fragmentList.insert(start, waveHeader);
    sample.bytes = sampleBytes + waveHeader.bytes + waveExtendedHeader.bytes;

    // Add PES header(s)
    addPes(id, pts, false);
}


// ===================================================================

static unsigned int sampleRateToAdts(unsigned int sampleRate)
{
  switch (sampleRate)
  {
  case 96000: return 0;
  case 88200: return 1;
  case 64000: return 2;
  case 48000: return 3;
  case 44100: return 4;
  case 32000: return 5;
  case 24000: return 6;
  case 22050: return 7;
  case 16000: return 8;
  case 12000: return 9;
  case 11025: return 10;
  case  8000: return 11;
  case  7350: return 12;
  default:     break;
  }
  return 4; // Default to 44k1
}

static unsigned int channelsToAdts(unsigned int channels)
{
  switch (channels)
  {
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 8:
    return channels;

  default:
    break;
  }
  return 0; // Default to 'Defined in AOT Specific Config'
}

void Playpump_t::makeAdtsChunk(
    unsigned int sampleRate, unsigned int channels, unsigned int id,
    TIME45k pts, const DataFragment_t *fragment, size_t n)
{
    // Count the fragment bytes and copy the fragments
    sample.clear();
    size_t sampleBytes = copyCountFragments(fragment, n);

    // AAC may be delivered raw, i.e. without any framing
    // We only support framed AAC, so add ADTS headers here
    // See http://wiki.multimedia.cx/index.php?title=ADTS
    unsigned int objectEnum     = 2; // 1: AAC Main, 2: AAC LC
    unsigned int sampleRateEnum = sampleRateToAdts(sampleRate);
    unsigned int channelEnum    = channelsToAdts(channels);
    unsigned int frames         = 1;
    uint8_t     *adtsHeader     = sample.adtsHeader;
    size_t       frameBytes     = sampleBytes + sizeof(sample.adtsHeader);
    adtsHeader[0] = 0xff;
    adtsHeader[1] = 0xf0 | (0 << 3) | (1 << 0);
    adtsHeader[2] = ((objectEnum - 1) << 6) | (sampleRateEnum << 2) | (channelEnum >> 2 << 0);
    adtsHeader[3] = ((channelEnum & 3) << 6) | (frameBytes >> 11 << 0);
    adtsHeader[4] = (frameBytes >> 3) & 0xff;
    adtsHeader[5] = ((frameBytes & 7) << 5);
    adtsHeader[6] = (frames - 1) << 0;

    // Prepend the ADTS header
    DataFragment_t adtsFragment(adtsHeader, sizeof(sample.adtsHeader));
    sample.fragmentList.insert(sample.fragmentList.begin(), adtsFragment);

    sample.bytes = sampleBytes + adtsFragment.bytes;

    // Add PES header(s)
    addPes(id, pts, false);
}


// ===================================================================

bool Playpump_t::getPlaypumpSpace(size_t bytes) const volatile
{
    NEXUS_PlaypumpStatus playpumpStatus;
    NEXUS_Error nexusStatus = NEXUS_Playpump_GetStatus(handle, &playpumpStatus);
    BME_DEBUG_TRACE(("%s(%p): +%6x space +%6x\n", __FUNCTION__, this,
        (unsigned int) bytes, (unsigned int) playpumpStatus.fifoSize - playpumpStatus.fifoDepth));
    return bytes <= playpumpStatus.fifoSize - playpumpStatus.fifoDepth
        && playpumpStatus.descFifoSize != playpumpStatus.descFifoDepth
        && nexusStatus == NEXUS_SUCCESS;
}

void Playpump_t::getPlaypumpBuffer(void *&buffer, size_t &bytes, bool block)
{
    if (flush) {
        BME_DEBUG_PRINT(("%s(%p): Entered with flush\n", __FUNCTION__, this));
        bytes = 0;
        return;
    }

    NEXUS_Error status;
    while (status = NEXUS_Playpump_GetBuffer(handle, &buffer, &bytes), !flush && bytes == 0) {
        BME_CHECK(status);
        if (!block)
            return;
        BKNI_WaitForEvent(event, BKNI_INFINITE);
    }

    if (flush) {
        BME_DEBUG_PRINT(("%s(%p): Dropping data on flush (Bytes %u)\n", __FUNCTION__, this, (unsigned int) bytes));
        if (bytes != 0) {
            // Drop any buffer
            BME_CHECK(NEXUS_Playpump_WriteComplete(handle, bytes, 0));
            bytes = 0;
        }
    }
}


// ===================================================================

#include "nexus_base_mmap.h"

static bool MustCopyFragment(const Broadcom::Media::DataFragment_t *Instance)
{
    return Instance->data == 0 || NEXUS_GetAddrType((const void *) Instance->data) == NEXUS_AddrType_eUnknown;
}

static bool extendFragment(NEXUS_DmaJobBlockSettings *fragment, const void *destination, const void *source, size_t bytes)
{
    const uint8_t *sourceEnd      = reinterpret_cast<const uint8_t *>(fragment->pSrcAddr)  + fragment->blockSize;
    const uint8_t *destinationEnd = reinterpret_cast<const uint8_t *>(fragment->pDestAddr) + fragment->blockSize;
    if (source != sourceEnd || destination != destinationEnd)
        return false;

    fragment->blockSize += bytes;
    return true;
}

void Playpump_t::clearDma(const NEXUS_DmaJobBlockSettings *descriptor, size_t n)
{
    size_t send;
    for (; n != 0; n -= send, descriptor += send) {
        send = std::min(n, maxDmaFragments);
        NEXUS_Error status =
            NEXUS_DmaJob_ProcessBlocks(dma.job, descriptor, n);
        if (status != NEXUS_DMA_QUEUED) {
            BKNI_SetEvent(dma.complete);
            BME_CHECK(status);
        }

        // Wait for the event if we're going to do multiple sends,
        // otherwise we could think we've completed whne the first
        // DMA completes
        if (n != send)
            BKNI_WaitForEvent(dma.complete, BKNI_INFINITE);
    }
}

// Works-around DRM limitaitons:
// - Requiring the same source and destination buffer layout
// - Not able do decrypt partial samples
void Playpump_t::decryptWorkaround(void *drmContext, uint64_t vector, bool block)
{
    MediaDrmContext *context = static_cast<MediaDrmContext *>(drmContext);

    // We always decrypt in-place here, which serves two purposes:
    // - Required if !canChangeLayout()
    // - Frees-up the dmaBuffer for use when !context->canDecryptPartial()

    // DMA the encrypted fragments into the playpump buffer
    clearDma(dma.fragment.decrypt.data(), dma.fragment.decrypt.size());
    // Don't wait for the DMA to complete here, as it's queued before the decryption

    // 1. Update the descriptors source to be the playpump buffer
    NEXUS_DmaJobBlockSettings *descriptor = dma.fragment.decrypt.data();
    for (size_t i = 0; i < dma.fragment.decrypt.size(); ++i)
        descriptor[i].pSrcAddr = descriptor[i].pDestAddr;

    // 2. Decrypt
    if (!context->canDecryptPartial() && sample.offset != 0) {
        // We need to insert sample.offset bytes of dummy data
        // - inefficient, but required as a fall-back
        size_t sent;
        for (size_t offset = sample.offset; offset != 0; offset -= sent) {
            sent = std::min(offset, dmaBuffer.allocated);
            NEXUS_DmaJobBlockSettings pad;
            NEXUS_DmaJob_GetDefaultBlockSettings(&pad);
            pad.cached    = false;
            pad.pSrcAddr  = dmaBuffer.buffer;
            pad.pDestAddr = dmaBuffer.buffer;
            pad.blockSize = sent;
            dma.fragment.decrypt.insert(dma.fragment.decrypt.begin(), pad);
        }

        context->decrypt(dma.fragment.decrypt.data(), dma.fragment.decrypt.size(), 0,             vector);
    } else {
        context->decrypt(dma.fragment.decrypt.data(), dma.fragment.decrypt.size(), sample.offset, vector);
    }

    // 3. Wait for the clear DMA to complete - will already have done so
    BKNI_WaitForEvent(dma.complete, BKNI_INFINITE);
}


bool Playpump_t::pushChunk(
    void                 *drmContext,
    uint64_t              vector,
    bool                  block)
{
    MediaDrmContext *context         = static_cast<MediaDrmContext *>(drmContext);
    DataFragment_t  *currentFragment = sample.fragmentList.data();

    while (sample.fragmentList.size() != 0) {
        // Get a playpump buffer
        uint8_t *buffer;
        size_t   bytes;
        getPlaypumpBuffer(reinterpret_cast<void *&>(buffer), bytes, block);
#ifdef TEST_MULTIPLE_FRAGMENTS
        bytes = std::min(bytes, (size_t) 0x1000);
#endif
        if (bytes == 0) {
            // Stopping or can't block
            BME_DEBUG_TRACE(("%s: Exiting\n", __FUNCTION__));
            return false;
        }

        // Form block lists for copying into the playpump buffer
        // and for decrypting the encrypted regions in-place
        dma.fragment.dma.clear();
        dma.fragment.decrypt.clear();

        bool   copy;
        size_t copied, totalCopied = 0;
        size_t encryptedBytes = 0;

        // Loop until the playpump buffer is full or we have completed the input fragments
        for (; bytes != 0 && sample.fragmentList.size() != 0;
            buffer += copied, bytes -= copied, totalCopied += copied) {
            // Should we copy the fragment? We must if it is not on a Nexus heap, and could
            // to minimise the number of fragments, but don't
            copy = MustCopyFragment(currentFragment);
            copied = std::min(currentFragment->bytes, bytes);
            if (copied == 0)
                std::raise(SIGABRT);  // This is a programming bug so crash intentionally
            const void *source;
            if (copy) {
                copied = dmaBuffer.addFragment(
                    reinterpret_cast<const uint8_t *>(currentFragment->data), copied);
                if (copied == 0)
                    break; // No more space available in the dmaBuffer
                source = dmaBuffer.removeFragment(copied);
            } else {
                source = reinterpret_cast<const uint8_t *>(currentFragment->data);
                NEXUS_Memory_FlushCache(const_cast<void *>(source), copied); // Ensure that external memory is correct
            }

            // Update the DMA fragment list
            if (!currentFragment->encrypted) {
                if (dma.fragment.dma.empty() || !extendFragment(&dma.fragment.dma.back(), buffer, source, copied)) {
                    // Can't extend the current fragment, so add a new one
                    NEXUS_DmaJobBlockSettings next;
                    NEXUS_DmaJob_GetDefaultBlockSettings(&next);
                    next.cached    = false;
                    next.pSrcAddr  = source;
                    next.pDestAddr = buffer;
                    next.blockSize = copied;
                    dma.fragment.dma.push_back(next);
                }
            } else {
                // Update the decrypt fragment list
                NEXUS_DmaJobBlockSettings next;
                NEXUS_DmaJob_GetDefaultBlockSettings(&next);
                next.cached    = false;
                next.pSrcAddr  = source;
                next.pDestAddr = buffer;
                next.blockSize = copied;
                dma.fragment.decrypt.push_back(next);
                encryptedBytes += copied;
            }

            // Update our fragment position
            sample.bytes -= copied;
            if (copied == currentFragment->bytes) {
                sample.fragmentList.erase(sample.fragmentList.begin());
                currentFragment = sample.fragmentList.data();
            } else {
                currentFragment->data   = reinterpret_cast<const uint8_t *>(currentFragment->data) + copied;
                currentFragment->bytes -= copied;
            }
        }

        // We have written DMA descriptors for as much as we can, so DMA / decrypt it
        // (We may run-out of space because:
        // - copied fragments have filled dmaBuffer
        // - Playpump buffer full
        // We cannot, therefore, guarantee to have all the data in the DMA descriptors)

        // Flush the DMA buffer to ensure that external memory is up to date
        dmaBuffer.flush();

        // DMA the fragments
        // We only start the DMA here, so that decryption can overlap it
        size_t dmaFragments = dma.fragment.dma.size();
        if (dmaFragments != 0)
            clearDma(dma.fragment.dma.data(), dmaFragments);

        size_t decryptFragments = dma.fragment.decrypt.size();
        if (decryptFragments != 0 && drmContext != 0) {
            dma.fragment.decrypt.front().resetCrypto              = true;
            dma.fragment.decrypt.front().scatterGatherCryptoStart = true;
            dma.fragment.decrypt.back().scatterGatherCryptoEnd    = true;

            // Some DRM implementations may have limitaitons that require work-arounds
            // Specifically, we require arbitrary source and destination layout and the
            // ability to decrypt from an offset within an encrypted sample
            // See decryptWorkaround() for work-around implementation details
            if (context->canChangeLayout()
                && (sample.offset == 0 || context->canDecryptPartial())) {
                context->decrypt(dma.fragment.decrypt.data(), decryptFragments, sample.offset, vector);
            } else {
                if (dmaFragments != 0) {
                    // Wait for any clear DMA to complete
                    BKNI_WaitForEvent(dma.complete, BKNI_INFINITE);
                    dmaFragments = 0; // Prevent another wait
                }
                decryptWorkaround(drmContext, vector, block);
            }
        }
        sample.offset += encryptedBytes;

        // Wait for any clear DMA to complete
        if (dmaFragments != 0)
            BKNI_WaitForEvent(dma.complete, BKNI_INFINITE);

        // All data in the playpump buffer is now clear (i.e. unencrypted)
        BME_CHECK(NEXUS_Playpump_WriteComplete(handle, 0, totalCopied));

        // Only reset dmaBuffer if there is no data left in it
        // Data will remain if the datapump buffer was not large enough, which
        // typically only occurs when we wrap (it's a circular buffer)
        if (dmaBuffer.bytes() == 0)
            dmaBuffer.reset();
    }
    return true;
}

void Playpump_t::pushClearChunk()
{
    if (secureBuffer != 0) {
        // Must use DMA
        pushChunk(0, 0, true);
        return;
    }

    const DataFragment_t *fragment        = sample.fragmentList.data();
    const DataFragment_t *currentFragment = &fragment[0];
    size_t                n               = sample.fragmentList.size();
    int fragmentIndex = 0;
    for (size_t fragmentOffset = 0; fragmentIndex < (int) n;) {
        // Get a playpump buffer
        uint8_t *buffer;
        size_t   bytes = 1; // Minimum bytes required
        getPlaypumpBuffer(reinterpret_cast<void *&>(buffer), bytes, true);
#ifdef TEST_MULTIPLE_FRAGMENTS
        bytes = std::min(bytes, (size_t) 0x1000);
#endif
        if (bytes == 0) {
            // Stopping - drop the chunk
            BME_DEBUG_TRACE(("%s: Exiting\n", __FUNCTION__));
            return;
        }
        const void *bufferStart = reinterpret_cast<void *>(buffer); // For later cache flushing

        // Loop until the playpump buffer is full or we have completed the input fragments
        size_t copied, totalCopied = 0;
        for (; bytes != 0 && fragmentIndex < (int) n;
            buffer += copied, bytes -= copied, totalCopied += copied) {
            // How much of currentFragment can we copy?
            TRLS_ASSERT(!currentFragment->encrypted);
            copied = std::min(currentFragment->bytes - fragmentOffset, bytes);
            const void *source = reinterpret_cast<const uint8_t *>(currentFragment->data) + fragmentOffset;

            // Copy the data
            memcpy(buffer, source, copied);

            // Update our fragment position
            fragmentOffset += copied;
            if (fragmentOffset == currentFragment->bytes) {
                ++fragmentIndex;
                ++currentFragment;
                fragmentOffset = 0;
            }
        }

        NEXUS_Memory_FlushCache(bufferStart, totalCopied);
        BME_CHECK(NEXUS_Playpump_WriteComplete(handle, 0, totalCopied));
    }
}


// ===================================================================

} // namespace Media
} // namespace Broadcom

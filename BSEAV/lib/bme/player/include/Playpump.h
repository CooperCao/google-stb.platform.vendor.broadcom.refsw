/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/

#ifndef BME_PLAYPUMP_H_
#define BME_PLAYPUMP_H_

#include <vector>

#include "bstd.h"
#include <stdint.h> // stdint must be included after bstd(_defs)
#include <stddef.h> // size_t

#include "bkni.h"
#include "bkni_multi.h"
#include "nexus_playpump.h"
#include "nexus_dma.h"

#include "bmedia_util.h" // B_MPEG2PES_BPP_LENGTH

#include "DataFragment.h" // DataFragment_t
#include "Media.h" // TIME45k
#include "Debug.h"


namespace Broadcom
{
namespace Media
{

class WavFormatHeader;

struct BME_SO_EXPORT DmaBuffer_t
{
    // A Nexus buffer to copy the stream to before DMAing to the playpump
    // This is necessary as Nexus cannot DMA from Linux-allocated memory
    uint8_t *buffer;
    size_t   allocated; // Bytes allocated
    size_t   head;      // Data head, advances as we copy-in
    size_t   tail;      // Data tail, advances as we copy-out

    DmaBuffer_t() : allocated(0)
    {
        reset();
    }
    ~DmaBuffer_t()
    {
        free();
    }

    bool allocate(size_t Bytes);
    void free();

    void reset()
    {
        tail = head = 0;
    }

    size_t bytes() const
    {
        return head - tail;
    }

    void flush() const;

    size_t addFragment(const void *buffer, size_t bytes);
    const void *removeFragment(size_t bytes);
};

struct BME_SO_EXPORT Playpump_t
{
    unsigned int initialised;

    DmaBuffer_t  dmaBuffer;

    // Prevent simultaneous control operations
    // We cannot flush a playpump between getting a buffer and calling write complete,
    // as the flush implicitly returns the buffer. This lock prevents that from happening
    // EoS can cause issues too, as Media Player automatically does a stop()
    // It also prevents a shutdown whilst we're still pushing data
    // Audio and Video must have seperate locks, as they block for playpump space whilst
    // holding them - we attempt to run full, so expect to be blocked most of the time
    BKNI_MutexHandle lock;

    NEXUS_PlaypumpHandle handle;
    const uint8_t       *end;
    BKNI_EventHandle     event;
    uint8_t             *secureBuffer;

    // When we flush we have to abort any inprogress data pushes
    // we set this flag and unblock any event waits, acquire the stream lock,
    // then reset the flag
    volatile bool        flush;

    // During some operations (stop, flush), we need to ensure that the producer
    // is not blocked waiting for a playpump buffer
    void acquireLock();

    struct
    {
#define DMABUFFER_BYTES (128 << 10)
        NEXUS_DmaHandle    handle;
        NEXUS_DmaJobHandle job;
        struct
        {
            std::vector<NEXUS_DmaJobBlockSettings> dma;
            std::vector<NEXUS_DmaJobBlockSettings> decrypt;
        } fragment;

        BKNI_EventHandle   complete;
    } dma;

    struct Sample_t
    {
        void clear()
        {
            fragmentList.clear();
            bytes = offset = 0;
        }

        typedef std::vector<DataFragment_t> FragmentList_t;
        FragmentList_t       fragmentList;
        size_t               offset; // Offset in the encrypted stream
        size_t               bytes;
        std::vector<uint8_t> pesHeaders;
        uint8_t              adtsHeader[7];
#define BCMV_HEADER_SIZE 10
        uint8_t              bcmvHeader[BCMV_HEADER_SIZE];
        uint8_t              flush[B_MPEG2PES_BPP_LENGTH];
        uint8_t              last[B_MPEG2PES_BPP_LENGTH];
        uint8_t              endOfChunk[B_MPEG2PES_BPP_LENGTH];
    } sample;

    Playpump_t() : handle(0), event(0), secureBuffer(0), flush(0)
    {
    }
    bool initialise(bool secure, size_t bytes, size_t dmaBytes = 512U << 10);
    void deinitialise();
    void setPid(uint16_t pid);

    size_t copyCountFragments(const DataFragment_t *fragment, size_t n);

    void makeVp9Chunk(unsigned int id, TIME45k pts, const DataFragment_t *fragment, size_t n);
    void makeAdtsChunk(
        unsigned int sampleRate, unsigned int channels, unsigned int id, TIME45k pts,
        const DataFragment_t *fragment, size_t n);
    void makePcmChunk(WavFormatHeader *header, unsigned int id, TIME45k pts,
        const DataFragment_t *fragment, size_t n);
    void makeVorbisChunk(WavFormatHeader *header, unsigned int id, TIME45k pts,
        const DataFragment_t *fragment, size_t n);
    void makePesChunk(
        unsigned int id, TIME45k pts,
        const DataFragment_t *fragment, size_t n, bool singlePes);

    void addPes(unsigned int id, TIME45k pts, bool singlePes);

    bool getPlaypumpSpace(size_t bytes) const volatile;
    void getPlaypumpBuffer(void *&buffer, size_t &bytes, bool block);

    // May block iff 'block'
    // Returns sent?
    bool pushChunk(
        void                 *drmContext,
        uint64_t              vector,
        bool                  block);

    // Pushes unencrypted chunks - optimised for non-secure playpumps
    // May block
    void pushClearChunk();

    // Layer to cope with DMA job limitations
    void clearDma(const NEXUS_DmaJobBlockSettings *descriptor, size_t n);
    void decryptWorkaround(
        void    *drmContext,
        uint64_t vector,
        bool     block);

    virtual void onError();
    virtual void onData();
};

} // namespace Media
} // namespace Broadcom


#endif /* ifndef BME_PLAYPUMP_H_ */

/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef __IWAVFORMATHEADERIMPL_H__
#define __IWAVFORMATHEADERIMPL_H__

#include "Media.h"

namespace Broadcom
{
namespace Media
{
class WavFormatHeader
{
public:
    WavFormatHeader(IMedia::AudioParameters param);
    ~WavFormatHeader();

    bool AddExtendHeader(const uint8_t *data, const uint32_t size);
    void SetPayloadSize(uint32_t size);

    uint8_t *GetWavHeader() const  {return wavHeader;}
    uint32_t GetWavHeaderSize() const {return wavHeaderSize;}
    uint8_t *GetExtHeader() const {return extHeader;}
    uint32_t GetExtHeaderSize() const  {return extHeaderSize;}

private:
    bool Initialize(IMedia::AudioParameters param);
    void Deinitialise();
    uint8_t* AllocateBuffer(uint32_t bytes);
    void FreeBuffer(uint8_t* buffer);

    uint8_t *wavHeader;
    uint8_t *extHeader;

    uint32_t wavHeaderSize;
    uint32_t extHeaderSize;
    IMedia::AudioCodec codec;
};
}  // namespace Media
}  // namespace Broadcom
#endif  // __IWAVFORMATHEADERIMPL_H__

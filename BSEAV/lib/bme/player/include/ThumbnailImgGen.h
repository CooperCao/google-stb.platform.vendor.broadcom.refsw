/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef COMPONENTS_MEDIASTREAMER_IMPL_THUMBNAILIMGGEN_H_
#define COMPONENTS_MEDIASTREAMER_IMPL_THUMBNAILIMGGEN_H_

#include "Observable.h"
#include "Media.h"
#include "MediaStream.h"


namespace Broadcom
{
namespace Media {

class ThumbnailHandles;

class ThumbnailOptions {
public:
    ThumbnailOptions()
        : enabled(false)
        , width(320)
        , height(180)
        , videoPid(0)
        , outputFilePath("")
        , outputFileName("capture.jpg")
        , time(0)
        , repeatEvery(0)
    {}
    bool enabled;
    unsigned width;
    unsigned height;
    unsigned videoPid;
    std::string outputFilePath; // Give full path for output file. If empty, current directory
    std::string outputFileName; //just filename. Dont include path
    unsigned time; // applicable for file source. Time in milliseconds
    unsigned repeatEvery; // Will generate image every n millisec.
    std::string inputFileName; // provide absolute path
    std::string inIndexFileName; // provide if available
    MediaStream* mediaStream;
};

enum class ThumbnailEvents {
    Completed,
    Error
};

enum class ThumbnailState {
    IdleState,
    InitializedState,
    GenImageState,
    StopGenImageState,
    ErrorState
};

class BME_SO_EXPORT ThumbnailImgGen
    : public Observable<ThumbnailEvents>
{
public:
    ThumbnailImgGen();
    virtual ~ThumbnailImgGen();
    void setThumbnailOptions(const ThumbnailOptions& options);
    ThumbnailOptions getThumbnailOptions();
    void startGenImage();
    void stopGenImage();

private:
    ThumbnailHandles* _ctx;
    ThumbnailState _state;
    ThumbnailOptions _options;
    void genImage();
    void waitIntervalTime(unsigned waitTime);
};

}  // namespace Media
}  // namespace Broadcom
#endif  // COMPONENTS_MEDIASTREAMER_IMPL_THUMBNAILIMGGEN_H_

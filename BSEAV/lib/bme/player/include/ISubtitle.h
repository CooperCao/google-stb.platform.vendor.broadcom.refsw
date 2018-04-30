/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef __ISUBTITLEDISPLAY_H__
#define __ISUBTITLEDISPLAY_H__

namespace Broadcom {
namespace Media {

class MediaStream;
class ISubtitle
{
    public:
        virtual void start(MediaStream* mediaStream) = 0;
        virtual void stop() = 0;
        virtual void setLanguage(const std::string& language) = 0;
        virtual void setVisibility(bool visible) = 0;
        virtual ~ISubtitle() {}
};

}  // namespace Media
}  // namespace Broadcom

#endif  // __ISUBTITLEDISPLAY_H__

/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef __DRM_PLAYREADY_H__
#define __DRM_PLAYREADY_H__

#include "Drm.h"

namespace Broadcom {
namespace Media {

class DrmPlayReady : public Drm
{
public:
    DrmPlayReady(MediaDrmAdaptor *mediaDrmAdaptor);
    virtual ~DrmPlayReady();

protected:
    bool parseInitData(
        const std::string& initData,
        std::string& wrmheader,
        std::string& pssh);

    bool playreadyExtractKeyId(
        const std::string& wrmheader,
        std::string& keyId);
};

}  // namespace Media
}  // namespace Broadcom

#endif  // __DRM_PLAYREADY_H__

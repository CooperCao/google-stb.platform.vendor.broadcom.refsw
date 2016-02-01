/***************************************************************************
*     Copyright (c) 2014, Broadcom Corporation*
*     All Rights Reserved*
*     Confidential Property of Broadcom Corporation*
*
*  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
*  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
*  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
*
***************************************************************************/

#include "nexus_platform.h"
#include "bstd.h"
#include "bkni.h"
#if NEXUS_HAS_VIDEO_ENCODER
#include "nexus_video_encoder.h"
#endif

BDBG_MODULE(nexus_platform_local);

unsigned NEXUS_Platform_GetVideoEncoderDisplay( unsigned videoEncoderIndex )
{
#if NEXUS_HAS_VIDEO_ENCODER
    NEXUS_VideoEncoderCapabilities cap;
    NEXUS_GetVideoEncoderCapabilities(&cap);
    if (videoEncoderIndex < NEXUS_MAX_VIDEO_ENCODERS && cap.videoEncoder[videoEncoderIndex].supported) {
        return cap.videoEncoder[videoEncoderIndex].displayIndex;
    }
#else
    BSTD_UNUSED(videoEncoderIndex);
#endif
    return 0;
}

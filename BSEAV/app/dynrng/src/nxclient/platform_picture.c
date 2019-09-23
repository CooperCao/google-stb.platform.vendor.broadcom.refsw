/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/
#include "nexus_platform_client.h"
#include "nxclient.h"
#include "platform.h"
#include "platform_priv.h"
#include "platform_picture_priv.h"
#include "picdecoder.h"
#include "nexus_types.h"
#if NEXUS_HAS_PICTURE_DECODER
#include "nexus_picture_decoder.h"
#endif
#include "bkni.h"
#include "bdbg.h"
#include <string.h>
#include <stdio.h>

BDBG_MODULE(platform_picture);

PlatformPictureHandle platform_picture_create(PlatformHandle platform, const char * picturePath)
{
    PlatformPictureHandle pic;
    NEXUS_SurfaceHandle nxSurface = NULL;
    picdecoder_t picdec;
#if NEXUS_HAS_PICTURE_DECODER
    NEXUS_PictureDecoderStatus status;
#endif

    BDBG_ASSERT(platform);
    BDBG_ASSERT(picturePath);

    picdec = picdecoder_open();
    if (picdec)
    {
        nxSurface = picdecoder_decode(picdec, picturePath);
#if NEXUS_HAS_PICTURE_DECODER
        picdecoder_get_status(picdec, &status);
#endif
        picdecoder_close(picdec);
    }
    if (!nxSurface) goto out_no_surface;
    pic = BKNI_Malloc(sizeof(*pic));
    BDBG_ASSERT(pic);
    BKNI_Memset(pic, 0, sizeof(*pic));
    pic->platform = platform;
    pic->nxSurface = nxSurface;
    pic->path = BKNI_Malloc(strlen(picturePath) + 1);
    BDBG_ASSERT(pic->path);
    strcpy(pic->path, picturePath);
#if NEXUS_HAS_PICTURE_DECODER
    platform_picture_p_info_from_nexus(&pic->info, &status);
#else
    pic->info.dynrng = PlatformDynamicRange_eSdr;
    pic->info.gamut = PlatformColorimetry_e709;
#endif
    if (!pic->info.format.width)
    {
        NEXUS_SurfaceCreateSettings picCreateSettings;
        NEXUS_Surface_GetCreateSettings(pic->nxSurface, &picCreateSettings);
        pic->info.format.width = picCreateSettings.width;
        pic->info.format.height = picCreateSettings.height;
        pic->info.depth = 8;
    }
    return pic;

out_no_surface:
    return NULL;
}

void platform_picture_destroy(PlatformPictureHandle pic)
{
    if (!pic) return;
    if (pic->nxSurface)
    {
        NEXUS_Surface_Destroy(pic->nxSurface);
        pic->nxSurface = NULL;
    }
    if (pic->path)
    {
        BKNI_Free(pic->path);
        pic->path = NULL;
    }
    BKNI_Free(pic);
}

const PlatformPictureInfo * platform_picture_get_info(PlatformPictureHandle pic)
{
    BDBG_ASSERT(pic);
    return &pic->info;
}

void platform_picture_get_dimensions(PlatformPictureHandle pic, unsigned * pWidth, unsigned * pHeight)
{
    BDBG_ASSERT(pic);
    BDBG_ASSERT(pWidth);
    BDBG_ASSERT(pHeight);
    *pWidth = pic->info.format.width;
    *pHeight = pic->info.format.height;
}

const char * platform_picture_get_path(PlatformPictureHandle pic)
{
    BDBG_ASSERT(pic);
    return pic->path;
}

const PlatformPictureFormat * platform_picture_get_format(PlatformPictureHandle pic)
{
    BDBG_ASSERT(pic);
    return &pic->info.format;
}

unsigned platform_picture_p_depth_from_pixel_format(NEXUS_PixelFormat format)
{
    unsigned depth = 0;

    switch(format)
    {
        case NEXUS_PixelFormat_ePalette1:
        case NEXUS_PixelFormat_eW1:
        case NEXUS_PixelFormat_eA1:
            depth = 1;
            break;
        case NEXUS_PixelFormat_ePalette2:
        case NEXUS_PixelFormat_eA2:
            depth = 2;
            break;
        case NEXUS_PixelFormat_eA4_R4_G4_B4:
        case NEXUS_PixelFormat_eX4_R4_G4_B4:
        case NEXUS_PixelFormat_eA4_B4_G4_R4:
        case NEXUS_PixelFormat_eX4_B4_G4_R4:
        case NEXUS_PixelFormat_eR4_G4_B4_A4:
        case NEXUS_PixelFormat_eR4_G4_B4_X4:
        case NEXUS_PixelFormat_eB4_G4_R4_A4:
        case NEXUS_PixelFormat_eB4_G4_R4_X4:
        case NEXUS_PixelFormat_eA4:
        case NEXUS_PixelFormat_ePalette4:
        case NEXUS_PixelFormat_eL4_A4:
            depth = 4;
            break;
        case NEXUS_PixelFormat_eR5_G6_B5:
        case NEXUS_PixelFormat_eB5_G6_R5:
        case NEXUS_PixelFormat_eA1_R5_G5_B5:
        case NEXUS_PixelFormat_eX1_R5_G5_B5:
        case NEXUS_PixelFormat_eA1_B5_G5_R5:
        case NEXUS_PixelFormat_eX1_B5_G5_R5:
        case NEXUS_PixelFormat_eR5_G5_B5_A1:
        case NEXUS_PixelFormat_eR5_G5_B5_X1:
        case NEXUS_PixelFormat_eB5_G5_R5_A1:
        case NEXUS_PixelFormat_eB5_G5_R5_X1:
            depth = 5;
            break;
        case NEXUS_PixelFormat_eA8_R8_G8_B8:
        case NEXUS_PixelFormat_eX8_R8_G8_B8:
        case NEXUS_PixelFormat_eA8_B8_G8_R8:
        case NEXUS_PixelFormat_eX8_B8_G8_R8:
        case NEXUS_PixelFormat_eR8_G8_B8_A8:
        case NEXUS_PixelFormat_eR8_G8_B8_X8:
        case NEXUS_PixelFormat_eB8_G8_R8_A8:
        case NEXUS_PixelFormat_eB8_G8_R8_X8:
        case NEXUS_PixelFormat_eA8:
        case NEXUS_PixelFormat_eA8_Palette8:
        case NEXUS_PixelFormat_ePalette8:
        case NEXUS_PixelFormat_eY8_Palette8:
        case NEXUS_PixelFormat_eA8_Y8:
        case NEXUS_PixelFormat_eCb8:
        case NEXUS_PixelFormat_eCr8:
        case NEXUS_PixelFormat_eY8:
        case NEXUS_PixelFormat_eCb8_Cr8:
        case NEXUS_PixelFormat_eCr8_Cb8:
        case NEXUS_PixelFormat_eY08_Cb8_Y18_Cr8:
        case NEXUS_PixelFormat_eY08_Cr8_Y18_Cb8:
        case NEXUS_PixelFormat_eY18_Cb8_Y08_Cr8:
        case NEXUS_PixelFormat_eY18_Cr8_Y08_Cb8:
        case NEXUS_PixelFormat_eCb8_Y08_Cr8_Y18:
        case NEXUS_PixelFormat_eCb8_Y18_Cr8_Y08:
        case NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08:
        case NEXUS_PixelFormat_eCr8_Y08_Cb8_Y18:
        case NEXUS_PixelFormat_eA8_Y8_Cb8_Cr8:
        case NEXUS_PixelFormat_eA8_Cr8_Cb8_Y8:
        case NEXUS_PixelFormat_eCr8_Cb8_Y8_A8:
        case NEXUS_PixelFormat_eY8_Cb8_Cr8_A8:
        case NEXUS_PixelFormat_eL8:
        case NEXUS_PixelFormat_eL8_A8:
        case NEXUS_PixelFormat_eCompressed_A8_R8_G8_B8:
        case NEXUS_PixelFormat_eR8_G8_B8:
        case NEXUS_PixelFormat_eUIF_R8_G8_B8_A8:
            depth = 8;
            break;
        case NEXUS_PixelFormat_eY10:
        case NEXUS_PixelFormat_eCb10_Cr10:
        case NEXUS_PixelFormat_eCr10_Cb10:
        case NEXUS_PixelFormat_eX2_Cr10_Y10_Cb10:
        case NEXUS_PixelFormat_eY010_Cb10_Y110_Cr10:
        case NEXUS_PixelFormat_eY010_Cr10_Y110_Cb10:
        case NEXUS_PixelFormat_eY110_Cb10_Y010_Cr10:
        case NEXUS_PixelFormat_eY110_Cr10_Y010_Cb10:
        case NEXUS_PixelFormat_eCb10_Y010_Cr10_Y110:
        case NEXUS_PixelFormat_eCb10_Y110_Cr10_Y010:
        case NEXUS_PixelFormat_eCr10_Y110_Cb10_Y010:
        case NEXUS_PixelFormat_eCr10_Y010_Cb10_Y110:
        case NEXUS_PixelFormat_eYCbCr422_10bit:
            depth = 10;
            break;
        case NEXUS_PixelFormat_eL15_L05_A6:
        case NEXUS_PixelFormat_eUnknown:
        case NEXUS_PixelFormat_eMax:
            depth = 0;
            break;
    }

    return depth;
}

PlatformColorSpace platform_picture_p_color_space_from_pixel_format(NEXUS_PixelFormat format)
{
    PlatformColorSpace space;

    switch(format)
    {
        case NEXUS_PixelFormat_eR5_G6_B5:
        case NEXUS_PixelFormat_eB5_G6_R5:
        case NEXUS_PixelFormat_eA1_R5_G5_B5:
        case NEXUS_PixelFormat_eX1_R5_G5_B5:
        case NEXUS_PixelFormat_eA1_B5_G5_R5:
        case NEXUS_PixelFormat_eX1_B5_G5_R5:
        case NEXUS_PixelFormat_eR5_G5_B5_A1:
        case NEXUS_PixelFormat_eR5_G5_B5_X1:
        case NEXUS_PixelFormat_eB5_G5_R5_A1:
        case NEXUS_PixelFormat_eB5_G5_R5_X1:
        case NEXUS_PixelFormat_eA4_R4_G4_B4:
        case NEXUS_PixelFormat_eX4_R4_G4_B4:
        case NEXUS_PixelFormat_eA4_B4_G4_R4:
        case NEXUS_PixelFormat_eX4_B4_G4_R4:
        case NEXUS_PixelFormat_eR4_G4_B4_A4:
        case NEXUS_PixelFormat_eR4_G4_B4_X4:
        case NEXUS_PixelFormat_eB4_G4_R4_A4:
        case NEXUS_PixelFormat_eB4_G4_R4_X4:
        case NEXUS_PixelFormat_eA8_R8_G8_B8:
        case NEXUS_PixelFormat_eX8_R8_G8_B8:
        case NEXUS_PixelFormat_eA8_B8_G8_R8:
        case NEXUS_PixelFormat_eX8_B8_G8_R8:
        case NEXUS_PixelFormat_eR8_G8_B8_A8:
        case NEXUS_PixelFormat_eR8_G8_B8_X8:
        case NEXUS_PixelFormat_eB8_G8_R8_A8:
        case NEXUS_PixelFormat_eB8_G8_R8_X8:
        case NEXUS_PixelFormat_eCompressed_A8_R8_G8_B8:
        case NEXUS_PixelFormat_eR8_G8_B8:
        case NEXUS_PixelFormat_eUIF_R8_G8_B8_A8:
            space = PlatformColorSpace_eRgb;
            break;
        case NEXUS_PixelFormat_eA8_Y8:
        case NEXUS_PixelFormat_eCb8:
        case NEXUS_PixelFormat_eCr8:
        case NEXUS_PixelFormat_eY8:
        case NEXUS_PixelFormat_eCb8_Cr8:
        case NEXUS_PixelFormat_eCr8_Cb8:
        case NEXUS_PixelFormat_eY10:
        case NEXUS_PixelFormat_eCb10_Cr10:
        case NEXUS_PixelFormat_eCr10_Cb10:
        case NEXUS_PixelFormat_eX2_Cr10_Y10_Cb10:
        case NEXUS_PixelFormat_eA8_Y8_Cb8_Cr8:
        case NEXUS_PixelFormat_eA8_Cr8_Cb8_Y8:
        case NEXUS_PixelFormat_eCr8_Cb8_Y8_A8:
        case NEXUS_PixelFormat_eY8_Cb8_Cr8_A8:
        case NEXUS_PixelFormat_eY08_Cb8_Y18_Cr8:
        case NEXUS_PixelFormat_eY08_Cr8_Y18_Cb8:
        case NEXUS_PixelFormat_eY18_Cb8_Y08_Cr8:
        case NEXUS_PixelFormat_eY18_Cr8_Y08_Cb8:
        case NEXUS_PixelFormat_eCb8_Y08_Cr8_Y18:
        case NEXUS_PixelFormat_eCb8_Y18_Cr8_Y08:
        case NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08:
        case NEXUS_PixelFormat_eCr8_Y08_Cb8_Y18:
        case NEXUS_PixelFormat_eY010_Cb10_Y110_Cr10:
        case NEXUS_PixelFormat_eY010_Cr10_Y110_Cb10:
        case NEXUS_PixelFormat_eY110_Cb10_Y010_Cr10:
        case NEXUS_PixelFormat_eY110_Cr10_Y010_Cb10:
        case NEXUS_PixelFormat_eCb10_Y010_Cr10_Y110:
        case NEXUS_PixelFormat_eCb10_Y110_Cr10_Y010:
        case NEXUS_PixelFormat_eCr10_Y110_Cb10_Y010:
        case NEXUS_PixelFormat_eCr10_Y010_Cb10_Y110:
        case NEXUS_PixelFormat_eYCbCr422_10bit:
            space = PlatformColorSpace_eYCbCr;
            break;
        case NEXUS_PixelFormat_eA8_Palette8:
        case NEXUS_PixelFormat_ePalette8:
        case NEXUS_PixelFormat_ePalette4:
        case NEXUS_PixelFormat_ePalette2:
        case NEXUS_PixelFormat_ePalette1:
        case NEXUS_PixelFormat_eY8_Palette8:
        case NEXUS_PixelFormat_eL8:
        case NEXUS_PixelFormat_eL4_A4:
        case NEXUS_PixelFormat_eL8_A8:
        case NEXUS_PixelFormat_eL15_L05_A6:
        case NEXUS_PixelFormat_eA8:
        case NEXUS_PixelFormat_eA4:
        case NEXUS_PixelFormat_eA2:
        case NEXUS_PixelFormat_eA1:
        case NEXUS_PixelFormat_eW1:
        case NEXUS_PixelFormat_eUnknown:
            space = PlatformColorSpace_eUnknown;
            break;
        case NEXUS_PixelFormat_eMax:
            space = PlatformColorSpace_eInvalid;
            break;
        default:
            BDBG_WRN(("Unhandled pixel format: %u", format));
            space = PlatformColorSpace_eUnknown;
            break;
    }
    return space;
}

unsigned platform_picture_p_color_sampling_from_pixel_format(NEXUS_PixelFormat format)
{
    unsigned sampling;

    switch(format)
    {
        case NEXUS_PixelFormat_eR5_G6_B5:
        case NEXUS_PixelFormat_eB5_G6_R5:
        case NEXUS_PixelFormat_eA1_R5_G5_B5:
        case NEXUS_PixelFormat_eX1_R5_G5_B5:
        case NEXUS_PixelFormat_eA1_B5_G5_R5:
        case NEXUS_PixelFormat_eX1_B5_G5_R5:
        case NEXUS_PixelFormat_eR5_G5_B5_A1:
        case NEXUS_PixelFormat_eR5_G5_B5_X1:
        case NEXUS_PixelFormat_eB5_G5_R5_A1:
        case NEXUS_PixelFormat_eB5_G5_R5_X1:
        case NEXUS_PixelFormat_eA4_R4_G4_B4:
        case NEXUS_PixelFormat_eX4_R4_G4_B4:
        case NEXUS_PixelFormat_eA4_B4_G4_R4:
        case NEXUS_PixelFormat_eX4_B4_G4_R4:
        case NEXUS_PixelFormat_eR4_G4_B4_A4:
        case NEXUS_PixelFormat_eR4_G4_B4_X4:
        case NEXUS_PixelFormat_eB4_G4_R4_A4:
        case NEXUS_PixelFormat_eB4_G4_R4_X4:
        case NEXUS_PixelFormat_eA8_R8_G8_B8:
        case NEXUS_PixelFormat_eX8_R8_G8_B8:
        case NEXUS_PixelFormat_eA8_B8_G8_R8:
        case NEXUS_PixelFormat_eX8_B8_G8_R8:
        case NEXUS_PixelFormat_eR8_G8_B8_A8:
        case NEXUS_PixelFormat_eR8_G8_B8_X8:
        case NEXUS_PixelFormat_eB8_G8_R8_A8:
        case NEXUS_PixelFormat_eB8_G8_R8_X8:
        case NEXUS_PixelFormat_eCompressed_A8_R8_G8_B8:
        case NEXUS_PixelFormat_eR8_G8_B8:
        case NEXUS_PixelFormat_eUIF_R8_G8_B8_A8:
        case NEXUS_PixelFormat_eA8_Y8:
        case NEXUS_PixelFormat_eCb8:
        case NEXUS_PixelFormat_eCr8:
        case NEXUS_PixelFormat_eY8:
        case NEXUS_PixelFormat_eCb8_Cr8:
        case NEXUS_PixelFormat_eCr8_Cb8:
        case NEXUS_PixelFormat_eY10:
        case NEXUS_PixelFormat_eCb10_Cr10:
        case NEXUS_PixelFormat_eCr10_Cb10:
        case NEXUS_PixelFormat_eX2_Cr10_Y10_Cb10:
        case NEXUS_PixelFormat_eA8_Y8_Cb8_Cr8:
        case NEXUS_PixelFormat_eA8_Cr8_Cb8_Y8:
        case NEXUS_PixelFormat_eCr8_Cb8_Y8_A8:
        case NEXUS_PixelFormat_eY8_Cb8_Cr8_A8:
            sampling = 444;
            break;
        case NEXUS_PixelFormat_eY08_Cb8_Y18_Cr8:
        case NEXUS_PixelFormat_eY08_Cr8_Y18_Cb8:
        case NEXUS_PixelFormat_eY18_Cb8_Y08_Cr8:
        case NEXUS_PixelFormat_eY18_Cr8_Y08_Cb8:
        case NEXUS_PixelFormat_eCb8_Y08_Cr8_Y18:
        case NEXUS_PixelFormat_eCb8_Y18_Cr8_Y08:
        case NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08:
        case NEXUS_PixelFormat_eCr8_Y08_Cb8_Y18:
        case NEXUS_PixelFormat_eY010_Cb10_Y110_Cr10:
        case NEXUS_PixelFormat_eY010_Cr10_Y110_Cb10:
        case NEXUS_PixelFormat_eY110_Cb10_Y010_Cr10:
        case NEXUS_PixelFormat_eY110_Cr10_Y010_Cb10:
        case NEXUS_PixelFormat_eCb10_Y010_Cr10_Y110:
        case NEXUS_PixelFormat_eCb10_Y110_Cr10_Y010:
        case NEXUS_PixelFormat_eCr10_Y110_Cb10_Y010:
        case NEXUS_PixelFormat_eCr10_Y010_Cb10_Y110:
        case NEXUS_PixelFormat_eYCbCr422_10bit:
            sampling = 422;
            break;
        case NEXUS_PixelFormat_eA8_Palette8:
        case NEXUS_PixelFormat_ePalette8:
        case NEXUS_PixelFormat_ePalette4:
        case NEXUS_PixelFormat_ePalette2:
        case NEXUS_PixelFormat_ePalette1:
        case NEXUS_PixelFormat_eY8_Palette8:
        case NEXUS_PixelFormat_eL8:
        case NEXUS_PixelFormat_eL4_A4:
        case NEXUS_PixelFormat_eL8_A8:
        case NEXUS_PixelFormat_eL15_L05_A6:
        case NEXUS_PixelFormat_eA8:
        case NEXUS_PixelFormat_eA4:
        case NEXUS_PixelFormat_eA2:
        case NEXUS_PixelFormat_eA1:
        case NEXUS_PixelFormat_eW1:
        case NEXUS_PixelFormat_eUnknown:
        case NEXUS_PixelFormat_eMax:
            sampling = 0;
            break;
        default:
            BDBG_WRN(("Unhandled pixel format: %u", format));
            sampling = 0;
            break;
    }
    return sampling;
}

#if NEXUS_HAS_PICTURE_DECODER
void platform_picture_p_info_from_nexus(PlatformPictureInfo * pInfo, NEXUS_PictureDecoderStatus * pStatus)
{
    BDBG_ASSERT(pInfo);
    BDBG_ASSERT(pStatus);
    BKNI_Memset(pInfo, 0, sizeof(*pInfo));
    pInfo->dynrng = PlatformDynamicRange_eSdr;
    pInfo->gamut = PlatformColorimetry_e709;
    if (pStatus->headerValid)
    {
        pInfo->format.height = pStatus->header.height;
        pInfo->format.width = pStatus->header.width;
        pInfo->format.interlaced = pStatus->header.multiscan;
        pInfo->depth = platform_picture_p_depth_from_pixel_format(pStatus->header.format);
        pInfo->space = platform_picture_p_color_space_from_pixel_format(pStatus->header.format);
        pInfo->sampling = platform_picture_p_color_sampling_from_pixel_format(pStatus->header.format);
    }
}
#endif

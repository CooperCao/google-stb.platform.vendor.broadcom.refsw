/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 *****************************************************************************/
#include "image_viewer.h"
#include "image_viewer_priv.h"
#include "platform.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

bool image_viewer_file_filter(const char * path)
{
    assert(path);

    if
    (
        strstr(path, ".png")
        ||
        strstr(path, ".jpg")
        ||
        strstr(path, ".bmp")
    )
    {
        return true;
    }
    else
    {
        return false;
    }
}

void image_viewer_get_default_create_settings(ImageViewerCreateSettings * pSettings)
{
    assert(pSettings);
    memset(pSettings, 0, sizeof(*pSettings));
}

ImageViewerHandle image_viewer_create(const ImageViewerCreateSettings * pSettings)
{
    ImageViewerHandle viewer;

    assert(pSettings);
    assert(pSettings->filer);

    viewer = malloc(sizeof(*viewer));
    if (!viewer) goto error;
    memset(viewer, 0, sizeof(*viewer));

    memcpy(&viewer->createSettings, pSettings, sizeof(*pSettings));


    return viewer;

error:
    image_viewer_destroy(viewer);
    return NULL;
}

static void image_viewer_p_unload_image(ImageViewerHandle viewer)
{
    if (viewer->pic)
    {
        if (viewer->createSettings.pictureChanged.callback)
        {
            viewer->createSettings.pictureChanged.callback(viewer->createSettings.pictureChanged.context, NULL);
        }
        platform_picture_destroy(viewer->pic);
        viewer->pic = NULL;
    }
}

void image_viewer_destroy(ImageViewerHandle viewer)
{
    if (!viewer) return;

    image_viewer_p_unload_image(viewer);

    free(viewer);
}

void image_viewer_view_image(ImageViewerHandle viewer, const char * imagePath)
{
    const char * fullPath = NULL;

    if (!imagePath) goto unload;

    fullPath = imagePath;
    if (imagePath[0] != '/')
    {
        fullPath = file_manager_find(viewer->createSettings.filer, imagePath);
    }
    if (!fullPath) { printf("viewer: image '%s' not found\n", imagePath); goto unload; }
    image_viewer_p_load_image(viewer, fullPath);
    return;
unload:
    image_viewer_p_unload_image(viewer);
}

void image_viewer_p_load_image(ImageViewerHandle viewer, const char * imagePath)
{
    const char * newPath = imagePath;
    const char * oldPath = NULL;

    assert(viewer);

    if (viewer->pic)
    {
        oldPath = platform_picture_get_path(viewer->pic);
    }

    if (!newPath || (newPath && oldPath && !strcmp(oldPath, newPath))) return;

    image_viewer_p_unload_image(viewer);

    if (newPath)
    {
        viewer->pic = platform_picture_create(viewer->createSettings.platform, newPath);
        if (viewer->pic)
        {
            if (viewer->createSettings.pictureChanged.callback)
            {
                viewer->createSettings.pictureChanged.callback(viewer->createSettings.pictureChanged.context, viewer->pic);
            }
        }
    }
}

const PlatformPictureInfo * image_viewer_get_picture_info(ImageViewerHandle viewer)
{
    if (viewer && viewer->pic)
    {
        return platform_picture_get_info(viewer->pic);
    }
    else
    {
        return NULL;
    }
}

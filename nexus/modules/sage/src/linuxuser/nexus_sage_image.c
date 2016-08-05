/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/

#if !defined(NEXUS_MODE_driver)

#include "bstd.h"

#include "bkni.h"
#include "bimg.h"

#include "nexus_base_types.h"
#include "nexus_base_os.h" /* For NEXUS_GetEnv */
#include "nexus_base_image.h"
#include "../nexus_sage_image.h"

BDBG_MODULE(nexus_sage_image);

static const char* firmware_images[SAGE_IMAGE_FirmwareID_Max] =
{
    "sage_bl_dev.bin",              /* SAGE boot loader for development (ZS) chip */
    "sage_framework_dev.bin",       /* SAGE kernel for development (ZS)c hip*/
    "sage_bl.bin",                  /* SAGE boot loader for production (ZB or customer specific) chip */
    "sage_framework.bin",           /* SAGE kernel for production (ZB or custoemr specific) chip */
    "sage_ta_secure_video_dev.bin", /* SAGE SVP TA binary for development (ZS) chip */
    "sage_ta_secure_video.bin",     /* SAGE SVP TA binary for production (ZB or customer specific) chip */
    "sage_ta_antirollback_dev.bin", /* SAGE AntiRollback TA binary for development (ZS) chip */
    "sage_ta_antirollback.bin",     /* SAGE AntiRollback TA binary for production (ZB or customer specific) chip */
};

void *SAGE_IMAGE_Context = (void *) firmware_images;

static NEXUS_Error sage_image_open(void *context, void **image, unsigned image_id)
{
    NEXUS_Error error;

    BDBG_ENTER(sage_image_open);
    BDBG_ASSERT(context == firmware_images);
    BDBG_ASSERT(image_id < SAGE_IMAGE_FirmwareID_Max);
    error = NEXUS_BaseImage_Open(image, firmware_images[image_id], NEXUS_GetEnv("SAGEBIN_PATH"));
    if (error) error = BERR_TRACE(error);
    BDBG_LEAVE(sage_image_open);

    return error;
}

static NEXUS_Error sage_image_next(void *image, unsigned chunk, const void **data, uint16_t length)
{
    NEXUS_Error error;

    BDBG_ENTER(sage_image_next);
    error = NEXUS_BaseImage_Next( image, chunk, data, length);
    if (error) error = BERR_TRACE(error);
    BDBG_LEAVE(sage_image_next);

    return error;
}

static void sage_image_close(void *image)
{
    BDBG_ENTER(sage_image_close);
    NEXUS_BaseImage_Close(image);
    BDBG_LEAVE(sage_image_close);
}

BIMG_Interface SAGE_IMAGE_Interface = {
    sage_image_open,
    sage_image_next,
    sage_image_close
};

#endif /* NEXUS_MODE_* */

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
#include "../nexus_sage_image.h"

BDBG_MODULE(nexus_sage_image);

#include <stdio.h>

#define SAGE_IMG_BUFFER_SIZE      (64*1024)

#define SAGE_DEFAULT_PATH         "."

#define boot_loader_image         "sage_bl.bin"
#define kernel_image              "sage_framework.bin"
#define dev_boot_loader_image     "sage_bl_dev.bin"
#define dev_kernel_image          "sage_framework_dev.bin"

#define dev_ta_svp     "sage_ta_secure_video_dev.bin"
#define ta_svp          "sage_ta_secure_video.bin"

#define dev_ta_ar     "sage_ta_antirollback_dev.bin"
#define ta_ar          "sage_ta_antirollback.bin"

#define SAGE_IMAGE_ContextEntry char

static const SAGE_IMAGE_ContextEntry* SAGE_IMAGE_FirmwareImages[SAGE_IMAGE_FirmwareID_Max] =
{
    (char *)dev_boot_loader_image, /* SAGE boot loader for development (ZS) chip */
    (char *)dev_kernel_image,      /* SAGE kernel for development (ZS)c hip*/
    (char *)boot_loader_image,     /* SAGE boot loader for production (ZB or customer specific) chip */
    (char *)kernel_image,          /* SAGE kernel for production (ZB or custoemr specific) chip */
    (char *)dev_ta_svp,            /* SAGE SVP TA binary for development (ZS) chip */
    (char *)ta_svp,                /* SAGE SVP TA binary for production (ZB or customer specific) chip */
    (char *)dev_ta_ar,             /* SAGE AntiRollback TA binary for development (ZS) chip */
    (char *)ta_ar,                 /* SAGE AntiRollback TA binary for production (ZB or customer specific) chip */
};

void *SAGE_IMAGE_Context = (void *) SAGE_IMAGE_FirmwareImages;

typedef struct SAGE_IMAGE_Container
{
    FILE            *fd;
    char            *filename;
    uint32_t        nb_next;
    uint32_t        uiImageSize;
    void            *buf;
    bool            returnSize;
} SAGE_IMAGE_Container;


static NEXUS_Error sage_p_open(void *context,
                 void **image,
                 unsigned image_id)
{
    FILE *fd = NULL;
    NEXUS_Error rc =  NEXUS_SUCCESS;
    SAGE_IMAGE_ContextEntry *pContextEntry = NULL;
    SAGE_IMAGE_Container *pImageContainer = NULL;
    long size = 0;
    const char *env_sage_path = NULL;
    char * filepath = NULL;
    size_t filepath_len = 0;

    BDBG_ENTER(sage_p_open);

    BDBG_ASSERT(context);
    BDBG_ASSERT(image);

    /* Validate the firmware ID range */
    BDBG_MSG(("%s - Validating image ID range", __FUNCTION__));
    if (image_id >= SAGE_IMAGE_FirmwareID_Max)
    {
        BDBG_ERR(("%s - Invalid image id %d", __FUNCTION__, image_id));
        rc = NEXUS_INVALID_PARAMETER;
        goto err;
    }

    /* Validate the image referenced by firmware ID exists in the context */
    pContextEntry = ((SAGE_IMAGE_ContextEntry**)context)[image_id];
    if (pContextEntry == NULL)
    {
        BDBG_ERR(("%s - Image id %d is NULL", __FUNCTION__, image_id));
        rc = NEXUS_INVALID_PARAMETER;
        goto err;
    }

    if (pContextEntry)
    {
        char *tmp = (char *)pContextEntry;
        while (*tmp++)
        {
            filepath_len++;
        }
    }

    env_sage_path = NEXUS_GetEnv("SAGEBIN_PATH");
    if (!env_sage_path)
    {
        env_sage_path = SAGE_DEFAULT_PATH;
    }

    {
        char *tmp = (char *)env_sage_path;
        while (*tmp++)
        {
            filepath_len++;
        }
    }

    filepath_len += 2; /* for '/' and trailing 0 */

    filepath = BKNI_Malloc(filepath_len);
    if (!filepath)
    {
        BDBG_ERR(("%s - Cannot allocate buffer for file path (%d bytes)", __FUNCTION__, (unsigned)filepath_len));
        rc = NEXUS_NOT_AVAILABLE;
        goto err;
    }

    if (BKNI_Snprintf(filepath, filepath_len, "%s/%s", env_sage_path, (char *)pContextEntry) != (int)(filepath_len-1))
    {
        BDBG_ERR(("%s - Cannot build final binary path", __FUNCTION__));
        rc = NEXUS_NOT_AVAILABLE;
        goto err;
    }

    /* Open the file */
    BDBG_MSG(("%s - Opening file %s", __FUNCTION__, filepath));
    fd = fopen(filepath, "r");
    if(fd == NULL)
    {
        BDBG_WRN(("%s - Cannot open file %s", __FUNCTION__, filepath));
        rc = NEXUS_NOT_AVAILABLE;
        goto err;
    }

    /* Get size of file */
    if(fseek(fd, 0, SEEK_END) == -1)
    {
        BDBG_ERR(("%s: Cannot get to end of file %s", __FUNCTION__, filepath));
        rc = NEXUS_NOT_AVAILABLE;
        goto err;
    }

    size = ftell(fd);
    if(size < 0){
        BDBG_ERR(("%s: Cannot get size of file %s", __FUNCTION__, filepath));
        rc = NEXUS_NOT_AVAILABLE;
        goto err;
    }

    if(fseek(fd, 0, SEEK_SET) == -1)
    {
        BDBG_ERR(("%s: Cannot get to beginning of file %s", __FUNCTION__, filepath));
        rc = NEXUS_NOT_AVAILABLE;
        goto err;
    }

    /* Allocate an image container struct */
    BDBG_MSG(("%s - Allocating memory for image container", __FUNCTION__));
    pImageContainer = (SAGE_IMAGE_Container*) BKNI_Malloc(sizeof(SAGE_IMAGE_Container));
    if(pImageContainer == NULL) {
        BDBG_ERR(("%s - Cannot allocate memory for image container", __FUNCTION__));
        rc = NEXUS_OUT_OF_SYSTEM_MEMORY;
        goto err;
    }
    BKNI_Memset(pImageContainer, 0, sizeof(SAGE_IMAGE_Container));

    /* Fill in the image container struct */
    pImageContainer->fd = fd;
    pImageContainer->filename = filepath;
    pImageContainer->uiImageSize = size;
    pImageContainer->returnSize = false;
    pImageContainer->nb_next = 0;

    /* Allocate buffer for read data */
    pImageContainer->buf = (uint8_t*) BKNI_Malloc(SAGE_IMG_BUFFER_SIZE);
    if(pImageContainer->buf == NULL) {
        BDBG_ERR(("%s - Cannot allocate buffer for read buffer", __FUNCTION__));
        rc = NEXUS_OUT_OF_SYSTEM_MEMORY;
        goto err;
    }

    *image = pImageContainer;

    BDBG_MSG(("%s: File %s opened, file id %p size %d\n", __FUNCTION__, pImageContainer->filename, (void *)pImageContainer->fd, pImageContainer->uiImageSize));
    return NEXUS_SUCCESS;
err:

    if(fd) {
        fclose(fd);
    }

    if (filepath) {
        BKNI_Free(filepath);
        filepath = NULL;
    }

    if(pImageContainer) {
        if(pImageContainer->buf) {
            BKNI_Free(pImageContainer->buf);
            pImageContainer->buf = NULL;
        }
        BKNI_Free(pImageContainer);
        pImageContainer = NULL;
    }
    return rc;
}


static NEXUS_Error sage_p_next(void *image, unsigned chunk, const void **data, uint16_t length)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    size_t read = 0;
    int a = 0;
    SAGE_IMAGE_Container *pImageContainer = NULL;

    BDBG_ENTER(sage_p_next);
    BDBG_ASSERT(image);
    BDBG_ASSERT(data);
    BDBG_ASSERT(length);
    BSTD_UNUSED(chunk);

    pImageContainer = (SAGE_IMAGE_Container*)image;

    /* return size of buffer */
    if((pImageContainer->returnSize == false) && (ftell(pImageContainer->fd) == 0)) {
        *data = &pImageContainer->uiImageSize;
        pImageContainer->returnSize = true;
        BDBG_MSG(("%s - Returning size (%u bytes) of file %s to calling user", __FUNCTION__, pImageContainer->uiImageSize, pImageContainer->filename));

        goto end;
    }

    if((pImageContainer->nb_next + length) > pImageContainer->uiImageSize)  {
        BDBG_ERR(("%s: File %s size exceeded\n", __FUNCTION__, pImageContainer->filename));
        rc = NEXUS_INVALID_PARAMETER;
        goto end;
    }

    BDBG_MSG(("%s - Reading %u bytes from file %s", __FUNCTION__, length, pImageContainer->filename));
    read = fread(pImageContainer->buf, 1, length, pImageContainer->fd);
    /* Error */
    if(read == 0)   {
        BDBG_ERR(("%s: Error reading from file. No data read\n", __FUNCTION__));
        rc = NEXUS_NOT_AVAILABLE;
        goto end;
    }
    else    {
        if(read != length)  {
            /* Error */
            BDBG_ERR(("%s: Error reading from file %s, read: %d, length: %d\n", __FUNCTION__, pImageContainer->filename, (unsigned)read, length));
            if(feof(pImageContainer->fd))   {
                BDBG_ERR(("%s: EOF reached\n", __FUNCTION__));
            }
            else {
                a = ferror(pImageContainer->fd);
                if(a > 0) {
                    BDBG_ERR(("%s: File error %d\n", __FUNCTION__, a));
                }

            }
            rc = NEXUS_NOT_AVAILABLE;
            goto end;
        }
        else    {
            /* Good */
            pImageContainer->nb_next += read;
            BDBG_MSG(("%s - Read so far: %u bytes", __FUNCTION__, pImageContainer->nb_next));
        }
    }

    *data = pImageContainer->buf;

end:
    return rc;
}

static void sage_p_close(void *image)
{
    SAGE_IMAGE_Container *pImageContainer = NULL;

    BDBG_ASSERT(image);
    pImageContainer = (SAGE_IMAGE_Container*)image;

    if(pImageContainer->fd) {
        fclose(pImageContainer->fd);
        pImageContainer->fd = NULL;
    }

    if(pImageContainer->filename) {
        BKNI_Free(pImageContainer->filename);
        pImageContainer->filename = NULL;
    }

    if(pImageContainer->buf) {
        BKNI_Free(pImageContainer->buf);
        pImageContainer->buf = NULL;
    }

    if(pImageContainer) {
        BKNI_Free(pImageContainer);
        pImageContainer = NULL;
    }
    return;
}

BIMG_Interface SAGE_IMAGE_Interface = {
    sage_p_open,
    sage_p_next,
    sage_p_close
};

#endif /* NEXUS_MODE_* */

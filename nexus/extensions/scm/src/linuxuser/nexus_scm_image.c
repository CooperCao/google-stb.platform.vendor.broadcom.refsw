/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 **************************************************************************/

#if !defined(NEXUS_MODE_driver)

#include "bstd.h"

#include "bkni.h"
#include "bimg.h"

#include "nexus_base_types.h"
#include "nexus_base_os.h" /* For NEXUS_GetEnv */

BDBG_MODULE(nexus_scm_image);

#include <stdio.h>

#define SCM_IMAGE_FirmwareID_Max (4)
#define SCM_IMG_BUFFER_SIZE      (64*1024)

#define SCM_DEFAULT_PATH         "."

#define dev_boot_loader_image     "scm_bl_dev.bin"

#define scm_generic_image "scm_generic.bin"
#define scm_arris_image "scm_arris.bin"
#define scm_cisco_image "scm_cisco.bin"

#define SCM_IMAGE_ContextEntry char

static const SCM_IMAGE_ContextEntry* SCM_IMAGE_FirmwareImages[SCM_IMAGE_FirmwareID_Max] =
{
    (char *)dev_boot_loader_image, /* SCM boot loader for development (ZS) chip */
    (char *)scm_generic_image,
    (char *)scm_arris_image,
    (char *)scm_cisco_image,

};

void *SCM_IMAGE_Context = (void *) SCM_IMAGE_FirmwareImages;

typedef struct SCM_IMAGE_Container
{
    FILE            *fd;
    char            *filename;
    uint32_t        nb_next;
    uint32_t        uiImageSize;
    void            *buf;
    bool            returnSize;
} SCM_IMAGE_Container;


static NEXUS_Error scm_p_open(void *context,
                 void **image,
                 unsigned image_id)
{
    FILE *fd = NULL;
    NEXUS_Error rc =  NEXUS_SUCCESS;
    SCM_IMAGE_ContextEntry *pContextEntry = NULL;
    SCM_IMAGE_Container *pImageContainer = NULL;
    long size = 0;
    const char *env_scm_path = NULL;
    char * filepath = NULL;
    size_t filepath_len = 0;

    BDBG_ENTER(scm_p_open);

    BDBG_ASSERT(context);
    BDBG_ASSERT(image);

    /* Validate the firmware ID range */
    BDBG_MSG(("%s - Validating image ID range", BSTD_FUNCTION));
    if (image_id >= SCM_IMAGE_FirmwareID_Max)
    {
        BDBG_ERR(("%s - Invalid image id %d", BSTD_FUNCTION, image_id));
        rc = NEXUS_INVALID_PARAMETER;
        goto err;
    }

    /* Validate the image referenced by firmware ID exists in the context */
    pContextEntry = ((SCM_IMAGE_ContextEntry**)context)[image_id];
    if (pContextEntry == NULL)
    {
        BDBG_ERR(("%s - Image id %d is NULL", BSTD_FUNCTION, image_id));
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

    env_scm_path = NEXUS_GetEnv("SCMBIN_PATH");
    if (!env_scm_path)
    {
        env_scm_path = SCM_DEFAULT_PATH;
    }

    {
        char *tmp = (char *)env_scm_path;
        while (*tmp++)
        {
            filepath_len++;
        }
    }

    filepath_len += 2; /* for '/' and trailing 0 */

    filepath = BKNI_Malloc(filepath_len);
    if (!filepath)
    {
        BDBG_ERR(("%s - Cannot allocate buffer for file path (%u bytes)", BSTD_FUNCTION, (unsigned)filepath_len));
        rc = NEXUS_NOT_AVAILABLE;
        goto err;
    }

    if (BKNI_Snprintf(filepath, filepath_len, "%s/%s", env_scm_path, (char *)pContextEntry) != (int)(filepath_len-1))
    {
        BDBG_ERR(("%s - Cannot build final binary path", BSTD_FUNCTION));
        rc = NEXUS_NOT_AVAILABLE;
        goto err;
    }

    /* Open the file */
    BDBG_MSG(("%s - Opening file %s", BSTD_FUNCTION, filepath));
    fd = fopen(filepath, "r");
    if(fd == NULL)
    {
        BDBG_ERR(("%s - Cannot open file %s", BSTD_FUNCTION, filepath));
        rc = NEXUS_NOT_AVAILABLE;
        goto err;
    }

    /* Get size of file */
    if(fseek(fd, 0, SEEK_END) == -1)
    {
        BDBG_ERR(("%s: Cannot get to end of file %s", BSTD_FUNCTION, filepath));
        rc = NEXUS_NOT_AVAILABLE;
        goto err;
    }

    size = ftell(fd);
    if(size < 0){
        BDBG_ERR(("%s: Cannot get size of file %s", BSTD_FUNCTION, filepath));
        rc = NEXUS_NOT_AVAILABLE;
        goto err;
    }

    if(fseek(fd, 0, SEEK_SET) == -1)
    {
        BDBG_ERR(("%s: Cannot get to beginning of file %s", BSTD_FUNCTION, filepath));
        rc = NEXUS_NOT_AVAILABLE;
        goto err;
    }

    /* Allocate an image container struct */
    BDBG_MSG(("%s - Allocating memory for image container", BSTD_FUNCTION));
    pImageContainer = (SCM_IMAGE_Container*) BKNI_Malloc(sizeof(SCM_IMAGE_Container));
    if(pImageContainer == NULL) {
        BDBG_ERR(("%s - Cannot allocate memory for image container", BSTD_FUNCTION));
        rc = NEXUS_OUT_OF_SYSTEM_MEMORY;
        goto err;
    }
    BKNI_Memset(pImageContainer, 0, sizeof(SCM_IMAGE_Container));

    /* Fill in the image container struct */
    pImageContainer->fd = fd;
    pImageContainer->filename = filepath;
    pImageContainer->uiImageSize = size;
    pImageContainer->returnSize = false;
    pImageContainer->nb_next = 0;

    /* Allocate buffer for read data */
    pImageContainer->buf = (uint8_t*) BKNI_Malloc(SCM_IMG_BUFFER_SIZE);
    if(pImageContainer->buf == NULL) {
        BDBG_ERR(("%s - Cannot allocate buffer for read buffer", BSTD_FUNCTION));
        rc = NEXUS_OUT_OF_SYSTEM_MEMORY;
        goto err;
    }

    *image = pImageContainer;

    BDBG_MSG(("%s: File %s opened, file id %p size %d\n", BSTD_FUNCTION, pImageContainer->filename, (void*)pImageContainer->fd, pImageContainer->uiImageSize));
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


static NEXUS_Error scm_p_next(void *image, unsigned chunk, const void **data, uint16_t length)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    size_t read = 0;
    int a = 0;
    SCM_IMAGE_Container *pImageContainer = NULL;

    BDBG_ENTER(scm_p_next);
    BDBG_ASSERT(image);
    BDBG_ASSERT(data);
    BDBG_ASSERT(length);
    BSTD_UNUSED(chunk);

    pImageContainer = (SCM_IMAGE_Container*)image;

    /* return size of buffer */
    if((pImageContainer->returnSize == false) && (ftell(pImageContainer->fd) == 0)) {
        *data = &pImageContainer->uiImageSize;
        pImageContainer->returnSize = true;
        BDBG_MSG(("%s - Returning size (%u bytes) of file %s to calling user", BSTD_FUNCTION, pImageContainer->uiImageSize, pImageContainer->filename));

        goto end;
    }

    if((pImageContainer->nb_next + length) > pImageContainer->uiImageSize)  {
        BDBG_ERR(("%s: File %s size exceeded\n", BSTD_FUNCTION, pImageContainer->filename));
        rc = NEXUS_INVALID_PARAMETER;
        goto end;
    }

    BDBG_MSG(("%s - Reading %u bytes from file %s", BSTD_FUNCTION, length, pImageContainer->filename));
    read = fread(pImageContainer->buf, 1, length, pImageContainer->fd);
    /* Error */
    if(read == 0)   {
        BDBG_ERR(("%s: Error reading from file. No data read\n", BSTD_FUNCTION));
        rc = NEXUS_NOT_AVAILABLE;
        goto end;
    }
    else    {
        if(read != length)  {
            /* Error */
            BDBG_ERR(("%s: Error reading from file %s, read: %d, length: %d\n", BSTD_FUNCTION, pImageContainer->filename, (unsigned)read, length));
            if(feof(pImageContainer->fd))   {
                BDBG_ERR(("%s: EOF reached\n", BSTD_FUNCTION));
            }
            else {
                a = ferror(pImageContainer->fd);
                if(a > 0) {
                    BDBG_ERR(("%s: File error %d\n", BSTD_FUNCTION, a));
                }

            }
            rc = NEXUS_NOT_AVAILABLE;
            goto end;
        }
        else    {
            /* Good */
            pImageContainer->nb_next += read;
            BDBG_MSG(("%s - Read so far: %u bytes", BSTD_FUNCTION, pImageContainer->nb_next));
        }
    }

    *data = pImageContainer->buf;

end:
    return rc;
}

static void scm_p_close(void *image)
{
    SCM_IMAGE_Container *pImageContainer = NULL;

    BDBG_ASSERT(image);
    pImageContainer = (SCM_IMAGE_Container*)image;

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

BIMG_Interface SCM_IMAGE_Interface = {
    scm_p_open,
    scm_p_next,
    scm_p_close
};

#endif /* NEXUS_MODE_* */

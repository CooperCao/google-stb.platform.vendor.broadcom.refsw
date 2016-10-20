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

#if !defined(NEXUS_MODE_driver) && !defined(NEXUS_BASE_OS_linuxkernel)

#include "bstd.h"
#include "bkni.h"
#include "bimg.h"

#include "nexus_base_types.h"
#include "nexus_base_os.h" /* For NEXUS_GetEnv */
#include "nexus_base_image.h"

BDBG_MODULE(nexus_base_image);

#include <stdio.h>

#define DEFAULT_PATH              "."
#define IMAGE_BUFFER_SIZE         (64 * 1024)

typedef struct
{
    FILE            *fd;
    char            *filename;
    uint32_t        nb_next;
    uint32_t        uiImageSize;
    void            *buf;
    bool            returnSize;
} ImageContainerStruct;

NEXUS_Error NEXUS_BaseImage_Open(void **image, const char *filename, const char *env_path)
{
    FILE *fd = NULL;
    NEXUS_Error rc = NEXUS_SUCCESS;
    ImageContainerStruct *pImageContainer = NULL;
    long size = 0;
    char * filepath = NULL;
    size_t filepath_len = 0;

    BDBG_ASSERT(image);

    {
        const char *tmp = filename;
        while (*tmp++) {
            filepath_len++;
        }

        if (!env_path) {
            env_path = DEFAULT_PATH;
        }

       tmp = (char *)env_path;
        while (*tmp++) {
            filepath_len++;
        }

        filepath_len += 2; /* for '/' and trailing 0 */
    }

    filepath = BKNI_Malloc(filepath_len);
    if (!filepath)
    {
        BDBG_ERR(("%s - Cannot allocate buffer for file path (%d bytes)", __FUNCTION__, (unsigned)filepath_len));
        rc = NEXUS_NOT_AVAILABLE;
        goto error;
    }

    if (BKNI_Snprintf(filepath, filepath_len, "%s/%s", env_path, filename) != (int)(filepath_len-1))
    {
        BDBG_ERR(("%s - Cannot build final binary path", __FUNCTION__));
        rc = NEXUS_NOT_AVAILABLE;
        goto error;
    }

    /* Open the file */
    BDBG_MSG(("%s - Opening file %s", __FUNCTION__, filepath));
    fd = fopen(filepath, "r");
    if(fd == NULL)
    {
        BDBG_ERR(("%s - Cannot open file %s", __FUNCTION__, filepath));
        rc = NEXUS_NOT_AVAILABLE;
        goto error;
    }

    /* Get size of file */
    if(fseek(fd, 0, SEEK_END) == -1)
    {
        BDBG_ERR(("%s: Cannot get to end of file %s", __FUNCTION__, filepath));
        rc = NEXUS_NOT_AVAILABLE;
        goto error;
    }

    size = ftell(fd);
    if(size < 0){
        BDBG_ERR(("%s: Cannot get size of file %s", __FUNCTION__, filepath));
        rc = NEXUS_NOT_AVAILABLE;
        goto error;
    }

    if(fseek(fd, 0, SEEK_SET) == -1)
    {
        BDBG_ERR(("%s: Cannot get to beginning of file %s", __FUNCTION__, filepath));
        rc = NEXUS_NOT_AVAILABLE;
        goto error;
    }

    /* Allocate an image container struct */
    BDBG_MSG(("%s - Allocating memory for image container", __FUNCTION__));
    pImageContainer = (ImageContainerStruct*) BKNI_Malloc(sizeof(ImageContainerStruct));
    if(pImageContainer == NULL) {
        BDBG_ERR(("%s - Cannot allocate memory for image container", __FUNCTION__));
        rc = NEXUS_OUT_OF_SYSTEM_MEMORY;
        goto error;
    }
    BKNI_Memset(pImageContainer, 0, sizeof(ImageContainerStruct));

    /* Fill in the image container struct */
    pImageContainer->fd = fd;
    pImageContainer->filename = filepath;
    pImageContainer->uiImageSize = size;
    pImageContainer->returnSize = false;
    pImageContainer->nb_next = 0;

    /* Allocate buffer for read data */
    pImageContainer->buf = (uint8_t*) BKNI_Malloc(IMAGE_BUFFER_SIZE);
    if(pImageContainer->buf == NULL) {
        BDBG_ERR(("%s - Cannot allocate buffer for read buffer", __FUNCTION__));
        rc = NEXUS_OUT_OF_SYSTEM_MEMORY;
        goto error;
    }

    *image = pImageContainer;
    BDBG_MSG(("%s: File %s opened, file id %p size %d", __FUNCTION__, pImageContainer->filename, (void *)pImageContainer->fd, pImageContainer->uiImageSize));
    return NEXUS_SUCCESS;

error:
    if(fd) {
        fclose(fd);
    }

    if (filepath) {
        BKNI_Free(filepath);
    }

    if(pImageContainer) {
        if(pImageContainer->buf) {
            BKNI_Free(pImageContainer->buf);
        }
        BKNI_Free(pImageContainer);
    }

    return rc;
}

NEXUS_Error NEXUS_BaseImage_Next(void *image, unsigned chunk, const void **data, uint16_t length)
{
    NEXUS_Error  rc = NEXUS_SUCCESS;
    size_t read = 0;
    int ferr = 0;
    ImageContainerStruct *pImageContainer = NULL;

    BDBG_ASSERT(image);
    BDBG_ASSERT(data);
    BDBG_ASSERT(length);
    BSTD_UNUSED(chunk);

    pImageContainer = (ImageContainerStruct*)image;

    /* return size of buffer */
    if((pImageContainer->returnSize == false) && (ftell(pImageContainer->fd) == 0)) {
        *data = &pImageContainer->uiImageSize;
        pImageContainer->returnSize = true;
        BDBG_MSG(("%s - Returning size (%u bytes) of file %s to calling user", __FUNCTION__, pImageContainer->uiImageSize, pImageContainer->filename));
         goto done;
    }

    if((pImageContainer->nb_next + length) > pImageContainer->uiImageSize)  {
        BDBG_ERR(("%s: File %s size exceeded", __FUNCTION__, pImageContainer->filename));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto done;
    }

    BDBG_MSG(("%s - Reading %u bytes from file %s", __FUNCTION__, length, pImageContainer->filename));
    read = fread(pImageContainer->buf, 1, length, pImageContainer->fd);
    /* Error */
    if(read == 0)   {
        BDBG_ERR(("%s: Error reading from file. No data read", __FUNCTION__));
        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
        goto done;
    }
    else {
        if(read != length)  {
            /* Error */
            BDBG_ERR(("%s: Error reading from file %s, read: %d, length: %d", __FUNCTION__, pImageContainer->filename, (unsigned)read, length));
            if(feof(pImageContainer->fd))   {
                BDBG_ERR(("%s: EOF reached", __FUNCTION__));
            }
            else {
                ferr = ferror(pImageContainer->fd);
                if(ferr > 0) {
                    BDBG_ERR(("%s: File error %d", __FUNCTION__, ferr));
                }

            }
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto done;
        }
        else    {
            /* Good */
            pImageContainer->nb_next += read;
            BDBG_MSG(("%s - Read so far: %u bytes", __FUNCTION__, pImageContainer->nb_next));
        }
    }

    *data = pImageContainer->buf;

done:
    return rc;
}

void NEXUS_BaseImage_Close(void *image)
{
    ImageContainerStruct *pImageContainer;

    BDBG_ASSERT(image);
    pImageContainer = (ImageContainerStruct*)image;

    if(pImageContainer->fd) {
        fclose(pImageContainer->fd);
    }
    if(pImageContainer->filename) {
        BKNI_Free(pImageContainer->filename);
    }
    if(pImageContainer->buf) {
        BKNI_Free(pImageContainer->buf);
    }
    BKNI_Free(pImageContainer);

    return;
}

#endif /* NEXUS_MODE_* */

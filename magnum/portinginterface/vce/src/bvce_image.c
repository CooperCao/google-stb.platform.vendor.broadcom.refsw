/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bimg.h"
#include "bvce_image.h"
#include "bvce_image_priv.h"
#include "bafl.h"
#include "bvce_platform.h"

BDBG_MODULE(BVCE_IMAGE);

/* This file contains the reference implementation for VCE's BIMG
 * firmware retrieval interface.  It provides access to the AVD Core's
 * inner and outer ELF firmware images using the standard BIMG
 * interface.  This interface can be overridden by the application by
 * setting the pImgInterface and pImgContext pointers accordingly.  If
 * a custom interface/context is used, to save code space, you can
 * also define BVCE_USE_CUSTOM_IMAGE=1 during compilation to prevent
 * the default firmware images from being linked in.
 *
 * If a custom interface is provided, the custom BIMG_Next() call MUST
 * adhere to the following rules:
 *
 * - Chunk #0-n is the actual firmware image. The provided length can
 *   be any size, but MUST be the same for all chunks.  The caller
 *   will know how many bytes of the image is valid
 *
 */

#if !(BVCE_USE_CUSTOM_IMAGE)
static BERR_Code BVCE_IMAGE_Open(void *context,
                                 void **image,
                                 unsigned image_id)
{
        BVCE_IMAGE_ContextEntry *pContextEntry = NULL;
        BVCE_IMAGE_Container *pImageContainer = NULL;

        BDBG_ENTER(BVCE_IMAGE_Open);

        BDBG_ASSERT(context);
        BDBG_ASSERT(image);

        /* Validate the firmware ID range */
        BDBG_MSG(("Validating image ID range"));
        if (image_id >= BVCE_IMAGE_FirmwareID_Max)
        {
                BDBG_ERR(("Invalid image id %d", image_id));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        /* Validate the image referenced by firmware ID exists in the context */
        pContextEntry = ((BVCE_IMAGE_ContextEntry**)context)[image_id];

        if (pContextEntry == NULL) {
                /* A NULL entry does not necessarily mean a hard error
                 * since we could be trying to load authenticated
                 * firmware.  So, we fail silently here and expect
                 * this error to be reported by the caller depending
                 * on the situation */
                return BERR_INVALID_PARAMETER;
        }

        /* Allocate an image container struct */
        BDBG_MSG(("Allocating image container"));
        pImageContainer = (BVCE_IMAGE_Container*) BKNI_Malloc(sizeof(BVCE_IMAGE_Container));
        if (pImageContainer == NULL) {
                BDBG_ERR(("Cannot allocate image container"));
                return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }
        BKNI_Memset(pImageContainer, 0, sizeof(BVCE_IMAGE_Container));

        /* Fill in the image container struct */
        pImageContainer->uiImageSize = *(pContextEntry->puiImageSize);
        pImageContainer->pImageData = pContextEntry->pImageData;
        pImageContainer->uiImageId = image_id;

        *image = pImageContainer;

        BDBG_LEAVE(BVCE_IMAGE_Open);
        return BERR_TRACE(BERR_SUCCESS);
}

static BERR_Code BVCE_IMAGE_Next(void *image,
                                 unsigned chunk,
                                 const void **data,
                                 uint16_t length)
{
        BVCE_IMAGE_Container *pImageContainer = (BVCE_IMAGE_Container*) image;

        BDBG_ENTER(BVCE_IMAGE_Next);
        BDBG_ASSERT(image);
        BDBG_ASSERT(data);
        BSTD_UNUSED(length);

        if ( 0 != chunk )
        {
           uint32_t uiOffset = (chunk-1)*length;

           if ( uiOffset < pImageContainer->uiImageSize )
           {
              BDBG_MSG(("Returning image chunk[%d]: %d bytes of image data", chunk, length));
              *data = (void *)((uint8_t *)(pImageContainer->pImageData) + uiOffset);
           }
           else
           {
              BDBG_ERR(("Error attempt to read beyond image size"));
              return BERR_TRACE( BERR_INVALID_PARAMETER );
           }
        }
        else
        {
           BKNI_Memset( &pImageContainer->stImageHeader, 0, sizeof( pImageContainer->stImageHeader ) );

           pImageContainer->stImageHeader.uiHeaderVersion = BAFL_IMAGE_HDR_VERSION;
           pImageContainer->stImageHeader.uiDevice = BVCE_P_CoreDeviceID[pImageContainer->uiImageId];
           pImageContainer->stImageHeader.uiHeaderSize = BAFL_IMAGE_HDR_SIZE;
           pImageContainer->stImageHeader.uiImageSize = pImageContainer->uiImageSize;

           *data = (void *)((uint8_t *)(&pImageContainer->stImageHeader));
        }

        BDBG_LEAVE(BVCE_IMAGE_Next);

        return BERR_TRACE(BERR_SUCCESS);
}

static void BVCE_IMAGE_Close(void *image)
{
        /* Free the image container struct */
        BDBG_ENTER(BVCE_IMAGE_Close);
        if (image != NULL) {
                BKNI_Free(image);
        }
        BDBG_LEAVE(BVCE_IMAGE_Close);
        return;
}

const BIMG_Interface BVCE_IMAGE_Interface =
{
        BVCE_IMAGE_Open,
        BVCE_IMAGE_Next,
        BVCE_IMAGE_Close
};
#endif

/***************************************************************************
 *    Copyright (c) 2004-2009, Broadcom Corporation
 *    All Rights Reserved
 *    Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *	 This module contains the code for the XVD FW image
 *   interface.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ****************************************************************************/
#include "bstd.h"
#include "bkni.h"
#if BXVD_P_AVD_ARC600
#include "bafl.h"
#endif
#include "bxvd_image.h"
#include "bxvd_image_priv.h"

BDBG_MODULE(BXVD_IMAGE);

/* This file contains the reference implementation for XVD's BIMG
 * firmware retrieval interface.  It provides access to the AVD Core's
 * inner and outer ELF firmware images using the standard BIMG
 * interface.  This interface can be overridden by the application by
 * setting the pImgInterface and pImgContext pointers accordingly.  If
 * a custom interface/context is used, to save code space, you can
 * also define BXVD_USE_CUSTOM_IMAGE=1 during compilation to prevent
 * the default firmware images from being linked in.
 *
 * If a custom interface is provided, the custom BIMG_Next() call MUST
 * adhere to the following rules:
 *
 * - Chunk #0 ALWAYS returns a header of type BXVD_AvdImgHdr.  The
 *   length should be sizeof(BXVD_AvdImgHdr)
 *
 * - Chunk #1-n is the actual firmware image. The provided length can
 *   be any size, but MUST be the same for all chunks.  The caller
 *   will know how many bytes of the image is valid
 *
 * See
 * http://twiki-01.broadcom.com/bin/view/Bseavsw/XVDNdsSvpDesign#Updating_XVD_to_use_the_BIMG_Int
 * for details */

#if !(BXVD_USE_CUSTOM_IMAGE)

#if !BXVD_P_AVD_ARC600
#define BXVD_P_IMG_BLK0_SIZE sizeof(BXVD_AvdImgHdr)
#else
#define BXVD_P_IMG_BLK0_SIZE BAFL_IMAGE_HDR_SIZE
#endif

#if BXVD_P_USE_BINARY_IMAGE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

static BERR_Code BXVD_IMAGE_Open(void *context,
				 void **image,
				 unsigned image_id)
{
   BXVD_IMAGE_ContextEntry *pContextEntry = NULL;
   BXVD_IMAGE_Container *pImageContainer = NULL;

#if BXVD_P_USE_BINARY_IMAGE
   char strFirmwareFile[256] = "\0";
   char *pszVariable = NULL;
#endif

   BDBG_ENTER(BXVD_IMAGE_Open);

   BDBG_ASSERT(context);
   BDBG_ASSERT(image);

   /* Validate the firmware ID range */
   BDBG_MSG(("Validating image ID range"));
   if (image_id >= BXVD_IMAGE_FirmwareID_Max)
   {
      BDBG_ERR(("Invalid image id %d", image_id));
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   /* Validate the image referenced by firmware ID exists in the context */
   pContextEntry = ((BXVD_IMAGE_ContextEntry**)context)[image_id];

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
   pImageContainer = (BXVD_IMAGE_Container*) BKNI_Malloc(sizeof(BXVD_IMAGE_Container));
   if (pImageContainer == NULL) {
      BDBG_ERR(("Cannot allocate image container"));
      return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }
   BKNI_Memset(pImageContainer, 0, sizeof(BXVD_IMAGE_Container));

#if !BXVD_P_AVD_ARC600
   /* Fill in the image container struct */
   if ((image_id == BXVD_IMAGE_FirmwareID_eAuthenticated_AVD0) ||
       (image_id == BXVD_IMAGE_FirmwareID_eAuthenticated_AVD1))
   {
      /* The image data is the binary output of the svp_relocate
       * tool, where the header is followed by relocated
       * firmware.  We copy the header into our container and
       * then set the pImageData to start immediately after the
       * header */
      BKNI_Memcpy(&pImageContainer->imageHeader,
                  pContextEntry->pImageData,
                  sizeof(BXVD_AvdImgHdr));
      pImageContainer->pImageData = (void*) ((uint32_t) pContextEntry->pImageData + sizeof(BXVD_AvdImgHdr));
   }
   else
   {
      pImageContainer->imageHeader.uiImageSize = *(pContextEntry->puiImageSize);
      pImageContainer->imageHeader.uiRelocationBase = *(pContextEntry->puiImageOffset);
      pImageContainer->pImageData = pContextEntry->pImageData;
   }
#else

#if BXVD_P_USE_BINARY_IMAGE

   if (image_id == BXVD_IMAGE_FirmwareID_eOuterELF_AVD0)
   {
      pszVariable = getenv("BXVD_IMG_OUTER");
   }
   else
   {
      pszVariable = getenv("BXVD_IMG_INNER");
   }

   if ( NULL != pszVariable )
   {
      strncpy(strFirmwareFile, pszVariable, 256);
      pImageContainer->fpBinImage = fopen(strFirmwareFile, "rb");
   }
   else
   {
      pImageContainer->fpBinImage = fopen(pContextEntry->BinaryFile, "rb");
   }

   if (pImageContainer->fpBinImage == 0)
   {
      BDBG_ERR(("Could not open FW Binary File %s", pContextEntry->BinaryFile));

      return (BERR_INVALID_PARAMETER);
   }

   if ( fread(&(pImageContainer->imageHeader), 1, sizeof(BAFL_ImageHeader), pImageContainer->fpBinImage) != sizeof(BAFL_ImageHeader))
   {
      BDBG_ERR(("Could not read Image Header from Binary file"));

      return (BERR_INVALID_PARAMETER);
   }

   pImageContainer->pBlockData = BKNI_Malloc(BXVD_P_BLK_SIZE); /* The memory where the binary file is read into, a block at a time. */

   if (pImageContainer->pBlockData == NULL)
   {
      BDBG_ERR(("Cannot allocate dynamic load buffer"));
      return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }
#else
   pImageContainer->imageHeader.uiHeaderVersion = BAFL_IMAGE_HDR_VERSION;
   pImageContainer->imageHeader.uiDevice = (pContextEntry->uiDevice);

   pImageContainer->imageHeader.uiImageSize = *(pContextEntry->puiImageSize);
   pImageContainer->pImageData = pContextEntry->pImageData;
#endif
#endif
   *image = pImageContainer;

   BDBG_LEAVE(BXVD_IMAGE_Open);
   return BERR_TRACE(BERR_SUCCESS);
}

static BERR_Code BXVD_IMAGE_Next(void *image,
				 unsigned chunk,
				 const void **data,
				 uint16_t length)
{
   BXVD_IMAGE_Container *pImageContainer = (BXVD_IMAGE_Container*) image;

   BDBG_ENTER(BXVD_IMAGE_Next);
   BDBG_ASSERT(image);
   BDBG_ASSERT(data);
   BSTD_UNUSED(length);

   if (chunk == 0)
   {
      BDBG_MSG(("Returning image chunk[%d]: %d bytes of image header", chunk, length));

      /* Validate length requested is same as our header size */
      if (length != BXVD_P_IMG_BLK0_SIZE )
      {
         BDBG_ERR(("Incorrect image header length requested"));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
      }

      /* Return a pointer to the header */
      *data = &pImageContainer->imageHeader;
   }
   else
   {
      uint32_t uiOffset = (chunk-1)*length;

      if ( uiOffset >= pImageContainer->imageHeader.uiImageSize)
      {
         BDBG_ERR(("Error attempt to read beyond image size"));
         return BERR_TRACE( BERR_INVALID_PARAMETER );
      }
      else
      {
         BDBG_MSG(("Returning image chunk[%d]: %d bytes of image data", chunk, length));

#if BXVD_P_USE_BINARY_IMAGE

         fseek(pImageContainer->fpBinImage,(((chunk-1) * length) + pImageContainer->imageHeader.uiHeaderSize), SEEK_SET);

         if (fread(pImageContainer->pBlockData, 1, length, pImageContainer->fpBinImage) > BXVD_P_BLK_SIZE)
         {
            BDBG_ERR(("Binary File read failure\n"));
         }

         *data = (void *) pImageContainer->pBlockData;
#else
         *data = (void *)((uint8_t *)(pImageContainer->pImageData) + (chunk-1)*length);
#endif
      }

   }
   BDBG_LEAVE(BXVD_IMAGE_Next);

   return BERR_TRACE(BERR_SUCCESS);
}

static void BXVD_IMAGE_Close(void *image)
{
   /* Free the image container struct */
   BDBG_ENTER(BXVD_IMAGE_Close);
   if (image != NULL)
   {
#if BXVD_P_USE_BINARY_IMAGE
      BXVD_IMAGE_Container *pImageContainer = (BXVD_IMAGE_Container*) image;

      fclose(pImageContainer->fpBinImage);

      BKNI_Free(pImageContainer->pBlockData);
#endif
      BKNI_Free(image);
   }
   BDBG_LEAVE(BXVD_IMAGE_Close);
   return;
}

const BIMG_Interface BXVD_IMAGE_Interface =
{
   BXVD_IMAGE_Open,
   BXVD_IMAGE_Next,
   BXVD_IMAGE_Close
};
#endif

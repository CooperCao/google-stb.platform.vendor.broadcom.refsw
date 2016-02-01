/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "bafl.h"

BDBG_MODULE(BVCE_IMAGE_FILE);

/* This file contains the reference implementation for VCE's BIMG
 * firmware retrieval interface.  It provides access to the VCE Core's
 * picarc and mbarc ELF firmware images using the standard BIMG
 * interface.  This version loads the firmware images from a binary
 * file.  It loads from bvce_fw_image_[picarc/mbarc].bin by default.
 * If the environment variables BVCE_IMG_PICARC and/or BVCE_IMG_MBARC
 * are set, the values of the variables will be used as the image file
 * name.
 *
 * - Chunk #0-n is the actual firmware image. The provided length can
 *   be any size, but MUST be the same for all chunks.  The caller
 *   will know how many bytes of the image is valid
 *
 */

typedef struct BVCE_Image_Container
{
   FILE *hImageFile;
   BAFL_ImageHeader stImageHeader;
   uint8_t *pImageDataChunk;
   uint32_t uiImageDataChunkLength;
} BVCE_Image_Container;

static BERR_Code BVCE_IMAGE_Open(void *context,
                                 void **image,
                                 unsigned image_id)
{
   BVCE_Image_Container *pImageContainer = NULL;
   char strFirmwareFile[256] = "\0";

   BDBG_ENTER(BVCE_IMAGE_Open);

   BSTD_UNUSED(context);
   BDBG_ASSERT(image);

   /* Validate the firmware ID range */
   BDBG_MSG(("Validating image ID range"));
   if (image_id >= BVCE_IMAGE_FirmwareID_Max)
   {
           BDBG_ERR(("Invalid image id %d", image_id));
           return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   pImageContainer = BKNI_Malloc( sizeof( BVCE_Image_Container ) );

   if ( NULL == pImageContainer )
   {
      BDBG_ERR(("Error allocating [%d] image context", image_id));
      return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }

   BKNI_Memset( pImageContainer, 0, sizeof( BVCE_Image_Container ) );

   switch(image_id)
   {
      char *pszVariable = NULL;

      case BVCE_IMAGE_FirmwareID_ePicArc:
         pszVariable = getenv("BVCE_IMG_PICARC");
         if ( NULL != pszVariable )
         {
            strncpy(strFirmwareFile, pszVariable, 256);
         }
         else
         {
            strncpy(strFirmwareFile, "bvce_fw_image_picarc.bin", 256);
         }
         break;
      case BVCE_IMAGE_FirmwareID_eMBArc:
         pszVariable = getenv("BVCE_IMG_MBARC");
         if ( NULL != pszVariable )
         {
            strncpy(strFirmwareFile, pszVariable, 256);
         }
         else
         {
            strncpy(strFirmwareFile, "bvce_fw_image_mbarc.bin", 256);
         }
         break;
      default:
         BKNI_Free( pImageContainer );
         return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   pImageContainer->hImageFile = fopen(strFirmwareFile, "rb");
   if ( NULL == pImageContainer->hImageFile )
   {
      BDBG_ERR(("Error opening [%d] image file %s", image_id, strFirmwareFile));
      BKNI_Free( pImageContainer );
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   *image = pImageContainer;

   printf("IMG [%d] loaded externally from %s\n", image_id, strFirmwareFile);
   return BERR_TRACE(BERR_SUCCESS);
}

static BERR_Code BVCE_IMAGE_Next(void *image,
                                 unsigned chunk,
                                 const void **data,
                                 uint16_t length)
{
   BVCE_Image_Container *pImageContainer = ( BVCE_Image_Container*) image;

   if ( 0 != chunk )
   {
      if (pImageContainer->pImageDataChunk == NULL) {
         /* Allocate image data chunk buffer */
         pImageContainer->pImageDataChunk = (uint8_t*) BKNI_Malloc(length);
         BKNI_Memset(pImageContainer->pImageDataChunk, 0, length);
         pImageContainer->uiImageDataChunkLength = length;
      } else if (pImageContainer->uiImageDataChunkLength != length) {
         /* Verify existing buffer is the right size */
         BDBG_ERR(("Inconsistent length detected. Specified length must be same for all chunks 1-n. You provided %d but we expected %d",
              length,
              pImageContainer->uiImageDataChunkLength));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
      }

      BKNI_Memset(pImageContainer->pImageDataChunk, 0, pImageContainer->uiImageDataChunkLength);

      /* Seek to the requested chunk in the file. We need to
       * make sure we offset by the header size */
      fseek(pImageContainer->hImageFile, BAFL_IMAGE_HDR_SIZE + (chunk - 1)*length, SEEK_SET);
      fread(pImageContainer->pImageDataChunk, 1, length, pImageContainer->hImageFile);

      *data = pImageContainer->pImageDataChunk;
   }
   else
   {
      fseek(pImageContainer->hImageFile, 0, SEEK_SET);
      fread(&pImageContainer->stImageHeader, 1, length, pImageContainer->hImageFile);

      *data = &pImageContainer->stImageHeader;
   }

   return BERR_TRACE(BERR_SUCCESS);
}

static void BVCE_IMAGE_Close(void *image)
{
   BVCE_Image_Container *pImageContainer = ( BVCE_Image_Container*) image;

   /* Free the image container struct */
   if (pImageContainer != NULL) {
      if (pImageContainer->hImageFile != NULL) {
         fclose(pImageContainer->hImageFile);
      }
      if (pImageContainer->pImageDataChunk != NULL) {
         BKNI_Free(pImageContainer->pImageDataChunk);
      }
      BKNI_Free(pImageContainer);
   }
}

const BIMG_Interface BVCE_IMAGE_Interface =
{
        BVCE_IMAGE_Open,
        BVCE_IMAGE_Next,
        BVCE_IMAGE_Close
};

/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bimg.h"
#include "basp_image.h"
#include "basp_image_priv.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "bafl.h"

BDBG_MODULE(BASP_IMAGE);

static BERR_Code BASP_IMAGE_Open(
                  void *context,
                  void **image,
                  unsigned image_id
                  )
{
   BASP_IMAGE_ContextEntry *pContextEntry = NULL;
   BASP_IMAGE_Container *pImageContainer = NULL;

   BDBG_ENTER(BASP_IMAGE_Open);

   BDBG_ASSERT(context);
   BDBG_ASSERT(image);

   if (image_id >= BASP_IMAGE_FirmwareID_eMax)
   {
      BDBG_ERR(("Invalid image id %d", image_id));
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   pContextEntry = ((BASP_IMAGE_ContextEntry**)context)[image_id];

   if (pContextEntry == NULL)
   {
                /* A NULL entry does not necessarily mean a hard error
                 * since we could be trying to load authenticated
                 * firmware.  So, we fail silently here and expect
                 * this error to be reported by the caller depending
                 * on the situation */
                return BERR_INVALID_PARAMETER;
   }

   BDBG_MSG(("Allocating image container"));
   pImageContainer = (BASP_IMAGE_Container*) BKNI_Malloc(sizeof(BASP_IMAGE_Container));
   if (pImageContainer == NULL)
   {
      BDBG_ERR(("Cannot allocate image container"));
      return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }
   BKNI_Memset(pImageContainer, 0 , sizeof(BASP_IMAGE_Container));


   /* Fill in the image container struct */
   pImageContainer->uiImageSize = *(pContextEntry->puiImageSize);
   pImageContainer->pImageData = pContextEntry->pImageData;
   pImageContainer->uiImageId = image_id;

   *image = pImageContainer;

   BDBG_LEAVE(BASP_IMAGE_Open);
   return BERR_TRACE(BERR_SUCCESS);
}

static BERR_Code BASP_IMAGE_Next(
                 void *image,
                 unsigned chunk,
                 const void **data,
                 uint16_t length
                 )
{
   BASP_IMAGE_Container *pImageContainer = (BASP_IMAGE_Container*) image;

   BDBG_ENTER(BASP_IMAGE_Next);
   BDBG_ASSERT(image);
   BDBG_ASSERT(data);
   BSTD_UNUSED(length);

   if (0 != chunk)
   {
      uint32_t ui32Offset = (chunk - 1)*length;

      if ( ui32Offset < pImageContainer->uiImageSize )
      {
         BDBG_MSG(("Returning image chunk[%d]: %d bytes of image data", chunk, length));
         *data = (void *)((uint8_t *)(pImageContainer->pImageData) + ui32Offset);
      }
      else
      {
         BDBG_ERR(("Error attempt to read beyond image size whereas image size is 0x%x", (unsigned)pImageContainer->uiImageSize));
         return BERR_TRACE( BERR_INVALID_PARAMETER );
      }
   }
   else
   {
      BKNI_Memset( &pImageContainer->stImageHeader, 0, sizeof( pImageContainer->stImageHeader ) );

      pImageContainer->stImageHeader.uiHeaderVersion = BAFL_IMAGE_HDR_VERSION;
      /* pImageContainer->stImageHeader.uiDevice; TODO: Later we will decide how to get the Device Id.*/
      pImageContainer->stImageHeader.uiHeaderSize = BAFL_IMAGE_HDR_SIZE;
      pImageContainer->stImageHeader.uiImageSize = pImageContainer->uiImageSize;

      *data = (void *)((uint8_t *)(&pImageContainer->stImageHeader));
   }

   BDBG_LEAVE(BASP_IMAGE_Next);

   return BERR_TRACE(BERR_SUCCESS);
}

static void BASP_IMAGE_Close(void *image)
{
        /* Free the image container struct */
        BDBG_ENTER(BASP_IMAGE_Close);
        if (image != NULL)
        {
           BKNI_Free(image);
        }
        BDBG_LEAVE(BASP_IMAGE_Close);
        return;
}

const BIMG_Interface BASP_IMAGE_Interface =
{
   BASP_IMAGE_Open,
   BASP_IMAGE_Next,
   BASP_IMAGE_Close
};

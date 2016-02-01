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
 *
 ******************************************************************************/

#include "bstd.h"
#include "bdbg.h"
#include "bimg.h"
#include "bsid_img.h"

/* This file contains the reference implementation for SID's BIMG
 * firmware retrieval interface.  It provides access to the SID Arc's
 * firmware image using the standard BIMG interface.
 * This interface can be overridden by the application by
 * setting the pImgInterface and pImgContext pointers accordingly.
 * If a custom interface/context is used, to save code space, you can
 * also define BSID_USE_CUSTOM_IMAGE=1 during compilation to prevent
 * the default firmware image from being linked in.
 *
 * If a custom interface is provided, the custom BIMG_Next() call MUST
 * adhere to the following rules:
 *
 * - Chunk #0 is reserved for the image header.  Since SID currently
 *   has no image header, the next must ignore this (although the
 *   current loader will never ask for chunk 0)
 * - Chunks 1-N are expected to be the firmware blocks
 */
BDBG_MODULE(BSID_IMG);

#if !(BSID_USE_CUSTOM_IMAGE)

static BERR_Code BSID_ImageOpen(void *pContext, void **ppImage, unsigned uiImageId);
static BERR_Code BSID_ImageNext(void *pImage, unsigned uiChunkId, const void **ppData, uint16_t uiLength);
static void BSID_ImageClose(void *pImage);

const BIMG_Interface BSID_ImageInterface =
{
    BSID_ImageOpen,
    BSID_ImageNext,
    BSID_ImageClose
};

typedef struct BSID_P_ImageContext
{
   const void * const *pFWData;
   const uint32_t *pChunkSize;
   const uint32_t *pNumChunks;
} BSID_P_ImageContext;

static const BSID_P_ImageContext g_Context =
{
   BSID_FW_Sid,
   &BSID_FW_Sid_ChunkSize,
   &BSID_FW_Sid_NumEntries
};

const void* const BSID_ImageContext = (void *)&g_Context;

/* NOTE: since SID only has one FW image to load, there is only one valid Image ID */
static BERR_Code BSID_ImageOpen(void *pContext, void **ppImage, unsigned uiImageId)
{
    BDBG_ENTER(BSID_ImageOpen);

    BDBG_ASSERT(pContext);
    BDBG_ASSERT(ppImage);

    *ppImage = pContext;

    /* Validate the firmware ID is correct (just a sanity check) */
    BDBG_MSG(("Validating image ID"));
    if (uiImageId != BSID_IMG_FIRMWARE_ID)
    {
        BDBG_ERR(("Invalid image id %d", uiImageId));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* NOTE: SID uses the interface's context directly - it does not
       need a per-image context */
    if (pContext != (void *)&g_Context)
    {
        BDBG_ERR(("Invalid Image Interface Context for built-in IMG API"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_LEAVE(BSID_ImageOpen);
    return BERR_SUCCESS;
}

/* NOTE: Chunk 0 is reserved for future header support.  It is not used here, and is skipped if
   supplied (i.e. it will do nothing & return a NULL pointer in *ppData) */
/* This will simply supply the next array entry that is part of bsid_fw.c
   uiLength must always be BSID_IMG_CHUNK_SIZE or sizeof(BSID_IMG_Sid_0) */
static BERR_Code BSID_ImageNext(void *pImage, unsigned uiChunkId, const void **ppData, uint16_t uiLength)
{
    BSID_P_ImageContext *pContext = (BSID_P_ImageContext *)pImage;
    BDBG_ENTER(BSID_ImageNext);

    BDBG_ASSERT(ppData);
    *ppData = NULL;

    if (pContext != &g_Context)
    {
       BDBG_ERR(("Invalid Image Context"));
       return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if (0 != uiChunkId)
    {
       unsigned uiIndex = uiChunkId-1;
       if (uiLength != *(pContext->pChunkSize))
       {
           BDBG_ERR(("This IMG API requires chunk size of: %d", *(pContext->pChunkSize)));
           return BERR_TRACE(BERR_INVALID_PARAMETER);
       }
       /* retrieve the "block" arrays from bsid_fw.c and provide the start address to caller */
       BDBG_MSG(("Returning image chunk [%d]: %d bytes of image data", uiIndex, uiLength));
       if (uiIndex < *(pContext->pNumChunks))
       {
          *ppData = (void *)(pContext->pFWData)[uiIndex];
       }
       else
       {
          BDBG_ERR(("Attempt to access non-existent chunk [%d]", uiIndex));
          return BERR_TRACE(BERR_INVALID_PARAMETER);
       }
    }
    else
    {
       /* Skip the chunk 0 - reserved for (future) header, which SID does not make use of */
       BDBG_WRN(("Attempt to retrieve chunk 0 (header) - not supported (ignored)"));
    }

    BDBG_LEAVE(BSID_ImageNext);

    return BERR_SUCCESS;
}

static void BSID_ImageClose(void *pImage)
{
    BDBG_ENTER(BSID_ImageClose);
    if (pImage != (void *)&g_Context)
    {
       BDBG_ERR(("Invalid Image Context"));
    }
    /* Nothing to do - no context used */
    BDBG_LEAVE(BSID_ImageClose);
    return;
}
#endif /* BSID_USE_CUSTOM_IMAGE */

/***************************
   End of File
****************************/

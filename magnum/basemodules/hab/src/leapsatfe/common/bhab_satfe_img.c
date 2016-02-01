/******************************************************************************
*    (c)2011-2013 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Module Description:
*
* Revision History:
*
* $brcm_Log: $
*
*****************************************************************************/
#include "bhab.h"
#include "bimg.h"

#include "bhab_satfe_img.h"

BDBG_MODULE(bhab_satfe_img);

typedef struct bhab_satfe_img_context {
    void *context; /* Initial IMG context from open */
    unsigned current_segment;
    uint8_t constructed_header[MAX_SATFE_IMG_QUERY_SIZE];
} bhab_satfe_img_context;

/******************************************************************************
 BHAB_SATFE_IMG_Open()
******************************************************************************/
static BERR_Code BHAB_SATFE_IMG_Open(void *context, void **image, unsigned image_id)
{
   bhab_satfe_img_context *img_context;
   BSTD_UNUSED(image_id);
   img_context = (bhab_satfe_img_context *)BKNI_Malloc(sizeof(*img_context));
   if (!img_context)
       return BERR_OUT_OF_SYSTEM_MEMORY;
   BKNI_Memset(img_context, 0, sizeof(*img_context));
   img_context->context = context;
   *image = img_context;
   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_SATFE_IMG_Next()
******************************************************************************/

/* Satellite firmware is in segments, and num_segments is not necessarily 1. 4538 is in two segments, for instance.
 *
 * IMG_Context for satellite firmware is defined as an array of two void*, e.g.:
 *     const void *BHAB_4538_IMG_Context[2] = {(void*)bcm4538_ap_image_segments, (void*)bcm4538_ap_image};
 *
 * The first pointer is defined as:
 *   { header_size, num_segments, segment_offset[0], segment_offset[1]... }
 *
 * The second pointer is the firmware array.
 *
 * Because of the semantics of IMG's next (chunk, size, and context are all we have), the interface for the satellite firmware function works like this:
 *
 * If chunk==0 && length == MAX_SATFE_IMG_QUERY_SIZE, return an arbitrary header constructed from the context.
 * This header is defined as:
 * { uint8_t header_size, uint8_t num_segments, 0, 0... }
 *
 * If chunk<num_segments && length == header_size, return header_size bytes from matching segment.
 * IMG uses this to reset to the next chunk and stores the information locally to know which segment is being fetched.
 *
 * ...else return length bytes from segment pointer + chunk*32768
 *
 * The caller should call with 0/16 to fetch number of segments, then fetch each segment with a header read, followed by reading the successive chunks.
 * Each segment must begin with a header read for that segment to reset the information on which segment is being read.
 *
 * (In essence, this all works around the semantics of next, which are not length+offset, and that the firmware segments are often larger than the max returnable size (64k-4).)
 *
 * MAX_SATFE_IMG_QUERY_SIZE and MAX_SATFE_IMG_CHUNK_SIZE are defined in bhab_satfe_img.h to allow tweaking as necessary.
 * For instance, if a segment header in the future requires 16 bytes, MAX_SATFE_IMG_QUERY_SIZE can be changed to 14, or 12, or 32 by changing the define to keep the first two cases unambiguous.
 * Similarly, the two values can be balanced specifically to work around a segment which has a tail which could be mistaken for a valid segment begin call.
 */

static BERR_Code BHAB_SATFE_IMG_Next(void *image, unsigned chunk, const void **data, uint16_t length)
{
   const uint8_t *pData;
   bhab_satfe_img_context *pImgContext;
   uint32_t *pContext;
   unsigned header_size, num_segments;
   unsigned chunk_size = MAX_SATFE_IMG_CHUNK_SIZE;

   pImgContext = (bhab_satfe_img_context *)image;
   pContext = (uint32_t *)((void**)pImgContext->context)[0];
   header_size = pContext[0];
   num_segments = pContext[1];
   pData = (const uint8_t *)((void**)pImgContext->context)[1];

   if (chunk==0 && length==MAX_SATFE_IMG_QUERY_SIZE) {
       BKNI_Memset(pImgContext->constructed_header, 0, MAX_SATFE_IMG_QUERY_SIZE);
       pImgContext->constructed_header[0] = (uint8_t)header_size;
       pImgContext->constructed_header[1] = (uint8_t)num_segments;
       *data = pImgContext->constructed_header;
   } else if (chunk<num_segments && length==header_size) {
       pData += pContext[2+chunk];
       pImgContext->current_segment = chunk;
       *data = pData;
   } else {
       pData += pContext[2+pImgContext->current_segment];
       pData += header_size;
       pData += chunk * chunk_size;
       *data = pData;
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_SATFE_IMG_Close()
******************************************************************************/
static void BHAB_SATFE_IMG_Close(void *image)
{
   BKNI_Free(image);
   return;
}


const BIMG_Interface BHAB_SATFE_IMG_Interface = {
   BHAB_SATFE_IMG_Open,
   BHAB_SATFE_IMG_Next,
   BHAB_SATFE_IMG_Close
};

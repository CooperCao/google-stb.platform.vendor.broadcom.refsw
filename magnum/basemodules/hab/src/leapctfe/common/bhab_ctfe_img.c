/******************************************************************************
*    (c)2016 Broadcom Corporation
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

#include "bhab_ctfe_img.h"

BDBG_MODULE(bhab_ctfe_img);

/******************************************************************************
 BHAB_CTFE_IMG_Open()
******************************************************************************/
static BERR_Code BHAB_CTFE_IMG_Open(void *context, void **image, unsigned image_id)
{
    BSTD_UNUSED(image_id);
    *image = context;
    return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_CTFE_IMG_Next()
******************************************************************************/

/* Cable/Terrestrial firmware is a single hex array.
 *
 * Return length bytes, starting at MAX_CTFE_IMG_CHUNK_SIZE*chunk.
 *
 * Caller is assumed to be able to properly calculate the array length.
 *
 * Caller can call the same chunk more than once, e.g. reading only the first X bytes to read the data necessary to calculate the length...
 */

static BERR_Code BHAB_CTFE_IMG_Next(void *image, unsigned chunk, const void **data, uint16_t length)
{
    const uint8_t *pData;
    unsigned chunk_size = MAX_CTFE_IMG_CHUNK_SIZE;

    BSTD_UNUSED(length);

    pData = (const uint8_t *)image;

    pData += chunk * chunk_size;
    *data = pData;

    return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_CTFE_IMG_Close()
******************************************************************************/
static void BHAB_CTFE_IMG_Close(void *image)
{
    BSTD_UNUSED(image);
    return;
}


const BIMG_Interface BHAB_CTFE_IMG_Interface = {
   BHAB_CTFE_IMG_Open,
   BHAB_CTFE_IMG_Next,
   BHAB_CTFE_IMG_Close
};

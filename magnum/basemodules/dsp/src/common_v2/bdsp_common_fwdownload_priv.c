/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*****************************************************************************/

#include "bdsp_common_priv_include.h"

BDBG_MODULE(bdsp_common_fwdownload);

BERR_Code BDSP_P_GetFWSize (
    const BIMG_Interface *iface,
    void *context,
    uint32_t firmware_id,
    uint32_t *size
)
{
    void *image;
    BERR_Code rc = BERR_SUCCESS;
    const void *data;
    uint32_t ui32DataSize = 0;

    BDBG_ASSERT(NULL != iface);
    BDBG_ASSERT(NULL != context);

    *size=0;
    rc = iface->open (context, &image, firmware_id);

    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Error in Opening the Image Interface for FW_ID =%d ",firmware_id));
        return BERR_TRACE(rc);
    }

    rc = iface->next(image, 0, &data, 8);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Error in fetching next chunk in Image Interface"));
        iface->close(image);
        return BERR_TRACE(rc);
    }

    ui32DataSize = ((uint32_t *) data)[0];

    *size = BDSP_ALIGN_SIZE(ui32DataSize,32);

    iface->close(image);
    return rc;
}

BERR_Code BDSP_P_CopyFWImageToMem(
    const BIMG_Interface *iface,
    void                 *pImgContext,
    BDSP_MMA_Memory      *pMemory,
    unsigned              firmware_id
)
{
    void *image = NULL;
    const void *data = NULL;
    void *context = pImgContext;
    uint8_t *pMemAddr;

    uint32_t ui32Size = 0, ui32numOfChunks = 0,ui32ChunkLen = 0;
    uint32_t ui32Count = 0;
    uint32_t uiSizeCopied=0;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ASSERT(iface);
    BDBG_ASSERT(pImgContext);

     rc = iface->open(context, &image, firmware_id);
    if (rc != BERR_SUCCESS)
    {
      BDBG_ERR(("Error in Opening the Image Interface"));
      return BERR_TRACE(rc);
    }

    rc = iface->next(image, 0, &data, 8);
    if (rc != BERR_SUCCESS)
    {
      BDBG_ERR(("Error in fetching next chunk in Image Interface"));
      iface->close(image);
      return BERR_TRACE(rc);
    }

    ui32Size =((uint32_t *) data)[0];
    ui32numOfChunks = ((uint32_t *) data)[1];
    pMemAddr = (uint8_t *)pMemory->pAddr;

    BDBG_MSG(("Downloading Algorithm at PhyAddr = "BDSP_MSG_FMT,BDSP_MSG_ARG(pMemory->offset)));

    BDBG_MSG(("Total Size = %d",ui32Size));
    for (ui32Count = 1;ui32Count <= ui32numOfChunks; ui32Count++)
    {
      /* The implementation of image interface has to be such that there is
        one header array and then there are firmware binary arrays each having
        bytes of size BDSP_IMG_CHUNK_SIZE. But the last array most probably will
        not have a size equal to exactly BDSP_IMG_CHUNK_SIZE. So we are testing
        for the last chunk here and getting the size based on the total firmware
        binary size and number of chunks*/

      ui32ChunkLen = (ui32Count == ui32numOfChunks) ?  \
          (ui32Size - ((ui32Count - 1)*BDSP_IMG_CHUNK_SIZE)): BDSP_IMG_CHUNK_SIZE;

      BDBG_ASSERT(ui32ChunkLen <= BDSP_IMG_CHUNK_SIZE);
      BDBG_MSG(("ui32Count = %d, ui32numOfChunks = %d , ui32ChunkLen =%d,pAddress = %p",
                ui32Count,ui32numOfChunks,ui32ChunkLen,pMemAddr));

      rc = iface->next(image, ui32Count, &data, ui32ChunkLen);
      if (rc != BERR_SUCCESS)
      {
          BDBG_ERR(("Error in fetching next chunk in Image Interface"));;
          iface->close(image);
          return BERR_TRACE(rc);
      }

      BKNI_Memcpy((void *)pMemAddr,data,ui32ChunkLen);

      pMemAddr +=ui32ChunkLen;
      uiSizeCopied +=  ui32ChunkLen;
    }

    if(uiSizeCopied != ui32Size)
    {
      BDBG_ERR(("FW Image (Id =%#x) not downloaded properly",firmware_id));
    }

    iface->close(image);
    return BERR_SUCCESS;
}

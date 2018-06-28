/***************************************************************************
*  Copyright (C) 2018 Broadcom.
*  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to
*  the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied),
*  right to use, or waiver of any kind with respect to the Software, and
*  Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
*  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
*  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization,
*  constitutes the valuable trade secrets of Broadcom, and you shall use all
*  reasonable efforts to protect the confidentiality thereof, and to use this
*  information only in connection with your use of Broadcom integrated circuit
*  products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
*  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
*  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
*  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
*  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
*  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
*  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
*  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
*  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
*  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
* API Description:
*   API name: FMT
*    Audio Format Related Routines
*
***************************************************************************/
#include "bape_buffer_filter_priv.h"
#include "bott_dma.h"

BDBG_MODULE(bape_buffer_filter_priv);

typedef struct BAPE_BufferGroupFilter {
    BAPE_Handle hDevice;
    BAPE_BufferGroupFilterFn *pFunc;
    void *ctx;
    BAPE_BufferGroupHandle hUpstreamBG;
    BAPE_BufferGroupHandle hBG;
    unsigned lowWater;
} BAPE_BufferGroupFilter;

BAPE_BufferGroupFilterHandle BAPE_BufferGroupFilter_P_Open(BAPE_Handle hDevice, BAPE_BufferGroupHandle hUpstreamBG, unsigned bufferSize, BAPE_BufferGroupFilterFn *pFunc, void *ctx)
{
    BAPE_BufferGroupFilterHandle hFilter;
    BERR_Code rc;
    BAPE_BufferGroupOpenSettings settings;

    hFilter = BKNI_Malloc(sizeof(*hFilter));
    if (hFilter == 0) {
        return NULL;
    }
    BKNI_Memset(hFilter, 0, sizeof(*hFilter));

    BAPE_BufferGroup_GetDefaultOpenSettings(&settings);
    settings.interleaved = true;
    settings.numChannels = 1;
    settings.bufferSize = bufferSize;
    rc = BAPE_BufferGroup_Open(hDevice, &settings, &hFilter->hBG);
    if (rc != BERR_SUCCESS) {
        BKNI_Free(hFilter);
        return NULL;
    }
    hFilter->lowWater = settings.bufferSize;
    hFilter->pFunc = pFunc;
    hFilter->ctx = ctx;
    hFilter->hUpstreamBG = hUpstreamBG;
    hFilter->hDevice = hDevice;
    return hFilter;
}

void
BAPE_BufferGroupFilter_P_Close(BAPE_BufferGroupFilterHandle hFilter)
{
    BAPE_BufferGroup_Close(hFilter->hBG);
    BKNI_Free(hFilter);
}

BERR_Code BAPE_BufferGroupFilter_P_Enable_isr(BAPE_BufferGroupFilterHandle hFilter, bool enable)
{
    return BAPE_BufferGroup_Enable_isr(hFilter->hBG, enable);
}

BERR_Code BAPE_BufferGroupFilter_P_Read_isr(BAPE_BufferGroupFilterHandle hFilter, BAPE_BufferDescriptor *pGroupDescriptor)
{
    BERR_Code rc;
    rc = BAPE_BufferGroup_Read_isr(hFilter->hBG, pGroupDescriptor);
    if (rc != BERR_SUCCESS) {
        return BERR_TRACE(rc);
    }
    if (pGroupDescriptor->bufferSize + pGroupDescriptor->wrapBufferSize < hFilter->lowWater) {
        BAPE_BufferDescriptor upstreamDesc, desc;
        rc = BAPE_BufferGroup_Read_isr(hFilter->hUpstreamBG, &upstreamDesc);
        if (rc != BERR_SUCCESS) {
            return BERR_TRACE(rc);
        }
        rc = BAPE_BufferGroup_GetWriteBuffers_isr(hFilter->hBG, &desc);
        if (rc != BERR_SUCCESS) {
            return BERR_TRACE(rc);
        }
        if (upstreamDesc.bufferSize) {
            unsigned inSize = 0, outSize = 0;
            (*hFilter->pFunc)(hFilter->ctx, &upstreamDesc,
                              upstreamDesc.buffers[0].pBuffer, upstreamDesc.bufferSize,
                              upstreamDesc.buffers[0].pWrapBuffer, upstreamDesc.wrapBufferSize,
                              desc.buffers[0].pBuffer, desc.bufferSize,
                              desc.buffers[0].pWrapBuffer, desc.wrapBufferSize,
                              &inSize, &outSize);
            if (inSize) {
                BAPE_BufferGroup_ReadComplete_isr(hFilter->hUpstreamBG, inSize);
            }
            if (outSize) {
                BAPE_BufferGroup_WriteComplete_isr(hFilter->hBG, outSize);
                rc = BAPE_BufferGroup_Read_isr(hFilter->hBG, pGroupDescriptor);
            }
        }
    }
    return rc;
}

BERR_Code BAPE_BufferGroupFilter_P_ReadComplete_isr(BAPE_BufferGroupFilterHandle hFilter, size_t size)
{
    return BAPE_BufferGroup_ReadComplete_isr(hFilter->hBG, size);
}

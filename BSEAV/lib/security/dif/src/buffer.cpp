/******************************************************************************
 *    (c)2015 Broadcom Corporation
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
 * Module Description:
 *
 * DRM Integration Framework
 *
 *****************************************************************************/
#include "streamer.h"
#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

BDBG_MODULE(buffer);
#undef LOGE
#undef LOGW
#undef LOGD
#undef LOGV
#define LOGE BDBG_ERR
#define LOGW BDBG_WRN
#define LOGD BDBG_MSG
#define LOGV BDBG_MSG

using namespace dif_streamer;

Buffer::~Buffer()
{
    if (!m_givenBuffer) {
        NEXUS_Memory_Free(m_data);
        m_data = NULL;
    }
}

bool Buffer::Initialize()
{
    if (m_data == NULL) {
        uint8_t *pBuf = NULL;

        NEXUS_MemoryAllocationSettings memSettings;
        NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
#ifdef NXCLIENT_SUPPORT
        NEXUS_ClientConfiguration clientConfig;
        NEXUS_Platform_GetClientConfiguration(&clientConfig);
        memSettings.heap = clientConfig.heap[NXCLIENT_FULL_HEAP];
#else
        memSettings.heap = NEXUS_MEMC0_MAIN_HEAP;
#endif

        NEXUS_Memory_Allocate(m_size, &memSettings, (void **)&pBuf);

        if (pBuf == NULL) {
            LOGE(("%s: Memory not allocated from Nexus heap", __FUNCTION__));
            return false;
        }
        m_data = pBuf;
    }

    return true;
}

void Buffer::Copy(uint32_t offset, uint8_t* dataToCopy, size_t size)
{
    BKNI_Memcpy(m_data + offset, dataToCopy, size);
}

void Buffer::Copy(uint32_t offset, IBuffer* bufToCopy, size_t size)
{
    if(bufToCopy->IsSecure()) {
        LOGE(("%s: not allowed to copy from secure to unsecure", __FUNCTION__));
    } else {
        BKNI_Memcpy(m_data + offset, bufToCopy->GetPtr(), size);
    }
}

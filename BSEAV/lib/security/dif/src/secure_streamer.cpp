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

 ******************************************************************************/
#include "sage_srai.h"

#include "streamer.h"

BDBG_MODULE(secure_streamer);
#undef LOGE
#undef LOGW
#undef LOGD
#undef LOGV
#define LOGE BDBG_ERR
#define LOGW BDBG_WRN
#define LOGD BDBG_MSG
#define LOGV BDBG_MSG

#ifdef USE_SECURE_PLAYBACK
using namespace dif_streamer;

SecureStreamer::SecureStreamer()
    : BaseStreamer()
{
    m_dmaHandle = NULL;
    m_dmaJob = NULL;
    LOGD(("SecureStreamer was created: %p", (void*)this));
}

bool SecureStreamer::Initialize()
{
    m_dmaHandle = NEXUS_Dma_Open(NEXUS_ANY_ID, NULL);

    if (m_dmaHandle == NULL) {
        LOGE(("Failed to NEXUS_Dma_Open !!!"));
        return false;
    }

    NEXUS_DmaJobSettings dmaJobSettings;
    NEXUS_DmaJob_GetDefaultSettings(&dmaJobSettings);
    dmaJobSettings.completionCallback.callback = NULL;
    dmaJobSettings.bypassKeySlot = NEXUS_BypassKeySlot_eGR2R;
    m_dmaJob = NEXUS_DmaJob_Create(m_dmaHandle, &dmaJobSettings);

    if (m_dmaJob == NULL) {
        LOGE(("Failed to NEXUS_DmaJob_Create !!!"));
        return false;
    }

    return true;
}

SecureStreamer::~SecureStreamer()
{
    LOGD(("SecureStreamer::%s %p enter", __FUNCTION__, (void*)this));
    if (m_playpump) {
        NEXUS_Playpump_Flush(m_playpump);
        NEXUS_Playpump_ClosePidChannel(m_playpump, m_pidChannel);
        NEXUS_Playpump_Stop(m_playpump);
        NEXUS_Playpump_Close(m_playpump);
        m_playpump = NULL;
    }

    if (m_playpumpBuffer) {
        BufferFactory::DestroyBuffer(m_playpumpBuffer);
        m_playpumpBuffer = NULL;
    }

    if (m_dmaJob) {
        NEXUS_DmaJob_Destroy(m_dmaJob);
        m_dmaJob = NULL;
    }

    if (m_dmaHandle) {
        NEXUS_Dma_Close(m_dmaHandle);
        m_dmaHandle = NULL;
    }
}

bool SecureStreamer::SetupPlaypump(
    NEXUS_PlaypumpOpenSettings &playpumpOpenSettings)
{
    playpumpOpenSettings.dataNotCpuAccessible = true;
    m_playpumpBuffer = BufferFactory::CreateBuffer(
        playpumpOpenSettings.fifoSize, NULL, true);
    if (m_playpumpBuffer == NULL ) {
        LOGE(("Failed to allocate from Secure Video heap"));
        return false;
    }

    playpumpOpenSettings.memory =
        NEXUS_MemoryBlock_FromAddress(m_playpumpBuffer->GetPtr());

    return true;
}

bool SecureStreamer::SetupPidChannel()
{
    NEXUS_SetPidChannelBypassKeyslot(m_pidChannel, NEXUS_BypassKeySlot_eGR2R);
    return true;
}

IBuffer* SecureStreamer::CreateBuffer(uint32_t size, uint8_t* data)
{
    SecureBuffer* buffer = reinterpret_cast<SecureBuffer*>(
        BufferFactory::CreateBuffer(size, data, true));
    if (buffer == NULL) {
        LOGE(("%s: failed to create secure buffer", __FUNCTION__));
        return NULL;
    }
    buffer->SetDmaJob(m_dmaJob);
    return buffer;
}
#endif

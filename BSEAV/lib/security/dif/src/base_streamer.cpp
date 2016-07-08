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

BDBG_MODULE(base_streamer);
#undef LOGE
#undef LOGW
#undef LOGD
#undef LOGV
#define LOGE BDBG_ERR
#define LOGW BDBG_WRN
#define LOGD BDBG_MSG
#define LOGV BDBG_MSG

using namespace dif_streamer;

BaseStreamer::BaseStreamer()
{
    m_internallyPushed = 0;
    m_offset = 0;
    m_playpump = NULL;
    m_playpumpBuffer = NULL;
    m_pidChannel = NULL;
}

NEXUS_PlaypumpHandle BaseStreamer::OpenPlaypump(bool isVideo)
{
    if (m_playpump != NULL) {
        LOGW(("Playpump already opened"));
        return m_playpump;
    }

    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    if (isVideo) {
        playpumpOpenSettings.fifoSize *= DEFAULT_VIDEO_MULTIPLE;
        playpumpOpenSettings.numDescriptors *= DEFAULT_VIDEO_MULTIPLE;
    }

    if (SetupPlaypump(playpumpOpenSettings) == false) {
        LOGE(("Failed to set up playpump"));
        return NULL;
    }

    m_playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
    if (!m_playpump) {
        LOGE(("@@@ Playpump Open FAILED----"));
    }
    return m_playpump;
}

NEXUS_PidChannelHandle BaseStreamer::OpenPidChannel(unsigned pid,
    const NEXUS_PlaypumpOpenPidChannelSettings *pSettings)
{
    if (m_playpump == NULL) {
        LOGE(("Playpump hasn't been opened"));
        return NULL;
    }

    if (m_pidChannel != NULL) {
        LOGW(("PidChannel already opened"));
        return m_pidChannel;
    }

    m_pidChannel = NEXUS_Playpump_OpenPidChannel(m_playpump, pid, pSettings);
    if (SetupPidChannel() == false) {
        LOGE(("Failed to set up pid channel"));
        return NULL;
    }

    return m_pidChannel;
}

IBuffer* BaseStreamer::GetBuffer(size_t size)
{
    if (m_playpump == NULL) {
        LOGE(("Playpump hasn't been opened"));
        return NULL;
    }

    uint8_t *playpumpBuffer;
    size_t bufferSize;
    for (;;) {
        size_t fragment_size = m_offset + size;
        NEXUS_Playpump_GetBuffer(m_playpump, (void**)&playpumpBuffer, &bufferSize);
        if (bufferSize == 0) {
            BKNI_Sleep(100);
            NEXUS_Playpump_GetBuffer(m_playpump, (void**)&playpumpBuffer, &bufferSize);
        }

        if (bufferSize >= fragment_size) {
            break;
        }

        if (m_offset > 0) {
            LOGD(("%s: internally push %u", __FUNCTION__, m_offset));
            NEXUS_Playpump_WriteComplete(m_playpump, 0, m_offset);
            m_internallyPushed += m_offset;
            m_offset = 0;
            continue;
        }

        if (bufferSize > 0) {
            LOGD(("%s: skip bufferSize=%u fragment_size=%u", __FUNCTION__, bufferSize, fragment_size));
            NEXUS_Playpump_WriteComplete(m_playpump, bufferSize, 0);
        }
    }

    IBuffer* buf = CreateBuffer(size, playpumpBuffer + m_offset);
    m_offset += size;

    return buf;
}

bool BaseStreamer::Push(uint32_t size)
{
    if (m_playpump == NULL) {
        LOGE(("Playpump hasn't been opened"));
        return false;
    }

    LOGD(("SecureStreamer::%s size=%u", __FUNCTION__, size));
    NEXUS_Error rc;
    if (size > m_internallyPushed) {
        rc = NEXUS_Playpump_WriteComplete(m_playpump, 0, size - m_internallyPushed);
        m_internallyPushed = 0;
    } else
        LOGW(("SecureStreamer::%s warning size=%u < internallyPushed=%u", __FUNCTION__, size, m_internallyPushed));

    m_offset = 0;
    return (rc == NEXUS_SUCCESS);
}

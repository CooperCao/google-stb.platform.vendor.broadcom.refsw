/***************************************************************************
 *     (c)2005-2013 Broadcom Corporation
 *  
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 **********************************************************************/
#include "nexus_transport_module.h"

#include "bchp_xpt_xcbuff.h"
#include <stdio.h>

BDBG_MODULE(nexus_xc_capture);

#define BDBG_MSG_TRACE(X) /* BDBG_MSG(X) */

typedef struct NEXUS_TransportClientCapture_P_OffsetMap
{
    uint32_t base;      /* Offset of the BASE register */
    uint32_t read;      /* Offset of the READ register */
    uint32_t valid;     /* Offset of the VALID register */
    uint32_t end;       /* Offset of the END register */
} NEXUS_TransportClientCapture_P_OffsetMap;

#define XC_REG_PTR_OFFSET(NAME) BCHP_XPT_XCBUFF_##NAME##_POINTER_RAVE_IBP0
#define XC_REG_PARSER_BLOCK_LENGTH (BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_IBP1 - BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_IBP0)
#define XC_REG_PB_OFFSET (BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_PBP0 - BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_IBP0)
#define XC_REG_RAVE_OFFSET (0)
#define XC_REG_MSG_OFFSET (BCHP_XPT_XCBUFF_BASE_POINTER_MSG_IBP0 - BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_IBP0)
#define XC_REG_RMX_OFFSET (BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_IBP0 - BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_IBP0)
#define XC_REG_CLIENT_BLOCK_LENGTH (BCHP_XPT_XCBUFF_BASE_POINTER_RMX1_IBP0 - BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_IBP0)

static void NEXUS_TransportClientCapture_P_GetOffsetMap(bool playback, unsigned parserIndex, NEXUS_TransportClientType clientType, unsigned clientIndex, NEXUS_TransportClientCapture_P_OffsetMap * pMap)
{
    uint32_t parserTypeOffset;
    uint32_t parserIndexOffset;
    uint32_t clientTypeOffset;
    uint32_t clientIndexOffset;

    BDBG_ASSERT(pMap);

    parserTypeOffset = playback ? XC_REG_PB_OFFSET : 0;
    parserIndexOffset = parserIndex * XC_REG_PARSER_BLOCK_LENGTH;

    switch (clientType)
    {
        case NEXUS_TransportClientType_eRave:
            clientTypeOffset = XC_REG_RAVE_OFFSET;
            break;
        case NEXUS_TransportClientType_eMessage:
            clientTypeOffset = XC_REG_MSG_OFFSET;
            break;
        case NEXUS_TransportClientType_eRemux:
            clientTypeOffset = XC_REG_RMX_OFFSET;
            break;
        default:
            break;
    }

    clientIndexOffset = clientIndex * XC_REG_CLIENT_BLOCK_LENGTH;

    pMap->base = XC_REG_PTR_OFFSET(BASE) + parserTypeOffset + parserIndexOffset + clientTypeOffset + clientIndexOffset;
    pMap->read = XC_REG_PTR_OFFSET(READ) + parserTypeOffset + parserIndexOffset + clientTypeOffset + clientIndexOffset;
    pMap->valid = XC_REG_PTR_OFFSET(VALID) + parserTypeOffset + parserIndexOffset + clientTypeOffset + clientIndexOffset;
    pMap->end = XC_REG_PTR_OFFSET(END) + parserTypeOffset + parserIndexOffset + clientTypeOffset + clientIndexOffset;
}

static const char clientTypeNames[] =
{
    'R', /* RAVE */
    'M', /* MSG */
    'X', /* RMX */
    0
};

void NEXUS_TransportClientCapture_GetDefaultCreateSettings(NEXUS_TransportClientCaptureCreateSettings * pCreateSettings)
{
    if (pCreateSettings)
    {
        BKNI_Memset(pCreateSettings, 0, sizeof(NEXUS_TransportClientCaptureCreateSettings));
    }
}

NEXUS_TransportClientCapture * NEXUS_TransportClientCapture_Create(const NEXUS_TransportClientCaptureCreateSettings * pCreateSettings)
{
    NEXUS_TransportClientCapture_P_OffsetMap offsetMap;
    NEXUS_TransportClientCapture * cap = NULL;
    uint32_t offset;
    NEXUS_PidChannelStatus status;
    char parserType;
    unsigned parserIndex;

    BDBG_ASSERT(pCreateSettings);
    BDBG_OBJECT_ASSERT(pCreateSettings->pidChannel, NEXUS_PidChannel);

    cap = BKNI_Malloc(sizeof(NEXUS_TransportClientCapture));
    BKNI_Memset(cap, 0, sizeof(NEXUS_TransportClientCapture));
    BKNI_Memset(cap->name, 0, MAX_XC_CAP_NAME_LEN);

    NEXUS_PidChannel_GetStatus(pCreateSettings->pidChannel, &status);

    if (status.playback)
    {
        parserType = 'P';
        parserIndex = status.playbackIndex;
    }
    else
    {
        parserType = 'I';
        parserIndex = status.parserBand;
    }

    BKNI_Snprintf(cap->name, MAX_XC_CAP_NAME_LEN,
        "%c%u%c%u",
        parserType,
        parserIndex,
        clientTypeNames[pCreateSettings->clientType],
        pCreateSettings->clientIndex);

    NEXUS_TransportClientCapture_P_GetOffsetMap(
        status.playback,
        parserIndex,
        pCreateSettings->clientType,
        pCreateSettings->clientIndex,
        &offsetMap);

    cap->xc.validOffsetOffset = offsetMap.valid;
    offset = BREG_Read32(g_pCoreHandles->reg, offsetMap.base);
    BDBG_ASSERT(offset);
    cap->xc.base = NEXUS_OffsetToCachedAddr(offset);
    BDBG_ASSERT(cap->xc.base);
    offset = BREG_Read32(g_pCoreHandles->reg, offsetMap.end);
    BDBG_ASSERT(offset);
    cap->xc.end = NEXUS_OffsetToCachedAddr(offset + 1); /* end is inclusive */
    BDBG_ASSERT(cap->xc.end);

    cap->createSettings = *pCreateSettings;

    BDBG_MSG(("%p created for %s client", cap, cap->name));
    return cap;
}

void NEXUS_TransportClientCapture_Destroy(NEXUS_TransportClientCapture *cap)
{
    BDBG_MSG(("%s destroyed", cap->name));
    BKNI_Free(cap);
    return;
}

void NEXUS_TransportClientCapture_Open(NEXUS_TransportClientCapture *cap)
{
    char name[64];

    BKNI_Snprintf(name, sizeof(name), "videos/xc_%s_%u.xpkt", cap->name, cap->no);
    cap->xc.file = fopen(name, "w+b");

    BDBG_MSG(("%s:%u opened", cap->name, cap->no));
}

void NEXUS_TransportClientCapture_Close(NEXUS_TransportClientCapture *cap)
{
    if (cap->xc.file)
    {
        fclose(cap->xc.file);
        cap->xc.file = NULL;
    }

    BDBG_MSG(("%s:%u closed", cap->name, cap->no));
    cap->no++;
}

void NEXUS_TransportClientCapture_Flush(NEXUS_TransportClientCapture *cap)
{
    bool running = cap->running;

    BDBG_MSG(("%s client flushed", cap->name));

    if (running)
    {
        BDBG_MSG(("flushed while running"));
        NEXUS_TransportClientCapture_Stop(cap);
    }
    NEXUS_TransportClientCapture_Close(cap);
    NEXUS_TransportClientCapture_Open(cap);
    if (running)
    {
        NEXUS_TransportClientCapture_Start(cap);
    }
}

static void NEXUS_TransportClientCapture_DoWrite(NEXUS_TransportClientCapture * cap, FILE * file, const void * data, int size)
{
    BSTD_UNUSED(cap);

    if (file && size)
    {
        BDBG_MSG_TRACE(("xc:%s:%u %#x:%u", cap->name, cap->no, (unsigned)data, (unsigned)size));
        NEXUS_FlushCache(data, size);
        fwrite(data, size, 1, file);
        fflush(file);
    }
}

static void NEXUS_TransportClientCapture_GetWritePointers(NEXUS_TransportClientCaptureDescriptor * desc)
{
    uint32_t validOffset;

    BKNI_EnterCriticalSection();
    validOffset = BREG_Read32(g_pCoreHandles->reg, desc->validOffsetOffset);
    BKNI_LeaveCriticalSection();

    if (validOffset)
    {
        validOffset += 1; /* valid is inclusive, so +1 */
        validOffset -= validOffset % 4; /* prefer efficient word-aligned access */
        desc->valid = NEXUS_OffsetToCachedAddr(validOffset);
    }
}

#define WARN_UNMOVED_CYCLES 1000

static void NEXUS_TransportClientCapture_Consume(NEXUS_TransportClientCapture * cap, NEXUS_TransportClientCaptureDescriptor * desc)
{
    int moved = 0;

    NEXUS_TransportClientCapture_GetWritePointers(desc);

    if (desc->valid)
    {
        BDBG_ASSERT(desc->read >= desc->base);
        BDBG_ASSERT(desc->valid >= desc->base);

        if (desc->valid != desc->read)
        {
            if (desc->valid > desc->read)
            {
                NEXUS_TransportClientCapture_DoWrite(cap, desc->file, desc->read, (uint8_t *)desc->valid - (uint8_t *)desc->read);
                moved = 1;
                desc->read = desc->valid;
            }
            else
            {
                BDBG_ASSERT(desc->end);
                if (desc->read != desc->end)
                {
                    BDBG_ASSERT(desc->read < desc->end);
                    BDBG_ASSERT(desc->valid < desc->end);
                    NEXUS_TransportClientCapture_DoWrite(cap, desc->file, desc->read, (uint8_t *)desc->end - (uint8_t *)desc->read);
                    moved = 1;
                }

                desc->read = desc->base;
            }
        }
    }

    if (!moved)
    {
        desc->unmoved++;
        if (desc->unmoved > WARN_UNMOVED_CYCLES)
        {
            BDBG_WRN(("%s pointer hasn't moved for %u cycles", cap->name, WARN_UNMOVED_CYCLES));
            desc->unmoved = 0;
        }
        BKNI_Sleep(1);
    }
    else
    {
        desc->unmoved = 0;
    }
}

static void NEXUS_TransportClientCapture_Run(void * ctx)
{
    NEXUS_TransportClientCapture * cap = ctx;
    NEXUS_TransportClientCaptureDescriptor * desc;

    while (cap->running)
    {
        desc = &cap->xc;
        NEXUS_TransportClientCapture_Consume(cap, desc);
    }
}

NEXUS_Error NEXUS_TransportClientCapture_Start(NEXUS_TransportClientCapture *cap)
{
    NEXUS_ThreadSettings settings;

    NEXUS_Thread_GetDefaultSettings(&settings);

    /* init read to base */
    cap->xc.read = cap->xc.base;
    cap->xc.unmoved = 0;

    cap->running = true;
    cap->thread = NEXUS_Thread_Create(cap->name, &NEXUS_TransportClientCapture_Run, cap, &settings);

    if (cap->thread)
    {
        BDBG_MSG(("%s started", cap->name));
        return NEXUS_SUCCESS;
    }
    else
    {
        BDBG_MSG(("%s failed to start", cap->name));
        return NEXUS_OS_ERROR;
    }
}

void NEXUS_TransportClientCapture_Stop(NEXUS_TransportClientCapture *cap)
{
    BDBG_MSG(("%s stopped", cap->name));
    cap->running = false;
    if (cap->thread)
    {
        NEXUS_Thread_Destroy(cap->thread);
        cap->thread = NULL;
    }
}

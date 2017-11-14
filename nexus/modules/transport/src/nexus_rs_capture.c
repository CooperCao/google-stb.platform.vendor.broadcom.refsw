/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description:
 *
 **********************************************************************/

#include "nexus_transport_module.h"

#include "bchp_xpt_rsbuff.h"
#include <stdio.h>

BDBG_MODULE(nexus_rs_capture);

#define BDBG_MSG_TRACE(X) /* BDBG_MSG(X) */

typedef struct NEXUS_TransportRsCapture_P_OffsetMap
{
    uint32_t base;      /* Offset of the BASE register */
    uint32_t read;      /* Offset of the READ register */
    uint32_t valid;     /* Offset of the VALID register */
    uint32_t end;       /* Offset of the END register */
    uint32_t wrap;  /* Offset of WRAP register, not present on older chips. */
} NEXUS_TransportRsCapture_P_OffsetMap;

#define RS_REG_PARSER_BLOCK_LENGTH (BCHP_XPT_RSBUFF_BASE_POINTER_IBP1 - BCHP_XPT_RSBUFF_BASE_POINTER_IBP0)

static void NEXUS_TransportRsCapture_P_GetOffsetMap(bool playback, unsigned parserIndex, NEXUS_TransportRsCapture_P_OffsetMap * pMap)
{
    BDBG_ASSERT(pMap);

    pMap->base = (playback ? BCHP_XPT_RSBUFF_BASE_POINTER_PBP0 : BCHP_XPT_RSBUFF_BASE_POINTER_IBP0) + parserIndex * RS_REG_PARSER_BLOCK_LENGTH;
    pMap->read = (playback ? BCHP_XPT_RSBUFF_READ_POINTER_PBP0 : BCHP_XPT_RSBUFF_READ_POINTER_IBP0) + parserIndex * RS_REG_PARSER_BLOCK_LENGTH;
    pMap->valid = (playback ? BCHP_XPT_RSBUFF_VALID_POINTER_PBP0 : BCHP_XPT_RSBUFF_VALID_POINTER_IBP0) + parserIndex * RS_REG_PARSER_BLOCK_LENGTH;
    pMap->end = (playback ? BCHP_XPT_RSBUFF_END_POINTER_PBP0 : BCHP_XPT_RSBUFF_END_POINTER_IBP0) + parserIndex * RS_REG_PARSER_BLOCK_LENGTH;
    #ifdef BCHP_XPT_RSBUFF_WRAP_POINTER_IPB0
    pMap->wrap = (playback ? BCHP_XPT_RSBUFF_WRAP_POINTER_PBP0 : BCHP_XPT_RSBUFF_WRAP_POINTER_IBP0) + parserIndex * RS_REG_PARSER_BLOCK_LENGTH;
    #else
    pMap->wrap = 0;
    #endif
}

void NEXUS_TransportRsCapture_GetDefaultCreateSettings(NEXUS_TransportRsCaptureCreateSettings * pCreateSettings)
{
    if (pCreateSettings)
    {
        BKNI_Memset(pCreateSettings, 0, sizeof(*pCreateSettings));
    }
}

NEXUS_TransportRsCapture * NEXUS_TransportRsCapture_Create(const NEXUS_TransportRsCaptureCreateSettings * pCreateSettings)
{
    NEXUS_TransportRsCapture_P_OffsetMap offsetMap;
    uint32_t offset;

    NEXUS_TransportRsCapture * cap = NULL;

    BDBG_ASSERT(pCreateSettings);

    cap = BKNI_Malloc(sizeof(*cap));
    BDBG_ASSERT(cap);
    BKNI_Memset(cap, 0, sizeof(*cap));
    BKNI_Memset(cap->name, 0, MAX_RS_CAP_NAME_LEN);

    BKNI_Snprintf(cap->name, MAX_RS_CAP_NAME_LEN,
        "%c%u",
        pCreateSettings->isPlayback ? 'P' : 'I',
        pCreateSettings->parserIndex);

    NEXUS_TransportRsCapture_P_GetOffsetMap(
       pCreateSettings->isPlayback,
       pCreateSettings->parserIndex,
        &offsetMap);

    cap->rs.validOffsetOffset = offsetMap.valid;
    offset = BREG_ReadAddr(g_pCoreHandles->reg, offsetMap.base);
    BDBG_ASSERT(offset);
    cap->rs.base = NEXUS_OffsetToCachedAddr(offset);
    BDBG_ASSERT(cap->rs.base);
    if (!NEXUS_P_CpuAccessibleAddress(cap->rs.base)) {
       BDBG_ERR(("unable to capture unmapped base pointer (%p, 0x%lx, 0x%lx)", (void *) cap->rs.base, (unsigned long) offset, (unsigned long) offsetMap.base));
       BDBG_ERR(("Change plaform init settings like so: platformSettings.heap[NEXUS_VIDEO_SECURE_HEAP].memoryType = NEXUS_MemoryType_eFull; "));
        goto done;
    }

    cap->rs.wrapOffsetOffset = offsetMap.wrap;

    offset = BREG_ReadAddr(g_pCoreHandles->reg, offsetMap.end);
    BDBG_ASSERT(offset);
    cap->rs.end = NEXUS_OffsetToCachedAddr(offset + 1); /* end is inclusive */
    BDBG_ASSERT(cap->rs.end);
    if (!NEXUS_P_CpuAccessibleAddress(cap->rs.end)) {
    BDBG_ERR(("unable to capture unmapped end pointer (%p, 0x%lx, 0x%lx)", (void *) cap->rs.end, (unsigned long) offset, (unsigned long) offsetMap.end));
    BDBG_ERR(("Change plaform init settings like so: platformSettings.heap[NEXUS_VIDEO_SECURE_HEAP].memoryType = NEXUS_MemoryType_eFull; "));
        goto done;
    }

    cap->createSettings = *pCreateSettings;

    BDBG_MSG(("%p created for RS Buffer %s", (void *) cap, cap->name));

    done:
    return cap;
}

void NEXUS_TransportRsCapture_Destroy(NEXUS_TransportRsCapture *cap)
{
    BDBG_MSG(("%s destroyed", cap->name));
    BKNI_Free(cap);
    return;
}

void NEXUS_TransportRsCapture_Open(NEXUS_TransportRsCapture *cap)
{
    char name[64];

    BKNI_Snprintf(name, sizeof(name), "videos/rs_%s_%u.xpkt", cap->name, cap->no);
    cap->rs.file = fopen(name, "w+b");

    BDBG_MSG(("%s:%u opened", cap->name, cap->no));
}

void NEXUS_TransportRsCapture_Close(NEXUS_TransportRsCapture *cap)
{
    if (cap->rs.file)
    {
        fclose(cap->rs.file);
        cap->rs.file = NULL;
    }

    BDBG_MSG(("%s:%u closed", cap->name, cap->no));
    cap->no++;
}

void NEXUS_TransportRsCapture_Flush(NEXUS_TransportRsCapture *cap)
{
    bool running = cap->running;

    BDBG_MSG(("%s client flushed", cap->name));

    if (running)
    {
        BDBG_MSG(("flushed while running"));
        NEXUS_TransportRsCapture_Stop(cap);
    }
    NEXUS_TransportRsCapture_Close(cap);
    NEXUS_TransportRsCapture_Open(cap);
    if (running)
    {
        NEXUS_TransportRsCapture_Start(cap);
    }
}

static void NEXUS_TransportRsCapture_DoWrite(NEXUS_TransportRsCapture * cap, FILE * file, const void * data, int size)
{
    BSTD_UNUSED(cap);

    if (file && size)
    {
        BDBG_MSG_TRACE(("rs:%s:%u %#x:%u", cap->name, cap->no, (unsigned)data, (unsigned)size));
        NEXUS_FlushCache(data, size);
        fwrite(data, size, 1, file);
        fflush(file);
    }
}

static void NEXUS_TransportRsCapture_GetWritePointers(NEXUS_TransportRsCaptureDescriptor * desc)
{
    uint32_t validOffset = 0;
    uint32_t wrapOffset = 0;

    BKNI_EnterCriticalSection();
    validOffset = BREG_ReadAddr(g_pCoreHandles->reg, desc->validOffsetOffset);
    if (desc->wrapOffsetOffset) {
       wrapOffset = BREG_ReadAddr(g_pCoreHandles->reg, desc->wrapOffsetOffset);
    }
    BKNI_LeaveCriticalSection();

    if (wrapOffset) {
       wrapOffset += 1;
       wrapOffset -= wrapOffset % 4;
       desc->wrap = NEXUS_OffsetToCachedAddr(wrapOffset);
    }

    if (validOffset)
    {
        validOffset += 1; /* valid is inclusive, so +1 */
        validOffset -= validOffset % 4; /* prefer efficient word-aligned access */
        desc->valid = NEXUS_OffsetToCachedAddr(validOffset);
    }
}

#define WARN_UNMOVED_CYCLES 10000

static void NEXUS_TransportRsCapture_Consume(NEXUS_TransportRsCapture * cap, NEXUS_TransportRsCaptureDescriptor * desc)
{
    int moved = 0;

    NEXUS_TransportRsCapture_GetWritePointers(desc);

    if (desc->valid)
    {
        BDBG_ASSERT(desc->read >= desc->base);
        BDBG_ASSERT(desc->valid >= desc->base);

        if (desc->valid != desc->read)
        {
            if (desc->valid > desc->read)
            {
           /* No wrap */
                NEXUS_TransportRsCapture_DoWrite(cap, desc->file, desc->read, (uint8_t *)desc->valid - (uint8_t *)desc->read);
                moved = 1;
                desc->read = desc->valid;
            }
            else
            {
           /* It did wrap */
                BDBG_ASSERT(desc->end);
#ifdef BCHP_XPT_RSBUFF_WRAP_POINTER_IPB0
                if (desc->read != desc->wrap)
                {
                    BDBG_ASSERT(desc->read < desc->wrap);
                    BDBG_ASSERT(desc->valid < desc->wrap);
                    NEXUS_TransportRsCapture_DoWrite(cap, desc->file, desc->read, (uint8_t *)desc->wrap - (uint8_t *)desc->read);
                    moved = 1;
                }
#else
                if (desc->read != desc->end)
                {
                    BDBG_ASSERT(desc->read < desc->end);
                    BDBG_ASSERT(desc->valid < desc->end);
                    NEXUS_TransportRsCapture_DoWrite(cap, desc->file, desc->read, (uint8_t *)desc->end - (uint8_t *)desc->read);
                    moved = 1;
                }
#endif
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

static void NEXUS_TransportRsCapture_Run(void * ctx)
{
    NEXUS_TransportRsCapture * cap = ctx;
    NEXUS_TransportRsCaptureDescriptor * desc;

    while (cap->running)
    {
        desc = &cap->rs;
        NEXUS_TransportRsCapture_Consume(cap, desc);
    }
}

NEXUS_Error NEXUS_TransportRsCapture_Start(NEXUS_TransportRsCapture *cap)
{
    NEXUS_ThreadSettings settings;

    NEXUS_Thread_GetDefaultSettings(&settings);

    /* init read to base */
    cap->rs.read = cap->rs.base;
    cap->rs.unmoved = 0;

    cap->running = true;
    cap->thread = NEXUS_Thread_Create(cap->name, &NEXUS_TransportRsCapture_Run, cap, &settings);

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

void NEXUS_TransportRsCapture_Stop(NEXUS_TransportRsCapture *cap)
{
    BDBG_MSG(("%s stopped", cap->name));
    cap->running = false;
    if (cap->thread)
    {
        NEXUS_Thread_Destroy(cap->thread);
        cap->thread = NULL;
    }
}

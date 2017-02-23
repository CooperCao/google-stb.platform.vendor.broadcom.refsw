/***************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include <stdio.h>

BDBG_MODULE(nexus_rave_capture);

#define NEXUS_RAVE_CONTEXT_MAP(RAVE) (((RAVE)->swRave.raveHandle && (RAVE)->swRave.enabled) ? &(RAVE)->swRave.xptContextMap : &(RAVE)->xptContextMap)
#define BDBG_MSG_TRACE(X) /* BDBG_MSG(X) */

void NEXUS_RaveCapture_GetDefaultCreateSettings(NEXUS_RaveCaptureCreateSettings * pCreateSettings)
{
    if (pCreateSettings)
    {
        BKNI_Memset(pCreateSettings, 0, sizeof(NEXUS_RaveCaptureCreateSettings));
    }
}

NEXUS_RaveCapture * NEXUS_RaveCapture_Create(const NEXUS_RaveCaptureCreateSettings * pCreateSettings)
{
    BAVC_XptContextMap *pXptContextMap = NULL;
    NEXUS_RaveCapture * cap = NULL;
    uint64_t baseOffset;

    BDBG_ASSERT(pCreateSettings);
    BDBG_OBJECT_ASSERT(pCreateSettings->rave, NEXUS_Rave);

    cap = BKNI_Malloc(sizeof(NEXUS_RaveCapture));
    BKNI_Memset(cap, 0, sizeof(NEXUS_RaveCapture));
    cap->index = pCreateSettings->rave->index;

    pXptContextMap = NEXUS_RAVE_CONTEXT_MAP(pCreateSettings->rave);

    baseOffset = BREG_ReadAddr(g_pCoreHandles->reg, pXptContextMap->CDB_Base);
    BDBG_ASSERT(baseOffset);
    cap->cdb.validOffsetOffset = pXptContextMap->CDB_Valid;
    cap->cdb.wrapOffsetOffset = pXptContextMap->CDB_Wrap;
    cap->cdb.base = NEXUS_OffsetToCachedAddr(baseOffset);
    BDBG_ASSERT(cap->cdb.base);

    baseOffset = BREG_ReadAddr(g_pCoreHandles->reg, pXptContextMap->ITB_Base);
    BDBG_ASSERT(baseOffset);
    cap->itb.validOffsetOffset = pXptContextMap->ITB_Valid;
    cap->itb.wrapOffsetOffset = pXptContextMap->ITB_Wrap;
    cap->itb.base = NEXUS_OffsetToCachedAddr(baseOffset);
    BDBG_ASSERT(cap->itb.base);

    cap->createSettings = *pCreateSettings;

    BDBG_MSG(("%p created for rave context %u", (void*)cap, cap->index));
    return cap;
}

void NEXUS_RaveCapture_Destroy(NEXUS_RaveCapture *cap)
{
    BDBG_MSG(("%u destroyed", cap->index));
    BKNI_Free(cap);
    return;
}

void NEXUS_RaveCapture_Open(NEXUS_RaveCapture *cap)
{
    char name[64];

    BKNI_Snprintf(name, sizeof(name), "videos/rave_%u_cdb_%u.bin", cap->index, cap->no);
    cap->cdb.file = fopen(name, "w+b");
    BKNI_Snprintf(name, sizeof(name), "videos/rave_%u_itb_%u.bin", cap->index, cap->no);
    cap->itb.file = fopen(name, "w+b");

    BDBG_MSG(("%u:%u opened", cap->index, cap->no));
}

void NEXUS_RaveCapture_Close(NEXUS_RaveCapture *cap)
{
    if (cap->cdb.file)
    {
        fclose(cap->cdb.file);
        cap->cdb.file = NULL;
    }

    if (cap->itb.file)
    {
        fclose(cap->itb.file);
        cap->itb.file = NULL;
    }

    BDBG_MSG(("%u:%u closed", cap->index, cap->no));
    cap->no++;
}

void NEXUS_RaveCapture_Flush(NEXUS_RaveCapture *cap)
{
    bool running = cap->running;

    BDBG_MSG(("%u flushed", cap->index));

    if (running)
    {
        BDBG_MSG(("flushed while running"));
        NEXUS_RaveCapture_Stop(cap);
    }
    NEXUS_RaveCapture_Close(cap);
    NEXUS_RaveCapture_Open(cap);
    if (running)
    {
        NEXUS_RaveCapture_Start(cap);
    }
}

static void NEXUS_RaveCapture_DoWrite(NEXUS_RaveCapture * cap, const char * name, FILE * file, const void * data, int size)
{
    if (file && size)
    {
        BDBG_MSG_TRACE(("%s:%u:%u %#x:%u", name, cap->index, cap->no, (unsigned)data, (unsigned)size));
        NEXUS_FlushCache(data, size);
        fwrite(data, size, 1, file);
        fflush(file);
    }
}

static void NEXUS_RaveCapture_GetWritePointers(NEXUS_RaveCaptureDescriptor * desc)
{
    uint64_t validOffset;
    uint64_t wrapOffset;

    BKNI_EnterCriticalSection();
    validOffset = BREG_ReadAddr(g_pCoreHandles->reg, desc->validOffsetOffset);
    wrapOffset = BREG_ReadAddr(g_pCoreHandles->reg, desc->wrapOffsetOffset);
    BKNI_LeaveCriticalSection();

    if (validOffset)
    {
        validOffset += 1; /* valid is inclusive, so +1 */
        validOffset -= validOffset % 4; /* prefer efficient word-aligned access */
        desc->valid = NEXUS_OffsetToCachedAddr(validOffset);
    }
    if (wrapOffset)
    {
        wrapOffset += 1;
        desc->wrap = NEXUS_OffsetToCachedAddr(wrapOffset);
    }
}

#define WARN_UNMOVED_CYCLES 1000

static void NEXUS_RaveCapture_Consume(NEXUS_RaveCapture * cap, const char * name, NEXUS_RaveCaptureDescriptor * desc)
{
    int moved = 0;

    NEXUS_RaveCapture_GetWritePointers(desc);

    if (desc->valid)
    {
        BDBG_ASSERT(desc->read >= desc->base);
        BDBG_ASSERT(desc->valid >= desc->base);

        if (desc->valid != desc->read)
        {
            if (desc->valid > desc->read)
            {
                NEXUS_RaveCapture_DoWrite(cap, name, desc->file, desc->read, (uint8_t *)desc->valid - (uint8_t *)desc->read);
                moved = 1;
                desc->read = desc->valid;
            }
            else
            {
                BDBG_ASSERT(desc->wrap);
                if (desc->read != desc->wrap)
                {
                    BDBG_ASSERT(desc->read < desc->wrap);
                    BDBG_ASSERT(desc->valid < desc->wrap);
                    NEXUS_RaveCapture_DoWrite(cap, name, desc->file, desc->read, (uint8_t *)desc->wrap - (uint8_t *)desc->read);
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
            BDBG_WRN(("%s:%u pointer hasn't moved for %u cycles", name, cap->index, WARN_UNMOVED_CYCLES));
            desc->unmoved = 0;
        }
        BKNI_Sleep(1);
    }
    else
    {
        desc->unmoved = 0;
    }
}

static void NEXUS_RaveCapture_Run(void * ctx)
{
    NEXUS_RaveCapture * cap = ctx;
    NEXUS_RaveCaptureDescriptor * desc;

    while (cap->running)
    {
        desc = &cap->cdb;
        NEXUS_RaveCapture_Consume(cap, "cdb", desc);

        desc = &cap->itb;
        NEXUS_RaveCapture_Consume(cap, "itb", desc);
    }
}

NEXUS_Error NEXUS_RaveCapture_Start(NEXUS_RaveCapture *cap)
{
    NEXUS_ThreadSettings settings;

    NEXUS_Thread_GetDefaultSettings(&settings);

    /* init read to base */
    cap->cdb.read = cap->cdb.base;
    cap->cdb.unmoved = 0;
    cap->itb.read = cap->itb.base;
    cap->itb.unmoved = 0;

    cap->running = true;
    cap->thread = NEXUS_Thread_Create("rave_capture", &NEXUS_RaveCapture_Run, cap, &settings);

    if (cap->thread)
    {
        BDBG_MSG(("%u started", cap->index));
        return NEXUS_SUCCESS;
    }
    else
    {
        BDBG_MSG(("%u failed to start", cap->index));
        return NEXUS_OS_ERROR;
    }
}

void NEXUS_RaveCapture_Stop(NEXUS_RaveCapture *cap)
{
    BDBG_MSG(("%u stopped", cap->index));
    cap->running = false;
    if (cap->thread)
    {
        NEXUS_Thread_Destroy(cap->thread);
        cap->thread = NULL;
    }
}

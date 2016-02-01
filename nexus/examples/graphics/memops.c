/******************************************************************************
 *    (c)2008-2012 Broadcom Corporation
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
 *****************************************************************************/

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_GRAPHICS2D
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "bm2mc_packet.h"
#include "nexus_core_utils.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>

BDBG_MODULE(memops);

void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static const BM2MC_PACKET_Blend copyColor = {BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eOne, false,
    BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, false, BM2MC_PACKET_BlendFactor_eZero};
static const BM2MC_PACKET_Blend copyAlpha = {BM2MC_PACKET_BlendFactor_eSourceAlpha, BM2MC_PACKET_BlendFactor_eOne, false,
    BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, false, BM2MC_PACKET_BlendFactor_eZero};

static NEXUS_Error graphics_memcpy(NEXUS_Graphics2DHandle gfx, BKNI_EventHandle checkpointEvent, NEXUS_MemoryBlockHandle dst, unsigned dst_offset, NEXUS_MemoryBlockHandle src, unsigned src_offset,size_t size)
{
    const unsigned stride = 4096;
    void *pkt_buffer, *next;
    size_t pkt_size;
    NEXUS_Error rc;
    NEXUS_Addr src_addr,dst_addr;

    if(size % stride) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    rc = NEXUS_Graphics2D_GetPacketBuffer(gfx, &pkt_buffer, &pkt_size, 1024);
    BDBG_ASSERT(!rc);


    rc = NEXUS_MemoryBlock_LockOffset(src, &src_addr);
    if(rc!=NEXUS_SUCCESS) {
        return BERR_TRACE(rc);
    }

    rc = NEXUS_MemoryBlock_LockOffset(dst, &dst_addr);
    if(rc!=NEXUS_SUCCESS) {
        return BERR_TRACE(rc);
    }

    next = pkt_buffer;
    {
        BM2MC_PACKET_PacketOutputFeeder *pPacket = next;
        BM2MC_PACKET_INIT(pPacket, OutputFeeder, false);
        pPacket->plane.address = dst_addr+dst_offset;
        pPacket->plane.pitch = stride;
        pPacket->plane.format = BM2MC_PACKET_PixelFormat_eA8_R8_G8_B8;
        pPacket->plane.width = stride/4;
        pPacket->plane.height = size / stride;
        next = ++pPacket;
    }

    {
        BM2MC_PACKET_PacketBlend *pPacket = (BM2MC_PACKET_PacketBlend *)next;
        BM2MC_PACKET_INIT( pPacket, Blend, false );
        pPacket->color_blend = copyColor;
        pPacket->alpha_blend = copyAlpha;
        pPacket->color = 0;
        next = ++pPacket;
    }
    {
        BM2MC_PACKET_PacketSourceFeeder *pPacket = next;
        BM2MC_PACKET_INIT(pPacket, SourceFeeder, false );
        pPacket->plane.address = src_addr+src_offset;
        pPacket->plane.pitch = stride;
        pPacket->plane.format = BM2MC_PACKET_PixelFormat_eA8_R8_G8_B8;
        pPacket->plane.width = stride/4;
        pPacket->plane.height = size / stride;
        next = ++pPacket;
    }
    {
        BM2MC_PACKET_PacketScaleBlit *pPacket = next;
        BM2MC_PACKET_INIT(pPacket, ScaleBlit, true);
        pPacket->src_rect.x = 0;
        pPacket->src_rect.y = 0;
        pPacket->src_rect.width = stride/4;
        pPacket->src_rect.height = size/stride;
        pPacket->out_rect.x = 0;
        pPacket->out_rect.y = 0;
        pPacket->out_rect.width = stride/4;
        pPacket->out_rect.height = size/stride;
        next = ++pPacket;
    }
    rc = NEXUS_Graphics2D_PacketWriteComplete(gfx, (uint8_t*)next - (uint8_t*)pkt_buffer);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
    } else if(rc!=NEXUS_SUCCESS) {
        return BERR_TRACE(rc);
    }
    NEXUS_MemoryBlock_UnlockOffset(src);
    NEXUS_MemoryBlock_UnlockOffset(dst);
    return NEXUS_SUCCESS;
}

static NEXUS_Error graphics_memset(NEXUS_Graphics2DHandle gfx, BKNI_EventHandle checkpointEvent, NEXUS_MemoryBlockHandle block, unsigned offset, int c, size_t size)
{
    const unsigned stride = 4096;
    void *pkt_buffer, *next;
    size_t pkt_size;
    NEXUS_Error rc;
    NEXUS_Addr addr;
    uint32_t c32;

    c32 = c;
    c32 = c32  | (c32<<8) | (c32<<16) | (c32<<24);

    if(size % stride) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }


    rc = NEXUS_Graphics2D_GetPacketBuffer(gfx, &pkt_buffer, &pkt_size, 1024);
    BDBG_ASSERT(!rc);


    rc = NEXUS_MemoryBlock_LockOffset(block, &addr);
    if(rc!=NEXUS_SUCCESS) {
        return BERR_TRACE(rc);
    }

    next = pkt_buffer;
    {
        BM2MC_PACKET_PacketOutputFeeder *pPacket = next;
        BM2MC_PACKET_INIT(pPacket, OutputFeeder, false);
        pPacket->plane.address = addr+offset;
        pPacket->plane.pitch = stride;
        pPacket->plane.format = BM2MC_PACKET_PixelFormat_eA8_R8_G8_B8;
        pPacket->plane.width = stride/4;
        pPacket->plane.height = size / stride;
        next = ++pPacket;
    }

    {
        BM2MC_PACKET_PacketBlend *pPacket = (BM2MC_PACKET_PacketBlend *)next;
        BM2MC_PACKET_INIT( pPacket, Blend, false );
        pPacket->color_blend = copyColor;
        pPacket->alpha_blend = copyAlpha;
        pPacket->color = 0;
        next = ++pPacket;
    }
    {
        BM2MC_PACKET_PacketSourceColor *pPacket = next;
        BM2MC_PACKET_INIT(pPacket, SourceColor, false );
        pPacket->color = c32;
        next = ++pPacket;
    }
    {
        BM2MC_PACKET_PacketFillBlit *pPacket = next;
        BM2MC_PACKET_INIT(pPacket, FillBlit, true);
        pPacket->rect.x = 0;
        pPacket->rect.y = 0;
        pPacket->rect.width = stride/4;
        pPacket->rect.height = size/stride;
        next = ++pPacket;
    }
    rc = NEXUS_Graphics2D_PacketWriteComplete(gfx, (uint8_t*)next - (uint8_t*)pkt_buffer);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
    } else if(rc!=NEXUS_SUCCESS) {
        return BERR_TRACE(rc);
    }
    NEXUS_MemoryBlock_UnlockOffset(block);
    return NEXUS_SUCCESS;
}

#define BLOCK_COUNT 4
int main(void)
{
    NEXUS_Graphics2DSettings gfxSettings;
    BKNI_EventHandle checkpointEvent, spaceAvailableEvent;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Error rc;
    unsigned i;
    int result;
    NEXUS_MemoryBlockHandle memBlock[BLOCK_COUNT];
    const unsigned blockSize = 4096;
    void *mem[BLOCK_COUNT];
    void *ptr[BLOCK_COUNT];

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);


    BKNI_CreateEvent(&checkpointEvent);
    BKNI_CreateEvent(&spaceAvailableEvent);

    gfx = NEXUS_Graphics2D_Open(0, NULL);

    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    gfxSettings.packetSpaceAvailable.callback = complete;
    gfxSettings.packetSpaceAvailable.context = spaceAvailableEvent;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);


    BDBG_LOG(("Allocating"));
    for(i=0;i<BLOCK_COUNT;i++) {
        NEXUS_HeapHandle heap = NULL; /* use default heap */
        memBlock[i] = NEXUS_MemoryBlock_Allocate(heap, blockSize, 0, NULL);
        BDBG_ASSERT(memBlock[i]);
        mem[i] = BKNI_Malloc(blockSize);
        BDBG_ASSERT(mem[i]);
    }

    BDBG_LOG(("memset"));
    for(i=0;i<BLOCK_COUNT;i++) {
        rc = graphics_memset(gfx, checkpointEvent, memBlock[i], 0, i+1, blockSize);
        BDBG_ASSERT(rc==NEXUS_SUCCESS);
        BKNI_Memset(mem[i], i+1, blockSize);
    }

    BDBG_LOG(("Lock"));
    for(i=0;i<BLOCK_COUNT;i++) {
        NEXUS_Addr addr;
        rc = NEXUS_MemoryBlock_Lock(memBlock[i], &ptr[i]);
        BDBG_ASSERT(rc==NEXUS_SUCCESS);
        rc = NEXUS_MemoryBlock_LockOffset(memBlock[i], &addr);
        BDBG_ASSERT(rc==NEXUS_SUCCESS);
        BDBG_LOG(("memory[%u] at " BDBG_UINT64_FMT "(%p)", i, BDBG_UINT64_ARG(addr), ptr[i]));
        NEXUS_MemoryBlock_UnlockOffset(memBlock[i]);
    }

    BDBG_LOG(("compare after memset"));
    for(i=0;i<BLOCK_COUNT;i++) {
        NEXUS_FlushCache(ptr[i], blockSize);
        result = BKNI_Memcmp(mem[i], ptr[i], blockSize);
        BDBG_ASSERT(result==0);
    }

    BDBG_LOG(("memcpy"));
    for(i=0;i<BLOCK_COUNT;i++) {
        NEXUS_FlushCache(ptr[i], blockSize);
    }
    for(i=1;i<BLOCK_COUNT;i++) {
        rc = graphics_memcpy(gfx, checkpointEvent, memBlock[i-1], 0, memBlock[i], 0, blockSize);
        BDBG_ASSERT(rc==NEXUS_SUCCESS);
    }
    BDBG_LOG(("compare after memcpy"));
    for(i=1;i<BLOCK_COUNT;i++) {
        NEXUS_FlushCache(ptr[i-1], blockSize);

        result = BKNI_Memcmp(mem[i], ptr[i-1], blockSize);

        BDBG_ASSERT(result==0);
    }

    BDBG_LOG(("Freeing"));
    for(i=0;i<BLOCK_COUNT;i++) {
        NEXUS_MemoryBlock_Unlock(memBlock[i]);
        NEXUS_MemoryBlock_Free(memBlock[i]);
        BKNI_Free(mem[i]);
    }

    BDBG_LOG(("Done. Press Enter to exit"));
    getchar();

    return 0;
}
#else /* NEXUS_HAS_GRAPHICS2D */
int main(void)
{
    printf("ERROR: NEXUS_Graphics2D not supported\n");
    return -1;
}
#endif

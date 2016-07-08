/***************************************************************************
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
 *
 * Module Description:
 *
 **************************************************************************/
#include "nexus_graphics2d_module.h"
#include "nexus_graphics2d_init.h"
#include "priv/nexus_graphics2d_standby_priv.h"

#include "bgrclib_packet.h"
#include "bgrc.h"
#include "bgrc_packet.h"
#include "blst_squeue.h"

#if NEXUS_MODE_client && NEXUS_WEBCPU_core1_server
#define NEXUS_CLIENT_RESOURCES_DISABLED 1
#undef NEXUS_OBJECT_ACQUIRE
#undef NEXUS_OBJECT_RELEASE
#undef NEXUS_OBJECT_REGISTER
#define NEXUS_OBJECT_ACQUIRE(OWNER, TYPE, HANDLE) do {BSTD_UNUSED(OWNER);BSTD_UNUSED(HANDLE);}while(0)
#define NEXUS_OBJECT_RELEASE(OWNER, TYPE, HANDLE) do {BSTD_UNUSED(OWNER);BSTD_UNUSED(HANDLE);}while(0)
#define NEXUS_OBJECT_REGISTER(TYPE, HANDLE, OP)
#endif

#define NEXUS_GRAPHICS2D_SUPPORT_SURFACE_COMPACTION 1
#define NEXUS_GRAPHICS2D_MAX_CONTEXTS 32
struct NEXUS_Graphics2DEngine {
    BGRC_Handle grc;
    BLST_S_HEAD(NEXUS_Graphics2DEngineContexts, NEXUS_Graphics2D) contexts;
    BKNI_EventHandle advanceEvent;
    NEXUS_EventCallbackHandle advanceEventCallback;
    unsigned allocatedContexts;
    BGRC_Packet_ContextStatus contextStatus[NEXUS_GRAPHICS2D_MAX_CONTEXTS];
    struct {
        NEXUS_HeapHandle heap;
        BGRC_Settings grcSettings;
    } standby;
    bool secure;
};

struct NEXUS_Graphics2D {
    NEXUS_OBJECT(NEXUS_Graphics2D);
    unsigned index;
    struct NEXUS_Graphics2DEngine *engine;

    /* magnum */
    BGRC_Handle grc;
    BGRClib_Handle grclib;
    BGRC_PacketContext_Handle functionContext; /* context used for functions */
    BGRC_PacketContext_Handle packetContext; /* context used for packet blit */
    BLST_S_ENTRY(NEXUS_Graphics2D) link; /* for NEXUS_Graphics2DEngine.contexts */

    NEXUS_Graphics2DOpenSettings openSettings;
    NEXUS_Graphics2DSettings settings;
    NEXUS_TaskCallbackHandle checkpoint;
    NEXUS_TaskCallbackHandle packetSpaceAvailable;
    unsigned packetSpaceAvailableCount;
    unsigned checkpointCount;
    unsigned checkpointWaitCount;
    unsigned blitCount;
    unsigned packetWriteCompleteCount;
    bool verifyPacketHeap;
    struct {
        unsigned counter;
        NEXUS_TimerHandle timer;
    } checkpointWatchdog;

    struct {
        BGRClib_BlitParams blitParams;
        BGRClib_BlitColorKeyParams colorKeyParams;
        BGRClib_BlitMatrixParams matrixParams;
        BGRClib_BlitPatternParams patternParams;
        BGRClib_BlitScalingControlParams scalingControlParams;
    } blitData;

    struct {
        BGRC_PacketContext_CreateSettings packetContextSettings;
    } standby;
    /* FIFO that is used to account for surfaces that should be locked in the memory for HW access */
    unsigned surfaceFifoCheckpoint; /* this is snapshort of surfaceFifoWrite at time of checkpoint function call */
    unsigned surfaceFifoRead; /* when checpoint arrives  all surfacess between Read and Checkpoint could be reccycled */
    unsigned surfaceFifoWrite; /* when new surface submitted to HW, it should be added into the FIFO */
    NEXUS_SurfaceHandle surfaceFifo[1]; /* variable size array, should be last in the structture */
};

typedef enum NEXUS_Graphics2DPowerState {
    NEXUS_Graphics2DPowerState_ePowerUp, /* power up the HW. */
    NEXUS_Graphics2DPowerState_eTryPowerDown, /* power down the HW if the last blit has completed. */
    NEXUS_Graphics2DPowerState_ePowerDown /* there are no more blits bpending. power down the HW. */
} NEXUS_Graphics2DPowerState;

void NEXUS_Graphics2D_P_SetPower(NEXUS_Graphics2DHandle gfx, NEXUS_Graphics2DPowerState state);
#if NEXUS_GRAPHICS2D_SUPPORT_SURFACE_COMPACTION
NEXUS_Error NEXUS_Graphics2D_P_LockPlaneAndPalette(NEXUS_Graphics2DHandle gfx, NEXUS_SurfaceHandle surface, BM2MC_PACKET_Plane *pPlane, NEXUS_Addr *pPaletteOffset);
#else
#define NEXUS_Graphics2D_P_LockPlaneAndPalette(gfx, surface, plane, pPaletteOffset) NEXUS_Surface_InitPlaneAndPaletteOffset(surface, plane, pPaletteOffset)
#endif

/* used for blit and destripe, so it can't be static */
extern const int32_t g_NEXUS_ai32_Matrix_YCbCrtoRGB[20];

/* global data. */
struct Graphics2DData {
    NEXUS_Graphics2DModuleInternalSettings moduleSettings;
    NEXUS_Graphics2DModuleSettings settings;
};
extern struct Graphics2DData g_NEXUS_graphics2DData;

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Graphics2D);



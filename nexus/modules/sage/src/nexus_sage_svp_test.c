/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/

#include "nexus_sage_module.h"

/* magnum basemodules */
#include "berr.h"
#include "bkni.h"

/* BVN Monitor */

#include "nexus_sage_svp_bvn.h"

#include "nexus_sage.h"
#include "priv/nexus_sage_priv.h"
#include "bsagelib.h"
#include "bsagelib_client.h"
#include "bsagelib_rpc.h"
#include "bchp_memc_clients.h"
#if defined(BCHP_MEMC_ARC_0_REG_START)
#include "bchp_memc_arc_0.h"
#endif
#if defined(BCHP_MEMC_ARC_1_REG_START)
#include "bchp_memc_arc_1.h"
#endif
#if defined(BCHP_MEMC_ARC_2_REG_START)
#include "bchp_memc_arc_2.h"
#endif
#include "bchp_sun_gisb_arb.h"

/* Macrovision (MV) Monitoring */
#include "nexus_sage_svp_mv.h"

BDBG_MODULE(nexus_sage_svp_test);

#if defined(BCHP_MEMC_ARC_0_REG_START)

#define ISR_RATE 1000

#define VIOLATION_SHOW(memc, idx, val) \
    val = BREG_Read32(g_pCoreHandles->reg, BCHP_MEMC_ARC_##memc##_ARC_##idx##_VIOLATION_INFO_CMD); \
    if(val) \
    { \
       BDBG_ERR(("!!!!ARC[%d][%d] VIOLATION: Client %d @ 0x%08x (0x%x)!!!!", memc, idx, \
                 BCHP_GET_FIELD_DATA(val, MEMC_ARC_##memc##_ARC_##idx##_VIOLATION_INFO_CMD, CLIENTID), \
                 BREG_Read32(g_pCoreHandles->reg, BCHP_MEMC_ARC_##memc##_ARC_##idx##_VIOLATION_INFO_START_ADDR)<<3, \
                 BCHP_GET_FIELD_DATA(val, MEMC_ARC_##memc##_ARC_##idx##_VIOLATION_INFO_CMD, REQ_TYPE))); \
       BREG_Write32(g_pCoreHandles->reg, BCHP_MEMC_ARC_##memc##_ARC_##idx##_VIOLATION_INFO_CLEAR, 1); \
    }

static const struct {
    const char achName[16];
    BAVC_Access eAccess;
    BAVC_CoreId  eCoreId;
} g_CoreInfoTbl[] = {
#define BCHP_P_MEMC_DEFINE_SVP_HWBLOCK(svp_block, access) { #svp_block, BAVC_Access_e##access, BAVC_CoreId_e##svp_block },
#include "memc/bchp_memc_svp_hwblock.h"
#undef BCHP_P_MEMC_DEFINE_SVP_HWBLOCK
};

static const struct {
    BCHP_MemcClient                    eClientId;
    BAVC_CoreId                        eCoreId;
} g_ClientMapTbl[] = {
#define BCHP_P_MEMC_DEFINE_CLIENT_MAP(client,block,svp_block) {BCHP_MemcClient_e##client, BAVC_CoreId_e##svp_block},
#include "memc/bchp_memc_clients_chip_map.h"
#undef BCHP_P_MEMC_DEFINE_CLIENT_MAP
    {BCHP_MemcClient_eMax, BAVC_CoreId_eInvalid}
};

typedef struct BMRC_P_ClientEntry
{
    const char   *pchClientName;
    BMRC_Client   eClient;
    uint16_t      ausClientId[3];
} BMRC_P_ClientEntry;

#define INVALID (~0)
static const BMRC_P_ClientEntry BMRC_P_astClientTbl[] = {
#if BCHP_P_MEMC_COUNT == 1
#define BCHP_P_MEMC_DEFINE_CLIENT(client,m0) {#client, BCHP_MemcClient_e##client, {m0}},
#elif BCHP_P_MEMC_COUNT == 2
#define BCHP_P_MEMC_DEFINE_CLIENT(client,m0,m1) {#client, BCHP_MemcClient_e##client, {m0,m1}},
#elif BCHP_P_MEMC_COUNT == 3
#define BCHP_P_MEMC_DEFINE_CLIENT(client,m0,m1,m2) {#client, BCHP_MemcClient_e##client, {m0,m1,m2}},
#else
#error "not supported"
#endif

#include "memc/bchp_memc_clients_chip.h"
};
#undef BCHP_P_MEMC_DEFINE_CLIENT

#define MAX_CLIENTS 8 /* Max memc clients per "core id" */
typedef struct coreMapInfo {
    int16_t client[MAX_CLIENTS];
}coreMapInfo;

#define CLIENT_INDEX_SIZE 8
#define ARC_IDX_MAX (BCHP_P_MEMC_COUNT)
#define ARC_DELTA (BCHP_MEMC_ARC_0_ARC_1_CNTRL-BCHP_MEMC_ARC_0_ARC_0_CNTRL)
#define SETINFO(MEMC, IDX, info) \
    info.cntrl = BCHP_MEMC_ARC_##MEMC##_ARC_##IDX##_CNTRL; \
    info.low = BCHP_MEMC_ARC_##MEMC##_ARC_##IDX##_ADRS_RANGE_LOW; \
    info.high = BCHP_MEMC_ARC_##MEMC##_ARC_##IDX##_ADRS_RANGE_HIGH; \
    info.readStart = BCHP_MEMC_ARC_##MEMC##_ARC_##IDX##_READ_RIGHTS_0; \
    info.writeStart = BCHP_MEMC_ARC_##MEMC##_ARC_##IDX##_WRITE_RIGHTS_0;

struct maskInfo {
    uint32_t read;
    uint32_t write;
};

struct memcInfo {
    struct maskInfo exclusive[CLIENT_INDEX_SIZE];
    struct maskInfo nonexclusive[CLIENT_INDEX_SIZE];
    NEXUS_Addr start;
    unsigned len;
};

struct svpTestInfo {
    struct memcInfo gConfig[ARC_IDX_MAX]; /* 1 URR per MEMC, plus SRR and CRR */
    BAVC_CoreList coreList;
    BBVN_Monitor_Handle hBvnMonitor;
    NEXUS_ThreadHandle hBvnThread;
    bool thread_stop;
    BKNI_MutexHandle bvnLock;
    BBVN_Monitor_Status bvnStatus;
    coreMapInfo coreMap[BAVC_CoreId_eMax];
    bool init;
    uint32_t v3dCount;
    bool custom_arc;
};

struct arc_reg_info {
    uint32_t cntrl;
    uint32_t low;
    uint32_t high;
    uint32_t readStart;
    uint32_t writeStart;
};

static struct svpTestInfo local_info;

static const struct {
    BAVC_Access eAccess;
} g_CoreIdAccess[] = {
#define BCHP_P_MEMC_DEFINE_SVP_HWBLOCK(svp_block, access) {BAVC_Access_e##access},
#include "memc/bchp_memc_svp_hwblock.h"
#undef BCHP_P_MEMC_DEFINE_SVP_HWBLOCK
};

static void addClient(unsigned int client, bool add, struct maskInfo *exc, struct maskInfo *nonExc, BAVC_Access access)
{
    int idx=0;

    if(client==(__typeof__(client))~0)
    {
        return;
    }

    if(access>=BAVC_Access_eInvalid)
    {
        return;
    }

    idx = client>>5;
    client %= 32;

    if(add)
    {
        if(access!=BAVC_Access_eWO)
        {
            nonExc[idx].read|=(1<<client);
        }
        if(access!=BAVC_Access_eRO)
        {
            exc[idx].write|=(1<<client);
            nonExc[idx].write|=(1<<client);
        }
    }
    else
    {
        if(access!=BAVC_Access_eWO)
        {
            nonExc[idx].read&=~(1<<client);
        }
        if(access!=BAVC_Access_eRO)
        {
            exc[idx].write&=~(1<<client);
            nonExc[idx].write&=~(1<<client);
        }
    }
}


/** generate local core map
* TBD on this... to prevent walking list(s) on each core add/remove, generate a
* local mapping from "core client id" to "memc client id". This should be done
* offline?
* Also sets some "intial" values... mainly HVD is allowed non-exclusive access
* to all URR's (and CRR in test code).
*/
static BERR_Code generateCoreMap(void)
{
    int i,j;
    BAVC_CoreId coreId;
    BCHP_MemcClient clientId;

    /* Non-zero init */
    for(i=0;i<BAVC_CoreId_eMax;i++)
    {
        for(j=0;j<MAX_CLIENTS;j++)
        {
            local_info.coreMap[i].client[j]=BCHP_MemcClient_eMax;
        }
    }

    for(i=0;g_ClientMapTbl[i].eClientId!=BCHP_MemcClient_eMax;i++)
    {
        coreId=g_ClientMapTbl[i].eCoreId;
        clientId=g_ClientMapTbl[i].eClientId;

        switch(coreId)
        {
            case BAVC_CoreId_eHVD_0:
#ifdef BCHP_HEVD_OL_CPU_REGS_1_REG_START
            case BAVC_CoreId_eHVD_1:
#endif
#ifdef BCHP_HEVD_OL_CPU_REGS_2_REG_START
            case BAVC_CoreId_eHVD_2:
#endif
                /* HVD should be added to all memc's non-exclusive mask by default */
                for(j=0;j<BCHP_P_MEMC_COUNT;j++)
                {
                    addClient(BMRC_P_astClientTbl[clientId].ausClientId[j],
                              true, local_info.gConfig[j].nonexclusive,
                              local_info.gConfig[j].nonexclusive, BAVC_Access_eRW);
                }
                coreId=BAVC_CoreId_eNOT_MAP;
                break;
            default:
                break;
        }

        if(coreId==BAVC_CoreId_eNOT_MAP)
            continue;

        for(j=0;j<MAX_CLIENTS;j++)
        {
            if(local_info.coreMap[coreId].client[j]==BCHP_MemcClient_eMax)
            {
                local_info.coreMap[coreId].client[j]=clientId;
                break;
            }
        }

        if(j>=MAX_CLIENTS)
        {
            BDBG_ERR(("MAX_CLIENTS exceeded!"));
            return BERR_UNKNOWN;
        }
    }

    return BERR_SUCCESS;
}

static void dumpArcInfo(void)
{
    int i, j;

    for(i=0;i<ARC_IDX_MAX;i++)
    {
        if(local_info.gConfig[i].len==0)
        {
            BDBG_MSG(("MEMC%d: N/A", i));
            continue;
        }

        BDBG_MSG(("MEMC%d: 0x%08x@" BDBG_UINT64_FMT, i, local_info.gConfig[i].len,
                  BDBG_UINT64_ARG(local_info.gConfig[i].start)));
        for(j=0;j<CLIENT_INDEX_SIZE;j++)
        {
            BDBG_MSG(("\tE%d:NE%d\tW - 0x%08x:0x%08x\tR - 0x%08x:0x%08x", j, j,
                      local_info.gConfig[i].exclusive[j].write,
                      local_info.gConfig[i].nonexclusive[j].write,
                      local_info.gConfig[i].exclusive[j].read,
                      local_info.gConfig[i].nonexclusive[j].read));
        }
    }

    for (i=0;i<BAVC_CoreId_eMax;i++)
    {
        if(local_info.coreList.aeCores[i])
        {
            BDBG_MSG(("CORE id[%d] - (%d)", i, local_info.coreList.aeCores[i]));
        }
    }
}

static void dumpGisbViolation(void)
{
    uint32_t status;
    uint32_t address;
    uint32_t master;
    master=BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_GISB_ARB_BP_CAP_MASTER);
    address=BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_GISB_ARB_BP_CAP_ADDR);
    status=BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_GISB_ARB_BP_CAP_STATUS);

    if(status&BCHP_SUN_GISB_ARB_BP_CAP_STATUS_valid_MASK)
    {
        BDBG_ERR(("GISB %s violation at address 0x%08x. MASTER=0x%08x",
            (status&BCHP_SUN_GISB_ARB_BP_CAP_STATUS_write_MASK) ? "WRITE" : "READ",
            address, master));
        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_GISB_ARB_BP_CAP_CLR, 0x1);
    }
}

static void dumpArcViolation(void)
{
    uint32_t val;

    VIOLATION_SHOW(0, 0, val);
    VIOLATION_SHOW(0, 1, val);
#if BCHP_P_MEMC_COUNT > 1
    VIOLATION_SHOW(1, 0, val);
    VIOLATION_SHOW(1, 1, val);
#endif
#if BCHP_P_MEMC_COUNT > 2
    VIOLATION_SHOW(2, 0, val);
    VIOLATION_SHOW(2, 1, val);
#endif

    /* For SRR and CRR */
    VIOLATION_SHOW(0, 3, val);
    VIOLATION_SHOW(0, 5, val);
}

static void v3d_gisb(bool on)
{
    uint32_t start;
    uint32_t end;
    uint32_t mask;
    uint32_t enable;

#if defined(BCHP_V3D_DBG_REG_START)
    start = BCHP_PHYSICAL_OFFSET + BCHP_V3D_DBG_REG_START;
    end = BCHP_PHYSICAL_OFFSET + BCHP_V3D_DBG_REG_END;
#elif defined(BCHP_V3D_QPUDBG_REG_START)
    start = BCHP_PHYSICAL_OFFSET + BCHP_V3D_QPUDBG_REG_START;
    end = BCHP_PHYSICAL_OFFSET + BCHP_V3D_QPUDBG_REG_END;
#elif defined(BCHP_V3D_QPUDBG_0_REG_START)
    start = BCHP_PHYSICAL_OFFSET + BCHP_V3D_QPUDBG_0_REG_START;
    end = BCHP_PHYSICAL_OFFSET + BCHP_V3D_QPUDBG_0_REG_END;
#else
#error non-supported 3D HW
#endif

    if(on)
    {
        BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_GISB_ARB_BP_CAP_CLR, 0x1);
        enable=0x7;
    }
    else
    {
        enable=0x0;
    }

    mask=0xFFFFFFFF;
    mask &= ~(BCHP_FIELD_DATA(SUN_GISB_ARB_BP_READ_0, bsp_0, 1));
    mask &= ~(BCHP_FIELD_DATA(SUN_GISB_ARB_BP_READ_0, scpu_0, 1));
    mask &= ~(BCHP_FIELD_DATA(SUN_GISB_ARB_BP_READ_0, reserved0, 1));
    mask &= ~(BCHP_FIELD_DATA(SUN_GISB_ARB_BP_READ_0, reserved1, 1));
    mask &= ~(BCHP_FIELD_DATA(SUN_GISB_ARB_BP_READ_0, reserved2, 1));
    mask &= ~(BCHP_FIELD_DATA(SUN_GISB_ARB_BP_READ_0, reserved3, 1));

    BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_GISB_ARB_BP_START_ADDR_0, start);
    BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_GISB_ARB_BP_END_ADDR_0, end);
    BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_GISB_ARB_BP_READ_0, mask);
    BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_GISB_ARB_BP_WRITE_0, mask);

    BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_GISB_ARB_BP_ENABLE_0, enable);

    BDBG_MSG(("%s V3D GISB BLOCKER: (0x%08x - 0x%08x)", on ? "ENABLE" :"DISABLE", start, end));
}

static void bvnMonitor_Uninit(void)
{
    local_info.thread_stop = true;
    if(local_info.hBvnThread)
    {
        NEXUS_Thread_Destroy(local_info.hBvnThread);
        local_info.hBvnThread = NULL;
    }

    if(local_info.hBvnMonitor)
    {
        BBVN_Monitor_Uninit(local_info.hBvnMonitor);
        local_info.hBvnMonitor = NULL;
    }
}

static void NEXUS_Sage_P_BvnMonitorThread(void *dummy)
{
    BSTD_UNUSED(dummy);

    while(!local_info.thread_stop)
    {
        /* Overload usage of this thread... until/unless irq hooked up, poll the ARC's for violations */
        dumpArcViolation();

        dumpGisbViolation();

        /* BVN Monitoring */
        BKNI_AcquireMutex(local_info.bvnLock);
        BBVN_Monitor_Check(local_info.hBvnMonitor, &local_info.coreList, &local_info.bvnStatus);
        BKNI_ReleaseMutex(local_info.bvnLock);
        if(local_info.bvnStatus.bViolation)
        {
            BDBG_ERR(("BVN VIOLATION!"));
            dumpArcInfo();
        }

        /* MV Monitoring */
        if (MV_Monitor_Check(g_pCoreHandles->reg))
        {
            BDBG_ERR(("Macrovision VIOLATION!"));
        }

        BKNI_Sleep(ISR_RATE);
    }

    BDBG_LOG(("Thread terminating"));

    return;
}

static int bvnMonitor_Init(void)
{
    NEXUS_ThreadSettings thSettings;

    /* Init */
    BBVN_Monitor_Init(&local_info.hBvnMonitor, g_pCoreHandles->reg);
    if(!local_info.hBvnMonitor)
    {
        BDBG_ERR(("Failed to init BVN Monitor"));
        goto ERROR_EXIT;
    }

    /* For simplicity, just fire off a thread that sleeps and calls the "isr" */
    NEXUS_Thread_GetDefaultSettings(&thSettings);
    local_info.hBvnThread = NEXUS_Thread_Create("BVN Monitor", NEXUS_Sage_P_BvnMonitorThread,
                                           NULL, &thSettings);
    if (!local_info.hBvnThread)
    {
        BDBG_ERR(("NEXUS_Thread_Create(BVN Monitor) failed"));
        BERR_TRACE(NEXUS_NOT_INITIALIZED);
        goto ERROR_EXIT;
    }

    return NEXUS_SUCCESS;

ERROR_EXIT:
    bvnMonitor_Uninit();
    return NEXUS_NOT_INITIALIZED;
}

/* This would be in sage init sequence and would be unconditional */
/* i.e. the RR's are always locked per below when SAGE is running */
static int arc_Init(void)
{
    unsigned heapIndex;
    NEXUS_Error rc;

    /* TODO: For NEXUS testing, setup interrupts for ARC violations? */

    /* Retrieves picture heap buffer boundaries. */
    for (heapIndex=0;heapIndex<NEXUS_MAX_HEAPS;heapIndex++) {
        NEXUS_MemoryStatus status;
        NEXUS_HeapHandle heap = g_pCoreHandles->heap[heapIndex].nexus;

        if (!heap) continue;
        rc=NEXUS_Heap_GetStatus(heap, &status);
        if(rc!=NEXUS_SUCCESS)
        {
            continue;
        }

        if ((status.heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFERS) &&
            (status.memoryType & NEXUS_MEMORY_TYPE_SECURE))
        {
            if(local_info.gConfig[status.memcIndex].start!=0)
            {
                BDBG_ERR(("CANNOT HAVE MULTIPLE SECURE PICTURE BUFFERS per MEMC"));
                continue;
            }

            local_info.gConfig[status.memcIndex].start = status.offset;
            local_info.gConfig[status.memcIndex].len = status.size;

            BDBG_MSG(("ARC Lock 0x%x@" BDBG_UINT64_FMT, status.size, BDBG_UINT64_ARG(status.offset)));

            /* Secure HW can always access */
            addClient(BMRC_P_astClientTbl[BCHP_MemcClient_eBSP].ausClientId[status.memcIndex], 1,
                      local_info.gConfig[status.memcIndex].nonexclusive,
                      local_info.gConfig[status.memcIndex].nonexclusive, BAVC_Access_eRW);
            addClient(BMRC_P_astClientTbl[BCHP_MemcClient_eSCPU].ausClientId[status.memcIndex], 1,
                      local_info.gConfig[status.memcIndex].nonexclusive,
                      local_info.gConfig[status.memcIndex].nonexclusive, BAVC_Access_eRW);
        }
    }

    /* Second pass for adjacent buffers */
    for (heapIndex=0;heapIndex<NEXUS_MAX_HEAPS;heapIndex++) {
        NEXUS_MemoryStatus status;
        NEXUS_HeapHandle heap = g_pCoreHandles->heap[heapIndex].nexus;

        if (!heap) continue;
        rc=NEXUS_Heap_GetStatus(heap, &status);
        if(rc!=NEXUS_SUCCESS)
        {
            continue;
        }

        if (status.heapType & NEXUS_HEAP_TYPE_SECURE_GRAPHICS)
        {
            /* This is a "upper" adjacent heap, adjust size only */
            local_info.gConfig[status.memcIndex].len += status.size;
        }
        if (status.heapType & NEXUS_HEAP_TYPE_PICTURE_BUFFER_EXT)
        {
            /* This is a "lower" adjacent heap, adjust size and offset */
            local_info.gConfig[status.memcIndex].start = status.offset;
            local_info.gConfig[status.memcIndex].len += status.size;
        }
    }

    return NEXUS_SUCCESS;
}

static NEXUS_Error svpTestInit(void)
{
    NEXUS_Error rc;

    /* When running as a NEXUS test, custom_arc MUST be defined, otherwise the unsecure ARC's
    * will not be available for use */
    if(NEXUS_GetEnv("custom_arc"))
    {
        local_info.custom_arc=true;
    }
    else
    {
        local_info.init=true;
        BDBG_ERR(("Test code will NOT function if custom_arc is NOT defined!"));
        rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto EXIT;
    }

    BKNI_CreateMutex(&local_info.bvnLock);

    generateCoreMap();

    rc = bvnMonitor_Init();
    rc |= arc_Init();

    local_info.init=true;

EXIT:
    BDBG_MSG(("SVP Test Code: INIT %s", (rc==NEXUS_SUCCESS) ? "OK" : "FAIL"));
    return rc;
}

static void svpTestUnint(void)
{
    if(local_info.custom_arc)
    {
        bvnMonitor_Uninit();

        /* Ok to not do anything for ARC's, this is just test code */
        BKNI_DestroyMutex(local_info.bvnLock);
    }

    BKNI_Memset(&local_info, 0, sizeof(local_info));

    BDBG_MSG(("SVP Test Code: TERM"));
}

static int setArch(void)
{
    uint32_t offset,  val;
    int i, j, k;
    uint32_t excl, nonExcl, mode;
    struct maskInfo *mask;
    struct arc_reg_info arcInfo;

    excl=BCHP_FIELD_DATA(MEMC_ARC_0_ARC_0_CNTRL, WRITE_CHECK, 1);
    excl|=BCHP_FIELD_DATA(MEMC_ARC_0_ARC_0_CNTRL, READ_CHECK, 1);
    excl|=BCHP_FIELD_DATA(MEMC_ARC_0_ARC_0_CNTRL, READ_CMD_ABORT, 1);
    excl|=BCHP_FIELD_DATA(MEMC_ARC_0_ARC_0_CNTRL, WRITE_CMD_ABORT, 1);
    nonExcl=excl | BCHP_FIELD_DATA(MEMC_ARC_0_ARC_0_CNTRL, MODE, BCHP_MEMC_ARC_0_ARC_1_CNTRL_MODE_NON_EXCLUSIVE);
    excl|=BCHP_FIELD_DATA(MEMC_ARC_0_ARC_0_CNTRL, MODE, BCHP_MEMC_ARC_0_ARC_1_CNTRL_MODE_EXCLUSIVE);

    for(i=0;i<ARC_IDX_MAX;i++)
    {
        /* Skip "empty" arc's */
        if(local_info.gConfig[i].len==0)
            continue;

        switch(i)
        {
            case 0:
                SETINFO(0, 0, arcInfo);
                break;
#if BCHP_P_MEMC_COUNT > 1
            case 1:
                SETINFO(1, 0, arcInfo);
                break;
#endif
#if BCHP_P_MEMC_COUNT > 2
            case 2:
                SETINFO(2, 0, arcInfo);
                break;
#endif
            default:
                BDBG_ERR(("Unsupported MEMC"));
                return -1;
        }

        for(j=0;j<2;j++)
        {
            offset = j*ARC_DELTA;

            switch(j)
            {
                case 0:
                    mask = local_info.gConfig[i].exclusive;
                    mode = excl;
                    break;
                case 1:
                    mask = local_info.gConfig[i].nonexclusive;
                    mode = nonExcl;
                    break;
            }

            val=BREG_Read32(g_pCoreHandles->reg, arcInfo.cntrl + offset);
            if(val && mode)
            {
                /* Disable first. TODO: Not necessary? */
                BREG_Write32(g_pCoreHandles->reg, arcInfo.cntrl + offset, 0x0);
            }

            BREG_Write32(g_pCoreHandles->reg, arcInfo.low + offset, (uint32_t)(local_info.gConfig[i].start>>3));
            BREG_Write32(g_pCoreHandles->reg, arcInfo.high + offset, (uint32_t)((local_info.gConfig[i].start + local_info.gConfig[i].len - 1)>>3));
            for(k=0;k<CLIENT_INDEX_SIZE;k++)
            {
                BREG_Write32(g_pCoreHandles->reg, arcInfo.readStart + offset + (k*sizeof(uint32_t)), mask[k].read);
                BREG_Write32(g_pCoreHandles->reg, arcInfo.writeStart + offset + (k*sizeof(uint32_t)), mask[k].write);
            }

            /* Enable/Re-enable */
            BREG_Write32(g_pCoreHandles->reg, arcInfo.cntrl + offset, mode);
        }
    }

    return 0;
}

NEXUS_Error NEXUS_Sage_P_SecureCores_test(const BAVC_CoreList *pCoreList, bool add)
{
    int i,memc;
    uint16_t scbClient;
    BAVC_CoreId coreId;
    bool dirty=false;
    uint8_t count=0;

    /* TODO: This bit of code would normally be in SAGE init */
    if(!local_info.init)
    {
        if(svpTestInit()!=0)
            return NEXUS_NOT_AVAILABLE;
    }

    if(!local_info.custom_arc)
    {
        return NEXUS_NOT_AVAILABLE;
    }

    if(pCoreList->aeCores[BAVC_CoreId_eV3D_0])
    {
        count++;
    }
    if(pCoreList->aeCores[BAVC_CoreId_eV3D_1])
    {
        count++;
    }

    if(count)
    {
        if(add)
        {
            if(local_info.v3dCount==0)
            {
                v3d_gisb(add);
            }
            local_info.v3dCount+=count;
        }
        else
        {
            if(count>local_info.v3dCount)
            {
                BDBG_ERR(("INVALID V3D USAGE"));
            }
            local_info.v3dCount-=count;
            if(local_info.v3dCount==0)
            {
                v3d_gisb(add);
            }
        }
    }

    /* Stop BVN monitor while list is updated */
    BKNI_AcquireMutex(local_info.bvnLock);

    for (coreId=0;coreId<BAVC_CoreId_eMax;coreId++)
    {
        if (!pCoreList->aeCores[coreId])
            continue;

        /* Handle multiple references to a single core */
        if(add)
        {
            local_info.coreList.aeCores[coreId]++;
            if(local_info.coreList.aeCores[coreId]!=1)
                continue; /* No "change", carry on */
        }
        else
        {
            local_info.coreList.aeCores[coreId]--;
            if(local_info.coreList.aeCores[coreId]!=0)
                continue; /* No "change", carry on */
        }

        dirty=true;
        for(i=0;i<MAX_CLIENTS;i++)
        {

            scbClient=local_info.coreMap[coreId].client[i];

            if(scbClient==BCHP_MemcClient_eMax)
                break;

            BDBG_MSG(("%s Secure core [%d].%d (%d)", add ? "add" : "remove", coreId,
                scbClient, BMRC_P_astClientTbl[scbClient].ausClientId[0]));

            for(memc=0;memc<BCHP_P_MEMC_COUNT;memc++)
            {
                addClient(BMRC_P_astClientTbl[scbClient].ausClientId[memc], add,
                    local_info.gConfig[memc].exclusive,
                    local_info.gConfig[memc].nonexclusive, g_CoreIdAccess[coreId].eAccess);
            }
        }
    }

    /* Do actual ARC programming */
    if(dirty)
    {
        setArch();
    }

    /* Allow BVN monitor, list update complete */
    BKNI_ReleaseMutex(local_info.bvnLock);

    dumpArcInfo();

    /* TODO: Not needed when/if interrupt setup */
    if(!add)
    {
        /* Check if all cores removed and cleanup */
        for (i=0;i<BAVC_CoreId_eMax;i++)
        {
            if(local_info.coreList.aeCores[i])
                break;
        }
        if(i>=BAVC_CoreId_eMax)
        {
            svpTestUnint();
        }
    }

    return NEXUS_SUCCESS;
}
#else /* BCHP_MEMC_ARC_0_REG_START */
/* For simplicity, since this is limited test code, do not try to support
* MEMC_GEN_ARC_0... register chips */
#warning SVP TEST CODE NOT SUPPORTED ON THIS HW

NEXUS_Error NEXUS_Sage_P_SecureCores_test(const BAVC_CoreList *pCoreList, bool add)
{
    BSTD_UNUSED(pCoreList);
    BSTD_UNUSED(add);

    return NEXUS_NOT_SUPPORTED;
}
#endif /* BCHP_MEMC_ARC_0_REG_START */

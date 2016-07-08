/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
/*
 * This "macro" expects the following defines are available before including
 * this file into your C file:
 *
 * NEXUS_PLATFORM_P_SHARED_GPIO_SUPPORTED() - returns whether shared gpio is supported
 * NEXUS_PLATFORM_P_VIRTUAL_IRQ_STATE() - returns a pointer to an instance of NEXUS_Platform_P_VirtualIrqState
 */
#include "bchp_common.h"
#include "b_virtual_irq_types.h"

#ifdef BCHP_IRQ0_REG_START
#include "bchp_int_id_irq0.h"
#endif
#ifdef BCHP_IRQ0_AON_REG_START
#include "bchp_int_id_irq0_aon.h"
#endif
#ifdef BCHP_UPG_MAIN_IRQ_REG_START
#include "bchp_int_id_upg_main_irq.h"
#endif
#ifdef BCHP_UPG_BSC_IRQ_REG_START
#include "bchp_int_id_upg_bsc_irq.h"
#endif
#ifdef BCHP_UPG_MAIN_AON_IRQ_REG_START
#include "bchp_int_id_upg_main_aon_irq.h"
#endif
#ifdef BCHP_UPG_BSC_AON_IRQ_REG_START
#include "bchp_int_id_upg_bsc_aon_irq.h"
#endif
#ifdef BCHP_UPG_SPI_AON_IRQ_REG_START
/*#include "bchp_int_id_upg_spi_aon_irq.h" DOESN"T EXIST YET*/
#endif

#ifdef BCHP_INT_ID_IRQ0_gio_irqen
#define VIRTUAL_INT_ID_gio BCHP_INT_ID_IRQ0_gio_irqen
#elif defined(BCHP_INT_ID_UPG_MAIN_IRQ_gio)
#define VIRTUAL_INT_ID_gio BCHP_INT_ID_UPG_MAIN_IRQ_gio
#else
#define VIRTUAL_INT_ID_gio 0
#endif
#ifdef BCHP_INT_ID_IRQ0_AON_gio_irqen
#define VIRTUAL_INT_ID_gio_aon BCHP_INT_ID_IRQ0_AON_gio_irqen
#elif defined(BCHP_INT_ID_UPG_MAIN_AON_IRQ_gio)
#define VIRTUAL_INT_ID_gio_aon BCHP_INT_ID_UPG_MAIN_AON_IRQ_gio
#else
#define VIRTUAL_INT_ID_gio_aon 0
#endif
#ifdef BCHP_INT_ID_iica_irqen
#define VIRTUAL_INT_ID_iica BCHP_INT_ID_iica_irqen
#elif defined (BCHP_INT_ID_iica)
#define VIRTUAL_INT_ID_iica BCHP_INT_ID_iica
#else
#define VIRTUAL_INT_ID_iica 0
#endif
#ifdef BCHP_INT_ID_iicb_irqen
#define VIRTUAL_INT_ID_iicb BCHP_INT_ID_iicb_irqen
#elif defined(BCHP_INT_ID_iicb)
#define VIRTUAL_INT_ID_iicb BCHP_INT_ID_iicb
#else
#define VIRTUAL_INT_ID_iicb 0
#endif
#ifdef BCHP_INT_ID_iicc_irqen
#define VIRTUAL_INT_ID_iicc BCHP_INT_ID_iicc_irqen
#elif defined(BCHP_INT_ID_iicc)
#define VIRTUAL_INT_ID_iicc BCHP_INT_ID_iicc
#else
#define VIRTUAL_INT_ID_iicc 0
#endif
#ifdef BCHP_INT_ID_iicd_irqen
#define VIRTUAL_INT_ID_iicd BCHP_INT_ID_iicd_irqen
#elif defined(BCHP_INT_ID_iicd)
#define VIRTUAL_INT_ID_iicd BCHP_INT_ID_iicd
#else
#define VIRTUAL_INT_ID_iicd 0
#endif
#ifdef BCHP_INT_ID_iice_irqen
#define VIRTUAL_INT_ID_iice BCHP_INT_ID_iice_irqen
#elif defined(BCHP_INT_ID_iice)
#define VIRTUAL_INT_ID_iice BCHP_INT_ID_iice
#else
#define VIRTUAL_INT_ID_iice 0
#endif
#ifdef BCHP_INT_ID_iicf_irqen
#define VIRTUAL_INT_ID_iicf BCHP_INT_ID_iicf_irqen
#elif defined(BCHP_INT_ID_iicf)
#define VIRTUAL_INT_ID_iicf BCHP_INT_ID_iicf
#else
#define VIRTUAL_INT_ID_iicf 0
#endif
#ifdef BCHP_INT_ID_iicg_irqen
#define VIRTUAL_INT_ID_iicg BCHP_INT_ID_iicg_irqen
#elif defined(BCHP_INT_ID_iicg)
#define VIRTUAL_INT_ID_iicg BCHP_INT_ID_iicg
#else
#define VIRTUAL_INT_ID_iicg 0
#endif
#ifdef BCHP_INT_ID_spi_irqen
#define VIRTUAL_INT_ID_spi BCHP_INT_ID_spi_irqen
#elif defined(BCHP_INT_ID_spi)
#define VIRTUAL_INT_ID_spi BCHP_INT_ID_spi
#else
#if defined(BCHP_UPG_SPI_AON_IRQ_CPU_STATUS)
#define VIRTUAL_INT_ID_spi BCHP_INT_ID_CREATE(BCHP_UPG_SPI_AON_IRQ_CPU_STATUS, BCHP_UPG_SPI_AON_IRQ_CPU_STATUS_spi_SHIFT)
#else
#define VIRTUAL_INT_ID_spi 0
#endif
#endif
#ifdef BCHP_INT_ID_irb_irqen
#define VIRTUAL_INT_ID_irb BCHP_INT_ID_irb_irqen
#elif defined(BCHP_INT_ID_irb)
#define VIRTUAL_INT_ID_irb BCHP_INT_ID_irb
#else
#define VIRTUAL_INT_ID_irb 0
#endif
#ifdef BCHP_INT_ID_icap_irqen
#define VIRTUAL_INT_ID_icap BCHP_INT_ID_icap_irqen
#elif defined(BCHP_INT_ID_icap)
#define VIRTUAL_INT_ID_icap BCHP_INT_ID_icap
#else
#define VIRTUAL_INT_ID_icap 0
#endif
#ifdef BCHP_INT_ID_kbd1_irqen
#define VIRTUAL_INT_ID_kbd1 BCHP_INT_ID_kbd1_irqen
#elif defined(BCHP_INT_ID_kbd1)
#define VIRTUAL_INT_ID_kbd1 BCHP_INT_ID_kbd1
#else
#define VIRTUAL_INT_ID_kbd1 0
#endif
#ifdef BCHP_INT_ID_kbd2_irqen
#define VIRTUAL_INT_ID_kbd2 BCHP_INT_ID_kbd2_irqen
#elif defined(BCHP_INT_ID_kbd2)
#define VIRTUAL_INT_ID_kbd2 BCHP_INT_ID_kbd2
#else
#define VIRTUAL_INT_ID_kbd2 0
#endif
#ifdef BCHP_INT_ID_kbd3_irqen
#define VIRTUAL_INT_ID_kbd3 BCHP_INT_ID_kbd3_irqen
#elif defined(BCHP_INT_ID_kbd3)
#define VIRTUAL_INT_ID_kbd3 BCHP_INT_ID_kbd3
#else
#define VIRTUAL_INT_ID_kbd3 0
#endif
#ifdef BCHP_INT_ID_ldk_irqen
#define VIRTUAL_INT_ID_ldk BCHP_INT_ID_ldk_irqen
#elif defined(BCHP_INT_ID_ldk)
#define VIRTUAL_INT_ID_ldk BCHP_INT_ID_ldk
#else
#define VIRTUAL_INT_ID_ldk 0
#endif
#ifdef BCHP_INT_ID_ua_irqen
#define VIRTUAL_INT_ID_ua BCHP_INT_ID_ua_irqen
#else
#define VIRTUAL_INT_ID_ua 0
#endif
#ifdef BCHP_INT_ID_ub_irqen
#define VIRTUAL_INT_ID_ub BCHP_INT_ID_ub_irqen
#else
#define VIRTUAL_INT_ID_ub 0
#endif
#ifdef BCHP_INT_ID_uc_irqen
#define VIRTUAL_INT_ID_uc BCHP_INT_ID_uc_irqen
#else
#define VIRTUAL_INT_ID_uc 0
#endif

static const struct {
    uint32_t baseAddr;
    NEXUS_Platform_P_VirtualL2 virtualL2;
} g_NEXUS_Platform_P_BaseAddrVirtualL2Map[] = {
#ifdef BCHP_IRQ0_IRQEN
    {BCHP_IRQ0_IRQEN, NEXUS_Platform_P_VirtualL2_eIrq0},
#endif
#ifdef BCHP_IRQ0_AON_IRQEN
    {BCHP_IRQ0_AON_IRQEN, NEXUS_Platform_P_VirtualL2_eAonIrq0},
#endif
#ifdef BCHP_UPG_MAIN_IRQ_CPU_STATUS
    {BCHP_UPG_MAIN_IRQ_CPU_STATUS, NEXUS_Platform_P_VirtualL2_eUpgMainIrq},
#endif
#ifdef BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS
    {BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS, NEXUS_Platform_P_VirtualL2_eUpgMainAonIrq},
#endif
#ifdef BCHP_UPG_BSC_IRQ_CPU_STATUS
    {BCHP_UPG_BSC_IRQ_CPU_STATUS, NEXUS_Platform_P_VirtualL2_eUpgBscIrq},
#endif
#ifdef BCHP_UPG_BSC_AON_IRQ_CPU_STATUS
    {BCHP_UPG_BSC_AON_IRQ_CPU_STATUS, NEXUS_Platform_P_VirtualL2_eUpgBscAonIrq},
#endif
#ifdef BCHP_UPG_SPI_AON_IRQ_CPU_STATUS
    {BCHP_UPG_SPI_AON_IRQ_CPU_STATUS, NEXUS_Platform_P_VirtualL2_eUpgSpiAonIrq},
#endif
    {0, NEXUS_Platform_P_VirtualL2_eMax}
};

static const uint32_t g_NEXUS_Platform_P_VirtualL2GroupBaseAddresses[] = {
#ifdef BCHP_IRQ0_AON_IRQEN
    BCHP_IRQ0_AON_IRQEN,
#endif
#ifdef BCHP_IRQ0_IRQEN
    BCHP_IRQ0_IRQEN,
#endif
#ifdef BCHP_UPG_MAIN_IRQ_CPU_STATUS
    BCHP_UPG_MAIN_IRQ_CPU_STATUS,
#endif
#ifdef BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS
    BCHP_UPG_MAIN_AON_IRQ_CPU_STATUS,
#endif
#ifdef BCHP_UPG_BSC_IRQ_CPU_STATUS
    BCHP_UPG_BSC_IRQ_CPU_STATUS,
#endif
#ifdef BCHP_UPG_BSC_AON_IRQ_CPU_STATUS
    BCHP_UPG_BSC_AON_IRQ_CPU_STATUS,
#endif
#ifdef BCHP_UPG_SPI_AON_IRQ_CPU_STATUS
    BCHP_UPG_SPI_AON_IRQ_CPU_STATUS,
#endif
    0
};

static const BINT_Id g_NEXUS_Platform_P_DeactivatedL2IntIds[] = {
#ifdef BCHP_INT_ID_ua_irqen
    BCHP_INT_ID_ua_irqen, /* shared irq */
#endif
#ifdef BCHP_INT_ID_ub_irqen
    BCHP_INT_ID_ub_irqen, /* shared irq */
#endif
#ifdef BCHP_INT_ID_uc_irqen
    BCHP_INT_ID_uc_irqen, /* shared irq */
#endif
#ifdef BCHP_INT_ID_uarta_irqen
    BCHP_INT_ID_uarta_irqen, /* unshared irq */
#endif
#ifdef BCHP_INT_ID_uartb_irqen
    BCHP_INT_ID_uartb_irqen, /* unshared irq */
#endif
#ifdef BCHP_INT_ID_uartc_irqen
    BCHP_INT_ID_uartc_irqen, /* unshared irq */
#endif
    0
};

static const struct {
    BINT_Id intId;
    b_virtual_irq_line irqLine;
} g_NEXUS_Platform_P_VirtualL2IntIdIrqLineMap[] = {
    {VIRTUAL_INT_ID_gio,     b_virtual_irq_line_gio},
    {VIRTUAL_INT_ID_gio_aon, b_virtual_irq_line_gio_aon},
    {VIRTUAL_INT_ID_iica,    b_virtual_irq_line_iica},
    {VIRTUAL_INT_ID_iicb,    b_virtual_irq_line_iicb},
    {VIRTUAL_INT_ID_iicc,    b_virtual_irq_line_iicc},
    {VIRTUAL_INT_ID_iicd,    b_virtual_irq_line_iicd},
    {VIRTUAL_INT_ID_iice,    b_virtual_irq_line_iice},
    {VIRTUAL_INT_ID_iicf,    b_virtual_irq_line_iicf},
    {VIRTUAL_INT_ID_iicg,    b_virtual_irq_line_iicg},
    {VIRTUAL_INT_ID_spi,     b_virtual_irq_line_spi},
    {VIRTUAL_INT_ID_irb,     b_virtual_irq_line_irb},
    {VIRTUAL_INT_ID_icap,    b_virtual_irq_line_icap},
    {VIRTUAL_INT_ID_kbd1,    b_virtual_irq_line_kbd1},
    {VIRTUAL_INT_ID_kbd2,    b_virtual_irq_line_kbd2},
    {VIRTUAL_INT_ID_kbd3,    b_virtual_irq_line_kbd3},
    {VIRTUAL_INT_ID_ldk,     b_virtual_irq_line_ldk},
    {VIRTUAL_INT_ID_ua,      b_virtual_irq_line_ua},
    {VIRTUAL_INT_ID_ub,      b_virtual_irq_line_ub},
    {VIRTUAL_INT_ID_uc,      b_virtual_irq_line_uc},
    {0,                        b_virtual_irq_line_max}
};

bool NEXUS_Platform_P_VirtualIrqSupported(void)
{
    return NEXUS_PLATFORM_P_VIRTUAL_IRQ_STATE()->featureSupported;
}

typedef struct NEXUS_Platform_P_VirtualL2State {
    b_virtual_irq_line irqLines[32]; /* irq_line for each L2 shift */
} NEXUS_Platform_P_VirtualL2State;

static NEXUS_Platform_P_VirtualL2State g_NEXUS_Platform_P_VirtualL2State[NEXUS_Platform_P_VirtualL2_eMax];

static void NEXUS_Platform_P_InitVirtualIrqState(void)
{
    BKNI_Memset(&g_NEXUS_Platform_P_VirtualL2State, 0, sizeof(g_NEXUS_Platform_P_VirtualL2State));
}

b_virtual_irq_line NEXUS_Platform_P_GetL2IrqLine(NEXUS_Platform_P_VirtualL2 virtualL2, int shift)
{
    b_virtual_irq_line line = g_NEXUS_Platform_P_VirtualL2State[virtualL2].irqLines[shift];
    BDBG_ASSERT(line != b_virtual_irq_line_unused);
    return line;
}

const b_virtual_irq_line * NEXUS_Platform_P_GetL2IrqLines(NEXUS_Platform_P_VirtualL2 virtualL2)
{
    return g_NEXUS_Platform_P_VirtualL2State[virtualL2].irqLines;
}

static NEXUS_Platform_P_VirtualL2 NEXUS_Platform_P_L2RegisterIndex(uint32_t baseAddr)
{
    unsigned i;
    NEXUS_Platform_P_VirtualL2 virtualL2 = NEXUS_Platform_P_VirtualL2_eMax;

    for (i = 0; i < sizeof(g_NEXUS_Platform_P_BaseAddrVirtualL2Map)/sizeof(g_NEXUS_Platform_P_BaseAddrVirtualL2Map[0]); i++)
    {
        if (g_NEXUS_Platform_P_BaseAddrVirtualL2Map[i].baseAddr == baseAddr)
        {
            virtualL2 = g_NEXUS_Platform_P_BaseAddrVirtualL2Map[i].virtualL2;
            break;
        }
    }

    return virtualL2;
}

static void NEXUS_Platform_P_ClearInt( BREG_Handle regHandle, uint32_t baseAddr, int shift )
{
    NEXUS_Platform_P_VirtualL2 virtualL2 = NEXUS_Platform_P_L2RegisterIndex(baseAddr);
    if (virtualL2 == NEXUS_Platform_P_VirtualL2_eMax) {
        NEXUS_PLATFORM_P_VIRTUAL_IRQ_STATE()->intSettings.pClearInt(regHandle, baseAddr, shift);
    } else {
        BDBG_TRACE_L2(("ClearInt %#x:%u %d", baseAddr, virtualL2, shift));
        NEXUS_Platform_P_L2ClearInt(virtualL2, shift);
    }
    return;
}

static void NEXUS_Platform_P_SetMask( BREG_Handle regHandle, uint32_t baseAddr, int shift )
{
    NEXUS_Platform_P_VirtualL2 virtualL2 = NEXUS_Platform_P_L2RegisterIndex(baseAddr);
    if (virtualL2 == NEXUS_Platform_P_VirtualL2_eMax) {
        NEXUS_PLATFORM_P_VIRTUAL_IRQ_STATE()->intSettings.pSetMask(regHandle, baseAddr, shift);
    } else {
        BDBG_TRACE_L2(("SetMask %#x:%u %d", baseAddr, virtualL2, shift));
        NEXUS_Platform_P_L2Mask(virtualL2, shift, true);
    }
    return;
}

static void NEXUS_Platform_P_ClearMask( BREG_Handle regHandle, uint32_t baseAddr, int shift )
{
    NEXUS_Platform_P_VirtualL2 virtualL2 = NEXUS_Platform_P_L2RegisterIndex(baseAddr);

    if (virtualL2 == NEXUS_Platform_P_VirtualL2_eMax) {
        NEXUS_PLATFORM_P_VIRTUAL_IRQ_STATE()->intSettings.pClearMask(regHandle, baseAddr, shift);
    } else {
        BDBG_TRACE_L2(("ClearMask %#x:%u %d", baseAddr, virtualL2, shift));
        NEXUS_Platform_P_L2Mask(virtualL2, shift, false);
    }
    return;
}

static uint32_t NEXUS_Platform_P_ReadStatus( BREG_Handle regHandle, uint32_t baseAddr )
{
    NEXUS_Platform_P_VirtualL2 virtualL2 = NEXUS_Platform_P_L2RegisterIndex(baseAddr);
    uint32_t status;

    if (virtualL2 == NEXUS_Platform_P_VirtualL2_eMax) {
        status = NEXUS_PLATFORM_P_VIRTUAL_IRQ_STATE()->intSettings.pReadStatus(regHandle, baseAddr);
    } else {
        status = NEXUS_Platform_P_L2Status(virtualL2);
        BDBG_TRACE_L2(("ReadStatus %#x:%u -> %#x", baseAddr, virtualL2, status));
    }
    return status;
}

NEXUS_Error NEXUS_Platform_P_UpdateIntSettings(BINT_Settings *pIntSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned i;
    unsigned n_virtual;
    BINT_P_IntMap *localIntMap;

    if(!NEXUS_PLATFORM_P_VIRTUAL_IRQ_STATE()->featureSupported) return NEXUS_SUCCESS;

    for(i = 0; ; i++) {
        const BINT_P_IntMap *L2Register = &pIntSettings->pIntMap[i];
        if (L2Register->L1Shift < 0) {
            break;
        }
    }
    i++;

    NEXUS_Platform_P_InitVirtualIrqState();

    /* 1. Clear interrupts which are deactivated */
    localIntMap = BKNI_Malloc(i*sizeof(*localIntMap));
    if (localIntMap == NULL) { rc = NEXUS_OUT_OF_SYSTEM_MEMORY; BERR_TRACE(rc); goto err_alloc; }
    NEXUS_PLATFORM_P_VIRTUAL_IRQ_STATE()->localIntMap = localIntMap;
    BKNI_Memcpy(localIntMap, pIntSettings->pIntMap, i * sizeof(*localIntMap));

    for(i = 0; i < sizeof(g_NEXUS_Platform_P_DeactivatedL2IntIds)/sizeof(g_NEXUS_Platform_P_DeactivatedL2IntIds[0]); i++) {
        uint32_t intId = g_NEXUS_Platform_P_DeactivatedL2IntIds[i];
        uint32_t L2RegOffset = BCHP_INT_ID_GET_REG(intId);
        uint32_t L2Mask = 1 << BCHP_INT_ID_GET_SHIFT(intId);
        unsigned j;
        if (!intId) continue;
        for (j = 0; ; j++) {
            BINT_P_IntMap *L2Register = &localIntMap[j];
            if (L2Register->L1Shift < 0) {
                break;
            }
            if (BINT_MAP_IS_EXTERNAL(L2Register)) {
                continue;
            }
            if (L2Register->L2RegOffset == L2RegOffset) {
                if (L2Mask & ~L2Register->L2InvalidMask) {
                    L2Register->L2InvalidMask |= L2Mask;
                    break;
                }
            }
        }
    }

    /* 2. Map other interrupt to virtual L1 */
    for (n_virtual = 0, i = 0; i < sizeof(g_NEXUS_Platform_P_VirtualL2GroupBaseAddresses)/sizeof(g_NEXUS_Platform_P_VirtualL2GroupBaseAddresses[0]); i++) {
        unsigned j;
        uint32_t virtualL2 = g_NEXUS_Platform_P_VirtualL2GroupBaseAddresses[i];
        if (!virtualL2) continue;
        for (j = 0; ; j++) {
            const BINT_P_IntMap *L2Register = &localIntMap[j];
            unsigned k;
            if (L2Register->L1Shift < 0) {
                break;
            }
            if (BINT_MAP_IS_EXTERNAL(L2Register)) {
                continue;
            }
            if (L2Register->L2RegOffset == virtualL2) {
                b_virtual_irq_group group;
                unsigned L1Shift = BINT_MAP_GET_L1SHIFT(L2Register);
                uint32_t mask = ~L2Register->L2InvalidMask;
                NEXUS_Platform_P_VirtualL2 virtualL2Index = NEXUS_Platform_P_L2RegisterIndex(virtualL2);
                BDBG_ASSERT(virtualL2Index != NEXUS_Platform_P_VirtualL2_eMax);
                BDBG_TRACE_L2(("%s:%u L1:%u L2:%#x mask:%#x", L2Register->L2Name, j, L1Shift, virtualL2, mask));
                /* a. make sure L2 is not mapped to multiple L1s as this is not supported in virtualization context */
                for (k = j + 1; ; k++) {
                    L2Register = &localIntMap[k];
                    if (L2Register->L1Shift < 0) {
                        L2Register = NULL;
                        break;
                    }
                    if (L2Register->L2RegOffset == virtualL2) {
                        if (mask & (~L2Register->L2InvalidMask)) {
                            break;
                        }
                    }
                }
                if (L2Register) {
                    BDBG_ERR(("L2 register:%#x mapped to L1 '%s' %u(%#x) and '%s' %u(%#x)", (unsigned)virtualL2, localIntMap[j].L2Name, (unsigned)L1Shift, mask, L2Register->L2Name, (unsigned)BINT_MAP_GET_L1SHIFT(L2Register), ~L2Register->L2InvalidMask));
                    rc = NEXUS_NOT_SUPPORTED; BERR_TRACE(rc); goto err_invalid_irq;
                }

                /* b. set up group request to group virtual L2s into virtual L1 */
                BKNI_Memset(&group, 0, sizeof(group));
                group.l1_shift = L1Shift;
                for (k = 0; k < sizeof(g_NEXUS_Platform_P_VirtualL2IntIdIrqLineMap)/sizeof(g_NEXUS_Platform_P_VirtualL2IntIdIrqLineMap[0]); k++) {
                    BINT_Id intId = g_NEXUS_Platform_P_VirtualL2IntIdIrqLineMap[k].intId;
                    if (!intId) continue;
                    if (BCHP_INT_ID_GET_REG(intId) == virtualL2) {
                        unsigned L2Shift = BCHP_INT_ID_GET_SHIFT(intId);
                        BDBG_ASSERT(L2Shift <= sizeof(group.lines)/sizeof(group.lines[0]));
                        if (mask & (1 << L2Shift)) {
                            NEXUS_Platform_P_VirtualL2State *virtualL2State = &g_NEXUS_Platform_P_VirtualL2State[virtualL2Index];
                            BDBG_TRACE_L2(("%#x:%u -> %u %#x", virtualL2, L2Shift, g_NEXUS_Platform_P_VirtualL2IntIdIrqLineMap[k].irqLine, mask));
                            mask &= ~(1 << L2Shift);
                            n_virtual++;
                            group.lines[L2Shift] = g_NEXUS_Platform_P_VirtualL2IntIdIrqLineMap[k].irqLine;
                            virtualL2State->irqLines[L2Shift] = g_NEXUS_Platform_P_VirtualL2IntIdIrqLineMap[k].irqLine;
                            if
                            (
                                NEXUS_PLATFORM_P_SHARED_GPIO_SUPPORTED()
                                &&
                                (
                                    (g_NEXUS_Platform_P_VirtualL2IntIdIrqLineMap[k].irqLine == b_virtual_irq_line_gio)
                                    ||
                                    (g_NEXUS_Platform_P_VirtualL2IntIdIrqLineMap[k].irqLine == b_virtual_irq_line_gio_aon)
                                )
                            )
                            {
                                group.indirect[L2Shift] = g_NEXUS_Platform_P_VirtualL2IntIdIrqLineMap[k].irqLine;
                            }
                        }
                    }
                }
                BDBG_TRACE_L2(("L1: %u L2 %#x mask %#x %u", L1Shift, virtualL2, mask, n_virtual));
                if (mask != 0)
                {
                    BDBG_ERR(("L2 interrupt group %s has invalid, unmapped bits left in its mask: %#x.", localIntMap[j].L2Name, mask));
                    BDBG_ERR(("Please see Jira issue SWSTB-1509 for an explanation of this assert."));
                }
                BDBG_ASSERT(mask == 0); /* we should have handled/mapped all L2 interrupts */
                rc = NEXUS_Platform_P_MakeGroup(&group);
                if (rc) { BERR_TRACE(rc); goto err_group; }
            }
        }
    }

    if (n_virtual) {
        NEXUS_PLATFORM_P_VIRTUAL_IRQ_STATE()->intSettings = *pIntSettings;
        pIntSettings->pClearInt = NEXUS_Platform_P_ClearInt;
        pIntSettings->pSetMask = NEXUS_Platform_P_SetMask;
        pIntSettings->pClearMask = NEXUS_Platform_P_ClearMask;
        pIntSettings->pReadStatus = NEXUS_Platform_P_ReadStatus;
        pIntSettings->pIntMap = localIntMap;
    } else {
        BKNI_Free(localIntMap);
    }

    return NEXUS_SUCCESS;

err_group:
err_invalid_irq:
    BKNI_Free(localIntMap);
    NEXUS_PLATFORM_P_VIRTUAL_IRQ_STATE()->localIntMap=NULL;
err_alloc:
    return rc;
}

/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 ***************************************************************************/

#include "kernel.h"
#include "pgtable.h"
#include "platform.h"
#include "brcmstb.h"
#include "libfdt.h"
#include "parse_utils.h"
#include "lib_printf.h"
#include "uart.h"

// Register group enums
enum {
    STB_REG_GROUP_HIF_CPUBIUARCH,
    STB_REG_GROUP_HIF_CPUBIUCTRL,
    STB_REG_GROUP_HIF_CONTINUATION,
    STB_REG_GROUP_SUN_TOP_CTRL,
    STB_REG_GROUP_MEMC_SENTINEL,
    STB_REG_GROUP_MEMC_TRACELOG,
    STB_REG_GROUP_LAST
};

// Register group map
struct RegGroupEntry {
    const char *compatible;
    uint32_t addr;
    uint32_t size;
};

static struct RegGroupEntry regGroupMap[] = {
    {"brcm,brcmstb-cpu-biu-arch",           0, 0}, // STB_REG_GROUP_HIF_CPUBIUARCH,
    {"brcm,brcmstb-cpu-biu-ctrl",           0, 0}, // STB_REG_GROUP_HIF_CPUBIUCTRL,
    {"brcm,brcmstb-hif-continuation",       0, 0}, // STB_REG_GROUP_HIF_CONTINUATION,
    {"brcm,brcmstb-sun-top-ctrl",           0, 0}, // STB_REG_GROUP_SUN_TOP_CTRL,
    {"brcm,brcmstb-memc-sentinel",          0, 0}, // STB_REG_GROUP_MEMC_SENTINEL,
    {"brcm,brcmstb-memc-tracelog",          0, 0}, // STB_REG_GROUP_MEMC_TRACELOG,
    {NULL,                                  0, 0}  // STB_REG_GROUP_LAST
};

// Register map
struct RegEntry {
    uint32_t group;

    // Addr offset in byte
    // A negative offset indicates counting backward from end of group
    int32_t offset;
};

static struct RegEntry regMap[] {

    // !!! THE ORDER MUST MATCH REGISTER ENUM DEFINES !!!

    {STB_REG_GROUP_SUN_TOP_CTRL,          0x304}, // STB_SUN_TOP_CTRL_RESET_SOURCE_ENABLE
    {STB_REG_GROUP_SUN_TOP_CTRL,          0x308}, // STB_SUN_TOP_CTRL_SW_MASTER_RESET

    {STB_REG_GROUP_HIF_CONTINUATION,        0x0}, // STB_HIF_CONTINUATION_STB_BOOT_HI_ADDR0
    {STB_REG_GROUP_HIF_CONTINUATION,        0x4}, // STB_HIF_CONTINUATION_STB_BOOT_ADDR0

    {STB_REG_GROUP_HIF_CPUBIUCTRL,         0x88}, // STB_HIF_CPUBIUCTRL_CPU0_PWR_ZONE_CNTRL_REG
    {STB_REG_GROUP_HIF_CPUBIUCTRL,        0x178}, // STB_HIF_CPUBIUCTRL_CPU_RESET_CONFIG_REG

    {STB_REG_GROUP_HIF_CPUBIUARCH,          0x0}, // STB_HIF_CPUBIUARCH_ADDRESS_RANGE0_ULIMIT

    {STB_REG_GROUP_MEMC_SENTINEL,           0x0}, // STB_MEMC_TRACELOG_SENTINEL_RANGE_START
    {STB_REG_GROUP_MEMC_SENTINEL,          -0x4}, // STB_MEMC_TRACELOG_SENTINEL_RANGE_END

    {STB_REG_GROUP_MEMC_TRACELOG,           0x0}, // STB_MEMC_TRACELOG_VERSION,
    {STB_REG_GROUP_MEMC_TRACELOG,           0x4}, // STB_MEMC_TRACELOG_CONTROL,
    {STB_REG_GROUP_MEMC_TRACELOG,           0xc}, // STB_MEMC_TRACELOG_COMMAND,
    {STB_REG_GROUP_MEMC_TRACELOG,          0x50}, // STB_MEMC_TRACELOG_BUFFER_PTR,
    {STB_REG_GROUP_MEMC_TRACELOG,          0x54}, // STB_MEMC_TRACELOG_BUFFER_PTR_EXT,
    {STB_REG_GROUP_MEMC_TRACELOG,          0x58}, // STB_MEMC_TRACELOG_BUFFER_SIZE,
    {STB_REG_GROUP_MEMC_TRACELOG,          0x5C}, // STB_MEMC_TRACELOG_BUFFER_WR_PTR,
    {STB_REG_GROUP_MEMC_TRACELOG,          0x90}, // STB_MEMC_TRACELOG_TRIGGER_MODE,
    {STB_REG_GROUP_MEMC_TRACELOG,         0x100}, // STB_MEMC_TRACELOG_FILTER_MODEi_ARRAY_BASE,
    {STB_REG_GROUP_MEMC_TRACELOG,         0x140}, // STB_MEMC_TRACELOG_FILTER_ADDR_LOWERi_ARRAY_BASE,
    {STB_REG_GROUP_MEMC_TRACELOG,         0x160}, // STB_MEMC_TRACELOG_FILTER_ADDR_UPPERi_ARRAY_BASE,
    {STB_REG_GROUP_LAST,                    0x0}  // STB_REG_LAST
};

// system uart base
static uint32_t sys_uart = 0;

extern uint32_t uart_base;

// number of cpus
uint32_t num_cpus = 0;

static void platform_init_uart(void *devTree)
{
    int node;
    int parent;
    int propLen;
    const struct fdt_property *prop;

    // Get chosen node
    char *stdoutPath = NULL;

    node = fdt_subnode_offset(devTree, 0, "chosen");
    if (node < 0) {
        if (node != -FDT_ERR_NOTFOUND) {
            kernelHalt("Failed to parse chosen node in device tree");
        }
    }
    else {
        // Get stdout-path from chosen node
        prop = fdt_get_property(devTree, node, "astra,stdout-path", &propLen);
        if (!prop) {
            if (propLen != -FDT_ERR_NOTFOUND) {
                kernelHalt("Failed to parse stdout-path property in chosen node");
            }
            // Look for alternative name
            prop = fdt_get_property(devTree, node, "stdout-path", &propLen);
            if (!prop) {
                if (propLen != -FDT_ERR_NOTFOUND) {
                    kernelHalt("Failed to parse stdout-path property in chosen node");
                }
            }
        }

        if (prop) {
            // Trim off speed info
            char *pstr = (char *)prop->data;
            while (*pstr != '\0' && *pstr != ':') pstr++;
            *pstr = '\0';
        }
    }

    // Get serial node
    node = -1;

    if (stdoutPath) {
        node = fdt_path_offset(devTree, stdoutPath);
        if (node < 0) {
            warn_msg("Failed to find chosen serial node in device tree");
        }
    }

    if (node < 0) {
        // Use first ns16550a compatible node
        node = fdt_node_offset_by_compatible(devTree, 0, "ns16550a");
        if (node < 0) {
            warn_msg("Failed to find any ns16550a compatible node in device tree");
        }
    }

    if (node < 0) {
        // No uart found
        return;
    }

    // Get parent node
    parent = fdt_parent_offset(devTree, node);
    if (parent < 0) {
        kernelHalt("Failed to find parent node in device tree");
    }

    // Parse address and size cells of parent node
    int addrCells;
    int sizeCells;

    prop = fdt_get_property(devTree, parent, "#address-cells", &propLen);
    if (!prop || propLen < sizeof(int))
        addrCells = 1;
    else
        addrCells = parseInt((void *)prop->data, propLen);

    prop = fdt_get_property(devTree, parent, "#size-cells", &propLen);
    if (!prop || propLen < sizeof(int))
        sizeCells = 1;
    else
        sizeCells = parseInt((void *)prop->data, propLen);

    int addrBytes = addrCells * sizeof(int);
    int sizeBytes = sizeCells * sizeof(int);

    // Get serial address and size
    prop = fdt_get_property(devTree, node, "reg", &propLen);
    if (!prop || propLen != addrBytes + sizeBytes) {
        kernelHalt("Failed to parse reg property in serial node");
    }

    uint32_t serialAddr = (uint32_t)((addrCells == 1) ?
        parseInt(prop->data, addrBytes) :
        parseInt64(prop->data, addrBytes));

    // Since the entire page is mapped, the actual size doesn't matter.
    //
    // uint32_t serialSize = (uint32_t)((sizeCells == 1) ?
    //  parseInt((uint8_t *)prop->data + addrBytes, sizeBytes) :
    //  parseInt64((uint8_t *)prop->data + addrBytes, sizeBytes));

    // Map serial registers if necessary
    uint8_t *uartStart = (uint8_t *)PAGE_START_4K(uart_base);
    uint8_t *uartEnd = uartStart + PAGE_SIZE_4K_BYTES - 1;

    uint8_t *serialStart = (uint8_t *)PAGE_START_4K(serialAddr);
    uint8_t *serialEnd = serialStart + PAGE_SIZE_4K_BYTES - 1;

    PageTable *kernelPageTable = PageTable::kernelPageTable();

    if (serialStart != uartStart ||
        serialEnd != uartEnd) {
        kernelPageTable->mapPageRange(
            serialStart, serialEnd, serialStart,
            MAIR_DEVICE, MEMORY_ACCESS_RW_KERNEL, true, false);
    }

    // Hold off uart switching till entire system init done
    sys_uart = serialAddr;
}

static void platform_init_regs(void *devTree)
{
    int node;
    int parent;
    int depth;
    int propLen;
    const struct fdt_property *prop;

    // Sanity check register group and register maps
    if (sizeof(regMap) / sizeof(RegEntry) != STB_REG_LAST + 1) {
        kernelHalt("Register enums and register map mismatch");
    }
    if (sizeof(regGroupMap) / sizeof(RegGroupEntry) != STB_REG_GROUP_LAST + 1) {
        kernelHalt("Register group enums and register group map mismatch");
    }

    // Get rdb node as parent
    parent = fdt_subnode_offset(devTree, 0, "rdb");
    if (parent < 0) {
        kernelHalt("Failed to find rdb node in device tree");
    }

    // Parse address and size cells of parent node
    int addrCells;
    int sizeCells;

    prop = fdt_get_property(devTree, parent, "#address-cells", &propLen);
    if (!prop || propLen < sizeof(int))
        addrCells = 1;
    else
        addrCells = parseInt((void *)prop->data, propLen);

    prop = fdt_get_property(devTree, parent, "#size-cells", &propLen);
    if (!prop || propLen < sizeof(int))
        sizeCells = 1;
    else
        sizeCells = parseInt((void *)prop->data, propLen);

    int addrBytes = addrCells * sizeof(int);
    int sizeBytes = sizeCells * sizeof(int);

    // Go through all subnodes
    node = parent;
    depth = 0;
    while (1) {
        node = fdt_next_node(devTree, node, &depth);
        if (depth <= 0)
            break;

        if (depth > 1)
            continue;

        // Check compatible for syscon
        if (fdt_node_check_compatible(devTree, node, "syscon"))
            continue;

        // Find compatible string
        prop = fdt_get_property(devTree, node, "compatible", &propLen);
        if (!prop) {
            kernelHalt("Failed to parse compatible property in syscon node");
        }

        // Find matching register group
        uint32_t regGroup;
        for (regGroup = 0; regGroup < STB_REG_GROUP_LAST; regGroup++) {
            if (!strcmp(prop->data, regGroupMap[regGroup].compatible))
                break;
        }

        if (regGroup == STB_REG_GROUP_LAST)
            continue;

        // Get register group address and size
        prop = fdt_get_property(devTree, node, "reg", &propLen);
        if (!prop || propLen != addrBytes + sizeBytes) {
            kernelHalt("Failed to parse reg property in syscon node");
        }

        uint32_t regGroupAddr = (uint32_t)((addrCells == 1) ?
            parseInt(prop->data, addrBytes) :
            parseInt64(prop->data, addrBytes));

        uint32_t regGroupSize = (uint32_t)((sizeCells == 1) ?
            parseInt((uint8_t *)prop->data + addrBytes, sizeBytes) :
            parseInt64((uint8_t *)prop->data + addrBytes, sizeBytes));

        // Map register group
        uint8_t *regGroupStart = (uint8_t *)PAGE_START_4K(regGroupAddr);
        uint8_t *regGroupEnd = (uint8_t *)(regGroupAddr + regGroupSize - 1);

        PageTable *kernelPageTable = PageTable::kernelPageTable();

        kernelPageTable->mapPageRange(
            regGroupStart, regGroupEnd, regGroupStart,
            MAIR_DEVICE, MEMORY_ACCESS_RW_KERNEL, true, false);

        regGroupMap[regGroup].addr = regGroupAddr;
        regGroupMap[regGroup].size = regGroupSize;
    }
}

static void platform_init_cpus(void *devTree)
{
    int node;
    int parent;
    int depth;
    int propLen;
    const struct fdt_property *prop;

    // Get cpus node as parent
    parent = fdt_subnode_offset(devTree, 0, "cpus");
    if (parent < 0) {
        kernelHalt("Failed to find cpus node in device tree");
    }

    // Go through all subnodes
    node = parent;
    depth = 0;
    while (1) {
        node = fdt_next_node(devTree, node, &depth);
        if (depth <= 0)
            break;

        if (depth > 1)
            continue;

        // Check device_type for cpu
        prop = fdt_get_property(devTree, node, "device_type", &propLen);
        if (!prop ||
            strcmp((char *)prop->data, "cpu"))
            continue;

        num_cpus++;
    }
}

void Platform::init(void *devTree)
{
    // Sanity check device tree
    if (fdt_check_header(devTree)) {
        kernelHalt("Invalid device tree header");
    }

    platform_init_uart(devTree);
    platform_init_regs(devTree);
    platform_init_cpus(devTree);
}

void Platform::setUart()
{
    if (sys_uart != uart_base) {

        info_msg("Switching to uart @ 0x%x...", (unsigned int)sys_uart);

        // Don't unmap serial registers from the page table.
        // There may be other registers sharing the same page.

        uart_base = sys_uart;
        uart_init();
    }
}

bool Platform::hasUart(void)
{
    return (sys_uart ? true : false);
}

/*
 * STB specific functions
 */

uint32_t stb_reg_addr(uint32_t reg) {
    if (reg >= STB_REG_LAST)
        return 0;

    struct RegEntry &regEntry = regMap[reg];
    struct RegGroupEntry &regGroupEntry = regGroupMap[regEntry.group];

    if (regEntry.offset >= 0) {
        return (regEntry.offset < regGroupEntry.size) ?
            regGroupEntry.addr + regEntry.offset : 0;
    }
    else {
        // negative offset, counting backward from end of group
        return (-regEntry.offset < regGroupEntry.size) ?
            regGroupEntry.addr + regGroupEntry.size + regEntry.offset : 0;
    }
}

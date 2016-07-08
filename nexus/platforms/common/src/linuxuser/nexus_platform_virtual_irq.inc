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
 */
#include "../nexus_platform_virtual_irq_common.h"

static NEXUS_Platform_P_VirtualIrqState g_NEXUS_Platform_P_VirtualIrqState;

bool NEXUS_Platform_P_IsVirtualIrq(const char * name)
{
    BSTD_UNUSED(name);
    return false;
}

void NEXUS_Platform_P_InitVirtualIrqSubmodule(void)
{
    int u_rc;
    bcmdriver_os_config os_cfg;

    BKNI_Memset(&os_cfg, 0, sizeof(os_cfg));
    u_rc = ioctl(g_NEXUS_driverFd, BRCM_IOCTL_GET_OS_CONFIG, &os_cfg);
    if (u_rc) {
        BDBG_WRN(("Nexus requires a new bcmdriver.ko ioctl API BRCM_IOCTL_GET_OS_CONFIG. Are you running with an old bcmdriver.ko?"));
    }
    g_NEXUS_Platform_P_VirtualIrqState.featureSupported = os_cfg.virtual_irq_supported == 1;
    BDBG_WRN(("virtual l2 %s", g_NEXUS_Platform_P_VirtualIrqState.featureSupported ? "enabled" : "disabled"));
}

void NEXUS_Platform_P_UninitVirtualIrqSubmodule(void)
{
    if (g_NEXUS_Platform_P_VirtualIrqState.localIntMap) {
        BKNI_Free(g_NEXUS_Platform_P_VirtualIrqState.localIntMap);
        g_NEXUS_Platform_P_VirtualIrqState.localIntMap=NULL;
    }
}

static void NEXUS_Platform_P_L2ClearInt(NEXUS_Platform_P_VirtualL2 virtualL2, int shift)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    int u_rc;
    bcmdriver_irq_control control;
    control.command = bcmdriver_irq_command_clear;
    control.data.clear.line = NEXUS_Platform_P_GetL2IrqLine(virtualL2, shift);
    u_rc = ioctl(g_NEXUS_driverFd, BRCM_IOCTL_IRQ_CONTROL, &control);
    if (u_rc) { rc = NEXUS_OS_ERROR; BERR_TRACE(rc); goto end; }

end:
    return;
}

static void NEXUS_Platform_P_L2Mask(NEXUS_Platform_P_VirtualL2 virtualL2, int shift, bool disable)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    int u_rc;
    bcmdriver_irq_control control;
    control.command = bcmdriver_irq_command_mask;
    control.data.mask.line = NEXUS_Platform_P_GetL2IrqLine(virtualL2, shift);
    control.data.mask.disable = disable ? 1 : 0;
    u_rc = ioctl(g_NEXUS_driverFd, BRCM_IOCTL_IRQ_CONTROL, &control);
    if (u_rc) { rc = NEXUS_OS_ERROR; BERR_TRACE(rc); goto end; }

end:
    return;
}

static uint32_t NEXUS_Platform_P_L2Status(NEXUS_Platform_P_VirtualL2 virtualL2)
{
    bcmdriver_irq_control control;
    control.command = bcmdriver_irq_command_status;
    BKNI_Memcpy(control.data.status.lines, NEXUS_Platform_P_GetL2IrqLines(virtualL2), sizeof(control.data.status.lines));
    control.data.status.status_word = 0;
    (void)ioctl(g_NEXUS_driverFd, BRCM_IOCTL_IRQ_CONTROL, &control);
    return control.data.status.status_word;
}

static NEXUS_Error NEXUS_Platform_P_MakeGroup(const b_virtual_irq_group * group)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    int u_rc;

    u_rc = ioctl(g_NEXUS_driverFd, BRCM_IOCTL_IRQ_MAKE_GROUP, group);
    if (u_rc) { rc = NEXUS_OS_ERROR; BERR_TRACE(rc); goto end; }

end:
    return rc;
}

#define NEXUS_PLATFORM_P_VIRTUAL_IRQ_STATE() (&g_NEXUS_Platform_P_VirtualIrqState)
#include "../nexus_platform_virtual_irq_common.inc"

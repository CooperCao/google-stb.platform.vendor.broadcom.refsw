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

#ifndef GIC_H_
#define GIC_H_

#include "arm/arm.h"
#include "arm/spinlock.h"

/* Table8-1 CortexA15 TRM */
#define ARM_CORTEX_A15_GICD_BASE  0x1000
#define ARM_CORTEX_A15_GICC_BASE  0x2000
#define ARM_CORTEX_A15_GICH_BASE  0x4000
#define ARM_CORTEX_A15_GICHP_BASE 0x5000
#define ARM_CORTEX_A15_GICV_BASE  0x6000

#define ARM_CORTEX_A15_SECURE_TIMER_IRQ   29

#define GIC_MEMMAP_SIZE           0xA000

#define TZ_SGI_IRQ                15
#define TZ_INVALID_IRQ            1024

class GIC {
public:
    static void init(void *devTree);
    static void secondaryCpuInit();

    static void intrDisable(int irq);
    static void intrEnable(int irq);
    static void sgiGenerate(int irq);

    static int currIntr(int *srcCpu);
    static void endIntr(int irq);

public:
    static void *distributorBase;
    static void *cpuInterfaceBase;

    static spinlock_t lock;

private:
};

/* WORD Offsets to get to individual GIC distributor registers */
#define GICD_CTL            (0x0000/4)
#define GICD_TYPE           (0x0004/4)
#define GICD_ID             (0x0008/4)
#define GICD_IGROUP         (0x0080/4)
#define GICD_ISENABLE       (0x0100/4)
#define GICD_ICENABLE       (0x0180/4)
#define GICD_ISPEND         (0x0200/4)
#define GICD_ICPEND         (0x0280/4)
#define GICD_ISACTIVE       (0x0300/4)
#define GICD_ICACTIVE       (0x0380/4)
#define GICD_IPRIORITY      (0x0400/4)
#define GICD_ITARGETS       (0x0800/4)
#define GICD_ICFG           (0x0C00/4)
#define GICD_PPIS           (0x0D00/4)
#define GICD_SPIS           (0x0D04/4)
#define GICD_SGI            (0x0F00/4)
#define GICD_CPENDSGI       (0x0F10/4)
#define GICD_SPENDSGI       (0x0F20/4)
#define GICD_PIDR4          (0x0FD0/4)
#define GICD_PIDR5          (0x0FD4/4)
#define GICD_PIDR6          (0x0FD8/4)
#define GICD_PIDR7          (0x0FDC/4)
#define GICD_PIDR0          (0x0FE0/4)
#define GICD_PIDR1          (0x0FE4/4)
#define GICD_PIDR2          (0x0FE8/4)
#define GICD_PIDR3          (0x0FEC/4)
#define GICD_CIDR0          (0x0FF0/4)
#define GICD_CIDR1          (0x0FF4/4)
#define GICD_CIDR2          (0x0FF8/4)
#define GICD_CIDR3          (0x0FFC/4)

#define GICD_REGION_SIZE    4096

/* WORD Offsets to get to individual GIC cpu interface registers */
#define GICC_CTLR            (0x0000/4)
#define GICC_PMR             (0x0004/4)
#define GICC_BPR             (0x0008/4)
#define GICC_IAR             (0x000C/4)
#define GICC_EOIR            (0x0010/4)
#define GICC_RPR             (0x0014/4)
#define GICC_HPPIR           (0x0018/4)
#define GICC_ABPR            (0x001C/4)
#define GICC_AIAR            (0x0020/4)
#define GICC_AEOIR           (0x0024/4)
#define GICC_AHPPIR          (0x0028/4)
#define GICC_APR0           (0x00D0/4)
#define GICC_NSAPR0         (0x00E0/4)
#define GICC_IIDR            (0x00FC/4)
#define GICC_DIR             (0x1000/4)

/* WORD Offsets to get to individual GIC virtual interface interface registers */
#define GICH_HCR             (0x0000/4)
#define GICH_VT             (0x0004/4)
#define GICH_VMCR           (0x0008/4)
#define GICH_MISR          (0x0010/4)
#define GICH_EISR0            (0x0020/4)
#define GICH_ELSR0           (0x0030/4)
#define GICH_AP             (0x00F0/4)
#define GICH_LR0             (0x0100/4)
#endif /* GIC_H_ */

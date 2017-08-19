/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

/*
 * gic.h
 *
 *  Created on: Oct 30, 2015
 *      Author: gambhire
 */

#ifndef ARCH_AARCH64_GIC_GIC_H_
#define ARCH_AARCH64_GIC_GIC_H_

#include "arm/arm.h"
#include <stdint.h>

#define GICD_CTLR				0x000
#define GICD_TYPER				0x004
#define GICD_IIDR				0x008
#define GICD_IGROUPR			0x080
#define GICD_ISENABLER			0x100
#define GICD_ICENABLER			0x180
#define GICD_ISPENDR			0x200
#define GICD_ICPENDR			0x280
#define GICD_ISACTIVER			0x300
#define GICD_ICACTIVER			0x380
#define GICD_IPRIORITYR			0x400
#define GICD_ITARGETSR			0x800
#define GICD_ICFGR				0xC00
#define GICD_NSACR				0xE00
#define GICD_SGIR				0xF00
#define GICD_CPENDSGIR			0xF10
#define GICD_SPENDSGIR			0xF20

#define GICD_ISENABLER_NUM_BITS		32
#define GICD_ICENABLER_NUM_BITS		32

#define GICC_CTLR				0x0000
#define GICC_PMR				0x0004
#define GICC_BPR				0x0008
#define GICC_IAR				0x000C
#define GICC_EOIR				0x0010
#define GICC_RPR				0x0014
#define GICC_HPPIR				0x0018
#define GICC_AIAR				0x0020
#define GICC_AEOIR				0x0024
#define GICC_AHPPIR				0x0028
#define GICC_APR				0x00D0
#define GICC_NSAPR				0x00E0
#define GICC_IIDR				0x00FC
#define GICC_DIR				0x1000

#define GICC_IAR_CPUID_BITPOS		10
#define GICC_IAR_CPUID_MASK			0x7
#define GICC_IAR_INTID_BITPOS   	0
#define GICC_IAR_INTID_MASK			0x3ff

#define GICC_EOIR_CPUID_BITPOS		10
#define GICC_EOIR_CPUID_MASK		0x7
#define GICC_EOIR_EOIINTID_BITPOS   0
#define GICC_EOIR_EOIINTID_MASK		0x3ff

int gicV2Init(void *deviceTree);
void gicV2InitSecondary();

uint32_t gicV2GetInterruptID(uint32_t *sourceCPU);
void gicV2InterruptProcessed(uint32_t intrId, uint32_t sourceCPU);

void gicV2InterruptEnable(uint32_t intrId);
void gicV2InterruptDisable(uint32_t intrId);
uint32_t gicV2currIntr(uint32_t *srcCpu);
void gicV2sgiGenerate(uint8_t cpuTargetList, uint32_t intrId);
#endif /* ARCH_AARCH64_GIC_GIC_H_ */

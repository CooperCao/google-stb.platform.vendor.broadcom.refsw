/***************************************************************************
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
 ***************************************************************************/

#include "arm/arm.h"
#include "arm/gic.h"

#include "gicv2.h"
#include "lib_printf.h"
#include "libfdt.h"

SpinLock GIC::lock;

#define kassert
#if 0
enum GicType {
	Unknown,
	GicV2,
	GicV3,
	GicV4
};

GicType gicType(void *devTree) {

	int nodeOffset = fdt_subnode_offset(devTree, 0, "interrupt-controller");
	kassert(nodeOffset > 0);

	// Parse the"compatible" property.
	int propLen = 0;
	const struct fdt_property *fpCompatible =
			fdt_get_property(devTree, nodeOffset, "compatible", &propLen);
	kassert((fpCompatible != nullptr) && ((unsigned int)propLen >= sizeof(int)));

	char *nextStr = (char *)fpCompatible->data;
	char *end = nextStr + propLen;
	while (nextStr < end) {
		if (substr(nextStr, "cortex-a15-gic") ||
			substr(nextStr, "brahma-b15-gic") ||
			substr(nextStr, "gic-400")) {

			return GicType::GicV2;
		}

		if (substr(nextStr, "gic-v3"))
			return GicType::GicV3;

		nextStr += strlen(nextStr) + 1;
	}

	return GicType::Unknown;
}
#endif
void GIC::init(void *devTree) {

	//GicType gic=gicType(devTree);
	//if (gic != GicType::GicV2) {
	//	printf("GIC type not supported\n");
	//	ARCH_HALT();
	//}

	if (arm::smpCpuNum() != 0) {
		secondaryCpuInit();
		return;
	}

	gicV2Init(devTree);

	spinLockInit(&lock);

    printf("GIC primary init done\n");
}

void GIC::secondaryCpuInit() {
	gicV2InitSecondary();
}

void GIC::intrDisable(int irq) {
	SpinLocker locker(&lock);

	if ((irq != ARCH_SPECIFIC_TIMER_INTERRUPT) && (irq != TZ_SGI_IRQ)
		&& (irq != TZ_SECURE_SGI_IRQ)) {
		err_msg("TZ: Attempt to disable a normal world interrupt disallowed\n");
		return;
	}

	gicV2InterruptDisable(irq);

}

void GIC::intrEnable(int irq) {
	SpinLocker locker(&lock);

	if ((irq != ARCH_SPECIFIC_TIMER_INTERRUPT) && (irq != TZ_SGI_IRQ)
		&& (irq != TZ_SECURE_SGI_IRQ)) {
		err_msg("TZ: Attempt to enable a normal world interrupt disallowed\n");
		return;
	}

	gicV2InterruptEnable(irq);

}

void GIC::sgiGenerate(uint8_t cpuTargetList, int irq) {
	gicV2sgiGenerate(cpuTargetList, irq);
}

int GIC::currIntr(int *srcCpu) {
	return gicV2currIntr((uint32_t *)srcCpu);
}

void GIC::endIntr(int irq) {
	gicV2InterruptProcessed(irq, 0);
}

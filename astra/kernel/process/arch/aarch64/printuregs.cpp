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
#include "tztask.h"

void TzTask::printURegs() {
	register unsigned long dfar;
	ARCH_SPECIFIC_GET_DFAR(dfar);

	unsigned long *regBase = savedRegBase - NUM_SAVED_CPU_REGS;
	printf("------------- Task %d: CPU Registers -----------------\n", tid);
	printf("elr_el1 : 0x%16lx\tspsr_el1 : 0x%16lx\tx30 : 0x%16lx\tx31 : 0x%16lx\n", regBase[0], regBase[1], regBase[2], regBase[3]);
	printf("x28 : 0x%16lx\tx29 : 0x%16lx\tx26 : 0x%16lx\tx27 : 0x%16lx\n", regBase[4], regBase[5], regBase[6], regBase[7]);
	printf("x24 : 0x%16lx\tx25 : 0x%16lx\tx22: 0x%16lx\tx23: 0x%16lx\n", regBase[8], regBase[9], regBase[10], regBase[11]);
	printf("x20 : 0x%16lx\tx21 : 0x%16lx\tx18: 0x%16lx\tx19: 0x%16lx\n", regBase[12], regBase[13], regBase[14], regBase[15]);
	printf("x16 : 0x%16lx\tx17 : 0x%16lx\tx14: 0x%16lx\tx15: 0x%16lx\n", regBase[16], regBase[17], regBase[18], regBase[19]);
	printf("x12 : 0x%16lx\tx13 : 0x%16lx\tx10: 0x%16lx\tx11: 0x%16lx\n", regBase[20], regBase[21], regBase[22], regBase[23]);
	printf("x8 : 0x%16lx\tx9 : 0x%16lx\tx6: 0x%16lx\tx7: 0x%16lx\n", regBase[24], regBase[25], regBase[26], regBase[27]);
	printf("x4 : 0x%16lx\tx5 : 0x%16lx\tx2: 0x%16lx\tx3: 0x%16lx\n", regBase[28], regBase[29], regBase[30], regBase[31]);
	printf("x0 : 0x%16lx\tx1 : 0x%16lx\tsp_el1 : 0x%16lx\tsp : 0x%16lx\n", regBase[32], regBase[33], regBase[34], regBase[35]);
	printf("\tFault Location : 0x%16lx\n", dfar);
	printf("-----------------------------------------------------\n");
}

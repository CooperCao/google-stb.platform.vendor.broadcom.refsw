/*
 * Copyright (c) 2014-2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __BCM97268A0_DEF_H__
#define __BCM97268A0_DEF_H__

#if RESET_TO_BL31
#error "BCM97268A0 is incompatible with RESET_TO_BL31!"
#endif

#define BCM97268A0_PRIMARY_CPU	0x0

/* Register base address */
#define IO_PHYS				(0xffae0000)
#define BCM_GIC_BASE		(IO_PHYS + 0x220000)
#define BCM_GIC_SIZE		0x170000

/* Device address spaces */
#define BCM_DEV_RNG0_BASE	0xf0200000
#define BCM_DEV_RNG0_SIZE	0x9000
#define BCM_DEV_RNG1_BASE	0xf0400000
#define BCM_DEV_RNG1_SIZE	0x60000
#define BCM_DEV_RNG2_BASE	BCM_GIC_BASE
#define BCM_DEV_RNG2_SIZE	BCM_GIC_SIZE

#define BCM_HIF_BASE		BCM_DEV_RNG0_BASE
#define BCM_HIF_CPU_CTRL_BASE	(BCM_HIF_BASE + 0x5000)
#define BCM_CPU_RESET_CONFIG	(BCM_HIF_CPU_CTRL_BASE + 0x8c)
#define BCM_CPU0_PWR_ZONE_CTL	(BCM_HIF_CPU_CTRL_BASE + 0xc4)
#define BCM_CPU1_PWR_ZONE_CTL	(BCM_HIF_CPU_CTRL_BASE + 0xc8)

#define BCM_CPU_PWR_ZONE_CTL(x) (BCM_CPU0_PWR_ZONE_CTL + \
	((x) * (BCM_CPU1_PWR_ZONE_CTL - BCM_CPU0_PWR_ZONE_CTL)))

#define ZONE_MEM_PWR_STATE	(1 << 29)
#define ZONE_DPG_PWR_STATE	(1 << 28)
#define ZONE_MANUAL_CONTROL	(1 << 7)
#define ZONE_MAN_ISO_CNTL	(1 << 6)
#define RESERVED1		(1 << 5)
#define ZONE_MAN_MEM_PWR	(1 << 4)
#define ZONE_MAN_RESET_CNTL	(1 << 1)
#define ZONE_MAN_CLKEN		(1 << 0)

#define BCM_SYS_CTRL_BASE	BCM_DEV_RNG1_BASE
#define BCM_SUN_TOP_CTRL_BASE	(BCM_SYS_CTRL_BASE + 0x4000)
#define BCM_RESET_SOURCE_ENABLE	(BCM_SUN_TOP_CTRL_BASE + 0x304)
#define BCM_SW_MASTER_RESET	(BCM_SUN_TOP_CTRL_BASE + 0x308)
#define SW_MASTER_RESET_ENABLE_BIT	(1 << 0)
#define CHIP_MASTER_RESET_BIT		(1 << 0)

#define BCM_BOOT_CONT_BASE	(BCM_DEV_RNG1_BASE + 0x52000)
#define BCM_HIF_BOOT_START	BCM_BOOT_CONT_BASE

/* SRAMROM related registers */
#define SRAMROM_SEC_CTRL	(SRAMROM_SEC_BASE + 0x4)
#define SRAMROM_SEC_ADDR	(SRAMROM_SEC_BASE + 0x8)

/* DEVAPC0 related registers */
#define DEVAPC0_MAS_SEC_0	(DEVAPC0_BASE + 0x500)
#define DEVAPC0_APC_CON		(DEVAPC0_BASE + 0xF00)

/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define BCM97268A0_UART0_BASE	(0xf040c000)
#define BCM97268A0_UART1_BASE	(0xf040d000)
#define BCM97268A0_UART2_BASE	(0xf040e000)

#define BCM97268A0_BAUDRATE		(115200)
#define BCM97268A0_UART_CLOCK	(0x4d3f640)

/*******************************************************************************
 * System counter frequency related constants
 ******************************************************************************/
#define SYS_COUNTER_FREQ_IN_TICKS	27000000

/*******************************************************************************
 * GIC-400 & interrupt handling related constants
 ******************************************************************************/

/* Base BCM_platform compatible GIC memory map */
#define BASE_GICD_BASE		(BCM_GIC_BASE + 0x1000)
#define BASE_GICC_BASE		(BCM_GIC_BASE + 0x2000)
#define BASE_GICR_BASE		0	/* no GICR in GIC-400 */
#define BASE_GICH_BASE		(BCM_GIC_BASE + 0x4000)
#define BASE_GICV_BASE		(BCM_GIC_BASE + 0x6000)
#define INT_POL_CTL0		0x10200620

#define GIC_PRIVATE_SIGNALS	(32)

/* FIQ platform related define */
#define BCM_IRQ_SEC_SGI_0	8
#define BCM_IRQ_SEC_SGI_1	9
#define BCM_IRQ_SEC_SGI_2	10
#define BCM_IRQ_SEC_SGI_3	11
#define BCM_IRQ_SEC_SGI_4	12
#define BCM_IRQ_SEC_SGI_5	13
#define BCM_IRQ_SEC_SGI_6	14
#define BCM_IRQ_SEC_SGI_7	15

#endif /* __BCM97268A0_DEF_H__ */

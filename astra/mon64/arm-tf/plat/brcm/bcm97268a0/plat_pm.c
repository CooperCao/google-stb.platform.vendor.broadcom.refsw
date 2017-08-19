/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
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

#include <arch_helpers.h>
#include <gicv2.h>
#include <assert.h>
#include <bakery_lock.h>
#include <console.h>
#include <debug.h>
#include <delay_timer.h>
#include <errno.h>
#include <mmio.h>
#include <bcm97268a0_def.h>
#include <plat_private.h>
#include <psci.h>

#define TIMEOUT_US	500000
#define WAIT_US		10

extern void plat_smp_enable(void);

typedef enum
{
	CPU_OFF = 0,	/* CPU off */
	CPU_OFF_PEND,	/* CPU marked to be turned off */
	CPU_ON_PEND,	/* CPU powered and out of reset */
	CPU_IDLE,	/* CPU in standby mode */
	CPU_ON,		/* CPU on */
} cpu_state_t;

static cpu_state_t cpu_state[PLATFORM_CORE_COUNT];

static inline int wait_bit_set(uintptr_t addr, uint32_t bits)
{
	int retry = TIMEOUT_US / WAIT_US;
	while (retry--) {
		if ((mmio_read_32(addr) & bits))
			return 0;
		udelay(WAIT_US);
	}
	return -1;
}

static inline int wait_bit_clr(uintptr_t addr, uint32_t bits)
{
	int retry = TIMEOUT_US / WAIT_US;
	while (retry--) {
		if (!(mmio_read_32(addr) & bits))
			return 0;
		udelay(WAIT_US);
	}
	return -1;
}

/* Power on the CPU represented by cpuid */
static int plat_poweron_cpu(unsigned long cpu_id,
			    unsigned long sec_entrypoint)
{
	uint32_t base, lo, hi;

	mmio_setbits_32(BCM_CPU_PWR_ZONE_CTL(cpu_id),
		(ZONE_MAN_ISO_CNTL | ZONE_MANUAL_CONTROL |
		RESERVED1 | ZONE_MAN_MEM_PWR));
	if (wait_bit_set(BCM_CPU_PWR_ZONE_CTL(cpu_id),
		ZONE_MEM_PWR_STATE))
		return -1;

	mmio_setbits_32(BCM_CPU_PWR_ZONE_CTL(cpu_id),
		ZONE_MAN_CLKEN);
	if (wait_bit_set(BCM_CPU_PWR_ZONE_CTL(cpu_id),
		ZONE_DPG_PWR_STATE))
		return -1;

	mmio_clrbits_32(BCM_CPU_PWR_ZONE_CTL(cpu_id),
		ZONE_MAN_ISO_CNTL);
	mmio_setbits_32(BCM_CPU_PWR_ZONE_CTL(cpu_id),
		ZONE_MAN_RESET_CNTL);

	/* Program CPU entry point */
	base = BCM_HIF_BOOT_START + (cpu_id << 3);
	/* Low 32 bits address + 8 bits high address */
	lo = (uint32_t)(sec_entrypoint & 0x00000000ffffffffULL);
	hi = (uint32_t)((sec_entrypoint >> 32) & 0x00000000ffffffffULL);
	mmio_write_32(base, hi);
	mmio_write_32(base + sizeof(uint32_t), lo);

	dsb();

	/* Take CPU out of reset */
	mmio_clrbits_32(BCM_CPU_RESET_CONFIG, (1 << cpu_id));

	return 0;
}

/* Power off the CPU represented by cpu_id */
static int plat_poweroff_cpu(unsigned long cpu_id)
{
	mmio_setbits_32(BCM_CPU_PWR_ZONE_CTL(cpu_id),
			ZONE_MAN_ISO_CNTL);

	mmio_setbits_32(BCM_CPU_PWR_ZONE_CTL(cpu_id),
			ZONE_MANUAL_CONTROL);
	mmio_clrbits_32(BCM_CPU_PWR_ZONE_CTL(cpu_id),
			(ZONE_MAN_RESET_CNTL | ZONE_MAN_CLKEN |
			 ZONE_MAN_MEM_PWR));
	if (wait_bit_clr(BCM_CPU_PWR_ZONE_CTL(cpu_id),
			 ZONE_MEM_PWR_STATE))
		return -1;

	mmio_clrbits_32(BCM_CPU_PWR_ZONE_CTL(cpu_id),
			RESERVED1);
	if (wait_bit_clr(BCM_CPU_PWR_ZONE_CTL(cpu_id),
			 ZONE_DPG_PWR_STATE));
		return -1;

	if (mmio_read_32(BCM_CPU_PWR_ZONE_CTL(cpu_id) == 0))
		return -1;

	dsb();

	/* Put CPU in reset */
	mmio_setbits_32(BCM_CPU_RESET_CONFIG, (1 << cpu_id));

	return 0;
}

/*******************************************************************************
 * BCM_platform handler called when an affinity instance is about to be turned
 * on. The level and mpidr determine the affinity instance.
 ******************************************************************************/
static int plat_affinst_on(unsigned long mpidr,
		    unsigned long sec_entrypoint,
		    unsigned int afflvl,
		    unsigned int state)
{
	unsigned long cpu_id;

	/*
	 * It's possible to turn on only affinity level 0 i.e. a cpu
	 * on the BCM_platform. Ignore any other affinity level.
	 */
	if (afflvl != MPIDR_AFFLVL0)
		return PSCI_E_SUCCESS;

	cpu_id = mpidr & MPIDR_CPU_MASK;

	if (cpu_id >= PLATFORM_CORE_COUNT)
		return PSCI_E_INVALID_PARAMS;

	if (cpu_state[cpu_id] == CPU_ON_PEND ||
	    cpu_state[cpu_id] == CPU_ON)
		return PSCI_E_ALREADY_ON;

	/* Power on CPU */
	if (plat_poweron_cpu(cpu_id, sec_entrypoint))
		return PSCI_E_INTERN_FAIL;
	cpu_state[cpu_id] = CPU_ON_PEND;

	flush_dcache_range((uint64_t)&cpu_state, sizeof(cpu_state));

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * BCM_platform handler called when an affinity instance has just been powered
 * on after being turned off earlier. The level and mpidr determine the affinity
 * instance. The 'state' arg. allows the platform to decide whether the cluster
 * was turned off prior to wakeup and do what's necessary to setup it up
 * correctly.
 ******************************************************************************/
static void plat_affinst_on_finish(unsigned int afflvl, unsigned int state)
{
	unsigned long cpu_id;

	/*
	 * It's possible to turn off only affinity level 0 i.e. a cpu
	 * on the BCM_platform. Ignore any other affinity level.
	 */
	if (afflvl != MPIDR_AFFLVL0)
		return;

	cpu_id = (read_mpidr_el1() & MPIDR_CPU_MASK);
	assert(cpu_state[cpu_id] == CPU_ON_PEND);

	plat_smp_enable();

	/* Enable the gic cpu interface */
	gicv2_cpuif_enable();
	gicv2_pcpu_distif_init();

	cpu_state[cpu_id] = CPU_ON;

	flush_dcache_range((uint64_t)&cpu_state, sizeof(cpu_state));
}

/*******************************************************************************
 * BCM_platform handler called when an affinity instance is about to be turned
 * off. The level and mpidr determine the affinity instance. The 'state' arg.
 * allows the platform to decide whether the cluster is being turned off and
 * take apt actions.
 ******************************************************************************/
static void plat_affinst_off(unsigned int afflvl, unsigned int state)
{
	unsigned long cpu_id;

	/*
	 * It's possible to turn off only affinity level 0 i.e. a cpu
	 * on the BCM_platform. Ignore any other affinity level.
	 */
	if (afflvl != MPIDR_AFFLVL0)
		return;

	cpu_id = (read_mpidr_el1() & MPIDR_CPU_MASK);
	assert(cpu_state[cpu_id] == CPU_ON);

	/* Prevent interrupts from spuriously waking up this cpu */
	gicv2_cpuif_disable();

	/*
	 * Not much to do here apart from marking this CPU as CPU_OFF_PEND.
	 * We cannot power down the CPU here as cpu state table for affinity
	 * info is updated from the same context after this call. The PSCI
	 * framework function makes platform specific cleanup calls (e.g cache
	 * flush/invalidate, SMP disable etc) and finally puts this CPU in WFI
	 * state. The expectation is for an external entity (power controller
	 * H/W or system power control core) to power down the CPU after it
	 * enters WFI state. Unfortunately, the power controllers in the STB
	 * SoCs does not support 'auto' mode, therefore it needs to be manually
	 * programmed by another ARM core. This is done by the affinst_off_finish
	 * callback.
	 */
	cpu_state[cpu_id] = CPU_OFF_PEND;

	flush_dcache_range((uint64_t)&cpu_state, sizeof(cpu_state));
}

/*******************************************************************************
 * BCM_platform handler called when an affinity instance needs to be manually
 * turned off for a particular core. The level and mpidr determine the affinity
 * instance.
 ******************************************************************************/
static void plat_affinst_off_finish(unsigned long mpidr, unsigned int afflvl)
{
	unsigned long cpu_id;

	/*
	 * It's possible to turn on only affinity level 0 i.e. a cpu
	 * on the BCM_platform. Ignore any other affinity level.
	 */
	if (afflvl != MPIDR_AFFLVL0)
		return;

	cpu_id = mpidr & MPIDR_CPU_MASK;

	assert(cpu_id < PLATFORM_CORE_COUNT);

	/* Make sure cpu is ready to be turned off */
	if (cpu_state[cpu_id] != CPU_OFF_PEND)
		return;

	/* Power down the cpu */
	if (plat_poweroff_cpu(cpu_id))
		assert(0);

	cpu_state[cpu_id] = CPU_OFF;

	flush_dcache_range((uint64_t)&cpu_state, sizeof(cpu_state));
}

/*******************************************************************************
 * BCM_platform handler called when an affinity instance is about to enter
 * standby.
 ******************************************************************************/
static void plat_affinst_standby(unsigned int power_state)
{
	unsigned int target_afflvl;
	unsigned long cpu_id;

	/* Sanity check the requested state */
	target_afflvl = psci_get_pstate_afflvl(power_state);

	/*
	 * It's possible to enter standby only on affinity level 0 i.e. a cpu
	 * on the BCM_platform. Ignore any other affinity level.
	 */
	if (target_afflvl == MPIDR_AFFLVL0) {
		/*
		 * Enter standby state. dsb is good practice before using wfi
		 * to enter low power states.
		 */
		cpu_id = (read_mpidr_el1() & MPIDR_CPU_MASK);
		cpu_state[cpu_id] = CPU_IDLE;

		dsb();
		wfi();

		cpu_state[cpu_id] = CPU_ON;
	}

	flush_dcache_range((uint64_t)&cpu_state, sizeof(cpu_state));
}

/*******************************************************************************
 * BCM handlers to shutdown/reboot the system
 ******************************************************************************/
static void __dead2 plat_system_off(void)
{
	int i;
	unsigned long cpu_id = (read_mpidr_el1() & MPIDR_CPU_MASK);

	INFO("BRCM System Off\n");

	for (i = 0; i < PLATFORM_CORE_COUNT; i++) {
		if (i == cpu_id)
			continue;

		/* Power down CPU */
		if (cpu_state[i] != CPU_OFF) {
			if (plat_poweroff_cpu(i))
				assert(0);
			cpu_state[i] = CPU_OFF;
		}
	}

	dsb();

	/* Finally put current CPU in reset */
	mmio_setbits_32(BCM_CPU_RESET_CONFIG, (1 << cpu_id));

	wfi();
	ERROR("BRCM System Off: Operation failed\n");
	panic();
}

static void __dead2 plat_system_reset(void)
{
	INFO("BRCM System Reset\n");

	mmio_setbits_32(BCM_RESET_SOURCE_ENABLE,
		SW_MASTER_RESET_ENABLE_BIT);
	mmio_setbits_32(BCM_SW_MASTER_RESET,
		CHIP_MASTER_RESET_BIT);

	wfi();
	ERROR("BRCM System Reset: Operation failed\n");
	panic();
}

/*******************************************************************************
 * Export the platform handlers to enable psci to invoke them
 ******************************************************************************/
static const plat_pm_ops_t plat_plat_pm_ops = {
	.affinst_on			= plat_affinst_on,
	.affinst_on_finish		= plat_affinst_on_finish,
	.affinst_off			= plat_affinst_off,
	.affinst_off_finish		= plat_affinst_off_finish,
	.affinst_standby		= plat_affinst_standby,
	.system_off			= plat_system_off,
	.system_reset			= plat_system_reset,
};

/*******************************************************************************
 * Export the platform specific power ops & initialize the bcm_platform power
 * controller
 ******************************************************************************/
int platform_setup_pm(const plat_pm_ops_t **plat_ops)
{
	int i;
	*plat_ops = &plat_plat_pm_ops;

	/* Initialize CPU states */
	cpu_state[0] = CPU_ON;
	for (i = 1; i < PLATFORM_CORE_COUNT; i++)
		cpu_state[i] = CPU_OFF;

	return 0;
}

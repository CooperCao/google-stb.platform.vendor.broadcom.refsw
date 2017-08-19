#
# Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# Neither the name of ARM nor the names of its contributors may be used
# to endorse or promote products derived from this software without specific
# prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

BCM_PLAT		:=	plat/brcm
BCM_PLAT_SOC		:=	${BCM_PLAT}/${PLAT}

PLAT_INCLUDES		:=	-I${BCM_PLAT}/common/				\
				-I${BCM_PLAT_SOC}/drivers/uart/			\
				-I${BCM_PLAT_SOC}/include/

PLAT_BL_COMMON_SOURCES	:=	lib/xlat_tables/xlat_tables_common.c		\
				lib/xlat_tables/aarch64/xlat_tables.c		\
				plat/common/aarch64/plat_common.c		\
				plat/common/plat_gicv2.c

BL31_SOURCES		+=	\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/arm/gic/common/gic_common.c	\
				drivers/console/console.S			\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				lib/cpus/aarch64/aem_generic.S			\
				lib/cpus/aarch64/brcm_b53.S			\
				plat/common/aarch64/platform_mp_stack.S		\
				${BCM_PLAT}/common/bcm_plat_common.c		\
				${BCM_PLAT_SOC}/aarch64/plat_helpers.S		\
				${BCM_PLAT_SOC}/aarch64/platform_common.c	\
				${BCM_PLAT_SOC}/bl31_plat_setup.c		\
				${BCM_PLAT_SOC}/drivers/uart/16550_console.S	\
				${BCM_PLAT_SOC}/plat_bcm_gic.c			\
				${BCM_PLAT_SOC}/plat_pm.c			\
				${BCM_PLAT_SOC}/plat_topology.c

# Flag used by the BCM_platform port to determine the version of ARM GIC
# architecture to use for interrupt management in EL3.
ARM_GIC_ARCH		:=	2
$(eval $(call add_define,ARM_GIC_ARCH))

# Enable workarounds for selected Cortex-A53 erratas.
ERRATA_A53_826319	:=	1
ERRATA_A53_836870	:=	1

# indicate the reset vector address can be programmed
PROGRAMMABLE_RESET_ADDRESS	:=	1

# So we can load as a code segment
SEPARATE_CODE_AND_RODATA := 1

$(eval $(call add_define,BCM_SIP_SET_AUTHORIZED_SECURE_REG_ENABLE))

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
#ifndef B_SHARED_GPIO_H
#define B_SHARED_GPIO_H 1

#include "b_shared_gpio_types.h"

#if defined(CONFIG_BRCMSTB_NEXUS_API)
#define B_SHARED_GPIO_OS_SUPPORT BRCMSTB_H_VERSION
#else
#define B_SHARED_GPIO_OS_SUPPORT 0
#endif

typedef struct b_shared_gpio_module_init_settings {
    int gio_linux_irq;
    int gio_aon_linux_irq;
} b_shared_gpio_module_init_settings;

typedef struct b_shared_gpio_capabilities {
    bool feature_supported;
} b_shared_gpio_capabilities;

void b_shared_gpio_init_submodule(const b_shared_gpio_module_init_settings * settings);
void b_shared_gpio_uninit_submodule(void);

int b_shared_gpio_open_submodule(void);
void b_shared_gpio_close_submodule(void);

void b_shared_gpio_get_capabilities(b_shared_gpio_capabilities * caps);

void b_shared_gpio_init_banks(const b_shared_gpio_init_banks_settings * settings);
int b_shared_gpio_open_pin(const b_shared_gpio_pin_desc * pin_desc, b_shared_gpio_irq_type irq_type);
void b_shared_gpio_close_pin(const b_shared_gpio_pin_desc * pin_desc);
void b_shared_gpio_get_int_status(b_shared_gpio_int_status * status);
int b_shared_gpio_clear_int(const b_shared_gpio_pin_desc * pin_desc);
int b_shared_gpio_set_int_mask(const b_shared_gpio_pin_desc * pin_desc, bool disabled);
int b_shared_gpio_set_standby(const b_shared_gpio_pin_desc * pin_desc, bool enabled);
/* this function expects the caller to acquire the irq spinlock */
void b_shared_gpio_reenable_irqs_spinlocked(void);

#endif /* B_SHARED_GPIO_H */

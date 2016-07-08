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
#ifndef B_SHARED_GPIO_TYPES_H
#define B_SHARED_GPIO_TYPES_H 1

#define B_SHARED_GPIO_MAX_BANKS 8

typedef struct b_shared_gpio_pin_desc {
    uint32_t bank_base_address;
    unsigned shift; /* which bit within the bank is this? */
} b_shared_gpio_pin_desc;

typedef enum b_shared_gpio_irq_type {
    b_shared_gpio_irq_type_disabled,      /* No interrupt */
    b_shared_gpio_irq_type_rising_edge,    /* Interrupt on a 0->1 transition */
    b_shared_gpio_irq_type_falling_edge,   /* Interrupt on a 1->0 transition */
    b_shared_gpio_irq_type_edge,          /* Interrupt on both a 0->1 and a 1->0 transition */
    b_shared_gpio_irq_type_low,           /* Interrupt on a 0 value */
    b_shared_gpio_irq_type_high,          /* Interrupt on a 1 value */
    b_shared_gpio_irq_type_max
} b_shared_gpio_irq_type;

typedef struct b_shared_gpio_init_banks_settings {
    uint32_t bank_base_addresses[B_SHARED_GPIO_MAX_BANKS];
    int bank_is_aon[B_SHARED_GPIO_MAX_BANKS]; /* bool */
} b_shared_gpio_init_banks_settings;

typedef struct b_shared_gpio_int_status {
    uint32_t bank_status[B_SHARED_GPIO_MAX_BANKS];
} b_shared_gpio_int_status;

#endif /* B_SHARED_GPIO_TYPES_H */

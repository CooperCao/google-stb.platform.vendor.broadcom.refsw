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
#ifndef B_VIRTUAL_IRQ_TYPES_H
#define B_VIRTUAL_IRQ_TYPES_H 1

typedef enum b_virtual_irq_line {
    b_virtual_irq_line_unused,
    b_virtual_irq_line_iica,
    b_virtual_irq_line_iicb,
    b_virtual_irq_line_iicc,
    b_virtual_irq_line_iicd,
    b_virtual_irq_line_iice,
    b_virtual_irq_line_iicf,
    b_virtual_irq_line_iicg,
    b_virtual_irq_line_gio,
    b_virtual_irq_line_gio_aon,
    b_virtual_irq_line_irb,
    b_virtual_irq_line_icap,
    b_virtual_irq_line_kbd1,
    b_virtual_irq_line_kbd2,
    b_virtual_irq_line_kbd3,
    b_virtual_irq_line_ldk,
    b_virtual_irq_line_spi,
    b_virtual_irq_line_ua,
    b_virtual_irq_line_ub,
    b_virtual_irq_line_uc,
    b_virtual_irq_line_max
} b_virtual_irq_line;

typedef struct b_virtual_irq_status {
    b_virtual_irq_line lines[32]; /* in */
    uint32_t status_word; /* out */
} b_virtual_irq_status;

typedef struct b_virtual_irq_group {
    unsigned l1_shift; /* virtual L1 interrupt */
    b_virtual_irq_line lines[32]; /* Linux L1 interrupts mapped to this virtual L1 interrupt */
    b_virtual_irq_line indirect[32]; /* Linux L1 interrupts mapped to this virtual L1 interrupt, but not by direct request_irq */
} b_virtual_irq_group;

#endif /* B_VIRTUAL_IRQ_TYPES_H */

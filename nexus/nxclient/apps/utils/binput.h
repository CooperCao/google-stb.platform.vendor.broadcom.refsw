/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef BINPUT_H__
#define BINPUT_H__

#include "nexus_types.h"
#if NEXUS_HAS_IR_INPUT
#include "nexus_ir_input.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
NOTE: This API is only example code. It is subject to change and
is not supported as a standard reference software deliverable.
**/

/**
IR input mapping
**/
typedef enum b_remote_key {
    b_remote_key_unknown,
    b_remote_key_play,
    b_remote_key_pause,
    b_remote_key_fast_forward,
    b_remote_key_rewind,
    b_remote_key_stop,
    b_remote_key_clear, /* or exit */
    b_remote_key_back, /* or last */
    b_remote_key_up,
    b_remote_key_down,
    b_remote_key_right,
    b_remote_key_left,
    b_remote_key_select,
    b_remote_key_power,
    b_remote_key_chan_up,
    b_remote_key_chan_down,
    b_remote_key_one,
    b_remote_key_two,
    b_remote_key_three,
    b_remote_key_four,
    b_remote_key_five,
    b_remote_key_six,
    b_remote_key_seven,
    b_remote_key_eight,
    b_remote_key_nine,
    b_remote_key_zero,
    b_remote_key_dot, /* or dash */
    b_remote_key_info,
    b_remote_key_guide,
    b_remote_key_menu,
    b_remote_key_max
} b_remote_key;

#if NEXUS_HAS_IR_INPUT
b_remote_key b_get_remote_key(NEXUS_IrInputMode irInput, unsigned code);
#endif

typedef struct binput *binput_t;

struct binput_settings
{
    NEXUS_CallbackDesc codeAvailable;
    const char *script;
    const char *script_file;
#if NEXUS_HAS_IR_INPUT && !NXCLIENT_SUPPORT
    NEXUS_IrInputMode irInputMode;
#endif
};
void binput_get_default_settings(struct binput_settings *psettings);
binput_t binput_open(const struct binput_settings *psettings);
void binput_close(binput_t input);

int binput_read(binput_t input, b_remote_key *key, bool *repeat);
int binput_read_no_repeat(binput_t input, b_remote_key *key);
int binput_wait(binput_t input, unsigned timeout_msec);
void binput_interrupt(binput_t input);
int binput_set_mask(binput_t input, uint32_t mask);
void binput_print_script_usage(void);
#if NEXUS_HAS_IR_INPUT && !NXCLIENT_SUPPORT
NEXUS_IrInputHandle binput_irhandle(binput_t input);
#endif

#ifdef __cplusplus
}
#endif

#endif /* BINPUT_H__ */

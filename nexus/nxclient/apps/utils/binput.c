/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/
#include "bstd.h"
#include "binput.h"
#include "bkni_multi.h"
#if !NEXUS_HAS_INPUT_ROUTER
/* for this file, no input router is the same as no nxclient */
#undef NXCLIENT_SUPPORT
#endif
#if NXCLIENT_SUPPORT
#include "nxclient.h"
#include "nexus_input_client.h"
#else
#include "nexus_ir_input.h"
#endif
#include <string.h>
#include <stdio.h>

BDBG_MODULE(binput);

enum binput_script
{
    binput_script_repeat = b_remote_key_max,
    binput_script_sleep,
    binput_script_max
};

struct keymap {
    unsigned ir_input_a;
    unsigned ir_cir_nec;
    unsigned ir_input_gisat;
    unsigned ir_input_rstep;
    unsigned keyboard;
    const char *script;
} g_input_keymap[binput_script_max] = {
                       /* ir_a, cir_nec,    gisat, rstep,   keyboard */
    /* unknown */      {0,      0,          0,     0,       0,                      ""},
    /* play */         {0x5038, 0xe21dff00, 0xb2e, 0x11ac9, 25  /* P */,            "play"},
    /* pause */        {0x001f, 0xe31cff00, 0xd2c, 0,       119 /* PAUSE/BREAK */,  "pause"},
    /* fast_forward */ {0x201d, 0xa659ff00, 0x128, 0x11ab9, 33  /* F */,            "ff"},
    /* rewind */       {0x101e, 0xe619ff00, 0x229, 0x11aba, 19  /* R */,            "rew"},
    /* stop */         {0x4039, 0xa35cff00, 0xe2d, 0x11aca, 57  /* SPACE */,        "stop"},
    /* clear */        {0xd012, 0xb24dff00, 0xf30, 0,       111 /* DELETE */,       "clear"},
    /* back */         {0x303a, 0xf906ff00, 0xf16, 0x11a82, 14  /* BACKSPACE */,    "back"},
    /* up */           {0x9034, 0xb14eff00, 0xf23, 0x11ae6, 103,                    "u"},
    /* down */         {0x8035, 0xf30cff00, 0xa20, 0x11ae5, 108,                    "d"},
    /* right */        {0x6037, 0xb649ff00, 0x301, 0x11ade, 106,                    "r"},
    /* left */         {0x7036, 0xf40bff00, 0x11d, 0x11ae2, 105,                    "l"},
    /* select */       {0xe011, 0xf708ff00, 0x21c, 0x11a84, 28  /* RETURN */,       "select"},
    /* power */        {0x600A, 0xf50aff00, 0xa06, 0x11ae9, 142 /* SLEEP */,        "power"},
    /* chan_up */      {0,      0xf609ff00, 0xe0b, 0x11ad6, 0,                      "chan_up"},
    /* chan_down */    {0,      0xf20dff00, 0x510, 0x11ad2, 0,                      "chan_down"},
    /* one */          {0,      0xe01fff00, 0,     0,       2,                      "1"},
    /* two */          {0,      0xa15eff00, 0,     0,       3,                      "2"},
    /* three */        {0,      0xa05fff00, 0,     0,       4,                      "3"},
    /* four */         {0,      0xe41bff00, 0,     0,       5,                      "4"},
    /* five */         {0,      0xa55aff00, 0,     0,       6,                      "5"},
    /* six */          {0,      0xa45bff00, 0,     0,       7,                      "6"},
    /* seven */        {0,      0xe817ff00, 0,     0,       8,                      "7"},
    /* eight */        {0,      0xa956ff00, 0,     0,       9,                      "8"},
    /* nine */         {0,      0xa857ff00, 0,     0,       10,                     "9"},
    /* zero */         {0,      0xad52ff00, 0,     0,       11,                     "0"},
    /* dot */          {0,      0xac53ff00, 0,     0,       52, /* DOT */           "."},
    /* info */         {0,      0xf00fff00, 0,     0,       23, /* I */             "info"},
    /* guide */        {0,      0xf10eff00, 0,     0,       34, /* G */             "guide"},
    /* menu */         {0,      0xb04fff00, 0,     0,       50, /* M */             "menu"},
    /* next */         {0x203b, 0xe718ff00, 0,     0,       0,                      "next"},
    /* prev */         {0,      0xa758ff00, 0,     0,       0,                      "prev"},

    /* script commands */
                       {0, 0, 0, 0, 0,                                              "repeat"},
                       {0, 0, 0, 0, 0,                                              "sleep"} /* TODO: hardcoded 1 second */
};

#if NEXUS_HAS_IR_INPUT
b_remote_key b_get_remote_key(NEXUS_IrInputMode irInput, unsigned code)
{
    b_remote_key key;

    for (key=1;key<b_remote_key_max;key++) {
        switch (irInput) {
        case NEXUS_IrInputMode_eCirNec:
            if (g_input_keymap[key].ir_cir_nec == code) return key;
            break;
        case NEXUS_IrInputMode_eRemoteA:
            if (g_input_keymap[key].ir_input_a == code) return key;
            break;
        case NEXUS_IrInputMode_eCirGISat:
            if (g_input_keymap[key].ir_input_gisat == code) return key;
            break;
        case NEXUS_IrInputMode_eCirRstep:
            if (g_input_keymap[key].ir_input_rstep == code) return key;
            break;
        default:
            BERR_TRACE(NEXUS_INVALID_PARAMETER);
            return b_remote_key_unknown;
        }
    }

    BDBG_MSG(("unknown key %#x", code));
    return b_remote_key_unknown;
}
#endif

b_remote_key b_get_evdev_key(unsigned code)
{
    b_remote_key key;

    for (key=1;key<b_remote_key_max;key++) {
        if (g_input_keymap[key].keyboard == code) return key;
    }

    BERR_TRACE(NEXUS_INVALID_PARAMETER);
    return b_remote_key_unknown;
}

struct binput
{
    struct binput_settings settings;
#if NXCLIENT_SUPPORT
    NxClient_AllocResults allocResults;
    NEXUS_InputClientHandle inputClient;
#else
    NEXUS_IrInputHandle irInput;
    NEXUS_IrInputMode irInputMode;
#endif
    BKNI_EventHandle event;
    struct {
#define MAX_SCRIPT 100
        unsigned key[MAX_SCRIPT];
        unsigned total, current;
        unsigned sleep_remaining;
    } script;
};

static unsigned binput_p_find_script(const char *s)
{
    unsigned key;
    for (key=1;key<binput_script_max;key++) {
        if (!strcmp(g_input_keymap[key].script, s)) return key;
    }
    return 0;
}

void binput_get_default_settings(struct binput_settings *psettings)
{
    BKNI_Memset(psettings, 0, sizeof(*psettings));
#if NEXUS_HAS_IR_INPUT && !NXCLIENT_SUPPORT
    psettings->irInputMode = NEXUS_IrInputMode_eCirNec;
#endif
    NEXUS_CallbackDesc_Init(&psettings->codeAvailable);
}

static void binput_p_callback(void *context, int param)
{
    binput_t input = context;
    BSTD_UNUSED(param);
    BKNI_SetEvent(input->event);
}

static void binput_p_add_script(binput_t input, const char *s)
{
    while (*s && input->script.total < MAX_SCRIPT) {
        const char *find = strchr(s, ';');
        char buf[64];
        unsigned len = find?find-s:(int)strlen(s);
        strncpy(buf, s, sizeof(buf));
        buf[len] = 0;
        input->script.key[input->script.total] = binput_p_find_script(buf);
        if (!input->script.key[input->script.total]) {
            BDBG_ERR(("invalid command: %s", buf));
            break;
        }
        input->script.total++;
        if (!find) break;
        s = ++find;
    }
}

binput_t binput_open(const struct binput_settings *psettings)
{
#if NXCLIENT_SUPPORT
    NxClient_AllocSettings allocSettings;
    NEXUS_InputClientSettings settings;
    int rc;
#else
    NEXUS_IrInputSettings irInputSettings;
#endif
    binput_t input;

    input = BKNI_Malloc(sizeof(*input));
    if (!input) return NULL;
    BKNI_Memset(input, 0, sizeof(*input));
    if (psettings) {
        input->settings = *psettings;
    }
    else {
        binput_get_default_settings(&input->settings);
    }

    BKNI_CreateEvent(&input->event);
#if NXCLIENT_SUPPORT
    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.inputClient = 1;
    rc = NxClient_Alloc(&allocSettings, &input->allocResults);
    if (rc) {BERR_TRACE(rc); goto error;}

    input->inputClient = NEXUS_InputClient_Acquire(input->allocResults.inputClient[0].id);
    if (!input->inputClient) goto error;

    NEXUS_InputClient_GetSettings(input->inputClient, &settings);
    settings.filterMask = 0xFFFFFFFF; /* everything */
    if (input->settings.codeAvailable.callback) {
        settings.codeAvailable = input->settings.codeAvailable;
    }
    else {
        settings.codeAvailable.callback = binput_p_callback;
        settings.codeAvailable.context = input;
    }
    NEXUS_InputClient_SetSettings(input->inputClient, &settings);
#else
    NEXUS_IrInput_GetDefaultSettings(&irInputSettings);
    irInputSettings.dataReady.callback = binput_p_callback;
    irInputSettings.dataReady.context = input;
    irInputSettings.mode = input->settings.irInputMode;
    input->irInput = NEXUS_IrInput_Open(0, &irInputSettings);
    if (!input->irInput) goto error;
#endif

    if (input->settings.script_file) {
        FILE *f = fopen(input->settings.script_file, "r");
        if (!f) {
            BDBG_ERR(("unable to read script file %s", input->settings.script_file));
        }
        else {
            while (!feof(f)) {
                char buf[256];
                unsigned len;
                fgets(buf, sizeof(buf), f);
                len = strlen(buf);
                if (len) buf[len-1] = 0; /* remove newline */
                binput_p_add_script(input, buf);
            }
            fclose(f);
        }
    }
    else if (input->settings.script) {
        binput_p_add_script(input, input->settings.script);
    }

    return input;

error:
    binput_close(input);
    return NULL;
}

void binput_close(binput_t input)
{
#if NXCLIENT_SUPPORT
    if (input->inputClient) {
        NEXUS_InputClient_Release(input->inputClient);
    }
#else
    if (input->irInput) {
        NEXUS_IrInput_Close(input->irInput);
    }
#endif
    if (input->event) {
        BKNI_DestroyEvent(input->event);
    }
#if NXCLIENT_SUPPORT
    NxClient_Free(&input->allocResults);
#endif
    BKNI_Free(input);
}

int binput_read(binput_t input, b_remote_key *key, bool *repeat)
{
#if NXCLIENT_SUPPORT
    NEXUS_InputRouterCode inputRouterCode;
    unsigned num;
#else
    NEXUS_IrInputEvent event;
    bool overflow;
    size_t num;
#endif
    int rc;

    if (input->script.key[input->script.current]) {
        if (input->script.key[input->script.current] == binput_script_repeat) {
            input->script.current = 0;
        }
        if (input->script.key[input->script.current] != binput_script_sleep) {
            *key = input->script.key[input->script.current++];
            *repeat = false;
            return 0;
        }
    }

#if NXCLIENT_SUPPORT
    rc = NEXUS_InputClient_GetCodes(input->inputClient, &inputRouterCode, 1, &num);
    if (rc || !num) {
        return -1;
    }
    *repeat = false;
    switch (inputRouterCode.deviceType) {
    case NEXUS_InputRouterDevice_eIrInput:
        *repeat = inputRouterCode.data.irInput.repeat;
        *key = b_get_remote_key(inputRouterCode.data.irInput.mode, inputRouterCode.data.irInput.code);
        return 0;
    case NEXUS_InputRouterDevice_eEvdev:
        /* evdev sends key up and key down status in the value, so ignore one of them to avoid double keypresses */
        if (inputRouterCode.data.evdev.type == 1 /* EV_KEY */ && inputRouterCode.data.evdev.value) {
            *key = b_get_evdev_key(inputRouterCode.data.evdev.code);
            return 0;
        }
        break;
    default:
        break;
    }
    *key = b_remote_key_unknown;
    return 0;
#else
    rc = NEXUS_IrInput_GetEvents(input->irInput, &event, 1, &num, &overflow);
    if (rc || !num) {
        return -1;
    }
    *repeat = event.repeat;
    *key = b_get_remote_key(input->settings.irInputMode, event.code);
    return 0;
#endif
}

int binput_read_no_repeat(binput_t input, b_remote_key *key)
{
    bool repeat;
    int rc;
    do {
        rc = binput_read(input, key, &repeat);
    } while (!rc && repeat);
    return rc;
}

int binput_wait(binput_t input, unsigned timeout_msec)
{
    if (input->script.key[input->script.current] == binput_script_sleep) {
        if (!input->script.sleep_remaining) {
            input->script.sleep_remaining = 1000; /* TODO */
        }
        if (timeout_msec > input->script.sleep_remaining) {
            timeout_msec = input->script.sleep_remaining;
        }
        BKNI_Sleep(timeout_msec);
        input->script.sleep_remaining -= timeout_msec;
        if (!input->script.sleep_remaining) {
            input->script.current++;
        }
        return BERR_TIMEOUT;
    }

    if (input->event) {
        return BKNI_WaitForEvent(input->event, timeout_msec);
    }
    else {
        return NEXUS_NOT_SUPPORTED;
    }
}

void binput_interrupt(binput_t input)
{
    if (input->event) {
        BKNI_SetEvent(input->event);
    }
}

int binput_set_mask(binput_t input, uint32_t mask)
{
#if NXCLIENT_SUPPORT
    NEXUS_InputClientSettings settings;
    NEXUS_InputClient_GetSettings(input->inputClient, &settings);
    settings.filterMask = mask;
    return NEXUS_InputClient_SetSettings(input->inputClient, &settings);
#else
    BSTD_UNUSED(input);
    BSTD_UNUSED(mask);
    return 0;
#endif
}

void binput_print_script_usage(void)
{
    unsigned i;
    BKNI_Printf("script commands\n");
    for (i=1;i<binput_script_max;i++) {
        BKNI_Printf("  %s\n", g_input_keymap[i].script);
    }
}

#if NEXUS_HAS_IR_INPUT && !NXCLIENT_SUPPORT
NEXUS_IrInputHandle binput_irhandle(binput_t input)
{
    return input->irInput;
}
#endif

/******************************************************************************
 *    (c)2011-2013 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *****************************************************************************/
#include "nxserverlib_impl.h"
#include "nxserverlib_evdev.h"

BDBG_MODULE(nxserverlib_input);

#if NEXUS_HAS_INPUT_ROUTER
static size_t get_ir_event( NEXUS_IrInputHandle irInput, unsigned mode, NEXUS_InputRouterCode *pCode )
{
    NEXUS_Error rc;
    size_t n = 0;
    if (irInput) {
        NEXUS_IrInputEvent irEvent;
        bool overflow;
        rc = NEXUS_IrInput_GetEvents(irInput, &irEvent, 1, &n, &overflow);
        if (n == 1) {
            BDBG_MSG(("irinput code=%#x", irEvent.code));
            NEXUS_InputRouter_GetDefaultCode(pCode);
            pCode->deviceType = NEXUS_InputRouterDevice_eIrInput;
            pCode->filterMask = 1<<pCode->deviceType;
            pCode->data.irInput.index = 0;
            pCode->data.irInput.code = irEvent.code;
            pCode->data.irInput.repeat = irEvent.repeat;
            pCode->data.irInput.mode = mode;
        } else {
            n = 0;
        }
    }
    return n;
}

#if NEXUS_HAS_KEYPAD
static size_t get_keypad_event( NEXUS_KeypadHandle keypad, NEXUS_InputRouterCode *pCode )
{
    size_t n = 0;
    NEXUS_Error rc;
    if (keypad) {
        NEXUS_KeypadEvent keypadEvent;
        bool overflow;
        rc = NEXUS_Keypad_GetEvents(keypad, &keypadEvent, 1, &n, &overflow);
        BDBG_ASSERT(!rc);
        if (n == 1) {
            BDBG_MSG(("keypad code=%#x", keypadEvent.code));
            NEXUS_InputRouter_GetDefaultCode(pCode);
            pCode->deviceType = NEXUS_InputRouterDevice_eKeypad;
            pCode->filterMask = 1<<pCode->deviceType;
            pCode->data.keypad.index = 0;
            pCode->data.keypad.code = keypadEvent.code;
            pCode->data.keypad.repeat = keypadEvent.repeat;
        }
        else {
            n = 0;
        }
    }
    return n;
}
#endif

static void nxserverlib_p_input_callback(void *context, int param)
{
    struct b_session *session = context;
    NEXUS_InputRouterCode code;

    while(1)
    {
        bool foundCode = false;

        switch (param)
        {
            case NEXUS_InputRouterDevice_eIrInput:
                if (get_ir_event(session->input.irInput, session->server->settings.session[session->index].ir_input_mode, &code)) foundCode = true;
                break;
#if NEXUS_HAS_KEYPAD
            case NEXUS_InputRouterDevice_eKeypad:
                if (get_keypad_event(session->input.keypad, &code)) foundCode = true;
                break;
#endif
            case NEXUS_InputRouterDevice_eEvdev:
                if (nxserverlib_get_evdev_input(session->input.evdevInput, &code)) foundCode = true;
                break;
            default:
                break;
        }

        if (foundCode) {
            NEXUS_InputRouter_SendCode(session->input.router, &code);
        } else break;
    }
}

int nxserverlib_send_input(nxclient_t client, unsigned inputClientId, const NEXUS_InputRouterCode *pCode)
{
    struct b_req *req;
    for (req = BLST_D_FIRST(&client->requests); req; req = BLST_D_NEXT(req, link)) {
        unsigned i;
        for (i=0;i<NXCLIENT_MAX_IDS;i++) {
            if (req->handles.inputClient[i].id == inputClientId) {
                NEXUS_InputRouter_SendClientCode(client->session->input.router, req->handles.inputClient[i].handle, pCode);
                return NEXUS_SUCCESS;
            }
        }
    }
    return BERR_TRACE(NEXUS_INVALID_PARAMETER);
}

int init_input_devices(struct b_session *session)
{
    nxserver_t server = session->server;

    session->input.router = NEXUS_InputRouter_Create(session->index);
    if (!session->input.router) return BERR_TRACE(NEXUS_UNKNOWN);

    if (server->settings.session[session->index].ir_input_mode != NEXUS_IrInputMode_eMax)
    {
        NEXUS_IrInputSettings irSettings;

        NEXUS_IrInput_GetDefaultSettings(&irSettings);
        irSettings.mode = server->settings.session[session->index].ir_input_mode;
        irSettings.dataReady.callback = nxserverlib_p_input_callback;
        irSettings.dataReady.context = session;
        irSettings.dataReady.param = NEXUS_InputRouterDevice_eIrInput;
        /* board must be wired for second channel: irSettings.channel_number = session->index; */
        session->input.irInput = NEXUS_IrInput_Open(session->index, &irSettings);
        if (!session->input.irInput) {BERR_TRACE(NEXUS_UNKNOWN);} /* keep going */
    }

#if NEXUS_HAS_KEYPAD
    if (session->index == 0) {
        NEXUS_KeypadSettings keypadSettings;
        NEXUS_Keypad_GetDefaultSettings(&keypadSettings);
        keypadSettings.dataReady.callback = nxserverlib_p_input_callback;
        keypadSettings.dataReady.context = session;
        keypadSettings.dataReady.param = NEXUS_InputRouterDevice_eKeypad;
        session->input.keypad = NEXUS_Keypad_Open(0, &keypadSettings);
    }
#endif

    if (server->settings.session[session->index].evdevInput) {
        struct nxserver_evdev_settings evdevSettings;
        nxserver_evdev_get_default_settings(&evdevSettings);
        evdevSettings.eventReady.callback = nxserverlib_p_input_callback;
        evdevSettings.eventReady.context = session;
        evdevSettings.eventReady.param = NEXUS_InputRouterDevice_eEvdev;
        session->input.evdevInput = nxserverlib_evdev_init(&evdevSettings);
        if (!session->input.evdevInput) {BERR_TRACE(NEXUS_UNKNOWN);} /* keep going */
    }

    return 0;
}
void uninit_input_devices(struct b_session *session)
{
    if (session->input.router) {
        NEXUS_InputRouter_Destroy(session->input.router);
        session->input.router = NULL;
    }
    if (session->input.irInput) {
        NEXUS_IrInput_Close(session->input.irInput);
        session->input.irInput = NULL;
    }
#if NEXUS_HAS_KEYPAD
    if (session->input.keypad) {
        NEXUS_Keypad_Close(session->input.keypad);
        session->input.keypad = NULL;
    }
#endif
    if (session->input.evdevInput) {
        nxserverlib_evdev_uninit(session->input.evdevInput);
    }
}
#else
int init_input_devices(struct b_session *session)
{
    BSTD_UNUSED(session);
    return 0;
}
void uninit_input_devices(struct b_session *session)
{
    BSTD_UNUSED(session);
}
#endif

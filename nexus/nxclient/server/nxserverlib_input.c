/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
#include "nxserverlib_impl.h"
#include "nxserverlib_evdev.h"

#if NEXUS_HAS_CEC
#include "nexus_cec.h"
#endif

BDBG_MODULE(nxserverlib_input);

#if NEXUS_HAS_CEC
struct nxserver_cec_ir_input
{
    NEXUS_CecHandle hCec;
};
static struct
{
    const char *strName;
    uint32_t   codeRemoteA;
    uint32_t   codeCirNec;
} g_cecRemoteCodes[] =
{
    /* strName           RemoteA     CirNec    */
    { "select",         0x0000e011, 0xF708FF00, },
    { "up",             0x00009034, 0xB14EFF00, },
    { "down",           0x00008035, 0xF30CFF00, },
    { "left",           0x00007036, 0xF40BFF00, },
    { "right",          0x00006037, 0xB649FF00, },
    { "back",           0x00000000, 0x00000000, },
    { "delete",         0x00000000, 0x00000000, },
    { "chUp",           0x0000500b, 0xF609FF00, },
    { "chDown",         0x0000400c, 0xF20DFF00, },
    { "volUp",          0x0000300d, 0xB748FF00, },
    { "volDown",        0x0000200e, 0xB34CFF00, },
    { "mute",           0x0000100f, 0xFE01FF00, },
    { "play",           0x0000401b, 0xE21DFF00, },
    /* duplicate play code for black RemoteA */
    { "play",           0x00005038, 0xE21DFF00, },
    { "stop",           0x00004039, 0xA35CFF00, },
    { "pause",          0x0000001f, 0xE31CFF00, },
    { "rewind",         0x0000101e, 0xE619FF00, },
    { "forward",        0x0000201d, 0xA659FF00, },
    { "record",         0x0000C031, 0xAB54FF00, },
    { "menu",           0x00006019, 0xB04FFF00, },
    { "info",           0x0000A033, 0xF00FFF00, },
    { "exit",           0x0000D012, 0x00000000, },
    { "dot",            0x0000B014, 0xAC53FF00, },
    /* duplicate play code for black RemoteA */
    { "dot",            0x0000F010, 0xAC53FF00, },
    { "enter",          0x00009016, 0xEC13FF00, },
    { "last",           0x0000C013, 0xF906FF00, },
    { "pip",            0x00008017, 0xE916FF00, },
    { "swap",           0x00006028, 0xED12FF00, },
    { "jumpFwd",        0x0000203B, 0xE718FF00, },
    { "jumpRev",        0x0000303A, 0xA758FF00, },
    { "power",          0x0000600A, 0xF50AFF00, },
    { "fav1",           0x00000000, 0xFD02FF00, },
    { "fav2",           0x00000000, 0xFC03FF00, },
    { "fav3",           0x00000000, 0xBD42FF00, },
    { "fav4",           0x00000000, 0xBC43FF00, },
    /* ascii codes */
    { "0",              0x00000000, 0xAD52FF00, },
    { "1",              0x0000F001, 0xE01FFF00, },
    { "2",              0x0000E002, 0xA15EFF00, },
    { "3",              0x0000D003, 0xA05FFF00, },
    { "4",              0x0000C004, 0xE41BFF00, },
    { "5",              0x0000B005, 0xA55AFF00, },
    { "6",              0x0000A006, 0xA45BFF00, },
    { "7",              0x00009007, 0xE817FF00, },
    { "8",              0x00008008, 0xA956FF00, },
    { "9",              0x00007009, 0xA857FF00, }
};

static uint32_t nxserver_p_cec_getRemoteCode(char *remoteStr, NEXUS_IrInputMode mode)
{
    unsigned i;
    for (i = 0; i < sizeof(g_cecRemoteCodes)/sizeof(g_cecRemoteCodes[0]); i++) {
        if (!strcmp(remoteStr,g_cecRemoteCodes[i].strName)) {
            switch(mode) {
            case NEXUS_IrInputMode_eRemoteA: return(g_cecRemoteCodes[i].codeRemoteA);
            case NEXUS_IrInputMode_eCirNec: return(g_cecRemoteCodes[i].codeCirNec);
            default: return 0;
            }
        }
    }
    return 0;
}
#endif
#if NEXUS_HAS_INPUT_ROUTER
static void build_ir_code(unsigned index, const NEXUS_IrInputEvent *irEvent, unsigned mode, NEXUS_InputRouterCode *pCode)
{
    NEXUS_InputRouter_GetDefaultCode(pCode);
    pCode->deviceType = NEXUS_InputRouterDevice_eIrInput;
    pCode->filterMask = 1<<pCode->deviceType;
    pCode->data.irInput.index = index;
    pCode->data.irInput.code = irEvent->code;
    pCode->data.irInput.repeat = irEvent->repeat;
    pCode->data.irInput.mode = mode;
    pCode->data.irInput.event = *irEvent;
}

static size_t get_ir_event(unsigned index, NEXUS_IrInputHandle irInput, unsigned mode, NEXUS_InputRouterCode *pCode )
{
    NEXUS_Error rc;
    size_t n = 0;
    if (irInput) {
        NEXUS_IrInputEvent irEvent;
        bool overflow;
        rc = NEXUS_IrInput_GetEvents(irInput, &irEvent, 1, &n, &overflow);
        BDBG_ASSERT(!rc);
        if (n == 1) {
            BDBG_MSG(("irinput code=%#x", irEvent.code));
            build_ir_code(index,&irEvent, mode, pCode);
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
    unsigned i;

    while(1)
    {
        bool foundCode = false;

        switch (param)
        {
            case NEXUS_InputRouterDevice_eIrInput:
                for (i=0;i<NXSERVER_IR_INPUTS;i++) {
                    if (get_ir_event(i, session->input.irInput[i], session->server->settings.session[session->index].ir_input.mode[i], &code)) {
                        foundCode = true;
                        break;
                    }
                }
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
    unsigned i;

    session->input.router = NEXUS_InputRouter_Create(session->index);
    if (!session->input.router) return BERR_TRACE(NEXUS_UNKNOWN);

    for (i=0;i<NXSERVER_IR_INPUTS;i++) {
        if (server->settings.session[session->index].ir_input.mode[i] != NEXUS_IrInputMode_eMax) {
            NEXUS_IrInputSettings irSettings;
            NEXUS_IrInput_GetDefaultSettings(&irSettings);
            irSettings.mode = server->settings.session[session->index].ir_input.mode[i];
            irSettings.channel_number = i; /* TODO: depends on interrupt device, which is hidden in nexus */
            irSettings.dataReady.callback = nxserverlib_p_input_callback;
            irSettings.dataReady.context = session;
            irSettings.dataReady.param = NEXUS_InputRouterDevice_eIrInput;
            session->input.irInput[i] = NEXUS_IrInput_Open(NEXUS_ANY_ID, &irSettings);
            if (!session->input.irInput[i]) {BERR_TRACE(NEXUS_UNKNOWN);} /* keep going */
        }
#if NEXUS_HAS_CEC
       if (server->settings.session[session->index].ir_input.cec) {
           session->input.cec = nxserverlib_init_cec_ir_input(session);
        }
#endif
    }

#if NEXUS_HAS_KEYPAD
    if (server->settings.session[session->index].keypad) {
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
    unsigned i;
    if (session->input.router) {
        NEXUS_InputRouter_Destroy(session->input.router);
        session->input.router = NULL;
    }
    for (i=0;i<NXSERVER_IR_INPUTS;i++) {
        if (session->input.irInput[i]) {
            NEXUS_IrInput_Close(session->input.irInput[i]);
            session->input.irInput[i] = NULL;
        }
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
#if NEXUS_HAS_CEC
    if (session->input.cec) {
        nxserverlib_uninit_cec_ir_input(session->input.cec);
    }
#endif
}
#if NEXUS_HAS_CEC
static void cecMsgReceived_callback(void *context, int param)
{
    struct b_session *session = context;
    NEXUS_IrInputEvent irEvent;
    NEXUS_InputRouterCode inputRouterCode;
    NEXUS_CecStatus status;
    NEXUS_CecReceivedMessage receivedMessage;
    NEXUS_Error rc;
    char msgBuffer[3*(NEXUS_CEC_MESSAGE_DATA_SIZE +1)];
    unsigned i, j;

    BSTD_UNUSED(param);
    rc = NEXUS_Cec_GetStatus(session->input.cec->hCec, &status);
    if (rc) { BERR_TRACE(rc); return; }

    if (!status.messageReceived) return;

    rc = NEXUS_Cec_ReceiveMessage(session->input.cec->hCec, &receivedMessage);
    if (rc) { BERR_TRACE(rc); return; }

    for (i = 0, j = 0; i < receivedMessage.data.length && j<(sizeof(msgBuffer)-1); i++) {
       j += BKNI_Snprintf(msgBuffer + j, sizeof(msgBuffer)-j, "%c",receivedMessage.data.buffer[i]);
    }

    BKNI_Memset(&irEvent, 0, sizeof(irEvent));
    irEvent.code = nxserver_p_cec_getRemoteCode(msgBuffer, session->server->settings.session[session->index].ir_input.mode[0]);
    if (irEvent.code) {
        BDBG_MSG(("CEC message %s -> IR code 0x%08x", msgBuffer, irEvent.code));
        build_ir_code(0,&irEvent,0,&inputRouterCode);
        NEXUS_InputRouter_SendCode(session->input.router, &inputRouterCode);
    }
}
nxserver_cec_ir_input_t nxserverlib_init_cec_ir_input(struct b_session *session)
{
    NEXUS_Error rc;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_CecSettings cecSettings;
    NEXUS_HdmiOutputHandle hdmiOutput;
    nxserver_cec_ir_input_t cec;

    NEXUS_Platform_GetConfiguration(&platformConfig);
    hdmiOutput = platformConfig.outputs.hdmi[0];
    if (!hdmiOutput) {
        BDBG_ERR(("unable to get hdmioutput handle"));
        return NULL;
    }
    cec = BKNI_Malloc(sizeof(*cec));
    if (!cec) {
        BDBG_ERR(("Unable to alloc memory for cec structure"));
        return NULL;
    }
    memset(cec, 0, sizeof(*cec));

    cec->hCec = platformConfig.outputs.cec[0];
    if (!cec->hCec) {
        BDBG_ERR(("unable to open CEC"));
        BKNI_Free(cec);
        return NULL;
    }
    rc = NEXUS_Cec_SetHdmiOutput(cec->hCec, hdmiOutput);
    if (rc) { BERR_TRACE(rc);}
    session->input.cec = cec;
    NEXUS_Cec_GetSettings(cec->hCec, &cecSettings);
    cecSettings.messageReceivedCallback.callback = cecMsgReceived_callback;
    cecSettings.messageReceivedCallback.context = session;
    rc = NEXUS_Cec_SetSettings(cec->hCec, &cecSettings);
    if (rc) { BERR_TRACE(rc);}

    /* Enable CEC core */
    NEXUS_Cec_GetSettings(cec->hCec, &cecSettings);
    cecSettings.enabled = true;
    rc = NEXUS_Cec_SetSettings(cec->hCec, &cecSettings);
    if (rc) { BERR_TRACE(rc);}

    return cec;
}
void nxserverlib_uninit_cec_ir_input(nxserver_cec_ir_input_t cec)
{
    BKNI_Free(cec);
}
#endif
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

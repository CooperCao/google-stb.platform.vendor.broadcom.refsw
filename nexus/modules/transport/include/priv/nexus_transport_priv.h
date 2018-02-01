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
#ifndef NEXUS_TRANSPORT_PRIV_H__
#define NEXUS_TRANSPORT_PRIV_H__

#include "bxpt.h"
#include "bxpt_rave.h"
#include "nexus_parser_band.h"
#include "nexus_recpump.h"
#include "nexus_playpump.h"

#if NEXUS_TRANSPORT_EXTENSION_TSMF
#include "nexus_tsmf.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
If you get the XPT handle, you must first lock the transport module, get the handle,
use the handle, then unlock the transport module. After unlocking the transport module
you may not use the XPT handle.

This is allowed:
    NEXUS_Module_Lock(transportModule);
    {
        BXPT_Handle xpt;
        NEXUS_Transport_GetXpt_priv(&xpt);
        BXPT_Foo(xpt);
    }
    NEXUS_Module_Unlock(transportModule);

This is not allowed:
    BXPT_Handle xpt;
    NEXUS_Module_Lock(transportModule);
    NEXUS_Transport_GetXpt_priv(&xpt);
    NEXUS_Module_Unlock(transportModule);
    BXPT_Foo(xpt); <-- this is a bug

To ensure you are following these rules, we recommend that you do not cache or otherwise store
the XPT handle. Only use a local variable that goes out of scope before unlocking the module (as above).
**/
void NEXUS_Transport_GetXpt_priv(
    BXPT_Handle *pXpt /* [out] */
    );

struct NEXUS_MtsifParserBandSettings {
    unsigned index;
    NEXUS_ParserBandSettings settings;
    struct {
        bool valid;
        unsigned hwIndex;
        bool pending; /* if true, the following settings have changed */
#if NEXUS_TRANSPORT_EXTENSION_TSMF
        NEXUS_TsmfSettings settings;
#endif
    } tsmf;
};

void NEXUS_ParserBand_GetMtsifConnections_priv(
    NEXUS_FrontendConnectorHandle connector,
    struct NEXUS_MtsifParserBandSettings *pSettings,
    unsigned numEntries,
    unsigned *pNumReturned
    );

typedef enum NEXUS_MtsifPidChannelState {
    NEXUS_MtsifPidChannelState_eNone = 0,
    NEXUS_MtsifPidChannelState_eChanged,
    NEXUS_MtsifPidChannelState_eClosed
} NEXUS_MtsifPidChannelState;

struct NEXUS_MtsifPidChannelSettings {
    NEXUS_MtsifPidChannelState state;
    NEXUS_PidChannelStatus status;
    NEXUS_FrontendConnectorHandle frontend;
};

void NEXUS_TransportModule_GetMtsifPidChannels_priv(
    struct NEXUS_MtsifPidChannelSettings *pSettings,
    unsigned numEntries,
    unsigned *pNumReturned
    );
void NEXUS_TransportModule_GetPidchannelEvent(BKNI_EventHandle *event);

struct NEXUS_TbgHostParserSettings {
    bool enabled;
    uint8_t markerTag;
    struct {
        bool changed;
        unsigned primaryParserBandIndex;
        unsigned unmappedPidChIndex;
    } band[NEXUS_NUM_PARSER_BANDS];
};

void NEXUS_Tbg_GetHostParserSettings_priv(
    struct NEXUS_TbgHostParserSettings *pSettings
    );

void NEXUS_ParserBand_P_MtsifErrorStatus_priv(unsigned lengthError, unsigned transportError);
bool NEXUS_TransportModule_P_IsMtsifEncrypted(void);
void NEXUS_TransportModule_P_ForceMtsifEncrypted(unsigned index);

NEXUS_Error NEXUS_TransportModule_PostInit_priv(
    RaveChannelOpenCB rave_regver /* synchronous rave region verification. must call into code which is already locked. */
    );

void NEXUS_TransportModule_GetRaveFirmware_isrsafe(
    void **ppBxptRaveInitData,
    unsigned *pBxptRaveInitDataSize  /* size of firmware in bytes. */
    );

NEXUS_Error NEXUS_TransportModule_PostResume_priv(void);

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Playpump);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Recpump);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

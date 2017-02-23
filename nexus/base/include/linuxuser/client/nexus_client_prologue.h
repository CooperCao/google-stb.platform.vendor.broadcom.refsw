/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2004-2016 Broadcom. All rights reserved.
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
*
***************************************************************************/

#ifndef _NEXUS_CLIENT_PROLOGUE_H_
#define _NEXUS_CLIENT_PROLOGUE_H_

#include "nexus_types.h"
#include "../server/nexus_ipc_api.h"
#include "priv/nexus_core.h"

#define NEXUS_P_API_ID(module, api) NEXUS_P_API_##module##api##_id

typedef struct NEXUS_P_ClientModule *NEXUS_P_ClientModuleHandle;

typedef struct NEXUS_P_ClientCall_State {
    NEXUS_P_ClientModuleHandle module;
    void *data;
    unsigned size;
    unsigned varargs_begin;
    unsigned varargs_offset;
    unsigned header;
} NEXUS_P_ClientCall_State;

NEXUS_Error NEXUS_P_Client_LockModule(NEXUS_P_ClientModuleHandle module, void **data, unsigned *size);
void NEXUS_P_Client_UnlockModule(NEXUS_P_ClientModuleHandle module);
NEXUS_Error NEXUS_P_Client_CallServer(NEXUS_P_ClientModuleHandle module, const void *in_params, unsigned in_param_size, void **out_params, unsigned out_param_mem, unsigned *p_out_param_size);

NEXUS_Error NEXUS_P_ClientCall_Begin(NEXUS_P_ClientModuleHandle module, NEXUS_P_ClientCall_State *state, unsigned header);
NEXUS_Error NEXUS_P_ClientCall_Call(NEXUS_P_ClientCall_State *state);
void NEXUS_P_ClientCall_VarArg_Begin(NEXUS_P_ClientCall_State *state, unsigned param_size);
NEXUS_Error NEXUS_P_ClientCall_InVarArg(NEXUS_P_ClientCall_State *state, unsigned vararg_size, const void *src, int *field, bool *is_null);
NEXUS_Error NEXUS_P_ClientCall_InVarArg_AddrField(NEXUS_P_ClientCall_State *state, unsigned struct_size, unsigned field_offset, unsigned count, int varArg, int *varArgField);
void NEXUS_P_ClientCall_End(NEXUS_P_ClientCall_State *state);
void *NEXUS_P_ClientCall_OffsetToAddr(NEXUS_Addr addr);
NEXUS_Addr NEXUS_P_ClientCall_AddrToOffset(const void *ptr);
void NEXUS_P_ClientCall_OutVarArg(NEXUS_P_ClientCall_State *state, unsigned vararg_size, void *dest, int field);

/* define prototypes for init/uninit functions */
#define NEXUS_PLATFORM_P_DRIVER_MODULE(module) \
BERR_Code nexus_client_##module##_init(NEXUS_P_ClientModuleHandle module); \
void nexus_client_##module##_uninit(void);
#include "nexus_driver_modules.h"
#undef NEXUS_PLATFORM_P_DRIVER_MODULE



#endif /* _NEXUS_CLIENT_PROLOGUE_H_ */

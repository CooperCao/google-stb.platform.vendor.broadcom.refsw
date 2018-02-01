/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef NEXUS_SECURITY_MODULE_H__
#define NEXUS_SECURITY_MODULE_H__

#include "nexus_security_thunks.h"
#include "nexus_base.h"
#include "nexus_security_datatypes.h"
#include "nexus_security.h"
#include "nexus_security_misc.h"
#include "nexus_security_init.h"
#include "nexus_region_verification.h"

#include "priv/nexus_core_security.h"
#include "priv/nexus_security_priv.h"

#if NEXUS_KEYLADDER
#include "nexus_keyladder.h"
#endif

#if NEXUS_SECURITY_VISTA_CWC
#include "nexus_keyladder_vista.h"
#endif

#if NEXUS_SECURITY_SCTE52
#include "nexus_dsk_scte52.h"
#endif

#if NEXUS_SECURITY_IRDETO
#include "nexus_irdeto.h"
#endif

#if NEXUS_SECURITY_ALPINE
#include "nexus_alpine.h"
#endif

#if NEXUS_OTPMSP
#include "nexus_otpmsp.h"
#endif

#if NEXUS_ZEUS_VERSION >= NEXUS_ZEUS_VERSION_CALC(1,0)
#include "priv/nexus_security_regver_priv.h"
#endif

#if NEXUS_SECURITY_BSECK_COMMAND
#include "nexus_security_bseckcmd.h"
#endif

#if NEXUS_OTPID
#include "nexus_read_otp_id.h"
#endif

#if NEXUS_OTPKEY
#include "nexus_prog_otp_key.h"
#endif

#if NEXUS_USERCMD
#include "nexus_random_number.h"
#include "nexus_hmac_sha_cmd.h"

#if NEXUS_ZEUS_VERSION >= NEXUS_ZEUS_VERSION_CALC(1,0)
#include "nexus_bsp_config.h"
#endif
#endif

#if NEXUS_SECURITY_RAWCMD
#include "nexus_security_rawcommand.h"
#endif

#if NEXUS_SECURITY_SECUREACCESS
#include "nexus_secureaccess.h"
#endif

#if NEXUS_SECURITY_SECURERSA
#include "nexus_security_rsa.h"
#endif

#if NEXUS_SECURITY_AVKEYLADDER
#include "nexus_avkeyladder.h"
#endif

#ifdef NEXUS_SECURITY_MSIPTV
#include "nexus_security_msiptv.h"
#endif

#ifdef NEXUS_SECURITY_GENROOTKEY
#include "nexus_genrootkey.h"
#endif

#ifdef NEXUS_SECURITY_PKL
#include "nexus_security_pkl.h"

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_PklInterface);
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NEXUS_MODULE_SELF
#error Cant be in two modules at the same time
#endif

#define NEXUS_MODULE_NAME security
#define NEXUS_MODULE_SELF NEXUS_P_SecurityModule

#include "bhsm.h"
typedef struct NEXUS_Security_P_Handle
{
    BHSM_Handle hHsm;
    NEXUS_SecurityModuleInternalSettings *pSettings;
} NEXUS_Security_P_Handle;


#define MAKE_HSM_ERR(x) (NEXUS_SECURITY_HSM_ERROR | x)

#if NEXUS_ZEUS_VERSION >= NEXUS_ZEUS_VERSION_CALC(1,0)
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_VirtualKeyLadder);
#endif

/* global handle. there is no global data. */
extern NEXUS_ModuleHandle NEXUS_P_SecurityModule;


/* converts a nexus keyslot type to a HSM keyslot type. */
BCMD_XptSecKeySlot_e NEXUS_Security_P_mapNexus2Hsm_KeyslotType( NEXUS_SecurityKeySlotType nexusType,
                                                                NEXUS_SecurityEngine engine );

/* converts a nexus keylayer type to a HSM keylayer type. */
BCMD_KeyRamBuf_e NEXUS_Security_P_mapNexus2Hsm_KeyLayer( NEXUS_SecurityKeyLayer keyLayer );


NEXUS_OBJECT_CLASS_DECLARE(NEXUS_RegionVerify);
/* Init/Uninit functions for region Verification */
NEXUS_Error NEXUS_RegionVerify_Init( void );
void NEXUS_RegionVerify_Uninit( void );

#ifdef __cplusplus
}
#endif

#endif

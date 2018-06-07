/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#ifndef NEXUS_SECURITY_MODULE_H__
#define NEXUS_SECURITY_MODULE_H__

#include "nexus_security_thunks.h"
#include "nexus_base.h"
#include "nexus_security_datatypes.h"
#include "nexus_security.h"
#include "nexus_security_init.h"
#include "nexus_security.h"
#include "nexus_security_datatypes.h"
#include "nexus_hash.h"
#include "nexus_hmac.h"
#include "nexus_keyladder.h"
#include "nexus_keyslot.h"
#include "nexus_otp_datasection.h"
#include "nexus_otp_key.h"
#include "nexus_otp_msp.h"
#include "nexus_random_number.h"
#include "nexus_regver.h"
#include "nexus_rsa.h"
#include "nexus_security_common.h"
#include "nexus_regver.h"
#include "nexus_regver_rsa.h"
#include "nexus_signed_command.h"
#include "nexus_pcie_window.h"
#ifdef NEXUS_POWAY_SUPPORT
 #include "nexus_poway.h"
#endif
#ifdef NEXUS_DUPLE_KEYLADDER_SUPPORT
 #include "nexus_duple_keyladder.h"
#endif
#ifdef NEXUS_ENCINITAS_SUPPORT
 #include "nexus_encinitas.h"
#endif
#include "priv/nexus_regver_priv.h"
#include "priv/nexus_core_security.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NEXUS_MODULE_SELF
#error Cant be in two modules at the same time
#endif

#define NEXUS_MODULE_NAME security
#define NEXUS_MODULE_SELF NEXUS_P_SecurityModule

/* global handle. there is no global data. */
extern NEXUS_ModuleHandle NEXUS_P_SecurityModule;

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Rsa);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_RegionVerify);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Hmac);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Hash);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_KeyLadder);

#ifdef NEXUS_POWAY_SUPPORT
 NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Poway);
#endif
#ifdef NEXUS_DUPLE_KEYLADDER_SUPPORT
 NEXUS_OBJECT_CLASS_DECLARE(NEXUS_DupleKeyladder);
#endif


#ifdef __cplusplus
}
#endif

#endif

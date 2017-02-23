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
#include "nexus_types.h"
#include "nexus_platform.h"
#include "nxclient.h"
#include "bipc_server.h"
#include "nxserver_ipc.h"
#include "nxserverlib.h"

#define B_NEXUS_COMPAT_TYPE(type) NEXUS_Compat_##type
#define B_NEXUS_COMPAT_TYPE_FAKE_HANDLE(handle) void *
#define B_NEXUS_COMPAT_TYPE_MISC(type,alias) type

#define B_SIZEOF(type, field) sizeof(((type *)0)->field)
#define VERIFY_BEGIN(x)    static void nxclient_p_verify_##x(void) {
#define VERIFY_STRUCT(type) BDBG_CASSERT(sizeof(type) == sizeof(B_NEXUS_COMPAT_TYPE(type)));
#define VERIFY_FIELD(type, field, sub_type) BDBG_CASSERT( (offsetof(type, field) == offsetof(B_NEXUS_COMPAT_TYPE(type), field)) && (B_SIZEOF(type,field) == B_SIZEOF(B_NEXUS_COMPAT_TYPE(type),field)) );
#define VERIFY_FIELD_STRUCT(type, field, sub_type) BDBG_CASSERT(0);
#define VERIFY_GENERIC(type, field, sub_type) BDBG_CASSERT(sizeof(sub_type)==sizeof(int));
#define VERIFY_FAKE_HANDLE(type, field, sub_type) BDBG_CASSERT(sizeof(sub_type)==sizeof(void *));
#define VERIFY_END(x)      return;}

#include "ipc_stubs_verify.h"

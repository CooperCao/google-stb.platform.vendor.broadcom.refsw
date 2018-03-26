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

#ifndef GATEKEEPER_TL_H__
#define GATEKEEPER_TL_H__


#include "bstd.h"
#include "bkni.h"

#include "nexus_security_types.h"
#include "bsagelib_types.h"
#include "bsagelib_crypto_types.h"

#include "keymaster_types.h"
#include "keymaster_err.h"


#ifdef __cplusplus
extern "C"
{
#endif


/* Max length of a gatekeeper password */
#define GATEKEEPER_MAX_PASSWORD     128

/* This handle is used to store the context of a GatekeeperTl instance */
typedef struct GatekeeperTl_Instance *GatekeeperTl_Handle;

/* Macro for swapping endianness of U64 */
#define GK_U64_MASK        0xffull
#define GK_UINT64_SWAP(v)  (((v) & (GK_U64_MASK <<  0)) << 56) | \
                           (((v) & (GK_U64_MASK <<  8)) << 40) | \
                           (((v) & (GK_U64_MASK << 16)) << 24) | \
                           (((v) & (GK_U64_MASK << 24)) <<  8) | \
                           (((v) & (GK_U64_MASK << 32)) >>  8) | \
                           (((v) & (GK_U64_MASK << 40)) >> 24) | \
                           (((v) & (GK_U64_MASK << 48)) >> 40) | \
                           (((v) & (GK_U64_MASK << 56)) >> 56)


/***************************************************************************
Summary:
Gatekeeper init settings

See Also:
GatekeeperTl_GetDefaultInitSettings()
GatekeeperTl_Init()
***************************************************************************/
typedef struct
{
    char drm_binfile_path[256];
} GatekeeperTl_InitSettings;

/***************************************************************************
Summary:
Gatekeeper struct to encapsulate the length and pointer for a password.

See Also:
***************************************************************************/
typedef struct
{
    uint32_t size;          /* Size of buffer below */
    uint8_t *buffer;
} GatekeeperTl_Password;

/***************************************************************************
Summary:
Get default settings for loading the Gatekeeper module on SAGE

Description:
Retrieve the set of default values used to load the Gatekeeper module on SAGE

See Also:
GatekeeperTl_Init()
***************************************************************************/
void
GatekeeperTl_GetDefaultInitSettings(GatekeeperTl_InitSettings *pModuleSettings);

/***************************************************************************
Summary:
Initialize an instance of the Gatekeeper module on SAGE

See Also:
GatekeeperTl_Uninit()
***************************************************************************/
BERR_Code GatekeeperTl_Init(
    GatekeeperTl_Handle *pHandle,
    GatekeeperTl_InitSettings *pModuleSettings);

/***************************************************************************
Summary:
Uninitialize the given instance of the Gatekeeper module on SAGE

See Also
GatekeeperTl_Init()
***************************************************************************/
void GatekeeperTl_Uninit(GatekeeperTl_Handle handle);

/***************************************************************************
Summary:
Enroll a password for a user, returning a password handle. If there is an
incoming password handle, change the existing handle password.

Returns:
  BERR_SUCCESS if successful
  BSAGE_ERR_KM_RETRY if in_enroll_password incorrect for in_password_handle
  BERR_INVALID_PARAMETER for incorrect parameters

See Also:
GatekeeperTl_Init()
***************************************************************************/
BERR_Code GatekeeperTl_Enroll(
    GatekeeperTl_Handle handle,
    uint32_t in_user_id,                                /* Android uid */
    gk_password_handle_t *in_password_handle,           /* Optional password handle, if changing existing password */
    GatekeeperTl_Password *in_enroll_password,          /* Optional password associated with original handle, above */
    GatekeeperTl_Password *in_provided_password,        /* Password for new password handle */
    uint32_t *out_retry_timeout,                        /* Out: milliseconds before retry allowed */
    gk_password_handle_t *out_password_handle);         /* Out: created password handle */

/***************************************************************************
Summary:
Verify password against a password handle and return an auth token.

Returns:
  BERR_SUCCESS if successful
  BSAGE_ERR_KM_RETRY if in_provided_password incorrect for in_password_handle
  BERR_INVALID_PARAMETER for incorrect parameters

See Also:
GatekeeperTl_Enroll()
***************************************************************************/
BERR_Code GatekeeperTl_Verify(
    GatekeeperTl_Handle handle,
    uint32_t in_user_id,                                /* Android uid */
    uint64_t in_challenge,                              /* Challenge to embed in auth token */
    gk_password_handle_t *in_password_handle,           /* Password handle to authorize against */
    GatekeeperTl_Password *in_provided_password,        /* Password for password handle */
    uint32_t *out_retry_timeout,                        /* Out: milliseconds before retry allowed */
    km_hw_auth_token_t *out_auth_token);                /* Out: created auth token */


#ifdef __cplusplus
}
#endif


#endif /* GATEKEEPER_TL_H__*/

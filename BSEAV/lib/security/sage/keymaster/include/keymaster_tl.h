/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifndef KEYMASTER_TL_H__
#define KEYMASTER_TL_H__


#include "bstd.h"
#include "bkni.h"

#include "nexus_security_types.h"
#include "bsagelib_types.h"
#include "bsagelib_crypto_types.h"

#include "keymaster_types.h"
#include "keymaster_err.h"
#include "keymaster_tags.h"


#ifdef __cplusplus
extern "C"
{
#endif


/* This handle is used to store the context of a KeymasterTl instance */
typedef struct KeymasterTl_Instance *KeymasterTl_Handle;


/***************************************************************************
Summary:
Keymaster init settings

See Also:
KeymasterTl_GetDefaultInitSettings()
KeymasterTl_Init()
***************************************************************************/
typedef struct
{
    char drm_binfile_path[256];
} KeymasterTl_InitSettings;

/***************************************************************************
Summary:
Keymaster struct to encapsulate a block of data with size and pointer

See Also:
***************************************************************************/
typedef struct
{
    uint32_t size;          /* Size of buffer below */
    uint8_t *buffer;
} KeymasterTl_DataBlock;

/***************************************************************************
Summary:
Get default settings for loading the Keymaster module on SAGE

Description:
Retrieve the set of default values used to load the Keymaster module on SAGE

See Also:
KeymasterTl_Init()
***************************************************************************/
void
KeymasterTl_GetDefaultInitSettings(KeymasterTl_InitSettings *pModuleSettings);

/***************************************************************************
Summary:
Initialize an instance of the Keymaster module on SAGE

See Also:
KeymasterTl_Uninit()
***************************************************************************/
BERR_Code KeymasterTl_Init(
    KeymasterTl_Handle *pHandle,
    KeymasterTl_InitSettings *pModuleSettings);

/***************************************************************************
Summary:
Uninitialize the given instance of the Keymaster module on SAGE

See Also
KeymasterTl_Init()
***************************************************************************/
void KeymasterTl_Uninit(KeymasterTl_Handle handle);

/***************************************************************************
Summary:
Configure the Keymaster handle. Call KeymasterTl_Init first to create handle.

See Also:
KeymasterTl_Init()
***************************************************************************/
BERR_Code KeymasterTl_Configure(
    KeymasterTl_Handle handle,
    KM_Tag_ContextHandle in_params);

/***************************************************************************
Summary:
Add RNG entropy. Call KeymasterTl_Configure first.

See Also:
KeymasterTl_Configure()
***************************************************************************/
BERR_Code KeymasterTl_AddRngEntropy(
    KeymasterTl_Handle handle,
    KeymasterTl_DataBlock *in_data);

/***************************************************************************
Summary:
Generate key.  Call KeymasterTl_Configure first. The function will allocate
a buffer of the correct size and fill in out_key_blob. The caller must call
SRAI_Memory_Free on the out_key_blob->buffer.

See Also:
KeymasterTl_Configure()
***************************************************************************/
BERR_Code KeymasterTl_GenerateKey(
    KeymasterTl_Handle handle,
    KM_Tag_ContextHandle in_key_params,
    KeymasterTl_DataBlock *out_key_blob);



/***************************************************************************
Summary:
KeymasterTl_GetKeyCharacteristics settings structure
***************************************************************************/
typedef struct KeymasterTl_GetKeyCharacteristicsSettings {
    KeymasterTl_DataBlock in_key_blob;
    KM_Tag_ContextHandle in_params;
    KM_Tag_ContextHandle out_hw_enforced;
    KM_Tag_ContextHandle out_sw_enforced;
} KeymasterTl_GetKeyCharacteristicsSettings;

/***************************************************************************
Summary:
Fill in default settings
***************************************************************************/
void KeymasterTl_GetDefaultKeyCharacteristicsSettings(
    KeymasterTl_GetKeyCharacteristicsSettings *settings);

/***************************************************************************
Summary:
Get key characteristics.  Call KeymasterTl_Configure first. The function
returns a newly allocated tag context for out_hw_enforced and out_sw_enforced.
The caller must call KM_Tag_DeleteContext on both.

See Also:
KeymasterTl_Configure()
***************************************************************************/
BERR_Code KeymasterTl_GetKeyCharacteristics(
    KeymasterTl_Handle handle,
    KeymasterTl_GetKeyCharacteristicsSettings *settings);

/***************************************************************************
Summary:
KeymasterTl_ImportKey settings structure
***************************************************************************/
typedef struct KeymasterTl_ImportKeySettings {
    KM_Tag_ContextHandle in_key_params;
    km_key_format_t in_key_format;
    KeymasterTl_DataBlock in_key_blob;
    KeymasterTl_DataBlock out_key_blob;
} KeymasterTl_ImportKeySettings;

/***************************************************************************
Summary:
Fill in default settings
***************************************************************************/
void KeymasterTl_GetDefaultImportKeySettings(
    KeymasterTl_ImportKeySettings *settings);

/***************************************************************************
Summary:
Import key.  Call KeymasterTl_Configure first. The function allocates an
out_key_blob.

See Also:
KeymasterTl_Configure()
***************************************************************************/
BERR_Code KeymasterTl_ImportKey(
    KeymasterTl_Handle handle,
    KeymasterTl_ImportKeySettings *settings);

/***************************************************************************
Summary:
KeymasterTl_ExportKey settings structure
***************************************************************************/
typedef struct KeymasterTl_ExportKeySettings {
    km_key_format_t in_key_format;
    KeymasterTl_DataBlock in_key_blob;
    KM_Tag_ContextHandle in_params;
    KeymasterTl_DataBlock out_key_blob;
} KeymasterTl_ExportKeySettings;

/***************************************************************************
Summary:
Fill in default settings
***************************************************************************/
void KeymasterTl_GetDefaultExportKeySettings(
    KeymasterTl_ExportKeySettings *settings);

/***************************************************************************
Summary:
Export key.  Call KeymasterTl_Configure first. The function allocates an
out_key_blob.

See Also:
KeymasterTl_Configure()
***************************************************************************/
BERR_Code KeymasterTl_ExportKey(
    KeymasterTl_Handle handle,
    KeymasterTl_ExportKeySettings *settings);

/***************************************************************************
Summary:
KeymasterTl_AttestKey settings structure
***************************************************************************/
typedef struct KeymasterTl_AttestKeySettings {
    KeymasterTl_DataBlock in_key_blob;
    KM_Tag_ContextHandle in_params;
    km_cert_chain_t out_cert_chain;
    KeymasterTl_DataBlock out_cert_chain_buffer;
} KeymasterTl_AttestKeySettings;

/***************************************************************************
Summary:
Fill in default settings
***************************************************************************/
void KeymasterTl_GetDefaultAttestKeySettings(
    KeymasterTl_AttestKeySettings *settings);

/***************************************************************************
Summary:
Attest key.  Call KeymasterTl_Configure first. The function must be passed
in the out_cert_chain pointer with data space pre-allocated. It will allocate
the data space for the out_cert_chain_buffer, which must be freed after use
by calling SRAI_Memory_Free.

See Also:
KeymasterTl_Configure()
***************************************************************************/
BERR_Code KeymasterTl_AttestKey(
    KeymasterTl_Handle handle,
    KeymasterTl_AttestKeySettings *settings);

/***************************************************************************
Summary:
Upgrade key.  Call KeymasterTl_Configure first. The function allocates an
out_key_blob.

See Also:
KeymasterTl_Configure()
***************************************************************************/
BERR_Code KeymasterTl_UpgradeKey(
    KeymasterTl_Handle handle,
    KeymasterTl_DataBlock *in_key_blob,
    KM_Tag_ContextHandle in_params,
    KeymasterTl_DataBlock *out_key_blob);

/***************************************************************************
Summary:
Delete key.  Call KeymasterTl_Configure first.

See Also:
KeymasterTl_Configure()
***************************************************************************/
BERR_Code KeymasterTl_DeleteKey(
    KeymasterTl_Handle handle,
    KeymasterTl_DataBlock *in_key_blob);

/***************************************************************************
Summary:
Delete all keys.  Call KeymasterTl_Configure first.

See Also:
KeymasterTl_Configure()
***************************************************************************/
BERR_Code KeymasterTl_DeleteAllKeys(KeymasterTl_Handle handle);

/***************************************************************************
Summary:
KeymasterTl_CryptoBegin settings structure
***************************************************************************/
typedef struct KeymasterTl_CryptoBeginSettings {
    km_purpose_t purpose;
    KeymasterTl_DataBlock in_key_blob;
    NEXUS_KeySlotHandle hKeyslot;
    KM_Tag_ContextHandle in_params;
    KM_Tag_ContextHandle out_params;
} KeymasterTl_CryptoBeginSettings;

/***************************************************************************
Summary:
Fill in default settings
***************************************************************************/
void KeymasterTl_GetDefaultCryptoBeginSettings(
    KeymasterTl_CryptoBeginSettings *settings);

/***************************************************************************
Summary:
Start a crypto operation.  Call KeymasterTl_Configure first.

See Also:
KeymasterTl_Configure()
KeymasterTl_CryptoUpdate()
KeymasterTl_CryptoFinish()
KeymasterTl_CryptoAbort()
***************************************************************************/
BERR_Code KeymasterTl_CryptoBegin(
    KeymasterTl_Handle handle,
    KeymasterTl_CryptoBeginSettings *settings,
    km_operation_handle_t *out_operation_handle);

/***************************************************************************
Summary:
KeymasterTl_CryptoUpdateSettings settings structure
***************************************************************************/
typedef struct KeymasterTl_CryptoUpdateSettings {
    KM_Tag_ContextHandle in_params;
    KeymasterTl_DataBlock in_data;
    uint32_t out_input_consumed;
    KM_Tag_ContextHandle out_params;
    KeymasterTl_DataBlock out_data;
    uint32_t out_data_size;
} KeymasterTl_CryptoUpdateSettings;

/***************************************************************************
Summary:
Fill in default settings
***************************************************************************/
void KeymasterTl_GetDefaultCryptoUpdateSettings(
    KeymasterTl_CryptoUpdateSettings *settings);

/***************************************************************************
Summary:
Add data to a crypto operation.  Call KeymasterTl_CryptoBegin first.
Sufficient space for out_data must be passed in to this function. The size
depends on the algorithm and purpose. The out_data->buffer and in_data->buffer
must be accessible by SAGE so a memcpy to/from a temporary buffer is not
required. out_data_size returns the amount of data put into out_data.

See Also:
KeymasterTl_CryptoBegin()
KeymasterTl_CryptoUpdate()
KeymasterTl_CryptoFinish()
KeymasterTl_CryptoAbort()
***************************************************************************/
BERR_Code KeymasterTl_CryptoUpdate(
    KeymasterTl_Handle handle,
    km_operation_handle_t in_operation_handle,
    KeymasterTl_CryptoUpdateSettings *settings);

/***************************************************************************
Summary:
KeymasterTl_CryptoFinishSettings settings structure
***************************************************************************/
typedef struct KeymasterTl_CryptoFinishSettings {
    KM_Tag_ContextHandle in_params;
    KeymasterTl_DataBlock in_data;
    KeymasterTl_DataBlock in_signature;
    KM_Tag_ContextHandle out_params;
    KeymasterTl_DataBlock out_data;
    uint32_t out_data_size;
} KeymasterTl_CryptoFinishSettings;

/***************************************************************************
Summary:
Fill in default settings
***************************************************************************/
void KeymasterTl_GetDefaultCryptoFinishSettings(
    KeymasterTl_CryptoFinishSettings *settings);

/***************************************************************************
Summary:
Finish a crypto operation, with optional data and signature.
Call KeymasterTl_CryptoBegin first. in_data is optional.
Sufficient space for out_data must be passed in to this function. The size
depends on the algorithm and purpose. An in_signature must be provided
when the purpose is VERIFY. The operation handle is invalid after calling
this function. The out_data->buffer and in_data->buffer must be accessible
by SAGE so a memcpy to/from a temporary buffer is not required.
out_data_size returns the amount of data put into out_data.

See Also:
KeymasterTl_CryptoBegin()
KeymasterTl_CryptoUpdate()
KeymasterTl_CryptoFinish()
KeymasterTl_CryptoAbort()
***************************************************************************/
BERR_Code KeymasterTl_CryptoFinish(
    KeymasterTl_Handle handle,
    km_operation_handle_t in_operation_handle,
    KeymasterTl_CryptoFinishSettings *settings);

/***************************************************************************
Summary:
Abort a crypto operation. Call KeymasterTl_CryptoBegin first. The operation
handle is invalid after calling this function.

See Also:
KeymasterTl_CryptoBegin()
KeymasterTl_CryptoUpdate()
KeymasterTl_CryptoFinish()
KeymasterTl_CryptoAbort()
***************************************************************************/
BERR_Code KeymasterTl_CryptoAbort(
    KeymasterTl_Handle handle,
    km_operation_handle_t in_operation_handle);


/***************************************************************************
Summary:
Get system configuration. Returns whether RPMB storage is enabled, which
is required to create rollback resistant keys (when rpmbEnabled is true).
When usingVms is true, RPMB data is being stored in VMS, rather than RPMB.
When rpmbEnabled is true, hwKeysAvailable will contain how many rollback
resistant keys are available.

See Also:
***************************************************************************/
BERR_Code KeymasterTl_GetConfiguration(
    KeymasterTl_Handle handle,
    bool *rpmbEnabled,
    bool *usingVms,
    uint32_t *hwKeysAvailable);


#ifdef __cplusplus
}
#endif


#endif /*KEYMASTER_TL_H__*/

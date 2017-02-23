/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include "drm_prdy.h"
#include "bdbg.h"
#include <string.h>
#include <stdio.h>
#include "bstd.h"
#include "bkni.h"
#include "drmmanager.h"
#include "drmbase64.h"
#include "drmmanagertypes.h"
#include "drmsoapxmlutility.h"
#include "oemcommon.h"
#include "nxclient.h"
#include "nexus_platform_client.h"
#include "drmconstants.h"
#include "drm_data.h"
#ifdef PLAYREADY_HOST_IMPL
#include "bsagelib_types.h"
#endif

BDBG_MODULE(drm_prdy);

#define DUMP_DATA_HEX(string,data,size) {        \
   char tmp[512]= "\0";                          \
   uint32_t i=0, l=strlen(string);               \
   sprintf(tmp,"%s",string);                     \
   while( i<size && l < 512) {                   \
    sprintf(tmp+l," %02x", data[i]); ++i; l+=3;} \
   printf(tmp); printf("\n");                    \
}

#define POLICY_POOL_SIZE (5)

// ~100 KB to start * 64 (2^6) ~= 6.4 MB, don't allocate more than ~6.4 MB
#define DRM_MAXIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE ( 64 * MINIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE )

#define DRM_LICENSE_STORAGE_FILE "sample.hds"
#define DRM_DEFAULT_REVOCATION_LIST_FILE "revpackage.xml"

typedef struct PRDY_APP_CONTEXT
{
    DRM_APP_CONTEXT     *pDrmAppCtx;          /* drm application context */
    DRM_VOID            *pOEMContext;         /* Oem Context */
    OEM_Settings         oemSettings;
    DRM_BYTE            *pbOpaqueBuffer;      /* Opaque buffer */
    DRM_DWORD            cbOpaqueBuffer;
    DRM_BYTE            *pbRevocationBuffer;
    DRM_Prdy_policy_t    protectionPolicy[POLICY_POOL_SIZE]; /* Some license have more than 1 policy to enforce before playing content. This array is
                                                                used to store them up when the bdrm policy callback fires. */
    uint32_t nbOfPolicyQueued;
} PRDY_APP_CONTEXT;

/*
typedef struct PRDY_DECRYPTOR_CONTEXT
{
    DRM_DECRYPT_CONTEXT *pDecryptor;

} PRDY_DECRYPTOR_CONTEXT;
*/

typedef struct DRM_PRDY_ND_TRANSMITTER_CONTEXT
{
    DRM_PRND_TRANSMITTER_CONTEXT  *pPrndTxContext;

} DRM_PRDY_ND_TRANSMITTER_CONTEXT;

typedef struct DRM_PRDY_ND_RECEIVER_CONTEXT
{
    DRM_PRND_RECEIVER_CONTEXT *pPrndRxContext;

} DRM_PRDY_ND_RECEIVER_CONTEXT;

typedef enum
{
    eDRM_OPAQUE_BUFFER_HANDLE_INTERNAL_TYPE_INVALID             = 0,
    eDRM_OPAQUE_BUFFER_HANDLE_INTERNAL_TYPE_CLEAR_DATA_SOURCE   = 1,    /* Input to encrypt (clear source about to be
                                                                           encrypted) */
    eDRM_OPAQUE_BUFFER_HANDLE_INTERNAL_TYPE_ENCRYPTED_DATA      = 2,    /* Output from encrypt (clear source after
                                                                           encryption -> encrypted) OR Input to decrypt
                                                                           (encrypted source about to be decrypted) */
    eDRM_OPAQUE_BUFFER_HANDLE_INTERNAL_TYPE_CLEAR_DATA_RENDER   = 3,    /* Output from decrypt (encrypted source after
                                                                           decryption -> clear) */
} DRM_OPAQUE_BUFFER_HANDLE_INTERNAL_TYPE;


typedef struct DRM_OPAQUE_BUFFER_HANDLE_INTERNAL
{
    DRM_OPAQUE_BUFFER_HANDLE_INTERNAL_TYPE   eType;
    DRM_BYTE                                *pbData;
    DRM_DWORD                                cbData;
} DRM_OPAQUE_BUFFER_HANDLE_INTERNAL;


static DRM_CONST_STRING   sDstrHDSPath = EMPTY_DRM_STRING;
static DRM_WCHAR          sRgwchHDSPath[ DRM_MAX_PATH ];

/***********************************************************************************
 * Internal function to convert PlayReady SDK result into DRM_Prdy_Error_e.
 *
 * **FixMe** doesn't yet cover the complete list of DRM_Prdy_Error_e errors.
 ***********************************************************************************/
static
DRM_Prdy_Error_e convertDrmResult(DRM_RESULT result)
{
    switch (result) {
        case DRM_SUCCESS:
            return DRM_Prdy_ok;
        case DRM_E_FAIL:
            return DRM_Prdy_fail;
        case DRM_E_BUFFERTOOSMALL:
            return DRM_Prdy_buffer_size;
        case DRM_E_INVALIDARG:
            return DRM_Prdy_invalid_parameter;
        default:
            return DRM_Prdy_fail;
    }
}


/***********************************************************************************
 * Internal/ Extended function to convert PlayReady SDK result into DRM_Prdy_Error_e.
 * convertDrmResult_Ex covers wider range of error codes.
 *
 * Currently used by only DRM_Prdy_Initialize_Ex.
 * Need to think about a new scheme for saving time.
 ***********************************************************************************/
static
DRM_Prdy_Error_e convertDrmResult_Ex(DRM_RESULT result)
{
    switch (result) {
        case DRM_SUCCESS:
            return DRM_Prdy_ok;
        case DRM_E_FAIL:
            return DRM_Prdy_fail;
        case DRM_E_BUFFERTOOSMALL:
            return DRM_Prdy_buffer_size;
        case DRM_E_INVALIDARG:
            return DRM_Prdy_invalid_parameter;
        case DRM_E_OUTOFMEMORY:
            return DRM_Prdy_outofmemory;
        case DRM_E_NOTIMPL:
            return DRM_Prdy_notimpl;
        case DRM_E_POINTER:
            return DRM_Prdy_pointer;
        case DRM_E_WIN32_FILE_NOT_FOUND:
            return DRM_Prdy_win32_file_not_found;
        case DRM_E_HANDLE:
            return DRM_Prdy_handle;
        case DRM_E_WIN32_NO_MORE_FILES:
            return DRM_Prdy_win32_no_more_files;
        case DRM_E_NOMORE:
            return DRM_Prdy_nomore;
        case DRM_E_ARITHMETIC_OVERFLOW:
            return DRM_Prdy_arithmetic_overflow;
        case DRM_E_NOT_FOUND:
            return DRM_Prdy_not_found;
        case DRM_E_INVALID_COMMAND_LINE:
            return DRM_Prdy_invalid_command_line;
        case DRM_E_FILENOTFOUND:
            return DRM_Prdy_filenotfound;
        case DRM_E_FILEOPEN:
            return DRM_Prdy_fileopen;
        case DRM_E_PARAMETERS_MISMATCHED:
            return DRM_Prdy_parameters_mismatched;
        case DRM_E_FAILED_TO_STORE_LICENSE:
            return DRM_Prdy_failed_to_store_license;
        case DRM_E_NOT_ALL_STORED:
            return DRM_Prdy_not_all_stored;
        case DRM_E_VERIFICATION_FAILURE:
            return DRM_Prdy_verification_failure;
        case DRM_E_RSA_SIGNATURE_ERROR:
            return DRM_Prdy_rsa_signature_error;
        case DRM_E_BAD_RSA_EXPONENT:
            return DRM_Prdy_bad_rsa_exponent;
        case DRM_E_P256_CONVERSION_FAILURE:
            return DRM_Prdy_p256_conversion_failure;
        case DRM_E_P256_PKCRYPTO_FAILURE:
            return DRM_Prdy_p256_pkcrypto_failure;
        case DRM_E_P256_PLAINTEXT_MAPPING_FAILURE:
            return DRM_Prdy_p256_plaintext_mapping_failure;
        case DRM_E_P256_INVALID_SIGNATURE:
            return DRM_Prdy_p256_invalid_signature;
        case DRM_E_P256_ECDSA_VERIFICATION_ERROR:
            return DRM_Prdy_p256_ecdsa_verification_error;
        case DRM_E_P256_ECDSA_SIGNING_ERROR:
            return DRM_Prdy_p256_ecdsa_signing_error;
        case DRM_E_P256_HMAC_KEYGEN_FAILURE:
            return DRM_Prdy_p256_hmac_keygen_failure;
        case DRM_E_CH_BASECODE:
            return DRM_Prdy_ch_basecode;
        case DRM_E_CH_VERSION_MISSING:
            return DRM_Prdy_ch_version_missing;
        case DRM_E_CH_KID_MISSING:
            return DRM_Prdy_ch_kid_missing;
        case DRM_E_CH_LAINFO_MISSING:
            return DRM_Prdy_ch_lainfo_missing;
        case DRM_E_CH_CHECKSUM_MISSING:
            return DRM_Prdy_ch_checksum_missing;
        case DRM_E_CH_ATTR_MISSING:
            return DRM_Prdy_ch_attr_missing;
        case DRM_E_CH_INVALID_HEADER:
            return DRM_Prdy_ch_invalid_header;
        case DRM_E_CH_INVALID_CHECKSUM:
            return DRM_Prdy_ch_invalid_checksum;
        case DRM_E_CH_UNABLE_TO_VERIFY:
            return DRM_Prdy_ch_unable_to_verify;
        case DRM_E_CH_UNSUPPORTED_VERSION:
            return DRM_Prdy_ch_unsupported_version;
        case DRM_E_CH_BAD_KEY:
            return DRM_Prdy_ch_bad_key;
        case DRM_E_CH_INCOMPATIBLE_HEADER_TYPE:
            return DRM_Prdy_ch_incompatible_header_type;
        case DRM_E_HEADER_ALREADY_SET:
            return DRM_Prdy_header_already_set;
        case DRM_E_LIC_BASECODE:
            return DRM_Prdy_lic_basecode;
        case DRM_E_LIC_PARAM_NOT_OPTIONAL:
            return DRM_Prdy_lic_param_not_optional;
        case DRM_E_LIC_INVALID_LICENSE:
            return DRM_Prdy_lic_invalid_license;
        case DRM_E_LIC_UNSUPPORTED_VALUE:
            return DRM_Prdy_lic_unsupported_value;
        case DRM_E_CPRMEXP_BASECODE:
            return DRM_Prdy_cprmexp_basecode;
        case DRM_E_CPRMEXP_NOERROR:
            return DRM_Prdy_cprmexp_noerror;
        case DRM_E_CPRMEXP_NO_OPERANDS_IN_EXPRESSION:
            return DRM_Prdy_cprmexp_no_operands_in_expression;
        case DRM_E_CPRMEXP_INVALID_TOKEN:
            return DRM_Prdy_cprmexp_invalid_token;
        case DRM_E_CPRMEXP_INVALID_CONSTANT:
            return DRM_Prdy_cprmexp_invalid_constant;
        case DRM_E_CPRMEXP_INVALID_VARIABLE:
            return DRM_Prdy_cprmexp_invalid_variable;
        case DRM_E_CPRMEXP_INVALID_FUNCTION:
            return DRM_Prdy_cprmexp_invalid_function;
        case DRM_E_CPRMEXP_INVALID_ARGUMENT:
            return DRM_Prdy_cprmexp_invalid_argument;
        case DRM_E_CPRMEXP_INVALID_CONTEXT:
            return DRM_Prdy_cprmexp_invalid_context;
        case DRM_E_CPRMEXP_MISSING_OPERAND:
            return DRM_Prdy_cprmexp_missing_operand;
        case DRM_E_CPRMEXP_OVERFLOW:
            return DRM_Prdy_cprmexp_overflow;
        case DRM_E_CPRMEXP_UNDERFLOW:
            return DRM_Prdy_cprmexp_underflow;
        case DRM_E_CPRMEXP_INCORRECT_NUM_ARGS:
            return DRM_Prdy_cprmexp_incorrect_num_args;
        case DRM_E_CPRMEXP_VARIABLE_EXPECTED:
            return DRM_Prdy_cprmexp_variable_expected;
        case DRM_E_CPRMEXP_RETRIEVAL_FAILURE:
            return DRM_Prdy_cprmexp_retrieval_failure;
        case DRM_E_CPRMEXP_UPDATE_FAILURE:
            return DRM_Prdy_cprmexp_update_failure;
        case DRM_E_CPRMEXP_STRING_UNTERMINATED:
            return DRM_Prdy_cprmexp_string_unterminated;
        case DRM_E_CPRMEXP_UPDATE_UNSUPPORTED:
            return DRM_Prdy_cprmexp_update_unsupported;
        case DRM_E_CPRMEXP_ISOLATED_OPERAND_OR_OPERATOR:
            return DRM_Prdy_cprmexp_isolated_operand_or_operator;
        case DRM_E_CPRMEXP_UNMATCHED:
            return DRM_Prdy_cprmexp_unmatched;
        case DRM_E_CPRMEXP_WRONG_TYPE_OPERAND:
            return DRM_Prdy_cprmexp_wrong_type_operand;
        case DRM_E_CPRMEXP_TOO_MANY_OPERANDS:
            return DRM_Prdy_cprmexp_too_many_operands;
        case DRM_E_CPRMEXP_UNKNOWN_PARSE_ERROR:
            return DRM_Prdy_cprmexp_unknown_parse_error;
        case DRM_E_CPRMEXP_UNSUPPORTED_FUNCTION:
            return DRM_Prdy_cprmexp_unsupported_function;
        case DRM_E_CPRMEXP_CLOCK_REQUIRED:
            return DRM_Prdy_cprmexp_clock_required;
        case DRM_E_LEGACY_BASECODE:
            return DRM_Prdy_legacy_basecode;
        case DRM_E_LIC_KEY_DECODE_FAILURE:
            return DRM_Prdy_lic_key_decode_failure;
        case DRM_E_LIC_SIGNATURE_FAILURE:
            return DRM_Prdy_lic_signature_failure;
        case DRM_E_KEY_MISMATCH:
            return DRM_Prdy_key_mismatch;
        case DRM_E_INVALID_SIGNATURE:
            return DRM_Prdy_invalid_signature;
        case DRM_E_SYNC_ENTRY_NOT_FOUND:
            return DRM_Prdy_sync_entry_not_found;
        case DRM_E_CIPHER_NOT_INITIALIZED:
            return DRM_Prdy_cipher_not_initialized;
        case DRM_E_DECRYPT_NOT_INITIALIZED:
            return DRM_Prdy_decrypt_not_initialized;
        case DRM_E_SECURESTORE_LOCK_NOT_OBTAINED:
            return DRM_Prdy_securestore_lock_not_obtained;
        case DRM_E_PKCRYPTO_FAILURE:
            return DRM_Prdy_pkcrypto_failure;
        case DRM_E_INVALID_DST_SLOT_SIZE:
            return DRM_Prdy_invalid_dst_slot_size;
        case DRM_E_UNSUPPORTED_VERSION:
            return DRM_Prdy_unsupported_version;
        case DRM_E_EXPIRED_CERT:
            return DRM_Prdy_expired_cert;
        case DRM_E_DRMUTIL_INVALID_CERT:
            return DRM_Prdy_drmutil_invalid_cert;
        case DRM_E_DEVICE_NOT_REGISTERED:
            return DRM_Prdy_device_not_registered;
        case DRM_E_TOO_MANY_INCLUSION_GUIDS:
            return DRM_Prdy_too_many_inclusion_guids;
        case DRM_E_REVOCATION_GUID_NOT_RECOGNIZED:
            return DRM_Prdy_revocation_guid_not_recognized;
        case DRM_E_LIC_CHAIN_TOO_DEEP:
            return DRM_Prdy_lic_chain_too_deep;
        case DRM_E_DEVICE_SECURITY_LEVEL_TOO_LOW:
            return DRM_Prdy_device_security_level_too_low;
        case DRM_E_DST_BLOCK_CACHE_CORRUPT:
            return DRM_Prdy_dst_block_cache_corrupt;
        case DRM_E_DST_BLOCK_CACHE_MISS:
            return DRM_Prdy_dst_block_cache_miss;
        case DRM_E_INVALID_METERRESPONSE_SIGNATURE:
            return DRM_Prdy_invalid_meterresponse_signature;
        case DRM_E_INVALID_LICENSE_REVOCATION_LIST_SIGNATURE:
            return DRM_Prdy_invalid_license_revocation_list_signature;
        case DRM_E_INVALID_METERCERT_SIGNATURE:
            return DRM_Prdy_invalid_metercert_signature;
        case DRM_E_METERSTORE_DATA_NOT_FOUND:
            return DRM_Prdy_meterstore_data_not_found;
        case DRM_E_INVALID_REVOCATION_LIST:
            return DRM_Prdy_invalid_revocation_list;
        case DRM_E_ENVELOPE_CORRUPT:
            return DRM_Prdy_envelope_corrupt;
        case DRM_E_ENVELOPE_FILE_NOT_COMPATIBLE:
            return DRM_Prdy_envelope_file_not_compatible;
        case DRM_E_EXTENDED_RESTRICTION_NOT_UNDERSTOOD:
            return DRM_Prdy_extended_restriction_not_understood;
        case DRM_E_INVALID_SLK:
            return DRM_Prdy_invalid_slk;
        case DRM_E_DEVCERT_MODEL_MISMATCH:
            return DRM_Prdy_devcert_model_mismatch;
        case DRM_E_OUTDATED_REVOCATION_LIST:
            return DRM_Prdy_outdated_revocation_list;
        case DRM_E_DEVICE_NOT_INITIALIZED:
            return DRM_Prdy_device_not_initialized;
        case DRM_E_DRM_NOT_INITIALIZED:
            return DRM_Prdy_drm_not_initialized;
        case DRM_E_INVALID_RIGHT:
            return DRM_Prdy_invalid_right;
        case DRM_E_INVALID_LICENSE:
            return DRM_Prdy_invalid_license;
        case DRM_E_CONDITION_NOT_SUPPORTED:
            return DRM_Prdy_condition_not_supported;
        case DRM_E_LICENSE_EXPIRED:
            return DRM_Prdy_license_expired;
        case DRM_E_RIGHTS_NOT_AVAILABLE:
            return DRM_Prdy_rights_not_available;
        case DRM_E_LICENSE_MISMATCH:
            return DRM_Prdy_license_mismatch;
        case DRM_E_WRONG_TOKEN_TYPE:
            return DRM_Prdy_wrong_token_type;
        case DRM_E_LICENSE_NOT_BOUND:
            return DRM_Prdy_license_not_bound;
        case DRM_E_HASH_MISMATCH:
            return DRM_Prdy_hash_mismatch;
        case DRM_E_LICENSESTORE_NOT_FOUND:
            return DRM_Prdy_licensestore_not_found;
        case DRM_E_LICENSE_NOT_FOUND:
            return DRM_Prdy_license_not_found;
        case DRM_E_LICENSE_VERSION_NOT_SUPPORTED:
            return DRM_Prdy_license_version_not_supported;
        case DRM_E_UNSUPPORTED_ALGORITHM:
            return DRM_Prdy_unsupported_algorithm;
        case DRM_E_INVALID_LICENSE_STORE:
            return DRM_Prdy_invalid_license_store;
        case DRM_E_FILE_READ_ERROR:
            return DRM_Prdy_file_read_error;
        case DRM_E_FILE_WRITE_ERROR:
            return DRM_Prdy_file_write_error;
        case DRM_E_DST_STORE_FULL:
            return DRM_Prdy_dst_store_full;
        case DRM_E_NO_XML_OPEN_TAG:
            return DRM_Prdy_no_xml_open_tag;
        case DRM_E_NO_XML_CLOSE_TAG:
            return DRM_Prdy_no_xml_close_tag;
        case DRM_E_INVALID_XML_TAG:
            return DRM_Prdy_invalid_xml_tag;
        case DRM_E_NO_XML_CDATA:
            return DRM_Prdy_no_xml_cdata;
        case DRM_E_DST_NAMESPACE_NOT_FOUND:
            return DRM_Prdy_dst_namespace_not_found;
        case DRM_E_DST_SLOT_NOT_FOUND:
            return DRM_Prdy_dst_slot_not_found;
        case DRM_E_DST_SLOT_EXISTS:
            return DRM_Prdy_dst_slot_exists;
        case DRM_E_DST_CORRUPTED:
            return DRM_Prdy_dst_corrupted;
        case DRM_E_DST_SEEK_ERROR:
            return DRM_Prdy_dst_seek_error;
        case DRM_E_INVALID_SECURESTORE_PASSWORD:
            return DRM_Prdy_invalid_securestore_password;
        case DRM_E_SECURESTORE_CORRUPT:
            return DRM_Prdy_securestore_corrupt;
        case DRM_E_SECURESTORE_FULL:
            return DRM_Prdy_securestore_full;
        case DRM_E_DUPLICATED_HEADER_ATTRIBUTE:
            return DRM_Prdy_duplicated_header_attribute;
        case DRM_E_NO_KID_IN_HEADER:
            return DRM_Prdy_no_kid_in_header;
        case DRM_E_NO_LAINFO_IN_HEADER:
            return DRM_Prdy_no_lainfo_in_header;
        case DRM_E_NO_CHECKSUM_IN_HEADER:
            return DRM_Prdy_no_checksum_in_header;
        case DRM_E_DST_BLOCK_MISMATCH:
            return DRM_Prdy_dst_block_mismatch;
        case DRM_E_LICENSE_TOO_LONG:
            return DRM_Prdy_license_too_long;
        case DRM_E_DST_EXISTS:
            return DRM_Prdy_dst_exists;
        case DRM_E_INVALID_DEVICE_CERTIFICATE:
            return DRM_Prdy_invalid_device_certificate;
        case DRM_E_DST_LOCK_FAILED:
            return DRM_Prdy_dst_lock_failed;
        case DRM_E_FILE_SEEK_ERROR:
            return DRM_Prdy_file_seek_error;
        case DRM_E_DST_NOT_LOCKED_EXCLUSIVE:
            return DRM_Prdy_dst_not_locked_exclusive;
        case DRM_E_DST_EXCLUSIVE_LOCK_ONLY:
            return DRM_Prdy_dst_exclusive_lock_only;
        case DRM_E_V1_NOT_SUPPORTED:
            return DRM_Prdy_v1_not_supported;
        case DRM_E_HEADER_NOT_SET:
            return DRM_Prdy_header_not_set;
        case DRM_E_NEED_DEVCERT_INDIV:
            return DRM_Prdy_need_devcert_indiv;
        case DRM_E_MACHINE_ID_MISMATCH:
            return DRM_Prdy_machine_id_mismatch;
        case DRM_E_CLK_INVALID_RESPONSE:
            return DRM_Prdy_clk_invalid_response;
        case DRM_E_CLK_INVALID_DATE:
            return DRM_Prdy_clk_invalid_date;
        case DRM_E_INVALID_DEVCERT_TEMPLATE:
            return DRM_Prdy_invalid_devcert_template;
        case DRM_E_DEVCERT_EXCEEDS_SIZE_LIMIT:
            return DRM_Prdy_devcert_exceeds_size_limit;
        case DRM_E_DEVCERT_READ_ERROR:
            return DRM_Prdy_devcert_read_error;
        case DRM_E_PRIVKEY_READ_ERROR:
            return DRM_Prdy_privkey_read_error;
        case DRM_E_DEVCERT_TEMPLATE_READ_ERROR:
            return DRM_Prdy_devcert_template_read_error;
        case DRM_E_CLK_NOT_SUPPORTED:
            return DRM_Prdy_clk_not_supported;
        case DRM_E_METERING_NOT_SUPPORTED:
            return DRM_Prdy_metering_not_supported;
        case DRM_E_CLK_RESET_STATE_READ_ERROR:
            return DRM_Prdy_clk_reset_state_read_error;
        case DRM_E_CLK_RESET_STATE_WRITE_ERROR:
            return DRM_Prdy_clk_reset_state_write_error;
        case DRM_E_XMLNOTFOUND:
            return DRM_Prdy_xmlnotfound;
        case DRM_E_METERING_WRONG_TID:
            return DRM_Prdy_metering_wrong_tid;
        case DRM_E_METERING_INVALID_COMMAND:
            return DRM_Prdy_metering_invalid_command;
        case DRM_E_METERING_STORE_CORRUPT:
            return DRM_Prdy_metering_store_corrupt;
        case DRM_E_CERTIFICATE_REVOKED:
            return DRM_Prdy_certificate_revoked;
        case DRM_E_CRYPTO_FAILED:
            return DRM_Prdy_crypto_failed;
        case DRM_E_STACK_CORRUPT:
            return DRM_Prdy_stack_corrupt;
        case DRM_E_UNKNOWN_BINDING_KEY:
            return DRM_Prdy_unknown_binding_key;
        case DRM_E_V1_LICENSE_CHAIN_NOT_SUPPORTED:
            return DRM_Prdy_v1_license_chain_not_supported;
        case DRM_E_POLICY_METERING_DISABLED:
            return DRM_Prdy_policy_metering_disabled;
        case DRM_E_CLK_NOT_SET:
            return DRM_Prdy_clk_not_set;
        case DRM_E_NO_CLK_SUPPORTED:
            return DRM_Prdy_no_clk_supported;
        case DRM_E_NO_URL:
            return DRM_Prdy_no_url;
        case DRM_E_UNKNOWN_DEVICE_PROPERTY:
            return DRM_Prdy_unknown_device_property;
        case DRM_E_METERING_MID_MISMATCH:
            return DRM_Prdy_metering_mid_mismatch;
        case DRM_E_METERING_RESPONSE_DECRYPT_FAILED:
            return DRM_Prdy_metering_response_decrypt_failed;
        case DRM_E_RIV_TOO_SMALL:
            return DRM_Prdy_riv_too_small;
        case DRM_E_STACK_ALREADY_INITIALIZED:
            return DRM_Prdy_stack_already_initialized;
        case DRM_E_DEVCERT_REVOKED:
            return DRM_Prdy_devcert_revoked;
        case DRM_E_OEM_RSA_DECRYPTION_ERROR:
            return DRM_Prdy_oem_rsa_decryption_error;
        case DRM_E_INVALID_DEVSTORE_ATTRIBUTE:
            return DRM_Prdy_invalid_devstore_attribute;
        case DRM_E_INVALID_DEVSTORE_ENTRY:
            return DRM_Prdy_invalid_devstore_entry;
        case DRM_E_OEM_RSA_ENCRYPTION_ERROR:
            return DRM_Prdy_oem_rsa_encryption_error;
        case DRM_E_DST_NAMESPACE_EXISTS:
            return DRM_Prdy_dst_namespace_exists;
        case DRM_E_PERF_SCOPING_ERROR:
            return DRM_Prdy_perf_scoping_error;
        case DRM_E_OEM_RSA_INVALID_PRIVATE_KEY:
            return DRM_Prdy_oem_rsa_invalid_private_key;
        case DRM_E_NO_OPL_CALLBACK:
            return DRM_Prdy_no_opl_callback;
        case DRM_E_INVALID_PLAYREADY_OBJECT:
            return DRM_Prdy_invalid_playready_object;
        case DRM_E_DUPLICATE_LICENSE:
            return DRM_Prdy_duplicate_license;
        case DRM_E_RECORD_NOT_FOUND:
            return DRM_Prdy_record_not_found;
        case DRM_E_BUFFER_BOUNDS_EXCEEDED:
            return DRM_Prdy_buffer_bounds_exceeded;
        case DRM_E_INVALID_BASE64:
            return DRM_Prdy_invalid_base64;
        case DRM_E_PROTOCOL_VERSION_NOT_SUPPORTED:
            return DRM_Prdy_protocol_version_not_supported;
        case DRM_E_INVALID_LICENSE_RESPONSE_SIGNATURE:
            return DRM_Prdy_invalid_license_response_signature;
        case DRM_E_INVALID_LICENSE_RESPONSE_ID:
            return DRM_Prdy_invalid_license_response_id;
        case DRM_E_LICENSE_RESPONSE_SIGNATURE_MISSING:
            return DRM_Prdy_license_response_signature_missing;
        case DRM_E_INVALID_DOMAIN_JOIN_RESPONSE_SIGNATURE:
            return DRM_Prdy_invalid_domain_join_response_signature;
        case DRM_E_DOMAIN_JOIN_RESPONSE_SIGNATURE_MISSING:
            return DRM_Prdy_domain_join_response_signature_missing;
        case DRM_E_ACTIVATION_REQUIRED:
            return DRM_Prdy_activation_required;
        case DRM_E_ACTIVATION_INTERNAL_ERROR:
            return DRM_Prdy_activation_internal_error;
        case DRM_E_ACTIVATION_GROUP_CERT_REVOKED_ERROR:
            return DRM_Prdy_activation_group_cert_revoked_error;
        case DRM_E_ACTIVATION_NEW_CLIENT_LIB_REQUIRED_ERROR:
            return DRM_Prdy_activation_new_client_lib_required_error;
        case DRM_E_ACTIVATION_BAD_REQUEST:
            return DRM_Prdy_activation_bad_request;
        case DRM_E_FILEIO_ERROR:
            return DRM_Prdy_fileio_error;
        case DRM_E_DISK_SPACE_ERROR:
            return DRM_Prdy_disk_space_error;
        case DRM_E_UPLINK_LICENSE_NOT_FOUND:
            return DRM_Prdy_uplink_license_not_found;
        case DRM_E_ACTIVATION_CLIENT_ALREADY_CURRENT:
            return DRM_Prdy_activation_client_already_current;
        case DRM_E_LICENSE_REALTIME_EXPIRED:
            return DRM_Prdy_license_realtime_expired;
        case DRM_E_LICEVAL_BASECODE:
            return DRM_Prdy_liceval_basecode;
        case DRM_E_LICEVAL_LICENSE_NOT_SUPPLIED:
            return DRM_Prdy_liceval_license_not_supplied;
        case DRM_E_LICEVAL_KID_MISMATCH:
            return DRM_Prdy_liceval_kid_mismatch;
        case DRM_E_LICEVAL_LICENSE_REVOKED:
            return DRM_Prdy_liceval_license_revoked;
        case DRM_E_LICEVAL_UPDATE_FAILURE:
            return DRM_Prdy_liceval_update_failure;
        case DRM_E_LICEVAL_REQUIRED_REVOCATION_LIST_NOT_AVAILABLE:
            return DRM_Prdy_liceval_required_revocation_list_not_available;
        case DRM_E_LICEVAL_INVALID_PRND_LICENSE:
            return DRM_Prdy_liceval_invalid_prnd_license;
        case DRM_E_XMR_BASECODE:
            return DRM_Prdy_xmr_basecode;
        case DRM_E_XMR_OBJECT_ALREADY_EXISTS:
            return DRM_Prdy_xmr_object_already_exists;
        case DRM_E_XMR_OBJECT_NOT_FOUND:
            return DRM_Prdy_xmr_object_not_found;
        case DRM_E_XMR_REQUIRED_OBJECT_MISSING:
            return DRM_Prdy_xmr_required_object_missing;
        case DRM_E_XMR_INVALID_UNKNOWN_OBJECT:
            return DRM_Prdy_xmr_invalid_unknown_object;
        case DRM_E_XMR_LICENSE_BINDABLE:
            return DRM_Prdy_xmr_license_bindable;
        case DRM_E_XMR_LICENSE_NOT_BINDABLE:
            return DRM_Prdy_xmr_license_not_bindable;
        case DRM_E_XMR_UNSUPPORTED_XMR_VERSION:
            return DRM_Prdy_xmr_unsupported_xmr_version;
        case DRM_E_CERT_BASECODE:
            return DRM_Prdy_cert_basecode;
        case DRM_E_INVALID_DEVCERT_ATTRIBUTE:
            return DRM_Prdy_invalid_devcert_attribute;
        case DRM_E_TEST_BASECODE:
            return DRM_Prdy_test_basecode;
        case DRM_E_TEST_PKCRYPTO_FAILURE:
            return DRM_Prdy_test_pkcrypto_failure;
        case DRM_E_TEST_PKSIGN_VERIFY_ERROR:
            return DRM_Prdy_test_pksign_verify_error;
        case DRM_E_TEST_ENCRYPT_ERROR:
            return DRM_Prdy_test_encrypt_error;
        case DRM_E_TEST_RC4KEY_FAILED:
            return DRM_Prdy_test_rc4key_failed;
        case DRM_E_TEST_DECRYPT_ERROR:
            return DRM_Prdy_test_decrypt_error;
        case DRM_E_TEST_DESKEY_FAILED:
            return DRM_Prdy_test_deskey_failed;
        case DRM_E_TEST_CBC_INVERSEMAC_FAILURE:
            return DRM_Prdy_test_cbc_inversemac_failure;
        case DRM_E_TEST_HMAC_FAILURE:
            return DRM_Prdy_test_hmac_failure;
        case DRM_E_TEST_INVALIDARG:
            return DRM_Prdy_test_invalidarg;
        case DRM_E_TEST_DEVICE_PRIVATE_KEY_INCORRECTLY_STORED:
            return DRM_Prdy_test_device_private_key_incorrectly_stored;
        case DRM_E_TEST_DRMMANAGER_CONTEXT_NULL:
            return DRM_Prdy_test_drmmanager_context_null;
        case DRM_E_TEST_UNEXPECTED_REVINFO_RESULT:
            return DRM_Prdy_test_unexpected_revinfo_result;
        case DRM_E_TEST_RIV_MISMATCH:
            return DRM_Prdy_test_riv_mismatch;
        case DRM_E_TEST_URL_ERROR:
            return DRM_Prdy_test_url_error;
        case DRM_E_TEST_MID_MISMATCH:
            return DRM_Prdy_test_mid_mismatch;
        case DRM_E_TEST_METER_CERTIFICATE_MISMATCH:
            return DRM_Prdy_test_meter_certificate_mismatch;
        case DRM_E_TEST_LICENSE_STATE_MISMATCH:
            return DRM_Prdy_test_license_state_mismatch;
        case DRM_E_TEST_SOURCE_ID_MISMATCH:
            return DRM_Prdy_test_source_id_mismatch;
        case DRM_E_TEST_UNEXPECTED_LICENSE_COUNT:
            return DRM_Prdy_test_unexpected_license_count;
        case DRM_E_TEST_UNEXPECTED_DEVICE_PROPERTY:
            return DRM_Prdy_test_unexpected_device_property;
        case DRM_E_TEST_DRMMANAGER_MISALIGNED_BYTES:
            return DRM_Prdy_test_drmmanager_misaligned_bytes;
        case DRM_E_TEST_LICENSE_RESPONSE_ERROR:
            return DRM_Prdy_test_license_response_error;
        case DRM_E_TEST_OPL_MISMATCH:
            return DRM_Prdy_test_opl_mismatch;
        case DRM_E_TEST_INVALID_OPL_CALLBACK:
            return DRM_Prdy_test_invalid_opl_callback;
        case DRM_E_TEST_INCOMPLETE:
            return DRM_Prdy_test_incomplete;
        case DRM_E_TEST_UNEXPECTED_OUTPUT:
            return DRM_Prdy_test_unexpected_output;
        case DRM_E_TEST_DLA_NO_CONTENT_HEADER:
            return DRM_Prdy_test_dla_no_content_header;
        case DRM_E_TEST_DLA_CONTENT_HEADER_FOUND:
            return DRM_Prdy_test_dla_content_header_found;
        case DRM_E_TEST_SYNC_LSD_INCORRECT:
            return DRM_Prdy_test_sync_lsd_incorrect;
        case DRM_E_TEST_TOO_SLOW:
            return DRM_Prdy_test_too_slow;
        case DRM_E_TEST_LICENSESTORE_NOT_OPEN:
            return DRM_Prdy_test_licensestore_not_open;
        case DRM_E_TEST_DEVICE_NOT_INITED:
            return DRM_Prdy_test_device_not_inited;
        case DRM_E_TEST_VARIABLE_NOT_SET:
            return DRM_Prdy_test_variable_not_set;
        case DRM_E_TEST_NOMORE:
            return DRM_Prdy_test_nomore;
        case DRM_E_TEST_FILE_LOAD_ERROR:
            return DRM_Prdy_test_file_load_error;
        case DRM_E_TEST_LICENSE_ACQ_FAILED:
            return DRM_Prdy_test_license_acq_failed;
        case DRM_E_TEST_UNSUPPORTED_FILE_FORMAT:
            return DRM_Prdy_test_unsupported_file_format;
        case DRM_E_TEST_PARSING_ERROR:
            return DRM_Prdy_test_parsing_error;
        case DRM_E_TEST_NOTIMPL:
            return DRM_Prdy_test_notimpl;
        case DRM_E_TEST_VARIABLE_NOTFOUND:
            return DRM_Prdy_test_variable_notfound;
        case DRM_E_TEST_VARIABLE_LISTFULL:
            return DRM_Prdy_test_variable_listfull;
        case DRM_E_TEST_UNEXPECTED_CONTENT_PROPERTY:
            return DRM_Prdy_test_unexpected_content_property;
        case DRM_E_TEST_PRO_HEADER_NOT_SET:
            return DRM_Prdy_test_pro_header_not_set;
        case DRM_E_TEST_NON_PRO_HEADER_TYPE:
            return DRM_Prdy_test_non_pro_header_type;
        case DRM_E_TEST_INVALID_DEVICE_WRAPPER:
            return DRM_Prdy_test_invalid_device_wrapper;
        case DRM_E_TEST_INVALID_WMDM_WRAPPER:
            return DRM_Prdy_test_invalid_wmdm_wrapper;
        case DRM_E_TEST_INVALID_WPD_WRAPPER:
            return DRM_Prdy_test_invalid_wpd_wrapper;
        case DRM_E_TEST_INVALID_FILE:
            return DRM_Prdy_test_invalid_file;
        case DRM_E_TEST_PROPERTY_NOT_FOUND:
            return DRM_Prdy_test_property_not_found;
        case DRM_E_TEST_METERING_DATA_INCORRECT:
            return DRM_Prdy_test_metering_data_incorrect;
        case DRM_E_TEST_FILE_ALREADY_OPEN:
            return DRM_Prdy_test_file_already_open;
        case DRM_E_TEST_FILE_NOT_OPEN:
            return DRM_Prdy_test_file_not_open;
        case DRM_E_TEST_PICT_COLUMN_TOO_WIDE:
            return DRM_Prdy_test_pict_column_too_wide;
        case DRM_E_TEST_PICT_COLUMN_MISMATCH:
            return DRM_Prdy_test_pict_column_mismatch;
        case DRM_E_TEST_TUX_TEST_SKIPPED:
            return DRM_Prdy_test_tux_test_skipped;
        case DRM_E_TEST_KEYFILE_VERIFICATION_FAILURE:
            return DRM_Prdy_test_keyfile_verification_failure;
        case DRM_E_TEST_DATA_VERIFICATION_FAILURE:
            return DRM_Prdy_test_data_verification_failure;
        case DRM_E_TEST_NET_FAIL:
            return DRM_Prdy_test_net_fail;
        case DRM_E_TEST_CLEANUP_FAIL:
            return DRM_Prdy_test_cleanup_fail;
        case DRM_E_LOGICERR:
            return DRM_Prdy_logicerr;
        case DRM_E_INVALID_REV_INFO:
            return DRM_Prdy_invalid_rev_info;
        case DRM_E_SYNCLIST_NOT_SUPPORTED:
            return DRM_Prdy_synclist_not_supported;
        case DRM_E_REVOCATION_BUFFER_TOO_SMALL:
            return DRM_Prdy_revocation_buffer_too_small;
        case DRM_E_DEVICE_ALREADY_REGISTERED:
            return DRM_Prdy_device_already_registered;
        case DRM_E_DST_NOT_COMPATIBLE:
            return DRM_Prdy_dst_not_compatible;
        case DRM_E_RSA_DECRYPTION_ERROR:
            return DRM_Prdy_rsa_decryption_error;
        case DRM_E_OEM_RSA_MESSAGE_TOO_BIG:
            return DRM_Prdy_oem_rsa_message_too_big;
        case DRM_E_METERCERT_NOT_FOUND:
            return DRM_Prdy_metercert_not_found;
        case DRM_E_MODULAR_ARITHMETIC_FAILURE:
            return DRM_Prdy_modular_arithmetic_failure;
        case DRM_E_REVOCATION_INVALID_PACKAGE:
            return DRM_Prdy_revocation_invalid_package;
        case DRM_E_HWID_ERROR:
            return DRM_Prdy_hwid_error;
        case DRM_E_DOMAIN_BASECODE:
            return DRM_Prdy_domain_basecode;
        case DRM_E_DOMAIN_INVALID_GUID:
            return DRM_Prdy_domain_invalid_guid;
        case DRM_E_DOMAIN_INVALID_CUSTOM_DATA_TYPE:
            return DRM_Prdy_domain_invalid_custom_data_type;
        case DRM_E_DOMAIN_STORE_ADD_DATA:
            return DRM_Prdy_domain_store_add_data;
        case DRM_E_DOMAIN_STORE_GET_DATA:
            return DRM_Prdy_domain_store_get_data;
        case DRM_E_DOMAIN_STORE_DELETE_DATA:
            return DRM_Prdy_domain_store_delete_data;
        case DRM_E_DOMAIN_STORE_OPEN_STORE:
            return DRM_Prdy_domain_store_open_store;
        case DRM_E_DOMAIN_STORE_CLOSE_STORE:
            return DRM_Prdy_domain_store_close_store;
        case DRM_E_DOMAIN_BIND_LICENSE:
            return DRM_Prdy_domain_bind_license;
        case DRM_E_DOMAIN_INVALID_CUSTOM_DATA:
            return DRM_Prdy_domain_invalid_custom_data;
        case DRM_E_DOMAIN_NOT_FOUND:
            return DRM_Prdy_domain_not_found;
        case DRM_E_DOMAIN_INVALID_DOMKEYXMR_DATA:
            return DRM_Prdy_domain_invalid_domkeyxmr_data;
        case DRM_E_DOMAIN_STORE_INVALID_KEY_RECORD:
            return DRM_Prdy_domain_store_invalid_key_record;
        case DRM_E_PC_BASECODE:
            return DRM_Prdy_pc_basecode;
        case DRM_E_DEVICE_DOMAIN_JOIN_REQUIRED:
            return DRM_Prdy_device_domain_join_required;
        case DRM_E_SERVER_BASECODE:
            return DRM_Prdy_server_basecode;
        case DRM_E_SERVER_INTERNAL_ERROR:
            return DRM_Prdy_server_internal_error;
        case DRM_E_SERVER_INVALID_MESSAGE:
            return DRM_Prdy_server_invalid_message;
        case DRM_E_SERVER_DEVICE_LIMIT_REACHED:
            return DRM_Prdy_server_device_limit_reached;
        case DRM_E_SERVER_INDIV_REQUIRED:
            return DRM_Prdy_server_indiv_required;
        case DRM_E_SERVER_SERVICE_SPECIFIC:
            return DRM_Prdy_server_service_specific;
        case DRM_E_SERVER_DOMAIN_REQUIRED:
            return DRM_Prdy_server_domain_required;
        case DRM_E_SERVER_RENEW_DOMAIN:
            return DRM_Prdy_server_renew_domain;
        case DRM_E_SERVER_UNKNOWN_METERINGID:
            return DRM_Prdy_server_unknown_meteringid;
        case DRM_E_SERVER_COMPUTER_LIMIT_REACHED:
            return DRM_Prdy_server_computer_limit_reached;
        case DRM_E_SERVER_PROTOCOL_FALLBACK:
            return DRM_Prdy_server_protocol_fallback;
        case DRM_E_SERVER_NOT_A_MEMBER:
            return DRM_Prdy_server_not_a_member;
        case DRM_E_SERVER_PROTOCOL_VERSION_MISMATCH:
            return DRM_Prdy_server_protocol_version_mismatch;
        case DRM_E_SERVER_UNKNOWN_ACCOUNTID:
            return DRM_Prdy_server_unknown_accountid;
        case DRM_E_SERVER_PROTOCOL_REDIRECT:
            return DRM_Prdy_server_protocol_redirect;
        case DRM_E_SERVER_UNKNOWN_TRANSACTIONID:
            return DRM_Prdy_server_unknown_transactionid;
        case DRM_E_SERVER_INVALID_LICENSEID:
            return DRM_Prdy_server_invalid_licenseid;
        case DRM_E_SERVER_MAXIMUM_LICENSEID_EXCEEDED:
            return DRM_Prdy_server_maximum_licenseid_exceeded;
        case DRM_E_SERVICES_BASECODE:
            return DRM_Prdy_services_basecode;
        case DRM_E_LICACQ_BASECODE:
            return DRM_Prdy_licacq_basecode;
        case DRM_E_LICACQ_TOO_MANY_LICENSES:
            return DRM_Prdy_licacq_too_many_licenses;
        case DRM_E_LICACQ_ACK_TRANSACTION_ID_TOO_BIG:
            return DRM_Prdy_licacq_ack_transaction_id_too_big;
        case DRM_E_LICACQ_ACK_MESSAGE_NOT_CREATED:
            return DRM_Prdy_licacq_ack_message_not_created;
        case DRM_E_INITIATORS_BASECODE:
            return DRM_Prdy_initiators_basecode;
        case DRM_E_INITIATORS_UNKNOWN_TYPE:
            return DRM_Prdy_initiators_unknown_type;
        case DRM_E_INITIATORS_INVALID_SERVICEID:
            return DRM_Prdy_initiators_invalid_serviceid;
        case DRM_E_INITIATORS_INVALID_ACCOUNTID:
            return DRM_Prdy_initiators_invalid_accountid;
        case DRM_E_INITIATORS_INVALID_MID:
            return DRM_Prdy_initiators_invalid_mid;
        case DRM_E_INITIATORS_MISSING_DC_URL:
            return DRM_Prdy_initiators_missing_dc_url;
        case DRM_E_INITIATORS_MISSING_CONTENT_HEADER:
            return DRM_Prdy_initiators_missing_content_header;
        case DRM_E_INITIATORS_MISSING_LAURL_IN_CONTENT_HEADER:
            return DRM_Prdy_initiators_missing_laurl_in_content_header;
        case DRM_E_INITIATORS_MISSING_METERCERT_URL:
            return DRM_Prdy_initiators_missing_metercert_url;
        case DRM_E_BCERT_BASECODE:
            return DRM_Prdy_bcert_basecode;
        case DRM_E_BCERT_INVALID_SIGNATURE_TYPE:
            return DRM_Prdy_bcert_invalid_signature_type;
        case DRM_E_BCERT_CHAIN_TOO_DEEP:
            return DRM_Prdy_bcert_chain_too_deep;
        case DRM_E_BCERT_INVALID_CERT_TYPE:
            return DRM_Prdy_bcert_invalid_cert_type;
        case DRM_E_BCERT_INVALID_FEATURE:
            return DRM_Prdy_bcert_invalid_feature;
        case DRM_E_BCERT_INVALID_KEY_USAGE:
            return DRM_Prdy_bcert_invalid_key_usage;
        case DRM_E_BCERT_INVALID_SECURITY_VERSION:
            return DRM_Prdy_bcert_invalid_security_version;
        case DRM_E_BCERT_INVALID_KEY_TYPE:
            return DRM_Prdy_bcert_invalid_key_type;
        case DRM_E_BCERT_INVALID_KEY_LENGTH:
            return DRM_Prdy_bcert_invalid_key_length;
        case DRM_E_BCERT_INVALID_MAX_LICENSE_SIZE:
            return DRM_Prdy_bcert_invalid_max_license_size;
        case DRM_E_BCERT_INVALID_MAX_HEADER_SIZE:
            return DRM_Prdy_bcert_invalid_max_header_size;
        case DRM_E_BCERT_INVALID_MAX_LICENSE_CHAIN_DEPTH:
            return DRM_Prdy_bcert_invalid_max_license_chain_depth;
        case DRM_E_BCERT_INVALID_SECURITY_LEVEL:
            return DRM_Prdy_bcert_invalid_security_level;
        case DRM_E_BCERT_PRIVATE_KEY_NOT_SPECIFIED:
            return DRM_Prdy_bcert_private_key_not_specified;
        case DRM_E_BCERT_ISSUER_KEY_NOT_SPECIFIED:
            return DRM_Prdy_bcert_issuer_key_not_specified;
        case DRM_E_BCERT_ACCOUNT_ID_NOT_SPECIFIED:
            return DRM_Prdy_bcert_account_id_not_specified;
        case DRM_E_BCERT_SERVICE_ID_NOT_SPECIFIED:
            return DRM_Prdy_bcert_service_id_not_specified;
        case DRM_E_BCERT_DOMAIN_URL_NOT_SPECIFIED:
            return DRM_Prdy_bcert_domain_url_not_specified;
        case DRM_E_BCERT_DOMAIN_URL_TOO_LONG:
            return DRM_Prdy_bcert_domain_url_too_long;
        case DRM_E_BCERT_HARDWARE_ID_TOO_LONG:
            return DRM_Prdy_bcert_hardware_id_too_long;
        case DRM_E_BCERT_CERT_ID_NOT_SPECIFIED:
            return DRM_Prdy_bcert_cert_id_not_specified;
        case DRM_E_BCERT_PUBLIC_KEY_NOT_SPECIFIED:
            return DRM_Prdy_bcert_public_key_not_specified;
        case DRM_E_BCERT_KEY_USAGES_NOT_SPECIFIED:
            return DRM_Prdy_bcert_key_usages_not_specified;
        case DRM_E_BCERT_STRING_NOT_NULL_TERMINATED:
            return DRM_Prdy_bcert_string_not_null_terminated;
        case DRM_E_BCERT_OBJECTHEADER_LEN_TOO_BIG:
            return DRM_Prdy_bcert_objectheader_len_too_big;
        case DRM_E_BCERT_INVALID_ISSUERKEY_LENGTH:
            return DRM_Prdy_bcert_invalid_issuerkey_length;
        case DRM_E_BCERT_BASICINFO_CERT_EXPIRED:
            return DRM_Prdy_bcert_basicinfo_cert_expired;
        case DRM_E_BCERT_UNEXPECTED_OBJECT_HEADER:
            return DRM_Prdy_bcert_unexpected_object_header;
        case DRM_E_BCERT_ISSUERKEY_KEYINFO_MISMATCH:
            return DRM_Prdy_bcert_issuerkey_keyinfo_mismatch;
        case DRM_E_BCERT_INVALID_MAX_KEY_USAGES:
            return DRM_Prdy_bcert_invalid_max_key_usages;
        case DRM_E_BCERT_INVALID_MAX_FEATURES:
            return DRM_Prdy_bcert_invalid_max_features;
        case DRM_E_BCERT_INVALID_CHAIN_HEADER_TAG:
            return DRM_Prdy_bcert_invalid_chain_header_tag;
        case DRM_E_BCERT_INVALID_CHAIN_VERSION:
            return DRM_Prdy_bcert_invalid_chain_version;
        case DRM_E_BCERT_INVALID_CHAIN_LENGTH:
            return DRM_Prdy_bcert_invalid_chain_length;
        case DRM_E_BCERT_INVALID_CERT_HEADER_TAG:
            return DRM_Prdy_bcert_invalid_cert_header_tag;
        case DRM_E_BCERT_INVALID_CERT_VERSION:
            return DRM_Prdy_bcert_invalid_cert_version;
        case DRM_E_BCERT_INVALID_CERT_LENGTH:
            return DRM_Prdy_bcert_invalid_cert_length;
        case DRM_E_BCERT_INVALID_SIGNEDCERT_LENGTH:
            return DRM_Prdy_bcert_invalid_signedcert_length;
        case DRM_E_BCERT_INVALID_PLATFORM_IDENTIFIER:
            return DRM_Prdy_bcert_invalid_platform_identifier;
        case DRM_E_BCERT_INVALID_NUMBER_EXTDATARECORDS:
            return DRM_Prdy_bcert_invalid_number_extdatarecords;
        case DRM_E_BCERT_INVALID_EXTDATARECORD:
            return DRM_Prdy_bcert_invalid_extdatarecord;
        case DRM_E_BCERT_EXTDATA_LENGTH_MUST_PRESENT:
            return DRM_Prdy_bcert_extdata_length_must_present;
        case DRM_E_BCERT_EXTDATA_PRIVKEY_MUST_PRESENT:
            return DRM_Prdy_bcert_extdata_privkey_must_present;
        case DRM_E_BCERT_INVALID_EXTDATA_LENGTH:
            return DRM_Prdy_bcert_invalid_extdata_length;
        case DRM_E_BCERT_EXTDATA_IS_NOT_PROVIDED:
            return DRM_Prdy_bcert_extdata_is_not_provided;
        case DRM_E_BCERT_HWIDINFO_IS_MISSING:
            return DRM_Prdy_bcert_hwidinfo_is_missing;
        case DRM_E_BCERT_EXTDATAFLAG_CERT_TYPE_MISMATCH:
            return DRM_Prdy_bcert_extdataflag_cert_type_mismatch;
        case DRM_E_BCERT_METERING_ID_NOT_SPECIFIED:
            return DRM_Prdy_bcert_metering_id_not_specified;
        case DRM_E_BCERT_METERING_URL_NOT_SPECIFIED:
            return DRM_Prdy_bcert_metering_url_not_specified;
        case DRM_E_BCERT_METERING_URL_TOO_LONG:
            return DRM_Prdy_bcert_metering_url_too_long;
        case DRM_E_BCERT_VERIFICATION_ERRORS:
            return DRM_Prdy_bcert_verification_errors;
        case DRM_E_BCERT_REQUIRED_KEYUSAGE_MISSING:
            return DRM_Prdy_bcert_required_keyusage_missing;
        case DRM_E_BCERT_NO_PUBKEY_WITH_REQUESTED_KEYUSAGE:
            return DRM_Prdy_bcert_no_pubkey_with_requested_keyusage;
        case DRM_E_BCERT_MANUFACTURER_STRING_TOO_LONG:
            return DRM_Prdy_bcert_manufacturer_string_too_long;
        case DRM_E_BCERT_TOO_MANY_PUBLIC_KEYS:
            return DRM_Prdy_bcert_too_many_public_keys;
        case DRM_E_BCERT_OBJECTHEADER_LEN_TOO_SMALL:
            return DRM_Prdy_bcert_objectheader_len_too_small;
        case DRM_E_BCERT_INVALID_WARNING_DAYS:
            return DRM_Prdy_bcert_invalid_warning_days;
        case DRM_E_XMLSIG_BASECODE:
            return DRM_Prdy_xmlsig_basecode;
        case DRM_E_XMLSIG_ECDSA_VERIFY_FAILURE:
            return DRM_Prdy_xmlsig_ecdsa_verify_failure;
        case DRM_E_XMLSIG_SHA_VERIFY_FAILURE:
            return DRM_Prdy_xmlsig_sha_verify_failure;
        case DRM_E_XMLSIG_FORMAT:
            return DRM_Prdy_xmlsig_format;
        case DRM_E_XMLSIG_PUBLIC_KEY_ID:
            return DRM_Prdy_xmlsig_public_key_id;
        case DRM_E_XMLSIG_INVALID_KEY_FORMAT:
            return DRM_Prdy_xmlsig_invalid_key_format;
        case DRM_E_XMLSIG_SHA_HASH_SIZE:
            return DRM_Prdy_xmlsig_sha_hash_size;
        case DRM_E_XMLSIG_ECDSA_SIGNATURE_SIZE:
            return DRM_Prdy_xmlsig_ecdsa_signature_size;
        case DRM_E_UTF_BASECODE:
            return DRM_Prdy_utf_basecode;
        case DRM_E_UTF_UNEXPECTED_END:
            return DRM_Prdy_utf_unexpected_end;
        case DRM_E_UTF_INVALID_CODE:
            return DRM_Prdy_utf_invalid_code;
        case DRM_E_SOAPXML_BASECODE:
            return DRM_Prdy_soapxml_basecode;
        case DRM_E_SOAPXML_INVALID_STATUS_CODE:
            return DRM_Prdy_soapxml_invalid_status_code;
        case DRM_E_SOAPXML_XML_FORMAT:
            return DRM_Prdy_soapxml_xml_format;
        case DRM_E_SOAPXML_WRONG_MESSAGE_TYPE:
            return DRM_Prdy_soapxml_wrong_message_type;
        case DRM_E_SOAPXML_SIGNATURE_MISSING:
            return DRM_Prdy_soapxml_signature_missing;
        case DRM_E_SOAPXML_PROTOCOL_NOT_SUPPORTED:
            return DRM_Prdy_soapxml_protocol_not_supported;
        case DRM_E_SOAPXML_DATA_NOT_FOUND:
            return DRM_Prdy_soapxml_data_not_found;
        case DRM_E_CRYPTO_BASECODE:
            return DRM_Prdy_crypto_basecode;
        case DRM_E_CRYPTO_PUBLIC_KEY_NOT_MATCH:
            return DRM_Prdy_crypto_public_key_not_match;
        case DRM_E_UNABLE_TO_RESOLVE_LOCATION_TREE:
            return DRM_Prdy_unable_to_resolve_location_tree;
        case DRM_E_ND_MUST_REVALIDATE:
            return DRM_Prdy_nd_must_revalidate;
        case DRM_E_ND_INVALID_MESSAGE:
            return DRM_Prdy_nd_invalid_message;
        case DRM_E_ND_INVALID_MESSAGE_TYPE:
            return DRM_Prdy_nd_invalid_message_type;
        case DRM_E_ND_INVALID_MESSAGE_VERSION:
            return DRM_Prdy_nd_invalid_message_version;
        case DRM_E_ND_INVALID_SESSION:
            return DRM_Prdy_nd_invalid_session;
        case DRM_E_ND_MEDIA_SESSION_LIMIT_REACHED:
            return DRM_Prdy_nd_media_session_limit_reached;
        case DRM_E_ND_UNABLE_TO_VERIFY_PROXIMITY:
            return DRM_Prdy_nd_unable_to_verify_proximity;
        case DRM_E_ND_INVALID_PROXIMITY_RESPONSE:
            return DRM_Prdy_nd_invalid_proximity_response;
        case DRM_E_ND_DEVICE_LIMIT_REACHED:
            return DRM_Prdy_nd_device_limit_reached;
        case DRM_E_ND_BAD_REQUEST:
            return DRM_Prdy_nd_bad_request;
        case DRM_E_ND_FAILED_SEEK:
            return DRM_Prdy_nd_failed_seek;
        case DRM_E_ND_INVALID_CONTEXT:
            return DRM_Prdy_nd_invalid_context;
        case DRM_E_ASF_BAD_ASF_HEADER:
            return DRM_Prdy_asf_bad_asf_header;
        case DRM_E_ASF_BAD_PACKET_HEADER:
            return DRM_Prdy_asf_bad_packet_header;
        case DRM_E_ASF_BAD_PAYLOAD_HEADER:
            return DRM_Prdy_asf_bad_payload_header;
        case DRM_E_ASF_BAD_DATA_HEADER:
            return DRM_Prdy_asf_bad_data_header;
        case DRM_E_ASF_INVALID_OPERATION:
            return DRM_Prdy_asf_invalid_operation;
        case DRM_E_ASF_EXTENDED_STREAM_PROPERTIES_OBJ_NOT_FOUND:
            return DRM_Prdy_asf_extended_stream_properties_obj_not_found;
        case DRM_E_ASF_INVALID_DATA:
            return DRM_Prdy_asf_invalid_data;
        case DRM_E_ASF_TOO_MANY_PAYLOADS:
            return DRM_Prdy_asf_too_many_payloads;
        case DRM_E_ASF_BANDWIDTH_OVERRUN:
            return DRM_Prdy_asf_bandwidth_overrun;
        case DRM_E_ASF_INVALID_STREAM_NUMBER:
            return DRM_Prdy_asf_invalid_stream_number;
        case DRM_E_ASF_LATE_SAMPLE:
            return DRM_Prdy_asf_late_sample;
        case DRM_E_ASF_NOT_ACCEPTING:
            return DRM_Prdy_asf_not_accepting;
        case DRM_E_ASF_UNEXPECTED:
            return DRM_Prdy_asf_unexpected;
        case DRM_E_NONCE_STORE_BASECODE:
            return DRM_Prdy_nonce_store_basecode;
        case DRM_E_NONCE_STORE_TOKEN_NOT_FOUND:
            return DRM_Prdy_nonce_store_token_not_found;
        case DRM_E_NONCE_STORE_OPEN_STORE:
            return DRM_Prdy_nonce_store_open_store;
        case DRM_E_NONCE_STORE_CLOSE_STORE:
            return DRM_Prdy_nonce_store_close_store;
        case DRM_E_NONCE_STORE_ADD_LICENSE:
            return DRM_Prdy_nonce_store_add_license;
        case DRM_E_POLICYSTATE_BASECODE:
            return DRM_Prdy_policystate_basecode;
        case DRM_E_POLICYSTATE_NOT_FOUND:
            return DRM_Prdy_policystate_not_found;
        case DRM_E_POLICYSTATE_CORRUPTED:
            return DRM_Prdy_policystate_corrupted;
        case DRM_E_MOVE_BASECODE:
            return DRM_Prdy_move_basecode;
        case DRM_E_MOVE_DENIED:
            return DRM_Prdy_move_denied;
        case DRM_E_INVALID_MOVE_RESPONSE:
            return DRM_Prdy_invalid_move_response;
        case DRM_E_MOVE_NONCE_MISMATCH:
            return DRM_Prdy_move_nonce_mismatch;
        case DRM_E_MOVE_STORE_OPEN_STORE:
            return DRM_Prdy_move_store_open_store;
        case DRM_E_MOVE_STORE_CLOSE_STORE:
            return DRM_Prdy_move_store_close_store;
        case DRM_E_MOVE_STORE_ADD_DATA:
            return DRM_Prdy_move_store_add_data;
        case DRM_E_MOVE_STORE_GET_DATA:
            return DRM_Prdy_move_store_get_data;
        case DRM_E_MOVE_FORMAT_INVALID:
            return DRM_Prdy_move_format_invalid;
        case DRM_E_MOVE_SIGNATURE_INVALID:
            return DRM_Prdy_move_signature_invalid;
        case DRM_E_COPY_DENIED:
            return DRM_Prdy_copy_denied;
        case DRM_E_XB_BASECODE:
            return DRM_Prdy_xb_basecode;
        case DRM_E_XB_OBJECT_NOTFOUND:
            return DRM_Prdy_xb_object_notfound;
        case DRM_E_XB_INVALID_OBJECT:
            return DRM_Prdy_xb_invalid_object;
        case DRM_E_XB_OBJECT_ALREADY_EXISTS:
            return DRM_Prdy_xb_object_already_exists;
        case DRM_E_XB_REQUIRED_OBJECT_MISSING:
            return DRM_Prdy_xb_required_object_missing;
        case DRM_E_XB_UNKNOWN_ELEMENT_TYPE:
            return DRM_Prdy_xb_unknown_element_type;
        case DRM_E_KEYFILE_BASECODE:
            return DRM_Prdy_keyfile_basecode;
        case DRM_E_KEYFILE_INVALID_PLATFORM:
            return DRM_Prdy_keyfile_invalid_platform;
        case DRM_E_KEYFILE_TOO_LARGE:
            return DRM_Prdy_keyfile_too_large;
        case DRM_E_KEYFILE_PRIVATE_KEY_NOT_FOUND:
            return DRM_Prdy_keyfile_private_key_not_found;
        case DRM_E_KEYFILE_CERTIFICATE_CHAIN_NOT_FOUND:
            return DRM_Prdy_keyfile_certificate_chain_not_found;
        case DRM_E_KEYFILE_KEY_NOT_FOUND:
            return DRM_Prdy_keyfile_key_not_found;
        case DRM_E_KEYFILE_UNKNOWN_DECRYPTION_METHOD:
            return DRM_Prdy_keyfile_unknown_decryption_method;
        case DRM_E_KEYFILE_INVALID_SIGNATURE:
            return DRM_Prdy_keyfile_invalid_signature;
        case DRM_E_KEYFILE_INTERNAL_DECRYPTION_BUFFER_TOO_SMALL:
            return DRM_Prdy_keyfile_internal_decryption_buffer_too_small;
        case DRM_E_KEYFILE_PLATFORMID_MISMATCH:
            return DRM_Prdy_keyfile_platformid_mismatch;
        case DRM_E_KEYFILE_CERTIFICATE_ISSUER_KEY_MISMATCH:
            return DRM_Prdy_keyfile_certificate_issuer_key_mismatch;
        case DRM_E_KEYFILE_ROBUSTNESSVERSION_MISMATCH:
            return DRM_Prdy_keyfile_robustnessversion_mismatch;
        case DRM_E_KEYFILE_FILE_NOT_CLOSED:
            return DRM_Prdy_keyfile_file_not_closed;
        case DRM_E_KEYFILE_NOT_INITED:
            return DRM_Prdy_keyfile_not_inited;
        case DRM_E_KEYFILE_FORMAT_INVALID:
            return DRM_Prdy_keyfile_format_invalid;
        case DRM_E_KEYFILE_UPDATE_NOT_ALLOWED:
            return DRM_Prdy_keyfile_update_not_allowed;
        case DRM_E_SERVICES_BASECODE_EX:
            return DRM_Prdy_services_basecode_ex;
        case DRM_E_PRND_BASECODE:
            return DRM_Prdy_prnd_basecode;
        case DRM_E_PRND_MESSAGE_VERSION_INVALID:
            return DRM_Prdy_prnd_message_version_invalid;
        case DRM_E_PRND_MESSAGE_WRONG_TYPE:
            return DRM_Prdy_prnd_message_wrong_type;
        case DRM_E_PRND_MESSAGE_INVALID:
            return DRM_Prdy_prnd_message_invalid;
        case DRM_E_PRND_SESSION_ID_INVALID:
            return DRM_Prdy_prnd_session_id_invalid;
        case DRM_E_PRND_PROXIMITY_DETECTION_REQUEST_CHANNEL_TYPE_UNSUPPORTED:
            return DRM_Prdy_prnd_proximity_detection_request_channel_type_unsupported;
        case DRM_E_PRND_PROXIMITY_DETECTION_RESPONSE_INVALID:
            return DRM_Prdy_prnd_proximity_detection_response_invalid;
        case DRM_E_PRND_PROXIMITY_DETECTION_RESPONSE_TIMEOUT:
            return DRM_Prdy_prnd_proximity_detection_response_timeout;
        case DRM_E_PRND_LICENSE_REQUEST_CID_CALLBACK_REQUIRED:
            return DRM_Prdy_prnd_license_request_cid_callback_required;
        case DRM_E_PRND_LICENSE_RESPONSE_CLMID_INVALID:
            return DRM_Prdy_prnd_license_response_clmid_invalid;
        case DRM_E_PRND_CERTIFICATE_NOT_RECEIVER:
            return DRM_Prdy_prnd_certificate_not_receiver;
        case DRM_E_PRND_CANNOT_RENEW_USING_NEW_SESSION:
            return DRM_Prdy_prnd_cannot_renew_using_new_session;
        case DRM_E_PRND_INVALID_CUSTOM_DATA_TYPE:
            return DRM_Prdy_prnd_invalid_custom_data_type;
        case DRM_E_PRND_CLOCK_OUT_OF_SYNC:
            return DRM_Prdy_prnd_clock_out_of_sync;
        case DRM_E_PRND_CANNOT_REBIND_PRND_RECEIVED_LICENSE:
            return DRM_Prdy_prnd_cannot_rebind_prnd_received_license;
        case DRM_E_PRND_CANNOT_REGISTER_USING_EXISTING_SESSION:
            return DRM_Prdy_prnd_cannot_register_using_existing_session;
        case DRM_E_PRND_BUSY_PERFORMING_RENEWAL:
            return DRM_Prdy_prnd_busy_performing_renewal;
        case DRM_E_PRND_LICENSE_REQUEST_INVALID_ACTION:
            return DRM_Prdy_prnd_license_request_invalid_action;
        case DRM_E_PRND_TRANSMITTER_UNAUTHORIZED:
            return DRM_Prdy_prnd_transmitter_unauthorized;
        case DRM_E_OEMHAL_BASECODE:
            return DRM_Prdy_oemhal_basecode;
        case DRM_E_OEMHAL_NOT_INITIALIZED:
            return DRM_Prdy_oemhal_not_initialized;
        case DRM_E_OEMHAL_OUT_OF_KEY_REGISTERS:
            return DRM_Prdy_oemhal_out_of_key_registers;
        case DRM_E_OEMHAL_KEYS_IN_USE:
            return DRM_Prdy_oemhal_keys_in_use;
        case DRM_E_OEMHAL_NO_KEY:
            return DRM_Prdy_oemhal_no_key;
        case DRM_E_OEMHAL_UNSUPPORTED_KEY_TYPE:
            return DRM_Prdy_oemhal_unsupported_key_type;
        case DRM_E_OEMHAL_UNSUPPORTED_KEY_WRAPPING_FORMAT:
            return DRM_Prdy_oemhal_unsupported_key_wrapping_format;
        case DRM_E_OEMHAL_UNSUPPORTED_KEY_LENGTH:
            return DRM_Prdy_oemhal_unsupported_key_length;
        case DRM_E_OEMHAL_UNSUPPORTED_HASH_TYPE:
            return DRM_Prdy_oemhal_unsupported_hash_type;
        case DRM_E_OEMHAL_UNSUPPORTED_SIGNATURE_SCHEME:
            return DRM_Prdy_oemhal_unsupported_signature_scheme;
        case DRM_E_OEMHAL_BUFFER_TOO_LARGE:
            return DRM_Prdy_oemhal_buffer_too_large;
        case DRM_E_OEMHAL_SAMPLE_ENCRYPTION_MODE_NOT_PERMITTED:
            return DRM_Prdy_oemhal_sample_encryption_mode_not_permitted;
        case DRM_E_M2TS_BASECODE:
            return DRM_Prdy_m2ts_basecode;
        case DRM_E_M2TS_PAT_PID_IS_NOT_ZERO:
            return DRM_Prdy_m2ts_pat_pid_is_not_zero;
        case DRM_E_M2TS_PTS_NOT_EXIST:
            return DRM_Prdy_m2ts_pts_not_exist;
        case DRM_E_M2TS_PES_PACKET_LENGTH_NOT_SPECIFIED:
            return DRM_Prdy_m2ts_pes_packet_length_not_specified;
        case DRM_E_M2TS_OUTPUT_BUFFER_FULL:
            return DRM_Prdy_m2ts_output_buffer_full;
        case DRM_E_M2TS_CONTEXT_NOT_INITIALIZED:
            return DRM_Prdy_m2ts_context_not_initialized;
        case DRM_E_M2TS_NEED_KEY_DATA:
            return DRM_Prdy_m2ts_need_key_data;
        case DRM_E_M2TS_DDPLUS_FORMAT_INVALID:
            return DRM_Prdy_m2ts_ddplus_format_invalid;
        case DRM_E_M2TS_NOT_UNIT_START_PACKET:
            return DRM_Prdy_m2ts_not_unit_start_packet;
        case DRM_E_M2TS_TOO_MANY_SUBSAMPLES:
            return DRM_Prdy_m2ts_too_many_subsamples;
        case DRM_E_M2TS_TABLE_ID_INVALID:
            return DRM_Prdy_m2ts_table_id_invalid;
        case DRM_E_M2TS_PACKET_SYNC_BYTE_INVALID:
            return DRM_Prdy_m2ts_packet_sync_byte_invalid;
        case DRM_E_M2TS_ADAPTATION_LENGTH_INVALID:
            return DRM_Prdy_m2ts_adaptation_length_invalid;
        case DRM_E_M2TS_PAT_HEADER_INVALID:
            return DRM_Prdy_m2ts_pat_header_invalid;
        case DRM_E_M2TS_PMT_HEADER_INVALID:
            return DRM_Prdy_m2ts_pmt_header_invalid;
        case DRM_E_M2TS_PES_START_CODE_NOT_FOUND:
            return DRM_Prdy_m2ts_pes_start_code_not_found;
        case DRM_E_M2TS_STREAM_OR_PACKET_TYPE_CHANGED:
            return DRM_Prdy_m2ts_stream_or_packet_type_changed;
        case DRM_E_M2TS_INTERNAL_ERROR:
            return DRM_Prdy_m2ts_internal_error;
        case DRM_E_M2TS_ADTS_FORMAT_INVALID:
            return DRM_Prdy_m2ts_adts_format_invalid;
        case DRM_E_M2TS_MPEGA_FORMAT_INVALID:
            return DRM_Prdy_m2ts_mpega_format_invalid;
        case DRM_E_M2TS_CA_DESCRIPTOR_LENGTH_INVALID:
            return DRM_Prdy_m2ts_ca_descriptor_length_invalid;
        case DRM_E_M2TS_CRC_FIELD_INVALID:
            return DRM_Prdy_m2ts_crc_field_invalid;
        case DRM_E_M2TS_INCOMPLETE_SECTION_HEADER:
            return DRM_Prdy_m2ts_incomplete_section_header;
        case DRM_E_M2TS_INVALID_UNALIGNED_DATA:
            return DRM_Prdy_m2ts_invalid_unaligned_data;
        case DRM_E_M2TS_GET_ENCRYPTED_DATA_FIRST:
            return DRM_Prdy_m2ts_get_encrypted_data_first;
        case DRM_E_M2TS_CANNOT_CHANGE_PARAMETER:
            return DRM_Prdy_m2ts_cannot_change_parameter;
        case DRM_E_M2TS_UNKNOWN_PACKET:
            return DRM_Prdy_m2ts_unknown_packet;
        case DRM_E_M2TS_DROP_PACKET:
            return DRM_Prdy_m2ts_drop_packet;
        case DRM_E_M2TS_DROP_PES:
            return DRM_Prdy_m2ts_drop_pes;
        case DRM_E_M2TS_INCOMPLETE_PES:
            return DRM_Prdy_m2ts_incomplete_pes;
        case DRM_E_M2TS_WAITED_TOO_LONG:
            return DRM_Prdy_m2ts_waited_too_long;
        case DRM_E_M2TS_SECTION_LENGTH_INVALID:
            return DRM_Prdy_m2ts_section_length_invalid;
        case DRM_E_M2TS_PROGRAM_INFO_LENGTH_INVALID:
            return DRM_Prdy_m2ts_program_info_length_invalid;
        case DRM_E_M2TS_PES_HEADER_INVALID:
            return DRM_Prdy_m2ts_pes_header_invalid;
        case DRM_E_M2TS_ECM_PAYLOAD_OVER_LIMIT:
            return DRM_Prdy_m2ts_ecm_payload_over_limit;
        case DRM_E_M2TS_SET_CA_PID_FAILED:
            return DRM_Prdy_m2ts_set_ca_pid_failed;
        case DRM_E_LICGEN_BASECODE:
            return DRM_Prdy_licgen_basecode;
        case DRM_E_LICGEN_CANNOT_PERSIST_LICENSE:
            return DRM_Prdy_licgen_cannot_persist_license;
        case DRM_E_LICGEN_PERSISTENT_REMOTE_LICENSE:
            return DRM_Prdy_licgen_persistent_remote_license;
        case DRM_E_LICGEN_EXPIRE_AFTER_FIRST_PLAY_REMOTE_LICENSE:
            return DRM_Prdy_licgen_expire_after_first_play_remote_license;
        case DRM_E_LICGEN_ROOT_LICENSE_CANNOT_ENCRYPT:
            return DRM_Prdy_licgen_root_license_cannot_encrypt;
        case DRM_E_LICGEN_EMBED_LOCAL_LICENSE:
            return DRM_Prdy_licgen_embed_local_license;
        case DRM_E_LICGEN_LOCAL_LICENSE_WITH_REMOTE_CERTIFICATE:
            return DRM_Prdy_licgen_local_license_with_remote_certificate;
        case DRM_E_LICGEN_PLAY_ENABLER_REMOTE_LICENSE:
            return DRM_Prdy_licgen_play_enabler_remote_license;
        case DRM_E_LICGEN_DUPLICATE_PLAY_ENABLER:
            return DRM_Prdy_licgen_duplicate_play_enabler;
        case DRM_E_HDCP_ERR:
            return DRM_Prdy_hdcp_err;
        default:
            return DRM_Prdy_fail;
    }
}

static
bool convertCStringToWString( char * pCStr, DRM_WCHAR * pWStr, DRM_DWORD * cchStr)
{
    DRM_RESULT         dr = DRM_SUCCESS;
    bool               result = false;
    DRM_SUBSTRING      tmpSubStr;
    DRM_CHAR           tmpCStr[ DRM_MAX_PATH ];
    DRM_WCHAR          tmpWChar[ DRM_MAX_PATH ];
    DRM_STRING         tmpWStr = EMPTY_DRM_STRING;

    if(( pCStr != NULL) && (pWStr != NULL))
    {
        /* Convert the given char * to DRM_CHAR * */
        ZEROMEM(tmpCStr,DRM_MAX_PATH);
        ChkDR( DRM_STR_StringCchCopyA(
                 tmpCStr,
                 SIZEOF(tmpCStr),
                 pCStr) );

        /* Make sure tmpWChar is NULL terminated */
        BKNI_Memset(tmpWChar, 0, (DRM_MAX_PATH * SIZEOF(DRM_WCHAR)));

        tmpSubStr.m_ich = 0;
        tmpSubStr.m_cch = strlen( (char*)tmpCStr );

        /* Convert the DRM_CHAR * to DRM_STRING. */
        tmpWStr.pwszString = tmpWChar;
        tmpWStr.cchString  = DRM_MAX_PATH;
        DRM_UTL_PromoteASCIItoUNICODE( tmpCStr,
                                       &tmpSubStr,
                                       &tmpWStr);

        BKNI_Memcpy(pWStr, tmpWStr.pwszString, (tmpWStr.cchString+1) * SIZEOF (DRM_WCHAR));
        *cchStr = tmpWStr.cchString;
        pWStr[tmpWStr.cchString] = g_wchNull;
        result = true;
    }

ErrorExit:
    return result;
}

static
bool convertWStringToCString( DRM_WCHAR * pWStr, char * pCStr, DRM_DWORD cchStr)
{
    if(( pCStr != NULL) && (pWStr != NULL))
    {
        DRM_UTL_DemoteUNICODEtoASCII( pWStr,
                                      pCStr,
                                      cchStr);

        return true;
    }

    return false;
}

/* Following OEM_Opaque static functions may be required for
 * Drm_LocalLicense_EncryptOpaqueSample() in the future.
 * They are copied from SDK: oemhaloemimpl.c.
 * Since they are not being used at the moment, commment them out */

#if 0
static
bool OEM_OpaqueBufferCreate( OEM_OPAQUE_BUFFER_HANDLE    *f_phBuf )
{
    DRM_RESULT                           dr     = DRM_SUCCESS;
    DRM_OPAQUE_BUFFER_HANDLE_INTERNAL   *pBuf   = NULL;

    ChkPtr( f_phBuf );
    *f_phBuf = OEM_OPAQUE_BUFFER_HANDLE_INVALID;

    ChkMem( pBuf = (DRM_OPAQUE_BUFFER_HANDLE_INTERNAL*)Oem_MemAlloc( SIZEOF(DRM_OPAQUE_BUFFER_HANDLE_INTERNAL) ) );
    OEM_SECURE_ZERO_MEMORY( pBuf, SIZEOF(DRM_OPAQUE_BUFFER_HANDLE_INTERNAL) );

    BDBG_ASSERT( pBuf->eType == eDRM_OPAQUE_BUFFER_HANDLE_INTERNAL_TYPE_INVALID );

    *f_phBuf = (OEM_OPAQUE_BUFFER_HANDLE)pBuf;
    pBuf     = NULL;

    SAFE_OEM_FREE( pBuf );
    return true;

ErrorExit:
    SAFE_OEM_FREE( pBuf );
    BDBG_ERR(("%s: OEM_OpaqueBufferCreate failed [0x%X], exiting...", __FUNCTION__,dr));
    return false;
}

static
bool OEM_OpaqueBufferCreateWithData(
    const DRM_BYTE                         *f_pbData,
    DRM_DWORD                               f_cbData,
    DRM_OPAQUE_BUFFER_HANDLE_INTERNAL_TYPE  f_type,
    OEM_OPAQUE_BUFFER_HANDLE               *f_phBuf )
{
    DRM_RESULT                           dr     = DRM_SUCCESS;
    DRM_OPAQUE_BUFFER_HANDLE_INTERNAL   *pBuf   = NULL;
    /*DRM_BYTE                            *pbData = NULL;*/

    ChkPtr( f_pbData );
    ChkArg( f_cbData != 0    );
    ChkPtr( f_phBuf );
    *f_phBuf = OEM_OPAQUE_BUFFER_HANDLE_INVALID;

    ChkMem( pBuf = (DRM_OPAQUE_BUFFER_HANDLE_INTERNAL*)Oem_MemAlloc( SIZEOF(DRM_OPAQUE_BUFFER_HANDLE_INTERNAL) ) );
    OEM_SECURE_ZERO_MEMORY( pBuf, SIZEOF(DRM_OPAQUE_BUFFER_HANDLE_INTERNAL) );

    /*ChkMem( pbData = (DRM_BYTE*)Oem_MemAlloc( f_cbData ) );
      OEM_SECURE_MEMCPY( pbData, f_pbData, f_cbData ); */

    pBuf->cbData = f_cbData;
    pBuf->eType  = f_type;
    pBuf->pbData = (DRM_BYTE *)f_pbData;
    /*pBuf->pbData = (DRM_BYTE *)pbData;*/
    /*pbData       = NULL;*/
    *f_phBuf = (OEM_OPAQUE_BUFFER_HANDLE)pBuf;
    pBuf     = NULL;

    SAFE_OEM_FREE( pBuf );
    /*SAFE_OEM_FREE( pbData );*/
    return true;

ErrorExit:
    SAFE_OEM_FREE( pBuf );
    /*SAFE_OEM_FREE( pbData );*/
    BDBG_ERR(("%s: OEM_OpaqueBufferCreateWithData failed [0x%X], exiting...\n", __FUNCTION__,dr));
    return false;
}


static
bool OEM_OpaqueBufferDestroy( OEM_OPAQUE_BUFFER_HANDLE   *f_phBuf )
{
    if( f_phBuf != NULL )
    {
        DRM_OPAQUE_BUFFER_HANDLE_INTERNAL *pBuf = (DRM_OPAQUE_BUFFER_HANDLE_INTERNAL*)*f_phBuf;
        if( *f_phBuf != OEM_OPAQUE_BUFFER_HANDLE_INVALID && pBuf != NULL )
        {
            /*SAFE_OEM_FREE( pBuf->pbData );*/
            pBuf->pbData = NULL; /* we don't free the pbData for the application */
            SAFE_OEM_FREE( pBuf );
        }
        *f_phBuf = OEM_OPAQUE_BUFFER_HANDLE_INVALID;
    }
    return true;
}
#endif


static
bool load_revocation_list(
        PRDY_APP_CONTEXT    *pPrdyCxt,
        char                *revListFile)
{
    DRM_RESULT dr = DRM_SUCCESS;
    FILE    * fRev;
    uint8_t * revBuf = NULL;
    size_t    fileSize = 0;
    uint32_t  currSize = 0;

    BDBG_ASSERT(pPrdyCxt != NULL);
    BDBG_ASSERT(revListFile != NULL);

    fRev = fopen(revListFile, "rb");
    if( fRev == NULL)
    {
        BDBG_WRN(("[WARNING] %s %d: Failed to open %s \n",__FUNCTION__,__LINE__,revListFile));
        return true;
    }

    /* get the size of the file */
    fseek(fRev, 0, SEEK_END);
    fileSize = ftell(fRev);
    fseek(fRev, 0, SEEK_SET);

    revBuf = BKNI_Malloc(fileSize);
    if( revBuf == NULL)
    {
        BDBG_ERR(("%s: Failed to allocate memory.\n",__FUNCTION__));
        goto ErrorExit;
    }

    BKNI_Memset(revBuf, 0x00, fileSize);

    for(;;) {
        uint8_t buf[512];
        int rc = fread(buf, 1, sizeof(buf), fRev);
        if(rc<=0) {
            break;
        }
        BKNI_Memcpy(revBuf+currSize, buf, rc);
        currSize += rc;
    }

    ChkDR( Drm_Revocation_StorePackage(
                pPrdyCxt->pDrmAppCtx,
                ( DRM_CHAR * )revBuf,
                fileSize ) );

    BDBG_MSG(("Drm_Revocation_StorePackage succeeded.\n"));

    if( revBuf != NULL)
        BKNI_Free(revBuf);

    return true;

ErrorExit:
    if( revBuf != NULL)
        BKNI_Free(revBuf);

    BDBG_ERR(("Revocation Store Package failed [0x%X]\n",(unsigned int)dr));
    return false;
}


/* internal call back function for Drm_Reader_Bind */
static
DRM_RESULT DRM_API DRM_Prdy_policy_callback(
    __in const DRM_VOID  *f_pvCallbackData,
    __in       DRM_DWORD  f_dwCallbackType,
    __in const DRM_VOID  *f_pv )
{
    PRDY_APP_CONTEXT  *pPrdyContext = (PRDY_APP_CONTEXT  *)f_pv;
    DRM_RESULT rc = DRM_SUCCESS;
    BDBG_MSG(("%s - entering", __FUNCTION__));
    DRM_Prdy_policy_t *policy = NULL;
    DRM_PLAY_OPL_EX2 *pPolicyData=NULL;

    BDBG_ASSERT(f_pvCallbackData != NULL);
    BDBG_ASSERT(f_pv != NULL);

    if(pPrdyContext->nbOfPolicyQueued < POLICY_POOL_SIZE){
         policy = &pPrdyContext->protectionPolicy[pPrdyContext->nbOfPolicyQueued];
         pPrdyContext->nbOfPolicyQueued++;
    }
    else {
        BDBG_ERR(("Policy lost. Policy Queue is full"));
        return DRM_E_RIGHTS_NOT_AVAILABLE;
    }


    policy->type = f_dwCallbackType;

    switch(policy->type){
        case PLAY_OPL:
            pPolicyData = (DRM_PLAY_OPL_EX2 *)f_pvCallbackData;
            policy->t.play.dwVersion = pPolicyData->dwVersion;
            BKNI_Memcpy(&policy->t.play.minOPL, &pPolicyData->minOPL, sizeof(DRM_Prdy_Minimum_Output_Protection_Levels_t));

            BKNI_Memcpy(&policy->t.play.oplIdReserved, &pPolicyData->oplIdReserved, sizeof(DRM_Prdy_Opl_Output_Ids_t));
            if(pPolicyData->oplIdReserved.cIds != 0)
                BKNI_Memcpy(policy->t.play.oplIdReserved.rgIds, &pPolicyData->oplIdReserved.rgIds, sizeof(DRM_Prdy_guid_t) * pPolicyData->oplIdReserved.cIds);

            BKNI_Memcpy(&policy->t.play.vopi, &pPolicyData->vopi, sizeof(DRM_Prdy_video_out_protection_ids_ex_t));
            BKNI_Memcpy(&policy->t.play.aopi, &pPolicyData->aopi, sizeof(DRM_Prdy_audio_out_protection_ids_ex_t));
#if 0 /* Depracated */
            policy->t.play.i_compressedDigitalVideo   =
                (uint32_t)((DRM_PLAY_OPL *)f_pvCallbackData)->minOPL.wCompressedDigitalVideo;
            policy->t.play.i_uncompressedDigitalVideo =
                (uint32_t)((DRM_PLAY_OPL *)f_pvCallbackData)->minOPL.wUncompressedDigitalVideo;
            policy->t.play.i_analogVideo              =
                (uint32_t)((DRM_PLAY_OPL *)f_pvCallbackData)->minOPL.wAnalogVideo;
            policy->t.play.i_compressedDigitalAudio   =
                (uint32_t)((DRM_PLAY_OPL *)f_pvCallbackData)->minOPL.wCompressedDigitalAudio;
            policy->t.play.i_uncompressedDigitalAudio =
                (uint32_t)((DRM_PLAY_OPL *)f_pvCallbackData)->minOPL.wUncompressedDigitalAudio;

            policy->t.play.i_resv_cIds  =
                (uint32_t)((DRM_PLAY_OPL *)f_pvCallbackData)->oplIdReserved.cIds;
            policy->t.play.i_resv_rgIds =
                (DRM_Prdy_guid_t *)((DRM_PLAY_OPL *)f_pvCallbackData)->oplIdReserved.rgIds;

            policy->t.play.i_vop_cEntries  =
                (uint32_t)((DRM_PLAY_OPL *)f_pvCallbackData)->vopi.cEntries;
            policy->t.play.i_vop           =
                (DRM_Prdy_video_out_protection_t *)((DRM_PLAY_OPL *)f_pvCallbackData)->vopi.rgVop;
#endif
            break;

        case COPY_OPL:
            policy->t.copy.i_minimumCopyLevel =
                (uint16_t)((DRM_COPY_OPL *)f_pvCallbackData)->wMinimumCopyLevel;
            policy->t.copy.i_includes_cIds    =
                (uint16_t)((DRM_COPY_OPL *)f_pvCallbackData)->oplIdIncludes.cIds;
            policy->t.copy.i_includes_rgIds   =
                (DRM_Prdy_guid_t *)((DRM_COPY_OPL *)f_pvCallbackData)->oplIdIncludes.rgIds;
            policy->t.copy.i_excludes_cIds    =
                (uint16_t)((DRM_COPY_OPL *)f_pvCallbackData)->oplIdExcludes.cIds;
            policy->t.copy.i_excludes_rgIds   =
                (DRM_Prdy_guid_t *)((DRM_COPY_OPL *)f_pvCallbackData)->oplIdExcludes.rgIds;
            break;
        case INCLUSION_LIST:
            BKNI_Memcpy(&policy->t.inc_list, f_pvCallbackData, sizeof(DRM_INCLUSION_LIST_CALLBACK_STRUCT));
            break;
        case EXTENDED_RESTRICTION_CONDITION:
            BKNI_Memcpy(&policy->t.condition, f_pvCallbackData, sizeof(DRM_EXTENDED_RESTRICTION_CALLBACK_STRUCT));
            break;
        case EXTENDED_RESTRICTION_ACTION:
            BKNI_Memcpy(&policy->t.action, f_pvCallbackData, sizeof(DRM_EXTENDED_RESTRICTION_CALLBACK_STRUCT));
            break;
        case EXTENDED_RESTRICTION_QUERY:
            BKNI_Memcpy(&policy->t.query, f_pvCallbackData, sizeof(DRM_EXTENDED_RESTRICTION_CALLBACK_STRUCT));
            break;
        case SECURE_STATE_TOKEN_RESOLVE:
            BKNI_Memcpy(&policy->t.token_res, f_pvCallbackData, sizeof(DRM_SECURE_STATE_TOKEN_RESOLVE_DATA));
            break;
        case RESTRICTED_SOURCEID:
            BKNI_Memcpy(&policy->t.restr_src, f_pvCallbackData, sizeof(DRM_RESTRICTED_SOURCEID_CALLBACK_STRUCT));
            break;
        default:
            /* Unsupported operation */
            rc = DRM_E_RIGHTS_NOT_AVAILABLE;
            break;
    }

    BDBG_MSG(("%s: Exiting, rc %d.\n", __FUNCTION__, rc));
    return rc;
}

void* DRM_Prdy_GetDrmAppContext(DRM_Prdy_Handle_t handle)
{
    PRDY_APP_CONTEXT  *pPrdyCxt = (PRDY_APP_CONTEXT*)handle;
    if(pPrdyCxt == NULL) {
        BDBG_ERR(("%s - Given handle is null\n", __FUNCTION__));
        return NULL;
    }

    return (void *)pPrdyCxt->pDrmAppCtx;
}

void DRM_Prdy_GetDefaultParamSettings( DRM_Prdy_Init_t *pPrdyParamSettings)
{
    BDBG_MSG(("%s - entering", __FUNCTION__));
    BDBG_ASSERT(pPrdyParamSettings != NULL);

    BKNI_Memset(pPrdyParamSettings, 0x00, sizeof(DRM_Prdy_Init_t));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
}

DRM_Prdy_Handle_t  DRM_Prdy_Initialize(DRM_Prdy_Init_t * pInitSetting)
{
    DRM_RESULT         dr = DRM_SUCCESS;
    PRDY_APP_CONTEXT  *pPrdyCxt = NULL;
    NEXUS_MemoryAllocationSettings heapSettings;
    NEXUS_ClientConfiguration platformConfig;
    char *             revFile = NULL;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    /* initialize the PRDY_APP_CONTEXT */
    pPrdyCxt = Oem_MemAlloc(sizeof(PRDY_APP_CONTEXT));
    if( pPrdyCxt == NULL ) {
        BDBG_ERR(("%s: Prdy Context alloc failed\n", __FUNCTION__));
        goto ErrorExit;
    }
    BKNI_Memset(pPrdyCxt, 0, sizeof(PRDY_APP_CONTEXT));

    pPrdyCxt->oemSettings.binFileName = NULL;
    pPrdyCxt->oemSettings.keyFileName = NULL;
    pPrdyCxt->oemSettings.keyHistoryFileName = NULL;
    pPrdyCxt->oemSettings.defaultRWDirName = NULL;

    /* initialize the DRM_APP_CONTEXT */
    pPrdyCxt->pDrmAppCtx = Oem_MemAlloc(SIZEOF(DRM_APP_CONTEXT));
    BDBG_MSG(("%s: allocated DrmAppCtx %p size %u",__FUNCTION__,pPrdyCxt->pDrmAppCtx,(SIZEOF(DRM_APP_CONTEXT))));
    ChkMem(pPrdyCxt->pDrmAppCtx);
    ZEROMEM( ( uint8_t * )pPrdyCxt->pDrmAppCtx, SIZEOF( DRM_APP_CONTEXT));

    /* copy the binFileName if provided */
    if( (pInitSetting != NULL) && (pInitSetting->binFileName != NULL))
    {
        DRM_DWORD  cchStr = 0;
        pPrdyCxt->oemSettings.binFileName = Oem_MemAlloc(SIZEOF(DRM_WCHAR) * DRM_MAX_PATH);
        BDBG_MSG(("%s: allocated binFileName %p size %u",__FUNCTION__,pPrdyCxt->oemSettings.binFileName,(SIZEOF(DRM_WCHAR) * DRM_MAX_PATH)));
        if( !convertCStringToWString(pInitSetting->binFileName, pPrdyCxt->oemSettings.binFileName, &cchStr))
        {
            SAFE_OEM_FREE(pPrdyCxt->oemSettings.binFileName);
        }
    }

    /* copy the keyFileName if provided */
    if( (pInitSetting != NULL) && (pInitSetting->keyFileName != NULL))
    {
        DRM_DWORD  cchStr = 0;
        pPrdyCxt->oemSettings.keyFileName = Oem_MemAlloc(SIZEOF(DRM_WCHAR) * DRM_MAX_PATH);
        BDBG_MSG(("%s: allocated keyFileName %p size %u",__FUNCTION__,pPrdyCxt->oemSettings.keyFileName,(SIZEOF(DRM_WCHAR) * DRM_MAX_PATH)));
        if( !convertCStringToWString(pInitSetting->keyFileName, pPrdyCxt->oemSettings.keyFileName, &cchStr))
        {
            SAFE_OEM_FREE(pPrdyCxt->oemSettings.keyFileName);
        }
    }

    /* copy the keyHistoryFileName if provided */
    if( (pInitSetting != NULL) && (pInitSetting->keyHistoryFileName != NULL))
    {
        DRM_DWORD  cchStr = 0;
        pPrdyCxt->oemSettings.keyHistoryFileName = Oem_MemAlloc(SIZEOF(DRM_WCHAR) * DRM_MAX_PATH);
        BDBG_MSG(("%s: allocated keyHistoryFileName %p size %u",__FUNCTION__,pPrdyCxt->oemSettings.keyHistoryFileName,(SIZEOF(DRM_WCHAR) * DRM_MAX_PATH)));
        if( !convertCStringToWString(pInitSetting->keyHistoryFileName, pPrdyCxt->oemSettings.keyHistoryFileName, &cchStr))
        {
            SAFE_OEM_FREE(pPrdyCxt->oemSettings.keyHistoryFileName);
        }
    }

    /* copy the defaultRWDirName if provided */
    if( (pInitSetting != NULL) && (pInitSetting->defaultRWDirName != NULL))
    {
        DRM_DWORD  cchStr = 0;
        pPrdyCxt->oemSettings.defaultRWDirName = Oem_MemAlloc(SIZEOF(DRM_WCHAR) * DRM_MAX_PATH);
        BDBG_MSG(("%s: allocated defaultRWDirName %p size %u",__FUNCTION__,pPrdyCxt->oemSettings.defaultRWDirName,(SIZEOF(DRM_WCHAR) * DRM_MAX_PATH)));
        if( !convertCStringToWString(pInitSetting->defaultRWDirName, pPrdyCxt->oemSettings.defaultRWDirName, &cchStr))
        {
            SAFE_OEM_FREE(pPrdyCxt->oemSettings.defaultRWDirName);
        }
    }

    /* Drm_Platform_Initialize */
    NEXUS_Memory_GetDefaultAllocationSettings(&heapSettings);
    NEXUS_Platform_GetClientConfiguration(&platformConfig);
    if (platformConfig.heap[NXCLIENT_FULL_HEAP])
    {
        NEXUS_HeapHandle heap = platformConfig.heap[NXCLIENT_FULL_HEAP];
        BDBG_MSG(("%s: heap = %p",__FUNCTION__,heap));
        NEXUS_MemoryStatus heapStatus;
        NEXUS_Heap_GetStatus(heap, &heapStatus);
        if (heapStatus.memoryType & NEXUS_MemoryType_eFull)
        {
            heapSettings.heap = heap;
        }
    }

    pPrdyCxt->oemSettings.heap = heapSettings.heap;
#ifdef PLAYREADY_HOST_IMPL
#if SAGE_VERSION >= SAGE_VERSION_CALC(3,0)
#ifdef USE_UNIFIED_COMMON_DRM
    pPrdyCxt->oemSettings.drmType = 0;
#else
    pPrdyCxt->oemSettings.drmType = BSAGElib_BinFileDrmType_ePlayready;
#endif
    BDBG_MSG(("%s:drmType 0x%x", __FUNCTION__,pPrdyCxt->oemSettings.drmType));
#endif
#endif
    pPrdyCxt->pOEMContext = Drm_Platform_Initialize(&pPrdyCxt->oemSettings);
    ChkMem(pPrdyCxt->pOEMContext);

    /* Initialize OpaqueBuffer and RevocationBuffer */
    pPrdyCxt->cbOpaqueBuffer = MINIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE;
    ChkMem( pPrdyCxt->pbOpaqueBuffer = ( uint8_t * )Oem_MemAlloc(MINIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE));
    ChkMem( pPrdyCxt->pbRevocationBuffer = ( uint8_t * )Oem_MemAlloc( REVOCATION_BUFFER_SIZE));

    BDBG_MSG(("%s: allocated OpaqueBuffer %p size %u", __FUNCTION__, pPrdyCxt->pbOpaqueBuffer, MINIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE));
    BDBG_MSG(("%s: allocated RevocationBuffer %p size %u", __FUNCTION__, pPrdyCxt->pbRevocationBuffer, REVOCATION_BUFFER_SIZE));

    /* Drm_Initialize */
    sDstrHDSPath.pwszString = sRgwchHDSPath;
    sDstrHDSPath.cchString = DRM_MAX_PATH;

    /* Convert the HDS path to DRM_STRING. */
    if((pInitSetting != NULL) && (pInitSetting->hdsFileName != NULL))
    {
        if( !convertCStringToWString(pInitSetting->hdsFileName, (DRM_WCHAR*)sDstrHDSPath.pwszString, &sDstrHDSPath.cchString))
        {
            goto ErrorExit;
        }
    }
    else
    {
        DRM_WCHAR *hdsDir = bdrm_get_hds_dir();
        DRM_WCHAR *hdsFname = bdrm_get_hds_fname();
        if (hdsFname != NULL && bdrm_get_hds_fname_lgth() > 0) {
            if (bdrm_get_hds_dir_lgth() > 0)
                BKNI_Memcpy((DRM_WCHAR*)sDstrHDSPath.pwszString, hdsDir, bdrm_get_hds_dir_lgth() * sizeof(DRM_WCHAR));
            BKNI_Memcpy((DRM_WCHAR*)sDstrHDSPath.pwszString + bdrm_get_hds_dir_lgth(),
                hdsFname, (bdrm_get_hds_fname_lgth() + 1) * sizeof(DRM_WCHAR));
        }
        else if( !convertCStringToWString(DRM_LICENSE_STORAGE_FILE, (DRM_WCHAR*)sDstrHDSPath.pwszString, &sDstrHDSPath.cchString))
        {
            goto ErrorExit;
        }
    }
   BDBG_MSG(("%s calling Drm_Initialize DrmAppCtx %p pOEMContext %p ",__FUNCTION__,pPrdyCxt->pDrmAppCtx, pPrdyCxt->pOEMContext));
   /* TODO: perform synchronous activation if Drm_Initialize fails */
   ChkDR( Drm_Initialize( pPrdyCxt->pDrmAppCtx,
                          pPrdyCxt->pOEMContext,
                          pPrdyCxt->pbOpaqueBuffer,
                          pPrdyCxt->cbOpaqueBuffer,
                          &sDstrHDSPath) );
   BDBG_MSG(("%s calling Drm_Revocation_SetBuffer pPrdyCxt->pDrmAppCtx %p",__FUNCTION__,pPrdyCxt->pDrmAppCtx));
   ChkDR( Drm_Revocation_SetBuffer( pPrdyCxt->pDrmAppCtx,
                                    pPrdyCxt->pbRevocationBuffer,
                                    REVOCATION_BUFFER_SIZE ) );

   /* load the revocation list */

   if( (revFile = pInitSetting->revocationListFileName) == NULL)
   {
       revFile = DRM_DEFAULT_REVOCATION_LIST_FILE;
   }

   if( !load_revocation_list( pPrdyCxt,revFile))
   {
       goto ErrorExit;
   }

#ifdef CMD_DRM_PLAYREADY_SAGE_IMPL
   {
    DRMSYSTEMTIME     systemTime;
    /* initialize the systemtime */
    DRM_APP_CONTEXT_INTERNAL    *poAppContextInternal = ( DRM_APP_CONTEXT_INTERNAL * )pPrdyCxt->pDrmAppCtx;
    Oem_Clock_GetSystemTime( pPrdyCxt->pOEMContext, &systemTime);
    Oem_Clock_SetSystemTime( &poAppContextInternal->oBlackBoxContext, &systemTime);
   }
#endif
   BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
   return (DRM_Prdy_Handle_t ) pPrdyCxt;

ErrorExit:
   BDBG_ERR(("%s: Exiting with error [0x%X].\n", __FUNCTION__,(unsigned int)dr));
   if( pPrdyCxt != NULL)
       DRM_Prdy_Uninitialize( pPrdyCxt);
   return NULL;
}

DRM_Prdy_Handle_t  DRM_Prdy_Initialize_Ex(DRM_Prdy_Init_t * pInitSetting, DRM_Prdy_Error_e *Drm_Prdy_Error)
{
    DRM_RESULT         dr = DRM_SUCCESS;
    PRDY_APP_CONTEXT  *pPrdyCxt = NULL;
    NEXUS_MemoryAllocationSettings heapSettings;
    NEXUS_ClientConfiguration platformConfig;
    char *             revFile = NULL;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    if( Drm_Prdy_Error == NULL ) {
        BDBG_ERR(("%s: invalid argument\n", __FUNCTION__));
        dr = DRM_E_INVALIDARG;
        goto ErrorExit;
    }

    /* initialize the PRDY_APP_CONTEXT */
    pPrdyCxt = Oem_MemAlloc(sizeof(PRDY_APP_CONTEXT));
    if( pPrdyCxt == NULL ) {
        BDBG_ERR(("%s: Prdy Context alloc failed\n", __FUNCTION__));
        goto ErrorExit;
    }
    BKNI_Memset(pPrdyCxt, 0, sizeof(PRDY_APP_CONTEXT));

    pPrdyCxt->oemSettings.binFileName = NULL;
    pPrdyCxt->oemSettings.keyFileName = NULL;
    pPrdyCxt->oemSettings.keyHistoryFileName = NULL;
    pPrdyCxt->oemSettings.defaultRWDirName = NULL;

    /* initialize the DRM_APP_CONTEXT */
    pPrdyCxt->pDrmAppCtx = Oem_MemAlloc(SIZEOF(DRM_APP_CONTEXT));
    ChkMem(pPrdyCxt->pDrmAppCtx);
    ZEROMEM( ( uint8_t * )pPrdyCxt->pDrmAppCtx, SIZEOF( DRM_APP_CONTEXT));

    /* copy the binFileName if provided */
    if( (pInitSetting != NULL) && (pInitSetting->binFileName != NULL))
    {
        DRM_DWORD  cchStr = 0;
        pPrdyCxt->oemSettings.binFileName = Oem_MemAlloc(SIZEOF(DRM_WCHAR) * DRM_MAX_PATH);
        if( !convertCStringToWString(pInitSetting->binFileName, pPrdyCxt->oemSettings.binFileName, &cchStr))
        {
            SAFE_OEM_FREE(pPrdyCxt->oemSettings.binFileName);
        }
    }

    /* copy the keyFileName if provided */
    if( (pInitSetting != NULL) && (pInitSetting->keyFileName != NULL))
    {
        DRM_DWORD  cchStr = 0;
        pPrdyCxt->oemSettings.keyFileName = Oem_MemAlloc(SIZEOF(DRM_WCHAR) * DRM_MAX_PATH);
        if( !convertCStringToWString(pInitSetting->keyFileName, pPrdyCxt->oemSettings.keyFileName, &cchStr))
        {
            SAFE_OEM_FREE(pPrdyCxt->oemSettings.keyFileName);
        }
    }

    /* copy the keyHistoryFileName if provided */
    if( (pInitSetting != NULL) && (pInitSetting->keyHistoryFileName != NULL))
    {
        DRM_DWORD  cchStr = 0;
        pPrdyCxt->oemSettings.keyHistoryFileName = Oem_MemAlloc(SIZEOF(DRM_WCHAR) * DRM_MAX_PATH);
        if( !convertCStringToWString(pInitSetting->keyHistoryFileName, pPrdyCxt->oemSettings.keyHistoryFileName, &cchStr))
        {
            SAFE_OEM_FREE(pPrdyCxt->oemSettings.keyHistoryFileName);
        }
    }

    /* copy the defaultRWDirName if provided */
    if( (pInitSetting != NULL) && (pInitSetting->defaultRWDirName != NULL))
    {
        DRM_DWORD  cchStr = 0;
        pPrdyCxt->oemSettings.defaultRWDirName = Oem_MemAlloc(SIZEOF(DRM_WCHAR) * DRM_MAX_PATH);
        if( !convertCStringToWString(pInitSetting->defaultRWDirName, pPrdyCxt->oemSettings.defaultRWDirName, &cchStr))
        {
            SAFE_OEM_FREE(pPrdyCxt->oemSettings.defaultRWDirName);
        }
    }

    /* Drm_Platform_Initialize */
    NEXUS_Memory_GetDefaultAllocationSettings(&heapSettings);
    NEXUS_Platform_GetClientConfiguration(&platformConfig);
    if (platformConfig.heap[NXCLIENT_FULL_HEAP])
    {
        NEXUS_HeapHandle heap = platformConfig.heap[NXCLIENT_FULL_HEAP];
        NEXUS_MemoryStatus heapStatus;
        NEXUS_Heap_GetStatus(heap, &heapStatus);
        if (heapStatus.memoryType & NEXUS_MemoryType_eFull)
        {
            heapSettings.heap = heap;
        }
    }

    pPrdyCxt->oemSettings.heap = heapSettings.heap;
    pPrdyCxt->pOEMContext = Drm_Platform_Initialize(&pPrdyCxt->oemSettings);
    ChkMem(pPrdyCxt->pOEMContext);

    /* Initialize OpaqueBuffer and RevocationBuffer */
    pPrdyCxt->cbOpaqueBuffer = MINIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE;
    ChkMem( pPrdyCxt->pbOpaqueBuffer = ( uint8_t * )Oem_MemAlloc(MINIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE));
    ChkMem( pPrdyCxt->pbRevocationBuffer = ( uint8_t * )Oem_MemAlloc( REVOCATION_BUFFER_SIZE));

    /* Drm_Initialize */
    sDstrHDSPath.pwszString = sRgwchHDSPath;
    sDstrHDSPath.cchString = DRM_MAX_PATH;

    /* Convert the HDS path to DRM_STRING. */
    if((pInitSetting != NULL) && (pInitSetting->hdsFileName != NULL))
    {
        if( !convertCStringToWString(pInitSetting->hdsFileName, (DRM_WCHAR*)sDstrHDSPath.pwszString, &sDstrHDSPath.cchString))
        {
            goto ErrorExit;
        }
    }
    else
    {
        DRM_WCHAR *hdsDir = bdrm_get_hds_dir();
        DRM_WCHAR *hdsFname = bdrm_get_hds_fname();
        if (hdsFname != NULL && bdrm_get_hds_fname_lgth() > 0) {
            if (bdrm_get_hds_dir_lgth() > 0)
                BKNI_Memcpy((DRM_WCHAR*)sDstrHDSPath.pwszString, hdsDir, bdrm_get_hds_dir_lgth() * sizeof(DRM_WCHAR));
            BKNI_Memcpy((DRM_WCHAR*)sDstrHDSPath.pwszString + bdrm_get_hds_dir_lgth(),
                hdsFname, (bdrm_get_hds_fname_lgth() + 1) * sizeof(DRM_WCHAR));
        }
        else if( !convertCStringToWString(DRM_LICENSE_STORAGE_FILE, (DRM_WCHAR*)sDstrHDSPath.pwszString, &sDstrHDSPath.cchString))
        {
            goto ErrorExit;
        }
    }

   /* TODO: perform synchronous activation if Drm_Initialize fails */
   ChkDR( Drm_Initialize( pPrdyCxt->pDrmAppCtx,
                          pPrdyCxt->pOEMContext,
                          pPrdyCxt->pbOpaqueBuffer,
                          pPrdyCxt->cbOpaqueBuffer,
                          &sDstrHDSPath) );

   ChkDR( Drm_Revocation_SetBuffer( pPrdyCxt->pDrmAppCtx,
                                    pPrdyCxt->pbRevocationBuffer,
                                    REVOCATION_BUFFER_SIZE ) );

   /* load the revocation list */

   if( (revFile = pInitSetting->revocationListFileName) == NULL)
   {
       revFile = DRM_DEFAULT_REVOCATION_LIST_FILE;
   }

   if( !load_revocation_list( pPrdyCxt,revFile))
   {
       goto ErrorExit;
   }

#ifdef CMD_DRM_PLAYREADY_SAGE_IMPL
   {
    DRMSYSTEMTIME     systemTime;
    /* initialize the systemtime */
    DRM_APP_CONTEXT_INTERNAL    *poAppContextInternal = ( DRM_APP_CONTEXT_INTERNAL * )pPrdyCxt->pDrmAppCtx;
    Oem_Clock_GetSystemTime( pPrdyCxt->pOEMContext, &systemTime);
    Oem_Clock_SetSystemTime( &poAppContextInternal->oBlackBoxContext, &systemTime);
   }
#endif
   BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
   *Drm_Prdy_Error = convertDrmResult(DRM_SUCCESS);
   return (DRM_Prdy_Handle_t ) pPrdyCxt;

ErrorExit:
   *Drm_Prdy_Error = convertDrmResult_Ex(dr);
   BDBG_ERR(("%s: Exiting with error [0x%X] (converted to [%d]).\n", __FUNCTION__,dr, *Drm_Prdy_Error));

   if( pPrdyCxt != NULL)
       DRM_Prdy_Uninitialize( pPrdyCxt);
   return NULL;
}

DRM_Prdy_Error_e DRM_Prdy_Uninitialize(DRM_Prdy_Handle_t  pPrdyContext)
{
    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    if( pPrdyContext->pDrmAppCtx ) {
        Drm_Uninitialize( pPrdyContext->pDrmAppCtx );
        SAFE_OEM_FREE( pPrdyContext->pDrmAppCtx );
    }
    SAFE_OEM_FREE(pPrdyContext->pbOpaqueBuffer);
    SAFE_OEM_FREE(pPrdyContext->pbRevocationBuffer);

    Drm_Platform_Uninitialize(pPrdyContext->pOEMContext);

    if(pPrdyContext->oemSettings.binFileName != NULL) SAFE_OEM_FREE(pPrdyContext->oemSettings.binFileName);
    if(pPrdyContext->oemSettings.keyFileName != NULL) SAFE_OEM_FREE(pPrdyContext->oemSettings.keyFileName);
    if(pPrdyContext->oemSettings.keyHistoryFileName != NULL) SAFE_OEM_FREE(pPrdyContext->oemSettings.keyHistoryFileName);
    if(pPrdyContext->oemSettings.defaultRWDirName != NULL) SAFE_OEM_FREE(pPrdyContext->oemSettings.defaultRWDirName);

    SAFE_OEM_FREE(pPrdyContext);

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;
}

DRM_Prdy_Error_e DRM_Prdy_Reinitialize(DRM_Prdy_Handle_t  pPrdyContext)
{
    DRM_RESULT        dr = DRM_SUCCESS;
    DRM_Prdy_Error_e  result = DRM_Prdy_fail;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    if( pPrdyContext->pDrmAppCtx ) {
        dr = Drm_Reinitialize( pPrdyContext->pDrmAppCtx );
        if (dr == DRM_SUCCESS) {
            result = DRM_Prdy_ok;
        }
    }
    else {
        result = DRM_Prdy_invalid_parameter;
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return result;
}

DRM_Prdy_Error_e DRM_Prdy_Get_Buffer_Size(
        DRM_Prdy_Handle_t            pPrdyContext,
        DRM_Prdy_GetBuffer_Type_e    chType,
        const uint8_t               *pData,
        uint32_t                     dataLen,
        uint32_t                    *pData1_size,
        uint32_t                    *pData2_size)
{
    DRM_RESULT        dr = DRM_SUCCESS;
    DRM_Prdy_Error_e  result = DRM_Prdy_ok;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);

    switch(chType) {
        case DRM_Prdy_getBuffer_content_property_header_kid:
        {
            DRM_DWORD cbHeader = 0;
            BDBG_ASSERT(pData1_size != NULL);
            dr = Drm_Content_GetProperty(
                    pPrdyContext->pDrmAppCtx,
                    DRM_CGP_HEADER_KID,
                    NULL,
                    &cbHeader );

            if ( dr == DRM_E_BUFFERTOOSMALL ) {
                *pData1_size = (uint32_t) cbHeader;
            }
            else {
                result = DRM_Prdy_fail;
            }
        }
        break;
        case DRM_Prdy_getBuffer_content_property_header_type:
        {
            DRM_DWORD cbHeader = 0;
            BDBG_ASSERT(pData1_size != NULL);
            dr = Drm_Content_GetProperty(
                    pPrdyContext->pDrmAppCtx,
                    DRM_CGP_HEADER_TYPE,
                    NULL,
                    &cbHeader );

            if ( dr == DRM_E_BUFFERTOOSMALL ) {
                *pData1_size = (uint32_t) cbHeader;
            }
            else {
                result = DRM_Prdy_fail;
            }
        }
        break;
        case DRM_Prdy_getBuffer_content_property_header:
        {
            DRM_DWORD cbHeader = 0;
            BDBG_ASSERT(pData1_size != NULL);
            dr = Drm_Content_GetProperty(
                    pPrdyContext->pDrmAppCtx,
                    DRM_CGP_HEADER,
                    NULL,
                    &cbHeader );

            if ( dr == DRM_E_BUFFERTOOSMALL ) {
                *pData1_size = (uint32_t) cbHeader;
            }
            else {
                result = DRM_Prdy_fail;
            }
        }
        break;
        case DRM_Prdy_getBuffer_content_property_playready_obj:
        {
            DRM_DWORD cbHeader = 0;
            BDBG_ASSERT(pData1_size != NULL);
            dr = Drm_Content_GetProperty(
                    pPrdyContext->pDrmAppCtx,
                    DRM_CGP_PLAYREADY_OBJ,
                    NULL,
                    &cbHeader );

            if ( dr == DRM_E_BUFFERTOOSMALL ) {
                *pData1_size = (uint32_t) cbHeader;
            }
            else {
                result = DRM_Prdy_fail;
            }
        }
        break;
        case DRM_Prdy_getBuffer_content_property_cipher_type:
        {
            DRM_DWORD cbHeader = 0;
            BDBG_ASSERT(pData1_size != NULL);
            dr = Drm_Content_GetProperty(
                    pPrdyContext->pDrmAppCtx,
                    DRM_CGP_CIPHER_TYPE,
                    NULL,
                    &cbHeader );

            if ( dr == DRM_E_BUFFERTOOSMALL ) {
                *pData1_size = (uint32_t) cbHeader;
            }
            else {
                result = DRM_Prdy_fail;
            }
        }
        break;
        case DRM_Prdy_getBuffer_content_property_decryptor_setup:
        {
            DRM_DWORD cbHeader = 0;
            BDBG_ASSERT(pData1_size != NULL);
            dr = Drm_Content_GetProperty(
                    pPrdyContext->pDrmAppCtx,
                    DRM_CGP_DECRYPTORSETUP,
                    NULL,
                    &cbHeader );

            if ( dr == DRM_E_BUFFERTOOSMALL ) {
                *pData1_size = (uint32_t) cbHeader;
            }
            else {
                result = DRM_Prdy_fail;
            }
        }
        break;
        case DRM_Prdy_getBuffer_licenseAcq_challenge:
        {
            //DRM_DWORD cbChallenge = 0;
            //DRM_DWORD cchURL = 0;
            DRM_CHAR *pszCustomDataUsed = NULL;
            DRM_DWORD cchCustomDataUsed = 0;
            const DRM_CONST_STRING *rgstrRights[ 1 ] = { &g_dstrWMDRM_RIGHT_PLAYBACK };
            if ((NULL != pData) && ( dataLen > 0))
            {
                pszCustomDataUsed = (DRM_CHAR *) pData;
                cchCustomDataUsed = (DRM_DWORD) dataLen;
            }

            dr = Drm_LicenseAcq_GenerateChallenge(
                    pPrdyContext->pDrmAppCtx,
                    rgstrRights,
                    sizeof(rgstrRights)/sizeof(DRM_CONST_STRING*), /*1,*/
                    NULL,
                    pszCustomDataUsed,
                    cchCustomDataUsed,
                    NULL,
                    pData1_size,
                    NULL,
                    NULL,
                    NULL,
                    pData2_size);

            if ( dr != DRM_E_BUFFERTOOSMALL ) {
                result = DRM_Prdy_fail;
            }
        }
        break;

        case DRM_Prdy_getBuffer_client_info:
        {
            DRM_DWORD  cch_ci = 0;
            BDBG_ASSERT(pData1_size != NULL);
            dr = Drm_Device_GetProperty(
                    pPrdyContext->pDrmAppCtx,
                    DRM_DGP_CLIENT_INFO,
                    NULL,
                    &cch_ci);

            if ( dr == DRM_E_BUFFERTOOSMALL ) {
                *pData1_size = (uint32_t) cch_ci;
            }
            else {
                BDBG_ERR(("%s: Drm_Device_GetProperty [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
                result = DRM_Prdy_fail;
            }
        }
        break;

        case DRM_Prdy_getBuffer_Additional_Response_Data_Custom_Data:
        {
            DRM_DWORD  cchData = 0;
            BDBG_ASSERT(pData1_size != NULL);

            dr = Drm_GetAdditionalResponseData(pPrdyContext->pDrmAppCtx,
                            pData,
                            dataLen,
                            DRM_GARD_CUSTOM_DATA,
                            NULL,
                            &cchData );
            if ( dr == DRM_E_BUFFERTOOSMALL ) {
                *pData1_size = (uint32_t) cchData;
            }
            else if ( dr == DRM_E_XMLNOTFOUND ) {
                result = DRM_Prdy_xml_not_found;
            }
            else {
                result = DRM_Prdy_fail;
            }
        }
        break;
        case DRM_Prdy_getBuffer_Additional_Response_Data_Redirect_Url:
        {
            DRM_DWORD  cchData = 0;
            BDBG_ASSERT(pData1_size != NULL);

            dr = Drm_GetAdditionalResponseData(pPrdyContext->pDrmAppCtx,
                            pData,
                            dataLen,
                            DRM_GARD_REDIRECT_URL,
                            NULL,
                            &cchData );
            if ( dr == DRM_E_BUFFERTOOSMALL ) {
                *pData1_size = (uint32_t) cchData;
            }
            else if ( dr == DRM_E_XMLNOTFOUND ) {
                result = DRM_Prdy_xml_not_found;
            }
            else {
                result = DRM_Prdy_fail;
            }
        }
        break;
        case DRM_Prdy_getBuffer_Additional_Response_Data_Service_Id:
        {
            DRM_DWORD  cchData = 0;
            BDBG_ASSERT(pData1_size != NULL);

            dr = Drm_GetAdditionalResponseData(pPrdyContext->pDrmAppCtx,
                            pData,
                            dataLen,
                            DRM_GARD_SERVICE_ID,
                            NULL,
                            &cchData );
            if ( dr == DRM_E_BUFFERTOOSMALL ) {
                *pData1_size = (uint32_t) cchData;
            }
            else if ( dr == DRM_E_XMLNOTFOUND ) {
                result = DRM_Prdy_xml_not_found;
            }
            else {
                result = DRM_Prdy_fail;
            }
        }
        break;
        case DRM_Prdy_getBuffer_Additional_Response_Data_Account_Id:
        {
            DRM_DWORD  cchData = 0;
            BDBG_ASSERT(pData1_size != NULL);

            dr = Drm_GetAdditionalResponseData(pPrdyContext->pDrmAppCtx,
                            pData,
                            dataLen,
                            DRM_GARD_ACCOUNT_ID,
                            NULL,
                            &cchData );
            if ( dr == DRM_E_BUFFERTOOSMALL ) {
                *pData1_size = (uint32_t) cchData;
            }
            else if ( dr == DRM_E_XMLNOTFOUND ) {
                result = DRM_Prdy_xml_not_found;
            }
            else {
                result = DRM_Prdy_fail;
            }
        }
        break;

        case DRM_Prdy_getBuffer_licenseAcq_ack_challenge:
        {
            dr = Drm_LicenseAcq_GenerateAck(
                pPrdyContext->pDrmAppCtx,
                (DRM_LICENSE_RESPONSE *)pData,
                NULL,
                pData1_size);
            if ( dr != DRM_E_BUFFERTOOSMALL ) {
                result = DRM_Prdy_fail;
            }
        }
        break;

        case DRM_Prdy_getBuffer_licenseAcq_challenge_Netflix:
        {
            //DRM_DWORD cbChallenge = 0;
            //DRM_DWORD cchURL = 0;
            DRM_CHAR *pszCustomDataUsed = NULL;
            DRM_DWORD cchCustomDataUsed = 0;
            DRM_BYTE tmp_nonce[DRM_PRDY_SESSION_ID_LEN];
            const DRM_CONST_STRING *rgstrRights[ 1 ] = { &g_dstrWMDRM_RIGHT_PLAYBACK };
            if ((NULL != pData) && ( dataLen > 0))
            {
                pszCustomDataUsed = (DRM_CHAR *) pData;
                cchCustomDataUsed = (DRM_DWORD) dataLen;
            }

            dr = Drm_LicenseAcq_GenerateChallenge_Netflix(
                    pPrdyContext->pDrmAppCtx,
                    rgstrRights,
                    sizeof(rgstrRights)/sizeof(DRM_CONST_STRING*), /*1,*/
                    NULL,
                    pszCustomDataUsed,
                    cchCustomDataUsed,
                    NULL,
                    pData1_size,
                    NULL,
                    NULL,
                    NULL,
                    pData2_size,
                    tmp_nonce,
                    false);

            if ( dr != DRM_E_BUFFERTOOSMALL ) {
                result = DRM_Prdy_fail;
            }
        }
        break;

        case DRM_Prdy_getBuffer_licenseAcq_challenge_NetflixWithLDL:
        {
            //DRM_DWORD cbChallenge = 0;
            //DRM_DWORD cchURL = 0;
            DRM_CHAR *pszCustomDataUsed = NULL;
            DRM_DWORD cchCustomDataUsed = 0;
            DRM_BYTE tmp_nonce[DRM_PRDY_SESSION_ID_LEN];
            const DRM_CONST_STRING *rgstrRights[ 1 ] = { &g_dstrWMDRM_RIGHT_PLAYBACK };
            if ((NULL != pData) && ( dataLen > 0))
            {
                pszCustomDataUsed = (DRM_CHAR *) pData;
                cchCustomDataUsed = (DRM_DWORD) dataLen;
            }

            dr = Drm_LicenseAcq_GenerateChallenge_Netflix(
                    pPrdyContext->pDrmAppCtx,
                    rgstrRights,
                    sizeof(rgstrRights)/sizeof(DRM_CONST_STRING*), /*1,*/
                    NULL,
                    pszCustomDataUsed,
                    cchCustomDataUsed,
                    NULL,
                    pData1_size,
                    NULL,
                    NULL,
                    NULL,
                    pData2_size,
                    tmp_nonce,
                    true);

            if ( dr != DRM_E_BUFFERTOOSMALL ) {
                result = DRM_Prdy_fail;
            }
        }
        break;

        default:
            BDBG_ERR(("%s: Unknown buffer type %d \n",__FUNCTION__,chType));
            result = DRM_Prdy_fail;

    } /* end switch */

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return result;
}

DRM_Prdy_Error_e DRM_Prdy_Content_GetProperty(
        DRM_Prdy_Handle_t                pPrdyContext,
        DRM_Prdy_ContentGetProperty_e    propertyType,
        uint8_t                         *pData,
        uint32_t                         dataSize)
{
    DRM_RESULT        dr = DRM_SUCCESS;
    DRM_Prdy_Error_e  result = DRM_Prdy_ok;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);

    dr = Drm_Content_GetProperty(
            pPrdyContext->pDrmAppCtx,
            (DRM_CONTENT_GET_PROPERTY) propertyType,
            (DRM_BYTE *)pData,
            (DRM_DWORD *) &dataSize);

    if(dr != DRM_SUCCESS){
        if(dr == DRM_E_CH_INVALID_HEADER) {
            result = DRM_Prdy_invalid_header;
        }
        else if (dr == DRM_E_XMLNOTFOUND) {
            result = DRM_Prdy_xml_not_found;
        }
        else {
            result = DRM_Prdy_fail;
        }
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return result;
}

DRM_Prdy_Error_e DRM_Prdy_Content_SetProperty(
        DRM_Prdy_Handle_t                pPrdyContext,
        DRM_Prdy_ContentSetProperty_e    propertyType,
        const uint8_t                   *pData,
        uint32_t                         dataSize)
{
    DRM_RESULT        dr = DRM_SUCCESS;
    DRM_Prdy_Error_e  result = DRM_Prdy_ok;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    dr = Drm_Content_SetProperty(
            pPrdyContext->pDrmAppCtx,
            (DRM_CONTENT_SET_PROPERTY) propertyType,
            (const DRM_BYTE *)pData,
            (DRM_DWORD)dataSize);

    if(dr == DRM_E_HEADER_ALREADY_SET)
    {
        Drm_Reinitialize(pPrdyContext->pDrmAppCtx);
        dr =Drm_Content_SetProperty( pPrdyContext->pDrmAppCtx,
                                     (DRM_CONTENT_SET_PROPERTY) propertyType,
                                     (const DRM_BYTE *)pData,
                                     (DRM_DWORD)dataSize);
    }

    if( dr != DRM_SUCCESS)
    {
        result = DRM_Prdy_fail;
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return result;
}

DRM_Prdy_Error_e DRM_Prdy_Get_Protection_Policy(
        DRM_Prdy_Handle_t      pPrdyContext,
        DRM_Prdy_policy_t     *pPolicy)
{
    BDBG_MSG(("%s - entering", __FUNCTION__));
    BDBG_ASSERT(pPrdyContext != NULL);
    DRM_Prdy_Error_e dr = DRM_Prdy_ok;

    if( pPolicy == NULL)
    {
        BDBG_ERR(("%s: pPolicy is NULL, xxiting\n", __FUNCTION__));
        return DRM_Prdy_fail;
    };

    if(pPrdyContext->nbOfPolicyQueued == 0) {
        BDBG_MSG(("%s: no policy found\n", __FUNCTION__));
        dr = DRM_Prdy_no_policy;
    }
    else {
        BDBG_MSG(("%s: policy found\n", __FUNCTION__));
        pPrdyContext->nbOfPolicyQueued--;
        BKNI_Memcpy(pPolicy, &pPrdyContext->protectionPolicy[pPrdyContext->nbOfPolicyQueued], sizeof(DRM_Prdy_policy_t));
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return dr;
}

void DRM_Prdy_GetDefaultDecryptSettings(
        DRM_Prdy_DecryptSettings_t  *pSettings
        )
{
    if( pSettings != NULL)
    {
        BKNI_Memset(pSettings, 0, sizeof(DRM_Prdy_DecryptSettings_t));
        pSettings->opType = NEXUS_SecurityOperation_eDecrypt;
        pSettings->algType = NEXUS_SecurityAlgorithm_eAes128;
        pSettings->algVariant = NEXUS_SecurityAlgorithmVariant_eCounter;
        pSettings->termMode = NEXUS_SecurityTerminationMode_eClear;
        pSettings->enableExtKey = true;
        pSettings->enableExtIv = true;
        pSettings->aesCounterSize = NEXUS_SecurityAesCounterSize_e64Bits;
        pSettings->keySlotType = NEXUS_SecurityKeyType_eOdd;
        pSettings->aesCounterMode = NEXUS_SecurityCounterMode_ePartialBlockInNextPacket;
    }
}

static DRM_Prdy_Error_e Drm_Prdy_AllocateDecryptContextKeySlot (
   DRM_Prdy_DecryptContextKey_t  **pDecryptContextKey
   )
{
    NEXUS_Error                    rc = NEXUS_SUCCESS;
    DRM_RESULT                     dr = DRM_SUCCESS;
    NEXUS_SecurityKeySlotSettings  keySlotSettings;
    NEXUS_MemoryAllocationSettings allocSettings;
    NEXUS_ClientConfiguration      platformConfig;

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    NEXUS_Platform_GetClientConfiguration(&platformConfig);

    if (platformConfig.heap[NXCLIENT_FULL_HEAP])
    {
        NEXUS_HeapHandle heap = platformConfig.heap[NXCLIENT_FULL_HEAP];
        NEXUS_MemoryStatus heapStatus;
        NEXUS_Heap_GetStatus(heap, &heapStatus);
        if (heapStatus.memoryType & NEXUS_MemoryType_eFull)
        {
            allocSettings.heap = heap;
        }
    }

    rc = NEXUS_Memory_Allocate(sizeof(DRM_Prdy_DecryptContextKey_t), &allocSettings, (void *)(pDecryptContextKey));
    if (rc)
    {
        BDBG_ERR(("%s: Failed to allocate memory %d, exiting...\n", __FUNCTION__,rc));
        dr = DRM_Prdy_fail;
        goto ErrorExit;
    }
    BKNI_Memset((*pDecryptContextKey), 0, sizeof(DRM_Prdy_DecryptContextKey_t));

    /* Allocate key slot */
    NEXUS_Security_GetDefaultKeySlotSettings(&keySlotSettings);
    keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    (*pDecryptContextKey)->keySlot = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
    if((*pDecryptContextKey)->keySlot == NULL)
    {
        BDBG_ERR(("%s: Failed to allocate Key Slot, exiting...\n", __FUNCTION__));
        dr = DRM_Prdy_fail;
        goto ErrorExit;
    }

    (*pDecryptContextKey)->refCount = 1;

ErrorExit:
    return dr;
}

static void Drm_Prdy_FreeDecryptContextKeySlot (
    DRM_Prdy_DecryptContextKey_t  *pDecryptContextKey
    )
{
    if(pDecryptContextKey)
    {
        if(pDecryptContextKey->refCount > 0) pDecryptContextKey->refCount--;

        if(pDecryptContextKey->refCount == 0){
            NEXUS_Security_FreeKeySlot(pDecryptContextKey->keySlot);
            NEXUS_Memory_Free(pDecryptContextKey);
        }
    }
    return;
}


DRM_Prdy_DecryptContext_t * DRM_Prdy_AllocateDecryptContext (
    const DRM_Prdy_DecryptSettings_t  *pSettings
    )
{
    NEXUS_Error                    rc = NEXUS_SUCCESS;
    DRM_RESULT                     dr = DRM_SUCCESS;
    DRM_Prdy_DecryptContext_t     *pDecryptContext;
    CommonCryptoKeyConfigSettings  algSettings;
    NEXUS_MemoryAllocationSettings allocSettings;
    NEXUS_ClientConfiguration      platformConfig;
    CommonCryptoHandle             cryptoHandle=0;
    CommonCryptoSettings           cryptoSettings;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    NEXUS_Platform_GetClientConfiguration(&platformConfig);

    if (platformConfig.heap[NXCLIENT_FULL_HEAP])
    {
        NEXUS_HeapHandle heap = platformConfig.heap[NXCLIENT_FULL_HEAP];
        NEXUS_MemoryStatus heapStatus;
        NEXUS_Heap_GetStatus(heap, &heapStatus);
        if (heapStatus.memoryType & NEXUS_MemoryType_eFull)
        {
            allocSettings.heap = heap;
        }
    }

    rc = NEXUS_Memory_Allocate(sizeof(DRM_Prdy_DecryptContext_t), &allocSettings, (void *)(&pDecryptContext));
    if (rc)
    {
        BDBG_ERR(("%s: Failed to allocate memory %d, exiting...\n", __FUNCTION__,rc));
        goto ErrorExit;
    }
    BKNI_Memset(pDecryptContext, 0, sizeof(*pDecryptContext));

    ChkMem(pDecryptContext->pDecrypt = Oem_MemAlloc(SIZEOF(DRM_DECRYPT_CONTEXT)));
    BKNI_Memset(pDecryptContext->pDecrypt, 0, SIZEOF(DRM_DECRYPT_CONTEXT));

    /* Allocate key slot */
    ChkDR(Drm_Prdy_AllocateDecryptContextKeySlot(&pDecryptContext->pKeyContext));

    CommonCrypto_GetDefaultKeyConfigSettings(&algSettings);
    algSettings.keySlot = pDecryptContext->pKeyContext->keySlot;
    algSettings.settings.opType = pSettings->opType;
    algSettings.settings.algType = pSettings->algType;
    algSettings.settings.algVariant = pSettings->algVariant;
    algSettings.settings.termMode = pSettings->termMode;
    algSettings.settings.enableExtKey = pSettings->enableExtKey;
    algSettings.settings.enableExtIv = pSettings->enableExtIv;
    algSettings.settings.aesCounterSize = pSettings->aesCounterSize;
    algSettings.settings.keySlotType = pSettings->keySlotType;
    algSettings.settings.aesCounterMode = pSettings->aesCounterMode;

    /* Configure key slot */
    CommonCrypto_GetDefaultSettings(&cryptoSettings);
    cryptoHandle = CommonCrypto_Open(&cryptoSettings);

    if(CommonCrypto_LoadKeyConfig(cryptoHandle, &algSettings) != NEXUS_SUCCESS)
    {
        BDBG_ERR(("%s - CommonCrypto_ConfigAlg failed aes ctr\n", __FUNCTION__));
        goto ErrorExit;
    }

    CommonCrypto_Close(cryptoHandle);
    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return pDecryptContext;

ErrorExit:
    if(cryptoHandle) CommonCrypto_Close(cryptoHandle);
    if(pDecryptContext->pKeyContext) Drm_Prdy_FreeDecryptContextKeySlot(pDecryptContext->pKeyContext);
    if(pDecryptContext->pDecrypt) Oem_MemFree(pDecryptContext->pDecrypt);

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return NULL;
}

DRM_Prdy_Error_e  DRM_Prdy_SetDecryptContext (
        const DRM_Prdy_DecryptSettings_t  *pSettings,
        DRM_Prdy_DecryptContext_t         *pDecryptContext
        )
{
    DRM_RESULT                     dr = DRM_Prdy_ok;
    CommonCryptoKeyConfigSettings  algSettings;
    CommonCryptoHandle             cryptoHandle=0;
    CommonCryptoSettings           cryptoSettings;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    if( pDecryptContext == NULL )
    {
        BDBG_ERR(("%s: Decrypt Context is NULL, exiting...\n", __FUNCTION__));
        goto ErrorExit;
    }

    if( pDecryptContext->pKeyContext != NULL )
    {
        BDBG_ERR(("%s: Decrypt Context key slot is already allocated...\n", __FUNCTION__));
        goto ErrorExit;
    }

    /* Allocate key slot */
    ChkDR(Drm_Prdy_AllocateDecryptContextKeySlot (&pDecryptContext->pKeyContext));

    CommonCrypto_GetDefaultKeyConfigSettings(&algSettings);
    algSettings.keySlot = pDecryptContext->pKeyContext->keySlot;
    algSettings.settings.opType = pSettings->opType;
    algSettings.settings.algType = pSettings->algType;
    algSettings.settings.algVariant = pSettings->algVariant;
    algSettings.settings.termMode = pSettings->termMode;
    algSettings.settings.enableExtKey = pSettings->enableExtKey;
    algSettings.settings.enableExtIv = pSettings->enableExtIv;
    algSettings.settings.aesCounterSize = pSettings->aesCounterSize;
    algSettings.settings.keySlotType = pSettings->keySlotType;
    algSettings.settings.aesCounterMode = pSettings->aesCounterMode;

    /* Configure key slot */
    CommonCrypto_GetDefaultSettings(&cryptoSettings);
    cryptoHandle = CommonCrypto_Open(&cryptoSettings);

    if(CommonCrypto_LoadKeyConfig(cryptoHandle, &algSettings) != NEXUS_SUCCESS)
    {
        BDBG_ERR(("%s - CommonCrypto_ConfigAlg failed aes ctr\n", __FUNCTION__));
        goto ErrorExit;
    }

    CommonCrypto_Close(cryptoHandle);
    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    if(cryptoHandle) CommonCrypto_Close(cryptoHandle);
    if(pDecryptContext->pKeyContext) Drm_Prdy_FreeDecryptContextKeySlot(pDecryptContext->pKeyContext);

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_fail;
}

void DRM_Prdy_FreeDecryptContext (
   DRM_Prdy_DecryptContext_t  *pDecryptContext
   )
{
    if(pDecryptContext )
    {
        if(pDecryptContext->pKeyContext) Drm_Prdy_FreeDecryptContextKeySlot(pDecryptContext->pKeyContext);
        if(pDecryptContext->pDecrypt) SAFE_OEM_FREE(pDecryptContext->pDecrypt);
        NEXUS_Memory_Free(pDecryptContext);
    }
    return;
}



DRM_Prdy_Error_e DRM_Prdy_Reader_Bind(
        DRM_Prdy_Handle_t          pPrdyContext,
        DRM_Prdy_DecryptContext_t  *pDecryptContext)
{
    DRM_RESULT              dr = DRM_SUCCESS;
    DRM_Prdy_Error_e        result = DRM_Prdy_ok;
    uint8_t                *pbNewOpaqueBuffer = NULL;
    const DRM_CONST_STRING *rgstrRights[ 1 ] = { &g_dstrWMDRM_RIGHT_PLAYBACK };

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pDecryptContext != NULL);

    if(pDecryptContext->pDecrypt == NULL){
        ChkMem(pDecryptContext->pDecrypt = Oem_MemAlloc(SIZEOF(DRM_DECRYPT_CONTEXT)));
        BKNI_Memset(pDecryptContext->pDecrypt, 0, SIZEOF(DRM_DECRYPT_CONTEXT));
    }

    while( (dr = Drm_Reader_Bind(
                    pPrdyContext->pDrmAppCtx,
                    rgstrRights,
                    1,
                    (DRMPFNPOLICYCALLBACK)DRM_Prdy_policy_callback,
                    (void *) pPrdyContext,
                    (DRM_DECRYPT_CONTEXT *) pDecryptContext->pDecrypt)) == DRM_E_BUFFERTOOSMALL)
    {
        uint32_t cbNewOpaqueBuffer = pPrdyContext->cbOpaqueBuffer * 2;
        BDBG_ASSERT( cbNewOpaqueBuffer > pPrdyContext->cbOpaqueBuffer ); /* overflow check */

        if( cbNewOpaqueBuffer > DRM_MAXIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE )
        {
            ChkDR( DRM_E_OUTOFMEMORY );
        }

        ChkMem( pbNewOpaqueBuffer = ( uint8_t* )Oem_MemAlloc( cbNewOpaqueBuffer ) );

        ChkDR( Drm_ResizeOpaqueBuffer(
                    pPrdyContext->pDrmAppCtx,
                    pbNewOpaqueBuffer,
                    cbNewOpaqueBuffer ) );

        /*
         Free the old buffer and then transfer the new buffer ownership
         Free must happen after Drm_ResizeOpaqueBuffer because that
         function assumes the existing buffer is still valid
        */
        SAFE_OEM_FREE( pPrdyContext->pbOpaqueBuffer );
        pPrdyContext->cbOpaqueBuffer = cbNewOpaqueBuffer;
        pPrdyContext->pbOpaqueBuffer = pbNewOpaqueBuffer;
        pbNewOpaqueBuffer = NULL;
    }

    if (DRM_FAILED( dr )) {
        if (dr == DRM_E_LICENSE_NOT_FOUND) {
            /* could not find a license for the KID */
            BDBG_ERR(("%s: no licenses found in the license store. Please request one from the license server.\n", __FUNCTION__));
            result = DRM_Prdy_license_not_found;
        }
        else if(dr == DRM_E_LICENSE_EXPIRED) {
            /* License is expired */
            BDBG_ERR(("%s: License expired. Please request one from the license server.\n", __FUNCTION__));
            result = DRM_Prdy_license_expired;
        }
        else if(  dr == DRM_E_RIV_TOO_SMALL ||
                  dr == DRM_E_LICEVAL_REQUIRED_REVOCATION_LIST_NOT_AVAILABLE )
        {
            /* Revocation Package must be update */
            BDBG_ERR(("%s: Revocation Package must be update. 0x%x\n", __FUNCTION__,(unsigned int)dr));
            result = DRM_Prdy_revocation_package_expired;
        }
        else {
            BDBG_ERR(("%s: unexpected failure during bind. 0x%x\n", __FUNCTION__,(unsigned int)dr));
            result = DRM_Prdy_fail;
        }
    }

   BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
   return result;

ErrorExit:
   BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
   if( pDecryptContext->pDecrypt != NULL) {
       SAFE_OEM_FREE(pDecryptContext->pDecrypt);
   }
   if( pbNewOpaqueBuffer != NULL) {
       SAFE_OEM_FREE(pbNewOpaqueBuffer);
   }

   return DRM_Prdy_fail;
}

/***********************************************************************************
 * Function: DRM_Prdy_Reader_Bind_Netflix()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Reader_Bind_Netflix(
        DRM_Prdy_Handle_t          pPrdyContext,
        uint8_t                    *pSessionID,
        DRM_Prdy_DecryptContext_t  *pDecryptContext)
{
    DRM_RESULT              dr = DRM_SUCCESS;
    DRM_Prdy_Error_e        result = DRM_Prdy_ok;
    uint8_t                *pbNewOpaqueBuffer = NULL;
    const DRM_CONST_STRING *rgstrRights[ 1 ] = { &g_dstrWMDRM_RIGHT_PLAYBACK };

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pDecryptContext != NULL);

    if(pDecryptContext->pDecrypt == NULL){
        ChkMem(pDecryptContext->pDecrypt = Oem_MemAlloc(SIZEOF(DRM_DECRYPT_CONTEXT)));
        BKNI_Memset(pDecryptContext->pDecrypt, 0, SIZEOF(DRM_DECRYPT_CONTEXT));
    }

    while( (dr = Drm_Reader_Bind_Netflix(
                    pPrdyContext->pDrmAppCtx,
                    rgstrRights,
                    1,
                    (DRMPFNPOLICYCALLBACK)DRM_Prdy_policy_callback,
                    (void *) pPrdyContext,
                    (DRM_BYTE *) pSessionID,
                    (DRM_DECRYPT_CONTEXT *) pDecryptContext->pDecrypt)) == DRM_E_BUFFERTOOSMALL)
    {
        uint32_t cbNewOpaqueBuffer = pPrdyContext->cbOpaqueBuffer * 2;
        BDBG_ASSERT( cbNewOpaqueBuffer > pPrdyContext->cbOpaqueBuffer ); /* overflow check */

        if( cbNewOpaqueBuffer > DRM_MAXIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE )
        {
            ChkDR( DRM_E_OUTOFMEMORY );
        }

        ChkMem( pbNewOpaqueBuffer = ( uint8_t* )Oem_MemAlloc( cbNewOpaqueBuffer ) );

        ChkDR( Drm_ResizeOpaqueBuffer(
                    pPrdyContext->pDrmAppCtx,
                    pbNewOpaqueBuffer,
                    cbNewOpaqueBuffer ) );

        /*
         Free the old buffer and then transfer the new buffer ownership
         Free must happen after Drm_ResizeOpaqueBuffer because that
         function assumes the existing buffer is still valid
        */
        SAFE_OEM_FREE( pPrdyContext->pbOpaqueBuffer );
        pPrdyContext->cbOpaqueBuffer = cbNewOpaqueBuffer;
        pPrdyContext->pbOpaqueBuffer = pbNewOpaqueBuffer;
        pbNewOpaqueBuffer = NULL;
    }

    if (DRM_FAILED( dr )) {
        if (dr == DRM_E_LICENSE_NOT_FOUND) {
            /* could not find a license for the KID */
            BDBG_ERR(("%s: no licenses found in the license store. Please request one from the license server.\n", __FUNCTION__));
            result = DRM_Prdy_license_not_found;
        }
        else if(dr == DRM_E_LICENSE_EXPIRED) {
            /* License is expired */
            BDBG_ERR(("%s: License expired. Please request one from the license server.\n", __FUNCTION__));
            result = DRM_Prdy_license_expired;
        }
        else if(  dr == DRM_E_RIV_TOO_SMALL ||
                  dr == DRM_E_LICEVAL_REQUIRED_REVOCATION_LIST_NOT_AVAILABLE )
        {
            /* Revocation Package must be update */
            BDBG_ERR(("%s: Revocation Package must be update. 0x%x\n", __FUNCTION__,(unsigned int)dr));
            result = DRM_Prdy_revocation_package_expired;
        }
        else {
            BDBG_ERR(("%s: unexpected failure during bind. 0x%x\n", __FUNCTION__,(unsigned int)dr));
            result = DRM_Prdy_fail;
        }
    }

   BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
   return result;

ErrorExit:
   BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
   if( pDecryptContext->pDecrypt != NULL) {
       SAFE_OEM_FREE(pDecryptContext->pDecrypt);
   }
   if( pbNewOpaqueBuffer != NULL) {
       SAFE_OEM_FREE(pbNewOpaqueBuffer);
   }

   return DRM_Prdy_fail;
}
#ifndef CMD_DRM_PLAYREADY_SAGE_IMPL
/***********************************************************************************
 * Function: DRM_Prdy_Reader_Bind_Netflix_WCK()
 *
 * Enables protection of the content key.
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Reader_Bind_Netflix_WCK(
        DRM_Prdy_Handle_t          pPrdyContext,
        uint8_t					   *pSessionID,
        DRM_Prdy_DecryptContext_t  *pDecryptContext)
{
    DRM_RESULT              dr = DRM_SUCCESS;
    DRM_Prdy_Error_e        result = DRM_Prdy_ok;
    uint8_t                *pbNewOpaqueBuffer = NULL;
    const DRM_CONST_STRING *rgstrRights[ 1 ] = { &g_dstrWMDRM_RIGHT_PLAYBACK };

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pDecryptContext != NULL);

    if(pDecryptContext->pDecrypt == NULL){
        ChkMem(pDecryptContext->pDecrypt = Oem_MemAlloc(SIZEOF(DRM_DECRYPT_CONTEXT)));
        BKNI_Memset(pDecryptContext->pDecrypt, 0, SIZEOF(DRM_DECRYPT_CONTEXT));
    }

    while( (dr = Drm_Reader_Bind_Netflix_WCK(
                    pPrdyContext->pDrmAppCtx,
                    rgstrRights,
                    1,
                    (DRMPFNPOLICYCALLBACK)DRM_Prdy_policy_callback,
                    (void *) pPrdyContext,
                    (DRM_BYTE *) pSessionID,
                    (DRM_DECRYPT_CONTEXT *) pDecryptContext->pDecrypt)) == DRM_E_BUFFERTOOSMALL)
    {

        uint32_t cbNewOpaqueBuffer = pPrdyContext->cbOpaqueBuffer * 2;
        BDBG_ASSERT( cbNewOpaqueBuffer > pPrdyContext->cbOpaqueBuffer ); /* overflow check */

        if( cbNewOpaqueBuffer > DRM_MAXIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE )
        {
            ChkDR( DRM_E_OUTOFMEMORY );
        }

        ChkMem( pbNewOpaqueBuffer = ( uint8_t* )Oem_MemAlloc( cbNewOpaqueBuffer ) );

        ChkDR( Drm_ResizeOpaqueBuffer(
                    pPrdyContext->pDrmAppCtx,
                    pbNewOpaqueBuffer,
                    cbNewOpaqueBuffer ) );

        /*
         Free the old buffer and then transfer the new buffer ownership
         Free must happen after Drm_ResizeOpaqueBuffer because that
         function assumes the existing buffer is still valid
        */
        SAFE_OEM_FREE( pPrdyContext->pbOpaqueBuffer );
        pPrdyContext->cbOpaqueBuffer = cbNewOpaqueBuffer;
        pPrdyContext->pbOpaqueBuffer = pbNewOpaqueBuffer;
        pbNewOpaqueBuffer = NULL;
    }

    if (DRM_FAILED( dr )) {
        if (dr == DRM_E_LICENSE_NOT_FOUND) {
            /* could not find a license for the KID */
            BDBG_ERR(("%s: no licenses found in the license store. Please request one from the license server.\n", __FUNCTION__));
            result = DRM_Prdy_license_not_found;
        }
        else if(dr == DRM_E_LICENSE_EXPIRED) {
            /* License is expired */
            BDBG_ERR(("%s: License expired. Please request one from the license server.\n", __FUNCTION__));
            result = DRM_Prdy_license_expired;
        }
        else if(  dr == DRM_E_RIV_TOO_SMALL ||
                  dr == DRM_E_LICEVAL_REQUIRED_REVOCATION_LIST_NOT_AVAILABLE )
        {
            /* Revocation Package must be update */
            BDBG_ERR(("%s: Revocation Package must be update. 0x%x\n", __FUNCTION__,(unsigned int)dr));
            result = DRM_Prdy_revocation_package_expired;
        }
        else {
            BDBG_ERR(("%s: unexpected failure during bind. 0x%x\n", __FUNCTION__,(unsigned int)dr));
            result = DRM_Prdy_fail;
        }
    }

   BDBG_MSG(("%s: Exiting with success\n", __FUNCTION__));
   return result;

ErrorExit:
   BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
   if( pDecryptContext->pDecrypt != NULL) {
       SAFE_OEM_FREE(pDecryptContext->pDecrypt);
   }
   if( pbNewOpaqueBuffer != NULL) {
       SAFE_OEM_FREE(pbNewOpaqueBuffer);
   }

   return DRM_Prdy_fail;
}
#endif
DRM_Prdy_Error_e DRM_Prdy_Reader_Commit(
        DRM_Prdy_Handle_t      pPrdyContext)
{
    DRM_RESULT   dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);

    ChkDR( Drm_Reader_Commit( pPrdyContext->pDrmAppCtx, NULL, NULL ) );

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Reader Commit failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_Reader_Decrypt(
        DRM_Prdy_DecryptContext_t        *pDecryptContext,
        DRM_Prdy_AES_CTR_Info_t          *pAesCtrInfo,
        uint8_t                          *pBuf,
        uint32_t                          dataSize)
{
    DRM_RESULT     dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pDecryptContext != NULL);
    BDBG_ASSERT(pDecryptContext->pDecrypt != NULL);
    BDBG_ASSERT(pAesCtrInfo != NULL);
    BDBG_ASSERT(pBuf != NULL);

    if(pDecryptContext->pDecrypt == NULL)
    {
        BDBG_ERR(("%s: Invalid Decryptor.\n", __FUNCTION__));
        goto ErrorExit;
    }
    ChkDR( Drm_Reader_Decrypt(
                    pDecryptContext->pDecrypt,
                    (DRM_AES_COUNTER_MODE_CONTEXT *) pAesCtrInfo,
                    pBuf,
                    dataSize ) );

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:

    BDBG_ERR(("%s: Reader Decrypt failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_Reader_DecryptOpaque(
        DRM_Prdy_DecryptContext_t        *pDecryptContext,
        DRM_Prdy_AES_CTR_Info_t          *pAesCtrInfo,
        void *                            pScatterGatherList,
        uint32_t                          nelem)
{
    DRM_RESULT     dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pDecryptContext != NULL);
    BDBG_ASSERT(pDecryptContext->pDecrypt != NULL);
    BDBG_ASSERT(pAesCtrInfo != NULL);
    BDBG_ASSERT(pScatterGatherList != NULL);

    if(pDecryptContext->pDecrypt == NULL)
    {
        BDBG_ERR(("%s: Invalid Decryptor.\n", __FUNCTION__));
        goto ErrorExit;
    }
    ChkDR( Drm_Reader_DecryptOpaque(
                    pDecryptContext->pDecrypt,
                    (DRM_AES_COUNTER_MODE_CONTEXT *) pAesCtrInfo,
                    (OEM_OPAQUE_BUFFER_HANDLE)pScatterGatherList,
                    (OEM_OPAQUE_BUFFER_HANDLE)pScatterGatherList,
                    nelem ) );
    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:

    BDBG_ERR(("%s: Reader Decrypt Opaque failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_Reader_CloneDecryptContext(
    DRM_Prdy_DecryptContext_t  *pDecryptContext,
    DRM_Prdy_DecryptContext_t  *pClonedDecryptContext
    )
{
    DRM_RESULT     dr = DRM_SUCCESS;

    BDBG_ASSERT(pDecryptContext  != NULL);
    BDBG_ASSERT(pDecryptContext->pDecrypt != NULL);
    BDBG_ASSERT(pClonedDecryptContext != NULL);

    /* Clean up pre-existing nexus keys. Drm_Reader_CloneDecryptContext() will get ride of pre-existing DRM_DECRYPT_CONTEXT */
    if(pClonedDecryptContext->pKeyContext){
        Drm_Prdy_FreeDecryptContextKeySlot(pClonedDecryptContext->pKeyContext);

        if(pDecryptContext->pKeyContext != NULL){
            pClonedDecryptContext->pKeyContext = pDecryptContext->pKeyContext;
            pClonedDecryptContext->pKeyContext->refCount++;
        }
    }

    ChkDR( Drm_Reader_CloneDecryptContext(pDecryptContext->pDecrypt, pClonedDecryptContext->pDecrypt));


    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:

    BDBG_ERR(("%s: DRM_Prdy_Reader_CloneDecryptContext failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_Reader_Close( DRM_Prdy_DecryptContext_t  *pDecryptContext)
{
    BDBG_MSG(("%s - entering", __FUNCTION__));
    BDBG_ASSERT(pDecryptContext != NULL);

    if( pDecryptContext->pDecrypt != NULL) {
        Drm_Reader_Close( (DRM_DECRYPT_CONTEXT *) pDecryptContext->pDecrypt );

        if(pDecryptContext->pKeyContext) Drm_Prdy_FreeDecryptContextKeySlot(pDecryptContext->pKeyContext);

        SAFE_OEM_FREE(pDecryptContext->pDecrypt);
        pDecryptContext->pDecrypt = NULL;
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;
}

DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_GenerateChallenge(
        DRM_Prdy_Handle_t      pPrdyContext,
        const char            *pCustomData, /* [in] - can be NULL */
        uint32_t               customDataLen,
        char                  *pCh_url,     /* [out] */
        uint32_t              *pUrl_len,
        char                  *pCh_data,    /* [out] */
        uint32_t              *pCh_len)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_BYTE *pbChallenge = (DRM_BYTE *) pCh_data;
    DRM_CHAR *pchURL = pCh_url;
    const DRM_CONST_STRING *rgstrRights[ 1 ] = { &g_dstrWMDRM_RIGHT_PLAYBACK };

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pCh_len != NULL);

    ChkDR( Drm_LicenseAcq_GenerateChallenge(
                pPrdyContext->pDrmAppCtx,
                rgstrRights,
                1,
                NULL,
                pCustomData,
                customDataLen,
                pchURL,
                pUrl_len, /*(pUrl_len>0)?&cchURL:NULL, */
                NULL,
                NULL,
                pbChallenge,
                pCh_len));

    if(dr != DRM_SUCCESS) {
       if(dr == DRM_E_BUFFERTOOSMALL) {
           BDBG_MSG(("%s - The given buffer size is too small.", __FUNCTION__));
           return DRM_Prdy_buffer_size;
       }
       else {
           goto ErrorExit;
       }
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: License generation failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

/***********************************************************************************
 * Function: DRM_Prdy_LicenseAcq_GenerateChallenge_Netflix()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_GenerateChallenge_Netflix(
        DRM_Prdy_Handle_t      pPrdyContext,
        const char            *pCustomData, /* [in] - can be NULL */
        uint32_t               customDataLen,
        char                  *pCh_url,     /* [out] */
        uint32_t              *pUrl_len,
        char                  *pCh_data,    /* [out] */
        uint32_t              *pCh_len,
        uint8_t               *pNonce,      /* [out] */
        bool                   isLDL)       /* [in] */
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_BYTE *pbChallenge = (DRM_BYTE *) pCh_data;
    DRM_CHAR *pchURL = pCh_url;
    const DRM_CONST_STRING *rgstrRights[ 1 ] = { &g_dstrWMDRM_RIGHT_PLAYBACK };

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pCh_len != NULL);
    BDBG_ASSERT(pNonce != NULL);    /* **FixMe** add this for safety now; though it might actually be an optional parameter at this level. */

    ChkDR( Drm_LicenseAcq_GenerateChallenge_Netflix(
                pPrdyContext->pDrmAppCtx,
                rgstrRights,
                1,
                NULL,
                pCustomData,
                customDataLen,
                pchURL,
                pUrl_len, /*(pUrl_len>0)?&cchURL:NULL, */
                NULL,
                NULL,
                pbChallenge,
                pCh_len,
                pNonce,
                isLDL) );

    if(dr != DRM_SUCCESS) {
       if(dr == DRM_E_BUFFERTOOSMALL) {
           BDBG_MSG(("%s - The given buffer size is too small.", __FUNCTION__));
           return DRM_Prdy_buffer_size;
       }
       else {
           goto ErrorExit;
       }
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: License generation failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_LicenseAcq_CancelChallenge_Netflix()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_CancelChallenge_Netflix(
        DRM_Prdy_Handle_t      pPrdyContext,
        uint8_t               *pNonce)     /* [in] */
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pNonce != NULL);

    ChkDR( Drm_LicenseAcq_CancelChallenge_Netflix(
                pPrdyContext->pDrmAppCtx,
                pNonce) );

    if(dr != DRM_SUCCESS) {
       goto ErrorExit;
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Operation failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_LicenseAcq_GetLdlSessionsLimit_Netflix()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_GetLdlSessionsLimit_Netflix(
        DRM_Prdy_Handle_t      pPrdyContext,
        uint32_t              *pLdlSessionsLimit)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pLdlSessionsLimit != NULL);

    ChkDR( Drm_LicenseAcq_GetLdlSessionsLimit_Netflix(
                pPrdyContext->pDrmAppCtx,
                pLdlSessionsLimit) );

    if(dr != DRM_SUCCESS) {
       goto ErrorExit;
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Operation failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_LicenseAcq_FlushLdlChallenges_Netflix()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_FlushLdlChallenges_Netflix(
        DRM_Prdy_Handle_t      pPrdyContext)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);

    ChkDR( Drm_LicenseAcq_FlushLdlChallenges_Netflix(
                pPrdyContext->pDrmAppCtx) );

    if(dr != DRM_SUCCESS) {
       goto ErrorExit;
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Operation failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return convertDrmResult(dr);
}

DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_ProcessResponse(
        DRM_Prdy_Handle_t            pPrdyContext,
        const char                  *pData,
        uint32_t                     dataLen,
        DRM_Prdy_License_Response_t *pResponse  /* [out]  can be null */
)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_LICENSE_RESPONSE oResponse;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pData != NULL);

    ZEROMEM( &oResponse, SIZEOF( DRM_LICENSE_RESPONSE ) );

    ChkDR( Drm_LicenseAcq_ProcessResponse(
                pPrdyContext->pDrmAppCtx,
                DRM_PROCESS_LIC_RESPONSE_NO_FLAGS,
                NULL,
                NULL,
                ( uint8_t * )pData,
                dataLen,
                &oResponse ) );

    if(pResponse != NULL)
    {
        BKNI_Memcpy(pResponse, (void*)&oResponse, sizeof(DRM_LICENSE_RESPONSE));
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: License Process Response failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_ProcessResponseNonPersistent(
        DRM_Prdy_Handle_t            pPrdyContext,
        const char                  *pData,
        uint32_t                     dataLen,
        DRM_Prdy_License_Response_t *pResponse  /* [out]  can be null */
)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_LICENSE_RESPONSE oResponse;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pData != NULL);

    ZEROMEM( &oResponse, SIZEOF( DRM_LICENSE_RESPONSE ) );

    ChkDR( Drm_LicenseAcq_ProcessResponse(
                pPrdyContext->pDrmAppCtx,
                DRM_PROCESS_LIC_RESPONSE_SIGNATURE_NOT_REQUIRED,
                NULL,
                NULL,
                ( uint8_t * )pData,
                dataLen,
                &oResponse ) );

    if(pResponse != NULL)
    {
        BKNI_Memcpy(pResponse, (void*)&oResponse, sizeof(DRM_LICENSE_RESPONSE));
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: License Process Response failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_ProcessResponse_SecStop(
        DRM_Prdy_Handle_t            pPrdyContext,
        const char                  *pData,
        uint32_t                     dataLen,
        uint8_t                     *pSessionID,
        DRM_Prdy_License_Response_t *pResponse  /* [out]  can be null */
)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_LICENSE_RESPONSE oResponse;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pData != NULL);

    ZEROMEM( &oResponse, SIZEOF( DRM_LICENSE_RESPONSE ) );

    ChkDR( Drm_LicenseAcq_ProcessResponse_SecStop(
                pPrdyContext->pDrmAppCtx,
                DRM_PROCESS_LIC_RESPONSE_NO_FLAGS,
                NULL,
                NULL,
                ( uint8_t * )pData,
                dataLen,
                ( DRM_BYTE *)pSessionID,
                &oResponse ) );

    if(pResponse != NULL)
    {
        BKNI_Memcpy(pResponse, (void*)&oResponse, sizeof(DRM_LICENSE_RESPONSE));
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: License Process Response failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_ProcessResponse_Netflix(
        DRM_Prdy_Handle_t            pPrdyContext,
        const char                  *pData,
        uint32_t                     dataLen,
        uint8_t                     *pSessionID,
        DRM_Prdy_License_Response_t *pResponse  /* [out]  can be null */
)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_LICENSE_RESPONSE oResponse;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pData != NULL);

    ZEROMEM( &oResponse, SIZEOF( DRM_LICENSE_RESPONSE ) );

    ChkDR( Drm_LicenseAcq_ProcessResponse_Netflix(
                pPrdyContext->pDrmAppCtx,
                DRM_PROCESS_LIC_RESPONSE_NO_FLAGS,
                NULL,
                NULL,
                ( uint8_t * )pData,
                dataLen,
                ( DRM_BYTE *)pSessionID,
                &oResponse ) );

    if(pResponse != NULL)
    {
        BKNI_Memcpy(pResponse, (void*)&oResponse, sizeof(DRM_LICENSE_RESPONSE));
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: License Process Response failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_GenerateAck(
        DRM_Prdy_Handle_t            pPrdyContext,
        DRM_Prdy_License_Response_t *pResponse, /* [in] */
        char                        *pCh_data,    /* [out] */
        uint32_t                    *pCh_len)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pResponse != NULL);
    BDBG_ASSERT(pCh_data != NULL);
    BDBG_ASSERT(pCh_len != NULL);

    ChkDR( Drm_LicenseAcq_GenerateAck(
                pPrdyContext->pDrmAppCtx,
                (DRM_LICENSE_RESPONSE *)pResponse,
                (DRM_BYTE *)pCh_data,
                pCh_len ));

    if(dr != DRM_SUCCESS) {
       if(dr == DRM_E_BUFFERTOOSMALL) {
           BDBG_MSG(("%s - The given buffer size is too small.", __FUNCTION__));
           return DRM_Prdy_buffer_size;
       }
       else {
           goto ErrorExit;
       }
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Generating ACK for license's response failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_LicenseAcq_ProcessAckResponse(
        DRM_Prdy_Handle_t            pPrdyContext,
        const char                  *pData,
        uint32_t                     dataLen
)
{
    DRM_RESULT dr = DRM_SUCCESS;

    DRM_RESULT f_pResult;
    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pData != NULL);

    ChkDR( Drm_LicenseAcq_ProcessAckResponse(
            pPrdyContext->pDrmAppCtx,
            ( DRM_BYTE * )pData,
            dataLen,
            &f_pResult));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: License Acq Process Ack Response failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}



DRM_Prdy_Error_e DRM_Prdy_Device_GetProperty(
        DRM_Prdy_Handle_t      pPrdyContext,
        char                  *pClient_info,     /* [out] */
        uint32_t               pCliInfo_len)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_CHAR *pbClientInfo = (DRM_CHAR *) pClient_info;
    DRM_BYTE *tmpClientInfo = NULL;
    DRM_DWORD cchLen = pCliInfo_len;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pClient_info != NULL);

    ChkMem(tmpClientInfo = (DRM_BYTE*) Oem_MemAlloc(cchLen * SIZEOF (DRM_BYTE)));

    ChkDR(Drm_Device_GetProperty(
                pPrdyContext->pDrmAppCtx,
                DRM_DGP_CLIENT_INFO,
                tmpClientInfo,
                &cchLen));

    /* DRM_DGP_CLIENT_INFO is UNICODE blob which is NOT NULL terminated */
    DRM_UTL_DemoteUNICODEtoASCII((DRM_WCHAR *)tmpClientInfo, pbClientInfo, cchLen);  /* transform from wide-char to ANSI */

    if( tmpClientInfo != NULL) {
        SAFE_OEM_FREE(tmpClientInfo);
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    if( tmpClientInfo != NULL) {
        SAFE_OEM_FREE(tmpClientInfo);
    }
    BDBG_ERR(("%s: Device GetProperty failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_GetAdditionalResponseData(
        DRM_Prdy_Handle_t   pPrdyContext,
        uint8_t            *f_pbResponse,
        uint32_t            f_cbResponse,
        uint32_t            f_dwDataType,
        char               *f_pchDataString,
        uint32_t            f_cbchDataString)
{

    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(f_pbResponse != NULL);
    BDBG_ASSERT(f_pchDataString != NULL);

    ChkDR(Drm_GetAdditionalResponseData(
                pPrdyContext->pDrmAppCtx,
                f_pbResponse,
                f_cbResponse,
                f_dwDataType,
                f_pchDataString,
                &f_cbchDataString ));


    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:

    BDBG_ERR(("%s: Device GetProperty failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_Revocation_StorePackage(
        DRM_Prdy_Handle_t      pPrdyContext,
        char                  *pbRevPackage,
        uint32_t               cbRevPackage)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pbRevPackage != NULL);

    ChkDR( Drm_Revocation_StorePackage(
                pPrdyContext->pDrmAppCtx,
                ( DRM_CHAR * )pbRevPackage,
                cbRevPackage ) );

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Revocation Store Package failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_LocalLicense_InitializePolicyDescriptor(
        DRM_Prdy_local_license_policy_descriptor_t    *pPoDescriptor)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPoDescriptor != NULL);

    ChkDR( Drm_LocalLicense_InitializePolicyDescriptor(
                                 (DRM_LOCAL_LICENSE_POLICY_DESCRIPTOR *) pPoDescriptor) );

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: InitializePolicyDescriptor failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;


}

DRM_Prdy_Error_e DRM_Prdy_LocalLicense_CreateLicense(
        DRM_Prdy_Handle_t                                  pPrdyContext,
        const DRM_Prdy_local_license_policy_descriptor_t  *pPoDescriptor,
        DRM_Prdy_local_license_type_e                      elicenseType,
        const Drm_Prdy_KID_t                              *pKeyId,
        uint16_t                                           cbRemoteCert,
        const uint8_t                                     *pbRemoteCert,
        const DRM_Prdy_license_handle                      hRootLicense,
        const DRM_Prdy_license_handle                     *phLicense)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pPoDescriptor  != NULL);
    BDBG_ASSERT(pKeyId       != NULL);
    BDBG_ASSERT(phLicense    != NULL);

    ChkDR( Drm_LocalLicense_CreateLicense(
                pPrdyContext->pDrmAppCtx,
                ( const DRM_LOCAL_LICENSE_POLICY_DESCRIPTOR * ) pPoDescriptor,
                ( DRM_LOCAL_LICENSE_TYPE ) elicenseType,
                ( const DRM_KID *) pKeyId,
                cbRemoteCert,
                pbRemoteCert,
                ( const DRM_LICENSE_HANDLE ) hRootLicense,
                ( DRM_LICENSE_HANDLE *) phLicense ) );

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Locallicens_CreateLicense failed [0x%X], exiting...\n",__FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_LocalLicense_StoreLicense(
        const DRM_Prdy_license_handle     hLicense,
        DRM_Prdy_local_license_store_e    eLicenseStore)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(hLicense != DRM_PRDY_LICENSE_HANDLE_INVALID );

    ChkDR( Drm_LocalLicense_StoreLicense(
                ( const DRM_LICENSE_HANDLE ) hLicense,
                ( const DRM_LOCAL_LICENSE_STORE ) eLicenseStore ) );

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: LocalLicense_StoreLicense failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;

}

DRM_Prdy_Error_e DRM_Prdy_LocalLicense_Release(DRM_Prdy_license_handle  *hLicense)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(hLicense != DRM_PRDY_LICENSE_HANDLE_INVALID );

    ChkDR( Drm_LocalLicense_Release( (DRM_LICENSE_HANDLE ) hLicense) );

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: LocalLicense_Release failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}


DRM_Prdy_Error_e DRM_Prdy_LocalLicense_EncryptSample(
        DRM_Prdy_license_handle   *hLicense,
        uint8_t                   *pInBuf,
        uint8_t                   *pOutBuf,
        uint32_t                   cbData,
        uint8_t                   *pIV)
{
    DRM_RESULT    dr = DRM_SUCCESS;
    DRM_DWORD     rgbSubsampleCount[1] = {0};
    DRM_BYTE      *rgbSubsamplePointer[1] = {0};

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(hLicense != DRM_PRDY_LICENSE_HANDLE_INVALID );
    BDBG_ASSERT(pInBuf != NULL );
    BDBG_ASSERT(pOutBuf != NULL );
    BDBG_ASSERT(pIV != NULL );

    BKNI_Memcpy(pOutBuf, pInBuf, cbData);

    rgbSubsampleCount[0]   = cbData;
    rgbSubsamplePointer[0] = (DRM_BYTE*) pOutBuf;

    ChkDR( Drm_LocalLicense_EncryptSample(
                (DRM_LICENSE_HANDLE ) hLicense,
                1,
                rgbSubsampleCount,
                rgbSubsamplePointer,
                (DRM_UINT64 *)pIV) );

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:

    BDBG_ERR(("%s: LocalLicense_EncryptSample failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}


DRM_Prdy_Error_e DRM_Prdy_LocalLicense_EncryptSubsamples(
        DRM_Prdy_license_handle   *hLicense,
        DRM_Prdy_sample_t         *pClearSamples,
        DRM_Prdy_sample_t         *pEncSamples,
        uint8_t                   *pIV)
{
    DRM_RESULT              dr = DRM_SUCCESS;
    DRM_DWORD              *rgbSubsampleCount = NULL;
    DRM_BYTE              **rgbSubsamplePointer = NULL;
    uint32_t                numOfSubsamples;
    DRM_Prdy_subSample_t   *clrSub = NULL;
    DRM_Prdy_subSample_t   *encSub = NULL;
    uint32_t               i = 0;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(hLicense != DRM_PRDY_LICENSE_HANDLE_INVALID );
    BDBG_ASSERT(pClearSamples != NULL );
    BDBG_ASSERT(pEncSamples != NULL );
    BDBG_ASSERT(pIV != NULL );

    numOfSubsamples = pClearSamples->numOfSubsamples;
    ChkMem(rgbSubsampleCount = Oem_MemAlloc(SIZEOF(DRM_DWORD) * numOfSubsamples));
    ChkMem(rgbSubsamplePointer = Oem_MemAlloc(SIZEOF(DRM_BYTE) * numOfSubsamples));

    clrSub = pClearSamples->subsamples;
    encSub = pEncSamples->subsamples;

    for( i =0; i < numOfSubsamples; ++i)
    {
        rgbSubsampleCount[i] = clrSub[i].size;
        /*
         * Assuming the memory of pEncSamples has been properly allocated
         * However, this memcpy can be avoided if we support in-place encryption
         */
        BKNI_Memcpy(encSub[i].sample, clrSub[i].sample, clrSub[i].size);
        /*
         * To avoid an extra memcpy for the output at the end,
         * we passed the pEncSamples[].subsamples.sample directly to the API
         * */
        rgbSubsamplePointer[i] = encSub[i].sample;
    }

    ChkDR( Drm_LocalLicense_EncryptSample(
                (DRM_LICENSE_HANDLE ) hLicense,
                numOfSubsamples,
                rgbSubsampleCount,
                rgbSubsamplePointer,
                (DRM_UINT64 *)pIV) );

    /* clean up */
    if( rgbSubsampleCount != NULL)  SAFE_OEM_FREE(rgbSubsampleCount);
    if( rgbSubsamplePointer  != NULL)  SAFE_OEM_FREE(rgbSubsamplePointer);

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:

    if( rgbSubsampleCount != NULL)  SAFE_OEM_FREE(rgbSubsampleCount);
    if( rgbSubsamplePointer  != NULL)  SAFE_OEM_FREE(rgbSubsamplePointer);

    BDBG_ERR(("%s: LocalLicense_EncryptSample failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_LocalLicense_GetKID(
        const DRM_Prdy_license_handle    hLicense,
        Drm_Prdy_KID_t                  *pKeyID )
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(hLicense != DRM_PRDY_LICENSE_HANDLE_INVALID );
    BDBG_ASSERT(pKeyID != NULL );

    ChkDR( Drm_LocalLicense_GetKID(
                (DRM_LICENSE_HANDLE ) hLicense,
                (DRM_KID *)pKeyID) );

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: LocalLicense_GetKID failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_LocalLicense_GetKID_base64W(
        const DRM_Prdy_license_handle    hLicense,
        uint16_t                        *pKidBase64W,
        uint32_t                        *pRequiredSize )
{
    DRM_RESULT      dr = DRM_SUCCESS;
    DRM_KID         pKeyID;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(hLicense != DRM_PRDY_LICENSE_HANDLE_INVALID );
    BDBG_ASSERT(pKidBase64W != NULL );
    BDBG_ASSERT(pRequiredSize != NULL );

    ChkDR( Drm_LocalLicense_GetKID(
                (DRM_LICENSE_HANDLE ) hLicense,
                &pKeyID) );

    *pRequiredSize = CCH_BASE64_EQUIV( SIZEOF( DRM_GUID ));

    if( DRM_B64_EncodeW(
        (const DRM_BYTE*)&pKeyID,
        SIZEOF( DRM_KID ),
        (DRM_WCHAR *)pKidBase64W,
        (DRM_DWORD *)pRequiredSize,
        0 ) == DRM_E_BUFFERTOOSMALL )
    {
       BDBG_MSG(("%s: KeyID Buffer too small, requiring %d, exiting\n", __FUNCTION__, *pRequiredSize));
       return DRM_Prdy_buffer_size;
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: LocalLicense_GetKID failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_StoreMgmt_DeleteLicenses(
        const DRM_Prdy_Handle_t            pPrdyContext,
        const uint16_t                     *pKidBase64W,
        uint32_t                            cbKidSize,
        uint32_t                           *pcLicDeleted)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_CONST_STRING dcstrKID = EMPTY_DRM_STRING;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pcLicDeleted != NULL);

    if( pKidBase64W != NULL)
    {
        dcstrKID.cchString = cbKidSize;
        ChkMem(dcstrKID.pwszString = (DRM_WCHAR *)Oem_MemAlloc(dcstrKID.cchString*SIZEOF(DRM_WCHAR)));
        BKNI_Memset((DRM_BYTE *)dcstrKID.pwszString, 0, dcstrKID.cchString*SIZEOF(DRM_WCHAR));
        BKNI_Memcpy((DRM_BYTE *)dcstrKID.pwszString, pKidBase64W, cbKidSize*SIZEOF(DRM_WCHAR));
        ChkDR( Drm_StoreMgmt_DeleteLicenses(
                    pPrdyContext->pDrmAppCtx,
                    &dcstrKID,
                    (DRM_DWORD *)pcLicDeleted));
    }
    else
    {
        ChkDR( Drm_StoreMgmt_DeleteLicenses(
                    pPrdyContext->pDrmAppCtx,
                    NULL,
                    (DRM_DWORD *)pcLicDeleted));
    }

    if( dcstrKID.pwszString != NULL) {
        SAFE_OEM_FREE(dcstrKID.pwszString);
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    if( dcstrKID.pwszString != NULL) {
        SAFE_OEM_FREE(dcstrKID.pwszString);
    }
    BDBG_ERR(("%s: Delete Licenses failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}


DRM_Prdy_Error_e DRM_Prdy_Cleanup_LicenseStores( DRM_Prdy_Handle_t   pPrdyContext)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);

    ChkDR(Drm_StoreMgmt_CleanupStore( pPrdyContext->pDrmAppCtx,
                                      DRM_STORE_CLEANUP_ALL,
                                      NULL,
                                      5,
                                      NULL));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to cleanup license store [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_Cleanup_Expired_LicenseStores( DRM_Prdy_Handle_t   pPrdyContext)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);

    ChkDR(Drm_StoreMgmt_CleanupStore( pPrdyContext->pDrmAppCtx,
                                      (DRM_STORE_CLEANUP_DELETE_EXPIRED_LICENSES
                                      |DRM_STORE_CLEANUP_DELETE_REMOVAL_DATE_LICENSES),
                                      NULL,
                                      5,
                                      NULL));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to delete expired licenses from license store [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

/*********************************************************************************
 *********************************************************************************
 *
 *     Prdy ND APIs
 *
 *********************************************************************************
 *********************************************************************************/

void DRM_Prdy_ND_MemFree( uint8_t   *pbToFree)
{
    BDBG_MSG(("%s - entering", __FUNCTION__));

    if( pbToFree != NULL)
    {
        Drm_Prnd_MemFree(pbToFree);
   }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
}

DRM_Prdy_Error_e DRM_Prdy_ND_GetMessageType(
        const uint8_t               *pbUnknownMessage,
        uint16_t                     cbUnknownMessage,
        DRM_Prdy_ND_Message_Type_e  *peMessageType )
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pbUnknownMessage != NULL);
    BDBG_ASSERT(peMessageType != NULL);

    ChkDR( Drm_Prnd_GetMessageType(
                (const DRM_BYTE *) pbUnknownMessage,
                (DRM_DWORD ) cbUnknownMessage,
                (DRM_PRND_MESSAGE_TYPE *) peMessageType));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to get message type [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_RegistrationRequest_Process(
        DRM_Prdy_Handle_t                         pPrdyContext,
        const uint8_t                            *pbReqMessage,
        uint16_t                                  cbReqMessage,
        DRM_Prdy_ND_Data_Callback                 pfnDataCallback,
        void                                     *pvDataCallbackContext,
        DRM_Prdy_ID_t                            *pSessionID,
        DRM_Prdy_ND_Proximity_Detection_Type_e   *peProximityDetectionType,
        uint8_t                                 **ppbTransmitterProximityDetectionChannel,
        uint16_t                                 *pcbTransmitterProximityDetectionChannel,
        uint16_t                                 *pdwFlags,
        uint32_t                                 *pPrdyResultCode )
{
    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pbReqMessage != NULL);
    BDBG_ASSERT(pSessionID != NULL);
    BDBG_ASSERT(peProximityDetectionType != NULL);
    BDBG_ASSERT(ppbTransmitterProximityDetectionChannel != NULL);
    BDBG_ASSERT(pcbTransmitterProximityDetectionChannel != NULL);
    BDBG_ASSERT(pdwFlags  != NULL);
    BDBG_ASSERT(pPrdyResultCode  != NULL);

    *pPrdyResultCode =
          (uint32_t) Drm_Prnd_Transmitter_RegistrationRequest_Process(
                            pPrdyContext->pDrmAppCtx,
                            (const DRM_BYTE *) pbReqMessage,
                            (DRM_DWORD) cbReqMessage,
                            (Drm_Prnd_Data_Callback) pfnDataCallback,
                            (DRM_VOID *) pvDataCallbackContext,
                            (DRM_ID *) pSessionID,
                            (DRM_PRND_PROXIMITY_DETECTION_TYPE *) peProximityDetectionType,
                            (DRM_BYTE **) ppbTransmitterProximityDetectionChannel,
                            (DRM_DWORD *) pcbTransmitterProximityDetectionChannel,
                            (DRM_DWORD *) pdwFlags );

    if( *pPrdyResultCode != DRM_SUCCESS) {
        goto ErrorExit;
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Transmitter_RegistrationRequest_Process failed [0x%X], exiting...\n", __FUNCTION__,*pPrdyResultCode));
    return DRM_Prdy_fail;
}

DRM_Prdy_ND_Transmitter_Context_t DRM_Prdy_ND_Transmitter_Initialize(void)
{
    DRM_PRDY_ND_TRANSMITTER_CONTEXT   *pTxCtx = NULL;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    pTxCtx = Oem_MemAlloc(sizeof(DRM_PRDY_ND_TRANSMITTER_CONTEXT));
    if( pTxCtx == NULL ) {
        BDBG_ERR(("%s: Transmitter Context alloc failed\n", __FUNCTION__));
        goto ErrorExit;
    }

    BKNI_Memset(pTxCtx, 0, sizeof(DRM_PRDY_ND_TRANSMITTER_CONTEXT));

    pTxCtx->pPrndTxContext = Oem_MemAlloc(sizeof(DRM_PRND_TRANSMITTER_CONTEXT));
    if( pTxCtx->pPrndTxContext == NULL ) {
        BDBG_ERR(("%s: DRM PRND Transmitter Context alloc failed\n", __FUNCTION__));
        goto ErrorExit;
    }

    BKNI_Memset(pTxCtx->pPrndTxContext, 0, sizeof(DRM_PRND_TRANSMITTER_CONTEXT));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return (DRM_Prdy_ND_Transmitter_Context_t) pTxCtx;

ErrorExit:
    BDBG_ERR(("%s: Transmitter_RegistrationRequest_Process failed, exiting...\n", __FUNCTION__));
    return NULL;
}


DRM_Prdy_ND_Receiver_Context_t DRM_Prdy_ND_Receiver_Initialize(void)
{
    DRM_PRDY_ND_RECEIVER_CONTEXT   *pRxCtx = NULL;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    pRxCtx = Oem_MemAlloc(sizeof(DRM_PRDY_ND_RECEIVER_CONTEXT));
    if( pRxCtx == NULL ) {
        BDBG_ERR(("%s: Receiver Context alloc failed\n", __FUNCTION__));
        goto ErrorExit;
    }

    BKNI_Memset(pRxCtx, 0, sizeof(DRM_PRDY_ND_RECEIVER_CONTEXT));

    pRxCtx->pPrndRxContext = Oem_MemAlloc(sizeof(DRM_PRND_RECEIVER_CONTEXT));
    if( pRxCtx->pPrndRxContext == NULL ) {
        BDBG_ERR(("%s: DRM PRND Receiver Context alloc failed\n", __FUNCTION__));
        goto ErrorExit;
    }

    BKNI_Memset(pRxCtx->pPrndRxContext, 0, sizeof(DRM_PRND_RECEIVER_CONTEXT));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return (DRM_Prdy_ND_Receiver_Context_t) pRxCtx;

ErrorExit:
    BDBG_ERR(("%s: Receiver_RegistrationRequest_Process failed, exiting...\n", __FUNCTION__));
    return NULL;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_Uninitialize( DRM_Prdy_ND_Transmitter_Context_t  pPrdyTxCtx)
{
    if(pPrdyTxCtx != NULL)
    {
        if((pPrdyTxCtx)->pPrndTxContext != NULL)
        {
            SAFE_OEM_FREE( pPrdyTxCtx->pPrndTxContext );
            pPrdyTxCtx->pPrndTxContext = NULL;
        }
    }
    return DRM_Prdy_ok;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_Uninitialize( DRM_Prdy_ND_Receiver_Context_t  pPrdyRxCtx)
{
    if(pPrdyRxCtx != NULL)
    {
        if(pPrdyRxCtx->pPrndRxContext != NULL)
        {
            SAFE_OEM_FREE( pPrdyRxCtx->pPrndRxContext );
            pPrdyRxCtx->pPrndRxContext = NULL;
        }
    }
    return DRM_Prdy_ok;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_BeginSession(
        DRM_Prdy_Handle_t                   pPrdyContext,
        DRM_Prdy_ND_Transmitter_Context_t   pPrdyTxCtx)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pPrdyTxCtx != NULL);

    ChkDR( Drm_Prnd_Transmitter_BeginSession(
                pPrdyContext->pDrmAppCtx,
                pPrdyTxCtx->pPrndTxContext));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Transmitter_BeginSession failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_BeginSession(
        DRM_Prdy_Handle_t                pPrdyContext,
        DRM_Prdy_ND_Receiver_Context_t   pPrdyRxCtx)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pPrdyRxCtx != NULL);

    ChkDR( Drm_Prnd_Receiver_BeginSession(
                pPrdyContext->pDrmAppCtx,
                pPrdyRxCtx->pPrndRxContext));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Receiver_BeginSession failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_EndSession( DRM_Prdy_Handle_t   pPrdyContext)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);

    ChkDR( Drm_Prnd_Transmitter_EndSession(
                pPrdyContext->pDrmAppCtx ));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Transmitter_EndSession failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_EndSession( DRM_Prdy_Handle_t   pPrdyContext)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);

    ChkDR( Drm_Prnd_Receiver_EndSession(
                pPrdyContext->pDrmAppCtx ));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Receiver_EndSession failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}


DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_RegistrationResponse_Generate(
        DRM_Prdy_Handle_t        pPrdyContext,
        const DRM_Prdy_ID_t     *pCustomDataTypeID,
        const uint8_t           *pbCustomData,
        uint16_t                 cbCustomData,
        uint16_t                 dwFlags,
        uint8_t                **ppbRespMessage,
        uint16_t                *pcbRespMessage)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BSTD_UNUSED(dwFlags);

    BDBG_ASSERT(pPrdyContext != NULL);

    ChkDR( Drm_Prnd_Transmitter_RegistrationResponse_Generate(
                pPrdyContext->pDrmAppCtx,
                (DRM_ID *) pCustomDataTypeID,
                (DRM_BYTE *) pbCustomData,
                (DRM_DWORD) cbCustomData,
                 DRM_PRND_FLAG_NO_FLAGS,
                (DRM_BYTE **) ppbRespMessage,
                (DRM_DWORD *) pcbRespMessage));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: RegistrationResponse Generation failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_RegistrationError_Generate(
        DRM_Prdy_Handle_t        pPrdyContext,
        uint32_t                 prdyResultCode,
        uint16_t                 dwFlags,
        uint8_t                **ppbErrMessage,
        uint16_t                *pcbErrMessage)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BSTD_UNUSED(dwFlags);

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(ppbErrMessage != NULL);
    BDBG_ASSERT(pcbErrMessage != NULL);

    ChkDR( Drm_Prnd_Transmitter_RegistrationError_Generate(
                pPrdyContext->pDrmAppCtx,
                (DRM_RESULT ) prdyResultCode,
                DRM_PRND_FLAG_NO_FLAGS,
                (DRM_BYTE **) ppbErrMessage,
                (DRM_DWORD *) pcbErrMessage));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: RegistrationError Generation failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_LicenseRequest_Process(
        DRM_Prdy_Handle_t                          pPrdyContext,
        const uint8_t                             *pbLicReqMessage,
        uint16_t                                   cbLicReqMessage,
        DRM_Prdy_ND_Data_Callback                  pfnDataCallback,
        void                                      *pvDataCallbackContext,
        Drm_Prdy_ND_Content_Identifier_Callback    pfnContentIdentifierCallback,
        void                                      *pvContentIdentifierCallbackContext,
        uint16_t                                  *pdwFlags,
        uint32_t                                  *pPrdyResultCode )
{
    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pbLicReqMessage != NULL);
    BDBG_ASSERT(pdwFlags != NULL);

    *pPrdyResultCode =
        (uint32_t) Drm_Prnd_Transmitter_LicenseRequest_Process(
                pPrdyContext->pDrmAppCtx,
                (const DRM_BYTE *) pbLicReqMessage,
                (DRM_DWORD) cbLicReqMessage,
                (Drm_Prnd_Data_Callback) pfnDataCallback,
                (DRM_VOID *) pvDataCallbackContext,
                (Drm_Prnd_Content_Identifier_Callback) pfnContentIdentifierCallback,
                (DRM_VOID *) pvContentIdentifierCallbackContext,
                (DRM_DWORD *) pdwFlags);

    if( *pPrdyResultCode != DRM_SUCCESS) {
        goto ErrorExit;
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Processeing LicenseRequest failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)pPrdyResultCode));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_LicenseTransmit_Generate(
        DRM_Prdy_Handle_t                          pPrdyContext,
        const DRM_Prdy_ID_t                       *pCustomDataTypeID,
        const uint8_t                             *pbCustomData,
        uint16_t                                   cbCustomData,
        uint16_t                                   dwFlags,
        uint8_t                                  **ppbLicTransmitMessage,
        uint16_t                                  *pcbLicTransmitMessage)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(ppbLicTransmitMessage != NULL);
    BDBG_ASSERT(pcbLicTransmitMessage != NULL);

    ChkDR( Drm_Prnd_Transmitter_LicenseTransmit_Generate(
                pPrdyContext->pDrmAppCtx,
                (DRM_ID *) pCustomDataTypeID,
                (DRM_BYTE *) pbCustomData,
                (DRM_DWORD) cbCustomData,
                (DRM_DWORD) dwFlags,
                (DRM_BYTE **) ppbLicTransmitMessage,
                (DRM_DWORD *) pcbLicTransmitMessage));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Generation for LicenseTransmit failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_PrepareLicensesForTransmit(
        DRM_Prdy_Handle_t         pPrdyContext)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);

    ChkDR( Drm_Prnd_Transmitter_PrepareLicensesForTransmit (
                pPrdyContext->pDrmAppCtx));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Preparing License for transmit failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_LicenseError_Generate(
        DRM_Prdy_Handle_t                          pPrdyContext,
        const DRM_Prdy_ID_t                       *pSessionID,
        uint32_t                                   prdyResultCode,
        uint16_t                                   dwFlags,
        uint8_t                                  **ppbErrMessage,
        uint16_t                                  *pcbErrMessage)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BSTD_UNUSED(dwFlags);

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(ppbErrMessage != NULL);
    BDBG_ASSERT(pcbErrMessage != NULL);

    ChkDR( Drm_Prnd_Transmitter_LicenseError_Generate(
                pPrdyContext->pDrmAppCtx,
                (const DRM_ID *) pSessionID,
                (DRM_RESULT ) prdyResultCode,
                DRM_PRND_FLAG_NO_FLAGS,
                (DRM_BYTE **) ppbErrMessage,
                (DRM_DWORD *) pcbErrMessage));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to generate License error [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_ProximityDetectionResponse_Process(
        DRM_Prdy_Handle_t                          pPrdyContext,
        const uint8_t                             *pbPDRespMessage,
        uint16_t                                   cbPDRespMessage,
        uint16_t                                   dwFlags,
        uint8_t                                  **ppbPDResultMessage,
        uint16_t                                  *pcbPDResultMessage,
        uint16_t                                  *pdwFlags)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BSTD_UNUSED(dwFlags);

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pbPDRespMessage != NULL);
    BDBG_ASSERT(ppbPDResultMessage != NULL);
    BDBG_ASSERT(pcbPDResultMessage != NULL);
    BDBG_ASSERT(pdwFlags != NULL);

    ChkDR( Drm_Prnd_Transmitter_ProximityDetectionResponse_Process(
                pPrdyContext->pDrmAppCtx,
                (const DRM_BYTE *) pbPDRespMessage,
                (DRM_DWORD ) cbPDRespMessage,
                DRM_PRND_FLAG_NO_FLAGS,
                (DRM_BYTE **) ppbPDResultMessage,
                (DRM_DWORD *) pcbPDResultMessage,
                (DRM_DWORD *) pdwFlags ));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to process Proximity Dectection Response [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_ProximityDetectionResult_Generate(
        DRM_Prdy_Handle_t                          pPrdyContext,
        const DRM_Prdy_ID_t                       *pSessionID,
        uint32_t                                   prdyResultCode,
        uint16_t                                   dwFlags,
        uint8_t                                  **ppbPDResultMessage,
        uint16_t                                  *pcbPDResultMessage)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BSTD_UNUSED(dwFlags);

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(ppbPDResultMessage != NULL);
    BDBG_ASSERT(pcbPDResultMessage != NULL);

    ChkDR( Drm_Prnd_Transmitter_ProximityDetectionResult_Generate(
                pPrdyContext->pDrmAppCtx,
                (const DRM_ID *) pSessionID,
                (DRM_RESULT ) prdyResultCode,
                DRM_PRND_FLAG_NO_FLAGS,
                (DRM_BYTE **) ppbPDResultMessage,
                (DRM_DWORD *) pcbPDResultMessage));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to generate proximity dectection result [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Transmitter_ProximityDetectionStart_Process(
        DRM_Prdy_Handle_t                          pPrdyContext,
        const uint8_t                             *pbPDStartMessage,
        uint16_t                                   cbPDStartMessage,
        uint16_t                                   dwFlags,
        uint8_t                                  **ppbPDChlgMessage,
        uint16_t                                  *pcbPDChlgMessage,
        uint16_t                                  *pdwFlags)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BSTD_UNUSED(dwFlags);

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pbPDStartMessage != NULL);
    BDBG_ASSERT(ppbPDChlgMessage != NULL);
    BDBG_ASSERT(pcbPDChlgMessage != NULL);
    BDBG_ASSERT(pdwFlags != NULL);

    ChkDR( Drm_Prnd_Transmitter_ProximityDetectionStart_Process(
                pPrdyContext->pDrmAppCtx,
                (const DRM_BYTE *) pbPDStartMessage,
                (DRM_DWORD ) cbPDStartMessage,
                DRM_PRND_FLAG_NO_FLAGS,
                (DRM_BYTE **) ppbPDChlgMessage,
                (DRM_DWORD *) pcbPDChlgMessage,
                (DRM_DWORD *) pdwFlags ));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to process Proximity Dectection Start message [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_RegistrationRequest_Generate(
        DRM_Prdy_Handle_t                          pPrdyContext,
        const DRM_Prdy_ID_t                       *pPreviousSessionID,
        const DRM_Prdy_ID_t                       *pCustomDataTypeID,
        const uint8_t                             *pbCustomData,
        uint16_t                                   cbCustomData,
        uint16_t                                   dwFlags,
        uint8_t                                  **ppbReqMessage,
        uint16_t                                  *pcbReqMessage)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BSTD_UNUSED(dwFlags);

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(ppbReqMessage != NULL);
    BDBG_ASSERT(pcbReqMessage != NULL);

    ChkDR( Drm_Prnd_Receiver_RegistrationRequest_Generate(
                pPrdyContext->pDrmAppCtx,
                (DRM_ID *) pPreviousSessionID,
                (DRM_ID *) pCustomDataTypeID,
                (const DRM_BYTE *) pbCustomData,
                (DRM_DWORD ) cbCustomData,
                (DRM_DWORD ) dwFlags,
                (DRM_BYTE **) ppbReqMessage,
                (DRM_DWORD *) pcbReqMessage));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to generate Registration request [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_RegistrationResponse_Process(
        DRM_Prdy_Handle_t                          pPrdyContext,
        const uint8_t                             *pbRespMessage,
        uint16_t                                   cbRespMessage,
        DRM_Prdy_ND_Data_Callback                  pfnDataCallback,
        void                                      *pvDataCallbackContext,
        DRM_Prdy_ID_t                             *pSessionID,
        DRM_Prdy_ND_Proximity_Detection_Type_e    *peProximityDetectionType,
        uint8_t                                  **ppbTransmitterProximityDetectionChannel,
        uint16_t                                  *pcbTransmitterProximityDetectionChannel,
        uint16_t                                  *pdwFlags,
        uint32_t                                  *pPrdyResultCode)
{
    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pbRespMessage != NULL);
    BDBG_ASSERT(pSessionID != NULL);
    BDBG_ASSERT(peProximityDetectionType != NULL);
    BDBG_ASSERT(ppbTransmitterProximityDetectionChannel != NULL);
    BDBG_ASSERT(pcbTransmitterProximityDetectionChannel != NULL);
    BDBG_ASSERT(pdwFlags  != NULL);
    BDBG_ASSERT(pPrdyResultCode  != NULL);

    *pPrdyResultCode =
          (uint32_t) Drm_Prnd_Receiver_RegistrationResponse_Process(
                            pPrdyContext->pDrmAppCtx,
                            (const DRM_BYTE *) pbRespMessage,
                            (DRM_DWORD) cbRespMessage,
                            (Drm_Prnd_Data_Callback) pfnDataCallback,
                            (DRM_VOID *) pvDataCallbackContext,
                            (DRM_ID *) pSessionID,
                            (DRM_PRND_PROXIMITY_DETECTION_TYPE *) peProximityDetectionType,
                            (DRM_BYTE **) ppbTransmitterProximityDetectionChannel,
                            (DRM_DWORD *) pcbTransmitterProximityDetectionChannel,
                            (DRM_DWORD *) pdwFlags );

    if( *pPrdyResultCode != DRM_SUCCESS) {
        goto ErrorExit;
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Processing Registration Response failed [0x%X], exiting...\n", __FUNCTION__,*pPrdyResultCode));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_RegistrationError_Process(
        DRM_Prdy_Handle_t                        pPrdyContext,
        const uint8_t                           *pbErrMessage,
        uint16_t                                 cbErrMessage,
        uint32_t                                *pPrdyResultCode,
        uint16_t                                *pdwFlags)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pbErrMessage != NULL);
    BDBG_ASSERT(pPrdyResultCode != NULL);
    BDBG_ASSERT(pdwFlags != NULL);

    ChkDR( Drm_Prnd_Receiver_RegistrationError_Process(
                pPrdyContext->pDrmAppCtx,
                (const DRM_BYTE *) pbErrMessage,
                (DRM_DWORD) cbErrMessage,
                (DRM_RESULT * ) pPrdyResultCode,
                (DRM_DWORD *) pdwFlags));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to process Registration Error message [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_LicenseRequest_Generate(
        DRM_Prdy_Handle_t                        pPrdyContext,
        const DRM_Prdy_guid_t                   *pguidRequestedAction,
        const DRM_Prdy_guid_t                   *pguidRequestedActionQualifier,
        DRM_Prdy_ND_Content_Identifier_Type_e    eContentIdentifierType,
        const uint8_t                           *pbContentIdentifier,
        uint16_t                                 cbContentIdentifier,
        const DRM_Prdy_ID_t                     *pCustomDataTypeID,
        const uint8_t                           *pbCustomData,
        uint16_t                                 cbCustomData,
        uint16_t                                 dwFlags,
        uint8_t                                **ppbLicReqMessage,
        uint16_t                                *pcbLicReqMessage)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pguidRequestedAction != NULL);
    BDBG_ASSERT(pbContentIdentifier != NULL);
    BDBG_ASSERT(ppbLicReqMessage != NULL);
    BDBG_ASSERT(pcbLicReqMessage != NULL);

    ChkDR( Drm_Prnd_Receiver_LicenseRequest_Generate (
                pPrdyContext->pDrmAppCtx,
                (const DRM_GUID *) pguidRequestedAction,
                (const DRM_GUID *) pguidRequestedActionQualifier,
                (DRM_PRND_CONTENT_IDENTIFIER_TYPE) eContentIdentifierType,
                (const DRM_BYTE *)pbContentIdentifier,
                (DRM_DWORD)cbContentIdentifier,
                (const DRM_ID *) pCustomDataTypeID,
                (const DRM_BYTE *) pbCustomData,
                (DRM_DWORD) cbCustomData,
                (DRM_DWORD) dwFlags,
                (DRM_BYTE **) ppbLicReqMessage,
                (DRM_DWORD *) pcbLicReqMessage));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to generate License Request [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_LicenseTransmit_Process(
        DRM_Prdy_Handle_t                        pPrdyContext,
        const uint8_t                           *pbLicTransmitMessage,
        uint16_t                                 cbLicTransmitMessage,
        DRM_Prdy_ND_Data_Callback                pfnDataCallback,
        void                                    *pvDataCallbackContext,
        uint16_t                                *pdwFlags)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pbLicTransmitMessage != NULL);
    BDBG_ASSERT(pdwFlags != NULL);

    ChkDR( Drm_Prnd_Receiver_LicenseTransmit_Process(
                pPrdyContext->pDrmAppCtx,
                (const DRM_BYTE *) pbLicTransmitMessage,
                (DRM_DWORD) cbLicTransmitMessage,
                (Drm_Prnd_Data_Callback) pfnDataCallback,
                (DRM_VOID *) pvDataCallbackContext,
                (DRM_DWORD *) pdwFlags));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to process License Response from Transmitter [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_LicenseError_Process(
        DRM_Prdy_Handle_t                        pPrdyContext,
        const uint8_t                           *pbErrMessage,
        uint16_t                                 cbErrMessage,
        uint32_t                                *pPrdyResultCode,
        uint16_t                                *pdwFlags)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pbErrMessage != NULL);
    BDBG_ASSERT(pPrdyResultCode != NULL);
    BDBG_ASSERT(pdwFlags != NULL);

    ChkDR( Drm_Prnd_Receiver_LicenseError_Process(
                pPrdyContext->pDrmAppCtx,
                (const DRM_BYTE *) pbErrMessage,
                (DRM_DWORD) cbErrMessage,
                (DRM_RESULT * ) pPrdyResultCode,
                (DRM_DWORD *) pdwFlags));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to process License Error message [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_ProximityDetectionStart_Generate(
        DRM_Prdy_Handle_t                        pPrdyContext,
        uint16_t                                 dwFlags,
        uint8_t                                **ppbPDStartMessage,
        uint16_t                                *pcbPDStartMessage)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BSTD_UNUSED(dwFlags);

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(ppbPDStartMessage != NULL);
    BDBG_ASSERT(ppbPDStartMessage != NULL);
    BDBG_ASSERT(pcbPDStartMessage != NULL);

    ChkDR( Drm_Prnd_Receiver_ProximityDetectionStart_Generate(
                pPrdyContext->pDrmAppCtx,
                DRM_PRND_FLAG_NO_FLAGS,
                (DRM_BYTE **) ppbPDStartMessage,
                (DRM_DWORD *) pcbPDStartMessage));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to generate a Proximity Detection Start message [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_ProximityDetectionChallenge_Process(
        DRM_Prdy_Handle_t                        pPrdyContext,
        const uint8_t                           *pbPDChlgMessage,
        uint16_t                                 cbPDChlgMessage,
        uint16_t                                 dwFlags,
        uint8_t                                **ppbPDRespMessage,
        uint16_t                                *pcbPDRespMessage,
        uint16_t                                *pdwFlags)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BSTD_UNUSED(dwFlags);

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pbPDChlgMessage != NULL);
    BDBG_ASSERT(ppbPDRespMessage != NULL);
    BDBG_ASSERT(pcbPDRespMessage != NULL);
    BDBG_ASSERT(pdwFlags != NULL);

    ChkDR( Drm_Prnd_Receiver_ProximityDetectionChallenge_Process(
                pPrdyContext->pDrmAppCtx,
                (const DRM_BYTE *) pbPDChlgMessage,
                (DRM_DWORD) cbPDChlgMessage,
                DRM_PRND_FLAG_NO_FLAGS,
                (DRM_BYTE **) ppbPDRespMessage,
                (DRM_DWORD *) pcbPDRespMessage,
                (DRM_DWORD *) pdwFlags));

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to process a Proximity Detection Challenge response message [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_ND_Receiver_ProximityDetectionResult_Process(
        DRM_Prdy_Handle_t                        pPrdyContext,
        const uint8_t                           *pbPDResultMessage,
        uint16_t                                 cbPDResultMessage,
        uint16_t                                *pdwFlags)
{
    DRM_RESULT dr = DRM_SUCCESS;
    DRM_RESULT drPDResult = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pbPDResultMessage != NULL);
    BDBG_ASSERT(pdwFlags != NULL);

    ChkDR( Drm_Prnd_Receiver_ProximityDetectionResult_Process(
                pPrdyContext->pDrmAppCtx,
                (const DRM_BYTE *) pbPDResultMessage,
                (DRM_DWORD) cbPDResultMessage,
                &drPDResult,
                (DRM_DWORD *) pdwFlags));

    ChkDR( drPDResult);

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed to process a Proximity Detection Challenge response message [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;
}


/* helper functions */
DRM_Prdy_Error_e DRM_Prdy_B64_EncodeW( uint8_t *pBytes, uint32_t cbBytes, uint16_t *pBase64W, uint32_t *pReqSize)
{
    if( DRM_B64_EncodeW(
        (const DRM_BYTE*)pBytes,
        cbBytes,
        (DRM_WCHAR *)pBase64W,
        (DRM_DWORD *)pReqSize,
        0 ) == DRM_E_BUFFERTOOSMALL )
    {
       return DRM_Prdy_buffer_size;
    }
    return DRM_Prdy_ok;
}

bool DRM_Prdy_convertCStringToWString( char * pCStr, wchar_t * pWStr, uint16_t * cchStr)
{
    return convertCStringToWString( pCStr, (DRM_WCHAR *) pWStr, (DRM_DWORD *) cchStr);
}

bool DRM_Prdy_convertWStringToCString( wchar_t * pWStr, char * pCStr, uint32_t cchStr)
{
    return convertWStringToCString( (DRM_WCHAR *) pWStr, pCStr, (DRM_DWORD ) cchStr);
}

void DRM_Prdy_qwordToNetworkbytes(uint8_t *pBytes, unsigned index, uint64_t qword)
{
    QWORD_TO_NETWORKBYTES(pBytes, index, qword );
}

uint32_t DRM_Prdy_Cch_Base64_Equiv(size_t cb)
{
    return CCH_BASE64_EQUIV(cb);
}


DRM_Prdy_Error_e DRM_Prdy_License_GetProperty(
        DRM_Prdy_Handle_t pPrdyContext,
        DRM_Prdy_license_property_e licenseProperty,
        const uint8_t *pLicensePropertyExtraData,
        const uint32_t *pLicensePropertyExtraDataSize,
        uint32_t *pLicensePropertyOutputData)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pLicensePropertyOutputData != NULL);

    ChkDR( Drm_License_GetProperty(
            pPrdyContext->pDrmAppCtx,
            (DRM_LICENSE_GET_PROPERTY)licenseProperty,
            (const DRM_BYTE *)pLicensePropertyExtraData,
            (const DRM_DWORD *)pLicensePropertyExtraDataSize,
            (DRM_DWORD *)pLicensePropertyOutputData) );

    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed [0x%X]\n", __FUNCTION__, (unsigned int)dr));
    return DRM_Prdy_fail;
}


DRM_Prdy_Error_e DRM_Prdy_LicenseQuery_GetState(
        DRM_Prdy_Handle_t pPrdyContext,
        DRM_Prdy_license_right_e licenseRight,
        DRM_Prdy_license_state_t *pLicenseState)
{
    DRM_RESULT dr = DRM_SUCCESS;
    const DRM_CONST_STRING *rightToQuery[1] = { NULL };
    DRM_LICENSE_STATE_DATA rightState[NO_OF(rightToQuery)];
    uint32_t i;

    BDBG_ASSERT(pPrdyContext != NULL);
    if (pLicenseState == NULL) {
        BDBG_ERR(("%s: output buffer is NULL\n", __FUNCTION__));
        return DRM_Prdy_invalid_parameter;
    }

    switch (licenseRight) {
        case eDRM_Prdy_license_right_none:
            rightToQuery[0] = &g_dstrWMDRM_RIGHT_NONE;
            break;
        case eDRM_Prdy_license_right_playback:
            rightToQuery[0] = &g_dstrWMDRM_RIGHT_PLAYBACK;
            break;
        case eDRM_Prdy_license_right_collaborative_play:
            rightToQuery[0] = &g_dstrWMDRM_RIGHT_COLLABORATIVE_PLAY;
            break;
        case eDRM_Prdy_license_right_copy_to_cd:
            rightToQuery[0] = &g_dstrWMDRM_RIGHT_COPY_TO_CD;
            break;
        case eDRM_Prdy_license_right_copy:
            rightToQuery[0] = &g_dstrWMDRM_RIGHT_COPY;
            break;
        case eDRM_Prdy_license_right_create_thumbnail_image:
            rightToQuery[0] = &g_dstrWMDRM_RIGHT_CREATE_THUMBNAIL_IMAGE;
            break;
        case eDRM_Prdy_license_right_move:
            rightToQuery[0] = &g_dstrWMDRM_RIGHT_MOVE;
            break;
        default:
            BDBG_ERR(("%s: Unknown right (%d)\n", __FUNCTION__, licenseRight));
            return DRM_Prdy_invalid_parameter;
    }
    ChkDR( Drm_LicenseQuery_GetState(
                pPrdyContext->pDrmAppCtx,
                rightToQuery,
                NO_OF( rightToQuery ),
                rightState,
                NULL,
                NULL) );

    pLicenseState->dwStreamId = rightState[0].dwStreamId;
    pLicenseState->dwCategory = rightState[0].dwCategory;
    pLicenseState->dwNumCounts = rightState[0].dwStreamId;
    for (i=0; i<pLicenseState->dwNumCounts; i++) {
        pLicenseState->dwCount[i] = rightState[0].dwCount[i];
    }
    pLicenseState->dwNumDates = rightState[0].dwNumDates;
    for (i=0; i<pLicenseState->dwNumDates; i++) {
        pLicenseState->date[i].dwFileTimeHigh = rightState[0].datetime[i].dwHighDateTime;
        pLicenseState->date[i].dwFileTimeLow = rightState[0].datetime[i].dwLowDateTime;
    }
    pLicenseState->dwVague = rightState[0].dwVague;

    return DRM_Prdy_ok;

ErrorExit:
    return DRM_Prdy_fail;
}

DRM_Prdy_Error_e DRM_Prdy_SecureClock_GenerateChallenge(
        DRM_Prdy_Handle_t      pPrdyContext,
        wchar_t               *pURL,
        uint32_t              *pURL_len,
        uint8_t               *pCh_data,    /* [out] */
        uint32_t              *pCh_len)
{
    DRM_Prdy_Error_e  result = DRM_Prdy_ok;
    DRM_RESULT        dr = DRM_SUCCESS;
    DRM_DWORD         cchURL = 0;
    /*DRM_WCHAR        *pwszURL = NULL;*/
    DRM_DWORD        cchChallenge       = 0;

    BDBG_MSG(("%s:%d - entering.", __FUNCTION__,__LINE__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pURL_len != NULL);
    BDBG_ASSERT(pCh_len != NULL);
    cchURL = (DRM_DWORD) *pURL_len;
    cchChallenge  = (DRM_DWORD)*pCh_len;
    /*
    if( pURL != NULL)
    {
        if( !DRM_Prdy_convertCStringToWString(pURL,(wchar_t *) pwszURL, (uint16_t *) &cchURL))
        {
            goto ErrorExit;
        }
    }
    */

    dr = Drm_SecureClock_GenerateChallenge(
                    pPrdyContext->pDrmAppCtx,
                    (DRM_WCHAR *) pURL,
                    &cchURL,
                    (DRM_BYTE *) pCh_data,
                    &cchChallenge );

    if( dr == DRM_E_BUFFERTOOSMALL )
    {
        BDBG_MSG(("%s: Exiting with Buffer too small...\n", __FUNCTION__));
        result = DRM_Prdy_buffer_size;
    }
    else if( dr != DRM_SUCCESS)
    {
        BDBG_LOG(("%s:%d returning [0x%x]\n", __FUNCTION__,__LINE__,dr));
        BDBG_MSG(("%s:%d returning [0x%x]\n", __FUNCTION__,__LINE__,dr));
        goto ErrorExit;
    }

    *pURL_len = cchURL;
    *pCh_len = cchChallenge;

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return result;

ErrorExit:
    BDBG_ERR(("%s:%d failed [0x%x], exiting...\n", __FUNCTION__,__LINE__,(unsigned int)dr));
    return DRM_Prdy_fail;
}


DRM_Prdy_Error_e DRM_Prdy_SecureClock_ProcessResponse(
        DRM_Prdy_Handle_t      pPrdyContext,
        uint8_t               *pChResponse,
        uint32_t               pChResponselen)
{
    DRM_RESULT        dr = DRM_SUCCESS;
    DRM_RESULT       drResponse = DRM_SUCCESS;
    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pChResponse != NULL);

    BDBG_MSG(("%s - entering", __FUNCTION__));

    ChkDR(Drm_SecureClock_ProcessResponse(
                    pPrdyContext->pDrmAppCtx,
                    pChResponse,
                    pChResponselen,
                    &drResponse ));

    if ( drResponse != DRM_SUCCESS )
    {
       dr = drResponse;
       ChkDR( drResponse);
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return DRM_Prdy_fail;

}

DRM_Prdy_Error_e DRM_Prdy_SecureClock_GetStatus(
        DRM_Prdy_Handle_t      pPrdyContext,
        uint32_t              *pStatus )
{
    DRM_RESULT        dr = DRM_SUCCESS;
    DRM_DWORD         cchSecTime         = 0;
    DRM_WCHAR        *pwszSecTime        = NULL;
    DRM_BYTE         *pbTimeStatus       = NULL;
    DRM_DWORD         cbTimeStatus       = 0;
    DRM_DWORD         dwFlag             = 0;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pStatus != NULL);

    dr = Drm_SecureClock_GetValue( pPrdyContext->pDrmAppCtx, pwszSecTime, &cchSecTime, &dwFlag, pbTimeStatus, &cbTimeStatus );

    if ( dr != DRM_E_BUFFERTOOSMALL )
    {
        goto ErrorExit;
    }

    ChkMem( pwszSecTime = (DRM_WCHAR*) Oem_MemAlloc( cchSecTime * SIZEOF( DRM_WCHAR ) ) );
    ChkMem( pbTimeStatus = (DRM_BYTE*) Oem_MemAlloc( cbTimeStatus ) );
    MEMSET( pwszSecTime, 'a', cchSecTime * SIZEOF( DRM_WCHAR ) );
    MEMSET( pbTimeStatus, 'b', cbTimeStatus );

    ChkDR( Drm_SecureClock_GetValue( pPrdyContext->pDrmAppCtx, pwszSecTime, &cchSecTime, &dwFlag, pbTimeStatus, &cbTimeStatus ) );
    *pStatus = dwFlag;

ErrorExit:

    if( pwszSecTime != NULL )
    {
        Oem_MemFree( pwszSecTime );
    }

    if( pbTimeStatus != NULL )
    {
        Oem_MemFree( pbTimeStatus );
    }

    if( dr != DRM_SUCCESS)
    {
        BDBG_ERR(("%s: failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));

        return DRM_Prdy_fail;
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));

    return DRM_Prdy_ok;

}

void DRM_Prdy_GetSystemTime(
        DRM_Prdy_Handle_t      pPrdyContext,
        uint16_t              *pYear,
        uint16_t              *pMonth,
        uint16_t              *pDayOfWeek,
        uint16_t              *pDay,
        uint16_t              *pHour,
        uint16_t              *pMinute,
        uint16_t              *pSecond,
        uint16_t              *pMilliseconds)
{
    DRMSYSTEMTIME     systemTime;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pYear != NULL);
    BDBG_ASSERT(pMonth != NULL);
    BDBG_ASSERT(pDayOfWeek != NULL);
    BDBG_ASSERT(pDay != NULL);
    BDBG_ASSERT(pHour != NULL);
    BDBG_ASSERT(pMinute != NULL);
    BDBG_ASSERT(pSecond != NULL);
    BDBG_ASSERT(pMilliseconds != NULL);

    Oem_Clock_GetSystemTime( pPrdyContext->pOEMContext, &systemTime);

    *pYear = systemTime.wYear;
    *pMonth = systemTime.wMonth;
    *pDayOfWeek = systemTime.wDayOfWeek;
    *pDay = systemTime.wDay;
    *pHour = systemTime.wHour;
    *pMinute = systemTime.wMinute;
    *pSecond = systemTime.wSecond;
    *pMilliseconds = systemTime.wMilliseconds;

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));

}

void DRM_Prdy_SetSystemTime(
        DRM_Prdy_Handle_t      pPrdyContext,
        uint16_t               year,
        uint16_t               month,
        uint16_t               dayOfWeek,
        uint16_t               day,
        uint16_t               hour,
        uint16_t               minute,
        uint16_t               second,
        uint16_t               milliseconds)
{
    DRMSYSTEMTIME     systemTime;
    DRM_APP_CONTEXT_INTERNAL    *poAppContextInternal = ( DRM_APP_CONTEXT_INTERNAL * )pPrdyContext->pDrmAppCtx;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);

    systemTime.wYear         = year;
    systemTime.wMonth        = month;
    systemTime.wDayOfWeek    = dayOfWeek;
    systemTime.wDay          = day;
    systemTime.wHour         = hour;
    systemTime.wMinute       = minute;
    systemTime.wSecond       = second;
    systemTime.wMilliseconds = milliseconds;

#ifdef CMD_DRM_PLAYREADY_SAGE_IMPL
    BDBG_MSG(("%s:%d - Calling Oem_Clock_SetSystemTime with BBX context %x\n",__FUNCTION__,__LINE__,&poAppContextInternal->oBlackBoxContext));
    Oem_Clock_SetSystemTime( &poAppContextInternal->oBlackBoxContext, &systemTime);
    poAppContextInternal->fClockSet = true;
#else
    Oem_Clock_SetSystemTime( pPrdyContext->pOEMContext, &systemTime);
#endif
    BDBG_MSG(("%s: Exiting", __FUNCTION__));
}

/***********************************************************************************
 * Function: DRM_Prdy_Reader_Unbind()
  ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Reader_Unbind(
        DRM_Prdy_Handle_t           pPrdyContext,
        DRM_Prdy_DecryptContext_t  *pDecryptContext )
{
    DRM_RESULT             dr;

    BDBG_MSG(("%s - entering", __FUNCTION__));
    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pDecryptContext != NULL);
    BDBG_ASSERT(pDecryptContext->pDecrypt  != NULL);

    dr = Drm_Reader_Unbind(
            pPrdyContext->pDrmAppCtx,
            (DRM_DECRYPT_CONTEXT *) pDecryptContext->pDecrypt);

    if(pDecryptContext->pKeyContext) Drm_Prdy_FreeDecryptContextKeySlot(pDecryptContext->pKeyContext);

    SAFE_OEM_FREE(pDecryptContext->pDecrypt);
    pDecryptContext->pDecrypt = NULL;

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));

    return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_SupportSecureStop()
 ***********************************************************************************/
bool DRM_Prdy_SupportSecureStop( void )
{
    return Drm_SupportSecureStop();
}

/***********************************************************************************
 * Function: DRM_Prdy_TurnSecureStop()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_TurnSecureStop(
        DRM_Prdy_Handle_t pPrdyContext,
        bool inOnOff)
{
    DRM_RESULT             dr;

    BDBG_ASSERT(pPrdyContext != NULL);

    dr = Drm_TurnSecureStop(pPrdyContext->pDrmAppCtx, inOnOff);
    BDBG_MSG(("exiting %s dr=0x%lx",__FUNCTION__,dr));
    return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_GetSecureStopIds()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_GetSecureStopIds(
        DRM_Prdy_Handle_t      pPrdyContext,
        uint8_t                pSessionIDs[DRM_PRDY_MAX_NUM_SECURE_STOPS][DRM_PRDY_SESSION_ID_LEN],
        uint32_t              *pCount )
{
    DRM_RESULT             dr;

    BDBG_ASSERT(pPrdyContext != NULL);

    dr = Drm_GetSecureStopIds(
            pPrdyContext->pDrmAppCtx,
            pSessionIDs,
            pCount);

    return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_GetSecureStop()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_GetSecureStop(
        DRM_Prdy_Handle_t       pPrdyContext,
        uint8_t                *pSessionID,
        uint8_t                *pSecureStopData,
        uint16_t               *pSecureStopLen )
{
    DRM_RESULT             dr;

    BDBG_ASSERT(pPrdyContext != NULL);

    dr = Drm_GetSecureStop(
            pPrdyContext->pDrmAppCtx,
            pSessionID,
            pSecureStopData,
            pSecureStopLen);

    return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_CommitSecureStop()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_CommitSecureStop(
        DRM_Prdy_Handle_t       pPrdyContext,
        uint8_t                *pSessionID )
{
    DRM_RESULT             dr;

    BDBG_ASSERT(pPrdyContext != NULL);

    dr = Drm_CommitSecureStop(pPrdyContext->pDrmAppCtx, pSessionID);

    return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_ResetSecureStops()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_ResetSecureStops(
        DRM_Prdy_Handle_t     pPrdyContext,
        uint16_t             *pCount )
{
    DRM_RESULT             dr;

    BDBG_ASSERT(pPrdyContext != NULL);

    dr = Drm_ResetSecureStops(pPrdyContext->pDrmAppCtx, pCount);

    return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_DeleteSecureStore()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_DeleteSecureStore(void)
{
    DRM_RESULT          dr;

    dr = Drm_DeleteSecureStore(&sDstrHDSPath);

    return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_GetSecureStoreHash()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_GetSecureStoreHash(
    uint8_t             *pSecureStoreHash )
{
    DRM_RESULT          dr;

    dr = Drm_GetSecureStoreHash(&sDstrHDSPath, pSecureStoreHash);

    return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_DeleteKeyStore()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_DeleteKeyStore(
    DRM_Prdy_Handle_t   pPrdyContext )
{
    DRM_RESULT          dr;

    if (pPrdyContext != NULL)
        dr = Drm_DeleteKeyStore(pPrdyContext->pDrmAppCtx);
    else
        dr = Drm_DeleteKeyStore(NULL);

    return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_GetKeyStoreHash()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_GetKeyStoreHash(
        DRM_Prdy_Handle_t   pPrdyContext,
        uint8_t             *pSecureStoreHash )
{
    DRM_RESULT          dr;

    if (pPrdyContext != NULL)
        dr = Drm_GetKeyStoreHash(pPrdyContext->pDrmAppCtx, pSecureStoreHash);
    else
        dr = Drm_GetKeyStoreHash(NULL, pSecureStoreHash);

    return convertDrmResult(dr);
}

/***********************************************************************************
 * Function: DRM_Prdy_Clock_GetSystemTime()
 ***********************************************************************************/
DRM_Prdy_Error_e DRM_Prdy_Clock_GetSystemTime(
        DRM_Prdy_Handle_t   pPrdyContext,
        uint8_t            *pSystemTime)
{
    DRM_RESULT dr = DRM_SUCCESS;

    BDBG_MSG(("%s - entering", __FUNCTION__));

    BDBG_ASSERT(pPrdyContext != NULL);
    BDBG_ASSERT(pSystemTime != NULL);

    ChkDR( Drm_Clock_GetSystemTime(
                pPrdyContext->pDrmAppCtx,
                (DRM_UINT64 *)pSystemTime) );

    if(dr != DRM_SUCCESS) {
       goto ErrorExit;
    }

    BDBG_MSG(("%s: Exiting\n", __FUNCTION__));
    return DRM_Prdy_ok;

ErrorExit:
    BDBG_ERR(("%s: Operation failed [0x%X], exiting...\n", __FUNCTION__,(unsigned int)dr));
    return convertDrmResult(dr);
}

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
#ifndef __DRM_WVOEMCRYPTO_TL_H__
#define __DRM_WVOEMCRYPTO_TL_H__

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"

#include "drm_common_tl.h"

#include "drm_common.h"

#ifdef __cplusplus
extern "C" {
#endif



static const size_t WVCDM_KEY_CONTROL_SIZE = 16;
static const size_t WVCDM_KEY_IV_SIZE = 16;
static const size_t WVCDM_KEY_PAD_SIZE = 16;
static const size_t WVCDM_KEY_SIZE = 16;
static const size_t WVCDM_MAC_KEY_SIZE = 32;
#define AES_BLOCK_SIZE 16

#define HMAC_SHA256_SIGNATURE_SIZE (32)
#define SHA256_DIGEST_SIZE         (32)
#define DRM_WVOEMCRYPTO_SHA256_DIGEST_LENGTH    32

#define DRM_WVOEMCRYPTO_NUM_SESSION_KEY_SLOT 2
#define DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT 20

#define DRM_WVOEMCRYPTO_CENTRAL_KEY_CACHE 0x1
#define DRM_WVOEMCRYPTO_BIG_USAGE_TABLE_FORMAT 0x2
#define DRM_WVOEMCRYPTO_EVENT_DRIVEN_VERIFY 0x4

typedef struct Drm_WVOemCryptoParamSettings_t
{
    BDBG_OBJECT(oemcrypto_brcm)
    DrmCommonInit_TL_t drmCommonInit;
    DrmCommonOperationStruct_t drmCommonOpStruct;
    char * drm_bin_file_path;
    uint32_t api_version;
}Drm_WVOemCryptoParamSettings_t;

typedef enum Drm_WVOemCryptoCipherMode {
    Drm_WVOemCrypto_CipherMode_CTR,
    Drm_WVOemCrypto_CipherMode_CBC,
} Drm_WVOemCryptoCipherMode;

typedef struct
{
    const uint8_t* key_id;
    size_t         key_id_length;
    const uint8_t* key_data_iv;
    const uint8_t* key_data;
    size_t         key_data_length;
    const uint8_t* key_control_iv;
    const uint8_t* key_control;
    Drm_WVOemCryptoCipherMode cipher_mode;
} Drm_WVOemCryptoKeyObject;

/* Better to have the same define in Sage side */
typedef struct
{
    uint32_t key_id;
    uint32_t key_id_length;
    uint32_t key_data_iv;
    uint32_t key_data;
    uint32_t key_data_length;
    uint32_t key_control_iv;
    uint32_t key_control;
    Drm_WVOemCryptoCipherMode cipher_mode;
} Drm_WVOemCryptoSageKeyObject;

typedef struct
{
    const uint8_t* key_id;
    size_t         key_id_length;
    const uint8_t* key_data_iv;
    const uint8_t* key_data;
    size_t         key_data_length;
    const uint8_t* key_control_iv;
    const uint8_t* key_control;
} Drm_WVOemCryptoKeyObject_V10;

/* Better to have the same define in Sage side */
typedef struct
{
    uint32_t key_id;
    uint32_t key_id_length;
    uint32_t key_data_iv;
    uint32_t key_data;
    uint32_t key_data_length;
    uint32_t key_control_iv;
    uint32_t key_control;
} Drm_WVOemCryptoSageKeyObject_V10;

typedef struct Drm_WVOemCryptoKeyRefreshObject
{
    const uint8_t* key_id;
    size_t         key_id_length;
    const uint8_t* key_control_iv;
    const uint8_t* key_control;

} Drm_WVOemCryptoKeyRefreshObject;

/* Better to have the same define in Sage side */
typedef struct Drm_WVOemCryptoSageKeyRefreshObject
{
    uint32_t key_id;
    uint32_t key_id_length;
    uint32_t key_control_iv;
    uint32_t key_control;
} Drm_WVOemCryptoSageKeyRefreshObject;

typedef enum Drm_WVOemCryptoBufferType_t
{
    Drm_WVOEMCrypto_BufferType_Clear,
    Drm_WVOEMCrypto_BufferType_Secure,
    Drm_WVOEMCrypto_BufferType_Direct
} Drm_WVOemCryptoBufferType_t;

typedef struct
{
    Drm_WVOemCryptoBufferType_t type;
    union {
        struct {                   /*/ type == OEMCrypto_BufferType_Clear*/
            uint8_t* address;
            size_t max_length;
        } clear;
        struct {                   /*/ type == OEMCrypto_BufferType_Secure*/
            void* handle;
            size_t max_length;
            size_t offset;
        } secure;
        struct {                   /*type == OEMCrypto_BufferType_Direct*/
            bool is_video;
        } direct;
    } buffer;
} Drm_WVOemCrypto_DestBufferDesc;

typedef enum Drm_WVOemCryptoAlgorithm
{
    Drm_WVOemCryptoAlgorithm_AES_ECB                = 0,
    Drm_WVOemCryptoAlgorithm_HMAC_SHA256            = 1,
    Drm_WVOemCryptoAlgorithm_AES_CBC_128_NO_PADDING = 2
} Drm_WVOemCryptoAlgorithm;

typedef struct Drm_WVoemCryptoKeySlot_t
{
    NEXUS_KeySlotHandle hSwKeySlot;
    uint32_t keySlotID;
} Drm_WVoemCryptoKeySlot_t;

typedef struct Drm_WVOemCryptoHostSessionCtx_t
{
    uint32_t session_id;
    uint8_t key_id[16];
    size_t key_id_length;
    DrmCommonOperationStruct_t drmCommonOpStruct;
    Drm_WVOemCryptoCipherMode cipher_mode;
    Drm_WVoemCryptoKeySlot_t *key_slot_ptr[DRM_WVOEMCRYPTO_MAX_NUM_KEY_SLOT];
    uint32_t num_key_slots;
    uint8_t *btp_sage_buffer;
    uint32_t ext_iv_offset;
    bool new_key_selected;
    bool key_slot_error_notification;
    time_t decrypt_verify_time;
}Drm_WVOemCryptoHostSessionCtx_t;

typedef struct Drm_WVOemCryptoEncryptPattern_t {
  size_t encrypt;  /* number of 16 byte blocks to decrypt. */
  size_t skip;     /* number of 16 byte blocks to leave in clear. */
  size_t offset;   /* (Ignore) offset into the pattern in blocks for this call. */
} Drm_WVOemCryptoEncryptPattern_t;

#define SAGE_WVKBOX_DEVID_LEN 32
#define SAGE_WVKBOX_DEVKEY_LEN 16
#define SAGE_WVKBOX_KEYDATA_LEN 72
#define SAGE_WVKBOX_MAGICWD_LEN 4
#define SAGE_WVKBOX_CRC_LEN 4

/*//This has to be in sync with the error codes on the host in OEMCryptoCENC.h file*/
typedef enum Sage_OEMcryptoResult {
    SAGE_OEMCrypto_SUCCESS                            = 0,
    SAGE_OEMCrypto_ERROR_INIT_FAILED                  = 1,
    SAGE_OEMCrypto_ERROR_TERMINATE_FAILED             = 2,
    SAGE_OEMCrypto_ERROR_OPEN_FAILURE                 = 3,
    SAGE_OEMCrypto_ERROR_CLOSE_FAILURE                = 4,
    SAGE_OEMCrypto_ERROR_ENTER_SECURE_PLAYBACK_FAILED = 5,
    SAGE_OEMCrypto_ERROR_EXIT_SECURE_PLAYBACK_FAILED  = 6,
    SAGE_OEMCrypto_ERROR_SHORT_BUFFER                 = 7,
    SAGE_OEMCrypto_ERROR_NO_DEVICE_KEY                = 8,
    SAGE_OEMCrypto_ERROR_NO_ASSET_KEY                 = 9,
    SAGE_OEMCrypto_ERROR_KEYBOX_INVALID               = 10,
    SAGE_OEMCrypto_ERROR_NO_KEYDATA                   = 11,
    SAGE_OEMCrypto_ERROR_NO_CW                        = 12,
    SAGE_OEMCrypto_ERROR_DECRYPT_FAILED               = 13,
    SAGE_OEMCrypto_ERROR_WRITE_KEYBOX                 = 14,
    SAGE_OEMCrypto_ERROR_WRAP_KEYBOX                  = 15,
    SAGE_OEMCrypto_ERROR_BAD_MAGIC                    = 16,
    SAGE_OEMCrypto_ERROR_BAD_CRC                      = 17,
    SAGE_OEMCrypto_ERROR_NO_DEVICEID                  = 18,
    SAGE_OEMCrypto_ERROR_RNG_FAILED                   = 19,
    SAGE_OEMCrypto_ERROR_RNG_NOT_SUPPORTED            = 20,
    SAGE_OEMCrypto_ERROR_SETUP                        = 21,
    SAGE_OEMCrypto_ERROR_OPEN_SESSION_FAILED          = 22,
    SAGE_OEMCrypto_ERROR_CLOSE_SESSION_FAILED         = 23,
    SAGE_OEMCrypto_ERROR_INVALID_SESSION              = 24,
    SAGE_OEMCrypto_ERROR_NOT_IMPLEMENTED              = 25,
    SAGE_OEMCrypto_ERROR_NO_CONTENT_KEY               = 26,
    SAGE_OEMCrypto_ERROR_CONTROL_INVALID              = 27,
    SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE              = 28,
    SAGE_OEMCrypto_ERROR_INVALID_CONTEXT              = 29,
    SAGE_OEMCrypto_ERROR_SIGNATURE_FAILURE            = 30,
    SAGE_OEMCrypto_ERROR_TOO_MANY_SESSIONS            = 31,
    SAGE_OEMCrypto_ERROR_INVALID_NONCE                = 32,
    SAGE_OEMCrypto_ERROR_TOO_MANY_KEYS                = 33,
    SAGE_OEMCrypto_ERROR_DEVICE_NOT_RSA_PROVISIONED   = 34,
    SAGE_OEMCrypto_ERROR_INVALID_RSA_KEY              = 35,
    SAGE_OEMCrypto_ERROR_KEY_EXPIRED                  = 36,
    SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES       = 37,
    SAGE_OEMCrypto_ERROR_INSUFFICIENT_HDCP            = 38,
    SAGE_OEMCrypto_ERROR_BUFFER_TOO_LARGE             = 39,
    SAGE_OEMCrypto_WARNING_GENERATION_SKEW            = 40,  // Warning, not an error.
    SAGE_OEMCrypto_ERROR_GENERATION_SKEW              = 41,
    SAGE_OEMCrypto_LOCAL_DISPLAY_ONLY                 = 42,  // Info, not an error.
    SAGE_OEMCrypto_ERROR_ANALOG_OUTPUT                = 43,
    SAGE_OEMCrypto_ERROR_WRONG_PST                    = 44,
    SAGE_OEMCrypto_ERROR_WRONG_KEYS                   = 45,
    SAGE_OEMCrypto_ERROR_MISSING_MASTER               = 46,
    SAGE_OEMCrypto_ERROR_LICENSE_INACTIVE             = 47,
    SAGE_OEMCrypto_ERROR_ENTRY_NEEDS_UPDATE           = 48,
    SAGE_OEMCrypto_ERROR_ENTRY_IN_USE                 = 49,
    SAGE_OEMCrypto_ERROR_USAGE_TABLE_UNRECOVERABLE    = 50,  // Reserved. Do not use.

} Sage_OEMCryptoResult;


void DRM_WVOEMCrypto_GetDefaultParamSettings(Drm_WVOemCryptoParamSettings_t *pOemCryptoParamSettings);

void DRM_WVOemCrypto_SetParamSettings(Drm_WVOemCryptoParamSettings_t *pWvOemCryptoParamSettings);
DrmRC DRM_WVOemCrypto_UnInit(int *wvRc);
DrmRC DRM_WVOemCrypto_Initialize(Drm_WVOemCryptoParamSettings_t *pWvOemCryptoParamSettings,int *wvRc);
DrmRC DRM_WVOemCrypto_OpenSession(uint32_t* session,int *wvRc);
DrmRC drm_WVOemCrypto_CloseSession(uint32_t session,int *wvRc);
DrmRC drm_WVOemCrypto_GenerateNonce(uint32_t session,
                                    uint32_t* nonce,
                                    int *wvRc);

DrmRC drm_WVOemCrypto_GenerateDerivedKeys(uint32_t session,
                                            const uint8_t* mac_key_context,
                                            uint32_t mac_key_context_length,
                                            const uint8_t* enc_key_context,
                                            uint32_t enc_key_context_length,
                                            int *wvRc);

DrmRC drm_WVOemCrypto_GenerateSignature(
                            uint32_t session,
                            const uint8_t* message,
                            size_t message_length,
                            uint8_t* signature,
                            size_t* signature_length,
                            int *wvRc);

DrmRC drm_WVOemCrypto_LoadKeys(uint32_t session,
                                   const uint8_t* message,
                                   uint32_t message_length,
                                   const uint8_t* signature,
                                   uint32_t signature_length,
                                   const uint8_t* enc_mac_key_iv,
                                   const uint8_t* enc_mac_keys,
                                   uint32_t num_keys,
                                   void* key_array,
                                   const uint8_t* pst,
                                   uint32_t pst_length,
                                   const uint8_t* srm_requirement,
                                   int * wVRc);

DrmRC drm_WVOemCrypto_LoadKeys_V11_or_V12(uint32_t session,
                                          const uint8_t* message,
                                          uint32_t message_length,
                                          const uint8_t* signature,
                                          uint32_t signature_length,
                                          const uint8_t* enc_mac_key_iv,
                                          const uint8_t* enc_mac_keys,
                                          uint32_t num_keys,
                                          void* key_array,
                                          const uint8_t* pst,
                                          uint32_t pst_length,
                                          int * wVRc);

DrmRC drm_WVOemCrypto_LoadKeys_V9_or_V10(uint32_t session,
                                   const uint8_t* message,
                                   uint32_t message_length,
                                   const uint8_t* signature,
                                   uint32_t signature_length,
                                   const uint8_t* enc_mac_key_iv,
                                   const uint8_t* enc_mac_keys,
                                   uint32_t num_keys,
                                   void* key_array,
                                   const uint8_t* pst,
                                   uint32_t pst_length,
                                   int * wVRc);

DrmRC drm_WVOemCrypto_RefreshKeys(uint32_t session,
                                      const uint8_t* message,
                                      uint32_t message_length,
                                      const uint8_t* signature,
                                      uint32_t signature_length,
                                      uint32_t num_keys,
                                      void* key_array,
                                      int * wVRc);

DrmRC drm_WVOemCrypto_SelectKey(const uint32_t session,
                                    const uint8_t* key_id,
                                    uint32_t key_id_length,
                                    int *wVRc);

DrmRC drm_WVOemCrypto_InstallKeybox(const uint8_t* keybox,
                                    uint32_t keyBoxLength);

DrmRC drm_WVOemCrypto_IsKeyboxValid(int *wvRc);

DrmRC drm_WVOemCrypto_GetDeviceID(uint8_t* deviceID,
                                  size_t* idLength,int*wvRc);

DrmRC drm_WVOemCrypto_GetKeyData(uint8_t* keyData,
                                 size_t* keyDataLength,int *wvRc);

DrmRC drm_WVOemCrypto_GetRandom(uint8_t* randomData, uint32_t dataLength);

DrmRC drm_WVOemCrypto_WrapKeybox(const uint8_t* keybox,
                                     uint32_t keyBoxLength,
                                     uint8_t* wrappedKeybox,
                                     uint32_t* wrappedKeyBoxLength,
                                     const uint8_t* transportKey,
                                     uint32_t transportKeyLength);

DrmRC drm_WVOemCrypto_RewrapDeviceRSAKey(uint32_t session,
                                             const uint8_t* message,
                                             uint32_t message_length,
                                             const uint8_t* signature,
                                             uint32_t signature_length,
                                             const uint32_t* nonce,
                                             const uint8_t* enc_rsa_key,
                                             uint32_t enc_rsa_key_length,
                                             const uint8_t* enc_rsa_key_iv,
                                             uint8_t* wrapped_rsa_key,
                                             size_t*  wrapped_rsa_key_length,
                                             int *wvRc);

DrmRC drm_WVOemCrypto_LoadDeviceRSAKey(uint32_t session,
                                           const uint8_t* wrapped_rsa_key,
                                           uint32_t wrapped_rsa_key_length,int *wvRc);

typedef uint8_t WvOemCryptoRSA_Padding_Scheme;
DrmRC drm_WVOemCrypto_GenerateRSASignature(uint32_t session,
                                               const uint8_t* message,
                                               uint32_t message_length,
                                               uint8_t* signature,
                                               size_t* signature_length,
                                               WvOemCryptoRSA_Padding_Scheme padding_scheme,
                                               int *wvRc);

DrmRC drm_WVOemCrypto_DeriveKeysFromSessionKey(
                                    uint32_t session,
                                    const uint8_t* enc_session_key,
                                    uint32_t enc_session_key_length,
                                    const uint8_t* mac_key_context,
                                    uint32_t mac_key_context_length,
                                    const uint8_t* enc_key_context,
                                    uint32_t enc_key_context_length,
                                    int *wvRc);


DrmRC drm_WVOemCrypto_GetHDCPCapability(uint32_t *current, uint32_t *maximum, int *wvRc);

DrmRC drm_WVOemCrypto_Generic_Encrypt(uint32_t session,
                                          const uint8_t* in_buffer,
                                          uint32_t buffer_length,
                                          const uint8_t* iv,
                                          Drm_WVOemCryptoAlgorithm algorithm,
                                          uint8_t* out_buffer,
                                          int *wvRc);

DrmRC drm_WVOemCrypto_Generic_Decrypt(uint32_t session,
                                          const uint8_t* in_buffer,
                                          uint32_t buffer_length,
                                          const uint8_t* iv,
                                          Drm_WVOemCryptoAlgorithm algorithm,
                                          uint8_t* out_buffer,
                                          int *wvRc);

DrmRC drm_WVOemCrypto_Generic_Sign(uint32_t session,
                                       const uint8_t* in_buffer,
                                       uint32_t buffer_length,
                                       int algorithm,
                                       uint8_t* signature,
                                       size_t* signature_length,
                                       int *wvRc);

DrmRC drm_WVOemCrypto_Generic_Verify(uint32_t session,
                                         const uint8_t* in_buffer,
                                         uint32_t buffer_length,
                                         int algorithm,
                                         const uint8_t* signature,
                                         uint32_t signature_length,
                                         int *wvRc) ;

/**********************************************************************************************
 * DRM_OEMCrypto_SupportsUsageTable
 *   This is used to determine if the device can support a usage table.
 *
 * PARAMETERS:
 *     none
 *
 * RETURNS:
 *     SAGE_OEMCrypto_SUCCESS supported
 *     SAGE_OEMCrypto_ERROR_NOT_IMPLEMENTED
 *
 * This method changed in API version 9.
 * ***********************************************************************/
DrmRC DRM_WVOemCrypto_SupportsUsageTable(int *wvRc);


/**********************************************************************************************
 * DRM_OEMCrypto_UpdateUsageTable
 * Propagate values from all open sessions to the Session Usage Table. If
 * any values have changed, increment the generation number, sign, and save the table. During
 * playback, this function will be called approximately once per minute.
 * Devices that do not implement a Session Usage Table may return
 * OEMCrypto_ERROR_NOT_IMPLEMENTED.
 *
 * This function will not be called simultaneously with any session functions.
 *
 * PARAMETERS:
 *     none
 *
 * RETURNS:
 *     SAGE_OEMCrypto_SUCCESS success
 *     SAGE_OEMCrypto_ERROR_NOT_IMPLEMENTED
 *     SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE
 *
 * This method changed in API version 9.
 * ***********************************************************************/
DrmRC DRM_WVOemCrypto_UpdateUsageTable(int *wvRc);


/***************************************************************************************
 * DRM_WVOemCrypto_DeactivateUsageEntry
 *
 * Find the entry in the Usage Table with a matching PST. Mark the status of that entry as
 * "inactive". If it corresponds to an open session, the status of that session will also be marked
 * as "inactive".
 * Increment Usage Table's generation number, sign, encrypt, and save the Usage Table.
 *
 * PARAMETERS:
 * [in] pst: pointer to memory containing Provider Session Token.
 * [in] pst_length: length of the pst, in bytes.
 *
 * RETURNS:
 * SAGE_OEMCrypto_SUCCESS success
 * SAGE_OEMCrypto_ERROR_INVALID_CONTEXT (no entry has matching PST.)
 * SAGE_OEMCrypto_ERROR_NOT_IMPLEMENTED
 * SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE
 *
 *
 * **************************************************************************************************/
DrmRC DRM_WVOemCrypto_DeactivateUsageEntry_V12(uint8_t *pst,
                                               uint32_t pst_length,
                                               int *wvRc);




typedef struct WvOEMCryptoPstReport
{
    uint8_t signature[20];     /* HMAC SHA1 of the rest of the report.*/
    uint8_t status; /* current status of pst entry.*/
    uint8_t clock_security_level;
    uint8_t pst_length;
    uint8_t padding;   /* make int64's word aligned. */
    uint64_t seconds_since_license_received; /* (present_time - time_of_license_received) */
    uint64_t seconds_since_first_decrypt; /* (present_time - time_of_first_decrypt) */
    uint64_t seconds_since_last_decrypt; /* (present_time - time_of_last_decrypt) */
    uint8_t pst[];
}WvOEMCryptoPstReport;

/*********************************************************************************
 * DRM_WVOemCrypto_ReportUsage
 *
 * Increment Usage Table's generation number, sign, encrypt, and save the Usage Table.
 *
 * PARAMETERS:
 * [in] sessionContext: handle for the session to be used.
 * [in] pst: pointer to memory containing Provider Session Token.
 * [in] pst_length: length of the pst, in bytes.
 * [out] buffer: pointer to buffer in which usage report should be stored.
 *               May be null on the first call in order to find required buffer size.
 * [in/out] buffer_length: (in) length of the report buffer, in bytes.
 * (out) actual length of the report
 *
 * RETURNS:
 * SAGE_OEMCrypto_SUCCESS success
 * SAGE_OEMCrypto_ERROR_SHORT_BUFFER (buffer not large enough to hold output signature)
 * SAGE_OEMCrypto_ERROR_INVALID_SESSION (no open session with specified id)
 * SAGE_OEMCrypto_ERROR_INVALID_CONTEXT (no entry has matching PST)
 * SAGE_OEMCrypto_ERROR_NOT_IMPLEMENTED
 * SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE
 *
 *
 ********************************************************************/
DrmRC DRM_WVOemCrypto_ReportUsage(uint32_t sessionContext,
                                  const uint8_t *pst,
                                  uint32_t pst_length,
                                  WvOEMCryptoPstReport *buffer,
                                  size_t *buffer_length,
                                  int *wvRc);


/***********************************************************************************************
 * DRM_WVOemCrypto_DeleteUsageEntry
 *
 * Verifies the signature of the given message using the session's mac_key[server]
 * and the algorithm HMACSHA256, then deletes an entry from the session table. The
 * session should already be associated with the given entry, from a previous call to
 * DRM_WVOemCrypto_ReportUsage.
 * After performing all verifications, and deleting the entry from the Usage Table,
 * the Usage Table generation number is incrememented, then signed, encrypted, and saved.
 *
 * PARAMETERS:
 * [in] sessionContext: handle for the session to be used.
 * [in] pst: pointer to memory containing Provider Session Token.
 * [in] pst_length: length of the pst, in bytes.
 * [in] message: pointer to memory containing message to be verified.
 * [in] message_length: length of the message, in bytes.
 * [in] signature: pointer to memory containing the signature.
 * [in] signature_length: length of the signature, in bytes.
 *
 * RETURNS:
 * SAGE_OEMCrypto_SUCCESS success
 * SAGE_OEMCrypto_ERROR_INVALID_SESSION no open session with that id.
 * SAGE_OEMCrypto_ERROR_SIGNATURE_FAILURE
 * SAGE_OEMCrypto_ERROR_NOT_IMPLEMENTED
 * SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE
 *
 ******************************************************************************************/
DrmRC DRM_WVOemCrypto_DeleteUsageEntry(uint32_t sessionContext,
                                        const uint8_t* pst,
                                        uint32_t pst_length,
                                        const uint8_t *message,
                                        uint32_t message_length,
                                        const uint8_t *signature,
                                        uint32_t signature_length,
                                        int *wvRc);


/*****************************************************************************************
 * DRM_WVOemCrypto_DeleteUsageTable
 *
 * This is called when the CDM system believes there are major problems or resource issues.
 * The entire table should be cleaned and a new table should be created.
 *
 * PARAMETERS:
 * none
 *
 * RETURNS:
 * SAGE_OEMCrypto_SUCCESS success
 * SAGE_OEMCrypto_ERROR_NOT_IMPLEMENTED
 * SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE
 *
 ****************************************************************************************/
DrmRC DRM_WVOemCrypto_DeleteUsageTable(int *wvRc);


/*WV v10*/
/*****************************************************************************************
 * DRM_WVOemCrypto_GetNumberOfOpenSessions
 *
 * Returns the current number of open sessions. The CDM and OEMCrypto consumers can query
 * this value so they can use resources more effectively
 *
 * PARAMETERS:
 * [out]noOfOpenSessions:this is the current number of opened sessions
 *
 * RETURNS:
 * SAGE_OEMCrypto_SUCCESS success
 * SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE
 *
 ****************************************************************************************/
DrmRC DRM_WVOemCrypto_GetNumberOfOpenSessions(uint32_t* noOfOpenSessions,int *wvRc);

/*****************************************************************************************
 * DRM_WVOemCrypto_GetMaxNumberOfSessions
 *
 * Returns the maximum number of concurrent OEMCrypto sessions supported by the device.
 *
 * PARAMETERS:
 * [noOfMaxSessions]noOfOpenSessions:this is the current number of opened sessions
 *
 * RETURNS:
 * SAGE_OEMCrypto_SUCCESS success
 * SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE
 *
 ****************************************************************************************/
DrmRC DRM_WVOemCrypto_GetMaxNumberOfSessions(uint32_t* noOfMaxSessions,int *wvRc);

/*****************************************************************************************
 * Drm_WVOemCrypto_CopyBuffer
 *
 * Copies the payload in the buffer referenced by the *data_addr parameter into the buffer
 * referenced by the destination parameter. The data is simply copied.
 *
 * PARAMETERS:
 * [in] data_addr: An unaligned pointer to the buffer to be copied.
 * [in] data_length: The length of the buffer, in bytes.
 * [in] destination: A callerowned buffer where the data has to be copied
 * [in] buffer_type: The type of output buffer memory
 * [in] subsample_flags: bitwise flags indicating if this is the first, middle,
 *      or last subsample in a chunk of data. 1 = first subsample, 2 = last
 *      subsample, 3 = both first and last subsample, 0 = neither first nor
 *      last subsample.
 *
 * RETURNS:
 * SAGE_OEMCrypto_SUCCESS success
 * SAGE_OEMCrypto_ERROR_INVALID_CONTEXT
 * SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES
 * SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE
 *
 ****************************************************************************************/
DrmRC Drm_WVOemCrypto_CopyBuffer(uint8_t* destination,
                           const uint8_t* data_addr,
                           uint32_t data_length,
                           Drm_WVOemCryptoBufferType_t buffer_type,
                           uint32_t subsample_flags
                           );

/*****************************************************************************************
 * Drm_WVOemCrypto_QueryKeyControl
 *
 * Returns the decrypted key control block for the given key_id.the returned key control block has
 * to be in network byte order
 *
 * PARAMETERS:
 * [in] session: Current session Id
 * [in] key_id: The unique id of the key of interest.
 * [in] key_id_length: The length of key_id, in bytes.
 * [out] key_control_block: A caller owned buffer to return teh keycontrol block.
 * [in/out] key_control_block_length. The length of key_control_block buffer.
 *
 * RETURNS:
 * SAGE_OEMCrypto_SUCCESS
 * SAGE_OEMCrypto_ERROR_INVALID_CONTEXT
 * SAGE_OEMCrypto_ERROR_INSUFFICIENT_RESOURCES
 * SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE
 *
 ****************************************************************************************/
DrmRC Drm_WVOemCrypto_QueryKeyControl(uint32_t session,
                                      const uint8_t* key_id,
                                      uint32_t key_id_length,
                                      uint8_t* key_control_block,
                                      uint32_t* key_control_block_length,
                                      int*wvRc);


/***********************************************************************************************
 * DRM_WVOemCrypto_ForceDeleteUsageEntry
 *
 * This function deletes an entry from the session usage table. This will be used for stale entries
 * without a signed request from the server, hence sifnature verification is not needed
 *
 * PARAMETERS:
 * [in] pst: pointer to memory containing Provider Session Token.
 * [in] pst_length: length of the pst, in bytes.
 *
 * RETURNS:
 * SAGE_OEMCrypto_SUCCESS
 * SAGE_OEMCrypto_ERROR_NOT_IMPLEMENTED
 * SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE
 *
 ******************************************************************************************/
DrmRC DRM_WVOemCrypto_ForceDeleteUsageEntry( const uint8_t* pst,
                                        uint32_t pst_length,
                                        int *wvRc);

/*****************************************************************************************
 * DRM_WVOemCrypto_Security_Patch_Level
 *
 * Returns the current OEMCrypto security patch level supported.
 *
 * PARAMETERS:
 * [security_patch_level]: Level of security patch
 *
 * RETURNS:
 * SAGE_OEMCrypto_SUCCESS success
 * SAGE_OEMCrypto_ERROR_UNKNOWN_FAILURE
 *
 ****************************************************************************************/
DrmRC DRM_WVOemCrypto_Security_Patch_Level(uint8_t* security_patch_level,int *wvRc);

/****************************************************************************************************
Description:
Decrypts (AES128CTR) or copies the payload in the buffer referenced by the *data_addr
parameter into the buffer referenced by the out_buffer parameter, using the session context
indicated by the session parameter.

Parameters:
[in] session: crypto session identifier.
[in] data_addr: An unaligned pointer to this segment of the stream.
[in] data_length: The length of this segment of the stream, in bytes.
[in] is_encrypted: True if the buffer described by data_addr, data_length is encrypted. If
is_encrypted is false, only the data_addr and data_length parameters are used. The iv and offset
arguments are ignored.
[in] iv: The initial value block to be used for content decryption.
[in] block_offset: If nonzero, the decryption block boundary is different from the start of the data.
[in] out_buffer: A callerowned descriptor that specifies the handling of the decrypted byte
stream. See OEMCrypto_DestbufferDesc for details.
[in] subsample_flags: bitwise flags indicating if this is the first, middle, or last subsample in a
chunk of data. 1 = first subsample, 2 = last subsample, 3 = both first and last subsample, 0 =
neither first nor last subsample.

Returns:
OEMCrypto_SUCCESS
OEMCrypto_ERROR_NO_DEVICE_KEY
OEMCrypto_ERROR_INVALID_SESSION
OEMCrypto_ERROR_INVALID_CONTEXT
OEMCrypto_ERROR_DECRYPT_FAILED
OEMCrypto_ERROR_KEY_EXPIRED
OEMCrypto_ERROR_INSUFFICIENT_RESOURCES
OEMCrypto_ERROR_UNKNOWN_FAILURE

Threading:
This function may be called simultaneously with functions on other sessions, but not with other
functions on this session.
******************************************************************************************************/

DrmRC drm_WVOemCrypto_DecryptCTR_V10(uint32_t session,
                                     const uint8_t* data_addr,
                                     uint32_t data_length,
                                     bool is_encrypted,
                                     Drm_WVOemCryptoBufferType_t buffer_type,
                                     const uint8_t* iv,
                                     uint32_t block_offset,
                                     void* out_buffer,
                                     uint32_t *out_sz,
                                     uint8_t subsample_flags,
                                     int *wvRc);

/****************************************************************************************************
Description:
Decrypts (AES128CTR) or copies the payload in the buffer referenced by the *data_addr
parameter into the buffer referenced by the out_buffer parameter, using the session context
indicated by the session parameter.

Parameters:
[in] session: crypto session identifier.
[in] data_addr: An unaligned pointer to this segment of the stream.
[in] data_length: The length of this segment of the stream, in bytes.
[in] is_encrypted: True if the buffer described by data_addr, data_length is encrypted. If
is_encrypted is false, only the data_addr and data_length parameters are used. The iv and offset
arguments are ignored.
[in] iv: The initial value block to be used for content decryption.
[in] block_offset: If nonzero, the decryption block boundary is different from the start of the data.
[in] out_buffer: A callerowned descriptor that specifies the handling of the decrypted byte
stream. See OEMCrypto_DestbufferDesc for details.
[in] pattern: struct to describe the pattern of encrypt/clear blocks in 16 byte lengths.
[in] subsample_flags: bitwise flags indicating if this is the first, middle, or last subsample in a
chunk of data. 1 = first subsample, 2 = last subsample, 3 = both first and last subsample, 0 =
neither first nor last subsample.

Returns:
OEMCrypto_SUCCESS
OEMCrypto_ERROR_NO_DEVICE_KEY
OEMCrypto_ERROR_INVALID_SESSION
OEMCrypto_ERROR_INVALID_CONTEXT
OEMCrypto_ERROR_DECRYPT_FAILED
OEMCrypto_ERROR_KEY_EXPIRED
OEMCrypto_ERROR_INSUFFICIENT_RESOURCES
OEMCrypto_ERROR_UNKNOWN_FAILURE

Threading:
This function may be called simultaneously with functions on other sessions, but not with other
functions on this session.
******************************************************************************************************/
DrmRC drm_WVOemCrypto_DecryptCENC(uint32_t session,
                                     const uint8_t* data_addr,
                                     uint32_t data_length,
                                     bool is_encrypted,
                                     Drm_WVOemCryptoBufferType_t buffer_type,
                                     const uint8_t* iv,
                                     uint32_t block_offset,
                                     void* out_buffer,
                                     uint32_t *out_sz,
                                     void *pattern,
                                     uint8_t subsample_flags,
                                     int *wvRc);

DrmRC DRM_WVOemCrypto_Create_Usage_Table_Header(uint8_t *header_buffer,
                                                uint32_t *header_buffer_length,
                                                int* wvRc);

DrmRC DRM_WVOemCrypto_Load_Usage_Table_Header(const uint8_t *header_buffer,
                                              uint32_t header_buffer_length,
                                              int* wvRc);

DrmRC DRM_WVOemCrypto_Create_New_Usage_Entry(uint32_t session,
                                             uint32_t *usage_entry_number,
                                             int *wvRc);

DrmRC DRM_WVOemCrypto_Load_Usage_Entry(uint32_t session,
                                       uint32_t usage_entry_number,
                                       const uint8_t *buffer,
                                       uint32_t buffer_length,
                                       int *wvRc);

DrmRC DRM_WVOemCrypto_Update_Usage_Entry(uint32_t session,
                                         uint8_t* header_buffer,
                                         uint32_t *header_buffer_length,
                                         uint8_t *entry_buffer,
                                         uint32_t *entry_buffer_length,
                                         int *wvRc);

DrmRC DRM_WVOemCrypto_Deactivate_Usage_Entry(uint32_t session,
                                             uint8_t *pst,
                                             uint32_t pst_length,
                                             int *wvRc);

DrmRC DRM_WVOemCrypto_Shrink_Usage_Table_Header(uint32_t new_entry_count,
                                                uint8_t *header_buffer,
                                                uint32_t *header_buffer_length,
                                                int *wvRc);

DrmRC DRM_WVOemCrypto_Move_Entry(uint32_t session,
                                 uint32_t new_index,
                                 int *wvRc);

DrmRC DRM_WVOemCrypto_Copy_Old_Usage_Entry(uint32_t session,
                                           const uint8_t *pst,
                                           uint32_t pst_length,
                                           int *wvRc);

DrmRC DRM_WVOemCrypto_Create_Old_Usage_Entry(uint64_t time_since_license_received,
                                             uint64_t time_since_first_decrypt,
                                             uint64_t time_since_last_decrypt,
                                             uint8_t status,
                                             uint8_t *server_mac_key,
                                             uint8_t *client_mac_key,
                                             const uint8_t *pst,
                                             uint32_t pst_length,
                                             int *wvRc);

bool DRM_WVOemCrypto_IsAntiRollbackHwPresent(void);

#ifdef __cplusplus
}
#endif

#endif /*__DRM_WVOEMCRYPTO_TL_H__*/

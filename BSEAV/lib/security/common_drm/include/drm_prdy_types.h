/******************************************************************************
 * (c) 2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ******************************************************************************/
#ifndef DRM_PRDY_TYPES_H__
#define DRM_PRDY_TYPES_H__

#include "nexus_security.h"
#include "bstd.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** This macro will calculate the maximum value of an unsigned data type
** where the type is passed in as x.
** For example DRM_PRDY_MAX_UNSIGNED_TYPE( uint8_t  ) will evaluate to 0xFF
**             DRM_PRDY_MAX_UNSIGNED_TYPE( uint16_t ) will evaluate to 0xFFFFFFFF
*/
#define DRM_PRDY_MAX_UNSIGNED_TYPE(x) ( (x)~((x)0) )


#define DRM_PRDY_MAX_INCLUSION_GUIDS     (20)  /*  Must match the value of DRM_MAX_INCLUSION_GUIDS in the playready SDK */
#define DRM_PRDY_MAX_LICENSE_CHAIN_DEPTH (2)   /* Must match the value of DRM_MAX_LICENSE_CHAIN_DEPTH in the playready SDK */

#define DRM_PRDY_MAX_TRANSACTION_ID  (100)   /* Redefinition of DRM_MAX_TRANSACTION_ID in the playready SDK */
#define DRM_PRDY_MAX_LICENSE_ACK     (20)    /* Redefinition of DRM_MAX_LICENSE_ACK in the playready SDK */

#define DRM_PRDY_CLK_NOT_SET         (0)
#define DRM_PRDY_CLK_SET             (1)
#define DRM_PRDY_CLK_NEEDS_REFRESH   (2)
#define DRM_PRDY_CLK_NOT_PRESENT     (0xFFFFFFFF)

typedef struct PRDY_APP_CONTEXT         *DRM_Prdy_Handle_t;         /* opaque APP Context */

typedef struct DRM_Prdy_DecryptSettings_t {
    NEXUS_SecurityOperation opType;              /* Operation to perfrom */
    NEXUS_SecurityAlgorithm algType;             /* Crypto algorithm */
    NEXUS_SecurityAlgorithmVariant algVariant;   /* Cipher chain mode for selected cipher */
    NEXUS_SecurityKeyType keySlotType;           /* Key destination entry type */
    NEXUS_SecurityTerminationMode termMode;      /* Termination Type for residual block to be ciphered */
    bool enableExtKey;                           /* Flag used to enable external key loading during dma transfer on the key slot.
                                                    true = key will prepend data in the dma descriptors. */
    bool enableExtIv;                            /* Flag used to enable external IV loading during dma transfer on the key slot.
                                                    true = iv will prepend data in the dma descriptors. */
    NEXUS_SecurityAesCounterSize aesCounterSize; /* This member is required for AES counter mode  */
    NEXUS_SecurityCounterMode    aesCounterMode; /* for Zeus 3.0 and later */

} DRM_Prdy_DecryptSettings_t;

typedef struct DRM_Prdy_DecryptContextKey_t{
    NEXUS_KeySlotHandle  keySlot;
    unsigned             refCount; /*  number of of DecryptContext referencing that key handle. */
} DRM_Prdy_DecryptContextKey_t;

typedef struct DRM_Prdy_DecryptContext_t
{
    DRM_Prdy_DecryptContextKey_t *pKeyContext;
    void                *pDecrypt;    /* Pointer to MS SDK DRM_DECRYPT_CONTEXT*/
} DRM_Prdy_DecryptContext_t;

typedef enum DRM_Prdy_Error_e{
    DRM_Prdy_ok                         = 0,  /* successful call */
    DRM_Prdy_fail                       = 1,  /* call failed */
    DRM_Prdy_buffer_size                = 2,  /* buffer too small */
    DRM_Prdy_license_not_found          = 3,  /* license not found */
    DRM_Prdy_license_expired            = 4,  /* license expired */
    DRM_Prdy_domain_join                = 5,  /* Needs to join domain */
    DRM_Prdy_revocation_package_expired = 6,  /* Revocation package needs to be udapted */
    DRM_Prdy_invalid_header             = 7,  /* Invalid header */
    DRM_Prdy_xml_not_found              = 8,  /* Xml tag not found */
    DRM_Prdy_header_not_set             = 9,  /* Content Header not set. */
    DRM_Prdy_invalid_parameter          = 10, /* Invalid Parameter. */
    DRM_Prdy_no_policy                  = 11  /* No more policies queued */
}DRM_Prdy_Error_e;

typedef enum DRM_Prdy_ContentGetProperty_e {
    DRM_Prdy_contentGetProperty_eHeaderKid           = 0,
    DRM_Prdy_contentGetProperty_eHeaderType          = 1,
    DRM_Prdy_contentGetProperty_eHeader              = 2,
    DRM_Prdy_contentGetProperty_ePlayreadyObject     = 3,
    DRM_Prdy_contentGetProperty_eCipherType          = 4,
    DRM_Prdy_contentGetProperty_eDecryptorSetup      = 5
} DRM_Prdy_ContentGetProperty_e;

typedef enum DRM_Prdy_ContentSetProperty_e{
    DRM_Prdy_contentSetProperty_eHeaderNotSet        = 0,
    DRM_Prdy_contentSetProperty_eV1Header            = 1,
    DRM_Prdy_contentSetProperty_eV2Header            = 2,
    DRM_Prdy_contentSetProperty_eKID                 = 3,
    DRM_Prdy_contentSetProperty_eV2_4Header          = 5,
    DRM_Prdy_contentSetProperty_eV4Header            = 6,
    DRM_Prdy_contentSetProperty_eAutoDetectHeader    = 7,
    DRM_Prdy_contentSetProperty_ePlayreadyObj        = 8,
    DRM_Prdy_contentSetProperty_eV4_1Header          = 9,
    DRM_Prdy_contentSetProperty_ePlayreadyObjWithKID = 10,
    DRM_Prdy_contentSetProperty_eHeaderComponents    = 11
} DRM_Prdy_ContentSetProperty_e;

typedef enum DRM_Prdy_GetBuffer_Type_e {
    DRM_Prdy_getBuffer_content_property_header_kid      = 0,
    DRM_Prdy_getBuffer_content_property_header_type     = 1,
    DRM_Prdy_getBuffer_content_property_header          = 2,
    DRM_Prdy_getBuffer_content_property_playready_obj   = 3,
    DRM_Prdy_getBuffer_content_property_cipher_type     = 4,
    DRM_Prdy_getBuffer_content_property_decryptor_setup = 5,

    DRM_Prdy_getBuffer_licenseAcq_challenge             = 6,
    DRM_Prdy_getBuffer_metering_challenge               = 7,
    DRM_Prdy_getBuffer_client_info                      = 8,

    DRM_Prdy_getBuffer_Additional_Response_Data_Custom_Data = 9,
    DRM_Prdy_getBuffer_Additional_Response_Data_Redirect_Url = 10,
    DRM_Prdy_getBuffer_Additional_Response_Data_Service_Id = 11,
    DRM_Prdy_getBuffer_Additional_Response_Data_Account_Id = 12,

    DRM_Prdy_getBuffer_licenseAcq_ack_challenge             = 13,

    DRM_Prdy_getBuffer_licenseAcq_challenge_Netflix         = 14,
    DRM_Prdy_getBuffer_licenseAcq_challenge_NetflixWithLDL  = 15


} DRM_Prdy_GetBuffer_Type_e;

typedef struct DRM_Prdy_Init_t {
    char *  hdsFileName;
    char *  binFileName;
    char *  keyFileName;
    char *  keyHistoryFileName;
    char *  revocationListFileName;
    char *  defaultRWDirName;        /* Specifies a default directory for all read/write files. Overridden by above file name paths */
} DRM_Prdy_Init_t;

typedef struct DRM_Prdy_AES_CTR_Info_t {
    uint64_t  qwInitializationVector;
    uint64_t  qwBlockOffset;
    uint8_t   bByteOffset;  /* Byte offset within the current block */
}  DRM_Prdy_AES_CTR_Info_t;

typedef enum DRM_Prdy_policy_type_e {
    PLAY_OPL                       = 0x1,
    COPY_OPL                       = 0x2,
    INCLUSION_LIST                 = 0x3,
    EXTENDED_RESTRICTION_CONDITION = 0x4,
    EXTENDED_RESTRICTION_ACTION    = 0x5,
    EXTENDED_RESTRICTION_QUERY     = 0x6,
    SECURE_STATE_TOKEN_RESOLVE     = 0x7,
    RESTRICTED_SOURCEID            = 0x8
}DRM_Prdy_policy_type_e;


typedef struct DRM_Prdy_guid_t {
    uint32_t  Data1;
    uint16_t  Data2;
    uint16_t  Data3;
    uint8_t   Data4 [8];
} DRM_Prdy_guid_t;

typedef enum DRM_Prdy_License_Protocol_Type_e {
    DRM_Prdy_License_Protocol_Type_eUnknownProtocol      = 0,
    DRM_Prdy_License_Protocol_Type_eV2Protocol           = 1,
    DRM_Prdy_License_Protocol_Type_eV3Protocol           = 2
} DRM_Prdy_License_Protocol_Type_e;

typedef struct DRM_Prdy_License_Ack_t
{
    DRM_Prdy_guid_t   oKID;
    DRM_Prdy_guid_t   oLID;
    uint32_t          dwResult;
    uint32_t          dwFlags;
} DRM_Prdy_License_Ack_t;

typedef struct DRM_Prdy_License_Response_t
{
    DRM_Prdy_License_Protocol_Type_e eType;
    uint8_t                   rgbTransactionID[ DRM_PRDY_MAX_TRANSACTION_ID ];
    uint32_t                  cbTransactionID;
    DRM_Prdy_License_Ack_t    rgoAcks[ DRM_PRDY_MAX_LICENSE_ACK ];
    DRM_Prdy_License_Ack_t   *pAcks;
    uint32_t                  cMaxAcks;
    uint32_t                  cAcks;
    DRM_Prdy_Error_e          dwResult;

} DRM_Prdy_License_Response_t;

typedef struct DRM_Prdy_Id_t
{
    uint8_t rgb [16];
} DRM_Prdy_Id_t;

typedef struct DRM_Prdy_subSample_t {
    uint8_t   *sample;
    uint32_t   size;
} DRM_Prdy_subSample_t;

typedef struct DRM_Prdy_samples_t {
    DRM_Prdy_subSample_t  *subsamples;
    uint32_t               numOfSubsamples;
} DRM_Prdy_sample_t;

typedef struct DRM_Prdy_video_out_protection_t{
    DRM_Prdy_guid_t guidId;
    uint8_t     bConfigData;
} DRM_Prdy_video_out_protection_t;


typedef struct DRM_Prdy_opl_play_t{

    uint32_t i_compressedDigitalVideo;    /* Mimimum output protection level to play compressed digital video */
    uint32_t i_uncompressedDigitalVideo;  /* Mimimum output protection level to play uncompressed digital video */
    uint32_t i_analogVideo;               /* Mimimum output protection level to play analog video */
    uint32_t i_compressedDigitalAudio;    /* Mimimum output protection level to play compressed digital audio */
    uint32_t i_uncompressedDigitalAudio;  /* Mimimum output protection level to play uncompressed digital video */

    /*oplIdReserved, refer to MS porting kit documentation on DRM_PLAY_OPL for more details */
    uint32_t         i_resv_cIds;         /* Number of ids*/
    DRM_Prdy_guid_t *i_resv_rgIds;        /* Output protection level id reserved guid */

    /* Video output protection ids. */
    uint32_t                         i_vop_cEntries; /* Number of video output protections ids. */
    DRM_Prdy_video_out_protection_t *i_vop;          /* Pointer to array of video output protections ids. */

}DRM_Prdy_opl_play_t;

/* DRM_PLAY_OPL_EX2 */

typedef struct DRM_Prdy_Opl_Output_Ids_t
{
    uint16_t  cIds;
    DRM_Prdy_guid_t  *rgIds;

} DRM_Prdy_Opl_Output_Ids_t;

typedef struct DRM_Prdy_Minimum_Output_Protection_Levels_t
{
    uint16_t wCompressedDigitalVideo;
    uint16_t wUncompressedDigitalVideo;
    uint16_t wAnalogVideo;
    uint16_t wCompressedDigitalAudio;
    uint16_t wUncompressedDigitalAudio;

} DRM_Prdy_Minimum_Output_Protection_Levels_t;

typedef struct DRM_Prdy_output_protection_ex_t
{
    uint32_t           dwVersion;
    DRM_Prdy_guid_t    guidId;
    uint32_t           dwConfigData;
} DRM_Prdy_output_protection_ex_t;

typedef DRM_Prdy_output_protection_ex_t DRM_Prdy_audio_out_protection_ex_t;
typedef DRM_Prdy_output_protection_ex_t DRM_Prdy_video_out_protection_ex_t;

typedef struct DRM_Prdy_video_out_protection_ids_ex_t
{
    uint32_t    dwVersion;
    uint16_t    cEntries;
    DRM_Prdy_video_out_protection_ex_t *rgVop;

} DRM_Prdy_video_out_protection_ids_ex_t;

typedef struct DRM_Prdy_audio_out_protection_ids_ex_t
{
    uint32_t    dwVersion;
    uint16_t    cEntries;
    DRM_Prdy_audio_out_protection_ex_t *rgAop;

} DRM_Prdy_audio_out_protection_ids_ex_t;

typedef struct DRM_Prdy_opl_play_ex2_t{
    uint32_t                                    dwVersion;
    DRM_Prdy_Minimum_Output_Protection_Levels_t minOPL;
    DRM_Prdy_Opl_Output_Ids_t                   oplIdReserved;
    DRM_Prdy_video_out_protection_ids_ex_t      vopi;
    DRM_Prdy_audio_out_protection_ids_ex_t      aopi;
}DRM_Prdy_opl_play_ex2_t;


typedef struct DRM_Prdy_opl_copy_t
{
    uint32_t         i_minimumCopyLevel;    /* Minimum copy level */
    uint32_t         i_includes_cIds;       /* Number of inclusions ids*/
    DRM_Prdy_guid_t *i_includes_rgIds;      /* Guid of inclusions on output protections levels */
    uint32_t         i_excludes_cIds;       /* Number of restriction ids*/
    DRM_Prdy_guid_t *i_excludes_rgIds;      /* Guid of restrictons on output protections levels */

}DRM_Prdy_opl_copy_t;


typedef uint32_t DRM_Prdy_bool_t;


typedef struct DRM_Prdy_xmr_dword_t
{
    DRM_Prdy_bool_t  fValid;
    uint32_t         dwValue;
} DRM_Prdy_xmr_dword_t;


typedef struct DRM_Prdy_xmr_word_t
{
    DRM_Prdy_bool_t  fValid;
    uint16_t         wValue;
} DRM_Prdy_xmr_word_t;

typedef struct DRM_Prdy_xmr_dword_versioned_t
{
    DRM_Prdy_bool_t  fValid;
    uint32_t         dwVersion;
    uint32_t         dwValue;
} DRM_Prdy_xmr_dword_versioned_t;

typedef struct DRM_Prdy_xmr_guid_t
{
    DRM_Prdy_bool_t  fValid;
    uint8_t         *pguidBuffer;
    uint32_t         iGuid;
} DRM_Prdy_xmr_guid_t;

typedef struct DRM_Prdy_xmr_guidlist_t
{
    DRM_Prdy_bool_t  fValid;
    uint32_t         cGUIDs;
    uint8_t         *pguidBuffer;
    uint32_t         iGuids;
} DRM_Prdy_xmr_guidlist_t;

typedef struct DRM_Prdy_xmr_bytearray_t
{
    DRM_Prdy_bool_t  fValid;
    uint32_t         cbData;
    uint8_t         *pbDataBuffer;
    uint32_t         iData;
} DRM_Prdy_xmr_bytearray_t;

typedef struct DRM_Prdy_xmr_empty_t
{
    DRM_Prdy_bool_t  fValid;
} DRM_Prdy_xmr_empty_t;

typedef struct DRM_Prdy_xmr_expiration_t
{
    DRM_Prdy_bool_t  fValid;
    uint32_t         dwBeginDate;
    uint32_t         dwEndDate;
} DRM_Prdy_xmr_expiration_t;


typedef struct DRM_Prdy_xmr_minimum_environment_t
{
    DRM_Prdy_bool_t  fValid;
    uint32_t         dwVersion;
    uint16_t         wMinimumSecurityLevel;
    uint32_t         dwMinimumAppRevocationListVersion;
    uint32_t         dwMinimumDeviceRevocationListVersion;
} DRM_Prdy_xmr_minimum_environment_t;

typedef DRM_Prdy_xmr_dword_t           DRM_Prdy_xmr_issuedate_t;
typedef DRM_Prdy_xmr_dword_t           DRM_Prdy_xmr_grace_period_t;
typedef DRM_Prdy_xmr_guid_t            DRM_Prdy_xmr_metering_t;
typedef DRM_Prdy_xmr_dword_versioned_t DRM_Prdy_xmr_expiration_after_firstuse_t;
typedef DRM_Prdy_xmr_dword_t           DRM_Prdy_xmr_expiration_after_firststore_t;
typedef DRM_Prdy_xmr_guidlist_t        DRM_Prdy_xmr_inclusion_list_t;
typedef DRM_Prdy_xmr_bytearray_t       DRM_Prdy_xmr_serial_number_t;
typedef DRM_Prdy_xmr_word_t            DRM_Prdy_xmr_rights_t;
typedef DRM_Prdy_xmr_dword_versioned_t DRM_Prdy_xmr_revocation_informaiton_version_t;
typedef DRM_Prdy_xmr_dword_t           DRM_Prdy_xmr_priority_t;
typedef DRM_Prdy_xmr_dword_t           DRM_Prdy_xmr_sourceid_t;
typedef DRM_Prdy_xmr_empty_t           DRM_Prdy_xmr_restricted_sourceid_t;
typedef DRM_Prdy_xmr_word_t            DRM_Prdy_xmr_embedding_behavior_t;

typedef struct DRM_Prdy_xmr_unknown_obj_t
{
    DRM_Prdy_bool_t                fValid;
    uint16_t                       wType;
    uint16_t                       wFlags;
    uint8_t                       *pbBuffer;
    uint32_t                       ibData;
    uint32_t                       cbData;
    struct DRM_Prdy_xmr_unknown_obj_t *pNext;
} DRM_Prdy_xmr_unknown_obj_t;

typedef struct DRM_Prdy_xmr_unknown_container_t {
   DRM_Prdy_bool_t                            fValid;
   uint16_t                                   wType;
   uint16_t                                   wFlags;
   DRM_Prdy_xmr_unknown_obj_t                *pObject;
   struct DRM_Prdy_xmr_unknown_container_t   *pUnkChildcontainer;/* Linked list */
   struct DRM_Prdy_xmr_unknown_container_t   *pNext; /* Linked list */
} DRM_Prdy_xmr_unknown_container_t;


typedef struct DRM_Prdy_xmr_domain_id_t {
    DRM_Prdy_bool_t  fValid;
    uint8_t         *pbAccountID;
    uint32_t         ibAccountID;
    uint32_t         cbAccountID;
    uint32_t         dwRevision;
} DRM_Prdy_xmr_domain_id_t;

typedef struct DRM_Prdy_xmr_policy_metadata_obj_t {
    DRM_Prdy_bool_t  fValid;
    uint8_t         *pMetadataTypeGuidBuffer;
    uint32_t         iMetadataTypeGuid;
    uint32_t         cbPolicyData;
    uint8_t         *pbPolicyDataBuffer;
    uint32_t         iPolicyData;
} DRM_Prdy_xmr_policy_metadata_obj_t;

typedef struct DRM_Prdy_xmr_policy_metadata_list_t {
    DRM_Prdy_xmr_policy_metadata_obj_t          MetadataObject;
    struct DRM_Prdy_xmr_policy_metadata_list_t *pNext;
} DRM_Prdy_xmr_policy_metadata_list_t;

typedef struct DRM_Prdy_xmr_policy_metadata_t {
    DRM_Prdy_bool_t                      fValid;
    uint32_t                             cPolicyMetadataObjects;
    DRM_Prdy_xmr_policy_metadata_list_t *plistPolicyMetadataObjects;
} DRM_Prdy_xmr_policy_metadata_t;

typedef struct DRM_Prdy_xmr_global_requirements_t {
    DRM_Prdy_bool_t                                fValid;
    DRM_Prdy_xmr_minimum_environment_t             MinimumEnvironment;
    DRM_Prdy_xmr_serial_number_t                   SerialNumber;
    DRM_Prdy_xmr_rights_t                          Rights;
    DRM_Prdy_xmr_priority_t                        Priority;
    DRM_Prdy_xmr_sourceid_t                        SourceID;
    DRM_Prdy_xmr_restricted_sourceid_t             RestrictedSourceID;
    DRM_Prdy_xmr_expiration_t                      Expiration;
    DRM_Prdy_xmr_issuedate_t                       IssueDate;
    DRM_Prdy_xmr_grace_period_t                    GracePeriod;
    DRM_Prdy_xmr_metering_t                        Metering;
    DRM_Prdy_xmr_expiration_after_firstuse_t       ExpirationAfterUse;
    DRM_Prdy_xmr_expiration_after_firststore_t     ExpirationAfterStore;
    DRM_Prdy_xmr_inclusion_list_t                  InclusionList;
    DRM_Prdy_xmr_revocation_informaiton_version_t  RevocationInformationVersion;
    DRM_Prdy_xmr_domain_id_t                       DomainID;
    DRM_Prdy_xmr_embedding_behavior_t              EmbeddingBehavior;
    DRM_Prdy_xmr_unknown_obj_t                    *pUnknownObjects;
    DRM_Prdy_xmr_policy_metadata_t                 PolicyMetadata;
} DRM_Prdy_xmr_global_requirements_t;


typedef DRM_Prdy_xmr_dword_t DRM_Prdy_xmr_playcount_t;

typedef struct DRM_Prdy_xmr_output_config_t {
    DRM_Prdy_bool_t  fValid;
    uint8_t         *pguidBuffer;
    uint32_t         iGuid;
    uint32_t         cbConfigData;
    uint8_t         *pbConfigDataBuffer;
    uint32_t         iConfigData;
} DRM_Prdy_xmr_output_config_t;

typedef struct DRM_Prdy_xmr_output_config_list_t {
    DRM_Prdy_xmr_output_config_t              Config;
    struct DRM_Prdy_xmr_output_config_list_t *pNext;
} DRM_Prdy_xmr_output_config_list_t;

typedef struct DRM_Prdy_xmr_explicit_output_protection_t {
    DRM_Prdy_bool_t                    fValid;
    uint32_t                           cOutputProtectionIDs;
    DRM_Prdy_xmr_output_config_list_t *plistOutputConfiguration;
} DRM_Prdy_xmr_explicit_output_protection_t;

typedef DRM_Prdy_xmr_explicit_output_protection_t DRM_Prdy_xmr_explicit_analog_video_protection_t;
typedef DRM_Prdy_xmr_explicit_output_protection_t DRM_Prdy_xmr_explicit_digital_audio_protection_t;

typedef struct DRM_Prdy_xmr_minimium_output_protection_levels_t {
    DRM_Prdy_bool_t fValid;
    uint16_t        wCompressedDigitalVideo;
    uint16_t        wUncompressedDigitalVideo;
    uint16_t        wAnalogVideo;
    uint16_t        wCompressedDigitalAudio;
    uint16_t        wUncompressedDigitalAudio;
} DRM_Prdy_xmr_minimium_output_protection_levels_t;


typedef struct DRM_Prdy_xmr_playback_rights_t {
    DRM_Prdy_bool_t                                   fValid;
    DRM_Prdy_xmr_playcount_t                          PlayCount;
    DRM_Prdy_xmr_minimium_output_protection_levels_t  opl;
    DRM_Prdy_xmr_explicit_analog_video_protection_t   containerExplicitAnalogVideoProtection;
    DRM_Prdy_xmr_explicit_digital_audio_protection_t  containerExplicitDigitalAudioProtection;
    DRM_Prdy_xmr_unknown_obj_t                       *pUnknownObjects;
    DRM_Prdy_xmr_unknown_container_t                  UnknownContainer;
} DRM_Prdy_xmr_playback_rights_t;

typedef struct DRM_Prdy_xmr_copy_to_pc_t {
    DRM_Prdy_bool_t   fValid;
} DRM_Prdy_xmr_copy_to_pc_t;

typedef DRM_Prdy_xmr_dword_t               DRM_Prdy_xmr_move_enabler_t;
typedef DRM_Prdy_xmr_dword_versioned_t     DRM_Prdy_xmr_copy_count;
typedef DRM_Prdy_xmr_word_t                DRM_Prdy_xmr_copy_protection_level_t;

typedef struct DRM_Prdy_xmr_copy_rights_t {
    DRM_Prdy_bool_t                        fValid;
    uint32_t                               dwVersion;
    DRM_Prdy_xmr_copy_count                CopyCount;
    DRM_Prdy_xmr_copy_protection_level_t   CopyProtectionLevel;
    DRM_Prdy_xmr_move_enabler_t            MoveEnabler;
    DRM_Prdy_xmr_unknown_obj_t            *pUnknownObjects;
    DRM_Prdy_xmr_unknown_container_t       UnknownContainer;
} DRM_Prdy_xmr_copy_rights_t;


typedef struct DRM_Prdy_xmr_playlist_burn_restrictions_t {
    DRM_Prdy_bool_t  fValid;
    uint32_t         dwMaxPlaylistBurnCount;
    uint32_t         dwPlaylistBurnTrackCount;
} DRM_Prdy_xmr_playlist_burn_restrictions_t;

typedef struct DRM_Prdy_xmr_playlist_burn_rights_t {
    DRM_Prdy_bool_t                            fValid;
    DRM_Prdy_xmr_playlist_burn_restrictions_t  Restrictions;
    DRM_Prdy_xmr_unknown_obj_t                *pUnknownObjects;
} DRM_Prdy_xmr_playlist_burn_rights_t;


typedef struct DRM_Prdy_xmr_rsa_pubkey_t {
    DRM_Prdy_bool_t  fValid;
    uint32_t         dwExponent;
    uint16_t         cbModulus;
    uint8_t         *pbModulusBuffer;
    uint32_t         iModulus;
} DRM_Prdy_xmr_rsa_pubkey_t;

typedef DRM_Prdy_xmr_rsa_pubkey_t     DRM_Prdy_xmr_device_key_t;
typedef DRM_Prdy_xmr_rsa_pubkey_t     DRM_Prdy_xmr_rsa_license_granter_key_t;

typedef struct DRM_Prdy_xmr_userid_t {
    DRM_Prdy_bool_t   fValid;
    uint32_t          cbUserID;
    uint8_t          *pbUserID;
    uint32_t          iUserID;
} DRM_Prdy_xmr_userid_t;

typedef struct DRM_Prdy_xmr_revocation_t {
    DRM_Prdy_bool_t                         fValid;
    DRM_Prdy_xmr_rsa_license_granter_key_t  RsaLicenseGranterKey;
    DRM_Prdy_xmr_userid_t                   UserID;
} DRM_Prdy_xmr_revocation_t;

/* Structures for Key Material container */
typedef struct DRM_Prdy_xmr_content_key_t {
    DRM_Prdy_bool_t  fValid;
    uint8_t         *pbguidKeyID;
    uint32_t         iguidKeyID;
    uint16_t         wSymmetricCipherType;
    uint16_t         wKeyEncryptionCipherType;
    uint16_t         cbEncryptedKey;
    uint8_t         *pbEncryptedKeyBuffer;
    uint32_t         iEncryptedKey;
} DRM_Prdy_xmr_content_key_t;

typedef struct DRM_Prdy_xmr_optimized_content_key_t {
    DRM_Prdy_bool_t    fValid;
    uint16_t           wKeyEncryptionCipherType;
    uint16_t           cbEncryptedKey;
    uint8_t           *pbEncryptedKeyBuffer;
    uint32_t           iEncryptedKey;
} DRM_Prdy_xmr_optimized_content_key_t;

typedef struct DRM_Prdy_xmr_ecc_device_key_t {
    DRM_Prdy_bool_t  fValid;
    uint16_t         wEccCurveType;
    uint32_t         iKeyData;
    uint16_t         cbKeyData;
    uint8_t         *pbKeyData;
} DRM_Prdy_xmr_ecc_device_key_t;


typedef struct DRM_Prdy_xmr_uplink_kid_t {
    DRM_Prdy_bool_t  fValid;
    uint32_t         dwVersion;
    uint8_t         *pbguidUplinkKID;
    uint32_t         iguidUplinkKID;
    uint16_t         cbChainedCheckSum;
    uint8_t         *pbChainedCheckSumBuffer;
    uint32_t         iChainedCheckSum;
    uint16_t         wChecksumType;
} DRM_Prdy_xmr_uplink_kid_t;

typedef struct DRM_Prdy_xmr_key_material_t {
    DRM_Prdy_bool_t                       fValid;
    DRM_Prdy_xmr_content_key_t            ContentKey;
    DRM_Prdy_xmr_optimized_content_key_t  OptimizedContentKey;
    DRM_Prdy_xmr_device_key_t             DeviceKey;
    DRM_Prdy_xmr_ecc_device_key_t         ECCKey;
    DRM_Prdy_xmr_uplink_kid_t             UplinkKid;
} DRM_Prdy_xmr_key_material_t;

typedef struct DRM_Prdy_xmr_signature_t {
    DRM_Prdy_bool_t  fValid;
    uint16_t         wType;
    uint8_t         *pbSignatureBuffer;
    uint32_t         iSignature;
    uint16_t         cbSignature;
} DRM_Prdy_xmr_signature_t;

typedef DRM_Prdy_xmr_dword_t DRM_Prdy_xmr_generation_number_t;

typedef struct DRM_Prdy_xmr_outer_container_t {
    DRM_Prdy_bool_t                       fValid;
    DRM_Prdy_xmr_global_requirements_t    containerGlobalPolicies;
    DRM_Prdy_xmr_playback_rights_t        containerPlaybackPolicies;
    DRM_Prdy_xmr_copy_rights_t            containerCopyPolicies;
    DRM_Prdy_xmr_copy_to_pc_t             containerCopyToPCPolicies;
    DRM_Prdy_xmr_playlist_burn_rights_t   containerPlaylistBurnPolicies;
    DRM_Prdy_xmr_generation_number_t      generationNumber;
    DRM_Prdy_xmr_unknown_container_t      containerUnknown;
    DRM_Prdy_xmr_revocation_t             containerRevocation;
    DRM_Prdy_xmr_key_material_t           containerKeys;
    DRM_Prdy_xmr_signature_t              signature;
} DRM_Prdy_xmr_outer_container_t;

typedef struct DRM_Prdy_xmr_license_t {
    uint8_t                        *pbRightsIdBuffer;
    uint32_t                        iRightsId;
    uint32_t                        dwVersion;
    uint8_t                        *pbSignedDataBuffer;
    uint32_t                        iSignedData;
    uint32_t                        cbSignedData;
    DRM_Prdy_xmr_outer_container_t  containerOuter;
    uint8_t                        *pbXMRLic;
    uint32_t                        cbXMRLic;
} DRM_Prdy_xmr_license_t;


typedef struct DRM_Prdy_ext_restrict_callback_info_t {
    uint16_t                      wRightID;
    DRM_Prdy_xmr_unknown_obj_t   *pRestriction;
    DRM_Prdy_xmr_license_t       *pXMRLicense;
    void *                        pContextSST;
} DRM_Prdy_ext_restrict_callback_info_t;


typedef struct DRM_Prdy_inclusion_list_t {
    DRM_Prdy_guid_t  rgInclusionList       [DRM_PRDY_MAX_INCLUSION_GUIDS];
    DRM_Prdy_bool_t  rgfInclusionListValid [DRM_PRDY_MAX_INCLUSION_GUIDS][DRM_PRDY_MAX_LICENSE_CHAIN_DEPTH];
    uint32_t     dwChainDepth;
}DRM_Prdy_inclusion_list_t;


typedef unsigned short PRDY_DRM_WCHAR;

typedef struct DRM_Prday_const_string_t {
    const PRDY_DRM_WCHAR  *pwszString;
    uint32_t               cchString;
} DRM_Prdy_const_string_t;

typedef struct DRM_Prdy_byteblob_t {
    uint8_t        *pbBlob;
    uint32_t        cbBlob;
} DRM_Prdy_byteblob_t;

typedef struct DRM_Prdy_token_t {
    uint32_t TokenType;
    union
    {
        uint64_t                 u64DateTime;
        long                     lValue;
        DRM_Prdy_const_string_t  stringValue;
        uint32_t                 fnValue;
        DRM_Prdy_byteblob_t      byteValue;
    } val;
}DRM_Prdy_token_t;

typedef struct DRM_Prdy_secure_state_token_resolve_data_t {
    DRM_Prdy_const_string_t *pdstrAttributeName;
    DRM_Prdy_token_t        *pOnDisk;
    DRM_Prdy_token_t        *pOriginallyIntendedValue;
    DRM_Prdy_bool_t          fNewAttribute;
    uint16_t                 wRightID;
    uint16_t                 wRestrictionID;

}DRM_Prdy_secure_state_token_resolve_data_t;

typedef struct DRM_Prdy_restricted_sourceid_t {
    uint32_t dwSourceID;
}DRM_Prdy_restricted_sourceid_t;


typedef struct DRM_Prdy_policy_t {
    DRM_Prdy_policy_type_e type;
    union{
        DRM_Prdy_opl_play_ex2_t                     play;
        DRM_Prdy_opl_copy_t                         copy;
        DRM_Prdy_inclusion_list_t                   inc_list;
        DRM_Prdy_ext_restrict_callback_info_t       condition;
        DRM_Prdy_ext_restrict_callback_info_t       action;
        DRM_Prdy_ext_restrict_callback_info_t       query;
        DRM_Prdy_secure_state_token_resolve_data_t  token_res;
        DRM_Prdy_restricted_sourceid_t              restr_src;
    }t;
}DRM_Prdy_policy_t;

typedef enum DRM_Prdy_license_property_e {
    DRM_Prdy_license_property_min_app_sec = 1,  /* Minimum app security level */
    DRM_Prdy_license_property_source_id,        /* Source ID */
    DRM_Prdy_license_property_revinfo_version,  /* RevInfo version */
    DRM_Prdy_license_property_mid,              /* Numer of MIDs set in the ExtraData */
    DRM_Prdy_license_property_has_inclusion_guid,   /* If the GUID was found */
    DRM_Prdy_license_property_block_ndt         /* If the license should be blocked for NDT */
} DRM_Prdy_license_property_e;

typedef enum DRM_Prdy_license_right_e {
    eDRM_Prdy_license_right_none = 0,
    eDRM_Prdy_license_right_playback,
    eDRM_Prdy_license_right_collaborative_play,
    eDRM_Prdy_license_right_copy_to_cd,
    eDRM_Prdy_license_right_copy,
    eDRM_Prdy_license_right_create_thumbnail_image,
    eDRM_Prdy_license_right_move
} DRM_Prdy_license_right_e;

/* Num 100 nanosec ticks since Jan. 1, 1601 */
typedef struct DRM_Prdy_file_time_t {
    uint32_t dwFileTimeHigh;
    uint32_t dwFileTimeLow;
} DRM_Prdy_file_time_t;

/* Same structure as the SDK */
typedef struct DRM_Prdy_license_state_t {
    uint32_t   dwStreamId;      /* 0 -> All streams, != 0 -> A particular stream. */
    uint32_t   dwCategory;      /* Indicates the category of string to be displayed. */
    uint32_t   dwNumCounts;     /* Number of items supplied in dwCount. */
    uint32_t   dwCount[4];      /* Up to 4 counts. */
    uint32_t   dwNumDates;      /* Number of items supplied in date. */
    DRM_Prdy_file_time_t date[4];    /* Up to 4 dates. */
    uint32_t   dwVague;         /* 0 -> certain, 1 -> atleast.  (There could be more */
                                /*               licenses. Aggregation not possible.) */
} DRM_Prdy_license_state_t;

/***********************************************
 ** local license defines and data structures **
 ***********************************************/

#define DRM_PRDY_MAX_LOCAL_LICENSE_EXPLICIT_OUTPUT_PROTECTION_CONFIG_DATA  16
#define DRM_PRDY_MAX_LOCAL_LICENSE_EXPLICIT_OUTPUT_PROTECTIONS             10
#define DRM_PRDY_MAX_LOCAL_LICENSE_PLAY_ENABLERS                           10
#define DRM_PRDY_LOCAL_LICENSE_EXPIRATION_MIN_BEGIN_DATE                   0
#define DRM_PRDY_LOCAL_LICENSE_INFINITE_EXPIRATION                         DRM_PRDY_MAX_UNSIGNED_TYPE( uint16_t )

typedef void *DRM_Prdy_license_handle;
#define DRM_PRDY_LICENSE_HANDLE_INVALID   ((DRM_Prdy_license_handle)NULL)

typedef enum DRM_Prdy_local_license_type_e {
    eDRM_Prdy_eLocal_license_bound_simple,
    eDRM_Prdy_eLocal_license_local_bound_root,
    eDRM_Prdy_eLocal_license_leaf,
    eDRM_Prdy_eLocal_license_remote_bound_simple,
    eDRM_Prdy_eLocal_license_remote_boot
} DRM_Prdy_local_license_type_e;

typedef enum DRM_Prdy_local_license_store_e {
    eDRM_Prdy_elocal_license_xmr_store,
    eDRM_Prdy_elocal_license_prnd_session_store
} DRM_Prdy_local_license_store_e;


/*********************************************************************************************
 * struct DRM_Prdy_local_license_expiration_t
 *
 * The dwBeginDate and dwEndDate values are expressed in seconds.
 * The dwBeginDate value can be set to DRM_PRDY_LOCAL_LICENSE_EXPIRATION_MIN_BEGIN_DATE to
 * indicate that the current date and time is to to be used for the begin date. The
 * dwEndDate value can be set to DRM_PRDY_LOCAL_LICENSE_INFINITE_EXPIRATION to indicate that
 * the license will not expire.
 *********************************************************************************************/
typedef DRM_Prdy_xmr_expiration_t  DRM_Prdy_local_license_expiration_t;
typedef DRM_Prdy_xmr_dword_t       DRM_Prdy_local_license_expiration_after_first_play_t;

typedef struct DRM_Prdy_ID_t {
    uint8_t data[16];
} DRM_Prdy_ID_t;

typedef DRM_Prdy_ID_t Drm_Prdy_KID_t;

typedef struct DRM_Prdy_local_license_source_id_t {
    DRM_Prdy_bool_t             fValid;
    uint32_t                    dwSourceId;
    DRM_Prdy_bool_t             fRestrictedSourceId;
} DRM_Prdy_local_license_source_id_t;

typedef struct DRM_Prdy_local_license_explicit_output_protection_t {
    DRM_Prdy_guid_t             oProtectionId;
    uint16_t                    cbConfigData;
    uint8_t                     rgbConfigData[DRM_PRDY_MAX_LOCAL_LICENSE_EXPLICIT_OUTPUT_PROTECTION_CONFIG_DATA];
} DRM_Prdy_local_license_explicit_output_protection_t;

/**********************************************************************************************
 * struct DRM_Prdy_local_license_policy_descriptor_t
 *
 * fCannotPersist
 *    - Indicates whether or not the license is in memory. TRUE if it's an in-memory license.
 * oExpiration
 *    - Indicates the duration for which a license is valid.
 * oExpirationAfterFirstPlay
 *    - Indicates the expiration time of a license after it is first used.
 * fRealTimeExpiration
 *    - Indicates whether or not the license can expire in real time. The default is FALSE.
 *      If TRUE, then the license can expire while it's being used, that is to say it can
 *      expire while decrypting content.
 * oSourceId
 *    - A DRM_Prdy_local_license_source_id_t object describing the source of the content.
 * wCompressedDigitalVideo
 *    - Output control level to compressed digital video.
 * wUncompressedDigitalVideo
 *    - Output controllevel to uncompressed digital video.
 * wAnalogVideo
 *    - Output control level to analog video.
 * wCompressedDigitalAudio
 *    - Output control level for compressed digital audio.
 * wUncompressedDigitalAudio
 *    - Output control level for uncompressed digital audio.
 * cExplicitAnalogVideoOutputProtections
 *    - The number of items contained in the rgoExplicitAnalogVideoOutputProtections list.
 * rgoExplicitAnalogVideoOutputProtections
 *    - List of DRM_LOCAL_LICENSE_EXPLICIT_OUTPUT_PROTECTION video output protection systems.
 * cExplicitDigitalAudioOoutputProtections
 *    - The number of items contained in the rgoExplicitDigitalAudioOutputProtections list.
 * rgoExplicitDigitalAudioOutputProtections
 *    - List of DRM_LOCAL_LICENSE_EXPLICIT_OUTPUT_PROTECTION digital audio output protection systems.
 * cPlayEnablers
 *    - The number of items contained in the rgoPlayEnablers list.
 * rgoPlayEnablers
 *    - List of identifiers for play enablers.
 *
 * Note:
 *   For the detail setting of the fields, please refer to Microsoft PlayReady Compliance and
 *   Robustness rules:  https://www.microsoft.com/playready/licensing/compliance/
 *
 *************************************************************************************************/
typedef struct DRM_Prdy_local_license_policy_descriptor_t {
    uint16_t                                             wSecurityLevel;
    DRM_Prdy_bool_t                                      fCannotPersist;
    DRM_Prdy_local_license_expiration_t                  oExpiration;
    DRM_Prdy_local_license_expiration_after_first_play_t oExpirationAfterFirstPlay;
    DRM_Prdy_bool_t                                      fRealTimeExpiration;
    DRM_Prdy_local_license_source_id_t                   oSourceId;
    uint16_t                                             wCompressedDigitalVideo;
    uint16_t                                             wUncompressedDigitalVideo;
    uint16_t                                             wAnalogVideo;
    uint16_t                                             wCompressedDigitalAudio;
    uint16_t                                             wUncompressedDigitalAudio;
    uint16_t                                             cExplicitAnalogVideoOutputProtections;
    DRM_Prdy_local_license_explicit_output_protection_t  rgoExplicitAnalogVideoOutputProtections[DRM_PRDY_MAX_LOCAL_LICENSE_EXPLICIT_OUTPUT_PROTECTIONS];
    uint16_t                                             cExplicitDigitalAudioOoutputProtections;
    DRM_Prdy_local_license_explicit_output_protection_t  rgoExplicitDigitalAudioOutputProtections[DRM_PRDY_MAX_LOCAL_LICENSE_EXPLICIT_OUTPUT_PROTECTIONS];
    uint16_t                                             cPlayEnablers;
    DRM_Prdy_guid_t                                      rgoPlayEnablers[DRM_PRDY_MAX_LOCAL_LICENSE_PLAY_ENABLERS];
} DRM_Prdy_local_license_policy_descriptor_t;


typedef struct DRM_Prdy_Content_Set_Property_Obj_With_KID_Data_t {
    const uint8_t *pbKeyID;        /* For this structure, pbKeyID is a base64-encoded UTF16 string */
    uint32_t       cbKeyID;

    const uint8_t *pbHeaderData;
    uint32_t       cbHeaderData;

} DRM_Prdy_Content_Set_Property_Obj_With_KID_Data_t;

/***************************************************/
/**************** Prdy ND specific *****************/
/***************************************************/

typedef struct DRM_PRDY_ND_TRANSMITTER_CONTEXT   *DRM_Prdy_ND_Transmitter_Context_t;  /* opaque prnd transmitter Context */
typedef struct DRM_PRDY_ND_RECEIVER_CONTEXT      *DRM_Prdy_ND_Receiver_Context_t;     /* opaque prnd receiver Context */

#define DRM_PRDY_ND_BCERT_MAX_FEATURES                                 32
#define DRM_PRDY_ND_CERTIFICATE_DATA_DIGEST_SIZE                       32
#define DRM_PRDY_ND_CERTIFICATE_DATA_CLIENT_ID_SIZE                    16
#define DRM_PRDY_ND_CERTIFICATE_DATA_MAX_MANUFACTURER_STRING_LENGTH    129 /* Null-terminated */
#define DRM_PRDY_ND_NO_FLAGS                                           0

static const DRM_Prdy_guid_t DRM_PRDY_ND_ACTION_PLAY =
   {0x4C3FC9B3, 0x31C2, 0x4FD4, {0x82, 0x4A, 0x04, 0xD4, 0x23, 0x41, 0xA9, 0xD3 }};

static const DRM_Prdy_guid_t DRM_PRDY_ND_PLAYENABLER_UNKNOWN_OUTPUT =
   {0x786627D8, 0xC2A6, 0x44BE, {0x8F, 0x88, 0x08, 0xAE, 0x25, 0x5B, 0x01, 0xA7} };

static const DRM_Prdy_guid_t DRM_PRDY_ND_PLAYENABLER_CONSTRAINED_RESOLUTION_UNKNOWN_OUTPUT =
   {0xB621D91F, 0xEDCC, 0x4035, {0x8D, 0x4B, 0xDC, 0x71, 0x76, 0x0D, 0x43, 0xE9} };

typedef enum DRM_Prdy_ND_Message_Type_e {
    eDRM_PRDY_ND_MESSAGE_TYPE_INVALID                       =  0,
    eDRM_PRDY_ND_MESSAGE_TYPE_REGISTRATION_REQUEST          =  1,
    eDRM_PRDY_ND_MESSAGE_TYPE_REGISTRATION_RESPONSE         =  2,
    eDRM_PRDY_ND_MESSAGE_TYPE_REGISTRATION_ERROR            =  3,
    eDRM_PRDY_ND_MESSAGE_TYPE_PROXIMITY_DETECTION_START     =  4,
    eDRM_PRDY_ND_MESSAGE_TYPE_PROXIMITY_DETECTION_CHALLENGE =  5,
    eDRM_PRDY_ND_MESSAGE_TYPE_PROXIMITY_DETECTION_RESPONSE  =  6,
    eDRM_PRDY_ND_MESSAGE_TYPE_PROXIMITY_DETECTION_RESULT    =  7,
    eDRM_PRDY_ND_MESSAGE_TYPE_LICENSE_REQUEST               =  8,
    eDRM_PRDY_ND_MESSAGE_TYPE_LICENSE_TRANSMIT              =  9,
    eDRM_PRDY_ND_MESSAGE_TYPE_LICENSE_ERROR                 = 10
} DRM_Prdy_ND_Message_Type_e;


typedef struct DRM_Prdy_ND_Certificate_Data_t {
    uint16_t  dwType;  /* Should be: DRM_BCERT_CERTTYPE_PC (0x1) or DRM_BCERT_CERTTYPE_DEVICE (0x2).  Refer to drmbcert.h. */
    uint16_t  dwPlatformIdentifier;   /* One of many DRM_BCERT_SECURITY_VERSION_PLATFORM_* values.    Refer to drmbcert.h. */
    uint16_t  dwSecurityLevel;        /* Typically will be 150 or 2000. */
    uint16_t  cSupportedFeatures;
    uint16_t  rgdwSupportedFeatureSet[ DRM_PRDY_ND_BCERT_MAX_FEATURES ];
    uint8_t   rgbClientID[ DRM_PRDY_ND_CERTIFICATE_DATA_CLIENT_ID_SIZE ];
    uint8_t   rgbModelDigest[ DRM_PRDY_ND_CERTIFICATE_DATA_DIGEST_SIZE ];
    uint8_t   szModelManufacturerName[ DRM_PRDY_ND_CERTIFICATE_DATA_MAX_MANUFACTURER_STRING_LENGTH ];
    uint8_t   szModelName[ DRM_PRDY_ND_CERTIFICATE_DATA_MAX_MANUFACTURER_STRING_LENGTH ];
    uint8_t   szModelNumber[ DRM_PRDY_ND_CERTIFICATE_DATA_MAX_MANUFACTURER_STRING_LENGTH ];
} DRM_Prdy_ND_Certificate_Data_t;

typedef DRM_Prdy_Error_e ( * DRM_Prdy_ND_Data_Callback )(
                      void                            *pvDataCallbackContext,
                      DRM_Prdy_ND_Certificate_Data_t  *pCertificateData,
                      const DRM_Prdy_ID_t             *pCustomDataTypeID,
                      const uint8_t                   *pbCustomData,
                      uint16_t                         cbCustomData );

typedef enum DRM_Prdy_ND_Proximity_Detection_Type_e {
    eDRM_PRDY_ND_PROXIMITY_DETECTION_TYPE_UDP                 = 0x00000001,
    eDRM_PRDY_ND_PROXIMITY_DETECTION_TYPE_TCP                 = 0x00000002,
    eDRM_PRDY_ND_PROXIMITY_DETECTION_TYPE_TRANSPORT_AGNOSTIC  = 0x00000004
    /* NOTE: This is used as a Bitmask, so new values must increase as multiples of two! */
} DRM_Prdy_ND_Proximity_Detection_Type_e;

typedef enum DRM_Prdy_ND_Content_Identifier_Type_e {
    eDRM_PRDY_ND_CONTENT_IDENTIFIER_TYPE_KID                  = 0x1,
    eDRM_PRDY_ND_CONTENT_IDENTIFIER_TYPE_CUSTOM               = 0x2
} DRM_Prdy_ND_Content_Identifier_Type_e;

typedef DRM_Prdy_Error_e ( * Drm_Prdy_ND_Content_Identifier_Callback )(
                       void                                    *pvContentIdentifierCallbackContext,
                       DRM_Prdy_ND_Content_Identifier_Type_e    eContentIdentifierType,
                       const uint8_t                           *pbContentIdentifier,
                       uint16_t                                 cbContentIdentifier,
                       Drm_Prdy_KID_t                          *pkidContent );

/***************************************************/
/**************** Secure Stop specific *************/
/***************************************************/
#define		DRM_PRDY_MAX_NUM_SECURE_STOPS		8		/*  Must match the value of TEE_MAX_NUM_SECURE_STOPS in the playready SDK */
#define		DRM_PRDY_SESSION_ID_LEN				16		/*  Must match the value of TEE_SESSION_ID_LEN in the playready SDK */

#ifdef __cplusplus
}
#endif

#endif /*DRM_PRDY_TYPES_H__*/

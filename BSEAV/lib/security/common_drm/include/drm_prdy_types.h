/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#if (NEXUS_SECURITY_API_VERSION==1)
    NEXUS_SecurityOperation opType;              /* Operation to perfrom */
    NEXUS_SecurityAlgorithm algType;             /* Crypto algorithm */
    NEXUS_SecurityAlgorithmVariant algVariant;   /* Cipher chain mode for selected cipher */
    NEXUS_SecurityKeyType keySlotType;           /* Key destination entry type */
    NEXUS_SecurityTerminationMode termMode;      /* Termination Type for residual block to be ciphered */
#else
    NEXUS_CryptographicOperation opType;
    NEXUS_CryptographicAlgorithm algType;        /* Crypto algorithm */
    NEXUS_CryptographicAlgorithmMode algVariant; /* Cipher chain mode for selected cipher */
    NEXUS_KeySlotPolarity keySlotType;           /* Key destination entry type */
    NEXUS_KeySlotTerminationMode termMode;       /* Termination Type for residual block to be ciphered */
    NEXUS_KeySlotBlockEntry keySlotEntryType;    /* different for decrypt and encrypt operations */
#endif

    bool enableExtKey;                           /* Flag used to enable external key loading during dma transfer on the key slot.
                                                    true = key will prepend data in the dma descriptors. */
    bool enableExtIv;                            /* Flag used to enable external IV loading during dma transfer on the key slot.
                                                    true = iv will prepend data in the dma descriptors. */
#if (NEXUS_SECURITY_API_VERSION==1)
    NEXUS_SecurityAesCounterSize aesCounterSize; /* This member is required for AES counter mode  */
    NEXUS_SecurityCounterMode    aesCounterMode; /* for Zeus 3.0 and later */
#else
	unsigned aesCounterSize;                                /* For algorithm modes predicated on a counter, this parameter spcifies
                                                        the size of the counter in bits. Supported values are 32, 64, 96 and 128 bits.*/
	NEXUS_CounterMode aesCounterMode;
#endif

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
    DRM_Prdy_no_policy                                                    =    11, /* No more policies queued */
    DRM_Prdy_more_data                                                    =	12	,
    DRM_Prdy_cprmexp_noerror                                              =	13	,
    DRM_Prdy_test_skip_file                                               =	14	,
    DRM_Prdy_test_converted_file                                          =	15	,
    DRM_Prdy_outofmemory                                                  =	16	,
    DRM_Prdy_notimpl                                                      =	17	,
    DRM_Prdy_pointer                                                      =	18	,
    DRM_Prdy_filenotfound                                                 =	19	,
    DRM_Prdy_fileopen                                                     =	20	,
    DRM_Prdy_verification_failure                                         =	21	,
    DRM_Prdy_rsa_signature_error                                          =	22	,
    DRM_Prdy_bad_rsa_exponent                                             =	23	,
    DRM_Prdy_p256_conversion_failure                                      =	24	,
    DRM_Prdy_p256_pkcrypto_failure                                        =	25	,
    DRM_Prdy_p256_plaintext_mapping_failure                               =	26	,
    DRM_Prdy_p256_invalid_signature                                       =	27	,
    DRM_Prdy_p256_ecdsa_verification_error                                =	28	,
    DRM_Prdy_p256_ecdsa_signing_error                                     =	29	,
    DRM_Prdy_p256_hmac_keygen_failure                                     =	30	,
    DRM_Prdy_ch_version_missing                                           =	32	,
    DRM_Prdy_ch_kid_missing                                               =	33	,
    DRM_Prdy_ch_lainfo_missing                                            =	34	,
    DRM_Prdy_ch_checksum_missing                                          =	35	,
    DRM_Prdy_ch_attr_missing                                              =	36	,
    DRM_Prdy_ch_invalid_header                                            =	37	,
    DRM_Prdy_ch_invalid_checksum                                          =	38	,
    DRM_Prdy_ch_unable_to_verify                                          =	39	,
    DRM_Prdy_ch_unsupported_version                                       =	40	,
    DRM_Prdy_ch_bad_key                                                   =	41	,
    DRM_Prdy_ch_incompatible_header_type                                  =	42	,
    DRM_Prdy_header_already_set                                           =	43	,
    DRM_Prdy_lic_param_not_optional                                       =	45	,
    DRM_Prdy_lic_invalid_license                                          =	46	,
    DRM_Prdy_lic_unsupported_value                                        =	47	,
    DRM_Prdy_cprmexp_no_operands_in_expression                            =	49	,
    DRM_Prdy_cprmexp_invalid_token                                        =	50	,
    DRM_Prdy_cprmexp_invalid_constant                                     =	51	,
    DRM_Prdy_cprmexp_invalid_variable                                     =	52	,
    DRM_Prdy_cprmexp_invalid_function                                     =	53	,
    DRM_Prdy_cprmexp_invalid_argument                                     =	54	,
    DRM_Prdy_cprmexp_invalid_context                                      =	55	,
    DRM_Prdy_cprmexp_missing_operand                                      =	56	,
    DRM_Prdy_cprmexp_overflow                                             =	57	,
    DRM_Prdy_cprmexp_underflow                                            =	58	,
    DRM_Prdy_cprmexp_incorrect_num_args                                   =	59	,
    DRM_Prdy_cprmexp_variable_expected                                    =	60	,
    DRM_Prdy_cprmexp_retrieval_failure                                    =	61	,
    DRM_Prdy_cprmexp_update_failure                                       =	62	,
    DRM_Prdy_cprmexp_string_unterminated                                  =	63	,
    DRM_Prdy_cprmexp_update_unsupported                                   =	64	,
    DRM_Prdy_cprmexp_isolated_operand_or_operator                         =	65	,
    DRM_Prdy_cprmexp_unmatched                                            =	66	,
    DRM_Prdy_cprmexp_wrong_type_operand                                   =	67	,
    DRM_Prdy_cprmexp_too_many_operands                                    =	68	,
    DRM_Prdy_cprmexp_unknown_parse_error                                  =	69	,
    DRM_Prdy_cprmexp_unsupported_function                                 =	70	,
    DRM_Prdy_cprmexp_clock_required                                       =	71	,
    DRM_Prdy_lic_key_decode_failure                                       =	73	,
    DRM_Prdy_lic_signature_failure                                        =	74	,
    DRM_Prdy_key_mismatch                                                 =	75	,
    DRM_Prdy_invalid_signature                                            =	76	,
    DRM_Prdy_sync_entry_not_found                                         =	77	,
    DRM_Prdy_cipher_not_initialized                                       =	78	,
    DRM_Prdy_decrypt_not_initialized                                      =	79	,
    DRM_Prdy_securestore_lock_not_obtained                                =	80	,
    DRM_Prdy_pkcrypto_failure                                             =	81	,
    DRM_Prdy_invalid_dst_slot_size                                        =	82	,
    DRM_Prdy_unsupported_version                                          =	83	,
    DRM_Prdy_expired_cert                                                 =	84	,
    DRM_Prdy_drmutil_invalid_cert                                         =	85	,
    DRM_Prdy_device_not_registered                                        =	86	,
    DRM_Prdy_too_many_inclusion_guids                                     =	87	,
    DRM_Prdy_revocation_guid_not_recognized                               =	88	,
    DRM_Prdy_lic_chain_too_deep                                           =	89	,
    DRM_Prdy_device_security_level_too_low                                =	90	,
    DRM_Prdy_dst_block_cache_corrupt                                      =	91	,
    DRM_Prdy_dst_block_cache_miss                                         =	92	,
    DRM_Prdy_invalid_meterresponse_signature                              =	93	,
    DRM_Prdy_invalid_license_revocation_list_signature                    =	94	,
    DRM_Prdy_invalid_metercert_signature                                  =	95	,
    DRM_Prdy_meterstore_data_not_found                                    =	96	,
    DRM_Prdy_invalid_revocation_list                                      =	97	,
    DRM_Prdy_envelope_corrupt                                             =	98	,
    DRM_Prdy_envelope_file_not_compatible                                 =	99	,
    DRM_Prdy_extended_restriction_not_understood                          =	100	,
    DRM_Prdy_invalid_slk                                                  =	101	,
    DRM_Prdy_devcert_model_mismatch                                       =	102	,
    DRM_Prdy_outdated_revocation_list                                     =	103	,
    DRM_Prdy_device_not_initialized                                       =	104	,
    DRM_Prdy_drm_not_initialized                                          =	105	,
    DRM_Prdy_invalid_right                                                =	106	,
    DRM_Prdy_invalid_license                                              =	107	,
    DRM_Prdy_condition_not_supported                                      =	108	,
    DRM_Prdy_rights_not_available                                         =	109	,
    DRM_Prdy_license_mismatch                                             =	110	,
    DRM_Prdy_wrong_token_type                                             =	111	,
    DRM_Prdy_license_not_bound                                            =	112	,
    DRM_Prdy_hash_mismatch                                                =	113	,
    DRM_Prdy_licensestore_not_found                                       =	114	,
    DRM_Prdy_license_version_not_supported                                =	115	,
    DRM_Prdy_unsupported_algorithm                                        =	116	,
    DRM_Prdy_invalid_license_store                                        =	117	,
    DRM_Prdy_file_read_error                                              =	118	,
    DRM_Prdy_file_write_error                                             =	119	,
    DRM_Prdy_dst_store_full                                               =	120	,
    DRM_Prdy_no_xml_open_tag                                              =	121	,
    DRM_Prdy_no_xml_close_tag                                             =	122	,
    DRM_Prdy_invalid_xml_tag                                              =	123	,
    DRM_Prdy_no_xml_cdata                                                 =	124	,
    DRM_Prdy_dst_namespace_not_found                                      =	125	,
    DRM_Prdy_dst_slot_not_found                                           =	126	,
    DRM_Prdy_dst_slot_exists                                              =	127	,
    DRM_Prdy_dst_corrupted                                                =	128	,
    DRM_Prdy_dst_seek_error                                               =	129	,
    DRM_Prdy_invalid_securestore_password                                 =	130	,
    DRM_Prdy_securestore_corrupt                                          =	131	,
    DRM_Prdy_securestore_full                                             =	132	,
    DRM_Prdy_duplicated_header_attribute                                  =	133	,
    DRM_Prdy_no_kid_in_header                                             =	134	,
    DRM_Prdy_no_lainfo_in_header                                          =	135	,
    DRM_Prdy_no_checksum_in_header                                        =	136	,
    DRM_Prdy_dst_block_mismatch                                           =	137	,
    DRM_Prdy_license_too_long                                             =	138	,
    DRM_Prdy_dst_exists                                                   =	139	,
    DRM_Prdy_invalid_device_certificate                                   =	140	,
    DRM_Prdy_dst_lock_failed                                              =	141	,
    DRM_Prdy_file_seek_error                                              =	142	,
    DRM_Prdy_dst_not_locked_exclusive                                     =	143	,
    DRM_Prdy_dst_exclusive_lock_only                                      =	144	,
    DRM_Prdy_v1_not_supported                                             =	145	,
    DRM_Prdy_need_devcert_indiv                                           =	146	,
    DRM_Prdy_machine_id_mismatch                                          =	147	,
    DRM_Prdy_clk_invalid_response                                         =	148	,
    DRM_Prdy_clk_invalid_date                                             =	149	,
    DRM_Prdy_invalid_devcert_template                                     =	150	,
    DRM_Prdy_devcert_exceeds_size_limit                                   =	151	,
    DRM_Prdy_devcert_read_error                                           =	152	,
    DRM_Prdy_privkey_read_error                                           =	153	,
    DRM_Prdy_devcert_template_read_error                                  =	154	,
    DRM_Prdy_clk_not_supported                                            =	155	,
    DRM_Prdy_metering_not_supported                                       =	156	,
    DRM_Prdy_clk_reset_state_read_error                                   =	157	,
    DRM_Prdy_clk_reset_state_write_error                                  =	158	,
    DRM_Prdy_xmlnotfound                                                  =	159	,
    DRM_Prdy_metering_wrong_tid                                           =	160	,
    DRM_Prdy_metering_invalid_command                                     =	161	,
    DRM_Prdy_metering_store_corrupt                                       =	162	,
    DRM_Prdy_certificate_revoked                                          =	163	,
    DRM_Prdy_crypto_failed                                                =	164	,
    DRM_Prdy_stack_corrupt                                                =	165	,
    DRM_Prdy_unknown_binding_key                                          =	166	,
    DRM_Prdy_v1_license_chain_not_supported                               =	167	,
    DRM_Prdy_policy_metering_disabled                                     =	168	,
    DRM_Prdy_clk_not_set                                                  =	169	,
    DRM_Prdy_no_clk_supported                                             =	170	,
    DRM_Prdy_no_url                                                       =	171	,
    DRM_Prdy_unknown_device_property                                      =	172	,
    DRM_Prdy_metering_mid_mismatch                                        =	173	,
    DRM_Prdy_metering_response_decrypt_failed                             =	174	,
    DRM_Prdy_riv_too_small                                                =	175	,
    DRM_Prdy_stack_already_initialized                                    =	176	,
    DRM_Prdy_devcert_revoked                                              =	177	,
    DRM_Prdy_oem_rsa_decryption_error                                     =	178	,
    DRM_Prdy_invalid_devstore_attribute                                   =	179	,
    DRM_Prdy_invalid_devstore_entry                                       =	180	,
    DRM_Prdy_oem_rsa_encryption_error                                     =	181	,
    DRM_Prdy_dst_namespace_exists                                         =	182	,
    DRM_Prdy_perf_scoping_error                                           =	183	,
    DRM_Prdy_oem_rsa_invalid_private_key                                  =	184	,
    DRM_Prdy_no_opl_callback                                              =	185	,
    DRM_Prdy_invalid_playready_object                                     =	186	,
    DRM_Prdy_duplicate_license                                            =	187	,
    DRM_Prdy_record_not_found                                             =	188	,
    DRM_Prdy_buffer_bounds_exceeded                                       =	189	,
    DRM_Prdy_invalid_base64                                               =	190	,
    DRM_Prdy_protocol_version_not_supported                               =	191	,
    DRM_Prdy_invalid_license_response_signature                           =	192	,
    DRM_Prdy_invalid_license_response_id                                  =	193	,
    DRM_Prdy_license_response_signature_missing                           =	194	,
    DRM_Prdy_invalid_domain_join_response_signature                       =	195	,
    DRM_Prdy_domain_join_response_signature_missing                       =	196	,
    DRM_Prdy_activation_required                                          =	197	,
    DRM_Prdy_activation_internal_error                                    =	198	,
    DRM_Prdy_activation_group_cert_revoked_error                          =	199	,
    DRM_Prdy_activation_new_client_lib_required_error                     =	200	,
    DRM_Prdy_activation_bad_request                                       =	201	,
    DRM_Prdy_fileio_error                                                 =	202	,
    DRM_Prdy_disk_space_error                                             =	203	,
    DRM_Prdy_uplink_license_not_found                                     =	204	,
    DRM_Prdy_activation_client_already_current                            =	205	,
    DRM_Prdy_license_realtime_expired                                     =	206	,
    DRM_Prdy_liceval_license_not_supplied                                 =	207	,
    DRM_Prdy_liceval_kid_mismatch                                         =	208	,
    DRM_Prdy_liceval_license_revoked                                      =	209	,
    DRM_Prdy_liceval_update_failure                                       =	210	,
    DRM_Prdy_liceval_required_revocation_list_not_available               =	211	,
    DRM_Prdy_liceval_invalid_prnd_license                                 =	212	,
    DRM_Prdy_xmr_object_already_exists                                    =	213	,
    DRM_Prdy_xmr_object_not_found                                         =	214	,
    DRM_Prdy_xmr_required_object_missing                                  =	215	,
    DRM_Prdy_xmr_invalid_unknown_object                                   =	216	,
    DRM_Prdy_xmr_license_bindable                                         =	217	,
    DRM_Prdy_xmr_license_not_bindable                                     =	218	,
    DRM_Prdy_xmr_unsupported_xmr_version                                  =	219	,
    DRM_Prdy_invalid_devcert_attribute                                    =	220	,
    DRM_Prdy_test_pkcrypto_failure                                        =	221	,
    DRM_Prdy_test_pksign_verify_error                                     =	222	,
    DRM_Prdy_test_encrypt_error                                           =	223	,
    DRM_Prdy_test_rc4key_failed                                           =	224	,
    DRM_Prdy_test_decrypt_error                                           =	225	,
    DRM_Prdy_test_deskey_failed                                           =	226	,
    DRM_Prdy_test_cbc_inversemac_failure                                  =	227	,
    DRM_Prdy_test_hmac_failure                                            =	228	,
    DRM_Prdy_test_invalidarg                                              =	229	,
    DRM_Prdy_test_device_private_key_incorrectly_stored                   =	230	,
    DRM_Prdy_test_drmmanager_context_null                                 =	231	,
    DRM_Prdy_test_unexpected_revinfo_result                               =	232	,
    DRM_Prdy_test_riv_mismatch                                            =	233	,
    DRM_Prdy_test_url_error                                               =	234	,
    DRM_Prdy_test_mid_mismatch                                            =	235	,
    DRM_Prdy_test_meter_certificate_mismatch                              =	236	,
    DRM_Prdy_test_license_state_mismatch                                  =	237	,
    DRM_Prdy_test_source_id_mismatch                                      =	238	,
    DRM_Prdy_test_unexpected_license_count                                =	239	,
    DRM_Prdy_test_unexpected_device_property                              =	240	,
    DRM_Prdy_test_drmmanager_misaligned_bytes                             =	241	,
    DRM_Prdy_test_license_response_error                                  =	242	,
    DRM_Prdy_test_opl_mismatch                                            =	243	,
    DRM_Prdy_test_invalid_opl_callback                                    =	244	,
    DRM_Prdy_test_incomplete                                              =	245	,
    DRM_Prdy_test_unexpected_output                                       =	246	,
    DRM_Prdy_test_dla_no_content_header                                   =	247	,
    DRM_Prdy_test_dla_content_header_found                                =	248	,
    DRM_Prdy_test_sync_lsd_incorrect                                      =	249	,
    DRM_Prdy_test_too_slow                                                =	250	,
    DRM_Prdy_test_licensestore_not_open                                   =	251	,
    DRM_Prdy_test_device_not_inited                                       =	252	,
    DRM_Prdy_test_variable_not_set                                        =	253	,
    DRM_Prdy_test_nomore                                                  =	254	,
    DRM_Prdy_test_file_load_error                                         =	255	,
    DRM_Prdy_test_license_acq_failed                                      =	256	,
    DRM_Prdy_test_unsupported_file_format                                 =	257	,
    DRM_Prdy_test_parsing_error                                           =	258	,
    DRM_Prdy_test_notimpl                                                 =	259	,
    DRM_Prdy_test_variable_notfound                                       =	260	,
    DRM_Prdy_test_variable_listfull                                       =	261	,
    DRM_Prdy_test_unexpected_content_property                             =	262	,
    DRM_Prdy_test_pro_header_not_set                                      =	263	,
    DRM_Prdy_test_non_pro_header_type                                     =	264	,
    DRM_Prdy_test_invalid_device_wrapper                                  =	265	,
    DRM_Prdy_test_invalid_wmdm_wrapper                                    =	266	,
    DRM_Prdy_test_invalid_wpd_wrapper                                     =	267	,
    DRM_Prdy_test_invalid_file                                            =	268	,
    DRM_Prdy_test_property_not_found                                      =	269	,
    DRM_Prdy_test_metering_data_incorrect                                 =	270	,
    DRM_Prdy_test_file_already_open                                       =	271	,
    DRM_Prdy_test_file_not_open                                           =	272	,
    DRM_Prdy_test_pict_column_too_wide                                    =	273	,
    DRM_Prdy_test_pict_column_mismatch                                    =	274	,
    DRM_Prdy_test_tux_test_skipped                                        =	275	,
    DRM_Prdy_test_keyfile_verification_failure                            =	276	,
    DRM_Prdy_test_data_verification_failure                               =	277	,
    DRM_Prdy_test_net_fail                                                =	278	,
    DRM_Prdy_test_cleanup_fail                                            =	279	,
    DRM_Prdy_logicerr                                                     =	280	,
    DRM_Prdy_invalid_rev_info                                             =	281	,
    DRM_Prdy_synclist_not_supported                                       =	282	,
    DRM_Prdy_revocation_buffer_too_small                                  =	283	,
    DRM_Prdy_device_already_registered                                    =	284	,
    DRM_Prdy_dst_not_compatible                                           =	285	,
    DRM_Prdy_rsa_decryption_error                                         =	286	,
    DRM_Prdy_oem_rsa_message_too_big                                      =	287	,
    DRM_Prdy_metercert_not_found                                          =	288	,
    DRM_Prdy_modular_arithmetic_failure                                   =	289	,
    DRM_Prdy_revocation_invalid_package                                   =	290	,
    DRM_Prdy_hwid_error                                                   =	291	,
    DRM_Prdy_domain_invalid_guid                                          =	292	,
    DRM_Prdy_domain_invalid_custom_data_type                              =	293	,
    DRM_Prdy_domain_store_add_data                                        =	294	,
    DRM_Prdy_domain_store_get_data                                        =	295	,
    DRM_Prdy_domain_store_delete_data                                     =	296	,
    DRM_Prdy_domain_store_open_store                                      =	297	,
    DRM_Prdy_domain_store_close_store                                     =	298	,
    DRM_Prdy_domain_bind_license                                          =	299	,
    DRM_Prdy_domain_invalid_custom_data                                   =	300	,
    DRM_Prdy_domain_not_found                                             =	301	,
    DRM_Prdy_domain_invalid_domkeyxmr_data                                =	302	,
    DRM_Prdy_domain_store_invalid_key_record                              =	303	,
    DRM_Prdy_device_domain_join_required                                  =	304	,
    DRM_Prdy_server_internal_error                                        =	305	,
    DRM_Prdy_server_invalid_message                                       =	306	,
    DRM_Prdy_server_device_limit_reached                                  =	307	,
    DRM_Prdy_server_indiv_required                                        =	308	,
    DRM_Prdy_server_service_specific                                      =	309	,
    DRM_Prdy_server_domain_required                                       =	310	,
    DRM_Prdy_server_renew_domain                                          =	311	,
    DRM_Prdy_server_unknown_meteringid                                    =	312	,
    DRM_Prdy_server_computer_limit_reached                                =	313	,
    DRM_Prdy_server_protocol_fallback                                     =	314	,
    DRM_Prdy_server_not_a_member                                          =	315	,
    DRM_Prdy_server_protocol_version_mismatch                             =	316	,
    DRM_Prdy_server_unknown_accountid                                     =	317	,
    DRM_Prdy_server_protocol_redirect                                     =	318	,
    DRM_Prdy_server_unknown_transactionid                                 =	319	,
    DRM_Prdy_server_invalid_licenseid                                     =	320	,
    DRM_Prdy_server_maximum_licenseid_exceeded                            =	321	,
    DRM_Prdy_licacq_too_many_licenses                                     =	322	,
    DRM_Prdy_licacq_ack_transaction_id_too_big                            =	323	,
    DRM_Prdy_licacq_ack_message_not_created                               =	324	,
    DRM_Prdy_initiators_unknown_type                                      =	325	,
    DRM_Prdy_initiators_invalid_serviceid                                 =	326	,
    DRM_Prdy_initiators_invalid_accountid                                 =	327	,
    DRM_Prdy_initiators_invalid_mid                                       =	328	,
    DRM_Prdy_initiators_missing_dc_url                                    =	329	,
    DRM_Prdy_initiators_missing_content_header                            =	330	,
    DRM_Prdy_initiators_missing_laurl_in_content_header                   =	331	,
    DRM_Prdy_initiators_missing_metercert_url                             =	332	,
    DRM_Prdy_bcert_invalid_signature_type                                 =	333	,
    DRM_Prdy_bcert_chain_too_deep                                         =	334	,
    DRM_Prdy_bcert_invalid_cert_type                                      =	335	,
    DRM_Prdy_bcert_invalid_feature                                        =	336	,
    DRM_Prdy_bcert_invalid_key_usage                                      =	337	,
    DRM_Prdy_bcert_invalid_security_version                               =	338	,
    DRM_Prdy_bcert_invalid_key_type                                       =	339	,
    DRM_Prdy_bcert_invalid_key_length                                     =	340	,
    DRM_Prdy_bcert_invalid_max_license_size                               =	341	,
    DRM_Prdy_bcert_invalid_max_header_size                                =	342	,
    DRM_Prdy_bcert_invalid_max_license_chain_depth                        =	343	,
    DRM_Prdy_bcert_invalid_security_level                                 =	344	,
    DRM_Prdy_bcert_private_key_not_specified                              =	345	,
    DRM_Prdy_bcert_issuer_key_not_specified                               =	346	,
    DRM_Prdy_bcert_account_id_not_specified                               =	347	,
    DRM_Prdy_bcert_service_id_not_specified                               =	348	,
    DRM_Prdy_bcert_domain_url_not_specified                               =	349	,
    DRM_Prdy_bcert_domain_url_too_long                                    =	350	,
    DRM_Prdy_bcert_hardware_id_too_long                                   =	351	,
    DRM_Prdy_bcert_cert_id_not_specified                                  =	352	,
    DRM_Prdy_bcert_public_key_not_specified                               =	353	,
    DRM_Prdy_bcert_key_usages_not_specified                               =	354	,
    DRM_Prdy_bcert_string_not_null_terminated                             =	355	,
    DRM_Prdy_bcert_objectheader_len_too_big                               =	356	,
    DRM_Prdy_bcert_invalid_issuerkey_length                               =	357	,
    DRM_Prdy_bcert_basicinfo_cert_expired                                 =	358	,
    DRM_Prdy_bcert_unexpected_object_header                               =	359	,
    DRM_Prdy_bcert_issuerkey_keyinfo_mismatch                             =	360	,
    DRM_Prdy_bcert_invalid_max_key_usages                                 =	361	,
    DRM_Prdy_bcert_invalid_max_features                                   =	362	,
    DRM_Prdy_bcert_invalid_chain_header_tag                               =	363	,
    DRM_Prdy_bcert_invalid_chain_version                                  =	364	,
    DRM_Prdy_bcert_invalid_chain_length                                   =	365	,
    DRM_Prdy_bcert_invalid_cert_header_tag                                =	366	,
    DRM_Prdy_bcert_invalid_cert_version                                   =	367	,
    DRM_Prdy_bcert_invalid_cert_length                                    =	368	,
    DRM_Prdy_bcert_invalid_signedcert_length                              =	369	,
    DRM_Prdy_bcert_invalid_platform_identifier                            =	370	,
    DRM_Prdy_bcert_invalid_number_extdatarecords                          =	371	,
    DRM_Prdy_bcert_invalid_extdatarecord                                  =	372	,
    DRM_Prdy_bcert_extdata_length_must_present                            =	373	,
    DRM_Prdy_bcert_extdata_privkey_must_present                           =	374	,
    DRM_Prdy_bcert_invalid_extdata_length                                 =	375	,
    DRM_Prdy_bcert_extdata_is_not_provided                                =	376	,
    DRM_Prdy_bcert_hwidinfo_is_missing                                    =	377	,
    DRM_Prdy_bcert_extdataflag_cert_type_mismatch                         =	378	,
    DRM_Prdy_bcert_metering_id_not_specified                              =	379	,
    DRM_Prdy_bcert_metering_url_not_specified                             =	380	,
    DRM_Prdy_bcert_metering_url_too_long                                  =	381	,
    DRM_Prdy_bcert_verification_errors                                    =	382	,
    DRM_Prdy_bcert_required_keyusage_missing                              =	383	,
    DRM_Prdy_bcert_no_pubkey_with_requested_keyusage                      =	384	,
    DRM_Prdy_bcert_manufacturer_string_too_long                           =	385	,
    DRM_Prdy_bcert_too_many_public_keys                                   =	386	,
    DRM_Prdy_bcert_objectheader_len_too_small                             =	387	,
    DRM_Prdy_bcert_invalid_warning_days                                   =	388	,
    DRM_Prdy_xmlsig_ecdsa_verify_failure                                  =	389	,
    DRM_Prdy_xmlsig_sha_verify_failure                                    =	390	,
    DRM_Prdy_xmlsig_format                                                =	391	,
    DRM_Prdy_xmlsig_public_key_id                                         =	392	,
    DRM_Prdy_xmlsig_invalid_key_format                                    =	393	,
    DRM_Prdy_xmlsig_sha_hash_size                                         =	394	,
    DRM_Prdy_xmlsig_ecdsa_signature_size                                  =	395	,
    DRM_Prdy_utf_unexpected_end                                           =	396	,
    DRM_Prdy_utf_invalid_code                                             =	397	,
    DRM_Prdy_soapxml_invalid_status_code                                  =	398	,
    DRM_Prdy_soapxml_xml_format                                           =	399	,
    DRM_Prdy_soapxml_wrong_message_type                                   =	400	,
    DRM_Prdy_soapxml_signature_missing                                    =	401	,
    DRM_Prdy_soapxml_protocol_not_supported                               =	402	,
    DRM_Prdy_soapxml_data_not_found                                       =	403	,
    DRM_Prdy_crypto_public_key_not_match                                  =	404	,
    DRM_Prdy_unable_to_resolve_location_tree                              =	405	,
    DRM_Prdy_nd_must_revalidate                                           =	406	,
    DRM_Prdy_nd_invalid_message                                           =	407	,
    DRM_Prdy_nd_invalid_message_type                                      =	408	,
    DRM_Prdy_nd_invalid_message_version                                   =	409	,
    DRM_Prdy_nd_invalid_session                                           =	410	,
    DRM_Prdy_nd_media_session_limit_reached                               =	411	,
    DRM_Prdy_nd_unable_to_verify_proximity                                =	412	,
    DRM_Prdy_nd_invalid_proximity_response                                =	413	,
    DRM_Prdy_nd_device_limit_reached                                      =	414	,
    DRM_Prdy_nd_bad_request                                               =	415	,
    DRM_Prdy_nd_failed_seek                                               =	416	,
    DRM_Prdy_nd_invalid_context                                           =	417	,
    DRM_Prdy_asf_bad_asf_header                                           =	418	,
    DRM_Prdy_asf_bad_packet_header                                        =	419	,
    DRM_Prdy_asf_bad_payload_header                                       =	420	,
    DRM_Prdy_asf_bad_data_header                                          =	421	,
    DRM_Prdy_asf_invalid_operation                                        =	422	,
    DRM_Prdy_asf_extended_stream_properties_obj_not_found                 =	423	,
    DRM_Prdy_asf_invalid_data                                             =	424	,
    DRM_Prdy_asf_too_many_payloads                                        =	425	,
    DRM_Prdy_asf_bandwidth_overrun                                        =	426	,
    DRM_Prdy_asf_invalid_stream_number                                    =	427	,
    DRM_Prdy_asf_late_sample                                              =	428	,
    DRM_Prdy_asf_not_accepting                                            =	429	,
    DRM_Prdy_asf_unexpected                                               =	430	,
    DRM_Prdy_nonce_store_token_not_found                                  =	431	,
    DRM_Prdy_nonce_store_open_store                                       =	432	,
    DRM_Prdy_nonce_store_close_store                                      =	433	,
    DRM_Prdy_nonce_store_add_license                                      =	434	,
    DRM_Prdy_policystate_not_found                                        =	435	,
    DRM_Prdy_policystate_corrupted                                        =	436	,
    DRM_Prdy_move_denied                                                  =	437	,
    DRM_Prdy_invalid_move_response                                        =	438	,
    DRM_Prdy_move_nonce_mismatch                                          =	439	,
    DRM_Prdy_move_store_open_store                                        =	440	,
    DRM_Prdy_move_store_close_store                                       =	441	,
    DRM_Prdy_move_store_add_data                                          =	442	,
    DRM_Prdy_move_store_get_data                                          =	443	,
    DRM_Prdy_move_format_invalid                                          =	444	,
    DRM_Prdy_move_signature_invalid                                       =	445	,
    DRM_Prdy_copy_denied                                                  =	446	,
    DRM_Prdy_xb_object_notfound                                           =	447	,
    DRM_Prdy_xb_invalid_object                                            =	448	,
    DRM_Prdy_xb_object_already_exists                                     =	449	,
    DRM_Prdy_xb_required_object_missing                                   =	450	,
    DRM_Prdy_xb_unknown_element_type                                      =	451	,
    DRM_Prdy_keyfile_invalid_platform                                     =	452	,
    DRM_Prdy_keyfile_too_large                                            =	453	,
    DRM_Prdy_keyfile_private_key_not_found                                =	454	,
    DRM_Prdy_keyfile_certificate_chain_not_found                          =	455	,
    DRM_Prdy_keyfile_key_not_found                                        =	456	,
    DRM_Prdy_keyfile_unknown_decryption_method                            =	457	,
    DRM_Prdy_keyfile_invalid_signature                                    =	458	,
    DRM_Prdy_keyfile_internal_decryption_buffer_too_small                 =	459	,
    DRM_Prdy_keyfile_platformid_mismatch                                  =	460	,
    DRM_Prdy_keyfile_certificate_issuer_key_mismatch                      =	461	,
    DRM_Prdy_keyfile_robustnessversion_mismatch                           =	462	,
    DRM_Prdy_keyfile_file_not_closed                                      =	463	,
    DRM_Prdy_keyfile_not_inited                                           =	464	,
    DRM_Prdy_keyfile_format_invalid                                       =	465	,
    DRM_Prdy_keyfile_update_not_allowed                                   =	466	,
    DRM_Prdy_prnd_message_version_invalid                                 =	467	,
    DRM_Prdy_prnd_message_wrong_type                                      =	468	,
    DRM_Prdy_prnd_message_invalid                                         =	469	,
    DRM_Prdy_prnd_session_id_invalid                                      =	470	,
    DRM_Prdy_prnd_proximity_detection_request_channel_type_unsupported    =	471	,
    DRM_Prdy_prnd_proximity_detection_response_invalid                    =	472	,
    DRM_Prdy_prnd_proximity_detection_response_timeout                    =	473	,
    DRM_Prdy_prnd_license_request_cid_callback_required                   =	474	,
    DRM_Prdy_prnd_license_response_clmid_invalid                          =	475	,
    DRM_Prdy_prnd_certificate_not_receiver                                =	476	,
    DRM_Prdy_prnd_cannot_renew_using_new_session                          =	477	,
    DRM_Prdy_prnd_invalid_custom_data_type                                =	478	,
    DRM_Prdy_prnd_clock_out_of_sync                                       =	479	,
    DRM_Prdy_prnd_cannot_rebind_prnd_received_license                     =	480	,
    DRM_Prdy_prnd_cannot_register_using_existing_session                  =	481	,
    DRM_Prdy_prnd_busy_performing_renewal                                 =	482	,
    DRM_Prdy_prnd_license_request_invalid_action                          =	483	,
    DRM_Prdy_prnd_transmitter_unauthorized                                =	484	,
    DRM_Prdy_oemhal_not_initialized                                       =	485	,
    DRM_Prdy_oemhal_out_of_key_registers                                  =	486	,
    DRM_Prdy_oemhal_keys_in_use                                           =	487	,
    DRM_Prdy_oemhal_no_key                                                =	488	,
    DRM_Prdy_oemhal_unsupported_key_type                                  =	489	,
    DRM_Prdy_oemhal_unsupported_key_wrapping_format                       =	490	,
    DRM_Prdy_oemhal_unsupported_key_length                                =	491	,
    DRM_Prdy_oemhal_unsupported_hash_type                                 =	492	,
    DRM_Prdy_oemhal_unsupported_signature_scheme                          =	493	,
    DRM_Prdy_oemhal_buffer_too_large                                      =	494	,
    DRM_Prdy_oemhal_sample_encryption_mode_not_permitted                  =	495	,
    DRM_Prdy_m2ts_pat_pid_is_not_zero                                     =	496	,
    DRM_Prdy_m2ts_pts_not_exist                                           =	497	,
    DRM_Prdy_m2ts_pes_packet_length_not_specified                         =	498	,
    DRM_Prdy_m2ts_output_buffer_full                                      =	499	,
    DRM_Prdy_m2ts_context_not_initialized                                 =	500	,
    DRM_Prdy_m2ts_need_key_data                                           =	501	,
    DRM_Prdy_m2ts_ddplus_format_invalid                                   =	502	,
    DRM_Prdy_m2ts_not_unit_start_packet                                   =	503	,
    DRM_Prdy_m2ts_too_many_subsamples                                     =	504	,
    DRM_Prdy_m2ts_table_id_invalid                                        =	505	,
    DRM_Prdy_m2ts_packet_sync_byte_invalid                                =	506	,
    DRM_Prdy_m2ts_adaptation_length_invalid                               =	507	,
    DRM_Prdy_m2ts_pat_header_invalid                                      =	508	,
    DRM_Prdy_m2ts_pmt_header_invalid                                      =	509	,
    DRM_Prdy_m2ts_pes_start_code_not_found                                =	510	,
    DRM_Prdy_m2ts_stream_or_packet_type_changed                           =	511	,
    DRM_Prdy_m2ts_internal_error                                          =	512	,
    DRM_Prdy_m2ts_adts_format_invalid                                     =	513	,
    DRM_Prdy_m2ts_mpega_format_invalid                                    =	514	,
    DRM_Prdy_m2ts_ca_descriptor_length_invalid                            =	515	,
    DRM_Prdy_m2ts_crc_field_invalid                                       =	516	,
    DRM_Prdy_m2ts_incomplete_section_header                               =	517	,
    DRM_Prdy_m2ts_invalid_unaligned_data                                  =	518	,
    DRM_Prdy_m2ts_get_encrypted_data_first                                =	519	,
    DRM_Prdy_m2ts_cannot_change_parameter                                 =	520	,
    DRM_Prdy_m2ts_unknown_packet                                          =	521	,
    DRM_Prdy_m2ts_drop_packet                                             =	522	,
    DRM_Prdy_m2ts_drop_pes                                                =	523	,
    DRM_Prdy_m2ts_incomplete_pes                                          =	524	,
    DRM_Prdy_m2ts_waited_too_long                                         =	525	,
    DRM_Prdy_m2ts_section_length_invalid                                  =	526	,
    DRM_Prdy_m2ts_program_info_length_invalid                             =	527	,
    DRM_Prdy_m2ts_pes_header_invalid                                      =	528	,
    DRM_Prdy_m2ts_ecm_payload_over_limit                                  =	529	,
    DRM_Prdy_m2ts_set_ca_pid_failed                                       =	530	,
    DRM_Prdy_licgen_cannot_persist_license                                =	531	,
    DRM_Prdy_licgen_persistent_remote_license                             =	532	,
    DRM_Prdy_licgen_expire_after_first_play_remote_license                =	533	,
    DRM_Prdy_licgen_root_license_cannot_encrypt                           =	534	,
    DRM_Prdy_licgen_embed_local_license                                   =	535	,
    DRM_Prdy_licgen_local_license_with_remote_certificate                 =	536	,
    DRM_Prdy_licgen_play_enabler_remote_license                           =	537	,
    DRM_Prdy_licgen_duplicate_play_enabler                                =	538	,
    DRM_Prdy_hdcp_err                                                     =	539	,
    DRM_Prdy_win32_file_not_found                                         =	540	,
    DRM_Prdy_handle                                                       =	541	,
    DRM_Prdy_win32_no_more_files                                          =	542	,
    DRM_Prdy_buffertoosmall                                               =	543	,
    DRM_Prdy_nomore                                                       =	544	,
    DRM_Prdy_arithmetic_overflow                                          =	545	,
    DRM_Prdy_not_found                                                    =	546	,
    DRM_Prdy_invalid_command_line                                         =	547	,
    DRM_Prdy_failed_to_store_license                                      =	548	,
    DRM_Prdy_parameters_mismatched                                        =	549	,
    DRM_Prdy_not_all_stored                                               =	550 ,
    DRM_Prdy_Err_Max
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

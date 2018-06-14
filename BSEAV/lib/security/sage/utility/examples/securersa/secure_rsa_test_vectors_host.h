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

#ifndef SECURE_RSA_TEST_VECTORS_HOST_H__
#define SECURE_RSA_TEST_VECTORS_HOST_H__


#include "bstd.h"


#define INPUT_DATA_VALUE  0x11

#define DIGEST_LEN_SHA1     20
#define DIGEST_LEN_SHA256   32

#define KEY_LEN_128         16
#define KEY_LEN_256         32

#define KEY3_KEY_SLOT_IKR_A  0
#define KEY3_KEY_SLOT_IKR_B  1
#define KEY3_KEY_SLOT_IKR_C  2
#define KEY3_KEY_SLOT_IKR_D  3
#define KEY3_KEY_SLOT_CA_A   4
#define KEY3_KEY_SLOT_CP_A   5
#define KEY3_KEY_SLOT_CA_B   6
#define KEY3_KEY_SLOT_CP_B   7

#define KPK_KEY_SLOT_A       0
#define KPK_KEY_SLOT_B       1

#define RSA_NUM_KEYS         6

#define RSA_KEY_LEN_1024   128
#define RSA_KEY_LEN_2048   256
#define RSA_KEY_LEN_3072   384


#ifdef SECURE_RSA_TEST_1024

#define RSA_KEY_LEN                 128
#define RSA_KEY_SLOT_SIGN_HOST_USAGE  0
#define RSA_KEY_SLOT_DECRYPT_IKR      1
#define RSA_KEY_SLOT_DECRYPT_CA       2
#define RSA_KEY_SLOT_DECRYPT_CP       3
#define RSA_KEY_SLOT_DECRYPT_KPK      4
#define RSA_KEY_SLOT_VERIFY           5

#elif defined(SECURE_RSA_TEST_2048)

#define RSA_KEY_LEN                 256
#define RSA_KEY_SLOT_SIGN_HOST_USAGE  0
#define RSA_KEY_SLOT_DECRYPT_IKR      1
#define RSA_KEY_SLOT_DECRYPT_CA       2
#define RSA_KEY_SLOT_DECRYPT_CP       3
#define RSA_KEY_SLOT_DECRYPT_KPK      4
#define RSA_KEY_SLOT_VERIFY           5

#elif defined(SECURE_RSA_TEST_3072)

#define RSA_KEY_LEN                 384
#define RSA_KEY_SLOT_DECRYPT_IKR      0
#define RSA_KEY_SLOT_SIGN_HOST_USAGE  1
#define RSA_KEY_SLOT_VERIFY           2
#define RSA_KEY_SLOT_DECRYPT_CA       3
#define RSA_KEY_SLOT_DECRYPT_CP       4
#define RSA_KEY_SLOT_DECRYPT_KPK      5

#endif


#if defined(SECURE_RSA_TEST_1024) || defined(SECURE_RSA_TEST_2048) || defined(SECURE_RSA_TEST_3072)

#define RSA_KEY_SLOT_PUBLIC_A         6
#define RSA_KEY_SLOT_PUBLIC_B         7

#define SALT_LEN_16 16

#define DATA_ABC_LEN 3
extern uint8_t DataAbc[DATA_ABC_LEN];

#define HMAC_SHA1_IKR128_LEN 20
extern uint8_t HmacSha1_Ikr128[HMAC_SHA1_IKR128_LEN];

#define HMAC_SHA256_IKR256_LEN 32
extern uint8_t HmacSha256_Ikr256[HMAC_SHA256_IKR256_LEN];

#define KEY_APPEND_SHA1_IKR128_A_LEN 20
extern uint8_t KeyAppendSha1_Ikr128_A[KEY_APPEND_SHA1_IKR128_A_LEN];

#define KEY_APPEND_SHA1_IKR128_B_LEN 20
extern uint8_t KeyAppendSha1_Ikr128_B[KEY_APPEND_SHA1_IKR128_B_LEN];

#define KEY_APPEND_SHA256_IKR256_A_LEN 32
extern uint8_t KeyAppendSha256_Ikr256_A[KEY_APPEND_SHA256_IKR256_A_LEN];

#define KEY_APPEND_SHA256_IKR256_B_LEN 32
extern uint8_t KeyAppendSha256_Ikr256_B[KEY_APPEND_SHA256_IKR256_B_LEN];

#define KEY_SETTINGS_IKR_LEN 16
extern uint8_t KeySettings_Ikr[KEY_SETTINGS_IKR_LEN];

#define KEY_SETTINGS_CA_128_4_LEN 16
extern uint8_t KeySettings_Ca_128_4[KEY_SETTINGS_CA_128_4_LEN];

#define KEY_SETTINGS_CA_128_5_LEN 16
extern uint8_t KeySettings_Ca_128_5[KEY_SETTINGS_CA_128_5_LEN];

#define KEY_SETTINGS_CP_128_4_LEN 16
extern uint8_t KeySettings_Cp_128_4[KEY_SETTINGS_CP_128_4_LEN];

#define KEY_SETTINGS_CP_128_5_LEN 16
extern uint8_t KeySettings_Cp_128_5[KEY_SETTINGS_CP_128_5_LEN];

#define KEY_SETTINGS_KPK_A_LEN 16
extern uint8_t KeySettings_KpkA[KEY_SETTINGS_KPK_A_LEN];

#define KEY_SETTINGS_KPK_B_LEN 16
extern uint8_t KeySettings_KpkB[KEY_SETTINGS_KPK_B_LEN];

#define KEY_SETTINGS_RENEWABLE_RSA_A_LEN 16
extern uint8_t KeySettings_RenewableRsaA[KEY_SETTINGS_RENEWABLE_RSA_A_LEN];

#define KEY_SETTINGS_RENEWABLE_RSA_B_LEN 16
extern uint8_t KeySettings_RenewableRsaB[KEY_SETTINGS_RENEWABLE_RSA_B_LEN];

#define CP_3DES_INPUT_LEN 32
extern uint8_t Cp_3Des_Input[CP_3DES_INPUT_LEN];

#define CP_3DES_OUTPUT_LEN 32
extern uint8_t Cp_3Des_Output[CP_3DES_OUTPUT_LEN];

#define CP_AESECB_INPUT_LEN 32
extern uint8_t Cp_AesEcb_Input[CP_AESECB_INPUT_LEN];

#define CP_AESECB_OUTPUT_LEN 32
extern uint8_t Cp_AesEcb_Output[CP_AESECB_OUTPUT_LEN];

#define CP_AESCBC_IV_LEN 16
extern uint8_t Cp_AesCbc_Iv[CP_AESCBC_IV_LEN];

#define CP_AESCBC_INPUT_LEN 400
extern uint8_t Cp_AesCbc_Input[CP_AESCBC_INPUT_LEN];

#define CP_AESCBC_OUTPUT_LEN 400
extern uint8_t Cp_AesCbc_Output[CP_AESCBC_OUTPUT_LEN];

#define CP_AESCTR_IV_LEN 16
extern uint8_t Cp_AesCtr_Iv[CP_AESCTR_IV_LEN];

#define CP_AESCTR_INPUT_LEN 400
extern uint8_t Cp_AesCtr_Input[CP_AESCTR_INPUT_LEN];

#define CP_AESCTR_OUTPUT_LEN 400
extern uint8_t Cp_AesCtr_Output[CP_AESCTR_OUTPUT_LEN];

#define CP_128_4_PROCIN_LEN 16
extern uint8_t Cp_128_4_ProcIn[CP_128_4_PROCIN_LEN];

#define CP_128_5_PROCIN_LEN 32
extern uint8_t Cp_128_5_ProcIn[CP_128_5_PROCIN_LEN];

#define CA_AESCBC_IV_LEN 16
extern uint8_t Ca_AesCbc_Iv[CA_AESCBC_IV_LEN];

#define CA_128_4_PROCIN_LEN 16
extern uint8_t Ca_128_4_ProcIn[CA_128_4_PROCIN_LEN];

#define CA_128_5_PROCIN_LEN 32
extern uint8_t Ca_128_5_ProcIn[CA_128_5_PROCIN_LEN];

#define IKRCP_CLEAR_KEY_LEN 16
extern uint8_t IkrCp_ClearKey[IKRCP_CLEAR_KEY_LEN];

#define IKR256_CLEAR_KEY_LEN 32
extern uint8_t Ikr256_ClearKey[IKR256_CLEAR_KEY_LEN];

#define IV_IKR_IKR_LEN 16
extern uint8_t Iv_IkrIkr[IV_IKR_IKR_LEN];

#define ENC_IKR128_IKR128_A_LEN 16
extern uint8_t Enc_Ikr128_Ikr128A[ENC_IKR128_IKR128_A_LEN];

#define ENC_IKR128_A_IKR128_B_LEN 16
extern uint8_t Enc_Ikr128A_Ikr128B[ENC_IKR128_A_IKR128_B_LEN];

#define ENC_IKR128_B_IKR128_C_LEN 16
extern uint8_t Enc_Ikr128B_Ikr128C[ENC_IKR128_B_IKR128_C_LEN];

#define ENC_IKR128_C_IKR256_LEN 32
extern uint8_t Enc_Ikr128C_Ikr256[ENC_IKR128_C_IKR256_LEN];

#define ENC_KPK_A_IKR128_LEN 16
extern uint8_t Enc_KpkA_Ikr128[ENC_KPK_A_IKR128_LEN];

#define IV_KPK_B_IKR256_LEN 16
extern uint8_t Iv_KpkB_Ikr256[IV_KPK_B_IKR256_LEN];

#define ENC_KPK_B_IKR256_LEN 32
extern uint8_t Enc_KpkB_Ikr256[ENC_KPK_B_IKR256_LEN];

#define RSA_PUBLIC_1024_LEN 128
extern uint8_t RsaPublic1024[RSA_PUBLIC_1024_LEN];

#define RSA_PUBLIC_2048_LEN 256
extern uint8_t RsaPublic2048[RSA_PUBLIC_2048_LEN];

#define RSA_PUBLIC_3072_LEN 384
extern uint8_t RsaPublic3072[RSA_PUBLIC_3072_LEN];

#define SIG_PUBLIC_1024_2048_PKCS_SHA1_LEN 128
extern uint8_t Sig_Public_1024_2048_PkcsSha256[SIG_PUBLIC_1024_2048_PKCS_SHA1_LEN];

#define SIG_PUBLIC_2048_3072_PKCS_SHA1_LEN 256
extern uint8_t Sig_Public_2048_3072_PssSha1Salt0[SIG_PUBLIC_2048_3072_PKCS_SHA1_LEN];

#define SIG_PUBLIC_3072_1024_PSS_SHA256_SALT16_LEN 384
extern uint8_t Sig_Public_3072_1024_PssSha256Salt16[SIG_PUBLIC_3072_1024_PSS_SHA256_SALT16_LEN];

#define RSA_PUBLIC_1024_OFFSET_POS 16
#define RSA_PUBLIC_OFFSET_1024_LEN 160
extern uint8_t RsaPublicOffset1024[RSA_PUBLIC_OFFSET_1024_LEN];

#define RSA_PUBLIC_2048_OFFSET_POS 16
#define RSA_PUBLIC_OFFSET_2048_LEN 272
extern uint8_t RsaPublicOffset2048[RSA_PUBLIC_OFFSET_2048_LEN];

#define RSA_PUBLIC_3072_OFFSET_POS 0
#define RSA_PUBLIC_OFFSET_3072_LEN 400
extern uint8_t RsaPublicOffset3072[RSA_PUBLIC_OFFSET_3072_LEN];

#define SIG_PUBLIC_OFFSET_1024_2048_PKCS_SHA1_LEN 128
extern uint8_t Sig_PublicOffset_1024_2048_PkcsSha256[SIG_PUBLIC_OFFSET_1024_2048_PKCS_SHA1_LEN];

#define SIG_PUBLIC_OFFSET_2048_3072_PKCS_SHA1_LEN 256
extern uint8_t Sig_PublicOffset_2048_3072_PssSha1Salt0[SIG_PUBLIC_OFFSET_2048_3072_PKCS_SHA1_LEN];

#define SIG_PUBLIC_OFFSET_3072_1024_PSS_SHA256_SALT16_LEN 384
extern uint8_t Sig_PublicOffset_3072_1024_PssSha256Salt16[SIG_PUBLIC_OFFSET_3072_1024_PSS_SHA256_SALT16_LEN];

#define IV_RENEWABLE_RSA_B_LEN 16
extern uint8_t Iv_RenewableRsaB[IV_RENEWABLE_RSA_B_LEN];

#define RSA_VERIFY_KEY_LEN RSA_KEY_LEN
extern uint8_t RsaVerifyKey[RSA_VERIFY_KEY_LEN];

#define SIG_PKCS_SHA1_LEN RSA_KEY_LEN
extern uint8_t Sig_PkcsSha1[SIG_PKCS_SHA1_LEN];

#define SIG_PKCS_SHA256_LEN RSA_KEY_LEN
extern uint8_t Sig_PkcsSha256[SIG_PKCS_SHA256_LEN];

#define SIG_PSS_SHA1_SALT0_LEN RSA_KEY_LEN
extern uint8_t Sig_PssSha1Salt0[SIG_PSS_SHA1_SALT0_LEN];

#define SIG_PSS_SHA256_SALT16_LEN RSA_KEY_LEN
extern uint8_t Sig_PssSha256Salt16[SIG_PSS_SHA256_SALT16_LEN];

#define SIG_PKCS_SHA1_RENEWABLE_LEN RSA_KEY_LEN
extern uint8_t Sig_PkcsSha1_Renewable[SIG_PKCS_SHA1_RENEWABLE_LEN];

#define HOST_USAGE_PRIVATE_A_LEN RSA_KEY_LEN
extern uint8_t HostUsage_PrivateA[HOST_USAGE_PRIVATE_A_LEN];

#define HOST_USAGE_PUBLIC_A_LEN RSA_KEY_LEN
extern uint8_t HostUsage_PublicA[HOST_USAGE_PUBLIC_A_LEN];

#define HOST_USAGE_PUBLIC_B_LEN RSA_KEY_LEN
extern uint8_t HostUsage_PublicB[HOST_USAGE_PUBLIC_B_LEN];

#define HOST_USAGE_PRIVATE_C_LEN RSA_KEY_LEN
extern uint8_t HostUsage_PrivateC[HOST_USAGE_PRIVATE_C_LEN];

#define HOST_USAGE_PUBLIC_C_LEN RSA_KEY_LEN
extern uint8_t HostUsage_PublicC[HOST_USAGE_PUBLIC_C_LEN];

#define ENC_IKR128_PKCS_LEN RSA_KEY_LEN
extern uint8_t Enc_Ikr128_Pkcs[ENC_IKR128_PKCS_LEN];

#define SIG_IKR128_PKCS_SHA1_LEN RSA_KEY_LEN
extern uint8_t Sig_Ikr128_PkcsSha1[SIG_IKR128_PKCS_SHA1_LEN];

#define ENC_RENEWABLE_IKR128_PKCS_LEN RSA_KEY_LEN
extern uint8_t Enc_Renewable_Ikr128_Pkcs[ENC_RENEWABLE_IKR128_PKCS_LEN];

#define SIG_RENEWABLE_IKR128_PKCS_SHA1_LEN RSA_KEY_LEN
extern uint8_t Sig_Renewable_Ikr128_PkcsSha1[SIG_RENEWABLE_IKR128_PKCS_SHA1_LEN];

#define ENC_IKR256_PKCS_LEN RSA_KEY_LEN
extern uint8_t Enc_Ikr256_Pkcs[ENC_IKR256_PKCS_LEN];

#define SIG_IKR256_PKCS_SHA256_LEN RSA_KEY_LEN
extern uint8_t Sig_Ikr256_PkcsSha256[SIG_IKR256_PKCS_SHA256_LEN];

#define ENC_IKRCA_OAEP_SHA256_LEN RSA_KEY_LEN
extern uint8_t Enc_IkrCa_OaepSha256[ENC_IKRCA_OAEP_SHA256_LEN];

#define SIG_IKRCA_PSS_SHA1_SALT0_LEN RSA_KEY_LEN
extern uint8_t Sig_IkrCa_PssSha1Salt0[SIG_IKRCA_PSS_SHA1_SALT0_LEN];

#define ENC_IKRCP_OAEP_SHA1_LEN RSA_KEY_LEN
extern uint8_t Enc_IkrCp_OaepSha1[ENC_IKRCP_OAEP_SHA1_LEN];

#define SIG_IKRCP_PSS_SHA256_SALT16_LEN RSA_KEY_LEN
extern uint8_t Sig_IkrCp_PssSha256Salt16[SIG_IKRCP_PSS_SHA256_SALT16_LEN];

#define ENC_CA_PKCS_LEN RSA_KEY_LEN
extern uint8_t Enc_Ca_Pkcs[ENC_CA_PKCS_LEN];

#define SIG_CA_128_4_PKCS_LEN RSA_KEY_LEN
extern uint8_t Sig_Ca_128_4_Pkcs[SIG_CA_128_4_PKCS_LEN];

#define SIG_CA_128_5_PKCS_LEN RSA_KEY_LEN
extern uint8_t Sig_Ca_128_5_Pkcs[SIG_CA_128_5_PKCS_LEN];

#define ENC_CP_PKCS_LEN RSA_KEY_LEN
extern uint8_t Enc_Cp_Pkcs[ENC_CP_PKCS_LEN];

#define SIG_CP_128_4_PKCS_LEN RSA_KEY_LEN
extern uint8_t Sig_Cp_128_4_Pkcs[SIG_CP_128_4_PKCS_LEN];

#define SIG_CP_128_5_PKCS_LEN RSA_KEY_LEN
extern uint8_t Sig_Cp_128_5_Pkcs[SIG_CP_128_5_PKCS_LEN];

#define ENC_KPK_A_PKCS_LEN RSA_KEY_LEN
extern uint8_t Enc_KpkA_Pkcs[ENC_KPK_A_PKCS_LEN];

#define SIG_KPK_A_PKCS_SHA1_LEN RSA_KEY_LEN
extern uint8_t Sig_KpkA_PkcsSha1[SIG_KPK_A_PKCS_SHA1_LEN];

#define SIG_KPK_A_PSS_SHA256_SALT16_LEN RSA_KEY_LEN
extern uint8_t Sig_KpkA_PssSha256Salt16[SIG_KPK_A_PSS_SHA256_SALT16_LEN];

#define ENC_KPK_B_OAEP_SHA1_LEN RSA_KEY_LEN
extern uint8_t Enc_KpkB_OaepSha1[ENC_KPK_B_OAEP_SHA1_LEN];

#define SIG_PUBLIC_VERIFY_1024_PKCS_SHA1_LEN RSA_KEY_LEN
extern uint8_t Sig_Public_Verify_1024_PkcsSha1[SIG_PUBLIC_VERIFY_1024_PKCS_SHA1_LEN];

#define SIG_PUBLIC_1024_VERIFY_PKCS_SHA1_LEN RSA_KEY_LEN_1024
extern uint8_t Sig_Public_1024_Verify_PkcsSha1[SIG_PUBLIC_1024_VERIFY_PKCS_SHA1_LEN];

#define ENC_RENEWABLE_RSA_A_LEN (2*RSA_KEY_LEN)
extern uint8_t Enc_RenewableRsaA[ENC_RENEWABLE_RSA_A_LEN];

#define SIG_RENEWABLE_RSA_A_PKCS_SHA1_LEN RSA_KEY_LEN
extern uint8_t Sig_RenewableRsaA_PkcsSha1[SIG_RENEWABLE_RSA_A_PKCS_SHA1_LEN];

#define SIG_RENEWABLE_RSA_A_PSS_SHA256_SALT16_LEN RSA_KEY_LEN
extern uint8_t Sig_RenewableRsaA_PssSha256Salt16[SIG_RENEWABLE_RSA_A_PSS_SHA256_SALT16_LEN];

#define ENC_RENEWABLE_RSA_B_LEN (2*RSA_KEY_LEN)
extern uint8_t Enc_RenewableRsaB[ENC_RENEWABLE_RSA_B_LEN];


#elif defined(SECURE_RSA_TEST_MULTI)

/* Multi context test data */

#define RSA_1024_KEY_SLOT_SIGN_HOST_USAGE       0
#define RSA_1024_KEY_SLOT_DECRYPT_IKR           1
#define RSA_1024_KEY_SLOT_VERIFY                5

#define RSA_2048_KEY_SLOT_SIGN_HOST_USAGE       0
#define RSA_2048_KEY_SLOT_DECRYPT_IKR           1
#define RSA_2048_KEY_SLOT_VERIFY                5

#define RSA_3072_KEY_SLOT_SIGN_HOST_USAGE       1
#define RSA_3072_KEY_SLOT_DECRYPT_IKR           0
#define RSA_3072_KEY_SLOT_VERIFY                2

#define KEY_SETTINGS_IKR_LEN 16
extern uint8_t KeySettings_Ikr[KEY_SETTINGS_IKR_LEN];

#define SIG_1024_PKCS_SHA1_LEN 128
extern uint8_t Sig_1024_PkcsSha1[SIG_1024_PKCS_SHA1_LEN];

#define SIG_2048_PKCS_SHA1_LEN 256
extern uint8_t Sig_2048_PkcsSha1[SIG_2048_PKCS_SHA1_LEN];

#define SIG_3072_PKCS_SHA1_LEN 384
extern uint8_t Sig_3072_PkcsSha1[SIG_3072_PKCS_SHA1_LEN];

#define HMAC_SHA1_IKR128_MULTI1_LEN 20
extern uint8_t HmacSha1_Ikr128_Multi1[HMAC_SHA1_IKR128_MULTI1_LEN];

#define HMAC_SHA1_IKR128_MULTI2_LEN 20
extern uint8_t HmacSha1_Ikr128_Multi2[HMAC_SHA1_IKR128_MULTI2_LEN];

#define HMAC_SHA1_IKR128_MULTI3_LEN 20
extern uint8_t HmacSha1_Ikr128_Multi3[HMAC_SHA1_IKR128_MULTI3_LEN];

#define ENC_1024_IKR128_MULTI1_PKCS_LEN 128
extern uint8_t Enc_1024_Ikr128_Multi1_Pkcs[ENC_1024_IKR128_MULTI1_PKCS_LEN];

#define SIG_1024_IKR128_MULTI1_PKCS_SHA1_LEN 128
extern uint8_t Sig_1024_Ikr128_Multi1_PkcsSha1[SIG_1024_IKR128_MULTI1_PKCS_SHA1_LEN];

#define ENC_2048_IKR128_MULTI2_PKCS_LEN 256
extern uint8_t Enc_2048_Ikr128_Multi2_Pkcs[ENC_2048_IKR128_MULTI2_PKCS_LEN];

#define SIG_2048_IKR128_MULTI2_PKCS_SHA1_LEN 256
extern uint8_t Sig_2048_Ikr128_Multi2_PkcsSha1[SIG_2048_IKR128_MULTI2_PKCS_SHA1_LEN];

#define ENC_3072_IKR128_MULTI3_PKCS_LEN 384
extern uint8_t Enc_3072_Ikr128_Multi3_Pkcs[ENC_3072_IKR128_MULTI3_PKCS_LEN];

#define SIG_3072_IKR128_MULTI3_PKCS_SHA1_LEN 384
extern uint8_t Sig_3072_Ikr128_Multi3_PkcsSha1[SIG_3072_IKR128_MULTI3_PKCS_SHA1_LEN];

#endif

#endif

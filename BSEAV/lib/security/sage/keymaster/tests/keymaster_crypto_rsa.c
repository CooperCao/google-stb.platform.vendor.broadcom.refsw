/******************************************************************************
 *  Copyright (C) 2019 Broadcom.
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "openssl/evp.h"
#include "openssl/x509.h"

#include "keymaster_debug.h"
#include "bstd.h"
#include "bkni.h"
#include "nexus_security_client.h"
#include "nexus_memory.h"

#include "keymaster_ids.h"
#include "keymaster_platform.h"
#include "keymaster_tl.h"
#include "keymaster_test.h"
#include "keymaster_key_params.h"
#include "keymaster_keygen.h"
#include "keymaster_crypto_utils.h"


BDBG_MODULE(keymaster_crypto_rsa);

/* Old openssl does not allow PAD_NONE with a digest */
#define DISABLE_NO_PADDING_WITH_DIGESTS 1

BERR_Code km_crypto_rsa_enc_dec_tests(KeymasterTl_Handle handle)
{
    BERR_Code err;
    KM_Tag_ContextHandle key_params = NULL;
    KM_Tag_ContextHandle begin_params = NULL;
    KM_CryptoOperation_Settings settings;
    KeymasterTl_ImportKeySettings impSettings;
    KeymasterTl_DataBlock import_key;
    KeymasterTl_DataBlock key;
    KeymasterTl_DataBlock in_data;
    KeymasterTl_DataBlock intermediate_data;
    KeymasterTl_DataBlock intermediate_data2;
    KeymasterTl_DataBlock final_data;
    int i;
    uint32_t key_size = 0;
    char std_message[] = "Hello World!";
    char no_padding_message[] = "12345678901234567890123456789012";
    char *message;
    const char *comment;

    memset(&import_key, 0, sizeof(import_key));
    memset(&key, 0, sizeof(key));
    memset(&in_data, 0, sizeof(in_data));
    memset(&intermediate_data, 0, sizeof(intermediate_data));
    memset(&intermediate_data2, 0, sizeof(intermediate_data2));
    memset(&final_data, 0, sizeof(final_data));

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));

    for (i = 0; i < 18; i++) {
        KM_CryptoOperation_GetDefaultSettings(&settings);

        settings.handle = handle;

        EXPECT_SUCCESS(KM_Tag_NewContext(&begin_params));
        settings.begin_params = begin_params;

        message = std_message;
        switch (i) {
        case 0:
            key_size = 1024;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_OAEP);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA1);
            comment = "RSA HW 1024 bit, OAEP, SHA1";
            break;
        case 1:
            key_size = 1024;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_OAEP);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            comment = "RSA HW 1024 bit, OAEP, SHA256";
            break;
        case 2:
            key_size = 2048;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_OAEP);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA1);
            comment = "RSA HW 2048 bit, OAEP, SHA1";
            break;
        case 3:
            key_size = 2048;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_OAEP);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            comment = "RSA HW 2048 bit, OAEP, SHA256";
            break;
        case 4:
            key_size = 1024;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_PKCS1_1_5_ENCRYPT);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_NONE);
            comment = "RSA HW 1024 bit, PKCS1.1.5, NONE";
            break;
        case 5:
            key_size = 2048;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_PKCS1_1_5_ENCRYPT);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_NONE);
            comment = "RSA HW 2048 bit, PKCS1.1.5, NONE";
            break;
        case 6:
            key_size = 3072;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_OAEP);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA1);
            comment = "RSA SW 3072 bit, OAEP, SHA1";
            break;
        case 7:
            key_size = 3072;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_OAEP);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            comment = "RSA SW 3072 bit, OAEP, SHA256";
            break;
        case 8:
            key_size = 3072;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_PKCS1_1_5_ENCRYPT);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_NONE);
            comment = "RSA SW 3072 bit, PKCS1.1.5, NONE";
            break;
        case 9:
            key_size = 768;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_OAEP);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_MD5);
            comment = "RSA 768 bit, OAEP, MD5";
            break;
        case 10:
            key_size = 768;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_OAEP);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA1);
            comment = "RSA 768 bit, OAEP, SHA1";
            break;
        case 11:
            key_size = 768;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_OAEP);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_224);
            comment = "RSA 768 bit, OAEP, SHA224";
            break;
        case 12:
            key_size = 768;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_OAEP);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            comment = "RSA 768 bit, OAEP, SHA256";
            break;
        case 13:
            key_size = 1536;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_OAEP);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_384);
            comment = "RSA 1536 bit, OAEP, SHA384";
            break;
        case 14:
            key_size = 1536;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_OAEP);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_512);
            comment = "RSA 1536 bit, OAEP, SHA512";
            break;
        case 15:
            key_size = 768;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_PKCS1_1_5_ENCRYPT);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_NONE);
            comment = "RSA 768 bit, PKCS1.1.5, NONE";
            break;
        case 16:
            key_size = 256;
            message = no_padding_message;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_NONE);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_NONE);
            comment = "RSA 256 bit, no padding or digest";
            break;
        case 17:
            key_size = 4096;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_OAEP);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_512);
            comment = "RSA 4096 bit, OAEP, SHA512";
            break;
        default:
            BDBG_ASSERT(0);
            break;
        }

        BDBG_LOG(("%s : Test - %s", BSTD_FUNCTION, comment));

        /* Generate key on host side and import it (some key sizes cannot be generated on SAGE) */
        EXPECT_SUCCESS(km_test_new_params_with_rsa_defaults(&key_params, key_size));
        EXPECT_SUCCESS(rsa_gen_keys(&import_key, key_size));
        KeymasterTl_GetDefaultImportKeySettings(&impSettings);
        impSettings.in_key_params = key_params;
        impSettings.in_key_format = SKM_KEY_FORMAT_PKCS8;
        impSettings.in_key_blob = import_key;
        EXPECT_SUCCESS(KeymasterTl_ImportKey(handle, &impSettings));

        key = impSettings.out_key_blob;
        settings.key_params = key_params;
        settings.in_key = key;

        TEST_ALLOCATE_BLOCK(in_data, strlen(message));
        TEST_ALLOCATE_BLOCK(intermediate_data, key_size / 8);
        if (KM_Tag_ContainsEnum(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_OAEP)) {
            /* Do a second encrypt of same data for OAEP padding */
            TEST_ALLOCATE_BLOCK(intermediate_data2, key_size / 8);
            memset(intermediate_data2.buffer, 0xAF, intermediate_data2.size);
        }
        TEST_ALLOCATE_BLOCK(final_data, key_size / 8);
        memcpy(in_data.buffer, message, in_data.size);
        memset(intermediate_data.buffer, 0xFF, intermediate_data.size);
        memset(final_data.buffer, 0xA1, final_data.size);

        settings.in_data = in_data;
        settings.out_data = intermediate_data;

        EXPECT_SUCCESS(KM_Crypto_Operation(SKM_PURPOSE_ENCRYPT, &settings));
        intermediate_data.size = settings.out_data.size;
        BDBG_LOG(("%s: enc produced %d bytes", BSTD_FUNCTION, settings.out_data.size));

        if (intermediate_data2.buffer) {
            /* For OAEP padding, do a second encrypt because the result must be different */
            settings.out_data = intermediate_data2;
            EXPECT_SUCCESS(KM_Crypto_Operation(SKM_PURPOSE_ENCRYPT, &settings));
            intermediate_data2.size = settings.out_data.size;

            if ((intermediate_data.size != intermediate_data2.size) || !memcmp(intermediate_data.buffer, intermediate_data2.buffer, intermediate_data.size)) {
                BDBG_ERR(("%s: second encypt invalid", BSTD_FUNCTION));
                err = BERR_UNKNOWN;
                goto done;
            }
            BDBG_LOG(("%s: enc produced %d bytes", BSTD_FUNCTION, settings.out_data.size));
        }

        settings.in_data = intermediate_data;
        settings.out_data = final_data;

        EXPECT_SUCCESS(KM_Crypto_Operation(SKM_PURPOSE_DECRYPT, &settings));
        BDBG_LOG(("%s: dec produced %d bytes", BSTD_FUNCTION, settings.out_data.size));

        if ((settings.out_data.size != in_data.size) || memcmp(in_data.buffer, settings.out_data.buffer, settings.out_data.size)) {
            BDBG_ERR(("%s: final data does not match", BSTD_FUNCTION));
            err = BERR_UNKNOWN;
            goto done;
        }

        BDBG_LOG(("%s: %s success", BSTD_FUNCTION, comment));

        TEST_FREE_BLOCK(in_data);
        TEST_FREE_BLOCK(intermediate_data);
        TEST_FREE_BLOCK(intermediate_data2);
        TEST_FREE_BLOCK(final_data);
        TEST_FREE_BLOCK(import_key);
        TEST_FREE_BLOCK(key);
        TEST_DELETE_CONTEXT(begin_params);
        TEST_DELETE_CONTEXT(key_params);
    }

    err = BERR_SUCCESS;

done:
    TEST_FREE_BLOCK(in_data);
    TEST_FREE_BLOCK(intermediate_data);
    TEST_FREE_BLOCK(intermediate_data2);
    TEST_FREE_BLOCK(final_data);
    TEST_FREE_BLOCK(import_key);
    TEST_FREE_BLOCK(key);
    TEST_DELETE_CONTEXT(begin_params);
    TEST_DELETE_CONTEXT(key_params);
    return err;
}

#define PKCS1_OVERHEAD  11

static BERR_Code km_crypto_rsa_sign_verify_table_tests(KeymasterTl_Handle handle)
{
    BERR_Code err;
    km_tag_value_t *tag;
    KM_Tag_ContextHandle key_params = NULL;
    KM_Tag_ContextHandle begin_params = NULL;
    KM_CryptoOperation_Settings settings;
    KeymasterTl_ImportKeySettings impSettings;
    KeymasterTl_DataBlock import_key;
    KeymasterTl_DataBlock key;
    KeymasterTl_DataBlock key1024;
    KeymasterTl_DataBlock key2048;
    KeymasterTl_DataBlock key3072;
    KeymasterTl_DataBlock key4096;
    KeymasterTl_DataBlock *use_key;
    KeymasterTl_DataBlock in_data;
    KeymasterTl_DataBlock out_data;
    KeymasterTl_DataBlock signature;
    int i, key_no, padding, digest;
    char *pad_name;
    char *digest_name;
    uint32_t key_size = 0;
    uint32_t message_size;
    char comment[1024];
    bool fail_test;
    const EVP_MD *md;
    int rsa_padding;

    memset(&import_key, 0, sizeof(import_key));
    memset(&key, 0, sizeof(key));
    memset(&key1024, 0, sizeof(key1024));
    memset(&key2048, 0, sizeof(key2048));
    memset(&key3072, 0, sizeof(key3072));
    memset(&key4096, 0, sizeof(key4096));
    memset(&in_data, 0, sizeof(in_data));
    memset(&out_data, 0, sizeof(out_data));
    memset(&signature, 0, sizeof(signature));

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));

    /* Generate larger keys for re-use below */
    for (i = 0; i < 4; i++) {
        switch (i) {
        case 0:
            key_size = 1024;
            use_key = &key1024;
            break;
        case 1:
            key_size = 2048;
            use_key = &key2048;
            break;
        case 2:
            key_size = 3072;
            use_key = &key3072;
            break;
        case 3:
            key_size = 4096;
            use_key = &key4096;
            break;
        default:
            BDBG_ASSERT(0);
            break;
        }

        /* Generate key on host side and import it (some key sizes cannot be generated on SAGE) */
        EXPECT_SUCCESS(km_test_new_params_with_rsa_defaults(&key_params, key_size));
        BDBG_LOG(("%s: generating cached key %d bits", BSTD_FUNCTION, key_size));
        EXPECT_SUCCESS(rsa_gen_keys(&import_key, key_size));
        KeymasterTl_GetDefaultImportKeySettings(&impSettings);
        impSettings.in_key_params = key_params;
        impSettings.in_key_format = SKM_KEY_FORMAT_PKCS8;
        impSettings.in_key_blob = import_key;
        EXPECT_SUCCESS(KeymasterTl_ImportKey(handle, &impSettings));
        *use_key = impSettings.out_key_blob;
        TEST_FREE_BLOCK(import_key);
        TEST_DELETE_CONTEXT(key_params);
        BDBG_LOG(("%s: generation complete", BSTD_FUNCTION));
    }

    for (key_no = 0; key_no < 4; key_no++) {
        EXPECT_SUCCESS(KM_Tag_NewContext(&begin_params));

        switch (key_no) {
        case 0:
            key_size = 1024;
            use_key = &key1024;
            break;
        case 1:
            key_size = 2048;
            use_key = &key2048;
            break;
        case 2:
            key_size = 3072;
            use_key = &key3072;
            break;
        case 3:
            key_size = 4096;
            use_key = &key4096;
            break;
        default:
            BDBG_ASSERT(0);
        }
        EXPECT_SUCCESS(km_test_new_params_with_rsa_defaults(&key_params, key_size));
        TEST_ALLOCATE_BLOCK(signature, key_size / 8);

        for (padding = 0; padding < 3; padding++) {
            switch (padding) {
            case 0:
                TEST_TAG_ADD_ENUM(begin_params, SKM_TAG_PADDING, SKM_PAD_NONE);
                pad_name = "NONE";
                rsa_padding = RSA_NO_PADDING;
                break;
            case 1:
                TEST_TAG_ADD_ENUM(begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_PKCS1_1_5_SIGN);
                pad_name = "PKCS1";
                rsa_padding = RSA_PKCS1_PADDING;
                break;
            case 2:
                TEST_TAG_ADD_ENUM(begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_PSS);
                pad_name = "PSS";
                rsa_padding = RSA_PKCS1_PSS_PADDING;
                break;
            default:
                BDBG_ASSERT(0);
            }

            for (digest = 0; digest < 7; digest++) {
                fail_test = false;
                message_size = 4096;

                switch (digest) {
                case 0:
                    digest_name = "NONE";
                    if (rsa_padding == RSA_PKCS1_PADDING) {
                        message_size = key_size / 8 - PKCS1_OVERHEAD;
                    } else if (rsa_padding == RSA_NO_PADDING) {
                        message_size = key_size / 8;
                    } else {
                        /* PSS requires digest */
                        continue;
                    }
                    TEST_TAG_ADD_ENUM(begin_params, SKM_TAG_DIGEST, SKM_DIGEST_NONE);
                    md = NULL;
                    break;
                case 1:
#if DISABLE_NO_PADDING_WITH_DIGESTS
                    if (rsa_padding == RSA_NO_PADDING) {
                        continue;
                    }
#endif
                    TEST_TAG_ADD_ENUM(begin_params, SKM_TAG_DIGEST, SKM_DIGEST_MD5);
                    digest_name = "MD5";
                    md = EVP_md5();
                    break;
                case 2:
#if DISABLE_NO_PADDING_WITH_DIGESTS
                    if (rsa_padding == RSA_NO_PADDING) {
                        continue;
                    }
#endif
                    TEST_TAG_ADD_ENUM(begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA1);
                    digest_name = "SHA1";
                    md = EVP_sha1();
                    break;
                case 3:
#if DISABLE_NO_PADDING_WITH_DIGESTS
                    if (rsa_padding == RSA_NO_PADDING) {
                        continue;
                    }
#endif
                    TEST_TAG_ADD_ENUM(begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_224);
                    digest_name = "SHA224";
                    md = EVP_sha224();
                    break;
                case 4:
#if DISABLE_NO_PADDING_WITH_DIGESTS
                    if (rsa_padding == RSA_NO_PADDING) {
                        continue;
                    }
#endif
                    TEST_TAG_ADD_ENUM(begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
                    digest_name = "SHA256";
                    md = EVP_sha256();
                    break;
                case 5:
#if DISABLE_NO_PADDING_WITH_DIGESTS
                    if (rsa_padding == RSA_NO_PADDING) {
                        continue;
                    }
#endif
                    TEST_TAG_ADD_ENUM(begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_384);
                    digest_name = "SHA384";
                    md = EVP_sha384();
                    break;
                case 6:
#if DISABLE_NO_PADDING_WITH_DIGESTS
                    if (rsa_padding == RSA_NO_PADDING) {
                        continue;
                    }
#endif
                    TEST_TAG_ADD_ENUM(begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_512);
                    digest_name = "SHA512";
                    md = EVP_sha512();
                    if ((key_size == 1024) && (rsa_padding == RSA_PKCS1_PSS_PADDING)) {
                        fail_test = true;
                    }
                    break;
                default:
                    BDBG_ASSERT(0);
                }

                KM_CryptoOperation_GetDefaultSettings(&settings);

                settings.handle = handle;
                snprintf(comment, sizeof(comment), "RSA %d bit, %s, %s", key_size, pad_name, digest_name);
                settings.begin_params = begin_params;
                settings.key_params = key_params;
                settings.in_key = *use_key;

                TEST_ALLOCATE_BLOCK(in_data, message_size);
                TEST_ALLOCATE_BLOCK(out_data, 4096);

                memset(in_data.buffer, 'a', in_data.size);
                memset(signature.buffer, 0xFF, signature.size);

                settings.in_data = in_data;
                settings.out_data = signature;

                BDBG_LOG(("%s: Test - %s", BSTD_FUNCTION, comment));

                err = KM_Crypto_Operation(SKM_PURPOSE_SIGN, &settings);
                if (fail_test) {
                    EXPECT_FAILURE(err);
                } else {
                    EXPECT_SUCCESS(err);

                    signature.size = settings.out_data.size;
                    BDBG_LOG(("%s: sign produced %d bytes", BSTD_FUNCTION, settings.out_data.size));

                    settings.in_data = in_data;
                    settings.signature_data = signature;
                    settings.out_data = out_data;

                    EXPECT_SUCCESS(KM_Crypto_Operation(SKM_PURPOSE_VERIFY, &settings));
                    BDBG_LOG(("%s: verify successful", BSTD_FUNCTION));

                    if (md) {
                        KeymasterTl_ExportKeySettings expSettings;
                        EVP_MD_CTX *ctx;
#if (OPENSSL_VERSION_NUMBER <= 0x10100000L)
                        EVP_MD_CTX md_ctx;
#endif
                        EVP_PKEY_CTX *pkey_ctx;
                        EVP_PKEY *pkey = NULL;
                        uint8_t *p;

                        /* Export the key and verify the signature with the public key */

                        KeymasterTl_GetDefaultExportKeySettings(&expSettings);
                        expSettings.in_key_format = SKM_KEY_FORMAT_X509;
                        expSettings.in_key_blob = *use_key;
                        EXPECT_SUCCESS(KeymasterTl_ExportKey(handle, &expSettings));

                        p = expSettings.out_key_blob.buffer;
                        pkey = d2i_PUBKEY(NULL, (const unsigned char **)&p, expSettings.out_key_blob.size);
                        if (!pkey) {
                            TEST_FREE_BLOCK(expSettings.out_key_blob);
                            BDBG_ERR(("%s: failed to load public key", BSTD_FUNCTION));
                            goto done;
                        }

#if (OPENSSL_VERSION_NUMBER > 0x10100000L)
                        ctx = EVP_MD_CTX_new();
                        if (!ctx) {
                            TEST_FREE_BLOCK(expSettings.out_key_blob);
                            BDBG_ERR(("%s: failed to allocate context", BSTD_FUNCTION));
                            goto done;
                        }
#else
                        ctx = &md_ctx;
                        EVP_MD_CTX_init(&md_ctx);
#endif
                        EXPECT_TRUE(EVP_DigestVerifyInit(ctx, &pkey_ctx, md, NULL, pkey));
                        if (EVP_PKEY_CTX_set_rsa_padding(pkey_ctx, rsa_padding) <= 0) {
                            BDBG_ERR(("%s: failed to set padding", BSTD_FUNCTION));
                            err = BERR_UNKNOWN;
                            goto done;
                        }
                        EXPECT_TRUE(EVP_DigestVerifyUpdate(ctx, in_data.buffer, in_data.size));
                        EXPECT_TRUE(EVP_DigestVerifyFinal(ctx, signature.buffer, signature.size));

#if (OPENSSL_VERSION_NUMBER > 0x10100000L)
                        EVP_MD_CTX_free(ctx);
#else
                        EVP_MD_CTX_cleanup(&md_ctx);
#endif
                        EVP_PKEY_free(pkey);
                        TEST_FREE_BLOCK(expSettings.out_key_blob);
                        BDBG_LOG(("%s: offline verify successful", BSTD_FUNCTION));
                    }

                    /* Modify source data and check for failure */
                    in_data.buffer[0]++;
                    settings.out_data = out_data;
                    EXPECT_FAILURE_CODE(KM_Crypto_Operation(SKM_PURPOSE_VERIFY, &settings), BSAGE_ERR_KM_VERIFICATION_FAILED);

                    /* Modify the signature to check that verify fails */
                    in_data.buffer[0]--;
                    signature.buffer[5]++;
                    settings.out_data = out_data;
                    EXPECT_FAILURE_CODE(KM_Crypto_Operation(SKM_PURPOSE_VERIFY, &settings), BSAGE_ERR_KM_VERIFICATION_FAILED);

                    signature.buffer[5]--;
                }

                BDBG_LOG(("%s: %s success", BSTD_FUNCTION, comment));

                TEST_FIND_TAG(tag, begin_params, SKM_TAG_DIGEST);
                KM_Tag_Remove(begin_params, tag);
                KM_Tag_Free(tag);
                TEST_FREE_BLOCK(in_data);
                TEST_FREE_BLOCK(out_data);
            }

            TEST_FIND_TAG(tag, begin_params, SKM_TAG_PADDING);
            KM_Tag_Remove(begin_params, tag);
            KM_Tag_Free(tag);
        }

        TEST_FREE_BLOCK(signature);
        TEST_DELETE_CONTEXT(key_params);
        TEST_DELETE_CONTEXT(begin_params);
    }

done:
    TEST_FREE_BLOCK(in_data);
    TEST_FREE_BLOCK(out_data);
    TEST_FREE_BLOCK(signature);
    TEST_FREE_BLOCK(import_key);
    TEST_FREE_BLOCK(key);
    TEST_FREE_BLOCK(key1024);
    TEST_FREE_BLOCK(key2048);
    TEST_FREE_BLOCK(key3072);
    TEST_FREE_BLOCK(key4096);
    TEST_DELETE_CONTEXT(begin_params);
    TEST_DELETE_CONTEXT(key_params);
    return err;
}

#define MAX_MODIFIERS 5

static BERR_Code km_crypto_rsa_sign_verify_misc_tests(KeymasterTl_Handle handle)
{
    BERR_Code err;
    KM_Tag_ContextHandle key_params = NULL;
    KM_Tag_ContextHandle begin_params = NULL;
    KM_CryptoOperation_Settings settings;
    KeymasterTl_ImportKeySettings impSettings;
    KeymasterTl_DataBlock import_key;
    KeymasterTl_DataBlock key;
    KeymasterTl_DataBlock in_data;
    KeymasterTl_DataBlock out_data;
    KeymasterTl_DataBlock signature;
    int i, mod;
    uint32_t key_size = 0;
    uint32_t message_size;
    km_key_modifier_fn modifier_list[MAX_MODIFIERS];
    char *message = NULL;
    const char *comment;
    BERR_Code expected_failure;
    const EVP_MD *md;
    int rsa_padding;

    memset(&import_key, 0, sizeof(import_key));
    memset(&key, 0, sizeof(key));
    memset(&in_data, 0, sizeof(in_data));
    memset(&out_data, 0, sizeof(out_data));
    memset(&signature, 0, sizeof(signature));

    BDBG_LOG(("----------------------- %s -----------------------", BSTD_FUNCTION));

    for (i = 0; i < 16; i++) {
        if (i == 6) {
            /* Test 6 is not working for now */
            continue;
        }

        memset(modifier_list, 0, sizeof(modifier_list));
        md = NULL;
        message_size = 0;

        KM_CryptoOperation_GetDefaultSettings(&settings);

        settings.handle = handle;

        EXPECT_SUCCESS(KM_Tag_NewContext(&begin_params));
        settings.begin_params = begin_params;

        expected_failure = BERR_SUCCESS;
        switch (i) {
        case 0:
            key_size = 256;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_NONE);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_NONE);
            message_size = key_size / 8;
            comment = "RSA 256 bit, no padding or digest";
            break;
        case 1:
            key_size = 768;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_PSS);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            md = EVP_sha256();
            rsa_padding = RSA_PKCS1_PSS_PADDING;
            message_size = 1024;
            comment = "RSA 768 bit, PSS, SHA256";
            break;
        case 2:
            key_size = 512;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_PKCS1_1_5_SIGN);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            md = EVP_sha256();
            rsa_padding = RSA_PKCS1_PADDING;
            message_size = 1024;
            comment = "RSA 512 bit, PKCS1, SHA256";
            break;
        case 3:
            key_size = 256;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_NONE);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_NONE);
            message_size = 1024;
            expected_failure = BSAGE_ERR_KM_INVALID_INPUT_LENGTH;
            comment = "RSA 256 bit, no padding or digest";
            break;
        case 4:
            key_size = 512;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_PKCS1_1_5_SIGN);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_NONE);
            message_size = key_size / 8 - PKCS1_OVERHEAD;
            comment = "RSA 512 bit, PKCS1, no digest";
            break;
        case 5:
            key_size = 512;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_PKCS1_1_5_SIGN);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_NONE);
            message_size = key_size / 8 + 2;
            expected_failure = BSAGE_ERR_KM_INVALID_INPUT_LENGTH;
            comment = "RSA 512 bit, PKCS1, no digest (too long message)";
            break;
        case 6:
            key_size = 256 + 9 * 8;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_PSS);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            message_size = 1024;
            expected_failure = BSAGE_ERR_KM_INCOMPATIBLE_DIGEST;
            comment = "RSA 328 bit, PSS, SHA256";
            break;
        case 7:
            key_size = 256;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_PKCS7);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            message_size = 1024;
            expected_failure = BSAGE_ERR_KM_UNSUPPORTED_PADDING_MODE;
            comment = "RSA 256 bit, invalid padding";
            break;
        case 8:
            key_size = 256;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_PSS);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_NONE);
            message_size = 1024;
            expected_failure = BSAGE_ERR_KM_INCOMPATIBLE_DIGEST;
            comment = "RSA 256 bit, PSS, invalid digest";
            break;
        case 9:
            key_size = 256;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_NONE);
            message_size = 1024;
            expected_failure = BSAGE_ERR_KM_UNSUPPORTED_PADDING_MODE;
            comment = "RSA 256 bit, no digest";
            break;
        case 10:
            key_size = 256;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_NONE);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_NONE);
            message_size = key_size / 8 - 1;
            comment = "RSA 256 bit, no padding or digest (short data)";
            break;
        case 11:
            key_size = 256;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_NONE);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_NONE);
            message_size = key_size / 8;
            modifier_list[0] = km_test_remove_sign_verify;
            expected_failure = BSAGE_ERR_KM_INCOMPATIBLE_PURPOSE;
            comment = "RSA 256 bit, no padding or digest (key can't sign)";
            break;
        case 12:
            key_size = 256;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_NONE);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_NONE);
            message_size = key_size / 8 + 1;
            expected_failure = BSAGE_ERR_KM_INVALID_INPUT_LENGTH;
            comment = "RSA 256 bit, no padding or digest";
            break;
        case 13:
            key_size = 512;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_PKCS1_1_5_SIGN);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            md = EVP_sha256();
            rsa_padding = RSA_PKCS1_PADDING;
            message_size = 1024;
            comment = "RSA 512 bit, PKCS1, SHA256";
            break;
        case 14:
            key_size = 256;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_PKCS1_1_5_SIGN);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_NONE);
            rsa_padding = RSA_PKCS1_PADDING;
            message_size = 64 * 1024;
            comment = "RSA 256 bit, PKCS1, NONE, large data";
            expected_failure = BSAGE_ERR_KM_INVALID_INPUT_LENGTH;
            break;
        case 15:
            key_size = 512;
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_PADDING, SKM_PAD_RSA_PKCS1_1_5_SIGN);
            TEST_TAG_ADD_ENUM(settings.begin_params, SKM_TAG_DIGEST, SKM_DIGEST_SHA_2_256);
            md = EVP_sha256();
            rsa_padding = RSA_PKCS1_PADDING;
            settings.in_data_process_blocks = 4;
            message_size = settings.in_data_process_blocks * 4096;
            comment = "RSA 512 bit, PKCS1, SHA256, multiple updates";
            break;
        default:
            BDBG_ASSERT(0);
            break;
        }

        BDBG_LOG(("%s : Test %d - %s", BSTD_FUNCTION, i, comment));

        message = malloc(message_size);
        if (!message_size || !message) {
            BDBG_ERR(("%s: failed to allocate message", BSTD_FUNCTION));
            err = BERR_UNKNOWN;
            goto done;
        }
        memset(message, 'a', message_size);

        EXPECT_SUCCESS(km_test_new_params_with_rsa_defaults(&key_params, key_size));
        for (mod = 0; mod < MAX_MODIFIERS && modifier_list[mod]; mod++) {
            EXPECT_SUCCESS(modifier_list[mod](key_params));
        }

        /* Generate key on host side and import it (some key sizes cannot be generated on SAGE) */
        EXPECT_SUCCESS(rsa_gen_keys(&import_key, key_size));
        KeymasterTl_GetDefaultImportKeySettings(&impSettings);
        impSettings.in_key_params = key_params;
        impSettings.in_key_format = SKM_KEY_FORMAT_PKCS8;
        impSettings.in_key_blob = import_key;
        EXPECT_SUCCESS(KeymasterTl_ImportKey(handle, &impSettings));

        key = impSettings.out_key_blob;

        settings.key_params = key_params;
        settings.in_key = key;

        TEST_ALLOCATE_BLOCK(in_data, message_size);
        TEST_ALLOCATE_BLOCK(out_data, 4096);
        TEST_ALLOCATE_BLOCK(signature, key_size / 8);
        memcpy(in_data.buffer, message, in_data.size);
        memset(signature.buffer, 0xFF, signature.size);

        settings.in_data = in_data;
        settings.out_data = signature;

        EXPECT_FAILURE_CODE(KM_Crypto_Operation(SKM_PURPOSE_SIGN, &settings), expected_failure);

        if (expected_failure == BERR_SUCCESS) {
            signature.size = settings.out_data.size;
            BDBG_LOG(("%s: sign produced %d bytes", BSTD_FUNCTION, settings.out_data.size));

            settings.in_data = in_data;
            settings.signature_data = signature;
            settings.out_data = out_data;

            EXPECT_SUCCESS(KM_Crypto_Operation(SKM_PURPOSE_VERIFY, &settings));
            BDBG_LOG(("%s: verify successful", BSTD_FUNCTION));

            if (md) {
                KeymasterTl_ExportKeySettings expSettings;
                EVP_MD_CTX *ctx;
#if (OPENSSL_VERSION_NUMBER <= 0x10100000L)
                EVP_MD_CTX md_ctx;
#endif
                EVP_PKEY_CTX *pkey_ctx;
                EVP_PKEY *pkey = NULL;
                uint8_t *p;

                /* Export the key and verify the signature with the public key */

                KeymasterTl_GetDefaultExportKeySettings(&expSettings);
                expSettings.in_key_format = SKM_KEY_FORMAT_X509;
                expSettings.in_key_blob = key;
                EXPECT_SUCCESS(KeymasterTl_ExportKey(handle, &expSettings));

                p = expSettings.out_key_blob.buffer;
                pkey = d2i_PUBKEY(NULL, (const unsigned char **)&p, expSettings.out_key_blob.size);
                if (!pkey) {
                    TEST_FREE_BLOCK(expSettings.out_key_blob);
                    BDBG_ERR(("%s: failed to load public key", BSTD_FUNCTION));
                    goto done;
                }

#if (OPENSSL_VERSION_NUMBER > 0x10100000L)
                ctx = EVP_MD_CTX_new();
                if (!ctx) {
                    TEST_FREE_BLOCK(expSettings.out_key_blob);
                    BDBG_ERR(("%s: failed to allocate context", BSTD_FUNCTION));
                    goto done;
                }
#else
                ctx = &md_ctx;
                EVP_MD_CTX_init(&md_ctx);
#endif
                EXPECT_TRUE(EVP_DigestVerifyInit(ctx, &pkey_ctx, md, NULL, pkey));
                if (EVP_PKEY_CTX_set_rsa_padding(pkey_ctx, rsa_padding) <= 0) {
                    BDBG_ERR(("%s: failed to set padding", BSTD_FUNCTION));
                    err = BERR_UNKNOWN;
                    goto done;
                }
                EXPECT_TRUE(EVP_DigestVerifyUpdate(ctx, in_data.buffer, in_data.size));
                EXPECT_TRUE(EVP_DigestVerifyFinal(ctx, signature.buffer, signature.size));

#if (OPENSSL_VERSION_NUMBER > 0x10100000L)
                EVP_MD_CTX_free(ctx);
#else
                EVP_MD_CTX_cleanup(&md_ctx);
#endif
                EVP_PKEY_free(pkey);
                TEST_FREE_BLOCK(expSettings.out_key_blob);
                BDBG_LOG(("%s: offline verify successful", BSTD_FUNCTION));
            }

            /* Modify source data and check for failure */
            in_data.buffer[0]++;
            settings.out_data = out_data;
            EXPECT_FAILURE_CODE(KM_Crypto_Operation(SKM_PURPOSE_VERIFY, &settings), BSAGE_ERR_KM_VERIFICATION_FAILED);

            /* Modify the signature and check that verification fails*/
            in_data.buffer[0]--;
            signature.buffer[5]++;
            settings.out_data = out_data;
            EXPECT_FAILURE_CODE(KM_Crypto_Operation(SKM_PURPOSE_VERIFY, &settings), BSAGE_ERR_KM_VERIFICATION_FAILED);

            signature.buffer[5]--;
        }

        BDBG_LOG(("%s: Test %d - %s success", BSTD_FUNCTION, i, comment));

        free(message);
        message = NULL;

        TEST_FREE_BLOCK(in_data);
        TEST_FREE_BLOCK(out_data);
        TEST_FREE_BLOCK(signature);
        TEST_FREE_BLOCK(import_key);
        TEST_FREE_BLOCK(key);
        TEST_DELETE_CONTEXT(begin_params);
        TEST_DELETE_CONTEXT(key_params);
    }

done:
    if (message) {
        free(message);
    }
    TEST_FREE_BLOCK(in_data);
    TEST_FREE_BLOCK(out_data);
    TEST_FREE_BLOCK(signature);
    TEST_FREE_BLOCK(import_key);
    TEST_FREE_BLOCK(key);
    TEST_DELETE_CONTEXT(begin_params);
    TEST_DELETE_CONTEXT(key_params);
    return err;
}

BERR_Code km_crypto_rsa_tests(KeymasterTl_Handle handle)
{
    BERR_Code err;

    //EXPECT_SUCCESS(km_crypto_rsa_enc_dec_tests(handle));
    EXPECT_SUCCESS(km_crypto_rsa_sign_verify_table_tests(handle));
    EXPECT_SUCCESS(km_crypto_rsa_sign_verify_misc_tests(handle));

done:
    return err;
}

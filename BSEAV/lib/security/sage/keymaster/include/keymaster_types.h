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

#ifndef KEYMASTER_TYPES_H__
#define KEYMASTER_TYPES_H__


#ifdef __cplusplus
extern "C"
{
#endif


#define KM_TAG_MASK_TYPE(tag)       ((tag) & (0xf << 28))
#define KM_TAG_MASK_NUMBER(tag)     ((tag) & 0x0fffffff)


/**
 * Authorization tags each have an associated type.  This enumeration facilitates tagging each with
 * a type, by using the high four bits (of an implied 32-bit unsigned enum value) to specify up to
 * 16 data types.  These values are ORed with tag IDs to generate the final tag ID values.
 */
typedef enum {
    KM_INVALID = 0 << 28, /* Invalid type, used to designate a tag as uninitialized */
    KM_ENUM = 1 << 28,
    KM_ENUM_REP = 2 << 28, /* Repeatable enumeration value. */
    KM_UINT = 3 << 28,
    KM_UINT_REP = 4 << 28, /* Repeatable integer value */
    KM_ULONG = 5 << 28,
    KM_DATE = 6 << 28,
    KM_BOOL = 7 << 28,
    KM_BIGNUM = 8 << 28,
    KM_BYTES = 9 << 28,
    KM_ULONG_REP = 10 << 28, /* Repeatable long value */
} km_tag_type_t;

typedef enum {
    KM_TAG_INVALID = KM_INVALID | 0,

    /*
     * Tags that must be semantically enforced by hardware and software implementations.
     */

    /* Crypto parameters */
    KM_TAG_PURPOSE = KM_ENUM_REP | 1,    /* km_purpose_t. */
    KM_TAG_ALGORITHM = KM_ENUM | 2,      /* km_algorithm_t. */
    KM_TAG_KEY_SIZE = KM_UINT | 3,       /* Key size in bits. */
    KM_TAG_BLOCK_MODE = KM_ENUM_REP | 4, /* km_block_mode_t. */
    KM_TAG_DIGEST = KM_ENUM_REP | 5,     /* km_digest_t. */
    KM_TAG_PADDING = KM_ENUM_REP | 6,    /* km_padding_t. */
    KM_TAG_CALLER_NONCE = KM_BOOL | 7,   /* Allow caller to specify nonce or IV. */
    KM_TAG_MIN_MAC_LENGTH = KM_UINT | 8, /* Minimum length of MAC or AEAD authentication tag in
                                          * bits. */
    KM_TAG_KDF = KM_ENUM_REP | 9,        /* km_kdf_t (keymaster2) */
    KM_TAG_EC_CURVE = KM_ENUM | 10,      /* km_ec_curve_t (keymaster2) */

    /* Algorithm-specific. */
    KM_TAG_RSA_PUBLIC_EXPONENT = KM_ULONG | 200,
    KM_TAG_ECIES_SINGLE_HASH_MODE = KM_BOOL | 201, /* Whether the ephemeral public key is fed into
                                                    * the KDF */
    KM_TAG_INCLUDE_UNIQUE_ID = KM_BOOL | 202,      /* If true, attestation certificates for this key
                                                    * will contain an application-scoped and
                                                    * time-bounded device-unique ID. (keymaster2) */

    /* Other hardware-enforced. */
    KM_TAG_BLOB_USAGE_REQUIREMENTS = KM_ENUM | 301, /* km_key_blob_usage_requirements_t */
    KM_TAG_BOOTLOADER_ONLY = KM_BOOL | 302,         /* Usable only by bootloader */

    /*
     * Tags that should be semantically enforced by hardware if possible and will otherwise be
     * enforced by software (keystore).
     */

    /* Key validity period */
    KM_TAG_ACTIVE_DATETIME = KM_DATE | 400,             /* Start of validity */
    KM_TAG_ORIGINATION_EXPIRE_DATETIME = KM_DATE | 401, /* Date when new "messages" should no
                                                           longer be created. */
    KM_TAG_USAGE_EXPIRE_DATETIME = KM_DATE | 402,       /* Date when existing "messages" should no
                                                           longer be trusted. */
    KM_TAG_MIN_SECONDS_BETWEEN_OPS = KM_UINT | 403,     /* Minimum elapsed time between
                                                           cryptographic operations with the key. */
    KM_TAG_MAX_USES_PER_BOOT = KM_UINT | 404,           /* Number of times the key can be used per
                                                           boot. */

    /* User authentication */
    KM_TAG_ALL_USERS = KM_BOOL | 500,           /* Reserved for future use -- ignore */
    KM_TAG_USER_ID = KM_UINT | 501,             /* Reserved for future use -- ignore */
    KM_TAG_USER_SECURE_ID = KM_ULONG_REP | 502, /* Secure ID of authorized user or authenticator(s).
                                                   Disallowed if KM_TAG_ALL_USERS or
                                                   KM_TAG_NO_AUTH_REQUIRED is present. */
    KM_TAG_NO_AUTH_REQUIRED = KM_BOOL | 503,    /* If key is usable without authentication. */
    KM_TAG_USER_AUTH_TYPE = KM_ENUM | 504,      /* Bitmask of authenticator types allowed when
                                                 * KM_TAG_USER_SECURE_ID contains a secure user ID,
                                                 * rather than a secure authenticator ID.  Defined in
                                                 * km_hw_authenticator_type_t in hw_auth_token.h. */
    KM_TAG_AUTH_TIMEOUT = KM_UINT | 505,        /* Required freshness of user authentication for
                                                   private/secret key operations, in seconds.
                                                   Public key operations require no authentication.
                                                   If absent, authentication is required for every
                                                   use.  Authentication state is lost when the
                                                   device is powered off. */
    KM_TAG_ALLOW_WHILE_ON_BODY = KM_BOOL | 506, /* Allow key to be used after authentication timeout
                                                 * if device is still on-body (requires secure
                                                 * on-body sensor. */

    /* Application access control */
    KM_TAG_ALL_APPLICATIONS = KM_BOOL | 600, /* Specified to indicate key is usable by all
                                              * applications. */
    KM_TAG_APPLICATION_ID = KM_BYTES | 601,  /* Byte string identifying the authorized
                                              * application. */
    KM_TAG_EXPORTABLE = KM_BOOL | 602,       /* If true, private/secret key can be exported, but
                                              * only if all access control requirements for use are
                                              * met. (keymaster2) */

    /*
     * Semantically unenforceable tags, either because they have no specific meaning or because
     * they're informational only.
     */
    KM_TAG_APPLICATION_DATA = KM_BYTES | 700,      /* Data provided by authorized application. */
    KM_TAG_CREATION_DATETIME = KM_DATE | 701,      /* Key creation time */
    KM_TAG_ORIGIN = KM_ENUM | 702,                 /* km_key_origin_t. */
    KM_TAG_ROLLBACK_RESISTANT = KM_BOOL | 703,     /* Whether key is rollback-resistant. */
    KM_TAG_ROOT_OF_TRUST = KM_BYTES | 704,         /* Root of trust ID. */
    KM_TAG_OS_VERSION = KM_UINT | 705,             /* Version of system (keymaster2) */
    KM_TAG_OS_PATCHLEVEL = KM_UINT | 706,          /* Patch level of system (keymaster2) */
    KM_TAG_UNIQUE_ID = KM_BYTES | 707,             /* Used to provide unique ID in attestation */
    KM_TAG_ATTESTATION_CHALLENGE = KM_BYTES | 708, /* Used to provide challenge in attestation */

    /* Tags used only to provide data to or receive data from operations */
    KM_TAG_ASSOCIATED_DATA = KM_BYTES | 1000, /* Used to provide associated data for AEAD modes. */
    KM_TAG_NONCE = KM_BYTES | 1001,           /* Nonce or Initialization Vector */
    KM_TAG_AUTH_TOKEN = KM_BYTES | 1002,      /* Authentication token that proves secure user
                                                 authentication has been performed.  Structure
                                                 defined in km_hw_auth_token_t in hw_auth_token.h. */
    KM_TAG_MAC_LENGTH = KM_UINT | 1003,       /* MAC or AEAD authentication tag length in
                                               * bits. */

    KM_TAG_RESET_SINCE_ID_ROTATION = KM_BOOL | 1004, /* Whether the device has beeen factory reset
                                                        since the last unique ID rotation.  Used for
                                                        key attestation. */
} km_tag_t;

/**
 * Algorithms that may be provided by keymaster implementations.  Those that must be provided by all
 * implementations are tagged as "required".
 */
typedef enum {
    /* Asymmetric algorithms. */
    KM_ALGORITHM_RSA = 1,
    // KM_ALGORITHM_DSA = 2, -- Removed, do not re-use value 2.
    KM_ALGORITHM_EC = 3,

    /* Block ciphers algorithms */
    KM_ALGORITHM_AES = 32,

    /* MAC algorithms */
    KM_ALGORITHM_HMAC = 128,
} km_algorithm_t;

/**
 * Symmetric block cipher modes provided by keymaster implementations.
 */
typedef enum {
    /* Unauthenticated modes, usable only for encryption/decryption and not generally recommended
     * except for compatibility with existing other protocols. */
    KM_MODE_ECB = 1,
    KM_MODE_CBC = 2,
    KM_MODE_CTR = 3,

    /* Authenticated modes, usable for encryption/decryption and signing/verification.  Recommended
     * over unauthenticated modes for all purposes. */
    KM_MODE_GCM = 32,
} km_block_mode_t;

/**
 * Padding modes that may be applied to plaintext for encryption operations.  This list includes
 * padding modes for both symmetric and asymmetric algorithms.  Note that implementations should not
 * provide all possible combinations of algorithm and padding, only the
 * cryptographically-appropriate pairs.
 */
typedef enum {
    KM_PAD_NONE = 1, /* deprecated */
    KM_PAD_RSA_OAEP = 2,
    KM_PAD_RSA_PSS = 3,
    KM_PAD_RSA_PKCS1_1_5_ENCRYPT = 4,
    KM_PAD_RSA_PKCS1_1_5_SIGN = 5,
    KM_PAD_PKCS7 = 64,
} km_padding_t;

/**
 * Digests provided by keymaster implementations.
 */
typedef enum {
    KM_DIGEST_NONE = 0,
    KM_DIGEST_MD5 = 1, /* Optional, may not be implemented in hardware, will be handled in software
                        * if needed. */
    KM_DIGEST_SHA1 = 2,
    KM_DIGEST_SHA_2_224 = 3,
    KM_DIGEST_SHA_2_256 = 4,
    KM_DIGEST_SHA_2_384 = 5,
    KM_DIGEST_SHA_2_512 = 6,
} km_digest_t;

/*
 * Key derivation functions, mostly used in ECIES.
 */
typedef enum {
    /* Do not apply a key derivation function; use the raw agreed key */
    KM_KDF_NONE = 0,
    /* HKDF defined in RFC 5869 with SHA256 */
    KM_KDF_RFC5869_SHA256 = 1,
    /* KDF1 defined in ISO 18033-2 with SHA1 */
    KM_KDF_ISO18033_2_KDF1_SHA1 = 2,
    /* KDF1 defined in ISO 18033-2 with SHA256 */
    KM_KDF_ISO18033_2_KDF1_SHA256 = 3,
    /* KDF2 defined in ISO 18033-2 with SHA1 */
    KM_KDF_ISO18033_2_KDF2_SHA1 = 4,
    /* KDF2 defined in ISO 18033-2 with SHA256 */
    KM_KDF_ISO18033_2_KDF2_SHA256 = 5,
} km_kdf_t;

/**
 * Supported EC curves, used in ECDSA/ECIES.
 */
typedef enum {
    KM_EC_CURVE_P_224 = 0,
    KM_EC_CURVE_P_256 = 1,
    KM_EC_CURVE_P_384 = 2,
    KM_EC_CURVE_P_521 = 3,
} km_ec_curve_t;

/**
 * The origin of a key (or pair), i.e. where it was generated.  Note that KM_TAG_ORIGIN can be found
 * in either the hardware-enforced or software-enforced list for a key, indicating whether the key
 * is hardware or software-based.  Specifically, a key with KM_ORIGIN_GENERATED in the
 * hardware-enforced list is guaranteed never to have existed outide the secure hardware.
 */
typedef enum {
    KM_ORIGIN_GENERATED = 0, /* Generated in keymaster.  Should not exist outside the TEE. */
    KM_ORIGIN_DERIVED = 1,   /* Derived inside keymaster.  Likely exists off-device. */
    KM_ORIGIN_IMPORTED = 2,  /* Imported into keymaster.  Existed as cleartext in Android. */
    KM_ORIGIN_UNKNOWN = 3,   /* Keymaster did not record origin.  This value can only be seen on
                              * keys in a keymaster0 implementation.  The keymaster0 adapter uses
                              * this value to document the fact that it is unkown whether the key
                              * was generated inside or imported into keymaster. */
} km_key_origin_t;

/**
 * Usability requirements of key blobs.  This defines what system functionality must be available
 * for the key to function.  For example, key "blobs" which are actually handles referencing
 * encrypted key material stored in the file system cannot be used until the file system is
 * available, and should have BLOB_REQUIRES_FILE_SYSTEM.  Other requirements entries will be added
 * as needed for implementations.
 */
typedef enum {
    KM_BLOB_STANDALONE = 0,
    KM_BLOB_REQUIRES_FILE_SYSTEM = 1,
} km_key_blob_usage_requirements_t;

/**
 * Possible purposes of a key (or pair).
 */
typedef enum {
    KM_PURPOSE_ENCRYPT = 0,    /* Usable with RSA, EC and AES keys. */
    KM_PURPOSE_DECRYPT = 1,    /* Usable with RSA, EC and AES keys. */
    KM_PURPOSE_SIGN = 2,       /* Usable with RSA, EC and HMAC keys. */
    KM_PURPOSE_VERIFY = 3,     /* Usable with RSA, EC and HMAC keys. */
    KM_PURPOSE_DERIVE_KEY = 4, /* Usable with EC keys. */
} km_purpose_t;

typedef enum {
    KM_VERIFIED_BOOT_VERIFIED = 0,    /* Full chain of trust extending from the bootloader to
                                       * verified partitions, including the bootloader, boot
                                       * partition, and all verified partitions*/
    KM_VERIFIED_BOOT_SELF_SIGNED = 1, /* The boot partition has been verified using the embedded
                                       * certificate, and the signature is valid. The bootloader
                                       * displays a warning and the fingerprint of the public
                                       * key before allowing the boot process to continue.*/
    KM_VERIFIED_BOOT_UNVERIFIED = 2,  /* The device may be freely modified. Device integrity is left
                                       * to the user to verify out-of-band. The bootloader
                                       * displays a warning to the user before allowing the boot
                                       * process to continue */
    KM_VERIFIED_BOOT_FAILED = 3,      /* The device failed verification. The bootloader displays a
                                       * warning and stops the boot process, so no keymaster
                                       * implementation should ever actually return this value,
                                       * since it should not run.  Included here only for
                                       * completeness. */
} km_verified_boot_t;

typedef enum {
    KM_SECURITY_LEVEL_SOFTWARE = 0,
    KM_SECURITY_LEVEL_TRUSTED_ENVIRONMENT = 1,
} km_security_level_t;

/**
 * Formats for key import and export.
 */
typedef enum {
    KM_KEY_FORMAT_X509 = 0,  /* for public key export */
    KM_KEY_FORMAT_PKCS8 = 1, /* for asymmetric key pair import */
    KM_KEY_FORMAT_RAW = 3,   /* for symmetric key import and export*/
} km_key_format_t;


#define KM_HW_AUTH_TOKEN_VERSION   0

/*
 * Additional auth token can be required to perform operations. This is
 * related to KM_TAG_AUTH_TOKEN which must be passed as an operation
 * parameter and the key must include a KM_TAG_USER_AUTH_TYPE in it.
 * The comment says that is related to hw_authenticator_type_t,
 * but I cannot find the definition for this.
 */
typedef enum {
    HW_AUTH_NONE = 0,
    HW_AUTH_PASSWORD = 1 << 0,
    HW_AUTH_FINGERPRINT = 1 << 1,
    // Additional entries should be powers of 2.
    HW_AUTH_ANY = UINT16_MAX,
} km_hw_authenticator_type_t;

typedef struct km_hw_auth_token_t {
    uint8_t version;              // Current version is 0
    uint64_t challenge;
    uint64_t user_id;             // secure user ID, not Android user ID
    uint64_t authenticator_id;    // secure authenticator ID
    uint32_t authenticator_type;  // km_hw_authenticator_type_t, in network order
    uint64_t timestamp;           // Problematic - no shared clock with SAGE?
    uint8_t hmac[32];
} km_hw_auth_token_t;

typedef uint64_t km_operation_handle_t;

/*
 * Variable size structure to store data for KM_BIGNUM and KM_BYTES. data[]
 * must be rounded up to be 4 bytes aligned, but data_length reflects the
 * size of useful data stored in data[].
 */
typedef struct {
    uint32_t data_length;
    uint8_t data[];
} km_blob_t;

/* Variable size structure to store a tag/value pair (variable part is blob) */
typedef struct {
    km_tag_t tag;
    union {
        uint32_t enumerated;    /* KM_ENUM and KM_ENUM_REP */
        bool boolean;           /* KM_BOOL */
        uint32_t integer;       /* KM_INT and KM_INT_REP */
        uint64_t long_integer;  /* KM_LONG */
        uint64_t date_time;     /* KM_DATE */
        uint32_t blob_data_length; /* KM_BIGNUM and KM_BYTES*/
    } value;
    uint8_t blob_data[];
} km_tag_value_t;


#define KM_BLOB_MAGIC               0x4B45594D      /* KEYM */

/* Right now define a max for KM_BYTES and KM_BIGNUM */
#define KM_TAG_VALUE_BLOB_MAX_SIZE  (128)

/* Block to receive km_tag_value_set_t must be this size */
#define KM_TAG_VALUE_BLOCK_SIZE     (4096)

/* Used in km_secure_nonce_t */
#define KM_NONCE_HMAC_KEY_SIZE      8

#define KM_RSA_MAX_KEY_SIZE         3072    /* generate is limited to 1024 */

#define KM_HMAX_MAX_KEY_SIZE        512

#define KM_HMAC_MIN_MAC_LENGTH_BITS 64

#define KM_SHA256_DIGEST_SIZE       (32)

/* HMAC for a KEY blob */
typedef uint8_t km_key_blob_hmac_t[KM_SHA256_DIGEST_SIZE];

/* Variable size structure to store an array of tag/value pairs */
typedef struct {
    uint32_t num;
    uint32_t size;
    uint8_t params[];
} km_tag_value_set_t;

typedef struct {
    uint64_t challenge;
    uint64_t counter;
    uint32_t hmac_key[KM_NONCE_HMAC_KEY_SIZE];
} km_secure_nonce_t;

/* Variable size structure to store a keymaster key blob */
typedef struct {
    uint32_t    magic;
    uint16_t    header_version;
    uint16_t    keymaster_version;
    uint32_t    blob_length;
    uint8_t     iv[16];
    uint8_t     blob[];             /* The blob and SHA256 HMAC follow */
} km_key_blob_t;

/* For key attestation cert chain */
typedef struct {
	uint32_t length;
	uint32_t offset;
} km_cert_t;

#define KM_CERTIFICATES_NUM_MAX 32

typedef struct {
	uint32_t num;
	km_cert_t certificates[KM_CERTIFICATES_NUM_MAX];
} km_cert_chain_t;


#ifdef __cplusplus
}
#endif


#endif  /* KEYMASTER_TYPES_H__ */

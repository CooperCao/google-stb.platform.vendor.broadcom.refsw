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

#ifndef KEYMASTER_TYPES_H__
#define KEYMASTER_TYPES_H__


#ifdef __cplusplus
extern "C"
{
#endif

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
    uint8_t *drm_binfile_buffer;
    uint32_t drm_binfile_size;
    uint32_t version;
} KeymasterTl_InitSettings;

#define SKM_TAG_TYPE_SHIFT           28
#define SKM_TAG_MASK_TYPE(tag)       ((tag) & (0xf << SKM_TAG_TYPE_SHIFT))
#define SKM_TAG_MASK_NUMBER(tag)     ((tag) & 0x0fffffff)


/**
 * Authorization tags each have an associated type.  This enumeration facilitates tagging each with
 * a type, by using the high four bits (of an implied 32-bit unsigned enum value) to specify up to
 * 16 data types.  These values are ORed with tag IDs to generate the final tag ID values.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
typedef enum {
    SKM_INVALID = 0 << SKM_TAG_TYPE_SHIFT, /* Invalid type, used to designate a tag as uninitialized */
    SKM_ENUM = 1 << SKM_TAG_TYPE_SHIFT,
    SKM_ENUM_REP = 2 << SKM_TAG_TYPE_SHIFT, /* Repeatable enumeration value. */
    SKM_UINT = 3 << SKM_TAG_TYPE_SHIFT,
    SKM_UINT_REP = 4 << SKM_TAG_TYPE_SHIFT, /* Repeatable integer value */
    SKM_ULONG = 5 << SKM_TAG_TYPE_SHIFT,
    SKM_DATE = 6 << SKM_TAG_TYPE_SHIFT,
    SKM_BOOL = 7 << SKM_TAG_TYPE_SHIFT,
    SKM_BIGNUM = 8 << SKM_TAG_TYPE_SHIFT,
    SKM_BYTES = 9 << SKM_TAG_TYPE_SHIFT,
    SKM_ULONG_REP = 10 << SKM_TAG_TYPE_SHIFT, /* Repeatable long value */
} km_tag_type_t;
#pragma GCC diagnostic pop

typedef enum {
    SKM_TAG_INVALID = SKM_INVALID | 0,

    /*
     * Tags that must be semantically enforced by hardware and software implementations.
     */

    /* Crypto parameters */
    SKM_TAG_PURPOSE = SKM_ENUM_REP | 1,    /* km_purpose_t. */
    SKM_TAG_ALGORITHM = SKM_ENUM | 2,      /* km_algorithm_t. */
    SKM_TAG_KEY_SIZE = SKM_UINT | 3,       /* Key size in bits. */
    SKM_TAG_BLOCK_MODE = SKM_ENUM_REP | 4, /* km_block_mode_t. */
    SKM_TAG_DIGEST = SKM_ENUM_REP | 5,     /* km_digest_t. */
    SKM_TAG_PADDING = SKM_ENUM_REP | 6,    /* km_padding_t. */
    SKM_TAG_CALLER_NONCE = SKM_BOOL | 7,   /* Allow caller to specify nonce or IV. */
    SKM_TAG_MIN_MAC_LENGTH = SKM_UINT | 8, /* Minimum length of MAC or AEAD authentication tag in
                                            * bits. */
    SKM_TAG_KDF = SKM_ENUM_REP | 9,        /* km_kdf_t (keymaster2) */
    SKM_TAG_EC_CURVE = SKM_ENUM | 10,      /* km_ec_curve_t (keymaster2) */

    /* Algorithm-specific. */
    SKM_TAG_RSA_PUBLIC_EXPONENT = SKM_ULONG | 200,
    SKM_TAG_ECIES_SINGLE_HASH_MODE = SKM_BOOL | 201, /* Whether the ephemeral public key is fed into
                                                      * the KDF */
    SKM_TAG_INCLUDE_UNIQUE_ID = SKM_BOOL | 202,      /* If true, attestation certificates for this key
                                                      * will contain an application-scoped and
                                                      * time-bounded device-unique ID. (keymaster2) */

    /* Other hardware-enforced. */
    SKM_TAG_BLOB_USAGE_REQUIREMENTS = SKM_ENUM | 301, /* km_key_blob_usage_requirements_t */
    SKM_TAG_BOOTLOADER_ONLY = SKM_BOOL | 302,         /* Usable only by bootloader */

    /*
     * Tags that should be semantically enforced by hardware if possible and will otherwise be
     * enforced by software (keystore).
     */

    /* Key validity period */
    SKM_TAG_ACTIVE_DATETIME = SKM_DATE | 400,             /* Start of validity */
    SKM_TAG_ORIGINATION_EXPIRE_DATETIME = SKM_DATE | 401, /* Date when new "messages" should no
                                                             longer be created. */
    SKM_TAG_USAGE_EXPIRE_DATETIME = SKM_DATE | 402,       /* Date when existing "messages" should no
                                                             longer be trusted. */
    SKM_TAG_MIN_SECONDS_BETWEEN_OPS = SKM_UINT | 403,     /* Minimum elapsed time between
                                                             cryptographic operations with the key. */
    SKM_TAG_MAX_USES_PER_BOOT = SKM_UINT | 404,           /* Number of times the key can be used per
                                                             boot. */

    /* User authentication */
    SKM_TAG_ALL_USERS = SKM_BOOL | 500,           /* Reserved for future use -- ignore */
    SKM_TAG_USER_ID = SKM_UINT | 501,             /* Reserved for future use -- ignore */
    SKM_TAG_USER_SECURE_ID = SKM_ULONG_REP | 502, /* Secure ID of authorized user or authenticator(s).
                                                     Disallowed if KM_TAG_ALL_USERS or
                                                     KM_TAG_NO_AUTH_REQUIRED is present. */
    SKM_TAG_NO_AUTH_REQUIRED = SKM_BOOL | 503,    /* If key is usable without authentication. */
    SKM_TAG_USER_AUTH_TYPE = SKM_ENUM | 504,      /* Bitmask of authenticator types allowed when
                                                   * KM_TAG_USER_SECURE_ID contains a secure user ID,
                                                   * rather than a secure authenticator ID.  Defined in
                                                   * km_hw_authenticator_type_t in hw_auth_token.h. */
    SKM_TAG_AUTH_TIMEOUT = SKM_UINT | 505,        /* Required freshness of user authentication for
                                                     private/secret key operations, in seconds.
                                                     Public key operations require no authentication.
                                                     If absent, authentication is required for every
                                                     use.  Authentication state is lost when the
                                                     device is powered off. */
    SKM_TAG_ALLOW_WHILE_ON_BODY = SKM_BOOL | 506, /* Allow key to be used after authentication timeout
                                                   * if device is still on-body (requires secure
                                                   * on-body sensor. */
    SKM_TAG_TRUSTED_USER_PRESENCE_REQUIRED = SKM_BOOL | 507,
    SKM_TAG_TRUSTED_CONFIRMATION_REQUIRED = SKM_BOOL | 508,
    SKM_TAG_UNLOCKED_DEVICE_REQUIRED = SKM_BOOL | 509, /* Specifies that the key may only be used when
                                                        * the device is unlocked. Must be software
                                                        * enforced. (keymaster4)
                                                        */

    /* Application access control */
    SKM_TAG_ALL_APPLICATIONS = SKM_BOOL | 600, /* Specified to indicate key is usable by all
                                                * applications. */
    SKM_TAG_APPLICATION_ID = SKM_BYTES | 601,  /* Byte string identifying the authorized
                                                * application. */
    SKM_TAG_EXPORTABLE = SKM_BOOL | 602,       /* If true, private/secret key can be exported, but
                                                * only if all access control requirements for use are
                                                * met. (keymaster2) */

    /*
     * Semantically unenforceable tags, either because they have no specific meaning or because
     * they're informational only.
     */
    SKM_TAG_APPLICATION_DATA = SKM_BYTES | 700,      /* Data provided by authorized application. */
    SKM_TAG_CREATION_DATETIME = SKM_DATE | 701,      /* Key creation time */
    SKM_TAG_ORIGIN = SKM_ENUM | 702,                 /* km_key_origin_t. */
    SKM_TAG_ROLLBACK_RESISTANT = SKM_BOOL | 703,     /* Whether key is rollback-resistant. */
    SKM_TAG_ROOT_OF_TRUST = SKM_BYTES | 704,         /* Root of trust ID. */
    SKM_TAG_OS_VERSION = SKM_UINT | 705,             /* Version of system (keymaster2) */
    SKM_TAG_OS_PATCHLEVEL = SKM_UINT | 706,          /* Patch level of system (keymaster2) */
    SKM_TAG_UNIQUE_ID = SKM_BYTES | 707,             /* Used to provide unique ID in attestation */
    SKM_TAG_ATTESTATION_CHALLENGE = SKM_BYTES | 708, /* Used to provide challenge in attestation */
    SKM_TAG_ATTESTATION_APPLICATION_ID = SKM_BYTES | 709, /* Used to identify the set of possible
                                                           * applications of which one has initiated
                                                           * a key attestation */
    SKM_TAG_ATTESTATION_ID_BRAND = SKM_BYTES | 710,  /* Used to provide the device's brand name to be
                                                        included in attestation */
    SKM_TAG_ATTESTATION_ID_DEVICE = SKM_BYTES | 711, /* Used to provide the device's device name to be
                                                        included in attestation */
    SKM_TAG_ATTESTATION_ID_PRODUCT = SKM_BYTES | 712, /* Used to provide the device's product name to
                                                         be included in attestation */
    SKM_TAG_ATTESTATION_ID_SERIAL = SKM_BYTES | 713, /* Used to provide the device's serial number to
                                                        be included in attestation */
    SKM_TAG_ATTESTATION_ID_IMEI = SKM_BYTES | 714,   /* Used to provide the device's IMEI to be
                                                        included in attestation */
    SKM_TAG_ATTESTATION_ID_MEID = SKM_BYTES | 715,   /* Used to provide the device's MEID to be
                                                        included in attestation */
    SKM_TAG_ATTESTATION_ID_MANUFACTURER = SKM_BYTES | 716, /* Used to provide the device's
                                                              manufacturer name to be included in
                                                              attestation */
    SKM_TAG_ATTESTATION_ID_MODEL = SKM_BYTES | 717,  /* Used to provide the device's model name to be
                                                        included in attestation */
    SKM_TAG_VENDOR_PATCHLEVEL = SKM_UINT | 718,      /* Used to store the vendor patch level, set during
                                                        configure - cannot be passed in as parameter */
    SKM_TAG_BOOT_PATCHLEVEL = SKM_UINT | 719,        /* Used to store the boot patch level - not used */

    /* Tags used only to provide data to or receive data from operations */
    SKM_TAG_ASSOCIATED_DATA = SKM_BYTES | 1000, /* Used to provide associated data for AEAD modes. */
    SKM_TAG_NONCE = SKM_BYTES | 1001,           /* Nonce or Initialization Vector */
    SKM_TAG_AUTH_TOKEN = SKM_BYTES | 1002,      /* Authentication token that proves secure user
                                                   authentication has been performed.  Structure
                                                   defined in km_hw_auth_token_t in hw_auth_token.h. */
    SKM_TAG_MAC_LENGTH = SKM_UINT | 1003,       /* MAC or AEAD authentication tag length in
                                                 * bits. */

    SKM_TAG_RESET_SINCE_ID_ROTATION = SKM_BOOL | 1004, /* Whether the device has beeen factory reset
                                                          since the last unique ID rotation.  Used for
                                                          key attestation. */
    SKM_TAG_CONFIRMATION_TOKEN = SKM_BYTES | 1005,     /* Used to deliver a cryptographic token proving that
                                                          the user confirmed a signing request. The content
                                                          is a full HMAC-SHA256 value. */
} km_tag_t;

/**
 * Algorithms that may be provided by keymaster implementations.  Those that must be provided by all
 * implementations are tagged as "required".
 */
typedef enum {
    /* Asymmetric algorithms. */
    SKM_ALGORITHM_RSA = 1,
    // KM_ALGORITHM_DSA = 2, -- Removed, do not re-use value 2.
    SKM_ALGORITHM_EC = 3,

    /* Block ciphers algorithms */
    SKM_ALGORITHM_AES = 32,
    SKM_ALGORITHM_TRIPLE_DES = 33,

    /* MAC algorithms */
    SKM_ALGORITHM_HMAC = 128,
} km_algorithm_t;

/**
 * Symmetric block cipher modes provided by keymaster implementations.
 */
typedef enum {
    /* Unauthenticated modes, usable only for encryption/decryption and not generally recommended
     * except for compatibility with existing other protocols. */
    SKM_MODE_ECB = 1,
    SKM_MODE_CBC = 2,
    SKM_MODE_CTR = 3,

    /* Authenticated modes, usable for encryption/decryption and signing/verification.  Recommended
     * over unauthenticated modes for all purposes. */
    SKM_MODE_GCM = 32,
} km_block_mode_t;

/**
 * Padding modes that may be applied to plaintext for encryption operations.  This list includes
 * padding modes for both symmetric and asymmetric algorithms.  Note that implementations should not
 * provide all possible combinations of algorithm and padding, only the
 * cryptographically-appropriate pairs.
 */
typedef enum {
    SKM_PAD_NONE = 1, /* deprecated */
    SKM_PAD_RSA_OAEP = 2,
    SKM_PAD_RSA_PSS = 3,
    SKM_PAD_RSA_PKCS1_1_5_ENCRYPT = 4,
    SKM_PAD_RSA_PKCS1_1_5_SIGN = 5,
    SKM_PAD_PKCS7 = 64,
} km_padding_t;

/**
 * Digests provided by keymaster implementations.
 */
typedef enum {
    SKM_DIGEST_NONE = 0,
    SKM_DIGEST_MD5 = 1, /* Optional, may not be implemented in hardware, will be handled in software
                         * if needed. */
    SKM_DIGEST_SHA1 = 2,
    SKM_DIGEST_SHA_2_224 = 3,
    SKM_DIGEST_SHA_2_256 = 4,
    SKM_DIGEST_SHA_2_384 = 5,
    SKM_DIGEST_SHA_2_512 = 6,
} km_digest_t;

/*
 * Key derivation functions, mostly used in ECIES.
 */
typedef enum {
    /* Do not apply a key derivation function; use the raw agreed key */
    SKM_KDF_NONE = 0,
    /* HKDF defined in RFC 5869 with SHA256 */
    SKM_KDF_RFC5869_SHA256 = 1,
    /* KDF1 defined in ISO 18033-2 with SHA1 */
    SKM_KDF_ISO18033_2_KDF1_SHA1 = 2,
    /* KDF1 defined in ISO 18033-2 with SHA256 */
    SKM_KDF_ISO18033_2_KDF1_SHA256 = 3,
    /* KDF2 defined in ISO 18033-2 with SHA1 */
    SKM_KDF_ISO18033_2_KDF2_SHA1 = 4,
    /* KDF2 defined in ISO 18033-2 with SHA256 */
    SKM_KDF_ISO18033_2_KDF2_SHA256 = 5,
} km_kdf_t;

/**
 * Supported EC curves, used in ECDSA/ECIES.
 */
typedef enum {
    SKM_EC_CURVE_P_224 = 0,
    SKM_EC_CURVE_P_256 = 1,
    SKM_EC_CURVE_P_384 = 2,
    SKM_EC_CURVE_P_521 = 3,
} km_ec_curve_t;

/**
 * The origin of a key (or pair), i.e. where it was generated.  Note that KM_TAG_ORIGIN can be found
 * in either the hardware-enforced or software-enforced list for a key, indicating whether the key
 * is hardware or software-based.  Specifically, a key with KM_ORIGIN_GENERATED in the
 * hardware-enforced list is guaranteed never to have existed outide the secure hardware.
 */
typedef enum {
    SKM_ORIGIN_GENERATED = 0, /* Generated in keymaster.  Should not exist outside the TEE. */
    SKM_ORIGIN_DERIVED = 1,   /* Derived inside keymaster.  Likely exists off-device. */
    SKM_ORIGIN_IMPORTED = 2,  /* Imported into keymaster.  Existed as cleartext in Android. */
    SKM_ORIGIN_UNKNOWN = 3,   /* Keymaster did not record origin.  This value can only be seen on
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
    SKM_BLOB_STANDALONE = 0,
    SKM_BLOB_REQUIRES_FILE_SYSTEM = 1,
} km_key_blob_usage_requirements_t;

/**
 * Possible purposes of a key (or pair).
 */
typedef enum {
    SKM_PURPOSE_ENCRYPT = 0,    /* Usable with RSA, EC and AES keys. */
    SKM_PURPOSE_DECRYPT = 1,    /* Usable with RSA, EC and AES keys. */
    SKM_PURPOSE_SIGN = 2,       /* Usable with RSA, EC and HMAC keys. */
    SKM_PURPOSE_VERIFY = 3,     /* Usable with RSA, EC and HMAC keys. */
    SKM_PURPOSE_DERIVE_KEY = 4, /* Usable with EC keys. */
    SKM_PURPOSE_WRAP = 5,       /* Usable with RSA keys. */
} km_purpose_t;

typedef enum {
    SKM_VERIFIED_BOOT_VERIFIED = 0,    /* Full chain of trust extending from the bootloader to
                                        * verified partitions, including the bootloader, boot
                                        * partition, and all verified partitions*/
    SKM_VERIFIED_BOOT_SELF_SIGNED = 1, /* The boot partition has been verified using the embedded
                                        * certificate, and the signature is valid. The bootloader
                                        * displays a warning and the fingerprint of the public
                                        * key before allowing the boot process to continue.*/
    SKM_VERIFIED_BOOT_UNVERIFIED = 2,  /* The device may be freely modified. Device integrity is left
                                        * to the user to verify out-of-band. The bootloader
                                        * displays a warning to the user before allowing the boot
                                        * process to continue */
    SKM_VERIFIED_BOOT_FAILED = 3,      /* The device failed verification. The bootloader displays a
                                        * warning and stops the boot process, so no keymaster
                                        * implementation should ever actually return this value,
                                        * since it should not run.  Included here only for
                                        * completeness. */
} km_verified_boot_t;

typedef enum {
    SKM_SECURITY_LEVEL_SOFTWARE = 0,
    SKM_SECURITY_LEVEL_TRUSTED_ENVIRONMENT = 1,
} km_security_level_t;

/**
 * Formats for key import and export.
 */
typedef enum {
    SKM_KEY_FORMAT_X509 = 0,  /* for public key export */
    SKM_KEY_FORMAT_PKCS8 = 1, /* for asymmetric key pair import */
    SKM_KEY_FORMAT_RAW = 3,   /* for symmetric key import and export*/
} km_key_format_t;


#define SKM_HW_AUTH_TOKEN_VERSION   0

/*
 * Additional auth token can be required to perform operations. This is
 * related to KM_TAG_AUTH_TOKEN which must be passed as an operation
 * parameter and the key must include a KM_TAG_USER_AUTH_TYPE in it.
 * The comment says that is related to hw_authenticator_type_t,
 * but I cannot find the definition for this.
 */
typedef enum {
    SKM_HW_AUTH_NONE = 0,
    SKM_HW_AUTH_PASSWORD = 1 << 0,
    SKM_HW_AUTH_FINGERPRINT = 1 << 1,
    // Additional entries should be powers of 2.
    SKM_HW_AUTH_ANY = UINT16_MAX,
} km_hw_authenticator_type_t;

typedef struct __attribute__((__packed__)) km_hw_auth_token_t {
    uint8_t version;              // Current version is 0
    uint64_t challenge;
    uint64_t user_id;             // secure user ID, not Android user ID
    uint64_t authenticator_id;    // secure authenticator ID
    uint32_t authenticator_type;  // km_hw_authenticator_type_t, in network order
    uint64_t timestamp;           // milliseconds since boot, when created
    uint8_t hmac[32];
} km_hw_auth_token_t;

typedef struct __attribute__((__packed__)) gk_password_handle_t {
    uint8_t version;
    uint64_t user_id;             // same as secure user ID, above
    uint64_t flags;               // Internal flags associated with password
    uint64_t salt;                // Random number, used in signature
    uint8_t signature[32];
    bool hardware_backed;         // Will always be true for HW signed passwords
} gk_password_handle_t;

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


#define SKM_BLOB_MAGIC               0x4B45594D      /* KEYM */

/* Right now define a max for KM_BYTES and KM_BIGNUM */
#define SKM_TAG_VALUE_BLOB_MAX_SIZE  (1024)

/* Block to receive km_tag_value_set_t must be this size */
#define SKM_TAG_VALUE_BLOCK_SIZE     (4096)

/* Max size - no derivation provided here */
#define SKM_MAX_KEY_BLOB_SIZE        (7248)

/* Max size of the wrapped key blob */
#define SKM_WRAPPED_BLOB_MAX_SIZE    (8192)
/* Size of mask applied to wrapping key during unwrap */
#define SKM_WRAPPED_KEY_MASK_SIZE    (32)

/* Used in km_secure_nonce_t */
#define SKM_NONCE_HMAC_KEY_SIZE      8

#define SKM_RSA_MAX_KEY_SIZE             4096
#define SKM_RSA_GENERATE_MAX_KEY_SIZE    4096    /* Max key size we can currently generate on SAGE */

#define SKM_HMAC_MIN_KEY_SIZE        64
#define SKM_HMAC_MAX_KEY_SIZE        2048

#define SKM_HMAC_MIN_MAC_LENGTH_BITS 64

#define SKM_SHA256_DIGEST_SIZE       (32)

/* HMAC for a KEY blob */
typedef uint8_t km_key_blob_hmac_t[SKM_SHA256_DIGEST_SIZE];

/* Variable size structure to store an array of tag/value pairs */
typedef struct {
    uint32_t num;
    uint32_t size;
    uint8_t params[];
} km_tag_value_set_t;

typedef struct {
    uint64_t challenge;
    uint64_t counter;
    uint32_t hmac_key[SKM_NONCE_HMAC_KEY_SIZE];
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

#define SKM_CERTIFICATES_NUM_MAX 32

typedef struct {
	uint32_t num;
	km_cert_t certificates[SKM_CERTIFICATES_NUM_MAX];
} km_cert_chain_t;

/* Main and StrongBox only, although can be generalized */
#define SKM_MAX_SHARING_PARAMS 2

typedef struct {
    uint8_t seed[SKM_SHA256_DIGEST_SIZE];
    uint8_t nonce[SKM_SHA256_DIGEST_SIZE];
} km_hmac_sharing_t;

typedef struct {
    uint64_t challenge;
    uint64_t timestamp;
    km_security_level_t security_level;
    uint8_t mac[SKM_SHA256_DIGEST_SIZE];
    /* Param auth is not supported so not in token structure */
} km_verification_token_t;


#ifdef __cplusplus
}
#endif


#endif  /* KEYMASTER_TYPES_H__ */

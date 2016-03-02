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
 ******************************************************************************/

#ifndef BSAGELIB_TYPES_H_
#define BSAGELIB_TYPES_H_

#include "bstd.h"
#include "berr_ids.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
Summary:
Platform IDs are allocated by Broadcom
The two following are the only standard ones present in all systems.
**/
#define BSAGE_PLATFORM_ID_UNKNOWN       (0)
#define BSAGE_PLATFORM_ID_COMMONDRM     (0x100)
#define BSAGE_PLATFORM_ID_HDCP22        (0x101)
#define BSAGE_PLATFORM_ID_MANUFACTURING (0x102)
#define BSAGE_PLATFORM_ID_UTILITY       (0x103)
#define BSAGE_PLATFORM_ID_SECURE_VIDEO  (0x107)

/**
Summary:
BERR_Code is the common return type for SAGE Interface.

Description:
- It uses 'Standard error code type', declared in berr.h
- Any value != BERR_SUCCESS is a failure.
- Extends the standard codes witht the following specifics (returned as BERR_Code as well)
**/

#define BSAGE_ERR_HSM                  BERR_MAKE_CODE(BERR_SAGElib_ID,  1) /* HSM error */
#define BSAGE_ERR_ALREADY_INITIALIZED  BERR_MAKE_CODE(BERR_SAGElib_ID,  2) /* Module is already initialized */
#define BSAGE_ERR_INSTANCE             BERR_MAKE_CODE(BERR_SAGElib_ID,  3) /* Given instance does not exist */
#define BSAGE_ERR_INTERNAL             BERR_MAKE_CODE(BERR_SAGElib_ID,  4) /* An internal error occurred on the SAGE side (see SAGE-side logs) */
#define BSAGE_ERR_SHI                  BERR_MAKE_CODE(BERR_SAGElib_ID,  5) /* SHI (transport, sage-side) error */
#define BSAGE_ERR_MODULE_ID            BERR_MAKE_CODE(BERR_SAGElib_ID,  6) /* Module ID is not valid within the given platform */
#define BSAGE_ERR_PLATFORM_ID          BERR_MAKE_CODE(BERR_SAGElib_ID,  7) /* Platform ID is not valid */
#define BSAGE_ERR_MODULE_COMMAND_ID    BERR_MAKE_CODE(BERR_SAGElib_ID,  8) /* Command ID is not valid within the given module */
#define BSAGE_ERR_SYSTEM_COMMAND_ID    BERR_MAKE_CODE(BERR_SAGElib_ID,  9) /* System Command ID is not valid (internal bug) */
#define BSAGE_ERR_STATE                BERR_MAKE_CODE(BERR_SAGElib_ID, 10) /* Instance is not in an appropriate state to handle the request */
#define BSAGE_ERR_CONTAINER_REQUIRED   BERR_MAKE_CODE(BERR_SAGElib_ID, 11) /* A container is mandatory to process the request and none supplied */
#define BSAGE_ERR_SIGNATURE_MISMATCH   BERR_MAKE_CODE(BERR_SAGElib_ID, 12) /* Signature verification failed */
#define BSAGE_ERR_RESET                BERR_MAKE_CODE(BERR_SAGElib_ID, 13) /* SAGE-CPU is under reset */

/* Bin file manager error codes */
#define BSAGE_ERR_BFM_BIN_FILE_LENGTH_SC           BERR_MAKE_CODE(BERR_SAGElib_ID, 14) /* A sanity check error occurred during processing of the binfile */
#define BSAGE_ERR_BFM_NUM_SUPPORTED_SC             BERR_MAKE_CODE(BERR_SAGElib_ID, 15) /* A sanity check error occurred regarding the number of supported DRMs in the binfile */
#define BSAGE_ERR_BFM_PARSE_HEADER_OFFSET          BERR_MAKE_CODE(BERR_SAGElib_ID, 16) /* badly formatted binfile header */
#define BSAGE_ERR_BFM_REGION_FIELD_SIZE            BERR_MAKE_CODE(BERR_SAGElib_ID, 17) /* data section field index size is too large */
#define BSAGE_ERR_BFM_OTP_READ                     BERR_MAKE_CODE(BERR_SAGElib_ID, 18) /* failure to read OTP ID */
#define BSAGE_ERR_BFM_PROC_IN1_MISMATCH            BERR_MAKE_CODE(BERR_SAGElib_ID, 19) /* error validating Type 2/3 procin 1 value */
#define BSAGE_ERR_BFM_PROC_IN2_MISMATCH            BERR_MAKE_CODE(BERR_SAGElib_ID, 20) /* error validating Type 2/3 procin 2 value */
#define BSAGE_ERR_BFM_INVALID_HEADER_FORMAT        BERR_MAKE_CODE(BERR_SAGElib_ID, 21) /* DRM bin file is neither type 1,2 or 3 */
#define BSAGE_ERR_BFM_INVALID_BINFILE_FORMAT_TYPE1 BERR_MAKE_CODE(BERR_SAGElib_ID, 22) /* DRM bin file or type 1 is not allowed */
#define BSAGE_ERR_BFM_DRM_TYPE_NOT_FOUND           BERR_MAKE_CODE(BERR_SAGElib_ID, 23) /* DRM type not found in offset header section of bin file */

/* HDCP 2.2 error codes */
#define BSAGE_ERR_HDCP22_STB_OWN_ID_MISMATCH                BERR_MAKE_CODE(BERR_SAGElib_ID, 24)
#define BSAGE_ERR_HDCP22_INVALID_STB_OWN_ID                 BERR_MAKE_CODE(BERR_SAGElib_ID, 25)
#define BSAGE_ERR_HDCP22_GLOBAL_KEY_OWN_ID_MSP0_MISMATCH    BERR_MAKE_CODE(BERR_SAGElib_ID, 26)
#define BSAGE_ERR_HDCP22_GLOBAL_KEY_OWN_ID_MSP1_MISMATCH    BERR_MAKE_CODE(BERR_SAGElib_ID, 27)
#define BSAGE_ERR_HDCP22_INVALID_GLOBAL_KEY_OWN_ID          BERR_MAKE_CODE(BERR_SAGElib_ID, 28)

/* Continuation of Bin file manager error codes */
#define BSAGE_ERR_BFM_SAGE_KEYLADDER_OTP_NOT_FOUND          BERR_MAKE_CODE(BERR_SAGElib_ID, 29) /* A SAGE keyladder enabled OTP key was not found */
#define BSAGE_ERR_BFM_DATA_SECTION_HASH_MISMATCH            BERR_MAKE_CODE(BERR_SAGElib_ID, 30) /* Data section hash of DRM failed to validate */
#define BSAGE_ERR_BFM_SAGE_KEYLADDER_OTP_INDEX              BERR_MAKE_CODE(BERR_SAGElib_ID, 31) /* DRM bin file format error. OTP key index was not allowed in Sage */
#define BSAGE_ERR_BFM_FILE_SIZE_SC                          BERR_MAKE_CODE(BERR_SAGElib_ID, 32) /* DRM bin file format error. Size detected in header does not match with runtime calculated value */
#define BSAGE_ERR_BFM_NUM_DATA_FIELDS_EXCEEDED              BERR_MAKE_CODE(BERR_SAGElib_ID, 33) /* DRM bin file format error. Number of data fields detected in data section exceeds current maximum limit */
#define BSAGE_ERR_BFM_PROCESS_RPK_SETUP                     BERR_MAKE_CODE(BERR_SAGElib_ID, 34) /* DRM bin file processing error.  Could not load the setup key */
#define BSAGE_ERR_BFM_PROCESS_RPK_BINDING                   BERR_MAKE_CODE(BERR_SAGElib_ID, 35) /* DRM bin file processing error.  Could not setup the binding key */

/* Elliptic Curve Cryptography error codes */
#define BSAGE_ERR_BFM_ECC_CURVE_NOT_FOUND                  BERR_MAKE_CODE(BERR_SAGElib_ID, 36) /* ECC curve not found in ECC DRM bin file */
#define BSAGE_ERR_BFM_KEYLENGTH_UNSUPPORTED                BERR_MAKE_CODE(BERR_SAGElib_ID, 37) /* Curve key length not supported by ECC validation module */

/* SVP error codes */
#define BSAGE_ERR_SVP_VIOLATION                   BERR_MAKE_CODE(BERR_SAGElib_ID, 38) /* SVP violation has occurred */
#define BSAGE_ERR_SVP_INVALID_URR                   BERR_MAKE_CODE(BERR_SAGElib_ID, 39) /* Invalid URR specified, SVP not possible */

#define BSAGE_INSUFFICIENT_HDCP_VERSION            BERR_MAKE_CODE(BERR_SAGElib_ID, 100)



/* String definitions of return codes*/
#define BSAGE_ERR_HSM_STRING                   "An error has occurred in HSM"
#define BSAGE_ERR_ALREADY_INITIALIZED_STRING   "Module is already initialized"
#define BSAGE_ERR_INSTANCE_STRING              "Given instance does not exist"
#define BSAGE_ERR_INTERNAL_STRING              "An internal error occurred on the SAGE side (see SAGE-side logs)"
#define BSAGE_ERR_SHI_STRING                   "SHI (transport, sage-side) error"
#define BSAGE_ERR_MODULE_ID_STRING             "Module ID is not valid within the given platform"
#define BSAGE_ERR_PLATFORM_ID_STRING           "Platform ID is not valid"
#define BSAGE_ERR_MODULE_COMMAND_ID_STRING     "Command ID is not valid within the given module"
#define BSAGE_ERR_SYSTEM_COMMAND_ID_STRING     "System Command ID is not valid (internal bug)"
#define BSAGE_ERR_STATE_STRING                 "Instance is not in an appropriate state to handle the request"
#define BSAGE_ERR_CONTAINER_REQUIRED_STRING    "A container is mandatory to process the request and none supplied"
#define BSAGE_ERR_SIGNATURE_MISMATCH_STRING    "Signature verification failed"
#define BSAGE_ERR_RESET_STRING                 "SAGE-CPU is under reset"

/* Bin file manager error code strings */
#define BSAGE_ERR_BFM_BIN_FILE_LENGTH_SC_STRING            "BinFile Manager open failed.  A sanity check error occurred during processing of the binfile with regards to the size of the data"
#define BSAGE_ERR_BFM_NUM_SUPPORTED_SC_STRING              "BinFile Manager open failed.  A sanity check error occurred regarding the number of supported DRMs in the binfile"
#define BSAGE_ERR_BFM_PARSE_HEADER_OFFSET_STRING           "BinFile Manager open failed.  An out-of-bound error occurred parsing the binfile.  Jump to calculated address bypasses DRM binfile length"
#define BSAGE_ERR_BFM_REGION_FIELD_SIZE_STRING             "Data section field index size is too large"
#define BSAGE_ERR_BFM_OTP_READ_STRING                      "BinFile Manager open failed.  An error occurred reading the OTP ID from the SAGE side"
#define BSAGE_ERR_BFM_PROC_IN1_MISMATCH_STRING             "BinFile Manager open failed.  An error occurred validating the procIn1 value of the DRM binfile. Verify that the DRM binfile being processed was generated with the current chip's OTP ID"
#define BSAGE_ERR_BFM_PROC_IN2_MISMATCH_STRING             "BinFile Manager open failed.  An error occurred validating the procIn2 value of the DRM binfile. Verify that the DRM binfile being processed was generated with the current chip's OTP ID"
#define BSAGE_ERR_BFM_INVALID_HEADER_FORMAT_STRING         "BinFile Manager open failed.  An invalid DRM binfile header detected"
#define BSAGE_ERR_BFM_INVALID_BINFILE_FORMAT_TYPE1_STRING  "BinFile Manager open failed.  Type 1 DRM binfile not supported on SAGE"
#define BSAGE_ERR_BFM_DRM_TYPE_NOT_FOUND_STRING            "BinFile Manager open failed.  The requested DRM type does not exist in the binfile sent to SAGE"


/* HDCP 2.2 error code strings */
#define BSAGE_ERR_HDCP22_STB_OWN_ID_MISMATCH_STRING               "LoadSecret failed.  Mismatch between STB owner ID value read from MSP and the one decrypted from the DRM bin file"
#define BSAGE_ERR_HDCP22_INVALID_STB_OWN_ID_STRING                "LoadSecret failed.  Invalid STB owner id select field"
#define BSAGE_ERR_HDCP22_GLOBAL_KEY_OWN_ID_MSP0_MISMATCH_STRING   "LoadSecret failed.  Mismatch between Global Key owner ID value read from MSP0 and the one decrypted from the bin file"
#define BSAGE_ERR_HDCP22_GLOBAL_KEY_OWN_ID_MSP1_MISMATCH_STRING   "LoadSecret failed.  Mismatch between Global Key owner ID value read from MSP1 and the one decrypted from the bin file"
#define BSAGE_ERR_HDCP22_INVALID_GLOBAL_KEY_OWN_ID_STRING         "LoadSecret failed.  Invalid global key owner id select field"


/* Continuation of Bin file manager error code strings */
#define BSAGE_ERR_BFM_SAGE_KEYLADDER_OTP_NOT_FOUND_STRING            "BinFile Manager open failed.  A SAGE keyladder enabled OTP key was not found"
#define BSAGE_ERR_BFM_DATA_SECTION_HASH_MISMATCH_STRING              "BinFile Manager error. Data section hash of DRM failed to validate "
#define BSAGE_ERR_BFM_SAGE_KEYLADDER_OTP_INDEX_STRING                "BinFile Manager open failed.  OTP key index was not allowed in Sage "
#define BSAGE_ERR_BFM_FILE_SIZE_SC_STRING                            "BinFile Manager open failed. Size detected in header does not match with runtime calculated value"
#define BSAGE_ERR_BFM_NUM_DATA_FIELDS_EXCEEDED_STRING                "BinFile Manager open failed. Number of data fields detected in data section exceeds current maximum limit"
#define BSAGE_ERR_BFM_PROCESS_RPK_SETUP_STRING                       "BinFile Manager error. Could not load the setup key"
#define BSAGE_ERR_BFM_PROCESS_RPK_BINDING_STRING                     "BinFile Manager error. Could not setup the binding key"

/* ECC error code strings */
#define BSAGE_ERR_BFM_ECC_CURVE_NOT_FOUND_STRING           "ECC curve not found in ECC DRM bin file"
#define BSAGE_ERR_BFM_KEYLENGTH_UNSUPPORTED_STRING         "Length of the ECC curve key is not supported by the ECC validation module"

/* SVP error codes */
#define BSAGE_ERR_SVP_VIOLATION_STRING                     "SVP Violation has occurred"
#define BSAGE_ERR_SVP_INVALID_URR_STRING                   "Invalid URR(s) specified for SVP"

#define BSAGE_INSUFFICIENT_HDCP_VERSION_STRING              "Insufficient HDCP version."

/**
Summary:
BSAGElib_State type represent an entity state at a given time
It is currently used to return platform state in order to implement
the platform initialization logic.
**/
typedef enum {
    BSAGElib_State_eUninit = 0,/* Uninitialized. Default state */
    BSAGElib_State_eInit,      /* Initialized */
    BSAGElib_State_eMax        /* number of possible different state values */
} BSAGElib_State;


/**
Summary:
Shared block is a fixed length structure that has the same
memory footprint under 32 and 64bits systems.
It represent a memory block by
 - either its virtual address value or its physical offset value
 - the length of the block
This structure is used to identify a memory block on both the Host and the SAGE side.
**/
typedef struct {
    uint32_t len;       /* memory block length */
    uint32_t _reserved; /* ! reserved */
    union {
        uint8_t *ptr;   /* memory block address */
        uint64_t offset;
    } data;
} BSAGElib_SharedBlock;


/**
Summary:
BSAGElib_InOutContainer is used to backhaul parameters
between the Host and the SAGE system.
The number of basic parameters and shared block is arbitrary
and could be extended to larger value if necessary.
However there should not be any need for so many parameters in any API.
**/
#define BSAGE_CONTAINER_MAX_BASICIN  8
#define BSAGE_CONTAINER_MAX_BASICOUT 4
#define BSAGE_CONTAINER_MAX_SHARED_BLOCKS 8
typedef struct {
    int32_t basicIn[BSAGE_CONTAINER_MAX_BASICIN];  /* Input parameters */
    int32_t basicOut[BSAGE_CONTAINER_MAX_BASICOUT];/* Output parameters */

    /* shared resources */
    BSAGElib_SharedBlock blocks[BSAGE_CONTAINER_MAX_SHARED_BLOCKS];
} BSAGElib_InOutContainer;

typedef enum {
    BSAGElib_eStandbyModeOn = 0,        /* Resume from Standby : S0 mode */
    BSAGElib_eStandbyModePassive = 2,   /* Enter Standby (passive) : S2 mode */
    BSAGElib_eStandbyModeDeepSleep = 3, /* Enter Standby (deep sleep) : S3 mode*/
    BSAGElib_eStandbyMax /* invalid */
} BSAGElib_eStandbyMode;

/* SAGE boot status */
typedef enum
{
    BSAGElibBootStatus_eStarted    = 0x0000,
    BSAGElibBootStatus_eBlStarted  = 0x0001,
    BSAGElibBootStatus_eNotStarted = 0x00FF,
    BSAGElibBootStatus_eError      = 0xFFFF
} BSAGElib_BootStatus;

/* SAGE general status */
/*To add new bit assignments, add a shift enum below and then generate a mask using the shift enum*/
typedef enum
{
    BSAGElibSageStatusFlags_eURRSecureStatus_Shift = 0
} BSAGElib_SageStatusFlags_bits;

#define BSAGElibSageStatusFlags_eURRSecureStatus_Mask (1 << BSAGElibSageStatusFlags_eURRSecureStatus_Shift)

typedef struct {
    uint32_t crrBufferOffset;
    uint32_t crrBufferSize;
    uint32_t encAESKeyOffset;
    uint32_t encAESKeySize;
    uint8_t keyslotId;
    uint8_t engineType;
    void * keySlotHandle;
    uint32_t reserved[8];/*Reserved for future use*/
} BSAGElib_SageLogBuffer;
#ifdef __cplusplus
}
#endif


#endif /* BSAGELIB_TYPES_H_ */

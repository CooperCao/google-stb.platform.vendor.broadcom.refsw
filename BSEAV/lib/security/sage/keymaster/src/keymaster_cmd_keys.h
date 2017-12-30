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

#ifndef KEYMASTER_CMD_KEYS_H__
#define KEYMASTER_CMD_KEYS_H__


/*
 * The following section defines the inout parameters to the command keys
 * functions. It makes the code more readable and less prone to bugs.
 */


/* inout parameters for the generate key start call */
#define KM_CMD_GENERATE_START_IN_NUM_KEY_PARAMS         inout->basicIn[0]
#define KM_CMD_GENERATE_START_IN_KEY_PARAMS_LEN         inout->blocks[0].len
#define KM_CMD_GENERATE_START_IN_KEY_PARAMS_PTR         inout->blocks[0].data.ptr
#define KM_CMD_GENERATE_START_OUT_NONCE_LEN             inout->blocks[1].len
#define KM_CMD_GENERATE_START_OUT_NONCE_PTR             inout->blocks[1].data.ptr
#define KM_CMD_GENERATE_START_OUT_KEY_BLOB_LEN          inout->basicOut[1]

/* inout parameters for the generate key complete call */
#define KM_CMD_GENERATE_COMPLETE_IN_NONCE_LEN           inout->blocks[0].len
#define KM_CMD_GENERATE_COMPLETE_IN_NONCE_PTR           inout->blocks[0].data.ptr
#define KM_CMD_GENERATE_COMPLETE_IN_NUM_KEY_PARAMS      inout->basicIn[0]
#define KM_CMD_GENERATE_COMPLETE_IN_KEY_PARAMS_LEN      inout->blocks[1].len
#define KM_CMD_GENERATE_COMPLETE_IN_KEY_PARAMS_PTR      inout->blocks[1].data.ptr
#define KM_CMD_GENERATE_COMPLETE_OUT_KEY_BLOB_LEN       inout->blocks[2].len
#define KM_CMD_GENERATE_COMPLETE_OUT_KEY_BLOB_PTR       inout->blocks[2].data.ptr
#define KM_CMD_GENERATE_COMPLETE_OUT_RET_KEY_BLOB_LEN   inout->basicOut[1]

/* inout parameters for the get characteristics call */
#define KM_CMD_GET_CHARACTERISTICS_IN_KEY_BLOB_LEN      inout->blocks[0].len
#define KM_CMD_GET_CHARACTERISTICS_IN_KEY_BLOB_PTR      inout->blocks[0].data.ptr
#define KM_CMD_GET_CHARACTERISTICS_IN_NUM_PARAMS        inout->basicIn[0]
#define KM_CMD_GET_CHARACTERISTICS_IN_PARAMS_LEN        inout->blocks[1].len
#define KM_CMD_GET_CHARACTERISTICS_IN_PARAMS_PTR        inout->blocks[1].data.ptr
#define KM_CMD_GET_CHARACTERISTICS_OUT_HW_ENFORCED_LEN  inout->blocks[2].len
#define KM_CMD_GET_CHARACTERISTICS_OUT_HW_ENFORCED_PTR  inout->blocks[2].data.ptr
#define KM_CMD_GET_CHARACTERISTICS_OUT_SW_ENFORCED_LEN  inout->blocks[3].len
#define KM_CMD_GET_CHARACTERISTICS_OUT_SW_ENFORCED_PTR  inout->blocks[3].data.ptr

/* inout parameters for the import key start call */
#define KM_CMD_IMPORT_START_IN_NUM_KEY_PARAMS           inout->basicIn[0]
#define KM_CMD_IMPORT_START_IN_KEY_PARAMS_LEN           inout->blocks[0].len
#define KM_CMD_IMPORT_START_IN_KEY_PARAMS_PTR           inout->blocks[0].data.ptr
#define KM_CMD_IMPORT_START_IN_KEY_FORMAT               inout->basicIn[1]
#define KM_CMD_IMPORT_START_IN_KEY_BLOB_LEN             inout->blocks[1].len
#define KM_CMD_IMPORT_START_IN_KEY_BLOB_PTR             inout->blocks[1].data.ptr
#define KM_CMD_IMPORT_START_OUT_NONCE_LEN               inout->blocks[2].len
#define KM_CMD_IMPORT_START_OUT_NONCE_PTR               inout->blocks[2].data.ptr
#define KM_CMD_IMPORT_START_OUT_KEY_BLOB_LEN            inout->basicOut[1]

/* inout parameters for the import key complete call */
#define KM_CMD_IMPORT_COMPLETE_IN_NONCE_LEN             inout->blocks[0].len
#define KM_CMD_IMPORT_COMPLETE_IN_NONCE_PTR             inout->blocks[0].data.ptr
#define KM_CMD_IMPORT_COMPLETE_IN_NUM_KEY_PARAMS        inout->basicIn[0]
#define KM_CMD_IMPORT_COMPLETE_IN_KEY_PARAMS_LEN        inout->blocks[1].len
#define KM_CMD_IMPORT_COMPLETE_IN_KEY_PARAMS_PTR        inout->blocks[1].data.ptr
#define KM_CMD_IMPORT_COMPLETE_IN_KEY_FORMAT            inout->basicIn[1]
#define KM_CMD_IMPORT_COMPLETE_IN_KEY_BLOB_LEN          inout->blocks[2].len
#define KM_CMD_IMPORT_COMPLETE_IN_KEY_BLOB_PTR          inout->blocks[2].data.ptr
#define KM_CMD_IMPORT_COMPLETE_OUT_KEY_BLOB_LEN         inout->blocks[3].len
#define KM_CMD_IMPORT_COMPLETE_OUT_KEY_BLOB_PTR         inout->blocks[3].data.ptr
#define KM_CMD_IMPORT_COMPLETE_OUT_RET_KEY_BLOB_LEN     inout->basicOut[1]

/* inout parameters for the export key start call */
#define KM_CMD_EXPORT_START_IN_EXPORT_FORMAT            inout->basicIn[0]
#define KM_CMD_EXPORT_START_IN_KEY_BLOB_LEN             inout->blocks[0].len
#define KM_CMD_EXPORT_START_IN_KEY_BLOB_PTR             inout->blocks[0].data.ptr
#define KM_CMD_EXPORT_START_IN_NUM_PARAMS               inout->basicIn[1]
#define KM_CMD_EXPORT_START_IN_PARAMS_LEN               inout->blocks[1].len
#define KM_CMD_EXPORT_START_IN_PARAMS_PTR               inout->blocks[1].data.ptr
#define KM_CMD_EXPORT_START_OUT_NONCE_LEN               inout->blocks[2].len
#define KM_CMD_EXPORT_START_OUT_NONCE_PTR               inout->blocks[2].data.ptr
#define KM_CMD_EXPORT_START_OUT_KEY_BLOB_LEN            inout->basicOut[1]

/* inout parameters for the export key complete call */
#define KM_CMD_EXPORT_COMPLETE_IN_NONCE_LEN             inout->blocks[0].len
#define KM_CMD_EXPORT_COMPLETE_IN_NONCE_PTR             inout->blocks[0].data.ptr
#define KM_CMD_EXPORT_COMPLETE_IN_EXPORT_FORMAT         inout->basicIn[0]
#define KM_CMD_EXPORT_COMPLETE_IN_KEY_BLOB_LEN          inout->blocks[1].len
#define KM_CMD_EXPORT_COMPLETE_IN_KEY_BLOB_PTR          inout->blocks[1].data.ptr
#define KM_CMD_EXPORT_COMPLETE_IN_NUM_PARAMS            inout->basicIn[1]
#define KM_CMD_EXPORT_COMPLETE_IN_PARAMS_LEN            inout->blocks[2].len
#define KM_CMD_EXPORT_COMPLETE_IN_PARAMS_PTR            inout->blocks[2].data.ptr
#define KM_CMD_EXPORT_COMPLETE_OUT_KEY_BLOB_LEN         inout->blocks[3].len
#define KM_CMD_EXPORT_COMPLETE_OUT_KEY_BLOB_PTR         inout->blocks[3].data.ptr
#define KM_CMD_EXPORT_COMPLETE_OUT_RET_KEY_BLOB_LEN     inout->basicOut[1]

/* inout parameters for the attest key start call */
#define KM_CMD_ATTEST_START_IN_KEY_BLOB_LEN             inout->blocks[0].len
#define KM_CMD_ATTEST_START_IN_KEY_BLOB_PTR             inout->blocks[0].data.ptr
#define KM_CMD_ATTEST_START_IN_NUM_KEY_PARAMS           inout->basicIn[0]
#define KM_CMD_ATTEST_START_IN_KEY_PARAMS_LEN           inout->blocks[1].len
#define KM_CMD_ATTEST_START_IN_KEY_PARAMS_PTR           inout->blocks[1].data.ptr
#define KM_CMD_ATTEST_START_OUT_NONCE_LEN               inout->blocks[3].len
#define KM_CMD_ATTEST_START_OUT_NONCE_PTR               inout->blocks[3].data.ptr
#define KM_CMD_ATTEST_START_OUT_CERT_CHAIN_BUFFER_LEN   inout->basicOut[1]

/* inout parameters for the attest key complete call */
#define KM_CMD_ATTEST_COMPLETE_IN_NONCE_LEN             inout->blocks[0].len
#define KM_CMD_ATTEST_COMPLETE_IN_NONCE_PTR             inout->blocks[0].data.ptr
#define KM_CMD_ATTEST_COMPLETE_IN_KEY_BLOB_LEN          inout->blocks[1].len
#define KM_CMD_ATTEST_COMPLETE_IN_KEY_BLOB_PTR          inout->blocks[1].data.ptr
#define KM_CMD_ATTEST_COMPLETE_IN_NUM_KEY_PARAMS        inout->basicIn[0]
#define KM_CMD_ATTEST_COMPLETE_IN_KEY_PARAMS_LEN        inout->blocks[2].len
#define KM_CMD_ATTEST_COMPLETE_IN_KEY_PARAMS_PTR        inout->blocks[2].data.ptr
#define KM_CMD_ATTEST_COMPLETE_OUT_CERT_CHAIN_LEN       inout->blocks[3].len
#define KM_CMD_ATTEST_COMPLETE_OUT_CERT_CHAIN_PTR       inout->blocks[3].data.ptr
#define KM_CMD_ATTEST_COMPLETE_OUT_CERT_CHAIN_BUFFER_LEN    inout->blocks[4].len
#define KM_CMD_ATTEST_COMPLETE_OUT_CERT_CHAIN_BUFFER_PTR    inout->blocks[4].data.ptr
#define KM_CMD_ATTEST_COMPLETE_OUT_RET_CERT_CHAIN_BUFFER_LEN   inout->basicOut[1]

/* inout parameters for the upgrade key start call */
#define KM_CMD_UPGRADE_START_IN_KEY_BLOB_LEN            inout->blocks[0].len
#define KM_CMD_UPGRADE_START_IN_KEY_BLOB_PTR            inout->blocks[0].data.ptr
#define KM_CMD_UPGRADE_START_IN_NUM_KEY_PARAMS          inout->basicIn[0]
#define KM_CMD_UPGRADE_START_IN_KEY_PARAMS_LEN          inout->blocks[1].len
#define KM_CMD_UPGRADE_START_IN_KEY_PARAMS_PTR          inout->blocks[1].data.ptr
#define KM_CMD_UPGRADE_START_OUT_NONCE_LEN              inout->blocks[2].len
#define KM_CMD_UPGRADE_START_OUT_NONCE_PTR              inout->blocks[2].data.ptr
#define KM_CMD_UPGRADE_START_OUT_KEY_BLOB_LEN           inout->basicOut[1]

/* inout parameters for the import key complete call */
#define KM_CMD_UPGRADE_COMPLETE_IN_NONCE_LEN            inout->blocks[0].len
#define KM_CMD_UPGRADE_COMPLETE_IN_NONCE_PTR            inout->blocks[0].data.ptr
#define KM_CMD_UPGRADE_COMPLETE_IN_KEY_BLOB_LEN         inout->blocks[1].len
#define KM_CMD_UPGRADE_COMPLETE_IN_KEY_BLOB_PTR         inout->blocks[1].data.ptr
#define KM_CMD_UPGRADE_COMPLETE_IN_NUM_KEY_PARAMS       inout->basicIn[0]
#define KM_CMD_UPGRADE_COMPLETE_IN_KEY_PARAMS_LEN       inout->blocks[2].len
#define KM_CMD_UPGRADE_COMPLETE_IN_KEY_PARAMS_PTR       inout->blocks[2].data.ptr
#define KM_CMD_UPGRADE_COMPLETE_OUT_KEY_BLOB_LEN        inout->blocks[3].len
#define KM_CMD_UPGRADE_COMPLETE_OUT_KEY_BLOB_PTR        inout->blocks[3].data.ptr
#define KM_CMD_UPGRADE_COMPLETE_OUT_RET_KEY_BLOB_LEN    inout->basicOut[1]

/* inout parameters for the delete key call */
#define KM_CMD_DELETE_KEY_IN_KEY_BLOB_LEN               inout->blocks[0].len
#define KM_CMD_DELETE_KEY_IN_KEY_BLOB_PTR               inout->blocks[0].data.ptr



#endif  /* KEYMASTER_CMD_KEYS_H__ */

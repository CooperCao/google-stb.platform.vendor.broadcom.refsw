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

#ifndef KEYMASTER_CMD_CRYPTO_H__
#define KEYMASTER_CMD_CRYPTO_H__


/*
 * The following section defines the inout parameters to the command crypto
 * functions. It makes the code more readable and less prone to bugs.
 */


/* inout parameters for the crypto begin call */
#define KM_CMD_CRYPTO_BEGIN_IN_PURPOSE                  inout->basicIn[0]
#define KM_CMD_CRYPTO_BEGIN_IN_KEY_BLOB_LEN             inout->blocks[0].len
#define KM_CMD_CRYPTO_BEGIN_IN_KEY_BLOB_PTR             inout->blocks[0].data.ptr
#define KM_CMD_CRYPTO_BEGIN_IN_KEY_SLOT_NUMBER          inout->basicIn[1]
#define KM_CMD_CRYPTO_BEGIN_IN_NUM_PARAMS               inout->basicIn[2]
#define KM_CMD_CRYPTO_BEGIN_IN_PARAMS_LEN               inout->blocks[1].len
#define KM_CMD_CRYPTO_BEGIN_IN_PARAMS_PTR               inout->blocks[1].data.ptr
#define KM_CMD_CRYPTO_BEGIN_OUT_PARAMS_LEN              inout->blocks[2].len
#define KM_CMD_CRYPTO_BEGIN_OUT_PARAMS_PTR              inout->blocks[2].data.ptr
#define KM_CMD_CRYPTO_BEGIN_OUT_OP_HANDLE_LEN           inout->blocks[3].len
#define KM_CMD_CRYPTO_BEGIN_OUT_OP_HANDLE_PTR           inout->blocks[3].data.ptr

/* inout parameters for the crypto update call */
#define KM_CMD_CRYPTO_UPDATE_IN_OP_HANDLE_LEN           inout->blocks[0].len
#define KM_CMD_CRYPTO_UPDATE_IN_OP_HANDLE_PTR           inout->blocks[0].data.ptr
#define KM_CMD_CRYPTO_UPDATE_IN_NUM_PARAMS              inout->basicIn[0]
#define KM_CMD_CRYPTO_UPDATE_IN_PARAMS_LEN              inout->blocks[1].len
#define KM_CMD_CRYPTO_UPDATE_IN_PARAMS_PTR              inout->blocks[1].data.ptr
#define KM_CMD_CRYPTO_UPDATE_IN_DATA_LEN                inout->blocks[2].len
#define KM_CMD_CRYPTO_UPDATE_IN_DATA_PTR                inout->blocks[2].data.ptr
#define KM_CMD_CRYPTO_UPDATE_OUT_INPUT_CONSUMED         inout->basicOut[1]
#define KM_CMD_CRYPTO_UPDATE_OUT_PARAMS_LEN             inout->blocks[3].len
#define KM_CMD_CRYPTO_UPDATE_OUT_PARAMS_PTR             inout->blocks[3].data.ptr
#define KM_CMD_CRYPTO_UPDATE_OUT_RET_DATA_LEN           inout->basicOut[2]
#define KM_CMD_CRYPTO_UPDATE_OUT_DATA_LEN               inout->blocks[4].len
#define KM_CMD_CRYPTO_UPDATE_OUT_DATA_PTR               inout->blocks[4].data.ptr

/* inout parameters for the crypto finish call */
#define KM_CMD_CRYPTO_FINISH_IN_OP_HANDLE_LEN           inout->blocks[0].len
#define KM_CMD_CRYPTO_FINISH_IN_OP_HANDLE_PTR           inout->blocks[0].data.ptr
#define KM_CMD_CRYPTO_FINISH_IN_NUM_PARAMS              inout->basicIn[0]
#define KM_CMD_CRYPTO_FINISH_IN_PARAMS_LEN              inout->blocks[1].len
#define KM_CMD_CRYPTO_FINISH_IN_PARAMS_PTR              inout->blocks[1].data.ptr
#define KM_CMD_CRYPTO_FINISH_IN_DATA_LEN                inout->blocks[2].len
#define KM_CMD_CRYPTO_FINISH_IN_DATA_PTR                inout->blocks[2].data.ptr
#define KM_CMD_CRYPTO_FINISH_IN_SIGNATURE_LEN           inout->blocks[3].len
#define KM_CMD_CRYPTO_FINISH_IN_SIGNATURE_PTR           inout->blocks[3].data.ptr
#define KM_CMD_CRYPTO_FINISH_OUT_PARAMS_LEN             inout->blocks[4].len
#define KM_CMD_CRYPTO_FINISH_OUT_PARAMS_PTR             inout->blocks[4].data.ptr
#define KM_CMD_CRYPTO_FINISH_OUT_RET_DATA_LEN           inout->basicOut[1]
#define KM_CMD_CRYPTO_FINISH_OUT_DATA_LEN               inout->blocks[5].len
#define KM_CMD_CRYPTO_FINISH_OUT_DATA_PTR               inout->blocks[5].data.ptr

/* DEFUNCT inout parameters for the crypto get data start call */
#define KM_CMD_CRYPTO_DATA_START_IN_OP_HANDLE_LEN       inout->blocks[0].len
#define KM_CMD_CRYPTO_DATA_START_IN_OP_HANDLE_PTR       inout->blocks[0].data.ptr
#define KM_CMD_CRYPTO_DATA_START_OUT_DATA_LEN           inout->blocks[1].len
#define KM_CMD_CRYPTO_DATA_START_OUT_DATA_PTR           inout->blocks[1].data.ptr

/* DEFUNCT inout parameters for the crypto get data complete call */
#define KM_CMD_CRYPTO_DATA_COMPLETE_IN_OP_HANDLE_LEN    inout->blocks[0].len
#define KM_CMD_CRYPTO_DATA_COMPLETE_IN_OP_HANDLE_PTR    inout->blocks[0].data.ptr
#define KM_CMD_CRYPTO_DATA_COMPLETE_IN_OUTPUT_CONSUMED  inout->basicIn[0]

/* inout parameters for the crypto abort call */
#define KM_CMD_CRYPTO_ABORT_IN_OP_HANDLE_LEN            inout->blocks[0].len
#define KM_CMD_CRYPTO_ABORT_IN_OP_HANDLE_PTR            inout->blocks[0].data.ptr



#endif  /* KEYMASTER_CMD_CRYPTO_H__ */

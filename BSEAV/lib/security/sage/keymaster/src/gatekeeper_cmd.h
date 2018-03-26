/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifndef GATEKEEPER_CMD_H__
#define GATEKEEPER_CMD_H__


/*
 * The following section defines the inout parameters to the Gatekeeper
 * functions. It makes the code more readable and less prone to bugs.
 */


/* inout parameters for the enroll call */
#define GK_CMD_ENROLL_IN_USER_ID               inout->basicIn[0]
#define GK_CMD_ENROLL_IN_PASS_HANDLE_LEN       inout->blocks[0].len
#define GK_CMD_ENROLL_IN_PASS_HANDLE_PTR       inout->blocks[0].data.ptr
#define GK_CMD_ENROLL_IN_ENROLL_PASS_LEN       inout->blocks[1].len
#define GK_CMD_ENROLL_IN_ENROLL_PASS_PTR       inout->blocks[1].data.ptr
#define GK_CMD_ENROLL_IN_PROVIDED_PASS_LEN     inout->blocks[2].len
#define GK_CMD_ENROLL_IN_PROVIDED_PASS_PTR     inout->blocks[2].data.ptr
#define GK_CMD_ENROLL_OUT_RETRY_TIMEOUT        inout->basicOut[1]
#define GK_CMD_ENROLL_OUT_PASS_HANDLE_LEN      inout->blocks[3].len
#define GK_CMD_ENROLL_OUT_PASS_HANDLE_PTR      inout->blocks[3].data.ptr

/* inout parameters for the verify call */
#define GK_CMD_VERIFY_IN_USER_ID               inout->basicIn[0]
#define GK_CMD_VERIFY_IN_CHALLENGE_LEN         inout->blocks[0].len
#define GK_CMD_VERIFY_IN_CHALLENGE_PTR         inout->blocks[0].data.ptr
#define GK_CMD_VERIFY_IN_PASS_HANDLE_LEN       inout->blocks[1].len
#define GK_CMD_VERIFY_IN_PASS_HANDLE_PTR       inout->blocks[1].data.ptr
#define GK_CMD_VERIFY_IN_PROVIDED_PASS_LEN     inout->blocks[2].len
#define GK_CMD_VERIFY_IN_PROVIDED_PASS_PTR     inout->blocks[2].data.ptr
#define GK_CMD_VERIFY_OUT_RETRY_TIMEOUT        inout->basicOut[1]
#define GK_CMD_VERIFY_OUT_AUTH_TOKEN_LEN       inout->blocks[3].len
#define GK_CMD_VERIFY_OUT_AUTH_TOKEN_PTR       inout->blocks[3].data.ptr


#endif  /* GATEKEEPER_CMD_H__ */

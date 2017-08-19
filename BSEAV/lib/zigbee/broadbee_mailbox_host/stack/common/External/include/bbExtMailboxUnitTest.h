/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

/******************************************************************************
 *
 * DESCRIPTION:
 *      declaration of testing requests for mailbox module.
 *
*******************************************************************************/

#ifdef MAILBOX_UNIT_TEST

#ifndef _MAIL_TESTFUNCTIONS_H
#define _MAIL_TESTFUNCTIONS_H

/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbSysPayload.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief MailUnitTest_f1 - sends parameters(with out data) in two direction
 */
typedef struct _MailUnitTest_f1ReqParams_t
{
    uint8_t     param1;
    uint32_t    param2;
    uint8_t     param3;
} MailUnitTest_f1ReqParams_t;

typedef struct _MailUnitTest_f1ConfParams_t
{
    uint32_t    param4;
    uint8_t     param5;
} MailUnitTest_f1ConfParams_t;

typedef struct _MailUnitTest_f1Descr_t MailUnitTest_f1Descr_t;

typedef void (*MailUnitTest_f1Callback_t)(MailUnitTest_f1Descr_t *req, MailUnitTest_f1ConfParams_t *conf);

typedef struct _MailUnitTest_f1Descr_t
{
    uint8_t                     service;
    MailUnitTest_f1ReqParams_t  params;
    MailUnitTest_f1Callback_t   callback;
} MailUnitTest_f1Descr_t;

/**//**
 * \brief MailUnitTest_f2 - request without parameters and confirm without data pointer
 */
typedef struct _MailUnitTest_f2ConfParams_t
{
    uint32_t    param4;
    uint8_t     param5;
} MailUnitTest_f2ConfParams_t;

typedef struct _MailUnitTest_f2Descr_t MailUnitTest_f2Descr_t;

typedef void (*MailUnitTest_f2Callback_t)(MailUnitTest_f2Descr_t *req, MailUnitTest_f2ConfParams_t *conf);

typedef struct _MailUnitTest_f2Descr_t
{
    uint8_t                     service;
    MailUnitTest_f2Callback_t   callback;
} MailUnitTest_f2Descr_t;

/**//**
 * \brief MailUnitTest_f3 - request without data pointer, no confirmation
 */
typedef struct _MailUnitTest_f3ReqParams_t
{
    uint8_t     param1;
    uint32_t    param2;
    uint8_t     param3;
} MailUnitTest_f3ReqParams_t;

typedef struct _MailUnitTest_f3Descr_t
{
    MailUnitTest_f3ReqParams_t  params;
} MailUnitTest_f3Descr_t;

/**//**
 * \brief MailUnitTest_f4 - request has a data pointer, confirm don't have
 */
typedef struct _MailUnitTest_f4ReqParams_t
{
    uint8_t             param1;
    uint32_t            param2;
    SYS_DataPointer_t   payload;
    uint8_t             param3;
} MailUnitTest_f4ReqParams_t;

typedef struct _MailUnitTest_f4ConfParams_t
{
    uint32_t    param4;
    uint8_t     param5;
} MailUnitTest_f4ConfParams_t;

typedef struct _MailUnitTest_f4Descr_t MailUnitTest_f4Descr_t;

typedef void (*MailUnitTest_f4Callback_t)(MailUnitTest_f4Descr_t *req, MailUnitTest_f4ConfParams_t *conf);

typedef struct _MailUnitTest_f4Descr_t
{
    uint8_t                     service;
    MailUnitTest_f4ReqParams_t  params;
    MailUnitTest_f4Callback_t   callback;
} MailUnitTest_f4Descr_t;

/**//**
 * \brief MailUnitTest_f5 - sends parameters, confirmation has payload
 */
typedef struct _MailUnitTest_f5ReqParams_t
{
    uint8_t             param1;
    uint32_t            param2;
    uint8_t             param3;
} MailUnitTest_f5ReqParams_t;

typedef struct _MailUnitTest_f5ConfParams_t
{
    uint32_t            param4;
    SYS_DataPointer_t   payload;
    uint8_t             param5;
} MailUnitTest_f5ConfParams_t;

typedef struct _MailUnitTest_f5Descr_t MailUnitTest_f5Descr_t;

typedef void (*MailUnitTest_f5Callback_t)(MailUnitTest_f5Descr_t *req, MailUnitTest_f5ConfParams_t *conf);

typedef struct _MailUnitTest_f5Descr_t
{
    uint8_t                     service;
    MailUnitTest_f5ReqParams_t  params;
    MailUnitTest_f5Callback_t   callback;
} MailUnitTest_f5Descr_t;

void MailUnitTest_f1(MailUnitTest_f1Descr_t *req);
void MailUnitTest_f2(MailUnitTest_f2Descr_t *req);
void MailUnitTest_f3(MailUnitTest_f3Descr_t *req);
void MailUnitTest_f4(MailUnitTest_f4Descr_t *req);
void MailUnitTest_f5(MailUnitTest_f5Descr_t *req);

#endif /* _MAIL_TESTFUNCTIONS_H */
#endif /* MAILBOX_UNIT_TEST */

/* eof bbExtMailboxUnitTest.h */
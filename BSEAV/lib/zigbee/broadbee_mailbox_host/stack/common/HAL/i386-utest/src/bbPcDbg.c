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
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/common/HAL/i386-utest/src/bbPcDbg.c $
*
* DESCRIPTION:
*   i386 Debugging Hardware implementation.
*
* $Revision: 3046 $
* $Date: 2014-07-24 20:36:50Z $
*
*****************************************************************************************/


/************************* INCLUDES *****************************************************/
#include "bbPcDbg.h"                /* i386 Debugging Hardware interface. */
#include <stdio.h>                  /* Standard input/output facilities. */


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Delivers error code to the Test-Harness and finishes test execution.
 * \param[in]   errorUid    Error identifier to be delivered.
 * \param[in]   fileName    Pointer to the constant string with the name of the file
 *  containing triggered asserted expression.
 * \param[in]   fileLine    Number of the line in the file at which the asserted
 *  expression was validated.
 * \details
 *  As a result of this call the test being performed will fail if the delivered error
 *  code was not planned for this test. And vice-versa, for the case of test on violation
 *  of an asserted expression, the test waits for this call to be performed for successful
 *  test completion.
*****************************************************************************************/
extern void TEST_DbgAssert(uint32_t errorUid, const char *const fileName, const uint32_t fileLine);


/*************************************************************************************//**
 * \brief   Logs custom debugging message into the Test-Harness console and proceeds with
 *  test execution.
 * \param[in]   message     Pointer to the constant string specifying message to be
 *  logged.
 * \details
 *  The trailing End-of-Line symbol is appended automatically.
*****************************************************************************************/
extern void TEST_DbgLogStr(const char *const message);


/************************* IMPLEMENTATION ***********************************************/
/*
 * Delivers error code to the Test-Harness and finishes test execution.
 */
void PC_DbgHalt(const uint32_t errorUid, const char *const fileName, const uint32_t fileLine)
{
    TEST_DbgAssert(errorUid, fileName, fileLine);
}


/*
 * Delivers warning code to the Test-Harness and finishes test execution.
 */
void PC_DbgLogId(const uint32_t warningUid, const char *const fileName, const uint32_t fileLine)
{
    TEST_DbgAssert(warningUid, fileName, fileLine);
}


/*
 * Logs custom debugging message into the Test-Harness console and proceeds with test
 * execution.
 */
void PC_DbgLogStr(const char *const message)
{
    TEST_DbgLogStr(message);
}


/* eof bbPcDbg.c */
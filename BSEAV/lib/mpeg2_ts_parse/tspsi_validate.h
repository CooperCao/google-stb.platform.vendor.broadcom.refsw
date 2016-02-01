/******************************************************************************
 * (c) 2004-2014 Broadcom Corporation
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
 *
 *****************************************************************************/

#ifndef TSPSI_VALIDATE_H__
#define TSPSI_VALIDATE_H__

/*=**************************************
This module is to verify that a correct message has been returned by
the transport message filtering hardware. Due to a hardware limitation,
the first message returned after starting message capture may be corrupt.

PR's for this issue include 11331 and 8622.

This check does not provide an absolute guarantee that the message is correct,
but it obtains a high degree of probability. The tests are as follows:

1. The application should only call these functions if it is the first
message captured after start.
2. Run TS_Filter_Compare to verify that the filter matches the data.
3. Run TS_Validate_Size to make verify that we have only whole messages.
*****************************************/

#ifndef USE_LEGACY_KERNAL_INTERFACE
#include <bstd.h>
#else
#include <brcm_t.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
CPU-based filter comparison.
**/
bool TS_Filter_Compare( const uint8_t *msg, const uint8_t *inclMask, const uint8_t *exclMask,
    const uint8_t *coef, size_t filterSize );

/**
Return the current message size.
Description:
This assumes msg is pointing to a valid message.
The size includes any end padding.
**/
int TS_Get_Message_Size(const uint8_t *msg);

/**
Validate that a message size is exactly equal to the sum of its section lengths.
Description:
This test is only valid if the message starts at the beginning of the buffer.
The transport hardware will only write whole messages into the buffer, but if
you're not at the beginning, you may have wrap around and this test will fail.
**/
bool TS_Validate_Size( const uint8_t *msg, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* TSPSI_VALIDATE_H__ */

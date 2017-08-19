/****************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ****************************************************************************/

#ifndef _DBG_INTERFACES_H_
#define _DBG_INTERFACES_H_

/* Definition of the interfaces that must be implemented by the target side
 * and the host side of the DBG module. */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "libfp/c_utils.h"

#include "libdspcontrol/CHIP.h"
#include "libdspcontrol/DBG.h"
#include "libdspcontrol/DSPLOG.h"


#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Initialises the DBG DSP-side interface.
 * Called as part of the DBG_init procedure.
 *
 * @param[in] dbg         the DBG instance
 * @param[in] parameters  configuration-specific parameters used to initialise the interface
 * @return                one of the #DBG_RET return values
 */
__attribute__((nonnull))
DBG_RET DBG_targetInit(DBG_TARGET *p_dbg_target, DBG_PARAMETERS *parameters);


/**
 * De-initialises the DBG DSP-side interface.
 * Called as part of the DBG_finish procedure.
 *
 * @param[in] dbg  the DBG instance
 */
__attribute__((nonnull))
void DBG_targetFinish(DBG_TARGET *p_dbg_target);


/**
 * Interacts with the DSP. It sends as much data as possible from the send buffer
 * and and receives as much data as possible, storing it in the receive buffer.
 * The order in which data is sent and received is implementation and configuration-specific.
 *
 * @param dbg[in]                 the DBG instance
 * @param send_buffer[in]         data from this buffer will be sent to target;
 *                                this buffer is not accessed and can be NULL if send_length is 0
 * @param send_length[in,out]     how many bytes to send from send_buffer upon calling,
 *                                the number of sent bytes on return
 * @param receive_buffer[out]     data received from the target will be stored in this buffer;
 *                                this buffer is not accessed and can be NULL if receive_length is 0
 * @param receive_length[in,out]  how big (in bytes) is the receive buffer upon calling,
 *                                the number of read bytes on return
 * @return                        true if more bytes are available to read, false otherwise; if
 *                                'peeking' is not supported, false is returned
 */
__attribute__((nonnull(1,3,5)))
bool DBG_targetExchangeData(DBG_TARGET *p_dbg_target, uint8_t *send_buffer, size_t *send_length, uint8_t *receive_buffer, size_t *receive_length);


/**
 * Syntactic sugar around #DBG_targetExchangeData for receiving data without sending.
 *
 * @param dbg[in]                 the DBG instance
 * @param receive_buffer[out]     data received from the target will be stored in this buffer;
 *                                this buffer is not accessed and can be NULL if receive_length is 0
 * @param receive_length[in,out]  how big (in bytes) is the receive buffer upon calling,
 *                                the number of read bytes on return
 * @return                        true if more bytes are available to read, false otherwise; if
 *                                'peeking' is not supported, false is returned
 */
__unused __alwaysinline __attribute__((nonnull))
static inline
bool DBG_targetReceiveData(DBG_TARGET *p_dbg_target, uint8_t *receive_buffer, size_t *receive_length)
{
    size_t send_length = 0;
    return DBG_targetExchangeData(p_dbg_target, NULL, &send_length, receive_buffer, receive_length);
}


/**
 * Syntactic sugar around #DBG_targetExchangeData for sending data without receiving.
 *
 * @param dbg[in]                 the DBG instance
 * @param receive_buffer[out]     data received from the target will be stored in this buffer;
 *                                this buffer is not accessed and can be NULL if receive_length is 0
 * @param receive_length[in,out]  how big (in bytes) is the receive buffer upon calling,
 *                                the number of read bytes on return
 */
__unused __alwaysinline __attribute__((nonnull))
static inline
bool DBG_targetSendData(DBG_TARGET *p_dbg_target, uint8_t *send_buffer, size_t *send_length)
{
    size_t receive_length = 0;
    return DBG_targetExchangeData(p_dbg_target, send_buffer, send_length, NULL, &receive_length);
}


/**
 * Initialises the DBG host-side interface.
 * Called as part of the DBG_init procedure.
 *
 * @param[in] dbg         the DBG instance
 * @param[in] parameters  configuration-specific parameters used to initialise the interface
 * @return                one of the #DBG_RET return values
 */
__attribute__((nonnull))
DBG_RET DBG_hostInit(DBG_HOST *p_dbg_host, DBG_PARAMETERS *parameters);


/**
 * De-initialises the DBG host-side interface.
 * Called as part of the DBG_finish procedure.
 *
 * @param[in] dbg  the DBG instance
 */
__attribute__((nonnull))
void DBG_hostFinish(DBG_HOST *p_dbg_host);


/**
 * Interacts with the host. It sends as much data as possible from the send buffer
 * and and receives as much data as possible, storing it in the receive buffer.
 * The order in which data is sent and received is implementation and configuration-specific.
 *
 * @param dbg[in]                 the DBG instance
 * @param send_buffer[in]         data from this buffer will be sent to host;
 *                                this buffer is not accessed and can be NULL if send_length is 0
 * @param send_length[in,out]     how many bytes to send from send_buffer upon calling,
 *                                the number of sent bytes on return
 * @param receive_buffer[out]     data received from the host will be stored in this buffer;
 *                                this buffer is not accessed and can be NULL if receive_length is 0
 * @param receive_length[in,out]  how big (in bytes) is the receive buffer upon calling,
 *                                the number of read bytes on return
 * @return                        true if more bytes are available to read, false otherwise; if
 *                                'peeking' is not supported, false is returned
 */
__attribute__((nonnull(1,3,5)))
bool DBG_hostExchangeData(DBG_HOST *p_dbg_host, uint8_t *send_buffer, size_t *send_length, uint8_t *receive_buffer, size_t *receive_length);


/**
 * Syntactic sugar around #DBG_hostExchangeData for receiving data without sending.
 *
 * @param dbg[in]                 the DBG instance
 * @param receive_buffer[out]     data received from the host will be stored in this buffer;
 *                                this buffer is not accessed and can be NULL if receive_length is 0
 * @param receive_length[in,out]  how big (in bytes) is the receive buffer upon calling,
 *                                the number of read bytes on return
 * @return                        true if more bytes are available to read, false otherwise; if
 *                                'peeking' is not supported, false is returned
 */
__unused __alwaysinline __attribute__((nonnull))
static inline
bool DBG_hostReceiveData(DBG_HOST *p_dbg_host, uint8_t *receive_buffer, size_t *receive_length)
{
    size_t send_length = 0;
    return DBG_hostExchangeData(p_dbg_host, NULL, &send_length, receive_buffer, receive_length);
}


/**
 * Syntactic sugar around #DBG_hostExchangeData for sending data without receiving.
 *
 * @param dbg[in]                 the DBG instance
 * @param send_buffer[in]         data from this buffer will be sent to host;
 *                                this buffer is not accessed and can be NULL if send_length is 0
 * @param send_length[in,out]     how many bytes to send from send_buffer upon calling,
 *                                the number of sent bytes on return
 */
__unused __alwaysinline __attribute__((nonnull))
static inline
void DBG_hostSendData(DBG_HOST *p_dbg_host, uint8_t *send_buffer, size_t *send_length)
{
    size_t receive_length = 0;
    DBG_hostExchangeData(p_dbg_host, send_buffer, send_length, NULL, &receive_length);
}


#ifdef __cplusplus
}
#endif

#endif // _DBG_INTERFACES_H_

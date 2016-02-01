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

#ifndef BSAGELIB_MANAGEMENT_H_
#define BSAGELIB_MANAGEMENT_H_

#include "bsagelib.h"

#ifdef __cplusplus
extern "C" {
#endif


/* ---------------------------------------- */
/* Management API (register callbacks for watchdog, ...  */

/* Watchdog callback is fired uppon watchdog event
 * This kind of event may occur if SAGE CPU is reseted. */
typedef void (*BSAGElib_Management_WatchdogCallback)(void);

typedef struct {
    BSAGElib_Management_WatchdogCallback watchdog_isr; /* fired under interrupt */
} BSAGElib_ManagementInterface;

/*
 * Order is predictible:
 * First registered, first fired.
 */

/* All non-null callbacks in BSAGElib_ManagementInterface are registered */
BERR_Code
BSAGElib_Management_Register(
    BSAGElib_Handle hSAGElib,
    BSAGElib_ManagementInterface *i_management);

/* All non-null callbacks in BSAGElib_ManagementInterface are unregistered */
BERR_Code
BSAGElib_Management_Unregister(
    BSAGElib_Handle hSAGElib,
    BSAGElib_ManagementInterface *i_management);

/* Reset RPC ISR and invalidate all the registered remotes
 * This API can only be used when:
 * - The system is resuming from S3 (deep sleep)
 / - A Watchdog event had occured */
void
BSAGElib_Management_Reset(BSAGElib_Handle hSAGElib);

/* ---------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* BSAGELIB_MANAGEMENT_H_ */

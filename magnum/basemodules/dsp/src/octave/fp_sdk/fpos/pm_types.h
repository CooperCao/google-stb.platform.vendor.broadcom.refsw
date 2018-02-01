/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *                Copyright (c) 2016 Broadcom Limited                         *
 *                                                                            *
 *      This material is the confidential trade secret and proprietary        *
 *      information of Broadcom Limited. It may not be reproduced, used       *
 *      sold or transferred to any third party without the prior written      *
 *            consent of Broadcom Limited. All rights reserved.               *
 *                                                                            *
 ******************************************************************************/

#pragma once

/**
 * \file
 * \brief Process manager API types which are shared with host code (or code
 * that cannot include the full pm_api.h).
 */

/** Size of the physmem for the debug buffer that should be passed to process
 * manager under the UUID of TX_UUID_DEBUG_CHANNEL_PHYSMEM, which will be used
 * for debug communications with the host.
 */
#define PM_HOST_DEBUG_BUFFER_SIZE 4096


/**
 * All calls to the process manager API return one of the following status
 * values.
 */
typedef enum
{
    PM_SUCCESS = 0,  /**< The call completed without error */
    PM_NO_ACCESS,    /**< The presented capability does not allow use if this API */
    PM_NO_PROCESS,   /**< There is no process on this system with the given id */
    PM_NO_THREAD,    /**< There is no thread in this process with the given id */
    PM_BUSY,         /**< The resource specified is already in use */
    PM_NO_RESOURCE,  /**< There is no resource with the given value */
    PM_BUFFER,       /**< There isn't room in the message buffer for the result */
    PM_ERRNO,        /**< The call failed due to an error and errno is set */
    PM_TX_ERROR      /**< The call failed due to an error and txt_status returned where available */
} pm_status_t;

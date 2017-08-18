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
*       Trust Center Memory
*
*******************************************************************************/

#ifndef _BB_ZBPRO_TC_ACCESS_H
#define _BB_ZBPRO_TC_ACCESS_H

/************************* INCLUDES *****************************************************/
#include "bbZbProTcMem.h"

/***************************** TYPES ****************************************************/

/**//**
 * \brief Gets TC mode setting value
 */
INLINE ZBPRO_TC_Mode_t zbProTcModeGet(void)
{
    return ZbProTcMemGet()->policy.mode;
}

/**//**
 * \brief Gets TC commissioningModeTimeout setting value
 */
INLINE ZBPRO_TC_CommissioningModeTimeout_t zbProTcCommissioningModeTimeoutGet(void)
{
    return ZbProTcMemGet()->policy.commissioningModeTimeout;
}

/**//**
 * \brief Gets TC allowJoins setting value
 */
INLINE ZBPRO_TC_AllowJoins_t zbProTcAllowJoinsGet(void)
{
    ZBPRO_TC_Mode_t mode = ZbProTcMemGet()->policy.mode;

    return ZbProTcMemGet()->policy.modePolicy[mode].allowJoins;
}

/**//**
 * \brief Gets TC allowAnyJoiningDevice setting value
 */
INLINE ZBPRO_TC_AllowAnyJoiningDevice_t zbProTcAllowAnyJoiningDeviceGet(void)
{
    ZBPRO_TC_Mode_t mode = ZbProTcMemGet()->policy.mode;

    return ZbProTcMemGet()->policy.modePolicy[mode].allowAnyJoiningDevice;
}

/**//**
 * \brief Gets TC transportKeyMethod setting value
 */
INLINE ZBPRO_TC_TransportKeyMethod_t zbProTcTransportKeyMethodGet(void)
{
    ZBPRO_TC_Mode_t mode = ZbProTcMemGet()->policy.mode;

    return ZbProTcMemGet()->policy.modePolicy[mode].transportKeyMethod;
}

#endif /* _BB_ZBPRO_TC_ACCESS_H */

/* eof bbZbProTcPolicyAccess.h */
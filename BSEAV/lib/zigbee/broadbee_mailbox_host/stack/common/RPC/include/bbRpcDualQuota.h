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

/*******************************************************************************
 *
 * DESCRIPTION:
 *      RPC Dual Quota interface.
 *
*******************************************************************************/

#ifndef _BB_RPC_DUAL_QUOTA_H
#define _BB_RPC_DUAL_QUOTA_H


/************************* INCLUDES *****************************************************/
#include "bbSysBasics.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Data type for Quota Counter value.
 */
typedef uint8_t  RpcDualQuotaCounter_t;


/**//**
 * \brief   Data type for Quota Limit value.
 */
typedef RpcDualQuotaCounter_t  RpcDualQuotaLimit_t;


/**//**
 * \brief   Structure for Quota single Channel descriptor.
 * \details
 *  The \c limit field denotes the maximum allowed value for the \c counter field - i.e.,
 *  if the \c counter is less then \c limit, the quoted resource may be granted; and if
 *  the \c counter equals to \c limit, the quota is exceeded. The \c counter may not be
 *  greater then \c limit.
 */
typedef union _RpcDualQuotaChannel_t
{
    uint16_t                   plain;       /*!< Plain value. */
    struct
    {
        RpcDualQuotaLimit_t    limit;       /*!< Quota Channel Limit. */

        RpcDualQuotaCounter_t  counter;     /*!< Quota Channel Counter. */
    };
} RpcDualQuotaChannel_t;


/**//**
 * \brief   Macro function initializes Quota Channel.
 * \param[out]  channel     Reference to the Quota Channel structure.
 * \param[in]   limit       Quota Channel Limit value.
 * \note
 *  Only unsigned data types are allowed for the \p limit parameter.
 */
#define RPC_DUAL_QUOTA_CHANNEL_INIT(channel, limit)\
        SYS_WRAPPED_BLOCK({\
            (channel).plain = (uint16_t)(limit);\
        })

/*
 * Validate size of the RpcDualQuotaChannel_t for RPC_DUAL_QUOTA_CHANNEL_INIT().
 */
SYS_DbgAssertStatic(2 == sizeof(RpcDualQuotaChannel_t));


/**//**
 * \brief   Data type for Quota Channel Index.
 */
typedef size_t  RpcDualQuotaChannelId_t;


/**//**
 * \brief   Number of channels in the Dual Quota.
 */
#define RPC_DUAL_QUOTA_CHANNELS_NUMBER  2


/**//**
 * \brief   Structure for Dual Quota descriptor.
 */
typedef struct _RpcDualQuota_t
{
    RpcDualQuotaChannel_t  channels[RPC_DUAL_QUOTA_CHANNELS_NUMBER];        /*!< Quota Channels array. */

} RpcDualQuota_t;


/************************* INLINES ******************************************************/
/**//**
 * \brief   Initializes Dual Quota.
 * \param[out]  quota       Pointer to the Dual Quota descriptor.
 * \param[in]   limits      Array of Limit values for Channels.
 */
INLINE void rpcDualQuotaInit(
                RpcDualQuota_t     *const  quota,
                const RpcDualQuotaLimit_t  limits[RPC_DUAL_QUOTA_CHANNELS_NUMBER])
{
    SYS_DbgAssert(NULL != quota, HALT_rpcDualQuotaInit_NullQuota);

    for (RpcDualQuotaChannelId_t i = 0; i < RPC_DUAL_QUOTA_CHANNELS_NUMBER; i++)
        RPC_DUAL_QUOTA_CHANNEL_INIT(quota->channels[i], limits[i]);
}


/**//**
 * \brief   Tries to occupy quoted resource on the specified channel.
 * \param[in]   quota           Pointer to the Dual Quota descriptor.
 * \param[in]   channelId       Numeric identifier of the Channel.
 * \return  TRUE if resource is granted; FALSE if quota is exceeded.
 */
INLINE bool rpcDualQuotaTry(
                RpcDualQuota_t         *const  quota,
                const RpcDualQuotaChannelId_t  channelId)
{
    SYS_DbgAssert(NULL != quota, HALT_rpcDualQuotaTry_NullQuota);
    SYS_DbgAssert(channelId < RPC_DUAL_QUOTA_CHANNELS_NUMBER, HALT_rpcDualQuotaTry_InvalidChannelId);

    RpcDualQuotaChannel_t *const  channel =         /* Pointer to the specified Channel descriptor. */
            &quota->channels[channelId];

    SYS_DbgAssertComplex(channel->counter <= channel->limit, HALT_rpcDualQuotaTry_ChannelIsBroken);

    if (channel->counter < channel->limit)
    {
        channel->counter++;
        return TRUE;
    }
    else
        return FALSE;
}


/**//**
 * \brief   Frees quoted resource on the specified channel.
 * \param[in]   quota           Pointer to the Dual Quota descriptor.
 * \param[in]   channelId       Numeric identifier of the Channel.
 */
INLINE void rpcDualQuotaFree(
                RpcDualQuota_t         *const  quota,
                const RpcDualQuotaChannelId_t  channelId)
{
    SYS_DbgAssert(NULL != quota, HALT_rpcDualQuotaFree_NullQuota);
    SYS_DbgAssert(channelId < RPC_DUAL_QUOTA_CHANNELS_NUMBER, HALT_rpcDualQuotaFree_InvalidChannelId);

    RpcDualQuotaChannel_t *const  channel =         /* Pointer to the specified Channel descriptor. */
            &quota->channels[channelId];

    SYS_DbgAssertComplex(channel->counter <= channel->limit, HALT_rpcDualQuotaFree_ChannelIsBroken);

    if (channel->counter > 0)
        channel->counter--;
    else
        SYS_DbgHalt(HALT_rpcDualQuotaFree_FreedMoreThenGranted);
}


/*
 * Repeat pragma GCC optimize because function definitions (including inlined) turn these pragrmas off automatically
 * when compiled by G++ but not GCC.
 */
#if (defined(__arm__) || defined(__i386__)) && !defined(__clang__)
# pragma GCC optimize "short-enums"     /* Implement short enums. */
# pragma GCC diagnostic ignored "-Wattributes"
#endif

#endif /* _BB_RPC_DUAL_QUOTA_H */

/* eof bbRpcDualQuota.h */
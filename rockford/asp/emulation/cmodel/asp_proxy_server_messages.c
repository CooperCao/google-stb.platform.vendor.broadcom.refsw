/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "asp_utils.h"
#include "asp_proxy_server_message_api.h"
#include <errno.h>

/*
 * Returns true only if full msgHeader is read. Otherwise, doesn't read the partial message from the socket.
 */
unsigned
ASP_ProxyServerMsg_RecvHeader(
    int sockFd,
    ASP_ProxyServerMsgHeader *pAspProxyServerSocketMsgHeader
    )
{
    struct msghdr msg;
    struct iovec iov[1];
    ssize_t bytesAvailable;

    iov[0].iov_len = sizeof(*pAspProxyServerSocketMsgHeader);
    iov[0].iov_base = pAspProxyServerSocketMsgHeader;

    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    bytesAvailable = recvmsg(sockFd, &msg, MSG_PEEK);
    if (bytesAvailable < (int)sizeof(*pAspProxyServerSocketMsgHeader))
    {
        fprintf(stdout, "%s: bytesAvailable=%d < (int)sizeof(*pAspProxyServerSocketMsgHeader)=%d sockFd= %u\n", __FUNCTION__, bytesAvailable, sizeof(*pAspProxyServerSocketMsgHeader), sockFd);
        return false;
    }

    /* Full msgHeader is available, so read it. */
    bytesAvailable = recvmsg(sockFd, &msg, 0);
    fprintf(stdout, "%s: rcvd=%zd bytes of msgHeader on sockFd=%d\n", __FUNCTION__, bytesAvailable, sockFd);

    return (true);
}

/*
 * Returns true only if full payload is read. Otherwise, doesn't read the partial payload from the socket.
 */
unsigned
ASP_ProxyServerMsg_RecvPayload(
    int sockFd,
    size_t msgPayloadLength,
    void *msgPayload
    )
{
    struct msghdr msg;
    struct iovec iov[1];
    ssize_t bytesAvailable;

    iov[0].iov_len = msgPayloadLength;
    iov[0].iov_base = msgPayload;

    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    bytesAvailable = recvmsg(sockFd, &msg, MSG_PEEK);
    if (bytesAvailable < (int)msgPayloadLength) return false;

    /* Full msgPayload is available, so read it. */
    bytesAvailable = recvmsg(sockFd, &msg, 0);
    fprintf(stdout, "%s: rcvd=%zd bytes of msg on sockFd=%d \n", __FUNCTION__, bytesAvailable, sockFd );

    return (true);
}

/*
 * Builds message header using type & length and sends it along w/ the msg payload.
 */
unsigned
ASP_ProxyServerMsg_Send(
    int sockFd,
    ASP_ProxyServerMsg msgType,
    size_t msgPayloadLength,
    void *pMsgPayload
    )
{
    struct msghdr msg;
    struct iovec iov[2];
    ASP_ProxyServerMsgHeader aspProxyServerMsgHeader;
    size_t bytesToSend;
    ssize_t returnSize=0;

    memset(&aspProxyServerMsgHeader, 0, sizeof(aspProxyServerMsgHeader));
    aspProxyServerMsgHeader.length = msgPayloadLength;
    aspProxyServerMsgHeader.type = msgType;

    iov[0].iov_len = sizeof(aspProxyServerMsgHeader);
    iov[0].iov_base = &aspProxyServerMsgHeader;

    iov[1].iov_len = msgPayloadLength;
    iov[1].iov_base = pMsgPayload;

    bytesToSend = iov[0].iov_len + iov[1].iov_len;

#if 0
    printf("\n%s; ===================> aspProxyServerMsgHeader.type=%d, aspProxyServerMsgHeader.length = %d, bytesToSend=%d sock=%u \n",__FUNCTION__, aspProxyServerMsgHeader.type , aspProxyServerMsgHeader.length, bytesToSend, sockFd);
#endif

    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = iov;
    msg.msg_iovlen = 2;
    #if 0
    assert( sendmsg(sockFd, &msg, 0) == (ssize_t)bytesToSend );
    #endif

    returnSize = sendmsg(sockFd, &msg, 0);
    if( returnSize != bytesToSend)
    {
        fprintf(stdout, "%s: sentmsg failed returnSize=%d and errno %d\n", __FUNCTION__, returnSize, errno);
        assert(returnSize == (ssize_t)bytesToSend);
    }

    fprintf(stdout, "%s: sent msgType=%d msgPayloadLength=%zu to sockFd=%d\n", __FUNCTION__, msgType, msgPayloadLength, sockFd);
    return (true);
}

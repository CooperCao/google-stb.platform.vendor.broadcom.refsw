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
/**
 * Extension of RTCP Instance Class to provide a SAT>IP Specific RTCP instance
 **/

#include "bip_priv.h"
#include "bip_rtsp_lm_sink.h"

BDBG_MODULE( bip_rtsp_lm_sink );

#if 0
/**
 *  Function: This function is the callback function that is used to wrap the C++ function so that we can call
 *  the function from scheduleDelayedTask() API.
 **/
void statsCollectorWrapper(
    void *context
    )
{
    BIP_RtpSink *pSink = (BIP_RtpSink *)context;

    BIP_CHECK_PTR_GOTO( pSink, "pSink is NULL", error, BIP_ERR_INVALID_PARAMETER );

    BDBG_MSG(( "%s: pSink %p", BSTD_FUNCTION, pSink ));
    pSink->statsCollector();
error:
    return;
}
#endif

BIP_RtpSink:: BIP_RtpSink(
    UsageEnvironment&env,
    Groupsock       *rtpGS,
    unsigned char    rtpPayloadType,
    u_int32_t        rtpTimestampFrequency,
    char const      *rtpPayloadFormatName,
    unsigned         numChannels
    )
    :  RTPSink( env, rtpGS, rtpPayloadType, rtpTimestampFrequency, rtpPayloadFormatName, numChannels ), fEnviron( env )
{
    #if 0
    int64_t timeoutMicroSeconds = 5 * 1000000;

    fStatsCollector = envir().taskScheduler().scheduleDelayedTask( timeoutMicroSeconds, (TaskFunc *)&statsCollectorWrapper, (void *)this );
    #endif
}

BIP_RtpSink::~BIP_RtpSink()
{
    #if 0
    BDBG_MSG(( "%s: destructor; unscheduleDelayedTask 0x%x", BSTD_FUNCTION, fStatsCollector ));
    // Turn off any periodic processing:
    if (fStatsCollector) {envir().taskScheduler().unscheduleDelayedTask( fStatsCollector ); }
    #endif
}

void BIP_RtpSink::destroy()
{
    BDBG_MSG(( "%s: this %p", BSTD_FUNCTION, (void *)this ));
    delete this;
}

Boolean BIP_RtpSink::continuePlaying()
{
    return( True );
}

unsigned BIP_RtpSink:: packetCountSet(
    unsigned newCount
    )
{
    fPacketCount = newCount;
    return( 0 );
}

unsigned BIP_RtpSink:: octetCountSet(
    unsigned newCount
    )
{
    fOctetCount = newCount;
    return( 0 );
}

#if 0
/**
 *  Function: This function will be called periodically to collect the packet count and octet count from the RTP
 *  streamer. These values will be used to create the Sender Report (SR) for RTCP.
 **/
void BIP_RtpSink:: statsCollector(
    void
    )
{
    int64_t timeoutMicroSeconds = 5 * 1000000;

    BDBG_MSG(( "%s: PktCnt 0x%x; OctetCnt 0x%x", BSTD_FUNCTION, fPacketCount, fOctetCount ));
    BDBG_ERR(( "%s: this (%p); fSeqNo %x; fPacketCount %p; fOctetCount %p", BSTD_FUNCTION, this, fSeqNo, fPacketCount, &fOctetCount ));

    fPacketCount   += 500;
    fOctetCount    += 1316*1000;
    fStatsCollector = envir().taskScheduler().scheduleDelayedTask( timeoutMicroSeconds, (TaskFunc *)&statsCollectorWrapper, (void *)this );
}
#endif

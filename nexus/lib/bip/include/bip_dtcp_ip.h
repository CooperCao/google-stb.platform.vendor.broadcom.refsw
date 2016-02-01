/******************************************************************************
 * (c) 2007-2015 Broadcom Corporation
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

#ifndef BIP_DTCP_IP_H
#define BIP_DTCP_IP_H

/*
 * BIP DTCP/IP Security related Data types Definitions
 */


#include "b_dtcp_applib.h"
#include "b_dtcp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Summary:
 * DTCP/IP PCP Header UsageRule definitions (PCP-UR field)
 *
 * Description:
 * Server can set the custom values in the PCP-UR fields.
 * Player can retrieve the values in the PCP-UR fields from the last PCP received.
 **/
typedef struct BIP_DtcpIpUsageRuleSettings
{
    bool             pcpUsageRuleEnabled;           /* optional: Enable PCP-UR feature in the DTCP library. */
                                                    /* true => PCP-UR of 16 bits (containing Media Usage Rule) + Nonce of 48 bits. App will set the specific Usage Rules via the BIP_HttpStreamerSettings. */
                                                    /* false => PCP-UR is not used & PCP will contain Nonce of 48 bits. */
                                                    /* defaults to false. */
    B_UR_Mode_T      usageRuleMode;                 /* PCP-UR field: usage rules modes: DTCP Vol-1 Supplement E Mapping DTCP to IP Rev 1.4 ED3: V1SE 4.26.1.2 */
                                                    /* defaults to B_UR_Mode_eNoInfo. */
    B_Content_Type_T contentType;                   /* PCP-UR field: content type of the media stream: DTCP Vol-1 Supplement E Mapping DTCP to IP Rev 1.4 ED3: V1SE 4.26.1.2 */
                                                    /* defaults to B_Content_eAudioVisual. */
    B_APS_CCI_T      analogContentProtection;       /* PCP-UR field: analog copy protection info: DTCP Vol-1: Section B.2.1 */
                                                    /* defaults to B_APS_eCopyFree */
    B_ICT_T          imageContrainedToken;          /* PCP-UR field: image contrained info: DTCP Vol-1: Section B.2.1 */
                                                    /* defaults to B_ICT_e0. */
    B_AST_T          analogSunsetToken;             /* PCP-UR field: analog sunset token info: DTCP Vol-1 Supplement E Mapping DTCP to IP Rev 1.4 ED3: V1SE 4.26.1.2 */
                                                    /* defaults to B_AST_e0. */
} BIP_DtcpIpUsageRuleSettings;



/**
 * Summary:
 * DtcpStreamerOutput specific settings.
 *
 * Description:
 * This allows App to customize streamer's output related settings.
 **/
typedef struct BIP_DtcpIpOutputSettings
{
    unsigned    akeTimeoutInMs;                 /* Optional: Timeout to wait in msec upto which AKE handshake should be completed with the client, otherwise, StartStreamer API would fail. */
                                                /* Defaults to 5000msec. */
    bool        exchangeKeyLabelValid;          /* Optional: set to true to indicate exchangeKeyLabel field (below) is valid */
                                                /* Defaults to false. */
    unsigned    exchangeKeyLabel;               /* Optional: If exchangeKeyLabelValid is set to true, DTCP/IP library will use ExchangeKey with this label value to derive Content key used for encryption. */
    unsigned    copyControlInfo;                /* Required: Copy Control Info: mapped to EMI */
                                                /* TODO: Need to define a enum for unified CCI values. */
    unsigned    pcpPayloadLengthInBytes;        /* How many bytes of encrypted content should be contained within each PCP. */
                                                /* DTCP/IP Spec allows max payload length of upto 128MB [DTCP/IP Spec V1SE 4.26.1]. */
                                                /* 0 value indicates that PCP header length is determined internally by the library. */
                                                /* Defaults to 8*1024*1024. */
    void       *dtcpIpInitCtx;                  /* optional: if app is not using BIP_DtcpIpServer interface & instead directly using BIP_HttpStreamer, */
                                                /* Then, it must provide the context pointer returned by the DtcpAppLib_Startup() of Broadcom's DTCP/IP library. */
                                                /* HttpStreamer module will use this handle to use the AkeHandle associated with this HTTP Socket and use it for encryption/decryption. */
                                                /* Defaults to NULL. */
    BIP_DtcpIpUsageRuleSettings usageRules;     /* optional: usage rules. */
} BIP_DtcpIpOutputSettings;

/**
 * Summary:
 * DTCP/IP RuntimeSettings
 **/
typedef struct BIP_DtcpIpSettings
{
    B_ExEMI_T       extendedEmi;                    /* Extended EMI (Encryption Mode Indicator) for the associated Streamer */
                                                    /* This can be changed before or during streaming (typically based on the Copy Control Info (CCI) in original stream. */
                                                    /* DTCP/IP Encryption will enabled or disabled based on this flag (meaning DTCP PCP Payload can be in clear). */
} BIP_DtcpIpSettings;



#ifdef __cplusplus
}
#endif

#endif /* BIP_DTCP_IP_H */

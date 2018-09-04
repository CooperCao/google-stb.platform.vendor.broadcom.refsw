/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
static void print_hdmi_status(void)
{
#if NEXUS_HAS_HDMI_OUTPUT
    int rc;
    NxClient_DisplayStatus status;
    rc = NxClient_GetDisplayStatus(&status);
    if (!rc) {
        char hdcp_buf[64];
        unsigned n;
        static const char *g_hdcpState[NEXUS_HdmiOutputHdcpState_eMax] = {
            "Unpowered",
            "Unauthenticated",
            "WaitForValidVideo",
            "InitializedAuthentication",
            "WaitForReceiverAuthentication",
            "ReceiverR0Ready",
            "R0LinkFailure",
            "ReceiverAuthenticated",
            "WaitForRepeaterReady",
            "CheckForRepeaterReady",
            "RepeaterReady",
            "LinkAuthenticated",
            "EncryptionEnabled",
            "RepeaterAuthenticationFailure",
            "RiLinkIntegrityFailure",
            "PjLinkIntegrityFailure"};
        n = snprintf(hdcp_buf, sizeof(hdcp_buf), "%s", g_hdcpState[status.hdmi.hdcp.hdcpState]);
        if (status.hdmi.hdcp.hdcpState == NEXUS_HdmiOutputHdcpState_eEncryptionEnabled) {
            if (sizeof(hdcp_buf) > n) {
                n += snprintf(&hdcp_buf[n], sizeof(hdcp_buf)-n, " %s", status.hdmi.hdcp.hdcp2_2Features ? "2.2" : "1.x");
            }
            if (status.hdmi.hdcp.hdcp2_2Features) {
                NEXUS_HdmiOutputHandle hdmiOutput;
                hdmiOutput = NEXUS_HdmiOutput_Open(NEXUS_ALIAS_ID + 0, NULL);
                if (hdmiOutput) {
                    NEXUS_HdmiOutputHdcpSettings settings;
                    NEXUS_HdmiOutput_GetHdcpSettings(hdmiOutput, &settings);
                    if (sizeof(hdcp_buf) > n) {
                        n += snprintf(&hdcp_buf[n], sizeof(hdcp_buf)-n, " content stream type %u", (settings.hdcp2xContentStreamControl == NEXUS_Hdcp2xContentStream_eType1) ? 1  : 0);
                    }
                    NEXUS_HdmiOutput_Close(hdmiOutput);
                }
            }
        }

        printf("  HDMI: %sconnected, preferred format %s, hdcp %s\n",
            status.hdmi.status.connected?"":"dis",
            lookup_name(g_videoFormatStrs, status.hdmi.status.preferredVideoFormat),
            hdcp_buf);
    }
#endif
}

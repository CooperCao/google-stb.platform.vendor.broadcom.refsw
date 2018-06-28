/***************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 **************************************************************************/
#include "nxclient.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BDBG_MODULE(otpgetchipid);

#ifdef NEXUS_HAS_SECURITY
#if NEXUS_SECURITY_API_VERSION == 2
#include "nexus_otp_key.h"
#else
#include "nexus_otpmsp.h"
#include "nexus_read_otp_id.h"
#endif

int main(void)
{
    NEXUS_Error rc;

    rc = NxClient_Join(NULL);
    if (rc) return -1;

#if NEXUS_SECURITY_API_VERSION == 2
    {
    NEXUS_OtpKeyInfo keyInfo;
    rc = NEXUS_OtpKey_GetInfo(0 /*key A*/, &keyInfo);
    if (rc) {
        BERR_TRACE(rc);
    }
    else {
        unsigned i;
        for (i=0;i<sizeof(keyInfo.id);i++) printf("%02X", keyInfo.id[i]);
        printf("\n");
    }
    }
#else
    {
    NEXUS_OtpIdOutput otpRead;
    rc = NEXUS_Security_ReadOtpId(NEXUS_OtpIdType_eA, &otpRead);
    if (rc) {
        BERR_TRACE(rc);
    }
    else {
        unsigned i;
        for (i=0;i<otpRead.size;i++) printf("%02X", otpRead.otpId[i]);
        printf("\n");
    }
    }
#endif

    NxClient_Uninit();
    return rc;
}
#else
int main(void)
{
    BDBG_ERR(("Nexus security not suported on this platform"));
    return 0;
}
#endif

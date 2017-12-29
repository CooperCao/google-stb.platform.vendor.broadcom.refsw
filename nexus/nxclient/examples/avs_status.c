/***************************************************************************
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
 **************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bstd.h"
#include "nexus_types.h"
#include "nexus_platform.h"
#include "nexus_avs.h"

#include "nxclient.h"
#include "bkni.h"
#include "bkni_multi.h"

int main(void)
{
    int rc;
    NEXUS_AvsStatus pStatus;

    rc = NxClient_Join(NULL);
    if (rc) return -1;

    while (1) {
        rc = NEXUS_GetAvsStatus(&pStatus);
        if ( rc ) {
            fprintf(stderr, "GetAvsStatus failed: %d\n", rc);
            return -1;
        }

        printf("Main voltage is:  %7.3fV\n",  pStatus.voltage/1000.);
        printf("Main temp is:     %+7.2fC\n", pStatus.temperature/1000.);
        printf("AVS is currently: %s\n",      pStatus.enabled?"Enabled":"Disabled");
        printf("AVS is currently: %s\n",      pStatus.tracking?"Tracking":"Idle");

        rc = NEXUS_GetAvsDomainStatus(NEXUS_AvsDomain_eCpu, &pStatus);
        if (!rc) {
            printf("CPU voltage is:   %7.3fV\n",  pStatus.voltage/1000.);
            printf("CPU temp is:      %+7.2fC\n", pStatus.temperature/1000.);
        }
        printf("AVS heartbeat is: %08x\n",    pStatus.heartbeat);
        printf("\n");
        BKNI_Sleep(1000);
    }

    NxClient_Uninit();
    return 0;
}

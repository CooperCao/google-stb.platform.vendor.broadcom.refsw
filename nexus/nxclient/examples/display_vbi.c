/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#include "nxclient.h"
#include "nxclient_global.h"
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* cc data is "[YELLS FIERCELY]" */
static uint8_t g_ccData[] = {
0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80,
0x94, 0x20, 0x80, 0x80,
0x94, 0x20, 0x80, 0x80,
0x94, 0xae, 0x80, 0x80,
0x94, 0xae, 0x80, 0x80,
0x94, 0xf4, 0x80, 0x80,
0x94, 0xf4, 0x80, 0x80,
0x97, 0x23, 0x80, 0x80,
0x97, 0x23, 0x80, 0x80,
0x5b, 0xd9, 0x80, 0x80,
0x45, 0x4c, 0x80, 0x80,
0x4c, 0xd3, 0x80, 0x80,
0x20, 0x46, 0x80, 0x80,
0x49, 0x45, 0x80, 0x80,
0x52, 0x43, 0x80, 0x80,
0x45, 0x4c, 0x80, 0x80,
0xd9, 0x5d, 0x80, 0x80,
0x94, 0x2f, 0x80, 0x80,
0x94, 0x2f, 0x80, 0x80,
};

int main(void)
{
    int rc;
    char buf[32];

    NxClient_Join(NULL);

    printf("Select VBI type: 0=tt, 1=cc, 2=wss, 3=cgms: ");
    fflush(stdout);
    fgets(buf, sizeof(buf), stdin);

    switch (atoi(buf)) {
    case 0:
        {
/* intentionally make it larger than encoder queue to show flow control */
#define NUM_TT_DATA 50
        NEXUS_TeletextLine ttData[NUM_TT_DATA];
        unsigned num;
        unsigned i;
        unsigned total = 0;

        for (i=0;i<NUM_TT_DATA;i++) {
            ttData[i].lineNumber = 0;
            ttData[i].framingCode = 0;
            ttData[i].data[0] = 0x00;
        }

        printf("write %d teletext entries\n", NUM_TT_DATA);
        while (1) {
            rc = NxClient_Display_WriteTeletext(&ttData[total], NUM_TT_DATA-total, &num);
            BDBG_ASSERT(!rc);
            printf(" wrote %d\n", num);
            total += num;
            if (total == NUM_TT_DATA) break;
            BKNI_Sleep(10); /* simple flow control */
        }
        }
        break;

    case 1:
        {
        unsigned total = 0;
        printf("write CC data\n");
        while (1) {
            NEXUS_ClosedCaptionData ccData[10];
            unsigned num;
            for (num=0;num<10 && (total+num)<sizeof(g_ccData);num++) {
                ccData[num].field = 0;
                ccData[num].data[0] = g_ccData[(total+num)*2];
                ccData[num].data[1] = g_ccData[(total+num)*2+1];
            }
            rc = NxClient_Display_WriteClosedCaption(ccData, num, &num);
            BDBG_ASSERT(!rc);
            if (num) {
                printf(" wrote %d\n", num);
                total += num;
                if (total*2 >= sizeof(g_ccData)) break;
            }
            else {
                BKNI_Sleep(100); /* simple flow control */
            }
        }
        }
        break;

    case 2:
        printf("set WSS\n");
        rc = NxClient_Display_SetWss(0x00);
        BDBG_ASSERT(!rc);
        break;

    case 3:
        printf("set CGMS\n");
        rc = NxClient_Display_SetCgms(0x00);
        BDBG_ASSERT(!rc);
        break;
    }

    return 0;
}

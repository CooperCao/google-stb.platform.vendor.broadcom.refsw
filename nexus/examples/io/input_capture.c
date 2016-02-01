/***************************************************************************
*     (c)2010-2014 Broadcom Corporation
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
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#if NEXUS_HAS_INPUT_CAPTURE
#include <stdio.h>
#include <stdlib.h>
#include "nexus_types.h"
#include "nexus_platform.h"
#include "bstd.h"
#include "bkni.h"
#include "nexus_input_capture.h"
#include "nexus_input_capture_init.h"

#define NEXUS_NUM_BCP_CHANNELS 2

static NEXUS_InputCaptureHandle handle[NEXUS_NUM_BCP_CHANNELS];

void capture_callback(void *p1, int p2)
{
    unsigned num_read;
    NEXUS_InputCaptureData data;
    BSTD_UNUSED(p1);

    while (1)
    {
        NEXUS_InputCapture_ReadData(
            handle[p2],
            &data, /* [out] attr{nelem=numEntries;nelem_out=pNumRead} */
            1, /* max number that can be read */
            &num_read /* [out] actual number that were read */
        );
        if (num_read)
            printf("ch%d:  data received from sw fifo = %04xh, polarity=%s\n", p2, data.clockCycles, data.polarity?"positive":"negative");
        else
            break;
    }
}

int main(int argc, char **argv)
{
    NEXUS_InputCaptureSettings settings;
    NEXUS_PlatformSettings platformSettings;
    int i;
    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

    NEXUS_InputCapture_GetDefaultSettings(&settings);
    settings.dataReady.callback = capture_callback;
    for (i=0; i<NEXUS_NUM_BCP_CHANNELS; i++)
    {
        settings.dataReady.param = i;   /* channel */
        settings.type = NEXUS_InputCaptureType_eBicap;
        handle[i] = NEXUS_InputCapture_Open(i, &settings);
        if (handle==NULL)
        {
            printf("error from NEXUS_InputCapture_Open\n");
            return 1;
        }
    }

    for (i=0; i<NEXUS_NUM_BCP_CHANNELS; i++)
        NEXUS_InputCapture_Enable(handle[i]);

    printf("press any key to exit");
    getchar();

    for (i=0; i<NEXUS_NUM_BCP_CHANNELS; i++)
        NEXUS_InputCapture_Close(handle[i]);

    NEXUS_Platform_Uninit();
    return 0;
}
#else
#include <stdio.h>
int main(void) {printf("no input_capture support\n");return 0;}
#endif

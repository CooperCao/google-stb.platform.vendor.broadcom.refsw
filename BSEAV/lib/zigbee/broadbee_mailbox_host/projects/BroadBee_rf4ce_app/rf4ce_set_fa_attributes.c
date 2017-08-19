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
 ******************************************************************************/

/* Simple RF4CE app */

#include "rf4ce_common.h"


const char usage[] = "\nrf4ce_set_fa_attributes - Set rf4ce Frequency Agility Attributes\n\n"
                     "    usage: nrf4ce_set_fa_attributes [threshold] [count threshold] [decrement]\n"
                     "    If any field missed ('-' is used for any other value followed), current value is used.\n";



int main(int argc, char *argv[])
{
    struct zigbeeCallback zcb;

    if (argc <2)
    {
        printf(usage);
        return -1;
    }

    RF4CE_StartReqDescr_t request;
    static struct termios oldt, newt;

#ifdef BYPASS_RPC
    extern int zigbee_init(int argc, char *argv[]);
    zigbee_init(argc, argv);
#endif

    /* Register the callback functions you are interested in.  Ones that are not filled out, won't be called back. */
    /* Calling Zigbee_GetDefaultSettings will initialize the callback structure */
    Zigbee_GetDefaultSettings(&zcb);
    Zigbee_Open(&zcb, "127.0.0.1");

    int threshold0 = BroadBee_RF4CE_Get_FA_Attributes(RF4CE_NWK_FA_SCAN_THRESHOLD);
    int cntThreshold0 = BroadBee_RF4CE_Get_FA_Attributes(RF4CE_NWK_FA_COUNT_THRESHOLD);
    int decrement0 = BroadBee_RF4CE_Get_FA_Attributes(RF4CE_NWK_FA_DECREMENT);

    int threshold = 0;
    int cntThreshold = 0;
    int decrement = 0;

    if (argc > 1)
        sscanf(argv[1], "%d", &threshold);
    if (argc > 2)
        sscanf(argv[2], "%d", &cntThreshold);
    if (argc > 3)
        sscanf(argv[3], "%d", &decrement);

    if (0 == threshold) threshold = threshold0;
    if (0 == cntThreshold) cntThreshold = cntThreshold0;
    if (0 == decrement) decrement = decrement0;

    printf("threshold    = %3d => %3d\n",threshold0,threshold);
    printf("cntThreshold = %3d => %3d\n",cntThreshold0,cntThreshold);
    printf("decrement    = %3d => %3d\n",decrement0, decrement);

    BroadBee_RF4CE_Set_FA_Attributes(RF4CE_NWK_FA_SCAN_THRESHOLD,(int8_t)threshold);
    BroadBee_RF4CE_Set_FA_Attributes(RF4CE_NWK_FA_COUNT_THRESHOLD,(int8_t)cntThreshold);
    BroadBee_RF4CE_Set_FA_Attributes(RF4CE_NWK_FA_DECREMENT,(int8_t)decrement);

    printf("Completed 'rf4ce_set_fa_attributes' application successfully.\n");
    Zigbee_Close();

    return 0;
}

/* eof rf4ce_set_fa_attributes.c */
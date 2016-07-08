/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *************************************************************/
#include <stdio.h>
/* Note: nexus can't include bip files, rather we will define the needed TCP state in nexus_asp.h public API file */
/* and let BIP module copy socket state into that. */
#include "bip_server.h"
#include "asp_simulator.h"

void NEXUS_Asp_Close(void)
{
    return;
}

/* TODO: return a handle here */
int NEXUS_Asp_Open(void)
{
    /* open a PI channel here */
    return 0;
}

/* TODO: add more APIs */
/* NEXUS_Asp_GetDefaultSettings, _GetSettings, _SetSettings */

/* TODO: fix params */
int NEXUS_Asp_Stop(SocketStateHandle socketState)
{
    int rc;
    /* get TCP info from ASP */
    /* TODO: need to make ASP NEXUS & PI calls here */
    rc = aspSimulator_Stop(socketState);
    CHECK_ERR_NZ_GOTO("aspSimulator_Start Failed ...", rc, error);

    return 0;
error:
    return -1;
}

/* TODO: fix params */
int NEXUS_Asp_Start(SocketStateHandle socketState, char *initialData, int initialDataLen)
{
    int rc;
    /* TODO: we should make PI calls here and then PI should make calls into the ASP Simulator */
    printf("%s: starting ASP...\n", __FUNCTION__);
    rc = aspSimulator_Start(socketState, initialData, initialDataLen);
    CHECK_ERR_NZ_GOTO("aspSimulator_Start Failed ...", rc, error);

    printf("%s: Done\n", __FUNCTION__);
    return 0;
error:
    return -1;
}

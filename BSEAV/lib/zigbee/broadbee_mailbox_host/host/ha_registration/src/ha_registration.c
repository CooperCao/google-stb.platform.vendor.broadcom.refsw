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

#include "bbMailAPI.h"
#include "ha_registration.h"


#define MAX_HA_REGISTRATION_ITMES   256

#define GET_REGISTER_ID(ITEM) ((ITEM)->registerId)

static HA_Registration_Info_t ha_Registration_Info[MAX_HA_REGISTRATION_ITMES];

static inline HA_Registration_Info_t *get_Registration_Info_Item(ZBPRO_APS_EndpointId_t endpoint)
{
    return &ha_Registration_Info[endpoint];
}

static bool registration_Info_Item_Is_Registerd(HA_Registration_Info_t *item)
{
    return item->state == HA_REGISTRATION_STATE_REGISTERD ? true : false;
}

void HA_Register_Itself(int socket, ZBPRO_APS_EndpointId_t endpoint)
{
    HA_Registration_Info_t *item = get_Registration_Info_Item(endpoint);
    item->state = HA_REGISTRATION_STATE_REGISTERD;
    item->registerId = socket;
}

void HA_Unregister_Itself(int socket, ZBPRO_APS_EndpointId_t endpoint)
{
    HA_Registration_Info_t *item = get_Registration_Info_Item(endpoint);
    item->state = HA_REGISTRATION_STATE_IDLE;
}

int HA_Registration_Get_Register_Id(ZBPRO_APS_EndpointId_t endpoint)
{
    int result = HA_INVALID_REGISTER_ID;

    HA_Registration_Info_t *item = get_Registration_Info_Item(endpoint);
    if(item == NULL)
        goto _HA_Registation_Get_Register_Id_exit;

    if(registration_Info_Item_Is_Registerd(item))
        result = GET_REGISTER_ID(item);

_HA_Registation_Get_Register_Id_exit:
    return result;
}

int HA_Registration_Get_All_Register_Id(int **sockets)
{
    int *s = (int *)malloc(sizeof(int) * MAX_HA_REGISTRATION_ITMES);
    *sockets = s;
    int numEndpoint = 0;
    for(ZBPRO_APS_EndpointId_t endpoint = 0; endpoint < MAX_HA_REGISTRATION_ITMES - 1; endpoint++){
        HA_Registration_Info_t *item = get_Registration_Info_Item(endpoint);

        if(registration_Info_Item_Is_Registerd(item))
            s[numEndpoint++] = GET_REGISTER_ID(item);
    }
    return numEndpoint;
}

void HA_Registration_Init()
{
    for(int i = 0; i < MAX_HA_REGISTRATION_ITMES; i++)
    {
        ha_Registration_Info[i].state = HA_REGISTRATION_STATE_IDLE;
        ha_Registration_Info[i].registerId = HA_INVALID_REGISTER_ID;
    }
}

/* eof ha_registration.c */
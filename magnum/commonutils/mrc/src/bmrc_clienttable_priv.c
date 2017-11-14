/***************************************************************************
 * Copyright (C) 2016-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/

#include "bstd.h"
#include "bmrc.h"
#include "bmrc_clienttable_priv.h"
#include "bchp_memc_clients.h"

BDBG_MODULE(BMRC_clienttable);
/* client entry structure */
typedef struct BMRC_P_ClientEntry
{
    const char   *pchClientName;
    BMRC_Client   eClient;
    uint16_t      ausClientId[BCHP_P_MEMC_COUNT];
} BMRC_P_ClientEntry;

static const BMRC_P_ClientEntry * BMRC_P_GetClientEntry_isrsafe(BMRC_Client eClient);

#define INVALID BMRC_Client_eInvalid


const char *BMRC_P_GET_CLIENT_NAME_isrsafe(BMRC_Client eClient)
{
    const BMRC_P_ClientEntry *client = BMRC_P_GetClientEntry_isrsafe(eClient);
    if(client) {
        return client->pchClientName;
    }
    return "Invalid";
}

int BMRC_P_GET_CLIENT_ID(uint16_t usMemcId, BMRC_Client   eClient)
{
    const BMRC_P_ClientEntry *client = BMRC_P_GetClientEntry_isrsafe(eClient);
    if(client) {
        return client->ausClientId[usMemcId];
    }
    return -1;
}

BERR_Code BMRC_Checker_P_GetClientInfo_isrsafe(unsigned memcId, BMRC_Client eClient, BMRC_ClientInfo *pClientInfo)
{
    BERR_Code rc = BERR_SUCCESS;
    if (eClient < BMRC_Client_eInvalid) {
        const BMRC_P_ClientEntry *client = BMRC_P_GetClientEntry_isrsafe(eClient);
        if(client) {
            pClientInfo->eClient = eClient;
            pClientInfo->pchClientName = client->pchClientName;
            pClientInfo->usClientId = client->ausClientId[memcId];
            return BERR_SUCCESS;
        } else {
            pClientInfo->eClient = eClient;
            rc = BERR_INVALID_PARAMETER; /* silent */
        }
    } else {
        pClientInfo->eClient = BMRC_Client_eInvalid;
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    pClientInfo->pchClientName = "UNKNOWN";
    pClientInfo->usClientId = BMRC_Client_eInvalid; /* NOTE: this assumes that the max SW enum is > the max HW client id */
    return rc;
}


static const BMRC_P_ClientEntry BMRC_P_astClientTbl[] = {
#if BCHP_P_MEMC_COUNT == 1
#define BCHP_P_MEMC_DEFINE_CLIENT(client,m0) {#client, BCHP_MemcClient_e##client, {m0}},
#elif BCHP_P_MEMC_COUNT == 2
#define BCHP_P_MEMC_DEFINE_CLIENT(client,m0,m1) {#client, BCHP_MemcClient_e##client, {m0,m1}},
#elif BCHP_P_MEMC_COUNT == 3
#define BCHP_P_MEMC_DEFINE_CLIENT(client,m0,m1,m2) {#client, BCHP_MemcClient_e##client, {m0,m1,m2}},
#else
#error "not supported"
#endif

#include "memc/bchp_memc_clients_chip.h"
};
#undef BCHP_P_MEMC_DEFINE_CLIENT


static const BMRC_P_ClientEntry * BMRC_P_GetClientEntry_isrsafe(BMRC_Client eClient)
{
    unsigned i;
    static const BMRC_P_ClientEntry *client_map[BMRC_Client_eMaxCount]; /* cache of translation entries */
    const BMRC_P_ClientEntry *client;

    if(eClient>=BMRC_Client_eMaxCount) {
        return NULL;
    }
    client = client_map[eClient]; /* try cache first */
    if(client) { /* NULL means uninitialized */
        return client;
    }
    for(i=0;i<sizeof(BMRC_P_astClientTbl)/sizeof(*BMRC_P_astClientTbl);i++) {
        client = &BMRC_P_astClientTbl[i];
        if(client->eClient == eClient) {
            client_map[eClient] = client; /* save in a cache */
            return client;
        }
    }
    return NULL;
}

BMRC_Client BMRC_P_GET_CLIENT_ENUM_isrsafe(uint16_t usMemcId, uint16_t usClientId)
{
    unsigned i;
    for(i=0;i<sizeof(BMRC_P_astClientTbl)/sizeof(*BMRC_P_astClientTbl);i++) {
        if(BMRC_P_astClientTbl[i].ausClientId[usMemcId] == usClientId) {
            return BMRC_P_astClientTbl[i].eClient;
        }
    }
    return BMRC_Client_eInvalid;
}

const char *BMRC_Checker_GetClientName(unsigned memc, unsigned clientId)
{
    if(memc<BCHP_P_MEMC_COUNT) {
        unsigned i;
        for(i=0;i<sizeof(BMRC_P_astClientTbl)/sizeof(*BMRC_P_astClientTbl);i++) {
            if(BMRC_P_astClientTbl[i].ausClientId[memc] == clientId) {
                return BMRC_P_astClientTbl[i].pchClientName;
            }
        }
    }
    return "UNKNOWN";
}

/* End of File */

/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
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

static const BMRC_P_ClientEntry * BMRC_P_GetClientEntry(BMRC_Client eClient);

#define INVALID BMRC_Client_eInvalid


const char *BMRC_P_GET_CLIENT_NAME(BMRC_Client eClient)
{
    const BMRC_P_ClientEntry *client = BMRC_P_GetClientEntry(eClient); 
    if(client) {
        return client->pchClientName;
    }
    return "Invalid";
}

int BMRC_P_GET_CLIENT_ID(uint16_t usMemcId, BMRC_Client   eClient)
{
    const BMRC_P_ClientEntry *client = BMRC_P_GetClientEntry(eClient); 
    if(client) {
        return client->ausClientId[usMemcId];
    }
    return -1;
}

BERR_Code BMRC_Checker_P_GetClientInfo_isrsafe(unsigned memcId, BMRC_Client eClient, BMRC_ClientInfo *pClientInfo)
{
    BERR_Code rc = BERR_SUCCESS;
    if (eClient < BMRC_Client_eInvalid) {
        const BMRC_P_ClientEntry *client = BMRC_P_GetClientEntry(eClient);
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


static const BMRC_P_ClientEntry * BMRC_P_GetClientEntry(BMRC_Client eClient)
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


/* End of File */

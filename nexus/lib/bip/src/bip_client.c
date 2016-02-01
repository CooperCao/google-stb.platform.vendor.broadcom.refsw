/******************************************************************************
 * (c) 2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#include <string.h>

#include "bip_priv.h"

BDBG_MODULE( bip_client );

BDBG_OBJECT_ID( BIP_Client );

typedef struct BIP_Client
{
    BDBG_OBJECT( BIP_Client )

    char    *clientName;
} BIP_Client;

/*****************************************************************************
 * Constructor: Create an instance of a class.
 *****************************************************************************/
BIP_ClientHandle
BIP_Client_Create(
    /* TODO: if parameters, then add GetDefaultInitSettings() */
    void )
{
    struct BIP_Client   *newBipClient;

    newBipClient = BKNI_Malloc( sizeof(BIP_Client));
    if (NULL == newBipClient)
    {
        BERR_TRACE( BIP_ERR_OUT_OF_SYSTEM_MEMORY );
        return(NULL);
    }

    BKNI_Memset( newBipClient, 0, sizeof(*newBipClient));
    BDBG_OBJECT_SET( newBipClient, BIP_Client );

    return(newBipClient);
}

/*****************************************************************************
 * Destructor: Destroy an instance a class.
 *****************************************************************************/
void
BIP_Client_Destroy( BIP_ClientHandle hBipClient )
{
    BDBG_OBJECT_ASSERT( hBipClient, BIP_Client );

#if 0  /* ==================== GARYWASHERE - Start of Original Code ==================== */
       /* Free up any fields that have been malloc'd. */
    if (hBipClient->clientName)
    {
        BKNI_Free( hBipClient->clientName );
        hBipClient->clientName = NULL;
    }
#endif /* ==================== GARYWASHERE - End of Modified Code   ==================== */

    /* Then finally free the BIP_Client struct. */
    BDBG_OBJECT_DESTROY( hBipClient, BIP_Client );
    BKNI_Free( hBipClient );
    return;
} /* BIP_Client_Destroy */

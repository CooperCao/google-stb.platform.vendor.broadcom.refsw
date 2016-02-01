/********************************************************************************************
*     (c)2004-2015 Broadcom Corporation                                                     *
*                                                                                           *
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,   *
*  and may only be used, duplicated, modified or distributed pursuant to the terms and      *
*  conditions of a separate, written license agreement executed between you and Broadcom    *
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants*
*  no license (express or implied), right to use, or waiver of any kind with respect to the *
*  Software, and Broadcom expressly reserves all rights in and to the Software and all      *
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU       *
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY                    *
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.                                 *
*
*  Except as expressly set forth in the Authorized License,                                 *
*
*  1.     This program, including its structure, sequence and organization, constitutes     *
*  the valuable trade secrets of Broadcom, and you shall use all reasonable efforts to      *
*  protect the confidentiality thereof,and to use this information only in connection       *
*  with your use of Broadcom integrated circuit products.                                   *
*                                                                                           *
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"          *
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR                   *
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO            *
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES            *
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,            *
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION             *
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF              *
*  USE OR PERFORMANCE OF THE SOFTWARE.                                                      *
*                                                                                           *
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS         *
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR             *
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR               *
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF             *
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT              *
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE            *
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF              *
*  ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *     IP AKE core session data functions.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *********************************************************************************************/
/*! \file b_dtcp_ake.c
 *  \brief define AKE core functions.
 */
#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "b_dtcp_applib.h"
#include "b_dtcp_ip_ake.h"
#include "b_ecc_wrapper.h"
#include "b_dtcp_status_codes.h"

BDBG_MODULE(b_dtcp_ip);

/*! \brief initialize AKE core data
 *  \param[in] CoreData pointer to AKE core data to be initialized.
 *  \param[in] Mode device mode, source/sink.
 */
B_AkeCoreData_T * B_DTCP_IpAkeCoreInit(B_DeviceParams_T * DeviceParams)
{
    B_AkeCoreData_T * pCore = NULL;
    B_IpAkeCoreData_T *pIpCore = NULL;

    BDBG_ENTER(B_DTCP_IpAkeCoreInit);
    /* Init the base Ake core data.*/
    if((pCore = B_DTCP_AkeCoreInit(DeviceParams) ) == NULL)
        return NULL;
    /* Allocate and init protocol specific data */
    pIpCore = BKNI_Malloc(sizeof(B_IpAkeCoreData_T));
    if(pIpCore == NULL) {
        BDBG_ERR(("Malloc failed for IP AKE core data.\n"));
        B_DTCP_AkeCore_UnInit(pCore);
        return NULL;
    }
    BLST_S_INIT( &(pIpCore->RttRegistry_list));
    pCore->pProtocolData = (void*)pIpCore;

    BDBG_LEAVE(B_DTCP_IpAkeCoreInit);
    return pCore;
}
/*! \brief cleanup IP Ake core data.
 *  \param[in] CoreData pointer to AKE core data.
 */
void B_DTCP_IpAkeCore_UnInit(B_AkeCoreData_T * CoreData)
{
    BDBG_ENTER(B_DTCP_IpAkeCore_UnInit);

    BKNI_Free(CoreData->pProtocolData);
    CoreData->pProtocolData = NULL;
    B_DTCP_AkeCore_UnInit(CoreData);

    BDBG_LEAVE(B_DTCP_IpAkeCore_UnInit);
}

/*! \brief create an Ip Ake Session, initialize session data.
 *  \param[in] CoreData AKE core data pointer.
 *  \param[in] aRemoteIp remote IP address.
 *  \param[in] aRemotePort remote port number.
 *  \param[in] BgRttTest indicate if the session support BG RTT test.
 *  \param[in] AkeType type of AKE to be performed.
 *  \param[in] DeviceMode, source or sink device.
 *  \param[in,out] returned Ake core session data handle.
 */
BERR_Code B_DTCP_CreateIpAkeSession(B_AkeCoreData_T * CoreData,
        const char * aRemoteIp,
        unsigned short aRemotePort,
        bool BgRttTest,
        B_AkeType_T AkeType,
        B_DeviceMode_T DeviceMode,
        B_AkeCoreSessionData_T ** pSession
        )
{
    BERR_Code retValue = BERR_SUCCESS;
    B_IpAkeSessionData_T * IpAkeSession = NULL;
    B_AkeCoreSessionData_T * Session = NULL;

    BDBG_ASSERT(aRemoteIp);
    BDBG_ASSERT(pSession);
    BDBG_ENTER(B_DTCP_CreateIpAkeSession);

    if((IpAkeSession = BKNI_Malloc(sizeof(B_IpAkeSessionData_T))) == NULL)
        return B_ERROR_OUT_OF_MEMORY;
    else
        BKNI_Memset(IpAkeSession, 0, sizeof(B_IpAkeSessionData_T));

    BKNI_Memcpy(IpAkeSession->RemoteIp, aRemoteIp, MAX_IP_ADDR_SIZE);
    IpAkeSession->RemotePort = aRemotePort;
    /* May need to get local ip/host first */
    BKNI_Memset(IpAkeSession->LocalIp, 0, MAX_IP_ADDR_SIZE);
    IpAkeSession->LocalPort = 0;
    IpAkeSession->ContKeyConfSession = NULL;
    IpAkeSession->BgRttTest = BgRttTest;

    IpAkeSession->RttN[0] = 0;
    IpAkeSession->RttN[1] = 0;
    IpAkeSession->RttValue = 0xFFFF;        /* Rtt unknown yet */
    IpAkeSession->ContKeyConfirmed = true;  /*if CKC failed, it will become false */
    IpAkeSession->StartContKeyConf = false;
    IpAkeSession->TerminateCKCThread = false;

    /* Initialize R value for content key confirm*/
    if (B_RNG(CoreData->pDeviceParams->hDtcpIpTl, &IpAkeSession->ContR[0], DTCP_CONT_KEY_CONF_R_SIZE) != BERR_SUCCESS)
    {
        BDBG_ERR(("Failed to initialize ContR\n"));
        BKNI_Free(IpAkeSession);
        return BERR_UNKNOWN;
    }
    /* Create base AKE session data */
    if((retValue = B_DTCP_CreateAkeSession(CoreData,
                AkeType,
                DeviceMode,
                pSession) ) != BERR_SUCCESS)
    {
        BDBG_ERR(("Failed to create base AKE session: %d\n", retValue));
        BKNI_Free(IpAkeSession);
        return BERR_UNKNOWN;
    }else {
        Session = (B_AkeCoreSessionData_T *)(*pSession);
        Session->pProtocolData = (void*)IpAkeSession;
    }
    /* Exchange Key type for dtcp-ip, AES */
    Session->ExchKeyType = B_ExchKey_eAES128;

    Session->RefCnt = 1;
    BDBG_MSG(("CreateIpAkeSession RefCnt %d", Session->RefCnt));
    BDBG_LEAVE(B_DTCP_CreateIpAkeSession);

    return retValue;
}
/*! \brief destroy IP AKE session cleanup allocated resource.
 *  \param[in] AkeHandle handle to an IP AKE session to be destroyed.
 */
void B_DTCP_DestroyIpAkeSession(void * AkeHandle)
{
    B_AkeCoreSessionData_T * pSession = (B_AkeCoreSessionData_T *)AkeHandle;
    B_IpAkeSessionData_T * pIpSession = NULL;
    BDBG_ASSERT(AkeHandle);
    pIpSession = pSession->pProtocolData;
    BDBG_ASSERT(pIpSession);

    BDBG_ENTER(B_DTCP_DestroyIpAkeSession);
    B_Mutex_Lock(pSession->pAkeCoreData->hMutex);
    if (pSession->RefCnt > 1) {
        pSession->RefCnt--;
        BDBG_MSG(("DESTROYING SESSION New RefCnt %d", pSession->RefCnt));
        B_Mutex_Unlock(pSession->pAkeCoreData->hMutex);
        goto out;
    }
    B_Mutex_Unlock(pSession->pAkeCoreData->hMutex);

    pSession->RefCnt = 0;
    BDBG_MSG(("DestroyIpAkeSession RefCnt %d", pSession->RefCnt));
    if(pIpSession->ContKeyConfSession != NULL)
    {
        BDBG_WRN(("ContKeyConfSession might hasn't been destroyed yet!\n"));
    }
    BKNI_Free(pIpSession);
    pSession->pProtocolData = NULL;

    /* destroy the base AKE session. */
    B_DTCP_DestroyAkeSession(pSession);
out:
    BDBG_LEAVE(B_DTCP_DestroyIpAkeSession);
}
/* Helper function to increment the RTT N value (Rtt trial counter )
 */
void B_DTCP_IncrementRttN(unsigned char n[2])
{
    if(n[1] == 0xff)
    {
        n[0] += 1;
        n[1] = 0;
    }else
        n[1] += 1;
}
bool B_DTCP_IP_IsContKeyConfirmed(B_AkeCoreSessionData_T * pSession)
{
    B_IpAkeSessionData_T * pIpAkeSession = (B_IpAkeSessionData_T *)pSession->pProtocolData;
    return pIpAkeSession->ContKeyConfirmed;
}

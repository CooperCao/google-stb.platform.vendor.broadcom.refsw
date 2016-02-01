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
 *     AKE core funtions.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *********************************************************************************************/
/*! \file b_dtcp_ake.c
 *  \brief define AKE core functions.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "b_dtcp_applib.h"
#include "b_dtcp_ake.h"
#include "b_dtcp_status_codes.h"
#include "b_ecc_wrapper.h"
#include "b_dtcp_stream.h"

#include "bcrypt.h"

BDBG_MODULE(b_dtcp_ip);

/*! \brief print out AKE message buffer, this probably shouldn't be here.
 */
void B_DTCP_DebugBuff(unsigned char * buf, int size)
{
    int i;
    char temp[1024];
    char *p = &temp[0];
    BDBG_ASSERT(buf);
    if (size >= 1024)
    {
        BDBG_WRN(("size %d is too big, skipping message dump\n", size));
        return;
    }
    for (i = 0; i < size; i++)
    {
        if (i == 0) {
            sprintf(p, "\t");
            p += strlen(p);
        }
        if ((i & 0x3) == 0) {
            sprintf(p, " ");
            p += strlen(p);
        }
        if ((i & 0xf) == 0) {
            sprintf(p, "\n\t");
            p += strlen(p);
        }
        sprintf(p, "%02X ", *(buf + i));
        p += strlen(p);
    }
    sprintf(p, "\n");
    BDBG_MSG(("%s", &temp[0]));
}
/*! \brief utility to get Ake Type based on device's parameter.
 *  \param[out] pAkeType AKE type.
 *  \param[in]  pDeviceParams device parameter pointer.
 */
BERR_Code B_DTCP_GetAkeTypeFromCertificate(B_AkeType_T * pAkeType, B_DeviceParams_T * pDeviceParams)
{
    BDBG_ASSERT(pDeviceParams);
    switch(pDeviceParams->CertSize)
    {
        case DTCP_BASELINE_FULL_CERT_SIZE:
        {
            *pAkeType = B_AkeType_eFull;
            break;
        }
        case DTCP_EXTENDED_FULL_CERT_SIZE:
        {
            *pAkeType = B_AkeType_eExtendedFull;
            break;
        }
        case DTCP_RESTRICTED_CERT_SIZE:
        {
            *pAkeType = B_AkeType_eRestricted;
            break;
        }
        default:
            return BERR_INVALID_PARAMETER;
    }
    return BERR_SUCCESS;
}
/*! \brief check if Addional Localization is required.
 *  \param[in] DeviceParams device parameter.
 *  \param[in] Session Ake session pointer.
 *  \retval true if AL required, false otherwise.
 */
bool B_DTCP_IsALRequired(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session)
{
    int ourAL = 0;
    bool retValue = false;
    B_BaselineFullCert_T * cert = (B_BaselineFullCert_T *)DeviceParams->Cert;
    ourAL = cert->AL & 0x01;
    if(ourAL && Session->OtherAL)
        retValue =  true;
    return retValue;
}
/*! \brief increase a 64 bits nonce by 1, called by content management and conteng key confirmation functions.
 *  \param[in] hMutex session's mutex handle.
 *  \param[in,out] nonce , nonce value to operate on.
 */
void B_DTCP_IncrementNonce(B_MutexHandle hMutex, unsigned char nonce[8])
{
    unsigned long * high, *low;
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
    high = (unsigned long *)&nonce[0];
    low = (unsigned long *)&nonce[4];
#else
    high = (unsigned long *)&nonce[4];
    low = (unsigned long *)&nonce[0];
#endif
    BDBG_MSG(("Incrementing nonce, old value=0x%08lx %08lx\n", *high, *low));
    if(hMutex != NULL) B_Mutex_Lock(hMutex);
    if( *low == 0xFFFFFFFF )
    {
        *low = 0;
        *high += 1;
    }else {
        *low += 1;
    }
    BDBG_MSG(("Incrementing nonce, new value=0x%08lx %08lx\n", *high, *low));
    if(hMutex != NULL) B_Mutex_Unlock(hMutex);
}
/*! \brief compare two 64 bits nonce value
 *  \param[in] first nonce value to compare.
 *  \param[in] second nonce value to compare.
 *
 *  This implementation assume the diff is within 32 bits long.
 */
long B_DTCP_GetNonceDiff(unsigned char anonce[8], unsigned char bnonce[8])
{
    unsigned long *ahigh, *alow, *bhigh, *blow;
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
    ahigh = (unsigned long *)&anonce[0];
    alow = (unsigned long *)&anonce[4];
    bhigh = (unsigned long *)&bnonce[0];
    blow = (unsigned long *)&bnonce[4];
#else
    ahigh = (unsigned long *)&anonce[4];
    alow = (unsigned long *)&anonce[0];
    bhigh = (unsigned long *)&bnonce[4];
    blow = (unsigned long *)&bnonce[0];
#endif
    if( *ahigh - *bhigh != 0)
    {
        BDBG_WRN(("Nonce diff is too big\n"));
    }
    return *alow - *blow;
}
/*! \brief get realtime nonce value from core data.
 *  \param[in] hMutex mutex to protect shared data.
 *  \param[in] data contain the Nonce to be copied.
 *  \param[out] pointer contain output buffer.
 */
void B_DTCP_GetSetRealTimeNonce(B_MutexHandle hMutex, const unsigned char RtNonceIn[DTCP_CONTENT_KEY_NONCE_SIZE],
        unsigned char RtNonceOut[DTCP_CONTENT_KEY_NONCE_SIZE])
{
    B_Mutex_Lock(hMutex);
    BKNI_Memcpy(RtNonceOut, RtNonceIn, DTCP_CONTENT_KEY_NONCE_SIZE);
    B_Mutex_Unlock(hMutex);
}

#ifdef DTCP_IP_RTP_SUPPORT
/*! \brief thread function for realtime nonce timer
 *  \param[in] user AKE core data pointer.
 */
void B_DTCP_RtNonceThreadFunc(void * user)
{
    B_AkeCoreData_T * Core = (B_AkeCoreData_T *)user;

    BDBG_ASSERT(user);
    BDBG_ENTER(B_DTCP_RtNonceThreadFunc);

    BDBG_MSG(("Starting realtime nonce thread\n"));
    /* Create initial nonce */
    #if 0
    if(B_RNG(Core->hBcrypt, &Core->RealTimeNonce[0], DTCP_CONTENT_KEY_NONCE_SIZE) != BERR_SUCCESS)
    #else
    if(B_RNG(Core->hBcrypt, &Core->RealTimeNonce[0], DTCP_CONTENT_KEY_NONCE_SIZE) != BERR_SUCCESS)
    #endif
    {
        BDBG_ERR(("%s: failed to get realtime nonce\n", __FUNCTION__));
        return ;
    }
    while(true)
    {
        B_Thread_Sleep(1000*DTCP_RTP_NONCE_TIMER_PERIOD);
        B_DTCP_IncrementNonce(Core->hMutex, Core->RealTimeNonce);
    }
    BDBG_MSG(("stopping realtime nonce thread\n"));
    BDBG_LEAVE(B_DTCP_RtNonceThreadFunc);
}
/*! \brief function to start realtime nonce timer thread, called by source device only.
 *
 *  This function is by streaming interface, if protocol is RTP.
 */
static BERR_Code B_DTCP_StartRtNonceThread(B_AkeCoreData_T * CoreData)
{
    BERR_Code retValue = BERR_SUCCESS;
    CoreData->hRtNonceThread = B_Thread_Create("RtNonce", CoreData->RtNonceThreadFunc, (void*)CoreData, NULL);
    if(CoreData->hRtNonceThread == NULL)
    {
        BDBG_ERR(("Failed to create RtNonce thread\n"));
        retValue = BERR_OS_ERROR;
    }
    return retValue;
}

/*! \brief function to stop realtime nonce timer thread.
 */
static void B_DTCP_StopRtNonceThread(B_AkeCoreData_T * CoreData)
{
    if(CoreData->hRtNonceThread != NULL)
        B_Thread_Destroy(CoreData->hRtNonceThread);
    BDBG_MSG(("realtime nonce thread stopped\n"));
}
#endif
/*! \brief thread function for exchange key timer scheduler.
 *  \param[in] data pointer to the scheduler.
 */
void B_DTCP_ExchKeyThreadFunc(void * data)
{
    BDBG_ASSERT(data);
    BDBG_MSG(("Starting ExchKey scheduler... 0x%08x\n", (unsigned)data));
    B_Scheduler_Run((B_SchedulerHandle)data);
    BDBG_MSG(("Scheduler 0x%08x terminated\n", (unsigned)data));
}
/*! \brief initialize AKE core data.
 *  \param[in] Mode device mode, source or sink.
 *  \retval pointer to the AKE core data, or NULL if failed.
 */
#if 0
B_AkeCoreData_T * B_DTCP_AkeCoreInit( B_DeviceMode_T Mode)
#else
B_AkeCoreData_T * B_DTCP_AkeCoreInit(B_DeviceParams_T * DeviceParams)
#endif
{
    B_AkeCoreData_T * CoreData = NULL;

    BDBG_ENTER(B_DTCP_AkeCoreInit);
    if((CoreData = BKNI_Malloc(sizeof(B_AkeCoreData_T))) == NULL)
    {
        BDBG_ERR(("Malloc failed for AKE Core data\n"));
        return NULL;
    }

    BLST_S_INIT(&CoreData->AkeSession_list);
    BLST_S_INIT(&CoreData->AuthDevice_list);

    CoreData->AuthDeviceCount = 0;
    CoreData->AkeLabelCounter = 0;
    #if 0 /* This code moved to SAGE */
    if(BCRYPT_Open(&(CoreData->hBcrypt)) != BERR_SUCCESS)
    {
        BDBG_ERR(("Failed to open bcrypt\n"));
        goto error4;
    }
    #else
    CoreData->pDeviceParams = DeviceParams;
    #endif
    if( (CoreData->hMutex = B_Mutex_Create(NULL)) == NULL)
    {
        BDBG_ERR(("Failed to create mutex for AKE Core\n"));
        goto error3;
    }
    #if 0
    CoreData->RtNonceThreadFunc = &B_DTCP_RtNonceThreadFunc;
    #else
    CoreData->RtNonceThreadFunc = NULL;
    #endif
    CoreData->hRtNonceThread = NULL;
    CoreData->pProtocolData = NULL;
    CoreData->ExchKeyData.Initialized = false;

    #if 0
    if(B_RNG(CoreData->hBcrypt, &CoreData->RealTimeNonce[0], DTCP_CONTENT_KEY_NONCE_SIZE) != BERR_SUCCESS)
    #else
    if(B_RNG(DeviceParams->hDtcpIpTl, &CoreData->RealTimeNonce[0], DTCP_CONTENT_KEY_NONCE_SIZE) != BERR_SUCCESS)
    #endif
    {
        BDBG_ERR(("Failed to generate initial value of realtime nonce \n"));
        goto error2;
    }

    #if 0
    if(B_DTCP_InitExchKeyData(CoreData->hBcrypt, &CoreData->ExchKeyData) != BERR_SUCCESS)
    #else
    if(B_DTCP_InitExchKeyData(DeviceParams, &CoreData->ExchKeyData) != BERR_SUCCESS)
    #endif
    {
        BDBG_ERR(("Failed to initialize exchange key data\n"));
        goto error2;
    }

#ifdef DTCP_IP_RTP_SUPPORT
    if(IS_SOURCE(DeviceParams->Mode) && (retValue = B_DTCP_StartRtNonceThread(CoreData)) != BERR_SUCCESS)
    {
        goto error2;
    }
#else
    (void)DeviceParams->Mode;
#endif
    /* Exchange key timer related init */
    CoreData->ExchKeyThreadFunc = &B_DTCP_ExchKeyThreadFunc;
    CoreData->hExchKeyThread = NULL;
    CoreData->ExchKeyTimer = NULL;
    CoreData->ckc_check = false;
    CoreData->dump = false;

    if((CoreData->hExchKeyScheduler = B_Scheduler_Create(NULL)) == NULL)
    {
        BDBG_ERR(("Failed to create exchange key scheduler\n"));
        goto error1;
    }

    CoreData->hExchKeyThread = B_Thread_Create("ExchKeyScheduler", CoreData->ExchKeyThreadFunc, (void*)CoreData->hExchKeyScheduler, NULL);
    if(CoreData->hExchKeyThread == NULL)
    {
        BDBG_ERR(("Failed to create exchange key thread\n"));
        goto error0;
    }
    if(IS_SOURCE(DeviceParams->Mode))
    {
        if(B_DTCP_StartExchKeyTimer(NULL, CoreData, B_DeviceMode_eSource) != BERR_SUCCESS)
        {
            BDBG_ERR(("Failed to start exchange key timer\n"));
            goto error;
        }
    }
    BDBG_LEAVE(B_DTCP_AkeCoreInit);

    return CoreData;
error:
    B_Thread_Destroy(CoreData->hExchKeyThread);
error0:
    B_Scheduler_Destroy(CoreData->hExchKeyScheduler);

error1:
#ifdef DTCP_IP_RTP_SUPPORT
    if (IS_SOURCE(DeviceParams->Mode))
        B_DTCP_StopRtNonceThread(CoreData);
#endif

error2:
    B_Mutex_Destroy(CoreData->hMutex);
error3:
    BKNI_Free(CoreData);
    BDBG_LEAVE(B_DTCP_AkeCoreInit);
    return NULL;
}
/*! \brief clean up Ake core data.
 *  \param[in] CoreData pointer to AKE core data.
 *  There must be no active AKE sessions before calling this function, e.g. CoreData->AkeSession_list is empty.
 */
void B_DTCP_AkeCore_UnInit(B_AkeCoreData_T * CoreData)
{
    BDBG_ENTER(B_DTCP_AkeCoreCleanup);
    BDBG_ASSERT(CoreData);
#ifdef DTCP_IP_RTP_SUPPORT
    B_DTCP_StopRtNonceThread(CoreData);
#endif
    if(CoreData->pProtocolData != NULL)
    {
        BDBG_WRN(("protocol data 0x%08x  hasn't been released yet?\n", CoreData->pProtocolData));
    }
    B_Scheduler_Stop(CoreData->hExchKeyScheduler);
    B_Scheduler_Destroy(CoreData->hExchKeyScheduler);
    B_Thread_Destroy(CoreData->hExchKeyThread);
    B_Mutex_Destroy(CoreData->hMutex);
    #if 0
    BCRYPT_Close(CoreData->hBcrypt);
    #endif

    BKNI_Free(CoreData);
    BDBG_LEAVE(B_DTCP_AkeCoreCleanup);
}

/*! \brief open an AKE session, initialize session data.
 *  \param[in] CoreData  AKE core data poiner.
 *  \param[in] AkeType type of the AKE , restricted, full, etc.
 *  \param[in] DeviceMode  source or sink device.
 *  \param[in,out] akeHandle returned AKE session handle, if success.
 */
BERR_Code B_DTCP_CreateAkeSession(B_AkeCoreData_T * CoreData,
        int AkeType,
        B_DeviceMode_T DeviceMode,
        B_AkeCoreSessionData_T ** akeHandle)
{
    B_AkeCoreSessionData_T * Session = NULL;
    BERR_Code retValue = BERR_SUCCESS;

    /* sanity check */
    BDBG_ASSERT(CoreData);
    BDBG_ASSERT(akeHandle);

    BDBG_ENTER(B_DTCP_CreateAkeSession);
    Session = BKNI_Malloc(sizeof(B_AkeCoreSessionData_T));
    if(Session == NULL){
        return B_ERROR_OUT_OF_MEMORY;
    }

    BKNI_Memset(Session, 0, sizeof(B_AkeCoreSessionData_T));
    BLST_S_INIT(&Session->Stream_list);

    Session->AkeType = AkeType;
    /* SessioId is protocol specific, initiaize as 0 for now */
    Session->SessionId = 0;
    Session->pAkeCoreData = CoreData;
    Session->DeviceMode = DeviceMode;
    Session->ExchKeyTimer = NULL;
    /* ExchKeyType */
    Session->OtherSrmReplaceRequired = false;
    Session->OtherSrmUpdateRequired = false;
    Session->OurSrmReplaceRequired = false;
    Session->OurSrmUpdateRequired = false;
    Session->OtherDeviceRevoked = false;
    Session->Authenticated = false;
    Session->CurrentState = B_AkeState_eIdle;
    Session->CmdBuffer = NULL;
    if(IS_SINK(DeviceMode))
    {
        Session->AkeLabel = CoreData->AkeLabelCounter;
    }

    if((Session->hMutex = B_Mutex_Create(NULL)) == NULL)
    {
        BDBG_ERR(("Failed to create mutex for AKE session data\n"));
        BKNI_Free(Session);
        return BERR_OS_ERROR;
    }
    if((Session->hMutexStateMachine = B_Mutex_Create(NULL)) == NULL)
    {
        BDBG_ERR(("Failed to create mutex for AKE session data\n"));
        BKNI_Free(Session);
        return BERR_OS_ERROR;
    }
    *akeHandle = Session;

    BDBG_LEAVE(B_DTCP_CreateAkeSession);
    return retValue;
}
/*! \brief destroy an AKe session, free allocated resources.
 *  \param[in] Session AKE session data to be destroyed.
 */
void B_DTCP_DestroyAkeSession(B_AkeCoreSessionData_T * pSession)
{

    struct __b_dtcp_stream_data * stream, * tmp;
    BDBG_ASSERT(pSession);
    BDBG_ENTER(B_DTCP_DestroyAkeSession);

    B_Mutex_Lock(pSession->hMutex);
    /*TODO: Clean stream list  */
    if(pSession->pProtocolData != NULL)
    {
        BDBG_WRN(("Protocol data hasn't been released when trying to destroy AKE session: ID = %d\n",
                    pSession->SessionId));
    }

    #if 0 /* Do we want to cancel timer for Source here ? This will cancel timer for all the connected clients if one disconnects? */
    if (pSession->DeviceMode == B_DeviceMode_eSource)
    {
        B_Scheduler_CancelTimer(pSession->pAkeCoreData->hExchKeyScheduler, pSession->pAkeCoreData->ExchKeyTimer);
    }
    else
    #endif
    if (pSession->DeviceMode == B_DeviceMode_eSink && pSession->ExchKeyTimer != NULL)
    {
        BDBG_MSG(("Cancelling AKE Session Timer"));
        B_Scheduler_CancelTimer(pSession->pAkeCoreData->hExchKeyScheduler, pSession->ExchKeyTimer);
    }

    stream = BLST_S_FIRST(&(pSession->Stream_list));
    while(stream != NULL)
    {
        /* Close all open stream for this session */
        tmp = stream;
        stream = BLST_S_NEXT(stream, node);
        /* Remove reference to this session in the stream */
        tmp->AkeHandle = NULL;
        /* B_DTCP_IP_CloseStream((B_StreamHandle_T )tmp); */
        BLST_S_REMOVE(&(pSession->Stream_list), tmp, __b_dtcp_stream_data, node);
    }
    B_Mutex_Unlock(pSession->hMutex);
    B_Mutex_Destroy(pSession->hMutex);
    B_Mutex_Destroy(pSession->hMutexStateMachine);
    BKNI_Free(pSession);

    pSession = NULL;
    BDBG_LEAVE(B_DTCP_DestroyAkeSession);
}
/*! \brief remove and destroy all active AKE sessions, called by exchange key timer for source device.
 *  \param[in] pAkeCoreData core AKE data.
 *  \param[in] destroy if it's true, all sessions will be destroyed after removing from list.
 *  \retval none
 */
void B_DTCP_CleanAkeSessionList(B_AkeCoreData_T * pAkeCoreData, bool destroy)
{
    B_AkeCoreSessionData_T * iter = NULL, * tmp = NULL;
    BDBG_ASSERT(pAkeCoreData);

    BDBG_ENTER(B_DTCP_CleanAkeSessionList);
    if(!BLST_S_EMPTY(&(pAkeCoreData->AkeSession_list)))
    {
        iter = BLST_S_FIRST(&(pAkeCoreData->AkeSession_list));

        while(iter)
        {
            tmp = iter;
            iter = BLST_S_NEXT(iter, node);
            if(destroy == true) {
                BDBG_MSG(("Removing AKE session: %08x\n", tmp));
                BLST_S_REMOVE(&(pAkeCoreData->AkeSession_list), tmp, B_DTCP_AkeCoreSessionData, node);
                /*
                 * Force release protocol specific data, e.g. IpAkeSession.
                 */
                BKNI_Free(tmp->pProtocolData);
                tmp->pProtocolData = NULL;
                B_DTCP_DestroyAkeSession(tmp);
            }else if(tmp->CurrentState == B_AkeState_eCompleted)
            {
                /* Server expired exchange key, setting all live AKE sessions to idle state */
                tmp->CurrentState = B_AkeState_eIdle;
            }
        }
    }

    BDBG_LEAVE(B_DTCP_CleanAkeSessionList);
}
/*! \brief Add a session to authenticated Session list
 *  \param[in] pSession session pointer.
 *  \retval none
 */
void B_DTCP_AddSessionToList( B_AkeCoreSessionData_T * pSession )
{
    BDBG_ASSERT(pSession);

    B_Mutex_Lock(pSession->pAkeCoreData->hMutex);

    BLST_S_INSERT_HEAD(&(pSession->pAkeCoreData->AkeSession_list), pSession, node);
    pSession->pAkeCoreData->AuthDeviceCount++;

    if(pSession->pAkeCoreData->AkeLabelCounter == 0xFF)
        pSession->pAkeCoreData->AkeLabelCounter = 0;
    else
        pSession->pAkeCoreData->AkeLabelCounter++;

    B_Mutex_Unlock(pSession->pAkeCoreData->hMutex);
}
/*! \brief Remove AKE session from authenticated session list.
 *  \param[in] pSession Authenticated AKE session pointer.
 *  \retval BERR_SUCCESS or other error code.
 */
void B_DTCP_RemoveSessionFromList( B_AkeCoreSessionData_T * pSession )
{
    BDBG_ASSERT(pSession);

    B_Mutex_Lock(pSession->pAkeCoreData->hMutex);
    if (pSession->RefCnt == 1) {
        BDBG_MSG(("RefCnt is 1, removing device node"));
        BLST_S_REMOVE(&pSession->pAkeCoreData->AkeSession_list,
                      pSession, B_DTCP_AkeCoreSessionData, node);
        pSession->pAkeCoreData->AuthDeviceCount--;
    }
    else
        BDBG_MSG(("RefCnt is %d", pSession->RefCnt));
    /*
     * caller need to destroy the session
     */
    B_Mutex_Unlock(pSession->pAkeCoreData->hMutex);
}
/*! \brief check if given device is authenticated, called by source device only.
 *  \param[in] DeviceId device id to be checked.
 *  \retval true if it's in authenticated device list, false otherwise.
 */
bool B_DTCP_IsDeviceAuthenticated(B_AkeCoreData_T * CoreData, unsigned char DeviceId[DTCP_DEVICE_ID_SIZE])
{
    bool retValue = false;
    B_AkeCoreSessionData_T * iter = NULL;

    B_Mutex_Lock(CoreData->hMutex);
    iter = BLST_S_FIRST(&(CoreData->AkeSession_list));
    BDBG_MSG(("Checking session 0x%08x\n", iter));
    while(iter != NULL){
        BDBG_MSG(("Iter DeviceID: %02x %02x %02x %02x %02x\n", iter->OtherDeviceId[0], iter->OtherDeviceId[1],iter->OtherDeviceId[2], iter->OtherDeviceId[3], iter->OtherDeviceId[4]));
        BDBG_MSG(("New DeviceID:  %02x %02x %02x %02x %02x\n", DeviceId[0], DeviceId[1], DeviceId[2], DeviceId[3], DeviceId[4]));
        if (!BKNI_Memcmp(iter->OtherDeviceId, DeviceId, DTCP_DEVICE_ID_SIZE))
        {
            retValue = true;
            break;
        }
        iter = BLST_S_NEXT(iter, node);
    }
    BDBG_MSG(("valid is %s\n", retValue == true? "true":"false"));
    B_Mutex_Unlock(CoreData->hMutex);
    return retValue;
}

/*! \brief check if given device is authenticated and return the pointer to the Session entry if it exists, called by source device only.
 *  \param[in] DeviceId device id to be checked                       .
 *  \param[out] pAkeSession, entry in the list if found.                                                                  .
 *  \retval true if it's in authenticated device list, false otherwise.
 */
bool B_DTCP_GetSessionEntryForDeviceId(B_AkeCoreData_T * CoreData, unsigned char DeviceId[DTCP_DEVICE_ID_SIZE], B_AkeCoreSessionData_T **pAkeSession)
{
    bool retValue = false;
    B_AkeCoreSessionData_T * iter = NULL;

    B_Mutex_Lock(CoreData->hMutex);
    iter = BLST_S_FIRST(&(CoreData->AkeSession_list));
    BDBG_MSG(("Checking session 0x%08x\n", iter));
    while(iter != NULL){
        if (!BKNI_Memcmp(iter->OtherDeviceId, DeviceId, DTCP_DEVICE_ID_SIZE))
        {
            retValue = true;
            *pAkeSession = iter;
            break;
        }
        iter = BLST_S_NEXT(iter, node);
    }
    BDBG_MSG(("valid is %s\n", retValue == true? "true":"false"));
    B_Mutex_Unlock(CoreData->hMutex);
    return retValue;
}


/*! \brief timer callback funcion used by source device to expire/update current exchange key.
 *  \param[in] data pointer to AKE core data.
 */
static void B_DTCP_AkeUpdateSourceExchKeys(void * data)
{
    B_AkeCoreData_T * CoreData = (B_AkeCoreData_T *)data;
    B_AkeCoreSessionData_T * Session;
    struct __b_dtcp_stream_data * stream;
    int KxExpire = KX_EXPIRE;

/* only for testing purposes. reduce the exchange key timer to 2 minutes instead of 2 hours*/
#ifdef BDBG_DEBUG_BUILD
    if(getenv("EXCH_KEY_DEBUG_MODE"))
    {
        KxExpire = 120;
    }
#endif

    BDBG_ASSERT(data);

    /*
     * Check if there are any openning stream with PCP transfer in progress, this would only happen
     * if client paused the stream playback for long time, we can't update exchange key when any session
     * has a PCP transfer in progress!
     */
    Session = BLST_S_FIRST(&(CoreData->AkeSession_list));
    while(Session != NULL)
    {
        stream = BLST_S_FIRST(&(Session->Stream_list));
        while(stream != NULL)
        {
            if(stream->hPacketHandle != NULL)
            {
                BDBG_WRN(("Timer is expiring but we are in middle of PCP so not updating keys. Resetting timer\n"));
                CoreData->ExchKeyTimer = B_Scheduler_StartTimer(CoreData->hExchKeyScheduler,
                    CoreData->hMutex, 1000*KxExpire, B_DTCP_AkeUpdateSourceExchKeys, (void*)CoreData);
                if(CoreData->ExchKeyTimer == NULL)
                {
                    BDBG_ERR(("Failed to start exchange key timer for source\n"));
                }
                return;
            }
            stream = BLST_S_NEXT(stream, node);
        }
        Session = BLST_S_NEXT(Session, node);
    }

    BDBG_WRN(("Updating source exchange keys!\n"));
    if(B_DTCP_UpdateSourceExchKeys(CoreData->pDeviceParams, &(CoreData->ExchKeyData)) == BERR_SUCCESS)
    {
        /* Remove all authenticaed sessions from list
         * note that sessions are not destroyed, still active but need to re-authenticated.
         */
        B_DTCP_CleanAkeSessionList(CoreData, false);
        CoreData->AuthDeviceCount = 0;
    }

    /* Restart the exchange key timer */
    CoreData->ExchKeyTimer = B_Scheduler_StartTimer(CoreData->hExchKeyScheduler,
        CoreData->hMutex, 1000*KxExpire, B_DTCP_AkeUpdateSourceExchKeys, (void*)CoreData);
    if(CoreData->ExchKeyTimer == NULL)
    {
        BDBG_ERR(("Failed to start exchange key timer for source\n"));
    }
}
/*! \brief timer callback used by sink device to invalidte exchange key.
 *  \param[in] data pointer to AKE session data.
 */
static void B_DTCP_AkeInvalidateSinkExchKeys(void * data)
{

    B_AkeCoreSessionData_T * Session = (B_AkeCoreSessionData_T *)data;
    struct __b_dtcp_stream_data * stream;
    int KxExpire = KX_EXPIRE;

/* only for testing purposes. reduce the exchange key timer to 2 minutes instead of 2 hours*/
#ifdef BDBG_DEBUG_BUILD
    if(getenv("EXCH_KEY_DEBUG_MODE"))
    {
        KxExpire = 120;
    }
#endif
    BDBG_ASSERT(Session);
    /*
     * If there any openning stream with PCP transfer in progress , we can't invalid exchange key!
     */

    stream = BLST_S_FIRST(&(Session->Stream_list));
    while(stream != NULL)
    {
        if(stream->hPacketHandle != NULL)
        {
            BDBG_WRN(("Rescheduling the Exchange Key Timer instead of invalidating the keys. Possible reason could be stream is PAUSED"));
            Session->ExchKeyTimer = B_Scheduler_StartTimer(Session->pAkeCoreData->hExchKeyScheduler,
                Session->hMutex, 1000*KxExpire, B_DTCP_AkeInvalidateSinkExchKeys, (void*)Session);
            if(Session->ExchKeyTimer == NULL)
            {
                BDBG_ERR(("Failed to start exchange key timer for source\n"));
            }
            return;
        }
        stream = BLST_S_NEXT(stream, node);
    }
    BDBG_WRN(("Invalidating sink exchange key!\n"));
    Session->ExchKeyData.Valid = 0;
    /* reschedule timer !*/
    Session->ExchKeyTimer = B_Scheduler_StartTimer(Session->pAkeCoreData->hExchKeyScheduler,
        Session->hMutex, 1000*KxExpire, B_DTCP_AkeInvalidateSinkExchKeys, (void*)Session);

    if(Session->ExchKeyTimer == NULL)
    {
        BDBG_ERR(("%s: Failed to start timer!\n"));
    }

}
/*! \brief start exchange key update/invalidate timer
 *
 *  For source device, this function is called after AKE core data is initialized, and will be
 *  called everytime exiting from PacketizeData function to reschedule the timer.
 *
 *  For sink device, this function will be called after AKE is done, and will be called everytime
 *  exiting from DepacketizeData function to reschedule timer.
 *
 *  \param[in] CoreData AKE core Data pointer.
 *  \param[in] Session AKE session data, if called by source device, it can be NULL.
 *  \param[in] Mode source or sink device?
 *  \retval BERR_SUCCESS or other error code.
 */
BERR_Code B_DTCP_StartExchKeyTimer(B_AkeCoreSessionData_T * Session, B_AkeCoreData_T * CoreData, B_DeviceMode_T Mode)
{
    int KxExpire = KX_EXPIRE;

/* only for testing purposes. reduce the exchange key timer to 2 minutes instead of 2 hours*/
#ifdef BDBG_DEBUG_BUILD
    if(getenv("EXCH_KEY_DEBUG_MODE"))
    {
        KxExpire = 120;
    }
#endif

    BDBG_ASSERT(CoreData);
    BDBG_ENTER(B_DTCP_StartExchKeyTimer);

    if(IS_SOURCE(Mode))
    {
        /* lock mutex?*/
        if(CoreData->ExchKeyTimer != NULL)
            B_Scheduler_CancelTimer(CoreData->hExchKeyScheduler, CoreData->ExchKeyTimer);

        CoreData->ExchKeyTimer = B_Scheduler_StartTimer(CoreData->hExchKeyScheduler,
            CoreData->hMutex, 1000*KxExpire, B_DTCP_AkeUpdateSourceExchKeys, (void*)CoreData);
        if(CoreData->ExchKeyTimer == NULL)
        {
            BDBG_ERR(("Failed to start exch key timer for source device!\n"));
            return BERR_OS_ERROR;
        }
    }else {
        BDBG_ASSERT(Session);
        if(Session->ExchKeyTimer != NULL)
            B_Scheduler_CancelTimer(CoreData->hExchKeyScheduler, Session->ExchKeyTimer);
        Session->ExchKeyTimer = B_Scheduler_StartTimer(CoreData->hExchKeyScheduler,
            Session->hMutex, 1000*KxExpire, B_DTCP_AkeInvalidateSinkExchKeys, (void*)Session);
        if(Session->ExchKeyTimer == NULL)
        {
            BDBG_ERR(("Failed to start exch key timer for sink device\n"));
            return BERR_OS_ERROR;
        }
    }
    BDBG_LEAVE(B_DTCP_StartExchKeyTimer);
    return BERR_SUCCESS;
}

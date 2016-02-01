/********************************************************************************************
*     (c)2004-2013 Broadcom Corporation                                                     *
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
 *    DTCP AKE core data
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *********************************************************************************************/
/*! \file b_dtcp_ake.h
 *  \brief define AKE core session data structure.
 */
#ifndef B_DTCP_AKE_H
#define B_DTCP_AKE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "b_os_lib.h"
#include "blst_slist.h"
#include "b_dtcp_constants.h"
#include "b_dtcp_types.h"
#include "b_dtcp_exch_key.h"
#include "bcrypt.h"

#define DTCP_CAPABILITY_NB_MASK			0x1
#define DTCP_CAPABILITY_CIH_MASK		0x1
/* #define DTCP_AKE_CMD_TIMEOUT			40000 */
#define DTCP_AKE_CMD_TIMEOUT			  5000

/* \skipline struct B_DTCP_AkeCoreSessionData
 */
struct B_DTCP_AkeCoreSessionData;

/*! \enum B_CType
 *
 */
typedef enum B_CType
{
	B_CType_eControl = 0,
	B_CType_eStatus = 1,
	B_CType_eSpecificInquiry = 2,
	B_CType_eNotify = 3,
	B_CType_eGeneralInquiry = 4
}B_CType_T;
/*! \enum B_AkeResponse
 */
typedef enum B_AkeResponse
{
	B_Response_eNotImplemented = 0x8,
	B_Response_eAccepted = 0x9,
	B_Response_eRejected = 0xA,
	B_Response_eInTransition = 0xB,
	B_Response_eStable = 0xC,
	B_Response_eChanged = 0xD,
	B_Response_eInterim = 0xF
}B_AkeResponse_T;

/*! \enum B_AkeCommands
 */
typedef enum B_AkeCommands
{
	B_AkeCmd_eChallenge = 1,
	B_AkeCmd_eResponse = 2,
	B_AkeCmd_eExchangeKey = 3,
	B_AkeCmd_eSRM = 4,
	B_AkeCmd_eResponse2 = 5,
	B_AkeCmd_eCapabilityExchange = 0x20,
	B_AkeCmd_eAkeCancel = 0xC0,
	B_AkeCmd_eContentKeyReq = 0x80,
	B_AkeCmd_eSetDtcpMode = 0x81,
	B_AkeCmd_eCapabilityReq = 0x82,
	B_AkeCmd_eStatus = 0xFF
}B_AkeCommands_T;


/*! \enum B_AkeStatus
 */
typedef enum B_AkeStatus
{
	B_AkeStatus_eOK = 0,
	B_AkeStatus_eNoMoreAuth,
	B_AkeStatus_eNoIso,
	B_AkeStatus_eNoP2P,
	B_AkeStatus_eNoAC,
	B_AkeStatus_eOther = 0x7,
	B_AkeStatus_eIncorrectCmdOrder = 0x8,
	B_AkeStatus_eAuthFailed = 0x9,
	B_AkeStatus_eSyntexError = 0xA,
	B_AkeStatus_eNoInfo = 0xF
}B_AkeStatus_T;

/*! \enum B_AkeState
 */
typedef enum B_AkeState
{
	B_AkeState_eIdle,
	B_AkeState_eChallenge,
	B_AkeState_eResponse,
	B_AkeState_eResponse2,
	B_AkeState_eExchangeKey,
	B_AkeState_eSrm,
	B_AkeState_eAuthenticated,
	B_AkeState_eCanceled,
	B_AkeState_eError,
	B_AkeState_eCompleted
}B_AkeState_T;

/*! \struct B_DTCP_AuthDeviceId
 *  \brief Authenticated device id struct wrapper, for linked list operation
 */
typedef struct B_DTCP_AuthDeviceId
{
	BLST_S_ENTRY(B_DTCP_AuthDeviceId) node;			/*!< list node. */
	unsigned char DeviceId[DTCP_DEVICE_ID_SIZE];	/*!< Authenticated device id */
}B_DTCP_AuthDeviceId_T;

/*! \struct B_DTCP_AkeCoreData
 *  \brief core data structure for source device to maintain AKE session for sink devices.
 */
typedef struct B_DTCP_AkeCoreData
{
	BLST_S_HEAD(DtcpAkeSessions, B_DTCP_AkeCoreSessionData) AkeSession_list;/*!< active AKE sessions */
	BLST_S_HEAD(AuthDeviceIds, B_DTCP_AuthDeviceId) AuthDevice_list;		/*!< dev's id list of authenticated sink device*/
	B_DTCP_ExchKeyData_T ExchKeyData;										/*!< Source device's exchange key data. */
	int AuthDeviceCount;													/*!< count of authenticated sink device */
	unsigned char RealTimeNonce[DTCP_CONTENT_KEY_NONCE_SIZE];				/*!< source device's content key nonce */
	B_ThreadHandle hRtNonceThread;											/*!< realtime nonce timer thread handle */
	B_ThreadFunc   RtNonceThreadFunc;										/*!< realtime nonce timer thread function.*/

	B_SchedulerHandle hExchKeyScheduler;									/*!< scheduler for exchange key timer */
	B_SchedulerTimerId ExchKeyTimer;										/*!< source device's exchange key timer id */
	B_ThreadHandle hExchKeyThread;											/*!< exchange key timer scheduler thread handle */
	B_ThreadFunc   ExchKeyThreadFunc;										/*!< exchange key timer scheduler thread function */
	B_MutexHandle hMutex;													/*!< mutex to protect shared data */
	BCRYPT_Handle hBcrypt;
	bool ckc_check;															/*!< if content key confirmation procedure is enabled */
	bool dump;																/*!< dump more debug messages */

	unsigned char AkeLabelCounter;											/*!< source device's AKE label counter.*/
	void * pProtocolData;													/*!< Protocol specific data pointer */
	void * pStack;                                                /*!< Context handle for the DTCP stack>*/
}B_AkeCoreData_T;

/* \skipline struct __b_dtcp_stream_data
 */
struct __b_dtcp_stream_data;

#define PCP_UR_SINK		(1 << 31)
#define PCP_UR_FLAG		1
/*! \struct B_DTCP_AkeCoreSessionData
 *  \brief Ake Core session data structure(per session data).
 */
typedef struct B_DTCP_AkeCoreSessionData
{
	unsigned int RefCnt;				            /*! < Session reference count */
	BLST_S_ENTRY(B_DTCP_AkeCoreSessionData) node;
	int SessionId;									/*!< session id */
	int origSessionId;								/*!< session id corresponding to the original AKE session */
	B_AkeCoreData_T * pAkeCoreData;					/*!< Ake Core data, shared among all sessions */
	B_DeviceMode_T	  DeviceMode;					/*!< sink or source device*/
	B_AkeType_T AkeType;							/*!< the type of AKE being performed */
	B_ExchangeKeyType_T  ExchKeyType;				/*!< the type of exchange key */
	B_DTCP_ExchKeyData_T ExchKeyData;				/*!< exchange key for this AKE */
	unsigned char AkeLabel;							/*!< Ake label for this session */
	B_SchedulerTimerId   ExchKeyTimer;				/*!< sink device exchange key timer id */
	unsigned char SourceExchKeyLabel;				/*!< source device's most recent exchange key label */
	unsigned char SourceRTNonce[DTCP_CONTENT_KEY_NONCE_SIZE];	/*!< Source device's most recent realtime nonce */

	unsigned char OtherDeviceId[DTCP_DEVICE_ID_SIZE];	/*!< device id of the other device */
	unsigned char Nonce[DTCP_FULL_AUTH_NONCE_SIZE];		/*!< our nonce */
	int NonceSize;										/*!< our nonce size */
	unsigned char OtherNonce[DTCP_FULL_AUTH_NONCE_SIZE];		/*!< Other device's nonce */
	int OtherNonceSize;											/*!< other device's nonce size */
	unsigned char OtherPublicKey[DTCP_PUBLIC_KEY_SIZE];			/*!< other device's public key*/
	int OtherKsv;									/*!< other device's key selction vector */
	unsigned short OtherSrmG;						/*!< other device's Supported SRM generation*/
	unsigned short OtherSrmV;						/*!< other device's SRM version */
	unsigned short OtherSrmC;						/*!< other device's SRM Current Generation */
	int OtherAP;									/*!< other device's AP flag */
	int OtherAL;									/*!< other device's AL flag */
	bool OtherSrmReplaceRequired;					/*!< replace other device's SRM with our SRM. */
	bool OtherSrmUpdateRequired;					/*!< update other device's SRM upto its max supported geneeration. */
	bool OurSrmReplaceRequired;					/*!< replace our SRM needed. */
	bool OurSrmUpdateRequired;					/*!< update our SRM upto its max supported geneeration. */
	bool OtherDeviceRevoked;						/*!< The sink device is revoked. */
	unsigned char FirstPhaseValue[DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE];	/*!< our ec-dh first phase value */
	unsigned char FirstPhaseSecret[DTCP_DH_FIRST_PHASE_SECRET_SIZE];	/*!< our ec-dh first phase secret*/
	unsigned char OtherFirstPhaseValue[DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE];
	unsigned char OtherFirstPhaseSecret[DTCP_DH_FIRST_PHASE_SECRET_SIZE];
	unsigned char AuthKey[DTCP_AUTH_KEY_SIZE];							/*!< derived Authentication key */
	unsigned char RealTimeNonce[DTCP_CONTENT_KEY_NONCE_SIZE];			/* sink device received content key nonce */
	unsigned int sink_cap;							/*!< sink capability */
	unsigned int pcp_ur_cap;						/*!< source device support pcp_ur or not */
	unsigned char CapabilityExchSignature[DTCP_SIGNATURE_SIZE];	/* Capability exchange message signature */
	bool Authenticated;								/*!< flag to indicated if the AKE succeedded or not*/
	unsigned char * CmdBuffer;						/*!< Ake command buffer */
	int CmdBufferSize;								/*!< command buffer size */
	int CurrentState;								/*!< current AKE state */
	B_MutexHandle hMutex;							/*!< Mutex to protect shared data.*/
	B_MutexHandle hMutexStateMachine;				/*!< Mutex to protect shared AkeSession during AKE state machine.*/

	/* Time out checking variables */
	B_Time ChallengeSentTime;
	B_Time ChallengeReceivedTime;
	B_Time ResponseSentTime;
	B_Time ResponseReceivedTime;
	B_Time ExchKeySentTime;
	B_Time ExchKeyReceivedTime;
	B_Time SrmReceivedTime;

	BLST_S_HEAD(streams, __b_dtcp_stream_data) Stream_list;		/*!< active streams */

	void * pProtocolData;							/*!< protocol specific data pointer */

}B_AkeCoreSessionData_T;

/* \brief For interface with streaming interface
 */
typedef void * B_AkeHandle_T;

/* --------------------------------------Function prototypes ----------------------------------------*/

/*! \brief utility to get Ake Type based on device's parameter.
 *  \param[out] pAkeType AKE type.
 *  \param[in]  pDeviceParams device parameter pointer.
 *  \retval BERR_SUCCESS or other error code.
 */
BERR_Code B_DTCP_GetAkeTypeFromCertificate(B_AkeType_T * pAkeType, B_DeviceParams_T * pDeviceParams);

/*! \brief allocate and initialize AKE core data.
 *  \param[in] Mode the device mode, source or sink.
 *  \retval pointer to the core data if success or NULL if failed.
 */
B_AkeCoreData_T * B_DTCP_AkeCoreInit( B_DeviceMode_T Mode);

/*! \brief clean up Ake core data.
 *  \param[in] CoreData pointer to AKE core data.
 *  \retval BERR_SUCCESS or other error code.
 *  There must be no active AKE sessions before calling this function, e.g. CoreData->AkeSession_list is empty.
 */
void B_DTCP_AkeCore_UnInit(B_AkeCoreData_T * CoreData);

/*! \brief open an AKE session, initialize session data.
 *  \param[in] CoreData  AKE core data poiner.
 *  \param[in] AkeType type of the AKE , restricted, full, etc.
 *  \param[in] DeviceMode  source or sink device.
 *  \param[in,out] akeHandle returned AKE session handle, if success.
 *  \retval BERR_SUCCESS or other error code.
 */
BERR_Code B_DTCP_CreateAkeSession(B_AkeCoreData_T * CoreData, int AkeType, B_DeviceMode_T DeviceMode,
		B_AkeCoreSessionData_T ** akeHandle);

/*! \brief destroy an AKe session, free allocted resources.
 *  \param[in] pSession AKE session data to be destroyed.
 *  \retval none.
 */
void B_DTCP_DestroyAkeSession(B_AkeCoreSessionData_T * pSession);

/*! \brief increase a 64 bits nonce(Number used once) by 1, called by content management and conteng key confirmation functions.
 *  \param[in] hMutex session's mutex handle.
 *  \param[in,out] nonce , nonce value to operate on.
 *  This function assume for 8 bytes nonce (content key nonce).
 */
void B_DTCP_IncrementNonce(B_MutexHandle hMutex, unsigned char nonce[8]);

/*! \brief compare two 64 bits nonce(Number used once ) value
 *  \param[in] anonce first value to compare.
 *  \param[in] bnonce second value to compare.
 *  \retval the difference between two nonce, in 32 bits number.
 *
 *  This implementation assume the diff is within 32 bits long.
 */
long B_DTCP_GetNonceDiff(unsigned char anonce[8], unsigned char bnonce[8]);

/*! \brief utility function to get or set realtime nonce value from core data.
 *  \param[in] hMutex mutex to protect shared data.
 *  \param[in] RtNonceIn pointer contain the Nonce to be copied.
 *  \param[out] RtNonceOut pointer contain output buffer.
 *  \retval none.
 *
 *  If hMutex is not NULL, it will be locked during the operation.
 */
void B_DTCP_GetSetRealTimeNonce(B_MutexHandle hMutex, const unsigned char RtNonceIn[DTCP_CONTENT_KEY_NONCE_SIZE],
		unsigned char RtNonceOut[DTCP_CONTENT_KEY_NONCE_SIZE]);

/*! \brief Add a session to authenticated Session list
 *  \param[in] pSession session pointer.
 *  \retval none
 */
void B_DTCP_AddSessionToList( B_AkeCoreSessionData_T * pSession );

/*! \brief Remove AKE session from authenticated session list.
 *  \param[in] pSession Authenticated AKE session pointer.
 *  \retval BERR_SUCCESS or other error code.
 */
void B_DTCP_RemoveSessionFromList( B_AkeCoreSessionData_T * pSession );
/*! \brief remove and clean all active AKE sessions, for source device.
 *  \param[in] pAkeCoreData core AKE data.
 *  \retval none
 */
void B_DTCP_CleanAkeSessionList(B_AkeCoreData_T * pAkeCoreData, bool destroy);

/*! \brief check if given device is authenticated, called by source device only.
 *  \param[in] DeviceId device id to be checked.
 *  \retval true if it's in authenticated device list, false otherwise.
 */
bool B_DTCP_IsDeviceAuthenticated(B_AkeCoreData_T * CoreData, unsigned char DeviceId[DTCP_DEVICE_ID_SIZE]);

/*! \brief check if given device is authenticated and return the pointer to the Session entry if it exists, called by source device only.
 *  \param[in] DeviceId device id to be checked 					  .
 *  \param[out] pAkeSession, entry in the list if found.																  .
 *  \retval true if it's in authenticated device list, false otherwise.
 */

bool B_DTCP_GetSessionEntryForDeviceId(B_AkeCoreData_T * CoreData, unsigned char DeviceId[DTCP_DEVICE_ID_SIZE], B_AkeCoreSessionData_T **pAkeSession);

/*! \brief check if Addional Localization is required.
 *  \param[in] DeviceParams device parameter.
 *  \param[in] Session Ake session pointer.
 *  \retval true if AL required, false otherwise.
 */
bool B_DTCP_IsALRequired(B_DeviceParams_T * DeviceParams, B_AkeCoreSessionData_T * Session);

/*! \brief start exchange key update/invalidate timer
 *
 *  For source device, this function is called after AKE core data is initialized, and will be
 *  called everytime exiting from PacketizeData function to reschedule the timer.
 *
 *  For sink device, this function will be called after AKE is done, and will be called everytime
 *  exiting from DepacketizeData function to reschedule timer.
 *
 *  \param[in] AkeCore AKE core Data pointer.
 *  \param[in] hAkeHandle AKE session data handle, if called by source device, it can be NULL.
 *  \param[in] Mode source or sink device?
 *  \retval BERR_SUCCESS or other error code.
 */
BERR_Code B_DTCP_StartExchKeyTimer(B_AkeCoreSessionData_T * Session, B_AkeCoreData_T * CoreData, B_DeviceMode_T Mode);

#ifdef BDBG_DEBUG_BUILD
void B_DTCP_DebugBuff(unsigned char * buffer, int size);
#define BDBG_BUFF(buff, size)	B_DTCP_DebugBuff(buff, size)
#else
#define BDBG_BUFF(buff, size)
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* B_DTCP_AKE_H */

/********************************************************************************************
*     (c)2004-2012 Broadcom Corporation                                                     *
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
 *    IP AKE Core session data definations and interfaces.
 * 
 * Revision History:
 * 
 * $brcm_Log: $
 * 
 *********************************************************************************************/
/*! \file b_dtcp_ip_ake.h
 *  \brief define IP-AKE core session data structure.
 */
#ifndef B_DTCP_IP_AKE_H
#define B_DTCP_IP_AKE_H   

#include "b_dtcp_ake.h" 

/*! \enum B_IpAkeCommands
 */
typedef enum B_IpAkeCommands
{
	B_IpAkeCmd_eRttReady = 0x91,
	B_IpAkeCmd_eRttSetup = 0x11,
	B_IpAkeCmd_eRttTest = 0x12,
	B_IpAkeCmd_eContKeyConf = 0x13,
	B_IpAkeCmd_eRttVerify = 0x92,
	B_IpAkeCmd_eBgRttInitiate = 0x90,
	B_IpAkeCmd_eMvInitiate = 0xA0,
	B_IpAkeCmd_eMvExchKey = 0x21,
	B_IpAkeCmd_eMvCancel = 0x28,
	B_IpAkeCmd_eMvFinalize = 0x22,
	B_IpAkeCmd_eMvComplete = 0x23,
	B_IpAkeCmd_eMvContKeyConf = 0x24
}B_IpAkeCommands_T;

/*! \enum B_IpAkeState
 */
typedef enum B_IpAkeState
{
	B_AkeState_eRttReady = B_AkeState_eCompleted + 1,	/* enum start from Base AKe state */
	B_AkeState_eRttSetup,
	B_AkeState_eRttTest,
	B_AkeState_eRttVerify,
	B_AkeState_eRttCompleted,
	B_AkeState_eContKeyConf,
	B_AkeState_eRttBg,
	B_AkeState_eMax
}B_IpAkeState_T;

/*! \struct __b_rtt_registry_data
 *  \brief DTCP-IP Rtt registry data, for linked list manipulation.
 */
typedef struct __b_rtt_registry_data
{
	BLST_S_ENTRY(__b_rtt_registry_data) node;
	unsigned char DeviceId[DTCP_DEVICE_ID_SIZE];
	unsigned int RttTimer;
}B_RttRegistryData_T;

/*! \struct B_IpAkeCoreData
 *  \brief DTCP-IP protocol specific AKE core data.
 */
typedef struct B_IpAkeCoreData
{
	BLST_S_HEAD(RttRegistries, __b_rtt_registry_data) RttRegistry_list;
	void * reserved;
}B_IpAkeCoreData_T;

/*! \struct __b_dtcp_ip_ake_session_data
 *
 *  DTCP-IP protocol specific AKE sessioin data
 */
typedef struct __b_dtcp_ip_ake_session_data
{
	char LocalIp[MAX_IP_ADDR_SIZE];		/*! < Local device's ip address */
	unsigned int LocalPort;				/*! < Local device's port number */
	char RemoteIp[MAX_IP_ADDR_SIZE];	/*! < Remote Device's ip address. */
	unsigned int RemotePort;			/*! < Remote device's port number. */
	struct __b_dtcp_ip_ake_session_data * ContKeyConfSession; /*!< reference to content key conf session . */
	bool BgRttTest;						/*! < flag to indicate wether to do BG rtt test.*/
	/* RTT measurement variables */
	B_Time RttReadySentTime;
	B_Time RttReadyReceivedTime;
	B_Time RttSetupSentTime;
	B_Time RttSetupReceivedTime;
	B_Time RttTestSentTime;
	B_Time RttTestReceivedTime;
	B_Time RttVerifySentTime;
	B_Time RttVerifyReceivedTime;
	unsigned char ContR[DTCP_CONT_KEY_CONF_R_SIZE];		/*! < content key confirmation R value */
	unsigned char RttN[DTCP_RTT_N_SIZE];				/*! < Rtt trial counter */
	long   RttValue;									/*! < Measured RTT time */
	unsigned char RttMac[DTCP_RTT_MAC_DATA_SIZE];		/*! < our mac value. */
	unsigned char OtherRttMac[DTCP_RTT_MAC_DATA_SIZE];	/*! < other's mac value. */
	unsigned char OkMsg[DTCP_RTT_MAC_DATA_SIZE];		/*! < our OK message(Computed) */
	unsigned char OtherOkMsg[DTCP_RTT_MAC_DATA_SIZE];	/*! < other's ok message(Received) */
	unsigned char ContKeyConfMac[DTCP_CONT_KEY_CONF_MAC_DATA_SIZE];
	unsigned char OtherContKeyConfMac[DTCP_CONT_KEY_CONF_MAC_DATA_SIZE];
	unsigned char NcT[DTCP_CONTENT_KEY_NONCE_SIZE];		/*!< source device received NcT for content key confirm */
	bool ContKeyConfirmed;
	bool StartContKeyConf;
	bool TerminateCKCThread;
	int ContKeyConfCount;
	B_ThreadHandle hContKeyConfThread;

}B_IpAkeSessionData_T;
/*--------------------------------------function prototypes-------------------------------------------*/
/*! \brief Initialize IP Ake core data.
 *  \param[in] CoreData pointer to core AKE data
 *  \retval BERR_SUCCESS or other error code
 */
B_AkeCoreData_T * B_DTCP_IpAkeCoreInit(B_DeviceMode_T Mode);

/*! \brief Cleanup IP Ake core data.
 *  \param[in] CoreData pointer to core AKE data that has been initalized.
 *  \retval BERR_SUCCESS or other error code.
 */
void B_DTCP_IpAkeCore_UnInit(B_AkeCoreData_T * CoreData);

/*! \brief Create and initialize IP Ake session data.
 *  \param[in] CoreData pointer to Ake core data.
 *  \param[in] aRemoteIp For sink devie, this is source device's IP address, for source device, this can be NULL.
 *  \param[in] aRemotePort DTCP port number.
 *  \param[in] BgRttTest specify if this session should do background RTT test.
 *  \param[in] AkeType type of AKE to be performed, full/restricted/enhanced full, etc.
 *  \param[in] DeviceMode Source or Sink device.
 *  \param[out] pSession pointer to newly created AKE session data pointer.
 *  \retval BERR_SUCCESS or other error code, if failed dereference pSession will be NULL.
 */
BERR_Code B_DTCP_CreateIpAkeSession(B_AkeCoreData_T * CoreData,
		const char * aRemoteIp, 
		unsigned short aRemotePort, 
		bool BgRttTest,
		B_AkeType_T AkeType,
		B_DeviceMode_T DeviceMode,
		B_AkeCoreSessionData_T ** pSession
		);
/*! \brief destroy IP Ake session data created by B_DTCP_CreateIpAkeSession() call
 *  \param[in] AkeHandle pointer to AkE session data
 *  \retval none
 */
void B_DTCP_DestroyIpAkeSession(void * AkeHandle);

/*! \brief helper function to increment the RTT N value (Rtt trial counter )
 *  \param[in] n RttN value to be incremented
 *  \retval none
 */
void B_DTCP_IncrementRttN(unsigned char n[2]);

/*! \brief helper function to retrive content key confirmation result
 *  \param[in] pSession AKE session pointer
 *  \retval true if the content key has been confirmed, false otherwise
 */
bool B_DTCP_IP_IsContKeyConfirmed(B_AkeCoreSessionData_T * pSession);

#endif /* B_DTCP_IP_AKE_H */

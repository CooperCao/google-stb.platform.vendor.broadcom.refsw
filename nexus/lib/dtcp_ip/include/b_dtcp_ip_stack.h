/********************************************************************************************
*     (c)2004-2009 Broadcom Corporation                                                     *
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
 *
 * DTCP-IP stack data and interface definations.
 * Revision History:
 *
 * $brcm_Log: $
 *
*********************************************************************************************/
/*! \file b_dtcp_ip_stack.h
 *  \brief Define DTCP IP stack data structure and function prototypes.
 */
#ifndef B_DTCP_IP_STACK_H
#define B_DTCP_IP_STACK_H
#ifdef __cplusplus
# extern "C" {
#endif
/*! \struct __dtcp_ip_stack_data
 *  \brief protocol specific data structure.
 */
struct __dtcp_ip_stack_data
{
	int ConnectSocket;					/* Connected socket fd, only used for server */
	char LocalIp[MAX_IP_ADDR_SIZE];
	unsigned int LocalPort;
	char RemoteIp[MAX_IP_ADDR_SIZE];
	unsigned int RemotePort;
};

/*! \brief initialize DTCP-IP stack context.
 *  \param[in] DeviceParams pointer to device parameter data.
 *  \param[in] UpdateSrm_Func  pointer to a function to update SRM.
 *  \retval handle to the DTCP stack if success or NULL if failed.
 */
B_DTCP_StackHandle_T B_DTCP_IP_Stack_Init(B_DeviceParams_T * DeviceParams, B_DTCP_UpdateSRM_Ptr UpdateSrm_Func);

/*! \brief cleanup DTCP-IP stack context.
 *  \param[in] pStack the stack handle .
 */
void B_DTCP_IP_Stack_UnInit(B_DTCP_StackHandle_T pStack);
/*! \brief perform Rtt, called by RTT-AKE procedure or Background RTT process.
 *
 *  Source device call this function for RTT-AKE after challenge-response procedure is done.
 *  or to initiate background RTT procedure.
 *  Sink device can only call this function to initiate background RTT procedure.
 *
 */
BERR_Code B_DTCP_IP_DoRtt(B_DTCP_StackHandle_T pStack, B_AkeCoreSessionData_T * pSession);

/*! \brief function perform background RTT procedure.
 *  \param[in] pStack DTCP-IP stack handle
 *  \param[in] RemoteIp source device's IP address.
 *  \param[in] RemotePort DTCP port bumber.
 *  \param[in] AkeHandle current active AKE session handle.
 *  \retval BERR_SUCCESS or other error code.
 */
BERR_Code B_DTCP_IP_DoBgRtt(B_DTCP_StackHandle_T pStack, unsigned char * RemoteIp, unsigned short RemotePort, B_AkeHandle_T * AkeHandle);

/*! \brief exported function to retrieve AKE session handle from authenticated session list.
 *  \param[in] pStack DTCP-IP stack handle
 *  \param[in] RemoteIp sink device's IP address.
 *  \param[in,out] AkeHandle pointer to AKE session data.
 *  \param[in] Flag to indicate is request coming from new connection. If true incerease RefCnt for device with same IP.
 *  \retval none.
 */
void B_DTCP_IP_GetAkeHandle(B_DTCP_StackHandle_T pStack, const char * RemoteIp, void ** AkeHandle, bool NewConnection);

/*! \brief exported function to verify current exchange key is still valid, for sink device only.
 *  \param[in] pStack DTCP-IP stack handle
 *  \param[in] AkeHandle current AKE handle.
 *  \retval BERR_SUCCESS or other error code.
 */
BERR_Code B_DTCP_IP_GetNewExchKey(B_DTCP_StackHandle_T pStack, B_AkeHandle_T AkeHhandle);

/*
 * Function to clean up Session list and decrease RefCnt
*/
BERR_Code B_DTCP_IP_SourceSessionClean(B_DTCP_Stack_T * pStack, B_AkeCoreSessionData_T * pAkeSession);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* B_DTCP_IP_STACK_H */

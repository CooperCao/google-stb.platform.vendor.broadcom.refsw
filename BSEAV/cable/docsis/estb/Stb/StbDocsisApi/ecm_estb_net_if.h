//****************************************************************************
//
// Copyright (c) 2003-2013 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized License, Broadcom grants no license
// (express or implied), right to use, or waiver of any kind with respect to
// the Software, and Broadcom expressly reserves all rights in and to the
// Software and all intellectual property rights therein.  IF YOU HAVE NO
// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
// SOFTWARE.  
//
// Except as expressly set forth in the Authorized License,
//                        
// 1.     This program, including its structure, sequence and organization,
// constitutes the valuable trade secrets of Broadcom, and you shall use all
// reasonable efforts to protect the confidentiality thereof, and to use this
// information only in connection with your use of Broadcom integrated circuit
// products.
//
// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//****************************************************************************
//
//  Filename:       ecm_estb_net_if.h
//  Author:         
//  Creation Date:  Sep 12, 2012
//
//****************************************************************************
//  Description: 
//                  
//
//****************************************************************************

#ifndef ECMESTBNETIF_H
#define ECMESTBNETIF_H

//********************** Include Files ***************************************
#if defined __cplusplus
extern "C" {
#endif

//********************** Global Types ****************************************

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@	
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ eCM - eSTB COMMUNICATION CONFIGURATION @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@	
#if ( BCM_DSG_DUAL_PROCESSOR_INTERFACE )

	#define kDefault_eStbSocketMaxRxBufferSize 6144
	#define kDefaultValue_EcmComAttemptMaxCount 30

		#define IPC_INTERFACE_NAME_MAX_LEN  16	// including the null character
					
		#if defined(USE_INTERFACE_ETH2)
			#define kDefaultEcmInterfaceName		"eth2:0"
		#else					
			#define kDefaultEcmInterfaceName		"eth0:0"
		#endif					

#if defined(IP_NONVOL_OVER_ETHERNET)
		#define kLocalRxIpAddress		0x0a18046a // 10.24.4.106 0x0a0a0a0a				// 10.10.10.10    STATIC IP ADDRESS OF eSTB
		#define kRemoteEcmIpAddress		0x0a1804de // 10.24.4.222 0x0a0a0a01				// 10.10.10.1     STATIC IP ADDRESS OF eCM
		#define kRemoteEcmSrcIpAddress  0x0a1804de // 10.24.4.222 0x0a0a0a01				// 10.10.10.1     RECEIVE FROM
#else
		#define kLocalRxIpAddress		0xC0A8110A				// 192.168.17.10    STATIC IP ADDRESS OF eSTB
		#define kRemoteEcmIpAddress		0xC0A81101				// 192.168.17.1     STATIC IP ADDRESS OF eCM
		#define kRemoteEcmSrcIpAddress  kRemoteEcmIpAddress
#endif
		
		#define kRemoteEcmMacAddress	{ 0x00, 0x10, 0x18, 0x00, 0xef, 0xdb }	

	// DUAL PROCESS CASE
#else
	#define kDefaultEcmInterfaceName			"lo0"					// loopback

	#define kLocalRxIpAddress			INADDR_LOOPBACK			// 127.0.0.1	  
	#define kRemoteEcmIpAddress			INADDR_LOOPBACK			// 127.0.0.1	  RECEIVE FROM
	#define kRemoteEcmSrcIpAddress		INADDR_LOOPBACK			// 127.0.0.1	  SEND TO
#endif

#if (BCM_CABLEHOME_SUPPORT)
	#define	kEstbIpStackNumber			8
#else
	#define	kEstbIpStackNumber			3
#endif

// These are the socket port numbers used for eCM-eSTB communication
#define ECM_DSG_REQUEST_PORT			0x4246		// Send commands to eCM
#define ESTB_DSG_RESPONSE_PORT			0x8887		// Wait for eCM response

#define ESTB_DSG_REQUEST_PORT			0x4247		// Wait on Notification from eCM

#define ESTB_DSG_DATA_PORT				0x8889		// Wait on DSG DATA

#define	kEcmReplyPktSize				16			// Size of the packet eCM uses to reply to eSTB requests.
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


#if defined __cplusplus
}
#endif

#endif

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
 * Revision History:
 * 
 * $brcm_Log: $
 * 
*********************************************************************************************/

#ifndef __DTCP_CONSTANTS_H__
#define __DTCP_CONSTANTS_H__

/*! \file
 *  \brief Defines constant values used throughout the DTCP baseline functions.
 */

#define DTCP_DEVICE_KEY_SIZE          					 8	 /*!< Size of device key in bytes */
#define DTCP_BASELINE_FULL_CERT_SIZE  					 88	 /*!< Size of baseline full-auth device certificate */
#define DTCP_EXTENDED_FULL_CERT_SIZE  					 132 /*!< size of extended full-auth  device certificate */
#define DTCP_RESTRICTED_CERT_SIZE     					 48  /*!< size of restricted auth device certificate */

#define DTCP_DEVICE_ID_SIZE           					 5	 /*!< size of device id */
#define DTCP_PUBLIC_KEY_SIZE              				 40  /*!< size of public key */
#define DTCP_PRIVATE_KEY_SIZE							 20  /*!< private key size */
#define DTCP_SIGNATURE_SIZE								 40  /*!< EC-DSA signature size */

/* AKE data sizes */

#define DTCP_FULL_AUTH_NONCE_SIZE						 16 /*!< Size of the random nonce for full auth */
#define DTCP_RESTRICTED_AUTH_NONCE_SIZE             	 8  /*!< Size of the random nonce for restricted auth */
#define DTCP_RESTRICTED_AUTH_RESPONSE_SIZE          	 8  /*!< Size of the response for restricted auth */
#define DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE           	 40 /*!< Size of the Diffie-Hellman first phase value */
#define DTCP_DH_FIRST_PHASE_SECRET_SIZE             	 20 /*!< Size of the Diffie-Hellman first phase secret */
#define DTCP_AUTH_KEY_SIZE                               12 /*!< Size of the authentication key */
#define DTCP_RESTRICTED_AUTH_KEY_SIZE                    8  /*!< Size of the restrcited auth authentication key */
#define DTCP_EXCHANGE_KEY_SIZE                           12 /*!< Size of the exchange key */
#define DTCP_CONTENT_KEY_NONCE_SIZE                      8  /*!< Size of the nonce for computing the content key */

#define DTCP_SRM_VERSION_NUMBER_SIZE                	 2	/*!< Size of the SRM version number */
#define DTCP_SRM_GENERATION_SIZE						 1  /*!< size of SRM generation in challenge response */
#define DTCP_SRM_HEADER_SIZE							 4	/*!< size of SRM header.*/
#define DTCP_SRM_FIRST_GEN_MAX_SIZE						128	/*!< first generation SRM max size */
#define DTCP_SRM_SECOND_GEN_MAX_SIZE					1024	/*!< Second generation SRM max size */
#define DTCP_EXCHANGE_KEY_LABEL_SIZE                     1  /*!< Size of the exchange key label */
#define DTCP_CONTENT_KEY_CONSTANT_SIZE                   12 /*!< Size of the content key constant */
#define DTCP_IP_CONTENT_KEY_SIZE                         16 /*!< Size of the content key for IP */

/* Sink capbilities for RESP2 */
#define DTCP_DEVICE_CAPABILITY_SIZE                      4  /*!< Size of the device capability for IP */

#define DTCP_FULL_AUTH_CHALLENGE_SIZE               	(DTCP_FULL_AUTH_NONCE_SIZE + DTCP_BASELINE_FULL_CERT_SIZE) /*!< Size of a full auth challenge */

#define DTCP_FULL_AUTH_RESPONSE_SIZE                	(DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE + \
														 DTCP_SRM_VERSION_NUMBER_SIZE + \
														 DTCP_SRM_GENERATION_SIZE + \
														 DTCP_SIGNATURE_SIZE)                       	/*!< Size of a full auth response */
#define DTCP_RESTRICTED_AUTH_CHALLENGE_SIZE             (DTCP_RESTRICTED_AUTH_NONCE_SIZE + 2 ) 	/*!< Size of a restricted auth challenge ( nonce + 12 bits SRM version + 4 bits padding) */
#define DTCP_RESTRICTED_AUTH_RESPONSE_SIZE				8
#define DTCP_ENH_RESTRICTED_AUTH_CHALLENGE_SINK_SIZE	(DTCP_RESTRICTED_CERT_SIZE + DTCP_RESTRICTED_AUTH_NONCE_SIZE)

#define DTCP_FULL_AUTH_RESPONSE2_SIZE				    (DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE + \
														 DTCP_SRM_VERSION_NUMBER_SIZE + 1 + \
														 DTCP_DEVICE_CAPABILITY_SIZE + \
														 DTCP_DEVICE_ID_SIZE + \
														 DTCP_SIGNATURE_SIZE) 						/*!< Size of a enhanced restricted auth challenge>*/
#define DTCP_EXTENDED_FULL_AUTH_CHALLENGE_SIZE			(DTCP_FULL_AUTH_NONCE_SIZE + DTCP_EXTENDED_FULL_CERT_SIZE)	  /*!<size of extended full-auth challenge*/

#define DTCP_CAPABILITY_REQ_SIZE                    	(4)										/*!< capability request command */

#define DTCP_EXCHANGE_KEY_CMD_DATA_SIZE             	(DTCP_EXCHANGE_KEY_SIZE + 2)            /*!< Size of a exchange key command( lable + cipher_algorithm + key ) */
#define DTCP_CONTENT_KEY_REQUEST_SIZE               	(DTCP_EXCHANGE_KEY_LABEL_SIZE + 3 + DTCP_CONTENT_KEY_NONCE_SIZE) /*!< Size of a content key request */


#define DTCP_SRM_FIRST_GEN_MAX_SIZE           		128   	/*!< Maximum size of a first generation SRM */
#define DTCP_SRM_SECOND_GEN_MAX_SIZE         		1024  	/*!< Maximum size of a second generation SRM */
#define DTCP_SRM_MAX_SIZE                    		DTCP_SRM_SECOND_GEN_MAX_SIZE   				/*!< Maximum SRM size*/
#define DTCP_SRM_CRL_MIN_SIZE                		(DTCP_SRM_CRL_LENGTH_SIZE + DTCP_SIGNATURE_SIZE) /*!< Minimum SRM CRL size*/
/*!< Size of the full auth response buffer that is signed */
#define DTCP_FULL_AUTH_RESPONSE_SIGN_BUFFER_SIZE    (DTCP_FULL_AUTH_NONCE_SIZE + \
                                                     DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE + \
                                                     DTCP_SRM_VERSION_NUMBER_SIZE + \
                                                     1)
/*!< Size of the full auth response buffer that is signed */
#define DTCP_FULL_AUTH_RESPONSE2_SIGN_BUFFER_SIZE   (DTCP_FULL_AUTH_RESPONSE_SIGN_BUFFER_SIZE + \
                                                     DTCP_DEVICE_CAPABILITY_SIZE + \
                                                     DTCP_DEVICE_ID_SIZE)

#define DTCP_CONTENT_PACKET_HEADER_SIZE         	14 /*!< Size of a protected content packet header */
#define DTCP_AES_KEY_SIZE                       	16 /*!< Size of an AES key */
#define DTCP_AES_IV_SIZE                        	16 /*!< Size of an AES IV */
#define DTCP_AES_BLOCK_SIZE                     	16 /*!< Size of an AES block */
#define DTCP_MAXIMUM_PROTECTED_PACKET_SIZE      	134217728   /*!< Maximum payload size of a protected content packet */
#define DTCP_SINK_COUNT_LIMIT                   	32 	/*!< Sink count limit*/

/* These should be in ECC.h */

#define ECC_PRIME_NUMBER_SIZE           			20  /*!< Size of the ECC prime number */
#define ECC_COEFFICIENT_SIZE            			20  /*!< Size of the ECC coefficient*/
#define ECC_BASEPOINT_SIZE              			40  /*!< Size of the ECC basepoint */
#define ECC_BASEPOINT_ORDER_SIZE        			20  /*!< Size of the ECC basepoint order */


#define KX_EXPIRE 									7200/*!< 2*60*60 = 7200 seconds = 2 hours */

/* Include protocol specific constsnts */
#include "b_dtcp_ip_constants.h"

#endif /* __DTCP_CONSTANTS_H__ */

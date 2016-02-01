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
 * 		DTCP IP supplement constants
 * Revision History:
 * 
 * $brcm_Log: $
 * 
*********************************************************************************************/
#ifndef B_DTCP_IP_CONSTANTS_H
#define B_DTCP_IP_CONSTANTS_H   

#define DTCP_RTT_N_SIZE							    2  						/*!< Size of the RTT Trials counter */
#define DTCP_RTT_MAC_SIZE						    10 						/*!< Size of the RTT MAC */
#define DTCP_RTT_SETUP_SIZE							(DTCP_RTT_N_SIZE)		/*!< Size of RTT setup command */
#define DTCP_RTT_TEST_SIZE							(DTCP_RTT_MAC_SIZE)		/*!< Size of a RTT Test command*/
#define DTCP_RTT_VERIFY_SIZE						(DTCP_RTT_MAC_SIZE)		/*!< Size of a RTT Verify command */
#define DTCP_CONT_KEY_CONF_MAC_SIZE					10 						/*!< Size of the Cont Key Conf MAC */

#define DTCP_CONT_KEY_CONF_R_SIZE					8  						/*!< Size of the Cont Key Conf R value */

#define DTCP_CONT_KEY_CONF_SIZE						(DTCP_CONTENT_KEY_NONCE_SIZE + DTCP_CONT_KEY_CONF_R_SIZE + DTCP_RTT_MAC_SIZE)	/*!< Size of Cont Key Conf command */
#define SHA1_DIGEST_SIZE							20						/*!< Size of the SHA1 digest*/

#define DTCP_RTT_MK_SIZE							(SHA1_DIGEST_SIZE)		/*!< Size of MK value */
#define DTCP_RTT_MAC_DATA_SIZE						(SHA1_DIGEST_SIZE)		/*!< Number of bits in the RTT MAC value */
#define DTCP_RTT_MK_DATA_SIZE						(DTCP_AUTH_KEY_SIZE + DTCP_AUTH_KEY_SIZE) /*!< Size of the RTT MK value */
#define DTCP_RTT_MACAB_SIZE							(DTCP_RTT_MAC_SIZE + DTCP_RTT_MAC_SIZE)	  /*!< Size of MAC1A + MAC1B */

#define DTCP_RTT_MAX_RETRIES						(1024)  				/*!< Maxium number of RTT retries */
#define DTCP_RTT_MAX_RTT_MS							(7) 					/*!< Maxium RTT in milliseconds */

#define DTCP_CONT_KEY_CONF_MX_SIZE					(SHA1_DIGEST_SIZE)
#define DTCP_CONT_KEY_CONF_MAC_DATA_SIZE			(DTCP_CONT_KEY_CONF_MX_SIZE)
#define DTCP_IV_CONSTANT_SIZE						8


#if 0
#define DTCP_CONT_KEY_CONF_MX_SIZE_BITS				(SHA1_DIGEST_SIZE_BITS)	/*!< Number of bits in MX*/
#define DTCP_CONT_KEY_CONF_MACAB_SIZE_BITS			(DTCP_RTT_MAC_SIZE_BITS + DTCP_RTT_MAC_SIZE_BITS)   /*!< Number of bits in MAC3A + MAC3B (SHA1)*/

#define DTCP_CONT_KEY_CONF_MAC_DATA_SIZE_BITS		(DTCP_CONT_KEY_CONF_MX_SIZE_BITS)	/*!< Number of bits in the Cont Key Conf MAC data */
#define DTCP_CONT_KEY_CONF_MX_DATA_SIZE_BITS		(DTCP_EXCHANGE_KEY_SIZE_BITS + DTCP_EXCHANGE_KEY_SIZE_BITS) /*!< Number of bits in the Cont Key Conf MX data */

#define DTCP_CONT_KEY_CONF_MX_SIZE					(DTCP_CONT_KEY_CONF_MX_SIZE_BITS / 8)
#define DTCP_CONT_KEY_CONF_MACAB_SIZE				(DTCP_CONT_KEY_CONF_MACAB_SIZE_BITS / 8)

#define DTCP_CONT_KEY_CONF_MAC_DATA_SIZE			(DTCP_CONT_KEY_CONF_MAC_DATA_SIZE_BITS / 8)
#define DTCP_CONT_KEY_CONF_MX_DATA_SIZE				(DTCP_CONT_KEY_CONF_MX_DATA_SIZE_BITS / 8)
#endif
#define DTCP_RTP_NONCE_TIMER_PERIOD              	60 

#define DTCP_TTL									3
#define MAX_IP_ADDR_SIZE							32
#define DTCP_IP_PORT								8000

#endif /* B_DTCP_IP_CONSTANTS_H */

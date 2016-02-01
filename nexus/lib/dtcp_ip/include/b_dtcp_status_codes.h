/******************************************************************************
 *    (c)2009-2010 Broadcom Corporation
 * 
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
 * USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
 * ANY LIMITED REMEDY.
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
 *****************************************************************************/

#ifndef __DTCP_STATUS_CODES_H__
#define __DTCP_STATUS_CODES_H__

/* \file
 * \brief Defines the status codes for the DTCP libraries
 */

#ifdef __cplusplus
extern  "C"
{
#endif

#define BERR_BAD_STRUCTURE_SIZE                 100
#define BERR_BAD_STRUCTURE_VERSION              101
#define BERR_APP_CALLBACKS_NOT_VALID            102
#define BERR_RESPONSE_TIMEOUT                   103
#define BERR_AKE_CANCELED                       104


#define BERR_INVALID_HANDLE                     201

#define BERR_INVALID_CERTIFICATE                302
#define BERR_DTCP_CERT_INVALID_FORMAT           303 
#define BERR_DTCP_DEVICE_ID_REQUIRED            304

#define BERR_INVALID_SRM                        401
#define BERR_SRM_UPDATE_FAILED                  402
#define BERR_DEVICE_REVOKED                     403 
#define BERR_RTT_FAILED                         404
#define BERR_CONT_KEY_CONF_FAILED			    405
#define BERR_CONT_KEY_CONF_NONONCE              406

#define BERR_DTCP_INVALID_COMMAND_SEQUENCE           505
#define BERR_DTCP_INVALID_DATA_FIELD                 506
#define BERR_DTCP_OTHER_DEVICE_RESPONSE_DATA_INVALID 507
#define BERR_DTCP_OTHER_DEVICE_CERTIFICATE_INVALID   508 
#define BERR_DTCP_SINK_COUNT_LIMIT_REACHED           509

#define BERR_CONTENT_SIZE_TOO_LARGE                 600
#define BERR_INVALID_PACKET_HEADER                  601
#define BERR_INVALID_BUFFER_LENGTH                  602
#define BERR_CONTENT_EXCEEDS_PACKET_SIZE            603
#define BERR_INVALID_EXCHANGE_KEY_LABEL             604
#define BERR_UNSUPPORTED_AKE_COMMAND				605
#define BERR_DUPLICATED_ENTRIES						606
#define BERR_INVALID_EXCHANGE_KEY					607

#define BERR_DTCP_AKE_TIMEOUT                       700
/* General errors */
#define BERR_CRYPT_FAILED							701
#define BERR_SOCKET_ERROR							702
#define BERR_IO_ERROR								703
#define BERR_FATAL_ERROR							704
#define BERR_NOT_FOUND								705
#define BERR_HW_SECURITY_FAILURE					706
#define BERR_SOCKET_CLOSE							707

#ifdef __cplusplus
}
#endif

#endif

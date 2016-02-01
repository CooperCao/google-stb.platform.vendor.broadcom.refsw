/******************************************************************************
 *    (c)2013 Broadcom Corporation
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
 * Module Description: SAGE - Shared types between the Host and SAGE
 * 
 * Revision History:
 * 
 *****************************************************************************/


#ifndef _SAGE_TYPES_H_
#define _SAGE_TYPES_H_


/**

Provide a backward compatibility header file to
make existing code based on SAGE 1.x API compatible with SAGE 2.x API

Existing code should be adapted to, and all new code should be based on,
the new SAGE 2.x API defined in SAGE system library

see public headers in magnum/syslib/sagelib/include

**/


#include "bsagelib_types.h"


#define SAGE_PLATFORM_ID_UNKNOWN         BSAGE_PLATFORM_ID_UNKNOWN
#define SAGE_PLATFORM_ID_COMMONDRM       BSAGE_PLATFORM_ID_COMMONDRM

typedef BERR_Code SAGE_Error;

#define SAGE_SUCCESS                     BERR_SUCCESS
#define SAGE_NOT_INITIALIZED             BERR_NOT_INITIALIZED
#define SAGE_INVALID_PARAMETER           BERR_INVALID_PARAMETER
#define SAGE_OUT_OF_SYSTEM_MEMORY        BERR_OUT_OF_SYSTEM_MEMORY
#define SAGE_OUT_OF_DEVICE_MEMORY        BERR_OUT_OF_DEVICE_MEMORY
#define SAGE_TIMEOUT                     BERR_TIMEOUT
#define SAGE_OS_ERROR                    BERR_OS_ERROR
#define SAGE_LEAKED_RESOURCE             BERR_LEAKED_RESOURCE
#define SAGE_NOT_SUPPORTED               BERR_NOT_SUPPORTED
#define SAGE_UNKNOWN                     BERR_UNKNOWN
#define SAGE_NOT_AVAILABLE               BERR_NOT_AVAILABLE

#define SAGE_HSM_ERROR                   BSAGE_ERR_HSM
#define SAGE_ALREADY_INITIALIZED         BSAGE_ERR_ALREADY_INITIALIZED
#define SAGE_INVALID_INSTANCE            BSAGE_ERR_INSTANCE
#define SAGE_INTERNAL_ERROR              BSAGE_ERR_INTERNAL
#define SAGE_IN_CONGESTION               BERR_NOT_INITIALIZED /* does not exists anymore */
#define SAGE_SHI_ERROR                   BSAGE_ERR_SHI
#define SAGE_INVALID_MODULE_ID           BSAGE_ERR_MODULE_ID
#define SAGE_INVALID_PLATFORM_ID         BSAGE_ERR_PLATFORM_ID
#define SAGE_INVALID_MODULE_COMMAND_ID   BSAGE_ERR_MODULE_COMMAND_ID
#define SAGE_INVALID_SYSTEM_COMMAND_ID   BSAGE_ERR_SYSTEM_COMMAND_ID
#define SAGE_INVALID_STATE               BSAGE_ERR_STATE
#define SAGE_CONTAINER_REQUIRED          BSAGE_ERR_CONTAINER_REQUIRED
#define SAGE_SIGNATURE_MISMATCH          BSAGE_ERR_SIGNATURE_MISMATCH
#define SAGE_RESET                       BSAGE_ERR_RESET

#define SAGE_FAILURE                     BSAGE_ERR_INTERNAL /* does not exists anymore */

typedef BSAGElib_State SAGE_State;
#define SAGE_State_eUninit BSAGElib_State_eUninit
#define SAGE_State_eInit BSAGElib_State_eInit
#define SAGE_State_eMax BSAGElib_State_eMax

typedef BSAGElib_SharedBlock SAGE_SharedBlock;

#define SAGE_CONTAINER_MAX_BASICIN       BSAGE_CONTAINER_MAX_BASICIN
#define SAGE_CONTAINER_MAX_BASICOUT      BSAGE_CONTAINER_MAX_BASICOUT
#define SAGE_CONTAINER_MAX_SHARED_BLOCKS BSAGE_CONTAINER_MAX_SHARED_BLOCKS

typedef BSAGElib_InOutContainer SAGE_InOutContainer;


#endif /* _SAGE_TYPES_H_ */

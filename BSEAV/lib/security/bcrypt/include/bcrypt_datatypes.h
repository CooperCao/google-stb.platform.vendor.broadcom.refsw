/******************************************************************************
 *    (c)2011 Broadcom Corporation
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
#ifndef BCRYPT_DATATYPES_H__
#define BCRYPT_DATATYPES_H__

#include "bkni.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Use printf to display debug message */
/*#define BCRYPT_DEBUG  */
#ifdef BCRYPT_DEBUG  
	#undef BDBG_MSG
	#undef BDBG_ERR
	#define BDBG_MSG(x) printf x
	#define BDBG_ERR(x) printf x
#endif



#define BCRYPT_LINUX_USER_MODE_DRV

/* Host Secure Module magic number used to check if opaque handle is corrupt */
#define BCRYPT_P_HANDLE_MAGIC_NUMBER           0xfeedbabe


#define BCRYPT_P_CHECK_ERR_CODE_FUNC( errCode, function )		\
	if( (errCode = (function)) != BERR_SUCCESS )	\
	{							\
		errCode = BERR_TRACE(errCode);	\
		goto BCRYPT_P_DONE_LABEL;	\
	}									

#define BCRYPT_P_CHECK_ERR_CODE_FUNC2( errCode, errCodeValue, function )			\
	if ( ( errCode = (function)) != BERR_SUCCESS ) \
	{										\
		errCode = BERR_TRACE(errCodeValue);	\
		goto BCRYPT_P_DONE_LABEL;							\
	}

	
#define BCRYPT_P_CHECK_ERR_CODE_CONDITION( errCode, errCodeValue, condition )			\
	if( (condition) ) \
	{ \
		errCode = BERR_TRACE((errCodeValue));		\
		goto BCRYPT_P_DONE_LABEL;							\
	}	
	
/* Definitions */




/* Host Secure module status codes */
typedef enum BCRYPT_STATUS_e{

	BCRYPT_STATUS_eOK,			
	BCRYPT_STATUS_eFAILED,	
	BCRYPT_STATUS_eTIME_OUT,
	BCRYPT_STATUS_eHW_BUSY_ERR,
	BCRYPT_STATUS_eOUT_OF_SYSTEM_MEMORY,
	BCRYPT_STATUS_eINVALID_PARAMETER

} BCRYPT_STATUS_eCode;




/***************************************************************************
Summary:
Parameters for BCRYPT handle.


Description:
This Structure defines the parameters for BCRYPT handle. It contains different handles 
such as HSM handle, DMA handle, Memory DMA handle, SHARF Memory DMA handle, 
DMA Event handle, HSM Mutex Handle, and DMA Mutex handle. Moreover, it contains 
bIsOpen and ulMagicNumber which must be equal to BCRYPT_P_HANDLE_MAGIC_NUMBER.


See Also:
BCRYPT_Setting

****************************************************************************/
typedef struct BCRYPT_P_Handle
{
	unsigned long       			ulMagicNumber; 	/* Out: BCRYPT Magic Number */
	bool						bIsOpen;	/* Out: Defines if BCRYPT handle is open */

} BCRYPT_P_Handle;



#ifdef __cplusplus
}
#endif

#endif /* BCRYPT_DATATYPES_H__ */





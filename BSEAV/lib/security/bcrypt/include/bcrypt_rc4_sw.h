/***************************************************************************
 *     (c)2003-2011 Broadcom Corporation
 *  
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
 *   
 *  Except as expressly set forth in the Authorized License,
 *   
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *   
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *  
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
 *  ANY LIMITED REMEDY.
 * 
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: Header file for RC4 software implementation.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BCRYPT_RC4_H_
#define BCRYPT_RC4_H_

#define RC4_TABLESIZE 256

#define USE_OPENSSL_RC4

#ifdef USE_OPENSSL_RC4
#include <openssl/rc4.h>

#define BCRYPT_RC4Key_t RC4_KEY
#else


/***************************************************************************
Summary:
Parameters for RC4 Key.


Description:
This Structure defines the parameters for RC4 Key. It contains the indices x and y, 
and state table.


See Also:
BCRYPT_S_RC4SwParam, BCrypt_RC4SetKey

****************************************************************************/
typedef struct BCRYPT_RC4Key
{
	unsigned int x, y;        				/* Indices */
	unsigned int data[RC4_TABLESIZE];		/* State table */
} BCRYPT_RC4Key_t;

#endif


/***************************************************************************
Summary:
Parameters for SW RC4 algorithm.


Description:
This Structure defines the parameters for the implementation of RC4 in software. It contains 
RC4 key, input data, size of the input data in bytes, and the output data.
and state table.


See Also:
BCRYPT_RC4Key, BCrypt_RC4Sw

****************************************************************************/
typedef struct BCRYPT_S_RC4SwParam
{
	BCRYPT_RC4Key_t	*key;			/* In: A pointer to RC4 key */
	unsigned long cbLen;				/* In: Size of the input data in bytes */
	unsigned char *pbDataIn;			/* In: A pointer to input data */
	unsigned char *pbDataOut;			/* Out: A pointer to output data */
} BCRYPT_S_RC4SwParam_t;




/* Basic Module Functions */


/*****************************************************************************
Summary:
This function sets the RC4 key.


Description:
This function sets the RC4 key. 


Calling Context:
The function shall be called from application level or from driver level, for example in Linux
during insmod.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.


Input:
key  - BCRYPT_RC4Key_t, a pointer/ref to the Rc4 key.
keyLen - uint32_t. 
keyData - uint32_t. 


Returns:
Void


See Also:
BCrypt_RC4Sw

******************************************************************************/
void BCrypt_RC4SetKey (BCRYPT_RC4Key_t *key, uint32_t keyLen, uint8_t *keyData);



/*****************************************************************************
Summary:
This function implements the RC4 algorithm in SW.


Description:
This function implements the RC4 algorithm in software. 


Calling Context:
The function shall be called from application level or from driver level, for example in Linux
during insmod.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.


Input:
hBcrypt  - BCRYPT_Handle
pInputParam - BCRYPT_S_RC4SwParam_t, a ref/pointer to the parameters 
		for the SW implementation of the RC4 algorithm. 

			
Returns:
BCRYPT_STATUS_eOK - success
BCRYPT_STATUS_eFAILED - failure 


See Also:
BCrypt_RC4SetKey

******************************************************************************/
BCRYPT_STATUS_eCode BCrypt_RC4Sw (	BCRYPT_Handle  hBcrypt, 				
									BCRYPT_S_RC4SwParam_t *pInputParam );	


#endif /* BCRYPT_RC4_H_ */

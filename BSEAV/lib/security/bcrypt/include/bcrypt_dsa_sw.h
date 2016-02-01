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
 * Module Description: Header file for RSA software implementation.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BCRYPT_DSA_H_
#define BCRYPT_DSA_H_



/***************************************************************************
Summary:
Struct to be used in the DSA key parameter.


Description:
This Structure is used for defining DSA key parameters. It includes an stream of data and 
its length. 


See Also:
BCRYPT_DSAKey

****************************************************************************/

typedef struct BCRYPT_DSAKey_Params
{
	unsigned char *pData;				/* In: A pointer to data */
	unsigned long len;					/* In: Length of data */
} BCRYPT_DSAKey_Params_t;


/***************************************************************************
Summary:
Parameters for defining DSA key.


Description:
This Structure defines the parameters DSA key. These parameters are first prime 
number p, second prime numberqe, generator g, public and private keys, and inverse of k.


See Also:
BCRYPT_DSAKey_Params, BCRYPT_S_DSASwParam

****************************************************************************/

typedef struct BCRYPT_DSAKey
{
	BCRYPT_DSAKey_Params_t	p;			/* In: 1st prime number */
	BCRYPT_DSAKey_Params_t	q;			/* In: 2nd prime number */
	BCRYPT_DSAKey_Params_t	g;			/* In: Generator  */
	BCRYPT_DSAKey_Params_t	pubkey;		/* In: public key */
	BCRYPT_DSAKey_Params_t	privkey;		/* In: private key */
	BCRYPT_DSAKey_Params_t	k;			/* In:  kinv, Inverse of k */
} BCRYPT_DSAKey_t;


/***************************************************************************
Summary:
Parameters for DSA operations in software.


Description:
This Structure defines the parameters for perfoming DSA operations in software. In includes
DSA key, input data, size of the input data, output signature, size of the output, and the 
dsa operation parameter. Different DSA operations are sign and verify. 


See Also:
BCRYPT_DSAKey_Params, BCRYPT_DSAKey

****************************************************************************/

typedef struct BCRYPT_S_DSASwParam
{
	BCRYPT_DSAKey_t	*key;		/* In: A pointer to RSA key (public or private key)*/
	bool	bDSASign;				/* In: DSA operation, true for sign, false for verify */
	unsigned char *pbDataIn;		/* In: A pointer to input data */
	int cbDataIn;					/* In: size of input data in bytes */
	unsigned char sigout[256];		/* Out: A pointer to output signature */
	unsigned int sigoutlen;			/* Out: size of output signature in bytes */
	unsigned char *sigin_r;		/* In: The r value of the signature to be verified */
	unsigned char *sigin_s;		/* In: The s value of the signature to be verified */

} BCRYPT_S_DSASwParam_t;



/* Basic Module Functions */


/*****************************************************************************
Summary:
This function implements different DSA operations in SW.


Description:
This function implements different DSA operations in software.
The inputs include DSA key, input data, size of the input data, output signature, size of the 
output signature, the values of r and s for the signature to be verified, and the dsa operation 
parameter. Different DSA operations that are supported are DSA Sign and DSA Verify. 


Calling Context:
The function shall be called from application level or from driver level, for example in Linux
during insmod.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.


Input:
hBcrypt  - BCRYPT_Handle
pInputParam - BCRYPT_S_DSASwParam_t, a ref/pointer to the parameters for SW 
			DSA operations.

			
Returns:
BCRYPT_STATUS_eOK - success
BCRYPT_STATUS_eFAILED - failure 


See Also:
BCRYPT_DSAKey_Params, BCRYPT_DSAKey, BCRYPT_S_DSASwParam
******************************************************************************/

BCRYPT_STATUS_eCode BCrypt_DSASw (	BCRYPT_Handle  hBcrypt, 				
									BCRYPT_S_DSASwParam_t *pInputParam );	

#endif /* BCRYPT_DSA_H_ */

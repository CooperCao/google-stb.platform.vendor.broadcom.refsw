/***************************************************************************
 *     (c)2006-2011 Broadcom Corporation
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
 * Module Description: 
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
 #ifndef BCRYPT_SHA1_SW_H__
#define BCRYPT_SHA1_SW_H__


/***************************************************************************
Summary:
Parameters for SHA-1 algorithm in software.


Description:
This Structure defines the parameters for SHA-1 algorithm in software. It contains input
source data, the length on input data in bytes, SHA-1 context, inital SHA-1 context, the
generated digest, and the length of digset in bytes.

See Also:
BCrypt_Sha1Hw

****************************************************************************/
typedef struct BCRYPT_Sha1Sw {
	unsigned char 	*pucSrcData;	/* In: A pointer to the input source data */
	unsigned long 	ulSrcDataLenInByte;	/* In: Length of the input data in bytes */
	unsigned long ulctxnum; 			/*In: Sha1 context, MUST be within 0-15. */
	bool   binitctx;  					/*In: Init SHA-1 context. */
	bool   bfinal;  					/*In: Generate final digest. */

	unsigned char 	*pucDigest;		/* Out: A pointer to the output digest value */
	unsigned char	ucDigestLenInByte;	/* Out: Length of the output digest in bytes */
	
}  BCRYPT_Sha1Sw_t;



/* Basic Module Functions */


/*****************************************************************************
Summary:
This function implements the SHA-1 algorithm in SW.


Description:
This function implements the SHA-1 algorithm in software. It is 
based on the SHA-1 implementation of the openssl code. The binaries of the openssl library is 
already included into the build environment.


Calling Context:
The function shall be called from application level or from driver level, for example in Linux
during insmod.


Performance and Timing:
This is a synchronous/blocking function that will not return until it is done or failed.


Input:
hBcrypt  - BCRYPT_Handle
inoutp_sha1SwIO - BCRYPT_Sha1Sw_t, a ref/pointer to the parameters 
		for the SW implementation of the SHA-1 algorithm. 

			
Returns:
BCRYPT_STATUS_eOK - success
BCRYPT_STATUS_eFAILED - failure 


See Also:
BCrypt_Sha256Sw

******************************************************************************/
BCRYPT_STATUS_eCode BCrypt_Sha1Sw( 
		BCRYPT_Handle 	hBcrypt,
		BCRYPT_Sha1Sw_t 	*inoutp_sha1SwIO
);

#endif /* BCRYPT_SHA1_SW_H__ */


/******************************************************************************
 *    (c)2007-2015 Broadcom Corporation
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
 *****************************************************************************/

#ifndef BHSM_USER_CMDS_H__
#define BHSM_USER_CMDS_H__

#include "bhsm_priv.h"


#include "bsp_s_user_random_number.h"
#include "bsp_s_pke.h"
#include "bsp_s_user_des_aes.h"
#include "bsp_s_hmac_sha1.h"
#include "bsp_s_cancel.h"  


#include "bhsm.h"
#include "bhsm_keyladder.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 09/28/05,Allen.C */
#define UINT32_SIZE_IN_BYTES 	(sizeof(uint32_t))
#define BSP_RETURN_SUCESS	0x00	/* BSP_STATUS_eSuccess */
#define BSP_NEED_MORE_DATA_PKE 0xa3	/* BCMD_STATUS_ePKEMoreData ,  lyang: 0xa3 to 0xa8*/
#define BSP_NEED_MORE_DATA_HMACSHA1 0xa8
/* Module Specific Functions */

#define BHSM_MAX_RANDOM_NUM_LEN	360
/* 2048 bit field in BCMD_KeyRamBuf_eFirstRNRam1024 or BCMD_KeyRamBuf_eSecondRNRam1024*/
#define MAX_DATA_LENTH_USER_RANDOM_NUMBER_RNRam1024		(2048/8)		

typedef struct BHSM_UserRandomNumberIO {

	/* In: Selects the random number generation type:
			0x00 = raw random number from RNG, i.e. BSP_RNG_RNG_CORE_CTRL_RNG_TYPE_RNGRAW 
			0x01 = random number processed according to FIPS186-2 'x' generation,  Use BSP_RNG_RNG_CORE_CTRL_RNG_TYPE_FIPS186_X
			0x02 = random number processed according to FIPS186-2 'k' generation; Use BSP_RNG_RNG_CORE_CTRL_RNG_TYPE_FIPS186_K     
			0x03 = RNG-SHA1;  Use BSP_RNG_RNG_CORE_CTRL_RNG_TYPE_RNGSHA                
	*/	
	unsigned char				ucRandomNumType; 

	/* In: select from BCMD_KeyRamBuf_e, specify the destination of the output
		ANDOM_NUMBER_OUTPUT_TO_HOST=0x00, Output as many random numbers as specified in the Length field below to Host; 
	    BCMD_KeyRamBuf_eFirstRNRam1024 = 20,  up to 2048-bit random numbers to the 1st 2048-bit RN buffer
	    BCMD_KeyRamBuf_eSecondRNRam1024= 21, up to 2048-bit random numbers to the 2nd 2048-bit RN buffer
	*/
	BCMD_KeyRamBuf_e		keyDest;  


     /* In: Length of the Random Number in bytes. It should be 32-bit word aligned. If 'Destination'  equals to 
             BCMD_KeyRamBuf_eFirstRNRam1024 or BCMD_KeyRamBuf_eSecondRNRam1024,  the maximum size shall be 256. 
             If 'Destination'  equals to 0x00, the maximum size shall be 360 bytes.*/
	unsigned int				unDataLen;

	/* Out: 0x00 - for success, 
 	 	    	0xff - a generic error
 	 	    	0x01 - a bad ramdom number generated  */
	uint32_t					unStatus;  

	/* Out: the returned/generated random number according to the inputs*/
	unsigned char				aucData[BHSM_MAX_RANDOM_NUM_LEN];

} BHSM_UserRandomNumberIO_t;

/*****************************************************************************
Summary:

This function is provided for MIPS host to use the random number generating (RNG) engine inside BSP to produce random numbers.
TBD.

Description:

TBD.

Calling Context:

This function can be called any time after the system and BSP is initialized, when the applications decides to utilize the 
RNG engine inside BSP.


Performance and Timing:
This is a synchronous/blocking function that won'tl return until it is done or failed.  


Input:
in_handle - BHSM_Handle, Host Secure module handle.
inoutp_userRandomNumberIO - BHSM_UserRandomNumberIO_t.


Output:
inoutp_userRandomNumberIO - BHSM_UserRandomNumberIO_t.


Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
*****************************************************************************/
BERR_Code     BHSM_UserRandomNumber (
		BHSM_Handle					in_handle,
		BHSM_UserRandomNumberIO_t	*inoutp_userRandomNumberIO
);


typedef struct BHSM_UserRsaIO {
	/* input */
	
	unsigned char				ucOprMode;		/* select operation mode, RSA Normal/0, Chinese Remainder Theorem/1, PKE_PassThrough/0xFF mainly for testing, others invalid*/
	BCMD_KeyRamBuf_e		keySrcIndex1;  	/* 8bit key source index of PublicKeyVerify:n,  or PrivateKeySign: p & q.   to specify BSP First/Second/ThirdRAM2048, i.e.17/18/19, others invalid  */
/*10/03/05,Allen.C add according to BCMD_PKE_RSAInCmdField_eExpSrcIdx in bsp_s_pke.h */	
	BCMD_KeyRamBuf_e		expSrcIdx ;    	/* 8bit exponent source index of PublicKeyVerify:e,  or PrivateKeySign: Priv.   to specify BSP First/Second/ThirdRAM2048, i.e.17/18/19, others invalid  */
	BCMD_KeyRamBuf_e		keySrcIndex2;    /* 8bit exponent source index2 of PrivateKeySign: dp & dq.   to specify BSP First/Second/ThirdRAM2048, i.e.17/18/19, others invalid  */
	unsigned int				unInputDataLen; /*16bit actual length in bytes of the following data for RSA processing */

	/* BSYT : Wrong max size.  Max size should be the max data PKE engine can handle */
	unsigned char				aucInputData[3* BCMD_PKE_RSA_MAX_DATA_LENGTH];  /* actual byte stream of RSA input data */


	/* Output */
	uint32_t					unStatus;
	uint32_t					unOutputDataLen;	/* 32bit actual length in bytes of the processed output RSA data  */

	/* BSYT : Wrong max size.  Max size should be the max data PKE engine can handle */	
	unsigned char				aucOutputData[3*BCMD_PKE_RSA_MAX_DATA_LENGTH]; /* actual byte stream of returned RSA output data */

	
} BHSM_UserRsaIO_t;


/*****************************************************************************
Summary:

This function is provided for MIPS host to use the PKE RSA engine inside BSP to do RSA Public key based verification(decryption), 
RSA private key based signing (encryption). RSA cryptographic functionality could be disabled via setting specific MSP bits.
Since the performance is not very fast, may not be widely used.

Though this function can be used to do RSA encryprtion using public key and RSA decryption using private key, it's rarely 
used this way due to its very slow processing speed compared to other regular (de)encryption algorithms. 

Description:

The given data block in one function call can be a first piece, middle piece, last piece of a big message or a complete message.
( the way to hold data using a fixed buffer may be replaced with a data pointer later)

The passthrough mode (0xFF) is mainly for testing purpose, contact Broadcom for details if it's really needed. Normal mode (0) uses 
(c^e mode n) formula. Chinese Remainder Theorem (1) mode TBD.

Each member of public key set (n, e) or private key set (p, q, d) can be as long as 2048bits=256bytes.


Calling Context:

This function can be called any time after the system and BSP is initialized, when the applications decides to utilize the 
PKE RSA engine inside BSP.


Performance and Timing:
This is a synchronous/blocking function that won'tl return until it is done or failed.  No testing data is available, and no  
estimated time either. Here's one for the reference.

PublicKeyVerify,   n=256 bytes, e=4 bytes, Msg=256bytes    time= 9270 us  rate = 220.9 kbps


Input:
in_handle  - BHSM_Handle, Host Secure module handle.
inoutp_userDesAesIO  - BHSM_UserDesAesIO_t, not all members in the input section are always needed,
	'ucOprMode'		 8 bit value to select operation mode, RSA Normal/0, Chinese Remainder Theorem/1, PKE_PassThrough/0xFF mainly for testing, others invalid
	'keySrcIndex1'  	 8bit key source index of PublicKeyVerify:n,  or PrivateKeySign: p & q.   to specify BSP First/Second/ThirdRAM2048, i.e.17/18/19, others invalid  
	'expSrcIdx'     8bit exponent source index of PublicKeyVerify:e,  or PrivateKeySign: Priv.   to specify BSP First/Second/ThirdRAM2048, i.e.17/18/19, others invalid  
	'keySrcIndex2'   8bit exponent source index2 of PrivateKeySign: dp & dq.   to specify BSP First/Second/ThirdRAM2048, i.e.17/18/19, others invalid  
	                      not needed for PublicKeyVerify
	'unInputDataLen' to state the actual length of message data piece to BSP
	'aucInputData'  to provide a message data piece or a whole message to do the PKE verification or signing
	
	the output section can be any value (normally input as 0).
			
Output:
inoutp_userDesAesIO  - BHSM_UserDesAesIO_t, the member in output section

	'unStatus' is modified to reflect the command processing status at BSP side, 0 for success, non-zero for certain error.
	'unOutputDataLen' is 16 bit value, in bits, the length of returned cleartext or ciphertext
	'aucOutputData' contains the returned PKE processed cleartext or ciphertext

Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_UserDSA
*****************************************************************************/
BERR_Code      BHSM_UserRSA (
		BHSM_Handle			in_handle,
		BHSM_UserRsaIO_t		*inoutp_userRsaIO
);




#define BHSM_MAX_USER_DSA_LEN	40

typedef struct BHSM_UserDsaIO {
	/* input */
	
	unsigned char				ucOprMode;   /*PrivateKeySign/0, PublicKeyVerify/1, PKE_PassThrough/0xFF mainly for testing, others invalid*/
	BCMD_KeyRamBuf_e		srcPIndex; 	/*8bit key source index of Public p, to specify BSP First/Second/ThirdRAM2048, i.e.17/18/19, others invalid  */
	BCMD_KeyRamBuf_e		srcGIndex;  /*8bit key source index of Public g, to specify BSP First/Second/ThirdRAM2048, i.e.17/18/19, others invalid  */
	BCMD_KeyRamBuf_e		srcQIndex;  /* 8bit source index of Public q, 160bits,  to specify BSP First/Second/ThirdRAM2048, i.e.17/18/19, others invalid  */
	BCMD_KeyRamBuf_e		privKeyXIndex;  /* 8bit source index of Private Key x, 160bits, using BSP FirstRN/SecondRNRam1024, i.e.20/21, others invalid  */
	BCMD_KeyRamBuf_e		privKeyKIndex; /*8bit source index of Private Key x, 160bits, using BSP FirstRN/SecondRNRam1024, i.e.20/21, others invalid  */
	unsigned int				unInputDataLen; /* 16bit actual length value, in bytes, of message data  to BSP, 20 bytes for signing,  4bytes for pass-through,  P+60 bytes for verification*/
	/* BSYT : Wrong max size.  Max size should be the max data PKE engine can handle */	
	unsigned char				aucInputData[3*BCMD_PKE_DSA_MAX_DATA_LENGTH]; /* to provide a message data to do the PKE DSA verification or signing*/

	/* Output */
	uint32_t					unStatus;
	/* BSYT : Wrong max size.  Max size should be the max data PKE engine can handle */	
	unsigned char				aucOutputData[3*BHSM_MAX_USER_DSA_LEN]; /*the returned PKE DSA processed signature (40 bytes) or verification result(20bytes)*/

	
} BHSM_UserDsaIO_t;


/*****************************************************************************
Summary:

This function is provided for MIPS host to use PKE Digital Signature Algorithm (DSA) engine inside BSP to advanced 
private key based signing (encryption) and public key based verification. DSA cryptographic functionality could be disabled via setting specific MSP bits.


Though this function can be used to do encryprtion using public key and decryption using private key, it's rarely 
used this way due to its very slow processing speed compared to other regular (de)encryption algorithms, and thus not 
supported.

Description:

The given data block in one function call can be a first piece, middle piece, last piece of a big message or a complete message.
( the way to hold data using a fixed buffer may be replaced with a data pointer later)

The passthrough mode (0xFF) is mainly for testing purpose, contact Broadcom for details if it's really needed. 

p, g, q can be as long as 2048bits, however x, k are up to 1024 bits only.



Calling Context:

This function can be called any time after the system and BSP is initialized, when the applications decides to utilize the 
PKE DSA engine inside BSP.


Performance and Timing:
This is a synchronous/blocking function that won'tl return until it is done or failed.  No testing data is available, and no  
estimated time either. 



Input:
in_handle  - BHSM_Handle, Host Secure module handle.
inoutp_userDsaIO  - BHSM_UserDsaIO_t *, not all members in the input section are always needed,
	'ucOprMode'		 8 bit value to select operation mode, PrivateKeySign/0, PublicKeyVerify/1, PKE_PassThrough/0xFF mainly for testing, others invalid
	'srcPIndex'  	 8bit key source index of Public p, to specify BSP First/Second/ThirdRAM2048, i.e.17/18/19, others invalid  
	'srcGIndex'     8bit source index of Public g,    to specify BSP First/Second/ThirdRAM2048, i.e.17/18/19, others invalid  
	'srcQIndex'     8bit source index of Public q, 160bits,  to specify BSP First/Second/ThirdRAM2048, i.e.17/18/19, others invalid  
	'privKeyXIndex'   8bit source index of Private Key x, 160bits, using BSP FirstRN/SecondRNRam1024, i.e.20/21, others invalid  
	'privKeyKIndex'   8bit source index of Private Key k, 160bits, using BSP FirstRN/SecondRNRam1024, i.e.20/21, others invalid  
	'unInputDataLen' 16bit actual length value, in bytes, of message data  to BSP, 20 bytes for signing,  4bytes for pass-through,  P+60 bytes for verification
	'aucInputData'  to provide a message data to do the PKE DSA verification or signing
	
	the output section can be any value (normally input as 0).
			
Output:
inoutp_userDsaIO  - BHSM_UserDsaIO_t *, the member in output section

	'unStatus' is modified to reflect the command processing status at BSP side, 0 for success, non-zero for certain error.
	'aucOutputData' contains the returned PKE DSA processed signature (40 bytes) or verification result(20bytes)

Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_UserRSA
*****************************************************************************/
BERR_Code      BHSM_UserDSA (
		BHSM_Handle			in_handle,
		BHSM_UserDsaIO_t		*inoutp_userDsaIO
);




typedef struct BHSM_UserDhIO {
	/* input */
	
	unsigned char				ucOprMode;
	BCMD_KeyRamBuf_e		srcPIndex; 
	BCMD_KeyRamBuf_e		srcGIndex;
	BCMD_KeyRamBuf_e		privKeyXIndex; 
	unsigned int				unInputDataLen;
	/* BSYT : Wrong max size.  Max size should be the max data PKE engine can handle */	
	unsigned char				aucInputData[3*BCMD_PKE_DH_MAX_DATA_LENGTH];

	/* Output */
	uint32_t					unStatus;
	uint32_t					unOutputDataLen;
	/* BSYT : Wrong max size.  Max size should be the max data PKE engine can handle */	
	unsigned char				aucOutputData[3*BCMD_PKE_DH_MAX_DATA_LENGTH];

	
} BHSM_UserDhIO_t;



/*****************************************************************************
Summary:

This function is provided for MIPS host to use the DH engine inside BSP to exchange private keys. 
TBD.

Description:

TBD.

Calling Context:

This function can be called any time after the system and BSP is initialized, when the applications decides to utilize the 
DH engine inside BSP.


Performance and Timing:
This is a synchronous/blocking function that won'tl return until it is done or failed.  No testing data is available, and no  
estimated time either. Here's one for the reference.


Input:
TBD.


Output:
TBD.


Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
*****************************************************************************/
BERR_Code      BHSM_UserDH (
		BHSM_Handle		in_handle,
		BHSM_UserDhIO_t	*inoutp_userDhIO
);


#if 0
#define MAX_DATA_LENTH_USER_DES_AES					328
#endif
#define BHSM_USER_DES_AES_OPRMODE_DECRYPTION 0x00
#define BHSM_USER_DES_AES_OPRMODE_ENCRYPTION 0x01

typedef struct BHSM_UserDesAesIO {
	/* In: 56/64/128/192-bit clear key for AES or DES, polulated from the beginning */	
	unsigned char				auKeyData[KEY_INPUT_PARAMETER_SIZE]; 
	
	/* In: 8bit value, select algorithm, DES_ECB/0, 3DESABA_ECB/1, AES128_ECB/2 */
	unsigned char				ucAlgorithm; 			

	/* In: 8bit value, select decryption/0 or encryption/1 processing at BSP
				0x00: decryption (if 3DES is selected, use DED for 3DES)
				0x01: encryption (if 3DES is selected, use EDE for 3DES)
	*/
	unsigned char				ucOprMode;				
	
	/* In: actually data in bytes, must be 4n alligned (word32)*/
	unsigned int				unInputDataLen;			

	/* In: buffered data to be encypted or decypted at BSP, up to 328 bytes for DES, 320 for AES, has to be 
	        padded if the last actual block is incomplte*/
	unsigned char				aucInputData[BCMD_USER_DES_AES_INPUT_DATA_MAX_LENGTH]; 

	/* Out: 0 for success, otherwise failed. */
	uint32_t					unStatus;

	/* Out: the length in bytes of returned encrypted or decrypted message data */
	uint32_t					unOutputDataLen;

	/* Out: the returned encrypted or decrypted message data */
	unsigned char				aucOutputData[BCMD_USER_DES_AES_INPUT_DATA_MAX_LENGTH];

	
} BHSM_UserDesAesIO_t;


/*****************************************************************************
Summary:

This function is provided for MIPS host to use the DES, AES engine inside BSP to "accelarate" DES/AES processing. 
Since the performance is not very fast, may not be widely used.

Description:

The given data block in one function call can be a first piece, middle piece, last piece of a big message or a complete message.
( the way to hold data using a fixed buffer may be replaced with a data pointer later)

So far, only clear key support is available ( later, may add the secure key support reusing keyladder operation).

The supported algorithm includes DES_ECB, 3DES_ECB, AES128_ECB for this time. Actually it's very convenient to add 
the support of many more algorithms, and will be added when the requirement emerges.

Calling Context:

This function can be called any time after the system and BSP is initialized, when the applications decides to utilize the 
crypto engine inside BSP.


Performance and Timing:
This is a synchronous/blocking function that won'tl return until it is done or failed.  


Input:
in_handle  - BHSM_Handle, Host Secure module handle.
inoutp_userDesAesIO  - BHSM_UserDesAesIO_t, all members in the input section are always needed,
	'auKeyData' is always needed to provide the clear key, populated from the array top if not fully occupied
	'ucAlgorithm'  to pick up a BSP algorithm, DES_ECB/0, 3DESABA_ECB/1, AES128_ECB/2 			
	'ucOprMode'  to select  decryption or encryption processing at BSP
	'unInputDataLen' to state the actual length of message data piece to BSP, has to be padded depending on 'ucAlgorithm' choice 
	'aucInputData'  to provide a message data piece or a whole message
	
	the output section can be any value (normally input as 0).
			
Output:
inoutp_userDesAesIO  - BHSM_UserDesAesIO_t, the member in output section

	'unStatus' is modified to reflect the command processing status at BSP side, 0 for success, non-zero for certain error.
	'unOutputDataLen' is the length in bytes of returned encrypted or decrypted message data
	'aucOutputData' contains the returned encrypted or decrypted message data

Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_GenerateRouteKey
*****************************************************************************/
BERR_Code      BHSM_UserDesAes (
		BHSM_Handle			in_handle,
		BHSM_UserDesAesIO_t	*inoutp_userDesAesIO
);


#define BHSM_HMACSHA_KEY_LEN            32       /* 20 for Sha1, 28 for Sha224, 32 for Sha256 */

#define BHSM_HMACSHA_DIGEST_LEN	        32       /* in bytes, same as HMACSHA key length */

#define BHSM_HMACSHA_OP_KEY_MASK        0xFFFFFF /* 24 bits */
#define	BHSM_HMACSHA_OP_SHIFT           0x0
#define BHSM_HMACSHA_KL_SHIFT           0x8      /* bits   8-15 - Key Layer */
#define	BHSM_HMACSHA_VKL_SHIFT          0x10     /* bits 16-23 - Virtual Key Ladder */

#define BHSM_HMACSHA_TYPE_CXT_MASK      0xFFFFFF /* 24 bits */
#define BHSM_HMACSHA_CTX_SHIFT          0x0      /* bits  0 - 7 - BSP Context ID */
#define BHSM_HMACSHA_SHATYPE_SHIFT      0x8      /* bits 8 - 15 - SHA Type        */
#define BHSM_HMACSHA_DRAM_SHIFT         0x10     /* bits 16-23 - DRAM mode flag */

#define BHSM_HMACSHA_INCLUDEKEY_SHIFT	0x18	/* bits 25-31 - include key for sha */

typedef enum 
{
	BHSM_HMACSHA_ContinualMode_eAllSeg  = 0x00,
	BHSM_HMACSHA_ContinualMode_eMoreSeg = 0x01,
	BHSM_HMACSHA_ContinualMode_eLastSeg = 0x10

}BHSM_HMACSHA_ContinualMode_e;

typedef enum 
{
	BHSM_HMACSHA_DataSource_eCmdBuff = 0,
	BHSM_HMACSHA_DataSource_eDRAM    = 1

}BHSM_HMACSHA_DataSource_e;

typedef enum 
{
	BHSM_HMACSHA_KeySource_eKeyVal    = 0,
	BHSM_HMACSHA_KeySource_eKeyLadder = 1

}BHSM_HMACSHA_KeySource_e;

typedef struct BHSM_UserHmacShaIO 
{
	/* In: Which digest calculating mode: Sha-1/ShaX or HMAC */
	BPI_HmacSha1_Op_e			oprMode;
	/* In: Which SHA mode - Sha160(Sha1), Sha224, or Sha256  */
	BPI_HmacSha1_ShaType_e		shaType;
	/* In: Virtual Key Ladder ID - For HMAC calculation */
	BCMD_VKLID_e				VirtualKeyLadderID;
	/* In: Key Layer - For HMAC calculation */
	BCMD_KeyRamBuf_e			keyLayer;
	/* In: Which BSP internal context to use - Only 0xE or 0xF is valid here */
	BPI_HmacSha1_Context_e		contextSwitch;
	unsigned int				unIncludeKey;
	/* In: Which data source to use.  Only DRAM data is supported now */
	BHSM_HMACSHA_DataSource_e	dataSource;
	/* In: Which key source to use.  Embedded key or key ladder */
	BHSM_HMACSHA_KeySource_e	keySource;
	unsigned int				unKeyLength;
	/* In: clear key, valid only for HMAC-clear-key opr mode, 20 bytes of big endian byte array */	
	unsigned char				keyData	[BHSM_HMACSHA_KEY_LEN];  
	/* In: the length of the message data for digest calculation */
	unsigned int				unInputDataLen;
	/* In: DRAM address of the buffer holding data for digest calculation */		
	unsigned char				*pucInputData;

	/* In: data passed in for HMAC or SHA digest calculation can be contained in one buffer or multiple 
	    buffers.  In the case of single buffer data, this needs to be set to  BHSM_HMACSHA_ContinualMode_eAllSeg,
	    0x00.  In the case of multiple buffer data, all of the commands except the last one need to have this
	    field set to BHSM_HMACSHA_ContinualMode_eMoreSeg, 0x01.  The last command for the last buffer
	    needs to have this field set to BHSM_HMACSHA_ContinualMode_eLastSeg. */
	BHSM_HMACSHA_ContinualMode_e	contMode;

	/* Out: Status of the command */
	uint32_t					unStatus;
	/* Out: size of the Digest string */
	uint32_t                    digestSize;
	/* Out: SHA or HMAC digest, max 32 byte array, big endian */	
	unsigned char				aucOutputDigest[BHSM_HMACSHA_DIGEST_LEN];  

} BHSM_UserHmacShaIO_t;

/*****************************************************************************
Summary:

This function is to calculate out a digial digest for a given message, using SHA1 or HMAC with clear key or HMAC with secure key. 

Description:

The given message can be of any length as long as the memory operation is succesful. The supported clear key is 20 bytes in length though 
the max key length can be 64 bytes theorically. The supported secure key to be generated can be 64/128/192 bits in length (normally use 128 bits). 
The output digest is 20 bytes always for any SHA1 or HMAC operation.

SHA1 operation need no any key. HMAC clear key operation need a provided key and message data in a same function call. HMAC secure key operation 
need a preceding Keyladder1 operation to generate a key5 ( via calling BHSM_GenerateRouteKey function three times, provided 3 sets of key seed/procin),
when the Key5 of KeyLadder1 is ready, call this BHSM_UserHmacSha1 function to provide the message data to BSP.


Calling Context:

- this function with SHA1 operation can be called any time after the system and BSP is initialized.
- this function with HMAC-with-clear-key can be called any time after the system and BSP is initialized.
- this function with HMAC-with-secure-key can only be called after a successful Keyladder1 operation to generate a valid Key5.
(of course the system and BSP had been initialized.)


Performance and Timing:
This is a synchronous/blocking function that won'tl return until it is done or failed.  Some bench test shows
        -  HMAC,  key=20 bytes, data =187 bytes, time =2369 us, rate =93500 Bps
        -  HMAC,  key=20 bytes, data =48128 bytes, time =339965 us, rate =141970 Bps
        -  SHA1,  no key,    data =1024 bytes, time =8376 us, rate =128000 Bps
        -  SHA1,  no key,    data =32768 bytes, time =231412 us, rate =141852 Bps


Input:
in_handle  - BHSM_Handle, Host Secure module handle.
inoutp_userHmacSha1IO  - BHSM_UserHmacSha1IO_t, in the input section
	'oprMode' is always needed to select digest calculation mode,SHA1/0, HMAC-clear-key/1, HMAC-secure-key/2, others invalid
	      Keyladder1 operation to generate Key5 is mandatory in advance if to select HAMC-secure-key
	'contextSwitch' is always needed to pick up a BSP internal context to use:0x0c~0x0F only, others invalid.			
	'keyData' is only needed for HMAC-clear-key/1 as selected in the 1st member
	'unInputDataLen' is always needed to state the actual length of message data for digest calculation
	'pucInputData' is always needed to provide the message. If not the last piece of message data, must be multiple of 64bytes.
	
	the output section can be any value (normally input as 0).
			
Output:
inoutp_userHmacSha1IO  - BHSM_UserHmacSha1IO_t, the member 'unStatus' in output section will be modified 
to reflect the command processing status at BSP side, 0 for success, non-zero for certain error.

	'aucOutputData' contains the returned digest, 20 bytes of big endian byte array.

Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
BHSM_GenerateRouteKey
*****************************************************************************/
BERR_Code      BHSM_UserHmacSha (
		BHSM_Handle				in_handle,
		BHSM_UserHmacShaIO_t	*inoutp_userHmacShaIO
);


typedef struct BHSM_CancelCmdIO {
	/* input */
	

	/* Output */
	uint32_t					unStatus;

	
} BHSM_CancelCmdIO_t;


/*****************************************************************************
Summary:

This function is to cancel an ongoing processing inside BSP as requested by MIPS previously. Now some high prority application 
wants to cancel it so that new request(s) can be sent to BSP for handling.

Description:

This command is sent to BSP on a higher priority command buffer (or interface channel). Mostly it should return successfully in any case. 
The cancelled action was sent to BSP via a regular priority command buffer. An example of such cancelled action could be a lengthy message 
digest calculation.

Calling Context:

This function is called when a previous request to BSP has being processed for a pretty long time, still no return or a higher 
priority application wants to use BSP to do something urgent.

Performance and Timing:
This is a synchronous/blocking function that won'tl return until it is done or failed. No testing data is available, and no  
estimated time either.

Input:
in_handle  - BHSM_Handle, Host Secure module handle.
inoutp_cancelCmdIO  - BHSM_CancelCmdIO_t, no any specific parameters are needed in input section.
					the output section can be any value (normally input as 0).
			
Output:
inoutp_cancelCmdIO  - BHSM_CancelCmdIO_t, the member 'unStatus' in output section will be modified 
to reflect the command processing status at BSP side, 0 for success, non-zero for certain error.

Returns:
BERR_SUCCESS - success
BHSM_STATUS_FAILED - failure

See Also:
*****************************************************************************/
BERR_Code      BHSM_CancelCmd (
		BHSM_Handle			in_handle,
		BHSM_CancelCmdIO_t	*inoutp_cancelCmdIO
);

/* End of Module Specific Functions */



#ifdef __cplusplus
}
#endif

#endif /* BHSM_USER_CMDS_H__ */

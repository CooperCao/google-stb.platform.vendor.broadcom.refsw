/***************************************************************************
 *     Copyright (c) 2005-2009, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 * Security Processor Code for Register Access
 ***************************************************************************/

#ifndef BSP_S_PKE_H__
#define BSP_S_PKE_H__


/* Max size of data length for each type of packet. */
#define BCMD_PKE_DH_MAX_DATA_LENGTH     344
#define BCMD_PKE_DSA_MAX_DATA_LENGTH    336
#define BCMD_PKE_RSA_MAX_DATA_LENGTH    344


/* Common status field for all PKE output buffers. */
typedef enum BCMD_PKE_CommonOutFields_e
{
    BCMD_PKE_CommonOutFields_eStatus = (5<<2) + 3   /* 8 bit index */
}BCMD_PKE_CommonOutFields_e;

/*

BCMD_PKE_ExecuteDH - Perform DH public or secret key generate.
Public Key Generate:
public key = g^x mod p
Secret Key Generate:
Shared secret key = (public key from other party)x mod p

Parameters passed:
eOp - perform public or secret key generation or pass through
	mode (send eData directly to public key engine).
eModSrcIdx - index to key buffer containing modulus p.
eBaseSrcIdx - index to key buffer containing base g for public key
	generate or index to store secret key (instead of storing secret
	key to buffer, it can also be returned through output buffer).
ePrivateKeySrcIdx - index to private key x.
eDataLen - length of data contained in eData.  For public key
	generate, no data is passed.  For secret key, the public key
	of other host is passed through here.
eData - pass through data or public key of other party.
	
Returned:
eStatus - Status of command.
eKeyLen - Length of key in bits.
eKeyBase - public key or private key.

BCMD_PKE_DHInputCmdField_e: 
|-------------------|-------------------|-------------------|-------------------| 
|  31..24           | 23..16            | 15..8             | 7..0             	| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused           | unused            | unused            | eOp - 8			| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused           | unused            | unused            | eModSrcIdx - 8	| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused           | unused            | unused            | eBaseSrcdx - 8	| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused           | unused            | unused            |ePrivateKeySrcIdx-8| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused           | unused            | eDataLen - 16							| 
|-------------------|-------------------|-------------------|-------------------| 
|  eData - eDataLen*8															| 
|-------------------|-------------------|-------------------|-------------------| 

    
BCMD_PKE_DHOutCmdField_e: 
|-------------------|-------------------|-------------------|-------------------| 
|  31..24           | 23..16            | 15..8             | 7..0             	| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused			|  unused			|  unused			| eStatus -	8		|
|-------------------|-------------------|-------------------|-------------------| 
|  unused			|  unused			|  eKeyLen - 16							|
|-------------------|-------------------|-------------------|-------------------| 
|  eKeyBase - eKeyLen															|
|-------------------|-------------------|-------------------|-------------------| 

*/
typedef enum BCMD_PKE_DHInputCmdField_e
{
    BCMD_PKE_DHInputCmdField_eOp            = (5<<2) + 3,   /* 8 bit index */
    BCMD_PKE_DHInputCmdField_eModSrcIdx          = (6<<2) + 3,   /* 8 bit index */
    BCMD_PKE_DHInputCmdField_eBaseSrcIdx         = (7<<2) + 3,   /* 8 bit index */
    BCMD_PKE_DHInputCmdField_ePrivateKeySrcIdx  = (8<<2) + 3,   /* 8 bit index */
#if defined(BSP_DEBUG_PKE_HAL_UNIT_TEST)
    BCMD_PKE_DHInputCmdField_eMask         = (7<<2) + 3,   /* 8 bit index */
    BCMD_PKE_DHInputCmdField_eMaskedValue  = (8<<2) + 3,   /* 8 bit index */
#endif    
    BCMD_PKE_DHInputCmdField_eDataLen          = (9<<2) + 2,   /* 16 bit index */
    BCMD_PKE_DHInputCmdField_eData            = (10<<2) + 0   /* 32 bit index */
}BCMD_PKE_DHInputCmdField_e;

typedef enum BCMD_PKE_DHOutCmdField_e
{
    BCMD_PKE_DHOutCmdField_eKeyLen = (6<<2) + 2,   /* 16 bit */
    BCMD_PKE_DHOutCmdField_eKeyBase = (7<<2) + 0   /* 32 bit */
}BCMD_PKE_DHOutCmdField_e;

/*

BCMD_PKE_ExecuteDSA - Perform DSA sign or verify
Sign:
r = (g^k mod p) mod q
s = (k^-1(h + xr)) mod q
Verify:
w = s^-1 mod q
u1 = ((hw) mod q
u2 = (rw) mod q
v = ((g^(u1)y^(u2)) mod p) mod q

Parameters passed:
eOp - sign, verify, or pass through mode.  In pass through mode, the data from
	eData is sent directly to the public key engine).
ePKeySrc - index to public key P.
eGKeySrc - index to public key G.
eQKeySrc - index to public key Q.
ePrivKeyX - index to private key X.
ePrivKeyK - index to private key K.
eDataLen - Length of hash for sign or length of public key y
	hash, and r,s when verify.
eData - Data.
	
Returned:
eStatus - Status of command.
eData - (r,s) for sign or v for verify

BCMD_PKE_DSAInCmdField_e: 
|-------------------|-------------------|-------------------|-------------------| 
|  31..24           | 23..16            | 15..8             | 7..0             	| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused           | unused            | unused            | eOp - 8			| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused           | unused            | unused            | ePKeySrc - 8		| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused           | unused            | unused            | eGKeySrc - 8		| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused           | unused            | unused            | eQKeySrc - 8		| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused           | unused            | unused            | ePrivKeyX - 8		| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused           | unused            | unused            | ePrivKeyK - 8		| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused			| unused			| eDataLen - 16							|																	| 
|-------------------|-------------------|-------------------|-------------------|
| eData	- eDataLen*8															|
|-------------------|-------------------|-------------------|-------------------|
    
BCMD_PKE_DSAOutCmdField_e: 
|-------------------|-------------------|-------------------|-------------------| 
|  31..24           | 23..16            | 15..8             | 7..0             	| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused			|  unused			|  unused			| eStatus -	8		|
|-------------------|-------------------|-------------------|-------------------| 
|  eData - 320 (r,s) or 160 (v)															|
|-------------------|-------------------|-------------------|-------------------| 

*/
typedef enum BCMD_PKE_DSAInCmdField_e
{
    BCMD_PKE_DSAInCmdField_eOp       = (5<<2) + 3,   /* 8 bit index */
    BCMD_PKE_DSAInCmdField_ePKeySrc       = (6<<2) + 3,   /* 8 bit index */
    BCMD_PKE_DSAInCmdField_eGKeySrc       = (7<<2) + 3,   /* 8 bit index */
    BCMD_PKE_DSAInCmdField_eQKeySrc       = (8<<2) + 3,   /* 8 bit index */
    BCMD_PKE_DSAInCmdField_ePrivKeyX   = (9<<2) + 3,   /* 8 bit index */
    BCMD_PKE_DSAInCmdField_ePrivKeyK   = (10<<2) + 3,   /* 8 bit index */
    BCMD_PKE_DSAInCmdField_eDataLen        = (11<<2) + 2,   /* 16 bit index */
    BCMD_PKE_DSAInCmdField_eData       = (12<<2) + 0    /* 32 bit index */
}BCMD_PKE_DSAInCmdField_e;

typedef enum BCMD_PKE_DSAOutCmdField_e
{
    BCMD_PKE_DSAOutCmdField_eData   = (6<<2) + 0    /* 32 bit index */
}BCMD_PKE_DSAOutCmdField_e;

/*

BCMD_PKE_ExecuteRSA - Perform encrpyt and decrypt.
Normal:
cleartext = c^e mod n.
Chinese Remainder Theorem:
v1 = md mod (p-1) mod p
v2 = md mod (q-1) mod q

C2 = p-1 mod q
u = (v2 - v1) C2 mod q
v1 + up = md mod n

Parameters passed:
eOp - normal, crt, or pass through (data is
	sent directly to public key engine).
eKeySrcIdx1 - index for source n (decrypt) or p and q (encrypt).
eExpSrcIdx - index for exponent e (decrypt) or pinv (encrypt).
eKeySrcIdx2 - index for dp and dq (encrypt).
eDataLen - length of clear text, cipher text, or pass through data.
eData - clear text, cipher text, or pass through data.
	
Returned:
eStatus - Status of command.
eDataLen - length of clear or cipher in bits.
eData - clear or cipher.

BCMD_PKE_RSAInCmdField_e: 
|-------------------|-------------------|-------------------|-------------------| 
|  31..24           | 23..16            | 15..8             | 7..0             	| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused           | unused            | unused            | eOp - 8			| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused           | unused            | unused            | eKeySrcIdx1 - 8	| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused           | unused            | unused            | eExpSrcIdx - 8	| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused           | unused            | unused            | eKeySrcIdx2 - 8	| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused			| unused			| eDataLen - 16							|																	| 
|-------------------|-------------------|-------------------|-------------------|
| eData	- eDataLen*8															|
|-------------------|-------------------|-------------------|-------------------|
    
BCMD_PKE_RSAOutCmdField_e: 
|-------------------|-------------------|-------------------|-------------------| 
|  31..24           | 23..16            | 15..8             | 7..0             	| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused			|  unused			|  unused			| eStatus -	8		|
|-------------------|-------------------|-------------------|-------------------| 
|  unused			|  unused			|  eDataLen - 16						|
|-------------------|-------------------|-------------------|-------------------| 
|  eData - eDataLen																|
|-------------------|-------------------|-------------------|-------------------| 

*/
typedef enum BCMD_PKE_RSAInCmdField_e
{
    BCMD_PKE_RSAInCmdField_eOp   = (5<<2) + 3,   /* 8 bit index */
    BCMD_PKE_RSAInCmdField_eKeySrcIdx1= (6<<2) + 3,   /* 8 bit index */
    BCMD_PKE_RSAInCmdField_eExpSrcIdx = (7<<2) + 3,   /* 8 bit index */
    BCMD_PKE_RSAInCmdField_eKeySrcIdx2= (8<<2) + 3,   /* 8 bit index */
    BCMD_PKE_RSAInCmdField_eDataLen = (9<<2) + 2,   /* 16 bit index */
    BCMD_PKE_RSAInCmdField_eData   = (10<<2) + 0   /* 32 bit index */
}BCMD_PKE_RSAInCmdField_e;

typedef enum BCMD_PKE_RSAOutCmdField_e
{
    BCMD_PKE_RSAOutCmdField_eDataLen   = (6<<2) + 2,   /* 16 bit index */
    BCMD_PKE_RSAOutCmdField_eData   = (7<<2) + 0   /* 32 bit index */
}BCMD_PKE_RSAOutCmdField_e;

/* PKE opcodes */
typedef enum BPI_PKE_Opcode_e
{
    BPI_PKE_Opcode_eDHPublicKeyGen = 1,
    BPI_PKE_Opcode_eDHPrivateKeyGen,
    BPI_PKE_Opcode_eRSANormal,
    BPI_PKE_Opcode_eRSACrt,
    BPI_PKE_Opcode_eDSASign,
    BPI_PKE_Opcode_eDSAVerify
}BPI_PKE_Opcode_e;

/* BAMCM_USER_DH packet operations */
typedef enum BPI_PKE_DHOperation_e
{
    BPI_PKE_DHOperation_ePublicKeyGen,
    BPI_PKE_DHOperation_eSecretKeyGen
}BPI_PKE_DHOperation_e;

/* BAMCM_USER_RSA packet operations */
typedef enum BPI_PKE_RSAOperation_e
{
    BPI_PKE_RSAOperation_eNormal,
    BPI_PKE_RSAOperation_eCrt
}BPI_PKE_RSAOperation_e;

/* BAMCM_USER_DSA packet operations */
typedef enum BPI_PKE_DSAOperation_e
{
    BPI_PKE_DSAOperation_ePrivateKeySign,
    BPI_PKE_DSAOperation_ePublicKeyVer
}BPI_PKE_DSAOperation_e;

/* PKE Specific command field definitions */
#define BPI_PKE_PASS_THROUGH	0xFF

/*

BCMD_cmdType_ePKE_Cmd_Poll_Status: Polling cmd to find status of outstanding PKE cmd.

Parameters passed:
None
	
Returned:
eStatus - Status of command.
eData - Data returned from last PKE cmd, if the last PKE cmd. is finished.

4 Possible Status error codes:
	BSP_STATUS_eSuccess(0x00) : Last PKE cmd has finished and the output follows in the cmd output buffer.
    BCMD_STATUS_ePKEMoreData(0xa3): BSP is waiting for next PKE cmd which will be continuation of previous PKE cmd.           
    BCMD_STATUS_ePKEInProgress(0xa4): BSP is working on the last PKE cmd. A new PKE cmd will be rejected at this time. A non-PKE cmd can be sent to the BSP user cmd thread.
	BCMD_STATUS_ePKEInactive(0xda): BSP user cmd thread, PKE cmds are inactive. It is ready to process a new PKE cmd. 

BCMD_PKE_CmdPollInCmdField_e: 
|-------------------|-------------------|-------------------|-------------------| 
|  31..24           | 23..16            | 15..8             | 7..0             	| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused           | unused            | unused            | unused			| 
|-------------------|-------------------|-------------------|-------------------| 
    
BCMD_PKE_CmdPollOutCmdField_e: 
|-------------------|-------------------|-------------------|-------------------| 
|  31..24           | 23..16            | 15..8             | 7..0             	| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused			|  unused			|  unused			| eStatus -	8		|
|-------------------|-------------------|-------------------|-------------------| 
|  eData - eDataLen																|
|-------------------|-------------------|-------------------|-------------------| 

*/

/* No input parameters required */
/* typedef enum BCMD_PKE_CmdPollInCmdField_e
{
}BCMD_PKE_RSAInCmdField_e;
*/

typedef enum BCMD_PKE_CmdPollOutCmdField_e
{
    BCMD_PKE_CmdPollOutCmdField_estatus = (5<<2) + 3,   /* 8 bit status */
    BCMD_PKE_CmdPollOutCmdField_eData   = (6<<2) + 0   /* 32 bit index */
}BCMD_PKE_CmdPollOutCmdField_e;


#endif /* BSP_S_PKE_H__ */


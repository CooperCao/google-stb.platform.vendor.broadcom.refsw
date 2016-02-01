/***************************************************************************
 *     Copyright (c) 2005-2013, Broadcom Corporation
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

#ifndef BSP_S_HMAC_SHA1_H__
#define BSP_S_HMAC_SHA1_H__


/* The max size the HMAC SHA1 data length can be. */
#define BCMD_HMACSHA1_MAX_DATA_LEN 332 /* 332 bytes */
#define BPI_HMACSHA1_DIGEST_BYTE_SIZE   20


/*

BCMD_HmacSha1_ExecuteCommand - Request to calculate HMAC of SHA1 digest

Parameters passed:
eOperation - Perform either HMAC or SHA1.
eContextId - which context to perform digest calculation.
eKeyData - 160 bits, 224 bits or 256 bits start key to use if HMAC digest requested
           depending on if SHA160, SHA224 or SHA256 needs to be performed
eDataLen - Length of data to follow that will be used to calculate HMAC or SHA1.
eData - Data used for calculation.
	
Returned:
eStatus - Status of command.
eDigest - 160 bits, 224 bits or 256 bits of calculated digest.

BCMD_HmacSha1_InCmdField_e: 
|-------------------|-------------------|-------------------|-------------------| 
|  31..24           | 23..16            | 15..8             | 7..0             	| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused           | unused            | unused            | eOperation - 8	| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused           | eIsAddress - 8    | eShaType - 8      | eContextId - 8	| 
|-------------------|-------------------|-------------------|-------------------| 
|  eKeyData - 160 bits (SHA160), 224 bits (SHA224) or 256 bits (SHA256)         | 
|-------------------|-------------------|-------------------|-------------------| 
|  																				| 
|-------------------|-------------------|-------------------|-------------------| 
|  																				| 
|-------------------|-------------------|-------------------|-------------------| 
|  																				| 
|-------------------|-------------------|-------------------|-------------------| 
|  																				| 
|-------------------|-------------------|-------------------|-------------------|
|                               eDataLen - 32							        | 
|-------------------|-------------------|-------------------|-------------------|
|  eData - eDataLen*8 (if eAddress == 0)                                        | 
|  eAddress - 32      (if eAddress == 1)                                        |
|-------------------|-------------------|-------------------|-------------------|
    
BCMD_HmacSha1_OutCmdField_e: 
|-------------------|-------------------|-------------------|-------------------| 
|  31..24           | 23..16            | 15..8             | 7..0             	| 
|-------------------|-------------------|-------------------|-------------------| 
|  unused			|  unused			|  unused			| eStatus -	8		|
|-------------------|-------------------|-------------------|-------------------| 
|  eDigest - 160 bits (SHA160), 224 bits (SHA224) or 256 bits (SHA256)          |
|-------------------|-------------------|-------------------|-------------------| 
|  																				| 
|-------------------|-------------------|-------------------|-------------------| 
|  																				| 
|-------------------|-------------------|-------------------|-------------------| 
|  																				| 
|-------------------|-------------------|-------------------|-------------------| 
|  																				| 
|-------------------|-------------------|-------------------|-------------------|

*/
#define USER_SHA_KEY_INCLUDE
#define SHA_256
#define USER_SHA_FIPS_KEY_256 

typedef enum BCMD_HmacSha1_InCmdField_e
{
	BCMD_HmacSha1_InCmdField_eVKL           = (5<<2) + 1,   /* 8 bit index */
	BCMD_HmacSha1_InCmdField_eKeyLayer      = (5<<2) + 2,   /* 8 bit index */
    BCMD_HmacSha1_InCmdField_eOperation     = (5<<2) + 3,   /* 8 bit index */
#if defined(USER_SHA_KEY_INCLUDE)
    BCMD_HmacSha1_InCmdField_eIncludeKey    = (6<<2) + 0,    /* 8 bit index */ 
#endif /* USER_SHA_KEY_INCLUDE */
    BCMD_HmacSha1_InCmdField_eIsAddress     = (6<<2) + 1,   /* 8 bit index */ 
#if defined(SHA_256)
    BCMD_HmacSha1_InCmdField_eShaType       = (6<<2) + 2,   /* 8 bit index */ 
#endif    
    BCMD_HmacSha1_InCmdField_eContextId     = (6<<2) + 3,   /* 8 bit index */
#if defined(USER_SHA_FIPS_KEY_256)
    BCMD_HmacSha1_InCmdField_eKeyLen        = (7<<2) + 3,
    BCMD_HmacSha1_InCmdField_eKeyData       = (8<<2) + 0    /* 160 bits, 224 bits or 256 bits depending on eShaType */
#else
    BCMD_HmacSha1_InCmdField_eKeyData       = (7<<2) + 0,    /* 160 bits, 224 bits or 256 bits depending on eShaType */
#endif /* USER_SHA_FIPS_KEY_256 */
#if defined(SHA_256)
    /* eDataLen and eData locations calculated depending on eShaType */
#else
    BCMD_HmacSha1_InCmdField_eDataLen       = (12<<2) + 0,  /* 32 bit index */
    BCMD_HmacSha1_InCmdField_eData          = (13<<2) + 0,  /* 32 bit index */
    BCMD_HmacSha1_InCmdField_eAddress       = (13<<2) + 0   /* 32 bit index */
#endif    
}BCMD_HmacSha1_InCmdField_e;

typedef enum BCMD_HmacSha1_OutCmdField_e
{
    BCMD_HmacSha1_OutCmdField_eStatus = (5<<2) + 3,     /* 8 bit */
    BCMD_HmacSha1_OutCmdField_eDigest = (6<<2) + 0      /* 160 bits for old SHA1 engine */
                                                        /* 160 bits, 224 bits or 256 bits depending on eShaType */
                                                        /* for new SHA2 engine */
}BCMD_HmacSha1_OutCmdField_e;

typedef enum BPI_HmacSha1_Op_e
{
    BPI_HmacSha1_Op_eSha1,
    BPI_HmacSha1_Op_eHmac
}BPI_HmacSha1_Op_e;

/* 
 * The following define is only used for chips with SHA-256 feature
 * such as 7420, 7125, 7340, 7342 etc.
 * For all older chips, only SHA1 (BPI_HmacSha1_ShaType_eSha160) is used
 */
typedef enum BPI_HmacSha1_ShaType_e
{
    BPI_HmacSha1_ShaType_eSha160 = 0,
    BPI_HmacSha1_ShaType_eSha224 = 1,
    BPI_HmacSha1_ShaType_eSha256 = 2,
    BPI_HmacSha1_ShaType_eMax
}BPI_HmacSha1_ShaType_e;

#if defined(USER_SHA_KEY_INCLUDE)
typedef enum BPI_HmacSha1_IncludeKey_e
{
    BPI_HmacSha1_IncludeKey_eNo          = 0x0,
    BPI_HmacSha1_IncludeKey_eAppend      = 0x1,
    BPI_HmacSha1_IncludeKey_eMax         = 0x2
} BPI_HmacSha1_IncludeKey_e;
#endif /* USER_SHA_KEY_INCLUDE */

typedef enum BPI_HmacSha1_Context_e
{
    BPI_HmacSha1_Context_eCpu0 = 0x0,      /* MIPS boot region */
    BPI_HmacSha1_Context_eCpu1 = 0x1,      /* MIPS region 1 */
    BPI_HmacSha1_Context_eCpu2 = 0x2,      /* MIPS region 2 */
    BPI_HmacSha1_Context_eCpu3 = 0x3,      /* MIPS region 3 */
    BPI_HmacSha1_Context_eCpu4 = 0x4,      /* MIPS region 4 */
    BPI_HmacSha1_Context_eCpu5 = 0x5,      /* MIPS region 5 */
    BPI_HmacSha1_Context_eCpu6 = 0x6,      /* MIPS region 6 */
    BPI_HmacSha1_Context_eCpu7 = 0x7,      /* MIPS region 7 */
    BPI_HmacSha1_Context_eAudio0 = 0x8,    /* Audio region */
    BPI_HmacSha1_Context_eVideo0 = 0x9,    /* Video0 Decoder */
 
#if defined(SECOND_AVD)
    BPI_HmacSha1_Context_eVideo1 = 0xA,    /* Video1 Decoder */
#else
    BPI_HmacSha1_Context_eReserved1 = 0xA, /* Reserved when only 1 AVD */
#endif    

    BPI_HmacSha1_Context_eRegionMax = 0xB, /* The last context representing a checkable region */
    
    BPI_HmacSha1_Context_eRave0 = 0xB,     /* Input command signature calculation region */
    BPI_HmacSha1_Context_eCmdSig = 0xC,
    
    BPI_HmacSha1_Context_eReserved2 = 0xD,     /* BRCM reserved */

    BPI_HmacSha1_Context_eHmacSha1Ctx0 = 0xE,     /* HMAC_SHA1 cmd */
    BPI_HmacSha1_Context_eHmacSha1Ctx1 = 0xF,     /* HMAC_SHA1 cmd */    
        
    BPI_HmacSha1_Context_eMax        
}BPI_HmacSha1_Context_e;


/* Note : IMP - Below contexts cannot NOT be used by continual mode commands
          MSIPTV Uses the following two context. The contexts are used only with a single command
          and other commands can reuse the contexts if needed (in non-continual mode).
*/
typedef enum BCMD_MSIPTV_HmacSha1_Context_e{

    BCMD_MSIPTV_HmacSha1_Context_eCtx0 = BPI_HmacSha1_Context_eCmdSig,
    BCMD_MSIPTV_HmacSha1_Context_eCtx1 = BPI_HmacSha1_Context_eReserved2 
}BCMD_MSIPTV_HmacSha1_Context_e;


#endif /* BSP_S_HMAC_SHA1_H__ */


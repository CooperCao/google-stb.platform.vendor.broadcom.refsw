/********************************************************************************************
*     (c)2004-2012 Broadcom Corporation                                                     *
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
 * DTCP base type definations
 * 
 * Revision History:
 * 
 * $brcm_Log: $
 * 
 *********************************************************************************************/
/*! \file b_dtcp_types.h
 *  \brief Define various DTCP base types.
 */
#ifndef __B_DTCP_TYPES_H__
#define __B_DTCP_TYPES_H__

#include "bcrypt.h"
#include "bcrypt_ecdsa_sw.h"
#include "b_dtcp_constants.h"

/*! \enum B_CertFormat
 *  \brief Device Certificate Format enumeration.
 */
typedef enum B_CertFormat
{
	B_CertFormat_eRestricted = 0,	/*!<Restricted authentication certificate.*/
	B_CertFormat_eBaselineFull,		/*!<Baseline full authentication certificate.*/
	B_CertFormat_eExtendedFull		/*!<Extended full authentication certificate.*/
}B_CFormat_T;


/*! \enum B_EMI_T
 *  \brief Encryption Mode Indicator enum
 */
typedef enum B_EMI
{
	B_Emi_eCopyFree = 0,
	B_Emi_eNoMoreCopy,
	B_Emi_eCopyOneGeneration,
	B_Emi_eCopyNever
}B_EMI_T;
/*! \enum B_ExEMI_T 
 *  \brief extended EMI (for DTCP-IP) enum
 */
typedef enum B_ExEMI
{
	B_ExtEmi_eCopyFree = 0,
	B_ExtEmi_eCopyFreeEPN = 2,
	B_ExtEmi_eNoMoreCopy = 4,
	B_ExtEmi_eMove = 6,
	B_ExtEmi_eCopyOneFN= 8,
	B_ExtEmi_eCopyOneFC = 10,
	B_ExtEmi_eCopyNever = 12,
	B_ExtEmi_eInvalid = 0xff
}B_ExEMI_T;
	

/*! \enum B_AkeType
 */
typedef enum B_AkeType
{
	B_AkeType_eRestricted = 1,
	B_AkeType_eEnRestricted = 0x2,
	B_AkeType_eFull = 0x4,
	B_AkeType_eExtendedFull = 0x8
}B_AkeType_T;

/*! \enum B_ExchangeKeyType
 *  \brief Type of the exchange key.
 */
typedef enum B_ExchangeKeyType
{
	B_ExchKey_eM6CopyNever = 1,
	B_ExchKey_eM6CopyOne = 0x2,
	B_ExchKey_eM6NoMoreCopy = 0x4,
	B_ExchKey_eAES128 = 0x8
}B_ExchangeKeyType_T;

/*! \enum B_ExchangeKeyCipher
 *  \brief Cipher algorithm for exchange key
 */
typedef enum B_ExchangeKeyCipher
{
	B_ExchKeyCipher_eM6,
	B_ExchKeyCipher_eAes
}B_ExchangeKeyCipher_T;

/*! \struct B_BaselineFullCert
 *  \brief Baseline full auth. certificate 
 */
typedef struct B_BaselineFullCert
{
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
	int CertType	: 4;			/*!<only valid value is 0 , for DTCP */
	int Format		: 4;			/*!<Certificate Format, refer to B_CertFormat */
	int DevGen		: 4;			/*!<Device generation */
	int Reserved	: 10;
	int AL			: 1;			/*!<Addional localization flag */
	int AP			: 1;			/*!<Authentication proxy flag.*/
	unsigned char DevID[5];			/*!<Device ID assigned by DTLA */
	unsigned char PublicKey[40];	/*!<EC-DSA public key */
	unsigned char Signature[40];	/*!<EC-DSA signature for preceding fields */
#else
	int Format		: 4;
	int CertType	: 4;
	int Reserved	: 4;
	int DevGen		: 4;
	int AP			: 1;
	int AL			: 1;
	int Resevred1	: 6;
	unsigned char DevID[5];	
	unsigned char PublicKey[40];
	unsigned char Signature[40];
#endif
} __attribute__((__packed__)) B_BaselineFullCert_T;

/*! \struct B_ExtFullCert
 *  \brief Extended full auth. certificate
 */
typedef struct B_ExtFullCert
{
	B_BaselineFullCert_T Baseline;
	unsigned int Capibility_mask;
	unsigned char Signature[40];
}B_ExtFullCert_T;

/*! \struct B_RestrictedCert
 *  \brief Restricted auth. certificate.
 */
typedef struct B_RestrictedCert
{
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
	int CertType	: 4;			/*!< only valid value is 0, for DTCP */
	int Format		: 4;			/*!< Certificate format, refert to B_CertFormat_T */
	int Reserved	: 3;			/*!< reserved.*/
	int AL			: 1;			/*!< Addional locatization flag*/
	int K_Vector	: 12;			/*!< key selection vector*/
	unsigned char DevID[5];			/*!< device ID assigned by DTLA */
	unsigned char Signature[40];	/*!< EC_DSA signature for preceding fields*/
#else
	int Format		: 4;
	int CertType	: 4;
	int K_Vector_MSB: 4;
	int AL			: 1;
	int Reserved	: 3;
	int	K_Vector_LSB: 8;
	unsigned char DevID[5];
	unsigned char Signature[40];
#endif
} __attribute__((__packed__)) B_RestrictedCert_T;
	
/*! \struct B_Srm_Header
 *  \brief System renewability message  header.
 */
typedef struct B_SRM_Header
{
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
	int Type	: 4;				/*! Type, same as Device certificate type, always 0 */
	int Gen		: 4;				/*! Generation of SRM  0 or 1 */
#else
	int Gen		: 4;
	int Type	: 4;
#endif
	unsigned char Reserved;
	unsigned short Version;			/*! Version number , 2 bytes. */
	unsigned short Length;			/*! CRL Length, 2 bytes */
} __attribute__((__packed__)) B_Srm_Header_T;

/*! \struct B_Srm
 *  \brief System renewability message.
 */
typedef struct B_Srm
{
#define DTCP_CRL_NUMBER_MASK	0x1F
#define DTCP_CRL_TYPE_SIZE		1
#define DTCP_CRL_TYPE_MASK		0x03
#define DTCP_CRL_TYPE_SHIFT		0x05
#define DTCP_CRL_TYPE_ID		0
#define DTCP_CRL_TYPE_IDS		1
	B_Srm_Header_T SrmHeader;		/* Srm header, 4 bytes */
	unsigned char  Crl[DTCP_SRM_SECOND_GEN_MAX_SIZE];			/* Crl entry start , variable size*/
}B_Srm_T;

/*! \struct B_DeviceParams 
 *  \brief Device parameters.
 */
typedef struct B_DeviceParams
{
	void * Cert;					/*!<Device certificate pointer, baseline full/ext full or Restricted*/
	int CertSize;					/*!<Size of the device certificate. */
	B_Srm_T *  Srm;					/*!<SRM message */
	int SrmSize;					/*!<Size of SRM message */
	unsigned char PublicKey[DTCP_PUBLIC_KEY_SIZE];		/*!<Device public key obtained from certificate*/
	unsigned char PrivateKey[DTCP_PRIVATE_KEY_SIZE];	/*!<Device private key */
	unsigned char dtlaPublicKey[DTCP_PUBLIC_KEY_SIZE];	/*!<DTLA public key */
	
	BCRYPT_ECCParam_t EccParams;	/*!<Ecc parameters */
	
	int BridgeDevice;				/*!< Is the device a bridge device ?*/
	int CommonDeviceCert;			/*!< is the device using a common device certificate?*/
	unsigned int capability;		/*!< Is the source device capable of processing Idu ?*/
	unsigned int pcp_ur_capability;	/*!< Is the source device capable of processing PCP_UR flag? */
	unsigned char IDu[DTCP_DEVICE_ID_SIZE];
	int IDuSize;
	B_DeviceMode_T Mode;			/*!<source/sink device */
	
}B_DeviceParams_T;

#endif /* __B_DTCP_TYPES_H__ */

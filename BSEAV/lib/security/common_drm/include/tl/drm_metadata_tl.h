/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
*******************************************************************************/

#ifndef __DRM_METADATA_TL_H__
#define __DRM_METADATA_TL_H__


/* ****************************  THIS IS REFERENCE DRM_METADATA ***************/

/*
//define for mainline DRM bin file size
*/
#define DRM_BIN_FILE_SIZE					(128*1024)


/*
//   DRM Types.  The following enum must match the SAGE side
*/
typedef enum DrmCommon_BinFileDrmType_e
{
	BinFileDrmType_eNetflix = 0x00010001,
	BinFileDrmType_eWidevine,
	BinFileDrmType_eDtcpIp,
	BinFileDrmType_ePlayready,
	BinFileDrmType_eSecureSwRsa,
	BinFileDrmType_eCustomPrivate,
	BinFileDrmType_eAdobeAxcess,
	BinFileDrmType_eHdcp22Rx,
	BinFileDrmType_eHdcp22Tx,
	BinFileDrmType_eSslCerts,
  BinFileDrmType_eGeneric,
  BinFileDrmType_eEdrm,
  BinFileDrmType_eEcc,
	BinFileDrmType_eMax

} DrmCommon_BinFileDrmType_e;

/********************************************************
	 Adobe definitions
*********************************************************/
typedef enum DrmAdobe_DataField_e
{
	DrmAdobe_DataField_eDrmPrivKey = 0,
	DrmAdobe_DataField_eDrmPublicKey = 1,
	DrmAdobe_DataField_eSigningPrivateKey = 2,
	DrmAdobe_DataField_eSigningPublicKey = 3,
	DrmAdobe_DataField_eDecryptionCertificate = 4,
	DrmAdobe_DataField_eSigningCertificate = 5,
	DrmAdobe_DataField_eMax
}DrmAdobe_DataField_e;

typedef struct drm_adobe_axcess4_0_data_t
{
	unsigned char	drm_decryption_private_key_size[16];
	unsigned char	*drm_decryption_private_key;
	unsigned char	drm_decryption_public_key_size[16];
	unsigned char	*drm_decryption_public_key;

	unsigned char	drm_signing_private_key_size[16];
	unsigned char	*drm_signing_private_key;
	unsigned char	drm_signing_public_key_size[16];
	unsigned char	*drm_signing_public_key;

	unsigned char	drm_decryption_cert_size[16];
	unsigned char	*drm_decryption_cert;

	unsigned char	drm_signing_cert_size[16];
	unsigned char	*drm_signing_cert;

}drm_adobe_axcess4_0_data_t;


/******************************************************************************************************************************
**  DRM Region header:
**		The DRM region header marks the beginning of every DRM/PMC data section
**		It contains the following fields:
** 		- DRM/PMC Type: DrmCommon_DataType_e
**		- Encrypted RPK: field containing the wrapped RPK used to encrypt the data in the data section
**		- Encrypted IV0: encrypted 16 bytes representing the initial IV used to wrap the data in the current data section.
**		- Number of fields: indicates the number of size of & data pairs to follow the data section
******************************************************************************************************************************/
#define ENCRYPTED_RPK_LENGTH (16)
#define ENCRYPTED_IV0_LENGTH (16)
#define NUM_SUPPORTED_TYPE_FIELD_LENGTH (16)

typedef struct data_region_header_t 
{
	 unsigned char data_type[16];
	 unsigned char encrypted_rpk[ENCRYPTED_RPK_LENGTH];
	 unsigned char encrypted_iv0[ENCRYPTED_IV0_LENGTH];
	 unsigned char number_of_fields[16];
} data_region_header_t;

/* Size of SHA256 at the end of every data section */
#define ENCRYPTED_DATA_SECTION_SHA256_SIGNATURE_SIZE (32)


/* DRM bin file header definitions */

#define FILE_SIZE_OFFSET			(80)
#define BINDING_MARKER_OFFSET		(160)
#define NUM_SUPPORTED_DRM_OFFSET	(192)

typedef struct drm_bin_header_t
{
	unsigned char cookie[16];
	
	unsigned char cust_key_select;
	unsigned char key_var_low;
	unsigned char key_var_high;
	unsigned char root_key_type;
	unsigned char procIn_generation;
	unsigned char padding[11];
	
	unsigned char proc_in1[16];
	unsigned char proc_in2[16];
	unsigned char proc_in3[16];

	unsigned char bin_file_size[16];

	unsigned char askm_parameters[16];

	unsigned char reserved[48];
	unsigned char binding_marker[32];
	unsigned char supported_data_types[16];
}drm_bin_header_t;

#endif /* __DRM_METADATA_TL_H__ */

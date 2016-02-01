/***************************************************************************
 *    (c)2006-2013 Broadcom Corporation
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
 ***************************************************************************/

#ifndef __DRM_METADATA_H__
#define __DRM_METADATA_H__


/* ****************************  THIS IS REFERENCE DRM_METADATA ***************/



/*  *********************************************************************************************/
/*  Flash Base offset address is platform dependent. But it should be at least 4k aligned,  */
/*  application should use the the Flash Base Address from the platform + the offset define here */
/*  to calculate the actual offset              */
/*  *********************************************************************************************/


/*
//  define common size in bytes
*/
#define DRM_SIZE_IN_BYTES_256B      (0x100) 
#define DRM_SIZE_IN_BYTES_512B      (0x200)    
#define DRM_SIZE_IN_BYTES_1K        (0x400)    
#define DRM_SIZE_IN_BYTES_2K        (0x800)    
#define DRM_SIZE_IN_BYTES_3K        (0xC00)    
#define DRM_SIZE_IN_BYTES_4K        (0x1000)   
#define DRM_SIZE_IN_BYTES_5K        (0x1400)   
#define DRM_SIZE_IN_BYTES_8K        (0x2000)   
#define DRM_SIZE_IN_BYTES_9K        (0x2400)   
#define DRM_SIZE_IN_BYTES_10K       (0x2800) 
#define DRM_SIZE_IN_BYTES_12K		(0x3000)
#define DRM_SIZE_IN_BYTES_14K       (0x3800)   
#define DRM_SIZE_IN_BYTES_15K       (0x3C00)   
#define DRM_SIZE_IN_BYTES_16K       (0x4000)
#define DRM_SIZE_IN_BYTES_19K       (0x4C00)
#define DRM_SIZE_IN_BYTES_20K       (0x5000)
#define DRM_SIZE_IN_BYTES_32K       (0x8000)   
#define DRM_SIZE_IN_BYTES_40K       (0xA000)   
#define DRM_SIZE_IN_BYTES_64K       (0x10000)  
#define DRM_SIZE_IN_BYTES_128K      (0x20000)  
#define DRM_SIZE_IN_BYTES_256K      (0x40000)  
#define DRM_SIZE_IN_BYTES_512K      (0x80000)  




/*
//  Each DRM regions size in bytes
*/
#define DRM_WMDRM_PD_SIZE_IN_BYTES     (DRM_SIZE_IN_BYTES_1K)
#define DRM_NETFLIX_SIZE_IN_BYTES      (DRM_SIZE_IN_BYTES_1K)
#define DRM_SFU_SIZE_IN_BYTES          (DRM_SIZE_IN_BYTES_1K)
#define DRM_WIDEVINE_SIZE_IN_BYTES     (DRM_SIZE_IN_BYTES_1K)
#define DRM_PLAY_READY_SIZE_IN_BYTES    (DRM_SIZE_IN_BYTES_1K)
#define DRM_DTCP_IP_SIZE_IN_BYTES        (DRM_SIZE_IN_BYTES_5K)
#define DRM_UBD_SIZE_IN_BYTES          (DRM_SIZE_IN_BYTES_1K)
#define DRM_OCAP_SIZE_IN_BYTES		   (DRM_SIZE_IN_BYTES_10K)
#define DRM_SECURE_SW_RSA_SIZE_IN_BYTES (DRM_SIZE_IN_BYTES_12K)
#define DRM_RESERVED_SIZE_IN_BYTES      (DRM_SIZE_IN_BYTES_6K)

/* Custom Private specific */
#define DRM_CUSTOM_PRIVATE_SIZE_IN_BYTES		   (DRM_SIZE_IN_BYTES_25K)



/***************************************************************************************/
/*                      LEGACY OFFSET DEFINITIONS                                      */
/*                   For DRM Utility 2.0 definitions                                   */
/***************************************************************************************/

/*  WMDRM-PD */
#define DRM_WMDRM_PD_OFFSET            (0x00)

/*  Netflix */
#define DRM_NETFLIX_OFFSET             (DRM_WMDRM_PD_OFFSET + DRM_WMDRM_PD_SIZE_IN_BYTES)

/* Secure Firmware Update */
#define DRM_SFU_OFFSET               (DRM_NETFLIX_OFFSET + DRM_NETFLIX_SIZE_IN_BYTES)

/*  Widevine */
#define DRM_WIDEVINE_OFFSET           (DRM_SFU_OFFSET + DRM_SFU_SIZE_IN_BYTES)

/* PLAY READY */
#define DRM_PLAY_READY_OFFSET         (DRM_WIDEVINE_OFFSET + DRM_WIDEVINE_SIZE_IN_BYTES)

/* DTCP-IP */
#define DRM_DTCP_IP_OFFSET		     (DRM_PLAY_READY_OFFSET + DRM_PLAY_READY_SIZE_IN_BYTES)

/* UBD */
#define DRM_UBD_OFFSET			     (DRM_DTCP_IP_OFFSET + DRM_DTCP_IP_SIZE_IN_BYTES)

/* OCAP */
#define DRM_OCAP_OFFSET              (DRM_UBD_OFFSET + DRM_UBD_SIZE_IN_BYTES)

/* Secure Software RSA */
#define DRM_SECURE_SW_RSA_OFFSET     (DRM_OCAP_OFFSET + DRM_OCAP_SIZE_IN_BYTES)

/*
// RESERVED
*/
#define DRM_RESERVED_OFFSET	         (DRM_SECURE_SW_RSA_OFFSET + DRM_SECURE_SW_RSA_SIZE_IN_BYTES)

/* Footer */
#define DRM_SIGNATURE_OFFSET	     (DRM_RESERVED_OFFSET + DRM_RESERVED_SIZE_IN_BYTES)


/**************** END OF OFFSET DEFINITIONS  *******************************************/


/*
//define for mainline DRM bin file size
*/
#define DRM_BIN_FILE_SIZE                   (DRM_SIZE_IN_BYTES_64K)
#define DRM_BIN_FILE_HEADER_SIZE            (256)


/*
//   BDVD DRM Type
*/
typedef enum drm_types_e {
    
     DRM_UNKNOWN  = 0x0,
     DRM_WMDRM_PD = 0x0101,
     DRM_NETFLIX,
     DRM_SFU,
     DRM_WIDEVINE,
     DRM_CI_PLUS,
     DRM_UBD,
     DRM_CI_PLUS_STD,
     DRM_DTCP_IP,
     DRM_PLAYREADY,
     DRM_RHAPSODY_CREDENTIALS,
     DRM_AVOD,
     DRM_OCAP,
     DRM_SECURE_SW_RSA,
     DRM_CUSTOM_PRIVATE,
     DRM_HDCP_RX,
     DRM_HDCP_TX,
     DRM_WIDEVINE_CENC,
} drm_types_e;


/*
//   DRM Data Type
*/
typedef enum drm_data_types_e 
{
    DRM_KEY_DATA_UNKNOWN        = 0x0,
    DRM_DATA_WMDRM_PD = 0x201,
    DRM_DATA_NETFLIX,
    DRM_DATA_SFU,
    DRM_DATA_WIDEVINE,
    DRM_DATA_CI_PLUS,
    DRM_DATA_UBD,
    DRM_DATA_CI_PLUS_STD,
    DRM_DATA_DTCP_IP,
    BDVD_DRM_DATA_PLAYREADY,      /* Playready data, encrypted */
    BDVD_DRM_DATA_WMDRM_SRM,      /* WMDRM SRM data, encrypted */
    BDVD_DRM_DATA_PLAYREADY_BIN,   /* Playready bin data, encrypted */
    DRM_DATA_RHAPSODY_CREDENTIALS,
    DRM_DATA_AVOD,
    DRM_DATA_OCAP,
    DRM_DATA_SECURE_SW_RSA,
    DRM_DATA_CUSTOM_PRIVATE,
    DRM_DATA_HDCP_RX,
    DRM_DATA_HDCP_TX,
    DRM_DATA_WIDEVINE_CENC,
} drm_data_types_e;




/* Size of Broadcom's region specific internal key use to protect data in drm regions*/
#define DRM_REGION_PRIV_KEY_SIZE                  (16)


/*
//   Each DRM region should be 256 bytes align, start address always end with 0x00
*/
#define DRM_REGION_ALIGNMENT                     (0x100)
#define DRM_FIRST_REGION_OFFSET                  (0x100)
#define DRM_FIRST_KEY_REGION_OFFSET              (0x100)
#define DRM_TO_SERIAL_NUM_OFFSET               (0x0068)


/*
//   Each Flash region will only load and support up to 8 DRMs
//   most of the cases, each flash region will only have one DRM
*/
#define MAX_DRM_FLASH_REGIONS                    (8)

/*
//   Each DRM support up to 6 different key size
*/
#define MAX_DRM_KEY_REGIONS                      (6)
#define MAX_DRM_DATA_REGIONS                     (4)
#define DRM_SERIAL_NUMBER_LENGTH                 (16)



#define DRM_KEY_SIZE_IN_20_BYTES                     (20)
#define DRM_KEY_SIZE_IN_40_BYTES                     (40)
#define DRM_KEY_SIZE_IN_259_BYTES                    (259)


/*
//  RSA signautre
*/
#define DRM_RSA_KEY_SIZE_IN_BYTES                    (259)
#define DRM_RSA_SIGNATURE_SIZE_IN_BYTES              (256)



/*  WMDRM definitions  */
#define DRM_WMDRM_PD_ENC_KEY_SIZE                (32)     /* Encrypted WMDRM-PD keys with AES. Since the WMDRM-keys are 20 bytes longs, but AES works on 16 bytes chuncks, we must add padding at the end*/
#define DRM_WMDRM_PD_KEY_SIZE                    (20)     /* "Unencrypted with AES" WMDRM-PD key size in bytes.*/


/*
//  WMDRM-PD Data
//  All the data are encrypted except DRMDataType
//
//    Example: for  WMDRM-PD Data
//
//       DRM_DataType        = DRM_DATA_WMDRM_PD;
*/
typedef struct drm_wmdrm_pd_data_s {

    drm_data_types_e   DRM_DataType;                                    /* specify this region data type */
    unsigned int            StructureSizeInBytes;                            /* sizeof this structure */
    unsigned char           group_priv_key[DRM_WMDRM_PD_ENC_KEY_SIZE];          /* Group Certificate private key.*/
    unsigned char           fallback_priv_key[DRM_WMDRM_PD_ENC_KEY_SIZE];       /* Fallback private key. */
    unsigned char           key[DRM_REGION_PRIV_KEY_SIZE];               /* key used to encrypt the fallback key and the group key. */

} drm_wmdrm_pd_data_t;

/* Map bdvd_drm_metadata.h types to drm_metadata.h types here to avoid plaging Playready's code with
a ton of if defs */
#define BDVD_SIZE_IN_BYTES_8K  DRM_SIZE_IN_BYTES_8K
#define BDVD_DRM_SERIAL_NUMBER_LENGTH DRM_SERIAL_NUMBER_LENGTH
#define BDVD_REGION_PRIV_KEY_SIZE DRM_REGION_PRIV_KEY_SIZE
#define bdvd_wmdrm_pd_data_t drm_wmdrm_pd_data_t
#define BDVD_REGION_PRIV_KEY_SIZE DRM_REGION_PRIV_KEY_SIZE
#define bdvd_drm_data_types_e drm_data_types_e
#define BDVD_DRM_DATA_WMDRM_PD DRM_DATA_WMDRM_PD

#define BDVD_WMDRM_PD_ENC_KEY_SIZE                DRM_WMDRM_PD_ENC_KEY_SIZE
#define BDVD_WMDRM_PD_KEY_SIZE                    DRM_WMDRM_PD_KEY_SIZE


/*
//  WMDRM_SRM Data
//  All the data is NOT encrypted 
//
//    Example: for  WMDRM SRM Data
//
//       DRM_DataType        = BDVD_DRM_DATA_WMDRM_SRM;
//
*/
typedef struct bdvd_wmdrm_srm_header {

    bdvd_drm_data_types_e   DRM_DataType;                                       /* specify this region data type */
    unsigned int            size;												/* size of WMDRM SRM */
    unsigned int            version;											/* version of WMDRM SRM */
    unsigned int            reserved;											/* reserved, must be set to zero */
} bdvd_wmdrm_srm_header_t;

/*
//
//  WMDRM-PD Data
//  All the data are encrypted 
//
//
*/
typedef struct bdvd_wmdrm_srm_key_data_s {

    unsigned char           SerialNumber[BDVD_DRM_SERIAL_NUMBER_LENGTH];        /* 16 bytes serial number */
    bdvd_wmdrm_pd_data_t    wmdrm_data;
	unsigned char			padding[8]; /* for encryption */
} bdvd_wmdrm_srm_key_data_t;


#define BDVD_PLAYREADY_MOD_KEY_SIZE               (32)     /* ECC 256 key.*/

#define BDVD_PLAYREADY_BIN_ENC_SIGN_SIZE_IN_BYTES	(32)
#define BDVD_PLAYREADY_BIN_HEADER_SIZE_IN_BYTES		(16)

#define PLAYREADY_AES_KEY_SIZE                    (16)     /* Size of the AES key in bytes*/
#define PLAYREADY_AES_GUID_SIZE                   (16)     /* Size of the AES Guid in bytes*/
#define PLAYREADY_VERSION_2_0 (2)        /* Playready SKD 2.0. With this version AesKey and AesGuid are present in drm_playready_data_t */

/*
//
//  Playready Data
//  All the data are encrypted except DRMDataType
//
//    Example: for  Playready Data
//
//       DRM_DataType        = BDVD_DRM_DATA_PLAYREADY
//
*/
typedef struct bdvd_playready_data_s {

    drm_data_types_e   DRM_DataType;                                         /* specify this region data type */
    unsigned int       StructureSizeInBytes;                                 /* sizeof this structure */
    unsigned char      model_cert_priv_key[BDVD_PLAYREADY_MOD_KEY_SIZE];     /* The PlayReady model certificate private key. */
    unsigned char      ndr_model_cert_priv_key[BDVD_PLAYREADY_MOD_KEY_SIZE]; /* The PlayReady NDR model certificate private key. */
    unsigned char      key[DRM_REGION_PRIV_KEY_SIZE];                        /* key used to encrypt the both model keys.*/
    unsigned char      aesKey[PLAYREADY_AES_KEY_SIZE];                       /* AES key, i.e. key used to protected the key file.a*/
    unsigned char      aesGuid[PLAYREADY_AES_KEY_SIZE];                      /* GUID identifier associated with the aesKey */

} bdvd_playready_data_t;

/*
//
//  PLAYREADY_BIN Header
//  All the data is NOT encrypted 
//
//    Example: for  PLAYREADY BIN Data
//
//       DRM_DataType        = BDVD_DRM_DATA_PLAYREADY_BIN;
//
*/
typedef struct bdvd_playready_bin_header {

	drm_data_types_e   DRM_DataType;                                   /* specify this region data type */
	unsigned int            size;											/* size of PLAYREADY BIN */
	unsigned int			version;										/* version of PLAYREADY BIN */
	unsigned char			cust_key;										/* Customer Key */
	unsigned char			var_hi;											/* Key Var High */
	unsigned char			var_lo;										/* Customer Key */
	unsigned char			reserved;										/* Reserved (0x00) */
} bdvd_playready_bin_header_t;

/*
//
//  PLAYREADY BIN Data
//  All the data are encrypted 
//
//
*/
typedef struct bdvd_playready_bin_key_data_s {

    unsigned char           SerialNumber[BDVD_DRM_SERIAL_NUMBER_LENGTH];        /* 16 bytes serial number */
	unsigned int			DeviceCertificateTemplateSize;
	bdvd_playready_data_t	playready_data;
} bdvd_playready_bin_key_data_t;

/*
//  Netflix Data
//  All the data is encrypted except DRMDataType
//
//    Example: for  Netflix Data
//
//       DRM_DataType        = DRM_DRM_DATA_NETFLIX;
*/

#define DRM_NETFLIX_KPE_SIZE                 (24)     /* Size of Netflix pre-shared encryption key in bytes. Kpe, base64 encoded is 24 ASCII characters long.*/
#define DRM_NETFLIX_ENC_KPE_SIZE             (32)     /* Size of encrypted kpe in flash expressed in bytes*/
#define DRM_NETFLIX_KPH_SIZE                 (44)     /* Size of Netflix pre-shared hmac key in bytes. Kph, base64 encoded is 44 ASCII characters long.*/
#define DRM_NETFLIX_ENC_KPH_SIZE             (64)     /* Size of encrypted kph in flash expressed in bytes*/
#define DRM_NETFLIX_ENC_ESN_SIZE             (64)     /* Size of the encryptedESN in bytes. The ESN is an ascii string.*/
#define DRM_NETFLIX_SWF_SIZE                 (272)  /* Size of the SWF key size in bytes.*/
#define DRM_NETFLIX_ROOT_CERT_SIZE           (18536)  /* Size of the root certificate size in bytes.*/
#define DRM_NETFLIX_ENC_ROOT_CERT_SIZE       (18544)  /* Size of the root certificate size in bytes.*/

typedef struct drm_nf_data_t 
{
    drm_data_types_e   DRM_DataType;                                    /* specify this region data type*/
    unsigned int            StructureSizeInBytes;                            /* sizeof this structure*/
    unsigned int            esn_size;                                        /* Size of the ESN in bytes.*/
    unsigned char           kpe[DRM_NETFLIX_ENC_KPE_SIZE];                  /* Netflix pre-shared encryption key.*/
    unsigned char           kph[DRM_NETFLIX_ENC_KPH_SIZE];                  /* Netflix pre-shared HMAC key.*/
    unsigned char           esn[DRM_NETFLIX_ENC_ESN_SIZE];                  /* Netflix ESN.*/
    unsigned char           key[DRM_REGION_PRIV_KEY_SIZE];                  /* key used to kpe and kph.*/
    unsigned char           swf[DRM_NETFLIX_SWF_SIZE];                  
} drm_nf_data_t;


#define DRM_NETFLIX_PMC_CON_KEY_SIZE       (128)     /* Size of encrypted kpe in flash expressed in bytes */
#define DRM_NETFLIX_PMC_CON_SECRET_SIZE    (128)     /* Size of Netflix pre-shared hmac key in bytes. Kph, base64 encoded is 44 ASCII characters long.*/
#define DRM_NETFLIX_PMC_ESN_PFX_SIZE       (32)     /* Size of encrypted kph in flash expressed in bytes*/

typedef struct drm_netflix_pmc_data_t
{
    unsigned char consumer_key[DRM_NETFLIX_PMC_CON_KEY_SIZE];
    unsigned char consumer_secret[DRM_NETFLIX_PMC_CON_SECRET_SIZE];
    unsigned char esn_prefix[DRM_NETFLIX_PMC_ESN_PFX_SIZE];
}drm_netflix_pmc_data_t;


/*
// Pandora definitions
*/
#define DRM_PANDORA_ENC_USERNAME_SIZE   (128)
#define DRM_PANDORA_ENC_PASSWD_SIZE     (128)
#define DRM_PANDORA_ENC_VERSION_SIZE    (64)
#define DRM_PANDORA_ENC_DEV_MODEL_SIZE  (64)
#define DRM_PANDORA_ENC_SYNC_KEY_SIZE   (64) /*Padded with 8 bytes, unencrypted 56-bytes*/
#define DRM_PANDORA_ENC_REQ_KEY_SIZE    (64) /*Padded with 8 bytes, unencrypted 56-bytes*/
#define DRM_PANDORA_ENC_HTTP_SIZE       (192)
#define DRM_PANDORA_ENC_HTTPS_SIZE      (192)

/*
//  Pandora Credential Data
*/
typedef struct drm_pandora_pmc_data_t
{
    unsigned char           username    [DRM_PANDORA_ENC_USERNAME_SIZE];     
    unsigned char           password    [DRM_PANDORA_ENC_PASSWD_SIZE];       
    unsigned char           device_model[DRM_PANDORA_ENC_DEV_MODEL_SIZE];    
    unsigned char           sync_bf_key [DRM_PANDORA_ENC_SYNC_KEY_SIZE];     
    unsigned char           req_bf_key  [DRM_PANDORA_ENC_REQ_KEY_SIZE];      
    unsigned char           http        [DRM_PANDORA_ENC_HTTP_SIZE];         
    unsigned char           https       [DRM_PANDORA_ENC_HTTPS_SIZE];        
} drm_pandora_pmc_data_t;


/*
// Napster definitions
*/
#define DRM_NAPSTER_ENC_API_KEY_SIZE    (128)
#define DRM_NAPSTER_STREAM_FORMAT_SIZE  (64)
#define DRM_NAPSTER_HTTP_SIZE           (64)
#define DRM_NAPSTER_HTTPS_SIZE          (64)

/*
//  Napster Credential Data
*/
typedef struct drm_napster_pmc_data_t
{
    unsigned char           api_key     [DRM_NAPSTER_ENC_API_KEY_SIZE]; 
    unsigned char           stream_format[DRM_NAPSTER_STREAM_FORMAT_SIZE];  
    unsigned char           http    [DRM_NAPSTER_HTTP_SIZE]; 
    unsigned char           https   [DRM_NAPSTER_HTTPS_SIZE];
} drm_napster_pmc_data_t;


/*
// Vudu definitions
*/
#define DRM_VUDU_ENC_MANU_ID_SIZE   (16)
#define DRM_VUDU_ENC_MODEL_ID_SIZE  (16)

/*
//  Vudu Credential Data
*/
typedef struct drm_vudu_pmc_data_t 
{
    unsigned char   manufacturer_id [DRM_VUDU_ENC_MANU_ID_SIZE];
    unsigned char   model_id    [DRM_VUDU_ENC_MODEL_ID_SIZE];   
}drm_vudu_pmc_data_t;


/*
// Generic PMC definitions
*/
#define DRM_GEN_ENC_CATEGORY_SIZE       (64)
#define DRM_GEN_ENC_MANU_ID_SIZE        (64)
#define DRM_GEN_ENC_MODEL_ID_SIZE       (64)
#define DRM_GEN_ENC_MODEL_GROUP_SIZE    (64)

/*
//  Generic Credential Data
*/
typedef struct drm_generic_pmc_data_t 
{
    unsigned char   category    [DRM_GEN_ENC_CATEGORY_SIZE];
    unsigned char   manufacturer_id [DRM_GEN_ENC_MANU_ID_SIZE];
    unsigned char   model_id    [DRM_GEN_ENC_MODEL_ID_SIZE];
    unsigned char   model_group [DRM_GEN_ENC_MODEL_GROUP_SIZE];
}drm_generic_pmc_data_t;

#define DRM_BBC_ENC_USERNAME_SIZE		(64)
#define DRM_BBC_ENC_PASSWORD_SIZE	(64)

/*
//  BBC Credential Data
*/
typedef struct drm_bbc_pmc_data_t 
{
	unsigned char	username	[DRM_BBC_ENC_USERNAME_SIZE];
	unsigned char	password	[DRM_BBC_ENC_PASSWORD_SIZE];
	
}drm_bbc_pmc_data_t;

/*******************************************
 * Rhapsody credential pointer and size
 ******************************************/
typedef struct drm_rhapsody_pmc_data_t
{
	unsigned char *	pCertificate;
	unsigned int 	uiCertificateSize;

	unsigned char * pKey;
	unsigned int 	uiKeySize;

}drm_rhapsody_pmc_data_t;

/*******************************************
 * AVOD credential pointer and size
 ******************************************/
typedef struct drm_avod_data_t
{
	unsigned char *	pCertificate;
	unsigned int 	uiCertificateSize;

	unsigned char * pKey;
	unsigned int 	uiKeySize;

}drm_avod_data_t;


/*******************************************
 * AMAZON credentials
 ******************************************/
#define DRM_AMAZON_ENC_CERT_SIZE	(20*DRM_SIZE_IN_BYTES_1K)

typedef struct drm_amazon_pmc_data_t
{
	unsigned int    cert_size;
	unsigned char   padding[12];
	unsigned char	cert	[DRM_AMAZON_ENC_CERT_SIZE];

}drm_amazon_pmc_data_t;


/*
// Adobe AIR definitions
*/
#define PMC_ADOBE_ROOTDIGEST_SIZE             (32)

#define PMC_ADOBE_TRANS_CERT_SIZE             (1201)
#define PMC_ADOBE_TRANS_CERT_ENC_SIZE         (1216)
#define PMC_ADOBE_TRANS_CERT_MAX_SIZE         (1536)

#define PMC_ADOBE_DEVICE_CREDENTIAL_SIZE      (6891)
#define PMC_ADOBE_DEVICE_CREDENTIAL_ENC_SIZE  (6896)
#define PMC_ADOBE_DEVICE_CREDENTIAL_MAX_SIZE  (8192)

#define PMC_ADOBE_DDC_PASSWORD_SIZE           (12)
#define PMC_ADOBE_DDC_PASSWORD_ENC_SIZE       (16)

#define PMC_ADOBE_SD_SIZE                     (6803)
#define PMC_ADOBE_SD_ENC_SIZE                 (6816)
#define PMC_ADOBE_SD_MAX_SIZE                 (8192)

#define PMC_ADOBE_SD_PASSWORD_SIZE            (12)
#define PMC_ADOBE_SD_PASSWORD_ENC_SIZE        (16)

typedef struct drm_adobe_pmc_data_t
{
	unsigned char	root_digest		[PMC_ADOBE_ROOTDIGEST_SIZE];
	unsigned char	transport_cert_size		[16];
	unsigned char	transport_cert	[PMC_ADOBE_TRANS_CERT_MAX_SIZE];
	unsigned char	device_cred_size		[16];
	unsigned char	device_cred		[PMC_ADOBE_DEVICE_CREDENTIAL_MAX_SIZE];
	unsigned char	device_cred_pwd_size	[16];
	unsigned char	device_cred_pwd	[PMC_ADOBE_DDC_PASSWORD_ENC_SIZE];
	unsigned char	shared_dom_0_size		[16];
	unsigned char	shared_dom_0	[PMC_ADOBE_SD_MAX_SIZE];
	unsigned char	shared_dom_0_pwd_size   [16];
	unsigned char	shared_dom_0_pwd[PMC_ADOBE_SD_PASSWORD_ENC_SIZE];
	unsigned char	shared_dom_1_size		[16];
	unsigned char	shared_dom_1	[PMC_ADOBE_SD_MAX_SIZE];
	unsigned char	shared_dom_1_pwd_size   [16];
	unsigned char	shared_dom_1_pwd[PMC_ADOBE_SD_PASSWORD_ENC_SIZE];
	unsigned char	shared_dom_2_size		[16];
	unsigned char	shared_dom_2	[PMC_ADOBE_SD_MAX_SIZE];
	unsigned char	shared_dom_2_pwd_size   [16];
	unsigned char	shared_dom_2_pwd[PMC_ADOBE_SD_PASSWORD_ENC_SIZE];
}drm_adobe_pmc_data_t;


/*******************************************/
/* WIDEVINE definitions                    */
/*******************************************/
#define DRM_WIDEVINE_KEY_SIZE                (16)     /* Size of the Widevine root key in bytes*/
#define DRM_WIDEVINE_KEYID_SIZE              (72)     /* Size of unencrypted KEY ID in bytes*/
#define DRM_WIDEVINE_ENC_KEYID_SIZE          (80)     /* Size of encrypted KEY ID in bytes*/
#define DRM_WIDEVINE_DEVICE_ID_SIZE          (32)     /* Size of the device ID*/

/*
//  Widevine Data
//  All the data is encrypted except DRMDataType
//
//    Example: for  Widevine Data
//
//       DRM_DataType        = DRM_DATA_WIDEVINE;
*/
typedef struct drm_wv_data_t {

    drm_data_types_e   DRM_DataType;                            
    unsigned int            StructureSizeInBytes;                    
    unsigned char           wvkey[DRM_WIDEVINE_KEY_SIZE];           /* Widevine Boxkey .*/
    unsigned char           wvkeyID[DRM_WIDEVINE_ENC_KEYID_SIZE];   /* Widevine Boxkey ID.*/
    unsigned char           wvDeviceID[DRM_WIDEVINE_DEVICE_ID_SIZE];/*Widevine Box Device ID */
    unsigned char           key[DRM_REGION_PRIV_KEY_SIZE];          /* key used to kpe and kph.*/

} drm_wv_data_t;


/*
//  Widevine CENC Data
//  All data pointers are encrypted except DRMDataType
//
//    Example: for  Widevine CENC Data
//
//       DRM_DataType        = DRM_DATA_WIDEVINE_CENC;
*/
#define DRM_WIDEVINE_MAGIC_SIZE        (4)
#define DRM_WIDEVINE_CRC_SIZE          (4)
typedef struct drm_wvcenc_data_t
{
    drm_data_types_e   DRM_DataType;
    unsigned int       StructureSizeInBytes;

    unsigned char      DeviceId[DRM_WIDEVINE_DEVICE_ID_SIZE]; /* all 32 bytes are used for encryption but string
                                                                 is null terminated and can be less */
    unsigned char      Key[DRM_WIDEVINE_KEY_SIZE];         /* 'Key' field */
    unsigned char      Id[DRM_WIDEVINE_KEYID_SIZE];    /* 'ID' field */
    unsigned char      Magic[DRM_WIDEVINE_MAGIC_SIZE]; /* Magic field */
    unsigned char      Crc[DRM_WIDEVINE_CRC_SIZE];   /* CRC */

    unsigned char      rpk[DRM_REGION_PRIV_KEY_SIZE];
    unsigned char      KeySerialNumber[DRM_REGION_PRIV_KEY_SIZE];
} drm_wvcenc_data_t;




#define DRM_SFU_KPUB_SIZE           (259)    /* Size of the RSA public key for secure firmware update in bytes.*/
#define DRM_SFU_ENC_KPUB_SIZE        (272)    /* Size of the encrypted RSA public key in flash for secure firmware update in bytes.*/
#define DRM_SFU_KU_SIZE             (16)     /* Size of the AES key for secure firmware update in bytes.*/

/*
//  Secure Firmware Update Data
//  All the data is encrypted except DRMDataType
//
//    Example: for  Secure Frimware Update Data
//
//       DRM_DataType        = DRM_DRM_DATA_SFU;
*/
typedef struct drm_sfu_data_t {

    drm_data_types_e   DRM_DataType;                
    unsigned int            StructureSizeInBytes;        
    unsigned char           ku_serial_number[DRM_SERIAL_NUMBER_LENGTH];
    unsigned char           kpub[DRM_SFU_ENC_KPUB_SIZE];
    unsigned char           ku[DRM_SFU_KU_SIZE];           /* Secure Firmware Update AES key.*/
    unsigned char           key[DRM_REGION_PRIV_KEY_SIZE]; /* RPK */
} drm_sfu_data_t;


/*
//  Update By Disk (UBD) Data
//  All the data is encrypted except DRMDataType
//
//       DRM_DataType        = DRM_DATA_UBD;
*/
#define DRM_UBD_ENCRYPTED_KEY_SIZE_IN_BYTES		(16)

typedef struct drm_ubd_data_t
{
	drm_data_types_e   DRM_DataType;
    unsigned int            StructureSizeInBytes;
    unsigned char           ubd_key[DRM_UBD_ENCRYPTED_KEY_SIZE_IN_BYTES];  /* UBD key.*/
    unsigned char           key[DRM_REGION_PRIV_KEY_SIZE]; /* RPK */
} drm_ubd_data_t;


/*
//  ULPK Data
//
//       DRM_DataType        = DRM_DRM_DATA_ULPK;
*/
#define ENCRYPTED_ULPK_SIZE     (128)
#define ULPK_CORE_SIZE          (80)

typedef struct bdvd_ulpk_data_t
{
    drm_data_types_e   DRM_DataType; 
    unsigned int            StructureSizeInBytes; 
    unsigned char           EncryptedUlpk[ENCRYPTED_ULPK_SIZE]; 
}bdvd_ulpk_data_t;


#define CI_PLUS_PADDED_ENC_CA_SIZE      (DRM_SIZE_IN_BYTES_2K) 
#define CI_PLUS_PADDED_ENC_ROOT_SIZE    (DRM_SIZE_IN_BYTES_2K) 
#define CI_PLUS_PADDED_ENC_CERT_SIZE    (DRM_SIZE_IN_BYTES_2K) 
#define CI_PLUS_PADDED_ENC_KEYS_SIZE    (DRM_SIZE_IN_BYTES_2K) 
/*
//  CI+ Data
//  All the data are encrypted except DRMDataType
//
//       DRM_DataType        = DRM_DRM_DATA_CI_PLUS;
*/
typedef struct drm_ci_plus_data {

    drm_data_types_e   DRM_DataType;
    unsigned char           padding[12];
    unsigned char           serial_number[DRM_SERIAL_NUMBER_LENGTH];   /* Serial number used for RPK encryption*/
    unsigned char           rpk[DRM_REGION_PRIV_KEY_SIZE]; 
    unsigned char           ca_size[16];
    unsigned char           ca[CI_PLUS_PADDED_ENC_CA_SIZE]; /*16-byte aligned*/
    unsigned char           root_size[16];
    unsigned char           root[CI_PLUS_PADDED_ENC_ROOT_SIZE]; /*16-byte aligned*/
    unsigned char           cert_filename[16];
    unsigned char           cert_size[16];
    unsigned char           cert[CI_PLUS_PADDED_ENC_CERT_SIZE]; /*16-byte aligned*/
    unsigned char           key_filename[16];
    unsigned char           key_size[16];
    unsigned char           keys[CI_PLUS_PADDED_ENC_KEYS_SIZE]; /*16-byte aligned*/

} drm_ci_plus_data_t;


/*
//  CI+ Standard Data
//  All the data are encrypted except DRMDataType
//
//       DRM_DataType        = DRM_DATA_CI_PLUS_STD;
*/

#define DRM_CI_PLUS_STD_KEY_SIZE	(16)

typedef struct drm_ci_plus_std_data {

	drm_data_types_e   DRM_DataType;
	unsigned char			padding[12];
	unsigned char           ci_plus_std_key[DRM_CI_PLUS_STD_KEY_SIZE];
	unsigned char			key[DRM_REGION_PRIV_KEY_SIZE];

} drm_ci_plus_std_data_t;


/*
//  Custom Private Data
//  All the data are encrypted except DRMDataType
//
//       DRM_DataType        = DRM_DATA_CUSTOM_PRIVATE;
*/

#define CUSTOM_PRIVATE_PADDED_ENCRYPTED_FIELD_SIZE	(2048)



typedef struct drm_custom_private_data
{
	bdvd_drm_data_types_e   DRM_DataType;
	unsigned char			padding[12];
    unsigned char           serial_number[DRM_SERIAL_NUMBER_LENGTH];	/* Serial number used for RPK encryption*/
	unsigned char           rpk[DRM_REGION_PRIV_KEY_SIZE];

	/* Shared files */
	unsigned char       shared_dev_cert_1[CUSTOM_PRIVATE_PADDED_ENCRYPTED_FIELD_SIZE]; /*16-byte aligned*/
	unsigned char		shared_dev_cert_1_size[4];
	unsigned char       shared_dev_cert_2[CUSTOM_PRIVATE_PADDED_ENCRYPTED_FIELD_SIZE];
	unsigned char		shared_dev_cert_2_size[4];
	unsigned char       shared_dev_cert_3[CUSTOM_PRIVATE_PADDED_ENCRYPTED_FIELD_SIZE];
	unsigned char		shared_dev_cert_3_size[4];
	unsigned char       shared_dev_key_1[CUSTOM_PRIVATE_PADDED_ENCRYPTED_FIELD_SIZE];
	unsigned char		shared_dev_key_1_size[4];
	unsigned char       shared_dev_key_2[CUSTOM_PRIVATE_PADDED_ENCRYPTED_FIELD_SIZE];
	unsigned char		shared_dev_key_2_size[4];
	unsigned char       shared_dev_key_3[CUSTOM_PRIVATE_PADDED_ENCRYPTED_FIELD_SIZE];
	unsigned char		shared_dev_key_3_size[4];

	/* Unique files */
	unsigned char       unique_dev_cert_1[CUSTOM_PRIVATE_PADDED_ENCRYPTED_FIELD_SIZE]; /*16-byte aligned*/
	unsigned char		unique_dev_cert_1_size[4];
	unsigned char       unique_dev_cert_2[CUSTOM_PRIVATE_PADDED_ENCRYPTED_FIELD_SIZE];
	unsigned char		unique_dev_cert_2_size[4];
	unsigned char       unique_dev_cert_3[CUSTOM_PRIVATE_PADDED_ENCRYPTED_FIELD_SIZE];
	unsigned char		unique_dev_cert_3_size[4];
	unsigned char       unique_dev_key_1[CUSTOM_PRIVATE_PADDED_ENCRYPTED_FIELD_SIZE];
	unsigned char		unique_dev_key_1_size[4];
	unsigned char       unique_dev_key_2[CUSTOM_PRIVATE_PADDED_ENCRYPTED_FIELD_SIZE];
	unsigned char		unique_dev_key_2_size[4];
	unsigned char       unique_dev_key_3[CUSTOM_PRIVATE_PADDED_ENCRYPTED_FIELD_SIZE];
	unsigned char		unique_dev_key_3_size[4];

} drm_custom_private_data_t;


/*
//
// OCAP
//
*/
#define OCAP_PADDED_ENC_MANUFACTURE_CA_SIZE		(DRM_SIZE_IN_BYTES_2K) 
#define OCAP_PADDED_ENC_MANUFACTURE_ROOT_SIZE	(DRM_SIZE_IN_BYTES_2K) 
#define OCAP_PADDED_ENC_DEVICE_CERT_SIZE		(DRM_SIZE_IN_BYTES_2K) 
#define OCAP_PADDED_ENC_DEVICE_KEY_SIZE			(DRM_SIZE_IN_BYTES_2K) 
#define OCAP_PADDED_ENC_DH_BASE					(DRM_SIZE_IN_BYTES_256B)
#define OCAP_PADDED_ENC_DH_PRIME				(DRM_SIZE_IN_BYTES_256B)



/*
//  OCAP Data
//  All the data are encrypted except DRMDataType
//
//       DRM_DataType        = DRM_DATA_OCAP;
*/
typedef struct drm_ocap_data {

	drm_data_types_e    DRM_DataType;
	unsigned char		padding[12];
    unsigned char       serial_number[DRM_SERIAL_NUMBER_LENGTH];	/* Serial number used for RPK encryption*/
	unsigned char       rpk[DRM_REGION_PRIV_KEY_SIZE];
	unsigned char		ca_size[16];
	unsigned char       ca[OCAP_PADDED_ENC_MANUFACTURE_CA_SIZE]; /*16-byte aligned*/
	unsigned char		root_size[16];
	unsigned char       root[OCAP_PADDED_ENC_MANUFACTURE_ROOT_SIZE]; /*16-byte aligned*/
	unsigned char		cert_filename[64];
	unsigned char		cert_size[16];
	unsigned char       cert[OCAP_PADDED_ENC_DEVICE_CERT_SIZE]; /*16-byte aligned*/
	unsigned char		key_filename[64];
	unsigned char		key_size[16];
	unsigned char       key[OCAP_PADDED_ENC_DEVICE_KEY_SIZE]; /*16-byte aligned*/
	unsigned char		base_size[16];
	unsigned char       base[OCAP_PADDED_ENC_DH_BASE]; /*16-byte aligned*/
	unsigned char		prime_size[16];
	unsigned char       prime[OCAP_PADDED_ENC_DH_PRIME]; /*16-byte aligned*/

} drm_ocap_data_t;


/*
//
// Secure Software RSA
//
*/
#define SECURE_SW_RSA_PADDED_ENC_CERT_SIZE     (DRM_SIZE_IN_BYTES_4K)
#define SECURE_SW_RSA_PADDED_ENC_KEY_SIZE      (DRM_SIZE_IN_BYTES_1K + DRM_SIZE_IN_BYTES_256B) 


/*
//  Secure Software RSA Data
//  All the data are encrypted except DRMDataType
//
//       DRM_DataType        = DRM_DATA_SECURE_SW_RSA;
*/
typedef struct drm_secure_sw_rsa_data {

	bdvd_drm_data_types_e   DRM_DataType;
	unsigned char           padding[12];
	unsigned char           serial_number[DRM_SERIAL_NUMBER_LENGTH];	/* Serial number used for RPK encryption*/
	unsigned char           rpk[DRM_REGION_PRIV_KEY_SIZE]; 
	unsigned char           device_unique_id[64];
	unsigned char           con_cert_size[16];
	unsigned char           con_cert[SECURE_SW_RSA_PADDED_ENC_CERT_SIZE]; /*16-byte aligned*/
	unsigned char           int_cert_size[16];
	unsigned char           int_cert[SECURE_SW_RSA_PADDED_ENC_CERT_SIZE]; /*16-byte aligned*/
	unsigned char           tls_cert_size[16];
	unsigned char           tls_cert[SECURE_SW_RSA_PADDED_ENC_CERT_SIZE]; /*16-byte aligned*/
	unsigned char           ta_cert_size[16];
	unsigned char           ta_cert[SECURE_SW_RSA_PADDED_ENC_CERT_SIZE]; /*16-byte aligned*/
	unsigned char           td_cert_size[16];
	unsigned char           td_cert[SECURE_SW_RSA_PADDED_ENC_CERT_SIZE]; /*16-byte aligned*/
	unsigned char           trust_status_cert_size[16];
	unsigned char           trust_status_cert[SECURE_SW_RSA_PADDED_ENC_CERT_SIZE]; /*16-byte aligned*/
	unsigned char           con_key_size[16];
	unsigned char           con_key[SECURE_SW_RSA_PADDED_ENC_KEY_SIZE];   /*16-byte aligned*/
	unsigned char           int_key_size[16];
	unsigned char           int_key[SECURE_SW_RSA_PADDED_ENC_KEY_SIZE];   /*16-byte aligned*/
	unsigned char           tls_key_size[16];
	unsigned char           tls_key[SECURE_SW_RSA_PADDED_ENC_KEY_SIZE];   /*16-byte aligned*/
	unsigned char           digest[32];
} drm_secure_sw_rsa_data_t;

typedef struct drm_secure_sw_rsa_data_1K_t {

	bdvd_drm_data_types_e   DRM_DataType;
	unsigned char           padding[12];
	unsigned char           serial_number[DRM_SERIAL_NUMBER_LENGTH];	/* Serial number used for RPK encryption*/
	unsigned char           rpk[DRM_REGION_PRIV_KEY_SIZE];
	unsigned char           device_unique_id[64];
	unsigned char           con_cert_size[16];
	unsigned char           con_cert[DRM_SIZE_IN_BYTES_1K + DRM_SIZE_IN_BYTES_256B]; /*16-byte aligned*/
	unsigned char           int_cert_size[16];
	unsigned char           int_cert[DRM_SIZE_IN_BYTES_1K + DRM_SIZE_IN_BYTES_256B]; /*16-byte aligned*/
	unsigned char           tls_cert_size[16];
	unsigned char           tls_cert[DRM_SIZE_IN_BYTES_1K + DRM_SIZE_IN_BYTES_256B]; /*16-byte aligned*/
	unsigned char           ta_cert_size[16];
	unsigned char           ta_cert[DRM_SIZE_IN_BYTES_1K + DRM_SIZE_IN_BYTES_256B]; /*16-byte aligned*/
	unsigned char           td_cert_size[16];
	unsigned char           td_cert[DRM_SIZE_IN_BYTES_1K + DRM_SIZE_IN_BYTES_256B]; /*16-byte aligned*/
	unsigned char           trust_status_cert_size[16];
	unsigned char           trust_status_cert[DRM_SIZE_IN_BYTES_1K + DRM_SIZE_IN_BYTES_256B]; /*16-byte aligned*/
	unsigned char           con_key_size[16];
	unsigned char           con_key[SECURE_SW_RSA_PADDED_ENC_KEY_SIZE];   /*16-byte aligned*/
	unsigned char           int_key_size[16];
	unsigned char           int_key[SECURE_SW_RSA_PADDED_ENC_KEY_SIZE];   /*16-byte aligned*/
	unsigned char           tls_key_size[16];
	unsigned char           tls_key[SECURE_SW_RSA_PADDED_ENC_KEY_SIZE];   /*16-byte aligned*/
	unsigned char           digest[32];
} drm_secure_sw_rsa_data_1K_t;


/***********************************************************************
//	DTCP-IP ECC Params
************************************************************************/
#define DRM_DTCP_IP_ECC_PARAMS_SIZE_IN_BYTES			(20)

/*
//	DTCP-IP other information
*/
/* Total File Size: Specific Order Format Data Size + SHA-1 Hash Size + File Header */
#define DRM_DTCP_IP_ORDER_FORMAT3_FILE_SIZE					(584)
#define DRM_DTCP_IP_PUBLIC_KEY_ECC_PARAMS_SIZE					(160)


/*
//	DTCP-IP Device Keys and Device Certificate
*/
#define DRM_DTCP_IP_DTLA_PUBLIC_KEY_SIZE_IN_BYTES					(40)
#define DRM_DTCP_IP_PRIVATE_KEY_SIZE_IN_BYTES						(20)
#define DRM_DTCP_IP_EXT_FULL_CERT_SIZE_IN_BYTES					(132)
#define DRM_DTCP_IP_BASELINE_FULL_CERT_SIZE_IN_BYTES				(88)
#define DRM_DTCP_IP_KSV_SIZE_IN_BYTES								(2) // the 4 MSB bits are reserved for alignment)
#define DRM_DTCP_IP_COPY_KEYS_SIZE_IN_BYTES			(96)
#define DRM_DTCP_IP_COPY_NO_MORE_RECEIVER_KEY_SIZE_IN_BYTES		(96)
#define DRM_DTCP_IP_COPY_ONCE_SOURCE_KEY_SIZE_IN_BYTES				(96)
#define DRM_DTCP_IP_COPY_NO_MORE_SOURCE_KEY_SIZE_IN_BYTES			(96)
#define DRM_DTCP_IP_SEED_RNG_SIZE_IN_BYTES							(20)
#define DRM_DTCP_IP_SEED_RNGF_SIZE_IN_BYTES						(20)
#define DRM_DTCP_IP_SEED_RNGR_SIZE_IN_BYTES						(20)
#define DRM_DTCP_IP_ORDER_FORMAT_SIZE_IN_BYTES						(4)
#define DRM_DTCP_IP_RESERVED_ALIGNMENT_SIZE_IN_BYTES				(2)
#define DRM_DTCP_IP_DEVICE_FULL_AUTH_PRIVATE_KEY_SIZE_IN_BITS		(160) // 1 * 160
#define DRM_DTCP_IP_RESTRICTED_AUTH_CERT							(48)

/*
//	DTCP-IP Encrypted Data
*/
#define DRM_DTCP_IP_ENC_PUBLIC_KEY_SIZE			(48)
#define DRM_DTCP_IP_ENC_PRIVATE_KEY_SIZE			(32)
#define DRM_DTCP_IP_ENC_EXT_FULL_AUTH_CERT_SIZE	(144)
#define DRM_DTCP_IP_ENC_BASE_FULL_AUTH_CERT_SIZE	(96)
#define DRM_DTCP_IP_ENC_REST_AUTH_CERT_SIZE		(48)
#define DRM_DTCP_IP_ENC_KSV_SIZE					(16) /* the 4 MSB bits are reserved for alignment */
#define DRM_DTCP_IP_ENC_COPY_KEY_SIZE				(96)
#define DRM_DTCP_IP_ENC_SEED_SIZE					(32)
#define DRM_DTCP_IP_ENC_ECC_DATA_SIZE					(32)


/*
//  DTCP-IP Data
//  All the data are encrypted except DRMDataType
//
//       DRM_DataType        = DRM_DATA_DTCP_IP;
*/
typedef struct drm_dtcp_ip_data_t {

	drm_data_types_e   DRM_DataType;
	unsigned int            StructureSizeInBytes;
	unsigned char           key[DRM_REGION_PRIV_KEY_SIZE];  /* DTCP-IP region private key */
	unsigned char           BaselineFullCert[DRM_DTCP_IP_ENC_BASE_FULL_AUTH_CERT_SIZE];		/* Baseline Full Authentication Ceritificate*/
	unsigned char           PrivateKey[DRM_DTCP_IP_ENC_PRIVATE_KEY_SIZE];	/* Device Full Authentication Private Key */
	unsigned char			Ksv[DRM_DTCP_IP_ENC_KSV_SIZE]; /* Key Selection Vector */
	unsigned char           RestrictedCopyOnceSinkKey[DRM_DTCP_IP_ENC_COPY_KEY_SIZE]; /* Restricted Authentication Copy Once Receiver Keys */
	unsigned char           RestrictedCopyNoMoreSinkKey[DRM_DTCP_IP_ENC_COPY_KEY_SIZE]; /* Restricted Authentication Copy No More Receiver Keys */
	unsigned char           RestrictedCopyOnceSourceKey[DRM_DTCP_IP_ENC_COPY_KEY_SIZE]; /* Restricted Authentication Copy Once Source Keys */
	unsigned char           RestrictedCopyNoMoreSourceKey[DRM_DTCP_IP_ENC_COPY_KEY_SIZE]; /* Restricted Authentication Copy No More Source Keys */
	unsigned char			RngSeed[DRM_DTCP_IP_ENC_SEED_SIZE];
	unsigned char			RestrictedCert[DRM_DTCP_IP_ENC_REST_AUTH_CERT_SIZE];	/* Restricted Authentication Certificate */
	unsigned char			DtlaPublicKey[DRM_DTCP_IP_ENC_PUBLIC_KEY_SIZE]; /* DTLA Public Key */
	unsigned char			PrimeGF[DRM_DTCP_IP_ENC_ECC_DATA_SIZE]; /* Prime of GF(p) */
	unsigned char           ACoefficientCurve[DRM_DTCP_IP_ENC_ECC_DATA_SIZE]; /* Coefficient of E Curve */
	unsigned char           BCoefficientCurve[DRM_DTCP_IP_ENC_ECC_DATA_SIZE]; /* Coefficient of E Curve */
	unsigned char           XBasePoint[DRM_DTCP_IP_ENC_ECC_DATA_SIZE]; /* ECC X Coordinate of Base Point */
	unsigned char			YBasePoint[DRM_DTCP_IP_ENC_ECC_DATA_SIZE]; /* ECC Y Coordinate of Base Point */
	unsigned char			OrderBasePoint[DRM_DTCP_IP_ENC_ECC_DATA_SIZE]; /* ECC Order of BAse Point */

} drm_dtcp_ip_data_t;


/*
//  HDCP Data
//
//       DRM_DataType        = DRM_DATA_HDCP_RX;
*/
typedef struct drm_hdcp_rx_data_t
{
	drm_data_types_e DRM_DataType;
	unsigned int     StructureSizeInBytes;
	unsigned char    rpk[DRM_REGION_PRIV_KEY_SIZE];  /* HDCP RX region private key */
	unsigned char    padding_0[8]; /* padding in order to make the structure 16 byte aligned */

	unsigned char    size_of_lc[16];
	unsigned char    licensed_constant[16];
	unsigned char    sha1_of_lc[20];
	unsigned char    padding_1[12]; /* padding in order to make the data 16 byte aligned */

	unsigned char    size_of_keyset[16];
	unsigned char    rx_id[5];
	unsigned char    device_pub_modulus[128];
	unsigned char    device_pub_exponent[3];
	unsigned char    protocol_descriptor[2];
	unsigned char    root_rsa_signature[384];
	unsigned char    p[64];
	unsigned char    q[64];
	unsigned char    d_mod_p1[64];
	unsigned char    d_mod_q1[64];
	unsigned char    q1_mod_p[64];
	unsigned char    keyset_sha1[20]; /* from rx_id to the end */

	unsigned char    padding_2[2]; /* padding in order to make the data 16 byte aligned */
} drm_hdcp_rx_data_t;


typedef struct drm_hdcp_tx_data_t
{
	drm_data_types_e DRM_DataType;
	unsigned int     StructureSizeInBytes;
	unsigned char    rpk[DRM_REGION_PRIV_KEY_SIZE];  /* HDCP TX region private key */
	unsigned char    padding_0[8]; /* padding in order to make the structure 16 byte aligned */

    unsigned char size_of_lc[16];
	unsigned char licensed_constant[16];
	unsigned char sha1_of_lc[20];
	unsigned char padding_1[12]; /* padding in order to make the data 16 byte aligned */

	unsigned char size_of_keyset[16];
	unsigned char root_public_modulus[384];
	unsigned char root_public_exponent[1];
	unsigned char sha1_of_ne[20];
	unsigned char padding_2[11]; /* padding in order to make the structure 16 byte aligned */
}drm_hdcp_tx_data_t;




/*
//  DRM Region
//  each KeyRegion and DataRegion should be at least 16 bytes aligned,
//  start address should end at 0x0
*/
typedef struct drm_region_t {

     drm_types_e       DRM_type;                                           /* WMDRM, NETFLIX, etc... */
     unsigned int           RegionSizeInBytes;                                  /* Region Size in Bytes;*/
     unsigned int           Region_Reserved[2];                                 /* Reserved, must set to zero*/
     unsigned int           TotalKeyRegions;                                    /* Total key regions;*/
     drm_data_types_e    Key_Type[MAX_DRM_KEY_REGIONS];                 
     unsigned int           KeyRegionsOffsetInBytes[MAX_DRM_KEY_REGIONS];  /* Max 6 key regions*/
     unsigned int           TotalDataRegions;                                   /* Total data regions*/
     drm_data_types_e    Data_Type[MAX_DRM_DATA_REGIONS];               
     unsigned int           DataRegionsOffsetInBytes[MAX_DRM_DATA_REGIONS];/* Max 4 data regions*/
     unsigned char          SerialNumber[DRM_SERIAL_NUMBER_LENGTH];        /* 16 bytes serial number*/
     unsigned int           Region_Reserved1[2];                                 /* Reserved, must set to zero*/

} drm_region_t;




#define BINDING_MARKER_OFFSET       240


typedef struct drm_header_t
{
    /*First 16-bytes*/
    unsigned char cookie[4];
    unsigned char reserved_1[12];
    
    /*2nd 16-bytes*/
    unsigned char cust_key_select;
    unsigned char key_var_low;
    unsigned char key_var_high;
    unsigned char drm_util_version[13];
    
    /*3rd, 4th and 5th 16-byte sets*/
    unsigned char proc_in1[16];
    unsigned char proc_in2[16];
    unsigned char proc_in3[16];

    /*6th 16-byte set*/
	unsigned char	manufacturer_id[2];
	unsigned char	manufacturer_name[13];
    unsigned char   production_flag;

    /*7th 16-byte set*/
    unsigned char box_unique_id[16];    
    
    /*8th, 9th and 10th 16-byte sets*/
    unsigned char bin_filename[32];
	unsigned char	reserved_2[12];
	unsigned char	bin_file_size[4];
        
    /*11th, 12th and 13th 16-byte sets*/
    unsigned char   mac_addr_0_15[16];
    unsigned char   mac_addr_16_31[16];
    unsigned char   mac_addr_32_47[16];
    
	/*14th 16-byte set*/
	unsigned char model_string[16];
    
    /*15th and 16th (reserved)*/
	unsigned char reserved_3[16];
    unsigned char binding_marker[16];
    
}drm_header_t;


#endif /*DRM_METADATA_H_*/

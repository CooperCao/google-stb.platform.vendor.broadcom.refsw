/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
* API Description:
*   API name: wmdrmpd
*    Specific APIs related to Microsoft Windows Media DRM PD
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef NEXUS_WMDRMPD_TYPES_H_
#define NEXUS_WMDRMPD_TYPES_H_

#include "nexus_security_types.h"
#include "nexus_security_datatypes.h"

#ifdef __cplusplus
extern "C" {
#endif


#define NEXUS_WMDRMPD_GUID_LENGTH (16)   /* Lenght of Windows Media DRM GUID in bytes */


/***************************************************************************
Summary: 
WMDRM specific error codes
***************************************************************************/
#define NEXUS_WMDRMPD_LICENSE_EXPIRED      NEXUS_MAKE_ERR_CODE(0x10B, 0)     /* This error is returned when no license valid license could be found to play the content.
                                                                                The application must get a new one from the license server */
#define NEXUS_WMDRMPD_REVOCATION_PACKAGE   NEXUS_MAKE_ERR_CODE(0x10B, 1)     /* This error is returned when the revocation package on the device needs to be updated.
                                                                                The application must get a new one from the license server */
#define NEXUS_WMDRMPD_INVALID_RESPONSE     NEXUS_MAKE_ERR_CODE(0x10B, 2)     /* This error is returned when processing of the license response failed */
#define NEXUS_WMDRMPD_CRYPTOGRAPHIC_ERROR  NEXUS_MAKE_ERR_CODE(0x10B, 3)     /* A cryptographic operation failed */
#define NEXUS_WMDRMPD_XML_NOT_FOUND        NEXUS_MAKE_ERR_CODE(0x10B, 4)     /* XML tag not found */
#define NEXUS_WMDRMPD_INVALID_HEADER       NEXUS_MAKE_ERR_CODE(0x10B, 5)     /* Invalid header */
#define NEXUS_WMDRMPD_CONTENT_HEADER_NOT_SET       NEXUS_MAKE_ERR_CODE(0x10B, 6)     /* Content Header Not Set */

/***************************************************************************
Summary: 
GUID (Globally Unique Identifier) for Windows Media DRM
***************************************************************************/
typedef struct NEXUS_WmDrmPdGuid
{
    uint8_t   data[NEXUS_WMDRMPD_GUID_LENGTH];
} NEXUS_WmDrmPdGuid;

/***************************************************************************
Summary: 
ASF PSI (Protection System Identifier) Object for Microsoft PlayReady
***************************************************************************/
typedef struct NEXUS_WmDrmPdPsiObjectInfo
{
    NEXUS_WmDrmPdGuid systemId;     /* Defines the GUID of the Content Protection system. Microsoft PlayReady systems must use ASF_Content_Protection_System_Microsoft_PlayReady */
    uint32_t sysVersion;            /* Version specific to each Content Protection system. Microsoft PlayReady must use System Version = 1*/
} NEXUS_WmDrmPdPsiObjectInfo;

/***************************************************************************
Summary: 
ASF Digisign (Digital Signature) Object for Microsoft Windows Media DRM 10
***************************************************************************/
typedef struct NEXUS_WmDrmPdDigsignObjectInfo
{
    uint32_t       type;        /* Digital signature type */
} NEXUS_WmDrmPdDigsignObjectInfo;


/***************************************************************************
Summary: 
MP4 PSSH (Protection System Specific Header) Box for Microsoft PlayReady
***************************************************************************/
typedef struct NEXUS_WmDrmPdMp4PsshBoxInfo
{
    NEXUS_WmDrmPdGuid systemId;     /* Defines the GUID of the Content Protection system. Microsoft PlayReady systems must use SystemID=9A04F079-9840-4286-AB92-E65BE0885F95. */
} NEXUS_WmDrmPdMp4PsshBoxInfo;


/***************************************************************************
Summary: 
Encryption types supported.
 
Description: 
***************************************************************************/
typedef enum NEXUS_WmDrmPdEncryptionType
{
    NEXUS_WmDrmPdEncryptionType_eNone,                /* no encryption */             
    NEXUS_WmDrmPdEncryptionType_eWmdrm,               /* wmdrm encrypted asf */       
    NEXUS_WmDrmPdEncryptionType_eAesCtr,              /* AES CTR encrypted stream */  
    NEXUS_WmDrmPdEncryptionType_eAesCbc,              /* AES CBC encrypted stream */  
    NEXUS_WmDrmPdEncryptionType_eMax
} NEXUS_WmDrmPdEncryptionType;


#define NEXUS_WMDRMPD_KEY_ID_LENGTH (16)   /* Key identifier length in bytes */

/***************************************************************************
Summary: 
MP4 Scheme Information Box for Microsoft PlayReady
***************************************************************************/
typedef struct NEXUS_WmDrmPdMp4SchemeInfo 
{
    NEXUS_WmDrmPdEncryptionType algorithm;  /* Default algorithm used to encrypt the track.*/
    unsigned  ivSize;      /* Default size in bytes of the Initialization Vector (IV) for this track. 
                               8 -  Specifies 64-bit initialization vectors. Supported for AES-CTR.
                               16  Specifies 128-bit initialization vectors. Supported for both AES-CTR and AES-CBC. */
    uint8_t   keyId[NEXUS_WMDRMPD_KEY_ID_LENGTH];   /* Default key identifier for this track. */
}NEXUS_WmDrmPdMp4SchemeInfo;

/***************************************************************************
Summary: 
MP4 Track Encryption Box for Microsoft PlayReady.
***************************************************************************/
typedef struct NEXUS_WmDrmPdMp4TrackEncryptionBox
{
    uint8_t version;
    uint32_t flags;
    NEXUS_WmDrmPdMp4SchemeInfo info;
} NEXUS_WmDrmPdMp4TrackEncryptionBox;

/***************************************************************************
Summary: 
MP4 Scheme Type Box for Microsoft PlayReady.
***************************************************************************/
typedef struct NEXUS_WmDrmPdMp4SchemeTypeBox
{
    uint8_t  version;
    uint32_t flags;
    uint32_t schemeType;
    uint32_t schemeVersion;
}NEXUS_WmDrmPdMp4SchemeTypeBox;

/***************************************************************************
Summary: 
MP4 Original Format Box for Microsoft PlayReady.
***************************************************************************/
typedef struct NEXUS_WmDrmPdMp4OriginalFormatBox 
{
    uint32_t codingName;
} NEXUS_WmDrmPdMp4OriginalFormatBox;

/***************************************************************************
Summary: 
MP4 Protection Scheme Box for Microsoft PlayReady. One per track. 
Describes the protection scheme to enforce on a given track
***************************************************************************/
typedef struct NEXUS_WmDrmPdMp4ProtectionSchemeInfo{
    NEXUS_WmDrmPdMp4SchemeTypeBox       schemeType;
    NEXUS_WmDrmPdMp4OriginalFormatBox   originalFormat;
    NEXUS_WmDrmPdMp4TrackEncryptionBox  trackEncryption;
    uint32_t                            trackId;
}NEXUS_WmDrmPdMp4ProtectionSchemeInfo;


/***************************************************************************
Summary: 
License Challenge Information
***************************************************************************/
typedef struct NEXUS_WmDrmPdLicenseChallenge
{
    unsigned     dataLength;        /* Length of data field in bytes */
    const char  *pData;             /* attr{memory=cached} Challenge Data */
    const char  *pUrl;              /* attr{memory=cached} URL the challenge data
                                       should be sent to.  Some streams may not have a URL
                                       tag in the header, in which case this pointer is NULL.
                                       In that case, it is assumed the application will know
                                       the appropriate server to send the challenge to. */
    bool         nonQuiet;          /* If true, non-quiet license acquisition is requested. */
    unsigned     appSecurityLevel;  /* Application Security Level */
    unsigned     maxResponseLength; /* Size of the response buffer in bytes */
    char        *pResponseBuffer;   /* attr{memory=cached} Buffer the response should be written to. */
} NEXUS_WmDrmPdLicenseChallenge;


/***************************************************************************
Summary:
Decrypt context for content decryption. 

Description:
See NEXUS_WmDrmPdCore_AllocateDecryptContext
***************************************************************************/
typedef struct NEXUS_WmDrmPdDecryptContext *NEXUS_WmDrmPdDecryptHandle;

/***************************************************************************
Summary: 
AES Counter Mode information.
***************************************************************************/
typedef struct NEXUS_WmdrmPdAesCounterInfo
{
    uint64_t nonce;
    uint64_t blockCounter;
    size_t   byteOffset;     /* Byte Offset within the current AES block. */
    NEXUS_WmDrmPdDecryptHandle decrypt; /* Set to 0, if the application doesn't use key rotation.
                                           Playready internal decryption handle will be used. */
}NEXUS_WmdrmPdAesCounterInfo;

/***************************************************************************
Summary: 
License Policy Types 
 
Description: 
Values in this enumeration will match the DRM_POLICY_CALLBACK_TYPE enum 
defined by the Microsoft SDK. 
***************************************************************************/
typedef enum NEXUS_WmDrmPdPolicyType
{
    NEXUS_WmDrmPdPolicyType_ePlay = 0x1,
    NEXUS_WmDrmPdPolicyType_eCopy = 0x2,
    NEXUS_WmDrmPdPolicyType_eInclusionList = 0x3,
    NEXUS_WmDrmPdPolicyType_eExtendedRestrictionCondition = 0x4,    /* Currently not supported */
    NEXUS_WmDrmPdPolicyType_eExtendedRestrictionAction = 0x5,       /* Currently not supported */
    NEXUS_WmDrmPdPolicyType_eExtendedRestrictionQuery = 0x6,        /* Currently not supported */
    NEXUS_WmDrmPdPolicyType_eSecureStateTokenResolve = 0x7          /* Currently not supported */
}NEXUS_WmDrmPdPolicyType;



/***************************************************************************
Summary: 
Identify which buffer to recover the required size for.

Microsoft Playready SDK 2.0 and older relies on the following memory allocation
scheme:
It is for the caller to allocate a buffer, call a function, and if the buffer 
is big enough to hold the returned data the function would return
DRM_E_BUFFERTOOSMALL along with the size required for the buffer. 
The caller would then reallocate the buffer to the given size and call the 
function again.

This structure is used to simplify the process within nexus and relies
on a single function that the application can query to retrives to buffer size
it needs to allocate.
 
Description: 
Values in this enumeration identify the type of buffer the application wants
to recover the size for. 
***************************************************************************/
typedef enum NEXUS_WmDrmPdBufferId
{
    NEXUS_WmDrmPdBufferId_eContentPropertyHeaderKid,
    NEXUS_WmDrmPdBufferId_eContentPropertyHeaderType,
    NEXUS_WmDrmPdBufferId_eContentPropertyHeader,
    NEXUS_WmDrmPdBufferId_eContentPropertyPlayreadyObject,
    NEXUS_WmDrmPdBufferId_eContentPropertyCipherType,
    NEXUS_WmDrmPdBufferId_eContentPropertyDecryptorSetup,
    NEXUS_WmDrmPdBufferId_eDevicePropertyDeviceCertificateMd,   /* Currently not implemented */
    NEXUS_WmDrmPdBufferId_eDevicePropertyDeviceCertificatPd,    /* Currently not implemented */
    NEXUS_WmDrmPdBufferId_eDevicePropertyClientInformation,     /* Currently not implemented */
    NEXUS_WmDrmPdBufferId_eDevicePropertyPlayreadyVersion,      /* Currently not implemented */
    NEXUS_WmDrmPdBufferId_eDevicePropertySecurityVersion,       /* Currently not implemented */
    NEXUS_WmDrmPdBufferId_eDevicePropertyWmdrmPdVersion,        /* Currently not implemented */
    NEXUS_WmDrmPdBufferId_eMax
}NEXUS_WmDrmPdBufferId;

/***************************************************************************
Summary: 
Indicates which property to set.
***************************************************************************/
typedef enum NEXUS_WmDrmPdContentSetProperty
{
    NEXUS_WmDrmPdContentSetProperty_eHeaderNotSet,
    NEXUS_WmDrmPdContentSetProperty_eV1Header,
    NEXUS_WmDrmPdContentSetProperty_eV2Header,
    NEXUS_WmDrmPdContentSetProperty_eKID,
    NEXUS_WmDrmPdContentSetProperty_eV2_4Header,
    NEXUS_WmDrmPdContentSetProperty_eV4Header,
    NEXUS_WmDrmPdContentSetProperty_eAutoDetectHeader,
    NEXUS_WmDrmPdContentSetProperty_ePlayreadyObject,
    NEXUS_WmDrmPdContentSetProperty_eV4_1Header,
    NEXUS_WmDrmPdContentSetProperty_ePlayreadyObjectWithKID,
    NEXUS_WmDrmPdContentSetProperty_eHeaderComponents,
    NEXUS_WmDrmPdContentSetProperty_eMax
}NEXUS_WmDrmPdContentSetProperty;

/***************************************************************************
Summary: 
Bounds for policy information
***************************************************************************/
#define NEXUS_WMDRMPD_MAX_INCLUSION_GUIDS     (20)  /*  Must match the value of DRM_MAX_INCLUSION_GUIDS in the playready SDK */
#define NEXUS_WMDRMPD_MAX_EXCLUSION_GUIDS     (20)
#define NEXUS_WMDRMPD_MAX_LICENSE_CHAIN_DEPTH (2)   /* Must match the value of DRM_MAX_LICENSE_CHAIN_DEPTH in the playready SDK */
#define NEXUS_WMDRMPD_MAX_VIDEO_OUTPUT_PROTECTION    (4)  /* Maximum number of video output protection entries in an XML license */
#define NEXUS_WMDRMPD_MAX_AUDIO_OUTPUT_PROTECTION    (4)  /* Maximum number of audio output protection entries in an XML license */


/***************************************************************************
Summary:
Audio/Video Output Protection Entry for Play Policies
***************************************************************************/
typedef struct NEXUS_WmDrmPdOutProtection
{
    uint32_t           version;
    NEXUS_WmDrmPdGuid  guid;
    uint32_t           configData;
} NEXUS_WmDrmPdOutProtection;

typedef NEXUS_WmDrmPdOutProtection NEXUS_WmDrmPdVideoOutProtection;


/***************************************************************************
Summary: 
Play Policy Definition
***************************************************************************/
typedef struct NEXUS_WmDrmPdPlayPolicy
{
    uint32_t compressedDigitalVideo;    /* Mimimum output protection level to play compressed digital video */
    uint32_t uncompressedDigitalVideo;  /* Mimimum output protection level to play uncompressed digital video */
    uint32_t analogVideo;               /* Mimimum output protection level to play analog video */
    uint32_t compressedDigitalAudio;    /* Mimimum output protection level to play compressed digital audio */
    uint32_t uncompressedDigitalAudio;  /* Mimimum output protection level to play uncompressed digital video */

    /* Video output protection ids fetched from the XML license. The number varies from license to license */
    unsigned                         numVideoOutputEntries;    /* Number of video output protections ids. */
    NEXUS_WmDrmPdOutProtection       videoOutputEntries[NEXUS_WMDRMPD_MAX_VIDEO_OUTPUT_PROTECTION];          /* Pointer to array of video output protections IDs. */

    /* Audio output protection ids fetched from the XML license. The number varies from license to license */
    unsigned                         numAudioOutputEntries;    /* Number of audio output protections ids. */
    NEXUS_WmDrmPdOutProtection       audioOutputEntries[NEXUS_WMDRMPD_MAX_AUDIO_OUTPUT_PROTECTION];          /* Pointer to array of audio output protections IDs. */

}NEXUS_WmDrmPdPlayPolicy;

/***************************************************************************
Summary: 
Copy Policy Definition
***************************************************************************/
typedef struct NEXUS_WmDrmPdCopyPolicy
{
    uint32_t           minimumCopyLevel;                        /* Minimum copy level */

    /* The number of inclusion ids is fetch from the XML license. This number varies from license to license */
    unsigned           numInclusionIds;                         /* Number of inclusions IDs*/ 
    NEXUS_WmDrmPdGuid  inclusionIds[NEXUS_WMDRMPD_MAX_INCLUSION_GUIDS]; /* Guid of inclusions on output protections levels */

    /* The number of eclsusion ids is fetch from the XML license. This number varies from license to license */
    unsigned           numExclusionIds;                         /* Number of restriction IDs*/ 
    NEXUS_WmDrmPdGuid  exclusionIds[NEXUS_WMDRMPD_MAX_EXCLUSION_GUIDS]; /* Guid of restrictons on output protections levels */
}NEXUS_WmDrmPdCopyPolicy;

/***************************************************************************
Summary: 
Inclusion List Definition
***************************************************************************/
typedef struct NEXUS_WmDrmPdInclusionList
{
    NEXUS_WmDrmPdGuid   inclusionList      [NEXUS_WMDRMPD_MAX_INCLUSION_GUIDS];
    bool                inclusionListValid [NEXUS_WMDRMPD_MAX_INCLUSION_GUIDS][NEXUS_WMDRMPD_MAX_LICENSE_CHAIN_DEPTH];
    unsigned            chainDepth;
}NEXUS_WmDrmPdInclusionList;

/***************************************************************************
Summary: 
Policy Status
***************************************************************************/
typedef struct NEXUS_WmDrmPdPolicyStatus
{
    NEXUS_WmDrmPdPolicyType policyType;
    union
    {
        NEXUS_WmDrmPdPlayPolicy    play;
        NEXUS_WmDrmPdCopyPolicy    copy;
        NEXUS_WmDrmPdInclusionList inclusionList;
    } policy;
} NEXUS_WmDrmPdPolicyStatus;


/***************************************************************************
Summary: 
External decryption context. Used for key rotation support.
***************************************************************************/
typedef struct NEXUS_WmDrmPdDecryptSettings
{
    NEXUS_SecurityOperation opType;              /* Operation to perfrom */
    NEXUS_SecurityAlgorithm algType;             /* Crypto algorithm */
    NEXUS_SecurityAlgorithmVariant algVariant;   /* Cipher chain mode for selected cipher */
    NEXUS_SecurityKeyType keySlotType;           /* Key destination entry type */
    NEXUS_SecurityTerminationMode termMode;      /* Termination Type for residual block to be ciphered */
    bool enableExtKey;                           /* Flag used to enable external key loading during dma transfer on the key slot.
                                                    true = key will prepend data in the dma descriptors. */
    bool enableExtIv;                            /* Flag used to enable external IV loading during dma transfer on the key slot.
                                                    true = iv will prepend data in the dma descriptors. */
    NEXUS_SecurityAesCounterSize aesCounterSize; /* This member is required for AES counter mode  */

}NEXUS_WmDrmPdDecryptSettings;

/***************************************************************************
Summary:
License protocol versions
***************************************************************************/
typedef enum NEXUS_WmDrmPdLicenseProtocolVersion
{
    NEXUS_WmDrmPdLicenseProtocolVersion_eUnknown,     /* Unknown */
    NEXUS_WmDrmPdLicenseProtocolVersion_eV2,          /* V2 */
    NEXUS_WmDrmPdLicenseProtocolVersion_eV3,          /* V3 */
    NEXUS_WmDrmPdLicenseProtocolVersion_eMax
} NEXUS_WmDrmPdLicenseProtocolVersion;

/***************************************************************************
Summary:
NEXUS_WmDrmPdID types
***************************************************************************/
#define NEXUS_WMDRM_ID_SIZE 16

typedef struct NEXUS_WmDrmPdID
{
    uint8_t id[NEXUS_WMDRM_ID_SIZE];
} NEXUS_WmDrmPdID;

typedef NEXUS_WmDrmPdID NEXUS_WmDrmPdKID;
typedef NEXUS_WmDrmPdID NEXUS_WmDrmPdLID;

/***************************************************************************
Summary:
License processing and storage result
***************************************************************************/
typedef struct NEXUS_WmDrmPdLicenseAck
{
    NEXUS_WmDrmPdKID kid;   /* kid of the license */
    NEXUS_WmDrmPdLID lid;   /* lid of the license */
    int32_t result;         /* DRM result code of the processing and storage */
    uint32_t flags;         /* Flag specifying post-processing behavior */
} NEXUS_WmDrmPdLicenseAck;

#define NEXUS_P_WMDRMPD_LICENSE_ACK_SIZE(num) ((num)*sizeof(NEXUS_WmDrmPdLicenseAck))

/***************************************************************************
Summary:
Structure that encapsulates the license processing result
***************************************************************************/
#define NEXUS_WMDRM_MAX_TRANSACTION_ID 100  /* Maximum size in bytes of a transaction ID */
#define NEXUS_WMDRM_MAX_LICENSE_ACK     20  /* Maximum number of licenses per license response */
#define NEXUS_WMDRM_MAX_EXT_LICENSE_ACK 75  /* Maximum number of licenses per license response using an external struct pAcks. Limit to keep size under 3kB. */

typedef struct NEXUS_WmDrmPdLicenseResponse
{
    NEXUS_WmDrmPdLicenseProtocolVersion protocolType;               /* Type of license protocol detected */
    uint8_t transactionID[ NEXUS_WMDRM_MAX_TRANSACTION_ID ];        /* Buffer to store the transaction ID */
    uint16_t transactionIDLength;                                   /* Size in bytes of the transaction ID */
    NEXUS_WmDrmPdLicenseAck acks[ NEXUS_WMDRM_MAX_LICENSE_ACK ];    /* License processing storage results */
    uint32_t acksCount;                                             /* Actual number of elements in the acks (or pAcks passed outside of NEXUS_WmDrmPdLicenseResponse) */
    int32_t result;                                                 /* Status code parsed from the server response */
} NEXUS_WmDrmPdLicenseResponse;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_WMDRMPD_TYPES_H_ */

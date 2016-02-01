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
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_DIVXDRM_H_
#define NEXUS_DIVXDRM_H_

#include "nexus_types.h"
#include "nexus_security_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NEXUS_DIVXDRM_REGISTRATION_CODE_LENGTH  11  /* Length in bytes for registration code string including 1 byte for NULL termination */
#define NEXUS_DIVXDRM_HARDWARE_SECRET_LENGTH    32
#define NEXUS_DIVXDRM_MEMORY_FRAGMENT1_LENGTH   20
#define NEXUS_DIVXDRM_MEMORY_FRAGMENT2_LENGTH   50
#define NEXUS_DIVXDRM_MEMORY_FRAGMENT3_LENGTH   10

#define NEXUS_DIVXDRM_MAX_TRACKS                32
#define NEXUS_DIVXDRM_MAX_FRAG_LEN              64
#define NEXUS_DIVXDRM_MAX_HEADER_DATA_LEN       2560
#define NEXUS_DIVXDRM_DD_CHUNK_LENGTH           10   /* 2 byte Frame Key Index, 4 byte Encryption offset, 4 byte Encryption Length */

/***************************************************************************
Summary:
DIVXDRM Handle
***************************************************************************/
typedef struct NEXUS_DivxDrm *NEXUS_DivxDrmHandle;

/***************************************************************************
Summary:
DIVXDRM Memory Structure
***************************************************************************/
typedef struct NEXUS_DivxDrmState {
    uint8_t fragment1[NEXUS_DIVXDRM_MAX_FRAG_LEN];         /* DrmMemory Fragment 1 */
    unsigned fragment1Length;
    uint8_t fragment2[NEXUS_DIVXDRM_MAX_FRAG_LEN];         /* DrmMemory Fragment 2 */
    unsigned fragment2Length;
    uint8_t fragment3[NEXUS_DIVXDRM_MAX_FRAG_LEN];         /* DrmMemory Fragment 3 */
    unsigned fragment3Length;
} NEXUS_DivxDrmState;


/***************************************************************************
Summary:
DIVXDRM Open settings.

Description:
Settings provided to create a DIVXDRM instance
***************************************************************************/
typedef struct NEXUS_DivxDrmCreateSettings
{
    uint8_t drmHardwareSecret[NEXUS_DIVXDRM_HARDWARE_SECRET_LENGTH];  /* Hardware Secret */
} NEXUS_DivxDrmCreateSettings;

/***************************************************************************
Summary:
DIVXDRM  settings.
***************************************************************************/
typedef struct NEXUS_DivxDrmStartSettings
{
    uint8_t drmHeaderData[NEXUS_DIVXDRM_MAX_HEADER_DATA_LEN];      /* Drm Header Data */
    unsigned drmHeaderDataLength;
    struct {
        NEXUS_PidType type;
        unsigned number;
    } track[NEXUS_DIVXDRM_MAX_TRACKS];
    NEXUS_CallbackDesc rentalMessageCallback;         /* Rental Message Callback */
    NEXUS_CallbackDesc outputProtectionCallback;      /* Output Protection callback */
    NEXUS_CallbackDesc ictCallback;                   /* ICT Signal callback */
} NEXUS_DivxDrmStartSettings;

/***************************************************************************
Summary:
DIVXDRM  registration and deactivation codes.
***************************************************************************/
typedef struct NEXUS_DivxDrmCodeString
{
    uint8_t codeString[NEXUS_DIVXDRM_REGISTRATION_CODE_LENGTH];
} NEXUS_DivxDrmCodeString;

/***************************************************************************
Summary:
DIVXDRM  Status
***************************************************************************/
typedef struct NEXUS_DivxDrmStatus
{
    struct {
        bool valid;
        unsigned useLimit;
        unsigned useCount;
    } rental;
    struct {
        bool valid;
        unsigned acptbSignal;
        unsigned cgmsaSignal;
    } outputProtection;
    struct {
        bool valid;
        unsigned ictSignal;
    } ict;
} NEXUS_DivxDrmStatus;

/***************************************************************************
Summary:
Get DIVXDRM default create settings.
***************************************************************************/
void NEXUS_DivxDrm_GetDefaultCreateSettings(
    NEXUS_DivxDrmCreateSettings *pSettings    /* [out] default open settings */
    );

/***************************************************************************
Summary:
Open a DIVXDRM handle.
***************************************************************************/
NEXUS_DivxDrmHandle NEXUS_DivxDrm_Create( /* attr{destructor=NEXUS_DivxDrm_Destroy} */
    const NEXUS_DivxDrmCreateSettings *pSettings
    );

/***************************************************************************
Summary:
Close a DIVXDRM handle.
***************************************************************************/
void NEXUS_DivxDrm_Destroy(
    NEXUS_DivxDrmHandle handle
    );

/***************************************************************************
Summary:
Get the DRM Memory buffers
***************************************************************************/
void NEXUS_DivxDrm_GetDrmState(
    NEXUS_DivxDrmHandle handle,
    NEXUS_DivxDrmState *pDrmState     /* [out] Divx Drm state */
    );

/***************************************************************************
Summary:
Set the DRM Memory buffers
***************************************************************************/
void NEXUS_DivxDrm_SetDrmState(
    NEXUS_DivxDrmHandle handle,
    const NEXUS_DivxDrmState *pDrmState
    );

/***************************************************************************
Summary:
Get DIVXDRM default create settings.
***************************************************************************/
void NEXUS_DivxDrm_GetDefaultStartSettings(
    NEXUS_DivxDrmStartSettings *pSettings    /* [out] default start settings */
    );

/***************************************************************************
Summary:
Start DIVXDRM
***************************************************************************/
NEXUS_Error NEXUS_DivxDrm_Start(
    NEXUS_DivxDrmHandle handle,
    const NEXUS_DivxDrmStartSettings *pSettings
    );

/***************************************************************************
Summary:
Stop DIVXDRM
***************************************************************************/
void NEXUS_DivxDrm_Stop(
    NEXUS_DivxDrmHandle handle
    );

/***************************************************************************
Summary:
Enable Decrypt. Called after start, after checking rental status
***************************************************************************/
NEXUS_Error NEXUS_DivxDrm_EnableDecrypt(
    NEXUS_DivxDrmHandle handle,
    NEXUS_KeySlotHandle keySlot
    );

/***************************************************************************
Summary:
Get status
***************************************************************************/
NEXUS_Error NEXUS_DivxDrm_GetStatus(
    NEXUS_DivxDrmHandle handle,
    NEXUS_DivxDrmStatus *pStatus       /* [out] Divx Drm status */
    );

/***************************************************************************
Summary:
Get the Registration Code String
***************************************************************************/
NEXUS_Error NEXUS_DivxDrm_GetRegistrationCodeString(
    NEXUS_DivxDrmHandle handle,
    NEXUS_DivxDrmCodeString *pRegistrationCode    /* [out] Registration Code String */
    );

/***************************************************************************
Summary:
Deactivate the device and get Deactivation Code String
***************************************************************************/
NEXUS_Error NEXUS_DivxDrm_Deactivate(
    NEXUS_DivxDrmHandle handle,
    NEXUS_DivxDrmCodeString  *pDeactivationCode   /* [out] De-activation Code String */
    );

/***************************************************************************
Summary:
DD chunk Information
***************************************************************************/
typedef struct NEXUS_DivxDrmInfo {
    uint8_t ddChunk[NEXUS_DIVXDRM_DD_CHUNK_LENGTH];
    unsigned trackNum;
} NEXUS_DivxDrmInfo;

/***************************************************************************
Summary:
Decrypt audio video data. used for MKV streams
***************************************************************************/
NEXUS_Error NEXUS_DivxDrm_Decrypt(
    NEXUS_DivxDrmHandle handle,
    NEXUS_KeySlotHandle keyslot,
    const void *pData,           /* attr{memory=cached} Payload to be decrypted */
    size_t length,               /* Length of payload to be decrypted */
    NEXUS_DivxDrmInfo *pDrmInfo  /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_DIVXDRM_H_ */

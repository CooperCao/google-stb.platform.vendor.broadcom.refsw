/******************************************************************************
 * (c) 2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#ifndef KEY_LOADER_TL_H__
#define KEY_LOADER_TL_H__

#include "bstd.h"
#include "bkni.h"

#include "bsagelib_crypto_types.h"
#include "nexus_security.h"

#include "key_loader_tl_types.h"


#ifdef __cplusplus
extern "C" {
#endif


/* This handle is used to store the context of a KeyLoaderTl instance */
typedef struct KeyLoaderTl_P_Instance *KeyLoaderTl_Handle;

typedef struct {
    uint32_t dummy; /* no module specific settings for now */
}KeyLoaderTlSettings;


/***************************************************************************
Summary:
Get default settings for loading the Key Loader module on SAGE

Description:
Retrieve the set of default values used to load the Key Loader module on SAGE

See Also:
KeyLoaderTl_Init()
***************************************************************************/
void KeyLoaderTl_GetDefaultSettings(KeyLoaderTlSettings *pKeyLoaderModuleSettings);

/***************************************************************************
Summary:
Initialize the Key Loader module on SAGE

Description:
Use the settings structure to pass any initialization values to the Key Loader
module on SAGE

See Also:
KeyLoaderTl_Uninit()
***************************************************************************/
BERR_Code KeyLoaderTl_Init(KeyLoaderTl_Handle *pKeyLoaderTlHandle, KeyLoaderTlSettings *pKeyLoaderModuleSettings);

/***************************************************************************
Summary:
Uninitialize the Key Loader module on SAGE

See Also:

***************************************************************************/
void KeyLoaderTl_Uninit(KeyLoaderTl_Handle hKeyLoaderTl);


/***************************************************************************
Summary:
Get default settings in order to allocate and configure a keyslot

Description:
Allows the caller to specify numerous variables for allocating and configuring
a keyslot to be used by SAGE.

See KeyLoader_keyslotConfigSettings structure definition

***************************************************************************/
void KeyLoader_GetDefaultConfigKeySlotSettings(KeyLoader_KeySlotConfigSettings *pKeyslotConfigSettings);


/***************************************************************************
Summary:
Allocate and configure a keyslot to use on SAGE

Description:
This function accepts a NULL NEXUS keyslot handle along with the settings to
apply to the SAGE keyslot to be allocated and configured.

A successful call to return a non-NULL value to the NEXUS keyslot handle
as well as BERR_SUCCESS return code

Example:
  {
       NEXUS_KeySlotHandle keySlotHandle = NULL;
       KeyLoader_keyslotConfigSettings keyslotConfigSettings;

      // Get Current Settings
      KeyLoader_GetDefaultConfigKeySlotSettings(&keyslotConfigSettings);

      //customize settings (example)
      keyslotConfigSettings.engine          = BSAGElib_Crypto_Engine_eCa;
      keyslotConfigSettings.algorithm       = BSAGElib_Crypto_Algorithm_eAes;
      keyslotConfigSettings.algorithmVar    = BSAGElib_Crypto_AlgorithmVariant_eCbc;
      keyslotConfigSettings.terminationMode = BSAGElib_Crypto_TerminationMode_eBlock;
      keyslotConfigSettings.solitarySelect  = BSAGElib_Crypto_SolitaryMode_eClear;
      keyslotConfigSettings.operation       = BSAGElib_Crypto_Operation_eDecrypt;
      keyslotConfigSettings.profileIndex    = 0;
      keyslotConfigSettings.custSubMode     = BSAGElib_Crypto_CustomerSubMode_eGeneric_CA_128_5;
      keyslotConfigSettings.keyType         = BSAGElib_Crypto_KeyType_eOddAndEven;

      // Apply settings
      rc = KeyLoader_AllocAndConfigKeySlot(&keySlotHandle, &keyslotConfigSettings);
      //check error code, etc....
  }
***************************************************************************/
BERR_Code KeyLoader_AllocAndConfigKeySlot(KeyLoaderTl_Handle hKeyLoaderTl,
                                          NEXUS_KeySlotHandle *pKeySlotHandle,
                                          KeyLoader_KeySlotConfigSettings *pKeySlotConfigSettings);

/***************************************************************************
Summary:
Free up a keyslot handle on SAGE

Description:
This function frees up the SAGE keyslot handle and the host side NEXUS
key slot handle

***************************************************************************/
void KeyLoader_FreeKeySlot(KeyLoaderTl_Handle hKeyLoaderTl,
                           NEXUS_KeySlotHandle hKeySlot);

/***************************************************************************
Summary:
Free a memory block

Description:
This function frees a memory block allocated using SRAI_Memory_Allocate()

See Also:
SRAI_Memory_Allocate()
***************************************************************************/
void KeyLoader_GetDefaultWrappedKeySettings(KeyLoader_WrappedKeySettings *pWrappedKeySettings);

/***************************************************************************
Summary:
Free a memory block

Description:
This function frees a memory block allocated using SRAI_Memory_Allocate()

See Also:
SRAI_Memory_Allocate()
***************************************************************************/
BERR_Code KeyLoader_LoadWrappedKey(KeyLoaderTl_Handle hKeyLoaderTl,
                                   NEXUS_KeySlotHandle hKeySlot,
                                   KeyLoader_WrappedKeySettings *pWrappedKeySettings);

/***************************************************************************
Summary:
Get default settings in order to update IV

Description:
Allows the caller to update the IV to be used by SAGE.

See KeyLoader_UpdateIvSettings structure definition

***************************************************************************/
void KeyLoader_GetDefaultUpdateIvSettings(KeyLoader_UpdateIvSettings *pUpdateIvSettings);

/**************************************************************************************
Summary:
Update IV use on SAGE

Description:
Use the settings structure to pass settings values to the Key loader Update IV
module on SAGE

**************************************************************************************/

BERR_Code KeyLoader_UpdateIv(KeyLoaderTl_Handle hKeyLoaderTl,
                             NEXUS_KeySlotHandle hKeySlot,
                             KeyLoader_UpdateIvSettings *pUpdateIvSettings);

#ifdef __cplusplus
}
#endif


#endif /*KEY_LOADER_TL_H__*/

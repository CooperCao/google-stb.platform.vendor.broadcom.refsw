/******************************************************************************
*(c) 2014 Broadcom Corporation
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
/******************************************************************************
 *
 * WVAssetRegistryAPI.h
 *
 * Asset registration API for offline playback
 *
 ******************************************************************************

#ifndef __WVASSETREGISTRYAPI_H__
#define __WVASSETREGISTRYAPI_H__

#include "WVTypes.h"
#include "WVStatus.h"


enum WVAssetStatus
{
    WVAssetStatus_Registered		= 0,
    WVAssetStatus_RequestingLicense	= 1,
    WVAssetStatus_HaveLicense		= 2,
    WVAssetStatus_UpdatingLicense	= 3,
    WVAssetStatus_RefusedLicense	= 4,
    WVAssetStatus_Expired		= 5,
    WVAssetStatus_ClearMedia		= 6,
    WVAssetStatus_Error			= 7
};


typedef void (*WVAssetRegistryStatusCB)(WVDictionary const& credentials,
					WVString const& assetPath,
					WVAssetStatus assetStatus,
					WVStatus failStatus,
                                        WVString const& failString);


WVStatus WV_AssetRegistryInitialize(WVDictionary const& credentials,
				    WVAssetRegistryStatusCB callback);


WVStatus WV_AssetRegistryTerminate(WVDictionary const& credentials);


WVStatus WV_RegisterAsset(WVDictionary const& credentials,
			  WVString const& assetPath,
			  bool requestLicense);


WVStatus WV_RegisterAsset(WVDictionary const& credentials,
			  unsigned long systemId,
			  unsigned long assetId,
			  unsigned long keyIndex,
			  bool requestLicense,
			  WVString& tempAssetPath);


WVStatus WV_UnregisterAsset(WVDictionary const& credentials,
			    WVString const& assetPath);


WVStatus WV_QueryAsset(WVDictionary const& credentials,
		       WVString const& assetPath,
		       WVDictionary& assetData);



WVStatus WV_QueryAllAssets(WVDictionary const& credentials,
			   WVTypedValueArray& assetData);


WVStatus WV_UpdateAssetLicenses(WVDictionary const& credentials,
				WVStringArray const& assetPaths);


WVStatus WV_UpdateAllAssetLicenses(WVDictionary const& credentials);


WVStatus WV_RemoveAssetLicenses(WVDictionary const& credentials,
				WVStringArray const& assetPaths);


WVStatus WV_ReplaceAssetPath(WVDictionary const& credentials,
			     WVString const& oldAssetPath,
			     WVString const& newAssetPath);



#endif // __WVASSETREGISTRYAPI_H__

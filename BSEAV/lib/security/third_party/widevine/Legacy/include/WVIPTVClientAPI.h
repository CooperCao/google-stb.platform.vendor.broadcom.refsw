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
 * WVIPTVClientAPI.h
 *
 * Declarations for Widevine IPTV Video Client API
 *****************************************************************************/

#ifndef __WV_IPTV_VIDEO_CLIENT_API_H__
#define __WV_IPTV_VIDEO_CLIENT_API_H__

#include "WVTypes.h"
#include "WVStatus.h"
#include "WVStreamControlAPI.h"

struct WVIptvCallbacks {
    void *(*allocBuffer)(size_t);
    void (*freeBuffer)(void *);
};

class WVIptvSession;

//API-wide resource initialization and deinitialization.
WVStatus WV_IPTV_Initialize(WVIptvCallbacks *callbacks = NULL);
WVStatus WV_IPTV_Terminate();

//Create and destroy per-stream state.
WVStatus WV_IPTV_CreateSession(WVIptvSession *&session, WVDictionary const &credentials);
WVStatus WV_IPTV_DestroySession(WVIptvSession *&session);

//
// METHOD: WV_IPTV_TSDecrypt
//
// Parameters:
//    [in] session - The session to query
//
//    [in] inBuffer - The encrypted source data buffer
//
//    [out] outBuffer - Decrypt destination data buffer (May be same as inBuffer for in-place decrypt)
//
//    [in] size - Number of bytes in source data buffer
//
//    [in] whichkey - Odd/Even key designation from scrambling bits
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//
WVStatus WV_IPTV_TSDecrypt(WVIptvSession *session, unsigned char *inBuffer, unsigned char *outBuffer, size_t *bytes);

//
// METHOD: WV_IPTV_GetCopyProtection
//
// This method retrieves information about the current copy protection settings
// as determined from the DRM license for the content.  The copy protection
// in the decoder (APS/Macrovision and/or HDCP) MUST be configured based on the
// information returned from this function.
//
// Parameters:
//    [in] session - The session to query
//
//    [out] macrovisionLevel - The requested macrovision settings
//
//    [out] enableHDCP - If true, HDCP should be enabled on the HDMI output
//
//    [out] CIT : If true, Image Constraint is required.
//
//  GetCopyProtection without CGMS-A (backward compatible).  This form is
//  deprecated and may be removed in a future release
//
// Returns:
//     WV_Status_OK on success, otherwise one of the WVStatus values
//     indicating the specific error.
//
WVStatus WV_IPTV_GetCopyProtection(WVIptvSession *session, WVMacrovision *macrovision, WVEMI *emi, bool *enableHDCP, bool *CIT);

//
// METHOD: WV_Info_GetVersion
//
// Parameters:
//    None
//
// Returns:
//      A string with the version information for the specific client build in use.
const char *WV_IPTV_GetVersion();

//
// METHOD: WV_Info_Unique_ID
//
// This routine will write the device's unique identifier into the buffer
// buffer must be at least 64-bytes.
//
// Parameters:
//    [out] buffer - A buffer to receive the device's unique identifier
//
WVStatus WV_IPTV_Unique_ID(char* buffer);

#endif //__WV_IPTV_VIDEO_CLIENT_API_H__

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
*   API name: Core Features
*       This file sets up chip-specific definitions for the core module
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef NEXUS_CORE_UTILS_H__
#define NEXUS_CORE_UTILS_H__

#include "nexus_types.h"
#include "berr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Information about a NEXUS_VideoFormat.

Description:
See NEXUS_VideoFormat_GetInfo
**/
typedef struct NEXUS_VideoFormatInfo
{
    uint32_t  width;                /* width in pixels */
    uint32_t  height;               /* height in pixels */
    uint32_t  digitalWidth;         /* digital width (in practice, always same as width) */
    uint32_t  digitalHeight;        /* digital height (in practice, same as height until this is an analog format of height 482 or 483) */
    uint32_t  scanWidth;            /* rasterization width */
    uint32_t  scanHeight;           /* rasterization height */
    uint32_t  topActive;            /* the start active video line of the top field or frame */
    uint32_t  topMaxVbiPassThru;    /* Maximum VBI Pass Through lines at the top field or frame */
    uint32_t  bottomMaxVbiPassThru; /* Maximum VBI Pass Through lines at the bottom field */
    uint32_t  verticalFreq;         /* picture vertical frequency, or refresh rate in units of 1/100th Hz.  For example 60.00Hz would be
                                       6000, and 59.94Hz would be 5994. */
    uint32_t  pixelFreq;            /* Pixel frequencies in units of 1/100th Mhz.  For example 74.24Mhz would be 7425, and
                                       148.50Mhz would be 14850. */
    bool      interlaced;           /* interlaced or progressive mode */
    NEXUS_AspectRatio aspectRatio;  /* Default Aspect Ratio associated with this format. */
    bool      isFullRes3d;          /* full-resolution 3D format or 2D format */
} NEXUS_VideoFormatInfo;

/**
Summary:
Get information about a NEXUS_VideoFormat.
**/
void NEXUS_VideoFormat_GetInfo(
    NEXUS_VideoFormat videoFmt,
    NEXUS_VideoFormatInfo *pVideoFmtInfo    /* [out] */
    );

typedef struct NEXUS_PixelFormatInfo {
    bool rgb;
    bool palette;
    bool alphaPerPixel;
    unsigned bpp; /* bits per pixel */
} NEXUS_PixelFormatInfo;

/**
Summary:
Get information about a NEXUS_PixelFormat.
**/
void NEXUS_PixelFormat_GetInfo(
    NEXUS_PixelFormat pixelFormat,
    NEXUS_PixelFormatInfo *pInfo
    );

/**
Summary:
Test if NEXUS_TransportType is a DSS type.

Description:
DSS is a 130-byte packet transport format.
**/
#define NEXUS_IS_DSS_MODE(TRANSPORTTYPE) \
    ((TRANSPORTTYPE) == NEXUS_TransportType_eDssEs || (TRANSPORTTYPE) == NEXUS_TransportType_eDssPes)


/**
Summary:
Stop all callbacks associated with this interface handle.

Description:
This waits for any running callbacks to finish and ensures that no other callbacks for this handle are fired.
If the handle is closed, the StopCallbacks request is cleared; there is no need to call StartCallbacks.
The Nexus proxy will automatically call StopCallbacks before an interface is closed.
**/
void NEXUS_StopCallbacks_tagged( /* attr{local=true} */
    void *interfaceHandle,
    const char *pFileName, unsigned lineNumber, const char *pFunctionName
    );
#if BDBG_DEBUG_BUILD
#define NEXUS_StopCallbacks(interfaceHandle) NEXUS_StopCallbacks_tagged(interfaceHandle, BSTD_FILE, BSTD_LINE, __func__)
#else
#define NEXUS_StopCallbacks(interfaceHandle) NEXUS_StopCallbacks_tagged(interfaceHandle, NULL, 0, NULL)
#endif

/**
Summary:
Re-enable all callbacks for this interface handle.

Description:
All interfaces default to being already started.
An app only needs to call StartCallbacks if it previously called StopCallbacks itself.
If you call StopCallbacks followed by StartCallbacks, no pending callbacks will be lost.
**/
void NEXUS_StartCallbacks_tagged( /* attr{local=true} */
    void *interfaceHandle,
    const char *pFileName, unsigned lineNumber, const char *pFunctionName
    );
#if BDBG_DEBUG_BUILD
#define NEXUS_StartCallbacks(interfaceHandle) NEXUS_StartCallbacks_tagged(interfaceHandle, BSTD_FILE, BSTD_LINE, __func__)
#else
#define NEXUS_StartCallbacks(interfaceHandle) NEXUS_StartCallbacks_tagged(interfaceHandle, NULL, 0, NULL)
#endif

/**
Summary:
Look up Nexus framerate enum

Description:
frameRateInteger is in units of 1000 FPS. For example:
    59940 => NEXUS_VideoFrameRate_e59_94
    23976 => NEXUS_VideoFrameRate_e23_976
    60000 => NEXUS_VideoFrameRate_e60

pNexusFrameRate will be NEXUS_VideoFrameRate_eUnknown if there is no match.
**/
void NEXUS_LookupFrameRate(
    unsigned frameRateInteger, /* framerate in units of 1000 fps. */
    NEXUS_VideoFrameRate *pNexusFrameRate /* [out] */
    );

/**
Summary:
Create generic keyslot.

Description:
See NEXUS_Security_AllocateKeySlot for Security module keyslot.
**/
NEXUS_KeySlotHandle NEXUS_KeySlot_Create( /* attr{destructor=NEXUS_KeySlot_Destroy} */
    void);
    
/**
Summary:
Destroy generic keyslot.

Description:
See NEXUS_Security_FreeKeySlot for Security module keyslot.
You can destroy a Security module with this, but destruction of internal resources will be deferred.
**/
void NEXUS_KeySlot_Destroy(
    NEXUS_KeySlotHandle keyslot
    );
    
typedef void *NEXUS_KeySlotTag;

void NEXUS_KeySlot_SetTag(
    NEXUS_KeySlotHandle keyslot,
    NEXUS_KeySlotTag tag
    );

void NEXUS_KeySlot_GetTag(
    NEXUS_KeySlotHandle keyslot,
    NEXUS_KeySlotTag *pTag
    );

void NEXUS_KeySlot_GetInfo(
    NEXUS_KeySlotHandle keyslot,
    NEXUS_SecurityKeySlotInfo *pKeyslotInfo
    );

#include "nexus_core_priv.h"

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_CORE_UTILS_H__ */


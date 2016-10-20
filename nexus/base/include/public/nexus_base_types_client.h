/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2004-2016 Broadcom. All rights reserved.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
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
***************************************************************************/

#ifndef NEXUS_BASE_TYPES_CLIENT_H
#define NEXUS_BASE_TYPES_CLIENT_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************************************************************
Summary:
Heap handle

Description:
This is used to manage custom memory configurations.
A NEXUS_HeapHandle is created by specifying custom heap options in NEXUS_PlatformSettings.
***************************************************************************/
typedef struct NEXUS_Heap *NEXUS_HeapHandle;


/***************************************************************************
Summary:
NEXUS_ClientResources allows server to limit resources available to untrusted clients

Resources are set by the server in two ways:
1. using NEXUS_ClientSettings.configuration when calling NEXUS_Platform_RegisterClient
2. using NEXUS_PlatformStartServerSettings.unauthenticatedConfiguration when calling NEXUS_Platform_StartServer

Resources are enforced in each module using nexus_client_resources.h macros

Resources can be read by client using NEXUS_Platform_GetClientConfiguration
***************************************************************************/
#define NEXUS_MAX_IDS 16
typedef struct NEXUS_ClientResourceIdList
{
    unsigned id[NEXUS_MAX_IDS]; /* packed array of 'total' elements. */
    unsigned total; /* count of elements in id[] */
} NEXUS_ClientResourceIdList;

typedef struct NEXUS_ClientResourceCount
{
    unsigned total; /* count of resources */
} NEXUS_ClientResourceCount;

typedef struct NEXUS_ClientResources
{
    NEXUS_ClientResourceIdList simpleAudioDecoder;
    NEXUS_ClientResourceIdList simpleVideoDecoder;
    NEXUS_ClientResourceIdList simpleEncoder;
    NEXUS_ClientResourceIdList surfaceClient;
    NEXUS_ClientResourceIdList inputClient;
    NEXUS_ClientResourceIdList audioCapture;
    NEXUS_ClientResourceIdList audioCrc;

    NEXUS_ClientResourceCount dma;
    NEXUS_ClientResourceCount graphics2d;
    NEXUS_ClientResourceCount graphicsv3d;
    NEXUS_ClientResourceCount pictureDecoder;
    NEXUS_ClientResourceCount playpump;
    NEXUS_ClientResourceCount recpump;
    NEXUS_ClientResourceCount simpleAudioPlayback;
    NEXUS_ClientResourceCount simpleStcChannel;
    NEXUS_ClientResourceCount surface;
    struct {
        unsigned sizeLimit;
    } temporaryMemory; /* memory that is allocated for duration of call to hold temporary data */
} NEXUS_ClientResources;

/**
Summary:
Client modes

See nexus/docs/Nexus_MultiProcess.pdf for full discussion of process isolation and multi-process application design.
**/
typedef enum NEXUS_ClientMode
{
    NEXUS_ClientMode_eUnprotected, /* deprecated */
    NEXUS_ClientMode_eVerified,    /* verify handle value, but not owner. unsynchronized caller may compromise nexus settings. */
    NEXUS_ClientMode_eProtected,   /* full handle verification. access to full API. if client crashes, server is protected. */
    NEXUS_ClientMode_eUntrusted,   /* full handle verification. access to limited API. see nexus/build/common/tools/nexus_untrusted_api.txt. if client crashes, server is protected. */
    NEXUS_ClientMode_eMax
} NEXUS_ClientMode;

/* NEXUS_MAX_HEAPS is the maximum number of heaps in the system, pointed to be NEXUS_HeapHandle.
A heap is required for any memory access, whether by HW or SW.
This depends on both HW capabilities & SW configuration. */
#define NEXUS_MAX_HEAPS 24

/* NEXUS_MAX_MEMC is the maximum number of memory controllers in the system. */
#define NEXUS_MAX_MEMC 3

/**
Summary:
Information provided by the server to the client
**/
typedef struct NEXUS_ClientConfiguration
{
    NEXUS_ClientMode mode; /* default is eProtected. */
    NEXUS_HeapHandle heap[NEXUS_MAX_HEAPS]; /* untrusted client will be restricted to these heaps. heap[0] will be its default. */
    NEXUS_ClientResources resources; /* resources granted by the server for untrusted clients */
} NEXUS_ClientConfiguration;

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_BASE_TYPES_CLIENT_H */

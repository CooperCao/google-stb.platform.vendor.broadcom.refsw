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
#ifndef NEXUS_PLATFORM_COMMON_H__
#define NEXUS_PLATFORM_COMMON_H__

#include "nexus_types.h"
#include "nexus_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Settings used to identify a client
**/
typedef struct NEXUS_ClientAuthenticationSettings
{
    NEXUS_Certificate certificate;

    /* TODO: struct {
    } preferences; */
} NEXUS_ClientAuthenticationSettings;

typedef struct NEXUS_PlatformObjectInstance {
    bool readOnly; /* set true for handles not owned by this client */
    NEXUS_AnyObject object;
} NEXUS_PlatformObjectInstance;

#define NEXUS_INTERFACE_NAME_MAX   32
/**
Summary:
**/
typedef struct NEXUS_InterfaceName {
    char name[NEXUS_INTERFACE_NAME_MAX];
} NEXUS_InterfaceName;

/**
Summary:
Initializes the NEXUS_Platform_InterfaceName structure
**/
void NEXUS_Platform_GetDefaultInterfaceName(
        NEXUS_InterfaceName *name
        );

#define NEXUS_PLATFORM_ERR_OVERFLOW     NEXUS_MAKE_ERR_CODE(0x10B, 0)

/**
Summary:
Server-side handle per client.
**/
typedef struct NEXUS_Client *NEXUS_ClientHandle;

/*
Summary:
Returns opened handles of specified type 

Description:
This function would return only objects that are accessible from the caller context

NEXUS_PLATFORM_ERR_OVERFLOW returned if not enough space to return all handles

If pObjects is NULL or nobjects is 0, pValidObjects will return the total number of objects that could be returned.
*/
NEXUS_Error NEXUS_Platform_GetClientObjects(
        NEXUS_ClientHandle client, /* attr{null_allowed=y} */
        const NEXUS_InterfaceName *pName, 
        NEXUS_PlatformObjectInstance *pObjects,   /* [out] attr{nelem=nobjects;nelem_out=pValidObjects;null_allowed=y} */
        size_t nobjects, 
        size_t *pValidObjects
        );

#define NEXUS_Platform_GetObjects(NAME, OBJECTS, NOBJECTS, PVALIDOBJECTS) NEXUS_Platform_GetClientObjects(NULL, NAME, OBJECTS, NOBJECTS, PVALIDOBJECTS)

#ifdef __cplusplus
}
#endif

#endif


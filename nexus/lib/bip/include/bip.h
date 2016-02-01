/***************************************************************************
*     (c)2015 Broadcom Corporation
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
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef BIP_H
#define BIP_H

/** @addtogroup bip

BIP Interface Definition.

   This include file defines the public interfaces to the BIP (Broadcom IPtv
   Library)

   This includes such things as:
   1.  BIP status code definitions
   2.  Public BIP macros, datatypes and enums.
   3.  Function prototypes for BIP APIs.
*/

     /* Define global BIP datatypes and enums. */

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "b_os_lib.h"
#include "bip_types.h"

    /* Define each of the BIP Interfaces. */

    /* Base header files: */
#include "bip_status.h"
#include "bip_string.h"
#include "bip_xml.h"
#include "bip_inet.h"
#include "bip_mem.h"
#include "bip_fd.h"
#include "bip_media_info.h"
#include "bip_url.h"
#include "bip_iochecker.h"
#include "bip_timer.h"
#include "bip_listener.h"
#include "bip_socket.h"
#include "bip_dtcp_ip.h"
#include "bip_http_header_list.h"
#include "bip_http_request.h"
#include "bip_http_request.h"
#include "bip_http_response.h"
#include "bip_http_response.h"
#include "bip_http_socket.h"
#include "bip_ssl.h"

    /* Client header files: */
#include "bip_client.h"
#include "bip_dtcp_ip_server.h"
#include "bip_dtcp_ip_client.h"
#include "bip_dlna.h"
#include "bip_player.h"

    /* Server  header files: */
#include "bip_transcode.h"
#include "bip_streamer.h"
#include "bip_http_streamer.h"
#include "bip_http_server.h"
#include "bip_igmp_listener.h"
#include "bip_rtsp_request.h"
#include "bip_rtsp_response.h"
#include "bip_rtsp_session.h"
#include "bip_rtsp_listener.h"
#include "bip_rtsp_server.h"
#include "bip_udp_streamer.h"

    /* Header files that reference all of BIP. */
#include "bip_to_str.h" /* Keep this and next one last so it sees everyone elses enum declarations. */
#include "bip_str_to_value.h"

    /*  Define prototypes for the public APIs defined in bip.c. */

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Settings for BIP_Init().

See Also:
BIP_Init
BIP_GetDefaultInitSettings
**/
typedef struct BIP_InitSettings
{
    BIP_SETTINGS(BIP_InitSettings)                   /*!< Internal use... for init verification. */
} BIP_InitSettings;
BIP_SETTINGS_ID_DECLARE(BIP_InitSettings);


/**
Summary:
Get default settings for BIP_Init().

See Also:
BIP_Init
BIP_InitSettings
**/
#define BIP_GetDefaultInitSettings( pSettings )                   \
    BIP_SETTINGS_GET_DEFAULT_BEGIN( pSettings, BIP_InitSettings ) \
    /* Set non-zero defaults explicitly. */                       \
    BIP_SETTINGS_GET_DEFAULT_END


/**
Summary:
Initialize the BIP infrastructure.

Description:
Allocates resources that are needed by various BIP components, and marks them as being in use.
When an application is finished with BIP and wants to release these resources, it will call
BIP_Uninit().

BIP_Init() and BIP_Uninit() maintain a reference count, so that they can be called by
multiple "client" applications (within the same process) and perform their respective
initialization and uninitialization  only during the first or last call, respectively.
Subsequent calls to BIP_Init() by other clients will simply increment the reference
count and return.

Likewise, BIP_Uninit() decrements and checks the reference count, so that when the last client
calls BIP_Uninit(), it is safe to deallocate any remaining BIP resources.

- Client A calls BIP_Init()
  - Reference count is zero, so BIP is initialized and reference count is set to one.
- Client B calls BIP_Init()
  - Reference count is non-zero, no initialization required, reference count set to two.
- Client B calls BIP_Uninit()
  - Reference count is decremented to one, but not yet zero, so return without freeing resources.
- Client A call BIP_Uninit()
  - Reference count is decremented to zero, indicating no more clients, so BIP resources are freed.

Return:
    BIP_SUCCESS :    Normal successful return.

See Also:
BIP_InitSettings
BIP_GetDefaultInitSettings
BIP_Uninit
**/
BIP_Status BIP_Init(BIP_InitSettings *pSettings);


/**
Summary:
Uninitialize the BIP infrastructure and free resources.

Description:
Indicates that BIP is no longer in use by a client application.  If the reference count
indicates that all other client applications have called BIP_Uninit(), then any remaining
BIP resources will be freed.

Return:
    BIP_SUCCESS             :    Normal successful return.

Return:
    BIP_ERR_NOT_INITIALIZED :    The reference count indicates that there are no active
                                 application clients.

See Also:
BIP_Init
**/
void BIP_Uninit(void);

/**
Summary:
Structure for returning BIP's Init status.

See Also:
BIP_Init
BIP_Init_GetStatus
**/
typedef struct BIP_InitStatus
{
    int   refCount;         /*!< Number of currently Initialized client applications. */
} BIP_InitStatus;


/**
Summary:
Return BIP's initialization status.

Return:
    BIP_SUCCESS             :    Normal successful return.

See Also:
BIP_InitStatus
BIP_Init
**/
BIP_Status BIP_Init_GetStatus( BIP_InitStatus *pStatus );


#ifdef __cplusplus
}
#endif

#endif /* !defined BIP_H */

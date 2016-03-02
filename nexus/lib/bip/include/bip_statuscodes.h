/***************************************************************************
*     (c)2007-2016 Broadcom Corporation
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
#ifndef BIP_STATUSCODES_H
#define BIP_STATUSCODES_H

/* All BIP-specific status codes should be defined in this file.
 * Please don't add anything else, since this file is used for
 * two different purposes (by redefining the BIP_MAKE_ERR_CODE
 * macro before including it).  */

/**
 * Summary:
 * List of all BIP-specific status codes.
 *
 *  */

/* Don't use the BIP_NEXUS_STATUS_CODE_ID for SUCCESS so that it is a real zero, same as NEXUS_SUCCESS and BERR_SUCCESS. */
BIP_MAKE_STATUS_CODE( BIP_SUCCESS, 0,  0x0) /* success (always zero) */

/* Now the remainder of the BIP status codes will be in their own range.  The
 * reserved range is all 32-bit numbers that have BIP_NEXUS_STATUS_CODE_ID
 * as the upper 16 bits.*/

/* Define the upper 16 bits for BIP status codes   Use "B1F" because it looks like "BIP" ;^)  */
#define BIP_NEXUS_STATUS_CODE_ID 0xB1F /* Used by BIP to call NEXUS_MAKE_ERR_CODE(id,num) */

/* General Purpose Status Codes:*/
    /* Range 0x0000 -> 0x007f: Error Codes: Normal discrete BIP "Error" statuses. */
    BIP_MAKE_STATUS_CODE( BIP_ERR_NOT_INITIALIZED,        BIP_NEXUS_STATUS_CODE_ID, 0x001) /* parameter not initialized */
    BIP_MAKE_STATUS_CODE( BIP_ERR_INVALID_PARAMETER,      BIP_NEXUS_STATUS_CODE_ID, 0x002) /* parameter is invalid */
    BIP_MAKE_STATUS_CODE( BIP_ERR_OUT_OF_SYSTEM_MEMORY,   BIP_NEXUS_STATUS_CODE_ID, 0x003) /* out of KNI module memory (aka OS memory) */
    BIP_MAKE_STATUS_CODE( BIP_ERR_OUT_OF_DEVICE_MEMORY,   BIP_NEXUS_STATUS_CODE_ID, 0x004) /* out of MEM module memory (ala heap memory) */
    BIP_MAKE_STATUS_CODE( BIP_ERR_OS_CHECK_ERRNO,         BIP_NEXUS_STATUS_CODE_ID, 0x005) /* Bad "errno" value from OS. Deprecated: Use status = BIP_StatusFromErrno(errnoValue) */
    BIP_MAKE_STATUS_CODE( BIP_ERR_NOT_SUPPORTED,          BIP_NEXUS_STATUS_CODE_ID, 0x006) /* requested feature is not supported */
    BIP_MAKE_STATUS_CODE( BIP_ERR_NOT_AVAILABLE,          BIP_NEXUS_STATUS_CODE_ID, 0x007) /* no resource available */
    BIP_MAKE_STATUS_CODE( BIP_ERR_INTERNAL,               BIP_NEXUS_STATUS_CODE_ID, 0x008)
    BIP_MAKE_STATUS_CODE( BIP_ERR_NETWORK_PEER_ABORT,     BIP_NEXUS_STATUS_CODE_ID, 0x009)
    BIP_MAKE_STATUS_CODE( BIP_ERR_NETWORK_TIMEOUT,        BIP_NEXUS_STATUS_CODE_ID, 0x00A)
    BIP_MAKE_STATUS_CODE( BIP_ERR_ALREADY_IN_PROGRESS,    BIP_NEXUS_STATUS_CODE_ID, 0x00B)
    BIP_MAKE_STATUS_CODE( BIP_ERR_INVALID_API_SEQUENCE,   BIP_NEXUS_STATUS_CODE_ID, 0x00C)
    BIP_MAKE_STATUS_CODE( BIP_ERR_OBJECT_BEING_DESTROYED, BIP_NEXUS_STATUS_CODE_ID, 0x00D) /* Object destruction has begun, no new APIs allowed. */
    BIP_MAKE_STATUS_CODE( BIP_ERR_B_OS_LIB,               BIP_NEXUS_STATUS_CODE_ID, 0x00E) /* B_Os Library Error */
    BIP_MAKE_STATUS_CODE( BIP_ERR_NEXUS,                  BIP_NEXUS_STATUS_CODE_ID, 0x00F) /* NEXUS Error */
    BIP_MAKE_STATUS_CODE( BIP_ERR_INVALID_REQUEST_TARGET, BIP_NEXUS_STATUS_CODE_ID, 0x010) /* HTTP Request Contains Invalid Request Target */
    BIP_MAKE_STATUS_CODE( BIP_ERR_CREATE_FAILED,          BIP_NEXUS_STATUS_CODE_ID, 0x011) /* A create API failed but didn't return a specific reason. */
    BIP_MAKE_STATUS_CODE( BIP_ERR_INVALID_HANDLE,         BIP_NEXUS_STATUS_CODE_ID, 0x012) /* An object handle pointed an object that didn't exist. */

    /* Range 0x0080 -> 0x00ff: Informational Codes:  Normal discrete BIP "Informational" statuses. */
    BIP_MAKE_STATUS_CODE( BIP_INF_IN_PROGRESS,            BIP_NEXUS_STATUS_CODE_ID, 0x081)
    BIP_MAKE_STATUS_CODE( BIP_INF_TIMEOUT,                BIP_NEXUS_STATUS_CODE_ID, 0x082) /* reached timeout limit */
    BIP_MAKE_STATUS_CODE( BIP_INF_END_OF_STREAM,          BIP_NEXUS_STATUS_CODE_ID, 0x083)
    BIP_MAKE_STATUS_CODE( BIP_INF_NOT_AVAILABLE,          BIP_NEXUS_STATUS_CODE_ID, 0x084) /* no resource available */

    /* BIP_Arb Status Codes: */
    /* Range 0x0100 -> 0x017f: Error Codes: Normal discrete BIP_Arb "Error" statuses. */
    BIP_MAKE_STATUS_CODE( BIP_ERR_ARB_INTERNAL,           BIP_NEXUS_STATUS_CODE_ID, 0x101)
    BIP_MAKE_STATUS_CODE( BIP_ERR_ARB_BEING_DESTROYED,    BIP_NEXUS_STATUS_CODE_ID, 0x102)

    /* Range 0x0180 -> 0x01ff: Informational Codes: Normal discrete BIP_Arb "Informational" statuses. */
    BIP_MAKE_STATUS_CODE( BIP_INF_ARB_IN_PROGRESS,        BIP_NEXUS_STATUS_CODE_ID, 0x181)

    /* BIP_Media_Info Status Codes*/
    /* Range 0x0200 -> 0x02ff: Error Codes: Normal discrete BIP_MediaInfo "Error" statuses. */
    BIP_MAKE_STATUS_CODE( BIP_ERR_MEDIA_INFO_BAD_INFOFILE_CONTENTS, BIP_NEXUS_STATUS_CODE_ID, 0x201) /* contents within info file invalid. May need to be regenerated */
    BIP_MAKE_STATUS_CODE( BIP_ERR_MEDIA_INFO_BAD_INFOFILE_PATH,     BIP_NEXUS_STATUS_CODE_ID, 0x202) /* info file specified does not exist, or bad path to info file. Check input to bip media info api */
    BIP_MAKE_STATUS_CODE( BIP_ERR_MEDIA_INFO_BAD_MEDIA_PATH,        BIP_NEXUS_STATUS_CODE_ID, 0x203) /* media file specified does not exist, or bad path to media file. Check input to bip media info api */
    BIP_MAKE_STATUS_CODE( BIP_ERR_MEDIA_INFO_BAD_HLS_PATH,          BIP_NEXUS_STATUS_CODE_ID, 0x204) /* hls master playlist  file specified does not exist, or bad path to master playlist  file. Check input to bip media info api */
    BIP_MAKE_STATUS_CODE( BIP_ERR_MEDIA_INFO_BAD_DASH_PATH,         BIP_NEXUS_STATUS_CODE_ID, 0x205) /* dash mpd file specified does not exist, or dash mpd path to info file. Check input to bip media info api */

    BIP_MAKE_STATUS_CODE( BIP_INF_MEDIA_INFO_UNKNOWN_CONTAINER_TYPE,BIP_NEXUS_STATUS_CODE_ID, 0x280) /* file is present but container type is unknown, may be text/html/m3u8, etc. type file */
    BIP_MAKE_STATUS_CODE( BIP_INF_MEDIA_INFO_VERSION_MISMATCH, BIP_NEXUS_STATUS_CODE_ID, 0x281)      /* media info file version mismatch. Need to regerate the info files.*/

    /* BIP_HttpServer Status Codes*/
    /* Range 0x0300 -> 0x03ff: Error Codes: Normal discrete BIP_HttpServer "Error" status. */
    BIP_MAKE_STATUS_CODE( BIP_ERR_SERVER_ERR,                       BIP_NEXUS_STATUS_CODE_ID, 0x301)  /* TODO: Add comment when error code is defined. */
    BIP_MAKE_STATUS_CODE( BIP_ERR_INTERFACE_NAME_TO_IP_ADDR,        BIP_NEXUS_STATUS_CODE_ID, 0x302)  /* Failed to Determine IP Address associated with the given Interface Name. */
    BIP_MAKE_STATUS_CODE( BIP_ERR_INITIAL_REQUEST_TIMEDOUT,         BIP_NEXUS_STATUS_CODE_ID, 0x303)  /* Initial HTTP Request is NOT received on HTTP Socket in the timeout interval. */
    BIP_MAKE_STATUS_CODE( BIP_ERR_PERSISTENT_CONNECTION_TIMEDOUT,   BIP_NEXUS_STATUS_CODE_ID, 0x304)  /* Next HTTP Request is NOT received on HTTP Socket in the timeout interval. */

    BIP_MAKE_STATUS_CODE( BIP_INF_NEXUS_RESOURCE_NOT_AVAILABLE,     BIP_NEXUS_STATUS_CODE_ID, 0x350)  /* Nexus Resources needed for Streaming are not available. */

    /* BIP_HttpDtcpIpServer Status Codes*/
    /* Range 0x0400 -> 0x045f: Error Codes: Normal discrete BIP_HttpDtcpIpServer "Error" status. */
    BIP_MAKE_STATUS_CODE( BIP_ERR_DTCPIP_HW_SECURITY_PARAMS,        BIP_NEXUS_STATUS_CODE_ID, 0x401)  /* DtcpInitHWSecurityParams() Failed: key loading error? */
    BIP_MAKE_STATUS_CODE( BIP_ERR_DTCPIP_SERVER_START,              BIP_NEXUS_STATUS_CODE_ID, 0x402)  /* DtcpAppLib_Startup Failed */
    BIP_MAKE_STATUS_CODE( BIP_ERR_DTCPIP_SERVER_LISTEN,             BIP_NEXUS_STATUS_CODE_ID, 0x403)  /* DtcpAppLib_Listen Failed */

    /* BIP_HttpDtcpIpClient Status Codes*/
    /* Range 0x0460 -> 0x04ff: Error Codes: Normal discrete BIP_DtcpIpClient "Error" status. */
    BIP_MAKE_STATUS_CODE( BIP_ERR_DTCPIP_CLIENT_INIT,               BIP_NEXUS_STATUS_CODE_ID, 0x461)  /* DtcpAppLib_Startup Failed for the Sink/Client interface. */
    BIP_MAKE_STATUS_CODE( BIP_ERR_DTCPIP_CLIENT_AKE,                BIP_NEXUS_STATUS_CODE_ID, 0x463)  /* DtcpAppLib_DoAke Failed */

    /* BIP HTTP Request & Response Error Status Codes: grouped into BIP_ERR_HTTP_MESSAGE_ */
    /* Range 0x0500 -> 0x055f: Error & Info Codes. */
    BIP_MAKE_STATUS_CODE( BIP_ERR_HTTP_MESSAGE_INVALID,        BIP_NEXUS_STATUS_CODE_ID, 0x501)  /* INVALID HTTP Request/Response Message. */
    BIP_MAKE_STATUS_CODE( BIP_ERR_HTTP_MESSAGE_HEADER_NOT_SET, BIP_NEXUS_STATUS_CODE_ID, 0x502)  /* App is not allowed to Set or Modify this Header. */

    /* Range 0x0560 -> 0x05ff: Error & Info Codes. */
    /* BIP HTTP Request & Response Info Status Codes */
    BIP_MAKE_STATUS_CODE( BIP_INF_HTTP_MESSAGE_HEADER_NOT_PRESENT, BIP_NEXUS_STATUS_CODE_ID, 0x561)  /* Header is not present. */
    BIP_MAKE_STATUS_CODE( BIP_INF_HTTP_MESSAGE_HEADER_ALREADY_SET, BIP_NEXUS_STATUS_CODE_ID, 0x562)  /* Header is already set/present. */

    /* Range 0x0600 -> 0x06ff: Error & Info Codes. */
    /* BIP Parser/Renderer Error Codes */
    BIP_MAKE_STATUS_CODE( BIP_ERR_DESERIALIZE_IN_PROGRESS,  BIP_NEXUS_STATUS_CODE_ID, 0x601)  /* Deserialize (parsing into an object) is in progress. */
    BIP_MAKE_STATUS_CODE( BIP_ERR_SERIALIZE_IN_PROGRESS, BIP_NEXUS_STATUS_CODE_ID, 0x602)     /* Serialize (rendering from an object) is in progress. */

    /* Range 0x0700 -> 0x08ff: Error & Info Codes for Client. */
    /* Range 0x0700 -> 0x07ff: Error & Info Codes for Client: Player. */

    /* Range 0x0700 -> 0x077f: DTCP/IP Error Codes */
    BIP_MAKE_STATUS_CODE( BIP_ERR_PLAYER_CONNECT,                   BIP_NEXUS_STATUS_CODE_ID, 0x701)   /* Connect to Server Failed. */
    BIP_MAKE_STATUS_CODE( BIP_ERR_PLAYER_INTERNAL,                  BIP_NEXUS_STATUS_CODE_ID, 0x702)   /* Player Internal Error. */
    BIP_MAKE_STATUS_CODE( BIP_ERR_PLAYER_PROBE,                     BIP_NEXUS_STATUS_CODE_ID, 0x703)   /* Media Probe Failed. */
    BIP_MAKE_STATUS_CODE( BIP_ERR_PLAYER_BEING_DISCONNECTED,        BIP_NEXUS_STATUS_CODE_ID, 0x704)   /* Media Player object is being Disconnected. */
    BIP_MAKE_STATUS_CODE( BIP_ERR_PLAYER_PBIP,                      BIP_NEXUS_STATUS_CODE_ID, 0x705)   /* Playback Ip Error. */
    BIP_MAKE_STATUS_CODE( BIP_ERR_PLAYER_MISSING_CONTENT_TYPE,      BIP_NEXUS_STATUS_CODE_ID, 0x706)   /* Content-type header is missing, required when DTCP/IP link protection is enabled. */
    BIP_MAKE_STATUS_CODE( BIP_ERR_PLAYER_PAUSE,                     BIP_NEXUS_STATUS_CODE_ID, 0x707)   /* Pause Failed. */
    BIP_MAKE_STATUS_CODE( BIP_ERR_PLAYER_PLAY,                      BIP_NEXUS_STATUS_CODE_ID, 0x708)   /* Play Failed. */
    BIP_MAKE_STATUS_CODE( BIP_ERR_PLAYER_SEEK,                      BIP_NEXUS_STATUS_CODE_ID, 0x709)   /* Seek Failed. */
    BIP_MAKE_STATUS_CODE( BIP_ERR_PLAYER_PLAY_AT_RATE,              BIP_NEXUS_STATUS_CODE_ID, 0x710)   /* PlayAtRate Failed. */

    /* Range 0x0780 -> 0x07ff: Informational Codes */
    BIP_MAKE_STATUS_CODE( BIP_INF_PLAYER_IN_PROGRESS,        BIP_NEXUS_STATUS_CODE_ID, 0x781)   /* API is in progress. */
    BIP_MAKE_STATUS_CODE( BIP_INF_PLAYER_CANT_PAUSE,         BIP_NEXUS_STATUS_CODE_ID, 0x782)   /* Can't Pause at the current position. */

    /* Range 0x0800 -> 0x081f: SSL Error Codes */
    BIP_MAKE_STATUS_CODE( BIP_ERR_SSL_INIT,                 BIP_NEXUS_STATUS_CODE_ID, 0x801)   /* SSL Init Failed. */
    /* Range 0x0900 -> 0x0fff: Unused (available for future) */

    /* BIP Errno Status Codes: */
    /* Range 0x1000 - 0x1fff: Error Codes:  Encoded "errno" value: 0x1000 + errno */
    BIP_MAKE_STATUS_CODE( BIP_ERR_OS_ERRNO_MIN         , BIP_NEXUS_STATUS_CODE_ID, 0x1001)
    BIP_MAKE_STATUS_CODE( BIP_ERR_OS_ERRNO_MAX         , BIP_NEXUS_STATUS_CODE_ID, 0x1FFD)
    BIP_MAKE_STATUS_CODE( BIP_ERR_OS_ERRNO             , BIP_NEXUS_STATUS_CODE_ID, 0x1FFE) /* Dummy status used to describe all from BIP_ERR_OS_ERRNO_MIN to BIP_ERR_OS_ERRNO_MAX. */
    BIP_MAKE_STATUS_CODE( BIP_ERR_OS_ERRNO_INVALID     , BIP_NEXUS_STATUS_CODE_ID, 0x1FFF) /* Probably passed an invalid "errno" value to BIP_StatusFromErrno(). */

    /* Range 0x2000 -> 0xffff: Unused (available for future) */

#endif /* !defined BIP_STATUSCODES_H */

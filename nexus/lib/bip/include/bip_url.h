/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ***************************************************************************/
#ifndef BIP_URL_H
    #define BIP_URL_H

/* BIP Url implements the functionality of parsing and manipulating URLs
 * (Universal Resource Locators). Note that it only supports URLs as used in the
 * context of BIP and is not a full-featured URL parser.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "bip.h"

/*************************************************************************
 *  Class: BIP_Url
 **************************************************************************/

typedef struct BIP_Url
{
    BDBG_OBJECT( BIP_Url )

    char *urlRaw;

    bool        parsed;

    char        *scheme;
    char        *userinfo;
    char        *host;
    char        *port;
    char        *path;
    char        *query;
    char        *fragment;
} BIP_Url;

typedef struct BIP_Url  *BIP_UrlHandle;

#define BIP_URL_PRINTF_FMT                                            \
    "[hUrl=%p Scheme=\"%s\" Userinfo=\"%s\" Host=\"%s\" Port=\"%s\" " \
    "Path=\"%s\" Query=\"%s\" Frag=\"%s\" Raw=\"%s\"]"

#define BIP_URL_PRINTF_ARG(hUrl)  \
    (void *)(hUrl),                       \
    (hUrl)->scheme,               \
    (hUrl)->userinfo,             \
    (hUrl)->host,                 \
    (hUrl)->port,                 \
    (hUrl)->path,                 \
    (hUrl)->query,                \
    (hUrl)->fragment,             \
    (hUrl)->urlRaw


/**
 * Summary:
 * Create a BIP_Url object.
 *
 * Description:
 * Allocates a BIP_Url structure (object) and associates it with a copy of the
 * specified URL string.
 *
 **/
BIP_UrlHandle  BIP_Url_Create( const char *urlString );

/**
 * Summary:
 * Create a BIP_Url object.
 *
 * Description:
 * Makes a copy of the specified string and associates it with an existing BIP_Url
 * object, replacing any previously associated URL string.
 *
 **/
BIP_Status  BIP_Url_SetUrl( BIP_UrlHandle hUrlUrl, const char *urlString );

/**
 * Summary:
 * Get the Scheme from a BIP_Url object
 *
 * Description:
 * Returns a pointer to a null-terminated string that represents the scheme.
 * e.g., for a Request containing a URL "GET http://www.cnn.com/index.html",
 * scheme string of "http", will be returned.
 * Likewise, scheme can also be "https" if SSL/TLS is being used.
 **/
const char * BIP_Url_GetScheme( BIP_UrlHandle hUrl );

/**
 * Summary:
 * Get the Path from a BIP_Url object
 *
 * Description:
 * Returns a pointer to a null-terminated string that represents the path associated with Request URL.
 * e.g., for a Request containing a URL "GET http://www.cnn.com/index.html",
 * path string of "index.html", will be returned.
 **/
const char * BIP_Url_GetPath( BIP_UrlHandle hUrl );

/**
 * Summary:
 * Destroy a BIP_Url object.
 *
 * Description:
 * Deallocates a BIP_Url structure (object) and any other memory that might be
 * associated with it.
 *
 **/
void  BIP_Url_Destroy( BIP_UrlHandle hUrl );

#ifdef __cplusplus
}
#endif

#endif /* !defined BIP_URL_H */

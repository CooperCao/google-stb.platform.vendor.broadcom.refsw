/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

#include <string.h>
#include <ctype.h>

#include "bip_priv.h"

BDBG_MODULE( bip_url );

BDBG_OBJECT_ID( BIP_Url );

/* Define a string descriptor struct to make things easier. */
struct strDescr
{
    const char        * ptr;
    unsigned    len;
};


/*****************************************************************************
 *
 * Summarized ABNF from RFC3986:
 *
 * URI:
 *
 *   URI = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
 *      scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
 *      hier-part = ("//" authority path-abempty / path-absolute / path-rootless / path-empty  <"/" unreserved pct-encoded sub-delims ":" "@" HEXDIG ":" "v" "."  DIGIT pchar>
 *          authority = [ userinfo "@" ] host [ ":" port ]                                      <unreserved pct-encoded sub-delims ":" "@" HEXDIG ":" "v" "."  DIGIT >
 *              userinfo = *( unreserved / pct-encoded / sub-delims / ":" )                     <unreserved pct-encoded sub-delims ":">
 *                  unreserved = ALPHA / DIGIT / "-" / "." / "_" / "~"                          <unreserved>
 *                  pct-encoded = "%" HEXDIG HEXDIG                                             <pct-encoded>
 *                  sub-delims =    "!" / "$" / "&" / "?" / "(" / ")" / "*" / "+" / "," / ";" / "=" <sub-delims>
 *              host = IP-literal / IPv4address / reg-name                                      <HEXDIG ":" "v" "." HEXDIG unreserved sub-delims DIGIT "." pct-encoded>
 *                  IP-literal = "[" ( IPv6address / IPvFuture ) "]"                            <HEXDIG ":" "v" "." HEXDIG unreserved sub-delims>
 *                      IPv6address = 6( h16 ":" ) ls32                                         <HEXDIG ":">
 *                                        / "::" 5( h16 ":" ) ls32
 *                                        / [ h16 ] "::" 4( h16 ":" ) ls32
 *                                        / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
 *                                        / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
 *                                        / [ *3( h16 ":" ) h16 ] "::" h16 ":" ls32
 *                                        / [ *4( h16 ":" ) h16 ] "::" ls32
 *                                        / [ *5( h16 ":" ) h16 ] "::" h16
 *                                        / [ *6( h16 ":" ) h16 ] "::"
 *                          h16 = 1*4HEXDIG                                                     <HEXDIG>
 *                          ls32 = ( h16 ":" h16 ) / IPv4address                                <HEXDIG DIGIT ":" ".">
 *                              IPv4address = dec-octet "." dec-octet "." dec-octet "." dec-octet   <DIGIT ".">
 *                                  dec-octet = DIGIT ; 0-9                                     <DIGIT>
 *                                              / %x31-39 DIGIT ; 10-99
 *                                              / "1" 2DIGIT ; 100-199
 *                                              / "2" %x30-34 DIGIT ; 200-249
 *                                              / "25" %x30-35 ; 250-255
 *                      IPvFuture = "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )        <"v" "." HEXDIG unreserved sub-delims ":">
 *                  reg-name = *( unreserved / pct-encoded / sub-delims )                       <unreserved pct-encoded sub-delims>
 *              port = *DIGIT                                                                   <DIGIT>
 *          path-abempty = *( "/" segment )                                                     <pchar "/">
 *              segment = *pchar                                                                <pchar>
 *                  pchar = unreserved / pct-encoded / sub-delims / ":" / "@"                   <unreserved pct-encoded sub-delims ":" "@">
 *          path-absolute = "/" [ segment-nz *( "/" segment ) ]                                 <pchar "/">
 *              segment-nz = 1*pchar                                                            <pchar>
 *          path-rootless = segment-nz *( "/" segment )                                         <pchar "/">
 *          path-empty = 0<pchar>                                                               <>
 *      query = *( pchar / "/" / "?" )                                                          <pchar "/" "?">
 *      fragment = *( pchar / "/" / "?" )
 *
 *
 *
 * Some common character groups:
 *
 *  unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"                           <unreserved>
 *  pct-encoded = "%" HEXDIG HEXDIG                                               <pct-encoded>
 *  sub-delims  = "!" / "$" / "&" / "?" / "(" / ")" / "*" / "+" / "," / ";" / "=" <sub-delims>
 *  pchar       = unreserved / pct-encoded / sub-delims / ":" / "@"               <unreserved pct-encoded sub-delims ":" "@">
 *
 *
 *
 * Summary of "heir-part" characters:
 *
 *  hier-part = ("//" authority path-abempty /                              <pchar "/">
 *              path-absolute /
 *              path-rootless /
 *              path-empty
 *      authority = [ userinfo "@" ] host [ ":" port ]                      <pchar>
 *          userinfo = *( unreserved / pct-encoded / sub-delims / ":" )     <unreserved pct-encoded sub-delims ":">
 *          host = IP-literal / IPv4address / reg-name                      <unreserved pct-encoded sub-delims ":" >
 *          port = *DIGIT                                                   <DIGIT>
 *      path-abempty                                                        <pchar "/">
 *      path-absolute = "/" [ segment-nz *( "/" segment ) ]                 <pchar "/">
 *      path-abempty = *( "/" segment )                                     <pchar "/">
 *      path-empty = 0<pchar>                                               <>
 *
 *****************************************************************************/



/*****************************************************************************
 *  Free the memory that holds a URL part, then clear the part's pointer.
 *****************************************************************************/
static void
BIP_Url_FreeUrlPart(char ** pPart)
{
    /* If the part pointer is pointing to something, free it. */
    if (*pPart)
    {
        B_Os_Free(*pPart);
        *pPart = NULL;      /* mark pointer as unused. */
    }

    return;
}

/*****************************************************************************
 *  Allocate memory for a URL part, then set it's part pointer to point to it.
 *****************************************************************************/
static BIP_Status
BIP_Url_SetUrlPart(char ** pPart, struct strDescr *pD)
{
    /* If the part pointer is pointing to something, free it. */
    if (*pPart)
    {
        B_Os_Free(*pPart);
        *pPart = NULL;      /* mark pointer as unused. */
    }

    if (pD->len)
    {
        *pPart = B_Os_Calloc(1, pD->len + 1);
        if (*pPart == NULL) return BERR_TRACE(BIP_ERR_OUT_OF_SYSTEM_MEMORY);

        /* Memory is allocated, now copy the URL part into it. */
        memcpy(*pPart, pD->ptr, pD->len);
        *(*pPart + pD->len) = '\0';      /* Null-terminate the string. */
    }
    return (BIP_SUCCESS);
}


/*****************************************************************************
 *  Parsing a URL string into components.  Refer to RFC3986 for URL format
 *  syntax.
 *
 *   URI         = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
 *
 *
 *
 *****************************************************************************/
static BIP_Status
BIP_Url_ParseUrlString( BIP_UrlHandle hUrl )
{
    BIP_Status       errCode = BIP_SUCCESS;

    const char    * pChar;
    const char    * pPort;

    struct strDescr     schemeD    = {NULL, 0};
    struct strDescr     authorityD = {NULL, 0};
    struct strDescr     userinfoD  = {NULL, 0};
    struct strDescr     hostD      = {NULL, 0};
    struct strDescr     portD      = {NULL, 0};
    struct strDescr     pathD      = {NULL, 0};
    struct strDescr     queryD     = {NULL, 0};
    struct strDescr     fragmentD  = {NULL, 0};

    BDBG_OBJECT_ASSERT( hUrl, BIP_Url );
    pChar = hUrl->urlRaw;

    /* URLs start with a "scheme" like this:
     *     scheme      = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
     * */
    schemeD.ptr = pChar;
    if (!isalpha(*pChar)) goto parse_error;   /* First char of scheme is non-alpha... fail. */

    pChar++;
    while (*pChar)
    {
        int c = *pChar;
        if (!isalnum(c) &&
             '+' != c &&
             '-' != c &&
             '.' != c ) break;          /* Found a non-scheme character. */

        pChar++;
    }


    if (*pChar != ':') goto parse_error;      /* Scheme not terminated with colon... fail. */
    schemeD.len = pChar - schemeD.ptr;

    pChar++;    /* Get past the scheme terminator (colon). */


    /*  hier-part   = "//" authority path-abempty /
     *                     path-absolute /
     *                     path-rootless /
     *                     path-empty
     *
     *  authority   = [ userinfo "@" ] host [ ":" port ]
     *  userinfo    = *( unreserved / pct-encoded / sub-delims / ":" )
     *  */

    if (strncmp(pChar, "//",2) == 0)
    {
        /* Handle the "authority path-abempty" case. Get the first field, with might
         * be either the "userinfo" or the "host".  We'll know which when we get to
         * the terminator. */
        pChar += 2;                         /* Get past "//" */
        authorityD.ptr = pChar;
        hostD.ptr      = pChar;            /* Assume that this is the "host".  If not, we'll reset this later. */

        while (*pChar)
        {
            int c = *pChar;

            /* BDBG_LOG(("*pChar=0x%02x (%c)", c, c)); */

            if (*pChar == '@')    /* Terminator for "userinfo" */
            {
                /* BDBG_LOG(("Found @ at %p", pChar)); */
                userinfoD.ptr = authorityD.ptr;
                userinfoD.len   = pChar - userinfoD.ptr;
                hostD.ptr = pChar + 1;
            }

            if (c == '/')   break;          /* Terminator for host if port not specified */
            if (c == '?')   break;          /* Terminator for host if no port or path then query */
            if (c == '#')   break;          /* Terminator for host if no port or path then fragment */

            pChar++;
        }

        /* Now we should be at the end of the "authority" field, but we haven't
         * parsed the port yet). */
        if (hostD.ptr == NULL) goto parse_error;

        authorityD.len  = pChar - authorityD.ptr;
        hostD.len       = pChar - hostD.ptr;

        if (hostD.len == 0) goto parse_error;

#if 0
        BDBG_LOG(("authorityD: ptr=%p len=%u", authorityD.ptr,  authorityD.len));
        if (authorityD.ptr) BDBG_LOG(("authorityD: %.*s", authorityD.len, authorityD.ptr  ));

        BDBG_LOG(("userinfoD:  ptr=%p len=%u", userinfoD.ptr,   userinfoD.len));
        if (userinfoD.ptr) BDBG_LOG(("userinfoD: %.*s", userinfoD.len, userinfoD.ptr  ));

        BDBG_LOG(("hostD:      ptr=%p len=%u", hostD.ptr,       hostD.len));
        if (hostD.ptr) BDBG_LOG(("hostD: %.*s", hostD.len, hostD.ptr  ));
#endif

        /* Now extract the port from the host string (if it's there). */
        for (pPort=hostD.ptr ; pPort<hostD.ptr+hostD.len ; pPort++)
        {
            if (*pPort == ':')
            {
                portD.ptr = pPort + 1;
                if (pPort < hostD.ptr + hostD.len)
                {
                    portD.len = hostD.ptr + hostD.len - portD.ptr;
                }
                hostD.len = pPort - hostD.ptr;
                break;
            }
        }

#if 0
        BDBG_LOG(("hostD:      ptr=%p len=%u", hostD.ptr,       hostD.len));
        if (hostD.ptr) BDBG_LOG(("hostD: %.*s", hostD.len, hostD.ptr  ));

        BDBG_LOG(("portD:      ptr=%p len=%u", portD.ptr,       portD.len));
        if (portD.ptr) BDBG_LOG(("portD: %.*s", portD.len, portD.ptr  ));
#endif

    }

    /* By now, we should be at the start of the "path" (if there is one). */
    if (*pChar != '?' && *pChar != '#')
    {
        pathD.ptr = pChar;

        while (*pChar)
        {
            int c = *pChar;
            if (!isprint(c)) goto parse_error;      /* Invalid non-printable character. */
            if (c == '?')   break;            /* Terminator for path if followed by query */
            if (c == '#')   break;            /* Terminator for path if followed by fragment */

            pChar++;
        }
        pathD.len = pChar - pathD.ptr;
    }

    /* End of the "path", now get the "query" if it exists. */
    if (*pChar == '?')
    {
        pChar++;
        queryD.ptr = pChar;

        while (*pChar)
        {
            int c = *pChar;
            if (!isprint(c)) goto parse_error;      /* Invalid non-printable character. */
            if (c == '#')   break;            /* Terminator for query if followed by fragment */

            pChar++;
        }
        queryD.len = pChar - queryD.ptr;
    }

    /* Done with the "query", now get the "fragment" if it's there. */
    if (*pChar == '#')
    {
        pChar++;
        fragmentD.ptr = pChar;

        while (*pChar)
        {
            int c = *pChar;
            if (!isprint(c)) goto parse_error;       /* Invalid non-printable character. */
            pChar++;
        }
        fragmentD.len = pChar - fragmentD.ptr;
    }

    /* Parsing was successful, now move components into the URL struct. */
    errCode = BIP_Url_SetUrlPart(&hUrl->scheme, &schemeD);
    if (errCode) goto error;

    errCode = BIP_Url_SetUrlPart(&hUrl->userinfo, &userinfoD);
    if (errCode) goto error;

    errCode = BIP_Url_SetUrlPart(&hUrl->host, &hostD);
    if (errCode) goto error;

    errCode = BIP_Url_SetUrlPart(&hUrl->port, &portD);
    if (errCode) goto error;

    errCode = BIP_Url_SetUrlPart(&hUrl->path, &pathD);
    if (errCode) goto error;

    errCode = BIP_Url_SetUrlPart(&hUrl->query, &queryD);
    if (errCode) goto error;

    errCode = BIP_Url_SetUrlPart(&hUrl->fragment, &fragmentD);
    if (errCode) goto error;

    return(BIP_SUCCESS);

parse_error:
#if 0
    BIP_PRINTERR_URL(("Error parsing URL: %s", hUrl->urlRaw));
    BIP_PRINTERR_URL(("Parse failed here: %*s", pChar - hUrl->urlRaw, "^"));
#endif /* 0 */

    BDBG_ERR(("Error parsing URL: %s", hUrl->urlRaw));
    BDBG_ERR(("Parse failed here: %*s", (int)(1 + pChar - hUrl->urlRaw), "^"));

    return(BIP_ERR_INVALID_PARAMETER);

error:
    return BERR_TRACE(errCode);

}

/*****************************************************************************
 * Constructor: Create an instance of a class.
 *****************************************************************************/
BIP_UrlHandle
BIP_Url_Create( const char *urlString )
{
    BIP_Url    *hUrl;
    BIP_Status   errCode;

    hUrl = B_Os_Calloc(1, sizeof(BIP_Url));
    if (NULL == hUrl)
    {
        BERR_TRACE( BIP_ERR_OUT_OF_SYSTEM_MEMORY );
        return(NULL);
    }

    BDBG_OBJECT_SET( hUrl, BIP_Url );

    if (urlString != NULL && strlen(urlString) > 0)
    {
        struct strDescr     urlD;
        urlD.ptr = urlString;
        urlD.len = strlen(urlString);

        errCode = BIP_Url_SetUrlPart(&hUrl->urlRaw, &urlD);
        if (errCode)
        {
            BERR_TRACE(errCode);
            goto error;
        }

        errCode = BIP_Url_ParseUrlString( hUrl );

        if (errCode != BIP_SUCCESS)
        {
            BIP_Url_Destroy(hUrl);
            return(NULL);
        }
    }

    return(hUrl);

error:
    return(NULL);

} /* BIP_Url_Create */

/*****************************************************************************
 *  Destructor: Destroy an instance a class.
 *****************************************************************************/
void
BIP_Url_Destroy( BIP_UrlHandle hUrl )
{
    BDBG_OBJECT_ASSERT( hUrl, BIP_Url );

    /* Free up any fields that have been malloc'd. */
    BIP_Url_FreeUrlPart(&hUrl->urlRaw);
    BIP_Url_FreeUrlPart(&hUrl->scheme);
    BIP_Url_FreeUrlPart(&hUrl->userinfo);
    BIP_Url_FreeUrlPart(&hUrl->host);
    BIP_Url_FreeUrlPart(&hUrl->port);
    BIP_Url_FreeUrlPart(&hUrl->path);
    BIP_Url_FreeUrlPart(&hUrl->query);
    BIP_Url_FreeUrlPart(&hUrl->fragment);

    /* Then finally free the BIP_Url struct. */
    BDBG_OBJECT_DESTROY( hUrl, BIP_Url );
    B_Os_Free( hUrl );
    return;
}



#if 0

/* Combining an relative URL with a Base URL */

if defined(R.scheme) then
    T.scheme    = R.scheme;
    T.authority = R.authority;
    T.path      = remove_dot_segments(R.path);
    T.query     = R.query;
else
    if defined(R.authority) then
        T.authority = R.authority;
        T.path      = remove_dot_segments(R.path);
        T.query     = R.query;
    else
        if (R.path == "") then
            T.path = Base.path;
            if defined(R.query) then
                T.query = R.query;
            else
                T.query = Base.query;
            endif;
        else
            if (R.path starts-with "/") then
                T.path = remove_dot_segments(R.path);
            else
                T.path = merge(Base.path, R.path);
                T.path = remove_dot_segments(T.path);
            endif;
            T.query = R.query;
        endif;
        T.authority = Base.authority;
    endif;
    T.scheme = Base.scheme;
endif;

T.fragment = R.fragment;









Start at beginning.
    Accumulate characters until either of :/?#
    If character is ":"
        Accumulated chars is scheme.
    else
        Start at beginning
    endif

#endif /* 0 */

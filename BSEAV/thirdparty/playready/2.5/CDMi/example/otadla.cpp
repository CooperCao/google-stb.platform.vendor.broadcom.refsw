/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#include <windows.h>
#include <wininet.h>

#include <stdio.h>
#include <stdlib.h>

#include "otadla.h"

#pragma comment( lib, "wininet.lib" )

#pragma warning ( disable: 6031 )

#define MAX_URL_SIZE                    1024
#define MAX_HTTP_HEADER_SIZE            4096
#define MAX_REDIRECTIONS_PER_REQUEST    5
#define MAX_HTTP_SERVER_NAME_LEN        50
#define MAX_HTTP_URL_LEN                100

#define HTTP_SERVER_PORT                80

#define HTTP_HEADER_LICGET "Content-Type: text/xml; charset=utf-8\r\nSOAPAction: \"http://schemas.microsoft.com/DRM/2007/03/protocols/AcquireLicense\"\r\n"

// HTTP status code of temporary redirection.
#define HTTP_STATUS_TEMPORARY_REDIRECT  307

// Parameters of HTTP connection.
typedef struct __tagOTADLA_HttpParams
{
    // Name (or IP address) of the server.
    CHAR m_rgchServerName[ MAX_HTTP_SERVER_NAME_LEN ];

    // TCP port of the server.
    DWORD m_dwHttpPort;

    // URL of the server.
    CHAR m_rgchURL[ MAX_HTTP_URL_LEN ];
} OTADLA_HttpParams;

OTADLA_HttpParams g_oServer = { "", HTTP_SERVER_PORT };

// Send license challenge and receive license response using HTTP.
// This function calls itself recursively if HTTP redirect is encountered.
bool NetClientSendRequest(
    __in HINTERNET f_hSession,
    __in HINTERNET *f_phConnect,
    __in HINTERNET *f_phHttpFile,
    __in_bcount(f_cbChallenge) BYTE *f_pbChallenge,
    __in DWORD f_cbChallenge,
    __in_z CHAR *f_pszHeader,
    __in DWORD f_dwLevel)
{
    bool result = false;
    CHAR rgbQuery[ MAX_URL_SIZE ] = { 0 };
    DWORD cbQuery = 0;
    URL_COMPONENTSA oUrlComponents = { 0 };
    CHAR rgbHostName[ MAX_URL_SIZE ] = { 0 };
    CHAR rgbUrlPath[ MAX_URL_SIZE ] = { 0 };

    if ( f_dwLevel > 0 )
    {
        cbQuery = sizeof( rgbQuery );

        // Retrieve the returned status code.
        if (!HttpQueryInfoA( *f_phHttpFile,
                             HTTP_QUERY_STATUS_CODE,
                             rgbQuery,
                             (LPDWORD)&cbQuery,
                             NULL ))
        {
            goto ErrorExit;
        }

        // Check if the status code is HTTP_STATUS_TEMPORARY_REDIRECT.
        if ( atoi( rgbQuery ) == HTTP_STATUS_TEMPORARY_REDIRECT )
        {
            cbQuery = sizeof( rgbQuery );

            // Retrieve the new location (URL) to be redirected.
            if (!HttpQueryInfoA( *f_phHttpFile,
                                 HTTP_QUERY_LOCATION ,
                                 rgbQuery,
                                 (LPDWORD)&cbQuery,
                                 NULL ))
            {
                goto ErrorExit;
            }

            memset( &oUrlComponents, 0, sizeof( URL_COMPONENTSA ) );

            oUrlComponents.dwStructSize = sizeof( URL_COMPONENTSA );
            oUrlComponents.lpszHostName = rgbHostName;
            oUrlComponents.dwHostNameLength = sizeof( rgbHostName );
            oUrlComponents.lpszUrlPath = rgbUrlPath;
            oUrlComponents.dwUrlPathLength = sizeof( rgbUrlPath );

            // Parse the full redirected URL string into components.
            if (!InternetCrackUrlA( rgbQuery,
                                    0,
                                    0,
                                    &oUrlComponents ))
            {
                goto ErrorExit;
            }
        }
        else
        {
            // The error status code is something else, exit the function.
            goto ErrorExit;
        }
    }
    else
    {
        oUrlComponents.lpszHostName = g_oServer.m_rgchServerName;
        oUrlComponents.nPort = ( INTERNET_PORT )g_oServer.m_dwHttpPort;
        oUrlComponents.lpszUrlPath = g_oServer.m_rgchURL;
    }

    // Close the current connection and file handles if opened.
    if ( *f_phHttpFile != NULL )
    {
        InternetCloseHandle( *f_phHttpFile );
    }

    if ( *f_phConnect != NULL )
    {
        InternetCloseHandle( *f_phConnect );
    }

    *f_phConnect = InternetConnectA( f_hSession,
                                     oUrlComponents.lpszHostName,
                                     oUrlComponents.nPort,
                                     NULL,
                                     NULL,
                                     INTERNET_SERVICE_HTTP,
                                     0,
                                     0 );

    if ( *f_phConnect == NULL )
    {
        goto ErrorExit;
    }

    *f_phHttpFile = HttpOpenRequestA( *f_phConnect,
                                      "POST",
                                      oUrlComponents.lpszUrlPath,
                                      NULL,
                                      NULL,
                                      0,
                                      INTERNET_FLAG_DONT_CACHE,
                                      0 );

    if ( *f_phHttpFile == NULL )
    {
        goto ErrorExit;
    }

    // Add the customized HTTP header to the HTTP request.
    if (!HttpAddRequestHeadersA( *f_phHttpFile,
                                 f_pszHeader,
                                 ( DWORD )strlen( f_pszHeader ),
                                 HTTP_ADDREQ_FLAG_ADD_IF_NEW ))
    {
        goto ErrorExit;
    }

    // Send the HTTP request to the server.
    if ( !HttpSendRequestA( *f_phHttpFile,
                            NULL,
                            0,
                            f_pbChallenge,
                            f_cbChallenge ) )
    {
        // The server returns an error, recursively checks whether
        // redirection is needed.
        if (!NetClientSendRequest( f_hSession,
                                   f_phConnect,
                                   f_phHttpFile,
                                   f_pbChallenge,
                                   f_cbChallenge,
                                   f_pszHeader,
                                   f_dwLevel + 1))
        {
            goto ErrorExit;
        }
    }

    result = true;

ErrorExit:
    return result;
}

// Send license challenge and receive license response using HTTP.
// HTTP redirect is handled internally.
bool NetClient(
    __in_bcount(f_cbChallenge) BYTE *f_pbChallenge,
    __in DWORD f_cbChallenge,
    __deref_out_bcount(*f_pcbResponse) BYTE **f_ppbResponse,
    __out DWORD *f_pcbResponse )
{
    bool result = false;;
    HINTERNET hSession = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hHttpFile = NULL;
    BYTE rgbHeaderBuffer[ MAX_HTTP_HEADER_SIZE ] = { 0 };
    DWORD cbHeaderBuffer = MAX_HTTP_HEADER_SIZE;
    BYTE *pbCurrent = NULL;
    DWORD cbCurrent = 0, cbRead = 0;
    CHAR *pszHeader = NULL;

    pszHeader = HTTP_HEADER_LICGET;

    // Use proxy settings configured in the Internet Explorer.
    hSession = InternetOpenA( "OTADLA",
                              PRE_CONFIG_INTERNET_ACCESS,
                              "",
                              INTERNET_INVALID_PORT_NUMBER,
                              0 );

    if ( hSession == NULL )
    {
        goto ErrorExit;
    }

    // Send the challenge to the server and handle possible
    // HTTP redirection(s).
    if (!NetClientSendRequest( hSession,
                               &hConnect,
                               &hHttpFile,
                               f_pbChallenge,
                               f_cbChallenge,
                               pszHeader,
                               0) )
    {
        goto ErrorExit;
    }

    // Query for the content length from the HTTP header of the
    // server response. Content length indicates the size of
    // HTTP data that follows the HTTP header in the server response.
    if (!HttpQueryInfoA( hHttpFile,
                         HTTP_QUERY_CONTENT_LENGTH,
                         rgbHeaderBuffer,
                         (LPDWORD)&cbHeaderBuffer,
                         NULL ))
    {
        goto ErrorExit;
    }

    // Convert the size of HTTP data into an integer.
    *f_pcbResponse = atol( ( CHAR * )rgbHeaderBuffer );

    *f_ppbResponse = new BYTE[*f_pcbResponse];
    if (*f_ppbResponse == NULL)
    {
        goto ErrorExit;
    }

    memset( *f_ppbResponse, 0, *f_pcbResponse );

    pbCurrent = *f_ppbResponse;
    cbCurrent = *f_pcbResponse;

    // Read the HTTP data of the server response into the
    // buffer just allocated.
    while ( cbCurrent > 0 )
    {
        if (!InternetReadFile( hHttpFile,
                               pbCurrent,
                               cbCurrent,
                               ( LPDWORD )&cbRead ))
        {
            goto ErrorExit;
        }

        pbCurrent += cbRead;

        cbCurrent -= cbRead;
    }

    result = true;

ErrorExit:
    if ( hHttpFile != NULL )
    {
        InternetCloseHandle( hHttpFile );
    }

    if ( hConnect != NULL )
    {
        InternetCloseHandle( hConnect );
    }

    if ( hSession != NULL )
    {
        InternetCloseHandle( hSession );
    }

    return result;
}

// Acquire a license from a license server specified by a URL
// using the supplied license challenge and get back a license
// response.
bool GetLicense(
    __in_z char *f_pszURL,
    __in_bcount(f_cbChallenge) uint8_t *f_pbChallenge,
    __in uint32_t f_cbChallenge,
    __deref_out_bcount(*f_pcbResponse) uint8_t **f_ppbResponse,
    __out uint32_t *f_pcbResponse)
{
    bool result = false;
    URL_COMPONENTSA oUrlComponents = { 0 };

    memset( &oUrlComponents, 0, sizeof( oUrlComponents ) );

    oUrlComponents.dwStructSize = sizeof( URL_COMPONENTSA );
    oUrlComponents.lpszHostName = g_oServer.m_rgchServerName;
    oUrlComponents.dwHostNameLength = sizeof( g_oServer.m_rgchServerName );
    oUrlComponents.lpszUrlPath = g_oServer.m_rgchURL;
    oUrlComponents.dwUrlPathLength = sizeof( g_oServer.m_rgchURL );

    // Parse the URL string into components.
    if (!InternetCrackUrlA( f_pszURL,
                            0,
                            0,
                            &oUrlComponents ))
    {
        goto ErrorExit;
    }

    if (!NetClient(f_pbChallenge,
                   f_cbChallenge,
                   f_ppbResponse,
                   (DWORD *)f_pcbResponse ) )
    {
        goto ErrorExit;
    }

    result = true;

ErrorExit:
    return result;
}

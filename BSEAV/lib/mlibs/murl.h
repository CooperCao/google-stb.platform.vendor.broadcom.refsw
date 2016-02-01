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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/
#ifndef MURL_H
#define MURL_H

#include "mstring.h"

#if defined(_WIN32)
    #include <winsock.h>
#endif

/**
* Url format:
* protocol://server:port/path#fragment
* protocol://server:port/path?search
*
* // is required before server, otherwise it's interpretted as a path
* :port is always optional
* protocol: is always optional
* "file:" protocol never has a server
*/
class MUrl {
public:
    MUrl(const char *url = NULL) {_path=NULL;set(url);}
    MUrl(const MString &url) {_path=NULL;set(url);}
    MUrl(const MUrl &url) {_path=NULL;set(url);}
    ~MUrl() {clear();}

    void set(const char *url);
    void clear();
    bool isNull() const {return _url.isNull();}

    const char *str() const {return _url;}
    const char *protocol() const {return _protocol;}
    const char *server() const {return _host;}
    int port() const {return _port;}

    /**
    * query is anything after host:port. It includes
    * path, search and fragment.
    */
    const char *query() const {return _query;}
    const char *path() const;
    const char *search() const;
    const char *fragment() const;

    /**
    * An url is absolute if it contains a server, or
    * if the query begins with /
    */
    bool isAbsolute();

    MUrl &operator =(const char *url) {set(url);return *this;}
    MUrl &operator +=(const char *url);
    MUrl &operator +=(const MUrl &url);
    operator const char *() const {return str();}
    bool operator !() const {return !isNull();}

    /**
    * Wrapper for getservbyname. The param service is the MUrl.protocol.
    */
    static int lookupDefaultPort(const char *service, const char *protocol = "tcp");

    /**
    * fromUrlText converts url encoding to origianl ascii.
    * + = space, %XX = character, etc.
    **/
    static MString fromEncoding(const char *str);

    /**
    * toUrlText converts unsafe and reserved ascii characters to url encoding format.
    **/
    static MString toEncoding(const char *str);

protected:
    MString _url, _protocol, _host;
    int _port;
    const char *_query; // ptr into _url
    MString *_path; // if NULL, then _path==_query
    int _search; // 0=none,1=search,2=fragment (use _path length as offset into query)
};

inline MUrl operator +(const MUrl &url1, const char *url2) {
    return MUrl(url1) += url2;
}

inline MUrl operator +(const char *url1, const MUrl &url2) {
    return MUrl(url1) += url2;
}

#endif //MURL_H

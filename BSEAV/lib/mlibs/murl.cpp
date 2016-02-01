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
#include "murl.h"
#include <string.h>
#include <stdio.h>
#if defined(__unix__) || defined(__vxworks)
    #include <netdb.h> //getservbyname
#endif

#if defined(__vxworks) // getservbyname not implemented on VxWorks

#include <in.h> //ntohs

struct servent* getservbyname(const char* name, const char* proto)
{
    static servent s;

    /* we can only support the basics here */
    if(!strcmp(proto,"http"))
        s.s_port=80;
    else if(!strcmp(proto,"ftp"))
        s.s_port=21;
    else if(!strcmp(proto,"telnet"))
        s.s_port=23;
    else if(!strcmp(proto,"smtp"))
        s.s_port=25;
    else if(!strcmp(proto,"tftp"))
        s.s_port=69;
    else if(!strcmp(proto,"gopher"))
        s.s_port=70;
    else if(!strcmp(proto,"finger"))
        s.s_port=79;
    else if(!strcmp(proto,"pop3"))
        s.s_port=110;
    else if(!strcmp(proto,"nntp"))
        s.s_port=119;
    else if(!strcmp(proto,"https"))
        s.s_port=443;
    else
        s.s_port=80;    /* default to something potentially useful */

    return(&s);
}

#endif // __vxworks

/**
* Examples of valid urls:
*
* file:filename.txt
* http://server/filename.txt
* http://server:port
* server/filename.txt
* server:port/filename.txt
* server
* filename.txt
*/
void MUrl::set(const char *url) {
    clear();
    _url = url;
    if (!_url)
        return;

    // convert to frontslashes
    _url.replace('\\','/');
    const char *s = _url;

    // file: is the only protocol that doesn't have a server.
    if (!strncmp(s, "file:", 5)) {
        _protocol = "file";
        _query = s+5;
        return;
    }

    char *findhost = (char *)strstr(s, "//");
    if (findhost) {
        if (findhost > s && *(findhost-1)==':') {
            _protocol.strncpy(s, findhost-s-1);
        }
        findhost += 2;
        char *find = strchr(findhost, '/');
        if (find) {
            _host.strncpy(findhost, find-findhost);
            _query = find;
        }
        else {
            _host = findhost;
            _query = NULL;
        }

        int colon = _host.find(':');
        if (colon != -1) {
            _port = atoi(_host.mid(colon+1));
            _host.setLength(colon);
        }
        else
            _port = lookupDefaultPort(_protocol);
    }
    else {
        _port = 0;
        _query = s;
    }

    if (_query) {
        char *search = (char *)strchr(_query, '?');
        if (search) {
            _path = new MString(_query, search-_query);
            _search = 1;
        }
        else {
            search = (char *)strchr(_query, '#');
            if (search) {
                _path = new MString(_query, search-_query);
                _search = 2;
            }
            else {
                _path = NULL;
                _search = 0;
            }
        }
    }
    else {
        _path = NULL;
        _search = 0;
    }
}


void MUrl::clear() {
    _url.clear();
    _protocol.clear();
    _host.clear();
    _query = NULL;
    _port = 0;
    if (_path)
        delete _path;
    _path = NULL;
    _search = 0;
}

const char *MUrl::path() const {
    return _path?(const char *)*_path:_query;
}

const char *MUrl::search() const {
    return _search==1?&_query[_path->length()+1]:NULL;
}

const char *MUrl::fragment() const {
    return _search==2?&_query[_path->length()+1]:NULL;
}

bool MUrl::isAbsolute() {
    return _host || _query && _query[0] == '/';
}

MUrl &MUrl::operator +=(const char *url) {
    return *this += MUrl(url);
}

#include <stdio.h>
MUrl &MUrl::operator +=(const MUrl &url) {
    if (url.server())
        set(url);
    else {
        const char *q = url.query();
        if (q && *q) {
            if (*q == '/') {
                // reassign the query
                if (query())
                    _url.truncate(query()-(const char *)_url);
                _url += q;
            }
            else {
                // append to the query
                // chop off at search or fragment
                const char *chop = search();
                if (!chop)
                    chop = fragment();
                if (chop) {
                    _url.truncate(chop-(const char *)_url);
                }
                if (!query() || _url[_url.length()-1] != '/')
                    _url += '/';
                _url += q;
            }
            set(_url);
        }
    }
    return *this;
}

// from p197, HTML: The Definitive Guide, 3rd ed.
// added space, remove /?:=
static char *g_unsafe = (char *)";@&<>\"#%{}|\\^~[]` ";

MString MUrl::toEncoding(const char *str)
{
    MString result;
    char buf[4];
    while (*str) {
        int n = strcspn(str, g_unsafe);
        if (n) {
            result.strncat(str, n);
            str += n;
            if (!*str)
                break;
        }

        /*sprintf(buf, "%%%02x", str[0]);*/
        snprintf(buf, sizeof(buf), "%%%02x", str[0]);
        result.strncat(buf);
        str++;
    }
    return result;
}

MString MUrl::fromEncoding(const char *str)
{
    MString result;
    char buf[3];
    const char *next;
    buf[2] = 0;
    while ((next = strchr(str, '%')) && next[1] && next[2]) {
        result.strncat(str, next-str);
        next++; // skip %
        memcpy(buf, next, 2);
        int ch;
        /*sscanf(buf, "%x", &ch);*/
        ch = strtoul(buf, NULL, 16);
        result += (char)ch;
        str = next + 2; // skip 2 digits
    }
    result.strncat(str);
    return result;
}

int MUrl::lookupDefaultPort(const char *service, const char *protocol) {
#ifndef NO_SOCKETS
    struct servent *s = getservbyname(service, protocol);
    if (s)
        return ntohs(s->s_port);
    else
#endif
        return 0;
}

#ifdef TEST
#include <stdio.h>
void print(const char *s) {
    printf("\nURL=%s\n", s);
    MUrl u(s);
    printf("  %s,%s,%d,qry=%s,path=%s\n  search=%s,frag=%s\n",
        u.protocol(),
        u.server(),
        u.port(),
        u.query(),
        u.path(),
        u.search(),
        u.fragment());
}

int main() {
/*
    print("file:path/filename.txt"); // proto must == file
    print("proto://server/filename.txt?this=is&a=test");
    print("proto://server/filename.txt#frag");
    print("proto://server:80/filename.txt#frag");
    print("proto://server:80");
    print("//server/filename.txt");
    print("//server:port/filename.txt");
    print("file:path/filename.txt");
    print("proto://server/filename.txt");
    print("//server");
    print("filename");
*/
/*
    print(MUrl("//server") + "path/file");
    print(MUrl("//server/path1") + "path/file");
    print(MUrl("//server/path1") + "/newpath/file");
    print(MUrl("//server/path1") + "http://newserver/newpath/file");
    print(MUrl("//server/path1") + "");
*/
    printf("'%s'\n", (const char *)MUrl::toEncoding("this is a test "));
    printf("'%s'\n", (const char *)MUrl::toEncoding("this is a test"));
    printf("'%s'\n", (const char *)MUrl::fromEncoding("this%20is%20a%20test%20"));
    printf("'%s'\n", (const char *)MUrl::fromEncoding("this%20is%20a%20test"));
}
#endif

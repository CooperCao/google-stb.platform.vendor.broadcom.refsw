/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
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
***************************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "../main/types.h"
#include "www.h"

/* #define DEBUG 1   */

/*----------------------------------------------------------------------*
 * Converts hexadecimal to decimal (character):
 */
static char hexToDec(char *what)
{
   char digit;

   digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
   digit *= 16;
   digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));

   return (digit);
}

/*----------------------------------------------------------------------*
 * Unescapes "%"-escaped characters in a query:
 */
static void unescapeUrl(char *url)
{
   int x, y, len;

   len = strlen(url);
   for (x = 0, y = 0; url[y]; x++, y++) {
      if ((url[x] = url[y]) == '%' && y < (len - 2)) {
	  url[x] = hexToDec(&url[y+1]);
	  y += 2;
      }
   }
   url[x] = '\0';
}

/*----------------------------------------------------------------------*/
void www_UrlDecode(char *s)
{
   char *pstr = s;

   /* convert plus (+) to space (' ') */
   for (pstr = s; pstr != NULL && *pstr != '\0'; pstr++) {
       if (*pstr == '+')
	   *pstr = ' ';
   }
   unescapeUrl(s);
}

/*----------------------------------------------------------------------*/
void www_UrlEncode(const char *s, char *t)
{
  while (*s) {
    if (*s == ' ') {
      *t++ = '+';
    } else if (isalnum(*s)) {
      *t++ = *s;
    } else {
      /* hex it */
      *t++ = '%';
      sprintf(t, "%2x", *s);
      t += 2;
    }
    s++;
  }
  *t = '\0';
}

/*----------------------------------------------------------------------*
 * parse url on form:
 *  "<proto>://<host>[:<port>][<uri>]"
 *  returns
 *    0 if parse ok
 *   -1 if parse failed
 *  port sets to 0 if no port is specified in URL
 *  uri is set to "" if no URI is specified
 */
int www_ParseUrl(const char *url, char *proto, char *host, int *port, char *uri)
{
	int n = 0;
	char *p = NULL;

	if (url == NULL)
	{
		fprintf(stderr, "www_ParseUrl is NULL");
		return -1;
	}
	*port = 0;
	strcpy(uri, "");

	/* proto */
	p = (char *) url;
	if ((p = strchr(url, ':')) == NULL) {
		return -1;
	}
	n = p - url;
	strncpy(proto, url, n);
	proto[n] = '\0';

	if ((strcmp(proto, "http")) &&
		(strcmp(proto, "https")) &&
		(strcmp(proto, "ftp")))
	{
		fprintf(stderr, "unrecognized proto in URL %s", url);
		return -1;
	}

	/* skip "://" */
	if (*p++ != ':') return -1;
	if (*p++ != '/') return -1;
	if (*p++ != '/') return -1;

	/* host */
	{
		char *pHost = host;
		char endChar1 = ':';  /* by default, host field ends if a colon is seen */
		char endChar2 = '/';  /* by default, host field ends if a / is seen */

		/* Check for a "[" which would indicate this is IPv6. */
		if (*p && *p == '[')
		{
			/*
			 * Square brackets are used to surround IPv6 addresses in : notation.
			 * So if a [ is detected, then keep scanning until the end bracket
			 * is seen.
			 */
			endChar1 = ']';
			endChar2 = ']';
			p++;  /* advance past the [ */
		}

		/* Pull out the host */
		while (*p && *p != endChar1 && *p != endChar2)
		{
			*pHost++ = *p++;
		}
		*pHost = '\0';

		/* look for the end of IPv6 */
		if (endChar1 == ']')
		{
			/* if endChar is ], then it must be found */
			if (*p != endChar1)
			{
				return -1;
			}
			else
			{
				p++;  /* advance past the ] */
			}
		}
	}

	if (strlen(host) == 0)
		return -1;

	/* end */
	if (*p == '\0') {
		*port = 0;
		strcpy(uri, "/");
		return 0;
	}

	/* port */
	if (*p == ':') {
		char buf[16];
		char *pp = buf;
		p++;
		while (isdigit(*p)) {
			*pp++ = *p++;
		}
		*pp = '\0';
		if (strlen(buf) == 0)
			return -1;
		*port = atoi(buf);
	}

	/* uri */
	if (*p == '/') {
		char *up = uri;
		while ((*up++ = *p++));
	}

	return 0;
}

/*----------------------------------------------------------------------*
 * returns
 *  0   if ok  (fd contains descriptor for connection)
 *  -1  if socket couldn't be created
 *  -2  if connection function could not be started.
 *
 * sock_fd will hold the socket.
 * The caller of www_Establishconnection must wait until write is possible
 * i.e. setListenerType(sockfd, ..., iListener_Write)
 * this to avoid blocking.
 */
int www_EstablishConnection(tIpAddr host_addr, int port, int *sock_fd)
{
  int fd;
  struct sockaddr_in sa;
  long flags;
  int res;

  memset(&sa, 0, sizeof(sa));
  sa.sin_family       = AF_INET;
  sa.sin_addr.s_addr  = htonl(host_addr);
  sa.sin_port         = htons(port);

  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    return -1;
  }

  /* set non-blocking */
  flags = (long) fcntl(fd, F_GETFL);
  flags |= O_NONBLOCK;
  res = fcntl(fd, F_SETFL, flags);

  errno = 0;
  if (connect(fd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
    if (errno != EINPROGRESS) {
      /* connect failed */
      close(fd);
      return -2;
    }
  }

  *sock_fd = fd;
  printf("www_EstablishConnection: passed\n");
  return 0;
}

/*----------------------------------------------------------------------
 * removes any trailing whitespaces, \r and \n
 * it destroys its argument...
 */
void www_StripTail(char *s)
{
  if (*s != '\0') {
    while(*s) s++;
    s--;
    while(*s == '\r' || *s == '\n' || *s == ' ' || *s == '\t') {
      *s = '\0';
      s--;
    }
  }
}


/*======================================================================*
 * module test code
 *======================================================================*/

#ifdef TEST
int main(int argc, char **argv)
{
  int res;
  char proto[256];
  char host[256];
  int port;
  char uri[256];

  strcpy(proto, "");

  res = www_ParseUrl(argv[1], proto, host, &port, uri);

  printf("result=%d\n", res);
  printf("proto= \"%s\"\n", proto);
  printf("host=  \"%s\"\n", host);
  printf("port=  %d\n", port);
  printf("uri=   \"%s\"\n", uri);
  return 0;
}
#endif

/******************************************************************************
 *	  (c)2010-2013 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * 1.	  This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 * $brcm_Log: $
 * 
 *****************************************************************************/

#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdarg.h>
#include "../main/types.h"

void initLog(int flag);
void slog(int level, const char* fmt, ...);

void vlog(int level, const char* fmt, va_list ap);
int dns_lookup(const char *name, tIpAddr *res);

extern const u_char zeroMac[6];

/* Files */
int  hasChanged(const char* new, const char* old);
int  mkdirs(const char *path);
void rmrf(const char* path);

/* Time */
time_t getCurrentTime(void);
int  cmpTime(struct timeval* t1, struct timeval* t2);
void addMs(struct timeval* t1, struct timeval* t2, int ms);
void subTime(struct timeval* t1, struct timeval* t2);
char *getXSIdateTime(time_t *tp);

/* hex */
extern const char *util_StringToHex(const char *s);

/* Addresses */
void readMac(u_char* mac, const char* val);
int  readIp(const char* ip);
int  readProto(const char* val);
int readMask(const char *mask);

char* writeMac(const u_char* mac);
char* writeCanonicalMac(const u_char* mac);
char* writeQMac(const u_char* mac);
void  writeIp_b(int ip, char *buf);
char* writeIp(int ip);
char* writeNet(int ip, int bits);
char* writeBcast(int ip, int bits);
char* writeMask(int bits);
char* writeRevNet(int ip, int bits);
char* writeRevHost(int ip, int bits);
char* writeProto(int proto);

/* Text handling and formatting */
void  readHash(u_char* hash, const char* val);
char* writeQHash(const u_char* mac);
char* unquoteText(const char* text);
char* quoteText(const char* text);
int streq(const char *s0, const char *s1);
int stricmp( const char *s1, const char *s2 );
const char *itoa(int i);
int testBoolean(const char *s);

typedef struct WanState {
  u_char mac[6];
  char *name;
  tIpAddr ip;
  tIpAddr mask;
  tIpAddr gw;
} WanState;

int getWanInfo(WanState *wanState);

typedef enum {
    eNone,
    eDigest,
    eBasic
} eAuthentication;
typedef enum {
	eNoQop,
	eAuth,
	eAuthInt
} eQop;

/* Used for both server/client */
typedef struct SessionAuth  {
	eQop	qopType;
	int		nonceCnt;
	char 	*nc;		/* str of nonceCnt */
    char    *nonce;
    char    *orignonce;
    char    *realm;
    char    *domain;
    char    *method;
    char    *cnonce;
	char 	*opaque;
	char	*qop;
    char    *user;
    char    *uri;
	char	*algorithm;
    char    *response;
    char    *basic;
    unsigned    char    requestDigest[33];
} SessionAuth;

char *generateWWWAuthenticateHdr(SessionAuth *sa, char *realm, char *domain, char* method);

int parseAuthorizationHdr(char *ahdr, SessionAuth *sa, char *username, char *password);

char *generateAuthorizationHdrValue( SessionAuth *sa, char *wwwAuth, char *method,
									  char *uri, char *user, char *pwd);
char *generateNextAuthorizationHdrValue(SessionAuth *, char *user, char *pwd );
eAuthentication parseWWWAuthenticate(char *ahdr, SessionAuth *sa);

#endif /* __UTILS_H__ */


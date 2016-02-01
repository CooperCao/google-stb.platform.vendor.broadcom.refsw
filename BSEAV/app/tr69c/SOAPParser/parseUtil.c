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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <fcntl.h>

#include "../nanoxml/nanoxml.h"
#include "parseUtil.h"
//#include "xmlParserSM.h"
#include "xmlTables.h"

extern int verbose;
extern XmlNodeDesc envelopeDesc[];
#define NO_ERROR    0
#define PARSE_ERROR 1

/*----------------------------------------------------------------------*/
static void parse_error(char *errfmt, ...) {
    va_list ap;

    va_start(ap, errfmt);
    vlog(LOG_ERR, errfmt, ap);
    va_end(ap);
    return;
}

/*******************************************************************/
/*----------------------------------------------------------------------*
 * parse from file or in-memory data
 *   parse_generic("/apa", NULL, 0, ...)   parses content in file /apa
 *   parse_generic(NULL, ptr, size, ...)      parses content pointed to by ptr
 *   parse_generic("/apa", ptr, size, ...)    error, illegal usage return NULL
 */
ParseResult *parse_generic(char *path, char *memory, int size, XmlNodeDesc *startNode, NameSpace *nspace)
{
    char buf[BUFSIZE];
    int done;
    int file;
    nxml_settings settings;
    nxml_t parser;
    char *xmlEnd;
    int error = NO_ERROR;

    if (path != NULL && memory != NULL) {
        slog(LOG_ERR, "parser", "internal error: parse_generic() can not parse both from file and memory\n");
        return NULL;
    }
    settings.tag_begin = xmlTagBegin;
    settings.tag_end = xmlTagEnd;
    settings.attribute_begin = xmlAttr;
    settings.attribute_value = xmlValue;
    settings.data = xmlData;

    if (nxml_open(&parser,&settings)) {
        parser->node = parser->nodestack[0] = startNode;
        parser->nameSpaces = nspace;        /* nameSpace table to use */
        parser->parse_error = parse_error;  /* set error handler */
        if (path != NULL) {
            if ( (file = open(path, O_RDONLY, 0 ))== -1){
                slog(LOG_ERR, "Parser:Could not open file %s", path);
                return NULL;
            }
        }
        do {
            if (path != NULL) {
                /* from file */
                size_t len = read(file, buf, sizeof(buf));
                done = len < sizeof(buf);
                if ( nxml_write(parser, buf, len, &xmlEnd)<1) {
                    slog(LOG_ERR, "Parser:invalid xml config in file %s",
                        path);
                    error = PARSE_ERROR;
                }
            } else {
                /* from memory */
                done = 1;
                if ( nxml_write(parser, memory, size, &xmlEnd)<1) {
                    slog(LOG_ERR, "parser", "invalid xml config in memory");
                    /* need line number of error here */
                    error = PARSE_ERROR;
                }
            }
        } while (error == NO_ERROR && !done);
    }
    if (path != NULL) {
        close(file);
    }
    nxml_close(parser);

    switch (start) {
    case S_SCANFILE:
        break;
    default:
        break;
    }
    return NULL;
}


/*----------------------------------------------------------------------*/
void slog(int level, const char* fmt, ...)
{
    va_list ap;
    va_start(ap,fmt);
    if (verbose) {
        vfprintf(stderr, fmt, ap);
    }
    vsyslog(level, fmt, ap);
    va_end(ap);
}

void vlog(int level, const char* fmt, va_list ap)
{
    if (verbose) {
        vfprintf(stderr, fmt, ap);
    }
    vsyslog(level, fmt, ap);
}

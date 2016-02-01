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
#include <fcntl.h>
#include <syslog.h>
#include <unistd.h>

#include "../nanoxml/nanoxml.h"
#include "../main/utils.h"
#include "xmlParserSM.h"

#define WHITESPACE  "\t\n\r "

/*----------------------------------------------------------------------*/
static void parse_error(char *errfmt, ...) {
    va_list ap;

    va_start(ap, errfmt);
    vlog(LOG_ERR, errfmt, ap);
    va_end(ap);
    return;
}

/* trim white space from beginning and end of buffer b */
/* returns pointer to first non-ws char and inserts \0 */
/* after last non-ws char in buffer. Input buffer is */
/* assumed to be null terminated */
static char *trimws(char *b, int lth)
{
    char *s, *e;
    int start;
    s = b+ (start = strspn(b,WHITESPACE));
    for ( e=b+lth-1; e>s; --e)
        if ( *e==' ' || *e=='\n' || *e=='\t' || *e=='\r')
            *e = '\0';
        else
            break;
    return s;
}

static int xmlnsPrefix(char *name)
{   char *s;
    if ( (s=strchr(name, ':')) )
        return !strncmp(name,"xmlns:",s-name+1);
    return 0;
}

/*
* searches the nameSpace table to see if we understand
* the urlvalue. If found then set the receive prefix
* and return 1;
* otherwise return 0
 * Assumes that the p->attrName is of the form
 * xmlns:prefix.
* In future we may want to add the unknow namespaces.
*/
static int tr69XMLaddNameSpace(nxml_t p, char *urlvalue)
{
    char *s;
    NameSpace *nsp;

    s = strchr(p->attrName, ':');
    if (s) {
        /* look for urlstring in namespaces table*/
        nsp = p->nameSpaces;
        while( nsp->nsURL ){
            if (!strcmp(nsp->nsURL, urlvalue)) {
                /* found namespace url */
                if (nsp->rcvPrefix)
                    free(nsp->rcvPrefix);
                nsp->rcvPrefix = strdup(s+1);
                return 1;
            }
            ++nsp;
        }
    }
    return 0;
}

/* name compare with prefix:xxx form checking */
/* This needs more thought--- The namespaces should */
/* be unchanging but this may not work with an */
/* ACS that is bent on messing up the CPE clients */
static int  nameCmp(char *name, XmlNodeDesc *node)
{
    char    *s;
    s = strchr(name,':');
    if (s )
    {
    	/* prefix is present*/
        if ( node->nameSpace
            && node->nameSpace!=iDEFAULT
            && node->nameSpace->rcvPrefix
            && strncmp(name, node->nameSpace->rcvPrefix, s-name+1))
    	{
            /*namespaces match */
#if 0
			printf("nameCmp1: name = %s tagname = %s\n", s+1, node->tagName);
#endif
            return strcmp(s+1,node->tagName);
        }
        else if (node->nameSpace==iDEFAULT)
        {
            /* a universal prefix value in node */
#if 0
			printf("nameCmp2: name = %s tagname = %s\n", s+1, node->tagName);
#endif
            return strcmp(s+1,node->tagName);
        }
        else if (node->nameSpace==iNULL)
        {
#if 0
			printf("nameCmp3: name = %s tagname = %s\n", s+1, node->tagName);
#endif
            return strcmp(s+1,node->tagName);
        }
#if 0
		else
		{
#if 0
			printf("nameCmp4: node sendPrefix = %s\n", node->nameSpace->sndPrefix);
			printf("nameCmp4: name = %s tagname = %s\n", s+1, node->tagName);
#endif
            return strcmp(s+1,node->tagName);
		}
#endif
    }
    else
    {
        /* no prefix */
        /* if(node->nameSpace==iNULL) */
#if 0
		printf("nameCmp5: name = %s tagname = %s\n", name, node->tagName);
#endif
        return strcmp(name,node->tagName);
    }
    return -1; /* no match*/
}

static void xmlTagBegin( nxml_t p, const char *tagName, unsigned lth)
{
    XmlNodeDesc *item;
    char tag[NXML_MAX_NAME_SIZE];

    strncpy(tag, tagName,lth);
    tag[lth]='\0';
    DEBUGP(("xmlTagBegin tag=%s lth=%d xmllevel=%d\n", tag,lth, p->level));
    if ( (p->nodeFlags & XML_SCAN_F) && p->scanSink)
        /* Scan mode is set write tag to data sink */
        p->scanSink(TAGBEGIN, tag );
    if ( (item = p->node))
    {
        while (item != NULL && item->tagName)
        {
            if (nameCmp(tag, item)==0)
            {
                /* found node entry */
				printf("=============================\n");
				printf("xmlTagBegin: found tag = %s\n", tag);
				printf("=============================\n");
                if ( item->setXmlFunc )  /* null callbacks are ok */
                    if (item->setXmlFunc(tag, TAGBEGIN, NULL) != XML_STS_OK)
                        p->parse_error("Parse error at begin Tag = %s ", tag);
                p->level++;
                p->nodestack[p->level] = p->node;
                p->itemstack[p->level] = item;
                p->node = item->leafNode;
                return;
            } else
                item++;
        }
    }
    /*if (p->node) */ /* only put out not found msg on high level tag */
        DEBUGP(("xmlTagBegin tag=%s not found\n", tag));
    p->level++;
    p->nodestack[p->level] = p->node;  /* this is so the parserSM will walk thru the tree */
    p->itemstack[p->level] = NULL;  /* and ignore unknown tags if the structure is correct*/
    p->node = NULL;
    return;
}
/*
* xmlTagEnd:
*     tagname lth may be 0 if the form is <tag attr=name />
*/
static void xmlTagEnd ( nxml_t p, const char *tagName, unsigned lth)
{
    XmlNodeDesc *item;
    char tag[NXML_MAX_NAME_SIZE];

    if (lth > 0)
        strncpy(tag, tagName,lth);
    tag[lth]='\0';
    /* clean up old accumlated value in case of error exit */
    free(p->valueptr);
    free(p->dataptr);
    p->valueptr = p->dataptr = NULL;
    p->valuelth = p->datalth = 0;

    if(p->level<=0){
        p->parse_error("Bad xml tree fromat tag=%s", tag);
        return;
    }
    p->node = p->nodestack[p->level];
    item = p->itemstack[p->level];
    p->level--;
    if ( item!=NULL)
    {
        if (lth>0)
        {
            if (nameCmp(tag, item)==0)
            {
                /* found node item entry */
                if (item->setXmlFunc != NULL)
                    if ( item->setXmlFunc(tag,TAGEND,NULL)!=XML_STS_OK)
                        /* error */
                        p->parse_error("Configuration function error at tag=%s ", item->tagName);
                DEBUGP(("xmlTagEnd tag=%s found at level %d\n", tag, p->level));
                return;
            }
            DEBUGP(("xmlTagEnd tag=%s internal nodepointer error level=%d\n", tag, p->level));
            return;
        }
        else
        {
            DEBUGP(("xmlTagEnd shortform tag=%s xmllevel=%d\n", item->tagName, p->level));
            if (item->setXmlFunc!=NULL)
                if ( item->setXmlFunc(tag,TAGEND,NULL)!=XML_STS_OK)
                    p->parse_error("Configuration function error at tag=%s ", item->tagName);
            return;
        }
    }

    if (p->nodeFlags&XML_SCAN_F && p->scanSink) {
        /* Scan mode is set write tag to data sink */
        p->scanSink(TAGEND, tag);
    }
    /* this is an error */
    DEBUGP(("xmlTagEnd tag=%s xml node not found at level %d\n", tag, p->level));
    return;
}

static void xmlAttr( nxml_t p, const char *attrName, unsigned lth)
{
    XmlNodeDesc *item;
    char attr[NXML_MAX_NAME_SIZE];

    strncpy(attr, attrName,lth);
    attr[lth]='\0';

    if (p->nodeFlags & XML_SCAN_F && p->scanSink) {
        /* Scan mode is set write tag to data sink */
        p->scanSink(ATTRIBUTE, attr );
    }
    if (xmlnsPrefix(attr)) {
        /* save name and prefix for xmlValue callback*/
        strcpy(p->attrName, attr);
        return;
    }
    if ((item = p->node)) {
        if (p->level>0) {
            while (item->tagName) {
                if (nameCmp(attr, item)==0) {
                    /* found node entry */
                    strcpy(p->attrName, attr);
                    DEBUGP(("xmlAttr attr=%s found at level %d\n", attr, p->level));
                    return;
                } else
                    item++;
            }
        }
    }
    if (p->node)   /* surpress error on attributes of unknown tags */
        p->parse_error("Unknow attribute %s\n", attr);
    DEBUGP(("xmlAttr attr=%s not found at level %d\n", attr, p->level));
    return;
}

static void xmlValue( nxml_t p, const char *attrValue, unsigned lth, int more)
{
    XmlNodeDesc *item;
    char *value;
    if ((value = realloc(p->valueptr, p->valuelth+lth+1))==NULL){
    	free(p->valueptr);
    	p->parse_error("Parser value allocation buffer failure at %d", p->level);
    	p->valueptr = NULL;
        return;
	}
    p->valueptr = value;
    memcpy(value+p->valuelth, attrValue, lth);
    p->valuelth += lth;
    if (more)
    	return;
    p->valueptr[p->valuelth]='\0';
    if (p->nodeFlags & XML_SCAN_F && p->scanSink) {
        /* Scan mode is set write tag to data sink */
        p->scanSink(ATTRIBUTEVALUE, value );
    }
    /* first check if attrname has an xmlns: prefix */
    if ( xmlnsPrefix(p->attrName)) {
        /* its a name space declaration */
        tr69XMLaddNameSpace(p, value);
        free(p->valueptr);
        p->valuelth =0; p->valueptr=NULL;
        return;
    }
    /* now lookup saved attrName in xml tables */
    if ( (item = p->node)) {
        if (p->level>0) {
            while (item->tagName) {
                if (nameCmp(p->attrName, item)==0) {
                    /* found node entry */
                    if (item->setXmlFunc!=NULL)
                        if ( item->setXmlFunc(p->attrName,ATTRIBUTEVALUE, value)!=XML_STS_OK)
                            p->parse_error("Attribute value error for %s=\"%s\"", item->tagName, value);
                    DEBUGP(("xmlValue attr=%s value=%s found at level %d\n", item->tagName,
                                        value, p->level));
        		    free(p->valueptr);
        		    p->valueptr = NULL;
        		    p->valuelth =0;
                    return;
                } else
                    item++;
            }
        }
    }
    free(p->valueptr);
    p->valueptr= NULL; p->valuelth=0;
    if(p->node)
        p->parse_error("Unknown attribute for value=%s at level %d", value, p->level);
    DEBUGP(("xmlValue attr=%s not found at level %d\n", p->attrName, p->level));
    return;
}

/* Callback from scanner for data between tags or attributes */
/* more flag indicates that a token terminated data, 0 more data value */
static void xmlData( nxml_t p, const char *data, unsigned lth, int more)
{
    XmlNodeDesc *item;
    char *dp;

    if ((dp = realloc(p->dataptr, p->datalth+lth+1))==NULL){
    	free(p->dataptr);
    	p->parse_error("Parser data allocation buffer failure at %d", p->level);
    	p->dataptr = NULL;
    	return;
	}
    p->dataptr = dp;
    memcpy(dp+p->datalth, data, lth);
    p->datalth += lth;
    if (more)
        return;
    p->dataptr[p->datalth]='\0';
    /* if using scansink -- needs to calback to it here */
    /* otherwise just call xmlSetFunc with TAGDATA */
    if ( (item = p->itemstack[p->level]))
    {
        if (item->setXmlFunc!=NULL)
        {
			printf("xmlData: 2\n");
            dp = trimws(dp,p->datalth);
            if ( item->setXmlFunc(item->tagName,TAGDATA, dp)!=XML_STS_OK)
                p->parse_error("Tag data error for %s=\"%s\"", item->tagName, dp);
        }
        DEBUGP(("xmlDATA tag=%s data=%s found at level %d\n", item->tagName,
                            dp, p->level));
        free(p->dataptr);
        p->dataptr = NULL;
        p->datalth =0;
        return;
    }
    free(p->dataptr);
    p->dataptr= NULL; p->datalth=0;
    if(p->node)
        p->parse_error("Unknown data=%s at level %d", dp, p->level);

    DEBUGP(("xmlDATA at level %d\n",p->level));
    return;
}

/*******************************************************************/
/*----------------------------------------------------------------------*
 * parse from file or in-memory data
 *   parse_generic("/apa", NULL, 0, ...)   parses content in file /apa
 *   parse_generic(NULL, ptr, size, ...)      parses content pointed to by ptr
 *   parse_generic("/apa", ptr, size, ...)    error, illegal usage return NULL
 */
eParseStatus parseGeneric(char *path, char *memory, int size, ParseHow *parseHow )
{
    char *buf=NULL;
    int done;
    int file=0;
    nxml_settings settings;
    nxml_t parser;
    char *xmlEnd;
    eParseStatus error = NO_ERROR;

printf("parseGeneric:...\n\n");
    if (path != NULL && memory != NULL) {
        slog(LOG_ERR, "parser: %s", "internal error: parse_generic() can not parse both from file and memory\n");
        return PARSE_ERROR;
    }
    settings.tag_begin = xmlTagBegin;
    settings.tag_end = xmlTagEnd;
    settings.attribute_begin = xmlAttr;
    settings.attribute_value = xmlValue;
    settings.data = xmlData;

    if ( xmlOpen(&parser,&settings))
    {
        parser->node = parser->nodestack[0] = parseHow->topLevel;
        parser->nameSpaces = parseHow->nameSpace;        /* nameSpace table to use */
        parser->parse_error = parse_error;  /* set error handler */
        if (path != NULL)
        {
            buf = (char *)malloc(BUFSIZE);
            if ( (file = open(path, O_RDONLY, 0 ))== -1)
            {
                slog(LOG_ERR, "Parser:Could not open file %s", path);
                return PARSE_ERROR;
            }
        }
        do
        {
            if (path != NULL) {
                /* from file */
                size_t len = read(file, buf, BUFSIZE);
                done = len < BUFSIZE;
                if ( xmlWrite(parser, buf, len, &xmlEnd)<1) {
                    slog(LOG_ERR, "Parser:invalid xml config in file %s",
                        path);
                    error = PARSE_ERROR;
                }
            } else {
                /* from memory */
                done = 1;
                if ( xmlWrite(parser, memory, size, &xmlEnd)<1)
                {
                    slog(LOG_ERR, "parser %s", "invalid xml config in memory");
                    /* need line number of error here */
                    error = PARSE_ERROR;
                }
            }
        } while (error == NO_ERROR && !done);
    }
    if (path != NULL)
    {
        free(buf);
        close(file);
    }
    xmlClose(parser);

    return error;
}

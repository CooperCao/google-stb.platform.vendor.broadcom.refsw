/***************************************************************************
 *     (c)2012-2014 Broadcom Corporation
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
#include "mxmlparser.h"

MXmlElement *MXmlParser::parse(const char *str) {
    MXmlElement *root = new MXmlElement(NULL, "ROOT");
    while ((str = findTag(root, str)));
    if (root->totalChildren())
        return root;
    else {
        delete root;
        return NULL;
    }
}

#define WHITESPACE " \t\r\n"
#define C(x) if (!x) return NULL

char *MXmlParser::findTag(MXmlElement *parent, const char *str) {
    // find the next tag
    char *tag = (char *)strchr(str, '<');
    C(tag);
    tag++;
    // check if this is a close tag
    if (*tag == '/')
        return tag;

    // parse the tag name and create a new element
    char *endTag = strpbrk(tag, WHITESPACE "/>");
    C(endTag);
    bool questionmark = false;
    if (*tag == '?') {
        tag++;
        questionmark = true;
    }

    if ((*tag == '!') && (*(tag+1) == '-') && (*(tag+2) == '-')) {
        str = strstr(str, "-->");
        if (str)
        {
            str+=3;
        }
        return (char *)str;
    }

    MXmlElement *element = new MXmlElement(parent, tag, endTag-tag);
    //printf("Adding new element, tag:%s (%p)  parent: %s (%p)\n", (const char *)element->tag(), element, parent->tag().s(), parent);

    str = strskip(endTag, WHITESPACE);

    // find attributes
    str = findAttrs(element, str);
    C(str);


    //printf("Check(2) for self-closing TAG %s (%p) str:\"%.40s\"\n", element->tag().s(), element, str);

    str = strskip(str, WHITESPACE);
    if (*str == '/') {
        //printf("Found self-closing TAG %s (%p) str:\"%.40s\"\n", element->tag().s(), element, str);
        // self-closing tag, so return ptr after the >
        str = strchr(str, '>');
        C(str);
        return (char*)str+1;
    }

    str = strchr(str, '>');
    C(str);
    str++;
    // str points to the char after the >

    // <? ...> is self closing
    if (questionmark)
        return (char *)str;

    //printf("Looking for children/data for %s (%p) str:\"%.40s\"\n", element->tag().s(), element, str);
    // find children and data
    do {
        str = strskip(str, WHITESPACE);
        if (!*str)
            return NULL;
        else if (*str == '<') {
            // child tag
            str = findTag(element, str);
            //printf("Found child TAG for %s (%p) str:\"%.40s\"\n", element->tag().s(), element, str);
        }
        else {
            // data
            char *endData = (char *)strchr(str, '<');
            MXmlElement *data = new MXmlElement(element, NULL);
            if (endData)
                data->setData(str, endData-str, 1);
            else
                data->setData(str, -1, 1);
            str = endData;
        }
        C(str);
    } while (*str != '/');
    //printf("Done with children/data for %s (%p) str:\"%.40s\"\n", element->tag().s(), element, str);

    // we've hit an unresolved closetag, which is either us,
    // or it's above us. in any case, we're leaving.
    str++;
    char *checkCloseTag = (char *)strpbrk(str, WHITESPACE ">");
    C(checkCloseTag);
    if (!element->tag().strncmp(str, checkCloseTag-str)) {
        checkCloseTag = strchr(checkCloseTag, '>');
        C(checkCloseTag);
        // it's us
        //printf("  found close\n");
        return checkCloseTag+1;
    }
    else
        // must be above us
        //printf("  unresolved close\n");
        return (char *)str-1;
}

char *MXmlParser::findAttrs(MXmlElement *parent, const char *str) {
    // assume we're in the tag, not on whitespace
    while (*str != '>' && *str != '?' && *str != '/') {
        // parse the attr name
        char *name = (char *)str;
        char *endName = strpbrk(name, WHITESPACE "/?=>");
        C(endName);
        MXmlAttribute *attr = new MXmlAttribute(name, endName-name);
        parent->add(attr);

        // search for the value
        char *value = strskip(endName, WHITESPACE);
        if (*value == '=') {
            value++;
            value = strskip(value, WHITESPACE);
            bool usequote = false;
            if (*value == '"') {
                usequote = true;
                value++;
            }
            char *endValue;
            if (usequote) {
                str = value;
                do {
                    endValue = (char *)strchr(str, '"');
                    C(endValue);
                    str = endValue+2;
                } while (*endValue == '"' && *(endValue+1) == '"');
            }
            else {
                endValue = (char *)strpbrk(str, WHITESPACE "/?>");
                C(endValue);
            }

            attr->setValue(value, endValue-value);
            if (usequote) {
                endValue++;
            }
            if (*endValue != '>' && *endValue != '?' && *endValue != '/') {
                endValue = strskip(endValue, WHITESPACE);
            }
            str = endValue;
        }
        else
            str = value;
    }
    return (char *)str;
}

//#define HAS_MAIN
#ifdef HAS_MAIN
int main() {
#define MAXBUF 4096
    // simple stdin parser for easy tests. build it like this:
    // c++ -o xml i386/libmnet.a i386/libmcommon.a

    char buf[MAXBUF];
    int len = fread(buf, 1, MAXBUF-1, stdin);
    buf[len] = 0;
    MXmlParser parser;
    MXmlElement *e = parser.parse(buf);
    if (e) {
        e->print();
        return 0;
    }
    else
        return 1;
}
#endif

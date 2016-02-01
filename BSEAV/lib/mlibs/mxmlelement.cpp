/***************************************************************************
 *     (c)2014 Broadcom Corporation
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
#include "mxmlelement.h"

static MString stub;

MXmlElement::MXmlElement(MXmlElement *parent, const char *tag, int len) :
    _tag(tag, len)
{
    if (parent)
        parent->add(this);
    else
        _parent = NULL;
}

MXmlAttribute *MXmlElement::findAttr(const char *name) {
    bool hasNamespace = strchr(name, ':');
    for (MXmlAttribute *a = _attrs.first(); a; a = _attrs.next()) {
        // disregard namespace if search name doesn't have namespace
        const char *aname;
        if (hasNamespace)
            aname = a->name();
        else {
            aname = strchr(a->name(), ':');
            if (aname)
                aname++;
            else
                aname = a->name();
        }

/* BUG: if (aname == name)*/
        if (!strncmp(aname, name, 512))
            return a;
    }
    return NULL;
}

const char *MXmlElement::attrValue(const char *name, const char *defaultValue) {
    MXmlAttribute *a = findAttr(name);
    if (a)
        return a->value();
    else
        return defaultValue;
}

int MXmlElement::attrInt(const char *name, int defaultValue) {
    MXmlAttribute *a = findAttr(name);
    if (a)
        return a->value().toInt();
    else
        return defaultValue;
}

#define printspaces(f, indent) for (int i=0;i<indent;i++) fputc(' ', f)

void MXmlElement::print(FILE *f, int indent) {
    printspaces(f, indent);
    if (tag().isNull()) {
        fprintf(f, "%s\n", (const char *)data());
    }
    else {
        fprintf(f, "<%s", (const char *)tag());
        for (MXmlAttribute *a = firstAttr(); a; a = nextAttr())
            fprintf(f, " %s=\"%s\"", (const char *)a->name(), (const char *)a->value());

        if (totalChildren() == 0) {
            fprintf(f, "></%s>\n", (const char *)tag());
        }
        else {
            fprintf(f, ">");
            // single data children don't need the newlines
            if (totalChildren() == 1 && firstChild()->tag().isNull()) {
                fprintf(f, "%s", (const char *)childData());
            }
            else {
                fprintf(f, "\n");
                for (MXmlElement *c = firstChild(); c; c = nextChild())
                    c->print(f, indent+2);
                printspaces(f, indent);
            }
            fprintf(f, "</%s>\n", (const char *)tag());
        }
    }
}

int MXmlElement::print(char *buf, int buflen) {
    if (tag().isNull()) {
        if (buflen > data().length())
            buflen = data().length();
        strncpy(buf, data(), buflen);
        return buflen;
    }
    else {
        int written = 0;
        int rc = snprintf(buf, buflen, "<%s", (const char *)tag());
        if (rc == -1) return buflen;
        written += rc;
        for (MXmlAttribute *a = firstAttr(); a; a = nextAttr()) {
            rc = snprintf(&buf[written], buflen-written, " %s=\"%s\"", (const char *)a->name(), (const char *)a->value());
            if (rc == -1) return buflen;
            written += rc;
        }
        rc = snprintf(&buf[written], buflen-written, ">");
        if (rc == -1) return buflen;
        written += rc;

        for (MXmlElement *c = firstChild(); c; c = nextChild()) {
            rc = c->print(&buf[written], buflen-written);
            if (rc == -1) return buflen;
            written += rc;
        }
        rc = snprintf(&buf[written], buflen-written, "</%s>", (const char *)tag());
        if (rc == -1) return buflen;
        written += rc;

        return written;
    }
}

MXmlElement *MXmlElement::createChild(const char *name, const char *data, int processSpecialChars) {
    MXmlElement *x = new MXmlElement(this, name);
    if (data)
        x->createData(data, processSpecialChars);
    return x;
}

MXmlElement *MXmlElement::createData(const char *data, int processSpecialChars) {
    MXmlElement *e = new MXmlElement(this);
    e->setData(data, -1, processSpecialChars);
    return e;
}

void MXmlElement::detachFromParent() {
    if (parent())
        parent()->removeChild(this);
}

bool MXmlElement::removeChild(MXmlElement *child) {
    if (_children.remove(child)) {
        child->_parent = NULL;
        return true;
    }
    else
        return false;
}

MXmlElement *MXmlElement::findChild(const char *tag, int len) {
    return findChild(tag, NULL, len);
}

MXmlElement *MXmlElement::findChild(const char *tag, MXmlElement * startChild, int len) {
    if (!startChild)
    {
        startChild = firstChild();
    }
    else
    {
        _children.index(startChild); // Move "current" position to startChild.
    }

    for (MXmlElement *c = startChild; c; c = nextChild()) {
        if (!c->tag().strncmp(tag, len))
            return c;
    }
    return NULL;
}

MString &MXmlElement::childData() {
    MXmlElement *child = firstChild();
    return child?child->data():stub;
}

struct SpecialChar {
    char *name;
    char ch;
};

static SpecialChar xmlSpecialChars[] = {
    { (char *) "amp", '&' },
    { (char *)"gt", '>' },
    { (char *)"lt", '<' },
    { (char *)"quot", '"' },
    { (char *)"nbsp", '&' },
    // there are a ton more. see appendix E in "HTML: The Definitive Guide"
    {0, 0}};

void MXmlElement::setData(const char *data, int len, int processSpecialChars) {
    if (processSpecialChars == 1 && data) {
        //printf("started with %s\n", MString(data, len).s());
        // get _data ready to receive about len characters
        _data.clear();
        if (len == -1)
            len = strlen(data);
        _data.allocate(len);

        // parse data
        while (1) {
            char *find = (char *)strchr(data, '&');
            if (find && find-data < len) {
                // take everything up to the &
                _data.strncat(data, find-data);
                char *tail = strchr(find, ';');
                if (tail && tail-data < len) {
                    find++;
                    // we've got a valid code
                    MString code(find, tail-find);
                    //printf("found code %s\n", code.s());
                    if (code[0] == '#') {
                        // ascii code
                        _data.insert(-1, atoi(code.mid(1)));
                    }
                    else {
                        for (int i=0;xmlSpecialChars[i].name;i++)
                            if (xmlSpecialChars[i].name == code) {
                                _data.insert(-1, xmlSpecialChars[i].ch);
                                break;
                            }
                        // if no match was found, then don't add anything
                    }

                    // we've consumed this much
                    len -= (tail+1) - data;
                    data = tail+1;
                }
                else {
                    // invalid xml! just throw the rest in and be done
                    _data.strncat(find, len);
                    break;
                }
            }
            else {
                _data.strncat(data, len);
                break;
            }
        };
        //printf("end with %s\n", _data.s());
    }
    else if (processSpecialChars == 2 && data) {
        //printf("started with %s\n", MString(data, len).s());
        // get _data ready to receive about len characters
        _data.clear();
        if (len == -1)
            len = strlen(data);
        // there is no way to estimate possible expansion (except an unreasonable max)
        // so we're counting on MString to expand as needed
        len = (int)(len * 1.05);
        _data.allocate(len);

        // parse data
        while (1) {
            const char *find = data + strcspn(data, "&<>");
            _data.strncat(data, find-data);
            if (*find) {
                // take everything up to the character
                switch (*find) {
                case '&': _data.strncat("&amp;", 5); break;
                case '<': _data.strncat("&lt;", 4); break;
                case '>': _data.strncat("&gt;", 4); break;
                }
                data = find+1;
            }
            else
                break;
        };
        //printf("end with %s\n", _data.s());
    }
    else
        _data.strncpy(data,len);
}

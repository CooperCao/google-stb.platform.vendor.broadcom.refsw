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
#ifndef MXMLELEMENT_H
#define MXMLELEMENT_H

#include "mstring.h"
#include "mlist.h"
#include <stdio.h>

class MXmlAttribute {
public:
    MXmlAttribute(const char *name, const char *value) : _name(name), _value(value) {}
    MXmlAttribute(const char *name, int len = -1) : _name(name, len) {}
    void setValue(const char *value, int len = -1) {_value.strncpy(value, len);}

    const MString &name() const {return _name;}
    const MString &value() const {return _value;}

protected:
    MString _name, _value;
};

class MXmlElement {
public:
    MXmlElement(MXmlElement *parent, const char *tag = NULL, int len = -1);

    const MString &tag() const {return _tag;}
    MXmlElement *parent() const {return _parent;}
    void detachFromParent();

    MString &data() {return _data;}
    const MString &data() const {return _data;}

    /**
    * Because data is under a child, even if there are no other children,
    * data() is annoying. Instead use childData() which is equivalent to
    * firstChild()->data(), but checks for a NULL firstChild().
    */
    MString &childData();

    /**
    * Set a data element. There should be no tag() on a data element.
    *
    * If processSpecialChars == 1, characters like &amp; will be converted to &
    * If processSpecialChars == 2, characters like & will be converted to &amp;
    * If processSpecialChars == 0, nothing is done
    */
    void setData(const char *data, int len = -1, int processSpecialChars = 0);

    void add(MXmlAttribute *attr) {_attrs.add(attr);}
    void addAttr(const char *name, const char *value) {add(new MXmlAttribute(name,value));}
    MXmlAttribute *firstAttr() {return _attrs.first();}
    MXmlAttribute *nextAttr() {return _attrs.next();}
    MXmlAttribute *findAttr(const char *name);
    const char *attrValue(const char *name, const char *defaultValue = NULL);
    int attrInt(const char *name, int defaultValue = 0);

    void add(MXmlElement *child) {_children.add(child);child->_parent=this;}
    MXmlElement *firstChild() {return _children.first();}
    MXmlElement *nextChild() {return _children.next();}
    MXmlElement *lastChild() {return _children.last();}
    MXmlElement *prevChild() {return _children.prev();}
    MXmlElement *childAt(int index) {return _children.at(index);}
    MXmlElement *findChild(const char *tag, int len = -1);
    MXmlElement *findChild(const char *tag,
                           MXmlElement * startChild,
                           int len = -1);

    int childIndex(MXmlElement *child) {return _children.index(child);}

    /**
    * Removes child from list, clears parent flag, does not delete element.
    * Returns true if the child was found.
    */
    bool removeChild(MXmlElement *child);
    int totalChildren() const {return _children.total();}
    void clearChildren() {_children.clear();}
    MXmlElement *createChild(const char *name, const char *data = NULL, int processSpecialChars = 0);
    MXmlElement *createData(const char *data, int processSpecialChars = 0);

    /**
    * printf(FILE*) uses nice indenting. It's meant for display.
    */
    void print(FILE *f = stdout, int indent = 0);
    /**
    * printf(buf) uses no unnecessary whitespace. It's meant for
    * data transfer. Returns the amout of data written.
    */
    int print(char *buf, int buflen);

protected:
    MXmlElement *_parent;
    MString _tag, _data;
    MAutoList<MXmlElement> _children;
    MAutoList<MXmlAttribute> _attrs;
};

#endif //MXMLELEMENT_H

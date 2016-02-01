/******************************************************************************
 * (c) 2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

/***************************************************************************
*     (c)2003-2012 Broadcom Corporation
*
*  Thi program is the proprietary software of Broadcom Corporation and/or its licensors,
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
* Description: C-callable Wrapper Functions for BSEAV mlibs XML Parsing C++ classes
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include <strings.h>

#include "b_playback_ip_lib.h"
#include "b_playback_ip_priv.h"

#include "mxmlparser.h"

BDBG_MODULE(b_playback_ip_xml_helper);

/* Local prototypes */
static void B_PlaybackIp_XmlElem_PrintImpl( MXmlElement  *xmlElem,
                                            int           level,
                                            bool          recurse);


/*****************************************************************************
 * Create a tree of XML objects by parsing an XML string.
 *****************************************************************************/
B_PlaybackIp_XmlElement B_PlaybackIp_Xml_Create( const char *str)
{
    MXmlParser    xmlParser;
    MXmlElement   *xmlElemRoot;

    // Parse the XML file and get a pointer to the root element.
    xmlElemRoot = xmlParser.parse(str);

    return xmlElemRoot;
}

/*****************************************************************************
 * Destroy the tree of XML objects.
 *****************************************************************************/
void B_PlaybackIp_Xml_Destroy( B_PlaybackIp_XmlElement xmlElemRoot )
{
    delete xmlElemRoot;
    return;
}

/*****************************************************************************
 * Return a pointer to an element's parent element.
 *****************************************************************************/
B_PlaybackIp_XmlElement B_PlaybackIp_XmlElem_Parent(
                                            B_PlaybackIp_XmlElement xmlElem)
{
    MXmlElement *xmlElemParent;

    xmlElemParent = xmlElem->parent();
    return xmlElemParent;
}

/*****************************************************************************
 * Return a pointer to an element's tag (so you can tell what the element is).
 *****************************************************************************/
const char *B_PlaybackIp_XmlElem_GetTag( B_PlaybackIp_XmlElement xmlElem)
{
    const char *pTag = NULL;

    pTag = xmlElem->tag();
    return pTag;
}

/*****************************************************************************
 * Return a pointer to an element's data (which technically belongs to
 * a tagless child element).
 *****************************************************************************/
const char *B_PlaybackIp_XmlElem_ChildData( B_PlaybackIp_XmlElement xmlElem)
{
    const char *pData = NULL;

    pData = xmlElem->childData();
    return pData;
}

/*****************************************************************************
 * Find a element's data, and return its value
 * as a signed long integer.
 *****************************************************************************/
long int B_PlaybackIp_XmlElem_ChildDataInt(
                                        B_PlaybackIp_XmlElement   xmlElem,
                                        long int                  defaultValue)
{
    const char *pValue = NULL;

    pValue = B_PlaybackIp_XmlElem_ChildData(xmlElem);
    if (!pValue) return defaultValue;

    return strtol(pValue, NULL, 10);
}

/*****************************************************************************
 * Find a element's data, and return its value
 * as an unsigned long integer.
 *****************************************************************************/
unsigned long B_PlaybackIp_XmlElem_ChildDataUnsigned(
                                        B_PlaybackIp_XmlElement   xmlElem,
                                        unsigned long             defaultValue)
{
    const char *pValue = NULL;
    unsigned long value;

    pValue = B_PlaybackIp_XmlElem_ChildData(xmlElem);
    if (!pValue) return defaultValue;

    value = strtoul(pValue, NULL, 10);

    return value;
}

/*****************************************************************************
 * Find a element's data, and return its value
 * as an boolean.
 *****************************************************************************/
unsigned long B_PlaybackIp_XmlElem_ChildDataBoolean(
                                        B_PlaybackIp_XmlElement   xmlElem,
                                        unsigned long             defaultValue)
{
    const char *pValue = NULL;
    unsigned long value;

    pValue = B_PlaybackIp_XmlElem_ChildData(xmlElem);
    if (!pValue) return defaultValue;

    value = defaultValue;

    if ((strcasecmp(pValue, "true") == 0) || (strcasecmp(pValue, "1") == 0))
    {
        value = true;
    }

    if ((strcasecmp(pValue, "false") == 0) || (strcasecmp(pValue, "0") == 0))
    {
        value = false;
    }

    return value;
}

/*****************************************************************************
 * Find the first child element.
 *****************************************************************************/
B_PlaybackIp_XmlElement B_PlaybackIp_XmlElem_FirstChild(
                                            B_PlaybackIp_XmlElement xmlElem)
{
    MXmlElement *xmlElemChild;

    xmlElemChild = xmlElem->firstChild();
    return xmlElemChild;
}

/*****************************************************************************
 * Find the next child element.
 *****************************************************************************/
B_PlaybackIp_XmlElement B_PlaybackIp_XmlElem_NextChild(
                                            B_PlaybackIp_XmlElement xmlElem)
{
    MXmlElement *xmlElemChild;

    xmlElemChild = xmlElem->nextChild();
    return xmlElemChild;
}

/*****************************************************************************
 * Find a child element with a specific tag name.
 *****************************************************************************/
B_PlaybackIp_XmlElement B_PlaybackIp_XmlElem_FindChild(
                                        B_PlaybackIp_XmlElement  xmlElem,
                                        const char              *tag)
{
    MXmlElement *xmlElemChild;

    xmlElemChild = xmlElem->findChild(tag);
    return xmlElemChild;
}

/*****************************************************************************
 * Find the next child with a specific tag name.
 *****************************************************************************/
B_PlaybackIp_XmlElement B_PlaybackIp_XmlElem_FindNextChild(
                                  B_PlaybackIp_XmlElement  xmlElem,
                                  B_PlaybackIp_XmlElement  xmlElemStartChild,
                                  const char              *tag)
{
    MXmlElement *xmlElemChild;

    xmlElemChild = xmlElem->findChild(tag, xmlElemStartChild);
    return xmlElemChild;
}

/*****************************************************************************
 * Find the next child (continuing from xmlElemStartChild) with a specific
 * tag name.  If xmlElemStartChild is NULL, it will return the first
 * child with the specified tag name.
 *****************************************************************************/
B_PlaybackIp_XmlElement B_PlaybackIp_XmlElem_FindNextChildSameTag(
                                  B_PlaybackIp_XmlElement  xmlElem,
                                  B_PlaybackIp_XmlElement  xmlElemStartChild,
                                  const char              *tag)
{
    MXmlElement *xmlElemChild = NULL;
    const char *pTag;

    /* If we have a "startChild", then we need to some things to make
     * sure that we start from the child after "startChild", otherwise,
     * the findChild() call will just return the same child.
     * */
    if (xmlElemStartChild)
    {
        pTag = xmlElemStartChild->tag();

        /* Make sure that the "StartChild" is the current element. */
        xmlElemChild = xmlElem->findChild(pTag, xmlElemStartChild);

        /* Make sure we found the same child (while making it current). */
        BDBG_ASSERT(xmlElemChild == xmlElemStartChild);

        /* Move past the current so we don't keep finding the same one. */
        xmlElemChild = xmlElem->nextChild();
        if (!xmlElemChild)
        {
            return NULL;
        }
    }

    /* Now find the next (or first) element that matches the tag. */
    xmlElemChild = xmlElem->findChild(tag, xmlElemChild);
    return xmlElemChild;
}


/*****************************************************************************
 * Find a specific attribute of an element (by name), and return a pointer to
 * it's value string.
 *****************************************************************************/
const char *B_PlaybackIp_XmlElem_FindAttrValue(
                                        B_PlaybackIp_XmlElement     xmlElem,
                                        const char                 *name)
{
    const char *pValue = NULL;
    MXmlAttribute *xmlAttr;

    xmlAttr = xmlElem->findAttr(name);
    if (xmlAttr)
    {
        pValue = xmlAttr->value().s();
    }

    return pValue;
}

/*****************************************************************************
 * Find a specific attribute of an element (by name), and return its value
 * as a signed long integer.
 *****************************************************************************/
long int B_PlaybackIp_XmlElem_FindAttrValueInt(
                                        B_PlaybackIp_XmlElement   xmlElem,
                                        const char               *name,
                                        long int                  defaultValue)
{
    const char *pValue = NULL;

    pValue = B_PlaybackIp_XmlElem_FindAttrValue(xmlElem, name);
    if (!pValue) return defaultValue;

    return strtol(pValue, NULL, 10);
}

/*****************************************************************************
 * Find a specific attribute of an element (by name), and return its value
 * as an unsigned long integer.
 *****************************************************************************/
unsigned long B_PlaybackIp_XmlElem_FindAttrValueUnsigned(
                                        B_PlaybackIp_XmlElement   xmlElem,
                                        const char               *name,
                                        unsigned long             defaultValue)
{
    const char *pValue = NULL;
    unsigned long value;

    pValue = B_PlaybackIp_XmlElem_FindAttrValue(xmlElem, name);
    if (!pValue) return defaultValue;

    value = strtoul(pValue, NULL, 10);

    return value;
}

/*****************************************************************************
 * Find a specific attribute of an element (by name), and return its value
 * as an boolean.
 *****************************************************************************/
unsigned long B_PlaybackIp_XmlElem_FindAttrValueBoolean(
                                        B_PlaybackIp_XmlElement   xmlElem,
                                        const char               *name,
                                        unsigned long             defaultValue)
{
    const char *pValue = NULL;
    unsigned long value;

    pValue = B_PlaybackIp_XmlElem_FindAttrValue(xmlElem, name);
    if (!pValue) return defaultValue;

    value = defaultValue;

    if ((strcasecmp(pValue, "true") == 0) || (strcasecmp(pValue, "1") == 0))
    {
        value = true;
    }

    if ((strcasecmp(pValue, "false") == 0) || (strcasecmp(pValue, "0") == 0))
    {
        value = false;
    }

    return value;
}

/*****************************************************************************
 * Print an XML element (non-recursively).
 *****************************************************************************/
void B_PlaybackIp_XmlElem_Print( MXmlElement  *xmlElem,
                                 int           level)
{
    B_PlaybackIp_XmlElem_PrintImpl( xmlElem, level, false); /* false => non-recursive print */
}

/*****************************************************************************
 * Print an XML element (recursively).
 *****************************************************************************/
void B_PlaybackIp_XmlElem_PrintRecursively( MXmlElement  *xmlElem,
                                            int           level)
{
    B_PlaybackIp_XmlElem_PrintImpl( xmlElem, level, true);  /* true => recursive print */
}

/****************************************************************************
 ****************************************************************************/

/*****************************************************************************
 * Local print function used by B_PlaybackIp_XmlElem_Print() and
 * B_PlaybackIp_XmlElem_PrintRecursively().
 *****************************************************************************/
static void B_PlaybackIp_XmlElem_PrintImpl( MXmlElement  *xmlElem,
                                            int           level,
                                            bool          recurse)
{
    MXmlElement   *xmlChildElem;
    MXmlAttribute *xmlAttr;
    unsigned        indent = level * 2;

    if (xmlElem)
    {
        if (xmlElem->tag())
        {
            BDBG_LOG(("%*s Element Tag: \"%s\"", indent, "", xmlElem->tag().s() ));
        }
        if (xmlElem->childData())
        {
            BDBG_LOG(("%*s Element Data: \"%s\"", indent, "", xmlElem->childData().s() ));
        }

        indent += 2;

        xmlAttr = xmlElem->firstAttr();
        while (xmlAttr)
        {
            BDBG_LOG(("%*s Attr: \"%s\"=\"%s\" ", indent, "", xmlAttr->name().s(), xmlAttr->value().s() ));

            xmlAttr = xmlElem->nextAttr();
            //BDBG_LOG(("%*s: xmlElem->nextAttr(): %p", indent, "", xmlAttr ));
        }

        xmlChildElem = xmlElem->firstChild();
        while (xmlChildElem)
        {
            if (recurse)
            {
                B_PlaybackIp_XmlElem_PrintImpl(xmlChildElem, level + 1, true);  /* true => print recursively */
            }
            else
            {
                BDBG_LOG(("%*s Child Element: \"%s\"", indent, "", xmlChildElem->tag().s()  ));
            }

            xmlChildElem = xmlElem->nextChild();
        }
        indent -= 2;
    }
}

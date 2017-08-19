/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/
#include <strings.h>

#include "bip_string.h"
#include "bip_priv.h"

#include "mxmlparser.h"
#include "mxmlelement.h"

BDBG_MODULE(bip_xml);

/* Local prototypes */
static void BIP_XmlElem_PrintImpl( MXmlElement  *xmlElem,
                                            int           level,
                                            bool          recurse);


/*****************************************************************************
 * Create a tree of XML objects by parsing an XML string.
 *****************************************************************************/
BIP_XmlElement BIP_Xml_Create( const char *str)
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
void BIP_Xml_Destroy( BIP_XmlElement xmlElemRoot )
{
    delete xmlElemRoot;
    return;
}

/*****************************************************************************
 * Return a pointer to an element's parent element.
 *****************************************************************************/
BIP_XmlElement BIP_XmlElem_Parent(
                                            BIP_XmlElement xmlElem)
{
    MXmlElement *xmlElemParent;

    xmlElemParent = xmlElem->parent();
    return xmlElemParent;
}

/*****************************************************************************
 * Return a pointer to an element's tag (so you can tell what the element is).
 *****************************************************************************/
const char *BIP_XmlElem_GetTag( BIP_XmlElement xmlElem)
{
    const char *pTag = NULL;

    pTag = xmlElem->tag();
    return pTag;
}

/*****************************************************************************
 * Return a pointer to an element's data (which technically belongs to
 * a tagless child element).
 *****************************************************************************/
const char *BIP_XmlElem_ChildData( BIP_XmlElement xmlElem)
{
    const char *pData = NULL;

    pData = xmlElem->childData();
    return pData;
}

/*****************************************************************************
 * Find a element's data, and return its value
 * as a signed long integer.
 *****************************************************************************/
long int BIP_XmlElem_ChildDataInt(
                                        BIP_XmlElement   xmlElem,
                                        long int                  defaultValue)
{
    const char *pValue = NULL;

    pValue = BIP_XmlElem_ChildData(xmlElem);
    if (!pValue) return defaultValue;

    return strtol(pValue, NULL, 10);
}

/*****************************************************************************
 * Find a element's data, and return its value
 * as an unsigned long integer.
 *****************************************************************************/
unsigned long BIP_XmlElem_ChildDataUnsigned(
                                        BIP_XmlElement   xmlElem,
                                        unsigned long             defaultValue)
{
    const char *pValue = NULL;
    unsigned long value;

    pValue = BIP_XmlElem_ChildData(xmlElem);
    if (!pValue) return defaultValue;

    value = strtoul(pValue, NULL, 10);

    return value;
}

/*****************************************************************************
 * Find a element's data, and return its value
 * as an boolean.
 *****************************************************************************/
unsigned long BIP_XmlElem_ChildDataBoolean(
                                        BIP_XmlElement   xmlElem,
                                        unsigned long             defaultValue)
{
    const char *pValue = NULL;
    unsigned long value;

    pValue = BIP_XmlElem_ChildData(xmlElem);
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
BIP_XmlElement BIP_XmlElem_FirstChild(
                                            BIP_XmlElement xmlElem)
{
    MXmlElement *xmlElemChild;

    xmlElemChild = xmlElem->firstChild();
    return xmlElemChild;
}

/*****************************************************************************
 * Find the next child element.
 *****************************************************************************/
BIP_XmlElement BIP_XmlElem_NextChild(
                                            BIP_XmlElement xmlElem)
{
    MXmlElement *xmlElemChild;

    xmlElemChild = xmlElem->nextChild();
    return xmlElemChild;
}

/*****************************************************************************
 * Find a child element with a specific tag name.
 *****************************************************************************/
BIP_XmlElement BIP_XmlElem_FindChild(
                                        BIP_XmlElement  xmlElem,
                                        const char              *tag)
{
    MXmlElement *xmlElemChild;

    xmlElemChild = xmlElem->findChild(tag);
    return xmlElemChild;
}

/*****************************************************************************
 * Find the next child with a specific tag name.
 *****************************************************************************/
BIP_XmlElement BIP_XmlElem_FindNextChild(
                                  BIP_XmlElement  xmlElem,
                                  BIP_XmlElement  xmlElemStartChild,
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
BIP_XmlElement BIP_XmlElem_FindNextChildSameTag(
                                  BIP_XmlElement  xmlElem,
                                  BIP_XmlElement  xmlElemStartChild,
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
const char *BIP_XmlElem_FindAttrValue(
                                        BIP_XmlElement     xmlElem,
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
long int BIP_XmlElem_FindAttrValueInt(
                                        BIP_XmlElement   xmlElem,
                                        const char               *name,
                                        long int                  defaultValue)
{
    const char *pValue = NULL;

    pValue = BIP_XmlElem_FindAttrValue(xmlElem, name);
    if (!pValue) return defaultValue;

    return strtol(pValue, NULL, 10);
}

/*****************************************************************************
 * Find a specific attribute of an element (by name), and return its value
 * as an unsigned long integer.
 *****************************************************************************/
unsigned long BIP_XmlElem_FindAttrValueUnsigned(
                                        BIP_XmlElement   xmlElem,
                                        const char               *name,
                                        unsigned long             defaultValue)
{
    const char *pValue = NULL;
    unsigned long value;

    pValue = BIP_XmlElem_FindAttrValue(xmlElem, name);
    if (!pValue) return defaultValue;

    value = strtoul(pValue, NULL, 10);

    return value;
}

/*****************************************************************************
 * Find a specific attribute of an element (by name), and return its value
 * as an uint64_t.
 *****************************************************************************/
int64_t BIP_XmlElem_FindAttrValueUnsigned64(
                                        BIP_XmlElement   xmlElem,
                                        const char               *name,
                                        unsigned long             defaultValue)
{
    const char *pValue = NULL;
    int64_t value;

    pValue = BIP_XmlElem_FindAttrValue(xmlElem, name);
    if (!pValue) return defaultValue;

    value = strtoll(pValue, NULL, 10);

    return value;
}

/*****************************************************************************
 * Find a specific attribute of an element (by name), and return its value
 * as an int64_t.
 *****************************************************************************/
uint64_t BIP_XmlElem_FindAttrValueInt64(
                                        BIP_XmlElement   xmlElem,
                                        const char               *name,
                                        unsigned long             defaultValue)
{
    const char *pValue = NULL;
    uint64_t value;

    pValue = BIP_XmlElem_FindAttrValue(xmlElem, name);
    if (!pValue) return defaultValue;

    value = strtoull(pValue, NULL, 10);

    return value;
}

/*****************************************************************************
 * Find a specific attribute of an element (by name), and return its value
 * as an boolean.
 *****************************************************************************/
unsigned long BIP_XmlElem_FindAttrValueBoolean(
                                        BIP_XmlElement   xmlElem,
                                        const char               *name,
                                        unsigned long             defaultValue)
{
    const char *pValue = NULL;
    unsigned long value;

    pValue = BIP_XmlElem_FindAttrValue(xmlElem, name);
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
void BIP_XmlElem_Print( MXmlElement  *xmlElem,
                                 int           level)
{
    BIP_XmlElem_PrintImpl( xmlElem, level, false); /* false => non-recursive print */
}

/*****************************************************************************
 * Print an XML element (recursively).
 *****************************************************************************/
void BIP_XmlElem_PrintRecursively( MXmlElement  *xmlElem,
                                            int           level)
{
    BIP_XmlElem_PrintImpl( xmlElem, level, true);  /* true => recursive print */
}

/****************************************************************************
 ****************************************************************************/

/*****************************************************************************
 * Local print function used by BIP_XmlElem_Print() and
 * BIP_XmlElem_PrintRecursively().
 *****************************************************************************/
static void BIP_XmlElem_PrintImpl( MXmlElement  *xmlElem,
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
                BIP_XmlElem_PrintImpl(xmlChildElem, level + 1, true);  /* true => print recursively */
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

/*****************************************************************************
* Create a child XML element that belongs to an existing parent.
* Specifying NULL for the parent will create a root element.
*****************************************************************************/
BIP_XmlElement BIP_XmlElem_Create(
                        BIP_XmlElement   xmlElem,   /* Parent element. Pass NULL to create root element. */
                        const char      *name      /*name of the element */
                        )
{
    MXmlElement   *newXmlElem = NULL;
    BIP_Status rc;

    if(xmlElem == NULL)
    {
        /* Creating the root element since there is no parent.*/
        newXmlElem  = new MXmlElement(NULL,name);
        BIP_CHECK_GOTO(( newXmlElem != NULL ), ( "Failed to allocate memory for MXmlElement Object"), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

        return newXmlElem;
    }
    else
    {
        /* Create the child element.*/
        newXmlElem = xmlElem->createChild(name );
        return newXmlElem;
    }
    return newXmlElem;
error:
    return NULL;
}

/*****************************************************************************
* Add an attribute to an existing XML element.
*****************************************************************************/
void BIP_XmlElem_AddAttr(
    BIP_XmlElement   xmlElem,
    const char *     name,
    const char *     value
    )
{
    if(xmlElem != NULL)
    {
        xmlElem->addAttr(name, value);
    }
    else
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "Trying to add an attribute for a non existent element. " BIP_MSG_PRE_ARG));
    }
}

/*****************************************************************************
* Add an integer attribute to an existing XML element.
*****************************************************************************/
BIP_Status BIP_XmlElem_AddAttrInt(
    BIP_XmlElement   xmlElem,
    const char *     name,
    int value
    )
{

    BIP_Status rc = BIP_SUCCESS;
    BIP_StringHandle hString = NULL;

    hString = BIP_String_CreateFromPrintf("%d",value);
    BIP_CHECK_GOTO((hString ), ( "%s:BIP_String_CreateFromPrintf failed \n", BSTD_FUNCTION ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    BIP_XmlElem_AddAttr(xmlElem, name, BIP_String_GetString(hString));

    BIP_String_Destroy(hString);
error:
    return rc;
}

/*****************************************************************************
* Add an unsigned integer attribute to an existing XML element.
*****************************************************************************/
BIP_Status BIP_XmlElem_AddAttrUnsigned(
    BIP_XmlElement   xmlElem,
    const char *     name,
    unsigned value
    )
{
    BIP_Status rc = BIP_SUCCESS;
    BIP_StringHandle hString = NULL;

    hString = BIP_String_CreateFromPrintf("%u",value);
    BIP_CHECK_GOTO((hString ), ( "%s:BIP_String_CreateFromPrintf failed \n", BSTD_FUNCTION ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    BIP_XmlElem_AddAttr(xmlElem, name, BIP_String_GetString(hString));

    BIP_String_Destroy(hString);

error:
    return rc;
}

/*****************************************************************************
* Add a float attribute to an existing XML element.
*****************************************************************************/
BIP_Status BIP_XmlElem_AddAttrFloat(
    BIP_XmlElement   xmlElem,
    const char *     name,
    float value
    )
{
    BIP_Status rc = BIP_SUCCESS;
    BIP_StringHandle hString = NULL;

    hString = BIP_String_CreateFromPrintf("%f",value);
    BIP_CHECK_GOTO((hString ), ( "%s:BIP_String_CreateFromPrintf failed \n", BSTD_FUNCTION ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    BIP_XmlElem_AddAttr(xmlElem, name, BIP_String_GetString(hString));

    BIP_String_Destroy(hString);

error:
    return rc;
}

/*****************************************************************************
* Add an int64_t attribute to an existing XML element.
*****************************************************************************/
BIP_Status BIP_XmlElem_AddAttrInt64(
    BIP_XmlElement   xmlElem,
    const char *     name,
    int64_t value
    )
{
    BIP_Status rc = BIP_SUCCESS;
    BIP_StringHandle hString = NULL;

    hString = BIP_String_CreateFromPrintf("%lld",value);
    BIP_CHECK_GOTO((hString ), ( "%s:BIP_String_CreateFromPrintf failed \n", BSTD_FUNCTION ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    BIP_XmlElem_AddAttr(xmlElem, name, BIP_String_GetString(hString));

    BIP_String_Destroy(hString);

error:
    return rc;
}

/*****************************************************************************
* Add an uint64_t attribute to an existing XML element.
*****************************************************************************/
BIP_Status BIP_XmlElem_AddAttrUnsigned64(
    BIP_XmlElement   xmlElem,
    const char *     name,
    uint64_t value
    )
{
    BIP_Status rc = BIP_SUCCESS;
    BIP_StringHandle hString = NULL;

    hString = BIP_String_CreateFromPrintf("%llu",value);
    BIP_CHECK_GOTO((hString ), ( "%s:BIP_String_CreateFromPrintf failed \n", BSTD_FUNCTION ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    BIP_XmlElem_AddAttr(xmlElem, name, BIP_String_GetString(hString));

    BIP_String_Destroy(hString);

error:
    return rc;
}

/*****************************************************************************
* Add a boolean attribute to an existing XML element.
*****************************************************************************/
BIP_Status BIP_XmlElem_AddAttrBool(
    BIP_XmlElement   xmlElem,
    const char *     name,
    bool value
    )
{
    BIP_Status rc = BIP_SUCCESS;
    BIP_StringHandle hString = NULL;

    if(true == value)
    {
        hString = BIP_String_CreateFromPrintf("%s","true");
        BIP_CHECK_GOTO((hString ), ( "%s:BIP_String_CreateFromPrintf failed \n", BSTD_FUNCTION ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );
    }
    else
    {
        hString = BIP_String_CreateFromPrintf("%s","false");
        BIP_CHECK_GOTO((hString ), ( "%s:BIP_String_CreateFromPrintf failed \n", BSTD_FUNCTION ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );
    }

    BIP_XmlElem_AddAttr(xmlElem, name, BIP_String_GetString(hString));

    BIP_String_Destroy(hString);

error:
    return rc;
}

/***************************************************************************
*  Save xml tree in to a file.
****************************************************************************/
void BIP_XmlElem_WriteToFile(
                            BIP_XmlElement   xmlElem,
                            FILE *xmlFile,
                            int indent
                            )
{
    if(xmlFile == NULL)
    {
        BDBG_ERR((BIP_MSG_PRE_FMT "Invalid xmlFile pointer" BIP_MSG_PRE_ARG));
    }
    else
    {
        if(indent != -1)
        {
            xmlElem->print(xmlFile, indent);
        }
        else
        {
            xmlElem->print(xmlFile);
        }
    }
}

/***************************************************************************
*  Save xml tree in to a file.
****************************************************************************/
void BIP_XmlElem_WriteToBuf(
                            BIP_XmlElement   xmlElem,
                            char *buf,
                            int buflen
                            )
{
    if((buf == NULL) || (buflen==0))
    {
        BDBG_ERR((BIP_MSG_PRE_FMT "Invalid buffer" BIP_MSG_PRE_ARG));
    }
    else
    {
        xmlElem->print(buf, buflen);
    }
}

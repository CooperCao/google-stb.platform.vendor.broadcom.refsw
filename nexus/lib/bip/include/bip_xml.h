/******************************************************************************
 * (c) 2015 Broadcom Corporation
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

#ifndef __BIP_XML_H__
#define __BIP_XML_H__

/* Wrappers for C++ XML Parser Functions in "bip_xml.cpp" */
#ifdef __cplusplus
extern "C" {
#endif

    typedef struct MXmlElement   *BIP_XmlElement;

    /*****************************************************************************
     * Create a tree of XML objects by parsing an XML string.
     *****************************************************************************/
    BIP_XmlElement BIP_Xml_Create( const char *str );

    /*****************************************************************************
     * Destroy the tree of XML objects.
     *****************************************************************************/
    void BIP_Xml_Destroy( BIP_XmlElement xmlElemRoot );

    /*****************************************************************************
     * Return a pointer to an element's parent element.
     *****************************************************************************/
    BIP_XmlElement BIP_XmlElem_Parent(
                                                BIP_XmlElement xmlElem);

    /*****************************************************************************
     * Print an XML element (non-recursively).
     *****************************************************************************/
    void BIP_XmlElem_Print( const  BIP_XmlElement xmlElem,
                                     int    level);

    /*****************************************************************************
     * Print an XML element and all elements underneath it (i.e., recursive).
     *****************************************************************************/
    void BIP_XmlElem_PrintRecursively(
                                    const BIP_XmlElement xmlElem,
                                    int   level);

    /*****************************************************************************
     * Return a pointer to an element's tag (so you can tell what the element is).
     *****************************************************************************/
    const char *BIP_XmlElem_GetTag( BIP_XmlElement xmlElem);

    /*****************************************************************************
     * Return a pointer to an element's data (which technically belongs to
     * a tagless child element).
     *****************************************************************************/
    const char *BIP_XmlElem_ChildData( BIP_XmlElement xmlElem);

    /*****************************************************************************
     * Find a element's data, and return its value
     * as a signed long integer.
     *****************************************************************************/
    long int BIP_XmlElem_ChildDataInt(
                                            BIP_XmlElement   xmlElem,
                                            long int                  defaultValue);

    /*****************************************************************************
     * Find a element's data, and return its value
     * as an unsigned long integer.
     *****************************************************************************/
    unsigned long BIP_XmlElem_ChildDataUnsigned(
                                            BIP_XmlElement   xmlElem,
                                            unsigned long             defaultValue);

    /*****************************************************************************
     * Find a element's data, and return its value
     * as an boolean.
     *****************************************************************************/
    unsigned long BIP_XmlElem_ChildDataBoolean(
                                            BIP_XmlElement   xmlElem,
                                            unsigned long             defaultValue);

    /*****************************************************************************
     * Find the first child element.
     *****************************************************************************/
    BIP_XmlElement BIP_XmlElem_FirstChild(
                                                BIP_XmlElement xmlElem);

    /*****************************************************************************
     * Find the next child element.
     *****************************************************************************/
    BIP_XmlElement BIP_XmlElem_NextChild(
                                                BIP_XmlElement xmlElem);

    /*****************************************************************************
     * Find a child element with a specific tag name.
     *****************************************************************************/
    BIP_XmlElement BIP_XmlElem_FindChild(
                                            BIP_XmlElement  xmlElem,
                                            const char              *tag);

    /*****************************************************************************
     * Find the next child with a specific tag name.
     *****************************************************************************/
    BIP_XmlElement BIP_XmlElem_FindNextChild(
                                    BIP_XmlElement  xmlElem,
                                    BIP_XmlElement  xmlElemStartChild,
                                    const char              *tag);

    /*****************************************************************************
     * Find the next child (continuing from xmlElemStartChild) with a specific
     * tag name.  If xmlElemStartChild is NULL, it will return the first
     * child with the specified tag name.
     *****************************************************************************/
    BIP_XmlElement BIP_XmlElem_FindNextChildSameTag(
                                  BIP_XmlElement  xmlElem,
                                  BIP_XmlElement  xmlElemStartChild,
                                  const char              *tag);

    /*****************************************************************************
     * Find a specific attribute of an element (by name), and return a pointer to
     * it's value string.
     *****************************************************************************/
    const char *BIP_XmlElem_FindAttrValue(
                                            BIP_XmlElement  xmlElem,
                                            const char              *name);

    /*****************************************************************************
     * Find a specific attribute of an element (by name), and return its value
     * as a signed long integer.
     *****************************************************************************/
    long int BIP_XmlElem_FindAttrValueInt(
                                            BIP_XmlElement  xmlElem,
                                            const char              *name,
                                            long int                 defaultValue);

    /*****************************************************************************
     * Find a specific attribute of an element (by name), and return its value
     * as an unsigned long integer.
     *****************************************************************************/
    unsigned long BIP_XmlElem_FindAttrValueUnsigned(
                                            BIP_XmlElement   xmlElem,
                                            const char               *name,
                                            unsigned long             defaultValue);

    /*****************************************************************************
     * Find a specific attribute of an element (by name), and return its value
     * as an uint64_t.
     *****************************************************************************/
    int64_t BIP_XmlElem_FindAttrValueUnsigned64(
                                            BIP_XmlElement   xmlElem,
                                            const char               *name,
                                            unsigned long             defaultValue);

    /*****************************************************************************
     * Find a specific attribute of an element (by name), and return its value
     * as an int64_t.
     *****************************************************************************/
    uint64_t BIP_XmlElem_FindAttrValueInt64(
                                            BIP_XmlElement   xmlElem,
                                            const char               *name,
                                            unsigned long             defaultValue);

    /*****************************************************************************
     * Find a specific attribute of an element (by name), and return its value
     * as an boolean.
     *****************************************************************************/
    unsigned long BIP_XmlElem_FindAttrValueBoolean(
                                            BIP_XmlElement   xmlElem,
                                            const char               *name,
                                            unsigned long             defaultValue);

    /*****************************************************************************
    * Create a child XML element that belongs to an existing parent.
    * Specifying NULL for the parent will create a root element.
    *****************************************************************************/
    BIP_XmlElement BIP_XmlElem_Create(
                        BIP_XmlElement   xmlElem,   /* Parent element. Pass NULL to create root element. */
                        const char      *name      /*name of the element */
                        );

    /*****************************************************************************
    * Add an attribute to an existing XML element.
    *****************************************************************************/
    void BIP_XmlElem_AddAttr(
            BIP_XmlElement   xmlElem,
            const char *     name,
            const char *     value
            );

    /*****************************************************************************
    * Add an integer attribute to an existing XML element.
    *****************************************************************************/
    BIP_Status BIP_XmlElem_AddAttrInt(
            BIP_XmlElement   xmlElem,
            const char *     name,
            int value
            );

    /*****************************************************************************
    * Add an unsigned integer attribute to an existing XML element.
    *****************************************************************************/
    BIP_Status BIP_XmlElem_AddAttrUnsigned(
            BIP_XmlElement   xmlElem,
            const char *     name,
            unsigned value
            );

    /*****************************************************************************
    * Add a float attribute to an existing XML element.
    *****************************************************************************/
    BIP_Status BIP_XmlElem_AddAttrFloat(
            BIP_XmlElement   xmlElem,
            const char *     name,
            float value
            );

    /*****************************************************************************
    * Add an int64_t attribute to an existing XML element.
    *****************************************************************************/
    BIP_Status BIP_XmlElem_AddAttrInt64(
            BIP_XmlElement   xmlElem,
            const char *     name,
            int64_t value
            );

    /*****************************************************************************
    * Add an uint64_t attribute to an existing XML element.
    *****************************************************************************/
    BIP_Status BIP_XmlElem_AddAttrUnsigned64(
            BIP_XmlElement   xmlElem,
            const char *     name,
            uint64_t value
            );

    /*****************************************************************************
    * Add a boolean attribute to an existing XML element.
    *****************************************************************************/
    BIP_Status BIP_XmlElem_AddAttrBool(
            BIP_XmlElement   xmlElem,
            const char *     name,
            bool value
            );

    /***************************************************************************
    *  Save xml tree in to a file.
    ****************************************************************************/
    void BIP_XmlElem_WriteToFile(
                            BIP_XmlElement   xmlElem,
                            FILE *xmlFile,
                            int indent
                            );

    /***************************************************************************
    *  Save xml tree in to a file.
    ****************************************************************************/
    void BIP_XmlElem_WriteToBuf(
                            BIP_XmlElement   xmlElem,
                            char *buf,
                            int buflen
                            );

#ifdef __cplusplus
}
#endif

#endif /* __BIP_XML_H__ */

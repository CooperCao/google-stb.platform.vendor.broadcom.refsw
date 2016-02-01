/***************************************************************************
*     (c)2003-2014 Broadcom Corporation
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
* Description: Prototypes and typedefs for b_playback_xml_helper.c.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#ifndef __B_PLAYBACK_IP_XML_HELPER_H__
#define __B_PLAYBACK_IP_XML_HELPER_H__

/* Wrappers for C++ XML Parser Functions in "b_playback_ip_xml_helper.cpp" */
#ifdef __cplusplus
extern "C" {
#endif

    typedef struct MXmlElement   *B_PlaybackIp_XmlElement;

    /*****************************************************************************
     * Create a tree of XML objects by parsing an XML string.
     *****************************************************************************/
    B_PlaybackIp_XmlElement B_PlaybackIp_Xml_Create( const char *str );

    /*****************************************************************************
     * Destroy the tree of XML objects.
     *****************************************************************************/
    void B_PlaybackIp_Xml_Destroy( B_PlaybackIp_XmlElement xmlElemRoot );

    /*****************************************************************************
     * Return a pointer to an element's parent element.
     *****************************************************************************/
    B_PlaybackIp_XmlElement B_PlaybackIp_XmlElem_Parent(
                                                B_PlaybackIp_XmlElement xmlElem);

    /*****************************************************************************
     * Print an XML element (non-recursively).
     *****************************************************************************/
    void B_PlaybackIp_XmlElem_Print( const  B_PlaybackIp_XmlElement xmlElem,
                                     int    level);

    /*****************************************************************************
     * Print an XML element and all elements underneath it (i.e., recursive).
     *****************************************************************************/
    void B_PlaybackIp_XmlElem_PrintRecursively(
                                    const B_PlaybackIp_XmlElement xmlElem,
                                    int   level);

    /*****************************************************************************
     * Return a pointer to an element's tag (so you can tell what the element is).
     *****************************************************************************/
    const char *B_PlaybackIp_XmlElem_GetTag( B_PlaybackIp_XmlElement xmlElem);

    /*****************************************************************************
     * Return a pointer to an element's data (which technically belongs to
     * a tagless child element).
     *****************************************************************************/
    const char *B_PlaybackIp_XmlElem_ChildData( B_PlaybackIp_XmlElement xmlElem);

    /*****************************************************************************
     * Find a element's data, and return its value
     * as a signed long integer.
     *****************************************************************************/
    long int B_PlaybackIp_XmlElem_ChildDataInt(
                                            B_PlaybackIp_XmlElement   xmlElem,
                                            long int                  defaultValue);

    /*****************************************************************************
     * Find a element's data, and return its value
     * as an unsigned long integer.
     *****************************************************************************/
    unsigned long B_PlaybackIp_XmlElem_ChildDataUnsigned(
                                            B_PlaybackIp_XmlElement   xmlElem,
                                            unsigned long             defaultValue);

    /*****************************************************************************
     * Find a element's data, and return its value
     * as an boolean.
     *****************************************************************************/
    unsigned long B_PlaybackIp_XmlElem_ChildDataBoolean(
                                            B_PlaybackIp_XmlElement   xmlElem,
                                            unsigned long             defaultValue);

    /*****************************************************************************
     * Find the first child element.
     *****************************************************************************/
    B_PlaybackIp_XmlElement B_PlaybackIp_XmlElem_FirstChild(
                                                B_PlaybackIp_XmlElement xmlElem);

    /*****************************************************************************
     * Find the next child element.
     *****************************************************************************/
    B_PlaybackIp_XmlElement B_PlaybackIp_XmlElem_NextChild(
                                                B_PlaybackIp_XmlElement xmlElem);

    /*****************************************************************************
     * Find a child element with a specific tag name.
     *****************************************************************************/
    B_PlaybackIp_XmlElement B_PlaybackIp_XmlElem_FindChild(
                                            B_PlaybackIp_XmlElement  xmlElem,
                                            const char              *tag);

    /*****************************************************************************
     * Find the next child with a specific tag name.
     *****************************************************************************/
    B_PlaybackIp_XmlElement B_PlaybackIp_XmlElem_FindNextChild(
                                    B_PlaybackIp_XmlElement  xmlElem,
                                    B_PlaybackIp_XmlElement  xmlElemStartChild,
                                    const char              *tag);

    /*****************************************************************************
     * Find the next child (continuing from xmlElemStartChild) with a specific
     * tag name.  If xmlElemStartChild is NULL, it will return the first
     * child with the specified tag name.
     *****************************************************************************/
    B_PlaybackIp_XmlElement B_PlaybackIp_XmlElem_FindNextChildSameTag(
                                  B_PlaybackIp_XmlElement  xmlElem,
                                  B_PlaybackIp_XmlElement  xmlElemStartChild,
                                  const char              *tag);

    /*****************************************************************************
     * Find a specific attribute of an element (by name), and return a pointer to
     * it's value string.
     *****************************************************************************/
    const char *B_PlaybackIp_XmlElem_FindAttrValue(
                                            B_PlaybackIp_XmlElement  xmlElem,
                                            const char              *name);

    /*****************************************************************************
     * Find a specific attribute of an element (by name), and return its value
     * as a signed long integer.
     *****************************************************************************/
    long int B_PlaybackIp_XmlElem_FindAttrValueInt(
                                            B_PlaybackIp_XmlElement  xmlElem,
                                            const char              *name,
                                            long int                 defaultValue);

    /*****************************************************************************
     * Find a specific attribute of an element (by name), and return its value
     * as an unsigned long integer.
     *****************************************************************************/
    unsigned long B_PlaybackIp_XmlElem_FindAttrValueUnsigned(
                                            B_PlaybackIp_XmlElement   xmlElem,
                                            const char               *name,
                                            unsigned long             defaultValue);

    /*****************************************************************************
     * Find a specific attribute of an element (by name), and return its value
     * as an boolean.
     *****************************************************************************/
    unsigned long B_PlaybackIp_XmlElem_FindAttrValueBoolean(
                                            B_PlaybackIp_XmlElement   xmlElem,
                                            const char               *name,
                                            unsigned long             defaultValue);

#ifdef __cplusplus
}
#endif

#endif /* __B_PLAYBACK_IP_XML_HELPER_H__ */

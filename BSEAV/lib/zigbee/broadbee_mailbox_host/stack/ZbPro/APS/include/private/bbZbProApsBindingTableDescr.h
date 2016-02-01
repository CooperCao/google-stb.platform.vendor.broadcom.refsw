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
******************************************************************************/
/*****************************************************************************
 *
 * FILENAME: $Workfile: trunk/stack/ZbPro/APS/include/private/bbZbProApsBindingTableDescr.h $
 *
 * DESCRIPTION:
 *   ZigBee PRO APS Binding Table descriptors.
 *
 * $Revision: 1355 $
 * $Date: 2014-02-06 19:24:48Z $
 *
 ****************************************************************************************/


#ifndef _ZBPRO_APS_BINDING_TABLE_DESCR_H
#define _ZBPRO_APS_BINDING_TABLE_DESCR_H


/************************* INCLUDES *****************************************************/
#include "bbZbProApsKeywords.h"     /* ZigBee PRO APS macro keywords definition. */
#include "bbZbProApsCommon.h"       /* ZigBee PRO APS general types definitions. */


/************************* TYPES ********************************************************/
/**//**
 * \brief The source of the binding link data structure.
 * \note The field SrcAddr (64-bit) is omitted because this implementation of APS does not
 *  support Binding Tables caching and as a result this field is constantly equal to this
 *  device MAC Extended Address for the ZigBee PRO context (see ZigBee Document
 *  08-0006-05 ZigBee PRO/2007 Layer PICS and Stack Profiles, item AZD600).
 * \note The field DstEndpoint is moved from the destination link data structure just to
 *  reduce the amount of memory needed for a single link-to-device record.
 * \note To compare the source of a link with a key, apply mask to the \c plain field in
 *  order to ignore the value of the \c dstEndpoint field. The field \c dstEndpoint is
 *  mapped on the most significant byte of the \c plain field.
 * \note To compare a whole link with a pattern while looking for the same record in the
 *  Binding Table, use the \c plain field without masking.
 * \note The \c dstEndpoint field must be filled with zero for link-to-group record.
 */
typedef union _ZbProApsBindingTableLinkSrc_t
{
    uint32_t                    plain;          /*!< The plain 32-bit data. */
    struct
    {
        /* 16-bit data. */
        ZBPRO_APS_ClusterId_t   clusterId;      /*!< The identifier of the cluster on the source device
                                                    that is bound to the destination device. */
        /* 8-bit data. */
        ZBPRO_APS_EndpointId_t  srcEndpoint;    /*!< The source endpoint of the binding entry. */

        ZBPRO_APS_EndpointId_t  dstEndpoint;    /*!< The endpoint identifier of the destination device of
                                                    the binding entry (only for link-to-device record). */
    };
} ZbProApsBindingTableLinkSrc_t;


/**//**
 * \brief The destination of the binding link data structure, for link to a device.
 * \note The field DstEndpoint is moved into the source link data structure just to reduce
 *  the amount of memory needed for a single link-to-device record.
 */
typedef struct _ZbProApsBindingTableLinkDstDevice_t
{
    /* 64-bit data. */
    ZBPRO_APS_ExtAddr_t     dstExtAddr;     /*!< The 64-bit address of the destination device
                                                of the binding entry. */
} ZbProApsBindingTableLinkDstDevice_t;


/**//**
 * \brief The destination of the binding link data structure, for link to a group.
 * \note The \c padding field must be filled with zero.
 */
typedef struct _ZbProApsBindingTableLinkDstGroup_t
{
    /* 16-bit data. */
    ZBPRO_APS_GroupId_t     dstGroupAddr;   /*!< The 16-bit address of the destination group
                                                of the binding entry. */
    uint16_t                padding;        /*!< Explicit padding to be filled with zero. */

} ZbProApsBindingTableLinkDstGroup_t;


/**//**
 * \brief The Binding Table single link-to-device record data structure.
 */
typedef struct _ZbProApsBindingTableLinkToDevice_t
{
    /* Structured / 32-bit data. */
    ZbProApsBindingTableLinkSrc_t       src;    /*!< The source of a binding link. */

    /* Structured / 64-bit data. */
    ZbProApsBindingTableLinkDstDevice_t dst;    /*!< The destination of a binding link. */

} ZbProApsBindingTableLinkToDevice_t;


/**//**
 * \brief The Binding Table single link-to-group record data structure.
 */
typedef struct _ZbProApsBindingTableLinkToGroup_t
{
    /* Structured / 32-bit data. */
    ZbProApsBindingTableLinkSrc_t       src;    /*!< The source of a binding link. */

    /* Structured / 32(16+padding)-bit data. */
    ZbProApsBindingTableLinkDstGroup_t  dst;    /*!< The destination of a binding link. */

} ZbProApsBindingTableLinkToGroup_t;


/**//**
 * \brief Binding Table size, in rows of the type link-to-group.
 * \details This constant specifies the maximum number of rows in the Binding Table of the
 *  type link-to-group. If there are some rows of the type link-to-device, the total
 *  number of all rows of both types must not exceed this constant.
 * \note The size of a row of the type link-to-group is 8 bytes, while the size of a row
 *  of the type link-to-device is 12 bytes, so the maximum number of rows of the type
 *  link-to-device equals to this constant multiplied by 2/3.
 * \note The maximum allowed value for this constant is 255.
 */
#define ZBPRO_APS_BINDING_TABLE_SIZE_LINKS_TO_GROUPS    ZBPRO_APS_BINDING_TABLE_SIZE


/**//**
 * \brief Binding Table size, in rows of the type link-to-device.
 * \note The multiplier 2/3 is the hard-coded value of the relation:
 *  sizeof(ZbProApsBindingTableLinkToGroup_t) / sizeof(ZbProApsBindingTableLinkToDevice_t)
 * \note The correct sizes of \c ZbProApsBindingTableLinkToGroup_t and
 *  \c ZbProApsBindingTableLinkToDevice_t are validated in the code of function
 *  \c zbProApsBindingTableReset.
 */
#define ZBPRO_APS_BINDING_TABLE_SIZE_LINKS_TO_DEVICES       \
        (ZBPRO_APS_BINDING_TABLE_SIZE_LINKS_TO_GROUPS * 2/3)


/**//**
 * \brief The Binding Table descriptor data structure.
 * \details The \c linksToDevicesNumber and \c linksToGroupsNumber stores number of rows
 *  of types link-to-device and link-to-group in the Binding Table correspondingly.
 * \note The total number of links to devices and to groups must be less or equal to 255.
 * \details The \c lastReturnedLinkIdx, \c lastReturnedIsValid, and \c lastSearchRequest
 *  are used by the search-by-the-key method to speed-up the search of the next matching
 *  link. If there was no changes made in the Binding Table past the previous search
 *  (i.e., no new rows were actually added and no rows were deleted), and the new search
 *  request is issued with the same key as the previous one, and with the same skip-peer
 *  counter value or with the value incremented by one or more to the previously saved,
 *  then the search will be continued from the last point denoted with
 *  \c lastReturnedLinkIdx; otherwise the search is started from the beginning of the
 *  Binding Table.
 * \details The field \c lastReturnedIsValid shows if the saved state of the previous
 *  search-request is valid. It is set to FALSE when changes are actually performed with
 *  the Binding Table (a new row is added, or an existing row is deleted); and it is set
 *  to TRUE with the first (or next) search-request to the Binding Table.
 * \details The field \c lastReturnedLinkIdx stores the index of the row that corresponds
 *  to the i-th element of the return on the previous search-request. If the value of this
 *  field is from 0 to (linksToDevicesNumber - 1), then the row belongs to the cluster of
 *  the type link-to-device. Otherwise the value is from the cluster of the type
 *  link-to-group, and the index inside that cluster equals to
 *  (lastReturnedLinkIdx - linksToDevicesNumber); it must not be greater than
 *  (linksToGroupsNumber - 1).
 * \details The initial value of this descriptor is all-zeroes. To reset the Binding
 *  Table, it is enough to reset only the \c currentState field to all zeroes.
 * \note To compare the stored request key with the new request key, apply mask to the
 *  \c lastSearchRequest.plain field in order to ignore the value of the \c peerCnt field.
 *  The field \c peerCnt is mapped on the most significant byte of the
 *  \c lastSearchRequest.plain field.
 */
typedef struct _ZbProApsBindingTableDescriptor_t
{
    /* Structured / 32-bit data. */
    union
    {
        uint32_t    plain;                  /*!< The plain 32-bit data. */
        struct
        {
            /* 8-bit data. */
            uint8_t linksToDevicesNumber;   /*!< The number of links to devices in the Binding Table. */
            uint8_t linksToGroupsNumber;    /*!< The number of links to groups in the Binding Table. */
            uint8_t lastReturnedLinkIdx;    /*!< The index of the last returned link
                                                from the search-by-the-key results set. */
            Bool8_t lastReturnedIsValid;    /*!< TRUE if the last returned link is still valid. */
        };
    } currentState;

    /* Structured / 32-bit data. */
    union
    {
        uint32_t                    plain;          /*!< The plain 32-bit data. */
        struct
        {
            /* 16-bit data. */
            ZBPRO_APS_ClusterId_t   clusterId;      /*!< The identifier of the cluster on the source device
                                                        that is bound to the destination device. */
            /* 8-bit data. */
            ZBPRO_APS_EndpointId_t  srcEndpoint;    /*!< The source endpoint of the binding entry. */

            uint8_t                 cntSkipPeer;    /*!< The number of peers passed for the previous request. */
        };
    } lastSearchRequest;

} ZbProApsBindingTableDescriptor_t;


/**//**
 * If this assert will be executed, it will mean that the type of the linksToGroupsNumber and linksToDevicesNumber
 * from ZbProApsBindingTableDescriptor_t must be changed to a bigger one.
*/
SYS_DbgAssertStatic(ZBPRO_APS_BINDING_TABLE_SIZE_LINKS_TO_GROUPS + ZBPRO_APS_BINDING_TABLE_SIZE_LINKS_TO_DEVICES <= UINT8_MAX);


/**//**
 * \brief Mask for the source of a link.
 * \details This mask is to be applied to the \c plain field of the
 *  \c ZbProApsBindingTableLinkSrc_t data structure, or to the \c plain field of the
 *  \c ZbProApsBindingTableDescriptor_t.lastSearchRequest data structure.
 */
#define ZBPRO_APS_BINDING_TABLE_LINK_SRC_MASK   0x00FFFFFF


#endif /* _ZBPRO_APS_BINDING_TABLE_DESCR_H */
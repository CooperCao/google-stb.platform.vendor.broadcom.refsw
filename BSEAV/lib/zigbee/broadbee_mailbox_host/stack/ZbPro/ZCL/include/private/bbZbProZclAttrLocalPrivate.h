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
* FILENAME: $Workfile$
*
* DESCRIPTION:
*   ZCL Attribute interface
*
* $Revision$
* $Date$
*
*****************************************************************************************/

#ifndef _BB_ZBPRO_ZCL_ATTR_LOCAL_PRIVATE_H
#define _BB_ZBPRO_ZCL_ATTR_LOCAL_PRIVATE_H

/************************* INCLUDES *****************************************************/
#include "bbZbProZclAttrLocal.h"
#include "private/bbZbProZclDispatcher.h"

/************************* DEFINITIONS **************************************************/

/**//**
 * \brief Max number of enpoints to be registred
 */
#define ZBPRO_ZCL_ATTR_ENDPOINTS_NUM    ZBPRO_APS_NODE_ENDPOINTS

/**//**
 * \brief Macro to declare attribute flag mask
 */
#define ZBPRO_ZCL_ATTR_FLAG_MASK_DECLARE(srcPrivate, srcHost, srcAir, special) \
        ((srcPrivate) | ((srcHost) << 8) | ((srcAir) << 16) | ((special) << 24))

/************************************************************************************//**
 \brief Attribute Access function shortcuts

 \param[in] endpoint    - endpoint Id
 \param[in] cluster     - cluster Id
 \param[in] attrId      - attribute Id
 \param[in] value       - pointer to calling function buffer
 ****************************************************************************************/
#define ZBPRO_ZCL_ATTR_PRIVATE_READ(endpoint, cluster, attrId, value) \
    zbProZclAttrAccess(ZBPRO_ZCL_ATTR_ACCESS_MODE_READ, ZBPRO_ZCL_ATTR_ACCESS_SRC_PRIVATE, \
        endpoint, cluster, attrId, value)
#define ZBPRO_ZCL_ATTR_PRIVATE_WRITE(endpoint, cluster, attrId, value) \
    zbProZclAttrAccess(ZBPRO_ZCL_ATTR_ACCESS_MODE_WRITE, ZBPRO_ZCL_ATTR_ACCESS_SRC_PRIVATE, \
        endpoint, cluster, attrId, value)
#define ZBPRO_ZCL_ATTR_HOST_READ(endpoint, cluster, attrId, value) \
    zbProZclAttrAccess(ZBPRO_ZCL_ATTR_ACCESS_MODE_READ, ZBPRO_ZCL_ATTR_ACCESS_SRC_HOST, \
        endpoint, cluster, attrId, value)
#define ZBPRO_ZCL_ATTR_HOST_WRITE(endpoint, cluster, attrId, value) \
    zbProZclAttrAccess(ZBPRO_ZCL_ATTR_ACCESS_MODE_WRITE, ZBPRO_ZCL_ATTR_ACCESS_SRC_HOST, \
        endpoint, cluster, attrId, value)
#define ZBPRO_ZCL_ATTR_AIR_READ(endpoint, cluster, attrId, value) \
    zbProZclAttrAccess(ZBPRO_ZCL_ATTR_ACCESS_MODE_READ, ZBPRO_ZCL_ATTR_ACCESS_SRC_AIR, \
        endpoint, cluster, attrId, value)
#define ZBPRO_ZCL_ATTR_AIR_WRITE(endpoint, cluster, attrId, value) \
    zbProZclAttrAccess(ZBPRO_ZCL_ATTR_ACCESS_MODE_WRITE, ZBPRO_ZCL_ATTR_ACCESS_SRC_AIR, \
        endpoint, cluster, attrId, value)


/**//**
 * \brief   Data type for ZCL Attribute Unique Identifier numeric value.
 * \details
 *  ZCL Attribute is uniquely identified by set of the following fields:
 *  - Cluster Id            numeric 16-bit value of Cluster Identifier,
 *  - Cluster Side          either Client (0x0) or Server (0x1),
 *  - Attributes Domain     either Manufacturer Specific (0x1) or ZCL Standard (0x0),
 *  - Manufacturer Code     numeric 16-bit value of Manufacturer Code, for the case of
 *      Manufacturer-Specific attribute; not used (assigned with zero) for the case of ZCL
 *      Standard attributes,
 *  - Attribute Id          numeric 16-bit value of Attribute Identifier local to
 *      particular Cluster, Side, and Manufacturer.
 *
 * \details
 *  All the listed fields are packed into plain 64-bit integer value to make Attribute Uid
 *  in the following order:
 *  - Manufacturer Code     occupies bits #63-48,
 *  - Cluster Id            occupies bits #47-32,
 *  - Frame Domain          occupies bit #24,
 *  - Cluster Side          occupies bit #16,
 *  - Attribute Id          occupies bits #15-0.
 *
 *  All the rest bits are assigned with zero.
 * \details
 *  Attributes are registered in relation to service that holds them. For example, if an
 *  attribute is held by the server side of a cluster, and requested remotely by the
 *  client side, then such an attribute shall be registered in relation with the server
 *  side service.
 */
typedef uint64_t  ZbProZclAttributeUid_t;


/**//**
 * \brief   Assembles Attribute Uid directly from distinct fields.
 * \param[in]   clusterId       Numeric 16-bit value of Cluster Identifier.
 * \param[in]   clusterSide     Either Client (0x0) or Server (0x1).
 * \param[in]   attrDomain      Either Manufacturer Specific (0x1) or ZCL Standard (0x0).
 * \param[in]   manufCode       Numeric 16-bit value of Manufacturer Code.
 * \param[in]   attributeId     Numeric 16-bit value of Attribute Identifier local to
 *  particular Cluster, Side, and Manufacturer.
 * \return  Numeric 64-bit value of Attribute Uid.
 */
#define ZBPRO_ZCL_MAKE_ATTRIBUTE_UID(clusterId, clusterSide, attrDomain, manufCode, attributeId)\
        (((ZBPRO_ZCL_FRAME_DOMAIN_ZCL_STANDARD != (attrDomain))\
                ? (0x0000000001000000 | (((ZbProZclAttributeUid_t)(manufCode)) << 48))\
                : 0x0000000000000000) |\
        (((ZbProZclAttributeUid_t)(clusterId)) << 32) |\
        (((ZbProZclAttributeUid_t)(clusterSide)) << 16) |\
        ((ZbProZclAttributeUid_t)(attributeId)))
/*
 * Validate definition of general case Attribute Uid assembling macro.
 */
SYS_DbgAssertStatic(0xEEEECCCC0101AAAA == ZBPRO_ZCL_MAKE_ATTRIBUTE_UID(
        /*clusterId*/ 0xCCCC, ZBPRO_ZCL_CLUSTER_SIDE_SERVER,
        ZBPRO_ZCL_FRAME_DOMAIN_MANUF_SPECIFIC, /*manufCode*/ 0xEEEE,
        /*attributeId*/ 0xAAAA));
SYS_DbgAssertStatic(0xEEEECCCC0100AAAA == ZBPRO_ZCL_MAKE_ATTRIBUTE_UID(
        /*clusterId*/ 0xCCCC, ZBPRO_ZCL_CLUSTER_SIDE_CLIENT,
        ZBPRO_ZCL_FRAME_DOMAIN_MANUF_SPECIFIC, /*manufCode*/ 0xEEEE,
        /*attributeId*/ 0xAAAA));
SYS_DbgAssertStatic(0x0000CCCC0001AAAA == ZBPRO_ZCL_MAKE_ATTRIBUTE_UID(
        /*clusterId*/ 0xCCCC, ZBPRO_ZCL_CLUSTER_SIDE_SERVER,
        ZBPRO_ZCL_FRAME_DOMAIN_ZCL_STANDARD, /*manufCode*/ 0xEEEE,
        /*attributeId*/ 0xAAAA));
SYS_DbgAssertStatic(0x0000CCCC0000AAAA == ZBPRO_ZCL_MAKE_ATTRIBUTE_UID(
        /*clusterId*/ 0xCCCC, ZBPRO_ZCL_CLUSTER_SIDE_CLIENT,
        ZBPRO_ZCL_FRAME_DOMAIN_ZCL_STANDARD, /*manufCode*/ 0xEEEE,
        /*attributeId*/ 0xAAAA));


/**//**
 * \brief   Extracts Attribute Id 16-bit field from Attribute Uid.
 * \param[in]   attributeUid    Numeric 64-bit value of Attribute Uid.
 * \return  Numeric 16-bit value of Attribute Id local to particular Cluster, Side and
 *  Manufacturer.
 */
#define ZBPRO_ZCL_EXTRACT_ATTRIBUTE_ID(attributeUid)\
        ((uint32_t)((attributeUid) & 0xFFFF))
/*
 * Validate definition of the Attribute Id extracting macro.
 */
SYS_DbgAssertStatic(0x0000AAAA == ZBPRO_ZCL_EXTRACT_ATTRIBUTE_ID(/*attributeUid*/ 0xFFFFFFFFFFFFAAAA));


/************************* TYPES *******************************************************/

/**//**
 * \brief Declaration of the attribute variables
 */
#define ZBPRO_ZCL_ATTR_DECLARE(clusterId, attrId, ZBPRO_ZCL_AttrDataType, actualType, defaultValue, zbProZclAttrFlagMask, accessFunc) \
        actualType var##attrId;
typedef struct _ZbProZclAttrVariables_t
{
    ZBPRO_ZCL_ATTR_DECLARATIONS
} ZbProZclAttrVariables_t;
#undef ZBPRO_ZCL_ATTR_DECLARE

/**//**
 * \brief Attributes storage type
 */
typedef ZbProZclAttrVariables_t ZbProZclAttrStorage_t[ZBPRO_ZCL_ATTR_ENDPOINTS_NUM];

/**//**
 * \brief Attribute access mode
 */
typedef enum _ZbProZclAttrAccessMode_t
{
    ZBPRO_ZCL_ATTR_ACCESS_MODE_READ,
    ZBPRO_ZCL_ATTR_ACCESS_MODE_WRITE,
    ZBPRO_ZCL_ATTR_ACCESS_MODE_LAST
} ZbProZclAttrAccessMode_t;

/**//**
 * \brief Attribute access status
 */
typedef enum _ZbProZclAttrAccessStatus_t
{
    ZBPRO_ZCL_ATTR_STATUS_SUCCESS                   = ZBPRO_ZCL_SUCCESS,
    ZBPRO_ZCL_ATTR_STATUS_UNSUPPORTED_ATTRIBUTE     = ZBPRO_ZCL_UNSUPPORTED_ATTRIBUTE,
    ZBPRO_ZCL_ATTR_STATUS_INVALID_DATA_TYPE         = ZBPRO_ZCL_INVALID_DATA_TYPE,
    ZBPRO_ZCL_ATTR_STATUS_NOT_AUTHORIZED            = ZBPRO_ZCL_NOT_AUTHORIZED,
    ZBPRO_ZCL_ATTR_STATUS_INVALID_VALUE             = ZBPRO_ZCL_INVALID_VALUE,
    ZBPRO_ZCL_ATTR_STATUS_READ_ONLY                 = ZBPRO_ZCL_READ_ONLY,
    ZBPRO_ZCL_ATTR_STATUS_WRITE_ONLY                = ZBPRO_ZCL_WRITE_ONLY,

    /* flags available only for private and host access */
    ZBPRO_ZCL_ATTR_STATUS_WRONG_ENDPOINT,
} ZbProZclAttrAccessStatus_t;

/**//**
 * \brief Attribute access source
 */
typedef enum _ZbProZclAttrAccessSrc_t
{
    ZBPRO_ZCL_ATTR_ACCESS_SRC_PRIVATE,
    ZBPRO_ZCL_ATTR_ACCESS_SRC_HOST,
    ZBPRO_ZCL_ATTR_ACCESS_SRC_AIR,
    ZBPRO_ZCL_ATTR_ACCESS_SRC_LAST
} ZbProZclAttrAccessSrc_t;

/**//**
 * \brief Flag bytes Ids
 */
typedef enum _ZbProZclAttrFlagByteId_t
{
    ZBPRO_ZCL_ATTR_FLAG_BYTE_ID_ACCESS_SRC_PRIVATE  = ZBPRO_ZCL_ATTR_ACCESS_SRC_PRIVATE,
    ZBPRO_ZCL_ATTR_FLAG_BYTE_ID_ACCESS_SRC_HOST     = ZBPRO_ZCL_ATTR_ACCESS_SRC_HOST,
    ZBPRO_ZCL_ATTR_FLAG_BYTE_ID_ACCESS_SRC_AIR      = ZBPRO_ZCL_ATTR_ACCESS_SRC_AIR,
    ZBPRO_ZCL_ATTR_FLAG_BYTE_ID_SPECIAL,
    ZBPRO_ZCL_ATTR_FLAG_BYTE_ID_LAST
} ZbProZclAttrFlagByteId_t;

/**//**
 * \brief Flag Mask array
 */
typedef uint8_t ZbProZclAttrFlagMaskArray_t[ZBPRO_ZCL_ATTR_FLAG_BYTE_ID_LAST];

/**//**
 * \brief Flag Mask
 */
typedef union _ZbProZclAttrFlagMask_t
{
    uint32_t                        plain;
    ZbProZclAttrFlagMaskArray_t     array;
} ZbProZclAttrFlagMask_t;
SYS_DbgAssertStatic(ZBPRO_ZCL_ATTR_FLAG_BYTE_ID_LAST == FIELD_SIZE(ZbProZclAttrFlagMask_t, plain));

/**//**
 * \brief Flag Ids
 */
typedef enum _ZbProZclAttrFlagId_t
{
    /* ZbProZclAttrAccessSrc_t dependent flags */
    ZBPRO_ZCL_ATTR_FLAG_ID_PERMISSION_READ          = ZBPRO_ZCL_ATTR_ACCESS_MODE_READ,
    ZBPRO_ZCL_ATTR_FLAG_ID_PERMISSION_WRITE         = ZBPRO_ZCL_ATTR_ACCESS_MODE_WRITE,
    ZBPRO_ZCL_ATTR_FLAG_ID_HOOK_READ,
    ZBPRO_ZCL_ATTR_FLAG_ID_HOOK_WRITE,

    /* special flags */
    ZBPRO_ZCL_ATTR_FLAG_ID_ATTR_PER_DEVICE,
    ZBPRO_ZCL_ATTR_FLAG_ID_LAST
} ZbProZclAttrFlagId_t;
SYS_DbgAssertStatic(ZBPRO_ZCL_ATTR_FLAG_ID_LAST <= CHAR_BIT);

/**//**
 * \brief Flag Bits
 */
typedef enum _ZbProZclAttrFlags_t
{
    ZBPRO_ZCL_ATTR_FLAG_BIT_PERMISSION_READ     = BIT(ZBPRO_ZCL_ATTR_FLAG_ID_PERMISSION_READ),
    ZBPRO_ZCL_ATTR_FLAG_BIT_PERMISSION_WRITE    = BIT(ZBPRO_ZCL_ATTR_FLAG_ID_PERMISSION_WRITE),
    ZBPRO_ZCL_ATTR_FLAG_BIT_HOOK_READ           = BIT(ZBPRO_ZCL_ATTR_FLAG_ID_HOOK_READ),
    ZBPRO_ZCL_ATTR_FLAG_BIT_HOOK_WRITE          = BIT(ZBPRO_ZCL_ATTR_FLAG_ID_HOOK_WRITE),
    ZBPRO_ZCL_ATTR_FLAG_BIT_PER_DEVICE          = BIT(ZBPRO_ZCL_ATTR_FLAG_ID_ATTR_PER_DEVICE),
} ZbProZclAttrFlags_t;

/**//**
 * \brief Dedicated attribute access function callback
 */
typedef ZbProZclAttrAccessStatus_t ZbProZclAttrAccess_t(
        ZbProZclAttrAccessMode_t    mode,
        ZbProZclAttrAccessSrc_t     srcType,
        ZBPRO_APS_EndpointId_t      endpoint,
        ZBPRO_ZCL_ClusterId_t       cluster,
        ZBPRO_ZCL_AttributeId_t     attrId,
        ZbProZclAttrValue_t         *value);


/************************* FUNCTIONS PROTOTYPES ****************************************/

/************************************************************************************//**
 \brief Attribute access function

 \param[in] mode        - read / write
 \param[in] srcType     - private/ host / air
 \param[in] endpoint    - endpoint Id
 \param[in] cluster     - cluster Id
 \param[in] attrId      - attribute Id
 \param[in] value       - pointer to calling function buffer

 \return status of the operation
 ****************************************************************************************/
ZbProZclAttrAccessStatus_t zbProZclAttrAccess(
        ZbProZclAttrAccessMode_t    mode,
        ZbProZclAttrAccessSrc_t     srcType,
        ZBPRO_APS_EndpointId_t      endpoint,
        ZBPRO_ZCL_ClusterId_t       cluster,
        ZBPRO_ZCL_AttributeId_t     attrId,
        ZbProZclAttrValue_t         *value);

/************************************************************************************//**
 \brief Initializes the attributes with their default values
 ****************************************************************************************/
void zbProZclAttrInit(void);


/**//**
 * \brief   Returns data type and value size of the specified attribute, if allowed, and
 *  status of attempt to read the attribute.
 * \param[in]   attributeUid        Numeric 64-bit value of attribute unique identifier.
 * \param[out]  attrDataType        Pointer to variable to accept data type of the
 *  attribute value.
 * \param[out]  attrValueSize       Pointer to variable to accept size of the attribute
 *  value, in bytes.
 * \return  Status of attempt to read attribute, one of ZCL legacy statuses.
 * \details
 *  This function doesn't read the attribute but does all the preparation work for it. It
 *  discovers if the specified attribute is implemented and may be accessed, and returns
 *  its data type and size of the current value. In order to read the attribute the caller
 *  shall allocate buffer of size returned in \p attrValueSize and call dedicated
 *  read-attribute function. Following statuses may be returned:
 *  - SUCCESS                   if attribute may be read successfully,
 *  - UNSUPPORTED_ATTRIBUTE     if attribute with such identifier is not supported,
 *  - WRITE_ONLY                if attribute is write-only,
 *  - NOT_AUTHORIZED            if requester has no rights to read this attribute value.
 */
ZBPRO_ZCL_Status_t  zbProZclAttrLocalTryRead(
                const ZbProZclAttributeUid_t     attributeUid,
                ZBPRO_ZCL_AttrDataType_t *const  attrDataType,
                SYS_DataLength_t *const          attrValueSize);


/**//**
 * \brief   Returns value of the requested attribute.
 * \param[in]   endpoint            Endpoint holding instance of ZCL-based profile which
 *  attribute was requested.
 * \param[in]   attributeUid        Numeric 64-bit value of attribute unique identifier.
 * \param[out]  attrValue           Buffer for the attribute value.
 * \param[in]   attrValueSize       Exact size of the attribute value.
 * \details
 *  This function is supposed to execute successfully. No status is returned by it.
 */
void zbProZclAttrLocalRead(
                const ZBPRO_APS_EndpointId_t  endpoint,
                const ZbProZclAttributeUid_t  attributeUid,
                uint8_t *const                attrValue,
                const SYS_DataLength_t        attrValueSize);


/**//**
 * \brief   Tries to perform attributes value assignment.
 * \param[in]       endpoint        Endpoint holding instance of ZCL-based profile which
 *  attribute was requested.
 * \param[in]       attributeUid    Numeric 64-bit value of attribute unique identifier.
 * \param[in]       attrDataType    Identifier of attribute data type.
 * \param[in/out]   bufferCursor    Pointer to Cursor to the deserialization buffer.
 * \param[in/out]   remainder       Pointer to counter of remaining bytes to deserialize.
 * \return  Status of attempt to write attribute, one of ZCL legacy statuses.
 * \details
 *  Prior to perform assignment this function validates all the given parameters. In the
 *  case of failure unsuccessful status is returned. Following statuses may be returned:
 *  - SUCCESS                   if attribute was written successfully,
 *  - INVALID_FIELD             if invalid (or unhandled) data type is specified for the
 *      attribute, or if the remaining part of the received payload is shorter than
 *      necessary to reconstruct an attribute value of the specified data type,
 *  - UNSUPPORTED_ATTRIBUTE     if attribute with such identifier is not supported,
 *  - INVALID_DATA_TYPE         if data type does not coincide exactly with the type of
 *      attribute,
 *  - INVALID_VALUE             if specified value is out of the allowed range,
 *  - READ_ONLY                 if attribute is read-only,
 *  - NOT_AUTHORIZED            if requester has no rights to assign this attribute value.
 *
 *  Values pointed by \p bufferCursor and \p remainder are properly updated. Cursor is
 *  shifted forward to the number of deserialized bytes, and the remainder is decremented
 *  by the same value.
 */
ZBPRO_ZCL_Status_t  zbProZclAttrLocalTryWrite(
                const ZBPRO_APS_EndpointId_t    endpoint,
                const ZbProZclAttributeUid_t    attributeUid,
                const ZBPRO_ZCL_AttrDataType_t  attrDataType,
                const uint8_t * *const          bufferCursor,
                SYS_DataLength_t *const         remaider);


#endif /* _BB_ZBPRO_ZCL_ATTR_LOCAL_PRIVATE_H */

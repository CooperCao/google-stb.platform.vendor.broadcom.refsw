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

/******************************************************************************
*
* DESCRIPTION:
*       ZCL Profile-Wide for attributes accessing SAP interface.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_ZCL_SAP_PROFILE_WIDE_ATTRIBUTES_H
#define _BB_ZBPRO_ZCL_SAP_PROFILE_WIDE_ATTRIBUTES_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZclCommon.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration of attribute data types.
 * \ingroup ZBPRO_ZCL_ProfileWideAttr
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.5.2, table 2-9.
 */
typedef enum _ZBPRO_ZCL_AttrDataType_t
{
    /* Null. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_NO_DATA        = 0x00,     /*!< No data. */

    /*                               0x01 ... 0x07         < reserved. */

    /* General data. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_DATA_8BIT      = 0x08,     /*!< 8-bit data. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_DATA_16BIT     = 0x09,     /*!< 16-bit data. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_DATA_24BIT     = 0x0A,     /*!< 24-bit data. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_DATA_32BIT     = 0x0B,     /*!< 32-bit data. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_DATA_40BIT     = 0x0C,     /*!< 40-bit data. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_DATA_48BIT     = 0x0D,     /*!< 48-bit data. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_DATA_56BIT     = 0x0E,     /*!< 56-bit data. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_DATA_64BIT     = 0x0F,     /*!< 64-bit data. */

    /* Logical. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_BOOLEAN        = 0x10,     /*!< Boolean. */

    /*                               0x11 ... 0x17         < reserved. */

    /* Bitmap. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_BITMAP_8BIT    = 0x18,     /*!< 8-bit bitmap. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_BITMAP_16BIT   = 0x19,     /*!< 16-bit bitmap. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_BITMAP_24BIT   = 0x1A,     /*!< 24-bit bitmap. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_BITMAP_32BIT   = 0x1B,     /*!< 32-bit bitmap. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_BITMAP_40BIT   = 0x1C,     /*!< 40-bit bitmap. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_BITMAP_48BIT   = 0x1D,     /*!< 48-bit bitmap. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_BITMAP_56BIT   = 0x1E,     /*!< 56-bit bitmap. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_BITMAP_64BIT   = 0x1F,     /*!< 64-bit bitmap. */

    /* Unsigned integer. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_UINT_8BIT      = 0x20,     /*!< Unsigned 8-bit integer. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_UINT_16BIT     = 0x21,     /*!< Unsigned 16-bit integer. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_UINT_24BIT     = 0x22,     /*!< Unsigned 24-bit integer. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_UINT_32BIT     = 0x23,     /*!< Unsigned 32-bit integer. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_UINT_40BIT     = 0x24,     /*!< Unsigned 40-bit integer. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_UINT_48BIT     = 0x25,     /*!< Unsigned 48-bit integer. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_UINT_56BIT     = 0x26,     /*!< Unsigned 56-bit integer. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_UINT_64BIT     = 0x27,     /*!< Unsigned 64-bit integer. */

    /* Signed integer. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_SINT_8BIT      = 0x28,     /*!< Signed 8-bit integer. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_SINT_16BIT     = 0x29,     /*!< Signed 16-bit integer. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_SINT_24BIT     = 0x2A,     /*!< Signed 24-bit integer. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_SINT_32BIT     = 0x2B,     /*!< Signed 32-bit integer. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_SINT_40BIT     = 0x2C,     /*!< Signed 40-bit integer. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_SINT_48BIT     = 0x2D,     /*!< Signed 48-bit integer. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_SINT_56BIT     = 0x2E,     /*!< Signed 56-bit integer. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_SINT_64BIT     = 0x2F,     /*!< Signed 64-bit integer. */

    /* Enumeration. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_ENUM_8BIT      = 0x30,     /*!< 8-bit enumeration. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_ENUM_16BIT     = 0x31,     /*!< 16-bit enumeration. */

    /*                               0x32 ... 0x37         < reserved. */

    /* Floating point. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_FP_SEMI        = 0x38,     /*!< Semi-precision. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_FP_SINGLE      = 0x39,     /*!< Single precision. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_FP_DOUBLE      = 0x3A,     /*!< Double precision. */

    /*                               0x3B ... 0x3F         < reserved. */

    /* String. */

    /*                                        0x40         < reserved. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_OSTR           = 0x41,     /*!< Octet string. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_CSTR           = 0x42,     /*!< Character string. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_OSTR_LONG      = 0x43,     /*!< Long octet string. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_CSTR_LONG      = 0x44,     /*!< Long character string. */

    /*                               0x45 ... 0x47         < reserved. */

    /* Ordered sequence. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_ARRAY          = 0x48,     /*!< Array. */

    /*                               0x49 ... 0x4B         < reserved. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_STRUCT         = 0x4C,     /*!< Structure. */

    /*                               0x4D ... 0x4F         < reserved. */

    /* Collection. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_SET            = 0x50,     /*!< Set. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_BAG            = 0x51,     /*!< Bag. */

    /*                               0x52 ... 0x57         < reserved. */

    /* Reserved. */

    /*                               0x58 ... 0xDF         < reserved. */

    /* Time. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_TIME           = 0xE0,     /*!< Time of day. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_DATE           = 0xE1,     /*!< Date. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_UTC            = 0xE2,     /*!< UTCTime. */

    /*                               0xE3 ... 0xE7         < reserved. */

    /* Identifier. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_CLUSTER_ID     = 0xE8,     /*!< Cluster ID. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_ATTR_ID        = 0xE9,     /*!< Attribute ID. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_BACNET_OID     = 0xEA,     /*!< BACnet OID. */

    /*                               0xEB ... 0xEF         < reserved. */

    /* Miscellaneous. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_IEEE           = 0xF0,     /*!< IEEE address. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_KEY            = 0xF1,     /*!< 128-bit security key. */

    /*                               0xF2 ... 0xFE         < reserved. */

    /* Unknown. */

    ZBPRO_ZCL_ATTR_DATA_TYPE_UNKNOWN        = 0xFF,     /*!< Unknown. */

} ZBPRO_ZCL_AttrDataType_t;


/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Read Attributes
 *  profile-wide command.
 * \ingroup ZBPRO_ZCL_ReadAttrReq
 * \note
 *  Only one parameter may be requested in the same command. Requesting multiple
 *  parameters simultaneously within one command is not supported.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.4.1, figure 2-5.
 */
typedef struct _ZBPRO_ZCL_ProfileWideCmdReadAttrReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;      /*!< Set of obligatory parameters of ZCL public
                                                                        interface to local application. */
    /* Custom parameters. */

    /* 16-bit data. */
#ifdef MULTIPLE_ATTRIBUTES_READ
    uint16_t                                attributeNumber;        /*!< Number of the attributes that are to be read. */
    ZBPRO_ZCL_AttributeId_t                 attributeId[40];        /*!< Identifier array of the attribute that are to be read. */
#else
    ZBPRO_ZCL_AttributeId_t                 attributeId;            /*!< Identifier of the attribute that is to be
                                                                        read. */
#endif
} ZBPRO_ZCL_ProfileWideCmdReadAttrReqParams_t;


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Read
 *  Attributes profile-wide command.
 * \ingroup ZBPRO_ZCL_ReadAttrConf
 * \details
 *  This structure takes its origin from ZCL Read Attributes Response profile-wide
 *  command.
 * \note
 *  Only one parameter may be received in the same command. Receiving multiple parameters
 *  simultaneously within one command is not supported.
 * \details
 *  If a Default Response was received instead of the Read Attributes Response, then the
 *  original Local Request to read attributes is confirmed with following parameters:
 *  \c Status is set after the Status received in such Default Response command,
 *  \c AttrDataType is set to NO_DATA, and \c payload is set to empty.
 * \details
 *  If a Read Attributes Response was received, it is processed against the following set
 *  of validation rules:
 *  - ZCL payload must contain at least the Attribute Identifier field (2 octets) and the
 *      Status field (1 octet),
 *  - the Attribute Identifier field must be equal to the same field of the original
 *      request - i.e., the attribute received in the response must be the one that was
 *      initially requested,
 *  - if Status field equals to SUCCESS, then at least the Attribute Data Type field
 *      (1 octet) must be presented in the ZCL payload.
 *
 *  If these rules are violated, then the received command is rejected, Default Response
 *  is sent back to the response generator node with status MALFORMED_COMMAND (if it is
 *  not disabled), and the original Local Request to read attributes is confirmed with
 *  following parameters: \c Status is set to FAILURE, \c AttrDataType is set to NO_DATA,
 *  and \c payload is set to empty.
 *
 *  If these rules are held, then the received command is accepted, Default Response is
 *  sent back to the response generator node with status SUCCESS (if it is not disabled),
 *  and the original Local Request to read attributes is confirmed with following
 *  parameters: \c Status is set after the Status field of the received Read Attributes
 *  Response command, and then depending on the status either (a) for SUCCESS status
 *  \c AttrDataType and \c payload are also set after corresponding fields of the received
 *  response, or for unsuccessful status \c AttrDataType is set to NO_DATA, and \c payload
 *  is set to empty.
 * \note
 *  Value of \c AttrDataType field of the received response is not validated against the
 *  list of allowed (nonreserved) data types. Even if a reserved (or unsupported) data
 *  type was specified in the received response, the command is successfully processed and
 *  the original Local Request is confirmed; Default Response is sent to the response
 *  generator with the SUCCEESS status (if it is not disabled).
 *
 *  Size, value and internal structure of \c AttributeValue field of the received response
 *  are not validated against the attribute data type and other rules. This field is saved
 *  into the \c payload parameter as-is - it means that all the remaining octets in the
 *  ZCL payload of the received response following the first \c AttrDataType field are
 *  considered as the Attribute Value; the command is successfully processed and the
 *  original Local Request is confirmed; Default Response is sent to the response
 *  generator with the SUCCEESS status (if it is not disabled).
 *
 *  Consequently, in all cases the higher-level layer (the application) that originally
 *  issued the Local Request to read attributes on a remote node shall validate the
 *  confirmed attribute data type and the attribute value (given in \c payload parameter).
 * \details
 *  Confirmation handler function in general shall dismiss the object described by the
 *  \c payload; it also may keep this payload object alive for further processing but
 *  finally it must be dismissed by a different task arranged by this confirmation
 *  handler; ZCL layer will not take care of this payload object.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.4.2, figures 2-6, 2-7.
 */
typedef struct _ZBPRO_ZCL_ProfileWideCmdReadAttrConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;      /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* Custom parameters. */

    /* 32-bit data. */

    SYS_DataPointer_t                       payload;                /*!< Current value of the requested attribute. */

    /* 8-bit data. */

    ZBPRO_ZCL_AttrDataType_t                attrDataType;           /*!< Data type of the attribute. */

} ZBPRO_ZCL_ProfileWideCmdReadAttrConfParams_t;

/*
 * Validate structures of ZCL Local Request and Confirm Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ProfileWideCmdReadAttrReqParams_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ProfileWideCmdReadAttrConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Read Attributes
 *  profile-wide command.
 * \ingroup ZBPRO_ZCL_ReadAttrReq
 */
typedef struct _ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t  ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Read Attributes
 *  profile-wide command.
 * \ingroup ZBPRO_ZCL_ReadAttrConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_ProfileWideCmdReadAttrConfCallback_t(
                ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t   *const  reqDescr,
                ZBPRO_ZCL_ProfileWideCmdReadAttrConfParams_t *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Read Attributes
 *  profile-wide command.
 * \ingroup ZBPRO_ZCL_ReadAttrReq
 */
struct _ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_ProfileWideCmdReadAttrConfCallback_t *callback;       /*!< ZCL Confirmation callback handler entry
                                                                        point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t            service;        /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_ProfileWideCmdReadAttrReqParams_t     params;         /*!< ZCL Request parameters structure. */
};

/*
 * Validate structures of ZCL Local Request Descriptors.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Indication of received Read Attributes
 *  profile-wide command.
 * \ingroup ZBPRO_ZCL_ReadAttrInd
 * \note
 *  Parsing of multiple Attribute Identifier array from a single payload must be performed
 *  by the handler.
 * \details
 *  The handler function assigned for this indication shall finally dismiss the
 *  \c payload; ZCL layer will not take care of this payload object.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.4.1, figure 2-5.
 */
typedef struct _ZBPRO_ZCL_ProfileWideCmdReadAttrIndParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;      /*!< Set of obligatory parameters of ZCL public
                                                                        interface to local application. */
    /* Custom parameters. */

    /* Structured / 32-bit data. */

    SYS_DataPointer_t                       payload;                /*!< Serialized array of Attribute Identifier
                                                                        fields. */
} ZBPRO_ZCL_ProfileWideCmdReadAttrIndParams_t;

/*
 * Validate structures of ZCL Local Request and Confirm Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ProfileWideCmdReadAttrIndParams_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Read Attributes
 *  Response profile-wide command.
 * \ingroup ZBPRO_ZCL_ReadAttrRespReq
 * \note
 *  Composing of multiple Read Attribute Status Records into a single payload must be
 *  performed by the caller.
 * \details
 *  The caller shall not dismiss the \c payload until confirmation is received on this
 *  request. But after reception of confirmation the caller shall free the \c payload;
 *  it also may keep this payload object alive for further processing but finally it must
 *  be dismissed by a different task arranged by such confirmation handler; ZCL layer will
 *  not take care of this payload object.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.4.2, figures 2-6, 2-7.
 */
typedef struct _ZBPRO_ZCL_ProfileWideCmdReadAttrResponseReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;      /*!< Set of obligatory parameters of ZCL public
                                                                        interface to local application. */
    /* Custom parameters. */

    /* Structured / 32-bit data. */

    SYS_DataPointer_t                       payload;                /*!< Serialized array of Read Attribute Status
                                                                        Records. */
} ZBPRO_ZCL_ProfileWideCmdReadAttrResponseReqParams_t;


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Read
 *  Attributes Response profile-wide command.
 * \details
 *  This structure takes its origin from ZCL Default Response profile-wide command.
 * \details
 *  Confirmation handler function in general shall dismiss the object described by the
 *  \c payload parameter of confirmed request; it also may keep this payload object alive
 *  for further processing but finally it must be dismissed by a different task arranged
 *  by this confirmation handler; ZCL layer will not take care of this payload object.
 * \ingroup ZBPRO_ZCL_ReadAttrRespConf
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.4.2, figures 2-6, 2-7.
 */
typedef struct _ZBPRO_ZCL_ProfileWideCmdReadAttrResponseConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;      /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* No custom parameters exist for this type of ZCL Local Confirm. */

} ZBPRO_ZCL_ProfileWideCmdReadAttrResponseConfParams_t;

/*
 * Validate structures of ZCL Local Request and Confirm Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ProfileWideCmdReadAttrResponseReqParams_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ProfileWideCmdReadAttrResponseConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Read Attributes
 *  Response profile-wide command.
 * \ingroup ZBPRO_ZCL_ReadAttrRespReq
 */
typedef struct _ZBPRO_ZCL_ProfileWideCmdReadAttrResponseReqDescr_t  ZBPRO_ZCL_ProfileWideCmdReadAttrResponseReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Read Attributes
 *  Response profile-wide command.
 * \ingroup ZBPRO_ZCL_ReadAttrRespConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_ProfileWideCmdReadAttrResponseConfCallback_t(
                ZBPRO_ZCL_ProfileWideCmdReadAttrResponseReqDescr_t   *const  reqDescr,
                ZBPRO_ZCL_ProfileWideCmdReadAttrResponseConfParams_t *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Read Attributes
 *  Response profile-wide command.
 * \ingroup ZBPRO_ZCL_ReadAttrRespReq
 */
struct _ZBPRO_ZCL_ProfileWideCmdReadAttrResponseReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_ProfileWideCmdReadAttrResponseConfCallback_t *callback;       /*!< ZCL Confirmation callback handler entry
                                                                                point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t                    service;        /*!< ZCL Request Descriptor service
                                                                                field. */

    ZBPRO_ZCL_ProfileWideCmdReadAttrResponseReqParams_t     params;         /*!< ZCL Request parameters structure. */
};

/*
 * Validate structures of ZCL Local Request Descriptors.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_ProfileWideCmdReadAttrResponseReqDescr_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Write Attributes
 *  profile-wide command.
 * \ingroup ZBPRO_ZCL_WriteAttrReq
 * \note
 *  Only one parameter may be specified in the same command. Assigning multiple parameters
 *  simultaneously within one command is not supported.
 * \note
 *  The outgoing Write Attributes command is composed and sent as-is - i.e., ZCL layer
 *  doesn't validate specified \c AttributeId, \c AttrDataType and \c payload.
 * \details
 *  The caller shall not dismiss the \c payload until confirmation is received on this
 *  request. But after reception of confirmation the caller shall free the \c payload;
 *  it also may keep this payload object alive for further processing but finally it must
 *  be dismissed by a different task arranged by such confirmation handler; ZCL layer will
 *  not take care of this payload object.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.4.3, figures 2-10, 2-11.
 */
typedef struct _ZBPRO_ZCL_ProfileWideCmdWriteAttrReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;      /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* Custom parameters. */

    /* 32-bit data. */

    SYS_DataPointer_t                       payload;                /*!< Actual value of the attribute that is to be
                                                                        written. */
    /* 16-bit data. */

    ZBPRO_ZCL_AttributeId_t                 attributeId;            /*!< Identifier of the attribute that is to be
                                                                        written. */
    /* 8-bit data. */

    ZBPRO_ZCL_AttrDataType_t                attrDataType;           /*!< Data type of the attribute. */

} ZBPRO_ZCL_ProfileWideCmdWriteAttrReqParams_t;


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Write
 *  Attributes profile-wide command.
 * \ingroup ZBPRO_ZCL_WriteAttrConf
 * \details
 *  This structure takes its origin from ZCL Write Attributes Response profile-wide
 *  command.
 * \note
 *  Only one parameter may be specified in the same command. Assigning multiple parameters
 *  simultaneously within one command is not supported.
 * \details
 *  If a Default Response was received instead of the Write Attributes Response, then the
 *  original Local Request to write attributes is confirmed with \c Status set after the
 *  Status received in such Default Response command.
 * \details
 *  If a Write Attributes Response was received, it is processed against the following set
 *  of validation rules:
 *  - ZCL payload must contain at least the Status field (1 octet),
 *  - if Status field doesn't equal to SUCCESS, then at least the Attribute Identifier
 *      field (2 octets) must be presented in the ZCL payload,
 *  - and for the second case the Attribute Identifier field must be equal to the same
 *      field of the original request - i.e., the attribute confirmed in the response must
 *      be the one that was initially assigned.
 *
 *  If these rules are violated, then the received command is rejected, Default Response
 *  is sent back to the response generator node with status MALFORMED_COMMAND (if it is
 *  not disabled), and the original Local Request to write attributes is confirmed with
 *  \c Status set to FAILURE.
 *
 *  If these rules are held, then the received command is accepted, Default Response is
 *  sent back to the response generator node with status SUCCESS (if it is not disabled),
 *  and the original Local Request to write attributes is confirmed with \c Status set
 *  after the Status field of the received Write Attributes Response command.
 * \note
 *  The received response command is not validated if it has additional octets after the
 *  first Write Attribute Status Record (i.e., after the first Status field or after the
 *  first Attribute identifier field, depending on the status); if there are such octets,
 *  they are just ignored.
 * \details
 *  Confirmation handler function in general shall dismiss the object described by the
 *  \c payload parameter of confirmed request; it also may keep this payload object alive
 *  for further processing but finally it must be dismissed by a different task arranged
 *  by this confirmation handler; ZCL layer will not take care of this payload object.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.4.5, figures 2-12, 2-13.
 */
typedef struct _ZBPRO_ZCL_ProfileWideCmdWriteAttrConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;      /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* No custom parameters exist for this type of ZCL Local Confirm. */

} ZBPRO_ZCL_ProfileWideCmdWriteAttrConfParams_t;

/*
 * Validate structures of ZCL Local Request and Confirm Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ProfileWideCmdWriteAttrReqParams_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ProfileWideCmdWriteAttrConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Write Attributes
 *  profile-wide command.
 * \ingroup ZBPRO_ZCL_WriteAttrReq
 */
typedef struct _ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t  ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Write Attributes
 *  profile-wide command.
 * \ingroup ZBPRO_ZCL_WriteAttrConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_ProfileWideCmdWriteAttrConfCallback_t(
                ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t   *const  reqDescr,
                ZBPRO_ZCL_ProfileWideCmdWriteAttrConfParams_t *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Write Attributes
 *  profile-wide command.
 * \ingroup ZBPRO_ZCL_WriteAttrReq
 */
struct _ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_ProfileWideCmdWriteAttrConfCallback_t *callback;      /*!< ZCL Confirmation callback handler entry
                                                                        point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t             service;       /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_ProfileWideCmdWriteAttrReqParams_t     params;        /*!< ZCL Request parameters structure. */
};

/*
 * Validate structures of ZCL Local Request Descriptors.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Indication of received Write Attributes
 *  profile-wide command.
 * \ingroup ZBPRO_ZCL_WriteAttrInd
 * \note
 *  Parsing of multiple Write Attribute Records from a single payload must be performed by
 *  the handler.
 * \details
 *  The handler function assigned for this indication shall finally dismiss the
 *  \c payload; ZCL layer will not take care of this payload object.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.4.3, figures 2-10, 2-11.
 */
typedef struct _ZBPRO_ZCL_ProfileWideCmdWriteAttrIndParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;      /*!< Set of obligatory parameters of ZCL public
                                                                        interface to local application. */
    /* Custom parameters. */

    /* Structured / 32-bit data. */

    SYS_DataPointer_t                       payload;                /*!< Serialized array of Write Attribute Records. */

} ZBPRO_ZCL_ProfileWideCmdWriteAttrIndParams_t;

/*
 * Validate structures of ZCL Local Request and Confirm Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ProfileWideCmdWriteAttrIndParams_t);


/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Write Attributes
 *  Response profile-wide command.
 * \ingroup ZBPRO_ZCL_WriteAttrRespReq
 * \note
 *  Composing of multiple Write Attribute Status Records into a single payload must be
 *  performed by the caller.
 * \details
 *  The caller shall not dismiss the \c payload until confirmation is received on this
 *  request. But after reception of confirmation the caller shall free the \c payload;
 *  it also may keep this payload object alive for further processing but finally it must
 *  be dismissed by a different task arranged by such confirmation handler; ZCL layer will
 *  not take care of this payload object.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.4.5, figures 2-12, 2-13.
 */
typedef struct _ZBPRO_ZCL_ProfileWideCmdWriteAttrResponseReqParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;      /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* Custom parameters. */

    /* Structured / 32-bit data. */

    SYS_DataPointer_t                       payload;                /*!< Serialized array of Write Attribute Status
                                                                        Records. */
} ZBPRO_ZCL_ProfileWideCmdWriteAttrResponseReqParams_t;


/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Write
 *  Attributes Response profile-wide command.
 * \ingroup ZBPRO_ZCL_WriteAttrRespConf
 * \details
 *  This structure takes its origin from ZCL Default Response profile-wide command.
 * \details
 *  Confirmation handler function in general shall dismiss the object described by the
 *  \c payload parameter of confirmed request; it also may keep this payload object alive
 *  for further processing but finally it must be dismissed by a different task arranged
 *  by this confirmation handler; ZCL layer will not take care of this payload object.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclauses 2.4.5, figures 2-12, 2-13.
 */
typedef struct _ZBPRO_ZCL_ProfileWideCmdWriteAttrResponseConfParams_t
{
    /* Obligatory fields. Do not change the order. Custom parameters, if exist, must follow these fields. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;      /*!< Set of obligatory parameters of ZCL public
                                                                            interface to local application. */
    /* No custom parameters exist for this type of ZCL Local Confirm. */

} ZBPRO_ZCL_ProfileWideCmdWriteAttrResponseConfParams_t;

/*
 * Validate structures of ZCL Local Request and Confirm Parameters.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ProfileWideCmdWriteAttrResponseReqParams_t);
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_ProfileWideCmdWriteAttrResponseConfParams_t);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Write Attributes
 *  Response profile-wide command.
 * \ingroup ZBPRO_ZCL_WriteAttrRespReq
 */
typedef struct _ZBPRO_ZCL_ProfileWideCmdWriteAttrResponseReqDescr_t
                ZBPRO_ZCL_ProfileWideCmdWriteAttrResponseReqDescr_t;


/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Write Attributes
 *  Response profile-wide command.
 * \ingroup ZBPRO_ZCL_WriteAttrRespConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_ProfileWideCmdWriteAttrResponseConfCallback_t(
                ZBPRO_ZCL_ProfileWideCmdWriteAttrResponseReqDescr_t   *const  reqDescr,
                ZBPRO_ZCL_ProfileWideCmdWriteAttrResponseConfParams_t *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Write Attributes
 *  Response profile-wide command.
 * \ingroup ZBPRO_ZCL_WriteAttrRespReq
 */
struct _ZBPRO_ZCL_ProfileWideCmdWriteAttrResponseReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_ProfileWideCmdWriteAttrResponseConfCallback_t *callback;      /*!< ZCL Confirmation callback handler entry
                                                                                point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t                     service;       /*!< ZCL Request Descriptor service
                                                                                field. */

    ZBPRO_ZCL_ProfileWideCmdWriteAttrResponseReqParams_t     params;        /*!< ZCL Request parameters structure. */
};

/*
 * Validate structures of ZCL Local Request Descriptors.
 */
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_DESCR_STRUCT(ZBPRO_ZCL_ProfileWideCmdWriteAttrResponseReqDescr_t);


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   Functions accept ZCL Local Requests to issue profile-wide commands.
 * \details
 *  The caller shall specify the following obligatory parameters of request:
 *  - callback                      assign with ZCL Local Confirm handler function,
 *  - remoteApsAddress.addrMode     specify remote (destination) addressing mode,
 *  - clusterId                     specify cluster to which this command is applied,
 *  - respWaitTimeout               specify timeout of waiting for response, in seconds.
 *      Use value 0x0000 or 0xFFFF as well if default ZCL timeout shall be accepted. Note
 *      that the default ZCL timeout may be different for different commands,
 *  - localEndpoint                 specify endpoint on this node with ZCL-based profile
 *      which will be used as the source endpoint,
 *  - direction                     specify the direction of command, either Client-to-
 *      Server or Server-to-Client,
 *  - manufSpecific                 specify the domain of command, either Manufacturer-
 *      Specific or ZCL Standard,
 *  - disableDefaultResp            set to TRUE if ZCL Default Response is necessarily
 *      needed even for the case of successful command processing on the remote node; set
 *      to FALSE if it's enough to have ZCL Default Response only for the case of failure
 *      on the remote node.
 *
 * \details
 *  For the case when remote (destination) node is bound to this (source) node, one may
 *  set the Remote Addressing Mode to NOT_PRESENT and specify only the Local Endpoint and
 *  Cluster on it. APS layer will then assume Remote node Address (extended or group) and
 *  Endpoint corresponding to the specified Local Endpoint according to the Binding Table.
 *  Otherwise, for direct addressing mode, the caller shall also specify the following
 *  parameters:
 *  - remoteApsAddress      specify destination address (either short, extended or group),
 *  - remoteEndpoint        specify destination endpoint on the remote node with the same
 *      ZCL-based profile as for the Local Endpoint.
 *
 * \details
 *  Depending on some parameters other parameters may be either adopted by the command
 *  processor or ignored (substituted with default values). Here is the list:
 *  - manufCode             specify Manufacturer Code if \c manufSpecific is set to TRUE.
 *      For the case of ZCL Standard command (i.e., when \c manufSpecific equals to FALSE)
 *      this parameter may be left unassigned; it is ignored by ZCL Dispatcher and not
 *      included into the ZCL Frame.
 *
 * \details
 *  For the case of Local Request to send Solicited Response Command the caller shall
 *  additionally specify the following parameters:
 *  - transSeqNum           specify value of the same parameter that was reported with
 *      indication of received remote request command,
 *  - overallStatus         specify ZCL Responder application status; this status will be
 *      included into the response command. For some statuses the Default Response may be
 *      generated instead of the Read/Write Attributes Response,
 *  - nonUnicastRequest     specify value of the same parameter that was reported with
 *      indication of received remote request command.
 *
 *  For the case of Local Request to send Request command these parameters are ignored.
 * \details
 *  Following parameters are ignored even if specified by the caller and reassigned by
 *  this command handler:
 *  - localApsAddress       assigned by APS layer with this node address,
 *  - commandId             assigned to Command Id according to particular Local Request,
 *  - clusterSpecific       assigned to FALSE (i.e., Profile-Wide type),
 *  - useSpecifiedTsn       assigned to FALSE for request commands; assigned to TRUE for
 *      solicited response commands.
 */

/**//**
 * \brief   Accepts ZCL Local Request to issue Read Attributes profile-wide command.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_ProfileWideCmdReadAttributesReq(
                ZBPRO_ZCL_ProfileWideCmdReadAttrReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Read Attributes Response profile-wide
 *  command.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_ProfileWideCmdReadAttributesResponseReq(
                ZBPRO_ZCL_ProfileWideCmdReadAttrResponseReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Write Attributes profile-wide command.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_ProfileWideCmdWriteAttributesReq(
                ZBPRO_ZCL_ProfileWideCmdWriteAttrReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZCL Local Request to issue Write Attributes Response profile-wide
 *  command.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_ProfileWideCmdWriteAttributesResponseReq(
                ZBPRO_ZCL_ProfileWideCmdWriteAttrResponseReqDescr_t *const  reqDescr);


/**//**
 * \brief   Functions handle ZCL Local Indication on reception of Read Attribute and Write
 *  Attribute profile-wide commands.
 * \details
 *  The higher-level layer (application) or one of ZHA Managers shall define these
 *  callback handler functions.
 */

/**//**
 * \brief   Handles ZCL Local Indication on reception of Read Attribute profile-wide
 *  command.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 * \return Nothing.
 */
void ZBPRO_ZCL_ProfileWideCmdReadAttributesInd(
                ZBPRO_ZCL_ProfileWideCmdReadAttrIndParams_t *const  indParams);


/**//**
 * \brief   Handles ZCL Local Indication on reception of Write Attribute profile-wide
 *  command.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 * \return Nothing.
 */
void ZBPRO_ZCL_ProfileWideCmdWriteAttributesInd(
                ZBPRO_ZCL_ProfileWideCmdWriteAttrIndParams_t *const  indParams);


#endif /* _BB_ZBPRO_ZCL_SAP_PROFILE_WIDE_ATTRIBUTES_H */

/* eof bbZbProZclSapProfileWideAttributes.h */
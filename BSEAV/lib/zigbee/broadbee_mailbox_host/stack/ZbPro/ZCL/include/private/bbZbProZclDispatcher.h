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
*   ZCL Dispatcher private interface.
*
* $Revision$
* $Date$
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZCL_DISPATCHER_H
#define _BB_ZBPRO_ZCL_DISPATCHER_H


/************************* INCLUDES *****************************************************/
#include "private/bbZbProZclTransaction.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Data type for numeric 32-bit value of ZCL-based Profile Instance identifier.
 * \details
 *  Value of this type has 32-bit width. The 16-bit MSW contains the Profile Identifier;
 *  and the 16-bit LSW contains the Instance Identifier.
 * \note
 *  Currently each profile may have only one instance. Due to this reason instance
 *  identifier (the 16-bit LSW) must be assigned with zero.
 */
typedef uint32_t  ZbProZclProfileInstanceId_t;


/**//**
 * \brief   Structure for descriptor of ZCL-based Profile Dispatcher.
 */
typedef struct _ZbProZclDispatcher_t
{
    /* Structured / 32-bit data. */

    SYS_QueueElement_t           queueElement;              /*!< Queue element to link all ZCL-based profiles
                                                                descriptors into a single queue. */

    ZbProZclProfileInstanceId_t  profileInstanceId;         /*!< Identifier of profile and its instance. */

    /* Structured data, aligned at 32 bits. */

    RpcDispatcher_t              rpcDispatcherDescr;        /*!< Embedded RPC Dispatcher descriptor. */

} ZbProZclDispatcher_t;


/**//**
 * \brief   Data type for ZCL Service Unique Identifier numeric value.
 * \details
 *  ZCL Service is uniquely identified by set of the following fields:
 *  - Cluster Id        numeric 16-bit value of Cluster Identifier,
 *  - Cluster Side      either Client (0x0) or Server (0x1).
 *
 * \details
 *  All the listed fields are packed into plain 32-bit integer value to make Service Uid
 *  in the following order:
 *  - Direction             occupies bit #16,
 *  - Cluster Id            occupies bits #15-0.
 *
 *  All the rest bits are assigned with zero.
 */
typedef uint32_t  ZbProZclServiceUid_t;

/*
 * Validate size of the Service Uid data type.
 */
SYS_DbgAssertStatic(sizeof(ZbProZclServiceUid_t) == sizeof(RpcServiceId_t));


/**//**
 * \brief   Assembles Service Uid directly from distinct fields.
 * \param[in]   clusterSide     Either Client (0x0) or Server (0x1).
 * \param[in]   clusterId       Numeric 16-bit value of Cluster Identifier.
 * \return  Numeric 32-bit value of Service Uid.
 */
#define ZBPRO_ZCL_MAKE_SERVICE_UID(clusterSide, clusterId)\
        ((((ZbProZclServiceUid_t)(clusterSide)) << 16) |\
        ((ZbProZclServiceUid_t)(clusterId)))
/*
 * Validate definition of the Service Uid assembling macro.
 */
SYS_DbgAssertStatic(0x0001CCCC == ZBPRO_ZCL_MAKE_SERVICE_UID(
        ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT, /*clusterId*/ 0xCCCC));
SYS_DbgAssertStatic(0x0000CCCC == ZBPRO_ZCL_MAKE_SERVICE_UID(
        ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER, /*clusterId*/ 0xCCCC));


/**//**
 * \brief   Data type for ZCL Command Unique Identifier numeric value.
 * \details
 *  ZCL Command is uniquely identified by set of the following fields:
 *  - Frame Type            either Cluster Specific (0x1) or Profile Wide (0x0),
 *  - Cluster Id            numeric 16-bit value of Cluster Identifier, for the case of
 *      Cluster-Specific command; not used (assigned with zero) for the case of
 *      Profile-Wide command,
 *  - Direction             either Client-to-Server (0x0) or Server-to-Client (0x1), for
 *      the case of Cluster-Specific command; not used (assigned with zero) for the case
 *      of Profile-Wide command,
 *  - Frame Domain          either Manufacturer Specific (0x1) or ZCL Standard (0x0); not
 *      used (assigned with zero) for the case of Profile-Wide command,
 *  - Manufacturer Code     numeric 16-bit value of Manufacturer Code, for the case of
 *      Cluster-Specific Manufacturer-Specific command; not used (assigned with zero) for
 *      cases of ZCL Standard and Profile-Wide commands,
 *  - Command Id            numeric 8-bit value of Command Identifier local to particular
 *      Cluster, Side, and Manufacturer,
 *  - Disabled              either TRUE if service (and its cluster-specific command) is
 *      disabled or merely not implemented, or FALSE if it's implemented and enabled; not
 *      used (assigned with FALSE - i.e., enabled) for the case of Profile-Wide command.
 *
 * \details
 *  All the listed fields are packed into plain 64-bit integer value to make Command Uid
 *  in the following order:
 *  - Manufacturer Code     occupies bits #63-48,
 *  - Cluster Id            occupies bits #47-32,
 *  - Frame Domain          occupies bit #24,
 *  - Disabled              occupies bit #17,
 *  - Direction             occupies bit #16,
 *  - Frame Type            occupies bit #8,
 *  - Command Id            occupies bits #7-0.
 *
 *  All the rest bits are assigned with zero.
 * \details
 *  Profile-Wide commands acts across all the clusters and both their client and server
 *  sides, irrespectively of the Manufacturer Code (if the frame is Manufacturer
 *  Specific), and even in the case when the specified cluster is disabled on the local
 *  endpoint or even not implemented.
 * \note
 *  For the case of Profile-Wide Commands there Uids numeric values include only the
 *  CommandId field indeed, while all other fields become zero.
 */
typedef uint64_t  ZbProZclCommandUid_t;


/**//**
 * \brief   Assembles Prefix of Command Uid.
 * \param[in]   frameType       Either Cluster Specific (0x1) or Profile Wide (0x0).
 * \param[in]   clusterId       Numeric 16-bit value of Cluster Identifier.
 * \param[in]   frameDomain     Either Manufacturer Specific (0x1) or ZCL Standard (0x0).
 * \param[in]   manufCode       Numeric 16-bit value of Manufacturer Code.
 * \return  Numeric 64-bit value of Command Uid Prefix.
 * \details
 *  Command Uid Prefix may be assembled once per group of commands of a single cluster and
 *  reused in definitions of particular commands' Uids.
 * \details
 *  Prefix includes the following fields of Command Uid:
 *  - Frame Type,
 *  - Cluster Id,
 *  - Frame Domain,
 *  - Manufacturer Code.
 *
 *  Following fields of Command Uid are not included into Prefix:
 *  - Direction,
 *  - Command Id.
 *
 *  To obtain complete Command Uid its Prefix must be bitwise ORed with fields not
 *  included into the Prefix.
 */
#define ZBPRO_ZCL_MAKE_COMMAND_UID_PREFIX(frameType, clusterId, frameDomain, manufCode)\
        ((ZBPRO_ZCL_FRAME_TYPE_PROFILE_WIDE_COMMAND != (frameType))\
                ?   (0x0000000000000100 |\
                    ((ZBPRO_ZCL_FRAME_DOMAIN_ZCL_STANDARD != (frameDomain))\
                            ? (0x0000000001000000 | (((ZbProZclCommandUid_t)(manufCode)) << 48))\
                            : 0x0000000000000000) |\
                    (((ZbProZclCommandUid_t)(clusterId)) << 32))\
                :   0x0000000000000000)
/*
 * Validate definition of the Command Uid Prefix assembling macro.
 */
SYS_DbgAssertStatic(0xEEEECCCC01000100 == ZBPRO_ZCL_MAKE_COMMAND_UID_PREFIX(
        ZBPRO_ZCL_FRAME_TYPE_CLUSTER_SPECIFIC_COMMAND, /*clusterId*/ 0xCCCC,
        ZBPRO_ZCL_FRAME_DOMAIN_MANUF_SPECIFIC, /*manufCode*/ 0xEEEE));
SYS_DbgAssertStatic(0x0000CCCC00000100 == ZBPRO_ZCL_MAKE_COMMAND_UID_PREFIX(
        ZBPRO_ZCL_FRAME_TYPE_CLUSTER_SPECIFIC_COMMAND, /*clusterId*/ 0xCCCC,
        ZBPRO_ZCL_FRAME_DOMAIN_ZCL_STANDARD, /*manufCode*/ 0xEEEE));
SYS_DbgAssertStatic(0x0000000000000000 == ZBPRO_ZCL_MAKE_COMMAND_UID_PREFIX(
        ZBPRO_ZCL_FRAME_TYPE_PROFILE_WIDE_COMMAND, /*clusterId*/ 0xCCCC,
        ZBPRO_ZCL_FRAME_DOMAIN_MANUF_SPECIFIC, /*manufCode*/ 0xEEEE));
SYS_DbgAssertStatic(0x0000000000000000 == ZBPRO_ZCL_MAKE_COMMAND_UID_PREFIX(
        ZBPRO_ZCL_FRAME_TYPE_PROFILE_WIDE_COMMAND, /*clusterId*/ 0xCCCC,
        ZBPRO_ZCL_FRAME_DOMAIN_ZCL_STANDARD, /*manufCode*/ 0xEEEE));


/**//**
 * \brief   Assembles Command Uid from its Prefix and Suffix; used for cluster-specific
 *  commands.
 * \param[in]   commandUidPrefix    Numeric 64-bit value of Command Uid Prefix.
 * \param[in]   direction           Either Client-to-Server (0x0) or
 *  Server-to-Client (0x1).
 * \param[in]   commandId           Numeric 8-bit value of Command Identifier local to
 *  particular Cluster, Side, and Manufacturer.
 * \return  Numeric 64-bit of Command Uid.
 * \details
 *  Parameter \p direction specifies which side of cluster generates the command.
 * \note
 *  This macro must not be used for Profile-Wide commands, because \p direction field is
 *  not ignored by this macro automatically for the case of Profile-Wide command.
 */
#define ZBPRO_ZCL_MAKE_CLUSTER_SPECIFIC_COMMAND_UID(commandUidPrefix, direction, commandId)\
        (((ZbProZclCommandUid_t)(commandUidPrefix)) |\
        (((ZbProZclCommandUid_t)(direction)) << 16) |\
        ((ZbProZclCommandUid_t)(commandId)))
/*
 * Validate definition of the Cluster-Specific Command Uid assembling macro.
 */
SYS_DbgAssertStatic(0xEEEECCCC010101AA == ZBPRO_ZCL_MAKE_CLUSTER_SPECIFIC_COMMAND_UID(
        /*commandUidPrefix*/ 0xEEEECCCC01000100, ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT, /*commandId*/ 0xAA));
SYS_DbgAssertStatic(0xEEEECCCC010001AA == ZBPRO_ZCL_MAKE_CLUSTER_SPECIFIC_COMMAND_UID(
        /*commandUidPrefix*/ 0xEEEECCCC01000100, ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER, /*commandId*/ 0xAA));


/**//**
 * \brief   Extracts Command Id 8-bit field from Command Uid.
 * \param[in]   commandUid      Numeric 64-bit value of Command Uid.
 * \return  Numeric 8-bit value of Command Id local to particular Cluster, Side and
 *  Manufacturer.
 */
#define ZBPRO_ZCL_EXTRACT_COMMAND_ID(commandUid)\
        ((uint32_t)((commandUid) & 0xFF))
/*
 * Validate definition of the Command Id extracting macro.
 */
SYS_DbgAssertStatic(0xAA == ZBPRO_ZCL_EXTRACT_COMMAND_ID(/*commandUid*/ 0xFFFFFFFFFFFFFFAA));


/**//**
 * \brief   Extracts command Direction field from Command Uid.
 * \param[in]   commandUid      Numeric 64-bit value of Command Uid.
 * \return  Command Direction: either Client-to-Server (0x0) or Server-to-Client (0x1).
 */
#define ZBPRO_ZCL_EXTRACT_COMMAND_DIRECTION(commandUid)\
        ((uint32_t)(((commandUid) >> 16) & 0x1))
/*
 * Validate definition of the command Direction extracting macro.
 */
SYS_DbgAssertStatic(ZBPRO_ZCL_FRAME_DIRECTION_SERVER_TO_CLIENT ==
        ZBPRO_ZCL_EXTRACT_COMMAND_DIRECTION(/*commandUid*/ 0x0000000000010000));
SYS_DbgAssertStatic(ZBPRO_ZCL_FRAME_DIRECTION_CLIENT_TO_SERVER ==
        ZBPRO_ZCL_EXTRACT_COMMAND_DIRECTION(/*commandUid*/ 0xFFFFFFFFFFFEFFFF));


/**//**
 * \brief   Special value for ZCL Timeout for the case when ZCL layer shall not wait for
 *  response at all.
 * \details
 *  This constant shall be assigned to the response waiting timeout obligatory parameter
 *  of a Local Request if it allows multiple remote responses and they delivered to the
 *  higher layer with distinct Local Indications but not in the Local Confirm on the
 *  original Request. In this case Local Confirm is raised just on confirmation from the
 *  APS layer; ZCL layer will doesn't wait for timeout and never returns TIMEOUT status.
 */
#define ZBPRO_ZCL_TIMEOUT_DONT_WAIT_RESP    0x0000


/**//**
 * \brief   Line-ups ZCL Timeout parameter of Local Request.
 * \param[in]   timeout     Reference to timeout parameter; it must be compatible with
 *  L-value use.
 * \details
 *  When called by the higher layer, a Local Request is allowed to have its response
 *  waiting timeout parameter equal to 0xFFFF or 0x0000 as well if the default ZCL Timeout
 *  shall be used. But value 0x0000 is reserver for internal use (as the flag "Don't wait
 *  for response"). Consequently, if the timeout parameter is assigned to 0x0000 by the
 *  higher layer, it must be substituted with 0xFFFF which only has the internal meaning
 *  "Use the default ZCL Timeout".
 */
#define ZBPRO_ZCL_TIMEOUT_LINEUP(timeout)\
        SYS_WRAPPED_BLOCK(\
            if (0x0000 == (timeout))\
                (timeout) = ZBPRO_ZCL_TIMEOUT_USE_DEFAULT;\
        )


/**//**
 * \brief   Converts custom ZCL Local Primitive Descriptor type to Prototype.
 * \paraim[in]  customDescr     Pointer to custom ZCL Local Primitive Descriptor.
 * \return  Pointer to ZCL Local Primitive Descriptor Prototype.
 * \details
 *  This macro returns NULL when \p customDescr is NULL.
 */
#define ZBPRO_ZCL_CAST_DESCR_TO_PROTOTYPE(customDescr)\
        ((ZbProZclLocalPrimitiveDescrPrototype_t *const)(void *const)(customDescr))
/*
 * Validate preserving of the NULL by the conversion macro.
 */
SYS_DbgAssertStatic(NULL == ZBPRO_ZCL_CAST_DESCR_TO_PROTOTYPE(NULL));


/**//**
 * \brief   Helps to convert ZCL Local Primitive Descriptor or Parameters Prototype to
 *  custom type.
 * \paraim[in]  protoDescrOrParams      Pointer to ZCL Local Primitive Descriptor or
 *  Parameters Prototype.
 * \return  Pointer to ZCL Local Primitive Descriptor or Parameters compatible with custom
 *  types.
 * \details
 *  This macro returns NULL when \p protoDescrOrParams is NULL.
 */
#define ZBPRO_ZCL_CAST_TO_CUSTOM(protoDescrOrParams)\
        ((void *const)(protoDescrOrParams))
/*
 * Validate preserving of the NULL by the conversion macro.
 */
SYS_DbgAssertStatic(NULL == ZBPRO_ZCL_CAST_TO_CUSTOM(NULL));


/**//**
 * \brief   Structure of Frame Control Field of ZCL Frame Header.
 * \details
 *  This structure may be used directly for serialization/deserialization of Frame Control
 *  Field of ZCL Frame Header.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.3.1.1, figure 2-3.
 */
typedef union _ZbProZclFrameControl_t
{
    /* 8-bit data. */

    BitField8_t                         plain;                          /*!< Plain value. */

    /* Structured / 8-bit data. */

    struct
    {
        ZBPRO_ZCL_FrameType_t           frameType          : 2;         /*!< Specifies whether command acts across the
                                                                            entire profile (0x0) or command is specific
                                                                            to a cluster (0x1). */

        ZBPRO_ZCL_FrameDomain_t         manufSpecific      : 1;         /*!< Specifies whether this command refers to a
                                                                            manufacturer specific extension to a profile
                                                                            (0x1) or command is covered by ZCL standard
                                                                            (0x0). */

        ZBPRO_ZCL_FrameDirection_t      direction          : 1;         /*!< Specifies whether command is being sent
                                                                            from the server side of a cluster to the
                                                                            client side of a cluster (0x1) or in
                                                                            opposite direction (0x0). */

        ZBPRO_ZCL_DisableDefaultResp_t  disableDefaultResp : 1;         /*!< Specifies whether the Default response
                                                                            command will only be returned if there is an
                                                                            error (0x1) or irrespective to the status
                                                                            (0x0). */

        BitField8_t                                        : 3;         /*!< Reserved. */
    };
} ZbProZclFrameControl_t;

/*
 * Validate size of ZCL Frame Control Field data type.
 */
SYS_DbgAssertStatic(1 == sizeof(ZbProZclFrameControl_t));


/**//**
 * \brief   Structure of deserialized ZCL Frame Header.
 * \details
 *  This structure may not be used for serialization/deserialization of ZCL Frame Header.
 *  It is intended only for storing of ZCL Frame Header fields set in structured form.
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 2.3.1, figure 2-2.
 */
typedef struct _ZbProZclFrameHeader_t
{
    /* 16-bit data. */

    ZBPRO_ZCL_ManufCode_t    manufCode;         /*!< Manufacturer Code field. This field is optional and used only when
                                                    \c frameControl.manufSpecific equals to TRUE, i.e. for Manufacturer
                                                    Specific frames. */

    /* 8-bit data. */

    ZbProZclFrameControl_t   frameControl;      /*!< Frame Control field. */

    ZBPRO_ZCL_TransSeqNum_t  transSeqNum;       /*!< Transaction Sequence Number field. */

    ZBPRO_ZCL_CommandId_t    commandId;         /*!< Command Identifier field. */

} ZbProZclFrameHeader_t;


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   Initializes ZCL Dispatcher for the specified ZCL-based profile on startup.
 * \param[out]  dispatcher              Pointer to the initialized Dispatcher descriptor.
 * \param[in]   schedulerPriority       Dispatcher task priority when extracted for
 *  execution.
 * \param[in]   transactionsPullHead    Pointer to the first element of Transactions Pull.
 * \param[in]   transactionsPullSize    Size of Transactions Pull, in elements.
 * \param[in]   transQuotaLimits        Array of two Limit values for Dual Quota over the
 *  Transactions Pull between CLIENT and SERVER types of Transactions.
 * \param[in]   profileId               Numeric identifier of ZCL-based profile.
 * \note
 *  Do not ever call this function during the ZCL Dispatcher work even if it is itself and
 *  all its linked Services are temporarily idle; use this function only during the
 *  startup sequence for memory initialization.
 */
void zbProZclDispatcherInit(
                ZbProZclDispatcher_t   *const  dispatcher,
                const SYS_SchedulerPriority_t  schedulerPriority,
                ZbProZclTransaction_t  *const  transactionsPullHead,
                const size_t                   transactionsPullSize,
                const RpcDualQuotaLimit_t      transQuotaLimits[RPC_DUAL_QUOTA_CHANNELS_NUMBER],
                const ZBPRO_APS_ProfileId_t    profileId);


/**//**
 * \brief   Accepts New Local ZCL Request for processing with ZCL Dispatcher.
 * \param[in]   zclLocalRequest     Pointer to Local ZCL Request Prototype.
 * \details
 *  This function shall be called by ZCL cluster dedicated functions accepting requests
 *  for particular commands. It initializes ZCL Local Request Descriptor Service field and
 *  commences composing and issuing of corresponding ZCL command frame.
 */
void zbProZclDispatcherAcceptNewLocalRequest(
                ZbProZclLocalPrimitiveDescrPrototype_t *const  zclLocalRequestPrototype);


/**//**
 * \brief   Accepts the APSDE-DATA.indication for the case of ZCL-based Profile and one of
 *  endpoints assigned to such profile as a destination endpoint.
 * \param[in]   indParams       Pointer to APSDE-DATA.Indication parameters.
 * \details
 *  The chunk of dynamic memory described with \c indParams.payload object is freed by
 *  this function itself or by a postponed task scheduled by this function. If APS layer
 *  intends to use this data for a different purpose (for example, indicate it to another
 *  endpoint), it shall duplicate the payload prior to call this function.
 * \details
 *  Object referenced by \p indParams may be allocated on the program stack. This function
 *  does not saves this pointer; so, the pointed object may be destroyed right after this
 *  function returns.
 * \details
 *  This function itself does change the object pointed by \p indParams; so, it must not
 *  be reused by the caller. If the caller needs this indication parameters for a
 *  different purpose after this function returns (for example, to indicate it to another
 *  endpoint) it shall duplicate the indication parameters and provide this function with
 *  its private copy of such object.
 */
void ZBPRO_ZCL_DataInd(
                ZBPRO_APS_DataIndParams_t *const  indParams);


#endif /* _BB_ZBPRO_ZCL_DISPATCHER_H */

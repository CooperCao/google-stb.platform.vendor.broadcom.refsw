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
*       ZDO / ZDP Mgmt_Leave and Mgmt_Permit_Joining Services interface.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_ZDO_SAP_TYPES_NODE_MANAGER_H
#define _BB_ZBPRO_ZDO_SAP_TYPES_NODE_MANAGER_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZdoCommon.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for parameters of ZDO Local Request to issue ZDP Mgmt_Leave_req
 *  command.
 * \ingroup ZBPRO_ZDO_MgmtLeaveReq
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclause 2.4.3.3.5, figure 2.57, table 2.84.
 */
typedef struct _ZBPRO_ZDO_MgmtLeaveReqParams_t
{
    /* 64-bit data. */

    ZBPRO_NWK_ExtAddr_t  deviceAddress;                     /*!< The 64-bit IEEE address of the entity to be removed
                                                                from the network or 0x0 if the recipient device shall
                                                                remove itself from the network. */
    /* Structured data, aligned at 32 bits. */

    ZBPRO_ZDO_Address_t  zdpDstAddress;                     /*!< Destination address. It shall be unicast only. The
                                                                recipient device, if its extended address coincides with
                                                                \c deviceAddress or if the \c deviceAddress is set to
                                                                zero, is asked to leave the network; otherwise the
                                                                recipient device is asked to request another device
                                                                specified with \c deviceAddress to leave the network. */
    /* 8-bit data. */

    BitField8_t          fieldsRemoveChildrenRejoin;        /*!< Bit #6 'RemoveChildren' - if assigned to one, the
                                                                recipient device being asked to leave the network is
                                                                also being asked to remove its child devices, if any.
                                                                Bit #7 'Rejoin' - if assigned to one, the recipient
                                                                device being asked to leave from the current parent is
                                                                requested to rejoin the network. Bits #0-5 are
                                                                reserved, assigned with zero on transmission and ignored
                                                                on reception. */
} ZBPRO_ZDO_MgmtLeaveReqParams_t;


/**//**
 * \brief   Obtains the \c RemoveChildren flag from the 8-bit binary value.
 * \param[in]   bitField        Binary value containing the flag.
 * \return  Binary value of the flag, compatible with TRUE/FALSE definitions.
 * \details
 *  The \c RemoveChildren flag occupies bit #6.
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclause 2.4.3.3.5, figure 2.57, table 2.84.
 */
#define ZBPRO_ZDO_MGMT_LEAVE_GET_REMOVE_CHILDREN_FLAG(bitField)         (((bitField) >> 6) & 0x1)


/**//**
 * \brief   Obtains the \c Rejoin flag from the 8-bit binary value.
 * \param[in]   bitField        Binary value containing the flag.
 * \return  Binary value of the flag, compatible with TRUE/FALSE definitions.
 * \details
 *  The \c Rejoin flag occupies bit #7.
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclause 2.4.3.3.5, figure 2.57, table 2.84.
 */
#define ZBPRO_ZDO_MGMT_LEAVE_GET_REJOIN_FLAG(bitField)                  (((bitField) >> 7) & 0x1)


/**//**
 * \brief   Structure for parameters of ZDO Local Confirmation on ZDP Mgmt_Leave_req
 *  command.
 * \ingroup ZBPRO_ZDO_MgmtLeaveConf
 * \note
 *  This structure takes its origin from ZDP Mgmt_Leave_rsp command.
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclause 2.4.4.3.5, figure 2.98, table 2.132.
 */
typedef struct _ZBPRO_ZDO_MgmtLeaveConfParams_t
{
    /* 8-bit data. */

    ZBPRO_ZDO_Status_t  status;         /*!< The status of the Mgmt_Leave_req command. */

} ZBPRO_ZDO_MgmtLeaveConfParams_t;


/**//**
 * \brief   Structure for descriptor of ZDO Local Request to issue ZDP Mgmt_Leave_req
 *  command.
 * \ingroup ZBPRO_ZDO_MgmtLeaveReq
 */
typedef struct _ZBPRO_ZDO_MgmtLeaveReqDescr_t  ZBPRO_ZDO_MgmtLeaveReqDescr_t;


/**//**
 * \brief   Data type for ZDO Local Confirmation callback function of ZDP Mgmt_Leave_req
 *  command.
 * \ingroup ZBPRO_ZDO_MgmtLeaveConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZDO_MgmtLeaveConfCallback_t(
                ZBPRO_ZDO_MgmtLeaveReqDescr_t   *const  reqDescr,
                ZBPRO_ZDO_MgmtLeaveConfParams_t *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZDO Local Request to issue ZDP Mgmt_Leave_req
 *  command.
 * \ingroup ZBPRO_ZDO_MgmtLeaveReq
 */
struct _ZBPRO_ZDO_MgmtLeaveReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZDO_MgmtLeaveConfCallback_t *callback;        /*!< ZDO Confirmation callback handler entry point. */

    /* Structured data, aligned at 32 bits. */

    ZbProZdoLocalRequest_t             service;         /*!< ZDO Request Descriptor service field. */

    ZBPRO_ZDO_MgmtLeaveReqParams_t     params;          /*!< ZDO Request parameters structure. */
};


/**//**
 * \brief   Structure for parameters of ZDO Local Request to issue ZDP
 *  Mgmt_Permit_Joining_req command.
 * \ingroup ZBPRO_ZDO_MgmtPermitJoiningReq
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclause 2.4.3.3.7, figure 2.59, table 2.86.
 */
typedef struct _ZBPRO_ZDO_MgmtPermitJoiningReqParams_t
{
    /* Structured data, aligned at 32 bits. */

    ZBPRO_ZDO_Address_t             zdpDstAddress;          /*!< Destination address. It shall be unicast or broadcast
                                                                to all routers and coordinator. */
    /* 8-bit data. */

    ZBPRO_NWK_PermitJoinDuration_t  permitDuration;         /*!< The length of time during which the ZigBee coordinator
                                                                or router will allow associations, in seconds. The value
                                                                0x00 and 0xff indicate that permission is disabled or
                                                                enabled, respectively, without a specified time
                                                                limit. */

    Bool8_t                         tcSignificance;         /*!< If this is set to 0x01 and the remote device is the
                                                                Trust Center, the command affects the Trust Center
                                                                authentication policy. */
} ZBPRO_ZDO_MgmtPermitJoiningReqParams_t;


/**//**
 * \brief   Structure for parameters of ZDO Local Confirmation on ZDP
 *  Mgmt_Permit_Joining_req command.
 * \ingroup ZBPRO_ZDO_MgmtPermitJoiningConf
 * \note
 *  This structure takes its origin from ZDP Mgmt_Permit_Joining_rsp command.
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclause 2.4.4.3.7, figure 2.100, table 2.134.
 */
typedef struct _ZBPRO_ZDO_MgmtPermitJoiningConfParams_t
{
    /* 8-bit data. */

    ZBPRO_ZDO_Status_t  status;         /*!< The status of the Mgmt_Leave_req command. */

} ZBPRO_ZDO_MgmtPermitJoiningConfParams_t;


/**//**
 * \brief   Structure for descriptor of ZDO Local Request to issue ZDP
 *  Mgmt_Permit_Joining_req command.
 * \ingroup ZBPRO_ZDO_MgmtPermitJoiningReq
 */
typedef struct _ZBPRO_ZDO_MgmtPermitJoiningReqDescr_t  ZBPRO_ZDO_MgmtPermitJoiningReqDescr_t;


/**//**
 * \brief   Data type for ZDO Local Confirmation callback function of ZDP
 *  Mgmt_Permit_Joining_req command.
 * \ingroup ZBPRO_ZDO_MgmtPermitJoiningConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZDO_MgmtPermitJoiningConfCallback_t(
                ZBPRO_ZDO_MgmtPermitJoiningReqDescr_t   *const  reqDescr,
                ZBPRO_ZDO_MgmtPermitJoiningConfParams_t *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZDO Local Request to issue ZDP
 *  Mgmt_Permit_Joining_req command.
 * \ingroup ZBPRO_ZDO_MgmtPermitJoiningReq
 */
struct _ZBPRO_ZDO_MgmtPermitJoiningReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZDO_MgmtPermitJoiningConfCallback_t *callback;        /*!< ZDO Confirmation callback handler entry point. */

    /* Structured data, aligned at 32 bits. */

    ZbProZdoLocalRequest_t                     service;         /*!< ZDO Request Descriptor service field. */

    ZBPRO_ZDO_MgmtPermitJoiningReqParams_t     params;          /*!< ZDO Request parameters structure. */
};


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   Accepts ZDO Local Request to issue ZDP Mgmt_Leave_req command.
 * \ingroup ZBPRO_ZDO_Functions
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZDO_MgmtLeaveReq(
                ZBPRO_ZDO_MgmtLeaveReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZDO Local Request to issue ZDP Mgmt_Permit_Joining_req command.
 * \ingroup ZBPRO_ZDO_Functions
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZDO_MgmtPermitJoiningReq(
                ZBPRO_ZDO_MgmtPermitJoiningReqDescr_t *const  reqDescr);


#endif /* _BB_ZBPRO_ZDO_SAP_TYPES_NODE_MANAGER_H */

/* eof bbZbProZdoSapTypesNodeManager.h */
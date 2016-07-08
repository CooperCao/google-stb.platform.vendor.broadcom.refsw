/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 ******************************************************************************
/*****************************************************************************
 *
 * FILENAME: $Workfile: trunk/stack/ZbPro/ZDO/include/bbZbProZdoSapTypesBindingManager.h $
 *
 * DESCRIPTION:
 *   This header describes types and API for the ZDO Binding Manager component.
 *
 * $Revision: 2876 $
 * $Date: 2014-07-10 09:58:52Z $
 *
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZDO_SAP_TYPES_BINDING_MANAGER_H
#define _BB_ZBPRO_ZDO_SAP_TYPES_BINDING_MANAGER_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZdoCommon.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   The maximum allowed number of Cluster Identifiers totally in two parameters of
 *  End_Device_Bind_req command: \c InClusterList and \c OutClusterList.
 * \details
 *  The total number of Cluster Identifiers is limited by the maximum ASDU size (i.e., the
 *  maximum ZDP command size). The obligatory part of the End_Device_Bind_req command
 *  includes the following fields:
 *  - 1 octet - Transaction Sequence Number,
 *  - 2 octets - Binding Target 16-bit NWK address,
 *  - 8 octets - Source device IEEE Address,
 *  - 1 octet - Source Endpoint,
 *  - 2 octets - Profile ID,
 *  - 1 octet - Number of Input Clusters,
 *  - 1 octet - Number of Output Clusters.
 *
 *  Totally all the obligatory fields give 16 octets. The rest of the ASDU may be used for
 *  \c InClusterList and \c OutClusterList variable size fields. Each element of a list
 *  has the size of 2 octets.
 */
#define ZBPRO_ZDO_END_DEVICE_BIND_MAX_REQUESTED_CLUSTER_AMOUNT\
        ((ZBPRO_APS_MAX_DU_SIZE - (1 + (2 + 8 + 1 + 2 + 1 + 0 + 1 + 0))) / 2)


/**//**
 * \brief   Structure for parameters of ZDO Local Confirmation on ZDP End_Device_Bind_req,
 *  Bind_req and Unbind_req commands.
 * \note
 * \brief End_Device_Bind_rsp, Bind_resp and Unbind_resp command formats. See ZigBee Spec r20, Figure 2.83/2.84/2.85.
 */
typedef struct _ZBPRO_ZDO_BindConfParams_t
{
    ZBPRO_ZDO_Status_t      status;
} ZBPRO_ZDO_BindConfParams_t;

/**//**
 * \brief End_device_bind_req command formats. See ZigBee Spec r20, Figure 2.42.
 * \note
 *  The following condition must be satisfied: numInClusters + numOutClusters <
 *  ZBPRO_ZDO_MAX_REQUESTED_CLUSTER_AMOUNT.
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclause 2.4.3.2.1, figure 2.42, table 2.68.
 */
typedef struct _ZBPRO_ZDO_EndDeviceBindReqParams_t
{
    /* 64-bit data. */

    ZBPRO_ZDO_ExtAddr_t    srcIeeeAddress;          /*!< The IEEE address of the device generating the request. */

    /* Structured / 32-bit data. */

    SYS_DataPointer_t      clusterList;             /*!< List of Input ClusterIDs followed with the list of Output
                                                        ClusterIDs to be used for matching. The first part, the
                                                        InClusterList, is the desired list to be matched by the ZigBee
                                                        coordinator with the Remote Device’s output clusters (the
                                                        elements of the InClusterList are supported input clusters of
                                                        the Local Device). The second part, the OutClusterList, is to be
                                                        matched with the Remote Device’s input clusters. */
    /* 16-bit data. */

    ZBPRO_ZDO_NwkAddr_t    bindingTarget;           /*!< The address of the target for the binding. This can be either
                                                        the primary binding cache device or the short address of the
                                                        local device. */

    ZBPRO_ZDO_ProfileId_t  profileId;               /*!< Profile identifier which is to be matched between two
                                                        End_Device_Bind_req received at the ZigBee Coordinator within
                                                        the timeout value pre-configured in the ZigBee Coordinator. */
    /* 8-bit data. */

    ZBPRO_ZDO_Endpoint_t   srcEndpoint;             /*!< The endpoint on the device generating the request. */

    uint8_t                numInClusters;           /*!< The number of Input Clusters provided for end device binding
                                                        within the InClusterList. */

    uint8_t                numOutClusters;          /*!< The number of Output Clusters provided for matching within
                                                        the OutClusterList. */
} ZBPRO_ZDO_EndDeviceBindReqParams_t;


/**//**
 * \brief   Structure for descriptor of ZDO Local Request to issue ZDP End_Device_Bind_req
 *  command.
 */
typedef struct _ZBPRO_ZDO_EndDeviceBindReqDescr_t  ZBPRO_ZDO_EndDeviceBindReqDescr_t;


/**//**
 * \brief   Data type for ZDO Local Confirmation callback function of ZDP
 *  End_Device_Bind_req command.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZDO_EndDeviceBindConfCallback_t(
                ZBPRO_ZDO_EndDeviceBindReqDescr_t *const  reqDescr,
                ZBPRO_ZDO_BindConfParams_t        *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZDO Local Request to issue ZDP End_Device_Bind_req
 *  command.
 */
struct _ZBPRO_ZDO_EndDeviceBindReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZDO_EndDeviceBindConfCallback_t *callback;        /*!< ZDO Confirmation callback handler entry point. */

    /* Structured data, aligned at 32 bits. */

    ZbProZdoLocalRequest_t                 service;         /*!< ZDO Request Descriptor service field. */

    ZBPRO_ZDO_EndDeviceBindReqParams_t     params;          /*!< ZDO Request parameters structure. */
};


/**//**
 * \brief   Structure for parameters of ZDO Local Request to issue ZDP Bind_req or
 *  Unbind_req command.
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclauses 2.4.3.2.2, 2.4.3.2.3, figures 2.43, 2.44,
 *  tables 2.69, 2.70.
 */
typedef struct _ZBPRO_ZDO_BindUnbindReqParams_t
{
    /* 64-bit data. */

    ZBPRO_ZDO_ExtAddr_t    srcAddress;          /*!< The IEEE address for the source. */

    /* Structured data, aligned at 32 bits. */

    ZBPRO_ZDO_Address_t    zdpDstAddress;       /*!< Destination address. It shall be unicast only, and shall be that of
                                                    a Primary binding table cache or to the SrcAddress itself. */

    ZBPRO_ZDO_Address_t    dstAddress;          /*!< The destination address for the binding entry, and the addressing
                                                    mode for the destination address used in this command. The
                                                    destination address may be either 16-bit group address for
                                                    DstAddress and DstEndp not present, or 64-bit extended address for
                                                    DstAddress and DstEndp present. */
    /* 16-bit data. */

    ZBPRO_ZDO_ClusterId_t  clusterId;           /*!< The identifier of the cluster on the source device that is bound to
                                                    the destination. */
    /* 8-bit data. */

    ZBPRO_ZDO_Endpoint_t   srcEndp;             /*!< The source endpoint for the binding entry. */

    ZBPRO_ZDO_Endpoint_t   dstEndp;             /*!< The destination endpoint for the binding entry. This parameter is
                                                    treated only if the DstAddrMode field has a value of 0x03; otherwise
                                                    it's ignored. */
} ZBPRO_ZDO_BindUnbindReqParams_t;


/**//**
 * \brief   Structure for descriptor of ZDO Local Request to issue ZDP Bind_req or
 *  Unbind_req command.
 */
typedef struct _ZBPRO_ZDO_BindUnbindReqDescr_t  ZBPRO_ZDO_BindUnbindReqDescr_t;


/**//**
 * \brief   Data type for ZDO Local Confirmation callback function of ZDP Bind_req or
 *  Unbind_req command.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZDO_BindUnbindConfCallback_t(
                ZBPRO_ZDO_BindUnbindReqDescr_t *const  reqDescr,
                ZBPRO_ZDO_BindConfParams_t     *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZDO Local Request to issue ZDP Bind_req or
 *  Unbind_req command.
 */
struct _ZBPRO_ZDO_BindUnbindReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZDO_BindUnbindConfCallback_t *callback;       /*!< ZDO Confirmation callback handler entry point. */

    /* Structured data, aligned at 32 bits. */

    ZbProZdoLocalRequest_t              service;        /*!< ZDO Request Descriptor service field. */

    ZBPRO_ZDO_BindUnbindReqParams_t     params;         /*!< ZDO Request parameters structure. */
};


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   Accepts ZDO Local Request to issue ZDP End_Device_Bind_req command.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZDO_EndDeviceBindReq(
                ZBPRO_ZDO_EndDeviceBindReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZDO Local Request to issue ZDP Bind_req command.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZDO_BindReq(
                ZBPRO_ZDO_BindUnbindReqDescr_t *const  reqDescr);


/**//**
 * \brief   Accepts ZDO Local Request to issue ZDP Unbind_req command.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZDO_UnbindReq(
                ZBPRO_ZDO_BindUnbindReqDescr_t *const  reqDescr);


#endif /* _BB_ZBPRO_ZDO_SAP_TYPES_BINDING_MANAGER_H */

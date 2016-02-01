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
* FILENAME: $Workfile: trunk/stack/ZbPro/APS/include/bbZbProApsSapTransportKey.h $
*
* DESCRIPTION:
*   APSME-TRANSPORT-KEY security service interface.
*
* $Revision: 2586 $
* $Date: 2014-06-03 07:39:31Z $
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_APS_SAP_TRANSPORT_KEY_H
#define _BB_ZBPRO_APS_SAP_TRANSPORT_KEY_H


/************************* INCLUDES *****************************************************/
#include "bbZbProSsp.h"
#include "bbZbProApsSapSecurityTypes.h"
#include "bbZbProApsSapTunnel.h"


/***************************** TYPES ****************************************************/

/**//**
 * \brief APSME-TRANSPORT-KEY.request parameters data structure.
 */
typedef struct _ZBPRO_APS_TransportKeyReqParams_t
{
    /* 64-bit data. */
    ZBPRO_APS_ExtAddr_t     destAddress;     /*!< The extended 64-bit address
                                                  of the destination device. */
    /* 64-bit data. */
    ZBPRO_APS_ExtAddr_t     parentPartnerAddress;  /*!< The extended 64-bit address of the parent
                                                 of the destination device given by the
                                                 \e DestAddress parameter. */
    /* 16x8-bit data. */
    ZbProSspKey_t           key;            /*!< The key to be transported. */
    /* 8-bit data. */
    ZbProSspNwkKeySeqNum_t  keySeqNumber;   /*!< A sequence number assigned to a network key by the Trust
                                                Center and used to distinguish network keys for purposes of
                                                key updates and incoming frame security operations. */
    Bool8_t                 useParent;      /*!< Indicates if the destination device's parent shall be used
                                                to forward the key to the destination device:
                                                TRUE = Use parent; FALSE = Do not use parent. */
    Bool8_t                 initiator;      /*!< Indicates if the destination device of this key requested it:
                                                TRUE = If the destination requested the key;
                                                FALSE = Otherwise. */
    /* 8-bit data. */
    ZBPRO_APS_KeyType_t     keyType;               /*!< Identifies the type of key material
                                                        that should be transported. */
    Bool8_t                 apsSecure;
} ZBPRO_APS_TransportKeyReqParams_t;


/**//**
 * \brief APSME-TRANSPORT-KEY.request descriptor data type declaration.
 */
typedef struct _ZBPRO_APS_TransportKeyReqDescr_t  ZBPRO_APS_TransportKeyReqDescr_t;


/**//**
 * \brief APSME-TRANSPORT-KEY.confirm callback function data type.
 * \details Call this function to issue APSME-TRANSPORT-KEY.confirm to the higher layer.
 * \param reqDescr Pointer to the confirmed request descriptor data structure.
 * \param confParams Pointer to the confirmation parameters data structure. Treat this
 *  data structure in the confirmation handler-function as it has been allocated in the
 *  program stack by APS before calling this callback-handler and will be destroyed just
 *  after this callback returns.
 * \note According to the Standard there is no confirmation assumed on the primitive
 *  APSME-TRANSPORT-KEY.request. Nevertheless, this function is called with confirmation
 *  status parameter that shows at least successful or unsuccessful result from NWK layer.
 */
typedef void ZBPRO_APS_TransportKeyConfCallback_t(ZBPRO_APS_TransportKeyReqDescr_t       *const reqDescr,
                                                  ZBPRO_APS_SecurityServicesConfParams_t *const confParams);


/**//**
 * \brief APSME-TRANSPORT-KEY.request descriptor data type.
 */
typedef struct _ZBPRO_APS_TransportKeyReqDescr_t
{
    /* Fields are arranged to minimize paddings */
    ZBPRO_APS_TransportKeyConfCallback_t    *callback;      /*!< Confirm callback function.  */

    ZBPRO_APS_TransportKeyReqParams_t       params;         /*!< Request parameters set.     */

    struct
    {
        union
        {
            SYS_QueueElement_t          queueElement;
            ZBPRO_APS_TunnelReqDescr_t      tunnelReq;
        };
    } service;
} ZBPRO_APS_TransportKeyReqDescr_t;


/**//**
 * \brief APSME-TRANSPORT-KEY.indication parameters data structure.
 */
typedef struct _ZBPRO_APS_TransportKeyIndParams_t
{
    /* 64-bit data. */
    ZBPRO_APS_ExtAddr_t      srcAddress;            /*!< The extended 64-bit address of the device that
                                                        is the original source of the transported key. */
    /* Structured / 16x8-bit data. */
    struct
    {
        /* 16x8-bit data. */
        union
        {
            ZbProSspKey_t  trustCenterKey;        /*!< The Trust Center master or link key. */

            ZbProSspKey_t  networkKey;            /*!< The network key. */

            ZbProSspKey_t  applicationKey;        /*!< The master or link key. */
        };

        /* 64-bit data. */
        union
        {
            struct
            {
                /* 64-bit data. */
                ZBPRO_APS_ExtAddr_t partnerAddress; /*!< The extended 64-bit address of the device
                                                        that was also sent this key. */

                Bool8_t             initiator;      /*!< if true, indicates the initiator */
            };

            /* 8-bit data. */
            ZbProSspNwkKeySeqNum_t  keySeqNumber;   /*!< A sequence number assigned to a network key by the Trust
                                                        Center and used to distinguish network keys for purposes
                                                        of key updates and incoming frame security operations. */
        };
    }                        transportKeyData;      /*!< The key that was transported along with
                                                        identification and usage parameters. */
    /* 8-bit data. */
    ZBPRO_APS_KeyType_t      keyType;               /*!< Identifies the type of key material
                                                        that was be transported. */
} ZBPRO_APS_TransportKeyIndParams_t;


/**//**
 * \brief APSME-TRANSPORT-KEY.indication callback function data type.
 * \details Call this function to issue APSME-TRANSPORT-KEY.indication to the higher
 *  layer.
 * \param indParams Pointer to the indication parameters data structure. Treat this data
 *  structure in the indication handler-function as it has been allocated in the program
 *  stack by APS before calling this callback-handler and will be destroyed just after
 *  this callback returns.
 */
typedef void ZBPRO_APS_TransportKeyIndCallback_t(ZBPRO_APS_TransportKeyIndParams_t *indParams);


/************************* PROTOTYPES ***************************************************/

/*************************************************************************************//**
  \brief
    Accepts APSME-TRANSPORT-KEY.request from ZDO Security Manager to ZigBee Pro APS
    and starts its processing.
  \param    reqDescr
    Pointer to the request descriptor data structure.
  \note
    Data structure pointed by \p reqDescr must reside in global memory space and must be
    preserved by the caller until confirmation from APS. The \c service field of request
    descriptor is used by APS during request processing. The caller shall set the
    \c callback field to the entry point of its APSME-TRANSPORT-KEY.confirm
    handler-function.
  \note
    It is allowed to commence new request to APS directly from the context of the
    confirmation handler. The same request descriptor data object may be used for the new
    request as that one returned with confirmation parameters.
*****************************************************************************************/
APS_PUBLIC void ZBPRO_APS_TransportKeyReq(ZBPRO_APS_TransportKeyReqDescr_t *reqDescr);


/*************************************************************************************//**
  \brief
    Issues APSME-TRANSPORT-KEY.indication to ZigBee PRO.
  \param    indParams
    Pointer to the indication parameters data structure.
  \note
    ZDO Security Manager shall provide APSME-TRANSPORT-KEY.indication handler-function
    according to the template:
  \code
    void ZBPRO_APS_TransportKeyInd(ZBPRO_APS_TransportKeyIndParams_t *indParams) { ... }
  \endcode
  \note
    Indication handler-function shall treat the data structure pointed with \p indParams
    as it has been allocated in the program stack by APS before calling this
    callback-handler and will be destroyed by APS just after this callback returns.
  \note
    It is allowed to commence new request to APS directly from the context of this
    indication handler.
*****************************************************************************************/
APS_PUBLIC ZBPRO_APS_TransportKeyIndCallback_t ZBPRO_APS_TransportKeyInd;


#endif /* _BB_ZBPRO_APS_SAP_TRANSPORT_KEY_H */
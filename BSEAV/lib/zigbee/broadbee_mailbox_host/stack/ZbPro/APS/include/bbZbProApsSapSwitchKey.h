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
*       APSME-SWITCH-KEY security service interface.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_APS_SAP_SWITCH_KEY_H
#define _BB_ZBPRO_APS_SAP_SWITCH_KEY_H


/************************* INCLUDES *****************************************************/
#include "bbZbProApsSapSecurityTypes.h"     /* APSME Security Services data types definitions. */
#include "bbZbProSsp.h"


/***************************** TYPES ****************************************************/
/**//**
 * \brief APSME-SWITCH-KEY.request parameters data structure.
 * \ingroup ZBPRO_APS_SwitchKeyReq
 */
typedef struct _ZBPRO_APS_SwitchKeyReqParams_t
{
    /* 64-bit data. */
    ZBPRO_APS_ExtAddr_t     destAddress;     /*!< The extended 64-bit address of the device
                                                to which the switch-key command is sent. */
    /* 8-bit data. */
    ZbProSspNwkKeySeqNum_t  keySeqNumber;    /*!< A sequence number assigned to a network key by the
                                                Trust Center and used to distinguish network keys. */
} ZBPRO_APS_SwitchKeyReqParams_t;


/**//**
 * \brief APSME-SWITCH-KEY.request descriptor data type declaration.
 * \ingroup ZBPRO_APS_SwitchKeyReq
 */
typedef struct _ZBPRO_APS_SwitchKeyReqDescr_t  ZBPRO_APS_SwitchKeyReqDescr_t;


/**//**
 * \brief APSME-SWITCH-KEY.confirm callback function data type.
 * \details Call this function to issue APSME-SWITCH-KEY.confirm to the higher layer.
 * \ingroup ZBPRO_APS_SwitchKeyConf
 * \param reqDescr Pointer to the confirmed request descriptor data structure.
 * \param confParams Pointer to the confirmation parameters data structure. Treat this
 *  data structure in the confirmation handler-function as it has been allocated in the
 *  program stack by APS before calling this callback-handler and will be destroyed just
 *  after this callback returns.
 * \note According to the Standard there is no confirmation assumed on the primitive
 *  APSME-SWITCH-KEY.request. Nevertheless, this function is called with confirmation
 *  status parameter that shows at least successful or unsuccessful result from NWK layer.
 */
typedef void ZBPRO_APS_SwitchKeyConfCallback_t(ZBPRO_APS_SwitchKeyReqDescr_t          *const reqDescr,
                                               ZBPRO_APS_SecurityServicesConfParams_t *const confParams);


/**//**
 * \brief APSME-SWITCH-KEY.request descriptor data type.
 * \ingroup ZBPRO_APS_SwitchKeyReq
 */
typedef struct _ZBPRO_APS_SwitchKeyReqDescr_t
{
    /* Fields are arranged to minimize paddings */
    ZBPRO_APS_SwitchKeyConfCallback_t *callback;        /*!< Confirm callback function.  */

    struct
    {
        SYS_QueueElement_t             queueElement;    /*!< APS requests service field. */
    } service;                                          /*!< Service field container. */

    ZBPRO_APS_SwitchKeyReqParams_t     params;          /*!< Request parameters set.     */

} ZBPRO_APS_SwitchKeyReqDescr_t;


/**//**
 * \brief APSME-SWITCH-KEY.indication parameters data structure.
 * \ingroup ZBPRO_APS_SwitchKeyInd
 */
typedef struct _ZBPRO_APS_SwitchKeyIndParams_t
{
    /* 64-bit data. */
    ZBPRO_APS_ExtAddr_t     srcAddress;      /*!< The extended 64-bit address of the device
                                                that sent the switch-key command. */
    /* 8-bit data. */
    ZbProSspNwkKeySeqNum_t  keySeqNumber;    /*!< A sequence number assigned to a network key by the
                                                Trust Center and used to distinguish network keys. */
} ZBPRO_APS_SwitchKeyIndParams_t;


/**//**
 * \brief APSME-SWITCH-KEY.indication callback function data type.
 * \details Call this function to issue APSME-SWITCH-KEY.indication to the higher
 *  layer.
 * \ingroup ZBPRO_APS_SwitchKeyInd
 * \param indParams Pointer to the indication parameters data structure. Treat this data
 *  structure in the indication handler-function as it has been allocated in the program
 *  stack by APS before calling this callback-handler and will be destroyed just after
 *  this callback returns.
 */
typedef void ZBPRO_APS_SwitchKeyInd_t(ZBPRO_APS_SwitchKeyIndParams_t *indParams);


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
  \brief
    Accepts APSME-SWITCH-KEY.request from ZDO Security Manager to ZigBee Pro APS
    and starts its processing.
  \ingroup ZBPRO_APS_Functions
  \param[in]    reqDescr
    Pointer to the request descriptor data structure.
  \note
    Data structure pointed by \p reqDescr must reside in global memory space and must be
    preserved by the caller until confirmation from APS. The \c service field of request
    descriptor is used by APS during request processing. The caller shall set the
    \c callback field to the entry point of its APSME-SWITCH-KEY.confirm
    handler-function.
  \note
    It is allowed to commence new request to APS directly from the context of the
    confirmation handler. The same request descriptor data object may be used for the new
    request as that one returned with confirmation parameters.
  \return Nothing.
*****************************************************************************************/
APS_PUBLIC void ZBPRO_APS_SwitchKeyReq(ZBPRO_APS_SwitchKeyReqDescr_t *reqDescr);


/*************************************************************************************//**
  \brief
    Issues APSME-SWITCH-KEY.indication to ZigBee PRO.
  \ingroup ZBPRO_APS_Functions
  \param[in]    indParams
    Pointer to the indication parameters data structure.
  \note
    ZDO Security Manager shall provide APSME-SWITCH-KEY.indication handler-function
    according to the template:
  \code
    void ZBPRO_APS_SwitchKeyInd(ZBPRO_APS_SwitchKeyIndParams_t *indParams) { ... }
  \endcode
  \note
    Indication handler-function shall treat the data structure pointed with \p indParams
    as it has been allocated in the program stack by APS before calling this
    callback-handler and will be destroyed by APS just after this callback returns.
  \note
    It is allowed to commence new request to APS directly from the context of this
    indication handler.
  \return Nothing.
*****************************************************************************************/
APS_PUBLIC ZBPRO_APS_SwitchKeyInd_t ZBPRO_APS_SwitchKeyInd;


#endif /* _BB_ZBPRO_APS_SAP_SWITCH_KEY_H */

/* eof bbZbProApsSapSwitchKey.h*/
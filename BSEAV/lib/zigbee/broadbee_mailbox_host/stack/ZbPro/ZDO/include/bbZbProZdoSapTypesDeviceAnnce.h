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
 * FILENAME: $Workfile: trunk/stack/ZbPro/ZDO/include/bbZbProZdoSapTypesDeviceAnnce.h $
 *
 * DESCRIPTION:
 *   This header describes types and API for the ZDO Device Annce component.
 *
 * $Revision: 2999 $
 * $Date: 2014-07-21 13:30:43Z $
 *
 ****************************************************************************************/


#ifndef _BB_ZBPRO_ZDO_SAP_TYPES_DEVICE_ANNCE_H
#define _BB_ZBPRO_ZDO_SAP_TYPES_DEVICE_ANNCE_H


/************************* INCLUDES *****************************************************/
#include "bbZbProZdoCommon.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for parameters of ZDO Local Request to issue ZDP Device_Annce
 *  command.
 * \par     Documentation
 *  See ZigBee Document 053474r20, subclause 2.4.3.1.11, figure 2.30, table 2.55.
 */
typedef struct _ZBPRO_ZDO_DeviceAnnceReqParams_t
{
    /* 64-bit data. */

    ZBPRO_ZDO_ExtAddr_t     extAddr;        /*!< IEEE address for the Local Device. */

    /* 16-bit data. */

    ZBPRO_ZDO_NwkAddr_t     nwkAddr;        /*!< NWK address for the Local Device. */

    /* 8-bit data. */

    ZBPRO_ZDO_Capability_t  capability;     /*!< Capability of the local device. */

} ZBPRO_ZDO_DeviceAnnceReqParams_t;


/**//**
 * \brief   Structure for parameters of ZDO Local Confirmation on ZDP Device_Annce
 *  command.
 */
typedef struct _ZBPRO_ZDO_DeviceAnnceConfParams_t
{
    /* 8-bit data. */

    ZBPRO_ZDO_Status_t  status;         /*!< Status field. */

} ZBPRO_ZDO_DeviceAnnceConfParams_t;


/**//**
 * \brief   Structure for descriptor of ZDO Local Request to issue ZDP Device_Annce
 *  command.
 */
typedef struct _ZBPRO_ZDO_DeviceAnnceReqDescr_t  ZBPRO_ZDO_DeviceAnnceReqDescr_t;


/**//**
 * \brief   Data type for ZDO Local Confirmation callback function of ZDP Device_Annce
 *  command.
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZDO_DeviceAnnceConfCallback_t(
                ZBPRO_ZDO_DeviceAnnceReqDescr_t   *const  reqDescr,
                ZBPRO_ZDO_DeviceAnnceConfParams_t *const  confParams);


/**//**
 * \brief   Structure for descriptor of ZDO Local Request to issue ZDP Device_Annce
 *  command.
 */
struct _ZBPRO_ZDO_DeviceAnnceReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZDO_DeviceAnnceConfCallback_t *callback;      /*!< ZDO Confirmation callback handler entry point. */

    /* Structured data, aligned at 32 bits. */

    ZbProZdoLocalRequest_t               service;       /*!< ZDO Request Descriptor service field. */

    ZBPRO_ZDO_DeviceAnnceReqParams_t     params;        /*!< ZDO Request parameters structure. */
};

/**//**
 * \brief   Data type for parameters of ZDO Indication for received ZDP Device_Annce command.
 * \par     Documentation
 */
typedef ZBPRO_ZDO_DeviceAnnceReqParams_t ZBPRO_ZDO_DeviceAnnceIndParams_t;

/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   Accepts ZDO Local Request to issue ZDP Device_Annce command.
 * \param[in]   reqDescr        Pointer to ZDO Local Request descriptor.
 */
void ZBPRO_ZDO_DeviceAnnceReq(
                ZBPRO_ZDO_DeviceAnnceReqDescr_t *const  reqDescr);

/**//**
 * \brief   ZDO Indication for received Device_Annce command
 * \param[in]   indParams       Pointer to ZDO Indication parameters.
 * \note
 *  This function must be provided by the application.
 */
void ZBPRO_ZDO_DeviceAnnceInd(ZBPRO_ZDO_DeviceAnnceIndParams_t *const indParams);

#endif /* _BB_ZBPRO_ZDO_SAP_TYPES_DEVICE_ANNCE_H */

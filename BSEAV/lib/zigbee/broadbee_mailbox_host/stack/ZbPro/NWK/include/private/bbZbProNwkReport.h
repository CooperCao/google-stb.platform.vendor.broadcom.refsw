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
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/private/bbZbProNwkReport.h $
*
* DESCRIPTION:
*   Contains declaration for network report command handler.
*
* $Revision: 2595 $
* $Date: 2014-06-03 15:11:16Z $
*
****************************************************************************************/
#ifndef _ZBPRO_NWK_REPORT_H
#define _ZBPRO_NWK_REPORT_H

/************************* INCLUDES ****************************************************/
#include "bbMacSapForZBPRO.h"
#include "private/bbZbProNwkServices.h"

/************************* DEFINITIONS **************************************************/
#define ZBPRO_NWK_REPORT_BASE_PAYLOAD_LENGTH                        \
    (sizeof(ZbProNwkCommandId_t)        /* cmd id */                \
     + sizeof(uint8_t)                  /* report options */        \
     + sizeof(ZBPRO_NWK_ExtPanId_t))    /* EPID */

#define ZBPRO_NWK_MAX_PANID_REPORT_SIZE                             \
    (ZBPRO_NWK_MAX_DU_SIZE                                          \
     - (NWK_SIZE_OF_BASE_HEADER + sizeof(ZBPRO_NWK_ExtAddr_t) * 2)  \
     - ZBPRO_NWK_REPORT_BASE_PAYLOAD_LENGTH)

#define ZBPRO_NWK_PANID_REPORT_LIST_AMOUNT \
    (ZBPRO_NWK_MAX_PANID_REPORT_SIZE / sizeof(ZBPRO_NWK_PanId_t))

/**//**
 * \brief Initializer for Report service descriptor.
 */
#define NWK_REPORT_SERVICE_DESCRIPTOR     \
{                                         \
    .payloadSize    = zbProNwkReportSize, \
    .fill           = zbProNwkReportFill, \
    .conf           = zbProNwkReportConf, \
    .ind            = zbProNwkReportInd,  \
    .reset          = zbProNwkReportReset,\
}

/************************* TYPES *******************************************************/
/*
 * \brief Report status enumeration.
 */
typedef enum
{
    ZBPRO_NWK_REPORT_PANID_CONFLICT = 0U,
} ZbProNwkReportStatus_t;

/**//**
 * \brief Network report request parameters.
 */
typedef struct _ZbProNwkReportReqParams_t
{
    SYS_DataPointer_t       payload;
    BitField8_t             infoCount   : 5;
    BitField8_t             reportCode  : 3;
} ZbProNwkReportReqParams_t;

#define NWK_GET_NWK_REPORT_INFO_COUNT(options)          GET_BITFIELD_VALUE(options, 0,  5)
#define NWK_GET_NWK_REPORT_CODE(options)                GET_BITFIELD_VALUE(options, 5,  3)
#define NWK_SET_NWK_REPORT_INFO_COUNT(options, value)   SET_BITFIELD_VALUE(options, 0,  5, value)
#define NWK_SET_NWK_REPORT_CODE(options, value)         SET_BITFIELD_VALUE(options, 5,  3, value)

/**//**
 * \brief Network report request descriptor prototype.
 */
typedef struct _ZbProNwkReportReqDescriptor_t ZbProNwkReportReqDescriptor_t;

/**//**
 * \brief Network report request callback type.
 */
typedef void (*ZbProNwkReportReqCallback_t) (ZbProNwkReportReqDescriptor_t *const reqDescr,
        ZbProNwkServiceDefaultConfParams_t *const confParams);
/**//**
 * \brief Network report request descriptor type.
 */
struct _ZbProNwkReportReqDescriptor_t
{
    ZbProNwkReportReqParams_t   params;
    ZbProNwkReportReqCallback_t callback;
};

/**//**
 * \brief Network report indication parameters.
 */
typedef struct _ZbProNwkReportReqParams_t ZbProNwkReportIndParams_t;

/**//**
 * \brief Network report requests service descriptor.
 */
typedef struct _ZbProNwkReportRequestServiceDescr_t
{
    ZbProNwkReportReqDescriptor_t  *reportReq;
    ZBPRO_NWK_ExtAddr_t             networkManagerExtAddr;
} ZbProNwkReportRequestServiceDescr_t;

/************************* FUNCTIONS PROTOTYPES *****************************************/
/*************************************************************************************//**
  \brief Request to send the network report command.
  \param[in] req - the request descriptor pointer.
*****************************************************************************************/
void zbProNwkReportReq(ZbProNwkReportReqDescriptor_t *const req);

/*************************************************************************************//**
  \brief Called when a panId conflict report has been received.
  \param[in] ind - indication parameters pointer.
*****************************************************************************************/
void zbProNwkReportPanIdConflictInd(ZbProNwkReportIndParams_t *const ind);

/************************************************************************************//**
    \brief Initialize service.
****************************************************************************************/
NWK_PRIVATE ZbProNwkResetServiceHandler_t       zbProNwkReportReset;

/************************************************************************************//**
  \brief Returns memory size required for the network report command.
 ****************************************************************************************/
NWK_PRIVATE ZbProNwkGetPayloadSizeServiceHandler_t  zbProNwkReportSize;

/*************************************************************************************//**
  \brief Fills network report command header and payload.
 ****************************************************************************************/
NWK_PRIVATE ZbProNwkFillPacketServiceHandler_t  zbProNwkReportFill;

/************************************************************************************//**
    \brief This function invoked when frame dropped into air.
    \param[in] outputBuffer - buffer pointer.
    \param[in] status       - transmission status
****************************************************************************************/
NWK_PRIVATE ZbProNwkConfServiceHandler_t        zbProNwkReportConf;

/************************************************************************************//**
    \brief This function invoked when network status frame has been received.
    \param[in] inputBuffer - buffer pointer.
****************************************************************************************/
NWK_PRIVATE ZbProNwkIndServiceHandler_t         zbProNwkReportInd;

#endif /* _ZBPRO_NWK_REPORT_H */
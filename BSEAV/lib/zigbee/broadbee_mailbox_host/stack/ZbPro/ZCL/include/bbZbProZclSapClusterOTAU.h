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
*       Server side ZCL OTA Upgrade cluster SAP interface.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_ZCL_SAP_CLUSTER_OTAU_H
#define _BB_ZBPRO_ZCL_SAP_CLUSTER_OTAU_H

/************************* INCLUDES *****************************************************/
#include "bbZbProZclCommon.h"

/*************************** TYPES ******************************************************/
/**//**
 * \brief Type is used to store the Image Type code.
 * \ingroup ZBPRO_ZCL_OTAUNextImageInd
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 11.4.2.6, table 11-4.
 */
typedef uint16_t     ZBPRO_ZCL_OTAUImageType_t;
SYS_DbgAssertStatic(sizeof(ZBPRO_ZCL_OTAUImageType_t) == 2);

/**//**
 * \brief Type is used to store the File Version code.
 * \ingroup ZBPRO_ZCL_OTAUNextImageInd
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 11.4.2.7, table 11-5.
 */
typedef uint32_t     ZBPRO_ZCL_OTAUFileVersion_t;
SYS_DbgAssertStatic(sizeof(ZBPRO_ZCL_OTAUFileVersion_t) == 4);

/**//**
 * \brief Type is used to store the Hardware Version code.
 * \ingroup ZBPRO_ZCL_OTAUNextImageInd
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 11.4.2.13, table 11-8.
 */
typedef uint16_t     ZBPRO_ZCL_OTAUHardwareVersion_t;
SYS_DbgAssertStatic(sizeof(ZBPRO_ZCL_OTAUHardwareVersion_t) == 2);

/**//**
 * \brief Structure for FieldControl field of the Query Next Image Request
 * command.
 * \ingroup ZBPRO_ZCL_OTAUNextImageInd
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 11.13.4.1, figure 11-9.
*/
typedef struct _ZBPRO_ZCL_OTAUQueryNextImageFieldControl_t
{
    BitField8_t  hardwareVersionPresent   : 1;                            /*!< Has hardware version? */
    BitField8_t  reserved                 : 7;                            /*!< Reserved */

} ZBPRO_ZCL_OTAUQueryNextImageFieldControl_t;
SYS_DbgAssertStatic(sizeof(ZBPRO_ZCL_OTAUQueryNextImageFieldControl_t) == 1);

/**//**
 * \brief OTA Upgrade ZCL cluster Query Next Image Request command indication parameters.
 * \ingroup ZBPRO_ZCL_OTAUNextImageInd
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 11.13.4.1, figure 11-9.
 */
typedef struct _ZBPRO_ZCL_OTAUCmdQueryNextImageRequestIndParams_t
{
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;            /*!< Set of obligatory parameters of ZCL public
                                                                               interface to local application. */
    /* 32 bits data. */

    ZBPRO_ZCL_OTAUFileVersion_t                  fileVersion;             /*!< File version */

    /* 16 bits data. */

    ZBPRO_ZCL_ManufCode_t                        otauManufacturerCode;    /*!< Manufacturer code */
    ZBPRO_ZCL_OTAUImageType_t                    imageType;               /*!< Image type */
    ZBPRO_ZCL_OTAUHardwareVersion_t              hardwareVersion;         /*!< Hardware version */

    /* 8 bits data. */

    ZBPRO_ZCL_OTAUQueryNextImageFieldControl_t   fieldControl;            /*!< Field control */

} ZBPRO_ZCL_OTAUCmdQueryNextImageRequestIndParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_OTAUCmdQueryNextImageRequestIndParams_t);



/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Query Next Image Respose
 * command specific to OTA Upgrade ZCL cluster.
 * \ingroup ZBPRO_ZCL_OTAUNextImageRespReq
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 11.13.5.1, figure 11-10.
 */
typedef struct _ZBPRO_ZCL_OTAUCmdQueryNextImageResponseReqParams_t
{
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                               interface to local application. */
    /* 32 bits data. */

    ZBPRO_ZCL_OTAUFileVersion_t     fileVersion;                          /*!< File version */
    uint32_t                        imageSize;                            /*!< Image size */

    /* 16 bits data. */

    ZBPRO_ZCL_ManufCode_t           otauManufacturerCode;                 /*!< Manufacturer code */
    ZBPRO_ZCL_OTAUImageType_t       imageType;                            /*!< Image type */

    /* 8 bits data. */

    ZBPRO_ZCL_Status_t              status;                               /*!< Request status */

} ZBPRO_ZCL_OTAUCmdQueryNextImageResponseReqParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_OTAUCmdQueryNextImageResponseReqParams_t);

/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Query Next Image Respose
 * command specific to OTA Upgrade ZCL cluster.
 * \ingroup ZBPRO_ZCL_OTAUNextImageRespConf
 *\note
 * This command has no response. So, Confirmation will be empty (with obligatory parameters only).
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 11.13.5.
 */
typedef struct _ZBPRO_ZCL_OTAUCmdQueryNextImageResponseConfParams_t
{
    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;            /*!< Set of obligatory parameters of ZCL public
                                                                               interface to local application. */
    /* No custom parameters. */

} ZBPRO_ZCL_OTAUCmdQueryNextImageResponseConfParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_OTAUCmdQueryNextImageResponseConfParams_t);

/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Query Next Image Respose
 * command specific to OTA Upgrade ZCL cluster.
 * \ingroup ZBPRO_ZCL_OTAUNextImageRespReq
 */
typedef struct   _ZBPRO_ZCL_OTAUCmdQueryNextImageResponseReqDescr_t     ZBPRO_ZCL_OTAUCmdQueryNextImageResponseReqDescr_t;

/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Query Next Image Respose
 * command specific to OTA Upgrade ZCL cluster.
 * \ingroup ZBPRO_ZCL_OTAUNextImageRespConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_OTAUCmdQueryNextImageResponseConfCallback_t(
        ZBPRO_ZCL_OTAUCmdQueryNextImageResponseReqDescr_t     *const  reqDescr,
        ZBPRO_ZCL_OTAUCmdQueryNextImageResponseConfParams_t   *const  confParams);

/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Query Next Image Respose
 * command specific to OTA Upgrade ZCL cluster.
 * \ingroup ZBPRO_ZCL_OTAUNextImageRespReq
 */
struct _ZBPRO_ZCL_OTAUCmdQueryNextImageResponseReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_OTAUCmdQueryNextImageResponseConfCallback_t      *callback; /*!< ZCL Confirmation callback handler entry
                                                                               point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t                        service;  /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_OTAUCmdQueryNextImageResponseReqParams_t          params;   /*!< ZCL Request parameters structure. */
};

/**//**
 * \brief Structure for FieldControl field of the Image Block Request
 * command.
 * \ingroup ZBPRO_ZCL_OTAUImageBlockInd
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 11.13.6.1, figure 11-11.
 */
typedef struct _ZBPRO_ZCL_OTAUImageBlockFieldControl_t
{
    BitField8_t  requestNodeAddressPresent : 1;                           /*!< Node address presents */
    BitField8_t  minBlockPeriodPresent     : 1;                           /*!< Minimum period for block defined */
    BitField8_t  reserved                  : 6;                           /*!< Reserved */
} ZBPRO_ZCL_OTAUImageBlockFieldControl_t;
SYS_DbgAssertStatic(sizeof(ZBPRO_ZCL_OTAUImageBlockFieldControl_t) == 1);

/**//**
 * \brief OTA Upgrade ZCL cluster Image Block Request command indication parameters.
 * \ingroup ZBPRO_ZCL_OTAUImageBlockInd
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 11.13.6.1, figure 11-11.
 */
typedef struct _ZBPRO_ZCL_OTAUCmdImageBlockRequestIndParams_t
{
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;            /*!< Set of obligatory parameters of ZCL public
                                                                               interface to local application. */
    /* 64-bit data. */

    ZBPRO_APS_ExtAddr_t                             requestNodeAddress;   /*!< Node Extended Address */

    /* 32-bit data. */

    ZBPRO_ZCL_OTAUFileVersion_t                     fileVersion;          /*!< File version */
    uint32_t                                        fileOffset;           /*!< File offset */

    /* 16-bit data. */

    ZBPRO_ZCL_ManufCode_t                           otauManufacturerCode; /*!< Manufacturer code */
    ZBPRO_ZCL_OTAUImageType_t                       imageType;            /*!< Image type */
    uint16_t                                        minimumBlockPeriod;   /*!< Minimum period for block in seconds */

    /* 8-bit data. */

    uint8_t                                         maxDataSize;          /*!< Maximum data size */
    ZBPRO_ZCL_OTAUImageBlockFieldControl_t          fieldControl;         /*!< Field control */

} ZBPRO_ZCL_OTAUCmdImageBlockRequestIndParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_OTAUCmdImageBlockRequestIndParams_t);



/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Image Block Response
 * command specific to OTA Upgrade ZCL cluster.
 * \ingroup ZBPRO_ZCL_OTAUImageBlockRespReq
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 11.13.8.1, figure 11-13.
 */
typedef struct _ZBPRO_ZCL_OTAUCmdImageBlockResponseReqParams_t
{
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                               interface to local application. */
    /* 32-bit data. */

    SYS_DataPointer_t             payload;                                /*!< Request Payload */

    ZBPRO_ZCL_OTAUFileVersion_t   fileVersion;                            /*!< File version */
    uint32_t                      fileOffset;                             /*!< File offset */

    uint32_t                      currentTime;                            /*!< Current time */
    uint32_t                      requestTime;                            /*!< Request time */

    /* 16-bit data. */

    ZBPRO_ZCL_ManufCode_t         otauManufacturerCode;                   /*!< Manufacturer code */
    ZBPRO_ZCL_OTAUImageType_t     imageType;                              /*!< Image type */
    uint16_t                      minimumBlockPeriod;                     /*!< Minimum period for block in seconds */

    /* 8-bit data. */

    ZBPRO_ZCL_Status_t            status;                                 /*!< Request status */

} ZBPRO_ZCL_OTAUCmdImageBlockResponseReqParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_OTAUCmdImageBlockResponseReqParams_t);

/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Image Block Response
 * command specific to OTA Upgrade ZCL cluster.
 * \ingroup ZBPRO_ZCL_OTAUImageBlockRespConf
 * \note
 * This command has no response. So, Confirmation will be empty (with obligatory parameters only).
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 11.13.5.
 */
typedef struct _ZBPRO_ZCL_OTAUCmdImageBlockResponseConfParams_t
{
    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;            /*!< Set of obligatory parameters of ZCL public
                                                                               interface to local application. */
    /* No custom parameters. */

} ZBPRO_ZCL_OTAUCmdImageBlockResponseConfParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_OTAUCmdImageBlockResponseConfParams_t);

/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Image Block Response
 * command specific to OTA Upgrade ZCL cluster.
 * \ingroup ZBPRO_ZCL_OTAUImageBlockRespReq
 */
typedef struct   _ZBPRO_ZCL_OTAUCmdImageBlockResponseReqDescr_t     ZBPRO_ZCL_OTAUCmdImageBlockResponseReqDescr_t;

/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Image Block Response
 * command specific to OTA Upgrade ZCL cluster.
 * \ingroup ZBPRO_ZCL_OTAUImageBlockRespConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_OTAUCmdImageBlockResponseConfCallback_t(
        ZBPRO_ZCL_OTAUCmdImageBlockResponseReqDescr_t     *const  reqDescr,
        ZBPRO_ZCL_OTAUCmdImageBlockResponseConfParams_t   *const  confParams);

/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Image Block Response
 * command specific to OTA Upgrade ZCL cluster.
 * \ingroup ZBPRO_ZCL_OTAUImageBlockRespReq
 */
struct _ZBPRO_ZCL_OTAUCmdImageBlockResponseReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_OTAUCmdImageBlockResponseConfCallback_t     *callback;      /*!< ZCL Confirmation callback handler entry
                                                                               point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t                   service;       /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_OTAUCmdImageBlockResponseReqParams_t         params;        /*!< ZCL Request parameters structure. */
};


/**//**
 * \brief OTA Upgrade ZCL cluster Upgrade End Request command indication parameters.
 *
 * \ingroup ZBPRO_ZCL_OTAUUpgradeEndInd
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 11.13.9, figure 11-16.
 */
typedef struct _ZBPRO_ZCL_OTAUCmdUpgradeEndRequestIndParams_t
{
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;            /*!< Set of obligatory parameters of ZCL public
                                                                               interface to local application. */

    /* 32-bit data. */

    ZBPRO_ZCL_OTAUFileVersion_t  fileVersion;                             /*!< File version */

    /* 16-bit data. */

    ZBPRO_ZCL_ManufCode_t        otauManufacturerCode;                    /*!< Manufacturer code */
    ZBPRO_ZCL_OTAUImageType_t    imageType;                               /*!< Image type */

    /* 8-bit data. */

    ZBPRO_ZCL_Status_t           status;                                  /*!< Request status */

} ZBPRO_ZCL_OTAUCmdUpgradeEndRequestIndParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_OTAUCmdUpgradeEndRequestIndParams_t);



/**//**
 * \brief   Structure for parameters of ZCL Local Request to issue Upgrade End Response
 * command specific to OTA Upgrade ZCL cluster.
 * \ingroup ZBPRO_ZCL_OTAUUpgradeEndRespReq
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 11.13.9.6.1., figure 11-17.
 */
typedef struct _ZBPRO_ZCL_OTAUCmdUpgradeEndResponseParams_t
{
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveObligatoryPart_t    zclObligatoryPart;          /*!< Set of obligatory parameters of ZCL public
                                                                               interface to local application. */
    /* 32-bit data. */

    ZBPRO_ZCL_OTAUFileVersion_t   fileVersion;                            /*!< File version */
    uint32_t                      currentTime;                            /*!< Current time */
    uint32_t                      upgradeTime;                            /*!< Upgrade time */

     /* 16-bit data. */

    ZBPRO_ZCL_ManufCode_t         otauManufacturerCode;                   /*!< Manufacturer code */
    ZBPRO_ZCL_OTAUImageType_t     imageType;                              /*!< Image type */

} ZBPRO_ZCL_OTAUCmdUpgradeEndResponseParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_OTAUCmdUpgradeEndResponseParams_t);

/**//**
 * \brief   Structure for parameters of ZCL Local Confirmation on request to issue Upgrade End Response
 * command specific to OTA Upgrade ZCL cluster.
 * \ingroup ZBPRO_ZCL_OTAUUpgradeEndRespConf
 * \note
 * This command has no response. So, Confirmation will be empty (with obligatory parameters only).
 * \par     Documentation
 *  See ZigBee Document 075123r05, subclause 11.13.9.6.
 */
typedef struct _ZBPRO_ZCL_OTAUCmdUpgradeEndResponseConfParams_t
{
    ZbProZclLocalPrimitiveObligatoryPart_t  zclObligatoryPart;            /*!< Set of obligatory parameters of ZCL public
                                                                               interface to local application. */
    /* No custom parameters. */

} ZBPRO_ZCL_OTAUCmdUpgradeEndResponseConfParams_t;
ZBPRO_ZCL_VALIDATE_LOCAL_PRIMITIVE_PARAMS_STRUCT(ZBPRO_ZCL_OTAUCmdUpgradeEndResponseConfParams_t);

/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Upgrade End Response
 * \ingroup ZBPRO_ZCL_OTAUUpgradeEndRespReq
 * command specific to OTA Upgrade ZCL cluster.
 */
typedef struct   _ZBPRO_ZCL_OTAUCmdUpgradeEndResponseReqDescr_t     ZBPRO_ZCL_OTAUCmdUpgradeEndResponseReqDescr_t;

/**//**
 * \brief   Data type for ZCL Local Confirmation callback function of Upgrade End Response
 * command specific to OTA Upgrade ZCL cluster.
 * \ingroup ZBPRO_ZCL_OTAUUpgradeEndRespConf
 * \param[in]   reqDescr        Pointer to the descriptor of request being confirmed.
 * \param[in]   confParams      Pointer to the confirmation parameters structure.
 */
typedef void ZBPRO_ZCL_OTAUCmdUpgradeEndResponseConfCallback_t(
        ZBPRO_ZCL_OTAUCmdUpgradeEndResponseReqDescr_t     *const  reqDescr,
        ZBPRO_ZCL_OTAUCmdUpgradeEndResponseConfParams_t   *const  confParams);

/**//**
 * \brief   Structure for descriptor of ZCL Local Request to issue Image Block Response
 * command specific to OTA Upgrade ZCL cluster.
 * \ingroup ZBPRO_ZCL_OTAUUpgradeEndRespReq
 */
struct _ZBPRO_ZCL_OTAUCmdUpgradeEndResponseReqDescr_t
{
    /* 32-bit data. */

    ZBPRO_ZCL_OTAUCmdUpgradeEndResponseConfCallback_t   *callback;        /*!< ZCL Confirmation callback handler entry
                                                                               point. */
    /* Structured data, aligned at 32 bits. */

    ZbProZclLocalPrimitiveDescrService_t                 service;         /*!< ZCL Request Descriptor service field. */

    ZBPRO_ZCL_OTAUCmdUpgradeEndResponseParams_t          params;          /*!< ZCL Request parameters structure. */
};

/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   Handles ZCL Local Indication on reception of Query Next Image Request command
 * specific to OTA Upgrade ZCL cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 * \note
 *  The Query Next Image Request command is sent by client. Due
 *  to this reason server side service provides this function.
 * \return Nothing.
 */
void ZBPRO_ZCL_OTAUCmdQueryNextImageRequestInd(ZBPRO_ZCL_OTAUCmdQueryNextImageRequestIndParams_t *const indParams);

/**//**
 * \brief   Accepts ZCL Local Request to issue Query Next Image Response
 *  command specific to OTA Upgrade ZCL cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_OTAUCmdQueryNextImageResponseReq(ZBPRO_ZCL_OTAUCmdQueryNextImageResponseReqDescr_t *const  reqDescr);


/**//**
 * \brief   Handles ZCL Local Indication on reception of Image Block Request command
 * specific to OTA Upgrade ZCL cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 * \note
 *  The Query Next Image Request command is sent by client. Due
 *  to this reason server side service provides this function.
 * \return Nothing.
 */
void ZBPRO_ZCL_OTAUCmdImageBlockRequestInd(ZBPRO_ZCL_OTAUCmdImageBlockRequestIndParams_t *const indParams);

/**//**
 * \brief   Accepts ZCL Local Request to issue Image Block Response
 *  command specific to OTA Upgrade ZCL cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_OTAUCmdImageBlockResponseReq(ZBPRO_ZCL_OTAUCmdImageBlockResponseReqDescr_t *const  reqDescr);


/**//**
 * \brief   Handles ZCL Local Indication on reception of Upgrade End Request command
 * specific to OTA Upgrade ZCL cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   indParams       Pointer to ZCL Local Indication parameters.
 * \note
 *  The Query Next Image Request command is sent by client. Due
 *  to this reason server side service provides this function.
 * \return Nothing.
 */
void ZBPRO_ZCL_OTAUCmdUpgradeEndRequestInd(ZBPRO_ZCL_OTAUCmdUpgradeEndRequestIndParams_t *const indParams);

/**//**
 * \brief   Accepts ZCL Local Request to issue Upgrade End Response
 *  command specific to OTA Upgrade ZCL cluster.
 * \ingroup ZBPRO_ZCL_Functions
 * \param[in]   reqDescr        Pointer to ZCL Local Request descriptor.
 * \return Nothing.
 */
void ZBPRO_ZCL_OTAUCmdUpgradeEndResponseReq(ZBPRO_ZCL_OTAUCmdUpgradeEndResponseReqDescr_t *const  reqDescr);

#endif /* _BB_ZBPRO_ZCL_SAP_CLUSTER_OTAU_H */

/* eof bbZbProZclSapClusterOTAU.h */
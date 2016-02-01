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
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/bbMacSapTypesAssociate.h $
*
* DESCRIPTION:
*   MLME-ASSOCIATE service data types definition.
*
* $Revision: 3417 $
* $Date: 2014-08-27 16:19:14Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_SAP_TYPES_ASSOCIATE_H
#define _BB_MAC_SAP_TYPES_ASSOCIATE_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapTypesCommStatus.h"    /* MLME-COMM-STATUS service data types. */
#include "bbMacSapService.h"            /* MAC-SAP service data types. */


/************************* VALIDATIONS **************************************************/
#if !defined(_MAC_CONTEXT_ZBPRO_)
# error This header shall be compiled only if the ZigBee PRO context is included into the build.
#endif


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration of values of the Alternate PAN Coordinator field of the device
 *  Capability Information parameter.
 * \note    This implementation of the ZigBee PRO stack is not capable to serve as an
 *  alternative PAN coordinator.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.3.1.2, figure 56.
 */
typedef enum _MAC_CapabilityAltPanCoord
{
    MAC_CAPABILITY_NOT_ALT_PAN_COORD = 0,       /*!< Device is not capable of becoming the PAN coordinator. */

    MAC_CAPABILITY_ALT_PAN_COORD     = 1,       /*!< Device is capable of becoming the PAN coordinator (i.e., it may
                                                    serve as an alternative PAN coordinator). */
} MAC_CapabilityAltPanCoord;


/**//**
 * \brief   Enumeration of values of the Device Type field of the device Capability
 *  Information parameter.
 * \note    This implementation of the ZigBee PRO stack is mostly the FFD, it is not
 *  capable to serve as an RFD.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.3.1.2, figure 56.
 */
typedef enum _MAC_CapabilityDeviceType
{
    MAC_CAPABILITY_DEVICE_TYPE_RFD = 0,     /*!< Device is an RFD. */

    MAC_CAPABILITY_DEVICE_TYPE_FFD = 1,     /*!< Device is an FFD. */

} MAC_CapabilityDeviceType;


/**//**
 * \brief   Enumeration of values of the Power Source field of the device Capability
 *  Information parameter.
 * \note    This implementation of the ZigBee PRO stack awaits power from the alternating
 *  current mains. The ZigBee RF4CE Controller stack, which will be battery powered, is
 *  not covered by this enumeration, because the ZigBee RF4CE does not implement
 *  association.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.3.1.2, figure 56.
 */
typedef enum _MAC_CapabilityPowerSource
{
    MAC_CAPABILITY_POWER_SOURCE_BATTERY = 0,        /*!< Device is powered from a battery or other limited source. */

    MAC_CAPABILITY_POWER_SOURCE_MAINS   = 1,        /*!< Device is powered from the alternating current mains. */

} MAC_CapabilityPowerSource;


/**//**
 * \brief   Enumeration of values of the Receiver On When Idle field of the device
 *  Capability Information parameter.
 * \note    This implementation of the ZigBee PRO stack will have transceiver enabled for
 *  reception continuously when not transmitting; it will not implement sleeping device.
 *  The ZigBee RF4CE Controller stack, which will be the battery powered sleeping device,
 *  is not covered by this enumeration, because the ZigBee RF4CE does not implement
 *  association.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.3.1.2, figure 56.
 */
typedef enum _MAC_CapabilityRxOnWhenIdle
{
    MAC_CAPABILITY_RX_MAY_OFF_WHEN_IDLE   = 0,      /*!< Device may disable its receiver during idle periods. */

    MAC_CAPABILITY_RX_ALWAYS_ON_WHEN_IDLE = 1,      /*!< Device does not disable its receiver during idle periods. */

} MAC_CapabilityRxOnWhenIdle;


/**//**
 * \brief   Enumeration of values of the MAC Security Capability field of the device
 *  Capability Information parameter.
 * \note    This implementation of the MAC does not support the MAC security.
 * \details The MAC will indicate the invalid Capability Information parameter value if
 *  the MLME-ASSOCIATE.request is called by the higher layer with the MAC Security
 *  Capability field set to enabled.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.3.1.2, figure 56.
 */
typedef enum _MAC_CapabilitySecurity
{
    MAC_CAPABILITY_NO_SECURITY = 0,     /*!< Device is not capable of sending and receiving secured MAC frames. */

    MAC_CAPABILITY_SECURITY    = 1,     /*!< Device is capable of sending and receiving secured MAC frames. */

} MAC_CapabilitySecurity;


/**//**
 * \brief   Enumeration of values of the Allocate 16-bit Address field of the device
 *  Capability Information parameter.
 * \note    This implementation of the ZigBee PRO stack, when issuing the
 *  MLME-ASSOCIATE.request to the MAC in order to join a PAN as a new member, will ask the
 *  coordinator to allocate a 16-bit short address.
 * \details The MAC will log warning if it receives a successful association response from
 *  the coordinator but the address assignment performed by the coordinator and returned
 *  in the association response does not coincide with the original request (i.e., the
 *  coordinator did not assign a short address when it was requested to do, or it assigned
 *  the short address when it was requested not to do).
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.3.1.2, figure 56.
 */
typedef enum _MAC_CapabilityAllocAddress
{
    MAC_CAPABILITY_USE_64BIT_ADDR   = 0,        /*!< Device wishes to use its 64-bit extended address for
                                                    communications. */

    MAC_CAPABILITY_ALLOC_16BIT_ADDR = 1,        /*!< Device wishes the coordinator to allocate a 16-bit short address as
                                                    a result of the association procedure. */
} MAC_CapabilityAllocAddress;


/**//**
 * \brief   Structure for the CapabilityInformation parameter of the
 *  MLME-ASSOCIATE.request/indication primitives and Capability Information bitmap field
 *  of the Association Request MAC Command frame.
 * \details This implementation of the MAC allows the higher layer to request association
 *  with arbitrary capability information except the MAC Security Capability which must be
 *  set to FALSE.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.3.1.2, figure 56.
 */
typedef union _MAC_CapabilityInfo_t
{
    /* Structured / 8-bit data. */
    struct
    {
        MAC_CapabilityAltPanCoord   altPanCoord     : 1;        /*!< If the device is capable of becoming the PAN
                                                                    coordinator. */

        MAC_CapabilityDeviceType    deviceType      : 1;        /*!< If the device is an FFD. */

        MAC_CapabilityPowerSource   powerSource     : 1;        /*!< If the device is receiving power from the
                                                                    alternating current mains. */

        MAC_CapabilityRxOnWhenIdle  rxOnWhenIdle    : 1;        /*!< If the device does not disable its receiver to
                                                                    conserve power during idle periods. */

        BitField8_t                 reserved        : 2;        /*!< Reserved. */

        MAC_CapabilitySecurity      securCapability : 1;        /*!< If the device is capable of sending and receiving
                                                                    cryptographically protected MAC frames. */

        MAC_CapabilityAllocAddress  allocAddress    : 1;        /*!< If the device wishes the coordinator to allocate a
                                                                    16-bit short address as a result of the association
                                                                    procedure. */
    };

    /* 8-bit data. */
    uint8_t                         plain;                      /*!< Plain data. */

} MAC_CapabilityInfo_t;


/**//**
 * \brief   Structure for parameters of the MLME-ASSOCIATE.request.
 * \note    Security parameters are excluded because the MAC Security is not implemented.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.1.3.1.1, table 47.
 */
typedef struct _MAC_AssociateReqParams_t
{
    /* 64-bit data. */
    MAC_Address_t           coordAddress;       /*!< The address of the coordinator with which to associate. */

    /* 16-bit data. */
    MAC_PanId_t             coordPanId;         /*!< The identifier of the PAN with which to associate. */

    /* 8-bit data. */
    MAC_AddrMode_t          coordAddrMode;      /*!< The coordinator addressing mode for this primitive and subsequent
                                                    MPDU. */

    PHY_LogicalChannelId_t  logicalChannel;     /*!< The logical channel on which to attempt association. */

    PHY_ChannelPageId_t     channelPage;        /*!< The channel page on which to attempt association. */

    MAC_CapabilityInfo_t    capabilityInfo;     /*!< Specifies the operational capabilities of the associating
                                                    device. */

    /* TODO: This field is redundant. Wrap it with a conditional build key. */
    MAC_SecurityLevel_t     securityLevel;      /*!< The security level to be used. */

} MAC_AssociateReqParams_t;


/**//**
 * \brief   Structure for parameters of the MLME-ASSOCIATE.confirm.
 * \details Possible values for the \c status parameter are the following:
 *  - SUCCESS                   The requested association was completed successfully.
 *  - CHANNEL_ACCESS_FAILURE    The Association Request MAC Command frame transmission
 *      could not take place due to activity on the channel, i.e., the CSMA-CA mechanism
 *      has failed.
 *  - NO_ACK                    No acknowledgment was received after macMaxFrameRetries on
 *      the transmitted Association Request MAC Command frame.
 *  - NO_DATA                   No Association Response MAC Command frame were available
 *      following a request.
 *  - INVALID_PARAMETER         A parameter in the primitive is either not supported or is
 *      out of the valid range.
 *  - RESET                     An MLME-RESET.request was issued prior to execution of the
 *      MLME-ASSOCIATE.request being confirmed.
 *
 * \note    The following values for the \c status parameter are not used due to listed
 *  reasons:
 *  - COUNTER_ERROR, FRAME_TOO_LONG, IMPROPER_KEY_TYPE, IMPROPER_SECURITY_LEVEL,
 *      SECURITY_ERROR, UNAVAILABLE_KEY, UNSUPPORTED_LEGACY,
 *      UNSUPPORTED_SECURITY        This implementation of the MAC does not support the
 *      MAC Security.
 *
 * \note    Security parameters are excluded because the MAC Security is not implemented.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.1.3.4.1, table 47.
 */
typedef struct _MAC_AssociateConfParams_t
{
    /* 16-bit data. */
    MAC_Addr16bit_t  assocShortAddress;     /*!< The short device address allocated by the coordinator on successful
                                                association. */
    /* 8-bit data. */
    MAC_Status_t     status;                /*!< The status of the association attempt. */

    /* TODO: This field is redundant. Wrap it with a conditional build key. */
    MAC_SecurityLevel_t  securityLevel;         /*!< The security level to be used; or the security level purportedly
                                                    used by the received frame. */
} MAC_AssociateConfParams_t;


/**//**
 * \brief   Structure for descriptor of the MLME-ASSOCIATE.request.
 */
typedef struct _MAC_AssociateReqDescr_t  MAC_AssociateReqDescr_t;


/**//**
 * \brief   Template for the callback handler-function of the MLME-ASSOCIATE.confirm.
 * \param[in]   reqDescr    Pointer to the confirmed request descriptor.
 * \param[in]   confParams  Pointer to the confirmation parameters object.
 * \details Call function of this type provided by the higher layer from the MAC to issue
 *  the MLME-ASSOCIATE.confirm to the ZigBee PRO higher layer that originally issued the
 *  request primitive to the MAC.
 * \details To issue the confirmation primitive the MAC calls the confirmation callback
 *  handler-function that was specified with the \c callback parameter of the original
 *  request primitive descriptor that is pointed here by the \p reqDescr argument.
 * \details The request descriptor object that was originally used to issue request to the
 *  MAC and is pointed here with the \p reqDescr is released by the MAC for random use by
 *  the higher layer (the owner of the request descriptor object) when this confirmation
 *  callback handler-function is called by the MAC.
 * \details Treat the parameters structure pointed by the \p confParams and passed into
 *  the confirmation callback handler-function as it has been allocated in the program
 *  stack by the MAC before calling this callback function and will be destroyed just
 *  after this callback function returns.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.1.3.4.
 */
typedef void MAC_AssociateConfCallback_t(MAC_AssociateReqDescr_t   *const reqDescr,
                                         MAC_AssociateConfParams_t *const confParams);


/**//**
 * \brief   Structure for descriptor of the MLME-ASSOCIATE.request.
 */
struct _MAC_AssociateReqDescr_t
{
    /* 32-bit data. */
    MAC_AssociateConfCallback_t *callback;      /*!< Entry point of the confirmation callback function. */

    /* Structured data. */
    MacServiceField_t            service;       /*!< MAC requests service field. */

    MAC_AssociateReqParams_t     params;        /*!< Request parameters structured object. */
};


/**//**
 * \brief   Structure for parameters of the MLME-ASSOCIATE.indication.
 * \note    Security parameters are excluded because the MAC Security is not implemented.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.1.3.2.1, table 48.
 */
typedef struct _MAC_AssociateIndParams_t
{
    /* 64-bit data. */
    MAC_Addr64bit_t       deviceAddress;        /*!< The 64-bit address of the device requesting association. */

    /* 8-bit data. */
    MAC_CapabilityInfo_t  capabilityInfo;       /*!< The operational capabilities of the device requesting
                                                    association. */

    PHY_Lqi_t             mpduLinkQuality;      /*!< LQI value measured during reception of the association request. */

    /* TODO: This field is redundant. Wrap it with a conditional build key. */
    MAC_SecurityLevel_t   securityLevel;        /*!< The security level purportedly used by the received MAC command
                                                    frame. */
} MAC_AssociateIndParams_t;


/**//**
 * \brief   Template for the callback handler-function of the MLME-ASSOCIATE.indication.
 * \param[in]   indParams   Pointer to the indication parameters object.
 * \details Call function of this type provided by the higher layer from the MAC to issue
 *  the MLME-ASSOCIATE.indication to the destination ZigBee PRO higher layer. The
 *  indication callback handler-function must be statically linked in the project.
 * \details Treat the parameters structure pointed by the \p indParams and passed into the
 *  indication callback handler-function as it has been allocated in the program stack by
 *  the MAC before calling this callback function and will be destroyed just after this
 *  callback function returns.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.1.3.2.
 */
typedef void MAC_AssociateIndCallback_t(MAC_AssociateIndParams_t *const indParams);


/**//**
 * \brief   Structure for parameters of the MLME-ASSOCIATE.response.
 * \details Possible values for the \c status parameter are the following:
 *  - ASSOCIATION_SUCCESSFUL    The higher layer confirms the requested association.
 *  - PAN_AT_CAPACITY           The higher layer refuses the requested association due to
 *      the PAN capacity limit reached.
 *  - PAN_ACCESS_DENIED         The higher layer refuses the requested association due to
 *      restrictions on access to the PAN.
 *
 * \note    The following custom parameters are included due to the following reasons:
 *  - devicePanId       This parameter shall not be assigned by the higher layer when
 *      issuing the MLME-ASSOCIATE.response; it is assigned by the MAC internally during
 *      processing of the response. The MAC stores in this parameter the current value of
 *      its macPANId attribute at the moment of the MLME-ASSOCIATE.response reception from
 *      the higher layer in order to set the SrcPANId field properly in the Association
 *      Response MAC Command frame when it will be transmitted on the discretion of the
 *      device requesting association.
 *
 * \note    Security parameters are excluded because the MAC Security is not implemented.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclause 7.1.3.3.1, table 49.
 */
typedef struct _MAC_AssociateRespParams_t
{
    /* 64-bit data. */
    MAC_Addr64bit_t  deviceAddress;         /*!< The 64-bit address of the device requesting association. */

    /* 16-bit data. */
    MAC_Addr16bit_t  assocShortAddress;     /*!< The 16-bit short device address allocated by the coordinator on
                                                successful association. */

    MAC_PanId_t      devicePanId;           /*!< The 16-bit PAN identifier of the device requesting association. This
                                                 field is assigned by the MAC according to its macPANId attribute
                                                 current value while processing the MLME-ASSOCIATE.response. */
    /* 8-bit data. */
    MAC_Status_t         status;                /*!< The status of the association attempt. */

    /* TODO: This field is redundant. Wrap it with a conditional build key. */
    MAC_SecurityLevel_t  securityLevel;         /*!< The security level to be used. */

} MAC_AssociateRespParams_t;


/**//**
 * \brief   Structure for parameters of the MLME-COMM-STATUS-ASSOCIATE.indication.
 * \details This is a particular case of the common MLME-COMM-STATUS.indication when it is
 *  issued by the MAC to the higher layer as a confirmation on a previous
 *  MLME-ASSOCIATE.response.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclauses 7.1.3.3, 7.1.12.1, table 69.
 */
typedef MAC_CommStatusIndParams_t  MAC_CommStatusAssociateIndParams_t;


/**//**
 * \brief   Structure for descriptor of the MLME-ASSOCIATE.response.
 */
typedef struct _MAC_AssociateRespDescr_t  MAC_AssociateRespDescr_t;


/**//**
 * \brief   Template for the callback handler-function of the
 *  MLME-COMM-STATUS-ASSOCIATE.indication.
 * \param[in]   respDescr   Pointer to the confirmed response descriptor.
 * \param[in]   indParams   Pointer to the indication parameters object.
 * \details Call function of this type provided by the higher layer from the MAC to issue
 *  the MLME-COMM-STATUS.indication to the ZigBee PRO higher layer that originally issued
 *  the MLME-ASSOCIATE.response primitive to the MAC.
 * \details To issue the indication primitive the MAC calls the indication callback
 *  handler-function that was specified with the \c callback parameter of the original
 *  response primitive descriptor that is pointed here by the \p respDescr parameter.
 * \details The response descriptor object that was originally used to issue response to
 *  the MAC and is pointed here with the \p respDescr is released by the MAC for random
 *  use by the higher layer (the owner of the response descriptor object) when this
 *  indication callback handler-function is called by the MAC.
 * \details Treat the parameters structure pointed by the \p indParams and passed into the
 *  indication callback handler-function as it has been allocated in the program stack by
 *  the MAC before calling this callback function and will be destroyed just after this
 *  callback function returns.
 * \par     Documentation
 *  See IEEE Std 802.15.4-2006, subclauses 7.1.3.3, 7.1.12.1.
 */
typedef void MAC_CommStatusAssociateIndCallback_t(MAC_AssociateRespDescr_t           *const respDescr,
                                                  MAC_CommStatusAssociateIndParams_t *const indParams);


/**//**
 * \brief   Structure for descriptor of the MLME-ASSOCIATE.response.
 */
struct _MAC_AssociateRespDescr_t
{
    /* 32-bit data. */
    MAC_CommStatusAssociateIndCallback_t *callback;     /*!< Entry point of the confirmation callback function. */

    /* Structured data. */
    MacServiceField_t                     service;      /*!< MAC requests service field. */

    MAC_AssociateRespParams_t             params;       /*!< Response parameters structured object. */
};


#endif /* _BB_MAC_SAP_TYPES_ASSOCIATE_H */
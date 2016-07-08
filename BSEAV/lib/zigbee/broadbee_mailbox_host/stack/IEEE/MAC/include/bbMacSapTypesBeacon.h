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
*
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/bbMacSapTypesBeacon.h $
*
* DESCRIPTION:
*   MLME-BEACON service data types definition.
*
* $Revision: 10263 $
* $Date: 2016-02-29 18:03:06Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_SAP_TYPES_BEACON_H
#define _BB_MAC_SAP_TYPES_BEACON_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapPib.h"            /* MAC-PIB for MAC-SAP definitions. */


/************************* VALIDATIONS **************************************************/
#if defined(_MAC_CONTEXT_RF4CE_CONTROLLER_) && !defined(MAILBOX_UNIT_TEST)
# error This header is not for the RF4CE-Controller build.
#endif


/************************* DEFINITIONS **************************************************/
#if defined(_MAC_TESTER_)
/**//**
 * \brief   Structure for parameters of MLME-BEACON.confirm primitive.
 * \note    This is a nonstandard primitive. It is used in conjunction with
 *  MLME-BEACON.response for the MAC Tester build configuration to provide ability to
 *  transmit beacons by the command from the Test Harness but not only on reception of
 *  the Beacon Request MAC Command.
 */
typedef struct _MAC_BeaconConfParams_t
{
    /* 32-bit data. */
    HAL_Symbol__Tstamp_t   timestamp;       /*!< The time, in symbols, at which the beacon was transmitted. */

    /* 8-bit data. */
    MAC_Status_t           status;          /*!< The status of the last beacon transmission. */

} MAC_BeaconConfParams_t;


/**//**
 * \brief   Structure for descriptor of MLME-BEACON.response.
 */
typedef struct _MAC_BeaconRespDescr_t  MAC_BeaconRespDescr_t;


/**//**
 * \brief   Template for callback handler-function of MLME-BEACON.confirm primitive.
 * \param[in]   respDescr   Pointer to the confirmed response descriptor object.
 * \param[in]   confParams  Pointer to the confirmation parameters object.
 * \details Call functions of this type provided by higher layers of corresponding MAC
 *  Contexts, ZigBee PRO and RF4CE, from the MAC-FE to issue the confirmation primitive to
 *  the higher layer that originally issued the response primitive to the MAC.
 * \details To issue the confirmation primitive the MAC-FE calls the confirmation callback
 *  handler-function that was specified with the \c callback parameter of the original
 *  response primitive descriptor that is pointed here by the \p respDescr argument.
 * \details The response descriptor object that was originally used to issue the response
 *  to the MAC and is pointed here with the \p respDescr is released by the MAC for random
 *  use by the higher layer (the owner of the response descriptor object) when this
 *  confirmation callback handler-function is called by the MAC-FE.
 * \details Treat the parameters structure pointed by the \p confParams and passed into
 *  the confirmation callback handler-function as it has been allocated in the program
 *  stack by the MAC-FE before calling this callback handler and will be destroyed just
 *  after this callback function returns.
 * \note    This is a nonstandard primitive. It is used in conjunction with
 *  MLME-BEACON.response for the MAC Tester build configuration to provide ability to
 *  transmit beacons by the command from the Test Harness but not only on reception of
 *  the Beacon Request MAC Command.
 */
typedef void MAC_BeaconConfCallback_t(MAC_BeaconRespDescr_t *const  respDescr,
                                      MAC_BeaconConfParams_t       *confParams);


/**//**
 * \brief   Structure for descriptor of MLME-BEACON.response.
 * \note    This is a nonstandard primitive. It is used for the MAC Tester build
 *  configuration to provide ability to transmit beacons by the command from the Test
 *  Harness but not only on reception of the Beacon Request MAC Command.
 * \note    The MLME-BEACON.response primitive has no parameters.
 */
struct _MAC_BeaconRespDescr_t
{
    /* 32-bit data. */
    MAC_DataConfCallback_t *callback;       /*!< Entry point of the confirmation callback function. */

    /* Structured data. */
    MacServiceField_t       service;        /*!< MAC requests service field. */

};
#endif /* _MAC_TESTER_ */


/**//**
 * \brief   Structure for Superframe Specification object.
 * \note    This structure is also used for description of Beacon frame Superframe
 *  Specification field.
 * \note    As for the subfields related to beacon-enabled PANs, this implementation of
 *  MAC generates Beacon frames with the following values for these subfields:
 *  - \c BeaconOrder = 15, means that the device does not emit periodic beacons;
 *  - \c SuperframeOrder = 15;
 *  - \c FinalCapSlot = 15;
 *  - \c BatteryLifeExtension = FALSE.
 *
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 7.2.2.1.2, 7.5.1.1, and figures 47, 66.
 */
typedef union _MAC_SuperframeSpec_t
{
    BitField16_t                 plain;                         /*!< Plain data. */

    struct
    {
        MAC_BeaconOrder_t        beaconOrder          : 4;      /*!< The transmission interval of the beacon. */

        MAC_SuperframeOrder_t    superframeOrder      : 4;      /*!< The length of time during which
                                                                    the superframe is active. */

        uint8_t                  finalCapSlot         : 4;      /*!< Specifies the final superframe slot
                                                                    utilized by the CAP: from 0 to 15. */

        MAC_BattLifeExt_t        batteryLifeExtension : 1;      /*!< TRUE if frames transmitted to the beaconing device
                                                                    during its CAP are required to start in or before
                                                                    macBattLifeExtPeriods full backoff periods after the
                                                                    IFS period following the beacon; FALSE otherwise. */

        uint8_t                                       : 1;      /*!< Reserved. */

        MAC_PanCoordinator_t     panCoordinator       : 1;      /*!< TRUE if the beacon frame is being transmitted by
                                                                    the PAN coordinator; FALSE otherwise. */

        MAC_AssociationPermit_t  associationPermit    : 1;      /*!< TRUE if the coordinator is currently accepting
                                                                    association requests on its network; FALSE
                                                                    otherwise. */
    };
} MAC_SuperframeSpec_t;


/*
 * Macro-constants describing the bit-fields of the
 * Beacon frame MPDU.MSDU.SuperframeSpec subfield.
 */
#define MAC_BEACON_SFS_BEACON_ORDER_START             0     /*!< Beacon Order subfield starts at bit #0. */
#define MAC_BEACON_SFS_BEACON_ORDER_WIDTH             4     /*!< Beacon Order subfield has 4 bits width. */

#define MAC_BEACON_SFS_SUPERFRAME_ORDER_START         4     /*!< Superframe Order subfield starts at bit #4. */
#define MAC_BEACON_SFS_SUPERFRAME_ORDER_WIDTH         4     /*!< Superframe Order subfield has 4 bits width. */

#define MAC_BEACON_SFS_FINAL_CAP_SLOT_START           8     /*!< Final CAP Slot subfield starts at bit #8. */
#define MAC_BEACON_SFS_FINAL_CAP_SLOT_WIDTH           4     /*!< Final CAP Slot subfield has 4 bits width. */

#define MAC_BEACON_SFS_BATTERY_LIFE_EXTENSION_START   12    /*!< Battery Life Extension subfield starts at bit #12. */
#define MAC_BEACON_SFS_BATTERY_LIFE_EXTENSION_WIDTH   1     /*!< Battery Life Extension subfield has 1 bits width. */

#define MAC_BEACON_SFS_PAN_COORDINATOR_START          14    /*!< PAN Coordinator subfield starts at bit #14. */
#define MAC_BEACON_SFS_PAN_COORDINATOR_WIDTH          1     /*!< PAN Coordinator subfield has 1 bits width. */

#define MAC_BEACON_SFS_ASSOCIATION_PERMIT_START       15    /*!< Assosiation Permit subfield starts at bit #15. */
#define MAC_BEACON_SFS_ASSOCIATION_PERMIT_WIDTH       1     /*!< Assosiation Permit subfield has 1 bits width. */


/**//**
 * \brief   Macro to assemble MPDU.MSDU.SuperframeSpec field from the set of subfields.
 */
#define MAC_BEACON_MAKE_SUPERFRAME_SPEC(\
                beaconOrder,\
                superframeOrder,\
                finalCapSlot,\
                batteryLifeExtension,\
                panCoordinator,\
                associationPermit)\
        (((beaconOrder)          << MAC_BEACON_SFS_BEACON_ORDER_START)           |\
         ((superframeOrder)      << MAC_BEACON_SFS_SUPERFRAME_ORDER_START)       |\
         ((finalCapSlot)         << MAC_BEACON_SFS_FINAL_CAP_SLOT_START)         |\
         ((batteryLifeExtension) << MAC_BEACON_SFS_BATTERY_LIFE_EXTENSION_START) |\
         ((panCoordinator)       << MAC_BEACON_SFS_PAN_COORDINATOR_START)        |\
         ((associationPermit)    << MAC_BEACON_SFS_ASSOCIATION_PERMIT_START))


/**//**
 * \brief   Macro to assemble MPDU.MSDU.SuperframeSpec field from the set of subfields.
 * \details Set the following subfields MPDU.MSDU.SuperframeSpec field to constant values:
 *  - Beacon Order      set to 15 - it means 'nonbeacon-enabled PAN',
 *  - Superframe Order  set to 15 - it means 'superframes are not used',
 *  - Final CAP Slot    set to 15 - it means 'no CFP (contention-free period) exists',
 *  - Battery Life Extension    set to FALSE - the default value.
 */
#define MAC_BEACON_MAKE_SUPERFRAME_SPEC_FOR_NONBEACONENABLED(\
                panCoordinator,\
                associationPermit)\
        MAC_BEACON_MAKE_SUPERFRAME_SPEC(\
                15,\
                15,\
                15,\
                FALSE,\
                panCoordinator,\
                associationPermit)


/**//**
 * \brief   Structure for PAN Descriptor object.
 * \note    Field \c GTSPermit is excluded because this implementation of MAC is only for
 *  nonbeacon-enabled PANs.
 * \note    Security parameters are excluded because MAC security is not implemented.
 * \note    The size of PAN Descriptor data structure equals 20 bytes.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.5.1.1, table 55.
 */
typedef struct _MAC_PanDescriptor_t
{
    /* 64-bit data. */
    MAC_Address_t           coordAddress;       /*!< The address of the coordinator as specified
                                                    in the received beacon frame. */
    /* 32-bit data. */
    HAL_Symbol__Tstamp_t    timestamp;          /*!< The time at which the beacon frame was received, in symbols. */

    /* 16-bit data. */
    MAC_PanId_t             coordPanId;         /*!< The PAN identifier of the coordinator as specified
                                                    in the received beacon frame. */

    MAC_SuperframeSpec_t    superframeSpec;     /*!< The superframe specification as specified
                                                    in the received beacon frame. */
    /* 8-bit data. */
    MAC_AddrMode_t          coordAddrMode;      /*!< The coordinator addressing mode corresponding
                                                    to the received beacon frame. */

    PHY_Channel_t           logicalChannel;     /*!< The current logical channel occupied by the network. */

    PHY_Page_t              channelPage;        /*!< The current channel page occupied by the network. */

    /* TODO: This field is redundant. Wrap it with a conditional build key. */
    Bool8_t                 gtsPermit;          /*!< TRUE if the beacon is from the PAN coordinator that is accepting
                                                    GTS requests.*/

    PHY_LQI_t               linkQuality;        /*!< The LQI at which the network beacon was received. */

    /* TODO: This field is redundant. Wrap it with a conditional build key. */
    MAC_Status_t            securityFailure;    /*!< One of status codes indicating an error in the security processing
                                                    or SUCCESS. */

    /* TODO: This field is redundant. Wrap it with a conditional build key. */
    MAC_SecurityLevel_t     securityLevel;      /*!< The security level purportedly used by the received beacon
                                                    frame. */

} MAC_PanDescriptor_t;


/**//**
 * \brief   Structure for parameters of MLME-BEACON-NOTIFY.indication primitive.
 * \note    Fields \c PendAddrSpec and \c AddrList are excluded because this
 *  implementation of MAC is only for nonbeacon-enabled PANs.
 * \note    Fields \c sduLength and \c sdu are implemented via \c payload.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.5.1, table 54.
 */
typedef struct _MAC_BeaconNotifyIndParams_t
{
    /* 32-bit data. */
    SYS_DataPointer_t    payload;           /*!< The set of octets comprising the beacon payload to be transferred from
                                                the MAC sublayer entity to the next higher layer. */
    /* Structured data. */
    MAC_PanDescriptor_t  panDescriptor;     /*!< The PANDescriptor for the received beacon. */

    /* 8-bit data. */
    MAC_Bsn_t            bsn;               /*!< The beacon sequence number */

} MAC_BeaconNotifyIndParams_t;


/**//**
 * \brief   Template for callback handler-function of MLME-BEACON-NOTIFY.indication
 *  primitive.
 * \param[in]   indParams   Pointer to the indication parameters object.
 * \details Call functions of this type provided by higher layers of corresponding MAC
 *  contexts, ZigBee PRO and RF4CE, from the MAC to issue the
 *  MLME-BEACON-NOTIFY.indication to the ZigBee PRO higher layer and/or RF4CE higher
 *  layer.
 * \note    The Beacon frames are always addressed to both stacks, ZigBee PRO and RF4CE;
 *  the indication handler from ZigBee PRO is called first and then the indication handler
 *  from RF4CE is called. Each handler function is called with its own copy of indication
 *  parameters and the Beacon payload contained in parameters, so both stacks higher
 *  layers are free to destroy or dismiss their copies of indication parameters in any
 *  way.
 * \note    Treat the parameters structure pointed by the \p indParams and passed into the
 *  indication callback handler-function as it has been allocated in the program stack by
 *  the MAC before calling this callback handler and will be destroyed just after this
 *  callback returns. The payload pointed by \p indParams->payload is allocated by MAC in
 *  the dynamic memory and must be dismissed by the higher layer; each stack higher layer
 *  must dismiss its own copy of payload.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.5.1.
 */
typedef void MAC_BeaconNotifyIndCallback_t(MAC_BeaconNotifyIndParams_t *const indParams);


#endif /* _BB_MAC_SAP_TYPES_BEACON_H */

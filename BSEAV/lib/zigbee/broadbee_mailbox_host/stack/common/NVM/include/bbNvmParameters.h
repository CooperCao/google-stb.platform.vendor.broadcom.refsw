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

/*******************************************************************************
 *
 * DESCRIPTION:
 *      Parameters defenition for NVM.
 *
*******************************************************************************/

#include "bbSysRepeatMacro.h"

#ifdef _RF4CE_
    PARAMETER_DECLARATION(NVM_RF4CE_NWK_FRAME_COUNTER, &GET_RF4CE_NWK_STATIC_DATA_FIELD()->NIB.frameCounter, sizeof(GET_RF4CE_NWK_STATIC_DATA_FIELD()->NIB.frameCounter))
    PARAMETER_DECLARATION(NVM_RF4CE_NWK_NIB, &GET_RF4CE_NWK_STATIC_DATA_FIELD()->NIB.storable, sizeof(GET_RF4CE_NWK_STATIC_DATA_FIELD()->NIB.storable))
    PARAMETER_DECLARATION(NVM_RF4CE_NWK_PANID, &GET_RF4CE_NWK_STATIC_DATA_FIELD()->panId, sizeof(GET_RF4CE_NWK_STATIC_DATA_FIELD()->panId))
    PARAMETER_DECLARATION(NVM_RF4CE_NWK_SHORT_ADDRESS, &GET_RF4CE_NWK_STATIC_DATA_FIELD()->shortAddress, sizeof(GET_RF4CE_NWK_STATIC_DATA_FIELD()->shortAddress))
#  if (1 == USE_RF4CE_PROFILE_GDP) || (1 == USE_RF4CE_PROFILE_ZRC) || (1 == USE_RF4CE_PROFILE_MSO)
    PARAMETER_DECLARATION(NVM_RF4CE_PM, &GET_RF4CE_PM_STATIC_DATA_FIELD()->storableProfilesData[0], sizeof(GET_RF4CE_PM_STATIC_DATA_FIELD()->storableProfilesData))
#   if (1 == USE_RF4CE_PROFILE_ZRC)
    PARAMETER_DECLARATION(NVM_RF4CE_ZRC, &GET_RF4CE_ZRC_STATIC_DATA_FIELD()->attributes, sizeof(GET_RF4CE_ZRC_STATIC_DATA_FIELD()->attributes))

#       if defined(USE_RF4CE_PROFILE_ZRC2)

    UNION_DECLARATION(NVM_RF4CE_ZRC2_RAM_ATTRIBUTES_FULL, &GET_RF4CE_ZRC_STATIC_DATA_FIELD()->zrc2Attributes, sizeof(GET_RF4CE_ZRC_STATIC_DATA_FIELD()->zrc2Attributes))

#           define NVM_RF4CE_ZRC2_RAM_ATTRIBUTES_OF_PEER_DECLARE(useless, pairIndex) \
                    PARAMETER_DECLARATION( \
                            NVM_RF4CE_ZRC2_SIMPLE_ATTRIBUTES_PAIR##pairIndex, \
                            &GET_RF4CE_ZRC_STATIC_DATA_FIELD()->zrc2Attributes[pairIndex].simple, \
                            sizeof(GET_RF4CE_ZRC_STATIC_DATA_FIELD()->zrc2Attributes[pairIndex].simple)) \
                    PARAMETER_DECLARATION( \
                            NVM_RF4CE_ZRC2_POLLCONFIGURATION_ATTRIBUTES_PAIR##pairIndex, \
                            &GET_RF4CE_ZRC_STATIC_DATA_FIELD()->zrc2Attributes[pairIndex].pollConfiguration, \
                            sizeof(GET_RF4CE_ZRC_STATIC_DATA_FIELD()->zrc2Attributes[pairIndex].pollConfiguration)) \
                    PARAMETER_DECLARATION( \
                            NVM_RF4CE_ZRC2_ATTRIBUTES_EXPLICITPADDING1_PAIR##pairIndex, \
                            &GET_RF4CE_ZRC_STATIC_DATA_FIELD()->zrc2Attributes[pairIndex].explicitPaddingForRamToNvmMapping1, \
                            sizeof(GET_RF4CE_ZRC_STATIC_DATA_FIELD()->zrc2Attributes[pairIndex].explicitPaddingForRamToNvmMapping1)) \
                    PARAMETER_DECLARATION( \
                            NVM_RF4CE_ZRC2_ATTRIBUTES_EXPLICITPADDING2_PAIR##pairIndex, \
                            &GET_RF4CE_ZRC_STATIC_DATA_FIELD()->zrc2Attributes[pairIndex].explicitPaddingForRamToNvmMapping2, \
                            sizeof(GET_RF4CE_ZRC_STATIC_DATA_FIELD()->zrc2Attributes[pairIndex].explicitPaddingForRamToNvmMapping2)) \
                    PARAMETER_DECLARATION( \
                            NVM_RF4CE_ZRC2_ATTRIBUTE_MAPPABLE_ACTIONS_PAIR##pairIndex, \
                            &GET_RF4CE_ZRC_STATIC_DATA_FIELD()->zrc2Attributes[pairIndex].mappableActions, \
                            sizeof(GET_RF4CE_ZRC_STATIC_DATA_FIELD()->zrc2Attributes[pairIndex].mappableActions))
    REPEAT_MACRO(NVM_RF4CE_ZRC2_RAM_ATTRIBUTES_OF_PEER_DECLARE, , RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES)
    NVM_RF4CE_ZRC2_RAM_ATTRIBUTES_OF_PEER_DECLARE( , RF4CE_ZRC2_ATTR_LOCAL_INDEX) /* LOCAL */

#           define NVM_RF4CE_ZRC2_ATTRIBUTE_ACTION_CODES_SUPPORTED_RX_DECLARE(useless, pairIndex) \
                    ARRAY_DECLARATION( \
                            NVM_RF4CE_ZRC2_ATTRIBUTE_ACTION_CODES_SUPPORTED_RX_PAIR##pairIndex, \
                            &GET_RF4CE_ZRC_STATIC_DATA_FIELD()->tmpBitmap256, \
                            RF4CE_ZRC2_ACTION_BANKS_MAX * sizeof(GET_RF4CE_ZRC_STATIC_DATA_FIELD()->tmpBitmap256))
    REPEAT_MACRO(NVM_RF4CE_ZRC2_ATTRIBUTE_ACTION_CODES_SUPPORTED_RX_DECLARE, , RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES)
    NVM_RF4CE_ZRC2_ATTRIBUTE_ACTION_CODES_SUPPORTED_RX_DECLARE( , RF4CE_ZRC2_ATTR_LOCAL_INDEX) /* LOCAL */

#           define NVM_RF4CE_ZRC2_ATTRIBUTE_ACTION_CODES_SUPPORTED_TX_DECLARE(useless, pairIndex) \
                    ARRAY_DECLARATION( \
                            NVM_RF4CE_ZRC2_ATTRIBUTE_ACTION_CODES_SUPPORTED_TX_PAIR##pairIndex, \
                            &GET_RF4CE_ZRC_STATIC_DATA_FIELD()->tmpBitmap256, \
                            RF4CE_ZRC2_ACTION_BANKS_MAX * sizeof(GET_RF4CE_ZRC_STATIC_DATA_FIELD()->tmpBitmap256))
    REPEAT_MACRO(NVM_RF4CE_ZRC2_ATTRIBUTE_ACTION_CODES_SUPPORTED_TX_DECLARE, , RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES)
    NVM_RF4CE_ZRC2_ATTRIBUTE_ACTION_CODES_SUPPORTED_TX_DECLARE( , RF4CE_ZRC2_ATTR_LOCAL_INDEX) /* LOCAL */

#           define NVM_RF4CE_ZRC2_ATTRIBUTE_HOME_AUTOMATION_SUPPORTED_DECLARE(useless, pairIndex) \
                    ARRAY_DECLARATION( \
                            NVM_RF4CE_ZRC2_ATTRIBUTE_HOME_AUTOMATION_SUPPORTED_PAIR##pairIndex, \
                            &GET_RF4CE_ZRC_STATIC_DATA_FIELD()->tmpBitmap256, \
                            RF4CE_ZRC2_HA_ENTITIES_MAX * sizeof(GET_RF4CE_ZRC_STATIC_DATA_FIELD()->tmpBitmap256))
    REPEAT_MACRO(NVM_RF4CE_ZRC2_ATTRIBUTE_HOME_AUTOMATION_SUPPORTED_DECLARE, , RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES)
    NVM_RF4CE_ZRC2_ATTRIBUTE_HOME_AUTOMATION_SUPPORTED_DECLARE( , RF4CE_ZRC2_ATTR_LOCAL_INDEX) /* LOCAL */

#           define NVM_RF4CE_ZRC2_ATTRIBUTE_ACTION_MAPPINGS_DECLARE(useless, pairIndex) \
                    ARRAY_DECLARATION( \
                            NVM_RF4CE_ZRC2_ATTRIBUTE_ACTION_MAPPINGS_PAIR##pairIndex, \
                            &GET_RF4CE_ZRC_STATIC_DATA_FIELD()->tmpActionMapping, \
                            RF4CE_ZRC2_ACTION_MAPPINGS_MAX * sizeof(GET_RF4CE_ZRC_STATIC_DATA_FIELD()->tmpActionMapping))
    REPEAT_MACRO(NVM_RF4CE_ZRC2_ATTRIBUTE_ACTION_MAPPINGS_DECLARE, , RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES)
    NVM_RF4CE_ZRC2_ATTRIBUTE_ACTION_MAPPINGS_DECLARE( , RF4CE_ZRC2_ATTR_LOCAL_INDEX) /* LOCAL */

    /* This special ZRC2 NVM parameter is used to init with zeros All NVM memory which belongs to ZRC2 attributes
     * on FactoryReset by writing only this parameter. Refer to MAN (l)seek and (f)write
     */
    PARAMETER_DECLARATION(NVM_RF4CE_ZRC2_ATTRIBUTE_LAST, &GET_RF4CE_ZRC_STATIC_DATA_FIELD()->tmpActionMapping.data[0], sizeof(uint8_t))

#       endif /* defined(USE_RF4CE_PROFILE_ZRC2) */
#   endif /* (1 == USE_RF4CE_PROFILE_ZRC) */
#   if (1 == USE_RF4CE_PROFILE_MSO)
     PARAMETER_DECLARATION(NVM_RF4CE_MSO_PA, &GET_RF4CE_MSO_STATIC_DATA_FIELD()->attributes, sizeof(GET_RF4CE_MSO_STATIC_DATA_FIELD()->attributes))
#   endif /* (1 == USE_RF4CE_PROFILE_MSO) */
#  endif /* (1 == USE_RF4CE_PROFILE_GDP) || (1 == USE_RF4CE_PROFILE_ZRC) || (1 == USE_RF4CE_PROFILE_MSO) */
#endif /* _RF4CE_ */
#ifdef _ZBPRO_
    UNION_DECLARATION(NVM_ZBPRO_NWK_IB_FULL, &nwkDescriptor.nib, sizeof(nwkDescriptor.nib))
# if !defined(_UNIT_TEST_) && !defined(__arm__) && !defined(_HOST_)
#  define NVM_ZBPRO_NWK_IB(name, pName) PARAMETER_DECLARATION(NVM_ZBPRO_NWK_IB_##name, &nwkDescriptor.nib.##pName, sizeof(nwkDescriptor.nib.##pName))
# else
#  define DOT() .
#  define NVM_ZBPRO_NWK_IB(name, pName) PARAMETER_DECLARATION(NVM_ZBPRO_NWK_IB_##name, &nwkDescriptor.nib##DOT##pName, sizeof(nwkDescriptor.nib##DOT##pName))
# endif
    NVM_ZBPRO_NWK_IB(EXTENDED_PAN_ID,                      nwkExtendedPanId)
    NVM_ZBPRO_NWK_IB(MIRRORED_IEEE_ADDR,                   nwkMirroredIeeeAddr)
    NVM_ZBPRO_NWK_IB(PASSIVE_ACK_TIMEOUT_OCT,              nwkPassiveAckTimeoutOct)
    NVM_ZBPRO_NWK_IB(NETWORK_BROADCAST_DELIVERY_TIME_OCT,  nwkNetworkBroadcastDeliveryTimeOct)
    NVM_ZBPRO_NWK_IB(PAN_ID,                               nwkPanId)
    NVM_ZBPRO_NWK_IB(TX_TOTAL,                             nwkTxTotal)
    NVM_ZBPRO_NWK_IB(MANAGER_ADDR,                         nwkManagerAddr)
    NVM_ZBPRO_NWK_IB(TRANSACTION_PERSISTENCE_TIME,         nwkTransactionPersistenceTime)
    NVM_ZBPRO_NWK_IB(NETWORK_ADDRESS,                      nwkNetworkAddress)
    NVM_ZBPRO_NWK_IB(TIME_BTWN_SCANS,                      nwkTimeBtwnScans)
    NVM_ZBPRO_NWK_IB(SEQUENCE_NUMBER,                      nwkSequenceNumber)
    NVM_ZBPRO_NWK_IB(MAX_BROADCAST_RETRIES,                nwkMaxBroadcastRetries)
    NVM_ZBPRO_NWK_IB(MAX_CHILDREN,                         nwkMaxChildren)
    NVM_ZBPRO_NWK_IB(MAX_DEPTH,                            nwkMaxDepth)
    NVM_ZBPRO_NWK_IB(MAX_ROUTERS,                          nwkMaxRouters)
    NVM_ZBPRO_NWK_IB(REPORT_CONSTANT_COST,                 nwkReportConstantCost)
    NVM_ZBPRO_NWK_IB(ADDR_ALLOC,                           nwkAddrAlloc)
    NVM_ZBPRO_NWK_IB(MAX_SOURCE_ROUTE,                     nwkMaxSourceRoute)
    NVM_ZBPRO_NWK_IB(UPDATE_ID,                            nwkUpdateId)
    NVM_ZBPRO_NWK_IB(STACK_PROFILE,                        nwkStackProfile)
    NVM_ZBPRO_NWK_IB(CONCENTRATOR_RADIUS,                  nwkConcentratorRadius)
    NVM_ZBPRO_NWK_IB(CONCENTRATOR_DISCOVERY_TIME_SEC,      nwkConcentratorDiscoveryTimeSec)
    NVM_ZBPRO_NWK_IB(SECURITY_LEVEL,                       nwkSecurityLevel)
    NVM_ZBPRO_NWK_IB(ACTIVE_KEY_SEQ_NUMBER,                nwkActiveKeySeqNumber)
    NVM_ZBPRO_NWK_IB(LINK_STATUS_PERIOD_SEC,               nwkLinkStatusPeriodSec)
    NVM_ZBPRO_NWK_IB(ROUTER_AGE_LIMIT,                     nwkRouterAgeLimit)
    NVM_ZBPRO_NWK_IB(CAPABILITY_INFORMATION,               nwkCapabilityInformation)
    NVM_ZBPRO_NWK_IB(ROUTE_REQUEST_ID,                     nwkRouteRequestId)
    NVM_ZBPRO_NWK_IB(MAX_LINK_ROUTE_COST,                  nwkMaxLinkRouteCost)
    NVM_ZBPRO_NWK_IB(NETWORK_CHANNEL,                      nwkNetworkChannel)
    NVM_ZBPRO_NWK_IB(DUMMY_FOR_NETWORK_PAGE,               nwkNetworkPage)
    NVM_ZBPRO_NWK_IB(DEVICE_TYPE,                          nwkDeviceType)
    NVM_ZBPRO_NWK_IB(DUMMY_FOR_NETWORK_DEPTH,              nwkDepth)
    NVM_ZBPRO_NWK_IB(SCAN_ATTEMPTS,                        nwkScanAttempts)
    NVM_ZBPRO_NWK_IB(TIME_STAMP,                           nwkTimeStamp)
    NVM_ZBPRO_NWK_IB(SYM_LINK,                             nwkSymLink)
    NVM_ZBPRO_NWK_IB(USE_TREE_ROUTING,                     nwkUseTreeRouting)
    NVM_ZBPRO_NWK_IB(USE_MULTICAST,                        nwkUseMulticast)
    NVM_ZBPRO_NWK_IB(IS_CONCENTRATOR,                      nwkIsConcentrator)
    NVM_ZBPRO_NWK_IB(ALL_FRESH,                            nwkAllFresh)
    NVM_ZBPRO_NWK_IB(SECURE_ALL_FRAMES,                    nwkSecureAllFrames)
    NVM_ZBPRO_NWK_IB(UNIQUE_ADDR,                          nwkUniqueAddr)
    NVM_ZBPRO_NWK_IB(LEAVE_REQUEST_ALLOWED,                nwkLeaveRequestAllowed)
    NVM_ZBPRO_NWK_IB(AUTHORIZED_FLAG,                      nwkAuthorizedFlag)
    NVM_ZBPRO_NWK_IB(CONCENTRATOR_NO_ROUTE_CACHE,          nwkConcentratorNoRouteCache)
    NVM_ZBPRO_NWK_IB(RESERVED_ALIGNMENT,                   nwkReservedAlignment0)
# undef NVM_ZBPRO_NWK_IB

    PARAMETER_DECLARATION(NVM_ZBPRO_NWK_IB_NEIGHBOR_TABLE,          &nwkDescriptor.neighborTable.table, sizeof(nwkDescriptor.neighborTable.table))
    PARAMETER_DECLARATION(NVM_ZBPRO_NWK_IB_SECURITY_MATERIAL_SET,   &nwkDescriptor.securityMaterial.storage, sizeof(nwkDescriptor.securityMaterial.storage))

    // Add tables here as below when needed.
    // PARAMETER_DECLARATION(ZBPRO_NWK_IB_GROUP_ID_TABLE, ...

    UNION_DECLARATION(NVM_ZBPRO_APS_IB_FULL, &ZbProApsMmDescr.aib, sizeof(ZbProApsMmDescr.aib))
# if !defined(_UNIT_TEST_) && !defined(__arm__) && !defined(_HOST_)
#  define NVM_ZBPRO_APS_IB(name, pName) PARAMETER_DECLARATION(NVM_ZBPRO_APS_IB_##name, &ZbProApsMmDescr.aib.##pName, sizeof(ZbProApsMmDescr.aib.##pName))
# else
#  define NVM_ZBPRO_APS_IB(name, pName) PARAMETER_DECLARATION(NVM_ZBPRO_APS_IB_##name, &ZbProApsMmDescr.aib##DOT##pName, sizeof(ZbProApsMmDescr.aib##DOT##pName))
# endif
    NVM_ZBPRO_APS_IB(TRUST_CENTER_ADDRESS,      apsTrustCenterAddress)
    NVM_ZBPRO_APS_IB(USE_EXTENDED_PANID,        apsUseExtendedPANID)
    NVM_ZBPRO_APS_IB(CHANNEL_MASK,              apsChannelMask)
    NVM_ZBPRO_APS_IB(ZDP_RESPONSE_TIMEOUT,      zdoZdpResponseTimeout)
    NVM_ZBPRO_APS_IB(END_DEV_BIND_TIMEOUT,      zdoEdBindTimeout)
    NVM_ZBPRO_APS_IB(ZHA_RESPONSE_TIMEOUT,      zclResponseTimeout)
    NVM_ZBPRO_APS_IB(DEBUG_ATTR,                apsFragTestSkipBlockMask)
    NVM_ZBPRO_APS_IB(MANUFACTURER_CODE,         zdoManufacturerCode)
    NVM_ZBPRO_APS_IB(SECURITY_TIME_OUT_PERIOD,  apsSecurityTimeOutPeriod)
    NVM_ZBPRO_APS_IB(PERMIT_JOIN_DURATION,      zdoPermitJoinDuration)
    NVM_ZBPRO_APS_IB(SCAN_DURATION,             zdoScanDuration)
    NVM_ZBPRO_APS_IB(NONMEMBER_RADIUS,          apsNonmemberRadius)
    NVM_ZBPRO_APS_IB(INTERFRAME_DELAY,          apsInterframeDelay)
    NVM_ZBPRO_APS_IB(LAST_CHANNEL_ENERGY,       apsLastChannelEnergy)
    NVM_ZBPRO_APS_IB(LAST_CHANNEL_FAILURE_RATE, apsLastChannelFailureRate)
    NVM_ZBPRO_APS_IB(CHANNEL_TIMER,             apsChannelTimer)
    NVM_ZBPRO_APS_IB(DESIGNATED_COORDINATOR,    apsDesignatedCoordinator)
    NVM_ZBPRO_APS_IB(USE_INSECURE_JOIN,         apsUseInsecureJoin)
    NVM_ZBPRO_APS_IB(INITIAL_SECURITY_STATUS,   zdoInitialSecurityStatus)
    NVM_ZBPRO_APS_IB(FRAG_THRESHOLD,            apsFragThreshold)

    NVM_ZBPRO_APS_IB(RESERVED_ALIGNMENT,        apsReservedAlignment0)
# undef ZBPRO_APS_IB

    PARAMETER_DECLARATION(NVM_ZBPRO_APS_IB_DEVICE_KEY_PAIR_SET,  &ZbProApsMmDescr.keyPairDescr.pairTable, sizeof(ZbProApsMmDescr.keyPairDescr.pairTable))

    PARAMETER_DECLARATION(NVM_ZBPRO_APS_IB_BINDING_TABLE_DESCR,   &ZbProApsMmDescr.bindingTableDescr,   sizeof(ZbProApsMmDescr.bindingTableDescr))
    PARAMETER_DECLARATION(NVM_ZBPRO_APS_IB_BINDING_TABLE,         &ZbProApsMmDescr.bindingTable,        sizeof(ZbProApsMmDescr.bindingTable))
    PARAMETER_DECLARATION(NVM_ZBPRO_APS_IB_GROUP_TABLE,           &ZbProApsMmDescr.groupTable,          sizeof(ZbProApsMmDescr.groupTable))


    PARAMETER_DECLARATION(NVM_ZBPRO_APS_ENDPOINT_TABLE_BASE, &ZbProApsMmDescr.endpointDescr.table, sizeof(ZbProApsMmDescr.endpointDescr.table))

#define NVM_ZBPRO_APS_ENDPOINT_TABLE_MAPPING_DECLARE(useless, index) \
        PARAMETER_DECLARATION(                                       \
                NVM_ZBPRO_APS_ENDPOINT_TABLE_CLUSTER_LIST##index,    \
                &GetBufferedTransactionBuffer(),                     \
                NVM_BUFFERED_TRANSACT_BUFFER_SIZE)

    REPEAT_MACRO(NVM_ZBPRO_APS_ENDPOINT_TABLE_MAPPING_DECLARE, , ZBPRO_APS_NODE_ENDPOINTS)

    // Add tables here as below when needed.
    // PARAMETER_DECLARATION(ZBPRO_APS_IB_PERMISSIONS_CONFIGURATION_ID, ...
#endif /* _ZBPRO_ */

/* eof bbNvmParameters.h */

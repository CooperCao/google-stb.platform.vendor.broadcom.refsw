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
* FILENAME: $Workfile: branches/akhromykh/BEE554_NVM_Implementation/common/Security/include/bbNvmScenarios.h $
*
* DESCRIPTION:
*   Scenarios definition for NVM.
*
* $Revision: 1832 $
* $Date: 2014-03-19 07:10:11Z $
*
*****************************************************************************************/

#ifdef _RF4CE_
# ifdef USE_RF4CE_NWK
    SCENARIO_DECLARATION(NVM_RF4CE_NWK, 4, NVM_RF4CE_NWK_FRAME_COUNTER_ID _ NVM_RF4CE_NWK_NIB_ID _ NVM_RF4CE_NWK_PANID_ID _ NVM_RF4CE_NWK_SHORT_ADDRESS_ID)
    SCENARIO_DECLARATION(NVM_RF4CE_NWK_PANID_SHORT_ADDRESS, 2, NVM_RF4CE_NWK_PANID_ID _ NVM_RF4CE_NWK_SHORT_ADDRESS_ID)
#  if (1 == USE_RF4CE_PROFILE_GDP) || (1 == USE_RF4CE_PROFILE_ZRC) || (1 == USE_RF4CE_PROFILE_MSO)
    SCENARIO_DECLARATION(NVM_RF4CE_PM_SCEN, 1, NVM_RF4CE_PM_ID)
#   if (1 == USE_RF4CE_PROFILE_MSO)
     SCENARIO_DECLARATION(NVM_RF4CE_MSO_PA_SCEN, 1, NVM_RF4CE_MSO_PA_ID)
#   endif /* (1 == USE_RF4CE_PROFILE_MSO) */
#  else /* (1 == USE_RF4CE_PROFILE_GDP) || (1 == USE_RF4CE_PROFILE_ZRC) || (1 == USE_RF4CE_PROFILE_MSO) */
    SCENARIO_DECLARATION(NVM_RF4CE_PM_SCEN, 2, NVM_RF4CE_NWK_FRAME_COUNTER_ID _ NVM_RF4CE_NWK_NIB_ID)
#  endif /* (1 == USE_RF4CE_PROFILE_GDP) || (1 == USE_RF4CE_PROFILE_ZRC) || (1 == USE_RF4CE_PROFILE_MSO) */
# else /* USE_RF4CE_NWK */
    SCENARIO_DECLARATION(NVM_NO_SCENARIO, 1, NVM_NO_PARAM_ID)
# endif /* USE_RF4CE_NWK */
#endif /* _RF4CE_ */

#ifdef _ZBPRO_
    SCENARIO_DECLARATION(NVM_ZBPRO_NWK_TABLES, 3,             \
        NVM_ZBPRO_NWK_IB_FULL_ID                            _ \
        NVM_ZBPRO_NWK_IB_NEIGHBOR_TABLE_ID                  _ \
        NVM_ZBPRO_NWK_IB_SECURITY_MATERIAL_SET_ID)

    SCENARIO_DECLARATION(NVM_ZBPRO_NWK_IB_RAW_TABLE, 1, NVM_ZBPRO_NWK_IB_FULL_ID)
    // NOTE: alternative way to store NIB, disabled to save memory
    /*
    SCENARIO_DECLARATION(NVM_ZBPRO_NWK_IB_ALL_ATTRIBUTES, 45, \
        NVM_ZBPRO_NWK_IB_EXTENDED_PAN_ID_ID                 _ \
        NVM_ZBPRO_NWK_IB_MIRRORED_IEEE_ADDR_ID              _ \
        NVM_ZBPRO_NWK_IB_PASSIVE_ACK_TIMEOUT_OCT_ID         _ \
        NVM_ZBPRO_NWK_IB_NETWORK_BROADCAST_DELIVERY_TIME_OCT_ID _ \
        NVM_ZBPRO_NWK_IB_PAN_ID_ID                          _ \
        NVM_ZBPRO_NWK_IB_TX_TOTAL_ID                        _ \
        NVM_ZBPRO_NWK_IB_MANAGER_ADDR_ID                    _ \
        NVM_ZBPRO_NWK_IB_TRANSACTION_PERSISTENCE_TIME_ID    _ \
        NVM_ZBPRO_NWK_IB_NETWORK_ADDRESS_ID                 _ \
        NVM_ZBPRO_NWK_IB_TIME_BTWN_SCANS_ID                 _ \
        NVM_ZBPRO_NWK_IB_SEQUENCE_NUMBER_ID                 _ \
        NVM_ZBPRO_NWK_IB_MAX_BROADCAST_RETRIES_ID           _ \
        NVM_ZBPRO_NWK_IB_MAX_CHILDREN_ID                    _ \
        NVM_ZBPRO_NWK_IB_MAX_DEPTH_ID                       _ \
        NVM_ZBPRO_NWK_IB_MAX_ROUTERS_ID                     _ \
        NVM_ZBPRO_NWK_IB_REPORT_CONSTANT_COST_ID            _ \
        NVM_ZBPRO_NWK_IB_ADDR_ALLOC_ID                      _ \
        NVM_ZBPRO_NWK_IB_MAX_SOURCE_ROUTE_ID                _ \
        NVM_ZBPRO_NWK_IB_UPDATE_ID_ID                       _ \
        NVM_ZBPRO_NWK_IB_STACK_PROFILE_ID                   _ \
        NVM_ZBPRO_NWK_IB_CONCENTRATOR_RADIUS_ID             _ \
        NVM_ZBPRO_NWK_IB_CONCENTRATOR_DISCOVERY_TIME_SEC_ID _ \
        NVM_ZBPRO_NWK_IB_SECURITY_LEVEL_ID                  _ \
        NVM_ZBPRO_NWK_IB_ACTIVE_KEY_SEQ_NUMBER_ID           _ \
        NVM_ZBPRO_NWK_IB_LINK_STATUS_PERIOD_SEC_ID          _ \
        NVM_ZBPRO_NWK_IB_ROUTER_AGE_LIMIT_ID                _ \
        NVM_ZBPRO_NWK_IB_CAPABILITY_INFORMATION_ID          _ \
        NVM_ZBPRO_NWK_IB_ROUTE_REQUEST_ID_ID                _ \
        NVM_ZBPRO_NWK_IB_MAX_LINK_ROUTE_COST_ID             _ \
        NVM_ZBPRO_NWK_IB_NETWORK_CHANNEL_ID                 _ \
        NVM_ZBPRO_NWK_IB_DUMMY_FOR_NETWORK_PAGE_ID          _ \
        NVM_ZBPRO_NWK_IB_DEVICE_TYPE_ID                     _ \
        NVM_ZBPRO_NWK_IB_DUMMY_FOR_NETWORK_DEPTH_ID         _ \
        NVM_ZBPRO_NWK_IB_SCAN_ATTEMPTS_ID                   _ \
        NVM_ZBPRO_NWK_IB_TIME_STAMP_ID                      _ \
        NVM_ZBPRO_NWK_IB_SYM_LINK_ID                        _ \
        NVM_ZBPRO_NWK_IB_USE_TREE_ROUTING_ID                _ \
        NVM_ZBPRO_NWK_IB_USE_MULTICAST_ID                   _ \
        NVM_ZBPRO_NWK_IB_IS_CONCENTRATOR_ID                 _ \
        NVM_ZBPRO_NWK_IB_ALL_FRESH_ID                       _ \
        NVM_ZBPRO_NWK_IB_SECURE_ALL_FRAMES_ID               _ \
        NVM_ZBPRO_NWK_IB_UNIQUE_ADDR_ID                     _ \
        NVM_ZBPRO_NWK_IB_LEAVE_REQUEST_ALLOWED_ID           _ \
        NVM_ZBPRO_NWK_IB_AUTHORIZED_FLAG_ID                 _ \
        NVM_ZBPRO_NWK_IB_CONCENTRATOR_NO_ROUTE_CACHE_ID)
    */

    SCENARIO_DECLARATION(NVM_ZBPRO_APS_TABLES, 5,             \
        NVM_ZBPRO_APS_IB_FULL_ID                            _ \
        NVM_ZBPRO_APS_IB_DEVICE_KEY_PAIR_SET_ID             _ \
        NVM_ZBPRO_APS_IB_GROUP_TABLE_ID                     _ \
        NVM_ZBPRO_APS_IB_BINDING_TABLE_DESCR_ID             _ \
        NVM_ZBPRO_APS_IB_BINDING_TABLE_ID)

    SCENARIO_DECLARATION(NVM_ZBPRO_APS_IB_RAW_TABLE, 1, NVM_ZBPRO_APS_IB_FULL_ID)
    // NOTE: alternative way to store AIB, disabled to save memory
    /*
    SCENARIO_DECLARATION(NVM_ZBPRO_APS_IB_ALL_ATTRIBUTES, 17, \
        NVM_ZBPRO_APS_IB_TRUST_CENTER_ADDRESS_ID            _ \
        NVM_ZBPRO_APS_IB_USE_EXTENDED_PANID_ID              _ \
        NVM_ZBPRO_APS_IB_CHANNEL_MASK_ID                    _ \
        NVM_ZBPRO_APS_IB_ZDP_RESPONSE_TIMEOUT_ID            _ \
        NVM_ZBPRO_APS_IB_END_DEV_BIND_TIMEOUT_ID            _ \
        NVM_ZBPRO_APS_IB_ZHA_RESPONSE_TIMEOUT_ID            _ \
        NVM_ZBPRO_APS_IB_MANUFACTURER_CODE_ID               _ \
        NVM_ZBPRO_APS_IB_PERMIT_JOIN_DURATION_ID            _ \
        NVM_ZBPRO_APS_IB_SCAN_DURATION_ID                   _ \
        NVM_ZBPRO_APS_IB_SECURITY_TIME_OUT_PERIOD_ID        _ \
        NVM_ZBPRO_APS_IB_NONMEMBER_RADIUS_ID                _ \
        NVM_ZBPRO_APS_IB_INTERFRAME_DELAY_ID                _ \
        NVM_ZBPRO_APS_IB_LAST_CHANNEL_ENERGY_ID             _ \
        NVM_ZBPRO_APS_IB_LAST_CHANNEL_FAILURE_RATE_ID       _ \
        NVM_ZBPRO_APS_IB_CHANNEL_TIMER_ID                   _ \
        NVM_ZBPRO_APS_IB_DESIGNATED_COORDINATOR_ID          _ \
        NVM_ZBPRO_APS_IB_USE_INSECURE_JOIN_ID               _ \
        NVM_ZBPRO_APS_IB_INITIAL_SECURITY_STATUS_ID)
    */
    SCENARIO_DECLARATION(NVM_ZBPRO_APS_IB_RAW_KEY_PAIR_SET, 1, NVM_ZBPRO_APS_IB_DEVICE_KEY_PAIR_SET_ID)

    SCENARIO_DECLARATION(NVM_ZBPRO_ALL_TABLES, 8,             \
        NVM_ZBPRO_NWK_IB_FULL_ID                            _ \
        NVM_ZBPRO_NWK_IB_NEIGHBOR_TABLE_ID                  _ \
        NVM_ZBPRO_NWK_IB_SECURITY_MATERIAL_SET_ID           _ \
        \
        NVM_ZBPRO_APS_IB_FULL_ID                            _ \
        NVM_ZBPRO_APS_IB_DEVICE_KEY_PAIR_SET_ID             _ \
        NVM_ZBPRO_APS_IB_GROUP_TABLE_ID                     _ \
        NVM_ZBPRO_APS_IB_BINDING_TABLE_DESCR_ID             _ \
        NVM_ZBPRO_APS_IB_BINDING_TABLE_ID)

    SCENARIO_DECLARATION(NVM_ZBPRO_ACTIVE_PARAMETERS, 4,      \
        NVM_ZBPRO_APS_IB_TRUST_CENTER_ADDRESS_ID            _ \
        NVM_ZBPRO_NWK_IB_FULL_ID                            _ \
        NVM_ZBPRO_NWK_IB_NEIGHBOR_TABLE_ID                  _ \
        NVM_ZBPRO_NWK_IB_SECURITY_MATERIAL_SET_ID)

#endif /* _ZBPRO_ */

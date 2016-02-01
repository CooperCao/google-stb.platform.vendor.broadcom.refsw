//****************************************************************************
//
// Copyright (c) 1999-2009 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized License, Broadcom grants no license
// (express or implied), right to use, or waiver of any kind with respect to
// the Software, and Broadcom expressly reserves all rights in and to the
// Software and all intellectual property rights therein.  IF YOU HAVE NO
// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
// SOFTWARE.  
//
// Except as expressly set forth in the Authorized License,
//
// 1.     This program, including its structure, sequence and organization,
// constitutes the valuable trade secrets of Broadcom, and you shall use all
// reasonable efforts to protect the confidentiality thereof, and to use this
// information only in connection with your use of Broadcom integrated circuit
// products.
//
// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//****************************************************************************
//
//  Filename:       TLVCodes.h
//  Author:         David Pullen
//  Creation Date:  April 28, 1999
//
//****************************************************************************


//****************************************************************************
//  Description:
//      This file just contains the DOCSIS-specified type values that are
//      defines for the T portion of TLV codes.
//
//****************************************************************************

#ifndef TLVCODES_H
#define TLVCODES_H

//********************** Include Files ***************************************

//********************** Global Types ****************************************

//********************** Global Constants ************************************

// These are the Type codes for the overall upstream channel parameters TLVs
// (table 6-18).
typedef enum
{
    kSymbolRate160KSymPerSec            = 1,
    kFrequency                          = 2,
    kPreambleSuperstringPattern         = 3,
    kBurstDescriptor                    = 4,
    kAdvPhyBurstDescriptor              = 5,
    kExtendedPreambleSuperstringPattern = 6,
    kScdmaModeEnable                    = 7,
    kScdmaSpreadingIntervalsPerFrame    = 8,
    kScdmaCodesPerMinislot              = 9,
    kScdmaNumberOfActiveCodes           = 10,
    kScdmaCodeHoppingSeed               = 11,
    kScdmaUsRatioNumeratorM             = 12,
    kScdmaUsRatioDenominatorN           = 13,
    kScdmaTimestampSnapshot             = 14,
    kOtfUcdMaintainPower                = 15,
    kOtfUcdRangingRequired              = 16,
        kOtfRngNone = 0,
        //
        // "unicast initial ranging" is a contradiction in terms.
        // initial ranging implies the following requirements:
        // 1) map info element with
        // 1.1) iuc == 3 (initial ranging)
        // 1.2) sid == 0x3ff (broadcast sid)
        // 1.3) large size in order to correct large timing offset.
        //
        // 2) CM must support backoff strategy with respect to
        // use of broadcast init ranging op's
        //
        // 3) T2 no init maint op timer in effect
        //
        // 4) T3 no RNG-RSP timer handling requires CM to change
        //    upstream tx power
        //
        // 5) for SCDMA upstream, spreader is off (which is always
        // true for initial ranging bursts, iuc 3). 
        //      
        // "unicast initial ranging" only needs requirements 5)
        // (and by association, requirement 1.1)).  "unicast initial ranging" 
        // has very little to do with "initial ranging."  it's really 
        // (SCDMA spreader off) station maintenance that uses iuc 3 
        // instead of iuc 4.
        // 
        kOtfRngUnicastInitial = 1,
        kOtfRngStationMaintIuc3or4 = kOtfRngUnicastInitial,
        kOtfRngBroadcastInitial = 2,
    kScdmaMaxScheduledCodesEnable       = 17,
    kInitialRangingDisable = 18,
    kChanSelectionEnable = 19


} UpstreamChannelParameterTypeCodes;

// These are the Type codes for the burst descriptor/profile TLVs (table 6-19).
typedef enum
{
    kModulationType                 = 1,
    kDifferentialEncoding           = 2,
    kPreambleSubstringLength        = 3,
    kPreambleSubstringValueOffset   = 4,
    kFecTBytes                      = 5,
    kFecKBytes                      = 6,
    kScramblerSeed                  = 7,
    kMaximumBurstSize               = 8,
    kGuardTimeSize                  = 9,
    kLastCodewordLength             = 10,
    kScramblerEnable                = 11,
    kRsInterleaverDepth             = 12,
    kRsInterleaverBlockSize         = 13,
    kPreambleType                   = 14,
    kScdmaSpreaderEnable            = 15,
    kScdmaCodesPerSubframe          = 16,
    kScdmaInterleaverStepSize       = 17,
    kScdmaTcmEncodingEnable         = 18
} BurstDescriptorTypeCodes;

// These are the Type codes for Ranging Response messages (table 6-21).
typedef enum
{
    kTimingAdjust                   = 1,
    kPowerLevelAdjust               = 2,
    kOffsetFrequencyAdjust          = 3,
    kTransmitEqualizationAdjust     = 4,
    kRangingStatus                  = 5,
        kRngContinue = 1,
        kRngAbort = 2,
        kRngSuccess = 3,
    kDownstreamFrequencyOverride    = 6,
    kUpstreamChannelIdOverride      = 7,

    // PR1590 - these were added for DOCSIS 2.0
    kFineTimingAdjustExtension      = 8,
    kTransmitEqualizationSet        = 9,

    // added for SCDMA power per code ranging hack...
    kScdmaMaxScheduledCodes         = 10,
    kScdmaPowerHeadroomQtrDb        = 11

} RangingResponseTypeCodes;

// This is the Type code for an Upstream Channel Change Request.
typedef enum
{
    kUccRngStrategy                 = 1,
        // perform initial ranging on the target us chan.  use existing
        // primary sid.  this is the default.  
        kRngStratInitial = 0,

        // use unicast station maintenance opportunities.
        kRngStratStation = 1,

        // use either initial ranging of unicast station maint, 
        // whichever comes first.
        // NOTE: our MAC chips do not support the "whichever comes
        // first" mode.  we should treat this value the same as kInitialRng.
        kRngStratInitialOrStation = 2,

        // "use new channel directly."  from the ranging point of view,
        // this effectively is the same as kStationMaint.  why does it 
        // exist at all?  reading between the lines, they may be implying 
        // that for other values data traffic should be stopped until a 
        // RNG-RSP w/status of success is received.
        kRngStratNoChange = 3
} UCCRequestTypeCodes;


// Type codes and value mappings specific to Dynamic Channel Change REQuest 
// (DCC-REQ) messages.
//
typedef enum
{
    kDccDestUsChanId = 1,

    kDccDestDsSettings = 2,
        kDccDsFrequencyHz = 1,
        kDccDsModulationType = 2,
            kDccDsMod64Qam = 0,
            kDccDsMod256Qam = 1,

        kDccDsSymbolRateEnum = 3,
            kDccDs5056941SymPerSec = 0,
            kDccDs5360537SymPerSec = 1,
            kDccDs6952000SymPerSec = 2,

        kDccDsInterleaverDepth = 4,
        kDccDestDsChanId = 5,

        kDccDsTimeSyncStrategy = 6,
            kDccDsStratWaitForSync = 0,
            kDccDsStratUseDirectly = 1,


    kDccUsInitStrategy = 3,
        kDccUsStratResetUs = 0,
        kDccUsStratRngInitial = 1,
        kDccUsStratRngStation = 2,
        kDccUsStratRngInitialOrStation = 3,
        kDccUsStratUseDirectly = 4,


    kSubUcd = 4,

    kSubSaid = 6,

    kSubQosFlowSettings = 7,
        kSubSfid = 1,
        kSubSid = 2,
        kSubClassifierId = 3,
        kSubPhsi = 4,
        kSubUgsTimeRef = 5,

    // This was made official by RFI-N-01060.
    kDccDestCmtsMacAddress = 8,

} DccReqTypeCodes;

// Type codes and value mappings specific to Dynamic Channel Change ReSPonse
// (DCC-RSP) messages.
//
typedef enum
{
    kCmJumpTime = 1,
        kJumpDurationTicks = 1,
        kJumpStartTimePair = 2

} DccRspTypeCodes;

// Type codes specific to DOCSIS 2.0 Phy Test Request (TST-REQ) message.
//
typedef enum
{
    // use types from UCD when possible.
    //kSymbolRate160KSymPerSec            = 1,
    //kFrequency                          = 2,
    kTxPowerDb                          = 3,
    kTstReqScdmaUsRatioNumeratorM       = 4,
    kTstReqScdmaUsRatioDenominatorN     = 5
} TstReqTypeCodes;


// Type codes and value mappings specific to MAC Domain Descriptor (MDD) messages.
//
typedef enum
{

    kDsActiveChanSettings = 1,
        kDsChanId = 1,
        kCenterFreqHz = 2,

        kModulationTypeAndAnnex = 3,
            // masks...
            kModulationBitMask = 0x0f,
            kAnnexBitMask = 0xf0,
            // modulation values...
            kMod64Qam = 0,
            kMod256Qam = 1,
            // annex values...
            kDsAnnexA = 0,
            kDsAnnexB = 0x10,
            kDsAnnexC = 0x20,

        kPrimaryCapable = 4,
            kDsNotPrimaryCapable = 0,
            kDsPrimaryCapable = 1,

        kStatusEventReportingBitmask = 5,


    kMdDsSvcGroupSettings = 2,
        kMdDsSgId = 1,
        kDsChanIdVector = 2,

    kDsAmbResTargetFreqVector = 3,

    kCmRcpControlSettings = 4,
        kRcpCenterFreqSpacing = 1,
            kCenterFreqSpacing6Mhz = 0,
            kCenterFreqSpacing8Mhz = 1,

        kRcpVerboseRptEnable = 2,

    kCmIpInitSettings = 5,
        kIpProvisioningMode = 1,
            kIpv4Only = 0,
            kIpv6Only = 1,
            kAlternateProvisioningMode = 2,
            kDualStack = 3,

        kPreRegDsid = 2,
    
    kCmEaeSetting = 6,

    kUsActiveChanSettings = 7,
        k3dfChanId = 1,

        k3dfStatusEventReportingBitmask = 2,

    kUsAmbResTargetUcidVector = 8,

    kPlantUsFreqRange = 9,
        kStdUsFreqRange = 0,
        kExtUsFreqRange = 1,

    kCmtsDsSymbolClkLockedToMasterClk = 10,

    kCmStatusEventControlSettings = 11,
        kStEventType = 1,
            // see CmStatusCodes enum for event type value mappings.
        kStEvMaxHoldoffTimer20Millisec = 2,
        kStEvMaxReportsPerTrans = 3,

    kUsTxPowerRptEnable = 12,

    kDsgDaToDsidAssoc = 13,

    kCm3dfNonChSpEvRptEnableSettings = 15

} MddTypeCodes;

// returns enum name as char*.  useful for debug output.
const char* DsModulationTypeName( unsigned char modulation_type_and_annex );
const char* DsAnnexTypeName( unsigned char modulation_type_and_annex );


// Type codes for DOCSIS 3.0 CM-STATUS message.
//
typedef enum
{
    kCmStatusEvent = 1,
        kCmEvType = 1,
            kCmEvNonPriDsMddFail = 1,
            kCmEvDsPhyLockFail = 2,
            kCmEvDsDataReseqFail = 3,
            kCmEvNonPriDsMddRescue = 4,
            kCmEvDsPhyLockRescue = 5,
            kCmEvT4NoStationMaintOpTimeout = 6,
            kCmEvMaxT3NoRngRspTimeouts = 7,
            kCmEvMaxT3NoRngRspRescue = 8,
            kCmEvAcPowerFail = 9,
            kCmEvAcPowerRescue = 10,

        kCmEvDescription = 2,
        kCmEvTransId = 3,
        kCmEvDsChanId = 4,
        kCmEvUsChanId = 5,
        kCmEvDsid = 6,

    // status event reporting enable bitmask values

} CmStatusCodes;

// Is3dfDsMacPhyEvType() - Returns true if specified event type is 
//      one of the downstream MAC/PHY failure or recovery events.
//      
bool Is3dfDsMacPhyEvType( unsigned int eventType );

// Is3dfUsMacPhyEvType() - Returns true if specified event type is 
//      one of the upstream MAC/PHY failure or recovery events.
//      
bool Is3dfUsMacPhyEvType( unsigned int eventType );

// Is3dfDsOrUsSpecificEvType() - Returns true if specified event type is 
//      one of the downstream or upstream MAC/PHY failure or recovery events.
//      
bool Is3dfDsOrUsSpecificEvType( unsigned int eventType );

// Is3dfNonChanSpecificEvType() - Returns true if specified event type is 
//      not downstream or upstream channel specific.
//      
bool Is3dfNonChanSpecificEvType( unsigned int eventType );

// EvTypeHasInstanceIdParam() - Returns true if specified event type 
//      requires event instance id parameter.
//      
bool EvTypeHasInstanceIdParam( unsigned int eventType );



// values for CM-STATUS reporting enable bitmask.  various combinations
// of the bits are used in several different 16-bit settings.  bit
// definitions appear consistent for all settings (as of 2007/04/12)...
typedef enum 
{
    kRptNonPriDsMddFail             = 0x0002, 
    kRptNonPriDsPhyLockFail         = 0x0004, 
                                    
    kRptDsDataReseqFail             = 0x0008,
                                    
    kRptNonPriDsMddRescue           = 0x0010,
    kRptNonPriDsPhyLockRescue       = 0x0020,

    kRptT4NoStationMaintOpTimeout   = 0x0040,
    kRptMaxT3NoRngRspTimeouts       = 0x0080,
    kRptMaxT3NoRngRspRescue         = 0x0100,

    kRptAcPowerFail                 = 0x0200,
    kRptAcPowerRescue               = 0x0400

} CmStRptEnableBitmaskValues;

// Is3dfReportingEnabled() - Returns true if reporting is enabled for the
//      specified event type in the specified reporting enable bitmask setting.
//
// Parameters:
//      statusEventReportingBitmask - target reporting enable bitmask setting.
//
//      eventType - 3df event type code, as used in CM-STATUS, MDD and CM-CTRL-xxx
//          MAC management messages.
//
bool Is3dfReportingEnabled( unsigned int statusEventReportingBitmask, unsigned int eventType );

// Type codes for DOCSIS 3.0 CM-CTRL-Rxx message.
//
typedef enum
{
    kPhyTxMuteTargetUsChanId = 1,
    kPhyTxMuteTimeoutPeriodMillisec = 2,
    kRestartCmDocsisMac = 3,
    kDisableDataFwdToCmci = 4,

    kOvRdCm3dfDsChSpEvRptEnableSettings = 5,
    kOvRdCm3dfUsChSpEvRptEnableSettings = 6,
        // the following sub-types apply to parent types 5 and 6.
        kOvRd3dfChanId = 1,
        kOvRd3dfStatusEventReportingBitmask = 2,

    kCmStatusEventEnableBitmask = 7

} CmCtrlRxxCodes;


// Type codes and value mappings specific to CM Receive Channel 
// Profile/Configuration (RCx) encodings used in DOCSIS 3.0 REG-RSP/ACK 
// and DBC messages.  
// kRcp??? names are specific to RCP encodings.
// kRcc??? names are specific to RCC encodings.
// kRcx??? names are used for both RCP and RCC encodings.
//
typedef enum
{
    kRcxId = 1,

    kRcpName = 2,                       // not supported in phase1

    kRcpCenterFreqSpacingHz = 3,        // not supported in phase1
       
    kRcpDsRxModuleProfile = 4,          // not supported in phase1
    kRccDsRxModuleConfig = 4,           // not supported in phase1
        kRxModuleIndex = 1,
        kRxModuleAdjChan = 2,

    kRcpDsRxChanProfile = 5,
    kRccDsRxChanConfig = 5,
        kRxChanIndex = 1,
        kRxChanConnBwBitmap = 2,
        kRxChanCenterFreqHz = 4,

        kRxChanPrimaryCapable = 5,
            kRxChanDsNotPrimaryCapable = 0,
            kRxChanDsPrimaryCapable = 1,

    kRccPartialServiceDownstreamChannels = 6,

    kRccErrorEncodings = 254,
        kRccErrorModuleOrChannel = 1,   
            kReceiveModule = 4,    // this is one of 2 valid 'values' for the kRccErrorModuleOrChannel 'type'
            kReceiveChannel = 5,   // this is one of 2 valid 'values' for the kRccErrorModuleOrChannel 'type'
        kRccErrorModuleOrChannelIndex = 2,
        kRccErrorReportedParameter = 3,
        kRccErrorCode = 4,
        kRccErrorMessage = 5
} RcxTypeCodes;


// Type codes and value mappings specific to DOCSIS 3.0 MAC CM Transmit 
// (a.k.a. upstream) Channel Configuration settings used in registration
// and dynamic bonding change messaging.
//
typedef enum
{
    kTccRefId = 1,

    kTccAction = 2,
        // value mapping
        kTccNoAction = 0,
        kTccAdd = 1,
        kTccChange = 2,
        kTccDelete = 3,
        kTccReplace = 4,
        kTccRange = 5,

    kTccUsChanId = 3,

    kTccNewUsChanId = 4,

    kTccUpstreamSettings = 5,
       
    kTccRngSid = 6,

    kTccRngStrategy = 7,
        // value mapping
        kTccRngBcastInit = 1,
        kTccRngUnicastInitOrStationMaint = 2,
        kTccRngBcastOrUnicastStationMaint = 3,
        kTccRngUseDirectly = 4,
        // assume that values ==0 or >4 is re-init MAC.

    kTccRngAdjSettings = 8,
        // subtypes...
        kTccRngRefUsChanId = 1,
        kTccTimingAdjIntTicksDiv64 = 2,
        kTccTimingAdjFracTicksDiv64x256 = 3,
        kTccPowerAdjQtrDb = 4,
        kTccFreqAdjHz = 5,

    kTccErrorEncoding = 254,
        // subtypes...
        kTccErrMungedMultiLevelErrType = 1,
        kTccErrCode = 2,
        kTccErrDescription = 3,


} TccTypeCodes;



// Type codes and value mappings specific to DOCSIS 3.0 MAC CM Service Flow Sid
// Cluster settings used in registration and dynamic bonding change messaging.
//
typedef enum
{
    kSidClusterSfid = 1,

    kCmSidClusterSettings = 2,
        kSidClusterId = 1,

        kSidToUsChanCmd = 2,

            kSidToUsCmdUsChanId = 1,
            kSidToUsCmdSid = 2,

            kSidToUsCmdAction = 3,
                // value mapping
                kSidToUsCmdAdd = 1,
                kSidToUsCmdDelete = 2,
                
        kSidClSwitchoverSettings = 3,
            kSwOvMaxTxBwReq = 1,
            kSwOvMaxOutstandingReqBwBytes = 2,
            kSwOvMaxTotalReqBwBytes = 3,
            kSwOvMaxElapsedTimeMillisec = 4

} CmSvcFlowSidClusterCodes;



// Top level type codes common to Cable modem configuration file
// and REG-REQ message.  Refer to App. C. in DOCSIS RFI spec.
typedef enum
{
    kOrDsFreq = 1,
    kOrUsChanId = 2,
    kNetAccessEnable = 3,
    kCosReqSettings = 4,            // sub-types
    kCmMic = 6,
    kCmtsMic = 7,
    kTelcoReturnSettings = 15,      // sub-types
    kCmBpkmSettings = 17,           // sub-types
    kMaxCpePerCm = 18,
    kTftpTimestamp = 19,
    kTftpProvCmIpAddr = 20,
    kUsClassifierSettings = 22,     // sub-types
    kDsClassifierSettings = 23,     // sub-types
    kQosUsFlowSettings = 24,        // sub-types
    kQosDsFlowSettings = 25,        // sub-types
    kPhsSettings = 26,              // sub-types
    kMaxAdmittedClassifiers = 28,
    kGlobalBpkmEnable = 29,
    kAuthBlock = 30,
    kSmMgtMibControl = 35,          // mutant
    kSmMgtCpeIpTable = 36,          // mutant
    kSmMgtFilterGroups = 37,        // mutant
    kDocsis20ModeEnable = 39,
    kDocsis20TestModeEnable = 40,
    kDsTargetList = 41,
        kDsTargetSingleFreqSettings = 1,    // sub-types
            kDsTargetTimeoutSecs = 1,
            kDsTargetFreqStartHz = 2,
        kDsTargetFreqRangeSettings = 2,     // sub-types
            // use same sub-type 1, 2 as used for single freq...
            kDsTargetFreqEndHz = 3,
            kDsTargetFreqStepSizeHz = 4,
        kDsTargetDefaultSettings = 3,       // mutant
        
    kVendorSpecific = 43,            // sub-types
        kRangingClassExt = 4,       // mutant
        kL2Vpn = 5,                 // mutant

        // This group added for PR12948.
        kExtendedCmtsMic = 6,
            kExtendedCmtsMicHmacType = 1,
            kExtendedCmtsMicBitmap = 2,
            kExtendedCmtsMicDigest = 3,

    kUsDropClassifierSettings = 60, // sub-types, PR12948
    kSmMgtCpeIpV6Table = 61,        // mutant
    kUsDropClassifierGroupId = 62,  // PR12948

} CmConfigAndRegReqTypeCodes;

// Top level type codes specific to Cable modem configuration file.
// Refer to App. C. in DOCSIS RFI spec.
typedef enum
{
    kEndOfData = 255,
    kPad = 0,
    kSwUpgradeFilename = 9,
    kSnmpWriteAccessControl = 10,
    kSnmpMibObject = 11,
    kCpeMacAddr = 14,
    kSwUpgradeServerIpAddr = 21,
    kManufCVC = 32,
    kCosignerCVC = 33,
    kSnmp3KickstartValue = 34,
        kKickSecurityName = 1,
        kKickMgrPublicNum = 2,
    kSnmp3NotifyReceiver = 38,
        kSnmpNotifyIp = 1,
        kSnmpNotifyPort = 2,
        kSnmpNotifyType = 3,
        kSnmpNotifyTimeout = 4,
        kSnmpNotifyRetries = 5,
        kSnmpNotifyFilterParams = 6,
        kSnmpNotifySecurityName = 7,
        kSnmpNotifyIpV6 = 8,
    kMcastMacAddr = 42,
    kDutFiltering = 45,
    kSnmpV1V2Coexistence = 53, 
        kSnmpV1V2CommunityName = 1, 
        kSnmpV1V2Transport = 2, 
            kSnmpV1V2TransportAddress = 1, 
            kSnmpV1V2TransportAddressMask = 2, 
        kSnmpV1V2ViewType = 3, 
        kSnmpV1V2ViewName = 4, 
    kSnmpV3AccessView = 54, 
        kSnmpV3AccessViewName = 1, 
        kSnmpV3AccessViewSubtree = 2, 
        kSnmpV3AccessViewMask = 3, 
        kSnmpV3AccessViewType = 4, 
    kSnmpCpeAccessEnable = 55,
    kSwUpgradeServerIpV6Addr = 58,
    kStaticMcastEncoding = 64,
        kStaticMcastGroupIpAddr = 1,
        kStaticMcastSourceIpAddr = 2,
        kStaticMcastCmim = 3,
    kL2VpnMacAging = 65,
        kL2VpnMacAgingMode = 1,
            kL2VpnMacAgingModeDisabled = 0,     // value for kL2VpnMacAgingMode type
            kL2VpnMacAgingModeEnabled = 1,      // value for kL2VpnMacAgingMode type
    kEstbConfig = 217
} CmConfigSpecificTypeCodes;

// Top level type codes specific to REGistration messages.
// Refer to App. C. in DOCSIS RFI spec.
typedef enum
{
    kCosRspSettings = 1,        // sub-types, specific to REG-RSP 
    kModemCapabilities = 5,     // sub-types
    kVendorId = 8,
    kDepracatedV10CmIpAddr = 12,
    kSvcNotAvailableRsp = 13,
        kSvcNaReasonOther = 1,
        kSvcNaReasonUnrecognizedSetting = 2,
        kSvcNaReasonTemp = 3,
        kSvcNaReasonPermanent = 4,

    kVendorSpecificModemCapabilities = 44,  // sub-types
        kVndSpNumDsRxSupported = 1,         // prototype ds channel bonding 
                                            // "number of downstream receivers supported." 
                                            // NOT official DOCSIS...

    kCmTccCmd = 46,                         // sub-types --> see TccTypeCodes

    kCmSvcFlowSidClusterSettings = 47,      // sub-types --> see CmSvcFlowSidClusterCodes

    kRcpCmMultiDsChanProfile = 48,			// sub-types --> see RcxTypeCodes

    kRccCmMultiDsChanConfig = 49,           // sub-types --> see RcxTypeCodes

    kRegCmDsidCommand = 50,                 // sub-types --> see CmDsidCommandTypeCodes

    kRegSecurityAssociation = 51,           // sub-types --> see SecurityAssociation

    kRegInitializingChannelTimeout = 52,

    kCmInitializationReason = 57  

} RegSpecificTypeCodes;


// Top level type codes specific to Dynamic Service (DSx) and Dynamic
// Channel Change (DCC) messages.
// Refer to App. C. in DOCSIS RFI spec.
typedef enum
{
    kDsxHmacDigest = 27, 
        kDsxHmacDigestLen = 20,
    kDsxAuthBlock = 30,    
    kDsxKeySeqNum = 31,
        kDsxKeySeqNumMin = 0,
        kDsxKeySeqNumMax = 15
} DsxDccSpecificTypeCodes;

// value codes for DOCSIS 1.0 REG-RSP Response data item (not really a TLV thing...)
typedef enum
{
    kOk = 0,
    kAuthenticationFailure = 1,
    kClassOfServiceFailure = 2
} RegResponseCodes;

// value codes for REG and DSx Confirmation Code data item (not really a TLV thing...)
typedef enum
{
    kConfOk = 0,
    kRejOther = 1,

    // aren't TLV parsers supposed to skip over unrecognized Type codes????
    kRejUnrecognizedConfigSetting = 2, 
    kRejOutOfRangeError = kRejUnrecognizedConfigSetting,

    kRejTemporary = 3,
    kRejPermanent = 4,
    kRejNotOwner = 5,
    kRejSvcFlowNotFound = 6,
    kRejSvcFlowExists = 7,
    kRejRequiredParamNotPresent = 8,
    kRejHeaderSuppression = 9,
    kRejUnknownTransId = 10,

    // computed HMAC digest for received msg did not match HMAC 
    // digest specified by the message sender.
    kRejAuthFailureBadHmac = 11,
    kRejAddAborted = 12,
    kRejMultipleErrors = 13,
    kRejClassifierNotFound = 14,
    kRejClassifierExists = 15,
    kRejPhsRuleNotFound = 16,
    kRejPhsRuleExists = 17,
    kRejDupeId = 18,
    kRejMultipleUsFlows = 19,
    kRejMultipleDsFlows = 20,
    kRejClassifierAssociatedWithAnotherFlow = 21,
    kRejPhsAssociatedWithAnotherFlow = 22,
    kRejParamInvalidForContext = 23,

    // "authorization module" (a.k.a. big bro) says requester is not
    // authorized.
    kRejAuthFailureBigBro = 24,

    kRejTempDcc = 25,
    kRejDownstreamInconsistency = 26,
    kRejUpstreamInconsistency = 27,
    kRejInsufficientSidClusterResources = 28,
    kRejMissingRcp = 29,
    kPartialService = 30,
    kRejTempDbc = 31,
    kRejUnknownDsid = 32,
    kRejUnknownSidCluster = 33,
    kRejInvalidInitializationTechnique = 34,
    kRejNoChange = 35,
    kRejInvalidDbcRequest = 36,
    kRejModeSwitch = 37,
    kRejInsufficientTransmitters = 38,
    kFkaRejInvalidReceiveChannelIndex39 = 39,
    kRejInsufficientDsidResources = 40,
    kRejInvalidDsidEncoding = 41,
    kRejUnknownClientMacAddress = 42,
    kRejUnknownSaid = 43,
    kRejInsufficientSaResources = 44,
    kRejSaEncoding = 45,
    kRejInvalidSaCryptoSuite = 46,
    kRejTekExists = 47,
    kRejInvalidSidClusterEncoding = 48,
    kRejInsufficientSidResources = 49,
    kRejUnsupportedParameterChange = 50,
    kRejPhsRuleFullyDefined = 51,
    kRejNoMapsOrUcds = 52,
    kErrT3RetriesExceeded = 53,
    kErrT2Timeout = 54,
    kErrT4Timeout = 55,
    kErrRangeAbort = 56,
    kErrInitChanTimeout = 57,
    kErrDbcReqIncomplete = 58,

    kRejVlanIdInUse = 100,      // L2VPN Confirmation Codes
    kRejMultipointL2vpn = 101,
    kRejMiltipointNSI = 102,

    kRejUnknownRcpId = 160,
    kRejMultipleRcpIDs = 161,
    kRejMissingReceiveModuleIndex = 162,
    kRejInvalidReceiveModuleIndex = 163,
    kRejInvalidReceiveChannelFrequency = 164,
    kRejInvalidRmFirstChannelFrequency = 165,
    kRejMissingRmFirstChannelCenterFrequency = 166,
    kRejNoPrimaryDownstreamChannelAssigned = 167,
    kRejMultiplePrimaryDownstreamChannelsAssigned = 168,
    kRejReceiveModuleConnectivityError = 169,
    kRejInvalidReceiveChannelIndex = 170,    
    kRejCenterFrequencyNotMultipleOf62500Hz = 171,

    // DCC specific conf codes
    kDccDepart = 180,
    kDccArrive = 181,
    kRejDccAlreadyThere = 182,
    kRejDcc20Dest20Disabled = 183,

    // conf codes for "major errors" (a.k.a. errors cannot be linked
    // to a particular Qos parameter set and therefore cannot be indicated
    // via an error set).
    kRejMajorFlowSetsError = 200,
    kRejMajorClassifierSetsError = 201,
    kRejMajorPhsSetsError = 202,
    kRejMultipleMajorErrors = 203,
    kRejMsgSyntaxError = 204,
    kRejPrimaryFlowError = 205,
    kRejMsgTooBig = 206,
    kRejModemCapabilitiesError = 207,
    kRejBadRcs = 208,
    kRejBadTcs = 209

} ConfirmationCodes;




// Sub-Type codes for V1.0 Class of Service request (kCosReqSettings)
typedef enum
{
    kSvcClassId = 1,
    kMaxDsRateBitsPerSec = 2,
    kMaxUsRateBitsPerSec = 3,
    kUsTrafficPriority = 4,
    kMinUsResRateBitsPerSec = 5,
    kMaxUsBurstBytes = 6,
    kCosBpkmEnable = 7
} CosReqSettingsCodes;

// Sub-Type codes for V1.0 Class of Service Response (kCosRspSettings)
typedef enum
{
    kRspSvcClassId = 1,
    kRspSid = 2
} CosRspSettingsCodes;

// Sub-Type codes for Modem Capabilities (kModemCapabilities)
typedef enum
{
    kConcatSupport = 1,
    kDocsisVersion = 2,
        // value codes for kDocsisVersion attribute.
        // NOTE: the docsis spec convention for version numbers is
        // different for MAC management message header version number
        // and modem capability docsis version supported.  way to go
        // white board guys and gals!  MAC mgt msg header version number
        // range begins with 1 while the modem capabilities version number
        // range begins with 0.
        //
        kDocsisV1_0 = 0,
        kDocsisV1_1 = 1,
        kDocsisV1_2 = 2,    // wacky name...
        kDocsisV2_0 = kDocsisV1_2,
        kDocsisV3_0 = 3,

        kMdmCapDocsisV1_0 = kDocsisV1_0,
        kMdmCapDocsisV1_1 = kDocsisV1_1,
        kMdmCapDocsisV2_0 = kDocsisV2_0,
        kMdmCapDocsisV3_0 = kDocsisV3_0,

    kFragSupport = 3,
    kPhsSupport = 4,
    kIgmpSupport = 5,
    kBpiVersion = 6,
        // value codes for kBpiVersion attribute
        kBpi = 0,
        kBpiPlus = 1,
    kNumDsSaidSupported = 7,
    kNumUsSidSupported = 8,
    kFilteringOptions = 9,
        // bitmask(!) value codes for kFilteringOptions attribute
        kFilter802_1P = 0x01,
        kFilter802_1Q = 0x02,
    kTransmitEqualizerTapsPerSymbol = 10,
    kNumberOfTransmitEqualizerTaps = 11,
    kDccSupport = 12,
    kNumberOfIpFiltersSupported = 13,
    kNumberOfLlcFiltersSupported = 14,
    kExpandedUnicastSidSpaceSupported = 15,
    kRangingClassId = 16,
        kRngIdCm = 1,
        kRngIdEps = 2,
        kRngIdEmta = 4,
        kRngIdDsgOrEstb = 8,
    kL2VpnCapability = 17,
    kESafeHostCapability = 18,
		kePS = 1,
		keMTA = 15,
		keSTB_IP = 16,
		keSTB_DSG = 17,
    kDutFilteringSupport = 19,

	// begin DOCSIS 3.0 capabilities...
	kUsFreqBand = 20,
		kUsFreqBand5to42MHzNorthAmer = 0,
		kUsFreqBand5to65MHzEuro = 1,
		kUsFreqBand5to85MHz = 2,

    kNumUsTxSupported = 24,    

    kNumDsRxSupported = 29,    
	kTotNumDsidsSupported = 30,
	kNumReseqDsidsSupported = 31,
	kNumMcastDsidsSupported = 32,
    
	kMcastDsidForwardingSupport = 33,       
		kMcastDsidFwdNone = 0,
		kMcastDsidFwdGmacExplicit = 1,
		kMcastDsidFwdGmacPromiscuous = 2,

	kFCType10ForwardCapabilitySupport = 34,
		kFCType10NotFwd = 0,
		kFCType10Fwd = 1,

    kNumUgsOnlyUsFlows = 36,
    kRxMapUcdOnNonPrimaryDsChan = 37,
    kUsDropClassifier = 38,
    kIpV6 = 39


	// end DOCSIS 3.0 capabilities...

} ModemCapabilitiesCodes;


// Sub-Type codes for CM Baseline Privacy settings (kCmBpkmSettings)
typedef enum
{
    kAuthWaitPeriodSec = 1,
    kReauthWaitPeriodSec = 2,
    kAuthGracePeriodSec = 3,
    kOpWaitPeriodSec = 4,
    kRekeyWaitPeriodSec = 5,
    kTekGracePeriodSec = 6,
    kAuthRejectWaitPeriodSec = 7,
    kSaMapWaitPeriodSec = 8,
    kSaMapMaxRetries = 9
} CmBpkmSettingsCodes;

// Sub-Type codes common to Upstream and Downstream Packet Classifier settings
typedef enum
{
    kCmClassifierRef = 1,
    kCmtsClassifierId = 2,
    kPktCmFlowRef = 3,
    kPktSfid = 4,
    kRulePriority = 5,
    kIsActive = 6,
    kDscAction = 7,
        // value codes for kDscAction attribute
        kDscAddClassifier = 0,
        kDscReplaceClassifier = 1,
        kDscDeleteClassifier = 2,
    kClassifierError = 8,
    kIpClassifierSets = 9,
    kEthernetClassifierSets = 10,
    k8021PqClassifierSets = 11,
    kIpv6ClassifierSets = 12,       // PR13079
    kCmimClassifier = 13,
    kClassifierVendorSpecific = 43
} PktClassifierCodes;

// Sub-Type codes specific to Upstream Packet Classifier settings
typedef enum
{
    kDscActivationSignal = 12,
        // value codes for kDscActivationSignal attribute
        kChangeOnRequest = 1,
        kChangeOnAck = 2
} UsSpecificPktClassifierCodes;

// Sub-Type codes for IP Classifier settings
typedef enum
{
    kTosRangeAndMask = 1,
    kProtocolType = 2,
    kSourceIpAddr = 3,
    kSourceIpMask = 4,
    kDestIpAddr = 5,
    kDestIpMask = 6,
    kSourcePortStart = 7,
    kSourcePortEnd = 8,
    kDestPortStart = 9,
    kDestPortEnd = 10
} kIpClassifierCodes;

// PR13079 - Sub-Type codes for IPv6 Classifier settings
typedef enum
{
    kIpv6TrafficClassRangeAndMask = 1,
    kIpv6FlowLabel = 2,
    kIpv6NextHeaderType = 3,
    kIpv6SourceAddress = 4,
    kIpv6SourcePrefixLength = 5,
    kIpv6DestAddress = 6,
    kIpv6DestPrefixLength = 7,
} Ipv6ClassifierCodes;

// Sub-Type codes for Ethernet LLC Classifier settings
typedef enum
{
    kDestMacAddrAndMask = 1,
    kSourceMacAddr = 2,
    kEtherType = 3
} kEthernetClassifierCodes;


// Sub-Type codes for IEEE 802.1P/Q Classifier settings
typedef enum
{
    kUserPriority = 1,
    kVLanId = 2
} k8021PqClassifierCodes;

// Sub-Type codes common to Upstream and Downstream QoS Flow settings
typedef enum
{
    kCmFlowRef = 1,
    kSfid = 2,
    kSid = 3,
    kSvcClassName = 4,
    kQosFlowError = 5,      // sub-types
    kQosParamSetType = 6,
        kProvisioned =      0x01,
        kAdmitted =         0x02,
        kActive =           0x04,
    kTrafficPriority = 7,
    kMaxRateBitsPerSec = 8,
    kMaxBurstBytes = 9,
    kMinResRateBitsPerSec = 10,
    kMinResRatePacketSizeBytes = 11,
    kInactivityTimerPeriodActiveSec = 12,
    kInactivityTimerPeriodAdmittedSec = 13,

    kIpTosOverwrite = 23,

    kCmtsFlowRequiredAttrMask = 31,
    kCmtsFlowForbiddenAttrMask = 32,
    kCmtsFlowAttrAggRuleMask = 33,
    kCmtsFlowAppId = 34,

    kQosVendorSpecific = 43
} QosCommonFlowSettingsCodes;

// Sub-Type codes specific to Upstream QoS Flow settings (kQosUsFlowSettings)
typedef enum
{
    kMaxConcatBurstBytes = 14,
    kUsBwCmtsSchedulingStrategy = 15,
        kCmtsVendorSpecific = 1,
        kBestEffort = 2,
        kNonRtpRequest = 3,
        kRtpRequest = 4,
        kUGrantActivityDetection = 5,
        kUGrant = 6,
    kCmReqTxOptions = 16,
        kNoBcastBwRequest =         0x0001,
        kNoMcastPriorityRequest =   0x0002,
        kNoBwRequestInReqData =     0x0004,
        kNoDataInReqData =          0x0008,
        kNoPiggybackRequestWGrant = 0x0010,
        kNoConcat =                 0x0020,
        kNoFrag =                   0x0040,
        kNoPhs =                    0x0080,
        kDropDataGtUgsGrant =       0x0100,
        kNoSegmentHeaders =         0x0200,
        kNoMultOutBcastBwReq =      0x0400,

    kRequestPeriodMicrosec = 17,
    kRequestJitterMicrosec = 18,
    kUGrantSizeMacBytes = 19,
    kUGrantPeriodMicrosec = 20,
    kUGrantJitterMicrosec = 21,
    kUGrantsPerPeriod = 22,
    kUGrantTimeRefDsSyncTicks = 24,

    kReqBkoffWinMult8x = 25,
    kByteReqSizeMult = 26

} QosUsSpecificFlowSettingsCodes;

// Sub-Type codes specific to Downstream QoS Flow settings (kQosDsFlowSettings)
typedef enum
{
    kMaxDsLatencyMicrosec = 14,
    kDownstreamResequencing = 15,

    kCmtsDsPeakTrafficRateBitsPerSec = 16,
    kCmtsDsReseqDisable = 17

} QosDsSpecificFlowSettingsCodes;

// Sub-Type codes for Payload Header Suppression settings (kPhs)
typedef enum
{
    kPhsCmClassifierRef = 1,
    kPhsCmtsClassifierId = 2,
    kPhsCmFlowRef = 3,
    kPhsSfid = 4,
    kPhsDscAction = 5,
        // value codes for kPhsDscAction attribute
        kDscAddPhsRule = 0,
        kDscSetPhsRule = 1,
        kDscDeletePhsRule = 2,
        kDscDeleteAllPhsRules = 3,
        kDscUndoSetPhsRule = 177,
        kDscReplacePhsRule = kDscUndoSetPhsRule,
    kPhsError = 6,
    kPhsfSuppressedString = 7,
    kPhsiIndex = 8,
    kPhsmMask = 9,
    kPhssSize = 10,
    kPhsvVerificationEnable = 11,
    kPhsDbcAction = 13,
        // value codes for kPhsDbcAction attribute
        kDbcAddPhsRule = 0,
        kDbcDeletePhsRule = 1,
    kPhsVendorSpecific = 43
} PhsSettingsCodes;


// Confirmation codes for DBC specific errors
typedef enum
{
    // UNFINISHED - THESE CODES HAVE BEEN DEPRICATED
    kRejUnknownChanID = 221,
    kRejInsufficientReceivers = 229,
} DbcErrorCodes;

// Sub-Type codes common to kQosFlowError, kClassifierError and kPhsError 
typedef enum
{
    kErrorType = 1,
    kErrorCode = 2,
        // value encodings for kErrorCode
        kErrSuccess = 0,
        kRejectOther = 1,
        kRejectUnrecognizedSetting = 2,
        kRejectTempUnavail = 3,
        kRejectAdmin = 4,
        kRejectNotOwner = 5,
        kRejectSvcFlowNotFound = 6,
        kRejectSvcFlowExists = 7,
        kRejectRequiredSettingNotPresent = 8,
        kRejectPhs = 9,
        kRejectBadTransId = 10,
        kRejectAuthenticationFailure = 11,
        kRejectAddAborted = 12,
        kRejectMultipleErrors = 13,
        kRejectClassifierNotFound = 14,
        kRejectClassifierExists = 15,
        kRejectPhsRuleNotFound = 16,
        kRejectPhsRuleExists = 17,
        kRejectDuplicateId = 18,
        kRejectMultipleUsFlows = 19,
        kRejectMultipleDsFlows = 20,
        kRejectClassifierForAnotherSvcFlow = 21,
        kRejectPhsForAnotherSvcFlow = 22,
        kRejectInvalidParameter = 23,
        kRejectAuthorizationFailure = 24,
        kRejectTempDcc = 25,
    kErrorString = 3
} ErrorSetCodes;


typedef enum
{
    kLkDirectionUpstream = 0,
    kLkDirectionDownstream = 1,
    kLkDirectionUnknown = 2
} LinkedQosFlowSettingsDirection;


// Sub-Type codes for Telco Return
typedef enum
{
    kTrIspName = 2,
    kPhoneNum1 = 3,
    kPhoneNum2 = 4,
    kPhoneNum3 = 5,
    kConnectionThreshold = 6,
    kTrUsername = 7,
    kTrPassword = 8,
    kDhcpServerAuthEnable = 9,
    kDhcpServerIpAddr = 10,
    kRadiusRealmName = 11,
    kPppAuthMethod = 12,
    kDdInactivityTimerPeriodSec = 13
} TelcoReturnSettingsCodes;


// Usage context codes used for configuration settings used in 
// CM configuration files, registration, and DSx messaging.
typedef enum
{
    // for contexts kCmConfigFile thru kRegAck, the context dictates
    // which side tx the msg and which side rx the msg.
    kCmConfigFile = 1,
    kRegReq,
        // NOTE: a CM can determine kRegReqDocs10 vs. kRegReqDocs11 prior
        // to encoding a REG-RSP, so the next (2) contexts are useful for
        // CM applications. 
        kRegReqDocsis10,
        kRegReqDocsis11,
    kRegRsp,
    kRegAck,

    // DSA-REQ msg tx by CM and rx by CMTS
    kCmTxDsaReq,
    //
    // DSA-REQ msg tx by CMTS and rx by CM
    kCmtsTxDsaReq,
    

    // DSA-RSP msg tx by CM and rx by CMTS
    kCmTxDsaRsp,
    //
    // DSA-RSP msg tx by CMTS and rx by CM
    kCmtsTxDsaRsp,


    // DSA-ACK msg tx by CM and rx by CMTS
    kCmTxDsaAck,
    //
    // DSA-ACK msg tx by CMTS and rx by CM
    kCmtsTxDsaAck,


    // DSC-REQ msg tx by CM and rx by CMTS
    kCmTxDscReq,
    //
    // DSC-REQ msg tx by CMTS and rx by CM
    kCmtsTxDscReq,
    

    // DSC-RSP msg tx by CM and rx by CMTS
    kCmTxDscRsp,
    //
    // DSC-RSP msg tx by CMTS and rx by CM
    kCmtsTxDscRsp,


    // DSC-ACK msg tx by CM and rx by CMTS
    kCmTxDscAck,
    //
    // DSC-ACK msg tx by CMTS and rx by CM
    kCmtsTxDscAck,


    // DSD-REQ msg tx by CM and rx by CMTS
    kCmTxDsdReq,
    //
    // DSD-REQ msg tx by CMTS and rx by CM
    kCmtsTxDsdReq,
    

    // DSD-RSP msg tx by CM and rx by CMTS
    kCmTxDsdRsp,
    //
    // DSD-RSP msg tx by CMTS and rx by CM
    kCmtsTxDsdRsp,

    // service flow in nominal state (no DSx transactions in progress)
    kSvcFlowNominal,

    // DBC-REQ msg tx by CMTS and rx by CM
    kDbcReq,

    // DBC-RSP msg tx by CM and rx by CMTS
    kDbcRsp,

    // DBC-ACK msg tx by CMTS and rx by CM
    kDbcAck,

    kUcEndOfList
} UsageContext;



// PHS context codes used for PHS Settings.
typedef enum
{
    // PHS is service flow based
    kServiceFlowPhs = 1,

    // PHS is multicast DSID based
    kMulticastDsidPhs,

    kPcEndOfList
} PhsContext;


typedef enum 
{
    // normal case: install upstream and downstream linked QoS settings 
    // (flow, classifier and phs settings).
    kDsxInstallUsAndDsLkFlowSets = 0,

    // special case: install upstream linked QoS settings 
    // (flow, classifier and phs settings).
    kDsxInstallUsLkFlowSets,

    // special case: install downstream linked QoS settings 
    // (flow, classifier and phs settings).
    kDsxInstallDsLkFlowSets

} DsxInstallMode;


typedef enum 
{
    kDSA = 0,   // DSX Add
    kDSC,       // DSX Change
    kDSD        // DSX Delete

} DsxMsgType;


typedef enum
{
    kBDcdDownstreamChannelRFParameters = 4,
        kBDcdDownstreamChannelId = 1,
        kBDcdRfChannelFrequency = 2,
        kBDcdRfChannelModulationType = 3,
            kBDcdModulationTypeQPSK = 0,
            kBDcdModulationType16Qam = 1,
            kBDcdModulationType32Qam = 2,
            kBDcdModulationType64Qam = 3,
            kBDcdModulationType128Qam = 4,
            kBDcdModulationType256Qam = 5,
            kBDcdModulationType512Qam = 6,
            kBDcdModulationType1024Qam = 7,
			kBDcdModulationTypeReserved = 8,
        kBDcdRfChannelAnnexType = 4,
            kBDcdAnnexTypeB = 0,
            kBDcdAnnexTypeA = 1,
            kBDcdAnnexTypeD = 2,
            kBDcdAnnexTypeH = 3,

    kBDCDDownstreamBondingGroups = 5,
        kBDcdDownstreamBondingGroupId = 1,
        kBDcdDownstreamPrimaryControlChannelId = 2,
        kBDcdDownstreamBondingChannelId = 3,
                                              
} kBDcdTypeCodes;

// Sub-Type codes for L2VPN (kL2Vpn)
typedef enum
{
    kVpnID = 1,
    kNsiEncap = 2,
    kESafeDhcpSnooping = 3,
    kCmim = 4,
    kAttachmentGroupID = 5,
    kSourceAttachmentGroupID = 6,
    kTargetAttachmentGroupID = 7,
    kIngressUserPriority = 8,
    kUserPriorityRange = 9,
    kL2VpnSaDescriptor = 10,
    kL2VpnVendorSpecific = 43,

    kDutFilteringEncoding = 45,
        kDutControl = 1,
        kDutCmim = 2,
} L2VpnCodes;


// Type codes and value mappings specific to type 50 DSID command encoding
// used in DOCSIS 3.0 REG-RSP and DBC messages.  
//
typedef enum
{
    kDsidValue = 1,

    kDsidAction = 2,
        kDsidAdd = 0,
        kDsidChange = 1,
        kDsidDelete = 2,

    kDsidReseqSettings = 3,
        kReseqEnable = 1,
            kReseqDisabled = 0,
            kReseqEnabled = 1,
        kReseqDsChanIdVector = 2,
        kReseqWaitTime100Microsec = 3,
        kReseqWarningThreshold100Microsec = 4,          
        kReseqCmStatHoldoffTimerPeriod20Millisec = 5,   

    kDsidMcastCommand = 4,                              // not supported in phase1
        kMcastClientMacAddrCmd  = 1,                    // not supported in phase1
            kClientMacAddrAction = 1,                   // not supported in phase1
                kClientMacAddrAdd = 0,                  // not supported in phase1
                kClientMacAddrDelete = 1,               // not supported in phase1
            kClientMacAddr = 2,                         // not supported in phase1

        kMcastCmim = 2,                                 // not supported in phase1
        kMcastGroupMacAddrVector = 3,                   // not supported in phase1    
        kMcastPhsEncoding = 26

} CmDsidCommandTypeCodes;



// Type codes and value mappings specific to type 51 security assocaition encoding
// used in DOCSIS 3.0 REG-RSP and DBC messages.  
//
typedef enum
{
    kSaAction = 1,
        kSaActionAdd = 0,
        kSaActionDelete = 1,                            
} SecurityAssociationActionCodes;


// Type codes and value mappings specific to type 57 CM initialization reason
// used in DOCSIS 3.0 REG-REQ and REG-REQ-MP messages, as defined in MULPI table C-3.  
//
typedef enum
{
    kPowerOn = 1,
    kT7LostSync = 2,
    kAllUsFailed = 3,
    kBadDhcpAck = 4,
    kLinkLocalAddressInUse = 5,
    kT6Expired = 6,
    kRegRspNotOk = 7,
    kBadRccTcc = 8,
    kFailedPrimDs = 9,
    kTcsFailedOnAllUs = 10,
    kFailedCLusterEncoding = 11,
    kExceededReqAckNoMtcm1 = 12,
    kExceededReqAckNoMtcm2 = 13,
    kExceededRegAckMtcm = 14,
    kMtcmChange = 15,
    kT4Expired = 16,
    kNoPrimUscan = 17,
    kCmCntrlInit = 18
} CmInitializationReasonCodes;

// returns enum name as char*.  useful for debug output.
const char* UsageContextName( int context );
const char* PhsContextName( int context );
const char* RngStatusName( int rng_status );
const char* RngStrategyName( int rng_strategy );
const char* ConfirmationCodeName( int conf_code );
const char* UsBwSchedulingStrategyName( int us_bw_strategy );
const char* CosSvcNaReasonName( int reason );
const char* DccUsInitStrategyName( int us_init_strategy );
const char* DsxInstallModeName( DsxInstallMode dsx_install_mode );
const char* DsidActionName( unsigned int dsid_action );
const char* CmStatEventTypeName( unsigned int event_type );
const char* TccActionName( unsigned int tcc_action );
const char* TccRngStrategyName( int tcc_rng_strategy );
const char* SidToUsCmdActionName( unsigned int sid_to_us_cmd_action );

//
//
// DocsisDccConfCodeToSnmpEventLogCode() - Converts Docsis App. C confirmation code
//      to SNMP event log code for Dynamic Channel Change messaging context.
//      NOTE: DOCSIS confirmation codes can apply to multiple messaging
//      contexts (DCC, DSA, DSC, DSD, etc.) while SNMP event codes tend
//      to be context specific.
//
int DocsisDccConfCodeToSnmpEventLogCode( int docsis_conf_code );

//
//
// DocsisDsxConfCodeToSnmpEventLogCode() - Converts Docsis App. C confirmation code
//      to SNMP event log code for Dynamic Channel Change messaging context.
//      NOTE: DOCSIS confirmation codes can apply to multiple messaging
//      contexts (DCC, DSA, DSC, DSD, etc.) while SNMP event codes tend
//      to be context specific.
//
int DocsisDsxConfCodeToSnmpEventLogCode( int docsis_conf_code, int usage_context );


// common name strings for debug output.
extern const char* spec;                 // setting value specified
extern const char* no_spec;              // setting value not specified
extern const char* reserved_value;      
extern const char* undefined_value;  

// Meaningful names for the Sevice Flow Encodings (SFE) TLV types.  These come
// from Appendix C in the spec.
#define kUpstreamServiceFlowEncodings 24
#define kDownstreamServiceFlowEncodings 25

#define kSFE_ServiceFlowIdentifier 2
#define kSFE_ServiceIdentifier 3
#define kSFE_ServiceClassName 4

// Other type values.
#define kEndOfDataMarker 255

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************

//********************** Inline Method Implementations ***********************

#endif



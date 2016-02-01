//****************************************************************************
//
// Copyright (c) 2011 Broadcom Corporation
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
//    Description:                                                          
//                                                                          
//****************************************************************************
//    Revision History:                                                     
//                                                                          
//****************************************************************************
//    Filename: ocStbHostBridge.cpp
//    Creation Date: Monday, June 13, 2011 at 13:53:49
//    Created by Broadcom BFC Mib Compiler BCMIBC.EXE version 3.6.1d
//    from input file OC-STB-HOST-MIB.mib
//
//****************************************************************************

#include "MibObjs.h"
#include "SnmpLog.h"
#include "MergedNonVolDefaults.h"
#include "ocStbHostBridge.h"


/******************************************************************************\

OC-STB-HOST-MIB DEFINITIONS ::= BEGIN

IMPORTS
    TenthdB,
    TenthdBmV
        FROM DOCS-IF-MIB                    -- RFC 2670
    OBJECT-GROUP,
    MODULE-COMPLIANCE
        FROM SNMPv2-CONF                    -- RFC 2580
    Integer32,
    Unsigned32,
    Counter32,
    TimeTicks,
    OBJECT-TYPE,
    MODULE-IDENTITY,
    OBJECT-IDENTITY
        FROM SNMPv2-SMI                     -- RFC 2578
    AutonomousType,
    DisplayString,
    TruthValue,
    RowPointer,
    TimeStamp,
    PhysAddress,
    MacAddress,
    DateAndTime,
    TEXTUAL-CONVENTION
        FROM SNMPv2-TC                      -- RFC 2579
    SnmpAdminString
        FROM SNMP-FRAMEWORK-MIB             -- RFC 3411
    InetAddressType,
    InetAddress
        FROM INET-ADDRESS-MIB               -- RFC 4001
    hrDeviceIndex
        FROM HOST-RESOURCES-MIB             -- RFC 2790
    clabProjOpenCable
        FROM CLAB-DEF-MIB
        ;

ocStbHostMibModule MODULE-IDENTITY
    LAST-UPDATED "201102040000Z"        -- February 4, 2011
    ORGANIZATION "Cable Television Laboratories, Inc."
    CONTACT-INFO
         "Postal: Cable Television Laboratories, Inc.
         858 Coal Creek Circle
         Louisville, Colorado 80027-9750
         U.S.A.
         Phone: +1 303-661-9100
         Fax:    +1 303-661-9199
         E-mail: mibs@cablelabs.com"
    DESCRIPTION
            "This MIB module contains the management objects for the
            management of OpenCable Set-top Host Device."
    REVISION "201102040000Z"        -- February 4, 2011
    DESCRIPTION
            "Added S3D support with ocStbHostDVIHDMISupported3DStructures
            and ocStbHostDVIHDMIActive3DStructure.
            Added ocStbHostFirmwareDownloadGroupId.
            Added ocStbHostDVIHDMI3DCompatibilityControl,
            ocStbHostDVIHDMI3DCompatibilityMsgDisplay,
            ocStbHostSoftwareApplicationInfoSigLastNetworkVersionRead,
            ocStbHostSoftwareApplicationInfoSigVersionInUse."
    REVISION "201005070000Z"        -- May 7, 2010
    DESCRIPTION
            "Deprecated ocStbHostSoftwareApplicationSigStatus
            and added replacement leaf objects,
            ocStbHostSoftwareApplicationInfoSigLastReceivedTime
            and ocStbHostSoftwareApplicationInfoSigLastReadStatus.
            Added ocStbHostFirmwareDownloadFailedCount.
            Added ochdtc(7) to ocStbHostCapabilities, other(4) to
            ocStbHostFirmwareCodeDownloadStatus, and
            ocStbHostDVIHDMIEdidVersion."
    REVISION "200912110000Z"        -- December 11, 2009
    DESCRIPTION
            "Added ocStbHostDVIHDMIPreferredVideoFormat,
            ocStbHostDVIHDMIAvailableVideoFormatTable,
            ocStbHostAvcSupport, and
            ocStbHostCardSnmpAccessControl. Also removed
            extraneous ',' in HDMI Features object."
    REVISION "200909040000Z"        -- September 4, 2009
    DESCRIPTION
            "Added 'other(6)' enumerated value to
            ocStbHostDVIHDMIAttachedDeviceType, changed
            ocStbHostSoftwareApplicationSigStatus SYNTAX to
            enumerated values(INTEGER)."
    REVISION "200905080000Z"        -- May 8, 2009
    DESCRIPTION
            "Added HDMI objects ocStbHostDVIHDMIEdid,
            ocStbHostDVIHDMILipSyncDelay,
            ocStbHostDVIHDMICecFeatures,
            ocStbHostDVIHDMIFeatures,
            ocStbHostDVIHDMIMaxDeviceCount.
            Added the following objects to support the enhanced
            diagnostics display: ocStbHostDVIHDMIAttachedDeviceType,
            ocStbHostSoftwareFirmwareReleaseDate,
            ocStbHostSoftwareOrganizationId,
            ocStbHostSoftwareApplicationId,
            ocStbHostSoftwareApplicationSigStatus,
            ocStbHostPatTimeoutCount,
            ocStbHostPmtTimeoutCount,
            ocStbHostOobCarouselTimeoutCount,
            ocStbHostInbandCarouselTimeoutCount,
            ocStbHostCardOpenedGenericResource,
            ocStbHostCardTimeZoneOffset,
            ocStbHostCardDaylightSavingsTimeDelta,
            ocStbHostCardDaylightSavingsTimeEntry,
            ocStbHostCardDaylightSavingsTimeExit,
            ocStbHostCardEaLocationCode,ocStbHostCardVctId,
            ocStbHostCardCpAuthKeyStatus,
            ocStbHostCardCpCertificateCheck,
            ocStbHostCardCpCciChallengeCount,
            ocStbHostCardCpKeyGenerationReqCount,
            ocStbHostCardCpIdList,ocStbHostBootStatus,
            ocStbHostInBandTunerBandwidth.
            Added frameRate enumeration of 60Hz to
            ocStbHostDVIHDMIFrameRate.
            Added DateAndTime textual convention import."
    REVISION "200902060000Z"        -- February 6, 2009
    DESCRIPTION
            "Added 1) Notification support to MIB Notifications
            section, 2) a trap for signaling the headend a dump is
            ready to be transferred, 3) ocStbHostDumpEventCount,
            ocStbHostDumpNow, ocStbHostDumpEventTimeout, and
            ocStbHostDumpFilePath objects for Panic Dump transfer
            support, 4) new group and trap to the Notifications
            conformance section, 5) ocStbHostCfrSpecificationIssue and
            ocStbHostMibSpecificationIssue to report the versions of
            the specifications implemented,
            6) ocStbHostSystemTempMaxValue,
            7)ocStbHostSystemMemoryReportMemoryType and
            ocStbHostSoftwareStatus Objects to the compliance
            section to track obsoleted values.
            Modified 1) ocStbHostSoftwareStatus with new enumerated
            values, 2) ocStbHostInBandTunerPower to read-only
            MAX-ACCESS and 3) ocStbHostCardMfgId by changing SA
            to Cisco and adding Conax."
    REVISION "200811140000Z"        -- November 14, 2008
    DESCRIPTION
            "Added/updated tables to align the MIB with the CCIF
            Diagnostic APDU. Added color space, sample size, and
            frame rate to the ocStbHostDVIHdmiTable, additional memory
            types to the ocStbHostSystemMemoryReportTable, SNMP
            proxy objects, and added Out of Band messaging mode object.
            Corrected ocStbHostCCApplicationType to include 0 as a
            valid index. Modified descriptions to clarify usage of
            ocStbHostMpeg2ContentDecoderPTS, ocStbHostCASystemId,
            ocStbHostCardIpAddressType, and ocStbHostCardIpAddressIp.
            Added TotalTuneCount, TuneFailureCount, and
            TuneFailureFreq columns in
            ocStbHostInBandTunerTable to support field
            diagnostics. Added OIDs for CableCARD ID, CableCARD/Host
            binding status. Deprecated homenetworking and
            ocStbHostSecurityIdentifier objects. Corrected UNITS for
            objects with tenthdB syntax."
    REVISION "200803280000Z"        -- March 28, 2008
    DESCRIPTION
            "Corrected index for ocStbHostSoftwareApplicationInfoTable
            that required adding an index object. Added
            ocStbHostRebootInfo, ocStbHostMemoryInfo, ocStbHostJVMInfo.
            Also repaired a problem in two areas where OID values were
            changed from previous HOSTMIB I02 version. These include
            realignment of objects within the ocStbHostDVIHDMITable and
            ocStbHostFirmwareDownloadStatus. Added 1080p(5) to the
            VideoOutputFormat textual convention; updated the
            ocStbHostAvInterfaceTypes Description field to
            reference the DVIHDMI and 1394 tables; corrected INTEGER
            syntax statements to specify ranges. This version is
            published as part of the OpenCable Host MIB specification,
            MIB-HOST2.X, version I04. Copyright 1999-2008 Cable
            Television Laboratories, Inc. All rights reserved."
    REVISION "200711130000Z"        -- November 13, 2007
    DESCRIPTION
            "Added diagnostics MIB objects to include Home Network
            Table, MemoryReportTable, Software App Table and addition
            of software info MIB objects. Added diagnostics MIB objects
            to include Firmware download
            fail status, device HDCP status, fixed inband tuner
            interleaver object, added host DRI interface, Host
            capabilities, RDC data rate object, added MacAddress syntax
            to imports from rfc2579, changes syntax on RDC and FDC
            power level MIB objects to TenthdBmV, changed the index in
            ocStbHostCCAppInfoTable. This version is published as part
            of the OpenCable Host MIB specification, MIB-HOST2.X,
            version I03. Copyright 1999-2007 Cable Television
            Laboratories, Inc. All rights reserved."
    REVISION "200709210000Z"        -- September 21, 2007
    DESCRIPTION
            "Updated version to cover the addition of
            physical address, address type, ip address and subnetmask
            mib objects to both ocStbCardInfo and ocStbHostInfo,
            Tuner status MIB objects to ocStbHostInBandTunerState,
            MIB Decoder objects to ocStbHostMpeg2ContentTable,
            QAM statistics MIB objects to ocStbHostQPSK,
            CableCard Diagnostics ocStbHostCCMMITable and
            ocStbHostCCAppInfo. This version is published as part of
            the OpenCable Host MIB specification MIB-HOST2.X,
            version I02. Copyright 1999-2007 Cable Television
            Laboratories, Inc. All rights reserved."
    ::= { clabProjOpenCable 1 }

--
-- Textual conventions
--

VideoOutputFormat ::= TEXTUAL-CONVENTION
    STATUS      current
    DESCRIPTION
            "Defines the output format of the video signal
            horizontal and vertical resolution."
    SYNTAX      INTEGER {
                format480i(1),
                format480p(2),
                format720p(3),
                format1080i(4),
                format1080p(5)
                }

AudioOutputFormat ::= TEXTUAL-CONVENTION
    STATUS      current
    DESCRIPTION
            "Defines the output format of the audio signal."
    SYNTAX      INTEGER
                {
                other(1),
                lpcm(2), -- CD-A, DVD-A
                ac3(3), -- Dolby Digital
                eac3(4), -- Dolby Digital Plus
                mpeg1L1L2(5), -- MPEG-1 Layers 1 & 2
                mpeg1L3(6), -- MPEG-1 Layer 3
                mpeg2(7), -- Advanced Audio Coding (AAC-LC)
                mpeg4(8), -- MPEG-4 Advanced Audio Coding
                          -- High Efficiency (AAC PLUS)
                dts(9), -- Digital Theater Sound
                atrac(10) -- Adaptive Transform Acoustic Coding
                }

VideoAspectRatio ::= TEXTUAL-CONVENTION
    STATUS      current
    DESCRIPTION
            "Defines the horizontal and vertical aspect ratio of the
            video signal.
            'fourByThree' means a 4 by 3 aspect ratio.
            'sixteenByNine' means a 16 by 9 aspect ratio.
            'other' indicates a non-specified aspect ratio."
    SYNTAX      INTEGER {
                other(1),
                fourByThree(2),
                sixteenByNine(3)
                }

--
-- Node definitions
--

ocStbHostNotifications OBJECT IDENTIFIER ::= { ocStbHostMibModule 0 }
ocStbPanicDumpTrap NOTIFICATION-TYPE
    OBJECTS {
             ocStbHostDumpFilePath
            }
    STATUS      current
    DESCRIPTION
            "Notifies host that a panic dump condition has occurred.
            The headend then may issue a TFTP Get to retrieve the
            dump or ignore the trap and allow the host
            ocStbHostDumpEventTimeout timer to expire. This trap is
            sent only when the ocStbHostDumpEventCount object has a
            non-zero value."
    ::= { ocStbHostNotifications 1 }

ocStbHostMibObjects OBJECT IDENTIFIER ::= { ocStbHostMibModule 1 }

ocStbHostSystem OBJECT IDENTIFIER ::= { ocStbHostMibObjects 1 }
ocStbHostHWIdentifiers OBJECT IDENTIFIER ::= { ocStbHostSystem 1 }
ocStbHostInterfaces OBJECT IDENTIFIER ::= { ocStbHostSystem 2 }


\******************************************************************************/




/*\
 *
 *  ocStbHostHWIdentifiersGroupBridge for bridging scalars from the ocStbHostHWIdentifiersGroup group 
 *
\*/

/* 
ocStbHostHWIdentifiers OBJECT IDENTIFIER ::= { ocStbHostSystem 1 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostHWIdentifiersGroupBridge::ocStbHostHWIdentifiersGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_ocStbHostHWIdentifiersGroup, pAgent)
{
}

ocStbHostHWIdentifiersGroupBridge::~ocStbHostHWIdentifiersGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostSerialNumber OBJECT-TYPE
    SYNTAX      SnmpAdminString
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The manufacturer's serial number for this device."
    ::= { ocStbHostHWIdentifiers 1 }
*/ 
int ocStbHostHWIdentifiersGroupBridge::Get_ocStbHostSerialNumber (BcmString &ocStbHostSerialNumber)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostSerialNumber" << endl;
  ocStbHostSerialNumber = "(unfinished)";

  return ocStbHostSerialNumber.length();
}


/* 
ocStbHostHostID OBJECT-TYPE
    SYNTAX      DisplayString (SIZE(17))
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Allows the manager to retrieve the 40-bit Host ID
            of the ocStbHost (SNMP apdu TLV #1) in the string
            format indicated below:

           M-MMU-UUU-UUU-UUL

           Where:

           M-MM is the decimal representation of the
           10-bit CHICA assigned manufacturer number.

           U-UUU-UUU-UU is the decimal representation
           of the 30-bit manufacturer assigned unit number.

           L is a Luhn check digit calculated over the
           preceding 12 decimal digits.

            If the Host ID is unknown, then the string shall
            be all zeros e.g., '0-000-000-000-000'."
    ::= { ocStbHostHWIdentifiers 2 }
*/ 
int ocStbHostHWIdentifiersGroupBridge::Get_ocStbHostHostID (BcmString &ocStbHostHostID)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostHostID" << endl;
  ocStbHostHostID = "(unfinished)";

  return ocStbHostHostID.length();
}


/* 
ocStbHostCapabilities OBJECT-TYPE
    SYNTAX      INTEGER {
                other(1),
                ochd2(2),
                embedded(3),
                dcas(4),
                ochd21(5),
                bocr(6),
                ochdtc(7)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The capabilities indication of the STB device.

           'other' means a device which is not described
               by one of the other enumerations in this list.
           'ochd2' means the STB device is an OpenCable Host Device
                Version 2 Compliant device.
            'embedded' means the STB device is of the non-removable
                security type.
            'dcas' means the STB device is of the downloadable
                security type.
            'ochd21' means the STB device is an OpenCable Host Device
                Version 2.1 Compliant device.
            'bocr' means the STB is bi-directional OCUR compliant.
            'ochdtc' means the STB is an OpenCable Host Thin Chassis
                device."
    ::= { ocStbHostHWIdentifiers 3 }
*/ 
int ocStbHostHWIdentifiersGroupBridge::Get_ocStbHostCapabilities ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCapabilities" << endl;
  return 0;
}


/* 
ocStbHostAvcSupport OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The AVC capabilities indication of the STB device.
            The object has a value of true (1) if AVC decode is
            supported. Otherwise, the object has a value of false (2).
            The object is required when the ocStbHostCapabilities value
            is ochd(2); otherwise the object is optional."
    ::= { ocStbHostHWIdentifiers 4 }
*/ 
bool ocStbHostHWIdentifiersGroupBridge::Get_ocStbHostAvcSupport ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostAvcSupport" << endl;
  return true;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostAVInterfaceEntryBridge for bridging elements in the ocStbHostAVInterfaceTable
 *
\*/

/* 
ocStbHostAVInterfaceTable OBJECT-TYPE
    SYNTAX      SEQUENCE OF OcStbHostAVInterfaceEntry
    MAX-ACCESS not-accessible
    STATUS      current
    DESCRIPTION
            "This table contains a list of A/V (Audio/Video)
            interfaces (video inputs/outputs, tuners, audio
            inputs/outputs) in the ocStbHost."
    ::= { ocStbHostInterfaces 2 }

INDEX { ocStbHostAVInterfaceIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostAVInterfaceEntryBridge::ocStbHostAVInterfaceEntryBridge (unsigned long ocStbHostAVInterfaceIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ocStbHostAVInterfaceTable,
                      BcmObjectId(ocStbHostAVInterfaceIndex),
                      pAgent)
{
}

ocStbHostAVInterfaceEntryBridge::~ocStbHostAVInterfaceEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostAVInterfaceIndex OBJECT-TYPE
    SYNTAX      Unsigned32 (1..4294967295)
    MAX-ACCESS not-accessible
    STATUS      current
    DESCRIPTION
            "Index of the table."
    ::= { ocStbHostAVInterfaceEntry 1 }
*/ 
unsigned int ocStbHostAVInterfaceEntryBridge::Get_ocStbHostAVInterfaceIndex ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostAVInterfaceIndex" << endl;
  return 0;
}


/* 
ocStbHostAVInterfaceType OBJECT-TYPE
    SYNTAX      AutonomousType
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "An indication of the type of interface.

            If this value is
               ocStbHostComponentOut
            then a corresponding entry exists in the
            ocStbHostComponentVideoTable.

            If this value is
               ocStbHostRFOutCh
            then a corresponding entry exists in the
            ocStbHostRFChannelOutTable.

            If this value is
               ocStbHostScte40FatRx
            then a corresponding entry exists in the
            ocStbHostInBandTunerTable.

            If this value is
               ocStbHostDviOut or ocStbHostHdmiOut
            then a corresponding entry exists in the
            ocStbHostDVIHDMITable

            If this value is
               ocStbHost1394Out
            then a corresponding entry exists in the
            ocStbHostIEEE1394Table

             If this value is
                hrDeviceOther
             then no further information is available for the device."
     ::= { ocStbHostAVInterfaceEntry 2 }
*/ 
int ocStbHostAVInterfaceEntryBridge::Get_ocStbHostAVInterfaceType (BcmObjectId &ocStbHostAVInterfaceType)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostAVInterfaceType" << endl;
  ocStbHostAVInterfaceType = "0.0";

  return ocStbHostAVInterfaceType.Length();
}


/* 
ocStbHostAVInterfaceDesc OBJECT-TYPE
    SYNTAX      SnmpAdminString
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Textual description of the A/V Interface "
    ::= { ocStbHostAVInterfaceEntry 3 }
*/ 
int ocStbHostAVInterfaceEntryBridge::Get_ocStbHostAVInterfaceDesc (BcmString &ocStbHostAVInterfaceDesc)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostAVInterfaceDesc" << endl;
  ocStbHostAVInterfaceDesc = "(unfinished)";

  return ocStbHostAVInterfaceDesc.length();
}


/* 
ocStbHostAVInterfaceStatus OBJECT-TYPE
    SYNTAX      INTEGER
                {
                enabled(1),
                disabled(2)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Status of the interface.
            'enabled' indicates the A/V interface is active and can be
            used for the user.
            'disabled' indicates the A/V interface is currently not
            available for the user due to hardware, software or
            configuration conditions.
            Additional information about the interface status may be
            available in the A/V Type interface Table extensions."
    ::= { ocStbHostAVInterfaceEntry 4 }
*/ 
int ocStbHostAVInterfaceEntryBridge::Get_ocStbHostAVInterfaceStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostAVInterfaceStatus" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostSPDIfEntryBridge for bridging elements in the ocStbHostSPDIfTable
 *
\*/

/* 
ocStbHostSPDIfTable OBJECT-TYPE
    SYNTAX      SEQUENCE OF OcStbHostSPDIfEntry
    MAX-ACCESS not-accessible
    STATUS      current
    DESCRIPTION
            "This table allows the manager to retrieve information
            about the SONY, Philips Digital (SPD) Interface of the
            ocStbHost."
    ::= { ocStbHostInterfaces 6 }

INDEX { ocStbHostAVInterfaceIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostSPDIfEntryBridge::ocStbHostSPDIfEntryBridge (unsigned long ocStbHostAVInterfaceIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ocStbHostSPDIfTable,
                      BcmObjectId(ocStbHostAVInterfaceIndex),
                      pAgent)
{
}

ocStbHostSPDIfEntryBridge::~ocStbHostSPDIfEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostSPDIfAudioFormat OBJECT-TYPE
    SYNTAX      AudioOutputFormat
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The format of the Audio stream transmitted over this
            interface."
    ::= { ocStbHostSPDIfEntry 1 }
*/ 
int ocStbHostSPDIfEntryBridge::Get_ocStbHostSPDIfAudioFormat ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostSPDIfAudioFormat" << endl;
  return 0;
}


/* 
ocStbHostSPDIfAudioMuteStatus OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The value 'true' indicates that the audio signal is muted
             over the SPDIF link. 'false' means the audio is not muted."
     ::= { ocStbHostSPDIfEntry 2 }
*/ 
bool ocStbHostSPDIfEntryBridge::Get_ocStbHostSPDIfAudioMuteStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostSPDIfAudioMuteStatus" << endl;
  return true;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostIEEE1394EntryBridge for bridging elements in the ocStbHostIEEE1394Table
 *
\*/

/* 
ocStbHostIEEE1394Table OBJECT-TYPE
    SYNTAX      SEQUENCE OF OcStbHostIEEE1394Entry
    MAX-ACCESS not-accessible
    STATUS      current
    DESCRIPTION
            "This table contains attributes associated with the
            ocStbHost Interfaces of type IEEE 1394."
    ::= { ocStbHostIEEE1394Objects 1 }

INDEX { ocStbHostAVInterfaceIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostIEEE1394EntryBridge::ocStbHostIEEE1394EntryBridge (unsigned long ocStbHostAVInterfaceIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ocStbHostIEEE1394Table,
                      BcmObjectId(ocStbHostAVInterfaceIndex),
                      pAgent)
{
}

ocStbHostIEEE1394EntryBridge::~ocStbHostIEEE1394EntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostIEEE1394ActiveNodes OBJECT-TYPE
    SYNTAX      Integer32 (-1 | 0..64)
     UNITS       "nodes"
     MAX-ACCESS read-only
     STATUS      current
     DESCRIPTION
             "This object allows the manager to retrieve the
             number of nodes connected to the 1394 bus of
             the ocStbHost.
             The value '-1' means unknown number of nodes."
     ::= { ocStbHostIEEE1394Entry 1 }
*/ 
int ocStbHostIEEE1394EntryBridge::Get_ocStbHostIEEE1394ActiveNodes ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostIEEE1394ActiveNodes" << endl;
  return 0;
}


/* 
ocStbHostIEEE1394DataXMission OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            state of data being transmitted over the 1394
            bus. True indicates that data is being transmitted
            over the 1394 bus."
    ::= { ocStbHostIEEE1394Entry 2 }
*/ 
bool ocStbHostIEEE1394EntryBridge::Get_ocStbHostIEEE1394DataXMission ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostIEEE1394DataXMission" << endl;
  return true;
}


/* 
ocStbHostIEEE1394DTCPStatus OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            state of DTCP on the device. True indicates
            that DTCP is enabled and content is being encrypted
            of the 1394 link on the ocStbHost."
    ::= { ocStbHostIEEE1394Entry 3 }
*/ 
bool ocStbHostIEEE1394EntryBridge::Get_ocStbHostIEEE1394DTCPStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostIEEE1394DTCPStatus" << endl;
  return true;
}


/* 
ocStbHostIEEE1394LoopStatus OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            state of a Loop present on the 1394 bus. True
            indicates that a Loop is present on the 1394 bus."
    ::= { ocStbHostIEEE1394Entry 4 }
*/ 
bool ocStbHostIEEE1394EntryBridge::Get_ocStbHostIEEE1394LoopStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostIEEE1394LoopStatus" << endl;
  return true;
}


/* 
ocStbHostIEEE1394RootStatus OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            state of Root Status on the device. True indicates
            that the set-top is the Root node on the 1394 bus."
    ::= { ocStbHostIEEE1394Entry 5 }
*/ 
bool ocStbHostIEEE1394EntryBridge::Get_ocStbHostIEEE1394RootStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostIEEE1394RootStatus" << endl;
  return true;
}


/* 
ocStbHostIEEE1394CycleIsMaster OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            state of Cycle Master on the device. True indicates
            that the set-top is Cycle Master on the 1394 bus."
    ::= { ocStbHostIEEE1394Entry 6 }
*/ 
bool ocStbHostIEEE1394EntryBridge::Get_ocStbHostIEEE1394CycleIsMaster ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostIEEE1394CycleIsMaster" << endl;
  return true;
}


/* 
ocStbHostIEEE1394IRMStatus OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            state of Isochronous Resource Master on the
            device. True indicates that the set-top is IRM
            on the 1394 bus."
    ::= { ocStbHostIEEE1394Entry 7 }
*/ 
bool ocStbHostIEEE1394EntryBridge::Get_ocStbHostIEEE1394IRMStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostIEEE1394IRMStatus" << endl;
  return true;
}


/* 
ocStbHostIEEE1394AudioMuteStatus OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The value 'true' indicates that the audio signal
            is muted for this Interface. 'false' means the audio
            is not muted."
    ::= { ocStbHostIEEE1394Entry 8 }
*/ 
bool ocStbHostIEEE1394EntryBridge::Get_ocStbHostIEEE1394AudioMuteStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostIEEE1394AudioMuteStatus" << endl;
  return true;
}


/* 
ocStbHostIEEE1394VideoMuteStatus OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The value 'true' indicates that the video signal
            is muted for this Interface. 'false' means the video
            signal is not muted."
    ::= { ocStbHostIEEE1394Entry 9 }
*/ 
bool ocStbHostIEEE1394EntryBridge::Get_ocStbHostIEEE1394VideoMuteStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostIEEE1394VideoMuteStatus" << endl;
  return true;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostIEEE1394ConnectedDevicesEntryBridge for bridging elements in the ocStbHostIEEE1394ConnectedDevicesTable
 *
\*/

/* 
ocStbHostIEEE1394ConnectedDevicesTable OBJECT-TYPE
    SYNTAX      SEQUENCE OF OcStbHostIEEE1394ConnectedDevicesEntry
    MAX-ACCESS not-accessible
    STATUS      current
    DESCRIPTION
            "This table contains A/D Source Selection attributes
            associated to devices connected to the ocStbHost
            Interfaces of type IEEE 1394."
    ::= { ocStbHostIEEE1394Objects 2 }

INDEX { ocStbHostIEEE1394ConnectedDevicesIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostIEEE1394ConnectedDevicesEntryBridge::ocStbHostIEEE1394ConnectedDevicesEntryBridge (unsigned long ocStbHostIEEE1394ConnectedDevicesIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ocStbHostIEEE1394ConnectedDevicesTable,
                      BcmObjectId(ocStbHostIEEE1394ConnectedDevicesIndex),
                      pAgent)
{
}

ocStbHostIEEE1394ConnectedDevicesEntryBridge::~ocStbHostIEEE1394ConnectedDevicesEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostIEEE1394ConnectedDevicesIndex OBJECT-TYPE
    SYNTAX      Unsigned32(1..4294967295)
    MAX-ACCESS not-accessible
    STATUS      current
    DESCRIPTION
            "The unique identifier of the connected device entry."
     ::= { ocStbHostIEEE1394ConnectedDevicesEntry 1 }
*/ 
unsigned int ocStbHostIEEE1394ConnectedDevicesEntryBridge::Get_ocStbHostIEEE1394ConnectedDevicesIndex ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostIEEE1394ConnectedDevicesIndex" << endl;
  return 0;
}


/* 
ocStbHostIEEE1394ConnectedDevicesAVInterfaceIndex OBJECT-TYPE
    SYNTAX      Unsigned32(1..4294967295)
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            index value associated with the IEEE1394
            instance of ocStbHostAVInterfaceIndex
            to which the device is physically connected."
    ::= { ocStbHostIEEE1394ConnectedDevicesEntry 2 }
*/ 
unsigned int ocStbHostIEEE1394ConnectedDevicesEntryBridge::Get_ocStbHostIEEE1394ConnectedDevicesAVInterfaceIndex ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostIEEE1394ConnectedDevicesAVInterfaceIndex" << endl;
  return 0;
}


/* 
ocStbHostIEEE1394ConnectedDevicesSubUnitType OBJECT-TYPE
    SYNTAX      INTEGER {
                monitor(0),
                audio(1),
                printer(2),
                disc(3),
                tape(4),
                tuner(5),
                ca(6),
                camera(7),
                reserved(8),
                panel(9),
                other(10)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            Subunit Type associated with the device connected
            to the ocStbHost's IEEE1394 interface."
    ::= { ocStbHostIEEE1394ConnectedDevicesEntry 3 }
*/ 
int ocStbHostIEEE1394ConnectedDevicesEntryBridge::Get_ocStbHostIEEE1394ConnectedDevicesSubUnitType ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostIEEE1394ConnectedDevicesSubUnitType" << endl;
  return 0;
}


/* 
ocStbHostIEEE1394ConnectedDevicesEui64 OBJECT-TYPE
    SYNTAX      OCTET STRING (SIZE (8))
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            64-bit Extended Unique Identifier (EUI-64)
            associated with the device connected
            to the ocStbHost's IEEE1394 interface."
    ::= { ocStbHostIEEE1394ConnectedDevicesEntry 4 }
*/ 
int ocStbHostIEEE1394ConnectedDevicesEntryBridge::Get_ocStbHostIEEE1394ConnectedDevicesEui64 (BcmString &ocStbHostIEEE1394ConnectedDevicesEui64)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostIEEE1394ConnectedDevicesEui64" << endl;
  ocStbHostIEEE1394ConnectedDevicesEui64 = "(unfinished)";

  return ocStbHostIEEE1394ConnectedDevicesEui64.length();
}


/* 
ocStbHostIEEE1394ConnectedDevicesADSourceSelectSupport OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            status of Analog/Digital Source Selection support
            associated with the device connected to the
            ocStbHost's IEEE1394 interface.
            'true' indicates that the device supports A/D
            source selection."
    ::= { ocStbHostIEEE1394ConnectedDevicesEntry 5 }
*/ 
bool ocStbHostIEEE1394ConnectedDevicesEntryBridge::Get_ocStbHostIEEE1394ConnectedDevicesADSourceSelectSupport ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostIEEE1394ConnectedDevicesADSourceSelectSupport" << endl;
  return true;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostDVIHDMIEntryBridge for bridging elements in the ocStbHostDVIHDMITable
 *
\*/

/* 
ocStbHostDVIHDMITable OBJECT-TYPE
    SYNTAX      SEQUENCE OF OcStbHostDVIHDMIEntry
    MAX-ACCESS not-accessible
    STATUS      current
    DESCRIPTION
            "This table allows the manager to retrieve information
            about the DVI/HDMI port of the ocStbHost."
    ::= { ocStbHostDigitalVideoOutput 1 }

INDEX { ocStbHostAVInterfaceIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostDVIHDMIEntryBridge::ocStbHostDVIHDMIEntryBridge (unsigned long ocStbHostAVInterfaceIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ocStbHostDVIHDMITable,
                      BcmObjectId(ocStbHostAVInterfaceIndex),
                      pAgent)
{
}

ocStbHostDVIHDMIEntryBridge::~ocStbHostDVIHDMIEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostDVIHDMIOutputType OBJECT-TYPE
    SYNTAX      INTEGER
                {
                dvi(1),
                hdmi(2)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object defines the operational mode of the
            interface, where valid modes are dependent on
            the physical interface defined.

            If ocStbHostAVInterfaceIndex is DVI, then
            this value is always set to 1 (DVI).

             If ocStbHostAVInterfaceIndex is HDMI, then
             this value is either 1 (DVI) or 2 (HDMI)."
     ::= { ocStbHostDVIHDMIEntry 2 }
*/ 
int ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMIOutputType ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMIOutputType" << endl;
  return 0;
}


/* 
ocStbHostDVIHDMIConnectionStatus OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            state of the DVI/HDMI port on the device.
            'true' indicates that a device is connected to
            the ocStbHost."
    ::= { ocStbHostDVIHDMIEntry 3 }
*/ 
bool ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMIConnectionStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMIConnectionStatus" << endl;
  return true;
}


/* 
ocStbHostDVIHDMIRepeaterStatus OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            presence of a repeater connected to the ocStbHost.
            'true' indicates that a repeater is connected
            to the ocStbHost."
    ::= { ocStbHostDVIHDMIEntry 4 }
*/ 
bool ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMIRepeaterStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMIRepeaterStatus" << endl;
  return true;
}


/* 
ocStbHostDVIHDMIVideoXmissionStatus OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            state of video transmission over the DVI/HDMI port
            of the ocStbHost. 'true' indicates that video is being
            transmitted over the DVI/HDMI link to a connected
            device."
    ::= { ocStbHostDVIHDMIEntry 5 }
*/ 
bool ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMIVideoXmissionStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMIVideoXmissionStatus" << endl;
  return true;
}


/* 
ocStbHostDVIHDMIHDCPStatus OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            state of the digital copy protection protocol in the
            interface.
            'true' indicates that HDCP encryption is enabled."
    ::= { ocStbHostDVIHDMIEntry 6 }
*/ 
bool ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMIHDCPStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMIHDCPStatus" << endl;
  return true;
}


/* 
ocStbHostDVIHDMIVideoMuteStatus OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The value 'true' indicates that the video signal
            is being muted over the DVI/HDMI link. 'false' means
            the video signal is not muted."
    ::= { ocStbHostDVIHDMIEntry 7 }
*/ 
bool ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMIVideoMuteStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMIVideoMuteStatus" << endl;
  return true;
}


/* 
ocStbHostDVIHDMIOutputFormat OBJECT-TYPE
    SYNTAX      VideoOutputFormat
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            video timing format currently being transmitted
            over the DVI/HDMI link."
    ::= { ocStbHostDVIHDMIEntry 8 }
*/ 
int ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMIOutputFormat ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMIOutputFormat" << endl;
  return 0;
}


/* 
ocStbHostDVIHDMIAspectRatio OBJECT-TYPE
    SYNTAX      VideoAspectRatio
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            aspect ratio applicable to the video timing
            format currently being transmitted over the
            DVI/HDMI link. e.g., 4/3 or 16/9."
    ::= { ocStbHostDVIHDMIEntry 9 }
*/ 
int ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMIAspectRatio ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMIAspectRatio" << endl;
  return 0;
}


/* 
ocStbHostDVIHDMIHostDeviceHDCPStatus OBJECT-TYPE
    SYNTAX      INTEGER
                {
                nonHDCPdevice(1),
                compliantHDCPdevice(2),
                revokedHDCPdevice(3)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Connected device HDCP status."
    ::= { ocStbHostDVIHDMIEntry 10 }
*/ 
int ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMIHostDeviceHDCPStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMIHostDeviceHDCPStatus" << endl;
  return 0;
}


/* 
ocStbHostDVIHDMIAudioFormat OBJECT-TYPE
    SYNTAX      AudioOutputFormat
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            audio format being transmitted over the
            HDMI link. If this entry corresponds to a DVI interface,
            this object is not instantiated."
    ::= { ocStbHostDVIHDMIEntry 11 }
*/ 
int ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMIAudioFormat ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMIAudioFormat" << endl;
  return 0;
}


/* 
ocStbHostDVIHDMIAudioSampleRate OBJECT-TYPE
    SYNTAX      INTEGER
                {
                other(1),
                samplerate32kHz(2),
                samplerate44kHz(3),
                samplerate48kHz(4),
                samplerate88kHz(5),
                samplerate96kHz(6),
                samplerate176kHz(7),
                samplerate192kHz(8)
                }
     MAX-ACCESS read-only
     STATUS      current
     DESCRIPTION
             "This object allows the manager to retrieve the
             audio sample rate being transmitted over the
             HDMI link. If this entry corresponds to a DVI interface,
             this object is not instantiated."
     ::= { ocStbHostDVIHDMIEntry 12 }
*/ 
int ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMIAudioSampleRate ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMIAudioSampleRate" << endl;
  return 0;
}


/* 
ocStbHostDVIHDMIAudioChannelCount OBJECT-TYPE
    SYNTAX      Unsigned32 (0..10)
    UNITS       "channels"
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            audio channel count being transmitted over the
            HDMI link. If this entry corresponds to a DVI interface,
            this object is not instantiated."
    ::= { ocStbHostDVIHDMIEntry 13 }
*/ 
unsigned int ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMIAudioChannelCount ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMIAudioChannelCount" << endl;
  return 0;
}


/* 
ocStbHostDVIHDMIAudioMuteStatus OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The value 'true' indicates that the audio signal is muted
            over the DVI/HDMI link. 'false' means the audio is not
            muted. This object is not instantiated for entries of
            ocStbHostDVIHDMIOutputType 'dvi'."
    ::= { ocStbHostDVIHDMIEntry 14 }
*/ 
bool ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMIAudioMuteStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMIAudioMuteStatus" << endl;
  return true;
}


/* 
ocStbHostDVIHDMIAudioSampleSize OBJECT-TYPE
    SYNTAX      INTEGER
                {
                notValid(0),
                sample16Bit(1),
                sample20Bit(2),
                sample24Bit(3)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Non-zero values pertain only to PCM audio formats and
            indicate the PCM sample size. This object applies only
            to HDMI outputs."
    ::= { ocStbHostDVIHDMIEntry 15 }
*/ 
int ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMIAudioSampleSize ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMIAudioSampleSize" << endl;
  return 0;
}


/* 
ocStbHostDVIHDMIColorSpace OBJECT-TYPE
    SYNTAX      INTEGER
                {
                rgb(0),
                ycc422(1),
                ycc444(2)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Color space for HDMI outputs only."
    ::= { ocStbHostDVIHDMIEntry 16 }
*/ 
int ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMIColorSpace ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMIColorSpace" << endl;
  return 0;
}


/* 
ocStbHostDVIHDMIFrameRate OBJECT-TYPE
    SYNTAX      INTEGER
                {
                frameRateCode1(1),
                frameRateCode2(2),
                frameRateCode3(3),
                  frameRateCode4(4),
                  frameRateCode5(5),
                  frameRateCode6(6)
                  }
    MAX-ACCESS    read-only
    STATUS        current
    DESCRIPTION
            "Frame rates for HDMI outputs as defined in CCIF spec:
                frameRateCode1 = 23.976 Hz
                frameRateCode2 = 24.0 Hz
                frameRateCode3 = 29.97 Hz
                frameRateCode4 = 30.0 Hz
                frameRateCode5 = 59.94 Hz
                frameRateCode6 = 60.0 Hz."
    ::= { ocStbHostDVIHDMIEntry 17 }
*/ 
int ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMIFrameRate ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMIFrameRate" << endl;
  return 0;
}


/* 
ocStbHostDVIHDMIAttachedDeviceType OBJECT-TYPE
    SYNTAX      INTEGER
                {
                tv(0),
                recordingDevice(1),
                tuner(3),
                playbackDevice(4),
                audioSystem(5),
                other(6)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The HDMI defined types of devices derived defined
            in the DHMI specification version 1.3a."
    ::= { ocStbHostDVIHDMIEntry 18 }
*/ 
int ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMIAttachedDeviceType ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMIAttachedDeviceType" << endl;
  return 0;
}


/* 
ocStbHostDVIHDMIEdid OBJECT-TYPE
    SYNTAX      OCTET STRING
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "EDID blob for debugging. A 0x00
            should be returned if an EDID is
            not available."
    ::= { ocStbHostDVIHDMIEntry 19 }
*/ 
int ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMIEdid (BcmString &ocStbHostDVIHDMIEdid)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMIEdid" << endl;
  ocStbHostDVIHDMIEdid = "(unfinished)";

  return ocStbHostDVIHDMIEdid.length();
}


/* 
ocStbHostDVIHDMILipSyncDelay OBJECT-TYPE
    SYNTAX Integer32
    MAX-ACCESS read-only
    STATUS current
    DESCRIPTION
            "Defines the amount of delay in milliseconds that is
            being applied to the audio delivered over the HDMI
            link. A value of zero indicates that either no delay
            is being applied or Lip Sync is not supported by the
            device."
    ::= { ocStbHostDVIHDMIEntry 20 }
*/ 
int ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMILipSyncDelay ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMILipSyncDelay" << endl;
  return 0;
}


/* 
ocStbHostDVIHDMICecFeatures OBJECT-TYPE
    SYNTAX      BITS
                {
                oneTouchPlay(0),
                systemStandby(1),
                oneTouchRecord(2),
                timerProgramming(3),
                deckControl(4),
                tunerControl(5),
                deviceMenuControl(6),
                remoteControlPassThrough(7),
                systemAudioControl(8),
                deviceOsdNameTransfer(9),
                devicePowerStatus(10),
                osdDisplay(11),
                routingControl(12),
                systemInformation(13),
                vendorSpecificCommands(14),
                audioRateControl(15)
                }
     MAX-ACCESS read-only
     STATUS     current
     DESCRIPTION
             "This object lists all of the CEC Features supported by the
             device. When the bit is set, the feature is supported."
     ::= { ocStbHostDVIHDMIEntry 21 }
*/ 
unsigned long ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMICecFeatures ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMICecFeatures" << endl;
  return 0;
}


/* 
ocStbHostDVIHDMIFeatures OBJECT-TYPE
    SYNTAX      BITS
                {
                deepColor(0),
                extendedColorGamut(1),
                oneBitAudio(2),
                lipSync(3),
                cec(4)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object lists all of the HDMI Features supported by
            the device. When the bit is set, the feature is supported."
    ::= { ocStbHostDVIHDMIEntry 22 }
*/ 
unsigned long ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMIFeatures ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMIFeatures" << endl;
  return 0;
}


/* 
ocStbHostDVIHDMIMaxDeviceCount OBJECT-TYPE
    SYNTAX      Integer32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Defines the maximum number of downstream devices
            that this device can support."
    ::= { ocStbHostDVIHDMIEntry 23 }
*/ 
int ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMIMaxDeviceCount ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMIMaxDeviceCount" << endl;
  return 0;
}


/* 
ocStbHostDVIHDMIPreferredVideoFormat OBJECT-TYPE
    SYNTAX      INTEGER (0..127)
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            preferred video format of the sink device. The
            value is an integer representing the video timing
            format expressed as a CEA-861 Video Identification
            Code (VIC).

             If the Host is not able to determine the preferred
             video format, or there is no corresponding VIC,
             then the Host sets this value to zero. The Host also
             sets this value to zero if there is no EDID or
             active HDMI/DVI connection in place."
     ::= { ocStbHostDVIHDMIEntry 24 }
*/ 
int ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMIPreferredVideoFormat ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMIPreferredVideoFormat" << endl;
  return 0;
}


/* 
ocStbHostDVIHDMIEdidVersion OBJECT-TYPE
    SYNTAX      SnmpAdminString
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "EDID version reported in bytes 18 and 19 of the EDID
            itself. It is in version.release format. A value of 0.0
            should be returned if an EDID is not available."
    ::= { ocStbHostDVIHDMIEntry 25 }
*/ 
int ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMIEdidVersion (BcmString &ocStbHostDVIHDMIEdidVersion)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMIEdidVersion" << endl;
  ocStbHostDVIHDMIEdidVersion = "(unfinished)";

  return ocStbHostDVIHDMIEdidVersion.length();
}


/* 
ocStbHostDVIHDMI3DCompatibilityControl OBJECT-TYPE
    SYNTAX      INTEGER
                {
                other(0),
                passthru3D(1),
                block3D(2)
                }
    MAX-ACCESS read-write
    STATUS      current
    DESCRIPTION
            "Defines what action to take for a display device that
            has not reported support for the received S3D content.
            passthru3D(1) requires the content to be passed
            onto the display device. blocked3D(2) requires the STB
            to not pass the content to the display device."
    DEFVAL { 1 }
    ::= { ocStbHostDVIHDMIEntry 26 }
*/ 
int ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMI3DCompatibilityControl ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMI3DCompatibilityControl" << endl;
  return 0;
}


/* 
ocStbHostDVIHDMI3DCompatibilityMsgDisplay OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-write
    STATUS      current
    DESCRIPTION
            "True(1) requires the STB to generate and transmit a warning
            message when the following occurs: 1) the STB receives
            S3D content and 2) the display device has not reported support
            for that format. The message should indicate that an
            incompatibility may exist between the source format and the
            display device capabilities. The STB must not send a warning
            message in this scenario if this object is set to false(2)."
    DEFVAL { 1 }
    ::= { ocStbHostDVIHDMIEntry 27 }
*/ 
bool ocStbHostDVIHDMIEntryBridge::Get_ocStbHostDVIHDMI3DCompatibilityMsgDisplay ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMI3DCompatibilityMsgDisplay" << endl;
  return true;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/

/* 
ocStbHostDVIHDMI3DCompatibilityControl OBJECT-TYPE
    SYNTAX      INTEGER
                {
                other(0),
                passthru3D(1),
                block3D(2)
                }
    MAX-ACCESS read-write
    STATUS      current
    DESCRIPTION
            "Defines what action to take for a display device that
            has not reported support for the received S3D content.
            passthru3D(1) requires the content to be passed
            onto the display device. blocked3D(2) requires the STB
            to not pass the content to the display device."
    DEFVAL { 1 }
    ::= { ocStbHostDVIHDMIEntry 26 }
*/ 
SNMP_STATUS ocStbHostDVIHDMIEntryBridge::Set_ocStbHostDVIHDMI3DCompatibilityControl (int ocStbHostDVIHDMI3DCompatibilityControl)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (ocStbHostDVIHDMI3DCompatibilityControl);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ocStbHostDVIHDMI3DCompatibilityControl != CONSTVAL_OCSTBHOSTDVIHDMI3DCOMPATIBILITYCONTROL_OTHER)
    &&  (ocStbHostDVIHDMI3DCompatibilityControl != CONSTVAL_OCSTBHOSTDVIHDMI3DCOMPATIBILITYCONTROL_PASSTHRU3D)
    &&  (ocStbHostDVIHDMI3DCompatibilityControl != CONSTVAL_OCSTBHOSTDVIHDMI3DCOMPATIBILITYCONTROL_BLOCK3D))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ocStbHostDVIHDMI3DCompatibilityControl" << endl;
  }

  return Status;
}


/* 
ocStbHostDVIHDMI3DCompatibilityMsgDisplay OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-write
    STATUS      current
    DESCRIPTION
            "True(1) requires the STB to generate and transmit a warning
            message when the following occurs: 1) the STB receives
            S3D content and 2) the display device has not reported support
            for that format. The message should indicate that an
            incompatibility may exist between the source format and the
            display device capabilities. The STB must not send a warning
            message in this scenario if this object is set to false(2)."
    DEFVAL { 1 }
    ::= { ocStbHostDVIHDMIEntry 27 }
*/ 
SNMP_STATUS ocStbHostDVIHDMIEntryBridge::Set_ocStbHostDVIHDMI3DCompatibilityMsgDisplay (bool ocStbHostDVIHDMI3DCompatibilityMsgDisplay)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;

  {
    if ((ocStbHostDVIHDMI3DCompatibilityMsgDisplay != true) && (ocStbHostDVIHDMI3DCompatibilityMsgDisplay != false))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ocStbHostDVIHDMI3DCompatibilityMsgDisplay" << endl;
  }

  return Status;
}






/*\
 *
 *  ocStbHostDVIHDMIAvailableVideoFormatEntryBridge for bridging elements in the ocStbHostDVIHDMIAvailableVideoFormatTable
 *
\*/

/* 
ocStbHostDVIHDMIAvailableVideoFormatTable OBJECT-TYPE
    SYNTAX      SEQUENCE OF OcStbHostDVIHDMIAvailableVideoFormatEntry
    MAX-ACCESS not-accessible
    STATUS      current
    DESCRIPTION
            "This table allows the manager to retrieve a list
            of all of the video timing formats reported by the
            sink device as per the device's EDID."
    ::= { ocStbHostDigitalVideoOutput 2 }

INDEX { ocStbHostDVIHDMIAvailableVideoFormatIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostDVIHDMIAvailableVideoFormatEntryBridge::ocStbHostDVIHDMIAvailableVideoFormatEntryBridge (int ocStbHostDVIHDMIAvailableVideoFormatIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ocStbHostDVIHDMIAvailableVideoFormatTable,
                      BcmObjectId(ocStbHostDVIHDMIAvailableVideoFormatIndex),
                      pAgent)
{
}

ocStbHostDVIHDMIAvailableVideoFormatEntryBridge::~ocStbHostDVIHDMIAvailableVideoFormatEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostDVIHDMIAvailableVideoFormatIndex OBJECT-TYPE
    SYNTAX      INTEGER (0..127)
     MAX-ACCESS not-accessible
     STATUS      current
     DESCRIPTION
             "Index in the ocStbHostDVIHDMIAvailableVideoFormatTable."
     ::= { ocStbHostDVIHDMIAvailableVideoFormatEntry 1 }
*/ 
int ocStbHostDVIHDMIAvailableVideoFormatEntryBridge::Get_ocStbHostDVIHDMIAvailableVideoFormatIndex ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMIAvailableVideoFormatIndex" << endl;
  return 0;
}


/* 
ocStbHostDVIHDMIAvailableVideoFormat OBJECT-TYPE
    SYNTAX      INTEGER (0..127)
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            available video formats of the sink device. The
            value is an integer representing the video timing
            format expressed as a CEA-861 Video Identification
            Code (VIC).

             If the Host is not able to determine the available
             video formats, or there is no corresponding VIC,
             then the Host makes an entry with a value of zero.
             The Host also makes an entry with a value of zero
             if there is no active HDMI/DVI connection in place."
     ::= { ocStbHostDVIHDMIAvailableVideoFormatEntry 2 }
*/ 
int ocStbHostDVIHDMIAvailableVideoFormatEntryBridge::Get_ocStbHostDVIHDMIAvailableVideoFormat ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMIAvailableVideoFormat" << endl;
  return 0;
}


/* 
ocStbHostDVIHDMISupported3DStructures OBJECT-TYPE
    SYNTAX      BITS
                {
                framePacking(0),
                fieldAlternative(1),
                lineAlternative(2),
                sideBySideFull(3),
                leftPlusDepth(4),
                leftPlusDepthPlusGraphicsPlusGraphicsDepth(5),
                topAndBottom(6),
                sideBySideHalf(7),
                sideBySideHalfQuincunx(8)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object reports the 3D structures supported by
            the sink as reported in the EDID VSDB. The supported
             structures are specified in [HDMI] Appendix H."
    ::= { ocStbHostDVIHDMIAvailableVideoFormatEntry 3 }
*/ 
unsigned long ocStbHostDVIHDMIAvailableVideoFormatEntryBridge::Get_ocStbHostDVIHDMISupported3DStructures ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMISupported3DStructures" << endl;
  return 0;
}


/* 
ocStbHostDVIHDMIActive3DStructure OBJECT-TYPE
    SYNTAX      INTEGER {
                other(0),
                na(1),
                framePacking(2),
                fieldAlternative(3),
                lineAlternative(4),
                sideBySideFull(5),
                leftPlusDepth(6),
                leftPlusDepthPlusGraphicsPlusGraphicsDepth(7),
                topAndBottom(8),
                sideBySideHalf(9),
                sideBySideHalfQuincunx(10),
                noAdditionalHDMIInfo(17),
                no3DInformation(18),
                infoFrameNotAvailable(19)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            contents of the HDMI Vendor Specific InfoFrame currently
             being transmitted by the source. The video signal
             corresponds to the row associated with sink's current
             'active' VIC. Only one row can reflect an active 3D format
             at any point in time. The remaining, 'inactive', rows in
             this table should report 'na(1)' for not applicable.

             This object must be updated whenever the current
             HDMI InfoFrame content differs from the last HDMI InfoFrame
             transmitted. All reported values are based on the
             HDMI InfoFrame packet contents defined in [HDMI].

             If 3D is currently being signaled, report the 3D structure
             identified in the InfoFrame.

             If an HDMI InfoFrame is being transmitted but no 3D structure
             is being signaled, use the HDMI_Video_format values in the
             HDMI Infoframe to report as follows:

             000 - 'noAdditionalHDMIInfo(17)'
             001 - 'no3DInformation(18)'

             If an HDMI InfoFrame is not being transmitted, report
             'infoFrameNotAvailable(19)'."

    ::= { ocStbHostDVIHDMIAvailableVideoFormatEntry 4 }
*/ 
int ocStbHostDVIHDMIAvailableVideoFormatEntryBridge::Get_ocStbHostDVIHDMIActive3DStructure ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDVIHDMIActive3DStructure" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostComponentVideoEntryBridge for bridging elements in the ocStbHostComponentVideoTable
 *
\*/

/* 
ocStbHostComponentVideoTable OBJECT-TYPE
    SYNTAX      SEQUENCE OF OcStbHostComponentVideoEntry
    MAX-ACCESS not-accessible
    STATUS      current
    DESCRIPTION
            "This table allows the manager to retrieve information
            about a Component Video interface of the ocStbHost."
    ::= { ocStbHostAnalogOutput 1 }

INDEX { ocStbHostAVInterfaceIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostComponentVideoEntryBridge::ocStbHostComponentVideoEntryBridge (unsigned long ocStbHostAVInterfaceIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ocStbHostComponentVideoTable,
                      BcmObjectId(ocStbHostAVInterfaceIndex),
                      pAgent)
{
}

ocStbHostComponentVideoEntryBridge::~ocStbHostComponentVideoEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostComponentVideoConstrainedStatus OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            state of Constrained Video Image on the port.
            True indicates that video is being constrained
             over the component video output port."
     ::= { ocStbHostComponentVideoEntry 1 }
*/ 
bool ocStbHostComponentVideoEntryBridge::Get_ocStbHostComponentVideoConstrainedStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostComponentVideoConstrainedStatus" << endl;
  return true;
}


/* 
ocStbHostComponentOutputFormat OBJECT-TYPE
    SYNTAX      VideoOutputFormat
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            video format currently being transmitted over
            the Component Video output port."
    ::= { ocStbHostComponentVideoEntry 2 }
*/ 
int ocStbHostComponentVideoEntryBridge::Get_ocStbHostComponentOutputFormat ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostComponentOutputFormat" << endl;
  return 0;
}


/* 
ocStbHostComponentAspectRatio OBJECT-TYPE
    SYNTAX      VideoAspectRatio
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            video aspect ratio currently being transmitted over
            the Component Video output port."
    ::= { ocStbHostComponentVideoEntry 3 }
*/ 
int ocStbHostComponentVideoEntryBridge::Get_ocStbHostComponentAspectRatio ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostComponentAspectRatio" << endl;
  return 0;
}


/* 
ocStbHostComponentVideoMuteStatus OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The value 'true' indicates that the video signal is
            muted for this Interface. 'false' means the video
            signal is not muted."
    ::= { ocStbHostComponentVideoEntry 4 }
*/ 
bool ocStbHostComponentVideoEntryBridge::Get_ocStbHostComponentVideoMuteStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostComponentVideoMuteStatus" << endl;
  return true;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostRFChannelOutEntryBridge for bridging elements in the ocStbHostRFChannelOutTable
 *
\*/

/* 
ocStbHostRFChannelOutTable OBJECT-TYPE
    SYNTAX      SEQUENCE OF OcStbHostRFChannelOutEntry
    MAX-ACCESS not-accessible
    STATUS      current
    DESCRIPTION
            "This table allows the manager to retrieve information
            about an RF channel output interface of the ocStbHost."
    ::= { ocStbHostAnalogOutput 2 }

INDEX { ocStbHostAVInterfaceIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostRFChannelOutEntryBridge::ocStbHostRFChannelOutEntryBridge (unsigned long ocStbHostAVInterfaceIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ocStbHostRFChannelOutTable,
                      BcmObjectId(ocStbHostAVInterfaceIndex),
                      pAgent)
{
}

ocStbHostRFChannelOutEntryBridge::~ocStbHostRFChannelOutEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostRFChannelOut OBJECT-TYPE
    SYNTAX      Unsigned32 (3..99)
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Indicates the RF channel out value. Typical values are
            3 or 4 for Channels 3 and 4, respectively."
    ::= { ocStbHostRFChannelOutEntry 2 }
*/ 
unsigned int ocStbHostRFChannelOutEntryBridge::Get_ocStbHostRFChannelOut ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostRFChannelOut" << endl;
  return 0;
}


/* 
ocStbHostRFChannelOutAudioMuteStatus OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The value 'true' indicates that the audio signal is
            muted for this Interface. 'false' means the audio
            is not muted."
    ::= { ocStbHostRFChannelOutEntry 3 }
*/ 
bool ocStbHostRFChannelOutEntryBridge::Get_ocStbHostRFChannelOutAudioMuteStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostRFChannelOutAudioMuteStatus" << endl;
  return true;
}


/* 
ocStbHostRFChannelOutVideoMuteStatus OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The value 'true' indicates that the video signal is
            muted for this Interface. 'false' means the video
            signal is not muted."
    ::= { ocStbHostRFChannelOutEntry 4 }
*/ 
bool ocStbHostRFChannelOutEntryBridge::Get_ocStbHostRFChannelOutVideoMuteStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostRFChannelOutVideoMuteStatus" << endl;
  return true;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostInBandTunerEntryBridge for bridging elements in the ocStbHostInBandTunerTable
 *
\*/

/* 
ocStbHostInBandTunerTable OBJECT-TYPE
    SYNTAX      SEQUENCE OF OcStbHostInBandTunerEntry
    MAX-ACCESS not-accessible
    STATUS      current
    DESCRIPTION
            "This Object Table allows the manager to retrieve
            information regarding the In-Band tuner(s) in
            the ocStbHost."
    ::= { ocStbHostServiceProgramInfo 1 }

INDEX { ocStbHostAVInterfaceIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostInBandTunerEntryBridge::ocStbHostInBandTunerEntryBridge (unsigned long ocStbHostAVInterfaceIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ocStbHostInBandTunerTable,
                      BcmObjectId(ocStbHostAVInterfaceIndex),
                      pAgent)
{
}

ocStbHostInBandTunerEntryBridge::~ocStbHostInBandTunerEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostInBandTunerModulationMode OBJECT-TYPE
    SYNTAX      INTEGER
                {
                other(1),
                analog(2),
                qam64(3),
                qam256(4)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The In-band tuner modulation mode.

            Modulation modes are as follows:
            other    - IB is configured in a manner not described
                       by any other listed mode.
            Analog   - IB is configured for analog service.
            QAM-64   - IB is configured for a digital service that
                       is 64-QAM.
            QAM-256 - IB is configured for a digital service that
                       is 256-QAM."
    ::= { ocStbHostInBandTunerEntry 1 }
*/ 
int ocStbHostInBandTunerEntryBridge::Get_ocStbHostInBandTunerModulationMode ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostInBandTunerModulationMode" << endl;
  return 0;
}


/* 
ocStbHostInBandTunerFrequency OBJECT-TYPE
    SYNTAX      Unsigned32
    UNITS       "hertz"
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the center
            frequency of the currently tuned channel in Hz."
    ::= { ocStbHostInBandTunerEntry 2 }
*/ 
unsigned int ocStbHostInBandTunerEntryBridge::Get_ocStbHostInBandTunerFrequency ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostInBandTunerFrequency" << endl;
  return 0;
}


/* 
ocStbHostInBandTunerInterleaver OBJECT-TYPE
    SYNTAX      INTEGER {
                unknown(1),
                other(2),
                taps64Increment2(3),
                taps128Increment1(4),
                taps128increment2(5),
                taps128increment3(6),
                taps128increment4(7),
                taps32increment4(8),
                taps16increment8(9),
                taps8increment16(10)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The in-band interface interleaver value.
               set to '3' current operational mode 'taps64Increment2'
               set to '4' current operational mode 'taps128Increment1'
               set to '5' current operational mode 'taps128increment2'
               set to '6' current operational mode 'taps128increment3'
               set to '7' current operational mode 'taps128increment4'
               set to '8' current operational mode 'taps32increment4'
               set to '9' current operational mode 'taps16increment8'
               set to '10' current operational mode 'taps8increment16'"
    ::= { ocStbHostInBandTunerEntry 3 }
*/ 
int ocStbHostInBandTunerEntryBridge::Get_ocStbHostInBandTunerInterleaver ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostInBandTunerInterleaver" << endl;
  return 0;
}


/* 
ocStbHostInBandTunerPower OBJECT-TYPE
    SYNTAX      TenthdBmV
    UNITS       "dBmV"
    MAX-ACCESS read-only
     STATUS      current
     DESCRIPTION
             "The received power level of this in-band interface."
     ::= { ocStbHostInBandTunerEntry 4 }
*/ 
int ocStbHostInBandTunerEntryBridge::Get_ocStbHostInBandTunerPower ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostInBandTunerPower" << endl;
  return 0;
}


/* 
ocStbHostInBandTunerAGCValue OBJECT-TYPE
    SYNTAX      Unsigned32(0..100)
    UNITS       "percentage"
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            In-band channel AGC value (%). When an In-band
            Carrier Lock is indicated, a value of 0 (zero)
            indicates that the ocStbHost does not compute the
            value."
    ::= { ocStbHostInBandTunerEntry 5 }
*/ 
unsigned int ocStbHostInBandTunerEntryBridge::Get_ocStbHostInBandTunerAGCValue ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostInBandTunerAGCValue" << endl;
  return 0;
}


/* 
ocStbHostInBandTunerSNRValue OBJECT-TYPE
    SYNTAX      TenthdB
    UNITS       "dB"
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            In-band channel SNR value (1/10 dB). When an
            In-band Carrier Lock is indicated, a value of 0
            (zero) indicates that the ocStbHost does not compute
            the value."
    ::= { ocStbHostInBandTunerEntry 6 }
*/ 
int ocStbHostInBandTunerEntryBridge::Get_ocStbHostInBandTunerSNRValue ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostInBandTunerSNRValue" << endl;
  return 0;
}


/* 
ocStbHostInBandTunerUnerroreds OBJECT-TYPE
    SYNTAX      Counter32
    UNITS       "codewords"
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Codewords received on this channel without error.
            Discontinuities in the value of this counter can occur
            at reinitialization of the managed system, and at other
            times as indicated by the value of
            ifCounterDiscontinuityTime for the associated ifIndex."
    ::= { ocStbHostInBandTunerEntry 7 }
*/ 
unsigned int ocStbHostInBandTunerEntryBridge::Get_ocStbHostInBandTunerUnerroreds ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostInBandTunerUnerroreds" << endl;
  return 0;
}


/* 
ocStbHostInBandTunerCorrecteds OBJECT-TYPE
    SYNTAX      Counter32
    UNITS       "codewords"
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Codewords received on this in-band channel with
            correctable errors. Discontinuities in the value of
            this counter can occur at reinitialization of the
            managed system."
    ::= { ocStbHostInBandTunerEntry 8 }
*/ 
unsigned int ocStbHostInBandTunerEntryBridge::Get_ocStbHostInBandTunerCorrecteds ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostInBandTunerCorrecteds" << endl;
  return 0;
}


/* 
ocStbHostInBandTunerUncorrectables OBJECT-TYPE
    SYNTAX      Counter32
    UNITS       "codewords"
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Codewords received on this in-band channel with
            uncorrectable errors.
            Discontinuities in the value of this counter can occur
            at reinitialization of the managed system."
    ::= { ocStbHostInBandTunerEntry 9 }
*/ 
unsigned int ocStbHostInBandTunerEntryBridge::Get_ocStbHostInBandTunerUncorrectables ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostInBandTunerUncorrectables" << endl;
  return 0;
}


/* 
ocStbHostInBandTunerCarrierLockLost OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The number of times the ocStbHost has detected carrier
            Lock is lost.
            Discontinuities in the value of this counter can occur
            at reinitialization of the managed system."
    ::= { ocStbHostInBandTunerEntry 10 }
*/ 
unsigned int ocStbHostInBandTunerEntryBridge::Get_ocStbHostInBandTunerCarrierLockLost ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostInBandTunerCarrierLockLost" << endl;
  return 0;
}


/* 
ocStbHostInBandTunerPCRErrors OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The number of times the Program Clock Reference (PCR)
            detected Synchronization errors; e.g., wrong PCR from
            multiplexer, excessive CPR Jitter, etc.
            Discontinuities in the value of this counter can occur
            at reinitialization of the managed system."
    ::= { ocStbHostInBandTunerEntry 11 }
*/ 
unsigned int ocStbHostInBandTunerEntryBridge::Get_ocStbHostInBandTunerPCRErrors ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostInBandTunerPCRErrors" << endl;
  return 0;
}


/* 
ocStbHostInBandTunerPTSErrors OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The number of Presentation TimeStamp (PTS) packets that
            encounter sequencing errors.
            Discontinuities in the value of this counter can occur
            at reinitialization of the managed system."
    ::= { ocStbHostInBandTunerEntry 12 }
*/ 
unsigned int ocStbHostInBandTunerEntryBridge::Get_ocStbHostInBandTunerPTSErrors ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostInBandTunerPTSErrors" << endl;
  return 0;
}


/* 
ocStbHostInBandTunerState OBJECT-TYPE
    SYNTAX      INTEGER {
                ready(1),
                waitingSync(2),
                waitingQam(3),
                foundSync(4),
                foundQam(5),
                unknown(6),
                standby(7)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The In-band tuner current state.

            Modulation modes are as follows:
            ready - indicates that the tuner is in idle mode (not
                currently in use).
            waitingSync - indicates that the tuner is trying to
                find sync on an analog channel.
            waitingQam - indicates that the tuner is trying to tune
                to the desired QAM frequency and lock onto valid data.
            foundSync - indicates that the tuner has successfully
                tuned to an analog channel.
            foundQam - indicates that the tuner has successfully
                tuned to a digital channel."
    ::= { ocStbHostInBandTunerEntry 13 }
*/ 
int ocStbHostInBandTunerEntryBridge::Get_ocStbHostInBandTunerState ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostInBandTunerState" << endl;
  return 0;
}


/* 
ocStbHostInBandTunerBER OBJECT-TYPE
    SYNTAX      INTEGER
                {
                berGreaterThan10e2(1),
                berRange10e2ToGreaterThan10e4(2),
                berRange10e4ToGreaterThan10e6(3),
                berRange10e6ToGreaterThan10e8(4),
                berRange10e8ToGreaterThan10e12(5),
                berEqualToOrLessThan10e12(6),
                berNotApplicable(7)
                }
     MAX-ACCESS read-only
     STATUS     current
     DESCRIPTION
             "The average Bit Error Rate range since last tune to a digital
             channel. This value is determined by dividing the number of
             erroneous bits by the total number of bits transmitted
             within the last 10 minutes of the object being polled.
             Report value of 7 if tuned to an analog channel."
     ::= { ocStbHostInBandTunerEntry 14 }
*/ 
int ocStbHostInBandTunerEntryBridge::Get_ocStbHostInBandTunerBER ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostInBandTunerBER" << endl;
  return 0;
}


/* 
ocStbHostInBandTunerSecsSinceLock OBJECT-TYPE
    SYNTAX      Unsigned32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The amount of time, in seconds, since the last tuner lock
            on a valid data signal. Each time the tuner locks the
            ocStbHostInBandTunerUnerroreds,
            ocStbHostInBandTunerCorrecteds, and
            ocStbHostInBandTunerUncorrectables, counters should be
            set to 0."
    ::= { ocStbHostInBandTunerEntry 15 }
*/ 
unsigned int ocStbHostInBandTunerEntryBridge::Get_ocStbHostInBandTunerSecsSinceLock ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostInBandTunerSecsSinceLock" << endl;
  return 0;
}


/* 
ocStbHostInBandTunerEqGain OBJECT-TYPE
    SYNTAX      Integer32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The value of the equalizer gain on the QAM data channel.
             A value of 0 (zero) indicates that the ocStbHost does not
             compute the value."
    ::= { ocStbHostInBandTunerEntry 16 }
*/ 
int ocStbHostInBandTunerEntryBridge::Get_ocStbHostInBandTunerEqGain ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostInBandTunerEqGain" << endl;
  return 0;
}


/* 
ocStbHostInBandTunerMainTapCoeff OBJECT-TYPE
    SYNTAX      INTEGER(1..32767)
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The normalized main tap coefficient used for calibrating
            data path delay or frequency response. A value of 0 (zero)
            indicates that the ocStbHost does not compute the value."
    ::= { ocStbHostInBandTunerEntry 17 }
*/ 
int ocStbHostInBandTunerEntryBridge::Get_ocStbHostInBandTunerMainTapCoeff ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostInBandTunerMainTapCoeff" << endl;
  return 0;
}


/* 
ocStbHostInBandTunerTotalTuneCount OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The number of times an attempt has been made to change
            the tuned frequency. This includes both successful and
            unsuccessful attempts."
    ::= { ocStbHostInBandTunerEntry 18 }
*/ 
unsigned int ocStbHostInBandTunerEntryBridge::Get_ocStbHostInBandTunerTotalTuneCount ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostInBandTunerTotalTuneCount" << endl;
  return 0;
}


/* 
ocStbHostInBandTunerTuneFailureCount OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The number of times an attempt to change the tuned
            frequency has resulted in a failure to acquire a signal
            (either analog or digital)."
    ::= { ocStbHostInBandTunerEntry 19 }
*/ 
unsigned int ocStbHostInBandTunerEntryBridge::Get_ocStbHostInBandTunerTuneFailureCount ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostInBandTunerTuneFailureCount" << endl;
  return 0;
}


/* 
ocStbHostInBandTunerTuneFailFreq OBJECT-TYPE
    SYNTAX      Unsigned32
    UNITS       "hertz"
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The most recent frequency at which an attempt to tune
            resulted in an increment of
            ocStbHostInBandTunerTuneFailureCount. If no tune attempt
            has failed since initialization, this value
            SHALL be zero."
    ::= { ocStbHostInBandTunerEntry 20 }
*/ 
unsigned int ocStbHostInBandTunerEntryBridge::Get_ocStbHostInBandTunerTuneFailFreq ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostInBandTunerTuneFailFreq" << endl;
  return 0;
}


/* 
ocStbHostInBandTunerBandwidth OBJECT-TYPE
    SYNTAX      INTEGER
                {
                other(1),
                mHz864(2),
                mHz1002(3)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The In-band tuner bandwidth.
            Tuner bandwidth choices are as follows:
            other     - Tuning range is not described
                        by any other listed range.
            mHz864    - Tuning range is 54 to 864 MHz
            mHz1002   - Tuning range is 54 to 1002 MHz."
    ::= { ocStbHostInBandTunerEntry 21 }
*/ 
int ocStbHostInBandTunerEntryBridge::Get_ocStbHostInBandTunerBandwidth ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostInBandTunerBandwidth" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostProgramStatusEntryBridge for bridging elements in the ocStbHostProgramStatusTable
 *
\*/

/* 
ocStbHostProgramStatusTable OBJECT-TYPE
    SYNTAX      SEQUENCE OF OcStbHostProgramStatusEntry
    MAX-ACCESS not-accessible
    STATUS      current
    DESCRIPTION
            "A table of current program content being forwarded by the
            ocStbHost."
    ::= { ocStbHostServiceProgramInfo 2 }

INDEX { ocStbHostProgramIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostProgramStatusEntryBridge::ocStbHostProgramStatusEntryBridge (unsigned long ocStbHostProgramIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ocStbHostProgramStatusTable,
                      BcmObjectId(ocStbHostProgramIndex),
                      pAgent)
{
}

ocStbHostProgramStatusEntryBridge::~ocStbHostProgramStatusEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostProgramIndex OBJECT-TYPE
    SYNTAX      Unsigned32 (1..20)
    MAX-ACCESS not-accessible
    STATUS      current
    DESCRIPTION
            "Index for Program Status Entry Table."
    ::= { ocStbHostProgramStatusEntry 1 }
*/ 
unsigned int ocStbHostProgramStatusEntryBridge::Get_ocStbHostProgramIndex ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostProgramIndex" << endl;
  return 0;
}


/* 
ocStbHostProgramAVSource OBJECT-TYPE
    SYNTAX      RowPointer
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Defines the source interface
            associated with this entry. Example sources are
            interfaces listed in the ifTable, interfaces listed in
            ocStbAVInterfaceTable, or a Host Resources Device such as a
            DVR.
            If the source corresponds to an entry in the
            ocStbAVInterfaceTable, this object reports the
            ocStbAVInterfaceType instance of the source interface.
            If the source corresponds to a resource listed in the
            ifTable, this object reports the ifIndex instance of
            the source interface.
            If the source corresponds to a resource listed in
            hrDeviceTable, this object reports the hrDeviceType
            instance of the source interface.
            Other table rows may be designated by this object
            as appropriate to the particular source."
    ::= { ocStbHostProgramStatusEntry 2 }
*/ 
int ocStbHostProgramStatusEntryBridge::Get_ocStbHostProgramAVSource (BcmObjectId &ocStbHostProgramAVSource)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostProgramAVSource" << endl;
  ocStbHostProgramAVSource = "0.0";

  return ocStbHostProgramAVSource.Length();
}


/* 
ocStbHostProgramAVDestination OBJECT-TYPE
    SYNTAX      RowPointer
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Defines the destination interface
            associated with this entry. Example destinations are
            interfaces listed in the ifTable, interfaces listed in
            ocStbAVInterfaceTable,
            or a Host Resource Device such as a DVR.
            If the destination corresponds to an entry in the
            ocStbAVInterfaceTable, this object reports the
            ocStbAVInterfaceType instance of the destination
            interface.
            If the destination corresponds to an interface listed in
            the ifTable, this object reports the ifIndex instance of
            the destination interface.
            If the destination corresponds to a resource listed in the
            hrDeviceTable, this object reports the hrDeviceType
            instance of the destination interface.
            Other table rows may be designated by this object
            as appropriate to the particular destination."
    ::= { ocStbHostProgramStatusEntry 3 }
*/ 
int ocStbHostProgramStatusEntryBridge::Get_ocStbHostProgramAVDestination (BcmObjectId &ocStbHostProgramAVDestination)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostProgramAVDestination" << endl;
  ocStbHostProgramAVDestination = "0.0";

  return ocStbHostProgramAVDestination.Length();
}


/* 
ocStbHostProgramContentSource OBJECT-TYPE
    SYNTAX      RowPointer
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "A pointer to the first visible object structure that
            defines the type of content associated to the A/V source
            interface. If the content is from an MPEG2 stream, this
            object points to the first visible object in
            ocStbHostMpeg2ContentEntry.
            Vendor-specific Content models may be defined in the vendor
            enterprise IANA assigned OID.
            This object reports 'zeroDotZero' value if no content
            information is available."
    ::= { ocStbHostProgramStatusEntry 4 }
*/ 
int ocStbHostProgramStatusEntryBridge::Get_ocStbHostProgramContentSource (BcmObjectId &ocStbHostProgramContentSource)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostProgramContentSource" << endl;
  ocStbHostProgramContentSource = "0.0";

  return ocStbHostProgramContentSource.Length();
}


/* 
ocStbHostProgramContentDestination OBJECT-TYPE
    SYNTAX      RowPointer
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "A pointer to the first visible object structure that
            defines the type of content associated to the A/V
            destination interface. For example, if the content for the
            destination interface is an MPEG2 stream
            (e.g., IEEE 1394 SPTS), this object points to the
            first visible object in ocStbHostMpeg2ContentEntry.
            Vendor-specific Content models may be defined in the vendor
            enterprise IANA assigned OID.
            This object reports 'zeroDotZero' value if no content
            information is available."
    ::= { ocStbHostProgramStatusEntry 5 }
*/ 
int ocStbHostProgramStatusEntryBridge::Get_ocStbHostProgramContentDestination (BcmObjectId &ocStbHostProgramContentDestination)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostProgramContentDestination" << endl;
  ocStbHostProgramContentDestination = "0.0";

  return ocStbHostProgramContentDestination.Length();
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostMpeg2ContentEntryBridge for bridging elements in the ocStbHostMpeg2ContentTable
 *
\*/

/* 
ocStbHostMpeg2ContentTable OBJECT-TYPE
    SYNTAX      SEQUENCE OF OcStbHostMpeg2ContentEntry
    MAX-ACCESS not-accessible
    STATUS      current
    DESCRIPTION
            "Contains information associated to an MPEG-2
            Program carried by an MPEG-2 transport stream.
            Entries in this table can be
            associated to interfaces receiving or forwarding
            MPEG-2 content."
    ::= { ocStbHostServiceProgramInfo 3 }

INDEX { ocStbHostMpeg2ContentIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostMpeg2ContentEntryBridge::ocStbHostMpeg2ContentEntryBridge (unsigned long ocStbHostMpeg2ContentIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ocStbHostMpeg2ContentTable,
                      BcmObjectId(ocStbHostMpeg2ContentIndex),
                      pAgent)
{
}

ocStbHostMpeg2ContentEntryBridge::~ocStbHostMpeg2ContentEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostMpeg2ContentIndex OBJECT-TYPE
    SYNTAX      Unsigned32 (1..20)
    MAX-ACCESS not-accessible
    STATUS      current
    DESCRIPTION
            "The unique identifier for the Mpeg2 Content entries in
            this table."
    ::= { ocStbHostMpeg2ContentEntry 1 }
*/ 
unsigned int ocStbHostMpeg2ContentEntryBridge::Get_ocStbHostMpeg2ContentIndex ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostMpeg2ContentIndex" << endl;
  return 0;
}


/* 
ocStbHostMpeg2ContentProgramNumber OBJECT-TYPE
    SYNTAX      Unsigned32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The program Number associated with active content within
            the ocStbHost."
    ::= { ocStbHostMpeg2ContentEntry 2 }
*/ 
unsigned int ocStbHostMpeg2ContentEntryBridge::Get_ocStbHostMpeg2ContentProgramNumber ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostMpeg2ContentProgramNumber" << endl;
  return 0;
}


/* 
ocStbHostMpeg2ContentTransportStreamID OBJECT-TYPE
    SYNTAX      Unsigned32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Defines the transport stream ID associated with the
            program number."
    ::= { ocStbHostMpeg2ContentEntry 3 }
*/ 
unsigned int ocStbHostMpeg2ContentEntryBridge::Get_ocStbHostMpeg2ContentTransportStreamID ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostMpeg2ContentTransportStreamID" << endl;
  return 0;
}


/* 
ocStbHostMpeg2ContentTotalStreams OBJECT-TYPE
    SYNTAX      Unsigned32
    UNITS       "streams"
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Defines the total number of elementary streams associated
            with the program number."
    ::= { ocStbHostMpeg2ContentEntry 4 }
*/ 
unsigned int ocStbHostMpeg2ContentEntryBridge::Get_ocStbHostMpeg2ContentTotalStreams ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostMpeg2ContentTotalStreams" << endl;
  return 0;
}


/* 
ocStbHostMpeg2ContentSelectedVideoPID OBJECT-TYPE
    SYNTAX      Integer32 (-1 | 1..8191)
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            PID associated to the selected video
            stream. The value '-1' indicates that no video stream
            is being selected or identified.
            The value is the 13-bit PID value expressed in decimal."
    ::= { ocStbHostMpeg2ContentEntry 5 }
*/ 
int ocStbHostMpeg2ContentEntryBridge::Get_ocStbHostMpeg2ContentSelectedVideoPID ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostMpeg2ContentSelectedVideoPID" << endl;
  return 0;
}


/* 
ocStbHostMpeg2ContentSelectedAudioPID OBJECT-TYPE
    SYNTAX      Integer32 (-1 | 1..8191)
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The selected audio PID.
            The value '-1' indicates no audio stream identified."
    ::= { ocStbHostMpeg2ContentEntry 6 }
*/ 
int ocStbHostMpeg2ContentEntryBridge::Get_ocStbHostMpeg2ContentSelectedAudioPID ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostMpeg2ContentSelectedAudioPID" << endl;
  return 0;
}


/* 
ocStbHostMpeg2ContentOtherAudioPIDs OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Then value 'true' indicates the MPEG-2 program
            has more than one audio PID.
            'false' indicates the MPEG-2 program has only one
            audio PID."
    ::= { ocStbHostMpeg2ContentEntry 7 }
*/ 
bool ocStbHostMpeg2ContentEntryBridge::Get_ocStbHostMpeg2ContentOtherAudioPIDs ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostMpeg2ContentOtherAudioPIDs" << endl;
  return true;
}


/* 
ocStbHostMpeg2ContentCCIValue OBJECT-TYPE
    SYNTAX      INTEGER
                {
                copyFreely(0),
                copyNoMore(1),
                copyOneGeneration(2),
                copyNever(3),
                undefined(4)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This value allows the manager to retrieve the
            encryption Mode Indication (EMI) Bits of the
            Copy Control Information (CCI) byte associated
            with the service."
    ::= { ocStbHostMpeg2ContentEntry 8 }
*/ 
int ocStbHostMpeg2ContentEntryBridge::Get_ocStbHostMpeg2ContentCCIValue ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostMpeg2ContentCCIValue" << endl;
  return 0;
}


/* 
ocStbHostMpeg2ContentAPSValue OBJECT-TYPE
    SYNTAX      INTEGER
                {
                type1(1),
                type2(2),
                type3(3),
                noMacrovision(4),
                notDefined(5)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This value allows the manager to retrieve the
            Analog Protection System (APS) bits of the
            Copy Control Information (CCI) byte
            associated with the service.
            'type1' indicates AGC ON, split burst OFF
            'type2' indicates AGC ON, 2-line split burst
            'type3' indicates AGC ON, 4-line split burst."
    ::= { ocStbHostMpeg2ContentEntry 9 }
*/ 
int ocStbHostMpeg2ContentEntryBridge::Get_ocStbHostMpeg2ContentAPSValue ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostMpeg2ContentAPSValue" << endl;
  return 0;
}


/* 
ocStbHostMpeg2ContentCITStatus OBJECT-TYPE
    SYNTAX      TruthValue
     MAX-ACCESS read-only
     STATUS      current
     DESCRIPTION
             "This value allows the manager to retrieve the state
             of the Constrained Image Trigger(CIT).

             'true' indicates image constraint required.
             'false' indicates image constraint not asserted."
     ::= { ocStbHostMpeg2ContentEntry 10 }
*/ 
bool ocStbHostMpeg2ContentEntryBridge::Get_ocStbHostMpeg2ContentCITStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostMpeg2ContentCITStatus" << endl;
  return true;
}


/* 
ocStbHostMpeg2ContentBroadcastFlagStatus OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This value allows the manager to retrieve the state
            of Broadcast Flag in the source interface.
            'true' indicates Broadcast Flag present.
            'false' indicates Broadcast Flag not present."
    ::= { ocStbHostMpeg2ContentEntry 11 }
*/ 
bool ocStbHostMpeg2ContentEntryBridge::Get_ocStbHostMpeg2ContentBroadcastFlagStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostMpeg2ContentBroadcastFlagStatus" << endl;
  return true;
}


/* 
ocStbHostMpeg2ContentEPNStatus OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            EPN status of the MPEG stream. 'true' indicates
            that EPN is asserted."
    ::= { ocStbHostMpeg2ContentEntry 12 }
*/ 
bool ocStbHostMpeg2ContentEntryBridge::Get_ocStbHostMpeg2ContentEPNStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostMpeg2ContentEPNStatus" << endl;
  return true;
}


/* 
ocStbHostMpeg2ContentPCRPID OBJECT-TYPE
    SYNTAX      Integer32 (-1 | 1..8191)
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            PCR PID associated to the selected video
            stream. The value '-1' indicates that no video stream
            is being selected or identified. The value is the 13-bit
            PCR PID value expressed in decimal."
    ::= { ocStbHostMpeg2ContentEntry 13 }
*/ 
int ocStbHostMpeg2ContentEntryBridge::Get_ocStbHostMpeg2ContentPCRPID ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostMpeg2ContentPCRPID" << endl;
  return 0;
}


/* 
ocStbHostMpeg2ContentPCRLockStatus OBJECT-TYPE
    SYNTAX      INTEGER
                {
                notLocked(1),
                locked(2)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The status of the Program Clock Reference lock."
    ::= { ocStbHostMpeg2ContentEntry 14 }
*/ 
int ocStbHostMpeg2ContentEntryBridge::Get_ocStbHostMpeg2ContentPCRLockStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostMpeg2ContentPCRLockStatus" << endl;
  return 0;
}


/* 
ocStbHostMpeg2ContentDecoderPTS OBJECT-TYPE
    SYNTAX      Integer32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The value of the MPEG decoder Presentation TimeStamp
            (PTS). The LSB should be dropped to reduce the PTS length
            to 32 bits."
    ::= { ocStbHostMpeg2ContentEntry 15 }
*/ 
int ocStbHostMpeg2ContentEntryBridge::Get_ocStbHostMpeg2ContentDecoderPTS ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostMpeg2ContentDecoderPTS" << endl;
  return 0;
}


/* 
ocStbHostMpeg2ContentDiscontinuities OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The sum of the audio and video PID discontinuities
            detected on the MPEG stream. The values are based on
            the time since PCR lock on the current PIDs or stream being
            reported"
    ::= { ocStbHostMpeg2ContentEntry 16 }
*/ 
unsigned int ocStbHostMpeg2ContentEntryBridge::Get_ocStbHostMpeg2ContentDiscontinuities ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostMpeg2ContentDiscontinuities" << endl;
  return 0;
}


/* 
ocStbHostMpeg2ContentPktErrors OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The number of errors detected in the MPEG stream. The
             Values are based on the time since PCR lock on the
             Current PIDs or stream being reported"
    ::= { ocStbHostMpeg2ContentEntry 17 }
*/ 
unsigned int ocStbHostMpeg2ContentEntryBridge::Get_ocStbHostMpeg2ContentPktErrors ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostMpeg2ContentPktErrors" << endl;
  return 0;
}


/* 
ocStbHostMpeg2ContentPipelineErrors OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The number of errors reported exclusively by the MPEG decoder."
    ::= { ocStbHostMpeg2ContentEntry 18 }
*/ 
unsigned int ocStbHostMpeg2ContentEntryBridge::Get_ocStbHostMpeg2ContentPipelineErrors ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostMpeg2ContentPipelineErrors" << endl;
  return 0;
}


/* 
ocStbHostMpeg2ContentDecoderRestarts OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The number of times the software driver restarted
            the MPEG decoding process."
    ::= { ocStbHostMpeg2ContentEntry 19 }
*/ 
unsigned int ocStbHostMpeg2ContentEntryBridge::Get_ocStbHostMpeg2ContentDecoderRestarts ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostMpeg2ContentDecoderRestarts" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostAnalogVideoEntryBridge for bridging elements in the ocStbHostAnalogVideoTable
 *
\*/

/* 
ocStbHostAnalogVideoTable OBJECT-TYPE
    SYNTAX      SEQUENCE OF OcStbHostAnalogVideoEntry
    MAX-ACCESS not-accessible
    STATUS      current
    DESCRIPTION
            "Contains information about the Analog Video signal.
            Entries in this table can be associated to interfaces
            receiving or forwarding analog content."
    ::= { ocStbHostServiceProgramInfo 4 }

INDEX { ocStbHostAVInterfaceIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostAnalogVideoEntryBridge::ocStbHostAnalogVideoEntryBridge (unsigned long ocStbHostAVInterfaceIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ocStbHostAnalogVideoTable,
                      BcmObjectId(ocStbHostAVInterfaceIndex),
                      pAgent)
{
}

ocStbHostAnalogVideoEntryBridge::~ocStbHostAnalogVideoEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostAnalogVideoProtectionStatus OBJECT-TYPE
    SYNTAX      INTEGER {
                copyProtectionOff(0),
                  splitBurstOff(1),
                  twoLineSplitBurst(2),
                  fourLineSplitBurst(3)
                  }
     MAX-ACCESS   read-only
     STATUS       current
     DESCRIPTION
             "The analog protection system (APS) of this Video content
             entry. The possible values are:
             'copyProtectionOff' means the Macrovision Analog protection
              is off
             'splitBurstOff' means the AGC (Automatic Gain Control)
              is on and the split burst is off.
             'twoLineSplitBurst' means AGC is on with two-line
              split burst.
             'fourLineSplitBurst' means AGC is on with four-line
              split burst.
             This object is instantiated only for A/V output interfaces
             that support APS like but not limited to:
             'ocStbHostRfOutCh', 'ocStbHostSVideoOut' and
             'ocStbHostBbVideoOut'.
             The A/V interfaces 'ocStbHostComponentOut'
             only support 'splitBurstOff' APS mode for the video formats
             'format480i' and 'format480p'."
     ::= { ocStbHostAnalogVideoEntry 1 }
*/ 
int ocStbHostAnalogVideoEntryBridge::Get_ocStbHostAnalogVideoProtectionStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostAnalogVideoProtectionStatus" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostQpskObjectsGroupBridge for bridging scalars from the ocStbHostQpskObjectsGroup group 
 *
\*/

/* 
ocStbHostQpskObjects OBJECT IDENTIFIER ::= {ocStbHostInterfaces 8}
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostQpskObjectsGroupBridge::ocStbHostQpskObjectsGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_ocStbHostQpskObjectsGroup, pAgent)
{
}

ocStbHostQpskObjectsGroupBridge::~ocStbHostQpskObjectsGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostQpskFDCFreq OBJECT-TYPE
    SYNTAX      Unsigned32
    UNITS       "hertz"
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The frequency to which the QPSK receiver is currently tuned."
    ::= { ocStbHostQpskObjects 1 }
*/ 
unsigned int ocStbHostQpskObjectsGroupBridge::Get_ocStbHostQpskFDCFreq ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostQpskFDCFreq" << endl;
  return 0;
}


/* 
ocStbHostQpskRDCFreq OBJECT-TYPE
    SYNTAX      Unsigned32
    UNITS       "hertz"
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The frequency to which the QPSK transmitter is currently tuned."
    ::= { ocStbHostQpskObjects 2 }
*/ 
unsigned int ocStbHostQpskObjectsGroupBridge::Get_ocStbHostQpskRDCFreq ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostQpskRDCFreq" << endl;
  return 0;
}


/* 
ocStbHostQpskFDCBer OBJECT-TYPE
    SYNTAX      INTEGER
                {
                berGreaterThan10e2(1),
                berRange10e2ToGreaterThan10e4(2),
                berRange10e4ToGreaterThan10e6(3),
                berRange10e6ToGreaterThan10e8(4),
                berRange10e8ToGreaterThan10e12(5),
                berEqualToOrLessThan10e12(6),
                berNotApplicable(7)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The rate at which bit errors are occurring based on the
            number of data errors since the frequency has been locked."
    ::= { ocStbHostQpskObjects 3 }
*/ 
int ocStbHostQpskObjectsGroupBridge::Get_ocStbHostQpskFDCBer ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostQpskFDCBer" << endl;
  return 0;
}


/* 
ocStbHostQpskFDCStatus OBJECT-TYPE
    SYNTAX      INTEGER
                {
                notLocked(1),
                locked(2)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Forward data channel lock status."
    ::= { ocStbHostQpskObjects 4 }
*/ 
int ocStbHostQpskObjectsGroupBridge::Get_ocStbHostQpskFDCStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostQpskFDCStatus" << endl;
  return 0;
}


/* 
ocStbHostQpskFDCBytesRead OBJECT-TYPE
    SYNTAX      Unsigned32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The number of bytes successfully read since the frequency
            was locked."
    ::= { ocStbHostQpskObjects 5 }
*/ 
unsigned int ocStbHostQpskObjectsGroupBridge::Get_ocStbHostQpskFDCBytesRead ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostQpskFDCBytesRead" << endl;
  return 0;
}


/* 
ocStbHostQpskFDCPower OBJECT-TYPE
    SYNTAX      TenthdBmV
    UNITS       "dBmV"
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The power level, in dBmV, of the forward data channel."
    ::= { ocStbHostQpskObjects 6 }
*/ 
int ocStbHostQpskObjectsGroupBridge::Get_ocStbHostQpskFDCPower ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostQpskFDCPower" << endl;
  return 0;
}


/* 
ocStbHostQpskFDCLockedTime OBJECT-TYPE
    SYNTAX      Unsigned32
    UNITS       "seconds"
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The number of seconds that the current FDC frequency
            has been locked."
    ::= { ocStbHostQpskObjects 7 }
*/ 
unsigned int ocStbHostQpskObjectsGroupBridge::Get_ocStbHostQpskFDCLockedTime ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostQpskFDCLockedTime" << endl;
  return 0;
}


/* 
ocStbHostQpskFDCSNR OBJECT-TYPE
    SYNTAX      TenthdB
    UNITS       "dB"
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            FDC SNR value (1/10 dB)."
    ::= { ocStbHostQpskObjects 8 }
*/ 
int ocStbHostQpskObjectsGroupBridge::Get_ocStbHostQpskFDCSNR ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostQpskFDCSNR" << endl;
  return 0;
}


/* 
ocStbHostQpskAGC OBJECT-TYPE
    SYNTAX      Unsigned32
    UNITS       "percent"
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the OOB channel
            Automatic Gain Control value (%)."
    ::= { ocStbHostQpskObjects 9 }
*/ 
unsigned int ocStbHostQpskObjectsGroupBridge::Get_ocStbHostQpskAGC ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostQpskAGC" << endl;
  return 0;
}


/* 
ocStbHostQpskRDCPower OBJECT-TYPE
    SYNTAX      TenthdBmV
    UNITS       "dBmV"
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
             "The power output level, in dBmV, of the QPSK transmitter."
     ::= { ocStbHostQpskObjects 10 }
*/ 
int ocStbHostQpskObjectsGroupBridge::Get_ocStbHostQpskRDCPower ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostQpskRDCPower" << endl;
  return 0;
}


/* 
ocStbHostQpskRDCDataRate OBJECT-TYPE
    SYNTAX      INTEGER
                {
                kbps256(1),
                kbps1544(2),
                kbps3088(3)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The data rate (in kbps) of the RDC."
    ::= { ocStbHostQpskObjects 11 }
*/ 
int ocStbHostQpskObjectsGroupBridge::Get_ocStbHostQpskRDCDataRate ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostQpskRDCDataRate" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostEasCodesGroupBridge for bridging scalars from the ocStbHostEasCodesGroup group 
 *
\*/

/* 
ocStbHostEasCodes   OBJECT IDENTIFIER ::= { ocStbHostEasObjects 1 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostEasCodesGroupBridge::ocStbHostEasCodesGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_ocStbHostEasCodesGroup, pAgent)
{
}

ocStbHostEasCodesGroupBridge::~ocStbHostEasCodesGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbEasMessageStateCode OBJECT-TYPE
    SYNTAX      Unsigned32 (0..99)
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            EAS State Code utilized by the STB. A
            value of 0 indicates that the code is not
            defined, thus the eSTB does not discriminate
            any State Code defined in an EAS message."
    REFERENCE
            "SCTE-18"
    ::= { ocStbHostEasCodes 1 }
*/ 
unsigned int ocStbHostEasCodesGroupBridge::Get_ocStbEasMessageStateCode ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbEasMessageStateCode" << endl;
  return 0;
}


/* 
ocStbEasMessageCountyCode OBJECT-TYPE
    SYNTAX      Unsigned32 (0..999)
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            EAS County Code utilized by the STB. A
            value of 0 indicates that the code is not
            defined, thus the eSTB ignores any County
            Code defined in an EAS message."
    REFERENCE
            "SCTE-18"
    ::= { ocStbHostEasCodes 2 }
*/ 
unsigned int ocStbHostEasCodesGroupBridge::Get_ocStbEasMessageCountyCode ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbEasMessageCountyCode" << endl;
  return 0;
}


/* 
ocStbEasMessageCountySubdivisionCode OBJECT-TYPE
    SYNTAX      Unsigned32 (0..9)
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object allows the manager to retrieve the
            EAS County Sub Code utilized by the STB. A
            value of 0 indicates that the code is not
            defined, thus the eSTB ignores it and any County
            Subdivision Code defined in an EAS message is accepted."
    REFERENCE
            "SCTE-18"
    ::= { ocStbHostEasCodes 3}
*/ 
unsigned int ocStbHostEasCodesGroupBridge::Get_ocStbEasMessageCountySubdivisionCode ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbEasMessageCountySubdivisionCode" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostSecuritySubSystemGroupBridge for bridging scalars from the ocStbHostSecuritySubSystemGroup group 
 *
\*/

/* 
ocStbHostSecuritySubSystem OBJECT IDENTIFIER ::= { ocStbHostMibObjects 2 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostSecuritySubSystemGroupBridge::ocStbHostSecuritySubSystemGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_ocStbHostSecuritySubSystemGroup, pAgent)
{
}

ocStbHostSecuritySubSystemGroupBridge::~ocStbHostSecuritySubSystemGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostCASystemIdentifier OBJECT-TYPE
    SYNTAX      SnmpAdminString
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Conditional access system identifier defined by DVB
            and MPEG. This object represents the 16-bit CA_system_id
            obtained from the ca_info_reply apdu. Only the first CAS ID
            value (16-bits) from ca_info()will be reported."
    ::= { ocStbHostSecuritySubSystem 3 }
*/ 
int ocStbHostSecuritySubSystemGroupBridge::Get_ocStbHostCASystemIdentifier (BcmString &ocStbHostCASystemIdentifier)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCASystemIdentifier" << endl;
  ocStbHostCASystemIdentifier = "(unfinished)";

  return ocStbHostCASystemIdentifier.length();
}


/* 
ocStbHostCAType OBJECT-TYPE
    SYNTAX      INTEGER {
                other(1),
                embedded(2),
                cablecard(3),
                dcas(4)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The Conditional Access (CA) supported by
            the host device.
            'other' means a CA which is not described by one of
            the other enumerations in this list.
            'embedded' means a vendor-specific embedded CA
            'cablecard' means OpenCable CableCARD is supported.
            'dcas' means software downloadable CA is
            supported."
    ::= { ocStbHostSecuritySubSystem 4 }
*/ 
int ocStbHostSecuritySubSystemGroupBridge::Get_ocStbHostCAType ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCAType" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostDeviceSoftwareBaseGroupBridge for bridging scalars from the ocStbHostDeviceSoftwareBaseGroup group 
 *
\*/

/* 
ocStbHostDeviceSoftwareBase OBJECT IDENTIFIER ::= { ocStbHostSoftware 1 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostDeviceSoftwareBaseGroupBridge::ocStbHostDeviceSoftwareBaseGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_ocStbHostDeviceSoftwareBaseGroup, pAgent)
{
}

ocStbHostDeviceSoftwareBaseGroupBridge::~ocStbHostDeviceSoftwareBaseGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostSoftwareFirmwareVersion OBJECT-TYPE
     SYNTAX      SnmpAdminString
     MAX-ACCESS read-only
     STATUS      current
     DESCRIPTION
             "The platform code software version currently operating in
             this device. This object should be in the syntax used by
             the individual vendor to identify software versions. The
             device must return a string descriptive of the current
             software load. This object is shared with
             docsDevSwCurrentVers. For a monolithic download image,
             this object has the same value as docsDevSwCurrentVers
             on the eCM."
     ::= { ocStbHostDeviceSoftwareBase 1 }
*/ 
int ocStbHostDeviceSoftwareBaseGroupBridge::Get_ocStbHostSoftwareFirmwareVersion (BcmString &ocStbHostSoftwareFirmwareVersion)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostSoftwareFirmwareVersion" << endl;
  ocStbHostSoftwareFirmwareVersion = "(unfinished)";

  return ocStbHostSoftwareFirmwareVersion.length();
}


/* 
ocStbHostSoftwareOCAPVersion OBJECT-TYPE
    SYNTAX      SnmpAdminString
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object returns a string indicating the latest version
            of the OCAP profile supported by this device, e.g., '1.0'."
    ::= { ocStbHostDeviceSoftwareBase 2 }
*/ 
int ocStbHostDeviceSoftwareBaseGroupBridge::Get_ocStbHostSoftwareOCAPVersion (BcmString &ocStbHostSoftwareOCAPVersion)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostSoftwareOCAPVersion" << endl;
  ocStbHostSoftwareOCAPVersion = "(unfinished)";

  return ocStbHostSoftwareOCAPVersion.length();
}


/* 
ocStbHostSoftwareFirmwareReleaseDate OBJECT-TYPE
    SYNTAX      DateAndTime
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object returns release date of this entire firmware
            image (the stack OS). The local time fields should be
            set to zero and the timezone information omitted."
    ::= { ocStbHostDeviceSoftwareBase 3 }
*/ 
time_t ocStbHostDeviceSoftwareBaseGroupBridge::Get_ocStbHostSoftwareFirmwareReleaseDate ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostSoftwareFirmwareReleaseDate" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostFirmwareDownloadStatusGroupBridge for bridging scalars from the ocStbHostFirmwareDownloadStatusGroup group 
 *
\*/

/* 
ocStbHostFirmwareDownloadStatus OBJECT IDENTIFIER ::= { ocStbHostSoftware 2 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostFirmwareDownloadStatusGroupBridge::ocStbHostFirmwareDownloadStatusGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_ocStbHostFirmwareDownloadStatusGroup, pAgent)
{
}

ocStbHostFirmwareDownloadStatusGroupBridge::~ocStbHostFirmwareDownloadStatusGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostFirmwareImageStatus OBJECT-TYPE
    SYNTAX      INTEGER
                {
                imageAuthorized(1),
                imageCorrupted(2),
                imageCertFailure(3),
                imageMaxDownloadRetry(4),
                imageMaxRebootRetry(5)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object details the image status recently downloaded
            'imageAuthorized' means the image is valid.
            'imageCorrupted' means the image is invalid.
            'imageCertFailure' means CVC authentication has failed.
            'imageMaxDownloadRetry' means the maximum number of code
            file download retries has been reached. When the value of
            this object is imageMaxDownloadRetry(4), the value of
            ocStbHostSoftwareCodeDownloadStatus must be
            downloadingFailed(3).
            'imageMaxRebootRetry' means the maximum number of reboots
            has occurred after code file download."
    ::= { ocStbHostFirmwareDownloadStatus 1 }
*/ 
int ocStbHostFirmwareDownloadStatusGroupBridge::Get_ocStbHostFirmwareImageStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostFirmwareImageStatus" << endl;
  return 0;
}


/* 
ocStbHostFirmwareCodeDownloadStatus OBJECT-TYPE
    SYNTAX      INTEGER
                {
                downloadingStarted(1),
                downloadingComplete(2),
                  downloadingFailed(3),
                  other(4)
                  }
    MAX-ACCESS    read-only
    STATUS        current
    DESCRIPTION
            "This object details the download status
            of the target image.
            'downloadingComplete' means the image download was
            successful.
            'downloadingFailed' means the image download failed.
            'other' means a status not described by the other
            enumerations."
    ::= { ocStbHostFirmwareDownloadStatus 2 }
*/ 
int ocStbHostFirmwareDownloadStatusGroupBridge::Get_ocStbHostFirmwareCodeDownloadStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostFirmwareCodeDownloadStatus" << endl;
  return 0;
}


/* 
ocStbHostFirmwareCodeObjectName OBJECT-TYPE
    SYNTAX      SnmpAdminString
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The file name of the software image to be loaded into this
            device. This object value may optionally contain server
            and path information about the file name when applicable.
            If unknown, the string '(unknown)' is returned. This
            object is shared with docsDevSwFilename."
    ::= { ocStbHostFirmwareDownloadStatus 3 }
*/ 
int ocStbHostFirmwareDownloadStatusGroupBridge::Get_ocStbHostFirmwareCodeObjectName (BcmString &ocStbHostFirmwareCodeObjectName)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostFirmwareCodeObjectName" << endl;
  ocStbHostFirmwareCodeObjectName = "(unfinished)";

  return ocStbHostFirmwareCodeObjectName.length();
}


/* 
ocStbHostFirmwareDownloadFailedStatus OBJECT-TYPE
    SYNTAX      INTEGER
                {
                cdlError1(1),
                cdlError2(2),
                cdlError3(3),
                cdlError4(4),
                cdlError5(5),
                cdlError6(6),
                cdlError7(7),
                cdlError8(8),
                cdlError9(9),
                cdlError10(10),
                cdlError11(11),
                cdlError12(12),
                cdlError13(13),
                cdlError14(14),
                cdlError15(15),
                cdlError16(16),
                cdlError17(17),
                cdlError18(18),
                cdlError19(19),
                cdlError20(20),
                cdlError21(21),
                cdlError22(22),
                cdlError23(23),
                cdlError24(24),
                cdlError25(25),
                cdlError99(99)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object details the firmware download
            failed status codes.

             cdlError1, No Failure
             cdlError2, Improper code file controls - CVC subject
                 organizationName for manufacturer does not match the
                 Host device manufacturer name
             cdlError3, Improper code file controls - CVC subject
                 organizationName for code cosigning agent does not
                 match the Host device current code cosigning agent.
             cdlError4, Improper code file controls - The
                 manufacturer's PKCS #7 signingTime value is equal-to
                 or less-than the codeAccessStart value currently held
                 in the Host device.
             cdlError5, Improper code file controls - The
                 manufacturer's PKCS #7 validity start time value is
                 less-than the cvcAccessStart value currently held in
                 the Host device.
             cdlError6, Improper code file controls - The manufacturer's
                 CVC validity start time is less-than the cvcAccessStart
                 value currently held in the Host device.
             cdlError7, Improper code file controls - The manufacturer's
                 PKCS #7 signingTime value is less-than the CVC validity
                 start time.
             cdlError8, Improper code file controls - Missing or
                 improper extendedKeyUsage extension in the
                 manufacturer CVC.
             cdlError9, Improper code file controls - The cosigner's
                 PKCS #7 signingTime value is equal-to or less-than the
                 codeAccessStart value currently held in the
                 Host device.
             cdlError10, Improper code file controls - The cosigner's
                 PKCS #7 validity start time value is less-than the
                 cvcAccessStart value currently held in the Host device.
             cdlError11, Improper code file controls - The cosigner's
                 CVC validity start time is less-than the cvcAccessStart
                 value currently held in the Host device.
             cdlError12, Improper code file controls - The cosigner's
                 PKCS #7 signingTime value is less-than the CVC validity
                 start time.
             cdlError13, Improper code file controls - Missing or
                 improper extended key-usage extension in the cosigner's
                 CVC.
             cdlError14, Code file manufacturer CVC validation failure.
             cdlError15, Code file manufacturer CVS validation failure.
             cdlError16, Code file cosigner CVC validation failure.
             cdlError17, Code file cosigner CVS validation failure.
             cdlError18, Improper eCM configuration file CVC format
                 (e.g., missing or improper key usage attribute).
             cdlError19, eCM configuration file CVC validation failure.
             cdlError20, Improper SNMP CVC format.
             cdlError21, CVC subject organizationName for manufacturer
                 does not match the Host devices manufacturer name.
             cdlError22, CVC subject organizationName for code cosigning
                 agent does not match the Host devices current code
                 cosigning agent.
             cdlError23, The CVC validity start time is less-than or
                 equal-to the corresponding subject's cvcAccessStart
                 value currently held in the Host device.
             cdlError24, Missing or improper key usage attribute.
             cdlError25, SNMP CVC validation failure.
             cdlError99, Other errors. Errors that do not fall into the
                 categories delineated above."
     ::= { ocStbHostFirmwareDownloadStatus 4 }
*/ 
int ocStbHostFirmwareDownloadStatusGroupBridge::Get_ocStbHostFirmwareDownloadFailedStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostFirmwareDownloadFailedStatus" << endl;
  return 0;
}


/* 
ocStbHostFirmwareDownloadFailedCount OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object details the number of times the product
            attempted to download the firmware object defined by
            ocStbHostFirmwareCodeObjectName. The value is reset when
            the product attempts to download a new firmware object."
    ::= { ocStbHostFirmwareDownloadStatus 5 }
*/ 
unsigned int ocStbHostFirmwareDownloadStatusGroupBridge::Get_ocStbHostFirmwareDownloadFailedCount ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostFirmwareDownloadFailedCount" << endl;
  return 0;
}


/* 
ocStbHostFirmwareDownloadGroupId OBJECT-TYPE
    SYNTAX      Unsigned32 (0..65535)
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object specifies the Group Identifier. This is persistent.
            The host may initiate a download if the group identifier in the
            Common Download code version table matches this value. Downloads
            can be triggered using the Common Download code version table
            message for global upgrade of a group of similar hosts. The
            Group Identifier is the same a group_id used in [CDL2]"
    DEFVAL { 0 }
    ::= { ocStbHostFirmwareDownloadStatus 6 }
*/ 
unsigned int ocStbHostFirmwareDownloadStatusGroupBridge::Get_ocStbHostFirmwareDownloadGroupId ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostFirmwareDownloadGroupId" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostSoftwareApplicationInfoGroupBridge for bridging scalars from the ocStbHostSoftwareApplicationInfoGroup group 
 *
\*/

/* 
ocStbHostSoftwareApplicationInfo OBJECT IDENTIFIER ::= { ocStbHostSoftware 3 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostSoftwareApplicationInfoGroupBridge::ocStbHostSoftwareApplicationInfoGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_ocStbHostSoftwareApplicationInfoGroup, pAgent)
{
}

ocStbHostSoftwareApplicationInfoGroupBridge::~ocStbHostSoftwareApplicationInfoGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostSoftwareApplicationInfoSigLastReceivedTime OBJECT-TYPE
    SYNTAX      DateAndTime
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Indicates the last time an XAIT was received from the
            network that was not the same version as already in-use.
            If the host device has not received an XAIT, then this
            returns a date and time of all zeros.
              - This must be updated ONLY when an XAIT is received
                from the network (e.g., not when loaded from persistent
                storage).
              - This must NOT be updated if an XAIT is received that
                has a version the same as the currently in-use version,
                UNLESS the in-use XAIT was loaded from persistent storage
                AND it is the FIRST time receiving an XAIT from the network
                without error.
              - This must NOT be updated if XAIT is received with a
                status of error(2).
              - This must be updated each time an XAIT is received for
                resulting in a status of okayButRejected(3).
              - This must be updated if an XAIT is received from the
                network resulting in a status of okay(1) AND its version
                is different than the currently in-use
                version."
    ::= { ocStbHostSoftwareApplicationInfo 2 }
*/ 
time_t ocStbHostSoftwareApplicationInfoGroupBridge::Get_ocStbHostSoftwareApplicationInfoSigLastReceivedTime ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostSoftwareApplicationInfoSigLastReceivedTime" << endl;
  return 0;
}


/* 
ocStbHostSoftwareApplicationInfoSigLastReadStatus OBJECT-TYPE
    SYNTAX      INTEGER
                {
                unknown(0),
                okay(1),
                error(2),
                okayButRejected(3)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Indicates the status of the last attempted read of the
            XAIT from the network that was not the same version as
            already in use except as noted below. Reports unknown(0)
            if no XAIT has been received. A value of okayButRejected
            means it was read without errors, but the monitor application
            has rejected it.
              - This must be updated ONLY when an XAIT is received
                from the network (e.g., not when loaded from persistent
                storage).
              - This must NOT be updated if an XAIT is received that has
                a version equal to the currently in-use version, UNLESS
                the in-use XAIT was loaded from persistent storage AND
                it is the FIRST time receiving an XAIT from the network
                without error.
              - The value of unknown(0) must be the initial value before
                any XAIT is ever received from the network.
              - This must NOT be set to unknown(0) if it has ever been set
                to another value."
    ::= { ocStbHostSoftwareApplicationInfo 3 }
*/ 
int ocStbHostSoftwareApplicationInfoGroupBridge::Get_ocStbHostSoftwareApplicationInfoSigLastReadStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostSoftwareApplicationInfoSigLastReadStatus" << endl;
  return 0;
}


/* 
ocStbHostSoftwareApplicationInfoSigLastNetworkVersionRead OBJECT-TYPE
    SYNTAX      Integer32 (-1 | 0..31)
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The version number of the XAIT that was last read
            successfully from the network, but not necessarily
            accepted by the monitor app. (LastReadStatus of okay
            or okayButRejected.) The version within the XAIT is
            a 5-bit unsigned integer (range 0..31), but if no XAIT
            has been read from the network, the value reported must
            be -1. (NOTE: Technically, each section has its own
            version number; however, they are required to all be the
            same, so this may be extracted from the first section.)
            If this is -1 and VersionInUse is not -1, it can be
            inferred that the version in use was loaded from
            persistent storage; however, if both versions are the
            same, it cannot be inferred that the initially used XAIT
            did not come from persistent storage."
    ::= { ocStbHostSoftwareApplicationInfo 4 }
*/ 
int ocStbHostSoftwareApplicationInfoGroupBridge::Get_ocStbHostSoftwareApplicationInfoSigLastNetworkVersionRead ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostSoftwareApplicationInfoSigLastNetworkVersionRead" << endl;
  return 0;
}


/* 
ocStbHostSoftwareApplicationInfoSigVersionInUse OBJECT-TYPE
    SYNTAX      Integer32 (-1 | 0..31)
    MAX-ACCESS read-only
     STATUS      current
     DESCRIPTION
             "The version number of the XAIT that is in use, which may
             have been read from the network or from persistent storage.
             The version within the XAIT is a 5-bit unsigned integer
             (range 0..31), but if no XAIT has been read from the network,
             the value reported must be -1. (NOTE: Technically, each
             section has its own version number; however, they are required
             to all be the same, so this may be extracted from the first
             section.) If this is not -1 and LastVersionHeard is -1, it
             can be inferred that the version in use was loaded from
             persistent storage; however, if both versions are the
             same, it cannot be inferred that the initially used XAIT
             did not come from persistent storage."
     ::= { ocStbHostSoftwareApplicationInfo 5 }
*/ 
int ocStbHostSoftwareApplicationInfoGroupBridge::Get_ocStbHostSoftwareApplicationInfoSigVersionInUse ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostSoftwareApplicationInfoSigVersionInUse" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostSoftwareApplicationInfoEntryBridge for bridging elements in the ocStbHostSoftwareApplicationInfoTable
 *
\*/

/* 
ocStbHostSoftwareApplicationInfoTable OBJECT-TYPE
    SYNTAX      SEQUENCE OF OcStbHostSoftwareApplicationInfoEntry
    MAX-ACCESS not-accessible
    STATUS      current
    DESCRIPTION
            "This table allows the manager to display and retrieve
            self-diagnostic software information of all available
            applications."
    ::= { ocStbHostSoftwareApplicationInfo 1 }

INDEX { ocStbHostSoftwareApplicationInfoIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostSoftwareApplicationInfoEntryBridge::ocStbHostSoftwareApplicationInfoEntryBridge (unsigned long ocStbHostSoftwareApplicationInfoIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ocStbHostSoftwareApplicationInfoTable,
                      BcmObjectId(ocStbHostSoftwareApplicationInfoIndex),
                      pAgent)
{
}

ocStbHostSoftwareApplicationInfoEntryBridge::~ocStbHostSoftwareApplicationInfoEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostSoftwareAppNameString OBJECT-TYPE
    SYNTAX      SnmpAdminString
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The application€s name string."
    ::= { ocStbHostSoftwareApplicationInfoEntry 1 }
*/ 
int ocStbHostSoftwareApplicationInfoEntryBridge::Get_ocStbHostSoftwareAppNameString (BcmString &ocStbHostSoftwareAppNameString)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostSoftwareAppNameString" << endl;
  ocStbHostSoftwareAppNameString = "(unfinished)";

  return ocStbHostSoftwareAppNameString.length();
}


/* 
ocStbHostSoftwareAppVersionNumber OBJECT-TYPE
    SYNTAX      SnmpAdminString
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The application€s version number."
    ::= { ocStbHostSoftwareApplicationInfoEntry 2 }
*/ 
int ocStbHostSoftwareApplicationInfoEntryBridge::Get_ocStbHostSoftwareAppVersionNumber (BcmString &ocStbHostSoftwareAppVersionNumber)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostSoftwareAppVersionNumber" << endl;
  ocStbHostSoftwareAppVersionNumber = "(unfinished)";

  return ocStbHostSoftwareAppVersionNumber.length();
}


/* 
ocStbHostSoftwareStatus OBJECT-TYPE
    SYNTAX      INTEGER
                {
                loaded(4),
                notLoaded(5),
                paused(6),
                running(7),
                destroyed(8)
                }
     MAX-ACCESS read-only
     STATUS     current
     DESCRIPTION
             "The software status of the application."
     ::= { ocStbHostSoftwareApplicationInfoEntry 3 }
*/ 
int ocStbHostSoftwareApplicationInfoEntryBridge::Get_ocStbHostSoftwareStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostSoftwareStatus" << endl;
  return 0;
}


/* 
ocStbHostSoftwareApplicationInfoIndex OBJECT-TYPE
    SYNTAX      Unsigned32 (1..4294967295)
    MAX-ACCESS not-accessible
    STATUS      current
    DESCRIPTION
            "Table Index added in this position (4) because it
            replaced an external index."
    ::= { ocStbHostSoftwareApplicationInfoEntry 4 }
*/ 
unsigned int ocStbHostSoftwareApplicationInfoEntryBridge::Get_ocStbHostSoftwareApplicationInfoIndex ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostSoftwareApplicationInfoIndex" << endl;
  return 0;
}


/* 
ocStbHostSoftwareOrganizationId OBJECT-TYPE
    SYNTAX      OCTET STRING (SIZE(4))
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Organization ID captured in the XAIT or AIT."
    ::= { ocStbHostSoftwareApplicationInfoEntry 5 }
*/ 
int ocStbHostSoftwareApplicationInfoEntryBridge::Get_ocStbHostSoftwareOrganizationId (BcmString &ocStbHostSoftwareOrganizationId)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostSoftwareOrganizationId" << endl;
  ocStbHostSoftwareOrganizationId = "(unfinished)";

  return ocStbHostSoftwareOrganizationId.length();
}


/* 
ocStbHostSoftwareApplicationId OBJECT-TYPE
    SYNTAX      OCTET STRING (SIZE(2))
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This 16 bit field uniquely identifies the
            application function as assigned by the
            responsible/owning organization identified in
            ocStbHostSoftwareOrganizationId."
    ::= { ocStbHostSoftwareApplicationInfoEntry 6 }
*/ 
int ocStbHostSoftwareApplicationInfoEntryBridge::Get_ocStbHostSoftwareApplicationId (BcmString &ocStbHostSoftwareApplicationId)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostSoftwareApplicationId" << endl;
  ocStbHostSoftwareApplicationId = "(unfinished)";

  return ocStbHostSoftwareApplicationId.length();
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostPowerGroupBridge for bridging scalars from the ocStbHostPowerGroup group 
 *
\*/

/* 
ocStbHostPower   OBJECT IDENTIFIER ::= { ocStbHostStatus 1 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostPowerGroupBridge::ocStbHostPowerGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_ocStbHostPowerGroup, pAgent)
{
}

ocStbHostPowerGroupBridge::~ocStbHostPowerGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostPowerStatus OBJECT-TYPE
    SYNTAX      INTEGER
                {
                powerOn(1),
                standby(2)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Indicates the current power status of the ocStbHost."
    ::= { ocStbHostPower 1 }
*/ 
int ocStbHostPowerGroupBridge::Get_ocStbHostPowerStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostPowerStatus" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostSystemMemoryReportEntryBridge for bridging elements in the ocStbHostSystemMemoryReportTable
 *
\*/

/* 
ocStbHostSystemMemoryReportTable OBJECT-TYPE
    SYNTAX      SEQUENCE OF OcStbHostSystemMemoryReportEntry
    MAX-ACCESS not-accessible
    STATUS      current
    DESCRIPTION
            "This table contains the Id of each memory type
            reported by the ocStbHost."
    ::= { ocStbHostSystemObjects 3 }

INDEX { ocStbHostSystemMemoryReportIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostSystemMemoryReportEntryBridge::ocStbHostSystemMemoryReportEntryBridge (unsigned long ocStbHostSystemMemoryReportIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ocStbHostSystemMemoryReportTable,
                      BcmObjectId(ocStbHostSystemMemoryReportIndex),
                      pAgent)
{
}

ocStbHostSystemMemoryReportEntryBridge::~ocStbHostSystemMemoryReportEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostSystemMemoryReportIndex OBJECT-TYPE
    SYNTAX      Unsigned32(1..4294967295)
    MAX-ACCESS not-accessible
    STATUS      current
    DESCRIPTION
            "The identifier of the memory type."
    ::= { ocStbHostSystemMemoryReportEntry 1 }
*/ 
unsigned int ocStbHostSystemMemoryReportEntryBridge::Get_ocStbHostSystemMemoryReportIndex ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostSystemMemoryReportIndex" << endl;
  return 0;
}


/* 
ocStbHostSystemMemoryReportMemoryType OBJECT-TYPE
    SYNTAX      INTEGER
                {
                rom(1),
                dram(2),
                sram(3),
                flash(4),
                nvm(5),
                videomemory(7),
                othermemory(8),
                reserved(9),
                internalHardDrive(10),
                externalHardDrive(11),
                opticalMedia(12)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Designates the type of memory that is being reported."
    ::= { ocStbHostSystemMemoryReportEntry 2 }
*/ 
int ocStbHostSystemMemoryReportEntryBridge::Get_ocStbHostSystemMemoryReportMemoryType ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostSystemMemoryReportMemoryType" << endl;
  return 0;
}


/* 
ocStbHostSystemMemoryReportMemorySize OBJECT-TYPE
    SYNTAX      Integer32
    UNITS       "kilobytes"
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Designates the physical size of the reported memory type.
            The units are kilobytes, defined to be 1,024 bytes."
    ::= { ocStbHostSystemMemoryReportEntry 3 }
*/ 
int ocStbHostSystemMemoryReportEntryBridge::Get_ocStbHostSystemMemoryReportMemorySize ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostSystemMemoryReportMemorySize" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbCardInfoGroupBridge for bridging scalars from the ocStbCardInfoGroup group 
 *
\*/

/* 
ocStbCardInfo OBJECT IDENTIFIER ::= { ocStbHostStatus 4 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbCardInfoGroupBridge::ocStbCardInfoGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_ocStbCardInfoGroup, pAgent)
{
}

ocStbCardInfoGroupBridge::~ocStbCardInfoGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostCardMacAddress OBJECT-TYPE
    SYNTAX      MacAddress
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The Mac Address associated with the Card."
    ::= { ocStbCardInfo 1 }
*/ 
void ocStbCardInfoGroupBridge::Get_ocStbHostCardMacAddress (BcmMacAddress &ocStbHostCardMacAddress)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCardMacAddress" << endl;
  ocStbHostCardMacAddress.Set (0,0,0,0,0,0);
}


/* 
ocStbHostCardIpAddressType OBJECT-TYPE
    SYNTAX      InetAddressType
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The network address Type of ocStbHostCardIpAddress.
            If the Host does not have the Card€s IP address (e.g.,
            the Card has not opened an IP Unicast Flow, which would
            require the Host to obtain the Card€s IP address, thus
            providing the Host with knowledge of the Card€s IP address
            type), then the Host replies with a value of unknown (0)."
    ::= { ocStbCardInfo 2 }
*/ 
int ocStbCardInfoGroupBridge::Get_ocStbHostCardIpAddressType ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCardIpAddressType" << endl;
  return 0;
}


/* 
ocStbHostCardIpAddress OBJECT-TYPE
    SYNTAX      InetAddress
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The network address of the Card. If the Host does not
            have the Card€s IP address (e.g., the Card has not opened
            an IP Unicast Flow, which would require the Host to obtain
            the Card€s IP address, thus providing the Host with
            knowledge of the Card€s IP address), then the Host replies
            with a value of zero (00)."
    ::= { ocStbCardInfo 3 }
*/ 
int ocStbCardInfoGroupBridge::Get_ocStbHostCardIpAddress (BcmString &ocStbHostCardIpAddress)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCardIpAddress" << endl;
  ocStbHostCardIpAddress = "(unfinished)";

  return ocStbHostCardIpAddress.length();
}


/* 
ocStbHostCardId OBJECT-TYPE
    SYNTAX      DisplayString (SIZE(17))
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Allows the manager to retrieve the 40 least-significant
            Bits of the CableCARD ID in the string
            format indicated below:

            M-MMU-UUU-UUU-UUL

            Where:

            M-MM is the decimal representation of the
            10-bit CHICA assigned manufacturer number.

            U-UUU-UUU-UU is the decimal representation
            of the 30-bit manufacturer assigned unit number.

            L is a Luhn check digit calculated over the
            preceding 12 decimal digits.

             If the Card ID is unknown, then the string shall
             be all zeros e.g., '0-000-000-000-000'."
     ::= { ocStbCardInfo 7 }
*/ 
int ocStbCardInfoGroupBridge::Get_ocStbHostCardId (BcmString &ocStbHostCardId)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCardId" << endl;
  ocStbHostCardId = "(unfinished)";

  return ocStbHostCardId.length();
}


/* 
ocStbHostCardBindingStatus OBJECT-TYPE
    SYNTAX      INTEGER {
                unknown(1),
                invalidCertificate(2),
                otherAuthFailure(3),
                bound(4)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The status of CableCARD/Host mutual authentication
            and binding.

             €unknown€ means the binding state has not been determined.
                  For example, mutual authentication may not have
                  commenced or may be in process.
             €invalidCertificate€ means the Host failed to establish
                  device certificate authenticity.
             €otherAuthFailure€ corresponds to other failures in
                  establishing mutual authentication.
             €bound€ indicates that mutual authentication has succeeded
                  and CableCARD and Host are bound successfully."
     ::= { ocStbCardInfo 8 }
*/ 
int ocStbCardInfoGroupBridge::Get_ocStbHostCardBindingStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCardBindingStatus" << endl;
  return 0;
}


/* 
ocStbHostCardOpenedGenericResource OBJECT-TYPE
    SYNTAX      OCTET STRING (SIZE (4))
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The hexadecimal representation of the generic feature
            control resource number opened using a profile
            reply() APDU resource_identifier field. The
            possible values are defined in the [CCIF]
            specification."
    ::= { ocStbCardInfo 9 }
*/ 
int ocStbCardInfoGroupBridge::Get_ocStbHostCardOpenedGenericResource (BcmString &ocStbHostCardOpenedGenericResource)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCardOpenedGenericResource" << endl;
  ocStbHostCardOpenedGenericResource = "(unfinished)";

  return ocStbHostCardOpenedGenericResource.length();
}


/* 
ocStbHostCardTimeZoneOffset OBJECT-TYPE
    SYNTAX      INTEGER(-12..12)
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Decimal value of the UTC offset taken from the
            feature_parameters() APDU for feature_id =5 where
            possible values are defined in the [CCIF]
            specification."
    ::= { ocStbCardInfo 10 }
*/ 
int ocStbCardInfoGroupBridge::Get_ocStbHostCardTimeZoneOffset ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCardTimeZoneOffset" << endl;
  return 0;
}


/* 
ocStbHostCardDaylightSavingsTimeDelta OBJECT-TYPE
    SYNTAX      OCTET STRING (SIZE (1))
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Decimal value of the daylight savings delta.
            A 0x00 should be returned if the Card does not
            support this feature or if there is no delta.
            The ambiguity may be resolved by querying
            ocStbHostCardDaylightSavingsTimeEntry object."
    ::= { ocStbCardInfo 11 }
*/ 
int ocStbCardInfoGroupBridge::Get_ocStbHostCardDaylightSavingsTimeDelta (BcmString &ocStbHostCardDaylightSavingsTimeDelta)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCardDaylightSavingsTimeDelta" << endl;
  ocStbHostCardDaylightSavingsTimeDelta = "(unfinished)";

  return ocStbHostCardDaylightSavingsTimeDelta.length();
}


/* 
ocStbHostCardDaylightSavingsTimeEntry OBJECT-TYPE
    SYNTAX      Unsigned32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Daylight savings entry time given as time lapsed
            since 12 AM Jan 6, 1980, in units of GPS seconds.
            All zeros should be returned if the Card does not
            support this feature."
    ::= { ocStbCardInfo 12 }
*/ 
unsigned int ocStbCardInfoGroupBridge::Get_ocStbHostCardDaylightSavingsTimeEntry ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCardDaylightSavingsTimeEntry" << endl;
  return 0;
}


/* 
ocStbHostCardDaylightSavingsTimeExit OBJECT-TYPE
    SYNTAX      Unsigned32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Daylight savings exit time given as time lapsed
            since 12 AM Jan 6, 1980, in units of GPS seconds.
            All zeros should be returned if the Card does not
            support this feature."
    ::= { ocStbCardInfo 13 }
*/ 
unsigned int ocStbCardInfoGroupBridge::Get_ocStbHostCardDaylightSavingsTimeExit ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCardDaylightSavingsTimeExit" << endl;
  return 0;
}


/* 
ocStbHostCardEaLocationCode OBJECT-TYPE
    SYNTAX      OCTET STRING (SIZE(3))
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Contains the EA location codes as defined
            in American National Standard, J-STD-042.
            The bits corresponding to ANSI EA location codes
            are indicated below:

             SSSSSSSS-DDDD-xx-CCCCCCCCCC

             Where:

             SSSSSSSS is An 8-bit unsigned number in the
             range 0 to 99 that represents the State or
             Territory affected by the emergency alert.

             DDDD is A 4-bit number in the range 0 to 9
             that defines county subdivisions.

            xx is reserved

            CCCCCCCCCC is 10-bit number between 0 to 999
            that identifies a county within a state.

             If the EA_Location code is unknown, then the string shall
             be all zeros e.g., '000000'."
     ::= { ocStbCardInfo 14 }
*/ 
int ocStbCardInfoGroupBridge::Get_ocStbHostCardEaLocationCode (BcmString &ocStbHostCardEaLocationCode)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCardEaLocationCode" << endl;
  ocStbHostCardEaLocationCode = "(unfinished)";

  return ocStbHostCardEaLocationCode.length();
}


/* 
ocStbHostCardVctId OBJECT-TYPE
    SYNTAX      OCTET STRING (SIZE(2))
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Display the VTC-ID if supported. If not supported
            then return 0x0000."
    ::= { ocStbCardInfo 15 }
*/ 
int ocStbCardInfoGroupBridge::Get_ocStbHostCardVctId (BcmString &ocStbHostCardVctId)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCardVctId" << endl;
  ocStbHostCardVctId = "(unfinished)";

  return ocStbHostCardVctId.length();
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostCCAppInfoEntryBridge for bridging elements in the ocStbHostCCAppInfoTable
 *
\*/

/* 
ocStbHostCCAppInfoTable OBJECT-TYPE
    SYNTAX      SEQUENCE OF OcStbHostCCAppInfoEntry
    MAX-ACCESS not-accessible
    STATUS      current
    DESCRIPTION
            "This table allows the manager to retrieve
            the HTML pages for Card applications retrieved by the
            Host using the Application Info resource."
    ::= { ocStbHostCCApplications 1 }

INDEX { ocStbHostCCApplicationType }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostCCAppInfoEntryBridge::ocStbHostCCAppInfoEntryBridge (unsigned long ocStbHostCCApplicationType, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_ocStbHostCCAppInfoTable,
                      BcmObjectId(ocStbHostCCApplicationType),
                      pAgent)
{
}

ocStbHostCCAppInfoEntryBridge::~ocStbHostCCAppInfoEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostCCAppInfoIndex OBJECT-TYPE
    SYNTAX      Unsigned32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object was the index for this table
            in a previous version of the MIB.
            Currently used only as a placeholder to
            avoid changing OID values."
    ::= { ocStbHostCCAppInfoEntry 1 }
*/ 
unsigned int ocStbHostCCAppInfoEntryBridge::Get_ocStbHostCCAppInfoIndex ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCCAppInfoIndex" << endl;
  return 0;
}


/* 
ocStbHostCCApplicationType OBJECT-TYPE
    SYNTAX      Unsigned32(0..4294967295)
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Index for this table.
            Contains the application_type field
            in the application_info_cnf() APDU."
    ::= { ocStbHostCCAppInfoEntry 2 }
*/ 
unsigned int ocStbHostCCAppInfoEntryBridge::Get_ocStbHostCCApplicationType ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCCApplicationType" << endl;
  return 0;
}


/* 
ocStbHostCCApplicationName OBJECT-TYPE
    SYNTAX      SnmpAdminString
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Contains the application name."
    ::= { ocStbHostCCAppInfoEntry 3 }
*/ 
int ocStbHostCCAppInfoEntryBridge::Get_ocStbHostCCApplicationName (BcmString &ocStbHostCCApplicationName)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCCApplicationName" << endl;
  ocStbHostCCApplicationName = "(unfinished)";

  return ocStbHostCCApplicationName.length();
}


/* 
ocStbHostCCApplicationVersion OBJECT-TYPE
    SYNTAX      Unsigned32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Contains the application version."
    ::= { ocStbHostCCAppInfoEntry 4 }
*/ 
unsigned int ocStbHostCCAppInfoEntryBridge::Get_ocStbHostCCApplicationVersion ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCCApplicationVersion" << endl;
  return 0;
}


/* 
ocStbHostCCAppInfoPage OBJECT-TYPE
    SYNTAX      OCTET STRING
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "All HTML data for this application.
            If a 'page' as reported from the Card contains
            links, the links shall be traversed, and all
            HTML from each page traversed shall be included
            in a single, valid HTML string (with per-page
            start/end HTML and BODY tags removed). The resulting
            HTML SHOULD be structured as an HTML table
            consisting of one column, with one row for each
            'page' from the Card. If the full HTML from the
            application (all pages concatenated) will not fit
            within the SNMP buffer, text indicating this
            SHALL follow the table; otherwise nothing except
            the close BODY and HTML tags SHOULD follow the table."
    ::= { ocStbHostCCAppInfoEntry 5 }
*/ 
int ocStbHostCCAppInfoEntryBridge::Get_ocStbHostCCAppInfoPage (BcmString &ocStbHostCCAppInfoPage)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCCAppInfoPage" << endl;
  ocStbHostCCAppInfoPage = "(unfinished)";

  return ocStbHostCCAppInfoPage.length();
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostSnmpProxyInfoGroupBridge for bridging scalars from the ocStbHostSnmpProxyInfoGroup group 
 *
\*/

/* 
ocStbHostSnmpProxyInfo OBJECT IDENTIFIER ::= { ocStbCardInfo 6 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostSnmpProxyInfoGroupBridge::ocStbHostSnmpProxyInfoGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_ocStbHostSnmpProxyInfoGroup, pAgent)
{
}

ocStbHostSnmpProxyInfoGroupBridge::~ocStbHostSnmpProxyInfoGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostCardMfgId OBJECT-TYPE
    SYNTAX      OCTET STRING (SIZE(2))
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The two-byte manufacturer identifier
            acquired from the CableCard. The high order byte
            defines the manufacturer as follows.
                00xx = Motorola
                01xx = Cisco (Scientific-Atlanta)
                02xx = NDS
                03xx = Conax
            This can be used with version and root OID to enable
            SNMP access to proprietary CC diagnostic information.
            The lower order octet can be used to privately identify
            product generation and derivatives."
    ::= { ocStbHostSnmpProxyInfo 1 }
*/ 
int ocStbHostSnmpProxyInfoGroupBridge::Get_ocStbHostCardMfgId (BcmString &ocStbHostCardMfgId)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCardMfgId" << endl;
  ocStbHostCardMfgId = "(unfinished)";

  return ocStbHostCardMfgId.length();
}


/* 
ocStbHostCardVersion OBJECT-TYPE
    SYNTAX      OCTET STRING (SIZE (2))
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The version of the CableCard. This can be used with
            manufacturer and root OID to enable SNMP access to
            proprietary diagnostic information."
    ::= { ocStbHostSnmpProxyInfo 2 }
*/ 
int ocStbHostSnmpProxyInfoGroupBridge::Get_ocStbHostCardVersion (BcmString &ocStbHostCardVersion)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCardVersion" << endl;
  ocStbHostCardVersion = "(unfinished)";

  return ocStbHostCardVersion.length();
}


/* 
ocStbHostCardRootOid OBJECT-TYPE
    SYNTAX      OBJECT IDENTIFIER
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "An ASN.1 encoding of the root OID of the
            CableCARD MIB defined in the CC Manufacturers private
            MIB name space. This OID will be compared against each
            object identifier in the VARBIND contained within the
            GetRequests, GetNextRequests, GetBulkRequests, and
            SetRequests received by the Host2.x if the objects are not
            in the Host2.x MIB. A match of OIDs root requires the Host
            to send a corresponding request to the Card. A 0.0 value
            explicitly states the Card does not support SNMP message
            processing."
    ::= { ocStbHostSnmpProxyInfo 3 }
*/ 
int ocStbHostSnmpProxyInfoGroupBridge::Get_ocStbHostCardRootOid (BcmObjectId &ocStbHostCardRootOid)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCardRootOid" << endl;
  ocStbHostCardRootOid = "0.0";

  return ocStbHostCardRootOid.Length();
}


/* 
ocStbHostCardSerialNumber OBJECT-TYPE
    SYNTAX      SnmpAdminString
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Serial Number of CableCARD."
    ::= { ocStbHostSnmpProxyInfo 4 }
*/ 
int ocStbHostSnmpProxyInfoGroupBridge::Get_ocStbHostCardSerialNumber (BcmString &ocStbHostCardSerialNumber)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCardSerialNumber" << endl;
  ocStbHostCardSerialNumber = "(unfinished)";

  return ocStbHostCardSerialNumber.length();
}


/* 
ocStbHostCardSnmpAccessControl OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-write
    STATUS      current
    DESCRIPTION
            "This object allows the manager to control access to the
            Card's MIB via networked devices (e.g., get/set by network
            attached managers). It has no impact on local
            access (e.g., application get/set of Card MIB objects).

             When set to TRUE, the Host is permitted to forward
             SNMP messages received on the WAN/LAN interface addressed
             to the OID of the Card subtree.

             When set to FALSE, the Host must discard and not forward
            to the Card any SNMP messages received on the WAN/LAN
            interface addressed to the OID of the Card subtree.

             Default of this object is TRUE."
     DEFVAL { 1 }
     ::= { ocStbHostSnmpProxyInfo 5 }
*/ 
bool ocStbHostSnmpProxyInfoGroupBridge::Get_ocStbHostCardSnmpAccessControl ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCardSnmpAccessControl" << endl;
  return true;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/

/* 
ocStbHostCardSnmpAccessControl OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-write
    STATUS      current
    DESCRIPTION
            "This object allows the manager to control access to the
            Card's MIB via networked devices (e.g., get/set by network
            attached managers). It has no impact on local
            access (e.g., application get/set of Card MIB objects).

             When set to TRUE, the Host is permitted to forward
             SNMP messages received on the WAN/LAN interface addressed
             to the OID of the Card subtree.

             When set to FALSE, the Host must discard and not forward
            to the Card any SNMP messages received on the WAN/LAN
            interface addressed to the OID of the Card subtree.

             Default of this object is TRUE."
     DEFVAL { 1 }
     ::= { ocStbHostSnmpProxyInfo 5 }
*/ 
SNMP_STATUS ocStbHostSnmpProxyInfoGroupBridge::Set_ocStbHostCardSnmpAccessControl (bool ocStbHostCardSnmpAccessControl)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;

  {
    if ((ocStbHostCardSnmpAccessControl != true) && (ocStbHostCardSnmpAccessControl != false))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ocStbHostCardSnmpAccessControl" << endl;
  }

  return Status;
}






/*\
 *
 *  ocStbHostCardCpInfoGroupBridge for bridging scalars from the ocStbHostCardCpInfoGroup group 
 *
\*/

/* 
ocStbHostCardCpInfo OBJECT IDENTIFIER ::= { ocStbCardInfo 16 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostCardCpInfoGroupBridge::ocStbHostCardCpInfoGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_ocStbHostCardCpInfoGroup, pAgent)
{
}

ocStbHostCardCpInfoGroupBridge::~ocStbHostCardCpInfoGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostCardCpAuthKeyStatus OBJECT-TYPE
    SYNTAX      INTEGER {
                ready(1),
                notReady(2)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This parameter SHALL be set to €Ready€ if the
            Card has requested the host€s authentication
            key via the APDU CP_data_req() with a
            Datatype_id = 22. This parameter SHALL be set
            to €Not Ready€ if the Cablecard has NOT
            requested the authentication key."
    ::= { ocStbHostCardCpInfo 1 }
*/ 
int ocStbHostCardCpInfoGroupBridge::Get_ocStbHostCardCpAuthKeyStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCardCpAuthKeyStatus" << endl;
  return 0;
}


/* 
ocStbHostCardCpCertificateCheck OBJECT-TYPE
    SYNTAX      INTEGER {
                ok(1),
                failed(2)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Display the results of the Card_DevCert verification
            as defined in the OpenCable CableCard Copy Protection
            Specification."
    ::= { ocStbHostCardCpInfo 2 }
*/ 
int ocStbHostCardCpInfoGroupBridge::Get_ocStbHostCardCpCertificateCheck ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCardCpCertificateCheck" << endl;
  return 0;
}


/* 
ocStbHostCardCpCciChallengeCount OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Display the Card CCI Challenge message
            count [CCCP] since the
            last boot or reset."
    ::= { ocStbHostCardCpInfo 3 }
*/ 
unsigned int ocStbHostCardCpInfoGroupBridge::Get_ocStbHostCardCpCciChallengeCount ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCardCpCciChallengeCount" << endl;
  return 0;
}


/* 
ocStbHostCardCpKeyGenerationReqCount OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Display the Copy Protection Key Generation
            Request count [CCCP] since
            the last boot or reset."
    ::= { ocStbHostCardCpInfo 4 }
*/ 
unsigned int ocStbHostCardCpInfoGroupBridge::Get_ocStbHostCardCpKeyGenerationReqCount ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCardCpKeyGenerationReqCount" << endl;
  return 0;
}


/* 
ocStbHostCardCpIdList OBJECT-TYPE
    SYNTAX      OCTET STRING (SIZE(4))
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Display the CP_system_id_bitmask field of the
            APDU CP_open_cnf() as defined in the CableCARD Copy
            Protection Specification Section 11.3.1.2."
    ::= { ocStbHostCardCpInfo 5 }
*/ 
int ocStbHostCardCpInfoGroupBridge::Get_ocStbHostCardCpIdList (BcmString &ocStbHostCardCpIdList)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCardCpIdList" << endl;
  ocStbHostCardCpIdList = "(unfinished)";

  return ocStbHostCardCpIdList.length();
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostInfoGroupBridge for bridging scalars from the ocStbHostInfoGroup group 
 *
\*/

/* 
ocStbHostInfo OBJECT IDENTIFIER ::= { ocStbHostStatus 5 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostInfoGroupBridge::ocStbHostInfoGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_ocStbHostInfoGroup, pAgent)
{
}

ocStbHostInfoGroupBridge::~ocStbHostInfoGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostIpAddressType OBJECT-TYPE
    SYNTAX      InetAddressType
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "This object was the address type for ocStbHostIpAddress,
            which was never implemented in a previous version of the
            MIB. Currently used only as a placeholder to avoid
            changing OID values."
    ::= { ocStbHostInfo 1 }
*/ 
int ocStbHostInfoGroupBridge::Get_ocStbHostIpAddressType ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostIpAddressType" << endl;
  return 0;
}


/* 
ocStbHostIpSubNetMask OBJECT-TYPE
    SYNTAX      InetAddress
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The IP subnet mask of the Host address."
    ::= { ocStbHostInfo 2 }
*/ 
int ocStbHostInfoGroupBridge::Get_ocStbHostIpSubNetMask (BcmString &ocStbHostIpSubNetMask)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostIpSubNetMask" << endl;
  ocStbHostIpSubNetMask = "(unfinished)";

  return ocStbHostIpSubNetMask.length();
}


/* 
ocStbHostOobMessageMode OBJECT-TYPE
    SYNTAX      INTEGER
                {
                scte55(1),
                dsg(2),
                other(3)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Out of Band Messaging communications mode currently
            employed by the Host2.x."
    ::= { ocStbHostInfo 3 }
*/ 
int ocStbHostInfoGroupBridge::Get_ocStbHostOobMessageMode ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostOobMessageMode" << endl;
  return 0;
}


/* 
ocStbHostBootStatus OBJECT-TYPE
    SYNTAX      INTEGER
                {
                completedSuccessfully(1),
                completeWithErrors(2),
                inProgressWithCodeDownload(3),
                inProgressNoCodeDownload(4),
                inProgressAwaitingMonitorApp(5),
                unknown(6)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "Defines the current status of the host during or after
            boot process."
    ::= { ocStbHostInfo 6 }
*/ 
int ocStbHostInfoGroupBridge::Get_ocStbHostBootStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostBootStatus" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostDumpTrapInfoGroupBridge for bridging scalars from the ocStbHostDumpTrapInfoGroup group 
 *
\*/

/* 
ocStbHostDumpTrapInfo OBJECT IDENTIFIER ::= { ocStbHostInfo 4 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostDumpTrapInfoGroupBridge::ocStbHostDumpTrapInfoGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_ocStbHostDumpTrapInfoGroup, pAgent)
{
}

ocStbHostDumpTrapInfoGroupBridge::~ocStbHostDumpTrapInfoGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostDumpEventCount OBJECT-TYPE
    SYNTAX      INTEGER(0..32)
    MAX-ACCESS read-write
    STATUS      current
    DESCRIPTION
            "The number of times the host should notify the headend(s)
            of a dump condition (e.g., stack dumps after boot).
            When an exception occurs, if ocStbHostDumpEventCount is
            non-zero, the host will send the notification to each
            defined notification receiver and decrement
            ocStbHostDumpEventCount. A non-zero count arms the trap.
            No trap will be sent if the value is 0."
    DEFVAL { 0 }
    ::= { ocStbHostDumpTrapInfo 1 }
*/ 
int ocStbHostDumpTrapInfoGroupBridge::Get_ocStbHostDumpEventCount ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDumpEventCount" << endl;
  return 0;
}


/* 
ocStbHostDumpNow OBJECT-TYPE
    SYNTAX      TruthValue
     MAX-ACCESS read-write
     STATUS      current
     DESCRIPTION
             "True(1) forces an immediate dump of a component of the
             Host. The Host will prepare the dump file then send the
             Notification-PDU to the headend. This simulates
             the manufacturer-defined event that would cause a dump.
             Reading this object returns false(2)."
     ::= { ocStbHostDumpTrapInfo 2 }
*/ 
bool ocStbHostDumpTrapInfoGroupBridge::Get_ocStbHostDumpNow ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDumpNow" << endl;
  return true;
}


/* 
ocStbHostDumpEventTimeout OBJECT-TYPE
    SYNTAX      Unsigned32(1..120)
    MAX-ACCESS read-write
    STATUS      current
    DESCRIPTION
            "The time, in seconds, after a SNMPv2-Trap-PDU as been sent
            to the headend that the Host will wait before abandoning
            the dump transfer. If the Host does not receive a TFTP Get
            from the headend within this timeout period, it will
            proceed with the manufacturer-defined recovery process
            from the condition that caused the dump. The intent is to
            prevent the Host from waiting indefinitely for a TFTP
            session to enable the dump transfer. The timer should be
            ignored if the Host receives a TFTP GET from the headend
            within the timeout period. The default timeout value
            is 5 seconds."
    DEFVAL { 5 }
    ::= { ocStbHostDumpTrapInfo 3 }
*/ 
unsigned int ocStbHostDumpTrapInfoGroupBridge::Get_ocStbHostDumpEventTimeout ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDumpEventTimeout" << endl;
  return 0;
}


/* 
ocStbHostDumpFilePath OBJECT-TYPE
SYNTAX      SnmpAdminString
MAX-ACCESS read-only
STATUS      current
DESCRIPTION
        "Dump file path and name."
    ::= { ocStbHostDumpTrapInfo 4 }
*/ 
int ocStbHostDumpTrapInfoGroupBridge::Get_ocStbHostDumpFilePath (BcmString &ocStbHostDumpFilePath)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostDumpFilePath" << endl;
  ocStbHostDumpFilePath = "(unfinished)";

  return ocStbHostDumpFilePath.length();
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/

/* 
ocStbHostDumpEventCount OBJECT-TYPE
    SYNTAX      INTEGER(0..32)
    MAX-ACCESS read-write
    STATUS      current
    DESCRIPTION
            "The number of times the host should notify the headend(s)
            of a dump condition (e.g., stack dumps after boot).
            When an exception occurs, if ocStbHostDumpEventCount is
            non-zero, the host will send the notification to each
            defined notification receiver and decrement
            ocStbHostDumpEventCount. A non-zero count arms the trap.
            No trap will be sent if the value is 0."
    DEFVAL { 0 }
    ::= { ocStbHostDumpTrapInfo 1 }
*/ 
SNMP_STATUS ocStbHostDumpTrapInfoGroupBridge::Set_ocStbHostDumpEventCount (int ocStbHostDumpEventCount)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (ocStbHostDumpEventCount);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ocStbHostDumpEventCount < MINVAL_OCSTBHOSTDUMPEVENTCOUNT)
    ||  (ocStbHostDumpEventCount > MAXVAL_OCSTBHOSTDUMPEVENTCOUNT))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ocStbHostDumpEventCount" << endl;
  }

  return Status;
}


/* 
ocStbHostDumpNow OBJECT-TYPE
    SYNTAX      TruthValue
     MAX-ACCESS read-write
     STATUS      current
     DESCRIPTION
             "True(1) forces an immediate dump of a component of the
             Host. The Host will prepare the dump file then send the
             Notification-PDU to the headend. This simulates
             the manufacturer-defined event that would cause a dump.
             Reading this object returns false(2)."
     ::= { ocStbHostDumpTrapInfo 2 }
*/ 
SNMP_STATUS ocStbHostDumpTrapInfoGroupBridge::Set_ocStbHostDumpNow (bool ocStbHostDumpNow)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;

  {
    if ((ocStbHostDumpNow != true) && (ocStbHostDumpNow != false))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ocStbHostDumpNow" << endl;
  }

  return Status;
}


/* 
ocStbHostDumpEventTimeout OBJECT-TYPE
    SYNTAX      Unsigned32(1..120)
    MAX-ACCESS read-write
    STATUS      current
    DESCRIPTION
            "The time, in seconds, after a SNMPv2-Trap-PDU as been sent
            to the headend that the Host will wait before abandoning
            the dump transfer. If the Host does not receive a TFTP Get
            from the headend within this timeout period, it will
            proceed with the manufacturer-defined recovery process
            from the condition that caused the dump. The intent is to
            prevent the Host from waiting indefinitely for a TFTP
            session to enable the dump transfer. The timer should be
            ignored if the Host receives a TFTP GET from the headend
            within the timeout period. The default timeout value
            is 5 seconds."
    DEFVAL { 5 }
    ::= { ocStbHostDumpTrapInfo 3 }
*/ 
SNMP_STATUS ocStbHostDumpTrapInfoGroupBridge::Set_ocStbHostDumpEventTimeout (unsigned int ocStbHostDumpEventTimeout)
{
  SNMP_STATUS Status = Uint32MibObject::TestDefaults (ocStbHostDumpEventTimeout);

  if (Status == SNMP_NO_ERROR)
  {
    if ((ocStbHostDumpEventTimeout < MINVAL_OCSTBHOSTDUMPEVENTTIMEOUT)
    ||  (ocStbHostDumpEventTimeout > MAXVAL_OCSTBHOSTDUMPEVENTTIMEOUT))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ocStbHostDumpEventTimeout" << endl;
  }

  return Status;
}






/*\
 *
 *  ocStbHostSpecificationsInfoGroupBridge for bridging scalars from the ocStbHostSpecificationsInfoGroup group 
 *
\*/

/* 
ocStbHostSpecificationsInfo OBJECT IDENTIFIER ::= { ocStbHostInfo 5 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostSpecificationsInfoGroupBridge::ocStbHostSpecificationsInfoGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_ocStbHostSpecificationsInfoGroup, pAgent)
{
}

ocStbHostSpecificationsInfoGroupBridge::~ocStbHostSpecificationsInfoGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostCfrSpecificationIssue OBJECT-TYPE
    SYNTAX      SnmpAdminString
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The Host CFR version implemented in this
            device (e.g., OC-SP-HOST2.1-CFR-I06-081114)."
    ::= { ocStbHostSpecificationsInfo 1 }
*/ 
int ocStbHostSpecificationsInfoGroupBridge::Get_ocStbHostCfrSpecificationIssue (BcmString &ocStbHostCfrSpecificationIssue)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostCfrSpecificationIssue" << endl;
  ocStbHostCfrSpecificationIssue = "(unfinished)";

  return ocStbHostCfrSpecificationIssue.length();
}


/* 
ocStbHostMibSpecificationIssue OBJECT-TYPE
    SYNTAX      SnmpAdminString
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The version implemented in this device
            (e.g., OC-SP-MIB-HOST2.X-I05-081114)."
    ::= { ocStbHostSpecificationsInfo 2 }
*/ 
int ocStbHostSpecificationsInfoGroupBridge::Get_ocStbHostMibSpecificationIssue (BcmString &ocStbHostMibSpecificationIssue)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostMibSpecificationIssue" << endl;
  ocStbHostMibSpecificationIssue = "(unfinished)";

  return ocStbHostMibSpecificationIssue.length();
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostContentErrorSummaryInfoGroupBridge for bridging scalars from the ocStbHostContentErrorSummaryInfoGroup group 
 *
\*/

/* 
ocStbHostContentErrorSummaryInfo OBJECT IDENTIFIER ::= { ocStbHostInfo 7 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostContentErrorSummaryInfoGroupBridge::ocStbHostContentErrorSummaryInfoGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_ocStbHostContentErrorSummaryInfoGroup, pAgent)
{
}

ocStbHostContentErrorSummaryInfoGroupBridge::~ocStbHostContentErrorSummaryInfoGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostPatTimeoutCount OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "These values increment every time a PAT timeout
            condition has occurred since the last boot or reset."
    ::= { ocStbHostContentErrorSummaryInfo 1 }
*/ 
unsigned int ocStbHostContentErrorSummaryInfoGroupBridge::Get_ocStbHostPatTimeoutCount ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostPatTimeoutCount" << endl;
  return 0;
}


/* 
ocStbHostPmtTimeoutCount OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "These values increment every time a PMT timeout
            condition has occurred since the last boot or reset."
    ::= { ocStbHostContentErrorSummaryInfo 2 }
*/ 
unsigned int ocStbHostContentErrorSummaryInfoGroupBridge::Get_ocStbHostPmtTimeoutCount ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostPmtTimeoutCount" << endl;
  return 0;
}


/* 
ocStbHostOobCarouselTimeoutCount OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "These values increment each time an implementation
            specific OOB Carousel timeout has occurred since the last
            boot or reset."
    ::= { ocStbHostContentErrorSummaryInfo 3 }
*/ 
unsigned int ocStbHostContentErrorSummaryInfoGroupBridge::Get_ocStbHostOobCarouselTimeoutCount ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostOobCarouselTimeoutCount" << endl;
  return 0;
}


/* 
ocStbHostInbandCarouselTimeoutCount OBJECT-TYPE
    SYNTAX      Counter32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "These values increment each time an implementation
            specific Inband Carousel timeout has occurred since the
            last boot or reset."
    ::= { ocStbHostContentErrorSummaryInfo 4 }
*/ 
unsigned int ocStbHostContentErrorSummaryInfoGroupBridge::Get_ocStbHostInbandCarouselTimeoutCount ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostInbandCarouselTimeoutCount" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostRebootInfoGroupBridge for bridging scalars from the ocStbHostRebootInfoGroup group 
 *
\*/

/* 
ocStbHostRebootInfo OBJECT IDENTIFIER ::= { ocStbHostStatus 6 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostRebootInfoGroupBridge::ocStbHostRebootInfoGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_ocStbHostRebootInfoGroup, pAgent)
{
}

ocStbHostRebootInfoGroupBridge::~ocStbHostRebootInfoGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostRebootType OBJECT-TYPE
    SYNTAX      INTEGER
                {
                unknown(0),
                davicDocsis(1),
                user(2),
                system(3),
                trap(4),
                silentWatchdog(5),
                bootloader(6),
                powerup(7),
                hostUpgrade(8),
                hardware(9),
                cablecardError(10)
                }
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
           "Describes the type of reboot that occurred the last time
           the host device rebooted.

           Unknown(0), unknown reboot. 0 is used as a sentinel value
           to denote that the reboot reporting mechanism is
           initialized and ready for use. If it appears in a reboot
           report as a reboot reason, it means that something
           (of unknown cause) prevented a reboot reason from being
           saved. This is a spontaneous reboot.

           Davic/DOCSIS(1), Davic/DOCSIS switch. The set-top rebooted
           because the network switched from Davic to DOCSIS or
           vice versa. This is a controlled reboot.

           User(2), User reboot. The set-top rebooted at the specific
           request of the user, typically the MSO, but can also be the
           subscriber. Includes, but not limited to, headend-initiated
           reboots, pass-thru and SNMP messages to reboot, new
           application versions that require a reboot in order to
           reload, the staging toolkit, hard disk re-format requests,
           CableCARD inserted or firmware upgrade, and more.
           This is a controlled reboot.

           System(3), System reboot. The set-top rebooted because its
           software determined it cannot continue operation. This can
           be due to any number of reasons, including: detection of
           logic errors, exhaustion of various system resources,
           detection of memory corruption, and more. This is a
           spontaneous reboot.

           Trap(4), Trap reboot. A fatal error detected by the CPU.
           This varies between CPU manufacturers; some possible causes
           are illegal address references, misaligned instruction or
           data access, memory protection violations, attempt to
           decode an unrecognized instruction, etc. This is a
           spontaneous reboot.

           Silent or Watchdog(5). Depending on platform, this is
           either a Silent Reboot (A reboot of unknown origin that
           reports no information) or a Watchdog reboot. This is a
           spontaneous reboot.

           Bootloader(6), Bootloader reboot. For use by bootloader
           software. This is a spontaneous reboot.

           PowerUp(7), Power-up reboot. The set-top rebooted because
           power was lost and then restored. This is a controlled
           reboot.

           Upgrade(8), Upgrade Reboot due to loading new resident
           set-top software. This is a controlled reboot.

           Hardware(9), Hardware Reset reboot. This depends on the
           design of a particular set-top. It might be due to a
           hardware reset line being triggered or by a button push of
           some kind. Manufacturers sometimes provide a way to
           hard reset a box besides removing power. This is a
           spontaneous reboot.

            CablecardError(10), CablecardError. The Host rebooted
            because the communication with the CableCARD was lost and
            could not be reestablished."
     ::= { ocStbHostRebootInfo 1 }
*/ 
int ocStbHostRebootInfoGroupBridge::Get_ocStbHostRebootType ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostRebootType" << endl;
  return 0;
}


/* 
ocStbHostRebootReset OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-write
    STATUS      current
    DESCRIPTION
            "Setting this object to true(1) causes the device to reset,
            equivalent to a power reset. Reading this object always
            returns false(2)."
    ::= { ocStbHostRebootInfo 2 }
*/ 
bool ocStbHostRebootInfoGroupBridge::Get_ocStbHostRebootReset ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostRebootReset" << endl;
  return true;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/

/* 
ocStbHostRebootReset OBJECT-TYPE
    SYNTAX      TruthValue
    MAX-ACCESS read-write
    STATUS      current
    DESCRIPTION
            "Setting this object to true(1) causes the device to reset,
            equivalent to a power reset. Reading this object always
            returns false(2)."
    ::= { ocStbHostRebootInfo 2 }
*/ 
SNMP_STATUS ocStbHostRebootInfoGroupBridge::Set_ocStbHostRebootReset (bool ocStbHostRebootReset)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;

  {
    if ((ocStbHostRebootReset != true) && (ocStbHostRebootReset != false))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_ocStbHostRebootReset" << endl;
  }

  return Status;
}






/*\
 *
 *  ocStbHostMemoryInfoGroupBridge for bridging scalars from the ocStbHostMemoryInfoGroup group 
 *
\*/

/* 
ocStbHostMemoryInfo OBJECT IDENTIFIER ::= { ocStbHostStatus 7 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostMemoryInfoGroupBridge::ocStbHostMemoryInfoGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_ocStbHostMemoryInfoGroup, pAgent)
{
}

ocStbHostMemoryInfoGroupBridge::~ocStbHostMemoryInfoGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostLargestAvailableBlock OBJECT-TYPE
    SYNTAX      Integer32
    UNITS       "kilobytes"
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The largest available continuous block of DRAM available
            on the host device, in kilobytes."
    ::= { ocStbHostMemoryInfo 1 }
*/ 
int ocStbHostMemoryInfoGroupBridge::Get_ocStbHostLargestAvailableBlock ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostLargestAvailableBlock" << endl;
  return 0;
}


/* 
ocStbHostTotalVideoMemory OBJECT-TYPE
    SYNTAX      Integer32
    UNITS       "kilobytes"
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The total amount of DRAM dedicated to video decoding and
            display on the host device, in kilobytes."
    ::= { ocStbHostMemoryInfo 2 }
*/ 
int ocStbHostMemoryInfoGroupBridge::Get_ocStbHostTotalVideoMemory ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostTotalVideoMemory" << endl;
  return 0;
}


/* 
ocStbHostAvailableVideoMemory OBJECT-TYPE
    SYNTAX      Integer32
    UNITS       "kilobytes"
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The total amount of DRAM dedicated to video decoding and
            display on the host device that is not allocated and
            available for use, in kilobytes."
    ::= { ocStbHostMemoryInfo 3 }
*/ 
int ocStbHostMemoryInfoGroupBridge::Get_ocStbHostAvailableVideoMemory ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostAvailableVideoMemory" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  ocStbHostJVMInfoGroupBridge for bridging scalars from the ocStbHostJVMInfoGroup group 
 *
\*/

/* 
ocStbHostJVMInfo OBJECT IDENTIFIER ::= { ocStbHostStatus 8 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

ocStbHostJVMInfoGroupBridge::ocStbHostJVMInfoGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_ocStbHostJVMInfoGroup, pAgent)
{
}

ocStbHostJVMInfoGroupBridge::~ocStbHostJVMInfoGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
ocStbHostJVMHeapSize OBJECT-TYPE
    SYNTAX      Integer32
    UNITS       "kilobytes"
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The total amount of DRAM allocated to the Java Virtual
            Machine heap. The units are kilobytes, defined to be
            1,024 bytes."
    ::= { ocStbHostJVMInfo 1 }
*/ 
int ocStbHostJVMInfoGroupBridge::Get_ocStbHostJVMHeapSize ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostJVMHeapSize" << endl;
  return 0;
}


/* 
ocStbHostJVMAvailHeap OBJECT-TYPE
    SYNTAX      Integer32
    UNITS       "kilobytes"
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The amount of DRAM in the Java Virtual Machine heap that
            is not allocated and is available for use.
            The units are kilobytes, defined to be 1,024 bytes."
    ::= { ocStbHostJVMInfo 2 }
*/ 
int ocStbHostJVMInfoGroupBridge::Get_ocStbHostJVMAvailHeap ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostJVMAvailHeap" << endl;
  return 0;
}


/* 
ocStbHostJVMLiveObjects OBJECT-TYPE
    SYNTAX      Integer32
    MAX-ACCESS read-only
     STATUS      current
     DESCRIPTION
             "The number of active object instances in the Java Virtual
             Machine that are reachable from running code."
     ::= { ocStbHostJVMInfo 3 }
*/ 
int ocStbHostJVMInfoGroupBridge::Get_ocStbHostJVMLiveObjects ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostJVMLiveObjects" << endl;
  return 0;
}


/* 
ocStbHostJVMDeadObjects OBJECT-TYPE
    SYNTAX      Integer32
    MAX-ACCESS read-only
    STATUS      current
    DESCRIPTION
            "The number of object instances in the Java Virtual Machine
            that are NOT reachable from running Code."
    ::= { ocStbHostJVMInfo 4 }
*/ 
int ocStbHostJVMInfoGroupBridge::Get_ocStbHostJVMDeadObjects ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_ocStbHostJVMDeadObjects" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required
\*/

/*\ 
 *  SET methods
\*/






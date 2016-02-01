//**************************************************************************
//
//    Copyright 1999  Broadcom Corporation
//    All Rights Reserved
//    No portions of this material may be reproduced in any form without the
//    written permission of:
//             Broadcom Corporation
//             16251 Laguna Canyon Road
//             Irvine, California  92618
//    All information contained in this document is Broadcom Corporation
//    company private, proprietary, and trade secret.
//
//
//
//**************************************************************************
//    Filename: EventLogApi.h
//    Author:   Kevin O'Neal
//    Creation Date: 10-april-2001
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Header file for event log constants and API's only
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef EVENTLOGAPI_H
#define EVENTLOGAPI_H

#include "typedefs.h"
#include "SnmpStatus.h"

typedef unsigned long EventLogIdCode;
typedef unsigned long DocsDevTrapTypeHint;
#define VENDOR_SPECIFIC_EVENT_ID_PREFIX   0x80000000

// Event log severity levels:
typedef enum
{
  kUseDefaultLevel = 0,
  kEmergency,
  kAlert,
  kCritical,
  kError,
  kWarning,
  kNotice,
  kInformation,
  kDebug
} EventLogSeverityLevel;

typedef enum
{
  kUseDefaultPolicy = 0,
  kCreateNew,
  kUpdateExisting,
  kDefaultNewRowBehavior // most events should use this policy (see kDefaultValue_EventLogOptimizeForSmallerSize in CustomerNonVolDefaults.h)
} EventLogPolicy;
// For legacy code (especially in vendor extensions):
#define kCreateNewIfTextChanged kDefaultNewRowBehavior

typedef enum
{
  kDefaultNvBehavior = 0, // most events should use this policy (see kDefaultValue_EventLogOptimizeForSmallerSize in CustomerNonVolDefaults.h)
  kImmediate, // Write immediately no matter what
  kDefer // Don't write immediately no matter what
} NVPolicy;
// For legacy code (especially in vendor extensions):
#define kIfNewOrTextChanged kDefaultNvBehavior

typedef enum
{
  kUnspecifiedTrap = 0,
  kColdStartTrap,
  kWarmStartTrap,
  kLinkDownTrap,
  kLinkUpTrap,
  kSnmpAuthFailureTrap,
  kEgpNeighborLossTrap,
  kEnterpriseTrap
} StandardTrapType;

struct DocsisEvent
{
  EventLogIdCode Id;
  EventLogSeverityLevel Level;
  char *pText;
  EventLogPolicy Policy;
  NVPolicy WriteToNV;
  DocsDevTrapTypeHint TrapType;
  char *pSyslogAppend;
};

// Event log ID codes
//
// ANATOMY OF AN EVENT ID:
// For DOCSIS 1.1, specific event ID's are defined for each event type in 
// Appendix J of the RFI spec.  RFI-J defines the events, but the ID's are
// defined in Appendix F of the OSS spec, and the method of encoding the ID's
// is defined in the OSS spec in section 4.4.2.2.2, thusly:
//
// "
// * The first byte (MSB) of the EventID is ASCII code for the letter in the
//   Error code from Appendix J.
// * Next 2 bytes of the EventID represent 2 or 3 digits before the dot in the
//   Error code.
// * The last byte is the number after the dot in the Error code.
// 
// For example, event D04.2 has the number (in HEX) 0x44000402
// Event I114.1 has the number 0x49007201
// "
//
// NOTE: this was modified in OSS-O-00108 to use a different scheme, like so:
// "For the standard DOCSIS events this number is converted from the error code using the
//  following rules:
//      The number is an eight digit decimal number.
//      The first two digits (left most) are the ASCII code for the letter in the Error
//    code.
//      Next four digits are filled by 2 or 3 digits between the letter and the dot in the
//    Error code with zero filling in the zap in the left side.
//      The last two digits are filled by the number after the dot in the Error code with
//    zero filling in the zap in the left.
//      For example, event D04.2 is converted into 65000402, and
//    Event I114.1 is converted into 73011401."
//
// (Note that the example of D04.2 does not appear to match the formula or the
// table, it should be 68000402.  I assume this is just a typo in the ECO and
// will use the numbers from the table).
//
// The numbers below now use this scheme.  12/19/2000.  
//
// Vendor specific event ID encoding is defined in 4.4.2.2.1
//
// "
// * Bit 31 top bit set on = Vendor Specific Event
// * Put the bottom 15 bits of the vendor's SNMP enterprise number in bits
//   30 to 16
// * Bits 15 to 0 can be used by vendor for events (65,535 possible events
//   per vendor)
// "
//
// Broadcom's SNMP enterprise number is 4413, or 0x113D.  Setting bit
// 31 gives us 0x913D.  So Broadcom's permissable range of vendor-specific
// events is 0x913D0000 - 0x913DFFFF.

#define BROADCOM_ENTERPRISE_ID            4413
#define BROADCOM_EVENT_ID_BASE            (BROADCOM_ENTERPRISE_ID << 16) | VENDOR_SPECIFIC_EVENT_ID_PREFIX

// Generic ID code used where nothing else is deemed appropriate.
// Mainly the EventLog command table "log_event" command.
#define kGeneric                                        BROADCOM_EVENT_ID_BASE

#endif


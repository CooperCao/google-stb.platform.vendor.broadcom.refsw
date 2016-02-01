//**************************************************************************
//                                                                          
//    Copyright 2008 Broadcom Corporation                                   
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
//    Description:                                                          
//                                                                          
//**************************************************************************
//    Revision History:                                                     
//                                                                          
//**************************************************************************
//    Filename: hrBridge.cpp
//    Creation Date: Wednesday, April 09, 2008 at 15:28:58
//    Created by Broadcom BFC / V2 Mib Compiler BCMIBC.EXE version 3.0.0k
//    from input file ocHostHr.mib
//
//**************************************************************************

#include "MibObjs.h"
#include "SnmpLog.h"
#include "hrBridge.h"


/******************************************************************************\

HOST-RESOURCES-MIB DEFINITIONS ::= BEGIN

IMPORTS
    MODULE-IDENTITY, OBJECT-TYPE, mib-2,
    Integer32, Counter32, Gauge32, TimeTicks  FROM SNMPv2-SMI

    TEXTUAL-CONVENTION, DisplayString,
    TruthValue, DateAndTime, AutonomousType   FROM SNMPv2-TC

    MODULE-COMPLIANCE, OBJECT-GROUP           FROM SNMPv2-CONF

    InterfaceIndexOrZero                      FROM IF-MIB;

host     OBJECT IDENTIFIER ::= { mib-2 25 }

hrStorage       OBJECT IDENTIFIER ::= { host 2 }
hrDevice        OBJECT IDENTIFIER ::= { host 3 }
hrSWRun         OBJECT IDENTIFIER ::= { host 4 }
hrSWRunPerf     OBJECT IDENTIFIER ::= { host 5 }

-- textual conventions:

KBytes ::= TEXTUAL-CONVENTION
    STATUS current
    DESCRIPTION
        "Storage size, expressed in units of 1024 bytes."
    SYNTAX Integer32 (0..2147483647)

ProductID ::= TEXTUAL-CONVENTION
    STATUS current
    DESCRIPTION
        "This textual convention is intended to identify the
        manufacturer, model, and version of a specific
        hardware or software product.  It is suggested that
        these OBJECT IDENTIFIERs are allocated such that all
        products from a particular manufacturer are registered
        under a subtree distinct to that manufacturer.  In
        addition, all versions of a product should be
        registered under a subtree distinct to that product.
        With this strategy, a management station may uniquely
        determine the manufacturer and/or model of a product
        whose productID is unknown to the management station.
        Objects of this type may be useful for inventory
        purposes or for automatically detecting
        incompatibilities or version mismatches between
        various hardware and software components on a system.

        For example, the product ID for the ACME 4860 66MHz
        clock doubled processor might be:
        enterprises.acme.acmeProcessors.a4860DX2.MHz66

        A software product might be registered as:
        enterprises.acme.acmeOperatingSystems.acmeDOS.six(6).one(1)
        "
    SYNTAX OBJECT IDENTIFIER

-- unknownProduct will be used for any unknown ProductID
-- unknownProduct OBJECT IDENTIFIER ::= { 0 0 }

InternationalDisplayString ::= TEXTUAL-CONVENTION
    STATUS current
    DESCRIPTION
        "This data type is used to model textual information
        in some character set.  A network management station
        should use a local algorithm to determine which
        character set is in use and how it should be
        displayed.  Note that this character set may be
        encoded with more than one octet per symbol, but will
        most often be NVT ASCII. When a size clause is
        specified for an object of this type, the size refers
        to the length in octets, not the number of symbols."
    SYNTAX OCTET STRING

-- The Host Resources Storage Group

-- Registration point for storage types, for use with hrStorageType.
-- These are defined in the HOST-RESOURCES-TYPES module.
hrStorageTypes          OBJECT IDENTIFIER ::= { hrStorage 1 }


\******************************************************************************/




/*\
 *
 *  hrStorageGroupBridge for bridging scalars from the hrStorageGroup group 
 *
\*/

/* 
hrStorage       OBJECT IDENTIFIER ::= { host 2 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

hrStorageGroupBridge::hrStorageGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_hrStorageGroup, pAgent)
{
}

hrStorageGroupBridge::~hrStorageGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
hrMemorySize OBJECT-TYPE
    SYNTAX     KBytes
    UNITS      "KBytes"
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
        "The amount of physical read-write main memory,
        typically RAM, contained by the host."
    ::= { hrStorage 2 }
*/ 
int hrStorageGroupBridge::Get_hrMemorySize ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrMemorySize" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  hrStorageEntryBridge for bridging elements in the hrStorageTable
 *
\*/

/* 
hrStorageTable OBJECT-TYPE
    SYNTAX     SEQUENCE OF HrStorageEntry
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
        "The (conceptual) table of logical storage areas on
        the host.

        An entry shall be placed in the storage table for each
        logical area of storage that is allocated and has
        fixed resource limits.  The amount of storage
        represented in an entity is the amount actually usable
        by the requesting entity, and excludes loss due to
        formatting or file system reference information.

        These entries are associated with logical storage
        areas, as might be seen by an application, rather than
        physical storage entities which are typically seen by
        an operating system.  Storage such as tapes and
        floppies without file systems on them are typically
        not allocated in chunks by the operating system to
        requesting applications, and therefore shouldn't
        appear in this table.  Examples of valid storage for
        this table include disk partitions, file systems, ram
        (for some architectures this is further segmented into
        regular memory, extended memory, and so on), backing
        store for virtual memory (`swap space').

        This table is intended to be a useful diagnostic for
        `out of memory' and `out of buffers' types of
        failures.  In addition, it can be a useful performance
        monitoring tool for tracking memory, disk, or buffer
        usage."
    ::= { hrStorage 3 }

INDEX { hrStorageIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

hrStorageEntryBridge::hrStorageEntryBridge (int hrStorageIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_hrStorageTable,
                      BcmObjectId(hrStorageIndex),
                      pAgent)
{
}

hrStorageEntryBridge::~hrStorageEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
hrStorageIndex OBJECT-TYPE
    SYNTAX     Integer32 (1..2147483647)
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
        "A unique value for each logical storage area
        contained by the host."
    ::= { hrStorageEntry 1 }
*/ 
int hrStorageEntryBridge::Get_hrStorageIndex ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrStorageIndex" << endl;
  return 0;
}


/* 
hrStorageType OBJECT-TYPE
    SYNTAX     AutonomousType
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
        "The type of storage represented by this entry."
    ::= { hrStorageEntry 2 }
*/ 
int hrStorageEntryBridge::Get_hrStorageType (BcmObjectId &hrStorageType)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrStorageType" << endl;
  hrStorageType = "0.0";

  return hrStorageType.Length();
}


/* 
hrStorageDescr OBJECT-TYPE
    SYNTAX     DisplayString
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
        "A description of the type and instance of the storage
        described by this entry."
    ::= { hrStorageEntry 3 }
*/ 
int hrStorageEntryBridge::Get_hrStorageDescr (BcmString &hrStorageDescr)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrStorageDescr" << endl;
  hrStorageDescr = "(unfinished)";

  return hrStorageDescr.length();
}


/* 
hrStorageAllocationUnits OBJECT-TYPE
    SYNTAX     Integer32 (1..2147483647)
    UNITS      "Bytes"
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
        "The size, in bytes, of the data objects allocated
        from this pool.  If this entry is monitoring sectors,
        blocks, buffers, or packets, for example, this number
        will commonly be greater than one.  Otherwise this
        number will typically be one."
    ::= { hrStorageEntry 4 }
*/ 
int hrStorageEntryBridge::Get_hrStorageAllocationUnits ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrStorageAllocationUnits" << endl;
  return 0;
}


/* 
hrStorageSize OBJECT-TYPE
    SYNTAX     Integer32 (0..2147483647)
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
        "The size of the storage represented by this entry, in
        units of hrStorageAllocationUnits. This object is
        writable to allow remote configuration of the size of
        the storage area in those cases where such an
        operation makes sense and is possible on the
        underlying system. For example, the amount of main
        memory allocated to a buffer pool might be modified or
        the amount of disk space allocated to virtual memory
        might be modified."
    ::= { hrStorageEntry 5 }
*/ 
int hrStorageEntryBridge::Get_hrStorageSize ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrStorageSize" << endl;
  return 0;
}


/* 
hrStorageUsed OBJECT-TYPE
    SYNTAX     Integer32 (0..2147483647)
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
        "The amount of the storage represented by this entry
        that is allocated, in units of
        hrStorageAllocationUnits."
    ::= { hrStorageEntry 6 }
*/ 
int hrStorageEntryBridge::Get_hrStorageUsed ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrStorageUsed" << endl;
  return 0;
}


/* 
hrStorageAllocationFailures OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
        "The number of requests for storage represented by
        this entry that could not be honored due to not enough
        storage.  It should be noted that as this object has a
        syntax of Counter32, that it does not have a defined
        initial value.  However, it is recommended that this
        object be initialized to zero, even though management
        stations must not depend on such an initialization."
    ::= { hrStorageEntry 7 }
*/ 
unsigned int hrStorageEntryBridge::Get_hrStorageAllocationFailures ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrStorageAllocationFailures" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\ 
 *  SET methods
\*/

/* 
hrStorageSize OBJECT-TYPE
    SYNTAX     Integer32 (0..2147483647)
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
        "The size of the storage represented by this entry, in
        units of hrStorageAllocationUnits. This object is
        writable to allow remote configuration of the size of
        the storage area in those cases where such an
        operation makes sense and is possible on the
        underlying system. For example, the amount of main
        memory allocated to a buffer pool might be modified or
        the amount of disk space allocated to virtual memory
        might be modified."
    ::= { hrStorageEntry 5 }
*/ 
SNMP_STATUS hrStorageEntryBridge::Set_hrStorageSize (int hrStorageSize)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (hrStorageSize);

  if (Status == SNMP_NO_ERROR)
  {
    if ((hrStorageSize < MINVAL_HRSTORAGESIZE)
    ||  (hrStorageSize > MAXVAL_HRSTORAGESIZE))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_hrStorageSize" << endl;
  }

  return Status;
}






/*\
 *
 *  hrDeviceEntryBridge for bridging elements in the hrDeviceTable
 *
\*/

/* 
hrDeviceTable OBJECT-TYPE
    SYNTAX     SEQUENCE OF HrDeviceEntry
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
        "The (conceptual) table of devices contained by the
        host."
    ::= { hrDevice 2 }

INDEX { hrDeviceIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

hrDeviceEntryBridge::hrDeviceEntryBridge (int hrDeviceIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_hrDeviceTable,
                      BcmObjectId(hrDeviceIndex),
                      pAgent)
{
}

hrDeviceEntryBridge::~hrDeviceEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
hrDeviceIndex OBJECT-TYPE
    SYNTAX     Integer32 (1..2147483647)
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
        "A unique value for each device contained by the host.
        The value for each device must remain constant at
        least from one re-initialization of the agent to the
        next re-initialization."
    ::= { hrDeviceEntry 1 }
*/ 
int hrDeviceEntryBridge::Get_hrDeviceIndex ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrDeviceIndex" << endl;
  return 0;
}


/* 
hrDeviceType OBJECT-TYPE
    SYNTAX     AutonomousType
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
        "An indication of the type of device.

        If this value is
        `hrDeviceProcessor { hrDeviceTypes 3 }' then an entry
        exists in the hrProcessorTable which corresponds to
        this device.

        If this value is
        `hrDeviceNetwork { hrDeviceTypes 4 }', then an entry
        exists in the hrNetworkTable which corresponds to this
        device.

        If this value is
        `hrDevicePrinter { hrDeviceTypes 5 }', then an entry
        exists in the hrPrinterTable which corresponds to this
        device.

        If this value is
        `hrDeviceDiskStorage { hrDeviceTypes 6 }', then an
        entry exists in the hrDiskStorageTable which
        corresponds to this device."
    ::= { hrDeviceEntry 2 }
*/ 
int hrDeviceEntryBridge::Get_hrDeviceType (BcmObjectId &hrDeviceType)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrDeviceType" << endl;
  hrDeviceType = "0.0";

  return hrDeviceType.Length();
}


/* 
hrDeviceDescr OBJECT-TYPE
    SYNTAX     DisplayString (SIZE (0..64))
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
        "A textual description of this device, including the
        device's manufacturer and revision, and optionally,
        its serial number."
    ::= { hrDeviceEntry 3 }
*/ 
int hrDeviceEntryBridge::Get_hrDeviceDescr (BcmString &hrDeviceDescr)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrDeviceDescr" << endl;
  hrDeviceDescr = "(unfinished)";

  return hrDeviceDescr.length();
}


/* 
hrDeviceID OBJECT-TYPE
    SYNTAX     ProductID
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
        "The product ID for this device."
    ::= { hrDeviceEntry 4 }
*/ 
int hrDeviceEntryBridge::Get_hrDeviceID (BcmObjectId &hrDeviceID)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrDeviceID" << endl;
  hrDeviceID = "0.0";

  return hrDeviceID.Length();
}


/* 
hrDeviceStatus OBJECT-TYPE
    SYNTAX     INTEGER {
                   unknown(1),
                   running(2),
                   warning(3),
                   testing(4),
                   down(5)
               }
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
        "The current operational state of the device described
        by this row of the table.  A value unknown(1)
        indicates that the current state of the device is
        unknown.  running(2) indicates that the device is up
        and running and that no unusual error conditions are
        known.  The warning(3) state indicates that agent has
        been informed of an unusual error condition by the
        operational software (e.g., a disk device driver) but
        that the device is still 'operational'.  An example
        would be a high number of soft errors on a disk.  A
        value of testing(4), indicates that the device is not
        available for use because it is in the testing state.
        The state of down(5) is used only when the agent has
        been informed that the device is not available for any
        use."
    ::= { hrDeviceEntry 5 }
*/ 
int hrDeviceEntryBridge::Get_hrDeviceStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrDeviceStatus" << endl;
  return 0;
}


/* 
hrDeviceErrors OBJECT-TYPE
    SYNTAX     Counter32
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
        "The number of errors detected on this device.  It
        should be noted that as this object has a SYNTAX of
        Counter32, that it does not have a defined initial
        value.  However, it is recommended that this object be
        initialized to zero, even though management stations
        must not depend on such an initialization."
    ::= { hrDeviceEntry 6 }
*/ 
unsigned int hrDeviceEntryBridge::Get_hrDeviceErrors ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrDeviceErrors" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  hrProcessorEntryBridge for bridging elements in the hrProcessorTable
 *
\*/

/* 
hrProcessorTable OBJECT-TYPE
    SYNTAX     SEQUENCE OF HrProcessorEntry
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
        "The (conceptual) table of processors contained by the
        host.

        Note that this table is potentially sparse: a
        (conceptual) entry exists only if the correspondent
        value of the hrDeviceType object is
        `hrDeviceProcessor'."
    ::= { hrDevice 3 }

INDEX { hrDeviceIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

// UNFINISHED: index hrDeviceIndex not from this table, verify type = ULONG
hrProcessorEntryBridge::hrProcessorEntryBridge (unsigned long hrDeviceIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_hrProcessorTable,
                      BcmObjectId(hrDeviceIndex),
                      pAgent)
{
  // SnmpLogRaw << "UNFINISHED: verify hrProcessorEntryBridge index hrDeviceIndex type=ULONG" << endl;
}

hrProcessorEntryBridge::~hrProcessorEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
hrProcessorFrwID OBJECT-TYPE
    SYNTAX     ProductID
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
        "The product ID of the firmware associated with the
        processor."
    ::= { hrProcessorEntry 1 }
*/ 
int hrProcessorEntryBridge::Get_hrProcessorFrwID (BcmObjectId &hrProcessorFrwID)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrProcessorFrwID" << endl;
  hrProcessorFrwID = "0.0";

  return hrProcessorFrwID.Length();
}


/* 
hrProcessorLoad OBJECT-TYPE
    SYNTAX     Integer32 (0..100)
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
        "The average, over the last minute, of the percentage
        of time that this processor was not idle.
        Implementations may approximate this one minute
        smoothing period if necessary."
    ::= { hrProcessorEntry 2 }
*/ 
int hrProcessorEntryBridge::Get_hrProcessorLoad ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrProcessorLoad" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  hrSWRunGroupBridge for bridging scalars from the hrSWRunGroup group 
 *
\*/

/* 
hrSWRun         OBJECT IDENTIFIER ::= { host 4 }
*/ 

/*\ 
 *  construction / destruction
\*/ 

hrSWRunGroupBridge::hrSWRunGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge (kOID_hrSWRunGroup, pAgent)
{
}

hrSWRunGroupBridge::~hrSWRunGroupBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
hrSWOSIndex OBJECT-TYPE
    SYNTAX     Integer32 (1..2147483647)
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
        "The value of the hrSWRunIndex for the hrSWRunEntry
        that represents the primary operating system running
        on this host.  This object is useful for quickly and
        uniquely identifying that primary operating system."
    ::= { hrSWRun 1 }
*/ 
int hrSWRunGroupBridge::Get_hrSWOSIndex ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrSWOSIndex" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\ 
 *  SET methods
\*/





/*\
 *
 *  hrSWRunEntryBridge for bridging elements in the hrSWRunTable
 *
\*/

/* 
hrSWRunTable OBJECT-TYPE
    SYNTAX     SEQUENCE OF HrSWRunEntry
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
        "The (conceptual) table of software running on the
        host."
    ::= { hrSWRun 2 }

INDEX { hrSWRunIndex }
*/ 

/*\ 
 *  construction / destruction
\*/ 

hrSWRunEntryBridge::hrSWRunEntryBridge (int hrSWRunIndex, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_hrSWRunTable,
                      BcmObjectId(hrSWRunIndex),
                      pAgent)
{
}

hrSWRunEntryBridge::~hrSWRunEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
hrSWRunIndex OBJECT-TYPE
    SYNTAX     Integer32 (1..2147483647)
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
        "A unique value for each piece of software running on
        the host.  Wherever possible, this should be the
        system's native, unique identification number."
    ::= { hrSWRunEntry 1 }
*/ 
int hrSWRunEntryBridge::Get_hrSWRunIndex ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrSWRunIndex" << endl;
  return 0;
}


/* 
hrSWRunName OBJECT-TYPE
    SYNTAX     InternationalDisplayString (SIZE (0..64))
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
        "A textual description of this running piece of
        software, including the manufacturer, revision,  and
        the name by which it is commonly known.  If this
        software was installed locally, this should be the
        same string as used in the corresponding
        hrSWInstalledName."
    ::= { hrSWRunEntry 2 }
*/ 
int hrSWRunEntryBridge::Get_hrSWRunName (BcmString &hrSWRunName)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrSWRunName" << endl;
  hrSWRunName = "(unfinished)";

  return hrSWRunName.length();
}


/* 
hrSWRunID OBJECT-TYPE
    SYNTAX     ProductID
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
        "The product ID of this running piece of software."
    ::= { hrSWRunEntry 3 }
*/ 
int hrSWRunEntryBridge::Get_hrSWRunID (BcmObjectId &hrSWRunID)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrSWRunID" << endl;
  hrSWRunID = "0.0";

  return hrSWRunID.Length();
}


/* 
hrSWRunPath OBJECT-TYPE
    SYNTAX     InternationalDisplayString (SIZE(0..128))
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
        "A description of the location on long-term storage
        (e.g. a disk drive) from which this software was
        loaded."
    ::= { hrSWRunEntry 4 }
*/ 
int hrSWRunEntryBridge::Get_hrSWRunPath (BcmString &hrSWRunPath)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrSWRunPath" << endl;
  hrSWRunPath = "(unfinished)";

  return hrSWRunPath.length();
}


/* 
hrSWRunParameters OBJECT-TYPE
    SYNTAX     InternationalDisplayString (SIZE(0..128))
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
        "A description of the parameters supplied to this
        software when it was initially loaded."
    ::= { hrSWRunEntry 5 }
*/ 
int hrSWRunEntryBridge::Get_hrSWRunParameters (BcmString &hrSWRunParameters)
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrSWRunParameters" << endl;
  hrSWRunParameters = "(unfinished)";

  return hrSWRunParameters.length();
}


/* 
hrSWRunType OBJECT-TYPE
    SYNTAX     INTEGER {
                   unknown(1),
                   operatingSystem(2),
                   deviceDriver(3),
                   application(4)
               }
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
        "The type of this software."
    ::= { hrSWRunEntry 6 }
*/ 
int hrSWRunEntryBridge::Get_hrSWRunType ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrSWRunType" << endl;
  return 0;
}


/* 
hrSWRunStatus OBJECT-TYPE
    SYNTAX     INTEGER {
                   running(1),
                   runnable(2),    -- waiting for resource
                                   -- (i.e., CPU, memory, IO)
                   notRunnable(3), -- loaded but waiting for event
                   invalid(4)      -- not loaded
               }
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
        "The status of this running piece of software.
        Setting this value to invalid(4) shall cause this
        software to stop running and to be unloaded. Sets to
        other values are not valid."
    ::= { hrSWRunEntry 7 }
*/ 
int hrSWRunEntryBridge::Get_hrSWRunStatus ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrSWRunStatus" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\ 
 *  SET methods
\*/

/* 
hrSWRunStatus OBJECT-TYPE
    SYNTAX     INTEGER {
                   running(1),
                   runnable(2),    -- waiting for resource
                                   -- (i.e., CPU, memory, IO)
                   notRunnable(3), -- loaded but waiting for event
                   invalid(4)      -- not loaded
               }
    MAX-ACCESS read-write
    STATUS     current
    DESCRIPTION
        "The status of this running piece of software.
        Setting this value to invalid(4) shall cause this
        software to stop running and to be unloaded. Sets to
        other values are not valid."
    ::= { hrSWRunEntry 7 }
*/ 
SNMP_STATUS hrSWRunEntryBridge::Set_hrSWRunStatus (int hrSWRunStatus)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (hrSWRunStatus);

  if (Status == SNMP_NO_ERROR)
  {
    if ((hrSWRunStatus != CONSTVAL_HRSWRUNSTATUS_RUNNING)
    &&  (hrSWRunStatus != CONSTVAL_HRSWRUNSTATUS_RUNNABLE)
    &&  (hrSWRunStatus != CONSTVAL_HRSWRUNSTATUS_NOTRUNNABLE)
    &&  (hrSWRunStatus != CONSTVAL_HRSWRUNSTATUS_INVALID))
      return SNMP_WRONG_VALUE;

    // UNFINISHED: really set something here if all constraints
    // check out
    SnmpLogRaw << "UNFINISHED: Set_hrSWRunStatus" << endl;
  }

  return Status;
}






/*\
 *
 *  hrSWRunPerfEntryBridge for bridging elements in the hrSWRunPerfTable
 *
\*/

/* 
hrSWRunPerfTable OBJECT-TYPE
    SYNTAX     SEQUENCE OF HrSWRunPerfEntry
    MAX-ACCESS not-accessible
    STATUS     current
    DESCRIPTION
        "The (conceptual) table of running software
        performance metrics."
    ::= { hrSWRunPerf 1 }

INDEX {  }
*/ 

/*\ 
 *  construction / destruction
\*/ 

// UNFINISHED: unknown index, verify type = ULONG
hrSWRunPerfEntryBridge::hrSWRunPerfEntryBridge (unsigned long Index, BcmSnmpAgent *pAgent)
  : TabularMibBridge (kOID_hrSWRunPerfTable,
                      Index,
                      pAgent)
{
  // SnmpLogRaw << "UNFINISHED: unknown index for hrSWRunPerfEntryBridge, verify ULONG" << endl;
}

hrSWRunPerfEntryBridge::~hrSWRunPerfEntryBridge ()
{
}

/*\ 
 *  GET methods
\*/

/* 
hrSWRunPerfCPU OBJECT-TYPE
    SYNTAX     Integer32 (0..2147483647)
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
        "The number of centi-seconds of the total system's CPU
        resources consumed by this process.  Note that on a
        multi-processor system, this value may increment by
        more than one centi-second in one centi-second of real
        (wall clock) time."
    ::= { hrSWRunPerfEntry 1 }
*/ 
int hrSWRunPerfEntryBridge::Get_hrSWRunPerfCPU ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrSWRunPerfCPU" << endl;
  return 0;
}


/* 
hrSWRunPerfMem OBJECT-TYPE
    SYNTAX     KBytes
    UNITS      "KBytes"
    MAX-ACCESS read-only
    STATUS     current
    DESCRIPTION
        "The total amount of real system memory allocated to
        this process."
    ::= { hrSWRunPerfEntry 2 }
*/ 
int hrSWRunPerfEntryBridge::Get_hrSWRunPerfMem ()
{
  // UNFINISHED: return meaningful value here!
  SnmpLogRaw << "UNFINISHED: Get_hrSWRunPerfMem" << endl;
  return 0;
}


/*\ 
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\ 
 *  SET methods
\*/




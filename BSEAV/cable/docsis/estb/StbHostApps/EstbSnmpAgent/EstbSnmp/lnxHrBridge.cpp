/******************************************************************************
 *    (c)2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 *****************************************************************************/

//**************************************************************************
//    Description:                                                          
//                                                                          
//**************************************************************************
//    Revision History:                                                     
//                                                                          
//**************************************************************************
//    Filename: lnxHrBridge.cpp
//    Creation Date: Tuesday, December 19, 2006 at 11:25:07
//    Created by Broadcom BFC / V2 Mib Compiler BCMIBC.EXE version 3.0.0k
//    from input file ocHostHr.mib
//
//**************************************************************************

#include "MibObjs.h"
#include "SnmpLog.h"
#include "lnxHrBridge.h"

#include <sys/stat.h>
#include <unistd.h>


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

hrSystem        OBJECT IDENTIFIER ::= { host 1 }
hrStorage       OBJECT IDENTIFIER ::= { host 2 }
hrDevice        OBJECT IDENTIFIER ::= { host 3 }
hrSWRun         OBJECT IDENTIFIER ::= { host 4 }
hrSWRunPerf     OBJECT IDENTIFIER ::= { host 5 }
hrSWInstalled   OBJECT IDENTIFIER ::= { host 6 }
hrMIBAdminInfo  OBJECT IDENTIFIER ::= { host 7 }

hostResourcesMibModule MODULE-IDENTITY
    LAST-UPDATED "200003060000Z"    -- 6 March 2000
    ORGANIZATION "IETF Host Resources MIB Working Group"
    CONTACT-INFO
        "Steve Waldbusser
        Postal: Lucent Technologies, Inc.
                1213 Innsbruck Dr.
                Sunnyvale, CA 94089
                USA
        Phone:  650-318-1251
        Fax:    650-318-1633
        Email:  waldbusser@lucent.com

        In addition, the Host Resources MIB mailing list is
        dedicated to discussion of this MIB. To join the
        mailing list, send a request message to
        hostmib-request@andrew.cmu.edu. The mailing list
        address is hostmib@andrew.cmu.edu."

    DESCRIPTION
        "This MIB is for use in managing host systems. The term
        `host' is construed to mean any computer that communicates
        with other similar computers attached to the internet and
        that is directly used by one or more human beings. Although
        this MIB does not necessarily apply to devices whose primary
        function is communications services (e.g., terminal servers,
        routers, bridges, monitoring equipment), such relevance is
        not explicitly precluded.  This MIB instruments attributes
        common to all internet hosts including, for example, both
        personal computers and systems that run variants of Unix."

    
    ::= { hrMIBAdminInfo 1 }


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


hrStorageTypes          OBJECT IDENTIFIER ::= { hrStorage 1 }


\******************************************************************************/

#define LNX_MEMDIVE_MEMTOTAL         1       
#define LNX_MEMDIVE_MEMFREE          2       
#define LNX_MEMDIVE_BUFFERS          3       
#define LNX_MEMDIVE_CACHED           4       
#define LNX_MEMDIVE_SWAPCACHED       5       
#define LNX_MEMDIVE_ACTIVE           6       
#define LNX_MEMDIVE_INACTIVE         7       
#define LNX_MEMDIVE_HIGHTOTAL        8       
#define LNX_MEMDIVE_HIGHFREE         9       
#define LNX_MEMDIVE_LOWTOTAL        10       
#define LNX_MEMDIVE_LOWFREE         11       
#define LNX_MEMDIVE_SWAPTOTAL       12       
#define LNX_MEMDIVE_SWAPFREE        13       
#define LNX_MEMDIVE_DIRTY           14       
#define LNX_MEMDIVE_WRITEBACK       15       
#define LNX_MEMDIVE_MAPPED          16       
#define LNX_MEMDIVE_SLAB            17       
#define LNX_MEMDIVE_COMMITLIMIT     18       
#define LNX_MEMDIVE_COMMITTED_AS    19       
#define LNX_MEMDIVE_PAGETABLES      20       
#define LNX_MEMDIVE_VMALLOCTOTAL    21       
#define LNX_MEMDIVE_VMALLOCUSED     22       
#define LNX_MEMDIVE_VMALLOCCHUNK    23       

int
linuxProcessorLoad () {
  double         load;
  FILE           *in = fopen("/proc/loadavg", "r");

  if (!in) {
    SnmpLogRaw << "linxuProcessorLoad() cannot open /proc/loadavg" << endl;
    return (-1);
  }

  fscanf(in, "%lf ", &load);
  fclose(in);

  return ((int) (load * 100));

}

int
linuxPagesize () {
  struct stat     kc_buf;

  stat("/proc/kcore", &kc_buf);

  return (kc_buf.st_size / 1024);       /* 4K too large ? */

}

int
linuxMemDive(int diveType)
{
  FILE           *fp;
  char            buf[100];
  int             retVal = -1;
  bool            found = false;

  if ((fp = fopen("/proc/meminfo", "r")) == NULL)
    return (retVal);

  while (fgets(buf, sizeof(buf), fp) != NULL) {
    switch (diveType) {
      case LNX_MEMDIVE_MEMTOTAL:
        if (!strncmp(buf, "MemTotal:", 9)) {
          sscanf(buf, "%*s %d", &retVal);
          found = true;
        }
        break;
      case LNX_MEMDIVE_MEMFREE:
        if (!strncmp(buf, "MemFree:", 8)) {
          sscanf(buf, "%*s %d", &retVal);
          found = true;
        }
        break;
      case LNX_MEMDIVE_VMALLOCTOTAL:
        if (!strncmp(buf, "VmallocTotal:", 13)) {
          sscanf(buf, "%*s %d", &retVal);
          found = true;
        }
        break;
      case LNX_MEMDIVE_VMALLOCUSED:
        if (!strncmp(buf, "VmallocUsed:", 12)) {
          sscanf(buf, "%*s %d", &retVal);
          found = true;
        }
        break;
    }
    if (found == true) {
      fclose(fp);
      return (retVal);
    }
  }

  fclose(fp);
  return (retVal);
}

int linuxProcessCpuUsage (int pid) {
  FILE   *fp;
  char   *cp, buf[256], path[256];
  int     i, retVal;

  sprintf(path, "/proc/%d/stat", pid);
  if ((fp = fopen(path, "r")) == NULL)
    return 0;
  fgets(buf, sizeof(buf), fp);
  cp = buf;
  for (i = 0; i < 13; ++i) {      /* skip 13 fields */
    while (*cp != ' ')
      ++cp;
    ++cp;
  }

  retVal = atoi(cp); /* utime */

  while (*cp != ' ')
    ++cp;
  ++cp;
  retVal += atoi(cp);        /* + stime */
  fclose(fp);

  return (retVal);
}

int linuxProcessMemUsage (int pid ) {
    FILE  *fp;
    char  *cp, buf[256], path[256];
    int    i, retVal;

    sprintf(path, "/proc/%d/stat", pid);
    if ((fp = fopen(path, "r")) == NULL)
        return 0;
    fgets(buf, sizeof(buf), fp);
    cp = buf;
    for (i = 0; i < 23; ++i) {      /* skip 23 fields */
        while (*cp != ' ')
            ++cp;
        ++cp;
    }
    retVal = atoi(cp) * (getpagesize() / 1024);        /* rss */
    fclose(fp);

  return (retVal);
}

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

lnxHrStorageGroupBridge::lnxHrStorageGroupBridge (BcmSnmpAgent *pAgent)
  : hrStorageGroupBridge (pAgent)
{
}

lnxHrStorageGroupBridge::~lnxHrStorageGroupBridge ()
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
int lnxHrStorageGroupBridge::Get_hrMemorySize ()
{
  return (linuxMemDive(LNX_MEMDIVE_MEMTOTAL));
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

lnxHrStorageEntryBridge::lnxHrStorageEntryBridge (int hrStorageIndex, BcmSnmpAgent *pAgent)
  : hrStorageEntryBridge (hrStorageIndex,
                      pAgent)
{
  fHrStorageIndex = hrStorageIndex;
}

lnxHrStorageEntryBridge::~lnxHrStorageEntryBridge ()
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
int lnxHrStorageEntryBridge::Get_hrStorageIndex ()
{
  return (fHrStorageIndex);
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
int lnxHrStorageEntryBridge::Get_hrStorageType (BcmObjectId &hrStorageType)
{
  if (fHrStorageIndex == HR_STORAGE_INDEX_RAM)
    hrStorageType = kOID_hrStorageTypeRam;
  else if (fHrStorageIndex == HR_STORAGE_INDEX_VIRTUAL)
    hrStorageType = kOID_hrStorageTypeVirtual;
  else
    hrStorageType = kOID_hrStorageTypeOther;

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
int lnxHrStorageEntryBridge::Get_hrStorageDescr (BcmString &hrStorageDescr)
{
  if (fHrStorageIndex == HR_STORAGE_INDEX_RAM)
    hrStorageDescr = "RAM";
  else if (fHrStorageIndex == HR_STORAGE_INDEX_VIRTUAL)
    hrStorageDescr = "Virtual Memory";
  else
    hrStorageDescr = "Other storage type";

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
int lnxHrStorageEntryBridge::Get_hrStorageAllocationUnits ()
{
  return (1024);
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
int lnxHrStorageEntryBridge::Get_hrStorageSize ()
{
  if (fHrStorageIndex == HR_STORAGE_INDEX_RAM)
    return (linuxMemDive(LNX_MEMDIVE_MEMTOTAL));
  else if (fHrStorageIndex == HR_STORAGE_INDEX_VIRTUAL)
    return (linuxMemDive(LNX_MEMDIVE_VMALLOCTOTAL));
  else
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
int lnxHrStorageEntryBridge::Get_hrStorageUsed ()
{
  if (fHrStorageIndex == HR_STORAGE_INDEX_RAM)
    return (linuxMemDive(LNX_MEMDIVE_MEMTOTAL) - linuxMemDive(LNX_MEMDIVE_MEMFREE));
  else if (fHrStorageIndex == HR_STORAGE_INDEX_VIRTUAL)
    return (linuxMemDive(LNX_MEMDIVE_VMALLOCUSED));
  else
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
        storage.  It should be noted that as this object has the
        syntax that it does, it does not have a defined
        initial value.  However, it is recommended that this
        object be initialized to zero, even though management
        stations must not depend on such an initialization."
    ::= { hrStorageEntry 7 }
*/ 
unsigned int lnxHrStorageEntryBridge::Get_hrStorageAllocationFailures ()
{
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
SNMP_STATUS lnxHrStorageEntryBridge::Set_hrStorageSize (int hrStorageSize)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (hrStorageSize);

  if (Status == SNMP_NO_ERROR)
  {
    if ((hrStorageSize < MINVAL_HRSTORAGESIZE)
    ||  (hrStorageSize > MAXVAL_HRSTORAGESIZE))
      return SNMP_WRONG_VALUE;

    Status = SNMP_WRONG_VALUE;
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

lnxHrDeviceEntryBridge::lnxHrDeviceEntryBridge (int hrDeviceIndex, 
  BcmObjectId hrDeviceType, BcmString hrDeviceDescr, 
  BcmObjectId hrDeviceId, BcmSnmpAgent *pAgent)
  : hrDeviceEntryBridge (hrDeviceIndex, pAgent)
{
  fIndex = hrDeviceIndex;
  fType = BcmObjectId(kOID_hrDeviceTypesGroup);
  fType += hrDeviceType;
  fDescr = hrDeviceDescr;
  fId = hrDeviceId;
  fpAgent = pAgent;
}

lnxHrDeviceEntryBridge::~lnxHrDeviceEntryBridge ()
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
int lnxHrDeviceEntryBridge::Get_hrDeviceIndex ()
{
  return fIndex;
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
int lnxHrDeviceEntryBridge::Get_hrDeviceType (BcmObjectId &hrDeviceType)
{
  hrDeviceType = fType;

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
int lnxHrDeviceEntryBridge::Get_hrDeviceDescr (BcmString &hrDeviceDescr)
{
  hrDeviceDescr = fDescr;

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
int lnxHrDeviceEntryBridge::Get_hrDeviceID (BcmObjectId &hrDeviceID)
{
  hrDeviceID = fId;

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
int lnxHrDeviceEntryBridge::Get_hrDeviceStatus ()
{
  return 2; 
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
unsigned int lnxHrDeviceEntryBridge::Get_hrDeviceErrors ()
{
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

lnxHrProcessorEntryBridge::lnxHrProcessorEntryBridge (unsigned long hrDeviceIndex, BcmSnmpAgent *pAgent)
  : hrProcessorEntryBridge (hrDeviceIndex, pAgent)
{
  fHrDeviceIndex = hrDeviceIndex;
}

lnxHrProcessorEntryBridge::~lnxHrProcessorEntryBridge ()
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
int lnxHrProcessorEntryBridge::Get_hrProcessorFrwID (BcmObjectId &hrProcessorFrwID)
{
  hrProcessorFrwID = kOID_bcm97401;

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
int lnxHrProcessorEntryBridge::Get_hrProcessorLoad ()
{
  return (linuxProcessorLoad());
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

lnxHrSWRunEntryBridge::lnxHrSWRunEntryBridge (int hrSWRunIndex, BcmSnmpAgent *pAgent)
  : hrSWRunEntryBridge (hrSWRunIndex, pAgent)
{
  fIndex = hrSWRunIndex;
}

lnxHrSWRunEntryBridge::~lnxHrSWRunEntryBridge ()
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
int lnxHrSWRunEntryBridge::Get_hrSWRunIndex ()
{
  return fIndex;
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
int lnxHrSWRunEntryBridge::Get_hrSWRunName (BcmString &hrSWRunName)
{
  FILE           *fp;
  char     string[256];
  char            buf[256];
  char *cp;
  
  sprintf(string, "/proc/%d/status", fIndex);
  if ((fp = fopen(string, "r")) == NULL)
    return 0;
  fgets(buf, sizeof(buf), fp);    /* Name: process name */
  if (strlen(buf) == 0) {
    fclose(fp);
    return 0;
  }
  cp = buf;
  while (*cp != ':')
     ++cp;
  ++cp;
  while (isspace(*cp))
    ++cp;
  strcpy(string, cp);
  fclose(fp);

  hrSWRunName = string;
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
int lnxHrSWRunEntryBridge::Get_hrSWRunID (BcmObjectId &hrSWRunID)
{
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
int lnxHrSWRunEntryBridge::Get_hrSWRunPath (BcmString &hrSWRunPath)
{
  FILE           *fp;
  char     string[256];
  char            buf[256];
  char *cp;

  sprintf(string, "/proc/%d/cmdline", fIndex);
  if ((fp = fopen(string, "r")) == NULL)
    return 0;
  if (fgets(buf, sizeof(buf) - 1, fp))    /* argv[0] '\0' argv[1] '\0' .... */
    strcpy(string, buf);
  else {
    /*
     * swapped out - no cmdline
     */
    fclose(fp);
    sprintf(string, "/proc/%d/status", fIndex);
    if ((fp = fopen(string, "r")) == NULL)
      return 0;
    fgets(buf, sizeof(buf), fp);        /* Name: process name */
    if (strlen(buf) == 0) {
      fclose(fp);
      return 0;
    }
    cp = strchr(buf, ':');
    ++cp;
    while (isspace(*cp))
      ++cp;
    strcpy(string, cp);
    cp = strchr(string, '\n');
    if (cp)
      *cp = 0;
  }
  fclose(fp);

  hrSWRunPath = string;
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
int lnxHrSWRunEntryBridge::Get_hrSWRunParameters (BcmString &hrSWRunParameters)
{
  FILE           *fp;
  char     string[256];
  char            buf[256];
  char *cp;

  sprintf(string, "/proc/%d/cmdline", fIndex);
  if ((fp = fopen(string, "r")) == NULL)
    return 0;
  memset(buf, 0, sizeof(buf));

  /*
   * argv[0] '\0' argv[1] '\0' ....
   */
  if (!fgets(buf, sizeof(buf) - 2, fp)) {
    /*
     * maybe be empty (even argv[0] is missing)
     */
    fclose(fp);
    return 0;
  }
  if (strlen(buf) == 0) {
    fclose(fp);
    return 0;
  }
  /*
   * Skip over argv[0]
   */
  cp = buf;
  while (*cp) {
    ++cp;
  }
  ++cp;
  /*
   * Now join together separate arguments.
   */
  while (1) {
    while (*cp) {
      ++cp;
    }
    if (*(cp + 1) == '\0')
      break;          /* '\0''\0' => End of command line */
    *cp = ' ';
  }

  cp = buf;
  while (*cp)
    ++cp;
  ++cp;
  strcpy(string, cp);
  fclose(fp);

  hrSWRunParameters = string;

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
int lnxHrSWRunEntryBridge::Get_hrSWRunType ()
{
  return 4;
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
int lnxHrSWRunEntryBridge::Get_hrSWRunStatus ()
{
  FILE           *fp;
  char     string[256];
  char            buf[256];
  char *cp;
  int  retVal = 4; // default value is "invalid"
  int i;

  sprintf(string, "/proc/%d/stat", fIndex);
  if ((fp = fopen(string, "r")) != NULL) {
    fgets(buf, sizeof(buf), fp);
    if (strlen(buf) == 0) {
      fclose(fp);
      return retVal;
    }
    cp = buf;
    for (i = 0; i < 2; ++i) {   /* skip two fields */
      while (*cp != ' ')
        ++cp;
      ++cp;
    }

    switch (*cp) {
    case 'R':
      retVal = 1;        /* running */
      break;
    case 'S':
      retVal = 2;        /* runnable */
      break;
    case 'D':
    case 'T':
      retVal = 3;        /* notRunnable */
      break;
    case 'Z':
    default:
      retVal = 4;        /* invalid */
      break;
    }
    fclose(fp);
  }

  return retVal;
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
SNMP_STATUS lnxHrSWRunEntryBridge::Set_hrSWRunStatus (int hrSWRunStatus)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (hrSWRunStatus);

  if (Status == SNMP_NO_ERROR)
  {
    if ((hrSWRunStatus != CONSTVAL_HRSWRUNSTATUS_RUNNING)
    &&  (hrSWRunStatus != CONSTVAL_HRSWRUNSTATUS_RUNNABLE)
    &&  (hrSWRunStatus != CONSTVAL_HRSWRUNSTATUS_NOTRUNNABLE)
    &&  (hrSWRunStatus != CONSTVAL_HRSWRUNSTATUS_INVALID))
      return SNMP_WRONG_VALUE;

  }

  return SNMP_WRONG_VALUE;
  // return Status;
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

lnxHrSWRunPerfEntryBridge::lnxHrSWRunPerfEntryBridge (unsigned long hrSwIndex, BcmSnmpAgent *pAgent)
  : hrSWRunPerfEntryBridge (hrSwIndex, pAgent)
{
  fLinuxPID = hrSwIndex;
}

lnxHrSWRunPerfEntryBridge::~lnxHrSWRunPerfEntryBridge ()
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
int lnxHrSWRunPerfEntryBridge::Get_hrSWRunPerfCPU ()
{
  return (linuxProcessCpuUsage(fLinuxPID));
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
int lnxHrSWRunPerfEntryBridge::Get_hrSWRunPerfMem ()
{
  return (linuxProcessMemUsage(fLinuxPID));
}


/*\ 
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\ 
 *  SET methods
\*/




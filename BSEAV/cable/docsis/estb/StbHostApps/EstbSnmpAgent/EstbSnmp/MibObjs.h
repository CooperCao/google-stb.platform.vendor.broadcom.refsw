//**************************************************************************
//
//    Copyright 2000 Broadcom Corporation
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
//    Filename: MibObjs.h
//    Author:   Kevin O'Neal
//    Creation Date: 21-Jan-2000
//
//**************************************************************************
//    Description:
//
//      Cablemodem V2 code
//      
//    Basic classes for dealing with SNMP objects and concepts
//
//      
//**************************************************************************
//    Revision History:
//
//**************************************************************************


#ifndef MIBOBJS_H
#define MIBOBJS_H

#include <stdlib.h>
#include <string.h>

#include "MessageLog.h"
#include "IpAddress.h"
#include "MutexSemaphore.h"
#include "SnmpType.h"
#include "CoreObjs.h"
#include "HalIf.h"


class BcmSnmpAgent;
class BcmMibGroup;
class BcmMibTable;
class BcmMibEntry;

class BcmMibBridge;
class ScalarMibBridge;
class TabularMibBridge;

class snmpCommunityTable;

// To create objects for the not-accessible MIB objects, define the following
// constant.  No real point in it, since that would just waste code space and
// memory, but for completeness they are coded up anyway.
//#define CREATE_NOT_ACCESSIBLE_OBJECTS

// To create objects for 64 bit counters, define this constant.  Note that at
// this time, 64 bit counters are not yet supported.  So they'll have to be
// implemented if this is defined.
//#define CREATE_64_BIT_COUNTERS

// Specify whether to use maps or lists for object containers.  Maps have a 
// performance advantage, but lists require less code space / memory.
#define USE_LISTS           1
#define USE_MAPS            2
#define USE_VECTORS         3
#define USE_WRAPPED_LISTS   4
#define MIB_OBJECT_CONTAINERS   USE_WRAPPED_LISTS

#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
#include <map>
#elif (MIB_OBJECT_CONTAINERS == USE_VECTORS)
#include <vector>
#elif (MIB_OBJECT_CONTAINERS == USE_LISTS)
#include <list>
#elif (MIB_OBJECT_CONTAINERS == USE_WRAPPED_LISTS)
#include "MibObjList.h"
#endif

// The following #define determines what happens in a call to 
// BcmMibObject::GetOid().  If it is defined, then that call will result in a
// recursive call up the parent chain which constructs the OID by adding each
// successive parent's node ID to an OID and building it that way.  If it's not
// defined, then we'll just build the OID from the OID string.  It's not
// defined for now since we store the whole OID string anyway, and building
// the OID from the string is actually faster than doing the recursion.  Also,
// as long as we do store the whole OID string, storing the node ID as well
// takes up additional RAM.  Code inspection has actually shown that reducing
// the OID string to just the node ID still doesn't save any RAM, because the
// OID string is actually an initialized const pointer stored in code space,
// and storing the node ID requires copies for each object, so there's actually
// no good reason to use the node ID method.  However, since the node ID code
// does work, I want to leave it in just in case we ever need to go that route
// in the future, and just compile it out via the absence of this compiler 
// directive.
// #define USE_NODE_ID

#define NOT_IMPLEMENTED_STRING  "Not implemented yet"
#define STUB_TABLE_ENTRIES      2


// The following #define determines our behaviour with respect to SNMP sets
// to objects in a read-create table whose row has not been created.  See
// RFC-1903, starting page 7, for the spec regarding row creation.  Turns out
// that if we get a set to an instance of an object (except the RowStatus object),
// that we MAY go ahead and create the row as though we'd had a createAndWait
// set to the RowStatus object.  This behaviour is allowed, but is not required.
// At the discretion of the agent, we may also reject such sets with a return
// code of INCONSISTANT NAME.  So, if the following macro is defined, we will
// support the indirect row creation method as described in note (4), third
// option.  If it's not, we'll reject such sets per note (4), first option.
// The note (4) I'm talking about here is in RFC-1903, on page 9.  Not clear
// what the CableLabs requirement here is, the spec is silent.  However, some
// TEPs seem to expect support of indirect row creation, so by default I'll
// define this.
#define SUPPORT_INDIRECT_ROW_CREATION


class BcmMibRequest
{
  public:
    BcmMibRequest (int Type, int Version, int fSrcIfIndex, const BcmObjectId &OID, const BcmObjectId &Index, const BcmIpAddress &SrcIp, const BcmIpAddress &DstIp, const BcmString &Cmty, BcmSnmpAgent *pAgent = NULL);
    BcmMibRequest (unsigned char *pPacketPdu);
    BcmMibRequest ();
    virtual ~BcmMibRequest ();
    
    SNMP_STATUS ParseFrom (unsigned char *pPacketPdu);
    
    // Load up an SnmpPacket structure from SnmpStatus.h with our data.
    void Get (SnmpPacket &Packet) const;
    
    typedef enum
    {
      kGetRequest = PDU_GET_REQUEST,
      kNextRequest = PDU_GET_NEXT_REQUEST,
      kGetResponse = PDU_GET_RESPONSE,
      kSetRequest = PDU_SET_REQUEST,
      kTrap = PDU_TRAP,
      kGetBulkRequest = PDU_GET_BULK_REQUEST,
      kInformRequest = PDU_INFORM_REQUEST,
      kTrap2 = PDU_TRAP2,
      kRport = PDU_REPORT,
      
      // The following are not real PDU types, but used internally
      kValidateSetRequest = PDU_VALIDATE_SET_REQUEST,
      kTestRequest = PDU_TEST_REQUEST, 
      kUnknownPdu = PDU_NONE
    } MibRequestType;
    
    virtual ostream &Print(ostream &outStream) const;
    
    // Make these public so no accessors are necessary
    int fVersion; // SNMPv1, v2, v3, or unknown...
    int fType; // get, set, next...
    int fSrcIfIndex; // ifIndex of the interface the request came in on
    BcmObjectId fOID; // OID being requested
    BcmObjectId fIndex; // index of object being requested
    BcmIpAddress fSrcIp; // IP addr from whence request came
    BcmIpAddress fDstIp; // IP addr the request was to (necessary for multi-agent implementation)
    BcmString fCmty; // community string
    BcmSnmpAgent *fpAgent;
};

class BcmMibObject
{
  public:
    typedef enum
    {
      kUnspecified = 0,
      kNotAccessible,
      kAccessibleForNotify,
      kReadOnly,
      kReadWrite,
      kReadCreate
    } MibObjectAccess;
  
    BcmMibObject (const char *pOID, const char *pName, BcmMibObject *pParent = NULL, MibObjectAccess Access = kUnspecified);
    virtual ~BcmMibObject ();

    typedef enum
    {
      kScalarMibObject = 0,
      kTabularMibObject,
      kMibEntry,
      kMibTable,
      kMibGroup
    } MibObjectType;
    
    virtual bool IsVisible () const;
    virtual bool IsAccessible (BcmMibRequest *pReq = NULL) const;
    virtual bool IsBridged () const;
   
    // Default action for GET or SET is to return NO_SUCH_NAME.
    // Objects which will have read and/or write permission must 
    // override these virtual functions.
    virtual SNMP_STATUS Get (BcmVarBind &VB) {return SNMP_NO_SUCH_NAME;};
    virtual SNMP_STATUS Test (BcmVarBind &VB) {return SNMP_NO_ERROR;};
    virtual SNMP_STATUS Set (BcmVarBind &VB) {return SNMP_NO_SUCH_NAME;};

    // These virtual functions are provided to allow tables which are populated
    // with standard MIB object types (instead of custom derived objects)
    // a place to talk to the system.  All the standard MIB objects call
    // these when having a SET or GET applied.  Used by MibEntries and MibGroups
    virtual SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB) {return SNMP_NO_OPINION;};
    virtual SNMP_STATUS TestLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB) {return SNMP_NO_ERROR;};
    virtual SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB) {return SNMP_NOT_WRITABLE;};
    
    // Convert 0/1 bool value to 2/1 SNMP truth value and vice-versa
    int BoolToTruth (bool BoolVal) const;
    bool TruthToBool (int TruthVal) const;

    virtual void GetIndex (BcmObjectId &Index) const;
    const char *Name () const;
    inline const char *OidString () const 
      {return fpOidString;}
    int ObjectType() const;
    MibObjectAccess Access() const;
    unsigned long Leaf() const;
    BcmMibRequest *LastRequestInfo () const;

    // GetOid may be const if we aren't using the node ID approach, but
    // not if we are using the node ID approach.
#ifdef USE_NODE_ID    
    virtual void GetOid (BcmObjectId &Oid);
#else
    virtual void GetOid (BcmObjectId &Oid) const;
#endif
    
    // Note that Print here is not a const method because it calls Get
    // to update the value before printing.
    virtual ostream &Print(ostream &outStream);
    
    // Bridge objects are specific to a given object.  Default value 
    // here is to do nothing.  Derived classes need to handle their own
    // particular kind of bridge objects by overriding these methods.
    virtual SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction = false);
    virtual SNMP_STATUS DeRegisterBridge (BcmMibBridge *pBridge);
    
    virtual BcmSnmpAgent *GetAgent ();
    
    inline const BcmMibObject *GetParent(void) const { return fpParent; }

  private:
    // Default constructor: not supported
    BcmMibObject (void);
    // Copy constructor: not supported
    BcmMibObject (const BcmMibObject &otherInstance);
    // Assignment operator: not supported      
    BcmMibObject & operator = (const BcmMibObject &otherInstance);
    
  protected:
    const char *fpName;
    const char *fpOidString;
    BcmMibObject *fpParent;
    BcmSnmpAgent *fpAgent;
    MibObjectType fObjType;
    MibObjectAccess fAccess;
    BcmMibRequest *fpRequest;

    virtual SNMP_STATUS SetParent (BcmMibObject *pParent);
    virtual SNMP_STATUS SetAgent (BcmSnmpAgent *pAgent);
    virtual SNMP_STATUS SetIndex (const BcmObjectId &Index);

#ifdef USE_NODE_ID    
    unsigned long *fpNodeId;
    unsigned short fNodeIdLen;
    virtual SNMP_STATUS BuildOid (BcmObjectId &OID);
#endif
  
  public:
    // Access to this object's bridge object
    BcmMibBridge *fpMibBridge;
    
    bool fDestroyBridge;

  friend class BcmMibEntry;
  friend class BcmMibTable;
  friend class BcmMibGroup;
  friend class BcmSnmpAgent;
};

#if (MIB_OBJECT_CONTAINERS == USE_MAPS)
typedef map  <BcmObjectId, BcmMibObject *> MibObjectContainer;
#elif (MIB_OBJECT_CONTAINERS == USE_VECTORS)
typedef vector <BcmMibObject *> MibObjectContainer;
#elif (MIB_OBJECT_CONTAINERS == USE_LISTS)
typedef list <BcmMibObject *> MibObjectContainer;
#elif (MIB_OBJECT_CONTAINERS == USE_WRAPPED_LISTS)
typedef BcmMibObjectList MibObjectContainer;
#endif

typedef MibObjectContainer::iterator MibObjectIterator;
typedef MibObjectContainer::const_iterator MibObjectConstIterator;

/* Following are a couple of standard object types */

class StringMibObject : public BcmMibObject
{
  public:
    StringMibObject (const char *pOID, const char *pName, BcmMibObject *pParent = NULL, MibObjectAccess Access = kUnspecified);
    virtual ~StringMibObject ();
   
    virtual SNMP_STATUS Get (BcmVarBind &VB);
    virtual SNMP_STATUS Test (BcmVarBind &VB);
    virtual SNMP_STATUS Set (BcmVarBind &VB);
    
    static SNMP_STATUS TestDefaults (const BcmString &TestVal);
    
  private:
    // Default constructor: not supported
    StringMibObject (void);
    // Copy constructor: not supported
    StringMibObject (const StringMibObject &otherInstance);
    // Assignment operator: not supported      
    StringMibObject & operator = (const StringMibObject &otherInstance);
};

class IpAddrMibObject : public BcmMibObject
{
  public:
    IpAddrMibObject (const char *pOID, const char *pName, BcmMibObject *pParent = NULL, MibObjectAccess Access = kUnspecified);
    virtual ~IpAddrMibObject ();
   
    virtual SNMP_STATUS Get (BcmVarBind &VB);
    virtual SNMP_STATUS Test (BcmVarBind &VB);
    virtual SNMP_STATUS Set (BcmVarBind &VB);
    
    static SNMP_STATUS TestDefaults (const BcmIpAddress &TestVal);
    
  private:
    // Default constructor: not supported
    IpAddrMibObject (void);
    // Copy constructor: not supported
    IpAddrMibObject (const IpAddrMibObject &otherInstance);
    // Assignment operator: not supported      
    IpAddrMibObject & operator = (const IpAddrMibObject &otherInstance);
};

class MacAddrMibObject : public BcmMibObject
{
  public:
    MacAddrMibObject (const char *pOID, const char *pName, BcmMibObject *pParent = NULL, MibObjectAccess Access = kUnspecified);
    virtual ~MacAddrMibObject ();
    
    typedef enum
    {
      kMacAddrSyntax = 0,
      kPhysAddrSyntax
    } MacAddrSyntax;
    
    virtual SNMP_STATUS SetSyntax (MacAddrSyntax Syntax);
    virtual int GetSyntax ();
   
    virtual SNMP_STATUS Get (BcmVarBind &VB);
    virtual SNMP_STATUS Test (BcmVarBind &VB);
    virtual SNMP_STATUS Set (BcmVarBind &VB);
    
    static SNMP_STATUS TestDefaults (const BcmMacAddress &TestVal);
    
  private:
    // Default constructor: not supported
    MacAddrMibObject (void);
    // Copy constructor: not supported
    MacAddrMibObject (const MacAddrMibObject &otherInstance);
    // Assignment operator: not supported      
    MacAddrMibObject & operator = (const MacAddrMibObject &otherInstance);
   
  protected:
    MacAddrSyntax fSyntax;
};

class Int32MibObject : public BcmMibObject
{
  public:
    Int32MibObject (const char *pOID, const char *pName, BcmMibObject *pParent = NULL, MibObjectAccess Access = kUnspecified);
    virtual ~Int32MibObject ();
   
    virtual SNMP_STATUS Get (BcmVarBind &VB);
    virtual SNMP_STATUS Test (BcmVarBind &VB);
    virtual SNMP_STATUS Set (BcmVarBind &VB);
    
    static SNMP_STATUS TestDefaults (int TestVal);
    
  private:
    // Default constructor: not supported
    Int32MibObject (void);
    // Copy constructor: not supported
    Int32MibObject (const Int32MibObject &otherInstance);
    // Assignment operator: not supported      
    Int32MibObject & operator = (const Int32MibObject &otherInstance);
};

class Uint32MibObject : public BcmMibObject
{
  public:
    Uint32MibObject (const char *pOID, const char *pName, BcmMibObject *pParent = NULL, MibObjectAccess Access = kUnspecified);
    virtual ~Uint32MibObject ();
   
    virtual SNMP_STATUS Get (BcmVarBind &VB);
    virtual SNMP_STATUS Test (BcmVarBind &VB);
    virtual SNMP_STATUS Set (BcmVarBind &VB);
    
    static SNMP_STATUS TestDefaults (unsigned long TestVal);
    
  private:
    // Default constructor: not supported
    Uint32MibObject (void);
    // Copy constructor: not supported
    Uint32MibObject (const Uint32MibObject &otherInstance);
    // Assignment operator: not supported      
    Uint32MibObject & operator = (const Uint32MibObject &otherInstance);
};

class Uint64MibObject : public BcmMibObject
{
  public:
    Uint64MibObject (const char *pOID, const char *pName, BcmMibObject *pParent = NULL, MibObjectAccess Access = kUnspecified);
    virtual ~Uint64MibObject ();
   
    virtual SNMP_STATUS Get (BcmVarBind &VB);
    virtual SNMP_STATUS Test (BcmVarBind &VB);
    virtual SNMP_STATUS Set (BcmVarBind &VB);
    
    static SNMP_STATUS TestDefaults (const BcmCounter64 &TestVal);
    
  private:
    // Default constructor: not supported
    Uint64MibObject (void);
    // Copy constructor: not supported
    Uint64MibObject (const Uint64MibObject &otherInstance);
    // Assignment operator: not supported      
    Uint64MibObject & operator = (const Uint64MibObject &otherInstance);
};

// argument for EndTime and StartTime in constructor below to mean current time
#define TIMETICKS_NOW -1
class TimeticksMibObject : public BcmMibObject
{
  public:
    TimeticksMibObject (const char *pOID, const char *pName, BcmMibObject *pParent = NULL, MibObjectAccess Access = kUnspecified);
    virtual ~TimeticksMibObject ();
   
    virtual SNMP_STATUS Get (BcmVarBind &VB);
    virtual SNMP_STATUS Test (BcmVarBind &VB);
    virtual SNMP_STATUS Set (BcmVarBind &VB);
    
    static SNMP_STATUS TestDefaults (time_t TestVal);
    
  private:
    // Default constructor: not supported
    TimeticksMibObject (void);
    // Copy constructor: not supported
    TimeticksMibObject (const TimeticksMibObject &otherInstance);
    // Assignment operator: not supported      
    TimeticksMibObject & operator = (const TimeticksMibObject &otherInstance);
};

class MsTimeticksMibObject : public BcmMibObject
{
  public:
    MsTimeticksMibObject (const char *pOID, const char *pName, BcmMibObject *pParent = NULL, MibObjectAccess Access = kUnspecified);
    virtual ~MsTimeticksMibObject ();
   
    virtual SNMP_STATUS Get (BcmVarBind &VB);
    virtual SNMP_STATUS Test (BcmVarBind &VB);
    virtual SNMP_STATUS Set (BcmVarBind &VB);
    
    static SNMP_STATUS TestDefaults (unsigned long TestVal);
    
  private:
    // Default constructor: not supported
    MsTimeticksMibObject (void);
    // Copy constructor: not supported
    MsTimeticksMibObject (const TimeticksMibObject &otherInstance);
    // Assignment operator: not supported      
    MsTimeticksMibObject & operator = (const TimeticksMibObject &otherInstance);
};

// Special BcmMibObject to control entry creation/deletion of certain tables.
// Since this type of object exists ONLY in tabular values, the BcmMibEntry argument
// to the constructor has no default value of NULL, in contrast to other MibObjects.
class RowStatusMibObject : public Int32MibObject
{
  public:
    RowStatusMibObject (BcmMibTable *pTable, BcmMibEntry *pEntry, const char *pOID, const char *pName, MibObjectAccess Access = (MibObjectAccess) kUnspecified);
    virtual ~RowStatusMibObject ();
    
    typedef enum
    {
      kUnspecified = 0,
      kFromProvisioning = -1,
      kCreatedDefault = -2,
      kFromMgmt = -3
    } RowStatusCreationMethod;
   
    virtual SNMP_STATUS Get (BcmVarBind &VB);
    virtual SNMP_STATUS Test (BcmVarBind &VB);
    virtual SNMP_STATUS Set (BcmVarBind &VB);
    
    static SNMP_STATUS TestDefaults (int TestVal, int CurVal);
    
    virtual int CreationMethod () const;
    
  protected:
    virtual SNMP_STATUS SetParent (BcmMibObject *pParent);
    
  private:
    // Default constructor: not supported
    RowStatusMibObject (void);
    // Copy constructor: not supported
    RowStatusMibObject (const RowStatusMibObject &otherInstance);
    // Assignment operator: not supported      
    RowStatusMibObject & operator = (const RowStatusMibObject &otherInstance);
    
  protected:
    BcmMibTable *fpTable;
    int fHowCreated; // one of the RowStatusCreationMethod enum values,
                     // or else an ifNumber.
    
  friend class TabularMibBridge;
  friend class BcmMibEntry;
  friend class BcmMibTable;
};

class TestAndIncrMibObject : public Int32MibObject
{
  public:
    TestAndIncrMibObject (const char *pOID, const char *pName, BcmMibObject *pParent = NULL, MibObjectAccess Access = kUnspecified);
    virtual ~TestAndIncrMibObject ();

    virtual SNMP_STATUS Get (BcmVarBind &VB);   
    virtual SNMP_STATUS Test (BcmVarBind &VB);
    virtual SNMP_STATUS Set (BcmVarBind &VB);
    
    static SNMP_STATUS TestDefaults (int TestVal, int CurVal);
    
  protected:
    int fValue;
    
  private:
    // Default constructor: not supported
    TestAndIncrMibObject (void);
    // Copy constructor: not supported
    TestAndIncrMibObject (const TestAndIncrMibObject &otherInstance);
    // Assignment operator: not supported      
    TestAndIncrMibObject & operator = (const TestAndIncrMibObject &otherInstance);
};

class OidMibObject : public BcmMibObject
{
  public:
    OidMibObject (const char *pOID, const char *pName, BcmMibObject *pParent = NULL, MibObjectAccess Access = kUnspecified);
    virtual ~OidMibObject ();
   
    virtual SNMP_STATUS Get (BcmVarBind &VB);
    virtual SNMP_STATUS Test (BcmVarBind &VB);
    virtual SNMP_STATUS Set (BcmVarBind &VB);
    
    static SNMP_STATUS TestDefaults (const BcmObjectId &TestVal);
    
  private:
    // Default constructor: not supported
    OidMibObject (void);
    // Copy constructor: not supported
    OidMibObject (const OidMibObject &otherInstance);
    // Assignment operator: not supported      
    OidMibObject & operator = (const OidMibObject &otherInstance);
   
  protected:
    BcmObjectId *fpValue;
};

class BitsMibObject : public BcmMibObject
{
  public:
    BitsMibObject (const char *pOID, const char *pName, BcmMibObject *pParent = NULL, MibObjectAccess Access = kUnspecified);
    virtual ~BitsMibObject ();
   
    virtual SNMP_STATUS Get (BcmVarBind &VB);
    virtual SNMP_STATUS Test (BcmVarBind &VB);
    virtual SNMP_STATUS Set (BcmVarBind &VB);
    
    static SNMP_STATUS TestDefaults (unsigned long TestVal);
    
  private:
    // Default constructor: not supported
    BitsMibObject (void);
    // Copy constructor: not supported
    BitsMibObject (const BitsMibObject &otherInstance);
    // Assignment operator: not supported      
    BitsMibObject & operator = (const BitsMibObject &otherInstance);
};

class DateTimeMibObject : public BcmMibObject
{
  public:
    DateTimeMibObject (const char *pOID, const char *pName, BcmMibObject *pParent = NULL, MibObjectAccess Access = kUnspecified);
    virtual ~DateTimeMibObject ();
   
    virtual SNMP_STATUS Get (BcmVarBind &VB);
    virtual SNMP_STATUS Test (BcmVarBind &VB);
    virtual SNMP_STATUS Set (BcmVarBind &VB);
    
    static SNMP_STATUS TestDefaults (time_t TestVal);
    
  private:
    // Default constructor: not supported
    DateTimeMibObject (void);
    // Copy constructor: not supported
    DateTimeMibObject (const DateTimeMibObject &otherInstance);
    // Assignment operator: not supported      
    DateTimeMibObject & operator = (const DateTimeMibObject &otherInstance);
};


// One or more MibObjects
class BcmMibEntry : public BcmMibObject
{
  public:
    BcmMibEntry (BcmMibTable *pTable, const char *pName, const char *pIndex = NULL);
    virtual ~BcmMibEntry ();
    
    virtual bool IsVisible () const;
    virtual bool IsAccessible (BcmMibRequest *pReq = NULL) const;
    virtual bool IsBridged () const;
   
    virtual void GetIndex (BcmObjectId &Index) const;
    
    // GetOid may be const if we aren't using the node ID approach, but
    // not if we are using the node ID approach.
#ifdef USE_NODE_ID    
    virtual void GetOid (BcmObjectId &Oid);
#else
    virtual void GetOid (BcmObjectId &Oid) const;
#endif
    
    virtual unsigned long LastColumn () const;
    
    virtual BcmMibObject *FindObject (const BcmObjectId &OID);
    virtual BcmMibObject *FindObject (const char *pOidStr);
    virtual BcmMibObject *FindObject (const BcmString &Name, const BcmObjectId &Index);
    virtual BcmMibObject *FindObject (const BcmString &Name);
    
    virtual RowStatusMibObject *GetStatusObject ();
    
    virtual SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction = false);
    virtual SNMP_STATUS DeRegisterBridge (BcmMibBridge *pBridge);
    
    virtual ostream &Print(ostream &outStream);
    
  private:
    // Default constructor: not supported
    BcmMibEntry (void);
    // Copy constructor: not supported
    BcmMibEntry (const BcmMibEntry &otherInstance);
    // Assignment operator: not supported      
    BcmMibEntry & operator = (const BcmMibEntry &otherInstance);
    
  protected:
    unsigned long *fpIndex;
    unsigned short fIndexLen;
    unsigned long fLastColumn;
    BcmMibTable  *fpTable;
    RowStatusMibObject *fpStatusObject;
    MibObjectContainer fObjects;

    virtual SNMP_STATUS SetIndex (const BcmObjectId &Index);
    virtual SNMP_STATUS SetAgent (BcmSnmpAgent *pAgent);
    virtual SNMP_STATUS AddObject (BcmMibObject *pObject);
    virtual SNMP_STATUS RemoveObject (BcmMibObject *pObject);

  friend class BcmMibObject;
  friend class RowStatusMibObject;
  friend class BcmMibTable;
  friend class BcmMibGroup;
  friend class BcmSnmpAgent;
};

typedef MibObjectContainer  MibEntryContainer;
typedef MibObjectIterator   MibEntryIterator;
typedef MibObjectConstIterator   MibEntryConstIterator;

// One or more BcmMibEntry's
class BcmMibTable : public BcmMibObject
{
  public:
    BcmMibTable (const char *pOID, const char *pName);
    virtual ~BcmMibTable ();
   
    virtual bool Populate ();
    virtual bool Depopulate ();
    
    virtual bool IsAccessible (BcmMibRequest *pReq = NULL) const;
    virtual bool IsBridged () const;
    
    virtual SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction = false);
    virtual SNMP_STATUS DeRegisterBridge (BcmMibBridge *pBridge);
    
    virtual int NumEntries () const;
    virtual bool DoesColumnExist (unsigned long Column) const;
   
    virtual SNMP_STATUS ExtractEntryIndex (const BcmObjectId &OID, BcmObjectId &Index);
    virtual SNMP_STATUS SetAgent (BcmSnmpAgent *pAgent);
   
    virtual BcmMibObject *FindObject (const BcmObjectId &OID);
    virtual BcmMibObject *FindObject (const char *pOidStr);
    virtual BcmMibObject *FindObject (const BcmString &Name, const BcmObjectId &Index);
    virtual BcmMibObject *FindObject (const BcmString &Name);
    
    virtual BcmMibEntry  *FindEntry  (const BcmObjectId &Index);
    virtual BcmMibEntry  *FindEntry  (const BcmObjectId &Index, MibEntryIterator &Pos);
    
    virtual SNMP_STATUS AddEntry (BcmMibEntry *pEntry);
    virtual SNMP_STATUS RemoveEntry (BcmMibEntry *pEntry);
    
    virtual BcmMibEntry  *FirstEntry ();
    virtual BcmMibEntry  *NextEntry (const BcmObjectId &Index);
    virtual BcmMibEntry  *NextEntry (const BcmObjectId &Index, MibEntryIterator &Pos);
    
    virtual BcmMibEntry  *LastEntry ();
    virtual BcmMibEntry  *PrevEntry (const BcmObjectId &Index);
    virtual BcmMibEntry  *PrevEntry (const BcmObjectId &Index, MibEntryIterator &Pos);
    
    inline BcmMibEntry  *FirstEntry (MibEntryIterator &Pos);
    inline BcmMibEntry  *NextEntry  (MibEntryIterator &Pos);
    virtual BcmMibEntry  *LastEntry  (MibEntryIterator &Pos);
    virtual BcmMibEntry  *PrevEntry  (MibEntryIterator &Pos);
    
    virtual BcmMibEntry *CreateEntry (const BcmObjectId &OID, BcmMibRequest *pReq = NULL);
    virtual BcmMibEntry *NewDefaultEntry (BcmObjectId &Index) {return NULL;};
    
    virtual SNMP_STATUS RemoveEntry (const BcmObjectId &OID);
    virtual SNMP_STATUS DestroyEntry (const BcmObjectId &OID);
    
    virtual void Flush ();
    
    virtual ostream &Print(ostream &outStream);
    
  private:
    // Default constructor: not supported
    BcmMibTable (void);
    // Copy constructor: not supported
    BcmMibTable (const BcmMibTable &otherInstance);
    // Assignment operator: not supported      
    BcmMibTable & operator = (const BcmMibTable &otherInstance);
   
  protected:
    unsigned long      fColumnMask;
    MibEntryContainer  fEntries;

  friend class BcmMibObject;
  friend class BcmMibEntry;
  friend class BcmMibGroup;
  friend class BcmSnmpAgent;
};


inline BcmMibEntry *BcmMibTable::FirstEntry (MibEntryIterator &Pos)
{
  Pos = fEntries.begin();

  if (fEntries.empty() == true)
    return NULL;

#if (MIB_OBJECT_CONTAINERS == USE_MAPS)  
  return (BcmMibEntry *)(*Pos).second;
#else
  return (BcmMibEntry *) *Pos;
#endif
}

inline BcmMibEntry *BcmMibTable::NextEntry (MibEntryIterator &Pos)
{
  /*\ 
   *  NOTE: don't lock the MIB here for 2 reasons.
   *
   *  1: This method is used in BcmSnmpAgent::ApplyPacketFilters
   *     for filter iteration, which puts it right in the data path.
   *     Don't want to add a mutex lock there.
   *  2: All we're doing is operating on the iterator anyway.  The caller
   *     needs to do the locking in his scope if he needs it.
  \*/

  // Increment our iterator.
  Pos++;

  // Get the next entry value.
  if (Pos != fEntries.end())
  {
    // KO MOD: still return an item, even if it is not visible.
    // It should be up to the caller to test this if need be, and
    // one should be able to recurse entries even if they are not
    // visible!
#if (MIB_OBJECT_CONTAINERS == USE_MAPS)  
    return(BcmMibEntry *)(*Pos).second;
#else
    return(BcmMibEntry *) *Pos;
#endif
  }

  return NULL;
}

typedef MibObjectContainer MibTableContainer;
typedef MibObjectIterator  MibTableIterator;
typedef MibObjectConstIterator  MibTableConstIterator;

typedef MibObjectContainer MibGroupContainer;
typedef MibObjectIterator  MibGroupIterator;
typedef MibObjectConstIterator  MibGroupConstIterator;

// One or more scalar BcmMibObject's and/or tabular BcmMibTable's and/or sub-BcmMibGroup's
class BcmMibGroup : public BcmMibObject
{
  public:
    BcmMibGroup(const char *pOID, const char *pName);
    virtual ~BcmMibGroup();
   
    virtual bool Populate ();
    virtual bool Depopulate ();
    
    virtual bool IsAccessible (BcmMibRequest *pReq = NULL) const;
    virtual bool IsBridged () const;
    
    virtual SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction = false);
    virtual SNMP_STATUS DeRegisterBridge (BcmMibBridge *pBridge);
    
    virtual SNMP_STATUS AddScalar (BcmMibObject *pObject);
    virtual SNMP_STATUS RemoveScalar (BcmMibObject *pObject);
    virtual BcmMibObject *FindObject (const BcmObjectId &OID);
    virtual BcmMibObject *FindObject (const char *pOidStr);
    virtual BcmMibObject *FindObject (const BcmString &Name, const BcmObjectId &Index);
    virtual BcmMibObject *FindObject (const BcmString &Name);
    virtual BcmMibObject *NextObject (const BcmObjectId &OID);
    
    virtual SNMP_STATUS AddTable (BcmMibTable *pTable);
    virtual SNMP_STATUS RemoveTable (BcmMibTable *pTable);
    virtual BcmMibTable *FindTable (const BcmObjectId &OID);
    virtual BcmMibTable *FindTable (const BcmString &Name);
    
    virtual SNMP_STATUS AddGroup (BcmMibGroup *pGroup);
    virtual SNMP_STATUS RemoveGroup (BcmMibGroup *pGroup);
    virtual BcmMibGroup *FindGroup (const BcmObjectId &OID);
    
    virtual BcmMibObject *FindOtherInstance (const BcmObjectId &OID, const BcmMibRequest *pReq);
    
    virtual SNMP_STATUS SetAgent (BcmSnmpAgent *pAgent);
   
    virtual SNMP_STATUS AddProxyTable (BcmMibTable *pTable);
    virtual SNMP_STATUS AddProxyScalar (BcmMibObject *pObject);

  private:
    // Default constructor: not supported
    BcmMibGroup (void);
    // Copy constructor: not supported
    BcmMibGroup (const BcmMibGroup &otherInstance);
    // Assignment operator: not supported      
    BcmMibGroup & operator = (const BcmMibGroup &otherInstance);
    
  protected:
    // Combined list for scalars, tables, and groups
    MibObjectContainer fObjects; 

  friend class BcmMibObject;
  friend class BcmMibEntry;
  friend class BcmMibTable;
};

// Note that the following operators do not take const arguments.
// This is necessary because, if fact, printing these objects is NOT
// a constant operation.  Before printing, the object gets its value,
// which may modify it.

inline ostream & operator << (ostream &outStream, BcmMibRequest &Obj)
{
    return Obj.Print(outStream);
}

inline ostream & operator << (ostream &outStream, BcmMibObject &Obj)
{
    return Obj.Print(outStream);
}

inline ostream & operator << (ostream &outStream, BcmMibEntry &Entry)
{
    return Entry.Print(outStream);
}

inline ostream & operator << (ostream &outStream, BcmMibTable &Table)
{
    return Table.Print(outStream);
}

#endif

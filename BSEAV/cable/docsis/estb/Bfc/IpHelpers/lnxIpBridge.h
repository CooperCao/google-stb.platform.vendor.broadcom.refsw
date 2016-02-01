//**************************************************************************
//                                                                          
//    Copyright 2003 Broadcom Corporation                                   
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
//    Filename: lnxIpBridge.h
//    Creation Date: 11/14/03
//    Created by Broadcom V2 Mib Compiler BCMIBC.EXE version 1.2.0b from input file rfc2011.mib
//
//**************************************************************************

#ifndef LNXIPBRIDGE_H
#define LNXIPBRIDGE_H


#include "ipBridge.h"
#include "IpDatagramFragmentReassembler.h"



/*\
 *  ipGroup 
\*/

class lnxIpGroupBridge : public ipGroupBridge
{
  friend class ipGroup;

  public:
    lnxIpGroupBridge (BcmSnmpAgent *pAgent);
   ~lnxIpGroupBridge ();
   
    /* GET methods */

    virtual int Get_ipForwarding ();
    virtual int Get_ipDefaultTTL ();
    virtual unsigned int Get_ipInReceives ();
    virtual unsigned int Get_ipInHdrErrors ();
    virtual unsigned int Get_ipInAddrErrors ();
    virtual unsigned int Get_ipForwDatagrams ();
    virtual unsigned int Get_ipInUnknownProtos ();
    virtual unsigned int Get_ipInDiscards ();
    virtual unsigned int Get_ipInDelivers ();
    virtual unsigned int Get_ipOutRequests ();
    virtual unsigned int Get_ipOutDiscards ();
    virtual unsigned int Get_ipOutNoRoutes ();
    virtual int Get_ipReasmTimeout ();
    virtual unsigned int Get_ipReasmReqds ();
    virtual unsigned int Get_ipReasmOKs ();
    virtual unsigned int Get_ipReasmFails ();
    virtual unsigned int Get_ipFragOKs ();
    virtual unsigned int Get_ipFragFails ();
    virtual unsigned int Get_ipFragCreates ();
    virtual unsigned int Get_ipRoutingDiscards ();

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

    virtual SNMP_STATUS Set_ipForwarding (int ipForwarding);
    virtual SNMP_STATUS Set_ipDefaultTTL (int ipDefaultTTL);

    /* Access methods */
    inline void RegisterBcmReassembler(BcmIpDatagramFragmentReassembler *reassembler);

    protected:
    void linux_read_ip_stat ();

    protected:
        unsigned long   IpForwarding;
        unsigned long   IpDefaultTTL;
        unsigned long   IpInReceives;
        unsigned long   IpInHdrErrors;
        unsigned long   IpInAddrErrors;
        unsigned long   IpForwDatagrams;
        unsigned long   IpInUnknownProtos;
        unsigned long   IpInDiscards;
        unsigned long   IpInDelivers;
        unsigned long   IpOutRequests;
        unsigned long   IpOutDiscards;
        unsigned long   IpOutNoRoutes;
        unsigned long   IpReasmTimeout;
        unsigned long   IpReasmReqds;
        unsigned long   IpReasmOKs;
        unsigned long   IpReasmFails;
        unsigned long   IpFragOKs;
        unsigned long   IpFragFails;
        unsigned long   IpFragCreates;
        BcmIpDatagramFragmentReassembler *BcmReassembler;

};

inline void lnxIpGroupBridge::RegisterBcmReassembler(BcmIpDatagramFragmentReassembler *reassembler) { BcmReassembler = reassembler;}

/*\
 *  ipNetToMediaEntry
\*/

#define MINVAL_IPNETTOMEDIAIFINDEX  1
#define MAXVAL_IPNETTOMEDIAIFINDEX  2147483647
#define CONSTVAL_IPNETTOMEDIATYPE_OTHER  1
#define CONSTVAL_IPNETTOMEDIATYPE_INVALID  2
#define CONSTVAL_IPNETTOMEDIATYPE_DYNAMIC  3
#define CONSTVAL_IPNETTOMEDIATYPE_STATIC  4

// Forward reference:
class lnxIpNetToMediaEntryBridge : public ipNetToMediaEntryBridge
{
  friend class ipNetToMediaEntry;

  public:
    lnxIpNetToMediaEntryBridge (const BcmMacAddress &ipNetToMediaPhysAddress, const BcmIpAddress &ipNetToMediaNetAddress, int ipNetToMediaType, BcmSnmpAgent *pAgent);
   ~lnxIpNetToMediaEntryBridge ();
    virtual SNMP_STATUS Register(CleanupMethod Cleanup, bool SilentFailure=false);
    virtual SNMP_STATUS DeRegister();
 

    /* GET methods */

    int Get_ipNetToMediaIfIndex ();
    void Get_ipNetToMediaPhysAddress (BcmMacAddress &ipNetToMediaPhysAddress);
    void Get_ipNetToMediaNetAddress (BcmIpAddress &ipNetToMediaNetAddress);
    int Get_ipNetToMediaType ();

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

    SNMP_STATUS Set_ipNetToMediaIfIndex (int ipNetToMediaIfIndex);
    SNMP_STATUS Set_ipNetToMediaPhysAddress (const BcmMacAddress &ipNetToMediaPhysAddress);
    SNMP_STATUS Set_ipNetToMediaNetAddress (const BcmIpAddress &ipNetToMediaNetAddress);
    SNMP_STATUS Set_ipNetToMediaType (int ipNetToMediaType);

    // Accessor method:
    inline int GetIpStackInterface (void) const { return fIf; }

  protected:
    BcmMacAddress fMac;
    BcmIpAddress fIp;
    int fType;
    int fIf;
};

class lnxIpNetToPhysicalEntryBridge : public ipNetToPhysicalEntryBridge
{
  friend class ipNetToPhysicalEntry;

  public:
    lnxIpNetToPhysicalEntryBridge (ipNetToMediaEntryBridge *pN2M);
   ~lnxIpNetToPhysicalEntryBridge ();

    /* GET methods */

    int Get_ipNetToPhysicalIfIndex ();
    int Get_ipNetToPhysicalNetAddressType ();
    int Get_ipNetToPhysicalNetAddress (BcmString &ipNetToPhysicalNetAddress);
    void Get_ipNetToPhysicalPhysAddress (BcmMacAddress &ipNetToPhysicalPhysAddress);
    unsigned long Get_ipNetToPhysicalLastUpdated ();
    int Get_ipNetToPhysicalType ();
    int Get_ipNetToPhysicalState ();
    int Get_ipNetToPhysicalRowStatus ();

    /* TEST methods */

    SNMP_STATUS Test_ipNetToPhysicalPhysAddress (const BcmMacAddress &ipNetToPhysicalPhysAddress);
    SNMP_STATUS Test_ipNetToPhysicalRowStatus (int ipNetToPhysicalRowStatus);

    /* SET methods */

    SNMP_STATUS Set_ipNetToPhysicalPhysAddress (const BcmMacAddress &ipNetToPhysicalPhysAddress);
    SNMP_STATUS Set_ipNetToPhysicalType (int ipNetToPhysicalType);
    SNMP_STATUS Set_ipNetToPhysicalRowStatus (int &ipNetToPhysicalRowStatus);


  protected:
    int fRowStatus;
    BcmMacAddress fMac;
    BcmString fIpAddr;
    int fIpAddrType;
    int fEntryType;
    int fIfIndex;
    unsigned long fLastChange;
    BcmSnmpAgent *fpAgent;
	ipNetToMediaEntryBridge *fpN2M;

};


/*\
 *  icmpGroup 
\*/


class lnxIcmpGroupBridge : public icmpGroupBridge
{
  friend class icmpGroup;

  public:
    lnxIcmpGroupBridge (BcmSnmpAgent *pAgent);
   ~lnxIcmpGroupBridge ();
   
    /* GET methods */

    virtual unsigned int Get_icmpInMsgs ();
    virtual unsigned int Get_icmpInErrors ();
    virtual unsigned int Get_icmpInDestUnreachs ();
    virtual unsigned int Get_icmpInTimeExcds ();
    virtual unsigned int Get_icmpInParmProbs ();
    virtual unsigned int Get_icmpInSrcQuenchs ();
    virtual unsigned int Get_icmpInRedirects ();
    virtual unsigned int Get_icmpInEchos ();
    virtual unsigned int Get_icmpInEchoReps ();
    virtual unsigned int Get_icmpInTimestamps ();
    virtual unsigned int Get_icmpInTimestampReps ();
    virtual unsigned int Get_icmpInAddrMasks ();
    virtual unsigned int Get_icmpInAddrMaskReps ();
    virtual unsigned int Get_icmpOutMsgs ();
    virtual unsigned int Get_icmpOutErrors ();
    virtual unsigned int Get_icmpOutDestUnreachs ();
    virtual unsigned int Get_icmpOutTimeExcds ();
    virtual unsigned int Get_icmpOutParmProbs ();
    virtual unsigned int Get_icmpOutSrcQuenchs ();
    virtual unsigned int Get_icmpOutRedirects ();
    virtual unsigned int Get_icmpOutEchos ();
    virtual unsigned int Get_icmpOutEchoReps ();
    virtual unsigned int Get_icmpOutTimestamps ();
    virtual unsigned int Get_icmpOutTimestampReps ();
    virtual unsigned int Get_icmpOutAddrMasks ();
    virtual unsigned int Get_icmpOutAddrMaskReps ();

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

    /* Access methods */

    protected:
      void linux_read_icmp_stat ();


    protected:
        unsigned long   IcmpInMsgs;
        unsigned long   IcmpInErrors;
        unsigned long   IcmpInDestUnreachs;
        unsigned long   IcmpInTimeExcds;
        unsigned long   IcmpInParmProbs;
        unsigned long   IcmpInSrcQuenchs;
        unsigned long   IcmpInRedirects;
        unsigned long   IcmpInEchos;
        unsigned long   IcmpInEchoReps;
        unsigned long   IcmpInTimestamps;
        unsigned long   IcmpInTimestampReps;
        unsigned long   IcmpInAddrMasks;
        unsigned long   IcmpInAddrMaskReps;
        unsigned long   IcmpOutMsgs;
        unsigned long   IcmpOutErrors;
        unsigned long   IcmpOutDestUnreachs;
        unsigned long   IcmpOutTimeExcds;
        unsigned long   IcmpOutParmProbs;
        unsigned long   IcmpOutSrcQuenchs;
        unsigned long   IcmpOutRedirects;
        unsigned long   IcmpOutEchos;
        unsigned long   IcmpOutEchoReps;
        unsigned long   IcmpOutTimestamps;
        unsigned long   IcmpOutTimestampReps;
        unsigned long   IcmpOutAddrMasks;
        unsigned long   IcmpOutAddrMaskReps;
};

#endif

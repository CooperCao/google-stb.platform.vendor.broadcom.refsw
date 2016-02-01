//**************************************************************************
//
//    Copyright 2001 Broadcom Corporation
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
//    Filename: docsDevTrap.h
//    Author:   Kevin O'Neal
//    Creation Date: 23-January-2001
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Traps associated with the docsDevNotifications group
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef DOCSDEVTRAP_H
#define DOCSDEVTRAP_H

#include "BcmTrap.h"
#include "EventLogApi.h"


// Forward references:
class EventLogEntry;
class BcmMibTable;

// DOCSIS 1.0 style trap.  Uses the same trap OID for all traps, formats
// text of event as varbind.
class docsDevTrap_10 : public BcmTrap
{ 
  public:
    docsDevTrap_10 (BcmSnmpAgent *pAgent, const EventLogEntry *pEvent);
    virtual ~docsDevTrap_10 ();
   
  protected:
    BcmString fFormattedMsg;
    const EventLogEntry *fpEvent;
    
    void FormatMsg ();
};

/* Base class for DOCS-CABLE-DEVICE-TRAP-MIB */
class docsDevTrap : public BcmTrap
{
  public:
    docsDevTrap (const char *pOID, const char *pName, BcmSnmpAgent *pAgent, const EventLogEntry *pEvent);
    virtual ~docsDevTrap();
    
  protected:
    const EventLogEntry *fpEvent;
};

/* DOCSIS 1.1 defined traps from the DOCS-CABLE-DEVICE-TRAP-MIB */

class docsDevCmInitTLVUnknownTrap : public docsDevTrap
{
  public:
    docsDevCmInitTLVUnknownTrap (BcmSnmpAgent *pAgent, const EventLogEntry *pEvent = NULL);
    virtual ~docsDevCmInitTLVUnknownTrap();
};

class docsDevCmDynServReqFailTrap : public docsDevTrap
{
  public:
    docsDevCmDynServReqFailTrap (BcmSnmpAgent *pAgent, const EventLogEntry *pEvent = NULL);
    virtual ~docsDevCmDynServReqFailTrap();
};

class docsDevCmDynServRspFailTrap : public docsDevTrap
{
  public:
    docsDevCmDynServRspFailTrap (BcmSnmpAgent *pAgent, const EventLogEntry *pEvent = NULL);
    virtual ~docsDevCmDynServRspFailTrap();
};

class docsDevCmDynServAckFailTrap : public docsDevTrap
{
  public:
    docsDevCmDynServAckFailTrap (BcmSnmpAgent *pAgent, const EventLogEntry *pEvent = NULL);
    virtual ~docsDevCmDynServAckFailTrap();
};

class docsDevCmBpiInitTrap : public docsDevTrap
{
  public:
    docsDevCmBpiInitTrap (BcmSnmpAgent *pAgent, const EventLogEntry *pEvent = NULL);
    virtual ~docsDevCmBpiInitTrap();
};

class docsDevCmBPKMTrap : public docsDevTrap
{
  public:
    docsDevCmBPKMTrap (BcmSnmpAgent *pAgent, const EventLogEntry *pEvent = NULL);
    virtual ~docsDevCmBPKMTrap();
};

class docsDevCmDynamicSATrap : public docsDevTrap
{
  public:
    docsDevCmDynamicSATrap (BcmSnmpAgent *pAgent, const EventLogEntry *pEvent = NULL);
    virtual ~docsDevCmDynamicSATrap();
};

class docsDevCmDHCPFailTrap : public docsDevTrap
{
  public:
    docsDevCmDHCPFailTrap (BcmSnmpAgent *pAgent, const EventLogEntry *pEvent = NULL);
    virtual ~docsDevCmDHCPFailTrap();
    
  protected:
    BcmMibObject *fpDocsDevServerDhcp;
};

class docsDevCmSwUpgradeInitTrap : public docsDevTrap
{
  public:
    docsDevCmSwUpgradeInitTrap (BcmSnmpAgent *pAgent, const EventLogEntry *pEvent = NULL);
    virtual ~docsDevCmSwUpgradeInitTrap();
    
  protected:
    BcmMibObject *fpDocsDevSwFilename;
    BcmMibObject *fpDocsDevSwServer;
};

class docsDevCmSwUpgradeFailTrap : public docsDevTrap
{
  public:
    docsDevCmSwUpgradeFailTrap (BcmSnmpAgent *pAgent, const EventLogEntry *pEvent = NULL);
    virtual ~docsDevCmSwUpgradeFailTrap();
    
  protected:
    BcmMibObject *fpDocsDevSwFilename;
    BcmMibObject *fpDocsDevSwServer;
};

class docsDevCmSwUpgradeSuccessTrap : public docsDevTrap
{
  public:
    docsDevCmSwUpgradeSuccessTrap (BcmSnmpAgent *pAgent, const EventLogEntry *pEvent = NULL);
    virtual ~docsDevCmSwUpgradeSuccessTrap();
    
  protected:
    BcmMibObject *fpDocsDevSwFilename;
    BcmMibObject *fpDocsDevSwServer;
};

class docsDevCmSwUpgradeCVCFailTrap : public docsDevTrap
{
  public:
    docsDevCmSwUpgradeCVCFailTrap (BcmSnmpAgent *pAgent, const EventLogEntry *pEvent = NULL);
    virtual ~docsDevCmSwUpgradeCVCFailTrap();
    
  protected:
    BcmMibObject *fpDocsDevSwFilename;
    BcmMibObject *fpDocsDevSwServer;
};

class docsDevCmTODFailTrap : public docsDevTrap
{
  public:
    docsDevCmTODFailTrap (BcmSnmpAgent *pAgent, const EventLogEntry *pEvent = NULL);
    virtual ~docsDevCmTODFailTrap();
    
  protected:
    BcmMibObject *fpDocsDevServerTime;
};

class docsDevCmDCCReqFailTrap : public docsDevTrap
{
  public:
    docsDevCmDCCReqFailTrap (BcmSnmpAgent *pAgent, const EventLogEntry *pEvent = NULL);
    virtual ~docsDevCmDCCReqFailTrap();
};

class docsDevCmDCCRspFailTrap : public docsDevTrap
{
  public:
    docsDevCmDCCRspFailTrap (BcmSnmpAgent *pAgent, const EventLogEntry *pEvent = NULL);
    virtual ~docsDevCmDCCRspFailTrap();
};

class docsDevCmDCCAckFailTrap : public docsDevTrap
{
  public:
    docsDevCmDCCAckFailTrap (BcmSnmpAgent *pAgent, const EventLogEntry *pEvent = NULL);
    virtual ~docsDevCmDCCAckFailTrap();
};


class dsgIfStdUpstreamEnabledNotify : public docsDevTrap
{
  public:
    dsgIfStdUpstreamEnabledNotify (BcmSnmpAgent *pAgent, const EventLogEntry *pEvent = NULL);
    virtual ~dsgIfStdUpstreamEnabledNotify();
};


class dsgIfStdUpstreamDisabledNotify : public docsDevTrap
{
  public:
    dsgIfStdUpstreamDisabledNotify (BcmSnmpAgent *pAgent, const EventLogEntry *pEvent = NULL);
    virtual ~dsgIfStdUpstreamDisabledNotify();
};


class dsgIfStdTdsg2TimeoutNotify : public docsDevTrap
{
  public:
    dsgIfStdTdsg2TimeoutNotify (BcmSnmpAgent *pAgent, const EventLogEntry *pEvent = NULL);
    virtual ~dsgIfStdTdsg2TimeoutNotify();
};


#endif

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
//    Filename: standardTrap.h
//    Author:   Kevin O'Neal
//    Creation Date: 24-January-2001
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

#ifndef STANDARDTRAP_H
#define STANDARDTRAP_H

#include "BcmTrap.h"


class BcmNiHalIf;

class coldStartTrap : public BcmTrap
{
  public:
    coldStartTrap (BcmSnmpAgent *pAgent);
    virtual ~coldStartTrap();
};

class warmStartTrap : public BcmTrap
{
  public:
    warmStartTrap (BcmSnmpAgent *pAgent);
    virtual ~warmStartTrap();
};

class linkDownTrap : public BcmTrap
{
  public:
    linkDownTrap (BcmSnmpAgent *pAgent, int IfIndex);
    virtual ~linkDownTrap();
};

class linkUpTrap : public BcmTrap
{
  public:
    linkUpTrap (BcmSnmpAgent *pAgent, int IfIndex);
    virtual ~linkUpTrap();
};

class authFailureTrap : public BcmTrap
{
  public:
    authFailureTrap (BcmSnmpAgent *pAgent);
    virtual ~authFailureTrap();
};

#endif



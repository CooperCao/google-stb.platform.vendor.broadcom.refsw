//****************************************************************************
//
//  Copyright (c) 2003  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16251 Laguna Canyon Road
//          Irvine, California 92618
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//  $Id$
//
//  Filename:       MibFactory.cpp
//  Author:         Kevin O'Neal
//  Creation Date:  June 4, 2003
//
//****************************************************************************
//  Description:
//      This class is responsible for creating the correct type of
//      various MIB objects, based on the OS selection.  In general these
//      are MIB which come from the OS or IP stack, and which vary per
//      OS
//
//****************************************************************************

#include "MibFactory.h"

#if defined(WIN32)
	#include "win32IpBridge.h"
    #include "udpBridge.h"
#elif defined(PSOS)       
	#include "ipBridge.h"
    #include "udpBridge.h"
#elif defined(TARGETOS_vxWorks)
	#include "vxIpBridge.h"
    #include "vxUdpBridge.h"
#elif defined(__linux__)       
    #include "lnxIpBridge.h"
    #include "lnxUdpBridge.h"
#elif defined(TARGETOS_eCos)       
    #include "ecosIpBridge.h"
    #include "ecosUdpBridge.h"
#endif


/*\
 *  RFC-2011 objects
\*/

ipGroupBridge *BcmMibFactory::NewIpGroupBridge (BcmSnmpAgent *pAgent)
{
#if defined(WIN32)
  return new win32IpGroupBridge (pAgent);
#elif defined(PSOS)
  return new ipGroupBridge (pAgent);
#elif defined(TARGETOS_vxWorks)
  return new vxIpGroupBridge (pAgent);
#elif defined(__linux__)       
  return new lnxIpGroupBridge (pAgent);
#elif defined(TARGETOS_eCos)
  return new ecosIpGroupBridge (pAgent);
#endif
}


icmpGroupBridge *BcmMibFactory::NewIcmpGroupBridge (BcmSnmpAgent *pAgent)
{
#if defined(WIN32)
  return new icmpGroupBridge (pAgent);
#elif defined(PSOS)
  return new icmpGroupBridge (pAgent);
#elif defined(TARGETOS_vxWorks)
  return new vxIcmpGroupBridge (pAgent);
#elif defined(__linux__)       
  return new lnxIcmpGroupBridge (pAgent);
#elif defined(TARGETOS_eCos)
  return new ecosIcmpGroupBridge (pAgent);
#endif
}



/*\
 *  RFC-2013 objects
\*/

udpGroupBridge *BcmMibFactory::NewUdpGroupBridge (BcmSnmpAgent *pAgent)
{
#if defined(WIN32)
  return new udpGroupBridge (pAgent);
#elif defined(PSOS)
  return new udpGroupBridge (pAgent);
#elif defined(TARGETOS_vxWorks)
  return new vxUdpGroupBridge (pAgent);
#elif defined(__linux__)       
  return new lnxUdpGroupBridge (pAgent);
#elif defined(TARGETOS_eCos)
  return new ecosUdpGroupBridge (pAgent);
#endif
}

void BcmMibFactory::UpdateUdpTable (BcmSnmpAgent *pAgent)
{
#if defined(WIN32)
  // Nothing to do...
#elif defined(PSOS)
  // Nothing to do...
#elif defined(TARGETOS_vxWorks)
  vxUdpGroupBridge::UpdateUdpTable (pAgent);
#elif defined(__linux__)       
  lnxUdpGroupBridge::UpdateUdpTable (pAgent);
#elif defined(TARGETOS_eCos)
  ecosUdpGroupBridge::UpdateUdpTable (pAgent);
#endif
}

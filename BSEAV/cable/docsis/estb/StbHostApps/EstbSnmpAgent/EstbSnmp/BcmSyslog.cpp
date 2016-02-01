//**************************************************************************
//
//    Copyright 2003  Broadcom Corporation
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
//    Filename: BcmSyslog.cpp
//    Author:   Kevin O'Neal
//    Creation Date: 4-march-2003
//
//**************************************************************************
//    Description:
//
//      Source file for syslog & trap objects, used by event log
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#include "BcmSyslog.h"
#include "SocketFactory.h"
#include "SnmpLog.h"

#include <errno.h>

bool BcmSyslog::fSilent = false;

BcmSyslog::BcmSyslog (const char *pMsg, int Severity, int SrcIf)
{
  fRawMsg = pMsg;
  fSeverity = Severity;
  fIfNum = SrcIf;
  fpSocket = NULL;
}

BcmSyslog::~BcmSyslog ()
{
  if (fpSocket)
    fpSocket->Close();
    
  delete fpSocket;
}

int BcmSyslog::Init ()
{
  // If we don't have a socket, get a new one from the factory.
  if (fpSocket == NULL)
  {
    // Create ourselves a new UDP socket for sysloggin'
    if ((fpSocket = BcmSocketFactory::NewSocket (AF_INET, SOCK_DGRAM, IPPROTO_UDP, fIfNum)) == NULL)
    {
      SnmpLogErr << "BcmSocketFactory failed to create a socket" << endl;
      return -1;
    }
  }
  // If we do, just re-create it.
  else
    fpSocket->Create (AF_INET, SOCK_DGRAM, IPPROTO_UDP, fIfNum);
  
  if (fpSocket->SocketDescriptor() == -1)
    SnmpLogErr << "Can't get syslog socket - " << errno << endl;
  
  return fpSocket->SocketDescriptor();
}

void BcmSyslog::FormatMsg (bool showYear)
{
  char SyslogMessage[512];
  char Text[256];
  
  strncpy (Text, fRawMsg.c_str(), sizeof(Text)-1);
  
  /*\
   *  Format the message, syslog style:
   *  "<%d>%.15s %s[%d]: %s",
   *  <priority, facillity code>, <date time: ctime()+4>,
   *  <process name>, <PID>, <message text>
   * 
   *  Facility code must be 128 per ATP.
   *  AKA LOG_LOCAL0
   * 
   *  Unfortunately, we don't have a good way to get the process (task)
   *  name or ID, so we will use hard-coded values unless we get
   *  complaints.
   * 
   *  Process Name = "SYSLOG"
   *  Process ID = 0
  \*/
  
  time_t Now = time (&Now);
  if(showYear)
  {
      sprintf (SyslogMessage, "<%d>%.20s %s[%d]: %s",
        fSeverity | LOG_LOCAL0, ctime(&Now)+4, "SYSLOG", 0, Text);
  }
  else
  {
      sprintf (SyslogMessage, "<%d>%.15s %s[%d]: %s",
        fSeverity | LOG_LOCAL0, ctime(&Now)+4, "SYSLOG", 0, Text);
  }
  
  fFormattedMsg = SyslogMessage;
}

SNMP_STATUS BcmSyslog::Send (const BcmIpAddress &IpAddr, bool showYear, bool Silent)
{
  struct sockaddr_in SyslogAddr;
  unsigned long lAddr;
  int Result;
  int Flags = 0;


  IpAddr.Get(lAddr);
  
  // If the IP addr is 0.0.0.0, forget about it.
  if (lAddr == 0)
    return SNMP_NO_ERROR;
    
  // Re-initialize my socket.  This is necessary because sockets can't be
  // shared among different threads, and we never know from what thread this
  // is being called.    
  if (Init() == -1)
    return SNMP_GEN_ERR;
    
  // Port 514 is the syslog port (UDP)
  SyslogAddr.sin_family = AF_INET;
  SyslogAddr.sin_port = htons(514);
  SyslogAddr.sin_addr.s_addr = htonl(lAddr);
  
  FormatMsg (showYear);

  if ((Silent == false) && (fSilent == false))
  {
    gLogMessageRaw << "Sending syslog message from IF " << fIfNum << " to " << IpAddr << ":\n  " 
               << fFormattedMsg << endl;
  }
  
  Result = fpSocket->Sendto ((char *)fFormattedMsg.c_str(), fFormattedMsg.size(),
           Flags, (sockaddr *)&SyslogAddr, sizeof (SyslogAddr));

  if (Result == -1)
    SnmpLogWarn << "docsDevEvSyslog::Send - sendto failure " << Result << endl;
  
  // No sense leaving this socket open, just close it.
  fpSocket->Close();

  if (Result == -1)
    return SNMP_GEN_ERR;
  
  return SNMP_NO_ERROR;
}



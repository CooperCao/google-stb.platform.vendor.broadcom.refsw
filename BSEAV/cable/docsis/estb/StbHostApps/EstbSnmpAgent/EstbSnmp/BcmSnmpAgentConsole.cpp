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

//****************************************************************************
//  $Id$
//
//  Filename:       BcmSnmpAgentConsole.cpp
//  Author:         Kevin O'Neal
//  Creation Date:  October 3, 2003
//
//****************************************************************************
//  Description:
//  InstallCommands and CommandHandler methods for BcmSnmpAgent class.
//
//****************************************************************************

//********************** Include Files ***************************************

#include "BcmSnmpAgent.h"
#include "SnmpTrapExploderThread.h"
#include "EstbOsMonitorThread.h"

#include "BoolCommandParameter.h"
#include "IpAddressCommandParameter.h"
#include "SignedIntCommandParameter.h"
#include "StringCommandParameter.h"
#include "StringSetCommandParameter.h"
#include "UnsignedIntCommandParameter.h"

#include "SnmpLog.h"
#include "standardTrap.h"


// The command ids for the commands I can handle.
enum
{
    kTest           /* generic test command for whatever I need at the moment */ = BcmNonVolSettingsCommandTable::kDerivedClassCommands, 
    kDebug,         // Turn SNMP agent core debug on or off
    kAgentCoreShow, // Print agent core things
    kShow,          // Print SNMP agent settings to console
    kGet,           // GET an object
    kSet,           // SET an object
    kFlush,         // Flush a table
    kSeverities,    // print out app-specific severities for SNMP
    kLog,           // turn on/off various debugging
    kSendTrap,      // send a trap
    kThreadInfo,    // print thread state stuff
    kView           // Set the default view used by the agent for V1/V2C
};

// Constants for the command parameters.
enum
{
    kPrimaryParameter = 1,
    kSecondParameter,
    kThirdParameter,
    kFourthParameter
};

/* String set values: */

// kSet
enum
{
    kIntType = 0,
    kUintType,
    kStringType,
    kOidType,
    kIpType
};

// kLog
enum
{
    kLogReq = 0,
    kLogNm,
    kLogFilt,
    kLogEvents,
    kLogThread
};

// kSendTrap
enum
{
  kColdStart = 0,
  kWarmStart,
  kLinkDown,
  kLinkUp,
  kAuthFail,
  kEgpNeighLoss,
  kEnterprise
};

// kFilters
enum
{
  kFiltersOn = 0,
  kFiltersOff,
  kFiltersOptimized
};

void BcmSnmpAgent::InstallCommands ()
{
  static bool CommandsInstalled = false;
  
  // Only install commands once.
  if (CommandsInstalled == true)
    return;
    
  CommandsInstalled = true;

  BcmCommand command;
    
  // Set up fields that don't change.
  command.fIsEnabled = true;
  command.fIsHidden = false;
  command.fCommandFunctionWithParms = GetCommandTableSingleton().CommandHandler;
  
  command.fCommandId = kTest;
  command.pfCommandName = "test";
  command.pfCommandParameterList = new BcmCommandParameterList;
  command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, true, "optional IP", 255, true));
  command.pfCommandHelp = "Test PR 6487.  To reproduce the crash from this PR, follow these steps:\n" \
                          "  1: Run as usual, become operational.\n" \
                          "  2: After going operational, cd to docsis and do an up_dis.\n" \
                          "  3: execute the test command in the SNMP directory.\n" \
                          "This should cause a crash in the PR 6487 style.  Note that if you specify\n" \
                          "an IP address as an argument, that IP will be used as the target.  If none\n" \
                          "is specified, 0.0.0.0 will be used.\n";
  command.pfExamples = "test\t";
  GetCommandTableSingleton().AddCommand(command);

  command.fCommandId = kDebug;
  command.pfCommandName = "debug";
  command.pfCommandParameterList = new BcmCommandParameterList;
  command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kPrimaryParameter, true, NULL, 0, 9));
  command.pfCommandHelp = "Set the debug level of the core agent.";
  command.pfExamples = "debug 0\tSet the core agent to be silent.\n"
                       "debug 9\tSet the core agent to max verbosity.";
  GetCommandTableSingleton().AddCommand(command);
  
  command.fCommandId = kAgentCoreShow;
  command.pfCommandName = "agent_core_show";
  command.pfCommandHelp = "Print agent-core specific info to the console.";
  command.pfExamples = "agent_core_show\t";
  GetCommandTableSingleton().AddCommand(command);

  command.fCommandId = kShow;
  command.pfCommandName = "show";
  command.pfCommandHelp = "Print the SNMP agent's settings to the console.";
  command.pfExamples = "show\t";
  GetCommandTableSingleton().AddCommand(command);

  command.fCommandId = kGet;
  command.pfCommandName = "get";
  command.pfCommandParameterList = new BcmCommandParameterList;
  command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, false, "object.index", 255, true));
  command.pfCommandHelp = "Get the specified SNMP object.\n"
                          "If no index is specified, gets the first instance available.\n"
                          "To query a table entry, use [tablename].[index], not [entryname].[index].";
  command.pfExamples = "get sysDescr.0\t\n"
                       "get ifTable.1\t";
  GetCommandTableSingleton().AddCommand(command);

  command.fCommandId = kSet;
  command.pfCommandName = "set";
  command.pfCommandParameterList = new BcmCommandParameterList;
  command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, false, "object.index", 255, false));
  command.pfCommandParameterList->AddOrderDependent(new BcmStringSetCommandParameter("int|uint|string|oid|ip", kSecondParameter, false));
  command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kThirdParameter, false, "value", 255, true));
  command.pfCommandHelp = "Set the specified SNMP object to the specified value.";
  command.pfExamples = "set sysName.0 STRING device.vendor.com\t";
  GetCommandTableSingleton().AddCommand(command);
  
  command.fCommandId = kFlush;
  command.pfCommandName = "flush";
  command.pfCommandParameterList = new BcmCommandParameterList;
  command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, false, "table_name", 255, true));
  command.pfCommandHelp = "Flush the specified SNMP table.\n";
  GetCommandTableSingleton().AddCommand(command);
  
  command.fCommandId = kSeverities;
  command.pfCommandName = "severities";
  command.pfCommandHelp = "List SNMP message log app-specific severity bits.";
  command.pfExamples = "severities\t";
  GetCommandTableSingleton().AddCommand(command);
  
  command.fCommandId = kLog;
  command.pfCommandName = "log";
  command.pfCommandParameterList = new BcmCommandParameterList;
  command.pfCommandParameterList->AddOrderDependent(new BcmStringSetCommandParameter("req|nm|filt|events|thread", kPrimaryParameter, false));
  command.pfCommandParameterList->AddOrderDependent(new BcmBoolCommandParameter(kSecondParameter, false, "enable"));
  command.pfCommandHelp = "Enable or disable various kinds of SNMP logging.";
  command.pfExamples = "log req true\tEnable SNMP request debug.\n"
                       "log nm true\tEnable SNMP NM authentication debug.\n"
                       "log filt true\tEnable packet filter debug.\n"
                       "log events true\tEnable event log debug.\n"
                       "log thread true\tEnable SNMP thread debug.\n";
  GetCommandTableSingleton().AddCommand(command);

  command.fCommandId = kSendTrap;
  command.pfCommandName = "trap";
  command.pfCommandParameterList = new BcmCommandParameterList;
  command.pfCommandParameterList->AddOrderDependent(new BcmStringSetCommandParameter("coldStart|warmStart|linkDown|linkUp|authFail", kPrimaryParameter, false));
  command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kSecondParameter, true, "snmpVersion", 1, 3));
  command.pfCommandParameterList->AddOrderDependent(new BcmIpAddressCommandParameter(kThirdParameter, true, "destIp"));
  command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kFourthParameter, true, "community", 255, true));
  command.pfCommandHelp = "Send the specified trap type using specified snmpVersion to destIp with specified community string\n"
      "\tsnmpVersion\t2=SNMPv1 trap\n"
      "\tsnmpVersion\t2=SNMPv2c trap\n"
      "\tsnmpVersion\t3=Notify";
  command.pfExamples = "trap coldStart 1 10.24.65.171 public\tSend a COLD_START SNMPv1 trap to 10.24.65.171 with community 'public'.";
  GetCommandTableSingleton().AddCommand(command);

  command.fCommandId = kThreadInfo;
  command.pfCommandName = "thread_info";
  command.pfCommandHelp = "Print the state of the SNMP threads and related info.";
  command.pfExamples = "thread_info\t";
  GetCommandTableSingleton().AddCommand(command);
  
  command.fCommandId = kView;
  command.pfCommandName = "view_v1v2";
  command.pfCommandParameterList = new BcmCommandParameterList;
  command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, false, "community", 255, true));
  command.pfCommandHelp = "Set the view used for SNMPv1/v2c queries for the specified agent.";
  command.pfExamples = "view_v1v2 docsisManagerView\t";
  GetCommandTableSingleton().AddCommand(command);
}

#include "BcmSocket.h"
#include "SocketFactory.h"
void BcmSnmpAgent::CommandHandler (const BcmCommand &command)
{
    const BcmCommandParameter *pParameter = NULL;
    
    // Many commands below have a single primary parameter.  Retrieve it for
    // them.
    if (command.pfCommandParameterList != NULL)
    {
        pParameter = command.pfCommandParameterList->Find(kPrimaryParameter);
    }

    switch (command.fCommandId)
    {
        case kTest:
        {
            BcmIpAddress Target = kAllZerosIp;
            
            if (pParameter->WasParsed())
            {
              const char *pIpStr = pParameter->AsString();
              Target.Set(pIpStr);
            }
        
            SnmpLogRaw << "Testing PR 6487.  Sending packet to " << Target << endl;
            
            char *pTestPkt = (char *) "Testing 1,2,3";
            BcmSocket *pMySock = BcmSocketFactory::NewSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, 2);
            
            pMySock->Sendto (pTestPkt, sizeof(pTestPkt), 0, Target, 4413);
            
            delete pMySock;
/*        
            // Hah, this oughta keep 'em guessing...
            if (pParameter->WasParsed())
              SnmpLogRaw << "Test " << pParameter->AsString() << " passed." << endl;
            else
              SnmpLogRaw << "Test  passed." << endl;
*/
            break;
        }

        case kDebug:
        {
            unsigned int Level;
             
            if (pParameter->WasParsed())
            {
              Level = pParameter->AsUnsignedInt();
              Level = AgentSetDebugLevel (Level);
            }
            else
              Level = AgentGetDebugLevel ();
        
            SnmpLogRaw << "Core agent's (" << AgentGetName() << ") debug level = " << Level << endl;
            break;
        }

        case kShow:
        {
            SnmpLogRaw << *this << endl;
            break;
        }
        
        case kAgentCoreShow:
        {
            AgentShow ();
            break;
        }

        case kGet:
        {
            const char *pParams = pParameter->AsString();
            char *IndexStr;
            BcmString Name;
            BcmObjectId Index, Oid;
            BcmMibObject *pObject;
            BcmMibTable *pTable;
        
        
            IndexStr = (char *) strrchr (pParams, '.');
        
            if ( IndexStr == NULL )
                Index = "0";
            else
            {
                Index = IndexStr;
                *IndexStr = 0;
            }
        
            Name = pParams;
        
            // Look for a table first.
            // If there's no index, maybe this is a table.
            pTable = FindTable (Name);
            if ( pTable )
            {
                BcmMibEntry *pEntry;
        
        
                // If no index was specified, print the whole dang table.
                if ( IndexStr == NULL )
                {
                    //SnmpLogRaw << "Found table " << *pTable << endl;
                    SnmpLogRaw << *pTable << endl;
                    return;
                }
        
                // Otherwise, find and print the entry specified by the index.
                pEntry = pTable->FindEntry (Index);
                if ( pEntry )
                {
                    // SnmpLogRaw << "Found " << Name << " entry " << Index << "\n" << *pEntry << endl;
                    SnmpLogRaw << *pEntry << endl;
                    return;
                }
            }
        
            // Finally, look for an object.  First try the specified index.
            pObject = FindObject (Name, Index);
            if ( pObject )
            {
                //SnmpLogRaw << "Found object " << *pObject << endl;
                SnmpLogRaw << *pObject << endl;
                return;
            }
        
            pObject = FindObject (Name);
            if ( pObject )
            {
                SnmpLogRaw << "First occurence of object " << Name << ":\n" << *pObject << endl;
                return;
            }
        
            // Try a straight OID if the name doesn't work
            Oid = Name;
            Oid += Index;
            pObject = FindObject (Oid);
            if ( pObject )
            {
                SnmpLogRaw << "Object as OID " << Oid << ":" << endl << *pObject << endl;
                return;
            }
        
            // If that fails, try to find the first occurence of the named object.
            SnmpLogRaw << "Failed to find " << Name << endl;
            break;
        }

        case kSet:
        {
            const BcmCommandParameter *pTypeParm = command.pfCommandParameterList->Find(kSecondParameter);
            const BcmCommandParameter *pValParm = command.pfCommandParameterList->Find(kThirdParameter);
            const char *ObjStr = pParameter->AsString();
            char *IndexStr; // Parsed from ObjStr
            unsigned int Type = pTypeParm->AsUnsignedInt();
            const char *ValStr = pValParm->AsString();
            BcmString Name;
            BcmObjectId Oid, Index;
            BcmMibObject *pObject;
            BcmVarBind Vb;
            SNMP_STATUS Status;
        
        
            IndexStr = (char *) strchr (ObjStr, '.');
        
            if ( IndexStr == NULL )
                Index = "0";
            else
            {
                Index = IndexStr;
                *IndexStr = 0;
            }
        
            Name = ObjStr;
        
            // Look for an object.
            pObject = FindObject (Name, Index);
            if ( pObject == NULL )
            {
                SnmpLogRaw << "Failed to find object " << ObjStr << Index << endl;
                break;
            }
        
            // Now load our VB based on the type argument.
            switch (Type)
            {
              case kIntType:
                  Vb.SetInt32 (atoi(ValStr));
                  break;
                
              case kUintType:
                  Vb.SetUint32 (atoi(ValStr));
                  break;
                  
              case kStringType:
                  Vb.SetString (ValStr);
                  break;
                  
              case kOidType:
                  Vb.SetOid (BcmObjectId(ValStr));
                  break;
                  
              case kIpType:
              {
                  BcmIpAddress Ip;
                  
                  Ip.Set (ValStr);
                  Vb.SetIp (Ip);
                  break;
              }
            }
        
            // First test the object and see what gives:
            pObject->GetOid (Oid);
            Status = TestObject (Oid, Vb, false);
        
            if ( Status == (SNMP_NO_ERROR | SNMP_TEST_DONT_SET) )
            {
                SnmpLogRaw << "BcmSnmpAgent::Test OK, but indicated DON'T SET.  All done." << endl;
                break;
            }
            else if ( Status != SNMP_NO_ERROR )
            {
                SnmpLogRaw << "BcmSnmpAgent::Test failed, return code = " << Status << endl;
                break;
            }
        
            // Rather than call pObject->Set directly, do it via a call into the agent.
            // This is more like the way calls proceed from the non-portable agent and
            // is necessary to get things like table entry creation to work.
            Status = SetObject (Oid, Vb, false);
        
            if ( Status == SNMP_NO_ERROR )
            {
                SnmpLogRaw << "Set was successful!" << endl;
        
                // Shoudn't do this just in case we just destroyed the object.
                //SnmpLogRaw << *pObject << endl;
            }
            else
                SnmpLogRaw << "SetFailure: " << Status << endl;
                
            break;
        }
        
        case kFlush:
        {
            const char *pParams = pParameter->AsString();
            BcmString Name;
            BcmMibTable *pTable;
        
        
            Name = pParams;
        
            // Look for the table.
            pTable = FindTable (Name);
            if ( pTable )
            {
              SnmpLogRaw << "Flushing " << Name << endl;
              pTable->Flush();
            }
            else
              SnmpLogRaw << "Couldn't find table " << Name << endl;
            
            break;
        }

        case kSeverities:
        {
            SnmpLogRaw << 
                "SNMP agent app-specific severities:\n"
                "  Startup messages:         0x00000040\n"
                "  Config file messages:     0x00000080\n"
                "  Initialization messages:  0x00000100\n"
                "  Request messsages:        0x00000200\n"
                "  NmAccess messages:        0x00000400\n"
                "  Packet filter messages:   0x00000800\n"
                "  Memory messages:          0x00001000\n"
                "  Search messages:          0x00002000\n"
                "  Event Log messages:       0x00004000\n"
                "  Thread messages:          0x00008000\n"
                "Set these bits using the usual methods from /msgLog.  Or, use the commands\n"
                "log_init, log_req, log_nm, log_flt from /snmp." << endl;
            
                // Examples of how to use logging methods:
                SnmpLogRaw << "All enabled severities should print a message below..." << endl;
            
            //    gAppMsg(BcmSnmpAgent::gMessageLogSettings, "SeveritiesCommand", BcmSnmpAgent::kInitializationMessages)
            //      << "Here is an initialization message!" << endl;    
            
                SnmpLogStart << "Here is a startup message!" << endl;
                SnmpLogCfg << "Here is a config file message!" << endl;
                SnmpLogInit << "Here is an initialization message!" << endl;
                SnmpLogReq << "Here is an request message!" << endl;
                SnmpLogNm << "Here is an NM access message!" << endl;
                SnmpLogFilt << "Here is an packet filter message!" << endl;
            break;
        }

        case kLog:
        {
            const BcmCommandParameter *pEnableParm = command.pfCommandParameterList->Find(kSecondParameter);
            int LogType = pParameter->AsUnsignedInt();
            unsigned long Severities;
        
        
            Severities = BcmSnmpAgent::gMessageLogSettings.GetEnabledSeverities();
            
            if (pEnableParm->AsBool() == true)
            {
                if (LogType == kLogReq)
                {
                  SnmpLogRaw << "Enabling SNMP request debug." << endl;
                  Severities |= BcmMessageLogSettings::kInformationalMessages | BcmSnmpAgent::kRequestMessages;
                }
                else if (LogType == kLogNm)
                {
                  SnmpLogRaw << "Enabling SNMP NM authentication debug." << endl;
                  Severities |= BcmMessageLogSettings::kInformationalMessages | BcmSnmpAgent::kNmAccessMessages;
                }
                else if (LogType == kLogFilt)
                {
                  SnmpLogRaw << "Enabling packet filter debug." << endl;
                  Severities |= BcmMessageLogSettings::kInformationalMessages | BcmSnmpAgent::kPktFilterMessages;
                }
                else if (LogType == kLogEvents)
                {
                  SnmpLogRaw << "Enabling event log debug." << endl;
                  Severities |= BcmMessageLogSettings::kInformationalMessages | BcmSnmpAgent::kEventLogMessages;
                }
                else if (LogType == kLogThread)
                {
                  SnmpLogRaw << "Enabling SNMP thread debug." << endl;
                  Severities |= BcmMessageLogSettings::kInformationalMessages | BcmSnmpAgent::kThreadMessages;
                }
            }
            else
            {
                if (LogType == kLogReq)
                {
                    SnmpLogRaw << "Disabling SNMP request debug." << endl;
                    Severities &= ~BcmSnmpAgent::kRequestMessages;
                }
                else if (LogType == kLogNm)
                {
                    SnmpLogRaw << "Disabling SNMP NM auth debug." << endl;
                    Severities &= ~BcmSnmpAgent::kNmAccessMessages;
                }
                else if (LogType == kLogFilt)
                {
                    SnmpLogRaw << "Disabling packet filter debug." << endl;
                    Severities &= ~BcmSnmpAgent::kPktFilterMessages;
                }
                else if (LogType == kLogEvents)
                {
                    SnmpLogRaw << "Disabling event log debug." << endl;
                    Severities &= ~BcmSnmpAgent::kEventLogMessages;
                }
                else if (LogType == kLogThread)
                {
                    SnmpLogRaw << "Disabling SNMP thread debug." << endl;
                    Severities &= ~BcmSnmpAgent::kThreadMessages;
                }
            }
            
            BcmSnmpAgent::gMessageLogSettings.SetEnabledSeverities(Severities);
            
            break;
        }

        case kSendTrap:
        {
            const BcmCommandParameter *pVersParam = command.pfCommandParameterList->Find(kSecondParameter);
            const BcmCommandParameter *pIpParam = command.pfCommandParameterList->Find(kThirdParameter);
            const BcmCommandParameter *pCmtyParam = command.pfCommandParameterList->Find(kFourthParameter);
            BcmTrap *pTrap;
            
            // If version, IP and community are not specified, just use the 
            // agent's internal mechanisms to send the trap wherever it is 
            // provisioned to be sent.  Eg use snmpTargetAddrTable if the agent 
            // uses that mechanism to send a trap.
            if ((pVersParam->WasParsed() == false)
            ||  (pIpParam->WasParsed() == false)
            ||  (pCmtyParam->WasParsed() == false))
            {
               //printf("SENDING ASYNC\n");
               //SendStandardTrap (pParameter->AsUnsignedInt());
               SendStandardTrap (pParameter->AsUnsignedInt(),true); //send async

               break;
            }
        
            switch ( pParameter->AsUnsignedInt() )
            {
                case kColdStart:
                    pTrap = new coldStartTrap(this);
                    break;
            
                case kWarmStart:
                    pTrap = new warmStartTrap(this);
                    break;
            
                case kLinkDown:
                    pTrap = new linkDownTrap(this, 0);
                    break;
            
                case kLinkUp:
                    pTrap = new linkUpTrap(this, 0);
                    break;
            
                case kAuthFail:
                    pTrap = new authFailureTrap(this);
                    break;
            
                default:
                    SnmpLogRaw << "No standard trap known of type " << pParameter->AsUnsignedInt() << endl;
                    return;
            }
        
            if ( pVersParam->AsUnsignedInt() == 1 )
                pTrap->SendV1Trap (GetIpAddr(), pIpParam->AsIpAddress(), pCmtyParam->AsString());
            else if ( pVersParam->AsUnsignedInt() == 2 )
                pTrap->SendV2Trap (GetIpAddr(), pIpParam->AsIpAddress(), pCmtyParam->AsString());
            else if ( pVersParam->AsUnsignedInt() )
                pTrap->SendNotify (GetIpAddr());
            else
                SnmpLogRaw << pVersParam->AsUnsignedInt() << " is not a valid trap version: 1, 2, or 3" << endl;
        
            delete pTrap;

            break;
        }

        case kThreadInfo:
        {
            SnmpLogRaw << BcmSnmpThread::Singleton() << endl;
            SnmpLogRaw << BcmSnmpTrapExploderThread::Singleton() << endl;
            // SnmpLogRaw << BcmEstbOsMonitorThread::Singleton() << endl;
            break;
        }
        
        case kView:
        {
            SetV1V2View (pParameter->AsString());
            break;
        }

        default:
            gLogMessageRaw << "WARNING - unknown command id (" << command.fCommandId << ")!  Ignoring...\n";
            break;
    }
}

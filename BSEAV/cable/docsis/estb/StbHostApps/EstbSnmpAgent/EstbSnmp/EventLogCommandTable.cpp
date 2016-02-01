//****************************************************************************
//
//  Copyright (c) 2000  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//
//  Filename:       EventLogCommandTable.cpp
//  Author:         David Pullen
//  Creation Date:  April 11, 2000
//
//****************************************************************************
//  Description:
//      This derived class handles SNMP commands.
//
//****************************************************************************

//********************** Include Files ***************************************

#include "EventLogCommandTable.h"
#include "EventLog.h"
#include "BcmSnmpAgent.h"
#include "SnmpLog.h"
#include "IpAddressCommandParameter.h"
#include "StringCommandParameter.h"
#include "UnsignedIntCommandParameter.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

static const char *pgTableDescr = "Commands to manipulate the Event Log";

// Constants for the commands.
enum
{
    kShow = 0,          // Dump event log to console
    kLogEvent,          // Log an event
    kLogGenericEvent,   // Log a generic event
    kFlushLog,          // Flush the event log
    kSetSyslogServer,   // Set the IP address of the syslog server
    kReadFromNV,        // Reload the event table from NV storage
    kControl,           // Show the control table
    kStress             // Invoke stress test
};

// Constants for the command parameters.
enum
{
    kPrimaryParameter = 1,
    kSecondParameter,
    kThirdParameter,
    kFourthParameter,
    kFifthParameter,
    kSixParameter,
};

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


// Default Constructor.  Creates the command table, adds all of the
// commands to it and sets the command handler function.
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmEventLogCommandTable::BcmEventLogCommandTable(void) :
    BcmCommandTable("Event Log commands", "event_log", "Event Log", pgTableDescr)
{
    BcmCommand command;

    // Set up fields that don't change.
    command.fIsEnabled = true;
    command.fIsHidden = false;
    command.fCommandFunctionWithParms = CommandHandler;
    
    command.fCommandId = kShow;
    command.pfCommandName = "show";
    command.pfCommandHelp = "Dump the contents of the event log to the console.";
    command.pfExamples = "show\t";
    AddCommand(command);
    
    command.fCommandId = kControl;
    command.pfCommandName = "control";
    command.pfCommandHelp = "Show or modify the contents of the control table.";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kPrimaryParameter, true, "level", 0, 8));
    command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kSecondParameter, true, "reporting", 0, 255));
    command.pfExamples = "control\tshow the control table\n"
                         "control  1 0x80\tset reporting for level 1 to 0x80\n"
                         "control  0 0x00\tset reporting for all levels to 0\n";
    AddCommand(command);
    
    command.fCommandId = kLogEvent;
    command.pfCommandName = "log_event";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, false, "eventId", 16));
    command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kSecondParameter, true, "evParm1", 128));
    command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kThirdParameter, true, "evParm2", 128));
    command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kFourthParameter, true, "evParm3", 128));
    command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kFifthParameter, true, "evParm4", 128));
    command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kSixParameter, true, "evParm5", 128));
    command.pfCommandHelp = "Log an event with the specified event id to the event log.  The event may have "
                            "up to 4 event-specific text parameters; if your parameter has a space, then "
                            "enclose it in quotes (e.g. \"this is parm 1\").";
    command.pfExamples = "log_event E102.0 basic.cfg\tLogs event E102.0, with the cfg file as evParm1.";
    AddCommand(command);
/*    
    command.fCommandId = kLogGenericEvent;
    command.pfCommandName = "log_generic_event";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kPrimaryParameter, false, "level", 1, 8));
    command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kSecondParameter, false, "text", 128, true));
    command.pfCommandHelp = "Log a generic event of the specified severity level to the event log.";
    command.pfExamples = "log_generic_event 1 this is an example event\t";
    AddCommand(command);
*/  
    command.fCommandId = kFlushLog;
    command.pfCommandName = "flush";
    command.pfCommandHelp = "Flush the contents of the event log, including stored events.";
    command.pfExamples = "flush\t";
    AddCommand(command);
    
    command.fCommandId = kSetSyslogServer;
    command.pfCommandName = "syslog";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmIpAddressCommandParameter(kPrimaryParameter, true));
    command.pfCommandHelp = "Set the IP address of the syslog server.  Use 0.0.0.0 to inhibit.";
    command.pfExamples = "syslog 11.24.4.3\t";
    AddCommand(command);
    
    command.fCommandId = kReadFromNV;
    command.pfCommandName = "read";
    command.pfCommandHelp = "Read the event log from NV storage and rebuild the table.";
    command.pfExamples = "read\t";
    AddCommand(command);
    
    command.fCommandId = kStress;
    command.pfCommandName = "stress";
    command.pfCommandHelp = "Start a stress test for the selected event log object.";
    command.pfExamples = "stress\t";
    AddCommand(command);
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmEventLogCommandTable::~BcmEventLogCommandTable()
{
    // Nothing to do...
}


// This is the entrypoint that is called whenever one of my commands is
// entered.
//
// Parameters:
//      pInstanceValue - the instance value to which the command should be
//                       applied.  Instances must be registered with the
//                       command table in order for them to receive the
//                       command.
//      command - the command (with parameters) that was entered by the user.
//
// Returns:  Nothing.
//
void BcmEventLogCommandTable::CommandHandler(void *pInstanceValue, 
                                             const BcmCommand &command)
{
    const BcmCommandParameter *pParameter;
    EventLog *pLog = (EventLog *) pInstanceValue;
    
    // Many commands below have a single primary parameter.  Retrieve it for
    // them.
    if (command.pfCommandParameterList != NULL)
    {
        pParameter = command.pfCommandParameterList->Find(kPrimaryParameter);
    }

    switch (command.fCommandId)
    {
        case kShow:
                gLogMessageRaw << *pLog << endl;
            break;
            
        case kControl:
        {
          unsigned long NumArgs = 0, Arg1, Arg2;
          
          const BcmCommandParameter *pControlParm1 = pParameter;
          const BcmCommandParameter *pControlParm2 = command.pfCommandParameterList->Find(kSecondParameter);
          
          if (pControlParm1->WasParsed())
          {
              Arg1 = pControlParm1->AsUnsignedInt();
              NumArgs++;
          }
          if (pControlParm2->WasParsed())
          {
              Arg2 = pControlParm2->AsUnsignedInt();
              NumArgs++;
          }
          
          if (NumArgs == 2)
          {
            // Convert SNMP bits to regular bitmask..
            unsigned char Byte = Arg2;
#if (SNMP_SUPPORT)
            BitsMibObject Bits (".1.2.3", "temp");
            BcmVarBind VB;
            VB.SetString (&Byte, 1);
            Bits.Set(VB);
            Bits.Get(VB);
            VB.GetBits (&Arg2);
#endif
            if (Arg1 != 0)
            {
              SnmpLogRaw << "Setting reporting for severity level " << Arg1 << " to " << inHex(Byte) << endl;
              pLog->SetLevelControl (Arg1, Arg2);
            }
            else
            {
              SnmpLogRaw << "Setting reporting for all severity levels to " << inHex(Byte) << endl;
              for (int Loop = 1; Loop <= 8; Loop++)
                pLog->SetLevelControl (Loop, Arg2);
            }
          }
          else if (NumArgs == 0)
          {
            pLog->PrintControl (SnmpLogRaw);
          }  
          
          break;
        }
            
        case kLogEvent:
            {
                const BcmCommandParameter *pEventParm1 = command.pfCommandParameterList->Find(kSecondParameter);
                const BcmCommandParameter *pEventParm2 = command.pfCommandParameterList->Find(kThirdParameter);
                const BcmCommandParameter *pEventParm3 = command.pfCommandParameterList->Find(kFourthParameter);
                const BcmCommandParameter *pEventParm4 = command.pfCommandParameterList->Find(kFifthParameter);
                const BcmCommandParameter *pEventParm5 = command.pfCommandParameterList->Find(kSixParameter);

                const char *pEventParm1String = NULL;
                const char *pEventParm2String = NULL;
                const char *pEventParm3String = NULL;
                const char *pEventParm4String = NULL;
                const char *pEventParm5String = NULL;

                unsigned int eventId;
                char eventLetter = ' ';
                unsigned int eventMajor = 10000, eventMinor = 100;
                
                // If first character is not numeric, parse and convert the
                // DOCSIS event code
                if ((pParameter->AsString()[0] < '0')
                ||  (pParameter->AsString()[0] > '9'))
                {
                  // Convert the event id string to the numerical equivalent.
                  // The parameter values have been primed with illegal values so
                  // that the sscanf will fail if the user didn't enter all of the
                  // id string, or if an illegal string was entered.
                  sscanf(pParameter->AsString(), "%c%d.%d", &eventLetter, &eventMajor, &eventMinor);

                  if ((eventLetter < 'A') || (eventLetter > 'Z') ||
                      (eventMajor > 9999) || (eventMinor > 99))
                  {
                      gLogMessageRaw << "ERROR:  Invalid event id '" << pParameter->AsString() << "'!\n";

                      break;
                  }
                  
                  eventId = (((unsigned int) eventLetter) * 1000000) +
                            (eventMajor * 100) + eventMinor;
                }
                // Otherwise, just grab the numeric value.
                else
                  sscanf (pParameter->AsString(), "%u", &eventId);
                          
                // If the optional parameters are present, then store pointers
                // to the strings so that we can pass NULL/non NULL.
                if (pEventParm1->WasParsed())
                {
                    pEventParm1String = pEventParm1->AsString();
                }
                if (pEventParm2->WasParsed())
                {
                    pEventParm2String = pEventParm2->AsString();
                }
                if (pEventParm3->WasParsed())
                {
                    pEventParm3String = pEventParm3->AsString();
                }
                if (pEventParm4->WasParsed())
                {
                    pEventParm4String = pEventParm4->AsString();
                }
                if (pEventParm5->WasParsed())
                {
                    pEventParm5String = pEventParm5->AsString();
                }
                
                if (pEventParm1String == NULL)
                {
                  SnmpLogRaw << "No args specified.  Using decoded objects." << endl;
                  
                  if (pLog->LogEvent((EventLogIdCode) eventId, kUseDefaultLevel, NULL, kUseDefaultPolicy) != SNMP_NO_ERROR)
                    gLogMessageRaw << "LogEvent failure" << endl;
                  else
                    gLogMessageRaw << "Event logged successfully" << endl;
                }
                else
                {
                  SnmpLogRaw << "Args specified.  Using replaced objects." << endl;
                
                  // Log the event with the parameters that were parsed.
                  if (pLog->LogEvent((EventLogIdCode) eventId, pEventParm1String,
                                   pEventParm2String, pEventParm3String, pEventParm4String, pEventParm5String) != SNMP_NO_ERROR)
                      gLogMessageRaw << "LogEvent failure" << endl;
                  else
                      gLogMessageRaw << "Event logged successfully" << endl;
                }
            }
            break;
            
        case kLogGenericEvent:
            {
                const BcmCommandParameter *pEventTextParm = command.pfCommandParameterList->Find(kSecondParameter);

                // Log this as a generic event, with the specified severity
                // level and text.
                if (pLog->LogEvent((EventLogIdCode) 0, 
                                        (EventLogSeverityLevel) (pParameter->AsUnsignedInt()), 
                                        pEventTextParm->AsString(), kUseDefaultPolicy) != SNMP_NO_ERROR)
                {
                    gLogMessageRaw << "LogEvent failure" << endl;
                }
                else
                {
                    gLogMessageRaw << "Event logged successfully" << endl;
                }
            }
            break;
            
        case kFlushLog:
            gLogMessageRaw << "Flushing the event log." << endl;
            pLog->ResetLog();
            break;
            
        case kSetSyslogServer:
            if (pParameter->WasParsed())
            {
                pLog->SetSyslogIp(pParameter->AsIpAddress());
            }

            gLogMessageRaw << "SysLog server IP address is " << pLog->GetSyslogIp() << endl;
            break;
            
        case kReadFromNV:
            gLogMessageRaw << "Reading event log from NV storage." << endl;
            pLog->InitFromNonVol();
            break;
            
        case kStress:
#ifdef NOTDEF
            pLog->StressTest();
            break;
#else
          {
            const BcmCommandParameter *pSilent = command.pfCommandParameterList->Find(kSecondParameter);
            bool Silent = false;
            unsigned int NumEvents = 0;

            if (pParameter->WasParsed())
              NumEvents = pParameter->AsUnsignedInt();

            if (pSilent->WasParsed())
              Silent = pSilent->AsBool();

            pLog->StartStressTest(NumEvents, Silent);
            break;
          }
#endif
        
        default:
            gLogMessageRaw << "BcmEventLogCommandTable::CommandHandler: WARNING - unknown command id (" << command.fCommandId << ")!  Ignoring..." << endl;
            break;
    }
}



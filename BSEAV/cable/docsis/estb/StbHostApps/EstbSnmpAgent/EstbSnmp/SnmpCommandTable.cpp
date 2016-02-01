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
//  $Id$
//
//  Filename:       BcmSnmpCommandTable.cpp
//  Author:         Kevin O'Neal
//  Creation Date:  Originally April 18, 2000
//                  Substantially re-written October 2, 2003
//
//****************************************************************************
//  Description:
//      SNMP commands
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "SnmpCommandTable.h"

#include "BcmSnmpAgent.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

static const char *pgTableDescr = "SNMP commands.";


//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


// Default Constructor.  Creates the command table, adds all of the
// commands to it and sets the command handler function.
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmSnmpCommandTable::BcmSnmpCommandTable(void) :
    BcmCommandTable("SNMP Commands", "snmp", "SNMP", pgTableDescr)
{
    // Normally, we would install the command table commands here.
    // However, this table is a little peculiar in that the various
    // BcmSnmpAgent objects install the commands in their InstallCommands
    // methods.  The base BcmSnmpAgent::InstallCommands method is in this
    // module.
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmSnmpCommandTable::~BcmSnmpCommandTable()
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
void BcmSnmpCommandTable::CommandHandler(void *pInstanceValue, 
                                         const BcmCommand &command)
{
    // By convention, the settings object registers its instance so that the
    // user value is the 'this' pointer.  This cast should be safe.
    BcmSnmpAgent *pAgent = (BcmSnmpAgent *) pInstanceValue;

    if (pAgent == NULL)
    {
        gLogMessageRaw << "BcmSnmpCommandTable::CommandHandler: ERROR - settings pointer is NULL!\n";

        return;
    }
    
    // Normally, we'd handle commands here.  However, this object is a little
    // unusual in that various BcmSnmpAgent objects can install and handle
    // commands in this table.  The base class BcmSnmpAgent::CommandHandler
    // method is in this same module.
    
    pAgent->CommandHandler (command);
}


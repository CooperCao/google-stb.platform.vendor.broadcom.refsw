//****************************************************************************
//
// Copyright (c) 2010 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized License, Broadcom grants no license
// (express or implied), right to use, or waiver of any kind with respect to
// the Software, and Broadcom expressly reserves all rights in and to the
// Software and all intellectual property rights therein.  IF YOU HAVE NO
// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
// SOFTWARE.
//
// Except as expressly set forth in the Authorized License,
//
// 1.     This program, including its structure, sequence and organization,
// constitutes the valuable trade secrets of Broadcom, and you shall use all
// reasonable efforts to protect the confidentiality thereof, and to use this
// information only in connection with your use of Broadcom integrated circuit
// products.
//
// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//****************************************************************************
//  $Id$
//
//  Filename:       TrapHandlerCommandTable.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  10 JUL 2010
//
//****************************************************************************
//  Description:
//      Trap handler commands
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "TrapHandlerCommandTable.h"

#include "BcmSnmpTrapHandler.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

static const char *pgTableDescr = "Trap Handler commands.";


//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


// Default Constructor.  Creates the command table, adds all of the
// commands to it and sets the command handler function.
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmTrapHandlerCommandTable::BcmTrapHandlerCommandTable(void) :
    BcmCommandTable("Trap Handler Commands", "trap", "Trap Handler", pgTableDescr)
{
cout << "****MOE**** BcmTrapHandlerCommandTable::BcmTrapHandlerCommandTable" << endl;
    // Normally, we would install the command table commands here.
    // However, this table is a little peculiar in that the various
    // BcmTrapHandler objects install the commands in their InstallCommands
    // methods.  The base BcmTrapHandlerAgent::InstallCommands method is in this
    // module.
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmTrapHandlerCommandTable::~BcmTrapHandlerCommandTable()
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
void BcmTrapHandlerCommandTable::CommandHandler(void *pInstanceValue, 
                                         const BcmCommand &command)
{
    // By convention, the settings object registers its instance so that the
    // user value is the 'this' pointer.  This cast should be safe.
    BcmSnmpTrapHandler *pHandler = (BcmSnmpTrapHandler *) pInstanceValue;

    if (pHandler == NULL)
    {
        gLogMessageRaw << "BcmTrapHandlerCommandTable::CommandHandler: ERROR - settings pointer is NULL!\n";

        return;
    }
    
    // Normally, we'd handle commands here.  However, this object is a little
    // unusual in that various BcmTrapHandlerAgent objects can install and handle
    // commands in this table.  The base class BcmTrapHandlerAgent::CommandHandler
    // method is in this same module.
    
    pHandler->CommandHandler (command);
}


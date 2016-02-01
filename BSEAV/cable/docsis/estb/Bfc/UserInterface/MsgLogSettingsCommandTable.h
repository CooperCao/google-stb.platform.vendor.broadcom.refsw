//****************************************************************************
//
//  Copyright (c) 2000, 2001  Broadcom Corporation
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
//  $Id: MsgLogSettingsCommandTable.h 1.4 2001/08/02 20:11:07Z dpullen Release $
//
//  Filename:       MsgLogSettingsCommandTable.h
//  Author:         David Pullen
//  Creation Date:  Feb 27, 2000
//
//****************************************************************************
//  Description:
//      This derived class handles set-up of the Message Log Settings commands
//      in a command table.  This is used by the BcmMessageLogSettings class.
//
//****************************************************************************

#ifndef MSGLOGSETTINGSCOMMANDTABLE_H
#define MSGLOGSETTINGSCOMMANDTABLE_H

//********************** Include Files ***************************************

// My base class.
#include "CommandTable.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

#if (BCM_TELNET_SUPPORT || BCM_HTTP_SUPPORT)

class BcmRemoteAccessCommandTable;

#endif

#if ENABLE_CALL_TRACING

class BcmCallTraceCommandTable;

#endif

//********************** Class Declaration ***********************************


class BcmMsgLogSettingsCommandTable : public BcmCommandTable
{
public:

    // Default Constructor.  Creates the command table, adds all of the
    // commands to it and sets the command handler function.
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmMsgLogSettingsCommandTable(void);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmMsgLogSettingsCommandTable();

protected:

private:

    #if (BCM_TELNET_SUPPORT || BCM_HTTP_SUPPORT)

    BcmRemoteAccessCommandTable *pfRemoteAccessCommandTable;

    #endif
    
    #if ENABLE_CALL_TRACING
    
    BcmCallTraceCommandTable *pfCallTraceCommandTable;

    #endif
    
private:

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
    static void CommandHandler(void *pInstanceValue, const BcmCommand &command);

private:

    // Copy Constructor.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmMsgLogSettingsCommandTable(const BcmMsgLogSettingsCommandTable &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmMsgLogSettingsCommandTable & operator = (const BcmMsgLogSettingsCommandTable &otherInstance);

};


//********************** Inline Method Implementations ***********************


#endif



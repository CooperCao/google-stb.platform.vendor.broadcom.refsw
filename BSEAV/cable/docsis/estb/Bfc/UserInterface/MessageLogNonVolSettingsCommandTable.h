//****************************************************************************
//
//  Copyright (c) 2000  Broadcom Corporation
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
//  $Id: MessageLogNonVolSettingsCommandTable.h 1.4 2005/12/06 17:54:45Z twhite Release $
//
//  Filename:       MessageLogNonVolSettingsCommandTable.h
//  Author:         David Pullen
//  Creation Date:  Nov 3, 2000
//
//****************************************************************************
//  Description:
//      This derived class handles set-up of the Message Log non-volatile
//      settings commands in a command table.
//
//****************************************************************************

#ifndef MESSAGELOGNONVOLSETTINGSCOMMANDTABLE_H
#define MESSAGELOGNONVOLSETTINGSCOMMANDTABLE_H

//********************** Include Files ***************************************

// My base class.
#include "NonVolSettingsCommandTable.h"

#if (BFC_INCLUDE_HTTP_SERVER_SUPPORT)
#include "HttpServerThread.h"
#endif

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************


// Hey, cool, is this a new record for a non-template class name?  :)
class BcmMessageLogNonVolSettingsCommandTable : public BcmNonVolSettingsCommandTable
{
public:

    // Default Constructor.  Creates the command table, adds all of the
    // commands to it and sets the command handler function.
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmMessageLogNonVolSettingsCommandTable(void);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmMessageLogNonVolSettingsCommandTable();

    // Sets accessor for HTTP server
    //
    // Parameters:  pHttpServer - Http server instance 
    //
    // Returns:  N/A
    //
    #if (BFC_INCLUDE_HTTP_SERVER_SUPPORT)
    static void SetHttpServer(BcmHttpServerThread * pHttpServer);
    #endif

protected:

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
    BcmMessageLogNonVolSettingsCommandTable(const BcmMessageLogNonVolSettingsCommandTable &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmMessageLogNonVolSettingsCommandTable & operator = (const BcmMessageLogNonVolSettingsCommandTable &otherInstance);

private:
    #if (BFC_INCLUDE_HTTP_SERVER_SUPPORT)
    static BcmHttpServerThread * pfHttpServer;
    #endif
};


//********************** Inline Method Implementations ***********************


#endif



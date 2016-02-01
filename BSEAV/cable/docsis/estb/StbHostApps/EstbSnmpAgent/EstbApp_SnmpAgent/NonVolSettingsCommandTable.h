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
//  $Id$
//
//  Filename:       NonVolSettingsCommandTable.h
//  Author:         David Pullen
//  Creation Date:  Oct 31, 2000
//
//****************************************************************************
//  Description:
//      This derived class handles set-up of the commands common to all of the
//      non-volatile settings command table (read/write/default/show_all).
//      Other non-vol settings command tables will generally be derived from
//      this one so that the same set of commands will be available across the
//      board.
//
//****************************************************************************

#ifndef NONVOLSETTINGSCOMMANDTABLE_H
#define NONVOLSETTINGSCOMMANDTABLE_H

//********************** Include Files ***************************************

// My base class.
#include "CommandTable.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************


class BcmNonVolSettingsCommandTable : public BcmCommandTable
{
public:

    // Default Constructor.  Creates the command table, adds all of the common
    // commands to it and sets the command handler function.  The derived class
    // will add its custom command after this.
    //
    // Parameters:
    //      pTableName - the string name to be displayed for this command table.
    //      pTableCommandName - the name of the "command" that will be displayed
    //                          and used when the user types "cd" or "help".
    //      pCommandPrompt - the command prompt to be displayed when this table
    //                       is active.
    //      pTableDescription - descriptive text telling what kinds of commands
    //                          are in this table.
    //
    // Returns:  N/A
    //
    BcmNonVolSettingsCommandTable(const char *pTableName = "Non-Volatile Settings Commands", 
                                  const char *pTableCommandName = "non-vol",
                                  const char *pCommandPrompt = "NonVol", 
                                  const char *pTableDescription = "Commands to change the non-volatile settings for the application.");

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmNonVolSettingsCommandTable();

    // Constants for the commands.
    enum
    {
        kWriteSettings = 0,         // Writes the settings to NonVol memory.
        kReadSettings,              // Reads the settings from NonVol memory.
        kRestoreDefaults,           // Restores the default settings.
        kShowSettings,              // Displays the current settings.
        kFlushUnrecognized,         // Gets rid of any unrecognized settings groups from a composite object.
        kTftpReadSettings,          // Reads the settings from a TFTP server.
        kPrintStats,                // Prints stats on buffer usage.
        kClearDevice,               // Erases the nonvol data on the device.
        kFlushExtraData,            // Gets rid of any extra data that is stored (PR925).

        // The derived class should start numbering its commands here.
        kDerivedClassCommands = 256
    };
    
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
    BcmNonVolSettingsCommandTable(const BcmNonVolSettingsCommandTable &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmNonVolSettingsCommandTable & operator = (const BcmNonVolSettingsCommandTable &otherInstance);

};


//********************** Inline Method Implementations ***********************


#endif



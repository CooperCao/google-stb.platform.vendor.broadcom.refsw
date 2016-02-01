//****************************************************************************
//
//  Copyright (c) 2000-2003  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92618
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//  $Id: NonVolSettingsCommandTable.h 1.12 2004/04/14 21:51:47Z dpullen Release $
//
//  Filename:       NonVolSettingsCommandTable.h
//  Author:         David Pullen
//  Creation Date:  Oct 31, 2000
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


/** \ingroup BfcNonVol
*
*      This derived class handles set-up of the commands common to all of the
*      non-volatile settings command table (read/write/default/show_all).
*      Other non-vol settings command tables will generally be derived from
*      this one so that the same set of commands will be available across the
*      board.
*/
class BcmNonVolSettingsCommandTable : public BcmCommandTable
{
public:

    /// Default Constructor.  Creates the command table, adds all of the common
    /// commands to it and sets the command handler function.  The derived class
    /// will add its custom command after this.
    ///
    /// \param
    ///      pTableName - the string name to be displayed for this command table.
    /// \param
    ///      pTableCommandName - the name of the "command" that will be displayed
    ///                          and used when the user types "cd" or "help".
    /// \param
    ///      pCommandPrompt - the command prompt to be displayed when this table
    ///                       is active.
    /// \param
    ///      pTableDescription - descriptive text telling what kinds of commands
    ///                          are in this table.
    ///
    BcmNonVolSettingsCommandTable(const char *pTableName = "Non-Volatile Settings Commands", 
                                  const char *pTableCommandName = "non-vol",
                                  const char *pCommandPrompt = "NonVol", 
                                  const char *pTableDescription = "Commands to change the non-volatile settings for the application.");

    /// Destructor.  Frees up any memory/objects allocated, cleans up internal
    /// state.
    ///
    virtual ~BcmNonVolSettingsCommandTable();

    /// Constants for the commands.
    enum
    {
        kWriteSettings = 0,         ///< Writes the settings to NonVol memory.
        kReadSettings,              ///< Reads the settings from NonVol memory.
        kRestoreDefaults,           ///< Restores the default settings.
        kIsManufactured,            ///< Checks the settings for manufacturing completeness
        kShowSettings,              ///< Displays the current settings.
        kFlushUnrecognized,         ///< Gets rid of any unrecognized settings groups from a composite object.
        kTftpReadSettings,          ///< Reads the settings from a TFTP server.
        kTftpWriteSettings,         ///< Writes the settings to a TFTP server.
        kPrintStats,                ///< Prints stats on buffer usage.
        kClearDevice,               ///< Erases the nonvol data on the device.
        kFlushExtraData,            ///< Gets rid of any extra data that is stored (PR925).

        /// The derived class should start numbering its commands here.
        kDerivedClassCommands = 256
    };
    
protected:

private:

    /// This is the entrypoint that is called whenever one of my commands is
    /// entered.
    ///
    /// \param
    ///      pInstanceValue - the instance value to which the command should be
    ///                       applied.  Instances must be registered with the
    ///                       command table in order for them to receive the
    ///                       command.
    /// \param
    ///      command - the command (with parameters) that was entered by the user.
    ///
    static void CommandHandler(void *pInstanceValue, const BcmCommand &command);

private:

    /// Copy Constructor.  Not supported.
    BcmNonVolSettingsCommandTable(const BcmNonVolSettingsCommandTable &otherInstance);

    /// Assignment operator.  Not supported.
    BcmNonVolSettingsCommandTable & operator = (const BcmNonVolSettingsCommandTable &otherInstance);

};


//********************** Inline Method Implementations ***********************


#endif



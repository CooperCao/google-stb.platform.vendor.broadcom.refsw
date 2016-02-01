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
//  $Id: NonVolSettingsCommandTable.cpp 1.19 2004/04/15 18:42:57Z dpullen Release $
//
//  Filename:       NonVolSettingsCommandTable.cpp
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

//********************** Include Files ***************************************

// My api and definitions...
#include "NonVolSettingsCommandTable.h"

#include "NonVolSettings.h"

#include "NonVolDevice.h"

#include "BoolCommandParameter.h"
#include "FlagCommandParameter.h"
#include "IpAddressCommandParameter.h"
#include "StringCommandParameter.h"
#include "StringSetCommandParameter.h"

#if (BCM_TFTP_SUPPORT || BFC_INCLUDE_TFTP_SUPPORT)
    #include "IpHelperFactory.h"
    #include "Tftp.h"
#endif

//********************** Local Types *****************************************

//********************** Local Constants *************************************

// Constants for the command parameters.
enum
{
    kPrimaryParameter = 1,
    kGroupOnlyFlag,
    kIpAddressParameter,
    kFilenameParameter,
};

//********************** Local Variables *************************************

//********************** External Variables **********************************

//********************** Class Method Implementations ************************


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
BcmNonVolSettingsCommandTable::BcmNonVolSettingsCommandTable(const char *pTableName, 
                                                             const char *pTableCommandName,
                                                             const char *pCommandPrompt, 
                                                             const char *pTableDescription) :
    BcmCommandTable(pTableName, pTableCommandName, pCommandPrompt, pTableDescription)
{
    BcmCommand command;

    // Set up fields that don't change.
    command.fIsEnabled = true;
    command.fIsHidden = false;
    command.fCommandFunctionWithParms = CommandHandler;

    command.fCommandId = kWriteSettings;
    command.pfCommandName = "write";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmStringSetCommandParameter("both|perm|dyn", kPrimaryParameter, true));
    command.pfCommandHelp = 
        "Writes the specified section of non-volatile settings to non-volatile memory.  "
        "If 'both' is specified or there are no parameters, then both Permanent and "
        "Dynamic settings will be written.  Otherwise, only the section specified will "
        "be written.";
    command.pfExamples = 
        "write\tWrites both Permanent and Dynamic sections of settings.\n"
        "write perm\tWrites only the Permanent section of non-vol settings.";
    AddCommand(command);

    command.fCommandId = kReadSettings;
    command.pfCommandName = "read";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmStringSetCommandParameter("both|perm|dyn", kPrimaryParameter, true));
    command.pfCommandHelp = 
        "Reads the specified section of non-volatile settings from non-volatile memory.  "
        "If 'both' is specified or there are no parameters, then both Permanent and "
        "Dynamic settings will be read.  Otherwise, only the section specified will be "
        "read.  This will destroy any changes you have made if you haven't written them!";
    command.pfExamples = 
        "read\tReads both Permanent and Dynamic sections of settings.\n"
        "read perm\tReads only the Permanent section of non-vol settings.";
    AddCommand(command);

    command.fCommandId = kShowSettings;
    command.pfCommandName = "show";
    command.pfCommandHelp = 
        "Displays the current non-volatile settings values for both the Permanent and "
        "Dynamic sections.";
    command.pfExamples = 
        "show\t";
    AddCommand(command);

    command.fCommandId = kRestoreDefaults;
    command.pfCommandName = "defaults";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmStringSetCommandParameter("both|perm|dyn", kPrimaryParameter, true));
    command.pfCommandHelp = 
        "Restores the settings to default values.  NOTE:  This does not write to NonVol "
        "memory!  You must do this manually.  Also note that some settings do not have "
        "good defaults, particularly the Permanent section!  Use this command with "
        "extreme care!";
    command.pfExamples = 
        "defaults\tSets both Permanent and Dynamic sections to defaults.\n"
        "defaults perm\tSets only the Permanent section to defaults.";
    AddCommand(command);
    
    command.fCommandId = kIsManufactured;
    command.pfCommandName = "manufactured";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmStringSetCommandParameter("both|perm|dyn", kPrimaryParameter, true));
    command.pfCommandHelp = 
        "Checks to see if the specified section is manufactured.";
    command.pfExamples = 
        "manufactured\tChecks both Permanent and Dynamic sections manufacturing state.\n"
        "manufactured perm\tChecks only the Permanent section manufacturing state.";
    AddCommand(command);

    #if (BCM_TFTP_SUPPORT || BFC_INCLUDE_TFTP_SUPPORT)

    command.fCommandId = kTftpReadSettings;
    command.fIsEnabled = true;
    command.pfCommandName = "tftp_read";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderIndependent(new BcmFlagCommandParameter('g', kGroupOnlyFlag));
    command.pfCommandParameterList->AddOrderDependent(new BcmStringSetCommandParameter("perm|dyn", kPrimaryParameter));
    command.pfCommandParameterList->AddOrderDependent(new BcmIpAddressCommandParameter(kIpAddressParameter));
    command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kFilenameParameter, false, "Filename", 128));
    command.pfCommandHelp = 
        "Causes the Permanent or Dynamic settings data to be read from a server via "
        "TFTP.  The file must be generated correctly, with all of the headers, CRC, and "
        "other information that would exist on the NonVol device.  The file will be read "
        "and evaluated for correctness before it is stored in the device.  If the file "
        "is rejected then the settings will be read from the device to undo any changes.  "
        "Normally, this will operate on all settings groups; if you specify -g then it "
        "will operate only on the current group (e.g. if you are in the SNMP nonvol dir, "
        "then -g will cause only the SNMP settings group to be read).";
    command.pfExamples = 
        "tftp_read perm 11.24.4.3 allsettings.bin\tReads all groups.\n"
        "tftp_read -g perm 11.24.4.3 bpi.bin\tReads only the current group.";
    AddCommand(command);

    command.fCommandId = kTftpWriteSettings;
    command.pfCommandName = "tftp_write";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderIndependent(new BcmFlagCommandParameter('g', kGroupOnlyFlag));
    command.pfCommandParameterList->AddOrderDependent(new BcmStringSetCommandParameter("perm|dyn", kPrimaryParameter));
    command.pfCommandParameterList->AddOrderDependent(new BcmIpAddressCommandParameter(kIpAddressParameter));
    command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kFilenameParameter, false, "Filename", 128));
    command.pfCommandHelp = 
        "Causes the Permanent or Dynamic settings data to be written to a server via "
        "TFTP.  Normally, this will operate on all settings groups; if you specify "
        "-g then it will operate only on the current group (e.g. if you are in the "
        "SNMP nonvol dir, then -g will cause only the SNMP settings group to be "
        "written).";
    command.pfExamples = 
        "tftp_write perm 11.24.4.3 nvsettings.bin\tWrites all groups.\n"
        "tftp_write -g perm 11.24.4.3 bpi.bin\tWrites only the current group.";
    AddCommand(command);

    #endif

    command.fCommandId = kPrintStats;
    command.pfCommandName = "stats";
    command.pfCommandHelp = 
        "Prints stats about buffer usage for Dynamic and Permanent settings.";
    command.pfExamples = 
        "stats\t";
    AddCommand(command);

    command.fCommandId = kClearDevice;
    command.pfCommandName = "clear_device";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmStringSetCommandParameter("both|perm|dyn", kPrimaryParameter, true));
    command.pfCommandHelp = 
        "Tells the nonvol device to wipe out the specified settings section.  All data "
        "will be lost; the device should be restored to its 'virgin' state.  For flash "
        "devices, the flash block is erased; for disk devices, the file is erased.";
    command.pfExamples = 
        "clear_device\tClears both Permanent and Dynamic sections.\n"
        "clear_device perm\tClears only the Permanent section.";
    AddCommand(command);

    // PR925
    command.fCommandId = kFlushExtraData;
    command.pfCommandName = "del_extra_data";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmStringSetCommandParameter("both|perm|dyn", kPrimaryParameter, true));
    command.pfCommandHelp = 
        "Gets rid of any extra data that is assiociated with the settings.  The extra "
        "data is usually caused by a downgrade, where settings written by a newer version "
        "are not understood by the older version, but are stored so that they will still "
        "exist when the new code is run.  However, in some cases, it is desirable to get "
        "rid of them if they are not going to be used.";
    command.pfExamples = 
        "del_extra_data perm\tGets rid of extra permanent data.";
    AddCommand(command);

    // This command must be enabled manually by someone on the outside who knows
    // for a fact that the instance associated with this command table is a
    // composite object.
    command.fCommandId = kFlushUnrecognized;
    command.fIsEnabled = false;
    command.pfCommandName = "flush";
    command.pfCommandHelp = 
        "For Composite NonVol Settings objects (which contain 1 or more groups of NonVol "
        "Settings), this command flushes out any unrecognized groups that were read from "
        "the device.  This is useful in cases where the settings group isn't needed by "
        "another application, and would just waste space.";
    command.pfExamples = 
        "flush\t";
    AddCommand(command);

}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmNonVolSettingsCommandTable::~BcmNonVolSettingsCommandTable()
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
void BcmNonVolSettingsCommandTable::CommandHandler(void *pInstanceValue,
                                                   const BcmCommand &command)
{
    const BcmCommandParameter *pParameter;
    bool doPermanent = true;
    bool doDynamic = true;

    // These are the string set offsets, with meaningful names.
    enum
    {
        kBothGroups = 0,
        kPermGroup = 1,
        kDynGroup = 2
    };

    enum
    {
        kTftpPerm = 0,
        kTftpDyn = 1
    };

    // By convention, the settings object registers its instance so that the
    // user value is the 'this' pointer.  This cast should be safe.
    BcmNonVolSettings *pNonVolSettings = (BcmNonVolSettings *) pInstanceValue;

    // Validate this just for sanity.
    if (pNonVolSettings == NULL)
    {
        gLogMessageRaw 
            << "BcmNonVolSettingsCommandTable::CommandHandler:  ERROR - "
            << "User value (pointer to NonVol Settings instance) is NULL!  Can't process the command!\n";

        return;
    }

    // Many commands below have a single primary parameter.  Retrieve it for
    // them.
    if (command.pfCommandParameterList != NULL)
    {
        pParameter = command.pfCommandParameterList->Find(kPrimaryParameter);

        switch (command.fCommandId)
        {
            case kRestoreDefaults:
            case kWriteSettings:
            case kReadSettings:
            case kClearDevice:
            case kFlushExtraData:       // PR925
            case kIsManufactured:
                // For these commands, the primary parameter is the string set
                // that tells which section to operate on.  This is common code
                // to set the flags for them.
                if (pParameter->WasParsed())
                {
                    switch (pParameter->AsUnsignedInt())
                    {
                        case kPermGroup:
                            doDynamic = false;
                            break;

                        case kDynGroup:
                            doPermanent = false;
                            break;
                    }
                }
                break;

            case kTftpReadSettings:
            case kTftpWriteSettings:
                // Similar to the other one, except that either dyn or perm must
                // be specified; both is not an option, so the offset is not the
                // same.
                if (pParameter->AsUnsignedInt() == kTftpPerm)
                {
                    doDynamic = false;
                }
                else
                {
                    doPermanent = false;
                }
                break;
        }
    }

    switch (command.fCommandId)
    {
        case kRestoreDefaults:
            {
                BcmBoolCommandParameter answer(0, false, NULL, true);

                answer.PromptUser("Are you sure you want to restore the settings to default values?");

                if (answer.AsBool() && doPermanent)
                {
                    answer.SetAsBool(false);

                    answer.PromptUser("WARNING:  You will be defaulting Permanent settings, most of which do not have\n"
                                      "good default values!  Are you REALLY sure you want to do this?");
                }

                if (answer.AsBool())
                {
                    if (doDynamic)
                    {
                        pNonVolSettings->ResetDefaults(BcmNonVolSettings::kDynamicSection);

                        gLogMessageRaw << "Dynamic Non-volatile settings have been restored to their default values...you must write the settings to lock them in.\n";
                    }

                    if (doPermanent)
                    {
                        pNonVolSettings->ResetDefaults(BcmNonVolSettings::kPermanentSection);

                        gLogMessageRaw << "Permanent Non-volatile settings have been restored to their default values...you must write the settings to lock them in.\n";
                    }
                }
            }
            break;
            
        case kIsManufactured:
            {
                if (doPermanent)
                {
                    BcmString Hint;
                    
                    gLogMessageRaw << "Permanent section manufactured: " << pNonVolSettings->IsManufactured(BcmNonVolSettings::kPermanentSection, Hint) << endl;
                    
                    if (Hint.length())
                        gLogMessageRaw << "  Not set: " << Hint << endl;
                }
                
                if (doDynamic)
                {
                    BcmString Hint;
                    
                    gLogMessageRaw << "Dynamic section manufactured: " << pNonVolSettings->IsManufactured(BcmNonVolSettings::kDynamicSection, Hint) << endl;
                    
                    if (Hint.length())
                        gLogMessageRaw << "  Not set: " << Hint << endl;
                }
            }
            break;

        case kWriteSettings:
            {
                BcmNonVolDevice *pNonVolDevice;

                // If a non-vol device was given to me, then use it.
                pNonVolDevice = BcmNonVolDevice::GetSingletonInstance();

                if (pNonVolDevice != NULL)
                {
                    bool result;

                    if (doDynamic)
                    {
                        result = pNonVolDevice->Write(*pNonVolSettings, BcmNonVolSettings::kDynamicSection);

                        if (result == false)
                        {
                            gLogMessageRaw << "ERROR - failed to write Dynamic Non-Vol Settings to the device!\n";
                        }
                        else
                        {
                            gLogMessageRaw << "Dynamic Non-Vol Settings successfully written to the device.\n";
                        }
                    }
                
                    if (doPermanent)
                    {
                        result = pNonVolDevice->Write(*pNonVolSettings, BcmNonVolSettings::kPermanentSection);

                        if (result == false)
                        {
                            gLogMessageRaw << "ERROR - failed to write Permanent Non-Vol Settings to the device!\n";
                        }
                        else
                        {
                            gLogMessageRaw << "Permanent Non-Vol Settings successfully written to the device.\n";
                        }
                    }
                }
                else
                {
                    gLogMessageRaw << "ERROR - No Non-Vol Device is present; can't write settings to the device!\n";
                }
            }
		    break;

        case kReadSettings:
            {
                BcmNonVolDevice *pNonVolDevice;

                // If a non-vol device was given to me, then use it.
                pNonVolDevice = BcmNonVolDevice::GetSingletonInstance();
                if (pNonVolDevice != NULL)
                {
                    bool result;

                    if (doDynamic)
                    {
                        result = pNonVolDevice->Read(*pNonVolSettings, BcmNonVolSettings::kDynamicSection);

                        if (result == false)
                        {
                            gLogMessageRaw << "ERROR - failed to read Dynamic Non-Vol Settings from the device!\n";
                        }
                        else
                        {
                            gLogMessageRaw << "Dynamic Non-Vol Settings successfully read from the device.\n";
                        }
                    }

                    if (doPermanent)
                    {
                        result = pNonVolDevice->Read(*pNonVolSettings, BcmNonVolSettings::kPermanentSection);

                        if (result == false)
                        {
                            gLogMessageRaw << "ERROR - failed to read Permanent Non-Vol Settings from the device!\n";
                        }
                        else
                        {
                            gLogMessageRaw << "Permanent Non-Vol Settings successfully read from the device.\n";
                        }
                    }
                }
                else
                {
                    gLogMessageRaw << "ERROR - No Non-Vol Device is present; can't read settings from the device!\n";
                }
            }
            break;

        case kShowSettings:
            // Dump the settings via the message logger.
            gLogMessageRaw << *pNonVolSettings;
            break;

        case kFlushUnrecognized:
            {
                BcmBoolCommandParameter answer(0, false, NULL, true);

                answer.PromptUser("This will permanently remove unrecognized settings from memory!\nAre you sure?");

                if (answer.AsBool())
                {
                    if (pNonVolSettings->FlushUnrecognized())
                    {
                        gLogMessageRaw << "\nAll unrecognized settings groups have been flushed!\nYou will need to write the settings in order to lock this in.\n";
                    }
                    else
                    {
                        gLogMessageRaw << "\nThere were no unrecognized settings groups to flush.\n";
                    }
                }
            }
            break;

        #if (BCM_TFTP_SUPPORT || BFC_INCLUDE_TFTP_SUPPORT)

        case kTftpReadSettings:
            {
                const BcmCommandParameter *pIpAddressParm;
                const BcmCommandParameter *pFilenameParm;

                pIpAddressParm = command.pfCommandParameterList->Find(kIpAddressParameter);
                pFilenameParm = command.pfCommandParameterList->Find(kFilenameParameter);

                // Get the physical device that will be used to store the file.
                BcmNonVolDevice *pNonVolDevice;

                pNonVolDevice = BcmNonVolDevice::GetSingletonInstance();

                if (pNonVolDevice == NULL)
                {
                    gLogMessageRaw << "ERROR - No Non-Vol Device is present; can't write settings to the device!\n";

                    break;
                }

                // Create a TFTP helper object.
                BcmTftp *pTftpHelper = BcmIpHelperFactory::NewTftp();

                if (pTftpHelper == NULL)
                {
                    gLogMessageRaw << "ERROR - TFTP helper isn't available on this platform!\n";

                    break;
                }

                // Try to open the file.
                if (!pTftpHelper->Open(pIpAddressParm->AsIpAddress(),
                                       pFilenameParm->AsString()))
                {
                    gLogMessageRaw 
                        << "ERROR - Failed to open file '" 
                        << pFilenameParm->AsString() << "' on server " 
                        << pIpAddressParm->AsIpAddress() << "!\n";
                }
                else
                {
                    // Create a buffer to store the file.
                    BcmOctetBuffer buffer(pNonVolDevice->MaxBufferSize());
                    BcmTftp::ReadResult result;

                    // Try to read the file.
                    result = pTftpHelper->Read(buffer);

                    // Close the TFTP helper.
                    pTftpHelper->Close();

                    switch (result)
                    {
                        case BcmTftp::kError:
                            gLogMessageRaw << "ERROR - Failed to read the buffer from the server!\n";
                            break;

                        case BcmTftp::kMoreData:
                            gLogMessageRaw << "ERROR - The file is too big to store in the NonVol device!\n";
                            break;

                        default:
                            // Get the device and settings to validate the buffer.
                            // If that succeeds, then write to the device;
                            // otherwise, read the settings from the device to
                            // undo the changes.
                            {
                                BcmNonVolSettings::NonVolSection section;
                                bool success;

                                if (doPermanent)
                                {
                                    section = BcmNonVolSettings::kPermanentSection;
                                }
                                else
                                {
                                    section = BcmNonVolSettings::kDynamicSection;
                                }

                                // If we were told to do only the current
                                // settings group, then just have it read its
                                // data from the buffer.  Otherwise, tell the
                                // device to do a full read for all groups as
                                // though it had read the buffer from the
                                // physical device itself.
                                if (command.pfCommandParameterList->Find(kGroupOnlyFlag)->WasParsed())
                                {
                                    unsigned int currentOffset = 0;

                                    success = pNonVolSettings->ReadFrom(buffer, currentOffset, section);
                                }
                                else
                                {
                                    success = pNonVolDevice->Read(*pNonVolSettings, section, &buffer);
                                }

                                if (success)
                                {
                                    gLogMessageRaw << "The settings are valid; writing to the device.\n";

                                    pNonVolDevice->Write(*pNonVolSettings, section);
                                }
                                else
                                {
                                    gLogMessageRaw << "\nERROR - The setting were not valid.  Restoring previous values.\n";

                                    if (pNonVolDevice->Read(*pNonVolSettings, section))
                                    {
                                        gLogMessageRaw << "\nThe settings were successfully read from the device.\n";
                                    }
                                    else
                                    {
                                        gLogMessageRaw << "ERROR - Failed to read the settings from the device.\n";
                                    }
                                }
                            }
                            break;
                    }
                }

                // Get rid of the TFTP helper object.
                delete pTftpHelper;
            }
            break;

        case kTftpWriteSettings:
            {
                const BcmCommandParameter *pIpAddressParm;
                const BcmCommandParameter *pFilenameParm;

                pIpAddressParm = command.pfCommandParameterList->Find(kIpAddressParameter);
                pFilenameParm = command.pfCommandParameterList->Find(kFilenameParameter);

                // Get the physical device that will be used to store the file.
                BcmNonVolDevice *pNonVolDevice;

                pNonVolDevice = BcmNonVolDevice::GetSingletonInstance();

                if (pNonVolDevice == NULL)
                {
                    gLogMessageRaw << "ERROR - No Non-Vol Device is present; can't read settings from the device!\n";

                    break;
                }

                // Create a TFTP helper object.
                BcmTftp *pTftpHelper = BcmIpHelperFactory::NewTftp();

                if (pTftpHelper == NULL)
                {
                    gLogMessageRaw << "ERROR - TFTP helper isn't available on this platform!\n";

                    break;
                }

                // Try to open the file.
                if (!pTftpHelper->Open(pIpAddressParm->AsIpAddress(),
                                       pFilenameParm->AsString(),
                                       BcmTftp::kWrite))
                {
                    gLogMessageRaw 
                        << "ERROR - Failed to open file '" 
                        << pFilenameParm->AsString() << "' on server " 
                        << pIpAddressParm->AsIpAddress() << "!\n";
                }
                else
                {
                    // Create a buffer to store the data.
                    BcmOctetBuffer buffer(pNonVolDevice->MaxBufferSize());
                    BcmTftp::WriteResult result;
                    bool success;

                    BcmNonVolSettings::NonVolSection section;

                    if (doPermanent)
                    {
                        section = BcmNonVolSettings::kPermanentSection;
                    }
                    else
                    {
                        section = BcmNonVolSettings::kDynamicSection;
                    }

                    // Get the nonvol data into the buffer.  We need to do
                    // different things depending on which flags were specified.
                    if (command.pfCommandParameterList->Find(kGroupOnlyFlag)->WasParsed())
                    {
                        // User specified the -g flag, so only do the current
                        // settings object.
                        success = pNonVolSettings->WriteTo(buffer, section);
                    }
                    else
                    {
                        // No flags specified, need to do all groups.
                        success = pNonVolDevice->Write(*pNonVolSettings,
                                                       section,
                                                       BcmNonVolDevice::kWriteModeSync,
                                                       &buffer);
                    }

                    if (!success)
                    {
                        gLogMessageRaw << "ERROR - Failed to store the settings to a buffer!\n";

                        break;
                    }

                    // Try to write the data.
                    result = pTftpHelper->Write(buffer);

                    // Close the TFTP helper.
                    pTftpHelper->Close();

                    switch (result)
                    {
                        case BcmTftp::kError:
                            gLogMessageRaw << "ERROR - Failed to write the buffer to the server!\n";
                            break;

                        default:
                            gLogMessageRaw << "\nThe settings were successfully written to the TFTP server.\n";
                            break;
                    }
                }

                // Get rid of the TFTP helper object.
                delete pTftpHelper;
            }
            break;

        #endif

        case kPrintStats:
            {
                // Get the physical device that will be used to store the file.
                BcmNonVolDevice *pNonVolDevice;

                pNonVolDevice = BcmNonVolDevice::GetSingletonInstance();

                if (pNonVolDevice != NULL)
                {
                    gLogMessageRaw 
                        << "Permanent settings using " 
                        << pNonVolDevice->PermanentBytesUsed() << " of " 
                        << pNonVolDevice->GetMaxSize(BcmNonVolSettings::kPermanentSection) 
                        << " bytes.\n";

                    gLogMessageRaw 
                        << "Dynamic settings using " 
                        << pNonVolDevice->DynamicBytesUsed() << " of " 
                        << pNonVolDevice->GetMaxSize(BcmNonVolSettings::kDynamicSection) 
                        << " bytes.\n\n";

                    gLogMessageRaw
                        << "Permanent NonVol checksum is "
                        << (void *) (pNonVolDevice->PermanentChecksum())
                        << '\n';

                    gLogMessageRaw
                        << "Dynamic NonVol checksum is   "
                        << (void *) (pNonVolDevice->DynamicChecksum())
                        << '\n';
                }
                else
                {
                    gLogMessageRaw << "No NonVol device; can't show stats.\n";
                }
            }
            break;

        case kClearDevice:
            {
                // Get the physical device that will be used to store the file.
                BcmNonVolDevice *pNonVolDevice;

                pNonVolDevice = BcmNonVolDevice::GetSingletonInstance();

                if (pNonVolDevice != NULL)
                {
                    BcmBoolCommandParameter answer(0, false, NULL, true);

                    answer.PromptUser("This will permanently clear/wipe out settings all from the device!\nAre you sure?");

                    if (answer.AsBool())
                    {
                        if (doDynamic)
                        {
                            gLogMessageRaw << "Clearing the Dynamic settings section..." << endl;

                            pNonVolDevice->ClearDevice(BcmNonVolSettings::kDynamicSection);
                        }

                        if (doPermanent)
                        {
                            gLogMessageRaw << "Clearing the Permanent settings section..." << endl;

                            pNonVolDevice->ClearDevice(BcmNonVolSettings::kPermanentSection);
                        }
                    }
                    else
                    {
                        gLogMessageRaw << "\nWhew...that was a close call...\n";
                    }
                }
                else
                {
                    gLogMessageRaw << "No NonVol device; can't clear.\n";
                }
            }
            break;

        case kFlushExtraData:   // PR925
            {
                BcmBoolCommandParameter answer(0, false, NULL, true);

                answer.PromptUser("WARNING:  This will permanently delete extra data from "
                                  "the settings!\nAre you sure you want to do this?");

                if (answer.AsBool())
                {
                    if (doDynamic)
                    {
                        pNonVolSettings->FlushExtraData(BcmNonVolSettings::kDynamicSection);

                        gLogMessageRaw << "Extra settings data in the Dynamic section has been deleted...you must write the settings to lock this in.\n";
                    }

                    if (doPermanent)
                    {
                        pNonVolSettings->FlushExtraData(BcmNonVolSettings::kPermanentSection);

                        gLogMessageRaw << "Extra settings data in the Permanent section has been deleted...you must write the settings to lock this in.\n";
                    }
                }
            }
            break;

        default:
            gLogMessageRaw << "WARNING - unknown command id (" << command.fCommandId << ")!  Ignoring...\n";
            break;
    }
}



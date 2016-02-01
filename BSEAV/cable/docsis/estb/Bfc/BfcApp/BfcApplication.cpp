//****************************************************************************
//
//  Copyright (c) 2003  Broadcom Corporation
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
//  $Id: BfcApplication.cpp 1.10 2006/01/17 22:15:43Z twhite Release $
//
//  Filename:       BfcApplication.cpp
//  Author:         David Pullen
//  Creation Date:  Sept 14, 2003
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "BfcApplication.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Local Functions *************************************

//********************** Class Method Implementations ************************


/// Default Constructor.  Initializes the state of the object...
///
/// Note that the constructor is called fairly early on in system startup.
/// This places some limits on what can be done here, especially if there
/// is any dependency on other system components.  We recommend that the
/// work done here be limited to setting variables to default values, setting
/// pointers to NULL, etc, and leaving the real work to the initialization
/// API.
///
/// \param
///      pInstanceName - name of this object (for debugging purposes).
///
BcmBfcApplication::BcmBfcApplication(const char *pInstanceName) :
    fMessageLogSettings("BcmBfcApplication", pInstanceName),
    pfBfcSystem(NULL),
    pfPacketSwitch(NULL)
{
    CallTrace("BcmBfcApplication", "Constructor");

    fMessageLogSettings.Register();
}


/// Destructor.  Frees up any memory/objects allocated, cleans up internal
/// state.
///
BcmBfcApplication::~BcmBfcApplication()
{
    CallTrace("BcmBfcApplication", "Destructor");

    // Just clear these pointers; I don't delete the objects.
    pfPacketSwitch = NULL;
    pfBfcSystem = NULL;

    fMessageLogSettings.Deregister();
}


/// Accessors to set/get the BFC System object associated with this object.
///
/// \param
///      pBfcSystem - the BFC System object to be associated with this
///                   object.
///
/// \return
///      A pointer to the BFC System object that is associated with this
///      object.
///
void BcmBfcApplication::RegisterBfcSystem(BcmBfcSystem *pBfcSystem)
{
    CallTrace("BcmBfcApplication", "RegisterBfcSystem");

    pfBfcSystem = pBfcSystem;
}


/// This method is called by the system startup code shortly after this
/// object is created.
///
/// Note that in the V2 code, this is where BSP-level kinds of
/// initialization would be done, such as configuring the vendor board HAL
/// (installing ISRs for h/w switches, etc).  In BFC, it's actually better to
/// put that kind of code in a BfcTarget class (derive your own class that
/// has the proprietary code in it), so that you will preserve portability
/// and can run the app in Win32.
///
/// This is called before nonvol has been read.
///
/// \retval
///      true if successful.
/// \retval
///      false if there was a problem.
/// 
bool BcmBfcApplication::Initialize(void)
{
    CallTrace("BcmBfcApplication", "Initialize");

    // Default implementation does nothing.
    return true;
}


/// This method gives the application a chance to create and register any
/// application-specific nonvol settings object(s) with the composite nonvol
/// object for the system.
///
/// Because the composite nonvol settings object will delete all settings
/// groups that are registered with it, you MUST create your settings objects
/// using new.
///
/// To register your nonvol settings object(s):
///
/// \code
///      #if (BFC_INCLUDE_NONVOL_SUPPORT)
///      {
///          BcmNonVolSettings *pSettings = new MyNonVolSettings;
///
///          pParentSettings->AddSettingsGroup(pSettings);
///      }
///      #endif
/// \endcode
///
/// This method will only be called if BFC_INCLUDE_NONVOL_SUPPORT is enabled
/// in the system configuration.
///
/// \param
///      pParentSettings - pointer to the composite settings object that
///                        will store and control your nonvol settings
///                        object(s).
///
/// \retval
///      true if the settings object(s) were created and registered.
/// \retval
///      false if there was a problem (usually because of a duplicate
///          magic number).
///
bool BcmBfcApplication::CreateAndRegisterNonVolSettings(BcmCompositeNonVolSettings *pParentSettings)
{
    CallTrace("BcmBfcApplication", "CreateAndRegisterNonVolSettings");

    // Default implementation does nothing.
    return true;
}


/// This method is called by the system startup code if permanent nonvol
/// was completely corrupted, and no usable settings were found.  Because
/// the operational parameters (MAC addresses, keys, etc) are lost, we
/// can't allow the system to attempt to operate significantly (especially
/// the WAN port, if there is one).
///
/// There really isn't much you can do here, except maybe turn on the
/// console if it was disabled.
///
void BcmBfcApplication::PermanentNonVolIsCorrupt(void)
{
    CallTrace("BcmBfcApplication", "PermanentNonVolIsCorrupt");

    // Default implementation does nothing.
}


/// This method is called by the system startup code if the board is 
/// determined not to be manufactured - that is, critical nonvol settings
/// (MAC addresses, keys, etc) remain at default values.  This gives the
/// apps a chance to take some kind of action if necessary to make the
/// board manufacturable.
///
/// This is different from the permanent NV being corrupt in that a fresh
/// board will only trigger the corrupt state the first time it is booted
/// up.  This state can exist across multiple reboots until NV is fully
/// provisioned.
///
void BcmBfcApplication::BoardManufacturingIncomplete(void)
{
    CallTrace("BcmBfcApplication", "BoardManufacturingIncomplete");

    // Default implementation does nothing.
}


/// This method gives the application component a chance to create any top
/// level command table(s) that are specific to the app, and to register
/// them with the specified parent command table.
///
/// You will be responsible for destroying the object(s) when your object is
/// destroyed.  If you create it as a static object (usually within this
/// method), then the compiler will generate code to destroy it for you.
///
/// To register your command table(s), use the AddSubtable() method of the
/// specified parent object:
///
/// \code
///      #if (BFC_INCLUDE_CONSOLE_SUPPORT)
///      {
///          static MyCommandTable myTable;
///
///          pParentTable->AddSubtable(&myTable);
///
///          #if (!BFC_CONFIG_MINIMAL_CONSOLE)
///          {
///              static MyOptionalCommandTable optionalTable;
///
///              pParentTable->AddSubtable(&optionalTable);
///          }
///          #endif
///      }
///      #endif
/// \endcode
///
/// You can also add individual commands to the parent command table, if you
/// don't want to implement an entire command table.
///
/// UNFINISHED - need an example.
///
/// This method will only be called if BFC_INCLUDE_CONSOLE_SUPPORT is
/// enabled in the system configuration.
///
/// Note that if BFC_CONFIG_MINIMAL_CONSOLE is enabled, you should try to
/// minimize the number of command tables/commands to the bare necessities.
/// The intent of that system config option is to minimize code size
/// contributed by the console.  In reality, you can do whatever you want,
/// but don't complain to us if the image is too big to fit...
///
/// \param
///      pParentTable - pointer to the parent command table that you should
///                     register your command table(s) with.  Your table(s)
///                     will appear as subdirectories under this table.
///
void BcmBfcApplication::CreateAndRegisterCommandTables(BcmCommandTable *pParentTable)
{
    CallTrace("BcmBfcApplication", "CreateAndRegisterCommandTables");

    // Default implementation does nothing.
}

/// This method gives the application component a chance to create any top
/// level command table(s) that are specific to the app, and to register
/// them with the specified parent command table.
///
/// You will be responsible for destroying the object(s) when your object is
/// destroyed.  If you create it as a static object (usually within this
/// method), then the compiler will generate code to destroy it for you.
///
/// To register your command table(s), use the AddSubtable() method of the
/// specified parent object:
///
/// \code
///      #if (BFC_INCLUDE_CONSOLE_SUPPORT)
///      {
///          static MyCommandTable myTable;
///
///          pParentTable->AddSubtable(&myTable);
///
///          #if (!BFC_CONFIG_MINIMAL_CONSOLE)
///          {
///              static MyOptionalCommandTable optionalTable;
///
///              pParentTable->AddSubtable(&optionalTable);
///          }
///          #endif
///      }
///      #endif
/// \endcode
///
/// You can also add individual commands to the parent command table, if you
/// don't want to implement an entire command table.
///
/// UNFINISHED - need an example.
///
/// This method will only be called if BFC_INCLUDE_CONSOLE_SUPPORT is
/// enabled in the system configuration.
///
/// Note that if BFC_CONFIG_MINIMAL_CONSOLE is enabled, you should try to
/// minimize the number of command tables/commands to the bare necessities.
/// The intent of that system config option is to minimize code size
/// contributed by the console.  In reality, you can do whatever you want,
/// but don't complain to us if the image is too big to fit...
///
/// \param
///      pParentTable - pointer to the parent command table that you should
///                     register your command table(s) with.  Your table(s)
///                     will appear as subdirectories under this table.
///
void BcmBfcApplication::CreateAndRegisterRemoteAccessCommandTables(BcmCommandTable *pParentTable)
{
    CallTrace("BcmBfcApplication", "CreateAndRegisterRemoteAccessCommandTables");

    // Default implementation does nothing.
}



/// This method gives the application a chance to create and register command
/// table(s) corresponding to the nonvol settings object(s) that were created
/// in CreateAndRegisterNonVolSettings().
///
/// You will be responsible for destroying the object(s) when your object is
/// destroyed.  If you create it as a static object (usually within this
/// method), then the compiler will generate code to destroy it for you.
///
/// To register your command table(s), use the AddSubtable() method of the
/// specified parent object:
///
/// \code
///      #if (BFC_INCLUDE_CONSOLE_SUPPORT) && (!BFC_CONFIG_MINIMAL_CONSOLE) && (BFC_INCLUDE_NONVOL_SUPPORT) && (BFC_INCLUDE_NONVOL_CONSOLE_SUPPORT)
///      {
///          static MyNonVolCommandTable myNonVolTable;
///
///          pParentTable->AddSubtable(&myNonVolTable);
///      }
///      #endif
/// \endcode
///
/// This method will only be called if BFC_INCLUDE_CONSOLE_SUPPORT is
/// enabled, BFC_INCLUDE_NONVOL_SUPPORT is enabled,
/// BFC_INCLUDE_NONVOL_CONSOLE_SUPPORT is enabled, and if
/// BFC_CONFIG_MINIMAL_CONSOLE is NOT enabled.  It sounds like an improbable
/// sequence of configurations that have to line up, but this is actually
/// the way most systems are normally configured.
///
/// \param
///      pParentTable - pointer to the parent command table that you should
///                     register your command table(s) with.  Your table(s)
///                     will appear as subdirectories under this table.  Note
///                     that this object will almost always be different from
///                     the parameter passed in to
///                     CreateAndRegisterCommandTables().
///
void BcmBfcApplication::CreateAndRegisterNonVolCommandTables(BcmCommandTable *pParentTable)
{
    CallTrace("BcmBfcApplication", "CreateAndRegisterNonVolCommandTables");

    // Default implementation does nothing.
}


/// This method is called by the system startup code after the nonvol objects
/// have been created, nonvol is read and validated (possibly resetting to
/// defaults if there is a problem).  Also, most of the command tables have
/// been created and registered with the console at this point (interface-
/// specific command tables are created after this, when the drivers are
/// loaded).
///
/// This is a good place to check your nonvol settings and do basic setup
/// with what the settings specify.
///
/// \retval
///      true if successful.
/// \retval
///      false if there was a problem.
///
bool BcmBfcApplication::PreDriverInitialization(void)
{
    CallTrace("BcmBfcApplication", "PreDriverInitialization");

    // Default implementation does nothing.
    return true;
}


/// Accessors to set/get the PacketSwitch object that this object is being
/// associated with.  This is called by the BFC system object after network
/// interfaces have been initialized.
///
/// This will only be called if BFC_INCLUDE_NETWORKING_SUPPORT is enabled.
///
/// \param
///      pPacketSwitch - the PacketSwitch object that this object is to use.
///
/// \return
///      The PacketSwitch object that this object is using.
///
void BcmBfcApplication::RegisterPacketSwitch(BcmPacketSwitch *pPacketSwitch)
{
    CallTrace("BcmBfcApplication", "RegisterPacketSwitch");

    pfPacketSwitch = pPacketSwitch;
}


/// This method is called by the system startup code after the various
/// drivers/interfaces (if any) have been initialized.
///
/// This is a good place to do interface-specific initialization (register
/// Snoops, etc).  The pfPacketSwitch member should be configured at this
/// point.
///
/// This will only be called if BFC_INCLUDE_NETWORKING_SUPPORT is enabled.
///
/// \retval
///      true if successful.
/// \retval
///      false if there was a problem.
///
bool BcmBfcApplication::PostDriverInitialization(void)
{
    CallTrace("BcmBfcApplication", "PostDriverInitialization");

    // Default implementation does nothing.
    return true;
}


/// This method is called by the system startup code after the admin
/// status of the interfaces have been set (usually up).
///
/// Any initialization that requires the forwarder and interfaces to be
/// ready to transmit (e.g., sending ARPs) should be done here.
///
/// This will only be called if BFC_INCLUDE_NETWORKING_SUPPORT is enabled.
///
/// \retval
///      true if successful.
/// \retval
///      false if there was a problem.
///
bool BcmBfcApplication::PostInterfacesReadyInitialization(void)
{
    CallTrace("BcmBfcApplication", "PostInterfacesReadyInitialization");

    // Default implementation does nothing.
    return true;
}

/// Gives the application object a chance to add it's own version information
/// to the version banner object.  It is useful to do this late so that the
/// information can be configured based on run-time information.
///
/// \param
///      pVersionBanner - pointer to the version banner object to which the
///                       version info should be added.
///
void BcmBfcApplication::AddVersionInfo(BcmVersionBanner *pVersionBanner)
{
    CallTrace("BcmBfcApplication", "AddVersionInfo");

    // Default implementation does nothing.
}


/// Called by the system object after all the basic initialization and
/// configuration has completed.  This is a good place to make sure your
/// threads/objects are in an initial state (resetting them as needed), and
/// then kick them off (or get them to wait for input/events that they need).
///
/// You MUST return from this method in order for the system to continue
/// with other processing.
///
/// Note that this method can be called multiple times!  Run() is a public
/// method of this class, and it can be called by several sources, including
/// the console after the system has already started up and is running.
///
/// \retval
///      true if the application started successfully.
/// \retval
///      false if there was a problem.
///
bool BcmBfcApplication::Run(void)
{
    CallTrace("BcmBfcApplication", "Run");

    // Default implementation does nothing.
    return true;
}


/// Called by the system object when it is time to shut everything down in
/// preparation for exiting/rebooting the system.  This is a good place to
/// stop threads, deregister things, etc.  You could wait to do this in your
/// destructor, but by then other object may already be destroyed.  This
/// method is called before anything is destroyed, so you know your pointers
/// should still be valid.
///
void BcmBfcApplication::DeInitialize(void)
{
    CallTrace("BcmBfcApplication", "DeInitialize");

    // Default implementation does nothing.
}




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
//  $Id: BfcApplication.h 1.11 2006/01/17 22:15:43Z twhite Release $
//
//  Filename:       BfcApplication.h
//  Author:         David Pullen
//  Creation Date:  August 15, 2003
//
//****************************************************************************

#ifndef BfcApplication_H
#define BfcApplication_H

//********************** Include Files ***************************************

#include "MessageLog.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class BcmBfcSystem;
class BcmCommandTable;
class BcmCompositeNonVolSettings;
class BcmPacketSwitch;
class BcmVersionBanner;

//********************** Class Declaration ***********************************


/** \ingroup BfcCore
*
*   This is the base class for all BFC-based application components.  Note
*   that there can be multiple of these in the system at any given time.
*
*   All of the virtual methods of this class have a default/dummy
*   implementation that doesn't do anything.  This allows derived classes
*   to minimize the amount of code that needs to be implemented, so that
*   you only have to write code for the things you care about.
*/
class BcmBfcApplication
{
public:

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
    BcmBfcApplication(const char *pInstanceName = "BFC Application");

    /// Destructor.  Frees up any memory/objects allocated, cleans up internal
    /// state.
    ///
    virtual ~BcmBfcApplication();

public:
  
    /// These methods are called at various points during system initialization.
    /// The order of the methods (from top to bottom) indicates the order in
    /// which they are called.  The comments for each method should indicate
    /// other conditions that may exist or that would keep the method from being
    /// called.


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
    virtual void RegisterBfcSystem(BcmBfcSystem *pBfcSystem);
    inline const BcmBfcSystem *BfcSystem(void) const;

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
    virtual bool Initialize(void);

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
    virtual bool CreateAndRegisterNonVolSettings(BcmCompositeNonVolSettings *pParentSettings);

    /// This method is called by the system startup code if permanent nonvol
    /// was completely corrupted, and no usable settings were found.  Because
    /// the operational parameters (MAC addresses, keys, etc) are lost, we
    /// can't allow the system to attempt to operate significantly (especially
    /// the WAN port, if there is one).
    ///
    /// There really isn't much you can do here, except maybe turn on the
    /// console if it was disabled.
    ///
    virtual void PermanentNonVolIsCorrupt(void);
    
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
    virtual void BoardManufacturingIncomplete(void);

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
    virtual void CreateAndRegisterCommandTables(BcmCommandTable *pParentTable);

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
    virtual void CreateAndRegisterRemoteAccessCommandTables(BcmCommandTable *pParentTable);

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
    virtual void CreateAndRegisterNonVolCommandTables(BcmCommandTable *pParentTable);

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
    virtual bool PreDriverInitialization(void);

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
    virtual void RegisterPacketSwitch(BcmPacketSwitch *pPacketSwitch);
    inline const BcmPacketSwitch *PacketSwitch(void) const;

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
    virtual bool PostDriverInitialization(void);

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
    virtual bool PostInterfacesReadyInitialization(void);
    
    /// Gives the application object a chance to add it's own version information
    /// to the version banner object.  It is useful to do this late so that the
    /// information can be configured based on run-time information.
    ///
    /// \param
    ///      pVersionBanner - pointer to the version banner object to which the
    ///                       version info should be added.
    ///
    virtual void AddVersionInfo(BcmVersionBanner *pVersionBanner);

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
    virtual bool Run(void);

    /// Called by the system object when it is time to shut everything down in
    /// preparation for exiting/rebooting the system.  This is a good place to
    /// stop threads, deregister things, etc.  You could wait to do this in your
    /// destructor, but by then other object may already be destroyed.  This
    /// method is called before anything is destroyed, so you know your pointers
    /// should still be valid.
    ///
    virtual void DeInitialize(void);

protected:

    /// Determines how much message logging I will do.
    BcmMessageLogSettings fMessageLogSettings;

    /// Pointer to the BFC System object (lets me access and configure certain
    /// aspects of the system).
    BcmBfcSystem *pfBfcSystem;

    /// Pointer to the PacketSwitch object with which we are associated (there
    /// may be more than one in the system, or there might not be any at all).
    BcmPacketSwitch *pfPacketSwitch;

private:

    /// Copy Constructor.  Not supported.
    BcmBfcApplication(const BcmBfcApplication &otherInstance);

    /// Assignment operator.  Not supported.
    BcmBfcApplication & operator = (const BcmBfcApplication &otherInstance);

};


//********************** Inline Method Implementations ***********************


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
inline const BcmBfcSystem *BcmBfcApplication::BfcSystem(void) const
{
    return pfBfcSystem;
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
inline const BcmPacketSwitch *BcmBfcApplication::PacketSwitch(void) const
{
    return pfPacketSwitch;
}


#endif



//****************************************************************************
//
// Copyright (c) 2009 Broadcom Corporation
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
//  Filename:       EstbSnmpAgentApplication.h
//  Author:         Pinar Taskiran Cyr
//  Creation Date:  June 25, 2009
//
//****************************************************************************

#ifndef EstbSnmpAgentApplication_H
#define EstbSnmpAgentApplication_H

//********************** Include Files ***************************************

// My base class.
#include "BfcApplication.h"

//********************** Global Types ****************************************
// Component banner/version info for STB application.
//
#define kVersion "N/A"

// Used to determine whether or not we have been released.
#define kReleaseState "$State: Release $"

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************


/** \ingroup BcmEstbSnmpAgentApplication
*
*   This derived class handles Estb Snmp Agent system initialization and
*   configuration.
*/
class BcmEstbSnmpAgentApplication : public BcmBfcApplication
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
    BcmEstbSnmpAgentApplication(void);

    /// Destructor.  Frees up any memory/objects allocated, cleans up internal
    /// state.
    ///
    virtual ~BcmEstbSnmpAgentApplication();

public:  // Accessor methods that were previously implemented in the
         // BcmCmApplication object for V2.  For BFC, these are implemented in
         // this object, and there is a class that maps the old object to
         // this one so that client code doesn't need to change.

    /// Tells the CM App whether to run the console or to leave it suspended.
    ///
    /// \param
    ///      runConsole - true to run the console; false to leave it suspended.
    ///
    void RunConsole(bool runConsole);

    /// Returns the run-console state.
    ///
    /// \retval
    ///     true if the console will be run.
    /// \retval
    ///     false if the console will be left suspended.
    ///
    bool RunConsole(void) const;

public:
  
    // These methods are called at various points during system initialization.
    // The order of the methods (from top to bottom) indicates the order in
    // which they are called.  The comments for each method should indicate
    // other conditions that may exist or that would keep the method from being
    // called.


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

    // Instantiate, register, populate and start the SNMP agent(s).
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    void StartSnmp(void);

protected:


private:

    /// Copy Constructor.  Not supported.
    BcmEstbSnmpAgentApplication(const BcmEstbSnmpAgentApplication &otherInstance);

    /// Assignment operator.  Not supported.
    BcmEstbSnmpAgentApplication & operator = (const BcmEstbSnmpAgentApplication &otherInstance);

};


//********************** Inline Method Implementations ***********************


#endif



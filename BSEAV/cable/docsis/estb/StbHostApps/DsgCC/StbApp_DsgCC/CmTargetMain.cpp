//****************************************************************************
//
//  Copyright (c) 2002  Broadcom Corporation
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
//  $Id$
//
//  Filename:       CmTargetMain.cpp
//  Author:         David Pullen
//  Creation Date:  Nov 27, 2002
//
//****************************************************************************
//  Description:
//      This file contains the target startup code (i.e. code related to the
//      embedded system target, as opposed to a host-based simulator target).
//
//      A bunch of this code was previously in the vxWorks/pSOS/Linux startup
//      code, and has been moved here since it isn't OS-specific, and we don't
//      like lots of duplicate code lying around.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "CmTargetMain.h"

// Handles message logging stuff.
#include "MessageLog.h"

// PR1465 Various drivers.
//#include "FlashDeviceDriver.h"
//#include "ProgramStoreDeviceDriver.h"
//#include "BootloaderStoreDeviceDriver.h"
//#include "NonVolDeviceDriver.h"

#include "bcmAlloc.h"

#if (TARGETOS_Linux)
// This variable must be defined!  If changing this variable, include the leading forward
// slash (for eg:  /home/sw/).  The non-vol file name just gets appended to this path.
#define PATH_TO_NONVOL "./"
#endif

//#include "FlashDriverCommandTable.h"

#ifdef BCM_SETTOP_SUPPORT
  //#include "StbProgramStoreDeviceDriverBridge.h"
  #if (TARGETOS_Linux)
    #include "DiskFileNonVolDevice.h"
  #else
    #include "NonVolDeviceDriverBridge.h"
  #endif
#else
  // Singleton objects that wrap the drivers.
  #include "NonVolDeviceDriverBridge.h"
  //#include "ProgramStoreDeviceDriverBridge.h"
  //#include "BootloaderStoreDeviceDriverBridge.h"
#endif

#include "MergedNonVolDefaults.h"

#if (BCM_REDUCED_IMAGE_SIZE != 1)
// Comment this out to disable the heap monitor; comment it back in to enable
// the heap monitor.  Note that the heap monitor is not available in the slim
// load.
//    #define RUN_HEAP_MONITOR 1
#endif

#if defined (RUN_HEAP_MONITOR)
    #include "HeapMonitorThread.h"
#endif

//#include "HalIfNonVolSettings.h"

#include "CommandParameterList.h"
#include "BitmaskCommandParameter.h"
#include "UnsignedIntCommandParameter.h"

#if (BOOT_LOADER_INCLUDE)
    #include "bootloaderstore.h"
#endif

#if (TARGETOS_Linux)
	#include "DiskFileNonVolDevice.h"
	#include "Command.h"
#endif

//********************** Local Types *****************************************

//********************** Local Constants *************************************

enum
{
    kHalDebugZone = 1,
    kVpmPowerdownChannel,
};

enum
{
    kPrimaryParameter = 1
};


#if defined(TARGETOS_Linux)

// For Linux, the thread priority is 99 for some reason.
#define kWatchdogThreadPriority 99

#else

// For pSOS and VxWorks, the thread priority is 250.
#define kWatchdogThreadPriority 250

#endif

//********************** Local Variables *************************************

static unsigned int gCommandIdBase = 0;

//********************** Local Functions *************************************

static void CommandHandler(void *pInstanceValue, const BcmCommand &command);

//********************** Global Variables ************************************

extern "C"
{
    // This is the HAL's debug control variable.
    extern uint16 DebugZone; 

    // Global mac address use by the ultra-lame HPNA driver, and quite possibly
    // others.  This is legacy stuff and shouldn't be used any more, but we
    // need to keep it around for now.
    extern unsigned char EthernetAddress[6];
}

//********************** Global Functions ************************************

extern "C"
{
    #ifdef BCM_SETTOP_SUPPORT

    #if (TARGETOS_Linux)
	void async_setup(void);
    #else
    void BcmHardwareInit(void);
    #endif

    #endif
}

//********************** Function Implementations ****************************


// This function does the target-specific initialization for CM hardware
// (loading low-level drivers for flash, nonvol, etc), and also creates
// singletons like the target-specific BcmNonVolDevice, etc.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void CmTargetLowLevelInit(void)
{

    gLogMessageRaw << "CmTargetLowLevelInit:  Loading Flash/ProgramStore/NonVol drivers..." << endl;

    #ifdef BCM_SETTOP_SUPPORT
    #if (TARGETOS_Linux)
	//async_setup();
    #else
    // Brcm HW init moved to here for a more generic vxWorks bsp 
    BcmHardwareInit();
    #endif
    #endif

    #if (!(TARGETOS_Linux))
    // Configure the flash driver to use the desired amount of memory for
    // permanent and dynamic nonvol.
    if ( kDefaultValue_PermanentNonvolSizeBytes > 0 )
    {
        FlashDriverConfigureRegionSize(kPermanentNonVolRegion,
                                       kDefaultValue_PermanentNonvolSizeBytes);
    }
    if ( kDefaultValue_DynamicNonvolSizeBytes > 0 )
    {
        FlashDriverConfigureRegionSize(kDynamicNonVolRegion,
                                       kDefaultValue_DynamicNonvolSizeBytes);
    }

    // PR1465 Initialize the flash driver.
    //FlashDriverInit();

    // PR1465 Initialize the program store driver, which uses the flash driver.
    //ProgramStoreDriverInit();

    // PR1465 Initialize the bootloader store driver, which also uses the flash driver.
    //BootloaderStoreDriverInit();

    // PR1465 Initialize the NonVol driver, which also uses the flash driver.
    NonVolDriverInit();
    #endif  // TARGETOS_Linux


    gLogMessageRaw << "CmTargetLowLevelInit:  Creating singletons for ProgramStore/BootloaderStore/NonVol devices...\n" << endl;

/*
    #ifdef BCM_SETTOP_SUPPORT
    
    // Create the Program Store device singleton for this target.  This will be
    // used to update the software image.
    BcmStbProgramStoreDeviceDriverBridge *pProgramStoreDevice = new BcmStbProgramStoreDeviceDriverBridge;
    
    #warning "Settop build: BootloaderStoreDevice not supported!"

    #else
    
    // Create the Program Store device singleton for this target.  This will be
    // used to update the software image.
    BcmProgramStoreDeviceDriverBridge *pProgramStoreDevice = new BcmProgramStoreDeviceDriverBridge;

    // Also create the Bootloader Store device singleton for this target.  This
    // will be used to update the bootloader image.
    BcmBootloaderStoreDeviceDriverBridge *pBootloaderStoreDevice = new BcmBootloaderStoreDeviceDriverBridge;

    #endif
*/    
    // Create the non-vol device for the target.  This also sets up the
    // singleton.
	#if (TARGETOS_Linux)
       // Create the full path to the dynamic non-vol file
        char *dynFileNameWithPath = new char [strlen(PATH_TO_NONVOL) + 17];
        strcpy(dynFileNameWithPath, (char *)PATH_TO_NONVOL);
        strcat(dynFileNameWithPath, "dsgccdynamic.bin");

        // Create the full path to the permanent non-vol file
        char *permFileNameWithPath = new char [strlen(PATH_TO_NONVOL) + 19];
        strcpy(permFileNameWithPath, (char *)PATH_TO_NONVOL);
        strcat(permFileNameWithPath, "dsgccpermanent.bin");

        BcmDiskFileNonVolDevice *pNonVolDevice = new BcmDiskFileNonVolDevice(dynFileNameWithPath, permFileNameWithPath);
        delete []dynFileNameWithPath;
        delete []permFileNameWithPath;
	#else
    BcmNonVolDeviceDriverBridge *pNonVolDevice = new BcmNonVolDeviceDriverBridge;
	#endif


    // Check to see if the bootloader needs to be updated.
    #if (BOOT_LOADER_INCLUDE)
    
    BcmUpdateBootLoader();

    #endif
    

    // Start the heap monitor thread if compiled in.
    #if defined (RUN_HEAP_MONITOR)
    
    BcmHeapMonitorThread *pHeapMonitorThread = new BcmHeapMonitorThread;

    #endif

}

#if 0

// This function does the target-specific startup.  This is called after nonvol
// settings have been read and validated.  This does stuff like configure DMA
// buffer allocations, start the watchdog, and other board/target initialization
// that requires nonvol settings.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void CmTargetPostNonvolInit(void)
{
    // Get the HalIf settings singleton for use below.  If this is NULL, then
    // we can't proceed.
    BcmHalIfNonVolSettings *pHalSettings = BcmHalIfNonVolSettings::GetSingletonInstance();

    if (pHalSettings == NULL)
    {
        gLogMessageRaw
            << "CmTargetPostNonvolInit:  ERROR - failed get the HalIf settings singleton!  Can't create HAL drivers!" << endl;

        while (1);
    }

    // Load the HAL Debug Zone variable with the new default.
    DebugZone = pHalSettings->HalDebugZones();

    // PR1930 - set the number of buffers that need to be allocated by BcmAlloc
    // for each buffer size.
    {
        // Use the new method/nonvol for configuring BcmAlloc.
        BcmAllocBufferConfig bufferConfig;

        bufferConfig.numberOfBufferTypes = pHalSettings->BcmAllocNumberOfBufferSizes();
        bufferConfig.bufferSizes = pHalSettings->BcmAllocBufferSizes();
        bufferConfig.numberOfBuffers = pHalSettings->BcmAllocBuffersNumbers();

        BcmAllocConfigure(bufferConfig);
    }


    BoardHalInitStruct boardHalInit;

    // UNFINISHED - this value works for VxWorks and pSOS, but we really should
    // use the OS abstractions to give us a good priority value.
    boardHalInit.watchdogThreadPriority = kWatchdogThreadPriority;
    boardHalInit.watchdogTimeoutSecs = pHalSettings->WatchdogTimeout();
    boardHalInit.watchdogEnabled = pHalSettings->WatchdogEnable();

    BoardHalInit(boardHalInit);

    // Bizzarro hack for the HPNA driver; rather than accepting the MAC address
    // as a parameter to their init function, they pull from a global variable.
    // Recently, we made lots of changes to how nonvol is initialized, and it
    // appears that the global variable is no longer initialized quite the same.
    // In order to satisfy the HPNA driver (and possibly others), we will
    // manually set it here.
    pHalSettings->MacAddressIp(1).Get(EthernetAddress[0],
                                      EthernetAddress[1],
                                      EthernetAddress[2],
                                      EthernetAddress[3],
                                      EthernetAddress[4],
                                      EthernetAddress[5]);
}


// This function installs the target-specific command table support with the
// specified parent command table.
//
// Parameters:
//      pParentCmdTable - the command table to be used as the parent.
//
// Returns:  Nothing.
//
void CmTargetCmdTableInit(BcmCommandTable *pParentCmdTable,
                          unsigned int commandIdBase)
{
    if (pParentCmdTable != NULL)
    {
        #if (BCM_CONSOLE_SUPPORT)

		#if (!(TARGETOS_Linux))
        // PR1465 Add the command table for the flash driver.
        BcmCommandTable *pCmdTable = new BcmFlashDriverCommandTable;

        pParentCmdTable->AddSubtable(pCmdTable);
		#endif

        // Store this for future use; it is the offset for all of my commands,
        // which allows me to avoid conflicts with other modules that are adding
        // commands to this command table.
        gCommandIdBase = commandIdBase;

        BcmCommand command;

        // Set up fields that don't change. 
        command.fCommandFunctionWithParms = CommandHandler;
        command.fIsEnabled = true;
        command.fIsHidden = false;

        command.fCommandId = kHalDebugZone + gCommandIdBase;
        command.pfCommandName = "zone";
        command.pfCommandParameterList = new BcmCommandParameterList;
        command.pfCommandParameterList->AddOrderDependent(
            new BcmBitmaskCommandParameter(kPrimaryParameter, true, NULL, 0xffff));
        command.pfCommandHelp =
            "Prints or sets the HAL debug zones; this determines what debug messages will be "
            "displayed by HAL drivers.  These bits correspond to the HAL debug zones:\n"
            "\t0x0001\tINIT\n"
            "\t0x0002\tTEST1\n"
            "\t0x0004\tTEST2\n"
            "\t0x0008\tTEST3\n"
            "\t0x0010\tTEST4\n"
            "\t0x0020\tTEST5\n"
            "\t0x0040\tTEST6\n"
            "\t0x0080\tBPI\n"
            "\t0x0100\tDOWNSTREAM\n"
            "\t0x0200\tUPSTREAM\n"
            "\t0x0400\tTUNER\n"
            "\t0x0800\tRANGING\n"
            "\t0x1000\tTESTSRAM\n"
            "\t0x2000\tTESTREG\n"
            "\t0x4000\tWARNING\n"
            "\t0x8000\tERROR";
        command.pfExamples = "zone 0xc000\tEnables ERROR and WARNING levels.";
        pParentCmdTable->AddCommand(command);

        #if (POWER_MGMT)
        
        // UNFINISHED - this command needs to be converted to user the new
        // command parameter objects.
        /*
        command.fCommandId = kVpmPowerdownChannel;
        command.pfCommandName = "vpm_powerdown_state";
        command.pfParameters = "[channel_mask] [ [operational_state] ]";
        command.pfCommandHelp = "The mask is four bits, with bits 3:0 representing. "
                                "channels 3:0, respectively.  For any bit, 1 specifies the "
                                "channel is ACTIVE, 0 specifies it should be shut down.\n\n"
                                "The VPM clock will be set according to the number of active channels as follows:\n\n"
                                "  3 or more active channels --> full speed\n"
                                "  2 active channels --> half speed\n"
                                "  1 active channel --> quarter speed\n"
                                "  0 channels --> OFF\n\n"
                                "The ELM will also be shut down automatically if all channels are OFF\n\n"
                                "operational_state sets the mode of ALL SLICs when operational as follows:\n\n"
                                "  0 = Lowerpower Standby, 1 = Forward Active, 2 = Unused, 3 = Reverse Active\n"
                                "  4 = Ringing, 5 = Forward Loopback, 6 = Tip Open, 7 = Power Denial\n\n"
                                "The default is Forward Active.  The high battery is always selected for "
                                "operational states, and the low battery is always selected for "
                                "powerdown states";
        command.pfExamples = "vpm_powerdown_state 0x0f\tEnables all 4 voice channels.\n"
                             "vpm_powerdown_state 0x02 3\tEnabled voice channel 1, sets operational state to Reverse Active.\n"
                             "vpm_powerdown_state\tShow status of VPM, ELMAC, and SLICs.";
        pCmApplication->MainCommandTable()->AddCommand(command);
        */

        #endif // POWER_MGMT
        
        #endif // BCM_CONSOLE_SUPPORT
    }
}


static void CommandHandler(void *pInstanceValue, const BcmCommand &command)
{
    const BcmCommandParameter *pParameter = NULL;
    
    if (command.pfCommandParameterList != NULL)
    {
        pParameter = command.pfCommandParameterList->Find(kPrimaryParameter);
    }

    switch (command.fCommandId - gCommandIdBase)
    {
        #if (BCM_CONSOLE_SUPPORT)
        
        case kHalDebugZone:
            {
                if (pParameter->WasParsed())
                {
                    DebugZone = (uint16) (pParameter->AsBitmask());
                }

                gLogMessageRaw << "HAL debug zone is " << (void *) DebugZone << endl;
            }
            break;

        #if (POWER_MGMT)
        
        // UNFINISHED - this command needs to be converted to user the new
        // command parameter objects.
        /*
        case kVpmPowerdownChannel:
        {
#define     DISPLAY_SLIC_STATE ((operState==0)?"Lowpower Standby":((operState==1)?"Forward Active": \
                               ((operState==2)?"Unused":((operState==3)?"Reverse Active":((operState==4)?"Ringing": \
                               ((operState==5)?"Forward Loopback":((operState==6)?"Tip Open":"Power Denial")))))))

            static uint8 configured = 0;
            static uint32 mask = 0;
            static SLIC_STATE operState = SS_FORWARD_ACTIVE;
            uint32 newMask, newOperState;
            unsigned int offset;
            BcmConsoleThread::ParseResult parseResult;
            uint8 active_channels=0, i;

			// get the mask
            parseResult = pConsoleThread->ParseParameter(pParameters, &offset, newMask,0x00,0x0f,false);

			// get the default operational state, if it exists
            if (parseResult == BcmConsoleThread::kSuccess)
            {
               parseResult = pConsoleThread->ParseParameter(pParameters + offset, &offset, newOperState,int(SS_LP_STANDBY),int(SS_POWER_DENIAL), false);

               if (parseResult == BcmConsoleThread::kSuccess)
			   {
                  operState = SLIC_STATE(newOperState);                  
                  gLogMessageRaw << "New operational state is " << DISPLAY_SLIC_STATE << "\n";
			   }
               else if(parseResult != BcmConsoleThread::kNoChange)
			   {
                  gLogMessageRaw << "INVALID operState\n";
                  break;
			   }
               else // no operState present
			   	;
            }
            else if(parseResult != BcmConsoleThread::kNoChange){
                  gLogMessageRaw << "INVALID mask\n";
                  break;
			}
			else // display status
			{
               uint32 TempReg;
			   // since there is no logic behind this, just read the appropraite registers
               TempReg = VPM->PwrMgmt & VPM_CLK_MASK;
               switch(TempReg)
			   {
                  case 0:
                  gLogMessageRaw << "VPM clock is OFF\n";
				  break;

                  case 1:
                  gLogMessageRaw << "VPM clock is at 1/4 speed\n";
				  break;

                  case 2:
                  gLogMessageRaw << "VPM clock is at 1/2 speed\n";
				  break;

                  case 3:
                  gLogMessageRaw << "VPM clock is at FULL speed\n";
				  break;
			   }
               if(APM->HostIfCtrl & ELMAC_ENABLE)
                  gLogMessageRaw << "ELMAC is ON\n";
               else
                  gLogMessageRaw << "ELMAC is OFF\n";

               if(configured)
			   {
	               if(mask&0x01)
		              gLogMessageRaw << "Channel0 AFEs and SLICs operational (" << DISPLAY_SLIC_STATE << ")\n";
	               else
		              gLogMessageRaw << "Channel0 AFEs and SLICs in powerdown mode\n";
				   if(mask&0x02)
		              gLogMessageRaw << "Channel1 AFEs and SLICs operational (" << DISPLAY_SLIC_STATE << ")\n";
				   else
		              gLogMessageRaw << "Channel1 AFEs and SLICs in powerdown mode\n";
				   if(mask&0x04)
		              gLogMessageRaw << "Channel2 AFEs and SLICs operational (" << DISPLAY_SLIC_STATE << ")\n";
				   else
		              gLogMessageRaw << "Channel2 AFEs and SLICs in powerdown mode\n";
				   if(mask&0x08)
		              gLogMessageRaw << "Channel3 AFEs and SLICs operational (" << DISPLAY_SLIC_STATE << ")\n";
				   else
		              gLogMessageRaw << "Channel3 AFEs and SLICs in powerdown mode\n";
               }
               else
                  gLogMessageRaw << "AFEs and SLICs have NOT been configured\n";
               break;
			}
            // set the new mask
            mask = newMask;
            // indicate mask is valid
			configured = 1;

            // determine active channels
            for(i=1; i<=8; i<<=1)
            {
			   if(mask&i)
			      active_channels++;
            }
	        // set VPM clock and elm state
            if(active_channels==0)
			{
               // VPM clock OFF - automatically turns OFF elm
               VpmSetClockRate(VPM_CLOCK_OFF);
               gLogMessageRaw << "VPM clock OFF\n";
               gLogMessageRaw << "ELMAC is OFF\n";
			}
            else if(active_channels==1)
            {
               // VPM clock quarter speed
               VpmSetClockRate(VPM_CLOCK_QS);
               gLogMessageRaw << "VPM clock at 1/4 speed\n";
               gLogMessageRaw << "ELMAC is ON\n";
			}
			else if(active_channels==2)
            {
               // VPM clock half speed
               VpmSetClockRate(VPM_CLOCK_HS);
               gLogMessageRaw << "VPM clock at 1/2 speed\n";
               gLogMessageRaw << "ELMAC is ON\n";
			}
			else
            {
               // VPM clock full speed
               VpmSetClockRate(VPM_CLOCK_FS);
               gLogMessageRaw << "VPM clock at full speed\n";
               gLogMessageRaw << "ELMAC is ON\n";
			}
            // set AFE/SLIC states
			if(mask&0x01)
			{
			   AfePowerdown(AFE_PDN_CH0,FALSE);
			   SlicSetState(SLIC_CHANNEL0, operState, BSEL_HIGH_BATTERY);
               gLogMessageRaw << "Channel0 AFEs and SLICs operational (" << DISPLAY_SLIC_STATE << ")\n";
			}
			else
			{
			   AfePowerdown(AFE_PDN_CH0,TRUE);
			   SlicSetState(SLIC_CHANNEL0, SS_POWER_DENIAL, BSEL_LOW_BATTERY);
               gLogMessageRaw << "Channel0 AFEs and SLICs in powerdown mode\n";
			}
			if(mask&0x02)
			{
			   AfePowerdown(AFE_PDN_CH1,FALSE);
			   SlicSetState(SLIC_CHANNEL1, operState, BSEL_HIGH_BATTERY);
               gLogMessageRaw << "Channel1 AFEs and SLICs operational (" << DISPLAY_SLIC_STATE << ")\n";
			}
			else
			{
			   AfePowerdown(AFE_PDN_CH1,TRUE);
			   SlicSetState(SLIC_CHANNEL1, SS_POWER_DENIAL, BSEL_LOW_BATTERY);
               gLogMessageRaw << "Channel1 AFEs and SLICs in powerdown mode\n";
			}
			if(mask&0x04)
			{
			   AfePowerdown(AFE_PDN_CH2,FALSE);
			   SlicSetState(SLIC_CHANNEL2, operState, BSEL_HIGH_BATTERY);
               gLogMessageRaw << "Channel2 AFEs and SLICs operational (" << DISPLAY_SLIC_STATE << ")\n";
			}
			else
			{
			   AfePowerdown(AFE_PDN_CH2,TRUE);
			   SlicSetState(SLIC_CHANNEL2, SS_POWER_DENIAL, BSEL_LOW_BATTERY);
               gLogMessageRaw << "Channel2 AFEs and SLICs in powerdown mode\n";
			}
			if(mask&0x08)
			{
			   AfePowerdown(AFE_PDN_CH3,FALSE);
			   SlicSetState(SLIC_CHANNEL3, operState, BSEL_HIGH_BATTERY);
               gLogMessageRaw << "Channel3 AFEs and SLICs operational (" << DISPLAY_SLIC_STATE << ")\n";
			}
			else
			{
			   AfePowerdown(AFE_PDN_CH3,TRUE);
			   SlicSetState(SLIC_CHANNEL3, SS_POWER_DENIAL, BSEL_LOW_BATTERY);
               gLogMessageRaw << "Channel3 AFEs and SLICs in powerdown mode\n";
			}
        }
        break;
        */

        #endif  // POWER_MGMT

        #endif  // BCM_CONSOLE_SUPPORT
           
        default:
            gLogMessageRaw 
                << "WARNING - unknown command id (" << command.fCommandId << ")!  Ignoring...\n";
            break;
    }
}


extern "C"
{
 
#if (!(TARGETOS_Linux))
// This is called by the ASSERT macro defined in vx_osl.h.
void V2Assert(const char *pAssertString)
{
    unsigned char *pVoid = NULL;
 
    if (pAssertString == NULL)
    {
        pAssertString = "Unspecified assertion; no string specified for file or line number!\n";
    }
 
    // Print the message.
    printf(pAssertString);
 
    // Wait for this to flush out.
    Sleep(25);
 
    // Dereference NULL so we crash.
    *pVoid = 0;
}
#endif
 
}




#endif


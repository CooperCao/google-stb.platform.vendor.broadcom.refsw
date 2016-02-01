//****************************************************************************
//
// Copyright (c) 2003-2008 Broadcom Corporation
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
//****************************************************************************//
//  Filename:       BfcNonVolDefaults.h
//  Author:         David Pullen
//  Creation Date:  Dec 23, 2003
//
//****************************************************************************
//  Description:
//      This file contains the default values that are to be used for nonvol
//      settings and as compile-time defaults.  This file controls the behavior
//      of the settings (hard-coded vs use nonvol) as well as specifying the
//      default values.
//
//****************************************************************************

#ifndef BfcNonVolDefaults_H
#define BfcNonVolDefaults_H

//********************** Include Files ***************************************

#if BFC_INCLUDE_NETWORKING_SUPPORT
#include "BspConstants.h"
#endif

//********************** Global Types ****************************************

//********************** Global Constants ************************************

// +--------------------+
// | NonVol memory size |
// +--------------------+
//
// If non-0, these values configure the number of bytes of flash memory will be
// reserved for Permanent and Dynamic nonvol memory.  The flash driver will
// set aside enough flash blocks to hold at least this many bytes.
//
// NOTE:  Changing the default value can cause the amount of flash memory
//        reserved for Image1 and Image2 to be reduced.  It may also cause
//        Permanent Nonvol to be placed in a different location, moving it from
//        the boot block (before Image1) to after Image1.
//
#define kDefaultValue_PermanentNonvolSizeBytes 0 //(64 * 1024)
#define kDefaultValue_DynamicNonvolSizeBytes   0

// +---------------------------------+
// | Message Logging NonVol Settings |
// +---------------------------------+
// 
// The customer can change the "CPE Access" IP stack address and subnet to suit
// the needs of their MSO.  Most of the time, you will want to use the
// 192.168.100.1 address and the default subnet mask.  If each of your MSOs
// have different values that must be used, then this needs to be a
// manufacturing item associated with a MIB.
//
// For the router address, if the OS is pSOS, then you will need to leave the
// address set to 0.0.0.0; otherwise it will need to be set to an on-net address.
// 
// None of the other settings can be modified.

// These are all IP addresses in string values.
#define kDefaultValue_RemoteAccessIpAddress     "192.168.100.1"
#define kDefaultValue_RemoteAccessSubnetMask    "255.255.255.0"
#define kDefaultValue_RemoteAccessRouterAddress "192.168.100.254"

#define kDefaultValue_RemoteAccessHttpUserId        ""
#define kDefaultValue_RemoteAccessHttpUserPassword  "Broadcom"
#define kDefaultValue_HttpAdminAccessEnabled        false
#define kDefaultValue_RemoteAccessHttpAdminId       ""
#define kDefaultValue_RemoteAccessHttpAdminPassword "aDm1n$TR8r"

// if form authorization is used, this is where the user
// is redirected once he logs in successfully
#define kDefaultValue_LoginAuthorizedHomePage   "RgConnect"

// if NO authorization is used, this is the home page
#define kDefaultValue_HomePage                  "RgConnect.asp"

// if NO authorization is used, this is the home page for 802.11 builds
#define kDefaultValue_80211HomePage             "wlanRadio.asp"

// if custom html files are used instead of BRCM template, define the prefix here
#define kDefaultValue_HtmlPrefix
// logo .gif file

#define kDefaultValue_HtmlLogoFile "VendorWebResource_logo_new_gif.cpp"
#define kDefaultValue_HtmlLogoName logo_new_gif

#define kDefaultValue_HtmlUrl "http://www.broadcom.com"

// HTML color definitions
// get common value definitions
#define kDefaultValue_HtmlPrimaryColor "#CE0000"
#define kDefaultValue_HtmlSecondaryColor "#CCCCCC"
// .asp generated by the code
#define kDefaultValue_HtmlTableStyleString "style=\"font-family: Helvetica;font-size:14\""
#define kDefaultValue_HtmlTableTitleColor "#9900FF"
#define kDefaultValue_HtmlTableOddRowColor "#9999CC"
#define kDefaultValue_HtmlTableEvenRowColor "#99CCFF"

// copyright
#define kDefaultValue_HtmlCopyright "2005 Broadcom Corporation"

// Unsigned integer value from 2-4.  Other values are reserved and must not
// be used.  Typically, you will want to set this to 2.  This controls the
// stack affinity in cases where multiple stacks have the same subnet, and
// tells the CPE Access agent which stack to initialize with the IP address
// and subnet specified above.  For the EMTA/Packet Cable build, this needs to
// be 3, since the EMTA uses stack 2.
#define kDefaultValue_RemoteAccessIpStackNumber 2

// PR4517 - new default controls and values for the telnet server.  The telnet
// server provides access to the console via telnet.  The telnet server is an
// optional feature that is compile in by default for fat images, and compiled
// out by default for slim images, though this can be overriden via build
// options "telnet" and "notelnet".  The following default values are used if
// the telnet server is compiled in to the image.
//
// This determines whether the telnet server will be enabled or not at startup.
// If not enabled, it can be started at runtime via the serial console or other
// hook (we will likely implement a MIB to do this).  This is a boolean value.
#define kDefaultValue_TelnetServerEnabled false

// These set the default values for the user name and password that must be
// entered when logging in via telnet.  Again, we may implement MIBs to control
// these; if an empty string is used, then you can just hit enter when prompted
// to log in.  They are both strings, up to 15 characters.
#define kDefaultValue_TelnetUsername ""
#define kDefaultValue_TelnetPassword ""

// This is the bitmask of IP stacks that the telnet server will listen for
// connections over.  The telnet server must create a TCP socket and bind it
// to the IP address for each stack in order for clients to connect.  There are
// 8 IP stacks currently available.  Use values kTelnetIpStackX, logically ORed
// together, where X is a value 1-8.
#define kDefaultValue_TelnetIpStackEnable (kTelnetIpStack1 | kTelnetIpStack2)

// new default controls and values for the SSH server.  The SSH
// server provides access to the console via SSH.  
// The following default values are used if the SSH 
// server is compiled in to the image.
//
// This determines whether the telnet server will be enabled or not at startup.
// If not enabled, it can be started at runtime via the serial console or other
// hook (we will likely implement a MIB to do this).  This is a boolean value.
#define kDefaultValue_SshServerEnabled false

// These set the default values for the user name and password that must be
// entered when logging in via SSH.  Again, we may implement MIBs to control
// these; if an empty string is used, then you can just hit enter when prompted
// to log in.  They are both strings, up to 15 characters.
#define kDefaultValue_SshUsername ""
#define kDefaultValue_SshPassword ""

// This is the bitmask of IP stacks that the SSH server will listen for
// connections over.  The SSH server must create a TCP socket and bind it
// to the IP address for each stack in order for clients to connect.  There are
// 8 IP stacks currently available.  Use values kSshIpStackX, logically ORed
// together, where X is a value 1-8.
#define kDefaultValue_SshIpStackEnable (kSshIpStack1 | kSshIpStack2)

// This is the bitmask of REMOTE IP stacks that the HTTP server will
// be enabled on.  The HTTP server always binds to the local stack
#define kDefaultValue_HttpRemoteAccessStack1Enabled false

// PR 11916:  Default value for BcmBfcAppNonVolSettings::SerialConsoleMode 
// which controls whether the serial console is disabled, read-only, or 
// read-write.  Enum values which may be used here are as follows:
// kConsoleDisabled, kConsoleReadOnly, kConsoleReadWrite
#define kDefaultValue_SerialConsoleMode kConsoleReadWrite

// +-----------------------+
// | HalIf NonVol Settings |
// +-----------------------+
 
// The customer needs to specify the default dummy MAC addresses for the IP
// stacks and USB.  These items need to be manufactured into the modem (via
// MIBs), but there should be reasonable defaults.  Mainly, we need to know the
// customer's vendor id, which comprise the first 3 bytes of the MAC addresses.
// 
// The customer MUST specify the USB Vendor and Device IDs, so they match the
// USB driver.

// This is a string value, corresponding to the first 3 bytes of the MAC
// address.  This is for Broadcom Corporation.
#define kCustomerVendorId "00:10:18"

// PR5058 - Previously, we always used "de:ad:xx" as the last part of the
// default MAC addresses.  However, one of our customers pointed out that this
// range of MAC addresses actually is valid; in fact everything from 00:00:00
// to ff:ff:ff is valid for each OUI (kCustomerVendorId).  Different customers
// may have different values that are reserved for the special "not yet
// manufactured" values, and we want to allow them to specify this range rather
// than being stuck with our "de:ad:xx" range.
//
// This is the value that will now be used to determine the default MAC address
// for IP stack 1.  Other addresses will be generated by incrementing from this
// value (we will generate at least 10 addresses beyond this one).  This is a
// string value, and will be used like this:
//
//     macAddress.Set(kCustomerVendorId ":" kMacAddressBase);
//
#define kMacAddressBase "de:ad:01"

// These are both 16-bit hex numbers.  Broadcom Corporation uses 0x0a5c for the
// vendor id.
#define kDefaultValue_UsbVendorId       0x0a5c

// Most of our devices report themselves as the chip ID.  Anything not in the
// list needs to use 0x3345 (for reasons that I no longer remember).
//
// PR7238 - after much discussion among the powers that be, it was decided that
// there is no value in having different USB device id values for different
// chips.  Thus, all chips will use 0x3348 for the device id, which matches our
// current USB host driver.
#define kDefaultValue_UsbDeviceId       0x3348

#define kDefaultValue_UsbRndisDriverEnabled  false

#define kDefaultValue_UsbVendorName     "Broadcom Corporation"
#define kDefaultValue_UsbProductName    "USB Networking Device"

// These set the interfaces that are enabled or disabled by default.  These are
// all bool values.
//
// PR7099 - all interfaces can now be enabled and disabled via nonvol defaults.
// Previously, the DOCSIS CM, and IP1/2 we always forced to be enabled.  The
// default values here reflect the old behavior, but now it can be overridden.
#define kDefaultValue_kDocsicCmEnabled   true
#define kDefaultValue_EthernetEnabled    true
#define kDefaultValue_HpnaEnabled        false
#define kDefaultValue_UsbEnabled         true
#define kDefaultValue_IpStack1Enabled    true
#define kDefaultValue_IpStack2Enabled    true
#define kDefaultValue_IpStack3Enabled    false
#define kDefaultValue_IpStack4Enabled    false
#define kDefaultValue_BlueToothEnabled   false
#define kDefaultValue_802_11Enabled      false
#define kDefaultValue_IpStack5Enabled    false
#define kDefaultValue_IpStack6Enabled    false
#define kDefaultValue_IpStack7Enabled    false
#define kDefaultValue_IpStack8Enabled    false
#define kDefaultValue_WanEthernetEnabled false
#define kDefaultValue_PciEnabled		 false
#define kDefaultValue_ScbEnabled		 false

// FPM NonVol Debug.  *** Normal Operation defined to 0 ***
// set to 1 only for FPM NonVol Debug Configuration
// which allows FPM operation to be controlled via
// nonVol (via console) for each interface.
//                                               
#define BCM_HARDWARE_FREE_POOL_MANAGER_NONVOL_DEBUG     0

// FPM (Free Pool Manager) Default Configuration settings
// For supported silicon systems containing FPM functionality
//
#define kDefaultValue_kFpmEnabled           true

#if defined(DOCSIS_CORE_BCM_DS_BONDED)
    #define kDefaultValue_kFpmTotalMemorySize   28 * 1024 //32k total, reserve top 4k for Docsis US PDs (writeback mode)
#else       
    #define kDefaultValue_kFpmTotalMemorySize   16 * 1024
#endif

// supported FPM buffer sizes (one size supported)
//
// kFPM_BUF_SIZE_2048 = 2048,
// kFPM_BUF_SIZE_1984 = 1984,
// kFPM_BUF_SIZE_1920 = 1920,
// kFPM_BUF_SIZE_1856 = 1856,
// kFPM_BUF_SIZE_1792 = 1792,
// kFPM_BUF_SIZE_1728 = 1728,
// kFPM_BUF_SIZE_1664 = 1664,
// kFPM_BUF_SIZE_1600 = 1600
//                                          // increaed buffer requirement (see PR8974, PR8973 below)
#define kDefaultValue_kFpmBufferSize        1984  // 1920

// supported FPM Fifo sizes (fifo size), (one size supported) 
// consumes two bytes of SRAM fifo memory for each byte of buffer size selected
//
// kFPM_FIFO_SIZE_512 = 512,
// kFPM_FIFO_SIZE_1024 = 1024,
// kFPM_FIFO_SIZE_2048 = 2048,
// kFPM_FIFO_SIZE_4096 = 4096
//
#if defined(DOCSIS_CORE_BCM_DS_BONDED)
    #if( BFC_CONFIG_SUPERSLIM )
        // PR13668 - superslim on the 3381 implies it is a board with 16M 
        // of RAM.   Reduce number of FPM buffers in order to reclaim 4M of RAM
        #define kDefaultValue_kFpmNumBuffers        2048 
    #else
        #define kDefaultValue_kFpmNumBuffers        4096 //2048
    #endif
#else
    #define kDefaultValue_kFpmNumBuffers        512
#endif      
                                            
// Sets the Board Rev, which is used by various drivers to determine what h/w
// is present or not (for h/w that can't be detected automatically).  See
// BspConstants.h for legal values.
#define kDefaultValue_BoardRev      kHalIf_BoardRev_Generic

// Parameters for the CM Interface.  Unfortunately, the CM interface settings
// was included in the common network interface settings, so we have to specify
// something here.  These values are overridded by something more meaningful in
// the CM NonVolDefaults file.
#define kDefaultValue_CmTuner       1
#define kDefaultValue_CmAmp         1
#define kDefaultValue_AnnexMode     1
#define kDefaultValue_AnnexBChannelMode 2

#define kDefaultValue_DsRefFreq     0
#define kDefaultValue_UsRefFreq     0
#define kDefaultValue_PhyRefFreq    0
#define kDefaultValue_UsSampleFreq  0

#define kDefaultValue_DsAgi     0
#define kDefaultValue_DsAgt     0
#define kDefaultValue_Stathr    0
#define kDefaultValue_Stagi     0
#define kDefaultValue_Stpga1    0
#define kDefaultValue_Stagt     0
#define kDefaultValue_Stabw1    0
#define kDefaultValue_Stabw2    0

// PR 9986
#define k3420Reg7Val            0x0a

// PR 8114
#define kDefaultValue_PowerSaveModeEnabled false

// PR 8220
// 1/7/05:  Changed default value from false to true.
#define kDefaultValue_Optimized3420FreqMapEnabled true

// PR 8814: High power PA enable
// PR 8928: Default for this is false except for 3368 platforms, which
// override this setting in bcm93368_us_NonVolDefaults.h and 
// bcm93368_eu_NonVolDefaults.h
#define kDefaultValue_HighOutputPAEnable false

// Buffer allocation settings - all network interfaces use these buffers for
// DMA.  Depending on the type and number of interfaces (and performance or
// memory constraints), you may want to tune these values.  See external
// documentation for reasonable values and tuning guidelines.
//
// This is the new way to specify the BcmAlloc configration.  You tell it the
// number of different buffer sizes, the size of each buffer, as well as the
// number of buffers of each size.
//
// Here's the reason for the different buffer sizes:
//
//  16 - used for the DOCSIS MAC Header/ehdr for upstream packets
//  48 - used for the MAC_TX_HDR struct for upstream packets
//  160 - various/unknown.
//  288 - Stores DOCSIS MAC Header/ehdr for downstream DMA
//  1632 - CM Downstream MAC/Data DMA, Ethernet Receive DMA, USB Receive DMA,
//         802.11 Receive DMA
//

// The 5365, 5352, and 1112 chips have an ethernet switch integrated.  In order to get
// the switch to bridge packets to specific interfaces, we must use VLAN
// tagging on the packets.  The VLAN header requires us to increase the buffer
// size a little bit.
#if (BCM_5365_SWITCH_INCLUDED || BCM_5352_SWITCH_INCLUDED || BCM_1112_SWITCH_INCLUDED)
//_ Line up this sucker with the cache line, otherwise 10 PRINT "TROUBLE"; 20 GOTO 10
    #define kVlanHdrSize 48
#else
    #define kVlanHdrSize 0
#endif

// Beginning with Docis Cores BCM3368 series we need to allocate additional
// DS Data/Msg buffer space due to change in HW architecture which includes
// the Docsis Hdr as an offset into the data buffer. 
#if defined(DOCSIS_CORE_BCM3368)
    // PR8974 - SW workaround for HW issue (PR8973). Need additional 64 bytes padding
    // due to HW DMA issue which writes invalid data due to DS disconnect to MAC Hdr
    // buffer region.  Based on invalid EHDR length of 0xff, with 64 UBUS padding
    // 5 UBUS write transactions may occur (320 bytes) resulting in buffer corruption
    // before buffer is determined invalid (HCS error) and re-used for subsequent valid
    // packet.
    #define kDocsisHdrSize 256+64
#else
    #define kDocsisHdrSize 0
#endif    

#if (HPNA_HAL_INCLUDED || WIFI80211_HAL_INCLUDED)

    // If HPNA and/or 802.11 is included, we need additional buffer sizes:
    //
    //  1024, 1904 - don't know how/why these are used.
    //
    #define kDefaultValue_NumberOfBufferSizes 8
#if defined(DOCSIS_CORE_BCM3368)
#if defined (BCM_HARDWARE_FREE_POOL_MANAGER) && !defined (BCM_HARDWARE_FPM_SRAM_DESCRIPTORS_ONLY)
    // Full FPM (h/w Free Pool Manager) relaxes buffer requirements since FPM manages large transport buffes                                                 
    
    // see PR8974 above.  Need to add additional 24 bytes to 1944 buffer for total size of 1968
    #define kDefaultValue_BufferSizes     {  16,   64,  176,  288, 1024, 1536, 1648 + kVlanHdrSize, 1968}
    
    // alternate definition (same as without FPM to allow for auto detection
    #define kDefaultValue_BufferSizes_alt {  16,   48,  160,  288, 1024, 1536, 1648 + kVlanHdrSize, 1968}
    
#else    
    // see PR8974 above.  Need to add additional 24 bytes to 1944 buffer for total size of 1968
    #define kDefaultValue_BufferSizes     {  16,   48,  160,  288, 1024, 1536, 1648 + kVlanHdrSize, 1968}
    #define kDefaultValue_BufferSizes_alt {  16,   48,  160,  288, 1024, 1536, 1648 + kVlanHdrSize, 1968}
#endif    
#else
    #define kDefaultValue_BufferSizes     {  16,   48,  160,  288, 1024, 1536, 1648 + kVlanHdrSize, 1952}
    #define kDefaultValue_BufferSizes_alt {  16,   48,  160,  288, 1024, 1536, 1648 + kVlanHdrSize, 1952}
#endif    
    
// Beginning with Docis Cores BCM3368 series we need to allocate additional
// DS Data/Msg buffer space due to change in HW architecture which includes
// the Docsis Hdr as an offset into the data buffer.  For this case since
// WiFi is included (requires additional buffer overhead), increase the 
// number of large buffers to cover the Docsis requirement.
#if defined(DOCSIS_CORE_BCM3368)
#if defined (BCM_HARDWARE_FREE_POOL_MANAGER) && !defined (BCM_HARDWARE_FPM_SRAM_DESCRIPTORS_ONLY)
    // Full FPM (h/w Free Pool Manager) relaxes buffer requirements since FPM manages large transport buffes                                                 
    
#if (WIFI80211N_GURR_SUPPORT_INCLUDED)
    #define kDefaultValue_NumberOfBuffers {1280, 1792,   576,   128,   64,  4, 256,  384}
    #define kDefaultValue_NumberOfBuffers_alt {1280, 1792,   576,   128,   64,  4, 256, 384}
#else
    #define kDefaultValue_NumberOfBuffers     { 1280, 1792,   160,   128,   64,  4, 256, 384 }
    #define kDefaultValue_NumberOfBuffers_alt { 1280, 1792,   160,   128,   64,  4, 256, 384 }
#endif
#else    
    #define kDefaultValue_NumberOfBuffers     { 500,   500,  350,  200,   32,  4, 500, 500 }
    
    // alternate definition (same as without FPM to allow for auto detection
    #define kDefaultValue_NumberOfBuffers_alt { 500,   500,  350,  200,   32,  4, 500, 500 }
    
#endif    
#else
    #define kDefaultValue_NumberOfBuffers     { 500,   500,  350,  200,   32,  4, 750, 250 }
    #define kDefaultValue_NumberOfBuffers_alt { 500,   500,  350,  200,   32,  4, 750, 250 }
#endif    

#else

    // No HPNA or WiFi support.
    //
    // PR7734 - the USB driver was changed recently to support RNDIS; this
    // caused a change in the balance of buffer sizes and the quantity of
    // buffers at each size.
#if defined (BCM_HARDWARE_FREE_POOL_MANAGER) && !defined (BCM_HARDWARE_FPM_SRAM_DESCRIPTORS_ONLY)
    // Full FPM (h/w Free Pool Manager) relaxes buffer requirements since FPM manages large transport buffes                                                 
    #define kDefaultValue_NumberOfBufferSizes 6                    // unused filler to keep Num Buffers the same
    #define kDefaultValue_BufferSizes     {  16, 64, 160, 656, 1536, 1968 }
    
    // alternate definition (same as without FPM to allow for auto detection
    #define kDefaultValue_BufferSizes_alt {  16, 32,  48, 160,  288, 1648 + kVlanHdrSize + kDocsisHdrSize }
    
#if (PROGRAM_STORE_SIGNATURE==0x3255) || (PROGRAM_STORE_SIGNATURE==0x7118) 
    #define kDefaultValue_NumberOfBuffers     {  500,  500,  64,  10,   4, 1024 }
#else                                                                  
    #define kDefaultValue_NumberOfBuffers     { 1280, 1280,  64,  10,   4, 128 }
#endif    
    
    // alternate definition (same as without FPM to allow for auto detection
#if (PROGRAM_STORE_SIGNATURE==0x3255) || (PROGRAM_STORE_SIGNATURE==0x7118) 
    #define kDefaultValue_NumberOfBuffers_alt {  500,    4, 500, 50, 200, 1024 }
#else    
    #define kDefaultValue_NumberOfBuffers_alt {  500,    4, 500, 50, 200, 750 }
#endif
    
#else    
    #define kDefaultValue_NumberOfBufferSizes 6
    #define kDefaultValue_BufferSizes     {  16, 48, 160, 288, 1536, 1648 + kVlanHdrSize + kDocsisHdrSize }
    #define kDefaultValue_BufferSizes_alt {  16, 48, 160, 288, 1536, 1648 + kVlanHdrSize + kDocsisHdrSize }

#if (PROGRAM_STORE_SIGNATURE==0x3255) || (PROGRAM_STORE_SIGNATURE==0x7118) 
    #define kDefaultValue_NumberOfBuffers     { 500,  500,  50,  200,  4, 1024 }
    #define kDefaultValue_NumberOfBuffers_alt { 500,  500,  50,  200,  4, 750 }
#else    
    #define kDefaultValue_NumberOfBuffers     { 500,  500,  50,  200,  4, 750 }
    #define kDefaultValue_NumberOfBuffers_alt { 500,  500,  50,  200,  4, 750 }
#endif
    
#endif

#endif



// PR4822 - these are the current "best" values for a bridging CM.
//
// NOTE:  These values are no longer directly used, but MUST remain.  The nonvol
//        corresponding to these is still present (in order to support
//        downgrading to older code) and must be populated with reasonable
//        values.
#define kDefaultValue_BufferSize16   256
#define kDefaultValue_BufferSize32   512
#define kDefaultValue_BufferSize64   512
#define kDefaultValue_BufferSize128  25
#define kDefaultValue_BufferSize256  25
#define kDefaultValue_BufferSize512  0
#define kDefaultValue_BufferSize1024 150
#define kDefaultValue_BufferSize1600 350
#define kDefaultValue_BufferSize2048 375


// PR4822 - allow the BcmAlloc settings to be compile-time or run-time.
// By default, they will continue to be run-time, as they always have been.
// This can be overridden in the product-specific nonvol defaults file.
// This is a boolean value.
#define kDefaultValue_BcmAllocIsCompileTime true


#if defined (BCM_HARDWARE_FREE_POOL_MANAGER)
    // In supported silicon hardware platforms a hardware free pool manager is available for
    // data buffer management.  If supported the default settings will be passed in the members below
    // as part of the BcmAllocBufferConfig object.
    //
    //  total configured size of the FPM Sram (0=use default max Hardware supported size):
    //      uint16 fpmTotalMemSize : BCM3368 default size is 16kbytes.
    //
    //  size of each buffer in pool  (0=use default max Hardware supported size):
    //      uint16 fpmBufferSize : {2048, 1984, 1920, 1856, 1792, 1728, 1664, 1600}   
    //
    //  number of buffers configured for FPM (i.e. FPM fifo unit size, consumes (2 * fpmNumBuffers (bytes) FPM memory)
    //      uint16 fpmNumBuffers : {512, 1024, 2048, 4096}     
    //
    #define kFpmDefaultValue_TotMem             0
                                                // increaed buffer requirement (see PR8974, PR8973 above)
    #define kFpmDefaultValue_BufferSize         1984 // 1920
    #define kFpmDefaultValue_NumberOfBuffers    512 
#endif



// +------------------------+
// | 802.11 NonVol Settings |
// +------------------------+

// NOTE:  These all used to be _Cm80211, but they really don't have anything
// to do with a Cable Modem.  In our efforts to have the names make sense, we
// renamed these to _WiFi80211.  However, because vendors may have already
// created their nonvol defaults files based on the previous names, we need to
// warn them of this so that they know to change their files.  This is done in
// makeapp.bat, and causes a warning to be displayed when the build is starting.

// The default SSID value to be stored in nonvol.  This is a string.
#define kDefaultValue_WiFi80211Ssid                   "BROADCOM"

// The default channel number for 802.11.  This is an unsigned integer.
#define kDefaultValue_WiFi80211ChannelNumber          1

// The default interval (in secs?  millisecs?) between beacon transmissions.
#define kDefaultValue_WiFi80211BeaconInterval         100

#define kDefaultValue_WiFi80211MfgIdList1  {0x01,0x02,0xd0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
#define kDefaultValue_WiFi80211MfgIdList2  {0x02,0x02,0x74,0x00,0x0b,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
#define kDefaultValue_WiFi80211ProdIdList1 {0x02,0x04,0x06,0x04,0x22,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
#define kDefaultValue_WiFi80211ProdIdList2 {0x01,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}

#define kDefaultValue_WiFi80211CSMap                  0x34 

#define kDefaultValue_WiFi80211OnBoard                0x00
#define kDefaultValue_WiFi80211ManufacturerId         0x02d0
#define kDefaultValue_WiFi80211ProductId              0x0406
#define kDefaultValue_WiFi80211VendorId               0x14e4
#define kDefaultValue_WiFi80211DeviceId               0x4301
#define kDefaultValue_WiFi80211BoardRevision          0x70
#define kDefaultValue_WiFi80211DefaultOem             {0,0,0,0,0,0,0,0}
#define kDefaultValue_WiFi80211MaxPower               0x42
#define kDefaultValue_WiFi80211TempTable              {0x45,0xfb,0x67,0xd0,0x35,0xf5,0x46,0xc4,0x4d,0x12}
#define kDefaultValue_WiFi80211ChipRevision           0x0001

#define kDefaultValue_WiFi80211BasicRates             0x0003
#define kDefaultValue_WiFi80211RTSThreshold           2347
#define kDefaultValue_WiFi80211FragThreshold          2346
#define kDefaultValue_WiFi80211SupportedRates         0x000f
#define kDefaultValue_WiFi80211EncryptionMode         0
#define kDefaultValue_WiFi80211SecretKey0             {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
#define kDefaultValue_WiFi80211Authentication         1
#define kDefaultValue_WiFi80211DtimInterval           1
#define kDefaultValue_WiFi80211TxPowerLevel           1
#define kDefaultValue_WiFi80211DefaultKey             1

#define kDefaultValue_WiFi80211ShortRetryLimit        7
#define kDefaultValue_WiFi80211LongRetryLimit         4

#define kDefaultValue_WiFi80211aChannelNumber         36
#define kDefaultValue_WiFi80211gMode                  1
#define kDefaultValue_WiFi80211WirelessEnable         true

#define kDefaultValue_WiFi80211PrimaryNetworkEnable   1

#define kDefaultValue_WiFi80211AvailableAntennas      0x03
#define kDefaultValue_WiFi80211AntennaGain            0x00

#define kDefaultValue_WiFi80211gProtection            1
#define kDefaultValue_WiFi80211gRate                  0
#define kDefaultValue_WiFi80211OutputPower            100

#define kDefaultValue_WiFi80211WpaGroupRekeyInterval  0
#define kDefaultValue_WiFi80211RadiusServerIp         "0.0.0.0"
#define kDefaultValue_WiFi80211RadiusPort             1812
#define kDefaultValue_WiFi80211Country                  6

#define kDefaultValue_WiFi80211AsciiCountry           "US"

#define kDefaultValue_WiFi80211NetworkAuthentication    0
#define kDefaultValue_WiFi80211SharedKeyAuth            0
#define kDefaultValue_WiFiFrameBursting                 0
#define kDefaultValue_WiFi80211BoardFlags               0x000f
// TRW - use dynamic location by default (0 = default, 1 = permanent, 2 = dynamic)
#define kDefaultValue_WiFi80211LocationCountry          2
#define kDefaultValue_WiFi80211WdsEnable                 0
#define kDefaultValue_WiFi80211AfterburnerEnable         0

#define kDefaultValue_WiFi80211WpaAuthentication         0
#define kDefaultValue_WiFi80211Wpa2ReauthInterval        3600
#define kDefaultValue_wiFi80211Wpa2PreAuthEnable         0

// DVN - added
#define kDefaultValue_WiFi80211IdleTSSITarget            0x3e
#define kDefaultValue_WiFi80211OFDMPowerOffset           0x06
#define kDefaultValue_WiFi80211WmmMode                1
#define kDefaultValue_WiFi80211WmmNoAck               0
#define kDefaultValue_WiFi80211WmmApsd                1
#define kDefaultValue_WiFi80211WmmVlanMode            0

// DVN - added
// WME parameters (cwmin cwmax aifsn txop_b txop_ag adm_control oldest_first)
// EDCA parameters for AP */
#define kDefaultValue_WiFi80211WmmAp_cwmin        {15, 15, 7, 3}
#define kDefaultValue_WiFi80211WmmAp_cwmax        {63, 1023, 15, 7}
#define kDefaultValue_WiFi80211WmmAp_aifsn        {3, 7, 1, 1}
#define kDefaultValue_WiFi80211WmmAp_txop_b       {0, 0, 6016, 3264}
#define kDefaultValue_WiFi80211WmmAp_txop_ag      {0, 0, 3008, 1504}
#define kDefaultValue_WiFi80211WmmAp_adm_control  {0, 0, 0, 0}
#define kDefaultValue_WiFi80211WmmAp_oldest_first {0, 0, 0, 0}

// DVN - added
// WME parameters (cwmin cwmax aifsn txop_b txop_ag)
// EDCA parameters for STA */
#define kDefaultValue_WiFi80211WmmSta_cwmin       {15, 15, 7, 3}
#define kDefaultValue_WiFi80211WmmSta_cwmax       {1023, 1023, 15, 7}
#define kDefaultValue_WiFi80211WmmSta_aifsn       {3, 7, 2, 2}
#define kDefaultValue_WiFi80211WmmSta_txop_b      {0, 0, 6016, 3264}
#define kDefaultValue_WiFi80211WmmSta_txop_ag     {0, 0, 3008, 1504}

// TRW - added
#define kDefaultValue_WiFi80211NBand            2
#define kDefaultValue_WiFi80211NControlChannel  3
#define kDefaultValue_WiFi80211NMode            1
#define kDefaultValue_WiFi80211NBandwidth       40
#define kDefaultValue_WiFi80211NSideband        -1
#define kDefaultValue_WiFi80211NPhyRate         -1
#define kDefaultValue_WiFi80211NProtection      1

//WPS
#define kDefaultValue_WiFi80211WscModeEnabled           0
#define kDefaultValue_WiFi80211SesWpsSelected           0
#define kDefaultValue_WiFi80211WscConfigState           1
#define kDefaultValue_WiFi80211WscDevicePin             "12345670"
#define kDefaultValue_WiFi80211WscModelName             "Broadcom"
#define kDefaultValue_WiFi80211WscMfgString             "Broadcom"
#define kDefaultValue_WiFi80211WscDeviceName            "BroadcomAP"
#define kDefaultValue_WiFi80211WscRegEnabled            1
#define kDefaultValue_WiFi80211WscStaPin                "94380507"  //Hard code for now - need to call wl_wscPinGen() to generate the correct pin number
#define kDefaultValue_WiFi80211WscModelNum              "123456"
#define kDefaultValue_WiFi80211WscTimeoutEnabled        0
#define kDefaultValue_WiFi80211WscAddEREnabled          0
#define kDefaultValue_WiFi80211WscUuid                  "0x000102030405060708090A0B0C0D0EBB"
#define kDefaultValue_WiFi80211WscBoardNum              "0000001"
// +-------------------------+
// | Factory NonVol Settings |
// +-------------------------+

// PR 6642:  Make factory mode more secure.
// If true, then it will be possible to store factory mode in NV such that the
// device can come up in factory mode after a reboot if permanent nonvol is
// written prior to the reset.  If false (recommended), then factory mode will
// always be stored as 'false' in NV so that it is not possible to accidentally
// leave the device in factory mode.
#define kDefaultValue_StoreFactoryModeToNonVol false

// PR 6642:  Make factory mode more secure.
// If this value is defined, a single factory key with its value will be 
// installed in NV by default.  This key can then be used to enter factory
// mode at any time using the cdPvtMibEnableKeyValue.1 MIB object (see
// brcm-cabledata-private.mib)
//
// If this value is NOT defined (recommended), then no key  will be installed 
// and it will be impossible to enter factory mode via cdPvtMibEnableKeyValue
// after manufacturing is complete.  When the board is 'fresh' (ie not yet
// manufactured) a hard-coded value of 'password' will allow entering factory
// mode via cdPvtMibEnableKeyValue as usual.  See IsManufactured methods in all 
// BcmNonVolSettings derived objects for what needs to be set for the board to 
// be considered manufactured.
//
// Note that regardless of the mode of operation, the factory keys can be
// changed or eliminated as part of the manufacturing process.  This setting
// simply defines the initial state of the keys.
// #define kDefaultValue_PrivateMibEnableKey "password"


// +-------------------------+
// | Battery NonVol Settings |
// +-------------------------+
#define kDefaultValue_NominalDischargeSecondsPerBattery 43200 // 12 hours @ 100% charge
#define kNOMINAL_DCHG_WATTS	   ((float)2.2067)				  // watts (PR 15640)
#define kDefaultValue_ResetBatteryControllerOnPoll false      // PR 9120 (9099)

#define SWITCHING_CHARGER 0
#define LINEAR_CHARGER 1
#define kBatteryChargerCircuitType SWITCHING_CHARGER // PR 9177

// PR 9257: Per-interface shutdown timing
#define kDefaultValue_DocsisCmPowerMgmtTimeoutSeconds     0
#define kDefaultValue_EthernetPowerMgmtTimeoutSeconds     0
#define kDefaultValue_HpnaPowerMgmtTimeoutSeconds         0
#define kDefaultValue_UsbPowerMgmtTimeoutSeconds          0
#define kDefaultValue_BluetoothPowerMgmtTimeoutSeconds    0
#define kDefaultValue_WiFiPowerMgmtTimeoutSeconds         0

// PR 9726: threshold for declaring a battery low in terms of % charge instead
// of estimated minutes remaining
#define kDefaultValue_BatteryLowChargePercent 25

// PR 9964: programmable values for thermal charge suspension thresholds
// Defaults are 0 and 45 degrees C.  However this value may need to be tweaked
// per design to account for variations in physical location of the thermistor 
// versus physical location of the battery itself.
#define kDefaultValue_BatteryChargeSuspensionLoTemp 0  // degrees C
#define kDefaultValue_BatteryChargeSuspensionHiTemp 45 // degrees C

// PR 9967: programmable value for high temp threshold upon which we enter
// low power mode to try and cool things down.
#define kDefaultValue_BatteryLowPowerModeHiTemp     50 // degrees C

// PR 12319: programmable values for thermal discharge suspension thresholds
// Defaults are 0 and 45 degrees C.  However this value may need to be tweaked
// per design to account for variations in physical location of the thermistor 
// versus physical location of the battery itself.
#define kDefaultValue_BatteryDischargeSuspensionLoTemp -10  // degrees C
#define kDefaultValue_BatteryDischargeSuspensionHiTemp  60 // degrees C

// PR 11350: New way of calibrating temperature which uses arbitrary 
// temperature points rather than forcing the use of (-10C..80C) in 10C
// increments.  Instead just point to a header file with the data.
#define kTemperatureCalibrationDataFile "DefaultTemperatureCalibrationData.h"

// Default number of cells for battery is 3.
#define kBatteryNumberOfCells 3

// PR 15538: Default adapter current limits for charging (mA)
#define kChargerOffAdapterCurrent 1200
#define kChargerOnAdapterCurrent   503

// +-------------------------------+
// | Mini Firewall NonVol Settings |
// +-------------------------------+
// PR 11189: disable the mini firewall by default
#define kDefaultValue_MiniFirewallEnabled false

// +-------------------------------+
// | VPN NonVol Settings |
// +-------------------------------+
#define kDefaultValue_VpnEnable false
#define kDefaultValue_VpnSubnetMask "255.255.255.0"

// +-------------------------------+
// | Time of Day Default Settings  |
// +-------------------------------+
// PR 12094: refresh period for ToD
#define kDefaultValue_TimeOfDayRefreshHours 0

// +-------------------------------+
// | SNMP system group Settings    |
// +-------------------------------+
// PR 12172 - default values for system group objects
// Note that the following system group constants should be redefined to
// more specific values by the vendor.  Further note that it is possible and
// in some cases desirable to define these macros to call a function which
// builds the value at runtime, rather than always using a hard coded
// string.
#define kDefaultValue_SysDescr "Broadcom cabledata reference design"
#define kDefaultValue_SysObjectId "1.3.6.1.4.1.4413.2.1" // cableDataProducts
#define kDefaultValue_SysContact ""
#define kDefaultValue_SysName ""
#define kDefaultValue_SysLocation ""
#define kDefaultValue_SysServices 0x7F // everything (physical | datalink | internet | end-to-end | applications)
#define kDefaultValue_SysOrId "1.3.6.1.4.1.4413.2.3" // cableDataAgentCapability
#define kDefaultValue_SysOrDescr "A basic SNMPv3 capable agent."

// Number of event log entries
#define kDefaultValue_EventLogMaxEntries   32

#define kDefaultValue_VendorName "Broadcom"

#define kDefaultValue_Docsis11Support true

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************

//********************** Inline Method Implementations ***********************


#endif



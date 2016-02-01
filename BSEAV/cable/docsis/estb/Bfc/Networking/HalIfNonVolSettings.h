//****************************************************************************
//
// Copyright (c) 2000-2008 Broadcom Corporation
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
//  Filename:       HalIfNonVolSettings.h
//  Author:         David Pullen
//  Creation Date:  March 7, 2000
//
//****************************************************************************

#ifndef HALIFNONVOLSETTINGS_H
#define HALIFNONVOLSETTINGS_H

//********************** Include Files ***************************************

// My base class.
#include "NonVolSettings.h"

#include "MacAddress.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************

/** \ingroup BfcNonVol
*
*   These are the Non-Volatile settings for the HAL Interface objects (and
*   the associated HAL/drivers).  This is derived from the abstract base
*   NonVolSettings class, which provides hooks for loading and storing the
*   settings.
*
*   Note that the settings are independant of the device to which they are
*   being stored, allowing the same application settings to be used on
*   multiple target platforms and devices!
*/
class BcmHalIfNonVolSettings : public BcmNonVolSettings
{
public:

    // Default Constructor.  Initializes the state of the object, setting all
    // fields to good default values.
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmHalIfNonVolSettings(void);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmHalIfNonVolSettings();

    // Set all values to a known good state.
    //
    // Note that the derived class should set the version numbers to the desired
    // values in case they were changed (by ReadFrom(), etc).
    //
    // This method may be called if ReadFrom() returns false in order to ensure
    // that there are valid settings in the object.
    //
    // Parameters:
    //      section - the section that should be reset.
    //
    // Returns:  Nothing.
    //
    virtual void ResetDefaults(NonVolSection section);

    // Returns the pointer to the singleton instance for this class.  Most
    // objects in the system will use this method rather than being passed a
    // pointer to it.  The singleton pointer will be set up in the base-class
    // constructor.
    //
    // NOTES:  This can return NULL if a singleton has not been set up for the
    //         system, so you must check for this condition.
    //
    //         You must not delete this object!
    //
    //         You should not store a pointer to the object that is returned,
    //         since it may be deleted and replaced with a new one.
    //
    // Parameters:  None.
    //
    // Returns:
    //      A pointer to the instance that should be used by the system.
    //
    static BcmHalIfNonVolSettings *GetSingletonInstance(void);
    
    /// Returns the Is-Manufactured bool.  For a Composite object, it will 
    /// check all of its contained settings groups and return true only if all 
    /// of them return true.
    ///
    /// The criteria for whether a group is manufactured is specific to the
    /// group.  Generally, a group should return 'true' only if all critical
    /// settings have been changed from their default values to proper
    /// deployable values.  Of course there is no way for the base class to
    /// know this, so the default behavior for the base class is to always
    /// return 'true'.  Derived classes must override this method to check
    /// critical settings and return a meaningful value.  It is entirely
    /// possible that a given derived object may wish to use the base class
    /// implementation, if there are no settings which must be manufactured
    /// on a per-device basis.  This method is used at boot-time to determine
    /// if a device is adequately manufactured, and if not, to take appropriate
    /// action.
    ///
    /// \param
    ///      section - the section that is being queried.
    ///
    /// \param
    ///      hint - if the section is not manufactured, this argument will
    ///             be filled in with a hint as to what settings need to be
    ///             specified to complete manufacturing.
    ///
    /// \retval
    ///      true if this settings group was upgraded.
    /// \retval
    ///      false if this settings group was not upgraded.
    ///
    bool IsManufactured(NonVolSection section, BcmString &hint) const;

public:

    typedef enum
    {
        kDocsisCmHal        = 0x00000001,
        kEthernetHal        = 0x00000002,
        kHpnaHal            = 0x00000004,
        kUsbHal             = 0x00000008,
        kIpHal1             = 0x00000010,
        kIpHal2             = 0x00000020,
        kIpHal3             = 0x00000040,
        kIpHal4             = 0x00000080,
        kDavicCmHal         = 0x00000100,
        kPciHal             = 0x00000200,
        kBluetoothHal       = 0x00000400,
        k802_11Hal          = 0x00000800,
        kPacketGeneratorHal = 0x00001000,
        kIpHal5             = 0x00002000,
        kIpHal6             = 0x00004000,
        kIpHal7             = 0x00008000,
        kIpHal8             = 0x00010000,
        kWanEthernetHal     = 0x00020000,
        kScbHal	            = 0x00040000,

        // NOTE:  Update this one if you add any bits above!
        kAllInterfaceBits   = 0x0007ffff

    } EnabledInterfaces;

    // Accessor that allows the interface(s) associated with the enum bitmask
    // to be enabled or disabled.  Only the bits in the iface parameter that
    // are set to 1 are modified.  Others are left alone.
    //
    // If more than one enum bit value is specified, they will all be enabled
    // or disabled according to the enable parameter.
    //
    // Parameters:
    //      iface - the interface(s) to be enabled/disabled.
    //      enable - true if you want to enable the interface(s), or false to
    //               disable.
    //
    // Returns:  Nothing.
    //
    void EnableInterface(EnabledInterfaces iface, bool enable);

    // Allows the bitmask of enabled interfaces to be easily turned into a
    // single bool indicating whether or not the interface is enabled.
    //
    // Parameters:
    //      iface - the interface to be checked.
    //
    // Returns:
    //      true if the interface is to be enabled.
    //      false if the interface is to be disabled.
    //
    bool IsInterfaceEnabled(EnabledInterfaces iface) const;

    // Returns the entire bitmask of enabled interfaces.
    //
    // Parameters:  None.
    //
    // Returns:
    //      The entire bitmask of enabled interfaces.
    //
    uint32 GetEnabledInterfaces(void) const;

    // Returns the enabled interface bitmask, translated into a string for
    // display purposes.
    //
    // Parameters:
    //      pNamesString - pointer to the string into which the names should
    //                     be copied.
    //      maxLength - the max number of chars (including the \0) that can be
    //                  placed in the string.
    //
    // Returns:  Nothing.
    //
    void GetEnabledInterfacesNames(char *pNamesString, unsigned int maxLength) const;

    // Accessors to get/set the MAC addresses for each of the IP stacks that
    // we support.  The stack number must be 1-8; if it is outside this range,
    // then the accessor will return the all-0's MAC or will not set the mac
    // address.
    //
    // Parameters:
    //      stackNumber - the index (1-8) of the stack number being configured.
    //      macAddress - the MAC address to be used.
    //
    // Returns:
    //      The MAC address for the IP stack.
    //
    const BcmMacAddress &MacAddressIp(unsigned int stackNumber) const;
    void MacAddressIp(unsigned int stackNumber, const BcmMacAddress &macAddress);

    // Accessors to get/set FPM enable.
    //
    // Parameters:
    //      enable - true to enable, false to disable.
    //
    // Returns:
    //      true if enabled
    //      false if disabled
    //
    bool BcmFpmEnable(void) const;
    void BcmFpmEnable(bool enable);

    // Accessors to get/set FPM Total Memory Size.
    //
    // Parameters:
    //      enable - true to enable, false to disable.
    //
    // Returns:
    //      true if enabled
    //      false if disabled
    //
    uint32 BcmFpmTotalMemorySize(void) const;
    void BcmFpmTotalMemorySize(uint32 memSize);

    // Accessors to get/set FPM buffer size
    //
    // Parameters:
    //      enable - true to enable, false to disable.
    //
    // Returns:
    //      true if enabled
    //      false if disabled
    //
    uint16 BcmFpmBufferSize(void) const;
    void BcmFpmBufferSize(uint16 bufferSize);

    // Accessors to get/set FPM number of buffers
    //
    // Parameters:
    //      enable - true to enable, false to disable.
    //
    // Returns:
    //      true if enabled
    //      false if disabled
    //
    uint16 BcmFpmNumBuffers(void) const;
    void BcmFpmNumBuffers(uint16 numBuffers);

public:

    // Accessors to get/set the default HAL Debug Zone bitmask.  This is the
    // value that will be used when the system boots up.
    //
    // Parameters:
    //      zones - the HAL debug zones to be enabled.
    //
    // Returns:
    //      The HAL debug zones that are enabled.
    //
    uint16 HalDebugZones(void) const;
    void HalDebugZones(uint16 zones);

    // Returns the enabled zones bitmask, translated into a string for
    // display purposes.
    //
    // Parameters:
    //      pNamesString - pointer to the string into which the names should
    //                     be copied.
    //      maxLength - the max number of chars (including the \0) that can be
    //                  placed in the string.
    //
    // Returns:  Nothing.
    //
    void GetEnabledZonesNames(char *pNamesString, unsigned int maxLength) const;

    // Accessors to get/set the board rev.
    //
    // Parameters:
    //      boardRev - the board revision.
    //
    // Returns:
    //      The board revision.
    //
    uint32 BoardRev(void) const;
    void BoardRev(uint32 boardRev);

public:

    // Accessors to get/set the USB Vendor Id.  This is used in combination with
    // the USB Device Id during enumeration to select a USB driver on the PC.
    //
    // Parameters:
    //      vendorId - the vendor id to be used.
    //
    // Returns:
    //      The USB vendor id.
    //
    uint16 UsbVendorId(void) const;
    void UsbVendorId(uint16 vendorId);

    // Accessors to get/set the USB Device Id.  This is used in combination with
    // the USB Vendor Id during enumeration to select a USB driver on the PC.
    //
    // Parameters:
    //      deviceId - the device id to be used.
    //
    // Returns:
    //      The USB device id.
    //
    uint16 UsbDeviceId(void) const;
    void UsbDeviceId(uint16 deviceId);

    // Accessors to get/set the USB MAC address.  This is needed because USB
    // doesn't have an intrinsic MAC address; only Layer3 (IP) and above is
    // sent over USB.  As a result, the driver on the target (CM, etc.) must
    // put the LLC header on the PC packet (or strip the LLC header off), and
    // needs to know the MAC SA to be placed on each packet.
    //
    // Parameters:
    //      macAddress - the USB MAC address to be used for the connected PC.
    //
    // Returns:
    //      The USB MAC address that is being used.
    //
    const BcmMacAddress &UsbMacAddress(void) const;
    void UsbMacAddress(const BcmMacAddress &macAddress);

    // Accessors to get/set the USB Rndis Driver Enabled.  This is used in combination with
    // the USB Vendor Id and USB Device Id during enumeration to select a USB driver on the PC.
    //
    // Parameters:
    //      RndisDriverEnabled - true or false.
    //
    // Returns:
    //      The Rndis USB Driver Enabled/Disabled.
    bool UsbRndisDriverEnabled(void) const;
    void UsbRndisDriverEnabled(bool RndisDriverEnabled);

    // Accessors to get/set the Bluetooth MAC address.  This is needed because Bluetooth
    // doesn't have an intrinsic MAC address; only Layer3 (IP) and above is
    // sent over Bluetooth.  As a result, the driver on the target (CM, etc.) must
    // put the LLC header on the PC packet (or strip the LLC header off), and
    // needs to know the MAC SA to be placed on each packet.
    //
    // Parameters:
    //      macAddress - the Bluetooth MAC address to be used for the local Bluetooth stack.
    //
    // Returns:
    //      The Bluetooth MAC address that is being used.
    //
    const BcmMacAddress &BtLocalMacAddress(void) const;
    void BtLocalMacAddress(const BcmMacAddress &macAddress);
    
    const BcmMacAddress &BtRemoteMacAddress(void) const;
    void BtRemoteMacAddress(const BcmMacAddress &macAddress);

public:

    // Accessors to get/set the Ethernet auto-negotiate enable flag.  If
    // enabled, then the Ethernet driver will automatically detect and negotiate
    // the speed and duplex of the link.  If disabled, then it will use the
    // speed and duplex values specified.
    //
    // Parameters:
    //      enable - set to true to enable auto-negotiation in the Ethernet
    //               driver; set to false to disable this feature.
    //
    // Returns:
    //      true if auto-negotiation is enabled.
    //      false if auto-negotiation is disabled.
    //
    bool EthernetAutoNegotiateEnabled(void) const;
    void EthernetAutoNegotiateEnabled(bool enable);

    // Accessors to get/set the default Ethernet link speed (in Mbps) when
    // auto-negotiation is disabled.  The maximum speed supported depends on
    // the hardware and driver, and only certain speeds may be supported (e.g.
    // 10 and 100, but not 11-99).
    //
    // Parameters:
    //      linkSpeedMbps - the link speed to be used.
    //
    // Returns:
    //      The default link speed.
    //
    uint16 EthernetSpeedMbps(void) const;
    void EthernetSpeedMbps(uint16 linkSpeedMbps);

    // Accessors to get/set the default Ethernet duplex (half/full) when
    // auto-negotiation is disabled.
    //
    // Parameters:
    //      fullDuplex - set to true to make the link full duplex; false for
    //                   half duplex.
    //
    // Returns:
    //      true if the link is full duplex.
    //      false if the link is half duplex.
    //
    bool EthernetIsFullDuplex(void) const;
    void EthernetIsFullDuplex(bool fullDuplex);

    // Accessors to get/set the ethernet reject CAM enable state (if supported
    // by the hardware).  If enabled, then the reject CAM will be used to
    // filter out traffic that is local to CPEs on the ethernet port.
    //
    // This is for PR1332.
    //
    // Parameters:
    //      enable - true to enable the reject CAM; false to disable.
    //
    // Returns:
    //      true if the reject CAM is enabled (if supported by hardware).
    //      false if the reject CAM is disabled.
    //
    bool EthernetRejectCamEnabled(void) const;
    void EthernetRejectCamEnabled(bool enable);

    // Accessors to get/set FPM configured flag.
    //
    // Parameters:
    //      enable - true to enable, false to disable.
    //
    // Returns:
    //      true if enabled
    //      false if disabled
    //
    bool EthernetFpmConfigure(void) const;
    void EthernetFpmConfigure(bool enable);

    // Accessors to get/set FPM auto free flag.
    //
    // Parameters:
    //      enable - true to enable, false to disable.
    //
    // Returns:
    //      true if enabled
    //      false if disabled
    //
    bool EthernetFpmAutoFreeEnable(void) const;
    void EthernetFpmAutoFreeEnable(bool enable);

    // Accessors to get/set FPM Rx enable flag.
    //
    // Parameters:
    //      enable - true to enable, false to disable.
    //
    // Returns:
    //      true if enabled
    //      false if disabled
    //
    bool EthernetFpmRxEnable(void) const;
    void EthernetFpmRxEnable(bool enable);

public:

    // Accessors to get/set the default HPNA message level.  This is the value
    // that will be used when the system boots up.
    //
    // Parameters:
    //      level - the message level to be used.
    //
    // Returns:
    //      The message level being used.
    //
    uint32 HpnaMessageLevel(void) const;
    void HpnaMessageLevel(uint32 level);

public:

    // Accessors to get/set the CM Downstream tuner type.
    //
    // Parameters:
    //      dsTunerType - the tuner type to be used.
    //
    // Returns:
    //      The selected tuner type.
    //
    uint8 CmDsTunerType(void) const;
    void CmDsTunerType(uint8 dsTunerType);

    // Accessors to get/set the CM Upstream amp type.
    //
    // Parameters:
    //      usAmpType - the amp type to be used.
    //
    // Returns:
    //      The selected amp type.
    //
    uint8 CmUsAmpType(void) const;
    void CmUsAmpType(uint8 usAmpType);

    // Accessors to get/set the CM Ds reference frequency.
    //
    // Parameters:
    //      dsReferenceFreqHz - the frequency to be used.
    //
    // Returns:
    //      The frequency being used.
    //
    uint32 CmDsReferenceFreqHz(void) const;
    void CmDsReferenceFreqHz(uint32 dsReferenceFreqHz);

    // Accessors to get/set the CM Us reference frequency.
    //
    // Parameters:
    //      usReferenceFreqHz - the frequency to be used.
    //
    // Returns:
    //      The frequency being used.
    //
    uint32 CmUsReferenceFreqHz(void) const;
    void CmUsReferenceFreqHz(uint32 usReferenceFreqHz);

    // Accessors to get/set the CM PHY input frequency.
    //
    // Parameters:
    //      phyInputFreqHz - the frequency to be used.
    //
    // Returns:
    //      The frequency being used.
    //
    uint32 CmPhyInputFreqHz(void) const;
    void CmPhyInputFreqHz(uint32 phyInputFreqHz);

    // Accessors to get/set the CM DOCSIS annex mode.
    //
    // Parameters:
    //      annexMode - the mode to be used.
    //
    // Returns:
    //      The mode being used.
    //
    uint8 CmAnnexMode(void) const;
    void CmAnnexMode(uint8 annexMode);

    // Accessors to get/set the CM DOCSIS annex B channel plan.
    //
    // Parameters:
    //      channelPlan - the mode to be used.
    //
    // Returns:
    //      The mode being used.
    //
    uint8 CmAnnexBChannelPlan(void) const;
    void CmAnnexBChannelPlan(uint8 channelPlan);

    // Accessors to get/set the CM equalizer threshold.
    //
    // Parameters:
    //      threshold - the value to be used.
    //
    // Returns:
    //      The value being used.
    //
    float CmEqualizerThreshold(void) const;
    void CmEqualizerThreshold(float threshold);

    // Accessors to get/set the spreader scale value.
    //
    // Parameters:
    //      value - the value to be used.
    //
    // Returns:
    //      The value being used.
    //
    uint8 SpreaderScale(void) const;
    void SpreaderScale(uint8 value);

    // Accessors to get/set the upstream sample frequency value.
    //
    // This was added for PR3576.
    //
    // Parameters:
    //      value - the value to be used.
    //
    // Returns:
    //      The value being used.
    //
    uint32 CmUsSampleFreqHz(void) const;
    void CmUsSampleFreqHz(uint32 value);

    // Accessors to get/set the 1024QAM enable flag.
    //
    // This was added for PR3861.
    //
    // Parameters:
    //      enable - true to enable, false to disable.
    //
    // Returns:
    //      true if enabled
    //      false if disabled
    //
    bool Cm1024QamEnable(void) const;
    void Cm1024QamEnable(bool enable);

    // Accessors to get/set the Advance Map RunAhead enable flag.
    //
    // This was added for PR7057.
    //
    // Parameters:
    //      enable - true to enable, false to disable.
    //
    // Returns:
    //      true if enabled
    //      false if disabled
    //
    bool CmAdvMapRunAheadEnable(void) const;
    void CmAdvMapRunAheadEnable(bool enable);

    // Accessors to get/set the DOCSIS 2.0 clipping enable flag.
    //
    // This was added for PR3861.
    //
    // Parameters:
    //      enable - true to enable, false to disable.
    //
    // Returns:
    //      true if enabled
    //      false if disabled
    //
    bool CmUseDocsis20Clipping(void) const;
    void CmUseDocsis20Clipping(bool enable);

    // Accessors to get/set the proprietary SCDMA Code Matrix enable flag.
    //
    // This was added for PR5446.
    //
    // Parameters:
    //      enable - true to enable, false to disable.
    //
    // Returns:
    //      true if enabled
    //      false if disabled
    //
    bool CmProprietaryScdmaCodeMatrixEnable(void) const;
    void CmProprietaryScdmaCodeMatrixEnable(bool enable);

    // Accessors for the CM HAL AGC Configuration nonvol values.  These
    // obscure names match some wacky registers in the h/w; if you don't know
    // what they mean, then you probably have no business reading or writing
    // new values anyway...
    //
    // This was added for PR4111 (which seems to have been lost in GNATs).
    //
    // Parameters:
    //      value - the value to be stored.
    //
    // Returns:
    //      The current value.
    //
    uint8 CmDsAgi(void) const;
    void CmDsAgi(uint8 value);

    uint8 CmDsAgt(void) const;
    void CmDsAgt(uint8 value);

    uint16 CmStathr(void) const;
    void CmStathr(uint16 value);

    uint32 CmStagi(void) const;
    void CmStagi(uint32 value);

    uint32 CmStpga1(void) const;
    void CmStpga1(uint32 value);

    uint32 CmStagt(void) const;
    void CmStagt(uint32 value);

    uint16 CmStabw1(void) const;
    void CmStabw1(uint16 value);

    uint16 CmStabw2(void) const;
    void CmStabw2(uint16 value);

    // Accessors for the CM HAL "short acquisition tries" nonvol (SHACK).  It
    // controls the number of times we will attempt to do the short
    // reacquisition thing when downstream interruption is detected.
    //
    // Note that 0 is a valid value, which means "don't run SHACK at all".
    //
    // This was added for PR5319.
    //
    // Parameters:
    //      value - the value to be stored.
    //
    // Returns:
    //      The current value.
    //
    uint8 CmNumberOfShackTries(void) const;
    void CmNumberOfShackTries(uint8 value);
    
    // Accessor for power save mode (PR 8114).
    //
    // Parameters:
    //      value - the value to be stored.
    //
    // Returns:
    //      The current value.
    // 
    bool PowerSaveModeEnabled(void) const;
    void PowerSaveModeEnable(bool enable);
    
    // Accessor for 3420 frequency map (PR 8220).
    //
    // Parameters:
    //      value - the value to be stored.
    //
    // Returns:
    //      The current value.
    // 
    bool Optimized3420FreqMapEnabled(void) const;
    void Optimized3420FreqMapEnabled(bool enable);
    
    // Accessor for high output PA (PR 8814).
    //
    // Parameters:
    //      value - the value to be stored.
    //
    // Returns:
    //      The current value.
    // 
    bool HighOutputPAEnable(void) const;
    void HighOutputPAEnable(bool enable);

    // Accessors to get/set FPM configured flag.
    //
    // Parameters:
    //      enable - true to enable, false to disable.
    //
    // Returns:
    //      true if enabled
    //      false if disabled
    //
    bool CmFpmConfigure(void) const;
    void CmFpmConfigure(bool enable);

    // Accessors to get/set FPM auto free flag.
    //
    // Parameters:
    //      enable - true to enable, false to disable.
    //
    // Returns:
    //      true if enabled
    //      false if disabled
    //
    bool CmFpmAutoFreeEnable(void) const;
    void CmFpmAutoFreeEnable(bool enable);

    // Accessors to get/set FPM Rx enable flag.
    //
    // Parameters:
    //      enable - true to enable, false to disable.
    //
    // Returns:
    //      true if enabled
    //      false if disabled
    //
    bool CmFpmRxEnable(void) const;
    void CmFpmRxEnable(bool enable);

    // Accessors to get/set Docsis Upstream Priority Queue Support enable flag.
    //
    // Parameters:
    //      enable - true to enable, false to disable.
    //
    // Returns:
    //      true if enabled
    //      false if disabled
    //
    bool CmUpstreamPriorityQueueSupportEnable(void) const;
    void CmUpstreamPriorityQueueSupportEnable(bool enable);

public:

    // Accessors to get/set the watchdog enable state.
    //
    // Parameters:
    //      enable - true to enable the watchdog, false to disable.
    //
    // Returns:
    //      The enable state.
    //
    bool WatchdogEnable(void) const;
    void WatchdogEnable(bool enable);

    // Accessors to get/set the watchdog timeout (the length of time that the
    // watchdog can run without being kicked before it reboots the hardware).
    //
    // Parameters:
    //      timeoutSec - the watchdog timeout (in seconds).
    //
    // Returns:
    //      The watchdog timeout (in seconds).
    //
    uint8 WatchdogTimeout(void) const;
    void WatchdogTimeout(uint8 timeoutSec);

    // Accessors to get/set the Bluetooth Mastering state.
    //
    // Parameters:
    //      enable - true to set CM as Bluetooth Master, false to disable.
    //
    // Returns:
    //      The enable state.
    //
    bool BtMasterEnable(void) const;
    void BtMasterEnable(bool enable);

public:

    // These represent the different BcmAlloc buffer sizes that can be
    // configured.  This was added for PR1930.
    typedef enum
    {
        kBcmAllocSize16 = 0,
        kBcmAllocSize32,
        kBcmAllocSize64,
        kBcmAllocSize128,
        kBcmAllocSize256,
        kBcmAllocSize512,
        kBcmAllocSize1024,
        kBcmAllocSize1600,
        kBcmAllocSize2048,

        // This must be the last one.  This list isn't really dynamic; BcmAlloc
        // only supports a fixed set of buffer sizes, but I hate hard coding
        // numbers for array sizes.
        kNumBcmAllocBufferGroups

    } BcmAllocBufferSizeEnum;

    /* These are the old BcmAlloc methods; they are deprecated.
    
    // Accessors to get/set the number of buffers that will be created by
    // BcmAlloc for the specified buffer size.
    //
    // Parameters:
    //      bufferSize - the buffer size to be configured, from the enum above.
    //      numberOfBuffers - the number of buffers to be allocated.
    //
    // Returns:
    //      The number of buffers to be allocated.
    //
    uint16 BcmAllocNumberOfBuffers(BcmAllocBufferSize bufferSize) const;
    void BcmAllocNumberOfBuffers(BcmAllocBufferSize bufferSize,
                                 uint16 numberOfBuffers);

    */

public:

    // Accessors for the number of BcmAlloc buffer sizes.  This determines
    // the size of the buffer size/number of buffer arrays.  For example, if
    // you want buffers of 16 bytes, 64 bytes, 256 bytes, and 1024 bytes, then
    // this value should be set to 4, since there are 4 different buffer sizes.
    //
    // Note that if you reduce this value (i.e. change it from 8 to 4), the
    // values from the end of the array will be lost.  If you increase this
    // value, the new slots will be filled with increasing buffer sizes, and the
    // number of buffers slots will be filled with 0's.
    //
    // Parameters:
    //      numberOfBufferSizes - the new value to use.  This must be less than
    //                            or equal to kMaxBcmAllocSizes.
    //
    // Returns:
    //      The number of buffer sizes that are configured.
    //
    uint8 BcmAllocNumberOfBufferSizes(void) const;
    void BcmAllocNumberOfBufferSizes(uint8 numberOfBufferSizes);

    // Accessors for individual buffer size and number of buffers entries.
    //
    // Parameters:
    //      offset - the offset into the array to read/write.  This must be
    //               less than the number of buffer sizes value.
    //      value - the value to be written.
    //
    // Returns:
    //      The value at the specified offset.
    //
    uint16 BcmAllocBufferSize(uint8 offset) const;
    uint16 BcmAllocNumberOfBuffers(uint8 offset) const;
    void BcmAllocBufferSize(uint8 offset, uint16 value);
    void BcmAllocNumberOfBuffers(uint8 offset, uint16 value);

    // Accessors for the arrays containing the buffer sizes and the number of
    // buffers of each size.  These are formatted for use when configuring the
    // BcmAlloc module.
    //
    // Parameters:  None.
    //
    // Returns:
    //      A pointer to the array of buffer size/number of buffers values.
    //
    const uint16 *BcmAllocBufferSizes(void) const;
    const uint16 *BcmAllocBuffersNumbers(void) const;

protected:

    // Here's where I write my members to the buffer in a flat format, in the
    // order described at the top of this file.  All multibyte values are
    // written in network byte order.
    //
    // Parameters:
    //      outputBuffer - the buffer into which the settings should be written.
    //      section - the section that should be written.
    //
    // Returns:
    //      true if successful.
    //      false if there was a problem (usually the buffer is too small).
    //
    virtual bool WriteToImpl(BcmOctetBuffer &outputBuffer, NonVolSection section) const;

    // Here's where I read my members from the buffer in a flat format, in the
    // order described at the top of this file.  All multibyte values are read
    // in network byte order.
    //
    // The currentOffset parameter tells the derived class where the app data
    // starts in the buffer.  It can be used directly in the calls to Read() or
    // NetworkRead() in BcmOctetBuffer (which expects an offset value).
    //
    // Parameters:
    //      inputBuffer - the buffer from which the settings should be read.
    //      currentOffset - the offset into the buffer from which the settings
    //                      should be read.  This is necessary to allow the
    //                      derived class to skip the header.
    //      section - the section that should be read.
    //
    // Returns:
    //      true if successful.
    //      false if there was a problem (version number not supported, not
    //          enough bytes in the buffer, etc.).
    //
    virtual bool ReadFromImpl(const BcmOctetBuffer &inputBuffer, 
                              unsigned int &currentOffset, NonVolSection section);

    // Causes me to do formatted output of my settings to the specified ostream,
    // after the base class does its stuff.
    //
    // Parameters:
    //      outputStream - the ostream to be used.
    //
    // Returns:
    //      The reference to the ostream that was passed in.
    //
    virtual ostream & PrintImpl(ostream &outputStream) const;

private:

    bool fConstructorIsRunning;

    // NOTE:  These are all read/written as part of the Permanent Group.  There
    //        are no settings that belong in the Dynamic Group.

    // Tells whether or not the specified interface should be created/installed.
    // This is a bitmask, defined by the enum above.
    uint32 fInterfacesEnabled;

    // Each internal IP stack needs to have its own unique MAC address; since we
    // support up to 8 IP stacks, we need 8 MAC addresses.
    BcmMacAddress fMacAddressIp[8];

    // Controls debug printf output in the HAL.
    uint16 fDefaultHalDebugZones;

    // Helps the drivers to tell what hardware is present, as well as other
    // board layout things.
    uint32 fBoardRev;

    // Settings for the USB HAL.
    uint16 fUsbVendorId;
    uint16 fUsbDeviceId;
    BcmMacAddress fUsbMacAddress;
    bool fUsbRndisDriverEnabled;

    // Settings for the Ethernet HAL.  This tells whether it should try to
    // detect and auto-negotiate the link settings or not.  If not, then we
    // have to specify the link speed and duplex ourselves.
    enum
    {
        kEthernetAutoNegotiateEnable = 0x01,
        kEthernetLinkIsFullDuplex =    0x02,

        // This is for PR1332.
        //
        // Note that this is a disable bit.  It was previously not used (0), and
        // we want the default to be enabled, so we configure this bit such that
        // 0 means the feature is enabled, and 1 means it is disabled.
        kEthernetRejectCamDisable =    0x04,

        // added FPM features (for supported HW)
        kEthernetFpmConfigure = 0x08,
        kEthernetFpmAutoFreeEnable = 0x10,
        kEthernetFpmRxEnable = 0x20,

    };

    uint8 fEthernetFlags;
    uint16 fLinkSpeedMbps;

    // Settings for the HPNA HAL.
    uint32 fHpnaMessageLevel;



    // Settings for the CM HAL.
    uint8 fDsTunerType;
    uint8 fUsAmpType;
    uint32 fDsReferenceFreqHz;
    uint32 fUsReferenceFreqHz;
    uint32 fPhyInputFreqHz;
    uint8 fAnnexMode;
    uint8 fAnnexBChannelPlan;
    float fEqualizerThreshold;      // Added with version 0.4
    uint32 fUsSampleFreqHz;         // PR3576 - added with version 0.8

    // More CM HAL settings; added for PR4111 (which seems to have been lost
    // in GNATs).  These all deal with CM AGC Configuration stuff.
    //
    // These were all added with version 0.10
    //
    // These obscure names match some wacky registers in the h/w; if you don't
    // know what they mean, then you probably have no business reading or
    // writing new values anyway...
    //
    uint8 fDsAgi;           // IF AGC Control
    uint8 fDsAgt;           // RF (Tuner) AGC Control
    uint16 fStathr;         // AGC Loop Threshold
    uint32 fStagi;          // IF AGC Gain Threshold
    uint32 fStpga1;         // IF PGA Control
    uint32 fStagt;          // RF (Tuner) AGC Gain Threshold
    uint16 fStabw1;         // AGC Bandwidth Control
    uint16 fStabw2;         // AGC Bandwidth Control



    // Settings for the board HAL.
    enum
    {
        kBoardHalWatchdogEnable                 = 0x01,
        kBoardHalBtMasterEnable                 = 0x02,
    };

    uint8 fBoardHalFlags;
    uint8 fWatchdogTimeoutSecs;

    // Added with version 0.5, for PR1930; they allow the number of buffer for
    // each allocation size to be configured.  We allow each buffer size to be
    // configured separately.
    //
    // NOTE:  This is deprecated; we now allow the more things to be configured.
    //        See the fBcmAlloc* variables below.  This array MUST remain in
    //        the code in order to preserve the nonvol footprint, and to allow
    //        older code to have reasonable values to use (if the code is
    //        downgraded).
    uint16 fNumBcmAllocBuffers[kNumBcmAllocBufferGroups];

    // Added with version 0.7 for Ray Whitehead.  This has something to do with
    // SCDMA operation.  He says the value must only be 1..8.
    uint8 fSpreaderScale;

    // PR3861 - Added with version 0.9 for KevC.  There are new test modes
    // (enable/disable) for the CM HAL.
    enum
    {
        kAllow1024Qam           = 0x01,
        kUseDocsis20Clipping    = 0x02,

        // PR5446 - added this bit for Joel/Daniel.
        kEnableProprietaryScdmaCodeMatrix = 0x04,

        // PR7057
        kDisableAdvanceMapRunAhead = 0x08,

        // added FPM features (for supported HW)
        kCmFpmConfigure = 0x10,
        kCmFpmAutoFreeEnable = 0x20,
        kCmFpmRxEnable = 0x40,

        // added for DocsisCmHalIf/CmHal upstream priority queuing feature
        kDisableUpstreamEnablePriorityQueueSupport = 0x80,

    };

    uint8 fCmFlags;

    // Added with version 0.13 for Ray and Margo (PR5319); this is the number
    // of times we try the short acquisition (SHACK) thing to recover from a
    // downstream interruption.
    uint8 fNumberOfShackTries;
    
    // PR 8114: enable / disable power save mode.
    bool fPowerSaveModeEnabled;
    
    // PR 8220: Use nominal or optimized 3420 frequency map.
    bool fOptimized3420FreqMapEnabled;
    
    // PR 8814: enable / disable high power PA
    bool fHighOutputPAEnable;

public:

    //Blue tooth mac address.
    BcmMacAddress fMacAddressLocalBt;
    BcmMacAddress fMacAddressRemoteBt;

public:

    // Note - this is a fixed constant, and must never change.
    #define kMaxBcmAllocSizes 32

    // These were added with permanent version 0.12.  They handle the new
    // approach to BcmAlloc buffer allocations.
    uint8 fBcmAllocNumberOfBufferSizes;
    uint16 fBcmAllocBufferSizes[kMaxBcmAllocSizes];
    uint16 fBcmAllocNumberOfBuffers[kMaxBcmAllocSizes];

private:

    // This is the singleton instance of this class.
    static BcmHalIfNonVolSettings *pfSingletonInstance;

private:

    // Copy Constructor.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmHalIfNonVolSettings(const BcmHalIfNonVolSettings &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmHalIfNonVolSettings & operator = (const BcmHalIfNonVolSettings &otherInstance);

};


//********************** Inline Method Implementations ***********************


// This allows you to do the following:
//
//    cout << settingsInstance << endl;
//
inline ostream & operator << (ostream &outputStream, const BcmHalIfNonVolSettings &settingsInstance)
{
    return settingsInstance.Print(outputStream);
}


#endif



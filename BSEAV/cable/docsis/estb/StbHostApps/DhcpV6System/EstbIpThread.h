//****************************************************************************
//
// Copyright (c) 2008-2012 Broadcom Corporation
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
//
//  Filename:       EstbIpThread.h
//  Author:         Pinar Taskiran
//  Creation Date:  March 10, 2008
//
//****************************************************************************
//  Description:
//      This class is the helper thread for the main DOCSIS Control thread.  It
//      handles the operation of all of the IP state, running the DHCP helper,
//      Time Of Day, and TFTPing the config file.
//
//      Additionally, it has the helper routine to handle software upgrades
//      asynchronously (TFTPing the s/w image, processing it, storing it in
//      flash, etc.).
//
//****************************************************************************

#ifndef ESTBIPTHREAD_H
#define ESTBIPTHREAD_H

//********************** Include Files ***************************************

// My base class.
#include "Thread.h"

#include "IpAddress.h"
#include "BcmString.h"

#include "EventPublisher.h"
#include "CompletionHandlerACT.h"

// PR13544
#include "CmDhcpV6MsgACT.h"
#ifdef STUB
#include "NonVolSettings.h"
#endif

#if (BFC_INCLUDE_DHCPV4_SUPPORT)
#include "DhcpClientLease.h"
#endif
 
#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 3)
#include <utility>
#include <list>
#else
#include "pair.h"
#include "list.h"
#endif

#if (BFC_INCLUDE_DHCPV6_SUPPORT)//(BCM_IPV6_SUPPORT)
#include "DhcpV6ClientIf.h"
#endif
#if (SNMP_SUPPORT)
// PTASKIRAN _ ESTB SNMP AGENT SUPPORT#include "CmSnmpNonVolSettings.h"
#endif
#ifdef STUB
#include "ModemCapabilities.h"
#endif // #ifdef STUB

#include "IpAddressList.h"
//********************** Global Types ****************************************

// Prototype for the 'C' callout function that produces the value to be used
// in the Option43 field of the CM's DHCP packets.  This is called before the
// first Discover is sent, and the value returned will be added to the packets
// that are sent.
//
// This function MUST NOT write the octets for the 43 or the length; this is
// handled by our code.  This function must only produce the data to be
// contained within the option.
//
// Parameters:
//      pOption43Buffer - buffer into which the Option43 octets should be
//                        placed.  The total number of bytes must be no more
//                        than 255 (this is the size of the buffer, and the
//                        max size that can be used in DHCP options).
//      pNumberOfBytes - (out) the number of bytes that were written into the
//                       buffer.  If 0, then Option43 will not be added.
//
// Returns:  Nothing.
//
typedef void (*Option43Callout)(unsigned char *pOption43Buffer,
                                unsigned int *pNumberOfBytes);

// Prototype for the 'C' callout function that allows the implementor of this
// function to look at the header of the image that is being downloaded and to
// decide whether or not it is 'valid'; this evaluation and the criteria, of
// course, will be vendor specific.
//
// This function (if registered) will be called after basic validation has
// been performed (the signature matches, HCS checks out, length is valid,
// etc).  This is the final step in header validation before the download
// commences in earnest.  If this function returns false (0), the download will
// be aborted in the same manner as would happen if the basic validation failed.
//
// Parameters:
//      pBuffer - pointer to the buffer containing the image header data.
//      numberOfBytes - the number of bytes of valid data in the buffer.  The
//                      header will almost certainly be much smaller than this.
//
// Returns:
//      0 (false) if the image is not valid for this platform and must be
//          rejected.  We will log event E107.0 and stop the download.
//      1 (true) if the image is valid for this platform and can be stored.
//
typedef unsigned int (*ValidateImageCallout)(const unsigned char *pBuffer,
                                             unsigned int numberOfBytes);

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

#if (NOT_INCLUDED_IN_ESTB_DHCP) 
class BcmTimeOfDayThread;
class BcmTftp;
#endif

class BcmOctetBuffer;
class BcmIpAddress;
class CmSecureDownload;
class BcmIpAddressList;
class BcmDhcpV6ClientEventDispatcherACT;
//********************** Class Declaration ***********************************


class BcmEstbIpThread : public BcmThread
{
public:

    // Default Constructor.  Starts the thread, and creates all of the objects
    // that it uses.
    //
    // Note that you will generally want to set the thread priority lower than
    // the priority of the Docsis Control/Management thread so that the TFTP
    // process doesn't block it from running.
    //
    // Parameters:
    //      initialPriority - the priority at which you want this thread to run.
    //
    // Returns:  N/A
    //
    BcmEstbIpThread(BcmOperatingSystem::ThreadPriority initialPriority = BcmOperatingSystem::kLowNormalPriority, unsigned int stackNumber=0);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmEstbIpThread();

    // This must be called after the IP stacks have been installed and
    // initialized.  I need to register myself with the stack so I can do DHCP,
    // etc.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    void RegisterWithIpStack(void);

    // Allows clients to subscribe for one or more of the events that I publish.
    //
    // Parameters:
    //      eventCode - the event code for the status event of interest (see
    //                  CmDocsisStatusEventCodes.h).
    //      pAct - pointer to the ACT that is to be called when publishing
    //             an event.  This is how the client is notified that the event
    //             has occurred.
    //
    // Returns:
    //      true if the client was successfully subscribed for the event.
    //      false if the client could not be subscribed for some reason, or
    //          if I don't publish the specified event.
    //
    bool SubscribeEventNote(unsigned int eventCode,
                            BcmCompletionHandlerACT *pAct);

    // Allows clients to unsubscribe from being notified for the event
    // specified.  The parameters MUST be the same as the ones specified in the
    // call to SubscribeEventNote; this pair of parameters is necessary to
    // uniquely identify a client.
    //
    // Parameters:
    //      eventCode - the event code for the status event of interest (see
    //                  CmDocsisStatusEventCodes.h).
    //      pAct - pointer to the ACT that is to be called when publishing
    //             an event.  This is how the client is notified that the event
    //             has occurred.
    //
    // Returns:
    //      true if the client was successfully unsubscribed for the event.
    //      false if the client could not be unsubscribed for some reason.
    //
    bool UnSubscribeEventNote(unsigned int eventCode,
                              BcmCompletionHandlerACT *pAct);

    // Sets the Option43 callout function.
    //
    // Parameters:
    //      callout - the address of the function to call.
    //
    // Returns:  Nothing.
    //
    void SetOption43Callout(Option43Callout callout);

    // Sets the image validation callout function.
    //
    // Parameters:
    //      callout - the address of the function to call.
    //
    // Returns:  Nothing.
    //
    void SetValidateImageCallout(ValidateImageCallout callout);
#ifdef STUB
    // Sets the modem capabilites string that I need to send in DHCP packets
    // as part of the Vendor Class Id option.  The capabilities should be the
    // full TLV, including the parent type (5) and length, plus all sub-TLVs,
    // in binary format.  I will convert them as needed to ASCII-hex data.
    //
    // Parameters:
    //      modemCapabilites - buffer containing the full modem capabilities.
    //
    // Returns:  Nothing.
    //
    void SetCapabilities(const BcmOctetBuffer &modemCapabilities);

    // Sets the modem capabilites object instance that is used when encoding
    // option 43 vendor specific sub-options.
    //
    // Parameters:
    //      modemCapabilites - buffer containing the full modem capabilities.
    //
    // Returns:  Nothing.
    //
    void SetCapabilitiesObject(const BcmModemCapabilities &modemCapabilities);
#endif
    // 
    typedef enum
    {
        kDocsis20IpInitMode,
        kDocsis30IpInitMode
    } DocsisMode;

    // This is what signals the thread to begin IP initialization.  It starts
    // doing DHCP to get its IP settings, then kicks off Time Of Day for the
    // ToD server specified in the DHCP Bind, then tries to TFTP the config
    // file specified in the DHCP Bind into the buffer that is specified.
    //
    // Clients will be notified when DHCP/ToD/TFTP start and finish (or stop
    // due to errors) via the Publish/Subscribe interface.
    //
    // Parameters:
    //      pConfigFileBuffer - the buffer into which you want the config file
    //                          to be stored.  It should be large enough to hold
    //                          the entire config file; if it is too small, then
    //                          this will cause a failure to be reported.  This
    //                          can be NULL if you don't want the config file.
    // Returns:
    //      true if the IP Initialization process was started.
    //      false if there was a problem (thread not running, invalid state,
    //          etc).
    //
    bool StartIpInit(BcmOctetBuffer *pConfigFileBuffer,
                     bool preferredIpVersionIsIPv6,
                     bool dualStackOperationEnabled,
                     bool alternameIpManagementModeEnabled,
                     DocsisMode docsisMode = kDocsis30IpInitMode );

    // This allows the client to restart TFTP of the config file.  It is
    // possible for the config file to be loaded successfully, but for the
    // client not to like what's in the file, in which case we need to go
    // through the whole "wait a while and retry" process, i.e. treat it the
    // same as if the file doesn't exist.
    //
    // Parameters:
    //      pConfigFileBuffer - the buffer into which you want the config file
    //                          to be stored.  It should be large enough to hold
    //                          the entire config file; if it is too small, then
    //                          this will cause a failure to be reported.  This
    //                          can be NULL if you don't want the config file.
    //                          Of course, if you don't want the config file,
    //                          then why are you calling this method?
    //
    // Returns:
    //      true if the TFTP read was restarted again.
    //      false if there was a problem (thread not running, invalid state,
    //          etc).
    //
    bool RestartTftpConfig(BcmOctetBuffer *pConfigFileBuffer);

    // This allows the client to abort any operation that is currently running
    // (DHCP, ToD, TFTP config file, s/w dload) before it has completed, and
    // will reset the state to an initial state (clearing the IP address, etc).
    // This is often used when a T4 timeout is detected or when we deregister
    // from the CMTS.
    //
    // This operates synchronously; i.e. the thread will stop operation and
    // reset state before this method returns.
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if the thread was stopped and reset successfully.
    //      false if it wasn't running, or if it couldn't be stopped for some
    //          reason.
    //
    bool StopAndReset(void);

    // These are the states that the IP init process goes through.  Note that
    // the ToD state is not reflected here (it operates asynchronously).
    typedef enum
    {
        // We haven't done DHCP, and haven't gotten the config file.
        kIpNotStarted,

        // DHCP is in progress.  This includes all of the stages of DHCP.
        kIpDhcpInProgress,

        // DHCP completed successfully, ToD has been started, and I'm trying to
        // TFTP the config file.
        kIpTftpInProgress,

        // DHCP and TFTP completed successfully.  Clients have been notified,
        // and the octet buffer contains the config file.
        kIpSuccess,

        // DHCP failed (no OFFERs received, OFFERs/BINDs were not acceptable,
        // couldn't store IP settings, etc.).
        kIpFailedDhcp,

        // DHCP completed, but TFTP failed (no response from TFTP server, file
        // not found, buffer too small to hold config file, etc.).
        kIpFailedTftpConfig,

        // Indicates that UpdateSoftware() has been called.  Call
        // GetSwUpdateState() to query the status of the update.
        kIpDoingSwUpdate

    } IpState;

    typedef enum
    {
        // We haven't done anything yet. 
        kIpV6NotStarted,

        // Router advertisement collection in progress
        kRouterAdvertisementCollectionInProgress,

        // Duplicate address detection of link-local address is in progress
        kDuplicateAddressDetectionOfLinkLocalAddressInProgress,

        // DHCPv6 is in progress.  This includes all of the stages of DHCPv6.
        kIpDhcpV6InProgress,

        // Duplicate address detection of acquired DHCPv6 address is in progress
        kDuplicateAddressDetectionOfDhcpAddressInProgress,

        // DHCP and TFTP completed successfully.  Clients have been notified,
        // and the octet buffer contains the config file.
        kIpV6Success,

        // DHCPv6 failed (no OFFERs received, OFFERs/BINDs were not acceptable,
        // couldn't store IP settings, etc.).
        kFailedDhcpV6

    } IpV6State;


    // Allows the client to retrieve the current internal state of the thread as
    // it performs IP initialization.
    //
    // Parameters:  None.
    //
    // Returns:
    //      The current state.
    //
    inline IpState GetIpState(void);
    inline IpV6State GetIpV6State(void) const;

    // IsDhcpBound() - Returns true if DHCP initialization has reached "bound"
    // state.
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if the DHCP client has acquired an IP address.
    //      false if the DHCP client has not acquired an IP address.
    //
    bool IsDhcpBound(void) const;
#if (BFC_INCLUDE_DHCPV4_SUPPORT)
    // Helper method to get the DHCP lease object for the stack that we are
    // controlling.
    //
    // Parameters:  None.
    //
    // Returns:
    //      A pointer to the lease object (or NULL if not found).
    //
    BcmDhcpClientLease *GetLease(void) const;
#endif
    // Helper to return the current IP address of our stack.
    //
    // Parameters:  None.
    //
    // Returns:
    //      The current IP address.
    //
    const BcmIpAddress &CurrentIpAddress(void) const;

    // Helper to return the (config file, not dload) TFTP server IP address.
    //
    // Parameters:  None.
    //
    // Returns:
    //      The (config file, not dload) TFTP server IP address.
    //
    const BcmIpAddress &CurrentTftpServerAddress(void) const;
    
    // Helper to return the (config file, not dload) TFTP filename.
    //
    // Parameters:  None.
    //
    // Returns:
    //      The (config file, not dload) TFTP filename.
    //
    const BcmString &CurrentTftpServerFilename(void) const;

    // Allows the client to query whether or not Time Of Day has completed
    // successfully.
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if ToD completed.
    //      false if ToD has not completed (may not even has been started).
    //
    inline bool TimeOfDayComplete(void);
    
    // Allows the client to query the time offset from the TOD exchange.
    //
    // Parameters:  None.
    //
    // Returns:
    //      The system clock offset induced by the last ToD exchange.
    //      Could be positive, negative, or zero.
    //
    int GetTodLastDelta(void);
    
	#if (NOT_INCLUDED_IN_ESTB_DHCP) 
    // Accessor for the TOD thread
    //
    // Parameters:  None.
    //
    // Returns:
    //      The TOD helper thread.  Could be NULL.
    //
    inline BcmTimeOfDayThread *GetTodThread (void);
	#endif
    // This method checks the name of the existing software image and compares
    // it to the one specified, returning false if the names are the different.
    // In some cases we don't want to begin a software update if the existing
    // image is the same as what is being requested.
    //
    // Parameters:
    //      pFilename - the name of the software image that would be updated.
    //
    // Returns:
    //      true if the filenames are the same.
    //      false if the filenames are different.
    //
    bool SoftwareImagesAreTheSame(const char *pFilename);

    // Begins the process of downloading a new software image for the CM.  This
    // is done using TFTP from the specified server, and is performed
    // asynchronously.
    //
    // This can only be called in the IpState is kIpSuccess, since we must have
    // our IP parameters in order to run.
    //
    // Parameters:
    //      pFilename - a string containing the name of the file to TFTP.  This
    //                  will usually come from the config file, but it can also
    //                  come from other sources (user input, etc.).
    //      ipAddress - the IP address of the TFTP server.  This will also
    //                  typically come from DHCP or the config file, but can
    //                  come from other sources.
    //      startedByManagement - tells me who started the update, management
    //                            (SNMP/console) or provisioning (config file).
    //      imageNumber - the image number to be updated.  0 = default.
    //      secureDownload - whether download is Secure or not
    //
    // Returns:
    //      true if the software update was started successfully.
    //      false if there was a problem (thread not running, invalid state,
    //          invalid parameters, etc.).
    //
    virtual bool StartSoftwareUpdate(const char *pFilename,
                                     const BcmIpAddress &ipAddress,
                                     bool startedByManagement,
                                     unsigned int imageNumber = 0,
                                     bool secureDownload = true);

    bool StopSoftwareUpdate(void);

    // These are the states that the software update process goes through.
    typedef enum
    {
        // Software update hasn't been started.
        kSwNotStarted,

        // Software update is running.
        kSwInProgress,

        // Software update has completed successfully.
        kSwSuccess,

        // Software update failed (file not found, bad software image, image
        // too big to store, etc.).
        kSwFailed,

        // Somebody called AbortSoftwareUpdate().
        kSwAbortedByClient

    } SwUpdateState;

    // Allows the client to retrieve the current internal state of the thread.
    //
    // Parameters:  None.
    //
    // Returns:
    //      The current state.
    //
    inline SwUpdateState GetSwUpdateState(void);

    // Allows the client to query whether or not Secure Download is enabled .
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if enabled
    //      false if not
    //
    inline bool SecureDownloadEnabled(void);

#if (EMTA_SUPPORT)
    // Sets the DHCP option number to use for PacketCable servers
    //
    // Parameters:  uint8 number
    //
    // Returns:     None
    //
    bool SetPacketCableDhcpOptionNumber( uint8 packetCableDhcpOptionNumber );
#endif

    // Allows a client to download a NonVol Settings binary image to be stored
    // in the specified object.  This supports both full section dload
    // (Permanent or Dynamic), as well as just dloading a group (BPI Permanent).
    //
    // This is mainly designed to support manufacturing where the nonvol
    // settings for the system are created by a host-based tool and then loaded
    // into the modem via ethernet, which is triggered by SNMP.
    //
    // This is for PR1815.
    //
    // Parameters:
    //      pFilename - the name of the nonvol image file to load.
    //      ipAddress - the IP address of the TFTP server.
    //      section - the nonvol section (Dynamic or Permanent) that is being
    //                read and stored.
    //      pSettings - if NULL, then it is assumed that all of nonvol is being
    //                  read and stored; if not NULL, then this must point to
    //                  the settings object (group) that is being read and
    //                  stored.
    //
    // Returns:
    //      true if the nonvol download was started successfully.
    //      false if there was a problem (thread not running, invalid state,
    //          invalid parameters, etc).
    //
/*
    bool StartNonVolDownload(const char *pFilename,
                             const BcmIpAddress &ipAddress,
                             BcmNonVolSettings::NonVolSection section,
                             BcmNonVolSettings *pSettings = NULL);
*/

	// eSafe service impact for SW download reset delay
    typedef enum
    {
       noUpdate = 0,
       significant,
       none
    };

// This is called only by EMTA IF to tell us of active call and SW download delay timers.
    //
    // Parameters:  impact: significant, none
    //              uint32 MaxResetDelay
    //              uint32 PostCallResetDelay
    //
    // Returns:     None
    //
    void UpdateSWDownloadResetDelayParams(int impact, uint32 MaxResetDelay, uint32 PostCallResetDelay );

public:

    // Called by the DHCP Client Event ACT to let me process events.
    //
    // Parameters:
    //      eventCode - the event that occurred.
    //
    // Returns:  Nothing.
    //
    void ProcessDhcpEvent(const BcmCompletionEvent &eventCode);

    // Called by the DHCPv6 Client Event ACT to let me process events.
    //
    // Parameters:
    //      eventCode - the event that occurred.
    //
    // Returns:  Nothing.
    //
    void ProcessDhcpV6Event(const BcmCompletionEvent &eventCode);

    // Called by the Neighbor Discovery ACT to let me process events.
    //
    // Parameters:
    //      eventCode - the event that occurred.
    //
    // Returns:  Nothing.
    //
    void ProcessNeighborDiscoveryEvent(const BcmCompletionEvent &eventCode);

#if (BFC_INCLUDE_DHCPV6_SUPPORT)
    // This is the callout function that is registered with the DHCPv6 Client
    // When an advertise/reply packet is received, the EMTA will inspect it.
    // If everything checks out then we will return true so the lease can be accepted
    //
    //
    // \param
    //      dhcpPacket - the recieved DHCPv6 packet.
    // \param
    //      SolicitNumTries - the number of Solicit messages this DHCPv6 Client has sent
    //
    // \retval
    //      true if the advertise/reply is acceptable.
    // \retval
    //      false if the advertise/reply is not acceptable, and should not be selected or
    //          should be NAKed.
    //
    static bool CmDhcpV6AdvertiseReplyCallout(const BcmDhcpV6Packet &dhcpPacket, unsigned int solicitNumTries);

	BcmIpAddressList & DnsServerList(void);
	bool DomainListIsPresent(BcmOctetBuffer& newDomainName);
	bool DomainListAdd(BcmOctetBuffer& newDomainName);
	bool DomainListRemove(BcmOctetBuffer& domainName);
	void This( BcmEstbIpThread* pThis);
#endif

	#if !(BCM_EVD_RPCCL_INTERFACE_INCLUDED)
	void SubscribeToEvents(void);
	#endif
	void DhcpV6LogEvent(uint32 Id);

protected:

    // Thread constructor - this is the first method called after the thread has
    // been spawned, and is where the thread should create all OS objects.  This
    // has to be done here, rather than in the object's constructor, because
    // some OS objects must be created in the context of the thread that will
    // use them.  The object's constructor is still running in the context of
    // the thread that created this object.
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if successful and ThreadMain() should be called.
    //      false if there was a problem (couldn't create an OS object, etc.)
    //          and ThreadMain() should not be called.
    //
    virtual bool Initialize(void);

    // This is the main body of the thread's code.  This is the only method
    // that absolutely must be provided in the derived class (since there is no
    // reasonable default behavior).
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    virtual void ThreadMain(void);

    // Thread destructor - this is the last method called when the thread is
    // exiting, and is where the thread should delete all of the OS objects that
    // it created.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    virtual void Deinitialize(void);

	// Reset IP state to it's initial value. This would normally be
	// kIpSuccess, for applications other than docsis it may be something
	// else.
	//
	// Parameters:  None.
	//
	// Returns:  Nothing.
	//
	virtual void RestoreApplicationSpecificIpState(void);
#if (SNMP_SUPPORT)
	// Reset docsDevSwAdminStatus to it's initial value. This would normally be
	// CONSTVAL_DOCSDEVSWADMINSTATUS_ALLOWPROVISIONINGUPGRADE, for applications
	// other than docsis it may be something else.
	//
	// Parameters:  None.
	//
	// Returns:  Nothing.
	//
	// PTASKIRAN _ ESTB SNMP AGENT SUPPORT virtual void RestoreApplicationSpecificSwAdminStatus(CmSnmpNonVolSettings *pSnmpSettings);
#endif
	// Do application specific program store driver configuration.
	//
	// Parameters:  None.
	//
	// Returns:  Nothing.
	//
	virtual void ApplicationSpecificProgramStoreDriverSettings(void);

	// Do application specific tftp configuration.
	//
	// Parameters:  None.
	//
	// Returns:  Nothing.
	//
	virtual void ApplicationSpecificTftpInitialization(void);

	// Append ESAFE name to option 43 suboption 3 if DSG is enabled.
	//
	// Parameters:  Sub-option octet buffer.
	//
	// Returns:  Nothing.
	//
	virtual void AppendApplicationSpecificEsafe(BcmOctetBuffer &subOpt3);

	// Append eSAFE-specific sub options to option 43 buffer.
	//
	// Parameters:  Option43 octet buffer.
	//
	// Returns:  Nothing.
	//
	virtual void AppendApplicationSpecificOption43SubOptions(BcmOctetBuffer &opt43);

	static BcmEstbIpThread *GetSingletonInstance(void);

protected:

    // Causes the specified event to be published (within the context of the
    // mutex semaphore).
    //
    // Parameters:
    //      event - the event to be published.
    //
    // Returns:  Nothing.
    //
    void PublishEvent(unsigned int event, uint32 value = 0);

private:
    
#if (BCM_IPV6_NEIGHBOR_DISCOVERY_SUPPORT)
    // Initiate DAD on the link-local address 
    //
    // Parameters:
    //      none
    //
    // Returns:  Nothing.
    //
    void InitiateDuplicateAddressDetectionOnLinkLocalAddress();

    // Result of link-local address DAD
    //
    // Parameters:
    //      none
    //
    // Returns:  Nothing.
    //
    void DuplicateAddressDetectionOnLinkLocalAddressComplete( bool duplicateDetected );
#endif

public:
#if (BFC_INCLUDE_DHCPV6_SUPPORT)
    // Obtain router advertisement
    //
    // Parameters:
    //      none
    //
    // Returns:  Nothing.
    //
    void ObtainRouterAdvertisement( void );

    // IPv6 IP address acquisition failed at one of a number of possible points
    //
    // Parameters:
    //      none
    //
    // Returns:  Nothing.
    //
    void IpV6AddressAcquisitionFailed();

    // Kicks off DHCPv6.  Takes nonvol settings into account.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    void StartDhcpV6(void);

    // Kicks off DHCPv6 after DHCPv4 has already taken CM in service. 
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    void StartDhcpV6AsNonPreferredIpVersion(void);

    // Handles the specified DHCPv6 event (sent to us by the DHCPv6 client
    // via our ACT).
    //
    // Parameters:
    //      event - the event that occurred.
    //
    // Returns:  Nothing.
    //
    void HandleDhcpV6Event(BcmDhcpV6ClientIf::DhcpV6ClientLeaseEvent event);

#endif

    // Kicks off DHCP.  Takes nonvol settings into account.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    void StartDhcp(void);

    // Kicks off DHCP after DHCPv6 has completed and CM is already operational. 
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    void StartDhcpAsNonPreferredIpVersion(void);
#if (BFC_INCLUDE_DHCPV4_SUPPORT)
    // Handles the specified DHCP event (sent to us by the DHCP lease object
    // via our ACT).
    //
    // Parameters:
    //      pLease - pointer to our lease
    //
    // Returns:  Nothing.
    //
    void SetupAndInitiateDhcpV4(BcmDhcpClientLease *pLease);

    // Handles the specified DHCP event (sent to us by the DHCP lease object
    // via our ACT).
    //
    // Parameters:
    //      event - the event that occurred.
    //
    // Returns:  Nothing.
    //
    void HandleDhcpEvent(BcmDhcpClientLease::DhcpClientLeaseEvent event);
#endif // #if (BFC_INCLUDE_DHCPV4_SUPPORT)
    // Kicks off the Time Of Day thread.  The ToD parameters are pulled from
    // the DHCP settings.
    //
    // Parameters:  
    //
    //      fakeTod - indicates we should fake out TOD
    //      todServerIp - either an IPv4 or IPv6 address of the time server
    //      timeOffset - time offset of TOD obtained from DHCP
    //
    // Returns:  Nothing.
    //
    void StartTod( bool fakeTod, BcmIpAddressList & timeServers, int32 timeOffset );

    // Handles the completion of ToD, checking the status, and doing whatever
    // cleanup is necessary.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    void HandleTodComplete(void);
    
    // Stops the basic IP state machine (not the s/w upgrade state machine),
    // and resets everything to an initial state.
    //
    // Parameters:
    //      releaseSemaphore - true if the semaphore should be released (meaning
    //                         that this is called because the client thread is
    //                         blocked); false if this is being called internally
    //                         and the semaphore should not be released.
    //
    // Returns:  Nothing.
    //
    void StopAndResetSync(bool releaseSemaphore);

    // Does the TFTP of the config file.  The parameters are pulled from the
    // DHCP settings.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    void TftpConfigFile();

    // Sets up the config file name, TFTP servers list, and backoff/retry settings.  
    // This is based on the Docsis version (Docsis 2.0 or Docsis3.0), the 
    // DHCPv4 or DHCPv6 lease settings, and possibly the the non-vol settings
    // for test modes like "force config file".
    //
    // Parameters:  none
    //
    // Returns:  none
    //
    bool SetupIPv4ConfigFileTftpParameters();

    // Sets up the config file name, TFTP servers list, and backoff/retry settings.  
    // This is based on the Docsis version (Docsis 2.0 or Docsis3.0), the 
    // DHCPv4 or DHCPv6 lease settings, and possibly the the non-vol settings
    // for test modes like "force config file".
    //
    // Parameters:  none
    //
    // Returns:  none
    //
    #if (BCM_IPV6_SUPPORT)
    bool SetupIPv6ConfigFileTftpParameters();
    #endif

    // Returns the TFTP IPv4 or IPv6 address
    //
    // Parameters:  None.
    //
    // Returns:  IP address of the TFTP server.   
    //           This may be an IPv4, or IPv6 address. 
    //
    BcmIpAddress GetTftpServerIpAddress();

    // Returns the config file name
    //
    // Parameters:  None.
    //
    // Returns:  pointer to config file name
    //
    const char * GetConfigFileName();

    // Does the downloading of a new software image, based on the client
    // parameters that were passed in to StartSoftwareUpdate().
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
//    virtual void TftpSoftwareImage(void);

    // Does the downloading of a new Combined software image, based on the client
    // parameters that were passed in to StartSoftwareUpdate().
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
#if (BCM_IPC_INCLUDED)
    void TftpCombinedSoftwareImage(void);
#endif
    
    // Stops the s/w upgrade process and resets the state machine.  This does
    // not effect the IP state machine.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    void StopTftpSoftwareImage(void);

    // process software download complete: 
    //    Publish kCmSwDownloadOk or start reset Delay timer
    //
    // Parameters:  None
    //
    // Returns:     None
    //
    void HandleSWDownloadComplete( void );

    // process software download complete: 
    //    Called when periodic timer expires, decide if need to restart or take reset action.
    //
    // Parameters:  None
    //
    // Returns:     None
    //
    void HandleMaxDelayTimer( void );

    // process software download complete: 
    //    Called when periodic timer expires, decide if need to restart or take reset action.
    //
    // Parameters:  None
    //
    // Returns:     None
    //
    void HandlePostCallDelayTimer( void );

	void AddCableLabsVendorSpecificInfoOption(BcmDhcpV6OptionsList &clientOptions);

protected:

	// Commands passed via the message queue.
	enum
	{
		kNopCommand = 0,        // Just a placeholder
		kExitThread,
		kStartIpV4Init,
        kStartIpV6Init,
        kObtainRouterAdvertisement,
        kPerformDhcpV6,
        kInitiateDuplicateAddressDetectionOnDhcpAddress,
		kRestartConfigFileTftp,
		kStopAndReset,
		kStartSoftwareDload,
		kStopSoftwareDload,
		kStartNonvolDload,       // PR1815
		kProcessDhcpEvent,
        kProcessDhcpV6Event,
        kStartPostCallTimer,     // PR11436
        kStopPostCallTimer,      // PR11436
	};

	// My internal states.

    IpState fIpState;
    IpV6State fIpV6State;

    SwUpdateState fSwUpdateState;
	bool fThreadIsRunning;

    // Tells the thread to abort its current operation; used only for s/w
    // dload.
    bool fAbortSwUpdate;

private:

    // This is the singleton instance of this class.
    static BcmEstbIpThread *pfSingletonInstance;

    // Tells whether or not ToD has finished.  This is an async process, so we
    // can't reflect this in our normal IpState.
    unsigned int fTodRetryCounter;
    bool fTimeOfDayCompleted;

    // This is given to me in the call to Start().  I will TFTP the config
    // file into this buffer.
    BcmOctetBuffer *pfConfigFileBuffer;
    
    // Config file TFTP settings - applies to both IPv4 and IPv6
    typedef pair< BcmIpAddress, unsigned int > tTftpServerIpAddressBackoffPair;
    list <tTftpServerIpAddressBackoffPair> fTftpServerIpAddressBackoffPairList;
    unsigned int fTftpBlockSize;
    BcmIpAddress fCurrentConfigFileIp;
    BcmOctetBuffer fConfigFileFilename;

    DocsisMode fDocsisMode;

protected:

    // These are the operational parameters of the software update process.
    char *pfSwUpdateFilename;
    BcmIpAddress fSwUpdateIpAddress;
    bool  fSwUpdateStartedByManagement;
    unsigned int fImageNumber;
    bool fSecureDownload;

private:

    // Additional parameter for nonvol downloading.
    //BcmNonVolSettings *pfSettingsToStore;

    // Used by various methods to wait for the thread to abort operation.
    BcmCountingSemaphore *pfWaitForAbortSemaphore;

    // Container for all of my events.
    BcmEventSet *pfEventSet;

protected:

    // This is what I use to tell the thread to do things.  It replaces what
    // used to be accomplished with several events.
    BcmMessageQueue *pfMessageQueue;

private:

    // This is what I pass to the ToD helper to allow it to tell me when it
    // is done.
    BcmEvent *pfTodHelperDoneEvent;

    // Used to tell me when it is Ok to try TFTP again (for either config file
    // or s/w update).
    BcmTimer *pfTftpTimer;

    // Determines when the next TFTP attempt will be.   
    // This differs between Docsis2.0 and Docsis 3.0. 
    unsigned int fTftpRetryTimeoutMs;

    // Used to time the period between router solicitations
    BcmTimer *pfRouterSolicitationTimer;

	#if (NOT_INCLUDED_IN_ESTB_DHCP) 
    // These are my IP helper objects.  I create them using the IP Helper
    // factory class.
    BcmTimeOfDayThread *pfTodThread;
	#endif

    // Used to delay reset after SW download in case of the active phone call.
    // Note: used only if emta present
    BcmTimer *pfSWDnldResetMaxDelayTimer;
    
    // Used to delay reset after termination of a phone call.
    // Note: used only if emta present
    BcmTimer *pfPostCallDelayTimer;

protected:

	#if (NOT_INCLUDED_IN_ESTB_DHCP) 
    BcmTftp *pfTftpHelper;
	#endif

private:

    // The stack I'm running on.
    unsigned int fStackNumber;

    // The modem capabilities that I will send in DHCPv4 packets (converted to
    // ASCII/hex).
    BcmOctetBuffer fModemCapabilities;

    // Same as above, but in binary format for DHCPv6.
    BcmOctetBuffer fModemCapabilitiesBinary;
#ifdef STUB
    // BcmModemCapabilities object instance is used as input for encoding
    // option 43 vendor specific sub-options.
    BcmModemCapabilities fModemCapabilitiesObject;
#endif // 
    // This is how the lease notifies me of events.
    BcmCompletionHandlerACT *pfDhcpAct;

    /// This is how the DHCPv6 client notifies me of events.
    BcmCompletionHandlerACT *pfDhcpV6Act;

    /// PR13544 - the is the second way the DHCPv6 client notifies me of events.
    BcmCmDhcpV6MsgACT *pfDhcpV6PacketAct;

    // Used to provide a publisher-subscriber interface for async CM DOCSIS
    // status event notification.  There are several requirements that have
    // been delegated to this object, so I need to publish them.
    BcmEventPublisher fDocsisStatusEventPublisher;

    // Used to protect access to the event publisher.
    BcmMutexSemaphore *pfEventPublisherMutex;

    // The callout function for handling DHCP Option43.
    Option43Callout fOption43Callout;

    // The DHCP option number to use for PacketCable servers
    uint8 fPacketCableDhcpOptionNumber;
    
    // In case of a Active phone call or Post Call delay timer use this to flag
    // pending SWDownloadOK event
    bool fSWDownloadOKPending;
    
    // Used to activate reset Delay after SW download completion
    bool fMaxResetDelayTimerActive;
    
    // Used to see if Max Reset Delay timer has expired for first time or not
    // After first expiry send an Event, on secont expiry reset the CM.
    bool fMaxDelayTimerFirstExpiry;

    // Used for SW download Post Call Completion Reset Delay
    bool fPostCallResetDelayTimerActive;

    // The SW download reset delay time
    uint32 fMaxResetDelay;
    
    // Post Call reset delay time
    uint32 fPostCallResetDelay;
    
    // Post Call timer pending time
    uint32 fPostCallDelayPendingTime;

    // The eSafe impact state
    uint32 fESafeImpact;

protected:
    // The callout function for handling image validation
    ValidateImageCallout fValidateImageCallout;

private:

    // Docsis 3.0 IP initialization parameters
    bool fPreferredIpVersionIsIPv6;
    bool fDualStackOperationEnabled;
    bool fAlternameIpManagementModeEnabled;

    unsigned int fRouterSolicitationCount;

    // saved IPv6 address assigned when the lease was first acquired
    BcmIpV6Address fIpV6Address;

	BcmIpAddressList fDnsServerList; 
	BcmOctetBuffer *pfDomainList;

	#if (BCM_EVD_INTERFACE_INCLUDED)
	BcmDhcpV6ClientEventDispatcherACT *pfDhcpV6EventDispatcherACT;
	#endif

	uint32 fLastEventIdLogged;

private:

    // Copy Constructor.  Initializes the state of this object to match that
    // of the instance passed in.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmEstbIpThread(const BcmEstbIpThread &otherInstance);

    // Assignment operator.  Copies the state of the instance passed in so that
    // this object is "identical".
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmEstbIpThread & operator = (const BcmEstbIpThread &otherInstance);

};

typedef BcmEstbIpThread::IpState CmIpState;
typedef BcmEstbIpThread::IpV6State CmIpV6State;

//********************** Inline Method Implementations ***********************


// Allows the client to retrieve the current internal state of the thread as
// it performs IP initialization.
//
// Parameters:  None.
//
// Returns:
//      The current state.
//
inline BcmEstbIpThread::IpState BcmEstbIpThread::GetIpState(void)
{
    return fIpState;
}

inline BcmEstbIpThread::IpV6State BcmEstbIpThread::GetIpV6State(void) const
{
    return fIpV6State;
}

// Allows the client to retrieve the current internal state of the thread.
//
// Parameters:  None.
//
// Returns:
//      The current state.
//
inline BcmEstbIpThread::SwUpdateState BcmEstbIpThread::GetSwUpdateState(void)
{
    return fSwUpdateState;
}


// Allows the client to query whether or not Time Of Day has completed
// successfully.
//
// Parameters:  None.
//
// Returns:
//      true if ToD completed.
//      false if ToD has not completed (may not even has been started).
//
inline bool BcmEstbIpThread::TimeOfDayComplete(void)
{
    return fTimeOfDayCompleted;
}

#if (NOT_INCLUDED_IN_ESTB_DHCP) 
// Accessor for the TOD thread
//
// Parameters:  None.
//
// Returns:
//      The TOD helper thread.  Could be NULL.
//
inline BcmTimeOfDayThread *BcmEstbIpThread::GetTodThread (void)
{
    return pfTodThread;
}
#endif

// Allows the client to query whether or not Secure Download is enabled .
//
// Parameters:  None.
//
// Returns:
//      true if enabled
//      false if not
//
inline bool BcmEstbIpThread::SecureDownloadEnabled(void)
{
    return fSecureDownload;
}



#endif




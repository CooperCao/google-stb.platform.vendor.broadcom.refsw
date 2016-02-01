//****************************************************************************
//
// Copyright (c) 2000-2010 Broadcom Corporation
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
//  Filename:       PingThread.h
//  Author:         David Pullen
//  Creation Date:  Jan 24, 2000
//
//****************************************************************************
//  Description:
//      This is the abstract base class for the ICMP Echo Request protocol,
//      commonly known as "Ping".  It runs as a separate thread, allowing it to
//      be controlled separately.  Most of the real work is left to derived
//      classes because of OS-specific network stack requirements.
//
//      This IP helper supports a LOT of different options, including variable
//      packet sizes, variable timeout options, etc.
//
//      This class is multiply instantiable, which means that you can have more
//      than one ping running at a time.
//
//      It is up to some external entity to figure out what options should be
//      used (usually this will be some command-line interpriter) and configure
//      the object.
//
//****************************************************************************

#ifndef PINGTHREAD_H
#define PINGTHREAD_H

//********************** Include Files ***************************************

// My base class...
#include "Thread.h"

// Other objects of interest.
#include "IpAddress.h"

    
#if ( BCM_PING_CONSOLE_SUPPORT )
#include "PingCommandTable.h"
#endif

#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <iostream>
#else
#include <iostream.h>
#endif

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class BcmEvent;
class BcmCountingSemaphore;

//********************** Class Declaration ***********************************


class BcmPingThread : public BcmThread
{
public:

    // Constructor.  Initializes the state of the object and kicks off the
    // thread.  Note that it doesn't begin pinging until it is configured and
    // told to start.
    //
    // Warning - don't set the thread priority too high, or it will consume a
    // LOT of the CPU!  In fact, you probably want it to run at or below the
    // priority of the thread that created it so that that thread will be able
    // to control it in a timely fashion.
    //
    // Parameters:
    //      pThreadName - the name of this thread (for debugging purposes).
    //      initialPriority - the thread priority that should be used.
    //
    // Returns:  N/A
    //
    BcmPingThread(const char *pThreadName = "Ping Thread",
                  BcmOperatingSystem::ThreadPriority initialPriority = BcmOperatingSystem::kNormalPriority);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmPingThread();

    // This allows a client to specify an event that should be posted when the
    // thread finished pinging (either because someone called Stop() or because
    // the requested number of pings has been sent).
    //
    // Note that you MUST call this with NULL to get the object to release the
    // pointer to the event before deleting the event!
    //
    // This object does not delete the event; it simply uses it.
    //
    // Parameters:
    //      pEvent - pointer to the event to be posted when pinging is done.
    //
    // Returns:
    //      A pointer to the previous event that was being used.
    //
    BcmEvent *SetEvent(BcmEvent *pEvent);

    // This resets the various ping options to a reasonable default - only 3
    // pings will be sent, they will all be 64 bytes, they will be sent 1 second
    // apart, the thread will wait 5 seconds for the reply, and it will do full
    // logging display.
    //
    // The IP address will be left alone.
    //
    // This can be changed while the ping is running.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    void ResetDefaultOptions(void);

    // Allows the client to set the IP address of the target to be pinged.
    //
    // This can be changed while the ping is running.
    //
    // Parameters:
    //      ipAddress - the IP address of the ping target.
    //
    // Returns:  Nothing.
    //
    void SetIpAddress(const BcmIpAddress &ipAddress);
    const BcmIpAddress &GetIpAddress () const;

    // Allows the client to set the number of pings to be sent.  This can be
    // set to any number between 1 and (2^32 - 1), or can be set to "infinite"
    // via the boolean parameter.
    //
    // Setting isInfinite to false and numberOfPings to 0 is not allowed.
    //
    // Note that if you set isInfinite to true, then you must call Stop() in
    // order to get it to stop.
    //
    // This can be changed while the ping is running.
    //
    // Parameters:
    //      isInfinite - set this to true to let the ping keep running until
    //                   stopped, or false to use numberOfPings.
    //      numberOfPings - the number of pings to send (ignored if isInfinite
    //                      is true).
    //
    // Returns:
    //      true if the values were set correctly.
    //      false if the combination of values is incorrect.
    //
    bool SetNumberOfPings(bool isInfinite, unsigned long numberOfPings);
    unsigned long GetNumberOfPings () const;

    // Allows the client to set the sizes of the ping packets that will be
    // sent.  This size includes the LLC header, IP header, ICMP header, and
    // the CRC32 at the end.  You must specify values between 64 and 1518, which
    // are the min and max size Ethernet frames.
    //
    // The step amount can be any number from 0 (which leaves the ping size
    // constant) up to (2^32 - 1).  Note that after adding the step amount to
    // the current ping size, if this is greater than the end size, then the
    // current size will be wrapped around to the start size.
    //
    // Note that 64 <= start <= end <= 1518.  If any of the values is outside
    // of the allowed range, then it is clamped to be valid.
    //
    // If you want to send ping packets of the same size every time, set
    // the start and end size to be equal, and the step amount to be 0.
    //
    // To do a sweeping ping of every packet size, set start to 64, end to
    // 1518, and step amount to 1.  You can set step amount to -1 to sweep in
    // the other direction.
    // 
    // This can be changed while the ping is running.
    //
    // Parameters:
    //      startSizeBytes - the packet size to use for the first ping.  Must
    //                       be 64 or greater.
    //      endSizeBytes - the largest ping packet to be sent.  Must be 1518 or
    //                     smaller.
    //      stepAmountBytes - the number of bytes that the ping packet should
    //                        be increased/decreased by each time.
    //
    // Returns:  Nothing.
    //
    void SetPingSize(unsigned long startSizeBytes, unsigned long endSizeBytes,
                     signed long stepAmountBytes);
    void GetPingSize (unsigned long &startSizeBytes, unsigned long &endSizeBytes,
                     signed long &stepAmountBytes) const;                     

    // Allows the client to run in Backoff-Retry Mode.  Time is stopped as soon as
    // a ping reply is successfully received.  If ping is timeout, it will sleep for  
    // Backoff-Retry Wait time before sending next ping.
    //
    // Note that the amount of time between sending one ping and sending the
    // next ping varies depending on whether or not you enable waiting for
    // replies, and how long it takes to receive the reply.
    //
    // If waiting for replies is disabled, then this is the time between sending
    // one ping and sending the next ping.
    //
    // This can be changed while the ping is running.
    //
    // Backoff-Retry Algorithm
    //          Wait time[0] = backoffRetryInitTimeMS
    //          Exp Wait Increment[0] = 0;
    //          Exp Wait Increment[1] = backoffRetryExpTimeMS
    //          Exp Wait Increment[n] = 2 * Exp Wait Increment[n-1] ; for n >=2
    //          Wait time [n] = Wait time[n-1]  + * Exp Wait Increment[n]; for n >=1
    //
    // Parameters:
    //      enable                             - enable/disable Backoff-Retry Mode.
    //      backoffRetryInitTimeMS - initial wait time in Backoff-Retry Algorithm.
    //      backoffRetryExpTimeMS - exponential increment in Backoff-Retry Algorithm.
    // Returns:  Nothing.
    //
    void SetBackoffRetryMode(bool enable, 
                             unsigned long backoffRetryInitTimeMS, 
                             unsigned long backoffRetryExpTimeMS);
    bool GetBackoffRetryMode(unsigned long &backoffRetryInitTimeMS,
                             unsigned long &backoffRetryExpTimeMS) const;
    
                     
    // Allows the client to set the amount of time (in milliseconds) that the
    // thread will sleep before sending a ping.  Generally, this is the time
    // between when a ping reply is received and when the next ping will be
    // sent.
    //
    // Note that the amount of time between sending one ping and sending the
    // next ping varies depending on whether or not you enable waiting for
    // replies, and how long it takes to receive the reply.
    //
    // If waiting for replies is disabled, then this is the time between sending
    // one ping and sending the next ping.
    //
    // This can be changed while the ping is running.
    //
    // Parameters:
    //      timeBetweenPingsMS - the number of ms to wait before sending a ping.
    //
    // Returns:  Nothing.
    //
    void SetTimeBetweenPingsMS(unsigned long timeBetweenPingsMS);
    unsigned long GetTimeBetweenPingsMS() const;

    // Allows the client to set the amount of time (in milliseconds) that the
    // thread should wait for a reply after sending a ping.
    //
    // If enable is set to true, then the thread will send a ping and wait for
    // a response for the specified amount of time.  Event if timeToWaitMS is
    // set to 0, the thread will check for and process a reply packet.
    //
    // If enable is set to false, then the thread will not wait at all, and
    // won't even check for any replies.
    //
    // This can be changed while the ping is running.
    //
    // Parameters:
    //      enable - set to true to enable waiting for replies, or false to
    //               skip reply processing altogether.
    //      timeToWaitMS - the number of ms to wait for a reply (ignored if
    //                     enable is false).
    //
    // Returns:  Nothing.
    //
    void SetTimeToWaitForReplyMS(bool enable, unsigned long timeToWaitMS);
    int GetTimeToWaitForReplyMS () const;

    // Allows the client to enable or disable reply verification (you must
    // enable waiting for replies for this to do anything).
    //
    // This can be changed while the ping is running.
    //
    // Parameters:
    //      enable - set to true to enable verification, or false to disable.
    //
    // Returns:  Nothing.
    //
    void SetReplyVerification(bool enable);
    bool GetReplyVerification () const;

    typedef enum
    {
        kNone,              // Displays nothing at any time during the ping.
        kSummaryOnly,       // Displays only the ping results after pinging has
                            // stopped.  This give you the most throughput.
        kMinimal,           // Displays a 'p' every second to show that it is
                            // still running.
        kFull               // Displayes the typical ping results of bytes sent,
                            // round-trip delay, etc.  This slows things down
                            // significantly!
    } ReplyDisplayMode;

    // Allows the client to select the amount of information that the thread
    // will display while pinging.
    //
    // This can be changed while the ping is running.
    //
    // Parameters:
    //      displayMode - the amount of log information that should be displayed.
    //
    // Returns:  Nothing.
    //
    void SetReplyDisplayMode(ReplyDisplayMode displayMode);
    ReplyDisplayMode GetReplyDisplayMode () const;

    // Allows the client to select the IP stack on which the pings will be
    // sent.  Specifying 0 means that you want to use the default IP stack.
    //
    // This can only be changed when the ping is not running.
    //
    // Parameters:
    //      stackNumber - the IP stack number to run on.
    //
    // Returns:  Nothing.
    //
    void SetIpStackNumber(unsigned int stackNumber);
    unsigned int GetIpStackNumber () const;

    // Tells the thread to begin pinging.  If the thread is already pinging,
    // then this does nothing.  The ping statistics will be reset.
    //
    // You must set the ping parameters before calling this!
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if the ping was started successfully or if the ping is already
    //          running.
    //      false if there was some problem with starting the ping.
    //
    bool Start(void);

    // If the ping is running, then this causes it to stop.  This is generally
    // only necessary to stop an "infinite" ping, but it can also be useful to
    // stop a regular one early.
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if the ping was stopped successfully, or if it was already
    //          stopped.
    //      false if there was a problem (timed out waiting, failed to signal
    //          the thread, etc.).
    //
    bool Stop(void);

    typedef struct
    {
        // Stats for the number of pings sent, replies received, and (if enabled)
        // the number of replies that failed verification.
        unsigned long pingsSent;
        unsigned long repliesReceived;
        unsigned long repliesPassedVerification;
        unsigned long repliesFailedVerification;

        // The total number of bytes sent and received; these are useful because
        // the packet size can vary.  The received bytes does not include replies
        // that failed verification.
        unsigned long totalBytesSent;
        unsigned long totalBytesReceived;

        // Stats for the time spend waiting for replies (min, max, and total).
        unsigned long minTimeWaitingForReplyMS;
        unsigned long maxTimeWaitingForReplyMS;
        unsigned long totalTimeWaitingForReplyMS;

        // The millisecond system clock value when the thread sent the first
        // ping.
        unsigned long startTimeMS;

        // The millisecond system clock value when the thread stopped sending
        // pings (this is only valid if the thread has stopped).
        unsigned long stopTimeMS;

       // Number of ICMP errors.
       unsigned long pingNumIcmpError;

       // The last ICMP error.
       unsigned long pingIcmpType;


    } PingStats;

    // Returns the ping counter statistics that have been gathered since the
    // last call to Start().
    //
    // This can be called while the ping is running.
    //
    // Parameters:
    //      pingStats - reference to a struct into which the stats should be
    //                  copied.
    //
    // Returns:  Nothing.
    //
    void GetStatistics(PingStats &pingStats);

    // Displays the settings to the specified ostream.
    //
    // Parameters:
    //      ostrm - the ostream to be written to.
    //
    // Returns:
    //      A reference to the ostream to allow chaining.
    //
    ostream &Print(ostream &ostrm) const;

    // Allows the caller to query whether or not the thread is pinging.
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if the thread is pinging.
    //      false if the thread is not pinging.
    //
    inline bool PingingInProgress(void) const;

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

    // Calculates the checksum required for ICMP, as described in RFC 1071.
    //
    // You must set the checksum field in the buffer to 0's before calling this.
    //
    // Parameters:
    //      buffer - reference to the octet buffer containing the data for which
    //               the checksum is to be calculated.
    //      numberOfBytes - the number of bytes to be used for the calculation.
    //
    // Returns:
    //      The checksum.
    //
    uint16 ChecksumRfc1071(BcmOctetBuffer &buffer, unsigned int numberOfBytes);

    // This method has the code that does the actual work.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    void PingImpl(void);

protected:

    // Various parameters/options that can be set.
    BcmIpAddress fTargetIpAddress;

    unsigned long fNumberOfPings;

    unsigned long fStartPingSizeBytes;
    unsigned long fEndPingSizeBytes;
    signed long fStepAmountBytes;

    // stop ping if a reply is successfully received, 
    // if ping timeout, retry another ping with exponential wait time between pings.
    // BackoffRetry takes preferences over TimeBetweenPingsMS
    bool fBackoffRetryMode;
    unsigned long fBackoffRetryInitTimeMS;
    unsigned long fBackoffRetryExpTimeMS;
    
    unsigned long fTimeBetweenPingsMS;

    bool fWaitForReply;
    unsigned long fTimeToWaitForReplyMS;

    bool fVerifyReply;

    ReplyDisplayMode fReplyDisplayMode;

    unsigned int fIpStackNumber;

    PingStats fPingStats;

    // Events to control the operation of the ping thread.
    BcmEvent *pfStartEvent;
    BcmEvent *pfStopEvent;
    bool fStopPinging;
    BcmCountingSemaphore *pfPingStoppedSemaphore;
    bool fExitThread;
    bool fPingingInProgress;

    BcmEvent *pfEventToTrigger;

public:


#if ( BFC_INCLUDE_PING_CONSOLE_SUPPORT )
    // This static method allows an object (including myself) to get access to
    // the singleton command table for controlling objects of my type.  This
    // helps avoid order of initialization problems by ensuring that the
    // object is created and initialized on-demand.
    //
    // Parameters:  None.
    //
    // Returns:
    //      A reference to the command table singleton for my class.
    //
    static BcmPingCommandTable &GetCommandTableSingleton(void);

    // Since the command handler code is in the command table, I need to allow
    // it full access to me.
    friend class BcmPingCommandTable;
#endif

private:

    // Copy Constructor.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmPingThread(const BcmPingThread &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmPingThread & operator = (const BcmPingThread &otherInstance);

};


// This is a helper that allows the PingStats structure to be dumped to an
// ostream.
ostream & operator << (ostream &ostrm, const BcmPingThread::PingStats &pingStats);


//********************** Inline Method Implementations ***********************

// Allows the caller to query whether or not the thread is pinging.
//
// Parameters:  None.
//
// Returns:
//      true if the thread is pinging.
//      false if the thread is not pinging.
//
inline bool BcmPingThread::PingingInProgress(void) const
{
    return fPingingInProgress;
}


// This allows the Ping settings to be dumped to an ostream.
inline ostream & operator << (ostream &ostrm, const BcmPingThread &ping)
{
    return ping.Print(ostrm);
}


#endif



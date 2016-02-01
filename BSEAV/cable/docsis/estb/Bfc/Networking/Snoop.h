//****************************************************************************
//
// Copyright (c) 2002-2010 Broadcom Corporation
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
//  Filename:       Snoop.h
//  Author:         David Pullen
//  Creation Date:  May 3, 2002
//
//****************************************************************************
//  Description:
//      This file contains the abstract base class for all types of Snoop
//      classes.  A Snoop is an object that inspects (snoops) a packet, and
//      decides whether to discard it, pass it, modify it, then pass it, or
//      possibly even to discard it and generate its own packet (back to the
//      sender).
//
//      A Snoop has both Inbound and Outbound processing capabilities, and
//      derived classes can choose to implement either or both of them if it
//      suites their needs.  The base class provides dummy implementations so
//      that derived classes only have to implement the method they are
//      interested in.
//
//      This file was added for PR1880.
//
//****************************************************************************

#ifndef SNOOP_H
#define SNOOP_H

// Set this to 0 to remove the Snoop debugging code (for performance and code
// size).  Set this to 1 to compile in Snoop debugging code so that it can be
// enabled/disabled at runtime.
//
// For "slim" images, always set this to 0.
#if !(BCM_REDUCED_IMAGE_SIZE)
#define SNOOP_DEBUG 0
#else
#define SNOOP_DEBUG 0
#endif

//********************** Include Files ***************************************

#include "MessageLog.h"

#if SNOOP_DEBUG
#include "EthernetPacket.h"
#endif

// For storing the HalIfs that I'm registered with.
#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <vector>
#else
#include <vector.h>
#endif

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class BcmHalIf;
class BcmEthernetPacket;

//********************** Class Declaration ***********************************


class BcmSnoop
{
public:

    typedef enum
    {
        kInbound,
        kOutbound,
        kInAndOutbound
    } SnoopDirection;


    // Initializing Constructor.  Initializes the state of the object...
    //
    // Parameters:
    //      pName - the name of the Snoop (for debug purposes).
    //      canBeDeleted - true if the object was "newed" and can be deleted.
    //                     false if the object was created statically or is
    //                     associated with some object that will delete it.
    //                     This is primarily to support the console, which
    //                     allows the user to create/delete Snoops.
    //
    // Returns:  N/A
    //
    BcmSnoop(const char *pName, bool canBeDeleted);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmSnoop();

public:     // Configuration methods.

    // Returns the name of the Snoop object.
    //
    // Parameters:  None.
    //
    // Returns:
    //      The name of the Snoop object.
    //
    inline const char *Name(void) const;

    void Name(const char *pName);

    // Returns information about whether or not the Snoop can be deleted (by the
    // console or some other client who didn't create the Snoop).  This is set
    // by the owner when the Snoop is created.
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if the Snoop can be deleted by a client other than the creator
    //          (after deregistering it and otherwise ensuring that it won't be
    //          used any more, of course).
    //      false if the Snoop was statically created or will be used and/or
    //          deleted by the creator.
    //
    inline bool CanBeDeleted(void) const;

    // Accessors to set/get the priority of the Snoop.  This is used when
    // sorting Snoops in a container, so that the highest priority Snoop is
    // applied first.
    //
    // Because a Snoop can be in both inbound and outbound lists, there is a
    // separate priority for each.  A Snoop might be applied first in one
    // direction, then last in the other direction.
    //
    // By default, the priority is set to 0 for both directions.
    //
    // Note:  The Snoop automatically calls the BcmHalIf objects that it is
    //        registered with so that the lists are sorted in the correct order.
    //
    // Parmaeters:
    //      priority - the new priority for the Snoop.
    //
    // Returns:
    //      The priority of the Snoop.
    //
    void InboundPriority(unsigned int priority);
    inline unsigned int InboundPriority(void) const;

    void OutboundPriority(unsigned int priority);
    inline unsigned int OutboundPriority(void) const;

    // Accessors to enable/disable a Snoop, and query the state.
    //
    // Parameters:
    //      enable - true to enable the Snoop, false to disable.
    //
    // Returns:
    //      true if the Snoop is enabled.
    //      false if the Snoop is disabled.
    //
    inline void Enable(bool enable);
    inline bool Enabled(void) const;

    // Accessors to enable/disable the before/after debug printing of packets.
    // This allows you to take a snapshot of the packet before sending it to
    // the derived class, and then snapshot the packet after the derived class
    // returns (only if it is passing the packet).
    //
    // Parameters:
    //      enable - true to enable Snoop debug.
    //
    // Returns:
    //      true if Snoop debug is enabled.
    //      false if Snoop debug is disabled.
    //
    inline void EnableDebug(bool enable, SnoopDirection direction=kInAndOutbound);
    inline bool DebugEnabled(void) const;

    inline void EnableDiscardDebug(bool enable);
        
    inline void EnableVerboseDebug(bool enable);
    inline bool EnableVerboseDebug(void) const;


    // Prints the state of the Snoop.
    //
    // Parameters:
    //      outStream - the ostream to print to.
    //
    // Returns:
    //      A reference to the ostream to support chaining.
    //
    ostream &Print(ostream &outStream) const;

public:     // Operational methods.

    typedef enum
    {
        // This means that the packet should be discarded without further
        // processing.  This may require the packet to be released back to
        // the source HalIf, depending on where in the system it is being
        // dropped.
        kDiscardPacket,

        // This means that the packet can be passed on for further processing,
        // either to the next Snoop, or to the next stage in the system (the
        // forwarder, the outbound HalIf, etc).
        kPassPacket,

        // This means that the packet was consumed by the Snoop, and will be
        // released back to the source HalIf by it.  The packet must not be
        // discarded and must not be passed on for further processing.  This
        // acts as a short-circuit, bypassing parts of the system.
        kPacketConsumed

    } SnoopResult;

    // This is the public entrypoint to the Snoop API for inbound Snoops.  It
    // checks the "enabled" flag, then calls the derived class if enabled.
    //
    // This method is inlined for efficiency.
    //
    // Parameters:
    //      pPacket - the packet to be Snooped.
    //      pSrcHalIf - the HalIf that the packet was received on.
    //
    // Returns:
    //      One of the SnoopResult enum values.
    //
    inline SnoopResult SnoopInbound(BcmEthernetPacket *pPacket,
                                    BcmHalIf *pSrcHalIf);

    // This is the public entrypoint to the Snoop API for outbound Snoops.  It
    // checks the "enabled" flag, then calls the derived class if enabled.
    //
    // This method is inlined for efficiency.
    //
    // Parameters:
    //      pPacket - the packet to be Snooped.
    //      pSrcHalIf - the HalIf that the packet was received on.
    //      pDestHalIf - the HalIf that the packet is being sent to.
    //
    // Returns:
    //      One of the SnoopResult enum values.
    //
    inline SnoopResult SnoopOutbound(BcmEthernetPacket *pPacket,
                                     BcmHalIf *pSrcHalIf,
                                     BcmHalIf *pDestHalIf);

public:     // These methods should only be called by BcmHalIf.  When I am
            // registered with the HalIf, it will register itself with me.

    // Called by the HalIf that this Snoop is being registered with; lets the
    // Snoop know that it is being added to the inbound set of Snoops for the
    // HalIf.  This is primarily used by the Snoop to get the HalIfs to sort
    // their lists of Snoops when the Snoop priority has changed; also used
    // for debug purposes.
    //
    // Parameters:
    //      pHalIf - the HalIf that the Snoop is being registered with.
    //
    // Returns:  Nothing.
    //
    void RegisterInbound(BcmHalIf *pHalIf);

    // Called by the HalIf that this Snoop is being deregistered from; lets the
    // Snoop know that it is being removed from the inbound set of Snoops for
    // the HalIf.
    //
    // Parameters:
    //      pHalIf - the HalIf that the Snoop is being deregistered from.
    //
    // Returns:  Nothing.
    //
    void DeregisterInbound(BcmHalIf *pHalIf);

    // Called by the HalIf that this Snoop is being registered with; lets the
    // Snoop know that it is being added to the outbound set of Snoops for the
    // HalIf.  This is primarily used by the Snoop to get the HalIfs to sort
    // their lists of Snoops when the Snoop priority has changed; also used
    // for debug purposes.
    //
    // Parameters:
    //      pHalIf - the HalIf that the Snoop is being registered with.
    //
    // Returns:  Nothing.
    //
    void RegisterOutbound(BcmHalIf *pHalIf);

    // Called by the HalIf that this Snoop is being deregistered from; lets the
    // Snoop know that it is being removed from the inbound set of Snoops for
    // the HalIf.
    //
    // Parameters:
    //      pHalIf - the HalIf that the Snoop is being deregistered from.
    //
    // Returns:  Nothing.
    //
    void DeregisterOutbound(BcmHalIf *pHalIf);

protected:

    // This method is called by SnoopInbound when the base class has finished
    // all of its pre-processing.
    //
    // The derived class can Snoop the packet here, overriding the base class
    // method, if it only needs to know the source HalIf and the packet to do
    // its job.
    //
    // The base class simply returns kPassPacket without any other processing.
    // This is for the convenience of Snoops that only implement an Outbound
    // Snoop.
    //
    // Parameters:
    //      pPacket - the packet to be Snooped.
    //      pSrcHalIf - the HalIf that the packet was received on.
    //
    // Returns:
    //      One of the SnoopResult enum values.
    //
    virtual SnoopResult SnoopInboundImpl(BcmEthernetPacket *pPacket,
                                         BcmHalIf *pSrcHalIf);

    // This method is called by SnoopOutbound when the base class has finished
    // all of its pre-processing.
    //
    // The derived class can Snoop the packet here, overriding the base class
    // method, if it needs to know the source/dest HalIfs and the packet to do
    // its job.
    //
    // The base class simply returns kPassPacket without any other processing.
    // This is for the convenience of Snoops that only implement an Inbound
    // Snoop.
    //
    // Parameters:
    //      pPacket - the packet to be Snooped.
    //      pSrcHalIf - the HalIf that the packet was received on.
    //      pDestHalIf - the HalIf that the packet is being sent to.
    //
    // Returns:
    //      One of the SnoopResult enum values.
    //
    virtual SnoopResult SnoopOutboundImpl(BcmEthernetPacket *pPacket,
                                          BcmHalIf *pSrcHalIf,
                                          BcmHalIf *pDestHalIf);

    // This method is called by Print when the base class has finished printing
    // it's state info.
    //
    // The derived class can override this method to print its state information
    // (if there is any).
    //
    // Parameters:
    //      outStream - the ostream to print to.
    //
    // Returns:
    //      A reference to the ostream to support chaining.
    //
    virtual ostream &PrintImpl(ostream &outStream) const;

protected:

    // Enables/disables before/after packet display and other Snoop debugging.
    bool fVerboseDebugEnabled;
    bool fInboundDebugEnabled;
    bool fOutboundDebugEnabled;
    bool fDiscardOnlyDebugEnabled;
    

    // For debug output.
    BcmMessageLogSettings fMessageLogSettings;

    // The HalIf objects that I am registered with.  This is primarily used when
    // my priority changes - I need to tell each of my HalIfs to sort the list
    // of Snoops.
    vector<BcmHalIf *> fInboundHalIfs;
    vector<BcmHalIf *> fOutboundHalIfs;
    
private:

    // This determines the order of the Snoops in the list kept by the BcmHalIf
    // class.  It sorts these numerically, where higher numbers come before
    // lower numbers.  Someone on the outside has to make sure that the
    // priorities are assigned such that they all make sense in all combinations
    // and they avoid overlap.
    unsigned int fInboundPriority;
    unsigned int fOutboundPriority;

    // Common way to enable/disable the Snoop.  If disabled, then they return
    // kPassPacket and don't run.
    bool fIsEnabled;

    // Some Snoops are "singleton-like", while others are created dynamically.
    // This is set when the object is created to let an external client know
    // whether or not the Snoop must be deleted when removing it from a list, or
    // if it can't be deleted because of the way it was created.
    bool fCanBeDeleted;

    // Text name for debug display.  This is set by the derived class.
    const char *pfSnoopName;

private:

    // Default Constructor.  Not supported.
    BcmSnoop(void);

    // Copy Constructor.  Not supported.
    BcmSnoop(const BcmSnoop &otherInstance);

    // Assignment operator.  Not supported.
    BcmSnoop & operator = (const BcmSnoop &otherInstance);

};


//********************** Inline Method Implementations ***********************


// Insertion operator for an ostream.
inline ostream & operator << (ostream &outStream, const BcmSnoop &snoop)
{
    return snoop.Print(outStream);
}


// Returns the name of the Snoop object.
//
// Parameters:  None.
//
// Returns:
//      The name of the Snoop object.
//
inline const char *BcmSnoop::Name(void) const
{
    return pfSnoopName;
}


// Returns information about whether or not the Snoop can be deleted (by the
// console or some other client who didn't create the Snoop).  This is set
// by the owner when the Snoop is created.
//
// Parameters:  None.
//
// Returns:
//      true if the Snoop can be deleted by a client other than the creator
//          (after deregistering it and otherwise ensuring that it won't be
//          used any more, of course).
//      false if the Snoop was statically created or will be used and/or
//          deleted by the creator.
//
inline bool BcmSnoop::CanBeDeleted(void) const
{
    return fCanBeDeleted;
}


// Accessors to set/get the priority of the Snoop.  This is used when
// sorting Snoops in a container, so that the highest priority Snoop is
// applied first.
//
// Because a Snoop can be in both inbound and outbound lists, there is a
// separate priority for each.  A Snoop might be applied first in one
// direction, then last in the other direction.
//
// By default, the priority is set to 0 for both directions.
//
// Note:  If you change the priority of a Snoop while it is in a container,
//        then you must resort the container manually in order for it to
//        take effect.
//
// Parmaeters:
//      priority - the new priority for the Snoop.
//
// Returns:
//      The priority of the Snoop.
//
inline unsigned int BcmSnoop::InboundPriority(void) const
{
    return fInboundPriority;
}


inline unsigned int BcmSnoop::OutboundPriority(void) const
{
    return fOutboundPriority;
}


// Accessors to enable/disable a Snoop, and query the state.
//
// Parameters:
//      enable - true to enable the Snoop, false to disable.
//
// Returns:
//      true if the Snoop is enabled.
//      false if the Snoop is disabled.
//
inline void BcmSnoop::Enable(bool enable)
{
    fIsEnabled = enable;
}


inline bool BcmSnoop::Enabled(void) const
{
    return fIsEnabled;
}


// Accessors to enable/disable the before/after debug printing of packets.
// This allows you to take a snapshot of the packet before sending it to
// the derived class, and then snapshot the packet after the derived class
// returns (only if it is passing the packet).
//
// Parameters:
//      enable - true to enable Snoop debug.
//
// Returns:
//      true if Snoop debug is enabled.
//      false if Snoop debug is disabled.
//
inline void BcmSnoop::EnableDebug(bool enable, SnoopDirection direction)
{
    if (direction == kInAndOutbound)
    {
        fInboundDebugEnabled = enable;
        fOutboundDebugEnabled = enable;
    }
    else if (direction == kInbound)
    {
        fInboundDebugEnabled = enable;
    }
    else
    {
        fOutboundDebugEnabled = enable;
    }
}


inline bool BcmSnoop::DebugEnabled(void) const
{
    return fInboundDebugEnabled | fOutboundDebugEnabled;
}


inline void BcmSnoop::EnableVerboseDebug(bool enable)
{
    fVerboseDebugEnabled = enable;
}

inline bool BcmSnoop::EnableVerboseDebug(void) const
{
    return fVerboseDebugEnabled;
}

inline void BcmSnoop::EnableDiscardDebug(bool enable)
{
    fDiscardOnlyDebugEnabled = enable;
}


// This is the public entrypoint to the Snoop API for inbound Snoops.  It
// checks the "enabled" flag, then calls the derived class if enabled.
//
// This method is inlined for efficiency.
//
// Parameters:
//      pPacket - the packet to be Snooped.
//      pSrcHalIf - the HalIf that the packet was received on.
//
// Returns:
//      One of the SnoopResult enum values.
//
inline BcmSnoop::SnoopResult BcmSnoop::SnoopInbound(BcmEthernetPacket *pPacket,
                                                    BcmHalIf *pSrcHalIf)
{
    SnoopResult result = kPassPacket;

    CallTraceDatapath("BcmSnoop", "SnoopInbound");

    #if SNOOP_DEBUG
    
    if (fInboundDebugEnabled)
    {
        if (fDiscardOnlyDebugEnabled )
        {
            // If we are debugging to see discarded packets,
            // Don't show any pre-processing debug!
        }

        else if (fVerboseDebugEnabled)
        {
            gAlwaysMsg(fMessageLogSettings, "SnoopInbound")
                << "Entering Inbound Snoop '" << pfSnoopName 
                << "'. Packet before processing:\n" << *pPacket << endl;
        }
        else
        {
            gAlwaysMsg(fMessageLogSettings, "SnoopInbound")
                << "Entering Inbound Snoop '" << pfSnoopName << "'." << endl;

            if (pgMessageLogStream)
            {
                pPacket->Print(*pgMessageLogStream,false);
            }
        }
        

        // Mark this so that we will know whether or not the packet actually
        // changed during the operation of the Snoop.
        pPacket->PacketChanged();
    }

    #endif

    if (fIsEnabled)
    {
        result = SnoopInboundImpl(pPacket, pSrcHalIf);

        #if SNOOP_DEBUG
        
        if (fInboundDebugEnabled)
        {
            if (result == kDiscardPacket)
            {
                gAlwaysMsg(fMessageLogSettings, "SnoopInbound")
                    << "Exiting Inbound Snoop '" << pfSnoopName 
                    << "'.  Packet is being discarded.\n" << *pPacket << endl;
                
            }
            else if (result == kPacketConsumed)
            {
                if (fDiscardOnlyDebugEnabled)
                {
                    // If we are debugging only to see discarded packets,
                    // don't show a consumed packet!
                }
                else
                {
                    gAlwaysMsg(fMessageLogSettings, "SnoopInbound")
                    << "Exiting Inbound Snoop '" << pfSnoopName 
                    << "'.  Packet was consumed by the Snoop.\n" << endl;
                }
            }
            else
            {
                if ( fDiscardOnlyDebugEnabled )
                {
                    // If we are debugging to see discarded packets,
                    // don't show a pass packet!
                }

                // Only print the packet if modified.
                else if (pPacket->PacketChanged())
                {
                    if (fVerboseDebugEnabled)
                    { 
                        gAlwaysMsg(fMessageLogSettings, "SnoopInbound")
                            << "Exiting Inbound Snoop '" << pfSnoopName 
                            << "'.  Packet after processing:\n"
                            << *pPacket << endl;
                    }
                    else
                    {
                        gAlwaysMsg(fMessageLogSettings, "SnoopInbound")
                            << "Exiting Inbound Snoop '" << pfSnoopName 
                            << "'." << endl;

                        if (pgMessageLogStream)
                        {
                            pPacket->Print(*pgMessageLogStream,false);
                        }
                    }

                }
                else
                {
                    gAlwaysMsg(fMessageLogSettings, "SnoopInbound")
                        << "Exiting Inbound Snoop '" << pfSnoopName
                        << "'.  Passing packet unmodified.\n" << endl;
                }
            }
        }

        #endif
    }

    #if SNOOP_DEBUG
    
    else if (fInboundDebugEnabled && !fDiscardOnlyDebugEnabled)
    {
        gAlwaysMsg(fMessageLogSettings, "SnoopInbound")
            << "Inbound Snoop '" << pfSnoopName 
            << "' is disabled; passing packet." << endl;
    }

    #endif

    return result;
}


// This is the public entrypoint to the Snoop API for outbound Snoops.  It
// checks the "enabled" flag, then calls the derived class if enabled.
//
// This method is inlined for efficiency.
//
// Parameters:
//      pPacket - the packet to be Snooped.
//      pSrcHalIf - the HalIf that the packet was received on.
//      pDestHalIf - the HalIf that the packet is being sent to.
//
// Returns:
//      One of the SnoopResult enum values.
//
inline BcmSnoop::SnoopResult BcmSnoop::SnoopOutbound(BcmEthernetPacket *pPacket,
                                                     BcmHalIf *pSrcHalIf,
                                                     BcmHalIf *pDestHalIf)
{
    SnoopResult result = kPassPacket;

    CallTraceDatapath("BcmSnoop", "SnoopOutbound");

    #if SNOOP_DEBUG
    
    if ( fOutboundDebugEnabled )
    {
        if ( fDiscardOnlyDebugEnabled )
        {
            // If we are debugging to see discarded packets,
            // Don't show any pre-processing debug!
        }

        else if (fVerboseDebugEnabled)
        {
            gAlwaysMsg(fMessageLogSettings, "SnoopOutbound")
                << "Entering Outbound Snoop '" << pfSnoopName 
                << "'.  Packet before processing:\n" << *pPacket << endl;
        }
        else
        {
            gAlwaysMsg(fMessageLogSettings, "SnoopOutbound")
                << "Entering Outbound Snoop '" << pfSnoopName << "'." << endl;

            if (pgMessageLogStream)
            {
                pPacket->Print(*pgMessageLogStream,false);
            }
        }
        
        // Mark this so that we will know whether or not the packet actually
        // changed during the operation of the Snoop.
        pPacket->PacketChanged();
    }

    #endif

    if (fIsEnabled)
    {
        result = SnoopOutboundImpl(pPacket, pSrcHalIf, pDestHalIf);

        #if SNOOP_DEBUG
    
        if (fOutboundDebugEnabled)
        {
            if (result == kDiscardPacket)
            {
                gAlwaysMsg(fMessageLogSettings, "SnoopOutbound")
                    << "Exiting Outbound Snoop '" << pfSnoopName 
                    << "'.  Packet is being discarded.\n" << *pPacket << endl;
            }
            else if (result == kPacketConsumed)
            {
                if (fDiscardOnlyDebugEnabled)
                {
                    // If we are debugging only to see discarded packets,
                    // don't show a consumed packet!
                }
                else
                {
                    gAlwaysMsg(fMessageLogSettings, "SnoopOutbound")
                        << "Exiting Outbound Snoop '" << pfSnoopName 
                        << "'.  Packet was consumed by the Snoop."  << endl;
                }
            }
            else
            {
                if ( fDiscardOnlyDebugEnabled )
                {
                    // If we are debugging to see discarded packets,
                    // don't show a pass packet!
                }

                // Only print the packet if modified.
                else if (pPacket->PacketChanged())
                {
                    if (fVerboseDebugEnabled)
                    {
                        gAlwaysMsg(fMessageLogSettings, "SnoopOutbound")
                            << "Exiting Outbound Snoop '" << pfSnoopName 
                            << "'.  Passing packet after processing:\n"
                            << *pPacket << endl;
                    }
                    else
                    {
                        gAlwaysMsg(fMessageLogSettings, "SnoopOutbound")
                            << "Exiting Outbound Snoop '" << pfSnoopName 
                            << "'." << endl;

                        if (pgMessageLogStream)
                        {
                            pPacket->Print(*pgMessageLogStream,false);
                        }
                    }
                }
                else
                {
                    gAlwaysMsg(fMessageLogSettings, "SnoopOutbound")
                        << "Exiting Outbound Snoop '" << pfSnoopName
                        << "'.  Passing packet unmodified.\n" << endl;
                }
            }
        }

        #endif
    }

    #if SNOOP_DEBUG

    else if (fOutboundDebugEnabled && !fDiscardOnlyDebugEnabled)
    {
        gAlwaysMsg(fMessageLogSettings, "SnoopInbound")
            << "Outbound Snoop '" << pfSnoopName 
            << "' is disabled; passing packet." << endl;
    }

    #endif

    return result;
}


#endif



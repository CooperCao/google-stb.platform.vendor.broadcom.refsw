//****************************************************************************
//
//  Copyright (c) 2007  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16251 Laguna Canyon Road
//          Irvine, California 92618
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//
//  Filename:       DsgBroadcastTunnelPktReassembler.cpp
//  Author:         Pinar Taskiran
//  Creation Date:  May 28, 2007
//
//****************************************************************************
//  Description:
//      Class BcmDsgBroadcastTunnelPktReassembler 
//		Class BcmDsgBroadcastTunnelPktReassemblyBuffer
//
//****************************************************************************

#include "DsgBroadcastTunnelPktReassembler.h"
#include "DsgBroadcastTunnelHeader.h"
#include "EthernetPacket.h"
//********************** Local Types *****************************************

// Class BcmDsgBroadcastTunnelPktReassemblyBuffer

BcmDsgBroadcastTunnelPktReassemblyBuffer::BcmDsgBroadcastTunnelPktReassemblyBuffer(BcmEthernetPacket &ipDatagramBtFragment)
  :
    pfPayloadBuffer(NULL),
	fBufferInUse(false),
	fNumberOfFragmentsReceived(0)
{
	pfPayloadBuffer = new BcmOctetBuffer(kBroadcastTunnelMpegTwoSectionMaxSize);

	if( pfPayloadBuffer == NULL )
	{
		// ERROR
		cout << "Failed to allocate packet reassembly buffer.  " << endl;
	}
	else
	{
		// mark the time when we were first created, so we know when to timeout
		// if all fragments do not arrive
		fCreationTimeMS = SystemTimeMS();

		// Copy the data
		// Remember UDP payload includes BT header.
		// UdpLengthBytes() includes UDP header length. The size of the BT payload fragment is  UdpLengthBytes()-UDP headerlen(8) - BT headerlen(4).
		pfPayloadBuffer->AddToEnd( ipDatagramBtFragment.UdpPdu() + BcmDsgBroadcastTunnelHeader::kBtHeaderSize, 
			                      (ipDatagramBtFragment.UdpLengthBytes() - (8 + BcmDsgBroadcastTunnelHeader::kBtHeaderSize)));

		fNumberOfFragmentsReceived++;
	}
}

 
BcmDsgBroadcastTunnelPktReassemblyBuffer::~BcmDsgBroadcastTunnelPktReassemblyBuffer()
{
	if( !fBufferInUse && pfPayloadBuffer )
	{
		cout << "deleting pfPayloadBuffer" << endl;
		delete pfPayloadBuffer;
	}
}

BcmOctetBuffer* BcmDsgBroadcastTunnelPktReassemblyBuffer::PayloadBuffer()
{
	return pfPayloadBuffer;
}

void BcmDsgBroadcastTunnelPktReassemblyBuffer::BufferInUse( bool value )
{
	fBufferInUse = value;
}

//
//
// Parameters:
//      ipDatagramFragment - the fragment to be processed
//
// Returns:
//      true if the packet has been completely reassembled into reassembledPacket
//      false if the last packet has not been found and reassembledPacket is unchanged
//
uint32 BcmDsgBroadcastTunnelPktReassemblyBuffer::ProcessFragment( BcmEthernetPacket &ipDatagramBtFragment, BcmDsgBroadcastTunnelHeader &btHeader )
{
	fNumberOfFragmentsReceived++;

	// Copy the data
	// Remember UDP payload includes BT header.
	// UdpLengthBytes() includes UDP header length. The size of the BT payload fragment is  UdpLengthBytes()-UDP headerlen(8) - BT headerlen(4).
	if( !pfPayloadBuffer->AddToEnd( ipDatagramBtFragment.UdpPdu() + BcmDsgBroadcastTunnelHeader::kBtHeaderSize, 
								   (ipDatagramBtFragment.UdpLengthBytes() - (8 + BcmDsgBroadcastTunnelHeader::kBtHeaderSize)))
	  )
	{
		// ERROR - Failed to write the fragment to the reassembly buffer.
		cout << "ERROR - Failed to write the fragment to the reassembly buffer" << endl;
		return kIllegalFragment;
	}

	// Check if last segment
	if( btHeader.LastSegment() )
	{
		if( fNumberOfFragmentsReceived != (btHeader.SegmentNumber() + 1 ) )
		{
			// ERROR - Last segment but not all fragments received.
			cout << "ERROR - Last segment but not all fragments received." << endl;
			return kNotAllFragmentsReceived;
		}

		return kAllFragmentsReceived;
	}

	return kNotAllFragmentsReceived;
}

//
// Parameters:
//
// Returns:
//      true if the fragment buffer has timed out
//      false if the fragment buffer has not timed out
//
bool BcmDsgBroadcastTunnelPktReassemblyBuffer::TimedOut()
{
    if( (SystemTimeMS() - fCreationTimeMS ) >= kMaximumReassembleyBufferLifespanSeconds * 1000 )
    {
        return true;
    }
    else
    {
        return false;
    }
}

ostream& BcmDsgBroadcastTunnelPktReassemblyBuffer::Print( ostream& ostrm ) const
{
#if !(BCM_REDUCED_IMAGE_SIZE)

    ostrm 
        << " Age: " << (SystemTimeMS() - fCreationTimeMS ) 
		<< " Number of fragments received: " << fNumberOfFragmentsReceived << endl;

	if( pfPayloadBuffer )
	{
		ostrm 
			<< *pfPayloadBuffer << endl;
	}

#endif
    return ostrm;
}
/////////////////////////////////////////////////////////////////////////////////////////////////





// Class BcmDsgBroadcastTunnelPktReassembler

BcmDsgBroadcastTunnelPktReassembler::BcmDsgBroadcastTunnelPktReassembler()
  :
    fReassemblyBufferMap()
{
}

 
BcmDsgBroadcastTunnelPktReassembler::~BcmDsgBroadcastTunnelPktReassembler()
{
}


ostream& BcmDsgBroadcastTunnelPktReassembler::PrintDetailed( ostream& ostrm ) const
{    

#if !(BCM_REDUCED_IMAGE_SIZE)

    // consider flow settings
    ostrm 
        << "\n BT Reassembly Buffer List:"
        << endl;

    // consider classifier settings
    if( !fReassemblyBufferMap.empty() )
    {
        ReassemblyBufferMapConstIt tunnelSet_it = fReassemblyBufferMap.begin();
        ReassemblyBufferMapConstIt tunnelSet_end = fReassemblyBufferMap.end();
        while( tunnelSet_it != tunnelSet_end )
        {
            // found classifier settings.
            ostrm << *(tunnelSet_it->second) << endl;

            // increment iterator.
            ++tunnelSet_it;
        }   
    }
#endif

    return ostrm;
}


ostream& BcmDsgBroadcastTunnelPktReassembler::Print( ostream& ostrm ) const
{
    return PrintDetailed( ostrm );
}

void BcmDsgBroadcastTunnelPktReassembler::Reset() 
{
    ReassemblyBufferMapIt buffer_it = fReassemblyBufferMap.begin();
    ReassemblyBufferMapIt buffer_it_end = fReassemblyBufferMap.end();
    while( buffer_it != buffer_it_end )
    {
		delete buffer_it->second;

        // increment iterator.
        fReassemblyBufferMap.erase(buffer_it++);
    }   
}

// Parameters:
//      ipDatagramBtFragment - the fragment to be processed
//      pReassembledPacket - if true returned, the reassembled packet is here.
//						     This buffer must be deleted by the caller. Otherwise,
//							 a memory leak will occur.
//
// Returns:
//      true if the packet has been completely reassembled into pReassembledPacket
//      false if the last packet has not been found
//
uint32 BcmDsgBroadcastTunnelPktReassembler::ProcessBtFragment( BcmEthernetPacket &ipDatagramBtFragment, BcmOctetBuffer** pReassembledPacket )
{
//    CallTrace("BcmIpDatagramFragmentReassembler", "ProcessIpFragment");

	// Parse the packet. 
	// Verify that it is UDP/IP
	if(!(ipDatagramBtFragment.ParseTcpUdp() && (ipDatagramBtFragment.IpProtocolTypeParsed() == BcmEthernetPacket::kUdp)))
	{
		// Can't process.
		return BcmDsgBroadcastTunnelPktReassemblyBuffer::kIllegalFragment;	
	}

	// This is a UDP/IP packet
	// Read BT header information.
	BcmDsgBroadcastTunnelHeader btHeader;
	if( btHeader.ParseFrom((uint8*)ipDatagramBtFragment.UdpPdu()) )
	{
		if(btHeader.IsSegmented())
		{
			ReassemblyBufferMapIt buffer_it = fReassemblyBufferMap.find(btHeader.IdNumber());

			// Check if complete.
			if( buffer_it == fReassemblyBufferMap.end() )
			{
				cout << "Starting a new packet reassembly." << endl;

				// This is the first fragment. Start a new reassembly buffer.
				// Alert if the segment number is not zero.
				if( btHeader.SegmentNumber() != 0 )
				{
					// WARNING
					cout << "WARNING - This is the first fragment but the segment number is not zero. Start a new reassembly buffer..." << endl;
				}

				// Create a new reassembly buffer.
				BcmDsgBroadcastTunnelPktReassemblyBuffer *pBuffer = new BcmDsgBroadcastTunnelPktReassemblyBuffer(ipDatagramBtFragment);
				
				if( pBuffer )
				{
					// Insert
					fReassemblyBufferMap.insert( ReassemblyBufferMap::value_type(btHeader.IdNumber(), pBuffer));
				}
				else
				{
					// ERROR - Failed to allocate BcmDsgBroadcastTunnelPktReassemblyBuffer
					cout << "ERROR - Failed to allocate BcmDsgBroadcastTunnelPktReassemblyBuffer." << endl;

				}
			}
			else
			{
				cout << "Fragment received for pkt ID: " << (uint32) btHeader.IdNumber() << endl;

				// Add the new fragment to the reassembly buffer
				uint32 result = buffer_it->second->ProcessFragment(ipDatagramBtFragment, btHeader);
				if( result == BcmDsgBroadcastTunnelPktReassemblyBuffer::kAllFragmentsReceived)
				{
					*pReassembledPacket = buffer_it->second->PayloadBuffer();
					if(*pReassembledPacket == NULL )
					{
						cout << "*pReassembledPacket == NULL" << endl;
					}
					//cout << *buffer_it->second << endl;

					// Set this to ture so that reassembly buffer is not deleted in the objects deconstructor
					// when we delete the memory below.
					buffer_it->second->BufferInUse(true);

					// Remove the entry from the list. Please note that the reassembled buffer is not deleted
					// when this entry is removed from the list. The buffer must be deleted by the caller since we
					// are returning the pointer to the buffer to the caller.
					delete buffer_it->second;
					fReassemblyBufferMap.erase(buffer_it);

					cout << "Packet reassembled" << endl;
					return BcmDsgBroadcastTunnelPktReassemblyBuffer::kAllFragmentsReceived;
				}
			}

			pReassembledPacket = NULL;
			return BcmDsgBroadcastTunnelPktReassemblyBuffer::kNotAllFragmentsReceived;		
		}
		else
		{
			// BT pkt is not segmented.
			pReassembledPacket = NULL;
			return BcmDsgBroadcastTunnelPktReassemblyBuffer::kNotFragmented;
		}
	}
  
	// ERROR - Failed to parse the buffer.
	cout << "ERROR - Failed to parse the buffer." << endl;
	return BcmDsgBroadcastTunnelPktReassemblyBuffer::kIllegalFragment;
}
	



/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
//
//****************************************************************************
//
//  Filename:       EstbCannedSnmp.cpp
//  Author:         Pinar Taskiran Cyr
//  Creation Date:  September 22, 2008
//
//****************************************************************************

//********************** Include Files ***************************************

#include	<sys/types.h>	/* basic system data types */
#if defined(WIN32)
#include	<WinSock2.h>	/* basic socket definitions */
#else
#include	<sys/socket.h>	/* basic socket definitions */
#endif
#ifdef TARGETOS_vxWorks
#include	<socklib.h>
#endif
#ifdef __linux__
#include	<sys/time.h>	/* timeval{} for select() */
#endif
#if !defined(WIN32)
#include	<sys/ioctl.h>
#include	<unistd.h>
#include	<netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include	<arpa/inet.h>	/* inet(3) functions */
#include	<net/if.h>
#endif
#include	<time.h>		/* timespec{} for pselect() */
#ifdef TARGETOS_vxWorks
#include	<inetLib.h>
#endif
#include	<iostream>
#include	<assert.h>
#include	<string.h>
  
// My api and definitions...
#include "MessageLog.h"
#include "EstbCannedSnmp.h"
#include "EstbCannedSnmpCommandTable.h"
//#include "EstbRemoteInterfaceStub.h" 
#include "dsg_api.h"


#include "OctetBuffer.h"


#if defined(WIN32)
#include <algorithm>
#else
#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <algorithm>
#else
#include <algo.h>
#endif
#endif
#include "ProgramStoreDeviceDriver.h"

#if defined(WIN32)
#define bzero(a, l) memset(a, 0, l)
#endif

//********************** Local Types *****************************************
#if defined(TARGETOS_vxWorks) || defined(WIN32)
typedef int socklen_t;
#endif

//********************** Local Constants *************************************

// Some application dependent defines. These would be replaced by the customer
#define SIGNED_IMAGE_FILENAME "signed_ecram_sto.bin"
#define SNMP_COMMUNITY "private"

/* SNMP types: */
/* ASN.1 class & form flags */
#define ASN_IDCF_MASK         0xE0 /* ID CLASS | ID FORM */
#define ASN_IDC_MASK          0xC0 /* ID CLASS */
#define ASN_IDF_MASK          0x20 /* ID FORM */
#define ASN_FLAG_MASK         0xE0

#ifndef ASN_UNIVERSAL		/* may be already defined in asn1.h */

#define ASN_UNIVERSAL         0x00
#define ASN_APPLICATION       0x40
#define ASN_CONTEXT           0x80
#define ASN_PRIVATE           0xC0
#define ASN_PRIMITIVE         0x00
#define ASN_CONSTRUCTOR       0x20
#define ASN_RESPONSE		  0x02
#define ASN_GET_OP			  0x00
#define ASN_SET_OP			  0x03

#define ASN_DEFAULT_SCOPE     ASN_CONTEXT

/* ASN.1 basic types: (UNIVERSAL scope) */
#define ASN_NO_ID             0x00
#define ASN_BOOLEAN           0x01
#define ASN_INTEGER           0x02
#define ASN_BITSTRING         0x03
#define ASN_OCTETSTRING       0x04
#define ASN_NULL              0x05
#define ASN_OBJECTID          0x06
#define ASN_SEQUENCE          0x10
#define ASN_SET               0x11
#else
#define ASN_OCTETSTRING       0x04
#define ASN_OBJECTID          0x06
#endif


/* Variable binding types are ASN.1 basic types w/ class & form flags applied */
#define VBT_NUMBER            ASN_INTEGER
#define VBT_STRING            ASN_OCTETSTRING
#define VBT_BITS              ASN_OCTETSTRING
#define VBT_OBJECT            ASN_OBJECTID
#define VBT_EMPTY             ASN_NULL
#define VBT_IPADDRESS        (ASN_APPLICATION | 0)
#define VBT_COUNTER          (ASN_APPLICATION | 1)
#define VBT_GAUGE            (ASN_APPLICATION | 2)
#define VBT_UNSIGNED32       (ASN_APPLICATION | 2)
#define VBT_TIMETICKS        (ASN_APPLICATION | 3)
#define VBT_OPAQUE           (ASN_APPLICATION | 4)
#define VBT_COUNTER64        (ASN_APPLICATION | 6)
#define VBT_UINTEGER32       (ASN_APPLICATION | 7)


//********************** Local Variables *************************************

//********************** Global Variables *************************************
// Set to NULL. Will be set up by constructor.
BcmEstbCannedSnmp *BcmEstbCannedSnmp::pfSingletonInstance = NULL;

//********************** Local Variables *************************************

//********************** Local Functions *************************************

//****************** Extern/Local Functions prototypes ************************

extern "C"
{

}

//********************** SNMP Helper Methods *********************************
static uint16 AddTL(BcmOctetBuffer &buffer, uint8 t);
static void CloseTLV(BcmOctetBuffer &buffer, uint16 offset);
static void AddTLV(BcmOctetBuffer &buffer, uint8 t, uint8 l, uint8 *v);
static bool SkipTL(BcmOctetBuffer &buffer, uint &offset, uint8 tag);
static bool SkipIntTLV(BcmOctetBuffer &buffer, uint &offset, uint8 len, uint16 val);
static bool GetIntTLV(BcmOctetBuffer &buffer, uint &offset, uint8 len, uint16 &v2);
static bool GetIpAddressTLV(BcmOctetBuffer &buffer, uint &offset, uint8 len, uint32 &v4);
static bool SkipOctetStringTLV(BcmOctetBuffer &buffer, uint &offset, uint8 tag, uint8 len, uint8* val);
static bool GetOctetStringTLV(BcmOctetBuffer &buffer, uint &offset, uint8 tag, BcmOctetBuffer &val);
static uint16 ParseVersionString(BcmOctetBuffer &version, uint &offset);
static void AddSNMPHeader(BcmOctetBuffer &packet, uint8 operation, uint16 rid, uint16 &seq1, uint16 &seq2, uint16 &seq3, uint16 &seq4);
static void CloseSNMPSocket(int sock);
static uint16 SendSNMPPacketAndWaitForReply(BcmOctetBuffer &packet);
static uint16 VerifySNMPResult(BcmOctetBuffer &packet, uint16 rid, uint &offset);

//********************** Class Method Implementations ************************

// BcmEstbCannedSnmp() - Object constructor
//		Initiates thread and initializes singleton.
//
// Parameters:
//      initialPriority - Initial thread priority.
//
// Returns:
//      None.
//
BcmEstbCannedSnmp::BcmEstbCannedSnmp():
fMyIpAddress(0)
{
    // Override the class name given by my parent.
    fMessageLogSettings.SetModuleName("BcmEstbCannedSnmp");

    // Set up the singleton.
    if (pfSingletonInstance != NULL)
    {
        gWarningMsg(fMessageLogSettings, "BcmEstbCannedSnmp")
            << "Singleton pointer is not NULL!  There are multiple BcmEstbCannedSnmp!  Leaving the singleton pointer alone..." << endl;
    }
    else
    {
        gInfoMsg(fMessageLogSettings, "BcmEstbCannedSnmp")
            << "Setting up singleton pointer." << endl;

        pfSingletonInstance = this;
    }

	#if ( BCM_CONSOLE_SUPPORT )
    // Register myself with the command table.
    GetCommandTableSingleton().AddInstance("ESTB CANNED SNMP", this);
	#endif

	// Initialize the SNMP request id sequence
	fRequestId = 1234;
}

// ~BcmEstbCannedSnmp() - Object constructor
//		Signals thread and waits for termination.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
BcmEstbCannedSnmp::~BcmEstbCannedSnmp()
{
    gFuncEntryExitMsg( fMessageLogSettings, "~BcmEstbCannedSnmp" ) << "Entering..." << endl;    

    gFuncEntryExitMsg(fMessageLogSettings, "~BcmEstbCannedSnmp") << "Exiting..." << endl;
}


// AddTL(BcmOctetBuffer &buffer, uint8 t)
//		Adds the type and length part of a TLV to the end of a buffer and returns
//		the offset of the data part. The length is stored as 0 since we don't know
//		what it is yet. A call to CloseTLV with the returned offset will set the
//		length part.
//
// Parameters:
//      buffer - Buffer for building TLVs.
//		t - The type value for this TLV
//
// Returns:
//      Offset to the value portion of the TLV.
//
static uint16 AddTL(BcmOctetBuffer &buffer, uint8 t)
{
	// Add the type octet
	buffer.AddToEnd(t);
	// Add placeholder for the length octet
	buffer.AddToEnd((uint8)0);
	// Return the offset to the value part
	return buffer.BytesUsed();
}

// CloseTLV(BcmOctetBuffer &buffer, uint16 offset)
//		Assume that we have just finished storing the value portion of a TLV
//		started using the AddTL call. Calculate the length part of the TLV and
//		store it.
//
// Parameters:
//      buffer - Buffer for building TLVs.
//		offset - offset in buffer of the start of the value portion of the TLV.
//
// Returns:
//      None.
//
static void CloseTLV(BcmOctetBuffer &buffer, uint16 offset)
{
	// Calculate the value part's length and store it in the length octet
	buffer.Overwrite(offset - 1, (uint8)(buffer.BytesUsed() - offset));
}

// AddTLV(BcmOctetBuffer &buffer, uint8 t, uint8 l, uint8 *v)
//		Adds the type, length, and value parts of a TLV to the end of a buffer.
//
// Parameters:
//      buffer - Buffer for building TLVs.
//		t - The type for this TLV
//		l - The length for this TLV
//		v - Pointer to the value for this TLV
//
// Returns:
//      None.
//
static void AddTLV(BcmOctetBuffer &buffer, uint8 t, uint8 l, uint8 *v)
{
	// Add the type octet
	buffer.AddToEnd(t);
	// Add the length octet
	buffer.AddToEnd(l);
	// Add the value octets.
	if (l)
		buffer.AddToEnd(v, l);
}

// SkipTL(BcmOctetBuffer &buffer, uint &offset, uint8 tag)
//		Increment the offset past the type and length parts of the TLV.
//		Verify the type part matches the specified tag.
//
// Parameters:
//      buffer - Buffer for building TLVs.
//		offset - At entry this is the offset of the type part of the TLV. At exit,
//				this is the offset to the value part of the TLV.
//		tag - Value expected in the type part of the TLV
//
// Returns:
//      True - All is well.
//		False - Either the type did not match or the TLV is incomplete.
//
static bool SkipTL(BcmOctetBuffer &buffer, uint &offset, uint8 tag)
{
	// Check for enough room for the type and length parts
	if ((buffer.BytesUsed() - offset) < 2)
		return false;
	// Extract and verify the type octet
	uint8 t;
	buffer.Read(offset, t);
	if (t != tag)
		return false;
	// Increment past the length octet
	offset++;
	return true;
}

// SkipIntTLV(BcmOctetBuffer &buffer, uint &offset, uint8 len, uint16 val)
//		Increment the offset past the TLV.
//		Verify the type is VBT_NUMBER, and that the length and value match the parameter
//		specified values.
//
// Parameters:
//      buffer - Buffer for building TLVs.
//		offset - At entry this is the offset of the type part of the TLV. At exit,
//				this is the offset to the value part of the TLV.
//		len - Value expected in the length part of the TLV
//		val - Value expected in the value part of the TLV
//
// Returns:
//      True - All is well.
//		False - Either the type, length or value did not match or the TLV is incomplete.
//
static bool SkipIntTLV(BcmOctetBuffer &buffer, uint &offset, uint8 len, uint16 val)
{
	// Check for enough room for the type and length parts
	if ((buffer.BytesUsed() - offset) < 2)
		return false;
	// Extract and verify the type octet
	uint8 t;
	buffer.Read(offset, t);
	if (t != VBT_NUMBER)
		return false;
	// Extract and verify the length octet
	buffer.Read(offset, t);
	if (t != len)
		return false;
	// Make sure there's enough left in the buffer to contain the value part
	if ((buffer.BytesUsed() - offset) < t)
		return false;
	// Extract and verify one or two octets depending on the length. We don't support
	// integer values of size greater than 2.
	uint8 v1;
	uint16 v2;
	switch (t) {
		case 1:
			buffer.Read(offset, v1);
			v2 = v1;
			break;
		case 2:
			buffer.Read(offset, v2);
			break;
		default:
			return false;
	}
	return v2 == val;
}

// GetIntTLV(BcmOctetBuffer &buffer, uint &offset, uint8 len, uint16 &v2)
//		Increment the offset past the TLV.
//		Verify the type is VBT_NUMBER, and that the length match the parameter
//		specified values. Extract and return the actual value of the integer TLV.
//
// Parameters:
//      buffer - Buffer for building TLVs.
//		offset - At entry this is the offset of the type part of the TLV. At exit,
//				this is the offset to the value part of the TLV.
//		len - Value expected in the length part of the TLV
//		v - 16 bit integer reference set to the TLVs value on exit.
//
// Returns:
//      True - All is well.
//		False - Either the type, length or value did not match or the TLV is incomplete.
//
static bool GetIntTLV(BcmOctetBuffer &buffer, uint &offset, uint8 len, uint16 &v2)
{
	// Check for enough room for the type and length parts
	if ((buffer.BytesUsed() - offset) < 2)
		return false;
	// Extract and verify the type octet
	uint8 t;
	buffer.Read(offset, t);
	if (t != VBT_NUMBER)
		return false;
	// Extract and verify the length octet
	buffer.Read(offset, t);
	if (t != len)
		return false;
	// Make sure there's enough left in the buffer to contain the value part
	if ((buffer.BytesUsed() - offset) < t)
		return false;
	// Extract and return one or two octets depending on the length. We don't support
	// integer values of size greater than 2.
	uint8 v1;
	switch (t) {
		case 1:
			buffer.Read(offset, v1);
			v2 = v1;
			break;
		case 2:
			buffer.Read(offset, v2);
			break;
		default:
			return false;
	}
	return true;
}

// GetIntTLV(BcmOctetBuffer &buffer, uint &offset, uint8 len, uint16 &v2)
//		Increment the offset past the TLV.
//		Verify the type is VBT_NUMBER, and that the length match the parameter
//		specified values. Extract and return the actual value of the integer TLV.
//
// Parameters:
//      buffer - Buffer for building TLVs.
//		offset - At entry this is the offset of the type part of the TLV. At exit,
//				this is the offset to the value part of the TLV.
//		len - Value expected in the length part of the TLV
//		v - 16 bit integer reference set to the TLVs value on exit.
//
// Returns:
//      True - All is well.
//		False - Either the type, length or value did not match or the TLV is incomplete.
//
static bool GetIpAddressTLV(BcmOctetBuffer &buffer, uint &offset, uint8 len, uint32 &v4)
{
	// Check for enough room for the type and length parts
	if ((buffer.BytesUsed() - offset) < 2)
		return false;
	// Extract and verify the type octet
	uint8 t;
	buffer.Read(offset, t);
	if (t != VBT_IPADDRESS)
		return false;
	// Extract and verify the length octet
	buffer.Read(offset, t);
	
	if (t != len)
		return false;

	// Make sure there's enough left in the buffer to contain the value part
	if ((buffer.BytesUsed() - offset) < t)
		return false;

	// Extract and return four octets.
	switch (t) {
		case 4:
			buffer.Read(offset, v4);
			break;

		default:
			return false;
	}
	return true;
}

// SkipOctetStringTLV(BcmOctetBuffer &buffer, uint &offset, uint8 tag, uint8 len, uint8* val)
//		Increment the offset past the TLV.
//		Verify the type, length and value match the parameter specified values.
//
// Parameters:
//      buffer - Buffer for building TLVs.
//		offset - At entry this is the offset of the type part of the TLV. At exit,
//				this is the offset to the value part of the TLV.
//		len - Value expected in the length part of the TLV
//		val - Pointer to the octet string we expect to find in the value part of the TLV
//
// Returns:
//      True - All is well.
//		False - Either the type, length or value did not match or the TLV is incomplete.
//			NOTE: In the failure case, the offset is not necessarily pointing to the next
//				TLV
//
static bool SkipOctetStringTLV(BcmOctetBuffer &buffer, uint &offset, uint8 tag, uint8 len, uint8* val)
{
	// Check for enough room for the type and length parts
	if ((buffer.BytesUsed() - offset) < 2)
		return false;
	// Extract and verify the type octet
	uint8 t;
	buffer.Read(offset, t);
	if (t != tag)
		return false;
	// Extract and verify the length octet
	buffer.Read(offset, t);
	if (t != len)
		return false;
	// Make sure there's enough left in the buffer to contain the value part
	if ((buffer.BytesUsed() - offset) < t)
		return false;
	// verify the value part
	uint8 v;
	while (t--) {
		buffer.Read(offset, v);
		if (v != *val++)
			return false;
	}
	return true;
}

// GetOctetStringTLV(BcmOctetBuffer &buffer, uint &offset, uint8 tag, BcmOctetBuffer &val)
//		Increment the offset past the TLV.
//		Verify the type, length match the parameter specified values.
//		Return the contents of the value part of the TLV.
//
// Parameters:
//      buffer - Buffer for building TLVs.
//		offset - At entry this is the offset of the type part of the TLV. At exit,
//				this is the offset to the value part of the TLV.
//		len - Value expected in the length part of the TLV
//		val - On exit buffer contains the value part of the TLV
//
// Returns:
//      True - All is well.
//		False - Either the type or length did not match or the TLV is incomplete.
//			NOTE: In the failure case, the offset is not necessarily pointing to the next
//				TLV
//
static bool GetOctetStringTLV(BcmOctetBuffer &buffer, uint &offset, uint8 tag, BcmOctetBuffer &val)
{
	// Check for enough room for the type and length parts
	if ((buffer.BytesUsed() - offset) < 2)
		return false;
	// Extract and verify the type octet
	uint8 t;
	buffer.Read(offset, t);
	if (t != tag)
		return false;
	// Extract and verify the length octet
	buffer.Read(offset, t);
	if (val.BufferSize() < t)
		return false;
	// Make sure there's enough left in the buffer to contain the value part
	if ((buffer.BytesUsed() - offset) < t)
		return false;
	// Extract the value part of the TLV and stuff it in the buffer reference parameter.
	uint8 v;
	val.Empty();
	while (t--) {
		buffer.Read(offset, v);
		val.AddToEnd(v);
	}
	return true;
}

// ParseVersionString(BcmOctetBuffer &version, uint &offset)
//		Return binary representation of ascii digits found in buffer at offset. The
//		ascii digit are either terminated at end of buffer or by an ascii period (.).
//		The offset is adjusted to point past the parsed string.
//
// Parameters:
//      buffer - Buffer containing the ascii string.
//		offset - At entry this is the offset to the start of the ascii string. At exit,
//				this is the offset past the end of the parsed digits.
//
// Returns:
//      16 bit binary representation of the parsed digits.
//
static uint16 ParseVersionString(BcmOctetBuffer &version, uint &offset)
{
	uint16 result = 0;
	while (offset < version.BytesUsed()) {
		uint8 digit;
		version.Read(offset, digit);
		if (digit == '.')
			break;
		result = (result * 10) + (digit - '0');
	}
	return result;
}

// AddSNMPHeader(BcmOctetBuffer &packet, uint8 operation, uint16 &seq1, uint16 &seq2, uint16 &seq3, uint16 &seq4)
//		Add the constant parts of an SNMP GET or SET message.
//
// Parameters:
//      packet - Buffer containing the ascii string.
//		operation - GET or SET operation.
//		rid - Reference to the request id counter.
//		seq1, seq2, seq3, seq4 - On exit, these contain the offset of the value parts of the four
//			sequence TLVs that need to be closed to complete the packet.
//
// Returns:
//      none.
//
static void AddSNMPHeader(BcmOctetBuffer &packet, uint8 operation, uint16 rid, uint16 &seq1, uint16 &seq2, uint16 &seq3, uint16 &seq4)
{
	// Start with nothing
	packet.Empty();
	// Add the header sequence TLV
	seq1 = AddTL(packet, ASN_CONSTRUCTOR | ASN_SEQUENCE);
	// Add the SNMP Version 2C tlv
	uint8 one = 1;
	AddTLV(packet, VBT_NUMBER, sizeof(one), &one);
	// Add the community string
	AddTLV(packet, VBT_STRING, strlen(SNMP_COMMUNITY), (uint8*)SNMP_COMMUNITY);
	// Add the context TLV
	seq2 = AddTL(packet, ASN_CONSTRUCTOR | ASN_CONTEXT | operation);
	// Add the request id
	AddTLV(packet, VBT_NUMBER, 2, (uint8*)&rid);
	// Add the error status and index
	uint8 zero = 0;
	AddTLV(packet, VBT_NUMBER, 1, &zero);
	AddTLV(packet, VBT_NUMBER, 1, &zero);
	// Start the the object sequence
	seq3 = AddTL(packet, ASN_CONSTRUCTOR | ASN_SEQUENCE);
	// Start the the OID value pair
	seq4 = AddTL(packet, ASN_CONSTRUCTOR | ASN_SEQUENCE);
}

static void CloseSNMPSocket(int sock)
{
#if defined(WIN32)
	closesocket(sock);
#else
	close(sock);
#endif
}

// SendSNMPPacketAndWaitForReply(BcmOctetBuffer &packet)
//		Open a UDP socket to the eCMs SNMP port, send the SNMP GET/SET packet and wait for
//		the response. We retry three times since this is a UDP protocol and delivery is not
//		guaranteed.
//
// Parameters:
//      packet - On entry, this contains the SNMP packet to be sent. On exit this contains
//			the SNMP response packet.
//
// Returns:
//      0 - All is well
//		>0 - Error code
//
static uint16 SendSNMPPacketAndWaitForReply(BcmOctetBuffer &packet)
{
	BcmOctetBuffer sendPacket = packet;
	// Create a socket
	int	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		return 1;
	}
	// Bind it to any available eSTB port
	struct sockaddr_in	addr;
	bzero((char *)&addr, sizeof(addr));
	addr.sin_family      = AF_INET;
	addr.sin_port        = htons(0); // any
#if defined(USE_PRIVATE_NETWORK)
	addr.sin_addr.s_addr = htonl(kLocalRxIpAddress);  // use my IP address
#else
	addr.sin_addr.s_addr = htonl(0);  // use any address
#endif
	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		CloseSNMPSocket(sockfd);
		return 4;
	}
	uint8 retries = 3;

	while (retries --)
	{
		// Send the packet to the eCM's SNMP port
		bzero((char *)&addr, sizeof(addr));
		addr.sin_family      = AF_INET;
		addr.sin_port        = htons(161); // SNMP
		addr.sin_addr.s_addr = htonl(0xC0A86401); // 192.168.100.1
		if (sendto(sockfd, (char *)packet.AccessBuffer(), sendPacket.BytesUsed(), 0, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
			CloseSNMPSocket(sockfd);
			return 2;
		}
		// Try to receive the response. We allow 1 second before timeout
		packet.SetBytesUsed(1024);
		socklen_t sLen = sizeof(addr);
		timeval t;
		t.tv_sec = 1;
		t.tv_usec = 0;
		setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&t, (socklen_t)sizeof(t));
		int l = recvfrom(sockfd, (char *)packet.AccessBuffer(), packet.BytesUsed(), 0, (struct sockaddr *) &addr, &sLen);
		if (l < 0) {
			if (retries)
				continue;
			CloseSNMPSocket(sockfd);
			return 3;
		}
		packet.SetBytesUsed(l);
		CloseSNMPSocket(sockfd);
		return 0;
	}
	CloseSNMPSocket(sockfd);
	return 4;
}

// VerifySNMPResult(BcmOctetBuffer &packet, uint16 &rid, uint &offset)
//		Verify the constant part of the response.
//
// Parameters:
//      packet - Buffer containing the SNMP response.
//		rid - Expected request id counter.
//		Offset - Offset to the remainder of the packet after parsing the constant header part.
//
// Returns:
//      0 - All is well.
//		>0 - Return code
//
static uint16 VerifySNMPResult(BcmOctetBuffer &packet, uint16 rid, uint &offset)
{
	if (!SkipTL(packet, offset, ASN_CONSTRUCTOR | ASN_SEQUENCE)) {
		return 1;
	}
	if (!SkipIntTLV(packet, offset, 1, 1)) {
		return 2;
	}
	if (!SkipOctetStringTLV(packet, offset, VBT_STRING, strlen(SNMP_COMMUNITY), (uint8*)SNMP_COMMUNITY)) {
		return 3;
	}
	if (!SkipTL(packet, offset, ASN_CONSTRUCTOR | ASN_CONTEXT | ASN_RESPONSE)) {
		return 4;
	}
	if (!SkipIntTLV(packet, offset, 2, rid)) {
		return 5;
	}
	if (!SkipIntTLV(packet, offset, 1, 0)) {
		return 6;
	}
	if (!SkipIntTLV(packet, offset, 1, 0)) {
		return 7;
	}
	if (!SkipTL(packet, offset, ASN_CONSTRUCTOR | ASN_SEQUENCE)) {
		return 8;
	}
	if (!SkipTL(packet, offset, ASN_CONSTRUCTOR | ASN_SEQUENCE)) {
		return 9;
	}
	return 0;
}

// Set_docsDevSwFilename(const char *fileName)
//		Create, send, and verify and SNMP SET for the docsDevSwFilename MIB.
//
// Parameters:
//      fileName - Pointer to null terminated file name ascii string.
//
// Returns:
//      true - All is well
//		False - Something didn't work
//
bool BcmEstbCannedSnmp::Set_docsDevSwFilename(const char *fileName)
{
	BcmOctetBuffer packet(1024);
	uint16 seq1, seq2, seq3, seq4;

	// build a SNMP GET packet for the docsDevSwCurrentVers.0 MIB
	AddSNMPHeader(packet, ASN_SET_OP, fRequestId, seq1, seq2, seq3, seq4);

	// Add the OID
	uint8 docsDevSwFilenameOid[] = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x45, 0x01, 0x03, 0x02, 0x00};
	AddTLV(packet, VBT_OBJECT, 10, docsDevSwFilenameOid);

	// Add the value
	AddTLV(packet, VBT_STRING, strlen(fileName), (uint8*)fileName);

	CloseTLV(packet, seq1);
	CloseTLV(packet, seq2);
	CloseTLV(packet, seq3);
	CloseTLV(packet, seq4);

	uint16 result = SendSNMPPacketAndWaitForReply(packet);
	if (result) {
		gErrorMsg(fMessageLogSettings, "Set_docsDevSwFilename")
			<< "Unable to complete SNMP operation, result = " << result << endl;
		return false;
	}

	uint offset = 0;
	result = VerifySNMPResult(packet, fRequestId++, offset);
	if (result) {
		gErrorMsg(fMessageLogSettings, "Set_docsDevSwFilename")
			<< "Unable to verify SNMP operation, result = " << result << endl;
		return false;
	}
	return true;
}

// Get_docsDevSwFilename(const char *fileName)
//		Create, send, and verify and SNMP SET for the docsDevSwFilename MIB.
//
// Parameters:
//      fileName - Pointer to null terminated file name ascii string.
//
// Returns:
//      true - All is well
//		False - Something didn't work
//
bool BcmEstbCannedSnmp::Get_docsDevSwFilename(char *fileName)
{
	BcmOctetBuffer packet(1024);
	uint16 seq1, seq2, seq3, seq4;

	// build a SNMP GET packet for the docsDevSwCurrentVers.0 MIB
	AddSNMPHeader(packet, ASN_GET_OP, fRequestId, seq1, seq2, seq3, seq4);

	// Add the OID
	uint8 docsDevSwFilenameOid[] = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x45, 0x01, 0x03, 0x02, 0x00};
	AddTLV(packet, VBT_OBJECT, 10, docsDevSwFilenameOid);

	// Add the value
	AddTLV(packet, VBT_EMPTY, 0, (uint8*)0);

	CloseTLV(packet, seq1);
	CloseTLV(packet, seq2);
	CloseTLV(packet, seq3);
	CloseTLV(packet, seq4);

	uint16 result = SendSNMPPacketAndWaitForReply(packet);
	if (result) {
		gErrorMsg(fMessageLogSettings, "Get_docsDevSwFilename")
			<< "Unable to complete SNMP operation, result = " << result << endl;
		return false;
	}

	uint offset = 0;
	result = VerifySNMPResult(packet, fRequestId++, offset);
	if (result) {
		gErrorMsg(fMessageLogSettings, "Get_docsDevSwFilename")
			<< "Unable to verify SNMP operation, result = " << result << endl;
		return false;
	}

	BcmOctetBuffer val(256);
	if (!SkipOctetStringTLV(packet, offset, VBT_OBJECT, 10, docsDevSwFilenameOid)) {
		gErrorMsg(fMessageLogSettings, "Get_docsDevSwFilename")
			<< "Unable to parse received SNMP packet, bad OID" << endl;
		return false;
	}

	// Finally get the string
	BcmOctetBuffer swFileName(32);
	if (!GetOctetStringTLV(packet, offset, VBT_STRING, swFileName)) {
		gErrorMsg(fMessageLogSettings, "Get_docsDevSwFilename")
			<< "Unable to parse received SNMP packet, bad version" << endl;
		return false;
	}

	memset(fileName, 0, sizeof(fileName));
	memcpy(fileName, (char*) swFileName.AccessBuffer(), swFileName.BytesUsed());

	return true;
}




// SetServerIpAddress()
//		Create, send, and verify and SNMP SET for the docsDevSwServer MIB. The
//		IP address used will always be 192.168.100.1 since this is the standard
//		address used by the eCM for early SNMP access.
//
// Parameters:
//      none.
//
// Returns:
//      true - All is well
//		False - Something didn't work
//
bool BcmEstbCannedSnmp::Set_docsDevSwServer(unsigned long ipaddress)
{
	BcmOctetBuffer packet(1024);
	uint16 seq1, seq2, seq3, seq4;

	// build a SNMP GET packet for the docsDevSwCurrentVers.0 MIB
	AddSNMPHeader(packet, ASN_SET_OP, fRequestId, seq1, seq2, seq3, seq4);

	// Add the OID
	uint8 docsDevSwServerOid[] = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x45, 0x01, 0x03, 0x01, 0x00};
	AddTLV(packet, VBT_OBJECT, 10, docsDevSwServerOid);

	// Add the value
	AddTLV(packet, VBT_IPADDRESS, sizeof(ipaddress), (uint8*)&ipaddress);

	CloseTLV(packet, seq1);
	CloseTLV(packet, seq2);
	CloseTLV(packet, seq3);
	CloseTLV(packet, seq4);

	uint16 result = SendSNMPPacketAndWaitForReply(packet);
	if (result) {
		gErrorMsg(fMessageLogSettings, "Set_docsDevSwServer")
			<< "Unable to complete SNMP operation, result = " << result << endl;
		return false;
	}

	uint offset = 0;
	result = VerifySNMPResult(packet, fRequestId++, offset);
	if (result) {
		gErrorMsg(fMessageLogSettings, "Set_docsDevSwServer")
			<< "Unable to verify SNMP operation, result = " << result << endl;
		return false;
	}
	return true;
}

// Get_docsDevSwFilename(const char *fileName)
//		Create, send, and verify and SNMP SET for the docsDevSwFilename MIB.
//
// Parameters:
//      fileName - Pointer to null terminated file name ascii string.
//
// Returns:
//      true - All is well
//		False - Something didn't work
//
bool BcmEstbCannedSnmp::Get_docsDevSwServer(unsigned long &ipaddress)
{
	BcmOctetBuffer packet(1024);
	uint16 seq1, seq2, seq3, seq4;

	// build a SNMP GET packet for the docsDevSwCurrentVers.0 MIB
	AddSNMPHeader(packet, ASN_GET_OP, fRequestId, seq1, seq2, seq3, seq4);

	// Add the OID
	uint8 docsDevSwServerOid[] = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x45, 0x01, 0x03, 0x01, 0x00};
	AddTLV(packet, VBT_OBJECT, 10, docsDevSwServerOid);

	// Add the value
	AddTLV(packet, VBT_EMPTY, 0, (uint8*)0);

	CloseTLV(packet, seq1);
	CloseTLV(packet, seq2);
	CloseTLV(packet, seq3);
	CloseTLV(packet, seq4);

	uint16 result = SendSNMPPacketAndWaitForReply(packet);
	if (result) {
		gErrorMsg(fMessageLogSettings, "Get_docsDevSwServer")
			<< "Unable to complete SNMP operation, result = " << result << endl;
		return false;
	}

	uint offset = 0;
	result = VerifySNMPResult(packet, fRequestId++, offset);
	if (result) {
		gErrorMsg(fMessageLogSettings, "Get_docsDevSwServer")
			<< "Unable to verify SNMP operation, result = " << result << endl;
		return false;
	}

	BcmOctetBuffer val(256);
	if (!SkipOctetStringTLV(packet, offset, VBT_OBJECT, 10, docsDevSwServerOid)) {
		gErrorMsg(fMessageLogSettings, "Get_docsDevSwServer")
			<< "Unable to parse received SNMP packet, bad OID" << endl;
		return false;
	}

	// Finally get the status
	if (!GetIpAddressTLV(packet, offset, 4, ipaddress)) {
		gErrorMsg(fMessageLogSettings, "Get_docsDevSwServer")
			<< "Unable to parse received SNMP packet, bad status" << endl;
		return false;
	}

	return true;
}



// Set_docsDevSwAdminStatus()
//		Create, send, and verify and SNMP SET for the docsDevSwAdminStatus MIB. The
//		MIB will always be set to one since this is the value we need to initiate a
//		secure software download.
//
// Parameters:
//      none.
//
// Returns:
//      true - All is well
//		False - Something didn't work
//
bool BcmEstbCannedSnmp::Set_docsDevSwAdminStatus(uint8 status)
{
	BcmOctetBuffer packet(1024);
	uint16 seq1, seq2, seq3, seq4;

	// build a SNMP GET packet for the docsDevSwCurrentVers.0 MIB
	AddSNMPHeader(packet, ASN_SET_OP, fRequestId, seq1, seq2, seq3, seq4);

	// Add the OID
	uint8 docsDevSwAdminStatusOid[] = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x45, 0x01, 0x03, 0x03, 0x00};
	AddTLV(packet, VBT_OBJECT, 10, docsDevSwAdminStatusOid);

	// Add the value
	AddTLV(packet, VBT_NUMBER, sizeof(status), &status);

	CloseTLV(packet, seq1);
	CloseTLV(packet, seq2);
	CloseTLV(packet, seq3);
	CloseTLV(packet, seq4);

	uint16 result = SendSNMPPacketAndWaitForReply(packet);
	if (result) {
		gErrorMsg(fMessageLogSettings, "Set_docsDevSwAdminStatus")
			<< "Unable to complete SNMP operation, result = " << result << endl;
		return false;
	}

	uint offset = 0;
	result = VerifySNMPResult(packet, fRequestId++, offset);
	if (result) {
		gErrorMsg(fMessageLogSettings, "Set_docsDevSwAdminStatus")
			<< "Unable to verify SNMP operation, result = " << result << endl;
		return false;
	}
	return true;
}

// Get_docsDevSwAdminStatus(uint16 &status)
//		Create, send, and verify and SNMP GET to retrieve the docsDevSwOperStatus MIB.
//
// Parameters:
//      status - The mib value returned.
//
// Returns:
//      true - All is well
//		False - Something didn't work
//
bool BcmEstbCannedSnmp::Get_docsDevSwAdminStatus(uint16 &status)
{
	BcmOctetBuffer packet(1024);
	uint16 seq1, seq2, seq3, seq4;

	// build a SNMP GET packet for the docsDevSwCurrentVers.0 MIB
	AddSNMPHeader(packet, ASN_GET_OP, fRequestId, seq1, seq2, seq3, seq4);

	// Add the OID
	uint8 docsDevSwAdminStatusOid[] = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x45, 0x01, 0x03, 0x03, 0x00};
	AddTLV(packet, VBT_OBJECT, 10, docsDevSwAdminStatusOid);

	// Add the value
	AddTLV(packet, VBT_EMPTY, 0, (uint8*)0);

	CloseTLV(packet, seq1);
	CloseTLV(packet, seq2);
	CloseTLV(packet, seq3);
	CloseTLV(packet, seq4);

	uint16 result = SendSNMPPacketAndWaitForReply(packet);
	if (result) {
		gErrorMsg(fMessageLogSettings, "Get_docsDevSwAdminStatus")
			<< "Unable to complete SNMP operation, result = " << result << endl;
		return false;
	}

	uint offset = 0;
	result = VerifySNMPResult(packet, fRequestId++, offset);
	if (result) {
		gErrorMsg(fMessageLogSettings, "Get_docsDevSwAdminStatus")
			<< "Unable to verify SNMP operation, result = " << result << endl;
		return false;
	}
	BcmOctetBuffer val(256);
	if (!SkipOctetStringTLV(packet, offset, VBT_OBJECT, 10, docsDevSwAdminStatusOid)) {
		gErrorMsg(fMessageLogSettings, "Get_docsDevSwAdminStatus")
			<< "Unable to parse received SNMP packet, bad OID" << endl;
		return false;
	}
	// Finally get the status
	if (!GetIntTLV(packet, offset, 1, status)) {
		gErrorMsg(fMessageLogSettings, "Get_docsDevSwAdminStatus")
			<< "Unable to parse received SNMP packet, bad status" << endl;
		return false;
	}
	return true;
}

// Set_docsDevSwOperStatus()
//		Create, send, and verify and SNMP SET for the docsDevSwOperStatus MIB. The
//		MIB will always be set to one since this is the value we need to initiate a
//		secure software download.
//
// Parameters:
//      none.
//
// Returns:
//      true - All is well
//		False - Something didn't work
//
bool BcmEstbCannedSnmp::Set_docsDevSwOperStatus(uint8 status)
{
	BcmOctetBuffer packet(1024);
	uint16 seq1, seq2, seq3, seq4;

	// build a SNMP GET packet for the docsDevSwCurrentVers.0 MIB
	AddSNMPHeader(packet, ASN_SET_OP, fRequestId, seq1, seq2, seq3, seq4);

	// Add the OID
	uint8 docsDevSwOperStatusOid[] = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x45, 0x01, 0x03, 0x04, 0x00};
	AddTLV(packet, VBT_OBJECT, 10, docsDevSwOperStatusOid);

	// Add the value
	AddTLV(packet, VBT_NUMBER, sizeof(status), &status);

	CloseTLV(packet, seq1);
	CloseTLV(packet, seq2);
	CloseTLV(packet, seq3);
	CloseTLV(packet, seq4);

	uint16 result = SendSNMPPacketAndWaitForReply(packet);
	if (result) {
		gErrorMsg(fMessageLogSettings, "Set_docsDevSwOperStatus")
			<< "Unable to complete SNMP operation, result = " << result << endl;
		return false;
	}

	uint offset = 0;
	result = VerifySNMPResult(packet, fRequestId++, offset);
	if (result) {
		gErrorMsg(fMessageLogSettings, "Set_docsDevSwOperStatus")
			<< "Unable to verify SNMP operation, result = " << result << endl;
		return false;
	}
	return true;
}

// Get_docsDevSwOperStatus(uint16 &status)
//		Create, send, and verify and SNMP GET to retrieve the docsDevSwOperStatus MIB.
//
// Parameters:
//      status - The mib value returned.
//
// Returns:
//      true - All is well
//		False - Something didn't work
//
bool BcmEstbCannedSnmp::Get_docsDevSwOperStatus(uint16 &status)
{
	BcmOctetBuffer packet(1024);
	uint16 seq1, seq2, seq3, seq4;

	// build a SNMP GET packet for the docsDevSwCurrentVers.0 MIB
	AddSNMPHeader(packet, ASN_GET_OP, fRequestId, seq1, seq2, seq3, seq4);

	// Add the OID
	uint8 docsDevSwOperStatusOid[] = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x45, 0x01, 0x03, 0x04, 0x00};
	AddTLV(packet, VBT_OBJECT, 10, docsDevSwOperStatusOid);

	// Add the value
	uint8 one = 1;
	AddTLV(packet, VBT_EMPTY, 0, (uint8*)0);

	CloseTLV(packet, seq1);
	CloseTLV(packet, seq2);
	CloseTLV(packet, seq3);
	CloseTLV(packet, seq4);

	uint16 result = SendSNMPPacketAndWaitForReply(packet);
	if (result) {
		gErrorMsg(fMessageLogSettings, "Get_docsDevSwOperStatus")
			<< "Unable to complete SNMP operation, result = " << result << endl;
		return false;
	}

	uint offset = 0;
	result = VerifySNMPResult(packet, fRequestId++, offset);
	if (result) {
		gErrorMsg(fMessageLogSettings, "Get_docsDevSwOperStatus")
			<< "Unable to verify SNMP operation, result = " << result << endl;
		return false;
	}
	BcmOctetBuffer val(256);
	if (!SkipOctetStringTLV(packet, offset, VBT_OBJECT, 10, docsDevSwOperStatusOid)) {
		gErrorMsg(fMessageLogSettings, "Get_docsDevSwOperStatus")
			<< "Unable to parse received SNMP packet, bad OID" << endl;
		return false;
	}
	// Finally get the status
	if (!GetIntTLV(packet, offset, 1, status)) {
		gErrorMsg(fMessageLogSettings, "Get_docsDevSwOperStatus")
			<< "Unable to parse received SNMP packet, bad status" << endl;
		return false;
	}
	return true;
}

// Get_docsDevSwCurrentVers(uint16 &major, uint16 &minor, uint16 &subMinor)
//		Using SNMP, retrieve the version of the image currently running in the eCM
//
// Parameters:
//      major - Major version number.
//      minor - Minor version number.
//      subMinor - Minor sub-version number.
//
// Returns:
//      None. If fails then major, minor and subMinor are returned as 0
//
bool BcmEstbCannedSnmp::Get_docsDevSwCurrentVers(uint16 &major, uint16 &minor, uint16 &subMinor)
{
	major = 0;
	minor = 0;
	subMinor = 0;

	BcmOctetBuffer packet(1024);
	uint16 seq1, seq2, seq3, seq4;

	// build a SNMP GET packet for the docsDevSwCurrentVers.0 MIB
	AddSNMPHeader(packet, ASN_GET_OP, fRequestId, seq1, seq2, seq3, seq4);

	// Add the OID
	uint8 docsDevSwCurrentVersOid[] = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x45, 0x01, 0x03, 0x05, 0x00};
	AddTLV(packet, VBT_OBJECT, 10, docsDevSwCurrentVersOid);

	// Add the value
	AddTLV(packet, VBT_EMPTY, 0, (uint8*)0);
	CloseTLV(packet, seq1);
	CloseTLV(packet, seq2);
	CloseTLV(packet, seq3);
	CloseTLV(packet, seq4);

	uint16 result = SendSNMPPacketAndWaitForReply(packet);
	if (result) {
		gErrorMsg(fMessageLogSettings, "Get_docsDevSwCurrentVers")
			<< "Unable to complete SNMP operation, result = " << result << endl;
		return false;
	}

	uint offset = 0;
	result = VerifySNMPResult(packet, fRequestId++, offset);
	if (result) {
		gErrorMsg(fMessageLogSettings, "Get_docsDevSwCurrentVers")
			<< "Unable to verify SNMP operation, result = " << result << endl;
		return false;
	}
	BcmOctetBuffer val(256);
	if (!SkipOctetStringTLV(packet, offset, VBT_OBJECT, 10, docsDevSwCurrentVersOid)) {
		gErrorMsg(fMessageLogSettings, "Get_docsDevSwCurrentVers")
			<< "Unable to parse received SNMP packet, bad OID" << endl;
		return false;
	}
	// Finally get the string
	BcmOctetBuffer version(32);
	if (!GetOctetStringTLV(packet, offset, VBT_STRING, version)) {
		gErrorMsg(fMessageLogSettings, "Get_docsDevSwCurrentVers")
			<< "Unable to parse received SNMP packet, bad version" << endl;
		return false;
	}

	offset = 0;
	major = ParseVersionString(version, offset);
	minor = ParseVersionString(version, offset);
	subMinor = ParseVersionString(version, offset);

	return true;
}

#if 0
// BCDToBin(uint16 bcd)
//		Convert 16 bit binary code decival value to 16 bit binary value
//
// Parameters:
//      bcd - 16 bit binary code decimal value.
//
// Returns:
//      16 bit binary value.
//
static uint16 BCDToBin(uint16 bcd)
{
	uint16 bin = 0, digit;

	for (digit = 0; digit < 4; digit++)
	{
		bin = (bin * 10) + ((bcd & 0xf000) >> 12);
		bcd <<= 4;
	}
	return bin;
}

static void GetLocalIpAddress()
{

#if defined(USE_PRIVATE_NETWORK)
	fMyIpAddress = kLocalRxIpAddress;
#else
	GetEstbIpAddress(&fMyIpAddress);
#endif

}
#endif

// Print() - Print detailed formatted object contents 
//      to specified ostream.
//
// Parameters:
//      ostream& ostrm - reference to the destination output stream.
//
// Returns:
//      reference to the destination output stream.
//
ostream& BcmEstbCannedSnmp::Print( ostream& ostrm ) const
{
	return ostrm;
}

// Returns the pointer to the singleton program store device.  Most objects
// in the system will use this method rather than being passed a pointer to
// the object.  The singleton pointer will be set up in the base-class
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
//      A pointer to the program store device that should be used by the
//      system.
//
BcmEstbCannedSnmp *BcmEstbCannedSnmp::GetSingletonInstance(void)
{
    if (pfSingletonInstance == NULL)
    {
        gLogMessageRaw
            << "BcmEstbCannedSnmp::GetSingletonInstance:  WARNING - the singleton instance is NULL, and someone is accessing it!" << endl;
    }
    return pfSingletonInstance;
}


#if ( BCM_CONSOLE_SUPPORT )
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
BcmEstbCannedSnmpCommandTable &BcmEstbCannedSnmp::GetCommandTableSingleton(void)
{
    static BcmEstbCannedSnmpCommandTable gCommandTable;

    return gCommandTable;
}
#endif


// Set_docsDevResetNow()
//		Create, send, and verify and SNMP SET for the docsDevResetNow MIB. The
//		IP address used will always be 192.168.100.1 since this is the standard
//		address used by the eCM for early SNMP access.
//
// Parameters:
//      none.
//
// Returns:
//      true - All is well
//		False - Something didn't work
//
bool BcmEstbCannedSnmp::Set_docsDevResetNow(uint8 docsDevResetNow)
{
	BcmOctetBuffer packet(1024);
	uint16 seq1, seq2, seq3, seq4;

	// build a SNMP GET packet for the docsDevResetNow.0 MIB
	AddSNMPHeader(packet, ASN_SET_OP, fRequestId, seq1, seq2, seq3, seq4);

	// Add the OID   6.1.2.1.69.1.1.3.0
	uint8 docsDevResetNowOid[] = {0x2b, 0x06, 0x01, 0x02, 0x01, 0x45, 0x01, 0x01, 0x03, 0x00};
	AddTLV(packet, VBT_OBJECT, 10, docsDevResetNowOid);

	// Add the value
	AddTLV(packet, VBT_NUMBER, sizeof(docsDevResetNow), &docsDevResetNow);

	CloseTLV(packet, seq1);
	CloseTLV(packet, seq2);
	CloseTLV(packet, seq3);
	CloseTLV(packet, seq4);

	uint16 result = SendSNMPPacketAndWaitForReply(packet);
	if (result) {
		gErrorMsg(fMessageLogSettings, "Set_docsDevResetNow")
			<< "Unable to complete SNMP operation, result = " << result << endl;
		return false;
	}

	uint offset = 0;
	result = VerifySNMPResult(packet, fRequestId++, offset);
	if (result) {
		gErrorMsg(fMessageLogSettings, "Set_docsDevResetNow")
			<< "Unable to verify SNMP operation, result = " << result << endl;
		return false;
	}
	return true;
}

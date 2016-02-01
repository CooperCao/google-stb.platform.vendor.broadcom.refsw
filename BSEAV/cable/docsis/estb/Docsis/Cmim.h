//****************************************************************************
//
// Copyright (c) 2010 Broadcom Corporation
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
//  $Id: Cmim.h 1.2 2006/07/21 23:53:55Z smuller Release $
//
//  Filename:       Cmim.h
//  Author:         Steve Muller
//  Creation Date:  June 21, 2006
//
//****************************************************************************
//  Description:
//      BcmCmim models a DOCSIS 3.0/L2VPN Cable Modem Interface Mask.
//
//****************************************************************************

#ifndef BCMCMIM_H
#define BCMCMIM_H

//********************** Include Files ***************************************

#include "typedefs.h"
#include "OctetBuffer.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************


class BcmCmim
{
public:
    // begin functions required to be param for STL containers...
    //


    /// Default constructor.
    ///
    BcmCmim() 
      :
        fU32Rep( 0 )
    {}


    /// Copy constructor.
    ///
    /// \param
    ///     rhs - the instance to be copied.
    ///
	BcmCmim( const BcmCmim& rhs )
        :
        fU32Rep( rhs.fU32Rep )
    {}


    /// Initializing constructor.  Uses the values specified to initialize
    /// the object.
    ///
    /// \param
    ///     cmimBits31To24 - the most significant bits of the cmim (bits 24 - 31).  
    /// \param
    ///     cmimBits23To16 - the middle (8) bits of the cmim (bits 16 - 23).
    /// \param
    ///     cmimBits15To8 - the middle (8) bits of the cmim (bits 8 - 15).
    /// \param
    ///     cmimBits7To0 - the least significant (8) bits of the cmim (bits 0 - 7).
    ///
	BcmCmim( uint8 cmimBits31To24, uint8 cmimBits23To16, uint8 cmimBits15To8, uint8 cmimBits7To0 )
    { 
        Set( cmimBits31To24, cmimBits23To16, cmimBits15To8, cmimBits7To0 );
    }


    /// Initializing constructor.  Reads (4) bytes from the buffer specified to
    /// initialize the object.  Note: cmim size is up to (32) bits.  
    ///
    /// \param
    ///     buf - pointer to a buffer with the 3 bytes to use.
    ///
	BcmCmim( const uint8* buf ) { Set( buf[ 0 ], buf[ 1 ], buf[ 2 ], buf[ 3 ] ); }


    // Destructor.
    //
    virtual ~BcmCmim();


    /// Assignment operator.
    ///
    /// \param
    ///     rhs - source object, a.k.a. right hand side.
    ///
    /// \return
    ///     reference to this object
    ///
    BcmCmim& operator=( const BcmCmim& rhs )
    {
        if( this == &rhs )
        {
            // special case: assignment to self
            return *this;
        }

        fU32Rep = rhs.fU32Rep;
        return *this;
    }

    /// Less than operator.
    ///
    /// \param
    ///     rhs - comparison object, a.k.a. right hand side.
    ///            
    /// \return
    ///     bool - true if this object is less than the comparison object.
    ///
    bool operator<( const BcmCmim& rhs ) const 
    { 
        return (fU32Rep < rhs.fU32Rep); 
    }

    /// Equality operator.
    ///
    /// \param
    ///     rhs - comparison object, a.k.a. right hand side.
    ///            
    /// \return
    ///     bool - true if objects are equal, else false.
    ///
	bool operator==( const BcmCmim& rhs ) const
    { 
        return (fU32Rep == rhs.fU32Rep); 
    }


    /// Inequality operator.
    ///
    /// \param
    ///     rhs - comparison object, a.k.a. right hand side.
    ///            
    /// \return
    ///     bool - true if objects are equal, else false.
    ///
	bool operator!=( const BcmCmim& rhs ) const
    { 
        return (fU32Rep != rhs.fU32Rep); 
    }
    // end functions required to be param for STL containers...


    /// Reset data members to default values.
    ///
    void Reset() { fU32Rep = 0; }


    /// Retrieve all 4 cmim bytes.
    ///
    /// \param
    ///     cmimBits31To24 - the most significant bits of the cmim (bits 24 - 31).  
    /// \param
    ///     cmimBits23To16 - the middle (8) bits of the cmim (bits 16 - 23).
    /// \param
    ///     cmimBits15To8 - the middle (8) bits of the cmim (bits 8 - 15).
    /// \param
    ///     cmimBits7To0 - the least significant (8) bits of the cmim (bits 0 - 7).
    ///
	void Get( uint8& cmimBits31To24, uint8& cmimBits23To16, uint8& cmimBits15To8, uint8& cmimBits7To0 ) const;

    /// Retrieve uint32 representation of cmim in host bit order.
    ///
    /// \param
    ///     dest_u32 - reference to destination uint32.
    ///
    void Get( uint32& dest_u32 ) const { dest_u32 = fU32Rep; }

    /// Retrieve uint32 representation of cmim in host bit order.
    ///
    /// \return
    ///     value of cmim
    ///
    uint32 Get( void ) const { return fU32Rep; }

    /// Set all 4 cmim bytes to the specified values.
    ///
    /// \param
    ///     cmimBits31To24 - the most significant bits of the cmim (bits 24 - 31).  
    /// \param
    ///     cmimBits23To16 - the middle (8) bits of the cmim (bits 16 - 23).
    /// \param
    ///     cmimBits15To8 - the middle (8) bits of the cmim (bits 8 - 15).
    /// \param
    ///     cmimBits7To0 - the least significant (8) bits of the cmim (bits 0 - 7).
    ///
	void Set( uint8 cmimBits31To24, uint8 cmimBits23To16, uint8 cmimBits15To8, uint8 cmimBits7To0 );
    

    /// Print formatted object contents to specified ostream.
    ///
    /// \param
    ///      ostream& ostrm - reference to the destination output stream.
    ///
    /// \return
    ///      Reference to the destination output stream.
    ///
    ostream& Print( ostream& ostrm ) const;


    /// Write the cmim to the end of specified buffer in network order.  
    ///
    /// \param
    ///      octetBuffer - reference to the destination buffer.
    ///
    /// \retval
    ///      true if successful.
    /// \retval
    ///      false if there was a problem (not enough space in the 
    ///          output buffer).
    ///
    bool WriteTo( BcmOctetBuffer &octetBuffer ) const;
        

    /// This methods causes the object to read its 4 bytes from the specified
    /// location in the buffer.  This is the opposite of WriteTo().  Note that
    /// the offset parameter will be adjusted to reflect the next position in the
    /// buffer that needs to read from.
    ///
    /// Note that if the read is successful, then the offset parameter will be
    /// incremented by 4.  If the read fails, then neither the cmim nor
    /// the offset parameter will be modified.
    ///
    /// \param
    ///      octetBuffer - reference to the buffer from which the bytes should
    ///                    be read.
    /// \param
    ///      offset - the offset into the buffer where the first IP address byte
    ///               is.
    /// \param
    ///      length - length of the cmim tlv.
    ///
    /// \retval
    ///      true if successful.
    /// \retval
    ///      false if there was a problem (not enough bytes of data in the
    ///          buffer).
    ///
    bool ReadFrom( const BcmOctetBuffer &octetBuffer, unsigned int &offset, uint16 length );


    // misc accessors
    //

protected:

    /// uint32 representation of cmim. uint32 rep is useful for optimizing comparison operations.
    uint32 fU32Rep;

};

inline ostream& operator<<( ostream& ostrm, const BcmCmim& rhs )
{
    return rhs.Print( ostrm );
}

#endif


//**************************************************************************
//
//    Copyright 2000 Broadcom Corporation
//    All Rights Reserved
//    No portions of this material may be reproduced in any form without the
//    written permission of:
//             Broadcom Corporation
//             16251 Laguna Canyon Road
//             Irvine, California  92618
//    All information contained in this document is Broadcom Corporation
//    company private, proprietary, and trade secret.
//
//
//
//**************************************************************************
//    Filename: BcmBasicString.h
//    Author:   Kevin O'Neal
//    Creation Date: 18-Sept-2001
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Wrapper for an STL basic_string to eliminate inline expansions.
//    The sole reason for doing this is to reduce code size.  Turns out that
//    savings can be considerable: ~85k for the pSOS build as of this
//    release.  Note that this object does not wrap all basic_string
//    methods, just the ones we need to build.  Since the whole point of
//    this is code savings, I intentionally left out all unused methods.
//    Any method that needs to be added later can be, you get the idea.
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef BCMBASICSTRING_H
#define BCMBASICSTRING_H

#include <string>
#include <iostream>

#if (__GNUC__ >= 3) || defined(WIN32)
using namespace std;
#endif

// forward references:
class BcmOctetBuffer;
class BcmIpAddress;

class BcmBasicString
{
  public:
    typedef char E;
    typedef E & reference;
    typedef const E & const_reference;
    typedef unsigned int size_type;
    
    BcmBasicString ();
    BcmBasicString (const E *s);
    BcmBasicString (const E *s, size_type n);
    BcmBasicString (const string &s);
    BcmBasicString (const BcmBasicString &s);
   ~BcmBasicString ();
   
    bool operator < (const BcmBasicString &s) const;
    bool operator == (const E *s) const;
    bool operator == (const BcmBasicString &s) const;
    bool operator != (const E *s) const;
    bool operator != (const BcmBasicString &s) const;
    BcmBasicString &operator = (E c);
    BcmBasicString &operator = (const E *s);
    BcmBasicString &operator = (const BcmBasicString &s);
    BcmBasicString &operator += (E c);
    BcmBasicString &operator += (const E *s);
    BcmBasicString &operator += (const BcmBasicString &rhs);
    //const_reference operator [] (size_type pos) const;
    const E operator [] (size_type pos) const;
    reference operator [] (size_type pos);
    size_type length () const;
    size_type size () const;
    bool empty () const;
    const E *c_str () const;
    const E *data () const;
    BcmBasicString &assign (const E *s, size_type n);
    BcmBasicString &assign (const BcmBasicString &str, size_type pos, size_type n);
    BcmBasicString &insert(size_type p0, const E *s);
    BcmBasicString& append(const E *s, size_type n);
    size_type find_first_of (E c, size_type pos = 0) const;
    size_type find (const BcmBasicString& str, size_type pos = 0) const;
    BcmBasicString substr (size_type pos = 0, size_type n = 0) const;
   
    ostream &Print(ostream &outStream) const;
    
    /* V2 specific methods, not STL string replacements */
    
    BcmBasicString (const BcmOctetBuffer &Buffer);
    BcmBasicString &operator = (const BcmOctetBuffer &Buffer);
    BcmBasicString &Set (const BcmOctetBuffer &Buffer);
    void Get (BcmOctetBuffer &Buffer) const;
    
    BcmBasicString (const BcmIpAddress &IP, bool DottedDecimal = false);
    BcmBasicString &operator = (const BcmIpAddress &IP);
    BcmBasicString &Set (const BcmIpAddress &IP, bool DottedDecimal=false);
   
  protected:
    basic_string <E> fString;
};

inline ostream &operator << (ostream &outStream, const BcmBasicString &String)
{
    return String.Print (outStream);
}

#endif

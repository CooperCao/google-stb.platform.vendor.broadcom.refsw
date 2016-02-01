//****************************************************************************
//
//  Copyright (c) 2001-2003  Broadcom Corporation
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
//  $Id: BitmaskCommandParameter.h 1.4 2004/01/05 14:25:54Z dpullen Release $
//
//  Filename:       BitmaskCommandParameter.h
//  Author:         David Pullen
//  Creation Date:  Feb 23, 2001
//
//****************************************************************************

#ifndef BITMASKCOMMANDPARAMETER_H
#define BITMASKCOMMANDPARAMETER_H

//********************** Include Files ***************************************

// My base class.
#include "CommandParameter.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************


/** \ingroup BfcConsole
*
*      This is the derived class for bitmask parameters.  It handles
*      parsing a mask of bits with optional operators (OR/AND/XOR/etc), and
*      limiting it to an optional set of valid bits.  This class can handle
*      up to 32 bits.
*
*      \note  All of the string pointers in the class MUST be static strings
*             that will not change or be deleted, etc., because only the string
*             pointers are stored!
*/
class BcmBitmaskCommandParameter : public BcmCommandParameter
{
public:

    /// Default/Initializing Constructor.  Initializes the state of the object,
    /// storing the values specified.
    ///
    /// If pParameterName is NULL, "Bitmask" will be used.
    ///
    /// \param
    ///      clientId - the id that the client wants to associate with this
    ///                 parameter.
    /// \param
    ///      isOptional - true if the parameter is optional, false if it is
    ///                   required.
    /// \param
    ///      pParameterName - the name of this parameter (for printing).
    /// \param
    ///      validBits - the set of bits that can be set to 1.
    ///
    BcmBitmaskCommandParameter(unsigned int clientId = 0, 
                               bool isOptional = false,
                               const char *pParameterName = NULL,
                               uint32 validBits = 0xffffffff);

    /// Destructor.  Frees up any memory/objects allocated, cleans up internal
    /// state.
    ///
    virtual ~BcmBitmaskCommandParameter();

    /// Prints the value that is currently stored (regardless of whether or not
    /// parsing was successful) to the specified ostream.
    ///
    /// This method must be implemented by the derived class.
    ///
    /// \param
    ///      outstream - the ostream to be used.
    ///
    /// \return
    ///      A reference to the ostream for chaining insertion operators.
    ///
    virtual ostream &PrintValue(ostream &outstream) const;

    /// Prints the value that is currently stored (regardless of whether or not
    /// parsing was successful) to the specified string.
    ///
    /// This method must be implemented by the derived class.
    ///
    /// \param
    ///      string - the string into which the value should be printed.
    ///
    virtual void PrintValue(char string[64]) const;

public:

    /// Performs the stored operation against the specified current value, using
    /// the value that was stored as a result of parsing or setting the value,
    /// returning the result.
    ///
    /// The general form of the equation is:
    ///
    ///     currentValue op= storedValue;
    ///
    /// See the Operation enum below for more details.
    ///
    /// \param
    ///     currentValue - the value to be used as the lval in the equation
    ///                    above.
    ///
    /// \return
    ///     The result of the equation.
    ///
    virtual uint32 AsBitmask(uint32 currentValue = 0) const;

public:

    /// Stores the specified value, and sets the operation to kEQUAL.
    ///
    /// \param
    ///     value - the value to be stored.
    ///
    /// \retval
    ///     true if the value was stored successfully.
    /// \retval
    ///     false if the value was not stored successfully.
    ///
    virtual bool SetAsBitmask(uint32 value);

protected:

    /// This must be implemented by the derived class.  It is a "template method"
    /// that allows the derived class to do the real work of parsing the string
    /// and updating its state.
    ///
    /// The derived class can assume that the pParameterString pointer is not
    /// NULL.
    ///
    /// \param
    ///      pParameterString - pointer to the string containing the parameters
    ///                         to be parsed.  It will not be modified.
    /// \param
    ///      offset - the offset into the string where parsing should begin.
    ///
    /// \retval
    ///      true if the parameter was successfully parsed.
    /// \retval
    ///      false if the parameter was not successfully parsed.
    ///
    virtual bool ParseImpl(const char *pParameterString, unsigned int &offset);

    /// Gives the derived class a chance to do some derived-class specific stuff
    /// during the print.
    ///
    /// \param
    ///      outstream - the ostream to be used.
    ///
    /// \return
    ///      A reference to the ostream for chaining insertion operators.
    ///
    virtual ostream &PrintImpl(ostream &outstream) const;

private:

    /// Operation values supported for this object.
    typedef enum
    {
        /// Used if '=' is parsed, or if no operation is specified; just
        /// stores the value:  value = newValue
        kEQUAL = 0,

        /// Used if '&' is parsed; uses bitwise AND operation to modify the
        /// existing value:  value &= newValue
        kAND,

        /// Used if '|' is parsed; uses bitwise OR operation to modify the
        /// existing value:  value |= newValue
        kOR,

        /// Used if '^' is parsed; uses bitwise XOR operation to modify the
        /// existing value:  value ^= newValue
        kXOR,

        /// Used if '~' is parsed; uses a combination of 1's complement and
        /// bitwise AND operation to modify the existing value:
        /// value &= ~newValue
        kNOTAND

    } Operation;

    /// The operation parsed.
    Operation fOperation;

    /// The value we store/parse/retrieve.
    uint32 fValue;

    /// The set of bits that I'm limiting the value to.
    uint32 fValidBits;

private:

    /// Copy Constructor.  Not supported.
    BcmBitmaskCommandParameter(const BcmBitmaskCommandParameter &otherInstance);

    /// Assignment operator.  Not supported.
    BcmBitmaskCommandParameter & operator = (const BcmBitmaskCommandParameter &otherInstance);

};


//********************** Inline Method Implementations ***********************


/// Insertion operator for this class.  It allows you to do the following:
///
///    cout << commandParameter << endl;
///
inline ostream & operator << (ostream &outputStream, const BcmBitmaskCommandParameter &commandParameter)
{
    return commandParameter.Print(outputStream);
}


#endif



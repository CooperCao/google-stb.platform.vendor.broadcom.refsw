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
//  $Id: CommandParameterList.h 1.3 2004/01/05 14:25:58Z dpullen Release $
//
//  Filename:       CommandParameterList.h
//  Author:         David Pullen
//  Creation Date:  Feb 26, 2001
//
//****************************************************************************

#ifndef COMMANDPARAMETERLIST_H
#define COMMANDPARAMETERLIST_H

//********************** Include Files ***************************************

#include "CommandParameter.h"

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <iostream>
#include <list>
#else
#include <iostream.h>
#include <list.h>
#endif

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************


/** \ingroup BfcConsole
*
*      This is a helper class that holds a set of BcmCommandParameter objects.
*      It knows how to iterate over the list in the right order and to get
*      each to parse itself from a string.
*/
class BcmCommandParameterList
{
public:

    /// Default Constructor.  Initializes the state of the object...
    ///
    BcmCommandParameterList(void);

    /// Destructor.  Frees up any memory/objects allocated, cleans up internal
    /// state.
    ///
    virtual ~BcmCommandParameterList();

    /// Parses the specified string of parameters, storing them in the objects
    /// contained in this object.  It tries the order-independent parameters
    /// (usually flag parameters) before doing the order-dependent parameters.
    ///
    /// Parsing starts by resetting all parameters to an un-parsed state.  Then,
    /// for each "word" in the parameter string, we check the order-independent
    /// parameters to see if it matches; if not, then we try to parse the next
    /// unparsed order-dependent parameter.
    ///
    /// In order for the parse to be successful, all of the required parameters
    /// must be parsed.  Also, any optional parameters that were present must
    /// be parsed correctly (without error).  Any extra un-used parameter "words"
    /// will be reported but won't be considered an error.
    /// 
    /// \param
    ///      pParameterString - pointer to the string of parameters to be parsed.
    ///
    /// \retval
    ///      true if parsing was successful.
    /// \retval
    ///      false if there was a problem.
    ///
    bool Parse(const char *pParameterString);

    /// Adds a new order-dependent parameter object to the list.  The parameters
    /// are stored and parsed in the order that they are added.
    ///
    /// The parameter object specified will be destroyed by this object; you must
    /// not destroy it yourself, and you must not add it to any other
    /// BcmCommandParameterList object.
    ///
    /// \note  All parameters added to this object should have a unique client
    ///        id value assigned to them; otherwise, Find() will only return the
    ///        first parameter object with the specified id!
    ///
    /// \param
    ///      pParameter - pointer to the object to be added.
    ///
    void AddOrderDependent(BcmCommandParameter *pParameter);

    /// Adds a new order-independent parameter object to the list.  This will
    /// usually be a flag parameter, since other types of parameters often
    /// don't have a way to delineate themselves.
    ///
    /// The parameter object specified will be destroyed by this object; you must
    /// not destroy it yourself, and you must not add it to any other
    /// BcmCommandParameterList object.
    ///
    /// \note  All parameters added to this object should have a unique client
    ///        id value assigned to them; otherwise, Find() will only return the
    ///        first parameter object with the specified id!
    ///
    /// \param
    ///      pParameter - pointer to the object to be added.
    ///
    void AddOrderIndependent(BcmCommandParameter *pParameter);

    /// Returns a pointer to the parameter object with the specified id.  Because
    /// this method uses the client id in the parameter objects as the key in the
    /// search, you should assign each parameter object a unique id.
    ///
    /// \param
    ///      clientId - the id of the parameter object to find.
    ///
    /// \return
    ///      A pointer to the object with the specified id, or NULL if not found.
    ///
    const BcmCommandParameter *Find(unsigned int clientId);

    /// Removes the parameter object with the specified id from the list and
    /// deletes it.
    ///
    /// \param
    ///      clientId - the id of the parameter object to delete.
    ///
    /// \retval
    ///      true if the parameter was found and deleted.
    /// \retval
    ///      false if the parameter was not found.
    ///
    bool Remove(unsigned int clientId);

    /// Removes and deletes all parameter objects from all lists.
    ///
    void RemoveAll(void);

    /// Prints itself (mainly the objects it contains) to the specified ostream.
    ///
    /// \param
    ///      outstream - the ostream to be used.
    ///
    /// \return
    ///      A reference to the ostream for chaining insertion operators.
    ///
    ostream &Print(ostream &outstream) const;

private:

    /// An STL object to store the order-dependent parameter objects.
    list<BcmCommandParameter *> fOrderDependentParameters;

    /// An STL object to store the order-independent parameter objects.
    list<BcmCommandParameter *> fOrderIndependentParameters;

private:

    /// Copy Constructor.  Not supported.
    BcmCommandParameterList(const BcmCommandParameterList &otherInstance);

    /// Assignment operator.  Not supported.
    BcmCommandParameterList & operator = (const BcmCommandParameterList &otherInstance);

};


//********************** Inline Method Implementations ***********************


/// Insertion operator for this class.  It allows you to do the following:
///
///    cout << commandParameter << endl;
///
inline ostream & operator << (ostream &outputStream, const BcmCommandParameterList &commandParameter)
{
    return commandParameter.Print(outputStream);
}


#endif



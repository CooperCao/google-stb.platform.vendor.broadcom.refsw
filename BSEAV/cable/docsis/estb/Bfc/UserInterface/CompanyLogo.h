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
//  $Id: CompanyLogo.h 1.2 2002/01/23 23:31:02Z msieweke Release $
//
//  Filename:       CompanyLogo.h
//  Author:         David Pullen
//  Creation Date:  March 12, 2000
//
//****************************************************************************
//  Description:
//      A simple class that is able to display the ASCII-art logo for your
//      company, along with copyright information, the build date/time, and
//      version information.
//
//****************************************************************************

#ifndef COMPANYLOGO_H
#define COMPANYLOGO_H

//********************** Include Files ***************************************

#include "typedefs.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************


class BcmCompanyLogo
{
public:

    // Default Constructor.  Initializes the state of the object...
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmCompanyLogo(void);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    ~BcmCompanyLogo();

    // This method allows someone to set the time/date when the application was
    // built.  By default, the date and time are set to the date and time when
    // this module was built.  Note, however, that the executable may have been
    // built at a later date (if this module wasn't changed, then it won't be
    // built).
    //
    // Parameters:
    //      pDateString - pointer to a const string containing the build date.
    //      pTimeString - pointer to a const string containing the build time.
    //
    // Returns:  Nothing.
    //
    void SetBuildDateTime(const char *pDateString, const char *pTimeString);

    // This method allows someone to set the username of the person who built
    // the application.
    //
    // Parameters:
    //      pUsernameString - pointer to a const string containing the username.
    //
    // Returns:  Nothing.
    //
    void SetUsername(const char *pUsernameString);

    // This method allows the client to set the revision string and the feature
    // codes string.
    //
    // The version string is simply a text string containing the software rev
    // that you want displayed (e.g. "2.0.1a").  There are no real restrictions
    // on what version info can be present.
    //
    // The features string is meant to show what features have been included
    // in the software build, differentiating it from other builds based on the
    // same code rev (e.g. "SNMP-Full QCLC EMTA").
    //
    // Parameters:
    //      pVersionString - pointer to a const string containing the version
    //                       number information.
    //      pFeaturesString - pointer to a const string containing the features
    //                        present in the build.
    //
    // Returns:  Nothing.
    //
    void SetRevisionString(const char *pVersionString, const char *pFeaturesString = NULL);

    // This does the job of printing the logo and other information.
    //
    // Parameters:
    //      outstream - the ostream to which the logo should be printed.
    //
    // Returns:
    //      A reference to the ostream to support chaining of operators.
    //
    ostream &Print(ostream &outstream) const;

protected:

private:

    // These are the strings that can be set by the client, and that will be
    // displayed.
    const char *pfDateString;
    const char *pfTimeString;
    const char *pfVersionString;
    const char *pfFeaturesString;
    const char *pfUsernameString;

private:

    // Copy Constructor.  Initializes the state of this object to match that
    // of the instance passed in.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmCompanyLogo(const BcmCompanyLogo &otherInstance);

    // Assignment operator.  Copies the state of the instance passed in so that
    // this object is "identical".
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmCompanyLogo & operator = (const BcmCompanyLogo &otherInstance);

};


//********************** Inline Method Implementations ***********************


inline ostream & operator << (ostream &outstream, const BcmCompanyLogo &logo)
{
    return logo.Print(outstream);
}


#endif



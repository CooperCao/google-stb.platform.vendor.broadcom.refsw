//****************************************************************************
//
// Copyright (c) 2000-2009 Broadcom Corporation
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
//  Filename:       CompanyLogo.cpp
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

//********************** Include Files ***************************************

// My api and definitions...
#include "CompanyLogo.h"

#include <string.h>

//********************** Local Types *****************************************

//********************** Local Constants *************************************

// The following strings will be concatenated together by the compiler into a
// single string.
static const char *pgCompanyLogo =
"\n\n"
"                          *\n"
"                         * *\n"
"                         * *\n"
"                        *   *\n"
"                        *   *\n"
"                       *     *\n"
"                       *     *\n"
"                       *     *\n"
"                      *       *\n"
"                      *       *\n"
"                      *       *\n"
"                     *         *\n"
"                     *         *\n"
"                     *         *\n"
"                     *         *\n"
"                    *           *\n"
"          *         *           *         *\n"
"        *   *       *           *       *   *          ***\n"
"*     *      *     *             *     *      *     *       *******************\n"
"   *          *   *               *   *          *\n"
"                *                   *\n\n"
"Copyright (c) 1999 - 2009 Broadcom Corporation\n\n";

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


// Default Constructor.  Initializes the state of the object...
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmCompanyLogo::BcmCompanyLogo(void)
{
    // Clear out my string pointers.
    pfDateString = NULL;
    pfTimeString = NULL;
    pfVersionString = NULL;
    pfFeaturesString = NULL;
    pfUsernameString = NULL;
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmCompanyLogo::~BcmCompanyLogo()
{
    // Clear out my string pointers.
    pfDateString = NULL;
    pfTimeString = NULL;
    pfVersionString = NULL;
    pfFeaturesString = NULL;
    pfUsernameString = NULL;
}


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
void BcmCompanyLogo::SetBuildDateTime(const char *pDateString, 
                                      const char *pTimeString)
{
    // Just store these.
    pfDateString = pDateString;
    pfTimeString = pTimeString;
}


// This method allows someone to set the username of the person who built
// the application.
//
// Parameters:
//      pUsernameString - pointer to a const string containing the username.
//
// Returns:  Nothing.
//
void BcmCompanyLogo::SetUsername(const char *pUsernameString)
{
    // Just store it.
    pfUsernameString = pUsernameString;
}


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
void BcmCompanyLogo::SetRevisionString(const char *pVersionString, 
                                       const char *pFeaturesString)
{
    // Just store these.
    pfVersionString = pVersionString;
    pfFeaturesString = pFeaturesString;
}


// This does the job of printing the logo and other information.
//
// Parameters:
//      outstream - the ostream to which the logo should be printed.
//
// Returns:
//      A reference to the ostream to support chaining of operators.
//
ostream &BcmCompanyLogo::Print(ostream &outstream) const
{
    const char *pDateString = pfDateString;
    const char *pTimeString = pfTimeString;
    const char *pVersionString = pfVersionString;
    const char *pFeaturesString = pfFeaturesString;
    const char *pUsernameString = pfUsernameString;

    // This string comes from the one at the top of this file.
    outstream << pgCompanyLogo;

    // Make sure the date/time strings are valid.  The client can override
    // these with values that are "newer".  If none was specified, then we use
    // the date/time when this module was built.
    if (pDateString == NULL)
    {
        pDateString = __DATE__;
    }
    if (pTimeString == NULL)
    {
        pTimeString = __TIME__;
    }

    // Make sure the version and feature strings are not NULL, set defaults.
    if (pVersionString == NULL)
    {
        pVersionString = "{none}";
    }
    if (pFeaturesString == NULL)
    {
        pFeaturesString = "{none}";
    }
    if (pUsernameString == NULL)
    {
        pUsernameString = "{unknown}";
    }

    // Finally, show the build date, time, and the version/feature strings.
    // Note that this gets printed inside of a nice box; I prime the output
    // with the right border so that it is at the correct offset, then I output
    // a CR which resets the cursor to column 0.  Then the rest of the string
    // (which is variable length) is output inside of the box.
    outstream
        << " +----------------------------------------------------------------------------+\n"
        << "                                                                              |\r"
        << " | Build Date:  " << pDateString << "\n"
        << "                                                                              |\r"
        << " | Build Time:  " << pTimeString << "\n"
        << "                                                                              |\r"
        << " | Built By:    " << pUsernameString << "\n"
        << " |                                                                            |\n"
        << "                                                                              |\r"
        << " | Revision:  " << pVersionString << "\n"
        << " |                                                                            |\n";

    // Split the feature string across multiple lines if it is too long to fit
    // on a single line.
    {
        unsigned int featureStringLength, currentOffset, charsToCopy, substrIndex;
        char featureSubstring[65];

        currentOffset = 0;

        // Length of the feature string, excluding the \0 terminator.
        featureStringLength = strlen(pFeaturesString);

        while (currentOffset < featureStringLength)
        {
            // Copy 64 chars, or the number of chars remaining, whichever is
            // smaller.
            charsToCopy = featureStringLength - currentOffset;
            if (charsToCopy > (sizeof(featureSubstring)))
            {
                charsToCopy = sizeof(featureSubstring);
            }

            // Copy the substring, and make sure it is \0 terminated.
            strncpy(featureSubstring, pFeaturesString + currentOffset, charsToCopy);
            featureSubstring[charsToCopy] = '\0';

            substrIndex = charsToCopy - 1;

            // Now step backwards from the end to find the first space.  We want
            // to provide a clean break across a whole word, if at all possible.
            while ((featureSubstring[substrIndex] != '\0') &&
                   (featureSubstring[substrIndex] != ' '))
            {
                // If we stepped all the way back without finding a space, then
                // break out and just print a partial word.
                if (substrIndex == 0)
                {
                    substrIndex = charsToCopy - 2;

                    break;
                }

                substrIndex--;
            }

            // Terminate the string at the space, and step forward in the main
            // string.
            featureSubstring[substrIndex] = '\0';
            currentOffset += substrIndex;

            // Skip any whitespace in the main string.
            while (pFeaturesString[currentOffset] == ' ')
            {
                currentOffset++;
            }

            outstream
                << "                                                                              |\r"
                << " | Features:  " << featureSubstring << "\n";
        }
    }

    outstream
        << " +----------------------------------------------------------------------------+\n"
        << endl;

    return outstream;
}



//****************************************************************************
//
//  Copyright (c) 2005  Broadcom Corporation
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
//  $Id$
//
//  Filename:       DsgAppName.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  February 2, 2005
//
//****************************************************************************
//  Description:
//      This object is an Application Name - Application ID pair.
//
//****************************************************************************

//********************** Include Files ***************************************

// For memset() and other memory functions, etc.
#include <string.h>
#include <stdlib.h>

#include "DsgAppName.h"


//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


// Default Constructor.  
//
//
// Parameters:
//      pBuffer - pointer to the buffer to be parsed.
//      bufferSize - the number of bytes in the buffer.
//      appId - the integral application ID to which this name maps.
//
// Returns:  N/A
//
BcmDSGAppName::BcmDSGAppName(uint8 *pAppName, int appNameLen, int appId)
{
   char nullTerm[] = "";
 
   isValid = false;
   pfAppNameString = NULL;
   fAppId = 0;

  // First, do all of our range checking
  if (pAppName == NULL)
    return;
  if (appNameLen < 1)
    return;
  if (appNameLen > 255)
    return;
  if (appId < 1)
    return;
  if (appId > 0x0000ffff)
    return;

  // Range checks OK, go ahead with object construction
  printf("new BcmDSGAppName %*s %d \n", appNameLen, pAppName, appId);
  pfAppNameString = new BcmString;
  if (pfAppNameString == NULL)
    return;
  pfAppNameString->assign((char *)pAppName, appNameLen);

  fAppId = appId;
 
  isValid = true;
}


// Alternate Constructor. This assumes that no range checking has been done. 
//
//
// Parameters:
//      pString - pointer to a BcmString with the application name.
//      appId - the integral application ID to which this name maps.
//
// Returns:  N/A
//
BcmDSGAppName::BcmDSGAppName(BcmString *appString, int appId)
{
  isValid = false;

  // First, do all of our range checking
  if (appString == NULL)
    return;
  if (appString->length() < 1)
    return;
  if (appString->length() > 255)
    return;
  if (appId < 1)
    return;
  if (appId > 0x0000ffff)
    return;

  // Range checks OK, go ahead with object construction
  printf("new BcmDSGAppName %*s %d \n", appString->length(), appString->c_str(), appId);
  fflush(stdout);
  appString->Print(cout);
  pfAppNameString = appString;

  fAppId = appId;
 
  isValid = true;
}


// Destructor.  If the buffer was created by this class, or if it was given
// ownership of a buffer, then the buffer will be deleted.  If a buffer was
// given to this class but it was not told to own the buffer, then it just
// "forgets" the pointer to the buffer without deleting it.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmDSGAppName::~BcmDSGAppName()
{

  delete pfAppNameString;

  fAppId = 0;
}

bool BcmDSGAppName::operator == (const char *s) const
{
  return (*pfAppNameString == s);
}

bool BcmDSGAppName::operator == (const BcmString &s) const
{
  return (*pfAppNameString == s);
}

bool BcmDSGAppName::operator == (const BcmDSGAppName &n) const
{
  return (pfAppNameString == n.pfAppNameString);
}




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
//  Filename:       DsgAppNameTable.h
//  Author:         Maurice Turcotte
//  Creation Date:  February 2, 2005
//
//****************************************************************************
//  Description:
//      This object is the table that maps Application Names to Application IDs
//      for DSG Clients who register by name. This table is built up from SCTE 65
//      Source Name Subtable (SNS) which lives in the DSG Broadcast Tunnel.
//
//
//****************************************************************************

#ifndef DSGAPPNAMETABLE_H
#define DSGAPPNAMETABLE_H

//********************** Include Files ***************************************

// For the portable types.                        
#include "typedefs.h"
#include <iostream>

// For the byte swapping functions.
// #include "Utilities.h"

#include <string.h>
#include "DsgAppName.h"


//********************** Global Types ****************************************

//********************** Global Constants ************************************
#define MAXAPPNAMETABLEENTRIES 255

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************
class BcmDSGAppName;


class BcmDSGAppNameTable
{

  public:

    // Default Constructor.  An SCTE 65 Network Text Table Section (NTT) is parsed
    // to create the App Name Table. The entries are actually parsed out of the 
    // Source Name Subtable. If there is any errors in parsing, then an empty
    // table will be created. 
    //
    // Parameters:
    //      none
    //
    // Returns:  N/A
    //
    BcmDSGAppNameTable(unsigned char *buf, int len);

    // Constructor.  An empty table is created.
    //
    // Parameters:
    //      none
    //
    // Returns:  N/A
    //
    BcmDSGAppNameTable(void);

    // Destructor.  Destroys all the entries, emptying the table.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmDSGAppNameTable(void);

    // Destroy all the entries, emptying the table.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    void Empty(void);

    // Appends the specified number of bytes to the end of the buffer.  If there
    // isn't enough room, then none of the bytes are appended and it returns
    // false.
    //
    // This method is intended to be used only for C strings (char *), flat
    // memory arrays (uint8 *), and other un-structured pointers to memory.
    //
    // Parameters:
    //      pData - pointer to the memory that is to be appended.
    //      numberOfBytes - the number of bytes that are to be appended.
    //
    // Returns:
    //      true if the bytes were appended.
    //      false if there wasn't enough room to append the bytes.
    //
    bool Add(BcmDSGAppName *appName);

    // Look up an App Id.
    //
    // Parameters:  A NULL termianted string with the name.
    //
    // Returns:  An App Id or 0 if not found.
    //
    int AppId(char *appName);

    // Look up an App Id.
    //
    // Parameters:  A BcmString with the name.
    //
    // Returns:  An App Id or 0 if not found.
    //
    int AppId(BcmString &appName);

    // Dump the contents of the table.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    void Dump();

    // Get the number of table entries.
    //
    // Parameters:  none.
    //
    // Returns:  The number of table entries.
    //
    inline int Entries() {return fAppNameTableEntries;};

  protected:
 
    int parse_mts(unsigned char *buf, int len);

    int parse_sns_descriptor(unsigned char *buf, int len);

    int parse_sns_record(unsigned char *buf, int len);

    int parse_sns(unsigned char *buf, int len);

    void parse_ntt(unsigned char *buf, int len);


  private:

    BcmDSGAppName *fAppNameTable[MAXAPPNAMETABLEENTRIES];

    // The number of entries in this table.
    unsigned int fAppNameTableEntries;
};


#endif



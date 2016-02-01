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
//  Filename:       DsgAppNameTable.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  February 2, 2005
//
//****************************************************************************
//  Description:
//      This object is the table that maps Application Names to Application IDs
//      for DSG Clients who register by name. This table is built up from SCTE 65
//      Source Name Subtable (SNS) which lives in the DSG Broadcast Tunnel. 
//
//****************************************************************************

//********************** Include Files ***************************************

// For memset() and other memory functions, etc.
#include <string.h>
#include <stdlib.h>

#include "DsgAppNameTable.h"
#include "DsgAppName.h"
#include "crc.h"


//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************

#define BYTES_PER_LINE_OUT 8

void
raw_dump(unsigned char *buf, int len) {
    register int nbytes;
    register int byte;


    nbytes = len;

    if (len <= 0) {
      printf("<empty buffer>\n");       /* empty buffer, just complain and return */
      return;
    } else if (buf == NULL) {
      printf("<bogus buffer>\n");       /* bad buffer address, just complain and return */
      return;
    }


    while (nbytes > 0) {
      for (byte = 0; byte < BYTES_PER_LINE_OUT; byte++) {
        printf( "  %02x", *buf++, nbytes--);
        if (byte & 1)
          printf( " ");      /* space between words */
        if (nbytes <= 0)
          break;
      }
      printf("\n");
    }
    printf("\n");
}



// Default Constructor. 
//
//
// Parameters:
//      none
//
// Returns:  N/A
//
BcmDSGAppNameTable::BcmDSGAppNameTable(unsigned char *buf, int len)
{
  fAppNameTableEntries = 0;
  parse_ntt(buf, len);
}


// Constructor.  Creates an empty table.
//
//
// Parameters:
//      none
//
// Returns:  N/A
//
BcmDSGAppNameTable::BcmDSGAppNameTable(void)
{
  fAppNameTableEntries = 0;
}


// Destructor. Be nice and empty the table first, though.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmDSGAppNameTable::~BcmDSGAppNameTable()
{
  // Call the common cleanup method.
  BcmDSGAppNameTable::Empty();
}


// Add an entry the table. Note that entries
// can only be added to the end, since there
// is no piecemeal deletion, just an empty
//
// Parameters:  
//     A pointer to an Application Name Object
//
// Returns:  1 = success, 0 = failure.
//
bool BcmDSGAppNameTable::Add(BcmDSGAppName *appName)
{
  if (fAppNameTableEntries >= MAXAPPNAMETABLEENTRIES)
    return false;

  fAppNameTable[fAppNameTableEntries++] = appName;
 
  return true;
}

// Clear out (empty) the table.  Destroy all the entries.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmDSGAppNameTable::Empty(void)
{
  while (fAppNameTableEntries > 0) {
    delete fAppNameTable[--fAppNameTableEntries];
  }
}


// Dump the contents of the table. 
//
// Parameters:  None.
//
// Returns:  nothing.
//
void BcmDSGAppNameTable::Dump(void)
{
  unsigned int i;

  for (i = 0; i < fAppNameTableEntries; i++) {
    if (fAppNameTable[i])
      fAppNameTable[i]->Dump();
    else
      printf("NULL entry at %d\n", i);
  }
}

// Look up an App Id. 
//
// Parameters:  A NULL termianted string with the name.
//
// Returns:  An App Id or 0 if not found.
//
int BcmDSGAppNameTable::AppId(char *appName)
{
  unsigned int i;

  for (i = 0; i < fAppNameTableEntries; i++) {
    if (fAppNameTable[i]->AppNameMatch(appName))
      return fAppNameTable[i]->AppId();
  }
  return 0;
}

// Look up an App Id. 
//
// Parameters:  A BcmString with the name.
//
// Returns:  An App Id or 0 if not found.
//
int BcmDSGAppNameTable::AppId(BcmString &appName)
{
  unsigned int i;

  for (i = 0; i < fAppNameTableEntries; i++) {
    if (fAppNameTable[i]->AppNameMatch(appName))
      return fAppNameTable[i]->AppId();
  }
  return 0;
}

int
BcmDSGAppNameTable::parse_mts(unsigned char *buf, int len) {
  // Parse a Multilingual Text String. We onlu support 
  // mode 0x00 (ascii) at this time.
  // Return the offset to get to the ascii text in the 
  // buffer, or -1 if there is a parse problem.
 
  int textLen;

  // printf("parse_mts: starting. \n");
  // raw_dump(buf, len);
  // We've got to have at least the mode and the length fields
  if (len < 2)
    return (-1);

  // We support only mode 0x00 (ascii)
  if (*buf != 0x00) 
    return (-1);
  buf++;
  len--;

  // Get length field
  textLen = *buf;
  buf++;
  len--;

  // Check length
  if (textLen != len)
    return (-1);

  return (2);
}

int
BcmDSGAppNameTable::parse_sns_descriptor(unsigned char *buf, int len) {
  // we really don't care about the contents of
  // descriptors, but we have to parse them to
  // get past them
 
  int descrLen;

  if (len < 2)
    return (0);
 
  descrLen = *(buf+1);

  // just return the length plus 2 for the tag and length fields
  return (descrLen + 2);
}


int
BcmDSGAppNameTable::parse_sns_record(unsigned char *buf, int len) {
  int i, appType, appID, nameLen, mtsOffset;
  int descrLen, descrCount, startingLen;
  unsigned char *appName;

  startingLen = len;

  // Get application_type
  if (len < 1)    // application_type is 1 byte - actually one bit
      return 0;
  appType = *buf;  // get it!
  // even if app type is 0, we still need to wade through this record
  // to get to the next one.
  if (appType == 0) {
    // printf("parse_sns_record: found source_ID.\n");
  } else if (appType == 0x80) {
    // printf("parse_sns_record: found application_ID.\n");
  } else {
    // printf("parse_sns_record: found bad zero field 0x%x, giving up!\n", appType);
    return 0;
  }
  buf++;    // application_type is 1 byte - actually one bit
  len--;    // application_type is 1 byte - actually one bit

  // Get application_ID (or source_ID)
  if (len < 2)    // application_ID is 2 bytes
      return 0;
  appID = *buf;  // get first byte
  appID <<= 8; 
  appID += *(buf+1);  // get next byte
  if (appType) {
    if (appID > 0) {
      // printf("parse_sns_record: found application_ID %d.\n", appID);
    } else {
      // printf("parse_sns_record: found bad application_ID %d, giving up!\n", appID);
      return 0;
    }
  }
  buf += 2;    // application_ID is 2 bytes
  len -= 2;    // application_ID is 2 bytes

  // Get name_length
  if (len < 1)    // name_length is 1 byte
      return 0;
  nameLen = *buf;  // get it
  if (nameLen > 0) {
    // printf("parse_sns_record: found name_length %d.\n", nameLen);
  } else {
    // printf("parse_sns_record: found bad name_length %d, giving up!\n", nameLen);
    return 0;
  }
  buf++;    // name_length is 1 byte
  len--;    // name_length is 1 byte
  if (nameLen > len) {
    // printf("parse_sns_record: name_length %d is too long for buffer.\n", nameLen);
    return 0;
  }

  // Get source_name, which in our case only matters if it is really an application name
  // so don't even parse it unless we really want it.
  if (appType) {           // This is an Application Name
    mtsOffset = BcmDSGAppNameTable::parse_mts(buf, nameLen);
    if (mtsOffset < 0) {
      // printf("parse_sns_record: bad mts.\n");
      return 0;
    }
    appName = (buf + mtsOffset);
    nameLen -= mtsOffset;
    // printf("parse_sns_record: source_name %*s\n", nameLen, appName);
    // raw_dump(buf, nameLen);
    buf += nameLen + mtsOffset;    // source_name is name_length bytes long
    len -= nameLen + mtsOffset;    // source_name is name_length bytes long
  } else {       // This is a source name, just skip over it
    buf += nameLen;    // source_name is name_length bytes long
    len -= nameLen;    // source_name is name_length bytes long
  }

  // Get SNS_descriptors_count
  if (len < 1)    // SNS_descriptors_count is 1 byte
      return 0;
  descrCount = *buf;  // get it!
  // printf("parse_sns_record: found SNS_descriptors_count %d.\n", descrCount);
  buf++;    // SNS_descriptors_count is 1 byte
  len--;    // SNS_descriptors_count is 1 byte

  for (i = 0; i < descrCount; i++) {
    descrLen = parse_sns_descriptor(buf, len);
    if (descrLen == 0) {
      // printf("parse_sns_record: found bad descriptor(), giving up!\n");
      return (0);  // something really bad happened
    } else {
      buf += descrLen;
      len -= descrLen;
    }
  }

  if (appType) {   // This record is an Application Name, not a Source name
    if (fAppNameTableEntries < MAXAPPNAMETABLEENTRIES) {
      fAppNameTable[fAppNameTableEntries] = new BcmDSGAppName((uint8 *) appName, nameLen, appID);
      if (fAppNameTable[fAppNameTableEntries]->IsValid())
        fAppNameTableEntries++;
      else
        return (0);  // bad app name entry! just give up here
    }
  }

  return (startingLen - len);

}

int
BcmDSGAppNameTable::parse_sns(unsigned char *buf, int len) {
  int snsRecords, i, bytesProcessed, startingLen;

  // printf("parse_sns: starting.\n");
  // raw_dump(buf, len);

  startingLen = len;
  // Get number_of_SNS_records
  if (len < 1)    // number_of_SNS_records is 1 byte
      return 0;
  snsRecords = *buf;  // get it!
  if (snsRecords > 0) {
    // printf("parse_sns: found number_of_SNS_records %d.\n", snsRecords);
  } else {
    // printf("parse_sns: found number_of_SNS_records %d, giving up!\n", snsRecords);
    return 0;
  }
  buf += 1;
  len -= 1;

  for (i=0; i < snsRecords; i++) {
    bytesProcessed = parse_sns_record(buf, len);
    if (bytesProcessed > 0) {
      buf += bytesProcessed;
      len -= bytesProcessed;
    } else {
      return 0;    // something is really wrong
    }
  }

  return (startingLen - len);
}

void
BcmDSGAppNameTable::parse_ntt(unsigned char *buf, int len) {

  int sectionLen = 0;
  int snsLen;

  if (buf == NULL)
    return;

  // Check table_ID
  if (len < 1)    // table_ID is one byte
      return;
  if (*buf == 0xc3) {
    // printf("parse_ntt: found good table_id.\n");
    // raw_dump(buf, len);
  } else {
    // printf("parse_ntt: invalid table_ID\n");
    return;
  }
  // Before we do anything else, let's make sure the CRC is OK
  if (len < 12) {    // We should have at least 12 bytes for an empty ntt
    // printf("parse_ntt: table less than 12 bytes, giving up!\n");
    return;
  }
  if (FastCrc32(buf, (unsigned int) len) != 0xffffffff) {
    // printf("parse_ntt: bad crc, punting!\n");
    return;
  }
  // CRC is OK, let's move on to the next data
  // printf("parse_ntt: crc OK!\n");
  buf++;
  len--;

  // Check zero and reserved fields
  if (len < 1)    // zero and reserved fields are two bits each
      return;
  if ((*buf & 0xf0) == 0x30) {
    // printf("parse_ntt: found good zero and reseved fields.\n");
  } else {
    // printf("parse_ntt: invalid zero or reserved field\n");
    return;
  }

  // Get section_length
  if (len < 2)    // section_length is 12 bits
      return;
  sectionLen = (*buf & 0x0f);    // get first nibble
  sectionLen <<= 8;
  sectionLen += *(buf+1);  // add next byte
  // section_length should be "all the rest of the bytes"
  // so we'll adjust buf and len and then check against len
  buf += 2;    // section_length is 12 bits
  len -= 2;    // section_length is 12 bits
  if (sectionLen == len) {
    // printf("parse_ntt: found section_length %d.\n", sectionLen);
  } else {
    // printf("parse_ntt: invalid section_length field\n");
    return;
  }

  // Check zero and protocol_version fields
  if (len < 1)    // zero field is 3 bits and protocol_version is 5 bits
      return;
  if (*buf == 0) {
    // printf("parse_ntt: found good zero and protocol version fields.\n");
  } else {
    // printf("parse_ntt: invalid zero or protocol_version fields.\n");
    return;
  }
  buf++;    // zero field is 3 bits and protocol_version is 5 bits
  len--;    // zero field is 3 bits and protocol_version is 5 bits

  // Check ISO_639_language_code
  if (len < 3)    // ISO_639_language_code is three bytes
      return;
  // The language code should be 'eng' or 0xffffff.
  if ((*buf  == 'e') && (*(buf+1)  == 'n') && (*(buf+2)  == 'g')) {
    // printf("parse_ntt: found ISO_639_language_code eng.\n");
  } else if ((*buf  == 0xff) && (*buf+1  == 0xff) && (*buf+2  == 0xff)) {
    // printf("parse_ntt: found wild card ISO_639_language_code.\n");
  } else {
    // printf("parse_ntt: invalid ISO_639_language_code.\n");
    return;
  }
  buf += 3;    // ISO_639_language_code is three bytes
  len -= 3;    // ISO_639_language_code is three bytes

  // Check transmission_medium and table_subtype fields
  if (len < 1)    // transmission_medium and table_subtype fields are 4 bits each
      return;
  if (*buf == 0x06) {    // we are only interested in the table_subtype 6, which is a SNS
    // printf("parse_ntt: found good transmission_medium and table_subtype fields.\n");
  } else {
    // printf("parse_ntt: invalid transmission_medium and table_subtype fields.\n");
    return;
  }
  buf++;    // transmission_medium and table_subtype fields are 4 bits each
  len--;    // transmission_medium and table_subtype fields are 4 bits each

  snsLen = parse_sns(buf, len - 4); // The 4 byte CRC_32 is not part of the SNS

  if (snsLen == 0)    // Something is wrong with the SNS table parsing
    return;
  // printf("parse_ntt: SNS parsed %d of %d bytes OK.\n", snsLen, len);

  // There may be some descriptors after the SNS table, but we really don't care

}

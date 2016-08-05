/*
  stub.c: STUB protocol code

  Copyright (c) Ixia 2014
  All rights reserved.

*/

/*>

  STUB Protocol Implementation

  DESCRIPTION:
  Routines to implement STUB on TCP/UDP

<*/

#include <stdio.h>
#include <stdarg.h>
#include "mntcpos.h"
#include "stub.h"
#include "mntcpapp.h"

/* stub version v4/v6, default v4 */
ubyte stubIPVersion = STUB_IPV4;


/*>>

  (void)ByteReverse(ubyte *data,
                    byte4 count);

  REQUIRES:
  #include "framework.h"

  DESCRIPTION:
  Reverses the specified number of bytes in a block of data in place.

  ARGS:
  data  pointer to data to reverse.
  count number of bytes to reverse.

  RETURNS:
  (void)

  SIDE EFFECTS:

  EXAMPLE:

    ubyte data[25];

  ByteReverse(data, 25);

<<*/

static void
ByteReverse(ubyte *data, ubyte4 count)
{
  ubyte tmp;
  ubyte4 i;

  /* Swap the first half of the bytes with the last half */
  for(i = 0; i < count/2; i++){
  tmp = *(data + i);
  *(data + i) = *(data + count - i - 1);
  *(data + count - i - 1) = tmp;
  }
}


/*>

  byte4 numBytes = Unpack(ubyte *data,
                          byte   *format,
              ...);

  REQUIRES:
  #include "framework.h"

  DESCRIPTION:
  Copies raw packed bytes from a buffer into C data as specified by a
  format string. The format string has the general form of a sequence
  of "unpack instructions" that look like:

    <DataType><Length><Modifiers>

  where <DataType> specifies the size of the data to be unpacked and
  <Length> and <Modifiers> are optional. Not all data types support all
  modifiers. Spaces are not allowed except between "unpack instructions"
  where they are required.

  All numerical data is assumed to be in "network byte order" in raw form
  and is byte-swapped appropriately when unpacked.
  <DataType> can be one of:

  B    corresponding argument is ubyte *.
  S    corresponding argument is ubyte2 *.
  L    corresponding argument is ubyte4 *.
  F    corresponding argument is real4 *.
  D    corresponding argument is real8 *.
  LL   corresponding argument is long8

  <Length> specifies that the argument points to an array of the specified
  data type and that number of elements of the given data type will be
  unpacked into it. If this field is absent, it is taken to be 1. The <Length>
  field can be one of:

    <num>  a decimal number that is the length.
  *{BSL}  a '*' followed by an optional <DataType>. A '*' alone indicates
      that the length can be found in the next argument as a ubyte4.
      If the optional <DataType> is specified after the '*' then the
      next argument is a pointer to that data type and contains the
      length.

  <Modifiers> are operations that can be performed on the data as it is
  unpacked. In the case of multiple modifiers, they are applied in order.
  Allowable modifiers are:

    ~    byte swap - reverse the order of the bytes unpacked.
  m<hex>  mask - each unpacked item is &'d with the hex mask <hex>.
  <<num>  left shift - shift each unpacked item left <num> bits.
  ><num>  right shift - shift each unpacked item right <num> bits.

  In addition, the following <DataType>'s are supported that do not allow
  modifiers:

  A    A pointer to into the string being unpacked is returned.
  P    A pointer into the data being unpacked is returned.
  +    Skip forward <Length> bytes.
  -    Skip backward <Length> bytes.

  ARGS:
  data    pointer to data to be unpacked
  format  format string as described in DESCRIPTION section
  ...    pointers to arguments as specified by format

  RETURNS:
  numBytes       the number of bytes unpacked (not including skipped bytes)
  UBYTE4_MAXVAL   if there was an error unpacking (message to stderr)

  SIDE EFFECTS:

  EXAMPLE:

    ubyte b, buf[20], n1, n2;
  ubyte2 b2;
  byte4 status;

  Simple unpack:
  status = Unpack(data, "B B20 S", &b, buf, &b2);

  Unpack a buffer where first byte indicates number of bytes following:
    status = Unpack(data, "B B*B", &b, &b, buf);

  Unpack the high nibble and low nibble into separate variables:
  status = Unpack(data, "BmF0>4 -1 Bm0F", &n1, &n2);

<*/

ubyte4
Unpack(ubyte *data, byte *format, ...)
{
  va_list argPtr;
  byte *modPtr, *ignore;
  ubyte c, dataType, *ptr;
  ubyte4 mask, shiftUp, shiftDown, i, bytesUnpacked;
  ubyte4 len, size;
  boolean done, modDone;

  va_start(argPtr, format);
  bytesUnpacked = 0;

#ifdef OPTIMIZE
  /* fast-path code for L, S, B, P, +[0-9] */

  for (;;) {
  ubyte *p1;
  ubyte2 *p2;
  ubyte4 *p4;
  ubyte **pp;

  if (format[1] == ' ') {
    if (format[0] == 'S') {
    p2 = va_arg(argPtr, ubyte2 *);
    *p2 = peek2(data);
    format += 2;
    data += 2;
    bytesUnpacked += 2;
    }
    else if (format[0] == 'B') {
    p1 = va_arg(argPtr, ubyte *);
    *p1 = *data;
    format += 2;
    data += 1;
    bytesUnpacked++;
    }
    else if (format[0] == 'L') {
    p4 = va_arg(argPtr, ubyte4 *);
    *p4 = peek4(data);
    format += 2;
    data += 4;
    bytesUnpacked += 4;
    }
    else if (format[0] == 'P') {
    pp = va_arg(argPtr, ubyte **);
    *pp = data;
    format += 2;
    }
    else {
    break;
    }
  }
  else if (format[0] == '+' && format[2] == ' ') {
    /*
     Check if format[1] is a digit between 0 and 9.  If it is not,
     the unsigned value with be 10 or more.
     */
    len = format[1] - '0';
    if (len < 10){
    format += 3;
    data += len;
    bytesUnpacked += len;
    }
    else {
    break;
    }
  }
  else {
    break;
  }
  }
#endif /* OPTIMIZE */

  /* For each entry */
  done = FALSE;

  while((*format) && (!done)){
    c = dataType = *format;
  size = 0;
  len= 1;

  /* Determine data type to be unpacked */
  switch(c){
  case ' ':
  case '\t':
    /* Ignore whitespace */
    format++;
    continue;
  case 'B':
    /* Byte */
    size = sizeof(ubyte);
    break;
  case 'S':
    /* 2 Bytes */
    size = sizeof(ubyte2);
    break;
  case 'L':
    /* 4 Bytes */
    if(*(format + 1) == 'L') {
      size = sizeof(long8);
      format++;
    }
    else {
      size = sizeof(ubyte4);
    }
    break;
  case 'F':
    /* 4 Bytes */
    size = sizeof(real4);
    break;
  case 'D':
    /* 8 Bytes */
    size = sizeof(real8);
    break;
  case 'A':
    /* Ascii string (null terminated) */
    size = 0;
    break;
  case 'P':
    /* Pointer to data */
    size = 0;
    break;
  case '+':
  case '-':
    /* Skip forwards or backwards */
    break;
  default:
    FPrintf(stderr, "! Bad Type '%c' near '%s'\n", c, format);
    Exit(1);
    //Error(FATAL, "Bad Type '%c' near '%s'\n", c, format);
    bytesUnpacked = UBYTE4_MAXVAL;
    done = TRUE;
    continue;
  }

  format++;

  /* Get the len if there is one specified */
  c = *format;
  if(IsDigit(c)){
    len = (ubyte4)strtol(format, &ignore, 10);
    format++;
    while(IsDigit(c = *format)){
    format++;
    }
  }
  else if(c == '*'){
    ubyte  *lenPtr;

    format++;
    switch(c = *format){
    case 'B':
    lenPtr = va_arg(argPtr, ubyte *);
    len = *lenPtr;
    format++;
    break;
    case 'S':
    lenPtr = (ubyte *)va_arg(argPtr, ubyte2 *);
    len = *(ubyte2 *)lenPtr;
    format++;
    break;
    case 'L':
    if(size == sizeof(long8)) {
      lenPtr = (ubyte *)va_arg(argPtr, long8 *);
      len = *(long8 *)lenPtr;
    }
    else {
      lenPtr = (ubyte *)va_arg(argPtr, ubyte4 *);
      len = *(ubyte4 *)lenPtr;
    }
    format++;
    break;
    case 'F':
    lenPtr = (ubyte *)va_arg(argPtr, real4 *);
    len = *(real4 *)lenPtr;
    format++;
    break;
    case 'D':
    lenPtr = (ubyte *)va_arg(argPtr, real8 *);
    len = *(real8 *)lenPtr;
    format++;
    break;
    default:
    len = va_arg(argPtr, ubyte4);
    break;
    }
  }

  /* Handle skips */
  if(dataType == '+'){
    data += len;
    continue;
  }
  else if(dataType == '-'){
    data -= len;
    continue;
  }

  /* Unpack the data */
  ptr = va_arg(argPtr, ubyte *);

  /* Do bytes all at once not one at a time if no modifiers */
  if((size == 1) && ((*format == ' ') || (*format == '\t'))){
    size = len;
    len = 1;
  }

  modPtr = format;
  for(i = 0; i < len; i++){

    /* Pointer into packet requested, return it */
    if(dataType == 'P'){
    *(ubyte **)ptr = data;
    }
    else if(dataType == 'A'){
    *(byte **)ptr = (byte *)data;
    size = StrLen(*(byte **)ptr) + 1;
    }
    else{
    /* Unpack the data */
    MemMove(ptr, data, (Size_t)size);

    /* Byte reverse data that is to be numerical */
    if((dataType == 'S') || (dataType == 'L') || (dataType == 'F')
       || (dataType == 'D')){
      NetworkByteOrder(ptr, (Size_t)size);
    }

    /* Process modifiers */
    format = modPtr;
    modDone = FALSE;
    while(*format && (!modDone) && (!done)){
      mask = shiftUp = shiftDown = 0;

      /* Parse modifiers if type supports them */
      switch(c = *format){
      case ' ':
      case '\t':
      /* Whitespace indicates end of modifiers */
      modDone = TRUE;
      continue;
      case '~':
      /* Byte swap explicitly */
      ByteReverse(ptr, size);
      format++;
      break;
      case 'm':
      /* Mask the data */
      format++;
      mask = strtol(format, &format, 16);
      if(mask == 0){
        FPrintf(stderr, "! Bad mask near: '%s'\n", format);
        Exit(1);
        //Error(FATAL, "Bad mask near: '%s'\n", format);
        bytesUnpacked = UBYTE4_MAXVAL;
        done = TRUE;
        continue;
      }
      /* More data, more mask */
      switch(dataType){
      case 'B':
        *ptr &= (ubyte)mask;
        break;
      case 'S':
        *(ubyte2 *)ptr &= (ubyte2)mask;
        break;
      case 'L':
        if(size == sizeof(long8)) {
          *(long8 *)ptr &= mask;
        }
        else {
          *(ubyte4 *)ptr &= mask;
        }
        break;
      default:
        FPrintf(stderr, "! Bad mask data size %ld near '%s'\n",
          size, format);
        Exit(1);
        //Error(FATAL, "Bad mask data size %ld near '%s'\n",
        //  size, format);
        bytesUnpacked = UBYTE4_MAXVAL;
        done = TRUE;
        continue;
      }
      break;
      case '<':
      /* Shift data left */
      format++;
      shiftUp = strtol(format, &format, 10);
      if(shiftUp == 0){
        FPrintf(stderr, "! Bad length near: '%s'\n", format);
        Exit(1);
        //Error(FATAL, "Bad length near: '%s'\n", format);
        bytesUnpacked = UBYTE4_MAXVAL;
        done = TRUE;
        continue;
      }
      switch(dataType){
      case 'B':
        *ptr <<= (ubyte)shiftUp;
        break;
      case 'S':
        *(ubyte2 *)ptr <<= (ubyte2)shiftUp;
        break;
      case 'L':
        if(size == sizeof(long8)) {
          *(long8 *)ptr <<= shiftUp;
        }
        else {
          *(ubyte4 *)ptr <<= shiftUp;
        }
        break;
      default:
        FPrintf(stderr, "! Bad shift left data size %ld near '%s'\n",
          size, format);
        Exit(1);
        //Error(FATAL, "Bad shift left data size %ld near '%s'\n",
          //size, format);
        bytesUnpacked = UBYTE4_MAXVAL;
        done = TRUE;
        continue;
      }
      break;
      case '>':
      /* Shift right */
      format++;
      shiftDown = strtol(format, &format, 10);
      if(shiftDown == 0){
        FPrintf(stderr, "! Bad length near: '%s'\n", format);
        Exit(1);
        //Error(FATAL, "Bad length near: '%s'\n", format);
        bytesUnpacked = UBYTE4_MAXVAL;
        done = TRUE;
        continue;
      }
      switch(dataType){
      case 'B':
        *ptr >>= (ubyte)shiftDown;
        break;
      case 'S':
        *(ubyte2 *)ptr >>= (ubyte2)shiftDown;
        break;
      case 'L':
        if(size == sizeof(long8)) {
          *(long8 *)ptr >>= shiftDown;
        }
        else {
          *(ubyte4 *)ptr >>= shiftDown;
        }
        break;
      default:
        FPrintf(stderr, "! Bad shift right data size %ld near '%s'\n",
          size, format);
        Exit(1);
    //    Error(FATAL, "Bad shift right data size %ld near '%s'\n",
     //     size, format);
        bytesUnpacked = UBYTE4_MAXVAL;
        done = TRUE;
        continue;
      }
      break;
      default:
        FPrintf(stderr, "! Bad modifier '%c' near '%s'\n", c, format);
        Exit(1);
      //Error(FATAL, "Bad modifier '%c' near '%s'\n", c, format);
      bytesUnpacked = UBYTE4_MAXVAL;
      done = TRUE;
      continue;
      }
    }
    /* Sometimes you're just tempted to use goto... */
    if(done){
      continue;
    }
    }

    /* Move pointers and keep track of bytes unpacked */
    ptr += size;
    data += size;
    bytesUnpacked += size;
  }
  }

  va_end(argPtr);
  return bytesUnpacked;
}

/*>

  ubyte4 numBytes = Pack(ubyte *data,
                        byte  *format,
            ...);

  REQUIRES:
  #include "framework.h"

  DESCRIPTION:
  Copies C data into raw packed bytes from a buffer as specified by a
  format string. The format string has the same form as that specified
  in Unpack() with the following changes:

  <DataType>
  Pack() does not allow the P data type (it would be meaningless).
  Pack defines the additional data type:

    X  pack <Length> zeroes into data.

  <Modifiers>
  The mask and shift modifiers are not supported in Pack() as they are
  easily accomodated on the function call line using C.
  Format Specifier "LL" Packs variable of type long8

  ARGS:
  data    pointer to buffer to pack data into.
  format  format string as described in DESCRIPTION section
  ...    pointers to arguments as specified by format

  RETURNS:
  numBytes        the number of bytes packed (not including skipped bytes)
  UBYTE4_MAXVAL    if there was an error packing (message to stderr)

  SIDE EFFECTS:

  EXAMPLE:

    ubyte b, buf[20];

  status = Pack(data, "B B20 S", &b, buf, &b2);

<*/

ubyte4
Pack(ubyte *data, byte *format, ...)
{
  va_list argPtr;
  byte *modPtr, *ignore;
  ubyte c, dataType, *ptr;
  ubyte4  mask, shiftUp, shiftDown, i, bytesPacked;
  ubyte4  size, len;
  boolean done, modDone;

  va_start(argPtr, format);
  bytesPacked = 0;

#ifdef OPTIMIZE
  /* fast-path code - handle L, S, B, B*, B[0-9] */
  for (;;) {
  ubyte4 *p4;
  ubyte2 *p2;
  ubyte *p;

  if (format[1] == ' ') {
    if (format[0] == 'S') {
    p2 = va_arg(argPtr, ubyte2 *);
    poke2(data, *p2);
    data += 2;
    bytesPacked += 2;
    format += 2;
    }
    else if (format[0] == 'B') {
    p = va_arg(argPtr, ubyte *);
    *data = *p;
    data++;
    bytesPacked++;
    format += 2;
    }
    else if (format[0] == 'L') {
    p4 = va_arg(argPtr, ubyte4 *);
    poke4(data, *p4);
    data += 4;
    bytesPacked += 4;
    format += 2;
    }
    else {
    break;
    }
  }
  else if (format[0] == 'B' && format[2] == ' ') {
    /*
     Check if format[1] is a digit between 0 and 9.  If it is not,
     the unsigned value with be 10 or more.
     */
    len = format[1] - '0';
    if (len < 10){
    p = va_arg(argPtr, ubyte *);
    memcpy(data, p, len);
    data += len;
    bytesPacked += len;
    format += 3;
    }
    else if (format[1] == '*') {
    len = va_arg(argPtr, ubyte4);
    p = va_arg(argPtr, ubyte *);
    memcpy(data, p, len);
    data += len;
    bytesPacked += len;
    format += 3;
    }
    else {
    break;
    }
  }
  else {
    break;
  }
  }
#endif /* OPTIMIZE */

  done = FALSE;
  /* For each entry */
  while((*format) && (!done)){
    c = dataType = *format;
  size = 0;
  len = 1;

  /* Determine data type to be unpacked */
  switch(c){
  case ' ':
  case '\t':
    /* Ignore whitespace */
    format++;
    continue;
  case 'B':
    /* Byte */
    size = sizeof(ubyte);
    break;
  case 'S':
    /* 2 Bytes */
    size = sizeof(ubyte2);
    break;
  case 'L':
    /* 4 Bytes */
    if(*(format+1) == 'L') {
      size = sizeof(long8);
      format++;
    }
    else {
      size = sizeof(ubyte4);
    }
    break;
  case 'F':
    /* 4 Bytes */
    size = sizeof(real4);
    break;
  case 'D':
    /* 8 Bytes */
    size = sizeof(real8);
    break;
  case 'X':
    /* Pack 0's */
    size = sizeof(ubyte);
    break;
  case 'A':
    /* Ascii string (null terminated) */
    size = 0;
    break;
  case '+':
  case '-':
    /* Skip forwards or backwards */
    break;
  default:
    FPrintf(stderr, "! Bad Function '%c' near '%s'\n", c, format);
    Exit(1);
    //Error(FATAL, "Bad Function '%c' near '%s'\n", c, format);
    bytesPacked = UBYTE4_MAXVAL;
    done = TRUE;
    continue;
  }

  format++;

  /* Get the len if there is one specified */
  c = *format;
  if(IsDigit(c)){
    len = (ubyte4)strtol(format, &ignore, 10);
    format++;
    while(IsDigit(c = *format)){
    format++;
    }
  }
  else if(c == '*'){
    ubyte  *lenPtr;

    format++;
    switch(c = *format){
    case 'B':
    lenPtr = va_arg(argPtr, ubyte *);
    len = *lenPtr;
    format++;
    break;
    case 'S':
    lenPtr = (ubyte *)va_arg(argPtr, ubyte2 *);
    len = *(ubyte2 *)lenPtr;
    format++;
    break;
    case 'L':
    if(size == sizeof(long8)) {
      lenPtr = (ubyte *)va_arg(argPtr, long8 *);
      len = *(long8 *)lenPtr;
    }
    else {
      lenPtr = (ubyte *)va_arg(argPtr, ubyte4 *);
      len = *(ubyte4 *)lenPtr;
    }
    format++;
    break;
    case 'F':
    lenPtr = (ubyte *)va_arg(argPtr, real4 *);
    len = *(real4 *)lenPtr;
    format++;
    break;
    case 'D':
    lenPtr = (ubyte *)va_arg(argPtr, real8 *);
    len = *(real8 *)lenPtr;
    format++;
    break;
    default:
    len = va_arg(argPtr, ubyte4);
    break;
    }
  }

  /* Handle skips */
  if(dataType == '+'){
    data += len;
    bytesPacked += len;
    continue;
  }
  else if(dataType == '-'){
    data -= len;
    bytesPacked -= len;
    continue;
  }
  else if(dataType == 'X'){
    MemSet(data, '\0', (Size_t)len);
    data += len;
    bytesPacked += len;
    continue;
  }

  /* Pack the data */
  ptr = va_arg(argPtr, ubyte *);

  /* Do bytes all at once not one at a time if no modifiers */
  if((size == 1) &&
     ((*format == '\0') || (*format == ' ') || (*format == '\t'))){
    size = len;
    len = 1;
  }

  modPtr = format;
  for(i = 0; i < len; i++){

    /* Pointer into packet requested, return it */
    if(dataType == 'A'){
    StrCpy((byte *)data, (byte *)ptr);
    size = StrLen((byte *)ptr) + 1;
    }
    else{
    /* Pack the data */
    MemMove(data, ptr, (Size_t)size);

    /* Byte reverse data that is numerical */
    if((dataType == 'S') || (dataType == 'L') || (dataType == 'F')
       || (dataType == 'D')){
      NetworkByteOrder(data, (Size_t)size);
    }

    /* Process modifiers */
    format = modPtr;
    modDone = FALSE;
    while(*format && (!modDone) && (!done)){
      mask = shiftUp = shiftDown = 0;

      /* Parse modifiers if type supports them */
      switch(c = *format){
      case ' ':
      case '\t':
      /* Whitespace indicates end of modifiers */
      modDone = TRUE;
      continue;
      case '~':
      /* Byte swap explicitly */
      ByteReverse(data, size);
      format++;
      break;
      default:
      FPrintf(stderr, "! Bad modifier '%c' near '%s'\n", c, format);
      Exit(1);
      //Error(FATAL, "Bad modifier '%c' near '%s'\n", c, format);
      bytesPacked = UBYTE4_MAXVAL;
      done = TRUE;
      continue;
      }
    }
    /* Sometimes you're just tempted to use goto... */
    if(done){
      continue;
    }
    }

    /* Move pointers and keep track of bytes unpacked */
    ptr += size;
    data += size;
    bytesPacked += size;
  }
  }

  va_end(argPtr);
  return bytesPacked;
}

/*>

  STUBForm_t *form  = STUBFormCreate();

  REQUIRES:

  DESCRIPTION:
  This function creates a data structure representing a packet that will be exchanged between ANVL and the stub application running on DUT.

  ARGS:

  RETURNS:
  form  ptr to a new (cleared) data structure

  SIDE EFFECTS:

<*/

STUBForm_t *
STUBFormCreate()
{
  STUBForm_t *form = 0;

  form = (STUBForm_t *)Malloc(sizeof(STUBForm_t));
  CLEAR_DATA(form);
  return form;
}

/*>

  STUBFormDestroy(STUBForm_t *form)

  REQUIRES:

  DESCRIPTION:
  Frees the data structure that contains all the fields of a STUB packet.

  ARGS:
  form      pointer to a data structure representing a STUB packet.

  RETURNS:
  void

  SIDE EFFECTS:

<*/

void
STUBFormDestroy(STUBForm_t *form)
{
  if(!form) {
    FPrintf(stderr, "! Invalid STUB form\n");
    Exit(1);
  }
  Free(form);
  form = 0;
}

/*>>

  void STUBPacketToForm(ubyte   *pktdata,
                       STUBForm_t   *form);

  REQUIRES:

  DESCRIPTION:
  Unpacks bytes out of a packet into a form, usually for printing or
  checking.

  ARGS:
  pkt   pointer to packet containing STUB packet.
  form  pointer to form to stuff values into.

  RETURNS:

  SIDE EFFECTS:

<<*/

void
STUBPacketToForm(ubyte *pktdata, STUBForm_t *form)
{

  Unpack(pktdata,
         "S S S S P",
         &form->msgType,
         &form->cmdID,
         &form->reserved,
         &form->paramsLen,
         &form->params);
}

/*>

  ubyte4 size = STUBBuild(STUBForm_t    *form,
                        ubyte       *buffer);

  REQUIRES:

  DESCRIPTION:
  Packs the data in a STUBForm data structure into network byte order
  to be passed to IP.

  ARGS:
  form      pointer to packet template.
  buffer    pointer to memory in which to pack the data.

  RETURNS:
  size      number of bytes packed into packet.

  SIDE EFFECTS:

<*/

ubyte4 STUBBuild(STUBForm_t *form, ubyte *buffer)
{
    return Pack(buffer,"S S S S",
              &form->msgType,
              &form->cmdID,
              &form->reserved,
              &form->paramsLen);
}

/*>

  GetVerRespParamForm_t *form  = GetVerRespParamFormCreate();

  REQUIRES:

  DESCRIPTION:
  This function creates a data structure representing the parameters field of GET_VERSION
  response message.

  ARGS:

  RETURNS:
  form  ptr to a new (cleared) data structure

  SIDE EFFECTS:

<*/

GetVerRespParamForm_t *GetVerRespParamFormCreate()
{
  GetVerRespParamForm_t *form = 0;

  form = (GetVerRespParamForm_t *)Malloc(sizeof(GetVerRespParamForm_t));
  CLEAR_DATA(form);
  return form;
}

/*>

  GetVerRespParamFormDestroy(GetVerRespParamForm_t *form)

  REQUIRES:

  DESCRIPTION:
  Frees the data structure that contains all the subfields in the parameters
  field of a GET_VERSION command message.

  ARGS:
  form   pointer to a data structure representing parameters subfield of a GET_VERSION reponse message

  RETURNS:
  void

  SIDE EFFECTS:

<*/

void
GetVerRespParamFormDestroy(GetVerRespParamForm_t *form)
{
  if(!form) {
    FPrintf(stderr, "! Invalid GetVerRespParam form\n");
    Exit(1);
  }
  Free(form);
  form = 0;
}

/*>>

  void GetVerRespParamToForm(ubyte *pktdata,
                       GetVerRespParamForm_t *form);

  REQUIRES:

  DESCRIPTION:
  Unpacks bytes out of a packet into a form, usually for printing or
  checking.

  ARGS:
  pktdata   buffer containing STUB parmeters.
  form  pointer to form to stuff values into.

  RETURNS:

  SIDE EFFECTS:

<<*/

void
GetVerRespParamToForm(ubyte *pktdata, GetVerRespParamForm_t *form)
{
  Unpack(pktdata,
         "S S",
         &form->majorVer,
         &form->minorVer);
}

/*>

  ubyte4 size = GetVerRespParamBuild(GetVerRespParamForm_t *form,
                        ubyte       *buffer);

  REQUIRES:

  DESCRIPTION:
  Packs the data in a GetVerRespParamForm data structure into network byte order
  to be passed to UDP.

  ARGS:
  form      pointer to packet template.
  buffer    pointer to memory in which to pack the data.

  RETURNS:
  size      number of bytes packed into packet.

  SIDE EFFECTS:

<*/

ubyte4 GetVerRespParamBuild(GetVerRespParamForm_t *form, ubyte *buffer)
{
  return Pack(buffer,"S S",
              &form->majorVer,
              &form->minorVer);
}

/*>

  ListenReqParamForm_t *form  = ListenReqParamFormCreate();

  REQUIRES:

  DESCRIPTION:
  This function creates a data structure representing the parameters field of Listen
  command message.

  ARGS:

  RETURNS:
  form  ptr to a new (cleared) data structure

  SIDE EFFECTS:

<*/

ListenReqParamForm_t *ListenReqParamFormCreate()
{
  ListenReqParamForm_t *form = 0;

  form = (ListenReqParamForm_t *)Malloc(sizeof(ListenReqParamForm_t));
  CLEAR_DATA(form);
  return form;
}

/*>

  ListenReqParamFormDestroy(ListenReqParamForm_t *form)

  REQUIRES:

  DESCRIPTION:
  Frees the data structure that contains all the subfields in the parameters
  field of a Listen command message.

  ARGS:
  form   pointer to a data structure representing parameters subfield of a Listen command message

  RETURNS:
  void

  SIDE EFFECTS:

<*/

void
ListenReqParamFormDestroy(ListenReqParamForm_t *form)
{
  //ASSERT_MSG(form, "! Invalid STUB form\n");
  if(!form) {
    FPrintf(stderr, "! Invalid ListenReqParam form '%s'\n");
    Exit(1);
  }
  Free(form);
  form = 0;
}

/*>>

  void ListenReqParamToForm(ubyte *pktdata,
                       ListenReqParamForm_t *form);

  REQUIRES:

  DESCRIPTION:
  Unpacks bytes out of a packet into a form, usually for printing or
  checking.

  ARGS:
  pktdata   buffer containing STUB parmeters.
  form  pointer to form to stuff values into.

  RETURNS:

  SIDE EFFECTS:

<<*/

void
ListenReqParamToForm(ubyte *pktdata, ListenReqParamForm_t *form)
{
  Unpack(pktdata,
         "S S",
         &form->portNum,
         &form->reserved);
}

/*>

  ubyte4 size = ListenReqParamBuild(ListenReqParamForm_t *form,
                        ubyte       *buffer);

  REQUIRES:

  DESCRIPTION:
  Packs the data in a ListenReqParamForm data structure into network byte order
  to be passed to UDP.

  ARGS:
  form      pointer to packet template.
  buffer    pointer to memory in which to pack the data.

  RETURNS:
  size      number of bytes packed into packet.

  SIDE EFFECTS:

<*/

ubyte4 ListenReqParamBuild(ListenReqParamForm_t *form, ubyte *buffer)
{
  return Pack(buffer,"S S",
              &form->portNum,
              &form->reserved);
}

/*>

  StartEndTestReqParamForm_t *form  = StartEndTestReqParamFormCreate();

  REQUIRES:

  DESCRIPTION:
  This function creates a data structure representing the parameters field of Start_Test/End_Test
  command message.

  ARGS:

  RETURNS:
  form  ptr to a new (cleared) data structure

  SIDE EFFECTS:

<*/

StartEndTestReqParamForm_t *StartEndTestReqParamFormCreate()
{
  StartEndTestReqParamForm_t *form = 0;

  form = (StartEndTestReqParamForm_t *)Malloc(sizeof(StartEndTestReqParamForm_t));
  CLEAR_DATA(form);
  return form;
}

/*>

  StartEndTestReqParamFormDestroy(StartEndTestReqParamForm_t *form)

  REQUIRES:

  DESCRIPTION:
  Frees the data structure that contains all the subfields in the parameters
  field of a Listen command message.

  ARGS:
  form   pointer to a data structure representing parameters subfield of a Start_test/End_Test command message

  RETURNS:
  void

  SIDE EFFECTS:

<*/

void
StartEndTestReqParamFormDestroy(StartEndTestReqParamForm_t *form)
{
  //ASSERT_MSG(form, "! Invalid STUB form\n");
  if(!form) {
    FPrintf(stderr, "! Invalid StartEndTestReqParam form '%s'\n");
    Exit(1);
  }
  Free(form);
  form = 0;
}

/*>>

  void StartEndTestReqParamToForm(ubyte *pktdata,
                       StartEndTestReqParamForm_t   *form);

  REQUIRES:

  DESCRIPTION:
  Unpacks bytes out of a packet into a form, usually for printing or
  checking.

  ARGS:
  pktdata   buffer containing STUB parmeters.
  form  pointer to form to stuff values into.

  RETURNS:

  SIDE EFFECTS:

<<*/

void
StartEndTestReqParamToForm(ubyte *pktdata, StartEndTestReqParamForm_t *form)
{
  Unpack(pktdata,
         "S S L P",
         &form->majorNum,
         &form->minorNum,
         &form->suiteNameLen,
         &form->suiteName);

  Unpack(pktdata + 8 + form->suiteNameLen,
         "P", &form->padding);
}

/*>

  ubyte4 size = StartEndTestReqParamBuild(StartEndTestReqParamForm_t *form,
                        ubyte       *buffer);

  REQUIRES:

  DESCRIPTION:
  Packs the data in a StartEndTestReqParamForm data structure into network byte order
  to be passed to UDP.

  ARGS:
  form      pointer to packet template.
  buffer    pointer to memory in which to pack the data.

  RETURNS:
  size      number of bytes packed into packet.

  SIDE EFFECTS:

<*/

ubyte4 StartEndTestReqParamBuild(StartEndTestReqParamForm_t *form, ubyte *buffer, ubyte2 paddingLen)
{
  return Pack(buffer,"S S L B* B*",
              &form->majorNum,
              &form->minorNum,
                          &form->suiteNameLen,
                          form->suiteNameLen, form->suiteName,
                          paddingLen, form->padding);
}

/*>

  GetSampleDataRespParamForm_t *form  = GetSampleDataRespParamFormCreate();

  REQUIRES:

  DESCRIPTION:
  This function creates a data structure representing the parameters field of GET_SAMPLE_DATA
  response message.

  ARGS:

  RETURNS:
  form  ptr to a new (cleared) data structure

  SIDE EFFECTS:

<*/

GetSampleDataRespParamForm_t *GetSampleDataRespParamFormCreate()
{
  GetSampleDataRespParamForm_t *form = 0;

  form = (GetSampleDataRespParamForm_t *)Malloc(sizeof(GetSampleDataRespParamForm_t));
  CLEAR_DATA(form);
  return form;
}

/*>

  GetSampleDataRespParamFormDestroy(GetSampleDataRespParamForm_t *form)

  REQUIRES:

  DESCRIPTION:
  Frees the data structure that contains all the subfields in the parameters
  field of a GET_SAMPLE_DATA command message.

  ARGS:
  form   pointer to a data structure representing parameters subfield of a GET_SAMPLE_DATA response message

  RETURNS:
  void

  SIDE EFFECTS:

<*/

void
GetSampleDataRespParamFormDestroy(GetSampleDataRespParamForm_t *form)
{
  if(!form) {
    FPrintf(stderr, "! Invalid GetSampleDataRespParam form\n");
    Exit(1);
  }
  Free(form);
  form = 0;
}

/*>>

  void GetSampleDataRespParamToForm(ubyte *pktdata,
                       GetSampleDataRespParamForm_t *form);

  REQUIRES:

  DESCRIPTION:
  Unpacks bytes out of a packet into a form, usually for printing or
  checking.

  ARGS:
  pktdata   buffer containing STUB parmeters.
  form  pointer to form to stuff values into.

  RETURNS:

  SIDE EFFECTS:

<<*/

void
GetSampleDataRespParamToForm(ubyte *pktdata, GetSampleDataRespParamForm_t *form)
{
  Unpack(pktdata,
         "L B* P",
         &form->dataLen,
         form->dataLen, form->data,
         &form->padding);
}

/*>

  ubyte4 size = GetSampleDataRespParamBuild(GetSampleDataRespParamForm_t *form,
                        ubyte       *buffer);

  REQUIRES:

  DESCRIPTION:
  Packs the data in a GetSampleDataRespParamForm data structure into network byte order
  to be passed to UDP.

  ARGS:
  form      pointer to packet template.
  buffer    pointer to memory in which to pack the data.

  RETURNS:
  size      number of bytes packed into packet.

  SIDE EFFECTS:

<*/
ubyte4 GetSampleDataRespParamBuild(GetSampleDataRespParamForm_t *form, ubyte *buffer, ubyte2 paddingLen)
{
  ubyte4 len = 0;
  len =  Pack(buffer,"L",
              &form->dataLen);
  if(form->dataLen != 0) {
    len += Pack(buffer + len, "B* B*", form->dataLen, form->data, paddingLen, form->padding);
  }
  return len;
}

/*>

  GetDataLenRespParamForm_t *form  = GetDataLenRespParamFormCreate();

  REQUIRES:

  DESCRIPTION:
  This function creates a data structure representing the parameters field of GET_DATA_LEN
  response message.

  ARGS:

  RETURNS:
  form  ptr to a new (cleared) data structure

  SIDE EFFECTS:

<*/

GetDataLenRespParamForm_t *GetDataLenRespParamFormCreate()
{
  GetDataLenRespParamForm_t *form = 0;

  form = (GetDataLenRespParamForm_t *)Malloc(sizeof(GetDataLenRespParamForm_t));
  CLEAR_DATA(form);
  return form;
}

/*>

  GetDataLenRespParamFormDestroy(GetDataLenRespParamForm_t *form)

  REQUIRES:

  DESCRIPTION:
  Frees the data structure that contains all the subfields in the parameters
  field of a GET_DATA_LEN command message.

  ARGS:
  form   pointer to a data structure representing parameters subfield of a GET_DATA_LEN reponse message

  RETURNS:
  void

  SIDE EFFECTS:

<*/

void
GetDataLenRespParamFormDestroy(GetDataLenRespParamForm_t *form)
{
  if(!form) {
    FPrintf(stderr, "! Invalid GetDataLenRespParam form\n");
    Exit(1);
  }
  Free(form);
  form = 0;
}

/*>>

  void GetDataLenRespParamToForm(ubyte *pktdata,
                       GetDataLenRespParamForm_t *form);

  REQUIRES:

  DESCRIPTION:
  Unpacks bytes out of a packet into a form, usually for printing or
  checking.

  ARGS:
  pktdata   buffer containing STUB parmeters.
  form  pointer to form to stuff values into.

  RETURNS:

  SIDE EFFECTS:

<<*/

void
GetDataLenRespParamToForm(ubyte *pktdata, GetDataLenRespParamForm_t *form)
{
  Unpack(pktdata,
         "S S",
         &form->dataLen,
         &form->checksum);
}

/*>

  ubyte4 size = GetDataLenRespParamBuild(GetDataLenRespParamForm_t *form,
                        ubyte       *buffer);

  REQUIRES:

  DESCRIPTION:
  Packs the data in a GetDataLenRespParamForm data structure into network byte order
  to be passed to UDP.

  ARGS:
  form      pointer to packet template.
  buffer    pointer to memory in which to pack the data.

  RETURNS:
  size      number of bytes packed into packet.

  SIDE EFFECTS:

<*/

ubyte4 GetDataLenRespParamBuild(GetDataLenRespParamForm_t *form, ubyte *buffer)
{
  return Pack(buffer,"S S",
              &form->dataLen,
              &form->checksum);
}

/*>

  ConnectReqParamForm_t *form  = ConnectReqParamFormCreate();

  REQUIRES:

  DESCRIPTION:
  This function creates a data structure representing the parameters field of CONNECT
  command message.

  ARGS:

  RETURNS:
  form  ptr to a new (cleared) data structure

  SIDE EFFECTS:

<*/

ConnectReqParamForm_t *ConnectReqParamFormCreate()
{
  ConnectReqParamForm_t *form = 0;

  form = (ConnectReqParamForm_t *)Malloc(sizeof(ConnectReqParamForm_t));
  CLEAR_DATA(form);
  return form;
}

/*>

  ConnectReqParamFormDestroy(ConnectReqParamForm_t *form)

  REQUIRES:

  DESCRIPTION:
  Frees the data structure that contains all the subfields in the parameters
  field of a CONNECT command message.

  ARGS:
  form   pointer to a data structure representing parameters subfield of a CONNECT command message

  RETURNS:
  void

  SIDE EFFECTS:

<*/

void
ConnectReqParamFormDestroy(ConnectReqParamForm_t *form)
{
  if(!form) {
    FPrintf(stderr, "! Invalid ConnectReqParam form\n");
    Exit(1);
  }
  Free(form);
  form = 0;
}

/*>>

  void ConnectReqParamToForm(ubyte *pktdata,
                       ConnectReqParamForm_t *form);

  REQUIRES:

  DESCRIPTION:
  Unpacks bytes out of a packet into a form, usually for printing or
  checking.

  ARGS:
  pktdata   buffer containing STUB parmeters.
  form  pointer to form to stuff values into.

  RETURNS:

  SIDE EFFECTS:

<<*/

void
ConnectReqParamToForm(ubyte *pktdata, ConnectReqParamForm_t *form)
{
    switch (stubIPVersion) {
        case STUB_IPV6:
            Unpack(pktdata,
                    "B16 L",
                    &form->anvlIPv6Addr,
                    &form->portNum);
            break;
        case STUB_IPV4:
            Unpack(pktdata,
                    "L L",
                    &form->anvlIPAddr,
                    &form->portNum);
            break;
        default:
            break;
    }
}

/*>

  ubyte4 size = ConnectReqParamBuild(ConnectReqParamForm_t *form,
                        ubyte       *buffer);

  REQUIRES:

  DESCRIPTION:
  Packs the data in a ConnectReqParamForm data structure into network byte order
  to be passed to UDP.

  ARGS:
  form      pointer to packet template.
  buffer    pointer to memory in which to pack the data.

  RETURNS:
  size      number of bytes packed into packet.

  SIDE EFFECTS:

<*/

ubyte4 ConnectReqParamBuild(ConnectReqParamForm_t *form, ubyte *buffer)
{
    switch (stubIPVersion) {
        case STUB_IPV6:
            return Pack(buffer,"B16 L",
                    &form->anvlIPv6Addr,
                    &form->portNum);
            break;
        case STUB_IPV4:
            return Pack(buffer,"L L",
                    &form->anvlIPAddr,
                    &form->portNum);
            break;
        default:
            break;
    }

    return 0;
}

/*>

  SendRptDataReqParamForm_t *form  = SendRptDataReqParamFormCreate();

  REQUIRES:

  DESCRIPTION:
  This function creates a data structure representing the parameters field of SEND_REPEAT_DATA
  command message.

  ARGS:

  RETURNS:
  form  ptr to a new (cleared) data structure

  SIDE EFFECTS:

<*/

SendRptDataReqParamForm_t *SendRptDataReqParamFormCreate()
{
  SendRptDataReqParamForm_t *form = 0;

  form = (SendRptDataReqParamForm_t *)Malloc(sizeof(SendRptDataReqParamForm_t));
  CLEAR_DATA(form);
  return form;
}

/*>

  SendRptDataReqParamFormDestroy(SendRptDataReqParamForm_t *form)

  REQUIRES:

  DESCRIPTION:
  Frees the data structure that contains all the subfields in the parameters
  field of a SEND_REPEAT_DATA command message.

  ARGS:
  form   pointer to a data structure representing parameters subfield of a SEND_REPEAT_DATA command message

  RETURNS:
  void

  SIDE EFFECTS:

<*/

void
SendRptDataReqParamFormDestroy(SendRptDataReqParamForm_t *form)
{
  if(!form) {
    FPrintf(stderr, "! Invalid SendRptDataReqParam form\n");
    Exit(1);
  }
  Free(form);
  form = 0;
}

/*>>

  void SendRptDataReqParamToForm(ubyte *pktdata,
                       SendRptDataReqParamForm_t *form);

  REQUIRES:

  DESCRIPTION:
  Unpacks bytes out of a packet into a form, usually for printing or
  checking.

  ARGS:
  pktdata   buffer containing STUB parmeters.
  form  pointer to form to stuff values into.

  RETURNS:

  SIDE EFFECTS:

<<*/

void
SendRptDataReqParamToForm(ubyte *pktdata, SendRptDataReqParamForm_t *form)
{
    switch (stubIPVersion) {
        case STUB_IPV6:
            Unpack(pktdata,
                    "B16 S B S P +1 S",
                    &form->anvlIPv6Addr,
                    &form->portNum,
                    &form->flags,
                    &form->dataLen,
                    &form->data,
                    &form->reserved);
            break;
        case STUB_IPV4:
            Unpack(pktdata,
                    "L S B S P +1 S",
                    &form->anvlIPAddr,
                    &form->portNum,
                    &form->flags,
                    &form->dataLen,
                    &form->data,
                    &form->reserved);
            break;
        default:
            break;
    }
}

/*>

  ubyte4 size = SendRptDataReqParamBuild(SendRptDataReqParamForm_t *form,
                        ubyte       *buffer);

  REQUIRES:

  DESCRIPTION:
  Packs the data in a SendRptDataReqParamForm data structure into network byte order
  to be passed to UDP.

  ARGS:
  form      pointer to packet template.
  buffer    pointer to memory in which to pack the data.

  RETURNS:
  size      number of bytes packed into packet.

  SIDE EFFECTS:

<*/

ubyte4 SendRptDataReqParamBuild(SendRptDataReqParamForm_t *form, ubyte *buffer)
{
    switch (stubIPVersion) {
        case STUB_IPV6:
            return Pack(buffer,"B16 S B S B* S",
                    &form->anvlIPv6Addr,
                    &form->portNum,
                    &form->flags,
                    &form->dataLen,
                    1, form->data,
                    &form->reserved);
            break;
        case STUB_IPV4:
            return Pack(buffer,"L S B S B* S",
                    &form->anvlIPAddr,
                    &form->portNum,
                    &form->flags,
                    &form->dataLen,
                    1, form->data,
                    &form->reserved);
            break;
        default:
            break;
    }

    return 0;
}

/*>

  SendDataReqParamForm_t *form  = SendDataReqParamFormCreate();

  REQUIRES:

  DESCRIPTION:
  This function creates a data structure representing the parameters field of SEND
  command message.

  ARGS:

  RETURNS:
  form  ptr to a new (cleared) data structure

  SIDE EFFECTS:

<*/

SendDataReqParamForm_t *SendDataReqParamFormCreate()
{
  SendDataReqParamForm_t *form = 0;

  form = (SendDataReqParamForm_t *)Malloc(sizeof(SendDataReqParamForm_t));
  CLEAR_DATA(form);
  return form;
}

/*>

  SendDataReqParamFormDestroy(SendDataReqParamForm_t *form)

  REQUIRES:

  DESCRIPTION:
  Frees the data structure that contains all the subfields in the parameters
  field of a SEND command message.

  ARGS:
  form   pointer to a data structure representing parameters subfield of a SEND command message

  RETURNS:
  void

  SIDE EFFECTS:

<*/

void
SendDataReqParamFormDestroy(SendDataReqParamForm_t *form)
{
  if(!form) {
    FPrintf(stderr, "! Invalid SendDataReqParam form\n");
    Exit(1);
  }
  Free(form);
  form = 0;
}

/*>>

  void SendDataReqParamToForm(ubyte *pktdata,
                       SendDataReqParamForm_t *form);

  REQUIRES:

  DESCRIPTION:
  Unpacks bytes out of a packet into a form, usually for printing or
  checking.

  ARGS:
  pktdata   buffer containing STUB parmeters.
  form  pointer to form to stuff values into.

  RETURNS:

  SIDE EFFECTS:

<<*/

void
SendDataReqParamToForm(ubyte *pktdata, SendDataReqParamForm_t *form)
{
    switch (stubIPVersion) {
        case STUB_IPV6:
            Unpack(pktdata,
                    "B16 S B B L",
                    &form->anvlIPv6Addr,
                    &form->portNum,
                    &form->flags,
                    &form->reserved,
                    &form->dataLen);

            form->data = (ubyte *)malloc((form->dataLen)+1);
            Unpack(pktdata + sizeof(ubyte16) + sizeof(ubyte2) + sizeof(ubyte) +
                    sizeof(ubyte) + sizeof(ubyte4), "B* P",
                    form->dataLen, form->data,
                    &form->padding);
            form->data[form->dataLen] = '\0';
            break;
        case STUB_IPV4:
            Unpack(pktdata,
                    "L S B B L",
                    &form->anvlIPAddr,
                    &form->portNum,
                    &form->flags,
                    &form->reserved,
                    &form->dataLen);

            form->data = (ubyte *)malloc((form->dataLen)+1);
            Unpack(pktdata + sizeof(ubyte4) + sizeof(ubyte2) + sizeof(ubyte) +
                    sizeof(ubyte) + sizeof(ubyte4), "B* P",
                    form->dataLen, form->data,
                    &form->padding);
            form->data[form->dataLen] = '\0';
            break;
        default:
            break;
    }
}

/*>

  ubyte4 size = SendDataReqParamBuild(SendDataReqParamForm_t *form,
                        ubyte       *buffer);

  REQUIRES:

  DESCRIPTION:
  Packs the data in a SendDataReqParamForm data structure into network byte order
  to be passed to UDP.

  ARGS:
  form      pointer to packet template.
  buffer    pointer to memory in which to pack the data.

  RETURNS:
  size      number of bytes packed into packet.

  SIDE EFFECTS:

<*/
ubyte4 SendDataReqParamBuild(SendDataReqParamForm_t *form, ubyte *buffer, ubyte2 paddingLen)
{
    ubyte4 len = 0;

    switch (stubIPVersion) {
        case STUB_IPV6:
            len =  Pack(buffer,"B16 S B B L B*",
                    &form->anvlIPv6Addr,
                    &form->portNum,
                    &form->flags,
                    &form->reserved,
                    &form->dataLen,
                    form->dataLen, form->data);
            break;
        case STUB_IPV4:
            len = Pack(buffer,"L S B B L B*",
                    &form->anvlIPAddr,
                    &form->portNum,
                    &form->flags,
                    &form->reserved,
                    &form->dataLen,
                    form->dataLen, form->data);
            break;
        default:
            break;
    }

    if(paddingLen) {
        len += Pack(buffer + len,"B*", paddingLen, form->padding);
    }

    return len;
}

/*>

  UDPSendRptDataReqParamForm_t *form  = UDPSendRptDataReqParamFormCreate();

  REQUIRES:

  DESCRIPTION:
  This function creates a data structure representing the parameters field of UDP_CMD_SEND_REPEAT_DATA
  command message.

  ARGS:

  RETURNS:
  form  ptr to a new (cleared) data structure

  SIDE EFFECTS:

<*/

UDPSendRptDataReqParamForm_t *UDPSendRptDataReqParamFormCreate()
{
  UDPSendRptDataReqParamForm_t *form = 0;

  form = (UDPSendRptDataReqParamForm_t *)Malloc(sizeof(UDPSendRptDataReqParamForm_t));
  CLEAR_DATA(&form->reserved);
  CLEAR_DATA(form);
  return form;
}

/*>

  UDPSendRptDataReqParamFormDestroy(UDPSendRptDataReqParamForm_t *form)

  REQUIRES:

  DESCRIPTION:
  Frees the data structure that contains all the subfields in the parameters
  field of a UDP_CMD_SEND_REPEAT_DATA command message.

  ARGS:
  form   pointer to a data structure representing parameters subfield of a UDP_CMD_SEND_REPEAT_DATA command message

  RETURNS:
  void

  SIDE EFFECTS:

<*/

void
UDPSendRptDataReqParamFormDestroy(UDPSendRptDataReqParamForm_t *form)
{
  if(!form) {
    FPrintf(stderr, "! Invalid UDPSendRptDataReqParam form\n");
    Exit(1);
  }
  Free(form);
  form = 0;
}

/*>>

  void UDPSendRptDataReqParamToForm(ubyte *pktdata,
                       UDPSendRptDataReqParamForm_t *form);

  REQUIRES:

  DESCRIPTION:
  Unpacks bytes out of a packet into a form, usually for printing or
  checking.

  ARGS:
  pktdata   buffer containing STUB parmeters.
  form  pointer to form to stuff values into.

  RETURNS:

  SIDE EFFECTS:

<<*/

void
UDPSendRptDataReqParamToForm(ubyte *pktdata, UDPSendRptDataReqParamForm_t *form)
{
    switch (stubIPVersion) {
        case STUB_IPV6:
            Unpack(pktdata,
                    "B16 S S L P +1 B*",
                    &form->anvlIPv6Addr,
                    &form->dstPort,
                    &form->srcPort,
                    &form->dataLen,
                    &form->data,
                    3, &form->reserved);
            break;
        case STUB_IPV4:
            Unpack(pktdata,
                    "L S S L P +1 B*",
                    &form->anvlIPAddr,
                    &form->dstPort,
                    &form->srcPort,
                    &form->dataLen,
                    &form->data,
                    3, &form->reserved);
            break;
        default:
            break;
    }
}

/*>

  ubyte4 size = UDPSendRptDataReqParamBuild(UDPSendRptDataReqParamForm_t *form,
                        ubyte       *buffer);

  REQUIRES:

  DESCRIPTION:
  Packs the data in a UDPSendRptDataReqParamForm data structure into network byte order
  to be passed to UDP.

  ARGS:
  form      pointer to packet template.
  buffer    pointer to memory in which to pack the data.

  RETURNS:
  size      number of bytes packed into packet.

  SIDE EFFECTS:

<*/

ubyte4 UDPSendRptDataReqParamBuild(UDPSendRptDataReqParamForm_t *form, ubyte *buffer)
{
    switch (stubIPVersion) {
        case STUB_IPV6:
            return Pack(buffer,"B16 S S L B* B*",
                    &form->anvlIPv6Addr,
                    &form->dstPort,
                    &form->srcPort,
                    &form->dataLen,
                    1, form->data,
                    3, &form->reserved);
            break;
        case STUB_IPV4:
            return Pack(buffer,"L S S L B* B*",
                    &form->anvlIPAddr,
                    &form->dstPort,
                    &form->srcPort,
                    &form->dataLen,
                    1, form->data,
                    3, &form->reserved);
            break;
        default:
            break;
    }

    return 0;
}

/*>

  UDPSendDataReqParamForm_t *form  = UDPSendDataReqParamFormCreate();

  REQUIRES:

  DESCRIPTION:
  This function creates a data structure representing the parameters field of UDP_CMD_SEND
  command message.

  ARGS:

  RETURNS:
  form  ptr to a new (cleared) data structure

  SIDE EFFECTS:

<*/

UDPSendDataReqParamForm_t *UDPSendDataReqParamFormCreate()
{
  UDPSendDataReqParamForm_t *form = 0;

  form = (UDPSendDataReqParamForm_t *)Malloc(sizeof(UDPSendDataReqParamForm_t));
  CLEAR_DATA(form);
  return form;
}

/*>

  UDPSendDataReqParamFormDestroy(UDPSendDataReqParamForm_t *form)

  REQUIRES:

  DESCRIPTION:
  Frees the data structure that contains all the subfields in the parameters
  field of a UDP_CMD_SEND command message.

  ARGS:
  form   pointer to a data structure representing parameters subfield of a UDP_CMD_SEND command message

  RETURNS:
  void

  SIDE EFFECTS:

<*/

void
UDPSendDataReqParamFormDestroy(UDPSendDataReqParamForm_t *form)
{
  if(!form) {
    FPrintf(stderr, "! Invalid UDPSendDataReqParam form\n");
    Exit(1);
  }
  Free(form);
  form = 0;
}

/*>>

  void UDPSendDataReqParamToForm(ubyte *pktdata,
                       UDPSendDataReqParamForm_t *form);

  REQUIRES:

  DESCRIPTION:
  Unpacks bytes out of a packet into a form, usually for printing or
  checking.

  ARGS:
  pktdata   buffer containing STUB parmeters.
  form  pointer to form to stuff values into.

  RETURNS:

  SIDE EFFECTS:

<<*/

void
UDPSendDataReqParamToForm(ubyte *pktdata, UDPSendDataReqParamForm_t *form)
{
    switch (stubIPVersion) {
        case STUB_IPV6:
            Unpack(pktdata,
                    "B16 S S L B*",
                    &form->anvlIPv6Addr,
                    &form->dstPort,
                    &form->srcPort,
                    &form->dataLen,
                    form->dataLen, form->data);
            break;
        case STUB_IPV4:
            Unpack(pktdata,
                    "L S S L B*",
                    &form->anvlIPAddr,
                    &form->dstPort,
                    &form->srcPort,
                    &form->dataLen,
                    form->dataLen, form->data);
            break;
        default:
            break;
    }
}

/*>

  ubyte4 size = UDPSendDataReqParamBuild(UDPSendDataReqParamForm_t *form,
                        ubyte       *buffer);

  REQUIRES:

  DESCRIPTION:
  Packs the data in a UDPSendDataReqParamForm data structure into network byte order
  to be passed to UDP.

  ARGS:
  form      pointer to packet template.
  buffer    pointer to memory in which to pack the data.

  RETURNS:
  size      number of bytes packed into packet.

  SIDE EFFECTS:

<*/
ubyte4 UDPSendDataReqParamBuild(UDPSendDataReqParamForm_t *form, ubyte *buffer)
{
    ubyte4 len = 0;

    switch (stubIPVersion) {
        case STUB_IPV6:
            len =  Pack(buffer, "B16 S S L B*",
                    &form->anvlIPv6Addr,
                    &form->dstPort,
                    &form->srcPort,
                    &form->dataLen,
                    form->dataLen, form->data);
            break;
        case STUB_IPV4:
            len = Pack(buffer, "L S S L B*",
                    &form->anvlIPAddr,
                    &form->dstPort,
                    &form->srcPort,
                    &form->dataLen,
                    form->dataLen, form->data);
            break;
        default:
            break;
    }

    return len;
}


/*>

  SetTTLParamForm_t *form  = SetTTLParamFormCreate();

  REQUIRES:

  DESCRIPTION:
  This function creates a data structure representing the parameters field of SET_TTL
  command/response message.

  ARGS:

  RETURNS:
  form  ptr to a new (cleared) data structure

  SIDE EFFECTS:

<*/

SetTTLParamForm_t *SetTTLParamFormCreate()
{
  SetTTLParamForm_t *form = 0;

  form = (SetTTLParamForm_t *)Malloc(sizeof(SetTTLParamForm_t));
  CLEAR_DATA(&form->reserved);
  form->ttl = 0;
  return form;
}

/*>

  SetTTLParamFormDestroy(SetTTLParamForm_t *form)

  REQUIRES:

  DESCRIPTION:
  Frees the data structure that contains all the subfields in the parameters
  field of a SET_TTL command message.

  ARGS:
  form   pointer to a data structure representing parameters subfield of a SET_TTL command message

  RETURNS:
  void

  SIDE EFFECTS:

<*/

void
SetTTLParamFormDestroy(SetTTLParamForm_t *form)
{
  if(!form) {
    FPrintf(stderr, "! Invalid SetTTL form\n");
    Exit(1);
  }
  Free(form);
  form = 0;
}

/*>>

  void SetTTLParamToForm(ubyte *pktdata,
                       SetTTLParamForm_t *form);

  REQUIRES:

  DESCRIPTION:
  Unpacks bytes out of a packet into a form, usually for printing or
  checking.

  ARGS:
  pktdata   buffer containing STUB parmeters.
  form  pointer to form to stuff values into.

  RETURNS:

  SIDE EFFECTS:

<<*/

void
SetTTLParamToForm(ubyte *pktdata, SetTTLParamForm_t *form)
{
  Unpack(pktdata,
         "B B*",
         &form->ttl,
         3, form->reserved);
}

/*>

  ubyte4 size = SetTTLParamBuild(SetTTLParamForm_t *form,
                        ubyte       *buffer);

  REQUIRES:

  DESCRIPTION:
  Packs the data in a SetTTLParamForm data structure into network byte order
  to be passed to UDP.

  ARGS:
  form      pointer to packet template.
  buffer    pointer to memory in which to pack the data.

  RETURNS:
  size      number of bytes packed into packet.

  SIDE EFFECTS:

<*/

ubyte4 SetTTLParamBuild(SetTTLParamForm_t *form, ubyte *buffer)
{
  return Pack(buffer,"B B*",
                     &form->ttl,
                     3, form->reserved);
}

#ifdef LIB_IPV6
/*>

  SetHopLimitParamForm_t *form  = SetHopLimitParamFormCreate();

  REQUIRES:

  DESCRIPTION:
  This function creates a data structure representing the parameters field of SET_HOPLIMIT
  command/response message.

  ARGS:

  RETURNS:
  form  ptr to a new (cleared) data structure

  SIDE EFFECTS:

<*/

SetHopLimitParamForm_t *SetHopLimitParamFormCreate()
{
  SetHopLimitParamForm_t *form = 0;

  form = (SetHopLimitParamForm_t *)Malloc(sizeof(SetHopLimitParamForm_t));
  CLEAR_DATA(&form->reserved);
  form->hopLimit = 0;
  return form;
}

/*>

  SetHopLimitParamFormDestroy(SetHopLimitParamForm_t *form)

  REQUIRES:

  DESCRIPTION:
  Frees the data structure that contains all the subfields in the parameters
  field of a SET_HOPLIMIT command message.

  ARGS:
  form   pointer to a data structure representing parameters subfield of a SET_HOPLIMIT command message

  RETURNS:
  void

  SIDE EFFECTS:

<*/

void
SetHopLimitParamFormDestroy(SetHopLimitParamForm_t *form)
{
  if(!form) {
    FPrintf(stderr, "! Invalid SetHopLimit form\n");
    Exit(1);
  }
  Free(form);
  form = 0;
}

/*>>

  void SetHopLimitParamToForm(ubyte *pktdata,
                       SetHopLimitParamForm_t *form);

  REQUIRES:

  DESCRIPTION:
  Unpacks bytes out of a packet into a form, usually for printing or
  checking.

  ARGS:
  pktdata   buffer containing STUB parmeters.
  form  pointer to form to stuff values into.

  RETURNS:

  SIDE EFFECTS:

<<*/

void
SetHopLimitParamToForm(ubyte *pktdata, SetHopLimitParamForm_t *form)
{
  Unpack(pktdata,
         "B B*",
         &form->hopLimit,
         3, form->reserved);
}

/*>

  ubyte4 size = SetHopLimitParamBuild(SetHopLimitParamForm_t *form,
                        ubyte       *buffer);

  REQUIRES:

  DESCRIPTION:
  Packs the data in a SetHopLimitParamForm data structure into network byte order
  to be passed to UDP.

  ARGS:
  form      pointer to packet template.
  buffer    pointer to memory in which to pack the data.

  RETURNS:
  size      number of bytes packed into packet.

  SIDE EFFECTS:

<*/

ubyte4 SetHopLimitParamBuild(SetHopLimitParamForm_t *form, ubyte *buffer)
{
  return Pack(buffer,"B B*",
                     &form->hopLimit,
                     3, form->reserved);
}
#endif
#if 0
/*>

  (ubyte4)len = STUBMsgConstruct(ubyte2 msgType,
                                      ubyte2 cmdID,
                                      ubyte *buffer,
                      TCPDUTCmdContext_t *dutSideContext);
  DESCRIPTION:
  This function constructs STUB message using command ID and message type.
  This message will be used as a payload in UDP packet exchanged between ANVL and DUT

  ARGS:
  msgType             Type of the STUB packet (Request/Acknowledgement/Response)
                   to be constructed
  cmdID            ID of a stub command
  buffer           pointer to memory in which to pack the data

  RETURNS:
  len              number of bytes packed into buffer

<*/


ubyte4
STUBMsgConstruct(STUBForm_t *form, ubyte *buffer, TCPDUTCmdContext_t *dutSideContext)
{
  ubyte *paramPointer = 0;
  ubyte2 major, minor = 0;
  ubyte4 suiteNameLen = 0 , testNumLen = 0, notifyStrLen = 0, i = 0;
  ubyte *testNum = 0;
  ubyte2 *majorPtr = 0;
  ubyte *lastPtr = 0;
  ubyte protocol[MAX_STRING];
  ubyte4 stubPacketLen = 0; /* Length of entire ICMP Packet */
  ubyte param[TCP_STUB_PARAM_LEN];
  CLEAR_DATA(&protocol);
  CLEAR_DATA(&param);

  switch(form->cmdID) {
    case NOTIFY_TEST_START:
    case NOTIFY_TEST_END:
      form->params = param;
      notifyStrLen = StrLen(notifyStr);
      lastPtr = notifyStr + notifyStrLen-1;
      for(i=0; i < notifyStrLen; i++) {
        lastPtr--;
        if((*lastPtr) == '-') {
          break;
        }
      }
      lastPtr++;
      testNumLen = i+2;
      suiteNameLen = notifyStrLen-testNumLen;
      StrNCpy(protocol, notifyStr, suiteNameLen);
      major = StrToL(lastPtr,0,10);
      lastPtr += 2;
      minor = StrToL(lastPtr,0,10);
      Pack(form->params, "S S L B*", &major, &minor, &suiteNameLen, suiteNameLen, protocol);
      form->paramsLen = sizeof(ubyte2) + sizeof(ubyte2)+ sizeof(ubyte4) + suiteNameLen;
      form->paramsLen += STUBMsgPaddingAdd(form);
      form->paramsLenOK = TRUE;
    break;
    case GET_VERSION:
    break;
    case LISTEN:
      *(ubyte2 *)paramPointer = dutSideContext->dutPort;
      paramPointer += sizeof(ubyte2) + 1;
      *(ubyte2 *)paramPointer = 0;
      form->paramsOK = TRUE;
      form->paramsLen = sizeof(ubyte4);
      form->paramsLenOK = TRUE;
      break;
    case SET_STATE:
      *(ubyte4 *)paramPointer = dutSideContext->state;
      form->paramsOK = TRUE;
      form->paramsLen = sizeof(ubyte4);
      form->paramsLenOK = TRUE;
      break;
    default:
      Error(FATAL,"! Illegal TCP test application command ID: %hu\n",
          form->cmdID);
    break;
  }

  /* Build the ICMP echo form */
  stubPacketLen = STUBBuild(form, buffer);

  return stubPacketLen;
}

/*>>

  static ubyte STUBMsgPaddingAdd(STUBForm_t *form);

  DESCRIPTION:
  This function adds padding (filled with zeroes) to the msg so
  that the length of the data is big enough to prevent the message
  from getting stuck in buffers for being "too small". Padding is added
  to make the STUB message size equal to multiple of 4.

  ARGS:
  msg      pointer to string buffer containing message to be padded

  RETURNS:
  paddingLen    number of bytes of padding added to end of msg

<<*/
ubyte
STUBMsgPaddingAdd(STUBForm_t *form)
{
  ubyte paddingLen = 0, *paddingPtr = 0, i = 0;
  ubyte4 msgLen = 0;

  msgLen = sizeof(ubyte2) + sizeof(ubyte2) + sizeof(ubyte2) + form->paramsLen;
  if (msgLen % 4) {
    /* calculate number of bytes of padding to add */
    paddingLen = (((msgLen-1)|3)+1)-msgLen;
    paddingPtr = form->params + msgLen;

    /* set length of message to minimum data length */
    msgLen += paddingLen;
    for (i = 0; i < paddingLen; i++) {
      *(paddingPtr++) = 0;
    }
    /* null-terminate new string */
    *paddingPtr = '\0';
  }
  else {
    /* data is already the correct size, so no need to pad */
  }
  return paddingLen;
}
#endif

byte*
STUBTypeToString(ubyte type, ubyte2 value, byte *string)
{
  if (type) {
    switch (value) {
      case COMMAND:
        SPrintf(string, "%s", "Command");
        break;
      case ACK:
        SPrintf(string, "%s", "Acknowledgement");
        break;
      case RESPONSE:
        SPrintf(string, "%s", "Response");
        break;
      default:
        SPrintf(string, "%s", "*INVALID* message");
        break;
    }
  }
  else {
    switch (value) {
      case TCP_CMD_GET_VERSION:
        SPrintf(string, "%s", "TCP_GET_VERSION");
        break;
      case TCP_CMD_CONNECT:
        SPrintf(string, "%s", "TCP_CONNECT");
        break;
      case TCP_CMD_SEND:
        SPrintf(string, "%s", "TCP_SEND");
        break;
      case TCP_CMD_SEND_REPEAT_DATA:
        SPrintf(string, "%s", "TCP_SEND_REPEAT_DATA");
        break;
      case TCP_CMD_RECV:
        SPrintf(string, "%s", "TCP_RECV");
        break;
      case TCP_CMD_LISTEN:
        SPrintf(string, "%s", "TCP_LISTEN");
        break;
      case TCP_CMD_CLOSE:
        SPrintf(string, "%s", "TCP_CLOSE");
        break;
      case TCP_CMD_ABORT:
        SPrintf(string, "%s", "TCP_ABORT");
        break;
      case TCP_CMD_SOCK_CREATE:
        SPrintf(string, "%s", "TCP_SOCK_CREATE");
        break;
      case TCP_CMD_SET_STATE:
        SPrintf(string, "%s", "TCP_SET_STATE");
        break;
      case TCP_CMD_GET_SAMPLE_DATA:
        SPrintf(string, "%s", "TCP_GET_SAMPLE_DATA");
        break;
      case TCP_CMD_GET_DATA:
        SPrintf(string, "%s", "TCP_GET_DATA");
        break;
      case TCP_CMD_GET_CODE:
        SPrintf(string, "%s", "TCP_GET_CODE");
        break;
      case TCP_CMD_GET_RETCODE:
        SPrintf(string, "%s", "TCP_GET_RETCODE");
        break;
      case TCP_CMD_ASYNC_RECV:
        SPrintf(string, "%s", "TCP_ASYNC_RECV");
        break;
      case TCP_CMD_NO_ASYNC_RECV:
        SPrintf(string, "%s", "TCP_NO_ASYNC_RECV");
        break;
      case TCP_CMD_DUT_REBOOT:
        SPrintf(string, "%s", "TCP_DUT_REBOOT");
        break;
      case TCP_CMD_GET_PERROR:
        SPrintf(string, "%s", "TCP_GET_PERROR");
        break;
      case TCP_CMD_RECV_LIMSZ:
        SPrintf(string, "%s", "TCP_RECV_LIMSZ");
        break;
      case TCP_CMD_RECV_ONCE:
        SPrintf(string, "%s", "TCP_RECV_ONCE");
        break;
      case TCP_CMD_SET_DF_BIT:
        SPrintf(string, "%s", "TCP_SET_DF_BIT");
        break;
      case TCP_CMD_SET_MSS:
        SPrintf(string, "%s", "TCP_SET_MSS");
        break;
      case TCP_CMD_SET_HOPLIMIT:
        SPrintf(string, "%s", "TCP_SET_HOPLIMIT");
        break;
      case TCP_CMD_SET_RCVBUF_LEN:
        SPrintf(string, "%s", "TCP_SET_RCVBUF_LEN");
        break;
      case TCP_CMD_NOTIFY_TEST_START:
        SPrintf(string, "%s", "TCP_NOTIFY_TEST_START");
        break;
      case TCP_CMD_NOTIFY_TEST_END:
        SPrintf(string, "%s", "TCP_NOTIFY_TEST_END");
        break;
      case TCP_CMD_API_ACCEPT:
        SPrintf(string, "%s", "TCP_API_ACCEPT");
        break;
      case TCP_CMD_OPT_STDURG:
        SPrintf(string, "%s", "TCP_OPT_STDURG");
        break;
      case TCP_CMD_OPT_URGINLINE:
        SPrintf(string, "%s", "TCP_OPT_URGINLINE");
        break;
      case TCP_CMD_SHUTDOWN_READ:
        SPrintf(string, "%s", "TCP_SHUTDOWN_READ");
        break;
      case TCP_CMD_SHUTDOWN_WRITE:
        SPrintf(string, "%s", "TCP_SHUTDOWN_WRITE");
        break;
      case TCP_CMD_SHUTDOWN_RD_WT:
        SPrintf(string, "%s", "TCP_SHUTDOWN_RD_WT");
        break;
      case UDP_CMD_SEND:
        SPrintf(string, "%s", "UDP_SEND");
        break;
      case UDP_CMD_SEND_REPEAT_DATA:
        SPrintf(string, "%s", "UDP_SEND_REPEAT_DATA");
        break;
      default:
        SPrintf(string, "%s", "*INVALID* command");
        break;
    }
  }

  return string;
}

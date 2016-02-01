//**************************************************************************
//
//    Copyright 2003 Broadcom Corporation
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
//    Filename: SLog.h
//    Author:   T.Clack
//    Creation Date: 08-August-2003
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Print and logging functions for SNMP
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef SLOG_H
#define SLOG_H




// values to be used for level and or format
#define SALW      0  // Always 
#define SERR      2  // ERROR
#define SWRN      4  // WARNING
#define SINF      6  // INFO
#define SDBG      8  // DEBUG

// can be used for format
#define SNONE     0  //no formatting


void SFormat(int format);

/*********************************************************************************
// FUNCTION:    SLog      
//
// Description: Prints a printf formatted style string.
// 
// Parameters:  level - tracelevel setting associated with this call. 
//                        e.g., SALW, SERR, SWRN, SDBG, ...
//                        
//              format - the format setting for this output. e.g., 0, SERR, SWRN, ...
//              fmt, ... - printf-style variable argument string
//                                                    
// Returns:  None  
**********************************************************************************/
void SLog( int level, int format, char *fmt, ... );

/*********************************************************************************
// FUNCTION:    SLogBuf      
//
// Description: Dumps a non-null terminated buffer of specified length.
// 
// Parameters:  level - tracelevel setting associated with this call. 
//                        e.g., SALW, SERR, SWRN, SDBG, ...
//                        
//              format - the format setting for this output. e.g., 0, SERR, SWRN, ...
//              text   - any text message that will be dumped
//              ucBuf  - the buffer to be dumped
//              BufLen - the length of ucBuf
//              bufFormat - how the buffer should be dumped.
//                          c = dump as chars
//                          d = dump as decimal numbers
//                          x = dump as hex numbers
//
// Returns:  None  
**********************************************************************************/
void SLogBuf(int level, int format, char * text, unsigned char * ucBuf, int BufLen, char bufFormat);


/*********************************************************************************
// FUNCTION:    SSimpleLog      
//
// Description: Prints a printf formatted style string.  Simple approach logs
//              if trace level is non-zero, no special formating.
// 
// Parameters:  fmt, ... - printf-style variable argument string
//                                                    
// Returns:  None  
**********************************************************************************/
void SSimpleLog( char *fmt, ... );

#endif

    

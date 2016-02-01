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
//    Filename: SLog.c
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


#include "SLog.h"
#include <stdio.h>
#include <stdarg.h>

extern int trace_level;

/*********************************************************************************
// FUNCTION:    SLog      
//
// Description: Prints a printf formatted style string.
// 
// Parameters:  level - trace_level setting associated with this call. 
//                        e.g., SALW, SERR, SWRN, SDBG, ...
//                        
//              format - the format setting for this output. e.g., 0, SERR, SWRN, ...
//              fmt, ... - printf-style variable argument string
//                                                    
// Returns:  None  
**********************************************************************************/
void SLog( int level, int format, char *fmt, ... )
{
    va_list args;

    if ( level <= trace_level )    //check for permission
    {
        if ( format > 0 )          //put some formatting in
            SFormat(format);

        va_start( args, fmt );
        vfprintf( stdout, fmt, args );
        va_end( args );

        fflush( stdout );
    }
}

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
void SSimpleLog( char *fmt, ... )
{
    if ( trace_level > 0 )    //check for permission
    {
        va_list args;
      
        va_start( args, fmt );
        vfprintf( stdout, fmt, args );
        va_end( args );
      
        fflush( stdout );
    }
}

/*********************************************************************************
// FUNCTION:    SLogBuf      
//
// Description: Dumps a non-null terminated buffer of specified length.
// 
// Parameters:  level - trace_level setting associated with this call. 
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
void SLogBuf(int level, int format, char * text, unsigned char * ucBuf, int BufLen, char bufFormat)
{
    int i;
    
    if ( level <= trace_level )     //check for permission
    {
        if ( format >= 0 )         //put some formatting in
            SFormat(format);

        printf("%s (%d) <",text, BufLen);

        switch ( bufFormat )
        {
        case 'c':
            for ( i=0;i<BufLen;i++ )
                printf("%c", ucBuf[i]);
            break;

        case 'x':
            for ( i=0;i<BufLen;i++ )
                printf("%.2x ", ucBuf[i]);
            break;

        case 'd':
            for ( i=0;i<BufLen;i++ )
                printf("%d ", ucBuf[i]);
            break;
        }
        printf(">\n");
    }
}

/*********************************************************************************
// FUNCTION:    SFormat    
//
// Description: Prints formatting text based on input.
// 
// Parameters:  format - 
//                                                    
// Returns:  None  
**********************************************************************************/
void SFormat(int format)
{
    switch ( format )
    {
    case SALW:
        break;
    case SERR:
        printf("ERR:- ");
        break;
    case SWRN:
        printf("WRN:- ");
        break;
    case SINF:
        printf("INF:- ");
        break;
    case SDBG:
        printf("DBG:- ");
        break;
    }
}

    

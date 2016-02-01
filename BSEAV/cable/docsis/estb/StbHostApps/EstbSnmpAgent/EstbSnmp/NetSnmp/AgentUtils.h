//**************************************************************************
//
//    Copyright 2003  Broadcom Corporation
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
//    Filename: AgentUtils.h
//    Author:   Thomas Clack
//    Creation Date: 21-April-2003
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//      Definitions and prototypes for nvhard.c.
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef AGENTUTILS_H
#define AGENTUTILS_H


#if defined (__cplusplus)
extern "C"
{
#endif

extern int trace_level;   
////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   DumpBuf
//                      
// PURPOSE:    A very useful dumper.          
//
// PARAMETERS: text:      A string of text chars to print.
//             ucBuf:     Ptr to the buffer to be printed.
//             BufLen:    Length of ucBuf, or number of ucBuf chars to be dumped.
//             format:    How to dump the buf ( chars, dec, hex)
//             
////////////////////////////////////////////////////////////////////////////////
    void DumpBuf(char * text, unsigned char * ucBuf, int BufLen, char format);

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:   DumpBufTrace
//                      
// PURPOSE:    Checks trace_level and then calls DumpBuf.
//             
// PARAMETERS: text:      A string of text chars to print.
//             ucBuf:     Ptr to the buffer to be printed.
//             BufLen:    Length of ucBuf, or number of ucBuf chars to be dumped.
//             format:    How to dump the buf ( chars, dec, hex)
//             trace:     The trace_level.
////////////////////////////////////////////////////////////////////////////////
    void DumpBufTrace(char * text, unsigned char * ucBuf, int BufLen, char format, int trace);



#if defined (__cplusplus)

}
#endif

#endif



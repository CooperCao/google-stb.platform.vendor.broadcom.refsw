//****************************************************************************
//
// Copyright (c) 2007-2010 Broadcom Corporation
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
//**************************************************************************
//    Filename: AgentUtils.c
//    Author:   Thomas Clack
//    Creation Date: 21-April-03
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Utility functions.
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#include "AgentUtils.h"
#include <stdio.h>

//#include "NetsnmpAgnt.h"

//#include "BcmV2glue.h"
#include <SnmpStatus.h>
#include <SnmpApi.h>

//#include <stdio.h>
#include <string.h>
//#include <net-snmp/agent/agent_registry.h>
//#include <net-snmp/library/vacm.h>



int trace_level;

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
void DumpBuf(char * text, unsigned char * ucBuf, int BufLen, char format)
{
    int i;
    printf("DumpBuf(%d): %s - <",BufLen,text); 

    switch ( format )
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
void DumpBufTrace(char * text, unsigned char * ucBuf, int BufLen, char format, int trace)
{
    if ( trace < trace_level )
    {
        printf("Trace "); 
        DumpBuf(text, ucBuf, BufLen, format);
    }
}


// Only include this for eCos, if we aren't using the Broadcom Heap Manager.
#if defined(TARGETOS_eCos) && !(BFC_INCLUDE_HEAP_MANAGER_SUPPORT)
void eCosDumpMemStats(char * str)
{
  struct mallinfo meminfo = mallinfo();
    printf("------eCos Memory ---(%s)\n",str);
    printf("<%d>\t  total size of memory arena \n", meminfo.arena);
    //printf("<%d>\t  number of ordinary memory blocks \n", meminfo. ordblks);
    //printf("<%d>\t  number of small memory blocks\n",meminfo.smblks);
    //printf("<%d>\t  number of mmapped regions \n", meminfo.hblks);
    //printf("<%d>\t  total space in mmapped regions \n",  meminfo.hblkhd);
    //printf("<%d>\t  space used by small memory blocks \n",meminfo.usmblks);
    //printf("<%d>\t  space available for small memory blocks \n", meminfo.fsmblks);
    printf("<%d>\t  space used by ordinary memory blocks \n", meminfo.uordblks);
    printf("<%d>\t  space free for ordinary blocks \n", meminfo.fordblks);
    //printf("<%d>\t  top-most, releasable (via malloc_trim) space \n", meminfo.keepcost);
    printf("<%d>\t  size of largest free block \n", meminfo.maxfree);
    printf("-----------------------------------------------\n");
}
#endif





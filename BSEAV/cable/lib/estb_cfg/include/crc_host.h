/**************************************************************************
    Broadcom MCNS Compliant Modem
    Subscriber Modem MAC ASIC
**************************************************************************

    Copyright 1997  Broadcom Corporation
    All Rights Reserved
    No portions of this material may be reproduced in any form without the
    written permission of:
             Broadcom Corporation
             16251 Laguna Canyon Road
             Irvine, California  92618
    All information contained in this document is Broadcom Corporation
    company private, proprietary, and trade secret.



**************************************************************************
    Filename:       global.h
    Author:         k. carmichael, d. gay
    Creation Date:  03 mar 1997

**************************************************************************
    Description:
                    Global definitions

**************************************************************************
    Revision History:

**************************************************************************/

#ifndef crc_h
#define crc_h

#define PROCESSOR_TYPE MIPS

#if __cplusplus
extern "C" {
#endif

typedef enum {
   CRC_CCITT = 0,
   CRC_32
} PolyType;

/* This is a very fast, table-based implementation of CRC_32.  You should
   call this instead of compute_crc for CRC_32.  It returns the CRC, rather
   than requiring you to pass tons of arguments. */
unsigned long FastCrc32_host(const void *pData, unsigned int numberOfBytes);

/*
 * FastCrc32PiecewiseBuffer can do CRC based on piecewise buffers. So a small buffer
 * can be allocated to do CRC for a big file. The data is feeded continuously into
 * FastCrc32PiecewiseBuffer and the CRC of the previous data is passed in as prev_crc
 * parameter, the flag indicates whether the buffer is the first, last block, or both.
 * This function should generate the same result as FastCrc32().
 */
#define FIRST_BLOCK 1
#define LAST_BLOCK 2
unsigned long FastCrc32PiecewiseBuffer(const void *pData, unsigned int numberOfBytes,
                                       unsigned long prev_crc, int flag);


#if __cplusplus
}
#endif

#endif


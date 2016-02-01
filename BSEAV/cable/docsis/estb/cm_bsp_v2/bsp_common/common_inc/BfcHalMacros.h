//**************************************************************************
//
//    Copyright 1999  Broadcom Corporation
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
//    Filename: BfcalMacros.h
//    Author:   Dannie Gay
//    Creation Date: 11-jul-00
//
//**************************************************************************
//    Description:
//
//    BFC HAL portable Macro defintions
//    (modify macros in this file to Handle specific HW endianess issues)
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef BfcHalMacros_h
#define BfcHalMacros_h

#if defined __cplusplus
extern "C" {
#endif

// Register reads.
#ifndef REG8
   #define REG8(addr)                  (*(volatile UCHAR  * const)(addr))
#endif
#ifndef REG16
   #define REG16(addr)                 ntohs((*(volatile USHORT * const)(addr)))
#endif   
#ifndef REG32
   #define REG32(addr)                 ntohl((*(volatile ULONG  * const)(addr)))
#endif   

#define REG16_w(addr)                 (*(volatile USHORT * const)(addr))
#define REG32_w(addr)                 (*(volatile ULONG  * const)(addr))

// Register writes.

#define wREG8(addr, val)			   REG8(addr) 	= val
#define orREG8(addr, val)			   REG8(addr)  |= val
#define andREG8(addr, val)			   REG8(addr)  &= val
#define plusREG8(addr, val)			   REG8(addr)  += val
#define minusREG8(addr, val)		   REG8(addr)  -= val

#define wREG16(addr, val)   		   REG16_w(addr) = htons(val)
#define orREG16(addr, val)			   REG16_w(addr)  |= htons(val)
#define andREG16(addr, val)			   REG16_w(addr)  &= htons(val)
#define plusREG16(addr, val)		   REG16_w(addr)  += htons(val)
#define minusREG16(addr, val)		   REG16_w(addr)  -= htons(val)

#define wREG32(addr, val)			   REG32_w(addr) = htonl(val)
#define orREG32(addr, val)			   REG32_w(addr) |= htonl(val)
#define andREG32(addr, val)			   REG32_w(addr) &= htonl(val)
#define plusREG32(addr, val)		   REG32_w(addr) += htonl(val)
#define minusREG32(addr, val)		   REG32_w(addr) -= htonl(val)

// These were in tst33reg.c - moved here so all changes can
// be made in one place.

#define MACREG_8(x)     REG8 ( REGS_BASE_ADDR_REG + (x) )
#define MACREG_16(x)    REG16( REGS_BASE_ADDR_REG + (x) )
#define MACREG_32(x)    REG32( REGS_BASE_ADDR_REG + (x) )

#define wMACREG_8(addr, val)      REG8(REGS_BASE_ADDR_REG + (addr)) = val
#define wMACREG_16(addr, val)     REG16(REGS_BASE_ADDR_REG + (addr)) = val
#define wMACREG_32(addr, val)     REG32(REGS_BASE_ADDR_REG + (addr)) = val

// Bit reverse byte (inline) Macro
// (MUST use as a void method passing argument by reference) (i.e.  BCM_BIT_REVERSE(data); )
#define BCM_BIT_REVERSE(x)           										                      \
	{														                                           \
		uint8 bcmBitReverseMask = 0x01;			                                           \
		uint8 bcmBitReverseVal = 0x00; 																	 \
		int bcmBitReverseCounter;                                                         \
		if (x != 0)                                                                       \
		{                                                                                 \
			for (bcmBitReverseCounter=7; bcmBitReverseCounter>=0; bcmBitReverseCounter--)  \
			{                                                                              \
				if (bcmBitReverseMask & x)                                                  \
					bcmBitReverseVal |= (0x01 << bcmBitReverseCounter);                      \
				bcmBitReverseMask <<= 1;                                                    \
			}                                                                              \
			x = bcmBitReverseVal;                                                          \
		}                                                                                 \
	}                                                                                    

#if defined __cplusplus
}
#endif

#endif


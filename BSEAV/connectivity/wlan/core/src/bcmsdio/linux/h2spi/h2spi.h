/*
 * Broadcom PCI-SPI Host Controller Driver Definitions
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#ifndef _H2SPI_H
#define _H2SPI_H

#define H2SPI_MAXLEN		4096

/* H2SPI ioctls */
#define	H2SPI_GBOARDREV		_IOW(0xc1, 0x1, unsigned long)
#define H2SPI_SCPOL		_IOW(0xc1, 0x2, unsigned long)
#define	H2SPI_GCPOL		_IOR(0xc1, 0x3, unsigned long)
#define H2SPI_SCPHA		_IOW(0xc1, 0x4, unsigned long)
#define	H2SPI_GCPHA		_IOR(0xc1, 0x5, unsigned long)
#define H2SPI_SSPOL		_IOW(0xc1, 0x6, unsigned long)
#define	H2SPI_GSPOL		_IOR(0xc1, 0x7, unsigned long)
#define	H2SPI_SCLOCK		_IOR(0xc1, 0x8, unsigned long)
#define	H2SPI_SCLOCKMAX		_IOR(0xc1, 0x9, unsigned long)
#define	H2SPI_GCLOCK		_IOW(0xc1, 0xa, unsigned long)
#define	H2SPI_SDEVSIG		_IOW(0xc1, 0xb, unsigned long)
#define	H2SPI_GDEVSIG		_IOW(0xc1, 0xc, unsigned long)
#define	H2SPI_SBSWAP		_IOW(0xc1, 0xd, unsigned long)
#define	H2SPI_GBSWAP		_IOW(0xc1, 0xe, unsigned long)
#define	H2SPI_SWORDLEN		_IOW(0xc1, 0xf, unsigned long)
#define	H2SPI_GWORDLEN		_IOW(0xc1, 0x10, unsigned long)
#define	H2SPI_GVOLTAGE		_IOR(0xc1, 0x11, unsigned long)
#define	H2SPI_GCURRENT		_IOR(0xc1, 0x12, unsigned long)
#define	H2SPI_SDISPMODE		_IOW(0xc1, 0x13, unsigned long)
#define	H2SPI_SDISPVAL		_IOW(0xc1, 0x14, unsigned long)
#define	H2SPI_SPWRCTL		_IOW(0xc1, 0x15, unsigned long)

#endif  /* _H2SPI_H */

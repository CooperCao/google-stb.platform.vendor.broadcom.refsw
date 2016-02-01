/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:  Definitions for UART block
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef _BCMUART_H
#define _BCMUART_H

#ifdef MIPS_SDE
    #include "bcmmips.h"
#endif
#include "bchp_common.h"
#include "bchp_uarta.h"
#include "bchp_uartb.h"

#if (BCHP_CHIP != 7325) && (BCHP_CHIP != 7468)
#include "bchp_uartc.h"
#endif

#if !defined _ASMLANGUAGE
#if __cplusplus
extern "C" {
#endif
#endif

/* UART register base addresses */
#if BCHP_CHIP == 7401
    #define UARTA_ADR_BASE   BCM_PHYS_TO_K1(BCHP_PHYSICAL_OFFSET+BCHP_UARTA_RCVSTAT)
    #define UARTB_ADR_BASE   BCM_PHYS_TO_K1(BCHP_PHYSICAL_OFFSET+BCHP_UARTB_RCVSTAT)
        #if BCHP_VER >= BCHP_VER_B0
            /* B0 UARTC has changed to one like 7400 */
            #define UARTC_ADR_BASE   BCM_PHYS_TO_K1(BCHP_PHYSICAL_OFFSET+BCHP_UARTC_RBR)
        #else
            #define UARTC_ADR_BASE   BCM_PHYS_TO_K1(BCHP_PHYSICAL_OFFSET+BCHP_UARTC_RCVSTAT)
        #endif
#else
    #ifdef MIPS_SDE
        #define UARTA_ADR_BASE   BCM_PHYS_TO_K1(BCHP_PHYSICAL_OFFSET+BCHP_UARTA_RBR)
        #define UARTB_ADR_BASE   BCM_PHYS_TO_K1(BCHP_PHYSICAL_OFFSET+BCHP_UARTB_RBR)
        #define UARTC_ADR_BASE   BCM_PHYS_TO_K1(BCHP_PHYSICAL_OFFSET+BCHP_UARTC_RBR)
    #else
        #define UARTA_ADR_BASE   BCHP_PHYSICAL_OFFSET+BCHP_UARTA_RBR
        #define UARTB_ADR_BASE   BCHP_PHYSICAL_OFFSET+BCHP_UARTB_RBR
        #define UARTC_ADR_BASE   BCHP_PHYSICAL_OFFSET+BCHP_UARTC_RBR
    #endif
#endif

/* Define console using UART_ADR_BASE here. Console could be UARTB or UARTC for 7401.
 * No other files need to be modified for SDE.
 */
#if defined(DIAGS_UART_A)
    #define UART_ADR_BASE UARTA_ADR_BASE
#elif defined(DIAGS_UART_B)
    #define UART_ADR_BASE UARTB_ADR_BASE
#elif defined(DIAGS_UART_C)
    #define UART_ADR_BASE UARTC_ADR_BASE
#endif

#if BCHP_CHIP==7401
    #define console_out		uartb_out 
#else
    #define console_out		_writeasm 
#endif

#if BCHP_CHIP == 7401
    /* used for UARTA and UARTB */
    #define UART_RXSTAT     	0x00
    #define UART_RXDATA     	0x04
    #define UART_CONTROL    	0x0c
    #define UART_BAUDHI     	0x10
    #define UART_BAUDLO     	0x14
    #define UART_TXSTAT     	0x18
    #define UART_TXDATA     	0x1c

    /* used for UARTC */
    #define UART_SDW_RBR     	0x00
    #define UART_SDW_THR     	0x00
    #define UART_SDW_DLL     	0x00
    #define UART_SDW_DLH     	0x04
    #define UART_SDW_IER     	0x04
    #define UART_SDW_IIR     	0x08
    #define UART_SDW_FCR     	0x08
    #define UART_SDW_LCR     	0x0c
    #define UART_SDW_MCR     	0x10
    #define UART_SDW_LSR     	0x14
    #define UART_SDW_MSR     	0x18
    #define UART_SDW_SCR     	0x1c
#else
    /* UART registers */
    #define UART_SDW_RBR     	0x00
    #define UART_SDW_THR     	0x00
    #define UART_SDW_DLL     	0x00
    #define UART_SDW_DLH     	0x04
    #define UART_SDW_IER     	0x04
    #define UART_SDW_IIR     	0x08
    #define UART_SDW_FCR     	0x08
    #define UART_SDW_LCR     	0x0c
    #define UART_SDW_MCR     	0x10
    #define UART_SDW_LSR     	0x14
    #define UART_SDW_MSR     	0x18
    #define UART_SDW_SCR     	0x1c
#endif

/* LCR bit definitions */

#define DLAB				0x80
#define EPS				0x10
#define	PEN				0x08
#define STOP				0x04
#define DLS8				0x03
#define DLS_8BITS			0x03 /* some common code uses this definition */
#define DLS7				0x02
#define DLS6				0x01
#define DLS5				0x00

/* LSR bit definitions */
#define RFE 				0x80
#define TEMT 				0x40
#define THRE 				0x20
#define BI  				0x10
#define FE  				0x08
#define PE  				0x04
#define OE  				0x02
#define DR  				0x01

/* IER bit definitions */
#define PTIME				0x80
#define ETBEI				0x02
#define ERBFI				0x01

/* FCR bit definition */
#define	FIFOE				0x1

/* These boards don't have the old style uarts */
#if (BCHP_CHIP==7401)

/* RXSTAT bit definitions */
#define PARERR				0x20
#define FRAMEERR			0x10
#define OVERRUNERR			0x08
#define RXDATARDY			0x04
#define RXINTEN				0x02

/* CONTROL bit definitions */
#define BITM8				0x10
#define	PAREN				0x08
#define	TXEN				0x04
#define	RXEN				0x02
#define	PODD				0x01

/* TXSTAT bit definitions */
#define	TXINTEN				0x04
#define	TXIDLE				0x02
#define	TXDREGEMT			0x01

#endif

#if !defined _ASMLANGUAGE

/**********************************************************************
  Uart Register Structure
 **********************************************************************/
#if BCHP_CHIP==7401
typedef struct UartChannelNew {
  volatile unsigned long sdw_rbr_thr_dll;	/* 0x00 */
  volatile unsigned long sdw_dlh_ier;	/* 0x04 */
  volatile unsigned long sdw_iir_fcr;	/* 0x08 */
  volatile unsigned long sdw_lcr;	/* 0x0c */
  volatile unsigned long sdw_mcr;	/* 0x10 */
  volatile unsigned long sdw_lsr;	/* 0x14 */
  volatile unsigned long sdw_msr;	/* 0x18 */
  volatile unsigned long sdw_scr;	/* 0x1c */
} UartChannelNew; /* Used for UARTC */

typedef struct UartChannel {
  volatile unsigned long rxstat;
  volatile unsigned long rxdata;
  volatile unsigned long unused;
  volatile unsigned long control;
  volatile unsigned long baudh;
    /* When divide SysClk/2/(1+baudword) we should get 32*bit-rate */
  volatile unsigned long baudl;
  volatile unsigned long txstat;
  volatile unsigned long txdata;
} UartChannel;

#else
typedef struct UartChannel {
    volatile unsigned long sdw_rbr_thr_dll;	/* 0x00 */
    volatile unsigned long sdw_dlh_ier;	/* 0x04 */
    volatile unsigned long sdw_iir_fcr;	/* 0x08 */
    volatile unsigned long sdw_lcr;	/* 0x0c */
    volatile unsigned long sdw_mcr;	/* 0x10 */
    volatile unsigned long sdw_lsr;	/* 0x14 */
    volatile unsigned long sdw_msr;	/* 0x18 */
    volatile unsigned long sdw_scr;	/* 0x1c */
} UartChannel;

/* UartChannelNew is used by soap server.
 * define UartChannelNew the same as UartChannel
 */
typedef struct UartChannelNew {
  volatile unsigned long sdw_rbr_thr_dll;	/* 0x00 */
  volatile unsigned long sdw_dlh_ier;	/* 0x04 */
  volatile unsigned long sdw_iir_fcr;	/* 0x08 */
  volatile unsigned long sdw_lcr;	/* 0x0c */
  volatile unsigned long sdw_mcr;	/* 0x10 */
  volatile unsigned long sdw_lsr;	/* 0x14 */
  volatile unsigned long sdw_msr;	/* 0x18 */
  volatile unsigned long sdw_scr;	/* 0x1c */
} UartChannelNew; /* Used for UARTC */
#endif

#define UARTA ((volatile UartChannel *) UARTA_ADR_BASE)
#define UARTB ((volatile UartChannel *) UARTB_ADR_BASE)
#define UARTC ((volatile UartChannel *) UARTC_ADR_BASE)

#define UART  ((volatile UartChannel *)	UART_ADR_BASE)


#endif /* _ASMLANGUAGE */

#if 0 /* !(BCHP_CHIP==7400 || BCHP_CHIP==7405 || BCHP_CHIP==7325 || BCHP_CHIP == 7335 || BCHP_CHIP==7340 || BCHP_CHIP==7342) */
/******************************************************************
 * Baud Rate Table
 * XTALFREQ / baud rate / 16
 ******************************************************************/
#define BAUD_VAL(x)     (((XTALFREQ/8/(x) + 1)/2) - 1)
#define BAUD_VAL_HI(x)  ((BAUD_VAL(x) >> 8) & 0xff)
#define BAUD_VAL_LO(x)  (BAUD_VAL(x) & 0xff)

#define BAUD_1200_HI    BAUD_VAL_HI(1200)
#define BAUD_1200_LO    BAUD_VAL_LO(1200)
#define BAUD_2400_HI    BAUD_VAL_HI(2400)
#define BAUD_2400_LO    BAUD_VAL_LO(2400)
#define BAUD_4800_HI    BAUD_VAL_HI(4800)
#define BAUD_4800_LO    BAUD_VAL_LO(4800)
#define BAUD_9600_HI    BAUD_VAL_HI(9600)
#define BAUD_9600_LO    BAUD_VAL_LO(9600)
#define BAUD_19200_HI   BAUD_VAL_HI(19200)
#define BAUD_19200_LO   BAUD_VAL_LO(19200)
#define BAUD_38400_HI   BAUD_VAL_HI(38400)
#define BAUD_38400_LO   BAUD_VAL_LO(38400)
#define BAUD_57600_HI   BAUD_VAL_HI(57600)
#define BAUD_57600_LO   BAUD_VAL_LO(57600)
#define BAUD_115200_HI  BAUD_VAL_HI(115200)
#define BAUD_115200_LO  BAUD_VAL_LO(115200)
#endif

#if !defined _ASMLANGUAGE
#if __cplusplus
}
#endif
#endif

#endif  /* _BCMUART_H */

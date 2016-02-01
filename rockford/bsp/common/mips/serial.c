/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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
 * Module Description:  PMON Serial Driver functions
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"
#include "termio.h"
#include "bcmuart.h"


unsigned char _uart0rx(void);
extern void _writeasm(char);


/*******************************************************
 *
 * BAUDRATE : CPU_CLK_FREQ/(32 * BAUDRATE) 
 *	(54000000)/(32*2400)	
 *******************************************************/
static const unsigned long baudtable[11][2] =
{ /* baud-rate, clock select value */
    {300,   0x15F8},	/* -.007% Error	*/
    {600,   0xAFC},		/* .01% Error */
    {1200,  0x57D},		/* -.03% Error */
    {2400,  0x2BE},		/* -.03% Error */
    {4800,  0x15F},		/* .16% Error */
    {9600,  0xAF},		/* .16% Error */
    {19200, 0x57},		/* .16% Error */
    {38400, 0x2B},		/* -1.36% Error	*/
    {57600, 0x1C},		/* -1.35% Error	*/
    {115200, 0x0E},		/* -1.35% Error	*/
    {0,     0xAF}   /* table ends with default of 9600 baud */
};

static int _check_uart0rx(void);
/***********************************************************************/
/* bcmuart() */
/***********************************************************************/
int bcmuart(int op, char *dat, int chan, int data)
{
	char c='\0';

	BSTD_UNUSED(dat);
	switch (op) {
		/*
		// init is done in sbdreset()
		*/
		case OP_INIT:
			
			return 0;
		break;
		/*
		// set the baud rate 
		*/
		case OP_XBAUD:
		case OP_BAUD:
#if 0
			if (chan==0) {
				*(unsigned long*)UART0BAUD = baudtable[data,1];
			}
			else {
				*(unsigned long*)UART1BAUD = baudtable[data,1];
			}
#endif
		break;
		/*
		// check if transmitter ready 
		*/
		case OP_TXRDY:
			return 1;	/* always since it waits in TX */
		break;
		/*
		// send data
		*/
		case OP_TX:
			if (chan==0) {
				_writeasm(data);
				/* txData1Out(data); */
			}
			else {
		/*		_writeasm2(data); */
				/* txData2Out(data); */
			}
		break;
		/*
		// check for Char available
		*/
		case OP_RXRDY:
			if (chan==0) {
			    return _check_uart0rx();

				/* l =rxData1Avail(); */
			}
			else {
			/* 	l =rxData2Avail(); */
			}
			
		break;
		/*
		// read the uart for a character and return it
		*/
		case OP_RX:
			if (chan==0) {
			    c = _uart0rx();
			/*	c = rxData1In(); */
			}
			else {
			/*	c = rxData1In(); */
			}
			return c;
		break;
		/*
		//
		*/
		case OP_RXSTOP:
#if 0
			if (data)	/* disable Rx */
			else		/* enable Rx */
#endif
		break;	
	}

return 0;
}
/*************************************************************
 * 
 *************************************************************/
void printfc(char ch) {
	while (!(bcmuart (OP_TXRDY, 0, 0,0)))
		;

	bcmuart (OP_TX, 0, 0, ch);
}

char getUARTc(void) {
	if (bcmuart (OP_RXRDY, 0, 0,0))
		return bcmuart (OP_RX, 0, 0, 0);
	return 0;
}

void printfs(char *str)
{
#if 0
 while(*str) _writeasm(*str++);
#else
	for (; *str; str++) {
		printfc(*str);
	}
#endif
}

#if 1 /*  BCHP_CHIP==7400 || BCHP_CHIP==7405 || BCHP_CHIP==7325 || BCHP_CHIP==7335 || BCHP_CHIP==7340 || BCHP_CHIP==7342 */
static int
_check_uart0rx()
{
	unsigned char stat;

	while (1) {
    		stat = UART->sdw_lsr;
    		if (! (stat & (PE | FE | OE))) {
			break;
		} else {
			/* throw away the garbage char */
			/*      printf("jk\n"); */
			stat = UART->sdw_rbr_thr_dll;
		}
	}

	if (stat & DR) {
		return 1;
	} else {
		return 0;
	}
}
#elif BCHP_CHIP==7401 || BCHP_CHIP==7403
static int
_check_uart0rx()
{
	unsigned char stat;

	while (1) {
    		stat = UART->rxstat;
    		if (! (stat & (PARERR | FRAMEERR | OVERRUNERR))) {
			break;
		} else {
			/* throw away the garbage char */
			/*      printf("jk\n");	*/
			stat = UART->rxdata;
		}
	}

	if (stat & RXDATARDY) {
		return 1;
	} else {
		return 0;
	}
}
#endif

unsigned char
_uart0rx()
{
	unsigned char c='\0';
#if BCHP_CHIP==7401 || BCHP_CHIP==7403 
	unsigned char stat;
#endif

#if BCHP_CHIP==7400
	c = UART->sdw_rbr_thr_dll;
	stat = UART->sdw_lsr;
#elif BCHP_CHIP==7401 || BCHP_CHIP==7403 
	c = UART->rxdata;
	stat = UART->rxstat;
#endif

	return c;
}

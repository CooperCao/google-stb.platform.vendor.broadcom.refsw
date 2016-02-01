/***************************************************************************
*     Copyright (c) 2005-2007, Broadcom Corporation
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
* Module Description:
*   This file is meant for unit testing of RAP PI for 7401. This file 
*   contains the implementation of Functions and Data Structures to test 
*   different features of RAP PI.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

/* Linux stuff */
#include <stdio.h>          /* for printf */
#include <stdlib.h>
#include <string.h>

/* Support for SetTop API */
#include "bsettop_board.h"
#include "bsettop_os.h"

BDBG_MODULE(splashgen);

int splash_capture(BCHP_Handle hChp, BREG_Handle hReg, BMEM_Heap_Handle hMem, BINT_Handle hInt, BREG_I2C_Handle hRegI2c) ;

#ifndef SPLASH_NOHDM
#define splash_i2c_handle			b_board.i2c[B_I2C_CHANNEL_HDMI]
#else
#define splash_i2c_handle			NULL
#endif

int main()
{
	bresult rc;

	/* Initialise the board */    
	rc = bsettop_boot_board();
	BDBG_ASSERT(!rc);

	BDBG_ERR(("******* SplashGen main !!!"));

	splash_capture(
		b_board.B_CORE_CHIP.chip, 
		b_board.B_CORE_CHIP.reg, 
		b_board.B_CORE_CHIP.mem, 
		b_board.B_CORE_CHIP.irq,
		splash_i2c_handle ) ; /* I2C channel */
	
	bsettop_shutdown_board();

	return 0;
}

/* End of File */

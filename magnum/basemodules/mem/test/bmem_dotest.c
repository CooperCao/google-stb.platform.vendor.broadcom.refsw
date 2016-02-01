/***************************************************************************
 *     Copyright (c) 2003, Broadcom Corporation
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
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"
#include "bmem_test.h"

int main(int argc, char *argv[])
{
	BERR_Code  err = BERR_SUCCESS;

	/* system initialization */
	err = BERR_TRACE(BDBG_Init());
	if (err != BERR_SUCCESS) goto done;

	/* display configuration */
	BMEM_Test_DisplayConfig();

	/* run test 1 */
	err = BERR_TRACE(BMEM_Test_DoTest1());
	if (err != BERR_SUCCESS)
	{
		/* error */
		goto done;
	}

	/* system shutdown */
	BDBG_Uninit();

done:
	/* return status */
	return (err != BERR_SUCCESS) ? err : 0;
}

/* End of file */

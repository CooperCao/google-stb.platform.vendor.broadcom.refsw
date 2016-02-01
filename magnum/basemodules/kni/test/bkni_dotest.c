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
#include "bkni.h"

#include "bkni_test.h"


#define DO_TEST(test) do { rc = test ; if (rc!=BERR_SUCCESS) {BKNI_Printf("Test %s failed rc=%d\n", #test, rc); goto failure;}}while(0)

int main(void)
{
    BERR_Code rc;

    DO_TEST(BKNI_Init());
    DO_TEST(BDBG_Init());

    DO_TEST(BKNI_TestAll());

    BDBG_Uninit();
    BKNI_Uninit();
    BKNI_Printf("Kernel test passed\n");  

    return 0;

failure:
    return 1;
}



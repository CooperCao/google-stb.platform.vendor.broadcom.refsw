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
 * Module Description: Cross-platform kernel inteface conformance test
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BKNI_TEST_H__
#define BKNI_TEST_H__

BERR_Code BKNI_TestAll(void);

BERR_Code BKNI_TestInit(void);
BERR_Code BKNI_TestMemoryFunctions(void);
BERR_Code BKNI_TestMemoryFunctions2(void);
BERR_Code BKNI_TestPrintFunctions(void);
BERR_Code BKNI_TestMallocAndFree(void);
BERR_Code BKNI_TestSleepAndDelay(void);
BERR_Code BKNI_TestEvents(void);
BERR_Code BKNI_TestEventWithTask(void);
BERR_Code BKNI_TestMutexes(void);
BERR_Code BKNI_TestMutexWithTask(void);

#endif /* BKNI_TEST_H__ */

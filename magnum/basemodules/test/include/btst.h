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
 * Helper functions to write unit tests
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BTST_H__
#define BTST_H__

#include "bkni.h"
#include "bint.h"
#include "bchp.h"
#include "breg_mem.h"

/***************************************************************************
Summary:
	Evaluate the test case.

Description:
     Evaluate expression and if result is not true, print message and increment error counter. 
     Required error_count variable declared in the parent context

Input:
    x - expression to evaluate and test

Output:
    error_count - increment counter if test doesn't pass

Example:
    int test=1;

    BTST_TEST(test==1);

Returns:
    
    N/A
****************************************************************************/
#define BTST_TEST(expr)  do { \
       if (!(expr)) {BKNI_Printf("### %s failed at %s:%d\n", #expr, __FILE__, __LINE__); error_count++; } \
    } while (0)

/***************************************************************************
Summary:
	Evaluate the test case.

Description:
     Evaluate expression and if result is not true, terminate test .

Input:
    exp - expression to evaluate and test

Example:

    int *ptr=malloc(sizeof(*ptr));

    BTST_TEST_CRITICAL(ptr!=NULL);

    *ptr = 0;

Returns:
    
    N/A
****************************************************************************/
#define BTST_TEST_CRITICAL(expr)  do { \
       if (!(expr)) {BKNI_Printf("### %s failed at %s:%d\n", #expr, __FILE__, __LINE__); BKNI_Fail(); } \
    } while (0)


/***************************************************************************
Summary:
	Evaluate the test case.

Description:
    Call functions and if API fails print message including function name and error code, then
    return from the function.
    Required return from the calling function have return type BERR_Code;

Input:
    func - expression to evaluate and test

Example:

    BTST_TEST_API(BFOO_Bar(foo_handle, some, parameter));

Returns:
    
    N/A
****************************************************************************/
#define BTST_TEST_API(func) \
	do {BERR_Code result = (func); \
		if (result) {BKNI_Printf("### %s failed with %d\n", #func, result); return result;}\
	} while (0)



#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
	Context for the unit test.

Description:
    The platform specific start-up code passes in this structure context informat
    to the unit test.
****************************************************************************/
typedef struct {
    BCHP_Handle hChip;  /* chip handle */
    BREG_Handle hRegister; /* memory mapped register handle */
    BINT_Handle hIntr; /* interrupt interface handle */
} BTST_TestContext;

/***************************************************************************
Summary:
	Entry point to the unit test.

Description:
    This function called from the system specific code into the unit test. The system specific
    code initialize essential modules like KernelInterface, MemortyInterface, DebugInterface and
    passes intitalized handles into the function. 
   

Input:
    context - context for the unit test

Example:

    BERR_Code BTST_EntryPoint(const BTST_TestContext *context) 
    {
    BERR_Code rc;
    BCHP_Handle hChip = context->hChip;
    BREG_Handle hRegister = context->hRegister;

        rc = BTST_Open(&hTst, hChip, hRegister, NULL);
        if (rc!=BERR_SUCCESS) {
            return rc;
        }
        BTST_Close(hTst);

        return BERR_SUCCESS;        
    }

Returns:
    BERR_SUCCESS - the unit test passed
    other - test failed
    
****************************************************************************/
BERR_Code BTST_EntryPoint(const BTST_TestContext *context);

#ifdef __cplusplus
}
#endif

#endif /* BTST_H__ */




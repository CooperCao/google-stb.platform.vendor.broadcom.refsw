/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
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

#ifndef BDBG_APP_H
#define BDBG_APP_H

#ifdef __cplusplus
extern "C" {
#endif

#if BDBG_DEBUG_BUILD

/***************************************************************************
Summary:
    Set the instance debug level.

Description:
    Set debug level for the module instance.

Input:
    handle - the module instance
    level - new debug level

Returns:
    BERR_SUCCESS - instance has been registered
    BERR_INVALID_PARAMETER - unknown instance
****************************************************************************/
BERR_Code BDBG_SetInstanceLevel(BDBG_Instance handle, BDBG_Level level);

/***************************************************************************
Summary:
    Get the instance debug level.

Description:
    Get current debug level for the module instance.

Input:
    handle - the module instance

Output:
    level - current debug level

Returns:
    BERR_SUCCESS - instance has been registered
    BERR_INVALID_PARAMETER - unknown instance
****************************************************************************/
BERR_Code BDBG_GetInstanceLevel(BDBG_Instance handle, BDBG_Level *level);

/***************************************************************************
Summary:
    Set the instance name.

Description:
    Set name for the module instance.

Input:
    handle - the module instance
    new - name for the module instance

Returns:
    BERR_SUCCESS - instance has been registered
    BERR_INVALID_PARAMETER - unknown instance
****************************************************************************/
BERR_Code BDBG_SetInstanceName(BDBG_Instance handle, const char *name);

/***************************************************************************
Summary:
    Set the module debug level.

Description:
    Set debug level for the module.

Input:
    name - the module or module:instance name
    level - new debug level

Returns:
    BERR_SUCCESS - the module debug level has been set
    BERR_INVALID_PARAMETER - unknown module
****************************************************************************/
BERR_Code BDBG_SetModuleLevel(const char *name, BDBG_Level level);

/***************************************************************************
Summary:
    Get module debug level.

Description:
    Get current debug level for the module.

Input:
    name - the module or module:instance name
    level - new debug level

Returns:
    BERR_SUCCESS - the module debug level has been returned
    BERR_INVALID_PARAMETER - unknown module
****************************************************************************/
BERR_Code BDBG_GetModuleLevel(const char *name, BDBG_Level *level);

/***************************************************************************
Summary:
    Get the module's instance debug level.

Description:
    Get current debug level for the module's instance.

Input:
    name - the module name
    handle - the module instance

Output:
    level - current debug level

Returns:
    BERR_SUCCESS - instance has been registered
    BERR_INVALID_PARAMETER - unknown instance
****************************************************************************/
BERR_Code BDBG_GetModuleInstanceLevel(const char *name, BDBG_Instance handle, BDBG_Level *level);

/***************************************************************************
Summary:
    Set the module's instance debug level.

Description:
    Set debug level for the module's instance.

Input:
    name - the module name
    handle - the instance
    level - new debug level

Returns:
    BERR_SUCCESS - the module debug level has been set
    BERR_INVALID_PARAMETER - unknown module
****************************************************************************/
BERR_Code BDBG_SetModuleInstanceLevel(const char *name, BDBG_Instance handle, BDBG_Level level);

/***************************************************************************
Summary:
    Set global debug level.

Description:
    Set the global debug level.

Input:
    level - new debug level

Returns:
    BERR_SUCCESS - global debug level has been set
****************************************************************************/
BERR_Code BDBG_SetLevel(BDBG_Level level);

/***************************************************************************
Summary:
    Get the global debug level.

Description:
    Return the global debug level.

Input:
    <none>

Output:
    level - current global debug level

Returns:
    BERR_SUCCESS - current debug level has been retrieved
****************************************************************************/
BERR_Code BDBG_GetLevel(BDBG_Level *level);

/***************************************************************************
Summary:
    Enumerates debug modules

Description:
    This function is used to enumerate all modules that are
	to the debug subsystem. Module becomes known to the debug subsystem
	after it was used any of following primitives:
	BDBG_ENTER, BDBG_LEAVE, BDBG_MSG,  BDBG_WRN, BDBG_ERR
	or debug modules was accessed using by BDBG_SetModuleLevel
	BDBG_GetModuleLevel functions.

Input:
    callback - callback function that is called for each module and instance
	cntx - user specified context that would be passed into the callback function

Output:
    None

Returns:
    None
****************************************************************************/
void BDBG_EnumerateAll(void (*callback)(void *cntx, const char *module, BDBG_Instance instance, const char *inst_name), void *cntx);


/***************************************************************************
Summary:
    Set function to receive output the module;

Description:
    Set function that would receive all debug output generated by the module. Setting this 
    function would not affect debug output that directed to usual output channel.
    If module_print

Input:
    name - the module or module:instance name
    module_print - function that would receive all debug output generated by the module

Returns:
    BERR_SUCCESS - the module debug level has been set
    BERR_INVALID_PARAMETER - unknown module
****************************************************************************/
BERR_Code BDBG_SetModulePrintFunction(const char *name, BDBG_DebugModule_Print module_print);

#else /* BDBG_DEBUG_BUILD */

#define BDBG_SetLevel(level)  (BERR_SUCCESS)
#define BDBG_GetLevel(pLevel) (*(pLevel) = BDBG_eMsg, BERR_SUCCESS)
#define BDBG_SetModuleLevel(module, level)  (BERR_SUCCESS)
#define BDBG_EnumerateAll(callback, cntx) BDBG_NOP()
#define BDBG_GetModuleLevel(module, pLevel) (*(pLevel) = BDBG_eMsg, BERR_SUCCESS)
#define BDBG_SetModuleInstanceLevel(module, handle, level) (BERR_SUCCESS)
#define BDBG_GetModuleInstanceLevel(module, handle, pLevel) (*(pLevel) = BDBG_eMsg, BERR_SUCCESS)
#define BDBG_SetInstanceLevel(handle, level) (BERR_SUCCESS)
#define BDBG_GetInstanceLevel(handle, pLevel) (*(pLevel) = BDBG_eMsg, BERR_SUCCESS)
#define BDBG_SetInstanceName(handle, name) (BERR_SUCCESS)
#endif /* BDBG_DEBUG_BUILD */

#ifdef __cplusplus
}
#endif

#endif /* BDBG_APP_H */


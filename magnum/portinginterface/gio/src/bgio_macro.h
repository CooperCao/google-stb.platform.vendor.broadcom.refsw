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
 *
 * Revision History:
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BGIO_MACRO_H__
#define BGIO_MACRO_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * {private}
 *
 */

/* This macro take the check for a validity of a handle, and
 * cast to context pointer.
 */
#define BGIO_GENERIC_GET_CONTEXT(handle, context, structname) \
{ \
	if(!(handle) || \
	   (((structname*)(handle))->ulBlackMagic != \
	    (sizeof(structname) & 0xbac98800))) \
	{ \
		BDBG_ERR(("NULL context handle\n")); \
		(context) = NULL; \
	} \
	else \
	{ \
		(context) = (handle); \
	} \
}

/* This macro set the black magic for later handle validation
 */	
#define BGIO_GENERIC_SET_BLACK_MAGIC(handle, structname) \
{ \
	((structname*)(handle))->ulBlackMagic = sizeof(structname) & 0xbac98800; \
} 

/* This macro shred the garbage for BKNI managed buffer
 */
#if BDBG_DEBUG_BUILD 
#define BGIO_GENERIC_DESTROY_CONTEXT(struct_ptr, structname) \
{ \
	BKNI_Memset((void*)struct_ptr, 0xA3, sizeof(structname)); \
	BKNI_Free((void*)struct_ptr); \
	/*(void*)struct_ptr = NULL;*/ \
}
#else
#define BGIO_GENERIC_DESTROY_CONTEXT(struct_ptr, structname) \
{ \
	BKNI_Free((void*)struct_ptr); \
}
#endif	

#define BGIO_RETURN_IF_ERR(result) \
	if ( BERR_SUCCESS != (result)) \
	{\
		return BERR_TRACE(result);  \
	}

#define BGIO_END_IF_ERR(success, result, label) \
	if ( false == (success) ) \
	{\
		eResult = BERR_TRACE(result);  \
		goto (label);  \
	}
	
#ifdef __cplusplus
}
#endif

#endif /* #ifndef BGIO_MACRO_H__ */

/* end of file */

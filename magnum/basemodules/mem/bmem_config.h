/***************************************************************************
 *     Copyright (c) 2001-2012, Broadcom Corporation
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
#ifndef BMEM_CONFIG_H__
#define BMEM_CONFIG_H__

#ifndef BMEM_SAFETY_CONFIG
/***************************************************************************
Summary:
	Used to choose the overall policy the memory manager will follow.

Description:
	Selecting one of these policies applies a number of configuration
	parameters. You can see exactly which ones by looking at the policy
	definitions themselves. They can be one of BMEM_CONFIG_FASTEST,
	BMEM_CONFIG_NORMAL, BMEM_CONFIG_TRACK, BMEM_CONFIG_SAFE, or
	BMEM_CONFIG_SAFEST. The default is to use BMEM_CONFIG_SAFE.

See Also:
	BMEM_CONFIG_FASTEST,
	BMEM_CONFIG_NORMAL,
	BMEM_CONFIG_TRACK,
	BMEM_CONFIG_SAFE,
	BMEM_CONFIG_SAFEST
****************************************************************************/
#define BMEM_SAFETY_CONFIG BMEM_CONFIG_SAFE
#endif


#ifndef BMEM_BOOKKEEPING_CONFIG
/***************************************************************************
Summary:
	Used to choose the location of the bookkeeping information.

Description:
	This information can be located either right with the allocated memory
	(typical for a UMA system) or in OS-allocated memory (typical for a
	slave or client system).

	If there is no time penalty for accessing the managed memory with the
	CPU, then it is recommended to use BMEM_BOOKKEEPING_LOCAL. Using OS-
	allocated memory with BMEM_BOOKKEEPING_SYSTEM is significantly slower
	since it uses a less efficient method of correlating bookkeeping to
	addresses.

	The default is to use BMEM_BOOKKEEPING_SYSTEM.

See Also:
	BMEM_BOOKKEEPING_LOCAL,
	BMEM_BOOKKEEPING_SYSTEM
****************************************************************************/
#define BMEM_BOOKKEEPING_CONFIG BMEM_BOOKKEEPING_SYSTEM
#endif


#ifndef BMEM_REENTRANT_CONFIG
/***************************************************************************
Summary:
	Used to determine whether the memory manager operates in a
	reentrant manner.

Description:
	In most multithreaded situations this must be defined to ensure safe
	operation.  In guaranteed non-reentrant single-threaded operation this
	may be undefined, which will result in a modest speed increase.

	Possible values are BMEM_REENTRANT or BMEM_NOT_REEENTRANT. The default
	is to use BMEM_NOT_REENTRANT.

See Also:
	BMEM_REENTRANT,
	BMEM_NOT_REEENTRANT
****************************************************************************/
#define BMEM_REENTRANT_CONFIG  BMEM_NOT_REENTRANT
#endif


/***************************************************************************
 * There is rarely any need to modify anything below this line.
 ***************************************************************************/

/***************************************************************************
Summary:
	Optimizes the settings for performance, but does no safety checks.

Description:
	The following settings are used:

	o No guard bytes
	o No guard byte checking
	o Don't track file and line number of allocations
	o Don't check if a block is allocated before freeing
	o Don't clear out free blocks
	o Don't check for overlapping blocks before freeing

See Also:
	BMEM_SAFETY_CONFIG
****************************************************************************/
#define BMEM_CONFIG_FASTEST 0


/***************************************************************************
Summary:
	Balances between performance and safety checks.

Description:
	The following settings are used:

	o Short guard byte series
	o Check guard bytes of the block when freed
	o Don't track file and line number of allocations
	o Don't check if a block is allocated before freeing
	o Don't clear out free blocks
	o Don't Check for overlapping blocks before freeing

See Also:
	BMEM_SAFETY_CONFIG
****************************************************************************/
#define BMEM_CONFIG_NORMAL 1


/***************************************************************************
Summary:
	Contains safety checks and tracks allocations.

Description:
	The following settings are used:

	o Short guard byte series
	o Check guard bytes of the block when freed
	o Track file and line number of allocations
	o Don't check if a block is allocated before freeing
	o Don't clear out free blocks
	o Don't Check for overlapping blocks before freeing

See Also:
	BMEM_SAFETY_CONFIG
****************************************************************************/
#define BMEM_CONFIG_TRACK 2


/***************************************************************************
Summary:
	Safe but slow.

Description:
	The following settings are used:

	o Medium guard byte series
	o Check all guards on allocation and free
	o Track file and line number of allocations
	o Check if a block is allocated before freeing
	o Don't clear out free blocks
	o Don't Check for overlapping blocks before freeing

See Also:
	BMEM_SAFETY_CONFIG
****************************************************************************/
#define BMEM_CONFIG_SAFE 3


/***************************************************************************
Summary:
	Safest configuration possible and very slow.

Description:
	The following settings are used:

	o Long guard byte series
	o Check all guards on allocation and free
	o Track file and line number of allocations
	o Check if a block is allocated before freeing.
	o Clear out free blocks
	o Check for overlapping blocks on free.

See Also:
	BMEM_SAFETY_CONFIG
****************************************************************************/
#define BMEM_CONFIG_SAFEST 4


/***************************************************************************
Summary:
	Use the managed memory to store bookkeeping information.

See Also:
	BMEM_BOOKKEEPING_CONFIG
****************************************************************************/
#define BMEM_BOOKKEEPING_LOCAL  0

/***************************************************************************
Summary:
	Use memory allocated by the OS to store bookkeeping information.

See Also:
	BMEM_BOOKKEEPING_CONFIG
****************************************************************************/
#define BMEM_BOOKKEEPING_SYSTEM  1 


/***************************************************************************
Summary:
	This module will use semaphores to protect against rentrancy.

See Also:
	BMEM_REENTRANT_CONFIG
****************************************************************************/
#define BMEM_REENTRANT       0


/***************************************************************************
Summary:
	User guaranteed that reentrant calls won't be made into this module.
	Semaphores will not be used.

See Also:
	BMEM_REENTRANT_CONFIG
****************************************************************************/
#define BMEM_NOT_REENTRANT   1


#endif /* #ifndef BMEM_CONFIG_H__ */

/* End of File */


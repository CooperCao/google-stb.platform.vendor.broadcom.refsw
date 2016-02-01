/***************************************************************************
 *     Copyright (c) 2007, Broadcom Corporation
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
 * Utility to LIFO (e.g. stack)
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef BLIFO_H__
#define BLIFO_H__

#define BLIFO_HEAD(name, type) struct name { type *bl_base; type *bl_last; type *bl_write; }

#define BLIFO_INIT(lifo, base, size) do { (lifo)->bl_base=(lifo)->bl_write=(base);\
	(lifo)->bl_last=(lifo)->bl_base+(size);}while(0)

#define BLIFO_WRITE(lifo) ((lifo)->bl_write)
#define BLIFO_READ(lifo) ((lifo)->bl_write-1)

#define BLIFO_WRITE_PEEK(lifo) ((lifo)->bl_last - (lifo)->bl_write)
#define BLIFO_READ_PEEK(lifo) ((lifo)->bl_write - (lifo)->bl_base)
#define BLIFO_WRITE_COMMIT(lifo, size) do { \
	BDBG_ASSERT((size)>0 && BLIFO_WRITE_PEEK(lifo) >= (size));  \
	(lifo)->bl_write += (size); \
	} while(0)

#define BLIFO_READ_COMMIT(lifo, size) do { \
	BDBG_ASSERT((size)>0 && BLIFO_READ_PEEK(lifo) >= (size)); \
	(lifo)->bl_write -= (size); \
	} while(0)

#define BLIFO_WRITE_LEFT(lifo)  BLIFO_WRITE_PEEK(lifo)

#define BLIFO_READ_LEFT(lifo)  BLIFO_READ_PEEK(lifo)
#define BLIFO_VALIDATE(lifo) do { \
		BDBG_ASSERT((lifo)->bl_write>=(lifo)->bl_base && (lifo)->bl_write<=(lifo)->bl_last); \
	} while(0)

#define BLIFO_STATUS(DBG, header,lifo)	DBG((header ":(%#lx:%#lx) write (%u/%#lx), read (%u/%#lx)", (unsigned long)(lifo)->bl_base, (unsigned long)(lifo)->bl_last, BLIFO_WRITE_PEEK(lifo), (unsigned long)BLIFO_WRITE(lifo), BLIFO_READ_PEEK(lifo), (unsigned long)BLIFO_READ(lifo)))


#endif /* BLIFO_H__ */

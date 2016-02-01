/***************************************************************************
 *     Copyright (c) 2006-2007, Broadcom Corporation
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
 * ASF parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/

#ifndef _BIOVEC_H__
#define _BIOVEC_H__

#define B_IOVEC_FAST	1
#ifndef B_IOVEC_memcpy
/* use memcpy in body of the bio_cursor_copy */
/* #define B_IOVEC_memcpy 1 */
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/* this type is used to define I/O vector */
typedef struct bio_vec {
	void *base;   /* Starting address */
	size_t len;   /* Number of bytes */
} bio_vec;

#define B_IO_ARRAY_INLINE	2
/* this type is used to describe Array of I/O vectors */
typedef struct bio_array {
	unsigned count; /* number of entrries in the array */
	bio_vec *vec; /* variable length array */
	bio_vec vec_[B_IO_ARRAY_INLINE]; /* inline array */
	unsigned size; /* number of allocated entries */
	size_t length; /* total length of data accumulated in the array */
	bio_vec const *owner_[B_IO_ARRAY_INLINE];
	bio_vec const * * owner; 
}bio_array;

/* this type is used to define read-only cursor for the array of I/O vectors */
typedef struct bio_cursor {
	const uint8_t *cursor;
	int	left;
	unsigned pos;
	const bio_array *array;
} bio_cursor;
#define BIO_CURSOR_DUMP(DBG, header, cursor) DBG((header "%#lx cursor:%#lx left:%d pos:%u array:%#x", (unsigned long)(cursor),  (cursor)->cursor, (cursor)->left, (cursor)->pos, (unsigned long)(cursor)->array))

/* this type is used to save read position in the cursor */
typedef struct bio_checkpoint {
	int	left;
	unsigned pos;
} bio_checkpoint;

/* this function initializes array */
void bio_array_init(bio_array *array);
/* this function created array from list of the I/O vectors */
void bio_array_from_iov(bio_array *array, const bio_vec *vec, unsigned count);

/* this function sets array length to zero */
void bio_array_reset(bio_array *array);

/* this function releases all resources allocated for the array */
void bio_array_shutdown(bio_array *array);

/* this function appends vector  to the array */
size_t bio_array_add_vec(bio_array *array, const bio_vec *vec);

/* this function appends block of memory to the array */
size_t bio_array_add(bio_array *array, const void *base, size_t len);

/* this function is used to trim array at current cursor position, the iov_free function would be called for each I/O vector removed from the array */
size_t bio_array_trim(bio_array *array, bio_cursor *cursor, void (*iov_free)(void *cnxt, const bio_vec *v), void *cnxt);

size_t bio_array_len(const bio_array *array);


/* this function is used to initialize cursor from array, after initialization cursor will point to first byte in the array */
void bio_cursor_init(bio_cursor *cursor, const bio_array *array);
/* this function is used to initialize cursor from list of I/O vectors, after initialization cursor will point to first byte in the first I/O vector*/
void bio_cursor_from_iov(bio_cursor *cursor, bio_array *array, const bio_vec *vec, unsigned count);
/* this function is used to initialize cursor from the memory range, after initialization cursor will point to first byte */
void bio_cursor_from_range(bio_cursor *cursor, bio_array *array, const void *base, size_t len);
/* this function is used to create empty cursor */
void bio_cursor_empty(bio_cursor *cursor);

/* this function saves current cursor position to the checkpoint */
void bio_cursor_save(const bio_cursor *cursor, bio_checkpoint *checkpoint);
/* this function restores cursor's position from the checkpoint */
void bio_cursor_rollback(bio_cursor *cursor, const bio_checkpoint *checkpoint);
/* this function extracts one I/O vector from the cursor, and then advanced to number of extracted bytes, if EOF reached function will return 0 */
size_t bio_cursor_get(bio_cursor *cursor, size_t size, bio_vec *vec, bio_vec const **owner);

#define BIO_EOF	(-1)

/* this function reads next byte from the cursor, if EOF reached it will return BIO_EOF */
int bio_cursor_next(bio_cursor *cursor);
/* this function advances current cursor position to 'count' bytes */
void bio_cursor_skip(bio_cursor *cursor, size_t count);

/* this function reserves 'count' bytes in the cursor, it returns number of bytes that were actually reserved */
size_t bio_cursor_reserve(bio_cursor *cursor, size_t count);

/* this function copies 'count' bytes from the cursor, it returns number of bytes actually copied */
size_t bio_cursor_copy(bio_cursor *cursor, void *dest, size_t count);

/* this function returns true if there is nothing to read from the cursor */
bool bio_cursor_eof(const bio_cursor *cursor);
/* this function returns current position in the cursor */
size_t bio_cursor_pos(const bio_cursor *cursor);

/* this function returns new cursor, intenticaled to the old one */
void bio_cursor_clone(bio_cursor *dst, const bio_cursor *src);

#if B_IOVEC_FAST
#define BIO_NEXT(d,c) do {if((c)->left>0) {(c)->left--;d=*(c)->cursor++;} else {d=bio_cursor_next(c);}}while(0)
#define BIO_IS_EOF(c) ((c)->left<0)
#define BIO_SAVE(cur,check) do {(check)->left=(cur)->left;(check)->pos=(cur)->pos;}while(0)
#define BIO_ALEN(a) ((a)->length) 
#define BIO_CLONE(dst,src) *(dst)=*(src)
#else
#define BIO_NEXT(d,c) (d)=bio_cursor_next(c)
#define BIO_IS_EOF(c) bio_cursor_eof(c)
#define BIO_SAVE(cur,check) bio_cursor_save((cur),(check))
#define BIO_ALEN(a) bio_array_len(a)
#define BIO_CLONE(dst,src) bio_cursor_clone((dst),(src))
#endif

#ifdef __cplusplus
}
#endif

#endif /* _BIOVEC_H__ */



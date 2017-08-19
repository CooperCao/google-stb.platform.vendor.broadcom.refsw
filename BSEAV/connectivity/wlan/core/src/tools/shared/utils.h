/*
 * utils.h - various portable utility routines and definitions
 *
 * Copyright 1999, Broadcom Corporation.
 * $Id$
 */

#ifndef _utils_h_
#define _utils_h_


#ifndef MIN
#define	MIN(a,b)	(((a) <= (b))? (a): (b))
#define	MAX(a,b)	(((a) >= (b))? (a): (b))
#endif

#define ROUNDUP(x, y)	((((x)+((y)-1))/(y))*(y))

#ifndef setbit
/* Bit map related macros.  */
#define NBBY		8
#define setbit(a,i)     ((a)[(i)/NBBY] |= 1<<((i)%NBBY))
#define clrbit(a,i)     ((a)[(i)/NBBY] &= ~(1<<((i)%NBBY)))
#endif /* setbit */

#define	ALIGNED(addr, alignment)	(((uint)(addr) & (alignment-1)) == 0)

#ifndef ASSERT
#ifdef BCMDBG
#define	ASSERT(exp)	if (exp) ; else assfail(#exp, __FILE__, __LINE__)
#else
#define	ASSERT(exp)
#endif
#endif

#ifndef PAGE_SIZE
#define	PAGE_SIZE	4096
#endif

#define NUM_ETCS 20

#ifdef __cplusplus
extern "C" {
#endif

/* externs */
extern int doingerrors;
extern int curcall;
extern int failcall;
extern int randfail;
extern int uniq;

/* prototypes */
extern void utils_init(void);
extern void assfail(char *exp, char *file, int line);
extern void pattern(uint8 *buf, uint len);
extern void *mallocdeadbeef(size_t len);
extern void deadbeef(void *buf, uint len);
extern void prhex(const char *msg, uchar *buf, uint nbytes);
extern int reterror(void);
extern void syserr(char *s);
extern void err(char *fmt, ...);
extern int fieldsize(uint size, uint fe, uint le);
extern int fieldmask(uint be);
extern int fieldoff(uint be);
extern int atomac(char* mac_string, uint8* mac);
extern int prmac(char *buf, uint8* addr);

#ifndef _UNISTD_H_
#define	F_OK	0
#define	X_OK	1
#define	W_OK	2
#define	R_OK	4

extern int access(const char *file, int acc);
#endif

extern void xmalloc_set_program_name(const char *name);
extern void *xmalloc(unsigned long size);
extern void *xrealloc(void *oldmem, unsigned long size);
extern char *concat(const char *first, ...);

#ifdef __cplusplus
}
#endif

#endif /* _utils_h_ */

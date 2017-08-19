#ifndef _OS_H
#define _OS_H

#include <asm/errno.h>

#define OS_DEBUG		if (0) os_printf
#define OS_ERROR		os_printerr

#ifndef NULL
#define NULL			((void *)0)
#endif

#define __STRING(x)		#x
#define ASSERT(expr)		((expr) ? 0 : os_assfail(__STRING(expr), __FILE__, __LINE__))

typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

typedef int int32;
typedef short int16;
typedef char int8;

typedef uint32 os_msec_t;

#define OS_MSEC_FOREVER		((os_msec_t)0xffffffff)

typedef struct {
	int data[16];
} __attribute__((aligned(8))) os_sem_t;

#define os_errno_set(errno)	(os_errno = (errno))

extern int os_errno;
extern char *os_strerror(int eno);

extern int os_printf(const char *fmt, ...)
     __attribute__ ((__format__ (__printf__, 1, 2)));
extern int os_printerr(const char *fmt, ...)
     __attribute__ ((__format__ (__printf__, 1, 2)));
extern int os_sprintf(char *buf, const char *fmt, ...)
     __attribute__ ((__format__ (__printf__, 2, 3)));

extern void os_assfail(char *expr, char *file, int line);

extern int os_msleep(os_msec_t dur);
extern os_msec_t os_msec(void);

extern void *os_malloc(unsigned long size);
extern void os_free(void *ptr);

extern void *os_memcpy(void *dst, const void *src, unsigned long size);
extern void *os_memset(void *dst, int ch, unsigned long size);

extern unsigned long os_strlen(const char *src);
extern int os_strcmp(const char *s1, const char *s2);
extern char *os_strcpy(char *dst, const char *src);
extern char *os_strncpy(char *dst, const char *src, unsigned long maxlen);

extern void os_sem_init(os_sem_t *sem, int count);
extern void os_sem_give(os_sem_t *sem);
extern int os_sem_take(os_sem_t *sem, os_msec_t tmout, int sig);

typedef struct os_pkt_opaque os_pkt_t;

#define os_pkt_alloc(len)		rst_pkt_alloc(len)
#define os_pkt_free(pkt)		rst_pkt_free(pkt)
#define os_pkt_data(pkt)		rst_pkt_data(pkt)
#define os_pkt_len(pkt)			rst_pkt_len(pkt)
#define os_pkt_output(pkt)		rst_pkt_output(pkt)

extern os_pkt_t *rst_pkt_alloc(int len);
extern void rst_pkt_free(os_pkt_t *pkt);
extern void *rst_pkt_data(os_pkt_t *pkt);
extern int rst_pkt_len(os_pkt_t *pkt);
extern int rst_pkt_output(os_pkt_t *pkt);

#endif /* _OS_H */

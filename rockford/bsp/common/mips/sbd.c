/* $Id: sbd.c,v 1.6 1998/03/18 15:33:29 chris Exp $ */
#include "bstd.h"
#include "bcmuart.h"
#include "termio.h"
#include <stdarg.h>

extern int bcmuart(void);

/*const*/ ConfigEntry ConfigTable[] =
{
    {(Addr)(UART_ADR_BASE), 0, bcmuart, 256, B9600},
    {0,0,0,0,0}
};


struct ifent {
    int		if_type;
    void	(*if_func)(void);
};

struct ifent __SDEFINI_LIST__[1];



int getpagesize (void)
{
    return 4096;
}


long gsignal (char *jb, int sig)
{
	BSTD_UNUSED(jb);
	BSTD_UNUSED(sig);
	return 0;
}

void sbdpoll (void)
{
    /* poll any special devices */
}

char *getenv (const char *name)
{
	BSTD_UNUSED(name);
	return 0;
}

int getbaudrate (char *p)
{
	BSTD_UNUSED(p);
	return 0;
}

int _mon_printf (const char *fmt, ...)
{
    int             len;
    va_list         ap;

    va_start(ap, fmt);
    len = vfprintf (stdout, fmt, ap);
    va_end(ap);
    return (len);
}

#if 0
long netopen (void)	{return -1;}
long netread (void)	{return -1;}
long netwrite (void)	{return -1;}
long netclose (void)	{return -1;}
long netlseek (void)	{return -1;}
long netioctl (void)	{return -1;}
void netpoll (void)	{};

void cerrpoll (void) {}


FILE  _iob[OPEN_MAX] = {}

_call_init_funcs() {}

int _mem_top = 64 * 0x100000;


/* stdio.h */
int ftrylockfile(FILE *file)
{
dbg_print("ftrylockfile");
}


/* #include "pthread.h" */
/* dynamic package initialization (POSIX) */

int _pthread_once (pthread_once_t *thread, void (* initFunc)(void) )
{
dbg_print("_pthread_once");
return 0;
}


/*
 * libc_thread.h
 *
 * In threaded mode, return a pointer to thread-private memory of
 * the same size as, and (initially) with the same contents as 'storage'. If
 * an error occurs, the 'error' parameter is returned.
 * In single-threaded mode, no storage is allocated. Instead, a pointer
 * to storage is always returned.
 */

void *	_libc_private_storage(struct _thread_private_key_struct *key,
			      void *param1, size_t size, void *param2)
{
dbg_print("_libc_private_storage");
}

#endif






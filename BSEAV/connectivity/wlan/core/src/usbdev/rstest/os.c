#include "os.h"

/*
 * A lot of stuff is here because Linux headers can't be included
 * directly in os.h.  They pull in Linux socket structure and macro
 * definitions, which then conflict in rsock.c.
 */

#include <linux/config.h>
#include <linux/version.h>

#include <linux/module.h>
#if defined(MODVERSIONS)
#include <linux/modversions.h>
#endif

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/slab.h>

#define DPRINTF			if (0) printk

int os_errno;

#define MSEC_TO_JIFFY(m)	(((m) * HZ + 999) / 1000)
#define JIFFY_TO_MSEC(j)	((j) * 1000 / HZ)

int
os_printf(const char *fmt, ...)
{
	va_list ap;
	char buf[256];
	int n;

	va_start(ap,fmt);
	n = vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	printk("%s", buf);

	return n;
}

int
os_printerr(const char *fmt, ...)
{
	va_list ap;
	char buf[256];
	int n;

	va_start(ap,fmt);
	n = vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	printk("%s", buf);

	return n;
}

int
os_sprintf(char *buf, const char *fmt, ...)
{
	va_list ap;
	int n;

	va_start(ap,fmt);
	n = vsprintf(buf, fmt, ap);
	va_end(ap);

	return n;
}

char *
os_strerror(int eno)
{
	static char buf[32];
	sprintf(buf, "errno=%d", eno);
	return buf;
}

void
os_assfail(char *expr, char *file, int line)
{
	printk("Assertion (%s) failed at %s:%d\n", expr, file, line);
}

int
os_msleep(os_msec_t dur)
{
	current->state = TASK_INTERRUPTIBLE;
	schedule_timeout(1 + MSEC_TO_JIFFY(dur));

	if (signal_pending(current)) {
		DPRINTF("os_msleep: interrupted\n");
		return EINTR;
	}

	return 0;
}

/*
 * Bug: os_msec does not wrap-around cleanly.
 * Higher precision (64-bit) arithmetic is needed.
 */

os_msec_t
os_msec(void)
{
	return JIFFY_TO_MSEC(jiffies);
}

void *
os_malloc(unsigned long size)
{
	return kmalloc(size, GFP_KERNEL);
}

void
os_free(void *ptr)
{
	kfree(ptr);
}

void *
os_memcpy(void *dst, const void *src, unsigned long size)
{
	return memcpy(dst, src, size);
}

void *
os_memset(void *dst, int ch, unsigned long size)
{
	return memset(dst, ch, size);
}

unsigned long
os_strlen(const char *src)
{
	return strlen(src);
}

int
os_strcmp(const char *s1, const char *s2)
{
	return strcmp(s1, s2);
}

char *
os_strcpy(char *dst, const char *src)
{
	return strcpy(dst, src);
}

char *
os_strncpy(char *dst, const char *src, unsigned long maxlen)
{
	return strncpy(dst, src, maxlen);
}

void
os_sem_init(os_sem_t *sem, int count)
{
	struct semaphore *lsem = (struct semaphore *)sem->data;

	/* Using sem->data[15] to indicate expiry */
	memset(sem, 0, sizeof(*sem));
	sema_init(lsem, count);
}

void
os_sem_give(os_sem_t *sem)
{
	struct semaphore *lsem = (struct semaphore *)sem->data;
	up(lsem);
}

static void
_os_sem_take_expire(unsigned long arg)
{
	os_sem_t *sem = (os_sem_t *)arg;
	struct semaphore *lsem = (struct semaphore *)sem->data;

	sem->data[15] = 1;	/* Indicate expiry */
	up(lsem);
}

/*
 * 'sig' indicates fast timeout mode for a signaling semaphore.  A signaling semaphore
 * is a binary semaphore (value 0 or 1) that only one task can wait on, and that must
 * be re-initialized before each use.
 */
int
os_sem_take(os_sem_t *sem, os_msec_t tmout, int sig)
{
	struct semaphore *lsem = (struct semaphore *)sem->data;
	int rv = 0;

	if (tmout == OS_MSEC_FOREVER) {
		if (down_interruptible(lsem))
			rv = EINTR;
	} else if (sig) {
		struct timer_list timer;

		init_timer(&timer);
		timer.expires = jiffies + MSEC_TO_JIFFY(tmout);
		timer.data = (unsigned long)sem;
		timer.function = _os_sem_take_expire;

		sem->data[15] = 0;
		add_timer(&timer);
		if (down_interruptible(lsem))
			rv = EINTR;
		else if (sem->data[15])
			rv = ETIMEDOUT;
		del_timer_sync(&timer);
	} else {
		unsigned int jiff = MSEC_TO_JIFFY(tmout);

		while (down_trylock(lsem)) {
			if (jiff == 0) {
				rv = ETIMEDOUT;
				break;
			}

			current->state = TASK_INTERRUPTIBLE;
			schedule_timeout(1);
			jiff--;

			if (signal_pending(current)) {
				rv = EINTR;
				break;
			}
		}
	}

	if (rv == ETIMEDOUT)
		DPRINTF("os_sem_take: timed out\n");

	if (rv == EINTR)
		DPRINTF("os_sem_take: interrupted\n");

	return rv;
}

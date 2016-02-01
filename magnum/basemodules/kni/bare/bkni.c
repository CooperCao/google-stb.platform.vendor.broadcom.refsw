/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 * $brcm_Log: $ *
 * 
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bkni_event_group.h"
#include "blst_list.h"
#include "b_bare_os.h"

#ifdef LINUX
#error No linux dependency
#endif

#if 0
static struct {
    int unused;
} BKNI_State;
#endif

static int BKNI_P_Printf(bool high_priority, const char *fmt, ...);


BDBG_MODULE(kernelinterface);

BDBG_OBJECT_ID(BKNI_EventGroup);
BDBG_OBJECT_ID(BKNI_Event);
BDBG_OBJECT_ID(BKNI_Mutex);

struct BKNI_MutexObj
{
    BDBG_OBJECT(BKNI_Mutex)
    b_bare_os_lock  lock;
};

struct BKNI_GroupObj {
    BDBG_OBJECT(BKNI_EventGroup)
    BLST_D_HEAD(group, BKNI_EventObj) members;
    b_bare_os_lock lock;
    b_bare_os_signal signal;
};

struct BKNI_EventObj {
    BDBG_OBJECT(BKNI_Event)
    BLST_D_ENTRY(BKNI_EventObj) list;
    struct BKNI_GroupObj *group;
    b_bare_os_signal signal;
};

#define _U  0x01    /* upper */
#define _L  0x02    /* lower */
#define _D  0x04    /* digit */
#define _C  0x08    /* cntrl */
#define _P  0x10    /* punct */
#define _S  0x20    /* white space (space/lf/tab) */
#define _X  0x40    /* hex digit */
#define _SP 0x80    /* hard space (0x20) */

static const unsigned char _ctype[] = {
_C,_C,_C,_C,_C,_C,_C,_C,            /* 0-7 */
_C,_C|_S,_C|_S,_C|_S,_C|_S,_C|_S,_C,_C,     /* 8-15 */
_C,_C,_C,_C,_C,_C,_C,_C,            /* 16-23 */
_C,_C,_C,_C,_C,_C,_C,_C,            /* 24-31 */
_S|_SP,_P,_P,_P,_P,_P,_P,_P,            /* 32-39 */
_P,_P,_P,_P,_P,_P,_P,_P,            /* 40-47 */
_D,_D,_D,_D,_D,_D,_D,_D,            /* 48-55 */
_D,_D,_P,_P,_P,_P,_P,_P,            /* 56-63 */
_P,_U|_X,_U|_X,_U|_X,_U|_X,_U|_X,_U|_X,_U,  /* 64-71 */
_U,_U,_U,_U,_U,_U,_U,_U,            /* 72-79 */
_U,_U,_U,_U,_U,_U,_U,_U,            /* 80-87 */
_U,_U,_U,_P,_P,_P,_P,_P,            /* 88-95 */
_P,_L|_X,_L|_X,_L|_X,_L|_X,_L|_X,_L|_X,_L,  /* 96-103 */
_L,_L,_L,_L,_L,_L,_L,_L,            /* 104-111 */
_L,_L,_L,_L,_L,_L,_L,_L,            /* 112-119 */
_L,_L,_L,_P,_P,_P,_P,_C,            /* 120-127 */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        /* 128-143 */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        /* 144-159 */
_S|_SP,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,   /* 160-175 */
_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,       /* 176-191 */
_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,       /* 192-207 */
_U,_U,_U,_U,_U,_U,_U,_P,_U,_U,_U,_U,_U,_U,_U,_L,       /* 208-223 */
_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,       /* 224-239 */
_L,_L,_L,_L,_L,_L,_L,_P,_L,_L,_L,_L,_L,_L,_L,_L};      /* 240-255 */


#define __ismask(x) (_ctype[(int)(unsigned char)(x)])

#define isalnum(c)  ((__ismask(c)&(_U|_L|_D)) != 0)
#define isalpha(c)  ((__ismask(c)&(_U|_L)) != 0)
#define iscntrl(c)  ((__ismask(c)&(_C)) != 0)
#define isdigit(c)  ((__ismask(c)&(_D)) != 0)
#define isgraph(c)  ((__ismask(c)&(_P|_U|_L|_D)) != 0)
#define islower(c)  ((__ismask(c)&(_L)) != 0)
#define isprint(c)  ((__ismask(c)&(_P|_U|_L|_D|_SP)) != 0)
#define ispunct(c)  ((__ismask(c)&(_P)) != 0)
#define isspace(c)  ((__ismask(c)&(_S)) != 0)
#define isupper(c)  ((__ismask(c)&(_U)) != 0)
#define isxdigit(c) ((__ismask(c)&(_D|_X)) != 0)

#define isascii(c) (((unsigned char)(c))<=0x7f)
#define toascii(c) (((unsigned char)(c))&0x7f)

BSTD_INLINE int __tolower(int c)
{
    return isupper(c)? c-('A'-'a'): c;
}

BSTD_INLINE  int __toupper(int c)
{
    return islower(c)? c-('a'-'A'): c;
}

#define tolower(c) __tolower(c)
#define toupper(c) __toupper(c)

static int
b_bare_atoi(const char **s)
{
    int i=0;

    while (isdigit(**s)) {
        i = i*10 + *((*s)++) - '0';
    }
    return i;
}

static b_bare_os_lock g_isr_lock;

/* This pointer must be loaded with the real interface structure populated with the correct OS funtion pointers. */
b_bare_os_interface *pb_bare_os = NULL;

BERR_Code
BKNI_Init(void)
{
    pb_bare_os = b_get_bare_os();
	if(!pb_bare_os) return BERR_OS_ERROR;

    g_isr_lock = pb_bare_os->get_interrupt_lock();
    return BERR_SUCCESS;
}

void
BKNI_Uninit(void)
{
    return;
}

void
BKNI_Fail(void)
{
    BKNI_P_Printf(true, "BKNI_Fail is intentionally causing a segfault. Please inspect any prior error messages or get a core dump stack trace to determine the cause of failure.\n");
    *(volatile unsigned char *)0;
}

#define B_BARE_ZEROPAD (1<<0)
#define B_BARE_SIGN    (1<<1)
#define B_BARE_PLUS    (1<<2)
#define B_BARE_SPACE   (1<<3)
#define B_BARE_LEFT    (1<<4)
#define B_BARE_SPECIAL (1<<5)
#define B_BARE_LARGE   (1<<6)

static char *
b_bare_printf_format_one(char * buf, char * end, const char *tmp, int digits, bool negative, int base, int size, int precision, int type)
{
    char sign;

    if (digits > precision) {
        precision = digits;
    }

    size -= precision;
    if (!(type&(B_BARE_ZEROPAD+B_BARE_LEFT))) {
        while(size-->0) {
            if (buf <= end) {
                *buf = ' ';
            }
            buf++;
        }
    }

    sign = '\0';
    if (type & B_BARE_SIGN) {
        if (negative) {
            sign = '-';
            size--;
        } else if (type & B_BARE_PLUS) {
            sign = '+';
            size--;
        } else if (type & B_BARE_SPACE) {
            sign = ' ';
            size--;
        }
    }

    if (sign) {
        if (buf <= end) {
            *buf = sign;
        }
        ++buf;
    }
    if (type & B_BARE_SPECIAL) {
        if (base==8) {
            if (buf <= end) {
                *buf = '0';
            }
            buf++;
        } else if (base==16) {
            if (buf <= end) {
                *buf = '0';
            }
            buf++;
            if (buf <= end) {
                *buf = type&B_BARE_LARGE?'X':'x';
            }
            buf++;
        }
    }
    if (!(type & B_BARE_LEFT)) {
        char c = (type & B_BARE_ZEROPAD) ? '0' : ' ';
        while (size-- > 0) {
            if (buf <= end) {
                *buf = c;
            }
            buf++;
        }
    }
    while (digits < precision--) {
        if (buf <= end) {
            *buf = '0';
        }
        buf++;
    }
    while (digits-- > 0) {
        if (buf <= end) {
            *buf = tmp[digits];
        }
        buf++;
    }
    while (size-- > 0) {
        if (buf <= end) {
            *buf = ' ';
        }
        buf++;
    }
    return buf;
}

static char *
b_bare_format_number(char * buf, char * end, long long num, int base, int size, int precision, int type)
{
    char tmp[64];
    const char *digits;
    static const char small_digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    static const char large_digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int i;
    bool negative;

    digits = (type & B_BARE_LARGE) ? large_digits : small_digits;
    if (base < 2 || base > 36) {
        return 0;
    }
    if (type & B_BARE_LEFT) {
        type &= ~B_BARE_ZEROPAD;
    }
    if (type & B_BARE_SPECIAL) {
        if (base == 16) {
            size -= 2;
        } else if (base == 8) {
            size--;
        }
    }

    if(num<0) {
        num = -num;
        negative = true;
    } else {
        negative = false;
    }
    i=0;
    do {
        unsigned index = num % base;
        num = num / base;
        tmp[i++] = digits[index];
    } while(num!=0);

    return b_bare_printf_format_one(buf, end, tmp, i, negative, base, size, precision, type);
}

#define FLOAT   128
union double_uint64 {
    double d;
    uint64_t x;
};

/* IEEE Standard 754 Floating Point Numbers */
static int b_bare_mini_isinf(double d) {
    union double_uint64 plus_inf, minus_inf, v;

    v.d = d;
    plus_inf.d = __builtin_inf();
    minus_inf.d = -plus_inf.d;
    if(v.x == plus_inf.x) {
        return 1;
    } else if(v.x == minus_inf.x) {
        return -1;
    } else {
        return 0;
    }
}

static bool mini_isnan(double d) {
    return d!=d;
}

static char *format_double(char *str, char *end, double d, int size, int precision, int type)
{
    char c,buf[64];
    unsigned long whole, frac;
    unsigned frac_scale;
    bool negative=false;
    unsigned i;
    const char *result;

    if(mini_isnan(d)) {
        static const char str_nan[]=">nan<";
        result = str_nan;
        i = sizeof(str_nan)-1;
    } else if(b_bare_mini_isinf(d)) {
        static const char str_inf[]=">fni<";
        result = str_inf;
        i = sizeof(str_inf)-1;
    } else {
        if(d<0) {
            d = -d;
            negative = true;
        }
        if(precision<0) {
            precision = 6;
        }
        frac_scale=1;
        if(precision>0) {
            for(i=0;(int)i<precision;i++) {
                frac_scale *= 10;
            }

            whole = d;
            frac = ((d-(double)whole) + (1.0/(2*frac_scale))) * (double)frac_scale;
        } else {
            whole = d+0.5;
            frac = 0;
        }
        /* printf("\nwhole %u frac %u\n", whole, frac); */
        for(i=0;frac_scale>1;) {
            c = frac % 10;
            frac_scale /= 10;
            frac /= 10;
            buf[i++] = c + '0';
            if(i>=sizeof(buf)) {
                break;
            }
        }
        if(i<sizeof(buf) && i!=0) {
            buf[i++] = '.';
        }
        do {
            if(i>=sizeof(buf)) {
                break;
            }
            c = whole % 10;
            whole /= 10;
            buf[i++] = c+'0';
        } while(whole>0);
        result = buf;
    }
    return b_bare_printf_format_one(str, end, result, i, negative, 10, size, precision, type);
}

static size_t
strnlen(const char * s, size_t count)
{
    const char *sc;

    for (sc = s; count-- && *sc != '\0'; ++sc) {
    }
    return sc - s;
}



#define WRITE_CH(ch) do {if (str <= end) { *str = (ch); } str++; } while(0)

int
BKNI_Vsnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
    size_t len;
    unsigned long long num;
    char qualifier;
    int i;
    unsigned base;
    char *str, *end, c;
    const char *s;

    unsigned flags;      /* flags to b_bare_format_number() */

    int field_width;    
    int precision;     

    end = buf + size - 1;

    if (end < buf - 1) {
        end = ((void *) -1);
        size = end - buf + 1;
    }

    for (str=buf; *fmt ; ++fmt) {
        if (*fmt != '%') {
            WRITE_CH(*fmt);
            continue;
        }

        /* process flags */
        flags = 0;
        do {
            fmt++;      
            switch (*fmt) {
                case '0': flags |= B_BARE_ZEROPAD; continue;
                case '#': flags |= B_BARE_SPECIAL; continue;
                case '-': flags |= B_BARE_LEFT; continue;
                case '+': flags |= B_BARE_PLUS; continue;
                case ' ': flags |= B_BARE_SPACE; continue;
            }
        } while(false);

        /* get field width */
        field_width = -1;
        if (isdigit(*fmt)) {
            field_width = b_bare_atoi(&fmt);
        } else if (*fmt == '*') {
            fmt++;
            /* it's the next argument */
            field_width = va_arg(args, int);
            if (field_width < 0) {
                field_width = -field_width;
                flags |= B_BARE_LEFT;
            }
        }

        /* get the precision */
        precision = -1;
        if (*fmt == '.') {
            ++fmt;
            if (isdigit(*fmt)) {
                precision = b_bare_atoi(&fmt);
            } else if (*fmt == '*') {
                ++fmt;
                /* it's the next argument */
                precision = va_arg(args, int);
            }
            if (precision < 0) {
                precision = 0;
            }
        }

        /* get the conversion qualifier */
        qualifier = '0';
        switch(*fmt) {
        case 'H':
        case 'l':
        case 'L':
        case 'Z':
            qualifier = *fmt;
            fmt++;
            if (qualifier == 'l' && *fmt == 'l') {
                qualifier = 'L';
                fmt++;
            }
            break;
        }

        /* default base */
        base = 10;

        switch (*fmt) {
            case 'c':
                if (!(flags & B_BARE_LEFT)) {
                    while (--field_width > 0) {
                        WRITE_CH(' ');
                    }
                }
                c = (unsigned char) va_arg(args, int);
                WRITE_CH(c);
                while (--field_width > 0) {
                    WRITE_CH(' ');
                }
                continue;

            case 's':
                s = va_arg(args, char *);
                if (s==NULL) {
                    s = "<NULL>";
                }

                len = strnlen(s, precision);
                len = strnlen(s, precision);

                if (!(flags & B_BARE_LEFT)) {
                    while (len < field_width--) {
                        WRITE_CH(' ');
                    }
                }
                for (i = 0; i < len; i++) {
                    WRITE_CH(s[i]);
                }
                while (len < field_width--) {
                    WRITE_CH(' ');
                }
                continue;

            case 'p':
                if (field_width == -1) {
                    field_width = 2*sizeof(void *);
                    flags |= B_BARE_ZEROPAD;
                }
                str = b_bare_format_number(str, end,
                        (unsigned long) va_arg(args, void *),
                        16, field_width, precision, flags);
                continue;

            case 'n':
                switch(qualifier){
                case 'l':
                    {
                        long * ip = va_arg(args, long *);
                        *ip = (str - buf);
                        break;
                    }
                case 'Z':
                    {
                        size_t * ip = va_arg(args, size_t *);
                        *ip = (str - buf);
                        break;
                    }
                default:
                    {
                        int * ip = va_arg(args, int *);
                        *ip = (str - buf);
                        break;
                    }
                }
                continue;

            case '%':
                WRITE_CH('%');
                continue;

                /* integer b_bare_format_number formats - set up the flags and "break" */
            case 'o':
                base = 8;
                break;

            case 'X':
                flags |= B_BARE_LARGE;
            case 'x':
                base = 16;
                break;

            case 'd':
            case 'i':
                flags |= B_BARE_SIGN;
            case 'u':
                break;
            case 'F':
                flags |= B_BARE_LARGE;
            case 'f':
                flags |= FLOAT|B_BARE_SIGN;
                break;
            default:
                WRITE_CH('%');
                if (*fmt) {
                    WRITE_CH(*fmt);
                } else {
                    fmt--;
                }
                continue;
        }
        if(flags & FLOAT) {
            double d;
            switch(qualifier) {
            case 'L':
                d = va_arg(args, long double);
                break;
            case 'l':
                d = va_arg(args, double);
                break;
            default:
                /*  'float' is promoted to 'double' when passed through '...' */
                d = va_arg(args, double);
                break;
            }
            str = format_double(str, end, d, field_width, precision, flags);
            continue;
        }
        switch(qualifier) {
        case 'L':
            num = va_arg(args, long long);
            break;
        case 'l':
            num = va_arg(args, unsigned long);
            if (flags & B_BARE_SIGN) {
                num = (signed long) num;
            }
            break;
        case 'Z':
            num = va_arg(args, size_t);
            break;
        case 'h':
            num = (unsigned short) va_arg(args, int);
            if (flags & B_BARE_SIGN) {
                num = (signed short) num;
            }
            break;
        default:
            num = va_arg(args, unsigned int);
            if (flags & B_BARE_SIGN) {
                num = (signed int) num;
            }
            break;
        }
        str = b_bare_format_number(str, end, num, base, field_width, precision, flags);
    }
    if (str <= end) {
        *str = '\0';
    } else if (size > 0) {
        *end = '\0';
    }
    return str-buf;
}

int
BKNI_Snprintf(char *s, size_t n, const char *fmt, ...)
{
    va_list ap;
    int rc;
    va_start(ap, fmt);
    rc = BKNI_Vsnprintf(s, n, fmt, ap);
    va_end(ap);
    return rc;
}


static int
BKNI_P_Vprintf(bool high_priority, const char *fmt, va_list ap)
{
    char stdout_buf[256]; /* keep it small */
    int rv;
    rv = BKNI_Vsnprintf(stdout_buf, sizeof(stdout_buf), fmt, ap);
    if (rv>0) {
        pb_bare_os->print_debug(high_priority, stdout_buf);
    }
    return rv;
}

static int
BKNI_P_Printf(bool high_priority, const char *fmt, ...)
{
    int rv;
    va_list arg;

    va_start(arg, fmt);
    rv = BKNI_P_Vprintf(high_priority, fmt, arg);
    va_end(arg);
    return rv;
}

int
BKNI_Vprintf(const char *fmt, va_list ap)
{
    return BKNI_P_Vprintf(false, fmt, ap);
}

int
BKNI_Printf(const char *fmt, ...)
{
    int rv;
    va_list arg;

    va_start(arg, fmt);
    rv = BKNI_Vprintf(fmt, arg);
    va_end(arg);
    return rv;
}


void
BKNI_Delay(unsigned int microsec)
{
    pb_bare_os->delay(microsec);
}

void *
BKNI_Malloc(size_t size)
{
    return pb_bare_os->malloc(size);
}

void
BKNI_Free(void *ptr)
{
    pb_bare_os->free(ptr);
}

BERR_Code
BKNI_Sleep(unsigned int millisec)
{
    pb_bare_os->sleep(millisec);
    return BERR_SUCCESS;
}


BERR_Code
BKNI_CreateEvent(BKNI_EventHandle *event)
{
    BERR_Code rc;
    BKNI_EventHandle e = BKNI_Malloc(sizeof(*e));
    if(e==NULL) {
        rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(e, BKNI_Event);
    e->group = NULL;
    e->signal = pb_bare_os->signal_create();
    if(e->signal==NULL) {
        rc=BERR_TRACE(BERR_OS_ERROR);
        goto err_signal;
    }
    *event = e;
    return BERR_SUCCESS;

err_signal:
    BKNI_Free(e);
err_alloc:
    return rc;
}

void
BKNI_DestroyEvent(BKNI_EventHandle event)
{
    BKNI_EventGroupHandle group;
    BDBG_OBJECT_ASSERT(event, BKNI_Event);
    group = event->group;
    if (event->group) {
        BDBG_WRN(("Event %#x still in the group %#x, removing it", (unsigned)event, (unsigned)group));
        pb_bare_os->lock_acquire(group->lock);
        BLST_D_REMOVE(&group->members, event, list);
        pb_bare_os->lock_release(group->lock);
    }
    pb_bare_os->signal_destroy(event->signal);
    BDBG_OBJECT_DESTROY(event, BKNI_Event);
    BKNI_Free(event);
}

void
BKNI_SetEvent(BKNI_EventHandle event)
{
    BKNI_EventGroupHandle group;
    BDBG_OBJECT_ASSERT(event, BKNI_Event);
    pb_bare_os->signal_set(event->signal);
    group = event->group;
    if(group) {
        pb_bare_os->signal_set(group->signal);
    }
}

void
BKNI_ResetEvent(BKNI_EventHandle event)
{
    BDBG_OBJECT_ASSERT(event, BKNI_Event);
    pb_bare_os->signal_wait(event->signal, 0);
}

BERR_Code
BKNI_WaitForEvent(BKNI_EventHandle event, int timeoutMsec)
{
    int rc;
    BDBG_OBJECT_ASSERT(event, BKNI_Event);

    rc = pb_bare_os->signal_wait(event->signal, timeoutMsec);
    if(rc==0) { return BERR_SUCCESS;}
    else if(rc>0) {return BERR_TIMEOUT;}
    else return BERR_TRACE(BERR_OS_ERROR);
}

void
BKNI_EnterCriticalSection(void)
{
    pb_bare_os->lock_acquire(g_isr_lock);
}

void
BKNI_LeaveCriticalSection(void)
{
    pb_bare_os->lock_release(g_isr_lock);
}

void
BKNI_P_AssertFailed(const char *expr, const char *file, unsigned line)
{
    BKNI_P_Printf(true, "!!! Assert '%s' Failed at %s:%d\n", expr, file, line);
    BKNI_P_Printf(true, "\n");
    BKNI_P_Printf(true, "\n");
    /* oops message will now print. we should be able to see the assert above it. */
    BKNI_Fail();
}


void
BKNI_AssertIsrContext_isr(const char *filename, unsigned lineno)
{
    BSTD_UNUSED(filename);
    BSTD_UNUSED(lineno);
    return;
}

BERR_Code
BKNI_CreateMutex(BKNI_MutexHandle *mutex)
{
    BERR_Code rc;
    BKNI_MutexHandle m;
    m = BKNI_Malloc(sizeof(*m));
    if (m==NULL) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}
    BDBG_OBJECT_INIT(m, BKNI_Mutex);
    m->lock = pb_bare_os->lock_create();
    if(!m->lock) { rc = BERR_TRACE(BERR_OS_ERROR);goto err_lock; }
    *mutex = m;

    return BERR_SUCCESS;
err_lock:
    BKNI_Free(m);
err_alloc:
    return rc;
}

void
BKNI_DestroyMutex(BKNI_MutexHandle mutex)
{
    BDBG_OBJECT_ASSERT(mutex, BKNI_Mutex);
    pb_bare_os->lock_destroy(mutex->lock);
    BDBG_OBJECT_DESTROY(mutex, BKNI_Mutex);
    BKNI_Free(mutex);
}


BERR_Code
BKNI_AcquireMutex(BKNI_MutexHandle mutex)
{
    int rc;
    BDBG_OBJECT_ASSERT(mutex, BKNI_Mutex);
    rc = pb_bare_os->lock_acquire(mutex->lock);
    if(rc!=0)  { return  BERR_TRACE(BERR_OS_ERROR);}
    return BERR_SUCCESS;
}

BERR_Code
BKNI_TryAcquireMutex(BKNI_MutexHandle mutex)
{
    int rc;
    BDBG_OBJECT_ASSERT(mutex, BKNI_Mutex);
    rc = pb_bare_os->lock_try_acquire(mutex->lock);
    if(rc>0) {return BERR_TIMEOUT;}
    if(rc<0) {return BERR_TRACE(BERR_OS_ERROR);}
    return BERR_SUCCESS;
}

void
BKNI_ReleaseMutex(BKNI_MutexHandle mutex)
{
    BDBG_OBJECT_ASSERT(mutex, BKNI_Mutex);
    pb_bare_os->lock_release(mutex->lock);
    return;
}

BERR_Code
BKNI_CreateEventGroup(BKNI_EventGroupHandle *pGroup)
{
    BKNI_EventGroupHandle group;
    BERR_Code rc;

    group = BKNI_Malloc(sizeof(*group));
    if (!group) { rc= BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto err_alloc; }
    BDBG_OBJECT_INIT(group, BKNI_EventGroup);
    BLST_D_INIT(&group->members);
    group->lock = pb_bare_os->lock_create();
    if(!group->lock) {rc=BERR_TRACE(BERR_OS_ERROR); goto err_lock; }
    group->signal = pb_bare_os->signal_create();
    if(!group->signal) {rc=BERR_TRACE(BERR_OS_ERROR); goto err_signal; }
    *pGroup = group;

    return BERR_SUCCESS;

err_signal:
    pb_bare_os->lock_destroy(group->lock);
err_lock:
    BKNI_Free(group);
err_alloc:
    return rc;
}

void
BKNI_DestroyEventGroup(BKNI_EventGroupHandle group)
{
    BKNI_EventHandle event;

    BDBG_OBJECT_ASSERT(group, BKNI_EventGroup);
    pb_bare_os->lock_acquire(group->lock);
    while(NULL != (event=BLST_D_FIRST(&group->members)) ) {
        event->group = NULL;
        BLST_D_REMOVE_HEAD(&group->members, list);
    }
    pb_bare_os->lock_release(group->lock);
    pb_bare_os->lock_destroy(group->lock);
    pb_bare_os->signal_destroy(group->signal);
    BDBG_OBJECT_DESTROY(group, BKNI_EventGroup);
    BKNI_Free(group);
    return;
}

BERR_Code
BKNI_AddEventGroup(BKNI_EventGroupHandle group, BKNI_EventHandle event)
{
    BERR_Code rc=BERR_SUCCESS;
    BDBG_OBJECT_ASSERT(group, BKNI_EventGroup);
    BDBG_OBJECT_ASSERT(event, BKNI_Event);
    pb_bare_os->lock_acquire(group->lock);
    if (event->group == NULL) {
        BLST_D_INSERT_HEAD(&group->members, event, list);
        event->group = group;
        pb_bare_os->signal_set(group->signal);
    } else {
        BDBG_ERR(("Event %#x already connected to the group %#x\n", (unsigned)event, (unsigned)group));
        rc = BERR_TRACE(BERR_OS_ERROR);
    }
    pb_bare_os->lock_release(group->lock);
    return rc;
}

BERR_Code
BKNI_RemoveEventGroup(BKNI_EventGroupHandle group, BKNI_EventHandle event)
{
    BERR_Code rc=BERR_SUCCESS;
    BDBG_OBJECT_ASSERT(group, BKNI_EventGroup);
    BDBG_OBJECT_ASSERT(event, BKNI_Event);
    pb_bare_os->lock_acquire(group->lock);
    if (event->group == group) {
        BLST_D_REMOVE(&group->members, event, list);
        event->group = NULL;
    } else {
        BDBG_ERR(("Event %#x doesn't belong to the group %#x\n", (unsigned)event, (unsigned)group));
        rc = BERR_TRACE(BERR_OS_ERROR);
    }
    pb_bare_os->lock_release(group->lock);
    return rc;
}

static unsigned
group_get_events(BKNI_EventGroupHandle group, BKNI_EventHandle *events, unsigned max_events)
{
    BKNI_EventHandle ev;
    unsigned event;

    pb_bare_os->lock_acquire(group->lock);
    for(event=0, ev=BLST_D_FIRST(&group->members); ev && event<max_events ; ev=BLST_D_NEXT(ev, list)) {
        int rc = pb_bare_os->signal_wait(ev->signal,0);
        if (rc==0) {
            events[event] = ev;
            event++;
        }
    }
    pb_bare_os->lock_release(group->lock);
    return event;
}

BERR_Code
BKNI_WaitForGroup(BKNI_EventGroupHandle group, int timeoutMsec, BKNI_EventHandle *events, unsigned max_events, unsigned *nevents)
{
    unsigned n;
    if (max_events<1) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    n = group_get_events(group, events, max_events);
    if(n==0) {
        int rc = pb_bare_os->signal_wait(group->signal, timeoutMsec);
        if(rc<0) { return BERR_TRACE(BERR_OS_ERROR);}
        n = group_get_events(group, events, max_events);
    }
    *nevents = n;
    return n>0?BERR_SUCCESS:BERR_TIMEOUT;
}

#define B_KERNEL_DCACHE_LINE_SIZE   64
#define BTRC_MEMCPY(x,y)
#define BTRC_MEMSET(x,y)

#define b_prepare_for_store(base) __asm__ __volatile__( \
    ".set push      \n"\
    ".set noreorder \n"\
    "pref 30,(0)(%0);    \n"\
    ".set pop       \n"\
    : \
    : "r" (base))


static void
memset_c_unaligned(uint8_t *d,int c,size_t count)
{
    BTRC_MEMSET(memset_unaligned, START);
    while(count--) {
        *d++ = c;
    }
    BTRC_MEMSET(memset_unaligned, STOP);
}

static void
memset_c_aligned(unsigned *d,uint8_t c,size_t bytes)
{
    uint8_t *d8;
    uint32_t c32;
    BTRC_MEMSET(memset_aligned, START);
    c32 = c;
    c32 = c32  | (c32<<8) | (c32<<16) | (c32<<24);
    BDBG_CASSERT(B_KERNEL_DCACHE_LINE_SIZE/sizeof(unsigned)==16);
    BDBG_CASSERT(sizeof(unsigned)==4);
    if(bytes>=B_KERNEL_DCACHE_LINE_SIZE) {
        /* align destination to the cache line boundary */
        for(;(((unsigned)d)%B_KERNEL_DCACHE_LINE_SIZE)!=0;bytes-=4) {
            *d++ = c32;
        }
        /* fill full cache lines */
        for(;bytes>=B_KERNEL_DCACHE_LINE_SIZE;bytes-=B_KERNEL_DCACHE_LINE_SIZE) {
            b_prepare_for_store(d);
            d[0] = c32;
            d[1] = c32;
            d[2] = c32;
            d[3] = c32;
            d[4] = c32;
            d[5] = c32;
            d[6] = c32;
            d[7] = c32;
            d[8] = c32;
            d[9] = c32;
            d[10] = c32;
            d[11] = c32;
            d[12] = c32;
            d[13] = c32;
            d[14] = c32;
            d[15] = c32;
            d+=16;
        }
        /* fill partial cache line */
        for(;bytes>4;bytes-=4) {
            *d++ = c32;
        }
    }
    /* fill rest */
    d8 = (uint8_t *)d;
    while(bytes--) {
        *d8++ = c;
    }
    BTRC_MEMSET(memset_aligned, STOP);
}
static void *
memset_c(void *dest,int c,size_t count)
{
    if( (((unsigned)dest)&(sizeof(unsigned)-1))==0  ) {
        memset_c_aligned(dest,c,count);
    } else {
        memset_c_unaligned(dest,c,count);
    }
    return dest;
}

static void memcpy_c_unaligned(uint8_t *d,const uint8_t *s,int count)
{
    BTRC_MEMCPY(memcpy_unaligned, START);
    while(count--) {
        *d++ = *s++;
    }
    BTRC_MEMCPY(memcpy_unaligned, STOP);
    return;
}

static void memcpy_c_aligned(unsigned *d,const unsigned *s,size_t bytes)
{
    uint8_t *d8;
    const uint8_t *s8;
    BTRC_MEMCPY(memcpy_aligned, START);
    BDBG_CASSERT(B_KERNEL_DCACHE_LINE_SIZE/sizeof(unsigned)==16);
    if(bytes>=B_KERNEL_DCACHE_LINE_SIZE) {
        /* align destination to the cache line boundary */
        for(;(((unsigned)d)%B_KERNEL_DCACHE_LINE_SIZE)!=0;bytes-=4) {
            *d++ = *s++;
        }
        /* copy full cache lines */
        for(;bytes>=B_KERNEL_DCACHE_LINE_SIZE;bytes-=B_KERNEL_DCACHE_LINE_SIZE) {
            unsigned t0,t1,t2,t3;
            b_prepare_for_store(d);
            t0 = s[0];
            t1 = s[1];
            t2 = s[2];
            t3 = s[3];
            d[0] = t0;
            d[1] = t1;
            d[2] = t2;
            d[3] = t3;

            t0 = s[4];
            t1 = s[5];
            t2 = s[6];
            t3 = s[7];
            d[4] = t0;
            d[5] = t1;
            d[6] = t2;
            d[7] = t3;

            t0 = s[8];
            t1 = s[9];
            t2 = s[10];
            t3 = s[11];
            d[8] = t0;
            d[9] = t1;
            d[10] = t2;
            d[11] = t3;

            t0 = s[12];
            t1 = s[13];
            t2 = s[14];
            t3 = s[15];
            d[12] = t0;
            d[13] = t1;
            d[14] = t2;
            d[15] = t3;

            d+=16;
            s+=16;
        }
    }
    /* copy partial cache line */
    for(;bytes>4;bytes-=4) {
        *d++ = *s++;
    }
    /* copy rest */
    d8 = (uint8_t *)d;
    s8 = (const uint8_t *)s;
    while(bytes--) {
        *d8++ = *s8++;
    }
    BTRC_MEMCPY(memcpy_aligned, STOP);
    return;
}

static void *memcpy_c(void *dest,const void *src,size_t count)
{
    if( (((unsigned)dest | (unsigned)src)&(sizeof(unsigned)-1))==0) {
        memcpy_c_aligned(dest,src,count);
    } else {
        memcpy_c_unaligned(dest,src,count);
    }
    return dest;
}

void *
BKNI_Memset(void *b, int c, size_t len)
{
    return memset_c(b, c, len);
}

void *
BKNI_Memcpy(void *dst, const void *src, size_t len)
{
    return memcpy_c(dst, src, len);
}

static int memcmp_c(const void *s1,const void *s2, size_t count)
{
    const uint8_t *b1= s1;
    const uint8_t *b2= s2;
    for(;count>0;count--) {
        int diff = *b1-*b2;
        if (diff) {
            return diff;
        }
        ++b1;++b2;
    }
    return  0;
}

int
BKNI_Memcmp(const void *b1, const void *b2, size_t len)
{
    return memcmp_c(b1, b2, len);
}

void *
BKNI_Memchr(const void *s, int c, size_t n)
{
    const uint8_t *b= s;
    for(;n>0;n--) {
        if(*b==c) {
            return (void *)b;
        }
        ++b;
    }
    return NULL;
}

void *
BKNI_Memmove(void *s1,const void *s2, size_t n)
{
    uint8_t *b1= s1;
    const uint8_t *b2= s2;

    if((uint8_t *)b1 <= b2) {
        /* forward copy */
        return BKNI_Memcpy(s1,s2, n);
    }
    /* reverse copy */
    b1+=n;
    b2+=n;
    for(;n>0;n--) {
        --b1;--b2;
        *b1=*b2;
    }
    return s1;
}


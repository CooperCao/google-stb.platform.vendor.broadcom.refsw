/***************************************************************************
 *  Copyright (C) 2009-2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 ************************************************************/
#include "nexus_platform_priv.h"
#include "nexus_base_types.h"

#ifndef NEXUS_BASE_OS_bare
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
#include <linux/kconfig.h>
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
#include <generated/autoconf.h>
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#include <linux/autoconf.h>
#else
#define PPROC_ROOT (&proc_root)
#include <linux/config.h>
#endif

#ifndef PPROC_ROOT
#define PPROC_ROOT NULL
#endif

#include "nexus_driver_procfs.h"
#include "bdbg.h"
#endif

BDBG_MODULE(nexus_driver_procfs);

#if defined(CONFIG_PROC_FS)
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
static struct proc_dir_entry *brcm_dir_entry;          /* /proc/brcm */
static struct proc_dir_entry *brcm_config_entry;       /* /proc/brcm/config */
#if BDBG_DEBUG_BUILD
static struct proc_dir_entry *brcm_debug_entry;        /* /proc/brcm/debug */

struct dbgprint_page {
    BLST_Q_ENTRY(dbgprint_page) link;
    char *buf;
    unsigned bufsize;
    unsigned wptr, rptr;
};

typedef struct nexus_driver_proc_data {
    const char *dbg_modules;
    const char *filename;
    void (*callback)(void *context);
    void *context;
    NEXUS_ModuleHandle handle;
} nexus_driver_proc_data;

/* same as in the BDBG_P_LogEntry from magnum/basemodules/dbg/bdbg.c */
#define NEXUS_P_DBGPRINT_LINE (256-sizeof(uint32_t)-sizeof(void *)-sizeof(int16_t))
/* any BDBG print longer than 80 characters may get truncated at the end of a page */
static bool dbgprint_page_done(const struct dbgprint_page *page) {return (page->bufsize - page->wptr) < NEXUS_P_DBGPRINT_LINE;}
static struct {
    struct dbgprint_page first; /* print directly to linux memory; no memcpy */
    BLST_Q_HEAD(dbgprint_page_list_t, dbgprint_page) additional; /* memcpy on subsequent proc reads */
    const nexus_driver_proc_data *current_proc; /* current file */
    BKNI_MutexHandle lock; /* lock to be held while modifying linked list */
} NEXUS_P_ProcFsState;

struct brcm_bdbg_context {
    int bufsize;
    char *modules;
    int modules_len;
};

static const char * const lvl_str[BDBG_P_eLastEntry] = {
        "unk", "trc", "msg", "wrn", "err"
};

#define NEXUS_P_PROC_DATA_MAX   32

static nexus_driver_proc_data g_proc_data[NEXUS_P_PROC_DATA_MAX] = {{NULL, NULL, NULL, NULL, NULL}};


void brcm_bdbg_callback(void *ctx, const char *module, BDBG_Instance instance, const char *inst_name)
{
    struct brcm_bdbg_context *context = (struct brcm_bdbg_context *)ctx;
    int l;

    /* We can't do the lookup here, so simply append all the module names
     * into a buffer for later lookup */
    l = snprintf(context->modules, context->bufsize - context->modules_len, "%s,",module); context->modules_len += l; context->modules += l;
}

static void brcm_bdbg_fetch_state(char *buf, int *len, int bufsize)
{
    struct brcm_bdbg_context ctx;
    char *modules;
    int l = 0;
    char *p;

    *len = 0;
    modules = BKNI_Malloc(bufsize);
    if (!modules) {BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); return;}

    ctx.bufsize = bufsize;
    ctx.modules = modules;
    ctx.modules_len = 0;
    BDBG_EnumerateAll(brcm_bdbg_callback, &ctx);

    ctx.modules_len--;
    modules[ctx.modules_len] = 0;
    p = modules;
    while (p && *p) {
        BDBG_Level lvl;
        char *q = strpbrk(p,",");
        char *m = p;
        p = q;
        if (q) {
            *q = 0;
            p++;
        }
        BDBG_GetModuleLevel(m,&lvl);
        l = snprintf(buf, bufsize - *len, "%s\t%s\n",m,lvl_str[lvl]);
        /*printk("available =%d, write=%d\n",(bufsize - *len),l);*/
        if(l>=(bufsize - *len))
        {
            l= (bufsize - *len);
            *len += l;
            break;
        }
        *len += l; buf += l;
    }
    BKNI_Free(modules);
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
static int brcm_proc_debug_read(char *buf, char **start, off_t off,
                          int bufsize, int *eof, void *unused)
{
    int len = 0;
    if (off > 0) return 0;
    brcm_bdbg_fetch_state(buf, &len, bufsize);
    return len;
}
#else
static ssize_t brcm_proc_debug_read(struct file *fp,char *buf,size_t bufsize, loff_t *offp )
{
    int len;
    static struct {
#define BRCM_PROC_DEBUG_CACHESIZE (3*4096)
        char *buffer;
        int len;
    } cache = {NULL,0};

    if (*offp == 0) {
        if (!cache.buffer) {
            cache.buffer = BKNI_Malloc(BRCM_PROC_DEBUG_CACHESIZE);
            if (!cache.buffer) {BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); return -1;}
        }
        brcm_bdbg_fetch_state(cache.buffer, &cache.len, BRCM_PROC_DEBUG_CACHESIZE);
    }

    len = cache.len - *offp;
    if (len == 0) {
        BKNI_Free(cache.buffer);
        cache.buffer = NULL;
        cache.len = 0;
    }
    else {
        if (len > bufsize) {
            len = bufsize;
        }
        BKNI_Memcpy(buf, &cache.buffer[*offp], len);
        *offp += len;
    }
    return len;
}
#endif

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
static int brcm_proc_debug_write(struct file * file, const char *buffer, unsigned long count,
    void *data)
#else
static ssize_t brcm_proc_debug_write(struct file *file, const char __user *buffer, size_t count,
    loff_t *data)
#endif
{
#define WHITESPACE " \t="
    char *endofmodule, *level;
#define MAX 64
    char name[MAX+1];
    int debug_level;
    int rc;

    if (copy_from_user(name, buffer, count))
    {
        count = -1; /* do not flush buffer, bad copy from user */
        goto end;
    }

    if (count && name[count] == '\n')
        name[count - 1] = 0;
    else
        name[count] = 0;

    endofmodule = strpbrk(name, WHITESPACE);
    if (!endofmodule) {
        printk("No level specified\n");
        goto end;
    }
    *endofmodule++ = 0;
    level = endofmodule + strspn(endofmodule, WHITESPACE);

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,18,0)
#define strncasecmp strnicmp
#endif

    if (!strncasecmp(level, "err", 3))
        debug_level = BDBG_eErr;
    else if (!strncasecmp(level, "wrn", 3))
        debug_level = BDBG_eWrn;
    else if (!strncasecmp(level, "msg", 3))
        debug_level = BDBG_eMsg;
    else if (!strncasecmp(level, "none", 4))
        debug_level = BDBG_eErr;
    else {
        printk("Invalid level. Should be: err, wrn, msg or none.\n");
        goto end;
    }

    rc = BDBG_SetModuleLevel(name, debug_level);

end:
    return count;
}
#endif

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
static int brcm_proc_config_read(char *buf, char **start, off_t off,
                          int bufsize, int *eof, void *unused)
#else
static ssize_t brcm_proc_config_read(struct file *fp,char *buf,size_t bufsize, loff_t *offp )
#endif
{
    /* currently no way to read the full list */
    return 0;
}


#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
static int brcm_proc_config_write(struct file * file, const char *buffer, unsigned long count,
    void *data)
#else
static ssize_t brcm_proc_config_write(struct file *file, const char __user *buffer, size_t count,
    loff_t *data)
#endif
{
    static const char whitespace[] = " \t\n=";
    char *value;
    char name[64+1];

    if(count > sizeof(name)-1)
        count = sizeof(name)-1;

    if (copy_from_user(name, buffer, count))
    {
        count = -1; /* do not flush buffer, bad copy from user */
        goto end;
    }

    name[count]=0;
    if (count && name[count-1] == '\n')
        name[count - 1] = 0;

    value = strpbrk(name, whitespace);
    /* NULL is OK to clear the value */
    if (value) {
        *value++ = 0;

        /* allow "name=" and "name" to clear */
        if (*value == '\n' || *value == 0) value = NULL;
    }

    if (*name) {
        BDBG_MSG(("calling NEXUS_SetEnv(%s,%s)", name, value));
        NEXUS_SetEnv(name, value);
    }

end:
    return count;
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
static struct proc_dir_entry *nexus_p_create_proc(struct proc_dir_entry *parent, const char *name, read_proc_t * read, write_proc_t *write, void *context)
{
    struct proc_dir_entry *entry;
    entry = create_proc_entry(name, S_IFREG|S_IRUGO, parent);
    if (!entry) return NULL;
    entry->read_proc = read;
    entry->write_proc = write;
    entry->data = context;
    return entry;
}
#else /* #if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0) */
#if BDBG_DEBUG_BUILD
static const struct file_operations read_fops_debug = {
read:   brcm_proc_debug_read,
write:  brcm_proc_debug_write
};
#endif
static const struct file_operations read_fops_config = {
read:   brcm_proc_config_read,
write:  brcm_proc_config_write
};

static struct proc_dir_entry *nexus_p_create_proc(struct proc_dir_entry *parent, const char *name, const struct file_operations *fops, void *context)
{
    return proc_create_data(name, S_IFREG|S_IRUGO, parent, fops, context);
}

#endif /* #if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0) */

int nexus_driver_proc_init(void)
{
#if BDBG_DEBUG_BUILD
    BERR_Code rc;

    rc = BKNI_CreateMutex(&NEXUS_P_ProcFsState.lock);
    if(rc!=BERR_SUCCESS) { goto error; }
#endif

    /* Root directory */
    brcm_dir_entry = proc_mkdir("brcm", PPROC_ROOT);
    if (!brcm_dir_entry) goto error;

#if BDBG_DEBUG_BUILD
    brcm_debug_entry = nexus_p_create_proc(brcm_dir_entry, "debug",
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
                        brcm_proc_debug_read, brcm_proc_debug_write,
#else
                        &read_fops_debug,
#endif
                        NULL);
    if (!brcm_debug_entry) goto error;
#endif

    brcm_config_entry = nexus_p_create_proc(brcm_dir_entry, "config",
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
                        brcm_proc_config_read, brcm_proc_config_write,
#else
                        &read_fops_config,
#endif
                        NULL);
    if (!brcm_config_entry) goto error;

    return 0;

error:
    nexus_driver_proc_done();
    return -1;
}


static void nexus_p_free_capture_buffers(void);

void nexus_driver_proc_done(void)
{
    int i;
    if (brcm_config_entry) {
        remove_proc_entry("config", brcm_dir_entry);
        brcm_config_entry = NULL;
    }
#if BDBG_DEBUG_BUILD
    if (brcm_debug_entry) {
        remove_proc_entry("debug", brcm_dir_entry);
        brcm_debug_entry = NULL;
    }
    for (i=0; i < NEXUS_P_PROC_DATA_MAX; i++) {
        if ( g_proc_data[i].filename !=NULL) {
            remove_proc_entry(g_proc_data[i].filename, brcm_dir_entry);
            g_proc_data[i].filename = NULL;
        }
    }
#endif
    if (brcm_dir_entry) {
        remove_proc_entry("brcm", NULL);
        brcm_dir_entry = NULL;
    }
#if BDBG_DEBUG_BUILD
    BKNI_AcquireMutex(NEXUS_P_ProcFsState.lock);
    nexus_p_free_capture_buffers();
    BKNI_ReleaseMutex(NEXUS_P_ProcFsState.lock);
    BKNI_DestroyMutex(NEXUS_P_ProcFsState.lock);
#endif
    return;
}


#if BDBG_DEBUG_BUILD
static void nexus_p_free_capture_buffers(void)
{
    struct dbgprint_page *page;
    while ((page = BLST_Q_FIRST(&NEXUS_P_ProcFsState.additional))) {
        BLST_Q_REMOVE_HEAD(&NEXUS_P_ProcFsState.additional, link);
        BKNI_Free(page->buf);
        BKNI_Free(page);
    }
}

static void
brcm_proc_dbgprint_capture(
        BDBG_ModulePrintKind kind, /* type of the output */
        BDBG_Level level, /* level of the debug output */
        const BDBG_DebugModuleFile *module, /* pointer to the debug module */
        const char *fmt,  /* format */
        va_list ap /* variable list of arguments */
)
{
    int l = 0;
    if ((level != BDBG_eTrace) && (kind != BDBG_ModulePrintKind_eHeader)) {
        struct dbgprint_page *page;
        int left;
        /* find available page or alloc a new one */
        if (!dbgprint_page_done(&NEXUS_P_ProcFsState.first)) {
            page = &NEXUS_P_ProcFsState.first;
        }
        else {
            page = BLST_Q_LAST(&NEXUS_P_ProcFsState.additional);
            if (!page || dbgprint_page_done(page)) {
                /* add another page */
                page = BKNI_Malloc(sizeof(*page));
                if (!page) {
                    BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
                    return;
                }
                BKNI_Memset(page, 0, sizeof(*page));
                page->bufsize = 4096;
                page->buf = BKNI_Malloc(page->bufsize);
                if (!page->buf) {
                    BKNI_Free(page);
                    BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
                    return;
                }
                BLST_Q_INSERT_TAIL(&NEXUS_P_ProcFsState.additional, page, link);
            }
        }
        left = page->bufsize-page->wptr;
        BDBG_ASSERT(left >= NEXUS_P_DBGPRINT_LINE);
        BDBG_CASSERT( NEXUS_P_DBGPRINT_LINE > 1);
        left--;
        l = vsnprintf(&page->buf[page->wptr], left, fmt, ap);
        if((l+1)<=left) { /* output was not truncated */
            page->buf[page->wptr+l] = '\n';
            page->buf[page->wptr+l+1] = '\0';
            page->wptr += l+1;
        } else { /* output was truncated */
            page->buf[page->wptr+(left-1)] = '\n'; /* replace last character with '\n' */
            page->buf[page->wptr+(left)] = '\0';
            page->wptr += left;
        }
    }
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
static int brcm_proc_dbgprint_read(char *buf, char **start, off_t off,
                          int bufsize, int *eof, void *data)
#else
static ssize_t brcm_proc_dbgprint_read(struct file *fp,char *buf,size_t bufsize, loff_t *offp )
#endif
{
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
    nexus_driver_proc_data *p = (nexus_driver_proc_data *)data;
#else
    nexus_driver_proc_data *p = (nexus_driver_proc_data *)PDE_DATA(file_inode(fp));
    off_t off = (loff_t) *offp;
#endif
    unsigned total = 0;
    if (off > 0) {
        /* subsequent read */
        struct dbgprint_page *page;
        if(NEXUS_P_ProcFsState.current_proc != p) {
            (void)BERR_TRACE(NEXUS_NOT_SUPPORTED); /* we support only one procfs node at a time */
            return -EIO;
        }
        BKNI_AcquireMutex(NEXUS_P_ProcFsState.lock);
        while ((page = BLST_Q_FIRST(&NEXUS_P_ProcFsState.additional))) {
            unsigned n = bufsize-total;
            if (!n) break;
            if (n > page->wptr - page->rptr) {
                n = page->wptr - page->rptr;
            }
            BKNI_Memcpy(&buf[total], &page->buf[page->rptr], n);
            page->rptr += n;
            total += n;
            if (page->rptr < page->wptr) break;
            BLST_Q_REMOVE_HEAD(&NEXUS_P_ProcFsState.additional, link);
            BKNI_Free(page->buf);
            BKNI_Free(page);
        }
        #if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
        *start = buf; /* reading fs/proc/generic.c, this is required for multi-page reads */
        if (!BLST_Q_FIRST(&NEXUS_P_ProcFsState.additional)) *eof = 1;
        #else
        *offp += total;
        #endif
        BKNI_ReleaseMutex(NEXUS_P_ProcFsState.lock);
        return total;
    }

    BKNI_AcquireMutex(NEXUS_P_ProcFsState.lock);
    NEXUS_P_ProcFsState.current_proc = p;
    /* first page is the linux-provided buffer */
    NEXUS_P_ProcFsState.first.buf = buf;
    NEXUS_P_ProcFsState.first.bufsize = bufsize;
    NEXUS_P_ProcFsState.first.rptr = 0;
    NEXUS_P_ProcFsState.first.wptr = 0;
    nexus_p_free_capture_buffers();

    /* call function which captures to NEXUS_P_ProcFsState */
    BDBG_SetModulePrintFunction(p->dbg_modules,brcm_proc_dbgprint_capture);
    NEXUS_Module_Lock(p->handle);
    p->callback(p->context);
    NEXUS_Module_Unlock(p->handle);
    BDBG_SetModulePrintFunction(p->dbg_modules,NULL);

    #if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
    *start = buf; /* reading fs/proc/generic.c, this is required for multi-page reads */
    if (!BLST_Q_FIRST(&NEXUS_P_ProcFsState.additional)) *eof = 1;
    #else
    *offp = NEXUS_P_ProcFsState.first.wptr;
    #endif
    total = NEXUS_P_ProcFsState.first.wptr;
    BKNI_ReleaseMutex(NEXUS_P_ProcFsState.lock);
    return total;
}
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
#else
static const struct file_operations read_fops_dbgprint = {
read:   brcm_proc_dbgprint_read
};
#endif
#endif /* #if BDBG_DEBUG_BUILD */

int nexus_driver_proc_register_status(const char *filename, NEXUS_ModuleHandle handle, const char *dbg_module_name, void (*callback)(void *context), void *context)
{
#if BDBG_DEBUG_BUILD
    struct proc_dir_entry *p;
    unsigned i;
    for (i=0; i < NEXUS_P_PROC_DATA_MAX; i++) {
        if (!g_proc_data[i].filename) {
            g_proc_data[i].filename = filename;
            g_proc_data[i].dbg_modules = dbg_module_name;
            g_proc_data[i].callback = callback;
            g_proc_data[i].context = context;
            g_proc_data[i].handle = handle;
            break;
        }
    }
    if(i>=NEXUS_P_PROC_DATA_MAX) {
        BDBG_ERR(("When adding '%s' overflow of the g_proc_data, increase NEXUS_P_PROC_DATA_MAX(%u)", filename, NEXUS_P_PROC_DATA_MAX));
        i = NEXUS_P_PROC_DATA_MAX-1;
    }

    p = nexus_p_create_proc(brcm_dir_entry, filename,
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
            brcm_proc_dbgprint_read, NULL,
#else
            &read_fops_dbgprint,
#endif
            &g_proc_data[i]);
    if (!p) { g_proc_data[i].filename = NULL; return -1; }
#endif
    return 0;
}

void nexus_driver_proc_unregister_status(const char *filename)
{
#if BDBG_DEBUG_BUILD
    int i;
    for (i=0; i < NEXUS_P_PROC_DATA_MAX; i++) {
        if (g_proc_data[i].filename==filename) {
            g_proc_data[i].filename = NULL;
            g_proc_data[i].dbg_modules = NULL;
            g_proc_data[i].callback = NULL;
            g_proc_data[i].context = NULL;
            remove_proc_entry(filename, brcm_dir_entry);
            break;
        }
    }
    if (i >= NEXUS_P_PROC_DATA_MAX) {
        BDBG_LOG(("Trying to remove unknown module '%s'", filename));
    }
#endif
}
#else /* CONFIG_PROC_FS) */
int nexus_driver_proc_init(void)
{
    return 0;
}
void nexus_driver_proc_done(void)
{
}
int nexus_driver_proc_register_status(const char *filename, NEXUS_ModuleHandle handle, const char *dbg_module_name, void (*callback)(void *context), void *context)
{
    BSTD_UNUSED(filename);
    BSTD_UNUSED(handle);
    BSTD_UNUSED(dbg_module_name);
    BSTD_UNUSED(callback);
    BSTD_UNUSED(context);
    BSTD_UNUSED(dbg_buf_size);
    return 0;
}
void nexus_driver_proc_unregister_status(const char *filename)
{
    BSTD_UNUSED(filename);
}
#endif

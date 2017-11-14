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
#include <linux/slab.h>
static struct proc_dir_entry *brcm_dir_entry;          /* /proc/brcm */
static struct proc_dir_entry *brcm_config_entry;       /* /proc/brcm/config */
#if BDBG_DEBUG_BUILD
static struct proc_dir_entry *brcm_debug_entry;        /* /proc/brcm/debug */

typedef struct nexus_driver_proc_data {
    const char *dbg_modules;
    const char *filename;
    void (*callback)(void *context);
    void *context;
    NEXUS_ModuleHandle handle;
} nexus_driver_proc_data;

struct nexus_driver_seq;
static struct {
    struct nexus_driver_seq *seq;
    BKNI_MutexHandle lock; /* lock to be held while capturing debug data */
    BKNI_MutexHandle procfs_lock; /* lock to be held when adding/removing procfs entries */
} NEXUS_P_ProcFsState;
static void nexus_driver_seq_capture_locked(BDBG_ModulePrintKind kind, BDBG_Level level, const BDBG_DebugModuleFile *module, const char *fmt, va_list ap);

struct brcm_bdbg_context {
    int bufsize;
    char *modules;
    int modules_len;
};

static const char * const lvl_str[BDBG_P_eLastEntry] = {
        "unk", "trc", "msg", "wrn", "err"
};

#define NEXUS_P_PROC_DATA_MAX   48

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

static ssize_t brcm_proc_debug_write(struct file *file, const char __user *buffer, size_t count,
    loff_t *data)
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

static ssize_t brcm_proc_config_read(struct file *fp,char __user *buf,size_t bufsize, loff_t *offp )
{
    /* currently no way to read the full list */
    return 0;
}


static ssize_t brcm_proc_config_write(struct file *file, const char __user *buffer, size_t count,
    loff_t *data)
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

#if BDBG_DEBUG_BUILD
#include "linux/seq_file.h"
#include "blst_squeue.h"

#define dbg_printk(...) /* printk(__VA_ARGS__) */

struct nexus_driver_seq_buf {
    BLST_SQ_ENTRY(nexus_driver_seq_buf) link;
    unsigned len; /* number of bytes (without trailing 0) used in buffer */
    char buf[PAGE_SIZE - sizeof(unsigned) - sizeof(void *)];
};

struct nexus_driver_seq {
    BLST_SQ_HEAD(nexus_driver_seq_buf_list, nexus_driver_seq_buf) buffers;
    bool out_of_memory;
};

static void *nexus_driver_seq_start(struct seq_file *s, loff_t *pos)
{
    struct nexus_driver_seq *seq = s->private;
    struct nexus_driver_seq_buf *buf;
    loff_t i;

    BDBG_ASSERT(seq);
    for(buf=BLST_SQ_FIRST(&seq->buffers),i=0;buf && i<*pos;i++) {
        buf = BLST_SQ_NEXT(buf, link);
    }
    if(buf) {
        dbg_printk("start: %u -> %p(%u)\n", (unsigned)*pos, buf, (buf->len));
    } else {
        dbg_printk("start: %u -> EOS\n", (unsigned)*pos);
    }
    return buf;
}

static void *nexus_driver_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
    struct nexus_driver_seq_buf *buf=v;
    BDBG_ASSERT(buf);
    (*pos) += 1;
    buf=BLST_SQ_NEXT(buf,link);
    if(buf) {
        dbg_printk("next: %u -> %p(%u)\n", (unsigned)*pos, buf, (buf->len));
    } else {
        dbg_printk("next: %u -> EOS\n", (unsigned)*pos);
    }
    return buf;
}

static void nexus_driver_seq_stop(struct seq_file *s, void *v)
{
    return;
}

static int nexus_driver_seq_show(struct seq_file *s, void *v)
{
    struct nexus_driver_seq_buf *buf=v;
    BDBG_ASSERT(buf);
    dbg_printk("show: %p(%u)\n", buf, (buf->len));
    /* dbg_printk("''%s''", buf->buf); */
    seq_write(s, buf->buf, buf->len);
    return 0;
}


static struct seq_operations nexux_driver_seq_ops = {
start: nexus_driver_seq_start,
next:  nexus_driver_seq_next,
stop:  nexus_driver_seq_stop,
show:  nexus_driver_seq_show
};

static struct nexus_driver_seq_buf *nexus_driver_seq_add_buffer_locked(struct nexus_driver_seq *seq)
{
    struct nexus_driver_seq_buf *buf;

    BDBG_CWARNING(sizeof(*buf)==PAGE_SIZE);
    buf = kmalloc(sizeof(*buf), GFP_KERNEL);
    if(buf==NULL) {
        seq->out_of_memory = true;
        (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);return NULL;
    }
    buf->len =0;
    buf->buf[0] = '\0';
    BLST_SQ_INSERT_TAIL(&seq->buffers, buf, link);
    return buf;
}

static void nexus_driver_seq_free_buffers_locked(struct nexus_driver_seq *seq)
{
    struct nexus_driver_seq_buf *buf;

    while (NULL != (buf = BLST_SQ_FIRST(&seq->buffers))) {
        BLST_SQ_REMOVE_HEAD(&seq->buffers, link);
        kfree(buf);
    }
    return;
}

static bool
nexus_driver_seq_capture_to_buf_locked(struct nexus_driver_seq_buf *buf, const char *fmt, va_list ap)
{
    int rc;
    size_t left;

    dbg_printk(">capture: %p(%u)\n", buf, (buf->len));
    if(buf->len >= sizeof(buf->buf)-2) { /* no space for \n and \0 */
        return false;
    }
    left = (sizeof(buf->buf) - 2) - buf->len;
    rc = vsnprintf(&buf->buf[buf->len], left, fmt, ap);
    if(rc<0) {
        buf->buf[buf->len]='\0';
        return true; /* there is nothing to do if formatting have failed */
    }
    if(rc>left) {
        if(buf->len!=0) { /* it was not full buffer, allocate full buffer and try again */
            buf->buf[buf->len]='\0';
            return false;
        }
        rc = left-1;
    }
    /* dbg_printk("<capture:>> ''%s''\n", buf->buf+buf->len); */
    buf->buf[buf->len+rc] = '\n';
    buf->buf[buf->len+rc+1] = '\0';
    /* dbg_printk("<capture:  ''%s''\n", buf->buf); */
    dbg_printk("<capture: %p(%u->%u)\n", buf, buf->len, buf->len+rc+1);
    buf->len += rc + 1;
    return true;
}

static void
nexus_driver_seq_capture_locked(BDBG_ModulePrintKind kind, BDBG_Level level, const BDBG_DebugModuleFile *module, const char *fmt, va_list ap)
{
    struct nexus_driver_seq *seq;
    struct nexus_driver_seq_buf *buf;

    if ((level == BDBG_eTrace) || (kind == BDBG_ModulePrintKind_eHeader)) {
        return;
    }
    seq = NEXUS_P_ProcFsState.seq;
    if(seq==NULL) {
        return; /* something that was captured out of sequence, for example from different thread */
    }
    if(seq->out_of_memory) {
        return; /* if already run out of memory don't try tool allocate some more, just return ASAP */
    }
    buf = BLST_SQ_LAST(&seq->buffers);

    BDBG_ASSERT(buf);
    if(!nexus_driver_seq_capture_to_buf_locked(buf, fmt, ap)) {
        buf = nexus_driver_seq_add_buffer_locked(seq);
        if(buf) {
            if(!nexus_driver_seq_capture_to_buf_locked(buf, fmt, ap)) {
                BDBG_ASSERT(0); /* second try must succeed */
            }
        }
    }
    return;
}

static int nexus_driver_seq_open_locked(const nexus_driver_proc_data *p, struct file *file)
{
    int rc;
    struct nexus_driver_seq *seq;
    struct nexus_driver_seq_buf *buf;
    struct seq_file *s;

    seq = kmalloc(sizeof(*seq), GFP_KERNEL);
    if(seq==NULL) {
        (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);rc = -ENOMEM;
        goto err_kmalloc;
    }
    BLST_SQ_INIT(&seq->buffers);
    seq->out_of_memory = false;

    buf = nexus_driver_seq_add_buffer_locked(seq);
    if(buf==NULL) {
        (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);rc = -ENOMEM;
        goto err_add_buffer;
    }

    if (NEXUS_Platform_P_ModuleInStandby(p->handle)) {
        buf->len = BKNI_Snprintf(buf->buf, sizeof(buf->buf), "%s in standby\n", NEXUS_Module_GetName(p->handle));
    }
    else {
        /* call function which captures to nexus_driver_seq */
        NEXUS_Module_Lock(p->handle);
        NEXUS_P_ProcFsState.seq = seq;
        BDBG_SetModulePrintFunction(p->dbg_modules,nexus_driver_seq_capture_locked);
        p->callback(p->context);
        BDBG_SetModulePrintFunction(p->dbg_modules,NULL);
        NEXUS_P_ProcFsState.seq = NULL;
        NEXUS_Module_Unlock(p->handle);
        if(seq->out_of_memory) {
            (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);rc = -ENOMEM;
            goto err_capture;
        }
    }

    rc = seq_open(file, &nexux_driver_seq_ops);
    if(rc!=0) {
        (void)BERR_TRACE(NEXUS_OS_ERROR);
        goto err_seq_open;
    }
    s = file->private_data;
    s->private = seq;
    return rc;

err_seq_open:
err_capture:
    nexus_driver_seq_free_buffers_locked(seq);
err_add_buffer:
    kfree(seq);
err_kmalloc:
    return rc;

}

static int nexus_driver_seq_open(struct inode *inode, struct file *file)
{
    int rc;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,9,0)
    const nexus_driver_proc_data *p = PDE(inode)->data;
#else
    const nexus_driver_proc_data *p = PDE_DATA(inode);
#endif
    BDBG_ASSERT(p);

    BKNI_AcquireMutex(NEXUS_P_ProcFsState.lock);
    rc = nexus_driver_seq_open_locked(p, file);
    BKNI_ReleaseMutex(NEXUS_P_ProcFsState.lock);
    return rc;
}

static int nexus_driver_seq_release(struct inode *inode, struct file *file)
{
    struct seq_file *s = file->private_data;
    struct nexus_driver_seq *seq;

    BDBG_ASSERT(s);
    seq = s->private;
    BDBG_ASSERT(seq);

    BKNI_AcquireMutex(NEXUS_P_ProcFsState.lock);
    nexus_driver_seq_free_buffers_locked(seq);
    BKNI_ReleaseMutex(NEXUS_P_ProcFsState.lock);
    kfree(seq);

    return seq_release(inode, file);
}


static const struct file_operations read_fops_seq_dbgprint = {
open:    nexus_driver_seq_open,
read:    seq_read,
llseek:  seq_lseek,
release: nexus_driver_seq_release,
};

#define BRCM_PROC_DEBUG_CACHESIZE (3*4096)
struct nexus_driver_seq_debug {
    int len;
    char buffer[BRCM_PROC_DEBUG_CACHESIZE-sizeof(int)];
};

static int nexus_driver_seq_debug_show(struct seq_file *s, void *v)
{
    struct nexus_driver_seq_debug *seq = s->private;
    BDBG_ASSERT(seq);
    seq_write(s, seq->buffer, seq->len);
    return 0;
}

static int nexus_driver_seq_debug_open(struct inode *inode, struct file *file)
{
    struct nexus_driver_seq_debug *seq;
    int rc;

    seq = kmalloc(sizeof(*seq), GFP_KERNEL);
    if(seq==NULL) {
        (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);rc = -ENOMEM;
        goto err_kmalloc;
    }
    seq->len = 0;
    brcm_bdbg_fetch_state(seq->buffer, &seq->len, sizeof(seq->buffer));
#define BRCM_PROC_DEBUG_CACHESIZE (3*4096)
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,10,0)
    rc = single_open(file, nexus_driver_seq_debug_show, seq);
#else
    rc = single_open_size(file, nexus_driver_seq_debug_show, seq, BRCM_PROC_DEBUG_CACHESIZE);
#endif
    if(rc!=0) { goto err_seq;}

    return rc;
err_seq:
    kfree(seq);
err_kmalloc:
    return rc;
}

static int nexus_driver_seq_debug_release(struct inode *inode, struct file *file)
{
    struct seq_file *s = file->private_data;
    struct nexus_driver_seq_debug *seq;
    BDBG_ASSERT(s);
    seq = s->private;
    BDBG_ASSERT(seq);
    kfree(seq);
    return single_release(inode, file);
}

static const struct file_operations read_fops_debug = {
release: nexus_driver_seq_debug_release,
open:    nexus_driver_seq_debug_open,
read:    seq_read,
llseek:  seq_lseek,
write:  brcm_proc_debug_write
};
#endif /* #if BDBG_DEBUG_BUILD */

static const struct file_operations read_fops_config = {
read:   brcm_proc_config_read,
write:  brcm_proc_config_write
};

static struct proc_dir_entry *nexus_p_create_proc(struct proc_dir_entry *parent, const char *name, const struct file_operations *fops, void *context)
{
    return proc_create_data(name, S_IFREG|S_IRUGO, parent, fops, context);
}

int nexus_driver_proc_init(void)
{
#if BDBG_DEBUG_BUILD
    BERR_Code rc;

    rc = BKNI_CreateMutex(&NEXUS_P_ProcFsState.lock);
    if(rc!=BERR_SUCCESS) { goto err_lock; }
    rc = BKNI_CreateMutex(&NEXUS_P_ProcFsState.procfs_lock);
    if(rc!=BERR_SUCCESS) { goto err_procfs_lock; }
#endif

    /* Root directory */
    brcm_dir_entry = proc_mkdir("brcm", PPROC_ROOT);
    if (!brcm_dir_entry) goto error;

#if BDBG_DEBUG_BUILD
    brcm_debug_entry = nexus_p_create_proc(brcm_dir_entry, "debug",
                        &read_fops_debug,
                        NULL);
    if (!brcm_debug_entry) goto error;
#endif

    brcm_config_entry = nexus_p_create_proc(brcm_dir_entry, "config",
                        &read_fops_config,
                        NULL);
    if (!brcm_config_entry) goto error;

    return 0;

error:
    nexus_driver_proc_done();
    return -1;

#if BDBG_DEBUG_BUILD
err_procfs_lock:
    BKNI_DestroyMutex(NEXUS_P_ProcFsState.lock);
err_lock:
    return -1;
#endif
}

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
    BKNI_AcquireMutex(NEXUS_P_ProcFsState.procfs_lock);
    for (i=0; i < NEXUS_P_PROC_DATA_MAX; i++) {
        if ( g_proc_data[i].filename !=NULL) {
            remove_proc_entry(g_proc_data[i].filename, brcm_dir_entry);
            g_proc_data[i].filename = NULL;
        }
    }
    BKNI_ReleaseMutex(NEXUS_P_ProcFsState.procfs_lock);
#endif
    if (brcm_dir_entry) {
        remove_proc_entry("brcm", NULL);
        brcm_dir_entry = NULL;
    }
#if BDBG_DEBUG_BUILD
    BKNI_DestroyMutex(NEXUS_P_ProcFsState.procfs_lock);
    BKNI_DestroyMutex(NEXUS_P_ProcFsState.lock);
#endif
    return;
}


#if BDBG_DEBUG_BUILD
void nexus_driver_proc_print(BDBG_ModulePrintKind kind, const char *fmt, va_list ap)
{
    nexus_driver_seq_capture_locked(kind, BDBG_eLog, NULL, fmt, ap);
}
#endif /* #if BDBG_DEBUG_BUILD */

int nexus_driver_proc_register_status(const char *filename, NEXUS_ModuleHandle handle, const char *dbg_module_name, void (*callback)(void *context), void *context)
{
#if BDBG_DEBUG_BUILD
    struct proc_dir_entry *p;
    unsigned i;
    BDBG_ASSERT(filename);
    BKNI_AcquireMutex(NEXUS_P_ProcFsState.procfs_lock);
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
    BKNI_ReleaseMutex(NEXUS_P_ProcFsState.procfs_lock);
    if(i>=NEXUS_P_PROC_DATA_MAX) {
        BDBG_ERR(("When adding '%s' overflow of the g_proc_data, increase NEXUS_P_PROC_DATA_MAX(%u)", filename, NEXUS_P_PROC_DATA_MAX));
        i = NEXUS_P_PROC_DATA_MAX-1;
    }

    p = nexus_p_create_proc(brcm_dir_entry, filename,
            &read_fops_seq_dbgprint,
            &g_proc_data[i]);
    if (!p) { g_proc_data[i].filename = NULL; return -1; }
#endif
    return 0;
}

void nexus_driver_proc_unregister_status(const char *filename)
{
#if BDBG_DEBUG_BUILD
    int i;
    BKNI_AcquireMutex(NEXUS_P_ProcFsState.procfs_lock);
    for (i=0; i < NEXUS_P_PROC_DATA_MAX; i++) {
        if (g_proc_data[i].filename==filename) {
            remove_proc_entry(filename, brcm_dir_entry);
            g_proc_data[i].filename = NULL;
            g_proc_data[i].dbg_modules = NULL;
            g_proc_data[i].callback = NULL;
            g_proc_data[i].context = NULL;
            break;
        }
    }
    BKNI_ReleaseMutex(NEXUS_P_ProcFsState.procfs_lock);
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

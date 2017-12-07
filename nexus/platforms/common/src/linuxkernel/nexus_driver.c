/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.

 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ******************************************************************************/
#include "nexus_platform_priv.h"
#include <linux/version.h>
#include "nexus_base_ioctl.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
#include <linux/kconfig.h>
#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/signal.h>
#include <linux/kernel.h>
#ifdef NEXUS_HAS_SOCKET_DRIVER
#include <linux/poll.h>
#include "nexus_platform_socket_ioctl.h"
#endif
#include "bchp_common.h"

#if NEXUS_USE_CMA
    /* no #include */
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,1)
    /* ARM bmem */
    #include <linux/brcmstb/bmem.h>
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
    #include <linux/brcmstb/brcmapi.h> /* bmem_region_info */
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    #include <asm/brcmstb/brcmapi.h> /* bmem_region_info */
#else
#undef BCHP_PHYSICAL_OFFSET
    #include <asm/brcmstb/common/brcmstb.h>
#undef BCHP_PHYSICAL_OFFSET

    /* These are included in the 2.6.12-4.0 kernel and later. */
    #if HAVE_RSRVD
    extern unsigned long get_RAM_size();
    extern unsigned long get_RSVD_size();
    #endif
#endif

#include "priv/nexus_generic_driver.h"

#if NEXUS_CPU_ARM
#include "bcmdriver_arm.c"
#endif

static int      nexus_driver_open(struct inode *inode, struct file *file);
static int      nexus_driver_close(struct inode *inode, struct file * file);
static long nexus_driver_ioctl(struct file * file, unsigned int cmd, unsigned long arg, bool compat);
static int      nexus_driver_mmap(struct file *file, struct vm_area_struct *vma);

static long
nexus_driver_ioctl_native(struct file * file, unsigned int cmd, unsigned long arg)
{
    return nexus_driver_ioctl(file, cmd, arg, false);
}

#ifdef CONFIG_COMPAT
static long
nexus_driver_ioctl_compat(struct file * file, unsigned int cmd, unsigned long arg)
{
#if NEXUS_COMPAT_32ABI
    return nexus_driver_ioctl(file, cmd, arg, true);
#else
    /* printk("32-bit ABI is not supported\n"); */
    return -EINVAL;
#endif
}
#endif

#ifdef NEXUS_HAS_SOCKET_DRIVER
BDBG_MODULE(nexus_driver);
#include "blst_slist.h"

#define BDBG_MSG_TRACE(x) BDBG_MSG(x)

struct nexus_driver_socket_buffer {
    unsigned len;
    char buf[8000];
};

BDBG_OBJECT_ID(nexus_driver_socket_connect);

struct nexus_driver_socket_connect {
    BDBG_OBJECT(nexus_driver_socket_connect)
    BLST_S_ENTRY(nexus_driver_socket_connect) link;
    struct nexus_driver_socket_accept *server; /* link to server connection. If NULL and server_closed==false, that this 'socket' is in the connect_list */
    bool server_closed;
    unsigned pid;
    wait_queue_head_t wq; /* for sake of simplicity the same wait queue for read and write operations */
    struct nexus_driver_socket_buffer buffer;
};

BDBG_OBJECT_ID(nexus_driver_socket_listen);
struct nexus_driver_socket_listen {
    BDBG_OBJECT(nexus_driver_socket_listen)
    BLST_S_HEAD(nexus_driver_socket_connect_list,  nexus_driver_socket_connect) connect_list; /* list of clients that have called 'connect' */
    struct nexus_generic_driver_state *generic; /* link back to the generic state */
    wait_queue_head_t wq;
};

BDBG_OBJECT_ID(nexus_driver_socket_accept);
struct nexus_driver_socket_accept {
    BDBG_OBJECT(nexus_driver_socket_accept)
    struct nexus_driver_socket_connect *client; /* pointer to client */
    wait_queue_head_t wq; /* for sake of simplicity the same wait queue for read and write operations */
    struct nexus_driver_socket_buffer buffer;
};

static struct nexus_driver_socket_listen *listen_socket = NULL; /* there is could be only single 'socket' that listens */
static DEFINE_MUTEX(socket_lock);

static void nexus_driver_socket_buffer_init(struct nexus_driver_socket_buffer *buffer)
{
    buffer->len = 0;
    return;
}

static bool nexus_driver_socket_buffer_can_read(const struct nexus_driver_socket_buffer *buffer)
{
    return buffer->len > 0;
}

static bool nexus_driver_socket_buffer_can_write(const struct nexus_driver_socket_buffer *buffer)
{
    return buffer->len < sizeof(buffer->buf);
}

static int nexus_driver_socket_buffer_read(struct nexus_driver_socket_buffer *buffer, char *data, size_t original_size)
{
    size_t size = original_size;

    if(size > buffer->len) {
        size = buffer->len;
    }
    if(copy_to_user(data, buffer->buf, size)!=0) {
        return -EFAULT;
    }
    BDBG_MSG_TRACE(("read:%p -> %u(%u) len:(%u->%u)", (void *)buffer, (unsigned)size, (unsigned)original_size, buffer->len, (unsigned)(buffer->len-size)));
    buffer->len -= size;
    if(buffer->len>0) {
        memmove(buffer->buf, buffer->buf + size,  buffer->len);
    }
    return (int)size;
}

static int nexus_driver_socket_buffer_write(struct nexus_driver_socket_buffer *buffer, const char *data, size_t original_size)
{
    unsigned left = sizeof(buffer->buf) - buffer->len;
    size_t size = original_size;

    if(size > left) {
        size = left;
    }
    if(copy_from_user(buffer->buf + buffer->len, data, size)!=0) {
        return -EFAULT;
    }
    BDBG_MSG_TRACE(("write:%p -> %u(%u) len:(%u->%u)", (void *)buffer, (unsigned)size, (unsigned)original_size, buffer->len, (unsigned)(buffer->len+size)));
    buffer->len += size;
    return (int)size;
}

static void nexus_driver_socket_wakeup_listener(struct nexus_driver_socket_listen *listen)
{
    BDBG_OBJECT_ASSERT(listen, nexus_driver_socket_listen);
    wake_up(&listen->wq);
    return;
}

static void nexus_driver_socket_wakeup_client(struct nexus_driver_socket_connect *connect)
{
    BDBG_OBJECT_ASSERT(connect, nexus_driver_socket_connect);
    wake_up(&connect->wq);
    return;
}

static void nexus_driver_socket_wakeup_server(struct nexus_driver_socket_accept *accept)
{
    BDBG_OBJECT_ASSERT(accept, nexus_driver_socket_accept);
    wake_up(&accept->wq);
    return;
}

static bool nexus_driver_socket_connect_read_condition_safe(struct nexus_driver_socket_connect *s)
{
    bool will_block = true;
    if(s->server_closed) {
        will_block = false;
    } else {
         if(nexus_driver_socket_buffer_can_read(&s->buffer)) {
             will_block = false;
         }
    }
    return !will_block;
}

static bool nexus_driver_socket_accept_read_condition_safe(struct nexus_driver_socket_accept *s)
{
    bool will_block = true;
    if(s->client==NULL) {
        will_block = false;
    } else {
         if(nexus_driver_socket_buffer_can_read(&s->buffer)) {
             will_block = false;
         }
    }
    return !will_block;
}

static ssize_t
nexus_driver_read(struct file * file, char *data, size_t size, loff_t *offset)
{
    struct nexus_generic_driver_state *state = file->private_data;
    int rc = 0;
    BDBG_MSG_TRACE((">>read %p(%u)", (void *)state, (unsigned) size));
    mutex_lock(&socket_lock);
    switch(state->kind) {
    case nexus_generic_driver_kind_connect:
        {
            struct nexus_driver_socket_connect *s = state->state.connect;
            BDBG_OBJECT_ASSERT(s, nexus_driver_socket_connect);
            BDBG_MSG_TRACE((">>read connect %p(%u) %u server:%p", (void *)s, (unsigned) size, s->buffer.len, (void *)s->server));
            for(;;) {
                if(nexus_driver_socket_buffer_can_read(&s->buffer)) {
                    rc = nexus_driver_socket_buffer_read(&s->buffer, data, size);
                    if(s->server) {
                        nexus_driver_socket_wakeup_server(s->server);
                    }
                    break;
                }
                if(s->server_closed) {
                    BDBG_MSG_TRACE(("server closed"));
                    rc = 0; /* server got closed, end of file */
                    break;
                }
                if(file->f_flags & O_NONBLOCK) {
                    rc = -EAGAIN;
                    break;
                }
                mutex_unlock(&socket_lock);
                rc = wait_event_interruptible(s->wq, nexus_driver_socket_connect_read_condition_safe(s));
                mutex_lock(&socket_lock);
                if(rc!=0) {
                    break;
                }
            }
            BDBG_MSG_TRACE((">>read connect %p(%u) %u server:%p -> %d", (void *)s, (unsigned) size, s->buffer.len, (void *)s->server, rc));
        }
        break;
    case nexus_generic_driver_kind_accept:
        {
            struct nexus_driver_socket_accept *s = state->state.accept;
            BDBG_OBJECT_ASSERT(s, nexus_driver_socket_accept);
            BDBG_MSG_TRACE((">>read accept %p(%u) %u client:%p", (void *)s, (unsigned) size, s->buffer.len, (void *)s->client));
            for(;;) {
                if(nexus_driver_socket_buffer_can_read(&s->buffer)) {
                    rc = nexus_driver_socket_buffer_read(&s->buffer, data, size);
                    if(s->client) {
                        nexus_driver_socket_wakeup_client(s->client);
                    }
                    break;
                }
                if(s->client==NULL) {
                    rc = 0; /* server got closed, end of file */
                    break;
                }
                if(file->f_flags & O_NONBLOCK) {
                    rc = -EAGAIN;
                    break;
                }
                mutex_unlock(&socket_lock);
                rc = wait_event_interruptible(s->wq, nexus_driver_socket_accept_read_condition_safe(s));
                mutex_lock(&socket_lock);
                if(rc!=0) {
                    break;
                }
            }
            BDBG_MSG_TRACE((">>read accept %p(%u) %u client:%p -> %d", (void *)s, (unsigned) size, s->buffer.len, s->client, rc));
        }
        break;
    default:
        rc = -ENOSYS;
        break;
    }
    mutex_unlock(&socket_lock);
    BDBG_MSG_TRACE(("<<read %p(%u) -> %d", (void *)state, (unsigned) size, rc));
    BSTD_UNUSED(offset);
    return rc;
}

static bool nexus_driver_socket_connect_write_condition_locked(struct nexus_driver_socket_connect *s)
{
    bool will_block = true;
    if(s->server_closed) {
        will_block = false;
    } else {
         struct nexus_driver_socket_accept *server = s->server;
         if(server) {
             BDBG_OBJECT_ASSERT(server, nexus_driver_socket_accept);
             if(nexus_driver_socket_buffer_can_write(&server->buffer)) {
                will_block = false;
             }
         }
    }
    return !will_block;
}

static bool nexus_driver_socket_connect_write_condition_unlocked(struct nexus_driver_socket_connect *s)
{
    bool result;
    mutex_lock(&socket_lock);
    result = nexus_driver_socket_connect_write_condition_locked(s);
    mutex_unlock(&socket_lock);
    return result;
}

static bool nexus_driver_socket_accept_write_condition_locked(struct nexus_driver_socket_accept *s)
{
    bool will_block = true;
    struct nexus_driver_socket_connect *client = s->client;
    if(client) {
         BDBG_OBJECT_ASSERT(client, nexus_driver_socket_connect);
         if(nexus_driver_socket_buffer_can_write(&client->buffer)) {
             will_block = false;
         }
    } else {
        will_block = false;
    }
    return !will_block;
}

static bool nexus_driver_socket_accept_write_condition_unlocked(struct nexus_driver_socket_accept *s)
{
    bool result;
    mutex_lock(&socket_lock);
    result = nexus_driver_socket_accept_write_condition_locked(s);
    mutex_unlock(&socket_lock);
    return result;
}

static ssize_t
nexus_driver_write(struct file * file, const char *data, size_t size, loff_t *offset)
{
    struct nexus_generic_driver_state *state = file->private_data;
    int rc = 0;
    BDBG_MSG_TRACE((">>write %p(%u)", (void *)state, (unsigned) size));
    mutex_lock(&socket_lock);
    switch(state->kind) {
    case nexus_generic_driver_kind_connect:
        {
            struct nexus_driver_socket_connect *s = state->state.connect;
            BDBG_OBJECT_ASSERT(s, nexus_driver_socket_connect);
            BDBG_MSG_TRACE((">>write connect %p(%u) => %p ", (void *)s, (unsigned) size, (void *)s->server));
            for(;;) {
                struct nexus_driver_socket_accept *server = s->server;
                if(server) {
                    BDBG_OBJECT_ASSERT(server, nexus_driver_socket_accept);
                    if(nexus_driver_socket_buffer_can_write(&server->buffer)) {
                        rc = nexus_driver_socket_buffer_write(&server->buffer, data, size);
                        nexus_driver_socket_wakeup_server(server);
                        break;
                    }
                }
                if(s->server_closed) {
                    rc = -EPIPE; /* server got closed, can't write */
                    break;
                }
                if(file->f_flags & O_NONBLOCK) {
                    rc = -EAGAIN;
                    break;
                }
                mutex_unlock(&socket_lock);
                rc = wait_event_interruptible(s->wq, nexus_driver_socket_connect_write_condition_unlocked(s));
                mutex_lock(&socket_lock);
                if(rc!=0) {
                    break;
                }
            }
            BDBG_MSG_TRACE((">>write connect %p(%u) => %p -> %d ", (void *)s, (unsigned) size, (void *)s->server, rc));
        }
        break;
    case nexus_generic_driver_kind_accept:
        {
            struct nexus_driver_socket_accept *s = state->state.accept;
            BDBG_OBJECT_ASSERT(s, nexus_driver_socket_accept);
            BDBG_MSG_TRACE((">>write accept %p(%u) => %p", (void *)s, (unsigned) size, (void *)s->client));
            for(;;) {
                struct nexus_driver_socket_connect *client = s->client;
                if(client) {
                    BDBG_OBJECT_ASSERT(client, nexus_driver_socket_connect);
                    if(nexus_driver_socket_buffer_can_write(&client->buffer)) {
                        rc = nexus_driver_socket_buffer_write(&client->buffer, data, size);
                        nexus_driver_socket_wakeup_client(client);
                        break;
                    }
                } else {
                    rc = -EPIPE; /* client got closed, can't write */
                    break;
                }
                if(file->f_flags & O_NONBLOCK) {
                    rc = -EAGAIN;
                    break;
                }
                mutex_unlock(&socket_lock);
                rc = wait_event_interruptible(s->wq, nexus_driver_socket_accept_write_condition_unlocked(s));
                mutex_lock(&socket_lock);
                if(rc!=0) {
                    break;
                }
            }
            BDBG_MSG_TRACE((">>write accept %p(%u) => %p -> %d", (void *)s, (unsigned) size, (void *)s->client, rc));
        }
        break;
    default:
        rc = -ENOSYS;
        break;
    }
    mutex_unlock(&socket_lock);
    BDBG_MSG_TRACE(("<<write %p(%u) -> %d", (void *)state, (unsigned) size, rc));
    BSTD_UNUSED(offset);
    return rc;
}

static unsigned
nexus_driver_poll(struct file * file, struct poll_table_struct * poll)
{
    unsigned long events = 0;
    struct nexus_generic_driver_state *state = file->private_data;

    BDBG_MSG_TRACE((">>poll %p", (void *)state));
    mutex_lock(&socket_lock);
    switch(state->kind) {
    case nexus_generic_driver_kind_connect:
        {
            struct nexus_driver_socket_connect *s = state->state.connect;
            BDBG_MSG_TRACE((">>poll connect %p", (void *)s));
            BDBG_OBJECT_ASSERT(s, nexus_driver_socket_connect);
            poll_wait(file, &s->wq, poll);
            if(nexus_driver_socket_connect_write_condition_locked(s)) {
                events |= POLLOUT | POLLWRNORM;
            }
            if(nexus_driver_socket_connect_read_condition_safe(s)) {
                events |= POLLIN | POLLRDNORM;
            }
            break;
        }
    case nexus_generic_driver_kind_accept:
        {
            struct nexus_driver_socket_accept *s = state->state.accept;
            BDBG_MSG_TRACE((">>poll accept %p", (void *)s));
            BDBG_OBJECT_ASSERT(s, nexus_driver_socket_accept);
            poll_wait(file, &s->wq, poll);
            if(nexus_driver_socket_accept_write_condition_locked(s)) {
                events |= POLLOUT | POLLWRNORM;
            }
            if(nexus_driver_socket_accept_read_condition_safe(s)) {
                events |= POLLIN | POLLRDNORM;
            }
            break;
        }
    case nexus_generic_driver_kind_listen:
        {
            struct nexus_driver_socket_listen *s = state->state.listen;
            BDBG_MSG_TRACE((">>poll listen %p", (void *)s));
            BDBG_OBJECT_ASSERT(s, nexus_driver_socket_listen);
            poll_wait(file, &s->wq, poll);
            if(BLST_S_FIRST(&s->connect_list)) {
                events |= POLLIN | POLLRDNORM;
            }
            break;
        }
    default:
        break;
    }
    mutex_unlock(&socket_lock);
    BDBG_MSG_TRACE((">>poll %p(%s%s)", (void *)state, (events & POLLIN) ? "RD ":"", (events & POLLOUT) ? "WR": ""));
    return events;
}
#endif /* #ifdef NEXUS_HAS_SOCKET_DRIVER */

static struct file_operations
nexus_driver_fops = {
    owner:          THIS_MODULE,
#ifdef NEXUS_HAS_SOCKET_DRIVER
    read:           nexus_driver_read,
    write:          nexus_driver_write,
    poll:           nexus_driver_poll,
#else
    read:           NULL,
    write:          NULL,
#endif
    unlocked_ioctl: nexus_driver_ioctl_native,
#ifdef CONFIG_COMPAT
    compat_ioctl: nexus_driver_ioctl_compat,
#endif
    mmap:           nexus_driver_mmap,
    open:           nexus_driver_open,
    release:        nexus_driver_close
};

static char b_env[256]="";
static char b_devname[16]="nexus";
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
module_param_string(config, b_env, sizeof(b_env), 0);
module_param_string(devname, b_devname, sizeof(b_devname), 0);
#endif

#ifdef B_REFSW_ANDROID
#include <linux/miscdevice.h>
static struct miscdevice nexus_miscdevice = {
   .minor = MISC_DYNAMIC_MINOR,
   .name = b_devname,
   .fops = &nexus_driver_fops,
};
#endif

static void
b_parse_env(char *env)
{
    char *s;
    const char *name;
    const char *value;
    /* traverse  string, and split it to name/value pairs */
    for(s=env, name=env, value=NULL;;s++) {
        switch(*s) {
        case '\0':
            goto done;
        case '=':
            *s = '\0';
            value = s+1;
            break;
        case ' ':
        case ':':
        case ';':
            *s = '\0';
            if (value==NULL) {
                value=s;
            }
            NEXUS_SetEnv(name, value);
            name = s+1;
            value = NULL;
            break;
        default:
            break;
        }
    }
done:
    if(*name) {
        if (value==NULL) {
            value=s;
        }
        NEXUS_SetEnv(name, value);
    }
    return;
}

static int
nexus_driver_open(struct inode *inode, struct file *file)
{
    int rc;
    struct nexus_generic_driver_state *state;
    const char *process_name;
    bool trusted;

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18)
    trusted = current->euid == 0;
    process_name = NULL;
#elif LINUX_VERSION_CODE < KERNEL_VERSION(3,14,0) /*TODO, find which kernel version has forced this change */
    trusted = current_euid() == 0;
    process_name = current->comm;
#else
    kuid_t euid;
    euid = current_euid();
    trusted = euid.val == 0;
    process_name = current->comm;
#endif
    rc = nexus_generic_driver_open(&state, current->group_leader->pid, process_name, trusted);
    if (rc) return -ENODEV;
    file->private_data = state;
    return 0;
}

static int
nexus_driver_close(struct inode *inode, struct file *file)
{
    struct nexus_generic_driver_state *state = file->private_data;
#ifdef NEXUS_HAS_SOCKET_DRIVER
    bool socket = true;

    mutex_lock(&socket_lock);
    BDBG_MSG_TRACE((">>close %p", (void *)state));
    switch(state->kind) {
    case nexus_generic_driver_kind_connect:
        {
            struct nexus_driver_socket_connect *s = state->state.connect;
            BDBG_OBJECT_ASSERT(s, nexus_driver_socket_connect);
            BDBG_MSG_TRACE((">>close connect %p server:%p", (void *)s, (void *)s->server));
            if(s->server) { /* accepted socket */
                s->server->client = NULL;
                nexus_driver_socket_wakeup_server(s->server);
            } else if(!s->server_closed && listen_socket) { /* socket was not accepted */
                BLST_S_REMOVE(&listen_socket->connect_list, s, nexus_driver_socket_connect , link);
                nexus_driver_socket_wakeup_listener(listen_socket);
            }
            BDBG_OBJECT_DESTROY(s, nexus_driver_socket_connect);
            BKNI_Free(s);
        }
        break;
    case nexus_generic_driver_kind_accept:
        {
            struct nexus_driver_socket_accept *s = state->state.accept;
            BDBG_OBJECT_ASSERT(s, nexus_driver_socket_accept);
            BDBG_MSG_TRACE((">>close accept %p client:%p", (void *)s, (void *)s->client));
            if(s->client) { /* socket is connected */
                s->client->server = NULL;
                s->client->server_closed = true;
                nexus_driver_socket_wakeup_client(s->client);
            }
            BDBG_OBJECT_DESTROY(s, nexus_driver_socket_accept);
            BKNI_Free(s);
            break;
        }
    case nexus_generic_driver_kind_listen:
        {
            struct nexus_driver_socket_listen *s = state->state.listen;
            BDBG_MSG_TRACE((">>close listen %p", (void *)s));
            BDBG_OBJECT_ASSERT(s, nexus_driver_socket_listen);
            BDBG_ASSERT(s == listen_socket);
            listen_socket = NULL;
            BDBG_OBJECT_DESTROY(s, nexus_driver_socket_listen);
            BKNI_Free(s);
            break;
        }
    default:
        socket = false;
        break;
    }
    if(socket) {
        state->kind = nexus_generic_driver_kind_uninitialized;
    }
    BDBG_MSG_TRACE(("<<close %p", (void *)state));
    mutex_unlock(&socket_lock);
#endif /* #ifdef NEXUS_HAS_SOCKET_DRIVER */
    nexus_generic_driver_close(state, signal_pending(current));
    return 0;
}

#define B_IS_INTERSECT(off1, len1, off2, len2) ((off1) <= ((off2)+(len2)-1) && (off2) <= ((off1)+(len1)-1))

static int nexus_driver_mmap(struct file *file, struct vm_area_struct *vma) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
    int rc;
    uint64_t offset = ((uint64_t)vma->vm_pgoff) << PAGE_SHIFT;
    struct nexus_generic_driver_state *state = file->private_data;

    rc = nexus_generic_driver_validate_mmap(state, offset, vma->vm_end - vma->vm_start);
    if (rc) return -EINVAL;
    
    /* allow uncached mmap */    
    if (file->f_flags & O_SYNC) {
        vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
    }
#if NEXUS_CPU_ARM && !NEXUS_CPU_ARM64 /* Arm64 kernel doesn't use these PTE options */
    else {
        /*
         * ARM has an explicit setting for WC. Use default for other
         * architectures.
         */
         vma->vm_page_prot = __pgprot_modify(vma->vm_page_prot,
                                    L_PTE_MT_MASK, L_PTE_MT_BUFFERABLE | L_PTE_MT_WRITEBACK);
    }
#if 0
    vma->vm_page_prot = __pgprot_modify(vma->vm_page_prot, L_PTE_MT_MASK, L_PTE_XN); /* /dev/nexus should not be used to map memory which is executable */
#else
    if(B_IS_INTERSECT(vma->vm_start, vma->vm_end - vma->vm_start, BCHP_PHYSICAL_OFFSET, BCHP_PHYSICAL_OFFSET + BCHP_REGISTER_END)) {
        (void)BERR_TRACE(NEXUS_NOT_SUPPORTED); /* mapping of register window is not supported */
        return -EINVAL;
    }
#endif
#endif

    rc = remap_pfn_range(vma,
        vma->vm_start,
        vma->vm_pgoff,
        vma->vm_end - vma->vm_start,
        vma->vm_page_prot);
    if (rc) {
        return -EAGAIN;
    }

    return rc;
#else
    /* only tested on 2.6 */
    return -ENODEV;
#endif
}

static long
nexus_driver_ioctl(struct file * file, unsigned int cmd, unsigned long arg, bool compat)
{
    int rc = 0;
    bool socket = true;
    unsigned module = NEXUS_IOCTL_NUM(cmd) / NEXUS_IOCTL_PER_MODULE;
    struct nexus_generic_driver_state *state = file->private_data;
    current->flags |= PF_NOFREEZE;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
    current->flags |= PF_FREEZER_SKIP;
#endif
#if NEXUS_HAS_SOCKET_DRIVER
    if(state->kind == nexus_generic_driver_kind_nexus) {
        socket = false;
    } else if(state->kind == nexus_generic_driver_kind_uninitialized) {
        BDBG_MSG_TRACE((">>kind:uninitialized:%p ioctl %#x", (void *)state, (unsigned)cmd));
        mutex_lock(&socket_lock);
        switch(cmd) {
        case NEXUS_PLATFORM_SOCKET_LISTEN:
            {
                struct nexus_driver_socket_listen *s;
                BDBG_MSG_TRACE((">>listen:%p", (void *)state));

                if(listen_socket!=NULL || !state->trusted) { /* only trusted applications could call listen */
                    (void)BERR_TRACE(NEXUS_NOT_AVAILABLE);
                    rc = -1;
                    break;
                }
                s = BKNI_Malloc(sizeof(*s));
                if(s==NULL) {
                    (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
                    rc = -1;
                    break;
                }
                BDBG_OBJECT_INIT(s, nexus_driver_socket_listen);
                init_waitqueue_head(&s->wq);
                BLST_S_INIT(&s->connect_list);
                s->generic = state;
                state->kind = nexus_generic_driver_kind_listen;
                state->state.listen = s;
                listen_socket = s;
                BDBG_MSG_TRACE(("<<listen(%p) %p", (void *)state, (void *)s ));
                break;
            }
        case NEXUS_PLATFORM_SOCKET_ACCEPT:
            {
                struct nexus_driver_socket_accept *s;
                struct nexus_driver_socket_connect *client;

                BDBG_MSG_TRACE((">>accept(%p) (listen:%p) ", (void *)state, (void *)listen_socket));
                if(listen_socket==NULL || listen_socket->generic->pid != state->pid ) { /* only the same application could use listen and accept */
                    (void)BERR_TRACE(NEXUS_NOT_AVAILABLE);
                    rc = -1;
                    break;
                }
                BDBG_OBJECT_ASSERT(listen_socket, nexus_driver_socket_listen);
                client = BLST_S_FIRST(&listen_socket->connect_list);
                if(client==NULL) {
                    (void)BERR_TRACE(NEXUS_NOT_AVAILABLE);
                    rc = -1;
                    break;
                }
                s = BKNI_Malloc(sizeof(*s));
                if(s==NULL) {
                    (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
                    rc = -1;
                    break;
                }
                BDBG_OBJECT_INIT(s, nexus_driver_socket_accept);
                nexus_driver_socket_buffer_init(&s->buffer);
                init_waitqueue_head(&s->wq);
                BLST_S_REMOVE_HEAD(&listen_socket->connect_list, link);
                s->client = client;
                client->server = s;
                state->state.accept = s;
                state->kind = nexus_generic_driver_kind_accept;
                BDBG_MSG_TRACE(("<<accept(%p) (listen:%p) client:%p -> %p ", (void *)state, (void *)listen_socket, (void *)client, (void*)s));
                nexus_driver_socket_wakeup_client(client);
            }
            break;
        case NEXUS_PLATFORM_SOCKET_CONNECT:
            {
                struct nexus_driver_socket_connect *s;
                BDBG_MSG_TRACE((">>connect(%p) (listen:%p) ", (void *)state, (void *)listen_socket));

                if(listen_socket==NULL) { /* connect could be only called if there is already 'listen' socket */
                    /* (void)BERR_TRACE(NEXUS_NOT_AVAILABLE); */
                    rc = -1;
                    break;
                }
                BDBG_OBJECT_ASSERT(listen_socket, nexus_driver_socket_listen);
                s = BKNI_Malloc(sizeof(*s));
                if(s==NULL) {
                    (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
                    rc = -1;
                    break;
                }
                BDBG_OBJECT_INIT(s, nexus_driver_socket_connect);
                nexus_driver_socket_buffer_init(&s->buffer);
                init_waitqueue_head(&s->wq);
                s->pid = state->pid;
                s->server = NULL;
                s->server_closed = false;
                BLST_S_INSERT_HEAD(&listen_socket->connect_list, s, link);
                state->state.connect = s;
                state->kind = nexus_generic_driver_kind_connect;
                BDBG_MSG_TRACE(("<<connect(%p) (listen:%p) -> %p ", (void *)state, (void *)listen_socket, (void*)s));
                nexus_driver_socket_wakeup_listener(listen_socket);
                break;
            }
            (void)BERR_TRACE(NEXUS_NOT_AVAILABLE);
        default:
            socket = false;
            break;
        }
        BDBG_MSG_TRACE(("<<kind:uninitialized:%p ioctl %#x -> %u %s", (void *)state, (unsigned)cmd, rc, socket?"socket":""));
        mutex_unlock(&socket_lock);
    } else if(state->kind == nexus_generic_driver_kind_accept) {
        switch(cmd) {
        case NEXUS_PLATFORM_SOCKET_GET_PID:
            {
                struct nexus_driver_socket_accept *s = state->state.accept;
                struct nexus_driver_socket_connect *client;
                BDBG_OBJECT_ASSERT(s, nexus_driver_socket_accept);
                mutex_lock(&socket_lock);
                client = s->client;
                if(client) {
                    BDBG_OBJECT_ASSERT(client, nexus_driver_socket_connect);
                    BDBG_MSG_TRACE(("<<accept GET_PID(%p,%p) client:%p -> %#x", (void *)state, (void *)s, (void*)client, client->pid));
                    if(copy_to_user((void *)arg, &client->pid, sizeof(client->pid))!=0) {
                        rc = -EFAULT;
                    }
                } else {
                    (void)BERR_TRACE(NEXUS_NOT_AVAILABLE);
                    rc = -ENOENT;
                }
                mutex_unlock(&socket_lock);
            }
            break;
        default:
            rc = -1;
            break;
        }
    } else {
        rc = -1;
    }
#else /* #if NEXUS_HAS_SOCKET_DRIVER */
    socket = false;
#endif
    if(rc==0 && !socket) {
        rc = nexus_generic_driver_ioctl(module, state, cmd, arg, compat);
    }
    current->flags &= ~PF_NOFREEZE;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
    current->flags &= ~PF_FREEZER_SKIP;
#endif
    
    if (rc) return -EFAULT;
    return 0;
}

int
nexus_init_module(void)
{
    int rc;
    struct nexus_generic_driver_init_settings settings;
    unsigned i;

    memset(&settings, 0, sizeof(settings));
    BSTD_UNUSED(i);

#if NEXUS_USE_CMA
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
    for (i=0;i<sizeof(settings.region)/sizeof(settings.region[0]);i++) {
  #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,1)
        phys_addr_t offset, size;
  #else
        unsigned long offset, size;
  #endif
        rc = bmem_region_info(i, &offset, &size);
        if (rc) break;
        settings.region[i].offset = offset;
        settings.region[i].size = size;
    }
#elif HAVE_RSRVD
#define LOWERMEM (256*1024*1024)
    if (get_RAM_size() > LOWERMEM) {
        /* get_RSVD_size() only applies to amount reserved from lower 256, not amount reserved from get_RAM_size() */
        settings.region[0].size = get_RSVD_size();
        settings.region[0].offset = LOWERMEM - settings.region[0].size;
        /* do not program settings.region[1] for upper 256. for 2.6.18, assume that it will be used by the kernel.
        2.6.31 and later gives options for nexus/magnum use. */
    }
    else {
        settings.region[0].size = get_RSVD_size();
        settings.region[0].offset = get_RAM_size() - settings.region[0].size;
    }
#else
#error
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0)) && (NEXUS_CPU_ARM || NEXUS_CPU_ARM64)
    {
        struct brcm_cache_info info;
        brcm_get_cache_info(&info);
        settings.maxDcacheLineSize = info.max_writeback;
}
    /* If MEM's alignment is not set to the MIPS L1 and (if present) L2 cache line size,
    we will have cache coherency problems (which lead to major system failures).
    This code verifies that Nexus's MEM configuration is compatible with the MIPS cache line size.
    If this code fails, please check to make sure the Linux kernel is configured right, then modify nexus_core_features.h to match. */
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)) && (NEXUS_CPU_ARM || NEXUS_CPU_ARM64)
    {
        struct brcm_cache_info info;
        brcm_get_cache_info(&info);
        settings.maxDcacheLineSize = info.max_writeback;
    }
#else
    settings.maxDcacheLineSize = cpu_data[0].dcache.linesz; /* L1 cache */
    if (cpu_data[0].scache.linesz > settings.maxDcacheLineSize) {
        settings.maxDcacheLineSize = cpu_data[0].scache.linesz; /* L2 cache */
    }
    /* Ignoring cpu_data[0].tcache.linesz for now. */
#endif

    b_parse_env(b_env);
    rc = nexus_generic_driver_init(&settings);
    if (rc) goto error;

#ifdef B_REFSW_ANDROID
    misc_register(&nexus_miscdevice);
#else
    register_chrdev(NEXUS_DRIVER_MAJOR, b_devname, &nexus_driver_fops);
#endif

    printk("nexus driver initialized\n");
    return 0;

error:
    return rc;
}

void
nexus_cleanup_module(void)
{
#ifdef B_REFSW_ANDROID
    misc_deregister(&nexus_miscdevice);
#else
    unregister_chrdev(NEXUS_DRIVER_MAJOR, b_devname);
#endif
    nexus_generic_driver_uninit();

    printk("nexus driver uninitialized\n");
    return;
}

/* needed for local init with proxy */
EXPORT_SYMBOL(nexus_generic_driver_open);
EXPORT_SYMBOL(nexus_generic_driver_close);

MODULE_LICENSE("Proprietary");

/* if NEXUS_EXTERNAL_MODULE_INIT is defined, nexus_driver.c is being compiled into
another module which will connect to Linux. */
#ifndef NEXUS_EXTERNAL_MODULE_INIT
#ifdef MODULE
module_init(nexus_init_module);
module_exit(nexus_cleanup_module);
#else
#error "Not supported"
#endif
#endif

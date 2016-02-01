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
 * Wrapper over host malloc and free routines
 *
 * Revision History:
 * $brcm_Log: $ *
 * 
 ***************************************************************************/

/*
   This "macro" expects the following defines as it input:
    B_TRACK_ALLOC_LOCK() - code to acquire lock
    B_TRACK_ALLOC_UNLOCK() - code to release lock
    B_TRACK_ALLOC_ALLOC(size)  - allocates block of the system memory
    B_TRACK_ALLOC_FREE(ptr)  - frees allocated block
    B_TRACK_ALLOC_OS  - string that reflect name of the OS, it's only used in the debug output
*/

#if BKNI_TRACK_MALLOCS
#define  BKNI_GARBLE_MALLOC 1

struct BKNI_TrackAllocEntry {
    const void *mem;
    size_t size;
    const char *file;
    unsigned line;
};

struct BKNI_P_AllocUser {
    unsigned count;
    unsigned size;
    const char *file;
    unsigned line;
};

/* keep one table ~4K in size */
#define BKNI_P_TRACK_TABLE_SIZE (4096/sizeof(struct BKNI_TrackAllocEntry))

struct BKNI_TrackAllocTable {
    struct BKNI_TrackAllocEntry entries[BKNI_P_TRACK_TABLE_SIZE];
};


static struct {
    size_t allocated, freed, peak;
    unsigned allocated_cnt, freed_cnt;
    unsigned new_table_size;
    unsigned table_size;
    unsigned used_entries;
    struct BKNI_TrackAllocTable **tables; /* pointer to array of pointers */
    struct {
        struct BKNI_TrackAllocEntry alloc;
        struct {
            const char *file;
            unsigned line;
        } free;
    } history[256]; /* FIFO of last freed objects */
    struct BKNI_P_AllocUser alloc_users[16]; /* temporary array used to account frequent users in the event of table resizing */
} g_alloc_state;

static void
b_memset32(void *ptr, size_t size, uint32_t word)
{
   unsigned i;

   if(ptr) {
       for(i=0; i+3 < size; i+=4) {
          *(uint32_t*)(((uint8_t *)ptr)+i)=word;
       }
   }
   return;
}

static const char *
b_shorten_filename(const char *pFileName)
{
    const char *s;
    unsigned i;

    if(pFileName==NULL) {
        return "unknown";
    }
    for(s=pFileName;*s != '\0';s++) { } /* search forward */

    for(i=0;s!=pFileName;s--) { /* search backward */
        if(*s=='/' || *s=='\\') {
            i++;
            if(i>4) {
                return s+1;
            }
        }
    }
    return pFileName;
}


static unsigned
b_alloc_hashkey(const void *mem)
{
    unsigned long hash = (unsigned long)mem;
    /* calculate a hash */
    hash = (hash ^ 61) ^ (hash>> 16);
    hash += (hash << 3);
    hash ^= (hash >> 4);
    hash *= 0x66635119; /* 1717784857  some prime number */
    hash ^= (hash >> 15);
    return hash;
}

static struct BKNI_TrackAllocEntry *
BKNI_P_GetTrackAllocEntry_one(const void *mem, const void *match, struct BKNI_TrackAllocTable **table, unsigned table_size)
{
    unsigned i;
    unsigned tableIndex;
    unsigned tableNo;
    struct BKNI_TrackAllocTable *tableCurrent;
    unsigned index;
    if(table_size==0) {
        return NULL;
    }
    index = b_alloc_hashkey(mem)%table_size;
    tableIndex = index%BKNI_P_TRACK_TABLE_SIZE;
    tableNo = index/BKNI_P_TRACK_TABLE_SIZE;
    tableCurrent = table[tableNo];
    for(i=0;i<table_size;i++) { /* limit number of itterations */
        struct BKNI_TrackAllocEntry *entry = &tableCurrent->entries[tableIndex];
        if(entry->mem==match) {
#if 0
            /* some profiling code to measure effectivness of the hash lookup */
            static unsigned max_count=0;
            static unsigned total_count=0;
            total_count+=i;
            if(i>max_count) {
                unsigned avg = (100*total_count)/(g_alloc_state.allocated_cnt+g_alloc_state.freed_cnt+1);
                max_count=i;
                BDBG_P_PrintString("BKNI_GetTrackAllocEntry: scan_count:%u(%u.%02u avg) addr:%p key:%u elements:%u(%u/%u)\n", i,  avg/100, avg%100, mem, (index%table_size), g_alloc_state.allocated_cnt-g_alloc_state.freed_cnt, table_size, g_alloc_state.used_entries);
            }
#endif
            return  entry;
        }
        tableIndex++;
        if(tableIndex>=BKNI_P_TRACK_TABLE_SIZE) {
            tableIndex = 0;
            tableNo ++;
            if(tableNo >= table_size/BKNI_P_TRACK_TABLE_SIZE) {
                tableNo = 0;
            }
            tableCurrent = table[tableNo];
        }
    }
    return NULL;
}

static void
BKNI_P_UpdateAllocUsers(struct BKNI_P_AllocUser *alloc_users, unsigned alloc_users_size, const struct BKNI_TrackAllocEntry *entry)
{
    unsigned j;
    for(j=0;j<alloc_users_size;j++) {
        if(alloc_users[j].count==0) {
            /* found empty slot, just use it */
            alloc_users[j].count = 1;
            alloc_users[j].size = entry->size;
            alloc_users[j].file = entry->file;
            alloc_users[j].line = entry->line;
            break;
        } else if(alloc_users[j].file == entry->file && alloc_users[j].line == entry->line) {
            /* found existing entry, update it and move it up to keep array sorted */
            alloc_users[j].count++;
            alloc_users[j].size += entry->size;
            for(;j>0;j--) {
                struct BKNI_P_AllocUser tmp;
                if(alloc_users[j].count <= alloc_users[j-1].count) {
                    break;
                }
                tmp = alloc_users[j];
                alloc_users[j] = alloc_users[j-1];
                alloc_users[j-1] = tmp;
            }
            break;
        }
    }
    if(j==alloc_users_size) { /* no empty slots */
        j=alloc_users_size/2; /* wipe out half of old entries */
        alloc_users[j].count = 1;
        alloc_users[j].size = entry->size;
        alloc_users[j].file = entry->file;
        alloc_users[j].line = entry->line;
        for(j=j+1;j<alloc_users_size;j++) {
            alloc_users[j].count = 0;
            alloc_users[j].size = 0;
            alloc_users[j].file = 0;
            alloc_users[j].line = 0;
        }
    }
    return;
}

static struct BKNI_TrackAllocTable **
BKNI_P_GetTrackAllocEntry_resize(struct BKNI_TrackAllocTable **old_table, unsigned old_table_size, unsigned new_table_size)
{
    unsigned i;
    struct BKNI_TrackAllocTable **new_table;
    unsigned new_table_count = new_table_size / BKNI_P_TRACK_TABLE_SIZE;

    new_table = B_TRACK_ALLOC_ALLOC(new_table_count*sizeof(*new_table));
    if(new_table==NULL) { goto error;}

    for(i=0;i<new_table_count;i++) {
        unsigned j;
        struct BKNI_TrackAllocTable *table;

        table = B_TRACK_ALLOC_ALLOC(sizeof(*table));
        if(table==NULL) { goto error; }

        new_table[i] = table;
        for(j=0;j<BKNI_P_TRACK_TABLE_SIZE;j++) {
            table->entries[j].mem = NULL;
            table->entries[j].size = 0;
            table->entries[j].file = NULL;
            table->entries[j].line = 0;
        }
    }

    if(old_table) {
        unsigned old_table_count = old_table_size / BKNI_P_TRACK_TABLE_SIZE;
        unsigned alloc_users_size = 0;
        struct BKNI_P_AllocUser *alloc_users = g_alloc_state.alloc_users;
        if(new_table_size > old_table_size && old_table_size >= 4096 /* keep old threshold */) {
            BDBG_P_PrintString("BKNI_P_GetTrackAllocEntry_resize: resizing from %u->%u\n", old_table_size, new_table_size);
            alloc_users_size = sizeof(g_alloc_state.alloc_users)/sizeof(g_alloc_state.alloc_users[0]);
            b_memset32(alloc_users, sizeof(*alloc_users)*alloc_users_size, 0);
        }
        for(i=0;i<old_table_count;i++) {
            const struct BKNI_TrackAllocTable *table = old_table[i];
            unsigned j;
            for(j=0;j<BKNI_P_TRACK_TABLE_SIZE;j++) {
                const struct BKNI_TrackAllocEntry *old_entry = &table->entries[j];
                if(old_entry->mem) {
                    struct BKNI_TrackAllocEntry *new_entry = BKNI_P_GetTrackAllocEntry_one(old_entry->mem, NULL, new_table, new_table_size);
                    if(new_entry==NULL) {
#if BDBG_DEBUG_BUILD
                        BKNI_Fail();/* after resize we _MUST_ be  to find empty slot */
#endif
                        goto error;
                    }
                    *new_entry = *old_entry; /* copy data to new hash */
                    BKNI_P_UpdateAllocUsers(alloc_users, alloc_users_size, new_entry);
                }
            }
        }
        if(alloc_users_size) {
            BDBG_P_PrintString("BKNI_Malloc(%s) top users:\n%10s, %7s, filename:line\n", B_TRACK_ALLOC_OS,  "blocks", "bytes");
            for(i=0;i<alloc_users_size;i++) {
                if(alloc_users[i].count==0) {
                    break;
                }
                if(alloc_users[i].count>=old_table_size/16) {
                    BDBG_P_PrintString("%10u, %7u, %s:%u\n", alloc_users[i].count, alloc_users[i].size,b_shorten_filename(alloc_users[i].file), alloc_users[i].line);
                }
            }
        }
        /* free old table */
        for(i=0;i<old_table_count;i++) {
            if(old_table[i]==NULL) {
                break;
            }
            B_TRACK_ALLOC_FREE(old_table[i]);
        }
        B_TRACK_ALLOC_FREE(old_table);
    }
    return new_table;
error:
    if (new_table) {
        for(i=0;i<new_table_count;i++) {
            if(new_table[i]==NULL) {
                break;
            }
            B_TRACK_ALLOC_FREE(new_table[i]);
        }
        B_TRACK_ALLOC_FREE(new_table);
    }
    BDBG_P_PrintString("BKNI_P_GetTrackAllocEntry_resize: can't reallocate alloc table %u->%u\n", old_table_size, new_table_size);
    return NULL;
}

/* this performs a lookup for the pointer. if it already exists, it is reused. otherwise, it is created.
if the existing table is filled, a larger table is created. */
static struct BKNI_TrackAllocEntry *
BKNI_P_CreateTrackAllocEntry(const void *mem)
{
    struct BKNI_TrackAllocTable **table = g_alloc_state.tables;
    unsigned table_size = g_alloc_state.table_size;
    struct BKNI_TrackAllocEntry *entry;

    if(g_alloc_state.used_entries * 5 >= table_size * 4) { /* grow table if utilization larger then 5/4 -> 80% */
        unsigned new_table_size =  g_alloc_state.new_table_size;
        struct BKNI_TrackAllocTable **new_table = BKNI_P_GetTrackAllocEntry_resize(table, table_size, new_table_size);
        if(new_table==NULL) {
            return NULL;
        }
        table = new_table;
        g_alloc_state.tables = new_table;
        g_alloc_state.new_table_size = new_table_size + (table_size ? table_size : new_table_size); /* use Fibonacci sequence */
        table_size = new_table_size;
        g_alloc_state.table_size = new_table_size;
    }
    entry = BKNI_P_GetTrackAllocEntry_one(mem, NULL, table, table_size);
    if(entry) {
        g_alloc_state.used_entries++;
    } else {
#if BDBG_DEBUG_BUILD
        BDBG_P_PrintString("BKNI_P_GetTrackAllocEntry_one: can't allocate entry %u:%u\n", table_size, g_alloc_state.used_entries);
        /* we _MUST_ be  to find empty slot */
        BKNI_Fail();
#endif
    }
    return entry;
}

/* this is a read-only lookup. if the entry does not exist, it will return NULL. */
static struct BKNI_TrackAllocEntry *
BKNI_P_GetTrackAllocEntry(const void *mem)
{
    return BKNI_P_GetTrackAllocEntry_one(mem, mem, g_alloc_state.tables, g_alloc_state.table_size);
}


void *
BKNI_Malloc_tagged(size_t size, const char *file, unsigned line)
{
    void *mem;
    ASSERT_NOT_CRITICAL();


#ifdef __KERNEL__
#ifdef BKNI_METRICS_ENABLED
    g_metrics.totalMemoryAllocated += size;
    g_metrics.totalMallocs++;
#endif
#endif
    mem = B_TRACK_ALLOC_ALLOC(size);

    if(mem) {
        struct BKNI_TrackAllocEntry *entry;
#if BKNI_GARBLE_MALLOC
        b_memset32(mem, size, 0xDEADDA7A);
#endif
        B_TRACK_ALLOC_LOCK();
        entry = BKNI_P_CreateTrackAllocEntry(mem);
        if(entry) {
            size_t used;
            entry->mem = mem;
            entry->size = size;
            entry->file = file;
            entry->line = line;
            /* Please see the comment in BKNI_Free_Tagged() for information about Coverity 'missing_lock' false positive */
            /* coverity[missing_lock: FALSE] */
            g_alloc_state.allocated += size;
            /* coverity[missing_lock: FALSE] */
            g_alloc_state.allocated_cnt ++;
            used = g_alloc_state.allocated - g_alloc_state.freed;
            if(used>g_alloc_state.peak) {
                /* coverity[missing_lock: FALSE] */
                g_alloc_state.peak = used;
            }
        } else {
            B_TRACK_ALLOC_FREE(mem);
           /* revert allocation */
           mem = NULL;
        }
        B_TRACK_ALLOC_UNLOCK();
    }
    if (!mem) {
        BDBG_P_PrintString("\n");
        BDBG_P_PrintString("BKNI_Alloc(%u): returned NULL at %s:%u\n", (unsigned)size, b_shorten_filename(file), line);
        BDBG_P_PrintString("\n");
    }
    return mem;
}

void
BKNI_Free_tagged(void *ptr, const char *file, unsigned line)
{
    ASSERT_NOT_CRITICAL();

    if(ptr) {
        struct BKNI_TrackAllocEntry *entry;
        size_t size=0;
        const unsigned history_size = sizeof(g_alloc_state.history)/sizeof(g_alloc_state.history[0]);
        B_TRACK_ALLOC_LOCK();
        entry = BKNI_P_GetTrackAllocEntry(ptr);
        if(entry) {
            unsigned history_index=g_alloc_state.freed_cnt%history_size;
            /* Coverity flagged 'missing_lock' when accessing g_alloc_state variable. Coverity identify 'missing_lock'
            using an algorithm where the Coverity see how many times the global variable is access with and without
            some kind of lock.  For offending variable, Coverity found access to this variable usually had a lock,
            so it flagged any access to the variable without the lock as a Coverity issue. For this file, we use
            one global lock for access to all the global variables this file. We lock/unlock access using
            B_TRACK_ALLOC_LOCK()/B_TRACK_ALLOC_UNLOCK(), therefore the 'missing_lock' is false positive */
            /* coverity[missing_lock: FALSE] */
            g_alloc_state.history[history_index].alloc = *entry;
            /* coverity[missing_lock: FALSE] */
            g_alloc_state.history[history_index].free.file = file;
            /* coverity[missing_lock: FALSE] */
            g_alloc_state.history[history_index].free.line = line;
            /* coverity[missing_lock: FALSE] */
            g_alloc_state.freed += entry->size;
            /* coverity[missing_lock: FALSE] */
            g_alloc_state.freed_cnt++;
            size = entry->size;
            entry->mem = NULL; /* free entry */
            /* coverity[missing_lock: FALSE] */
            g_alloc_state.used_entries--;
            if(g_alloc_state.table_size > BKNI_P_TRACK_TABLE_SIZE && g_alloc_state.used_entries * 5 < g_alloc_state.table_size) {
                unsigned new_table_size = g_alloc_state.new_table_size - g_alloc_state.table_size; /* reverse Fibonacci sequence */
                struct BKNI_TrackAllocTable **new_table = BKNI_P_GetTrackAllocEntry_resize(g_alloc_state.tables, g_alloc_state.table_size, new_table_size);
                if(new_table) {
                    /* coverity[missing_lock: FALSE] */
                    g_alloc_state.tables = new_table;
                    /* coverity[missing_lock: FALSE] */
                    g_alloc_state.new_table_size = g_alloc_state.table_size;
                    /* coverity[missing_lock: FALSE] */
                    g_alloc_state.table_size = new_table_size;
                }
            }
        } else if(g_alloc_state.tables!=NULL) {
            unsigned i;
            BDBG_P_PrintString("BKNI_Free of unknown ptr: %#lx, %s:%u\n", (unsigned long)ptr, b_shorten_filename(file), line);
            for(i=0;i<history_size;i++) {
                if(g_alloc_state.history[i].alloc.mem == ptr) {
                    BDBG_P_PrintString("ptr: %#lx was previously allocated at %s:%u and freed at %s:%u\n", (unsigned long)ptr, b_shorten_filename(g_alloc_state.history[i].alloc.file), g_alloc_state.history[i].alloc.line, b_shorten_filename(g_alloc_state.history[i].free.file), g_alloc_state.history[i].free.line);
                }
            }
            BKNI_Fail();
        }
        B_TRACK_ALLOC_UNLOCK();
#if BKNI_GARBLE_MALLOC
        b_memset32(ptr, size, 0xDA7ADEAD);
#endif
    } else {
        BDBG_P_PrintString("BKNI_Free of NULL ptr: %#lx, %s:%u\n", (unsigned long)ptr, b_shorten_filename(file), line);
        BKNI_Fail();
    }
    B_TRACK_ALLOC_FREE(ptr);
    return;
}



#define BKNI_DUMP_SIZE_THRESHOLD        (1024)

static void
BKNI_DumpMallocs_Size(size_t threshold)
{
    unsigned i;
    bool header = false;
    struct BKNI_TrackAllocTable * const *tables;
    unsigned table_count;

    B_TRACK_ALLOC_LOCK();
    tables = g_alloc_state.tables;
    table_count = g_alloc_state.table_size / BKNI_P_TRACK_TABLE_SIZE;
    for (i=0;i<table_count;i++) {
        const struct BKNI_TrackAllocTable *table = tables[i];
        unsigned j;
        for(j=0;j<BKNI_P_TRACK_TABLE_SIZE;j++) {
            const struct BKNI_TrackAllocEntry *entry = &table->entries[j];
            if (entry->mem && entry->size > threshold) {
                if(!header) {
                    header=true;
                    BDBG_P_PrintString("BKNI_Malloc(%s) report:\n%10s, %7s, filename:line\n", B_TRACK_ALLOC_OS, "address", "size");
                }
                BDBG_P_PrintString("%#10lx, %7u, %s:%u\n", (unsigned long)entry->mem, (unsigned)entry->size, b_shorten_filename(entry->file), entry->line);
            }
        }
    }
    if (header) {
        BDBG_P_PrintString("BKNI_Malloc(%s): allocated:%u(%u) freed:%u(%u) peak:%u\n", B_TRACK_ALLOC_OS, (unsigned)g_alloc_state.allocated, g_alloc_state.allocated_cnt, (unsigned)g_alloc_state.freed, g_alloc_state.freed_cnt, (unsigned)g_alloc_state.peak);
    }
    B_TRACK_ALLOC_UNLOCK();
    return;
}

void
BKNI_DumpMallocs(void)
{
    BKNI_DumpMallocs_Size(BKNI_DUMP_SIZE_THRESHOLD);
}

BERR_Code
BKNI_GetMallocEntryInfo( const void *mem, struct BKNI_MallocEntryInfo *entry)
{
    const struct BKNI_TrackAllocEntry *allocated;

    entry->free_file = NULL;
    entry->free_line = 0;
    entry->alive = true;
    B_TRACK_ALLOC_LOCK();
    allocated = BKNI_P_GetTrackAllocEntry(mem);
    if(!allocated) {
        unsigned i;
        for(i=0;i<sizeof(g_alloc_state.history)/sizeof(g_alloc_state.history[0]);i++) {
            if(g_alloc_state.history[i].alloc.mem == mem) {
                allocated = &g_alloc_state.history[i].alloc;
                entry->free_file = g_alloc_state.history[i].free.file;
                entry->free_line = g_alloc_state.history[i].free.line;
                entry->alive = false;
                break;
            }
        }
    }
    if(allocated) {
        /* Please see the comment in BKNI_Free_Tagged() for information about Coverity 'missing_lock' false positive */
        /* coverity[missing_lock: FALSE] */
        entry->mem = allocated->mem;
        /* coverity[missing_lock: FALSE] */
        entry->size = allocated->size;
        /* coverity[missing_lock: FALSE] */
        entry->malloc_file = allocated->file;
        /* coverity[missing_lock: FALSE] */
        entry->malloc_line = allocated->line;
    }
    B_TRACK_ALLOC_UNLOCK();
    if(allocated) {
        entry->malloc_file = b_shorten_filename(entry->malloc_file);
        entry->free_file = b_shorten_filename(entry->free_file);
        return BERR_SUCCESS;
    }
    return BERR_NOT_SUPPORTED;
}

#undef BKNI_Malloc
void *
BKNI_Malloc(size_t size)
{
    return BKNI_Malloc_tagged(size, NULL, 0);
}

#undef BKNI_Free
void
BKNI_Free(void *ptr)
{
    BKNI_Free_tagged(ptr, NULL, 0);
    return;
}

static void
BKNI_P_TrackAlloc_Init(void)
{
    b_memset32(&g_alloc_state, sizeof(g_alloc_state), 0);
    g_alloc_state.table_size = 0;
    g_alloc_state.used_entries = 0;
    g_alloc_state.new_table_size = 1*BKNI_P_TRACK_TABLE_SIZE;
    g_alloc_state.tables = NULL;
    return;
}

static void
BKNI_P_TrackAlloc_Uninit(void)
{
    struct BKNI_TrackAllocTable **tables = g_alloc_state.tables;
    unsigned table_count = g_alloc_state.table_size / BKNI_P_TRACK_TABLE_SIZE;
    unsigned i;

    BKNI_DumpMallocs_Size(0);
    for(i=0;i<table_count;i++) {
        if(tables[i]==NULL) {
            break;
        }
        B_TRACK_ALLOC_FREE(tables[i]);
        tables[i] = NULL;
    }
    if(tables) {
        B_TRACK_ALLOC_FREE(tables);
    }
    g_alloc_state.tables = NULL;
    g_alloc_state.table_size = 0;
    g_alloc_state.new_table_size = 1*BKNI_P_TRACK_TABLE_SIZE;

    return;
}

#else /* BKNI_TRACK_MALLOCS */

void *
BKNI_Malloc(size_t size)
{
    void *ptr;
    ASSERT_NOT_CRITICAL();
    ptr = B_TRACK_ALLOC_ALLOC(size);
    return ptr;
}

void
BKNI_Free(void *ptr)
{
    ASSERT_NOT_CRITICAL();
    B_TRACK_ALLOC_FREE(ptr);
    return;
}

void *
BKNI_Malloc_tagged(size_t size, const char *file, unsigned line)
{
    BSTD_UNUSED(file);
    BSTD_UNUSED(line);
    return BKNI_Malloc(size);
}

void
BKNI_Free_tagged(void *ptr, const char *file, unsigned line)
{
    BSTD_UNUSED(file);
    BSTD_UNUSED(line);
    BKNI_Free(ptr);
    return;
}

BERR_Code BKNI_GetMallocEntryInfo( const void *mem, struct BKNI_MallocEntryInfo *entry)
{
    BSTD_UNUSED(mem);
    BSTD_UNUSED(entry);
    return BERR_NOT_SUPPORTED;
}

static void
BKNI_P_TrackAlloc_Init(void)
{
    return;
}

static void
BKNI_P_TrackAlloc_Uninit(void)
{
    return;
}

void
BKNI_DumpMallocs(void)
{
    return;
}

#endif /* BKNI_TRACK_MALLOCS */
#if  0
static void  allocator_unit_test(void)
{
    unsigned k;
    for(k=0;k<16;k++) {
        unsigned i;
        size_t max_block = 4*1024;
        size_t max_mem = 128*1024*1024;
        size_t total=0;
        void *ptrs[65536];
        for(i=0;i<sizeof(ptrs)/sizeof(*ptrs);i++) {
            size_t size = (random()%max_block)+1;
            ptrs[i] = NULL;
            if(total + size >= max_mem) {
                break;
            }
            BDBG_MSG(("alloc: %u:%u", i, size));
            ptrs[i] = BKNI_Malloc(size);
            if(ptrs[i]==NULL) {
                break;
            }
            total += size;
        }
        BDBG_LOG(("[%u]:allocated %uMBytes(%u)", k, total/(1024*1024),i));
        for(i=0;i<sizeof(ptrs)/sizeof(*ptrs);i++) {
            if(ptrs[i]==NULL) {
                break;
            }
            BKNI_Free(ptrs[i]);
            ptrs[i]=NULL;
        }
    }
}
#endif

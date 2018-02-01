/***************************************************************************
 * Copyright (C) 2003-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bdbg_os_priv.h"
#include "bdbg_log.h"
#include "blst_slist.h"
#include "bdbg_priv.h"

/* max number of instances per module */
#define MAX_MODULE_INSTANCES   32
#define BDBG_P_MAX_MODULE_NAME_LENGTH   64

#undef BDBG_SetLevel
#undef BDBG_GetLevel
#undef BDBG_SetModuleLevel
#undef BDBG_GetModuleLevel
#undef BDBG_SetInstanceLevel
#undef BDBG_GetInstanceLevel
#undef BDBG_SetInstanceName
#undef BDBG_EnumerateAll
#undef BDBG_GetModuleInstanceLevel
#undef BDBG_SetModuleInstanceLevel


struct BDBG_DebugInstModule {
      BLST_S_ENTRY(BDBG_DebugInstModule) link;

      BDBG_pDebugModuleFile pModule; /* pointer to module */
      BDBG_Level eLevel;
};

struct BDBG_DebugModuleInst {
      BLST_S_ENTRY(BDBG_DebugModuleInst) link;
      BDBG_Instance handle;
      BDBG_Level level;
      const char *name;
      BDBG_pDebugModuleFile module; /* pointer to module */

      BLST_S_HEAD(BDBG_DebugInstModuleHead, BDBG_DebugInstModule) modules;
};

typedef struct BDBG_P_LogEntry {
    unsigned long tag; /* tag is value of stack pointer and LSB  used to indicate header or body */
    int16_t rc; /* result from printf */
    char str[256-sizeof(unsigned long)-sizeof(void *)-sizeof(int16_t)];
} BDBG_P_LogEntry;

typedef struct BDBG_P_Dequeue_Context {
    bool used;
    uint16_t hdr_len;
    unsigned header_buf_size;
    unsigned long tag;
    char *header_buf;
} BDBG_P_Dequeue_Context;
#define BDBG_P_DEQUEUE_MAX_CONTEXTS 16

static struct {
    BLST_S_HEAD(BDBG_DebugModuleHead, BDBG_DebugModuleFile) modules;  /* sorted list of modules */
    BLST_S_HEAD(BDBG_DebugInstHead, BDBG_DebugModuleInst) instances;  /* sorted list of instances */
    BLST_S_HEAD(BDBG_DebugUnregInstHead, BDBG_DebugModuleInst) unregistered_instances;  /* list of unregistered instances */
    BDBG_Level level;
    BDBG_Fifo_Handle dbgLog;
    struct {
        unsigned last_used;
        unsigned nodata_count;
        BDBG_P_Dequeue_Context contexts[BDBG_P_DEQUEUE_MAX_CONTEXTS];
    } dequeueState;
    char assertMessage[128]; /* if used, this will be terminal. So use a static array to avoid stack blowout. */
} gDbgState = {
   BLST_S_INITIALIZER(BDBG_DebugModuleFile),
   BLST_S_INITIALIZER(BDBG_DebugModuleInst),
   BLST_S_INITIALIZER(BDBG_DebugModuleInst),
   BDBG_eWrn, /* default level is a warning level */
   NULL,
   {0,0,{{false,0,0,0,NULL}}},
   ""
};

static const char gDbgPrefix[][4] =
{
   "",
   "...",
   "---",
   "***",
   "###",
   "   "
};

const char BDBG_P_EmptyString[] = "";

static struct BDBG_DebugModuleInst * BDBG_P_GetInstance(BDBG_Instance handle);
static const char *BDBG_P_GetPrefix_isrsafe(BDBG_Level level);
static BERR_Code BDBG_P_RegisterModuleFile_isrsafe(BDBG_pDebugModuleFile dbg_module);
static bool BDBG_P_TestModule_isrsafe(BDBG_pDebugModuleFile dbg_module, BDBG_Level level);
static BDBG_pDebugModuleFile BDBG_P_GetModuleByName(const char *name);
static BDBG_pDebugModuleFile BDBG_P_GetModuleByName_sync_isrsafe(const char *name, BDBG_pDebugModuleFile module /* optional placeholder */ );
static int BDBG_P_StrCmp_isrsafe(const char *str1, const char *str2);
static BERR_Code BDBG_P_CheckDebugLevel(BDBG_Level level);

static struct BDBG_DebugInstModule * BDBG_P_GetInstanceModule(struct BDBG_DebugModuleInst *pInstance, BDBG_pDebugModuleFile pModule);
static struct BDBG_DebugInstModule * BDBG_P_GetInstanceModule_sync_isrsafe(struct BDBG_DebugModuleInst *pInstance, BDBG_pDebugModuleFile pModule);
static BERR_Code BDBG_P_SetInstanceModuleLevel(struct BDBG_DebugModuleInst *pInstance, BDBG_pDebugModuleFile pModule, BDBG_Level eLevel);
static struct BDBG_DebugModuleInst * BDBG_P_GetInstanceByName(const char *name);
static struct BDBG_DebugModuleInst * BDBG_P_GetInstanceByName_sync(const char *name);
static void BDBG_P_Dequeue_FreeMemory(void);
static void BDBG_P_Dequeue_FilterString(char *str);

#ifdef BDBG_ANDROID_LOG
#include <cutils/log.h>
static android_LogPriority BDBG_P_Level2Android(BDBG_Level level)
{
  switch ( level ) {
  case BDBG_eWrn: return ANDROID_LOG_WARN;
  case BDBG_eErr: return ANDROID_LOG_ERROR;
  default: return ANDROID_LOG_INFO;
  }
}
#endif

static const char *
BDBG_P_GetPrefix_isrsafe(BDBG_Level level)
{
   BDBG_CASSERT(sizeof(gDbgPrefix)/sizeof(*gDbgPrefix) == BDBG_P_eLastEntry);
   if (level<BDBG_P_eLastEntry) {
      return gDbgPrefix[level];
   } else {
      return gDbgPrefix[0];
   }
}

static int
BDBG_P_StrCmp_isrsafe(const char *str1, const char *str2)
{
   int ch1, ch2, diff;

   for(;;) {
      ch1 = *str1++;
      ch2 = *str2++;
      if (ch1=='*' || ch2=='*') {
         return 0;
      }
      diff = ch1 - ch2;
      if (diff) {
         return diff;
      } else if (ch1=='\0') {
         return 0;
      }
   }
}

static char * BDBG_P_StrnCpy(char *dest, const char *src, size_t num)
{
   char *p = dest;

   /* Always set the last character to NULL in the destination */
   num--;
   dest[num] = '\0';

   while (num-- && (*dest++ = *src++));

   return p;
}

static char * BDBG_P_StrChrNul(const char *str, int c)
{
   char *p = (char *) str;
   while ((*p != '\0') && (*p != c)) p++;

   return p;
}

static BERR_Code
BDBG_P_CheckDebugLevel(BDBG_Level level)
{
    if(level>=BDBG_P_eLastEntry) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    return BERR_SUCCESS;
}

static BERR_Code
BDBG_P_RegisterModuleFile_isrsafe(BDBG_pDebugModuleFile dbg_module)
{
   BDBG_pDebugModuleFile module;
   BERR_Code rc = BERR_SUCCESS;
   BDBG_pDebugModuleFile prev;

   BDBG_P_Lock_isrsafe();
   /* test if module has been already registered */
   if (dbg_module->level!=BDBG_P_eUnknown) {
      goto done; /* module was already registered */
   }

   module = BDBG_P_GetModuleByName_sync_isrsafe(dbg_module->name, dbg_module);
   if (!module) {
      rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
      goto done;
   }
   /* check whether new file exists in the module chain */
   for(prev=module; module!=NULL && dbg_module!=module ; module = BLST_S_NEXT(module, link)) {
      if(BDBG_P_StrCmp_isrsafe(dbg_module->name, module->name)!=0) {
         break;
      }
   }
   if(dbg_module!=module) {
      /* file not in the chain, add it */
      /* 1. copy level information */
      dbg_module->level = prev->level;
      dbg_module->module_level = prev->module_level;
      dbg_module->module_print = prev->module_print;
      /* 2. add into the sorted list */
      BLST_S_INSERT_AFTER(&gDbgState.modules, prev, dbg_module, link);
   }
  done:
   BDBG_P_Unlock_isrsafe();
   return rc;
}

static struct BDBG_DebugModuleInst *
BDBG_P_GetInstance_sync_isrsafe(BDBG_Instance handle)
{
   struct BDBG_DebugModuleInst *instance;

   for(instance = BLST_S_FIRST(&gDbgState.instances); instance ; instance = BLST_S_NEXT(instance, link)) {
      if ((uint8_t *)instance->handle >= (uint8_t *)handle) {
         break;
      }
   }
   if(instance && instance->handle!=handle) {
      instance = NULL;
   }
   return instance;
}

static struct BDBG_DebugModuleInst *
BDBG_P_GetInstance(BDBG_Instance handle)
{
   struct BDBG_DebugModuleInst *instance;

   BDBG_P_Lock_isrsafe();
   instance = BDBG_P_GetInstance_sync_isrsafe(handle);
   BDBG_P_Unlock_isrsafe();
   return instance;
}

static bool
BDBG_P_TestModule_isrsafe(BDBG_pDebugModuleFile dbg_module, BDBG_Level level)
{
   BERR_Code rc;

   /*
    * try do a trick don't acquire mutex on first test, and use double test techinques.
    * Note: this technique would have a race condition on the SMP systems
    */
   if (dbg_module->level==BDBG_P_eUnknown) {
      /* register module file in the system */
      rc = BDBG_P_RegisterModuleFile_isrsafe(dbg_module);
      if (rc!=BERR_SUCCESS) {
         return false;
      }
   }
   return (int)level >= dbg_module->level;
}

static size_t
BDBG_P_strlen(const char *str)
{
   size_t len;
   for(len=0;*str++!='\0';len++) { }
   return len;
}

/* should be called with mutex already held */
static BDBG_pDebugModuleFile
BDBG_P_GetModuleByName_sync_isrsafe(const char *name, BDBG_pDebugModuleFile new_module)
{
   BDBG_pDebugModuleFile module, prev;
   int cmp=-1;

   BDBG_CASSERT(sizeof(char)==1); /* prerequesite for the block allocation */

   /* traverse all known modules */
   for(prev=NULL, module = BLST_S_FIRST(&gDbgState.modules); module ; module = BLST_S_NEXT(module, link)) {
      cmp = BDBG_P_StrCmp_isrsafe(name, module->name);
      if(cmp>=0) {
         break;
      }
      prev = module;
   }
   if(cmp==0) { /* exact match */
      goto done;
   }
   /* else insert new module */
   module = new_module;
   if(module==NULL) { goto done; }
   module->module_alloc = false;
   module->module_print = NULL;
   module->level = gDbgState.level;
   module->module_level = gDbgState.level;
   module->name = name;
   if(prev) {
      BLST_S_INSERT_AFTER(&gDbgState.modules, prev, module, link);
   } else {
      BLST_S_INSERT_HEAD(&gDbgState.modules, module, link);
   }

  done:
   return module;
}


static BDBG_pDebugModuleFile
BDBG_P_GetModuleByName(const char *name)
{
    BDBG_pDebugModuleFile old_module;
    BDBG_pDebugModuleFile new_module;
    size_t name_len;
    char *copy_name;

    /* we can't allocate memory inside BDBG_P_GetModuleByName_sync_isrsafe, so try to find existing module first */
    BDBG_P_Lock_isrsafe();
    old_module = BDBG_P_GetModuleByName_sync_isrsafe(name, NULL);
    BDBG_P_Unlock_isrsafe();
    if(old_module) {goto done;}
    /* and if it fails allocate memory, copy string and prepapre to free allocated memory if returned something else */

    name_len=BDBG_P_strlen(name)+1; /* string length plus trailing 0 */
    /* this code should never executed at the interrupt time */
    new_module = BKNI_Malloc(sizeof(*new_module)+name_len);
    if (!new_module) {
        return NULL;
    }
    copy_name = (char *)new_module + sizeof(*new_module);
    BKNI_Memcpy(copy_name, name, name_len); /* copy name */

    BDBG_P_Lock_isrsafe();
    old_module = BDBG_P_GetModuleByName_sync_isrsafe(copy_name, new_module);
    if(old_module==new_module) {
        new_module->module_alloc = true;
    }
    BDBG_P_Unlock_isrsafe();

    if(old_module!=new_module) {
        BKNI_Free(new_module);
    }
done:
    return old_module;
}

static struct BDBG_DebugModuleInst *
BDBG_P_GetInstanceByName_sync(const char *name)
{
   struct BDBG_DebugModuleInst *pInstance;

   /* traverse all known instances */
   for(pInstance = BLST_S_FIRST(&gDbgState.instances); pInstance ; pInstance = BLST_S_NEXT(pInstance, link))
   {
      if (pInstance->name)
      {
         if (BDBG_P_StrCmp_isrsafe(name, pInstance->name) == 0)
         {
            return pInstance;
         }
      }
   }

   return NULL;
}

static struct BDBG_DebugModuleInst *
BDBG_P_GetInstanceByName(const char *name)
{
   struct BDBG_DebugModuleInst * pInstance;

   BDBG_P_Lock_isrsafe();

   pInstance = BDBG_P_GetInstanceByName_sync(name);

   BDBG_P_Unlock_isrsafe();
   return pInstance;
}

#define BDBG_P_Vprintf_module(kind, instance, _level, dbg_module, fmt, ap) do { \
    BDBG_DebugModule_Print module_print = dbg_module->module_print; \
    bool normal_print = instance || (module_print==NULL ? ((int)_level >= dbg_module->level) : ((int)_level >= -dbg_module->level)); \
    if(module_print) { module_print(kind, _level, dbg_module, fmt, ap); } \
    else if(normal_print) { BDBG_P_Vprintf_Log_isrsafe(kind, fmt, ap); } \
} while(0)

void
BDBG_P_Vprintf_Log_isrsafe(BDBG_ModulePrintKind kind, const char *fmt, va_list ap)
{
    BDBG_Fifo_Handle dbgLog = gDbgState.dbgLog;
    if(dbgLog) {
        BDBG_Fifo_Token token;
        BDBG_P_LogEntry *log = BDBG_Fifo_GetBuffer(dbgLog, &token);
        if(log) {
            log->tag = (unsigned long)&token | kind;
            log->rc = BKNI_Vsnprintf(log->str, sizeof(log->str), fmt, ap);
            BDBG_Fifo_CommitBuffer(&token);
        }
    } else {
#ifdef  BDBG_ANDROID_LOG
        /* Most prints should be caught in the test/print macros above but this will catch any others */
        LOG_PRI_VA(ANDROID_LOG_WARN, "NEXUS", fmt, ap);
#else
#ifdef NEXUS_BASE_OS_ucos_ii
        BDBG_P_Dequeue_FilterString((char *)fmt);
#endif
        BKNI_Vprintf(fmt, ap);
        if(kind==BDBG_ModulePrintKind_eHeaderAndBody  || kind==BDBG_ModulePrintKind_eBody ) {
            #if defined(NEXUS_BASE_OS_ucos_ii) && !defined(MIPS_SDE)
                dbg_print(" \n");
            #endif
            BKNI_Vprintf("\n",ap);
        }
#endif
    }
    return ;
}

/* to save tiny bit of stack, merge two parameters, 'bool instance' and 'BDBG_Level level' into single parameter, 'unsigned kind' */
#define BDBG_P_INSTANCE_BIT (1<<4)
#define BDBG_P_LEVEL_MASK 0xF
static void BDBG_P_PrintHeader_isrsafe(unsigned kind, BDBG_pDebugModuleFile dbg_module, const char *fmt, ...) BDBG_P_PRINTF_FORMAT(3, 4);
static void
BDBG_P_PrintHeader_isrsafe(unsigned kind, BDBG_pDebugModuleFile dbg_module, const char *fmt, ...)
{
    bool instance = (kind & BDBG_P_INSTANCE_BIT)!=0;
    BDBG_Level level = kind & BDBG_P_LEVEL_MASK;
    va_list ap;
    va_start(ap, fmt);
    BDBG_P_Vprintf_module(BDBG_ModulePrintKind_eHeader, instance, level, dbg_module, fmt, ap);
    va_end( ap );
    return;
}


static void
BDBG_P_VprintBody_isrsafe(bool instance, BDBG_Level level, BDBG_pDebugModuleFile dbg_module, const char *fmt, va_list ap)
{
    BDBG_P_Vprintf_module(BDBG_ModulePrintKind_eBody, instance, level, dbg_module, fmt, ap);
    return;
}

static void BDBG_P_PrintTrace_isrsafe(BDBG_pDebugModuleFile dbg_module, const char *fmt, ...) BDBG_P_PRINTF_FORMAT(2, 3);
static void
BDBG_P_PrintTrace_isrsafe(BDBG_pDebugModuleFile dbg_module, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    BDBG_P_Vprintf_module(BDBG_ModulePrintKind_eHeaderAndBody, false, BDBG_eTrace, dbg_module, fmt, ap);
    va_end( ap );
    return;
}


void /* only used externally if BDBG_P_UNWRAP is undefined */
BDBG_P_PrintWithNewLine_isrsafe(const char *fmt, ...)
{
   va_list ap;

   va_start(ap, fmt);
   BDBG_P_Vprintf_Log_isrsafe(BDBG_ModulePrintKind_eBody, fmt, ap);
   va_end( ap );
   return;
}

bool
BDBG_P_TestAndPrint_isrsafe(BDBG_Level level, BDBG_pDebugModuleFile dbg_module, const char *fmt, ...)
{
    if( BDBG_P_TestModule_isrsafe(dbg_module, level)) {
        union {
            char timeStamp[16];
            va_list ap;
        } u; /* use union to merge two fields on stack */
#ifdef BDBG_ANDROID_LOG
        if ( fmt ) {
          android_LogPriority prio = BDBG_P_Level2Android(level);
          LOG_PRI(prio, "NEXUS", "%s: ", dbg_module->name);
          va_start(u.ap, fmt);
          LOG_PRI_VA(prio, "NEXUS", fmt, u.ap);
          va_end(u.ap);
        }
        else {
#endif
        BDBG_P_GetTimeStamp_isrsafe(u.timeStamp, sizeof(u.timeStamp));
        BDBG_P_PrintHeader_isrsafe(level, dbg_module, "%s %s %s: ", BDBG_P_GetPrefix_isrsafe(level), u.timeStamp, dbg_module->name);
        if(fmt) { /* also print body */
            va_start(u.ap, fmt);
            BDBG_P_VprintBody_isrsafe(false, level, dbg_module, fmt, u.ap);
            va_end( u.ap );
        }
#ifdef BDBG_ANDROID_LOG
        }
#endif
        return true;
    }
    return false;
}

bool
BDBG_P_TestAndPrint_BDBG_eWrn_isrsafe(BDBG_pDebugModuleFile dbg_module, const char *fmt, ...)
{
    if( BDBG_P_TestModule_isrsafe(dbg_module, BDBG_eWrn)) {
        union {
            char timeStamp[16];
            va_list ap;
        } u; /* use union to merge two fields on stack */
#ifdef BDBG_ANDROID_LOG
        if ( fmt ) {
          android_LogPriority prio = BDBG_P_Level2Android(BDBG_eWrn);
          LOG_PRI(prio, "NEXUS", "%s: ", dbg_module->name);
          va_start(u.ap, fmt);
          LOG_PRI_VA(prio, "NEXUS", fmt, u.ap);
          va_end(u.ap);
        }
        else {
#endif
        BDBG_P_GetTimeStamp_isrsafe(u.timeStamp, sizeof(u.timeStamp));
        BDBG_P_PrintHeader_isrsafe(BDBG_eWrn, dbg_module, "%s %s %s: ", BDBG_P_GetPrefix_isrsafe(BDBG_eWrn), u.timeStamp, dbg_module->name);
        if(fmt) { /* also print body */
            va_start(u.ap, fmt);
            BDBG_P_VprintBody_isrsafe(false, BDBG_eWrn, dbg_module, fmt, u.ap);
            va_end( u.ap );
        }
#ifdef BDBG_ANDROID_LOG
        }
#endif
        return true;
    }
    return false;
}

bool
BDBG_P_TestAndPrint_BDBG_eErr_isrsafe(BDBG_pDebugModuleFile dbg_module, const char *fmt, ...)
{
    if( BDBG_P_TestModule_isrsafe(dbg_module, BDBG_eErr)) {
        union {
            char timeStamp[16];
            va_list ap;
        } u; /* use union to merge two fields on stack */
#ifdef BDBG_ANDROID_LOG
        if ( fmt ) {
          android_LogPriority prio = BDBG_P_Level2Android(BDBG_eErr);
          LOG_PRI(prio, "NEXUS", "%s: ", dbg_module->name);
          va_start(u.ap, fmt);
          LOG_PRI_VA(prio, "NEXUS", fmt, u.ap);
          va_end(u.ap);
        }
        else {
#endif
        BDBG_P_GetTimeStamp_isrsafe(u.timeStamp, sizeof(u.timeStamp));
        BDBG_P_PrintHeader_isrsafe(BDBG_eErr, dbg_module, "%s %s %s: ", BDBG_P_GetPrefix_isrsafe(BDBG_eErr), u.timeStamp, dbg_module->name);
        if(fmt) { /* also print body */
            va_start(u.ap, fmt);
            BDBG_P_VprintBody_isrsafe(false, BDBG_eErr, dbg_module, fmt, u.ap);
            va_end( u.ap );
        }
#ifdef BDBG_ANDROID_LOG
        }
#endif
        return true;
    }
    return false;
}

bool
BDBG_P_TestAndPrint_BDBG_eLog_isrsafe(BDBG_pDebugModuleFile dbg_module, const char *fmt, ...)
{
    if( BDBG_P_TestModule_isrsafe(dbg_module, BDBG_eLog)) {
        union {
            char timeStamp[16];
            va_list ap;
        } u; /* use union to merge two fields on stack */
#ifdef BDBG_ANDROID_LOG
        if ( fmt ) {
          android_LogPriority prio = BDBG_P_Level2Android(BDBG_eLog);
          LOG_PRI(prio, "NEXUS", "%s: ", dbg_module->name);
          va_start(u.ap, fmt);
          LOG_PRI_VA(prio, "NEXUS", fmt, u.ap);
          va_end(u.ap);
        }
        else {
#endif
        BDBG_P_GetTimeStamp_isrsafe(u.timeStamp, sizeof(u.timeStamp));
        BDBG_P_PrintHeader_isrsafe(BDBG_eErr, dbg_module, "%s %s %s: ", BDBG_P_GetPrefix_isrsafe(BDBG_eLog), u.timeStamp, dbg_module->name);
        if(fmt) { /* also print body */
            va_start(u.ap, fmt);
            BDBG_P_VprintBody_isrsafe(false, BDBG_eLog, dbg_module, fmt, u.ap);
            va_end( u.ap );
        }
#ifdef BDBG_ANDROID_LOG
        }
#endif
        return true;
    }
    return false;
}


bool
BDBG_P_InstTestAndPrint_isrsafe(BDBG_Level level, BDBG_pDebugModuleFile dbg_module, BDBG_Instance handle, const char *fmt, ...)
{
   struct BDBG_DebugInstModule *pInstanceModule = NULL;
   struct BDBG_DebugModuleInst *instance;
   bool module_result = BDBG_P_TestModule_isrsafe(dbg_module, level);
   unsigned instance_result;

   BDBG_P_Lock_isrsafe();
   instance = BDBG_P_GetInstance_sync_isrsafe(handle);

   if (instance) {
      BDBG_pDebugModuleFile pModule = BDBG_P_GetModuleByName_sync_isrsafe(dbg_module->name, dbg_module);
      if(pModule) {
        pInstanceModule = BDBG_P_GetInstanceModule_sync_isrsafe(instance, pModule);
      }
   }
   BDBG_P_Unlock_isrsafe();

   instance_result = ((instance && level >= instance->level) || (pInstanceModule && level >= pInstanceModule->eLevel)) ? BDBG_P_INSTANCE_BIT : 0;
   if(module_result || instance_result) {
      union {
         char timeStamp[16];
         va_list ap;
      } u; /* use union to merge two fields on stack */
#ifdef BDBG_ANDROID_LOG
        if ( fmt ) {
          android_LogPriority prio = BDBG_P_Level2Android(level);
          if(instance && instance->name) {
            LOG_PRI(prio, "NEXUS", "%s(%s): ", dbg_module->name, instance->name);
          } else {
            LOG_PRI(prio, "NEXUS", "%s(%p): ", dbg_module->name, handle);
          }
          va_start(u.ap, fmt);
          LOG_PRI_VA(prio, "NEXUS", fmt, u.ap);
          va_end(u.ap);
        }
        else {
#endif
      BDBG_P_GetTimeStamp_isrsafe(u.timeStamp, sizeof(u.timeStamp));
      if(instance && instance->name) {
         BDBG_P_PrintHeader_isrsafe(instance_result | level, dbg_module, "%s %s %s(%s): ", BDBG_P_GetPrefix_isrsafe(level), u.timeStamp, dbg_module->name, instance->name);
      } else {
         BDBG_P_PrintHeader_isrsafe(instance_result | level, dbg_module, "%s %s %s(%p): ", BDBG_P_GetPrefix_isrsafe(level), u.timeStamp, dbg_module->name, (void *)handle);
      }
      if(fmt) { /* also print body */

          va_start(u.ap, fmt);
          BDBG_P_VprintBody_isrsafe(instance_result, level, dbg_module, fmt, u.ap);
          va_end( u.ap );
      }
#ifdef BDBG_ANDROID_LOG
      }
#endif
      return true;
   }
   return false;
}

static BDBG_pDebugModuleFile
BDBG_P_GetModule(BDBG_pDebugModuleFile module)
{
   /* This return value is not checked intentionally */
   /* coverity[check_return] */
   /* coverity[unchecked_value] */
   BDBG_P_TestModule_isrsafe(module, BDBG_eTrace);
   return module;
}

void
BDBG_P_RegisterInstance(BDBG_Instance handle, BDBG_pDebugModuleFile dbg_module)
{
   BDBG_pDebugModuleFile module;
   struct BDBG_DebugModuleInst *instance = NULL;
   struct BDBG_DebugModuleInst *previous, *current;
   struct BDBG_DebugInstModule *pInstanceModule = NULL;

   module = BDBG_P_GetModule(dbg_module);
   if (!module) {
      return;
   }

   /* PR56629: Search for and re-use the previously
    * unregistered instance to prevent memory leak */
   BDBG_P_Lock_isrsafe();
   for(current = BLST_S_FIRST(&gDbgState.unregistered_instances); current ; current = BLST_S_NEXT(current, link))
   {
      if ( ( current->module == module )
           && (current->handle == handle ) )
      {
         /* We found a previously unregistered instance */
         BLST_S_REMOVE(&gDbgState.unregistered_instances, current, BDBG_DebugModuleInst, link);
         instance = current;

         /* Remove and free each module instance */
         while( (pInstanceModule = BLST_S_FIRST(&instance->modules)) != NULL ) {
            BLST_S_REMOVE_HEAD(&instance->modules,link);
            BDBG_P_Unlock_isrsafe();
            BKNI_Free(pInstanceModule);
            BDBG_P_Lock_isrsafe();
         }

         break;
      }
   }

   if ( NULL == instance )
   {
      BDBG_P_Unlock_isrsafe();
      instance = BKNI_Malloc(sizeof(*instance));
      if(!instance) {
         /* too bad */
         return;
      }
      BDBG_P_Lock_isrsafe();
   }

   instance->module = module;
   instance->name = NULL;
   instance->level = module->module_level;
   instance->handle = handle;
   BLST_S_INIT(&instance->modules);

   /* 1. Find spot in the sorted list */
   for(previous=NULL, current = BLST_S_FIRST(&gDbgState.instances); current ; current = BLST_S_NEXT(current, link)) {
      if ((uint8_t *)current->handle >= (uint8_t *)handle) {
         break;
      }
      previous=current;
   }
   if(previous) {
      BLST_S_INSERT_AFTER(&gDbgState.instances, previous, instance, link);
   } else {
      BLST_S_INSERT_HEAD(&gDbgState.instances, instance, link);
   }
   BDBG_P_Unlock_isrsafe();

   return;
}

void
BDBG_P_UnRegisterInstance(BDBG_Instance handle, BDBG_pDebugModuleFile dbg_module)
{
   struct BDBG_DebugModuleInst *instance;

   BSTD_UNUSED(dbg_module);

   instance = BDBG_P_GetInstance(handle);
   if(instance) {
      BDBG_P_Lock_isrsafe();
      BLST_S_REMOVE(&gDbgState.instances, instance, BDBG_DebugModuleInst, link);

      /* Add instance to unregistered instance list to be freed later
       * during BDBG_Uninit() */
      BLST_S_INSERT_HEAD(&gDbgState.unregistered_instances, instance, link);
      BDBG_P_Unlock_isrsafe();
   }
   return;
}

static void
BDBG_P_EnterLeaveFunction_isrsafe(BDBG_pDebugModuleFile dbg_module, const char *function, const char *kind)
{
   if(BDBG_P_TestModule_isrsafe(dbg_module, BDBG_eTrace)) {
      char timeStamp[16];
      BDBG_P_GetTimeStamp_isrsafe(timeStamp, sizeof(timeStamp));

      BDBG_P_PrintTrace_isrsafe(dbg_module, "%s %s %s: %s", gDbgPrefix[BDBG_eTrace], timeStamp, kind, function);
   }
}

void
BDBG_EnterFunction_isrsafe(BDBG_pDebugModuleFile dbg_module, const char *function)
{
   BDBG_P_EnterLeaveFunction_isrsafe(dbg_module, function, "Enter");
   return;
}

void
BDBG_LeaveFunction_isrsafe(BDBG_pDebugModuleFile dbg_module, const char *function)
{
   BDBG_P_EnterLeaveFunction_isrsafe(dbg_module, function, "Leave");
   return;
}


BERR_Code
BDBG_SetLevel(BDBG_Level level)
{
   BERR_Code rc = BDBG_P_CheckDebugLevel(level);
   BDBG_pDebugModuleFile module;

   if (rc!=BERR_SUCCESS) {
      return BERR_TRACE(rc);
   }

   gDbgState.level = level;
   /* traverse all known modules */
   BDBG_P_Lock_isrsafe();
   for(module = BLST_S_FIRST(&gDbgState.modules); module ; module = BLST_S_NEXT(module, link)) {
      module->level = (level < module->module_level)?level:module->module_level; /* update current module level with smallest of global level and module level */
   }
   BDBG_P_Unlock_isrsafe();
   return BERR_SUCCESS;
}

BERR_Code
BDBG_GetLevel(BDBG_Level *level)
{
   *level = gDbgState.level;
   return BERR_SUCCESS;
}



BERR_Code
BDBG_SetInstanceLevel(BDBG_Instance handle, BDBG_Level level)
{
   struct BDBG_DebugModuleInst *instance;

   BERR_Code rc = BDBG_P_CheckDebugLevel(level);

   if (rc!=BERR_SUCCESS) {
      return BERR_TRACE(rc);
   }

   instance = BDBG_P_GetInstance(handle);
   if (!instance) {
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }
   instance->level = level;

   return BERR_SUCCESS;
}

BERR_Code
BDBG_GetInstanceLevel(BDBG_Instance handle, BDBG_Level *level)
{
   struct BDBG_DebugModuleInst *instance;

   instance = BDBG_P_GetInstance(handle);
   if (!instance) {
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }
   *level = instance->level;

   return BERR_SUCCESS;
}

BERR_Code
BDBG_SetInstanceName(BDBG_Instance handle, const char *name)
{
   struct BDBG_DebugModuleInst *instance;

   instance = BDBG_P_GetInstance(handle);
   if (!instance) {
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }
   instance->name = name;

   return BERR_SUCCESS;
}

static struct BDBG_DebugInstModule *
BDBG_P_GetInstanceModule_sync_isrsafe(struct BDBG_DebugModuleInst *pInstance, BDBG_pDebugModuleFile pModule)
{
   struct BDBG_DebugInstModule * pInstanceModule;
   for(pInstanceModule = BLST_S_FIRST(&pInstance->modules); pInstanceModule ; pInstanceModule = BLST_S_NEXT(pInstanceModule, link)) {
      if (pInstanceModule->pModule >= pModule) break;
   }
   if (pInstanceModule && (pInstanceModule->pModule != pModule)) pInstanceModule = NULL;
   return pInstanceModule;
}

static struct BDBG_DebugInstModule *
BDBG_P_GetInstanceModule(struct BDBG_DebugModuleInst *pInstance, BDBG_pDebugModuleFile pModule)
{
   struct BDBG_DebugInstModule * pInstanceModule;

   BDBG_P_Lock_isrsafe();
   pInstanceModule = BDBG_P_GetInstanceModule_sync_isrsafe(pInstance, pModule);
   BDBG_P_Unlock_isrsafe();
   return pInstanceModule;
}

BERR_Code
BDBG_P_SetInstanceModuleLevel(struct BDBG_DebugModuleInst *pInstance, BDBG_pDebugModuleFile pModule, BDBG_Level eLevel)
{
   struct BDBG_DebugInstModule *pInstanceModule, *pCurInstanceModule, *pPrevInstanceModule;

   pInstanceModule = BDBG_P_GetInstanceModule(pInstance, pModule);

   if (pInstanceModule == NULL)
   {
      /* Add instance to instance module list */
      pInstanceModule = BKNI_Malloc(sizeof(struct BDBG_DebugInstModule));
      if (pInstanceModule)
      {
         pInstanceModule->pModule = pModule;

         BDBG_P_Lock_isrsafe();

         /* 1. Find spot in the sorted list */
         for(pPrevInstanceModule=NULL, pCurInstanceModule = BLST_S_FIRST(&pInstance->modules); pCurInstanceModule ; pCurInstanceModule = BLST_S_NEXT(pCurInstanceModule, link)) {
            if ((uint8_t *)pCurInstanceModule->pModule >= (uint8_t *)pModule) {
               break;
            }
            pPrevInstanceModule=pCurInstanceModule;
         }
         if(pPrevInstanceModule) {
            BLST_S_INSERT_AFTER(&pInstance->modules, pPrevInstanceModule, pInstanceModule, link);
         } else {
            BLST_S_INSERT_HEAD(&pInstance->modules, pInstanceModule, link);
         }

         BDBG_P_Unlock_isrsafe();
      }
   }

   if (pInstanceModule != NULL)
   {
      pInstanceModule->eLevel = eLevel;
      return BERR_SUCCESS;
   }
   else
   {
      return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }
}

BERR_Code
BDBG_GetModuleInstanceLevel(const char *name, BDBG_Instance handle, BDBG_Level *level)
{
   struct BDBG_DebugModuleInst *pInstance;
   BDBG_pDebugModuleFile pModule;
   struct BDBG_DebugInstModule *pInstanceModule;

   /* Find the instance struct using the handle */
   pInstance = BDBG_P_GetInstance(handle);
   if (!pInstance)
   {
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   /* Find the module struct using the name */
   pModule = BDBG_P_GetModuleByName(name);
   if (!pModule) {
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   /* Fine the instance module struct using the instance and module */
   pInstanceModule = BDBG_P_GetInstanceModule(pInstance, pModule);
   if (!pInstanceModule)
   {
      /* An instance module level wasn't set, so we return the
       * instance's level instead */
      *level = pInstance->level;
   }
   else
   {
      *level = pInstanceModule->eLevel;
   }

   return BERR_SUCCESS;
}

BERR_Code
BDBG_SetModuleInstanceLevel(const char *name, BDBG_Instance handle, BDBG_Level level)
{
   struct BDBG_DebugModuleInst *pInstance;
   BDBG_pDebugModuleFile pModule;

   /* Find the instance struct using the handle */
   pInstance = BDBG_P_GetInstance(handle);
   if (!pInstance)
   {
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   /* Find the module struct using the name */
   pModule = BDBG_P_GetModuleByName(name);
   if (!pModule) {
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   /* Set instance module level using the instance and module */
   return BDBG_P_SetInstanceModuleLevel(pInstance, pModule, level);
}

BERR_Code
BDBG_SetModuleLevel(const char *name, BDBG_Level level)
{
   BDBG_pDebugModuleFile module;
   struct BDBG_DebugModuleInst *pInstance;
   char module_name[BDBG_P_MAX_MODULE_NAME_LENGTH];
   char *instance_name = NULL;
   BERR_Code rc = BDBG_P_CheckDebugLevel(level);

   if (rc!=BERR_SUCCESS) {
      return BERR_TRACE(rc);
   }

   BDBG_P_StrnCpy(module_name, name, BDBG_P_MAX_MODULE_NAME_LENGTH);
   instance_name = BDBG_P_StrChrNul(module_name, ':');
   if (*instance_name != '\0')
   {
      *instance_name = '\0';
      instance_name++;
   }

   pInstance = BDBG_P_GetInstanceByName(instance_name);

   if (pInstance)
   {
      module = BDBG_P_GetModuleByName(module_name);
      if (!module) {
         return BERR_TRACE(BERR_INVALID_PARAMETER);
      }

      BDBG_P_SetInstanceModuleLevel(pInstance, module, level);
   }
   else
   {
      module = BDBG_P_GetModuleByName(name);
      if (!module) {
         return BERR_TRACE(BERR_INVALID_PARAMETER);
      }

      /* We set the module level */
      /* traverse all files of given module */
      BDBG_P_Lock_isrsafe();
      for(; module ; module = BLST_S_NEXT(module, link)) {
         if(BDBG_P_StrCmp_isrsafe(name, module->name)!=0) {
            break;
         }
         module->module_level = level;
         module->level = (gDbgState.level < level)?gDbgState.level:level; /* update current module level with smallest  of global level and module level */
      }
      BDBG_P_Unlock_isrsafe();
   }

   return BERR_SUCCESS;
}

BERR_Code
BDBG_GetModuleLevel(const char *name, BDBG_Level *level)
{
   BDBG_pDebugModuleFile module;
   struct BDBG_DebugModuleInst *pInstance;
   struct BDBG_DebugInstModule *pInstanceModule = NULL;
   char module_name[BDBG_P_MAX_MODULE_NAME_LENGTH];
   char *instance_name = NULL;

   BDBG_P_StrnCpy(module_name, name, BDBG_P_MAX_MODULE_NAME_LENGTH);
   instance_name = BDBG_P_StrChrNul(module_name, ':');
   if (*instance_name != '\0')
   {
      *instance_name = '\0';
      instance_name++;
   }

   pInstance = BDBG_P_GetInstanceByName(instance_name);

   if (pInstance)
   {
      module = BDBG_P_GetModuleByName(module_name);
      if (!module) {
         return BERR_TRACE(BERR_INVALID_PARAMETER);
      }

      /* Fine the instance module struct using the instance and module */
      pInstanceModule = BDBG_P_GetInstanceModule(pInstance, module);
      if (!pInstanceModule)
      {
         /* An instance module level wasn't set, so we return the
          * instance's level instead */
         *level = pInstance->level;
      }
      else
      {
         *level = pInstanceModule->eLevel;
      }
   }
   else
   {
      module = BDBG_P_GetModuleByName(name);
      if (!module) {
         return BERR_TRACE(BERR_INVALID_PARAMETER);
      }
      *level = module->level;
   }
   return BERR_SUCCESS;
}

BERR_Code
BDBG_Init(void)
{
   BDBG_P_InitializeTimeStamp();
   return BDBG_P_OsInit();
}

void
BDBG_Uninit(void)
{
   BDBG_pDebugModuleFile module ;
   struct BDBG_DebugModuleInst *instance;
   struct BDBG_DebugInstModule *pInstanceModule = NULL;

   while( (module = BLST_S_FIRST(&gDbgState.modules)) != NULL ) {
      BLST_S_REMOVE_HEAD(&gDbgState.modules,link);
      if (module->module_alloc) {
         BKNI_Free(module);
      }
   }
   while( (instance = BLST_S_FIRST(&gDbgState.instances)) != NULL ) {
      BLST_S_REMOVE_HEAD(&gDbgState.instances,link);

      /* Remove and free each module instance */
      while( (pInstanceModule = BLST_S_FIRST(&instance->modules)) != NULL ) {
         BLST_S_REMOVE_HEAD(&instance->modules,link);
         BKNI_Free(pInstanceModule);
      }

      BKNI_Free(instance);
   }

   while( (instance = BLST_S_FIRST(&gDbgState.unregistered_instances)) != NULL ) {
      BLST_S_REMOVE_HEAD(&gDbgState.unregistered_instances,link);

      /* Remove and free each module instance */
      while( (pInstanceModule = BLST_S_FIRST(&instance->modules)) != NULL ) {
         BLST_S_REMOVE_HEAD(&instance->modules,link);
         BKNI_Free(pInstanceModule);
      }

      BKNI_Free(instance);
   }
   BDBG_P_Dequeue_FreeMemory();

   BDBG_P_OsUninit();

   return;
}

/* BDBG_OBJECT_ID(bdbg_invalid); */
static const char bdbg_id__bdbg_invalid[]="invalid";


void
BDBG_Object_Init_isrsafe(void *ptr, size_t size, struct bdbg_obj *obj, const char *id)
{
   unsigned i;

   if ( NULL == id )
   {
       id = bdbg_id__bdbg_invalid;
   }

   for(i=0; i+3 < size; i+=4) {
      *(uint32_t*)(((uint8_t *)ptr)+i)=0xDEADBEEF;
   }
   obj->bdbg_obj_id=id;
   return;
}

void
BDBG_Object_Assert_isrsafe(const void *ptr, size_t size, const struct bdbg_obj *obj, const char *id, const char *file, unsigned line) {

    BSTD_UNUSED(size);

    if (ptr && obj->bdbg_obj_id==id) {
        return;
    }

#if BKNI_TRACK_MALLOCS
    if(ptr) {
        BKNI_MallocEntryInfo entry;

        if(BKNI_GetMallocEntryInfo_isrsafe(ptr, &entry)==BERR_SUCCESS) {
            gDbgState.assertMessage[0] = '\0';
            if(!entry.alive) {
                BKNI_Snprintf(gDbgState.assertMessage, sizeof(gDbgState.assertMessage), "and freed at %s:%u", entry.free_file, entry.free_line);
            }
            BDBG_P_PrintString_isrsafe("BDBG_OBJECT_ASSERT on object %p (%u:%u bytes) was allocated at %s:%u %s\n", ptr, (unsigned)entry.size, (unsigned)size, entry.malloc_file, entry.malloc_line, gDbgState.assertMessage);
        }
    }
#endif
    gDbgState.assertMessage[0] = '\0';
    if(ptr==NULL) {
        BKNI_Snprintf(gDbgState.assertMessage, sizeof(gDbgState.assertMessage), "NULL pointer was used as %s", id);
    } else {
        if (obj->bdbg_obj_id == bdbg_id__bdbg_invalid) {
            BKNI_Snprintf(gDbgState.assertMessage, sizeof(gDbgState.assertMessage), "Recycled pointer was used %s:%p", id, ptr);
        } else {
            /* This can be caused by a closed or otherwise invalid handle */
            BKNI_Snprintf(gDbgState.assertMessage, sizeof(gDbgState.assertMessage), "Bad object of expected type %s:%p (%p:%p)", id, ptr, obj->bdbg_obj_id, id);
        }
    }

    BDBG_P_AssertFailed_isrsafe(gDbgState.assertMessage, file, line);
    return;
}

void
BDBG_EnumerateAll(void (*callback)(void *cntx, const char *module, BDBG_Instance instance, const char *inst_name), void *cntx)
{
   struct BDBG_DebugModuleInst *instance=NULL;
   BDBG_pDebugModuleFile module;
   const char *last_name;

   BDBG_P_Lock_isrsafe();
   for(last_name=NULL, module = BLST_S_FIRST(&gDbgState.modules); module ; module = BLST_S_NEXT(module, link)) {
      if(last_name==NULL || BDBG_P_StrCmp_isrsafe(last_name, module->name)!=0) {
         last_name = module->name;
         callback(cntx, module->name, NULL, NULL);
      }
   }
   for(instance = BLST_S_FIRST(&gDbgState.instances); instance ; instance = BLST_S_NEXT(instance, link)) {
      callback(cntx, instance->module->name, instance->handle, instance->name);
   }
   BDBG_P_Unlock_isrsafe();
   return ;
}

void
BDBG_P_Release(BDBG_pDebugModuleFile dbg_module)
{
   BDBG_P_Lock_isrsafe();
   if(dbg_module->level!=BDBG_P_eUnknown) {
        BLST_S_REMOVE(&gDbgState.modules, dbg_module, BDBG_DebugModuleFile, link);
        dbg_module->level = BDBG_P_eUnknown;
   }
   BDBG_P_Unlock_isrsafe();
   return;
}


BERR_Code
BDBG_SetModulePrintFunction(const char *name, BDBG_DebugModule_Print module_print)
{
#if defined(BDBG_P_UNWRAP)
    BDBG_pDebugModuleFile module;

    module = BDBG_P_GetModuleByName(name);
    if (!module) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* We set the module level */
    /* traverse all files of given module */
    BDBG_P_Lock_isrsafe();
    for(; module ; module = BLST_S_NEXT(module, link)) {
        if(BDBG_P_StrCmp_isrsafe(name, module->name)!=0) {
            break;
        }
        module->level = (gDbgState.level < module->module_level)?gDbgState.level:module->module_level; /* update current module level with smallest  of global level and module level */
        module->module_print = module_print;
        if(module_print) {
            module->level = -module->level; /* make it negative so it'd always pass test */
        }
    }
    BDBG_P_Unlock_isrsafe();

    return BERR_SUCCESS;
#else
    BSTD_UNUSED(name);
    BSTD_UNUSED(module_print);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}


#define BDBG_P_CONTEXT_THRESHOLD    512

static BDBG_P_Dequeue_Context *BDBG_P_Dequeue_FindContext(unsigned long tag)
{
    unsigned i;
    for(i=0;i<gDbgState.dequeueState.last_used;i++) {
        BDBG_P_Dequeue_Context *context = gDbgState.dequeueState.contexts+i;
        if(context->used) {
            long diff = tag - context->tag;
            if( -BDBG_P_CONTEXT_THRESHOLD < diff && diff < BDBG_P_CONTEXT_THRESHOLD) {
                return context;
            }
        }
    }
    return NULL;
}

static BDBG_P_Dequeue_Context *BDBG_P_Dequeue_FindFree(unsigned hdr_len)
{
    unsigned i;
    BDBG_P_Dequeue_Context *context=NULL;

    for(i=0;i<gDbgState.dequeueState.last_used;i++) {
        BDBG_P_Dequeue_Context *cur_context = gDbgState.dequeueState.contexts+i;
        if(!cur_context->used) {
            context = cur_context;
            break;
        }
    }
    if(context == NULL) {
        if(gDbgState.dequeueState.last_used >= BDBG_P_DEQUEUE_MAX_CONTEXTS) {
            return NULL;
        }
        context = gDbgState.dequeueState.contexts+gDbgState.dequeueState.last_used;
        gDbgState.dequeueState.last_used++;
        context->used = false;
        context->tag = 0;
        context->header_buf = NULL;
        context->header_buf_size = 0;
    }
    if(context->header_buf_size < hdr_len)  {
        if(context->header_buf) {
            context->header_buf_size = 0;
            BKNI_Free(context->header_buf);
        }
        context->header_buf = BKNI_Malloc(hdr_len);
        if(context->header_buf==NULL) {
            return NULL;
        }
        context->header_buf_size = hdr_len;
    }
    return context;
}

static void BDBG_P_Dequeue_FreeMemory(void)
{
    unsigned i;
    for(i=0;i<gDbgState.dequeueState.last_used;i++) {
        BDBG_P_Dequeue_Context *context = gDbgState.dequeueState.contexts+i;
        if(context->header_buf) {
            BKNI_Free(context->header_buf);
            context->used = false;
            context->header_buf = NULL;
            context->header_buf_size = 0;
        }
    }
    gDbgState.dequeueState.nodata_count = 0;
    gDbgState.dequeueState.last_used = 0;
    return;
}

typedef struct BDBG_P_StrBuf {
    char *str;
    size_t size;
    unsigned len;
}BDBG_P_StrBuf;

static void BDBG_P_StrBuf_Printf( BDBG_P_StrBuf *buf, const char *fmt, ...) BDBG_P_PRINTF_FORMAT(2, 3);

static void BDBG_P_StrBuf_Printf( BDBG_P_StrBuf *buf, const char *fmt, ...)
{
    int rc;
    size_t size = buf->size - buf->len;
    va_list ap;
    va_start(ap, fmt);
    rc = BKNI_Vsnprintf(buf->str+buf->len, size, fmt, ap);
    va_end( ap );
    if(rc>0) {
        buf->len = (unsigned)rc<size  ? buf->len + rc : buf->size - 1;
    }
    return;
}

#if 0
static void BDBG_P_StrBuf_PrintChar( BDBG_P_StrBuf *buf, char c)
{
    if(buf->len + 1 > buf->size) {
        buf->str[buf->len]=c;
        buf->len++;
        buf->str[buf->len]='\0';
    }
    return;
}
#endif

static void BDBG_P_StrBuf_Memcpy( BDBG_P_StrBuf *buf, const void *b, size_t len)
{
    if(buf->len + len + 1 < buf->size) {
        /* buffer large enough, do nothing, keep going */
    } else if(buf->len + 1 < buf->size) {
        /* there is a space for at least one character */
        len = buf->size  - (buf->len + 1);
    } else {
        /* there is no space */
        return;
    }
    BKNI_Memcpy(buf->str+buf->len, b, len);
    buf->len += len;
    buf->str[buf->len]='\0';
    return;
}

static void BDBG_P_StrBuf_AddLog( BDBG_P_StrBuf *buf, const BDBG_P_LogEntry *log)
{
    size_t len;
    if(log->rc>0) {
        len = (unsigned)log->rc < sizeof(log->str) ? (unsigned)log->rc : sizeof(log->str)-1;
        BDBG_P_StrBuf_Memcpy(buf, log->str, len);
    }
}

static void BDBG_P_Dequeue_Flush(BDBG_P_StrBuf *buf)
{
    unsigned i;

    BDBG_P_StrBuf_Printf(buf, "___ OVERFLOW ___");
    for(i=0;i<gDbgState.dequeueState.last_used;i++) {
        BDBG_P_Dequeue_Context *context = gDbgState.dequeueState.contexts+i;
        if(context->used) {
            BDBG_P_StrBuf_Printf(buf, "\n___ %s ### OVERFLOW ###", context->header_buf);
            context->used = false;
        }
    }
    return;
}

static void
BDBG_P_Dequeue_FilterString(char *str)
{
    /* remove control character */
    for(;;) {
        char ch = *str;
        if(ch=='\0') {
            break;
        }
        switch(ch) {
        case '\r':
        case '\n':
        case '\b':
        case '\f':
        case '\a':
        case 27: /* escape */
        case 127: /* delete */
            *str = ' ';
            break;
        default:
            break;
        }
        str++;
    }
}

BERR_Code 
BDBG_Log_Dequeue(BDBG_FifoReader_Handle logReader, unsigned *timeout, char *str, size_t str_size, size_t *str_len)
{
    BERR_Code rc;
    BDBG_ModulePrintKind kind;
    BDBG_P_LogEntry logEntry;
    BDBG_P_Dequeue_Context *context;
    BDBG_P_StrBuf buf;
    unsigned hdr_len;

    BDBG_ASSERT(timeout);
    BDBG_ASSERT(str);
    BDBG_ASSERT(str_len);

    *timeout = 0;
    *str_len = 0;
    *str = '\0';
    buf.str = str;
    buf.len = 0;
    buf.size = str_size;
    rc = BDBG_FifoReader_Read(logReader, &logEntry, sizeof(logEntry));
    switch(rc) {
    case BERR_SUCCESS:
        break;
    case BERR_FIFO_NO_DATA:
        *timeout = 5;
        if(gDbgState.dequeueState.nodata_count<16) {
            gDbgState.dequeueState.nodata_count++;
            if(gDbgState.dequeueState.nodata_count<4) {
                *timeout = 0;
            }
            *timeout = 1;
        }
        return BERR_SUCCESS;
    case BERR_FIFO_BUSY:
        *timeout = 1;
        return BERR_SUCCESS;
    case BERR_FIFO_OVERFLOW:
        BDBG_P_Dequeue_Flush(&buf);
        *str_len = buf.len;
        BDBG_FifoReader_Resync(logReader);
        gDbgState.dequeueState.nodata_count = 0;
        return BERR_SUCCESS;
    default:
        return BERR_TRACE(rc);
    }
    gDbgState.dequeueState.nodata_count = 0;
    kind = logEntry.tag & 0x03;
    switch(kind) {
    case BDBG_ModulePrintKind_eString:
    case BDBG_ModulePrintKind_eHeaderAndBody:
        BDBG_P_Dequeue_FilterString(logEntry.str);
        BDBG_P_StrBuf_AddLog(&buf, &logEntry);
        break;
    case BDBG_ModulePrintKind_eHeader:
        context = BDBG_P_Dequeue_FindContext(logEntry.tag);
        if(context) {
            BDBG_P_StrBuf_Printf(&buf, "___ %s ### MISSING ###", context->header_buf);
            context->used = false;
        }
        if(logEntry.rc>0) {
            hdr_len = (unsigned)logEntry.rc < sizeof(logEntry.str) ? (unsigned)logEntry.rc +1 : sizeof(logEntry.str);
            context = BDBG_P_Dequeue_FindFree(hdr_len);
            if(context) {
                context->hdr_len = hdr_len;
                context->used = true;
                context->tag = logEntry.tag;
                BKNI_Memcpy(context->header_buf, logEntry.str, hdr_len);
            }
        } 
        break;
    case BDBG_ModulePrintKind_eBody:
        context = BDBG_P_Dequeue_FindContext(logEntry.tag);
        if(context) {
            BDBG_P_Dequeue_FilterString(logEntry.str);
            BDBG_P_StrBuf_Memcpy(&buf, context->header_buf, context->hdr_len-1);
            BDBG_P_StrBuf_AddLog(&buf, &logEntry);
            context->used = false;
        } else {
            BDBG_P_StrBuf_Printf(&buf, "___ ### MISSING ### %s", logEntry.str);
        }
        break;
    /* default statement is intentional */
    /* coverity[dead_error_begin] */
    default:
        break;
    }
    *str_len = buf.len;
    return BERR_SUCCESS;
}

void 
BDBG_Log_GetElementSize( size_t *elementSize )
{
    BDBG_ASSERT(elementSize);
    *elementSize = sizeof(BDBG_P_LogEntry);
    return;
}

void
BDBG_Log_SetFifo(BDBG_Fifo_Handle fifo)
{
    gDbgState.dbgLog = fifo;
    return;
}

const char *
BDBG_GetPrintableFileName(const char *pFileName)
{
    const char *s;
    unsigned i;

    if(pFileName==NULL) {
        return "";
    }
    for(s=pFileName;*s != '\0';s++) { } /* search forward */

    for(i=0;s!=pFileName;s--) { /* search backward */
        if(*s=='/' || *s=='\\') {
            i++;
            if(i>3) {
                return s+1;
            }
        }
    }
    return pFileName;
}

const char *BDBG_P_Int64DecArg(int64_t x, char *buf, size_t buf_size)
{
    const int base = 10;
    int i;
    bool negative;

    negative = x<0;
    if(negative) {
        x = -x;
    }
    i=buf_size-1;
    if(i>=0) {
        buf[i]='\0';
        i--;
    }
    for(;i>=0;) {
        unsigned index = x % base;
        x = x / base;
        buf[i] = index + '0';
        i--;
        if(x==0) {
            break;
        }
    }
    if(i<0) {
        return "[OVF]";
    }
    if(negative) {
        if(i==0) {
            return "[OVF]";
        }
        buf[i]='-';
        i--;
    }
    return buf+i+1;
}

#if B_REFSW_DEBUG_COMPACT_ERR
void
BDBG_P_Assert_isrsafe(bool expr, const char *file, unsigned line)
{
    if (expr) return;
    BDBG_P_AssertFailed_isrsafe(NULL, file, line);
}
#endif

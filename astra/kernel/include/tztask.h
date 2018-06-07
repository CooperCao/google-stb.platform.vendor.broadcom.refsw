/***************************************************************************
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
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ***************************************************************************/
#ifndef TASK_H_
#define TASK_H_

#include <cstdint>

#include "arm/arm.h"
#include "pgtable.h"
#include "elfimage.h"
#include "fs/fs.h"
#include "waitqueue.h"
#include "clock.h"
#include "utils/vector.h"
#include "cpulocal.h"
#include "hwtimer.h"
#include "tzmqueue.h"

#include "ldt.h"
#include "poll.h"
#include "tzsignal.h"
#include "tzfcntl.h"

#include "lib_printf.h"

typedef int (*TaskFunction)(void *task, void *ctx);
typedef void (*SignalHandler)(int);
typedef void (*SignalRestorer)();

extern "C" void yieldCurrentTask(unsigned int, unsigned int);
extern "C" void resumeCurrentTask();

class ProcessGroup {
private:
    void *operator new (size_t sz);
    void operator delete (void *pgrp);
    ProcessGroup(int pgid);
    ProcessGroup() = delete;
    ~ProcessGroup();

public:
    int signalGroup(int sigNum);
    int getIdxFromTid(unsigned int tid);
    int getPgid() const { return pgid; };

    static void init();
    static ProcessGroup * create(int pgid);
    static ProcessGroup * lookUp(int pgid);
    static int addGroupMember(int pgid, TzTask *);
    static int removeGroupMember(int pgid, TzTask *);
    static int signalGroup(int pgid, int sigNum);

private:
    int pgid;
    tzutils::Vector<TzTask *> taskList;

private:
    static tzutils::Vector<ProcessGroup *> pgList;
    static ObjCacheAllocator<ProcessGroup> pgAllocator;
};

class TzTask {
public:
    enum State {
        Init,
        Ready,
        Running,
        Wait,
        Defunct
    };

    enum Mode {
        User,
        Kernel
    };

    enum Type {
        CFS_Task = SCHED_OTHER,                 /* Completely Fair Scheduling Task */
        EDF_Task = SCHED_DEADLINE,              /* Deadline Scheduling Task */
        WORKHORSE_RR_TASK = SCHED_RR,           /* WorkHorse Task Preemptible - Round Robin */
        WORKHORSE_FIFO_TASK = SCHED_FIFO,       /* WorkHorse Task Non-Preemptible - FIFO */
    };

    ARCH_SPECIFIC_USER_REG_LIST;

    enum FTEType {
        File,
        Directory,
        IpcPipe,
        MQueue,
        Socket
    };

    struct FileTableEntry {
        FTEType type;
        uintptr_t iNo;
        union {
            IFile *file;
            IDirectory *dir;
            MsgQueue *queue;
            void *data;
        };
        uint64_t offset;
        bool read;
        bool write;
        bool closeOnExec;
    };

    typedef unsigned int IDType;

    static const unsigned int MAX_FD = 32;

    static const unsigned int MAX_NUM_TLS_ENTRIES = 16;

    static const unsigned int THREAD_INFO_OFFSET = 1024;

    static const unsigned int BRK_MAX = 4*1024*1024;
    static const unsigned int MMAP_MAX = 8*1024*1024;

    enum DefaultSigAction {
        Terminate,
        CoreDump,
        Ignore,
        ContinueTask,
        StopTask
    };
    static const uint8_t SIGNAL_BLOCKED = 1;
    static const uint8_t SIGNAL_PENDING = (1 << 1);
    static const uint8_t SIGNAL_ACTIVE  = (1 << 2);

    struct Signal {
        SignalHandler handler;
        SignalRestorer restorer;
        DefaultSigAction defaultAction;

        sigset_t maskOnActive;
        int flags;

        siginfo_t sigInfo;
        bool haveSigInfo;

        uint8_t status;
    };

    struct SigReturn {
        unsigned long r0;
        unsigned long r1;
        unsigned long r2;
        unsigned long r3;
        unsigned long r12;

        unsigned long sp_usr;
        unsigned long lr_usr;
        unsigned long lr;

        int sigNum;
        siginfo_t sigInfo;
    };
    static const int NumSignals = SIGRTMAX + 1;

public:
    static void init();
    static void initSecondaryCpu();

    static void initNoTask();

    static void terminationLock();
    static void terminationUnlock();

    static TzTask *nwProxy() { return nwProxyTask.cpuLocal(); }

    static IDType nextTaskId();

public:

    // Use this constructor to create a kernel-only task.
    TzTask(TaskFunction entry, void *taskFuncCtxParam, unsigned int priority, const char *taskName = nullptr, TzTask *parent = nullptr);

    // Use this constructor to create a user-space process.
    TzTask(IFile *exeFile, unsigned int priority, IDirectory *workDir = nullptr, const char *taskName = nullptr, TzTask *parent = nullptr,
        char **argv = nullptr, char **envp = nullptr);

    // Use this constructor to fork() a process.
    TzTask(TzTask& forkFromTask);

    // Use this constructor to clone() a process/thread
    TzTask(TzTask& cloneFromTask, unsigned long flags, void *childStack, void *ptid, void *ctid, void *tls);

    inline unsigned long userReg(const UserRegs regName) const {
        unsigned long *base = savedRegBase - NUM_SAVED_CPU_REGS;
        return base[regName];
    }

    inline void writeUserReg(const UserRegs regName, const unsigned long value) {
        unsigned long *base = savedRegBase - NUM_SAVED_CPU_REGS;
        base[regName] = value;
    }

    static TzTask *current();

    PageTable* userPageTable() { return pageTable; }

    uint16_t owner() const { return uid; }
    int changeOwner(uint16_t newOwner);

    uint16_t group() const { return gid; }
    int changeGroup(uint16_t newGroup);

    unsigned int processGroup() const { return pgid; }
    int changeProcessGroup(unsigned int newPgid);

    IDirectory *currDir() { return currWorkDir; }
    int setCurrDir(const char *dirPath);
    int setCurrDir(int fd);

    friend void yieldCurrentTask(unsigned int, unsigned int);
    friend void resumeCurrentTask();
    friend void printReg();
    void yield();

    void dataAbortException();

    void run();

    void *operator new (size_t sz);
    void operator delete (void *task);

    IDType id() { return tid; }
    unsigned int getpriority() { return priority; }

    int execve(IFile *exeFile, IDirectory *exeDir, char **argv, char **envp);
    int wait4(int pid, int *status, int options);

    int access(char *path, int mode);
    int open(char *filePath, int flags, int mode );
    int read(int fd, void *userBuffer, size_t size);
    int write(int fd, void *userBuffer, size_t size);
    int close(int fd);
    int readdir(int fd, void *userBuffer, size_t size);
    int llseek(int fd, uint64_t offset, loff_t *result, unsigned int whence);
    int dup(int oldFd);
    int dup2(int oldFd, int newFd);
    int dup3(int oldFd, int newFd, long flags);
    int fstat(int fd, struct stat *statbuf);

    int chmod(const char *filePath, uint32_t mode);
    int fchmod(int fd, uint32_t mode);

    int chown(const char *path, uint16_t owner, uint16_t group);
    int fchown(int fd, uint16_t owner, uint16_t group);

    int nice(int incr);

    void *brk(void *newBrk);
    int mmap(TzMem::VirtAddr addr, TzMem::VirtAddr *allocated, size_t len, int prot, int flags);
    int mmap(TzMem::VirtAddr addr, TzMem::VirtAddr *allocated, size_t len, int prot, int flags, int fd, uint64_t offset);
    int munmap(TzMem::VirtAddr addr, size_t len);
    int mprotect(TzMem::VirtAddr addr, size_t len, int prot);

    int setThreadArea(struct user_desc *);
    int getThreadArea(struct user_desc *);
    int setTidAddress(int *tidPtr);

    int poll(struct pollfd *fds, nfds_t nfds, int timeout);
    ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
    ssize_t readv(int fd, const struct iovec *iov, int iovcnt);

    bool getScheduler(int *policy, int *schedPriority);
    bool setScheduler(int policy, int schedPriority);

    int pause();
    int sigprocmask(int how, const sigset_t *set, sigset_t *oldSet);
    int sigQueue(int sigNum, siginfo_t *siginfo=nullptr);
    int sigAction(int sigNum, k_sigaction *action, k_sigaction *old);
    void sigReturn();

    int mqOpen(const char *mqName, int oflag, int mode, mq_attr *attr);
    int mqSend(int fd, const char *msg, size_t msgSize, int prio, uint64_t timeout);
    int mqRecv(int fd, char *msg, size_t msgSize, int *prio, uint64_t timeout);
    int mqNotify(int fd, struct sigevent *ev);
    int mqAttrChange(int fd, mq_attr *newAttr, mq_attr *oldAttr);

    int pipeOpen(int *pfds, long flags);

    int setPgId(unsigned int pid, unsigned int newPgid);
    int getPgId(unsigned int pid);

    inline void enableSecCompStrict() { seccompStrict = true; }
    inline bool isSecCompStrict() { return seccompStrict; }

    inline bool dominates(TzTask *other) {
        if (other == nullptr)
            return true;
        if(other->keepAtQueueHead)
            return false;
        if(type == TzTask::Type::EDF_Task) {
            if(priority <= other->priority)
                return true;
            else
                return false;
        }
        //return ((totalRunTime * other->priority )<= (other->totalRunTime * priority));
        if(slotTimeSlice == other->slotTimeSlice)
            return (totalRunTime <= other->totalRunTime);
        else
            return (slotTimeSlice < other->slotTimeSlice);
    }

    inline uint64_t slot() { return slotTimeSlice; }
    inline uint64_t getRunValue() { return taskRunTime; }
    inline void setRunValue(uint64_t runTime) { taskRunTime = runTime; }

    inline uint64_t pqValue() { return totalRunTime; }

    inline State status() { return state; }

    inline bool terminated() { return (state == Defunct); }
    inline bool terminatedBySignal() { return (state == Defunct) && (termSignal > 0); }
    inline bool exited() { return (state == Defunct) && (termSignal < 0); }
    inline int exitCode() { return termCode; }
    inline int terminationSignal() { return termSignal; }
    inline bool isCloneThread() { return threadCloned; }
    void nanosleep(unsigned long ticks, unsigned long *remainTicks);

    TzClock::TaskClock *getTaskClock() { return &taskClock; }

    friend class WaitQueue;
    friend class EventQueue;
    friend class Scheduler;
    friend class TzClock::TaskClock;
    friend class SysCalls;

public:

    void terminate(int termCode = 0, int termSignal = -1);
    void awaken();
    bool signalDispatch();
    TzMem::VirtAddr GetThreadInfo() {return threadInfo; }
    void SetThreadInfo(uintptr_t p) {
        // Destroy the kernel TLS region if mapped
        if (threadInfo != nullptr) {
            TzMem::VirtAddr tiVa = PAGE_START_4K(threadInfo);
            TzMem::PhysAddr tiPa = kernPageTable->lookUp(tiVa);
            // Only if kernel TLS region is still in use (mapped)
            if (tiPa) {
                kernPageTable->unmapPage(tiVa);
                pageTable->unmapPage(tiVa);
                TzMem::freePage(tiPa);
            }
        }
        threadInfo = (void *)p;
#ifdef __aarch64__
        savedRegs[SAVED_REG_TPIDR_EL0] = (unsigned long)threadInfo;
#endif
    }
    static TzTask* taskFromId(int tid);

    // TzTask objects should only be allocated on the heap. Never on the stack.
    ~TzTask();

private:

    int waitAnyChild(int *status, int options);
    int waitAnyPgChild(int pgid, int *status, int options);
    int openDir(IDirectory *dir, int fd, bool closeOnExec);

    void initFileTable();
    void inheritFileTable(const TzTask& parent);

    void initSignalState();
    void terminateSignalState();
    void inheritSignalState(const TzTask& parent);
    void sendSigChldToParent(bool exited, int exitCode, int origSignal);

    void createUContext();
    void destroyUContext();
    void printURegs();
    void doCoreDump();
    void allocateStack(uint32_t size);
    void copyParentStack() {
        ElfImage* parentImg = parent->image;
        void * stackTopPage = PAGE_START_4K(parentImg->stackBase());
        void * stackAddrPage = PAGE_START_4K(parent->userReg(TzTask::UserRegs::sp_usr));
        int numOfPages = ((uintptr_t)stackTopPage - (uintptr_t)stackAddrPage) / PAGE_SIZE_4K_BYTES;
        image->allocAndCopyFrom(&parentImg->userPageTable, stackAddrPage, stackTopPage, numOfPages);
    }

public:

    class TaskStartInfo {
    public:
        static const int MAX_NUM_ARGS = 64;
        static const int MAX_NUM_ENVS = 128;
        static const int MAX_NUM_PROG_HEADERS = 8;
        static const int hwCaps = HWCAPS_AUXVAL;  //"arm v7 neon vfpv3 tls tzos"

    public:
        TaskStartInfo(ElfImage *image, const char *progName, int numArgs, char **argv, int numEnvs, char **envp);
        ~TaskStartInfo();

        int prepareUserStack(TzMem::VirtAddr userStack);

        bool constructed() { return state; }

    private:
        inline bool fits(char *curr, char *stop, const char *paramStr);
        inline void *toUser(void *va);

        ElfImage *image;
        char *progName;

        unsigned int numArgs;
        char *argv[MAX_NUM_ARGS];

        unsigned int numEnvs;
        char *envs[MAX_NUM_ENVS];

        unsigned int numProgHeaders;
        Elf_Phdr *phdrs[MAX_NUM_PROG_HEADERS];

        unsigned int *phwCaps;

        bool state;

    };

private:
    unsigned long savedRegs[NUM_SAVED_CPU_REGS * 2];
    unsigned long *savedRegBase;

    long double savedNeonRegs[NUM_SAVED_NEON_REGS * 2];
    long double *savedNeonRegBase;

    State state;
    Mode mode;
    Type type;

    PageTable *pageTable;
    PageTable *kernPageTable;
    TzMem::VirtAddr stackKernel;
    uint32_t stackKernelSize;
    TzMem::VirtAddr threadInfo;
    bool threadInfoCloned;

    unsigned int priority;
    uint64_t startedAt;
    uint64_t totalRunTime;
    uint64_t cumRunTime;
    uint64_t lastScheduledAt;
    uint64_t quantaRunTime;
    uint64_t slotTimeSlice;
    uint64_t taskRunTime;

    WaitQueue::WQState wqState;

    TzTask *parent;
    tzutils::Vector<TzTask *> children;
    WaitQueue childTermQueue;
    char taskName[8];

    ElfImage *image;
    bool vmCloned;
    bool threadCloned;

    IDirectory *currWorkDir;
    FileTableEntry *files;
    bool filesCloned;

    IDType tid;
    struct user_desc tlsEntry[MAX_NUM_TLS_ENTRIES];
    int *setChildTid;
    int *clearChildTid;

    Signal *signals;
    SigReturn *sigParamsStack;
    SigReturn *sigParamsStackUser;
    int sigParamsStackTop;
    int sigParamsStackMax;
    bool signalled;
    bool signalsCloned;

    int termCode;
    int termSignal;

    TzClock::TaskClock taskClock;
    Timer sleepTimer;
    uint64_t sleepUntil;

    uint16_t uid;
    uint16_t gid;

    unsigned int pgid;

    TzMem::VirtAddr brkStart;
    TzMem::VirtAddr brkCurr;
    TzMem::VirtAddr brkMax;
    TzMem::VirtAddr mmapMaxVa;

    TzMem::VirtAddr ucontext;
    TzMem::VirtAddr ucontextUserVa;

    TzMem::VirtAddr quickPages;
    bool quickPagesMapped;

    bool seccompStrict;

    SpinLock lock;

    bool keepAtQueueHead = false;
private:
    static SpinLock termLock;
    static tzutils::Vector<TzTask *>tasks;

    static PerCPU<TzTask *> nwProxyTask;
};

#endif /* TASK_H_ */

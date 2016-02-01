/***************************************************************************
//
//    Copyright 1999  Broadcom Corporation
//    All Rights Reserved
//    No portions of this material may be reproduced in any form without the
//    written permission of:
//             Broadcom Corporation
//             16251 Laguna Canyon Road
//             Irvine, California  92618
//    All information contained in this document is Broadcom Corporation
//    company private, proprietary, and trade secret.
//
//
//
// **************************************************************************
//    Filename: SharedMemory.c
//    Author:   Karthik Balasubramanian
//    Creation Date: Aug. 31, 2005
//
// **************************************************************************
//    Description:
//
//		DSG/eCM code
//
//    A more efficient IPC implementation as an alternative to using
//    local sockets.
//
//
// **************************************************************************
//    Revision History:
//
// **************************************************************************/

#include <stdio.h>
#include <memory.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <errno.h>
#include "BcmSharedMemory.h"

#define SEM_RINGBUFF_NUM_FULL   0	// Semaphore used by readers
#define SEM_RINGBUFF_NUM_EMPTY  1	// Semaphore used by writers
#define MAX_QUEUED_MESSAGES     20	// Depth of each shared memory segment
#define MAX_MESSAGE_SIZE	1536

// This struct encapsulates a message. It uses the MTU
// specified in the docsis code where each packet cannot
// be more than 1536 bytes.  If it is, it will be split
// into multiple packets, so we should be ok here.
typedef struct
{
   char data[MAX_MESSAGE_SIZE];
   int  size;
}msg_t;

// Shared memory data structure.  We'll create one of
// these for each data pipe we need.  For example, one
// of these will be used for each client that registers
// with the DSG-CC.
struct bcmSharedMemory_t
{
    msg_t *msg;    	// Pointer to message struct.
    int   r_slot;	// Next slot to read from.
    int   w_slot;	// Next slot to write to.
    int   m_sem;	// Semaphore that sychronizes access.
};

// Semaphore struct needed by semctl system call.
union semun
{
    int 	       val;
    struct semid_ds    *buf;
    unsigned short int *array;
    struct seminfo     *__buf;
};

// This should really be a dynamically managaed structure, ie., a linked list.
// For now, I simply statically allocated an array of structures.
static struct bcmSharedMemory_t SHM_RingBuffer[10];
static int SHM_count = 0;

// Create semaphores that synchronize access to the shared memory area.
// This function is called by the BcmSharedMemoryCreate() function. There
// are two semaphores created:  one for reading and one for writing.  The
// write semaphore will be initialized to MAX_QUEUED_MESSAGES, which is the
// number of available slots, and the read semaphore will be initialized to
// 0.
static int BcmSharedMemoryCreateSemaphores(int key, unsigned char owner)
{
    int flags = 0;

    if (owner)
    	flags = IPC_CREAT;

    // Create 2 semaphores, one for read and one for write.
    if ((SHM_RingBuffer[SHM_count].m_sem = semget((key_t)key, 2, flags)) == -1)
    {
        printf("shmget: Error creating semaphore\n");
        return -1;
    }

    // Initialize the semaphores to starting values.
    if (owner)
    {
        union semun sem_ctrl;
        // Initialize empty semaphore first
        sem_ctrl.val = MAX_QUEUED_MESSAGES;
        if(semctl(SHM_RingBuffer[SHM_count].m_sem, SEM_RINGBUFF_NUM_EMPTY, SETVAL, sem_ctrl) == -1)
        {
            printf("Error inititalizing SEM_RINGBUFF_NUM_EMPTY!\n");
            return -1;
        }

        // Initialize the full semaphore
        sem_ctrl.val = 0;
        if(semctl(SHM_RingBuffer[SHM_count].m_sem, SEM_RINGBUFF_NUM_FULL, SETVAL, sem_ctrl) == -1)
        {
            printf("Error inititalizing SEM_RINGBUFF_NUM_FULL!\n");
            return -1;
        }
    }

    return 0;
}

// Create a shared memory segment and attach it to a message buffer.
// The key passed in must be the same as the other side uses in order
// to establish a common area.  The same key will be used to create the
// sempahore as well, so if this is off, then the two entities will
// never communicate!
int BcmSharedMemoryCreate(int key, unsigned char owner)
{
    int shmID;
    /*int shmFlags = (SHM_R | SHM_W);*/
    int size = sizeof(msg_t) * MAX_QUEUED_MESSAGES;

    // Acquire a shared memory segment to use as a pipe between server and client
    shmID = shmget((key_t)key, size , SHM_R | SHM_W | IPC_CREAT);
    if (shmID == -1)
    {
        printf("shmget: Error creating shared memory segment\n");
        return -1;
    }

    // Attach it to our ring buffer pointer
    SHM_RingBuffer[SHM_count].msg = (msg_t *)shmat(shmID, (void *) 0, 0);
    if (SHM_RingBuffer[SHM_count].msg == (msg_t *) -1)
    {
        printf("shmat: Error attaching to shared memory segment\n");
        return -1;
    }

    // Create semaphores to serialize access to the ring buffer
    if (BcmSharedMemoryCreateSemaphores(key, owner) == -1)
    {
        printf("Failed to create semaphores\n");
        return -1;
    }

    // Initialize read and write pointers
    SHM_RingBuffer[SHM_count].r_slot = SHM_RingBuffer[SHM_count].w_slot = 0;
    return SHM_count++;
}

// Delete a shared memory segment
int BcmSharedMemoryDelete(int shm_id)
{
    if (shm_id >= 0)
    {
        return shmdt(SHM_RingBuffer[shm_id].msg);
    }
    else
    {
        printf("Invalid shared memory id\n");
        return -1;
    }
}

// Write a message to a shared memory segment.  If the nowait flag is
// set, then the function will not block if there's no space available.
// In this case, the function will return 0 if there was no space, -1
// in case of an error, and num. bytes written on success.
int BcmSharedMemoryWrite(int shm_id, char *buffer, int size, int nowait)
{
    struct sembuf sem_oper;

    if (size > MAX_MESSAGE_SIZE)
    {
        printf("libSharedMemory.a: Warning:  Cannot write greater than MTU size of %d bytes!\n", MAX_MESSAGE_SIZE);
        printf("Truncating message to %d bytes\n", MAX_MESSAGE_SIZE);
        size = MAX_MESSAGE_SIZE;
    }

    // Set up sembuf struct to decremement NUM_EMPTY semaphore.
    sem_oper.sem_num = SEM_RINGBUFF_NUM_EMPTY;
    sem_oper.sem_op = -1;
    sem_oper.sem_flg = 0;

    if (nowait)
        sem_oper.sem_flg = IPC_NOWAIT;

    // Acquire write semaphore (we'll block if there's no space to write)
    if (semop(SHM_RingBuffer[shm_id].m_sem, &sem_oper, 1) == -1)
    {
        if ((nowait) && (errno == EAGAIN))
        {
            return 0;
        }
        else
        {
            printf("Failed to acquire SEM_RINGBUFF_NUM_EMPTY!\n");
            return -1;
        }
    }

    // At this point, we know there's at least 1 slot left in the buffer
    memcpy((void *)SHM_RingBuffer[shm_id].msg[SHM_RingBuffer[shm_id].w_slot].data, (const void *)buffer, size);
    SHM_RingBuffer[shm_id].msg[SHM_RingBuffer[shm_id].w_slot].size = size;

    // Update w_slot
    SHM_RingBuffer[shm_id].w_slot = (SHM_RingBuffer[shm_id].w_slot + 1) % MAX_QUEUED_MESSAGES;

    // Now signal the reader process that data's available
    sem_oper.sem_num = SEM_RINGBUFF_NUM_FULL;
    sem_oper.sem_op = 1;
    sem_oper.sem_flg = 0;
    if (semop(SHM_RingBuffer[shm_id].m_sem, &sem_oper, 1) == -1)
    {
        printf("Failed to signal on the SEM_RINGBUFF_NUM_FULL semaphore!\n");
        return -1;
    }

    return size;
}

// Read the next message from shared memory segment.  If the nowait flag is
// set, then the function will not block if there's no data available to read.
// In this case, the function will return 0 if there was no message, -1
// in case of an error, and num. bytes read on success.
int BcmSharedMemoryRead(int shm_id, char *buff, int nowait)
{
    struct sembuf sem_oper;
    int    size;

    if (!buff)
    {
        printf("libSharedMemory.a: Error:  Null buffer passed in!\n");
        return -1;
    }

    // Setup read semaphore for semop
    sem_oper.sem_num = SEM_RINGBUFF_NUM_FULL;
    sem_oper.sem_op = -1;
    sem_oper.sem_flg = 0;

    if (nowait)
        sem_oper.sem_flg = IPC_NOWAIT;

    // Acquire read semaphore (we'll block until there's data available)
    if (semop(SHM_RingBuffer[shm_id].m_sem, &sem_oper, 1) == -1)
    {
        buff = (char *) 0;
        if ((nowait) && (errno == EAGAIN))
        {
            return 0;
        }
        else
        {
            perror("semop:");
            return -1;
        }
    }

    // Get the size of the message we're going to read.
    size = SHM_RingBuffer[shm_id].msg[SHM_RingBuffer[shm_id].r_slot].size;
    // At this point, we have data to copy over
    memcpy(buff, SHM_RingBuffer[shm_id].msg[SHM_RingBuffer[shm_id].r_slot].data, size);
    buff[size+1] = '\0';
    // Update r_slot
    SHM_RingBuffer[shm_id].r_slot = (SHM_RingBuffer[shm_id].r_slot + 1) % MAX_QUEUED_MESSAGES;

    // Now update number of free slots and signal writer process
    sem_oper.sem_num = SEM_RINGBUFF_NUM_EMPTY;
    sem_oper.sem_op = 1;
    sem_oper.sem_flg = 0;
    if (semop(SHM_RingBuffer[shm_id].m_sem, &sem_oper, 1) == -1)
    {
         printf("Failed to signal on the SEM_RINGBUFF_NUM_EMPTY semaphore!\n");
         return -1;
    }

    return size;
}

// The semtimedop function is not available in our kernel (??)
#if 0
int BcmSharedMemoryTimedRead(int shm_id, char *buff, int timeout)
{
    struct sembuf   sem_oper;
    struct timespec sem_timeout;
    int    	    size;

    sem_oper.sem_num = SEM_RINGBUFF_NUM_FULL;
    sem_oper.sem_op = -1;
    sem_oper.sem_flg = 0;

    sem_timeout.tv_sec = timeout;

    if (sys_semtimedop(SHM_RingBuffer[shm_id].m_sem, &sem_oper, 1, (const struct timespec *)&sem_timeout) == -1)
    {
        buff = (unsigned char *) 0;
        return -1;
    }

    size = SHM_RingBuffer[shm_id].msg[SHM_RingBuffer[shm_id].r_slot].size;
    // At this point, we have data to copy over
    memcpy(buff, SHM_RingBuffer[shm_id].msg[SHM_RingBuffer[shm_id].r_slot].data, size);
    buff[size+1] = '\0';
    // Update r_slot
    SHM_RingBuffer[shm_id].r_slot = (SHM_RingBuffer[shm_id].r_slot + 1) % MAX_QUEUED_MESSAGES;

    // Now update number of free slots
    sem_oper.sem_num = SEM_RINGBUFF_NUM_EMPTY;
    sem_oper.sem_op = 1;
    sem_oper.sem_flg = 0;

    if (semop(SHM_RingBuffer[shm_id].m_sem, &sem_oper, 1) == -1)
    {
         printf("Failed to signal on the SEM_RINGBUFF_NUM_EMPTY semaphore!\n");
         return -1;
    }
    return size;
}
#endif

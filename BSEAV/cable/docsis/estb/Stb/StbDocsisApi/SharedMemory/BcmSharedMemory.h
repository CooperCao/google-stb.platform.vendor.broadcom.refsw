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

int BcmSharedMemoryCreate(int key, unsigned char owner);
int BcmSharedMemoryDelete(int shm_id);
int BcmSharedMemoryWrite(int shm_id, char *buffer, int size, int nowait);
int BcmSharedMemoryRead(int shm_id, char *buffer, int nowait);

/* The semtimedop function is not available in our kernel (??) */
#if 0
int BcmSharedMemoryTimedRead(int shm_id, char *buffer, int timeout);
#endif

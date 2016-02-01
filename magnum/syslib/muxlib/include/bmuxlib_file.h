/***************************************************************************
 *     Copyright (c) 2010-2011, Broadcom Corporation
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
 * [File Description:]
 * Muxlib Storage (File) I/O Interface
 *
 * Note that the storage does not necessarily need to be a file, and if the
 * system decides it has enough memory (based on the expected size provided by
 * the caller), it may decide not to use a file and use memory instead.
 *
 * One storage "system" that oversees and manages the individual storage entities
 * (represented by an "interface" to that entity).
 * Individual storage "interfaces" have context and can be passed around as handles.
 *
 * [Revision History:]
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BMUXLIB_FILE_H__
#define BMUXLIB_FILE_H__

/* Includes */

#ifdef __cplusplus
extern "C" {
#endif

/****************************/
/* Storage System Interface */
/****************************/

/* In the following, pDesc is expected to be of the type: BMUXlib_StorageDescriptor * */
#define BMUXLIB_STORAGE_GET_NEXT_DESC(pDesc)  \
   (BMUXlib_StorageDescriptor *)((uint8_t *)(pDesc) + sizeof(BMUXlib_StorageDescriptor) + ((pDesc)->uiVectorCount * sizeof(BMUXlib_StorageBuffer))- sizeof(BMUXlib_StorageBuffer))

/*
 * Storage buffer definition. This is defined to be the same as struct iovec
 * defined in <sys/uio.h> which is not used here due to potential porting issues.
 */
typedef struct BMUXlib_StorageBuffer
{
   void *pBufferAddress; /* pointer to the data to write or the buffer to read into */
   /* NB: a zero length *is* supported in the storage buffers uiLength member and must
      NOT alter the storage (a NOP) */
   size_t uiLength;      /* length in bytes of the data to read or write */
} BMUXlib_StorageBuffer;

/*
 * The definition of storage descriptor written to disk or other output media. We
 * currently support disk based vectored I/O. See the man pages for writev/readv
 * for a description of vectored (scatter/gather) I/O.
 */
typedef struct BMUXlib_StorageDescriptor
{
   uint64_t uiOffset;              /* offset of descriptor in storage media */
   bool bWriteOperation;           /* false = read operation */
   uint16_t uiVectorCount;         /* number of vector elements for scatter/gather I/O */
   /* the following may actually extend beyond 1 element, as determined by uiVectorCount */
   BMUXlib_StorageBuffer iov[1]; /* actual item(s) written to storage */
} BMUXlib_StorageDescriptor;

typedef BERR_Code (*BMUXlib_AddStorageDescriptors)(
                        void *pStorageInterfaceContext,                    /* [in] context for this interface (i.e. BMUXlib_StorageObjectInterface.pContext) */
                        const BMUXlib_StorageDescriptor *pDescriptors,     /* [in] pointer to descriptors to process */
                        size_t uiNumDescriptors,                           /* [in] number of descriptors pointed to by pDescriptors */
                        size_t *puiQueuedCount);                           /* [out] location of count updated with actual number of descriptors processed */

typedef BERR_Code (*BMUXlib_GetCompletedStorageDescriptors)(
                        void *pStorageInterfaceContext,                    /* [in] context for this interface (i.e. BMUXlib_StorageObjectInterface.pContext) */
                        size_t *puiCompletedCount);                        /* [out] location of count updated with number of descriptors completed */

typedef struct
{
   void *pContext;
   BMUXlib_AddStorageDescriptors pfAddDescriptors;                         /* queue up descriptors for storage */
   BMUXlib_GetCompletedStorageDescriptors pfGetCompleteDescriptors;        /* get count of queued descriptors that have been processed */
} BMUXlib_StorageObjectInterface;

typedef struct
{
   uint64_t uiExpectedSizeBytes;    /* the expected size of this data - if not known, set this to zero */
} BMUXlib_StorageSettings;

/* obtain the default settings to use for creating a new storage interface */
typedef void (*BMUXlib_GetDefaultStorageSettings)(
                        BMUXlib_StorageSettings *pStorageSettings);        /* [out] location to write default storage settings */

/* Create a new storage object, and associated interface to access it.
   It is assumed that all storage created by this API is temporary and is opened in binary r/w mode */
typedef BERR_Code (*BMUXlib_CreateStorageObject)(
                        void *pStorageSystemContext,                       /* [in] storage context (i.e. BMUXlib_StorageSystemInterface.pContext) */
                        BMUXlib_StorageObjectInterface *pStorageObject,    /* [out] pointer to the location to create the storage object */
                        const BMUXlib_StorageSettings *pStorageSettings);  /* [in] settings to use for creation of this storage object */

/* destroy the specified storage object, and destroy the associated storage (i.e. if file, remove the file) */
/* FIXME: Can destroy fail? What if unable to remove a file (read-only lock for example)? */
typedef void (*BMUXlib_DestroyStorageObject)(
                        void *pStorageSystemContext,                       /* [in] storage context (i.e. BMUXlib_StorageSystemInterface.pContext) */
                        void *pStorageObjectContext);                      /* [in] storage object context (i.e. BMUXlib_StorageObjectInterface.pContext)  */

/* the interface to the storage system, allowing creation/destruction of new storage objects */
typedef struct
{
   void *pContext;
   BMUXlib_GetDefaultStorageSettings pfGetDefaultStorageSettings;          /* obtain default settings for creation of storage */
   BMUXlib_CreateStorageObject pfCreateStorageObject;                      /* create new storage object and initialise the associated interface */
   BMUXlib_DestroyStorageObject pfDestroyStorageObject;                    /* destroy the specified storage object and interface */
} BMUXlib_StorageSystemInterface;


#ifdef __cplusplus
}
#endif

#endif /* BMUXLIB_FILE_H__ */

/*****************************************************************************
* EOF
******************************************************************************/

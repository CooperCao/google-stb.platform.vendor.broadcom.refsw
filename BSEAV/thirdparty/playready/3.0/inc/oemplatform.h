/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#ifndef __OEMPLATFORM_H__
#define __OEMPLATFORM_H__

ENTER_PK_NAMESPACE

/* OEM critical sectiion function. */
#ifdef ANDROID
#include <pthread.h>
typedef pthread_mutex_t OEM_CRITICAL_SECTION;
#else
#if __MACINTOSH__
#if DRM_64BIT_TARGET
#define OEM_CRITICAL_SECTION_SIZE     8
#else
#define OEM_CRITICAL_SECTION_SIZE     4
#endif
#else
#define OEM_CRITICAL_SECTION_SIZE     48 /* BRCM sizeof(pthread_mutex_t) is 48 bytes on 7271 (64 bits) */
#endif

typedef struct __tagOEM_CRITICAL_SECTION
{
    DRM_BYTE rgb[ OEM_CRITICAL_SECTION_SIZE ];
} OEM_CRITICAL_SECTION;
#endif

/**********************************************************************
** Function:    Oem_CritSec_Initialize
** Synopsis:    Initializes critical section.
** Arguments:   [f_pCS]--Pointer to OEM_CRITICAL_SECTION structure to be
**              initialized.
***********************************************************************/
DRM_API_VOID DRM_VOID DRM_CALL Oem_CritSec_Initialize(
    __inout OEM_CRITICAL_SECTION *f_pCS );

/**********************************************************************
** Function:    Oem_CritSec_Delete
** Synopsis:    Deletes existing critical section.
** Arguments:   [f_pCS]--Pointer to OEM_CRITICAL_SECTION structure to be
**              deleted.
***********************************************************************/
DRM_API_VOID DRM_VOID DRM_CALL Oem_CritSec_Delete(
    __inout OEM_CRITICAL_SECTION *f_pCS );

/**********************************************************************
** Function:    Oem_CritSec_Enter
** Synopsis:    Enters critical section.
** Arguments:   [f_pCS]--Pointer to OEM_CRITICAL_SECTION structure to be
**              entered.
***********************************************************************/
DRM_API_VOID DRM_VOID DRM_CALL Oem_CritSec_Enter(
    __inout OEM_CRITICAL_SECTION *f_pCS );

/**********************************************************************
** Function:    Oem_CritSec_Leave
** Synopsis:    Leaves critical section.
** Arguments:   [f_pCS]--Pointer to OEM_CRITICAL_SECTION structure to be
**              left.
***********************************************************************/
DRM_API_VOID DRM_VOID DRM_CALL Oem_CritSec_Leave(
    __inout OEM_CRITICAL_SECTION *f_pCS );


/*BRCM: These 2 APIs are only defined in oem/ansi, which isn't used to build the SDK linux platform. */
/*DRM_API_VOID DRM_VOID DRM_CALL Oem_Test_Mem_Alloc_Check_Leakscan();
DRM_API_VOID DRM_VOID DRM_CALL Oem_Test_Mem_Alloc_Clear_Leakscan();*/

EXIT_PK_NAMESPACE

#endif  /* __OEMPLATFORM_H__ */

/*=============================================================================
Copyright (c) 2009-2014 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  sync-list validation

FILE DESCRIPTION
Drivers use this interface to validate the sync-lists against the control list
=============================================================================*/

#ifndef _KHRN_SYNCLIST_VALIDATE_H_

#include "middleware/khronos/common/khrn_fmem.h"
#include "middleware/khronos/common/khrn_synclist.h"

VCOS_EXTERN_C_BEGIN

extern bool khrn_synclist_validate(const V3D_BIN_RENDER_INFO_T *br_info,
                    const struct khrn_handle_list *bin_sync_list,
                    const struct khrn_handle_list *render_sync_list);

VCOS_EXTERN_C_END

#endif // _KHRN_SYNCLIST_VALIDATE_H_

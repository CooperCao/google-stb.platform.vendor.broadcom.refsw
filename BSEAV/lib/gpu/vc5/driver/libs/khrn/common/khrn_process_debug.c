/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "khrn_process_debug.h"

#ifdef KHRN_GEOMD
#include "khrn_fmem_debug_info.h"
#include "libs/tools/v3d_debug_helper/v3d_debug_helper.h"

static v3d_addr_t get_event_addr(const struct v3d_debug_event *e)
{
   switch (e->type)
   {
   case V3D_DEBUG_EVENT_BIN_START:     return e->u.bin_start.last_bcfg_instr_end_addr;
   case V3D_DEBUG_EVENT_PTB_PRIM:      return e->u.ptb_prim.shader_record_addr;
   case V3D_DEBUG_EVENT_RENDER_START:  return e->u.render_start.last_rcfg_instr_end_addr;
   case V3D_DEBUG_EVENT_TLB_STORE:     return e->u.tlb_store.last_rcfg_instr_end_addr;
   default:                            unreachable(); return 0;
   }
}

void khrn_debug_callback(const struct v3d_debug_event *e, void *p)
{
   struct v3d_debug_helper_event_extra extra;
   if (!fmem_debug_info_query(get_event_addr(e), &extra.rs_id, &extra.draw_id))
      return;
   v3d_debug_helper_handle_event(e, &extra);
}
#else
void khrn_debug_callback(const struct v3d_debug_event *e, void *p)
{
   vcos_unused(e);
   vcos_unused(p);
}
#endif

/*******************************************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *******************************************************************************************************/
/* See http://confluence.broadcom.com/x/1JGFBw */

#include "early_z.h"
#include "libs/util/log/log.h"

LOG_DEFAULT_CAT("early_z");

void early_z_init(EARLY_Z_STATE_T *state, bool enable)
{
   log_trace("early_z_init, enabled = %u [%p]", enable, state);

   /* Initialise to something so we don't need a special case for the compare
    * at the end of early_z_update_cfg for the first update */
   state->cfg_bits_ez = false;
   state->cfg_bits_ez_update = false;

   /* Pick LT/LE arbitrarily to start with. We may change this later --
    * chosen_direction=false indicates that we haven't really chosen yet. We
    * don't set this to INVALID because we might never really choose and we
    * need to stick a valid value in the CLE instruction */
   state->rcfg_ez_direction = V3D_EZ_DIRECTION_LT_LE;
   /* This will be set to false if we ever have cfg_bits_ez=true */
   state->rcfg_ez_disable = true;

   state->disabled_for_rest_of_frame = !enable;
   state->chosen_direction = false;
}

/* TODO Could be smarter, eg:
 * - if back-face culling is enabled, back stencil op != keep doesn't matter
 * - if stencil test set to never, depth test/update doesn't matter */
bool early_z_update_cfg(EARLY_Z_STATE_T *state,
   v3d_compare_func_t depth_func, bool depth_update, bool stencil,
   v3d_stencil_op_t front_fail_op, v3d_stencil_op_t front_depth_fail_op,
   v3d_stencil_op_t back_fail_op, v3d_stencil_op_t back_depth_fail_op)
{
   bool cfg_bits_ez = false, cfg_bits_ez_update = false;
   bool cfg_bits_changed;

   if (!state->disabled_for_rest_of_frame)
   {
      bool depth_func_compatible;

      switch (depth_func)
      {
      case V3D_COMPARE_FUNC_NEVER:
      case V3D_COMPARE_FUNC_EQUAL:
         /* Compatible with both directions, so no need to choose yet... */
         depth_func_compatible = true;
         break;
      case V3D_COMPARE_FUNC_NOTEQUAL:
      case V3D_COMPARE_FUNC_ALWAYS:
         /* Incompatible with both directions, so no point in choosing yet... */
         depth_func_compatible = false;
         break;
      case V3D_COMPARE_FUNC_LESS:
      case V3D_COMPARE_FUNC_LEQUAL:
         if (!state->chosen_direction)
         {
            state->rcfg_ez_direction = V3D_EZ_DIRECTION_LT_LE;
            state->chosen_direction = true;
         }
         depth_func_compatible = (state->rcfg_ez_direction == V3D_EZ_DIRECTION_LT_LE);
         break;
      case V3D_COMPARE_FUNC_GREATER:
      case V3D_COMPARE_FUNC_GEQUAL:
         if (!state->chosen_direction)
         {
            state->rcfg_ez_direction = V3D_EZ_DIRECTION_GT_GE;
            state->chosen_direction = true;
         }
         depth_func_compatible = (state->rcfg_ez_direction == V3D_EZ_DIRECTION_GT_GE);
         break;
      default:
         unreachable();
      }

      if (depth_update && !depth_func_compatible)
      {
         state->disabled_for_rest_of_frame = true;
      }
      else
      {
         cfg_bits_ez = depth_func_compatible && (!stencil || (
            (front_fail_op == V3D_STENCIL_OP_KEEP) &&
            (front_depth_fail_op == V3D_STENCIL_OP_KEEP) &&
            (back_fail_op == V3D_STENCIL_OP_KEEP) &&
            (back_depth_fail_op == V3D_STENCIL_OP_KEEP)));
         /* true would also be fine -- the hardware will ignore this bit if
          * depth_update isn't set */
         cfg_bits_ez_update = depth_update;
      }
   }

   if (state->disabled_for_rest_of_frame)
   {
      cfg_bits_ez = false;
      cfg_bits_ez_update = false; /* Disable update to save power */
   }

   if (cfg_bits_ez)
   {
      state->rcfg_ez_disable = false;
   }

   cfg_bits_changed =
      (cfg_bits_ez != state->cfg_bits_ez) ||
      (cfg_bits_ez_update != state->cfg_bits_ez_update);

   state->cfg_bits_ez = cfg_bits_ez;
   state->cfg_bits_ez_update = cfg_bits_ez_update;

   return cfg_bits_changed;
}

bool early_z_disable(EARLY_Z_STATE_T *state)
{
   log_trace("early_z_disable");

   bool cfg_bits_ez = false, cfg_bits_ez_update = false;

   bool cfg_bits_changed = (cfg_bits_ez != state->cfg_bits_ez) ||
                           (cfg_bits_ez_update != state->cfg_bits_ez_update);

   state->disabled_for_rest_of_frame = true;

   state->cfg_bits_ez = cfg_bits_ez;
   state->cfg_bits_ez_update = cfg_bits_ez_update;

   return cfg_bits_changed;
}

static const char *early_z_direction_str(v3d_ez_direction_t dir)
{
   switch (dir)
   {
   case V3D_EZ_DIRECTION_LT_LE:   return "LT_LE";
   case V3D_EZ_DIRECTION_GT_GE:   return "GT_GE";
   case V3D_EZ_DIRECTION_INVALID: return "INVALID";
   default:                       unreachable();
   }
}

extern void early_z_log(const EARLY_Z_STATE_T *state)
{
   if (log_trace_enabled())
   {
      log_trace("Early Z data for [%p]", state);
      log_trace("  Early Z config       %s", state->cfg_bits_ez ? "ENABLED" : "DISABLED");
      log_trace("  Early Z update       %s", state->cfg_bits_ez_update ? "ENABLED" : "DISABLED");
      log_trace("  Early Z direction    %s", early_z_direction_str(state->rcfg_ez_direction));
      log_trace("  Early Z renderConfig %s", state->rcfg_ez_disable ? "DISABLED" : "ENABLED");
   }
}

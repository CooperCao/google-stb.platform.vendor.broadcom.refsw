/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "v3d_common.h"
#include "v3d_cl_compr.h"
#include "v3d_cl.h"

#include "vcos.h"
#include <stdbool.h>
#include "libs/core/v3d/v3d_addr.h"
#include "libs/core/v3d/v3d_printer.h"

bool v3d_cl_has_common_ind_encoding(v3d_cl_compr_type_t type)
{
   switch (type)
   {
#if V3D_HAS_INLINE_CLIP
   case V3D_CL_COMPR_TYPE_PRIM_ID8:
   case V3D_CL_COMPR_TYPE_PRIM_ID32:
   case V3D_CL_COMPR_TYPE_CLIPPED_PRIM:
#else
   case V3D_CL_COMPR_TYPE_REL_BRANCH:
   case V3D_CL_COMPR_TYPE_BRANCH:
#endif
   case V3D_CL_COMPR_TYPE_IID8:
   case V3D_CL_COMPR_TYPE_IID32:
   case V3D_CL_COMPR_TYPE_ESCAPE:
      return true;
   default:
      return false;
   }
}

uint32_t v3d_cl_compr_size(
   const V3D_CL_PRIM_LIST_FORMAT_T *plist_fmt,
   v3d_cl_compr_type_t compr_type)
{
   if (plist_fmt->xy)
   {
      assert(plist_fmt->n_verts == 3);
      assert(!plist_fmt->d3dpvsf);
      return v3d_cl_compr_xy_tri_packed_size(compr_type);
   }
#if V3D_HAS_TNG
   else if(plist_fmt->n_verts > 3)
   {
      return v3d_cl_compr_ind_generic_packed_size(compr_type);
   }
#endif
   else if (v3d_cl_has_common_ind_encoding(compr_type))
      return v3d_cl_compr_ind_common_packed_size(compr_type);
   else if (plist_fmt->d3dpvsf)
   {
      assert(plist_fmt->n_verts == 3);
      return v3d_cl_compr_ind_d3dpvsf_tri_packed_size(compr_type);
   }
   else switch (plist_fmt->n_verts)
   {
   case 3: return v3d_cl_compr_ind_tri_packed_size(compr_type);
   case 2: return v3d_cl_compr_ind_line_packed_size(compr_type);
   case 1: return v3d_cl_compr_ind_point_packed_size(compr_type);
   default: unreachable(); return 0;
   }
}

void v3d_cl_unpack_compr(struct v3d_cl_compr *compr,
   const V3D_CL_PRIM_LIST_FORMAT_T *plist_fmt,
   const uint8_t *packed_compr)
{
   compr->plist_fmt = *plist_fmt;
   if (plist_fmt->xy)
   {
      assert(plist_fmt->n_verts == 3);
      assert(!plist_fmt->d3dpvsf);
      V3D_CL_COMPR_XY_TRI_T tri;
      v3d_unpack_cl_compr_xy_tri(&tri, packed_compr);
      compr->compr_type = tri.type;
      memcpy(&compr->u, &tri.u, sizeof(tri.u));
   }
#if V3D_HAS_TNG
   else if(plist_fmt->n_verts > 3)
   {
      V3D_CL_COMPR_IND_GENERIC_T ind;
      v3d_unpack_cl_compr_ind_generic(&ind, packed_compr);
      compr->compr_type = ind.type;
      memcpy(&compr->u, &ind.u, sizeof(ind.u));
   }
#endif
   else
   {
      V3D_CL_COMPR_IND_COMMON_T common;
      v3d_unpack_cl_compr_ind_common(&common, packed_compr);
      if (common.type != V3D_CL_COMPR_TYPE_NOT_COMMON)
      {
         compr->compr_type = common.type;
         memcpy(&compr->u, &common.u, sizeof(common.u));
      }
      else if (plist_fmt->d3dpvsf)
      {
         assert(plist_fmt->n_verts == 3);
         V3D_CL_COMPR_IND_D3DPVSF_TRI_T tri;
         v3d_unpack_cl_compr_ind_d3dpvsf_tri(&tri, packed_compr);
         compr->compr_type = tri.type;
         memcpy(&compr->u, &tri.u, sizeof(tri.u));
      }
      else switch (plist_fmt->n_verts)
      {
      case 3:
      {
         V3D_CL_COMPR_IND_TRI_T tri;
         v3d_unpack_cl_compr_ind_tri(&tri, packed_compr);
         compr->compr_type = tri.type;
         memcpy(&compr->u, &tri.u, sizeof(tri.u));
         break;
      }
      case 2:
      {
         V3D_CL_COMPR_IND_LINE_T line;
         v3d_unpack_cl_compr_ind_line(&line, packed_compr);
         compr->compr_type = line.type;
         memcpy(&compr->u, &line.u, sizeof(line.u));
         break;
      }
      case 1:
      {
         V3D_CL_COMPR_IND_POINT_T point;
         v3d_unpack_cl_compr_ind_point(&point, packed_compr);
         compr->compr_type = point.type;
         memcpy(&compr->u, &point.u, sizeof(point.u));
         break;
      }
      default:
         unreachable();
      }
   }
}

void v3d_cl_pack_compr(uint8_t *packed_compr,
   const struct v3d_cl_compr *compr)
{
   if (compr->plist_fmt.xy)
   {
      assert(compr->plist_fmt.n_verts == 3);
      assert(!compr->plist_fmt.d3dpvsf);
      V3D_CL_COMPR_XY_TRI_T tri;
      tri.type = compr->compr_type;
      memcpy(&tri.u, &compr->u, sizeof(tri.u));
      v3d_pack_cl_compr_xy_tri(packed_compr, &tri);
   }
#if V3D_HAS_TNG
   else if(compr->plist_fmt.n_verts > 3)
   {
      /* Generic compressed indices. The common encodings for this type are not
       * actually common. */
      V3D_CL_COMPR_IND_GENERIC_T ind;
      ind.type = compr->compr_type;
      memcpy(&ind.u, &compr->u, sizeof(ind.u));
      v3d_pack_cl_compr_ind_generic(packed_compr, &ind);
   }
#endif
   else if (v3d_cl_has_common_ind_encoding(compr->compr_type))
   {
      V3D_CL_COMPR_IND_COMMON_T common;
      common.type = compr->compr_type;
      memcpy(&common.u, &compr->u, sizeof(common.u));
      v3d_pack_cl_compr_ind_common(packed_compr, &common);
   }
   else if (compr->plist_fmt.d3dpvsf)
   {
      assert(compr->plist_fmt.n_verts == 3);
      V3D_CL_COMPR_IND_D3DPVSF_TRI_T tri;
      tri.type = compr->compr_type;
      memcpy(&tri.u, &compr->u, sizeof(tri.u));
      v3d_pack_cl_compr_ind_d3dpvsf_tri(packed_compr, &tri);
   }
   else switch (compr->plist_fmt.n_verts)
   {
   case 3:
   {
      V3D_CL_COMPR_IND_TRI_T tri;
      tri.type = compr->compr_type;
      memcpy(&tri.u, &compr->u, sizeof(tri.u));
      v3d_pack_cl_compr_ind_tri(packed_compr, &tri);
      break;
   }
   case 2:
   {
      V3D_CL_COMPR_IND_LINE_T line;
      line.type = compr->compr_type;
      memcpy(&line.u, &compr->u, sizeof(line.u));
      v3d_pack_cl_compr_ind_line(packed_compr, &line);
      break;
   }
   case 1:
   {
      V3D_CL_COMPR_IND_POINT_T point;
      point.type = compr->compr_type;
      memcpy(&point.u, &compr->u, sizeof(point.u));
      v3d_pack_cl_compr_ind_point(packed_compr, &point);
      break;
   }
   default:
      unreachable();
   }
}

void v3d_cl_print_compr(const uint8_t *packed_compr,
   const V3D_CL_PRIM_LIST_FORMAT_T *plist_fmt,
   struct v3d_printer *printer)
{
   if (plist_fmt->xy)
   {
      assert(plist_fmt->n_verts == 3);
      assert(!plist_fmt->d3dpvsf);
      v3d_print_cl_compr_xy_tri(packed_compr, printer);
   }
#if V3D_HAS_TNG
   else if(plist_fmt->n_verts > 3)
   {
      v3d_print_cl_compr_ind_generic(packed_compr, printer);
   }
#endif
   else
   {
      V3D_CL_COMPR_IND_COMMON_T common;
      v3d_unpack_cl_compr_ind_common(&common, packed_compr);
      if (common.type != V3D_CL_COMPR_TYPE_NOT_COMMON)
         v3d_print_cl_compr_ind_common(packed_compr, printer);
      else if (plist_fmt->d3dpvsf)
      {
         assert(plist_fmt->n_verts == 3);
         v3d_print_cl_compr_ind_d3dpvsf_tri(packed_compr, printer);
      }
      else switch (plist_fmt->n_verts)
      {
      case 3: v3d_print_cl_compr_ind_tri(packed_compr, printer); break;
      case 2: v3d_print_cl_compr_ind_line(packed_compr, printer); break;
      case 1: v3d_print_cl_compr_ind_point(packed_compr, printer); break;
      default: unreachable();
      }
   }
}

void v3d_cl_log_cat_compr(
   struct log_cat *cat, log_level_t level, const char *line_prefix,
   const uint8_t *packed_compr, const struct v3d_cl_source *source,
   const V3D_CL_PRIM_LIST_FORMAT_T *plist_fmt)
{
   if (!log_cat_enabled(cat, level))
      return;

   {
      struct v3d_cl_compr c;
      v3d_cl_unpack_compr(&c, plist_fmt, packed_compr);

      struct v3d_cl_source truncated_source = *source;
      v3d_cl_truncate_source(&truncated_source, v3d_cl_compr_size_2(&c));

      char desc[256];
      size_t offset = VCOS_SAFE_SPRINTF(desc, 0, "(");
      offset = v3d_cl_sprint_plist_fmt(desc, sizeof(desc), offset, plist_fmt);
      offset = VCOS_SAFE_SPRINTF(desc, offset, ") %s", v3d_desc_cl_compr_type(c.compr_type));
      assert(offset < sizeof(desc));

      v3d_cl_log_cat_bytes(cat, level, line_prefix, desc, packed_compr, &truncated_source);
   }

   {
      char prefix[256];
      verif(VCOS_SAFE_SPRINTF(prefix, 0, "%s   ", line_prefix) < sizeof(prefix));

      struct v3d_basic_log_cat_printer p;
      v3d_basic_log_cat_printer_init(&p, cat, level, prefix);
      v3d_cl_print_compr(packed_compr, plist_fmt, &p.base.base);
   }
}

/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "v3d_common.h"
#include "v3d_printer.h"

#include "libs/util/assert_helpers.h"
#include "libs/util/demand.h"
#include "vcos_string.h"
#include <stdarg.h>

void v3d_printer_addr_field(struct v3d_printer *root, const char *name, v3d_addr_t value)
{
   root->vtbl->field(root, name, "0x%08x", value);
}

void v3d_printer_boolean_field(struct v3d_printer *root, const char *name, bool value)
{
   root->vtbl->field(root, name, value ? "true" : "false");
}

/** v3d_basic_printer */

static void line_vprintf(struct v3d_basic_printer_line *line, const char *format, va_list args)
{
   line->offset = vcos_safe_vsprintf(line->buf, sizeof(line->buf), line->offset, format, args);
}

static void line_printf(struct v3d_basic_printer_line *line, const char *format, ...)
{
   va_list args;
   va_start(args, format);
   line_vprintf(line, format, args);
   va_end(args);
}

static void line_print_name_part(struct v3d_basic_printer_line *line,
   bool *first, v3d_printer_container_type_t parent_type, const char *name)
{
   switch (parent_type)
   {
   case V3D_PRINTER_STRUCT:
   case V3D_PRINTER_UNION:    line_printf(line, "%s%s", *first ? "" : ".", name); break;
   case V3D_PRINTER_ARRAY:    line_printf(line, "[%s]", name); break;
   default:                   unreachable();
   }
   *first = false;
}

/* Returns true if any name was printed */
static bool line_print_name(struct v3d_basic_printer_line *line,
   const struct v3d_basic_printer *p, uint32_t skip, const char *name)
{
   bool first = true;

   assert(skip <= p->num_containers);
   v3d_printer_container_type_t parent_type =
      (skip == 0) ? V3D_PRINTER_STRUCT : p->containers[skip - 1].type;
   for (uint32_t i = skip; i != p->num_containers; ++i)
   {
      const struct v3d_basic_printer_container *c = &p->containers[i];
      bool last = (i == (p->num_containers - 1)) && !name;
      if (c->name && (!c->hidden || last))
         line_print_name_part(line, &first, parent_type, c->name);
      parent_type = c->type;
   }

   if (name)
      line_print_name_part(line, &first, parent_type, name);

   return !first;
}

/* Returns true if any name was printed */
static bool line_print_prefix(struct v3d_basic_printer_line *line,
   const struct v3d_basic_printer *p, const char *name)
{
   const struct v3d_basic_printer_container *top_c =
      (p->num_containers == 0) ? NULL : &p->containers[p->num_containers - 1];

   line_printf(line, "%s", p->line_prefix);

   if (top_c)
      for (uint32_t i = 0; i != top_c->indent; ++i)
         line_printf(line, "   ");

   return line_print_name(line, p, top_c ? top_c->skip : 0, name);
}

static void line_done(struct v3d_basic_printer *p, const struct v3d_basic_printer_line *line)
{
   const struct v3d_basic_printer_vtbl *vtbl = (const struct v3d_basic_printer_vtbl *)p->base.vtbl;
   assert(line->offset < sizeof(line->buf));
   vtbl->line(p, line->buf);
}

static void begin_container_or_field(struct v3d_basic_printer *p, bool field)
{
   if (!p->top_c_empty)
      return;
   p->top_c_empty = false;

   if (p->num_containers == 0)
      return;
   struct v3d_basic_printer_container *top_c = &p->containers[p->num_containers - 1];

   assert(!p->inline_array);
   p->inline_array = p->inline_arrays && (top_c->type == V3D_PRINTER_ARRAY) && field;
   if (p->inline_array)
   {
      p->ia_line.offset = 0;
      if (line_print_prefix(&p->ia_line, p, NULL))
         line_printf(&p->ia_line, " = ");
      line_printf(&p->ia_line, "{");
   }
   else
   {
      struct v3d_basic_printer_line line = {0};
      line_print_name(&line, p, top_c->skip, "");
      if (line.offset > p->name_len_limit)
      {
         line.offset = 0;
         verif(line_print_prefix(&line, p, NULL));
         line_done(p, &line);
         ++top_c->indent;
         top_c->skip = p->num_containers;
      }
   }
}

void v3d_basic_printer_begin(struct v3d_printer *root, v3d_printer_container_type_t type, const char *name, bool hidden)
{
   struct v3d_basic_printer *p = (struct v3d_basic_printer *)root;

   begin_container_or_field(p, /*field=*/false);

   assert(!p->inline_array);

   const struct v3d_basic_printer_container *parent_c =
      (p->num_containers == 0) ? NULL : &p->containers[p->num_containers - 1];

   assert(p->num_containers < countof(p->containers));
   struct v3d_basic_printer_container *c = &p->containers[p->num_containers++];

   c->type = type;
   c->name = name;
   c->hidden = hidden;

   c->indent = parent_c ? parent_c->indent : 0;
   c->skip = parent_c ? parent_c->skip : 0;

   p->top_c_empty = true;
}

void v3d_basic_printer_end(struct v3d_printer *root)
{
   struct v3d_basic_printer *p = (struct v3d_basic_printer *)root;

   assert(p->num_containers > 0);
   --p->num_containers;

   p->top_c_empty = false;

   if (p->inline_array)
   {
      line_printf(&p->ia_line, "}");
      line_done(p, &p->ia_line);
      p->inline_array = false;
   }
}

void v3d_basic_printer_field(struct v3d_printer *root, const char *name, const char *value_format, ...)
{
   struct v3d_basic_printer *p = (struct v3d_basic_printer *)root;

   bool top_c_empty = p->top_c_empty;
   begin_container_or_field(p, /*field=*/true);

   va_list args;
   va_start(args, value_format);

   if (p->inline_array)
   {
      if (!top_c_empty)
         line_printf(&p->ia_line, ",");
      line_vprintf(&p->ia_line, value_format, args);
   }
   else
   {
      struct v3d_basic_printer_line line = {0};
      if (line_print_prefix(&line, p, name))
         line_printf(&line, " = ");
      line_vprintf(&line, value_format, args);
      line_done(p, &line);
   }

   va_end(args);
}

void v3d_basic_printer_init(struct v3d_basic_printer *p,
   const char *line_prefix)
{
   p->line_prefix = line_prefix;
   p->inline_arrays = true;
   p->name_len_limit = 10;

   p->num_containers = 0;

   p->top_c_empty = true;

   p->inline_array = false;
}

/** v3d_basic_file_printer */

void v3d_basic_file_printer_line(struct v3d_basic_printer *root, const char *line)
{
   struct v3d_basic_file_printer *p = (struct v3d_basic_file_printer *)root;
   demand(fprintf(p->f, "%s\n", line) >= 0);
}

static const struct v3d_basic_printer_vtbl v3d_basic_file_printer_vtbl = {
   V3D_BASIC_PRINTER_VTBL_INIT(v3d_basic_file_printer)};

void v3d_basic_file_printer_init(struct v3d_basic_file_printer *p,
   FILE *f, const char *line_prefix)
{
   v3d_basic_printer_init(&p->base, line_prefix);
   p->base.base.vtbl = &v3d_basic_file_printer_vtbl.base;
   p->f = f;
}

/** v3d_basic_log_cat_printer */

void v3d_basic_log_cat_printer_line(struct v3d_basic_printer *root, const char *line)
{
   struct v3d_basic_log_cat_printer *p = (struct v3d_basic_log_cat_printer *)root;
   log_cat_msg(p->cat, p->level, "%s", line);
}

static const struct v3d_basic_printer_vtbl v3d_basic_log_cat_printer_vtbl = {
   V3D_BASIC_PRINTER_VTBL_INIT(v3d_basic_log_cat_printer)};

void v3d_basic_log_cat_printer_init(struct v3d_basic_log_cat_printer *p,
   struct log_cat *cat, log_level_t level, const char *line_prefix)
{
   v3d_basic_printer_init(&p->base, line_prefix);
   p->base.base.vtbl = &v3d_basic_log_cat_printer_vtbl.base;
   p->cat = cat;
   p->level = level;
}

/** v3d_basic_string_printer */

void v3d_basic_string_printer_line(struct v3d_basic_printer *root, const char *line)
{
   struct v3d_basic_string_printer *p = (struct v3d_basic_string_printer *)root;
   p->offset = vcos_safe_sprintf(p->buf, p->buf_size, p->offset, "%s\n", line);
}

static const struct v3d_basic_printer_vtbl v3d_basic_string_printer_vtbl = {
   V3D_BASIC_PRINTER_VTBL_INIT(v3d_basic_string_printer)};

void v3d_basic_string_printer_init(struct v3d_basic_string_printer *p,
   char *buf, size_t buf_size, size_t offset)
{
   v3d_basic_printer_init(&p->base, "");
   p->base.base.vtbl = &v3d_basic_string_printer_vtbl.base;
   p->buf = buf;
   p->buf_size = buf_size;
   p->offset = offset;
}

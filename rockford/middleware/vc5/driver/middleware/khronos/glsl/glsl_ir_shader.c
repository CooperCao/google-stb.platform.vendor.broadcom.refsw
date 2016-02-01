/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include <stdio.h>
#include <stdlib.h>

#include "glsl_errors.h"
#include "glsl_ir_shader.h"

#define DF_COPY_CHUNK_SIZE 16

typedef struct
{
   struct {
      int  new_id;
      bool seen;
   } *df;
   int df_count;

   Dataflow *blob_ptr;
   int next_new_id;
   int blob_ptr_size;
} GLSLCopyContext;

static int new_df_id(GLSLCopyContext *ctx)
{
   if (ctx->next_new_id == ctx->blob_ptr_size) {
      /* Currently full. Expand the blob */
      Dataflow *new_alloc;
      ctx->blob_ptr_size += DF_COPY_CHUNK_SIZE;
      new_alloc = realloc(ctx->blob_ptr, ctx->blob_ptr_size * sizeof(Dataflow));
      if (new_alloc == NULL) {
         free(ctx->blob_ptr);
         free(ctx->df);
         glsl_compile_error(ERROR_CUSTOM, 6, -1, NULL);
      }
      ctx->blob_ptr = new_alloc;
   }

   assert(ctx->next_new_id < ctx->blob_ptr_size);
   return ctx->next_new_id++;
}

static int copy(GLSLCopyContext *ctx, Dataflow *dataflow)
{
   Dataflow df;

   if (dataflow == NULL)
      return -1;

   assert(dataflow->id < ctx->df_count);
   if (ctx->df[dataflow->id].seen)
      return ctx->df[dataflow->id].new_id;

   df.id = new_df_id(ctx);

   ctx->df[dataflow->id].seen   = true;
   ctx->df[dataflow->id].new_id = df.id;

   df.flavour = dataflow->flavour;
   df.type = dataflow->type;
   df.dependencies_count = dataflow->dependencies_count;
   for (int i=0; i<DATAFLOW_MAX_DEPENDENCIES; i++)
      df.d.reloc_deps[i] = -1;
   for (int i=0; i<dataflow->dependencies_count; i++)
      df.d.reloc_deps[i] = copy(ctx, dataflow->d.dependencies[i]);
   memcpy(&df.u, &dataflow->u, sizeof(df.u));
   df.age = dataflow->age;

   memcpy(&ctx->blob_ptr[df.id], &df, sizeof(Dataflow));

   return df.id;
}

static void cfg_block_term(CFGBlock *b) {
   if (b == NULL) return;

   free(b->dataflow);
   free(b->outputs);
}

IRShader *glsl_ir_shader_create() {
   IRShader *ret = malloc(sizeof(IRShader));
   if(!ret) return NULL;

   ret->blocks         = NULL;
   ret->num_cfg_blocks = 0;
   ret->outputs        = NULL;
   ret->num_outputs    = 0;

   return ret;
}

void glsl_ir_shader_free(IRShader *sh)  {
   if(!sh) return;

   for (int i=0; i<sh->num_cfg_blocks; i++) {
      cfg_block_term(&sh->blocks[i]);
   }

   free(sh->blocks);
   free(sh->outputs);

   free(sh);
}

static GLSLCopyContext *glsl_ir_copy_context_new() {
   GLSLCopyContext *ctx = glsl_safemem_malloc(sizeof(GLSLCopyContext));

   ctx->next_new_id = 0;
   ctx->blob_ptr_size = DF_COPY_CHUNK_SIZE;
   ctx->blob_ptr = malloc(ctx->blob_ptr_size * sizeof(Dataflow));
   ctx->df_count = glsl_dataflow_get_count();
   ctx->df = calloc(ctx->df_count, sizeof(*ctx->df));

   if (!ctx->blob_ptr || !ctx->df) {
      free(ctx->blob_ptr);
      free(ctx->df);
      glsl_compile_error(ERROR_CUSTOM, 6, -1, NULL);
   }

   return ctx;
}

static void glsl_ir_copy_context_delete(GLSLCopyContext *ctx, Dataflow **df_out, int *df_count_out) {
   free(ctx->df);
   *df_out = ctx->blob_ptr;
   *df_count_out = ctx->next_new_id;

   glsl_safemem_free(ctx);
}

bool glsl_ir_copy_block(CFGBlock *b, Dataflow **dataflow_in, int count) {
   b->num_outputs = count;
   b->outputs = malloc(count * sizeof(int));
   if (!b->outputs) { return false; }

   GLSLCopyContext *ctx = glsl_ir_copy_context_new();
   for (int i=0; i<count; i++) {
      b->outputs[i] = copy(ctx, dataflow_in[i]);
   }
   glsl_ir_copy_context_delete(ctx, &b->dataflow, &b->num_dataflow);
   return true;
}

IRShader *glsl_ir_shader_from_blocks(CFGBlock *blocks, int num_blocks, IROutput *outputs, int num_outputs) {
   IRShader *sh_out = glsl_ir_shader_create();
   if (sh_out == NULL) goto fail;

   bool out_of_memory = false;
   sh_out->blocks = malloc(num_blocks * sizeof(CFGBlock));
   if (sh_out->blocks == NULL) goto fail;
   for (int i=0; i<num_blocks; i++) {
      sh_out->blocks[i].dataflow = malloc(blocks[i].num_dataflow * sizeof(Dataflow));
      sh_out->blocks[i].outputs  = malloc(blocks[i].num_outputs  * sizeof(int));
      out_of_memory = (!sh_out->blocks[i].dataflow || !sh_out->blocks[i].outputs);

      if (!out_of_memory) {
         memcpy(sh_out->blocks[i].dataflow, blocks[i].dataflow, blocks[i].num_dataflow * sizeof(Dataflow));
         memcpy(sh_out->blocks[i].outputs,  blocks[i].outputs,  blocks[i].num_outputs  * sizeof(int));
      }
      sh_out->blocks[i].num_dataflow = blocks[i].num_dataflow;
      sh_out->blocks[i].num_outputs  = blocks[i].num_outputs;
      sh_out->blocks[i].successor_condition = blocks[i].successor_condition;
      sh_out->blocks[i].next_if_true = blocks[i].next_if_true;
      sh_out->blocks[i].next_if_false = blocks[i].next_if_false;
   }
   sh_out->num_cfg_blocks = num_blocks;
   if (out_of_memory) goto fail;

   sh_out->outputs = malloc(num_outputs * sizeof(IROutput));
   if (sh_out->outputs == NULL) goto fail;
   memcpy(sh_out->outputs, outputs, num_outputs * sizeof(IROutput));
   sh_out->num_outputs = num_outputs;

   return sh_out;

fail:
   glsl_ir_shader_free(sh_out);
   return NULL;
}

static void cfg_block_to_file(const CFGBlock *b, FILE *f) {
   fwrite(&b->successor_condition, sizeof(b->successor_condition), 1, f);
   fwrite(&b->next_if_true,        sizeof(b->next_if_true),        1, f);
   fwrite(&b->next_if_false,       sizeof(b->next_if_false),       1, f);

   fwrite(&b->num_dataflow, sizeof(b->num_dataflow), 1,               f);
   fwrite(&b->num_outputs,  sizeof(b->num_outputs),  1,               f);

   fwrite(b->dataflow,      sizeof(Dataflow),        b->num_dataflow, f);
   fwrite(b->outputs,       sizeof(b->outputs[0]),   b->num_outputs,  f);
}

static bool cfg_block_from_file(CFGBlock *b, FILE *f) {
   fread(&b->successor_condition, sizeof(b->successor_condition), 1, f);
   fread(&b->next_if_true,        sizeof(b->next_if_true),        1, f);
   fread(&b->next_if_false,       sizeof(b->next_if_false),       1, f);

   fread(&b->num_dataflow, sizeof(b->num_dataflow), 1,                f);
   b->dataflow = malloc(b->num_dataflow * sizeof(Dataflow));

   fread(&b->num_outputs,  sizeof(b->num_outputs),  1,                f);
   b->outputs = malloc(b->num_outputs * sizeof(int));

   if (!b->dataflow || !b->outputs) {
      free(b->dataflow);
      free(b->outputs);
      return false;
   }

   fread(b->dataflow,      sizeof(Dataflow),        b->num_dataflow,  f);
   fread(b->outputs,       sizeof(b->outputs[0]),   b->num_outputs,  f);

   return true;
}

void glsl_ir_shader_to_file(const IRShader *sh, const char *fname)
{
   FILE *f = fopen(fname, "w");

   fwrite(&sh->num_cfg_blocks, sizeof(sh->num_cfg_blocks), 1, f);
   for (int i=0; i<sh->num_cfg_blocks; i++)
      cfg_block_to_file(&sh->blocks[i], f);

   fwrite(&sh->num_outputs, sizeof(sh->num_outputs), 1, f);
   fwrite(sh->outputs,      sizeof(sh->outputs[0]), sh->num_outputs, f);

   fclose(f);
}

IRShader *glsl_ir_shader_from_file(const char *fname)
{
   IRShader *ret;

   FILE *f = fopen(fname, "r");
   if (f == NULL) return NULL;

   ret = glsl_ir_shader_create();
   if(!ret) goto fail;

   fread(&ret->num_cfg_blocks, sizeof(ret->num_cfg_blocks), 1, f);
   ret->blocks = malloc(ret->num_cfg_blocks * sizeof(CFGBlock));
   if (ret->blocks == NULL) goto fail;

   bool oom = false;
   for (int i=0; i<ret->num_cfg_blocks; i++) {
      if (!cfg_block_from_file(&ret->blocks[i], f)) oom = true;
   }
   if (oom) goto fail;

   fread(&ret->num_outputs, sizeof(ret->num_outputs), 1, f);
   ret->outputs = malloc(ret->num_outputs * sizeof(IROutput));
   if (!ret->outputs) goto fail;
   fread(ret->outputs, sizeof(IROutput), ret->num_outputs, f);

   fclose(f);
   return ret;

fail:
   fclose(f);
   glsl_ir_shader_free(ret);
   return NULL;
}

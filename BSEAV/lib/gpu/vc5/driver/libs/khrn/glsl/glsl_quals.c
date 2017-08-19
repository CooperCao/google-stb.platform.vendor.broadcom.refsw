/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glsl_common.h"
#include "glsl_fastmem.h"
#include "glsl_globals.h"
#include "glsl_builders.h"    /* For create_mixed_lq. TODO: Move here? */
#include "glsl_primitive_types.auto.h"    /* For making gl_WorkGroupSize */
#include "glsl_intern.h"                  /* Ditto */
#include "glsl_layout.h"

#include "../glxx/glxx_int_config.h"

QualList *qual_list_new(Qualifier *head) {
   QualList *ret = malloc_fast(sizeof(QualList));
   QualListNode *n = malloc_fast(sizeof(QualListNode));
   n->q = head;
   n->next = NULL;
   ret->head = ret->tail = n;
   return ret;
}

QualList *qual_list_append(QualList *list, Qualifier *q) {
   QualListNode *n = malloc_fast(sizeof(QualListNode));
   n->q = q;
   n->next = NULL;

   list->tail->next = n;
   list->tail = n;
   return list;
}

Qualifier *new_qual_storage(StorageQualifier q) {
   Qualifier *ret = malloc_fast(sizeof(Qualifier));
   ret->flavour = QUAL_STORAGE;
   ret->u.storage = q;
   return ret;
}

Qualifier *new_qual_auxiliary(AuxiliaryQualifier q) {
   Qualifier *ret = malloc_fast(sizeof(Qualifier));
   ret->flavour = QUAL_AUXILIARY;
   ret->u.auxiliary = q;
   return ret;
}

Qualifier *new_qual_memory(MemoryQualifier q) {
   Qualifier *ret = malloc_fast(sizeof(Qualifier));
   ret->flavour = QUAL_MEMORY;
   ret->u.memory = q;
   return ret;
}

Qualifier *new_qual_layout(LayoutIDList *q) {
   Qualifier *ret = malloc_fast(sizeof(Qualifier));
   ret->flavour = QUAL_LAYOUT;
   ret->u.layout = q;
   return ret;
}

Qualifier *new_qual_prec(PrecisionQualifier q) {
   Qualifier *ret = malloc_fast(sizeof(Qualifier));
   ret->flavour = QUAL_PREC;
   ret->u.precision = q;
   return ret;
}

Qualifier *new_qual_interp(InterpolationQualifier q) {
   Qualifier *ret = malloc_fast(sizeof(Qualifier));
   ret->flavour = QUAL_INTERP;
   ret->u.interp = q;
   return ret;
}

Qualifier *new_qual_invariant(void) {
   Qualifier *ret = malloc_fast(sizeof(Qualifier));
   ret->flavour = QUAL_INVARIANT;
   return ret;
}

Qualifier *new_qual_precise(void) {
   Qualifier *ret = malloc_fast(sizeof(Qualifier));
   ret->flavour = QUAL_PRECISE;
   return ret;
}

static bool is_valid_es3_flavour(QualFlavour f) {
   switch (f) {
      case QUAL_STORAGE:
      case QUAL_AUXILIARY:
      case QUAL_LAYOUT:
      case QUAL_PREC:
      case QUAL_INTERP:
      case QUAL_INVARIANT:
         return true;
      default:
         return false;
   }
}

static bool is_valid_es3_storage_qual(StorageQualifier sq) {
   switch (sq) {
      case STORAGE_NONE:
      case STORAGE_CONST:
      case STORAGE_IN:
      case STORAGE_OUT:
      case STORAGE_UNIFORM:
         return true;
      default:
         return false;
   }
}

static bool validate_quallist_es3(const QualList *l) {
   static const QualFlavour valid_order[] =
            { QUAL_LAYOUT, QUAL_INVARIANT, QUAL_INTERP, QUAL_AUXILIARY, QUAL_STORAGE, QUAL_PREC };
   static const int n_locs = sizeof(valid_order) / sizeof(QualFlavour);
   int cur_loc = -1;
   bool seen[QUAL_FLAVOUR_LAST+1] = { false, };

   for (QualListNode *n = l->head; n; n=n->next) {
      if (!is_valid_es3_flavour(n->q->flavour)) return false;

      if (seen[n->q->flavour]) return false;
      seen[n->q->flavour] = true;

      if (n->q->flavour == QUAL_STORAGE) {
         if (!is_valid_es3_storage_qual(n->q->u.storage))
            return false;

         /* centroid can only appear as part of 'centroid in' or 'centroid out' */
         if (seen[QUAL_AUXILIARY]) {
            if (n->q->u.storage != STORAGE_IN && n->q->u.storage != STORAGE_OUT)
               return false;
         }
      }

      int new_loc;
      for (new_loc=0; new_loc<n_locs; new_loc++) {
         if (valid_order[new_loc] == n->q->flavour) break;
      }
      assert(new_loc < n_locs);
      if (new_loc < cur_loc) return false;
      cur_loc = new_loc;
   }

   if (seen[QUAL_LAYOUT] && (seen[QUAL_INVARIANT] || seen[QUAL_INTERP]))
      return false;
   if ((seen[QUAL_INVARIANT] || seen[QUAL_AUXILIARY]) && !seen[QUAL_STORAGE])
      return false;

   return true;
}

void qualifiers_from_list_context_sq(Qualifiers *q, QualList *l, StorageQualifier sq) {
   q->sq = STORAGE_NONE;
   q->iq = INTERP_SMOOTH;
   q->aq = AUXILIARY_NONE;
   q->lq = NULL;
   q->pq = PREC_NONE;
   q->mq = MEMORY_NONE;
   q->invariant = false;

   if (l == NULL) return;

   bool seen[QUAL_FLAVOUR_LAST+1] = { false, };

   for (QualListNode *n = l->head; n; n=n->next) {
      Qualifier *qin = n->q;

      /* TODO: Multiple precision qualifiers are not banned in the spec. Should we allow them? */
      if (seen[qin->flavour]) {
         if (qin->flavour != QUAL_MEMORY && qin->flavour != QUAL_LAYOUT)
            glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "Invalid repeated qualifiers");
      }

      seen[qin->flavour] = true;
      switch(qin->flavour) {
         case QUAL_STORAGE:
            q->sq = qin->u.storage;
            break;
         case QUAL_AUXILIARY:
            q->aq = qin->u.auxiliary;
            break;
         case QUAL_MEMORY:
            if (q->mq & qin->u.memory)
               glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "Invalid repeated memory qualifier");
            q->mq |= qin->u.memory;
            break;
         case QUAL_LAYOUT:
            q->lq = glsl_create_mixed_lq(q->lq, glsl_layout_create(qin->u.layout));
            break;
         case QUAL_PREC:
            q->pq = qin->u.precision;
            break;
         case QUAL_INTERP:
            q->iq = qin->u.interp;
            break;
         case QUAL_INVARIANT:
            q->invariant = true;
            break;
         case QUAL_PRECISE:
            /* XXX I don't think that we actually do anything that the precise
             * qualifier would prevent. */
            break;
      }
   }

   if (q->sq == STORAGE_INOUT)
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "inout is not a storage class");

   if ((seen[QUAL_INTERP] || seen[QUAL_AUXILIARY]) && !(seen[QUAL_STORAGE] || sq != STORAGE_NONE))
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "Interpolation and centroid qualifiers must have a storage qualifier");

   if (seen[QUAL_INTERP] && seen[QUAL_AUXILIARY] && q->aq == AUXILIARY_PATCH)
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "Interpolation qualifiers may not be used with 'patch'");

   if (g_ShaderVersion <= GLSL_SHADER_VERSION(3, 0, 1) && !validate_quallist_es3(l))
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "qualifiers or ordering not valid for version 300 es");

   /* Flat supersedes all the auxiliary qualifiers */
   if (q->iq == INTERP_FLAT) q->aq = AUXILIARY_NONE;
}

void qualifiers_from_list(Qualifiers *q, QualList *l) {
   qualifiers_from_list_context_sq(q, l, STORAGE_NONE);
}

static bool is_param_qualifier(StorageQualifier q) {
   return q == STORAGE_IN || q == STORAGE_OUT || q == STORAGE_INOUT;
}

static bool validate_param_quallist_es3(const QualList *l) {
   bool const_valid = true, param_valid = true, prec_valid = true;

   for (QualListNode *n = l->head; n; n=n->next) {
      if (n->q->flavour == QUAL_STORAGE && n->q->u.storage == STORAGE_CONST) {
         if (!const_valid) return false;
         const_valid = false;
      } else if (n->q->flavour == QUAL_STORAGE) {
         if (!param_valid || !is_param_qualifier(n->q->u.storage)) return false;
         const_valid = param_valid = false;
      } else if (n->q->flavour == QUAL_PREC) {
         if (!prec_valid) return false;
         const_valid = param_valid = prec_valid = false;
      } else
         return false;
   }
   return true;
}

void param_quals_from_list(Qualifiers *q, ParamQualifier *param_qual, QualList *l) {
   q->sq = STORAGE_NONE;
   q->iq = INTERP_SMOOTH;
   q->aq = AUXILIARY_NONE;
   q->lq = NULL;
   q->pq = PREC_NONE;
   q->mq = MEMORY_NONE;
   q->invariant = false;
   *param_qual = PARAM_QUAL_IN;

   if (l == NULL) return;

   if (g_ShaderVersion <= GLSL_SHADER_VERSION(3, 0, 1) && !validate_param_quallist_es3(l)) {
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "qualifiers or ordering not valid for version 300 es");
      return;
   }

   /* TODO: What are the qualifier repeating rules here? None are in the spec */
   for (QualListNode *n = l->head; n; n=n->next) {
      if (n->q->flavour != QUAL_STORAGE && n->q->flavour != QUAL_PREC &&
          n->q->flavour != QUAL_MEMORY  && n->q->flavour != QUAL_PRECISE)
      {
         glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "Invalid qualifier on parameter");
      }

      if (n->q->flavour == QUAL_STORAGE) {
         switch (n->q->u.storage) {
            case STORAGE_CONST: q->sq = STORAGE_CONST;          break;
            case STORAGE_IN:    *param_qual = PARAM_QUAL_IN;    break;
            case STORAGE_OUT:   *param_qual = PARAM_QUAL_OUT;   break;
            case STORAGE_INOUT: *param_qual = PARAM_QUAL_INOUT; break;
            default:
               glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "Invalid parameter qualifier");
               break;
         }
      } else if (n->q->flavour == QUAL_MEMORY) {
         if (q->mq & n->q->u.memory)
            glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "Invalid repeated memory qualifier");
         q->mq |= n->q->u.memory;
      } else {
         q->pq = n->q->u.precision;
      }
   }
}

/* TODO: Qualifier validation happens in too many places at the moment. */
void qualifiers_process_default(QualList *l, SymbolTable *table, DeclDefaultState *dflt) {
   if (!g_InGlobalScope)
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "Default qualifiers only valid at global scope");

   StorageQualifier sq = STORAGE_NONE;

   for (QualListNode *n = l->head; n; n=n->next) {
      if (n->q->flavour == QUAL_STORAGE) {
         sq = n->q->u.storage;
         break;
      }
   }

   if (sq == STORAGE_UNIFORM || sq == STORAGE_BUFFER) {
      Qualifiers q;
      qualifiers_from_list(&q, l);
      if (!q.lq || !(q.lq->qualified & UNIF_QUALED)) return;

      uint32_t *d = (sq == STORAGE_UNIFORM) ? &dflt->uniform_layout : &dflt->buffer_layout;
      *d  = glsl_layout_combine_block_bits(*d, q.lq->unif_bits);
   }

   if (sq == STORAGE_OUT) {
      if (dflt->output_size > 0) return;

      for (QualListNode *n = l->head; n; n=n->next) {
         if (n->q->flavour == QUAL_LAYOUT) {
            for (LayoutIDList *idn = n->q->u.layout; idn; idn=idn->next) {
               if (idn->l->id == LQ_VERTICES) dflt->output_size = idn->l->argument;
            }
         }
      }
   }

   if (sq == STORAGE_IN) {
      bool seen_gs_size   = false;
      bool seen_wg_size   = false;
      unsigned gs_size    = 0;   /* Initialised to pacify compiler */
      unsigned wg_size[3] = { 1, 1, 1};

      for (QualListNode *n = l->head; n; n=n->next) {
         if (n->q->flavour == QUAL_LAYOUT) {
            for (LayoutIDList *idn = n->q->u.layout; idn; idn=idn->next) {
               switch (idn->l->id) {
                  case LQ_POINTS:              seen_gs_size = true; gs_size = 1;                   break;
                  case LQ_LINES:               seen_gs_size = true; gs_size = 2;                   break;
                  case LQ_LINES_ADJACENCY:     seen_gs_size = true; gs_size = 4;                   break;
                  case LQ_TRIANGLES:           seen_gs_size = true; gs_size = 3;                   break;
                  case LQ_TRIANGLES_ADJACENCY: seen_gs_size = true; gs_size = 6;                   break;
                  case LQ_SIZE_X:              seen_wg_size = true; wg_size[0] = idn->l->argument; break;
                  case LQ_SIZE_Y:              seen_wg_size = true; wg_size[1] = idn->l->argument; break;
                  case LQ_SIZE_Z:              seen_wg_size = true; wg_size[2] = idn->l->argument; break;
                  default:                                                                         break;
               }
            }
         }
      }

      if (seen_gs_size && g_ShaderFlavour == SHADER_GEOMETRY) {
         dflt->input_size = gs_size;
      }

      if (seen_wg_size) {
         const char *name = glsl_intern("gl_WorkGroupSize", false);
         Symbol *existing = glsl_symbol_table_lookup(table, name);
         if (existing != NULL) {
            /* New declaration must match the old */
            for (int i=0; i<3; i++) {
               if (existing->u.var_instance.compile_time_value[i] != wg_size[i])
                  glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "Inconsistent workgroup size declarations");
            }
         } else {
            /* Add this declaration to the symbol table */
            Qualifiers q = { .invariant = false,
                             .lq = NULL,
                             .sq = STORAGE_CONST,
                             .iq = INTERP_SMOOTH,
                             .aq = AUXILIARY_NONE,
                             .pq = PREC_HIGHP,
                             .mq = MEMORY_NONE };
            const_value *v = malloc_fast(3 * sizeof(const_value));
            for (int i=0; i<3; i++) v[i] = wg_size[i];
            Symbol *s = malloc_fast(sizeof(Symbol));
            glsl_symbol_construct_var_instance(s, name, &primitiveTypes[PRIM_UVEC3], &q, v, NULL);
            glsl_symbol_table_insert(table, s);
         }
      }
   }
}

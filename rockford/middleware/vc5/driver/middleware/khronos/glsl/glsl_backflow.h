/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_BACKFLOW_H
#define GLSL_BACKFLOW_H

#include "glsl_dataflow.h"
#include "glsl_fastmem.h"
#include "glsl_gadgettype.h"
#include "glsl_list.h"
#include "glsl_ir_shader.h"
#include "glsl_binary_shader.h"
#include "interface/khronos/glxx/glxx_int_config.h"
#include "middleware/khronos/glxx/glxx_shader_cache.h"

typedef struct backflow_s Backflow;

/* Declare backflow chains */
DECLARE_NODE_LIST(Backflow,malloc_fast)

void glsl_backflow_chain_init(BackflowChain* chain);
void glsl_backflow_chain_append(BackflowChain* chain, Backflow *backflow);
void glsl_backflow_chain_remove(BackflowChain *chain, Backflow *backflow);
bool glsl_backflow_chain_contains(BackflowChain *chain, Backflow *backflow);

/* TODO: Consider separating the backend shader gen functions into their own lib */
typedef enum {
   // ALU
   BACKFLOW_ADD,
   BACKFLOW_PACK_FLOAT16,
   BACKFLOW_IADD,
   BACKFLOW_ISUB,
   BACKFLOW_SUB,
   BACKFLOW_IMIN,
   BACKFLOW_IMAX,
   BACKFLOW_UMIN,
   BACKFLOW_UMAX,
   BACKFLOW_SHL,
   BACKFLOW_SHR,
   BACKFLOW_ASHR,
   BACKFLOW_MIN,
   BACKFLOW_MAX,
   BACKFLOW_BITWISE_AND,
   BACKFLOW_BITWISE_OR,
   BACKFLOW_BITWISE_XOR,

   BACKFLOW_BITWISE_NOT,
   BACKFLOW_IARITH_NEGATE,
   BACKFLOW_FRAG_SUBMIT_MSF,
   BACKFLOW_TIDX,
   BACKFLOW_FXCD,
   BACKFLOW_XCD,
   BACKFLOW_FYCD,
   BACKFLOW_YCD,
   BACKFLOW_MSF,
   BACKFLOW_REVF,
   BACKFLOW_TMUWT,
   BACKFLOW_VPM_SETUP,
   BACKFLOW_ARITH_NEGATE,
   BACKFLOW_FCMP,

   BACKFLOW_ROUND,
   BACKFLOW_FTOI_NEAREST,
   BACKFLOW_TRUNC,
   BACKFLOW_FTOI_TRUNC,
   BACKFLOW_FLOOR,
   BACKFLOW_FTOUZ,
   BACKFLOW_CEIL,
   BACKFLOW_FTOC,
   BACKFLOW_FDX,
   BACKFLOW_FDY,

   BACKFLOW_ITOF,
   BACKFLOW_CLZ,
   BACKFLOW_UTOF,

   BACKFLOW_UMUL,
   BACKFLOW_SMUL,
   BACKFLOW_MULTOP,
   BACKFLOW_MUL,

   BACKFLOW_MOV,

   BACKFLOW_ABS,
   BACKFLOW_FMOV,
   BACKFLOW_UNPACK_16A_F,
   BACKFLOW_UNPACK_16B_F,

   BACKFLOW_UNIFORM,
   BACKFLOW_TEX_GET,
   BACKFLOW_FRAG_GET_TLB,
   BACKFLOW_FRAG_GET_TLBU,
   BACKFLOW_ATTRIBUTE,
   BACKFLOW_WRTMUC,

   BACKFLOW_VARYING,
   BACKFLOW_VARYING_LINE_COORD,
   BACKFLOW_VARYING_FLAT,

   BACKFLOW_IMUL32,
   BACKFLOW_THREADSWITCH,  /* Only appears after scheduler */
   BACKFLOW_DUMMY,         /* No op. Used to aggregate dependencies (e.g sbwait) */

   BACKFLOW_LDVPM,         /* Only usable on v3.4 */
   BACKFLOW_STVPM,

   BACKFLOW_FLAVOUR_COUNT
} BackflowFlavour;

typedef enum
{
   ALU_M,
   ALU_A, ALU_A_SWAP0, ALU_A_SWAP1,
   ALU_MOV, ALU_FMOV,
   SIG,
   SPECIAL_THRSW,
   SPECIAL_IMUL32,
   SPECIAL_VARYING,
   SPECIAL_VOID,
} ALU_TYPE_T;

struct tmu_dep_s {
   struct tmu_lookup_s *l;
   struct tmu_dep_s *next;
};

#define BACKFLOW_DEP_COUNT 4
struct backflow_s {
   int pass;

   /* This can be useful for debugging, but makes copying nodes harder */
   BackflowChain data_dependents;

   uint32_t phase; /* not visited, stacked or complete */
   uint32_t age;
   int delay;

   uint32_t reg;          /* Which register (acc or rf) result is in */
   uint32_t io_timestamp; /* Timestamp of first use. iodeps can come after this */
   uint32_t remaining_dependents;

   struct tmu_dep_s *tmu_deps;

   /* These should not be written by the backend because they won't be set
    * up again properly before the next try if threading fails.
    */
   ALU_TYPE_T type;
   uint32_t op;
   uint32_t op1;
   uint32_t op2;
   uint32_t sigbits;
   uint32_t magic_write;
   uint32_t varying;
   uint32_t cond_setf;

   uint32_t unif_type;
   uint32_t unif;

   struct backflow_s *dependencies[BACKFLOW_DEP_COUNT];
   BackflowChain io_dependencies;
   bool any_io_dependents;
};

extern void glsl_iodep(Backflow *consumer, Backflow *supplier);
extern Backflow *glsl_backflow_thrsw(void);
extern Backflow *glsl_backflow_tmuwt(void);
extern Backflow *glsl_backflow_dummy(void);

typedef enum
{
   GLSL_TRANSLATION_UNVISITED = 0, /* This should be 0 as nodes start off cleared with zeroes */
   GLSL_TRANSLATION_VOID,
   GLSL_TRANSLATION_WORD,
   GLSL_TRANSLATION_VEC4, /* Only used for texture lookup nodes */
   GLSL_TRANSLATION_BOOL_FLAG,    /* (flag==0) = (reg!=0) = False, (flag==1) = (reg==0) = True */
   GLSL_TRANSLATION_BOOL_FLAG_N,  /* (flag==0) = (reg!=0) = True,  (flag==1) = (reg==0) = False */
} GLSL_TRANSLATION_TYPE_T;

typedef struct {
   bool sample_alpha_to_coverage;
   bool sample_mask;
   bool ms;
   bool skip_discard;

   int render_target_type[V3D_MAX_RENDER_TARGETS];
} GLSL_FRAGMENT_BACKEND_STATE_T;

struct tlb_read_s {
   /* QPU does a chain of reads for each of 4 samples per RT */
   Backflow *first;
   Backflow *last;
   uint8_t samples_read;
};

struct tmu_lookup_s {
   /* Inputs (things that are filled in during translation) */
   struct tmu_dep_s *tmu_deps;

   uint32_t write_count;
   uint32_t config_count;
   uint32_t read_count;

   Backflow *first_write;
   Backflow *last_write;

   Backflow *first_read;
   Backflow *last_read;

   uint32_t age;

   struct tmu_lookup_s *next;

   /* Temporaries used only in fix_tmu_dependencies */
   bool done;
   uint32_t words_in_output_fifo;
   struct tmu_lookup_s *write_read_dependency;
   uint32_t read_section;
};

typedef struct
{
   GLSL_TRANSLATION_TYPE_T type;
   bool per_sample;
   struct backflow_s *node[4];
} GLSL_TRANSLATION_T;

typedef struct _GLSL_TRANSLATION_LIST_T {
   GLSL_TRANSLATION_T *value;
   struct _GLSL_TRANSLATION_LIST_T *next;
} GLSL_TRANSLATION_LIST_T;

/* TODO: This structure is kind of poorly thought out. It really needs to be
 * made less stage-specific and better defined.                               */
typedef struct {
   Backflow *w;
   Backflow *w_c;
   Backflow *z;

   Backflow *vertexid;
   Backflow *instanceid;

   Backflow *point_x;
   Backflow *point_y;
   Backflow *line;

   Backflow *inputs[V3D_MAX_VARYING_COMPONENTS];

   /* This dependency must be scheduled in order that reading be completed */
   Backflow *read_dep;

   /* Last read of each row from the VPM. Writes to these rows must wait until after */
#define MAX_VPM_DEPENDENCY (4*V3D_MAX_ATTR_ARRAYS + 2)
   Backflow *vpm_dep[MAX_VPM_DEPENDENCY];
} SchedShaderInputs;

typedef struct reg_list {
   Backflow *node;
   uint32_t reg;
   struct reg_list *next;
} RegList;

typedef struct ext_list {
   Backflow *node;
   int block;
   int output;
   struct ext_list *next;
} ExternalList;

typedef struct phi_list {
   const Dataflow *phi;
   struct phi_list *next;
} PhiList;

typedef struct {
   Backflow **outputs;
   int num_outputs;

   int branch_cond;     /* Output idx of branch condition */

   BackflowChain iodeps;   /* I/O that must be completed during the block */

   PhiList      *phi_list;
   ExternalList *external_list;

   bool per_sample;

   /* These are filled in during translation and used for scheduling */
   struct tmu_lookup_s *tmu_lookups;

   /* TODO: How the backend handles this and transition to per-sample is very unclear */
   /* TODO: This could only be used in translation, so should probably not live here */
   struct tlb_read_s tlb_read[V3D_MAX_RENDER_TARGETS];

   /* These are also needed by the scheduler for iodeps */
   Backflow *first_tlb_read;
   Backflow *last_tlb_read;
   Backflow *first_tlb_write;
   Backflow *last_vpm_read;      /* TODO: Employ a better strategy than having all      */
   Backflow *first_vpm_write;    /*       reads/writes in the first/last thread section */
} SchedBlock;

typedef struct
{
   int v3d_version;

   const Dataflow *df_arr;

   /* Translated dataflow (looked up by dataflow->id) */
   GLSL_TRANSLATION_T *translations;
   int translations_count;

   /* These translations need to be done multiple times during multisampling */
   GLSL_TRANSLATION_LIST_T *per_sample_clear_list;
   int sample_num;      /* Which sample we're currently translating */

   glsl_gadgettype_t gadgettype[GLXX_CONFIG_MAX_TEXTURE_UNITS];

   const LinkMap *link_map;

   SchedShaderInputs *in;
   SchedBlock *block;
} GLSL_TRANSLATE_CONTEXT_T;

extern Backflow *glsl_backflow_fake_tmu(SchedBlock *b);

extern GLSL_TRANSLATION_T *glsl_translate_to_backend(GLSL_TRANSLATE_CONTEXT_T *stuff,
                                                     int df_idx);

extern void fragment_shader_inputs(SchedShaderInputs *ins,
                                   uint32_t primitive_type,
                                   const VARYING_INFO_T *varying);

extern void vertex_shader_inputs(SchedShaderInputs *ins,
                                 const ATTRIBS_USED_T *attr_info,
                                 uint32_t *reads_total);

SchedBlock *translate_block(const CFGBlock *b_in, const LinkMap *link_map,
                            const bool *output_active, SchedShaderInputs *ins,
                            const GLXX_LINK_RESULT_KEY_T *key, int v3d_version);

extern void glsl_fragment_translate(
   SchedBlock *block,
   int block_id,
   const IRShader *sh,
   const LinkMap *link_map,
   const GLXX_LINK_RESULT_KEY_T *key,
   bool *does_discard_out,
   bool *does_z_change_out);

extern void glsl_vertex_translate(
   SchedBlock *block,
   int block_id,
   const IRShader *sh,
   const LinkMap *link_map,
   SchedShaderInputs *ins,
   const GLXX_LINK_RESULT_KEY_T *key,
   glsl_binary_shader_flavour_t shader_flavour,
   const GLSL_VARY_MAP_T *vary_map);

typedef struct {
   int size;
   int used;

   Backflow **nodes;
} BackflowPriorityQueue;

extern void glsl_backflow_priority_queue_init(BackflowPriorityQueue* queue, int size);
extern void glsl_backflow_priority_queue_heapify(BackflowPriorityQueue* queue);

extern void glsl_backflow_priority_queue_push(BackflowPriorityQueue* queue, Backflow *node);
extern Backflow *glsl_backflow_priority_queue_pop(BackflowPriorityQueue* queue);

#endif // GLSL_BACKFLOW_H

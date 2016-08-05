/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
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
#include "../glxx/glxx_int_config.h"
#include "../glxx/glxx_shader_cache.h"

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
   BACKFLOW_VFPACK,
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
   BACKFLOW_ROR,
   BACKFLOW_MIN,
   BACKFLOW_MAX,
   BACKFLOW_AND,
   BACKFLOW_OR,
   BACKFLOW_XOR,

   BACKFLOW_NOT,
   BACKFLOW_INEG,
   BACKFLOW_SETMSF,
   BACKFLOW_TIDX,
   BACKFLOW_EIDX,
   BACKFLOW_FL,
   BACKFLOW_FLN,
   BACKFLOW_FXCD,
   BACKFLOW_XCD,
   BACKFLOW_FYCD,
   BACKFLOW_YCD,
   BACKFLOW_MSF,
   BACKFLOW_REVF,
#if V3D_HAS_TNG
   BACKFLOW_IID,
#endif
#if V3D_HAS_SRS
   BACKFLOW_SAMPID,
#endif
#if V3D_HAS_TCS_BARRIER
   BACKFLOW_PATCHID,
#endif
   BACKFLOW_TMUWT,
#if V3D_HAS_TCS_BARRIER
   BACKFLOW_VPMWT,
#endif

#if V3D_HAS_LDVPM
   BACKFLOW_LDVPMV_IN,
# if V3D_HAS_TCS_BARRIER
   BACKFLOW_LDVPMV_OUT,
# endif
   BACKFLOW_LDVPMD_IN,
# if V3D_HAS_TCS_BARRIER
   BACKFLOW_LDVPMD_OUT,
# endif
   BACKFLOW_LDVPMP,
   BACKFLOW_LDVPMG_IN,
   BACKFLOW_LDVPMG_OUT,
   BACKFLOW_STVPMV,
   BACKFLOW_STVPMD,
   BACKFLOW_STVPMP,
#else
   BACKFLOW_VPMSETUP,
#endif
   BACKFLOW_NEG,
   BACKFLOW_FCMP,

   BACKFLOW_ROUND,
   BACKFLOW_FTOIN,
   BACKFLOW_TRUNC,
   BACKFLOW_FTOIZ,
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
   BACKFLOW_FMOV,
   BACKFLOW_MOV,
   BACKFLOW_MUL,

   /* TODO: These are not real flavours. They are a hack for converting */
   BACKFLOW_IMUL32,
   BACKFLOW_THREADSWITCH,  /* Only appears after scheduler */
   BACKFLOW_DUMMY,         /* No op. Used to aggregate dependencies (e.g sbwait) */

   BACKFLOW_FLAVOUR_COUNT
} BackflowFlavour;

typedef enum
{
   ALU_M, ALU_A,
   ALU_MOV, ALU_FMOV,
   SIG,
   SPECIAL_THRSW,
   SPECIAL_IMUL32,
   SPECIAL_VARYING,
   SPECIAL_VOID,
} SchedNodeType;

typedef enum {
   VARYING_DEFAULT    = 1,
   VARYING_LINE_COORD = 2,
   VARYING_FLAT       = 3
} VaryingType;

typedef enum {
   SIGBIT_THRSW  = (1<<0),
   SIGBIT_LDUNIF = (1<<1),
   SIGBIT_LDTMU  = (1<<2),
   SIGBIT_LDVARY = (1<<3),
#if !V3D_HAS_LDVPM
   SIGBIT_LDVPM  = (1<<4),
#endif
   SIGBIT_IMMED  = (1<<5),
   SIGBIT_LDTLB  = (1<<6),
   SIGBIT_LDTLBU = (1<<7),
   SIGBIT_UCB    = (1<<8),
   SIGBIT_ROTATE = (1<<9),
#if V3D_HAS_NEW_TMU_CFG
   SIGBIT_WRTMUC = (1<<10)
#endif
} SigFlavour;

typedef enum {
   UNPACK_ABS   = 0,
   UNPACK_NONE  = 1,
   UNPACK_F16_A = 2,
   UNPACK_F16_B = 3
} SchedNodeUnpack;

struct tmu_dep_s {
   struct tmu_lookup_s *l;
   struct tmu_dep_s *next;
};

#define BACKFLOW_DEP_COUNT 4
struct backflow_s {
   int pass;

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
   SchedNodeType type;
   union {
      struct {
         BackflowFlavour op;
         SchedNodeUnpack unpack[2];
      } alu;

      struct {
         VaryingType type;
         uint32_t    row;
      } varying;

      SigFlavour sigbits;
   } u;

   uint32_t magic_write;
   uint32_t cond_setf;

   uint32_t unif_type;
   uint32_t unif;

   struct backflow_s *dependencies[BACKFLOW_DEP_COUNT];
   BackflowChain data_dependents;

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
   bool ms;
   bool sample_alpha_to_coverage;
   bool sample_mask;
   bool fez_safe_with_discard;
   bool early_fragment_tests;

   struct {
      int type;
      bool alpha_16_workaround;
   } rt[V3D_MAX_RENDER_TARGETS];
} GLSL_FRAGMENT_BACKEND_STATE_T;

struct tlb_read_s {
   /* QPU does a chain of reads for each of 4 samples per RT */
   Backflow *first;
   Backflow *last;
#if !V3D_HAS_SRS
   uint8_t samples_read;
#endif
};

struct tmu_lookup_s {
   bool is_modify; /* Is a write or atomic op? */

   uint32_t write_count;
   uint32_t read_count;

   Backflow *first_write;
   Backflow *last_write;

   Backflow *first_read;
   Backflow *last_read;

   uint32_t age;

   struct tmu_lookup_s *next;

   /* Temporary used only in fix_tmu_dependencies */
   bool done;
};

typedef struct
{
   GLSL_TRANSLATION_TYPE_T type;
   struct backflow_s *node[4];
#if !V3D_HAS_SRS
   bool per_sample;
#endif
} GLSL_TRANSLATION_T;

#if !V3D_HAS_SRS
typedef struct _GLSL_TRANSLATION_LIST_T {
   GLSL_TRANSLATION_T *value;
   struct _GLSL_TRANSLATION_LIST_T *next;
} GLSL_TRANSLATION_LIST_T;
#endif

/* TODO: This structure is kind of poorly thought out. It really needs to be
 * made less stage-specific and better defined.                               */
typedef struct {
   Backflow *w;
   Backflow *w_c;
   Backflow *z;

   Backflow *vertexid;
   Backflow *instanceid;
   Backflow *baseinstance;

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

   /* These are also needed by the scheduler for iodeps */
   Backflow *first_tlb_read;
   Backflow *last_tlb_read;
   Backflow *first_tlb_write;
   Backflow *last_vpm_read;      /* TODO: Employ a better strategy than having all      */
   Backflow *first_vpm_write;    /*       reads/writes in the first/last thread section */
} SchedBlock;

typedef struct
{
   const Dataflow *df_arr;

   /* Translated dataflow (looked up by dataflow->id) */
   GLSL_TRANSLATION_T *translations;
   int translations_count;

   bool ms;

#if !V3D_HAS_SRS
   /* These translations need to be done multiple times during multisampling */
   GLSL_TRANSLATION_LIST_T *per_sample_clear_list;
   int sample_num;      /* Which sample we're currently translating */
#endif

   bool *per_quad;

   /* TODO: How the backend handles this and the transition to per-sample is very unclear */
   struct tlb_read_s tlb_read[V3D_MAX_RENDER_TARGETS];

#if !V3D_VER_AT_LEAST(3,3,0,0)
   glsl_gadgettype_t gadgettype[GLXX_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS];
   glsl_gadgettype_t img_gadgettype[GLXX_CONFIG_MAX_IMAGE_UNITS];
#endif

   const LinkMap *link_map;

   SchedShaderInputs *in;
   SchedBlock *block;
} GLSL_TRANSLATE_CONTEXT_T;

extern bool glsl_sched_node_requires_regfile(BackflowFlavour op);
extern bool glsl_sched_node_admits_unpack(BackflowFlavour op);

extern Backflow *create_sig(uint32_t sigbits);

extern Backflow *create_node(BackflowFlavour flavour, SchedNodeUnpack unpack, uint32_t cond_setf, Backflow *flag,
                             Backflow *left, Backflow *right, Backflow *output);

extern Backflow *glsl_backflow_fake_tmu(SchedBlock *b);

extern void fragment_shader_inputs(SchedShaderInputs *ins,
                                   uint32_t primitive_type,
                                   const VARYING_INFO_T *varying);

extern void vertex_shader_inputs(SchedShaderInputs *ins,
                                 const ATTRIBS_USED_T *attr_info,
                                 uint32_t *reads_total);

SchedBlock *translate_block(const CFGBlock *b_in, const LinkMap *link_map,
                            const bool *output_active, SchedShaderInputs *ins,
                            const GLXX_LINK_RESULT_KEY_T *key);

extern void glsl_fragment_translate(
   SchedBlock *block,
   int block_id,
   const IRShader *sh,
   const LinkMap *link_map,
   const GLXX_LINK_RESULT_KEY_T *key,
   bool early_fragment_tests,
   bool *does_discard_out,
   bool *does_z_change_out);

extern void glsl_vertex_translate(
   SchedBlock *block,
   int block_id,
   const IRShader *sh,
   const LinkMap *link_map,
   SchedShaderInputs *ins,
   const GLXX_LINK_RESULT_KEY_T *key,
   bool bin_mode,
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

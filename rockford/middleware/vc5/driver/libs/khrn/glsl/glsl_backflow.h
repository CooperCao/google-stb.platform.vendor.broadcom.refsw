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

void glsl_backflow_chain_init(BackflowChain *chain);
void glsl_backflow_chain_append(BackflowChain *chain, Backflow *backflow);
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
   BACKFLOW_SETREVF,
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

   BACKFLOW_IADD_M,
   BACKFLOW_ISUB_M,
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

   int branch_cond;        /* Output idx of branch condition */

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
#if !V3D_HAS_LDVPM
   Backflow *last_vpm_read;      /* TODO: Employ a better strategy than having all      */
   Backflow *first_vpm_write;    /*       reads/writes in the first/last thread section */
#endif
} SchedBlock;

Backflow *create_sig(uint32_t sigbits);

Backflow *create_node(BackflowFlavour flavour, SchedNodeUnpack unpack, uint32_t cond_setf, Backflow *flag,
                      Backflow *left, Backflow *right, Backflow *output);

Backflow *tr_external(int block, int output, ExternalList **list);

void glsl_iodep(Backflow *consumer, Backflow *supplier);

bool glsl_sched_node_requires_regfile(BackflowFlavour op);
bool glsl_sched_node_admits_unpack(BackflowFlavour op);

Backflow *glsl_backflow_fake_tmu(SchedBlock *b);

void tr_read_tlb(bool ms, uint32_t rt_num, uint32_t rt_type, uint32_t required_components,
                 Backflow **result, Backflow **first_read, Backflow **last_read);

void fragment_shader_inputs(SchedShaderInputs *ins,
                            uint32_t primitive_type,
                            const VARYING_INFO_T *varying);

unsigned vertex_shader_inputs(SchedShaderInputs *ins,
                              const ATTRIBS_USED_T *attr_info);

SchedBlock *translate_block(const CFGBlock *b_in, const LinkMap *link_map,
                            const bool *output_active, SchedShaderInputs *ins,
                            const GLXX_LINK_RESULT_KEY_T *key);

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

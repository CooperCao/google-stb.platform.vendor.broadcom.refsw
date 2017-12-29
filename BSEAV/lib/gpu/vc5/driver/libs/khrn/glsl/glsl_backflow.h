/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_dataflow.h"
#include "glsl_fastmem.h"
#include "glsl_ir_shader.h"
#include "glsl_binary_shader.h"
#include "glsl_backend_uniforms.h"
#include "glsl_backend_reg.h"

#include "libs/core/v3d/v3d_qpu_instr.h"

typedef struct backflow_s Backflow;

/* Declare BackflowChain */
#define C_LIST_MALLOC malloc_fast
#define C_LIST_FREE(p)
#define C_LIST glsl_backflow_chain
#define C_LIST_VALUE_TYPE Backflow*
#include "libs/util/c_list.h"
typedef glsl_backflow_chain      BackflowChain;
typedef glsl_backflow_chain_node BackflowChainNode;

/* Declare BackflowIODepChain */
typedef struct BackflowIODep
{
   Backflow* dep;
   int io_timestamp_offset;   // offset relative to Backflow.io_timestamp
} BackflowIODep;
#define C_LIST_MALLOC malloc_fast
#define C_LIST_FREE(p)
#define C_LIST glsl_backflow_iodep_chain
#define C_LIST_VALUE_TYPE BackflowIODep
#include "libs/util/c_list.h"
typedef glsl_backflow_iodep_chain      BackflowIODepChain;
typedef glsl_backflow_iodep_chain_node BackflowIODepChainNode;

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
#if V3D_VER_AT_LEAST(4,0,2,0)
   BACKFLOW_IID,
   BACKFLOW_SAMPID,
   BACKFLOW_BARRIERID,
#endif
   BACKFLOW_TMUWT,
#if V3D_VER_AT_LEAST(4,0,2,0)
   BACKFLOW_VPMWT,

   BACKFLOW_LDVPMV_IN,
   BACKFLOW_LDVPMV_OUT,
   BACKFLOW_LDVPMD_IN,
   BACKFLOW_LDVPMD_OUT,
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

typedef enum {
   ALU_M, ALU_A,
   SIG,
   SPECIAL_THRSW,
   SPECIAL_IMUL32,
   SPECIAL_VARYING,
   SPECIAL_VOID,
} SchedNodeType;

typedef enum {
   VARYING_DEFAULT = 1,
   VARYING_NOPERSP = 2,
   VARYING_FLAT    = 3
} VaryingType;

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
   uint32_t age;
   struct tmu_dep_s *tmu_deps;

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

      v3d_qpu_sigbits_t sigbits;
   } u;

   backend_reg magic_write;
   uint32_t cond_setf;

   BackendUniformFlavour unif_type;
   uint32_t unif;

   struct backflow_s *dependencies[BACKFLOW_DEP_COUNT];

   BackflowIODepChain io_dependencies;
   bool any_io_dependents;
};

struct tmu_lookup_s {
   bool is_modify; /* Is a write or atomic op? */

   uint32_t write_count;
   uint32_t read_count;

   Backflow *first_write;
   Backflow *last_write; // This must point at the retiring write node directly

   Backflow *first_read;
   Backflow *last_read;

   uint32_t age;

   struct tmu_lookup_s *next;

   /* Temporary used only in fix_tmu_dependencies */
   bool done;
};

#if V3D_VER_AT_LEAST(4,1,34,0)
typedef struct ldunifa_lookup
{
   struct ldunifa_lookup* next;
   Backflow* write_unifa;
   Backflow* last_ldunifa;
} ldunifa_lookup;
#endif

/* TODO: This structure is kind of poorly thought out. It really needs to be
 * made less stage-specific and better defined.                               */
typedef struct {
   Backflow *rf0; // w;
   Backflow *rf1; // w_c;
   Backflow *rf2; // z;

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

   Backflow *branch_cond;
   bool branch_invert;
   bool branch_per_quad;

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
#if !V3D_VER_AT_LEAST(4,0,2,0)
   Backflow *last_vpm_read;      /* TODO: Employ a better strategy than having all      */
   Backflow *first_vpm_write;    /*       reads/writes in the first/last thread section */
#endif
#if V3D_VER_AT_LEAST(4,1,34,0)
   ldunifa_lookup* ldunifa_lookups;
#endif
} SchedBlock;

Backflow *create_sig(uint32_t sigbits);

Backflow *create_node(BackflowFlavour flavour, SchedNodeUnpack unpack, uint32_t cond_setf, Backflow *flag,
                      Backflow *left, Backflow *right, Backflow *output);

Backflow *tr_external(int block, int output, ExternalList **list);

void glsl_iodep_offset(Backflow *consumer, Backflow *supplier, int io_timestamp_offset);
void glsl_iodep(Backflow *consumer, Backflow *supplier);

bool glsl_sched_node_requires_regfile(const Backflow *node);
bool glsl_sched_node_admits_unpack(BackflowFlavour op);

void tr_read_tlb(bool ms, uint32_t rt_num, bool is_16, bool is_int, uint32_t required_components,
                 Backflow **result, Backflow **first_read, Backflow **last_read);

void fragment_shader_inputs(SchedShaderInputs *ins,
                            uint32_t primitive_type,
                            bool ignore_centroids,
                            const VARYING_INFO_T *varying);

unsigned vertex_shader_inputs(SchedShaderInputs *ins,
                              const ATTRIBS_USED_T *attr_info);

SchedBlock *translate_block(const CFGBlock *b_in, const LinkMap *link_map,
                            const bool *output_active, SchedShaderInputs *ins,
                            const struct glsl_backend_cfg *key, const bool *per_quad);

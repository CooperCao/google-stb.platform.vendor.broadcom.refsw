import sys, re
from collections import namedtuple
from generation_utils import *
from parsers import parse_table

ValueType     = namedtuple('ValueType',   [ 'scalar_type', 'index_count', 'indexed_type', 'flags', 'gl_type' ])
sampler_flags = set(["sampler_type"])
image_flags   = set(["image_type"])
atomic_flags  = set(["atomic_type"])

### Deriving basic data from the primitive type information

def find_value_types(scalar_types):
    ret = {}
    for s,t in scalar_types.iteritems():
        f = "%s_type" % s
        ret[s] = ValueType(scalar_type=s, index_count=1, indexed_type=None, flags=t.operations | set([f,"scalar_type"]), gl_type="GL_%s" % t.gl_type.upper())
        if t.max_vector != None and t.max_vector > 1:
            for i in range(t.max_vector-1):
                vec_name = "%s%d" % (t.vector_prefix, i+2)
                ret[vec_name] = ValueType(scalar_type=s, index_count=i+2, indexed_type=s, flags=t.vector_operations | set([f,"vector_type"]), gl_type="GL_%s_VEC%d" % (t.gl_type.upper(),i+2))
        if t.max_matrix != None and t.max_matrix > 1:
            for i in range(t.max_matrix-1):
                for j in range(t.max_matrix-1):
                    if i == j:
                        mat_idx = "%d" % (i+2)
                    else:
                        mat_idx = "%dx%d" % (i+2,j+2)
                    mat_name   = "%s%s" % (t.matrix_prefix,mat_idx)
                    index_name = "%s%d" % (t.vector_prefix,j+2)
                    ret[mat_name] = ValueType(scalar_type=s, index_count=i+2, indexed_type=index_name, flags=t.matrix_operations | set([f,"matrix_type"]), gl_type="GL_%s_MAT%s" % (s.upper(),mat_idx))
    return ret

def find_matrix_types(value_type_map):
    matrix_types = []
    for n,t in value_type_map.iteritems():
        if t.indexed_type != None and t.scalar_type != t.indexed_type:
            matrix_types.append(n)
    return matrix_types

def find_scalar_count(prim_name):
    scalarcount = 1
    idxtype = prim_name
    while True:
        scalarcount *= value_types[idxtype].index_count
        idxtype      = value_types[idxtype].indexed_type
        if idxtype == None:
            break
    return scalarcount

def find_ordered_primitive_types(scalar_types,
                                 value_types,
                                 sampler_types,
                                 image_types):
    ret = ["void"]
    sorted_scalar_types = scalar_types.keys()
    sorted_scalar_types.sort()
    for s in sorted_scalar_types:
        values_for_scalar_type = []
        for name,valuetype in value_types.iteritems():
            if valuetype.scalar_type == s:
                values_for_scalar_type.append(name)
        values_for_scalar_type.sort()
        ret.extend(values_for_scalar_type)
    ret.extend(["atomic_uint"])
    sampler_names = sampler_types.keys()
    sampler_names.sort()
    ret.extend(sampler_names)
    image_names = image_types.keys()
    image_names.sort()
    ret.extend(image_names)
    return ret

def find_flags(value_types, sampler_flags, image_flags, atomic_flags):
    ret = set()
    ret.update(sampler_flags)
    ret.update(image_flags)
    ret.update(atomic_flags)
    for v in value_types.values():
        ret.update(v.flags)
    return ret

### Naming

def prim_index(p):
    return "PRIM_%s" % p.upper()

def df_index(d):
    return "DF_%s" % d.upper()

### Producing parts of the file

def print_header_api(outf, scalar_types):
    print_banner(outf, ["The callable API for primitive types"])
    print >>outf, "void               glsl_prim_init();"
    print >>outf, "PrimSamplerInfo   *glsl_prim_get_sampler_info(PrimitiveTypeIndex idx);"
    print >>outf, "PrimSamplerInfo   *glsl_prim_get_image_info  (PrimitiveTypeIndex idx);"
    print >>outf, "DataflowType       glsl_prim_index_to_df_type(PrimitiveTypeIndex idx);"
    print >>outf, "unsigned int       glsl_prim_matrix_type_subscript_dimensions(PrimitiveTypeIndex index, int dimension);"
    print >>outf, "PrimitiveTypeIndex glsl_prim_matrix_type_subscript_vector    (PrimitiveTypeIndex index, int dimension);"
    print >>outf
    print >>outf, "static inline bool glsl_prim_is_prim_sampler_type(const SymbolType *type) {"
    print >>outf, "   if(type->flavour != SYMBOL_PRIMITIVE_TYPE) return false;"
    print >>outf, "   return (primitiveTypeFlags[type->u.primitive_type.index] & PRIM_SAMPLER_TYPE);"
    print >>outf, "}"
    print >>outf
    print >>outf, "static inline bool glsl_prim_is_prim_image_type(const SymbolType *type) {"
    print >>outf, "   if(type->flavour != SYMBOL_PRIMITIVE_TYPE) return false;"
    print >>outf, "   return (primitiveTypeFlags[type->u.primitive_type.index] & PRIM_IMAGE_TYPE);"
    print >>outf, "}"
    print >>outf
    print >>outf, "static inline bool glsl_prim_is_prim_atomic_type(const SymbolType *type) {"
    print >>outf, "   if(type->flavour != SYMBOL_PRIMITIVE_TYPE) return false;"
    print >>outf, "   return (primitiveTypeFlags[type->u.primitive_type.index] & PRIM_ATOMIC_TYPE);"
    print >>outf, "}"
    print >>outf
    print >>outf, "static inline bool glsl_prim_is_opaque_type(const SymbolType *type) {"
    print >>outf, "   return glsl_prim_is_prim_sampler_type(type) ||"
    print >>outf, "          glsl_prim_is_prim_image_type(type)   ||"
    print >>outf, "          glsl_prim_is_prim_atomic_type(type);"
    print >>outf, "}"
    print >>outf
    print >>outf, "static inline bool glsl_prim_is_prim_with_base_type(const SymbolType *type, PrimitiveTypeIndex base_idx) {"
    print >>outf, "   if(type->flavour != SYMBOL_PRIMITIVE_TYPE) return false;"
    print >>outf, "   return primitiveScalarTypeIndices[type->u.primitive_type.index] == base_idx;"
    print >>outf, "}"
    print >>outf
    print >>outf, "static inline SymbolType *glsl_prim_vector_type(PrimitiveTypeIndex base_idx, int scalar_count) {"
    print >>outf, "   if(scalar_count < 1 || scalar_count > primitiveTypeVectorCount[base_idx]) {"
    print >>outf, "      return NULL;"
    print >>outf, "   }"
    print >>outf, "   return &primitiveTypes[primitiveTypeVectors[base_idx][scalar_count-1]];"
    print >>outf, "}"
    print >>outf
    print >>outf, "static inline bool glsl_prim_is_vector_type(const SymbolType *type, PrimitiveTypeIndex base_idx, int scalar_count) {"
    print >>outf, "   return type == glsl_prim_vector_type(base_idx, scalar_count) ;"
    print >>outf, "}"
    print >>outf
    print >>outf, "static inline SymbolType *glsl_prim_same_shape_type(const SymbolType *type, PrimitiveTypeIndex base_idx) {"
    print >>outf, "   SymbolType *new_type;"
    print >>outf, "   if(type->flavour != SYMBOL_PRIMITIVE_TYPE) {"
    print >>outf, "      return NULL;"
    print >>outf, "   }"
    print >>outf, "   new_type = &primitiveTypes[base_idx];"
    print >>outf, "   if(primitiveTypeSubscriptTypes[type->u.primitive_type.index] == NULL) {"
    print >>outf, "      return new_type;"
    print >>outf, "   } else {"
    print >>outf, "      const SymbolType *dim1_type = primitiveTypeSubscriptTypes[type->u.primitive_type.index];"
    print >>outf, "      if(primitiveTypeSubscriptTypes[dim1_type->u.primitive_type.index] == NULL) {"
    print >>outf, "         return glsl_prim_vector_type(new_type->u.primitive_type.index,"
    print >>outf, "                                     primitiveTypeSubscriptDimensions[type->u.primitive_type.index]);"
    print >>outf, "      } else {"
    print >>outf, "          new_type = glsl_prim_vector_type(new_type->u.primitive_type.index,"
    print >>outf, "                                           primitiveTypeSubscriptDimensions[dim1_type->u.primitive_type.index]);"
    print >>outf, "          new_type = glsl_prim_vector_type(new_type->u.primitive_type.index,"
    print >>outf, "                                           primitiveTypeSubscriptDimensions[type->u.primitive_type.index]);"
    print >>outf, "          return new_type;"
    print >>outf, "      }"
    print >>outf, "   }"
    print >>outf, "}"
    print >>outf
    print >>outf, "static inline bool glsl_prim_is_base_of_prim_type(const SymbolType *compound, const SymbolType *base) {"
    print >>outf, "   const SymbolType *cur;"
    print >>outf, "   if(compound->flavour != SYMBOL_PRIMITIVE_TYPE || base->flavour != SYMBOL_PRIMITIVE_TYPE) {"
    print >>outf, "      return false;"
    print >>outf, "   }"
    print >>outf, "   for(cur = compound;"
    print >>outf, "       primitiveTypeSubscriptTypes[cur->u.primitive_type.index];"
    print >>outf, "       cur = primitiveTypeSubscriptTypes[cur->u.primitive_type.index]);"
    print >>outf, "   return base == cur;"
    print >>outf, "}"
    print >>outf
    print >>outf, "static inline bool glsl_prim_is_prim_with_same_shape_type(SymbolType *type, PrimitiveTypeIndex base_idx) {"
    print >>outf, "   return glsl_prim_same_shape_type(type, base_idx) != NULL;"
    print >>outf, "}"

    print >>outf, "static inline bool glsl_prim_is_scalar_type(PrimitiveTypeIndex type_index) {"
    print >>outf, "   return %s;" % (
        " || ".join(["type_index == %s" % prim_index(s) for s in scalar_types]))
    print >>outf, "}"


def print_globals(outf, matrix_types):
    print_banner(outf, ["Globally visible arrays containing primitive type data"])
    max_dimension = max([value_types[m].index_count for m in matrix_types])+1
    print >>outf, "extern GLenum                    primitiveTypesToGLenums         [PRIMITIVE_TYPES_COUNT];"
    print >>outf, "extern SymbolType                primitiveTypes                  [PRIMITIVE_TYPES_COUNT];"
    print >>outf, "extern Symbol                    primitiveParamSymbols           [PRIMITIVE_TYPES_COUNT];"
    print >>outf, "extern SymbolType               *primitiveTypeSubscriptTypes     [PRIMITIVE_TYPES_COUNT];"
    print >>outf, "extern unsigned int              primitiveTypeSubscriptDimensions[PRIMITIVE_TYPES_COUNT];"
    print >>outf, "extern const PrimitiveTypeIndex  primitiveMatrixTypeIndices      [%d][%d];" % (max_dimension, max_dimension)
    print >>outf, "extern PrimitiveTypeIndex        primitiveScalarTypeIndices      [PRIMITIVE_TYPES_COUNT];"
    print >>outf, "extern PRIMITIVE_TYPE_FLAGS_T    primitiveTypeFlags              [PRIMITIVE_TYPES_COUNT];"
    print >>outf, "extern const int                 primitiveTypeVectorCount        [PRIMITIVE_TYPES_COUNT];"
    print >>outf, "extern const PrimitiveTypeIndex *primitiveTypeVectors            [PRIMITIVE_TYPES_COUNT];"
    print >>outf

def print_sampler_info(outf, sampler_names, scalar_types, value_types, sampler_types, output_name):
    type_map = {}
    for s,t in scalar_types.iteritems():
        for u,v in value_types.iteritems():
            if v.index_count == 4 and v.indexed_type == s:
                type_map[s] = u

    if len(sampler_names) == 0:
        dim = 0
    else:
        dim = "PRIMITIVE_TYPES_COUNT - %s" % prim_index(sampler_names[0])
    print >>outf, "PrimSamplerInfo %s[%s] = {"  % ( output_name, dim )
    maxnamelen = max([len(s) for s in sampler_names])
    typenames  = [prim_index(type_map[sampler_types[s].scalar_type])
                  if not sampler_types[s].shadow else prim_index(sampler_types[s].scalar_type)
                  for s in sampler_names]
    maxtypelen = max([len(t) for t in typenames])
    for i,s in enumerate(sampler_names):
        name_pad = " " * (maxnamelen - len(s))
        type_pad = " " * (maxtypelen - len(typenames[i]))
        if sampler_types[s].array:
            arraysuffix = "_ARRAY"
        else:
            arraysuffix = ""
        coords = int(sampler_types[s].dim)
        size_dim = coords
        if sampler_types[s].array:
            size_dim += 1
        if sampler_types[s].cube:
            dim_part  = "CUBE"
            size_dim -= 1
        else:
            dim_part = "%dD" % int(sampler_types[s].dim)
        print >>outf, "   { %s,%s %d, %d, %s,%s %s, %s, GLSL_TEXBITS_%s%s }," % (
            prim_index(s), name_pad, coords, size_dim, typenames[i], type_pad,
            tf(sampler_types[s].array), tf(sampler_types[s].shadow), dim_part, arraysuffix)
    print >>outf, "};"
    print >>outf

if __name__ == "__main__":
    output_dir, include_dirs, input_files = parse_opts()

    if len(input_files) != 3:
        print >>sys.stderr, "Usage: %s [options] <scalars_table> <samplers_table> <images_table>" % sys.argv[0]
        sys.exit(1)
    scalar_table  = find_file(input_files[0], include_dirs)
    sampler_table = find_file(input_files[1], include_dirs)
    image_table   = find_file(input_files[2], include_dirs)
    if scalar_table == None or sampler_table == None or image_table == None:
        print >>stderr, "Could not find required file %s" % (input_files[0] if scalar_table == None else (input_files[1] if sampler_table == None else image_table))
        sys.exit(1)

    with open(scalar_table, "r") as fp:
        lines = fp.readlines()
        ScalarType,scalar_types = parse_table("ScalarType", lines, "name")
    with open(sampler_table, "r") as fp:
        lines = fp.readlines()
        SamplerType,sampler_types = parse_table("SamplerType", lines, "name")
    with open(image_table, "r") as fp:
        lines = fp.readlines()
        ImageType,image_types = parse_table("ImageType", lines, "name")
    value_types   = find_value_types(scalar_types)
    prim_names    = find_ordered_primitive_types(
        scalar_types, value_types, sampler_types, image_types)
    matrix_types   = find_matrix_types(value_types)
    matrix_types.sort()
    sampler_names = sampler_types.keys()
    sampler_names.sort()
    image_names = image_types.keys()
    image_names.sort()
    all_flags = list(find_flags(value_types, sampler_flags, image_flags, atomic_flags))
    all_flags.sort()

    with open("glsl_primitive_type_index.auto.h", "w") as outf:
        print_inclusion_guard_start(outf, "glsl_primitive_type_index.auto.h")
        print_disclaimer(outf)
        print_enum(outf, "PrimitiveTypeIndex",
                   [prim_index(p) for p in prim_names] +
                   ["PRIMITIVE_TYPES_COUNT",
                    "PRIMITIVE_TYPE_UNDEFINED"])
        print >>outf
        print_defines(outf, [(prim_index(f), "(1<<%d)" % i) for i,f in enumerate(all_flags)])
        print >>outf
        print >>outf, "typedef unsigned int PRIMITIVE_TYPE_FLAGS_T;"
        print >>outf, """
/* Extra info about sampler types */
typedef struct {
   PrimitiveTypeIndex type;         /* The type itself, to validate the table */
   int coord_count;                 /* How many coordinates are required in lookups */
   int size_dim;                    /* How many size dimensions the texture has */
   PrimitiveTypeIndex return_type;  /* Type of data returned */
   bool array;
   bool shadow;
   uint32_t bits;                   /* The hardware bitcode associated */
} PrimSamplerInfo;
"""
        print_inclusion_guard_end(outf, "glsl_primitive_type_index.auto.h")

    with open("glsl_primitive_types.auto.h", "w") as outf:
        print_inclusion_guard_start(outf, "glsl_primitive_types.auto.h")
        print_disclaimer(outf)
        print_includes(outf, ["interface/khronos/include/GLES3/gl32.h"])
        print_includes(outf, ["glsl_symbols.h", "glsl_primitive_type_index.auto.h"])
        print_globals(outf, matrix_types)
        print_header_api(outf, scalar_types)
        print_inclusion_guard_end(outf, "glsl_primitive_types.auto.h")

    with open("glsl_primitive_types.auto.table", "w") as outf:
        print_disclaimer(outf, "#")
        table_rows = []
        for p in prim_names:
            if p in value_types:
                base = value_types[p].scalar_type
                df   = df_index(p) if value_types[p].scalar_type == p else None
                dim1 = value_types[p].index_count if p != value_types[p].scalar_type else None
                dim2 = (value_types[value_types[p].indexed_type].index_count
                        if (value_types[p].indexed_type != None and value_types[p].scalar_type != value_types[p].indexed_type) else None)
                table_rows.append([p, prim_index(p), base, df, dim1, dim2, value_types[p].gl_type])
            else:
                table_rows.append([p, prim_index(p), None, None, None, None, sampler_types[p].gl_type if p in sampler_types else "GL_NONE"])
        print_table(outf, ["name", "prim_name", "base_type", "df_type", "dim1", "dim2", "gl_type"], table_rows)

    with open("glsl_primitive_types.auto.c", "w") as outf:
        print_disclaimer(outf)
        print_includes(outf, ["glsl_common.h", "glsl_tex_params.h", "glsl_primitive_types.auto.h"])
        print >>outf, """
SymbolType*        primitiveTypeSubscriptTypes[PRIMITIVE_TYPES_COUNT] = { NULL };
SymbolType         primitiveTypes             [PRIMITIVE_TYPES_COUNT];
"""

        print_sampler_info(outf, sampler_names, scalar_types, value_types, sampler_types, "sampler_gadgets")
        print_sampler_info(outf, image_names,   scalar_types, value_types, image_types,   "image_gadgets")

        ## GLEnums
        gl_types = {}
        for p in prim_names:
            if p == "void":
                gl_types[p] = "GL_NONE"
            elif p in value_types:
                gl_types[p] = value_types[p].gl_type
            elif p == "atomic_uint":
                gl_types[p] = "GL_UNSIGNED_INT_ATOMIC_COUNTER"
            elif p in image_types:
                gl_types[p] = image_types[p].gl_type
            else:
                gl_types[p] = sampler_types[p].gl_type
        gl_enum_values = [(gl_types[p], prim_index(p)) for p in prim_names]
        print_array(outf, "GLenum", "primitiveTypesToGLenums", "PRIMITIVE_TYPES_COUNT", gl_enum_values)

        ## Subscript dimension
        subscript_dimension_values = []
        for p in prim_names:
            if p in value_types and  value_types[p].indexed_type != None:
                index_count = value_types[p].index_count
            else:
                index_count = 0
            subscript_dimension_values.append((index_count, prim_index(p)))
        print_array(outf, "unsigned int", "primitiveTypeSubscriptDimensions", "PRIMITIVE_TYPES_COUNT", subscript_dimension_values)

        scalar_indices = {}
        for p in prim_names:
            if p == "void":
                scalar_indices[p] = "PRIMITIVE_TYPE_UNDEFINED"
            elif p in value_types:
                scalar_indices[p] = "PRIM_%s" % value_types[p].scalar_type.upper()
            else:
                scalar_indices[p] = "PRIM_%s" % p.upper()
        scalar_type_values = [(scalar_indices[p],prim_index(p)) for p in prim_names]
        print_array(outf, "unsigned int", "primitiveScalarTypeIndices", "PRIMITIVE_TYPES_COUNT", scalar_type_values)

        # Primitive type flags
        array_values = []
        for p in prim_names:
            flags = set()
            if p in value_types:
                flags.update(value_types[p].flags)
            elif p in sampler_types:
                flags.update(sampler_flags)
            elif p in image_types:
                flags.update(image_flags)
            elif p == "atomic_uint":
                flags.update(atomic_flags)

            array_values.append(([prim_index(f) for f in flags], prim_index(p)) if len(flags) > 0 else ([0],prim_index(p)))
        print_flag_array(outf, "PRIMITIVE_TYPE_FLAGS_T", "primitiveTypeFlags", "PRIMITIVE_TYPES_COUNT", array_values)

        # Vector types
        vector_arrays = []
        vector_counts = []
        for p in prim_names:
            scalar_count = 1
            array_values = [(prim_index(p), "1")]
            while(True):
                scalar_count += 1
                vector_type = None
                for k,v in value_types.iteritems():
                    if v.index_count == scalar_count and v.indexed_type == p:
                        vector_type = k
                        break
                else:
                    break
                array_values.append((prim_index(vector_type), "%d" % scalar_count))
            array_name = "primitiveTypeVectors_%s" % prim_index(p)
            print_array(outf, "const PrimitiveTypeIndex", array_name, "%d" % (scalar_count-1), array_values)
            vector_arrays.append((array_name, prim_index(p)))
            vector_counts.append((scalar_count-1, prim_index(p)))
        print_array(outf, "const PrimitiveTypeIndex*", "primitiveTypeVectors", "PRIMITIVE_TYPES_COUNT", vector_arrays)
        print_array(outf, "const int", "primitiveTypeVectorCount", "PRIMITIVE_TYPES_COUNT", vector_counts)

        # Matrix minor types
        max_dimension = max([value_types[m].index_count for m in matrix_types])+1
        max_len       = max(len(m) for m in matrix_types)
        array_values  = []
        for i in range(max_dimension):
            row = []
            for j in range(max_dimension):
                mtype = "void"
                for m in matrix_types:
                    if value_types[m].index_count == i and value_types[value_types[m].indexed_type].index_count == j:
                        mtype = m
                        break
                row.append(prim_index(mtype))
            array_values.append((row, "%d rows" % i))
        print_2d_array(outf, "const PrimitiveTypeIndex", "primitiveMatrixTypeIndices", max_dimension,max_dimension, array_values)

        #### Functions
        print >>outf, "unsigned int glsl_prim_matrix_type_subscript_dimensions(PrimitiveTypeIndex type_idx, int dim_idx) {"
        print >>outf, "   int dim;"
        print >>outf
        print >>outf, "   switch(type_idx) {"
        maxlen = max([len(s) for s in matrix_types])
        for m in matrix_types:
            major_dim = value_types[m].index_count
            minor_dim = value_types[value_types[m].indexed_type].index_count
            pad       = " " * (maxlen - len(m))
            if minor_dim == major_dim:
                print >>outf, "   case %s:%s dim = %d; break;" % (
                    prim_index(m), pad, major_dim)
            else:
                print >>outf, "   case %s:%s dim = (dim_idx == 0) ? %d : %d; break;" % (
                    prim_index(m), pad, major_dim, minor_dim)

        print >>outf, "   default: dim = 0; break;"
        print >>outf, "   }"
        print >>outf, "  return dim;"
        print >>outf, "}"

        print >>outf, "unsigned int glsl_prim_matrix_type_subscript_vector(PrimitiveTypeIndex type_idx, int dim_idx) {"
        print >>outf, "   PrimitiveTypeIndex sub;"
        print >>outf
        print >>outf, "   switch(type_idx) {"
        maxlen = max([len(s) for s in matrix_types])
        for m in matrix_types:
            major_dim = value_types[m].index_count
            minor_dim = value_types[value_types[m].indexed_type].index_count
            pad       = " " * (maxlen - len(m))
            if minor_dim == major_dim:
                print >>outf, "   case %s:%s sub = %s; break;" % (
                    prim_index(m), pad, prim_index(value_types[m].indexed_type))
            else:
                target_indexed_type = value_types[value_types[m].indexed_type].indexed_type
                for n,v in value_types.iteritems():
                    if v.index_count == major_dim and v.indexed_type == target_indexed_type:
                        minor_indexed_type = n
                        break
                else:
                    assert(False) # Missing minor index type for matrix type m
                print >>outf, "   case %s:%s sub = (dim_idx == 0) ? %s : %s; break;" % (
                    prim_index(m), pad,
                    prim_index(minor_indexed_type),
                    prim_index(value_types[m].indexed_type))

        print >>outf, "   default: sub = PRIM_VOID; break;"
        print >>outf, "   }"
        print >>outf
        print >>outf, "   return sub;"
        print >>outf, "}"

        print >>outf, "PrimSamplerInfo *glsl_prim_get_sampler_info(PrimitiveTypeIndex idx) {"
        print >>outf, "   assert(idx >= %s && idx < PRIMITIVE_TYPES_COUNT);" % prim_index(sampler_names[0])
        print >>outf, "   assert(sampler_gadgets[idx - %s].type == idx);" % prim_index(sampler_names[0])
        print >>outf, "   return &sampler_gadgets[idx - %s];" % prim_index(sampler_names[0])
        print >>outf, "}"
        print >>outf
        print >>outf, "PrimSamplerInfo *glsl_prim_get_image_info(PrimitiveTypeIndex idx) {"
        print >>outf, "   assert(idx >= %s && idx < PRIMITIVE_TYPES_COUNT);" % prim_index(image_names[0])
        print >>outf, "   assert(image_gadgets[idx - %s].type == idx);" % prim_index(image_names[0])
        print >>outf, "   return &image_gadgets[idx - %s];" % prim_index(image_names[0])
        print >>outf, "}"
        print >>outf
        print >>outf, "DataflowType glsl_prim_index_to_df_type(PrimitiveTypeIndex pti) {"
        print >>outf, "   switch (pti) {"
        pure_types = ["void"] + scalar_types.keys()
        maxlen = max([len(d) for d in pure_types])
        for s in pure_types:
            pad = " " * (maxlen - len(s))
            print >>outf, "   case %s:%s return %s;" % (prim_index(s), pad, df_index(s))
        print >>outf, "   default: UNREACHABLE(); return DF_INVALID;"
        print >>outf, "   }"
        print >>outf, "}"

        ## The API
        print_banner(outf, ["The API functions for primitive types"])
        print >>outf, """\
void glsl_prim_init(void) {
   for (int i = 0; i < PRIMITIVE_TYPES_COUNT; ++i) {
      primitiveTypes[i].flavour                = SYMBOL_PRIMITIVE_TYPE;
      primitiveTypes[i].u.primitive_type.index = i;
   }
"""
        maxlen = max([len(p) for p in prim_names])
        for p in prim_names:
            pad = " " * (maxlen - len(p))
            if p == "void":
                sc = 0
                name = p
            elif p in value_types:
                sc = find_scalar_count(p)
                name = p
            else:
                sc = 1
                name = p
                if p in sampler_types and sampler_types[p].brcm:
                    name += "BRCM"
            print >>outf, "   primitiveTypes[%s%s].name                   = \"%s\";" % (prim_index(p),pad,name)
            print >>outf, "   primitiveTypes[%s%s].scalar_count           = %d;" % (prim_index(p), pad, sc)
            print >>outf

        maxlen = max([len(p) for p in prim_names])
        for p in prim_names:
            pad = " " * (maxlen - len(p))
            if p in value_types:
                sub = value_types[p].indexed_type
            else:
                sub = None
            if sub == None:
                sub = "NULL"
            else:
                sub = "&primitiveTypes[PRIM_%s]" % sub.upper()
            print >>outf, "   primitiveTypeSubscriptTypes[%s%s] = %s;" % (prim_index(p),pad,sub)
        print >>outf, "}"

from __future__ import print_function
#
# This giant python script is responsible for building glsl_stdlib.auto.{c,h} out of a directory of C and Python
# files. It does this by parsing out function prototypes and extracting function bodies. The GLSL is then
# pushed into strings and dissected into various struct data held statically in the compiler. C code
# is copied verbatim, but has to be parsed so that we can (a) declare function prototypes (b) connect
# C implementations automatically to their GLSL counterparts.
#

import sys, re, os
from generation_utils import *
from parsers import parse_source, parse_props, parse_table

def print_helpers(outf):
    print_banner(outf, ["Static helper functions used by the API"])
    print("""\
static uint64_t get_version_mask(int version) {
   switch(version) {
   case GLSL_SHADER_VERSION(1,0,1):  return GLSL_STDLIB_PROPERTY_VERSION1;
   case GLSL_SHADER_VERSION(3,0,1):  return GLSL_STDLIB_PROPERTY_VERSION3;
   case GLSL_SHADER_VERSION(3,10,1): return GLSL_STDLIB_PROPERTY_VERSION31;
   case GLSL_SHADER_VERSION(3,20,1): return GLSL_STDLIB_PROPERTY_VERSION32;
   default: unreachable();           return 0;
   }
}

static uint64_t get_flavour_mask(ShaderFlavour flavour) {
   switch(flavour) {
   case SHADER_VERTEX:          return GLSL_STDLIB_PROPERTY_VERTEX;
   case SHADER_TESS_CONTROL:    return GLSL_STDLIB_PROPERTY_TESS_C;
   case SHADER_TESS_EVALUATION: return GLSL_STDLIB_PROPERTY_TESS_E;
   case SHADER_GEOMETRY:        return GLSL_STDLIB_PROPERTY_GEOMETRY;
   case SHADER_FRAGMENT:        return GLSL_STDLIB_PROPERTY_FRAGMENT;
   case SHADER_COMPUTE:         return GLSL_STDLIB_PROPERTY_COMPUTE;
   default: unreachable();      return 0;
   }
}

static bool select_from_props(uint64_t stdlib_props, uint64_t input_props) {
    int v_mask = GLSL_STDLIB_PROPERTY_VERSIONS_ALL | GLSL_STDLIB_PROPERTY_EXTENSIONS_ALL;
    int s_mask = GLSL_STDLIB_PROPERTY_STAGES_ALL;

    return (stdlib_props & input_props & v_mask) != 0 && (stdlib_props & input_props & s_mask) != 0;
}

static void correct_overloads_for_mask(uint64_t mask) {
   for(int i=0; i<GLSL_STDLIB_FUNCTION_COUNT; i++) {
      if(select_from_props(glsl_stdlib_function_properties[i], mask)) {
         Symbol *overload = glsl_stdlib_functions[i].u.function_instance.next_overload;
         glsl_stdlib_functions[i].u.function_instance.next_overload = NULL;
         while(overload) {
            const size_t overload_index = glsl_stdlib_function_index(overload);
            if(select_from_props(glsl_stdlib_function_properties[overload_index], mask)) {
               glsl_stdlib_functions[i].u.function_instance.next_overload = overload;
               break;
            }
            overload = overload->u.function_instance.next_overload;
         }
      }
   }
}
""", file=outf)

def print_header_api(outf):
    print("""\
/* This contains the source code for all functions in the standard library */
extern const char *glsl_stdlib_function_bodies[GLSL_STDLIB_FUNCTION_COUNT];

/* All preprocessor directives in the GLSL are gathered here and become the first parsed data */
extern const char *glsl_stdlib_setup;

/* These are the properties set on the standard functions (e.g. fragment-only etc) */
extern const uint64_t glsl_stdlib_function_properties[GLSL_STDLIB_FUNCTION_COUNT];

/* Initialization functions. The masks indicate properties that will cause an entry to be
   added to the symbol table. */
void glsl_stdlib_init(void);
void glsl_stdlib_populate_symbol_table_with_functions(SymbolTable *symbol_table, uint64_t mask);
void glsl_stdlib_populate_symbol_table_with_variables(SymbolTable *symbol_table, uint64_t mask);
void glsl_stdlib_populate_symbol_table_with_types    (SymbolTable *symbol_table, uint64_t mask);
void glsl_stdlib_populate_scalar_value_map(Map *map);

/* Functions for creating/getting the builtin interface blocks */
void glsl_stdlib_populate_symbol_table_with_gl_in(SymbolTable *symbol_table, unsigned array_size);
void glsl_stdlib_populate_symbol_table_with_gl_out(SymbolTable *symbol_table, unsigned array_size);

/* Sort the stdlib symbols by storage qual into interfaces */
void glsl_stdlib_interfaces_update(ShaderInterfaces *interfaces, uint64_t property_mask);

/* Get the property mask for symbols based on shader's version and flavour */
uint64_t glsl_stdlib_get_property_mask(ShaderFlavour flavour, int version);

/* Query functions */
const Symbol *glsl_stdlib_get_function(glsl_stdlib_function_index_t index);
const Symbol *glsl_stdlib_get_variable(glsl_stdlib_variable_index_t index);
bool glsl_stdlib_is_stdlib_symbol(const Symbol *sym);
bool glsl_stdlib_is_stdlib_function(const Symbol *sym);
bool glsl_stdlib_is_stdlib_interface_block(const Symbol *sym);
glsl_stdlib_function_index_t glsl_stdlib_function_index(const Symbol *sym);
""", file=outf)

def print_api(outf):
    print_banner(outf, ["The external API used by clients of this file"])
    print("""\
void glsl_stdlib_init() {
   glsl_stdlib_variables = malloc_fast(sizeof(*glsl_stdlib_variables) * GLSL_STDLIB_VARIABLE_COUNT);
   glsl_stdlib_functions = malloc_fast(sizeof(*glsl_stdlib_functions) * GLSL_STDLIB_FUNCTION_COUNT);
   glsl_stdlib_gl_in     = NULL;
   glsl_stdlib_gl_out    = NULL;
}

void glsl_stdlib_populate_symbol_table_with_functions(SymbolTable *symbol_table, uint64_t property_mask) {
   glsl_stdlib_init_functions();
   correct_overloads_for_mask(property_mask);
   for(int i=0; i<GLSL_STDLIB_FUNCTION_COUNT; i++) {
      if(select_from_props(glsl_stdlib_function_properties[i], property_mask)) {
         glsl_symbol_table_insert(symbol_table, &glsl_stdlib_functions[i]);
      }
   }
}

void glsl_stdlib_populate_symbol_table_with_variables(SymbolTable *symbol_table, uint64_t property_mask) {
   glsl_stdlib_init_variables();
   for(int i=0; i<GLSL_STDLIB_VARIABLE_COUNT; i++) {
      if(select_from_props(glsl_stdlib_variable_properties[i], property_mask)) {
         glsl_symbol_table_insert(symbol_table, &glsl_stdlib_variables[i]);
      }
   }
}

void glsl_stdlib_populate_symbol_table_with_types(SymbolTable *symbol_table, uint64_t property_mask) {
   glsl_stdlib_init_types();
   for (int i=0; i<GLSL_STDLIB_TYPE_COUNT; i++) {
      if (select_from_props(glsl_stdlib_type_properties[i], property_mask)) {
         if (glsl_stdlib_types[i].flavour != SYMBOL_STRUCT_TYPE) continue;
         glsl_symbol_table_insert(symbol_table, &glsl_stdlib_type_symbols[i]);
      }
   }
}

void glsl_stdlib_populate_scalar_value_map(Map *map) {
   glsl_stdlib_fill_scalar_value_map(map);
}

void glsl_stdlib_interfaces_update(ShaderInterfaces *interfaces, uint64_t property_mask) {
   for(int i=0; i<GLSL_STDLIB_VARIABLE_COUNT; i++) {
      if(select_from_props(glsl_stdlib_variable_properties[i], property_mask)) {
         glsl_shader_interfaces_update(interfaces, &glsl_stdlib_variables[i]);
      }
   }

   if (glsl_stdlib_gl_in)
      glsl_shader_interfaces_update(interfaces, glsl_stdlib_gl_in);
   if (glsl_stdlib_gl_out)
      glsl_shader_interfaces_update(interfaces, glsl_stdlib_gl_out);
}

static Symbol *in_out_block(SymbolTable *symbol_table, const char *name, StorageQualifier sq, unsigned array_size) {
   Qualifiers q = { .invariant = false,
                    .lq = NULL,
                    .sq = sq,
                    .iq = INTERP_SMOOTH,
                    .aq = AUXILIARY_NONE,
                    .pq = PREC_HIGHP,
                    .mq = MEMORY_NONE };

   SymbolType *resultType   = malloc_fast(sizeof(SymbolType));
   resultType->flavour      = SYMBOL_BLOCK_TYPE;
   resultType->name         = glsl_intern("gl_PerVertex", false);
   resultType->scalar_count = 5;

   StructMember *memb = malloc_fast(sizeof(StructMember) * 2);
   memb[0].name   = glsl_intern("gl_Position", false);
   memb[0].type   = &primitiveTypes[PRIM_VEC4];
   memb[0].layout = NULL;
   memb[0].prec   = PREC_HIGHP;
   memb[0].memq   = MEMORY_NONE;
   memb[0].interp = INTERP_SMOOTH;
   memb[0].auxq   = AUXILIARY_NONE;

   memb[1].name   = glsl_intern("gl_PointSize", false);
   memb[1].type   = &primitiveTypes[PRIM_FLOAT];
   memb[1].layout = NULL;
   memb[1].prec   = PREC_HIGHP;
   memb[1].memq   = MEMORY_NONE;
   memb[1].interp = INTERP_SMOOTH;
   memb[1].auxq   = AUXILIARY_NONE;

   resultType->u.struct_type.member_count = 2;
   resultType->u.struct_type.member       = memb;

   resultType->u.block_type.lq = NULL;
   resultType->u.block_type.layout = malloc_fast(sizeof(MemLayout));
   glsl_mem_calculate_block_layout(resultType->u.block_type.layout, resultType, /*for_tmu*/false);
   resultType->u.block_type.has_named_instance = false;

   SymbolType *arr_type = glsl_symbol_type_construct_array(resultType, array_size);

   Symbol *symbol = malloc_fast(sizeof(Symbol));
   SymbolType *ref_type = &primitiveTypes[PRIM_UINT];
   ref_type = glsl_symbol_type_construct_array(ref_type, array_size);
   glsl_symbol_construct_interface_block(symbol, glsl_intern("gl_PerVertex", false), ref_type, arr_type, &q);
   glsl_symbol_table_insert(symbol_table, symbol);

   Symbol *s = malloc_fast(sizeof(Symbol));
   glsl_symbol_construct_var_instance(s, name, arr_type, &q, NULL, symbol);
   glsl_symbol_table_insert(symbol_table, s);

   return symbol;
}

void glsl_stdlib_populate_symbol_table_with_gl_in(SymbolTable *symbol_table, unsigned array_size) {
   glsl_stdlib_gl_in = in_out_block(symbol_table, glsl_intern("gl_in", false), STORAGE_IN, array_size);
}

void glsl_stdlib_populate_symbol_table_with_gl_out(SymbolTable *symbol_table, unsigned array_size) {
   glsl_stdlib_gl_out = in_out_block(symbol_table, glsl_intern("gl_out", false), STORAGE_OUT, array_size);
}

uint64_t glsl_stdlib_get_property_mask(ShaderFlavour flavour, int version) {
   return get_version_mask(version) | get_flavour_mask(flavour);
}

const Symbol *glsl_stdlib_get_function(glsl_stdlib_function_index_t index) {
   return &glsl_stdlib_functions[index];
}
const Symbol *glsl_stdlib_get_variable(glsl_stdlib_variable_index_t index) {
   return &glsl_stdlib_variables[index];
}
bool glsl_stdlib_is_stdlib_function(const Symbol *sym) {
   return (sym >= glsl_stdlib_functions && sym < glsl_stdlib_functions + GLSL_STDLIB_FUNCTION_COUNT);
}
bool glsl_stdlib_is_stdlib_interface_block(const Symbol *sym) {
   return (sym && (sym == glsl_stdlib_gl_in || sym == glsl_stdlib_gl_out));
}
bool glsl_stdlib_is_stdlib_symbol(const Symbol *sym) {
   if(sym >= glsl_stdlib_variables && sym < glsl_stdlib_variables + GLSL_STDLIB_VARIABLE_COUNT) {
      return true;
   }
   return glsl_stdlib_is_stdlib_function(sym) ||
          glsl_stdlib_is_stdlib_interface_block(sym);
}

glsl_stdlib_function_index_t glsl_stdlib_function_index(const Symbol *sym) {
   return (glsl_stdlib_function_index_t)(sym - glsl_stdlib_functions);
}
""", file=outf)

def param_string(p):
    name, types = p
    return "%s %s" % (" ".join(types),name)

def print_function(outf,fn):
    fn_name, fn_return, fn_params, fn_body = fn

    outf.write(" ".join(fn_return) + " " + fn_name + "(" + ", ".join([param_string(p) for p in fn_params]) + ") {" + fn_body + "}\n")

def output_prototype(outf,fn):
    fn_name, fn_return, fn_params, fn_body = fn

    outf.write(" ".join(fn_return) + " " + fn_name + "(" + ", ".join([param_string(p) for p in fn_params]) + ");\n")

def manglename(fn):
    fn_name, fn_return, fn_params, fn_body = fn

    return "__".join(["glsl_stdlib_fn"] + [fn_name,"_".join(fn_return)]+["_".join(munge_array_type(ptype)) for (pname,ptype) in fn_params])

def fn_index(fn):
    return manglename(fn).upper()

def output_sources(outf, functions):
    outf.write("const char *glsl_stdlib_function_bodies[GLSL_STDLIB_FUNCTION_COUNT] = {\n")
    for fn in functions:
        fn_name, fn_return, fn_params, fn_body = fn

        header = " ".join(fn_return) + " " + fn_name + "(" + ", ".join([param_string(p) for p in fn_params]) + ") {"

        lines = fn_body.strip().rstrip().split("\n")
        lines = [ header ] + lines

        body = ""
        for line in lines:
            if len(line.strip()) == 0:
                body += "\n"
            else:
                body += "      \"" + line.replace("\"", "\\\"") + "\\n\"" + "\n"
        body += "      \"}\\n\",\n\n"

        print("      /* %s */" % manglename(fn), file=outf)
        outf.write(body)
    outf.write("};\n")

stage_props = {"VERTEX", "TESS_E", "TESS_C", "GEOMETRY", "FRAGMENT", "COMPUTE"}
version_props = {"VERSION1", "VERSION3", "VERSION31", "VERSION32"}
extension_props = { "GL_OES_EGL_IMAGE_EXTERNAL",
                    "GL_BRCM_TEXTURE_1D",
                    "GL_BRCM_SAMPLER_FETCH",
                    "GL_BRCM_TEXTURE_GATHER_LOD",
                    "GL_BRCM_SHADER_FRAMEBUFFER_FETCH_DEPTH_STENCIL",
                    "GL_EXT_SHADER_TEXTURE_LOD",
                    "GL_OES_STANDARD_DERIVATIVES",
                    "GL_EXT_SHADER_INTEGER_MIX",
                    "GL_OES_TEXTURE_CUBE_MAP_ARRAY",
                    "GL_OES_TEXTURE_BUFFER",
                    "GL_OES_TEXTURE_STORAGE_MULTISAMPLE_2D_ARRAY",
                    "GL_OES_SHADER_IMAGE_ATOMIC",
                    "GL_OES_GPU_SHADER5",
                    "GL_OES_SAMPLE_VARIABLES",
                    "GL_OES_SHADER_MULTISAMPLE_INTERPOLATION",
                    "GL_OES_TESSELLATION_SHADER",
                    "GL_OES_GEOMETRY_SHADER",
                    "GL_OES_PRIMITIVE_BOUNDING_BOX",
                    "GL_EXT_PRIMITIVE_BOUNDING_BOX"}

def get_prop_enum(index, props):
    s_props = set()
    v_props = set()
    e_props = set()
    m_props = set()

    if index in props and len(props[index]) != 0:
        s_props = props[index] & stage_props
        v_props = props[index] & version_props
        e_props = props[index] & extension_props
        m_props = props[index] - (stage_props | version_props | extension_props)

    if len(v_props) == 0 and len(e_props) == 0:
        v_props = version_props
    if len(s_props) == 0:
        s_props = stage_props
    return (" | ".join(["GLSL_STDLIB_PROPERTY_%s" % p for p in (s_props | v_props | e_props | m_props)]), index)

def output_properties(outf, functions, variables, structs, arrays, props):
    prop_enum = (lambda x, lookup: get_prop_enum(lookup(x), props))

    print_array(outf, "const uint64_t", "glsl_stdlib_function_properties", "GLSL_STDLIB_FUNCTION_COUNT",
                [prop_enum(fn,fn_index) for fn in functions])

    print_array(outf, "const uint64_t", "glsl_stdlib_variable_properties", "GLSL_STDLIB_VARIABLE_COUNT",
                [prop_enum(var,var_index) for var in variables])

    print_array(outf, "const uint64_t", "glsl_stdlib_type_properties", "GLSL_STDLIB_TYPE_COUNT",
                [prop_enum((s,), compoundtypeindex) for s,b in structs] +
                [prop_enum( a,   compoundtypeindex) for a   in arrays ])

def output_properties_define(outf, props):
    misc_props = set()
    for f,p in props.items():
        misc_props.update(p)
    misc_props = misc_props - (stage_props | version_props | extension_props)
    misc_list = list(misc_props)
    extension_list = list(extension_props)
    misc_list.sort()
    extension_list.sort()
    props_list = list(stage_props) + list(version_props) + extension_list + misc_list

    print_defines(outf,[("GLSL_STDLIB_PROPERTY_%s" % p, "(1ull<<%d)" % i) for i,p in enumerate(props_list)])
    print_defines(outf, [("GLSL_STDLIB_PROPERTY_STAGES_ALL", "( " + " | ".join(["GLSL_STDLIB_PROPERTY_%s" % p for p in stage_props]) + " )")])
    print_defines(outf, [("GLSL_STDLIB_PROPERTY_VERSIONS_ALL", "( " + " | ".join(["GLSL_STDLIB_PROPERTY_%s" % p for p in version_props]) + " )")])
    print_defines(outf, [("GLSL_STDLIB_PROPERTY_EXTENSIONS_ALL", "( " + " | ".join(["GLSL_STDLIB_PROPERTY_%s" % p for p in extension_props]) + " )")])

def munge_array_type(t):
    if t[-1].count("[") > 0:
        new_t = t[:-2]
        new_t.append("array")
        new_t.append("of")
        new_t.append(t[-1][1:-1])
        new_t.append("of")
        new_t.append(t[-2])
        return new_t
    else:
        return t

def make_sig(fn):
    fn_name, fn_return, fn_params, fn_body = fn
    return tuple([" ".join(fn_return)] + [" ".join(munge_array_type(t)) for (p,t) in fn_params])

def find_fn_names(functions):
    names = set()
    for fn in functions:
        fn_name, fn_return, fn_params, fn_body = fn
        names.add(fn_name)
    return list(names)

def find_var_names(variables):
    names = set()
    for var in variables:
        var_name, var_type, rhs = var
        names.add(var_name)
    return list(names)

def find_types(functions):
    types = set()
    for fn in functions:
        fn_name, fn_return, fn_params, fn_body = fn
        tyname = fn_return[-1]
        types.add(tyname)
        for p,t in fn_params:
            tyname = t[-1]
            types.add(tyname)
    return list(types)

def find_sigs(functions):
    sigs = set()
    for fn in functions:
        sig = make_sig(fn)
        sigs.add(sig)
    return list(sigs)

def find_params(functions):
    params = set()
    for fn in functions:
        fn_name, fn_return, fn_params, fn_body = fn
        for p,t in fn_params:
            params.add(tuple(t))
        params.add(tuple(fn_return))
    return list(params)

def sigenumname(sig):
    return "__".join(["glsl_stdlib_sig"] + [s.replace(" ","_") for s in sig]).upper()

def paramenumname(param):
    return "glsl_stdlib_params[%s]" % param_index(param)

def sigstructname(sig):
    return "glsl_stdlib_function_signatures[" + sigenumname(sig) + "]"

def sigstring(sig):
    return "(" + ", ".join([s for s in sig[1:]]) + ")"

def param_index(param):
    return "__".join(["glsl_stdlib_param"] + [p.replace(" ","_") for p in munge_array_type(list(param))]).upper()

def param_qual(param):
    quals = ["in", "out", "inout"]
    for q in quals:
        if q in param:
            return "PARAM_QUAL_%s" % q.upper()
    return "PARAM_QUAL_IN"

def mem_quals(param):
    quals = ["coherent", "volatile", "restrict", "readonly", "writeonly"]
    qstr = "MEMORY_NONE"
    for q in param:
        if q in quals:
            qstr = "%s | MEMORY_%s" % (qstr, q.upper())
    return qstr

def find_type(t, primtypemap):
    return typeindex((t.split()[-1],), primtypemap)[0]

def c_name(fn):
    fn_name, fn_return, fn_params, fn_body = fn
    c_return = list(fn_return)
    if "const" not in c_return:
        c_return = ["const"] + c_return
    c_paramtypes = []
    for p,t in fn_params:
        cpt = list(t)
        if "const" not in cpt:
            cpt = ["const"] + cpt
        c_paramtypes.append(cpt)
    return "__".join([fn_name,"_".join(c_return)] + [ "_".join(c) for c in  c_paramtypes])

def safe_name(name):
    return name.replace("$", "_")

def output_interns(outf,names):
    s_names = [safe_name(n) for n in names]
    max_name_len = max([len(n) for n in names])
    for i,name in enumerate(names):
        pad_string = " "*(max_name_len - len(name))
        outf.write("   const char *intern_%s%s = glsl_intern(\"%s\",%s false);\n" % (
                s_names[i], pad_string, name, pad_string))
    outf.write("\n")

def output_sig(outf,sig, primtypemap):
    outf.write("   " + sigstructname(sig) + ".name                        = \"" + sigstring(sig) + "\";\n")
    outf.write("   " + sigstructname(sig) + ".u.function_type.param_count = %d;\n" % (len(sig)-1))
    outf.write("   " + sigstructname(sig) + ".u.function_type.params      = (Symbol **)malloc_fast(sizeof(Symbol*) * %d);\n" % (len(sig)-1))
    outf.write("   " + sigstructname(sig) + ".u.function_type.return_type = &primitiveTypes[%s];\n" % (
            find_type(sig[0],primtypemap)))
    for p_idx in range(len(sig)-1):
        outf.write("   " + sigstructname(sig) + ".u.function_type.params[%d]   = &glsl_stdlib_params[%s];\n" % (
                p_idx,param_index(tuple(sig[p_idx+1].split()))))
    outf.write("\n")

def output_param(outf, param, primtypemap):
    print("    glsl_symbol_construct_param_instance(&glsl_stdlib_params[%s], \"\", &%s, STORAGE_NONE, %s, %s);" % (
         param_index(param), typeenumname(make_type(None, param),primtypemap), param_qual(param), mem_quals(param)), file=outf)

def output_fn_struct(outf,fn, overload, c_func):
    fn_name, fn_return, fn_params, fn_body = fn

    outf.write("   glsl_symbol_construct_function_instance(\n")
    outf.write("      &glsl_stdlib_functions[" + fn_index(fn) + "],\n")
    outf.write("      intern_" + fn_name + ",\n")
    outf.write("      &" + sigstructname(make_sig(fn)) + ",\n")
    if c_func == None:
        outf.write("      NULL,  /* Folding function would be %s */\n" % c_name(fn))
    else:
        outf.write("      (FoldingFunction)"+c_func+",\n")
    if overload == None:
        outf.write("      NULL,  /* No overload */\n")
    else:
        outf.write("      &glsl_stdlib_functions[" + fn_index(overload) + "],\n")
    outf.write("      true); /* Has a prototype */\n")

def qual_from_set(var_type, recognised_quals):
    quals = set()
    for t in var_type:
        if t in recognised_quals:
            quals.add(t)

    if len(quals) == 0:
        return "None"
    if len(quals) != 1:
        sys.stderr.write("Invalid qualifiers %s for variable %s\n" % (" ".join(quals), var_name))
        sys.exit(1)

    return quals.pop()

def memory_qualifiers(vtype):
    qs = set([ "readonly", "writeonly", "coherent", "volatile", "restrict" ])
    return [a for a in vtype if (a in qs)];

def storage_qualifier(vtype):       # TODO: Allowing 'inout' here is a hack from the GLSL grammar
    return qual_from_set(vtype, set([ "const", "uniform", "in", "out", "inout" ]))

def interp_qualifier(vtype):
    return qual_from_set(vtype, set([ "noperspective", "flat" ]))

def aux_qualifier(vtype):
    return qual_from_set(vtype, set([ "centroid", "patch", "sample" ]))

def type_precision(vtype):
    return qual_from_set(vtype, set([ "mediump", "lowp", "highp"]))

def compoundtypeindex(typet):
    if len(typet) == 1:
        return "%s" % ("glsl_stdlib_type__%s" % typet).upper()
    else:
        return "%s" % (
            "__".join(["glsl_stdlib_type",
                       "array_" +
                       "_".join(["of_%s" % t for t in (list(typet[1:]) + [typet[0]])])])).upper()

def typeindex(typet, primtypemap):
    if len(typet) == 1 and typet[0] in primtypemap:
        return (primtypemap[typet[0]].prim_name, True)
    else:
        return (compoundtypeindex(typet),False)

def find_component_type(typet,i,primtypemap,structmap):
    if len(typet) == 1:
        if typet[0] in primtypemap:
            assert(primtypemap[typet[0]].base_type != None)
            return (primtypemap[typet[0]].base_type,)
        else:
            return make_type_from_var(structmap[typet[0]][i])
    else:
        return tuple(typet[1:])

def typeenumname(typet, primtypemap):
    typeidx,isprim = typeindex(typet, primtypemap)
    if isprim:
        return "primitiveTypes[%s]" % typeidx
    return "glsl_stdlib_types[%s]" % typeidx

def typesymbolenumname(typet):
    return "glsl_stdlib_type_symbols[%s]" % compoundtypeindex(typet)

def make_type(var_name, var_type):
    storage_qual = storage_qualifier(var_type)
    memory_quals = memory_qualifiers(var_type)
    interp_qual = interp_qualifier(var_type)
    aux_qual = aux_qualifier(var_type)
    precision = type_precision(var_type)
    residual = [a for a in var_type if (a != interp_qual) and (a != aux_qual) and (a != storage_qual) and (a not in memory_quals) and (a != precision)]
    tname = None
    array_levels = []
    for r in residual:
        if r[0] == "[":
            array_levels.append(r[1:-1])
        else:
            if tname != None:
                sys.stderr.write("Bad typenames '%s' '%s' for variable %s\n" % (r, tname, var_name))
                sys.exit(1)
            tname = r
    if tname == None:
        sys.stderr.write("No typename for variable %s\n" % (var_name))
        sys.exit(1)
    return tuple([tname] + array_levels)

def make_type_from_var(var):
    var_name, var_type, rhs = var
    return make_type(var_name, var_type)

def var_index(var):
    var_name, var_type, rhs = var
    var_name = safe_name(var_name)
    i_qual = interp_qualifier(var_type)
    a_qual = aux_qualifier(var_type)
    s_qual = storage_qualifier(var_type)
    precision = type_precision(var_type)
    tname = make_type_from_var(var)
    q = []
    if (i_qual != "None"):
        q = q + [i_qual]
    if (a_qual != "None"):
        q = q + [a_qual]
    q = q + [s_qual]
    q_str = ["__".join(q)]
    if precision == "None":
        precision = []
    else:
        precision = [precision]
    if len(tname) == 1:
        return ("__".join(["glsl_stdlib_var"] + q_str + precision + [tname[0], var_name])).upper()
    else:
        return ("__".join(["glsl_stdlib_var"] + q_str + precision +
                          ["_".join(["array"] + ["OF_%s" % t for t in list(tname[1:])+[tname[0]]])] +
                          [var_name])).upper()

def find_arrays(variables, functions):
    arrays = set()
    for var in variables:
        t = make_type_from_var(var)
        if len(t) > 1:
            for i in reversed(list(range(len(t)-1))):
                partial = tuple([t[0]] + list(t[i+1:]))
                arrays.add(partial)
    for fn in functions:
        fn_name, fn_return, fn_params, fn_body = fn
        for p in fn_params:
            pname, ptype = p
            t = make_type(pname, ptype)
            if len(t) > 1:
                for i in reversed(list(range(len(t)-1))):
                    partial = tuple([t[0]] + list(t[i+1:]))
                    arrays.add(partial)
    return list(arrays)

def find_constants(variables):
    constants = {}
    for v,t,r in variables:
        if "const" in t and r != None:
            # constant detected
            r = " ".join(r)
            while " ".join(r) in constants:
                r = constants[r]
                r = " ".join(r)
            constants[v] = r
    return constants

def output_array(outf, array_in, constant_map, primtypemap):
    arrayname    = array_in[0]
    arrayindices = array_in[1:]
    array        = []
    for x in array_in:
        if x in constant_map:
            array.append(constant_map[x])
        else:
            array.append(x)
    array = tuple(array)
    arrayindex   = array[1]
    ename        = typeenumname(array_in, primtypemap)

    if len(arrayindices) > 1:
        elename = (arrayname,) +  arrayindices[1:]
    else:
        elename = (arrayname,)
    outf.write("   /* %s */\n" % (arrayname+"".join("[%s]" % idx for idx in arrayindices)))
    outf.write("   {\n")
    outf.write("      %s.flavour =\n         SYMBOL_ARRAY_TYPE;\n"  % ename)
    outf.write("      %s.name =\n          \"%s%s\";\n" % (
            ename, arrayname, "".join("[%s]" % idx for idx in arrayindices)))
    outf.write("      %s.scalar_count  =\n         %s.scalar_count  * %s;\n" % (
            ename, typeenumname(elename, primtypemap), arrayindex))
    outf.write("      %s.u.array_type.member_count =\n         %s;\n" % (
            ename, arrayindex))
    outf.write("      %s.u.array_type.member_type =\n         &%s;\n" % (
            ename, typeenumname(elename, primtypemap)))
    outf.write("   }\n")
    outf.write("\n")

def output_struct_type(outf, struct,primtypemap):
    struct_name, struct_body = struct
    ename         = typeenumname((struct_name,), primtypemap)
    var_types = []
    for v in struct_body:
        var_types.append(make_type_from_var(v))

    outf.write("   /* %s */\n" % struct_name)
    outf.write("   {\n")
    outf.write("      %s.flavour =\n         SYMBOL_STRUCT_TYPE;\n" % ename)
    outf.write("      %s.name =\n         glsl_intern(\"%s\", false);\n" % (ename, struct_name))
    outf.write("      %s.scalar_count =\n         %s;\n" % (
            ename, " +\n         ".join(["%s.scalar_count" % typeenumname(vt,primtypemap) for vt in var_types])))
    outf.write("      %s.u.struct_type.member_count =\n         %d;\n" % (
            ename, len(struct_body)))
    outf.write("      %s.u.struct_type.member =\n               malloc_fast(sizeof(*%s.u.struct_type.member) * %d);\n" % (
            ename, ename, len(struct_body)))
    for i,v in enumerate(struct_body):
        var_name, var_type, rhs = v
        outf.write("      %s.u.struct_type.member[%d].name   = \"%s\";\n" % (
                ename, i, var_name))
        outf.write("      %s.u.struct_type.member[%d].type   = &%s;\n" % (
                ename, i, typeenumname(var_types[i],primtypemap)))
        outf.write("      %s.u.struct_type.member[%d].prec   = PREC_%s;\n" % (
                ename, i, type_precision(var_type).upper()))
        outf.write("      %s.u.struct_type.member[%d].layout = NULL;\n" % (
                ename, i))
        outf.write("      %s.u.struct_type.member[%d].memq   = MEMORY_NONE;\n" % (
                ename, i))
    outf.write("      glsl_symbol_construct_type(&%s,\n                                 &%s);\n" % (
            typesymbolenumname((struct_name,)), ename))
    outf.write("   }\n")

def parse_rhs(rhs):
    if rhs == None:
        return None, []

    rhs = " ".join(rhs)
    match = re.match('([^(]+)\((.*)\)', rhs)
    if match:
        kind = match.group(1)
        rhs  = match.group(2)
    else:
        if rhs.count("(") > 0 or rhs.count(","):
            print("Bad rhs variable %s: %s" % (var_name, rhs), file=sys.stderr)
            sys.exit(1)
        kind = "int"
    return kind,rhs.split(",")

def output_var_struct(outf, var, primtypemap, structmap, struct=None):
    var_name, var_type, rhs = var

    rhs_kind, rhs_values = parse_rhs(rhs)
    is_const = "const" in var_type

    if "invariant" in var_type:
        invariant_str = "true"
    else:
        invariant_str = "false"

    interp = interp_qualifier(var_type)
    if interp == "None":
       interp = "smooth"

    assert((is_const and len(rhs_values) > 0) or not is_const)

    outf.write("   /* %s */\n" % var_name)
    outf.write("   {\n")
    outf.write("      Qualifiers q = { .invariant = " + invariant_str + ",\n")
    outf.write("                       .lq        = NULL,\n")
    outf.write("                       .sq        = STORAGE_"   + storage_qualifier(var_type).upper() + ",\n")
    outf.write("                       .iq        = INTERP_"    + interp.upper()                      + ",\n")
    outf.write("                       .aq        = AUXILIARY_" + aux_qualifier(var_type).upper()     + ",\n")
    outf.write("                       .pq        = PREC_"      + type_precision(var_type).upper()    + ",\n")
    outf.write("                       .mq        = "           + mem_quals(memory_qualifiers(var_type)) + " };\n")

    if not is_const:
        outf.write("      const_value *compile_time_value = NULL;\n")
    else:
        outf.write("      const_value *compile_time_value = malloc_fast(%d * sizeof(const_value));\n" % (len(rhs_values)))
        for i, v in enumerate(rhs_values):
            v = v.strip().rstrip()
            outf.write("      compile_time_value[%d] = %s;\n" % (i, v))

    outf.write("      glsl_symbol_construct_var_instance(\n")
    outf.write("         &glsl_stdlib_variables[" +  var_index(var)  + "],\n")
    outf.write("         intern_" + safe_name(var_name) + ",\n")
    outf.write("         &" + typeenumname(make_type_from_var(var),primtypemap) + ",\n")
    outf.write("         &q,\n")
    outf.write("         compile_time_value,\n")

    if struct == None:
        outf.write("         NULL); /* No containing block */\n")
    else:
        outf.write("         %s);\n" % make_struct_name(struct))
    outf.write("   }\n")

def output_var_scalar_values(outf, var, map_name):
    var_name, var_type, rhs = var

    rhs_kind, rhs_values = parse_rhs(rhs)
    is_const = "const" in var_type

    if len(rhs_values) > 0 and not is_const:
        outf.write("\n")
        outf.write("   /* " + var_name + " */\n")
        outf.write("   scalar_values = malloc_fast(sizeof(Dataflow*) * %d);\n" % len(rhs_values))
        for i,v in enumerate(rhs_values):
            v = v.strip().rstrip()
            comp_type = find_component_type(make_type_from_var(var),i,primtypemap, structmap)
            outf.write("   scalar_values[%d] = glsl_dataflow_construct_nullary_op(%s);\n" % (i, v))
        outf.write("   glsl_map_put(%s, &glsl_stdlib_variables[%s], scalar_values);\n" % (map_name, var_index(var)))

def output_functions_enum(outf, functions):
    print_enum(outf, "glsl_stdlib_function_index_t",
               [fn_index(fn) for fn in functions] +
               ["GLSL_STDLIB_FUNCTION_COUNT"])

def output_sigs_enum(outf, sigs):
    print_enum(outf, "glsl_stdlib_function_signature_index_t",
               [ sigenumname(sig) for sig in sigs ] +
               [ "GLSL_STDLIB_FUNCTION_SIG_COUNT"])

def output_params_enum(ouf, params):
    print_enum(outf, "glsl_stdlib_param_index_t",
               [ param_index(param) for param in params ] +
               [ "GLSL_STDLIB_PARAM_COUNT" ])

def output_variables_enum(outf, variables):
    print_enum(outf, "glsl_stdlib_variable_index_t",
               [ var_index(var) for var in variables ] +
               [ "GLSL_STDLIB_VARIABLE_COUNT"])

def output_types_enum(outf, structs, arrays, constant_map):
    print_enum(outf, "glsl_stdlib_type_index_t",
               [compoundtypeindex((s,)) for s,b in structs] +
               [compoundtypeindex(a)    for a   in arrays ] +
               [ "GLSL_STDLIB_TYPE_COUNT"])

def output_typedefs(outf, primtypemap):
    for p in list(primtypemap.values()):
        print("typedef const_value const_%s%s%s;" % (
            p.name, "[%d]" % p.dim1 if p.dim1 != None else "", "[%d]" % p.dim2 if p.dim2 != None else ""), file=outf)

if __name__ == "__main__":
    glsl_functions    = []
    glsl_variables    = []
    glsl_structs      = []
    glsl_directives   = []
    c_functions       = []
    c_function_bodies = []
    properties        = {}
    primtypemap       = {}
    structmap         = {}
    if len(sys.argv) < 2:
        sys.stderr.write("Usage: %s <options> <input files>\n" % sys.argv[0])
        sys.exit(1)

    output_dir, include_dirs, input_files = parse_opts()

    for forig in input_files:
        f = find_file(forig, include_dirs)
        if f == None:
            sys.stderr.write("Could not find file %s\n" % forig)
            sys.exit(1)
        with open(f, "r") as fp:
            lines = fp.readlines()
        if re.search('\.glsl$', f):
            functions,variables,structs,directives = parse_source(lines)
            glsl_functions.extend(functions)
            glsl_variables.extend(variables)
            glsl_structs.extend(structs)
            glsl_directives.extend(directives)
        elif re.search('\.props$', f):
            parse_props(lines, properties, fn_index, var_index, compoundtypeindex)
        elif re.search('\.table$', f):
            PrimType,primtypemap = parse_table("PrimType", lines, "name")
        else:
            # assume C code
            functions, variables, structs, directives = parse_source(lines)
            c_functions.extend(functions)
            c_function_bodies.extend(lines)

    # Magical builtin
    discard = ("$$discard", ["out", "bool"], None)
    glsl_variables.append(discard)
    properties[var_index(discard)] = { "FRAGMENT" }

    sigs      = find_sigs     (glsl_functions)
    types     = find_types    (glsl_functions)
    fn_names  = find_fn_names (glsl_functions)
    params    = find_params   (glsl_functions)
    var_names = find_var_names(glsl_variables)
    arrays    = find_arrays   (glsl_variables, glsl_functions)
    constants = find_constants(glsl_variables)
    structmap = dict(glsl_structs)

    glsl_functions.sort(key=fn_index)
    types.sort()
    sigs.sort()
    fn_names.sort()
    var_names.sort()
    arrays.sort()
    params.sort()

    ## PRODUCING THE HEADER FILE
    with open(os.path.join(output_dir,"glsl_stdlib.auto.h"), "w") as outf:
        print_inclusion_guard_start(outf, "glsl_stdlib.auto.h")
        print_disclaimer(outf)
        print_includes(outf, ["glsl_symbols.h", "glsl_symbol_table.h", "glsl_shader_interfaces.h"])
        output_functions_enum(outf, glsl_functions)
        output_variables_enum(outf, glsl_variables)
        output_properties_define(outf, properties)
        print_header_api(outf)
        print_inclusion_guard_end(outf, "glsl_stdlib.auto.h")

    ## PRODUCING THE IMPLEMENTATION FILE
    with open(os.path.join(output_dir,"glsl_stdlib.auto.c"), "w") as outf:
        print_disclaimer(outf)
        print_includes(outf, ["stdlib.h", "stdio.h"], system=True)
        print_includes(outf, ["glsl_ast.h",             "glsl_compiler.h",    "glsl_common.h",
                              "glsl_const_operators.h", "glsl_fastmem.h",     "glsl_globals.h",
                              "glsl_intern.h",          "glsl_symbol_table.h","glsl_stdlib.auto.h",
                              "glsl_primitive_types.auto.h", "../glxx/glxx_int_config.h",
                              "GLES3/gl32.h", "GLES3/gl3ext_brcm.h", "libs/util/gfx_util/gfx_util.h"])
        output_sigs_enum     (outf, sigs)
        output_types_enum    (outf, glsl_structs, arrays, constants)
        output_params_enum   (outf, params)
        output_typedefs      (outf, primtypemap)

        # global variables
        print("""
Symbol     *glsl_stdlib_functions;
Symbol     *glsl_stdlib_variables;
static SymbolType glsl_stdlib_function_signatures[GLSL_STDLIB_FUNCTION_SIG_COUNT];
static SymbolType glsl_stdlib_types              [GLSL_STDLIB_TYPE_COUNT];
static Symbol     glsl_stdlib_type_symbols       [GLSL_STDLIB_TYPE_COUNT];
static Symbol     glsl_stdlib_params             [GLSL_STDLIB_PARAM_COUNT];

const char *glsl_stdlib_setup = """, end=' ', file=outf)

        if len(glsl_directives) == 0:
            outf.write("NULL;\n");
        else:
            outf.write("\n" + "\n".join(["   \""+ d.replace("\"", "\\\"").rstrip() + "\\n\"" for d in glsl_directives]) + ";\n")
        outf.write("\n")
        output_sources   (outf, glsl_functions)
        outf.write("\n")

        # creating property flags
        print_banner(outf, [ "Property definitions for the functions in the standard library"])
        output_properties(outf, glsl_functions, glsl_variables, glsl_structs, arrays, properties)

        # writing prototypes for the C functions
        if(len(c_functions) > 0):
            print_banner(outf, ["Static prototypes for C functions, in case they refer to one another"])
            for fn in c_functions:
                output_prototype(outf, fn)
            outf.write("\n")

        # stuffing the c functions back in at the top
        if len(c_function_bodies) > 0:
            print_banner(outf, ["Constant folding implementations of library functions"])
            for line in c_function_bodies:
                outf.write(line)
            outf.write("\n")

        # stuffing the signatures in
        print_banner(outf, ["Initializing static types for library elements (signatures, structs and arrays)"])
        print("""\
static void glsl_stdlib_init_types(void) {
   for (int i = 0; i < GLSL_STDLIB_FUNCTION_SIG_COUNT; i++) {
      glsl_stdlib_function_signatures[i].flavour       = SYMBOL_FUNCTION_TYPE;
      glsl_stdlib_function_signatures[i].scalar_count  = 0;
   }\
""", file=outf)
        outf.write("\n")
        for sig in sigs:
            output_sig(outf,sig,primtypemap)
        for struct in glsl_structs:
            output_struct_type(outf, struct, primtypemap)
        for array in arrays:
            output_array(outf, array, constants, primtypemap)
        for param in params:
            output_param(outf,param,primtypemap)
        outf.write("}\n")
        outf.write("\n")

        print_helpers(outf)

        # stuffing the struct construction in
        print_banner(outf, ["Initializing symbols for functions in the library"])
        print("static void glsl_stdlib_init_functions(void) {", file=outf)
        c_names = set(find_fn_names(c_functions))

        output_interns(outf,fn_names)

        overloads = {}
        for fn in glsl_functions:
            fn_name, fn_return, fn_params, fn_body = fn

            overload = None
            if fn_name in overloads:
                overload = overloads[fn_name]
            overloads[fn_name] = fn

            c_function = None
            if c_name(fn) in c_names:
                c_function = c_name(fn)

            output_fn_struct(outf,fn, overload, c_function)
            outf.write("\n")
        outf.write("}\n")
        outf.write("\n")

        # structs for variables
        print_banner(outf, ["Initializing symbols for variables in the library"])
        print("static void glsl_stdlib_init_variables() {", file=outf)

        output_interns(outf,var_names)

        for var in glsl_variables:
            output_var_struct(outf, var, primtypemap, structmap)
            outf.write("\n")
        outf.write("}\n")
        outf.write("\n")

        print("static void glsl_stdlib_fill_scalar_value_map(Map *scalar_value_map) {", file=outf)
        outf.write("   Dataflow** scalar_values;\n")
        for var in glsl_variables:
            output_var_scalar_values(outf, var, "scalar_value_map")
        outf.write("}\n")

        print("\nSymbol *glsl_stdlib_gl_in;", file=outf)
        print("Symbol *glsl_stdlib_gl_out;\n", file=outf)

        # producing the external api
        print_api(outf)

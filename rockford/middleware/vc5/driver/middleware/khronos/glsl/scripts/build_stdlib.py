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
    print >>outf, """\
static void glsl_stdlib_correct_overloads_for_mask(int mask) {
   for(int i=0; i<GLSL_STDLIB_FUNCTION_COUNT; i++) {
      if(!(glsl_stdlib_function_properties[i] & mask)) {
         Symbol *overload = glsl_stdlib_functions[i].u.function_instance.next_overload;
         glsl_stdlib_functions[i].u.function_instance.next_overload = NULL;
         while(overload) {
            const size_t overload_index = glsl_stdlib_function_index(overload);
            if(!(glsl_stdlib_function_properties[overload_index] & mask)) {
               glsl_stdlib_functions[i].u.function_instance.next_overload = overload;
               break;
            }
            overload = overload->u.function_instance.next_overload;
         }
      }
   }
}
"""

def print_header_api(outf):
    print >>outf, """\
/* This contains the source code for all functions in the standard library */
extern const char *glsl_stdlib_function_bodies[GLSL_STDLIB_FUNCTION_COUNT];

/* All preprocessor directives in the GLSL are gathered here and become the first parsed data */
extern const char *glsl_stdlib_setup;

/* These are the properties set on the standard functions (e.g. fragment-only etc) */
extern const int   glsl_stdlib_function_properties[GLSL_STDLIB_FUNCTION_COUNT];

/* Initialization functions. The masks indicate properties that must _not_ be set for an entry
   to be added to the symbol table. */
void glsl_stdlib_init(void);
void glsl_stdlib_populate_symbol_table_with_functions(SymbolTable *symbol_table, int mask);
void glsl_stdlib_populate_symbol_table_with_variables(SymbolTable *symbol_table, int mask);
void glsl_stdlib_populate_symbol_table_with_types    (SymbolTable *symbol_table, int mask);
void glsl_stdlib_populate_scalar_value_map(Map *map);

/* Query functions */
const Symbol *glsl_stdlib_get_function(glsl_stdlib_function_index_t index);
Symbol       *glsl_stdlib_get_variable(glsl_stdlib_variable_index_t index);
bool glsl_stdlib_is_stdlib_symbol(const Symbol *sym);
glsl_stdlib_function_index_t glsl_stdlib_function_index(Symbol *sym);
"""

def print_api(outf):
    print_banner(outf, ["The external API used by clients of this file"])
    print >>outf, """\
void glsl_stdlib_init() {
   glsl_stdlib_variables = malloc_fast(sizeof(*glsl_stdlib_variables) * GLSL_STDLIB_VARIABLE_COUNT);
   glsl_stdlib_functions = malloc_fast(sizeof(*glsl_stdlib_functions) * GLSL_STDLIB_FUNCTION_COUNT);
}

void glsl_stdlib_populate_symbol_table_with_functions(SymbolTable *symbol_table, int property_mask) {
   glsl_stdlib_init_functions();
   glsl_stdlib_correct_overloads_for_mask(property_mask);
   for(int i=0; i<GLSL_STDLIB_FUNCTION_COUNT; i++) {
      if(!(glsl_stdlib_function_properties[i] & property_mask)) {
         glsl_symbol_table_insert(symbol_table, &glsl_stdlib_functions[i]);
      }
   }
}

void glsl_stdlib_populate_symbol_table_with_variables(SymbolTable *symbol_table, int property_mask) {
   glsl_stdlib_init_variables();
   for(int i=0; i<GLSL_STDLIB_VARIABLE_COUNT; i++) {
      if(!(glsl_stdlib_variable_properties[i] & property_mask)) {
         Symbol *symbol = &glsl_stdlib_variables[i];
         glsl_symbol_table_insert(symbol_table, symbol);
         glsl_shader_interfaces_update(g_ShaderInterfaces, symbol);
      }
   }
}

void glsl_stdlib_populate_symbol_table_with_types(SymbolTable *symbol_table, int property_mask) {
   glsl_stdlib_init_types();
   for (int i=0; i<GLSL_STDLIB_TYPE_COUNT; i++) {
      if (!(glsl_stdlib_type_properties[i] & property_mask)) {
         if (glsl_stdlib_types[i].flavour != SYMBOL_STRUCT_TYPE) continue;
         glsl_symbol_table_insert(symbol_table, &glsl_stdlib_type_symbols[i]);
      }
   }
}

void glsl_stdlib_populate_scalar_value_map(Map *map) {
   glsl_stdlib_fill_scalar_value_map(map);
}
const Symbol *glsl_stdlib_get_function(glsl_stdlib_function_index_t index) {
   return &glsl_stdlib_functions[index];
}
Symbol *glsl_stdlib_get_variable(glsl_stdlib_variable_index_t index) {
   return &glsl_stdlib_variables[index];
}
bool glsl_stdlib_is_stdlib_symbol(const Symbol *sym) {
   if(sym >= glsl_stdlib_functions && sym < glsl_stdlib_functions + GLSL_STDLIB_FUNCTION_COUNT) {
      return true;
   }
   if(sym >= glsl_stdlib_variables && sym < glsl_stdlib_variables + GLSL_STDLIB_VARIABLE_COUNT) {
      return true;
   }
   return false;
}
glsl_stdlib_function_index_t glsl_stdlib_function_index(Symbol *sym) {
   return (glsl_stdlib_function_index_t)(sym - glsl_stdlib_functions);
}
"""

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

    return "__".join(["glsl_stdlib_fn"] + [fn_name,"_".join(fn_return)]+["_".join(ptype) for (pname,ptype) in fn_params])

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

        print >>outf, "      /* %s */" % manglename(fn)
        outf.write(body)
    outf.write("};\n")

def output_properties(outf, functions, variables, structs, arrays, props):
    prop_enum = (lambda x,lookup: (" | ".join(["GLSL_STDLIB_PROPERTY_%s" % p for p in props[lookup(x)]]), lookup(x))
                 if   lookup(x) in props and len(props[lookup(x)]) > 0
                 else (0, lookup(x)))
    print_array(outf, "const int", "glsl_stdlib_function_properties", "GLSL_STDLIB_FUNCTION_COUNT",
                [prop_enum(fn,fn_index) for fn in functions])

    print_array(outf, "const int", "glsl_stdlib_variable_properties", "GLSL_STDLIB_VARIABLE_COUNT",
                [prop_enum(var,var_index) for var in variables])

    print_array(outf, "const int", "glsl_stdlib_type_properties", "GLSL_STDLIB_TYPE_COUNT",
                [prop_enum((s,), compoundtypeindex) for s,b in structs] +
                [prop_enum( a,   compoundtypeindex) for a   in arrays ])


def output_properties_define(outf, functions, props):
    all_props = set()
    for f,p in props.iteritems():
        all_props.update(p)
    all_props = list(all_props)
    all_props.sort()
    print_defines(outf,[("GLSL_STDLIB_PROPERTY_%s" % p, "(1<<%d)" % i) for i,p in enumerate(all_props)])

def make_sig(fn):
    fn_name, fn_return, fn_params, fn_body = fn
    return tuple([" ".join(fn_return)] + [" ".join(t) for (p,t) in fn_params])

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
    return "__".join(["glsl_stdlib_param"] + [p.replace(" ","_") for p in param]).upper()

def param_qual(param):
    quals = ["in", "out", "inout"]
    for q in quals:
        if q in param:
            return "PARAM_QUAL_%s" % q.upper()
    return "PARAM_QUAL_IN"

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
    print >>outf, "    glsl_symbol_construct_param_instance(&glsl_stdlib_params[%s], \"\", &%s, STORAGE_NONE, %s);" % (
         param_index(param), typeenumname((param[-1],),primtypemap), param_qual(param))

def output_fn_struct(outf,fn, overload, c_func):
    fn_name, fn_return, fn_params, fn_body = fn

    outf.write("   glsl_symbol_construct_function_instance(\n")
    outf.write("      &glsl_stdlib_functions[" + fn_index(fn) + "],\n")
    outf.write("      intern_" + fn_name + ",\n")
    outf.write("      &" + sigstructname(make_sig(fn)) + ",\n")
    if c_func == None:
        outf.write("      NULL, /* Folding function would be %s */\n" % c_name(fn))
    else:
        outf.write("      (FoldingFunction)"+c_func+",\n")
    if overload == None:
        outf.write("      NULL, /* No overload */\n")
    else:
        outf.write("      &glsl_stdlib_functions[" + fn_index(overload) + "],\n")
    outf.write("      false); /* No prototype */\n")

def qual_from_set(var, recognised_quals):
    var_name, var_type, rhs = var

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

def storage_qualifier(var):
    return qual_from_set(var, set([ "const", "uniform", "in", "out" ]))

def type_qualifier(var):
    return qual_from_set(var, set([ "centroid", "flat" ]))

def type_precision(var):
    return qual_from_set(var, set([ "mediump", "lowp", "highp"]))

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

def make_type_from_var(var):
    var_name, var_type, rhs = var
    storage_qual = storage_qualifier(var)
    type_qual = type_qualifier(var)
    precision = type_precision(var)
    residual = filter(lambda a: (a != type_qual) and (a != storage_qual) and (a != precision), var_type)
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

def var_index(var):
    var_name, var_type, rhs = var
    var_name = safe_name(var_name)
    t_qual = type_qualifier(var)
    s_qual = storage_qualifier(var)
    precision = type_precision(var)
    tname = make_type_from_var(var)
    if (t_qual == "None"):
        q_str = [s_qual]
    else:
        q_str = ["__".join([t_qual] + [s_qual])]
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

def find_arrays(variables):
    arrays = set()
    for var in variables:
        t = make_type_from_var(var)
        if len(t) > 1:
            for i in reversed(range(len(t)-1)):
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
        outf.write("      %s.u.struct_type.member[%d].name =\n         \"%s\";\n" % (
                ename, i, var_name))
        outf.write("      %s.u.struct_type.member[%d].type =\n         &%s;\n" % (
                ename, i, typeenumname(var_types[i],primtypemap)))
        outf.write("      %s.u.struct_type.member[%d].layout =\n      NULL;\n" % (
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
            print >>sys.stderr, "Bad rhs variable %s: %s" % (var_name, rhs)
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

    assert((is_const and len(rhs_values) > 0) or not is_const)

    outf.write("   /* %s */\n" % var_name)
    outf.write("   {\n")
    outf.write("      FullySpecType fs_type;\n")
    outf.write("      fs_type.quals.invariant = " +  invariant_str + ";\n")
    outf.write("      fs_type.quals.lq        = NULL;\n")
    outf.write("      fs_type.quals.sq        = STORAGE_"   + storage_qualifier(var).upper() + ";\n")
    outf.write("      fs_type.quals.tq        = TYPE_QUAL_" + type_qualifier(var).upper()    + ";\n")
    outf.write("      fs_type.quals.pq        = PREC_"      + type_precision(var).upper()    + ";\n")
    outf.write("      fs_type.type            = &"+typeenumname(make_type_from_var(var),primtypemap) + ";\n")

    outf.write("      const_value *compile_time_value = NULL;\n")
    if is_const:
        outf.write("         compile_time_value = malloc_fast(%d * sizeof(const_value));\n" % (len(rhs_values)))
        for i, v in enumerate(rhs_values):
            v = v.strip().rstrip()
            outf.write("      compile_time_value[%d] = %s;\n" % (i, v))

    outf.write("      glsl_symbol_construct_var_instance(\n")
    outf.write("         &glsl_stdlib_variables[" +  var_index(var)  + "],\n")
    outf.write("         intern_" + safe_name(var_name) + ",\n")
    outf.write("         &fs_type,\n")
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
    for p in primtypemap.values():
        print >>outf, "typedef const_value const_%s%s%s;" % (
            p.name, "[%d]" % p.dim1 if p.dim1 != None else "", "[%d]" % p.dim2 if p.dim2 != None else "")

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
    properties[var_index(discard)] = { "FRAGMENT_ONLY" }

    sigs      = find_sigs     (glsl_functions)
    types     = find_types    (glsl_functions)
    fn_names  = find_fn_names (glsl_functions)
    params    = find_params   (glsl_functions)
    var_names = find_var_names(glsl_variables)
    arrays    = find_arrays   (glsl_variables)
    constants = find_constants(glsl_variables)
    structmap = dict(glsl_structs)

    glsl_functions.sort(lambda x,y: cmp(x[0],y[0]))
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
        print_includes(outf, ["glsl_symbols.h", "glsl_symbol_table.h"])
        output_functions_enum(outf, glsl_functions)
        output_variables_enum(outf, glsl_variables)
        output_properties_define(outf, glsl_functions, properties)
        print_header_api(outf)
        print_inclusion_guard_end(outf, "glsl_stdlib.auto.h")

    ## PRODUCING THE IMPLEMENTATION FILE
    with open(os.path.join(output_dir,"glsl_stdlib.auto.c"), "w") as outf:
        print_disclaimer(outf)
        print_includes(outf, ["stdlib.h", "stdio.h"], system=True)
        print_includes(outf, ["glsl_ast.h",             "glsl_compiler.h",    "glsl_common.h",
                              "glsl_const_operators.h", "glsl_fastmem.h",     "glsl_globals.h",
                              "glsl_intern.h",          "glsl_stdlib.auto.h", "glsl_trace.h",
                              "glsl_primitive_types.auto.h", "glsl_symbol_table.h"])
        output_sigs_enum     (outf, sigs)
        output_types_enum    (outf, glsl_structs, arrays, constants)
        output_params_enum   (outf, params)
        output_typedefs      (outf, primtypemap)

        # global variables
        print >>outf, """
Symbol     *glsl_stdlib_functions;
Symbol     *glsl_stdlib_variables;
static SymbolType glsl_stdlib_function_signatures[GLSL_STDLIB_FUNCTION_SIG_COUNT];
static SymbolType glsl_stdlib_types              [GLSL_STDLIB_TYPE_COUNT];
static Symbol     glsl_stdlib_type_symbols       [GLSL_STDLIB_TYPE_COUNT];
static Symbol     glsl_stdlib_params             [GLSL_STDLIB_PARAM_COUNT];

const char *glsl_stdlib_setup = """,

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
        print >>outf, """\
static void glsl_stdlib_init_types(void) {
   for (int i = 0; i < GLSL_STDLIB_FUNCTION_SIG_COUNT; i++) {
      glsl_stdlib_function_signatures[i].flavour       = SYMBOL_FUNCTION_TYPE;
      glsl_stdlib_function_signatures[i].scalar_count  = 0;
   }\
"""
        outf.write("\n")
        for sig in sigs:
            output_sig(outf,sig,primtypemap)
        for array in arrays:
            output_array(outf, array, constants, primtypemap)
        for struct in glsl_structs:
            output_struct_type(outf, struct, primtypemap)
        for param in params:
            output_param(outf,param,primtypemap)
        outf.write("}\n")
        outf.write("\n")

        print_helpers(outf)

        # stuffing the struct construction in
        print_banner(outf, ["Initializing symbols for functions in the library"])
        print >>outf, "static void glsl_stdlib_init_functions(void) {"
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
        print >>outf, "static void glsl_stdlib_init_variables() {"

        output_interns(outf,var_names)

        for var in glsl_variables:
            output_var_struct(outf, var, primtypemap, structmap)
            outf.write("\n")
        outf.write("}\n")
        outf.write("\n")

        print >>outf, "static void glsl_stdlib_fill_scalar_value_map(Map *scalar_value_map) {"
        outf.write("   Dataflow** scalar_values;\n")
        for var in glsl_variables:
            output_var_scalar_values(outf, var, "scalar_value_map")
        outf.write("}\n")

        # producing the external api
        print_api(outf)

import sys, os

def print_banner(outf, lines, comment_char = None):
    if comment_char == None:
        print >>outf, "/*" + ("*" * 100)
        line_start = " *"
    else:
        print >>outf, comment_char + (comment_char * (100/len(comment_char)))
        line_start = comment_char
    for line in lines:
        print >>outf, "%s %s" % (line_start,line)
    if comment_char == None:
        print >>outf, " " + ("*" * 100) + "*/"
    else:
        print >>outf, comment_char + (comment_char * (100/len(comment_char)))
    print >>outf

def print_disclaimer(outf, comment_char = None):
    lines = []

    lines.append("This file was AUTO-GENERATED at BUILD time. Editing it is FUTILE. The command was:")

    pad_column = max([len(arg)+3 for arg in sys.argv] + [len(sys.executable)])
    pad        = " "*(pad_column + 3 - len(sys.executable))
    lines.append("   %s%s\\" % (sys.executable,pad))

    for idx in range(len(sys.argv)):
        if idx < (len(sys.argv)-1):
            pad = " " * (pad_column - len(sys.argv[idx])) + "\\"
        else:
            pad = ""
        lines.append("      %s%s" % (sys.argv[idx],pad))
    lines.append("")
    lines.append("To make alterations, edit either the script or the library files referenced on the")
    lines.append("given command line, as appropriate.")

    print_banner(outf, lines, comment_char)

def make_2d_data(values, start="", end="", join_str=" "):
    maxlens = [0] * max([len(row) for row in values])
    for row in values:
        for i,v in enumerate(row):
            maxlens[i] = max(len(str(v)), maxlens[i])
    ret = []
    for row in values:
        ret.append(start + join_str.join(["%s%s" % (v, " "*(maxlens[i]-len(str(v))) if i!=len(row)-1 else "") for i,v in enumerate(row)]) + end)
    return ret

def print_array(outf, elt_type, name, dimension, values=[]):
    if len(values) == 0:
        print >>outf, "%s %s[1] = {};" % (elt_type, name)
        return

    print >>outf, "%s %s[%s] = {" % (elt_type, name, dimension)

    maxlen = max([len(str(v)) for v,k in values])
    for v,k in values:
        v = str(v)
        pad = " " * (maxlen-len(v))
        print >>outf, "   %s,%s /* %s */" % (v,pad,k)
    print >>outf, "};"
    print >>outf

def print_2d_array(outf, elt_type, name, dimension1, dimension2, values=[]):
    rows = make_2d_data([["%s," % r if i != len(v)-1 else r for i,r in enumerate(v)] for v,k in values], "{", "}")
    print_array(outf, elt_type, name, "%s][%s" % (dimension1,dimension2) , [(r,values[i][1]) for i,r in enumerate(rows)])

def print_flag_array(outf, elt_type, name, dimension, values=[]):
    rows = make_2d_data([v for v,k in values], "", "", " | ")
    print_array(outf, elt_type, name, dimension, [(r,values[i][1]) for i,r in enumerate(rows)])

def print_table(outf, header, rows):
    data = make_2d_data([header]+[[v if v != None else "-" for v in r] for r in rows])
    for d in data:
        print >>outf, d

def define_name(filename):
    return "%s_INCLUDED" % filename.replace(".", "_").upper()

def print_includes(outf, includes, system=False):
    active = list(includes)
    active.sort()
    format_string = "#include <%s>" if system else "#include \"%s\""
    for inc in active:
        print >>outf, format_string % inc
    print >>outf

def print_inclusion_guard_start(outf, filename):
    define = define_name(filename)
    print >>outf, "#ifndef %s" % define
    print >>outf, "#define %s" % define
    print >>outf

def print_inclusion_guard_end(outf, filename):
    define = define_name(filename)
    print >>outf, "#endif /* %s */" % define

def print_enum(outf, name, elements):
    print >>outf, "typedef enum _%s {" % name
    for element in elements:
        print >>outf, "   %s," % element
    print >>outf, "} %s;" % name
    print >>outf

def print_defines(outf, elements):
    maxlen = max([len(m) for m,v in elements])
    for m,v in elements:
        pad = " " * (maxlen - len(m))
        print >>outf, "#define %s%s %s" % (m,pad,v)
    print >>outf

def tf(v):
    if(v):
        return " true"
    return "false"


def bad_arg(arg):
    sys.stderr.write("Bad argument %s" % arg)
    sys.exit(1)

def parse_opts(first_arg=1):
    output_dir   = "."
    include_dirs = [ "." ]
    input_files  = []
    want_include = False
    want_output = False
    force_file  = False
    for arg in sys.argv[first_arg:]:
        if force_file:
            input_files.append(arg)
            continue
        if want_include:
            include_dirs.append(arg)
            want_include = False
            continue
        if want_output:
            output_dir = arg
            want_output = False
            continue
        if arg[0] == "-":
            if len(arg) == 1:
                bad_arg(arg)
            if arg[1] == 'I':
                if len(arg) > 2:
                    include_dirs.append(arg[2:])
                else:
                    want_include = True
            elif arg[1] == 'O':
                if len(arg) > 2:
                    output_dir = arg[2:]
                else:
                    want_output = True
            elif arg[1] == '-':
                if len(arg) > 2:
                    bad_arg(arg)
                else:
                    force_file = True
            else:
                bad_arg(arg)
        else:
            input_files.append(arg)
            force_file = True

    return output_dir, include_dirs, input_files

def find_file(f, include_dirs):
    if os.path.exists(f):
        return f
    for inc in include_dirs:
        fnew = os.path.join(inc, f)
        if os.path.exists(fnew):
            return fnew
    return None

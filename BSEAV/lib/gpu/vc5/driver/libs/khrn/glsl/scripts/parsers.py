from __future__ import print_function
import re, sys
from collections import namedtuple

def strip_array_parts(variables):
    if variables == None:
        return None
    temp_variables = []
    for name,returns,rhs in variables:
        if name[0] == '[':
            new_name    = returns[-1]
            new_returns = list(returns[:-1])
        elif name.count("[") > 0:
            parts = name.split("[")
            new_name = parts[0]
            new_returns = list(returns)
            new_returns.extend(["[%s" % p for p in parts[1:]])
        else:
            new_name = name
            new_returns = list(returns)
        temp_variables.append((new_name, new_returns, rhs))
    return temp_variables

def transfer_arrayness(pname, ptype):
    if pname.count("[") > 0:
        parts = pname.split("[")
        new_name = parts[0]
        new_type = list(ptype)
        new_type.extend(["[%s" % p for p in parts[1:]])
    else:
        new_name = pname
        new_type = list(ptype)
    return (new_name, new_type)

def parse_source(lines, need_body=True):
    functions        = []
    variables        = []
    directives       = []
    structs          = []
    fn_name          = None
    fn_body          = ""
    struct_name      = None
    struct_body      = []
    active_braces    = 0
    active_return    = []
    active_type      = []
    active_params    = []
    active_rhs       = []
    seen_open_paren  = False
    seen_close_paren = False
    want_rhs         = False
    saw_struct       = False
    want_struct      = False
    require_semicolon = False

    for line in lines:
        active_string = line
        while len(active_string)>0:
            if active_braces == 0 and seen_close_paren:
                if not need_body:
                    active_string = active_string.strip();
                    if active_string[0] != ";":
                        sys.stderr.write("Error: missing semicolon for function prototype in %s\n" % line)
                        sys.exit(1)
                    functions.append((fn_name, active_return, active_params, None))
                    fn_name          = None
                    fn_body          = ""
                    active_return    = []
                    active_params    = []
                    seen_open_paren  = False
                    seen_close_paren = False
                    break
                else:
                    # we are about to start a function or struct body
                    if len(active_string.strip()) == 0:
                        # ignore blank lines here
                        active_string = ""
                        continue
                    if active_string[0] != "{":
                        sys.stderr.write("Error: parser wanted an open brace on this line and didn't get it in %s" % line)
                        sys.exit(1)
                    active_braces = 1
                    active_string = active_string.strip("{")
            if active_braces > 0:
                # parsing a function body
                for i in range(len(active_string)):
                    if active_string[i] == "{":
                        active_braces += 1
                    elif active_string[i] == "}":
                        active_braces -= 1
                        if(active_braces == 0):
                            fn_body += active_string[:i]
                            active_string = active_string[i+1:]
                            # This is the moment when a function is complete if there is a body
                            functions.append((fn_name, active_return, active_params, fn_body))
                            fn_name          = None
                            fn_body          = ""
                            active_return    = []
                            active_params    = []
                            seen_open_paren  = False
                            seen_close_paren = False
                            break
                else:
                    fn_body      += active_string
                    active_string = ""
                    continue
            # At this point we are not parsing a function body
            # chop out comments here so that they still make it into bodies but
            # don't confuse matters in the header
            if active_string.count("//") > 0:
                # C++ style comment chops off the end of the line
                active_string = active_string[:active_string.find("//")]
            if re.search("\/\*",active_string) != None or re.search("\*\/", active_string) != None:
                # C-style comments
                sys.stderr.write("Error: avoid C-style comments in between functions in "
                                 "this glsl because the parser does not handle them in %s" % line)
                sys.exit(1)
            if len(active_string) > 0 and active_string[0] == "#":
                # preprocessor directive
                directives.append(active_string)
                active_string = ""
                continue
            # blank line at this point can be disappeared
            if len(active_string.strip()) == 0:
                active_string = ""
                continue
            # Parsing a symbol declaration
            # Tokenise the string, ensuring that ';' gets its own token
            tokens = active_string.replace(';', ' ; ').split()
            active_string = ""
            for tok_idx in range(len(tokens)):
                token = tokens[tok_idx].rstrip()
                if token == "struct":
                    saw_struct = True
                    continue
                if require_semicolon:
                    if token != ';':
                        sys.stderr.write("Expected ';' in line %s" % line)
                        sys.exit(1)
                    require_semicolon = False
                    continue
                if fn_name == None:
                    if token.count("=") > 0 and not want_struct:
                        parts = token.split("=")
                        if len(parts[0]) > 0:
                            fn_name = parts[0]
                        else:
                            fn_name = active_return.pop()
                        want_rhs = True
                        tail = "=".join(parts[1:])
                        if len(tail) > 1:
                            token = tail
                        else:
                            continue
                    elif saw_struct and token.count("{") > 0:
                        if len(active_return) != 1:
                            sys.stderr.write("Bad struct definition in line %s" % line)
                            sys.exit(1)
                        struct_name    = active_return.pop()
                        want_struct    = True
                        parts          = token.split("{")
                        active_string += "{".join(parts[1:])
                        break
                    elif want_struct and token.count("}") > 0:
                        structs.append((struct_name,struct_body))
                        saw_struct     = False
                        want_struct    = False
                        struct_body    = []
                        struct_name    = None
                        require_semicolon = True
                        parts          = token.split("}")
                        active_string += "}".join(parts[1:])
                        break
                    elif token.count(";") > 0:
                        fn_name = active_return.pop()
                        if want_struct:
                            struct_body.append((fn_name, active_return, None))
                        elif saw_struct and not need_body:
                            if len(active_return) != 0:
                                sys.stderr.write("Bad struct definition in line %s\n" % line)
                                sys.exit(1)
                            struct_name = fn_name
                            structs.append((struct_name, None))
                            struct_name = None
                            saw_struct  = False
                        else:
                            variables.append((fn_name, active_return, None))
                        fn_name         = None
                        active_return   = []
                        active_rhs = []
                        continue
                    elif token.count("(") == 0:
                        active_return.append(token)
                        continue
                    elif token.count("(") > 1:
                        sys.stderr.write("Error: parser didn't recognise function definition in line %s" % line)
                        sys.exit(1)
                    else:
                        parts           = token.split("(")
                        seen_open_paren = True
                        active_type     = []
                        if len(parts[0]) > 0:
                            fn_name = parts[0]
                        else:
                            fn_name = active_return.pop()
                        if len(parts[1]) > 0:
                            # e.g. the token was "fn_name(int"
                            # token is assigned so we can process it
                            # in the second part of the loop
                            token = parts[1]
                        else:
                            # e.g. the token was "foo(" or "("
                            continue
                if not seen_open_paren and not want_rhs:
                    # ensuring we have an open paren to begin the argument list
                    if token[0] == ";":
                        variables.append((fn_name, active_return, None))
                        fn_name         = None
                        active_return   = []
                        active_rhs = []
                        active_string  += token[1:]
                        break
                    elif token[0] == "=":
                        want_rhs = True
                        token = token[1:]
                    else:
                        if token[0] != "(":
                            sys.stderr.write("Error: parser expected an open paren next and didn't get it in line %s" % line)
                            sys.exit(1)
                        seen_open_paren = True
                        active_type     = []
                        if len(token) > 1:
                            token = token[1:]
                        else:
                            continue
                if want_rhs:
                    if token.count(";") == 0:
                        active_rhs.append(token)
                        continue
                    else:
                        variables.append((fn_name, active_return, active_rhs))
                        want_rhs   = False
                        fn_name         = None
                        active_return   = []
                        active_rhs = []
                        break
                if not seen_close_paren:
                    # parsing the argument list, which is comma separated
                    if token.count(")") > 1:
                        sys.stderr.write("Error: parser didn't understand your function definition (too many close parens) in line %s" % line)
                        sys.exit(1)
                    if token.count(")") == 0:
                        head_token, tail_token = token, None
                    else:
                        head_token, tail_token = token.split(")")
                        seen_close_paren = True
                    if head_token.count(",") > 1:
                        sys.stderr.write("Error: parser didn't understand your function definition (too many commas in one token) in line %s" % line)
                        sys.exit(1)
                    if head_token.count(",") == 1:
                        # e.g. the head token is something like is "a,int" or "a,"
                        param_name, next_type = head_token.split(",")
                        if len(param_name) == 0:
                            param_name = active_type.pop()
                    elif head_token != "":
                        param_name, next_type = head_token, None
                    else:
                        param_name, next_type = None, None

                    if tail_token == None:
                        if next_type != None:
                            active_params.append(transfer_arrayness(param_name, active_type))
                            if len(next_type) > 0:
                                active_type = [next_type]
                            else:
                                active_type = []
                        elif param_name != None:
                            active_type.append(param_name)
                    else:
                        if next_type != None:
                            #e.g. the token is "a,int)" or "a,)"
                            if len(next_type) > 0:
                                sys.stderr.write("Error: parser didn't understand your function definition (isolated typename?) in line %s" % line)
                            else:
                                sys.stderr.write("Error: parser didn't understand your function definition (trailing comma) in line %s" % line)
                            sys.exit(1)
                        if param_name != None:
                            active_params.append(transfer_arrayness(param_name, active_type))
                            active_type = []
                            active_string = tail_token
                if seen_close_paren:
                    break
            # set up active_string with the rest of the line
            if tok_idx < len(tokens)-1:
                active_string += " ".join(tokens[tok_idx+1:])

    # The variables require fixups because they might contain e.g. ("blah[10] or blah [10]")
    variables = strip_array_parts(variables)
    temp_structs = []
    for name,body in structs:
        new_body = strip_array_parts(body)
        temp_structs.append((name,new_body))

    return functions, variables, temp_structs, directives

def parse_props(lines, propmap, name_map_func, var_map_func, struct_map_func):
    active_props = []
    for line in lines:
        if line[0] == '#' or len(line.strip()) == 0:
            continue
        if not line[0].isspace():
            line_props = line.strip().rstrip().split()
            active_props = set()

            if len(line_props) > 0:
                if len(line_props[-1]) == 0 or line_props[-1][-1] != ":":
                    print("Error: property lines must end with a ':'", file=sys.stderr)
                    sys.exit(1)
                line_props[-1] = line_props[-1].rstrip(":")

            for p in line_props:
                p = p.replace("-", "_").upper()
                if len(p) > 0:
                    active_props.add(p)
        else:
            if len(active_props) == 0:
                print("Error: bad indentation in properties file", file=sys.stderr)
                sys.exit(1);
            else:
                functions,variables,structs,directives = parse_source([line], need_body=False)
                if len(directives) > 0 or (len(functions) == 0 and len(variables)==0 and len(structs)==0):
                    print("Error: Bad line %s in properties file" % line, file=sys.stderr)
                    sys.exit(1);
                for func in functions:
                    lookup = name_map_func(func)
                    if lookup not in propmap:
                        propmap[lookup] = set()
                    propmap[lookup].update(active_props)
                for var in variables:
                    lookup = var_map_func(var)
                    if lookup not in propmap:
                        propmap[lookup] = set()
                    propmap[lookup].update(active_props)
                for struct in structs:
                    lookup = struct_map_func((struct[0],))
                    if lookup not in propmap:
                        propmap[lookup] = set()
                    propmap[lookup].update(active_props)

def parse_table(name,lines,key=None):
    rtype   = None
    first   = True
    headers = None
    if key == None:
        rows = []
    else:
        rows = {}
    for line in lines:
        line = line.rstrip()
        if len(line) == 0 or line[0] == "#":
            continue
        if first:
            first   = False
            headers = line.split()
            rtype   = namedtuple(name, headers)
            continue
        fields = line.split()
        if len(fields) != len(headers):
            sys.stderr.write("Bad table file at line %s" % line)
            sys.exit(1)
        # standard substitutions
        initialiser = {}
        for i,f in enumerate(fields):
            if f == "y" or f == "Y" or f == "t" or f == "T":
                value = True
            elif f == "n" or f == "N" or f == "f" or f == "F":
                value = False
            elif f == "-" or f == "None":
                value = None
            else:
                value = f
                match = re.match("\"(.*)\"$", value)
                if match:
                    value = match,group(1)
                elif f.count(",") > 0:
                    value = set([a for a in f.split(",") if len(a) > 0])
                else:
                    match = re.match("[0-9]+$", value)
                    if match:
                        value = int(value)
            initialiser[headers[i]] = value

        row = rtype(**initialiser)
        if key == None:
            rows.append(row)
        else:
            rows[fields[headers.index(key)]] = row
    return rtype,rows

#!/usr/bin/env python
""" This script builds binary CLM BLOBs that may subsequently packaged as incremental CLM BLOBsor used for various test purposes

Usage:
./BuildBlob.py [options] clm__file.xml binary_blob_file_name

Important options

--clm_compiler_args First step of binary BLOB generation is generation of C-BLOB with
                    ClmCompiler. Hence this script has --clm_compiler_args switch that passes
                    its value as parameters to ClmCompiler (e.g.
                    --clm_compiler_args "--ccrev all --max_chains 1" will build base
                    (nonincremental) BLOB with all regions for use in single-antenna device)
                    Default is "--ccrev all"

--blob_type         Second step is compiling and linking C-BLOB created on previous step.
                    To be usable on a particular target platform binary BLOB shall have same
                    endianness and pointer length as used on this platform. These parameters
                    are specified with --blob_type switch. Following types are supported:
                    be32, le32, be64, le64 - for big/little endian 32/64 bit pointers
                    Also there is 'current' type - it builds BLOB for curent host platform
                    Several types may be specified, comma separated (on which case blob type
                    is appended to file name as suffix)
                    Default is "be32,le32,be64,le64"

Other options

Switches presented above are sufficient provided this script can find all it needs for its
operation. If it is not the case, it shall be assisted with the following switches:

--clm_compiler_dir   Where ClmCompiler.py located?
--clm_include_dirs   What directories to use to look for include files used for C-BLOB
                     generation (they are changed pretty often)
--cross_compiler_dir Where are cross-compilers are located
--verbose            Detailed report on what's going on

Requirements on environment

This script shall be run from bash or tcsh (CygWin is OK)

For building be/le 32/64 BLOBs this script uses cross-compilers. Broadcom cross-compilers
located here: /projects/hnd/tools/linux If for some reason they can't be used then
custom-built cross-compilers shall be used. Thsi script can use powerpc-elf and ia64-elf
cross-compilers built according to this recipe: http://wiki.osdev.org/GCC_Cross-Compiler

Thsi script returns exit code of 0 if requested BLOB(s) were built correctly, 1 otherwise
"""

import os
import sys
import getopt
import re
import platform
import tempfile
import subprocess

###############################################################################
# VARIOUS PARAMETERS
###############################################################################

# Default directories to look for ClmCompiler (relative to script directory)
_def_clm_compiler_search_path = ".:../ClmCompiler"

# Default directories to look for BLOB-related include files
_def_clm_includes_search_path = "../include:../../../shared/bcmwifi/include:../../../include:../ClmApi"

# Colon-separated list of default cross-compiler directories
_def_cross_comp_dirs = "/projects/hnd/tools/linux:/usr/local/cross"

# Default ClmCompiler arguments
_def_clm_comp_args = "--ccrev all"

# ClmCompiler script name
_clm_compiler_name = "ClmCompiler.py"

###############################################################################


class Options:
    """ Options obtained from command line

    Attributes:
    blob_types      - List of names of BLOB types that shall be built
    cross_comp_dirs - Directories under which cross-compilers are searched
    inc_dirs        - Colon-separated list of include directories for BLOB compilation
    clm_compiler    - Full name of ClmCompiler script
    clm_comp_args   - Arguments for CLM compiler
    clm             - CLM XML file name
    blob            - Binary BLOB name
    """
    def __init__(self):
        self.blob_types = []
        self.cross_comp_dirs = None
        self.inc_dirs = None
        self.clm_compiler = None
        self.clm_comp_args = None
        self.clm = None
        self.blob = None

###############################################################################

# True if intermediate printing is enabled
_verbose = False


def verbose_print(msg):
    """ Prints given message if verbose printing turned on """
    if _verbose:
        print msg


###############################################################################


class TargetOptions:
    """ Parameters for building some particulat BLOB type using some particular compiler

    Attributes:
    target        -- Empty for native (noncross) compiler, target name for custom-built
                     cross-compiler, "prefix/target" for Broadcom cross-compilers
                     (prefix is a directory that contains 'bin' folder)
    gcc_switches  -- Switches for gcc
    ld_switches   -- Switches for ld
    gcc_full_name -- Full name of gcc cross-compiler (empty for native
                     compiler). Unlike other attributes (set by constructor),
                     this attribute is set by BlobType.get_target_options()
                     during BLOB building
    """
    def __init__(self, target, gcc_switches, ld_switches):
        """ Constructor """
        self.target = target
        self.gcc_switches = gcc_switches
        self.ld_switches = ld_switches
        self.gcc_full_name = ""

    def gcc_cmd(self):
        """ Returns gcc command with target-specific switches """
        return "%sgcc %s" % (self._prefix(), self.gcc_switches)

    def ld_cmd(self):
        """ Returns ld command wth target-specific switches """
        return "%sld %s%s" % (self._prefix(),
                               ("-L %s " % os.path.join(self._root(), "lib")) if self.target else "",
                                self.ld_switches)

    def objcopy_cmd(self):
        """ Returns objcopy command wth target-specific switches """
        return "%sobjcopy" % self._prefix()

    def _prefix(self):
        """ Returns executable prefix """
        if not self.target:
            return ""
        return re.match(r"^(.*-)gcc[^/]*$", self.gcc_full_name).group(1)

    def _root(self):
        """ Returns root of cross compilers directory """
        return re.match(r"^(.*)/bin/[^/]*-gcc[^/]*$", self.gcc_full_name).group(1)


###############################################################################


class BlobType:
    """ Information needed to build BLOB of certain type

    Attributes:
    name    -- BLOB type name to use in --blob_type switch
    dsc     -- BLOB type description to show in help message
    targets -- List of TargetOptions objects for building this blob type
    """

    # List of cross compiler GCC executables
    _cross_compilers = None

    def __init__(self, name, dsc, default_build, targets):
        """ Constructor

        Arguments:
        name          -- BLOB type name to use in --blob_type switch
        dsc           -- BLOB type description to show in help message
        default_build -- Build if BLOB type not specified in command line
        targets       -- List of TargetOptions objects for building this blob
                         type
        """
        self.name = name
        self.dsc = dsc
        self.default_build = default_build
        self.targets = targets

    def get_target_options(self, options):
        """ Returns TargetOptions object that encapsulates cross-compiler for
        building BLOB of this type

        Arguments:
        options -- Command line options
        Returns TargetOptions object or None
        """
        for target_options in self.targets:
            m = re.match("([^/]*)(/([^/]*))?", target_options.target)
            target_prefix = m.group(1) if m.group(3) else None
            target_target = m.group(3) if m.group(3) else m.group(1)
            if not target_target:
                target_options.gcc_full_name = ""
                return target_options
            if BlobType._cross_compilers is None:
                BlobType._find_cross_compilers(options)
            for compiler in BlobType._cross_compilers:
                m = re.match(r"^.*/([^/]*)/bin/([^/]*)-gcc[^/]*$", compiler)
                compiler_prefix = m.group(1)
                compiler_target = m.group(2)
                if (compiler_target == target_target) and (not target_prefix or (target_prefix == compiler_prefix)):
                    target_options.gcc_full_name = compiler
                    return target_options
        return None

    @staticmethod
    def _find_cross_compilers(options):
        """ Fills _cross_compilers with list of found cross-compilers
        Each list element is a tuple. First element is full prefix for compiler
        (part of compiler name before '-gcc'), second element is short prefix
        (part of base fle name before '-gcc'), third element is full compiler
        name
        """
        BlobType._cross_compilers = []
        verbose_print("Looking for cross compiers. Following were found:")
        for compilers_root in options.cross_comp_dirs:
            try:
                found = subprocess.Popen('find %s -maxdepth 2 -perm /a+x -type f -path "*/bin/*-gcc*"' % os.path.join(compilers_root, "*"),
                                         shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0]
                for compiler in found.split("\n"):
                    if re.match(r"^.*/bin/[^/]+-gcc(\.exe)?$", compiler):
                        verbose_print(compiler)
                        BlobType._cross_compilers.append(compiler)
            except:
                pass
        if not BlobType._cross_compilers:
            verbose_print("No cross-compilers found!")

# Currently supported BLOB types
_blob_types = [
    BlobType("current", "Current architecture", False,
             [TargetOptions("", "", "")]),
    BlobType("le32", "32-bit little-endian", True,
             [TargetOptions("powerpc-elf", "-m32 -mlittle", "-EL"),
              TargetOptions("mips-sde6.06.1/sde", "-mips32 -mabi=32 -EL -mel", "-EL -m elf32ltsmip")]),
    BlobType("be32", "32-bit big-endian", True,
             [TargetOptions("powerpc-elf", "-m32 -mbig", "-EB"),
              TargetOptions("mips-sde6.06.1/sde", "-mips32 -mabi=32 -EB -meb", "-EB -m elf32btsmip")]),
    BlobType("le64", "64-bit little-endian (64-bit ptr align)", True,
             [TargetOptions("ia64-elf", "-mlittle-endian -Wa,-mle", "-EL"),
              TargetOptions("mips-sde6.06.1/sde", "-mips64 -mabi=64 -EL -mel", "-EL -m elf64ltsmip")]),
    BlobType("be64", "64-bit big-endian (64-bit ptr align)", True,
             [TargetOptions("ia64-elf", "-mbig-endian -Wa,-mbe", "-EB"),
              TargetOptions("mips-sde6.06.1/sde", "-mips64 -mabi=64 -EB -meb", "-EB -m elf64btsmip")]),
]

###############################################################################


# Help message
_usage = """ Usage
./BuildBlob.py [options] source.xml blob_file
Options are:
--blob_type type         - Blob type or comma-separated list of types.
                           Available types are: %s
                           Default is: "%s"
--clm_compiler_args args - ClmCompiler arguments without CLM and BLOB file
                           names. Default is "%s"
                           Since some versions of 'bcmwifi_rates.h' do not
                           define VHT rates, this switch _must_ be specified
                           explicitly if process fails with compilation
                           messages on 'WL_RATE_..._VHT... undefined'
                           (e.g. "--ccrev all --abgn")
--clm_compiler_dir dir   - Directory where ClmCompiler.py located. If not
                           specified then
                           %s
                           relative to script directory are searched
--clm_include_dirs dirs  - Colon-separated list of directories to look
                           for include files for BLOB compilation. If not
                           specified then
                           %s
                           relative to script directory are searched
--cross_compiler_dir dir - Directory under which (no more than 3 level deep)
                           cross-compilers are located. By default following
                           candidates are searched:
                           %s
--verbose                  Prints report on what's going on
--help                   - Prints this text
source.xml               - CLM XML file
blob_file                - Resulted binary BLOB file (just as it shall be in
                           executable - no headers/trailers/encodings are
                           added). If more than one architecture is requested
                           - architecture names are appended to resulted file
                           names

All blob types except 'current' are built by means of gcc cross-compilers.
Cross compilers may be standard Broadcom ones (located in
/projects/hnd/tools/linux) or custom-built (this script may use powerpc-elf
and ia64-elf built per this receipt: http://wiki.osdev.org/GCC_Cross-Compiler)
""" % (
"".join("\n                           %s - %s" % (bt.name, bt.dsc) for bt in _blob_types),
",".join(bt.name for bt in _blob_types if bt.default_build),
_def_clm_comp_args,
",".join(_def_clm_compiler_search_path.split(":")),
",".join(_def_clm_includes_search_path.split(":")),
"\n                           ".join(_def_cross_comp_dirs.split(":")))

###############################################################################


def error(errmsg):
    """ Prints given error message and exits """
    print >> sys.stderr, errmsg
    sys.exit(1)

#------------------------------------------------------------------------------


def get_args():
    """ Parses command-line arguments

    Returns Options object properly filled
    """
    global _verbose
    ret = Options()
    script_dir = os.path.split(sys.argv[0])[0]
    ret.blob_types = [bt.name for bt in _blob_types]
    ret.cross_comp_dirs = _def_cross_comp_dirs.split(":")
    ret.inc_dirs = ":".join([os.path.join(script_dir, rel_path) for rel_path in _def_clm_includes_search_path.split(":")])
    ret.clm_comp_args = _def_clm_comp_args
    try:
        opts, args = getopt.getopt(sys.argv[1:], "?",
                                   ["blob_type=", "cross_compiler_dir=", "clm_compiler_dir=",
                                    "clm_include_dirs=", "clm_compiler_args=", "verbose", "help"])
    except getopt.GetoptError:
        error(str(sys.exc_info()[1]))
    for opt, arg in opts:
        if opt == "--blob_type":
            ret.blob_types = arg.split(",")
        elif opt == "--cross_compiler_dir":
            if not os.path.isdir(arg):
                error("Switch %s invalid: directory %s not found" % (opt, arg))
            ret.cross_comp_dirs = [arg]
        elif opt == "--clm_compiler_dir":
            ret.clm_compiler = os.path.join(arg, _clm_compiler_name)
            if not os.path.isfile(ret.clm_compiler):
                error("Switch %s invalid: directory %s does not contain %s" % (opt, arg, _clm_compiler_name))
        elif opt == "--clm_include_dirs":
            ret.inc_dirs = arg
        elif opt == "--clm_compiler_args":
            ret.clm_comp_args = arg
        elif opt == "--verbose":
            _verbose = True
        elif opt in ("-?", "--help"):
            print _usage
            sys.exit(0)
        else:
            error("Internal error. Unprocessed option %s" % opt)
    if ret.clm_compiler is None:
        for rel_path in _def_clm_compiler_search_path.split(":"):
            full_name = os.path.join(script_dir, rel_path, _clm_compiler_name)
            if os.path.isfile(full_name):
                ret.clm_compiler = full_name
                break
        else:
            error("%s not found in default places. Please use --clm_compiler_dir switch" % _clm_compiler_name)
    if len(args) >= 1:
        if not os.path.isfile(args[0]):
            error("CLM file %s not found" % args[0])
        ret.clm = args[0]
    else:
        error("CLM file name not specified")
    if len(args) >= 2:
        ret.blob = args[1]
    else:
        error("Binary BLOB file name not specified")
    return ret

#------------------------------------------------------------------------------


def execute(cmdline):
    """ Executes given command in shell

    Arguments:
    cmdline - Shell command line
    """
    try:
        verbose_print(cmdline)
        sp = subprocess.Popen(args=cmdline, shell=True)
        exit_code = sp.wait()
        if exit_code:
            error("Command failed with exit code %s" % exit_code)
    except SystemExit:
        raise
    except:
        error(str(sys.exc_info()[1]))

#------------------------------------------------------------------------------


def build_blob(blob_type, c_blob_name, blob_name, options, sections):
    """ Builds BLOB of given type

    Arguments:
    blob_type   -- BLOB type desccriptor (BlobType object)
    c_blob_name -- C-file that ciontains BLOB in C form
    blob_name   -- Name of binary file for resultng BLOB
    options     -- Options from command line
    sections    -- List of sections to extract from executable
    """
    try:
        o_blob_name = tempfile.mkstemp(prefix=os.path.split(options.blob)[1], suffix=".o")[1]
        e_blob_name = tempfile.mkstemp(prefix=os.path.split(options.blob)[1], suffix=".elf")[1]
        target_options = blob_type.get_target_options(options)
        if target_options is None:
            error("No cross compiler found for building %s BLOB" % blob_type.name)
        execute("%s -c -nostartfiles -nostdlib -o %s %s %s" %
                    (target_options.gcc_cmd(), o_blob_name, " ".join(["-I%s" % inc_dir for inc_dir in options.inc_dirs.split(":")]), c_blob_name))
        execute("%s -e 0 --sort-section name -o %s %s" % (target_options.ld_cmd(), e_blob_name, o_blob_name))
        execute("%s -O binary -j %s %s %s" % (target_options.objcopy_cmd(), " -j ".join(sections), e_blob_name, blob_name))
    finally:
        execute("rm -f %s" % o_blob_name)
        execute("rm -f %s" % e_blob_name)

#------------------------------------------------------------------------------


if __name__ == "__main__":
    options = get_args()
    verbose_print("Building architecture-specific BLOBs using GCC cross-compilers\n")
    shell = os.environ.get("SHELL", "")
    if ("bash" not in shell) and ("tcsh" not in shell):
        error("This script shall be run from bash or tcsh shell")
    blob_type_dict = {}
    for blob_type in _blob_types:
        blob_type_dict[blob_type.name] = blob_type
    success = True
    try:
        # Creating C-BLOB
        c_blob_name = tempfile.mkstemp(prefix=os.path.split(options.blob)[1], suffix=".c")[1]
        execute("python %s --force_section blob %s %s %s" % (options.clm_compiler, options.clm_comp_args, options.clm, c_blob_name))
        # Obtaining all section names used. This block of code is useful for per-variable sections.
        # Currently one section is used, but block left for possible future use
        f = open(c_blob_name)
        sections = []
        for m in re.finditer(r'__attribute__\s*\(\s*\(__section__\s*\(\s*"([^"]+)"\s*\)\s*\)\s*\)', f.read()):
            if m.group(1) not in sections:
                sections.append(m.group(1))
        f.close()
        # Building all necessary BLOBs
        for blob_type_name in options.blob_types:
            if blob_type_name not in blob_type_dict:
                error("Wrong BLOB type: %s" % blob_type_name)
            blob_name = (options.blob + blob_type_name) if len(options.blob_types) > 1 else options.blob
            verbose_print("\nBuilding %s BLOB \"%s\"" % (blob_type_name, blob_name))
            build_blob(blob_type_dict[blob_type_name], c_blob_name, blob_name, options, sections)
    finally:
        execute("rm -f %s" % c_blob_name)
    verbose_print("\nAll BLOBs successfully built")
    sys.exit(0)

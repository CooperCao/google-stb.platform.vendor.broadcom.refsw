#!/usr/bin/env python2.7
"""

This module manages the core configuration database, based on
YAML. Here's how it works:

Normally, reading a YAML document into a Python program results in
a data structure which is a tree of collections (dicts and lists)
with scalars at the leaves. However, we use a custom loader which
uses OrderedDicts instead of regular dicts. Furthermore, there's
a post-load tweak which converts lists into OrderedDicts too with
integers (0, 1, 2, ...) as the keys.  The net result is a tree
composed entirely of OrderedDicts with scalars at the edges.
From here on the terms "dict" and "OrderedDict" will be used
synonymously; as a subclass of dict, OrderedDict really is a dict.

Once the YAML file is read in as a structure of dicts, a traversal
is made during which a number of post-processing tasks take place.
Most importantly, each dict is replaced with a "proxy object"
containing that dict and configured to act like a dict. In
other words given a node "foo" we can still use "foo[key]",
"foo.get(key)", "for key in foo", etc but this now goes through the
proxy to reach the dict.

During the same traversal, subtrees may be copied wherever the COPY
key is found. Also, any VAR keys are found, processed, removed
from the tree, and pushed downward such that inheritance moves
towards the leaves.

The resulting structure in memory looks like the YAML document
in terms of order but VAR keys are removed and replaced with an
internal dictionary of variables derived from them. COPY keys
are removed and replaced with a copy of the subtree they pointed
to. The proxy objects may also contain additional metadata.

There is a related, user-focused description in the twiki page:
http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/GubConfig

"""

# pylint: disable=too-many-ancestors

from __future__ import print_function
from __future__ import unicode_literals

# OrderedDict is new in Python 2.7 but a compatible version exists.
try:
    from collections import OrderedDict
except ImportError:
    from ordereddict import OrderedDict  # pylint: disable=import-error

import distutils.version
import os
import re
import string
import sys

# Look for the fast C-based yaml parser first.
import yaml
try:
    from yaml import CLoader as Loader
except ImportError:
    from yaml import Loader
import yaml.constructor

import lib.consts
import lib.util

VAR_RE = re.compile(r'\s*(export\s+)?([A-Z_\d]+)\s*([:.+-]*)=\s*(.*)')

BUILDABLES = 'buildables'
COMMENT_KEY = 'COMMENT'
COPY_KEY = 'COPY'
COPY_RE = re.compile(r'\$\[(.+?)\]')
TAGS = 'tags'
VARS_KEY = 'VARS'


class ODLoader(Loader):

    """
    Load mappings into ordered dictionaries and produce unicode strings.

    Also allows a custom !include directive (which is nonstandard YAML).
    Derived from http://stackoverflow.com question #5121931.
    See also http://stackoverflow.com question #2890146.
    """

    INCLUDE = '!include'
    MAP_TAG = 'tag:yaml.org,2002:map'
    OMAP_TAG = 'tag:yaml.org,2002:omap'
    STR_TAG = 'tag:yaml.org,2002:str'

    def __init__(self, stream):
        self._root = os.path.split(stream.name)[0]
        self.add_constructor(ODLoader.INCLUDE, type(self).include)
        self.add_constructor(ODLoader.STR_TAG, type(self).construct_yaml_str)
        self.add_constructor(ODLoader.MAP_TAG, type(self).construct_yaml_map)
        self.add_constructor(ODLoader.OMAP_TAG, type(self).construct_yaml_map)
        super(ODLoader, self).__init__(stream)

    def include(self, node):
        """Implement the !include keyword."""
        path = os.path.join(self._root, self.construct_scalar(node))

        with open(path, 'r') as f:
            return yaml.load(f, ODLoader)

    def construct_yaml_str(self, node):
        """Convert regular byte strings to unicode."""
        return self.construct_scalar(node)

    def construct_yaml_map(self, node):
        data = OrderedDict()
        yield data
        value = self.construct_mapping(node)
        data.update(value)

    def construct_mapping(self, node, deep=False):
        if isinstance(node, yaml.MappingNode):
            self.flatten_mapping(node)
        else:
            warning = 'expected a mapping node, but found %s' % node.id
            raise yaml.constructor.ConstructorError(None, None, warning,
                                                    node.start_mark)

        mapping = OrderedDict()
        for key_node, value_node in node.value:
            key = self.construct_object(key_node, deep=deep)
            try:
                hash(key)
            except TypeError as exc:
                static_warning = 'while constructing a mapping'
                warning = 'found unacceptable key (%s)' % exc
                raise yaml.constructor.ConstructorError(static_warning,
                                                        node.start_mark,
                                                        warning,
                                                        key_node.start_mark)
            value = self.construct_object(value_node, deep=deep)
            mapping[key] = value
        return mapping


class Var(object):

    """
    A class to handle variables and the various assignment type
    and substitution rules.
    """

    def __init__(self, text, gvars=None):
        match = re.match(VAR_RE, text)
        assert match, 'illegal assignment: %s' % text
        self.name = match.group(2)
        if match.group(1) is not None or self.name in os.environ:
            self.exported = True
        elif gvars and self.name in gvars:
            self.exported = gvars[self.name].exported
        else:
            self.exported = False
        modifiers = match.group(3)
        self.sticky = '.' in modifiers  # currently undocumented
        rhs = match.group(4).rstrip()
        if '+' in modifiers:
            assert '-' not in modifiers
            if ':' in modifiers:
                # We'd prefer not to expand path settings
                # here because a task could still get shipped to
                # another machine. But it's tricky.
                orig = '$[%s]' % self.name
                entries = rhs.split()
                if modifiers.index('+') < modifiers.index(':'):
                    entries.append(orig)
                else:
                    entries.insert(0, orig)
                rhs = os.pathsep.join(entries)
            else:
                # Handle the "+=" operator. If both sides look like numbers
                # add them up; else treat them as strings and append the RHS.
                orig = '$[%s]' % self.name
                try:
                    prev = Var.subst(orig, gvars)
                    added = int(prev) + int(rhs)
                except ValueError:
                    try:
                        added = float(prev) + float(rhs)
                    except ValueError:
                        rhs = ' '.join([orig, rhs])
                    else:
                        rhs = '%f' % added
                else:
                    rhs = '%d' % added
            rhs = Var.subst(rhs, gvars)
        elif '-' in modifiers:
            if ':' in modifiers:
                # In path subtraction, order (-:= vs :-=) doesn't matter.
                orig = Var.subst('$[%s]' % self.name, gvars).split(os.pathsep)
                if orig:
                    for entry in rhs.split():
                        if entry in orig:
                            orig.remove(entry)
                    rhs = os.pathsep.join(orig)
            else:
                rhs = Var.subst(rhs, gvars)
                assert rhs
                orig = Var.subst('$[%s]' % self.name, gvars)
                assert orig
                rhs = ' '.join([w for w in orig.split()
                                if w not in rhs.split()])
        elif ':' in modifiers:
            rhs = Var.subst(rhs, gvars)

        self.value = rhs

    def __str__(self):
        return self.value

    def __repr__(self):
        return self.value

    def __hash__(self):
        return self.name.__hash__()

    def __cmp__(self, other):
        return cmp(self.name, other.name)

    class _Replace(string.Template):

        """
        Subclass of Template with a new method which replaces
        undefined variables with null strings.
        """

        # pylint: disable=no-member
        def substitute_or_null(self, *args, **kws):
            """
            This method is a copy from the 2.7.3 standard library
            string.Template.safe_substitute(), modified such
            that undefined variable references resolve to the
            null string instead of being left untouched.
            """

            if len(args) > 1:
                raise TypeError('Too many positional arguments')
            if not args:
                mapping = kws
            else:
                mapping = args[0]

            def convert(mo):
                """Helper function for .sub()"""
                named = mo.group('named')
                if named is not None:
                    try:
                        return '%s' % (mapping[named],)
                    except KeyError:
                        return self.delimiter + named
                braced = mo.group('braced')
                if braced is not None:
                    try:
                        return '%s' % (mapping[braced],)
                    except KeyError:
                        return ''
                if mo.group('escaped') is not None:
                    return self.delimiter
                if mo.group('invalid') is not None:
                    return self.delimiter
                raise ValueError('Unrecognized named group in pattern',
                                 self.pattern)
            return self.pattern.sub(convert, self.template)

    class _ReplaceSquare(_Replace):

        """
        Variant of _Replace which expands $[FOO] while leaving traditional
        shell-type expansions $FOO and ${FOO} alone for later build processes.
        """

        # The '(?!)' below is a guaranteed-not-to-match RE.
        pattern = r"""
        \$(?:
          (?P<escaped>(?!))               | # no escape char
          (?P<named>(?!))                 | # do not match $FOO, ${FOO}
          \[(?P<braced>[A-Z_][A-Z_\d]+)\] | # do match $[FOO]
          (?P<invalid>)
        )
        """

    @staticmethod
    def subst(text, gvars, final=False):
        """
        Replace $[FOO] tokens in the text string with their
        values, or with the null string if not defined. Note
        that definitions from both the environment and the
        config file are used, but only $[FOO] replacements
        are made - ${FOO} and $FOO are left alone.
        """
        if text and isinstance(text, basestring):
            vardict = os.environ.copy()
            if gvars:
                for k in gvars:
                    vardict[k] = gvars[k].value
            for _ in range(100):
                # Replace our own $[FOO] format from gvars + env.
                tmpl = Var._ReplaceSquare(text)
                sub = tmpl.substitute_or_null(vardict)
                # Optionally replace std ${FOO} and $FOO from env.
                if '$' in sub and final:
                    tmpl = Var._Replace(text)
                    sub = tmpl.substitute_or_null(vardict)
                if sub == text:
                    break
                text = sub

        return text

    @staticmethod
    def yaml_representer(dumper, obj):
        """For pretty-print debugging via __repr__()"""
        return dumper.represent_scalar('!var', repr(obj))


class ConfigProxy(object):

    """
    Hold the data required to make a Config object without doing so.
    """

    def __init__(self, cfgfile=None, gvars=None):
        self.cfgfile = cfgfile
        self.gvars = gvars

    def __repr__(self):
        return str(self.__dict__)

    def parse(self):
        """Return a Config object representing the parsed config file."""
        cfg = Config(cfgfile=self.cfgfile, gvars=self.gvars)
        return cfg


class Config(object):

    """
    A "proxy object" which is designed to hold a collection type
    and forward most methods to it, while maintaining its own
    set of metadata. The idea is that client programs should be
    able to navigate a tree of Config objects without having to
    interact with YAML at all.
    """

    root = None

    def __init__(self, coll=None, gvars=None, cfgfile=None):
        self.kpath = ['']
        self.gvars = ({} if gvars is None else gvars.copy())
        self.literal = {}
        if isinstance(coll, dict):
            self.ytype = 'dict'
        elif isinstance(coll, list):
            self.ytype = 'list'
            l2d = coll
            coll = OrderedDict()
            for i in range(len(l2d)):
                coll[i] = l2d[i]
        else:
            self.ytype = 'scalar'

        if Config.root:
            self.coll = coll
        else:
            assert cfgfile
            Config.root = self

            self.coll = yaml.load(open(cfgfile, 'r'), ODLoader)

            # Allow command-line overrides of root variables.
            if self.gvars:
                for assignment in self.gvars:
                    self.coll[VARS_KEY].append(assignment)
                self.gvars = {}

            # Post-process the data structure recursively.
            self.evaluate(gvars=self.gvars)

    # Pass methods like self.keys() to self.coll.keys()
    def __getattr__(self, name, *args, **kwargs):
        return getattr(self.coll, name)

    # Make object subscriptable
    def __getitem__(self, key):
        return self.get(key)

    # Make object iterable
    def __iter__(self):
        if isinstance(self.coll, OrderedDict):
            subbed = self.coll.copy()
            for k in subbed:
                if isinstance(subbed[k], basestring):
                    subbed[k] = Var.subst(subbed[k], self.gvars)
            return iter(subbed)
        else:
            return iter(self.coll)

    def __len__(self):
        return len(self.coll)

    def get(self, key, default=None, extravars=None, final=False):
        """Get the value of the requested key at this node."""
        if extravars:
            subvars = self.gvars.copy()
            for extravar in extravars:
                xvar = Var(extravar)
                subvars[xvar.name] = xvar
        else:
            subvars = self.gvars

        raw = self.coll.get(key, default)
        if isinstance(raw, basestring):
            subbed = Var.subst(raw, subvars, final=final)
            return subbed
        else:
            return raw

    def getvar(self, name, default=None, required=False):
        """Look up a variable at this node and return the value."""
        raw = self.gvars.get(name)
        if raw:
            return Var.subst(raw.value, self.gvars, final=True)
        else:
            assert not required, 'missing required value: ' + name
            return default

    def getvars(self):
        """Return the list of variables at this node."""
        return self.gvars.keys()

    # Interprets any of "{False,Off,No,None,Null,0}" (case insensitive)
    # or 0 or the null string to mean False. Everything else is True.
    def getbool(self, ref, default=None):
        """Look up a variable and return its value as a boolean."""
        if not ref.startswith('$'):
            ref = '$[%s]' % ref
        term = self.sub(ref, final=True)
        if term == '' and default is not None:
            return default
        else:
            return lib.util.boolean(term)

    def sub(self, data, tidy=True, extravars=None, final=False):
        """Subject the supplied data to standard variable replacement."""
        if extravars:
            subvars = self.gvars.copy()
            for extravar in extravars:
                xvar = Var(extravar)
                subvars[xvar.name] = xvar
        else:
            subvars = self.gvars

        # Assume this is either a string or a list of strings.
        if isinstance(data, basestring):
            subbed = Var.subst(data, subvars, final=final)
            if tidy:
                subbed = ' '.join(subbed.split())
        else:
            subbed = []
            for datum in data:
                if isinstance(datum, basestring):
                    item = Var.subst(datum, subvars, final=final)
                    if tidy:
                        item = ' '.join(item.split())
                    subbed.append(item)
                else:
                    subbed.append(datum)
        return subbed

    @staticmethod
    def resolve(ref, source=None):
        """Follow a COPY ref and return the target."""
        match = COPY_RE.search(ref.strip('/'))
        if not match:
            raise KeyError('bad reference: ' + ref)
        copied = Config.root
        for pk in match.group(1).split('/'):
            if not copied:
                raise KeyError('bad reference: ' + ref)
            if source and pk == '&':
                pk = source
            copied = copied[pk]
        return copied

    def evaluate(self, gvars=None):
        """Traverse the data tree recursively to post-process each node."""
        self.gvars = (gvars.copy() if gvars else {})

        # Handle variable (VARS) inheritance separately.
        vlist = self.pop(VARS_KEY, None)
        if vlist:
            assert isinstance(vlist, list), '%s must be a list' % VARS_KEY
            self.literal[VARS_KEY] = vlist
            for line in vlist:
                assert isinstance(line, basestring), 'not a string: %s' % line
                if not line or line.startswith(COMMENT_KEY):
                    pass
                elif line.startswith(COPY_KEY):
                    ref = line.split(None, 1)[1]
                    copied = Config.resolve(ref, self.kpath[-1])
                    if copied:
                        self.gvars.update(copied.gvars)
                else:
                    gv = Var(line, self.gvars)
                    name = gv.name
                    if name not in self.gvars or not self.gvars[name].sticky:
                        self.gvars[name] = gv

        coll = self.coll
        assert isinstance(coll, dict)

        # Ignore comment keys (which are largely redundant because
        # yaml supports actual comments).
        if COMMENT_KEY in coll:
            del coll[COMMENT_KEY]

        # Handle generic key inheritance.
        if COPY_KEY in coll:
            ref = coll.pop(COPY_KEY)
            other = Config.resolve(ref, self.kpath[-1])
            self.merge(other)

        # This is a workaround for an unsolved bug. A buildable item with only
        # a VARS entry (meant for inheritance) seems to be ignored, so when
        # that happens we stick in a dummy key.
        if not coll:
            coll['__dummy__'] = 'ignore this key'

        # Now recurse through all iterable keys.
        for k in coll:
            if isinstance(coll[k], dict) or isinstance(coll[k], list):
                coll[k] = Config(coll=self.coll[k], gvars=gvars)
                coll[k].kpath = ([] if self.kpath is None else self.kpath[:])
                coll[k].kpath.append(k)
                coll[k].evaluate(gvars=self.gvars)

    def merge(self, other):
        """Combine subtrees in COPY situations."""
        if not other:
            return
        for k in self.coll:
            if isinstance(self.coll[k], dict) or \
                    isinstance(self.coll[k], list):
                self.coll[k] = Config(coll=self[k], gvars=self.gvars)
        for k in other:
            if isinstance(other.coll[k], Config):
                if k not in self:
                    self.coll[k] = Config(coll=OrderedDict(), gvars=self.gvars)
                    self.coll[k].kpath.append(k)
                self[k].merge(other[k])
            elif k not in self:
                self.coll[k] = other.coll[k]
        for k in other.gvars:
            if k not in self.gvars:
                self.gvars[k] = other.gvars[k]
        self.ytype = other.ytype

    def exports(self, env=None, final=False):
        """Return the set of exported variables at the current node."""
        if env is None:
            env = {}
        for k in self.gvars:
            gv = self.gvars[k]
            if gv.exported:
                subbed = Var.subst(gv.value, self.gvars, final=final)
                env[gv.name] = subbed
        return env

    def __repr__(self):
        return yaml.dump(self.coll, indent=2, default_flow_style=False)

    def __str__(self):
        return '/'.join(self.kpath)

    def buildable(self, name):
        """Return the subkey of the named buildable."""
        return self.get(BUILDABLES, {}).get(name)

    def buildables(self):
        """Return the entire 'buildables' subkey."""
        return self.get(BUILDABLES, {})

    def known_buildables(self):
        """Return a list of all defined buildable items."""
        chosen = [b for b in self.buildables() if not b[0].isupper()]
        return sorted(chosen)

    def enabled_buildables(self, tag):
        """Return the list of buildables enabled for the specified tag."""
        chosen = []
        tcfg = self.tags.get(tag)
        if tcfg:
            for name in sorted(tcfg.get(BUILDABLES, {}).values()):
                if name in self.buildables():
                    chosen.append(name)
                else:
                    lib.util.warn('unknown buildable "%s"' % name)

        return chosen

    def enabled_for(self, day):
        """Is this buildable configured to build on this day?"""
        days = self.get(lib.consts.NIGHTLY_DAYS)
        return not days or day in days.split(',')

    def build_prereqs(self):
        """Return the set of builds this buildable relies on."""
        return self.get('prereqs', {})

    def uses_hdw(self):
        """Return True if this buildable relies on HDW."""
        prqs = self.build_prereqs()
        return prqs and lib.consts.HDW in prqs

    def makeprog(self, tag, default=None):
        """Return the name of the requested make program."""
        # Determine which make program to use. Default is GNU make;
        # supported variants include lsmake and emake (deprecated).
        # A request to use lsmake/emake may be either a blanket request
        # ("True") or a RE matched against the branch name in case only
        # certain branches have been enhanced to support the variant.
        mkspec = self.getvar('LSMAKE')
        if not mkspec:
            mkspec = self.getvar('EMAKE')
        if mkspec:
            if mkspec == 'False':
                pass
            elif mkspec == 'True' or re.search(mkspec, tag):
                return 'emake' if self.getvar('EMAKE') else 'lsmake'
        return default

    @staticmethod
    def _is_active_tag(tcfg):
        """Return True if this tag is configured for scheduled builds."""
        return tcfg.get('nightly_offset') is not None

    @property
    def tags(self):
        """Return a list of everything under the 'tags' key."""
        return self.get(TAGS, {})

    def active_tags(self):
        """Return a list of tags marked as "active"."""
        tags = self.tags
        chosen = [t for t in tags if Config._is_active_tag(tags[t])]

        def by_version(tag):
            """Sort derived tags by major.minor for readability."""
            # Make sure "trunk", "master", etc. sort above all else.
            match = re.search(r'_([\d_]*)$', tag)
            vers = match.group(1) if match else '10000.0'
            return distutils.version.LooseVersion(vers)
        return sorted(chosen, key=by_version, reverse=True)

    def enabled_tags(self, buildable):
        """Return a list of tags where the specified buildable is enabled."""
        chosen = set()
        tcfg = self.tags
        if tcfg:
            tags = self.active_tags()
            for tag in tags:
                if buildable in tcfg[tag][BUILDABLES].values():
                    chosen.add(tag)
        return sorted(chosen)

    def known_tags(self):
        """Return a list of all tags we know of."""
        return sorted(self.tags)

    def expand_tag(self, tag, unique=True):
        """Provide tail-matching of known tags for user convenience."""
        tag = tag.strip('/').replace('.', '_')
        known = self.known_tags()
        if tag in known:
            candidates = [tag]
        else:
            candidates = [c for c in known
                          if c.endswith(tag) and
                          Config._is_active_tag(self.tags[c])]
        if unique:
            if not candidates:
                candidates = [tag]
            elif len(candidates) > 1:
                lib.util.die('ambiguous tag match: ' + ', '.join(candidates))
            return candidates[0]
        else:
            return candidates

    @staticmethod
    def yaml_representer(dumper, obj):
        """For pretty-print debugging via __repr__()."""
        coll = (obj.coll if isinstance(obj, Config) else obj)
        if isinstance(coll, OrderedDict):
            if obj.ytype == 'list':
                result = dumper.represent_sequence('!list', coll.values())
            else:
                nd = OrderedDict(obj.literal)
                nd.update(coll)
                result = dumper.represent_mapping('!odict', nd)
        else:
            result = dumper.represent_scalar('!cfg', repr(coll))
        return result


yaml.add_representer(Config, Config.yaml_representer)
yaml.add_representer(Var, Var.yaml_representer)


def main():
    """For debug and unit tests."""
    sys.exit(9)

if __name__ == '__main__':
    main()

# vim: ts=8:sw=4:tw=80:et:

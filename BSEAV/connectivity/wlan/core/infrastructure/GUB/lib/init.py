"""Set up on-demand importing via bin/{hnd,gub,wcc}."""

from __future__ import print_function
from __future__ import unicode_literals

# Used for on-demand importing via bin/{hnd,gub,wcc}.
HND_SUBCMD_TEMPLATE = '; '.join([
    'from {base}.{subcmd} import parse_cli as {subcmd}_cli',
    'from {base}.{subcmd} import __doc__ as {alias}_doc',
    '{subcmd}_cli(lib.consts.PROG, b"{alias}", subparser, advanced, ' +
    'string.Template({alias}_doc.strip())' +
    '.substitute(lib.consts.__dict__))',
])


def setup(base=None, subcmds=None):
    """Add subcommands to the parser."""
    imports = ['import string']
    cmdnames = set()
    for subcmd in sorted(subcmds):
        aliases = set([subcmd])
        if subcmds[subcmd].get('aliases'):
            aliases |= set(subcmds[subcmd]['aliases'])
        for alias in aliases:
            cmdnames.add(alias)
            imports.append(HND_SUBCMD_TEMPLATE.format(
                alias=alias,
                base=base,
                subcmd=subcmd,
            ))

    return '\n'.join(imports), cmdnames

# vim: ts=8:sw=4:tw=80:et:

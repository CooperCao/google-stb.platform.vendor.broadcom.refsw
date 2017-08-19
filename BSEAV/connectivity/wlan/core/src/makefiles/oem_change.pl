#!/usr/bin/perl

# Replaces an inline recipe in msvs_rules.mk in the same directory.

print "sed \\\n";
for (<>) {
    if (s{^STR_(OEM_\S+)\s+L"(.*)"}{-e "s!%$1%!$2!g" \\}) {
	s{[\$]}{\\\$};
	print "  $_";
    }
}
print "  -e 's/\\\\\$/\\\$/' \n";

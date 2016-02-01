BEGIN {
    print "#include \"boxmodes_defines.h\"";
    print "bmemconfig_box_info g_bmemconfig_box_info[BMEMCONFIG_MAX_BOXMODES]={ {0,1"; # the rest of this line will be created
    # in either the bmemconfig_box_info.awk file (for 28nm platforms) or in the bmemconfig_box_40nm_info.awk file (for 40nm platforms)
    print "";
}
{
}
END {
}

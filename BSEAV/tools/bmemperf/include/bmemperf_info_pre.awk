BEGIN {
    print "#include \"bmemperf.h\"";
    print "#include \"bmemperf_info.h\"";
    print "bmemperf_info g_bmemperf_info={0,0,0};";
    print "char *g_client_name[BMEMPERF_MAX_NUM_CLIENT];";
    print "";
}
{
}
END {
}

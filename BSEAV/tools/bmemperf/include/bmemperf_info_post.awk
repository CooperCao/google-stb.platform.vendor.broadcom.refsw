BEGIN {
    struct_count = 0;
    ### printf ("bmemperf_boxmode_info g_boxmodes[]={");
    appendage = "bmemperf_boxmode_info g_boxmodes[]={";
###    {1,&g_bmemperf_info1,&g_client_name1[0]}
###    ,{2,&g_bmemperf_info2,&g_client_name2[0]}
###    ,{3,&g_bmemperf_info3,&g_client_name3[0]}
###    ,{4,&g_bmemperf_info4,&g_client_name4[0]}
###    ,{5,&g_bmemperf_info5,&g_client_name5[0]}
###    ,{6,&g_bmemperf_info6,&g_client_name6[0]}
###    ,{8,&g_bmemperf_info8,&g_client_name8[0]}
###    ,{9,&g_bmemperf_info9,&g_client_name9[0]}
###    ,{1000,&g_bmemperf_info1000,&g_client_name1000[0]}
###};
}
{
    ### bmemperf_info l_bmemperf_info1000={3,1067,432};
    ### char *l_bmemperf_clients1000[BMEMPERF_MAX_NUM_CLIENT] = {"xpt_wr_rs",
    field2a = substr($2,1,11);
    field2b = substr($2,2,11);
    if (field2a == "l_bmemperf_")
    {
        struct_count++;

        split($2,field_parts,"=");
        boxmode = substr(field_parts[1],16);
        ### printf ("/* field2 ("$2"); field2a ("field2a"); parts1 ("field_parts[1]"); boxmode ("boxmode") */\n");
        if ( struct_count > 1) appendage = appendage ",";
        appendage = appendage "{"boxmode",&"field_parts[1]",";
    }
    else if (field2b == "l_bmemperf_")
    {
        split($2,field_parts,"[");
        boxmode = substr(field_parts[1],20);
        ### printf ("/* field2 ("$2"); field2b ("field2b"); parts1 ("field_parts[1]"); boxmode ("boxmode") */\n");
        ### printf ("&"substr(field_parts[1],2)"[0]}\n");
        appendage = appendage "&"substr(field_parts[1],2)"[0]}\n";
    }
}
END {
    appendage = appendage "};\n"
    printf(appendage);
}

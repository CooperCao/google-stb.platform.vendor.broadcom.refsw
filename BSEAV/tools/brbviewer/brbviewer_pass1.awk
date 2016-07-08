##*****************************************************************************
## Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
##
## This program is the proprietary software of Broadcom and/or its
## licensors, and may only be used, duplicated, modified or distributed pursuant
## to the terms and conditions of a separate, written license agreement executed
## between you and Broadcom (an "Authorized License").  Except as set forth in
## an Authorized License, Broadcom grants no license (express or implied), right
## to use, or waiver of any kind with respect to the Software, and Broadcom
## expressly reserves all rights in and to the Software and all intellectual
## property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
## HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
## NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
##
## Except as expressly set forth in the Authorized License,
##
## 1. This program, including its structure, sequence and organization,
##    constitutes the valuable trade secrets of Broadcom, and you shall use all
##    reasonable efforts to protect the confidentiality thereof, and to use
##    this information only in connection with your use of Broadcom integrated
##    circuit products.
##
## 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
##    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
##    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
##    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
##    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
##    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
##    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
##    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
##
## 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
##    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
##    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
##    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
##    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
##    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
##    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
##    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
##******************************************************************************

### boxes[0] has names for MVD, VFD, DVI
### boxes[1] has names for SCL, XSRC, MVP, DNR, TNTD, FCH, LBOX, F_DRAIN
### boxes[2] has names for CAP, CMP, CRC, B_DRAIN
### boxes[3] has names for LPB

BEGIN {
    found_beginning_of_names=0;
    boxes_0 = "";
    boxes_1 = "";
    boxes_2 = "";
    boxes_3 = "";
    box0_count = 0;
    box1_count = 0;
    box2_count = 0;
    box3_count = 0;
    line = 0;
    reg_count = 0;
    loopbacks_num = 0; ### used during frontend processing to count the number of loopback (lpb) registers
    loopbacks_frontend = "";
    front_or_back = "";
    new_line = "";
    new_chars_count = 0;
    new_chars = "";
    box1_srcs = "";
    box1_srcs_count = 0; ### the 16th entry should be "none" for some reason
    box23_srcs = "";
    box23_srcs_count = 0; ### the 16th entry should be "none" for some reason
    bvnview_addrs_1 = "";
    bvnview_addrs_2 = "";
    bvnview_addrs_3 = "";
    VNET_ENABLE_ADDRS_FILE = "vnet_enable_addrs.txt"
}
function bvnview_addrs_add( new_addrs ) {
   if (length(bvnview_addrs_1) > 500 ) {
      if (length(bvnview_addrs_2) > 500 ) {
         bvnview_addrs_3 = bvnview_addrs_3 ",\\\"0xf0" new_addrs "\\\"";
      } else {
         bvnview_addrs_2 = bvnview_addrs_2 ",\\\"0xf0" new_addrs "\\\"";
      }
   } else {
      bvnview_addrs_1 = bvnview_addrs_1 ",\\\"0xf0" new_addrs "\\\"";
   }
}
function set_new_line( reg_abbrev, box_id, offset, addr_prefix, box ) {
### new_line = ",\\\"" temp "\\\":{\\\"reg_name\\\":\\\"" tolower($2) "\\\",\\\"box_id\\\":" box2_count ",\\\"name\\\":\\\"" temp "\\\",\\\"addr\\\":\\\"0xf0" substr($3,5,6) "\\\",\\\"box\\\":2}\""; box2_count++;
    if (new_chars_count != 0) {
       new_line = ",";
    }
    new_line = new_line "\\\"" reg_abbrev "\\\":{\\\"box_id\\\":" box_id",\\\"name\\\":\\\"" reg_abbrev "\\\",\\\"" addr_prefix "\\\":\\\"0xf0" offset "\\\",\\\"box\\\":" box "}\\n\"";
    if (box_id == 0) {
       box0_count++;
    } else if (box_id == 1) {
       box1_count++;
    } else if (box_id == 2) {
       box2_count++;
    } else if (box_id == 3) {
       box3_count++;
    }
}
function read_vnet_f_enable_addrs_file() {
    num_elements = 0;
    while (getline name_and_enable_addr < VNET_ENABLE_ADDRS_FILE) {
       if ( length(name_and_enable_addr) ) {
            num=split(name_and_enable_addr,parts,",");
            if (num) {
               enable_addr[parts[1]] = parts[2];
               ### print "/* got pair (" parts[1] ") and (" parts[2] ")*/";
               num_elements++;
            }
       }
    }
    ### print "/* num_elements (" num_elements ")*/";
}
{
    line++;
    if (line == 1)
    {
        num=split(FILENAME,filename_paths,"/");
        ### printf ("/* num filename_paths is (" num ") */\n");
        num=split(filename_paths[num],filename_parts,"_");
        ### printf ("/* num filename_parts is (" num ") */\n");
        printf ("/* Processing: (" FILENAME "); */\n");
        which_file = index(FILENAME,"bchp_vnet_b.h");
        ### printf "/* FILENAME:" FILENAME "\n";
        if ( which_file != 0 )
        {
             front_or_back = "b";
             getline loopbacks_num  < "loopbacks_num.txt";
             ### printf "/* backend: loopbacks_num = " loopbacks_num " */\n";
             getline new_chars_count < "new_chars_count.txt";
             tag_start = "BCHP_VNET_B_REVISION";
             tag_end = "BCHP_VNET_B_SCRATCH_REGISTER";
             printf "const unsigned long VNET_B_REGISTERS[] = {\n";
             read_vnet_f_enable_addrs_file();
        }
        else
        {
           front_or_back = "f";
            printf "#include \"bchp_vnet_b.h\"\n";
            printf "#include \"bchp_vnet_f.h\"\n\n";
            tag_start = "BCHP_VNET_F_REVISION";
            tag_end = "BCHP_VNET_F_SCRATCH_REGISTER";
            printf "\n\nconst unsigned long VNET_F_REGISTERS[] = {\n";

            ### new_chars = "static char h_info_" new_chars_count "[]=\"\";\n";
            ### new_chars_count++;
            print "" > VNET_ENABLE_ADDRS_FILE
        }
### printf "/* start:" tag_start "; end:" tag_end " */\n";
    }

    ### printf "/* line is " $1 " " $2 " " $3 " */\n";
    if ( found_beginning_of_names == 0 && $1 == "#define" && $2 == tag_start )
    {
        ### printf "/* found beginning of registers */\n";
        found_beginning_of_names = 1;
        printf $2 "  /* 0xf0" substr($3,5,6) " */\n";
        bvnview_addrs_add( substr($3,5,6) );
        reg_count++;
    }
    else if ( found_beginning_of_names == 1 && $1 == "#define" && $2 == tag_end && reg_count > 2 )
    {
        found_beginning_of_names = 0;
        printf "," $2 "  /* 0xf0" substr($3,5,6) " */\n};\n";
        bvnview_addrs_add( substr($3,5,6) );
        ### printf "found end of defines \n";
        reg_count++;
    }
    ###  this happens on the 7425 where the SCRATCH register is the 2nd one in the list
    else if ( found_beginning_of_names == 1 && index($1,"/********************************") )
    {
        ### printf "found comment \n";
        found_beginning_of_names = 0;
        printf "\n};\n";
    }
    else if ( found_beginning_of_names == 1 && $1 == "#define" )
    {
        ### printf "/* found register (" $2 ")*/\n";
### BCHP_VNET_F_MFD_0_ENABLE
        if ( index($2,"BCHP_VNET_F_MFD_") > 0 ) ### does not have reg name
        {
            enable = index($2,"_ENABLE");
            temp = tolower(substr($2,13,enable-13));
            if ( length(boxes_0) ) boxes_0 = boxes_0 ",";
            boxes_0 = boxes_0 "\\\"" temp "\\\"";
            if ( box1_srcs_count ) box1_srcs = box1_srcs ","; box1_srcs = box1_srcs "\\\"" temp "\\\""; box1_srcs_count++;
            ### if ( box1_srcs_count ) box1_srcs = box1_srcs ","; box1_srcs = box1_srcs temp; box1_srcs_count++;
            if ( new_chars_count > 0 ) new_line = ",";
            new_line = new_line "\\\"" temp "\\\":{\\\"box_id\\\":" box0_count ",\\\"name\\\":\\\"" temp "\\\",\\\"enable_addr\\\":\\\"0xf0" substr($3,5,6) "\\\",\\\"box\\\":0}\\n\""; box0_count++;
            ###set_new_line(temp, box0_count, substr($3,5,6), "enable_addr", 0 );
            new_chars = new_chars "static char h_info_" new_chars_count "[]=\"" new_line ";\n";
            new_chars_count++;
        }
### BCHP_VNET_F_VFD_0_ENABLE
        else if ( index($2,"BCHP_VNET_F_VFD_") > 0 ) ### does not have reg name
        {
            enable = index($2,"_ENABLE");
            temp = tolower(substr($2,13,enable-13));
            boxes_0 = boxes_0 ",\\\"" temp "\\\"";
            if ( box1_srcs_count ) box1_srcs = box1_srcs ","; box1_srcs = box1_srcs "\\\"" temp "\\\""; box1_srcs_count++;
            ### if ( box1_srcs_count ) box1_srcs = box1_srcs ","; box1_srcs = box1_srcs temp; box1_srcs_count++;
            new_line = ",\\\"" temp "\\\":{\\\"box_id\\\":" box0_count ",\\\"name\\\":\\\"" temp "\\\",\\\"enable_addr\\\":\\\"0xf0" substr($3,5,6) "\\\",\\\"box\\\":0}\\n\""; box0_count++;
            ### set_new_line(temp, box0_count, substr($3,5,6), "enable_addr", 0 );
            new_chars = new_chars "static char h_info_" new_chars_count "[]=\"" new_line ";\n";
            new_chars_count++;
        }
### BCHP_VNET_F_DVI_0_ENABLE
        else if ( index($2,"BCHP_VNET_F_DVI_") > 0 ) ### does not have reg name + does not have addr
        {
            enable = index($2,"_ENABLE");
            temp = "hd_" tolower(substr($2,13,enable-13));
            boxes_0 = boxes_0 ",\\\"" temp "\\\"";
            if ( box1_srcs_count ) box1_srcs = box1_srcs ","; box1_srcs = box1_srcs "\\\"" temp "\\\""; box1_srcs_count++;
            ### if ( box1_srcs_count ) box1_srcs = box1_srcs ","; box1_srcs = box1_srcs temp; box1_srcs_count++;
            new_line = ",\\\"" temp "\\\":{\\\"box_id\\\":" box0_count ",\\\"name\\\":\\\"" temp "\\\",\\\"box\\\":0}\\n\""; box0_count++;
            new_chars = new_chars "static char h_info_" new_chars_count "[]=\"" new_line ";\n";
            new_chars_count++;
        }
        else if ( index($2,"BCHP_VNET_F_LPB_") > 0 )
        {
           enable = index($2,"_ENABLE");
           temp = tolower(substr($2,13,enable-13));
           ### We need to compute the number of loopbacks during the frontend processing so that when we process
           ### the backend registers, we can number them in reverse order.
           ### This number will get written to a file which will be read when we process the backend registers.
           loopbacks_num++;
           loopbacks_frontend = loopbacks_frontend ",\"enable_addr\":\"0xf0" substr($3,5,6) "\"\n";
           print temp "," substr($3,5,6) >> VNET_ENABLE_ADDRS_FILE;
        }
### BCHP_VNET_B_LOOPBACK_0_SRC
        else if ( index($2,"BCHP_VNET_B_LOOPBACK_") > 0 )
        {
            enable = index($2,"_SRC"); /* 23 */
            num = substr($2,22,enable-22);
            temp = "lpb_" num;
            ### temp = tolower( gensub(/LOOPBACK/, "lpb", substr($2,22,enable-22) ) );
            if ( length(boxes_3) ) boxes_3 = boxes_3 ",";
            boxes_3 = boxes_3 "\\\"" temp "\\\"";
            if (loopbacks_num) {
               loopbacks_num--;
            }
            new_line = ",\\\"" temp "\\\":{\\\"reg_name\\\":\\\"bchp_vnet_b_" temp "_src\\\",\\\"box_id\\\":" loopbacks_num ",\\\"name\\\":\\\"" temp "\\\",\\\"addr\\\":\\\"0xf0" substr($3,5,6) "\\\",\\\"enable_addr\\\":\\\"0xf0" "999999" "\\\",\\\"box\\\":3}\\n\""; box3_count++;
            new_chars = new_chars "static char h_info_" new_chars_count "[]=\"" new_line ";\n";
            new_chars_count++;
        }
### BCHP_VNET_B_CAP_0_SRC
        else if ( index($2,"BCHP_VNET_B_CAP_") > 0 )
        {
            enable = index($2,"_SRC");
            temp = tolower(substr($2,13,enable-13));
            if ( length(boxes_2) ) boxes_2 = boxes_2 ",";
            boxes_2 = boxes_2 "\\\"" temp "\\\"";
            ### set_new_line(temp, box2_count, substr($3,5,6), "addr", 2 );
            new_line = ",\\\"" temp "\\\":{\\\"reg_name\\\":\\\"" tolower($2) "\\\",\\\"box_id\\\":" box2_count ",\\\"name\\\":\\\"" temp "\\\",\\\"addr\\\":\\\"0xf0" substr($3,5,6) "\\\",\\\"box\\\":2}\\n\""; box2_count++;
            new_chars = new_chars "static char h_info_" new_chars_count "[]=\"" new_line ";\n";
            new_chars_count++;
        }
### BCHP_VNET_B_CMP_0_V0_SRC
        else if ( index($2,"BCHP_VNET_B_CMP_") > 0 )
        {
            enable = index($2,"_SRC");
            temp = tolower(substr($2,13,enable-13));
            boxes_2 = boxes_2 ",\\\"" temp "\\\"";
            ### set_new_line(temp, box2_count, substr($3,5,6), "addr", 2 );
            new_line = ",\\\"" temp "\\\":{\\\"reg_name\\\":\\\"" tolower($2) "\\\",\\\"box_id\\\":" box2_count ",\\\"name\\\":\\\"" temp "\\\",\\\"addr\\\":\\\"0xf0" substr($3,5,6) "\\\",\\\"box\\\":2}\\n\""; box2_count++;
            new_chars = new_chars "static char h_info_" new_chars_count "[]=\"" new_line ";\n";
            new_chars_count++;
        }
### BCHP_VNET_B_CRC_SRC
        else if ( index($2,"BCHP_VNET_B_CRC_") > 0 )
        {
            enable = index($2,"_SRC");
            if ( enable > 0 ) { ### there are other registers that have the same beginning but do not have _SRC at the end
                temp = tolower(substr($2,13,enable-13));
                boxes_2 = boxes_2 ",\\\"" temp "\\\"";
                new_line = ",\\\"" temp "\\\":{\\\"reg_name\\\":\\\"" tolower($2) "\\\",\\\"box_id\\\":" box2_count ",\\\"name\\\":\\\"" temp "\\\",\\\"addr\\\":\\\"0xf0" substr($3,5,6) "\\\",\\\"box\\\":2}\\n\""; box2_count++;
                new_chars = new_chars "static char h_info_" new_chars_count "[]=\"" new_line ";\n";
                new_chars_count++;
            }
        }
### BCHP_VNET_B_DRAIN_0_SRC
        else if ( index($2,"BCHP_VNET_B_DRAIN_") > 0 )
        {
            enable = index($2,"_SRC");
            if ( enable > 0 ) { ### there are other registers that have the same beginning but do not have _SRC at the end
                temp = tolower(substr($2,11,enable-11));
                boxes_2 = boxes_2 ",\\\"" temp "\\\"";
                new_line = ",\\\"" temp "\\\":{\\\"reg_name\\\":\\\"" tolower($2) "\\\",\\\"box_id\\\":" box2_count ",\\\"name\\\":\\\"" temp "\\\",\\\"addr\\\":\\\"0xf0" substr($3,5,6) "\\\",\\\"box\\\":2}\\n\""; box2_count++;
                new_chars = new_chars "static char h_info_" new_chars_count "[]=\"" new_line ";\n";
                new_chars_count++;
            }
        }
### BCHP_VNET_F_SCL_0_SRC
        else if ( index($2,"BCHP_VNET_F_SCL_") > 0 )
        {
            enable = index($2,"_SRC");
            temp = tolower(substr($2,13,enable-13));
            if ( length(boxes_1) ) boxes_1 = boxes_1 ",";
            boxes_1 = boxes_1 "\\\"" temp "\\\"";
            if ( box23_srcs_count ) box23_srcs = box23_srcs ","; box23_srcs = box23_srcs "\\\"" temp "\\\""; box23_srcs_count++;
            ### if ( box23_srcs_count ) box23_srcs = box23_srcs ","; box23_srcs = box23_srcs temp; box23_srcs_count++;
            new_line = ",\\\"" temp "\\\":{\\\"reg_name\\\":\\\"" tolower($2) "\\\",\\\"box_id\\\":" box1_count ",\\\"name\\\":\\\"" temp "\\\",\\\"enable_addr\\\":\\\"0xf0999999\\\",\\\"addr\\\":\\\"0xf0" substr($3,5,6) "\\\",\\\"box\\\":1}\\n\""; box1_count++;
            new_chars = new_chars "static char h_info_" new_chars_count "[]=\"" new_line ";\n";
            new_chars_count++;
        }
### BCHP_VNET_F_XSRC_0_SRC
        else if ( index($2,"BCHP_VNET_F_XSRC_") > 0 )
        {
            enable = index($2,"_SRC");
            temp = tolower(substr($2,13,enable-13));
            boxes_1 = boxes_1 ",\\\"" temp "\\\"";
            if ( box23_srcs_count ) box23_srcs = box23_srcs ","; box23_srcs = box23_srcs "\\\"" temp "\\\""; box23_srcs_count++;
            ### if ( box23_srcs_count ) box23_srcs = box23_srcs ","; box23_srcs = box23_srcs temp; box23_srcs_count++;
            new_line = ",\\\"" temp "\\\":{\\\"reg_name\\\":\\\"" tolower($2) "\\\",\\\"box_id\\\":" box1_count ",\\\"name\\\":\\\"" temp "\\\",\\\"enable_addr\\\":\\\"0xf0999999\\\",\\\"addr\\\":\\\"0xf0" substr($3,5,6) "\\\",\\\"box\\\":1}\\n\""; box1_count++;
            new_chars = new_chars "static char h_info_" new_chars_count "[]=\"" new_line ";\n";
            new_chars_count++;
        }
### BCHP_VNET_F_MVP_0_SRC
        else if ( index($2,"BCHP_VNET_F_MVP_") > 0 )
        {
            enable = index($2,"_SRC");
            temp = tolower(substr($2,13,enable-13));
            boxes_1 = boxes_1 ",\\\"" temp "\\\"";
            if ( box23_srcs_count ) box23_srcs = box23_srcs ","; box23_srcs = box23_srcs "\\\"" temp "\\\""; box23_srcs_count++;
            ### if ( box23_srcs_count ) box23_srcs = box23_srcs ","; box23_srcs = box23_srcs temp; box23_srcs_count++;
            new_line = ",\\\"" temp "\\\":{\\\"reg_name\\\":\\\"" tolower($2) "\\\",\\\"box_id\\\":" box1_count ",\\\"name\\\":\\\"" temp "\\\",\\\"enable_addr\\\":\\\"0xf0999999\\\",\\\"addr\\\":\\\"0xf0" substr($3,5,6) "\\\",\\\"box\\\":1}\\n\""; box1_count++;
            new_chars = new_chars "static char h_info_" new_chars_count "[]=\"" new_line ";\n";
            new_chars_count++;
        }
### BCHP_VNET_F_DNR_0_SRC
        else if ( index($2,"BCHP_VNET_F_DNR_") > 0 )
        {
            enable = index($2,"_SRC");
            temp = tolower(substr($2,13,enable-13));
            boxes_1 = boxes_1 ",\\\"" temp "\\\"";
            if ( box23_srcs_count ) box23_srcs = box23_srcs ","; box23_srcs = box23_srcs "\\\"" temp "\\\""; box23_srcs_count++;
            ### if ( box23_srcs_count ) box23_srcs = box23_srcs ","; box23_srcs = box23_srcs temp; box23_srcs_count++;
            new_line = ",\\\"" temp "\\\":{\\\"reg_name\\\":\\\"" tolower($2) "\\\",\\\"box_id\\\":" box1_count ",\\\"name\\\":\\\"" temp "\\\",\\\"enable_addr\\\":\\\"0xf0999999\\\",\\\"addr\\\":\\\"0xf0" substr($3,5,6) "\\\",\\\"box\\\":1}\\n\""; box1_count++;
            new_chars = new_chars "static char h_info_" new_chars_count "[]=\"" new_line ";\n";
            new_chars_count++;
        }
### BCHP_VNET_B_DNR_0_ENABLE
        else if ( index($2,"BCHP_VNET_B_DNR_") > 0 )
        {
            enable = index($2,"_ENABLE");
            temp = tolower(substr($2,13,enable-13));
            print temp "," substr($3,5,6) >> VNET_ENABLE_ADDRS_FILE;
        }
### BCHP_VNET_F_TNTD_0_SRC
        else if ( index($2,"BCHP_VNET_F_TNTD_") > 0 )
        {
            enable = index($2,"_SRC");
            temp = tolower(substr($2,13,enable-13));
            boxes_1 = boxes_1 ",\\\"" temp "\\\"";
            if ( box23_srcs_count ) box23_srcs = box23_srcs ","; box23_srcs = box23_srcs "\\\"" temp "\\\""; box23_srcs_count++;
            ### if ( box23_srcs_count ) box23_srcs = box23_srcs ","; box23_srcs = box23_srcs temp; box23_srcs_count++;
            new_line = ",\\\"" temp "\\\":{\\\"reg_name\\\":\\\"" tolower($2) "\\\",\\\"box_id\\\":" box1_count ",\\\"name\\\":\\\"" temp "\\\",\\\"enable_addr\\\":\\\"0xf0999999\\\",\\\"addr\\\":\\\"0xf0" substr($3,5,6) "\\\",\\\"box\\\":1}\\n\""; box1_count++;
            new_chars = new_chars "static char h_info_" new_chars_count "[]=\"" new_line ";\n";
            new_chars_count++;
        }
### BCHP_VNET_F_FCH_0_SRC
        else if ( index($2,"BCHP_VNET_F_FCH_") > 0 )
        {
            enable = index($2,"_SRC");
            temp = tolower(substr($2,13,enable-13));
            boxes_1 = boxes_1 ",\\\"" temp "\\\"";
            if ( box23_srcs_count ) box23_srcs = box23_srcs ","; box23_srcs = box23_srcs "\\\"" temp "\\\""; box23_srcs_count++;
            ### if ( box23_srcs_count ) box23_srcs = box23_srcs ","; box23_srcs = box23_srcs temp; box23_srcs_count++;
            new_line = ",\\\"" temp "\\\":{\\\"reg_name\\\":\\\"" tolower($2) "\\\",\\\"box_id\\\":" box1_count ",\\\"name\\\":\\\"" temp "\\\",\\\"enable_addr\\\":\\\"0xf0999999\\\",\\\"addr\\\":\\\"0xf0" substr($3,5,6) "\\\",\\\"box\\\":1}\\n\""; box1_count++;
            new_chars = new_chars "static char h_info_" new_chars_count "[]=\"" new_line ";\n";
            new_chars_count++;
        }
### BCHP_VNET_B_FDTHR_0_ENABLE
        else if ( index($2,"BCHP_VNET_B_FDTHR_") > 0 )
        {
            enable = index($2,"_ENABLE");
            temp = tolower(substr($2,19,enable-19));
            print "fch_" temp "," substr($3,5,6) >> VNET_ENABLE_ADDRS_FILE;
        }
### BCHP_VNET_B_MVP_0_ENABLE
        else if ( index($2,"BCHP_VNET_B_MVP_") > 0 )
        {
            enable = index($2,"_ENABLE");
            temp = tolower(substr($2,13,enable-13));
            print temp "," substr($3,5,6) >> VNET_ENABLE_ADDRS_FILE;
        }
### BCHP_VNET_B_SCL_0_ENABLE
        else if ( index($2,"BCHP_VNET_B_SCL_") > 0 )
        {
            enable = index($2,"_ENABLE");
            temp = tolower(substr($2,13,enable-13));
            print temp "," substr($3,5,6) >> VNET_ENABLE_ADDRS_FILE;
        }
### BCHP_VNET_B_XSRC_0_ENABLE
        else if ( index($2,"BCHP_VNET_B_XSRC_") > 0 )
        {
            enable = index($2,"_ENABLE");
            temp = tolower(substr($2,13,enable-13));
            print temp "," substr($3,5,6) >> VNET_ENABLE_ADDRS_FILE;
        }
### BCHP_VNET_B_TNTD_0_ENABLE
        else if ( index($2,"BCHP_VNET_B_TNTD_") > 0 )
        {
            enable = index($2,"_ENABLE");
            temp = tolower(substr($2,13,enable-13));
            print temp "," substr($3,5,6) >> VNET_ENABLE_ADDRS_FILE;
        }
### BCHP_VNET_F_LBOX_0_SRC
        else if ( index($2,"BCHP_VNET_F_LBOX_") > 0 )
        {
            enable = index($2,"_SRC");
            temp = tolower(substr($2,13,enable-13));
            boxes_1 = boxes_1 ",\\\"" temp "\\\"";
            new_line = ",\\\"" temp "\\\":{\\\"reg_name\\\":\\\"" tolower($2) "\\\",\\\"box_id\\\":" box1_count ",\\\"name\\\":\\\"" temp "\\\",\\\"addr\\\":\\\"0xf0" substr($3,5,6) "\\\",\\\"box\\\":1}\\n\""; box1_count++;
            new_chars = new_chars "static char h_info_" new_chars_count "[]=\"" new_line ";\n";
            new_chars_count++;
        }
### BCHP_VNET_F_DRAIN_0_SRC
        else if ( index($2,"BCHP_VNET_F_DRAIN_") > 0 )
        {
            enable = index($2,"_SRC");
            if ( enable > 0 ) { ### there are other registers that have the same beginning but do not have _SRC at the end
                temp = tolower(substr($2,11,enable-11));
                boxes_1 = boxes_1 ",\\\"" temp "\\\"";
                new_line = ",\\\"" temp "\\\":{\\\"reg_name\\\":\\\"" tolower($2) "\\\",\\\"box_id\\\":" box1_count ",\\\"name\\\":\\\"" temp "\\\",\\\"addr\\\":\\\"0xf0" substr($3,5,6) "\\\",\\\"box\\\":1}\\n\""; box1_count++;
                new_chars = new_chars "static char h_info_" new_chars_count "[]=\"" new_line ";\n";
                new_chars_count++;
            }
        }
        printf "," $2 " /* 0xf0" substr($3,5,6) " */\n";
        bvnview_addrs_add( substr($3,5,6) );
        reg_count++;
    }
    else
    {
        ### printf "#if 0\n/* unknown line " $1 " " $3 " " $4 " " $5 " " $6" */\n#endif\n";
    }
}
END {
   printf  "#define " tag_end "_MAX " reg_count "\n\n";

    if ( length(boxes_0)) {
        boxes_0 = boxes_0 ",\\\"none\\\"";
        temp = "none"

        ### the "none" will be added in brbviewer.c
        ### if ( box1_srcs_count ) box1_srcs = box1_srcs ","; box1_srcs = box1_srcs "\\\"" temp "\\\""; box1_srcs_count++;
        ### if ( box1_srcs_count ) box1_srcs = box1_srcs ","; box1_srcs = box1_srcs temp; box1_srcs_count++;

        new_line = ",\\\"" temp "\\\":{\\\"box_id\\\":" box0_count ",\\\"name\\\":\\\"" temp "\\\",\\\"box\\\":0}\\n\""; box0_count++;
        new_chars = new_chars "static char h_info_" new_chars_count "[]=\"" new_line ";\n";
        new_chars_count++;
        printf "const char javascript_boxes_0[] = \"" boxes_0 "\";\n";
    }
    if ( length(boxes_1)) {
### printf "/* boxes_1:" boxes_1 " */\n";
        printf "const char javascript_boxes_1[] = \"" boxes_1 "\";\n";
    }
    if ( length(boxes_2)) {
### printf "/* boxes_2:" boxes_2 " */\n";
        printf "const char javascript_boxes_2[] = \"" boxes_2 "\";\n";
    }
    if ( length(boxes_3)) {
        ### we need to reverse the order of the loopback names
        num = split(boxes_3,tt,",");
        ### printf "/* boxes_3:" boxes_3 "; num:" num " */\n";
        printf "const char javascript_boxes_3[] = \"";
        for(idx=0; idx<num;idx++) {
           if(idx>0) printf ",";
           idx2 = num-idx;
           printf tt[idx2];
        }
        printf("\";\n");
    }

    ### during frontend processing, save the number of loopback registers we found (number to be used during backend processing)
    if ( loopbacks_num > 0 && which_file == 0 ) {
       print loopbacks_num > "loopbacks_num.txt"; ### ends up being in obj.<platform>/nexus/brbviewer
       print loopbacks_frontend >> "loopbacks_num.txt";
    }
    if ( new_chars_count > 0 && which_file == 0 ) {
       print new_chars_count > "new_chars_count.txt"; ### ends up being in obj.<platform>/nexus/brbviewer
    }
    ### output the list of new strings that will be concatinated into one big string
    print new_chars;

    ### if we are processing the second file (backend)
    if ( which_file != 0 ) {
       printf "\n#define H_INFO_MAX " new_chars_count "\n";
       printf "char * h_info[H_INFO_MAX] = {\n";
       for (idx=0; idx<new_chars_count; idx++) {
          if (idx) {
             printf ",";
          }
          printf "h_info_" idx "\n";
       }
       printf "};";
    }
    if ( box1_srcs_count ) {
       printf "char box1_srcs[] = \"" box1_srcs "\"\n\"";
       for (idx=0; idx<loopbacks_num; idx++) {
          printf ",\\\"lpb_" idx "\\\"";
          ### printf ",lpb_" idx;
       }
       printf "\";\n";
    }
    if ( box23_srcs_count ) {
       printf "char box23_srcs[] = \"" box23_srcs "\";\n";
    }
    print "\n\nchar bvnview_addrs_1_" front_or_back "[] = \"" bvnview_addrs_1 "\";\n";
    print "\n\nchar bvnview_addrs_2_" front_or_back "[] = \"" bvnview_addrs_2 "\";\n";
    print "\n\nchar bvnview_addrs_3_" front_or_back "[] = \"" bvnview_addrs_3 "\";\n";
}

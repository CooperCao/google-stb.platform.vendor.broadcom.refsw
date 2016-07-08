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
### This script is used to parse the temporary file brbviewer_regs_autogen.tmp. When brbviewer_regs_autogen.tmp
### is created, it creates some spacers in the file using the tag 999999. This number needs to be replaced with
### the accurate addresses of the particular registers. This script looks for all lines that have the 999999
### tag on it, and matches the element name with one in the VNET_ENABLE_ADDRS_FILE file. This second file has
### been previously created by the brbview_pass1.awk scrpit to include all elements and addresses that have the
### "enable_addr" field in it.
BEGIN {
   VNET_ENABLE_ADDRS_FILE = "vnet_enable_addrs.txt"
}
function read_vnet_f_enable_addrs_file() {
    num_elements = 0;
    num_lines = 0;
    ### print "/* top ... num_elements (" num_elements ")*/";
    while (getline name_and_enable_addr < VNET_ENABLE_ADDRS_FILE ) {
       num_lines++;
       ### print "got line (" name_and_enable_addr ")";
       if ( length(name_and_enable_addr) ) {
            num=split(name_and_enable_addr,parts,",");
            if (num) {
               enable_addr[parts[1]] = parts[2];
               ### print "/* got pair (" parts[1] ") and (" parts[2] ")*/";
               num_elements++;
            }
       } else if (num_lines > 1) {
          ### print "/* break (" num_elements ")*/";
          break;
       }
    }
    ### print "/* num_elements (" num_elements ")*/";
}
{
    line++;
    if (line == 1) {
       read_vnet_f_enable_addrs_file();
    }
    ### look for lines like this:
    ### static char h_info_15[]=",\"scl_0\":{\"reg_name\":\"bchp_vnet_f_scl_0_src\",\"box_id\":0,\"name\":\"scl_0\",\"enable_addr\":\"0xf0999999\",\"addr\":\"0xf06220a0\",\"box\":1}\n";
    temp = index($3,"0xf0999999");
    if (temp) {
       temp = index($3,"=");
       if (temp) {
          temp+=5; ### advance past the equals sign, comma, backslash, and quote
          partial = substr($3,temp);
          temp = index(partial,"\\" ); ### find the end of the element's name
          ### print "/* temp1:" temp ";   partial 1 is (" partial ")*/";
          if (temp) {
             name = substr(partial,0,temp-1);
             ### print "/* name2 (" name") */";
             addr = enable_addr[name];
             if (length(addr)) {
                ### print "/* name3 (" name ") has addr (" addr ") */";
                temp = index($3,"enable_addr" ); ### find the element that has the address we need to fix
                if (temp) {
                   printf $1 " " $2 " " substr($3,0,temp-1); ### output the first part of the line up to the beginning of enable_addr field
                   partial = substr($3,temp);
                   temp = index(partial,",");
                   printf "enable_addr\\\":\\\"0xf0" addr "\\\"" substr(partial,temp) "\n";
                } else { ### something went wrong. output the entire line
                   print "error1:" $0;
                }
             } else { ### something went wrong. output the entire line
                print "error2:" $0;
             }
          } else { ### something went wrong. output the entire line
              print "error3:" $0;
          }
       }
    } else {
       temp = index($3,"0xf0888888");
       if (temp) {
          temp = index($3,"=");
          if (temp) {
             temp+=5; ### advance past the equals sign, comma, backslash, and quote
             partial = substr($3,temp);
             temp = index(partial,"\\" ); ### find the end of the element's name
             ### print "/* temp:" temp ";   partial 1 is (" partial ")*/";
             if (temp) {
                name = substr(partial,0,temp-1);
                ### print "/* name (" name") */";
                addr = enable_addr[name];
                if (length(addr)) {
                   ### print "/* name (" name ") has addr (" addr ") */";
                   temp = index($3,"\"addr" ); ### find the element that has the address we need to fix
                   if (temp) {
                      printf $1 " " $2 " " substr($3,0,temp); ### output the first part of the line up to the beginning of enable_addr field
                      partial = substr($3,temp);
                      temp = index(partial,",");
                      printf "addr\\\":\\\"0xf0" addr "\\\"" substr(partial,temp) "\n";
                   } else { ### something went wrong. output the entire line
                      print "error1:" $0;
                   }
                } else { ### something went wrong. output the entire line
                   print "error2:" $0;
                }
             } else { ### something went wrong. output the entire line
                 print "error3:" $0;
             }
          }
       } else {
          print $0;
       }
    }
}
END {
}

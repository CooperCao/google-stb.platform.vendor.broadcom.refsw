#!/bin/sh
#
# Script to strip out unwanted registers from either driver
# or epidiag register dumps.  The list of unwanted regs
# is kept in src/doc/phyregdiff.txt.  This script uses that
# file. 
# 
# Usage: cat dumpfile | stripunwantedregs 
#
cat - | awk --non-decimal-data '
{
   # Read in dumpfile into associatve array
       if ($1 == "bphy:"){
	       bphy[strtonum($2)] = $3;
       } else if ($1 == "aphy:"){
	       aphy[strtonum($2)] = $3;
	       # For aphy: sense if regs are 0x400 based or 0x0 based.
	       # G band => 0x400, A band => 0 based. 
	       if ( strtonum($2) > 1024)
	           aphy_offset = 1024;
       } else if ($1 == "gphy:")
	       gphy[strtonum($2)] = $3;
       else{
            # Assume there are less than 30 tables
       	    if (strtonum($1) < 30)
	       table[strtonum($1),strtonum($2)] = $3 " " $4
	    else 
	        # Let unknown stuff pass through
	    	print $0
       }
}

END { 
   # Read in filter and overwrite any matching entries in array
   while (getline  < "/projects/hnd/software/gallery/src/doc/phyregdiff.txt" > 0 ) {
       if ($1 == "bphy:"){
		if ($2 == "range:") {
			for (mindex = strtonum($3); mindex <= strtonum($4); mindex++){
			       bphy[mindex] = ""
			}
		} else {
		       bphy[strtonum($2)] = ""
		}
	} else if ($1 == "aphy:"){
		if ($2 == "range:") {
			for (mindex = strtonum($3) + aphy_offset; mindex <= strtonum($4) + aphy_offset; mindex++){
			       aphy[mindex] = ""
			}
		} else {
		       aphy[strtonum($2) + aphy_offset] = ""
		}
	} else if ($1 == "gphy:"){
		if ($2 == "range:") {
			for (mindex = strtonum($3); mindex <= strtonum($4); mindex++)
			       gphy[mindex] = ""
		} else {
		       gphy[strtonum($2)] = ""
		}
	} else if ($1 == "table:"){
		   for (reg = 0; reg < 1024; reg++)
		     table[strtonum($2), reg] = "" 
	} else {
		# print "Unknown directive:  " $0
	}
   }

    # Dump results from array
   for (reg = 0; reg < 1024; reg++)
     if (bphy[reg] != "") printf "bphy: 0x%3.3x %s\n", reg, bphy[reg]
   for (reg = 0 + aphy_offset; reg < 1024 + aphy_offset; reg++)
     if (aphy[reg] != "") printf "aphy: 0x%3.3x %s\n", reg, aphy[reg]
   for (reg = 2048; reg < 3072; reg++)
     if (gphy[reg] != "") printf "gphy: 0x%3.3x %s\n", reg, gphy[reg]
   for (tbl = 0; tbl < 30; tbl++)
	   for (reg = 0; reg < 1024; reg++)
	     if (table[tbl, reg] != "") 
	          printf "0x%x: 0x%3.3x %s\n", tbl, reg, table[tbl, reg]
}
'

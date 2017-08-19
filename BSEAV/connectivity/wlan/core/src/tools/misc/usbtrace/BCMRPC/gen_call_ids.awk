#
#  Broadcom USB RPC - Script support
#
#  $ Copyright Broadcom Corporation $
#
#  $Id: gen_call_ids.awk,v 1.1 2010-05-28 22:02:15 stafford Exp $
#

# Description:
#
# Generate RPC call IDs for the USB CATC Scripting Language add on scripts from
# the rpc header source file. The USB Decoder scripts in this directory use the
# BCMRPC_Call_IDs.inc file to define RPC call IDs in the UBS packets.  As the
# driver changes the RPC calls, this .inc file will need to be updated.  This
# utility awk file will parse wlc_rpc.h to produce and updated
# BCMRPC_Call_IDs.inc file. This script works will wlc_rpc.h v1.117 and earlier
# but may fail to parse if the file changes the lines this script uses to find
# the constants.
#
# usage: awk -f gen_call_ids.awk < src/wl/sys/wlc_rpc.h > new_ids.inc
#
# wlc_rpc.h is expected to have an enum def like this:
#
# typedef enum {
#	WLRPC_NULL_ID = 0,
#	...
#	WLRPC_LAST
# } wlc_rpc_id_t;
#
# This script looks for the list bounded by "WLRPC_NULL_ID" and "wlc_rpc_id_t" lines,
# and pulls out each constant matching WLRPC_*
#
# If there are errors in parsing, "MISMATCH:" lines will be printed
# at the head of the file
#
# The resulting output can replace contents of the file "BCMRPC_Call_IDs.inc"
#

BEGIN { 
    gIDCount = 0
}

# For each line in the RPC ID enumeration ...
/WLRPC_NULL_ID/, /wlc_rpc_id_t/ {

    # do not include WLRPC_LAST if present
    if ( /WLRPC_LAST/ ) next

    # do not process the end of the block
    if ( /wlc_rpc_id_t/ ) next

    # All identifiers start with "WLRPC_"
    # print an error if it does not seem like we are
    # in the enum block
    if ( ! /^[ \t]*WLRPC_/ ) {
	print "MISMATCH: " $0
    }

    # isolate the identifier starting with "RPC_"
    match( $0, /RPC_[_A-Za-z0-9]+/ )

    gFnNames[gIDCount++] = substr($0, RSTART, RLENGTH)
}

END {
    # print out the file contents for USB Trace script
    print_header()

    dump_constants()

    print ""
    print ""

    dump_name_array()
}

function print_header () {
    print "#"
    print "#  Broadcom USB RPC - RPC call IDs and string names"
    print "#"
    print "#  $ Copyright Broadcom Corporation $"
    print "#"
    print "#  $Id: gen_call_ids.awk,v 1.1 2010-05-28 22:02:15 stafford Exp $"
    print "#"
    print ""
}

#    const RPC_NULL_ID	= 0;
function dump_constants (	i, name) {

    for (i = 0; i < gIDCount; i++) {
	name = gFnNames[i]
	printf( "const %-24s\t= %d;\n", name, i )
    }

}

function dump_name_array (	i, name) {

    print "set RPC_Function_IDs ="
    print "["

    for (i = 0; i < gIDCount; i++) {
	name = format_fn( gFnNames[i] )
	# print the array line, with a comma on all but 
	# the last line
	printf( "\t[ %d,\t\"%s\"\t]%s\n", i, name, (i < (gIDCount - 1)) ? "," : "")
    }

    print "];"
}

# Given an RPC ID such as "RPC_NULL_ID", create the user friendly
# name like "NULL()"
function format_fn ( fn_id ) {

    sub( /^RPC_/, "", fn_id )
    sub( /_ID$/, "", fn_id )

    return fn_id "()"
}

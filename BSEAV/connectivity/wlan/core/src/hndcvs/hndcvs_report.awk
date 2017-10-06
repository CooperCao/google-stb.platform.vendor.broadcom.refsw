#!/usr/bin/awk -f

# From Phillipe 2008.04.17
# 
BEGIN {print_on=0; save_text=0; text="";}

#begining of html portion. Print till first table header
/xml version/ {
    print_on=1;
}


#beging of header, save till we can decide if we want to print it
/class="csetheader"/ {
    print_on=0;
    if (save_module == 1) {
	# yep, this needs to be printed.
	if (saved_modules == 1) {
	    print table_text;
	    print comment_text;
	    print saved_modules_text;
	    print module_text;
	}
	
    }
    save_table =1;
    save_module = 0;
    table_text = "";
    comment_text = "";
    saved_modules_text= "";
    module_text="";
    saved_modules=0;
}

{ 
    if (print_on ==1) {
	print $0;
	next;
    }
}


/<\/table>/ {
    if(save_table==1) { 
	save_table=0;
	save_comment=1;
    }
}

/<\/tr>/ {
    if(save_module == 1) {
	module_text = module_text "\n" $0;
	if (match_found == 1) {
	    saved_modules_text = saved_modules_text "\n" module_text;
	    saved_modules=1;
	}
	match_found=0;
	module_text="";
    }
    if(save_comment==1) { 
	comment_text = comment_text "\n" $0;
	save_comment=0;
	save_module=1;
    }
    next;
}


/found in/{
    match_file = gensub(/(file )([[:alnum:]_/.-]+)( found in )([[:alnum:]_/]+)( with TAG )([[:alnum:]_]+)(.*)/,"\\2...\\6",1);
    match_module = gensub(/(file )([[:alnum:]_/.-]+)( found in )([[:alnum:]_/-]+)( with TAG )([[:alnum:]_]+)(.*)/,"\\4",1);
    match_files = match_file"..."match_module " " match_files;
}

/on branch/{
    if (save_table==1) table_text = table_text "\n" $0;
    branch = gensub(/(.*)(Commit from)(.*)(class=\"branch\">)([[:alnum:]_]+)(.*)/,"\\5",1);
    next;
}

/Commit from/{
    if (save_table==1) table_text = table_text "\n" $0;
    branch="TOT";
    next;
}

# TODO replace src with the module name
/>src</ {
    if(save_module == 1) 
	module_text = module_text "\n" $0;
    src_found=1;
    next;
}

/item/{
    if (src_found ==1) {
	file = gensub(/(.*)(>)([[:alnum:]_/.-]+)(.*)/,"\\3",1);
	file ="src/"file ;
	files = file"..."branch " " files;
	# ok, try to find this in the match_files list
	# first, escape / and . 
	gsub(/\//,"\\/",file);
	gsub(/\./,"\\.",file);
	regexp = sprintf("%s\\.\\.\\.%s\\.\\.\\.([[:alnum:]_/]+)",file,branch);
#	print regexp;
	res = match(match_files, regexp, match_res);
	match_found=0;
	if (res !=0) {
#	    print "file " file " with tag " branch " modified in module " match_res[1];
	    match_found=1;
	}
	src_found=0;	    
    }
}

/<\/body>/ {
    print_on=1;
}

{
    if (print_on==1) print; 
    if(save_table==1) 
	table_text = table_text "\n" $0;
    if(save_comment==1)
	comment_text = comment_text "\n" $0;
    if(save_module == 1)
	module_text = module_text "\n" $0;
}

END {
 #   print files;
# print match_files;
}

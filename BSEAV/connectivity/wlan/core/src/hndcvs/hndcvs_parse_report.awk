#!/usr/bin/awk -f

# From Phillipe 2008.04.17
# 
BEGIN {}

/on branch/{
    branch = gensub(/(.*)(Commit from)(.*)(class=\"branch\">)([[:alnum:]_]+)(.*)/,"\\5",1);
    next;
}

/Commit from/{
    branch="TOT";
    next;
}

/>src</ {
    src_found=1;
    next;
}

/item/{
    if (src_found ==1) {
	file = gensub(/(.*)(>)([[:alnum:]_/.-]+)(.*)/,"\\3",1);
	file ="src/"file ;
	files = file"..."branch " " files;
	src_found=0;	    
    }
}

END {
    print files;
}

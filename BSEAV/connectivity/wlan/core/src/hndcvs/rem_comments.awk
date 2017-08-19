#!/usr/bin/awk -f

BEGIN {
    open_comm=0;
}

/#endif/ {
    if (open_comm != 1)
	print "#endif" ;
    next;
}

/#else/ {
        if (open_comm != 1)
	    print;
}

# remove c comment, filter doesn't like these 
/\/\*/  {
    open_comm=1

}

/\*\// {
    open_comm=0
    next;
}



/#if/ {
        if (open_comm != 1)
	    print;
}


# remove comments, gcc doesn't like those
/#/ {
    next;
}

{
    if (open_comm != 1)
	print;

}

END {

}


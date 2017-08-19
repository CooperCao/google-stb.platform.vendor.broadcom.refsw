#!/usr/bin/awk -f

BEGIN {tags=""; base_dir="/projects/hnd_software/gallery/BOMS/"}

/hndcvs_cvsreport\./{
    tag = gensub(/(.*).hndcvs_cvsreport.([[:alnum:]_/-]+).([[:alnum:]_]+).([[:alnum:]_]+).html/,"\\3:\\2",1);
    date = gensub(/(.*).hndcvs_cvsreport.([[:alnum:]_/-]+).([[:alnum:]_]+).([[:alnum:]_]+).html/,"\\4",1);
    str=" &nbsp;<a href=\"" base_dir $0"\">"tag"</a>&nbsp; ";
    tags = tags str ;
    next;
}

$0 ~ date {
    str = gensub(/(.*)(<\/tr>)/,"\\1",1);
    printf str "<td>" tags "</td></tr>\n";
    next;
}
/Branches/ {
    str = gensub(/(.*)(Tags<\/th>)(<\/tr>)/,"\\1\\2<th>BOMs</th>\\3",1);
    print str;
    next;
}

{ print; }

END {
}

#!/usr/bin/awk -f

# From Phillipe 2008.04.17
# 
# ... run a diff like this :
# 
# cvs -Q rdiff -s -r PBR_REL_5_10_7 -r PBR_BRANCH_5_10 linux-router > diff.txt
# 
# It takes a couple minutes.
# 
# Then :
# 
# awk -f hndcvs_diff.awk diff.txt > diff.html
# 
# and load the page [in a browser].

BEGIN{
    print "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"; 
    print "<!DOCTYPE html"; 
    print " PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"";
    print " \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">";
    print "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">";
    print "<head>";
    print "<title> cvs -Q rdiff -r -r ... to be improved ...  </title>";
    print "<style type=\"text/css\">";
    print "  table.csetheader { color: inherit; background: #ebebeb; border-style: solid; border-color: #8d8d8d; border-width: 0px 1px 1px 0px; }";
    print " table.history { border: solid black 1px; }";
    print " table.diff { border: solid black 1px; }";
    print "       body       { color: black; background: white; }";
    print "       p.footer   { border-top: solid black 1px; }";
    print "       em.user    { font-style: normal; color: #000080; background: inherit; }";
    print "       em.branch  { font-style: normal; color: #800000; background: inherit; }";
    print "       b.diffA    { font-weight: normal; color: inherit; background: #e0f4e0; }";
    print "       b.diffR    { font-weight: normal; color: inherit; background: #f4e0e0; }";
    print "      tr.history { color: inherit; background-color: #c0c0c0; }";
    print "      th.history { padding-right: 3em; text-align: left; }";
    print "      th.haction { padding-right: 1em; text-align: left; }";
    print "      td.item    { padding-right: 3em; font-family: monospace; }";
    print "      td.itemact { padding: 0em 0.5em; font-family: monospace; text-align: center; }";
    print "      td.itemrev { padding-right: 1em; font-family: monospace; }";
    print "      tr.itemA0  { color: inherit; background: #e0f4e0; margin: 15px; }";
    print "      tr.itemA1  { color: inherit; background: #f0fff0; margin: 15px; }";
    print "      tr.itemM0  { color: inherit; background: #eaeaea; margin: 15px; }";
    print "      tr.itemM1  { color: inherit; background: #f4f4f4; margin: 15px; }";
    print "      tr.itemR0  { color: inherit; background: #f4e0e0; margin: 15px; }";
    print "      tr.itemR1  { color: inherit; background: #fff0f0; margin: 15px; }";
    print "      tr.diffhd  { color: inherit; background: #c0c0c0; }";
    print "      th.diffhd  { padding-left: 2em; text-align: left; font-family: monospace; }";
    print "      tr.diffblk { color: inherit; background: #ffffff; }";
    print "      td.diffblk { font-weight: bold; padding-top: 0.5em; padding-left: 1.5em; border-bottom: solid black 1px; }";
    print "      <table cellspacing=\"0\" cellpadding=\"3\" class=\"history\">";	 
    print "      td.diffA   { color: inherit; background: #f0fff0; font-family: monospace; }";
    print "      td.diffM   { color: inherit; background: #f4f4f4; font-family: monospace; }";
    print "      td.diffR   { color: inherit; background: #fff0f0; font-family: monospace; }";	
    print "   </style>";
    print " </head>";
    print "<body>";
    print "   <table width=\"100%\" class=\"csetheader\">";
    print "     <tr>";
    print "        <td align=\"left\"><b> Diff of "DIFF_BRAND" between "DIFF_TAG1" and "DIFF_TAG2" </b></td>";
    print "        <td align=\"right\">2008-04-17 00:24 PDT</td>";
    print "      </tr>";
    print "    </table>";
    print "    <table cellspacing=\"0\" cellpadding=\"3\" class=\"history\">";

#    print "<tr class=\"history\">";
#    print "<th class=\"haction\"></th>";
#    print "<th class=\"history\">Module</th>";
#    print "<th class=\"history\">Tag1</th>";
#    print "<th class=\"history\">Tag2</th>";
#    print "<th class=\"history\">File name</th>";
#    print "<th class=\"history\" colspan=\"3\">Revision</th>"

    h1="<td class=\"itemrev\"><a href=\"http://home.sj.broadcom.com/cgi-bin/viewcvs/cgi/viewcvs.cgi/"
    h2="?rev="
    h3="&amp;content-type=text/vnd.viewcvs-markup\">"
    h4="</a></td>"
    h5="&amp;r2="
    h6="?only_with_tag="
    h7="<a href=\"http://home.sj.broadcom.com/cgi-bin/viewcvs/cgi/viewcvs.cgi/"
}

/Using BRANCH HEAD/{
    branch = gensub(/(Using BRANCH[ ]+)([[:alnum:]_]+)/,"MAIN",1);
    print "    </table>";
    print "   <table width=\"100%\" class=\"csetheader\">";
    print "     <tr>";
    print "        <td align=\"left\"><b>"module" between "tag" and "branch" </b></td>";
    print "        <td align=\"right\">2008-04-17 00:24 PDT</td>";
    print "      </tr>";
    print "    </table>";
    print "    <table cellspacing=\"0\" cellpadding=\"3\" class=\"history\">";
    print "<tr class=\"history\">";
    print "<th class=\"haction\"></th>";
    print "<th class=\"history\">Module</th>";
    print "<th class=\"history\">Tag1</th>";
    print "<th class=\"history\">Tag2</th>";
    print "<th class=\"history\">File name</th>";
    print "<th class=\"history\" colspan=\"3\">Revision</th>"
    next;
}
/Using BRANCH/{
    branch = gensub(/(Using BRANCH[ ]+)([[:alnum:]_]+)/,"\\2",1);
    print "    </table>";
    print "<br>"
    print "   <table width=\"100%\" class=\"csetheader\">";
    print "     <tr>";
    print "        <td align=\"left\"><b>"module" between "tag" and "branch" </b></td>";
    print "      </tr>";
    print "    </table>";
    print "<br>"
    print "    <table cellspacing=\"0\" cellpadding=\"3\" class=\"history\">";
    print "<tr class=\"history\">";
    print "<th class=\"haction\"></th>";
    print "<th class=\"history\">Module</th>";
    print "<th class=\"history\">Tag1</th>";
    print "<th class=\"history\">Tag2</th>";
    print "<th class=\"history\">File name</th>";
    print "<th class=\"history\" colspan=\"3\">Revision</th>"
}

/Using TAG/{
    tag = gensub(/(Using TAG[ ]+)([[:alnum:]_]+)/,"\\2",1);
}

/Diffing MODULE/{
    module = gensub(/(Diffing MODULE[ ]+)([[:alnum:]./_\-]+)/,"\\2",1);
    
}

/is new/{
f = gensub(/(File[ ]+)([[:alnum:]./_\-]+)([ ]+is new;[ ]+[[:alnum:]_]+[ ]+revision[ ]+)(.*)/, "\\2", 1);
v2 = gensub(/(File[ ]+)([[:alnum:]./_\-]+)([ ]+is new;[ ]+[[:alnum:]_]+[ ]+revision[ ]+)(.*)/, "\\4", 1);
  print "</tr>";
  print "<tr class=\"itemM0\">";
  print "<td class=\"itemact\"></td>";
  print "<td class=\"item\"><b>" module  "</b></td>";
  print "<td class=\"item\"><b>" tag  "</b></td>";
  print "<td class=\"item\"><b>" branch "</b></td>";
  print "<td class=\"item\">" h7 f h6 branch "\">" f "</a>";
  print "<td class=\"item\"><b>"  "new file"  "</b></td>";
  print "<td class=\"item\"><b>"  "vers.  "  "</b></td>";
  print h1 f h2 v2 h3 v2 h4;
  print "</tr>"
}

/is removed/{
f = gensub(/(File[ ]+)([[:alnum:]./_\-]+)([ ]+is removed;[ ]+[[:alnum:]_]+[ ]+revision[ ]+)(.*)/, "\\2", 1);
v2 = gensub(/(File[ ]+)([[:alnum:]./_\-]+)([ ]+is removed;[ ]+[[:alnum:]_]+[ ]+revision[ ]+)(.*)/, "\\4", 1);
  print "</tr>";
  print "<tr class=\"itemM0\">";
  print "<td class=\"itemact\"></td>";
  print "<td class=\"item\"><b>" module  "</b></td>";
  print "<td class=\"item\"><b>" tag "</b></td>";
  print "<td class=\"item\"><b>" branch  "</b></td>";
  print "<td class=\"item\">" h7 f h6 branch "\">" f "</a>";
  print "<td class=\"item\"><b>"  "removed file"  "</b></td>";
  print "<td class=\"item\"><b>"  "vers.  "  "</b></td>";
  print h1 f h2 v2 h3 v2 h4;
  print "</tr>"
}

/changed from/ {
    f = gensub(/(File[ ]+)([[:alnum:]./_\-]+)([ ]+changed from revision[ ]+)([0-9.]+)([ ]+to[ ]+)(.*)/, "\\2", 1);
    v1 = gensub(/(File[ ]+)([[:alnum:]./_\-]+)([ ]+changed from revision[ ]+)([0-9.]+)([ ]+to[ ]+)(.*)/, "\\4", 1);
    v2 = gensub(/(File[ ]+)([[:alnum:]./_\-]+)([ ]+changed from revision[ ]+)([0-9.]+)([ ]+to[ ]+)(.*)/, "\\6", 1);
    print "</tr>";
    print "<tr class=\"itemM0\">";
    print "<td class=\"itemact\"></td>";
    print "<td class=\"item\"><b>" module "</b></td>";
    print "<td class=\"item\"><b>" tag "</b></td>";
    print "<td class=\"item\"><b>" branch "</b></td>";
    print "<td class=\"item\">" h7 f h6 branch "\">" f "</a>";
    print h1 f  h2 v1 h3 v1 h4;
    print h1 f "?r1=" v1 h5 v2 "\">>>>" h4;
    print h1 f h2 v2 h3 v2 h4;
    print "</tr>"
}

END {
    print "    </table>";
    print " </body>";
    print "</html>";
}

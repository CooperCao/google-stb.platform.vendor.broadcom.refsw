#!/usr/bin/env perl
# /usr/local/bin/perl contextgen.pl -s KIRIN_REL_5_100_82_1 -e KIRIN_REL_5_100_82_4 -o KIRIN_BRANCH_5_100 -r randfromcgi
# /usr/local/bin/perl contextgen.pl -s PHYDEV_REL_5_60_5_2 -e BASS_REL_5_60_8 -o wl-src-bom -r randfromcgi

use Getopt::Std;
use Net::Gnats;
use File::Temp qw/ tempfile tempdir /;
use File::Path qw(make_path remove_tree);
$| = 1;

getopt( 's:e:o:r:');
die "Usage: $0 -s Start Tag -e End Tag [-o Optional Bom Name] [-r randfromcgi] \n"
     unless ($opt_s and $opt_e);
  my $tag=$opt_s;
  my $branch=$opt_e;
  my $branchOrbom=$opt_o;

  #There were issues with setting up cvspass and NIS user, so coding it for now.
  my $mycvsroot=":pserver:wlanweb:Brcm123!:\@cvsps-sj1-1.sj.broadcom.com:/projects/cvsroot";
  my $bomed_list="bomed_list.txt";
  my $src_bomed_list="src/hndcvs/$bomed_list";
  my @bomName;

  my $dir = File::Temp->newdir();
  my $template=tempXXXXXXXXXX;
  $dir = File::Temp->newdir( $template );
  $tempDirname = $dir->dirname;

chdir($dir);
unlink($src_bomed_list);
system("cvs -d $mycvsroot co -A $src_bomed_list");
open (BLIST, $src_bomed_list) or die "Can't read $src_bomed_list: $!";
  my $stringlist=join("",<BLIST>);
if (! $branchOrbom){
  my @partbranchOrbom=split('_',$tag);
  $partbranchOrbom=@partbranchOrbom[0];
  if ($stringlist =~ /^$partbranchOrbom(.+?)="\n(.+?)"$/sm){
    $branchOrbom=$partbranchOrbom.$1;
  }
}
if ($stringlist =~ /^$branchOrbom="\n(.+?)"$/sm){
  @bomName=split("\n",$1);
} else {
  #User input bomName;
  push(@bomName,$branchOrbom);
}

  my $PASS_PERL=$ENV{'PASS_PERL'};
if ($PASS_PERL) {
  system("echo $branchOrbom > /tmp/$PASS_PERL");
}

  my $tb=$tag."___".$branch;
  my $btb=$tb."___".$branchOrbom;
  my $logbtb="log___$btb";
  #my $fileDrop="/projects/hnd/swbuild/build_admin/notes/releasenotes/samples";
  my $fileDrop=".";
  my $outFile="$fileDrop/$logbtb.html";
  my $csvoutFile="$fileDrop/$logbtb.csv";
  my $xmloutFile="$fileDrop/$logbtb.xml";
  my $exlTmpltName="jjv";

  my $viewCvsBase="http://home.sj.broadcom.com/cgi-bin/viewcvs/cgi/viewcvs.cgi/";
  my $viewCvsMarkup="&amp;content-type=text/vnd.viewcvs-markup\">";
  my $gnatsUrl="http://gnatsweb.broadcom.com/cgi-bin/gnatsweb.pl?database=HND_WLAN";
  my $gnatsPrUrl="<a href=\"$gnatsUrl;cmd=view+audit-trail;pr=$2\">$1</a>";

  my $gnatsDb = Net::Gnats->new("gnatsweb.broadcom.com","1530");
  $gnatsDb->connect();
  my @dbNames = $gnatsDb->getDBNames();
  $gnatsDb->login("hnd_wlan","jvarghes","hndwlan");


#CODE FROM CVSREPORT, so not sure about copyrights
#Return previous revision from a revision (revise that)
#
#BEGIN THIRD PARTY CODE
sub get_previous_rev {
    my @rev = split /\./, shift;

    # Even number count : branch, increment is 2
    # Odd number count : regular revision, increment is 1
    my $step = (@rev & 1) ? 2 : 1;
    $rev[-1] -= $step if $rev[-1] >= $step;

    # If we reach the branch, prune it.
    if (@rev > 2 && $rev[-1] == 0) {
        $#rev -=2;
    }
    return join '.', @rev;
}
#END THIRD PARTY CODE

sub csvprint {
  unlink ($csvoutFile);
  open (CSVOUTFILE, ">>$csvoutFile") or die "Can't write to $csvoutFile: $!";
  print CSVOUTFILE "BomName,Module,Tag,Branch,Date,Time,Author,PR_Number,Customer,Release Notes,Comments,File_name,Subdirectory\n";
}
csvprint();

if (! -d "../archivedir") {mkdir ("../archivedir")};

sub htmlprint {
  unlink ($outFile);
  open (OUTFILE, ">>$outFile") or die "Can't write to $outFile: $!";
      print OUTFILE "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
      print OUTFILE "<!DOCTYPE html";
      print OUTFILE " PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"";
      print OUTFILE " \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">";
      print OUTFILE "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">";
      print OUTFILE "<head>";
      print OUTFILE "<title> Checkin comments for $module between $tag and $branch  </title>";
      print OUTFILE "<script src=\"sorttable.js\"></script>";
      print OUTFILE "<style type=\"text/css\">";
      print OUTFILE "  table.csetheader { color: inherit; background: #ebebeb; border-style: solid; border-color: #8d8d8d; border-width: 0px 1px 1px 0px; }";
      print OUTFILE " table.history { border: solid black 1px; }";
      print OUTFILE " table.diff { border: solid black 1px; }";
      print OUTFILE "       body       { color: black; background: white; }";
      print OUTFILE "       p.footer   { border-top: solid black 1px; }";
      print OUTFILE "       em.user    { font-style: normal; color: #000080; background: inherit; }";
      print OUTFILE "       em.branch  { font-style: normal; color: #800000; background: inherit; }";
      print OUTFILE "       b.diffA    { font-weight: normal; color: inherit; background: #e0f4e0; }";
      print OUTFILE "       b.diffR    { font-weight: normal; color: inherit; background: #f4e0e0; }";
      print OUTFILE "      tr.history { color: inherit; background-color: #c0c0c0; }";
      print OUTFILE "      th.history { padding-right: 3em; text-align: left; }";
      print OUTFILE "      th.haction { padding-right: 1em; text-align: left; }";
      print OUTFILE "      td.item    { padding-right: 3em; font-family: monospace; }";
      print OUTFILE "      td.itemact { padding: 0em 0.5em; font-family: monospace; text-align: center; }";
      print OUTFILE "      td.itemrev { padding-right: 1em; font-family: monospace; }";
      print OUTFILE "      tr.itemA0  { color: inherit; background: #e0f4e0; margin: 15px; }";
      print OUTFILE "      tr.itemA1  { color: inherit; background: #f0fff0; margin: 15px; }";
      print OUTFILE "      tr.itemM0  { color: inherit; background: #eaeaea; margin: 15px; }";
      print OUTFILE "      tr.itemM1  { color: inherit; background: #f4f4f4; margin: 15px; }";
      print OUTFILE "      tr.itemR0  { color: inherit; background: #f4e0e0; margin: 15px; }";
      print OUTFILE "      tr.itemR1  { color: inherit; background: #fff0f0; margin: 15px; }";
      print OUTFILE "      tr.diffhd  { color: inherit; background: #c0c0c0; }";
      print OUTFILE "      th.diffhd  { padding-left: 2em; text-align: left; font-family: monospace; }";
      print OUTFILE "      tr.diffblk { color: inherit; background: #ffffff; }";
      print OUTFILE "      td.diffblk { font-weight: bold; padding-top: 0.5em; padding-left: 1.5em; border-bottom: solid black 1px; }";
      print OUTFILE "      <table cellspacing=\"0\" cellpadding=\"3\" class=\"history\">";
      print OUTFILE "      td.diffA   { color: inherit; background: #f0fff0; font-family: monospace; }";
      print OUTFILE "      td.diffM   { color: inherit; background: #f4f4f4; font-family: monospace; }";
      print OUTFILE "      td.diffR   { color: inherit; background: #fff0f0; font-family: monospace; }";
      print OUTFILE "   </style>";
      print OUTFILE " </head>";
      print OUTFILE "<body>";
      print OUTFILE "   <table width=\"100%\" class=\"csetheader\">";
      print OUTFILE "     <tr>";
  
      print OUTFILE "    </table>";
      print OUTFILE "   <table width=\"100%\" class=\"csetheader\">";
      print OUTFILE "     <tr>";
      ####print OUTFILE "        <td align=\"left\"><b>$bomName between $tag and $branch </b></td>";
      ####print OUTFILE "        <td align=\"right\">2008-04-17 00:24 PDT</td>";
      print OUTFILE "      </tr>";
      print OUTFILE "    </table>";
      print OUTFILE "    <table cellspacing=\"0\" cellpadding=\"3\" class=\"history\">";
      #print OUTFILE "    <table cellspacing=\"0\" cellpadding=\"3\" class=\"sortable\">";
      print OUTFILE "<tr class=\"history\">";
      print OUTFILE "<th class=\"haction\"></th>";
      print OUTFILE "<th class=\"history\">BOM</th>";
      print OUTFILE "<th class=\"history\">Module</th>";
      print OUTFILE "<th class=\"history\">Tag</th>";
      print OUTFILE "<th class=\"history\">Branch</th>";
      print OUTFILE "<th class=\"history\">Date</th>";
      print OUTFILE "<th class=\"history\">Time</th>";
      print OUTFILE "<th class=\"history\">Author</th>";
      print OUTFILE "<th class=\"history\">PR Number</th>";
      print OUTFILE "<th class=\"history\">Customer</th>";
      print OUTFILE "<th class=\"history\">Release Notes</th>";
      print OUTFILE "<th class=\"history\">Comments</th>";
      print OUTFILE "<th class=\"history\">File name</th>";
      print OUTFILE "<th class=\"history\" colspan=\"1\">Subdirectory</th>";
      print OUTFILE "<tr>";
}
htmlprint();


####################################
#XML BEGIN
####################################
sub xmlprint {
  unlink ($xmloutFile);
  open (XMLOUTFILE, ">>$xmloutFile") or die "Can't write to $xmloutFile: $!";
      print XMLOUTFILE "<?xml version=\"1.0\"?>\n";
      print XMLOUTFILE "<?mso-application progid=\"Excel.Sheet\"?>\n";
      print XMLOUTFILE "<Workbook xmlns=\"urn:schemas-microsoft-com:office:spreadsheet\"\n";
      print XMLOUTFILE " xmlns:o=\"urn:schemas-microsoft-com:office:office\"\n";
      print XMLOUTFILE " xmlns:x=\"urn:schemas-microsoft-com:office:excel\"\n";
      print XMLOUTFILE " xmlns:ss=\"urn:schemas-microsoft-com:office:spreadsheet\"\n";
      print XMLOUTFILE " xmlns:html=\"http://www.w3.org/TR/REC-html40\">\n";
      print XMLOUTFILE " <DocumentProperties xmlns=\"urn:schemas-microsoft-com:office:office\">\n";
      print XMLOUTFILE "  <Author>Daniel Edelson</Author>\n";
      print XMLOUTFILE "  <LastAuthor>Daniel Edelson</LastAuthor>\n";
      print XMLOUTFILE "  <Created>2010-09-23T07:44:38Z</Created>\n";
      print XMLOUTFILE "  <LastSaved>2010-09-23T07:49:16Z</LastSaved>\n";
      print XMLOUTFILE "  <Company>Broadcom Corporation</Company>\n";
      print XMLOUTFILE "  <Version>12.00</Version>\n";
      print XMLOUTFILE " </DocumentProperties>\n";
      print XMLOUTFILE " <ExcelWorkbook xmlns=\"urn:schemas-microsoft-com:office:excel\">\n";
      print XMLOUTFILE "  <WindowHeight>14565</WindowHeight>\n";
      print XMLOUTFILE "  <WindowWidth>28635</WindowWidth>\n";
      print XMLOUTFILE "  <WindowTopX>120</WindowTopX>\n";
      print XMLOUTFILE "  <WindowTopY>60</WindowTopY>\n";
      print XMLOUTFILE "  <ProtectStructure>False</ProtectStructure>\n";
      print XMLOUTFILE "  <ProtectWindows>False</ProtectWindows>\n";
      print XMLOUTFILE " </ExcelWorkbook>\n";
      print XMLOUTFILE " <Styles>\n";
      print XMLOUTFILE "  <Style ss:ID=\"Default\" ss:Name=\"Normal\">\n";
      print XMLOUTFILE "   <Alignment ss:Vertical=\"Bottom\"/>\n";
      print XMLOUTFILE "   <Borders/>\n";
      print XMLOUTFILE "   <Font ss:FontName=\"Calibri\" x:Family=\"Swiss\" ss:Size=\"11\" ss:Color=\"#000000\"/>\n";
      print XMLOUTFILE "   <Interior/>\n";
      print XMLOUTFILE "   <NumberFormat/>\n";
      print XMLOUTFILE "   <Protection/>\n";
      print XMLOUTFILE "  </Style>\n";

      print XMLOUTFILE " <Style ss:ID=\"s61\" ss:Name=\"Hyperlink\">\n";
      print XMLOUTFILE "  <Font ss:FontName=\"Calibri\" x:Family=\"Swiss\" ss:Size=\"11\" ss:Color=\"#0000FF\"\n";
      print XMLOUTFILE "  ss:Underline=\"Single\"/>\n";
      print XMLOUTFILE " </Style>\n";

      print XMLOUTFILE "  <Style ss:ID=\"s62\">\n";
      print XMLOUTFILE "   <Borders>\n";
      print XMLOUTFILE "    <Border ss:Position=\"Bottom\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>\n";
      print XMLOUTFILE "    <Border ss:Position=\"Left\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>\n";
      print XMLOUTFILE "    <Border ss:Position=\"Right\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>\n";
      print XMLOUTFILE "    <Border ss:Position=\"Top\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>\n";
      print XMLOUTFILE "   </Borders>\n";
      print XMLOUTFILE "   <Font ss:FontName=\"Calibri\" x:Family=\"Swiss\" ss:Size=\"11\" ss:Color=\"#000000\"\n";
      print XMLOUTFILE "    ss:Bold=\"1\"/>\n";
      print XMLOUTFILE "   <Interior ss:Color=\"#FFFF00\" ss:Pattern=\"Solid\"/>\n";
      print XMLOUTFILE "  </Style>\n";
      print XMLOUTFILE "  <Style ss:ID=\"s63\">\n";
      print XMLOUTFILE "   <NumberFormat ss:Format=\"Short Date\"/>\n";
      print XMLOUTFILE "  </Style>\n";
      print XMLOUTFILE "  <Style ss:ID=\"s64\">\n";
      print XMLOUTFILE "   <NumberFormat ss:Format=\"h:mm:ss\"/>\n";
      print XMLOUTFILE "  </Style>\n";

      print XMLOUTFILE "  <Style ss:ID=\"s68\" ss:Parent=\"s61\">\n";
      print XMLOUTFILE "   <Alignment ss:Vertical=\"Bottom\"/>\n";
      print XMLOUTFILE "   <NumberFormat/>\n";
      print XMLOUTFILE "   <Protection/>\n";
      print XMLOUTFILE "  </Style>\n";

      print XMLOUTFILE " </Styles>\n";
      print XMLOUTFILE " <Worksheet ss:Name=\"Sheet1\">\n";
      print XMLOUTFILE "  <Names>\n";
      print XMLOUTFILE "   <NamedRange ss:Name=\"$exlTmpltName\"\n";
      print XMLOUTFILE "    ss:RefersTo=\"=Sheet1!R1C1:R4C13\"/>\n";
      print XMLOUTFILE "  </Names>\n";
      print XMLOUTFILE "  <Table ss:ExpandedColumnCount=\"13\" ss:ExpandedRowCount=\"20000\" x:FullColumns=\"1\"\n";
      print XMLOUTFILE "   x:FullRows=\"1\" ss:DefaultRowHeight=\"15\">\n";
      print XMLOUTFILE "   <Column ss:Width=\"69.75\"/>\n";
      print XMLOUTFILE "   <Column ss:Width=\"75.75\"/>\n";
      print XMLOUTFILE "   <Column ss:Width=\"111\" ss:Span=\"1\"/>\n";
      print XMLOUTFILE "   <Column ss:Index=\"5\" ss:Width=\"51\"/>\n";
      print XMLOUTFILE "   <Column ss:AutoFitWidth=\"0\" ss:Width=\"42.75\"/>\n";
      print XMLOUTFILE "   <Column ss:AutoFitWidth=\"0\" ss:Width=\"45.75\"/>\n";
      print XMLOUTFILE "   <Column ss:Width=\"63\"/>\n";
      print XMLOUTFILE "   <Column ss:Width=\"50.25\"/>\n";
      print XMLOUTFILE "   <Column ss:Width=\"72.75\"/>\n";
      print XMLOUTFILE "   <Column ss:Width=\"426\"/>\n";
      print XMLOUTFILE "   <Column ss:Width=\"78.75\"/>\n";
      print XMLOUTFILE "   <Column ss:Width=\"85.5\"/>\n";
      print XMLOUTFILE "   <Row ss:AutoFitHeight=\"0\">\n";
      print XMLOUTFILE "    <Cell ss:StyleID=\"s62\"><Data ss:Type=\"String\">BOM</Data><NamedCell\n";
      print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";
      print XMLOUTFILE "    <Cell ss:StyleID=\"s62\"><Data ss:Type=\"String\">Module</Data><NamedCell\n";
      print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";
      print XMLOUTFILE "    <Cell ss:StyleID=\"s62\"><Data ss:Type=\"String\">Tag</Data><NamedCell\n";
      print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";
      print XMLOUTFILE "    <Cell ss:StyleID=\"s62\"><Data ss:Type=\"String\">Branch</Data><NamedCell\n";
      print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";
      print XMLOUTFILE "    <Cell ss:StyleID=\"s62\"><Data ss:Type=\"String\">Date</Data><NamedCell\n";
      print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";
      print XMLOUTFILE "    <Cell ss:StyleID=\"s62\"><Data ss:Type=\"String\">Time</Data><NamedCell\n";
      print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";
      print XMLOUTFILE "    <Cell ss:StyleID=\"s62\"><Data ss:Type=\"String\">Author</Data><NamedCell\n";
      print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";
      print XMLOUTFILE "    <Cell ss:StyleID=\"s62\"><Data ss:Type=\"String\">PR Number</Data><NamedCell\n";
      print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";
      print XMLOUTFILE "    <Cell ss:StyleID=\"s62\"><Data ss:Type=\"String\">Customer</Data><NamedCell\n";
      print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";
      print XMLOUTFILE "    <Cell ss:StyleID=\"s62\"><Data ss:Type=\"String\">Release Notes</Data><NamedCell\n";
      print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";
      print XMLOUTFILE "    <Cell ss:StyleID=\"s62\"><Data ss:Type=\"String\">Comments</Data><NamedCell\n";
      print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";
      print XMLOUTFILE "    <Cell ss:StyleID=\"s62\"><Data ss:Type=\"String\">File name</Data><NamedCell\n";
      print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";
      print XMLOUTFILE "    <Cell ss:StyleID=\"s62\"><Data ss:Type=\"String\">Subdirectory</Data><NamedCell\n";
      print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";
      print XMLOUTFILE "   </Row>\n";
      #print XMLOUTFILE "   <Row ss:Index=\"3\" ss:AutoFitHeight=\"0\">\n";
      #print XMLOUTFILE "   <Row ss:AutoFitHeight=\"0\">\n";
}
xmlprint();

####################################
#XML END
####################################

foreach (@bomName){
  $_=~ s/(\s)*//g;
  my $bomName=$_;
  my @hArray=();
  my $mtb=$tb."___".$bomName;
  $inFile="diff___$mtb.txt";
#IN Final version, remove this IF condition
#since want to have an updated file every time.
#DEBUG
##  if (! -s "../archivedir/$inFile"){
    #2010system("src/tools/build/hndcvs -rlog -dr $bomName $tag $branch");
    system("/home/hwnbuild/src/tools/build/contentlist/generaterep.bash $tag $branch $bomName $inFile > genrep.out 2>&1");
    next if (! -s $inFile);
    system("cp -f $inFile ../archivedir/");
##  }
  
  open (INDIFF, "../archivedir/$inFile") or die "Can't read $inFile: $!";
  
  while (<INDIFF>){
    $myLine=$_;
    if ($myLine =~/Diffing MODULE (.*)/){
      $submodule=$1;
      $myLine = <INDIFF>;
    }
    if ($myLine =~/Using TAG (.*)/){
      $subtag=$1;
      $myLine = <INDIFF>;
    }
    if ($myLine =~/Using BRANCH (.*)/){
      $subbranch=$1;
      $myLine = <INDIFF>;
    }
    if ($myLine =~/^RCS file: (.*)/)
    {
      my @fullPath=split(/cvsroot\/(.*),v/,$1);
      $fileName=$fullPath[1];
      #Read until ======
      my $inblock="true";
      my %hsh=();
      my @ARR=();
      while ($inblock eq "true"){
        $myLine = <INDIFF>;
        if ($myLine =~/^======/){ 
          $inblock="false";
        }else{
          if ($myLine =~/^revision (.*)/)
          {
            $rev=$1;
            $prevRev=get_previous_rev $rev;
            my $aa=<INDIFF>;
            @ARR=split(' ', $aa);
            # read until ---
            $comment=<INDIFF>;
            my $cmtbegin="true";
            while ($cmtbegin eq "true") {
              $ctmp=<INDIFF>;
            
              if ($ctmp =~/----/) {$cmtbegin="false";}
              elsif ($ctmp =~/====/) {$cmtbegin="false";$inblock="false";}
              else{
                chomp($ctmp);
                $comment.=$ctmp;
              }
            }
            #Enter in hash
            $hsh{$rev}=[$ARR[0],$ARR[1],$ARR[2],$ARR[3],$ARR[4],$ARR[5], $comment, $prevRev, $submodule, $subtag, $subbranch];
          }
        }
      }
      #Enter it in array of hash
      if (%hsh) {
        push(@hArray,[$fileName,\%hsh]);
      }
    }
  }
  
  
     #Not really sure of history -> . print OUTFILE "<tr class=\"history\">";

 for $i ( 0 .. $#hArray ) {
    #File name
    my $changedFile=$hArray[$i][0];
    $changedFile=`basename "$hArray[$i][0]"`;
    chomp($changedFile);
    $changedDir=`dirname "$hArray[$i][0]"`;
    chomp($changedDir);
    my @sorted_keys=sort(keys %{$hArray[$i][1]});
      my $customerValue="";
  
    foreach my $revisionNum (@sorted_keys){
      my $dateValue="$hArray[$i][1]{$revisionNum}[1]";
      my $timeValue="$hArray[$i][1]{$revisionNum}[2]";
      $timeValue=~ s/;//g;
      my $authorValue="$hArray[$i][1]{$revisionNum}[4]";
      $authorValue=~ s/;//g;
      my $commentsValue="$hArray[$i][1]{$revisionNum}[6]";
      my $prevRevValue="$hArray[$i][1]{$revisionNum}[7]";
      my $module="$hArray[$i][1]{$revisionNum}[8]";
      my $tag="$hArray[$i][1]{$revisionNum}[9]";
      my $branch="$hArray[$i][1]{$revisionNum}[10]";
      my $prNumber="";
      my $prDigits="";
      my @prNumber=();
      my $prGnatsObj=();
      my $releaseNotes="";

      my $tempchangedfile=$changedFile;
      my $xmlchangedFile=$changedFile;
      $changedFile=~ s#$changedFile#<a href=\"$viewCvsBase$changedDir/$changedFile\?r1\=$prevRevValue\&r2\=$revisionNum\"\>$tempchangedfile#g;
      $xmlchangedFile=~ s#$xmlchangedFile#$viewCvsBase$changedDir/$xmlchangedFile\?r1\=$prevRevValue\&amp\;r2\=$revisionNum#g;
      @prNumber=$commentsValue=~ /PR[\s\#:]*(\d+)\b/g;

        $sizeArray=@prNumber;
        if ($sizeArray > 1){
          for (0 .. ($sizeArray-2)){
            @prNumber[$_]=~ s/$/\,/;
          }
        }
        @genericprNumber=@prNumber;
        $xmlprNumber="$gnatsUrl;cmd\=view+audit-trail;pr\=@prNumber[0]";

      foreach(@prNumber){
        $prNumber=$_;
        $_="\<a href\=\"$gnatsUrl;cmd\=view+audit-trail;pr\=$prNumber\"\>$prNumber\<\/a\>";
          $prGnatsObj=$gnatsDb->getPRByNumber($prNumber);
          $customerValue=$prGnatsObj->getField('Customer');
          $prGnatsObj=$gnatsDb->getPRByNumber($prNumber);
          $releaseNotes=$prGnatsObj->getField('Root-Cause-and-Customer-Notes');
      }

      $genericcommentsValue=$commentsValue;
      $xmlcommentsValue=$commentsValue;
      chomp($genericcommentsValue);
      chomp($xmlcommentsValue);
        $xmlcommentsValue=~ s#&#&amp;#g;
        $xmlcommentsValue=~ s#<#&lt;#g;
        $xmlcommentsValue=~ s#>#&gt;#g;
        $xmlcommentsValue=~ s#"#&quot;#g;
        $xmlcommentsValue=~ s#'#&apos;#g;
      $commentsValue=~ s#(PR[\s\#:]*(\d+)\b)#<a href="$gnatsUrl;cmd=view+audit-trail;pr=$2">$1</a>#g;

      print OUTFILE "</tr>";
      print OUTFILE "<tr class=\"itemM0\">";
      print OUTFILE "<td class=\"itemact\"></td>";
      print OUTFILE "<td class=\"item\"> $bomName </td>";
      print OUTFILE "<td class=\"item\"> $module </td>";
      print OUTFILE "<td class=\"item\"> $tag </td>";
      print OUTFILE "<td class=\"item\"> $branch </td>";
      print OUTFILE "<td class=\"item\"> $dateValue</a>";
      print OUTFILE "<td class=\"item\"> $timeValue</a>";
      print OUTFILE "<td class=\"item\"> $authorValue</a>";
      print OUTFILE "<td class=\"item\"> @prNumber</a>";
      print OUTFILE "<td class=\"item\"> $customerValue</a>";
      print OUTFILE "<td class=\"item\"> $releaseNotes</a>";
      print OUTFILE "<td class=\"item\"> $commentsValue</a>";
      print OUTFILE "<td class=\"item\"> $changedFile</a>";
      print OUTFILE "<td class=\"item\"> $changedDir</a>";
      print OUTFILE "</tr>";

####################
#CSVBEGIN
####################
      $changedFile=$tempchangedfile;
      print CSVOUTFILE "$bomName,$module,$tag,$branch,$dateValue,$timeValue,$authorValue,\"@genericprNumber\",$customerValue,$releaseNotes,\"$genericcommentsValue\",$changedFile,$changedDir\n";
####################
#XMLBEGIN
####################
      print XMLOUTFILE "   <Row ss:AutoFitHeight=\"0\">\n";
      print XMLOUTFILE "    <Cell><Data ss:Type=\"String\">$bomName</Data><NamedCell\n";
      print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";
      print XMLOUTFILE "    <Cell><Data ss:Type=\"String\">$module</Data><NamedCell\n";
      print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";
      print XMLOUTFILE "    <Cell><Data ss:Type=\"String\">$tag</Data><NamedCell\n";
      print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";
      print XMLOUTFILE "    <Cell><Data ss:Type=\"String\">$branch</Data><NamedCell\n";
      print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";
      $xmlDate="$dateValue"."T00";
      $xmlDate=~s#/#-#g;
      print XMLOUTFILE "    <Cell ss:StyleID=\"s63\"><Data ss:Type=\"DateTime\">$xmlDate:00:00.000</Data><NamedCell\n";
      print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";
      print XMLOUTFILE "    <Cell ss:StyleID=\"s64\"><Data ss:Type=\"DateTime\">1899-12-31T$timeValue.000</Data><NamedCell\n";
      print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";
      print XMLOUTFILE "    <Cell><Data ss:Type=\"String\">$authorValue</Data><NamedCell\n";
      print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";
      #print XMLOUTFILE "    <Cell><Data ss:Type=\"String\" x:Ticked=\"1\">@prNumber</Data><NamedCell\n";
      #print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";
     
      print XMLOUTFILE "    <Cell ss:StyleID=\"s68\" ss:HRef=\"$xmlprNumber\"><Data\n";
      print XMLOUTFILE "      ss:Type=\"Number\">@genericprNumber[0]</Data></Cell>\n";

      print XMLOUTFILE "    <Cell><Data ss:Type=\"String\">$customerValue</Data><NamedCell\n";
      print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";
      print XMLOUTFILE "    <Cell><Data ss:Type=\"String\">$releaseNotes</Data><NamedCell\n";
      print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";
      print XMLOUTFILE "    <Cell ss:Index=\"11\"><Data ss:Type=\"String\">$xmlcommentsValue</Data><NamedCell\n";
      print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";

      #print XMLOUTFILE "    <Cell><Data ss:Type=\"String\">$changedFile</Data><NamedCell\n";
      #print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";

      print XMLOUTFILE "    <Cell ss:StyleID=\"s61\" ss:HRef=\"$xmlchangedFile\"><Data\n";
      print XMLOUTFILE "      ss:Type=\"String\">$changedFile</Data></Cell>\n";

      print XMLOUTFILE "    <Cell><Data ss:Type=\"String\">$changedDir</Data><NamedCell\n";
      print XMLOUTFILE "      ss:Name=\"$exlTmpltName\"/></Cell>\n";
      print XMLOUTFILE "   </Row>\n";
####################
#XMLEND
####################
}
  }
}#foreach
      print OUTFILE "</tr>";
      print OUTFILE "    </table>";
      print OUTFILE " </body>";
      print OUTFILE "</html>";

####################
#XMLBEGIN
####################
      print XMLOUTFILE "  </Table>\n";
      print XMLOUTFILE "  <WorksheetOptions xmlns=\"urn:schemas-microsoft-com:office:excel\">\n";
      print XMLOUTFILE "   <PageSetup>\n";
      print XMLOUTFILE "    <Header x:Margin=\"0.3\"/>\n";
      print XMLOUTFILE "    <Footer x:Margin=\"0.3\"/>\n";
      print XMLOUTFILE "    <PageMargins x:Bottom=\"0.75\" x:Left=\"0.7\" x:Right=\"0.7\" x:Top=\"0.75\"/>\n";
      print XMLOUTFILE "   </PageSetup>\n";
      print XMLOUTFILE "   <Unsynced/>\n";
      print XMLOUTFILE "   <Selected/>\n";
      print XMLOUTFILE "   <Panes>\n";
      print XMLOUTFILE "    <Pane>\n";
      print XMLOUTFILE "     <Number>3</Number>\n";
      print XMLOUTFILE "     <ActiveRow>24</ActiveRow>\n";
      print XMLOUTFILE "     <ActiveCol>4</ActiveCol>\n";
      print XMLOUTFILE "    </Pane>\n";
      print XMLOUTFILE "   </Panes>\n";
      print XMLOUTFILE "   <ProtectObjects>False</ProtectObjects>\n";
      print XMLOUTFILE "   <ProtectScenarios>False</ProtectScenarios>\n";
      print XMLOUTFILE "  </WorksheetOptions>\n";
      print XMLOUTFILE "  <QueryTable xmlns=\"urn:schemas-microsoft-com:office:excel\">\n";
      print XMLOUTFILE "   <Name>$exlTmpltName</Name>\n";
      print XMLOUTFILE "   <AutoFormatFont/>\n";
      print XMLOUTFILE "   <AutoFormatPattern/>\n";
      print XMLOUTFILE "   <QuerySource>\n";
      print XMLOUTFILE "    <QueryType>Web</QueryType>\n";
      print XMLOUTFILE "    <EnableRedirections/>\n";
      print XMLOUTFILE "    <RefreshedInXl9/>\n";
      print XMLOUTFILE "    <EntirePage/>\n";
      print XMLOUTFILE "    <URLString\n";
      print XMLOUTFILE "     x:HRef=\"file:///Z:$fileDrop/$logbtb.html\"/>\n";
      print XMLOUTFILE "    <VersionLastEdit>3</VersionLastEdit>\n";
      print XMLOUTFILE "    <VersionLastRefresh>3</VersionLastRefresh>\n";
      print XMLOUTFILE "   </QuerySource>\n";
      print XMLOUTFILE "  </QueryTable>\n";
      print XMLOUTFILE " </Worksheet>\n";
      print XMLOUTFILE " <Worksheet ss:Name=\"Sheet2\">\n";
      print XMLOUTFILE "  <Table ss:ExpandedColumnCount=\"1\" ss:ExpandedRowCount=\"1\" x:FullColumns=\"1\"\n";
      print XMLOUTFILE "   x:FullRows=\"1\" ss:DefaultRowHeight=\"15\">\n";
      print XMLOUTFILE "   <Row ss:AutoFitHeight=\"0\"/>\n";
      print XMLOUTFILE "  </Table>\n";
      print XMLOUTFILE "  <WorksheetOptions xmlns=\"urn:schemas-microsoft-com:office:excel\">\n";
      print XMLOUTFILE "   <PageSetup>\n";
      print XMLOUTFILE "    <Header x:Margin=\"0.3\"/>\n";
      print XMLOUTFILE "    <Footer x:Margin=\"0.3\"/>\n";
      print XMLOUTFILE "    <PageMargins x:Bottom=\"0.75\" x:Left=\"0.7\" x:Right=\"0.7\" x:Top=\"0.75\"/>\n";
      print XMLOUTFILE "   </PageSetup>\n";
      print XMLOUTFILE "   <Unsynced/>\n";
      print XMLOUTFILE "   <ProtectObjects>False</ProtectObjects>\n";
      print XMLOUTFILE "   <ProtectScenarios>False</ProtectScenarios>\n";
      print XMLOUTFILE "  </WorksheetOptions>\n";
      print XMLOUTFILE " </Worksheet>\n";
      print XMLOUTFILE " <Worksheet ss:Name=\"Sheet3\">\n";
      print XMLOUTFILE "  <Table ss:ExpandedColumnCount=\"1\" ss:ExpandedRowCount=\"1\" x:FullColumns=\"1\"\n";
      print XMLOUTFILE "   x:FullRows=\"1\" ss:DefaultRowHeight=\"15\">\n";
      print XMLOUTFILE "   <Row ss:AutoFitHeight=\"0\"/>\n";
      print XMLOUTFILE "  </Table>\n";
      print XMLOUTFILE "  <WorksheetOptions xmlns=\"urn:schemas-microsoft-com:office:excel\">\n";
      print XMLOUTFILE "   <PageSetup>\n";
      print XMLOUTFILE "    <Header x:Margin=\"0.3\"/>\n";
      print XMLOUTFILE "    <Footer x:Margin=\"0.3\"/>\n";
      print XMLOUTFILE "    <PageMargins x:Bottom=\"0.75\" x:Left=\"0.7\" x:Right=\"0.7\" x:Top=\"0.75\"/>\n";
      print XMLOUTFILE "   </PageSetup>\n";
      print XMLOUTFILE "   <Unsynced/>\n";
      print XMLOUTFILE "   <ProtectObjects>False</ProtectObjects>\n";
      print XMLOUTFILE "   <ProtectScenarios>False</ProtectScenarios>\n";
      print XMLOUTFILE "  </WorksheetOptions>\n";
      print XMLOUTFILE " </Worksheet>\n";
      print XMLOUTFILE "</Workbook>\n";


####################
#XMLEND
####################

$gnatsDb->disconnect();
close INDIFF;
close BLIST;
      close CSVOUTFILE;
      close XMLOUTFILE;
      close OUTFILE;

###if (! -s "../archivedir/$outFile") {
system("cp -f $outFile ../archivedir/");
###}
###if (! -s "../archivedir/$csvoutFile") {
system("cp -f $csvoutFile ../archivedir/");
###}
###if (! -s "../archivedir/$xmloutFile") {
system("cp -f $xmloutFile ../archivedir/");
###}
###if (! -s "../archivedir/$inFile") {
system("cp -f $inFile ../archivedir/");
###}

remove_tree($tempDirname);

print "Completed";

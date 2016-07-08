##############################################################################
#  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to the terms and
#  conditions of a separate, written license agreement executed between you and Broadcom
#  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#  no license (express or implied), right to use, or waiver of any kind with respect to the
#  Software, and Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
##############################################################################

#!/usr/brcm/ba/bin/perl
# SecureSign Request Automation sample
#use WWW::Mechanize;
use LWP;
use HTML::Form;
use MIME::Base64;
use strict;
use warnings;

#use Getopt::Long; # TODO: Accept CL options
#
# SecureSign Website - ensure this is the site to point at
my $url = "http://securesign.broadcom.com/"; # TODO: Make parameteric
my $SS_URL = 'http://securesign.broadcom.com';

#  Credentials to use the website (from Environment vars ?)
if (!$ENV{'SS_USER'} || !$ENV{'SS_PASS'}) {die("SS_USER or SS_PASS Missing\n");}
if (!$ENV{'BCHP_ID'}) {die("BCHP_ID Missing\n");}

my $KeyGroup = "";
my $Algo = "";
#my $KeyVariant = "";
#if ($ENV{'CHP_VARIANT'} =~ /ZB/)
#{
#  $KeyVariant = "_prod_key0";
#  $KeyVariant = "_Prod_Key2";
#} elsif ($ENV{'CHP_VARIANT'} =~ /ZD/)
#{
#  $KeyVariant = "ZS_dev_key0";
#} else
#{
#  die ("Cannot Determine Part Variant. Supported Types: ZB, ZD.\n");
#}

#$KeyGroup = "BCM$ENV{'BCHP_ID'}$KeyVariant";
$KeyGroup = "BCM$ENV{'BCHP_ID'}$ARGV[1]";
$Algo = "SHA256";
print "parameters = $KeyGroup -- $Algo\n";

my $passfld = "Basic ".MIME::Base64::encode_base64("$ENV{'SS_USER'}:$ENV{'SS_PASS'}");
# Detect current user, use file in homedir
my @pw = getpwnam($ENV{'USER'});
my $t = time();
my $fname = $ARGV[0];
if (!-f $fname ) {die("File Missing ($fname)\n");}

my @BinsToSign = `cat $ARGV[0]`;
chomp(@BinsToSign);
my $NumberOfBins = scalar @BinsToSign;

# Fill In Your request specific values here !
my %fval = (
  'sskeygrp' => $KeyGroup,
  'ssalgo' => $Algo,
  'comments' => "Test By Automation script ($pw[0], $t).\n",
);

my $i = 0;
for my $FileName (@BinsToSign)
{
  $fval{'filename'.$i} = $FileName;
  $i++;
}

# LWP -Based automation
my $ua = LWP::UserAgent->new();
$ua->default_header('Authorization', $passfld);
# Request Form
my $resp = $ua->get($url);
#print ("Some Info:: $resp->base $resp->content_charset \n");
my $cont = $resp->decoded_content();
# Add file fields dynamically on the form
$cont = addfilefields($cont, $NumberOfBins);

#print("$cont");

my $L_SS_URL = "${SS_URL}" . "\/\?act\=newreq";
my ($form) = HTML::Form->parse($cont, $L_SS_URL);
$form->strict(1);
# Add request specific values

map({$form->value($_, $fval{$_});}  keys(%fval));
#$form->dump();

# Submit by click+request
my $req = $form->click();
$resp = $ua->request($req);

# Check the success !
if (!$resp->is_success()) {die("Problems submitting the request: ".$resp->status_line()."\n");}
# Analyze response content (for errors)
$cont = $resp->decoded_content();
#DEBUG:print($cont);

my $reqid = 0;
if ($cont =~ /^Error/) {die("Errors Found\n\n");} # TODO: Extract the error message
elsif ($cont =~ /\(ID:\s*(\d+)\)/) {$reqid = $1;print("Signing request successfully submitted to Secure Signing Server. (REQ_ID: $reqid)\n\n");} # (ID: 115)
if (!$reqid) {print("No RequestID");}
#DEBUG:print($cont);
monitor_status($ua, $reqid);

for my $FileName (@BinsToSign)
{
  #Remove leading output directory path. We only need the file name here.
  $FileName =~ s{.*/}{};
  system("cp $ENV{'OUT_DIR'}/SecureSign_Req_${reqid}_*/$FileName.$KeyGroup.sign.* $ENV{'OUT_DIR'}/$FileName.signature.be");
}

system ("cp $ENV{'OUT_DIR'}/SecureSign_Req_${reqid}_*/$KeyGroup.pem $ENV{'OUT_DIR'}/$ENV{'BCHP_ID'}_ss_fw_key2.pem");

# The rest of the processing is asynchronous. Try monitoring the progress from the URL of single request page.
sub monitor_status {
  my ($ua, $reqid) = @_;
  my $i = 0;
  my $download_found = 0;
  my $urlmon = "$SS_URL" . "/?act=shrq&rid=" . $reqid;# (Use $reqid parameter above)
  my $resp;

  #fishing for download option
  do
  {
     $resp = $ua->get($urlmon);
     if (! $resp->is_success)
     {
       print "Request for $urlmon FAILED\n";
       print $resp->status_line;
       exit 1;
     }
     my $cont = $resp->decoded_content();
     my $statpatt = qr/Request Status<\/td><td><span class=".*">(\w+)\s*<\/span>/;
     my $status = '';
     if ($cont =~ /$statpatt/) {$status = $1;} # Looking for "completed".
     print("Your request $reqid is in status '$status'\n");
     if ($status =~ /completed/)
     {
        #Check for download link availability
        if ($cont =~ m/Download Complete File Bundle/i) {
          print "Found Download Bundle \n";
          $download_found = 1;
        } else {
          print "Download Bundle NOT FOUND\n";
          print "$urlmon\n";
          exit 1;
        }
     }
     else
     {
        print "Will query server again in 10 seconds ...\n";
        sleep 10;
     }

  } until($i++ > 130 ||  $download_found == 1);

  my $filename = "$ENV{'OUT_DIR'}/signature_".${reqid}.'.tar.gz';
  my $uri = "$SS_URL" . "/?act=dnloadall&rid=${reqid}";
  $req = HTTP::Request->new(GET => $uri);
  $resp = $ua->request($req, $filename);
  if (! $resp->is_success) {
     print "File Download Failed. $uri \n";
     print $resp->status_line;
     exit 1;
  }
  system("tar xvfz $filename -C $ENV{'OUT_DIR'}");

}
exit(0);
# WWW::Mechanize based automation becomes impractical as we need to dynamically add fields on form
#my $mech = WWW::Mechanize->new();
#$mech->add_header('Authorization', $passfld);
#$mech->get($url);
#$mech->form_number(1);
#my $cont = $mech->content();
#$cont = addfilefields($cont, 1);
#map({$mech->field($_, $fval{$_});}  keys(%fval));
#$mech->submit();
#DEBUG:print($cont);exit;

sub addfilefields {
    my ($cont, $num) = @_;
    my $addfin = '';
    for (my $i = 0; $i < $num; $i++) {
        $addfin .= "<input type=\"file\" name=\"filename".$i."\" size=\"100\">\n<br>\n";
    }
   $cont =~ s/(<div id="fdiv">)/$1\n\n$addfin/;
   return($cont);
}

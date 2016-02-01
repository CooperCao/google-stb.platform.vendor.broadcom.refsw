############################################################################
#     (c)2015 Broadcom Corporation
#
#  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
#
###########################################################################
#!/usr/bin/perl

my $filename = shift; # Get the filename from command line.
my $counter = 0;

open(FILE, $filename) or die "Can't open $filename: $!";

foreach $gpioType (gpio, sgpio, aon_gpio, aon_sgpio) {

    # Change aon_gpio -> aonGpio to match Nexus coding convention.
    my $gpioString = $gpioType;
    $gpioString =~ s/_([a-z]+)/\u$1/;

    my @gpioArray;
    my $gpioData;

    while (<FILE>) { # Set $_ to each line of the file

        if ($_ =~ m/BCHP_([A-Z_]+)_PIN_MUX_CTRL_([0-9]+)_([a-z_]+)_([0-9]+)_MASK/) {

            my $reg = $2;
            my $pin = $4;
            my $currentGpioType = $3;
            my $gpioBase = $1;
            my $lineEnding = "";
            my $pinLenght = 2;

            # Perl gibberish to decide how many digits in the pin number.
            if ($pin =~ m/^\d{3}$/) {
                $pinLength = 3;
            } else {
                if (m/\d/) {
                    $pinLength = 2;
                }
            }

            if ($currentGpioType eq $gpioType) {

                if ($counter != 0) { # This is to avoid the comma at the end of the data array.
                    $lineEnding = ",\n";
                }

                if ($pinLength == 3) {

                   $gpioData = sprintf("%s    { BCHP_%s_PIN_MUX_CTRL_%d, BCHP_%s_PIN_MUX_CTRL_%d_%s_%03d_MASK, BCHP_%s_PIN_MUX_CTRL_%d_%s_%03d_SHIFT}",
                                        $lineEnding,
                                        $gpioBase,$reg,
                                        $gpioBase,$reg,$gpioType,$pin,
                                        $gpioBase,$reg,$gpioType,$pin);
                } else {

                    $gpioData = sprintf("%s    { BCHP_%s_PIN_MUX_CTRL_%d, BCHP_%s_PIN_MUX_CTRL_%d_%s_%02d_MASK, BCHP_%s_PIN_MUX_CTRL_%d_%s_%02d_SHIFT}",
                                        $lineEnding,
                                        $gpioBase,$reg,
                                        $gpioBase,$reg,$gpioType,$pin,
                                        $gpioBase,$reg,$gpioType,$pin);
                }
                $counter++;

                push(@gpioArray,$gpioData);
            }
        }
    }

    if ($counter > 0) {
        print ("\n");
        print ("static const NEXUS_GpioTable g_","$gpioString","Table\[\] = {\n");
        print ("@gpioArray");
        print ("\n};\n\n");

        printf ("#undef NEXUS_%s_PINS\n",uc $gpioType);
        printf ("#define NEXUS_%s_PINS (sizeof(g_%sTable)/sizeof(g_%sTable[0]))\n\n",uc $gpioType, $gpioString,$gpioString);
    }

    seek(FILE, 0, 0); # reset to start of file
    $counter = 0; # reset counter dtecting start of array
    @gpioArray = ();  # clear array
}

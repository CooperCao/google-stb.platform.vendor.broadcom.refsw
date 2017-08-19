# Creates Excel Charts from data input file, using Win32 OLE.
# Works only on Windows, as MS Excel must be installed!
# Written by John Brearley 2006

# Load required packages.
use Cwd;
use File::Basename;
eval "use Win32::OLE";
if ($@ ne "") {
   print "$0 ERROR: WIN32::OLE is not installed!\n";
   exit 1;
}


#==================== Data definitions ============================
# Data definitions, Excel magic numbers.
#==================================================================
$company_name = "broadcom.com" ;# company name
$file_format_xls = -4143;# file format for normal XLS workbook
$max_blank_rows = 100;# maximum consecutive blank rows allowed in worksheet before aborting data processing.
$worksheet_max_column = 256;# maximum columns allowed in a worksheet
$worksheet_max_row = 65536;# maximum rows allowed in a worksheet
$xlColumns = 2;# needed by chart routines.
$xlLegendPositionBottom = -4107;
$xlRows = 1;# needed by chart routines.
$xlUnderlineStyleSingle = 2;

# Setup hash of all documented Excel chart type values.
%chart_types = (xlLine=>4, xlLineMarkersStacked=>66, xlLineStacked=>63,
   xlPie=>5, xlPieOfPie=>68, xlPyramidBarStacked=>110, xlPyramidCol=>112, 
   xlPyramidColClustered=>106, xlPyramidColStacked=>107, 
   xlPyramidColStacked100=>108, xlRadar=>-4151, xlRadarFilled=>82, 
   xlRadarMarkers=>81, xlStockHLC=>88, xlStockOHLC=>89, xlStockVHLC=>90,
   xlStockVOHLC=>91, xlSurface=>83, xlSurfaceTopView=>85, 
   xlSurfaceTopViewWireframe=>86, xlSurfaceWireframe=>84, 
   xlXYScatter=>-4169, xlXYScatterLines=>74, xlXYScatterLinesNoMarkers=>75,
   xlXYScatterSmooth=>72, xlXYScatterSmoothNoMarkers=>73, xl3DArea=>-4098,
   xl3DAreaStacked=>78, xl3DAreaStacked100=>79, xl3DBarClustered=>60, 
   xl3DBarStacked=>61, xl3DBarStacked100=>62, xl3DColumn=>-4100,
   xl3DColumnClustered=>54, xl3DColumnStacked=>55, xl3DColumnStacked100=>56,
   xl3DLine=>-4101, xl3DPie=>-4102, xl3DPieExploded=>70, xlArea=>1,
   xlAreaStacked=>76, xlAreaStacked100=>77, xlBarClustered=>57, xlBarOfPie=>71,
   xlBarStacked=>58, xlBarStacked100=>59, xlBubble=>15, xlBubble3DEffect=>87,
   xlColumnClustered=>51, xlColumnStacked=>52, xlColumnStacked100=>53,
   xlConeBarClustered=>102, xlConeBarStacked=>103, xlConeBarStacked100=>104,
   xlConeCol=>105, xlConeColClustered=>99, xlConeColStacked=>100, 
   xlConeColStacked100=>101, xlCylinderBarClustered=>95, 
   xlCylinderBarStacked=>96, xlCylinderBarStacked100=>97,
   xlCylinderCol=>98, xlCylinderColClustered=>92, xlCylinderColStacked=>93,
   xlCylinderColStacked100=>94, xlDoughnut=>-4120, xlDoughnutExploded=>80,
   xlLineMarkers=>65, xlLineMarkersStacked100=>67, xlLineStacked100=>64,
   xlPieExploded=>69, xlPyramidBarClustered=>109,
   xlPyramidBarStacked100=>111);

# Add the reverse engineered graph values. Names are of my choosing,
# since I dont know the real Excel names.
# $chart_types{"xlLineColumn"}=-4111;# chart doesnt display as expected


#==================== Cleanup =====================================
# This routine quits from Excel & cleans up multiple file saves.
#
# Calling parameters: none
# Returns: Exits the script with the error count.
#==================================================================
sub cleanup {

   # Do one last file save.
   file_save();

   # Quit from Excel without prompts.
   $main::excel->{DisplayAlerts} = "False";
   $main::excel->Quit();

   # Wait for the filesystem to release the file locks.
   # If we dont wait, then the rename command often fails.
   sleep 2;

   # Now we can rename the last saved file.
   my $temp_out = "${main::out_file}${main::file_save_cnt}";
   # print "cleanup temp_out=$temp_out\n";
   my $cmd_rc = rename("$temp_out", "$main::out_file");
   # print "cleanup rename cmd_rc=$cmd_rc\n";
   if ($cmd_rc != 1) {
      print "cleanup ERROR: File rename failed!\n";
      exit 1;
   }

   # Now we cleanup the intermediate temporary saved files, if any.
   for (my $i = 0; $i <= $main::file_save_cnt; $i++) {
      $temp_out = "${main::out_file}${i}";
      if (-e "$temp_out") {
         # print "cleanup erasing temp_out=$temp_out\n";
         unlink("$temp_out");
      }
   }

   # Did we produce any charts?
   if ($main::total == 0) {
      print "\ncleanup ERROR: No charts produced!\n";
      $main::errors++;
   }

   # Get overall time taken
   my $total_sec = time() - $main::overall_start_sec;

   # Thats all!
   print "\n$main::self all done. Total_Charts=$main::total Errors=$main::errors Total_Time=$total_sec seconds \nSee out_file: $main::out_file\n";
   exit $main::errors;
}


#==================== Create_Excel_Chart ==========================
# This routine creates a single Excel chart.
#
# Calling parameters: none
# Returns: OK
#==================================================================
sub create_excel_chart {

   # Add new chart object. In order for the chart object to be accessible
   # after the eval statement, the chart object may not be a local variable.
   eval {$main::chart = $main::excel->Charts()->Add()};
   if (! defined($main::chart)) {
      print "create_excel_chart ERROR: Could not add new chart!\n";
      $main::errors++;
      return "ERROR";
   }

   # While the VB documentation suggests that you can compose a range
   # using the cells method with numeric indices, it wont work for me.
   # So we compose the range in the traditional A1:D4 format using 
   # the indices read from the data block.
   my $c1 = integer_2_letter($main::range_column_start);
   my $c2 = integer_2_letter($main::range_column_end);
   my $range_string = "${c1}${main::range_row_start}:${c2}${main::range_row_end}";
   print "create_excel_chart row/column indices: ($main::range_row_start,$main::range_column_start) ($main::range_row_end,$main::range_column_end) => range_string=$range_string\n";

   # Setup chart range data. 
   # $range_string = "";# test code
   eval {$main::range = $main::stats_sheet->Range("$range_string")};
   if (! defined($main::range)) {
      print "create_excel_chart ERROR: Could not set chart range!\n";
      $main::errors++;
      undef $main::chart;
      return "ERROR";
   }

   # Apply range data to chart. If there is an error here, no status is
   # returned, but the script does not crash.
   $main::chart->SetSourceData({Source => $main::range, PlotBy => $main::xlRows});

   # At this point, we have produced a new basic chart.
   $main::total++;

   # Set chart type as specified by user.
   my $chart_type_value = @main::chart_types{$main::chart_type};
   if (! defined($chart_type_value)) {
      $chart_type_value = 65;# use xlLineMarkers as default
   }
   print "create_excel_chart chart_type=$main::chart_type chart_type_value=$chart_type_value\n";
   $main::chart->{ChartType} = $chart_type_value;

   # We can read back the chart type as shown below.
   # my $data = $chart->ChartType();
   # print "data=$data\n";

   # Add user specified title text to top of chart.
   $main::chart->{HasTitle} = 1;
   $main::chart->ChartTitle->{Text} = "$main::chart_title";

   # Change chart tab name to user specified name string.
   $main::chart->{Name} = "$main::chart_tab_name";
   # $main::chart->{Name} = "$main::chart_type";# all chart type test code

   # Put the legend on the bottom of the chart.
   $main::chart->Legend()->{Position} = $main::xlLegendPositionBottom;

   # Add print footer data to chart
   $main::chart->PageSetup()->{LeftFooter} = "From: $main::email";
   $main::chart->PageSetup()->{CenterFooter} = "Confidential $main::company_name";
   my $date_time = localtime();
   $main::chart->PageSetup()->{RightFooter} = "$date_time";

   # Controlling chart properties
   # $main::chart->Axes(2)->{HasMajorGridlines} = 1;
   # $main::chart->Axes(2)->{HasMinorGridlines} = 1;
   # $main::chart->Axes(2)->{MajorUnit} = 1/4;
   # $main::chart->Axes(2)->{MinorUnit} = 1/8;

   # Trash the chart & range objects
   undef $main::chart;
   undef $main::range;
   return "OK";
}


#==================== File_Save ===================================
# This routine saves the active workbook in XLS format.
#
# Calling parameters: none
# Returns: OK, ERROR
#==================================================================
sub file_save {

   # The Save method seems to always want to stick a "1" at the
   # end of the filename and put the new file in "My Documents"
   # folder. Next time you do a save, it doesnt want to overwrite
   # the exsisting file.

   # To workaround this mess, we do a "SaveAs" and then rename
   # the resulting file. Unfortunately, Excel keeps the newly
   # saved file locked until Excel quits. So we have to do
   # the file rename at the very end as part of the cleanup
   # routine.

   # Setup the next temporary saveas filename in sequence.
   $main::file_save_cnt++;
   my $temp_out = "${main::out_file}${main::file_save_cnt}";
   # print "file_save temp_out=$temp_out\n";
   if (-e "$temp_out") {
      # print "file_save erasing existing file=$temp_out\n";
      unlink("$temp_out");
   }

   # Save changes to the existing workbook, specifying the
   # file format. On the first save, the CSV file is converted
   # to normal XLS format. 
   # NB: dont put quotes around the format parameter!
   # $temp_out = "e:\ccc.ddd";# test code
   my $cmd_rc = $main::book->SaveAs("$temp_out", $main::file_format_xls);
   # print "file_save cmd_rc=$cmd_rc\n";
   if ($cmd_rc == 1) {
      return "OK";
   } else {
      print "file_save ERROR: could not save file: $temp_out\n";
      $main::errors++;
      return "ERROR";
   }
}


#==================== Help ========================================
# This routine provides online help information. 
#
# Calling parameters: none
# Returns: OK or exits the script.
#==================================================================
sub help {

   # Get script name
   $main::self = basename($0);

   # Check if help was requested.
   my $x = lc($ARGV[0]);
   if (!($x =~ m/-h/) && !($x =~ m/\/\?/)) {
      return OK;
   }

   # Give help info.
   print "Basic usage: perl $main::self [in_file.csv] <in_file.txt>\n";
   print "\n";
   print "The input CSV file is formatted in blocks of data, 1 block per desired chart.\n";
   print "\n";
   print "Each chart data block is expected to have header lines showing the chart\n";
   print "chart title, tab name and chart type. The next row of data is used\n";
   print "as the chart X-axis labels. Each row of data after that is used for the\n";
   print "chart lines. Finally, the end of the chart data block is a row of \"========\".\n";
   print "\n";
   print "You can have as many chart data blocks in the file as you need.\n";
   print "\n";
   print "If you supply the optional second in_file.txt file, the contents will be \n";
   print "copied to a separate worksheet in the output file, with no other processing.\n";
   exit 1;
}


#==================== Import_Raw_Bug_Data =========================
# This routine imports the optional raw bug .txt file into a 
# seperate worksheet.
#
# Calling parameters: none
# Returns: OK, ERROR
#==================================================================
sub import_raw_bug_data {

   # Record start time.
   my $start_sec = time();

   # Look for optional TXT file. If not present, we are done here.
   if ($main::txt_file eq "") {
      return "OK";
   }

   # Open the TXT file for reading.
   my $cmd_rc = open (TABTXT, "<$main::txt_file");
   if ($cmd_rc != 1) {
      print "import_raw_bug_data ERROR: can not open file: $main::txt_file";
      $main::errors++;
      return "ERROR";
   }
   print "import_raw_bug_data starting file: $main::txt_file ...\n";

   # Add a new worksheet and rename it.
   eval {$main::bugs_sheet = $main::book->Worksheets()->Add()};
   if (!defined($main::bugs_sheet)) {
      print "import_raw_bug_data ERROR: Could not add worksheet!\n";
      $main::errors++;
      return "ERROR";
   }
   $main::bugs_sheet->{Name} = 'Raw Bug Data';

   # Remove the Excel application from view. This speeds up the processing
   # here by a factor of 3. If Excel is visible, every time we write to a
   # cell, Excel will refresh its screen display, which is very CPU intensive.
   # Even when we write to cells that are offscreen, the right hand scroll
   # bar is resized, causing a screen refresh.
   $main::excel->{Visible} = 0;

   # Insert header row title text.
   my $bugs_row_cnt = 1;
   $main::bugs_sheet->Cells($bugs_row_cnt,1)->{Value} = "Number";
   $main::bugs_sheet->Cells($bugs_row_cnt,2)->{Value} = "Category";
   $main::bugs_sheet->Cells($bugs_row_cnt,3)->{Value} = "Synopsis";
   $main::bugs_sheet->Cells($bugs_row_cnt,4)->{Value} = "Priority";
   $main::bugs_sheet->Cells($bugs_row_cnt,5)->{Value} = "Responsible";
   $main::bugs_sheet->Cells($bugs_row_cnt,6)->{Value} = "State";
   $main::bugs_sheet->Cells($bugs_row_cnt,7)->{Value} = "Arrival-Date";
   $main::bugs_sheet->Cells($bugs_row_cnt,8)->{Value} = "Closed-Date";
   $main::bugs_sheet->Cells($bugs_row_cnt,9)->{Value} = "Release";
   $main::bugs_sheet->Cells($bugs_row_cnt,10)->{Value} = "Fix-for";
   $main::bugs_sheet->Cells($bugs_row_cnt,11)->{Value} = "Fixed-in";
   $main::bugs_sheet->Cells($bugs_row_cnt,12)->{Value} = "Class";
   $main::bugs_sheet->Cells($bugs_row_cnt,13)->{Value} = "Reporter";

   # Resize the columns.
   $main::bugs_sheet->Columns("A:A")->{ColumnWidth} = 8;# number
   $main::bugs_sheet->Columns("B:B")->{ColumnWidth} = 9;# category
   $main::bugs_sheet->Columns("C:C")->{ColumnWidth} = 80;# synopsis
   $main::bugs_sheet->Columns("D:D")->{ColumnWidth} = 10;# priority
   $main::bugs_sheet->Columns("E:E")->{ColumnWidth} = 12;# responsible
   $main::bugs_sheet->Columns("F:F")->{ColumnWidth} = 10;# state
   $main::bugs_sheet->Columns("G:G")->{ColumnWidth} = 22;# arrival-date
   $main::bugs_sheet->Columns("H:H")->{ColumnWidth} = 22;# closed-date
   $main::bugs_sheet->Columns("I:I")->{ColumnWidth} = 20;# release
   $main::bugs_sheet->Columns("J:J")->{ColumnWidth} = 20;# fix-for
   $main::bugs_sheet->Columns("K:K")->{ColumnWidth} = 20;# fixed-in
   $main::bugs_sheet->Columns("L:L")->{ColumnWidth} = 10;# class
   $main::bugs_sheet->Columns("M:M")->{ColumnWidth} = 12;# reporter

   # Make header row font bold & underlined.
   $main::bugs_sheet->Rows("$bugs_row_cnt:$bugs_row_cnt")->Font->{Bold} = 1;
   $main::bugs_sheet->Rows("$bugs_row_cnt:$bugs_row_cnt")->Font->{Underline} = $xlUnderlineStyleSingle;

   # Add print footer data to worksheet.
   $main::bugs_sheet->PageSetup()->{LeftFooter} = "From: $main::email";
   $main::bugs_sheet->PageSetup()->{CenterFooter} = "Confidential $main::company_name";
   my $date_time = localtime();
   $main::bugs_sheet->PageSetup()->{RightFooter} = "$date_time";

   # Copy the raw bug data from the TXT file 1 line at a time.
   my $total_bug_cnt = 0;
   my $file_line_cnt = 0;
   while ( 1 ) {

      # Look for normal EOF.
      if (eof(TABTXT)) {
         print "import_raw_bug_data file: $main::txt_file hit normal EOF at file_line_cnt=$file_line_cnt\n";
         last;
      }

      # Put out progress indicator every 200 lines.
      $file_line_cnt++;
      my $prog_line = (int($file_line_cnt / 200)) * 200;
      if ($prog_line == $file_line_cnt) {
         print "import_raw_bug_data file_line_cnt=$file_line_cnt ...\n";
      }

      # Get next line from file and split by tabs.
      my $line = <TABTXT>;
      chomp($line);
      $line =~ s/^\s*//g;# remove leading whitespace only!
      my @line_array = split(/\011/,$line);
      # print "file_line_cnt=$file_line_cnt size_line_array=$#line_array line=$line\n";

      # Filter out blank lines.
      if ($#line_array < 0 ) {
         # print "skipping file_line_cnt=$file_line_cnt line=$line\n";
         next;
      }

      # Watch for large files that exceed Excel worksheet row limit.
      $bugs_row_cnt++;
      if ($bugs_row_cnt > $main::worksheet_max_row) {
         # Put error message at top of worksheet where it will be visible.
         $main::bugs_sheet->Cells(1,1)->{Value} = "ERROR: Could not import all raw bug data!";
         $main::bugs_sheet->Cells(1,1)->Font->{Bold} = 1;
         $main::bugs_sheet->Cells(1,1)->Font->{ColorIndex} = 3;# Red
         $main::bugs_sheet->Cells(1,2)->{Value} = "";
         $main::bugs_sheet->Cells(1,3)->{Value} = "";
         print "import_raw_bug_data ERROR: exceeded Excel worksheet_max_row=$main::worksheet_max_row at file_line_cnt=$file_line_cnt line=$line\n";
         $main::errors++;
         last;
      }

      # Write the line to the bugs worksheet.
      # NB: Although PERL arrays start at index 0, Excel rows & columns start at 1!
      $total_bug_cnt++;
      for (my $i = 0; $i <= $#line_array; $i++) {
         my $temp = $line_array[$i];
         my $j = $i + 1;# Excel columns start at 1, not 0 !
         # print "bugs_row_cnt=$bugs_row_cnt j=$j temp=$temp\n";
         $main::bugs_sheet->Cells($bugs_row_cnt,$j)->{Value} = "$temp";
      }
   }

   # Show the Excel application on screen again.
   $main::excel->{Visible} = 1;
   undef $main::bugs_sheet;

   # Compute total time taken for bug data import.
   my $total_sec = time() - $start_sec;

   # Did we get any bug data?
   if ($total_bug_cnt > 0) {
      print "import_raw_bug_data done, imported $total_bug_cnt bugs in $total_sec seconds.\n";
      return "OK";
   } else {
      print "import_raw_bug_data ERROR: No bugs imported in $total_sec seconds!\n";
      $main::errors++;
      return "ERROR";
   }
}


#==================== Integer_2_Letter ============================
# This routine converts an integer to its letter equivalent value.
# Useful in dealing with Excel worksheet columns by their letter
# index.
#
# Calling parameters: integer in range 1 - 256
# Returns: letter equivalent
#==================================================================
sub integer_2_letter {

   # Get the required calling token.
   if ($#_ < 0) {
      print "integer_2_letter ERROR: missing calling integer!\n";
      $main::errors++;
      return "ERROR";
   }
   my $integer = "$_[0]";

   # Convert number to integer and check range.
   $integer = int($integer);
   if ($integer < 1 || $integer > 256) {
      print "integer_2_letter ERROR: integer=$integer must be in range 1 - 256\n";
      $main::errors++;
      return "ERROR";
   }

   # Set up list of capital letters.
   my @letters_array = (A .. Z);
   # print "letters_array=@letters_array\n";
   
   # For integers 1 - 26, we return a single letter.
   my $result = "";
   if ($integer <= 26) {
      $result = $letters_array[$integer - 1];
      # print "integer=$integer result=$result\n";
      return $result;
   }

   # For integers > 26, get modulus & multiple
   my $base = 26;
   my $multiple = int($integer/$base);
   my $modulus = $integer % $base;

   # For transition to next mulitple of 26, need to adjust the variables.
   if ($modulus == 0) {
      $modulus = 26;
      $multiple--;
   }

   # We return two letters. 
   # print "\nmultiple=$multiple modulus=$modulus base=$base\n";
   my $result1 = $letters_array[$multiple - 1];
   my $result2 = $letters_array[$modulus - 1];
   $result = "${result1}${result2}";
   # print "integer=$integer result=$result\n";
   return $result;
}


#==================== Miscellaneous_Formatting ====================
# This routine does odds 'n ends formatting at the end of the 
# report creation.
#
# Calling parameters: none
# Returns: OK
#==================================================================
sub miscellaneous_formatting {

   # Resize the columns in the CSV stats sheet
   $main::stats_sheet->Columns("A:A")->{ColumnWidth} = 20;
   $main::stats_sheet->Columns("B:IV")->{ColumnWidth} = 10;

   # Activate the first chart we produced. This will be the first
   # thing the user sees when the report file is first opened.
   if ($main::total > 0) {
      my $chart1 = $book->Charts(1); 
      $chart1->Activate();
   }
   return "OK";
}


#==================== Read Chart Data =============================
# This routine reads a block of chart data from the CSV worksheet.
#
# EOF is returned if we go past the worksheet maximum row number. 
# As a means of speeding up the program, we also return EOF if we
# get too many consectutive blank rows.
#
# Calling parameters: none
# Returns: OK, ERROR, EOF
# A variety of global variables are set up with chart related data.
#==================================================================
sub read_chart_data_block {

   # Initialize variables.
   my $cell_data = "";
   $main::chart_tab_name = "";
   $main::chart_title = "";
   $main::chart_type = "";
   $main::range_column_start = 0;
   $main::range_column_end = 0;
   $main::range_row_start = 0;
   $main::range_row_end = 0;

   # Activate the CSV data worksheet.
   $main::stats_sheet->Activate();

   # Look for Chart Title, which is the start of a chart data block.
   my $blank_cnt = 0;
   while ( 1 ) {

      # Goto next row of CSV worksheet. This is the normal end worksheet / EOF exit.
      $main::csv_row_cnt++;
      if ($main::csv_row_cnt > $main::worksheet_max_row || $blank_cnt >= $main::max_blank_rows) {
         print "read_chart_data looking for chart title, hit normal EOF at csv_row_cnt=$main::csv_row_cnt blank_cnt=$blank_cnt\n";
         return "EOF";# This is not an error!
      }

      # Get data from column 1 cells.
      $cell_data = $main::stats_sheet->Cells($main::csv_row_cnt,1)->{Value};
      if (! defined($cell_data)) {
            $cell_data = "";
      }
      # print "csv_row_cnt=$main::csv_row_cnt cell_data=$cell_data\n"; 

      # Keep track of consecutive blank rows. There may be data in the
      # CSV file that is not intended to be charted. We need to plough
      # through all this data in case there are more charts later on.
      # $cell_data = "";# test code.
      if ($cell_data eq "") {
         $blank_cnt++;
      } else {
         $blank_cnt = 0;
      }

      # Look for chart title.
      if ($cell_data =~ m/chart title:/i) {
         $cell_data =~ m/: (.*)/;
         if (defined($1)) {
            $main::chart_title = $1;
         }
         # print "read_chart_data_block csv_row_cnt=$main::csv_row_cnt chart_title=$main::chart_title\n";
         last;
      }
   }

   # Look for chart tab name.
   $blank_cnt = 0;
   while ( 1 ) {

      # Goto next row of CSV worksheet. EOF here is an error.
      $main::csv_row_cnt++;
      if ($main::csv_row_cnt > $main::worksheet_max_row || $blank_cnt >= $main::max_blank_rows) {
         print "read_chart_data ERROR: looking for chart tab name, hit EOF at csv_row_cnt=$main::csv_row_cnt blank_cnt=$blank_cnt\n";
         $main::errors++;
         return "EOF";
      }

      # Get data from column 1 cells.
      $cell_data = $main::stats_sheet->Cells($main::csv_row_cnt,1)->{Value};
      if (! defined($cell_data)) {
            $cell_data = "";
      }
      # print "csv_row_cnt=$main::csv_row_cnt cell_data=$cell_data\n"; 

      # Keep track of consecutive blank rows.
      # $cell_data = "";# test code.
      if ($cell_data eq "") {
         $blank_cnt++;
      } else {
         $blank_cnt = 0;
      }

      # Look for chart tab name.
      if ($cell_data =~ m/chart tab name:/i) {
         $cell_data =~ m/: (.*)/;
         if (defined($1)) {
            $main::chart_tab_name = $1;
         }
         # print "read_chart_data_block csv_row_cnt=$main::csv_row_cnt chart_tab_name=$main::chart_tab_name\n";
         last;
      }
   }

   # Look for chart type.
   $blank_cnt = 0;
   while ( 1 ) {

      # Goto next row of CSV worksheet. EOF here is an error.
      $main::csv_row_cnt++;
      if ($main::csv_row_cnt > $main::worksheet_max_row || $blank_cnt >= $main::max_blank_rows) {
         print "read_chart_data ERROR: looking for chart type, hit EOF at csv_row_cnt=$main::csv_row_cnt blank_cnt=$blank_cnt\n";
         $main::errors++;
         return "EOF";
      }

      # Get data from column 1 cells.
      $cell_data = $main::stats_sheet->Cells($main::csv_row_cnt,1)->{Value};
      if (! defined($cell_data)) {
            $cell_data = "";
      }
      # print "csv_row_cnt=$main::csv_row_cnt cell_data=$cell_data\n"; 

      # Keep track of consecutive blank rows.
      # $cell_data = "";# test code.
      if ($cell_data eq "") {
         $blank_cnt++;
      } else {
         $blank_cnt = 0;
      }

      # Look for chart type.
      if ($cell_data =~ m/chart type:/i) {
         $cell_data =~ m/: (.*)/;
         if (defined($1)) {
            $main::chart_type = $1;
         }
         # print "read_chart_data_block csv_row_cnt=$main::csv_row_cnt chart_type=$main::chart_type\n";
         last;
      }
   }

   # The next non-blank line is the start of the chart data range.
   $blank_cnt = 0;
   while ( 1 ) {

      # Goto next row of CSV worksheet. EOF here is an error.
      $main::csv_row_cnt++;
      if ($main::csv_row_cnt > $main::worksheet_max_row || $blank_cnt >= $main::max_blank_rows) {
         print "read_chart_data ERROR: looking for start of chart data, hit EOF at csv_row_cnt=$main::csv_row_cnt blank_cnt=$blank_cnt\n";
         $main::errors++;
         return "EOF";
      }

      # Get data from column 1 cells.
      $cell_data = $main::stats_sheet->Cells($main::csv_row_cnt,1)->{Value};
      if (! defined($cell_data)) {
            $cell_data = "";
      }
      # print "csv_row_cnt=$main::csv_row_cnt cell_data=$cell_data\n"; 

      # Save start of chart data range.
      # $cell_data = "";# test code.
      if ($cell_data eq "") {
         $blank_cnt++;
      } else {
         $main::range_row_start = $main::csv_row_cnt;
         $main::range_row_end = $main::csv_row_cnt;
         $main::range_column_start = 1;
         # print "read_chart_data_block csv_row_cnt=$main::csv_row_cnt range_row_start=$main::range_row_start range_column_start=$main::range_column_start\n";
         last;
      }
   }

   # Now we scan the current row to find the end column of the chart data range.
   for (my $i = 1; $i <= $main::worksheet_max_column; $i++) {

      # Get data from current row cells.
      $cell_data = $main::stats_sheet->Cells($main::csv_row_cnt,$i)->{Value};
      if (! defined($cell_data)) {
            $cell_data = "";
      }
      # print "csv_row_cnt=$main::csv_row_cnt column=$i cell_data=$cell_data\n";

      # First blank cell indicates the end of the data range columns.
      if ($cell_data eq "") {
         last;
      } else {
         $main::range_column_end = $i;
      }
   }
   # print "read_chart_data_block csv_row_cnt=$main::csv_row_cnt range_column_end=$main::range_column_end\n";

   # Now find the rest of the chart data range. There should be a minimum of 2 rows
   # of chart data, one for the x-axis labels and minimum one for the chart itself.
   $blank_cnt = 0;
   while ( 1 ) {

       # Goto next row of CSV worksheet. EOF here is an error.
      $main::csv_row_cnt++;
      if ($main::csv_row_cnt > $main::worksheet_max_row || $blank_cnt >= $main::max_blank_rows) {
         print "read_chart_data ERROR: looking for end of chart data, hit EOF at csv_row_cnt=$main::csv_row_cnt blank_cnt=$blank_cnt\n";
         $main::errors++;
         return "EOF";
      }

      # Get data from column 1 cells.
      $cell_data = $main::stats_sheet->Cells($main::csv_row_cnt,1)->{Value};
      if (! defined($cell_data)) {
            $cell_data = "";
      }

      # Keep track of consecutive blank rows.
      # $cell_data = "";# test code.
      if ($cell_data eq "") {
         $blank_cnt++;
      } else {
         $blank_cnt = 0;
      }
      # print "csv_row_cnt=$main::csv_row_cnt cell_data=$cell_data\n"; 

      # Line with many equal signs signals end of chart data block.
      if ($cell_data =~ m/========/) {
         last;
      }

      # Save last non-blank row as part of the chart data range.
      if ($cell_data ne "") {
         $main::range_row_end = $main::csv_row_cnt;
      }
   }
   # print "read_chart_data_block csv_row_cnt=$main::csv_row_cnt range_row_end=$main::range_row_end\n";

   # Sanity checks on chart data block.
   if ($main::chart_title eq "") {
      print "read_chart_data_block ERROR: missing chart_title!\n";
      $main::errors++;
      return "ERROR";
   }
   if ($main::chart_tab_name eq "") {
      print "read_chart_data_block ERROR: missing chart_tab_name!\n";
      $main::errors++;
      return "ERROR";
   }
   if ($main::chart_type eq "") {
      print "read_chart_data_block ERROR: missing chart_type!\n";
      $main::errors++;
      return "ERROR";
   }
   if ($main::range_row_start == 0 || $main::range_row_end == 0 || $main::range_column_start == 0 || $main::range_column_end == 0) {
      print "read_chart_data_block ERROR: missing chart range data ($main::range_row_start,$main::range_column_start : $main::range_row_end,$main::range_column_end)\n";
      $main::errors++;
      return "ERROR";
   }
   if ($main::range_row_start == $main::range_row_end) {
      print "read_chart_data_block WARNING: found only 1 row of chart data ($main::range_row_start,$main::range_column_start : $main::range_row_end,$main::range_column_end)\n";
      return "WARNING";
   }
   if ($main::range_column_start == $main::range_column_end) {
      print "read_chart_data_block WARNING: found only 1 column of chart data ($main::range_row_start,$main::range_column_start : $main::range_row_end,$main::range_column_end)\n";
      return "WARNING"
   }

   # We have a valid chart data block.
   print "read_chart_data_block found valid chart data block, csv_row_cnt=$main::csv_row_cnt\n";
   return "OK";
}

#==================== Setup =======================================
# This routine does general setup. 
#
# Calling parameters: none
# Returns: OK, exits the script on error 
#==================================================================
sub setup {

   # Initialize stats counters.
   $main::csv_row_cnt = 0;
   $main::errors = 0;
   $main::file_save_cnt = 0;
   $main::total = 0;

   # Get script name
   $main::self = basename($0);

   # Check for required calling tokens.
   if ($#ARGV < 0) {
      print "setup ERROR: Missing calling parameters! \nFor more info, type: perl $main::self -h\n";
      exit 1;
   }

   # Get the present working directory.
   $main::pwd = cwd();

   # Although PERL likes it path names with forward slashes,
   # when you call a Microsoft method, it expects pathnames
   # with backslashes. So we convert.
   $main::pwd =~ s/\//\\/g;
   # print "pwd=$main::pwd\n";

   # Get the required input CSV file name.
   $main::csv_file = $ARGV[0];

   # Check input file type is csv. Watch out for file names that have
   # multiple periods in them.
   my @temp = split(/\./,"$main::csv_file");
   $ft=$temp[$#temp];# get last element of array
   # print "csv_file=$csv_file ft=$ft\n";
   if (lc($ft) ne "csv") {
      print "setup ERROR: Input file type must be csv, not $ft\n";
      exit 1;
   }

   # Make sure we have full pathname. The Microsoft methods use "My Documents"
   # as the default directory for relative filenames, which is not helpful.
   # We have to be running on Windows, so we can assume pathnames start with
   # c: or d:
   if (!("$main::csv_file" =~ m/^[cCdD]:/)) {
      $main::csv_file = "${main::pwd}\\${main::csv_file}";# add pwd
   }
   # print "csv_file=$main::csv_file\n";

   # Check input CSV file exists.
   if (! -e "$main::csv_file") {
      print "setup ERROR: Input CSV file $main::csv_file not found!\n";
      exit 1;
   }

   # Look for optional second input TXT file.
   $main::txt_file = "";
   if ($#ARGV > 0) {
      $main::txt_file = $ARGV[1];

      # Check input file type is txt.
      @temp = split(/\./,"$main::txt_file");
      $ft=$temp[$#temp];# get last element of array
      # print "txt_file=$txt_file ft=$ft\n";
      if (lc($ft) ne "txt") {
         print "setup ERROR: Optional input file type must be txt, not $ft\n";
         exit 1;
      }

      # Make sure we have full pathname.
      if (!("$main::txt_file" =~ m/^[cCdD]:/)) {
         $main::txt_file = "${main::pwd}\\${main::txt_file}";# add pwd
      }
      # print "txt_file=$main::txt_file\n";

      # Check input TXT file exists.
      if (! -e "$main::txt_file") {
         print "setup ERROR: Optional input TXT file $main::txt_file not found!\n";
         exit 1;
      }
   }

   # Create output file pathname from input CSV pathname.
   $main::out_file = $main::csv_file;
   $main::out_file =~ s/.csv$/.xls/i;
   # print "out_file=$main::out_file\n";

   # Get the current username, define the users email.
   $main::username = "unknown";
   if (defined($ENV{USERNAME})) {
      $main::username = $ENV{USERNAME};
   }
   $main::email = "${main::username}\@${main::company_name}";
   # print "username=$main::username email=$main::email\n";

   # See if Excel is already running.
   eval {$main::excel = Win32::OLE->GetActiveObject('Excel.Application')};
   if ($@ ne "") {
      print "setup ERROR: Excel is not installed! $@\n";
      exit 1;
   }
   if (defined $main::excel) {
      print "setup ERROR: Excel is already running! \nPlease save your current work and quit from Excel.\n";
      exit 1;
   }

   # Start up the Excel application.
   # print "Starting up Excel...\n";
   $main::excel = Win32::OLE->new('Excel.Application');
   if (! defined $main::excel) {
      print "setup ERROR: Sorry, cannot start Excel!\n";
      exit 1;
   }
   # print "excel=$main::excel\n";

   # Show the Excel application on screen.
   $main::excel->{Visible} = 1;

   # Open the CSV input file, with no alerts allowed.
   # $main::csv_file = "aaa.csv";# test code
   $main::excel->{DisplayAlerts} = "False";
   eval {$main::book = $main::excel->Workbooks->Add($main::csv_file)};
   if (!defined($main::book)) {
      print "setup ERROR: Could not open file: $main::csv_file\n";
      exit 1;
   }
   $main::excel->{DisplayAlerts} = "True";
   # print "book=$main::book\n";

   # Rename the CSV stats tab.
   eval {$main::stats_sheet = $main::book->Worksheets(1)};
   if (!defined($main::stats_sheet)) {
      print "setup ERROR: Could not access stats_sheet!\n";
      exit 1;
   }
   $main::stats_sheet->{Name} = 'Raw Statistics';
   # print "stats_sheet=$main::stats_sheet\n";

   # Add print footer data to worksheet.
   $main::stats_sheet->PageSetup()->{LeftFooter} = "From: $main::email";
   $main::stats_sheet->PageSetup()->{CenterFooter} = "Confidential $main::company_name";
   my $date_time = localtime();
   $main::stats_sheet->PageSetup()->{RightFooter} = "$date_time";

   # Code to keep perl warnings quiet.
   my $temp = $main::xlColumns;
   $temp = $main::xlRows;

   # Setup is done.
   print "setup complete.\n";
   return "OK";
}


#==================== Main Program ================================
# This is the main program for creating Excel charts.
#==================================================================

# Record overall start time
$overall_start_sec = time();

# Give help if necessary.
help();

# Basic setup.
setup();
file_save();

# Test code for integer_2_letter routine.
# integer_2_letter();# no parms test
# for ($i = 0; $i <= 256; $i++) {
#    $x = integer_2_letter($i);
#    print "i=$i x=$x\n";
# }

# Main processing loop.
while ( 1 ) {

   # Get next block of chart data.
   $resp = read_chart_data_block();

   # If valid chart data block found, create Excel chart.
   if ($resp eq "OK") {
      create_excel_chart();
      file_save();
      # last;# all chart type test code 
   }

   # Exit loop at end of input file.
   if ($resp eq "EOF") {
      last;
   }
}

# Test code to walk through all known chart types.
# NB: Must exit main loop above with valid data loaded
# in memory. Dont hit EOF, or all data reset!
# @key_list=keys(%chart_types);
# @key_list=sort(@key_list);
# foreach $chart_type (@key_list) {
#   create_excel_chart();
# }

# Process the optional second input file, if any.
import_raw_bug_data();
file_save();

# Do miscellaneous formatting
miscellaneous_formatting();

# Cleanup routine.
cleanup();


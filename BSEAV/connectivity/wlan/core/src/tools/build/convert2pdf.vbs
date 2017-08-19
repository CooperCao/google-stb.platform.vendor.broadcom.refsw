Rem 
Rem VB script to convert Microsoft document formats to Adobe .pdf format
Rem given by brcm helpdesk
Rem 
Rem It converts files (word, power point, excel, project, visio, rtf, txt,
Rem html, htm, vbs, cpp, h, jpg, jpeg, jpe, bmp, dib, jfif, gif, tif, tiff,
Rem png) natively to PDF.
Rem
Rem Usage: <this_script> "<source_directory_with_microsoft_docs>" "<dest_dir>"
Rem        e.g: copy myword.doc c:\temp\conv2pdf\source
Rem             <this_script> "c:\temp\conv2pdf\source" "c:\temp\conv2pdf\dest"
Rem 
Rem To use this script microsoft office should be installed locally and 
Rem Adobe PDF printer should be visible in "Start->Printers and Faxes", 
Rem ensure that "Start->Printers and Faxes->Adobe Pdf Printer (rightclick)
Rem ->Printing Preferences-> View Adobe PDF Results" property is unchecked
Rem
Rem $Id: convert2pdf.vbs,v 1.2 2006-03-24 22:59:46 prakashd Exp $
Rem 

Set WSHShell = CreateObject("WScript.Shell")
spath=wscript.arguments(0)
dpath=wscript.arguments(1)
Dim fs: Set fs = CreateObject("Scripting.FileSystemObject")
Set source= fs.GetFolder(spath)
Set allfiles= source.Files
If fs.FolderExists(dpath) Then
Else
	fs.CreateFolder(dpath)
End If
counter=0
For Each fl in allfiles
	extension=Right(fl.Name,3)
	If extension = "jpg" or extension = "bmp" or extension = "dib" or extension = "jpeg" or extension = "jpe" or extension = "jfif" or extension = "gif" or extension = "tif" or extension = "tiff" or extension = "png" Then
		imgextension=extension
		extension="image"
	End If
		Select Case extension
			Case "pdf"
			Case "ppt"
				reg0="HKCU\SOFTWARE\Adobe\Acrobat Distiller\PrinterJobControl\POWERPNT.EXE\"
				WSHShell.RegWrite reg0 , dpath & "\" & fl.Name & ".pdf"
				Set ppnt = CreateObject("PowerPoint.Application")
				set PP= ppnt.Presentations.Open(spath & "\" & fl.Name,-1,0,0)
				set options=PP.PrintOptions
				oldprinter=options.ActivePrinter
				options.ActivePrinter = "Adobe PDF"
				options.PrintInBackground = 0
				PP.PrintOut 1,9999,"",1,0
				PP.Saved=1
				options.ActivePrinter =oldprinter
				PP.Close
				ppnt.quit
				counter = counter+1

			Case "xls"
				reg0="HKCU\SOFTWARE\Adobe\Acrobat Distiller\PrinterJobControl\EXCEL.EXE\"
				WSHShell.RegWrite reg0 , dpath & "\" & fl.Name & ".pdf"
				Set xls = CreateObject("Excel.Application")
				set xl= xls.Workbooks.Open(spath & "\" & fl.Name,True,,,,True,,,False,False,,False)
				xl.Activate
				xl.PrintOut 1,9999,1,False,"Adobe PDF",False,False
				xl.Close 1
				xls.quit
				counter = counter+1
			Case "vsd"
				reg0="HKCU\SOFTWARE\Adobe\Acrobat Distiller\PrinterJobControl\VISIO.EXE\"
				WSHShell.RegWrite reg0 , dpath & "\" & fl.Name & ".pdf"
				Set vsd = CreateObject("Visio.Application")
				vsd.Visible=false
				oldprinter=vsd.ActivePrinter
				vsd.ActivePrinter = "Adobe PDF"
				vsd.Documents.Open spath & "\" & fl.Name
				vsd.ActiveDocument.Print()
				vsd.ActiveDocument.Close
				vsd.ActivePrinter=oldprinter
				vsd.quit
				counter = counter+1
			Case "mpp"
				reg0="HKCU\SOFTWARE\Adobe\Acrobat Distiller\PrinterJobControl\WINPROJ.EXE\"
				WSHShell.RegWrite reg0 , dpath & "\" & fl.Name & ".pdf"
				set mpp = CreateObject("MSProject.Application")
				mpp.Visible= False
				mpp.FileOpen spath & "\" & fl.Name
				mpp.FilePrintSetup "Adobe PDF"
				mpp.FilePrint 1,9999,,,,,,,0,,0
				mpp.FileClose 0
				mpp.quit
				counter=counter+1
			Case "image"
				htmlfile=spath&"\"&Left(fl.Name,Len(fl.Name)-4)
				Set textstream=fs.CreateTextFile(htmlfile&".html")
				textstream.write "<HTML><img src=" & spath & "\" & fl.Name & "></HTML>"
				textstream.close
				reg0="HKCU\SOFTWARE\Adobe\Acrobat Distiller\PrinterJobControl\WINWORD.EXE\"
				WSHShell.RegWrite reg0 , dpath & "\" & fl.Name & ".pdf"
				Set word = CreateObject("Word.Application")
				oldprinter=word.ActivePrinter
				word.ActivePrinter = "Adobe PDF"
				word.Documents.Open htmlfile & ".html"
				word.WordBasic.FilePrint
				word.WordBasic.FileClose 2
				word.ActivePrinter =oldprinter
				word.quit
				fs.deletefile htmlfile & ".html"
				counter=counter+1 
			
			Case Else
				reg0="HKCU\SOFTWARE\Adobe\Acrobat Distiller\PrinterJobControl\WINWORD.EXE\"
				WSHShell.RegWrite reg0 , dpath & "\" & fl.Name & ".pdf"
				Set word = CreateObject("Word.Application")
				oldprinter=word.ActivePrinter
				word.ActivePrinter = "Adobe PDF"
				word.Documents.Open spath & "\" & fl.Name
				word.WordBasic.FilePrint
				word.WordBasic.FileClose 2
				word.ActivePrinter =oldprinter
				word.quit
				counter = counter+1
		End Select
Next

Rem MsgBox "Conversion finished. " & counter & " files converted" & vbCRLF & "Source Folder: " & spath & vbCRLF & "Destination Folder: " & dpath

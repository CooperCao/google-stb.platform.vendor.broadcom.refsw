#include <Windows.h>
#include <stdio.h>

int main( int argc, char** argv )
	{
	IMAGE_DOS_HEADER dosHeader;
	DWORD peSignature;
	IMAGE_FILE_HEADER peHeader;
	IMAGE_OPTIONAL_HEADER peOptionHeader;
	IMAGE_SECTION_HEADER peSection;
	IMAGE_IMPORT_DESCRIPTOR* importDescriptor;
	unsigned int i, importTableRVA, importTableSize, dllOffset;
	char dllName[256];

	char* iData = NULL;
	FILE* file = NULL;
	int result = 1;

	if( argc != 4 )
		{
		fprintf( stderr, "Renames DLL imports.\n"
				 "Usage: '%s <Exe to edit> <Current DLL import name> <New DLL import Name>", argv[0]);
		goto quit;
		}
	if( strlen( argv[2] ) < strlen( argv[3] ) )
		{
		fprintf( stderr, "The new DLL name must be smaller than, or equal to the existing name in length." );
		goto quit;
		}	
	file = fopen( argv[1], "r+b" );
	if( NULL == file )
		{
		fprintf( stderr, "Failed to open \"%s\" with r+b\n", argv[1] );
		goto quit;
		}

	if( 1 != fread( &dosHeader, sizeof(IMAGE_DOS_HEADER), 1, file )	||
		fseek( file, dosHeader.e_lfanew, SEEK_SET )					||
		1 != fread( &peSignature, sizeof(DWORD), 1, file )			||
		1 != fread( &peHeader, sizeof(IMAGE_FILE_HEADER), 1, file )	||
		1 != fread( &peOptionHeader, sizeof(IMAGE_OPTIONAL_HEADER), 1, file ) )
		{
		fprintf( stderr, "Read error.\n" );
		goto quit;
		}

	if( IMAGE_DOS_SIGNATURE != dosHeader.e_magic ||
		IMAGE_NT_SIGNATURE != peSignature )
		{
		fprintf( stderr, "Invalid PE image.\n" );
		goto quit;
		}

	importTableRVA = peOptionHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	importTableSize = peOptionHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;

	/* Iterate through section table */
	for( i=0; i<peHeader.NumberOfSections; i++ )
		{
		if( 1 != fread( &peSection, sizeof(IMAGE_SECTION_HEADER), 1, file ) )
			{
			fprintf( stderr, "Read error.\n" );
			goto quit;
			}
		if( peSection.VirtualAddress <= importTableRVA && importTableRVA < peSection.VirtualAddress+peSection.SizeOfRawData )
			break;
		}
	if( i == peHeader.NumberOfSections )
		{
		fprintf( stderr, "Can't find the section containing the import table." );
		goto quit;
		}

	iData = (char*)malloc( peSection.SizeOfRawData );
	if( NULL == iData )
		{
		fprintf( stderr, "Out of memory\n" );
		goto quit;
		}

	/* Seek to and read section containing iData */
	if( fseek( file, peSection.PointerToRawData, SEEK_SET )		||
		1 != fread( iData, peSection.SizeOfRawData, 1, file ) )
		{
		fprintf( stderr, "Read error.\n" );
		goto quit;
		}

	importDescriptor = (IMAGE_IMPORT_DESCRIPTOR*)(iData + (importTableRVA - peSection.VirtualAddress));
	/* Iterate through the import descriptors */
	while( importDescriptor->Name )
		{
		if( 0 == stricmp( argv[2], iData + (importDescriptor->Name - peSection.VirtualAddress) ) )
			break;
		importDescriptor++;
		}
	if( !importDescriptor->Name )
		{
		fprintf( stderr, "No import from \"%s\" found.\n", argv[2] );
		goto quit;
		}
	
	dllOffset = peSection.PointerToRawData + (importDescriptor->Name - peSection.VirtualAddress);
	/* Double check we have the correct file offset. */
	if( fseek( file, dllOffset, SEEK_SET )		||
		1 != fread( dllName, strlen(iData + (importDescriptor->Name - peSection.VirtualAddress))+1, 1, file ) )
		{
		fprintf( stderr, "Read error.\n" );
		goto quit;
		}
	if( 0 != stricmp( argv[2], dllName ) )
		{
		fprintf( stderr, "Invalid file offset to import name.\n" );
		goto quit;
		}
	fprintf( stderr, "Changing import dll name \"%s\" in file \"%s\" at file offset %lu to \"%s\".\n",
			 dllName, argv[1], dllOffset, argv[3] );

	if( fseek( file, dllOffset, SEEK_SET ) 			||
		1 != fwrite( argv[3], strlen(argv[3])+1, 1, file ) )
		{
		fprintf( stderr, "Write error.\n" );
		goto quit;
		}

	result = 0;

	  quit:
	if( NULL != importDescriptor )
		free( importDescriptor );
	if( NULL != file )
		fclose( file );
	return result;
	}

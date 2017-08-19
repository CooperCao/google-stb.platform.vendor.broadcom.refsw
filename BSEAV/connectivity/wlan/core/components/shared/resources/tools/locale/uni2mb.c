#include <windows.h>
#include <stdio.h>

FILE* inFile;
FILE* outFile;
unsigned int inBufSize, inBufPos;
unsigned int inReadSize, outWriteSize;
char* inBuf;
char* outBuf;
int outBufSize, requiredSize;
int uni2mb;
DWORD usedDefaultChar;
unsigned short uc;
char ch;
int foundCodePage;

unsigned long codePage = 1252;
int bigEndian = 0;

int convert( void )
	{
	if( uni2mb )
		{
		inBuf[inBufPos] = '\0'; inBuf[inBufPos+1] = '\0';
		if( 1 == swscanf( (wchar_t*)inBuf, L"#pragma code_page ( %lu ) ", &codePage ) )
			{
			/* fprintf( stderr, "Code page set to %lu.\n", codePage ); */
			foundCodePage=1;
			}
		requiredSize = WideCharToMultiByte( codePage, 0, (wchar_t*)inBuf, -1, outBuf, 0, NULL, NULL );
		}
	else
		{
		inBuf[inBufPos] = '\0';
		if( 1 == sscanf( inBuf, "#pragma code_page ( %lu ) ", &codePage ) )
			{
			/* fprintf( stderr, "Code page set to %lu.\n", codePage ); */
			foundCodePage=1;
			}
		requiredSize = MultiByteToWideChar( codePage, 0, inBuf, -1, (wchar_t*)outBuf, 0 )*sizeof(short);
		}

	if( outBufSize <= requiredSize )
		{
		outBufSize = requiredSize < 2*outBufSize ? 2*outBufSize : requiredSize;
		outBuf = (char*)realloc( outBuf, outBufSize*sizeof(char) );
		if( NULL == outBuf )
			return -10;
		}

	usedDefaultChar = 0;
	if( uni2mb )
		{
		if( !WideCharToMultiByte( codePage, 0, (wchar_t*)inBuf, -1,
								  outBuf, outBufSize, NULL, &usedDefaultChar ) )
			{
			fprintf( stderr, "WideCharToMultiByte() failed.\n" );
			return -3;
			}
		}
	else
		{
		if( !MultiByteToWideChar( codePage, 0, inBuf, -1, (wchar_t*)outBuf, outBufSize/sizeof(short) ) )
			{
			fprintf( stderr, "WideCharToMultiByte() failed.\n" );
			return -3;
			}
		}
	if( usedDefaultChar )
		fprintf( stderr, "Warning: Used default char.\n" );
	requiredSize -= outWriteSize;
	if( requiredSize != fwrite( outBuf, sizeof(char), requiredSize, outFile ) )
		{
		fprintf( stderr, "Write failed.\n" );
		return -5;
		}
	return 0;
}

int main( int argc, char** argv )
	{
	foundCodePage = 0;

	if( NULL == strstr( argv[0], "uni2mb" ) )
		uni2mb = 0;
	else
		uni2mb = 1;

	if( 1 < argc && strcmp( argv[1], "-" ) )
		{
		inFile = fopen( argv[1], "rb" );
		if( NULL == inFile )
			{
			fprintf( stderr, "Unable to open %s to convert it.\n", argv[1] );
			return -1;
			}
		}
	else
		inFile = stdin;

	if( argc == 3 && strcmp( argv[2], "-" ) )
		{
		outFile = fopen( argv[2], "wb" );
		if( NULL == outFile )
			{
			fprintf( stderr, "Unable to open %s for writing.\n", argv[2] );
			return -1;
			}
		}
	else
		outFile = stdout;

	inBufSize = 1024;
	inBuf = (char*)malloc( inBufSize*sizeof(char) );
	if( NULL == inBuf )
		return -10;
	outBufSize = inBufSize;
	outBuf = (char*)malloc( outBufSize*sizeof(char) );
	if( NULL == outBuf )
		return -10;
	inBufPos = 0;

	if( uni2mb )
		{
		if( 1 != fread( &uc, sizeof(short), 1, inFile ) )
			{
			fprintf( stderr, "Read error with file %s.\n", argv[1] );
			return -1;
			}
		if( 0xFEFF != uc && 0xFFFE != uc )
			{
			fprintf( stderr, "Warning: no Byte Order Mark (BOM) found. Assuming UTF 16 little endian.\n", argv[1] );
			*(short*)inBuf = uc;
			inBufPos+=2;
			}
		else
			{
			if( 0xFFFE == uc )
				bigEndian = 1;
			}
		inReadSize=2; outWriteSize=1;
		}
	else
		{
		*(short*)outBuf = 0xFEFF;
		if( 1 != fwrite( outBuf, 2, 1, outFile ) )
			{
			fprintf( stderr, "Write failed.\n" );
			return -5;
			}
		inReadSize=1; outWriteSize=2;
		}

	while( 1 == fread( &(inBuf[inBufPos]), inReadSize, 1, inFile ) )
		{
		if( uni2mb && bigEndian )
			{
			ch = inBuf[inBufPos];
			inBuf[inBufPos] = inBuf[inBufPos+1];
			inBuf[inBufPos+1] = ch;
			}

		if( uni2mb )
			uc = *((short*)&(inBuf[inBufPos]));
		else
			ch = inBuf[inBufPos];


		inBufPos += inReadSize;
		if( inBufSize <= inBufPos+inReadSize )
			{
			inBufSize *= 2;
			inBuf = (char*)realloc( inBuf, inBufSize*sizeof(char) );
			if( NULL == inBuf )
				return -10;
			}

		if( (uni2mb && 0x000A == uc ) ||
			(!uni2mb && 0x0A == ch ) )
			{
			if( convert() < 0 )
				return -11;
			inBufPos = 0;
			}
		}
	if( !feof( inFile ) )
		{
		fprintf( stderr, "Read error.\n", argv[1] );
		return -1;
		}
	if( convert() < 0 )
		return -11;

	if( !foundCodePage )
		fprintf( stderr, "Warning: No code page pragma found.\n" );

	fclose(inFile);
	fclose(outFile);

	return 0;
	}

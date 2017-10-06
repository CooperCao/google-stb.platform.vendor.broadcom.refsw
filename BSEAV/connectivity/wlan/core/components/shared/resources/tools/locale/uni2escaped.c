#include <stdio.h>

int main( int argc, char** argv )
	{
	FILE* file;
	unsigned short uc, temp;

	int bigEndian = 0;

	if( argc != 2 )
		{
		fprintf( stderr, "Please specify a unicode file to be escaped.\n" );
		return -1;
		}
	file = fopen( argv[1], "rb" );
	if( NULL == file )
		{
		fprintf( stderr, "Unable to open %s to escape it.\n", argv[1] );
		return -1;		
		}
	if( 1 != fread( &uc, sizeof(short), 1, file ) )
		{
		fprintf( stderr, "Read error with file %s.\n", argv[1] );
		return -1;
		}
	if( 0xFEFF != uc && 0xFFFE != uc )
		{
		fprintf( stderr, "Warning: no Byte Order Mark (BOM) found\n", argv[1] );
		fseek( file, 0, SEEK_SET );
		}
	else
		{
		if( 0xFFFE == uc )
			bigEndian = 1;
		}
	
	while( 1 == fread( &uc, sizeof(short), 1, file ) )
		{
		if( bigEndian )
			{
			temp = uc >> 8;
			temp |= (uc << 8);
			uc = temp;
			}

		if( 0x000D == uc )
			continue;
		if( uc < 128 )
			{
			fprintf( stdout, "%c", uc );
			continue;
			}
		if( fprintf( stdout, "\\x%02X%02X", (uc >> 8 ), (uc & 0x00ff) ) < 0 )
			return -2;
		}

	if( !feof( file ) )
		{
		fprintf( stderr, "Read error.\n", argv[1] );
		return -1;
		}
	return 0;
	}

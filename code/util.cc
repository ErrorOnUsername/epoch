#include "util.hh"
#include <cstdio>
#include <cstdlib>


char const* read_entire_file( char const* path )
{
	if ( !path ) return nullptr;

	FILE* f = fopen( path, "rb" );
	if ( !f )
	{
		fprintf( stderr, "Could not read file: '%s'\n", path );
		return nullptr;
	}

	fseek( f, 0, SEEK_END );
	size_t file_size = ftell( f );
	fseek( f, 0, SEEK_SET );

	char* data = (char*)malloc( file_size + 1 );
	fread( data, 1, file_size, f );

	data[file_size] = '\0';

	fclose( f );

	return (const char*)data;
}

#include "buffer.hh"
#include <cstdlib>


bool create_buffer( Buffer* buffer, char const* path )
{
	buffer->name = path; // FIXME: Write a strdup function that can do this for us bc we need to own that string

	FILE* file = fopen( path, "rb" );
	if ( !file )
	{
		return false;
	}

	fseek( file, 0, SEEK_END );
	size_t size = ftell( file );
	fseek( file, 0, SEEK_SET );

	char* data = (char*)malloc( size + 1 );
	fread( data, size, 1, file );
	data[size] = 0;

	buffer->contents.resize( size + 1 );
	buffer->contents = data;

	return true;
}


void destory_buffer( Buffer* buffer )
{
	// free( (void*)buffer->name ); FIXME: See above FIXME
	buffer->contents.clear();
}


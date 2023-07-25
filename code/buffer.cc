#include "buffer.hh"
#include <cstdlib>


bool create_buffer( Buffer* buffer, std::string const& path )
{
	buffer->name = path; // FIXME: Write a strdup function that can do this for us bc we need to own that string

	char const* file_data = read_entire_file( path.c_str() );
	if ( !file_data ) return false;

	buffer->contents = file_data;

	return true;
}


void destory_buffer( Buffer* buffer )
{
	// free( (void*)buffer->name ); FIXME: See above FIXME
	buffer->contents.clear();
}


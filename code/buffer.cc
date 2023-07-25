#include "buffer.hh"
#include <cstdlib>

#include "draw_gl.hh"
#include "window.hh"


bool create_buffer( Buffer* buffer, std::string const& path )
{
	buffer->name = path;

	char const* file_data = read_entire_file( path.c_str() );
	if ( !file_data ) return false;

	buffer->contents = file_data;

	tokenize_buffer( buffer );

	return true;
}


void destory_buffer( Buffer* buffer )
{
	buffer->contents.clear();
}


void render_buffer( Buffer* buffer )
{
	Vec3 pos { 30.0f, window_get_height() - 30.0f, 0.0f };

	for ( size_t i = 0; i < buffer->lines.size(); i++ )
	{
		Line* l = &buffer->lines[i];
		immediate_push_text( pos, Vec3 { 1.0f, 1.0f, 1.0f }, &buffer->contents[l->start], l->end - l->start );

		pos.y -= 18.0f;
	}
}


void tokenize_buffer( Buffer* buffer )
{
	size_t start = 0;
	for ( size_t i = start; i < buffer->contents.size(); i++ )
	{
		if ( buffer->contents[i] == '\n' )
		{
			Line line { start, i };
			start = i + 1;

			buffer->lines.push_back( line );
		}
	}
}


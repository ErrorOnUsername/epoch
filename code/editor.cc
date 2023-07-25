#include "editor.hh"
#include <cstdlib>
#include <cstring>

#include "draw_gl.hh"
#include "window.hh"


void editor_render( Editor* editor )
{
	immediate_push_text( { 30.0f, window_get_height() - 30.0f, 0.0f }, { 1.0f, 0.5f, 0.02f }, editor->focused_buffer->contents.c_str() );
}


bool open_buffer( Editor* editor, std::string const& path )
{
	// First look and see if we actually need to create a
	// new buffer or if we've already opened it

	for ( size_t i = 0; i < editor->active_buffers.size(); i++ )
	{
		// FIXME: This doesn't work if the file names are loaded throught
		//        weird redundant directory names.
		//        For example:
		//           ./code/main.cc
		//
		//           would not match
		//
		//           ./../proj/code/main.cc
		//
		//           even though in this example they're the same file.
		//        A soultion to this would be to get the cannonical path,
		//        but that could prove to be expensive although just 
		//        hacking it in by prepending the working dir to the path
		//        and manually removing the '.' and '..' refs should be fine.

		Buffer* other_buffer = &editor->active_buffers[i];

		// TODO: Figure out how to handle case-insensitive
		//       filesystems (i.e. APFS and NTFS)
		if ( path == other_buffer->name )
		{
			editor->focused_buffer = other_buffer;
			return true;
		}
	}


	Buffer new_buffer;
	bool created_successfully = create_buffer( &new_buffer, path );
	if ( !created_successfully ) return false;

	editor->active_buffers.push_back( new_buffer );
	editor->focused_buffer = &editor->active_buffers.back();

	return true;
}


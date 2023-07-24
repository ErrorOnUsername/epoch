#include "editor.hh"
#include <cstdlib>
#include <cstring>


void editor_init( Editor* editor )
{
	if ( !editor ) return;

	editor->active_buffers      = nullptr;
	editor->active_buffer_count = 0;
	editor->focused_buffer      = nullptr;
}


void editor_deinit( Editor* editor )
{
	if ( !editor || !editor->active_buffers ) return;

	for ( size_t i = 0; i < editor->active_buffer_count; i++ )
	{
		destory_buffer( &editor->active_buffers[i] );
	}

	free( (void*)editor->active_buffers );

	editor->active_buffers      = nullptr;
	editor->active_buffer_count = 0;
	editor->focused_buffer      = nullptr;
}


bool open_buffer( Editor* editor, char const* path )
{
	// First look and see if we actually need to create a
	// new buffer or if we've already opened it

	size_t path_len = strlen( path );
	for ( size_t i = 0; i < editor->active_buffer_count; i++ )
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
		size_t other_len = strlen( other_buffer->name );

		if ( path_len != other_len ) continue;

		// TODO: Figure out how to handle case-insensitive
		//       filesystems (i.e. APFS and NTFS)
		if ( strncmp( path, other_buffer->name, path_len ) == 0 )
		{
			editor->focused_buffer = other_buffer;
			return true;
		}
	}


	Buffer new_buffer;
	bool created_successfully = create_buffer( &new_buffer, path );
	if ( !created_successfully ) return false;

	{
		Buffer* new_buffers = (Buffer*)malloc( sizeof( Buffer ) * ( editor->active_buffer_count + 1 ) );
		new ( &new_buffers[editor->active_buffer_count].contents ) std::string();

		if ( editor->active_buffers )
		{
			memcpy( (void*)new_buffers, (void*)editor->active_buffers, sizeof( Buffer ) * editor->active_buffer_count );
			free( (void*)editor->active_buffers );
		}

		editor->active_buffers = new_buffers;
	}

	editor->active_buffers[editor->active_buffer_count] = new_buffer;
	editor->active_buffer_count++;
	editor->focused_buffer = &editor->active_buffers[editor->active_buffer_count - 1];

	return true;
}


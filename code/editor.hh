#pragma once

#include "buffer.hh"


struct Editor
{
	size_t  active_buffer_count;
	Buffer* active_buffers;
	Buffer* focused_buffer;
};


void editor_init( Editor* editor );
void editor_deinit( Editor* editor );

bool open_buffer( Editor* editor, char const* path );


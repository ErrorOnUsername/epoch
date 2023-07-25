#pragma once
#include <string>
#include <vector>

#include "buffer.hh"


struct Editor
{
	std::vector<Buffer> active_buffers;
	Buffer*             focused_buffer;
};


void editor_render( Editor* editor );

bool open_buffer( Editor* editor, std::string const& path );


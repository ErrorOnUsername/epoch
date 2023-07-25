#pragma once
#include <string>
#include <vector>

#include "util.hh"


struct Line
{
	size_t start;
	size_t end;
};

struct Buffer
{
	std::string name;
	std::string contents; // This kinda sucks. We should make a custom structure that we can control the allocation behavior of

	std::vector<Line> lines;

	size_t cursor_pos;
};


bool create_buffer( Buffer* buffer, std::string const& path );
void destory_buffer( Buffer* buffer );

void render_buffer( Buffer* buffer );

void tokenize_buffer( Buffer* buffer );


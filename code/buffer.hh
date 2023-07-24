#pragma once
#include <string>


struct Buffer
{
	char const* name;
	std::string contents; // This kinda sucks. We should make a custom structure that we can control the allocation behavior of
};


bool create_buffer( Buffer* buffer, char const* path );
void destory_buffer( Buffer* buffer );


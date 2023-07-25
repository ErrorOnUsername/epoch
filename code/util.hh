#pragma once


struct Vec3
{
	union
	{
		struct
		{
			float x;
			float y;
			float z;
		};
		struct
		{
			float r;
			float g;
			float b;
		};
	};
};

struct Vec2
{
	union
	{
		struct
		{
			float x;
			float y;
		};
		struct
		{
			float r;
			float g;
		};
		struct
		{
			float u;
			float v;
		};
	};
};


char const* read_entire_file( char const* path );

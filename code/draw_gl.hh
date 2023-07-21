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
			float x;
		};
	};
};

void renderer_init();
void renderer_deinit();

void renderer_clear( float r, float g, float b );

void immediate_begin( float width, float height );
void immediate_flush();

void immediate_push_rect( Vec3 pos, Vec2 size, Vec3 color );

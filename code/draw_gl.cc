#include "draw_gl.hh"

#include <glad/glad.h>


void renderer_init()
{
}


void renderer_deinit()
{
}


void renderer_clear( float r, float g, float b )
{
	glClearColor( r, g, b, 1.0f );

	glClear( GL_COLOR_BUFFER_BIT );
}

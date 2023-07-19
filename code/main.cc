#include <cstdio>

#include "window.hh"
#include "draw_gl.hh"

#define DEFAULT_WIDTH  1280
#define DEFAULT_HEIGHT 720

int main()
{
	printf( "Hello, Editor!\n" );

	window_init( DEFAULT_WIDTH, DEFAULT_HEIGHT, "Epoch" );
	renderer_init();

	while ( !window_should_close() )
	{
		renderer_clear( 0.18, 0.18, 0.18 );
	}

	renderer_deinit();
	window_deinit();

	return 0;
}

#include <cstdio>

#include "window.hh"
#include "draw_gl.hh"

#define DEFAULT_WIDTH  1280
#define DEFAULT_HEIGHT 720

int main()
{
	WindowResult res = window_init( DEFAULT_WIDTH, DEFAULT_HEIGHT, "Epoch" );
	if ( res != WindowResult_Success )
	{
		fprintf( stderr, "Window Creation Failed!\n\tmsg: %s\n", window_result_as_str( res ) );
	}

	renderer_init();

	while ( !window_should_close() )
	{
		window_poll_events();

		renderer_clear( 0.18, 0.18, 0.18 );

		window_swap_buffers();
	}

	renderer_deinit();
	window_deinit();

	return 0;
}

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

		// Draw UI
		immediate_begin( window_get_width(), window_get_height() );

		immediate_push_text( { 30.0f, 500.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, "Hello, World!" );

		immediate_flush();

		window_swap_buffers();
	}

	renderer_deinit();
	window_deinit();

	return 0;
}


#include <cstdio>

#include "editor.hh"
#include "draw_gl.hh"
#include "window.hh"

#define DEFAULT_WIDTH  1280
#define DEFAULT_HEIGHT 720

static Editor s_editor;

int main()
{
	WindowResult res = window_init( DEFAULT_WIDTH, DEFAULT_HEIGHT, "Epoch" );
	if ( res != WindowResult_Success )
	{
		fprintf( stderr, "Window Creation Failed!\n\tmsg: %s\n", window_result_as_str( res ) );
	}

	renderer_init();

	bool did_open = open_buffer( &s_editor, "../dragonfly/test_files/test.df" );

	while ( !window_should_close() )
	{
		window_poll_events();

		renderer_clear( 0.18f, 0.18f, 0.18f );

		// Draw literally everything lol
		immediate_begin( window_get_width(), window_get_height() );

		editor_render( &s_editor );

		immediate_flush();

		window_swap_buffers();
	}

	renderer_deinit();
	window_deinit();

	return 0;
}


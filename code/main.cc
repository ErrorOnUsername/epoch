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
	editor_init( &s_editor );

	bool did_open = open_buffer( &s_editor, "code/main.cc" );
	if ( !did_open )
	{
		fprintf( stderr, "Couldn't open test file\n" );
	}
	else
	{
		printf( "%s\n", s_editor.focused_buffer->contents.c_str() );
	}

	while ( !window_should_close() )
	{
		window_poll_events();

		renderer_clear( 0.18f, 0.18f, 0.18f );

		// Draw literally everything lol
		immediate_begin( window_get_width(), window_get_height() );

		immediate_push_text( { 30.0f, 500.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, "yo, wus good mah bois" );

		immediate_flush();

		window_swap_buffers();
	}

	editor_deinit( &s_editor );
	renderer_deinit();
	window_deinit();

	return 0;
}


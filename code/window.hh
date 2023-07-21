#pragma once

#include "util.hh"

enum WindowResult {
	WindowResult_Success,
	WindowResult_GLFWInitFailed,
	WindowResult_WindowCreateFailed,
	WindowResult_ContextCreateFailed,
	WindowResult_Count,
};

WindowResult window_init( int width, int height, char const* title );
void window_deinit();

void window_poll_events();
void window_swap_buffers();
bool window_should_close();
void window_set_title( char const* title );
float window_get_width();
float window_get_height();
Vec2 window_get_scale();

char const* window_result_as_str( WindowResult result );
